/* emu.c
 * 6502emu
 * Aditya Goturu <aditya@sudoforlunch.org>
 */

#include <stdint.h>
#include "emu.h"

static uint16_t pc;
static uint8_t ac, x, y, sr, sp;
static uint16_t invalid_instruction = 0;

/* acquired from http://e-tradition.net/bytes/6502/6502_instruction_set.html */
static Opcode opc_table[0x100] = {
        {BRK, A_IMP}, {ORA, A_INX}, {INV, A_IMP}, {INV, A_IMP}, {TSB, A_ZPG}, {ORA, A_ZPG}, {ASL, A_ZPG}, {INV, A_IMP}, {PHP, A_IMP}, {ORA, A_IMM}, {ASC, A_IMP}, {INV, A_IMP}, {TSB, A_ABS}, {ORA, A_ABS}, {ASL, A_ABS}, {INV, A_IMP},
        {BPL, A_REL}, {ORA, A_INY}, {ORA, A_INZ}, {INV, A_IMP}, {TRB, A_ZPG}, {ORA, A_ZPX}, {ASL, A_ZPX}, {INV, A_IMP}, {CLC, A_IMP}, {ORA, A_ABY}, {INA, A_IMP}, {INV, A_IMP}, {TRB, A_ABS}, {ORA, A_ABX}, {ASL, A_ABX}, {INV, A_IMP}, 
        {JSR, A_ABS}, {AND, A_INX}, {INV, A_IMP}, {INV, A_IMP}, {BIT, A_ZPG}, {AND, A_ZPG}, {ROL, A_ZPG}, {INV, A_IMP}, {PLP, A_IMP}, {AND, A_IMM}, {RLC, A_IMP}, {INV, A_IMP}, {BIT, A_ABS}, {AND, A_ABS}, {ROL, A_ABS}, {INV, A_IMP}, 
        {BMI, A_REL}, {AND, A_INY}, {AND, A_INZ}, {INV, A_IMP}, {BIT, A_ZPX}, {AND, A_ZPX}, {ROL, A_ZPX}, {INV, A_IMP}, {SEC, A_IMP}, {AND, A_ABY}, {DEA, A_IMP}, {INV, A_IMP}, {BIT, A_ABX}, {AND, A_ABX}, {ROL, A_ABX}, {INV, A_IMP},
        {RTI, A_IMP}, {EOR, A_INX}, {INV, A_IMP}, {INV, A_IMP}, {INV, A_IMP}, {EOR, A_ZPG}, {LSR, A_ZPG}, {INV, A_IMP}, {PHA, A_IMP}, {EOR, A_IMM}, {LSC, A_IMP}, {INV, A_IMP}, {JMP, A_ABS}, {EOR, A_ABS}, {LSR, A_ABS}, {INV, A_IMP},
        {BVC, A_REL}, {EOR, A_INY}, {EOR, A_INZ}, {INV, A_IMP}, {INV, A_IMP}, {EOR, A_ZPX}, {LSR, A_ZPX}, {INV, A_IMP}, {CLI, A_IMP}, {EOR, A_ABY}, {PHY, A_IMP}, {INV, A_IMP}, {INV, A_IMP}, {EOR, A_ABX}, {LSR, A_ABX}, {INV, A_IMP}, 
        {RTS, A_IMP}, {ADC, A_INX}, {INV, A_IMP}, {INV, A_IMP}, {STZ, A_ZPG}, {ADC, A_ZPG}, {ROR, A_ZPG}, {INV, A_IMP}, {PLA, A_IMP}, {ADC, A_IMM}, {RRC, A_IMP}, {INV, A_IMP}, {JMP, A_IND}, {ADC, A_ABS}, {ROR, A_ABS}, {INV, A_IMP},
        {BVS, A_REL}, {ADC, A_INY}, {ADC, A_INZ}, {INV, A_IMP}, {STZ, A_ZPX}, {ADC, A_ZPX}, {ROR, A_ZPX}, {INV, A_IMP}, {SEI, A_IMP}, {ADC, A_ABY}, {PLY, A_IMP}, {INV, A_IMP}, {JMP, A_IAX}, {ADC, A_ABX}, {ROR, A_ABX}, {INV, A_IMP}, 
        {BRA, A_REL}, {STA, A_INX}, {INV, A_IMP}, {INV, A_IMP}, {STY, A_ZPG}, {STA, A_ZPG}, {STX, A_ZPG}, {INV, A_IMP}, {DEY, A_IMP}, {BIT, A_IMM}, {TXA, A_IMP}, {INV, A_IMP}, {STY, A_ABS}, {STA, A_ABS}, {STX, A_ABS}, {INV, A_IMP}, 
        {BCC, A_REL}, {STA, A_INY}, {STA, A_INZ}, {INV, A_IMP}, {STY, A_ZPX}, {STA, A_ZPX}, {STX, A_ZPY}, {INV, A_IMP}, {TYA, A_IMP}, {STA, A_ABY}, {TXS, A_IMP}, {INV, A_IMP}, {STZ, A_ABS}, {STA, A_ABX}, {STZ, A_ABX}, {INV, A_IMP}, 
        {LDY, A_IMM}, {LDA, A_INX}, {LDX, A_IMM}, {INV, A_IMP}, {LDY, A_ZPG}, {LDA, A_ZPG}, {LDX, A_ZPG}, {INV, A_IMP}, {TAY, A_IMP}, {LDA, A_IMM}, {TAX, A_IMP}, {INV, A_IMP}, {LDY, A_ABS}, {LDA, A_ABS}, {LDX, A_ABS}, {INV, A_IMP},
        {BCS, A_REL}, {LDA, A_INY}, {LDA, A_INZ}, {INV, A_IMP}, {LDY, A_ZPX}, {LDA, A_ZPX}, {LDX, A_ZPY}, {INV, A_IMP}, {CLV, A_IMP}, {LDA, A_ABY}, {TSX, A_IMP}, {INV, A_IMP}, {LDY, A_ABX}, {LDA, A_ABX}, {LDX, A_ABY}, {INV, A_IMP}, 
        {CPY, A_IMM}, {CMP, A_INX}, {INV, A_IMP}, {INV, A_IMP}, {CPY, A_ZPG}, {CMP, A_ZPG}, {DEC, A_ZPG}, {INV, A_IMP}, {INY, A_IMP}, {CMP, A_IMM}, {DEX, A_IMP}, {INV, A_IMP}, {CPY, A_ABS}, {CMP, A_ABS}, {DEC, A_ABS}, {INV, A_IMP},
        {BNE, A_REL}, {CMP, A_INY}, {CMP, A_INZ}, {INV, A_IMP}, {INV, A_IMP}, {CMP, A_ZPX}, {DEC, A_ZPX}, {INV, A_IMP}, {CLD, A_IMP}, {CMP, A_ABY}, {PHX, A_IMP}, {INV, A_IMP}, {INV, A_IMP}, {CMP, A_ABX}, {DEC, A_ABX}, {INV, A_IMP}, 
        {CPX, A_IMM}, {SBC, A_INX}, {INV, A_IMP}, {INV, A_IMP}, {CPX, A_ZPG}, {SBC, A_ZPG}, {INC, A_ZPG}, {INV, A_IMP}, {INX, A_IMP}, {SBC, A_IMM}, {NOP, A_IMP}, {INV, A_IMP}, {CPX, A_ABS}, {SBC, A_ABS}, {INC, A_ABS}, {INV, A_IMP},
        {BEQ, A_REL}, {SBC, A_INY}, {SBC, A_INZ}, {INV, A_IMP}, {INV, A_IMP}, {SBC, A_ZPX}, {INC, A_ZPX}, {INV, A_IMP}, {SED, A_IMP}, {SBC, A_ABY}, {PLX, A_IMP}, {INV, A_IMP}, {INV, A_IMP}, {SBC, A_ABX}, {INC, A_ABX}, {INV, A_IMP}
};

