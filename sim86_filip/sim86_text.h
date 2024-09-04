static b32 IsPrintable(instruction Instruction);
static void PrintInstruction (instruction Instruction, FILE *Dest);
static void PrintSimulatedInstruction(sim_register *Registers, flags *Flags, instruction Instruction, FILE *Dest);
static void PrintFinalRegisters(sim_register *Registers, flags *Flags, u8 RegisterSize, FILE *Dest);
static void PrintRegisterChange(char const *DestRegName, u8 DestRegIndex, sim_register *Registers, FILE *Dest);
static void PrintClocks(instruction Instruction, u16 Clocks, u32 *TotalClocks, s8 EffectiveAddressTime);
static void PrintExplainClocks(instruction Instruction, u16 Clocks, s8 EffectiveAddressTime, u32 *TotalClocks);
