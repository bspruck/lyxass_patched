/* lyxass.c
 * begonnen : 15.4.96
 * (c) Bastian Schick
 *
 * history:
 *
 *  date    who     version what
 *  8.8.99  42BS    v0.44   changed error-format for emacs
 *  31.8.99 42BS    v0.45   changed include-format : 
 *                            "file" searches current path
 *                            <file> searches BLL_ROOT/file
 *                            'file' searches upper-case path
 *  2017 BS                   clean up opcode search (long cast/compare of strings)                        
 */
 
#define MAIN 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "my.h"
#include "label.h"
#include "global_vars.h"
#include "error.h"
#include "parser.h"


int CheckPseudo(char *s);
int CheckMnemonic(char *);
int CheckMacro(char *);

void InitTransASCII(void);
int mainloop(int );

int uni(long *);


extern REFERENCE *refFirst;
extern REFERENCE *refLast;

char info[] = "tjass/lyxass C-version V 0.51\n(c) 1993..1999 42Bastian Schick, modified by BS 2011,2017,2018\n";

/********************************************************************/
char *outfile = 0;

int warning = 0;
int verbose = 0;
int data = 0;
int dumpGlobals = 0;
int symbols = 0;      /* write EQU file with global symbols */
int error;            /* flag indicating an error to lower sub-routines */
int cntError = 0;     /* # of errors */
int cntWarning = 0;   /* # of warning */
int cntMacroExpand = 0;
int cntRef = 0;
FILE *my_stderr;      /* handle for error-output */
/***************************************************************
 * Malloc
 ***************************************************************
 */
long totalMemory = 0;

void * my_malloc(long size)
{
  void * help;
  if ( (help = malloc(size)) != NULL){
    totalMemory += size;
    return help;
  } else {
    Error(NOMEM_ERR,"");
  }
  return NULL;
}
/********************/
/* code generation  */
/********************/

void writeByte(char c)
{
  if (Global.genesis >0 ){
    if ( code.Size + 1 > MAX_CODE_SIZE ){
      Error(CODEMEM_ERR,"");
      Global.genesis = -1;
    } else {
      *code.Ptr++ = c;
      code.Size++;
    }
  }
  Global.pc++;
}

void writeSameBytes(char c,int count)
{
  if (Global.genesis >0 ){
    if ( code.Size + count > MAX_CODE_SIZE ){
      Error(CODEMEM_ERR,"");
      Global.genesis = -1;
    } else {
      int i = count+1;
      while ( --i ){
        *code.Ptr++ = c;
      }
      code.Size+= count;
    }
  }
  Global.pc+=count;
}

void writeBytes(char *src,int count)
{
  if (Global.genesis >0 ){
    if ( code.Size + count > MAX_CODE_SIZE ){
      Error(CODEMEM_ERR,"");
      Global.genesis = -1;
    } else {
      int i = count+1;
      while ( --i ){
        *code.Ptr++ = *src++;
      }
      code.Size+= count;
    }
  }
  Global.pc+=count;
}


void writeWordLittle(short w)
{
  if (Global.genesis >0 ){
    if ( code.Size + 2 > MAX_CODE_SIZE ){
      Error(CODEMEM_ERR,"");
      Global.genesis = -1;
    } else {
      *code.Ptr++ = (char) (w & 0xff);
      *code.Ptr++ = (char) (w >> 8);
      code.Size += 2;
    }
  }
  Global.pc+=2;
}

void writeWordBig(short w)
{
  //  printf("%04x ",w & 0xffff);
  if (Global.genesis >0 ){
    if ( code.Size + 2 > MAX_CODE_SIZE ){
      Error(CODEMEM_ERR,"");
      Global.genesis = -1;
    } else {
      *code.Ptr++ = (char) (w >> 8);
      *code.Ptr++ = (char) (w & 0xff);
      code.Size += 2;
    }
  }
  Global.pc+=2;
}

