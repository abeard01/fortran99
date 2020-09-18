/*                                                                   
  version.c : generates new version files in:

      99 FORTRAN FORMAT statement
      a99 assembler TEXT statement
*/
#include <stdio.h>

#define OPENRD "r"		/* open for read access */
#define OPENWT "w"		/* open for write access */

FILE *chin, *fopen() ;			/* file pointers */

main(argc,argv)
int argc;
char *argv[];
{
  char line[41],date[41];
  float version;
  int i;

  printf("\nVersion Generation\n");

  fclose(chin);           /* close the input file */
  if (!(chin = fopen("version.current",OPENRD)))
     {
     printf("Fatal Error: Can't open version.current file, error %d\n",
        chin );
     exit(999);
     }

  if (!fgets(line,40,chin))
    {
     printf("\nError: Missing VERSION in version.current\n");
     exit();
    }
  fclose(chin);

  if ( !(chin = fopen("now",OPENRD)))
    {
    printf("Fatal Error: Can't open date file, error %d\n",chin);
    exit(999);
    }

  if ( !fgets(date,40,chin))
    {
    printf("Fatal Error: Can't read date file\n");
    exit(999);
    }

  fclose(chin);

  for ( i=0; i<41; i++ )
    {
    if ( date[i] == 0 )
      {
      i = i - 1;
      date[i] = 0;
      goto dexit;
      }
    }

dexit:
  sscanf ( line, "%f", &version );
  printf ( " Current Version is %7.3f\n", version );
  version = version + 0.001;		/* bump version number */
  printf ( " New Version     is %7.3f %s\n", version, date );

  if (!(chin = fopen("version.current",OPENWT)))
     {
     printf("Fatal Error: Can't open version.current file, error %d\n",
        chin );
     exit(999);
     }

  fprintf(chin, "%f %s\n", version, date);
  fclose ( chin );

  if (!(chin = fopen("version.f99",OPENWT)))
     {
     printf("Fatal Error: Can't open version.f99 file, error %d\n",
        chin );
     exit(999);
     }

  fprintf(chin, "  9101 format ('+ Version %7.3f (%s)')\n", version, date);
  fclose ( chin );

  if (!(chin = fopen("version.a99",OPENWT)))
     {
     printf("Fatal Error: Can't open version.a99 file, error %d\n",
        chin );
     exit(999);
     }

  fprintf(chin, "       TEXT 'Version %7.3f (%s)'\n", version, date);
  fclose ( chin );
  exit(0);
  }
