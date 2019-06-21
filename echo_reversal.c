#include "types.h"
#include "stat.h"
#include "user.h"
 
int
main(int argc, char *argv[])
{
 int i;
 
 for(i = 1; i < argc; i++)
 {
    int j;
    for(j = strlen(argv[i])-1; j >= 0;j--)
       printf(1,"%c",argv[i][j]);
    printf(1,"%s", i+1 < argc ? " " : "\n");
 }
 exit();
}
