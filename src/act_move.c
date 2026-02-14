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
#include <time.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include "merc.h"
#include "lang.h"
#include "interp.h"
#include "recycle.h"
#include "magic.h"
#include "tables.h"

DECLARE_DO_FUN(do_chpos         ); /* MGD */

bool  check_quiet_movement   args( ( CHAR_DATA *ch) );
void one_hit     		args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt, bool secondary ) );

char *	const	dir_name	[]		=
{
    "north", "east", "south", "west", "up", "down"
};

char *  const   dir_verbose_name []             =
{
    "northward", "eastward", "southward", "westward", "upward", "downward"
};

const	sh_int	rev_dir		[]		=
{
    2, 3, 0, 1, 5, 4
};

/* Amount of points per respective room sector that will be subtracted when character moves */
const	sh_int	movement_loss	[SECT_MAX]	=
{
  1,  /* SECT_INSIDE */
  2,	/* SECT_CITY */
  2,	/* SECT_FIELD */
  3,	/* SECT_FOREST */
  4,	/* SECT_HILLS */
  6,  /* SECT_MOUNTAIN */
  4,  /* SECT_WATER_SWIM */
  1,  /* SECT_WATER_NOSWIM */
  6,  /* SECT_UNUSED */
  10,	/* SECT_AIR */
  6,	/* SECT_DESERT */
  1,	/* SECT_GRAVEYARD */
  6,	/* SECT_POLAR */
  3,	/* SECT_SWAMP */
  1,	/* SECT_TRANSPORT */
  2,	/* SECT_UNDERWATER */
  1,	/* SECT_KOT */
  1,	/* SECT_SOLAMNIC */
  1,	/* SECT_CONCLAVE */
  1,	/* SECT_AESTHETIC */
  1,	/* SECT_THIEVES */
  1,	/* SECT_HOLYORDER */
  1,	/* SECT_MERCENARY */
  1,	/* SECT_UNDEAD */
  1,	/* SECT_FORESTER */
  1,	/* SECT_ARTISANS	*/
  1,	/* SECT_BLACKORDER */
  1,	/* SECT_CORRUPTION */ 
  1,	/* SECT_DEATH	*/
  1,	/* SECT_BLESSING	*/
  1,	/* SECT_KNOWLEDGE	*/
  1,	/* SECT_VIRTUE	*/
  1,	/* SECT_CHIVALRY	*/
  1,	/* SECT_WEALTH	*/
  1,	/* SECT_WISDOM	*/
  1,	/* SECT_BEAUTY	*/
  1,	/* SECT_DISEASE	*/
  1,	/* SECT_NATURE	*/
  1,	/* SECT_FLAME	*/
  1,	/* SECT_DESTRUCTION */
  1,	/* SECT_JUSTICE	*/
  1,	/* SECT_LIFE	*/
  1,	/* SECT_HATRED	*/
  1,	/* SECT_STORM	*/
  1,	/* SECT_BATTLE	*/
  1,	/* SECT_RMAGIC	*/
  1,	/* SECT_BMAGIC	*/
  1	/* SECT_WMAGIC	*/
};



/*
 * Local functions.
 */
bool	has_key		args( ( CHAR_DATA *ch, int key ) );
void 	chpos			args( (CHAR_DATA *ch, int POS ) );
void  poison		args( ( CHAR_DATA *ch ) );
void  slow			args( ( CHAR_DATA *ch ) );

/* Distance Rank*/
void do_advance (CHAR_DATA *ch, char *arg)
{
    chpos(ch, TRUE);
}

void do_backward (CHAR_DATA *ch, char *arg)
{
    chpos(ch, FALSE);
}


void chpos (CHAR_DATA *ch, int POS)
{
    CHAR_DATA *ich;
    int chance;
    int skills;
    int front_count = 0;
    int mid_count   = 0;
    int rear_count  = 0;
    bool fighting = FALSE;

    switch(POS)
    {

    /* checking for impossible mov before skill_improve*/
    case TRUE:
        if (ch->fight_pos == FIGHT_FRONT)
        {
                send_to_char("You are already positioned in the"
                " front rank of the group.\r\n", ch);
                return;
        }
	
		
    break; 
    case FALSE:
    for ( ich=ch->in_room->people ; ich != NULL ; ich=ich->next_in_room )
    {
        if ( is_same_group ( ch, ich ) 
        && ( ich != ch))
        {
          if (ich->fight_pos == FIGHT_FRONT)
            front_count ++;
          if (ich->fight_pos == FIGHT_MIDDLE)
            mid_count ++;
          if (ich->fight_pos == FIGHT_REAR)
            rear_count ++;
        }
     }
        
     if (ch->fight_pos == FIGHT_FRONT
     &&  front_count < 1)
     {
        send_to_char("There will not be enough people in the front row.\n\r",ch);
        return;
     } 

     if (ch->fight_pos == FIGHT_MIDDLE)
     {
        if (front_count < 1)
        {
           send_to_char("There are not enough people in the front row to move backward.\n\r",ch);
           return;
        }
      }
     if (ch->fight_pos == FIGHT_REAR)
     {
        send_to_char("You are already in the rear row of the group.\n\r",ch);
        return;
     }
    
     break;
     }
   for ( ich=ch->in_room->people ; ich != NULL ; ich=ich->next_in_room )
    {
        if ( is_same_group ( ch, ich )
        && ich->fighting)
	fighting = TRUE;
    }

	skills = get_skill(ch, gsn_fight_pos);
	chance = number_percent();
	
	if (skills > chance)
	{
		WAIT_STATE(ch, 15);
		check_improve(ch, gsn_fight_pos, TRUE, 1);
	}	
        else if ( 80 > chance )
        {
		if (fighting)
		WAIT_STATE(ch,30);
		else
		WAIT_STATE(ch, 15);
        	act("$n begins moving around somewhat awkwardly.", 
		ch, NULL, NULL, TO_ROOM);
            send_to_char("You begin to move around somewhat awkwardly.\r\n", ch);
            check_improve(ch,gsn_fight_pos,FALSE,1);
        }
        else
        {
    		if (ch->riding)
    		{
	  	  ch->riding->rider = NULL;
	  	  ch->riding        = NULL;
		}
            ch->position = POS_RESTING;
            WAIT_STATE(ch, 2 * skill_table[gsn_trip].beats);
		if (fighting)
		{
            send_to_char("You fail to place yourself in a better position.\r\n",ch);
            act("$n pauses for a moment, but fails to achieve a better position.",
		ch, NULL, NULL, TO_ROOM);
	      }
		else
		{
		send_to_char("You pause to estimate the best position to be in.\n\r", ch);
		act("$n pauses for a moment, estimating how to better position $mself", ch, NULL, NULL, TO_ROOM);
		}
		return;
        }

    
    switch(POS) {

    /* Moving forward in rank */
    case TRUE:
	if (ch->fight_pos == FIGHT_FRONT)
	{
		send_to_char("You are already positioned in the"
		" front rank of the group.\r\n", ch);
		return;
	}
	else if (ch->fight_pos == FIGHT_MIDDLE)
	{
		act("$n moves forward to the front rank of the group.", 
		ch, NULL, NULL, TO_ROOM);
		act("You move forward to the front rank of the group.",
		ch, NULL, NULL, TO_CHAR);

		ch->fight_pos = FIGHT_FRONT;
	if (ch->riding && !IS_NPC(ch))
	{
		ch->riding->fight_pos = FIGHT_FRONT;
		act("$n moves forward to the front rank of the group.",
		ch->riding, NULL, NULL, TO_ROOM);
	}

		return;
	}		
	else
	{
		act("$n moves forward to the middle rank of the group.",
		ch, NULL, NULL, TO_ROOM);
		act("You move forward to the middle rank of the group.",
		ch, NULL, NULL, TO_CHAR);

		ch->fight_pos = FIGHT_MIDDLE;
	if (ch->riding && !IS_NPC(ch))
	{
		ch->riding->fight_pos = FIGHT_MIDDLE;
		act("$n moves forward to the middle rank of the group.",
		ch->riding, NULL, NULL, TO_ROOM);
	}
		return;
	}
    break;

    /* Moving back in rank */
    case FALSE:
     
	if (ch->fight_pos == FIGHT_FRONT)
	{

        	act("$n moves back to the middle rank of the group.", 
		ch, NULL, NULL, TO_ROOM);
		act("You move back to the middle rank of the group.",
		ch, NULL, NULL, TO_CHAR);
		ch->fight_pos = FIGHT_MIDDLE;
	
	if (ch->riding && !IS_NPC(ch))
	{
		ch->riding->fight_pos = FIGHT_MIDDLE;
		act("$n moves back to the middle rank of the group.",
		ch->riding, NULL, NULL, TO_ROOM);
	}
		return;
	}
	else if (ch->fight_pos == FIGHT_MIDDLE)
	{

        	act("$n moves back to the rear rank of the group.", 
		ch, NULL, NULL, TO_ROOM);
		act("You move back to the rear rank of the group.",
		ch, NULL, NULL, TO_CHAR);

		ch->fight_pos = FIGHT_REAR;
	if (ch->riding && !IS_NPC(ch))
	{
		ch->riding->fight_pos = FIGHT_REAR;
		act("$n moves back to the rear rank of the group.",
		ch->riding, NULL, NULL, TO_ROOM);
		if (ch->riding->fighting)
		stop_fighting(ch->riding, FALSE);
	}
		if (ch->fighting)
			stop_fighting(ch, FALSE);
		return;
	}		
	else
	{
		send_to_char("You are already positioned in the"
		" rear rank of the group.\r\n", ch);
		return;
	}
    break;
    }
}


