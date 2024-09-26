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
        case Op_cmp:
        {
            Clocks = 3;
        } break;
        case Op_inc:
        {
            Clocks = 2;
        } break;
        case Op_jne:
        {
            Clocks = 16;
        } break;
        case Op_je:
        {
            Clocks = 4;
        } break;
        case Op_test:
        {
            Clocks = 3;
        } break; 
        case Op_xor:
        {
            Clocks = 3;
        } break;
    }

    clocks CurrentClocks = {};
    CurrentClocks.Clocks = Clocks;
    CurrentClocks.EffectiveAddressTime = EffectiveAddressTime;
    CurrentClocks.Transfers = Transfers;

    return CurrentClocks;
}
