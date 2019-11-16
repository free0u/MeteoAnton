Import("env")

print('Inc BuildVersion')

import os 
# dir_path = os.path.dirname(os.path.realpath(__file__))

path = env['PROJECTSRC_DIR']

path += '/BuildVersion.h'

print(path)


def incVersion(ver):
    parts = ver.strip().split()
    return parts[0] + ' ' + parts[1] + ' ' + str(int(parts[2]) + 1) + '\n'

lines = []
with open(path, 'r') as fo:
    lines = fo.readlines()

with open(path, 'w'): pass

with open(path, 'w') as fo:
    fo.write(lines[0])
    fo.write(lines[1])
    fo.write(incVersion(lines[2]))
    fo.write(lines[3])
    fo.write(lines[4])

print('Inc BuildVersion... Done')
