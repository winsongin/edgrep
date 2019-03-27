//Winson Gin
//223C Section 01
//Monday/Wednesday 2-3:50PM

#include <signal.h>
#include <setjmp.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>
#include "grep.h"

const int BLKSIZE = 4096;  const int NBLK = 2047;  const int FNSIZE = 128;  const int LBSIZE = 4096;
const int ESIZE = 256; const int GBSIZE = 256;  const int NBRA = 5;  const int KSIZE = 9;  const int CBRA = 1;
const int CCHR = 2;  const int CDOT = 4;  const int CCL = 6;  const int NCCL = 8;  const int CDOL = 10;
const int CEOF = 11;  const int CKET = 12;  const int CBACK = 14;  const int CCIRC = 15;  const int STAR = 01;
const int READ = 0;  const int WRITE = 1;  /* const int EOF = -1; */

int  peekc, lastc, given, ninbuf, io, pflag;
int  vflag  = 1, oflag, listf, listn, col, tfile  = -1, tline, iblock  = -1, oblock  = -1, ichanged, nleft;
int  names[26], anymarks, nbra, subnewa, subolda, fchange, wrapp, bpagesize = 20;
unsigned nlall = 128;  unsigned int  *addr1, *addr2, *dot, *dol, *zero;

long  count;
char  Q[] = "", T[] = "TMP", savedfile[FNSIZE], file[FNSIZE], linebuf[LBSIZE], rhsbuf[LBSIZE/2], expbuf[ESIZE+4];
char  genbuf[LBSIZE], *nextip, *linebp, *globp, *mktemp(char *), tmpXXXXX[50] = "/tmp/eXXXXX";
char  *tfname, *loc1, *loc2, ibuff[BLKSIZE], obuff[BLKSIZE], WRERR[]  = "WRITE ERROR", *braslist[NBRA], *braelist[NBRA];
char  line[70];  char  *linp  = line;

//------------- Prototypes moved to edgrep.h --------------------------------------------------

jmp_buf  savej;
char grepbuf[GBSIZE];

typedef void  (*SIG_TYP)(int);
SIG_TYP  oldhup, oldquit;  //const int SIGHUP = 1;  /* hangup */   const int SIGQUIT = 3;  /* quit (ASCII FS) */

int main(int argc, char *argv[]) {  char *p1, *p2;  SIG_TYP oldintr;  oldquit = signal(SIGQUIT, SIG_IGN);
  oldhup = signal(SIGHUP, SIG_IGN);  oldintr = signal(SIGINT, SIG_IGN);
  if (signal(SIGTERM, SIG_IGN) == SIG_DFL) { signal(SIGTERM, quit); }  argv++;
  //while (argc > 1 && **argv=='-') { // original from McCarthy
  //------- Change argc == 3 -----------------
  while(argc == 3 && **argv == '-'){
    switch((*argv)[1]) {
    case '\0': vflag = 0;  break;
    case 'q': signal(SIGQUIT, SIG_DFL);  vflag = 1;  break;
    case 'o': oflag = 1;  break;
    }
    argv++;  argc--;
  }
  if (oflag) {  p1 = "/dev/stdout";  p2 = savedfile;  while ((*p2++ = *p1++) == 1) { } }
  // if(argc > 1) {
  if(argc != 3) {
    printf("Enter exactly 3 arguments.\n"); // if the command line doesn't receive exactly 3 arguments
    // p1 = *argv; p2 = savedfile;
  // if (argc > 1) {  p1 = *argv;  p2 = savedfile; // original from source code
    // while ((*p2++ = *p1++) == 1) {  if (p2 >= &savedfile[sizeof(savedfile)]) { p2--; }  }  globp = "r";
  }
  zero = (unsigned *)malloc(nlall * sizeof(unsigned));  tfname = mktemp(tmpXXXXX);  init();
  if (oldintr!=SIG_IGN) { signal(SIGINT, onintr); }  if (oldhup!=SIG_IGN) { signal(SIGHUP, onhup); }
  setjmp(savej);
  commands();
  quit(0);  return 0;
}
void commands(void) {  unsigned int *a1;  int c, temp;  char lastsep;
  for (;;) {
    if (pflag) { pflag = 0;  addr1 = addr2 = dot;  print(); }  c = '\n';
    for (addr1 = 0;;) {
      lastsep = c;  a1 = address();  c = getchr();
      if (c != ',' && c != ';') { break; }  if (lastsep==',') { error(Q); }
      if (a1==0) {  a1 = zero+1;  if (a1 > dol) { a1--; }  }  addr1 = a1;  if (c == ';') { dot = a1; }
    }
    if (lastsep != '\n' && a1 == 0) { a1 = dol; }
    if ((addr2 = a1)==0) { given = 0;  addr2 = dot;  } else { given = 1; }
    if (addr1==0) { addr1 = addr2; }
    switch(c) {
    case EOF:  return;
    case '\n':  if (a1 == 0) { a1 = dot + 1;  addr2 = a1;  addr1 = a1; }
                if (lastsep == ';') { addr1 = a1; }  print();  continue;
    // case 'e':  setnoaddr(); if (vflag && fchange) { fchange = 0;  error(Q); } filename(c);  init();
    //            addr2 = zero;  goto caseread;
    // modified case 'e'
    case 'e': grepreadfile(c);
              continue;
    case 'g':  global(1);  continue;
    case 'p':  case 'P':  newline();  print();  continue;
    case 'Q':  fchange = 0;  case 'q':  setnoaddr();  newline();  quit(0);
    // caseread:
    //     if ((io = open((const char*)file, 0)) < 0) { lastc = '\n';  error(file); }  setwide();  squeeze(0);
    //              ninbuf = 0;  c = zero != dol;
    //     append(getfile, addr2);  exfile();  fchange = c; continue;
    case 'z':  grepline();  continue;

    default:  // fallthrough
    caseGrepError:  greperror(c);  continue;
    }  error(Q);
  }
}
//------------- New function -----------------------------
// void search(char*, searchfor, char* inthis){

