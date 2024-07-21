#include "sim86.h"

#include "sim86_simulator.h"
#include "sim86_memory.h"
#include "sim86_text.h"
#include "sim86_decode.h"

#include "sim86_memory.cpp"
#include "sim86_text.cpp"
#include "sim86_decode.cpp"

static void DisAsm8086(memory *Memory, u32 DisAsmByteCount, segmented_access DisAsmStart)
{
    segmented_access At = DisAsmStart;

    disasm_context Context = DefaultDisAsmContext();

    u32 Count = DisAsmByteCount;
    while(Count)
    {
        instruction Instruction = DecodeInstruction(&Context, Memory, &At);
        if(Instruction.Op)
        {
            if(Count >= Instruction.Size)
            {
                Count -= Instruction.Size;
            }
            else
            {
                fprintf(stderr, "ERROR: Instruction extends outside disassembly region\n");
                break;
            }

            UpdateContext(&Context, Instruction);
            if(IsPrintable(Instruction))
            {
                PrintInstruction(Instruction, stdout);
                printf("\n");
            }
        }
        else
        {
            fprintf(stderr, "ERROR: Unrecognized binary in instruction stream.\n");
            break;
        }
    }
}

static void Simulate8086(memory *Memory, u32 DisAsmByteCount, segmented_access DisAsmStart)
{
    sim_register Registers[11] = {
        {0, 0, "ax",}, {0, 0, "bx"}, 
        {0, 0, "cx",}, {0, 0, "dx"},
        {0, 0, "sp",}, {0, 0, "bp"}, 
        {0, 0, "si",}, {0, 0, "di"},
        {0, 0, "es",}, {0, 0, "ss",}, {0, 0, "ds"},

    };

    segmented_access At = DisAsmStart;

    disasm_context Context = DefaultDisAsmContext();

    u32 Count = DisAsmByteCount;
    while(Count)
    {
        instruction Instruction = DecodeInstruction(&Context, Memory, &At);
        if(Instruction.Op)
        {
            if(Count >= Instruction.Size)
            {
                Count -= Instruction.Size;
            }
            else
            {
                fprintf(stderr, "ERROR: Instruction extends outside disassembly region\n");
                break;
            }

            UpdateContext(&Context, Instruction);
            if(IsPrintable(Instruction))
            {
                PrintSimulatedInstruction(Registers, Instruction, stdout);
                printf("\n");
            }
        }
        else
        {
            fprintf(stderr, "ERROR: Unrecognized binary in instruction stream.\n");
            break;
        }
    }

    PrintFinalRegisters(Registers, ArrayCount(Registers));
}

int main(int ArgCount, char **Args)
{
    memory *Memory = (memory *)malloc(sizeof(memory));
    
    if(ArgCount > 1)
    {
        for(int ArgIndex = 1; ArgIndex < ArgCount; ++ArgIndex)
        {
            if(strcmp(Args[ArgIndex], "-exec") == 0)
            {
                char *FileName = Args[++ArgIndex];
                u32 BytesRead = LoadMemoryFromFile(FileName, Memory, 0);
                
                printf("--- %s execution ---\n", FileName);

                Simulate8086(Memory, BytesRead, {});
            }
            else
            {
                char *FileName = Args[ArgIndex];
                u32 BytesRead = LoadMemoryFromFile(FileName, Memory, 0);

                printf("; %s disassembly:\n", FileName);
                printf("bits 16\n");

                DisAsm8086(Memory, BytesRead, {});
            }
        }
    }
    else
    {
        fprintf(stderr, "USAGE: %s [8086 machine code file] ...\n", Args[0]);
        fprintf(stderr, "USAGE: %s -exec [8086 machine code file] ...\n", Args[0]);
    }
    
    return 0;
}
