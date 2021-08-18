from subprocess import Popen, PIPE
from datetime import datetime, date

import time
import os

path_dir = '4kb/'
file_path = os.listdir(path_dir)
full_path = [path_dir + x for x in file_path]
issue_time = []
complete_time = []
i = 1
qd = 0
max_qd = 0

for path in full_path:
    res = Popen(['sudo', '-S', 'hdparm', '--fibmap', path], stdout=PIPE)
    out, err = res.communicate()
    file_lba = int(out.decode()[164:175])
    issue_time.append([i, file_lba])
    complete_time.append([i, file_lba])
    i += 1

f = open('temp.txt', 'r')
today = date.today().strftime("%Y-%m-%d ")
trace = []

while True:
    line = f.readline()
    if not line:
        break
    splited = line.split(' ')
    blk_time = splited[6]
    blk_datetime = today + blk_time[:-8]
    try:
        blk_micro = float(blk_time[-7:-1]) / 1000000
        blk_timestamp = time.mktime(datetime.strptime(blk_datetime, '%Y-%m-%d %H:%M:%S').timetuple()) + blk_micro
    except ValueError:
        continue
    splited[6] = blk_timestamp
    trace.append(splited)

for i in trace:

    blk_lba = int(i[4])
    blk_timestamp = i[6]

    if i[3] == 'D':
        qd += 1
        if max_qd < qd:
            max_qd = qd
        for j in issue_time:
            if blk_lba == j[1]:
                j[1] = blk_timestamp
                break
    elif i[3] == 'C':
        qd -= 1
        for j in complete_time:
            if blk_lba == j[1]:
                j[1] = blk_timestamp
                break
f.close()

for issue, complete in zip(issue_time, complete_time):
    print(str(issue[1]) + "," + str(complete[1]))

print(max_qd)