void move_char( CHAR_DATA *ch, int door, bool follow )
{
    CHAR_DATA *fch;
    CHAR_DATA *fch_next;
    ROOM_INDEX_DATA *in_room;
    ROOM_INDEX_DATA *to_room;
    EXIT_DATA *pexit;
    int quiet = 0; /* for quiet movement */
    int *dirtab;
    int vdoor;
    bool allowed = TRUE;
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *blood;
    OBJ_DATA *obj;
    int dam;
    int mirrors;
    CHAR_DATA *gch;

    if (IS_AFFECTED2(ch, AFF_PARALYSIS) )
    {
      send_to_char("You have been paralized! You couldn't even move a finger!\n\r",ch);
      return;
    }

    if (IS_AFFECTED3(ch, AFF_HOLDPERSON) )
    {
      send_to_char("You can not move at all!\n\r",ch);
      return;
    }

    if (IS_AFFECTED3(ch, AFF_QUICKSAND) )
    {

	if ( ch->move < 50 )
	{
	  send_to_char( "You are too exhausted.\n\r", ch );
	  return;
	}

	if (number_percent() > 50)
      {
        act("You try to move, but the quicksand holds you firmly in the ground!", ch,NULL,NULL,TO_CHAR);
        act("$n tries to move, but the quicksand holds $m firmly in the ground!", ch,NULL,NULL,TO_ROOM);
	  WAIT_STATE( ch, 15 );
	  return;
      }
      else
      {
        act("You successfully break free of the quicksand, but feel extremely exhausted from the process.", ch,NULL,NULL,TO_CHAR);
        act("$n successfully breaks free of the quicksand, but now looks extremely exhausted from the process", ch,NULL,NULL,TO_ROOM);
	  affect_strip(ch,skill_lookup("quicksand"));
	  ch->move = ch->move/2;
	  WAIT_STATE( ch, 8 );
      }
    }

    if (!IS_NPC(ch))
    {
      /* Uh oh, another drunk Frenchman on the loose! :) */
      if ((ch->pcdata->condition[COND_DRUNK] > 10)
      && !is_affected(ch,gsn_brainjar))
      {
        if (ch->pcdata->condition[COND_DRUNK] > number_percent())
        {
          act("You feel a little drunk.. not to mention kind of lost..",
              ch,NULL,NULL,TO_CHAR);
          act("$n looks a little drunk.. not to mention kind of lost..",
              ch,NULL,NULL,TO_ROOM);
          door = number_range(0,5);
        }
        else
        {
          act("You feel a little.. drunk..",ch,NULL,NULL,TO_CHAR);
          act("$n looks a little.. drunk..",ch,NULL,NULL,TO_ROOM);
        }
      }

      if (is_affected(ch,gsn_brainjar))
      {
        act("Your brain was jarred and your feeling dizzy.. not to mention kind of lost..",
              ch,NULL,NULL,TO_CHAR);
        act("$n looks incoherent... not to mention kind of lost..",
              ch,NULL,NULL,TO_ROOM);
        door = number_range(0,5);
      }

    }

    if ( door < 0 || door > 5 )
    {
	bug( "Do_move: bad door %d.", door );
	return;
    }

    /*
     * Exit trigger, if activated, bail out. Only PCs are triggered.
     */
    for (mirrors = 0, gch = char_list; gch != NULL; gch = gch->next)
    {

      if (IS_NPC(gch)
      && is_affected(gch,gsn_mirror)
      && is_affected(gch,gsn_doppelganger)
      && gch->doppel == ch)
        mirrors++;

      if (!IS_NPC(ch)
	&&  !IS_NPC(gch)
      && (mirrors == 0)
      && ( p_exit_trigger( ch, door, PRG_MPROG ) 
      ||   p_exit_trigger( ch, door, PRG_OPROG )
      ||   p_exit_trigger( ch, door, PRG_RPROG )))
	  return;
    }

    vdoor = door; /* For misdirection, but retain origional direction for move */

    if( IS_AFFECTED2(ch, AFF_LOOKING_GLASS) )
        dirtab = oppossite_dir;
    else
        dirtab = regular_dir;
    in_room = ch->in_room;
    if ( ( pexit   = in_room->exit[dirtab[vdoor]] ) == NULL
    ||   ( to_room = pexit->u1.to_room   ) == NULL 
    ||   !can_see_room(ch,pexit->u1.to_room))
    {
      if (!IS_NPC(ch)
	&& ch->pcdata->condition[COND_DRUNK] > 10
      && !is_affected(ch,gsn_brainjar)
	&& !ch->riding)
      {
        act("You stumble about aimlessly and fall down drunk.",
            ch, NULL, dir_verbose_name[vdoor], TO_CHAR);
        act("$n stumbles about aimlessly and falls down drunk.",
            ch, NULL, dir_verbose_name[vdoor], TO_ROOM);
        ch->position = POS_RESTING;
      }
      else
      if (!IS_NPC(ch)
      &&  is_affected(ch,gsn_brainjar)
	&& !ch->riding)
      {
        act("You stumble about aimlessly and fall down from dizziness.",
            ch, NULL, dir_verbose_name[vdoor], TO_CHAR);
        act("$n stumbles about aimlessly and falls down from dizziness.",
            ch, NULL, dir_verbose_name[vdoor], TO_ROOM);
        ch->position = POS_RESTING;
      }
      else
      {
        act("You almost go $T, but suddenly realize that there's no exit there.", ch, NULL, dir_verbose_name[vdoor], TO_CHAR);
        act("$n looks like $e's about to go $T, but stops short and looks confused.", ch, NULL, dir_verbose_name[vdoor], TO_ROOM);
      }
	return;
    }

    if ( IS_SET(pexit->exit_info,EX_NOPASS)
    && !IS_IMMORTAL(ch)
    && IS_SET(pexit->exit_info, EX_CLOSED)
    && IS_AFFECTED2(ch, AFF_WRAITHFORM) )
    {
	act( "You can't seem to go through the $d.", ch, NULL, pexit->keyword, TO_CHAR );
	return;
    }

    if ( IS_SET(pexit->exit_info,EX_NOPASS)
    && !IS_IMMORTAL(ch)
    && IS_SET(pexit->exit_info, EX_CLOSED)
    && IS_AFFECTED (ch, AFF_PASS_DOOR) )
    {
	act( "You can't seem to go through the $d.", ch, NULL, pexit->keyword, TO_CHAR );
	return;
    }

    if ( IS_SET(pexit->exit_info, EX_CLOSED)
    && ( IS_AFFECTED(ch, AFF_PASS_DOOR) 
    ||   IS_AFFECTED2(ch, AFF_WRAITHFORM))
    && ( ch->in_room->sector_type == SECT_TRANSPORT
    ||   ch->in_room->sector_type == SECT_TRANSPORT))
    {
        act( "The ship isn't docked, you'd step off into the water!", ch, NULL, pexit->keyword, TO_CHAR );
        return;
    }

    if ( IS_SET(pexit->exit_info, EX_CLOSED)
    &&   !IS_IMMORTAL(ch)
    &&   !IS_AFFECTED(ch, AFF_PASS_DOOR)
    &&   !IS_AFFECTED2(ch, AFF_WRAITHFORM))
    {
	act( "The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR );
	return;
    }

    if ( IS_SET(pexit->exit_info, EX_CLOSED)
    &&   (IS_AFFECTED(ch, AFF_PASS_DOOR)
    ||   IS_AFFECTED2(ch, AFF_WRAITHFORM))
    &&   ch->riding)
    {
	send_to_char("Can't go through it while mounted.\n\r", ch);
	return;
    }

    if ( IS_AFFECTED(ch, AFF_CHARM)
    &&   ch->master != NULL
    &&   in_room == ch->master->in_room )
    {
	send_to_char( "What?  And leave your beloved master?\n\r", ch );
	return;
    }

    if ( ch->riding && ch->riding->position < POS_STANDING )
    {
        send_to_char( "Your ride doesn't want to move right now.\n\r", ch );
        return;
    }

    if ( !is_room_owner(ch,to_room) && room_is_private( to_room ))
    {
	send_to_char( "That room is private right now.\n\r", ch );
	return;
    }

    if (IS_AFFECTED(ch, AFF_HIDE) && !IS_AFFECTED(ch, AFF_SNEAK) )
    {
        REMOVE_BIT( ch->affected_by, AFF_HIDE );
        act("You step out of the shadows.", ch, NULL, NULL, TO_CHAR);
        act("$n steps out of the shadows.", ch, NULL, NULL, TO_ROOM);
    }

    if ( IS_AFFECTED2(ch, AFF_CAMOUFLAGE) )
    {
        REMOVE_BIT( ch->affected2_by, AFF_CAMOUFLAGE );
        act("You leave your cover.", ch, NULL, NULL, TO_CHAR);
        act("$n leaves $s cover.", ch, NULL, NULL, TO_ROOM);
    }

    check_cpose(ch);

    if ( !IS_NPC(ch) )
    {
	int iClass, iGuild;
	int move;

      for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
	{
	    for ( iGuild = 0; iGuild < MAX_GUILD; iGuild ++)	
	    {
                if (to_room->vnum == class_table[iClass].guild[iGuild])
                {
                    if ( iClass == ch->class
			  || IS_IMMORTAL(ch))
                    {
                        allowed = TRUE;
                        iClass = MAX_CLASS;
                    }
                    else
                    {
                        allowed = FALSE;
                    }
                }
        	if (allowed == FALSE)
        	{
            send_to_char("You aren't allowed in there.\n\r", ch );
            return;
            }
	    }
        }

  if ( is_affected(ch,gsn_treeform)
  && ( ( ( in_room->sector_type == SECT_CITY )
  ||  ( to_room->sector_type == SECT_CITY ))
  || (( in_room->sector_type == SECT_WATER_SWIM )
  ||  ( to_room->sector_type == SECT_WATER_SWIM ))
  || (( in_room->sector_type == SECT_WATER_NOSWIM )
  ||	( to_room->sector_type == SECT_WATER_NOSWIM ))
  || (( in_room->sector_type == SECT_UNUSED )
  ||	( to_room->sector_type == SECT_UNUSED ))
  || (( in_room->sector_type == SECT_AIR )
  ||	( to_room->sector_type == SECT_AIR ))
  || (( in_room->sector_type == SECT_POLAR )
  ||	( to_room->sector_type == SECT_POLAR ))
  || (( in_room->sector_type == SECT_TRANSPORT )
  ||	( to_room->sector_type == SECT_TRANSPORT ))
  || (( in_room->sector_type == SECT_UNDERWATER )
  ||	( to_room->sector_type == SECT_UNUSED ))
  || (( in_room->sector_type == SECT_KOT )
  ||	( to_room->sector_type == SECT_UNUSED ))
  || (( in_room->sector_type == SECT_SOLAMNIC )
  ||	( to_room->sector_type == SECT_UNUSED ))
  || (( in_room->sector_type == SECT_CONCLAVE )
  ||	( to_room->sector_type == SECT_UNUSED ))
  || (( in_room->sector_type == SECT_AESTHETIC )
  ||	( to_room->sector_type == SECT_UNUSED )) 
  || (( in_room->sector_type == SECT_THIEVES )
  ||	( to_room->sector_type == SECT_UNUSED ))
  || (( in_room->sector_type == SECT_HOLYORDER )
  ||	( to_room->sector_type == SECT_UNUSED ))
  || (( in_room->sector_type == SECT_MERCENARY )
  ||	( to_room->sector_type == SECT_UNUSED ))
  || (( in_room->sector_type == SECT_UNDEAD )
  ||	( to_room->sector_type == SECT_UNUSED ))
  || (( in_room->sector_type == SECT_ARTISANS )
  ||	( to_room->sector_type == SECT_ARTISANS ))
  || (( in_room->sector_type == SECT_BLACKORDER )
  ||	( to_room->sector_type == SECT_BLACKORDER ))
  || (( in_room->sector_type == SECT_CORRUPTION )
  ||	( to_room->sector_type == SECT_CORRUPTION ))
  || (( in_room->sector_type == SECT_DEATH )
  ||	( to_room->sector_type == SECT_DEATH ))
  || (( in_room->sector_type == SECT_BLESSING )
  ||	( to_room->sector_type == SECT_BLESSING ))
  || (( in_room->sector_type == SECT_KNOWLEDGE )
  ||	( to_room->sector_type == SECT_KNOWLEDGE ))
  || (( in_room->sector_type == SECT_VIRTUE )
  ||	( to_room->sector_type == SECT_VIRTUE ))
  || (( in_room->sector_type == SECT_CHIVALRY )
  ||	( to_room->sector_type == SECT_CHIVALRY )) 
  || (( in_room->sector_type == SECT_WEALTH )
  ||	( to_room->sector_type == SECT_WEALTH  ))
  || (( in_room->sector_type == SECT_WISDOM )
  ||	( to_room->sector_type == SECT_WISDOM ))
  || (( in_room->sector_type == SECT_BEAUTY )
  ||	( to_room->sector_type == SECT_BEAUTY ))
  || (( in_room->sector_type == SECT_DISEASE )
  ||	( to_room->sector_type == SECT_DISEASE )) 
  || (( in_room->sector_type == SECT_NATURE )
  ||	( to_room->sector_type == SECT_NATURE ))
  || (( in_room->sector_type == SECT_FLAME )
  ||	( to_room->sector_type == SECT_FLAME ))
  || (( in_room->sector_type == SECT_DESTRUCTION )
  ||	( to_room->sector_type == SECT_DESTRUCTION ))
  || (( in_room->sector_type == SECT_JUSTICE )
  ||	( to_room->sector_type == SECT_JUSTICE )) 
  || (( in_room->sector_type == SECT_LIFE )
  ||	( to_room->sector_type == SECT_LIFE ))
  || (( in_room->sector_type == SECT_HATRED )
  ||	( to_room->sector_type == SECT_HATRED ))
  || (( in_room->sector_type == SECT_STORM )
  ||	( to_room->sector_type == SECT_STORM ))
  || (( in_room->sector_type == SECT_BATTLE ) 
  ||	( to_room->sector_type == SECT_BATTLE ))
  || (( in_room->sector_type == SECT_RMAGIC )
  ||	( to_room->sector_type == SECT_RMAGIC ))
  || (( in_room->sector_type == SECT_BMAGIC )
  ||	( to_room->sector_type == SECT_BMAGIC ))
  || (( in_room->sector_type == SECT_WMAGIC )
  ||	( to_room->sector_type == SECT_WMAGIC ))
  || (( in_room->sector_type == SECT_NONE )
  ||	( to_room->sector_type == SECT_NONE ))
  || (( IS_SET(ch->in_room->room_flags, ROOM_INDOORS)) 
  ||	( IS_SET(to_room->room_flags, ROOM_INDOORS))) ) )
  {
    affect_strip(ch,gsn_treeform);
    send_to_char("Moving onto ground where forests do not normally flourish,\n\r",ch);
    send_to_char("you lose your {gTreeform{x.\n\r\n\r",ch);
  }

	if ( in_room->sector_type == SECT_AIR
	||   to_room->sector_type == SECT_AIR )
	{
	    if ( ch->riding )
	    {
		if ( !IS_AFFECTED( ch->riding, AFF_FLYING ) ) 
		{
		    send_to_char( "Your mount can't fly.\n\r", ch );
		    return;
		}
	    }
	    else
	    {
	      if ( !IS_AFFECTED(ch, AFF_FLYING) && (!IS_AFFECTED2(ch, AFF_GLIDE) 
	      && !IS_IMMORTAL(ch) && !IS_AFFECTED2(ch, AFF_WRAITHFORM)))
	      {
		  send_to_char( "You can't fly.\n\r", ch );
		  return;
	      }
	    }
	}

      if ( in_room->sector_type == SECT_UNDERWATER
      ||   to_room->sector_type == SECT_UNDERWATER )
      {
            if ( !IS_AFFECTED(ch, AFF_WATERBREATHING) && !IS_IMMORTAL(ch)  )
            {
                send_to_char( "You can't breathe underwater.\n\r", ch );
                return;
            }
      }

      /* Start of code for Clan Guards, prevents players from going any further if called on */

      /* Thieves */
      if (to_room->sector_type == SECT_THIEVES)
      {
        for ( fch = in_room->people; fch != NULL; fch = fch_next )
        {
	  fch_next = fch->next_in_room;

	  if (IS_NPC(fch)
          && (fch->spec_fun == spec_lookup("spec_orgguard"))
          && (fch->fighting == NULL)
          &&  IS_AWAKE(fch)
          && !IS_IMMORTAL(ch)
	  && !IS_IMPLEMENTOR(ch))
          {
            switch(fch->spec_state)
            {
             case 0: /* normal */
              if(IS_THIEVES(ch))
              {
       	        if( ch->sex == 1)
                  sprintf(buf, "Ah, Sire, I trust your journeys have proven profitable.");
                else
                  sprintf(buf, "Ah, Mistress, you are simply ravashing today.");
	          do_say(fch,buf);
		    send_to_char( "\n\r", ch );
                continue;
              }
              else
              {
                if (number_percent() < 80)
                {
                do_say(fch, "We strongly suggest you find another place to travel.");
		    send_to_char( "\n\r", ch );
                act("$n knocks you on the head, then proceeds to drag you out.", fch, NULL, ch, TO_VICT);
		    send_to_char( "\n\r", ch );
                act("$n knocks $N on the head, then proceeds to drag $m out.", fch, NULL, ch, TO_NOTVICT);
	          sprintf(buf, "%s %d", ch->name, 1815);
                do_transfer(ch, buf);
	          return;
                }
                else
                {
	          sprintf(buf, "Fellow Brothers, our informants suggest a possible invasion by %s, so be prepared.", ch->name);
	          do_message(fch,buf);
                continue;
                }
              }
             case 1: /*alert*/
              if(IS_THIEVES(ch))
              {
                if( ch->sex == 1)
                sprintf(buf, "We are on full alert Master %s, be prepared for invasion", ch->name);
               else
                sprintf(buf, "We are on full alert Mistress %s, be prepared for invasion", ch->name);
                act("$n nods at you.", fch, NULL, ch, TO_VICT);
                act("$n nods at $N.", fch, NULL, ch, TO_NOTVICT);
                do_say(fch,buf);
		    send_to_char( "\n\r", ch );
                continue;
              }
              else
              {
                if( !IS_AFFECTED2(ch, AFF_INVUL) )
                {
                  do_say(fch, "Fool! Dare you to intrude!");
		      send_to_char( "\n\r", ch );
                  do_say(fch, "Here's a present from the good brothers.");
		      send_to_char( "\n\r", ch );
                  act("$n glares at you with fiery eyes.", fch, NULL, ch, TO_VICT);
                  act("$n glares at $N with fiery eyes.", fch, NULL, ch, TO_NOTVICT);
	            act("$n plunges his dagger deep into $N's back.", fch, NULL, ch, TO_NOTVICT);
	            act("$n plunges his dagger deep into your back.", fch, NULL, ch, TO_VICT);
	            one_hit( fch, ch, gsn_backstab,TRUE);
	            act("$n twists his dagger deeper into $N's back.", fch, NULL, ch, TO_NOTVICT);
	            act("$n twists his dagger deeper into your back.", fch, NULL, ch, TO_VICT);
	            one_hit( fch, ch, gsn_double_bs,TRUE);
                  do_function(fch, &do_murder, ch->name);
                  return;
                }
                else
                {
			sprintf(buf, "%s %d", ch->name, 1815);
                	do_transfer(ch, buf);
                  return;
		    }
              }
             case 2: /*off duty*/
              if(IS_THIEVES(ch))
              {
                continue;
              }
              else
              {
                continue;
              }
             default:
               continue;
            }
          }
        }
      }

      /* Conclave */
      if (to_room->sector_type == SECT_CONCLAVE)
      {
        for ( fch = in_room->people; fch != NULL; fch = fch_next )
        {
	  fch_next = fch->next_in_room;

	  if (IS_NPC(fch)
          && (fch->spec_fun == spec_lookup("spec_orgguard"))
          && (fch->fighting == NULL)
          &&  IS_AWAKE(fch)
          && !IS_IMMORTAL(ch)
	  && !IS_IMPLEMENTOR(ch))
          {
            switch(fch->spec_state)
            {
             case 0: /* normal */
              if(IS_CONCLAVE(ch))
              {
       	    if( ch->sex == 1)
                  sprintf(buf, "You may pass, Master %s.", ch->name);
                else
                  sprintf(buf, "You may pass, Mistress %s.", ch->name);
	          do_say(fch,buf);
		    send_to_char( "\n\r", ch );
                continue;
              }
              else
              {
                if (number_percent() < 80)
                {
                do_say(fch, "You are not welcome in the Tower of High Sorcery.");
		    send_to_char( "\n\r", ch );
                act("$n begings to speak in the tongue of magic.", fch, NULL, ch, TO_VICT);
		    send_to_char( "\n\r", ch );
                act("$n begins to speak in the tongue of magic.", fch, NULL, ch, TO_NOTVICT);
	          sprintf(buf, "%s %d", ch->name, 14325);
                do_transfer(ch, buf);
	          return;
                }
                else
                {
	          sprintf(buf, "The fool %s dares to tread near our tower, radiant ones!", ch->name);
	          do_message(fch,buf);
                continue;
                }
              }
             case 1: /*alert*/
              if(IS_CONCLAVE(ch))
              {
                if( ch->sex == 1)
                sprintf(buf, "We are on full alert Master %s, be prepared for invasion", ch->name);
               else
                sprintf(buf, "We are on full alert Mistress %s, be prepared for invasion", ch->name);
                act("$n nods at you.", fch, NULL, ch, TO_VICT);
                act("$n nods at $N.", fch, NULL, ch, TO_NOTVICT);
                do_say(fch,buf);
		    send_to_char( "\n\r", ch );
                continue;
              }
              else
              {
                if( !IS_AFFECTED2(ch, AFF_INVUL) )
                {
                  do_say(fch, "Fool! Dare you to intrude!");
		      send_to_char( "\n\r", ch );
                  act("$n glares at you with fiery eyes.", fch, NULL, ch, TO_VICT);
		      send_to_char( "\n\r", ch );
                  act("$n glares at $N with fiery eyes.", fch, NULL, ch, TO_NOTVICT);
                  do_function(fch, &do_murder, ch->name);
                  return;
                }
                else
                {
			sprintf(buf, "%s %d", ch->name, 14325);
                	do_transfer(ch, buf);
                  return;
		    }
              }
             case 2: /*off duty*/
              if(IS_CONCLAVE(ch))
              {
                continue;
              }
              else
              {
                continue;
              }
             default:
               continue;
            }
          }
        }
      }

      /* Solamnic */
      if (to_room->sector_type == SECT_SOLAMNIC)
      {
        for ( fch = in_room->people; fch != NULL; fch = fch_next )
        {
	  fch_next = fch->next_in_room;

	  if (IS_NPC(fch)
          && (fch->spec_fun == spec_lookup("spec_orgguard"))
          && (fch->fighting == NULL)
          &&  IS_AWAKE(fch)
          && !IS_IMMORTAL(ch)
	  && !IS_IMPLEMENTOR(ch))
          {
            switch(fch->spec_state)
            {
             case 0: /* normal */
              if(IS_SOLAMNIC(ch))
              {
       	    if( ch->sex == 1)
                  sprintf(buf, "We are honored by your presence, noble Lord %s.", ch->name);
                else
                  sprintf(buf, "We are honored by your presence, noble Lady %s.", ch->name);
	          do_say(fch,buf);
		    send_to_char( "\n\r", ch );
                interpret(fch,"salute");
		    send_to_char( "\n\r", ch );
                continue;
              }
              else
              {
                if (number_percent() < 80)
                {
                  do_say(fch, "Only Government Officials are allowed beyond this point.");
		      send_to_char( "\n\r", ch );
                  do_say(fch, "Foreigners and Non-Officials must have an invitation to enter the Solamnic Headquarters.");
  		      send_to_char( "\n\r", ch );
		      do_say(fch, "Let me show you the way out.");
		      send_to_char( "\n\r", ch );
                  act("$n escorts you out of the Solamnic Headquarters.", fch, NULL, ch, TO_VICT);
		      send_to_char( "\n\r", ch );
                  act("$n escorts $N out of the Solamnic Headquarters.", fch, NULL, ch, TO_NOTVICT);
	            sprintf(buf, "%s %d", ch->name, 3266);
                  do_transfer(ch, buf);
	            return;
                }
                else
                {
	            sprintf(buf, "Fellow Solamnics! Intruder %s is in our Headquarters!", ch->name);
	            do_message(fch,buf);
                  continue;
                }
              }
             case 1: /*alert*/
              if(IS_SOLAMNIC(ch))
              {
                if( ch->sex == 1)
                sprintf(buf, "Lord %s! The citadel is on alert! Be careful!", ch->name);
               else
                sprintf(buf, "Lady %s! The citadel is on alert! Be careful!", ch->name);
                do_say(fch,buf);
		    send_to_char( "\n\r", ch );
                interpret(fch,"salute");
		    send_to_char( "\n\r", ch );
                continue;
              }
              else
              {
                if( !IS_AFFECTED2(ch, AFF_INVUL) )
                {
                  if( (ch->hometown == 0)
                  &&  (ch->org_id == ORG_NONE))
                  {
                    do_say(fch,"Sorry, citizens may not adventure the Solamnic Headquarters at this time.");
		        send_to_char( "\n\r", ch );
			  sprintf(buf, "%s %d", ch->name, 3266);
                	  do_transfer(ch, buf);
                    return;
                  }
                  else
                  {
                    do_say(fch, "Intruder alert!");
		        send_to_char( "\n\r", ch );
                    act("$n glares at you with fiery eyes.", fch, NULL, ch, TO_VICT);
		        send_to_char( "\n\r", ch );
                    act("$n glares at $N with fiery eyes.", fch, NULL, ch, TO_NOTVICT);
                    do_function(fch, &do_murder, ch->name);
                    return;
                  }
                }
                else
                {
			sprintf(buf, "%s %d", ch->name, 3266);
                	do_transfer(ch, buf);
                  return;
		    }
              }
             case 2: /*off duty*/
              if(IS_SOLAMNIC(ch))
              {
                continue;
              }
              else
              {
                continue;
              }
             default:
               continue;
            }
          }
        }
      }

      /* KOT */
      if (to_room->sector_type == SECT_KOT)
      {
        for ( fch = in_room->people; fch != NULL; fch = fch_next )
        {
	  fch_next = fch->next_in_room;

	  if (IS_NPC(fch)
          && (fch->spec_fun == spec_lookup("spec_orgguard"))
          && (fch->fighting == NULL)
          &&  IS_AWAKE(fch)
          && !IS_IMMORTAL(ch)
	  && !IS_IMPLEMENTOR(ch))
          {
            switch(fch->spec_state)
            {
             case 0: /* normal */
              if(IS_KOT(ch))
              {
       	    if( ch->sex == 1)
                  sprintf(buf, "Enter, Lord %s.  May you bring strength to the Knights of Takhisis.", ch->name);
                else
                  sprintf(buf, "Enter, Lady %s.  May you bring strength to the Knights of Takhisis.", ch->name);
	          do_say(fch,buf);
		    send_to_char( "\n\r", ch );
                interpret(fch,"salute");
		    send_to_char( "\n\r", ch );
                continue;
              }
              else
              {
                if (number_percent() < 80)
                {
                  do_say(fch, "Only Government Officials are allowed beyond this point.");
		      send_to_char( "\n\r", ch );
                  do_say(fch, "Foreigners and Non-Officials must have an invitation to enter the KoT Stronghold.");
  		      send_to_char( "\n\r", ch );
		      do_say(fch, "Let me show you the way out.");
		      send_to_char( "\n\r", ch );
                  act("$n escorts you out of the KoT Stronghold.", fch, NULL, ch, TO_VICT);
		      send_to_char( "\n\r", ch );
                  act("$n escorts $N out of the KoT Stronghold.", fch, NULL, ch, TO_NOTVICT);
	            sprintf(buf, "%s %d", ch->name, 11283);
                  do_transfer(ch, buf);
	            return;
                }
                else
                {
	            sprintf(buf, "Fellow Knights of Takhisis! Intruder %s is in our Stronghold!", ch->name);
	            do_message(fch,buf);
                  continue;
                }
              }
             case 1: /*alert*/
              if(IS_KOT(ch))
              {
                if( ch->sex == 1)
                sprintf(buf, "Lord %s, there is word of a possible invasion. Be on alert!", ch->name);
               else
                sprintf(buf, "Lady %s, there is word of a possible invasion. Be on alert!", ch->name);
                do_say(fch,buf);
		    send_to_char( "\n\r", ch );
                interpret(fch,"salute");
		    send_to_char( "\n\r", ch );
                continue;
              }
              else
              {
                if( !IS_AFFECTED2(ch, AFF_INVUL) )
                {
                  if( (ch->hometown == 5)
                  &&  (ch->org_id == ORG_NONE))
                  {
                    do_say(fch,"Sorry, citizens may not adventure the KoT Stronghold at this time.");
		        send_to_char( "\n\r", ch );
			  sprintf(buf, "%s %d", ch->name, 11283);
                	  do_transfer(ch, buf);
                    return;
                  }
                  else
                  {
                    do_say(fch, "Intruder alert!");
		        send_to_char( "\n\r", ch );
                    act("$n glares at you with fiery eyes.", fch, NULL, ch, TO_VICT);
		        send_to_char( "\n\r", ch );
                    act("$n glares at $N with fiery eyes.", fch, NULL, ch, TO_NOTVICT);
                    do_function(fch, &do_murder, ch->name);
                    return;
                  }
                }
                else
                {
			sprintf(buf, "%s %d", ch->name, 11283);
                	do_transfer(ch, buf);
                  return;
		    }
              }
             case 2: /*off duty*/
              if(IS_KOT(ch))
              {
                continue;
              }
              else
              {
                continue;
              }
             default:
               continue;
            }
          }
        }
      }

      /* Mercenary */
      if (to_room->sector_type == SECT_MERCENARY)
      {
        for ( fch = in_room->people; fch != NULL; fch = fch_next )
        {
	  fch_next = fch->next_in_room;

	  if (IS_NPC(fch)
          && (fch->spec_fun == spec_lookup("spec_orgguard"))
          && (fch->fighting == NULL)
          &&  IS_AWAKE(fch)
          && !IS_IMMORTAL(ch)
	  && !IS_IMPLEMENTOR(ch))
          {
            switch(fch->spec_state)
            {
             case 0: /* normal */
              if(IS_MERCENARY(ch))
              {
       	    if( ch->sex == 1)
                  sprintf(buf, "Ah, Sire, I trust your journeys have proven profitable.");
                else
                  sprintf(buf, "Ah, Mistress, you are simply ravashing today.");
	          do_say(fch,buf);
		    send_to_char( "\n\r", ch );
                continue;
              }
              else
              {
                if (number_percent() < 80)
                {
                do_say(fch, "We strongly suggest you find another place to travel.");
		    send_to_char( "\n\r", ch );
                act("$n knocks you on the head, then proceeds to drag you out.", fch, NULL, ch, TO_VICT);
		    send_to_char( "\n\r", ch );
                act("$n knocks $N on the head, then proceeds to drag $m out.", fch, NULL, ch, TO_NOTVICT);
	          sprintf(buf, "%s %d", ch->name, 5618);
                do_transfer(ch, buf);
	          return;
                }
                else
                {
	          sprintf(buf, "Fellow Mercenaries, our informants suggest a possible invasion by %s, so be prepared.", ch->name);
	          do_message(fch,buf);
                continue;
                }
              }
             case 1: /*alert*/
              if(IS_MERCENARY(ch))
              {
                if( ch->sex == 1)
                sprintf(buf, "We are on full alert Master %s, be prepared for invasion", ch->name);
               else
                sprintf(buf, "We are on full alert Mistress %s, be prepared for invasion", ch->name);
                act("$n nods at you.", fch, NULL, ch, TO_VICT);
                act("$n nods at $N.", fch, NULL, ch, TO_NOTVICT);
                do_say(fch,buf);
		    send_to_char( "\n\r", ch );
                continue;
              }
              else
              {
                if( !IS_AFFECTED2(ch, AFF_INVUL) )
                {
                  do_say(fch, "Fool! Dare you to intrude!");
		      send_to_char( "\n\r", ch );
                  do_say(fch, "Here's a present from the Good Mercenaries.");
		      send_to_char( "\n\r", ch );
                  act("$n glares at you with fiery eyes.", fch, NULL, ch, TO_VICT);
                  act("$n glares at $N with fiery eyes.", fch, NULL, ch, TO_NOTVICT);
	            act("$n plunges his dagger deep into $N's back.", fch, NULL, ch, TO_NOTVICT);
	            act("$n plunges his dagger deep into your back.", fch, NULL, ch, TO_VICT);
	            one_hit( fch, ch, gsn_backstab,TRUE);
	            act("$n twists his dagger deeper into $N's back.", fch, NULL, ch, TO_NOTVICT);
	            act("$n twists his dagger deeper into your back.", fch, NULL, ch, TO_VICT);
	            one_hit( fch, ch, gsn_double_bs,TRUE);
                  do_function(fch, &do_murder, ch->name);
                  return;
                }
                else
                {
			sprintf(buf, "%s %d", ch->name, 5618);
                	do_transfer(ch, buf);
                  return;
		    }
              }
             case 2: /*off duty*/
              if(IS_MERCENARY(ch))
              {
                continue;
              }
              else
              {
                continue;
              }
             default:
               continue;
            }
          }
        }
      }

      /* Holy Order */
      if (to_room->sector_type == SECT_HOLYORDER)
      {
        for ( fch = in_room->people; fch != NULL; fch = fch_next )
        {
	  fch_next = fch->next_in_room;

	  if (IS_NPC(fch)
          && (fch->spec_fun == spec_lookup("spec_orgguard"))
          && (fch->fighting == NULL)
          &&  IS_AWAKE(fch)
          && !IS_IMMORTAL(ch)
	  && !IS_IMPLEMENTOR(ch))
          {
            switch(fch->spec_state)
            {
             case 0: /* normal */
              if(IS_HOLYORDER(ch))
              {
       	    if( ch->sex == 1)
                  sprintf(buf, "Greetings, Sir %s.", ch->name);
                else
                  sprintf(buf, "Greetings, Lady %s.", ch->name);
	          do_say(fch,buf);
		    send_to_char( "\n\r", ch );
                continue;
              }
              else
              {
                if (number_percent() < 80)
                {
                do_say(fch, "I'm sorry, but only Members of the Holy Order may enter the Temple.");
		    send_to_char( "\n\r", ch );
                act("$n shows you the way out.", fch, NULL, ch, TO_VICT);
		    send_to_char( "\n\r", ch );
                act("$n shows $N the way out.", fch, NULL, ch, TO_NOTVICT);
	          sprintf(buf, "%s %d", ch->name, 5719);
                do_transfer(ch, buf);
	          return;
                }
                else
                {
	          sprintf(buf, "Fellow Brothers and Sisters, our informants suggest a possible invasion by %s.", ch->name);
	          do_message(fch,buf);
                continue;
                }
              }
             case 1: /*alert*/
              if(IS_HOLYORDER(ch))
              {
                if( ch->sex == 1)
                sprintf(buf, "Greetings Sir %s, be on the lookout for possible intruders!", ch->name);
               else
                sprintf(buf, "Greetings Lady %s, be on the lookout for possible intruders!", ch->name);
                do_say(fch,buf);
		    send_to_char( "\n\r", ch );
                continue;
              }
              else
              {
                if( !IS_AFFECTED2(ch, AFF_INVUL) )
                {
                  do_say(fch, "Intruder! In the name of all that is Holy!");
		      send_to_char( "\n\r", ch );
                  do_function(fch, &do_murder, ch->name);
                  return;
                }
                else
                {
			sprintf(buf, "%s %d", ch->name, 5719);
                	do_transfer(ch, buf);
                  return;
		    }
              }
             case 2: /*off duty*/
              if(IS_HOLYORDER(ch))
              {
                continue;
              }
              else
              {
                continue;
              }
             default:
               continue;
            }
          }
        }
      }

      /* Aesthetic */
      if (to_room->sector_type == SECT_AESTHETIC)
      {
        for ( fch = in_room->people; fch != NULL; fch = fch_next )
        {
	  fch_next = fch->next_in_room;

	  if (IS_NPC(fch)
          && (fch->spec_fun == spec_lookup("spec_orgguard"))
          && (fch->fighting == NULL)
          &&  IS_AWAKE(fch)
          && !IS_IMMORTAL(ch)
	  && !IS_IMPLEMENTOR(ch))
          {
            switch(fch->spec_state)
            {
             case 0: /* normal */
              if(IS_AESTHETIC(ch))
              {
       	    if( ch->sex == 1)
                  sprintf(buf, "Greetings Librarian %s.", ch->name);
                else
                  sprintf(buf, "Greetings Librarian %s.", ch->name);
	          do_say(fch,buf);
		    send_to_char( "\n\r", ch );
                continue;
              }
              else
              {
                if (number_percent() < 80)
                {
		    send_to_char( "\n\r", ch );
                do_say(fch, "I'm sorry, Astinus has asked that there are to be no visitors today.");
		    send_to_char( "\n\r", ch );
                do_say(fch, "I'm really sorry. Here, let me show you the way out.");
		    send_to_char( "\n\r", ch );
                act("$n leads you out of the library.", fch, NULL, ch, TO_VICT);
		    send_to_char( "\n\r", ch );
                act("$n leads $N out of the library.", fch, NULL, ch, TO_NOTVICT);
	          sprintf(buf, "%s %d", ch->name, 3137);
                do_transfer(ch, buf);
	          return;
                }
                else
                {
	          sprintf(buf, "Fellow Librarians, our informants suggest a possible invasion by %s.", ch->name);
	          do_message(fch,buf);
                continue;
                }
              }
             case 1: /*alert*/
              if(IS_AESTHETIC(ch))
              {
                if( ch->sex == 1)
                sprintf(buf, "Greetings Librarian %s, be on the lookout for possible intruders!", ch->name);
               else
                sprintf(buf, "Greetings Librarian %s, be on the lookout for possible intruders!", ch->name);
                do_say(fch,buf);
		    send_to_char( "\n\r", ch );
                continue;
              }
              else
              {
                if( !IS_AFFECTED2(ch, AFF_INVUL) )
                {
                  do_say(fch, "Intruder! In the name of all that is Sacred!");
		      send_to_char( "\n\r", ch );
                  do_function(fch, &do_murder, ch->name);
                  return;
                }
                else
                {
			sprintf(buf, "%s %d", ch->name, 3137);
                	do_transfer(ch, buf);
                  return;
		    }
              }
             case 2: /*off duty*/
              if(IS_HOLYORDER(ch))
              {
                continue;
              }
              else
              {
                continue;
              }
             default:
               continue;
            }
          }
        }
      }

      /* Undead */
      if (to_room->sector_type == SECT_UNDEAD)
      {
        for ( fch = in_room->people; fch != NULL; fch = fch_next )
        {
	  fch_next = fch->next_in_room;

	  if (IS_NPC(fch)
          && (fch->spec_fun == spec_lookup("spec_orgguard"))
          && (fch->fighting == NULL)
          &&  IS_AWAKE(fch)
          && !IS_IMMORTAL(ch)
	  && !IS_IMPLEMENTOR(ch))
          {
            switch(fch->spec_state)
            {
             case 0: /* normal */
              if(IS_UNDEAD(ch))
              {
       	    if( ch->sex == 1)
                  sprintf(buf, "Greetings, Dark Lord %s.", ch->name);
                else
                  sprintf(buf, "Greetings, Dark Lady %s.", ch->name);
	          do_say(fch,buf);
		    send_to_char( "\n\r", ch );
                continue;
              }
              else
              {
                if (number_percent() < 80)
                {
                do_say(fch, "Begone vile creature of the Living!");
		    send_to_char( "\n\r", ch );
                act("$n speaks a few words of magic, sending you away.", fch, NULL, ch, TO_VICT);
		    send_to_char( "\n\r", ch );
                act("$n speaks a few words of magic, sending $N away.", fch, NULL, ch, TO_NOTVICT);
	          sprintf(buf, "%s %d", ch->name, 4838);
                do_transfer(ch, buf);
	          return;
                }
                else
                {
	          sprintf(buf, "Fellow Undead, Skeleton Forces suggest a possible invasion by %s.", ch->name);
	          do_message(fch,buf);
                continue;
                }
              }
             case 1: /*alert*/
              if(IS_UNDEAD(ch))
              {
                if( ch->sex == 1)
                sprintf(buf, "Greetings Dark Lord %s, be on the lookout for possible intruders!", ch->name);
               else
                sprintf(buf, "Greetings Dark Lady %s, be on the lookout for possible intruders!", ch->name);
                do_say(fch,buf);
		    send_to_char( "\n\r", ch );
                continue;
              }
              else
              {
                if( !IS_AFFECTED2(ch, AFF_INVUL) )
                {
                  do_say(fch, "Intruder! Your Soul shall soon belong to Chemosh forever!");
		      send_to_char( "\n\r", ch );
                  do_function(fch, &do_murder, ch->name);
                  return;
                }
                else
                {
			sprintf(buf, "%s %d", ch->name, 4838);
                	do_transfer(ch, buf);
                  return;
		    }
              }
             case 2: /*off duty*/
              if(IS_UNDEAD(ch))
              {
                continue;
              }
              else
              {
                continue;
              }
             default:
               continue;
            }
          }
        }
      }

      /* Forester */
      if (to_room->sector_type == SECT_FORESTER)
      {
        for ( fch = in_room->people; fch != NULL; fch = fch_next )
        {
	  fch_next = fch->next_in_room;

	  if (IS_NPC(fch)
          && (fch->spec_fun == spec_lookup("spec_orgguard"))
          && (fch->fighting == NULL)
          &&  IS_AWAKE(fch)
          && !IS_IMMORTAL(ch)
	  && !IS_IMPLEMENTOR(ch))
          {
            switch(fch->spec_state)
            {
             case 0: /* normal */
              if(IS_FORESTER(ch))
              {
       	    if( ch->sex == 1)
                  sprintf(buf, "Greetings Lord of the Forest, %s.", ch->name);
                else
                  sprintf(buf, "Greetings Lady of the Forest, %s.", ch->name);
	          do_say(fch,buf);
		    send_to_char( "\n\r", ch );
                continue;
              }
              else
              {
                if (number_percent() < 80)
                {
                do_say(fch, "It is the will of the Forest Masters, that you turn back now!");
		    send_to_char( "\n\r", ch );
                act("$n shows you the way out of the Temple of Nature.", fch, NULL, ch, TO_VICT);
		    send_to_char( "\n\r", ch );
                act("$n shows $N the way out of the Temple of Nature.", fch, NULL, ch, TO_NOTVICT);
	          sprintf(buf, "%s %d", ch->name, 2469);
                do_transfer(ch, buf);
	          return;
                }
                else
                {
	          sprintf(buf, "Fellow Foresters, a possible invasion detected by %s.", ch->name);
	          do_message(fch,buf);
                continue;
                }
              }
             case 1: /*alert*/
              if(IS_FORESTER(ch))
              {
                if( ch->sex == 1)
                sprintf(buf, "Greetings Lord of the Forest, %s, possible intruders may be about!", ch->name);
               else
                sprintf(buf, "Greetings Lady of the Forest, %s, possible intruders may be about!", ch->name);
                do_say(fch,buf);
		    send_to_char( "\n\r", ch );
                continue;
              }
              else
              {
                if( !IS_AFFECTED2(ch, AFF_INVUL) )
                {
                  do_say(fch, "Intruder! You shall learn the consequences of trespassing on sacred grounds!");
		      send_to_char( "\n\r", ch );
                  do_function(fch, &do_murder, ch->name);
                  return;
                }
                else
                {
			sprintf(buf, "%s %d", ch->name, 2469);
                	do_transfer(ch, buf);
                  return;
		    }
              }
             case 2: /*off duty*/
              if(IS_FORESTER(ch))
              {
                continue;
              }
              else
              {
                continue;
              }
             default:
               continue;
            }
          }
        }
      }

      /* Artisans */
      if (to_room->sector_type == SECT_ARTISANS)
      {
        for ( fch = in_room->people; fch != NULL; fch = fch_next )
        {
	  fch_next = fch->next_in_room;

	  if (IS_NPC(fch)
          && (fch->spec_fun == spec_lookup("spec_orgguard"))
          && (fch->fighting == NULL)
          &&  IS_AWAKE(fch)
          && !IS_IMMORTAL(ch)
	  && !IS_IMPLEMENTOR(ch))
          {
            switch(fch->spec_state)
            {
             case 0: /* normal */
              if(IS_ARTISANS(ch))
              {
       	    if( ch->sex == 1)
                  sprintf(buf, "I've been expecting you, %s my lad. There's work to be done!", ch->name);
                else
                  sprintf(buf, "I've been expecting you, %s my lass.  The workers are expecting those sandwiches!", ch->name);
	          do_say(fch,buf);
		    send_to_char( "\n\r", ch );
                continue;
              }
              else
              {
                if (number_percent() < 80)
                {
                do_say(fch, "This passage is closed off.  Keep to the main mine shaft!");
		    send_to_char( "\n\r", ch );
	          sprintf(buf, "%s %d", ch->name, 10723);
                do_transfer(ch, buf);
	          return;
                }
                else
                {
	          sprintf(buf, "Fellow Artisans, Intruders at the Forge.");
	          do_message(fch,buf);
                continue;
                }
              }
             case 1: /*alert*/
              if(IS_ARTISANS(ch))
              {
                if( ch->sex == 1)
                sprintf(buf, "To work with you, %s my lad!  The stone's out of the quarry, if you catch my meaning!", ch->name);
               else
                sprintf(buf, "Get in there, %s my lass! Those lads need your help, if you know what I mean!", ch->name);
                do_say(fch,buf);
		    send_to_char( "\n\r", ch );
                continue;
              }
              else
              {
                if( !IS_AFFECTED2(ch, AFF_INVUL) )
                {
                  do_say(fch, "Out with ye! I told ye, this mine is closed!");
		      send_to_char( "\n\r", ch );
                  do_function(fch, &do_murder, ch->name);
                  return;
                }
                else
                {
			sprintf(buf, "%s %d", ch->name, 10723);
                	do_transfer(ch, buf);
                  return;
		    }
              }
             case 2: /*off duty*/
              if(IS_ARTISANS(ch))
              {
                continue;
              }
              else
              {
                continue;
              }
             default:
               continue;
            }
          }
        }
      }

      /* Black Order */
      if (to_room->sector_type == SECT_BLACKORDER)
      {
        for ( fch = in_room->people; fch != NULL; fch = fch_next )
        {
	  fch_next = fch->next_in_room;

	  if (IS_NPC(fch)
          && (fch->spec_fun == spec_lookup("spec_orgguard"))
          && (fch->fighting == NULL)
          &&  IS_AWAKE(fch)
          && !IS_IMMORTAL(ch)
	  && !IS_IMPLEMENTOR(ch))
          {
            switch(fch->spec_state)
            {
             case 0: /* normal */
              if(IS_BLACKORDER(ch))
              {
       	    if( ch->sex == 1)
                  sprintf(buf, "Greetings %s, chosen Dark One of the Black Order!", ch->name);
                else
                  sprintf(buf, "Greetings %s, chosen Dark One of the Black Order!", ch->name);
	          do_say(fch,buf);
		    send_to_char( "\n\r", ch );
                continue;
              }
              else
              {
                if (number_percent() < 80)
                {
                do_say(fch, "You are not welcome amongst the Unholy!");
		    send_to_char( "\n\r", ch );
	          sprintf(buf, "%s %d", ch->name, 11123);
                do_transfer(ch, buf);
	          return;
                }
                else
                {
	          sprintf(buf, "Fellow Dark Ones, Intruders infiltrate our Temple.");
	          do_message(fch,buf);
                continue;
                }
              }
             case 1: /*alert*/
              if(IS_BLACKORDER(ch))
              {
                if( ch->sex == 1)
                sprintf(buf, "Greetings your most Unholiness, %s", ch->name);
               else
                sprintf(buf, "Greetings your most Unholiness, %s!", ch->name);
                do_say(fch,buf);
		    send_to_char( "\n\r", ch );
                continue;
              }
              else
              {
                if( !IS_AFFECTED2(ch, AFF_INVUL) )
                {
                  do_say(fch, "Foul Blasphemer! You shall die!");
		      send_to_char( "\n\r", ch );
                  do_function(fch, &do_murder, ch->name);
                  return;
                }
                else
                {
			sprintf(buf, "%s %d", ch->name, 11123);
                	do_transfer(ch, buf);
                  return;
		    }
              }
             case 2: /*off duty*/
              if(IS_BLACKORDER(ch))
              {
                continue;
              }
              else
              {
                continue;
              }
             default:
               continue;
            }
          }
        }
      }

      if (( to_room->sector_type == SECT_FOREST
      ||    to_room->sector_type == SECT_HILLS ))
      {
	quiet = check_quiet_movement(ch);
      }

	if (( in_room->sector_type == SECT_WATER_NOSWIM
	|| to_room->sector_type == SECT_WATER_NOSWIM )
  	&& (!IS_AFFECTED(ch,AFF_FLYING)
	&& (!IS_AFFECTED2(ch,AFF_WRAITHFORM)
	&& (!IS_AFFECTED2(ch,AFF_GLIDE)))))
	{
	    OBJ_DATA *obj;
	    bool found;

	    /*
	     * Look for a boat.
	     */
	    found = FALSE;

	    if (IS_IMMORTAL(ch))
		found = TRUE;

	    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
	    {
		if ( obj->item_type == ITEM_BOAT )
		{
		    found = TRUE;
		    break;
		}
	    }
	    if ( !found )
	    {
		send_to_char( "You need a boat to go there.\n\r", ch );
		return;
	    }
	}

       if (IS_AFFECTED (ch, AFF_SNEAK )
       &&  ( to_room->sector_type == SECT_FOREST
       ||    to_room->sector_type == SECT_FIELD
       ||    to_room->sector_type == SECT_MOUNTAIN
       ||    to_room->sector_type == SECT_WATER_SWIM
       ||    to_room->sector_type == SECT_WATER_NOSWIM
       ||    to_room->sector_type == SECT_DESERT
       ||    to_room->sector_type == SECT_SWAMP
       ||    to_room->sector_type == SECT_POLAR
       ||    to_room->sector_type == SECT_UNDERWATER
       ||    to_room->sector_type == SECT_HILLS ))
       {
           REMOVE_BIT(ch->affected_by, AFF_SNEAK);
           affect_strip ( ch, gsn_sneak); 
           send_to_char("You no longer feel stealthy.\n\r",ch);
       }

       if (IS_AFFECTED (ch, AFF_HIDE )
       &&  ( to_room->sector_type == SECT_FOREST
       ||    to_room->sector_type == SECT_FIELD
       ||    to_room->sector_type == SECT_MOUNTAIN 
       ||    to_room->sector_type == SECT_WATER_SWIM
       ||    to_room->sector_type == SECT_WATER_NOSWIM
       ||    to_room->sector_type == SECT_DESERT
       ||    to_room->sector_type == SECT_SWAMP
       ||    to_room->sector_type == SECT_POLAR
       ||    to_room->sector_type == SECT_UNDERWATER
       ||    to_room->sector_type == SECT_HILLS ))
       {
	  send_to_char( "You step out of the shadows.\n\r", ch);
        REMOVE_BIT(ch->affected_by, AFF_HIDE);  
        act("$n steps out of the shadows.\n\r", ch, NULL, NULL, TO_ROOM); 
       }

      move = movement_loss[UMIN(SECT_MAX-1, in_room->sector_type)]
	     + movement_loss[UMIN(SECT_MAX-1, to_room->sector_type)]
	     ;

        move /= 2;  /* i.e. the average */

	/* conditional effects */
	if (IS_AFFECTED(ch,AFF_FLYING)
	|| (IS_AFFECTED2(ch,AFF_WRAITHFORM)
	|| (IS_AFFECTED(ch,AFF_HASTE)
      &&  IS_AFFECTED2(ch,AFF_GLIDE))))
	    move /= 2;

      if ( ch->riding )
	    move /=2 + move;

	if (IS_AFFECTED(ch,AFF_SLOW))
	    move *= 2;

	if (is_affected(ch,gsn_treeform))
	    move *= 2;

	if ( ch->move < move )
	{
	    send_to_char( "You are too exhausted.\n\r", ch );
	    return;
	}

	WAIT_STATE( ch, 1 );
	ch->move -= move;
    }

    if ((IS_AFFECTED(ch, AFF_FLYING)
    ||  IS_AFFECTED2(ch,AFF_GLIDE))
    &&   ch->invis_level < LEVEL_HERO)
	act( "$n flies $T.", ch, NULL, dir_verbose_name[vdoor], TO_ROOM );
else
    if (!IS_NPC(ch) && ch->riding)
    {
     buf[0] = '\0';
     if( !IS_AFFECTED(ch->riding, AFF_FLYING) )
       sprintf(buf, "$n leaves $T, riding on %s.", ch->riding->short_descr);
     else
       sprintf(buf, "$n soars $T, on %s.",ch->riding->short_descr);
     act( buf, ch, NULL, dir_verbose_name[vdoor], TO_ROOM );
    }
else
    if ( !IS_AFFECTED(ch, AFF_SNEAK)
    &&   ch->invis_level < LEVEL_HERO
    &&   !ch->rider
    && ( !quiet ))
    {
	if (!IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10)
      act("$n stumbles off drunkenly on $s way $T.", ch, NULL, dir_verbose_name[vdoor], TO_ROOM);
    else
      if ( in_room->sector_type == SECT_MOUNTAIN )
	  act( "$n hikes $T.", ch, NULL, dir_verbose_name[vdoor], TO_ROOM );
      else
      if ( IS_AFFECTED2(ch,AFF_WRAITHFORM ))
	  act( "$n floats $T.", ch, NULL, dir_verbose_name[vdoor], TO_ROOM );
      else
        act("$n leaves $T.", ch, NULL, dir_verbose_name[vdoor], TO_ROOM);
    }

    char_from_room( ch );
    char_to_room( ch, to_room );

    if (((IS_AFFECTED(ch, AFF_FLYING)
    || IS_AFFECTED2(ch,AFF_GLIDE))
    && ch->invis_level < LEVEL_HERO)
    && !ch->rider
    && ( !quiet )
    && !IS_AFFECTED(ch, AFF_SNEAK))
	act( "$n flies in like the wind.", ch, NULL, NULL, TO_ROOM );
else
    if ( !IS_AFFECTED(ch, AFF_SNEAK)
    &&   (ch->invis_level < LEVEL_HERO)
    && ( !quiet ))
    {
	     if ( ch->riding )
	    act( "$n has arrived mounted on $N.", ch, NULL, ch->riding, TO_ROOM );
	else
	    if ( !ch->rider )
		{
		 if (!IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10)
        	 act("$n stumbles in drunkenly, looking all nice and toasty.", ch,NULL,NULL,TO_ROOM);
    	  	else
        	 act("$n has arrived.",ch,NULL,NULL,TO_ROOM);
		}
    }

    do_function(ch, &do_look, "auto" );

    if (in_room == to_room) /* no circular follows */
	return;

    for ( fch = in_room->people; fch != NULL; fch = fch_next )
    {
	fch_next = fch->next_in_room;

	if ( fch->master == ch && IS_AFFECTED(fch,AFF_CHARM) 
	&&   fch->position < POS_STANDING)
	    do_function(fch, &do_stand, "");

	if ( fch->master == ch && fch->position == POS_STANDING 
	&&   can_see_room(fch,to_room)
      && (!IS_AFFECTED3(fch, AFF_HOLDPERSON)) )
	{

	    if (IS_SET(ch->in_room->room_flags,ROOM_LAW)
	    &&  (IS_NPC(fch) && IS_SET(fch->act,ACT_AGGRESSIVE)))
	    {
		act("You can't bring $N into the city.",
		    ch,NULL,fch,TO_CHAR);
		act("You aren't allowed in the city.",
		    fch,NULL,NULL,TO_CHAR);
		continue;
	    }

	    if(fch->tethered) continue;

	    act( "You follow $N.", fch, NULL, ch, TO_CHAR );
	    move_char( fch, door, TRUE );
	}
    }

    if (!IS_NPC(ch) && ch->hit <= ch->max_hit /2
    && !IS_IMMORTAL(ch)
    && !IS_AFFECTED2(ch, AFF_PASSTRACE))
    {
      blood =  create_object( get_obj_index(OBJ_VNUM_BLOOD),ch->level);
      obj_to_room( blood, ch->in_room );
      sprintf(buf,"A trail of blood lies here.");
      free_string( blood->description );
      blood->description = str_dup( buf );
      blood->timer = 2;
    }

    /* 
     * If someone is following the char, these triggers get activated
     * for the followers before the char, but it's safer this way...
     */
    if ( IS_NPC( ch ) && HAS_TRIGGER_MOB( ch, TRIG_ENTRY ) )
	p_percent_trigger( ch, NULL, NULL, NULL, NULL, NULL, TRIG_ENTRY );
    if ( !IS_NPC( ch ) )
    {
    	p_greet_trigger( ch, PRG_MPROG );
	p_greet_trigger( ch, PRG_OPROG );
	p_greet_trigger( ch, PRG_RPROG );
    }

    for ( obj = ch->in_room->contents; obj != NULL; obj = obj->next_content )
    {
    if ( (obj->item_type == ITEM_BOOBYTRAP || obj->item_type == ITEM_TRAP)
    &&  obj->value[1] == 0)
    {
      if (IS_SET(obj->wear_flags, ITEM_TAKE)) 
       return;

      if (IS_AFFECTED(ch,AFF_FLYING)
      && (!ch->riding))  
        return;

      if (IS_NPC(ch))
       return;

      if( IS_IMMORTAL(ch))
       return;

      if (number_percent( ) < ch->pcdata->learned[gsn_disarm_traps])
      {
       check_improve(ch,gsn_disarm_traps,TRUE,3);
       sprintf(buf, "\n\r$n spots $p on the ground, skillfully disarms and discards it.");
         act( buf, ch, obj, NULL, TO_ROOM );
       sprintf(buf, "\n\rYou spot %s on the ground. You skillfully disarm and discard it.\n\r",
         obj->short_descr);
	 send_to_char( buf, ch );
       extract_obj(obj);
      }
      else
      {
        if ( ch->riding)
        {
	  fch = ch->riding;

          if (fch->master == ch)
          {
            send_to_char("\n\rYour mount trips a wire and explodes into pieces from\n\r",ch);
            send_to_char("a well placed exploding boobytrap, but saves you in the process.\n\r",ch);
	      act( "$n's mount is blown to pieces, sending $n flying to the ground.",
	      ch, NULL, NULL, TO_NOTVICT );
	      ch->riding    = NULL;
	      ch->position  = POS_RESTING;
            extract_char( fch, TRUE );
            extract_obj(obj);
            return;
          }
        }
       dam = number_range( ch->level / 2, ch->level * 3 / 2 );
       if( ch->pcdata->learned[gsn_disarm_traps] > 1 )
        check_improve(ch,gsn_disarm_traps,FALSE,3);
       sprintf(buf, "\n\r$n trips a wire and $p explodes!");
       act( buf, ch, obj, NULL, TO_ROOM );
	 sprintf( buf, "\n\rYou trip a wire and %s explodes! [%d]\n\r",
	  obj->short_descr, dam);
	 send_to_char( buf, ch );
       ch->hit -= dam;
       extract_obj(obj);
      }
    }

    if ( (obj->item_type == ITEM_BOOBYTRAP || obj->item_type == ITEM_TRAP)
    &&  obj->value[1] == 11)
    {
      if (IS_SET(obj->wear_flags, ITEM_TAKE)) 
        return;

      if (IS_AFFECTED(ch,AFF_FLYING)
      && (!ch->riding))  
        return;

      if (IS_NPC(ch))
        return;

      if( IS_IMMORTAL(ch))
       return;

      if (number_percent( ) < ch->pcdata->learned[gsn_disarm_traps])
      {
       check_improve(ch,gsn_disarm_traps,TRUE,3);
       sprintf(buf, "\n\r$n spots $p on the ground, skillfully disarms and discards it.");
         act( buf, ch, obj, NULL, TO_ROOM );
       sprintf(buf, "\n\rYou spot %s on the ground. You skillfully disarm and discard it.\n\r",
         obj->short_descr);
	 send_to_char( buf, ch );
       extract_obj(obj);
      }
      else
      {
        CHAR_DATA *ich;
        if( ch->pcdata->learned[gsn_disarm_traps] > 1 )
          check_improve(ch,gsn_disarm_traps,FALSE,3);
       sprintf(buf, "\n\r$n trips a wire and $p lets out purple smoke!");
       act( buf, ch, obj, NULL, TO_ROOM );
	 sprintf( buf, "\n\rYou trip a wire and %s lets out purple smoke!\n\r",
	  obj->short_descr);
	 send_to_char( buf, ch );

        for ( ich = ch->in_room->people; ich != NULL; ich = ich->next_in_room )
        {
          if ( !IS_NPC(ich) && IS_SET(ich->act, PLR_WIZINVIS) )
            continue;
          affect_strip ( ich, gsn_invis                   );
          affect_strip ( ich, gsn_sneak                   );
          REMOVE_BIT   ( ich->affected_by, AFF_HIDE       );
          REMOVE_BIT   ( ich->affected_by, AFF_INVISIBLE  );
          REMOVE_BIT   ( ich->affected_by, AFF_SNEAK      );
	    REMOVE_BIT   ( ich->affected2_by, AFF_CAMOUFLAGE	);
	    REMOVE_BIT   ( ich->affected2_by, AFF_WRAITHFORM	);
          act( "$n is revealed!", ich, NULL, NULL, TO_ROOM );
          send_to_char( "You are revealed!\n\r", ich );
        }
	 extract_obj(obj);
      }
    }

    if ( (obj->item_type == ITEM_BOOBYTRAP || obj->item_type == ITEM_TRAP)
    &&  obj->value[1] == 2)
    {
      if (IS_NPC(ch))
       return;

      if (IS_AFFECTED(ch,AFF_FLYING)
      && (!ch->riding))  
        return;

      if (IS_IMMORTAL(ch))
       return;

      if (ch->level <= 5)
       return;

      if (number_percent( ) < ch->pcdata->learned[gsn_disarm_traps])
      {
       check_improve(ch,gsn_disarm_traps,TRUE,3);
       act("\n\rYou spot a waiting trap, but you know you can't disarm it.\n\r",
        ch, NULL, NULL, TO_CHAR);
       act("You instead avoid it altogether.\n\r",
        ch, NULL, NULL, TO_CHAR);
       return;
      }

      if (IS_AFFECTED2(ch, AFF_PASSTRACE))
      {
       act("\n\rYou spot a waiting trap and avoid it!\n\r",
        ch, NULL, NULL, TO_CHAR);
       return;
      }
 
      dam = number_range( ch->level / 3, ch->level * 2 / 2 );
      sprintf(buf, "\n\r$n sets off a waiting spell and it explodes!");
      act( buf, ch, obj, NULL, TO_ROOM );
	sprintf( buf, "\n\rA small glint catches your eye, but all too late as it explodes! [%d]\n\r", dam);
	send_to_char( buf, ch );
      ch->hit -= dam;
      extract_obj(obj);
    }

    if ( (obj->item_type == ITEM_BOOBYTRAP || obj->item_type == ITEM_TRAP)
    &&  obj->value[1] == 1)
    {
     if (IS_NPC(ch))
      return; 

     if (IS_SET(obj->wear_flags, ITEM_TAKE))
       return;

     if (IS_AFFECTED(ch,AFF_FLYING)
     && (!ch->riding))  
       return;

      if( IS_IMMORTAL(ch))
       return;

     if (number_percent( ) < ch->pcdata->learned[gsn_disarm_traps])
     {
       check_improve(ch,gsn_disarm_traps,TRUE,3);
       sprintf(buf, "\n\r$n spots $p on the ground, skillfully disarms and discards it.");
       act( buf, ch, obj, NULL, TO_ROOM );
       sprintf(buf, "\n\rYou spot %s on the ground. You skillfully disarm and discard it.\n\r",
         obj->short_descr);
	 send_to_char( buf, ch );
       extract_obj(obj);
     }
     else
     {
       if ( ch->riding)
       {
	 fch = ch->riding;

         if (fch->master == ch)
         {
         send_to_char("\n\rYour mount gets caught in a claw type trap, forcing you to kill\n\r",ch);
         send_to_char("and then bury it.\n\r",ch);
	   act( "$n's mount get's caught in a trap, forcing $n to kill it then bury it.",
	   ch, NULL, NULL, TO_NOTVICT );
	   ch->riding    = NULL;
	   ch->position  = POS_STANDING;
         extract_char( fch, TRUE );
         extract_obj(obj);
         return;
         }
       }

      if( ch->pcdata->learned[gsn_disarm_traps] > 1 )
      check_improve(ch,gsn_disarm_traps,FALSE,3);
      ch->move = -50;
      sprintf(buf, "\n\r$n falls to the ground as $s leg gets caught by $p!");
      act( buf, ch, obj, NULL, TO_ROOM );
	sprintf( buf, "\n\rYou fall to the ground as your leg gets caught in %s!\n\r",
       obj->short_descr);
	send_to_char( buf, ch );
      extract_obj(obj);
	ch->position = POS_RESTING;
     }
    }

    if ( (obj->item_type == ITEM_BOOBYTRAP || obj->item_type == ITEM_TRAP)
    &&  obj->value[1] == 3)
    {
      char buf[MAX_STRING_LENGTH];
      CHAR_DATA *alarm;

      if( IS_NPC(ch))
       return;

      if( IS_IMMORTAL(ch))
       return;

      if( !IS_AFFECTED(ch, AFF_INVISIBLE))
      {
       alarm = create_mobile( get_mob_index(MOB_VNUM_SPELL_ALARM));
       char_to_room(alarm, ch->in_room);
       sprintf(buf, "Intruder! Intruder at %s!", ch->in_room->name);
       alarm->comm = COMM_NOTELL;
       do_yell(alarm, buf);
       extract_char(alarm,TRUE);
      }
    }

    if ( (obj->item_type == ITEM_BOOBYTRAP || obj->item_type == ITEM_TRAP)
    &&  obj->value[1] == 5)
    {
     if (IS_NPC(ch))
      return;

     if (IS_AFFECTED(ch,AFF_FLYING)
     && (!ch->riding))  
       return;

     if (IS_SET(obj->wear_flags, ITEM_TAKE))
      return;

      if( IS_IMMORTAL(ch))
       return;

     if (number_percent( ) < ch->pcdata->learned[gsn_disarm_traps])
     {
       check_improve(ch,gsn_disarm_traps,TRUE,3);
       sprintf(buf, "\n\r$n spots $p on the ground, skillfully disarms and discards it.");
       act( buf, ch, obj, NULL, TO_ROOM );
       sprintf(buf, "\n\rYou spot %s on the ground. You skillfully disarm and discard it.\n\r",
         obj->short_descr);
	 send_to_char( buf, ch );
       extract_obj(obj);
     }
     else
     {
       dam = obj->level * 3;
       if( ch->pcdata->learned[gsn_disarm_traps] > 1 )
        check_improve(ch,gsn_disarm_traps,FALSE,3);
       sprintf(buf, "\n\r$n trips a wire and an arrow shoots out from a concealed location!");
       act( buf, ch, obj, NULL, TO_ROOM );
	 sprintf( buf, "\n\rYou trip a wire and an arrow shoots out and hits you! [%d]\n\r",
	  dam);
	 send_to_char( buf, ch );
       ch->hit -= dam;
       extract_obj(obj);
     }
    }

    if ( (obj->item_type == ITEM_BOOBYTRAP || obj->item_type == ITEM_TRAP)
    &&  obj->value[1] == 4)
    {
     if (IS_NPC(ch))
      return; 

     if (IS_AFFECTED(ch,AFF_FLYING)
     && (!ch->riding))  
       return;

     if (IS_SET(obj->wear_flags, ITEM_TAKE))
      return;

      if( IS_IMMORTAL(ch))
       return;

     if (number_percent( ) < ch->pcdata->learned[gsn_disarm_traps])
     {
       check_improve(ch,gsn_disarm_traps,TRUE,3);
       sprintf(buf, "\n\r$n spots $p on the ground, skillfully disarms and discards it.");
       act( buf, ch, obj, NULL, TO_ROOM );
       sprintf(buf, "\n\rYou spot %s on the ground. You skillfully disarm and discard it.\n\r",
         obj->short_descr);
	 send_to_char( buf, ch );
       extract_obj(obj);
     }
     else
     {
       if( ch->pcdata->learned[gsn_disarm_traps] > 1 )
        check_improve(ch,gsn_disarm_traps,FALSE,3);
       sprintf(buf, "\n\r$n trips a wire, releasing a cloud of poisonous gas!");
       act( buf, ch, obj, NULL, TO_ROOM );
	 sprintf( buf, "\n\rYou trip a wire, releasing a cloud of poisonous gas!\n\r");
	 send_to_char( buf, ch );
       send_to_char("You feel poison coursing through your veins.\n\r",ch);
       poison(ch);
       extract_obj(obj);
     }
    }

    if ( (obj->item_type == ITEM_BOOBYTRAP || obj->item_type == ITEM_TRAP)
    &&  obj->value[1] == 9)
    {
     if (IS_NPC(ch))
      return;

     if (IS_AFFECTED(ch,AFF_FLYING)
     && (!ch->riding))  
       return;

     if (IS_SET(obj->wear_flags, ITEM_TAKE))
      return;

      if( IS_IMMORTAL(ch))
       return;

     if (number_percent( ) < ch->pcdata->learned[gsn_disarm_traps])
     {
       check_improve(ch,gsn_disarm_traps,TRUE,3);
       sprintf(buf, "\n\r$n spots $p on the ground, skillfully disarms and discards it.");
       act( buf, ch, obj, NULL, TO_ROOM );
       sprintf(buf, "\n\rYou spot %s on the ground. You skillfully disarm and discard it.\n\r",
         obj->short_descr);
	 send_to_char( buf, ch );
       extract_obj(obj);
     }
     else
     {
      if( ch->pcdata->learned[gsn_disarm_traps] > 1 )
      check_improve(ch,gsn_disarm_traps,FALSE,3);
      ch->move = -200;
      sprintf(buf, "\n\r$n trips a wire and a large rock falls from the sky onto $m!");
      act( buf, ch, obj, NULL, TO_ROOM );
	sprintf( buf, "\n\rYou trip a wire and a large rock falls down on you, crushing your leg!\n\r");
	send_to_char( buf, ch );
	if (!IS_SET(ch->loc_hp2,BROKEN_LEG) && !IS_SET(ch->loc_hp2,LOST_LEG))
      {
	  SET_BIT(ch->loc_hp2,BROKEN_LEG);
      }
      extract_obj(obj);
      if ( ch->riding)
      {
	  ch->riding->rider = NULL;
	  ch->riding    = NULL;
      }
	ch->position = POS_RESTING;
     }
    }

    if ( (obj->item_type == ITEM_BOOBYTRAP || obj->item_type == ITEM_TRAP)
    &&  obj->value[1] == 8)
    {
      if (IS_SET(obj->wear_flags, ITEM_TAKE)) 
       return;

      if (IS_AFFECTED(ch,AFF_FLYING)
      && (!ch->riding))  
        return;

      if (IS_NPC(ch))
       return;

      if( IS_IMMORTAL(ch))
       return;

      if (number_percent( ) < ch->pcdata->learned[gsn_disarm_traps])
      {
       check_improve(ch,gsn_disarm_traps,TRUE,3);
       sprintf(buf, "\n\r$n spots $p on the ground, skillfully disarms and discards it.");
         act( buf, ch, obj, NULL, TO_ROOM );
       sprintf(buf, "\n\rYou spot %s on the ground. You skillfully disarm and discard it.\n\r",
         obj->short_descr);
	 send_to_char( buf, ch );
       extract_obj(obj);
      }
      else
      {
       dam = number_range( ch->level / 2, ch->level * 3 / 2 );
       if( ch->pcdata->learned[gsn_disarm_traps] > 1 )
        check_improve(ch,gsn_disarm_traps,FALSE,3);
       sprintf(buf, "\n\r$n trips a wire and a heavy portcullis slams down on $m!");
       act( buf, ch, obj, NULL, TO_ROOM );
	 sprintf( buf, "\n\rYou trip a wire and a heavy portcullis slams down on you! [%d]\n\r", dam);
	 send_to_char( buf, ch );
       ch->hit -= dam;
       if (!IS_SET(ch->loc_hp,BROKEN_SKULL))
       {
        make_part(ch,"brain");
        SET_BIT(ch->loc_hp,BROKEN_SKULL);
       }
       extract_obj(obj);
      }
    }

    if ( (obj->item_type == ITEM_BOOBYTRAP || obj->item_type == ITEM_TRAP)
    &&  obj->value[1] == 10)
    {
     if (IS_NPC(ch))
      return;

     if (IS_AFFECTED(ch,AFF_FLYING)
     && (!ch->riding))  
       return;

     if (IS_SET(obj->wear_flags, ITEM_TAKE))
      return;

      if( IS_IMMORTAL(ch))
       return;

     if (number_percent( ) < ch->pcdata->learned[gsn_disarm_traps])
     {
       check_improve(ch,gsn_disarm_traps,TRUE,3);
       sprintf(buf, "\n\r$n spots a hole in the ground and fills it in.");
       act( buf, ch, obj, NULL, TO_ROOM );
       send_to_char("You find a hole in the ground and fill it back in.\n\r",ch);
       extract_obj(obj);
     }
     else
     {
      if ( ch->riding)
      {
	fch = ch->riding;

        if (fch->master == ch)
        {
          send_to_char("\n\rYour mount walks over a small leaf-covered hole, it's leg falling in\n\r",ch);
          send_to_char("and snapping in half, forcing you to kill it, then bury it.\n\r",ch);
	    act( "$n's falls into a hole and snaps it's legs, forcing $n to kill it, then bury it.",
	    ch, NULL, NULL, TO_NOTVICT );
	    ch->riding    = NULL;
	    ch->position  = POS_RESTING;
          extract_char( fch, TRUE );
          extract_obj(obj);
          return;
        }
      }

      if( ch->pcdata->learned[gsn_disarm_traps] > 1 )
      check_improve(ch,gsn_disarm_traps,FALSE,3);
      ch->move = -100;
      sprintf(buf, "\n\r$n walks over a small leaf-covered hole and $s leg falls in!");
      act( buf, ch, obj, NULL, TO_ROOM );
	sprintf( buf, "\n\rYou walk over a leaf-coverd hole and your left leg falls in,\n\rbreaking it in the process!\n\r");
	send_to_char( buf, ch );
	if (!IS_SET(ch->loc_hp2,BROKEN_LEG) && !IS_SET(ch->loc_hp2,LOST_LEG))
      {
	  SET_BIT(ch->loc_hp2,BROKEN_LEG);
      }
      extract_obj(obj);
	ch->position = POS_RESTING;
     }
    }

    if ( (obj->item_type == ITEM_BOOBYTRAP || obj->item_type == ITEM_TRAP)
    &&  obj->value[1] == 7)
    {
     if (IS_NPC(ch))
      return;

     if (IS_AFFECTED(ch,AFF_FLYING)
     && (!ch->riding))  
       return;

     if (IS_SET(obj->wear_flags, ITEM_TAKE))
      return;

      if( IS_IMMORTAL(ch))
       return;

     dam = number_range( ch->level / 4, ch->level * 3 / 3 );

     if (IS_AFFECTED(ch, AFF_WATERBREATHING))
     {
	 send_to_char( "\n\rYou set off a trap which shoots forth water, trying to drown you.\n\r", ch );
       send_to_char( "Good thing you can breathe in water.\n\r", ch );
       extract_obj(obj);
     }
     else
     if (number_percent( ) < ch->pcdata->learned[gsn_disarm_traps])
     {
       check_improve(ch,gsn_disarm_traps,TRUE,3);
       sprintf(buf, "\n\r$n spots $p on the ground, skillfully disarms and discards it.");
       act( buf, ch, obj, NULL, TO_ROOM );
       sprintf(buf, "\n\rYou spot %s on the ground. You skillfully disarm and discard it.\n\r",
         obj->short_descr);
	 send_to_char( buf, ch );
       extract_obj(obj);
     }
     else
     {
      if( ch->pcdata->learned[gsn_disarm_traps] > 1 )
      check_improve(ch,gsn_disarm_traps,FALSE,3);
       sprintf(buf, "\n\r$n trips a wire and a rush of water fills $s lungs!");
       act( buf, ch, obj, NULL, TO_ROOM );
	 sprintf( buf, "\n\rYou trip a wire and a rush of water fills your lungs! [%d]\n\r", dam);
	 send_to_char( buf, ch );
       ch->hit -= dam;
      extract_obj(obj);
      if ( ch->riding)
      {
        send_to_char("\n\rThe blast of water knocks you off your mount.\n\r",ch);
	  ch->riding->rider = NULL;
	  ch->riding    = NULL;
      }
	ch->position = POS_RESTING;
     }
    }

    if ( (obj->item_type == ITEM_BOOBYTRAP || obj->item_type == ITEM_TRAP)
    &&  obj->value[1] == 6)
    {
     if (IS_NPC(ch))
      return;

     if (IS_AFFECTED(ch,AFF_FLYING)
     && (!ch->riding))  
       return;

     if (IS_SET(obj->wear_flags, ITEM_TAKE))
      return;

      if( IS_IMMORTAL(ch))
       return;

     if (number_percent( ) < ch->pcdata->learned[gsn_disarm_traps])
     {
       check_improve(ch,gsn_disarm_traps,TRUE,3);
       sprintf(buf, "\n\r$n spots $p on the ground, skillfully disarms and discards it.");
       act( buf, ch, obj, NULL, TO_ROOM );
       sprintf(buf, "\n\rYou spot %s on the ground. You skillfully disarm and discard it.\n\r",
         obj->short_descr);
	 send_to_char( buf, ch );
       extract_obj(obj);
     }
     else
     {
       if( ch->pcdata->learned[gsn_disarm_traps] > 1 )
        check_improve(ch,gsn_disarm_traps,FALSE,3);
       sprintf(buf, "\n\r$n trips a wire, releasing a net that covers and slows $m down!");
       act( buf, ch, obj, NULL, TO_ROOM );
	 sprintf( buf, "\n\rYou trip a wire, releasing a net that covers you, slowing you down!\n\r");
	 send_to_char( buf, ch );
       slow(ch);
       extract_obj(obj);
       if ( ch->riding)
       {
	   fch = ch->riding;

         if (fch->master == ch)
         {
           send_to_char("\n\rYour mount is frightened and takes off away.\n\r",ch);
	     ch->riding    = NULL;
           extract_char( fch, TRUE );
         }
       }
     }
    }

    }
    return;
}



void do_north( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_NORTH, FALSE );
    if (!IS_NPC(ch) && (!str_cmp(ch->pcdata->lokprot, "on")))
    {
      send_to_char( "lokprotmovednorth", ch );
    }
    return;
}



