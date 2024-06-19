#include <stdio.h>
#include <string.h>

void printBits(unsigned char Byte) {
    // Iterate through each bit position (from MSB to LSB)
    for (int i = 7; i >= 0; --i) {
        // Check if the i-th bit is set (1) or not (0)
        if (Byte & (1 << i))
            printf("1");
        else
            printf("0");
    }
    printf("\n");
}

int main(int argc, char *argv[])
{
    FILE *Fp;
    unsigned char Bytes[2];
    char Instructions[11];

    Fp = fopen(argv[1], "r");
    if (Fp == NULL) 
    {
        perror("Error opening file");
        return 1;
    }


    while(fread(Bytes, sizeof(Bytes), 1, Fp) > 0)
    {
        unsigned char Opcode = 0;
        unsigned char D = 0;
        unsigned char W = 0;
        unsigned char Mod = 0;
        unsigned char Reg = 0;
        unsigned char RM = 0;

        // for(int i = 0; i < 2; i++)
        // {
        //     printBits(Bytes[i]);
        // }

        Opcode = Bytes[0] >> 2; 
        D = (Bytes[0] >> 1) & 0b1; // Shift right by 6 bits and mask with 0x01 (binary: 00000001)
        W = Bytes[0] & 0b1; // Shift right by 6 bits and mask with 0x01 (binary: 00000001)

        Mod = (Bytes[1] >> 6); // Shift right by 6 bits and mask with 0x03
        Reg = (Bytes[1] >> 3) & 0b111; // Shift right by 3 bits and mask with 0x07
        RM = Bytes[1] & 0b111; // Mask with 0x07 to isolate the last three bits

        if(Opcode == 0b100010) // 100010
        {
            strcat(Instructions, "mov ");
        }

        if(Mod == 0b11 && W == 0b1 && RM == 0b001)
        {
            strcat(Instructions, "cx, ");
        }

        if(Reg == 0b011 && W == 0b1)
        {
            strcat(Instructions, "bx");
        }
    }

    printf("; %s disassembly:\n", argv[1]);
    printf("bits 16\n");
    printf("%s\n", Instructions);

    fclose(Fp);

    return 0;
}
