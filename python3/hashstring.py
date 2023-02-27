#!/usr/bin/env python3
# -*- coding:utf-8 -*-
import sys

def hash_string( string ):
    try:
        hash = 5381

        for x in string.upper():
            hash = (( hash << 5 ) + hash ) + ord(x)

        return hash & 0xFFFFFFFF
    except:
        pass

if __name__ in '__main__':
    try:
        print('0x%x' % hash_string(sys.argv[1]));
    except IndexError:
        print('usage: %s [string]' % sys.argv[0]);
