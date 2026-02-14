/******************************************************************
 *                     LASTON.C Version 1.1                       *
 *        Written by Voltec (Voltec@cyberdude.com) 1999           *
 *      For Empire Of The Night (eotn.oaktree.co.uk 4100)         *
 *       All the usual DIKU/MERC/ROM/EOTN Licences apply          *
 *  If you use this code, drop me a line so I can see if people   *
 *                 Actually find this usefull.                    *
 *                         REMEMBER                               *
 *      Share and Enjoy!                      -Voltec 99          *
 ******************************************************************/

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <dirent.h>

#include "merc.h"
#include "recycle.h"

#define TOO_OLD 30

/* Voltecs attempt at a command to see when a given
 * player last logged in.  Can be useful for imms
 * wanting to know easily how long it has been scince
 * a builder was seen in the game...
 *
 * Not to be given to mortals, as Version 1.1
 * exposes hidden / invisible players...
 *
 * Oh, and it prints the date in D/M/YYYY format, 
 * if you don't like that go change it yerself...
 *
 * Syntax to be...
 *
 * laston <char>
 *
 * - voltec@cyberdude.com - 1999
 */

int get_pfile_stat(char * directory, char * filename, struct stat *file_struct);


void do_laston(CHAR_DATA *ch, char *argument)
{
   CHAR_DATA    *victim;
   char         buf[MAX_STRING_LENGTH];
   char         arg1[MAX_STRING_LENGTH];
   struct stat  file_struct;   /* File structure as used by the stat(2) command */
   int result;

   argument = one_argument(argument, arg1);

   if (IS_NPC(ch))
      return;

   if ( arg1[0] =='\0' )
   {
      send_to_char("Syntax : laston <char>\n\r", ch);
      return;
   }

   victim = get_char_world(ch, arg1);

   if (victim != NULL)   /* Player exists */
   {
      if (IS_NPC(victim))   /* But is an NPC */
      {
         send_to_char("Not on NPC's\n\r", ch);
         return;
      }
	else
      if ( get_trust( victim ) > get_trust( ch ))
      {
	  send_to_char( "You failed. You can not check the status on higher level Immortals!!!\n\r", ch );
	  return;
      }
      else
      {
         sprintf(buf, "%s is currently playing.\n\r", victim->name);
         send_to_char(buf, ch);
         return;
      }
   }

      //C068

      result=get_pfile_stat(PLAYER_DIR, arg1, &file_struct);

      if (result==0)
      {
	  if (((!str_cmp(arg1,"hiddukel"))
	  ||  (!str_cmp(arg1,"paladine")))
	  && (ch->level < 110))
        {
	    send_to_char( "You failed. You can not check the status on higher level Immortals!!!\n\r", ch );
	    return;
        }
	  else
	  {
          sprintf(buf, "%s last left the game %s", capitalize(arg1),ctime(&file_struct.st_mtime));
          send_to_char(buf, ch);
	  }
      }
      else
      {
          if(result==1)
          {
              sprintf(buf, "%s has never logged in.\n\r", arg1);
              send_to_char(buf, ch);
          }
          else
          {
              send_to_char("laston : stat failed, please report to Hiddukel!", ch);
          }
      }
}

int get_pfile_stat(char * directory, char * filename, struct stat *file_struct)
{
    FILE * tmpfp;
    char path[MIL],buf[MSL];

    sprintf(path, "%s%s", directory, capitalize( filename ));

    fclose(fpReserve);
 
    if ((tmpfp = fopen(path, "r")) == NULL) 
    {
       /* Cant open the p-file, so player doesn't exist */
       fpReserve = fopen( NULL_FILE, "r" );
       return 1;
    }
    else   
    {
       /* if we get this far, they have played, but aren't at the moment... */
       
       errno = 0;   /* Zero the error trapping number... */
 
       if( stat(path, &(*file_struct)) == -1 )
       {
          sprintf(buf, "do_laston : STAT FAILED : %s", strerror(errno) );
          bug(buf, 0);
          return -1;
       }
 
       /* so by now, file_struct should have info about the p-file */
       return 0;
    }
      fclose(tmpfp);
      fpReserve = fopen( NULL_FILE, "r" );
}


void do_laston_immortals (CHAR_DATA *ch, char *argument)
{
    DIR * directory;
    struct dirent* contents;
    char buf[MSL];
    BUFFER *output;
    struct stat file_struct;
    time_t now;
    int age;

    directory = opendir(GOD_DIR);

    contents = readdir(directory);

    output=new_buf();
    while (contents)
    {
        if (contents->d_name[0]!='.')
        {
            get_pfile_stat(GOD_DIR,contents->d_name,&file_struct);

            time(&now); //take the time at the mo
            //calculate the age of the file in days
            age = (difftime(now,file_struct.st_mtime))/60/60/24;
            /*nowtime=gmtime(&current_time);
            godfiletime=gmtime(&file_struct.st_mtime);*/
            sprintf(buf, "%-20s%s %-20s%s\r",
            contents->d_name,( age < TOO_OLD ) ? "" : C_B_RED,
            ctime(&file_struct.st_mtime),
            CLEAR);
            add_buf(output,buf);
        }
        contents = readdir(directory);
    }
    closedir(directory);

    send_to_char(buf_string(output),ch);
    free_buf(output);
}