void do_east( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_EAST, FALSE );
    if (!IS_NPC(ch) && (!str_cmp(ch->pcdata->lokprot, "on")))
    {
      send_to_char( "lokprotmovedeast", ch );
    }
    return;
}



void do_south( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_SOUTH, FALSE );
    if (!IS_NPC(ch) && (!str_cmp(ch->pcdata->lokprot, "on")))
    {
      send_to_char( "lokprotmovedsouth", ch );
    }
    return;
}



void do_west( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_WEST, FALSE );
    if (!IS_NPC(ch) && (!str_cmp(ch->pcdata->lokprot, "on")))
    {
      send_to_char( "lokprotmovedwest", ch );
    }
    return;
}



void do_up( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_UP, FALSE );
    if (!IS_NPC(ch) && (!str_cmp(ch->pcdata->lokprot, "on")))
    {
      send_to_char( "lokprotmovedup", ch );
    }
    return;
}



void do_down( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_DOWN, FALSE );
    if (!IS_NPC(ch) && (!str_cmp(ch->pcdata->lokprot, "on")))
    {
      send_to_char( "lokprotmoveddown", ch );
    }
    return;
}



int find_door( CHAR_DATA *ch, char *arg )
{
    EXIT_DATA *pexit;
    int door;
    int *dirtab;

    if( IS_AFFECTED2(ch, AFF_LOOKING_GLASS) )
        dirtab = oppossite_dir;
    else
        dirtab = regular_dir;

	 if ( !str_cmp( arg, "n" ) || !str_cmp( arg, "north" ) ) door = dirtab[0];
    else if ( !str_cmp( arg, "e" ) || !str_cmp( arg, "east"  ) ) door = dirtab[1];
    else if ( !str_cmp( arg, "s" ) || !str_cmp( arg, "south" ) ) door = dirtab[2];
    else if ( !str_cmp( arg, "w" ) || !str_cmp( arg, "west"  ) ) door = dirtab[3];
    else if ( !str_cmp( arg, "u" ) || !str_cmp( arg, "up"    ) ) door = dirtab[4];
    else if ( !str_cmp( arg, "d" ) || !str_cmp( arg, "down"  ) ) door = dirtab[5];
    else
    {
	for ( door = 0; door <= 5; door++ )
	{
	    if ( ( pexit = ch->in_room->exit[door] ) != NULL
	    &&   IS_SET(pexit->exit_info, EX_ISDOOR)
	    &&   pexit->keyword != NULL
	    &&   is_name( arg, pexit->keyword ) )
		return door;
	}
	act( "I see no $T here.", ch, NULL, arg, TO_CHAR );
	return -1;
    }

    if ( ( pexit = ch->in_room->exit[door] ) == NULL )
    {
	act( "I see no door $T here.", ch, NULL, arg, TO_CHAR );
	return -1;
    }

    if ( !IS_SET(pexit->exit_info, EX_ISDOOR) )
    {
	send_to_char( "You can't do that.\n\r", ch );
	return -1;
    }

    return door;
}



