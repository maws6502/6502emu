/* emu.c
 * 6502emu
 * Aditya Goturu <aditya@sudoforlunch.org>
 */

#include <stdint.h>
#include "cpu.h"
#include "cpu_intrn.h"

/* acquired from http://e-tradition.net/bytes/6502/6502_instruction_set.html
 * http://www.6502.org/tutorials/65c02opcodes.html#2 
 * http://www.llx.com/~nparker/a2/opcodes.html */ 
static Opcode emu65_opc_table[0x100] = {
        {BRK, A_IMP}, {ORA, A_INX}, {NO2, A_IMP}, {NO1, A_IMP}, {TSB, A_ZPG}, {ORA, A_ZPG}, {ASL, A_ZPG}, {RM0, A_ZPG}, {PHP, A_IMP}, {ORA, A_IMM}, {ASC, A_IMP}, {NO1, A_IMP}, {TSB, A_ABS}, {ORA, A_ABS}, {ASL, A_ABS}, {BR0, A_ZPR},
        {BPL, A_REL}, {ORA, A_INY}, {ORA, A_INZ}, {NO1, A_IMP}, {TRB, A_ZPG}, {ORA, A_ZPX}, {ASL, A_ZPX}, {RM1, A_ZPG}, {CLC, A_IMP}, {ORA, A_ABY}, {INA, A_IMP}, {NO1, A_IMP}, {TRB, A_ABS}, {ORA, A_ABX}, {ASL, A_ABX}, {BR1, A_ZPR}, 
        {JSR, A_ABS}, {AND, A_INX}, {NO2, A_IMP}, {NO1, A_IMP}, {BIT, A_ZPG}, {AND, A_ZPG}, {ROL, A_ZPG}, {RM2, A_ZPG}, {PLP, A_IMP}, {AND, A_IMM}, {RLC, A_IMP}, {NO1, A_IMP}, {BIT, A_ABS}, {AND, A_ABS}, {ROL, A_ABS}, {BR2, A_ZPR}, 
        {BMI, A_REL}, {AND, A_INY}, {AND, A_INZ}, {NO1, A_IMP}, {BIT, A_ZPX}, {AND, A_ZPX}, {ROL, A_ZPX}, {RM3, A_ZPG}, {SEC, A_IMP}, {AND, A_ABY}, {DEA, A_IMP}, {NO1, A_IMP}, {BIT, A_ABX}, {AND, A_ABX}, {ROL, A_ABX}, {BR3, A_ZPR},
        {RTI, A_IMP}, {EOR, A_INX}, {NO2, A_IMP}, {NO1, A_IMP}, {NO2, A_IMP}, {EOR, A_ZPG}, {LSR, A_ZPG}, {RM4, A_ZPG}, {PHA, A_IMP}, {EOR, A_IMM}, {LSC, A_IMP}, {NO1, A_IMP}, {JMP, A_ABS}, {EOR, A_ABS}, {LSR, A_ABS}, {BR4, A_ZPR},
        {BVC, A_REL}, {EOR, A_INY}, {EOR, A_INZ}, {NO1, A_IMP}, {NO2, A_IMP}, {EOR, A_ZPX}, {LSR, A_ZPX}, {RM5, A_ZPG}, {CLI, A_IMP}, {EOR, A_ABY}, {PHY, A_IMP}, {NO1, A_IMP}, {NO3, A_IMP}, {EOR, A_ABX}, {LSR, A_ABX}, {BR5, A_ZPR}, 
        {RTS, A_IMP}, {ADC, A_INX}, {NO2, A_IMP}, {NO1, A_IMP}, {STZ, A_ZPG}, {ADC, A_ZPG}, {ROR, A_ZPG}, {RM6, A_ZPG}, {PLA, A_IMP}, {ADC, A_IMM}, {RRC, A_IMP}, {NO1, A_IMP}, {JMP, A_IND}, {ADC, A_ABS}, {ROR, A_ABS}, {BR6, A_ZPR},
        {BVS, A_REL}, {ADC, A_INY}, {ADC, A_INZ}, {NO1, A_IMP}, {STZ, A_ZPX}, {ADC, A_ZPX}, {ROR, A_ZPX}, {RM7, A_ZPG}, {SEI, A_IMP}, {ADC, A_ABY}, {PLY, A_IMP}, {NO1, A_IMP}, {JMP, A_IAX}, {ADC, A_ABX}, {ROR, A_ABX}, {BR7, A_ZPR}, 
        {BRA, A_REL}, {STA, A_INX}, {NO2, A_IMP}, {NO1, A_IMP}, {STY, A_ZPG}, {STA, A_ZPG}, {STX, A_ZPG}, {SM0, A_ZPG}, {DEY, A_IMP}, {BIT, A_IMM}, {TXA, A_IMP}, {NO1, A_IMP}, {STY, A_ABS}, {STA, A_ABS}, {STX, A_ABS}, {BS0, A_ZPR}, 
        {BCC, A_REL}, {STA, A_INY}, {STA, A_INZ}, {NO1, A_IMP}, {STY, A_ZPX}, {STA, A_ZPX}, {STX, A_ZPY}, {SM1, A_ZPG}, {TYA, A_IMP}, {STA, A_ABY}, {TXS, A_IMP}, {NO1, A_IMP}, {STZ, A_ABS}, {STA, A_ABX}, {STZ, A_ABX}, {BS1, A_ZPR}, 
        {LDY, A_IMM}, {LDA, A_INX}, {LDX, A_IMM}, {NO1, A_IMP}, {LDY, A_ZPG}, {LDA, A_ZPG}, {LDX, A_ZPG}, {SM2, A_ZPG}, {TAY, A_IMP}, {LDA, A_IMM}, {TAX, A_IMP}, {NO1, A_IMP}, {LDY, A_ABS}, {LDA, A_ABS}, {LDX, A_ABS}, {BS2, A_ZPR},
        {BCS, A_REL}, {LDA, A_INY}, {LDA, A_INZ}, {NO1, A_IMP}, {LDY, A_ZPX}, {LDA, A_ZPX}, {LDX, A_ZPY}, {SM3, A_ZPG}, {CLV, A_IMP}, {LDA, A_ABY}, {TSX, A_IMP}, {NO1, A_IMP}, {LDY, A_ABX}, {LDA, A_ABX}, {LDX, A_ABY}, {BS3, A_ZPR}, 
        {CPY, A_IMM}, {CMP, A_INX}, {NO2, A_IMP}, {NO1, A_IMP}, {CPY, A_ZPG}, {CMP, A_ZPG}, {DEC, A_ZPG}, {SM4, A_ZPG}, {INY, A_IMP}, {CMP, A_IMM}, {DEX, A_IMP}, {NO1, A_IMP}, {CPY, A_ABS}, {CMP, A_ABS}, {DEC, A_ABS}, {BS4, A_ZPR},
        {BNE, A_REL}, {CMP, A_INY}, {CMP, A_INZ}, {NO1, A_IMP}, {NO2, A_IMP}, {CMP, A_ZPX}, {DEC, A_ZPX}, {SM5, A_ZPG}, {CLD, A_IMP}, {CMP, A_ABY}, {PHX, A_IMP}, {NO1, A_IMP}, {NO3, A_IMP}, {CMP, A_ABX}, {DEC, A_ABX}, {BS5, A_ZPR}, 
        {CPX, A_IMM}, {SBC, A_INX}, {NO2, A_IMP}, {NO1, A_IMP}, {CPX, A_ZPG}, {SBC, A_ZPG}, {INC, A_ZPG}, {SM6, A_ZPG}, {INX, A_IMP}, {SBC, A_IMM}, {NO1, A_IMP}, {NO1, A_IMP}, {CPX, A_ABS}, {SBC, A_ABS}, {INC, A_ABS}, {BS6, A_ZPR},
        {BEQ, A_REL}, {SBC, A_INY}, {SBC, A_INZ}, {NO1, A_IMP}, {NO2, A_IMP}, {SBC, A_ZPX}, {INC, A_ZPX}, {SM7, A_ZPG}, {SED, A_IMP}, {SBC, A_ABY}, {PLX, A_IMP}, {NO1, A_IMP}, {NO3, A_IMP}, {SBC, A_ABX}, {INC, A_ABX}, {BS7, A_ZPR}
};



