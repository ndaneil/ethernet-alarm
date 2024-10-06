/**
 * ----------------------------------------------------------------------------------------------------
 * Includes
 * ----------------------------------------------------------------------------------------------------
 */
#include <stdio.h>
#include <string.h>
#include "hardware/gpio.h"

#include "port_common.h"

#include "wizchip_conf.h"
#include "w5x00_spi.h"

#include "httpServer.h"
#include "webpage.h"

#include "dhcp.h"

#include "timer.h"

/**
 * ----------------------------------------------------------------------------------------------------
 * Macros
 * ----------------------------------------------------------------------------------------------------
 */
/* Clock */
#define PLL_SYS_KHZ (133 * 1000)

/* Buffer */
#define ETHERNET_BUF_MAX_SIZE (1024 * 2)

/* Socket */
#define SOCKET_DHCP 0

/* Retry count */
#define DHCP_RETRY_COUNT 5

/* Socket */
#define HTTP_SOCKET_MAX_NUM 4

/**
 * ----------------------------------------------------------------------------------------------------
 * Variables
 * ----------------------------------------------------------------------------------------------------
 */
/* Network */
static wiz_NetInfo g_net_info =
    {
        .mac = {0x00, 0x08, 0xDC, 0x12, 0x34, 0x56}, // MAC address
        .ip = {192, 168, 11, 2},                     // IP address
        .sn = {255, 255, 255, 0},                    // Subnet Mask
        .gw = {192, 168, 11, 1},                     // Gateway
        .dns = {8, 8, 8, 8},                         // DNS server
        .dhcp = NETINFO_DHCP                         // DHCP enable/disable
};
static uint8_t g_ethernet_buf[ETHERNET_BUF_MAX_SIZE] = {
    0,
}; // common buffer

/* HTTP */
static uint8_t g_http_send_buf[ETHERNET_BUF_MAX_SIZE] = {
    0,
};
static uint8_t g_http_recv_buf[ETHERNET_BUF_MAX_SIZE] = {
    0,
};
static uint8_t g_http_socket_num_list[HTTP_SOCKET_MAX_NUM] = {0, 1, 2, 3};

/* DHCP */
static uint8_t g_dhcp_get_ip_flag = 0;


/* ALARM VARIABLES */
#define PIR_PIN 2
#define DOOR_PIN 3

static long last_movement_time = 0;
static long last_door_change_time = 0;
static bool door_open = true;

/**
 * ----------------------------------------------------------------------------------------------------
 * Functions
 * ----------------------------------------------------------------------------------------------------
 */
/* Clock */
static void set_clock_khz(void);

/* DHCP */
static void wizchip_dhcp_init(void);
static void wizchip_dhcp_assign(void);
static void wizchip_dhcp_conflict(void);

/* Interrupt callback */
static void gpio_change_callback(uint, uint32_t);

/**
 * ----------------------------------------------------------------------------------------------------
 * Main
 * ----------------------------------------------------------------------------------------------------
 */
