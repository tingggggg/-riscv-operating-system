# amoadd.w
# Format:
#	amoadd.w rd,rs2,(rs1)
# Description:
#	atomically load a 32-bit signed data value from the address in rs1, 
#   place the value into register rd, apply add the loaded value and the 
#   original 32-bit signed value in rs2, then store the result back to the address in rs1.

    .text           # Define beginning of text section
    .global _start  # Define entry _start

_start:
    li x6, 2        # x6 = 1
    la x7, value
    amoadd.w x5, x6, (x7)

    lw x7, value

    # 1: /z $x5 = 0x12345678
    # 2: /z $x6 = 0x00000002
    # 3: /z $x7 = 0x1234567a


value: 
    .word 0x12345678

stop:
    j stop          # Infinite loop to stop execution

    .end
