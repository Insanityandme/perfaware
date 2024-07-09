#include <stdio.h>
#include <string.h>
#include <stdint.h>

char Instructions[3000];
static int ByteIndex;

uint8_t Count;
uint8_t Opcode;
uint8_t SecondByte;
uint8_t ThirdByte;
uint8_t FourthByte;
uint8_t FifthByte;

uint8_t D;
uint8_t S;
uint8_t W;
uint8_t Mod;
uint8_t Reg;
uint8_t RM;

uint8_t Data8;
int8_t Data8sig;

uint16_t Data16;
int16_t Data16sig;

char Data8str[20];

void 
printBits(unsigned char Byte) 
{
    // Iterate through each bit position (from MSB to LSB)
    for (int i = 7; i >= 0; --i) 
    {
        // Check if the i-th bit is set (1) or not (0)
        if (Byte & (1 << i))
            printf("1");
        else
            printf("0");
    }
    printf("\n");
}

void
data8ToStr(uint8_t D, uint8_t Data8) 
{
    if(D == 0b1)
    {
        char Data8str[20];
        sprintf(Data8str, "+ %hu]", Data8);
        strcat(Instructions, Data8str);
    }
    else if(D == 0b0)
    {
        char Data8str[20];
        sprintf(Data8str, "+ %hu], ", Data8);
        strcat(Instructions, Data8str);
    }
}

// Function to get the size of a file
long
getFileSize(const char *filename) 
{
    FILE *file = fopen(filename, "rb"); // Open file in binary mode
    long fileSize = -1;

    if (file != NULL) {
        fseek(file, 0, SEEK_END);   // Seek to the end of the file
        fileSize = ftell(file);     // Get the current file pointer (which is the size)
        fclose(file);               // Close the file
    }

    return fileSize;
}

void 
effectiveAddressCalculation(uint8_t D, uint8_t Mod, uint8_t RM, 
                                 uint8_t ThirdByte,
                                 uint8_t FourthByte,
                                 unsigned char *Bytes)
{
    if(Mod == 0b00)
    {
        if(D == 0b0)
        {
            switch(RM)
            {
                case 0b000:
                    strcat(Instructions, "[bx+si], ");
                    break;
                case 0b001:
                    strcat(Instructions, "[bx+di], ");
                    break;
                case 0b010:
                    strcat(Instructions, "[bp+si], ");
                    break;
                case 0b011:
                    strcat(Instructions, "[bp+di], ");
                    break;
                case 0b100:
                    strcat(Instructions, "[si], ");
                    break;
                case 0b101:
                    strcat(Instructions, "[di], ");
                    break;
                case 0b110:
                    ThirdByte = Bytes[++ByteIndex];
                    FourthByte = Bytes[++ByteIndex];
                    // printBits(ThirdByte);
                    // printBits(FourthByte);

                    Data16 = (FourthByte << 8) | ThirdByte;
                    char Data16str[20];
                    sprintf(Data16str, "[%hu]", Data16);
                    strcat(Instructions, Data16str);
                    break;
                case 0b111:
                    strcat(Instructions, "bh, ");
                    break;
            }
        }
        else if(D == 0b1)
        {
            switch(RM)
            {
                case 0b000:
                    strcat(Instructions, "[bx+si]");
                    break;
                case 0b001:
                    strcat(Instructions, "[bx+di]");
                    break;
                case 0b010:
                    strcat(Instructions, "[bp+si]");
                    break;
                case 0b011:
                    strcat(Instructions, "[bp+di]");
                    break;
                case 0b100:
                    strcat(Instructions, "[si]");
                    break;
                case 0b101:
                    strcat(Instructions, "[di]");
                    break;
                case 0b110:
                    ThirdByte = Bytes[++ByteIndex];
                    FourthByte = Bytes[++ByteIndex];
                    // printBits(ThirdByte);
                    // printBits(FourthByte);

                    Data16 = (FourthByte << 8) | ThirdByte;
                    char Data16str[20];
                    sprintf(Data16str, "[%hu]", Data16);
                    strcat(Instructions, Data16str);
                    break;
                case 0b111:
                    strcat(Instructions, "bh");
                    break;
            }
        }
    }
    else if(Mod == 0b01)
    {
        ThirdByte = Bytes[++ByteIndex];
        // printBits(ThirdByte);
        switch(RM)
        {
            case 0b000:
                strcat(Instructions, "[bx + si ");
                break;
            case 0b001:
                strcat(Instructions, "[bx + di ");
                break;
            case 0b010:
                strcat(Instructions, "[bp + si ");
                break;
            case 0b011:
                strcat(Instructions, "[bp + di ");
                break;
            case 0b100:
                strcat(Instructions, "[si ");
                break;
            case 0b101:
                strcat(Instructions, "[di ");
                break;
            case 0b110:
                strcat(Instructions, "[bp ");
                break;
            case 0b111:
                strcat(Instructions, "[bx ");
                break;
        }

        data8ToStr(D, ThirdByte);
    }
    else if(Mod == 0b10)
    {
        ThirdByte = Bytes[++ByteIndex];
        FourthByte = Bytes[++ByteIndex];
        // printBits(ThirdByte);
        // printBits(FourthByte);
    }
}

