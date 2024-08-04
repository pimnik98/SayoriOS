// https://github.com/mak4444/gnu-efi-code-forth/blob/main/apps/Forth64S/Meta_x86_64/Mak64Forth.cpp
#include <kernel.h>
#include "drv/ps2.h"

//extern bool    echo;            ///< Включен ли вывод?
extern uint8_t kbdstatus;	///< Статус клавиатуры
extern char G_CLI_PATH[];
extern uint32_t tty_pos_x;	///< Позиция на экране по X
extern uint32_t tty_pos_y;	///< Позиция на экране по Y
extern uint32_t tty_text_color;	///< Текущий цвет шрифта
extern uint32_t tty_bg_color;          ///< Текущий задний фон
extern uint32_t tty_off_pos_x;
extern uint32_t tty_off_pos_h;

char* input_buffer[256];

#define STACK_SIZE 1000 /* cells reserved for the stack */
#define RSTACK_SIZE 1000 /* cells reserved for the return stack */
#define HERE_SIZE1 0x10000

typedef long unsigned Cell;
typedef long sCell;
typedef  void (*proc)(void);

#define pp(cName) const sCell p##cName = (sCell)cName;
#define PP(cName) const sCell P##cName = (sCell)cName;

static int forth_run;
sCell * HereArea;
sCell * StackArea;
sCell * RStackArea;
sCell * here;
sCell *Stack;
sCell *rStack;
sCell * ip ;
sCell ireg;
sCell Tos;
sCell * Handler = NULL;

Cell numericBase = 10;
 
void Co( sCell cod ){ *here++ =  cod ;}
void Co2( sCell cod1,sCell cod2 ){Co(cod1); Co(cod2); }
void Co3( sCell cod1,sCell cod2,sCell cod3 ){Co2(cod1,cod2); ; Co(cod3); }
void Co4( sCell cod1,sCell cod2,sCell cod3,sCell cod4 ){  Co3(cod1,cod2,cod3); ; Co(cod4); }
void Co5( sCell cod1,sCell cod2,sCell cod3,sCell cod4,sCell cod5 )
{ Co4(cod1,cod2,cod3,cod4); ; Co(cod5); }
void Co6( sCell cod1,sCell cod2,sCell cod3,sCell cod4,sCell cod5,sCell cod6 )
{ Co5(cod1,cod2,cod3,cod4,cod5); Co(cod6); }
void Co7( sCell cod1,sCell cod2,sCell cod3,sCell cod4,sCell cod5,sCell cod6,sCell cod7 )
{ Co6(cod1,cod2,cod3,cod4,cod5,cod6);  Co(cod7);}

void  Noop(void) {}  pp(Noop)

void  DoVar(void){   *--Stack= Tos; Tos =(sCell)ip; ip = (sCell*)*rStack++; } pp(DoVar)
void  DoConst(void){  *--Stack= Tos; Tos = *ip; ip = (sCell*)*rStack++; } pp(DoConst)
void Execute()
    {
//	tty_printf("Execute\n");
   ireg =Tos; Tos=*Stack++;
	if ( (ireg<0) ^ (pNoop<0) ) {
	((proc) (~ireg))();
	return;
    	}
    *--rStack = (sCell) ip;
        ip = (sCell *) ireg;
    } pp(Execute)

void DoDefer(){
//	tty_printf("DoDefer\n");
    ireg = *ip;
    if ( (ireg<0) ^ (pNoop<0) ) {
        ip = (sCell*)*rStack++; // exit
        ((proc)(~ireg))();
            return;
        }
         ip =  (sCell *) ireg;

} pp(DoDefer)

void Lit_(){  *--Stack = Tos; Tos = *ip++; } pp(Lit_)
void Lit( sCell val) {Co(~pLit_);  *here++ = val;  }
void Compile(){  *here++ = Tos; Tos = *Stack++;  }  pp(Compile)
void LitCo(){  Co(~pLit_); Compile();}  pp(LitCo)
void Allot(){ *(sCell*)&here += Tos; Tos = *Stack++;  }  pp(Allot)

void  Exit() {
//      tty_printf("Exit %x ",ip); // addresses area
 ip = (sCell*)*rStack++;
//      tty_printf("-> %x \n",ip); // addresses area
 } pp(Exit)
void  Here(void){  *--Stack = Tos; Tos = (sCell)here;  } pp(Here)

void Branch(){
//      tty_printf("Branch %x ",ip); // addresses area
 ip = *(sCell**)ip;
//      tty_printf("-> %x \n",ip); // addresses area
 } pp(Branch)

void QBranch(){
//      tty_printf("QBranch %x ",ip); // addresses area
    if(Tos) ip++;
    else    ip = *(sCell**)ip;
    Tos =   *Stack++;
//      tty_printf("-> %x \n",ip); // addresses area
} pp(QBranch)

void Str() {
    *--Stack = Tos;
    *--Stack = (Cell)ip+1;
        Tos = *(char unsigned *)ip;
    ip = (sCell*) ((Cell)ip + Tos + 1);
    ip = (sCell*) ( ( (Cell)ip + sizeof(Cell) - 1 ) & (-sizeof(Cell) )  );

}  pp(Str)

void Dup(){   *--Stack= Tos;  } pp(Dup)
void Drop(){  Tos = *Stack++;  } pp(Drop)
void Nip(){   Stack++;   } pp(Nip)
void QDup(){   if(Tos) *--Stack= Tos;   } pp(QDup)
void Over(){   *--Stack= Tos; Tos = Stack[1];    } pp(Over)
void Tuck(){    sCell tt=*Stack; *Stack=Tos; *--Stack=tt;  }  pp(Tuck)
void Pick(){    Tos = Stack[Tos];  }  pp(Pick)
void i2dup(){   *--Stack= Tos; *--Stack= Stack[1];  } pp(i2dup)
void i2over(){   *--Stack= Tos;  *--Stack= Stack[3]; Tos= Stack[3];  } pp(i2over)
void i2drop(){  Stack++; Tos = *Stack++; } pp(i2drop)
void Swap(){  sCell tt=Tos; Tos=Stack[0]; Stack[0]=tt;  }  pp(Swap)
void i2Swap(){
    sCell   tt=Tos; Tos=Stack[1]; Stack[1]=tt;
            tt=Stack[0]; Stack[0]=Stack[2]; Stack[2]=tt;  }  pp(i2Swap)
void Rot(){ Cell tt=Stack[1]; Stack[1]=Stack[0]; Stack[0]=Tos; Tos=tt; } pp(Rot)
void Add(){ Tos += *Stack++;  } pp(Add)
void Sub(){ Tos = -Tos;  Tos += *Stack++; } pp(Sub)
void Negate(){ Tos = -Tos; } pp(Negate)
void Invert(){ Tos = ~Tos; } pp(Invert)
void i1Add(){ Tos++; } pp(i1Add)
void i1Sub(){ Tos--; } pp(i1Sub)
void i2Add(){ Tos +=2; } pp(i2Add)
void i2Sub(){ Tos -=2; } pp(i2Sub)
void Mul(){ Tos *= *Stack++; } pp(Mul)
void Div(){ sCell tt=*Stack++; Tos = tt/Tos; } pp(Div)
void i2Mul(){ Tos *= 2; } pp(i2Mul)
void i2Div(){ Tos /= 2; } pp(i2Div)
void Mod(){ sCell tt=*Stack++; Tos = tt%Tos; } pp(Mod)
void UMul(){ Tos = (Cell) Tos * (Cell) *Stack++; } pp(UMul)
void UDiv(){ Cell tt=*Stack++; Tos = tt/(Cell)Tos; } pp(UDiv)
void And(){ Tos &= *Stack++; } pp(And)
void AndC(){ Tos = ~Tos & *Stack++; } pp(AndC)
void Or(){  Tos |= *Stack++; } pp(Or)
void Xor(){ Tos ^= *Stack++; } pp(Xor)
void ARshift(){  Tos = *Stack++ >> Tos ; } pp(ARshift)
void Rshift(){  Tos = *(Cell*)Stack++ >> Tos ; } pp(Rshift)
void Lshift(){  Tos = *Stack++ << Tos ; } pp(Lshift)

void HDot(){
    tty_printf("%x ",Tos);
    Tos = *Stack++;
}   pp(HDot)

void UDot() {
    __uint8_t buffer [44];
    __uint8_t* p = &buffer;

    size_t s = Tos;

    do {
        ++p;
        s = s / numericBase;
    } while(s);

    *p = ' ';
    p[1] = '\0';

    do {
	__uint8_t nn= (Tos % numericBase);
	if(nn<10)  *--p = '0' + nn;
	else  *--p = 'A' + nn - 10 ;
        Tos = Tos / numericBase;
    } while(Tos);
    tty_puts(&buffer);
    Tos = *Stack++; 
}   pp(UDot)

