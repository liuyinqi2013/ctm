#!/usr/bin/python
import os
import sys 
import time

if __name__ == '__main__':
    now = time.time()
    if len(sys.argv) >= 2:
        now = int(sys.argv[1])

    print(time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(now)))
