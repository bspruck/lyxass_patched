/*

  opcode.c

  search opcode and execute function

*/


#include <stdio.h>
#include <string.h>

#include "my.h"
#include "error.h"
#include "label.h"
#include "global_vars.h"

#include "opcode.h"


/* SearchOpcode

   looks through opcode-list and jumps into opcode-function

   returns either error-code or -1 for no opcode found
*/

int SearchOpcode(const struct opcode_s *,char *);
int SearchOpcode(const struct opcode_s *list,char *s)
{
  const struct opcode_s *curr;
  
  if ( strlen(s) > 7 ){
    return -1;
  }

  // get opcode and transfer into upper-case

  curr = list;
  
  while ( curr->name[0] ){

    //mesg(curr->name);

    if ( strcasecmp(curr->name,s)==0 ){
      //      printf("Found (%s)\n",curr->name);
      return curr->func( curr->misc );
    }
    curr++;
  }
  return -1;
} 

int SearchOpcode2(const struct opcode_s *,char *, int (**)(int ),int *);
int SearchOpcode2(const struct opcode_s *list,char *s,int (**fun)(int ), int *para)
{
  const struct opcode_s *curr;
  
  if ( strlen(s) > 7 ){
    return -1;
  }

  // get opcode and transfer into upper-case

  curr = list;
  
  while ( curr->name[0] ){
    //mesg(curr->name);

     if ( strcasecmp(s,curr->name)==0 ){
      //      printf("Found (%s)\n",curr->name);
      *fun = curr->func;
      *para = curr->misc;
      return 1;
    }
    curr++;
  }
  return -1;
} 
