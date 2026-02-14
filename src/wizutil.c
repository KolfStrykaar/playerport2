/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvements copyright (C) 1992, 1993 by Michael         *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefiting.  We hope that you share your changes too.  What goes       *
 *  around, comes around.                                                  *
 ***************************************************************************/

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include "merc.h"
#include "recycle.h"
#include "interp.h"


const char wizutil_id [] = "$Id: wizutil.c,v 1.6 1996/01/04 21:30:45 root Exp root $";

/*
===========================================================================
This snippet was written by Erwin S. Andreasen, erwin@pip.dknet.dk. You may 
use this code freely, as long as you retain my name in all of the files. You
also have to mail me telling that you are using it. I am giving this,
hopefully useful, piece of source code to you for free, and all I require
from you is some feedback.

Please mail me if you find any bugs or have any new ideas or just comments.

All my snippets are publically available at:

http://pip.dknet.dk/~pip1773/

If you do not have WWW access, try ftp'ing to pip.dknet.dk and examine
the /pub/pip1773 directory.
===========================================================================

  Various administrative utility commands.
  Version: 3 - Last update: January 1996.

  To use these 2 commands you will have to add a filename field to AREA_DATA.

  This value can be found easily in load_area while booting - the filename

  of the current area boot_db is reading from is in the strArea global.


  Since version 2 following was added:


  A rename command which renames a player. Search for do_rename to see

  more info on it.


  A FOR command which executes a command at/on every player/mob/location.  

  Fixes since last release: None.
  
  
*/


/* To have VLIST show more than vnum 0 - 9900, change the number below: */ 

#define MAX_SHOW_VNUM   99 /* show only 1 - 100*100 */

#define NUL '\0'


extern ROOM_INDEX_DATA *       room_index_hash         [MAX_KEY_HASH]; /* db.c */

/* opposite directions */
const sh_int opposite_dir [6] = { DIR_SOUTH, DIR_WEST, DIR_NORTH, DIR_EAST, DIR_DOWN, DIR_UP };


/* get the 'short' name of an area (e.g. MIDGAARD, MIRROR etc. */
/* assumes that the filename saved in the AREA_DATA struct is something like midgaard.are */
char * area_name (AREA_DATA *pArea)
{
	static char buffer[64]; /* short filename */
	char  *period;

	assert (pArea != NULL);
	
	strncpy (buffer, pArea->file_name, 64); /* copy the filename */	 //Changed filename to file_name Sirrion C004
	period = strchr (buffer, '.'); /* find the period (midgaard.are) */
	if (period) /* if there was one */
		*period = '\0'; /* terminate the string there (midgaard) */
		
	return buffer;	
}

typedef enum {exit_from, exit_to, exit_both} exit_status;

/* depending on status print > or < or <> between the 2 rooms */
void room_pair (ROOM_INDEX_DATA* left, ROOM_INDEX_DATA* right, exit_status ex, char *buffer)
{
	char *sExit;
	
	switch (ex)
	{
		default:
			sExit = "??"; break; /* invalid usage */
		case exit_from:
			sExit = "< "; break;
		case exit_to:
			sExit = " >"; break;
		case exit_both:
			sExit = "<>"; break;
	}
	
sprintf (buffer, "%5d %-26.26s %s%5d %-26.26s(%-8.8s)\n\r",
			  left->vnum, left->name, 
			  sExit,
			  right->vnum, right->name,
			  area_name(right->area)
	    );
}