void do_open( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Open what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_here( ch, NULL, arg ) ) != NULL )
    {
 	/* open portal */
	if (obj->item_type == ITEM_PORTAL)
	{
	    if (!IS_SET(obj->value[1], EX_ISDOOR))
	    {
		send_to_char("You can't do that.\n\r",ch);
		return;
	    }

	    if (!IS_SET(obj->value[1], EX_CLOSED))
	    {
		send_to_char("It's already open.\n\r",ch);
		return;
	    }

	    if (IS_SET(obj->value[1], EX_LOCKED))
	    {
		send_to_char("It's locked.\n\r",ch);
		return;
	    }

	    REMOVE_BIT(obj->value[1], EX_CLOSED);
	    act("You open $p.",ch,obj,NULL,TO_CHAR);
	    act("$n opens $p.",ch,obj,NULL,TO_ROOM);
	    return;
 	}

	/* 'open object' */
	if ( obj->item_type != ITEM_CONTAINER)
	    { send_to_char( "That's not a container.\n\r", ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "It's already open.\n\r",      ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSEABLE) )
	    { send_to_char( "You can't do that.\n\r",      ch ); return; }
	if ( IS_SET(obj->value[1], CONT_LOCKED) )
	    { send_to_char( "It's locked.\n\r",            ch ); return; }

	REMOVE_BIT(obj->value[1], CONT_CLOSED);
	act("You open $p.",ch,obj,NULL,TO_CHAR);
	act( "$n opens $p.", ch, obj, NULL, TO_ROOM );
	return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	/* 'open door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit = ch->in_room->exit[door];
	if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "It's already open.\n\r",      ch ); return; }
	if (  IS_SET(pexit->exit_info, EX_LOCKED) )
	    { send_to_char( "It's locked.\n\r",            ch ); return; }
	if (  IS_SET(pexit->exit_info, EX_MAGICSEAL) )
	    { send_to_char( "It's magically sealed.\n\r",  ch ); return; }

	REMOVE_BIT(pexit->exit_info, EX_CLOSED);
	act( "$n opens the $d.", ch, NULL, pexit->keyword, TO_ROOM );
	send_to_char( "Ok.\n\r", ch );

	/* open the other side */
	if ( ( to_room   = pexit->u1.to_room            ) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
	&&   pexit_rev->u1.to_room == ch->in_room )
	{
	    CHAR_DATA *rch;

	    REMOVE_BIT( pexit_rev->exit_info, EX_CLOSED );
	    for ( rch = to_room->people; rch != NULL; rch = rch->next_in_room )
		act( "The $d opens.", rch, NULL, pexit_rev->keyword, TO_CHAR );
	}
    }

    return;
}