int main()
{
    /* Initialize */
    uint8_t retval = 0;
    uint8_t dhcp_retry = 0;
    uint8_t i = 0;
    long msSinceBoot = 0;
    char str_movement[LAST_MOTION_MS_LENGTH+1];
    char str_door_state_open[] = "  \"OPEN\"";
    char str_door_state_closed[] = "\"CLOSED\"";
    char str_door[LAST_DOOR_CHANGE_MS_LENGTH+1];

    set_clock_khz();

    stdio_init_all();

    wizchip_spi_initialize();
    wizchip_cris_initialize();

    wizchip_reset();
    wizchip_initialize();
    wizchip_check();

    if (g_net_info.dhcp == NETINFO_DHCP) // DHCP
    {
        wizchip_dhcp_init();
    }
    else // static
    {
        network_initialize(g_net_info);

        /* Get network information */
        print_network_information(g_net_info);
    }

    httpServer_init(g_http_send_buf, g_http_recv_buf, HTTP_SOCKET_MAX_NUM, g_http_socket_num_list);

    reg_httpServer_webContent("index.html", WEBPAGE_HTML);

    /* Setting up GPIO and interrupts */
    gpio_init(PIR_PIN);
    gpio_init(DOOR_PIN);
    gpio_set_dir(PIR_PIN, GPIO_IN);
    gpio_set_dir(DOOR_PIN, GPIO_IN);
    gpio_pull_up(DOOR_PIN);

    gpio_set_irq_enabled_with_callback(DOOR_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_change_callback);
    gpio_set_irq_enabled_with_callback(PIR_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_change_callback);


    door_open = gpio_get(DOOR_PIN);




    /* Infinite loop */
    while (1)
    {
        /* Assigned IP through DHCP */
        if (g_net_info.dhcp == NETINFO_DHCP)
        {
            retval = DHCP_run();

            if (retval == DHCP_IP_LEASED)
            {
                if (g_dhcp_get_ip_flag == 0)
                {
                    printf(" DHCP success\n");

                    g_dhcp_get_ip_flag = 1;
                }
            }
            else if (retval == DHCP_FAILED)
            {
                g_dhcp_get_ip_flag = 0;
                dhcp_retry++;

                if (dhcp_retry <= DHCP_RETRY_COUNT)
                {
                    printf(" DHCP timeout occurred and retry %d\n", dhcp_retry);
                }
            }

            if (dhcp_retry > DHCP_RETRY_COUNT)
            {
                printf(" DHCP failed\n");

                DHCP_stop();

                while (1)
                    ;
            }

            wizchip_delay_ms(1000); // wait for 1 second
        }

        msSinceBoot = to_ms_since_boot(get_absolute_time());

        snprintf(str_movement, LAST_MOTION_MS_LENGTH+1, "%12d", msSinceBoot-last_movement_time);
        strncpy(&WEBPAGE_HTML[LAST_MOTION_MS_START], str_movement, LAST_MOTION_MS_LENGTH);
        snprintf(str_door, LAST_DOOR_CHANGE_MS_LENGTH+1, "%12d", msSinceBoot-last_door_change_time);
        strncpy(&WEBPAGE_HTML[LAST_DOOR_CHANGE_MS_START], str_door, LAST_DOOR_CHANGE_MS_LENGTH);
        strncpy(&WEBPAGE_HTML[DOOR_STATE_START], door_open?str_door_state_open:str_door_state_closed, DOOR_STATE_LENGTH);

        for (i = 0; i < HTTP_SOCKET_MAX_NUM; i++)
        {
            httpServer_run(i);
        }

    }
}

/**
 * ----------------------------------------------------------------------------------------------------
 * Functions
 * ----------------------------------------------------------------------------------------------------
 */

/* GPIO interrupt handling */
static void gpio_change_callback(uint gpio, uint32_t events) {
    if(gpio == DOOR_PIN) {
        door_open = gpio_get(DOOR_PIN);
        last_door_change_time = to_ms_since_boot(get_absolute_time());
    } else if(gpio == PIR_PIN) {
        last_movement_time = to_ms_since_boot(get_absolute_time());
    }
}

/* Clock */
static void set_clock_khz(void)
{
    // set a system clock frequency in khz
    set_sys_clock_khz(PLL_SYS_KHZ, true);

    // configure the specified clock
    clock_configure(
        clk_peri,
        0,                                                // No glitchless mux
        CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS, // System PLL on AUX mux
        PLL_SYS_KHZ * 1000,                               // Input frequency
        PLL_SYS_KHZ * 1000                                // Output (must be same as no divider)
    );
}

/* DHCP */
static void wizchip_dhcp_init(void)
{
    printf(" DHCP client running\n");

    DHCP_init(SOCKET_DHCP, g_ethernet_buf);

    reg_dhcp_cbfunc(wizchip_dhcp_assign, wizchip_dhcp_assign, wizchip_dhcp_conflict);
}

static void wizchip_dhcp_assign(void)
{
    getIPfromDHCP(g_net_info.ip);
    getGWfromDHCP(g_net_info.gw);
    getSNfromDHCP(g_net_info.sn);
    getDNSfromDHCP(g_net_info.dns);

    g_net_info.dhcp = NETINFO_DHCP;

    /* Network initialize */
    network_initialize(g_net_info); // apply from DHCP

    print_network_information(g_net_info);
    printf(" DHCP leased time : %ld seconds\n", getDHCPLeasetime());
}

static void wizchip_dhcp_conflict(void)
{
    printf(" Conflict IP from DHCP\n");

    // halt or reset or any...
    while (1)
        ; // this example is halt.
}
