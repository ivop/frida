
; use 0100H so we can use the CP/M loader for convenience

    org $0100

; $00-$0f

label:
    nop
    ld bc,$55ee
    ld (bc),a
    inc bc
    inc b
    dec b
    ld b,$5e
    rlca
    ex af,af'
    add hl,bc
    ld a,(bc)
    dec bc
    inc c
    dec c
    ld c,$5e
    rrca

; $10-$1f

    djnz label
    ld de,$55ee
    ld (de),a
    inc de
    dec d
    ld d,$5e
    rla
    jr label
    add hl,de
    ld a,(de)
    dec de
    inc e
    dec e
    ld e,$5e
    rra
