static void ExtractRegistersAddress(effective_address_expression Address, sim_register *Registers, 
                                    u16 *RegisterAddress1, u16 *RegisterAddress2)
{
    char const *OriginalRegisterName = GetEffectiveAddressExpression(Address);
    char RegisterName[100];
    char RegisterNameToCompare[100];
    
    strncpy(RegisterName, OriginalRegisterName, sizeof(RegisterName));

    char delimiter[] = "+";
    char *FirstPart = strtok(RegisterName, delimiter);
    char *SecondPart = strtok(NULL, delimiter);

    for(int i = 0; i < sizeof(Registers); i++)
    {
        if(Registers[i].RegName != 0 && RegisterNameToCompare != 0)
        {
            strncpy(RegisterNameToCompare, Registers[i].RegName, sizeof(RegisterNameToCompare));

            if(FirstPart != 0)
            {
                if(strcmp(FirstPart, RegisterNameToCompare) == 0)
                {
                    *RegisterAddress1 = Registers[i].RegisterValue;
                }
            }

            if(SecondPart != 0)
            {
                if(strcmp(SecondPart, RegisterNameToCompare) == 0)
                {
                    *RegisterAddress2 = Registers[i].RegisterValue;
                }
            }
        }
    }
}

static b32 CalculateParity(u16 x)
{
    // NOTE: Normally you would use a population count instruction for this operation,
    // but the only way to do that in vanilla C++ is to use the std:: library which I don't
    // think is a good idea in general for a variety of reasons. So the parity is computed
    // manually here using Hacker's Delight Second Edition page 96 (I have kept the variable
    // names the same as what appears there):
    
    u16 y = x ^ (x >> 1);
    y = y ^ (y >> 2);
    y = y ^ (y >> 4);

    return ((~y & 0x1) << 2);
}

static clocks CalculateClocks(instruction Instruction)
{
    u32 Flags = Instruction.Flags;
    u16 Clocks = 0;
    s8 EffectiveAddressTime = 0;
    u16 Displacement = 0;
    s8 Transfers = 0;

    for(u32 OperandIndex = 0; OperandIndex < ArrayCount(Instruction.Operands); ++OperandIndex)
    {
        instruction_operand Operand = Instruction.Operands[OperandIndex];
        if(Operand.Type != Operand_None)
        {
            switch(Operand.Type)
            {
                case Operand_Memory:
                {
                    effective_address_expression Address = Operand.Address;

                    char const *AddressExpression = GetEffectiveAddressExpression(Address);

                    if(Address.Displacement != 0)
                    {
                        Displacement = Address.Displacement;
                        // Displacement Only
                        if(*AddressExpression == '\0')
                        {
                            EffectiveAddressTime = 6;
                        }
                        // Displacement + Base or Index
                        if(strcmp(AddressExpression, "bx") == 0 || strcmp(AddressExpression, "bp") == 0 || 
                           strcmp(AddressExpression, "si") == 0 || strcmp(AddressExpression, "di") == 0)
                        {
                            EffectiveAddressTime = 9;
                        }
                        // Displacement + Base + Index
                        else if(strcmp(AddressExpression, "bp+di") == 0 || strcmp(AddressExpression, "bx+si") == 0)
                        {
                            EffectiveAddressTime = 11 ;
                        }
                        else if(strcmp(AddressExpression, "bp+si") == 0 || strcmp(AddressExpression, "bx+di") == 0)
                        {
                            EffectiveAddressTime = 12;
                        }
                    }
                    else 
                    {
                        // Base Or Index Only
                        if(strcmp(AddressExpression, "bx") == 0 || strcmp(AddressExpression, "bp") == 0 || 
                           strcmp(AddressExpression, "si") == 0 || strcmp(AddressExpression, "di") == 0)
                        {
                            EffectiveAddressTime = 5;
                        }
                        // Base + Index
                        else if(strcmp(AddressExpression, "bp+di") == 0 || strcmp(AddressExpression, "bx+si") == 0)
                        {
                            EffectiveAddressTime = 7;
                        }
                        else if(strcmp(AddressExpression, "bp+si") == 0 || strcmp(AddressExpression, "bx+di") == 0)
                        {
                            EffectiveAddressTime = 8;
                        }
                    }
                } break;
            }
        }
    }

    operand_type OpTypeDest = Instruction.Operands[0].Type;
    operand_type OpTypeSrc = Instruction.Operands[1].Type;

    switch(Instruction.Op)
    {
        case Op_mov:
        {
            if(OpTypeDest == Operand_Register && OpTypeSrc == Operand_Register)
            {
                Clocks = 2;
            }
            else if(OpTypeDest == Operand_Memory && OpTypeSrc == Operand_Immediate)
            {
                Clocks = 10;
            }
            else if(OpTypeDest == Operand_Register && OpTypeSrc == Operand_Immediate)
            {
                Clocks = 4;
            }
            else if(OpTypeDest == Operand_Memory && OpTypeSrc == Operand_Register)
            {
                Clocks = 9;
            }
            else if(OpTypeDest == Operand_Register && OpTypeSrc == Operand_Memory)
            {
                Clocks = 8;
            }
        } break;
        case Op_add:
        {
            if(OpTypeDest == Operand_Register && OpTypeSrc == Operand_Register)
            {
                Clocks = 3;
            }
            else if(OpTypeDest == Operand_Memory && OpTypeSrc == Operand_Immediate)
            {
                Clocks = 17;
            }
            else if(OpTypeDest == Operand_Register && OpTypeSrc == Operand_Immediate)
            {
                Clocks = 4;
            }
            else if(OpTypeDest == Operand_Memory && OpTypeSrc == Operand_Register)
            {
                if(Displacement % 2 != 0)
                {
                    Clocks = 16 + 8;
                    Transfers = 8;
                }
                else 
                {
                    Clocks = 16;
                }
            }
            else if(OpTypeDest == Operand_Register && OpTypeSrc == Operand_Memory)
            {
                if(Displacement % 2 != 0)
                {
                    Clocks = 9 + 4;
                    Transfers = 4;
                }
                else
                {
                    Clocks = 9;
                }
            }
        } break;
    }

    clocks CurrentClocks = {};
    CurrentClocks.Clocks = Clocks;
    CurrentClocks.EffectiveAddressTime = EffectiveAddressTime;
    CurrentClocks.Transfers = Transfers;

    return CurrentClocks;
}

