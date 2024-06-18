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
    char Instructions[5];

    Fp = fopen(argv[1], "rb");
    if (Fp == NULL) 
    {
        perror("Error opening file");
        return 1;
    }

    printf("; %s disassembly:\n", argv[1]);
    printf("bits 16\n");

    while(fread(Bytes, sizeof(Bytes), 1, Fp) > 0)
    {
        unsigned char Operand = 0;
        bool Destination = false;
        bool Width = false;
        unsigned char Mode = 0;
        unsigned char Register = 0;
        unsigned char RegisterMemoryField = 0;

        for (int i = 0;
             i < 2; 
             i++) 
        {
            if (i == 0)
            {
                Operand = Bytes[i] >> 2; 
                Destination = (Bytes[i] >> 6) & 0x01; // Shift right by 6 bits and mask with 0x01 (binary: 00000001)
                Width = (Bytes[i] >> 7) & 0x01; // Shift right by 6 bits and mask with 0x01 (binary: 00000001)
                printf("%d\n", Destination);
                printf("%d\n", Width);
            }

            if (Operand == 34 && i == 0) 
            {
                strcat(Instructions, "mov ");
            }
            printBits(Bytes[i]);
        }
    }

    printf("%s", Instructions);

    fclose(Fp);

    return 0;
}