void Dot() {
 if(Tos<0){_tty_putchar('-',0); Negate();} 
 UDot();
}   pp(Dot)

void Load(){  Tos =  *(Cell*)Tos;  } pp(Load)
void Store(){ *(Cell*)Tos = *Stack++;  Tos = *Stack++;} pp(Store)
void CLoad(){ Tos =  (Cell)*(__uint8_t*) Tos;  } pp(CLoad)
void CStore(){ *(__uint8_t*)Tos = (__uint8_t)*Stack++; Tos = *Stack++;  } pp(CStore)
void CAddStore(){ *(__uint8_t*)Tos += (__uint8_t)*Stack++; Tos = *Stack++;  } pp(CAddStore)
void CStoreA(){ *(__uint8_t*)Tos = (__uint8_t)*Stack++; } pp(CStoreA)
void WLoad(){ Tos =  (Cell)*(__uint16_t*) Tos;  } pp(WLoad)
void WStore(){ *(__uint16_t*)Tos = (__uint16_t)*Stack++; Tos = *Stack++;  } pp(WStore)

void i2Store(){
 __uint64_t val = ((__uint64_t)(Cell)Stack[1]<<32) + (__uint64_t)(Cell)Stack[0];
   *(__uint64_t*)Tos = val;
   Stack += 2 ;  Tos = *Stack++;} pp(i2Store)

void i2Load(){  __uint64_t val = *(__uint64_t*)Tos; Tos= val; *--Stack=val>>32;} pp(i2Load)

void AddStore(){ *(Cell*)Tos += *Stack++;  Tos = *Stack++;} pp(AddStore)
void Count(){ *--Stack = Tos+1; Tos = (sCell) *(char *)Tos; } pp(Count)
void On(){  *(Cell*)Tos = -1; Tos = *Stack++; } pp(On)
void Off(){ *(Cell*)Tos = 0; Tos = *Stack++;  } pp(Off)
void Incr(){  *(Cell*)Tos += 1; Tos = *Stack++; } pp(Incr)
void ZEqual(){ Tos = -(Tos==0); } pp(ZEqual)
void ZNEqual(){ Tos = -(Tos!=0); } pp(ZNEqual)
void DZEqual(){  Tos = -( (Tos | *Stack++) == 0); } pp(DZEqual)
void ZLess(){ Tos = -(Tos<0); } pp(ZLess)
void Equal(){  Tos = -(*Stack++==Tos); } pp(Equal)
void NEqual(){  Tos = -(*Stack++!=Tos); } pp(NEqual)
void Less(){   Tos = -(*Stack++<Tos);  } pp(Less)
void Great(){  Tos = -(*Stack++>Tos);  } pp(Great)
void ULess(){  Tos = -((Cell)*Stack++ < (Cell)Tos); } pp(ULess)
void UGreat(){ Tos = -((Cell)*Stack++ > (Cell)Tos); } pp(UGreat)

void Max(){ sCell tt = *Stack++; if(tt>Tos) Tos=tt; } pp(Max)
void Min(){ sCell tt = *Stack++; if(tt<Tos) Tos=tt; } pp(Min)
void i0Max(){  if(Tos<0) Tos=0; } pp(i0Max)

void ToR(){   *--rStack = Tos; Tos = *Stack++; }	pp(ToR)
void RLoad(){ *--Stack = Tos; Tos = *rStack; }		pp(RLoad)
void FromR(){ *--Stack = Tos; Tos = *rStack++; }	pp(FromR)
void i2ToR(){  *--rStack = *Stack++; *--rStack = Tos ; Tos = *Stack++; } pp(i2ToR)
void i2RLoad(){ *--Stack = Tos; Tos = *rStack; *--Stack = rStack[1];	  } pp(i2RLoad)
void i2FromR(){ *--Stack = Tos; Tos = *rStack++; *--Stack = *rStack++;	  } pp(i2FromR)
void RDrop(){ *rStack++; }    pp(RDrop)
void RPGet(){ *--Stack = Tos; Tos = (Cell) rStack; } pp(RPGet)
void SPGet(){ *--Stack = Tos; Tos = (Cell) Stack ; } pp(SPGet)
void RPSet(){   rStack = (sCell*)Tos; Tos = *Stack++; } pp(RPSet)
void SPSet(){    Stack = (sCell*)(Tos+sizeof(sCell)); Tos = Stack[-1]; } pp(SPSet)

void ZCount(){ *--Stack= Tos; Tos = strlen((char *)Tos); } pp(ZCount)

void Punch() {punch();} pp(Punch)

void Emit() {
	drawRect(tty_pos_x, tty_pos_y, tty_off_pos_x, tty_off_pos_h, tty_bg_color);
	_tty_putchar((char)Tos,0); // punch();
 Tos = *Stack++; } pp(Emit)

void Space() {
	drawRect(tty_pos_x, tty_pos_y, tty_off_pos_x, tty_off_pos_h, tty_bg_color); // punch();	
	tty_pos_x += tty_off_pos_x;
 } pp(Space)

void Cr() { tty_putchar('\n',0); } pp(Cr)

void Type() {
	char * str = (char *) *Stack;
	drawRect(tty_pos_x, tty_pos_y, tty_off_pos_x*Tos, tty_off_pos_h, tty_bg_color);

    for (size_t i = 0; i < Tos; i++) {
        _tty_putchar(str[i], str[i+1]);
        if (isUTF(str[i])){
            i++;
        }
    } // punch();
	*Stack++;
    Tos =  *Stack++;
} pp(Type)

void ZType() {_tty_puts(Tos); Tos = *Stack++;} pp(ZType)

void SetXY() // ( x y -- )
{	tty_pos_y = Tos*tty_off_pos_h;
	tty_pos_x = *Stack++ * tty_off_pos_x ;
	Tos = *Stack++;
} pp(SetXY)

void GetXY() // ( -- x y )
{	*--Stack = Tos;
	Tos = tty_pos_y / tty_off_pos_h ;
	*--Stack = tty_pos_x /  tty_off_pos_x ;
} pp(GetXY)

extern uint32_t tty_pos_x;	///< Позиция на экране по X
extern uint32_t tty_pos_y;	///< Позиция на экране по Y

void Ahead(){ Co(~pBranch); *--Stack = Tos; Tos = (sCell)here; Co(0);} pp(Ahead)
void If(){ Co(~pQBranch); *--Stack = Tos; Tos= (sCell)here; Co(0);} pp(If)
void Then(){  *(sCell**)Tos++ = here; Tos = *Stack++; } pp(Then)
void Else(){  Ahead();    Swap(); Then(); } pp(Else)
void Begin(){ *--Stack = Tos; Tos =  (sCell)here; } pp(Begin)
void Until(){ Co(~pQBranch);   *here++ = (sCell)Tos; Tos = *Stack++; } pp(Until)
void Again(){ Co(~pBranch);  *here++ = (sCell)Tos; Tos = *Stack++; } pp(Again)
void While(){ If(); Swap(); } pp(While)
void Repeat(){ Again(); Then(); }   pp(Repeat)

void DNegate(){ __int64_t val =
 -(__int64_t)( ((__uint64_t)(Cell)Tos<<32) + (__uint64_t)(Cell)Stack[0] ) ;
	Tos= val>>32;
	Stack[0]=val;
  } pp(DNegate)

void DAbs(){   if(Tos<0) DNegate();  } pp(DAbs)

void DAdd()
{ __uint32_t sum= ((__uint64_t)(Cell)Tos<<32) + (__uint64_t)(Cell)Stack[0] +
	 ((__uint64_t)(Cell)Stack[1]<<32) + (__uint64_t)(Cell)Stack[2];
	Stack += 2 ;
	Tos= sum>>32;
	Stack[0]=sum;
} pp(DAdd)

void UMMul()
{ __uint64_t mul= (__uint64_t)(Cell)Tos * (__uint64_t)(Cell)Stack[0] ;
	Tos= mul>>32;
	Stack[0]=mul;
} pp(UMMul)

/* Divide 64-bit unsigned number (high half *b, low half *c) by
   32-bit unsigend number in *a. Quotient in *b, remainder in *c.
*/
static void udiv(__uint32_t a,__uint32_t *b,__uint32_t *c)
{
 __uint32_t d,qh,ql;
 int i,cy;
 qh=*b;ql=*c;d=a;
 if(qh==0) {
  *b=ql/d;
  *c=ql%d;
 } else {
  for(i=0;i<32;i++) {
   cy=qh&0x80000000;
   qh<<=1;
   if(ql&0x80000000)qh++;
   ql<<=1;
   if(qh>=d||cy) {
    qh-=d;
    ql++;
    cy=0;
   }
   *c=qh;
   *b=ql;
  }
 }
}
void UMMOD()
{	if(Tos<=*Stack) { /*overflow */
	*++Stack=-1;
	 Tos = -1; return;
        }
        udiv(Tos,Stack,&Stack[1]);
        Tos = *Stack++;
} pp(UMMOD)