void do_close( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Close what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_here( ch, NULL, arg ) ) != NULL )
    {
	/* portal stuff */
	if (obj->item_type == ITEM_PORTAL)
	{

	    if (!IS_SET(obj->value[1],EX_ISDOOR)
	    ||   IS_SET(obj->value[1],EX_NOCLOSE))
	    {
		send_to_char("You can't do that.\n\r",ch);
		return;
	    }

	    if (IS_SET(obj->value[1],EX_CLOSED))
	    {
		send_to_char("It's already closed.\n\r",ch);
		return;
	    }

	    SET_BIT(obj->value[1],EX_CLOSED);
	    act("You close $p.",ch,obj,NULL,TO_CHAR);
	    act("$n closes $p.",ch,obj,NULL,TO_ROOM);
	    return;
	}

	/* 'close object' */
	if ( obj->item_type != ITEM_CONTAINER )
	    { send_to_char( "That's not a container.\n\r", ch ); return; }
	if ( IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "It's already closed.\n\r",    ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSEABLE) )
	    { send_to_char( "You can't do that.\n\r",      ch ); return; }

	SET_BIT(obj->value[1], CONT_CLOSED);
	act("You close $p.",ch,obj,NULL,TO_CHAR);
	act( "$n closes $p.", ch, obj, NULL, TO_ROOM );
	return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	/* 'close door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit	= ch->in_room->exit[door];
	if ( IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "It's already closed.\n\r",    ch ); return; }

	SET_BIT(pexit->exit_info, EX_CLOSED);
	act( "$n closes the $d.", ch, NULL, pexit->keyword, TO_ROOM );
	send_to_char( "Ok.\n\r", ch );

	/* close the other side */
	if ( ( to_room   = pexit->u1.to_room            ) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != 0
	&&   pexit_rev->u1.to_room == ch->in_room )
	{
	    CHAR_DATA *rch;

	    SET_BIT( pexit_rev->exit_info, EX_CLOSED );
	    for ( rch = to_room->people; rch != NULL; rch = rch->next_in_room )
		act( "The $d closes.", rch, NULL, pexit_rev->keyword, TO_CHAR );
	}
    }

    return;
}



bool has_key( CHAR_DATA *ch, int key )
{
    OBJ_DATA *obj;

    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
	if ( obj->pIndexData->vnum == key )
	    return TRUE;
    }

    return FALSE;
}



void do_lock( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Lock what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_here( ch, NULL, arg ) ) != NULL )
    {
	/* portal stuff */
	if (obj->item_type == ITEM_PORTAL)
	{
	    if (!IS_SET(obj->value[1],EX_ISDOOR)
	    ||  IS_SET(obj->value[1],EX_NOCLOSE))
	    {
		send_to_char("You can't do that.\n\r",ch);
		return;
	    }
	    if (!IS_SET(obj->value[1],EX_CLOSED))
	    {
		send_to_char("It's not closed.\n\r",ch);
	 	return;
	    }

	    if (obj->value[4] < 0 || IS_SET(obj->value[1],EX_NOLOCK))
	    {
		send_to_char("It can't be locked.\n\r",ch);
		return;
	    }

	    if (!has_key(ch,obj->value[4]))
	    {
		send_to_char("You lack the key.\n\r",ch);
		return;
	    }

	    if (IS_SET(obj->value[1],EX_LOCKED))
	    {
		send_to_char("It's already locked.\n\r",ch);
		return;
	    }

	    SET_BIT(obj->value[1],EX_LOCKED);
	    act("You lock $p.",ch,obj,NULL,TO_CHAR);
	    act("$n locks $p.",ch,obj,NULL,TO_ROOM);
	    return;
	}

	/* 'lock object' */
	if ( obj->item_type != ITEM_CONTAINER )
	    { send_to_char( "That's not a container.\n\r", ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( obj->value[2] < 0 )
	    { send_to_char( "It can't be locked.\n\r",     ch ); return; }
	if ( !has_key( ch, obj->value[2] ) )
	    { send_to_char( "You lack the key.\n\r",       ch ); return; }
	if ( IS_SET(obj->value[1], CONT_LOCKED) )
	    { send_to_char( "It's already locked.\n\r",    ch ); return; }

	SET_BIT(obj->value[1], CONT_LOCKED);
	act("You lock $p.",ch,obj,NULL,TO_CHAR);
	act( "$n locks $p.", ch, obj, NULL, TO_ROOM );
	return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	/* 'lock door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit	= ch->in_room->exit[door];
	if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( pexit->key < 0 )
	    { send_to_char( "It can't be locked.\n\r",     ch ); return; }
	if ( !has_key( ch, pexit->key) )
	    { send_to_char( "You lack the key.\n\r",       ch ); return; }
	if ( IS_SET(pexit->exit_info, EX_LOCKED) )
	    { send_to_char( "It's already locked.\n\r",    ch ); return; }

	SET_BIT(pexit->exit_info, EX_LOCKED);
	send_to_char( "*Click*\n\r", ch );
	act( "$n locks the $d.", ch, NULL, pexit->keyword, TO_ROOM );

	/* lock the other side */
	if ( ( to_room   = pexit->u1.to_room            ) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != 0
	&&   pexit_rev->u1.to_room == ch->in_room )
	{
	    SET_BIT( pexit_rev->exit_info, EX_LOCKED );
	}
    }

    return;
}



void do_unlock( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Unlock what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_here( ch, NULL, arg ) ) != NULL )
    {
 	/* portal stuff */
	if (obj->item_type == ITEM_PORTAL)
	{
	    if (!IS_SET(obj->value[1],EX_ISDOOR))
	    {
		send_to_char("You can't do that.\n\r",ch);
		return;
	    }

	    if (!IS_SET(obj->value[1],EX_CLOSED))
	    {
		send_to_char("It's not closed.\n\r",ch);
		return;
	    }

	    if (obj->value[4] < 0)
	    {
		send_to_char("It can't be unlocked.\n\r",ch);
		return;
	    }

	    if (!has_key(ch,obj->value[4]))
	    {
		send_to_char("You lack the key.\n\r",ch);
		return;
	    }

	    if (!IS_SET(obj->value[1],EX_LOCKED))
	    {
		send_to_char("It's already unlocked.\n\r",ch);
		return;
	    }

	    REMOVE_BIT(obj->value[1],EX_LOCKED);
	    act("You unlock $p.",ch,obj,NULL,TO_CHAR);
	    act("$n unlocks $p.",ch,obj,NULL,TO_ROOM);
	    return;
	}

	/* 'unlock object' */
	if ( obj->item_type != ITEM_CONTAINER )
	    { send_to_char( "That's not a container.\n\r", ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( obj->value[2] < 0 )
	    { send_to_char( "It can't be unlocked.\n\r",   ch ); return; }
	if ( !has_key( ch, obj->value[2] ) )
	    { send_to_char( "You lack the key.\n\r",       ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_LOCKED) )
	    { send_to_char( "It's already unlocked.\n\r",  ch ); return; }

	REMOVE_BIT(obj->value[1], CONT_LOCKED);
	act("You unlock $p.",ch,obj,NULL,TO_CHAR);
	act( "$n unlocks $p.", ch, obj, NULL, TO_ROOM );
	return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	/* 'unlock door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit = ch->in_room->exit[door];
	if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( pexit->key < 0 )
	    { send_to_char( "It can't be unlocked.\n\r",   ch ); return; }
	if ( !has_key( ch, pexit->key) )
	    { send_to_char( "You lack the key.\n\r",       ch ); return; }
	if ( !IS_SET(pexit->exit_info, EX_LOCKED) )
	    { send_to_char( "It's already unlocked.\n\r",  ch ); return; }

	REMOVE_BIT(pexit->exit_info, EX_LOCKED);
	send_to_char( "*Click*\n\r", ch );
	act( "$n unlocks the $d.", ch, NULL, pexit->keyword, TO_ROOM );

	/* unlock the other side */
	if ( ( to_room   = pexit->u1.to_room            ) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
	&&   pexit_rev->u1.to_room == ch->in_room )
	{
	    REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED );
	}
    }

    return;
}



void do_pick( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *gch;
    OBJ_DATA *obj;
    int door;

    if ( ch->riding )
    {
        send_to_char( "You can't do that while mounted.\n\r", ch );
        return;
    }

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Pick what?\n\r", ch );
	return;
    }

    WAIT_STATE( ch, skill_table[gsn_pick_lock].beats );

    /* look for guards */
    for ( gch = ch->in_room->people; gch; gch = gch->next_in_room )
    {
	if ( IS_NPC(gch) && IS_AWAKE(gch) && ch->level + 5 < gch->level )
	{
	    act( "$N is standing too close to the lock.",
		ch, NULL, gch, TO_CHAR );
	    return;
	}
    }

    if ( !IS_NPC(ch) && number_percent( ) > get_skill(ch,gsn_pick_lock))
    {
	send_to_char( "You failed.\n\r", ch);
	check_improve(ch,gsn_pick_lock,FALSE,2);
	return;
    }

    if ( ( obj = get_obj_here( ch, NULL, arg ) ) != NULL )
    {
	/* portal stuff */
	if (obj->item_type == ITEM_PORTAL)
	{
	    if (!IS_SET(obj->value[1],EX_ISDOOR))
	    {	
		send_to_char("You can't do that.\n\r",ch);
		return;
	    }

	    if (!IS_SET(obj->value[1],EX_CLOSED))
	    {
		send_to_char("It's not closed.\n\r",ch);
		return;
	    }

	    if (obj->value[4] < 0)
	    {
		send_to_char("It can't be unlocked.\n\r",ch);
		return;
	    }

	    if (IS_SET(obj->value[1],EX_PICKPROOF))
	    {
		send_to_char("You failed.\n\r",ch);
		return;
	    }

	    REMOVE_BIT(obj->value[1],EX_LOCKED);
	    act("You pick the lock on $p.",ch,obj,NULL,TO_CHAR);
	    act("$n picks the lock on $p.",ch,obj,NULL,TO_ROOM);
	    check_improve(ch,gsn_pick_lock,TRUE,2);
	    return;
	}

	    


	
	/* 'pick object' */
	if ( obj->item_type != ITEM_CONTAINER )
	    { send_to_char( "That's not a container.\n\r", ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( obj->value[2] < 0 )
	    { send_to_char( "It can't be unlocked.\n\r",   ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_LOCKED) )
	    { send_to_char( "It's already unlocked.\n\r",  ch ); return; }
	if ( IS_SET(obj->value[1], CONT_PICKPROOF) )
	    { send_to_char( "You failed.\n\r",             ch ); return; }

	REMOVE_BIT(obj->value[1], CONT_LOCKED);
        act("You pick the lock on $p.",ch,obj,NULL,TO_CHAR);
        act("$n picks the lock on $p.",ch,obj,NULL,TO_ROOM);
	check_improve(ch,gsn_pick_lock,TRUE,2);
	return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	/* 'pick door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit = ch->in_room->exit[door];
	if ( !IS_SET(pexit->exit_info, EX_CLOSED) && !IS_IMMORTAL(ch))
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( pexit->key < 0 && !IS_IMMORTAL(ch))
	    { send_to_char( "It can't be picked.\n\r",     ch ); return; }
	if ( !IS_SET(pexit->exit_info, EX_LOCKED) )
	    { send_to_char( "It's already unlocked.\n\r",  ch ); return; }
	if ( IS_SET(pexit->exit_info, EX_PICKPROOF) && !IS_IMMORTAL(ch))
	    { send_to_char( "You failed.\n\r",             ch ); return; }

	REMOVE_BIT(pexit->exit_info, EX_LOCKED);
	send_to_char( "*Click*\n\r", ch );
	act( "$n picks the $d.", ch, NULL, pexit->keyword, TO_ROOM );
	check_improve(ch,gsn_pick_lock,TRUE,2);

	/* pick the other side */
	if ( ( to_room   = pexit->u1.to_room            ) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
	&&   pexit_rev->u1.to_room == ch->in_room )
	{
	    REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED );
	}
    }

    return;
}




