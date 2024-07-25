static void SimulateInstruction(sim_register *Registers, flags *RegFlags, 
                                      instruction Instruction)
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
                } break;
            }
        }
    }

    u8 DestRegIndex = Instruction.Operands[0].Register.Index - 1;

    // Store previous value in PreviousRegisterValue
    Registers[DestRegIndex].PreviousRegisterValue = Registers[DestRegIndex].RegisterValue;

    char const *InstructionOp = GetMnemonic(Instruction.Op);
    if(InstructionOp == "mov")
    {
        // Move value in source register to destination register
        if(Instruction.Operands[1].Type == Operand_Register)
        {
            Registers[DestRegIndex].RegisterValue = Registers[LatestRegIndex].RegisterValue;
        }
        else
        {
            Registers[DestRegIndex].RegisterValue = Immediate;
        }
    }
    else if(InstructionOp == "sub")
    {
        u16 Parity = 0;

        u16 SubResult = Registers[DestRegIndex].RegisterValue - Registers[LatestRegIndex].RegisterValue;
        Registers[DestRegIndex].RegisterValue = SubResult;

        if(SubResult == 0)
        {
            RegFlags->ZF = true;
        }

        if(SubResult & 0x8000)
        {
            RegFlags->SF = true;
        }
        else
        {
            RegFlags->SF = false;
        }

        while(SubResult)
        {
            Parity ^= 1;
            SubResult &= (SubResult - 1);
        }

        if(Parity)
        {
            // Odd
            RegFlags->PF = false;
        }
        else 
        {
            // Even
            RegFlags->PF = true;
        }
    }
    else if(InstructionOp == "add")
    {
        u16 AddResult = Registers[DestRegIndex].RegisterValue + Immediate; 
        Registers[LatestRegIndex].RegisterValue = AddResult;
    }
    else if(InstructionOp == "cmp")
    {
    }
}
