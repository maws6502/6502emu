/* emu.c
 * 6502emu
 * Aditya Goturu <aditya@sudoforlunch.org>
 */

#include <stdint.h>
#include "cpu.h"
#include "cpu_intrn.h"

static uint16_t pc;
static uint8_t ac, x, y, sr, sp;


/* acquired from http://e-tradition.net/bytes/6502/6502_instruction_set.html
 * http://www.6502.org/tutorials/65c02opcodes.html#2 
 * http://www.llx.com/~nparker/a2/opcodes.html */ 
static Opcode emu65_opc_table[0x100] = {
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
emu65_hws_push(Emu65Device *dev, uint8_t val)
{
        dev->memory[0x100 + dev->sp] = val;
        if (dev->sp)
                dev->sp--;
        else dev->sp = 0xFF;
        return;
}

static uint8_t
emu65_hws_pop(Emu65Device *dev)
{
       if (dev->sp == 0xFF) dev->sp = 0x0;
       else dev->sp++;
       return dev->memory[0x100 + dev->sp];
}

static uint16_t
emu65_get_addr(Emu65Device *dev, int adm)
{
        uint8_t low, high, efflow, effhigh;
        int8_t offset;
        switch (adm) {
                case A_ABS:
                        low = dev->memory[dev->pc++];
                        high = dev->memory[dev->pc++];
                        return low + (high << 8);
                case A_ABX:
                        low = dev->memory[dev->pc++];
                        high = dev->memory[dev->pc++];
                        return low + (high << 8) + dev->x;
                case A_ABY:
                        low = dev->memory[dev->pc++];
                        high = dev->memory[dev->pc++];
                        return low + (high << 8) + dev->y;
                case A_IMM:
                        return dev->pc++;
                case A_IND:
                        low = dev->memory[dev->pc++];
                        high = dev->memory[dev->pc++];
                        efflow = dev->memory[low + (high << 8)];
                        effhigh = dev->memory[low + (high << 8) + 1];
                        return efflow + (effhigh << 8);
                case A_IAX:
                        low = dev->memory[dev->pc++];
                        high = dev->memory[dev->pc++];
                        efflow = dev->memory[low + (high << 8) + dev->x];
                        effhigh = dev->memory[low + (high << 8) + 1 + dev->x];
                        return efflow + (effhigh << 8);
                case A_INX:
                        low = (dev->memory[dev->pc++] + dev->x) % 0xFF;
                        high = ++low % 0xFF;
                        return dev->memory[low] + (dev->memory[high] << 8);
                case A_INY:
                        low = dev->memory[dev->pc++];
                        high = ++low % 0xFF;
                        return dev->memory[low] + (dev->memory[high] << 8) + dev->y;
                case A_INZ:
                        low = dev->memory[dev->pc++];
                        high = ++low % 0xFF;
                        return dev->memory[low] + (dev->memory[high] << 8);
                case A_REL:
                        offset = (int8_t) dev->memory[dev->pc++];
                        return dev->pc + offset;
                case A_ZPG:
                        return dev->memory[dev->pc++];
                case A_ZPX:
                        return (dev->memory[dev->pc++] + dev->x) % 0xFF;
                case A_ZPY:
                        return (dev->memory[dev->pc++] + dev->y) % 0xFF;
                case A_ACC:
                case A_IMP:
                default:
                        return 0;
        }
}

static int
emu65_run_op(Emu65Device *dev, Opcode op)
{
        uint16_t addr = emu65_get_addr(dev, op.adm);
        uint8_t m;
        unsigned int tmp;
        switch (op.op){
                case ADC:
                        /* stolen from https://github.com/gianlucag/mos6502 because */
                        m = dev->memory[addr];
                        tmp = m + dev->ac + getstat(CARRY);
                        setstat(ZERO, !(tmp & 0xFF));
                        if (getstat(DECIMAL)) {
                                if ((dev->ac & 0xF) + (m & 0xF) + getstat(CARRY) > 9) tmp+=6;
                                setstat(NEGATIVE, tmp & 0x80);
                                setstat(OVERFLOW, !((dev->ac ^ m) & 0x80) && ((dev->ac ^ tmp) & 0x80));
                                if (tmp > 0x99) tmp += 96;
                                setstat(CARRY, tmp > 0x99);
                        } else {
                                setstat(NEGATIVE, tmp & 0x80);
                                setstat(OVERFLOW, !((dev->ac ^ m) & 0x80) && ((dev->ac ^ tmp) & 0x80));
                                setstat(CARRY, tmp > 0xFF);
                        }
                        dev->ac = tmp & 0xFF;
                        break;
                case AND:
                        dev->ac &= dev->memory[addr];
                        setstat(NEGATIVE, dev->ac & 0x80);
                        setstat(ZERO, !dev->ac);
                        break;
                case ASC:
                        setstat(CARRY, dev->ac & 0x80);
                        dev->ac <<= 1;
                        setstat(ZERO, !dev->ac);
                        setstat(NEGATIVE, dev->ac & 0x80);
                        break;
                case ASL:
                        setstat(CARRY, dev->memory[addr] & 0x80);
                        dev->memory[addr] <<= 1;
                        setstat(ZERO, !dev->memory[addr]);
                        setstat(NEGATIVE, dev->memory[addr] & 0x80);
                        break;
                case BCC:
                        if(!getstat(CARRY))
                                dev->pc = addr;
                        break;
                case BCS:
                        if(getstat(CARRY))
                                dev->pc = addr;
                        break;
                case BEQ:
                        if(getstat(ZERO))
                                dev->pc = addr;
                        break;
                case BIT:
                        setstat(NEGATIVE, dev->memory[addr]);
                        setstat(OVERFLOW, dev->memory[addr] & OVERFLOW);
                        setstat(ZERO, dev->ac & dev->memory[addr]);
                        break;
                case BMI:
                        if(getstat(NEGATIVE))
                                dev->pc = addr;
                        break;
                case BNE:
                        if(!getstat(ZERO))
                                dev->pc = addr;
                        break;
                case BPL:
                        if(!getstat(NEGATIVE))
                                dev->pc = addr;
                        break;
                case BRK:
                        dev->pc++;
                        emu65_hws_push(dev, (dev->pc >> 8) & 0xFF);
                        emu65_hws_push(dev, dev->pc & 0xFF);
                        setstat(BREAK, 1);
                        emu65_hws_push(dev, dev->sr);
                        setstat(INTERRUPT, 1);
                        dev->pc = dev->memory[0xFFFE] + (dev->memory[0xFFFF] << 8);
                        break;
                case BVC:
                        if(!getstat(OVERFLOW))
                                dev->pc = addr;
                        break;
                case BVS:
                        if(getstat(OVERFLOW))
                                dev->pc = addr;
                        break;
                case BRA:
                        dev->pc = addr;
                        break;
                case CLC:
                        cbi(dev->sr, CARRY);
                        break;
                case CLD:
                        cbi(dev->sr, DECIMAL);
                        break;
                case CLI:
                        cbi(dev->sr, INTERRUPT);
                        break;
                case CLV:
                        cbi(dev->sr, OVERFLOW);
                        break;
                case CMP:
                        tmp = dev->ac + dev->memory[addr];
                        setstat(ZERO, !(tmp & 0xFF));
                        setstat(CARRY, tmp < 0x100);
                        setstat(NEGATIVE, tmp & 0x80);
                        break;
                case CPX:
                        tmp = dev->x + dev->memory[addr];
                        setstat(ZERO, !(tmp & 0xFF));
                        setstat(CARRY, tmp < 0x100);
                        setstat(NEGATIVE, tmp & 0x80);
                        break;
                case CPY:
                        tmp = dev->y + dev->memory[addr];
                        setstat(ZERO, !(tmp & 0xFF));
                        setstat(CARRY, tmp < 0x100);
                        setstat(NEGATIVE, tmp & 0x80);
                        break;
                case DEC:
                        dev->memory[addr]--;
                        setstat(NEGATIVE, dev->memory[addr] & 0x80);
                        setstat(ZERO, !dev->memory[addr]);
                        break;
                case DEA:
                        dev->ac--;
                        setstat(NEGATIVE, dev->ac & 0x80);
                        setstat(ZERO, !dev->ac);
                        break;
                case DEX:
                        dev->x--;
                        setstat(NEGATIVE, dev->x & 0x80);
                        setstat(ZERO, !dev->x);
                        break;
                case DEY:
                        dev->y--;
                        setstat(NEGATIVE, dev->y & 0x80);
                        setstat(ZERO, !dev->y);
                        break;
                case EOR:
                        dev->ac ^= dev->memory[addr];
                        setstat(NEGATIVE, dev->ac & 0x80);
                        setstat(ZERO, !dev->ac);
                        break;
                case INC:
                        dev->memory[addr]++;
                        setstat(NEGATIVE, dev->memory[addr] & 0x80);
                        setstat(ZERO, !dev->memory[addr]);
                        break;
                case INA:
                        dev->ac++;
                        setstat(NEGATIVE, dev->ac & 0x80);
                        setstat(ZERO, !dev->ac);
                        break;
                case INX:
                        dev->x++;
                        setstat(NEGATIVE, dev->x & 0x80);
                        setstat(ZERO, !dev->x);
                        break;
                case INY:
                        dev->y++;
                        setstat(NEGATIVE, dev->y & 0x80);
                        setstat(ZERO, !dev->y);
                        break;
                case JMP:
                        dev->pc = addr;
                        break;
                case JSR:
                        dev->pc--;
                        emu65_hws_push(dev, (dev->pc >> 8) & 0xFF);
                        emu65_hws_push(dev, dev->pc & 0xFF);
                        dev->pc = addr;
                        break;
                case LDA:
                        dev->ac = dev->memory[addr];
                        setstat(NEGATIVE, dev->ac & 0x80);
                        setstat(ZERO, !dev->ac);
                        break;
                case LDX:
                        dev->x = dev->memory[addr];
                        setstat(NEGATIVE, dev->x & 0x80);
                        setstat(ZERO, !dev->x);
                        break;
                case LDY:
                        dev->y = dev->memory[addr];
                        setstat(NEGATIVE, dev->y & 0x80);
                        setstat(ZERO, !dev->y);
                        break;
                case LSC:
                        setstat(CARRY, dev->ac & 0x01);
                        dev->ac >>= 1;
                        setstat(NEGATIVE, dev->ac & 0x80);
                        setstat(ZERO, !dev->ac);
                        break;
                case LSR:
                        setstat(CARRY, dev->memory[addr] & 0x01);
                        dev->memory[addr] >>= 1;
                        setstat(NEGATIVE, dev->memory[addr] & 0x80);
                        setstat(ZERO, !dev->memory[addr]);
                        break;
                case NOP:
                        break;
                case ORA:
                        dev->ac |= dev->memory[addr];
                        setstat(NEGATIVE, dev->ac & 0x80);
                        setstat(ZERO, !dev->ac);
                        break;
                case PHA:
                        emu65_hws_push(dev, dev->ac);
                        break;
                case PHX:
                        emu65_hws_push(dev, dev->x);
                        break;
                case PHY:
                        emu65_hws_push(dev, dev->y);
                        break;
                case PHP:
                        emu65_hws_push(dev, dev->sr);
                        break;
                case PLA:
                        dev->ac = emu65_hws_pop(dev);
                        setstat(NEGATIVE, dev->ac & 0x80);
                        setstat(ZERO, !dev->ac);
                        break;
                case PLX:
                        dev->x = emu65_hws_pop(dev);
                        setstat(NEGATIVE, dev->x & 0x80);
                        setstat(ZERO, !dev->x);
                        break;
                case PLY:
                        dev->y = emu65_hws_pop(dev);
                        setstat(NEGATIVE, dev->y & 0x80);
                        setstat(ZERO, !dev->y);
                        break;
                case PLP:
                        dev->sr = emu65_hws_pop(dev);
                        break;
                case ROL:
                        tmp = dev->memory[addr];
                        tmp <<= 1;
                        tmp |= getstat(CARRY);
                        setstat(CARRY, tmp > 0xFF);
                        setstat(NEGATIVE, tmp & 0x80);
                        setstat(ZERO, !tmp);
                        dev->memory[addr] = tmp;
                        break;
                case RLC:
                        tmp = dev->ac;
                        tmp <<= 1;
                        tmp |= getstat(CARRY);
                        setstat(CARRY, tmp > 0xFF);
                        setstat(NEGATIVE, tmp & 0x80);
                        setstat(ZERO, !(tmp & 0xFF));
                        dev->ac = tmp & 0xFF;
                        break;
                case ROR:
                        tmp = dev->memory[addr];
                        tmp |= getstat(CARRY) << 8;
                        setstat(CARRY, tmp & 0x01);
                        tmp >>= 1;
                        setstat(NEGATIVE, tmp & 0x80);
                        setstat(ZERO, !(tmp & 0xFF));
                        dev->memory[addr] = tmp;
                        break;
                case RRC:
                        tmp = dev->ac;
                        tmp |= getstat(CARRY) << 8;
                        setstat(CARRY, tmp & 0x01);
                        tmp >>= 1;
                        setstat(NEGATIVE, tmp & 0x80);
                        setstat(ZERO, !(tmp & 0xFF));
                        dev->ac = tmp;
                        break;
                case RTI:
                        dev->sr = emu65_hws_pop(dev);
                        dev->pc = emu65_hws_pop(dev);
                        dev->pc += emu65_hws_pop(dev) << 8;
                        break;
                case RTS:
                        dev->pc = emu65_hws_pop(dev);
                        dev->pc += emu65_hws_pop(dev) << 8;
                        dev->pc++;
                        break;
                case SBC:
                        m = dev->memory[addr];
                        tmp = dev->ac - m - (getstat(CARRY) ? 0 : 1);
                        setstat(NEGATIVE, tmp & 0x80);
                        setstat(ZERO, !(tmp & 0xFF));
                        setstat(OVERFLOW, ((dev->ac ^ m) & 0x80) && ((dev->ac ^ tmp) & 0x80));
                        if(getstat(DECIMAL)) {
                                if (((dev->ac & 0x0F) - (getstat(CARRY) ? 0 : 1)) < (m & 0x0F))
                                        tmp -= 6;
                                if (tmp > 0x99)
                                        tmp -= 0x60;
                        }
                        setstat(CARRY, tmp < 0x100);
                        dev->ac = tmp & 0xFF;
                        break;
                case SEC:
                        sbi(dev->sr, CARRY);
                        break;
                case SED:
                        sbi(dev->sr, DECIMAL);
                        break;
                case SEI:
                        sbi(dev->sr, INTERRUPT);
                        break;
                case STA:
                        dev->memory[addr] = dev->ac;
                        break;
                case STX:
                        dev->memory[addr] = dev->x;
                        break;
                case STZ:
                        dev->memory[addr] = 0x00;
                        break;
                case STY:
                        dev->memory[addr] = dev->y;
                        break;
                case TAX:
                        dev->x = dev->ac;
                        setstat(NEGATIVE, dev->ac & 0x80);
                        setstat(ZERO, !dev->ac);
                        break;
                case TAY:
                        dev->y = dev->ac;
                        setstat(NEGATIVE, dev->ac & 0x80);
                        setstat(ZERO, !dev->ac);
                        break;
                case TSX:
                        dev->x = dev->sp;
                        setstat(NEGATIVE, dev->x & 0x80);
                        setstat(ZERO, !dev->x);
                        break;
                case TXA:
                        dev->ac = dev->x;
                        setstat(NEGATIVE, dev->ac & 0x80);
                        setstat(ZERO, !dev->ac);
                        break;
                case TXS:
                        dev->sp = dev->x;
                        setstat(NEGATIVE, dev->x & 0x80);
                        setstat(ZERO, !dev->x);
                        break;
                case TYA:
                        dev->ac = dev->y;
                        setstat(NEGATIVE, dev->ac & 0x80);
                        setstat(ZERO, !dev->ac);
                        break;
                case TRB:
                        dev->memory[addr] &= ~dev->ac;
                        setstat(ZERO, dev->ac & dev->memory[addr]);
                        break;
                case TSB:
                        dev->memory[addr] |= dev->ac;
                        setstat(ZERO, dev->ac & dev->memory[addr]);
                        break;
                case INV:
                        return 0;
        }
        return 1;
}

uint16_t
emu65_cycle(Emu65Device *dev)
{
        Opcode op;
        op = emu65_opc_table[dev->memory[dev->pc++]];
        if(emu65_run_op(dev, op)) return 0;
        else return dev->pc;
}

void
emu65_reset(Emu65Device *dev)
{
        dev->pc = (dev->memory[rstH] << 8) + dev->memory[rstL];
        dev->ac = 0x00;
        dev->x = 0x00;
        dev->y = 0x00;
        dev->sp = 0xFD; /* http://mametesters.org/view.php?id=6486 */
        dev->sr = 0x00;
        sbi(dev->sr, _IGNORE);
        return;

}

void
emu65_irq(Emu65Device *dev)
{
        if(!getstat(INTERRUPT)) {
                emu65_hws_push(dev, (dev->pc >> 8) & 0xFF);
                emu65_hws_push(dev, dev->pc & 0xFF);
                emu65_hws_push(dev, dev->sr);
                cbi(dev->sr, BREAK);
                sbi(dev->sr, INTERRUPT);
                pc = (dev->memory[irqH] << 8) + dev->memory[irqL];
        }
        return;
}

void
emu65_nmi(Emu65Device *dev)
{
        emu65_hws_push(dev, (dev->pc >> 8) & 0xFF);
        emu65_hws_push(dev, dev->pc & 0xFF);
        emu65_hws_push(dev, dev->sr);
        cbi(dev->sr, BREAK);
        sbi(dev->sr, INTERRUPT);
        pc = (dev->memory[nmiH] << 8) + dev->memory[nmiL];
        return;
}