void DIVMOD() // n1 n2 -- rem quot
{	sCell tt=*Stack;
        *Stack = tt%Tos ;
	Tos = tt/Tos;
} pp(DIVMOD)

void Align()
{   Cell sz = ( sizeof (Cell) - 1 ) ;
    char * chere = (char *)here;
    while( (Cell) chere & sz ) *chere++ = 0 ;
    here = (sCell *)chere;
}
// CODE FILL ( c-addr u char -- ) \ 94
void Fill()
{    Cell len =  *Stack++;
    __uint8_t *adr = (__uint8_t *) *Stack++;
  while (len-- > 0)  *adr++ = (__uint8_t)Tos;
  Tos =  *Stack++;
}  pp(Fill)

void Cmove()
{
  __uint8_t *c_to = (__uint8_t *) *Stack++;
  __uint8_t *c_from =(__uint8_t *) *Stack++;
  while (Tos-- > 0)
    *c_to++ = *c_from++;
  Tos =  *Stack++;
}  pp(Cmove)

void Cmove_up()
{
  __uint8_t *c_to = (__uint8_t *) *Stack++;
  __uint8_t *c_from =(__uint8_t *) *Stack++;
  while (Tos-- > 0)
    c_to[Tos] = c_from[Tos];
  Tos =  *Stack++;
}  pp(Cmove_up)

void StrComp(const char * s, sCell len)
{   char * chere = (char *)here;
    len &= 0xff ;
    *chere++ = (char)len;                /* store count byte */
    while (--len >= 0)          /* store string */
        *chere++ = *s++;

    here = (sCell *)chere;
    Align();
}

void StrCmp(){  StrComp((char *) *Stack++, Tos); Tos = *Stack++; } pp(StrCmp)

void Tp(const char * s) {
    Co(~pStr);
    StrComp(s, strlen(s));
    Co(~pType);
}

void SpSet(){    Stack = (sCell*)*Stack; } pp(SpSet)

sCell  ForthWordlist[] = {0,0,0};

const Cell ContextSize = 10;
sCell * Context[ContextSize] = {ForthWordlist};
sCell * Current[] = {ForthWordlist};

sCell * Last;
sCell * LastCFA;


void  WordBuild (const char * name, sCell cfa )
{
    LastCFA=here;
    Co(cfa);
    Co(0); // flg
    Co(** (sCell **) Current);
    Last=here;
    StrComp(name, strlen(name));
}

void Smudge(){ **(sCell***) Current=Last; } pp(Smudge)

void Immediate(){ Last[-2] |= 1; } pp(Immediate)

void FthItem (const char * name, sCell cfa ){
    WordBuild (name, cfa );
    Smudge();
}

sCell Header(const char * name) {
    FthItem (name,0);
    *(sCell **)LastCFA = here;
    return  *(sCell *)LastCFA;
}

sCell Variable (const char * name ) {
    FthItem(name,0);
    *(sCell **) LastCFA = here;
    *here++ =  pDoVar;
    *here++ = 0;
    return  *(sCell *)LastCFA;
}

sCell VVariable (const char * name, sCell val ) {
    FthItem(name,0);
    *(sCell **) LastCFA = here;
    *here++ = ~pDoVar;
    *here++ = val;
    return  *(sCell *)LastCFA;
}

sCell Constant (const char * name, sCell val ) {
    FthItem(name,0);
    *(sCell **) LastCFA = here;
    *here++ = ~pDoConst;
    *here++ = val;
    return  *(sCell *)LastCFA;
}
char atib[256]={"atib atib qwerty"};
sCell tib[]={0,(sCell)&atib}; PP(tib)
sCell ntib;
void Source(){
 *--Stack = Tos;
 *--Stack = tib[1];
    Tos =  ntib;
  } pp(Source)

void SourceSet(){
  ntib = Tos;
 tib[1] = *Stack++;
 Tos = *Stack++;
  } pp(SourceSet)

// ALLOCATE ( u -- a-addr ior ) 
void Allocate()
{
	*--Stack= Tos;
	
	*Stack= (sCell) malloc(Tos);
	Tos=0;
  	if(*Stack==0) Tos=-59;

} pp(Allocate)

void Free()
{
	kfree(Tos);
	Tos=0;

} pp(Free)

sCell i2in[] = {0 , 0  }; PP(i2in)
sCell *v2in = (sCell *) &i2in[1];

sCell SourceId[] = { 0, 0 }; PP(SourceId)

void Accept() // ( c-addr +n -- +n' )
{	keyboardctl(KEYBOARD_ECHO, true);
	*(char *)*Stack=0;
	gets_max((char *)*Stack,Tos);
	Tos=strlen((char *)*Stack);
	Stack++;
} pp(Accept)

void ParseName() {
    Cell addr,Waddr,Eaddr;
    addr=  tib[1] + *v2in;
    Eaddr= tib[1] + ntib;

    *--Stack = Tos;
    while (  addr<Eaddr ) { if( *(__uint8_t*)addr > ' ') break;
        addr++; }
    *--Stack=Waddr=addr;
    *v2in = addr - tib[1];
    while ( addr<=Eaddr ) { (*v2in)++; if( *(__uint8_t*)addr <= ' ') break;
     addr++; }
    Tos=addr-Waddr;
} pp(ParseName)

void Parse() {
    Cell addr,Waddr,Eaddr;
	if(((__uint8_t*)tib[1])[ntib] == '\r' ) ntib--;
    addr=  tib[1]  + *v2in;
    Eaddr= tib[1]  + ntib;

    char cc = (char)Tos;
    *--Stack=Waddr=addr;
    while ( addr<=Eaddr ) {  (*v2in)++;  if(*(__uint8_t*)addr == cc ) break;
        addr++;}
    Tos=addr-Waddr;
} pp(Parse)

#ifndef islower
__uint8_t islower (__uint8_t c)
{
    if  ( c >= 'a' && c <= 'z' ) return 1;
    return 0;
}
#endif

#ifndef toupper
__uint8_t toupper(__uint8_t c)
{
  return islower (c) ? c - 'a' + 'A' : c;
}
#endif

#ifndef memcasecmp

Cell memcasecmp (const void *vs1, const void *vs2, Cell n)
{
    unsigned int i;
    __uint8_t const *s1 = (__uint8_t const *) vs1;
    __uint8_t const *s2 = (__uint8_t const *) vs2;
    for (i = 0; i < n; i++)
    {
        __uint8_t u1 = *s1++;
        __uint8_t u2 = *s2++;
        if (toupper (u1) != toupper (u2))
            return toupper (u1) - toupper (u2);
    }
    return 0;
}
#endif

Cell CCompare( void * caddr1  ,  Cell len1 ,  void * caddr2  ,  Cell len2) {
    if (len1 < len2) return -1;
    if (len1 > len2) return  1;

//    auto cmpResult = std::memcmp(caddr1, caddr2, len1);
    auto cmpResult = memcasecmp(caddr1, caddr2, len1);

    if (cmpResult < 0) return -1;
    if (cmpResult > 0) return  1;
    return   0;
}

void UCompare(){ 
	char * caddr1 = (char *) *Stack++;
	sCell  len1 =  *Stack++;
	char * caddr2 = (char *) *Stack++;

    if (len1 != Tos) {  Tos -= len1; return; }

    Tos = memcasecmp(caddr1, caddr2, Tos);  } pp(UCompare)

char *SEARCH(char **wid,  char * word , Cell len)
{ char * addr= (char *) *wid;
    for(;;)
    {   if(!addr) return NULL;
        char * caddr = addr ;
        if( !CCompare(word, len, caddr+1, *caddr ))
            return  addr;
        addr = ((char **)addr)[-1];
    }
}

void FromName(){  Tos=((sCell *)Tos)[-3]; } pp(FromName)

void SearchWordList() // ( c-addr u wid --- 0 | xt 1 xt -1 )
{
    char ** addr=  (char **) Tos;
    Cell  len=Stack[0];
    char * word= (char * ) Stack[1];

    if(!addr) { Stack+=2; Tos=0; return; }
    Cell * nfa= (Cell*) SEARCH(addr,word,len);
    if(!nfa) {
        Stack+=2; Tos=0;
        return;
    }
    Stack++;
    Stack[0]=nfa[-3];
    Tos = nfa[-2]&1 ? 1 : -1;

}  pp(SearchWordList)

