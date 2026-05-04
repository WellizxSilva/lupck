# Check the last 16 bytes of a file for a marker

import sys

if len(sys.argv) != 2:
    print("Usage: marker_checker.py <filename>")
    sys.exit(1)

filename = sys.argv[1]

with open(filename,"rb") as f:
    f.seek(-16,2)
    print(f.read())
