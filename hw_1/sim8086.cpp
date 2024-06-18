#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[])
{
    FILE *fp;
    unsigned char bytes[2];
    char instructions[5];

    fp = fopen(argv[1], "rb");
    if (fp == NULL) 
    {
        perror("Error opening file");
        return 1;
    }

    printf("; %s disassembly:\n", argv[1]);
    printf("bits 16\n");

    while(fread(bytes, sizeof(bytes), 1, fp) > 0)
    {
        unsigned char operand = 0;

        int bitIndex = 0;
        for (int i = 0;
             i < 2; 
             i++) 
        {
            for (int j = 7; 
                 j >= 0; 
                 j--) 
            {
                // Extract each bit from the byte, starting from the most significant bit
                unsigned char bit = (bytes[i] >> j) & 1;
                if (i == 0)
                {
                    operand |= bit << (5 - bitIndex);
                }
                bitIndex++;
                printf("%u", bit); // Print the bit (for demonstration purposes)
            }

            if (operand == 34 && i == 0) 
            {
                strcat(instructions, "mov ");
            }

            printf("\n");
        }
        printf("Operand in number: %d\n", operand);
    }

    printf("%s", instructions);

    fclose(fp);

    return 0;
}
