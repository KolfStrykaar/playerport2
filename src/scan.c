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

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"

char *const distance[4]=
{
"right here.", "nearby to the %s.", "not far %s.", "off in the distance %s."
};

void scan_list           args((ROOM_INDEX_DATA *scan_room, CHAR_DATA *ch,
                               sh_int depth, sh_int door));
void scan_char           args((CHAR_DATA *victim, CHAR_DATA *ch,
                               sh_int depth, sh_int door));
void do_scan(CHAR_DATA *ch, char *argument)
{
    extern char * const dir_name[];
    char *arg = argument;
    char buf[MAX_STRING_LENGTH];
    char dir[6];
    CHAR_DATA *victim;
    EXIT_DATA *pexit;
    ROOM_INDEX_DATA *in_room;
    ROOM_INDEX_DATA *scan_room;
    int door, found, i, range, showed_something;
    int quiet = FALSE;

    if ( ch->desc == NULL )
	return;

   if ( ch->position < POS_SLEEPING )
   {
	send_to_char( "You can't see anything but stars!\n\r", ch );
	return;
   }

    /* see if "quiet" mode req'd (used by do_scout) */
    if ( arg[0] == 'q' )
    {
	quiet = TRUE;
	arg++;
    }

    /* find direction */
         if ( !str_cmp( arg, "n" ) || !str_cmp( arg, "north" ) ) door = 0;
    else if ( !str_cmp( arg, "e" ) || !str_cmp( arg, "east"  ) ) door = 1;
    else if ( !str_cmp( arg, "s" ) || !str_cmp( arg, "south" ) ) door = 2;
    else if ( !str_cmp( arg, "w" ) || !str_cmp( arg, "west"  ) ) door = 3;
    else if ( !str_cmp( arg, "u" ) || !str_cmp( arg, "up"    ) ) door = 4;
    else if ( !str_cmp( arg, "d" ) || !str_cmp( arg, "down"  ) ) door = 5;
    else
    {
	send_to_char( "You must specify a direction n,s,e,w,u, or d.\n\r", ch );
	return;
    }

    /* get name of direction */
    strcpy( dir, dir_name[door] );

    if ( !quiet )
    {
	sprintf( buf, "$n scans %s.", dir );
    	act( buf, ch, NULL, NULL, TO_ROOM );
    }

    /* figure out how far to look */
    if ( IS_AFFECTED2(ch, AFF_FARSIGHT) )
	{
   	if( ch->level >= 35 )
		range = 6;
    	else if ( ch->level >= 15)
		range = 4;
    	else
		range = 2;
	}
    else
   	if( ch->level >= 35 )
		range = 3;
    	else if ( ch->level >= 15)
		range = 2;
    	else
		range = 1;

    /* 'look direction' */
    in_room = ch->in_room;
    showed_something = FALSE;
    for(i = 0; i < range; i++) {
	if ( ( pexit = in_room->exit[door] ) == NULL )
	{
	    break;
	}
	if ( (scan_room = pexit->u1.to_room) == NULL
	    ||   !can_see_room(ch, scan_room )
	    ||   IS_SET(pexit->exit_info, EX_CLOSED) )
	{
	    break;
	}

	if ( !IS_SET(ch->act, PLR_HOLYLIGHT)
	&&   room_is_dark( scan_room ) )
	{
	    break;
	}

	if ( scan_room->people == NULL)
	{
	    in_room = scan_room;
	    continue;
	}

	/* look at occupants */
	for ( found = 0, victim = scan_room->people; victim != NULL; 
	victim = victim->next_in_room)
	{
	  if ( !can_see(ch, victim) )
          continue;
	
        if (is_affected(victim, gsn_doppelganger))
          continue;

/*        if (is_affected(victim, gsn_treeform))
          continue; */

	    /* found someone to show */
	    if( !found )
	     {
		char buf2[MAX_STRING_LENGTH];  		
		if (is_affected(victim, gsn_doppelganger)
            && !IS_SET(ch->act,PLR_HOLYLIGHT)
            && !IS_NPC(ch))
		{
              sprintf( buf2, "%s\n\r", PERS_DOPPEL(victim, ch) );
            }
            else
		if (is_affected(victim, gsn_treeform)
            && !IS_SET(ch->act,PLR_HOLYLIGHT)
            && !IS_NPC(ch))
		{
              sprintf( buf2, "A mighty oak tree\n\r" );
            }
            else
	      {
              sprintf( buf2, "%s\n\r", PERS( victim, ch ) );
	      } 
		dir[0] = UPPER( dir[0] );
		sprintf( buf, "%5s, range %d: %s",
		  dir, i + 1, buf2);
		found = showed_something = TRUE;
	      }
	    else 
	    {
	      if (is_affected(victim, gsn_doppelganger) 
		&& !IS_SET(ch->act,PLR_HOLYLIGHT)
		&& !IS_NPC(ch))
	      {
		  sprintf( buf, "                %s\n\r", PERS_DOPPEL(victim, ch) );
		}
	 	else
	      if (is_affected(victim, gsn_treeform) 
		&& !IS_SET(ch->act,PLR_HOLYLIGHT)
		&& !IS_NPC(ch))
	      {
		  sprintf( buf, "                A mighty oak tree\n\r" );
		}
	 	else
		{
        	  sprintf( buf, "                %s\n\r", PERS(victim, ch));
		}
	    }
	    send_to_char( buf, ch );
	}
	in_room = scan_room;
    }

    /* make sure they get some kinda acknowledge */
    if( !showed_something && !quiet ) {
	sprintf( buf, "You scan %s.", dir_name[door] );
	act( buf, ch, NULL, NULL, TO_CHAR );
    }
}

