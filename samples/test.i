.memorymap
slotsize $4000
defaultslot 0
slot 0 $C000
.endme
.rombankmap
bankstotal 1
banksize $4000
banks 1
.endro

.bank 0 slot 0
.orga $C000

start:
ldx #$0
printnext:
lda.w txt,x
beq done
jsr putc
inx
bne printnext
done:
jsr getc
bcc done
jsr putc
jmp done

.db $02


txt:
.db "Hello, world", $0A, $0

.orga $F000
/* SYSTEM ROUTINES */
putc:
sta $FF0E
lda #$FF
sta $FF0F
rts

getc:
lda $FF0D
BNE return_char
clc
rts
return_char:
sec
lda #$0
sta $FF0D
lda $FF0C
rts

.orga $FFFA
.db $00 $00 $00 $C0 $00 $00