static void
hws_push(uint8_t val)
{
        memory[0x100 + sp] = val;
        if (sp)
                sp--;
        else sp = 0xFF;
        return;
}

static uint8_t
hws_pop(void)
{
       if (sp == 0xFF) sp = 0x0;
       else sp++;
       return memory[0x100 + sp];
}

static uint16_t
get_addr(int adm)
{
        uint8_t low, high, efflow, effhigh;
        int8_t offset;
        switch (adm) {
                case A_ABS:
                        low = memory[pc++];
                        high = memory[pc++];
                        return low + (high << 8);
                case A_ABX:
                        low = memory[pc++];
                        high = memory[pc++];
                        return low + (high << 8) + x;
                case A_ABY:
                        low = memory[pc++];
                        high = memory[pc++];
                        return low + (high << 8) + y;
                case A_IMM:
                        return pc++;
                case A_IND:
                        low = memory[pc++];
                        high = memory[pc++];
                        efflow = memory[low + (high << 8)];
                        effhigh = memory[low + (high << 8) + 1];
                        return efflow + (effhigh << 8);
                case A_IAX:
                        low = memory[pc++];
                        high = memory[pc++];
                        efflow = memory[low + (high << 8) + x];
                        effhigh = memory[low + (high << 8) + 1 + x];
                        return efflow + (effhigh << 8);
                case A_INX:
                        low = (memory[pc++] + x) % 0xFF;
                        high = ++low % 0xFF;
                        return memory[low] + (memory[high] << 8);
                case A_INY:
                        low = memory[pc++];
                        high = ++low % 0xFF;
                        return memory[low] + (memory[high] << 8) + y;
                case A_INZ:
                        low = memory[pc++];
                        high = ++low % 0xFF;
                        return memory[low] + (memory[high] << 8);
                case A_REL:
                        offset = (int8_t) memory[pc++];
                        return pc + offset;
                case A_ZPG:
                        return memory[pc++];
                case A_ZPX:
                        return (memory[pc++] + x) % 0xFF;
                case A_ZPY:
                        return (memory[pc++] + y) % 0xFF;
                case A_ACC:
                case A_IMP:
                default:
                        return 0;
        }
}

