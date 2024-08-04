REQUIRE CHAR-UPPERCASE ForthLib\spf\uppercase.4th

: CEQUAL-U ( a1 u1 a2 u2 -- flag )
  ROT TUCK <> IF DROP 2DROP FALSE EXIT THEN
  0 ?DO ( a1i a2i ) 2DUP
  C@ CHAR-UPPERCASE SWAP C@ CHAR-UPPERCASE <> IF 2DROP UNLOOP FALSE EXIT THEN
  SWAP CHAR+ SWAP CHAR+
  LOOP 2DROP TRUE
;

: COMMENT:  ( -- )
  BEGIN
    PARSE-NAME DUP 0=
    IF  NIP  REFILL   0= IF DROP TRUE THEN
    ELSE  S" COMMENT;" CEQUAL-U  THEN
  UNTIL
; IMMEDIATE

: (*  ( -- )
  BEGIN
    PARSE-NAME DUP 0=
    IF  NIP  REFILL   0= IF DROP TRUE THEN
    ELSE  S" *)" CEQUAL-U  THEN
  UNTIL
; IMMEDIATE


: [ELSE]   \ 94 TOOLS EXT
    1
    BEGIN
      PARSE-NAME DUP
      IF  
         2DUP   S" \"  CEQUAL-U   IF 2DROP POSTPONE \	ELSE 
         2DUP   S" COMMENT:"  CEQUAL-U   IF 2DROP POSTPONE COMMENT:	ELSE 
         2DUP   S" (*"  CEQUAL-U   IF 2DROP POSTPONE (*	ELSE 
         2DUP 3 UMIN
  S" [IF"  \ все слова с префиксом "[IF"
            CEQUAL-U  IF 2DROP 1+                 ELSE 
         2DUP S" [else]" CEQUAL-U  IF 2DROP 1- DUP  IF 1+ THEN ELSE 
              S" [then]" CEQUAL-U  IF       1-                 THEN
		THEN  THEN   THEN  THEN   THEN 
      ELSE 2DROP REFILL  AND \   SOURCE TYPE
      THEN DUP 0=
    UNTIL  DROP 
;  IMMEDIATE

: [IF] \ 94 TOOLS EXT
  0= IF POSTPONE [ELSE] THEN
; IMMEDIATE


: [THEN] \ 94 TOOLS EXT
; IMMEDIATE
