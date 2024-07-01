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

// Function to get the size of a file
long getFileSize(const char *filename) {
    FILE *file = fopen(filename, "rb"); // Open file in binary mode
    long fileSize = -1;

    if (file != NULL) {
        fseek(file, 0, SEEK_END);   // Seek to the end of the file
        fileSize = ftell(file);     // Get the current file pointer (which is the size)
        fclose(file);               // Close the file
    }

    return fileSize;
}

int main(int argc, char *argv[])
{
    FILE *Fp;
    long fileSize = getFileSize(argv[1]);
    unsigned char Bytes[fileSize];
    char Instructions[3000] = {0};

    Fp = fopen(argv[1], "rb");
    if (Fp == NULL) 
    {
        perror("Error opening file");
        return 1;
    }

    fread(Bytes, sizeof(char), fileSize, Fp);
    fclose(Fp);

    uint8_t Count = 0;
    uint8_t Opcode = 0;
    uint8_t SecondByte = 0;
    uint8_t ThirdByte = 0;
    uint8_t FourthByte = 0;
    uint8_t D = 0;
    uint8_t W = 0;
    uint8_t Mod = 0;
    uint8_t Reg = 0;
    uint8_t RM = 0;
    uint8_t Data8 = 0;
    int8_t Data8sig = 0;
    uint16_t Data16 = 0;
    int16_t Data16sig = 0;

    for(int i = 0; i < fileSize; i++)
    {
        Opcode = Bytes[i]; 
        SecondByte = Bytes[++i];
        Reg = (SecondByte >> 3) & 0b111;

        if((Opcode >> 2) == 0b100010)
        {
            strcat(Instructions, "mov ");
            D = (Opcode >> 1) & 0b1; 
            W = Opcode & 0b1;
            SecondByte = Bytes[i+1];
            Mod = (SecondByte >> 6);
            Reg = (SecondByte >> 3) & 0b111;
            RM = SecondByte & 0b111; 

            if(W == 0b1)
            {
                if(D == 0b0)
                {
                    if(Mod == 0b11)
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
                }

                switch(Reg)
                {
                    case 0b111:
                        strcat(Instructions, "di");
                        break;
                    case 0b011:
                        if(D == 0b0)
                        {
                            strcat(Instructions, "bx");
                        }
                        break;
                    case 0b100:
                        strcat(Instructions, "ah");
                        break;
                    case 0b000:
                        if(D == 0b1)
                        {
                            strcat(Instructions, "ax, ");
                        }
                        else if(D == 0b0)
                        {
                            strcat(Instructions, "ax");
                        }
                        break;
                    case 0b110:
                        strcat(Instructions, "si");
                        break;
                }

                if(Mod == 0b00)
                {
                    switch(Reg)
                    {
                        case 0b011:
                            strcat(Instructions, "bx, ");
                            break;
                        case 0b000:
                            strcat(Instructions, "al, ");
                            break;
                        case 0b101:
                            if(D == 0b1)
                            {
                                strcat(Instructions, "bp, ");
                            }
                    }

                    // Direct address
                    if(RM == 0b110)
                    {
                        Data16 = (Bytes[i+3] << 8) | Bytes[i+2];
                        char Data16str[20];
                        sprintf(Data16str, "[%hu]", Data16);
                        strcat(Instructions, Data16str);
                    }
                }
                else if(Mod == 0b01)
                {
                    if(Reg == 0b010)
                    {
                        strcat(Instructions, "dx, ");
                        if(RM == 0b110)
                        {
                            strcat(Instructions, "[bp]");
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
                    else if(RM == 0b001)
                    {
                        Data8sig = Bytes[i+2];
                        char Data8str[4];
                        strcat(Instructions, "[bx + di ");
                        sprintf(Data8str, "- %d]", -Data8sig);
                        strcat(Instructions, Data8str);
                    }
                    else if(RM == 0b110)
                    {
                        if(D == 0b0)
                        {
                            strcat(Instructions, "[bp], ");
                        }
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

                    if(Reg == 0b001)
                    {
                        strcat(Instructions, "cx");
                    }
                }
                else if(Mod == 0b00)
                {
                    if(RM == 0b001)
                    {
                        if(D == 0b0)
                        {
                            strcat(Instructions, "[bx + di], ");
                        }
                    }
                    else if(RM == 0b011)
                    {
                        strcat(Instructions, "[bp + di]");
                    }
                }

                if(D == 0b0)
                {
                    if(Mod == 0b00)
                    {
                        switch(Reg)
                        {
                            case 0b001:
                                strcat(Instructions, "cx");
                                break;
                        }
                    }
                    else if(Mod == 0b01)
                    {
                        if(RM == 0b110)
                        {
                            strcat(Instructions, "[bp], ");
                        }
                    }

                }

            }
            else if(W == 0b0)
            {
                if(Mod == 0b11)
                {
                    if(RM == 0b110)
                    {
                        strcat(Instructions, "dh, ");
                    }
                }
                else if(Mod == 0b00)
                {
                    if(RM == 0b010)
                    {
                        strcat(Instructions, "[bp + si], ");
                    }
                }
                if(D == 0b0)
                {
                    if(Mod == 0b01)
                    {
                        if(RM == 0b110)
                        {
                            strcat(Instructions, "[bp], ");
                        }
                    }
                }

                switch(Reg)
                {
                    case 0b100:
                        if(D == 0b0)
                        {
                            strcat(Instructions, "ah");
                        }
                        else if(D == 0b1)
                        {
                            strcat(Instructions, "ah, ");
                        }
                        break;
                    case 0b101:
                        if(D == 0b1)
                        {
                            strcat(Instructions, "ch");
                        }
                        break;
                    case 0b001:
                        if(D == 0b0)
                        {
                            strcat(Instructions, "cl");
                        }
                        break;
                    case 0b000:
                        if(D == 0b1)
                        {
                            strcat(Instructions, "al, ");
                        }
                        else if(D == 0b0)
                        {
                            strcat(Instructions, "al");
                        }
                        break;
                }

                if(Mod == 0b01)
                {
                    if(RM == 0b101)
                    {
                        strcat(Instructions, "ch, ");
                    }
                }
                else if(Mod == 0b00)
                {
                    if(RM == 0b000)
                    {
                        if(D == 0b1)
                        {
                            strcat(Instructions, "[bx + si]");
                        }
                    }
                }

                if(Reg == 0b101)
                {
                    strcat(Instructions, "ch");
                }
            }

            if(Mod == 0b01)
            {
                if(RM == 0b000)
                {
                    Data8 = Bytes[i+2];
                    char Data8str[4];
                    strcat(Instructions, "[bx + si + ");
                    sprintf(Data8str, "%u]", Data8);
                    strcat(Instructions, Data8str);
                }
            }
            else if(Mod == 0b10)
            {
                if(RM == 0b000)
                {
                    Data16 = (Bytes[i+3] << 8) | Bytes[i+2];
                    char Data16str[20];
                    strcat(Instructions, "[bx + si + ");
                    sprintf(Data16str, "%hu]", Data16);
                    strcat(Instructions, Data16str);
                }
            }

            strcat(Instructions, "\n");
        }
        else if((Opcode >> 4) == 0b1011)
        {
            strcat(Instructions, "mov ");

            W = (Opcode >> 3) & 0x01;
            Reg = Opcode & 0x07;
            if(W == 0b0)
            {
                SecondByte = Bytes[i+1];
                Data8 = SecondByte;
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
                SecondByte = Bytes[i+1];
                Data16 = (Bytes[i+2] << 8) | SecondByte;
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
            W = Opcode & 0b1;
            SecondByte = Bytes[i+1];
            Mod = (SecondByte >> 6);
            Reg = (SecondByte >> 3) & 0b111;
            RM = SecondByte & 0b111; 

            strcat(Instructions, "mov ");
            if(Mod == 0b00)
            {
                if(Reg == 0b000)
                {
                    if(RM == 0b011)
                    {
                        strcat(Instructions, "[bp + di], ");
                    }
                    if(W == 0b0)
                    {
                        Data8 = Bytes[i+2];
                        char Data8str[4];
                        sprintf(Data8str, "byte %u", Data8);
                        strcat(Instructions, Data8str);
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
                    if(W == 0b1)
                    {
                        Data16 = (Bytes[i+5] << 8) | Bytes[i+4];
                        char Data16str[20];
                        sprintf(Data16str, "word %hu", Data16);
                        strcat(Instructions, Data16str);
                    }
                }
            }


            strcat(Instructions, "\n");
        }
        else if((Opcode >> 1) == 0b1010000)
        {
            strcat(Instructions, "mov ax, ");

            SecondByte = Bytes[i+1];
            Data16 = (Bytes[i+2] << 8) | SecondByte;
            char Data16str[20];
            sprintf(Data16str, "[%hu]", Data16);
            strcat(Instructions, Data16str);

            strcat(Instructions, "\n");
        }
        else if((Opcode >> 1) == 0b1010001)
        {
            SecondByte = Bytes[i+1];
            Data16 = (Bytes[i+2] << 8) | SecondByte;
            char Data16str[24];
            sprintf(Data16str, "mov [%hu], ", Data16);
            strcat(Instructions, Data16str);
            strcat(Instructions, "ax");
            strcat(Instructions, "\n");
        }
        else if((Opcode >> 2) == 0b000000 ||
                ((Opcode >> 2) == 0b100000 && Reg == 0b000) ||
                (Opcode >> 2) == 0b000010)
        {
            printBits(Opcode);
            printBits(SecondByte);

            strcat(Instructions, "add ");         
            Count++;

            W = Opcode & 0b1;
            D = (Opcode >> 1) & 0b1; 
            Mod = SecondByte >> 6;
            RM = SecondByte & 0b111; 

            if(W == 0b1)
            {
                if(Reg == 0b011)
                {
                    strcat(Instructions, "bx, ");
                }
            }
            else if(W == 0b0)
            {
            }

            if(Mod == 0b00)
            {
                RM = SecondByte & 0b111;
                if(RM == 0b110)
                {
                    ThirdByte = Bytes[++i];
                    FourthByte = Bytes[++i];
                    printBits(ThirdByte);
                    printBits(FourthByte);
                }
                else if(RM == 0b000)
                {
                    strcat(Instructions, "[bx+si]");
                }
            }
            else if(Mod == 0b01)
            {
                ThirdByte = Bytes[++i];
                printf("hi");
                printBits(ThirdByte);
                if(RM == 0b110)
                {
                    strcat(Instructions, "[bp]");
                }
            }
            else if(Mod == 0b10)
            {
                ThirdByte = Bytes[++i];
                FourthByte = Bytes[++i];
                printBits(ThirdByte);
                printBits(FourthByte);
            }
            else if(Mod == 0b11)
            {
            }

            printf("\n\n");
            strcat(Instructions, "\n");
        }
        else if((Opcode >> 2) == 0b001010)
        {
            SecondByte = Bytes[++i];
            Mod = SecondByte >> 6;
            Reg = (SecondByte >> 3) & 0b111;

            // Immediate to register/memory
            if(Reg == 0b000)
            {
            }

            printBits(Opcode);
            printBits(SecondByte);

            strcat(Instructions, "sub ");         
            Count++;
            if(Mod == 0b00)
            {
                RM = SecondByte & 0b111;
                if(RM == 0b110)
                {
                    ThirdByte = Bytes[++i];
                    FourthByte = Bytes[++i];
                    printBits(ThirdByte);
                    printBits(FourthByte);
                }
            }
            else if(Mod == 0b01)
            {
                ThirdByte = Bytes[++i];
                printBits(ThirdByte);
            }
            else if(Mod == 0b10)
            {
                ThirdByte = Bytes[++i];
                FourthByte = Bytes[++i];
                printBits(ThirdByte);
                printBits(FourthByte);
            }
            else if(Mod == 0b11)
            {
            }

            printf("\n\n");
            strcat(Instructions, "\n");
        }
        else if((Opcode >> 2) == 0b001110)
        {
            SecondByte = Bytes[++i];
            Mod = SecondByte >> 6;

            printBits(Opcode);
            printBits(SecondByte);

            strcat(Instructions, "cmp ");         
            Count++;
            if(Mod == 0b00)
            {
                RM = SecondByte & 0b111;
                if(RM == 0b110)
                {
                    ThirdByte = Bytes[++i];
                    FourthByte = Bytes[++i];
                    printBits(ThirdByte);
                    printBits(FourthByte);
                }
            }
            else if(Mod == 0b01)
            {
                ThirdByte = Bytes[++i];
                printBits(ThirdByte);
            }
            else if(Mod == 0b10)
            {
                ThirdByte = Bytes[++i];
                FourthByte = Bytes[++i];
                printBits(ThirdByte);
                printBits(FourthByte);
            }
            else if(Mod == 0b11)
            {
            }

            printf("\n\n");
            strcat(Instructions, "\n");
        }
        // Immediate from register/memory for add/sub/cmp
        // else if((Opcode >> 2) == 0b100000)
        // {
        //     SecondByte = Bytes[++i];
        //     Reg = (SecondByte >> 3) & 0b111;
        //     if(Reg == 0b000)
        //     {
        //         Count++;
        //         strcat(Instructions, "add ");
        //     }
        //     else if(Reg == 0b101)
        //     {
        //         strcat(Instructions, "sub ");
        //     }
        //     else if(Reg == 0b111)
        //     {
        //         strcat(Instructions, "cmp ");
        //     }

        //     strcat(Instructions, "\n");
        // }
    }

    printf("Count: %d\n", Count);
    printf("; %s disassembly:\n", argv[1]);
    printf("bits 16\n");
    printf("%s\n", Instructions);

    return 0;
}