static void
emu65_hws_push(Emu65Device *dev, uint8_t val)
{
        dev->memwrite(dev->param, 0x100 + dev->sp, val);
        if(dev->sp)
                dev->sp--;
        else dev->sp = 0xFF;
        return;
}

static uint8_t
emu65_hws_pop(Emu65Device *dev)
{
       if(dev->sp == 0xFF) dev->sp = 0x0;
       else dev->sp++;
       return dev->memread(dev->param, 0x100 + dev->sp);
}

static uint16_t
emu65_get_addr(Emu65Device *dev, int adm)
{
        uint16_t low, high, efflow, effhigh;
        int8_t offset;
        switch (adm) {
                case A_ZPR:
                case A_ABS:
                        low = dev->memread(dev->param, dev->pc++);
                        high = dev->memread(dev->param, dev->pc++);
                        return low | (high << 8);
                case A_ABX:
                        low = dev->memread(dev->param, dev->pc++);
                        high = dev->memread(dev->param, dev->pc++);
                        return (low | (high << 8)) + dev->x;
                case A_ABY:
                        low = dev->memread(dev->param, dev->pc++);
                        high = dev->memread(dev->param, dev->pc++);
                        return (low | (high << 8)) + dev->y;
                case A_IMM:
                        return dev->pc++;
                case A_IND:
                        low = dev->memread(dev->param, dev->pc++);
                        high = dev->memread(dev->param, dev->pc++);
                        efflow = dev->memread(dev->param, low | (high << 8));
                        effhigh = dev->memread(dev->param, (low | (high << 8)) + 1);
                        return efflow | (effhigh << 8);
                case A_IAX:
                        low = dev->memread(dev->param, dev->pc++);
                        high = dev->memread(dev->param, dev->pc++);
                        efflow = dev->memread(dev->param, (low | (high << 8)) + dev->x);
                        effhigh = dev->memread(dev->param, (low | (high << 8)) + 1 + dev->x);
                        return efflow | (effhigh << 8);
                case A_INX:
                        low = (dev->memread(dev->param, dev->pc++) + dev->x) & 0x00FF;
                        high = low + 1 & 0x00FF;
                        return dev->memread(dev->param, low) | (dev->memread(dev->param, high) << 8);
                case A_INY:
                        low = dev->memread(dev->param, dev->pc++);
                        high = low + 1 & 0x00FF;
                        return (dev->memread(dev->param, low) | (dev->memread(dev->param, high) << 8)) + dev->y;
                case A_INZ:
                        low = dev->memread(dev->param, dev->pc++);
                        high = ++low & 0x00FF;
                        return dev->memread(dev->param, low) | (dev->memread(dev->param, high) << 8);
                case A_REL:
                        offset = (int8_t) dev->memread(dev->param, dev->pc++);
                        return dev->pc + offset;
                case A_ZPG:
                        return dev->memread(dev->param, dev->pc++);
                case A_ZPX:
                        return (dev->memread(dev->param, dev->pc++) + dev->x) & 0x00FF;
                case A_ZPY:
                        return (dev->memread(dev->param, dev->pc++) + dev->y) & 0x00FF;
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
        int8_t b;
        unsigned int tmp;
        switch (op.op){
                case ADC:
                        /* stolen from https://github.com/gianlucag/mos6502 because */
                        m = dev->memread(dev->param, addr);
                        tmp = m + dev->ac + getstat(CARRY);
                        setstat(ZERO, !(tmp & 0xFF));
                        if(getstat(DECIMAL)) {
                                if((dev->ac & 0xF) + (m & 0xF) + getstat(CARRY) > 9) tmp+=6;
                                setstat(NEGATIVE, tmp & 0x80);
                                setstat(OVERFLOW, !((dev->ac ^ m) & 0x80) && ((dev->ac ^ tmp) & 0x80));
                                if(tmp > 0x99) tmp += 96;
                                setstat(CARRY, tmp > 0x99);
                        } else {
                                setstat(NEGATIVE, tmp & 0x80);
                                setstat(OVERFLOW, !((dev->ac ^ m) & 0x80) && ((dev->ac ^ tmp) & 0x80));
                                setstat(CARRY, tmp > 0xFF);
                        }
                        dev->ac = tmp & 0xFF;
                        break;
                case AND:
                        dev->ac &= dev->memread(dev->param, addr);
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
                        m = dev->memread(dev->param, addr);
                        setstat(CARRY, m & 0x80);
                        m <<= 1;
                        setstat(ZERO, !m);
                        setstat(NEGATIVE, m & 0x80);
                        dev->memwrite(dev->param, addr, m);
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
                        setstat(ZERO, !(dev->ac & dev->memread(dev->param, addr)));
                        dev->sr = (dev->sr & 0x3F) | (dev->memread(dev->param, addr) & 0xC0);
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
                        emu65_hws_push(dev, dev->sr | 1 << BREAK);
                        setstat(INTERRUPT, 1);
                        dev->pc = dev->memread(dev->param, 0xFFFE) + (dev->memread(dev->param, 0xFFFF) << 8);
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
                        m = dev->memread(dev->param, addr);
                        tmp = dev->ac - m;
                        setstat(NEGATIVE, tmp & 0x80);
                        setstat(ZERO, dev->ac == m);
                        setstat(CARRY, dev->ac >= m);
                        break;
                case CPX:
                        m = dev->memread(dev->param, addr);
                        tmp = dev->x - m;
                        setstat(NEGATIVE, tmp & 0x80);
                        setstat(ZERO, dev->x == m);
                        setstat(CARRY, dev->x >= m);
                        break;
                case CPY:
                        m = dev->memread(dev->param, addr);
                        tmp = dev->y - m;
                        setstat(NEGATIVE, tmp & 0x80);
                        setstat(ZERO, dev->y == m);
                        setstat(CARRY, dev->y >= m);
                        break;
                case DEC:
                        m = dev->memread(dev->param, addr);
                        m--;
                        dev->memwrite(dev->param, addr, m);
                        setstat(NEGATIVE, m & 0x80);
                        setstat(ZERO, !m);
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
                        dev->ac ^= dev->memread(dev->param, addr);
                        setstat(NEGATIVE, dev->ac & 0x80);
                        setstat(ZERO, !dev->ac);
                        break;
                case INC:
                        m = dev->memread(dev->param, addr);
                        m++;
                        dev->memwrite(dev->param, addr, m);
                        setstat(NEGATIVE, m & 0x80);
                        setstat(ZERO, !m);
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
                        dev->ac = dev->memread(dev->param, addr);
                        setstat(NEGATIVE, dev->ac & 0x80);
                        setstat(ZERO, !dev->ac);
                        break;
                case LDX:
                        dev->x = dev->memread(dev->param, addr);
                        setstat(NEGATIVE, dev->x & 0x80);
                        setstat(ZERO, !dev->x);
                        break;
                case LDY:
                        dev->y = dev->memread(dev->param, addr);
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
                        m = dev->memread(dev->param, addr);
                        setstat(CARRY, m & 0x01);
                        m >>= 1;
                        dev->memwrite(dev->param, addr, m);
                        setstat(NEGATIVE, m & 0x80);
                        setstat(ZERO, !m);
                        break;
                case NO1:
                        break;
                case NO2:
                        dev->pc++;
                        break;
                case NO3:
                        dev->pc += 2;
                        break;
                case ORA:
                        dev->ac |= dev->memread(dev->param, addr);
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
                        emu65_hws_push(dev, dev->sr | 1 << BREAK);
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
                        dev->sr = (dev->sr & (1 << _IGNORE | 1 << BREAK)) | emu65_hws_pop(dev) & ~(1 << BREAK);
                        break;
                case ROL:
                        tmp = dev->memread(dev->param, addr);
                        tmp <<= 1;
                        tmp |= getstat(CARRY);
                        setstat(CARRY, tmp > 0xFF);
                        setstat(NEGATIVE, tmp & 0x80);
                        setstat(ZERO, !tmp);
                        dev->memwrite(dev->param, addr, tmp);
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
                        tmp = dev->memread(dev->param, addr);
                        tmp |= getstat(CARRY) << 8;
                        setstat(CARRY, tmp & 0x01);
                        tmp >>= 1;
                        setstat(NEGATIVE, tmp & 0x80);
                        setstat(ZERO, !(tmp & 0xFF));
                        dev->memwrite(dev->param, addr, tmp);
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
                        dev->sr = getstat(BREAK) << BREAK | emu65_hws_pop(dev) | 1 << _IGNORE;
                        dev->pc = emu65_hws_pop(dev);
                        dev->pc += emu65_hws_pop(dev) << 8;
                        break;
                case RTS:
                        dev->pc = emu65_hws_pop(dev);
                        dev->pc += emu65_hws_pop(dev) << 8;
                        dev->pc++;
                        break;
                case SBC:
                        m = dev->memread(dev->param, addr);
                        tmp = dev->ac - m - (getstat(CARRY) ? 0 : 1);
                        setstat(NEGATIVE, tmp & 0x80);
                        setstat(ZERO, !(tmp & 0xFF));
                        setstat(OVERFLOW, ((dev->ac ^ m) & 0x80) && ((dev->ac ^ tmp) & 0x80));
                        if(getstat(DECIMAL)) {
                                if(((dev->ac & 0x0F) - (getstat(CARRY) ? 0 : 1)) < (m & 0x0F))
                                        tmp -= 6;
                                if(tmp > 0x99)
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
                        dev->memwrite(dev->param, addr, dev->ac);
                        break;
                case STX:
                        dev->memwrite(dev->param, addr, dev->x);
                        break;
                case STZ:
                        dev->memwrite(dev->param, addr, 0x00);
                        break;
                case STY:
                        dev->memwrite(dev->param, addr, dev->y);
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
                        break;
                case TYA:
                        dev->ac = dev->y;
                        setstat(NEGATIVE, dev->ac & 0x80);
                        setstat(ZERO, !dev->ac);
                        break;
                case TRB:
                        m = dev->memread(dev->param, addr);
                        m &= ~dev->ac;
                        dev->memwrite(dev->param, addr, m);
                        setstat(ZERO, dev->ac & m);
                        break;
                case TSB:
                        m = dev->memread(dev->param, addr);
                        m |= dev->ac;
                        dev->memwrite(dev->param, addr, m);
                        setstat(ZERO, dev->ac & m);
                        break;
                case BS0:
                        m = addr & 0xFF;
                        b = addr >> 8 & 0xFF;
                        m = dev->memread(dev->param, m);
                        if(m & 1)
                                dev->pc += b;
                        break;
                case BS1:
                        m = addr & 0xFF;
                        b = addr >> 8 & 0xFF;
                        m = dev->memread(dev->param, m);
                        if(m & 1 << 1)
                                dev->pc += b;
                        break;
                case BS2:
                        m = addr & 0xFF;
                        b = addr >> 8 & 0xFF;
                        m = dev->memread(dev->param, m);
                        if(m & 1 << 2)
                                dev->pc += b;
                        break;
                case BS3:
                        m = addr & 0xFF;
                        b = addr >> 8 & 0xFF;
                        m = dev->memread(dev->param, m);
                        if(m & 1 << 3)
                                dev->pc += b;
                        break;
                case BS4:
                        m = addr & 0xFF;
                        b = addr >> 8 & 0xFF;
                        m = dev->memread(dev->param, m);
                        if(m & 1 << 4)
                                dev->pc += b;
                        break;
                case BS5:
                        m = addr & 0xFF;
                        b = addr >> 8 & 0xFF;
                        m = dev->memread(dev->param, m);
                        if(m & 1 << 5)
                                dev->pc += b;
                        break;
                case BS6:
                        m = addr & 0xFF;
                        b = addr >> 8 & 0xFF;
                        m = dev->memread(dev->param, m);
                        if(m & 1 << 6)
                                dev->pc += b;
                        break;
                case BS7:
                        m = addr & 0xFF;
                        b = addr >> 8 & 0xFF;
                        m = dev->memread(dev->param, m);
                        if(m & 1 << 7)
                                dev->pc += b;
                        break;
                case BR0:
                        m = addr & 0xFF;
                        b = addr >> 8 & 0xFF;
                        m = dev->memread(dev->param, m);
                        if(!(m & 1))
                                dev->pc += b;
                        break;
                case BR1:
                        m = addr & 0xFF;
                        b = addr >> 8 & 0xFF;
                        m = dev->memread(dev->param, m);
                        if(!(m & 1 << 1))
                                dev->pc += b;
                        break;
                case BR2:
                        m = addr & 0xFF;
                        b = addr >> 8 & 0xFF;
                        m = dev->memread(dev->param, m);
                        if(!(m & 1 << 2))
                                dev->pc += b;
                        break;
                case BR3:
                        m = addr & 0xFF;
                        b = addr >> 8 & 0xFF;
                        m = dev->memread(dev->param, m);
                        if(!(m & 1 << 3))
                                dev->pc += b;
                        break;
                case BR4:
                        m = addr & 0xFF;
                        b = addr >> 8 & 0xFF;
                        m = dev->memread(dev->param, m);
                        if(!(m & 1 << 4))
                                dev->pc += b;
                        break;
                case BR5:
                        m = addr & 0xFF;
                        b = addr >> 8 & 0xFF;
                        m = dev->memread(dev->param, m);
                        if(!(m & 1 << 5))
                                dev->pc += b;
                        break;
                case BR6:
                        m = addr & 0xFF;
                        b = addr >> 8 & 0xFF;
                        m = dev->memread(dev->param, m);
                        if(!(m & 1 << 6))
                                dev->pc += b;
                        break;
                case BR7:
                        m = addr & 0xFF;
                        b = addr >> 8 & 0xFF;
                        m = dev->memread(dev->param, m);
                        if(!(m & 1 << 7))
                                dev->pc += b;
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
        op = emu65_opc_table[dev->memread(dev->param, dev->pc++)];
        if(emu65_run_op(dev, op)) return 0;
        else return dev->pc;
}

void
emu65_reset(Emu65Device *dev)
{
        dev->pc = (dev->memread(dev->param, rstH) << 8) + dev->memread(dev->param, rstL);
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
                cbi(dev->sr, BREAK);
                emu65_hws_push(dev, dev->sr);
                sbi(dev->sr, INTERRUPT);
                dev->pc = (dev->memread(dev->param, irqH) << 8) + dev->memread(dev->param, irqL);
        }
        return;
}

void
emu65_nmi(Emu65Device *dev)
{
        emu65_hws_push(dev, (dev->pc >> 8) & 0xFF);
        emu65_hws_push(dev, dev->pc & 0xFF);
        cbi(dev->sr, BREAK);
        emu65_hws_push(dev, dev->sr);
        sbi(dev->sr, INTERRUPT);
        dev->pc = (dev->memread(dev->param, nmiH) << 8) + dev->memread(dev->param, nmiL);
        return;
}
