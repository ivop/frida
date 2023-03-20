
    org $4000

; mads      (NoMoreSecrets/KickAss)

; ASO       (SLO)

    aso $ff
    aso $ff,x
    aso ($ff,x)
    aso ($ff),y
    aso $ffff
    aso $ffff,x
    aso $ffff,y

; RLN       (RLA)

    rln $ff
    rln $ff,x
    rln ($ff,x)
    rln ($ff),y
    rln $ffff
    rln $ffff,x
    rln $ffff,y

; LSE       (SRE)

    lse $ff
    lse $ff,x
    lse ($ff,x)
    lse ($ff),y
    lse $ffff
    lse $ffff,x
    lse $ffff,y

; RRD       (RRA)

    rrd $ff
    rrd $ff,x
    rrd ($ff,x)
    rrd ($ff),y
    rrd $ffff
    rrd $ffff,x
    rrd $ffff,y

; SAX       (SAX)

    sax $ff
    sax $ff,y
    sax ($ff,x)
    sax $ffff

; LAX       (LAX)

    lax $ff
    lax $ff,y
;    lax ($ff,x)             ; MADS 2.1.5 BUG: $9F, should be $A3
    .byte $a3, $ff
    lax ($ff),y
    lax $ffff
    lax $ffff,y

; DCP       (DCP)

    dcp $ff
    dcp $ff,x
    dcp ($ff,x)
    dcp ($ff),y
    dcp $ffff
    dcp $ffff,x
    dcp $ffff,y

; ISB       (ISC)

    isb $ff
    isb $ff,x
    isb ($ff,x)
    isb ($ff),y
    isb $ffff
    isb $ffff,x
    isb $ffff,y

; ANC       (ANC)

    anc #$ff

;           (ANC2)

    .byte $2b, $ff

; ALR       (ALR)

    alr #$ff

; ARR       (ARR)

    arr #$ff

; SBX       (SBX)

    sbx #$ff

;           (SBC2)

    .byte $eb, $ff

; ANX       (LAX)

    anx #$ff            ; unstable

; ANE       (ANE)

    ane #$ff            ; unstable

; SHA       (SHA)

    sha ($ff),y         ; somewhat unstable
    sha $ffff,y         ; somewhat unstable

; SHY       (SHY)

    shy $ffff,x         ; somewhat unstable     ; BUG not marked as unstable

; SHX       (SHX)

    shx $ffff,y         ; somewhat unstable     ; BUG not marked as unstable

; SHS       (TAS)

    shs $ffff,y         ; somewhat unstable

; LAS       (LAS)

    las $ffff,y         ; somewhat unstable

; NPO       (NOP)

    npo                 ; $1A

    .byte $3a           ; NOP
    .byte $5a           ; NOP
    .byte $7a           ; NOP
    .byte $da           ; NOP
    .byte $fa           ; NOP

    dop #$ff            ;    .byte $80, $ff      ; NOP #$ff
    .byte $82, $ff      ; NOP #$ff
    .byte $c2, $ff      ; NOP #$ff
    .byte $e2, $ff      ; NOP #$ff
    .byte $89, $ff      ; NOP #$ff

    .byte $04, $ff      ; NOP $ff
    dop $ff             ; .byte $44, $ff      ; NOP $ff
    .byte $64, $ff      ; NOP $ff

    .byte $14, $ff      ; NOP $ff,x
    .byte $34, $ff      ; NOP $ff,x
    dop $ff,x           ; .byte $54, $ff      ; NOP $ff,x
    .byte $74, $ff      ; NOP $ff,x
    .byte $d4, $ff      ; NOP $ff,x
    .byte $f4, $ff      ; NOP $ff,x

    top $ffff           ; .byte $0c, $ff, $ff ; NOP $ffff

    top $ffff,x         ; .byte $1c, $ff, $ff ; NOP $ffff,x
    .byte $3c, $ff, $ff ; NOP $ffff,x
    .byte $5c, $ff, $ff ; NOP $ffff,x
    .byte $7c, $ff, $ff ; NOP $ffff,x
    .byte $dc, $ff, $ff ; NOP $ffff,x
    .byte $fc, $ff, $ff ; NOP $ffff,x

; CIM       (JAM)

    cim

    .byte $12, $ff      ; CIM #$ff
    .byte $22, $ff      ; CIM $ff
    .byte $32, $ff      ; CIM $ff,x
    .byte $42, $ff      ; CIM $ff,y
    .byte $52, $ff      ; CIM ($ff,x)
    .byte $62, $ff      ; CIM ($ff),y
    .byte $72, $ff, $ff ; CIM $ffff
    .byte $92, $ff, $ff ; CIM $ffff,x
    .byte $b2, $ff, $ff ; CIM $ffff,y
    .byte $d2, $ff, $ff ; CIM ($ffff)
    .byte $f2, $fe      ; CIM relative
