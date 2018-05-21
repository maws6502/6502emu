#define sbi(x, y) (x |= 1 << y)
#define cbi(x, y) (x &= ~(1 << y))
#define tbi(x, y) (x ^= 1 << y)
#define gbi(x, y) ((x & 1 << y) >> y)
#define setcon(x, y, exp) (exp ? sbi(x, y) : cbi(x, y))
#define setstat(y, exp) setcon(dev->sr, y, exp)
#define getstat(x) gbi(dev->sr, x)


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
        NO1,
        NO2,
        NO3,
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
        TYA,

        /* 65C02 instructions */
        /* http://www.llx.com/~nparker/a2/opcodes.html */
        BRA,
        STZ,
        PHX,
        PHY,
        PLX,
        PLY,
        INA,
        DEA,
        TRB,
        TSB,
        RM0,
        RM1,
        RM2,
        RM3,
        RM4,
        RM5,
        RM6,
        RM7,
        SM0,
        SM1,
        SM2,
        SM3,
        SM4,
        SM5,
        SM6,
        SM7,
        BR0,
        BR1,
        BR2,
        BR3,
        BR4,
        BR5,
        BR6,
        BR7,
        BS0,
        BS1,
        BS2,
        BS3,
        BS4,
        BS5,
        BS6,
        BS7,
};

enum {
        A_ACC,
        A_IMP,
        A_ABS,
        A_ABX,
        A_ABY,
        A_IMM,
        A_IND,
        A_IAX,
        A_INX,
        A_INY,
        A_INZ,
        A_REL,
        A_ZPG,
        A_ZPX,
        A_ZPY,
        A_ZPR
};
