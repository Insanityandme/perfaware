static b32 IsPrintable(instruction Instruction);
static void PrintInstruction (instruction Instruction, FILE *Dest);
static void PrintSimulatedInstruction(sim_register *Registers, instruction Instruction, FILE *Dest);
static void PrintFinalRegisters(sim_register *Registers, u8 RegisterSize);
