#define sbi(x, y) (x |= 1 << y)
#define cbi(x, y) (x &= ~(1 << y))
#define tbi(x, y) (x ^= 1 << y)
#define gbi(x, y) ((x & 1 << y) >> y)
#define setcon(x, y, exp) (exp ? sbi(x, y) : cbi(x, y))
#define setstat(y, exp) setcon(sr, y, exp)
#define getstat(x) gbi(sr, x)

uint8_t memory[0x10000];
void reset_cpu(void);
void irq(void);
void nmi(void);
int cycle(void);

struct opcode {
        int op;
        int adm;
};

typedef struct opcode Opcode;

enum
{
        nmiL = 0xFFFA,
        nmiH,
        rstL,
        rstH,
        irqL,
        irqH

};

enum
{
        CARRY,
        ZERO,
        INTERRUPT,
        DECIMAL,
        BREAK,
        _IGNORE,
        OVERFLOW,
        NEGATIVE
};
enum
{
        INV,
        ADC,
        AND,
        ASL,
        ASC,
        BCC,
        BCS,
        BEQ,
        BIT,
        BMI,
        BNE,
        BPL,
        BRK,
        BVC,
        BVS,
        CLC,
        CLD,
        CLI,
        CLV,
        CMP,
        CPX,
        CPY,
        DEC,
        DEX,
        DEY,
        EOR,
        INC,
        INX,
        INY,
        JMP,
        JSR,
        LDA,
        LDX,
        LDY,
        LSR,
        LSC,
        NOP,
        ORA,
        PHA,
        PHP,
        PLA,
        PLP,
        ROL,
        RLC,
        ROR,
        RRC,
        RTI,
        RTS,
        SBC,
        SEC,
        SED,
        SEI,
        STA,
        STX,
        STY,
        TAX,
        TAY,
        TSX,
        TXA,
        TXS,
        TYA
};
enum {
        A_ACC,
        A_IMP,
        A_ABS,
        A_ABX,
        A_ABY,
        A_IMM,
        A_IND,
        A_INX,
        A_INY,
        A_REL,
        A_ZPG,
        A_ZPX,
        A_ZPY
};
