Import("env")

print('Inc BuildVersion')

import os 
# dir_path = os.path.dirname(os.path.realpath(__file__))

path = env['PROJECT_SRC_DIR']
path += '/system/BuildVersion.h'

pathVer = env['PROJECT_SRC_DIR']
pathVer += '/../version.txt'

print(path)
print(env.Dump())
# for key in env:
#     print(key)
#     print(env[key])

def incVersion(ver):
    parts = ver.strip().split()
    return parts[0] + ' ' + parts[1] + ' ' + str(int(parts[2]) + 1) + '\n'

lines = []
with open(path, 'r') as fo:
    lines = fo.readlines()

with open(path, 'w'): pass

ver = []
with open(pathVer, 'r') as fo:
    ver = fo.readlines()

with open(path, 'w') as fo:
    fo.write(lines[0])
    fo.write(lines[1])
    fo.write(incVersion(lines[2]))
    # fo.write(lines[3])
    fo.write("#define FIRMWARE_VERSION ")
    fo.write(ver[0].strip() + '\n')
    fo.write(lines[4])

print('Inc BuildVersion... Done')
