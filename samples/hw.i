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

ldx #0
printnext:
lda.w thetext,x
beq done
jsr $BC00
inx
bne printnext
done:
.db $02

thetext:
.db  "Hello, world", $0a, 0

.orga $FFFA
.db $00 $00 $00 $C0 $00 $00
