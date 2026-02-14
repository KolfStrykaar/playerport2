/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
*	ROM 2.4 is copyright 1993-1998 Russ Taylor			   *
*	ROM has been brought to you by the ROM consortium		   *
*	    Russ Taylor (rtaylor@hypercube.org)				   *
*	    Gabrielle Taylor (gtaylor@hypercube.org)			   *
*	    Brian Moore (zump@rom.org)					   *
*	By using this code, you have agreed to follow the terms of the	   *
*	ROM license, in the file Rom24/doc/rom.license			   *
***************************************************************************/

/***************************************************************************
*       Player File Management Tools by Ewan Gatherer                      *
*       ewangatherer@bigfoot.com                                           *
*       Created for the mud Storm Over Krynn                               *
*       krynnstorm.mudservices.com:4004                                    *
***************************************************************************/

#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include "merc.h"
#include "interp.h"
#include "recycle.h"
#include "tables.h"
#include "lookup.h"
#include <dirent.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>

#define DELETE_DIR "../todelete"
#define MOVE_CMD "mv"
#define TOO_OLD 120
#define TOO_BIG 20000

#define KEY2( literal, field, value )					\
				if ( !str_cmp( word, literal ) )	\
				{					\
				    field  = value;			\
				    fMatch = TRUE;			\
				}


bool get_specific_flag_from_char_file(char *player, char *key_field, long *numb);
extern int get_pfile_stat(char * directory, char * filename, struct stat *file_struct);
bool remove_pfile(char *pfile);


//modified version of do_deny orignal from rom24b6
void do_deny( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;
    char strsave[MAX_INPUT_LENGTH];
    int id;
    char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Deny whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if (victim->level < 2)
    {
       SET_BIT(victim->act, PLR_DENY);
       send_to_char( "\n\r", victim );
       send_to_char( "The actions you have taken have been deemed grossly unacceptable\n\r", victim );
       send_to_char( "to the administration of this mud. You have been DENIED access from\n\r", victim );
       send_to_char( "this MUD with the current character your using. Feel free to start a\n\r", victim );
       send_to_char( "new character though. If you continue to perform the same actions\n\r", victim );
       send_to_char( "or any actions like them, you will not only lose your new character\n\r", victim );
       send_to_char( "but you will also lose access to this MUD, your site will be banned.\n\r", victim );
       send_to_char( "\n\r", victim );
       sprintf(buf,"$N denies access to %s",victim->name);
       wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
       send_to_char( "OK.\n\r", ch );
       save_char_obj(victim);
       stop_fighting(victim,TRUE);
       id = victim->id;
       d = victim->desc;
       extract_char( victim, TRUE );
       if ( d != NULL )
    	 close_socket( d );
	 unlink(strsave);
       return;
    }

    if (!IS_SET(PLR_DENY, victim->act))
    {
	 /* Remove name from the Clan Roster, if we were in a clan. */
	 if(victim->org_id != ORG_NONE)
	 {
	 	rem_name_clan(victim->name, ch->org_id, ch->level);
	 }

       SET_BIT(victim->act, PLR_DENY);
       send_to_char( "\n\r", victim );
       send_to_char( "The actions you have taken have been deemed grossly unacceptable\n\r", victim );
       send_to_char( "to the administration of this mud. You have been DENIED access from\n\r", victim );
       send_to_char( "this MUD with the current character your using. Feel free to start a\n\r", victim );
       send_to_char( "new character though. If you continue to perform the same actions\n\r", victim );
       send_to_char( "or any actions like them, you will not only lose your new character\n\r", victim );
       send_to_char( "but you will also lose access to this MUD, your site will be banned.\n\r", victim );
       send_to_char( "\n\r", victim );
        sprintf(buf,"$N denies access to %s",victim->name);
        wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
        send_to_char( "OK.\n\r", ch );
        save_char_obj(victim);
        stop_fighting(victim,TRUE);
	  victim->quit_timer = 1;
        do_function(victim, &do_quit, "" );
    }
    else
    {

	  /* Add name to the Roster, if we are in a clan. */
	  if(victim->org_id != ORG_NONE)
	  {
		add_name_clan(victim->name, victim->org_id, victim->level);
	  }

        REMOVE_BIT(victim->act, PLR_DENY);
        sprintf(buf, "You have removed the denied access flag from %s!\n\r", victim->name);
        send_to_char(buf,ch);
        sprintf(buf,"$N removes deny access from %s",victim->name);
        wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
        send_to_char( "OK.\n\r", ch );
        save_char_obj(victim);
        do_function(victim, &do_quit, "" );
    }

    return;
}