void regFieldEncoding(uint8_t D, uint8_t W, uint8_t Mod, uint8_t RM)
{
    if(W == 0b1)
    {
        if(D == 0b1)
        {
            switch(Reg)
            {
                case 0b000:
                    strcat(Instructions, "ax, ");
                    break;
                case 0b001:
                    strcat(Instructions, "cx, ");
                    break;
                case 0b010:
                    strcat(Instructions, "dx, ");
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
                case 0b110:
                    strcat(Instructions, "si, ");
                    break;
                case 0b111:
                    strcat(Instructions, "di, ");
                    break;
            }
        }
        else if(D == 0b0)
        {
            if(Mod == 0b11)
            {
                switch(RM)
                {
                    case 0b000:
                        strcat(Instructions, "ax, ");
                        break;
                    case 0b001:
                        strcat(Instructions, "cx");
                        break;
                    case 0b010:
                        strcat(Instructions, "dx");
                        break;
                    case 0b011:
                        strcat(Instructions, "bx");
                        break;
                    case 0b100:
                        strcat(Instructions, "sp");
                        break;
                    case 0b101:
                        strcat(Instructions, "bp");
                        break;
                    case 0b110:
                        strcat(Instructions, "si");
                        break;
                    case 0b111:
                        strcat(Instructions, "di");
                        break;
                }
            }

            switch(Reg)
            {
                case 0b000:
                    strcat(Instructions, "ax");
                    break;
                case 0b001:
                    strcat(Instructions, "cx");
                    break;
                case 0b010:
                    strcat(Instructions, "dx");
                    break;
                case 0b011:
                    strcat(Instructions, "bx");
                    break;
                case 0b100:
                    strcat(Instructions, "sp");
                    break;
                case 0b101:
                    strcat(Instructions, "bp");
                    break;
                case 0b110:
                    strcat(Instructions, "si");
                    break;
                case 0b111:
                    strcat(Instructions, "di");
                    break;
            }
        }
    }
    else if(W == 0b0)
    {
        if(D == 0b1)
        {
            switch(Reg)
            {
                case 0b000:
                    strcat(Instructions, "al, ");
                    break;
                case 0b001:
                    strcat(Instructions, "cl, ");
                    break;
                case 0b010:
                    strcat(Instructions, "dl, ");
                    break;
                case 0b011:
                    strcat(Instructions, "bl, ");
                    break;
                case 0b100:
                    strcat(Instructions, "ah, ");
                    break;
                case 0b101:
                    strcat(Instructions, "ch, ");
                    break;
                case 0b110:
                    strcat(Instructions, "dh, ");
                    break;
                case 0b111:
                    strcat(Instructions, "bh, ");
                    break;
            }
        }
        else if(D == 0b0)
        {
            if(Mod == 0b11)
            {
                switch(RM)
                {
                    case 0b000:
                        strcat(Instructions, "al, ");
                        break;
                    case 0b001:
                        strcat(Instructions, "cl");
                        break;
                    case 0b010:
                        strcat(Instructions, "dl");
                        break;
                    case 0b011:
                        strcat(Instructions, "bl");
                        break;
                    case 0b100:
                        strcat(Instructions, "ah");
                        break;
                    case 0b101:
                        strcat(Instructions, "ch");
                        break;
                    case 0b110:
                        strcat(Instructions, "dh");
                        break;
                    case 0b111:
                        strcat(Instructions, "bh");
                        break;
                }
            }
            switch(Reg)
            {
                case 0b000:
                    strcat(Instructions, "al");
                    break;
                case 0b001:
                    strcat(Instructions, "cl");
                    break;
                case 0b010:
                    strcat(Instructions, "dl");
                    break;
                case 0b011:
                    strcat(Instructions, "bl");
                    break;
                case 0b100:
                    strcat(Instructions, "ah");
                    break;
                case 0b101:
                    strcat(Instructions, "ch");
                    break;
                case 0b110:
                    strcat(Instructions, "dh");
                    break;
                case 0b111:
                    strcat(Instructions, "bh");
                    break;
            }
        }
    }
}

