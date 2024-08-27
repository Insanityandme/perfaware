char const *OpcodeMnemonics[] = 
{
    "",

#define INST(Mnemonic, ...) #Mnemonic,
#define INSTALT(...)
#include "sim86_instruction_table.inl"
};

static char const *GetMnemonic(operation_type Op)
{
    char const *Result = OpcodeMnemonics[Op];
    return Result;
}

static char const *GetRegName(register_access Reg)
{
    char const *Names[][3] =
    {
        {"", "", ""},
        {"al", "ah", "ax"},
        {"bl", "bh", "bx"},
        {"cl", "ch", "cx"},
        {"dl", "dh", "dx"},
        {"sp", "sp", "sp"},
        {"bp", "bp", "bp"},
        {"si", "si", "si"},
        {"di", "di", "di"},
        {"es", "es", "es"},
        {"cs", "cs", "cs"},
        {"ss", "ss", "ss"},
        {"ds", "ds", "ds"},
        {"ip", "ip", "ip"},
        {"flags", "flags", "flags"}
    };
    static_assert(ArrayCount(Names) == Register_count, "Text table mismatch for register_index");

    char const *Result = Names[Reg.Index][(Reg.Count == 2) ? 2 : Reg.Offset&1];
    return Result;
}

static char const *GetEffectiveAddressExpression(effective_address_expression Address)
{
    char const *RMBase[] =
    {
        "",
        "bx+si",
        "bx+di",
        "bp+si",
        "bp+di",
        "si",
        "di",
        "bp",
        "bx",
    };
    static_assert(ArrayCount(RMBase) == EffectiveAddress_count, "Text table mismatch for effective_base_address");
    char const *Result = RMBase[Address.Base];
    return Result;
}

static b32 IsPrintable(instruction Instruction)
{
    b32 Result = !((Instruction.Op == Op_lock) ||
                   (Instruction.Op == Op_rep) ||
                   (Instruction.Op == Op_segment));

    return Result;
}

static void PrintRegisterChange(char const *DestRegName, u8 DestRegIndex, 
                                sim_register *Registers, FILE *Dest)
{
    if(Registers[DestRegIndex].RegisterValue == Registers[DestRegIndex].PreviousRegisterValue)
    {
        fprintf(Dest, " ; ip:0x%x->0x%x ", Registers[13].PreviousRegisterValue, Registers[13].RegisterValue);
    }
    else if(Registers[DestRegIndex].RegisterValue != 0 || Registers[DestRegIndex].RegName != 0)
    {
        fprintf(Dest, " ; %s:0x%x->0x%x ip:0x%x->0x%x ", DestRegName,
                                           Registers[DestRegIndex].PreviousRegisterValue, 
                                           Registers[DestRegIndex].RegisterValue,
                                           Registers[13].PreviousRegisterValue,
                                           Registers[13].RegisterValue);
    }
    else
    {
        fprintf(Dest, " ; ip:0x%x->0x%x ", Registers[13].PreviousRegisterValue, Registers[13].RegisterValue);
    }
}

static void PrintInstruction(instruction Instruction, FILE *Dest)
{
    u32 Flags = Instruction.Flags;
    u32 W = Flags & Inst_Wide;

    if(Flags & Inst_Lock)
    {
        if(Instruction.Op == Op_xchg)
        {
            // NOTE(casey): This is just a stupidity for matching assembler expectations.
            instruction_operand Temp = Instruction.Operands[0];
            Instruction.Operands[0] = Instruction.Operands[1];
            Instruction.Operands[1] = Temp;
        }
        fprintf(Dest, "lock ");
    }

    char const *MnemonicSuffix = "";
    if(Flags & Inst_Rep)
    {
        printf("rep ");
        MnemonicSuffix = W ? "w" : "b";
    }

    fprintf(Dest, "%s%s ", GetMnemonic(Instruction.Op), MnemonicSuffix);

    char const *Seperator = "";
    for(u32 OperandIndex = 0; OperandIndex < ArrayCount(Instruction.Operands); ++OperandIndex)
    {
        instruction_operand Operand = Instruction.Operands[OperandIndex];
        if(Operand.Type != Operand_None)
        {
            fprintf(Dest, "%s", Seperator);
            Seperator = ", ";

            switch(Operand.Type)
            {
                case Operand_None: {} break;

                case Operand_Register:
                {
                    fprintf(Dest, "%s", GetRegName(Operand.Register));
                } break;
                                   
                case Operand_Memory:
                {
                    effective_address_expression Address = Operand.Address;

                    if(Instruction.Operands[0].Type != Operand_Register)
                    {
                        fprintf(Dest, "%s ", W ? "word" : "byte");
                    }

                    if(Flags & Inst_Segment)
                    {
                        printf("%s:", GetRegName({Address.Segment, 0, 2}));
                    }

                    fprintf(Dest, "[%s", GetEffectiveAddressExpression(Address));
                    if(Address.Displacement != 0)
                    {
                        fprintf(Dest, "%+d", Address.Displacement); 
                    }
                    fprintf(Dest, "]");
                } break;

                case Operand_Immediate:
                {
                    fprintf(Dest, "%d", Operand.ImmediateS32);
                } break;

                case Operand_RelativeImmediate:
                {
                    fprintf(Dest, "$%+d", Operand.ImmediateS32);
                } break;
            }
        }
    }
}

