
#define EXKEY_UP                         0x480000
#define EXKEY_DOWN                       0x500000
#define EXKEY_RIGHT                      0x4D0000
#define EXKEY_LEFT                       0x4B0000
#define EXKEY_HOME                       0x470000
#define EXKEY_END                        0x4F0000
#define EXKEY_INSERT                     0x520000
#define EXKEY_DELETE                     0x530000
#define EXKEY_PAGE_UP                    0x490000
#define EXKEY_PAGE_DOWN                  0x510000
#define EXKEY_F1                         0x3B0000
#define EXKEY_F2                         0x3C0000
#define EXKEY_F3                         0x3D0000
#define EXKEY_F4                         0x3E0000
#define EXKEY_F5                         0x3F0000
#define EXKEY_F6                         0x400000
#define EXKEY_F7                         0x410000
#define EXKEY_F8                         0x420000
#define EXKEY_F9                         0x430000
#define EXKEY_F10                        0x440000
#define EXKEY_F11                        0x570000
#define EXKEY_F12                        0x580000
#define EXKEY_ESC                        0x010000

\- SHIFT+	: SHIFT+ $100 OR ;
\- CTL+		: CTL+   $200 OR ;
\- ALT+		: ALT+   $400 OR ;

: EXKEY
 PUNCH
 CURSOR
 SCANKEY DUP SCAN2UN DUP $FF53 =
 IF   DROP $10 LSHIFT
 ELSE NIP  DUP $5B00 = IF DROP 8 THEN
 THEN
\ + SHIFT?  SHIFT? IF SHIFT+ THEN
\+ CTL?  CTL? IF CTL+ THEN
\+ ALT?  ALT? IF ALT+ THEN

CURSOR
 ;

' EXKEY TO KEY

