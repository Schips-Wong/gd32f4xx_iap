# -*- coding: utf-8 -*-
  
# File Name : hex2bin.py
# Version : V1.0.0001
# Date : 2020/01/28
# Author : shuchen_zhang 
  
import os
import time
import six
from functools import reduce
   
# get '.hex' file name from current path
def getFileNamebyEX(path):
    f_list = os.listdir(path)
    for i in f_list:
        filename = os.path.splitext(i)[0]
        extname = os.path.splitext(i)[1]
        if extname == '.hex':
            print(" *Hex file : " + filename + extname+'*')
            return i
  
bin_buf = []# raw data of binary is to be stored here
   
def hex2bin(hex_file_name,bin_file_name):
    with open(hex_file_name,'r') as frd:
        print(' *Hex file : \''+hex_file_name+'\''+' is opened*')
        byte_num = 0
        addr_end = 0
        for line in frd.readlines():
            #line.strip();       #cut off CR
            if(line[0] == ':'):
                if(line[7:9] == '04'):               #Extended Linear Address Record
                   #print('Extended Linear Address Record');
                   line = char2hex(line)
                   if checksum(line) == 0:         #checksum passed
                       addr_h = (line[4]<<24) +(line[5]<<16)
                   else:
                       print('checksum failed!'+str(list(map(hex,line))))
                elif (line[7:9] == '00'):          #Data Record
                    line = char2hex(line)
                    if checksum(line) == 0:
                        addr_l = (line[1]<<8) + line[2]
                        LL = line[0]
                        byte_num = byte_num + LL
                        for data in line[4:-1]:
                            bin_buf.append(data)
                        if LL!=0x10:
                            addr_end = addr_h + addr_l + LL - 1
                        else: 
                            pass
                    else:
                        print('checksum failed!'+str(list(map(hex,line))))
                elif(line[7:9] == '05'):
                    #print('Extended Segment Address Record');
                    line = char2hex(line)
                    if checksum(line) == 0:    pass
                    else:
                        print('checksum failed!'+str(list(map(hex,line))))
                elif(line[7:9] == '01'):         #End of FileRecord
                    #print('End of FileRecord');
                    line = char2hex(line)
                    if checksum(line) == 0:
                        print(' *Hex file successed resolved*')
                        print(' *addr_start: 0x%08X' %(addr_end + 1 - len(bin_buf)))
                        print(' *addr_end  : 0x%08X' %addr_end)
                        print(' *Total size : %d Bytes'%len(bin_buf))
                    else:
                        print('checksum failed!'+str(list(map(hex,line))))
                else:
                    pass       #don't care
            else:
                print('illegal format!')
  
# write data in bin_buf to '*.bin' file
def wr_bin(bin_buf):
    bin_buf_byte = list(map(six.int2byte,bin_buf))
    with open(bin_file_name,'wb') as fwrb:
        print(' *Bin file \''+bin_file_name+'\''+' is opened for write*')
        for data in bin_buf_byte:
            fwrb.write(data)
        print(' *Bin file is successfully written!*')
  
#one line string to hex-8 list,except ':' and CR
def char2hex(line):
    line=list(map(ord,list(line)))
    for num in range(len(line)):
        if line[num]>=0x30 and line[num]<=0x39:
            line[num] = line[num] - 0x30
        elif line[num]>=0x41 and line[num]<=0x5A:
            line[num] = line[num] - 55
        else:
            pass
    line=line[1:-1]     #delete CR and ':', in terms of byte
    for i in range(0,len(line),2):
        line[i] = line[i]*16 + line[i+1]
        newline = line[::2]
    return newline
    
#checksum calculation of every line
def checksum(line):
    #considering if the checksum calculation result is 0x100
    sum = (0x100 - (reduce(lambda x,y:x+y,line[:-1]) % 256)) % 256
    if sum == line[-1]:     #check if sum calculated is equal to checksum byte in hex file
        return 0
    else:
        return 1

starttime = time.time()
hex_file_name = getFileNamebyEX('.')
bin_file_name = hex_file_name[:-4]+'.bin'
hex2bin(hex_file_name,bin_file_name)
wr_bin(bin_buf)
endtime = time.time()
  
print(' Time elapsed:' + str(endtime - starttime))