/* for every exit in 'room' which leads to or from pArea but NOT both, print it */
void checkexits (ROOM_INDEX_DATA *room, AREA_DATA *pArea, char* buffer)
{
	char buf[MAX_STRING_LENGTH];
	int i;
	EXIT_DATA *exit;
	ROOM_INDEX_DATA *to_room;
	
	strcpy (buffer, "");
	for (i = 0; i < 6; i++)
	{
		exit = room->exit[i];
		if (!exit)
			continue;
		else
			to_room = exit->u1.to_room; //changed to_room to u1.to_room Sirrion C004
		
                if (to_room)  /* there is something on the other side */
                {

			if ( (room->area == pArea) && (to_room->area != pArea) )
			{ /* an exit from our area to another area */
			  /* check first if it is a two-way exit */
			
				if ( to_room->exit[opposite_dir[i]] &&
					to_room->exit[opposite_dir[i]]->u1.to_room == room ) //changed to_room to u1.to_room Sirrion C004
					room_pair (room,to_room,exit_both,buf); /* <> */
				else
					room_pair (room,to_room,exit_to,buf); /* > */
				
				strcat (buffer, buf);				
			}			
			else			
			if ( (room->area != pArea) && (exit->u1.to_room->area == pArea) ) //changed to_room to u1.to_room Sirrion C004
			{ /* an exit from another area to our area */

				if  (!
			    	 (to_room->exit[opposite_dir[i]] &&
				      to_room->exit[opposite_dir[i]]->u1.to_room == room ) //changed to_room to u1.to_room Sirrion C004
					)
				/* two-way exits are handled in the other if */
				{						
					room_pair (to_room,room,exit_from,buf);
					strcat (buffer, buf);
				}
				
			} /* if room->area */

                }

	} /* for */
	
}

/* for now, no arguments, just list the current area */
void do_exlist (CHAR_DATA *ch, char * argument)
{
	AREA_DATA* pArea;
	ROOM_INDEX_DATA* room;
	int i;
	char buffer[MAX_STRING_LENGTH];
	
	pArea = ch->in_room->area; /* this is the area we want info on */
	for (i = 0; i < MAX_KEY_HASH; i++) /* room index hash table */
	for (room = room_index_hash[i]; room != NULL; room = room->next)
	/* run through all the rooms on the MUD */
	
	{
		checkexits (room, pArea, buffer);
		send_to_char (buffer, ch);
	}
} 

/* show a list of all used VNUMS */

#define COLUMNS 		5   /* number of columns */
#define MAX_ROW 		((MAX_SHOW_VNUM / COLUMNS)+1) /* rows */

/* Commented out this code - the vlist arguments with objects, rooms, mobs, etc.
   has been replaced with FVLIST.  That makes this command redundant and no
   longer needed - Hiddukel - 23 November 2013
*/
//void do_vlist (CHAR_DATA *ch, char *argument)
//{
//	int i,j,vnum;
//	ROOM_INDEX_DATA *room;
//	char buffer[MAX_ROW*100]; /* should be plenty */
//	char buf2 [100];
//  char arg[MAX_INPUT_LENGTH];
//  char *string;
//
//  string = one_argument(argument,arg);
//	
//  if (arg[0] == '\0')
//   {
//	for (i = 0; i < MAX_ROW; i++)
//	{
//		strcpy (buffer, ""); /* clear the buffer for this row */
//		
//		for (j = 0; j < COLUMNS; j++) /* for each column */
//		{
//			vnum = ((j*MAX_ROW) + i); /* find a vnum whih should be there */
//			if (vnum < MAX_SHOW_VNUM)
//			{
//				room = get_room_index (vnum * 100 + 1); /* each zone has to have a XXX01 room */
//				sprintf (buf2, "%3d %-8.8s  ", vnum, 
//						 room ? area_name(room->area) : "-" ); 
//						 /* something there or unused ? */
//				strcat (buffer,buf2);				
//			} 
//		} /* for columns */
//		
//		send_to_char (buffer,ch);
//		send_to_char ("\n\r",ch);
//	} /* for rows */
//  }
//  j=0;
//  if (!str_cmp(arg,"obj"))
//    {
//      printf_to_char(ch,"{WFree {C%s{W vnum listing for area {C%s{x\n\r",arg,
//		     ch->in_room->area->name);
//      printf_to_char(ch,"{Y=============================================================================={C\n\r");
//      for (i = ch->in_room->area->min_vnum; i <= ch->in_room->area->max_vnum; i++) {
//	if (get_obj_index(i) == NULL) {
//	  printf_to_char(ch,"%8d, ",i);
//	  if (j == COLUMNS) {
//	    send_to_char("\n\r",ch);
//	    j=0;
//	  }
//	  j++;
//	}
//      }
//      send_to_char("{x\n\r",ch);
//      return;
//    }
//
//  if (!str_cmp(arg,"mob"))
//    { 
//      printf_to_char(ch,"{WFree {C%s {Wvnum listing for area {C%s{x\n\r",arg,
//		     ch->in_room->area->name);
//      printf_to_char(ch,"{Y=============================================================================={C\n\r");
//      for (i = ch->in_room->area->min_vnum; i <= ch->in_room->area->max_vnum; i++) {
//	if (get_mob_index(i) == NULL) {
//	  printf_to_char(ch,"%8d, ",i);
//	  if (j == COLUMNS) {
//	    send_to_char("\n\r",ch);
//	    j=0;
//	  }
//	  else j++;
//	}
//      }
//      send_to_char("{x\n\r",ch);
//      return;
//    }
//  if (!str_cmp(arg,"room"))
//    { 
//      printf_to_char(ch,"{WFree {C%s {Wvnum listing for area {C%s{x\n\r",arg,
//		     ch->in_room->area->name);
//      printf_to_char(ch,"{Y=============================================================================={C\n\r");
//      for (i = ch->in_room->area->min_vnum; i <= ch->in_room->area->max_vnum; i++) {
//	if (get_room_index(i) == NULL) {
//	  printf_to_char(ch,"%8d, ",i);
//	  if (j == COLUMNS) {
//	    send_to_char("\n\r",ch);
//	    j=0;
//	  }
//	  else j++;
//	}
//      }
//      send_to_char("{x\n\r",ch);
//      return;
//    }
//
//}


