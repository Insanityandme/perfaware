struct sim_register
{
    u16 PreviousRegisterValue;
    u16 RegisterValue;
    char const *RegName;
};

struct flags
{
    b32 SF;
    b32 ZF;
    b32 PF;
    b32 AF;
    b32 CF;
};

static void SimulateInstruction(sim_register *Registers, flags *Flags, instruction Instruction,
                                segmented_access *At);
