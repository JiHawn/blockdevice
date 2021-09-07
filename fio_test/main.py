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
f.close()

rm_start = float(logs[-1][6])
rm_end = 0
ra_start = float(logs[-1][6])
ra_end = 0
r_start = float(logs[-1][6])
r_end = 0
rm = 0
ra = 0
r = 0
qd_single = 0
qd = []

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

    if i[3] == 'D':
        qd_single += 1
    elif i[3] == 'C':
        qd_single -= 1
    qd.append(qd_single)

# print('# of RM:', str(rm))
# print('RM spend time:', (rm_end - rm_start) * 1000)
# print('# of RA:', str(ra))
# print('RA spend time:', (ra_end - ra_start) * 1000)
# print('# of R:', str(r))
# print('R spend time:', (r_end - r_start) * 1000)

avg = sum(qd, 0.0) / len(qd)
print("average of qd:", str(qd))