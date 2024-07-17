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
    unsigned char Bytes[22];
    char Instructions[100];

    Fp = fopen(argv[1], "rb");
    if (Fp == NULL) 
    {
        perror("Error opening file");
        return 1;
    }

    fread(Bytes, sizeof(char), 22, Fp);
    fclose(Fp);

    unsigned char Opcode = 0;
    unsigned char D = 0;
    unsigned char W = 0;
    unsigned char Mod = 0;
    unsigned char Reg = 0;
    unsigned char RM = 0;

    for(int i = 0; i < 22; i++)
    {
        if(i % 2 == 0)
        {
            Opcode = Bytes[i] >> 2; 
            D = (Bytes[i] >> 1) & 0b1; 
            W = Bytes[i] & 0b1;

            if(Opcode == 0b100010)
            {
                strcat(Instructions, "mov ");
            }
        }
        else 
        {
            Mod = (Bytes[i] >> 6);
            Reg = (Bytes[i] >> 3) & 0b111;
            RM = Bytes[i] & 0b111; 

            if(Mod == 0b11)
            {
                if(W == 0b1)
                {
                    switch(RM)
                    {
                        case 0b001:
                            strcat(Instructions, "cx, ");
                            break;
                        case 0b010:
                            strcat(Instructions, "dx, ");
                            break;
                        case 0b110:
                            strcat(Instructions, "si, ");
                            break;
                        case 0b011:
                            strcat(Instructions, "bx, ");
                            break;
                        case 0b100:
                            strcat(Instructions, "sp, ");
                            break;
                        case 0b101:
                            strcat(Instructions, "bp, ");
                            break;
                    }
                }
                else if(W == 0b0)
                {
                    if(RM == 0b101)
                    {
                        strcat(Instructions, "ch, ");
                    }
                    else if(RM == 0b000)
                    {
                        strcat(Instructions, "al, ");
                    }
                }
            }
            if(W == 0b1)
            {
                switch(Reg)
                {
                    case 0b111:
                        strcat(Instructions, "di");
                        break;
                    case 0b011:
                        strcat(Instructions, "bx");
                        break;
                    case 0b100:
                        strcat(Instructions, "ah");
                        break;
                    case 0b000:
                        strcat(Instructions, "ax");
                        break;
                    case 0b110:
                        strcat(Instructions, "si");
                        break;
                }
            }
            else if(W == 0b0)
            {
                switch(Reg)
                {
                    case 0b100:
                        strcat(Instructions, "ah");
                        break;
                    case 0b101:
                        strcat(Instructions, "ch");
                        break;
                    case 0b001:
                        strcat(Instructions, "cl");
                        break;
                }
            }

            strcat(Instructions, "\n");
        }
    }

    printf("; %s disassembly:\n", argv[1]);
    printf("bits 16\n");
    printf("%s\n", Instructions);

    return 0;
}
