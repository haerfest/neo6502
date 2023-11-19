#!/usr/bin/env python


import sys


for filepath in sys.argv[1:]:
    with open(filepath, 'rb') as f:
        all_bytes = f.read()
        as_hex    = ['0x{:02X}'.format(byte) for byte in all_bytes]
        as_string = ','.join(as_hex)
        print(as_string)
            