/*
 * do_scout
 */
void do_scout( CHAR_DATA *ch, char *argument )
{
    if ( ch->desc == NULL || IS_NPC( ch ) )
	return;

    if ( ch->position < POS_SLEEPING )
    {
	send_to_char( "You can't see anything but stars!\n\r", ch );
	return;
    }

    if ( ch->position == POS_SLEEPING )
    {
	send_to_char( "You can't see anything, you're sleeping!\n\r", ch );
	return;
    }

    if (!IS_NPC(ch)
    && !IS_SET(ch->act,PLR_HOLYLIGHT)
    && IS_AFFECTED(ch, AFF_BLIND))
    { 
	send_to_char( "You can't see a thing!\n\r", ch ); 
	return; 
    }

    if ( number_percent( ) < ch->pcdata->learned[gsn_scout] )
    {
	do_scan( ch, "qn" );
	do_scan( ch, "qs" );
	do_scan( ch, "qe" );
	do_scan( ch, "qw" );
	do_scan( ch, "qu" );
	do_scan( ch, "qd" );
	WAIT_STATE(ch,skill_table[gsn_scout].beats);  
	check_improve(ch,gsn_scout,TRUE,3);
    }
    else 
    {
	send_to_char( "You fail.\n\r", ch );
	WAIT_STATE(ch,skill_table[gsn_scout].beats);
	check_improve(ch,gsn_scout,FALSE,3);
    }
}

void scan_list(ROOM_INDEX_DATA *scan_room, CHAR_DATA *ch, sh_int depth,
               sh_int door)
{
   CHAR_DATA *rch;

   if (scan_room == NULL) return;
   for (rch=scan_room->people; rch != NULL; rch=rch->next_in_room)
   {
      if (rch == ch) continue;
      if (!IS_NPC(rch) && rch->invis_level > get_trust(ch)) continue;
      if (can_see(ch, rch)) scan_char(rch, ch, depth, door);
   }
   return;
}

void scan_char(CHAR_DATA *victim, CHAR_DATA *ch, sh_int depth, sh_int door)
{
   extern char *const dir_name[];
   extern char *const distance[];
   char buf[MAX_INPUT_LENGTH], buf2[MAX_INPUT_LENGTH];

   buf[0] = '\0';

   strcat(buf, PERS(victim, ch));
   strcat(buf, ", ");
   sprintf(buf2, distance[depth], dir_name[door]);
   strcat(buf, buf2);
   strcat(buf, "\n\r");
 
   send_to_char(buf, ch);
   return;
}