// }


//void add(int i) {  if (i && (given || dol>zero)) {  addr1--;  addr2--;  }  squeeze(0);  newline();  append(gettty, addr2); }
unsigned int* address(void) {  int sign;  unsigned int *a, *b;  int opcnt, nextopand;  int c;
  nextopand = -1;  sign = 1;  opcnt = 0;  a = dot;
  do {
    do c = getchr(); while (c==' ' || c=='\t');
    if ('0'<=c && c<='9') {  peekc = c;  if (!opcnt)  { a = zero; }  a += sign*getnum();
    } else switch (c) {
      case '$':  a = dol;  /* fall through */
      case '.':  if (opcnt) { error(Q); } break;
      case '\'':
        c = getchr();  if (opcnt || c<'a' || 'z'<c) { error(Q); }  a = zero;
        do { a++; } while (a<=dol && names[c-'a']!=(*a&~01));  break;
      case '?':  sign = -sign;  /* fall through */
      case '/':
        compile(c);  b = a;
        for (;;) {
          a += sign;
          if (a<=zero) { a = dol; }  if (a>dol) { a = zero; }  if (execute(a)) { break; }  if (a==b)  { error(Q); }
        }
        break;
      default:
        if (nextopand == opcnt) {  a += sign;  if (a < zero || dol < a)  { continue; } /* error(Q); */ }
        if (c!='+' && c!='-' && c!='^') {  peekc = c;  if (opcnt==0) { a = 0; }  return (a);  }
        sign = 1;  if (c!='+') { sign = -sign; }  nextopand = ++opcnt;  continue;
    }
    sign = 1;  opcnt++;
  } while (zero<=a && a<=dol);
  error(Q);  /*NOTREACHED*/  return 0;
}
int advance(char *lp, char *ep) {  char *curlp;  int i;
  for (;;) {
    switch (*ep++) {
      case CCHR:  if (*ep++ == *lp++) { continue; } return(0);
      case CDOT:  if (*lp++) { continue; }    return(0);
      case CDOL:  if (*lp==0) { continue; }  return(0);
      case CEOF:  loc2 = lp;  return(1);
      case CCL:   if (cclass(ep, *lp++, 1)) {  ep += *ep;  continue; }  return(0);
      case NCCL:  if (cclass(ep, *lp++, 0)) { ep += *ep;  continue; }  return(0);
      case CBRA:  braslist[*ep++] = lp;  continue;
      case CKET:  braelist[*ep++] = lp;  continue;
      case CBACK:
        if (braelist[i = *ep++] == 0) { error(Q); }
        if (backref(i, lp)) { lp += braelist[i] - braslist[i];  continue; }  return(0);
      case CBACK|STAR:
        if (braelist[i = *ep++] == 0) { error(Q); }  curlp = lp;
        while (backref(i, lp)) { lp += braelist[i] - braslist[i]; }
        while (lp >= curlp) {  if (advance(lp, ep)) { return(1); }  lp -= braelist[i] - braslist[i];  }  continue;
      case CDOT|STAR:  curlp = lp;  while (*lp++) { }                do {  lp--;  if (advance(lp, ep)) { return(1); } } while (lp > curlp);  return(0);
      case CCHR|STAR:  curlp = lp;  while (*lp++ == *ep) { }  ++ep;  do {  lp--;  if (advance(lp, ep)) { return(1); } } while (lp > curlp);  return(0);
      case CCL|STAR:
      case NCCL|STAR:  curlp = lp;  while (cclass(ep, *lp++, ep[-1] == (CCL|STAR))) { }  ep += *ep;  do {  lp--;  if (advance(lp, ep)) { return(1); } } while (lp > curlp);  return(0);
      default: error(Q);
    }
  }
}
int append(int (*f)(void), unsigned int *a) {  unsigned int *a1, *a2, *rdot;  int nline, tl;  nline = 0;  dot = a;
  while ((*f)() == 0) {
    if ((dol-zero)+1 >= nlall) {  unsigned *ozero = zero;  nlall += 1024;
      if ((zero = (unsigned *)realloc((char *)zero, nlall*sizeof(unsigned)))==NULL) {  error("MEM?");  onhup(0);  }
      dot += zero - ozero;  dol += zero - ozero;
    }
    tl = putline();  nline++;  a1 = ++dol;  a2 = a1+1;  rdot = ++dot;
    while (a1 > rdot) { *--a2 = *--a1; }  *rdot = tl;
  }
  return(nline);
}
int backref(int i, char *lp) {  char *bp;  bp = braslist[i];
  while (*bp++ == *lp++) { if (bp >= braelist[i])   { return(1); } }  return(0);
}
void blkio(int b, char *buf, long (*iofcn)(int, void*, unsigned long)) {
  lseek(tfile, (long)b*BLKSIZE, 0);  if ((*iofcn)(tfile, buf, BLKSIZE) != BLKSIZE) {  error(T);  }
}

