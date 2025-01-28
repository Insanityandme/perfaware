#include <stdio.h>
#include <stdint.h>
#include <windows.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

#include "listing_0117_virtual_address.cpp"

static void PrintBinaryBits(u64 Value, u32 FirstBit, u32 BitCount)
{
    for(u32 BitIndex = 0; BitIndex < BitCount; ++BitIndex)
    {
        u64 Bit = (Value >> ((BitCount - 1 - BitIndex) + FirstBit)) & 1;
        printf("%c", Bit ? '1' : '0');
    }
}

int main(void)
{
    for(int PointerIndex = 0; PointerIndex < 16; ++PointerIndex)
    {
        void *Pointer = (u8 *)VirtualAlloc(0, 1024*1024, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

        u64 Address = (u64)Pointer;
        PrintBinaryBits(Address, 48, 16);
        printf("|");
        PrintBinaryBits(Address, 39, 9);
        printf("|");
        PrintBinaryBits(Address, 30, 9);
        printf("|");
        PrintBinaryBits(Address, 21, 9);
        printf("|");
        PrintBinaryBits(Address, 12, 9);
        printf("|");
        PrintBinaryBits(Address, 0, 12);
        printf("\n");

        PrintAsLine(" 4k paging: ", DecomposePointer4K(Pointer));
        PrintAsLine(" 2mb paging: ", DecomposePointer2MB(Pointer));
        PrintAsLine(" 1gb paging: ", DecomposePointer1GB(Pointer));

        printf("\n");
    }
}
