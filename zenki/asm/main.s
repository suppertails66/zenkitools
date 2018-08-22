
.include "sys/sms_arch.s"
  ;.include "base/ram.s"
;.include "base/macros.s"
  ;.include "res/defines.s"

.rombankmap
  bankstotal 64
  banksize $4000
  banks 64
.endro

.emptyfill $FF

.background "zenki.gg"

.unbackground $7F94 $7FEF

.unbackground $A0000 $A3FFF

;===============================================
; Update header after building
;===============================================
.smstag

;===============================================
; startup splash screen
;===============================================

/*.bank $0 slot 0
.org $00A4
.section "do startup splash screen" overwrite
  call startupSplashScreenInit
.ends

.bank $1 slot 1
.section "jump to startup splash screen" free
  startupSplashScreenInit:
    push af
      ld a,($FFFF)
      push af
        ld a,:startupSplashScreen
        ld ($FFFF),a
        call startupSplashScreen
      pop af
      ld ($FFFF),a
    pop af
    ; make up work
    jp $0273
    
.ends

.slot 2
.section "startup splash screen" superfree
  startupSplashScreen:
    
    ret
.ends*/

.define splashScreenShownMemString $DFEB
.define splashScreenShownStringVal "ZENK"

.bank 0 slot 0
.org $06B3
.section "do startup splash screen" overwrite
  call startupSplashScreenInit
.ends

.bank $1 slot 1
.section "jump to startup splash screen" free
  startupSplashScreenInit:
    ld a,:startupSplashScreen
    ld ($FFFF),a
    jp startupSplashScreen

/*  ;=======================================
  ; CUTSCENE TEST
  ;=======================================
  
  gameOverTest:
    ld ($C1B3),a
    ld a,$83
    ld ($C014),a
    jp $6337
    
  orbTest:
    ld ($C010),a
    ld a,$92
    ld ($C014),a
    jp $6337
    
  startupSplashScreenTrigger:
    
    ; ???
    ld a,$01
    ld ($C013),a
    
    ; run through alt branches of zenki resurrection scene
    
      ; first-time visit, not all orbs collected
      xor a
      call orbTest
      
      ; repeat visit, not all orbs collected
      ld a,$40
      call orbTest
      
      ; first-time visit, all orbs collected
      ld a,$1F
      call orbTest
      
      ; repeat visit, all orbs collected
      ld a,$5F
      call orbTest
  
    ; loop through main cutscenes
  
    ld a,$80
    ld b,$2B
    
    -:
      push af
        
        ; cutscene ID with bit 7 set
        ; (cutscene 06 was previously a dummy slot)
        ld ($C014),a
          
        ; run script
        call $6337
      
      pop af
        
;      ; if game over scene just played, play the alternate game over scenes
;      cp $83
;      jr nz, +
;        ld a,$80
;        ld ($C010),a
;        ld a,$81
;        call $6337
;        xor a
;        ld ($C010),a
;        
;        
;        ld a,$03
;        call gameOverTest
;        
;        ld a,$02
;        call gameOverTest
;        
;        ld a,$83
;        jr @postCheckDone
;      +:
      
      @postCheckDone:
      
      inc a
      
      @skipBad:
        
        cp $87
        jr z,@increment
        cp $9C
        jr z,@increment
        ; resurrection cutscene
        cp $92
        jr z,@increment
        jr @loop
        
        @increment:
        inc a
        jr @skipBad
      
      @loop:
      
      djnz -
    
    @done:
    ret
  
  ;=======================================
  ; END CUTSCENE TEST
  ;======================================= */
  
  ;=======================================
  ; startup screen trigger
  ;=======================================
    
  startupSplashScreenTrigger:
    ; run script
    call $6337
    ; make up work again (for upcoming Sega screen)
    call $44BC
    @done:
    ret
    
.ends

