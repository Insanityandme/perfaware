#include <stdio.h>
#include <string.h>
#include <stdint.h>

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
    unsigned char Bytes[39];
    char Instructions[1000] = {0};

    Fp = fopen(argv[1], "rb");
    if (Fp == NULL) 
    {
        perror("Error opening file");
        return 1;
    }

    fread(Bytes, sizeof(char), 39, Fp);
    fclose(Fp);

    uint8_t Opcode = 0;
    uint8_t D = 0;
    uint8_t W = 0;
    uint8_t Mod = 0;
    uint8_t Reg = 0;
    uint8_t RM = 0;
    uint8_t Data8 = 0;
    int8_t Data8sig = 0;
    uint16_t Data16 = 0;
    int16_t Data16sig = 0;

    for(int i = 0; i < 39; i++)
    {
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
            }

            if(Mod == 0b00)
            {
                if(W == 0b0)
                {
                    if(Reg == 0b000)
                    {
                        strcat(Instructions, "al, ");
                    }
                }
                else if(W == 0b1)
                {
                    if(Reg == 0b011)
                    {
                        strcat(Instructions, "bx, ");
                    }
                    if(D == 0b1)
                    {
                        if(Reg == 0b101)
                        {
                            strcat(Instructions, "bp, ");
                        }
                    }
                }

                if(RM == 0b000)
                {
                    strcat(Instructions, "[bx + si]");
                }
                else if(RM == 0b011)
                {
                    strcat(Instructions, "[bp + di]");
                }
                else if(RM == 0b110)
                {
                    Data16 = (Bytes[i+3] << 8) | Bytes[i+2];
                    char Data16str[20];
                    sprintf(Data16str, "[%hu]", Data16);
                    strcat(Instructions, Data16str);
                }
            }

            if(Mod == 0b01)
            {
                if(W == 0b1)
                {
                    if(Reg == 0b010)
                    {
                        strcat(Instructions, "dx, ");
                        if(RM == 0b110)
                        {
                            strcat(Instructions, "[bp]");
                        }
                    }
                    else if(Reg == 0b000)
                    {
                        strcat(Instructions, "ax, ");
                    }
                }
                else if(W == 0b0)
                {
                    if(Reg == 0b100)
                    {
                        strcat(Instructions, "ah, ");
                    }
                }

                if(RM == 0b000)
                {
                    Data8 = Bytes[i+2];
                    char Data8str[4];
                    strcat(Instructions, "[bx + si + ");
                    sprintf(Data8str, "%u]", Data8);
                    strcat(Instructions, Data8str);
                }
                else if(RM == 0b001)
                {
                    Data8sig = Bytes[i+2];
                    char Data8str[4];
                    strcat(Instructions, "[bx + di ");
                    sprintf(Data8str, "- %d]", -Data8sig);
                    strcat(Instructions, Data8str);
                }
                else if(RM == 0b111)
                {
                    Data8sig = Bytes[i+2];
                    char Data8str[4];
                    strcat(Instructions, "[bx ");
                    sprintf(Data8str, "- %d]", -Data8sig);
                    strcat(Instructions, Data8str);
                }
            }

            if(Mod == 0b10)
            {
                if(RM == 0b100)
                {
                    Data16sig = (Bytes[i+3] << 8) | Bytes[i+2];
                    char Data16str[20];
                    strcat(Instructions, "[si ");
                    sprintf(Data16str, "- %hd], ", -Data16sig);
                    strcat(Instructions, Data16str);
                }
                if(W == 0b1)
                {
                    if(Reg == 0b001)
                    {
                        strcat(Instructions, "cx");
                    }
                }

                if(W == 0b0)
                {
                    if(Reg == 0b000)
                    {
                        strcat(Instructions, "al, ");
                    }
                }

                if(RM == 0b000)
                {
                    Data16 = (Bytes[i+3] << 8) | Bytes[i+2];
                    char Data16str[20];
                    strcat(Instructions, "[bx + si + ");
                    sprintf(Data16str, "%hu]", Data16);
                    strcat(Instructions, Data16str);
                }
            }

            if(Mod == 0b00)
            {
                if(D == 0b0)
                {
                    if(RM == 0b001)
                    {
                        strcat(Instructions, "[bx + di], ");
                    }

                    if(W == 0b1)
                    {
                        if(Reg == 0b001)
                        {
                            strcat(Instructions, "cx");
                        }
                    }
                    else if(W == 0b0)
                    {
                        if(Reg == 0b001)
                        {
                            strcat(Instructions, "cl");
                        }
                    }
                }
            }
            else if(Mod == 0b01)
            {
                if(D == 0b0)
                {
                    if(RM == 0b110)
                    {
                        strcat(Instructions, "[bp], ");
                    }

                    if(W == 0b0)
                    {
                        if(Reg == 0b101)
                        {
                            strcat(Instructions, "ch");
                        }
                    }
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
        else if((Opcode >> 1) == 0b1100011)
        {
            W = Bytes[i] & 0b1;
            Mod = (Bytes[i+1] >> 6);
            Reg = (Bytes[i+1] >> 3) & 0b111;
            RM = Bytes[i+1] & 0b111; 

            strcat(Instructions, "mov ");
            if(Mod == 0b00)
            {
                if(Reg == 0b000)
                {
                    if(RM == 0b011)
                    {
                        strcat(Instructions, "[bp + di], ");
                    }
                }
            }
            else if(Mod == 0b10)
            {
                if(Reg == 0b000)
                {
                    if(RM == 0b101)
                    {
                        strcat(Instructions, "[di ");
                        Data16 = (Bytes[i+3] << 8) | Bytes[i+2];
                        char Data16str[20];
                        sprintf(Data16str, "+ %hu], ", Data16);
                        strcat(Instructions, Data16str);
                    }
                }
            }

            if(W == 0b0)
            {
                Data8 = Bytes[i+2];
                char Data8str[4];
                sprintf(Data8str, "byte %u", Data8);
                strcat(Instructions, Data8str);
            }
            else if(W == 0b1)
            {
                Data16 = (Bytes[i+5] << 8) | Bytes[i+4];
                char Data16str[20];
                sprintf(Data16str, "word %hu", Data16);
                strcat(Instructions, Data16str);
            }

            strcat(Instructions, "\n");
        }
        else if((Opcode >> 1) == 0b1010000)
        {
            strcat(Instructions, "mov ax, ");

            Data16 = (Bytes[i+2] << 8) | Bytes[i+1];
            char Data16str[20];
            sprintf(Data16str, "[%hu]", Data16);
            strcat(Instructions, Data16str);

            strcat(Instructions, "\n");
        }
        else if((Opcode >> 1) == 0b1010001)
        {

            Data16 = (Bytes[i+2] << 8) | Bytes[i+1];
            char Data16str[24];
            sprintf(Data16str, "mov [%hu], ", Data16);
            strcat(Instructions, Data16str);
            strcat(Instructions, "ax");
            strcat(Instructions, "\n");
        }
    }

    printf("; %s disassembly:\n", argv[1]);
    printf("bits 16\n");
    printf("%s\n", Instructions);

    return 0;
}
