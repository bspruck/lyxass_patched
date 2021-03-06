/* global vars */

#ifndef GLOBAL_VARS
#define GLOBAL_VARS

#ifdef MAIN
#define EXTERN
#else
#define EXTERN extern
#endif

#define ATOM short int
#ifndef EOF
#  define EOF -1
#endif
#define EOL -2
#define SPACE  0x20


#define ISALPHA(c) ( ((c) >= 'A' && (c) <= 'Z') || ((c) >= 'a' && (c) <= 'z') )
#define ISNUM(c)  ( (c) >= '0' && (c) <= '9' )

#define targetLITTLE_ENDIAN 0x4321
#define targetBIG_ENDIAN    0x1234

#define MAX_INCLUDE     64
#define MAX_IF          16
#define MAX_CODE_SIZE   64*1024

#define EXPR_OK       0
#define EXPR_ERROR    1
#define EXPR_UNSOLVED 2


typedef struct {
  int line;
  char name[512];	        /* current filename */
}file_info;

#define REFERENCE struct reference_s

struct reference_s{
  REFERENCE *up,*down;
  int pc;
  int line;
  int file;
  int macroline;
  int macrofile;
  int macro;
  int var;
  int mode;
  LABEL * unknown;
  char *codePtr;
  char src[1];
};


/* global vars */

EXTERN file_info file_list[MAX_INCLUDE+1];
EXTERN char filename[512];

struct macro_s{
  int Processing;    // macro processed ; 0 == no macro
  int Define;
  int Line;          // line of macro-call
  int File;          // file of macro-call
  char * Name;
  int NPara;
  int invoked;      // number of invokations
};



// struct to save all current info



EXTERN struct global_s{
  long pc;            // program-counter
  long OldPC;         // dito, but before parsing
  int Lines;         // all assembled Lines
  int Files;         // all included files
  int run;           // != 0 => dest. address
  int genesis;       // != 0 => generate code
  int var;           // '@' value
  int mainMode;      // main source-Mode
  char Path[512];
} Global;

EXTERN struct current_s{
  int Line;          // source-line 
  int File;          // file-nr => act. FileName
  struct macro_s Macro;


  LABEL Label;       // line-label
  LABEL *LabelPtr;
  int CmdLen;
  char Cmd[80];      // command

  int pass2;         // != 0 => Pass 2
  int varModifier;   // != 0 => a pseudo-opcode set modifies variables
  int doRef;         // != 0 => label defined in this line
  int ParseOnly;

  int ifCnt;
  int ifFlag;       // != 0 => assemble line
  int ifSave[MAX_IF+1];

  int switchCnt;
  int switchFlag;
  int switchCaseMatch;
  long switchValue;

  char * SrcPtr;

  char * reptStart;
  int rept;
  long reptValue;
  int reptLine;
  
} Current;


EXTERN LABEL * unknownLabel;

EXTERN struct code_s{
  char *Mem;
  long Size;
  char *Ptr;
  char *OldPtr;
}code;

EXTERN int sourceMode;

#define LYNX      1
#define JAGUAR    2
#define ARM_THUMB 3

EXTERN char * bll_root;
EXTERN int chgPathToUpper;

EXTERN long *p_cycles;
#define CYCLES (*p_cycles)

extern char macrovars[16][16][80];

void * my_malloc(long size);

#endif