int cclass(char *set, int c, int af) {  int n;  if (c == 0) { return(0); }  n = *set++;
  while (--n) { if (*set++ == c) { return(af); } }  return(!af);
}
void compile(int eof) {  int c, cclcnt;  char *ep = expbuf, *lastep, bracket[NBRA], *bracketp = bracket;
  if ((c = getchr()) == '\n') { peekc = c;  c = eof; }
  if (c == eof) {  if (*ep==0) { error(Q); }  return; }
  nbra = 0;  if (c=='^') { c = getchr();  *ep++ = CCIRC; }  peekc = c;  lastep = 0;
  for (;;) {
    if (ep >= &expbuf[ESIZE]) { expbuf[0] = 0;  nbra = 0;  error(Q); }  c = getchr();  if (c == '\n') { peekc = c;  c = eof; }
    if (c==eof) { if (bracketp != bracket) { expbuf[0] = 0;  nbra = 0;  error(Q); }  *ep++ = CEOF;  return;  }
    if (c!='*') { lastep = ep; }
    switch (c) {
      case '\\':
        if ((c = getchr())=='(') {
          if (nbra >= NBRA) { expbuf[0] = 0;  nbra = 0;  error(Q); }  *bracketp++ = nbra;  *ep++ = CBRA;  *ep++ = nbra++;  continue;
        }
        if (c == ')') {  if (bracketp <= bracket) { expbuf[0] = 0;  nbra = 0;  error(Q); }  *ep++ = CKET;  *ep++ = *--bracketp;  continue; }
        if (c>='1' && c<'1'+NBRA) { *ep++ = CBACK;  *ep++ = c-'1';  continue; }
        *ep++ = CCHR;  if (c=='\n') { expbuf[0] = 0;  nbra = 0;  error(Q); }  *ep++ = c;  continue;
      case '.': *ep++ = CDOT;  continue;
      case '\n':  expbuf[0] = 0;  nbra = 0;  error(Q);
      case '*':  if (lastep==0 || *lastep==CBRA || *lastep==CKET) { *ep++ = CCHR;  *ep++ = c; }  *lastep |= STAR; continue;
      case '$':  if ((peekc=getchr()) != eof && peekc!='\n') { *ep++ = CCHR;  *ep++ = c; }  *ep++ = CDOL;  continue;
      case '[':  *ep++ = CCL;  *ep++ = 0;  cclcnt = 1;  if ((c=getchr()) == '^') {  c = getchr();  ep[-2] = NCCL; }
        do {
          if (c=='\n') { expbuf[0] = 0;  nbra = 0;  error(Q); }  if (c=='-' && ep[-1]!=0) {
            if ((c=getchr())==']') { *ep++ = '-';  cclcnt++;  break; }
            while (ep[-1] < c) {  *ep = ep[-1] + 1;  ep++;  cclcnt++;  if (ep >= &expbuf[ESIZE]) { expbuf[0] = 0;  nbra = 0;  error(Q); } }
          }
          *ep++ = c;  cclcnt++;  if (ep >= &expbuf[ESIZE]) { expbuf[0] = 0;  nbra = 0;  error(Q); }
        } while ((c = getchr()) != ']');
        lastep[1] = cclcnt;  continue;
    }
  }
}

