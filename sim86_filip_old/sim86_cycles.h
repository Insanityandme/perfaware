struct clocks
{
    u16 Clocks;
    s8 EffectiveAddressTime;
    s8 Transfers;
};

static clocks CalculateClocks(instruction Instruction);
