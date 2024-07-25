struct sim_register
{
    b32 FirstPass;
    u16 PreviousRegisterValue;
    u16 RegisterValue;
    char const *RegName;
};

struct flags
{
    b32 SF;
    b32 ZF;
    b32 PF;
};

static void SimulateInstruction(sim_register *Registers, flags *Flags, instruction Instruction);