static void
run_op(Opcode op)
{
        uint16_t addr = get_addr(op.adm);
        uint8_t m;
        unsigned int tmp;
        switch (op.op){
                case ADC:
                        /* stolen from https://github.com/gianlucag/mos6502 because */
                        m = memory[addr];
                        tmp = m + ac + getstat(CARRY);
                        setstat(ZERO, !(tmp & 0xFF));
                        if (getstat(DECIMAL)) {
                                if ((ac & 0xF) + (m & 0xF) + getstat(CARRY) > 9) tmp+=6;
                                setstat(NEGATIVE, tmp & 0x80);
                                setstat(OVERFLOW, !((ac ^ m) & 0x80) && ((ac ^ tmp) & 0x80));
                                if (tmp > 0x99) tmp += 96;
                                setstat(CARRY, tmp > 0x99);
                        } else {
                                setstat(NEGATIVE, tmp & 0x80);
                                setstat(OVERFLOW, !((ac ^ m) & 0x80) && ((ac ^ tmp) & 0x80));
                                setstat(CARRY, tmp > 0xFF);
                        }
                        ac = tmp & 0xFF;
                        return;
                case AND:
                        ac = ac & memory[addr];
                        setstat(NEGATIVE, ac & 0x80);
                        setstat(ZERO, !ac);
                        return;
                case ASC:
                        setstat(CARRY, ac & 0x80);
                        ac <<= 1;
                        setstat(ZERO, !ac);
                        setstat(NEGATIVE, ac & 0x80);
                        return;
                case ASL:
                        setstat(CARRY, memory[addr] & 0x80);
                        memory[addr] <<= 1;
                        setstat(ZERO, !memory[addr]);
                        setstat(NEGATIVE, memory[addr] & 0x80);
                        return;
                case BCC:
                        if(!getstat(CARRY))
                                pc = addr;
                        return;
                case BCS:
                        if(getstat(CARRY))
                                pc = addr;
                        return;
                case BEQ:
                        if(getstat(ZERO))
                                pc = addr;
                        return;
                case BIT:
                        setstat(NEGATIVE, memory[addr]);
                        setstat(OVERFLOW, memory[addr] & OVERFLOW);
                        setstat(ZERO, ac & memory[addr]);
                        return;
                case BMI:
                        if(getstat(NEGATIVE))
                                pc = addr;
                        return;
                case BNE:
                        if(!getstat(ZERO))
                                pc = addr;
                        return;
                case BPL:
                        if(!getstat(NEGATIVE))
                                pc = addr;
                        return;
                case BRK:
                        pc++;
                        hws_push((pc >> 8) & 0xFF);
                        hws_push(pc & 0xFF);
                        setstat(BREAK, 1);
                        hws_push(sr);
                        setstat(INTERRUPT, 1);
                        pc = memory[0xFFFE] + (memory[0xFFFF] << 8);
                        return;
                case BVC:
                        if(!getstat(OVERFLOW))
                                pc = addr;
                        return;
                case BVS:
                        if(getstat(OVERFLOW))
                                pc = addr;
                        return;
                case BRA:
                        pc = addr;
                        return;
                case CLC:
                        cbi(sr, CARRY);
                        return;
                case CLD:
                        cbi(sr, DECIMAL);
                        return;
                case CLI:
                        cbi(sr, INTERRUPT);
                        return;
                case CLV:
                        cbi(sr, OVERFLOW);
                        return;
                case CMP:
                        tmp = ac + memory[addr];
                        setstat(ZERO, !(tmp & 0xFF));
                        setstat(CARRY, tmp < 0x100);
                        setstat(NEGATIVE, tmp & 0x80);
                        return;
                case CPX:
                        tmp = x + memory[addr];
                        setstat(ZERO, !(tmp & 0xFF));
                        setstat(CARRY, tmp < 0x100);
                        setstat(NEGATIVE, tmp & 0x80);
                        return;
                case CPY:
                        tmp = y + memory[addr];
                        setstat(ZERO, !(tmp & 0xFF));
                        setstat(CARRY, tmp < 0x100);
                        setstat(NEGATIVE, tmp & 0x80);
                        return;
                case DEC:
                        memory[addr]--;
                        setstat(NEGATIVE, memory[addr] & 0x80);
                        setstat(ZERO, !memory[addr]);
                        return;
                case DEA:
                        ac--;
                        setstat(NEGATIVE, ac & 0x80);
                        setstat(ZERO, !ac);
                        return;
                case DEX:
                        x--;
                        setstat(NEGATIVE, x & 0x80);
                        setstat(ZERO, !x);
                        return;
                case DEY:
                        y--;
                        setstat(NEGATIVE, y & 0x80);
                        setstat(ZERO, !y);
                        return;
                case EOR:
                        ac ^= memory[addr];
                        setstat(NEGATIVE, ac & 0x80);
                        setstat(ZERO, !ac);
                        return;
                case INC:
                        memory[addr]++;
                        setstat(NEGATIVE, memory[addr] & 0x80);
                        setstat(ZERO, !memory[addr]);
                        return;
                case INA:
                        ac++;
                        setstat(NEGATIVE, ac & 0x80);
                        setstat(ZERO, !ac);
                        return;
                case INX:
                        x++;
                        setstat(NEGATIVE, x & 0x80);
                        setstat(ZERO, !x);
                        return;
                case INY:
                        y++;
                        setstat(NEGATIVE, y & 0x80);
                        setstat(ZERO, !y);
                        return;
                case JMP:
                        pc = addr;
                        return;
                case JSR:
                        pc--;
                        hws_push((pc >> 8) & 0xFF);
                        hws_push(pc & 0xFF);
                        pc = addr;
                        return;
                case LDA:
                        ac = memory[addr];
                        setstat(NEGATIVE, ac & 0x80);
                        setstat(ZERO, !ac);
                        return;
                case LDX:
                        x = memory[addr];
                        setstat(NEGATIVE, x & 0x80);
                        setstat(ZERO, !x);
                        return;
                case LDY:
                        y = memory[addr];
                        setstat(NEGATIVE, y & 0x80);
                        setstat(ZERO, !y);
                        return;
                case LSC:
                        setstat(CARRY, ac & 0x01);
                        ac >>= 1;
                        setstat(NEGATIVE, ac & 0x80);
                        setstat(ZERO, !ac);
                        return;
                case LSR:
                        setstat(CARRY, memory[addr] & 0x01);
                        memory[addr] >>= 1;
                        setstat(NEGATIVE, memory[addr] & 0x80);
                        setstat(ZERO, !memory[addr]);
                        return;
                case NOP:
                        return;
                case ORA:
                        ac |= memory[addr];
                        setstat(NEGATIVE, ac & 0x80);
                        setstat(ZERO, !ac);
                        return;
                case PHA:
                        hws_push(ac);
                        return;
                case PHX:
                        hws_push(x);
                        return;
                case PHY:
                        hws_push(y);
                        return;
                case PHP:
                        hws_push(sr);
                        return;
                case PLA:
                        ac = hws_pop();
                        setstat(NEGATIVE, ac & 0x80);
                        setstat(ZERO, !ac);
                        return;
                case PLX:
                        x = hws_pop();
                        setstat(NEGATIVE, x & 0x80);
                        setstat(ZERO, !x);
                        return;
                case PLY:
                        y = hws_pop();
                        setstat(NEGATIVE, y & 0x80);
                        setstat(ZERO, !y);
                        return;
                case PLP:
                        sr = hws_pop();
                        return;
                case ROL:
                        tmp = memory[addr];
                        tmp <<= 1;
                        tmp |= getstat(CARRY);
                        setstat(CARRY, tmp > 0xFF);
                        setstat(NEGATIVE, tmp & 0x80);
                        setstat(ZERO, !tmp);
                        memory[addr] = tmp;
                        return;
                case RLC:
                        tmp = ac;
                        tmp <<= 1;
                        tmp |= getstat(CARRY);
                        setstat(CARRY, tmp > 0xFF);
                        setstat(NEGATIVE, tmp & 0x80);
                        setstat(ZERO, !(tmp & 0xFF));
                        ac = tmp & 0xFF;
                        return;
                case ROR:
                        tmp = memory[addr];
                        tmp |= getstat(CARRY) << 8;
                        setstat(CARRY, tmp & 0x01);
                        tmp >>= 1;
                        setstat(NEGATIVE, tmp & 0x80);
                        setstat(ZERO, !(tmp & 0xFF));
                        memory[addr] = tmp;
                        return;
                case RRC:
                        tmp = ac;
                        tmp |= getstat(CARRY) << 8;
                        setstat(CARRY, tmp & 0x01);
                        tmp >>= 1;
                        setstat(NEGATIVE, tmp & 0x80);
                        setstat(ZERO, !(tmp & 0xFF));
                        ac = tmp;
                        return;
                case RTI:
                        sr = hws_pop();
                        pc = hws_pop();
                        pc += hws_pop() << 8;
                        return;
                case RTS:
                        pc = hws_pop();
                        pc += hws_pop() << 8;
                        pc++;
                        return;
                case SBC:
                        m = memory[addr];
                        tmp = ac - m - (getstat(CARRY) ? 0 : 1);
                        setstat(NEGATIVE, tmp & 0x80);
                        setstat(ZERO, !(tmp & 0xFF));
                        setstat(OVERFLOW, ((ac ^ m) & 0x80) && ((ac ^ tmp) & 0x80));
                        if(getstat(DECIMAL)) {
                                if (((ac & 0x0F) - (getstat(CARRY) ? 0 : 1)) < (m & 0x0F))
                                        tmp -= 6;
                                if (tmp > 0x99)
                                        tmp -= 0x60;
                        }
                        setstat(CARRY, tmp < 0x100);
                        ac = tmp & 0xFF;
                        return;
                case SEC:
                        sbi(sr, CARRY);
                        return;
                case SED:
                        sbi(sr, DECIMAL);
                        return;
                case SEI:
                        sbi(sr, INTERRUPT);
                        return;
                case STA:
                        memory[addr] = ac;
                        return;
                case STX:
                        memory[addr] = x;
                        return;
                case STZ:
                        memory[addr] = 0x00;
                        return;
                case STY:
                        memory[addr] = y;
                        return;
                case TAX:
                        x = ac;
                        setstat(NEGATIVE, ac & 0x80);
                        setstat(ZERO, !ac);
                        return;
                case TAY:
                        y = ac;
                        setstat(NEGATIVE, ac & 0x80);
                        setstat(ZERO, !ac);
                        return;
                case TSX:
                        x = sp;
                        setstat(NEGATIVE, x & 0x80);
                        setstat(ZERO, !x);
                        return;
                case TXA:
                        ac = x;
                        setstat(NEGATIVE, ac & 0x80);
                        setstat(ZERO, !ac);
                        return;
                case TXS:
                        sp = x;
                        setstat(NEGATIVE, x & 0x80);
                        setstat(ZERO, !x);
                        return;
                case TYA:
                        ac = y;
                        setstat(NEGATIVE, ac & 0x80);
                        setstat(ZERO, !ac);
                        return;
                case TRB:
                        memory[addr] &= ~ac;
                        setstat(ZERO, ac & memory[addr]);
                        return;
                case TSB:
                        memory[addr] |= ac;
                        setstat(ZERO, ac & memory[addr]);
                        return;
                case INV:
                        invalid_instruction = pc;
                        return;
        }
}

uint16_t
cycle(void)
{
        Opcode op;
        op = opc_table[memory[pc++]];
        run_op(op);
        return invalid_instruction;
}

void
reset_cpu(void)
{
        pc = (memory[rstH] << 8) + memory[rstL];
        ac = 0x00;
        x = 0x00;
        y = 0x00;
        sp = 0xFD; /* http://mametesters.org/view.php?id=6486 */
        sr = 0x00;
        sbi(sr, _IGNORE);
        return;

}

void
irq(void)
{
        if(!getstat(INTERRUPT)) {
                hws_push((pc >> 8) & 0xFF);
                hws_push(pc & 0xFF);
                hws_push(sr);
                cbi(sr, BREAK);
                sbi(sr, INTERRUPT);
                pc = (memory[irqH] << 8) + memory[irqL];
        }
        return;
}

void
nmi(void)
{
        hws_push((pc >> 8) & 0xFF);
        hws_push(pc & 0xFF);
        hws_push(sr);
        cbi(sr, BREAK);
        sbi(sr, INTERRUPT);
        pc = (memory[nmiH] << 8) + memory[nmiL];
        return;
}
