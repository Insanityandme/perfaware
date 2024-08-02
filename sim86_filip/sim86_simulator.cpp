static void SimulateInstruction(sim_register *Registers, flags *RegFlags, 
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
    u16 Immediate = 0;
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
                    effective_address_expression Address = Operand.Address;
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

    u8 DestRegIndex = 0;
    u8 SourceRegIndex = 0;

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

        } break;
        case Op_add:
        {
            u16 AddResult = Registers[DestRegIndex].RegisterValue + Immediate; 

            RegFlags->AF = ((Registers[DestRegIndex].RegisterValue & 0x0F) + (Immediate & 0x0F)) > 0x0F;

            Registers[LatestRegIndex].RegisterValue = AddResult;
        } break;
        case Op_sub:
        {
            u16 SubResult = 0;

            // Means that there is no source register
            if(Registers[SourceRegIndex].RegisterValue == 0)
            {
                SubResult = Registers[DestRegIndex].RegisterValue - Immediate;    
            }
            else 
            {
                SubResult = Registers[DestRegIndex].RegisterValue - Registers[SourceRegIndex].RegisterValue;
            }

            RegFlags->AF = ((Registers[DestRegIndex].RegisterValue & 0x0F) -
                            (Registers[LatestRegIndex].RegisterValue & 0x0F)) < 0;
            RegFlags->CF = Registers[DestRegIndex].RegisterValue < Registers[LatestRegIndex].RegisterValue;

            Registers[DestRegIndex].RegisterValue = SubResult;

            RegFlags->ZF = (SubResult == 0);
            RegFlags->SF = (SubResult & 0x8000);

            u16 Parity = 0;
            while(SubResult)
            {
                Parity ^= 1;
                SubResult &= (SubResult - 1);
            }

            RegFlags->PF = !Parity;
        } break;
        case Op_jne:
        {
            if(RegFlags->ZF == 1)
            {
                break;
            }

            Registers[13].RegisterValue = -Immediate;
            At->SegmentOffset = -Immediate;
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
