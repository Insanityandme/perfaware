#include "sim86.h"

#include "sim86_memory.h"
#include "sim86_simulator.h"
#include "sim86_text.h"
#include "sim86_decode.h"

#include "sim86_memory.cpp"
#include "sim86_text.cpp"
#include "sim86_decode.cpp"
#include "sim86_simulator.cpp"

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
    sim_register Registers[14] = {};
    Registers[13].RegName = "ip";
    Registers[13].RegisterValue = 0;
    flags Flags = {};

    segmented_access At = DisAsmStart;

    disasm_context Context = DefaultDisAsmContext();

    u32 InstructionSize = DisAsmByteCount;
    // Run until the value in the IP is greater than the file size loaded
    while(Registers[13].RegisterValue < InstructionSize)
    {
        instruction Instruction = DecodeInstruction(&Context, Memory, &At);
        if(Instruction.Op)
        {
            UpdateContext(&Context, Instruction);
            Registers[13].RegisterValue = At.SegmentOffset;

            SimulateInstruction(Memory, Registers, &Flags, Instruction, &At);

            if(IsPrintable(Instruction))
            {
                PrintSimulatedInstruction(Registers, &Flags, Instruction, stdout);
                printf("\n");
            }

            Registers[13].PreviousRegisterValue = Registers[13].RegisterValue;
        }
        else
        {
            fprintf(stderr, "ERROR: Unrecognized binary in instruction stream.\n");
            break;
        }
    }

    PrintFinalRegisters(Registers, &Flags, ArrayCount(Registers), stdout);
}

int main(int ArgCount, char **Args)
{
    size_t size = sizeof(memory);
    memory *Memory = (memory *)malloc(size);
    
    if(ArgCount > 1)
    {
        b32 ExecFlag = false;
        b32 DumpFlag = false;
        char *FileName;
        u32 BytesRead = 0;

        for(int ArgIndex = 1; ArgIndex < ArgCount; ++ArgIndex)
        {
            if(strcmp(Args[ArgIndex], "-exec") == 0)
            {
                FileName = Args[++ArgIndex];
                ExecFlag = true;
            }
            else if(strcmp(Args[ArgIndex], "-dump") == 0)
            {
                DumpFlag = true;
            }
        }

        if(FileName != 0)
        {
            BytesRead = LoadMemoryFromFile(FileName, Memory, 0);
        }

        if(ExecFlag == true)
        {
            printf("--- %s execution ---\n", FileName);

            Simulate8086(Memory, BytesRead, {});
        }

        if(DumpFlag == true && ExecFlag == true)
        {
            FILE *file = fopen("sim86_memory_0.data", "wb");
            if(!file)
            {
                perror("Failed to open file");
                free(Memory);
                return 1;
            }

            size_t written = fwrite(Memory, 1, size, file);

            if(written != size)
            {
                perror("Failed to write memory to file");
                free(Memory);
                fclose(file);
                return 1;
            }

            fclose(file);
            free(Memory);

            return 0;
        }

        if(ExecFlag == 0 && DumpFlag == 0)
        {
            printf("; %s disassembly:\n", FileName);
            printf("bits 16\n");

            DisAsm8086(Memory, BytesRead, {});
        }
    }
    else
    {
        fprintf(stderr, "USAGE: %s [8086 machine code file] ...\n", Args[0]);
        fprintf(stderr, "USAGE: %s -exec [8086 machine code file] ...\n", Args[0]);
        fprintf(stderr, "USAGE: %s -dump -exec [8086 machine code file] ...\n", Args[0]);
    }
    
    
    return 0;
}
