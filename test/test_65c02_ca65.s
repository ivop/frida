
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

    bbr0 $ff,label  ; $0f
    bbr1 $ff,label  ; $1f
    bbr2 $ff,label  ; $2f
    bbr3 $ff,label  ; $3f
    bbr4 $ff,label  ; $4f
    bbr5 $ff,label  ; $5f
    bbr6 $ff,label  ; $6f
    bbr7 $ff,label  ; $7f
    bbs0 $ff,label  ; $8f
    bbs1 $ff,label  ; $9f
    bbs2 $ff,label  ; $af
    bbs3 $ff,label  ; $bf
    bbs4 $ff,label  ; $cf
    bbs5 $ff,label  ; $df
    bbs6 $ff,label  ; $ef
    bbs7 $ff,label  ; $ff

; RMB, SMB, Reset or Set Memory Bit

    rmb0 $ff        ; $07
    rmb1 $ff        ; $17
    rmb2 $ff        ; $27
    rmb3 $ff        ; $37
    rmb4 $ff        ; $47
    rmb5 $ff        ; $57
    rmb6 $ff        ; $67
    rmb7 $ff        ; $77
    smb0 $ff        ; $87
    smb1 $ff        ; $97
    smb2 $ff        ; $a7
    smb3 $ff        ; $b7
    smb4 $ff        ; $c7
    smb5 $ff        ; $d7
    smb6 $ff        ; $e7
    smb7 $ff        ; $f7

; STP Stop

    stp             ; $db

; WAI WAit for Interrupt

    wai             ; $cb

