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
    unsigned char Bytes[32];
    char Instructions[200] = {0};

    Fp = fopen(argv[1], "rb");
    if (Fp == NULL) 
    {
        perror("Error opening file");
        return 1;
    }

    fread(Bytes, sizeof(char), 32, Fp);
    fclose(Fp);

    unsigned char Opcode = 0;
    unsigned char D = 0;
    unsigned char W = 0;
    unsigned char Mod = 0;
    unsigned char Reg = 0;
    unsigned char RM = 0;
    unsigned char Data8 = 0;
    unsigned short int Data16 = 0;

    for(int i = 0; i < 32; i++)
    {
        printBits(Bytes[i]);
        Opcode = Bytes[i]; 
        if((Opcode >> 2) == 0b100010)
        {
            strcat(Instructions, "mov ");

            D = (Bytes[i] >> 1) & 0b1; 
            W = Bytes[i] & 0b1;
            Mod = (Bytes[i+1] >> 6);
            Reg = (Bytes[i+1] >> 3) & 0b111;
            RM = Bytes[i+1] & 0b111; 

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
                    else if(RM == 0b110)
                    {
                        strcat(Instructions, "dh, ");
                    }
                    else if(RM == 0b000)
                    {
                        if(Mod == 0b00)
                        {                            
                            strcat(Instructions,"[bx + si]");
                        }
                    }
                    else if(RM == 0b011)
                    {
                        if(Mod == 0b00)
                        {
                            strcat(Instructions,"[bp + di]");
                        }
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
                    case 0b000:
                        strcat(Instructions, "al");
                        break;
                }
            }
            strcat(Instructions, "\n");
        }
        else if((Opcode >> 4) == 0b1011)
        {
            strcat(Instructions, "mov ");

            W = (Bytes[i] >> 3) & 0x01;
            Reg = Bytes[i] & 0x07;
            if(W == 0b0)
            {
                Data8 = Bytes[i+1];
                if(Reg == 0b001)
                {
                    strcat(Instructions, "cl, ");
                }
                else if(Reg == 0b101)
                {
                    strcat(Instructions, "ch, ");
                }
                char Data8str[4];
                sprintf(Data8str, "%u", Data8);
                strcat(Instructions, Data8str);
            }
            else if(W == 0b1)
            {
                Data16 = (Bytes[i+2] << 8) | Bytes[i+1];
                // printf("%d\n", Data16);
                if(Reg == 0b001)
                {
                    strcat(Instructions, "cx, ");
                }
                else if(Reg == 0b010)
                {
                    strcat(Instructions, "dx, ");
                }
                char Data16str[20];
                sprintf(Data16str, "%hu", Data16);
                strcat(Instructions, Data16str);
            }
            strcat(Instructions, "\n");
        }
    }

    printf("; %s disassembly:\n", argv[1]);
    printf("bits 16\n");
    printf("%s\n", Instructions);

    return 0;
}