void SFind()
{	sCell * voc=  (sCell *) Context;
    *--Stack = Tos;
    while( *voc )
    {	*--Stack = Stack[1];
        *--Stack = Stack[1]; Tos=*voc;
        SearchWordList();
        if(Tos)
        {   Stack[2]=Stack[0];  Stack+=2; // 2nip
            return;
        }   voc++;
    }

} pp(SFind)

Cell State;

void StateQ(){ *--Stack= Tos; Tos = State; } pp(StateQ)

void IMode(){ State = 0;}  pp(IMode)
void CMode(){ State = -1;}  pp(CMode)

sCell * YDP;
sCell * YDP0;

sCell YDPFL[] = { pDoConst, 0 }; pp(YDPFL)

void QYDpDp()
{
  if(YDPFL[1] == 0) return;
   sCell * tmp = YDP ;
    YDP = here ;
    here = tmp ;
}

void SBuild()
{    char * name = (char * ) *Stack++ ;
	QYDpDp();
    LastCFA=here;
    Co(0);
    Co(0); // flg
    Co(** (sCell  **) Current);
    Last=here;
    StrComp(name, Tos);
    Tos = *Stack++;
	QYDpDp();
    *(sCell **)LastCFA = here;
}

void Build()
{ //   *--Stack = Tos; Tos=(sCell)pNoop;
    ParseName();
    SBuild();
} pp(Build)

void SHeader()
{
	SBuild();
	Smudge();  
} pp(SHeader)

void SNumber0() // ( str len -- m flg )
{
    char* rez;
    char  NumStr[44];
    sCell signedFlg = 1;
    Cell len = Tos;
    char * caddr = (char*) Stack[0];
    if(caddr[0]=='-') { len--; caddr++; signedFlg = -1; }
    NumStr[len]=0;
    while(len){ --len; NumStr[len] = caddr[len]; }
    *Stack = strtoul( NumStr,  &rez, numericBase) * signedFlg;
    Tos =  strlen(rez);
}  pp(SNumber0)

void Colon(){
  Build();
  CMode(); } pp( Colon)
void Semicolon(){ Co(~pExit); Smudge(); IMode(); } pp(Semicolon)

void to_catch(){
    *--rStack = (sCell)Handler;
    *--rStack = (sCell)Stack;
    Handler = rStack;
    Execute();
} pp(to_catch)

void from_catch(){
    rStack++;
    Handler = (sCell*)*rStack++;
    *--Stack = Tos;  Tos = 0;
    ip = (sCell*)*rStack++; // exit
} pp(from_catch)

sCell Catch[] = { 0,0 }; PP(Catch)

void FThrowDo()
{   *--Stack = Tos;
    if (Handler == NULL); //  TODO("Handler=0")
    rStack =   Handler ;
    Stack = (sCell*)*rStack++;
    Handler = (sCell*)*rStack++;
    ip = (sCell * ) *rStack++;
}

void FThrow(){
    if (Tos == 0){  Tos = *Stack++; return;  }
    FThrowDo();
} pp(FThrow)

sCell Lastin =0;
sCell SaveErrQ = -1;
sCell ErrIn;

void SaveErr0()
{ if(SaveErrQ & Tos )
    {  SaveErrQ = 0;
       ErrIn = *v2in ;
    }

} pp(SaveErr0)


void PrintErr0()
{  numericBase = 10;
     _tty_printf("Err=%d\n",Tos);
     Tos = *Stack++;
     SaveErrQ=-1;
} pp(PrintErr0)

// R/O ( -- fam )
void readOnly() { *--Stack = Tos; Tos = O_READ; }  pp(readOnly)

// R/W ( -- fam )
void readWrite() { *--Stack = Tos; Tos = O_WRITE | O_READ; } pp(readWrite)

// W/O ( -- fam )
void writeOnly() { *--Stack = Tos; Tos = O_WRITE ; } pp(writeOnly)


/**
 * @brief Структура файла. Требуется для работы с VFS
 * 
 */
typedef struct FILEID {
	FILE fl;
        char filename[0];
} FILEID;


// OPEN-FILE ( c-addr u fam -- fileid ior )

void openFile() {
    Cell flen = *Stack++;
    Cell plen = 0;
    char * caddr = (char*) *Stack;

	if(caddr[1]!=':') plen = strlen(&G_CLI_PATH);
	
	FILE* file = kcalloc(sizeof(FILE)+plen+flen+1, 1);

	char * filename =&((FILEID*)file)->filename[0];

	filename[plen+flen]=0;
    
    while(flen){ --flen; filename[plen+flen] = caddr[flen]; }

    while(plen){ --plen; filename[plen] = G_CLI_PATH[plen]; }

	if(Tos & 0x8000 )  nvfs_create(filename, 0);
	FSM_FILE finfo = nvfs_info(filename);
	if (finfo.Ready == 0) {
        //kfree(file);
        qemu_err("Failed to open file: %s (Exists: %d)",
			filename,
			finfo.Ready);
		Tos = -69;
		return ;
	}

	file->open = 1;		// Файл успешно открыт
	file->fmode = Tos;	// Режим работы с файлом
	file->size = finfo.Size;// Размер файла
	file->path = filename;	// Полный путь к файлу
	file->pos = 0;		// Установка указателя в самое начало
	file->err = 0;		// Ошибок в работе нет

	*Stack =(sCell)file;
	Tos = 0;

} pp(openFile)



// CLOSE-FILE ( fileid -- ior )
void closeFile() { fclose(Tos); Tos = 0; } pp(closeFile)

// READ-FILE ( c-addr u1 fileid -- u2 ior )
void readFile() {
	
    Cell len = *Stack++;
    char * buffer = (char*) *Stack;
    FILE* file = (FILE*) Tos;
    if( len > (file->size - file->pos)) len = file->size - file->pos;
    if(!len) { *Stack=0; Tos = 0; return; }

    *Stack = fread( file , 1, len, buffer);
	Tos = 0;
	if(*Stack==-1) Tos = -70; 

} pp(readFile)

// READ-LINE ( c-addr u1 fileid -- u2 flag ior )
void readLine() {

	FILE* file = (FILE*) Tos;

	if(file->pos == file->size){ Stack[1]=*Stack=Tos=0; return;}
	
	Cell len = *Stack;
	char * buffer = (char*) Stack[1];
	if( len > (file->size+1 - file->pos)) len = file->size+1 - file->pos;

	*Stack = fread( file , 1, len, buffer);
	Tos = 0;
	if(*Stack==-1){  Tos = -71; return; }
	file->pos -= len;
    	len = 0;
	while(file->size > file->pos)
	{  file->pos++;
	 if(buffer[len]=='\n'){ break;}
	  len++;
	}
     	if(buffer[len]=='\r') len--;
	Stack[1]=len;
	*Stack=-1;
	Tos=0;

} pp(readLine)

// WRITE-FILE ( c-addr u1 fileid -- ior )
void writeFile() {
	
    Cell len = *Stack++;
    char * buffer = (char*) *Stack++;
    FILE* file = (FILE*) Tos;

    Tos = fwrite( file , len, 1, buffer);
	Tos = 0; // &= -70;

} pp(writeFile)

// RESIZE-FILE ( ud fileid -- ior ) 

void resizeFile() {
    FILE* file = (FILE*) Tos;
   file->size = *Stack++;
   Tos=0;
} pp(resizeFile)


    char filename[111];	

//int tshell();
void  Test1(void) {
// tshell();
 }  pp(Test1)

void  Test2(void) {

	const char* filename = "T:/filename.txt";
	if(touch(filename))
	{ FILE* file;
	  file = fopen(filename, "r");
	  if(file) 
  	  {  fwrite(file, 5 , 1, "bytes");
	     fclose(file);  	  
  	  }
  	  else
  	  { tty_printf("fopen %s err\n",filename);
  	  }
	}
	else
	{ tty_printf("touch err\n");
	}
	
	filename = "T:/filenameq.txt";
	if(touch(filename))
	{ FILE* file;
	  file = fopen(filename, "r");
	  if(file) 
  	  {  fwrite(file, 5 , 1, "bytes");
	     fclose(file);  	  
  	  }
  	  else
  	  { tty_printf("fopen %s err\n",filename);
  	  }
	}
	else
	{ tty_printf("touch err\n");
	}

 tty_printf("Test2\n"); }  pp(Test2)

void  Test3(void) {


	FILE* file_ = (FILE*)Tos;


    size_t filesize = fsize(file_);

    uint8_t* buffer = kcalloc(1,filesize + 1);

    fread(file_, 1, filesize, buffer);

	tty_printf("%s", buffer);

    fclose(file_);

	kfree(buffer);

	Drop();

 }  pp(Test3)



