REQUIRE [IF] ~mak/CompIF1.f
REQUIRE [IFNDEF] ~nn\lib\ifdef.f

[IFNDEF] CSP
VARIABLE CSP    \ ”казатель стека контрол€
[THEN]

[IFNDEF] ?CSP
: !CSP          ( -- )  \ save current stack pointer for later stack depth check
                SP@ CSP ! ;

: ?CSP          ( -- )  \ check current stack pointer against saved stack pointer
                SP@ CSP @ XOR IF -330 THROW THEN ;
[THEN]


: CASE 
  CSP @ SP@ CSP ! ; IMMEDIATE

: ?OF_ 
  POSTPONE IF POSTPONE DROP ; IMMEDIATE

: OF 
  POSTPONE OVER POSTPONE = POSTPONE ?OF_ ; IMMEDIATE


: ENDOF 
  POSTPONE ELSE ; IMMEDIATE

: DUPENDCASE
  BEGIN SP@ CSP @ <> WHILE POSTPONE THEN REPEAT
  CSP ! ; IMMEDIATE

: ENDCASE 
  POSTPONE DROP   POSTPONE DUPENDCASE 
; IMMEDIATE

: OF\
  POSTPONE OVER POSTPONE <> POSTPONE IF ; IMMEDIATE

: OF;
  POSTPONE OVER POSTPONE = POSTPONE IF 2>R
  POSTPONE DUPENDCASE  2R>
  POSTPONE DROP ; IMMEDIATE