void writeLongLittle(long l)
{
  if (Global.genesis >0 ){
    if ( code.Size + 4 > MAX_CODE_SIZE ){
      Error(CODEMEM_ERR,"");
      Global.genesis = -1;
    } else {
      *code.Ptr++ = (char) (l & 0xff);
      *code.Ptr++ = (char) (l >> 8);
      *code.Ptr++ = (char) (l >> 16);
      *code.Ptr++ = (char) (l >> 24);
      code.Size += 4;
    }
  }
  Global.pc+=4;
}

void writeLongBig(long l)
{
  if (Global.genesis >0 ){
    if ( code.Size + 4 > MAX_CODE_SIZE ){
      Error(CODEMEM_ERR,"");
      Global.genesis = -1;

    } else {
      *code.Ptr++ = (char) (l >> 24);
      *code.Ptr++ = (char) (l >> 16);
      *code.Ptr++ = (char) (l >> 8);
      *code.Ptr++ = (char) (l & 0xff);
      code.Size += 4;
    }
  }
  Global.pc+=4;
}

void ConvertFilename(char *fn)
{
   while ( *fn ){
    if ( *fn == '\\' ) *fn = '/';
    ++fn;
  }
}


int LoadFile(char *dst, long offset, long max_len, char *fn, long *len)
{
  FILE *f;
  long readlen;

  ConvertFilename(fn);

  if ( (f = fopen(fn,"rb")) == NULL ) return Error(FILE_ERR,fn);

  if ( offset ){
    fseek(f,offset,SEEK_SET);
  }
 
  readlen = fread(dst,1,max_len,f);

  *len = readlen;

  fclose( f);

  return 0;
}

int writeFile(char *fn,char *src,long len)
{
  FILE *f;
  
  if ( (f=fopen(fn,"wb+")) != NULL ){
    fwrite(src,len,1,f);
    fclose(f);
    return TRUE;
  } else {
    return FALSE;
  }
}

void usage()
{
  printf("%s\nUsage:",info);
  printf("lyxass {\"-\"(o fn|r|v|w|s|d|D label[=value])} infile\n");
}
void help()
{
  printf("%s\n%s",info,
         "-------------- pseudo-opcodes\n"
	 "                         Pseudos are case-insensitive !\n"
         "lynx,gpu,dsp           - switch to lynx,gpu or dsp mode\n"
	 "org                    - set pc\n"
	 "run                    - set pc, first run sets start address and\n"
	 "                         enables code-generation\n"
	 "end                    - take a guess !\n"
         "dc.b / dc.w / dc.l     - define data, dc.b allows strings,\n"
         "                         w and l only of 2 resp. 4 bytes\n"
         "dc.a                   - like dc.b only bytes are translated before storing\n"
         "ds.b / ds.w / ds.l     - reserve space in chunks of 1,2 or 4 bytes\n"
         "inc@/dec@              - increase/decrease @-var\n"
	 "set@                   - set @-var\n"
	 "(these commands need a leading SPACE !)\n"
	 "set                    - redefine a label\n"
	 "equ                    - define a constant label\n"
	 "macro / endm           - define a macro\n"
	 "path  [\"path\"]         - add \"path\" to the internal path,\n"
	 "                          empty clears it\n"
	 "trans \"filename\",off   - load a dc.a translation table\n"
	 "ibytes \"filename\",off  - load a binary\n"
	 "include \"filename\"     - include source-file in current path\n"
	 "include 'filename'     - include source-file in upper case\n"
	 "include <filename>     - include source-file in $BLL_ROOT\n"
	 "if expression          - condional assembly\n"
	 "ifd label              - label defined ?\n"
	 "ifnd label             - label not defined ?\n"
	 "ifvar \\x               - macro var \\x given ?\n"
	 "else / endif           - more than one ELSE is allowed \n"
	 "switch expression      - conditional assembly\n"
	 "case expression        - \n"
	 "ends                   - \n"
	 "align expression       - align PC to expression\n"
	 "rept expression        - repeat following source\n"
	 "endr                   -\n"
	 "list expression        - set verbose (1 normal , 2 macros)\n"
	 "global label[,label    - define label as global\n"
	 "echo \"%%Dlabel %%Hlabel\" - print text and label-values \n"
         "                        %%D as decimal %%H as hex\n" 
	 "-------------- labels\n"
	 " General: Labels are case-sensitive, valid characters are :\n"
	 " 0..9, \"_\",\".\", A..Z, a..z\n"
	 " Labels may not start with with a dot or digit.\n"
	 " Local labels start with a dot (may be followed by a digit)\n"
         " Global labels end with a double double-colon.\n"
	 " A @ inside a label is replaced by a 4 hexdigits.\n"
	 " Macro-labels start with \".\\\".\n"
	 " Macro-names follow the rules for normal labels.\n"
	 "-------------- macros\n"
	 "There are no symbolic macro-parameters, but \\0..\\15 are replaced\n"
	 "by the correponding parameter. Empty parameters are allowed.\n"
	 "To protect commas you can surround a parameter with braces\n"
  );
}