static void SimulateInstruction(memory *Memory, sim_register *Registers, flags *RegFlags, 
                                      instruction Instruction, segmented_access *At)
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
    }

    u8 LatestRegIndex = 0;
    u8 DestRegIndex = 0;
    u8 SourceRegIndex = 0;
    u16 Immediate = 0;
    effective_address_expression Address = {};

    for(u32 OperandIndex = 0; OperandIndex < ArrayCount(Instruction.Operands); ++OperandIndex)
    {
        instruction_operand Operand = Instruction.Operands[OperandIndex];

        if(Operand.Type != Operand_None)
        {

            switch(Operand.Type)
            {
                case Operand_None: {} break;

                case Operand_Register:
                {
                    LatestRegIndex = Operand.Register.Index - 1;
                    Registers[LatestRegIndex].RegName = GetRegName(Operand.Register);
                } break;
                                   
                case Operand_Memory:
                {
                    Address = Operand.Address;
                } break;

                case Operand_Immediate:
                {
                    Immediate = (u16)Operand.ImmediateS32;
                } break;

                case Operand_RelativeImmediate:
                {
                    Immediate = (u16)Operand.ImmediateS32;
                } break;
            }
        }
    }

    operand_type OpTypeDest = Instruction.Operands[0].Type;
    operand_type OpTypeSrc = Instruction.Operands[1].Type;
    if(Instruction.Operands)
    {
        if(OpTypeDest == Operand_Register)
        {
            DestRegIndex = Instruction.Operands[0].Register.Index - 1;
            Registers[DestRegIndex].PreviousRegisterValue = Registers[DestRegIndex].RegisterValue;
        }

        if(OpTypeSrc == Operand_Register)
        {
            SourceRegIndex = Instruction.Operands[1].Register.Index - 1;
        }
    }

    u16 RegisterAddress1 = 0;
    u16 RegisterAddress2 = 0;
    switch(Instruction.Op)
    {
        case Op_mov:
        {
            if(OpTypeDest == Operand_Register && OpTypeSrc == Operand_Register)
            {
                Registers[DestRegIndex].RegisterValue = Registers[LatestRegIndex].RegisterValue;
            }
            else if(OpTypeDest == Operand_Memory && OpTypeSrc == Operand_Immediate)
            {
                ExtractRegistersAddress(Address, Registers, &RegisterAddress1, &RegisterAddress2);

                WriteMemory(Memory, (RegisterAddress1 + RegisterAddress2 + (u32)Address.Displacement), Immediate);
            }
            else if(OpTypeDest == Operand_Register && OpTypeSrc == Operand_Immediate)
            {
                Registers[DestRegIndex].RegisterValue = Immediate;
            }
            else if(OpTypeDest == Operand_Memory && OpTypeSrc == Operand_Register)
            {
                ExtractRegistersAddress(Address, Registers, &RegisterAddress1, &RegisterAddress2);

                u16 Read = Registers[SourceRegIndex].RegisterValue;
                u16 Result = WriteMemory(Memory, (RegisterAddress1 + RegisterAddress2 + (u32)Address.Displacement), Read);
            }
            else if(OpTypeDest == Operand_Register && OpTypeSrc == Operand_Memory)
            {
                ExtractRegistersAddress(Address, Registers, &RegisterAddress1, &RegisterAddress2);

                u8 Read = ReadMemory(Memory, (RegisterAddress1 + RegisterAddress2 + (u32)Address.Displacement));
                Registers[DestRegIndex].RegisterValue = Read;
            }


        } break;
        case Op_add:
        {
            u16 AddResult = 0;

            if(OpTypeDest == Operand_Register && OpTypeSrc == Operand_Register)
            {
                AddResult = Registers[DestRegIndex].RegisterValue + Registers[SourceRegIndex].RegisterValue;

                RegFlags->AF = ((Registers[DestRegIndex].RegisterValue & 0x0F) + 
                                (Registers[SourceRegIndex].RegisterValue & 0x0F)) > 0x0F;
            }
            else
            {
                AddResult = Registers[DestRegIndex].RegisterValue + Immediate; 

                RegFlags->AF = ((Registers[DestRegIndex].RegisterValue & 0x0F) + (Immediate & 0x0F)) > 0x0F;
            }

            RegFlags->PF = CalculateParity(AddResult);

            Registers[DestRegIndex].RegisterValue = AddResult;
        } break;
        case Op_sub:
        {
            u16 SubResult = 0;

            // Means that there is no source register
            if(Registers[SourceRegIndex].RegisterValue == 0)
            {
                SubResult = Registers[DestRegIndex].RegisterValue - Immediate;    
                RegFlags->AF = ((Registers[DestRegIndex].RegisterValue & 0x0F) -
                                (Immediate & 0x0F)) < 0;
            }
            else 
            {
                SubResult = Registers[DestRegIndex].RegisterValue - Registers[SourceRegIndex].RegisterValue;
                RegFlags->AF = ((Registers[DestRegIndex].RegisterValue & 0x0F) -
                                (Registers[SourceRegIndex].RegisterValue & 0x0F)) < 0;
            }

            RegFlags->CF = Registers[DestRegIndex].RegisterValue < Registers[SourceRegIndex].RegisterValue;

            Registers[DestRegIndex].RegisterValue = SubResult;

            RegFlags->ZF = (SubResult == 0);
            RegFlags->SF = (SubResult & 0x8000);
            RegFlags->PF = CalculateParity(SubResult);
        } break;
        case Op_cmp:
        {
            u16 CmpResult = 0;

            // Means that there is no source register
            if(Registers[SourceRegIndex].RegisterValue == 0)
            {
                CmpResult = Registers[DestRegIndex].RegisterValue - Immediate;    
            }
            else
            {
                u16 CmpResult = Registers[DestRegIndex].RegisterValue - Registers[SourceRegIndex].RegisterValue;
            }

            RegFlags->ZF = (CmpResult == 0);
            RegFlags->AF = ((Registers[DestRegIndex].RegisterValue & 0x0F) -
                            (Registers[SourceRegIndex].RegisterValue & 0x0F)) < 0;
            RegFlags->PF = CalculateParity(CmpResult);
            RegFlags->CF = Registers[DestRegIndex].RegisterValue < Registers[SourceRegIndex].RegisterValue;
            RegFlags->SF = (CmpResult & 0x8000);
            
        } break;
        case Op_jne:
        {
            if(RegFlags->ZF == 1)
            {
                break;
            }

            Registers[13].RegisterValue = Registers[13].PreviousRegisterValue + Immediate;
            At->SegmentOffset = Registers[13].PreviousRegisterValue + Immediate;
        } break;
        case Op_loop:
        {
            printf("loop");
        } break;
        case Op_loopz:
        {
            printf("loopz");
        } break;
        case Op_loopnz:
        {
            printf("loopnz");
        } break;
    }
}
