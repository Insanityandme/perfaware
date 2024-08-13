
static u16 WriteMemory(memory *Memory, s32 Displacement, u16 Value)
{
    Memory->Bytes[Displacement] = Value;
    u16 Result = Memory->Bytes[Displacement];
    return Result;
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

    if(Instruction.Operands)
    {
        if(Instruction.Operands[0].Type == Operand_Register)
        {
            DestRegIndex = Instruction.Operands[0].Register.Index - 1;
            Registers[DestRegIndex].PreviousRegisterValue = Registers[DestRegIndex].RegisterValue;
        }
        if(Instruction.Operands[1].Type == Operand_Register)
        {
            SourceRegIndex = Instruction.Operands[1].Register.Index - 1;
        }
    }


    switch(Instruction.Op)
    {
        case Op_mov:
        {
            if(Instruction.Operands[1].Type == Operand_Register)
            {
                Registers[DestRegIndex].RegisterValue = Registers[LatestRegIndex].RegisterValue;
            }
            else
            {
                Registers[DestRegIndex].RegisterValue = Immediate;
            }

            if(Instruction.Operands[0].Type == Operand_Memory)
            {
                char const *RegisterName = GetEffectiveAddressExpression(Address);
                u16 Displacement = 0;
                u16 RegisterAddress = 0;

                for(int i = 0; i < sizeof(Registers); i++)
                {
                    if(Registers[i].RegName == RegisterName)
                    {
                        RegisterAddress = Registers[i].RegisterValue;
                    }
                }
                if(Address.Displacement != 0)
                {
                    Displacement = Address.Displacement;
                }
                u16 Result = WriteMemory(Memory, (RegisterAddress + Address.Displacement), Immediate);
            }
            else if(Instruction.Operands[1].Type == Operand_Memory)
            {
                u16 Read = ReadMemory(Memory, Address.Displacement);
                Registers[DestRegIndex].RegisterValue = Read;
            }


        } break;
        case Op_add:
        {
            u16 AddResult = Registers[DestRegIndex].RegisterValue + Immediate; 

            RegFlags->AF = ((Registers[DestRegIndex].RegisterValue & 0x0F) + (Immediate & 0x0F)) > 0x0F;

            Registers[LatestRegIndex].RegisterValue = AddResult;
            RegFlags->PF = CalculateParity(AddResult);
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
            if(Registers[DestRegIndex].RegisterValue == Registers[LatestRegIndex].RegisterValue)
            {
                RegFlags->ZF = true;
            }

            u16 CmpResult = Registers[DestRegIndex].RegisterValue - Registers[LatestRegIndex].RegisterValue;
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

            Registers[13].RegisterValue = Registers[13].PreviousRegisterValue - (-Immediate);
            At->SegmentOffset = Registers[13].PreviousRegisterValue - (-Immediate);
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