/*
 * do_rename renames a player to another name.
 * PCs only. Previous file is deleted, if it exists.
 * Char is then saved to new file.
 * New name is checked against std. checks, existing offline players and
 * online players. 
 * .gz files are checked for too, just in case.
 */

bool check_parse_name (char* name);  /* comm.c */
char *initial( const char *str );    /* comm.c */

/* Super-AT command:

FOR ALL <action>
FOR MORTALS <action>
FOR GODS <action>
FOR MOBS <action>
FOR EVERYWHERE <action>


Executes action several times, either on ALL players (not including yourself),
MORTALS (including trusted characters), GODS (characters with level higher than
L_HERO), MOBS (Not recommended) or every room (not recommended either!)

If you insert a # in the action, it will be replaced by the name of the target.

If # is a part of the action, the action will be executed for every target
in game. If there is no #, the action will be executed for every room containg
at least one target, but only once per room. # cannot be used with FOR EVERY-
WHERE. # can be anywhere in the action.

Example: 

FOR ALL SMILE -> you will only smile once in a room with 2 players.
FOR ALL TWIDDLE # -> In a room with A and B, you will twiddle A then B.

Destroying the characters this command acts upon MAY cause it to fail. Try to
avoid something like FOR MOBS PURGE (although it actually works at my MUD).

FOR MOBS TRANS 3054 (transfer ALL the mobs to Midgaard temple) does NOT work
though :)

The command works by transporting the character to each of the rooms with 
target in them. Private rooms are not violated.

*/

/* Expand the name of a character into a string that identifies THAT
   character within a room. E.g. the second 'guard' -> 2. guard
*/   
const char * name_expand (CHAR_DATA *ch)
{
	int count = 1;
	CHAR_DATA *rch;
	char name[MAX_INPUT_LENGTH]; /*  HOPEFULLY no mob has a name longer than THAT */

	static char outbuf[MAX_INPUT_LENGTH];	
	
	if (!IS_NPC(ch))
		return ch->name;
		
	one_argument (ch->name, name); /* copy the first word into name */
	
	if (!name[0]) /* weird mob .. no keywords */
	{
		strcpy (outbuf, ""); /* Do not return NULL, just an empty buffer */
		return outbuf;
	}
		
	for (rch = ch->in_room->people; rch && (rch != ch);rch = rch->next_in_room)
		if (is_name (name, rch->name))
			count++;
			

	sprintf (outbuf, "%d.%s", count, name);
	return outbuf;
}


