
ptr = $e0

    org $4000

    jmp main

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

main
    lda #<block1
    sta ptr
    lda #>block1
    sta ptr+1

    lda #<block2
    sta ptr
    lda #>block2
    sta ptr+1

    lda #<block3
    sta ptr
    lda #>block3
    sta ptr+1

    lda #<block4
    sta ptr
    lda #>block4
    sta ptr+1

    jmp *

