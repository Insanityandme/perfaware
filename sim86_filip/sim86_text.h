static b32 IsPrintable(instruction Instruction);
static void PrintInstruction (instruction Instruction, FILE *Dest);
static void PrintSimulatedInstruction(sim_register *Registers, flags *Flags, instruction Instruction, FILE *Dest);
static void PrintFinalRegisters(sim_register *Registers, flags *Flags, u8 RegisterSize, FILE *Dest);