void  Bye(void) {forth_run=0;}  pp(Bye)

const char *initScript =
        " : 2NIP 2SWAP 2DROP ;\n"
        " : COMPILE, , ;\n"
        " : HEX 16 BASE ! ;\n"
        ": DECIMAL 10 BASE ! ;\n"
        ": HEADER BUILD SMUDGE ;\n"
        ": CONSTANT HEADER DOCONST , , ;\n"
        ": CREATE HEADER DOVAR , ;\n"
        ": VARIABLE CREATE 0 , ;\n"
        ": [COMPILE] ' , ; IMMEDIATE\n"
        ": CELL+ CELL + ;\n"
        ": CELL- CELL - ;\n"
        ": CELLS CELL * ;\n"
        ": >BODY CELL+ ;\n"
        ": COMPILE R> DUP @ , CELL+ >R ;\n"
        ": CHAR  PARSE-NAME DROP C@ ;\n"
        ": [CHAR] CHAR LIT,  ; IMMEDIATE\n"
        ": [']  ' LIT, ; IMMEDIATE\n"
        ": .( [CHAR] ) PARSE TYPE ; IMMEDIATE\n"
        ": ( [CHAR] ) PARSE 2DROP ; IMMEDIATE\n"
        ": SLIT, ( string -- ) COMPILE <$> $, ;\n"
        ": \\ 10 PARSE 2DROP  ; IMMEDIATE\n"
        ": .\\ 10 PARSE TYPE cr ; IMMEDIATE\n"
        ": .\" [CHAR] \" PARSE SLIT, COMPILE TYPE   ; IMMEDIATE\n"
        ": S\" [CHAR] \" PARSE ?STATE IF SLIT, THEN ; IMMEDIATE\n"
        ": ABORT -1 THROW ;\n"
        ": POSTPONE\n" // 94
        "  PARSE-NAME SFIND DUP\n"
        "  0= IF -321 THROW THEN \n"
        "  1 = IF COMPILE,\n"
        "      ELSE LIT, ['] COMPILE, COMPILE, THEN\n"
        "; IMMEDIATE\n"
        ": TO '\n"
        "   ?STATE 0= IF >BODY ! EXIT THEN\n"
        "    >BODY LIT, POSTPONE ! ; IMMEDIATE\n"
	": ERASE 0 FILL ;\n"
	": $!\n" //	( addr len dest -- )
	"SWAP 255 AND SWAP	2DUP C! 1+ SWAP CMOVE ;\n"
        ": DEFER@  ( xt1 -- xt2 )  >BODY @ ;\n"
        ": VALUE CONSTANT ;\n"
        ": (DO)   ( n1 n2 ---)\n"
        // Runtime part of DO.
        " R> ROT ROT SWAP >R >R >R ;\n"
        ": (?DO)  ( n1 n2 ---)\n"
        // Runtime part of ?DO
        "  OVER OVER - IF R> ROT ROT SWAP >R >R CELL+ >R \n"
        "                 ELSE DROP DROP R> @ >R\n" // Jump to leave address if equal
        "                 THEN ;\n"
        ": I ( --- n )\n"
        // Return the counter (index) of the innermost DO LOOP
        "  POSTPONE R@ ; IMMEDIATE\n"
                ": z\\ 10 PARSE h. h. ; IMMEDIATE\n"

        ": J  ( --- n)\n"
        // Return the counter (index) of the next loop outer to the innermost DO LOOP
        " RP@ 3 CELLS + @ ;\n"
        "VARIABLE 'LEAVE ( --- a-addr)\n" // This variable is  used  for  LEAVE address resolution.

        ": (LEAVE)   ( --- )\n"
        // Runtime part of LEAVE
        " R> @ R> DROP R> DROP >R ;\n" // Remove loop parameters and replace top of ret\n"
        // stack by leave address.\n"

        ": UNLOOP ( --- )\n"
        // Remove one set of loop parameters from the return stack.
        "   R> R> DROP R> DROP >R ;\n"

        ": (LOOP) ( ---)\n"
        // Runtime part of LOOP
        "  R> R> 1+ DUP R@ = \n"   // Add 1 to count and compare to limit.
        "  IF \n"
        "   R> DROP DROP CELL+ >R\n" // Discard parameters and skip leave address.
        "  ELSE \n"
        "   >R @ >R\n" // Repush counter and jump to loop start address.
        "  THEN ;\n"

        ": (+LOOP) ( n ---)\n"
        // Runtime part of +LOOP
        // Very similar to (LOOP), but the compare condition is different.
        //  exit if ( oldcount - lim < 0) xor ( newcount - lim < 0).
        "     R> SWAP R> DUP R@ - ROT ROT + DUP R@ - ROT XOR 0 < \n"
        "     IF R> DROP DROP CELL+ >R\n"
        "     ELSE >R @ >R THEN ;\n"

        ": DO ( --- x)\n"
        // Start a DO LOOP.
        // Runtime: ( n1 n2 --- ) start a loop with initial count n2 and
        // limit n1.
        "  POSTPONE (DO) 'LEAVE @  HERE 0 'LEAVE ! \n"
        "   ; IMMEDIATE\n"

        ": ?DO  ( --- x )\n"
        // Start a ?DO LOOP.\n"
        // Runtime: ( n1 n2 --- ) start a loop with initial count n2 and
        // limit n1. Exit immediately if n1 = n2.
        "  POSTPONE (?DO)  'LEAVE @ HERE 'LEAVE ! 0 , HERE ; IMMEDIATE\n"

        ": LEAVE ( --- )\n"
        // Runtime: leave the matching DO LOOP immediately.
        // All places where a leave address for the loop is needed are in a linked\n"
        // list, starting with 'LEAVE variable, the other links in the cells where
        // the leave addresses will come.
        "  POSTPONE (LEAVE) HERE 'LEAVE @ , 'LEAVE ! ; IMMEDIATE\n"
        ": RESOLVE-LEAVE\n"
        // Resolve the references to the leave addresses of the loop.
        "         'LEAVE @\n"
        "         BEGIN DUP WHILE DUP @ HERE ROT ! REPEAT DROP ;\n"

        ": LOOP  ( x --- )\n"
        // End a DO LOOP.
        // Runtime: Add 1 to the count and if it is equal to the limit leave the loop.
        " POSTPONE (LOOP) ,  RESOLVE-LEAVE  'LEAVE ! ; IMMEDIATE\n"

        ": +LOOP  ( x --- )\n"
        // End a DO +LOOP
        // Runtime: ( n ---) Add n to the count and exit if this crosses the
        // boundary between limit-1 and limit.
        " POSTPONE (+LOOP) , RESOLVE-LEAVE 'LEAVE ! ; IMMEDIATE\n"

        ": (;CODE) ( --- )\n"
        // Runtime for DOES>, exit calling definition and make last defined word
        // execute the calling definition after (;CODE)
        "  R> LAST @  NAME>  ! ;\n"

        ": DOES>  ( --- )\n"
        // Word that contains DOES> will change the behavior of the last created
        // word such that it pushes its parameter field address onto the stack
        // and then executes whatever comes after DOES>
        " POSTPONE (;CODE) \n"
        " POSTPONE R>\n" // Compile the R> primitive, which is the first
        // instruction that the defined word performs.
        "; IMMEDIATE\n"

    ": SET-CURRENT ( wid -- )\n" // 94 SEARCH
    "        CURRENT ! ;\n"

    ": GET-CURRENT ( -- wid )\n" // 94 SEARCH
    "        CURRENT @ ;\n"

    ": GET-ORDER ( -- widn ... wid1 n )\n"  // 94 SEARCH
        " SP@ >R 0 >R\n"
        " CONTEXT\n"
        " BEGIN DUP @ ?DUP\n"
        " WHILE >R CELL+\n"
        " REPEAT  DROP\n"
        " BEGIN R> DUP 0=\n"
        " UNTIL DROP\n"
        "R> SP@ - CELL / 1- ; \n"

	" HERE S\" FORTH\" $, FORTH-WORDLIST CELL+ !\n"

        ": VOC-NAME. ( wid -- )\n"
        "DUP CELL+ @ DUP IF COUNT TYPE BL EMIT DROP ELSE DROP .\" <NONAME>:\" U. THEN ;\n"

        ": ORDER ( -- )\n" // 94 SEARCH EXT
        "GET-ORDER .\" Context: \" \n"
        "0 ?DO ( DUP .) VOC-NAME. SPACE LOOP CR\n"
        ".\" Current: \" GET-CURRENT VOC-NAME. CR ;\n"

        ": SET-ORDER ( wid1 ... widn n -- )\n"
        "DUP -1 = IF\n"
        "DROP  FORTH-WORDLIST 1\n"
        "THEN\n"
        "DUP  CONTEXT-SIZE  U> IF -49 THROW THEN\n"
        "DUP CELLS context + 0!\n"
        "0 ?DO I CELLS context + ! LOOP ;\n"
        "CREATE VOC-LIST FORTH-WORDLIST CELL+ CELL+ ,\n"

        ": FORTH FORTH-WORDLIST CONTEXT ! ;\n"
        ": DEFINITIONS  CONTEXT @ CURRENT ! ;\n"

        ": WORDLIST ( -- wid )\n" // 94 SEARCH
        " HERE 0 , 0 , \n"
        " HERE VOC-LIST  @ , .\" W=\" DUP H.  VOC-LIST ! ;\n"

	": ONLY ( -- ) -1 SET-ORDER ;\n"
	": ALSO ( -- )   GET-ORDER OVER SWAP 1+ SET-ORDER ;\n"
	": PREVIOUS ( -- ) GET-ORDER NIP 1- SET-ORDER ;\n"


	": LATEST ( -> NFA ) CURRENT @ @ ;\n"

	": VOCABULARY ( <spaces>name -- )\n"
	"WORDLIST CREATE DUP ,\n"
	"LATEST SWAP CELL+ !\n"
	"DOES>  @ CONTEXT ! ;\n"
	" VARIABLE CURSTR\n"

	": ->DEFER ( cfa <name> -- )  HEADER DODEFER , , ;\n"
	": DEFER ( <name> -- ) ['] ABORT ->DEFER ;\n"

        ": VECT DEFER ;\n"

	": FQUIT  BEGIN REFILL WHILE CURSTR 1+!\n"
	"  INTERPRET  REPEAT ;\n"

	": LALIGNED  3 + 3 ANDC ;\n"

	" 255 CONSTANT TC/L\n"

 ": INCLUDE-FILE\n" // ( fid --- )