void do_for (CHAR_DATA *ch, char *argument)
{
	char range[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	bool fGods = FALSE, fMortals = FALSE, fMobs = FALSE, fLevel = FALSE, fEverywhere = FALSE, found;
	ROOM_INDEX_DATA *room, *old_room;
	CHAR_DATA *p, *p_next;
	int i;
	
	argument = one_argument (argument, range);
	
	if (!range[0] || !argument[0]) /* invalid usage? */
	{
		//do_help (ch, "for"); //Commented out due to change C006

		send_to_char ("usage : for <WHO> <ACTION>\n\r",ch);
		return;
	}
	
	if (!str_prefix("quit", argument))
	{
		send_to_char ("Are you trying to crash the MUD or something?\n\r",ch);
		return;
	}
	
	
	if (!str_cmp (range, "all"))
	{
		fMortals = TRUE;
		fGods = TRUE;
	}
	else if (!str_cmp (range, "gods"))
		fGods = TRUE;
	else if( is_number(range) )
              if ( ( atoi( range ) < 1 ) || ( atoi( range ) > 110 ) )
              {
		    send_to_char ("Level argument may not be less than level 1, nor exceed level 110!\n\r",ch);
		    fLevel = FALSE;
	        }
              else
		    fLevel = TRUE;
	else if (!str_cmp (range, "mortals"))
		fMortals = TRUE;
	else if (!str_cmp (range, "mobs"))
		fMobs = TRUE;
	else if (!str_cmp (range, "everywhere"))
		fEverywhere = TRUE;
	else
		do_help (ch, "for"); /* show syntax */

	/* do not allow # to make it easier */		
	if (fEverywhere && strchr (argument, '#'))
	{
		send_to_char ("Cannot use FOR EVERYWHERE with the # thingie.\n\r",ch);
		return;
	}
		
	if (strchr (argument, '#')) /* replace # ? */
	{ 
		for (p = char_list; p ; p = p_next)
		{
			p_next = p->next; /* In case someone DOES try to AT MOBS SLAY # */
			found = FALSE;
			
			if (!(p->in_room) || room_is_private(p->in_room) || (p == ch))
				continue;
			
			if (IS_NPC(p) && fMobs)
				found = TRUE;
			else if (!IS_NPC(p) && p->level >= LEVEL_IMMORTAL && fGods)
				found = TRUE;
			else if (!IS_NPC(p) && p->level < LEVEL_IMMORTAL && fMortals)
				found = TRUE;
			else if (!IS_NPC(p) && p->level >= atoi( range ) && fLevel)
				found = TRUE;

			/* It looks ugly to me.. but it works :) */				
			if (found) /* p is 'appropriate' */
			{
			  char *pSource = argument; /* head of buffer to be parsed */
			  char *pDest = buf; /* parse into this */
				
			  while (*pSource)
			  {
			    if (*pSource == '#') /* Replace # with name of target */
			    {
			      const char *namebuf = name_expand (p);
						
			      if (namebuf) /* in case there is no mob name ?? */
			      while (*namebuf) /* copy name over */
			      *(pDest++) = *(namebuf++);

			      pSource++;
			    }
			    else
			      *(pDest++) = *(pSource++);
			  }
			  *pDest = '\0'; /* Terminate */

			  old_room = ch->in_room;
			  char_from_room (ch);
			  char_to_room (ch,p->in_room);
			  interpret (ch, buf);
			  char_from_room (ch);
			  char_to_room (ch,old_room);
				
			}
		}
	}
	else /* just for every room with the appropriate people in it */
	{
		for (i = 0; i < MAX_KEY_HASH; i++) /* run through all the buckets */
			for (room = room_index_hash[i] ; room ; room = room->next)
			{
				found = FALSE;
				
				/* Anyone in here at all? */
				if (fEverywhere) /* Everywhere executes always */
					found = TRUE;
				else if (!room->people) /* Skip it if room is empty */
					continue;
					
					
				/* Check if there is anyone here of the requried type */
				/* Stop as soon as a match is found or there are no more ppl in room */
				for (p = room->people; p && !found; p = p->next_in_room)
				{

					if (p == ch) /* do not execute on oneself */
						continue;
						
					if (IS_NPC(p) && fMobs)
						found = TRUE;
					else if (!IS_NPC(p) && (p->level >= LEVEL_IMMORTAL) && fGods)
						found = TRUE;
					else if (!IS_NPC(p) && (p->level <= LEVEL_IMMORTAL) && fMortals)
						found = TRUE;
				} /* for everyone inside the room */
						
				if (found && !room_is_private(room)) /* Any of the required type here AND room not private? */
				{
					/* This may be ineffective. Consider moving character out of old_room
					   once at beginning of command then moving back at the end.
					   This however, is more safe?
					*/
				
	old_room = ch->in_room;
	char_from_room (ch);
	char_to_room (ch, room);
	interpret (ch, argument);
	char_from_room (ch);
        char_to_room (ch, old_room);
      }
    }
  }
}

void do_users(CHAR_DATA *ch, char *argument )
{

    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    BUFFER *output;
    DESCRIPTOR_DATA *d;
    int nNumber;
    int nMatch;
 
    nNumber = 0;
 
    /*
     * show matching chars.
     */
    nMatch = 0;
    buf[0] = '\0';
    output = new_buf();

    sprintf(buf,"{yName            {GCla  {rLvl  {YHrs   {cInc  {bWiz  {WRoom   {MAlign   {gReligion\n\r{x");
    add_buf(output,buf);
    sprintf(buf,"-------------------------------------------------------------------------------\n\r");
    add_buf(output,buf);
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        CHAR_DATA *wch;
        char const *race;

        if ( d->connected != CON_PLAYING || !can_see( ch, d->character ) )
            continue;

        wch   = ( d->original != NULL ) ? d->original : d->character;


        nMatch++;
 
        if(wch->pcdata->racetext!=NULL)
        {
            race = str_dup(wch->pcdata->racetext);
        }
        else
        {
            race = pc_race_table[wch->race].who_name;
            switch ( wch->level )
            {
            default: break;
                {
                    case MAX_LEVEL - 0 : race  = "{BGod{x"; break;
                    case MAX_LEVEL - 1 : race  = "{BGod{x"; break;
                    case MAX_LEVEL - 2 : race  = "{BGod{x"; break;
                    case MAX_LEVEL - 3 : race  = "{BGod{x"; break;
                    case MAX_LEVEL - 4 : race  = "{BGod{x"; break;
                    case MAX_LEVEL - 5 : race  = "{BGod{x"; break;
                    case MAX_LEVEL - 6 : race  = "{BGod{x"; break;
                    case MAX_LEVEL - 7 : race  = "{BGod{x"; break;
                    case MAX_LEVEL - 8 : race  = "{BGod{x"; break;
                }
            }
        }

        sprintf(buf,"{y%-16s{G%-5s{C%-4d {r%-6d{Y%-5d{c%-5d{b%-7d{W%-8d{M%s{M%s{M%s{M%s{x\n\r",
                wch->name,
                class_table[wch->class].who_name,
                wch->level,
                (int) (wch->played + current_time - wch->logon) / 3600,
                wch->incog_level,
                wch->invis_level,
                wch->in_room->vnum,
                wch->alignment,
	        wch->devotee_id && !wch->religion_id ?
                religion_table[wch->devotee_id].longname : "" ,
	        wch->devotee_id && !wch->religion_id ? "(Devotee)" : "",
		wch->religion_id ? religion_table[RELIGION(wch->religion_id)].longname : "",
	        wch->religion_id & RELIGION_LEADER ? "(leader)" :
                ( wch->pcdata->highpriest == 1 ? "(high)" : "" ) );

        add_buf(output,buf);
    }

    sprintf( buf2, "\n\rPlayers found: %d\n\r", nMatch );
    add_buf(output,buf2);
    page_to_char( buf_string(output), ch );
    free_buf(output);
    return;
}

