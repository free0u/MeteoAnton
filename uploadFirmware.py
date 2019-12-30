#!/usr/bin/python

exit(0)

import os 
dir_path = os.path.dirname(os.path.realpath(__file__))

print(dir_path)


buildVersionPath = dir_path + '/src/BuildVersion.h'


print(buildVersionPath)


lines = []
with open(buildVersionPath, 'r') as fo:
    lines = fo.readlines()

def parseVersion(s):
    parts = s.strip().split()
    return parts[2]


ver = parseVersion(lines[3])
print(ver)



with open(dir_path + '/.pio/build/nodemcuv2/version.txt', 'w') as fo:
    fo.write(ver)
