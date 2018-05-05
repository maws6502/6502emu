struct emu65_device {
        uint16_t pc;
        uint8_t ac, x, y, sr, sp;
        uint8_t memory[0x10000];
};
typedef struct emu65_device Emu65Device;

uint16_t emu65_cycle(Emu65Device *dev);
void emu65_reset(Emu65Device *dev);
void emu65_irq(Emu65Device *dev);
void emu65_nmi(Emu65Device *dev);
