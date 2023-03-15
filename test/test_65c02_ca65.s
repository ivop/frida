
    .org $4000

    .setcpu "65c02"

; (zp) addressing mode

    adc ($ff)       ; $72
    and ($ff)       ; $32
    cmp ($ff)       ; $d2
    eor ($ff)       ; $52
    lda ($ff)       ; $b2
    ora ($ff)       ; $12
    sbc ($ff)       ; $f2
    sta ($ff)       ; $92

; BIT imm zp,x and abs,x addressing modes

    bit #$ff        ; $89
    bit $ff,x       ; $34
    bit $ffff,x     ; $3c

; DEC, INC accumulator

    dec             ; $3a
    inc             ; $1a

; JMP (abs,x) addressing mode

    jmp ($ffff,x)   ; $7c

; BRA BRanch Always

label:
    bra label       ; $80

; PHX, PHY, PLX, PLY, Push and Pull X and Y registers

    phx             ; $da
    phy             ; $5a
    plx             ; $fa
    ply             ; $7a

; STZ store zero

    stz $ff         ; $64
    stz $ff,x       ; $74
    stz $ffff       ; $9c
    stz $ffff,x     ; $9e

; TRB Test and Reset Bits

    trb $ff         ; $14
    trb $ffff       ; $1c

; TSB Test and Set Bits

    tsb $ff         ; $04
    tsb $ffff       ; $0c

; Additional Rockwell and WDC 65C02 instructions

; BBR, BBS, Branch on Bit Reset or Set

; RMB, SMB, Reset or Set Memory Bit

; STP Stop

; WAI WAit for Interrupt

