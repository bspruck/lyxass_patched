/*

  error.c

*/

#include <stdio.h>
#include <stdlib.h>

#include "my.h"
#include "label.h"
#include "global_vars.h"
#include "parser.h"
#include "error.h"

extern int cntError;
extern int cntWarning;
extern int error;
extern FILE *my_stderr;      /* handle for error-output */
extern int verbose;
extern int warning;

int Error(int err_num,char s[])
{
  ++error;

  ++cntError;

  if ( err_num < 0 ){
    fprintf(stderr,"Fatal Error %d!\n",-err_num);
  }

   if (Current.File >= 0){
    if ( Current.Macro.Line ){
      fprintf(my_stderr,"%s;%d:ERROR:Macro-Line %5d:<%s><%s>\n",
	      file_list[Current.File].name,Current.Line,
	      Current.Macro.Line,file_list[Current.Macro.File].name,s);
    }else{
      fprintf(my_stderr,"%s:%5d:ERROR:\n",file_list[Current.File].name,Current.Line);
    }
  }
  switch (err_num) {
  case  INCLUDE_ERR:
    fprintf(my_stderr,"To many includes !\n");
    break;
  case FILE_ERR:
    fprintf(my_stderr,"Can't open file <%s> !\n",s);
    break;
  case LABEL_ERR:
    fprintf(my_stderr,"Wrong character in label!\n");
    break;
  case MLABEL_ERR:
    fprintf(my_stderr,"Macro-label outside macro !\n");
    break;
  case NOMEM_ERR:
    fprintf(my_stderr,"No more memory available !\n");
    break;
  case REDEF_ERR:
    fprintf(my_stderr,"Label redefined ! First use here: %5d : <%s>\n",
	    ((LABEL *)s)->line,file_list[ ((LABEL *)s)->file ].name );
    break;
  case LREDEF_ERR:
    fprintf(my_stderr,"Local label redefined ! First use here: %5d : <%s>\n",
	    ((LABEL *)s)->line,file_list[ ((LABEL *)s)->file ].name );
    break;
  case LMREDEF_ERR:
    fprintf(my_stderr,"Local macro label redefined ! First use here: %5d : <%s>\n",
	    ((LABEL *)s)->line,file_list[ ((LABEL *)s)->file ].name );
    break;
  case MREDEF_ERR:
    fprintf(my_stderr,"Macro-name redefined ! First use here: %5d : <%s>\n",
	    ((LABEL *)s)->line,file_list[ ((LABEL *)s)->file ].name );
    break;
  case TOOMANY_ERR:
    fprintf(my_stderr,"Global symbol memory exhausted !\n");
    break;
  case LTOOMANY_ERR:
    fprintf(my_stderr,"Local symbol memory exhausted !\n");
    break;
  case LMTOOMANY_ERR:
    fprintf(my_stderr,"Local-macro symbol memory exhausted !\n");
    break;
  case MTOOMANY_ERR:
    fprintf(my_stderr,"Macro-name memory exhausted !\n");
    break;
  case UNKNOWN_ERR:
    fprintf(my_stderr,"Unknown opcode : (%s) !\n",s);
    break;    
  case TOOBIG_ERR:
    fprintf(my_stderr,"Constant to big !\n");
    break;
  case EXPR_ERR:
    fprintf(my_stderr,"Error in expression !\n");
    break;
  case SYNTAX_ERR:
    fprintf(my_stderr,"Syntax Error !\n");
    break;
  case NEEDCONST_ERR:
    fprintf(my_stderr,"%s needs constant expression !\n",s);
    break;
  case MLABEL2_ERR:
    fprintf(my_stderr,"Only .\\label allowed inside a macro !\n");
    break;
  case CODEMEM_ERR:
    fprintf(my_stderr,"Code-memory exhausted. Code > %dK !\n",MAX_CODE_SIZE/1024);
    break;
  case DISTANCE_ERR:
    fprintf(my_stderr,"Relative distance to large !\n");
    break;
  case NOREFMEM_ERR:
    fprintf(my_stderr,"No more reference-memory !\n");
    break;
  case GARBAGE_ERR:
    fprintf(my_stderr,"Garbage at the end of line !\n");
    break;
  case MACRONESTED_ERR:
    fprintf(my_stderr,"Macro-definition nested to deep (max. 2) !\n");
    break;
  case BYTE_ERR:
    fprintf(my_stderr,"Byte value out of range !\n");
    break;
  case WORD_ERR:
    fprintf(my_stderr,"Word value out of range !\n");
    break;
  case TYPE_ERR:
    fprintf(my_stderr,"Label-type mismatch !\n");
    break;
  case UNSOLVED_ERR:
    fprintf(my_stderr,"Unsolved %s labels !\n",s);
    break;
  case TOOMANYIF_ERR:
    fprintf(my_stderr,"To many nested IF !\n");
    break;
  case ENDIF1_ERR:
    fprintf(my_stderr,"ENDIF without IF !\n");
    break;
  case FAIL_ERR:
    break;    
  case SWITCH_ERR:
    fprintf(my_stderr,"Nested SWITCH !\n");
    break;
  case CASE_ERR:
    fprintf(my_stderr,"CASE without SWITCH !\n");
    break;
  case DEFAULT_ERR:
    fprintf(my_stderr,"DEFAULT/ELSES without SWITCH !\n");
    break;
  case ENDS_ERR:
    fprintf(my_stderr,"ENDS without SWITCH !\n");
    break;
  case REPT1_ERR:
    fprintf(my_stderr,"Nested REPT !\n");
    break;
  case REPT2_ERR:
    fprintf(my_stderr,"ENDR without REPT !\n");
    break;
  case ISYMS_ERR:
    fprintf(my_stderr,"ISYMS not supported !\n");
    break;
  case CMD_ERR:
    fprintf(my_stderr,"Error in command-line !\n");
    break;
  case WRITE_ERR:
    fprintf(my_stderr,"Could not write :%s\n",s);
    break;
  case LOAD_ERR:
    fprintf(my_stderr,"LoadBuffer not big enough for :%s\n",s);
    break;
  case IMM_ERR:
    fprintf(my_stderr,"Quick immediate value out of range (%s)!\n",s);
    break;
  case REG_ERR:
    fprintf(my_stderr,"Missing Register !\n");
    break;
  case REG1_ERR:
    fprintf(my_stderr,"Redefining register-symbol !\n");
    break;
  }


 
  if ( !verbose) verbose=1;
  killLine();
  
  if ( err_num < 0 ){
    fprintf(stderr,"leaving ...\n");
    exit(1);
  }
  return err_num;
}

void Warning(char *s)
{
  cntWarning++;
  if (warning && Current.File >= 0)
    printf("WARNING:%5d(%5d):<%s>:%s\n",Current.Line,Current.Macro.Line,file_list[Current.File].name,s);
  
}
