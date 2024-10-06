#ifndef _WEBPAGE_H_
#define _WEBPAGE_H_

char WEBPAGE_HTML[] = R"(<!DOCTYPE html><html lang="en"><head><meta charset="UTF-8"><meta name="viewport" content="width=device-width, initial-scale=1.0"><meta http-equiv="refresh" content="10"><title>Home alarm system</title><style>body{font-family:'Arial',sans-serif;margin:0;padding:20px;background-color:#f5f5f5}.container{max-width:600px;margin:0 auto;background-color:#fff;padding:20px;border-radius:10px;box-shadow:0 2px 5px rgb(0 0 0 / .1)}h1{text-align:center;color:#333}.status-card{margin-bottom:20px;padding:15px;border-radius:5px;background-color:#f8f9fa;border-left:5px solid #007bff}.status-card h2{margin-top:0;color:#007bff}.time-display{font-size:20px;font-weight:700;margin:10px 0}.exact-time{font-size:16px;color:#666;margin-top:5px}.door-status{display:inline-block;padding:5px 10px;border-radius:3px;font-weight:700}.door-open{background-color:#dc3545;color:#fff}.door-closed{background-color:#28a745;color:#fff}.refresh-notice{text-align:center;color:#666;font-size:14px;margin-top:20px}#countdown{font-weight:700}</style></head><body><div class="container"><h1>Home alarm system</h1><div class="status-card"><h2>Motion Detection</h2><div id="lastMotion" class="time-display">Checking...</div><div id="exactMotionTime" class="exact-time"></div></div><div class="status-card"><h2>Door Status</h2><div id="doorStatus" class="door-status">Checking...</div><div id="lastDoorChange" class="time-display">Checking...</div><div id="exactDoorTime" class="exact-time"></div></div><div class="refresh-notice">Page refreshes automatically every 10 seconds. Next refresh in <span id="countdown">10</span> seconds.</div></div><script>const serverData={lastMotionMs:            ,doorState:  "OPEN", lastDoorChangeMs:            ,currentServerTime:Date.now()};function formatDuration(ms){const seconds=Math.floor(ms/1000);const minutes=Math.floor(seconds/60);const hours=Math.floor(minutes/60);const days=Math.floor(hours/24);const remainingHours=hours%24;const remainingMinutes=minutes%60;const remainingSeconds=seconds%60;const parts=[];if(days>0){parts.push(`${days} day${days > 1 ? 's' : ''}`)}
if(remainingHours>0){parts.push(`${remainingHours} hour${remainingHours > 1 ? 's' : ''}`)}
if(remainingMinutes>0){parts.push(`${remainingMinutes} minute${remainingMinutes > 1 ? 's' : ''}`)}
if(remainingSeconds>0){parts.push(`${remainingSeconds} second${remainingSeconds > 1 ? 's' : ''}`)}
if(parts.length===0){return'just now'}
return `${parts.join(', ')} ago`}
function formatExactTime(timestamp){const date=new Date(timestamp);return date.toLocaleString('en-US',{year:'numeric',month:'short',day:'numeric',hour:'2-digit',minute:'2-digit',second:'2-digit'})}
function updateTimes(){const now=Date.now();const motionElapsed=now-(serverData.currentServerTime-serverData.lastMotionMs);document.getElementById('lastMotion').textContent=`Last movement: ${formatDuration(motionElapsed)}`;document.getElementById('exactMotionTime').textContent=`Exact time: ${formatExactTime(serverData.currentServerTime - serverData.lastMotionMs)}`;const doorStatus=document.getElementById('doorStatus');doorStatus.textContent=serverData.doorState.toUpperCase();doorStatus.className=`door-status door-${serverData.doorState.toLowerCase()}`;const doorElapsed=now-(serverData.currentServerTime-serverData.lastDoorChangeMs);document.getElementById('lastDoorChange').textContent=`Last change: ${formatDuration(doorElapsed)}`;document.getElementById('exactDoorTime').textContent=`Exact time: ${formatExactTime(serverData.currentServerTime - serverData.lastDoorChangeMs)}`}
function updateCountdown(){const countdown=document.getElementById('countdown');let seconds=parseInt(countdown.textContent);if(seconds>0){countdown.textContent=seconds-1}}
updateTimes();setInterval(updateTimes,1000);setInterval(updateCountdown,1000)
    </script>
</body>
</html>
)";

#define LAST_MOTION_MS_START 1650
#define LAST_MOTION_MS_LENGTH 12
#define DOOR_STATE_START 1673
#define DOOR_STATE_LENGTH 8
#define LAST_DOOR_CHANGE_MS_START 1700
#define LAST_DOOR_CHANGE_MS_LENGTH 12

#endif /* _WEBPAGE_H_ */
