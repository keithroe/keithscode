#!/usr/bin/env python

import sys

def isRegular( num ):
    strnum = str(num)
    for c in strnum:
        if c != '3' and c != '5':
            return False

    return True

def findLargestRegular( num_digits ):
    max = int( '5'*num_digits )
    min = int( '3'*num_digits )

    print "reg with {} digits is {}".format( num_digits, 0 )
    print "\tsearching {}-{}".format( min, max )
    for num in xrange( max, min, -2 ): # small shortcut, we at least know it is odd
        if isRegular( num ):
            return num
    return -1

lines = sys.stdin.readlines()
for line in lines[1:]: 
    print findLargestRegular( int(line) )
