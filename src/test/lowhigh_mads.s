
ptr = $e0

    org $4000

    jmp allblocks

block1
:32 dta 'block1dataabcdefghijklm'
block2
:32 dta 'block2dataabcdefghijklm'
block3
:32 dta 'block3dataabcdefghijklm'
block4
:32 dta 'block4dataabcdefghijklm'

:32 nop

lowbytes
    .byte <block1, <block2, <block3, <block4

    nop     ; something in between

highbytes
    .byte >block1, >block2, >block3, >block4

:32 nop

lowbytes2
    .byte <block1, <block2, <block3, <block4
    ; nothing in between
highbytes2
    .byte >block1, >block2, >block3, >block4

; inside code

allblocks
    lda #<block1
    sta ptr
    lda #>block1
    sta ptr+1

startblock2
    lda #<block2
    sta ptr
    lda #>block2
    sta ptr+1

startblock3
    lda #<block3
    sta ptr
    lda #>block3
    sta ptr+1

startblock4
    lda #<block4
    sta ptr
    lda #>block4
    sta ptr+1

    jmp *

:32 nop

jumptable_low
    .byte <(allblocks-1), <(startblock2-1), <(startblock3-1), <(startblock4-1)
jumptable_high
    .byte >(allblocks-1), >(startblock2-1), >(startblock3-1), >(startblock4-1)

:32 nop

    ldx #0
jumper
    lda jumptable_high,x
    pha
    lda jumptable_low,x
    pha
    rts