void error(char *s) {  int c;  wrapp = 0;  listf = 0;  listn = 0;  putchr_('?');  puts_(s);
  count = 0;  lseek(0, (long)0, 2);  pflag = 0;  if (globp) { lastc = '\n'; }  globp = 0;  peekc = lastc;
  if(lastc) { while ((c = getchr()) != '\n' && c != EOF) { } }
  if (io > 0) { close(io);  io = -1; }  longjmp(savej, 1);
}
int execute(unsigned int *addr) {  char *p1, *p2 = expbuf;  int c;
  for (c = 0; c < NBRA; c++) {  braslist[c] = 0;  braelist[c] = 0;  }
  if (addr == (unsigned *)0) {
    if (*p2 == CCIRC) { return(0); }  p1 = loc2; } else if (addr == zero) { return(0); }
  else { p1 = getline_blk(*addr); }
  if (*p2 == CCIRC) {  loc1 = p1;  return(advance(p1, p2+1)); }
  if (*p2 == CCHR) {    /* fast check for first character */  c = p2[1];
    do {  if (*p1 != c) { continue; }  if (advance(p1, p2)) {  loc1 = p1;  return(1); }
    } while (*p1++);
    return(0);
  }
  do {  /* regular algorithm */   if (advance(p1, p2)) {  loc1 = p1;  return(1);  }  } while (*p1++);  return(0);
}
void exfile(void) {  close(io);  io = -1;  if (vflag) {  putd();  putchr_('\n'); }  }
void filename(const char* f) {
  strcpy(file, f);
}
//---------------- NEW grepopenfile() function ----------------------------------------
void grepreadfile(int c){

  setnoaddr();
  if (vflag && fchange) {
    fchange = 0;  error(Q);
  }
  filename(c);  init();
  addr2 = zero;

  if ((io = open((const char*)file, 0)) < 0) { // caseread
    lastc = '\n';
    error(file);
  }
  setwide();
  squeeze(0);
  ninbuf = 0;
  c = zero != dol;
  append(getfile, addr2);
  exfile();
  fchange = c;
}



char * getblock(unsigned int atl, int iof) {  int off, bno = (atl/(BLKSIZE/2));  off = (atl<<1) & (BLKSIZE-1) & ~03;
  if (bno >= NBLK) {  lastc = '\n';  error(T);  }  nleft = BLKSIZE - off;
  if (bno==iblock) {  ichanged |= iof;  return(ibuff+off);  }  if (bno==oblock)  { return(obuff+off);  }
  if (iof==READ) {
    if (ichanged) { blkio(iblock, ibuff, (long (*)(int, void*, unsigned long))write); }
    ichanged = 0;  iblock = bno;  blkio(bno, ibuff, read);  return(ibuff+off);
  }
  if (oblock>=0) { blkio(oblock, obuff, (long (*)(int, void*, unsigned long))write); }
  oblock = bno;  return(obuff+off);
}
char inputbuf[GBSIZE];

