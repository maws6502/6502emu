struct emu65_device {
        uint16_t pc;
        uint8_t ac, x, y, sr, sp;
        void (*memwrite)(void *, uint16_t, uint8_t);
        uint8_t (*memread)(void *, uint16_t);
        void *param;
};
typedef struct emu65_device Emu65Device;

uint16_t emu65_cycle(Emu65Device *dev);
void emu65_reset(Emu65Device *dev);
void emu65_irq(Emu65Device *dev);
void emu65_nmi(Emu65Device *dev);