// Read lines from the file identified by fid and interpret them.
// INCLUDE and EVALUATE nest in arbitrary order.
	"SOURCE-ID >R >IN @ >R LASTIN @ >R CURSTR @ >R CURSTR 0!\n"
	"SOURCE 2>R\n"
	" TC/L ALLOCATE THROW TC/L SOURCE!\n"
	"TO SOURCE-ID\n"
	"['] FQUIT CATCH SAVEERR\n"
	"TIB FREE DROP\n"
	"2R> SOURCE!\n"

	"R> CURSTR ! R> LASTIN ! R> >IN ! R> TO SOURCE-ID\n"
	"THROW ;\n"

	": FREFILL0\n" // (  -- flag )
	"  TIB TC/L SOURCE-ID READ-LINE THROW\n"
	"  SWAP  #TIB !  0 >IN ! CURSTR 1+!\n"
	"  0 SOURCE + C! ;\n"
	"' FREFILL0 TO FREFILL\n"

  "444 CONSTANT  CFNAME_SIZE\n"
  "CREATE CURFILENAME  CFNAME_SIZE 255 + 1+ ALLOT\n"
  "CURFILENAME  CFNAME_SIZE 255 + 1+  ERASE\n"

  ": CFNAME-SET\n" // ( adr len -- )
  "DUP 1+ >R  CURFILENAME CURFILENAME R@ + CFNAME_SIZE R> - CMOVE>\n"
  "CURFILENAME $! ;\n"

  ": CFNAME-FREE\n" //  ( -- )
  "CURFILENAME COUNT + CURFILENAME\n"
  "CFNAME_SIZE CURFILENAME C@ - 255 +  CMOVE ;\n"

 ": INCLUDED\n" // ( c-addr u ---- )
 "2DUP CFNAME-SET\n"
 "R/O OPEN-FILE THROW\n"
 "DUP >R ['] INCLUDE-FILE CATCH\n"
 "DUP IF cr .\" in <\" CURFILENAME COUNT TYPE .\" >\" CURSTR @ . THEN  CFNAME-FREE\n"
 "R> CLOSE-FILE DROP THROW ;\n"

 ": EVALUATE\n" // ( i*x c-addr u -- j*x ) \ 94
 "SOURCE-ID >R SOURCE 2>R >IN @ >R\n"
 "-1 TO SOURCE-ID\n"
 "SOURCE! >IN 0!\n"
 "['] INTERPRET CATCH\n"
 "R> >IN ! 2R> SOURCE! R> TO SOURCE-ID\n"
 "THROW ;\n"

 ": FLOAD PARSE-NAME INCLUDED ;\n"

 ": [DEFINED]\n" //  ( -- f ) \ "name"
 "PARSE-NAME  SFIND  IF DROP -1 ELSE 2DROP 0 THEN ; IMMEDIATE\n"

 ": [UNDEFINED]\n" //  ( -- f ) \ "name"
 "POSTPONE [DEFINED] 0= ; IMMEDIATE\n"

 ": \\+	POSTPONE [UNDEFINED]	IF POSTPONE \\ THEN ; IMMEDIATE\n"
 ": \\-	POSTPONE [DEFINED]	IF POSTPONE \\ THEN ; IMMEDIATE\n"

 ": BREAK  POSTPONE EXIT POSTPONE THEN ; IMMEDIATE\n"

 ": PRIM? 0< ['] DUP 0< = ;\n"

 ": ?CONST\n" // ( cfa -- cfa flag )
 "DUP PRIM? IF 0 BREAK\n"
 "DUP @ DOCONST = ;\n"

 ": ?VARIABLE\n" // ( cfa -- cfa flag )
 "DUP PRIM? IF 0 BREAK\n"
 "DUP @ DOVAR = ;\n"

  "S\" autoexec.4th\" INCLUDED"
;

void  InitStringSet()
{ 	tib[1]=(__uint32_t)initScript;
	ntib=strlen(initScript);
    *v2in = 0;
} pp(InitStringSet)


void  pek()
{ kbdstatus=0;
  while(!kbdstatus);
}


void  KeyQ()
{ *--Stack= Tos;
	Tos = inb(PS2_STATE_REG)&1;
} pp(KeyQ)

void  Key()
{   __uint8_t cc[1];
	keyboardctl(KEYBOARD_ECHO, false);
  *--Stack= Tos;
	gets_max(&cc,1);
  Tos= (Cell) cc[0] ;

} pp(Key)

void LastKey()
{
  *--Stack= Tos;
 Tos = getCharRaw();
} pp(LastKey)

void ChLastKey()
{
 static lgetCharRaw = 0;
  *--Stack= Tos;
  do{
  while(Tos==lgetCharRaw) Tos = getCharRaw() ;
  lgetCharRaw=Tos;
  }while(!Tos);

} pp(ChLastKey)

void KBctl()
{
  keyboardctl(Tos,*Stack++);
  Tos = *Stack++;
}  pp(KBctl)

extern int lastKey;

void ScanKey()
{

    static bool kmutex = false;
    mutex_get(&kmutex, true);

//    keyboardctl(KEYBOARD_ECHO, false);

  *--Stack= Tos;

    while(lastKey==0 || (lastKey & 0x80)) { sleep_ms(11); }
    Tos = lastKey;
    lastKey = 0;

//    keyboardctl(KEYBOARD_ECHO, true);
    mutex_release(&kmutex);

} pp(ScanKey)

void Scan2Un()
{
 Tos = *(uint16_t*)getCharKeyboard(Tos, false);

} pp(Scan2Un)


extern bool    SHIFT,key_alt;

void QShift()
{
  *--Stack= Tos;
 Tos = SHIFT;
} pp(QShift);

void QCtrl()
{
  *--Stack= Tos;
 Tos = is_lctrl_key();
} pp(QCtrl);

void QAlt()
{
  *--Stack= Tos;
 Tos = key_alt;
} pp(QAlt);

extern uint32_t framebuffer_height;			///< Высота экрана

uint32_t CursorHSize = 5;

void Cursor()
{
    int ox = 0, oy = 0;
        oy = getPosY();
	if( (oy+tty_off_pos_h-3) > framebuffer_height ) return;
	ox = getPosX();
	uint8_t* pixels = framebuffer_addr + (ox * (framebuffer_bpp >> 3)) + (oy+tty_off_pos_h-3) * framebuffer_pitch;
	
	uint32_t ii = framebuffer_bpp;
	while(ii--) pixels[ii] ^= 255;

	uint32_t jj = CursorHSize;
	while(jj--)
	{	pixels -= framebuffer_pitch;
		ii = framebuffer_bpp;
		while(ii--) pixels[ii] ^= 255;
	}

} pp(Cursor)