void CommandLine(int *_argc, char **_argv)
{


  int argc = *_argc;
  char **argv = _argv;
  int c_arg = 0;

  if ( argc < 2 ){
    usage();
    exit(1);
  }
  
  while ( c_arg++,argv++,--argc ){
    char *s = *argv;
    if ( s[0] != '-' ) break;
    switch ( argv[0][1] ){
    case 'o':
      --argc;
      ++argv;
      c_arg++;
      outfile = my_malloc(strlen(*argv));
      strcpy(outfile,*argv);
      break;
    case 'v':
      if ( verbose < 3 ) ++verbose;
      break;
    case 'w':
      warning = 1;
      break;
    case 'd':
      data = 1;
      break;
    case 'D':
      {
	LABEL label;
	int solved;

	--argc;
	++argv;
	++c_arg;
	srcLinePtr = *argv;
	atom = ' ';
	if ( GetLabel( &label ) ) Error(CMD_ERR,"");
	label.value = 1;
	label.type = NORMAL;
	if ( TestAtom('=') ){
	  long l;
	  if ( uni(&l ) ) Error(CMD_ERR,"");
	  label.value = l;
	}
	DefineLabel(&label, &solved);
      }
      break;
    case 'h':
      help();
      exit(1);
    case 'r':
      dumpGlobals = 1;
      break;
    case 's':
      symbols = 1;
      break;
    default:
      Error(CMD_ERR,"");
    }
  }
  if ( argc != 1 ) Error(CMD_ERR,"");
  
  if ( !outfile ){
    char *p;
    outfile = my_malloc(strlen(*argv)+2);
    strcpy(outfile,*argv);
    p = outfile+strlen(outfile);
    while ( *p != '.' && p != outfile )
      --p;

    if ( p == outfile ){ 
      p += strlen(outfile);
      p[0] = '.';
    }
    p[1] = 'o';
    p[2] = 0;
  }

  *_argc = c_arg;
  
}

struct label_s _cycles = { 6,NORMAL,0,0,0,0,(LABEL *)0,(LABEL *)0,"CYCLES" };

