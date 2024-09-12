#include "sim86.h"

/* NOTE(casey): _CRT_SECURE_NO_WARNINGS is here because otherwise we cannot
   call fopen(). If we replace fopen() with fopen_s() to avoid the warning,
   then the code doesn't compile on Linux anymore, since fopen_s() does not
   exist there.
   
   What exactly the CRT maintainers were thinking when they made this choice,
   I have no idea.
*/
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

// TODO(filip): Change so this is not used anywhere else than in this file
enum sim_flags
{
    SimFlag_Exec = 0x1,
    SimFlag_Dump = 0x2,
    SimFlag_ShowClocks = 0x4,
    SimFlag_ExplainClocks = 0x8,
    SimFlag_StopOnRet = 0x10,
};

#include "sim86_instruction.h"
#include "sim86_memory.h"
#include "sim86_execute.h"
#include "sim86_text.h"
#include "sim86_decode.h"
#include "sim86_cycles.h"

#include "sim86_memory.cpp"
#include "sim86_text.cpp"
#include "sim86_cycles.cpp"
#include "sim86_decode.cpp"
#include "sim86_execute.cpp"

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

static b32 IsRet(operation_type Op)
{
    // b32 Result = ((Op == Op_ret) ||
    //               (Op == Op_retf));
    return Op == Op_ret;
}

static void Simulate8086(u32 SimFlags, memory *Memory, u32 DisAsmByteCount, segmented_access DisAsmStart)
{
    u32 TotalClocks = 0;

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
            SimulateInstruction(Memory, Registers, &Flags, Instruction, &At);

            if((SimFlags & SimFlag_StopOnRet) &&
               IsRet(Instruction.Op))
            {
                fprintf(stdout, "STOPONRET: Return encountered at address %u.\n", Instruction.Address);
                break;
            }

            UpdateContext(&Context, Instruction);
            Registers[13].RegisterValue = At.SegmentOffset;

            if(IsPrintable(Instruction))
            {
                PrintInstruction(Instruction, stdout);
                printf(" ; ");

                if(SimFlags & SimFlag_ShowClocks || SimFlags & SimFlag_ExplainClocks)
                {
                    clocks Clocks = CalculateClocks(Instruction);
                    if(SimFlags & SimFlag_ShowClocks)
                    {
                        PrintClocks(Instruction, Clocks.Clocks, &TotalClocks, Clocks.EffectiveAddressTime);
                    }
                    else if(SimFlags & SimFlag_ExplainClocks)
                    {
                        PrintExplainClocks(Instruction, Clocks.Clocks, Clocks.EffectiveAddressTime, Clocks.Transfers, &TotalClocks);
                    }
                }

                PrintSimulatedInstruction(Registers, &Flags, Instruction, stdout);
            }
            printf("\n");

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
    u32 SimFlags = 0;
    size_t size = sizeof(memory);
    memory *Memory = (memory *)malloc(size);
    
    if(ArgCount > 1)
    {
        char *FileName;
        u32 BytesRead = 0;

        for(int ArgIndex = 1; ArgIndex < ArgCount; ++ArgIndex)
        {
            if(strcmp(Args[ArgIndex], "-exec") == 0)
            {
                FileName = Args[++ArgIndex];
                SimFlags |= SimFlag_Exec;
            }
            else if(strcmp(Args[ArgIndex], "-stoponret") == 0)
            {
                SimFlags |= SimFlag_StopOnRet;
            }
            else if(strcmp(Args[ArgIndex], "-dump") == 0)
            {
                SimFlags |= SimFlag_Dump;
            }
            else if(strcmp(Args[ArgIndex], "-showclocks") == 0)
            {
                SimFlags |= SimFlag_ShowClocks;
            }
            else if(strcmp(Args[ArgIndex], "-explainclocks") == 0)
            {
                SimFlags |= SimFlag_ExplainClocks;
            }
            else
            {
                FileName = Args[ArgIndex];
            }
        }

        if(FileName != 0)
        {
            BytesRead = LoadMemoryFromFile(FileName, Memory, 0);
        }

        if(SimFlags & SimFlag_Exec)
        {
            printf("--- %s execution ---\n", FileName);

            Simulate8086(SimFlags, Memory, BytesRead, {});
        }

        if((SimFlags & SimFlag_ShowClocks) && !(SimFlags & SimFlag_Exec) && 
          !(SimFlags & SimFlag_Dump) || SimFlags & SimFlag_ExplainClocks)
        {
            printf("\nWARNING: Clocks reported by this utility are stricly from the 8086 manual.");
            printf("\nThey will be inaccurate, both because the manual clocks are estimates, and because");
            printf("\nsome of the entries in the manual look highly suspicious and are probably typos.");
            printf("\n\n");
            printf("--- %s execution ---\n", FileName);

            Simulate8086(SimFlags, Memory, BytesRead, {});
        }

        if(SimFlags & SimFlag_Dump && SimFlags & SimFlag_Exec)
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

        if(!(SimFlags & SimFlag_Exec) && !(SimFlags & SimFlag_Dump) && 
           !(SimFlags & SimFlag_ShowClocks) && !(SimFlags & SimFlag_ExplainClocks))
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
        fprintf(stderr, "USAGE: %s -showclocks [8086 machine code file] ...\n", Args[0]);
        fprintf(stderr, "USAGE: %s -explainclocks [8086 machine code file] ...\n", Args[0]);
    }
    
    
    return 0;
}