void  OpenDir() // ( c-addr lem -- dir-id flag )
{   Cell dlen = Tos;
    Cell plen = 0;
    char * caddr = (char*) *Stack;

	if(caddr[1]!=':') plen = strlen(&G_CLI_PATH);
	
	char * path = kcalloc(plen+dlen+1, 1);

	path[plen+dlen]=0;
    
    while(dlen){ --dlen; path[plen+dlen] = caddr[dlen]; }

    while(plen){ --plen; path[plen] = G_CLI_PATH[plen]; }

	 FSM_DIR* Dir = nvfs_dir(path);
	Tos = -(Dir->Ready != 1) ;
	*Stack = (Cell)Dir;

} pp(OpenDir)

void  DirI2Name() // ( dir-id n -- z-addr )
{     Tos = (Cell)((FSM_DIR*)*Stack++)->Files[Tos].Name ;
} pp(DirI2Name)

void  DirI2Type() // ( dir-id n -- n )
{     Tos = ((FSM_DIR*)*Stack++)->Files[Tos].Type ;
} pp(DirI2Type)

void  Dir2Count() // ( dir-id -- n )
{     Tos = (Cell)((FSM_DIR*)Tos)->Count ;
} pp(Dir2Count)

void  CloseDir() // ( dir-id -- flg )
{      FSM_DIR* Dir = (FSM_DIR*)Tos;
	/// Сначала чистим массив внутри массива
	kfree(Dir->Files);
	/// А потом только основной массив
	kfree(Dir);
	Tos = 0;
} pp(CloseDir)

void  ZCli()
{   cli_handler( (char*) Tos);
    Tos = *Stack++;
}  pp(ZCli)



void  MakeImag(void)
{
//     tty_printf("MakeImag run\n");
    FthItem("NOOP",~pNoop );
    FthItem("+",~pAdd );
    FthItem("-",~pSub );
    FthItem("D+",~pDAdd );
    FthItem("1+",~pi1Add );
    FthItem("1-",~pi1Sub );
    FthItem("2+",~pi2Add );
    FthItem("2-",~pi2Sub );
    FthItem("INVERT",~pInvert);
    FthItem("NEGATE",~pNegate);
    FthItem("DNEGATE",~pDNegate);
    FthItem("DABS",~pDAbs);
    FthItem("*",~pMul);
    FthItem("/",~pDiv);
    FthItem("2*",~pi2Mul);
    FthItem("2/",~pi2Div);
    FthItem("MOD",~pMod);
    FthItem("U*",~pUMul);
    FthItem("U/",~pUDiv);
    FthItem("UM*",~pUMMul);
    FthItem("UM/MOD",~pUMMOD);
    FthItem("/MOD",~pDIVMOD);
    FthItem("AND",~pAnd);
    FthItem("ANDC",~pAndC);
    FthItem("OR",~pOr);
    FthItem("XOR",~pXor);
    FthItem("ARSHIFT",~pARshift);
    FthItem("RSHIFT",~pRshift);
    FthItem("LSHIFT",~pLshift);
    FthItem("DUP",~pDup );
    FthItem("CS-DUP",~pDup );
    FthItem("?DUP",~pQDup );
    FthItem("OVER",~pOver );
    FthItem("CS-OVER",~pOver );
    FthItem("TUCK",~pTuck );
    FthItem("PICK",~pPick );
    FthItem("CS-PICK",~pPick );
    FthItem("SWAP",~pSwap );
    FthItem("CS-SWAP",~pSwap );
    FthItem("2SWAP",~pi2Swap );
    FthItem("ROT",~pRot );
    FthItem("DROP",~pDrop );
    FthItem("NIP",~pNip );
    FthItem("2DROP",~pi2drop );
    FthItem("2DUP",~pi2dup );
    FthItem("2OVER",~pi2over);
    FthItem(".",~pDot);
    FthItem("U.",~pUDot);
    FthItem("H.",~pHDot);
    FthItem("CATCH",PCatch);
    FthItem("THROW",~pFThrow);
    FthItem("[",~pIMode); Immediate();
    FthItem("]",~pCMode);
    FthItem("@",~pLoad);
    FthItem("C@",~pCLoad);
    FthItem("C!",~pCStore);
    FthItem("C+!",~pCAddStore);
    FthItem("C!A",~pCStoreA);
    FthItem("W@",~pWLoad);
    FthItem("W!",~pWStore);
    FthItem("2!",~pi2Store);
    FthItem("2@",~pi2Load);
    FthItem("COUNT",~pCount);
    FthItem("!",~pStore);
    FthItem("+!",~pAddStore);
    FthItem("1+!",~pIncr);
    FthItem("0!",~pOff);
    FthItem("OFF",~pOff);
    FthItem("ON",~pOn);
    FthItem("=",~pEqual);
    FthItem("<>",~pNEqual);
    FthItem("0<",~pZLess);
    FthItem("0=",~pZEqual);
    FthItem("0<>",~pZNEqual);
    FthItem("D0=",~pDZEqual);
    FthItem("<",~pLess);
    FthItem(">",~pGreat);
    FthItem("U<",~pULess);
    FthItem("U>",~pUGreat);
    FthItem("MAX",~pMax);
    FthItem("MIN",~pMin);
    FthItem("0MAX",~pi0Max);
    FthItem(">R",~pToR);
    FthItem("R>",~pFromR);
    FthItem("RDROP",~pRDrop);
    FthItem("R@",~pRLoad);
    FthItem("2>R",~pi2ToR);
    FthItem("2R>",~pi2FromR);
    FthItem("2R@",~pi2RLoad);
    FthItem("RP@",~pRLoad);
    FthItem("RP@",~pRPGet);
    FthItem("SP@",~pSPGet);
    FthItem("RP!",~pRPSet);
    FthItem("SP!",~pSPSet);
    FthItem(",",~pCompile);
    FthItem("ALLOT",~pAllot);
    FthItem("$,",~pStrCmp);
    FthItem("<$>",~pStr);
    FthItem("EXECUTE",~pExecute);
    FthItem("SMUDGE",~pSmudge);
    FthItem("TYPE",~pType);
    FthItem("ZTYPE",~pZType);
    FthItem("CR",~pCr);
    FthItem("SPACE",~pSpace);
    FthItem("EMIT",~pEmit);
    FthItem("PUNCH",~pPunch);
    FthItem(">IN",Pi2in);
    FthItem("PARSE-NAME",~pParseName);
    FthItem("PARSE",~pParse);
    FthItem("SHEADER",~pSHeader);
    FthItem("BUILD",~pBuild);
    FthItem("SFIND",~pSFind);
    FthItem("SEARCH-WORDLIST",~pSearchWordList);
    FthItem("UCOMPARE",~pUCompare);
    FthItem("FILL",~pFill);
    FthItem("CMOVE",~pCmove);
    FthItem("CMOVE>",~pCmove_up);
    FthItem("ZCOUNT",~pZCount);

    FthItem("KEY?",~pKeyQ);
    FthItem("KEY",~pKey);
    sCell PKey = Header("KEY");  Co2(~pDoDefer,~pKey);

    FthItem("LASTKEY",~pLastKey);
    FthItem("CHLASTKEY",~pChLastKey);
    FthItem("SCANKEY",~pScanKey);
    FthItem("KEYBCTL",~pKBctl);
    FthItem("SCAN2UN",~pScan2Un);
    FthItem("CURSOR",~pCursor);
    Constant("CURSOR%",(sCell)&CursorHSize);


    FthItem("SHIFT?",~pQShift);
    FthItem("CTL?",~pQCtrl);
    FthItem("ALT?",~pQAlt);


    FthItem("TEST1",~pTest1);
    FthItem("TEST2",~pTest2);
    FthItem("TEST3",~pTest3);

    FthItem("IMMEDIATE",~pImmediate);
    FthItem(":",~pColon);
    FthItem(";",~pSemicolon);   Immediate();
    FthItem("IF",~pIf);         Immediate();
    FthItem("ELSE",~pElse);     Immediate();
    FthItem("THEN",~pThen);     Immediate();
    FthItem("BEGIN",~pBegin);   Immediate();
    FthItem("UNTIL",~pUntil);   Immediate();
    FthItem("AGAIN",~pAgain);   Immediate();
    FthItem("WHILE",~pWhile);   Immediate();
    FthItem("REPEAT",~pRepeat); Immediate();

    sCell PTrue = Constant("TRUE",-1);

    FthItem("EXIT",~pExit );
    Constant("STATE",(sCell) &State );
    FthItem("?STATE",~pStateQ);

    Constant("DOVAR",~pDoVar );
    Constant("DOCONST",~pDoConst );
    Constant("DODEFER",~pDoDefer );
    Constant("DP", (sCell)&here );
    Constant("LAST", (sCell)&Last );
    Constant("LASTCFA", (sCell)&LastCFA );
    VVariable("WARNING",-1);
    FthItem("HERE",~pHere);
    Constant("BL",(sCell)' ' );
    sCell PCell = Constant("CELL",sizeof(Cell) );

    FthItem("NAME>",~pFromName);
    Constant("BASE",(sCell)&numericBase);

    Header("'");   Co5(~pParseName,~pSFind,~pZEqual,~pFThrow,~pExit);

    Constant("STATE",(sCell) &State );
    sCell PHi = Header("HI"); Tp("Hello!!!"); Co(~pExit);
    sCell PLastin = Constant("LASTIN", (sCell)&Lastin );
    sCell PSaveErrQ = Constant("SAVEERR?", (sCell)&SaveErrQ );

    FthItem("SAVEERR0",~pSaveErr0);
    sCell PSaveErr = Header("SAVEERR");  Co2(~pDoDefer,~pSaveErr0);
    FthItem("PRINTERR0",~pPrintErr0);

    sCell PContext = Constant("CONTEXT",(sCell) &Context );
    Constant("CURRENT",(sCell) &Current );
    Constant("IMAGE-BEGIN",(sCell)HereArea );
    Constant("FORTH-WORDLIST",(sCell) &ForthWordlist );
    Constant("CONTEXT-SIZE",ContextSize );
    sCell PSP0 = VVariable("SP0",(sCell) &StackArea[STACK_SIZE-9] );

    FthItem("R/O",~preadOnly);
    FthItem("R/W",~preadWrite);
    FthItem("W/O",~pwriteOnly);

    FthItem("OPEN-FILE",~popenFile);
    FthItem("READ-FILE",~preadFile);
    FthItem("READ-LINE",~preadLine);
    FthItem("WRITE-FILE",~pwriteFile);
    FthItem("RESIZE-FILE",~presizeFile);


    FthItem("CLOSE-FILE",~pcloseFile);

    FthItem("OPEN-DIR",~pOpenDir);
    FthItem("DIRI2NAME",~pDirI2Name);
    FthItem("DIRI2TYPE",~pDirI2Type);
    FthItem("DIR2COUNT",~pDir2Count);
    FthItem("CLOSE-DIR",~pCloseDir);
    Constant("G_CLI_PATH",(sCell)&G_CLI_PATH);

    FthItem("ZCLI",~pZCli);

    FthItem("TIB",Ptib);
    sCell PATib = Constant("ATIB",(sCell)&atib);
    sCell Pntib = Constant("#TIB",(sCell)&ntib);

    FthItem("SOURCE",~pSource);
    FthItem("SOURCE!",~pSourceSet);
    FthItem("SOURCE-ID",PSourceId);

    FthItem("ALLOCATE",~pAllocate);
    FthItem("FREE",~pFree);

    Constant("YDP", (sCell)&YDP);
    Constant("YDP0", (sCell)&YDP0);
    FthItem("YDP_FL",~pYDPFL);

    Constant("&XPOS", (sCell)&tty_pos_x);
    Constant("&YPOS", (sCell)&tty_pos_y);
	FthItem("SETXY",~pSetXY);
	FthItem("GETXY",~pGetXY);

    Constant("&COLOR", (sCell)&tty_text_color);
    Constant("&BGCOLOR", (sCell)&tty_bg_color);
    FthItem("PAGE",~(Cell)clean_tty_screen );

    sCell PErrDO1 = Header("ERROR_DO1");	Co3(PSaveErr,~pPrintErr0,~pExit);
    sCell PErrDO = Header("ERROR_DO");  Co2(~pDoDefer,PErrDO1);

    sCell PAccept = Header("ACCEPT");  Co2(~pDoDefer,~pAccept);
    sCell PQuery = Header("QUERY");
    Co4(Ptib,~pLit_,256,PAccept);
    Co5(Pntib,~pStore,Pi2in,~pOff,~pExit);


//    FthItem("QUERY",~pQuery);

    sCell PBye = Header("BYE"); Co2(~pBye,PBye);

    sCell PLitC = Header("LIT,");  Co2(~pDoDefer,~pLitCo);
    sCell PPre = Header("<PRE>");  Co2(~pDoDefer,~pNoop);
    sCell PFileRefill = Header("FREFILL");  Co2(~pDoDefer,~pNoop);
    sCell PQStack = Header("?STACK");  Co2(~pDoDefer,~pNoop);

    sCell PRefill = Header("REFILL");
	Co(PSourceId);
    If();   Co2(PFileRefill,~pDup);  If(); Co(PPre); Then();
    Else(); Co2(PQuery,PTrue);
    Then(); Co(~pExit);

    FthItem("SNUMBER0",~pSNumber0);

    sCell PSNumber = Header("SNUMBER");  Co2(~pDoDefer,~pSNumber0 );

    sCell PQSLiteral0 = Header("?SLITERAL0");
	Co(PSNumber);
	If(); Lit(-13); Co(~pFThrow);
	Else(); Co(~pStateQ); If(); Co(PLitC); Then();
	Then();
    Co(~pExit);

    sCell PQSLiteral = Header("?SLITERAL");
    Co2(~pDoDefer,PQSLiteral0);

    sCell PInterpret1 = Header("INTERPRET1");
    Begin();
        Co6(Pi2in,~pLoad,PLastin,~pStore,PSaveErrQ,~pOn);
        Co2(~pParseName,~pDup);
    While();  Co2(~pSFind,~pQDup);
        If();
            Co2(~pStateQ,~pEqual);
            If();   Co(~pCompile );
            Else(); Co(~pExecute );
            Then();
        Else(); Co(PQSLiteral);
        Then(); Co(PQStack);
    Repeat();
    Co2(~pi2drop,~pExit);

    sCell PInterpret = Header("INTERPRET");
    Co2(~pDoDefer,PInterpret1 );

    sCell PQuit = Header("QUIT");
    Begin();	Co(PRefill); 
    While();	Co(PInterpret);	Tp(" ok\n>");
    Repeat();	Co(~pExit);

    sCell PWords = Header("WORDS");
    Co3(PContext,~pLoad,~pLoad);
    Begin(); Co(~pDup);
    While(); Co7(~pDup,~pCount,~pType,~pSpace,PCell,~pSub,~pLoad );
    Repeat(); Co2(~pDrop,~pExit );

    ip = here;  // SYS START

	Tp("Forth\n");

    Co(~pInitStringSet);
    Co5(~pIMode,~pLit_,PInterpret,PCatch,~pQDup );
    If(); Co5(PErrDO,PSP0,~pLoad,~pSPSet,~pCr ) ;
    Then();

    Begin();
	Co4(PATib,~pLit_,(sCell)&tib[1],~pStore);
        Co5(~pIMode,~pLit_,PQuit,PCatch,PErrDO);
        Co4(PSP0,~pLoad,~pSPSet,~pCr ) ;
    Again();
}


