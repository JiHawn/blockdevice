data_group = []
f = open("parse_data.txt", 'r')

while True:
    line = f.readline()
    if not line: break
    data = line.split(' ')
    data = data[:-1]
    if len(data) == 8:
        data_group.append(data)
data_group.sort(key=lambda x:x[7])
qd = 0
max = 0
for i in data_group:
    if i[3] == 'D':
        qd += 1
    elif i[3] == 'C':
        qd -= 1
    if max < qd:
        max = qd
print(max)