void 
immediateToRegisterMemory(uint8_t S, uint8_t W, 
                          uint8_t Mod, uint8_t RM,
                          unsigned char *Bytes)
{
    if(W == 0b1)
    {
        if(S == 0b1)
        {
            if(Mod == 0b11)
            {
                if(RM == 0b110)
                {
                    ThirdByte = Bytes[++ByteIndex];
                    strcat(Instructions, "si, ");
                    char Data8str[4];
                    sprintf(Data8str, "%u", ThirdByte);
                    strcat(Instructions, Data8str);
                }
                else if(RM == 0b101)
                {
                    ThirdByte = Bytes[++ByteIndex];
                    strcat(Instructions, "bp, ");
                    char Data8str[4];
                    sprintf(Data8str, "%u", ThirdByte);
                    strcat(Instructions, Data8str);
                }
                else if(RM == 0b001)
                {
                    ThirdByte = Bytes[++ByteIndex];
                    strcat(Instructions, "cx, ");
                    char Data8str[4];
                    sprintf(Data8str, "%u", ThirdByte);
                    strcat(Instructions, Data8str);
                }
            }
            else if(Mod == 0b10)
            {
                if(RM == 0b010)
                {
                    strcat(Instructions, "word [bp + si + ");
                    ThirdByte = Bytes[++ByteIndex];
                    FourthByte = Bytes[++ByteIndex];

                    Data16 = (FourthByte << 8) | ThirdByte;
                    char Data16str[20];
                    sprintf(Data16str, "%hu], ", Data16);
                    strcat(Instructions, Data16str);

                    FifthByte = Bytes[++ByteIndex];
                    char Data8str[4];
                    sprintf(Data8str, "%u", FifthByte);
                    strcat(Instructions, Data8str);
                }
            }
            else if(Mod == 0b00)
            {
                if(RM == 0b001)
                {
                    strcat(Instructions, "word [bx + di], ");
                    ThirdByte = Bytes[++ByteIndex];
                    char Data8str[4];
                    sprintf(Data8str, "%u", ThirdByte);
                    strcat(Instructions, Data8str);
                }
                else if(RM == 0b110)
                {
                    strcat(Instructions, "word ");
                    ThirdByte = Bytes[++ByteIndex];
                    FourthByte = Bytes[++ByteIndex];

                    Data16 = (FourthByte << 8) | ThirdByte;
                    char Data16str[20];
                    sprintf(Data16str, "[%hu], ", Data16);
                    strcat(Instructions, Data16str);

                    FifthByte = Bytes[++ByteIndex];
                    char Data8str[4];
                    sprintf(Data8str, "%u", FifthByte);
                    strcat(Instructions, Data8str);
                }
            }
        }
    }
    else if(W == 0b0)
    {
        if(Mod == 0b00)
        {
            if(RM == 0b111)
            {
                ThirdByte = Bytes[++ByteIndex];
                strcat(Instructions, "byte [bx], ");
                char Data8str[4];
                sprintf(Data8str, "%u", ThirdByte);
                strcat(Instructions, Data8str);
            }
        }
    }

}

