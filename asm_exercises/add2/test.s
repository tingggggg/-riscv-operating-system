# Add
# Format:
#	ADD RD, RS1, RS2
# Description:
#	The contents of RS1 is added to the contents of RS2 and the result is 
#	placed in RD.

    .text           # Define beginning of text section
    .global _start  # Define entry _start

_start:
    li x6, 1        # x6 = 1
    li x7, -2       # x7 = -2
    add x5, x6, x7  # x5 = x6 + x7

stop:
    j stop          # Infinite loop to stop execution

    .end

# 1: /z $x5 = 0xffffffff
# 2: /z $x6 = 0x00000001
# 3: /z $x7 = 0xfffffffe