int main(int argc, char **argv)
{
  // init
  
  my_stderr = stdout; //stderr;
  chgPathToUpper = 0;

  InitLabels();
  InitTransASCII();
  
  InitParser();

  memset( (char *)&Current, 0, sizeof(struct current_s) );
  memset( (char *)&Global, 0, sizeof(struct global_s) );
  Global.run = -1;
  sourceMode = LYNX;

  code.Mem = my_malloc(MAX_CODE_SIZE);
  code.Size = 12;
  code.Ptr = code.Mem+12;
  
  bll_root = getenv("BLL_ROOT");
 
  CommandLine( &argc, argv );

  {
    int solved;
    LABEL * l;
    l=DefineLabel(&_cycles, &solved);
    l->type |= VARIABLE;
    p_cycles = &l->value;
  }

  /*
  if ( argc != 2 ) {
    printf("Aufruf : dump filename \n");
    return FALSE;
  }
  */
  if (LoadSource(argv[argc])){

    mainloop(0);

    if ( refFirst ){
      REFERENCE *ptr1,*ptr = refFirst;

      Error(UNSOLVED_ERR,"\b");

      while ( ptr ){
	printf("<%32s> [%5d %s]\n",ptr->unknown->name,ptr->line+1,file_list[ptr->file].name);
	ptr1 = ptr;
	ptr = ptr->up;
	free(ptr1);
      }
    }
    if ( verbose ){
      printf("\nCode-size : %ld\n"
	     "Lines : %d\n"
	     "Macros expanded : %d\n"
	     "Pass 2 runs : %d\n"
	     "Start-address : $%04x\n"
	     "Memory usage : %ld\n",
	     code.Size,Global.Lines,
	     cntMacroExpand,cntRef,Global.run,totalMemory);
    }

    if ( cntError ){
      printf("Total Errors :%d\nNo file written !\n",cntError);
    } else {
      if ( Global.run != -1 ){
	if ( Global.mainMode == LYNX ){
	  code.Mem += 2;
	  code.Size -=2;
	  
	  code.Mem[0] = -0x80;
	  code.Mem[1] = 0x08;

	  code.Mem[2] = Global.run >> 8;
	  code.Mem[3] = Global.run & 0xff;
	  code.Mem[4] = code.Size >> 8;
	  code.Mem[5] = code.Size & 0xff;
	  
	  code.Mem[6] = 'B';
	  code.Mem[7] = 'S';
	  code.Mem[8] = '9';
	  code.Mem[9] = '3';
	  /*
	  code.Mem += 4;
	  code.Size -= 4;
	  code.Mem[0] = (char)0x81;
	  code.Mem[1] = 'P';
	  code.Mem[2] = Global.run >> 8;
	  code.Mem[3] = Global.run & 0xff;
	  code.Mem[4] = ((code.Size-6) >> 8) ^ 0xff;
	  code.Mem[5] = ((code.Size-6) & 0xff) ^ 0xff;
	  */
	} else {
	  code.Mem[0] = 'B';
	  code.Mem[1] = 'S';
	  code.Mem[2] = '9';
	  code.Mem[3] = '4';
	  code.Mem[4] = Global.run >> 24;
	  code.Mem[5] = (Global.run >> 16) & 255;
	  code.Mem[6] = (Global.run >>  8) & 255;
	  code.Mem[7] = Global.run & 255;
	  code.Mem[8] = (code.Size-12)>> 24;
	  code.Mem[9] = ((code.Size-12) >> 16) & 255;
	  code.Mem[10] = ((code.Size-12) >>  8) & 255;
	  code.Mem[11] = (code.Size-12) & 255;
	}
	if ( data ){
	  writeFile(outfile,code.Mem+10,code.Size-10);
	} else {
	  writeFile(outfile,code.Mem,code.Size);
	}
	if ( symbols ){
	  writeSymbols(outfile);
	}
      }else{
	printf("No RUN statement ! No outputfile generated.\n");
      }
    }
    if ( dumpGlobals ){
      DumpGlobals();
    }
  }

  //  hashStatistics();
  return cntError;
}

void CheckForUnsolvedLocals()
{
  if ( refFirst ){
    REFERENCE *ptr1,*ptr = refFirst;
    int flag = 1;
    
    while ( ptr ){
      ptr1 = ptr->up;

      if ( (ptr->unknown->type & (LOCAL|MACRO)) == LOCAL){
	if ( flag ){
	  Error(UNSOLVED_ERR,"local");
	  flag=0;
	}
	printf("<%32s> [%5d %s]\n",ptr->unknown->name,ptr->line+1,file_list[ptr->file].name);
	ptr->unknown = 0;
	
	if ( ptr->down ){
	  ptr->down->up = ptr->up;
	}
	if ( ptr->up ){
	  ptr->up->down = ptr->down;
	}
	if ( ptr == refFirst ) refFirst = ptr->up;
	if ( ptr == refLast ) refLast = ptr->down;
	free(ptr);
      }
      ptr = ptr1;
    }
  }
}

