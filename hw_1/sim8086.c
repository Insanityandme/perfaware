#include <stdio.h>

int main(int argc, char *argv[])
{
    FILE *fp;
    unsigned char c;

    fp = fopen(argv[1], "rb");
    if (fp == NULL) 
    {
        perror("Error opening file");
        return 1;
    }
    printf("; %s disassembly:\n", argv[1]);

    while(fread(&c, sizeof(char), 1, fp) > 0)
    {
        printf("%d\n", c);
    }

    // Close the file
    fclose(fp);

    return 0;
}
