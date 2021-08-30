from datetime import datetime, date
import time

f = open('blkoutput.txt', 'r')
today = date.today().strftime("%Y-%m-%d ")
logs = []

while True:
    line = f.readline()
    if not line:
        break
    split = line.split(' ')
    ts = split[6]
    dt = today + ts[:-8]
    try:
        micro = float(ts[-7:-1]) / 1000000
        timestamp = time.mktime(datetime.strptime(dt, '%Y-%m-%d %H:%M:%S').timetuple()) + micro
    except ValueError:
        continue
    split[6] = str(timestamp)
    logs.append(split)

rm_start = float(logs[-1][6])
rm_end = 0
ra_start = float(logs[-1][6])
ra_end = 0
r_start = float(logs[-1][6])
r_end = 0
rm = 0
ra = 0
r = 0

for i in logs:
    ts = float(i[6])
    if i[2] == 'RM':
        rm += 1
        if ts < rm_start:
            rm_start = ts
        if ts > rm_end:
            rm_end = ts
    elif i[2] == 'RA':
        ra += 1
        if ts < ra_start:
            ra_start = ts
        if ts > rm_end:
            ra_end = ts
    elif i[2] == 'R':
        r += 1
        if ts < r_start:
            r_start = ts
        if ts > r_end:
            r_end = ts

print('# of RM:', str(rm), 'RM spend time:', (rm_end - rm_start) * 1000)
print('# of RA:', str(ra), 'RA spend time:', (ra_end - ra_start) * 1000)
print('# of R:', str(r), 'R spend time:', (r_end - r_start) * 1000)