void doReference(void)
{
  if ( error || !Current.doRef || Current.pass2) return;

  if ( refFirst ){
    
    REFERENCE *ptr1,*ptr = refFirst;
    struct current_s save_current = Current;
    struct code_s save_code = code;
    struct global_s save_global = Global;
    int mode = sourceMode;

    //    printf("Pass 2:....");

    Current.doRef = 0;

    while ( ptr ){
      ptr1 = ptr->up;

      //      printf("->%p = %p %p\n",ptr->unknown,l,ptr1);

      if ( ptr->unknown == save_current.LabelPtr ){
	cntRef++;
	Current.Line = ptr->line;
	Current.File = ptr->file;
	Current.Macro.Line = ptr->macroline;
	Current.Macro.File = ptr->macrofile;
	Global.pc = ptr->pc;
	//Current.var = ptr->var;
	code.Ptr = ptr->codePtr;
	Current.SrcPtr = ptr->src;
	sourceMode = ptr->mode;

	//mesg(Current.SrcPtr);

	mainloop( 1 );

	ptr->unknown = 0;

	if ( ptr->down ){
	  ptr->down->up = ptr->up;
	}
	if ( ptr->up ){
	  ptr->up->down = ptr->down;
	}
	if ( ptr == refFirst ) refFirst = ptr->up;
	if ( ptr == refLast ) refLast = ptr->down;

	free(ptr);
      }
      ptr = ptr1;
    }
    
    //exit(1); 
    sourceMode = mode;
    code = save_code;
    Current = save_current;
    Global = save_global;
    //    printf("finished\n");
  }
  Current.doRef = 0;
}

int checkCode(char * s)
{
  int err;

  if ( (err = CheckPseudo(s)) >= 0) return err;

  if ( Current.Macro.Define ) return 0;

  if ( (err = CheckMnemonic(s)) >= 0) return err;

  return CheckMacro(s);
}

int mainloop(int pass2){

  //static int count = 0;

  //printf("Mainloop:%d\n",++count);


  Current.ifFlag = 1;
  Current.switchFlag = 1;

  Current.pass2 = pass2;

  for ( ; GetLine() ; killLine(),doReference() ) {

    Global.OldPC = Global.pc;
    Global.Lines++;

    code.OldPtr = code.Ptr;

    Current.Line++;
    Current.varModifier = 0;    
    Current.LabelPtr = 0;
    Current.doRef = 0;

    error = 0;

    if ( !Current.pass2 && Current.ifFlag && Current.switchFlag && verbose){
      if ( verbose > 2 || (!Current.Macro.Processing && verbose > 1) )
	printf("%4d:$%04lx: <%s>\n",Current.Line,Global.pc,srcLine);
    }
 
    if ( !atom ) continue;
   
    if ( atom == ' ' ){
      
      if ( GetCmd() )  continue; // empty line
      if ( checkCode(Current.Cmd) >= 0) continue;

      Error(UNKNOWN_ERR,Current.Cmd);
      continue;
    }
    
    if ( GetLabel( & Current.Label ) )  continue;
   
    if ( (Current.Label.type & UNSURE) ){

      if ( checkCode(Current.Label.name ) >= 0) continue;

      Current.Label.type = NORMAL;

    }
    if ( Current.Label.len &&
	 !Current.pass2 &&
	 !Current.Macro.Define &&
	 Current.ifFlag &&
	 Current.switchFlag){

      Current.Label.value = Global.pc;
      Current.LabelPtr = DefineLabel( &Current.Label , &Current.doRef);
    }
    //
    // we have a line-label, so store command elsewhere
    //

    if ( GetCmd() ) continue;

    if ( checkCode( Current.Cmd ) >= 0 ) continue;

    Error(UNKNOWN_ERR,Current.Cmd);
       
  }

  //  printf("Mainloop:%d\n",count--);

  return error;
}
