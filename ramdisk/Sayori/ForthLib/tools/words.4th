
VARIABLE NNN
VARIABLE >OUT
VARIABLE W-CNT

: ?CR-BREAK ( NFA -- NFA TRUE | FALSE )
  DUP
\  CR ." NL2" CR   DUP HH.
  IF
   DUP C@ >OUT @ +    64 >
     IF >OUT 0!
\		CR ." s<"  .S ." >"
        NNN @
        IF    -1 NNN +!  TRUE
        ELSE	." Q - quit" CR 6 NNN !
              KEY 0x20 OR 
                [CHAR] q <>    AND
               ?DUP 0<>
        THEN
     ELSE TRUE
     THEN
  THEN
;

: NLIST ( A -> )
  L@  >OUT   0!
   CR W-CNT 0!  6 NNN L!
  BEGIN
   ?CR-BREAK
  WHILE
    W-CNT 1+!
    DUP ID.
    DUP C@ >OUT +!
    8 >OUT @ 8 MOD - DUP >OUT +! SPACES
    CDR
  REPEAT \ KEY? IF KEY DROP THEN
  CR CR ." Words: " W-CNT @ U.
   CR
;

: WORDS ( -- ) \ 94 TOOLS
  CONTEXT @  NLIST ;