void do_clean( CHAR_DATA *ch, char *argument )
{
    //Parses all pfiles removing denied or old pfiles
    char arg[MIL];
    char buf[MSL];
    long pfile_act;
    DIR * directory;
    struct dirent* contents;
    BUFFER *output;
    bool old=FALSE, deny=FALSE, delete_flag, size=FALSE;
    int result;
    struct stat file_struct;
    int age, count=0, delcount=0;
    time_t now;

    one_argument( argument, arg );
    if ( !str_cmp(arg,"old"))
    {
        old=TRUE;
    }

    if ( !str_cmp(arg,"deny"))
    {
        deny=TRUE;
    }

    if ( !str_cmp(arg,"size"))
    {
        size=TRUE;
    }

    if ( !str_cmp(arg,"all"))
    {
        old=TRUE;
        deny=TRUE;
        size=TRUE;
    }

    if (old==FALSE && deny==FALSE && size==FALSE)
    {
        send_to_char("Usage - clean old\n\r\tclean deny\n\r\tclean size\n\r\tclean all",ch);
    }

    directory = opendir(PLAYER_DIR); //open up the player directory
    contents = readdir(directory); //read the first file from the directory

    output=new_buf();
    while (contents)
    {
        delete_flag=FALSE;
        if (contents->d_name[0]!='.') //dont touch directory(.) or parent (..)
        {

            if (old)
            {
                //grab the stat of the file
                get_pfile_stat(PLAYER_DIR,contents->d_name,&file_struct);
                time(&now); //take the time at the mo
                //calculate the age of the file in days
                age = (difftime(now,file_struct.st_mtime))/60/60/24;
                if(age>TOO_OLD)
                {
                    delete_flag=TRUE;
                }
            }

            if (deny)
            {
                if(get_specific_flag_from_char_file(contents->d_name,"Act",&pfile_act)==FALSE)
                {
                    sprintf(buf,"Failed to read Act from file %s\n\r",contents->d_name);
                    add_buf(output,buf);
                }
                else
                {
                    if (IS_SET(PLR_DENY,pfile_act))
                    { //if the player is denied delete it
                        delete_flag=TRUE;
                    }
                }
            }

            if (size)
            {
                get_pfile_stat(PLAYER_DIR,contents->d_name,&file_struct);
                if ((int)file_struct.st_size>TOO_BIG)
                {
                    sprintf(buf,"%-20s %dbytes\n\r",contents->d_name,(int)file_struct.st_size);
                    add_buf(output,buf);
                }
            }

            if(delete_flag)
            {
                delcount++;
                result=remove_pfile(contents->d_name);
                if(!result)
                {
                    sprintf(buf, "Deleted - %s\n\r",contents->d_name);
                }
                else
                {
                    sprintf(buf, "{BDeletion Failed{x - %s\n\r",contents->d_name);
                }
                logpf(buf);
                add_buf(output,buf);
            }
        count++;
        }
        contents = readdir(directory); //read the next file in the directory
    }
    closedir(directory);

    if (size)
    {
        sprintf(buf,"\n\rIf you wish to flag a pfile that is too large for deletion, \n\rplease pload and deny then run {Wclean deny{x.");
        add_buf(output,buf);
    }

    sprintf(buf,"\n\rParsed through %d pfiles, of which %d where deleted.\n\rCurrent player base is %d.\n\r",count,delcount,count-delcount);
    add_buf(output,buf);
    page_to_char(buf_string(output),ch);
    free_buf(output);
}

bool remove_pfile(char *pfile)
{
    //Perform the remove command - currently set to mv pfile ../todelete/pfile
    char move_command[MIL];
    int result;

    sprintf(move_command,"%s %s%s %s",MOVE_CMD,PLAYER_DIR,pfile,DELETE_DIR);
    result=system(move_command);

    return result;
}

/* Modified get_specific_flag_from_char_file
 Original From The Mage. */
bool get_specific_flag_from_char_file(char *player, char *key_field, long *numb)
{
    FILE *fp;
    char *word;
    bool fMatch;
    long field=0;
    char strsave[MSL];
    char letter;

    sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( player ) );
    if ((fp = fopen(strsave,"r")) == NULL)
	return(FALSE);

    letter = fread_letter( fp );

    word = fread_word( fp );
    if ( str_cmp( word, "PLAYER" ) )
    {
        return FALSE;
    }

    for ( ; ; )
    {
	word   = feof( fp ) ? "End" : fread_word( fp );
	fMatch = FALSE;
        if (!strcmp(word,key_field))
        {
            KEY2(key_field, field,fread_flag( fp ) );
            *numb = field;
	    fclose(fp);
	    return(TRUE);
	}
        if (!strcmp(word,"End"))
        {
	    fclose(fp);
	    return(FALSE);
	}
    }	
}
