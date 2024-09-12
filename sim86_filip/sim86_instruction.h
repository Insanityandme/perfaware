// typedef u32 register_index;
// 
// // NOTE(casey): To make it easier to compile with languages which do not
// // have auto-typedef'ing (like C, etc.), all types are manually typedef'd here.
// typedef struct register_access register_access;
// typedef struct effective_address_term effective_address_term;
// typedef struct effective_address_expression effective_address_expression;
// typedef struct immediate immediate;
// typedef struct instruction_operand instruction_operand;
// typedef struct instruction instruction;
// 
// typedef enum operation_type : u32

enum operation_type
{
    Op_None,

#define INST(Mnemonic, ...) Op_##Mnemonic,
#define INSTALT(...)
#include "sim86_instruction_table.inl"

    Op_Count,
};

enum instruction_flag 
{
    Inst_Lock = (1 << 0),
    Inst_Rep = (1 << 1),
    Inst_Segment = (1 << 2),
    Inst_Wide = (1 << 3),
};

enum register_index
{
    Register_none,

    Register_a,
    Register_b,
    Register_c,
    Register_d,
    Register_sp,
    Register_bp,
    Register_si,
    Register_di,
    Register_es,
    Register_cs,
    Register_ss,
    Register_ds,
    Register_ip,
    Register_flags,

    Register_count,
};

enum effective_address_base
{
    EffectiveAddress_direct,

    EffectiveAddress_bx_si,
    EffectiveAddress_bx_di,
    EffectiveAddress_bp_si,
    EffectiveAddress_bp_di,
    EffectiveAddress_si,
    EffectiveAddress_di,
    EffectiveAddress_bp,
    EffectiveAddress_bx,

    EffectiveAddress_count,
};

struct effective_address_expression
{
    register_index Segment;
    effective_address_base Base;
    s32 Displacement;
};

struct register_access
{
    register_index Index;
    u8 Offset;
    u8 Count;
};

enum operand_type
{
    Operand_None,
    Operand_Register,
    Operand_Memory,
    Operand_Immediate,
    Operand_RelativeImmediate,
};

struct instruction_operand
{
    operand_type Type;
    union
    {
        effective_address_expression Address;
        register_access Register;
        u32 ImmediateU32;
        s32 ImmediateS32;
    };
};

struct instruction
{
    u32 Address;
    u32 Size;

    operation_type Op;
    u32 Flags;

    instruction_operand Operands[2];
};