static void PrintSimulatedInstruction(sim_register *Registers, flags *RegFlags, 
                                      instruction Instruction, FILE *Dest)
{
    u32 Flags = Instruction.Flags;
    u32 W = Flags & Inst_Wide;

    if(Flags & Inst_Lock)
    {
        if(Instruction.Op == Op_xchg)
        {
            // NOTE(casey): This is just a stupidity for matching assembler expectations.
            instruction_operand Temp = Instruction.Operands[0];
            Instruction.Operands[0] = Instruction.Operands[1];
            Instruction.Operands[1] = Temp;
        }
        fprintf(Dest, "lock ");
    }

    char const *MnemonicSuffix = "";
    if(Flags & Inst_Rep)
    {
        printf("rep ");
        MnemonicSuffix = W ? "w" : "b";
    }

    fprintf(Dest, "%s%s ", GetMnemonic(Instruction.Op), MnemonicSuffix);

    char const *Seperator = "";
    for(u32 OperandIndex = 0; OperandIndex < ArrayCount(Instruction.Operands); ++OperandIndex)
    {
        instruction_operand Operand = Instruction.Operands[OperandIndex];
        if(Operand.Type != Operand_None)
        {
            fprintf(Dest, "%s", Seperator);
            Seperator = ", ";

            switch(Operand.Type)
            {
                case Operand_None: {} break;

                case Operand_Register:
                {
                    fprintf(Dest, "%s", GetRegName(Operand.Register));
                } break;
                                   
                case Operand_Memory:
                {
                    effective_address_expression Address = Operand.Address;

                    if(Instruction.Operands[0].Type != Operand_Register)
                    {
                        fprintf(Dest, "%s ", W ? "word" : "byte");
                    }

                    if(Flags & Inst_Segment)
                    {
                        printf("%s:", GetRegName({Address.Segment, 0, 2}));
                    }

                    fprintf(Dest, "[%s", GetEffectiveAddressExpression(Address));
                    if(Address.Displacement != 0)
                    {
                        fprintf(Dest, "%+d", Address.Displacement); 
                    }
                    fprintf(Dest, "]");
                } break;

                case Operand_Immediate:
                {
                    fprintf(Dest, "%d", Operand.ImmediateS32);
                } break;

                case Operand_RelativeImmediate:
                {
                    fprintf(Dest, "$%+d", Operand.ImmediateS32);
                } break;
            }
        }
    }

    u8 DestRegIndex = Instruction.Operands[0].Register.Index - 1;

    char const *InstructionOp = GetMnemonic(Instruction.Op);
    char const *DestinationRegName = GetRegName(Instruction.Operands[0].Register);

    switch(Instruction.Op)
    {
        case Op_mov:
            PrintRegisterChange(DestinationRegName, DestRegIndex, Registers, stdout);
            break;
        case Op_sub:
            PrintRegisterChange(DestinationRegName, DestRegIndex, Registers, stdout);
            fprintf(Dest, "flags:");
            fprintf(Dest, RegFlags->AF ? "A->": "");
            fprintf(Dest, RegFlags->CF ? "C": "");
            fprintf(Dest, RegFlags->SF ? "S": "");
            fprintf(Dest, RegFlags->PF ? "P": "");
            fprintf(Dest, RegFlags->ZF ? "Z": "");
            break;
        case Op_add:
        {
            PrintRegisterChange(DestinationRegName, DestRegIndex, Registers, stdout);
            fprintf(Dest, RegFlags->CF ? "flags:->C": "");
            fprintf(Dest, RegFlags->PF ? "P": "");
            fprintf(Dest, RegFlags->AF ? "A": "");
            fprintf(Dest, RegFlags->SF ? "S": "");
            fprintf(Dest, RegFlags->ZF ? "Z": "");
        } break;
        case Op_cmp:
        {
            PrintRegisterChange(DestinationRegName, DestRegIndex, Registers, stdout);
            fprintf(Dest, "; flags:->");
            fprintf(Dest, RegFlags->CF ? "C": "");
            fprintf(Dest, RegFlags->PF ? "P": "");
            fprintf(Dest, RegFlags->AF ? "A": "");
            fprintf(Dest, RegFlags->SF ? "S": "");
            fprintf(Dest, RegFlags->ZF ? "Z": "");
        } break;
        case Op_jne:
        {
            fprintf(Dest, " ; ip:0x%x->0x%x ", Registers[13].PreviousRegisterValue,
                                               Registers[13].RegisterValue);
        } break;
        case Op_loopnz:
        {
            fprintf(Dest, "loopnz");
        } break;
    }
}

static void PrintFinalRegisters(sim_register *Registers, flags *Flags, u8 RegisterSize, FILE *Dest)
{
    printf("\nFinal registers:\n");
    for(int Index = 0; Index < RegisterSize; ++Index)
    {
        if(Registers[Index].RegName != 0 && Registers[Index].RegisterValue != 0)
        {
            fprintf(Dest, "\t%s: 0x%04x (%d)\n", Registers[Index].RegName, 
                                          Registers[Index].RegisterValue,
                                          Registers[Index].RegisterValue);
        }
    }

    if(Flags->SF || Flags->PF || Flags->ZF || Flags->AF || Flags->CF)
    {
        fprintf(Dest, "   Flags: ");
    }

    fprintf(Dest, Flags->CF ? "C": "");
    fprintf(Dest, Flags->SF ? "S": "");
    fprintf(Dest, Flags->PF ? "P": "");
    fprintf(Dest, Flags->ZF ? "Z": "");
    fprintf(Dest, Flags->AF ? "A": "");
}