int main(int argc, char *argv[])
{
    FILE *Fp;
    long FileSize = getFileSize(argv[1]);
    unsigned char Bytes[FileSize];

    Fp = fopen(argv[1], "rb");
    if (Fp == NULL) 
    {
        perror("Error opening file");
        return 1;
    }

    fread(Bytes, sizeof(char), FileSize, Fp);
    fclose(Fp);

    // printf("FileSize: %d\n", FileSize);

    for(ByteIndex = 0; ByteIndex < FileSize; ByteIndex++)
    {
        Opcode = Bytes[ByteIndex]; 
        SecondByte = Bytes[++ByteIndex];
        Reg = (SecondByte >> 3) & 0b111;

        if((Opcode >> 2) == 0b100010)
        {
            strcat(Instructions, "mov ");
            D = (Opcode >> 1) & 0b1; 
            W = Opcode & 0b1;
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
                        Data16 = (Bytes[ByteIndex+3] << 8) | Bytes[ByteIndex+2];
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
                            Data8sig = Bytes[ByteIndex+2];
                            char Data8str[4];
                            strcat(Instructions, "[bx ");
                            sprintf(Data8str, "- %d]", -Data8sig);
                            strcat(Instructions, Data8str);
                        }
                    }
                    else if(RM == 0b001)
                    {
                        Data8sig = Bytes[ByteIndex+2];
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
                        Data16sig = (Bytes[ByteIndex+3] << 8) | Bytes[ByteIndex+2];
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
                    Data8 = Bytes[ByteIndex+2];
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
                    Data16 = (Bytes[ByteIndex+3] << 8) | Bytes[ByteIndex+2];
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
                SecondByte = Bytes[ByteIndex+1];
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
                SecondByte = Bytes[ByteIndex+1];
                Data16 = (Bytes[ByteIndex+2] << 8) | SecondByte;
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
            SecondByte = Bytes[ByteIndex+1];
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
                        Data8 = Bytes[ByteIndex+2];
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
                        Data16 = (Bytes[ByteIndex+3] << 8) | Bytes[ByteIndex+2];
                        char Data16str[20];
                        sprintf(Data16str, "+ %hu], ", Data16);
                        strcat(Instructions, Data16str);
                    }
                    if(W == 0b1)
                    {
                        Data16 = (Bytes[ByteIndex+5] << 8) | Bytes[ByteIndex+4];
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

            SecondByte = Bytes[ByteIndex+1];
            Data16 = (Bytes[ByteIndex+2] << 8) | SecondByte;
            char Data16str[20];
            sprintf(Data16str, "[%hu]", Data16);
            strcat(Instructions, Data16str);

            strcat(Instructions, "\n");
        }
        else if((Opcode >> 1) == 0b1010001)
        {
            SecondByte = Bytes[ByteIndex+1];
            Data16 = (Bytes[ByteIndex+2] << 8) | SecondByte;
            char Data16str[24];
            sprintf(Data16str, "mov [%hu], ", Data16);
            strcat(Instructions, Data16str);
            strcat(Instructions, "ax");
            strcat(Instructions, "\n");
        }
        // Reg/memory with register to either - Add
        else if((Opcode >> 2) == 0b000000)
        {
            // printBits(Opcode);
            // printBits(SecondByte);

            strcat(Instructions, "add ");         
            Count++;

            D = (Opcode >> 1) & 0b1; 
            W = Opcode & 0b1;
            Mod = SecondByte >> 6;
            RM = SecondByte & 0b111; 

            if(D == 0b1)
            {
                regFieldEncoding(D, W, Mod, RM);
                effectiveAddressCalculation(D, Mod, RM, ThirdByte, FourthByte, Bytes);
            }
            else if(D == 0b0)
            {
                effectiveAddressCalculation(D, Mod, RM, ThirdByte, FourthByte, Bytes);
                regFieldEncoding(D, W, Mod, RM);
            }

            printf("\n\n");
            strcat(Instructions, "\n");
        }
        // Immediate from memory - Add
        else if(((Opcode >> 2) == 0b100000 && Reg == 0b000))
        {
            // printBits(Opcode);
            // printBits(SecondByte);
            // printBits(ThirdByte);

            strcat(Instructions, "add ");         
            Count++;

            W = Opcode & 0b1;
            S = (Opcode >> 1) & 0b1; 
            Mod = SecondByte >> 6;
            RM = SecondByte & 0b111; 

            immediateToRegisterMemory(S, W, Mod, RM, Bytes);

            printf("\n\n");
            strcat(Instructions, "\n");
        }
        // Immediate from accumulator - Add
        else if((Opcode >> 1) == 0b0000010)
        {
            // printBits(Opcode);
            // printBits(SecondByte);
            strcat(Instructions, "add ");
            W = Opcode & 0b1;
            if(W == 0b1)
            {
                strcat(Instructions, "ax, ");
                ThirdByte = Bytes[++ByteIndex];
                Data16 = (ThirdByte << 8) | SecondByte;
                char Data16str[20];
                sprintf(Data16str, "%hu", Data16);
                strcat(Instructions, Data16str);
            }
            else if(W == 0b0)
            {
                strcat(Instructions, "al, ");
                char Data8str[4];
                sprintf(Data8str, "%d", SecondByte);
                strcat(Instructions, Data8str);
            }

            Count++;
            printf("\n\n");
            strcat(Instructions, "\n");
        }
        // Immediate to register/memory - Sub
        else if((Opcode >> 2) == 0b001010)
        {
            // printBits(Opcode);
            // printBits(SecondByte);

            strcat(Instructions, "sub ");         
            Count++;

            D = (Opcode >> 1) & 0b1; 
            W = Opcode & 0b1;
            Mod = SecondByte >> 6;
            RM = SecondByte & 0b111; 

            if(D == 0b1)
            {
                regFieldEncoding(D, W, Mod, RM);
                effectiveAddressCalculation(D, Mod, RM, ThirdByte, FourthByte, Bytes);
            }
            else if(D == 0b0)
            {
                effectiveAddressCalculation(D, Mod, RM, ThirdByte, FourthByte, Bytes);
                regFieldEncoding(D, W, Mod, RM);
            }

            printf("\n\n");
            strcat(Instructions, "\n");
        }
        // Immediate from memory - Sub
        else if(((Opcode >> 2) == 0b100000 && Reg == 0b101))
        {
            // printBits(Opcode);
            // printBits(SecondByte);
            // printBits(ThirdByte);

            strcat(Instructions, "sub ");         
            Count++;

            W = Opcode & 0b1;
            S = (Opcode >> 1) & 0b1; 
            Mod = SecondByte >> 6;
            RM = SecondByte & 0b111; 

            immediateToRegisterMemory(S, W, Mod, RM, Bytes);

            printf("\n\n");
            strcat(Instructions, "\n");
        }
        // Immediate from accumulator - Sub
        else if((Opcode >> 1) == 0b0010110)
        {
            // printBits(Opcode);
            // printBits(SecondByte);
            strcat(Instructions, "sub ");
            W = Opcode & 0b1;
            if(W == 0b1)
            {
                strcat(Instructions, "ax, ");
                ThirdByte = Bytes[++ByteIndex];
                // printBits(ThirdByte);
                Data16 = (ThirdByte << 8) | SecondByte;
                char Data16str[20];
                sprintf(Data16str, "%hu", Data16);
                strcat(Instructions, Data16str);
            }
            else if(W == 0b0)
            {
                strcat(Instructions, "al, ");
                char Data8str[4];
                sprintf(Data8str, "%d", SecondByte);
                strcat(Instructions, Data8str);
            }

            Count++;
            printf("\n\n");
            strcat(Instructions, "\n");
        }
        // Immediate to register/memory - Cmp
        else if((Opcode >> 2) == 0b001110)
        {
            // printBits(Opcode);
            // printBits(SecondByte);

            strcat(Instructions, "cmp ");         
            Count++;

            D = (Opcode >> 1) & 0b1; 
            W = Opcode & 0b1;
            Mod = SecondByte >> 6;
            RM = SecondByte & 0b111; 

            if(D == 0b1)
            {
                regFieldEncoding(D, W, Mod, RM);
                effectiveAddressCalculation(D, Mod, RM, ThirdByte, FourthByte, Bytes);
            }
            else if(D == 0b0)
            {
                effectiveAddressCalculation(D, Mod, RM, ThirdByte, FourthByte, Bytes);
                regFieldEncoding(D, W, Mod, RM);
            }

            printf("\n\n");
            strcat(Instructions, "\n");
        }
        // Immediate from memory - Cmp
        else if(((Opcode >> 2) == 0b100000 && Reg == 0b111))
        {
            // printBits(Opcode);
            // printBits(SecondByte);
            // printBits(ThirdByte);

            strcat(Instructions, "cmp ");         
            Count++;

            W = Opcode & 0b1;
            S = (Opcode >> 1) & 0b1; 
            Mod = SecondByte >> 6;
            RM = SecondByte & 0b111; 

            immediateToRegisterMemory(S, W, Mod, RM, Bytes);

            printf("\n\n");
            strcat(Instructions, "\n");
        }
        // Immediate from accumulator - Cmp
        else if((Opcode >> 1) == 0b0011110)
        {
            // printBits(Opcode);
            // printBits(SecondByte);
            strcat(Instructions, "cmp ");
            W = Opcode & 0b1;
            if(W == 0b1)
            {
                strcat(Instructions, "ax, ");
                ThirdByte = Bytes[++ByteIndex];
                // printBits(ThirdByte);

                Data16 = (ThirdByte << 8) | SecondByte;
                char Data16str[20];
                sprintf(Data16str, "%hu, ", Data16);
                strcat(Instructions, Data16str);
            }
            else if(W == 0b0)
            {
                strcat(Instructions, "al, ");
                char Data8str[4];
                sprintf(Data8str, "%d", SecondByte);
                strcat(Instructions, Data8str);
            }

            Count++;
            printf("\n\n");
            strcat(Instructions, "\n");
        }

        switch(Opcode)
        {
            case 0b01110100:
                strcat(Instructions, "je label");

                Data8sig = SecondByte;
                char Data8str[4];
                sprintf(Data8str, "; -%d", -Data8sig);
                strcat(Instructions, Data8str);

                strcat(Instructions, "\n");
                break;
            case 0b01111100:
                strcat(Instructions, "jl label ");
                Data8sig = SecondByte;
                sprintf(Data8str, "; -%d", -Data8sig);
                strcat(Instructions, Data8str);
                strcat(Instructions, "\n");
                break;
            case 0b01111110:
                strcat(Instructions, "jle label ");
                Data8sig = SecondByte;
                sprintf(Data8str, "; -%d", -Data8sig);
                strcat(Instructions, Data8str);
                strcat(Instructions, "\n");
                break;
            case 0b01110010:
                strcat(Instructions, "jb label ");
                Data8sig = SecondByte;
                sprintf(Data8str, "; -%d", -Data8sig);
                strcat(Instructions, Data8str);
                strcat(Instructions, "\n");
                break;
            case 0b01110110:
                strcat(Instructions, "jbe label ");
                Data8sig = SecondByte;
                sprintf(Data8str, "; -%d", -Data8sig);
                strcat(Instructions, Data8str);
                strcat(Instructions, "\n");
                break;
            case 0b01111010:
                strcat(Instructions, "jp label ");
                Data8sig = SecondByte;
                sprintf(Data8str, "; -%d", -Data8sig);
                strcat(Instructions, Data8str);
                strcat(Instructions, "\n");
                break;
            case 0b01110000:
                strcat(Instructions, "jo label ");
                Data8sig = SecondByte;
                sprintf(Data8str, "; -%d", -Data8sig);
                strcat(Instructions, Data8str);
                strcat(Instructions, "\n");
                break;
            case 0b01111000:
                strcat(Instructions, "js label ");
                Data8sig = SecondByte;
                sprintf(Data8str, "; -%d", -Data8sig);
                strcat(Instructions, Data8str);
                strcat(Instructions, "\n");
                break;
            case 0b01110101:
                strcat(Instructions, "jne label ");
                Data8sig = SecondByte;
                sprintf(Data8str, "; %d", -Data8sig);
                strcat(Instructions, Data8str);
                strcat(Instructions, "\n");
                break;
            case 0b01111101:
                strcat(Instructions, "jnl label ");
                Data8sig = SecondByte;
                sprintf(Data8str, "; -%d", -Data8sig);
                strcat(Instructions, Data8str);
                strcat(Instructions, "\n");
                break;
            case 0b01111111:
                strcat(Instructions, "jg label ");
                Data8sig = SecondByte;
                sprintf(Data8str, "; -%d", -Data8sig);
                strcat(Instructions, Data8str);
                strcat(Instructions, "\n");
                break;
            case 0b01110011:
                strcat(Instructions, "jnb label ");
                Data8sig = SecondByte;
                sprintf(Data8str, "; -%d", -Data8sig);
                strcat(Instructions, Data8str);
                strcat(Instructions, "\n");
                break;
            case 0b01110111:
                strcat(Instructions, "ja label ");
                Data8sig = SecondByte;
                sprintf(Data8str, "; -%d", -Data8sig);
                strcat(Instructions, Data8str);
                strcat(Instructions, "\n");
                break;
            case 0b01111011:
                strcat(Instructions, "jnp label ");
                Data8sig = SecondByte;
                sprintf(Data8str, "; -%d", -Data8sig);
                strcat(Instructions, Data8str);
                strcat(Instructions, "\n");
                break;
            case 0b01110001:
                strcat(Instructions, "jno label ");
                Data8sig = SecondByte;
                sprintf(Data8str, "; -%d", -Data8sig);
                strcat(Instructions, Data8str);
                strcat(Instructions, "\n");
                break;
            case 0b01111001:
                strcat(Instructions, "jns label ");
                Data8sig = SecondByte;
                sprintf(Data8str, "; -%d", -Data8sig);
                strcat(Instructions, Data8str);
                strcat(Instructions, "\n");
                break;
            case 0b11100010:
                strcat(Instructions, "loop label ");
                Data8sig = SecondByte;
                sprintf(Data8str, "; -%d", -Data8sig);
                strcat(Instructions, Data8str);
                strcat(Instructions, "\n");
                break;
            case 0b11100001:
                strcat(Instructions, "loopz label ");
                Data8sig = SecondByte;
                sprintf(Data8str, "; -%d", -Data8sig);
                strcat(Instructions, Data8str);
                strcat(Instructions, "\n");
                break;
            case 0b11100000:
                strcat(Instructions, "loopnz label ");
                Data8sig = SecondByte;
                sprintf(Data8str, "; -%d", -Data8sig);
                strcat(Instructions, Data8str);
                strcat(Instructions, "\n");
                break;
            case 0b11100011:
                strcat(Instructions, "jcxz label ");
                Data8sig = SecondByte;
                sprintf(Data8str, "; -%d", -Data8sig);
                strcat(Instructions, Data8str);
                strcat(Instructions, "\n");
                break;

        }
    }

    // printf("Count: %d\n", Count);
    printf("; %s disassembly:\n", argv[1]);
    printf("bits 16\n");
    printf("%s\n", Instructions);

    return 0;
}
