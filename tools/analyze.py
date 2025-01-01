import subprocess


r = subprocess.Popen(['readelf', '-s', 'iso/boot/kernel.elf'], stdout=subprocess.PIPE)
d = r.stdout.read().split(b'\n')[3:];

processed = []

for i in d:
    data = [i for i in i.strip().decode('utf-8').split(' ') if i != '']

    if len(data) == 0:
        continue

    if data[2].startswith('0x'):
        data[2] = int(data[2], base=16)
    else:
        data[2] = int(data[2], base=10)

    processed.append(data)

processed = sorted(processed, key=lambda x: x[2], reverse=True)

column_widths = [0] * len(processed[0])

for n, _ in enumerate(processed[0]):
    for i in processed:
        if len(i) != len(processed[0]):
            continue

        column_widths[n] = max(column_widths[n], len(str(i[n])))


for i in processed:
    for n, j in enumerate(i):
        print(j, " " * ((column_widths[n] - len(str(j))) + 1), sep='', end='')
    print()