void do_stand( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj = NULL;

    if ( ch->riding )
    {
        send_to_char( "Your mounted, you couldn't possibly do that.\n\r", ch );
        return;
    }

    if (argument[0] != '\0')
    {
	if (ch->position == POS_FIGHTING)
	{
	    send_to_char("Maybe you should finish fighting first?\n\r",ch);
	    return;
	}
	obj = get_obj_list(ch,argument,ch->in_room->contents);
	if (obj == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
	if (obj->item_type != ITEM_FURNITURE
	||  (!IS_SET(obj->value[2],STAND_AT)
	&&   !IS_SET(obj->value[2],STAND_ON)
	&&   !IS_SET(obj->value[2],STAND_IN)))
	{
	    send_to_char("You can't seem to find a place to stand.\n\r",ch);
	    return;
	}
	if (ch->on != obj && count_users(obj) >= obj->value[0])
	{
	    act_new("There's no room to stand on $p.",
		ch,obj,NULL,TO_CHAR,POS_DEAD);
	    return;
	}
 	ch->on = obj;
if ( HAS_TRIGGER_OBJ( obj, TRIG_SIT ) )
	p_percent_trigger( NULL, obj, NULL, ch, NULL, NULL, TRIG_SIT );
    }
    
    switch ( ch->position )
    {
    case POS_SLEEPING:
	if ( IS_AFFECTED(ch, AFF_SLEEP) )
	    { send_to_char( "You can't wake up!\n\r", ch ); return; }

	if ( IS_AFFECTED2(ch, AFF_BLACKJACK) )
	    { send_to_char( "You can't wake up!\n\r", ch ); return; }
	
	if (obj == NULL)
	{
	    send_to_char( "You wake and climb quickly to your feet.\n\r", ch );
	    act( "$n arises from $s slumber.", ch, NULL, NULL, TO_ROOM );
	    ch->on = NULL;
	}
	else if (IS_SET(obj->value[2],STAND_AT))
	{
	   act_new("You wake and stand at $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
	   act("$n wakes and stands at $p.",ch,obj,NULL,TO_ROOM);
	}
	else if (IS_SET(obj->value[2],STAND_ON))
	{
	    act_new("You wake and stand on $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
	    act("$n wakes and stands on $p.",ch,obj,NULL,TO_ROOM);
	}
	else 
	{
	    act_new("You wake and stand in $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
	    act("$n wakes and stands in $p.",ch,obj,NULL,TO_ROOM);
	}
	ch->position = POS_STANDING;
	do_function(ch, &do_look, "auto");

      if ((!IS_NPC(ch))
      && (buf_string(ch->pcdata->buffer)[0] != '\0'))
      {
        send_to_char("\n\r",ch);
        send_to_char("You have missed tells, please type 'replay' to see them.\n\r",ch);
        send_to_char("\n\r",ch);
      }
	break;

    case POS_RESTING:
	if (obj == NULL)
	{
	    send_to_char( "You gather yourself and stand up.\n\r", ch );
	    act( "$n rises from $s rest.", ch, NULL, NULL, TO_ROOM );
	    ch->on = NULL;
	}
	else if (IS_SET(obj->value[2],STAND_AT))
	{
	    act("You stand at $p.",ch,obj,NULL,TO_CHAR);
	    act("$n stands at $p.",ch,obj,NULL,TO_ROOM);
	}
	else if (IS_SET(obj->value[2],STAND_ON))
	{
	    act("You stand on $p.",ch,obj,NULL,TO_CHAR);
	    act("$n stands on $p.",ch,obj,NULL,TO_ROOM);
	}
	else
	{
	    act("You stand in $p.",ch,obj,NULL,TO_CHAR);
	    act("$n stands on $p.",ch,obj,NULL,TO_ROOM);
	}
	ch->position = POS_STANDING;
	break;

    case POS_SITTING:
	if (obj == NULL)
	{
	    send_to_char( "You move quickly to your feet.\n\r", ch );
	    act( "$n rises up.", ch, NULL, NULL, TO_ROOM );
	    ch->on = NULL;
	}
	else if (IS_SET(obj->value[2],STAND_AT))
	{
	    act("You stand at $p.",ch,obj,NULL,TO_CHAR);
	    act("$n stands at $p.",ch,obj,NULL,TO_ROOM);
	}
	else if (IS_SET(obj->value[2],STAND_ON))
	{
	    act("You stand on $p.",ch,obj,NULL,TO_CHAR);
	    act("$n stands on $p.",ch,obj,NULL,TO_ROOM);
	}
	else
	{
	    act("You stand in $p.",ch,obj,NULL,TO_CHAR);
	    act("$n stands on $p.",ch,obj,NULL,TO_ROOM);
	}
	ch->position = POS_STANDING;
	break;

    case POS_STANDING:
	send_to_char( "You are already standing.\n\r", ch );
	break;

    case POS_FIGHTING:
	send_to_char( "You are already fighting!\n\r", ch );
	break;
    }
    check_cpose(ch);
    return;
}



void do_rest( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj = NULL;

    if ( ch->riding )
    {
        send_to_char( "Perhaps you should get your ass off your mount first.\n\r", ch );
        return;
    }

    if (ch->position == POS_FIGHTING)
    {
	send_to_char("Not while your fighting there slim!\n\r",ch);
	return;
    }

    /* okay, now that we know we can rest, find an object to rest on */
    if (argument[0] != '\0')
    {
	obj = get_obj_list(ch,argument,ch->in_room->contents);
	if (obj == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
    }
    else obj = ch->on;

    if (obj != NULL)
    {
        if (obj->item_type != ITEM_FURNITURE
    	||  (!IS_SET(obj->value[2],REST_ON)
    	&&   !IS_SET(obj->value[2],REST_IN)
    	&&   !IS_SET(obj->value[2],REST_AT)))
    	{
	    send_to_char("You can't rest on that.\n\r",ch);
	    return;
    	}

        if (obj != NULL && ch->on != obj && count_users(obj) >= obj->value[0])
        {
	    act_new("There's no more room on $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
	    return;
    	}
	
	ch->on = obj;
if ( HAS_TRIGGER_OBJ( obj, TRIG_SIT ) )
	p_percent_trigger( NULL, obj, NULL, ch, NULL, NULL, TRIG_SIT );
    }

    switch ( ch->position )
    {
    case POS_SLEEPING:
	if (IS_AFFECTED(ch,AFF_SLEEP))
	{
	    send_to_char("You can't wake up!\n\r",ch);
	    return;
	}

	if (IS_AFFECTED2(ch,AFF_BLACKJACK))
	{
	    send_to_char("You can't wake up!\n\r",ch);
	    return;
	}

	if (obj == NULL)
	{
	    send_to_char( "You rouse from your slumber.\n\r", ch );
	    act ("$n rouses from $s slumber.",ch,NULL,NULL,TO_ROOM);
	}
	else if (IS_SET(obj->value[2],REST_AT))
	{
	    act_new("You wake up and rest at $p.",
		    ch,obj,NULL,TO_CHAR,POS_SLEEPING);
	    act("$n wakes up and rests at $p.",ch,obj,NULL,TO_ROOM);
	}
        else if (IS_SET(obj->value[2],REST_ON))
        {
            act_new("You wake up and rest on $p.",
                    ch,obj,NULL,TO_CHAR,POS_SLEEPING);
            act("$n wakes up and rests on $p.",ch,obj,NULL,TO_ROOM);
        }
        else
        {
            act_new("You wake up and rest in $p.",
                    ch,obj,NULL,TO_CHAR,POS_SLEEPING);
            act("$n wakes up and rests in $p.",ch,obj,NULL,TO_ROOM);
        }
	ch->position = POS_RESTING;
	break;

    case POS_RESTING:
	send_to_char( "You are already resting.\n\r", ch );
	break;

    case POS_STANDING:
	if (obj == NULL)
	{
	    send_to_char( "You sprawl out haphazardly.\n\r", ch );
	    act( "$n sprawls out haphazardly.", ch, NULL, NULL, TO_ROOM );
	}
        else if (IS_SET(obj->value[2],REST_AT))
        {
	    act("You sit down at $p and rest.",ch,obj,NULL,TO_CHAR);
	    act("$n sits down at $p and rests.",ch,obj,NULL,TO_ROOM);
        }
        else if (IS_SET(obj->value[2],REST_ON))
        {
	    act("You sit on $p and rest.",ch,obj,NULL,TO_CHAR);
	    act("$n sits on $p and rests.",ch,obj,NULL,TO_ROOM);
        }
        else
        {
	    act("You rest in $p.",ch,obj,NULL,TO_CHAR);
	    act("$n rests in $p.",ch,obj,NULL,TO_ROOM);
        }
	ch->position = POS_RESTING;
	break;

    case POS_SITTING:
	if (obj == NULL)
	{
	    send_to_char("You lie back and sprawl out to rest.\n\r",ch);
	    act("$n lies back and sprawls out to rest.",ch,NULL,NULL,TO_ROOM);
	}
        else if (IS_SET(obj->value[2],REST_AT))
        {
	    act("You rest at $p.",ch,obj,NULL,TO_CHAR);
	    act("$n rests at $p.",ch,obj,NULL,TO_ROOM);
        }
        else if (IS_SET(obj->value[2],REST_ON))
        {
	    act("You rest on $p.",ch,obj,NULL,TO_CHAR);
	    act("$n rests on $p.",ch,obj,NULL,TO_ROOM);
        }
        else
        {
	    act("You rest in $p.",ch,obj,NULL,TO_CHAR);
	    act("$n rests in $p.",ch,obj,NULL,TO_ROOM);
	}
	ch->position = POS_RESTING;
	break;
    }

    check_cpose(ch);
    return;
}


void do_sit (CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj = NULL;

    if ( ch->riding )
    {
        send_to_char( "Perhaps you should get your ass off your mount first.\n\r", ch );
        return;
    }

    if (ch->position == POS_FIGHTING)
    {
	send_to_char("Maybe you should finish this fight first?\n\r",ch);
	return;
    }

    /* okay, now that we know we can sit, find an object to sit on */
    if (argument[0] != '\0')
    {
	obj = get_obj_list(ch,argument,ch->in_room->contents);
	if (obj == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
    }
    else obj = ch->on;

    if (obj != NULL)                                                              
    {
	if (obj->item_type != ITEM_FURNITURE
	||  (!IS_SET(obj->value[2],SIT_ON)
	&&   !IS_SET(obj->value[2],SIT_IN)
	&&   !IS_SET(obj->value[2],SIT_AT)))
	{
	    send_to_char("You can't sit on that.\n\r",ch);
	    return;
	}

	if (obj != NULL && ch->on != obj && count_users(obj) >= obj->value[0])
	{
	    act_new("There's no more room on $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
	    return;
	}

	ch->on = obj;
if ( HAS_TRIGGER_OBJ( obj, TRIG_SIT ) )
	p_percent_trigger( NULL, obj, NULL, ch, NULL, NULL, TRIG_SIT );
    }
    switch (ch->position)
    {
	case POS_SLEEPING:
	    if (IS_AFFECTED(ch,AFF_SLEEP))
	    {
		send_to_char("You can't wake up!\n\r",ch);
		return;
	    }

	    if (IS_AFFECTED2(ch,AFF_BLACKJACK))
	    {
		send_to_char("You can't wake up!\n\r",ch);
		return;
	    }

            if (obj == NULL)
            {
            	send_to_char( "You wake and sit up.\n\r", ch );
            	act( "$n wakes and sits up.", ch, NULL, NULL, TO_ROOM );
            }
            else if (IS_SET(obj->value[2],SIT_AT))
            {
            	act_new("You wake and sit at $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
            	act("$n wakes and sits at $p.",ch,obj,NULL,TO_ROOM);
            }
            else if (IS_SET(obj->value[2],SIT_ON))
            {
            	act_new("You wake and sit on $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
            	act("$n wakes and sits at $p.",ch,obj,NULL,TO_ROOM);
            }
            else
            {
            	act_new("You wake and sit in $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
            	act("$n wakes and sits in $p.",ch,obj,NULL,TO_ROOM);
            }

	    ch->position = POS_SITTING;
	    break;
	case POS_RESTING:
	    if (obj == NULL)
		send_to_char("You stop resting.\n\r",ch);
	    else if (IS_SET(obj->value[2],SIT_AT))
	    {
		act("You sit at $p.",ch,obj,NULL,TO_CHAR);
		act("$n sits at $p.",ch,obj,NULL,TO_ROOM);
	    }

	    else if (IS_SET(obj->value[2],SIT_ON))
	    {
		act("You sit on $p.",ch,obj,NULL,TO_CHAR);
		act("$n sits on $p.",ch,obj,NULL,TO_ROOM);
	    }
	    ch->position = POS_SITTING;
	    break;
	case POS_SITTING:
	    send_to_char("You are already sitting down.\n\r",ch);
	    break;
	case POS_STANDING:
	    if (obj == NULL)
    	    {
		send_to_char("You sit down.\n\r",ch);
    	        act("$n sits down on the ground.",ch,NULL,NULL,TO_ROOM);
	    }
	    else if (IS_SET(obj->value[2],SIT_AT))
	    {
		act("You sit down at $p.",ch,obj,NULL,TO_CHAR);
		act("$n sits down at $p.",ch,obj,NULL,TO_ROOM);
	    }
	    else if (IS_SET(obj->value[2],SIT_ON))
	    {
		act("You sit on $p.",ch,obj,NULL,TO_CHAR);
		act("$n sits on $p.",ch,obj,NULL,TO_ROOM);
	    }
	    else
	    {
		act("You sit down in $p.",ch,obj,NULL,TO_CHAR);
		act("$n sits down in $p.",ch,obj,NULL,TO_ROOM);
	    }
    	    ch->position = POS_SITTING;
    	    break;
    }
    check_cpose(ch);
    return;
}


void do_sleep( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj = NULL;

    if ( ch->riding )
    {
        send_to_char( "Perhaps you should get your ass off your mount first.\n\r", ch );
        return;
    }

    if( IS_AFFECTED2(ch,AFF_SLEEPCURSE) )
    {
	  send_to_char("A sleeping curse wracks your mind, preventing you from sleeping.\n\r",ch);
	  return;
    }

    switch ( ch->position )
    {
    case POS_SLEEPING:
	send_to_char( "You are already sleeping.\n\r", ch );
	break;

    case POS_RESTING:
	if (argument[0] == '\0' && ch->on == NULL)
	{
	send_to_char( "You close your eyes and drift into slumber.\n\r", ch );
	act( "$n closes $s eyes and drifts into a deep slumber.", ch, NULL, NULL, TO_ROOM );
	ch->position = POS_SLEEPING;
	}
	else  /* find an object and sleep on it */
	{
	    if (argument[0] == '\0')
		obj = ch->on;
	    else
	    	obj = get_obj_list( ch, argument,  ch->in_room->contents );

	    if (obj == NULL)
	    {
		send_to_char("You don't see that here.\n\r",ch);
		return;
	    }
	    if (obj->item_type != ITEM_FURNITURE
	    ||  (!IS_SET(obj->value[2],SLEEP_ON) 
	    &&   !IS_SET(obj->value[2],SLEEP_IN)
	    &&	 !IS_SET(obj->value[2],SLEEP_AT)))
	    {
		send_to_char("You can't sleep on that!\n\r",ch);
		return;
	    }

	    if (ch->on != obj && count_users(obj) >= obj->value[0])
	    {
		act_new("There is no room on $p for you.",
		    ch,obj,NULL,TO_CHAR,POS_DEAD);
		return;
	    }

	    ch->on = obj;
if ( HAS_TRIGGER_OBJ( obj, TRIG_SIT ) )
	p_percent_trigger( NULL, obj, NULL, ch, NULL, NULL, TRIG_SIT );
	    if (IS_SET(obj->value[2],SLEEP_AT))
	    {
		act("You go to sleep at $p.",ch,obj,NULL,TO_CHAR);
		act("$n goes to sleep at $p.",ch,obj,NULL,TO_ROOM);
	    }
	    else if (IS_SET(obj->value[2],SLEEP_ON))
	    {
	        act("You go to sleep on $p.",ch,obj,NULL,TO_CHAR);
	        act("$n goes to sleep on $p.",ch,obj,NULL,TO_ROOM);
	    }
	    else
	    {
		act("You go to sleep in $p.",ch,obj,NULL,TO_CHAR);
		act("$n goes to sleep in $p.",ch,obj,NULL,TO_ROOM);
	    }
	    ch->position = POS_SLEEPING;
	}
	break;

    case POS_SITTING:
	if (argument[0] == '\0' && ch->on == NULL)
	{
	send_to_char( "You slump over and fall dead asleep.\n\r", ch );
	act( "$n nods off and slowly slumps over, dead asleep.", ch, NULL, NULL, TO_ROOM );
	ch->position = POS_SLEEPING;
	}
	else  /* find an object and sleep on it */
	{
	    if (argument[0] == '\0')
		obj = ch->on;
	    else
	    	obj = get_obj_list( ch, argument,  ch->in_room->contents );

	    if (obj == NULL)
	    {
		send_to_char("You don't see that here.\n\r",ch);
		return;
	    }
	    if (obj->item_type != ITEM_FURNITURE
	    ||  (!IS_SET(obj->value[2],SLEEP_ON) 
	    &&   !IS_SET(obj->value[2],SLEEP_IN)
	    &&	 !IS_SET(obj->value[2],SLEEP_AT)))
	    {
		send_to_char("You can't sleep on that!\n\r",ch);
		return;
	    }

	    if (ch->on != obj && count_users(obj) >= obj->value[0])
	    {
		act_new("There is no room on $p for you.",
		    ch,obj,NULL,TO_CHAR,POS_DEAD);
		return;
	    }

	    ch->on = obj;
if ( HAS_TRIGGER_OBJ( obj, TRIG_SIT ) )
	p_percent_trigger( NULL, obj, NULL, ch, NULL, NULL, TRIG_SIT );
	    if (IS_SET(obj->value[2],SLEEP_AT))
	    {
		act("You go to sleep at $p.",ch,obj,NULL,TO_CHAR);
		act("$n goes to sleep at $p.",ch,obj,NULL,TO_ROOM);
	    }
	    else if (IS_SET(obj->value[2],SLEEP_ON))
	    {
	        act("You go to sleep on $p.",ch,obj,NULL,TO_CHAR);
	        act("$n goes to sleep on $p.",ch,obj,NULL,TO_ROOM);
	    }
	    else
	    {
		act("You go to sleep in $p.",ch,obj,NULL,TO_CHAR);
		act("$n goes to sleep in $p.",ch,obj,NULL,TO_ROOM);
	    }
	    ch->position = POS_SLEEPING;
	}
	break;

    case POS_STANDING: 
	if (argument[0] == '\0' && ch->on == NULL)
	{
	    send_to_char( "You go to sleep.\n\r", ch );
	    act( "$n goes to sleep.", ch, NULL, NULL, TO_ROOM );
	    ch->position = POS_SLEEPING;
	}
	else  /* find an object and sleep on it */
	{
	    if (argument[0] == '\0')
		obj = ch->on;
	    else
	    	obj = get_obj_list( ch, argument,  ch->in_room->contents );

	    if (obj == NULL)
	    {
		send_to_char("You don't see that here.\n\r",ch);
		return;
	    }
	    if (obj->item_type != ITEM_FURNITURE
	    ||  (!IS_SET(obj->value[2],SLEEP_ON) 
	    &&   !IS_SET(obj->value[2],SLEEP_IN)
	    &&	 !IS_SET(obj->value[2],SLEEP_AT)))
	    {
		send_to_char("You can't sleep on that!\n\r",ch);
		return;
	    }

	    if (ch->on != obj && count_users(obj) >= obj->value[0])
	    {
		act_new("There is no room on $p for you.",
		    ch,obj,NULL,TO_CHAR,POS_DEAD);
		return;
	    }

	    ch->on = obj;
if ( HAS_TRIGGER_OBJ( obj, TRIG_SIT ) )
	p_percent_trigger( NULL, obj, NULL, ch, NULL, NULL, TRIG_SIT );
	    if (IS_SET(obj->value[2],SLEEP_AT))
	    {
		act("You go to sleep at $p.",ch,obj,NULL,TO_CHAR);
		act("$n goes to sleep at $p.",ch,obj,NULL,TO_ROOM);
	    }
	    else if (IS_SET(obj->value[2],SLEEP_ON))
	    {
	        act("You go to sleep on $p.",ch,obj,NULL,TO_CHAR);
	        act("$n goes to sleep on $p.",ch,obj,NULL,TO_ROOM);
	    }
	    else
	    {
		act("You go to sleep in $p.",ch,obj,NULL,TO_CHAR);
		act("$n goes to sleep in $p.",ch,obj,NULL,TO_ROOM);
	    }
	    ch->position = POS_SLEEPING;
	}
	break;

    case POS_FIGHTING:
	send_to_char( "You are already fighting!\n\r", ch );
	break;
    }
    check_cpose(ch);
    return;
}



void do_wake( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
	{ do_function(ch, &do_stand, ""); return; }

    if ( !IS_AWAKE(ch) )
	{ send_to_char( "You are asleep yourself!\n\r",       ch ); return; }

    if ( ( victim = get_char_room( ch, NULL, arg ) ) == NULL )
	{ send_to_char( "They aren't here.\n\r",              ch ); return; }

    if ( IS_AWAKE(victim) )
	{ act( "$N is already awake.", ch, NULL, victim, TO_CHAR ); return; }

    if ( IS_AFFECTED(victim, AFF_SLEEP) )
	{ act( "You can't wake $M!",   ch, NULL, victim, TO_CHAR );  return; }

    if ( IS_AFFECTED2(victim, AFF_BLACKJACK) )
	{ act( "You can't wake $M!",   ch, NULL, victim, TO_CHAR );  return; }

    act_new( "$n wakes you.", ch, NULL, victim, TO_VICT,POS_SLEEPING );
    do_function(victim, &do_stand, "");
    check_cpose(ch);
    return;
}



void do_sneak( CHAR_DATA *ch, char *argument )
{
    AFFECT_DATA af;

   if ( ch->riding )
   {
     send_to_char( "You can't do that while mounted.\n\r", ch );
     return;
   }

   if ((ch->size > SIZE_MEDIUM)
   && !IS_AFFECTED2(ch, AFF_WRAITHFORM))
   {
     send_to_char("You are too big to sneak.",ch);
     return;
   }

   if (is_affected(ch, gsn_sneak))
   {
	send_to_char( "You attempt to move silently.\n\r", ch );
	WAIT_STATE(ch,skill_table[gsn_sneak].beats);  
	return;
   }

    send_to_char( "You attempt to move silently.\n\r", ch );
    affect_strip( ch, gsn_sneak );
    WAIT_STATE(ch,skill_table[gsn_sneak].beats);
    if ( number_percent( ) < get_skill(ch,gsn_sneak))
    {
	check_improve(ch,gsn_sneak,TRUE,3);
	af.where     = TO_AFFECTS;
	af.type      = gsn_sneak;
	af.level     = ch->level; 
	af.duration  = ch->level;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = AFF_SNEAK;
	affect_to_char( ch, &af );
    }
    else
	check_improve(ch,gsn_sneak,FALSE,3);

    return;
}

/*
 * Quiet movement by Mortimor
 */
bool check_quiet_movement( CHAR_DATA *ch)
{
    ROOM_INDEX_DATA  *in_room; 

    in_room = ch->in_room;

    switch(in_room->sector_type)
    {
    case SECT_FIELD:
    case SECT_DESERT:
    case SECT_POLAR:
    case SECT_FOREST:
    case SECT_HILLS:
    case SECT_MOUNTAIN:
    case SECT_SWAMP:
      if ( (IS_NPC(ch)
      || number_percent( ) < ch->pcdata->learned[gsn_quiet_movement]) )
     	{	
    	    check_improve(ch, gsn_quiet_movement, TRUE, 3);
    	    return TRUE;
    	}
	else
	    return FALSE;
    break;
    default:
    	return FALSE;
    break;
    }
}

void do_hide( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;  
    int iWear; 
    bool found = FALSE; 
    int Light_Num = 0;

    if ( ch->riding )
    {
        send_to_char( "You can't do that while mounted.\n\r", ch );
        return;
    }

    for ( iWear = 0; (iWear < MAX_WEAR && !found); iWear ++)
    {
      if ((obj = get_eq_char(ch,iWear)) == NULL)
        continue;

      if ((IS_OBJ_STAT(obj,ITEM_GLOW))      
      && (obj->wear_loc != WEAR_MARK))
         Light_Num++;

      if ((Light_Num >= 3) 
      || (obj->item_type == ITEM_LIGHT)) 
      {
        send_to_char("You are producing too much light to hide in the shadows.\n\r",ch); 
        return;
      }
    }

    if  ( ch->in_room->sector_type == SECT_WATER_SWIM
    ||    ch->in_room->sector_type == SECT_WATER_NOSWIM
    ||    ch->in_room->sector_type == SECT_UNDERWATER)
    {
          send_to_char("You cannot hide here.\n\r",ch);
          return;
    }

    if (ch->size > SIZE_MEDIUM)
    {
        send_to_char("You are too big to hide in the shadows.\n\r",ch);
        return;
    }

    if (ch->fighting != NULL)
    {
      send_to_char("Your in the middle of a fight, you couldn't possibly hide anywhere.\n\r",ch);
      return;
    }

    send_to_char( "You attempt to hide.\n\r", ch );

    if ( IS_AFFECTED(ch, AFF_HIDE) )
    REMOVE_BIT(ch->affected_by, AFF_HIDE);
    if ( number_percent( ) < get_skill(ch,gsn_hide) )
    {
        SET_BIT( ch->affected_by, AFF_HIDE );
	check_improve(ch,gsn_hide,TRUE,3);
    }
    else
	check_improve(ch,gsn_hide,FALSE,3);

    return;
}

void do_visible( CHAR_DATA *ch, char *argument )
{
  if(IS_AFFECTED2 (ch, AFF_WRAITHFORM) )
  {
    act("$n looks solid and heavy again as the wraithlike form diminishes.", ch, NULL, NULL, TO_ROOM );
    send_to_char("You feel solid and heavy again as your wraithlike form diminishes.\n", ch);
    affect_strip (ch, gsn_wraithform);
    REMOVE_BIT (ch->affected2_by, AFF_WRAITHFORM);
    REMOVE_BIT (ch->affected2_by, AFF_WRAITHFORM);
  }

  if ( IS_AFFECTED(ch, AFF_HIDE) )
  {
    REMOVE_BIT( ch->affected_by, AFF_HIDE );
    act( "$n steps out of the shadows, making $mself known.", ch, NULL, NULL, TO_ROOM );
    act( "You step out of the shadows and make yourself known.", ch, NULL, NULL, TO_CHAR );
  }

  if ( IS_AFFECTED2(ch, AFF_CAMOUFLAGE) )
  {
    REMOVE_BIT( ch->affected2_by, AFF_CAMOUFLAGE );
    act( "$n steps out of $s camouflaged position, making $mself known.", ch, NULL, NULL, TO_ROOM );
    act( "You step out of your camouflaged position, making yourself known to everyone around you.", ch, NULL, NULL, TO_CHAR );
  }

  if ( IS_AFFECTED(ch, AFF_INVISIBLE) )
  {
    affect_strip( ch, gsn_invis );
    affect_strip( ch, gsn_mass_invis );
    REMOVE_BIT( ch->affected_by, AFF_INVISIBLE );
    act( "$n fades into existence.", ch, NULL, NULL, TO_ROOM );
    act( "You fade back into existence, making yourself visible to everyone around you.", ch, NULL, NULL, TO_CHAR );
  }

  affect_strip ( ch, gsn_sneak				);
  REMOVE_BIT   ( ch->affected_by, AFF_SNEAK		);
  return;
}



void do_recall( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    ROOM_INDEX_DATA *location;
    int recl_room;

    if (IS_NPC(ch) && !IS_SET(ch->act,ACT_PET))
    {
	send_to_char("Only players can recall.\n\r",ch);
	return;
    }

    if (ch->level > 10 && ch->religion_id == RELIGION_NONE)
    { 
      send_to_char("You have no God to pray to for transportation!\n\r", ch);
      return;
    } 

    if (IS_AFFECTED (ch, AFF_SHACKLES))
    {
	send_to_char("You are chained to the floor, and can not be sent home!\n\r",ch);
	return;
    }

    /* see if post fight timer is still running */
    if( ch->fight_end_timer )
    {
        send_to_char( "You are too pumped to pray.\n\r", ch );
        return;
    }
 
    if (ch->org_id != ORG_NONE )
    {
      argument = one_argument( argument, arg1 );
      
      if ( arg1[0] == '\0')
      {
	  send_to_char( "Syntax: recall hometown\n\r", ch );
	  send_to_char( "               clanhall\n\r", ch );
	  return;
      }
      if ( !str_cmp( arg1, "hometown" ) )
      {
	  recl_room = ch->recall_point;
	}
      else
      if ( !str_cmp( arg1, "clanhall" ) )
      {
        switch ( ch->org_id )
        {
          case ORG_UNDEAD:
            recl_room = 14005;
          break;
          case 267:
            recl_room = 14005;
          break;
          case ORG_THIEVES:
            recl_room = 1926;
          break;
          case 257:
            recl_room = 1926;
          break;
          case ORG_CONCLAVE:
            recl_room = 15195;
          break;
	    case 258:
            recl_room = 15195;
          break;
          case ORG_SOLAMNIC:
            recl_room = 13700;
          break;
          case 259:
            recl_room = 13700;
          break;
          case ORG_KOT:
            recl_room = 11608;
          break;
          case 260:
            recl_room = 11608;
          break;
          case ORG_HOLYORDER:
            recl_room = 851;
          break;
          case 261:
            recl_room = 851;
          break;
          case ORG_MERCENARY:
            recl_room = 13201;
          break;
          case 262:
            recl_room = 13201;
          break;
          case ORG_ARTISANS:
            recl_room = 401;
          break;
          case 263:
            recl_room = 401;
          break;
          case ORG_FORESTER:
            recl_room = 504;
          break;
          case 264:
            recl_room = 504;
          break;
          case ORG_AESTHETIC:
            recl_room = 2906;
          break;
          case 265:
            recl_room = 2906;
          break;
          case ORG_BLACKORDER:
            recl_room = 152;
          break;
          case 266:
            recl_room = 152;
          break;
	    default: 
	      recl_room = ch->recall_point;
	    break;
        }
	}
      else
      {
	  send_to_char( "Syntax: recall hometown\n\r", ch );
	  send_to_char( "               clanhall\n\r", ch );
	  return;
	}
    }
    else
    {
      recl_room = ch->recall_point;
    }

    act( "$n prays for transportation!", ch, 0, 0, TO_ROOM );

    if ( ( location = get_room_index(  recl_room ) ) == NULL )
    {
        send_to_char( "You are completely lost.\n\r", ch );
        return;
    }

    if ( ch->in_room == location || ch->in_room == NULL )
        return;

    if ( IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
    ||   IS_AFFECTED(ch, AFF_CURSE))
    {
	send_to_char( "You have been forsaken.\n\r", ch );
	return;
    }

    if ( ( victim = ch->fighting ) != NULL )
    {
	int lose,skill;

	skill = get_skill(ch,gsn_recall);

	if ( number_percent() < 80 * skill / 100 )
	{
	    check_improve(ch,gsn_recall,FALSE,6);
	    WAIT_STATE( ch, 4 );
	    sprintf( buf, "You failed!.\n\r");
	    send_to_char( buf, ch );
	    return;
	}

	if (IS_NPC(victim))
	{
	  lose = (ch->desc != NULL) ? 25 : 50;
	  gain_exp( ch, 0 - lose );
	  check_improve(ch,gsn_recall,TRUE,4);
	  sprintf( buf, "You recall from combat!  You lose %d exps.\n\r", lose );
	  send_to_char( buf, ch );
	  stop_fighting( ch, TRUE );
	}
	else
	{
	  lose = (ch->desc != NULL) ? 1000 : 5000;
	  gain_exp( ch, 0 - lose );
	  check_improve(ch,gsn_recall,TRUE,4);
	  sprintf( buf, "You recall from combat!  You lose %d exps.\n\r", lose );
	  send_to_char( buf, ch );
	  stop_fighting( ch, TRUE );
	}
	
    }

    ch->move /= 2;
    act( "$n disappears.", ch, NULL, NULL, TO_ROOM );
    char_from_room( ch );
    char_to_room( ch, location );
    act( "$n appears in the room.", ch, NULL, NULL, TO_ROOM );
    do_function(ch, &do_look, "auto" );

    if (ch->pet != NULL)
    {
      char_from_room( ch->pet );
      char_to_room( ch->pet, location );
    }

    check_cpose(ch);
    return;
}



void do_train( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *mob;
    sh_int stat = - 1;
    char *pOutput = NULL;
    int cost;

    if ( IS_NPC(ch) )
	return;

    /*
     * Check for trainer.
     */
    for ( mob = ch->in_room->people; mob; mob = mob->next_in_room )
    {
	if ( IS_NPC(mob) && IS_SET(mob->act, ACT_TRAIN) )
	    break;
    }

    if ( mob == NULL )
    {
	send_to_char( "You can't do that here.\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	sprintf( buf, "You have %d training sessions.\n\r", ch->train );
	send_to_char( buf, ch );
	argument = "foo";
    }

    cost = 1;

    if ( !str_cmp( argument, "str" ) )
    {
	if ( class_table[ch->class].attr_prime == STAT_STR )
	    cost    = 1;
	stat        = STAT_STR;
	pOutput     = "strength";
    }

    else if ( !str_cmp( argument, "int" ) )
    {
	if ( class_table[ch->class].attr_prime == STAT_INT )
	    cost    = 1;
	stat	    = STAT_INT;
	pOutput     = "intelligence";
    }

    else if ( !str_cmp( argument, "wis" ) )
    {
	if ( class_table[ch->class].attr_prime == STAT_WIS )
	    cost    = 1;
	stat	    = STAT_WIS;
	pOutput     = "wisdom";
    }

    else if ( !str_cmp( argument, "dex" ) )
    {
	if ( class_table[ch->class].attr_prime == STAT_DEX )
	    cost    = 1;
	stat  	    = STAT_DEX;
	pOutput     = "dexterity";
    }

    else if ( !str_cmp( argument, "con" ) )
    {
	if ( class_table[ch->class].attr_prime == STAT_CON )
	    cost    = 1;
	stat	      = STAT_CON;
	pOutput     = "constitution";
    }

    else if ( !str_cmp( argument, "chr" ) )
    {
        if ( class_table[ch->class].attr_prime == STAT_CHR )
          cost        = 1;
          stat        = STAT_CHR;
          pOutput     = "charisma";
    }

    else if ( !str_cmp(argument, "hp" ) )
	cost = 1;

    else if ( !str_cmp(argument, "mana" ) )
	cost = 1;

    else
    {
	strcpy( buf, "You can train:" );
	if ( ch->perm_stat[STAT_STR] < get_max_train(ch,STAT_STR)) 
	    strcat( buf, " str" );
	if ( ch->perm_stat[STAT_INT] < get_max_train(ch,STAT_INT))  
	    strcat( buf, " int" );
	if ( ch->perm_stat[STAT_WIS] < get_max_train(ch,STAT_WIS)) 
	    strcat( buf, " wis" );
	if ( ch->perm_stat[STAT_DEX] < get_max_train(ch,STAT_DEX))  
	    strcat( buf, " dex" );
	if ( ch->perm_stat[STAT_CON] < get_max_train(ch,STAT_CON))  
	    strcat( buf, " con" );
      if ( ch->perm_stat[STAT_CHR] < get_max_train(ch,STAT_CHR))  
          strcat( buf, " chr" );
	strcat( buf, " hp mana");

	if ( buf[strlen(buf)-1] != ':' )
	{
	    strcat( buf, ".\n\r" );
	    send_to_char( buf, ch );
	}
	else
	{
	    /*
	     * This message dedicated to Jordan ... you big stud!
	     */
	    act( "You have nothing left to train, you $T!",
		ch, NULL,
		ch->sex == SEX_MALE   ? "big stud" :
		ch->sex == SEX_FEMALE ? "hot babe" :
					"wild thing",
		TO_CHAR );
	}

	return;
    }

    if (!str_cmp("hp",argument))
    {
    	if ( cost > ch->train )
    	{
       	    send_to_char( "You don't have enough training sessions.\n\r", ch );
            return;
        }
 
	ch->train -= cost;
        ch->pcdata->perm_hit += 10;
        ch->max_hit += 10;
        ch->hit +=10;
        act( "Your durability increases!",ch,NULL,NULL,TO_CHAR);
        act( "$n's durability increases!",ch,NULL,NULL,TO_ROOM);
        return;
    }
 
    if (!str_cmp("mana",argument))
    {
        if ( cost > ch->train )
        {
            send_to_char( "You don't have enough training sessions.\n\r", ch );
            return;
        }

	ch->train -= cost;
        ch->pcdata->perm_mana += 10;
        ch->max_mana += 10;
        ch->mana += 10;
        act( "Your power increases!",ch,NULL,NULL,TO_CHAR);
        act( "$n's power increases!",ch,NULL,NULL,TO_ROOM);
        return;
    }

    if ( ch->perm_stat[stat]  >= get_max_train(ch,stat) )
    {
	act( "Your $T is already at maximum.", ch, NULL, pOutput, TO_CHAR );
	return;
    }

    if ( cost > ch->train )
    {
	send_to_char( "You don't have enough training sessions.\n\r", ch );
	return;
    }

    ch->train		-= cost;
  
    ch->perm_stat[stat]		+= 1;
    act( "Your $T increases!", ch, NULL, pOutput, TO_CHAR );
    act( "$n's $T increases!", ch, NULL, pOutput, TO_ROOM );
    return;
}

void do_camouflage ( CHAR_DATA *ch, char *argument )
{

    if ( ch->riding )
    {
        send_to_char( "You can't do that while mounted.\n\r", ch );
        return;
    }

    if (ch->fighting != NULL)
    {
      send_to_char("Your in the middle of a fight, you couldn't possibly camoflauge yourself.\n\r",ch);
      return;
    }

    if ( (ch->in_room->sector_type != SECT_FOREST)
    &&   (ch->in_room->sector_type != SECT_MOUNTAIN)
    &&   (ch->in_room->sector_type != SECT_SWAMP)
    &&   (ch->in_room->sector_type != SECT_HILLS))
    {
        send_to_char( "You can not blend into these surroundings.\n\r", ch );
        return;
    }

    send_to_char( "You attempt to blend in with your surroundings.\n\r", ch );

    if ( IS_AFFECTED2( ch, AFF_CAMOUFLAGE ) )
        REMOVE_BIT( ch->affected2_by, AFF_CAMOUFLAGE );

    if ( number_percent( ) < get_skill(ch,gsn_camouflage) )
    {
        SET_BIT( ch->affected2_by, AFF_CAMOUFLAGE );
	check_improve(ch,gsn_camouflage,TRUE,3);
    }
    else
	check_improve(ch,gsn_camouflage,FALSE,3);

    return;
}

//C053
ROOM_INDEX_DATA *get_home(CHAR_DATA *ch)
{
    ROOM_INDEX_DATA *location=NULL;
    char buf[MSL];

    if (!IS_NPC(ch)) //C064
    {

    if ( ( location = get_room_index( hometown_table[ch->hometown].recall )) == NULL )
        {
            sprintf(buf,"No temple!");
            bug(buf,0);
            send_to_char( "You are completely lost.\n\r", ch );
        }
    }
    
    if (location==NULL)
    {
    if ( ( location = get_room_index( hometown_table[ch->hometown].recall )) == NULL )
        {
            sprintf(buf,"No temple!");
            bug(buf,0);
            send_to_char( "You are completely lost.\n\r", ch );
        }
    }

    return location;
}


void do_glide( CHAR_DATA *ch, char *argument )
{

    if ( ch->race != race_lookup("draconian"))
     {
     send_to_char("You look pretty stupid flapping your arms about like a chicken.\n\r",ch);
     return;
     }

else

    if ( IS_AFFECTED2(ch, AFF_GLIDE) )
    {
     send_to_char("You're already airborne.\n\r",ch);
     return;
    }

else

    SET_BIT(ch->affected2_by, AFF_GLIDE);
    send_to_char( "You unfold your wings and with a strong lunge you glide up high into the air.\n\r", ch );
     act( "$n unfolds $s massive wings and glides high into the air.", ch, NULL, NULL, TO_ROOM );

    if ( number_percent( ) < get_skill(ch,gsn_hide))
    {
	SET_BIT(ch->affected2_by, AFF_GLIDE);
	check_improve(ch,gsn_glide,TRUE,3);
    }
    else
	check_improve(ch,gsn_glide,FALSE,3);

    return;
}

void do_land( CHAR_DATA *ch, char *argument )
{

    if ( ch->race != race_lookup("draconian"))
     {
     send_to_char("Your standing on the ground, how could you possibly land?\n\r",ch);
     return;
     }

else

      if (!IS_AFFECTED2(ch,AFF_GLIDE))
      {
         send_to_char("Your wings are still tucked in.\n\r",ch);
         return;
      }

    else

        {
        REMOVE_BIT( ch->affected2_by, AFF_GLIDE );
        affect_strip(ch,0);
        send_to_char( "You gently land on the ground and fold your wings in.\n\r", ch );
     act( "$n folds $s massive wings and lands gently on the ground.", ch, NULL, NULL, TO_ROOM );
        }
    return;
}

void do_mount( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;

    if (!IS_NPC( ch ) 
    && ch->level < skill_table[gsn_mount].skill_level[ch->class] )
    {
	send_to_char( "I don't think that would be a good idea...\n\r", ch );
	return;
    }

    if (!IS_NPC( ch ) && ch->pcdata->condition[COND_DRUNK] > 10)
    {
	send_to_char( "Your too drunk to drive right now!\n\r", ch );
	return;
    }

    if ( ch->riding )
    {
	send_to_char( "You're already mounted!\n\r", ch );
	return;
    }

    if ( !( victim = get_char_room( ch, NULL, argument ) ) )
    {
	send_to_char( "You can't find that here.\n\r", ch );
	return;
    }

    if (ch->fight_pos != victim->fight_pos)
    {
	send_to_char( "You need to be in the same rank position as your mount!\n\r", ch );
	return;
    }

    if ( !IS_NPC( victim )
    || !IS_SET( victim->act, ACT_MOUNTABLE ) )
    {
	send_to_char( "You can't mount that!\n\r", ch );
	return;
    }

    if (victim->leader != ch)
    {
      send_to_char("That mount does not belong to you!\n\r",ch);
      return;
    }

    if ( ch->rider )
    {
        send_to_char( "You are being ridden by someone else!\n\r", ch );
        return;
    }

    if ( victim->rider )
    {
	send_to_char( "That mount already has a rider.\n\r", ch );
	return;
    }

    if ( victim->position < POS_STANDING )
    {
	send_to_char( "Your mount must be standing.\n\r", ch );
	return;
    }

    if ( victim->position == POS_FIGHTING
    || victim->fighting )
    {
	send_to_char( "Your mount is moving around too much.\n\r", ch );
	return;
    }

    if (is_affected(ch,gsn_treeform))    
    {   
      send_to_char("You couldn't possibly mount anything while in the form of a giant oak tree.\n\r",ch);
      return;
    }

    if (IS_AFFECTED3(ch,AFF_QUICKSAND))    
    {   
      send_to_char("You couldn't possibly mount anything while waist deep in quicksand.\n\r",ch);
	return;
    }

    check_cpose(ch);

    WAIT_STATE( ch, skill_table[gsn_mount].beats );

    if ( IS_NPC( ch )
    || number_percent( ) < ch->pcdata->learned[gsn_mount] )
    {
	victim->rider = ch;
	ch->riding    = victim;

	if(victim->tethered)
	{
	  victim->tethered = FALSE;
	  act( "You untether and mount $N.", ch, NULL, victim, TO_CHAR );
	  act( "$n untethers and skillfully mounts $N.", ch, NULL, victim, TO_NOTVICT );
	  act( "$n untethers and mounts you.", ch, NULL, victim, TO_VICT );
	}
	else
	{
	  act( "You mount $N.", ch, NULL, victim, TO_CHAR );
	  act( "$n skillfully mounts $N.", ch, NULL, victim, TO_NOTVICT );
	  act( "$n mounts you.", ch, NULL, victim, TO_VICT );
	}

	check_improve( ch, gsn_mount, TRUE, 1 );
    }
    else
    {
	act( "You unsuccessfully try to mount $N.", ch, NULL, victim, TO_CHAR );
	act( "$n unsuccessfully attempts to mount $N.", ch, NULL, victim, TO_NOTVICT );
	act( "$n tries to mount you.", ch, NULL, victim, TO_VICT );
	check_improve( ch, gsn_mount, FALSE, 1 );
    }

    return;
}


void do_dismount( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;

    if ( !( victim = ch->riding ) )
    {
	send_to_char( "You're not mounted.\n\r", ch );
	return;	
    }

    victim->tethered = FALSE;
    WAIT_STATE( ch, skill_table[gsn_mount].beats );
    if ( IS_NPC( ch )
	|| number_percent( ) < ch->pcdata->learned[gsn_mount] )
    {
	act( "You dismount $N.",            ch, NULL, victim, TO_CHAR );
	act( "$n skillfully dismounts $N.", ch, NULL, victim, TO_NOTVICT );
	act( "$n dismounts you.  Whew!",    ch, NULL, victim, TO_VICT );
	victim->rider = NULL;
	ch->riding    = NULL;
	ch->position  = POS_STANDING;
    }
    else
    {
	act( "You fall off while dismounting $N.  Ouch!", ch, NULL, victim, TO_CHAR );
	act( "$n falls off of $N while dismounting.", ch, NULL, victim, TO_NOTVICT );
	act( "$n falls off your back.", ch, NULL, victim, TO_VICT );
	victim->rider = NULL;
	ch->riding    = NULL;
	ch->position  = POS_RESTING;
    }
    check_cpose(ch);
    return;

}

void do_mountjack( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    CHAR_DATA *jacked;

    if (    !IS_NPC( ch ) 
    && ch->level < skill_table[gsn_mountjack].skill_level[ch->class] )
    {
	send_to_char( "I don't think that would be a good idea...\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
        send_to_char( "Use: mountjack <person who is mounted>.\n\r", ch );
        return;
    }

    if ( ch->move < 200 )
    {
        send_to_char( "You need 200 movement to do that.\n\r", ch );
        return;
    }

    if ( ch->riding )
    {
        send_to_char( "You're already mounted!\n\r", ch );
        return;
    }

    if ( ch->rider )
    {
        send_to_char( "You are being ridden by someone else!\n\r", ch );
        return;
    }

    if ( !( victim = get_char_room( ch, NULL, argument ) ) )
    {
        send_to_char( "They're not here.\n\r", ch );
        return;
    }

    if ( !victim->riding )
    {
        send_to_char( "They're not mounted...\n\r", ch );
        return;
    }

    if ( victim->position == POS_FIGHTING
        || victim->fighting )
    {
        send_to_char( "Wait until they're done fighting.\n\r", ch );
        return;
    }

    if ( number_percent( ) <= 50 )
    {
        ch->move -= 200;
        jacked = victim->riding;
        victim->riding = NULL;
        victim->master = NULL;
        victim->leader = NULL;
        victim->pet = NULL;
	  jacked->leader = NULL;  /* removed it from original owners group */
        jacked->master = NULL;  /* makes the mob stop following the orig owner */
	  jacked->leader = ch;    /* added it to thiefs group             */
        ch->pet = jacked;	  /* makes it the thiefs pet(but not follow) */
        jacked->master = ch;    /* makes the mounted mob go with thief   */
        ch->riding = jacked;

        act( "You steal $N's mount!", ch, NULL, victim, TO_CHAR );
        act( "$n has jacked your mount!", ch, NULL, victim, TO_VICT );
        return;
    }

    act( "You tried to steal $N's mount but failed.", ch, NULL, victim, TO_CHAR );
    act( "$n has tried to jack your mount!", ch, NULL, victim, TO_VICT );
    return;
}

void do_unmask( CHAR_DATA *ch, char *argument )
{
  if (is_affected(ch,gsn_doppelganger))    
  {   
    affect_strip(ch,gsn_doppelganger);
    send_to_char("You unmask yourself, returning to your natural look.\n\r",ch);
  }

  if (is_affected(ch,gsn_treeform))    
  {   
    affect_strip(ch,gsn_treeform);
    send_to_char("You morph back into your natural look.\n\r",ch);
  }

  return;
}

void do_disguise( CHAR_DATA *ch, char *argument)
{

   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   AFFECT_DATA af;
   one_argument(argument,arg);

   if (ch->level < skill_table[gsn_disguise].skill_level[ch->class]) 
   {
     send_to_char("You are not skilled in the art of disguise.\n\r",ch);
     return;
   }

   if ( argument[0] == '\0' )
   {
     send_to_char( "Usage: disguise <name of player in same room>.\n\r", ch );
     return;
   }

   if ((victim = get_char_room(ch, NULL, arg)) == NULL)
   {
     send_to_char("They aren't here.\n\r",ch);
     return;
   }

   if ( ch == victim)
   {
     act("You already look like $M.",ch,NULL,victim,TO_CHAR);
     return;
   }

   if (IS_NPC(victim))
   {
     act("You cannot disguise yourself to look like $N.",ch,NULL,victim,TO_CHAR);
     return;
   }

   if (IS_IMMORTAL(victim))
   {
     act("You are not worthy enough to look like $M.",ch,NULL,victim,TO_CHAR);
     return;
   }

   if ( is_affected(victim,gsn_treeform))
   {
     send_to_char("You can't disguise yourself to look like a tree.\n\r",ch);
     return;
   }

   if (ch->sex != victim->sex)
   {
      if (victim->sex == SEX_FEMALE)
         act("$N is much too feminine to use as a disguise.",ch,NULL,victim,TO_CHAR);
      if (victim->sex == SEX_MALE)
         act("$N is much too masculine to use as a disguise.",ch,NULL,victim,TO_CHAR);
      if (victim->sex == SEX_NEUTRAL)
         send_to_char("You cannot disguise yourself as one who is neither male nor female.\n\r",ch);
      return;
    }

   if (ch->race != victim->race)
   {
      send_to_char("You cannot disguise yourself to look like a different race.\n\r",ch);
      return;
   }

   if (is_affected(ch,gsn_doppelganger))
   {
      send_to_char("You are already wearing a disguise.\n\r",ch);
      return;
   }


   if (number_percent( ) < ch->pcdata->learned[gsn_disguise])
       check_improve(ch,gsn_disguise,TRUE,1);
   else
   {
      act("$n smears some paint on $s face.",ch,NULL,NULL,TO_ROOM);
      send_to_char("You smear some paint on your face.",ch);
      check_improve(ch,gsn_disguise,FALSE,1);
      return;
   } 

  act("You diguise yourself to look like $N.",ch,NULL,victim,TO_CHAR);

   af.type        = gsn_doppelganger;
   af.level       = ch->level;
   af.duration    = ch->level;
   af.location    = APPLY_NONE;
   af.modifier    = 0;
   af.bitvector   = 0;
  
   affect_to_char (ch,&af);
   if( is_affected( victim, gsn_doppelganger ) )
      ch->doppel = victim->doppel;
   else
      ch->doppel = victim;

return;
}

void do_heel (CHAR_DATA *ch, char *argument)
{
    if ( ch->pet == NULL )
    {
        send_to_char ( "You don't have a pet!\n\r", ch );
        return;
    }

    if (ch->pet->in_room == ch->in_room)
    {
        send_to_char ( "Your pet is already here with you! Trying to confuse it?\n\r", ch );
        return;
    }

    if ((ch->pet)->in_room->area != ch->in_room->area)
    {
        send_to_char ( "Your pet is nowhere near your location! Try in a different area!\n\r", ch );
        return;
    }

    if(ch->pet->tethered)
    {
	  act("$N is tethered, and can't go anywhere!",ch,NULL,ch->pet,TO_CHAR);
	  return;
    }

    char_from_room ( ch->pet );
    char_to_room ( ch->pet, ch->in_room );
    act ( "$n lets out a loud whistle and $N comes running.", ch, NULL,
          ch->pet, TO_ROOM );
    act ( "You let out a loud whistle and $N comes running.", ch, NULL,
          ch->pet, TO_CHAR );
}

void poison ( CHAR_DATA *ch )
{
  AFFECT_DATA af;

    	af.where     = TO_AFFECTS;
    	af.type      = gsn_poison;
    	af.level     = ch->level;
    	af.duration  = 5;
    	af.location  = APPLY_STR;
    	af.modifier  = -1;
    	af.bitvector = AFF_POISON;
    	affect_to_char( ch, &af );
}

void slow ( CHAR_DATA *ch )
{
  AFFECT_DATA af;

    	af.where     = TO_AFFECTS;
    	af.type      = skill_lookup("slow");
    	af.level     = ch->level;
    	af.duration  = 4;
    	af.location  = APPLY_DEX;
    	af.modifier  = -2;
    	af.bitvector = AFF_SLOW;
    	affect_to_char( ch, &af );
}

void do_tether( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	AFFECT_DATA *paf;
	bool can_tether = FALSE;

	one_argument(argument, arg);

	if( arg[0] == '\0' )
	{
		send_to_char("Tether what?\n\r",ch);
		return;
	}

	if( (victim = get_char_room(ch, NULL, arg)) == NULL )
	{
		send_to_char("They aren't here.\n\r",ch);
		return;
	}

	if( !IS_NPC(victim) )
	{
		send_to_char("They wouldn't like that...\n\r",ch);
		return;
	}


	if( IS_AFFECTED(victim, AFF_CHARM) && IS_SET(victim->act, ACT_PET) )
		can_tether = TRUE;

	/* Temporary charmies are not allowed to be tethered. */
	for(paf = victim->affected; paf != NULL; paf = paf->next)
	{
		if( paf->type == skill_lookup("charm person") && paf->duration > -1 )
			can_tether = FALSE;
	}

	if(!can_tether)
	{
		send_to_char("You cannot tether them.\n\r",ch);
		return;
	}

	if(victim->tethered)
	{
		act("$N is already tethered.",ch,NULL,victim,TO_CHAR);
		return;
	}

	if( !IS_OUTSIDE(victim) )
	{
		send_to_char("Try tethering them outside.\n\r",ch);
		return;
	}

	if(victim->master != ch)
	{
		act("$N refuses to be tethered by you.",ch,NULL,victim,TO_CHAR);
		return;
	}

	if(victim->rider && victim->rider != ch)
	{
		act("It seems that $N is being ridden by someone else right now.",ch,NULL,victim,TO_CHAR);
		return;
	}

	if(ch->fighting)
	{
		send_to_char("You are too busy right now to tether them!\n\r",ch);
		return;
	}

	if(victim->fighting)
	{
		send_to_char("They are too busy fighting to be tethered!\n\r",ch);
		return;
	}

	if(ch->riding == victim)
	{
		act("You dismount and tether $N.",ch,NULL,victim,TO_CHAR);
		act("$n dismounts and tethers $N.",ch,NULL,victim,TO_ROOM);
		victim->rider = NULL;
		ch->riding = NULL;
		ch->position = POS_STANDING;
		victim->tethered = TRUE;
		return;
	}

	act("You tether $N.",ch,NULL,victim,TO_CHAR);
	act("$n tethers $N.",ch,NULL,victim,TO_ROOM);

	victim->tethered = TRUE;
	return;
}

void do_untether( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	one_argument(argument, arg);

	if( arg[0] == '\0' )
	{
		send_to_char("Untether what?\n\r",ch);
		return;
	}

	if( (victim = get_char_room(ch, NULL, arg)) == NULL )
	{
		send_to_char("They aren't here.\n\r",ch);
		return;
	}

	if( !IS_NPC(victim) )
	{
		send_to_char("They wouldn't like that...\n\r",ch);
		return;
	}

	if(!victim->tethered)
	{
		act("$N is not tethered.",ch,NULL,victim,TO_CHAR);
		return;
	}

	if( victim->master != ch )
	{
		act("$N's master wouldn't like that too much.",ch,NULL,victim,TO_CHAR);
		return;
	}

	if(ch->fighting)
	{
		send_to_char("You are too busy to untether them right now.\n\r",ch);
		return;
	}

	act("You untether $N.",ch,NULL,victim,TO_CHAR);
	act("$n untethers $N.",ch,NULL,victim,TO_ROOM);

	victim->tethered = FALSE;
	return;
}

void do_treeform( CHAR_DATA *ch, char *argument )
{
  AFFECT_DATA af;
  int sect;

  sect = ch->in_room->sector_type;

  if (ch->level < skill_table[gsn_treeform].skill_level[ch->class]) 
  {
    send_to_char("You are not skilled enough to morph into the form of a tree.\n\r",ch);
    return;
  }

  if (is_affected(ch,gsn_treeform))
  {
    send_to_char("You are already morphed into the form of a tree.\n\r",ch);
    return;
  }

  if ( ch->riding )
  {
    send_to_char( "You can not morph into a tree while mounted.\n\r", ch );
    return;
  }

  if (is_affected(ch,gsn_doppelganger))
  {
    send_to_char("You can not morph into a tree while doppeled.\n\r",ch);
    return;
  }

  if (IS_AFFECTED3(ch, AFF_HOLDPERSON) )
  {
    send_to_char("You can't move at all, let alone morph into a tree!\n\r",ch);
    return;
  }

  if ( IS_AFFECTED2( ch, AFF_INVUL ) )
  {
    send_to_char( "You are invulnerable.\n\r", ch );
    return;
  }

  if (IS_AFFECTED2(ch,AFF_RAINBOW_PATTERN))
  {
    send_to_char("You can't move at all to morph into a tree.\n\r",ch);
    return;
  }

  if ( IS_AFFECTED2( ch, AFF_FEAR ))
  {
    send_to_char("Fear still courses through your body, making it impossible to morph into a tree.\n\r", ch );
    return;
  }

  if (IS_AFFECTED2(ch, AFF_PARALYSIS) )
  {
    send_to_char("You have been paralized! You couldn't possibly morph into a tree!\n\r",ch);
    return;
  }

  if ( ( sect == SECT_CITY )
  || ( sect == SECT_WATER_SWIM )
  || ( sect == SECT_WATER_NOSWIM )
  || ( sect == SECT_UNUSED ) 
  || ( sect == SECT_AIR )
  || ( sect == SECT_POLAR )
  || ( sect == SECT_TRANSPORT )
  || ( sect == SECT_UNDERWATER ) 
  || ( sect == SECT_KOT )
  || ( sect == SECT_SOLAMNIC )
  || ( sect == SECT_CONCLAVE )
  || ( sect == SECT_AESTHETIC ) 
  || ( sect == SECT_THIEVES )
  || ( sect == SECT_HOLYORDER )
  || ( sect == SECT_MERCENARY )
  || ( sect == SECT_UNDEAD ) 
  || ( sect == SECT_ARTISANS )
  || ( sect == SECT_BLACKORDER )
  || ( sect == SECT_CORRUPTION )
  || ( sect == SECT_DEATH ) 
  || ( sect == SECT_BLESSING )
  || ( sect == SECT_KNOWLEDGE )
  || ( sect == SECT_VIRTUE )
  || ( sect == SECT_CHIVALRY ) 
  || ( sect == SECT_WEALTH )
  || ( sect == SECT_WISDOM )
  || ( sect == SECT_BEAUTY )
  || ( sect == SECT_DISEASE ) 
  || ( sect == SECT_NATURE )
  || ( sect == SECT_FLAME )
  || ( sect == SECT_DESTRUCTION )
  || ( sect == SECT_JUSTICE ) 
  || ( sect == SECT_LIFE )
  || ( sect == SECT_HATRED )
  || ( sect == SECT_STORM )
  || ( sect == SECT_BATTLE ) 
  || ( sect == SECT_RMAGIC )
  || ( sect == SECT_BMAGIC )
  || ( sect == SECT_WMAGIC )
  || ( sect == SECT_NONE )
  || ( IS_SET(ch->in_room->room_flags, ROOM_INDOORS) ) )
  {
    send_to_char("Your magic will not work in this area.  You must be outdoors and on land where trees would normally grow.\n\r",ch);
    return;
  }

  if (number_percent( ) < ch->pcdata->learned[gsn_treeform])
       check_improve(ch,gsn_treeform,TRUE,1);
  else
  {
    act("$n's body morphs into an unrecognizable shape, before reverting back to normal.",ch,NULL,NULL,TO_ROOM);
    send_to_char("Your body morphs into an unrecognizable shape, before reverting back to its normal state.",ch);
    check_improve(ch,gsn_treeform,FALSE,1);
    return;
  }

  af.type        = gsn_treeform;
  af.level       = ch->level;
  af.duration    = ch->level/2;
  af.location    = APPLY_NONE;
  af.modifier    = 0;
  af.bitvector   = 0;
  affect_to_char (ch,&af);

  af.where        = TO_AFFECTS;
  af.type 	      = gsn_treeform;
  af.level	  	= ch->level;
  af.duration  	= ch->level/2;
  af.location  	= APPLY_STR;
  af.modifier  	= 3; 
  af.bitvector 	= 0;
  affect_join(ch,&af);

  af.where	 	= TO_VULN;
  af.type         = gsn_treeform;
  af.level	 	= ch->level;
  af.duration 	= ch->level/2;
  af.location  	= APPLY_NONE;
  af.modifier  	= 0;     
  af.bitvector 	= VULN_FIRE;
  affect_to_char( ch, &af );

  if ( ch->level <= 70 )
  {
    af.location  = APPLY_AC;
    af.modifier  = (-20+(-1*(ch->level/2)));
    af.bitvector = 0;
    affect_to_char( ch, &af );
  }
  else
  if ( ch->level > 70 && ch->level <= 85 )
  {
    af.location  = APPLY_AC;
    af.modifier  = (-60+(-1*(ch->level/2)));
    af.bitvector = 0;
    affect_to_char( ch, &af );
  }
  else
  if ( ch->level > 85 )
  {
    af.location  = APPLY_AC;
    af.modifier  = (-100+(-1*(ch->level/2)));
    af.bitvector = 0;
    affect_to_char( ch, &af );
  }

  act( "$n's arms morph into tree limbs and $s legs become one\n\r$n's skin begins to turn into bark and $s hair turns into leaves\n\r$n suddenly takes on the shape of a mighty oak tree", ch, NULL, NULL, TO_ROOM );
  send_to_char( "Your arms morph into tree limbs and your legs become one\n\rYour skin begins to turn into bark and your hair turns into leaves\n\rYou suddenly take on the shape of a mighty oak tree\n\r", ch );

return;
}