void do_sendhome(CHAR_DATA *ch, char *argument )
{
    char buf[MSL];
    CHAR_DATA *victim;
    ROOM_INDEX_DATA *location;
    char arg[MAX_INPUT_LENGTH];

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Send who home?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->desc == NULL )
    {
	send_to_char( "No descriptor to the char.\n\r", ch );
	return;
    }

    if (IS_NPC(victim) && !IS_SET(victim->act,ACT_PET))
    {
	send_to_char("Only players can be sent home.\n\r",ch);
	return;
    }

    if ((location=get_home(victim))==NULL)
    {
	send_to_char( "No clan hall or temple to send them to.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ))
    {
	send_to_char( "Nice try, but you failed.\n\r", ch );
	return;
    }

    if ( victim->in_room == location )
	return;

    if ( victim->fighting != NULL )
    {
	stop_fighting( victim, TRUE );
    }

    act( "$n disappears.", victim, NULL, NULL, TO_ROOM );
    char_from_room( victim );
    char_to_room( victim, location );
    act( "$n appears in the room.", victim, NULL, NULL, TO_ROOM );
    do_function(victim, &do_look, "auto" );
    sprintf(buf,"You have sent %s home.\n\r",victim->name);
    send_to_char(buf,ch);
    if (can_see(victim,ch))
    {
        sprintf(buf,"%s sent you home.\n\r",ch->name);
    }
    else
    {
        sprintf(buf,"An Unseen God sent you home.\n\r");
    }
    send_to_char(buf,victim);

    
    if (victim->pet != NULL)
    {
        char_from_room( victim->pet);
        char_to_room( victim->pet, location );
        act( "$n appears in the room.", victim->pet, NULL, NULL, TO_ROOM );
    }

    return;

}

void do_igrab(CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if (IS_NPC(ch))
    {
      send_to_char("Players or switched players only.\n\r",ch);
      return;
    }
    
    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char("\n\rSyntax:\n\r",ch);
	send_to_char("igrab <item> <char|mob>\n\r",ch);
	send_to_char("Takes an object off a player/mob, ignoring noremove flags etc.\n\r\n\r",ch);
	return; 
    }
    
    if((victim = get_char_room(ch, NULL, arg2)) == NULL)
    { 
        send_to_char( "There is no mob/player by that name here.\n\r", ch );
	return; 
    }


    
    if(!IS_NPC(victim)
    &&  victim->level > get_trust(ch) )
    {
        send_to_char( "They are too high level for you to do that.\n\r", ch );
	return; 
    }
    
    if ( ( obj = get_obj_carry( victim, arg1, ch ) ) == NULL )
    {
        if ( ( obj = get_obj_wear( victim, arg1, TRUE ) ) == NULL )
        {
            send_to_char( "You can't find it.\n\r", ch );
            return;
        }
    }
	
    obj_from_char( obj );
    obj_to_char( obj, ch );
    act("You grab $p.",ch,obj,NULL,TO_CHAR);
    send_to_char( "Got it!\n\r", ch );
    return;
}

void do_grab(CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Grab what from whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg2 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "That's pointless.\n\r", ch );
	return;
    }

    if(!IS_NPC(victim)
    &&  victim->level >= get_trust(ch) )
    {
        send_to_char( "They are too high level for you to do that.\n\r", ch );
	return; 
    }

    if (IS_NPC(victim))
    {
	send_to_char( "\n\rYour not allowed to GRAB items off of MOBs.\n\r", ch );
	send_to_char( "It screws up resets. If you want that item, KILL that MOB.\n\r\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_carry( victim, arg1, ch ) ) == NULL )
    {
        if ( ( obj = get_obj_wear( victim, arg1, TRUE ) ) == NULL )
        {
            send_to_char( "You can't find it.\n\r", ch );
            return;
        }
    }
	
    obj_from_char( obj );
    obj_to_char( obj, ch );
    act("You grab $p.",ch,obj,NULL,TO_CHAR);
    send_to_char( "Got it!\n\r", ch );
    return;
}