int getchr(void) {  char c;
  if ((lastc=peekc)) {  peekc = 0;  return(lastc); }
  if (globp) {  if ((lastc = *globp++) != 0) { return(lastc); }  globp = 0;  return(EOF);  }
  if (read(0, &c, 1) <= 0) { return(lastc = EOF); }
  lastc = c&0177;  return(lastc);
}
// int getcopy(void) { if (addr1 > addr2) { return(EOF); }  getline_blk(*addr1++);  return(0); }
int getfile(void) {  int c;  char *lp = linebuf, *fp = nextip;
  do {
    if (--ninbuf < 0) {
      if ((ninbuf = (int)read(io, genbuf, LBSIZE)-1) < 0) {
        if (lp>linebuf) { puts_("'\\n' appended");  *genbuf = '\n';  } else { return(EOF); }
      }
      fp = genbuf;  while(fp < &genbuf[ninbuf]) {  if (*fp++ & 0200) { break; }  }  fp = genbuf;
    }
    c = *fp++;  if (c=='\0') { continue; }
    if (c&0200 || lp >= &linebuf[LBSIZE]) {  lastc = '\n';  error(Q);  }  *lp++ = c;  count++;
  } while (c != '\n');
  *--lp = 0;  nextip = fp;  return(0);
}
char* getline_blk(unsigned int tl) {  char *bp, *lp;  int nl;  lp = linebuf;  bp = getblock(tl, READ);
  nl = nleft;  tl &= ~((BLKSIZE/2)-1);
  while ((*lp++ = *bp++)) { if (--nl == 0) {  bp = getblock(tl+=(BLKSIZE/2), READ);  nl = nleft;  } }
  return(linebuf);
}
int getnum(void) { int r = 0, c;
  while ((c = getchr())>='0' && c <= '9') { r = r * 10 + c - '0'; }  peekc = c;  return (r);
}

void global(int k) {  char *gp;  int c;  unsigned int *a1;  char globuf[GBSIZE];
  //fgetc(filename());
  if (globp) { error(Q); }  setwide();  squeeze(dol > zero);
  if ((c = getchr()) == '\n') { error(Q); }  compile(c);  gp = globuf;
  while ((c = getchr()) != '\n') {
    if (c == EOF) { error(Q); }
    if (c == '\\') {  c = getchr();  if (c != '\n') { *gp++ = '\\'; }  }
    *gp++ = c;  if (gp >= &globuf[GBSIZE-2]) { error(Q); }
  }
  if (gp == globuf) { *gp++ = 'p'; }  *gp++ = '\n';  *gp++ = 0;
  for (a1 = zero; a1 <= dol; a1++) {  *a1 &= ~01;  if (a1>=addr1 && a1<=addr2 && execute(a1)==k) { *a1 |= 01; } }
// file grep doesn't delete
//  if (globuf[0] == 'd' && globuf[1] == '\n' && globuf[2] == '\0') {  gdelete();  return; }  // special: g/.../d avoid n^2
  for (a1 = zero; a1 <= dol; a1++) {
    if (*a1 & 01) {  *a1 &= ~01;  dot = a1;  globp = globuf;  commands();  a1 = zero; }
  }
}
void greperror(char c) {  getchr();  /* throw away '\n' */
  snprintf(grepbuf, sizeof(grepbuf), "\'%c\' is a non-grep command", c);  puts_(grepbuf);  }
void grepline(void) {
//  puts_("------------------------------------ ");
  getchr();  // throw away newline after command
  for (int i = 0; i < 50; ++i) { putchr_('-'); }   putchr_('\n');
}
void init(void) {  int *markp;  close(tfile);  tline = 2;
  for (markp = names; markp < &names[26]; )  {  *markp++ = 0;  }
  subnewa = 0;  anymarks = 0;  iblock = -1;  oblock = -1;  ichanged = 0;
  close(creat(tfname, 0600));  tfile = open(tfname, 2);  dot = dol = zero;  memset(inputbuf, 0, sizeof(inputbuf));
}