.slot 2
.section "startup splash screen" superfree

  splashScreenShownString:
    .asc splashScreenShownStringVal
  splashScreenShownStringEnd:
  
  .define splashScreenShownStringLen splashScreenShownStringEnd-splashScreenShownString
  
  softResetCheck:
    ld hl,splashScreenShownMemString
    ld de,splashScreenShownString
    ld b,splashScreenShownStringEnd-splashScreenShownString
    
    @checkLoop:
      ; check if next char in memory matches src string
      ld a,(hl)
      ld c,a
      ld a,(de)
      cp c
      jr z,@loopEnd
      
      ; mismatch: return 0
      xor a
      ret
      
      ; match: move to next character
      @loopEnd:
      inc hl
      inc de
      djnz @checkLoop
    
    ; match: return nonzero
    ld a,$FF
    ret
    
  copySoftResetString:
    ; copy soft reset check string to memory
    ld hl,splashScreenShownMemString
    ld de,splashScreenShownString
    ld b,splashScreenShownStringEnd-splashScreenShownString
    -:
      ld a,(de)
      ld (hl),a
      inc hl
      inc de
      djnz -
    ret
  
  startupSplashScreen:
    
    ; make up work
    call $44BC
    
    ; check if soft reset
    call softResetCheck
    or a
    jr z,@doIt
    jp startupSplashScreenTrigger@done
    
    @doIt:
    
    call copySoftResetString
    
    ; do init
    call $0A6B
    call $0A33
    
    ; cutscene ID with bit 7 set
    ; (cutscene 06 was previously a dummy slot)
    ld a,$86
    ld ($C014),a
    ; ???
    ld a,$01
    ld ($C013),a
    
    ; get out of slot 2
    jp startupSplashScreenTrigger
    
.ends

;===============================================
; disable diacritic check, freeing up code
;===============================================

.bank $1 slot 1
.org $28FC
.section "skip diacritic check" overwrite
  jp $6910
.ends

;===============================================
; allow multiple linebreaks
; (up to 4 lines, though using the last one
; requires additional script editing so the
; "blanking" tilemap will cover it up between
; boxes)
;===============================================

.bank $1 slot 1
.org $28FF
.section "new linebreak logic" overwrite
  ; at this point, DE is the current VDP dstaddr
  ; 7B4E-7B72 = line 1
  ; 7B8E-7BB2 = line 2
  ; 7BCE-7BF2 = line 3
  ; 7C0E+     = line 4
  
  newLinebreakLogic:
    ld a,$8C
    cp e
    jr c,+
    @finish:
      ld e,a
      ret
    +:
    ld a,$CC
    cp e
    jr nc,@finish
    +:
    ld de,$7C0C
    ret
  
.ends

.bank $1 slot 1
.org $28E9
.section "use new linebreak logic" overwrite
  
  call newLinebreakLogic
  
.ends

; lower the area blanked between text boxes by a line to cover
; the full area we're using
.bank $1 slot 1
.org $287F
.section "clear full text area" overwrite
  ld de,$7B40
.ends

; move initial VDP addr left a tile
.bank $1 slot 1
.org $289B
.section "move text area left" overwrite
  ld hl,$7B4C
.ends

;===============================================
; Change tile used for cursor on game over
; yes/no prompt from 81 to B4
;===============================================

.bank $1 slot 1
.org $268E
.section "game over cursor 1" overwrite
  ld hl,$1FB4   ; tile indices: blank / cursor
.ends

.bank $1 slot 1
.org $2694
.section "game over cursor 2" overwrite
  ld hl,$B41F   ; tile indices: cursor / blank
.ends

;===============================================
; Redirect string lookups to new target bank
;===============================================
.define newStringBank $20

; when retrieving initial pointer
.bank $1 slot 1
.org $28A7
.section "new string lookup bank 1" overwrite
  ld a,newStringBank
.ends

; when looking up character
.bank $1 slot 1
.org $28C7
.section "new string lookup bank 2" overwrite
  ld a,newStringBank
.ends