uint32_t forth_sys(uint32_t argc, char** argv) {

	forth_run=1;

     	tty_printf("Hello from Forth!!!\n");

	set_cursor_enabled(false);
//         pek();
	tib[0]=~pDoConst;
	i2in[0]=~pDoVar;
	SourceId[0]=~pDoConst;
	tib[0]=~pDoConst;
	Catch[0] = ~pto_catch;
	Catch[1] = ~pfrom_catch;
		memset(input_buffer, 0, 256);

	HereArea = kcalloc(sizeof(sCell), HERE_SIZE1);
	StackArea = kcalloc(sizeof(sCell), STACK_SIZE);
	RStackArea = kcalloc(sizeof(sCell), RSTACK_SIZE);

	here = HereArea ;
	Stack = &StackArea[STACK_SIZE-8] ;
	rStack = &RStackArea[RSTACK_SIZE-8] ;

        ForthWordlist[0] = 0;
        ForthWordlist[1] = 0;
        ForthWordlist[2] = 0;

	Context[0] = ForthWordlist;
	Context[1] = 0;
	Current[0] = ForthWordlist;
	ireg = ~(sCell)MakeImag;
  if(pNoop>0){
      _tty_printf("positiv\n"); // addresses area
	
      while (forth_run)
      {   do{
              ((proc) (~ireg) )();
              ireg = *ip++;
          }while ( ireg<0);
          do{
              *--rStack = (sCell) ip;  ip =  (sCell *) ireg;
              ireg = *ip++;
          }while ( ireg>0);
      }
  }
  else{
      _tty_printf("negative\n"); // addresses area
      while (forth_run)
        {   do{
                ((proc) (~ireg) )();
                ireg = *ip++;
            }while ( ireg>0);
            do{
                *--rStack = (sCell) ip;  ip =  (sCell *) ireg;
                ireg = *ip++;
            }while ( ireg<0);
        }
    }

	keyboardctl(KEYBOARD_ECHO, true);

}