void newline(void) {  int c;  if ((c = getchr()) == '\n' || c == EOF) { return; }
  if (c == 'p' || c == 'l' || c == 'n') {  pflag++;
    if (c == 'l') { listf++;  } else if (c == 'n') { listn++; }
    if ((c = getchr()) == '\n') { return; }
  }  error(Q);
}
void nonzero(void) { squeeze(1); }
void onhup(int n) {
  signal(SIGINT, SIG_IGN);  signal(SIGHUP, SIG_IGN);
  if (dol > zero) {  addr1 = zero+1;  addr2 = dol;  io = creat("ed.hup", 0600);  if (io > 0) { putfile(); } }
  fchange = 0;  quit(0);
}
void onintr(int n) { signal(SIGINT, onintr);  putchr_('\n');  lastc = '\n';  error(Q);  }
//char* place(char *sp, char *l1, char *l2) {
//  while (l1 < l2) { *sp++ = *l1++;  if (sp >= &genbuf[LBSIZE]) { error(Q); } }  return(sp);
//}
void print(void) {  unsigned int *a1 = addr1;  nonzero();
  do {  if (listn) {  count = a1 - zero;  putd();  putchr_('\t');  }  puts_(getline_blk(*a1++));  } while (a1 <= addr2);
  dot = addr2;  listf = 0;  listn = 0;  pflag = 0;
}
void putchr_(int ac) {  char *lp = linp;  int c = ac;
  if (listf) {
    if (c == '\n') {
      if (linp != line && linp[-1]==' ') {  *lp++ = '\\';  *lp++ = 'n';  }
    } else {
      if (col > (72 - 4 - 2)) {  col = 8;  *lp++ = '\\';  *lp++ = '\n';  *lp++ = '\t';  }  col++;
      if (c=='\b' || c=='\t' || c=='\\') {
        *lp++ = '\\';
        if (c=='\b') { c = 'b'; }  else if (c=='\t') { c = 't'; }  col++;
      } else if (c<' ' || c=='\177') {
        *lp++ = '\\';  *lp++ =  (c>>6) +'0';  *lp++ = ((c >> 3) & 07) + '0';  c = (c & 07) + '0';  col += 3;
      }
    }
  }
  *lp++ = c;
  if (c == '\n' || lp >= &line[64]) {  linp = line;  write(oflag ? 2 : 1, line, lp - line);  return;  }  linp = lp;
}
void putd(void) {  int r = count % 10;  count /= 10;  if (count) { putd(); }  putchr_(r + '0');  }
void putfile(void) {  unsigned int *a1;  char *fp, *lp;  int n, nib = BLKSIZE;  fp = genbuf;  a1 = addr1;
  do {
    lp = getline_blk(*a1++);
    for (;;) {
      if (--nib < 0) {
        n = (int)(fp-genbuf);
        if (write(io, genbuf, n) != n) {  puts_(WRERR);  error(Q);  }  nib = BLKSIZE-1;  fp = genbuf;
      }
      count++;  if ((*fp++ = *lp++) == 0) {  fp[-1] = '\n';  break; }
    }
  } while (a1 <= addr2);
  n = (int)(fp-genbuf);  if (write(io, genbuf, n) != n) {  puts_(WRERR);  error(Q); }
}
int putline(void) {  char *bp, *lp;  int nl;  unsigned int tl;  fchange = 1;  lp = linebuf;
  tl = tline;  bp = getblock(tl, WRITE);  nl = nleft;  tl &= ~((BLKSIZE/2)-1);
  while ((*bp = *lp++)) {
    if (*bp++ == '\n') {  *--bp = 0;  linebp = lp;  break;  }
    if (--nl == 0) {  bp = getblock(tl += (BLKSIZE/2), WRITE);  nl = nleft;  }
  }
  nl = tline;  tline += (((lp - linebuf) + 03) >> 1) & 077776;  return(nl);
}
void puts_(char *sp) {  col = 0;  while (*sp) { putchr_(*sp++); }  putchr_('\n');  }
void quit(int n) { if (vflag && fchange && dol!=zero) {  fchange = 0;  error(Q);  }  unlink(tfname); exit(0); }
void reverse(unsigned int *a1, unsigned int *a2) {  int t;
  for (;;) {  t = *--a2;  if (a2 <= a1) { return; }  *a2 = *a1;  *a1++ = t;  }
}
void setnoaddr(void) { if (given) { error(Q); } }
void setwide(void) { if (!given) { addr1 = zero + (dol>zero);  addr2 = dol; } }
void squeeze(int i) { if (addr1 < zero+i || addr2 > dol || addr1 > addr2) { error(Q); } }
