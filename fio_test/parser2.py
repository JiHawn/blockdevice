c = open('qd_log_scenario_c.txt', 'r') # 9.4s
d = open('qd_log_scenario_d.txt', 'r') # 7.1s + 0.5s

c_logs = []
d_logs = []
issue = 0
complete_time = []

while True:
    line = c.readline()
    if not line:
        break
    c_logs.append(line.split(','))

while True:
    line = d.readline()
    if not line:
        break
    d_logs.append(line.split(','))

# for i in c_logs:
#     if i[5] == '4096' and i[2] == "RA":
#         if i[3] == 'D':
#             issue = float(i[6])
#         elif i[3] == "C":
#             complete_time.append(float(i[6]) - issue)
#
# print(f"scenario C: only data I/O average time = {sum(complete_time)/len(complete_time)*1000}")
# print(f"scenario C: sum of I/O time (only data) = {sum(complete_time)}")
#
# issue = 0
# complete_time = []
# for i in d_logs:
#     if i[5] == '4096' and i[2] == "RA":
#         if i[3] == 'D':
#             issue = float(i[6])
#         elif i[3] == "C":
#             complete_time.append(float(i[6]) - issue)
#
# print(f"scenario D: only data I/O average time = {sum(complete_time)/len(complete_time)*1000}")
# print(f"scenario D: sum of I/O time (only data) = {sum(complete_time)}")
#
# issue = {}
# complete_time = []
# for i in c_logs:
#     if i[2] == "RA":
#         if i[3] == 'D':
#             issue[i[4]] = float(i[6])
#         elif i[3] == "C":
#             complete_time.append(float(i[6]) - issue[i[4]])
#             del issue[i[4]]
#
# print(f"scenario C: I/O average time = {sum(complete_time)/len(complete_time)*1000}")
# print(f"scenario C: sum of I/O time = {sum(complete_time)}")
#
# issue = {}
# complete_time = []
# for i in d_logs:
#     if i[2] == "RA":
#         if i[3] == 'D':
#             issue[i[4]] = float(i[6])
#         elif i[3] == "C":
#             complete_time.append(float(i[6]) - issue[i[4]])
#             del issue[i[4]]
#
# print(f"scenario D: I/O average time = {sum(complete_time)/len(complete_time)*1000}")
# print(f"scenario D: sum of I/O time = {sum(complete_time)}")

# issue = {}
# complete_time = []
# for i in c_logs:
#     if i[5] != '4096' and i[2] == "RA":
#         if i[3] == 'D':
#             issue[i[4]] = float(i[6])
#         elif i[3] == "C":
#             complete_time.append(float(i[6]) - issue[i[4]])
#             del issue[i[4]]
#
# # print(f"scenario C: D to C gap = {sum(complete_time)/len(complete_time)*1000}")
# print(f"scenario C: sum of metadata access time = {sum(complete_time)}s")
#
# issue = {}
# complete_time = []
# for i in d_logs:
#     if i[5] != '4096' and i[2] == "RA":
#         if i[3] == 'D':
#             issue[i[4]] = float(i[6])
#         elif i[3] == "C":
#             complete_time.append(float(i[6]) - issue[i[4]])
#             del issue[i[4]]
#
# # print(f"scenario D: I/O average time with RM = {sum(complete_time) / len(complete_time)*1000}")
# print(f"scenario D: sum of metadata access time = {sum(complete_time)}s")
#
#
# issue = {}
# complete_time = []
# for i in c_logs:
#     if i[3] == 'D':
#         issue[i[4]] = float(i[6])
#     elif i[3] == "C":
#         complete_time.append(float(i[6]) - issue[i[4]])
#         del issue[i[4]]
#
# # print(f"scenario C: D to C gap = {sum(complete_time)/len(complete_time)*1000}")
# print(f"scenario C: sum of issue to complete time = {sum(complete_time)}s")
#
# issue = {}
# complete_time = []
# for i in d_logs:
#     if i[3] == 'D':
#         issue[i[4]] = float(i[6])
#     elif i[3] == "C":
#         complete_time.append(float(i[6]) - issue[i[4]])
#         del issue[i[4]]
#
# # print(f"scenario D: I/O average time with RM = {sum(complete_time) / len(complete_time)*1000}")
# print(f"scenario D: sum of issue to complete time = {sum(complete_time)}s")

complete = 0
time = []
for i in c_logs:
    if i[3] == 'C':
        complete = float(i[6])
    elif i[3] == 'D':
        if complete != 0:
            time.append(float(i[6]) - complete)

# print(f"scenario C: average of distance of I/O = {sum(time)/len(time)*1000}")
print(f"scenario C: minimum of complete to next issue time = {min(time)}s")
print(f"scenario C: maximum of complete to next issue time = {max(time)}s")

complete = 0
time = []
for i in d_logs:
    if i[3] == 'C':
        complete = float(i[6])
    elif i[3] == 'D':
        if complete != 0:
            time.append(float(i[6]) - complete)

# print(f"scenario D: average of distance of I/O = {sum(time) / len(time)*1000}")
print(f"scenario D: minimum of complete to next issue time = {min(time)}s")
print(f"scenario D: maximum of complete to next issue time = {max(time)}s")
