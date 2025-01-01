
# Documentation Credits
### Josh @ Amazon.com
Comment Link: https://www.amazon.com/gp/customer-reviews/RJFTW648Q1UIO/ref=cm_cr_dp_d_rvw_ttl?ie=UTF8&ASIN=B003L14X3A

```
I was disappointed the serial protocol wasn't documented, so i ran the included application and logged the data, with the following result:

Connect 9600 baud, 8N1
Query String (send to switcher): 20 3F 00 00 F1 20 08 00 00 55 20 07 00 00 0A
Responses: (note that for capture purposes, only one input was present at a time, and the device was allowed to autoselect the input)
No inputs:
80 0B C0 01 08 48 52 4D 2D 32 32 31 38 E9 80 02 01 00 52 80 02 01 00 52
Channel 1 Selected:
80 0B C0 01 08 48 52 4D 2D 32 32 31 38 E9 80 02 01 01 0C 80 02 01 01 0C
Channel 2 Selected:
80 0B C0 01 08 48 52 4D 2D 32 32 31 38 E9 80 02 01 02 EE 80 02 01 02 EE
Channel 3 Selected:
80 0B C0 01 08 48 52 4D 2D 32 32 31 38 E9 80 02 01 03 B0 80 02 01 03 B0
Channel 4 Selected:
80 0B C0 01 08 48 52 4D 2D 32 32 31 38 E9 80 02 01 04 33 80 02 01 04 33
Channel 5 Selected:
80 0B C0 01 08 48 52 4D 2D 32 32 31 38 E9 80 02 01 05 6D 80 02 01 05 6D
Channel 6 Selected:
80 0B C0 01 08 48 52 4D 2D 32 32 31 38 E9 80 02 01 06 8F 80 02 01 06 8F
Channel 7 Selected:
80 0B C0 01 08 48 52 4D 2D 32 32 31 38 E9 80 02 01 07 D1 80 02 01 07 D1
Channel 8 Selected:
80 0B C0 01 08 48 52 4D 2D 32 32 31 38 E9 80 02 01 08 90 80 02 01 08 90

Commands:
Switch to input 1:
20 02 01 01 A5 20 08 01 00 91 20 01 01 01 41 20 07 01 00 CE

Switch to input 2:
20 02 01 02 47 20 08 01 00 91 20 01 01 02 A3 20 07 01 00 CE

Switch to input 3:
20 02 01 03 19 20 08 01 00 91 20 01 01 03 FD 20 07 01 00 CE

Switch to input 4:
20 02 01 04 9A 20 08 01 00 91 20 01 01 04 7E 20 07 01 00 CE

Switch to input 5:
20 02 01 05 C4 20 08 01 00 91 20 01 01 05 20 20 07 01 00 CE

Switch to input 6:
20 02 01 06 26 20 08 01 00 91 20 01 01 06 C2 20 07 01 00 CE

Switch to input 7:
20 02 01 07 78 20 08 01 00 91 20 01 01 07 9C 20 07 01 00 CE

Switch to input 8:
20 02 01 08 39 20 08 01 00 91 20 01 01 08 DD 20 07 01 00 CE

Audio and video can be switched separately:
To switch only audio to channel 1:
20 02 01 01 A5 20 08 01 00 91
To switch only video to channel 1:
20 01 01 01 41 20 07 01 00 CE
To switch both at the same time: (just run the commands together)
20 02 01 01 A5 20 08 01 00 91 20 01 01 01 41 20 07 01 00 CE

Checksum calculation:
Taking the command to switch audio to channel 1, the breakdown is:
20 - Prefix
02 - Indicates we're switching audio (01 is video)
01 - Unknown
01 - Channel number to switch to
A5 - Checksum of first four bytes (calculated via CRC-8/MAXIM 1 Wire CRC)
20 - Unknown
08 - Unknown
01 - Unknown
00 - Unknown
91 - Checksum of last four bytes (calculated via CRC-8/MAXIM 1 Wire CRC)
```