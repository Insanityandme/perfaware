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

                    char const *RegName = GetRegName(Operand.Register);
                    if(RegName == "al" || RegName == "ah")
                    {
                        Registers[LatestRegIndex].RegName = "ax";
                    }
                    else
                    {
                        Registers[LatestRegIndex].RegName = GetRegName(Operand.Register);
                    }

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
            Registers[SourceRegIndex].PreviousRegisterValue = Registers[SourceRegIndex].RegisterValue;
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
            else if(OpTypeDest == Operand_Register && OpTypeSrc == Operand_Memory)
            {
                ExtractRegistersAddress(Address, Registers, &RegisterAddress1, &RegisterAddress2);

                u8 Read = ReadMemory(Memory, (RegisterAddress1 + RegisterAddress2 + (u32)Address.Displacement));
                AddResult = Registers[DestRegIndex].RegisterValue + Read;

                RegFlags->AF = ((Registers[DestRegIndex].RegisterValue & 0x0F) + 
                                (Read)) > 0x0F;
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
                CmpResult = Registers[DestRegIndex].RegisterValue - Registers[SourceRegIndex].RegisterValue;
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
            if(RegFlags->ZF == 0)
            {
                Registers[13].RegisterValue = Registers[13].PreviousRegisterValue + Immediate;
                At->SegmentOffset = Registers[13].PreviousRegisterValue + Immediate;
            }
            else
            {
                RegFlags->ZF = 1;
            }

        } break;
        case Op_je:
        {
           if(RegFlags->ZF == 1)
           {
                Registers[13].RegisterValue = Registers[13].PreviousRegisterValue + Immediate;
                At->SegmentOffset = Registers[13].PreviousRegisterValue + Immediate;
           }
           else
           {
               RegFlags->ZF = 0;
           }
        } break;
        case Op_test:
        {
            u16 Result = Registers[DestRegIndex].RegisterValue & Registers[SourceRegIndex].RegisterValue;

            RegFlags->ZF = (Result == 0);
            RegFlags->SF = (Result & 0x8000);
            RegFlags->PF = CalculateParity(Result);
        } break;
        case Op_xor:
        {
            u16 XorResult = 0;

            // Means that there is no source register
            if(Registers[SourceRegIndex].RegisterValue == 0)
            {
                XorResult = Registers[DestRegIndex].RegisterValue ^ Immediate;    
            }
            else
            {
                XorResult = Registers[DestRegIndex].RegisterValue ^ Registers[SourceRegIndex].RegisterValue;
            }

            RegFlags->ZF = (XorResult == 0);
            RegFlags->PF = CalculateParity(XorResult);
        } break;
        case Op_ret:
        {
        } break;
        case Op_inc:
        {
            u16 Result = Registers[SourceRegIndex].RegisterValue + 1;

            Registers[SourceRegIndex].RegisterValue = Result;

            RegFlags->ZF = (Result == 0);
            RegFlags->SF = (Result < 0);
            RegFlags->PF = CalculateParity(Result);
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
