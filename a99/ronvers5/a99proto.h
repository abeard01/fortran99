/*  a99proto.h         Function prototypes for a99  */

#ifndef NARGS

extern int  addtxt(char*, char);
extern void aquit(void);
extern void chkpage(void);
extern void ckreg(int*);
extern int  clower(char);
extern int  cupper(char);
extern int  dstow(char*, long*);     /* 1-10-89 last arg to long from int */
extern void emit(int, int, char*);
extern int  emitbyte(int, int);
extern void emitr(int, int, char*);
extern int  emlst(int, int, char*, char);
extern int  emmaccall(int, char*);
extern void emtdef (char*, int, int);
extern void emtlv(int);
extern void emtobj(int, int, int);   /* 1-10-89 last arg to int  from char */
extern void emtref(void);
extern void emtsr(char*);
extern void emtv(int, int, int);
extern void emval(int);
extern void emvalr(int);
extern void endp1(void);
extern void enter(char*, int, char);
extern int  eqlstr(char*, char*);
extern int  eqlstr(char*, char*);
extern int  error(int);
extern int  eval(char*, int*);
extern int  expandmacro(char*, char);
extern void flushobj(void);
extern void g015(int*, char*);
extern int  getmline(char*);
extern int  gopcod(char*, char*, int*);
extern int  gopnd(int*, char*, int*, char*, int*);
extern void greg(int*, char*);
extern int  gtok(char*, char*);
extern int  hstow(char*, long*);     /* 1-10-89 last arg to long from int  */
extern void init(void);
extern int  lookup(char*, long*);    /* 1-10-89 changed to long from int  */
extern void lower(char*);
extern int  newlin(void);
extern int  outcom(char*);
extern int  outmac(char*);
extern int  p2err(char*);
extern int  p2errt(char*, char*);
extern int  pass1(void);
extern int  pass2(void);
extern void psop(char*, char);
extern int  savemacro(void);
extern void set2(int*, int);
extern int  showerrors(void);
extern int  strcpy(unsigned char*, unsigned char*);
extern int  strlen(unsigned char*);
extern void upper(char*);
extern void wtohs(int, char*);

#else

extern int  addtxt();
extern void aquit();
extern void chkpage();
extern void ckreg();
extern int  clower();
extern int  cupper();
extern int  dstow();
extern void emit();
extern int  emitbyte();
extern void emitr();
extern int  emlst();
extern int  emmaccall();
extern void emtdef ();
extern void emtlv();
extern void emtobj();
extern void emtref();
extern void emtsr();
extern void emtv();
extern void emval();
extern void emvalr();
extern void endp1();
extern void enter();
extern int  eqlstr();
extern int  eqlstr();
extern int  error();
extern int  eval();
extern int  expandmacro();
extern void flushobj();
extern void g015();
extern int  getmline();
extern int  gopcod();
extern int  gopnd();
extern void greg();
extern int  gtok();
extern int  hstow();
extern void init();
extern int  lookup();
extern void lower();
extern int  newlin();
extern int  outcom();
extern int  outmac();
extern int  p2err();
extern int  p2errt();
extern int  pass1();
extern int  pass2();
extern void psop();
extern int  savemacro();
extern void set2();
extern int  showerrors();
extern int  strcpy();
extern int  strlen();
extern void upper();
extern void wtohs();

#endif
