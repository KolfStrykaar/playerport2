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
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include "merc.h"
#include "interp.h"
#include "magic.h"
#include "recycle.h"
#include "tables.h"
#include "lookup.h"

/*
 * Local functions.
 */
void check_assist		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool check_dodge		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void check_killer		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool check_parry		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool check_shield_block	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void dam_message 		args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam,int dt, bool immune ) );
void death_cry		args( ( CHAR_DATA *ch ) );
void group_gain		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
int  xp_compute		args( ( CHAR_DATA *gch, CHAR_DATA *victim,int total_levels, int ngrp ) );
bool is_safe		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void make_corpse		args( ( CHAR_DATA *ch ) );
void one_hit     		args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt, bool secondary ) );
void mob_hit		args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) );
void raw_kill		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void mercy_kill      	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void set_fighting		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void disarm			args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool check_counter   	args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt ) );
bool check_critical  	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool check_guard     	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool check_displacement	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool check_tumble    	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool check_sidestep  	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool check_pivot      	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool check_redirect   	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool check_riposte   	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool check_evade     	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool check_acrobatics 	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void check_advance	      ( CHAR_DATA *ch, CHAR_DATA *victim );
int  get_armor_penalty  args( ( CHAR_DATA *ch ) );
bool is_leather		args((OBJ_DATA *obj));
bool is_chain		args((OBJ_DATA *obj));
bool is_plate		args((OBJ_DATA *obj));
bool is_hating       	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void stop_hating     	args( ( CHAR_DATA *ch ) );
void start_hating		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void stop_fighting   	args( ( CHAR_DATA *ch, bool fBoth ) );
bool	remove_obj	args( (CHAR_DATA *ch, int iWear, bool fReplace ) );
bool check_heroic		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool check_arrow_defl   args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );

extern bool spec_rloader( CHAR_DATA *ch );

/*
 * Control the fights going on.
 * Called periodically by update_handler.
 */
void violence_update( void )
{
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;
    CHAR_DATA *victim;
    OBJ_DATA *obj, *obj_next;
    bool room_trig = FALSE;

    for ( ch = char_list; ch != NULL; ch = ch->next )
    {
	ch_next = ch->next;

	if ( ( victim = ch->fighting ) == NULL || ch->in_room == NULL )
	    continue;

	if ( IS_AWAKE(ch) && ch->in_room == victim->in_room ) 	
       {
         multi_hit( ch, victim, TYPE_UNDEFINED );
       } 	 	
      else
         stop_fighting( ch, FALSE );

      if( IS_AFFECTED3(victim, AFF_SUBDUE) 
	&& IS_SET(ch->act,PLR_AUTOMERCY))
	   stop_fighting(ch, TRUE);

      if (!IS_NPC(ch) && !IS_NPC(victim)
      && (ABS(ch->level - victim->level) > 10)
      && (ch->level <= LEVEL_HERO)
      && IS_AFFECTED3(victim, AFF_SUBDUE))
          stop_fighting(ch, TRUE);
	
       if ( ( victim = ch->fighting ) == NULL )
	    continue;

	/*
	 * Fun for the whole family!
	 */
	check_assist(ch,victim);
      /* Check ranks of fighters */
      check_advance(ch, victim);
  	if ( IS_NPC( ch ) )
	{
	    if ( HAS_TRIGGER_MOB( ch, TRIG_FIGHT ) )
		p_percent_trigger( ch, NULL, NULL, victim, NULL, NULL, TRIG_FIGHT );
	    if ( HAS_TRIGGER_MOB( ch, TRIG_HPCNT ) )
		p_hprct_trigger( ch, victim );
	}
	for ( obj = ch->carrying; obj; obj = obj_next )
	{
	    obj_next = obj->next_content;

	    if ( obj->wear_loc != WEAR_NONE && HAS_TRIGGER_OBJ( obj, TRIG_FIGHT ) )
		p_percent_trigger( NULL, obj, NULL, victim, NULL, NULL, TRIG_FIGHT );
	}

	if ( HAS_TRIGGER_ROOM( ch->in_room, TRIG_FIGHT ) && room_trig == FALSE )
	{
	    room_trig = TRUE;
	    p_percent_trigger( NULL, NULL, ch->in_room, victim, NULL, NULL, TRIG_FIGHT );
	}
	if (ch->fight_pos == FIGHT_REAR)
	{
		bug("Stopping fighting, fight = REAR", 0);
		stop_fighting(ch, TRUE);
	}
    }

    return;
}

int get_armor_penalty ( CHAR_DATA *ch )
{
    OBJ_DATA *obj;
    int incr;
    int chance  = 0;
    int counted = 0;
    int penalty = 0;
    int ii;
    
/* Need to check  body, arms, legs, head                 for leather,
                  body, arms, legs, head, hands, feet    for plate and chain */
                 
           
/* Check the body */

for ( ii = 8 ; ii < 257 ; ii *= 2 )
{
if ( (obj = get_eq_char(ch,WEAR_BODY)) != NULL )
{
    if ( ii == 8 || ii == 32 || ii == 256 )
    {
    if (is_leather(obj))
        {
            chance = get_skill(ch,gsn_wear_leather);
                      
            for (incr = 0; incr < 3; incr++)
            if ( number_percent() > chance ) counted = 1;
           
	    if (counted == 0) check_improve(ch,gsn_wear_leather,TRUE,5);
	    
            if (counted == 1) 
            {
                check_improve(ch,gsn_wear_leather,FALSE,5);
                penalty++;
            }            
        }   
     }

	if (is_chain(obj))
        {
            chance = get_skill(ch,gsn_wear_chain);
                      
            for (incr = 0; incr < 3; incr++)
            if ( number_percent() > chance ) counted = 1;
           
 	    if (counted == 0) check_improve(ch,gsn_wear_chain,TRUE,5);
	    
            if (counted == 1) 
            {
                check_improve(ch,gsn_wear_chain,FALSE,5);
                penalty += 2;
            }

        }
       
	if (is_plate(obj))
        {
            chance = get_skill(ch,gsn_wear_plate);
                       
            for (incr = 0; incr < 3; incr++)
            if ( number_percent() > chance ) counted = 1;
           
 	    if (counted == 0) check_improve(ch,gsn_wear_plate,TRUE,5);
	    
            if (counted == 1) 
            {
                check_improve(ch,gsn_wear_plate,FALSE,5);
                penalty += 3;
            }
                    
        }
   }        
 }
return(penalty);
}

/* for auto assisting */
void check_assist(CHAR_DATA *ch,CHAR_DATA *victim)
{
    CHAR_DATA *rch, *rch_next;

    for (rch = ch->in_room->people; rch != NULL; rch = rch_next)
    {
	rch_next = rch->next_in_room;

	if (rch->fight_pos == FIGHT_REAR)
	continue;

	if (IS_AWAKE(rch) && rch->fighting == NULL)
	{

	    /* guards of player assist their charge */
	    if ( (ch->guarded == rch) && (rch->position != POS_DEAD)  )
	      {
      	act("$n maneuvers to defend $N.", rch, NULL, ch, TO_NOTVICT);
		act("You maneuver to defend $N.", rch, NULL, ch, TO_CHAR);
		act("$n maneuvers to defend you.", rch, NULL, ch, TO_VICT);
		multi_hit(rch,victim,TYPE_UNDEFINED);
                continue;            
            }

	    /* quick check for ASSIST_PLAYER */
	    if (!IS_NPC(ch) && IS_NPC(rch) 
	    && IS_SET(rch->off_flags,ASSIST_PLAYERS)
	    &&  rch->level + 6 > victim->level)
	    {
		do_function(rch, &do_emote, "screams and attacks!");
		multi_hit(rch,victim,TYPE_UNDEFINED);
		continue;
	    }

	    /* PCs next */
	    if (!IS_NPC(ch) || IS_AFFECTED(ch,AFF_CHARM))
	    {
		if ( ( (!IS_NPC(rch) && IS_SET(rch->act,PLR_AUTOASSIST))
		||     IS_AFFECTED(rch,AFF_CHARM)) 
		&&   is_same_group(ch,rch) 
		&&   !is_safe(rch, victim))
		    multi_hit (rch,victim,TYPE_UNDEFINED);
		
		continue;
	    }
  	
	    /* now check the NPC cases */
	    
 	    if (IS_NPC(ch) && !IS_AFFECTED(ch,AFF_CHARM))
	
	    {
		if ( (IS_NPC(rch) && IS_SET(rch->off_flags,ASSIST_ALL))
		||   (IS_NPC(rch) && rch->group && rch->group == ch->group)
		||   (IS_NPC(rch) && rch->race == ch->race 
		   && IS_SET(rch->off_flags,ASSIST_RACE))
		||   (IS_NPC(rch) && IS_SET(rch->off_flags,ASSIST_ALIGN)
		   &&   ((IS_GOOD(rch)    && IS_GOOD(ch))
		     ||  (IS_EVIL(rch)    && IS_EVIL(ch))
		     ||  (IS_NEUTRAL(rch) && IS_NEUTRAL(ch)))) 
		||   (rch->pIndexData == ch->pIndexData 
		   && IS_SET(rch->off_flags,ASSIST_VNUM)))
	   	{
		    CHAR_DATA *vch;
		    CHAR_DATA *target;
		    int number;

		    if (number_bits(1) == 0)
			continue;
		
		    target = NULL;
		    number = 0;
		    for (vch = ch->in_room->people; vch; vch = vch->next)
		    {
			if (can_see(rch,vch)
			&&  is_same_group(vch,victim)
			&&  number_range(0,number) == 0)
			{
			    target = vch;
			    number++;
			}
		    }

		    if (target != NULL)
		    {
			do_function(rch, &do_emote, "screams and attacks!");
			multi_hit(rch,target,TYPE_UNDEFINED);
		    }
		}	
	    }
	}
    }
}

void check_advance (CHAR_DATA *ch, CHAR_DATA *victim)
{
    CHAR_DATA *vch;
    int Done = FALSE;
    int has_front = FALSE;
    int num = 0;

    if (!IS_NPC(ch))
	return;

    for (vch = ch->in_room->people ; vch != NULL; vch = vch->next_in_room)
    {
	if (is_same_group(victim, vch))
	{
		if (vch->fight_pos == FIGHT_FRONT && vch->fighting
                    && !IS_NPC(vch)) 
			has_front = TRUE;
		num++;
	}
    }

    for (vch = ch->in_room->people ; vch != NULL; vch = vch->next_in_room)
    {
	if (is_same_group(victim, vch))
	{
           	if (has_front  && vch->fight_pos == FIGHT_REAR)
			stop_fighting(vch, TRUE); 
	}
    }

    if (has_front)
	return;


    for (vch = ch->in_room->people ; vch ; vch = vch->next_in_room)
    {
	if (vch == ch)
		continue;

	if (is_same_group(victim, vch))
	{
		if ((vch->fighting == NULL && num > 1) || !can_see(ch, vch))
			continue;
		if (vch->fight_pos == FIGHT_FRONT)
			return;
		if (vch->fight_pos == FIGHT_MIDDLE)
		{
			vch->fight_pos = FIGHT_FRONT;
			Done = TRUE;
			break;
		}
	}
    }

    if (Done)
    {

	act("{wThe monsters advance.{x", ch, NULL, NULL, TO_ROOM);
	act("{wThe monsters advance.{x", ch, NULL, NULL, TO_CHAR);
	return; 
    }

    for (vch = ch->in_room->people ; vch ; vch = vch->next_in_room)
    {
	if (vch == ch)
		continue;

	if (is_same_group(victim, vch))
	{
		if ((vch->fighting == NULL && num > 1) || !can_see(ch, vch))
			;
		if (vch->fight_pos == FIGHT_REAR)
		{
			vch->fight_pos = FIGHT_MIDDLE;
			Done = TRUE;
		}
	}
    }

    if (Done)
    {
	act("{wThe monsters advance.{x", ch, NULL, NULL, TO_ROOM);
	act("{wThe monsters advance.{x", ch, NULL, NULL, TO_CHAR);
	return;
    }
}

/*
 * Do one group of attacks.
 */
void multi_hit( CHAR_DATA *ch, CHAR_DATA *victim, int dt )
{
    int penalty;
    int chance;
    int dskill;
    int dsn;

    /* decrement the wait */
    if (ch->desc == NULL)
	ch->wait = UMAX(0,ch->wait - PULSE_VIOLENCE);

    if (ch->desc == NULL)
	ch->daze = UMAX(0,ch->daze - PULSE_VIOLENCE); 


    /* no attacks for stunnies -- just a check */
    if (ch->position < POS_RESTING)
	return;

    if (IS_NPC(ch))
    {
	mob_hit(ch,victim,dt);
	return;
    }

    /* Now for PC's */
    penalty = get_armor_penalty(ch);

    ch->damroll -= penalty;
    ch->hitroll -= penalty;

    if (!ch->fighting)
        set_fighting(ch, victim);

    one_hit( ch, victim, dt, FALSE );

    if (IS_AFFECTED3(victim, AFF_SUBDUE)
    && IS_SET(ch->act,PLR_AUTOMERCY))
       return;

    if (!IS_NPC(ch) && !IS_NPC(victim)
    && (ABS(ch->level - victim->level) > 10)
    && (ch->level <= LEVEL_HERO)
    && IS_AFFECTED3(victim, AFF_SUBDUE))
	 return;

    if (get_eq_char (ch, WEAR_SECONDARY) != NULL)
    {
      dsn = get_second_weapon_sn(ch);
      dskill = (get_skill(ch,gsn_dual_wield) * (get_weapon_skill(ch,dsn)))/100;
      if( number_percent() < dskill )
      {
        check_improve(ch,gsn_dual_wield,TRUE,5);
        one_hit( ch, victim, dt, TRUE );
      }
    } 

    if (ch->fighting != victim)
    {
        ch->damroll += penalty;
        ch->hitroll += penalty;        
        return;
    }

    if (IS_AFFECTED3(victim, AFF_SUBDUE)
    && IS_SET(ch->act,PLR_AUTOMERCY))
       return;

    if (!IS_NPC(ch) && !IS_NPC(victim)
    && (ABS(ch->level - victim->level) > 10)
    && (ch->level <= LEVEL_HERO)
    && IS_AFFECTED3(victim, AFF_SUBDUE))
	 return;

    if (IS_AFFECTED(ch,AFF_HASTE))
	one_hit(ch,victim,dt,FALSE);

    if ( ch->fighting != victim || dt == gsn_backstab || dt == gsn_double_bs )
    {
        ch->damroll += penalty;
        ch->hitroll += penalty;
        return;
    }
    if ( ch->fighting != victim || dt == gsn_ambush )
    {
        ch->damroll += penalty;
        ch->hitroll += penalty;
        return;
    }
    if ( ch->fighting != victim || dt == gsn_circle )
    {
        ch->damroll += penalty;
        ch->hitroll += penalty;
        return;
    }

    if (ch->riding && !IS_NPC(ch))
    {
      if (number_percent() > get_skill(ch,gsn_mounted_combat))
      {
        act("You lose your balance and fall off $N.",
             ch,NULL,ch->riding,TO_CHAR);
        act("$n loses $s balance and falls off $N.",
            ch,NULL,ch->riding,TO_ROOM);
	  ch->riding->rider = NULL;
	  ch->riding        = NULL;
        check_improve(ch,gsn_mounted_combat,FALSE,1);  
      }
      check_improve(ch,gsn_mounted_combat,TRUE,8);
    }

    chance = get_skill(ch,gsn_second_attack)/2;

    if (IS_AFFECTED(ch,AFF_SLOW))
	chance /= 2;

    if (IS_AFFECTED3(victim, AFF_SUBDUE)
    && IS_SET(ch->act,PLR_AUTOMERCY))
       return;

    if (!IS_NPC(ch) && !IS_NPC(victim)
    && (ABS(ch->level - victim->level) > 10)
    && (ch->level <= LEVEL_HERO)
    && IS_AFFECTED3(victim, AFF_SUBDUE))
	 return;

    if ( number_percent( ) < chance )
    {
	one_hit( ch, victim, dt, FALSE );
	check_improve(ch,gsn_second_attack,TRUE,5);

      if (get_eq_char(ch, WEAR_SECONDARY) != NULL)
      {
        if (IS_AFFECTED3(victim, AFF_SUBDUE)
        && IS_SET(ch->act,PLR_AUTOMERCY))
    	     return;

	  if (!IS_NPC(ch) && !IS_NPC(victim)
        && (ABS(ch->level - victim->level) > 10)
        && (ch->level <= LEVEL_HERO)
        && IS_AFFECTED3(victim, AFF_SUBDUE))
	     return;

        if ( number_percent( ) < 50 )
        {
          dsn = get_second_weapon_sn(ch);
          dskill = ((get_skill(ch,gsn_dual_wield) * (get_weapon_skill(ch,dsn)))/100)*3/4;
          if( number_percent() < dskill )
          {
		act( "You switch your stance and return with a strike from your other weapon!", ch, NULL,victim, TO_CHAR ); 
            check_improve(ch,gsn_dual_wield,TRUE,1);
            one_hit( ch, victim, dt, TRUE );
          }
        } 
      }
      if ( ch->fighting != victim )
      {
            ch->hitroll += penalty;
            ch->damroll += penalty;
            return;
	}
    }

    chance = get_skill(ch,gsn_third_attack)/3;

    if (IS_AFFECTED(ch,AFF_SLOW))
	chance = 0;

    if (IS_AFFECTED3(victim, AFF_SUBDUE)
    && IS_SET(ch->act,PLR_AUTOMERCY))
       return;

    if (!IS_NPC(ch) && !IS_NPC(victim)
    && (ABS(ch->level - victim->level) > 10)
    && (ch->level <= LEVEL_HERO)
    && IS_AFFECTED3(victim, AFF_SUBDUE))
	 return;

    if ( number_percent( ) < chance )
    {
	one_hit( ch, victim, dt, FALSE );
	check_improve(ch,gsn_third_attack,TRUE,6);

      if (get_eq_char(ch, WEAR_SECONDARY) != NULL)
      {
        if (IS_AFFECTED3(victim, AFF_SUBDUE)
        && IS_SET(ch->act,PLR_AUTOMERCY))
    	     return;

	  if (!IS_NPC(ch) && !IS_NPC(victim)
        && (ABS(ch->level - victim->level) > 10)
        && (ch->level <= LEVEL_HERO)
        && IS_AFFECTED3(victim, AFF_SUBDUE))
	     return;

        if ( number_percent( ) < 25 )
        {
          dsn = get_second_weapon_sn(ch);
          dskill = ((get_skill(ch,gsn_dual_wield) * (get_weapon_skill(ch,dsn)))/100)*3/4;
          if( number_percent() < dskill )
          {
		act( "You switch your stance and return with a strike from your other weapon!", ch, NULL,victim, TO_CHAR ); 
            check_improve(ch,gsn_dual_wield,TRUE,1);
            one_hit( ch, victim, dt, TRUE );
          }
        } 
      }

      if ( ch->fighting != victim )
      {
        ch->hitroll += penalty;
        ch->damroll += penalty;          
        return;
	}
    }

    chance = get_skill(ch,gsn_fourth_attack)/4;

    if (IS_AFFECTED(ch,AFF_SLOW))
              chance = 0;

    if (IS_AFFECTED3(victim, AFF_SUBDUE)
    && IS_SET(ch->act,PLR_AUTOMERCY))
       return;

    if (!IS_NPC(ch) && !IS_NPC(victim)
    && (ABS(ch->level - victim->level) > 10)
    && (ch->level <= LEVEL_HERO)
    && IS_AFFECTED3(victim, AFF_SUBDUE))
	 return;

    if ( number_percent( ) < chance )
    {
	one_hit( ch, victim, dt, FALSE );
	check_improve(ch,gsn_fourth_attack, TRUE,6);
        if ( ch->fighting != victim )
        {
            ch->hitroll += penalty;
            ch->damroll += penalty;          
            return;
	  }
    }

    ch->hitroll += penalty;
    ch->damroll += penalty;

    if (ch->fight_pos == FIGHT_REAR)
    {
	check_assist(ch, victim);
	stop_fighting(ch, FALSE);
    }
    return;
}

/* procedure for all mobile attacks */
void mob_hit (CHAR_DATA *ch, CHAR_DATA *victim, int dt)
{
    int chance,number;
    CHAR_DATA *vch, *vch_next;

    one_hit(ch,victim,dt,FALSE);

    if (ch->fighting != victim)
	return;

    /* Area attack -- BALLS nasty! */
 
    if (IS_SET(ch->off_flags,OFF_AREA_ATTACK))
    {
	for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
	{
	    vch_next = vch->next;
	    if ((vch != victim && vch->fighting == ch))
		one_hit(ch,vch,dt,FALSE);
	}
    }

    if (IS_AFFECTED(ch,AFF_HASTE) 
    ||  (IS_SET(ch->off_flags,OFF_FAST) && !IS_AFFECTED(ch,AFF_SLOW)))
	one_hit(ch,victim,dt,FALSE);

    if (ch->fighting != victim || dt == gsn_backstab || dt == gsn_double_bs)
	return;

    if (ch->fighting != victim || dt == gsn_ambush )
	return;

    if (ch->fighting != victim || dt == gsn_circle )
	return;

    chance = get_skill(ch,gsn_second_attack)/2;

    if (IS_AFFECTED(ch,AFF_SLOW) && !IS_SET(ch->off_flags,OFF_FAST))
	chance /= 2;

    if (number_percent() < chance)
    {
	one_hit(ch,victim,dt,FALSE);
	if (ch->fighting != victim)
	    return;
    }

    chance = get_skill(ch,gsn_third_attack)/4;

    if (IS_AFFECTED(ch,AFF_SLOW) && !IS_SET(ch->off_flags,OFF_FAST))
	chance = 0;

    if (number_percent() < chance)
    {
	one_hit(ch,victim,dt,FALSE);
	if (ch->fighting != victim)
	    return;
    } 

    /* oh boy!  Fun stuff! */

    if (ch->wait > 0)
	return;

    number = number_range(0,2);

    if (number == 1 && IS_SET(ch->act,ACT_MAGE))
    {
	/*  { mob_cast_mage(ch,victim); return; } */ ;
    }

    if (number == 2 && IS_SET(ch->act,ACT_CLERIC))
    {	
	/* { mob_cast_cleric(ch,victim); return; } */ ;
    }

    /* now for the skills */

    number = number_range(0,8);

    switch(number) 
    {
    case (0) :
	if (IS_SET(ch->off_flags,OFF_BASH))
	    do_function(ch, &do_bash, "");
	break;

    case (1) :
	if (IS_SET(ch->off_flags,OFF_BERSERK) && !IS_AFFECTED(ch,AFF_BERSERK))
	    do_function(ch, &do_berserk, "");
	break;


    case (2) :
	if (IS_SET(ch->off_flags,OFF_DISARM) 
	|| (get_weapon_sn(ch) != gsn_hand_to_hand 
	&& (IS_SET(ch->act,ACT_WARRIOR)
   	||  IS_SET(ch->act,ACT_THIEF))))
	    do_function(ch, &do_disarm, "");
	break;

    case (3) :
	if (IS_SET(ch->off_flags,OFF_KICK))
	    do_function(ch, &do_kick, "");
	break;

    case (4) :
	if (IS_SET(ch->off_flags,OFF_KICK_DIRT))
	    do_function(ch, &do_dirt, "");
	break;

    case (5) :
	if (IS_SET(ch->off_flags,OFF_TAIL))
	{
	    do_function(ch, &do_tail, "");
	}
	break; 

    case (6) :
	if (IS_SET(ch->off_flags,OFF_TRIP))
	    do_function(ch, &do_trip, "");
	break;

    case (7) :
	if (IS_SET(ch->off_flags,OFF_CRUSH))
	{
	    do_function(ch, &do_crush, "");
	}
	break;
    case (8) :
	if (IS_SET(ch->off_flags,OFF_BACKSTAB))
	{
	    do_function(ch, &do_backstab, "");
	}
    }
}
	

/*
 * Hit one guy once.
 */
void one_hit ( CHAR_DATA *ch, CHAR_DATA *victim, int dt, bool secondary )
{
    OBJ_DATA *wield;
    OBJ_DATA *armor;
    OBJ_DATA *dual = NULL;
    OBJ_DATA *obj;
    int iWear, chance, range, loc, num_range = 0;
    int victim_ac;
    int thac0;
    int thac0_00;
    int thac0_32;
    int dam;
    int diceroll;
    int sn,skill;
    int dam_type;
    bool result = FALSE;

    sn = -1;


    /* just in case */
    if (victim == ch || ch == NULL || victim == NULL)
	return;

    /*
     * Can't beat a dead char!
     * Guard against weird room-leavings.
     */
    if ( victim->position == POS_DEAD || ch->in_room != victim->in_room )
	return;

    /*
     * Figure out the type of damage message.
     * if secondary == true, use the second weapon.
     */
    if (!secondary)
        wield = get_eq_char( ch, WEAR_WIELD );
    else
        wield = get_eq_char( ch, WEAR_SECONDARY );

    if ( dt == TYPE_UNDEFINED )
    {
	dt = TYPE_HIT;
	if ( wield != NULL && wield->item_type == ITEM_WEAPON )
	    dt += wield->value[3];
	else 
	    dt += ch->dam_type;
    }

    if (dt < TYPE_HIT)
    	if (wield != NULL)
    	    dam_type = attack_table[wield->value[3]].damage;
    	else
    	    dam_type = attack_table[ch->dam_type].damage;
    else
    	dam_type = attack_table[dt - TYPE_HIT].damage;

    if (dam_type == -1)
	dam_type = DAM_BASH;

    /* get the weapon skill */
    sn = get_weapon_sn(ch);
    skill = 20 + get_weapon_skill(ch,sn);

    /*
     * Calculate to-hit-armor-class-0 versus armor.
     */
    if ( IS_NPC(ch) )
    {
	thac0_00 = 20;
	thac0_32 = -4;   /* as good as a thief */ 
	if (IS_SET(ch->act,ACT_WARRIOR))
	    thac0_32 = -10;
	else if (IS_SET(ch->act,ACT_THIEF))
	    thac0_32 = -4;
	else if (IS_SET(ch->act,ACT_CLERIC))
	    thac0_32 = 2;
	else if (IS_SET(ch->act,ACT_MAGE))
	    thac0_32 = 6;
    }
    else
    {
	thac0_00 = class_table[ch->class].thac0_00;
	thac0_32 = class_table[ch->class].thac0_32;
    }
    thac0  = interpolate( ch->level, thac0_00, thac0_32 );

    if (thac0 < 0)
        thac0 = thac0/2;

    if (thac0 < -5)
        thac0 = -5 + (thac0 + 5) / 2;

    thac0 -= GET_HITROLL(ch) * skill/100;
    thac0 += 5 * (100 - skill) / 100;

    if (dt == gsn_cutthroat || dt == gsn_cutthroat)
       thac0 -= 10 * (100 - get_skill(ch,gsn_cutthroat));  

    if (dt == gsn_backstab)
	thac0 -= 10 * (100 - get_skill(ch,gsn_backstab));

    if (dt == gsn_ambush)
	thac0 -= 10 * (100 - get_skill(ch,gsn_ambush));

    if (dt == gsn_circle)
        thac0 -= 10 * (100 - get_skill(ch,gsn_circle));

    switch(dam_type)
    {
	case(DAM_PIERCE):victim_ac = GET_AC(victim,AC_PIERCE)/10;	break;
	case(DAM_BASH):	 victim_ac = GET_AC(victim,AC_BASH)/10;		break;
	case(DAM_SLASH): victim_ac = GET_AC(victim,AC_SLASH)/10;	break;
	default:	 victim_ac = GET_AC(victim,AC_EXOTIC)/10;	break;
    }; 
	
    if (victim_ac < -15)
	victim_ac = (victim_ac + 15) / 5 - 15;
     
    if ( !can_see( ch, victim ) )
	victim_ac -= 4;

    if ( victim->position < POS_FIGHTING)
	victim_ac += 4;
 
    if (victim->position < POS_RESTING)
	victim_ac += 6;

    /*
     * The moment of excitement!
     */
    while ( ( diceroll = number_bits( 5 ) ) >= 20 )
	;

    if ( diceroll == 0
    || ( diceroll != 19 && diceroll < thac0 - victim_ac ) )
    {
	/* Miss. */
	damage( ch, victim, 0, dt, dam_type, TRUE );
	tail_chain( );
	return;
    }

    /*
     * Hit.
     * Calc damage.
     */
    if ( IS_NPC(ch) && (!ch->pIndexData->new_format || wield == NULL))
	if (!ch->pIndexData->new_format)
	{
	    dam = number_range( ch->level / 2, ch->level * 3 / 2 );
	    if ( wield != NULL )
	    	dam += dam / 2;
	}
	else
	    dam = dice(ch->damage[DICE_NUMBER],ch->damage[DICE_TYPE]);
	
    else
    {
	if (sn != -1)
	    check_improve(ch,sn,TRUE,5);
	if ( wield != NULL )
	{
	    if (wield->pIndexData->new_format)
		dam = dice(wield->value[1],wield->value[2]) * skill/100;
	    else
	    	dam = number_range( wield->value[1] * skill/100, 
				wield->value[2] * skill/100);

	    if (get_eq_char(ch,WEAR_SHIELD) == NULL)  /* no shield = more */
		dam = dam * 11/10;

	    /* sharpness! */
	    if (IS_WEAPON_STAT(wield,WEAPON_SHARP))
	    {
		int percent;

		if ((percent = number_percent()) <= (skill / 8))
		    dam = 2 * dam + (dam * 2 * percent / 100);
	    }
	}
      else
                    /* Lets give bonuses to people wearing gloves */
      if ((( obj = get_eq_char(ch,WEAR_HANDS)) != NULL )
      &&   ( wield == NULL))
      {
	  if (is_plate(obj))
	    dam = number_range( 1 + 4 * skill/20, 2 * ch->level/3 * skill/20);
        else
        if (is_chain(obj))
	    dam = number_range( 1 + 4 * skill/40, 2 * ch->level/3 * skill/40);
        else
        if (is_leather(obj))
	    dam = number_range( 1 + 4 * skill/60, 2 * ch->level/3 * skill/60);
        else
	    dam = number_range( 1 + 4 * skill/80, 2 * ch->level/3 * skill/80);
	}
	else
	    dam = number_range( 1 + 4 * skill/100, 2 * ch->level/3 * skill/100);
    }

    /*
     * Bonuses.
     */
    if ( get_skill(ch,gsn_enhanced_damage) > 0 )
    {
        diceroll = number_percent();
        if (diceroll <= get_skill(ch,gsn_enhanced_damage))
        {
            check_improve(ch,gsn_enhanced_damage,TRUE,6);
            dam += 2 * ( dam * diceroll/300);
        }

    if ( wield != NULL && IS_OBJ_STAT(wield,ITEM_DEATH) )
    {
        if (victim->alignment > 700)
                dam = (110 * dam) / 100;

        else if (victim->alignment > 350)
                dam = (105*dam) / 100;

        else dam = (102*dam) / 100;
    }

    if ( check_critical(ch,victim) )         
      dam *= 2;
    }

    if ( ch->fight_pos == FIGHT_MIDDLE )  
        dam /= 2;

    if ( victim->fight_pos == FIGHT_MIDDLE )   
        dam /= 2;

    if ( !IS_AWAKE(victim) )
	dam *= 2;
     else if (victim->position < POS_FIGHTING)
	dam = dam * 3 / 2;

    if ( (dt == gsn_backstab || dt == gsn_double_bs) && wield != NULL)
    {
    	if ( wield->value[0] != 2 )
	    dam *= 2 + (ch->level / 10); 
	else 
            dam *= 2 + (ch->level / 8);
    }

    if ( dt == gsn_cleave  && wield != NULL)
    {
      dam *= 2 + (ch->level / 12);
    }

    if ( dt == gsn_ambush && wield != NULL)
    {
    	if ( wield->value[0] != 2 )
	    dam *= 2 + (ch->level / 9); 
	else 
            dam *= 2 + (ch->level / 7);
    }

    if ( dt == gsn_cutthroat && wield != NULL)
    {
    	if ( wield->value[0] != 2 )
	    dam *= 2 + (ch->level / 9); 
	else 
            dam *= 2 + (ch->level / 7);

            if( dt == gsn_cutthroat)
                dam = dam * 4 / 7;  
    }

    if ( dt == gsn_circle && wield != NULL)
    {
        if ( wield->value[0] != 2 )
           dam *= .5+ (ch->level / 20);
        else
           dam *= .5 + (ch->level / 18);
        ch->fight_pos = FIGHT_FRONT; 
    }

    dam += GET_DAMROLL(ch) * UMIN(100,skill) /100;

    if ( dam <= 0 )
	dam = 1;

        if ( !check_counter( ch, victim, dam, dt ) )
        result = damage( ch, victim, dam, dt, dam_type, TRUE );
	  else return;

/* armor and weapon fighting condition code. */
	chance = 4;
	range = 500;

/* using a var like num_range instead of continually recalculating chances; gives the same
 * amount of chance to every item. If the chance is recalucluated on every item by using
 * number_range(0,range) everytime, the armor is hardly ever hit. Calculating chance only once
 * and using the var should increase chances of armor being hit; instead of having it be
 * (4/500 * 4/500) probability, making it just 4/500 probabilty, or around one every 20 fights.
 *
 * It was changed to be like this, as it was noticed that armor was hardly ever hit for condition loss.
 */
	num_range = number_range(0,range);
	if( result && (num_range <= chance) )
	{
		if(wield != NULL)
		{
			if( wield->condition > 0 )
			{
				if( (num_range <= chance/2) && IS_OBJ_STAT(wield, ITEM_STAINLESS) )
					wield->condition -= 1;
			
				if( !IS_OBJ_STAT(wield, ITEM_STAINLESS) )
					wield->condition -= 1;
			}
		}

		/* Dual wielded weapon gets hit too, and it gets hit more often. */
		if( (dual = get_eq_char(ch, WEAR_SECONDARY)) != NULL )
		{
			if(dual->condition > 0)
			{
				if( (num_range <= chance/2) && IS_OBJ_STAT(dual, ITEM_STAINLESS) )
					dual->condition -= 1;
			
				if( !IS_OBJ_STAT(dual, ITEM_STAINLESS) )
					dual->condition -= 1;
			}
		}

		/* go through all wear locations and see if there's armor to hit.
		 * if there is, run the chance to see if it's hit; and condition is lost. */
		for(iWear = 0; iWear < MAX_WEAR; iWear++)
		{
			if( (armor = get_eq_char(victim, iWear)) == NULL )
				continue;
	
			/* randomly pick some armor to hit */
			loc = number_range(iWear, MAX_WEAR);

			/* The tradeoff here is the less armor the victim wears, the less likely
			 * it'll be hit for condition loss. Obviously we want all of it to be hit,
			 * but it might be passed up because of the random armor picker (above).
			 */
			if( (armor = get_eq_char(victim, loc)) == NULL )
				continue;

			/* if the armor is at or below 0, don't reduce it anymore, 
			 * because it'll be destroyed. 
			 */
			if( armor->condition <= 0 )
				continue;

			switch(loc)
			{
				case WEAR_FINGER_L:
				case WEAR_FINGER_R:
				case WEAR_NECK_1:
				case WEAR_NECK_2:
				case WEAR_BODY:
				case WEAR_HEAD:
				case WEAR_FACE:
				case WEAR_LEGS:
				case WEAR_ANKLE_L:
				case WEAR_ANKLE_R:
				case WEAR_FEET:
				case WEAR_HANDS:
				case WEAR_ARMS:
				case WEAR_SHOULDERS:
				case WEAR_SHIELD:
				case WEAR_ABOUT:
				case WEAR_WAIST:
				case WEAR_WRIST_L:
				case WEAR_WRIST_R:
				case WEAR_HOLD:
				case WEAR_FLOAT:
				  if( IS_OBJ_STAT(armor, ITEM_STAINLESS) )
				  {
				    if(num_range <= chance/2)
				    {
					if( is_chain(armor) || is_plate(armor) )
					{
						armor->condition -= 1;
						break;
					}
					else if( is_leather(armor) || !str_infix(armor->material, "leather") )
					{
						armor->condition -= 2;
						break;
					}
					else
					{
						armor->condition -= 3;
						break;
					}
				    }
				  }
				  else
				  {
				    if( !IS_OBJ_STAT(armor, ITEM_STAINLESS) )
				    {
					if( is_chain(armor) || is_plate(armor) )
					{
						armor->condition -= 1;
						break;
					}
					else if( is_leather(armor) || !str_infix(armor->material, "leather") )
					{
						armor->condition -= 2;
						break;
					}
					else
					{
						armor->condition -= 3;
						break;
					}
				    }
				  }

				/* No condition loss */
				case WEAR_LIGHT:
				case WEAR_WIELD:
				case WEAR_TATTOO:
				case WEAR_SECONDARY:
				case WEAR_MARK:
				default:
					break;
			} //end switch
			break;
		} //end FOR
	} //end condition code
    
    /* but do we have a funky weapon? */
    if (result && wield != NULL)
    { 
	int dam;

	if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_POISON))
	{
	    int level;
	    AFFECT_DATA *poison, af;

	    if ((poison = affect_find(wield->affected,gsn_poison)) == NULL)
		level = wield->level;
	    else
		level = poison->level;
	
	    if (!saves_spell(level / 2,victim,DAM_POISON)) 
	    {
		send_to_char("You feel poison coursing through your veins.\n\r",
		    victim);
		act("$n is poisoned by the venom on $p.",
		    victim,wield,NULL,TO_ROOM);

    		af.where     = TO_AFFECTS;
    		af.type      = gsn_poison;
    		af.level     = level * 3/4;
    		af.duration  = level / 2;
    		af.location  = APPLY_STR;
    		af.modifier  = -1;
    		af.bitvector = AFF_POISON;
    		affect_join( victim, &af );
	    }

	    /* weaken the poison if it's temporary */
	    if (poison != NULL)
	    {
	    	poison->level = UMAX(0,poison->level - 2);
	    	poison->duration = UMAX(0,poison->duration - 1);
	
	    	if (poison->level == 0 || poison->duration == 0)
		    act("The poison on $p has worn off.",ch,wield,NULL,TO_CHAR);
	    }
 	}


    	if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_VAMPIRIC))
	{
	    dam = number_range(1, wield->level / 5 + 1);
	    act("$p draws life from $n.",victim,wield,NULL,TO_ROOM);
	    act("You feel $p drawing your life away.",
		victim,wield,NULL,TO_CHAR);
	    damage(ch,victim,dam,0,DAM_NEGATIVE,FALSE);
	    ch->alignment = UMAX(-1000,ch->alignment - 1);
	    ch->hit += dam/2;
	}

	if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_FLAMING))
	{
	    dam = number_range(1,wield->level / 4 + 1);
	    act("$n is burned by $p.",victim,wield,NULL,TO_ROOM);
	    act("$p sears your flesh.",victim,wield,NULL,TO_CHAR);
	    fire_effect( (void *) victim,wield->level/2,dam,TARGET_CHAR);
	    damage(ch,victim,dam,0,DAM_FIRE,FALSE);
	}

	if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_CHAOTIC))
	{
	    if (!IS_NPC(victim) && victim->ethos == ETHOS_LAWFUL  )
	    {
	      dam = number_range(1,wield->level / 2 + 2);
	      act("$n is chaotically torn apart by $p.",victim,wield,NULL,TO_ROOM);
	      act("$p's chaotic properties tear you apart.",victim,wield,NULL,TO_CHAR);
	      damage(ch,victim,dam,0,DAM_OTHER,FALSE);
	    }
	}

	if (ch->fighting == victim 
      && IS_WEAPON_STAT(wield,WEAPON_BRILLIANT)
      && (victim->alignment > -750))
	{
	   dam = number_range(1,wield->level / 2 + 10);
	   act("$n is {rannihilated{x by a {Wbrilliant energy{X from $p.",victim,wield,NULL,TO_ROOM);
	   act("$p's {Wbrilliant energy{x annihilates you.",victim,wield,NULL,TO_CHAR);
	   damage(ch,victim,dam,0,DAM_ENERGY,FALSE);
	}

	if (ch->fighting == victim 
      && IS_WEAPON_STAT(wield,WEAPON_DISRUPTION) 
      && (victim->alignment <= -750))
	{
	   dam = number_range(1,wield->level + 2);
	   act("$n is sliced apart by $p.",victim,wield,NULL,TO_ROOM);
	   act("$p slices you apart.",victim,wield,NULL,TO_CHAR);
	   damage(ch,victim,dam,0,DAM_HOLY,FALSE);
	}

	if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_FLAMING_BURST))
	{
	   dam = number_range(1,wield->level / 2 + 20);
	   act("$n is scorched with bursts of {rfire{x by $p.",victim,wield,NULL,TO_ROOM);
	   act("$p unleashes bursts of {rfire{x, scorching your skin.",victim,wield,NULL,TO_CHAR);
	   damage(ch,victim,dam,0,DAM_FIRE,FALSE);
	}

	if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_SHOCKING_BURST))
	{
	   dam = number_range(1,wield->level / 2 + 20);
	   act("$n is struck with bursts of {wlightning{x by $p.",victim,wield,NULL,TO_ROOM);
	   act("$p unleashes bursts of {wlightning{x, striking you hard.",victim,wield,NULL,TO_CHAR);
	   damage(ch,victim,dam,0,DAM_LIGHTNING,FALSE);
	}

	if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_ICY_BURST))
	{
	   dam = number_range(1,wield->level / 2 + 20);
	   act("$n is blasted with bursts of {Bice{x by $p.",victim,wield,NULL,TO_ROOM);
	   act("$p blasts you with large bursts of {Bice{x.",victim,wield,NULL,TO_CHAR);
	   damage(ch,victim,dam,0,DAM_COLD,FALSE);
	}

	if (ch->fighting == victim
      && IS_WEAPON_STAT(wield,WEAPON_HOLY)
      && (victim->alignment <= -750))
	{
	   dam = number_range(1,wield->level / 2 + 2);
	   act("$n is struck down by the {YHoly Power{x of $p.",victim,wield,NULL,TO_ROOM);
	   act("$p's {YHoly Power{x strikes through your evil soul.",victim,wield,NULL,TO_CHAR);
	   damage(ch,victim,dam,0,DAM_HOLY,FALSE);
	}

	if (ch->fighting == victim
      && IS_WEAPON_STAT(wield,WEAPON_UNHOLY)
      && (victim->alignment > 750))
	{
	   dam = number_range(1,wield->level / 2 + 2);
	   act("$n is overcome with the {REvil Power{x of $p.",victim,wield,NULL,TO_ROOM);
	   act("$p's {REvil Powers{x overwhelm you with negative energy.",victim,wield,NULL,TO_CHAR);
	   damage(ch,victim,dam,0,DAM_NEGATIVE,FALSE);
	}

	if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_LAWFUL))
	{
	    if (!IS_NPC(victim) && victim->ethos == ETHOS_CHAOTIC  )
	    {
	      dam = number_range(1,wield->level / 2 + 2);
	      act("$n has been Judged by $p's Lawful Power.",victim,wield,NULL,TO_ROOM);
	      act("$p's lawful properties rip into your chaotic persona.",victim,wield,NULL,TO_CHAR);
	      damage(ch,victim,dam,0,DAM_OTHER,FALSE);
	    }
	}

	if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_THUNDERING))
	{
	      dam = number_range(1,wield->level / 2 + 2);
	      act("$n is blasted with {CSonic Energy{x by $p's cacophonous roar.",victim,wield,NULL,TO_ROOM);
	      act("$p's cacophonous roar blasts you with {CSonic Energy{x.",victim,wield,NULL,TO_CHAR);
	      damage(ch,victim,dam,0,DAM_LIGHTNING,FALSE);
	}

	if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_BSTAFF))
	{
	AFFECT_DATA af;
	 switch(number_range(0,30))    
       {
	  case 1:
 	  case 2:
	  case 3:
        act("$n's battle staff emits a charge of lightning.",ch,NULL,NULL,TO_ROOM);
        send_to_char("Your battle staff emits a charge of lightning.\n\r",ch);
        damage(ch,victim,number_range(2,ch->level),gsn_lightning,DAM_LIGHTNING, TRUE);
       break;
       case 4:
        act("$n's battle staff emits a blinding flash of light.",ch,NULL,NULL,TO_ROOM);
        send_to_char("Your battle staff emits a blinding flash of light.\n\r",ch);
       if (!saves_spell(ch->level, victim, DAM_LIGHTNING)
	 && (!IS_AFFECTED(ch,AFF_BLIND)))
       {
       af.where		= TO_AFFECTS;
       af.type     	= gsn_blindness;
       af.level     	= ch->level / 10;
       af.duration  	= 7;
       af.modifier  	= -4;
       af.location  	= APPLY_HITROLL;
       af.bitvector 	= AFF_BLIND;
       affect_to_char( victim, &af );
       act("The bright flash blinds $N!.", ch, NULL, ch->fighting, TO_CHAR);
       act("The bright flash blinds $N!.", ch, NULL, ch->fighting, TO_NOTVICT);
 	  }
 	  break;
	  case 5:
 	  if (!IS_AFFECTED(ch,AFF_HASTE))
 	  { 

   	   af.where		= TO_AFFECTS;
   	   af.type      	= skill_lookup("haste");  
   	   af.level     	= ch->level;
   	   af.duration  	= 5;
   	   af.modifier  	= 1 + (ch->level >= 18) + (ch->level >= 25) + (ch->level >= 32);
    	   af.location  	= APPLY_DEX;
   	   af.bitvector 	= AFF_HASTE;
   	   affect_to_char(ch,&af);

   	   act("$n's battle staff hums softly.",ch,NULL,NULL,TO_ROOM);
   	   send_to_char("Your battle staff hums softly.\n\r",ch);
   	   act("$n appears to be moving faster.",ch,NULL,NULL,TO_ROOM);
   	   send_to_char("You appear to be moving faster.\n\r",ch);
 	  }
  	break;
  	case 6:
 	 if ( !IS_AFFECTED(victim, AFF_INCENDIARY_CLOUD) )  
 	 {
   	   af.where		= TO_AFFECTS;
  	   af.type      	= skill_lookup("incendiary cloud");
 	   af.level     	= ch->level;
   	   af.duration   	= 10;
   	   af.modifier  	= ch->level *2;
    	   af.location  	= APPLY_AC;
    	   af.bitvector 	= AFF_INCENDIARY_CLOUD;
         affect_to_char( victim, &af );    

    	   act("$n's battle staff emits a pink light.",ch,NULL,NULL,TO_ROOM);
    	   send_to_char("Your battle staff emits a pink light.\n\r",ch);
   	   send_to_char( "You are surrounded by a pink outline.\n\r", ch->fighting );
   	   act( "$n is surrounded by a pink outline.", ch->fighting, NULL, NULL, TO_ROOM );

   	  }
  	  break;
  	  }
	}

	if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_DSTAFF))
	{
      AFFECT_DATA af;
      CHAR_DATA *vch;
      CHAR_DATA *vch_next;

        switch(number_range(0,25))
        {
          case 1:
          case 2:
          case 3:
            act("$n's druid staff emits a light.",ch,NULL,NULL,TO_ROOM);
            send_to_char("Your druid staff emits a blue light.\n\r",ch);

            for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
            {
              vch_next = vch->next_in_room;
              if (vch == ch || is_same_group(vch,ch) )
              {
                send_to_char("You feel better.\n\r",vch);
                vch->hit = UMIN( vch->hit + 25, vch->max_hit ); 
              }
            }
          break;
          case 4:
            if ( !IS_AFFECTED(victim, AFF_SLOW) )
            {
		  af.where		= TO_AFFECTS;
              af.type      	= skill_lookup("slow");
              af.level     	= ch->level;
              af.duration   	= 10;
              af.modifier  	= -1 - (ch->level >= 18) - (ch->level >= 25) - (ch->level >= 32); 
              af.location  	= APPLY_DEX;
              af.bitvector 	= AFF_SLOW;
              affect_to_char( victim, &af );

              act("$n's druid staff emits a purple light.",ch,NULL,NULL,TO_ROOM);
              send_to_char("Your druid staff emits a purple light.\n\r",ch);
              send_to_char( "You feel yourself moving more slowly\n\r", victim );
              act( "$n is moving more slowly.", victim, NULL, NULL, TO_ROOM);
            }
          break;
          case 5:
            if (!IS_AFFECTED(ch,AFF_HASTE))
            {

		  af.where		= TO_AFFECTS;
              af.type      	= skill_lookup("haste");
              af.level     	= ch->level;
              af.duration  	= 3;
              af.modifier  	= 1 + (ch->level >= 18) + (ch->level >= 25) + (ch->level >= 32);
              af.location  	= APPLY_DEX;
              af.bitvector 	= AFF_HASTE;
              affect_to_char(ch,&af);

              act("$n's druid staff hums softly.",ch,NULL,NULL,TO_ROOM);
              send_to_char("Your druid staff hums softly.\n\r",ch);
              act("$n appears to be moving faster.",ch,NULL,NULL,TO_ROOM);
              send_to_char("You appear to be moving faster.\n\r",ch);
            }
          break;
  	    case 6:
    		if (!IS_AFFECTED(victim,AFF_WEAKEN))
    		{
		  af.where		= TO_AFFECTS;
    		  af.type      	= skill_lookup("weaken");
    		  af.level     	= ch->level;
     		  af.duration  	= 10;
    		  af.location  	= APPLY_STR;
    		  af.modifier  	= -1 * (ch->level / 5);
    		  af.bitvector 	= AFF_WEAKEN;
    		  affect_to_char( victim, &af );

    		  act("$n's druid staff emits a purple light.",ch,NULL,NULL,TO_ROOM);
    		  send_to_char("Your druid staff emits a purple light.\n\r",ch);
    		  send_to_char( "You feel weaker.\n\r", victim );
      	  act("$n looks tired and weak.",victim,NULL,NULL,TO_ROOM);
  		}
  	    break;
          }
      } 

	if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_FROST))
	{
	    dam = number_range(1,wield->level / 6 + 2);
	    act("$p freezes $n.",victim,wield,NULL,TO_ROOM);
	    act("The cold touch of $p surrounds you with ice.",
		victim,wield,NULL,TO_CHAR);
	    cold_effect(victim,wield->level/2,dam,TARGET_CHAR);
	    damage(ch,victim,dam,0,DAM_COLD,FALSE);
	}

  	if (ch->fighting == victim && IS_OBJ_STAT(wield,ITEM_DEATHBLADE)) 
 	{ 
  	    dam = number_range(1,ch->level / 4 + 1);  /* amount of extra damage */
  	    act("$n is slashed by $p's edge of Death.",victim,wield,NULL,TO_ROOM); 
  	    act("$p slashes you with a deadly edge of Death.",victim,wield,NULL,TO_CHAR);
  	    fire_effect( (void *) victim,ch->level/2,dam,TARGET_CHAR);
  	    damage(ch,victim,dam,0,DAM_FIRE,FALSE); 
  	}

	if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_SHOCKING))
	{
	    dam = number_range(1,wield->level/5 + 2);
	    act("$n is struck by lightning from $p.",victim,wield,NULL,TO_ROOM);
	    act("You are shocked by $p.",victim,wield,NULL,TO_CHAR);
	    shock_effect(victim,wield->level/2,dam,TARGET_CHAR);
	    damage(ch,victim,dam,0,DAM_LIGHTNING,FALSE);
	}

        if (ch->fighting == victim && IS_OBJ_STAT(wield,ITEM_DEATH))
	{
            dam = number_range(3,wield->level / 4 + 4);
            act("The evil power of $p torments $n.",victim,wield,NULL,TO_ROOM);
            act("The evil power of $p torments you.",victim,wield,NULL,TO_CHAR);
            damage(ch,victim,dam,0,DAM_NEGATIVE,FALSE);

            if ( !IS_NPC(victim) && ( victim->alignment > 0 )  )
                victim->alignment -= 1;
	}

    }
    tail_chain( );
    return;
}


/*
 * Inflict damage from a hit.
 */
bool damage(CHAR_DATA *ch,CHAR_DATA *victim,int dam,int dt,int dam_type, bool show) 
{
   char buf[MAX_STRING_LENGTH];
   OBJ_DATA *corpse;
   bool immune;

   if ( victim->position == POS_DEAD )
	return FALSE;

   if (IS_AFFECTED3(victim, AFF_SUBDUE)
   &&  IS_SET(ch->act,PLR_AUTOMERCY))
   {
     send_to_char("Not on someone already subdued!\n\r", ch);
     stop_fighting(victim, TRUE);

     if (IS_AFFECTED(victim, AFF_SLEEP))
	 victim->position = POS_SLEEPING;

     return FALSE;
   }

   if (!IS_NPC(ch) && !IS_NPC(victim)
   && (ABS(ch->level - victim->level) > 10)
   && (ch->level <= LEVEL_HERO)
   && IS_AFFECTED3(victim, AFF_SUBDUE))
   {
     send_to_char("Not on someone so weak compared to you!\n\r",ch);
     stop_fighting(victim, TRUE);

     if (IS_AFFECTED(victim, AFF_SLEEP))
       victim->position = POS_SLEEPING;

     return FALSE;
   }

   if( IS_AFFECTED2(victim, AFF_INVUL))
   {
	stop_fighting(victim, TRUE);
      return FALSE;
   }

    /*
     * Stop up any residual loopholes.
     */
    if ( dam > 1200 && dt >= TYPE_HIT)
    {
	bug( "Damage: %d: more than 1200 points!", dam );
	dam = 1200;
      if (ch->fighting != NULL)
      {
	OBJ_DATA *obj;
	obj = get_eq_char( ch, WEAR_WIELD );
      send_to_char("Your weapon did so much damage, it shatters into tiny fragments.\n\r",ch);
	if (obj != NULL)
	  extract_obj(obj);
      }
    }

    /* damage reduction */
     if ( dam > 200)
       dam = (dam - 200)/2 + 200;

     if ( dam > 1400)
       dam = (dam - 300)/3 + 100;

    /*
     * Lets make that damaged equipment disappear
     */
    if (ch->fighting != NULL)
    {
	OBJ_DATA *obj;
     	int iWear;
     	for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
     	{
       obj = get_eq_char( ch, iWear );
	 if(obj == NULL)
	  continue;

       if ( obj->condition <= 0 )
 	 {
	   if (obj->item_type == ITEM_WEAPON)
	   {
          sprintf(buf, "\n\r-> Excessive damage to $p causes it to shatter into pieces.");
          act( buf, ch, obj, NULL, TO_CHAR );
	   }
         else
         {
          sprintf(buf, "\n\r-> Due to excessive damage, $p falls off your body in pieces.");
          act( buf, ch, obj, NULL, TO_CHAR );
         }
         if (obj != NULL)
	   extract_obj(obj);
       }
     	}
    }
   
    if ( victim != ch )
    {
	/*
	 * Certain attacks are forbidden.
	 * Most other attacks are returned.
	 */
	if ( is_safe( ch, victim ) )
	    return FALSE;
	check_killer( ch, victim );

	if ( victim->position > POS_STUNNED )
	{
	    if ( victim->fighting == NULL )
		{
		set_fighting( victim, ch );
            if ( IS_NPC ( victim ) && HAS_TRIGGER_MOB ( victim, TRIG_KILL ) )
               p_percent_trigger ( victim, NULL, NULL, ch, NULL, NULL, TRIG_KILL );
		}
	}

        if ( victim->position > POS_STUNNED )
        {
            if ( ch->fighting == NULL )
                set_fighting( ch, victim );
            /*
             * If victim is charmed, ch might attack victim's master.
             */
            if ( IS_NPC(ch)
            &&   IS_AFFECTED(victim, AFF_CHARM)
            &&   victim->master != NULL
            &&   victim->master->in_room == ch->in_room
            &&   number_bits( 3 ) == 0 )
            {
                stop_fighting( ch, FALSE );
                multi_hit( ch, victim->master, TYPE_UNDEFINED );
                return FALSE;
            }
        }

	/*
	 * More charm stuff.
	 */
	if ( victim->master == ch )
	    stop_follower( victim );
    }

    /*
     * Inviso attacks ... not.
     */

    if ( IS_AFFECTED2(ch, AFF_WRAITHFORM) )
    {
	affect_strip( ch, gsn_wraithform);
      REMOVE_BIT( ch->affected2_by, AFF_WRAITHFORM ); /*wraith has*/
      REMOVE_BIT( ch->affected2_by, AFF_WRAITHFORM ); /*two applied*/
      act( "$n looks solid and heavy again as the wraithlike form diminishes.", ch, NULL, NULL, TO_ROOM );
      act( "You feel solid and heavy again as your wraithlike form diminishes.", ch, NULL, NULL, TO_CHAR );
    }

    if ( IS_AFFECTED(ch, AFF_INVISIBLE) )
    {
	affect_strip( ch, gsn_invis );
	affect_strip( ch, gsn_mass_invis );
	REMOVE_BIT( ch->affected_by, AFF_INVISIBLE );
	act( "$n fades into existence.", ch, NULL, NULL, TO_ROOM );
    }

    if ( IS_AFFECTED(ch, AFF_HIDE) )
    {
      REMOVE_BIT( ch->affected_by, AFF_HIDE );
      act( "$n steps out of the shadows.", ch, NULL, NULL, TO_ROOM );
      act( "You step out of the shadows.", ch, NULL, NULL, TO_CHAR );
    }

    if ( IS_AFFECTED2(ch, AFF_CAMOUFLAGE) )
    {
        REMOVE_BIT( ch->affected2_by, AFF_CAMOUFLAGE );
        act( "$n makes $mself known.", ch, NULL, NULL, TO_ROOM );
        act( "You make yourself known to others.", ch, NULL, NULL, TO_CHAR );
    }

    if ( IS_AFFECTED2(victim, AFF_PARALYSIS) )
    {
	  affect_strip(victim,skill_lookup("paralysis"));
        REMOVE_BIT( victim->affected2_by, AFF_PARALYSIS );
    }

    if ( IS_AFFECTED3(victim, AFF_HOLDPERSON) )
    {
        affect_strip(victim,skill_lookup("dazzle"));
        affect_strip(victim,skill_lookup("hypnotic pattern"));
        REMOVE_BIT(victim->affected3_by,AFF_HOLDPERSON);
    }

    if (is_affected (victim, gsn_lull))
    {
      affect_strip( victim, gsn_lull );
    }

    check_cpose(ch);

    /*
     * Damage modifiers.
     */

    if ( dam > 1 && !IS_NPC(victim) 
    &&   victim->pcdata->condition[COND_DRUNK]  > 10 )
	dam = 9 * dam / 10;

   if ( dam > 1 && IS_AFFECTED(victim, AFF_SANCTUARY) )
	dam /= 2;

   if ( IS_AFFECTED2(victim, AFF_DARK_TAINT)) 
   {
      if (dam_type == DAM_NEGATIVE)
          dam += dam/4;
   }

   if( IS_AFFECTED2(victim, AFF_MOONCLOAK) )
   {
	if (dam_type == DAM_NEGATIVE)
        dam /= 4;
   	if (dam_type == DAM_COLD)
	  dam /= 4;
   }

   if (victim->in_room != NULL)
   {  

    if (( IS_AFFECTED2(victim, AFF_NATURE)
    && ((victim->in_room->sector_type == SECT_FOREST)
    || ( victim->in_room->sector_type == SECT_FIELD)
    || ( victim->in_room->sector_type == SECT_DESERT)
    || ( victim->in_room->sector_type == SECT_HILLS)
    || ( victim->in_room->sector_type == SECT_POLAR)
    || ( victim->in_room->sector_type == SECT_MOUNTAIN)))
    && (!IS_AFFECTED(victim, AFF_SANCTUARY)))
        dam /= 2;
  }

    if ( dam > 1 && ((IS_AFFECTED(victim, AFF_PROTECT_EVIL) && IS_EVIL(ch) )
    ||		     (IS_AFFECTED(victim, AFF_PROTECT_GOOD) && IS_GOOD(ch) )))
	dam -= dam / 4;

    if (victim == ch->guarding || victim == ch->guarded)
    {
          ch->guarded = NULL;
          ch->guarding = NULL;
          victim->guarded = NULL;
          victim->guarding = NULL;
    }

    immune = FALSE;

    /*
     * Check for counter, dodge, parry, and shield block
     */
    if ( dt >= TYPE_HIT && ch != victim)
	{
      if ( check_parry( ch, victim ) )
	    return FALSE;
	if ( check_dodge( ch, victim ) )
	    return FALSE;
      if (check_evade( ch, victim) )
          return FALSE;
      if (check_sidestep( ch, victim) )
          return FALSE;
      if (check_pivot( ch, victim) )
          return FALSE;
      if (check_redirect( ch, victim) )
          return FALSE;
      if (check_riposte( ch, victim) )
          return FALSE;
      if (check_acrobatics( ch, victim) )
          return FALSE;
	if ( check_shield_block(ch,victim))
	    return FALSE;
      if ( check_displacement( ch, victim) )
          return FALSE;
      if (check_tumble( ch, victim) )
          return FALSE;
      if (check_guard( ch, victim) )
          return FALSE;
	}

    switch(check_immune(victim,dam_type))
    {
	case(IS_IMMUNE):
	    immune = TRUE;
	    dam = 0;
	    break;
	case(IS_RESISTANT):	
	    dam -= dam/3;
	    break;
	case(IS_VULNERABLE):
	    dam += dam/2;
	    break;
    }

    /*
     * Hurt the victim.
     * Inform the victim of his new state.
     */

    if (show)
    	dam_message( ch, victim, dam, dt, immune );

    if (dam == 0)
	return FALSE;

   victim->hit -= dam;

    if ( !IS_NPC(victim)
    &&   victim->level >= LEVEL_AVATAR
    &&   victim->hit < 1 )
	victim->hit = 1;

    if (victim->hit <= 0)
    {
      if((( IS_SET(ch->act,PLR_AUTOMERCY)
      && !IS_NPC(victim))
      || (IS_NPC(ch)
      && (ch->master != NULL)
      && IS_SET(ch->master->act, PLR_AUTOMERCY)
      && !IS_NPC(victim)
      && ch->in_room == ch->master->in_room))
      && (victim != ch))
      {
        mercy_kill(ch, victim);
        return TRUE;
      }

	if (IS_NPC(ch)
      && (ch->leader != NULL)
      && IS_SET(ch->leader->act, PLR_AUTOMERCY)
      && !IS_NPC(victim)
      && (ch->in_room == ch->leader->in_room)
      && (victim != ch))
      {
        mercy_kill(ch, victim);
        return TRUE;
      }

      if (!IS_NPC(ch)
      && !IS_NPC(victim)
      && ABS(ch->level - victim->level) > 10
	&& ch->level <= LEVEL_HERO)
      {
        mercy_kill(ch, victim);
        return TRUE;
      }

      if (IS_NPC(ch)
      && !IS_NPC(victim)
      && (ch->master != NULL)
      && (ch->in_room == ch->master->in_room)
      && ABS(ch->master->level - victim->level) > 10)
      {
        mercy_kill(ch, victim);
        return TRUE;
	}

      if (IS_NPC(ch)
      && !IS_NPC(victim)
      && (ch->leader != NULL)
      && (ch->in_room == ch->leader->in_room)
      && ABS(ch->leader->level - victim->level) > 10)
      {
        mercy_kill(ch, victim);
        return TRUE;
      }
    }

    update_pos( victim );

    switch( victim->position )
    {
    case POS_MORTAL:
	act( "{R$n is mortally wounded, and will die soon, if not aided.{x",
	    victim, NULL, NULL, TO_ROOM );
	send_to_char( 
	    "{RYou are mortally wounded, and will die soon, if not aided.{x\n\r",
	    victim );
	break;

    case POS_INCAP:
	act( "{R$n is incapacitated and will slowly die, if not aided.{x",
	    victim, NULL, NULL, TO_ROOM );
	send_to_char(
	    "{RYou are incapacitated and will slowly die, if not aided.{x\n\r",
	    victim );
	break;

    case POS_STUNNED:
	act( "$n is stunned, but will probably recover.",
	    victim, NULL, NULL, TO_ROOM );
	send_to_char("{CYou are stunned, but will probably recover.{x\n\r",
	    victim );
	break;

    case POS_DEAD:
	  if((( IS_SET(ch->act,PLR_AUTOMERCY)
        && !IS_NPC(victim))
	  || (IS_NPC(ch)
        && (ch->master != NULL) 
	  && IS_SET(ch->master->act, PLR_AUTOMERCY)
        && !IS_NPC(victim)
	  && ch->in_room == ch->master->in_room))
	  && (victim != ch))
	  {
	    mercy_kill(ch, victim);
	    return TRUE;
	  }

        if (!IS_NPC(ch) && !IS_NPC(victim)
	  && ABS(ch->level - victim->level) > 10
	  && ch->level <= LEVEL_HERO)
        {
          mercy_kill(ch, victim);
          return TRUE;
        }

	  if (IS_NPC(ch)
        && !IS_NPC(victim)
        && (ch->master != NULL)
	  && (ch->in_room == ch->master->in_room)
	  && ABS(ch->master->level - victim->level) > 10
	  && ch->master->level <= LEVEL_HERO)
	  {
          mercy_kill(ch, victim);
          return TRUE;
        }

        if ( get_skill(victim, gsn_heroic) >= 1 )
        {
          if (number_percent( ) < get_skill(victim,gsn_heroic))
          {
            if (( number_percent( ) < 50 )
            && !IS_NPC(victim)
            && (ch->hit > ch->max_hit/10))
            {
              send_to_char("\n\r",victim );
              send_to_char("In one final desperate attempt to hurt your enemy, you pull out a small\n\r",victim );
              send_to_char("dagger and lunge with it, striking a critical blow before you fall to\n\r",victim );
              send_to_char("your own death!\n\r",victim );
              send_to_char("\n\r",victim );
              act( "In one final heroic like move, $N strikes you with a critical blow before.", ch, NULL, victim, TO_CHAR    );
              act( "falling to $S death!", ch, NULL, victim, TO_CHAR    );
              ch->hit = (ch->max_hit/10);
              check_improve(victim,gsn_heroic,TRUE,2);
            }
          }
        }
	act( "$n is DEAD!!", victim, 0, 0, TO_ROOM );
	send_to_char( "{MYou have been {RKILLED!!{x\n\r\n\r", victim );
      if ( victim->rider )
      {
        act( "$n dies suddenly, and you fall to the ground.", 
	  victim, NULL, victim->rider, TO_VICT );
        victim->rider->riding	  = NULL;
        victim->rider->position = POS_RESTING;
        victim->rider		  = NULL;
      }

	if (!IS_NPC(ch)
	&& !IS_NPC(victim)
	&& (victim->org_id != ORG_NONE)
	&& (ch->org_id != ORG_NONE)
	&& (str_cmp(ch->in_room->area->name, "Palanthas"))
	&& (str_cmp(ch->in_room->area->name, "Neraka"))
	&& (str_cmp(ch->in_room->area->name, "Neraka2")))
	{
	  if((victim->org_id == ORG_KOT)
	  && (ch->org_id == ORG_SOLAMNIC)
	  && (ch->in_room->area->faction != 0)
	  && (ch->in_room->area->faction != 250))
	  {
          if((ch->in_room->area->faction >= -2)
	    &&(ch->in_room->area->faction < 0))
	    {
		ch->in_room->area->faction = 1;
	    }
	    else
          if(ch->in_room->area->faction >= 249)
	    {
		ch->in_room->area->faction = 250;
	    }
	    else
	    {
	      ch->in_room->area->faction += 2;
	    }

	    send_to_char("Control over these lands have gained a little favor for the Solamnic Knights!\n\r",ch);
          if (!IS_SET(ch->in_room->area->area_flags, AREA_CHANGED))
          {
            SET_BIT( ch->in_room->area->area_flags, AREA_CHANGED );
	    }
	  }

	  if((victim->org_id == ORG_KOT)
	  && (ch->org_id == ORG_CONCLAVE)
	  && (ch->in_room->area->faction != 0)
	  && (ch->in_room->area->faction != 250)
	  && (number_percent() <= 50))
	  {
          if((ch->in_room->area->faction >= -1)
	    &&(ch->in_room->area->faction < 0))
	    {
		ch->in_room->area->faction = 1;
	    }
	    else
          if(ch->in_room->area->faction >= 249)
	    {
		ch->in_room->area->faction = 250;
	    }
	    else
	    {
	      ch->in_room->area->faction += 1;
	    }

	    send_to_char("Control over these lands have gained a little favor for the Solamnic Knights!\n\r",ch);
	  }

	  if((victim->org_id == ORG_SOLAMNIC)
	  && (ch->org_id == ORG_KOT)
	  && (ch->in_room->area->faction != 0)
	  && (ch->in_room->area->faction != -250))
	  {
          if((ch->in_room->area->faction <= 2)
	    &&(ch->in_room->area->faction > 0))
	    {
		ch->in_room->area->faction = -1;
	    }
	    else
          if(ch->in_room->area->faction <= -249)
	    {
		ch->in_room->area->faction = -250;
	    }
	    else
	    {
	      ch->in_room->area->faction -= 2;
	    }

	    send_to_char("Control over these lands have gained a little favor for the Knights of Takhisis!\n\r",ch);
	  }

	  if((victim->org_id == ORG_SOLAMNIC)
	  && (ch->org_id == ORG_BLACKORDER)
	  && (ch->in_room->area->faction != 0)
	  && (ch->in_room->area->faction != -250)
	  && (number_percent() <= 50))
	  {
          if((ch->in_room->area->faction <= 1)
	    &&(ch->in_room->area->faction > 0))
	    {
		ch->in_room->area->faction = -1;
	    }
	    else
          if(ch->in_room->area->faction <= -249)
	    {
		ch->in_room->area->faction = -250;
	    }
	    else
	    {
	      ch->in_room->area->faction -= 1;
	    }

	    send_to_char("Control over these lands have gained a little favor for the Knights of Takhisis!\n\r",ch);
	  }
	}

	if( !IS_NPC ( victim ) )
      {
      act( "A spirit appears to rise from the lifeless corpse of $n.\n\r",victim, 0, 0, TO_ROOM );
      send_to_char("Your spirit rises and hovers above your lifeless corpse while you\n\r",victim );
      send_to_char("wait for a new body to arrive.\n\r",victim );
      }
	break;

    default:
	if ( dam > victim->max_hit / 4 )
	    send_to_char( "That really did {RHURT!{x\n\r", victim );
	if ( victim->hit < victim->max_hit / 4 )
	    send_to_char( "You sure are {RBLEEDING!{x\n\r", victim );
	break;
    }

    /*
     * Sleep spells and extremely wounded folks.
     */
    if ( !IS_AWAKE(victim) )
	stop_fighting( victim, FALSE );

    /*
     * Payoff for killing things.
     */
    if ( victim->position == POS_DEAD )
    {
	group_gain( ch, victim );

	if ( !IS_NPC(victim) )
	{
	    sprintf( log_buf, "%s killed by %s at %d",
		victim->name,
		(IS_NPC(ch) ? ch->short_descr : ch->name),
		ch->in_room->vnum );
	    log_string( log_buf );

		/*
             * Dying penalty if killed by MOB (no penalty for PK):
             * 1/2 of exp gain needed per level at current level
             * This allows xp deficit for the inept.
             */
            if( IS_NPC( ch ) )
		  gain_exp( victim, -(victim->level*100) );

		/*
             * Dying penalty if killed by another Player
             * Installed to penalize people having a friend
             * kill them to remove FLAGS
             */
            if( (!IS_NPC(ch))
            &&  (IS_SET(victim->act, PLR_KILLER)
	      ||   IS_SET(victim->act, PLR_THIEF)
	      ||   IS_SET(victim->act, PLR_ASSAULT)
	      ||   IS_SET(victim->act, PLR_TREASON)
	      ||   IS_SET(victim->act, PLR_OUTLAW)) )
               gain_exp( victim, -(victim->level*1000) );

	}

      sprintf( log_buf, "%s got toasted by %s at %s [room %d]",
      	(IS_NPC(victim) ? victim->short_descr : victim->name),
            (IS_NPC(ch) ? ch->short_descr : ch->name),
            ch->in_room->name, ch->in_room->vnum);
 
      if (IS_NPC(victim))
      	wiznet(log_buf,NULL,NULL,WIZ_MOBDEATHS,0,0);
      else
            wiznet(log_buf,NULL,NULL,WIZ_DEATHS,0,0);


	/* Begin PK Logger */
	if(!IS_NPC(victim) && ch != victim)
	{
		CHAR_DATA *iPked;

		iPked = ch;

		/* Follow charm thread to responsible character. */
    		while ( IS_AFFECTED(iPked, AFF_CHARM) && iPked->master != NULL )
			iPked = iPked->master;

		/* Log both Cheap PKILL via your charmie and Regular PKILL */
		if( !IS_NPC(iPked) || !IS_NPC(ch) )
		{ 
			FILE *fp;
			char *strtime = NULL;

  			/* write to the pk log */
    			fclose(fpReserve);
    			if( (fp = fopen(PK_LOG, "a")) == NULL)
    			{
				bug("Error opening pk_log.txt",0);
				fclose(fp);
				fpReserve = fopen(NULL_FILE, "r");
    			}
			else /* have to do it like this, otherwise, we'll probably go bonkers */
			{    /* if we error, because we can't use return; */

				sprintf( log_buf, "%s was killed by %s at room %d",
      				victim->name, ch->name, ch->in_room->vnum);

    				strtime = ctime(&current_time);
    				strtime[strlen(strtime)-1] = '\0';

				sprintf(buf, "%s :: %s\n", strtime, log_buf);
    				fprintf(fp, "%s", buf);
    				fclose(fp);
    				fpReserve = fopen(NULL_FILE, "r");
			}
		}
	 } /* End PK Logger */

	/*
	 * Death trigger
	 */
	if ( IS_NPC( victim ) && HAS_TRIGGER_MOB( victim, TRIG_DEATH) )
	{
	    victim->position = POS_STANDING;
	    p_percent_trigger( victim, NULL, NULL, ch, NULL, NULL, TRIG_DEATH );
	}
        /* Make the head for a bountied PC */
        if(!IS_NPC(victim) && !IS_NPC(ch)
        && is_bountied(victim))
        {
            char buf[MAX_STRING_LENGTH];
            char *name;
            OBJ_DATA *obj;

            name = str_dup(victim->name);
            obj = create_object(get_obj_index(OBJ_VNUM_SEVERED_HEAD),0);

            obj->extra_flags |= ITEM_NODROP|ITEM_NOUNCURSE;

            sprintf(buf, "%s %s", "head", name);
            free_string( obj->name );
            obj->name = str_dup( buf);

            sprintf( buf, obj->short_descr, name );
            free_string( obj->short_descr );
            obj->short_descr = str_dup( buf );

            sprintf( buf, obj->description, name );
            free_string( obj->description );
            obj->description = str_dup( buf );

            obj_to_char(obj,ch);
            free_string(name);
        }

      if (IS_NPC(ch)
      && !IS_NPC(victim)
      && (ch->spec_fun == spec_lookup("spec_orgguard"))
      && (ch->org_id != ORG_NONE)
      && !SAME_ORG(ch,victim))
      {
	  sprintf(buf, "%s %d", victim->name, 3054);
        do_transfer(victim, buf);
      }

	raw_kill( ch, victim );
        /* dump the flags */
        if (ch != victim && !IS_NPC(ch))
        {
            if (IS_SET(victim->act,PLR_KILLER))
                REMOVE_BIT(victim->act,PLR_KILLER);
            else
                REMOVE_BIT(victim->act,PLR_THIEF);
        }

        /* RT new auto commands */

	if (!IS_NPC(ch)
	&&  (corpse = get_obj_list(ch,"corpse",ch->in_room->contents)) != NULL
	&&  corpse->item_type == ITEM_CORPSE_NPC && can_see_obj(ch,corpse))
	{
	    OBJ_DATA *coins;

	    corpse = get_obj_list( ch, "corpse", ch->in_room->contents ); 

	    if ( IS_SET(ch->act, PLR_AUTOLOOT) &&
		 corpse && corpse->contains) /* exists and not empty */
            {
		do_function(ch, &do_get, "all corpse");
	    }

 	    if (IS_SET(ch->act,PLR_AUTOGOLD) &&
	        corpse && corpse->contains  && /* exists and not empty */
		!IS_SET(ch->act,PLR_AUTOLOOT))
	    {
		if ((coins = get_obj_list(ch,"gcash",corpse->contains))
		     != NULL)
		{
		    do_function(ch, &do_get, "all.gcash corpse");
	      	}
	    }
            
	    if (IS_SET(ch->act, PLR_AUTOSAC))
	    {
       	        if (IS_SET(ch->act,PLR_AUTOLOOT) && corpse && corpse->contains)
       	      	{
		    return TRUE;  /* leave if corpse has treasure */
	      	}
	        else
		{
		    do_function(ch, &do_sacrifice, "corpse");
		}
	    }
	}

	return TRUE;
    }

    if ( victim == ch )
	return TRUE;

    /*
     * Take care of link dead people.
     */
    if ( !IS_NPC(victim) && victim->desc == NULL )
    {
	if ( number_range( 0, victim->wait ) == 0 )
	{
	    do_function(victim, &do_recall, "" );
	    return TRUE;
	}
    }

    /*
     * Wimp out?
     */
    if ( IS_NPC(victim) && dam > 0 && victim->wait < PULSE_VIOLENCE / 2)
    {
	if ( ( IS_SET(victim->act, ACT_WIMPY) && number_bits( 2 ) == 0
	&&   victim->hit < victim->max_hit / 5) 
	||   ( IS_AFFECTED(victim, AFF_CHARM) && victim->master != NULL
	&&     victim->master->in_room != victim->in_room ) )
	{
	    do_function(victim, &do_flee, "" );
	    victim->hunting = NULL;
	}
    }

    if ( !IS_NPC(victim)
    &&   victim->hit > 0
    &&   victim->position != POS_DEAD
    &&   victim->hit <= victim->wimpy
    &&   victim->wait < PULSE_VIOLENCE / 2 )
    {
	do_function (victim, &do_flee, "" );
    }

    if ( !IS_NPC(victim) 
    && ( !IS_NPC(ch) && IS_SET(ch->act,PLR_AUTOYIELD))
    &&   victim->hit > 0
    &&   victim->position != POS_DEAD
    &&   victim->hit <= victim->yield
    &&   victim->wait < PULSE_VIOLENCE / 2 )
	do_yield(victim, "" );

    if (ch->fight_pos == FIGHT_REAR)
    {
	check_assist(ch, victim);
	stop_fighting(ch, FALSE);
    }
    tail_chain( );
    return TRUE;
}

bool is_safe(CHAR_DATA *ch, CHAR_DATA *victim)
{

    if (victim->in_room == NULL || ch->in_room == NULL)
	return TRUE;

    if (victim->fighting == ch || victim == ch)
	return FALSE;

    if (IS_IMMORTAL(ch) && ch->level > LEVEL_AVATAR)
	return FALSE;

    /* safe room? */ //C072
    if (IS_SET(ch->in_room->room_flags, ROOM_SAFE)
    && !IS_IMMORTAL(ch)
    && !IS_SET(victim->act, PLR_KILLER)
    && !IS_SET(victim->act, PLR_THIEF)
    && !IS_SET(victim->act, PLR_TREASON)
    && !IS_SET(victim->act, PLR_OUTLAW)
    && !IS_SET(victim->act, PLR_ASSAULT))
    {
        send_to_char( "The gods prevent fighting here.\n\r", ch);
        return TRUE;
    }

    if( IS_AFFECTED2(ch, AFF_INVUL) )
    {
        send_to_char( "You are invulnerable.\n\r", ch );
        return TRUE;
    }

    if( IS_AFFECTED3(victim, AFF_SUBDUE)
    && !IS_SET(ch->act,PLR_AUTOMERCY))
        return FALSE;

    if ( IS_AFFECTED(ch, AFF_SHACKLES))
    {
	  send_to_char("You cannot attack while you are bound by shackles!\n\r",ch);
	  return TRUE;
    }
    
    if( IS_AFFECTED2(victim, AFF_INVUL) || victim->position == POS_DEAD)
    {
        act( "$N is invulnerable.", ch, NULL, victim, TO_CHAR );
        act( "You are invulnerable to $n.", ch, NULL, victim, TO_VICT );
        if (IS_NPC(victim))
        {
          victim->hunting = NULL;
        }
        return TRUE;
    }

    /* killing mobiles */
    if (IS_NPC(victim))
    {

	if (victim->pIndexData->pShop != NULL)
	{
	    send_to_char("The shopkeeper wouldn't like that.\n\r",ch);
	    return TRUE;
	}

	/* no killing healers, trainers, etc */
	if (IS_SET(victim->act,ACT_TRAIN)
	||  IS_SET(victim->act,ACT_PRACTICE)
	||  IS_SET(victim->act,ACT_IS_HEALER)
	||  IS_SET(victim->act,ACT_IS_CHANGER)
	||  IS_SET(victim->act,ACT_BANKER))
	{        
       send_to_char("I don't think the Gods would approve.\n\r",ch);
	 return TRUE;
	}
    }

    if (IS_NPC(ch))
    {
        /* charmed mobs and pets cannot attack players outside owner's PK */
        if (!IS_NPC(victim) && (IS_AFFECTED(ch,AFF_CHARM))
                            &&  IS_SET(ch->act,ACT_PET))
        {
            CHAR_DATA *master;
            master = ch->master;
            if( master == NULL 
             || master->level < 12
             || victim->level < 12 )
                return TRUE;
            else
                return FALSE;
        }
     }
     else /* Not NPC */
     {  
        if (IS_IMMORTAL(ch))
            return FALSE;

        /* from here, NPCs are all killable */
        if(IS_NPC(victim) || IS_NPC(ch))
            return FALSE;

        /* make sure they're high enough level to begin PK'ing */
        if(ch->level < 12)
        {
		send_to_char( "You must reach at least level 12 before you can PK anyone.\n\r", ch);
            return TRUE;
        }

        /* check range on players */
        if ((victim->level < 12) && !IS_NPC(victim))
	  {
            act("The gods protect $N from you, pick on someone your own size.\n\r",
                ch, NULL, victim, TO_CHAR );
            act("The gods protect you from $n.\n\r",
                ch, NULL, victim, TO_VICT);
            act("The gods protect $N from $n.\n\r",
                ch, NULL, victim, TO_NOTVICT);
            return TRUE;
        }

	  /* PK privileges removed? */
	  if(IS_SET(ch->comm, COMM_NOPKILL) && ch->fighting == NULL) //lets the ch defend
	  {
		send_to_char("The gods prevent you from PKing anyone.\n\r",ch);
		return TRUE;
	  }

	  /* Removed PK restrictions for now... 
        if( !IS_NPC(victim) && !IS_IMMORTAL(ch))
        {
            if( victim->level < 12
             || ch->level < 12
             || ABS( victim->level - ch->level ) > 12 )
             {
               if (can_see( ch, victim ) )
                act("The gods protect you from $N's attempted attack.",ch,NULL,victim,TO_CHAR);
               if (can_see(victim,ch))
                act("The gods protect $n from your attempted attack.",ch,NULL,victim,TO_VICT);
               return TRUE;
             }
        }		*/

        return FALSE;
    }
  return FALSE;
}
 
bool is_safe_spell(CHAR_DATA *ch, CHAR_DATA *victim, bool area )
{
    if (victim->in_room == NULL || ch->in_room == NULL)
        return TRUE;

    if (victim == ch && area)
	return TRUE;

    if (victim->fighting == ch || victim == ch)
	return FALSE;

    if (IS_IMMORTAL(ch) && ch->level > LEVEL_AVATAR && !area)
	return FALSE;

    /* killing mobiles */
    if (IS_NPC(victim))
    {
	/* safe room? */
	if (IS_SET(victim->in_room->room_flags,ROOM_SAFE))
	    return TRUE;

	if (victim->pIndexData->pShop != NULL)
	    return TRUE;

	/* no killing healers, trainers, etc */
	if (IS_SET(victim->act,ACT_TRAIN)
	||  IS_SET(victim->act,ACT_PRACTICE)
	||  IS_SET(victim->act,ACT_IS_HEALER)
	||  IS_SET(victim->act,ACT_IS_CHANGER)
	||  IS_SET(victim->act,ACT_BANKER))
	    return TRUE;

    /* either party invul */
    if( IS_AFFECTED2(ch, AFF_INVUL) || IS_AFFECTED2(victim, AFF_INVUL) )
        return TRUE;

	if (!IS_NPC(ch))
	{
	    /* no pets */
	    if (IS_SET(victim->act,ACT_PET))
	   	return TRUE;

	    /* no charmed creatures unless owner */
	    if (IS_AFFECTED(victim,AFF_CHARM) && (area || ch != victim->master))
		return TRUE;

	    /* legal kill? -- cannot hit mob fighting non-group member */
	    if (victim->fighting != NULL && !is_same_group(ch,victim->fighting))
		return TRUE;
	}
	else
	{
	    /* area effect spells do not hit other mobs */
	    if (area && !is_same_group(victim,ch->fighting))
		return TRUE;
	}
    }
    /* killing players */
    else
    {
	if (area && IS_IMMORTAL(victim) && victim->level > LEVEL_AVATAR)
	    return TRUE;

	/* NPC doing the killing */
	if (IS_NPC(ch))
	{
	    /* charmed mobs and pets cannot attack players while owned */
	    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master != NULL
	    &&  ch->master->fighting != victim)
		return TRUE;
	
	    /* safe room? */
	    if (IS_SET(victim->in_room->room_flags,ROOM_SAFE))
		return TRUE;

	    /* legal kill? -- mobs only hit players grouped with opponent*/
	    if (ch->fighting != NULL && !is_same_group(ch->fighting,victim))
		return TRUE;
	}

	/* player doing the killing */
	else
	{
	    /* PK privileges removed? */
	    if(IS_SET(ch->comm, COMM_NOPKILL) && ch->fighting == NULL) //lets the ch defend
		return TRUE;

	    /* Killer, thief, etc is not safe. */
	    if (IS_SET(victim->act, PLR_KILLER)
	    || IS_SET(victim->act, PLR_THIEF)
	    || IS_SET(victim->act, PLR_ASSAULT)
	    || IS_SET(victim->act, PLR_TREASON)
	    || IS_SET(victim->act, PLR_OUTLAW))
		return FALSE;

	/* remove PK restrictions for now...
	    if (ch->level > victim->level + 8)
		return TRUE;	*/
	}

    }
    return FALSE;
}
/*
 * See if an attack justifies a KILLER flag.
 */
void check_killer( CHAR_DATA *ch, CHAR_DATA *victim )
{
    char buf[MAX_STRING_LENGTH];
    FILE *fp;
    char *strtime;
    bool AttackCharmie = FALSE;

    /*
     * Follow charm thread to responsible character.
     * Attacking someone's charmed char is hostile!
     */
    while ( IS_AFFECTED(victim, AFF_CHARM) && victim->master != NULL )
    {
	if(IS_NPC(victim)) AttackCharmie = TRUE; //for pk logger 2

	victim = victim->master;
    }

    /*
     * NPC's are fair game.
     * So are killers and thieves.
     */
    if ( IS_NPC(victim)
    || IS_SET(victim->act, PLR_KILLER)
    || IS_SET(victim->act, PLR_THIEF)
    || IS_SET(victim->act, PLR_ASSAULT)
    || IS_SET(victim->act, PLR_TREASON)
    || IS_SET(victim->act, PLR_OUTLAW))
	return;

    /*
     * Charm-o-rama.
     */
    if ( IS_SET(ch->affected_by, AFF_CHARM) )
    {
	if ( ch->master == NULL )
	{
	    char buf[MAX_STRING_LENGTH];

	    sprintf( buf, "Check_killer: %s bad AFF_CHARM",
		IS_NPC(ch) ? ch->short_descr : ch->name );
	    bug( buf, 0 );
	    affect_strip( ch, gsn_charm_person );
	    REMOVE_BIT( ch->affected_by, AFF_CHARM );
	    return;
	}
/*
	send_to_char( "*** You are now a KILLER!! ***\n\r", ch->master );
  	SET_BIT(ch->master->act, PLR_KILLER);
*/

	stop_follower( ch );
	return;
    }

    /*
     * NPC's are cool of course (as long as not charmed).
     * Hitting yourself is cool too (bleeding).
     * So is being immortal (Alander's idea).
     * And current killers stay as they are.
     */
    if ( IS_NPC(ch)
    ||   ch == victim
    ||   ch->level >= LEVEL_AVATAR
    ||   IS_SET(ch->act, PLR_KILLER) 
    ||   ch->fighting  == victim)
	return;


    /* Added so that if an offensive spell is cast on a sleeping
     * victim, that victim will wake up. Was a BUG. This will not
     * effect those who are sleeping due to a spell or skill though.
     */

    if (victim->position == POS_SLEEPING)
    {
      if (IS_AFFECTED(victim, AFF_SLEEP)
      ||  IS_AFFECTED2(victim, AFF_BLACKJACK))
      {
      }
      else
      {
        victim->position = POS_STANDING;
      }
    }

    if (ch->position == POS_SLEEPING)
    {
      if (IS_AFFECTED(ch, AFF_SLEEP)
      ||  IS_AFFECTED2(ch, AFF_BLACKJACK))
      {
      }
      else
      {
        ch->position = POS_STANDING;
      }
    }

    sprintf(buf,"$N is attempting to murder %s!",victim->name);
    wiznet(buf,ch,NULL,WIZ_FLAGS,0,0);
    save_char_obj( ch );

    if (IS_SET(victim->comm,COMM_AFK))
    {
      sprintf(buf,"$N is attempting to murder %s while he/she is AFK!",victim->name);
      wiznet(buf,ch,NULL,WIZ_FLAGS,0,0);
      save_char_obj( ch );
	sprintf( log_buf, "%s attempting to PK %s at %d while he/she is AFK!",
		(IS_NPC(ch) ? ch->short_descr : ch->name),
		victim->name,
		ch->in_room->vnum );
	log_string( log_buf );
    }

    /* PK Logger 2 */
    /* write to the pk attempts log */

/* DO NOT LOG attacks to a person's npc charmie - only on a real PC */
  if(!AttackCharmie)
  {
    fclose(fpReserve);
    if( (fp = fopen(PK_ATTEMPTS_LOG, "a")) == NULL)
    {
	bug("Error opening pk_attempts.txt",0);
	fclose(fp);
	fpReserve = fopen(NULL_FILE, "r");
	return;
    }

    strtime = ctime(&current_time);
    strtime[strlen(strtime)-1] = '\0';

    if (IS_SET(victim->comm,COMM_AFK))
    {
      sprintf(buf, "%s :: %s is attempting to murder %s while he/she is AFK!\n",strtime, ch->name, victim->name);
      fprintf(fp, "%s", buf);
      fclose(fp);
      fpReserve = fopen(NULL_FILE, "r");
    }
    else
    {
      sprintf(buf, "%s :: %s is attempting to murder %s!\n",strtime, ch->name, victim->name);
      fprintf(fp, "%s", buf);
      fclose(fp);
      fpReserve = fopen(NULL_FILE, "r");
    }
  }

  return;
}

bool check_pivot( CHAR_DATA *ch, CHAR_DATA *victim )
{
    OBJ_DATA *wield;
    int chance;

    wield = get_eq_char( victim, WEAR_WIELD );

    if (IS_NPC(victim))
	return FALSE;

    if ( wield == NULL)
	return FALSE;

    if ( !IS_AWAKE(victim) )
        return FALSE;

    if (( wield->value[0] != WEAPON_SPEAR )
    &&  ( wield->value[0] != WEAPON_STAFF ))
    	return FALSE;

    if (victim->riding || ch->riding)
      return FALSE;

    chance  = victim->pcdata->learned[gsn_pivot] / 2;

    chance += get_curr_stat(victim, STAT_DEX)/3; 
    chance -= ch->hitroll /9 + (ch->level <=5) + (ch->level <= 10);
    if ( number_percent( ) >= chance + victim->level - ch->level )
        return FALSE;

    act( "You skillfully pivot your weapon in order to block $n's attack.",  
           ch, NULL, victim, TO_VICT    );
    act( "$N skillfully pivots $s weapon in order to block your attack.", 
          ch, NULL, victim, TO_CHAR    );
    check_improve(victim,gsn_pivot,TRUE,6);
    return TRUE;
}

bool check_redirect( CHAR_DATA *ch, CHAR_DATA *victim )
{
    OBJ_DATA *wield;
    int chance;

    wield = get_eq_char( victim, WEAR_WIELD );

    if (IS_NPC(victim))
	return FALSE;

    if ( wield == NULL)
	return FALSE;

    if ( !IS_AWAKE(victim) )
        return FALSE;

    if (( wield->value[0] != WEAPON_SPEAR )
    &&  ( wield->value[0] != WEAPON_STAFF ))
    	return FALSE;

    chance  = victim->pcdata->learned[gsn_redirect] / 2;

    chance += get_curr_stat(victim, STAT_DEX)/3; 
    chance -= ch->hitroll /9 + (ch->level <=5) + (ch->level <= 10);  
    if ( number_percent( ) >= chance + victim->level - ch->level )
        return FALSE;

    act( "You redirect $n's attack with a defensive staff maneuver.",  
          ch, NULL, victim, TO_VICT    );
    act( "$N redirects your attack with a defensive staff maneuver.", 
          ch, NULL, victim, TO_CHAR    );
    check_improve(victim,gsn_redirect,TRUE,6);
    return TRUE;
}

bool check_riposte( CHAR_DATA *ch, CHAR_DATA *victim )
{
    OBJ_DATA *wield;
    int chance;

    wield = get_eq_char( victim, WEAR_WIELD );

    if (IS_NPC(victim))
	return FALSE;

    if ( wield == NULL)
	return FALSE;

    if ( !IS_AWAKE(victim) )
        return FALSE;

    if ( wield->value[0] != WEAPON_SWORD )
    	return FALSE;

    chance  = victim->pcdata->learned[gsn_riposte] / 2;

    chance += get_curr_stat(victim, STAT_DEX)/4;
    chance -= ch->hitroll /9 + (ch->level <=5) + (ch->level <= 10);  
    if ( number_percent( ) >= chance + victim->level - ch->level )
        return FALSE;

    act( "You parry $n's attack, and follow through with a quick riposte.",  
          ch, NULL, victim, TO_VICT    );
    act( "$N parries your attack, and follows through with a quick riposte.", 
          ch, NULL, victim, TO_CHAR    );
    check_improve(victim,gsn_riposte,TRUE,6);
    one_hit(victim, ch, gsn_riposte,FALSE);
    return TRUE;
}

bool check_evade( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance, skill;

    if (IS_NPC(victim))
	return FALSE;

    if ( !IS_AWAKE(victim) )
        return FALSE;

    if (victim->level < skill_table[gsn_evade].skill_level[victim->class])
       return FALSE;

    skill = victim->pcdata->learned[gsn_evade];

    if( skill <= 1 )
        return FALSE;

    chance  = victim->pcdata->learned[gsn_evade] / 2;
    chance += get_curr_stat(victim, STAT_DEX)/4;
    chance -= ch->hitroll /9 + (ch->level <=5) + (ch->level <= 10);  
    if ( number_percent( ) >= chance + victim->level - ch->level )
        return FALSE;
    act( "You evade $n's attack.", ch, NULL, victim, TO_VICT    );
    act( "$N skillfully evades your attack.", ch, NULL, victim, TO_CHAR );
    check_improve(victim,gsn_evade,TRUE,6);
    return TRUE;
}


bool check_sidestep( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance, skill;

    if (IS_NPC(victim))
	return FALSE;

    if (victim->level < skill_table[gsn_sidestep].skill_level[victim->class])
       return FALSE;

    if ( !IS_AWAKE(victim) )
        return FALSE;

    skill = victim->pcdata->learned[gsn_sidestep];

    if( skill <= 1 )
        return FALSE;

    if (victim->riding)
     return FALSE;

    chance  = victim->pcdata->learned[gsn_sidestep] / 2;
    chance += get_curr_stat(victim, STAT_DEX)/3;
    chance -= ch->hitroll /9 + (ch->level <=5) + (ch->level <= 10);  
    if ( number_percent( ) >= chance + victim->level - ch->level )
        return FALSE;
    act( "You sidestep $n's attack.", ch, NULL, victim, TO_VICT    );
    act( "$N sidesteps your attack.", ch, NULL, victim, TO_CHAR    );
    check_improve(victim,gsn_sidestep,TRUE,6);
    return TRUE;
}

bool check_acrobatics( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance, skill;

    if (IS_NPC(victim))
	return FALSE;

    if ( !IS_AWAKE(victim) )
        return FALSE;

    if (victim->level < skill_table[gsn_acrobatics].skill_level[victim->class])
       return FALSE;

    skill = victim->pcdata->learned[gsn_acrobatics];

    if( skill <= 1 )
        return FALSE;

    if (victim->riding)
     return FALSE;

    chance  = victim->pcdata->learned[gsn_acrobatics] / 2;
    chance += get_curr_stat(victim, STAT_DEX) /2;
    chance -= ch->hitroll /9 + (ch->level <=5) + (ch->level <= 10);  
    if ( number_percent( ) >= chance + victim->level - ch->level )
        return FALSE;
    act( "You flip aside, avoiding $n's attack.", ch, NULL, victim, TO_VICT );
    act( "$N flips away from your attack.", ch, NULL, victim, TO_CHAR );
    check_improve(victim,gsn_acrobatics,TRUE,6);
    return TRUE;
}


/*
 * Check for parry.
 */
bool check_parry( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance;

    if ( !IS_AWAKE(victim) )
	return FALSE;

    chance = get_skill(victim,gsn_parry) / 2;

    if ( get_eq_char( victim, WEAR_WIELD ) == NULL )
    {
	if (IS_NPC(victim))
	    chance /= 2;
	else
	    return FALSE;
    }

    if (!can_see(ch,victim))
	chance /= 2;

    if ( number_percent( ) >= chance + victim->level - ch->level )
	return FALSE;

    act( "You parry $n's attack.",  ch, NULL, victim, TO_VICT    );
    act( "$N parries your attack.", ch, NULL, victim, TO_CHAR    );
    check_improve(victim,gsn_parry,TRUE,6);
    return TRUE;
}

/*
 * Check for shield block.
 */
bool check_shield_block( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance;

    if ( !IS_AWAKE(victim) )
        return FALSE;


    chance = get_skill(victim,gsn_shield_block) / 5 + 3;


    if ( get_eq_char( victim, WEAR_SHIELD ) == NULL )
        return FALSE;

    if ( number_percent( ) >= chance + victim->level - ch->level )
        return FALSE;

    act( "You block $n's attack with your shield.",  ch, NULL, victim, 
TO_VICT    );
    act( "$N blocks your attack with a shield.", ch, NULL, victim, 
TO_CHAR    );
    check_improve(victim,gsn_shield_block,TRUE,6);
    return TRUE;
}


/*
 * Check for dodge.
 */
bool check_dodge( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance;

    if ( !IS_AWAKE(victim) )
	return FALSE;

    if ( IS_NPC(victim) )
        chance  = UMIN( 30, victim->level );
    else
        chance  = victim->pcdata->learned[gsn_dodge] / 2;
    chance += get_curr_stat(victim, STAT_DEX)/3;
    chance -= ch->hitroll /9 + (ch->level <=5) + (ch->level <= 10);  
    if (victim->level < skill_table[gsn_dodge].skill_level[victim->class]
    && !IS_NPC(victim))
       return FALSE;
    if ( number_percent( ) >= chance + victim->level - ch->level )
        return FALSE;

    act( "You dodge $n's attack.", ch, NULL, victim, TO_VICT    );
    act( "$N dodges your attack.", ch, NULL, victim, TO_CHAR    );
    check_improve(victim,gsn_dodge,TRUE,6);
    return TRUE;
}



/*
 * Set position of a victim.
 */
void update_pos( CHAR_DATA *victim )
{
    if ( victim->hit > 0 )
    {
    	if ( victim->position <= POS_STUNNED )
	    victim->position = POS_SITTING;
	return;
    }

    if ( IS_NPC(victim) && victim->hit < 1 )
    {
	if ( victim->riding )
	{
	    act( "$n falls from $N.", victim, NULL, victim->riding, TO_ROOM );
	    victim->riding->rider = NULL;
	    victim->riding        = NULL;
	}
	victim->position = POS_DEAD;
	return;
    }

    if ( victim->hit <= -11 )
    {
	if ( victim->riding )
	{
	    act( "$n falls from $N.", victim, NULL, victim->riding, TO_ROOM );
	    victim->riding->rider = NULL;
	    victim->riding        = NULL;
	}
	victim->position = POS_DEAD;
	return;
    }

         if ( victim->hit <= -6 ) victim->position = POS_MORTAL;
    else if ( victim->hit <= -3 ) victim->position = POS_INCAP;
    else                          victim->position = POS_STUNNED;

    if ( victim->riding )
    {
	act( "$n falls unconscious from $N.", victim, NULL, victim->riding, TO_ROOM );
	victim->riding->rider = NULL;
	victim->riding        = NULL;
    }

    return;
}



/*
 * Start fights.
 */
void set_fighting( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( ch->fighting != NULL )
    {
	bug( "Set_fighting: already fighting", 0 );
	return;
    }

    check_cpose(ch);

    if ( IS_AFFECTED(ch, AFF_SLEEP) )
    {
      affect_strip(ch, gsn_sleep );
	ch->position = POS_STANDING;
    }

    if ( IS_AFFECTED(victim, AFF_SLEEP))
    {
      affect_strip(victim, gsn_sleep );
	act("$N wakes up and gets ready to fight.\n\r",ch,NULL,victim,TO_ROOM);
	victim->position = POS_STANDING;
    } 

    if ( IS_AFFECTED2(ch, AFF_BLACKJACK) )
	affect_strip( ch, gsn_blackjack );

    ch->fighting = victim;
    ch->lastfought = victim;
    if (!IS_NPC(victim))
    {
      free_string(ch->last_pc_fought);
      ch->last_pc_fought = str_dup(victim->name);
    }

    if ( ch->position == POS_STANDING)  /* no auto_stand in fight */
     ch->position = POS_FIGHTING;

    if (IS_NPC(ch)
    && !IS_NPC(victim) 
    && IS_SET(ch->act,ACT_TRACK))
    {
	ch->hunting = victim;
    }

    return;
}



/*
 * Stop fights.
 */
void stop_fighting( CHAR_DATA *ch, bool fBoth )
{
  CHAR_DATA *fch;

  for ( fch = char_list; fch != NULL; fch = fch->next )
  {
    if ( fch == ch || ( fBoth && fch->fighting == ch ) )
    {
	fch->fighting	= NULL;
	fch->position	= IS_NPC(fch) ? fch->default_pos : POS_STANDING;
	update_pos( fch );
      fch->fight_end_timer = 2;
      if ( fch->quit_timer >= 1 )
      {
        fch->quit_timer = 0;
      }

      /* if PC victim, then you can't quit right away
         sets quit_timer if needed */
      if ( fch->lastfought == NULL )
        return;

      /*  The quit_timer value is registered in ticks.  When it reaches
          0, the PC can quit.  Change this value if needed  */
      if ( !IS_NPC(fch->lastfought) )
      {
        fch->quit_timer = 0;
        fch->quit_timer = -8; 
      } 
    }
  }
  return;
}



/*
 * Make a corpse out of a character.
 */
void make_corpse( CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *corpse;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    char *name;

    if ( IS_NPC(ch) )
    {
	name			= ch->short_descr;
	corpse		= create_object(get_obj_index(OBJ_VNUM_CORPSE_NPC), 0);
	corpse->timer	= number_range( 3, 6 );
	if ( ch->steel > 0 )
	{
	    obj_to_obj( create_money( ch->steel, ch->gold ), corpse );
	    ch->steel = 0;
	    ch->gold = 0;
	}
	corpse->cost = 0;
    }
    else
    {
	name			= ch->name;
	corpse		= create_object(get_obj_index(OBJ_VNUM_CORPSE_PC), 0);
	corpse->timer	= number_range( 25, 40 );
	REMOVE_BIT(ch->act,PLR_CANLOOT);
	corpse->owner 	= str_dup(ch->name);
	if (ch->steel > 1 || ch->gold > 1)
	{
		obj_to_obj(create_money(ch->steel / 2, ch->gold/2), corpse);
		ch->steel -= ch->steel/2;
		ch->gold -= ch->gold/2;
	 }	
	corpse->cost = 0;
    }

    corpse->level = ch->level;

    sprintf( buf, corpse->short_descr, name );
    free_string( corpse->short_descr );
    corpse->short_descr = str_dup( buf );

    sprintf( buf, corpse->description, name );
    free_string( corpse->description );
    corpse->description = str_dup( buf );

    for ( obj = ch->carrying; obj != NULL; obj = obj_next )
    {
	bool floating = FALSE;

	obj_next = obj->next_content;
	if (obj->wear_loc == WEAR_FLOAT)
	    floating = TRUE;
	obj_from_char( obj );
	if (obj->item_type == ITEM_POTION)
	    obj->timer = number_range(500,1000);
	if (obj->item_type == ITEM_SCROLL)
	    obj->timer = number_range(1000,2500);
	if (IS_SET(obj->extra_flags,ITEM_ROT_DEATH)
      && !floating
	&& !IS_NPC(ch))
	{
	    obj->timer = number_range(5,10);
	    REMOVE_BIT(obj->extra_flags,ITEM_ROT_DEATH);
	}
	REMOVE_BIT(obj->extra_flags,ITEM_VIS_DEATH);

      /* if it's low level eq, set auto cleanup timer */
      set_low_eq_tmr( obj );

	if ( IS_SET( obj->extra_flags, ITEM_INVENTORY ) )
	    extract_obj( obj );
	else if (floating)
	{
	    if (IS_OBJ_STAT(obj,ITEM_ROT_DEATH)) /* get rid of it! */
	    { 
		if (obj->contains != NULL)
		{
		    OBJ_DATA *in, *in_next;

		    act("$p evaporates,scattering its contents.",
			ch,obj,NULL,TO_ROOM);
		    for (in = obj->contains; in != NULL; in = in_next)
		    {
			in_next = in->next_content;
			obj_from_obj(in);
			obj_to_room(in,ch->in_room);
		    }
		 }
		 else
		    act("$p evaporates.",
			ch,obj,NULL,TO_ROOM);
		 extract_obj(obj);
	    }
	    else
	    {
		act("$p falls to the floor.",ch,obj,NULL,TO_ROOM);
		obj_to_room(obj,ch->in_room);
	    }
	}
	else
	    obj_to_obj( obj, corpse );
    }

    obj_to_room( corpse, ch->in_room );

    return;
}

/*
 * Improved Death_cry contributed by Diavolo.
 */
void death_cry( CHAR_DATA *ch )
{
    ROOM_INDEX_DATA *was_in_room;
    char *msg;
    int door;
    int vnum;
    vnum = 0;
    msg = "You hear $n's death cry.";

    switch ( number_bits(4))
    {
    case  0: msg  = "$n hits the ground ... DEAD.";			break;
    case  1: 
	if (ch->material == 0)
	{
	    msg  = "$n splatters blood on your armor.";		
	    break;
	}
    case  2: 							
	if (IS_SET(ch->parts,PART_GUTS))
	{
	    msg = "$n spills $s guts all over the floor.";
	    vnum = OBJ_VNUM_GUTS;
	}
	break;
    case  3: 
	if (IS_SET(ch->parts,PART_HEAD))
	{
	    msg  = "$n's severed head plops on the ground.";
	    vnum = OBJ_VNUM_SEVERED_HEAD;				
	}
	break;
    case  4: 
	if (IS_SET(ch->parts,PART_HEART))
	{
	    msg  = "$n's heart is torn from $s chest.";
	    vnum = OBJ_VNUM_TORN_HEART;				
	}
	break;
    case  5: 
	if (IS_SET(ch->parts,PART_ARMS))
	{
	    msg  = "$n's arm is sliced from $s dead body.";
	    vnum = OBJ_VNUM_SLICED_ARM;				
	}
	break;
    case  6: 
	if (IS_SET(ch->parts,PART_LEGS))
	{
	    msg  = "$n's leg is sliced from $s dead body.";
	    vnum = OBJ_VNUM_SLICED_LEG;				
	}
	break;
    case 7:
	if (IS_SET(ch->parts,PART_BRAINS))
	{
	    msg = "$n's head is shattered, and $s brains splash all over you.";
	    vnum = OBJ_VNUM_BRAINS;
	}
    }

    act( msg, ch, NULL, NULL, TO_ROOM );

    if ( vnum != 0 )
    {
	char buf[MAX_STRING_LENGTH];
	OBJ_DATA *obj;
	char *name;

	name		= IS_NPC(ch) ? ch->short_descr : ch->name;
	obj		= create_object( get_obj_index( vnum ), 0 );
	obj->timer	= number_range( 4, 7 );

	sprintf( buf, obj->short_descr, name );
	free_string( obj->short_descr );
	obj->short_descr = str_dup( buf );

	sprintf( buf, obj->description, name );
	free_string( obj->description );
	obj->description = str_dup( buf );

	if (obj->item_type == ITEM_FOOD)
	{
	    if (IS_SET(ch->form,FORM_POISON))
		obj->value[3] = 1;
	    else if (!IS_SET(ch->form,FORM_EDIBLE))
		obj->item_type = ITEM_TRASH;
	}

	obj_to_room( obj, ch->in_room );
    }

    if ( IS_NPC(ch) )
	msg = "You hear something's death cry.";
    else
	msg = "You hear someone's death cry.";

    was_in_room = ch->in_room;
    for ( door = 0; door <= 5; door++ )
    {
	EXIT_DATA *pexit;

	if ( ( pexit = was_in_room->exit[door] ) != NULL
	&&   pexit->u1.to_room != NULL
	&&   pexit->u1.to_room != was_in_room )
	{
	    ch->in_room = pexit->u1.to_room;
	    act( msg, ch, NULL, NULL, TO_ROOM );
	}
    }
    ch->in_room = was_in_room;

    return;
}

/*
 * invulnerable
 *
 * Makes a char temporarily invulnerable.  Can't attack stuff either
 * tho.
 */
void invulnerable( CHAR_DATA * ch )
{
    AFFECT_DATA af;

    af.where	 = TO_AFFECTS;
    af.type        = gsn_infravision;
    af.level	 = ch->level;
    af.duration    = 5;
    af.location    = APPLY_NONE;
    af.modifier    = 0;
    af.bitvector   = AFF_INFRARED;
    affect_to_char( ch, &af );

    send_to_char( "Your spirit is temporarily protected.\n\r", ch );

    af.where	= TO_AFFECTS2;
    af.type 	= gsn_invul;
    af.level 	= ch->level;
    af.duration	= 5;
    af.location	= APPLY_NONE;
    af.modifier	= 0;
    af.bitvector  = AFF_INVUL;
    affect_to_char(ch,&af);
}

void raw_kill( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int i;
    OBJ_DATA *tattoo;
    OBJ_DATA *mark_item; 
    mark_item = NULL;

    if (victim == NULL)
    {
    bugf("raw_kill: Trying to operate RAW_KILL with no victim.\n");
    return;
    }

    stop_fighting( victim, TRUE );
    if (IS_NPC(victim))
    {
	victim->hunting = NULL;
    }
    death_cry( victim );

    mark_item = get_eq_char(victim, WEAR_MARK); /* keep religion mark */
    if (mark_item != NULL)
    {
      obj_from_char(mark_item);
    }
    
    tattoo = get_eq_char(victim, WEAR_TATTOO); /* keep tattoo */
    if (tattoo != NULL)
        obj_from_char(tattoo);
    make_corpse( victim );

    if( IS_NPC(victim) && (victim->spec_fun != NULL)
      && (victim->spec_fun == spec_lookup("spec_rloader")) )
		spec_rloader(ch);

    if ( victim->rider )
    {
        act( "$n dies suddenly, and you fall to the ground.", 
	    victim, NULL, victim->rider, TO_VICT );
        victim->rider->riding	  = NULL;
        victim->rider->position = POS_RESTING;
        victim->rider		  = NULL;
    }

    if ( victim->riding )
    {
        act( "$n falls off you, dead.", 
	    victim, NULL, victim->riding, TO_VICT );
        victim->riding->rider	   = NULL;
        victim->riding		   = NULL;
    }

    if ( IS_NPC(victim) )
    {
	victim->pIndexData->killed++;
	kill_table[URANGE(0, victim->level, MAX_LEVEL-1)].killed++;
	extract_char( victim, TRUE );
	if (tattoo != NULL)
        {
            obj_to_char(tattoo, victim);
            equip_char(victim, tattoo, WEAR_TATTOO);
        }
        return;
    }

    if (mark_item != NULL)
    {
        obj_to_char(mark_item, victim);
        equip_char(victim, mark_item, WEAR_MARK);
    }

    while ( victim->affected )
    affect_remove( victim, victim->affected );
    victim->affected_by	= race_table[victim->race].aff;
    for (i = 0; i < 4; i++)
    victim->armor[i]= 100;
    victim->position	= POS_DEAD;
    victim->hit		= 1;
    victim->mana		= 1;
    victim->move		= 1;
    victim->loc_hp      = 0;
    victim->loc_hp2     = 0;
    victim->pcdata->condition[COND_THIRST] = 65;
    victim->pcdata->condition[COND_FULL]   = 65;
    /* RT added to prevent infinite deaths */
    REMOVE_BIT(victim->act, PLR_KILLER);
    REMOVE_BIT(victim->act, PLR_THIEF);
    REMOVE_BIT(victim->act, PLR_ASSAULT);
    REMOVE_BIT(victim->act, PLR_TREASON);
    REMOVE_BIT(victim->act, PLR_OUTLAW);
    /* increase death counter */
    if (victim->level > 8)
    {
     victim->pcdata->deaths++;
    }
    invulnerable(victim);
/*  save_char_obj( victim ); we're stable enough to not need this :) */
    return;
}

void mercy_kill( CHAR_DATA *ch, CHAR_DATA *victim )
{
    AFFECT_DATA af;
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj, *obj_next;
    OBJ_DATA *mark_item; 
    mark_item = NULL;
    stop_fighting( victim, TRUE );

    act( "You bestow mercy upon $N, choosing to subdue rather than killing $M outright!",ch,NULL,victim,TO_CHAR);
    sprintf(buf, "%s bestows mercy upon you, choosing to subdue rather than kill you!\n\r", ch->name);
    send_to_char( buf, victim );
    act( "$n bestows mercy upon $N, choosing to subdue rather than killing $M.",ch,NULL,victim,TO_NOTVICT);

    sprintf( log_buf, "%s got subdued by %s at %s [room %d]",
      	(IS_NPC(victim) ? victim->short_descr : victim->name),
            (IS_NPC(ch) ? ch->short_descr : ch->name),
            ch->in_room->name, ch->in_room->vnum);

    wiznet(log_buf,NULL,NULL,WIZ_SUBDUES,0,0);

    if ( victim->riding )
    {
	act( "$n falls from $N.",victim,NULL,victim->riding,TO_ROOM );
	victim->riding->rider = NULL;
	victim->riding        = NULL;
    }

    stop_fighting( victim, TRUE );

    mark_item = get_eq_char(victim, WEAR_MARK);
    if (mark_item != NULL)
       obj_from_char(mark_item);
    
    for ( obj = victim->carrying; obj != NULL; obj = obj_next )
    {
      obj_next = obj->next_content;
	
      if (IS_SET(obj->extra_flags,ITEM_LIGHT)
	&& can_drop_obj(victim, obj) 
	&& (!IS_SET(obj->extra_flags, ITEM_NOREMOVE)))
	{
	  unequip_char(victim, obj);
	  obj_from_char(obj);
	  obj_to_room(obj, victim->in_room);
	}
	else
      if (IS_SET(obj->wear_flags,ITEM_HOLD|ITEM_WIELD|ITEM_WEAR_SHIELD)  
      && can_drop_obj(victim, obj)
      && (!IS_SET(obj->extra_flags, ITEM_NOREMOVE)))
      {
        unequip_char(victim, obj);
        obj_from_char(obj);
        obj_to_room(obj, victim->in_room);
      }
    }
    
    if (mark_item != NULL) 
    {
      obj_to_char(mark_item, victim);
      equip_char(victim, mark_item, WEAR_MARK);
    }
    victim->hit         = UMAX( 1, victim->hit  );
    victim->mana        = UMAX( 1, victim->mana );
    victim->move        = UMAX( 1, victim->move );
    victim->loc_hp      = 0;
    victim->loc_hp2     = 0;
    victim->position    = POS_SLEEPING;

    if (!is_affected(victim, gsn_subdue))
    {
      af.where		= TO_AFFECTS3;
      af.type     	= gsn_subdue;
      af.level    	= 0;
      af.duration 	= 2;
      af.location 	= APPLY_NONE;
      af.modifier 	= 0;
      af.bitvector 	= AFF_SUBDUE;
      affect_to_char(victim,&af);

      af.where		= TO_AFFECTS;
      af.type     	= skill_lookup( "sleep" );
      af.level    	= 0;
      af.duration 	= 2;
      af.location 	= APPLY_NONE;
      af.modifier 	= 0;
      af.bitvector 	= AFF_SLEEP;
      affect_to_char(victim,&af);
    
      victim->position = POS_SLEEPING;
    }
    if(is_affected(victim, gsn_poison))
      affect_strip(victim, gsn_poison);
    if(is_affected(victim, gsn_plague))
      affect_strip(victim, gsn_plague);
    if (IS_AFFECTED(victim, AFF_POISON))
      REMOVE_BIT(victim->affected_by, AFF_POISON);
    if (IS_AFFECTED(victim, AFF_PLAGUE))
      REMOVE_BIT(victim->affected_by, AFF_PLAGUE);
    return;
}

void group_gain( CHAR_DATA *ch, CHAR_DATA *victim )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *gch;
    CHAR_DATA *lch;
    int xp;
    int members;
    int group_levels;

    /*
     * Monsters don't get kill xp's or alignment changes.
     * P-killing doesn't help either.
     * Dying of mortal wounds or poison doesn't give xp to anyone!
     */
    if ( !IS_NPC(victim) || victim == ch )
        return;
    
    members = 0;
    group_levels = 0;

    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
        if ( is_same_group( gch, ch ) )
        {
            if( !IS_NPC(gch) )
            {
		if ( ABS(gch->level - ch->level) > GROUP_RANGE )
		  members=1100;
		else
                  members++;

                group_levels += gch->level;
            }
            else if ( IS_NPC(gch) && members >= 3 )
                members++;      
        }
    }

    if ( members == 0 )
    {
	members = 1;
	group_levels = ch->level ;
    }
    lch = (ch->leader != NULL) ? ch->leader : ch;
    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
      OBJ_DATA *obj;
      OBJ_DATA *obj_next;

      if ( !is_same_group( gch, ch ) || IS_NPC(gch))
        continue;
      if (!IS_SET(ch->act,PLR_NOEXP))
	{
        if (ch->level < 100)
	  {
          xp = xp_compute( gch, victim, group_levels, members );  
	    if(gch->class == class_lookup("blacksmith")) //Smiths get 0 exp for kills.
		xp = 0;
          if (IS_SET(victim->act, ACT_NO_XP_GAIN))
            xp = 0;
          sprintf( buf, "You receive %d experience points.\n\r", xp );
          send_to_char( buf, gch );
	    gain_exp( gch, xp );
	  }
	  else
	  {
	  }
	}

	for ( obj = ch->carrying; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;
	    if ( obj->wear_loc == WEAR_NONE )
		continue;

	    if ( ( IS_OBJ_STAT(obj, ITEM_ANTI_EVIL)    && IS_EVIL(ch)    )
	    ||   ( IS_OBJ_STAT(obj, ITEM_ANTI_GOOD)    && IS_GOOD(ch)    )
	    ||   ( IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch) ) )
	    {
		act( "You are zapped by $p.", ch, obj, NULL, TO_CHAR );
		act( "$n is zapped by $p.",   ch, obj, NULL, TO_ROOM );
		obj_from_char( obj );
          if ( IS_OBJ_STAT(obj,ITEM_NODROP) )
            obj_to_char( obj, ch );
          else
		obj_to_room( obj, ch->in_room );
	    }
	}
    }

    if (IS_SET(ch->act, PLR_QUESTOR)&&IS_NPC(victim))
    {
        if (ch->questmob == victim->pIndexData->vnum)
        {
            send_to_char("You have almost completed your QUEST!\n\r",ch);
            send_to_char("Return to the questmaster before your time runs out!\n\r",ch);
            ch->questmob = -1;
        }
    }

    return;
}



/*
 * Compute xp for a kill.
 * Also adjust alignment of killer.
 * Edit this function to change xp computations.
 */
int xp_compute( CHAR_DATA *gch, CHAR_DATA *victim, int total_levels, int ngrp )
{
    int xp,base_exp;
    int change;
    int level_range;
    double align, valign, adj, factor, mag_align;
    level_range = victim->level - gch->level;
    /* compute the base exp */
    switch (level_range)
    {
 	default : 	base_exp =   0;		break;
	case -9 :	base_exp =   1;		break;
	case -8 :	base_exp =   2;		break;
	case -7 :	base_exp =   5;		break;
	case -6 : 	base_exp =   9;		break;
	case -5 :	base_exp =  11;		break;
	case -4 :	base_exp =  22;		break;
	case -3 :	base_exp =  33;		break;
	case -2 :	base_exp =  50;		break;
	case -1 :	base_exp =  66;		break;
	case  0 :	base_exp =  83;		break;
	case  1 :	base_exp =  99;		break;
	case  2 :	base_exp = 121;		break;
	case  3 :	base_exp = 143;		break;
	case  4 :	base_exp = 165;		break;
    } 
    
    if (level_range > 4)
        base_exp = 250 + 20 * (level_range - 4);
    /* calculate exp multiplier */
    if (IS_SET(victim->act,ACT_NOALIGN))
        xp = base_exp;
    else 
    {
        /*
            try this curve-- if you're strongly it makes a gentle
            rise if you kill oppositie aligned stuff, a sharpish
            dip if you kill liked aligned.  if you're less
            aligned, you get a gentle boost as you kill either
            to either end of the alignment scale.
            generates 'factor' from near zero to 1.33.
        */

        align = gch->alignment / 1000.0;
        valign = victim->alignment / 1000.0;
        mag_align = ABS(align);
        adj = (align - valign) / 2;
        if(align < 0) adj = - adj;
        adj = mag_align * (-1.0 + 2 * adj) + (1.0 - mag_align) * ABS(adj);
        adj -= 1.0;
        factor = -0.33 * adj * adj + 1.33;
        xp = (int)(factor * (double) base_exp);
        /* now calc align delta */
        adj = -valign;
        adj = ((mag_align * 0.666666) + 0.3333) * adj;
        change = (int) (adj * 20);
	if ((gch->alignment < 1000) && (gch->alignment > -1000))
        gch->alignment += get_align_change(gch->alignment, victim->alignment);
        if (gch->alignment > 1000)
         gch->alignment = 1000;
        else if (gch->alignment < -1000)
         gch->alignment = -1000;
    }
    /*
        Adjust based on level.  More exp at the low and very 
        high levels.
    */
    if (gch->level < 20)
        xp = (10 * xp / (gch->level + 4)) * 2;
    else if (gch->level < 65)
        xp = (xp / 2) * 2;
    else if (gch->level < 80)
        xp = (xp / 2 ) * 5;

#if 0
    /* less at high */
    if (gch->level > 90 )
        xp =  (15 * xp / (gch->level - 20 )) * 14;
    /* reduce for playing time */
    else
    {
        /* compute quarter-hours per level */
        time_per_level = 4 *
                         (gch->played + (int) (current_time - gch->logon))/3600
                         / gch->level;
        time_per_level = URANGE(2,time_per_level,12);
        if (gch->level < 15)  /* make it a curve */
            time_per_level = UMAX(time_per_level,(15 - gch->level));
        xp = xp * time_per_level / 8;
    }
#endif 
    /* randomize the rewards */
    xp = number_range (xp * 3 / 4, xp * 4 / 3);
    /* adjust for grouping */
    if (ngrp > 1000)
	xp = 2;
    else
	xp = xp * gch->level / (total_levels / ngrp);

    /* figure bonus based on group size */
    switch( ngrp ) {
    case 1:
        break;
    case 2:
        xp = (xp * 7) / 4;
        break;
    case 3:
        xp = (xp * 5) / 4;
        break;
    case 4:
        xp = (xp * 3) / 4;
    default:
        xp = (xp * 1) / 2;
        break;
    }
    /* DEBUG */
    if( xp < 0) {
        char buf[100];
        sprintf(buf,
         "xp_compute: xp=%d, gch=%s victim=%s gchlev=%d viclev=%d\n\r",
         xp,
         IS_NPC(gch) ? gch->short_descr : gch->name,
         IS_NPC(victim) ? victim->short_descr : victim->name,
         gch->level,
         victim->level);
        bug(buf, 0);
        sprintf(buf,"gchalign=%d vicalign=%d\n\r",
         gch->alignment,
         victim->alignment);
        bug(buf, 0);
    } /* end DEBUG */
    if (xp > 2000) xp = number_range(1890,2018);
    return xp;
}

void make_part( CHAR_DATA *ch, char *argument )
{
  char  arg [MAX_INPUT_LENGTH];
  int vnum;

  argument = one_argument(argument,arg);
  vnum = 0;

  if (arg[0] == '\0') return;
  if (!str_cmp(arg,"head")) vnum = OBJ_VNUM_SEVERED_HEAD;
  else if (!str_cmp(arg,"arm")) vnum = OBJ_VNUM_SLICED_ARM;
  else if (!str_cmp(arg,"leg")) vnum = OBJ_VNUM_SLICED_LEG;
  else if (!str_cmp(arg,"heart")) vnum = OBJ_VNUM_TORN_HEART;
  else if (!str_cmp(arg,"turd")) vnum = OBJ_VNUM_TORN_HEART;
  else if (!str_cmp(arg,"entrails")) vnum = OBJ_VNUM_GUTS;
  else if (!str_cmp(arg,"brain")) vnum = OBJ_VNUM_BRAINS;
  else if (!str_cmp(arg,"eyeball")) vnum = OBJ_VNUM_SQUIDGY_EYEBALL;
  else if (!str_cmp(arg,"blood")) vnum = OBJ_VNUM_SPILT_BLOOD;
  else if (!str_cmp(arg,"face")) vnum = OBJ_VNUM_RIPPED_FACE;
  else if (!str_cmp(arg,"windpipe")) vnum = OBJ_VNUM_TORN_WINDPIPE;
  else if (!str_cmp(arg,"cracked_head")) vnum = OBJ_VNUM_CRACKED_HEAD;
  else if (!str_cmp(arg,"ear")) vnum = OBJ_VNUM_SLICED_EAR;
  else if (!str_cmp(arg,"nose")) vnum = OBJ_VNUM_SLICED_NOSE;
  else if (!str_cmp(arg,"tooth")) vnum = OBJ_VNUM_KNOCKED_TOOTH;
  else if (!str_cmp(arg,"tongue")) vnum = OBJ_VNUM_TORN_TONGUE;
  else if (!str_cmp(arg,"hand")) vnum = OBJ_VNUM_SEVERED_HAND;
  else if (!str_cmp(arg,"foot")) vnum = OBJ_VNUM_SEVERED_FOOT;
  else if (!str_cmp(arg,"thumb")) vnum = OBJ_VNUM_SEVERED_THUMB;
  else if (!str_cmp(arg,"index")) vnum = OBJ_VNUM_SEVERED_INDEX;
  else if (!str_cmp(arg,"middle")) vnum = OBJ_VNUM_SEVERED_MIDDLE;
  else if (!str_cmp(arg,"ring")) vnum = OBJ_VNUM_SEVERED_RING;
  else if (!str_cmp(arg,"little")) vnum = OBJ_VNUM_SEVERED_LITTLE;
  else if (!str_cmp(arg,"toe")) vnum = OBJ_VNUM_SEVERED_TOE;

  if ( vnum != 0 )
  {
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    char *name;

    name = IS_NPC(ch) ? ch->short_descr : ch->name;
    obj = create_object( get_obj_index( vnum ), 0 );
    if (IS_NPC(ch)) obj->timer = number_range(2,5);
    else obj->timer = 100;

    if (vnum == OBJ_VNUM_SPILT_BLOOD) obj->timer = 2;

    if (!IS_NPC(ch))
    {
      sprintf( buf, obj->name, name );
      free_string( obj->name );
      obj->name = str_dup( buf );
    }
    else
    {
      sprintf( buf, obj->name, "mob" );
      free_string( obj->name );
      obj->name = str_dup( buf );
    }
    sprintf( buf, obj->short_descr, name );
    free_string( obj->short_descr );
    obj->short_descr = str_dup( buf );
    sprintf( buf, obj->description, name );
    free_string( obj->description );
    obj->description = str_dup( buf );
    if( ch->in_room != NULL && obj->in_room == NULL )
      obj_to_room( obj, ch->in_room );
  }
  return;
}


void dam_message( CHAR_DATA *ch, CHAR_DATA *victim,int dam,int dt,bool immune )
{
    char buf1[256], buf2[256], buf3[256]; /* defaults changed from 256 */
    const char *vs;
    const char *vp;
    const char *attack;
    char punct;
    int damp;

    if (ch == NULL || victim == NULL)
	return;

	   if ( dam <=   0 ) { vs = "miss";	vp = "misses";		      }
    else if ( dam <=   4 ) { vs = "{Gscratch{x"; vp = "{Gscratches{x";  }
    else if ( dam <=  12 ) { vs = "{Ggraze{x";	vp = "{Ggrazes{x";      }
    else if ( dam <=  20 ) { vs = "{Ghit{x";	vp = "{Ghits{x";        }
    else if ( dam <=  25 ) { vs = "{Binjure{x";	vp = "{Binjures{x";     }
    else if ( dam <=  30 ) { vs = "{ywound{x";	vp = "{ywounds{x";      }
    else if ( dam <=  35 ) { vs = "{Ymaul{x";   vp = "{Ymauls{x";       }
    else if ( dam <=  38 ) { vs = "{rdecimate{x"; vp = "{rdecimates{x"; }
    else if ( dam <=  40 ) { vs = "{Rdevastate{x"; vp = "{Rdevastates{x"; }
    else if ( dam <=  45 ) { vs = "{Cmaim{x";	vp = "{Cmaims{x";       }
    else if ( dam <=  48 ) { vs = "{MMUTILATE{x"; vp = "{MMUTILATES{x"; }
    else if ( dam <=  50 ) { vs = "{RDISEMBOWEL{x"; vp = "{RDISEMBOWELS{x"; }
    else if ( dam <=  55 ) { vs = "{RDISMEMBER{x"; vp = "{RDISMEMBERS{x"; }
    else if ( dam <=  59 ) { vs = "{MMASSACRE{x"; vp = "{MMASSACRES{x"; }
    else if ( dam <=  65 ) { vs = "{YMANGLE{x";	vp = "{YMANGLES{x";     }
    else if ( dam <=  75 ) { vs = "{M*** {RDEMOLISH{M ***{x";
			     vp = "{M*** {RDEMOLISHES{M ***{x";         }
    else if ( dam <=  90 ) { vs = "{R*** {CDEVASTATE{R ***{x";
			     vp = "{R*** {CDEVASTATES{R ***{x";         }
    else if ( dam <= 130)  { vs = "{B==={R OBLITERATE{B ==={x";
			     vp = "{B=== {ROBLITERATES{B ==={x";        }
    else if ( dam <= 150)  { vs = "{R>>>{B ANNIHILATE{R <<<{x";
			     vp = "{R>>>{B ANNIHILATES{R <<<{x";        }
    else if ( dam <= 200)  { vs = "{R<<<{Y ERADICATE{R >>>{x";
			     vp = "{R<<<{Y ERADICATES{R >>>{x";         }
    else if ( dam <= 250) { vs = "{Y---{B=== {RSLAUGHTERS{B ==={Y---{x";
                       vp = "{Y---{B=== {RSLAUGHTERS{B ==={Y---{x"; }
    else if ( dam <= 300) { vs = "{C---=== {RBUTCHERS {C===---{x";
                       vp = "{C---=== {RBUTCHERS {C===---{x"; }
    else if ( dam <= 400) { vs = "do {MUNSPEAKABLE{x things to{x";
			     vp = "does {MUNSPEAKABLE{x things to{x"; }
    else                  { vs = "{WCOMPL{RETELY DEST{WROYS{x";
                       vp =  "{WCOMPL{RETELY DEST{WROYS{x"; }
    punct   = (dam <= 24) ? '.' : '!';

    if (IS_NPC(victim)) damp = 0;
    else damp = -10;

    if ( dt == TYPE_HIT )
    {
	if (ch  == victim)
	{
	    sprintf( buf1, "$n %s $melf%c",vp,punct);
	    sprintf( buf2, "You %s yourself%c [%d]",vs,punct, dam);
	}
	else
	{
	    sprintf( buf1, "$n %s $N%c",  vp, punct);
	    sprintf( buf2, "You %s $N%c [%d]", vs, punct, dam );
	    sprintf( buf3, "$n %s you%c [%d]", vp, punct, dam );
	}
    }
    else
    {
	if ( dt >= 0 && dt < MAX_SKILL )
	    attack	= skill_table[dt].noun_damage;
	else if ( dt >= TYPE_HIT
	&& dt < (TYPE_HIT + MAX_DAMAGE_MESSAGE + 1)) 
	    attack	= attack_table[dt - TYPE_HIT].noun;
	else
      if ( dt == -8 ) /*custom messages. Use negative numbers*/
	{
	  attack = str_dup("Sonic Wave");
	}
	else
	{
	    bug( "Dam_message: bad dt %d.", dt );
	    dt  = TYPE_HIT;
	    attack  = attack_table[0].name;
	}

	if (immune)
	{
	    if (ch == victim)
	    {
		sprintf(buf1,"$n is unaffected by $s own %s.",attack);
		sprintf(buf2,"Luckily, you are immune to that.");
	    } 
	    else
	    {
	    	sprintf(buf1,"$N is unaffected by $n's %s!",attack);
	    	sprintf(buf2,"$N is unaffected by your %s!",attack);
	    	sprintf(buf3,"$n's %s is powerless against you.",attack);
	    }
	}
	else
	{
	    if (ch == victim)
	    {
		sprintf( buf1, "$n's %s %s $m%c",attack,vp,punct);
		sprintf( buf2, "Your %s %s you%c [%d]",attack,vp,punct, dam);
	    }
	    else
	    {
	    	sprintf( buf1, "$n's %s %s $N%c",  attack, vp, punct);
	    	sprintf( buf2, "Your %s %s $N%c [%d]",  attack, vp, punct, dam );
	    	sprintf( buf3, "$n's %s %s you%c [%d]", attack, vp, punct, dam );
	    }
	}
    }

    if (ch == victim)
    {
	act(buf1,ch,NULL,NULL,TO_ROOM);
	act(buf2,ch,NULL,NULL,TO_CHAR);
    }
    else
    {
    	act( buf1, ch, NULL, victim, TO_NOTVICT );
    	act( buf2, ch, NULL, victim, TO_CHAR );
    	act( buf3, ch, NULL, victim, TO_VICT );
    }

  if ( dt >= 0 && dt < MAX_SKILL )
	    attack	= skill_table[dt].noun_damage;
  else if ( dt >= TYPE_HIT
       && dt < (TYPE_HIT + MAX_DAMAGE_MESSAGE + 1)) 
	    attack	= attack_table[dt - TYPE_HIT].noun;
  else
  if ( dt == -8 ) /*custom messages. Use negative numbers*/
  {
    attack = str_dup("Sonic Wave");
  }
  else
  {
	    bug( "Dam_message: bad dt %d.", dt );
	    dt  = TYPE_HIT;
	    attack  = attack_table[0].name;
  }
  if ( (attack == attack_table[18].name 
  || attack == attack_table[17].name)
  && (number_percent() < 2) )
  {
    damp=number_range(1,5);
    if ( damp == 1 )
    {
      act("You ram your fingers into $N's eye sockets and rip $S face off.", ch, NULL, victim, TO_CHAR);
      act("$n rams $s fingers into $N's eye sockets and rips $S face off.", ch, NULL, victim, TO_NOTVICT);
      act("$n rams $s fingers into your eye sockets and rips your face off.", ch, NULL, victim, TO_VICT);
      make_part(victim,"face");
    }
    else if ( damp == 2 )
    {
      act("You grab $N by the throat and tear $S windpipe out.", ch, NULL, victim, TO_CHAR);
      act("$n grabs $N by the throat and tears $S windpipe out.", ch, NULL, victim, TO_NOTVICT);
      act("$n grabs you by the throat and tears your windpipe out.", ch, NULL, victim, TO_VICT);
      make_part(victim,"windpipe");
    }
    else if ( damp == 3 )
    {
      act("You punch your fist through $N's stomach and rip out $S entrails.", ch, NULL, victim, TO_CHAR);
      act("$n punches $s fist through $N's stomach and rips out $S entrails.", ch, NULL, victim, TO_NOTVICT);
      act("$n punches $s fist through your stomach and rips out your entrails.", ch, NULL, victim, TO_VICT);
      make_part(victim,"entrails");
    }
    else if ( damp == 4 )
    {
      if (!IS_SET(victim->loc_hp,BROKEN_SPINE))
      SET_BIT(victim->loc_hp,BROKEN_SPINE);
      act("You hoist $N above your head and slam $M down upon your knee.\n\rThere is a loud cracking sound as $N's spine snaps.", ch, NULL, victim, TO_CHAR);
      act("$n hoists $N above $s head and slams $M down upon $s knee.\n\rThere is a loud cracking sound as $N's spine snaps.", ch, NULL, victim, TO_NOTVICT);
      act("$n hoists you above $s head and slams you down upon $s knee.\n\rThere is a loud cracking sound as your spine snaps.", ch, NULL, victim, TO_VICT);
    }
    else if ( damp == 5 )
    {
      act("You lock your arm around $N's head, and give it a vicious twist.", ch, NULL, victim, TO_CHAR);
      act("$n locks $s arm around $N's head, and gives it a vicious twist.", ch, NULL, victim, TO_NOTVICT);
      act("$n locks $s arm around your head, and gives it a vicious twist.", ch, NULL, victim, TO_VICT);
      if (!IS_SET(victim->loc_hp,BROKEN_NECK))
      {
        act("There is a loud snapping noise as your neck breaks.", victim, NULL, NULL, TO_CHAR);
        act("There is a loud snapping noise as $n's neck breaks.", victim, NULL, NULL, TO_ROOM);
        SET_BIT(victim->loc_hp,BROKEN_NECK);
      }
    }
  }
  else
  if ((attack == attack_table[3].name
  ||   attack == attack_table[1].name)
  &&  (number_percent() < 2))
  {
    damp=number_range(1,8);
    if ( damp == 1 )
    {
      act("You swing your blade in a low arc, rupturing $N's abdominal cavity.\n\r$S entrails spray out over a wide area.", ch, NULL, victim, TO_CHAR);
      act("$n swings $s blade in a low arc, rupturing $N's abdominal cavity.\n\r$S entrails spray out over a wide area.", ch, NULL, victim, TO_NOTVICT);
      act("$n swings $s blade in a low arc, rupturing your abdominal cavity.\n\rYour entrails spray out over a wide area.", ch, NULL, victim, TO_VICT);
      make_part(victim,"entrails");
    }
    else if ( damp == 2 )
    {
      act("You thrust your blade into $N's mouth and twist it viciously.\n\rThe end of your blade bursts through the back of $S head.", ch, NULL, victim, TO_CHAR);
      act("$n thrusts $s blade into $N's mouth and twists it viciously.\n\rThe end of the blade bursts through the back of $N's head.", ch, NULL, victim, TO_NOTVICT);
      act("$n thrusts $s blade into your mouth and twists it viciously.\n\rYou feel the end of the blade burst through the back of your head.", ch, NULL, victim, TO_VICT);
    }
    else if ( damp == 3 )
    {
      if (!IS_SET(victim->loc_hp,CUT_THROAT))
          SET_BIT(victim->loc_hp,CUT_THROAT);
      if (!IS_SET(victim->loc_hp2,BLEEDING_THROAT))
      {
      SET_BIT(victim->loc_hp2,BLEEDING_THROAT);
      act("Your blow slices open $N's carotid artery, spraying blood everywhere.", ch, NULL, victim, TO_CHAR);
      act("$n's blow slices open $N's carotid artery, spraying blood everywhere.", ch, NULL, victim, TO_NOTVICT);
      act("$n's blow slices open your carotid artery, spraying blood everywhere.", ch, NULL, victim, TO_VICT);
      make_part(victim,"blood");
      }
    }
    else if ( damp == 4 )
    {
      if (!IS_SET(victim->loc_hp,CUT_THROAT))
        SET_BIT(victim->loc_hp,CUT_THROAT);
      if (!IS_SET(victim->loc_hp2,BLEEDING_THROAT))
      {
        SET_BIT(victim->loc_hp2,BLEEDING_THROAT);
      act("You swing your blade across $N's throat, showering the area with blood.", ch, NULL, victim, TO_CHAR);
      act("$n swings $s blade across $N's throat, showering the area with blood.", ch, NULL, victim, TO_NOTVICT);
      act("$n swings $s blade across your throat, showering the area with blood.", ch, NULL, victim, TO_VICT);
      make_part(victim,"blood");
      }
    }
    else if ( damp == 5 )
    {
      if (!IS_SET(victim->loc_hp,BROKEN_SKULL))
      {
        act("You swing your blade down upon $N's head, splitting it open.\n\r$N's brains pour out of $S forehead.", ch, NULL, victim, TO_CHAR);
        act("$n swings $s blade down upon $N's head, splitting it open.\n\r$N's brains pour out of $S forehead.", ch, NULL, victim, TO_NOTVICT);
        act("$n swings $s blade down upon your head, splitting it open.\n\rYour brains pour out of your forehead.", ch, NULL, victim, TO_VICT);
        make_part(victim,"brain");
        SET_BIT(victim->loc_hp,BROKEN_SKULL);
      }
      else
      {
        act("You plunge your blade deep into $N's chest.", ch, NULL, victim, TO_CHAR);
        act("$n plunges $s blade deep into $N's chest.", ch, NULL, victim, TO_NOTVICT);
        act("$n plunges $s blade deep into your chest.", ch, NULL, victim, TO_VICT);
      }
    }
    else if ( damp == 6 )
    {
      act("You swing your blade between $N's legs, nearly splitting $M in half.", ch, NULL, victim, TO_CHAR);
      act("$n swings $s blade between $N's legs, nearly splitting $M in half.", ch, NULL, victim, TO_NOTVICT);
      act("$n swings $s blade between your legs, nearly splitting you in half.", ch, NULL, victim, TO_VICT);
    }
    else if ( damp == 7 )
    {
      if (!IS_SET(victim->loc_hp2,LOST_ARM_L) && IS_SET(victim->parts,PART_ARMS))
      {
        act("You swing your blade in a wide arc, slicing off $N's arm.", ch, NULL, victim, TO_CHAR);
        act("$n swings $s blade in a wide arc, slicing off $N's arm.", ch, NULL, victim, TO_NOTVICT);
        act("$n swings $s blade in a wide arc, slicing off your arm.", ch, NULL, victim, TO_VICT);
        make_part(victim,"arm");
        SET_BIT(victim->loc_hp2,LOST_ARM_L);
        if (!IS_SET(victim->loc_hp2,BLEEDING_ARM))
           SET_BIT(victim->loc_hp2,BLEEDING_ARM);
        if (IS_SET(victim->loc_hp2,BLEEDING_HAND))
           REMOVE_BIT(victim->loc_hp2,BLEEDING_HAND);
      }
      else if (!IS_SET(victim->loc_hp,LOST_ARM_R) && IS_SET(victim->parts,PART_ARMS))
      {
        act("You swing your blade in a wide arc, slicing off $N's arm.", ch, NULL, victim, TO_CHAR);
        act("$n swings $s blade in a wide arc, slicing off $N's arm.", ch, NULL, victim, TO_NOTVICT);
        act("$n swings $s blade in a wide arc, slicing off your arm.", ch, NULL, victim, TO_VICT);
        make_part(victim,"arm");
        SET_BIT(victim->loc_hp,LOST_ARM_R);         if (!IS_SET(victim->loc_hp2,BLEEDING_ARM))
           SET_BIT(victim->loc_hp2,BLEEDING_ARM);
        if (IS_SET(victim->loc_hp2,BLEEDING_HAND))
           REMOVE_BIT(victim->loc_hp2,BLEEDING_HAND);
      }
      else
      {
        act("You plunge your blade deep into $N's chest.", ch, NULL, victim, TO_CHAR);
        act("$n plunges $s blade deep into $N's chest.", ch, NULL, victim, TO_NOTVICT);
        act("$n plunges $s blade deep into your chest.", ch, NULL, victim, TO_VICT);
      }
    }
    else if ( damp == 8 )
    {
      if (!IS_SET(victim->loc_hp2,LOST_LEG) && IS_SET(victim->parts,PART_LEGS))
      {
        act("You swing your blade in a low arc, slicing off $N's leg at the hip.", ch, NULL, victim, TO_CHAR);
        act("$n swings $s blade in a low arc, slicing off $N's leg at the hip.", ch, NULL, victim, TO_NOTVICT);
        act("$n swings $s blade in a wide arc, slicing off your leg at the hip.", ch, NULL, victim, TO_VICT);
        make_part(victim,"leg");
        SET_BIT(victim->loc_hp2,LOST_LEG);
        if (!IS_SET(victim->loc_hp2,BLEEDING_LEG))
           SET_BIT(victim->loc_hp2,BLEEDING_LEG);
        if (IS_SET(victim->loc_hp2,BLEEDING_FOOT))
           REMOVE_BIT(victim->loc_hp2,BLEEDING_FOOT);
      }
      else
      {
        act("You plunge your blade deep into $N's chest.", ch, NULL, victim, TO_CHAR);
        act("$n plunges $s blade deep into $N's chest.", ch, NULL, victim, TO_NOTVICT);
        act("$n plunges $s blade deep into your chest.", ch, NULL, victim, TO_VICT);
      }
    }
  }
  else
  if ((attack == attack_table[2].name
  ||   attack == attack_table[11].name)
  &&  (number_percent() < 2) )
  {
    damp=number_range(1,5);
    if ( damp == 1 )
    {
      act("You deftly invert your weapon and plunge it point first into $N's chest.\n\rA shower of blood sprays from the wound, showering the area.", ch, NULL, victim, TO_CHAR);
      act("$n deftly inverts $s weapon and plunge it point first into $N's chest.\n\rA shower of blood sprays from the wound, showering the area.", ch, NULL, victim, TO_NOTVICT);
      act("$n deftly inverts $s weapon and plunge it point first into your chest.\n\rA shower of blood sprays from the wound, showering the area.", ch, NULL, victim, TO_VICT);
      make_part(victim,"blood");
    }
    else if ( damp == 2 )
    {
      act("You thrust your blade into $N's mouth and twist it viciously.\n\rThe end of your blade bursts through the back of $S head.", ch, NULL, victim, TO_CHAR);
      act("$n thrusts $s blade into $N's mouth and twists it viciously.\n\rThe end of the blade bursts through the back of $N's head.", ch, NULL, victim, TO_NOTVICT);
      act("$n thrusts $s blade into your mouth and twists it viciously.\n\rYou feel the end of the blade burst through the back of your head.", ch, NULL, victim, TO_VICT);
    }
    else if ( damp == 3 )
    {
      act("You thrust your weapon up under $N's jaw and through $S head.", ch, NULL, victim, TO_CHAR);
      act("$n thrusts $s weapon up under $N's jaw and through $S head.", ch, NULL, victim, TO_NOTVICT);
      act("$n thrusts $s weapon up under your jaw and through your head.", ch, NULL, victim, TO_VICT);
    }
    else if ( damp == 4 )
    {
      act("You ram your weapon through $N's body, pinning $M to the ground.", ch, NULL, victim, TO_CHAR);
      act("$n rams $s weapon through $N's body, pinning $M to the ground.", ch, NULL, victim, TO_NOTVICT);
      act("$n rams $s weapon through your body, pinning you to the ground.", ch, NULL, victim, TO_VICT);
    }
    else if ( damp == 5 )
    {
      act("You stab your weapon into $N's eye and out the back of $S head.", ch, NULL, victim, TO_CHAR);
      act("$n stabs $s weapon into $N's eye and out the back of $S head.", ch, NULL, victim, TO_NOTVICT);
      act("$n stabs $s weapon into your eye and out the back of your head.", ch, NULL, victim, TO_VICT);
      if (!IS_SET(victim->loc_hp,LOST_EYE_L) && number_percent() < 50)
              SET_BIT(victim->loc_hp,LOST_EYE_L);
      else if (!IS_SET(victim->loc_hp,LOST_EYE_R))
              SET_BIT(victim->loc_hp,LOST_EYE_R);
      else if (!IS_SET(victim->loc_hp,LOST_EYE_L))
              SET_BIT(victim->loc_hp,LOST_EYE_L);
    }
  }
  else
  if ((attack == attack_table[6].name 
  ||   attack == attack_table[7].name
  ||   attack == attack_table[8].name)
  &&  (number_percent() < 2))
  {
    damp=number_range(1,3);
    if ( damp == 1)
    {
      act("Your blow smashes through $N's chest, caving in half $S ribcage.", ch, NULL, victim, TO_CHAR);
      act("$n's blow smashes through $N's chest, caving in half $S ribcage.", ch, NULL, victim, TO_NOTVICT);
      act("$n's blow smashes through your chest, caving in half your ribcage.", ch, NULL, victim, TO_VICT);
      if (IS_SET(victim->loc_hp,BROKEN_RIBS_1 ))
          REMOVE_BIT(victim->loc_hp,BROKEN_RIBS_1);
      if (IS_SET(victim->loc_hp,BROKEN_RIBS_2 ))
          REMOVE_BIT(victim->loc_hp,BROKEN_RIBS_2);
      if (IS_SET(victim->loc_hp,BROKEN_RIBS_4 ))
          REMOVE_BIT(victim->loc_hp,BROKEN_RIBS_4);
      if (IS_SET(victim->loc_hp,BROKEN_RIBS_8 ))
          REMOVE_BIT(victim->loc_hp,BROKEN_RIBS_8);
      if (IS_SET(victim->loc_hp,BROKEN_RIBS_16))
          REMOVE_BIT(victim->loc_hp,BROKEN_RIBS_16);
      if (!IS_SET(victim->loc_hp,BROKEN_RIBS_16))
          SET_BIT(victim->loc_hp,BROKEN_RIBS_16);
      if (!IS_SET(victim->loc_hp,BROKEN_RIBS_8))
          SET_BIT(victim->loc_hp,BROKEN_RIBS_8);
      if (!IS_SET(victim->loc_hp,BROKEN_RIBS_4))
          SET_BIT(victim->loc_hp,BROKEN_RIBS_4);
      if (!IS_SET(victim->loc_hp,BROKEN_RIBS_2))
          SET_BIT(victim->loc_hp,BROKEN_RIBS_2);
      if (!IS_SET(victim->loc_hp,BROKEN_RIBS_1))
          SET_BIT(victim->loc_hp,BROKEN_RIBS_1);
    }
    else if ( damp == 2)
    {
      act("Your blow smashes $N's spine, shattering it in several places.", ch, NULL, victim, TO_CHAR);
      act("$n's blow smashes $N's spine, shattering it in several places.", ch, NULL, victim, TO_NOTVICT);
      act("$n's blow smashes your spine, shattering it in several places.", ch, NULL, victim, TO_VICT);
      if (!IS_SET(victim->loc_hp,BROKEN_SPINE))
         SET_BIT(victim->loc_hp,BROKEN_SPINE);
    }
    else if ( damp == 3)
    {
      if (!IS_SET(victim->loc_hp,BROKEN_SKULL))
      {
        act("You swing your weapon down upon $N's head.\n\r$N's head cracks open like an overripe melon, leaking out brains.", ch, NULL, victim, TO_CHAR);
        act("$n swings $s weapon down upon $N's head.\n\r$N's head cracks open like an overripe melon, covering you with brains.", ch, NULL, victim, TO_NOTVICT);
        act("$n swings $s weapon down upon your head.\n\rYour head cracks open like an overripe melon, spilling your brains everywhere.", ch, NULL, victim, TO_VICT);
        make_part(victim,"brain");
        SET_BIT(victim->loc_hp,BROKEN_SKULL);
      }
      else
      {
        act("You hammer your weapon into $N's side, crushing bone.", ch, NULL, victim, TO_CHAR);
        act("$n hammers $s weapon into $N's side, crushing bone.", ch, NULL, victim, TO_NOTVICT);
        act("$n hammers $s weapon into your side, crushing bone.", ch, NULL, victim, TO_VICT);
      }
    }
  }
  else if ( !IS_NPC( ch )
       && (attack == attack_table[10].name) && (number_percent() < 40))
  {
    act("You sink your teeth into $N's throat and tear out $S jugular vein.\n\rYou wipe the blood from your chin with one hand.", ch, NULL, victim, TO_CHAR);
    act("$n sink $s teeth into $N's throat and tears out $S jugular vein.\n\r$n wipes the blood from $s chin with one hand.", ch, NULL, victim, TO_NOTVICT);
    act("$n sink $s teeth into your throat and tears out your jugular vein.\n\r$n wipes the blood from $s chin with one hand.", ch, NULL, victim, TO_VICT);
    make_part(victim,"blood");
    if (!IS_SET(victim->loc_hp,CUT_THROAT))
       SET_BIT(victim->loc_hp,CUT_THROAT);
    if (!IS_SET(victim->loc_hp2,BLEEDING_THROAT))
       SET_BIT(victim->loc_hp2,BLEEDING_THROAT);
  }
  else
  if ( !IS_NPC(ch)
  && (attack == attack_table[5].name
  ||  attack == attack_table[22].name)
  && (number_percent() < 2))
  {
    damp=number_range(1,2);
    if ( damp == 1 )
    {
      act("You tear out $N's throat, showering the area with blood.", ch, NULL, victim, TO_CHAR);
      act("$n tears out $N's throat, showering the area with blood.", ch, NULL, victim, TO_NOTVICT);
      act("$n tears out your throat, showering the area with blood.", ch, NULL, victim, TO_VICT);
      make_part(victim,"blood");
      if (!IS_SET(victim->loc_hp,CUT_THROAT))
         SET_BIT(victim->loc_hp,CUT_THROAT);
      if (!IS_SET(victim->loc_hp2,BLEEDING_THROAT))
         SET_BIT(victim->loc_hp2,BLEEDING_THROAT);
    }
    if ( damp == 2 )
    {
      if (!IS_SET(victim->loc_hp,LOST_EYE_L) && number_percent() < 50)
      {
        act("You rip an eyeball from $N's face.", ch, NULL, victim, TO_CHAR);
        act("$n rips an eyeball from $N's face.", ch, NULL, victim, TO_NOTVICT);
        act("$n rips an eyeball from your face.", ch, NULL, victim, TO_VICT);
        make_part(victim,"eyeball");
        SET_BIT(victim->loc_hp,LOST_EYE_L);
      }
      else if (!IS_SET(victim->loc_hp,LOST_EYE_R))
      {
        act("You rip an eyeball from $N's face.", ch, NULL, victim, TO_CHAR);
        act("$n rips an eyeball from $N's face.", ch, NULL, victim, TO_NOTVICT);
        act("$n rips an eyeball from your face.", ch, NULL, victim, TO_VICT);
        make_part(victim,"eyeball");
        SET_BIT(victim->loc_hp,LOST_EYE_R);
      }
      else if (!IS_SET(victim->loc_hp,LOST_EYE_L))
      {
        act("You rip an eyeball from $N's face.", ch, NULL, victim, TO_CHAR);
        act("$n rips an eyeball from $N's face.", ch, NULL, victim, TO_NOTVICT);
        act("$n rips an eyeball from your face.", ch, NULL, victim, TO_VICT);
        make_part(victim,"eyeball");
        SET_BIT(victim->loc_hp,LOST_EYE_L);
      }
      else
      {
        act("You claw open $N's chest.", ch, NULL, victim, TO_CHAR);
        act("$n claws open $N's chest.", ch, NULL, victim, TO_NOTVICT);
        act("$n claws open $N's chest.", ch, NULL, victim, TO_VICT);
      }
    }
  }
  else if ( attack == attack_table[4].name && (number_percent() < 15) )
  {
    act("You entangle $N around the neck, and squeeze out $S life.", ch, NULL, victim, TO_CHAR);
    act("$n entangles $N around the neck, and squeezes out $S life.", ch, NULL, victim, TO_NOTVICT);
    act("$n entangles you around the neck, and squeezes the life out of you.", ch, NULL, victim, TO_VICT);
    if (!IS_SET(victim->loc_hp,BROKEN_NECK))
       SET_BIT(victim->loc_hp,BROKEN_NECK);
  }
  else if ( (attack == attack_table[12].name
       || attack == attack_table[9].name)
       && (number_percent() < 15) )
  {
    act("You place your weapon on $N's head and suck out $S brains.", ch, NULL, victim, TO_CHAR);
    act("$n places $s weapon on $N's head and suck out $S brains.", ch, NULL, victim, TO_NOTVICT);
    act("$n places $s weapon on your head and suck out your brains.", ch, NULL, victim, TO_VICT);
  }
  else
  {}

    return;
}


/*
 * Disarm a creature.
 * Caller must check for successful attack.
 */
void disarm( CHAR_DATA *ch, CHAR_DATA *victim )
{
    OBJ_DATA *obj;
    OBJ_DATA *obj2;

    if ( IS_AFFECTED2( ch, AFF_INVUL ) )
    {
        send_to_char( "You are invulnerable.\n\r", ch );
        return;
    }

    if (IS_AFFECTED2(ch,AFF_RAINBOW_PATTERN))
    {
	send_to_char("You can't move at all to kill anyone.\n\r",ch);
	return;
    }

    if ( IS_AFFECTED2( ch, AFF_FEAR ))
    {
	  send_to_char("Fear still courses through your body, making it impossible to start fights.\n\r", ch );
	return;
    }

    if (IS_AFFECTED2(ch, AFF_PARALYSIS) )
    {
        send_to_char("You have been paralized! You couldn't possibly start a fight!\n\r",ch);
        return;
    }

    if (IS_AFFECTED3(ch, AFF_HOLDPERSON) )
    {
        send_to_char("You can't move at all, let alone start a fight!\n\r",ch);
        return;
    }

    if ( ch->riding )
    {
        send_to_char( "You can't get close enough while mounted.\n\r", ch );
        return;
    }

    if ( ( obj = get_eq_char( victim, WEAR_WIELD ) ) == NULL )
	return;

    if ( IS_OBJ_STAT(obj,ITEM_NOREMOVE))
    {
	act("$S weapon won't budge!",ch,NULL,victim,TO_CHAR);
	act("$n tries to disarm you, but your weapon won't budge!",
	    ch,NULL,victim,TO_VICT);
	act("$n tries to disarm $N, but fails.",ch,NULL,victim,TO_NOTVICT);
	return;
    }

    act( "$n DISARMS you and sends your weapon flying!", 
	 ch, NULL, victim, TO_VICT    );
    act( "You disarm $N!",  ch, NULL, victim, TO_CHAR    );
    act( "$n disarms $N!",  ch, NULL, victim, TO_NOTVICT );

    obj_from_char( obj );
    if ( IS_OBJ_STAT(obj,ITEM_NODROP) || IS_OBJ_STAT(obj,ITEM_INVENTORY) )
	obj_to_char( obj, victim );
    else
    {
	obj_to_room( obj, victim->in_room );
	if (IS_NPC(victim) && victim->wait == 0 && can_see_obj(victim,obj))
	    get_obj(victim,obj,NULL);
    }

    obj2 = get_eq_char(victim,WEAR_SECONDARY);
    if (get_eq_char (victim, WEAR_SECONDARY) != NULL)
    {
     remove_obj(victim, WEAR_SECONDARY, TRUE);
     equip_char ( victim, obj2, WEAR_WIELD);
     send_to_char ("You quickly switch your secondary weapon to your primary weapon.\n\r",victim);
    }

    return;
}

void do_berserk( CHAR_DATA *ch, char *argument)
{
    int chance, hp_percent;

    if ((chance = get_skill(ch,gsn_berserk)) == 0
    ||  (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_BERSERK))
    ||  (!IS_NPC(ch)
    &&   ch->level < skill_table[gsn_berserk].skill_level[ch->class]))
    {
	send_to_char("You turn red in the face, but nothing happens.\n\r",ch);
	return;
    }

    if (IS_AFFECTED(ch,AFF_BERSERK) || is_affected(ch,gsn_berserk)
    ||  is_affected(ch,skill_lookup("frenzy")))
    {
	send_to_char("You get a little madder.\n\r",ch);
	return;
    }

    if (IS_AFFECTED(ch,AFF_CALM))
    {
	send_to_char("You're feeling to mellow to berserk.\n\r",ch);
	return;
    }

    if (IS_AFFECTED2(ch,AFF_RAINBOW_PATTERN))
    {
	send_to_char("You're too fascinated with the rainbow in the sky to berserk.\n\r",ch);
	return;
    }

    if (ch->mana < 50)
    {
	send_to_char("You can't get up enough energy.\n\r",ch);
	return;
    }

    /* modifiers */

    /* fighting */
    if (ch->position == POS_FIGHTING)
	chance += 10;

    /* damage -- below 50% of hp helps, above hurts */
    hp_percent = 100 * ch->hit/ch->max_hit;
    chance += 25 - hp_percent/2;

    if (number_percent() < chance)
    {
	AFFECT_DATA af;

	WAIT_STATE(ch,PULSE_VIOLENCE);

      if((ch->race == race_lookup("aghar")
      ||  ch->race == race_lookup("neidar")
	||  ch->race == race_lookup("hylar")
	||  ch->race == race_lookup("theiwar")
	||  ch->race == race_lookup("daergar")
	||  ch->race == race_lookup("daewar")
	||  ch->race == race_lookup("klar"))
      &&  ch->class == class_lookup("warrior"))
	{
	}
	else
	{
	  ch->mana -= 50;
	  ch->move /= 2;
      }

	/* heal a little damage */
	ch->hit += ch->level * 2;
	ch->hit = UMIN(ch->hit,ch->max_hit);

	send_to_char("Your pulse races as you are consumed by rage!\n\r",ch);
	act("$n gets a wild look in $s eyes.",ch,NULL,NULL,TO_ROOM);
	check_improve(ch,gsn_berserk,TRUE,2);

	af.where	  = TO_AFFECTS;
	af.type	  = gsn_berserk;
	af.level	  = ch->level;
	af.duration	  = number_fuzzy(ch->level / 8);
	af.modifier	  = UMAX(1,ch->level/5);
	af.bitvector  = AFF_BERSERK;
	af.location	  = APPLY_HITROLL;
	affect_to_char(ch,&af);

	af.location	  = APPLY_DAMROLL;
	affect_to_char(ch,&af);

	af.modifier	  = UMAX(10,10 * (ch->level/5));
	af.location	  = APPLY_AC;
	affect_to_char(ch,&af);
    }

    else
    {
	WAIT_STATE(ch,3 * PULSE_VIOLENCE);

      if((ch->race == race_lookup("aghar")
      ||  ch->race == race_lookup("neidar")
	||  ch->race == race_lookup("hylar")
	||  ch->race == race_lookup("theiwar")
	||  ch->race == race_lookup("daergar")
	||  ch->race == race_lookup("daewar")
	||  ch->race == race_lookup("klar"))
      &&  ch->class == class_lookup("warrior"))
	{
	}
	else
	{      
	  ch->mana -= 25;
	  ch->move /= 2;
      }

	send_to_char("Your pulse speeds up, but nothing happens.\n\r",ch);
	check_improve(ch,gsn_berserk,FALSE,2);
    }
}

void do_bash( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;

    if ( IS_AFFECTED2( ch, AFF_INVUL ) )
    {
        send_to_char( "You are invulnerable.\n\r", ch );
        return;
    }

    if (IS_AFFECTED2(ch,AFF_RAINBOW_PATTERN))
    {
	send_to_char("You can't move at all to kill anyone.\n\r",ch);
	return;
    }

    if ( IS_AFFECTED2( ch, AFF_FEAR ))
    {
	  send_to_char("Fear still courses through your body, making it impossible to start fights.\n\r", ch );
	return;
    }

    if (IS_AFFECTED2(ch, AFF_PARALYSIS) )
    {
        send_to_char("You have been paralized! You couldn't possibly start a fight!\n\r",ch);
        return;
    }

    if (IS_AFFECTED3(ch, AFF_HOLDPERSON) )
    {
        send_to_char("You can't move at all, let alone start a fight!\n\r",ch);
        return;
    }

    if ( ch->riding )
    {
        send_to_char( "You can't get close enough while mounted.\n\r", ch );
        return;
    }

    if (ch->fight_pos > FIGHT_FRONT )
    {
        send_to_char("You are not close enough to bash!\n\r", ch);
        return;
    }

    one_argument(argument,arg);
 
    if ( (chance = get_skill(ch,gsn_bash)) == 0
    ||	 (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_BASH))
    ||	 (!IS_NPC(ch)
    &&	  ch->level < skill_table[gsn_bash].skill_level[ch->class]))
    {	
	send_to_char("Bashing? What's that?\n\r",ch);
	return;
    }
 
    if (arg[0] == '\0')
    {
	victim = ch->fighting;
	if (victim == NULL)
	{
	    send_to_char("But you aren't fighting anyone!\n\r",ch);
	    return;
	}
    }

    else if ((victim = get_char_room(ch, NULL, arg)) == NULL)
    {
	send_to_char("They aren't here.\n\r",ch);
	return;
    }

    if (victim->position < POS_FIGHTING)
    {
	act("You'll have to let $M get back up first.",ch,NULL,victim,TO_CHAR);
	return;
    } 

    if (victim == ch)
    {
	send_to_char("You try to bash your brains out, but fail.\n\r",ch);
	return;
    }

    if (is_safe(ch,victim))
	return;

    if ( IS_NPC(victim) && 
	victim->fighting != NULL && 
	!is_same_group(ch,victim->fighting))
    {
        send_to_char("Kill stealing is not permitted.\n\r",ch);
        return;
    }

    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
    {
	act("But $N is your friend!",ch,NULL,victim,TO_CHAR);
	return;
    }

    if (victim->riding && !IS_IMMORTAL(ch))
    {
      send_to_char("You are not big enough to bash a mounted person.\n\r",ch);
      return;
    }

    /* modifiers */

    /* size  and weight */
    chance += ch->carry_weight / 250;
    chance -= victim->carry_weight / 200;

    if (ch->size < victim->size)
	chance += (ch->size - victim->size) * 15;
    else
	chance += (ch->size - victim->size) * 10; 


    /* stats */
    chance += get_curr_stat(ch,STAT_STR);
    chance -= (get_curr_stat(victim,STAT_DEX) * 4)/3;
    chance -= GET_AC(victim,AC_BASH) /25;
    /* speed */
    if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE))
        chance += 10;
    if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,AFF_HASTE))
        chance -= 30;

    /* level */
    chance += (ch->level - victim->level);

    if (!IS_NPC(victim) 
	&& chance < get_skill(victim,gsn_dodge) )
    {	/*
        act("$n tries to bash you, but you dodge it.",ch,NULL,victim,TO_VICT);
        act("$N dodges your bash, you fall flat on your face.",ch,NULL,victim,TO_CHAR);
        WAIT_STATE(ch,skill_table[gsn_bash].beats);
        return;*/
	chance -= 3 * (get_skill(victim,gsn_dodge) - chance);
    }

    /* now the attack */
    if (number_percent() < chance )
    {
      if (!victim->riding)
      {
	act("$n sends you sprawling with a powerful bash!",
		ch,NULL,victim,TO_VICT);
	act("You slam into $N, and send $M flying!",ch,NULL,victim,TO_CHAR);
	act("$n sends $N sprawling with a powerful bash.",
		ch,NULL,victim,TO_NOTVICT);
      }
      else if (victim->riding && !IS_NPC(victim))
      {
      act("$n knocks you off your mount with a powerful bash!",
              ch,NULL,victim,TO_VICT); 
      act("You slam into $N, and send $M flying off $s mount!",
              ch,victim,victim,TO_CHAR);  
      act("$n sends $N sprawling with a powerful bash off $s mount!",
              ch,victim,victim,TO_NOTVICT);
	victim->riding->rider = NULL;
	victim->riding        = NULL;  
      }
	check_improve(ch,gsn_bash,TRUE,1);
	DAZE_STATE(victim, 3 * PULSE_VIOLENCE);
	WAIT_STATE(ch,skill_table[gsn_bash].beats);
	victim->position = POS_RESTING;
	damage(ch,victim,number_range(2,2 + 2 * ch->size + chance/20),gsn_bash,
	    DAM_BASH,FALSE);
	
    }
    else
    {
	damage(ch,victim,0,gsn_bash,DAM_BASH,FALSE);
	act("You fall flat on your face!",
	    ch,NULL,victim,TO_CHAR);
	act("$n falls flat on $s face.",
	    ch,NULL,victim,TO_NOTVICT);
	act("You evade $n's bash, causing $m to fall flat on $s face.",
	    ch,NULL,victim,TO_VICT);
	check_improve(ch,gsn_bash,FALSE,1);
	ch->position = POS_RESTING;
	WAIT_STATE(ch,skill_table[gsn_bash].beats * 3/2); 
    }
	check_killer(ch,victim);
}

void do_dirt( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;

    if ( IS_AFFECTED2( ch, AFF_INVUL ) )
    {
        send_to_char( "You are invulnerable.\n\r", ch );
        return;
    }

    if (IS_AFFECTED2(ch,AFF_RAINBOW_PATTERN))
    {
	send_to_char("You can't move at all to kill anyone.\n\r",ch);
	return;
    }

    if ( IS_AFFECTED2( ch, AFF_FEAR ))
    {
	  send_to_char("Fear still courses through your body, making it impossible to start fights.\n\r", ch );
	return;
    }

    if (IS_AFFECTED2(ch, AFF_PARALYSIS) )
    {
        send_to_char("You have been paralized! You couldn't possibly start a fight!\n\r",ch);
        return;
    }

    if (IS_AFFECTED3(ch, AFF_HOLDPERSON) )
    {
        send_to_char("You can't move at all, let alone start a fight!\n\r",ch);
        return;
    }

    if ( ch->riding )
    {
        send_to_char( "Your feet aren't even touching the deck.\n\r", ch );
        return;
    }

    one_argument(argument,arg);

    if ( (chance = get_skill(ch,gsn_dirt)) == 0
    ||   (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_KICK_DIRT))
    ||   (!IS_NPC(ch)
    &&    ch->level < skill_table[gsn_dirt].skill_level[ch->class]))
    {
	send_to_char("You get your feet dirty.\n\r",ch);
	return;
    }

    if (arg[0] == '\0')
    {
	victim = ch->fighting;
	if (victim == NULL)
	{
	    send_to_char("But you aren't in combat!\n\r",ch);
	    return;
	}
    }

    else if ((victim = get_char_room(ch, NULL, arg)) == NULL)
    {
	send_to_char("They aren't here.\n\r",ch);
	return;
    }

    if (IS_AFFECTED(victim,AFF_BLIND))
    {
	act("$E's already been blinded.",ch,NULL,victim,TO_CHAR);
	return;
    }

    if (victim == ch)
    {
	send_to_char("Very funny.\n\r",ch);
	return;
    }

    if (is_safe(ch,victim))
	return;

    if (IS_NPC(victim) &&
	 victim->fighting != NULL && 
	!is_same_group(ch,victim->fighting))
    {
        send_to_char("Kill stealing is not permitted.\n\r",ch);
        return;
    }

    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
    {
	act("But $N is such a good friend!",ch,NULL,victim,TO_CHAR);
	return;
    }

    /* modifiers */

    /* dexterity */
    chance += get_curr_stat(ch,STAT_DEX);
    chance -= 2 * get_curr_stat(victim,STAT_DEX);

    /* speed  */
    if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE))
	chance += 10;
    if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,AFF_HASTE))
	chance -= 25;

    /* level */
    chance += (ch->level - victim->level) * 2;

    /* sloppy hack to prevent false zeroes */
    if (chance % 5 == 0)
	chance += 1;

    /* terrain */

    switch(ch->in_room->sector_type)
    {
	case(SECT_INSIDE):		chance -= 20;	break;
	case(SECT_CITY):		chance -= 10;	break;
	case(SECT_FIELD):		chance +=  5;	break;
	case(SECT_FOREST):				break;
	case(SECT_HILLS):				break;
	case(SECT_MOUNTAIN):		chance -= 10;	break;
	case(SECT_WATER_SWIM):		chance  =  0;	break;
	case(SECT_WATER_NOSWIM):	chance  =  0;	break;
	case(SECT_AIR):			chance  =  0;  	break;
	case(SECT_DESERT):		chance += 10;   break;
    }

    if (chance == 0)
    {
	send_to_char("There isn't any dirt to kick.\n\r",ch);
	return;
    }

    /* now the attack */
    if (number_percent() < chance)
    {
	AFFECT_DATA af;
	act("$n is blinded by the dirt in $s eyes!",victim,NULL,NULL,TO_ROOM);
	act("$n kicks dirt in your eyes!",ch,NULL,victim,TO_VICT);
        damage(ch,victim,number_range(2,5),gsn_dirt,DAM_NONE,FALSE);
	send_to_char("You can't see a thing!\n\r",victim);
	check_improve(ch,gsn_dirt,TRUE,2);
	WAIT_STATE(ch,skill_table[gsn_dirt].beats);

	af.where		= TO_AFFECTS;
	af.type 		= gsn_dirt;
	af.level 		= ch->level;
	af.duration		= 1;
	af.location		= APPLY_HITROLL;
	af.modifier		= -4;
	af.bitvector 	= AFF_BLIND;

	affect_to_char(victim,&af);
    }
    else
    {
	damage(ch,victim,0,gsn_dirt,DAM_NONE,TRUE);
	check_improve(ch,gsn_dirt,FALSE,2);
	WAIT_STATE(ch,skill_table[gsn_dirt].beats);
    }
	check_killer(ch,victim);
}

void do_trip( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;

    if ( IS_AFFECTED2( ch, AFF_INVUL ) )
    {
        send_to_char( "You are invulnerable.\n\r", ch );
        return;
    }

    if (IS_AFFECTED2(ch,AFF_RAINBOW_PATTERN))
    {
	send_to_char("You can't move at all to kill anyone.\n\r",ch);
	return;
    }

    if ( IS_AFFECTED2( ch, AFF_FEAR ))
    {
	  send_to_char("Fear still courses through your body, making it impossible to start fights.\n\r", ch );
	return;
    }

    if (IS_AFFECTED2(ch, AFF_PARALYSIS) )
    {
        send_to_char("You have been paralized! You couldn't possibly start a fight!\n\r",ch);
        return;
    }

    if (IS_AFFECTED3(ch, AFF_HOLDPERSON) )
    {
        send_to_char("You can't move at all, let alone start a fight!\n\r",ch);
        return;
    }

    if ( ch->riding )
    {
        send_to_char( "You can't trip anyone while mounted.\n\r", ch );
        return;
    }

    if ( ch->fight_pos > FIGHT_FRONT )
    {
        send_to_char("You aren't close enough to trip!\n\r",ch);
        return;
    }

    one_argument(argument,arg);

    if ( (chance = get_skill(ch,gsn_trip)) == 0
    ||   (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_TRIP))
    ||   (!IS_NPC(ch) 
	  && ch->level < skill_table[gsn_trip].skill_level[ch->class]))
    {
	send_to_char("Tripping?  What's that?\n\r",ch);
	return;
    }


    if (arg[0] == '\0')
    {
	victim = ch->fighting;
	if (victim == NULL)
	{
	    send_to_char("But you aren't fighting anyone!\n\r",ch);
	    return;
 	}
    }

    else if ((victim = get_char_room(ch, NULL, arg)) == NULL)
    {
	send_to_char("They aren't here.\n\r",ch);
	return;
    }

    if (is_safe(ch,victim))
	return;

    if (IS_NPC(victim) &&
	 victim->fighting != NULL && 
	!is_same_group(ch,victim->fighting))
    {
	send_to_char("Kill stealing is not permitted.\n\r",ch);
	return;
    }
    
    if (IS_AFFECTED(victim,AFF_FLYING)
    || IS_AFFECTED2(ch,AFF_GLIDE)
    || victim->riding)
    {
	act("$S feet aren't on the ground.",ch,NULL,victim,TO_CHAR);
	return;
    }

    if (victim->position < POS_FIGHTING)
    {
	act("$N is already down.",ch,NULL,victim,TO_CHAR);
	return;
    }

    if (victim == ch)
    {
	send_to_char("You fall flat on your face!\n\r",ch);
	WAIT_STATE(ch,2 * skill_table[gsn_trip].beats);
	act("$n trips over $s own feet!",ch,NULL,NULL,TO_ROOM);
	return;
    }

    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
    {
	act("$N is your beloved master.",ch,NULL,victim,TO_CHAR);
	return;
    }

    /* modifiers */

    /* size */
    if (ch->size < victim->size)
        chance += (ch->size - victim->size) * 10;  /* bigger = harder to trip */

    /* dex */
    chance += get_curr_stat(ch,STAT_DEX);
    chance -= get_curr_stat(victim,STAT_DEX) * 3 / 2;

    /* speed */
    if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE))
	chance += 10;
    if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,AFF_HASTE))
	chance -= 20;

    /* level */
    chance += (ch->level - victim->level) * 2;

    if (number_percent() < chance)
    {
	act("$n trips you and you go down!",ch,NULL,victim,TO_VICT);
	act("You trip $N and $N goes down!",ch,NULL,victim,TO_CHAR);
	act("$n trips $N, sending $M to the ground.",ch,NULL,victim,TO_NOTVICT);
	check_improve(ch,gsn_trip,TRUE,1);

	DAZE_STATE(victim,2 * PULSE_VIOLENCE);
        WAIT_STATE(ch,skill_table[gsn_trip].beats);
	victim->position = POS_RESTING;
	damage(ch,victim,number_range(2, 2 +  2 * victim->size),gsn_trip,
	    DAM_BASH,TRUE);
    }
    else
    {
	damage(ch,victim,0,gsn_trip,DAM_BASH,TRUE);
	WAIT_STATE(ch,skill_table[gsn_trip].beats*2/3);
	check_improve(ch,gsn_trip,FALSE,1);
    } 
	check_killer(ch,victim);
}



void do_kill( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;


    if ( IS_AFFECTED2( ch, AFF_INVUL ) )
    {
        send_to_char( "You are invulnerable.\n\r", ch );
        return;
    }

    if (ch->class == class_lookup("blacksmith"))
    {
         send_to_char( "You are Blacksmith.. you couldn't fight your way out a paper bag.\n\r", ch );
        return;
    }

    if (IS_AFFECTED2(ch,AFF_RAINBOW_PATTERN))
    {
	send_to_char("You can't move at all to kill anyone.\n\r",ch);
	return;
    }

    if ( IS_AFFECTED2( ch, AFF_FEAR ))
    {
	  send_to_char("Fear still courses through your body, making it impossible to start fights.\n\r", ch );
	return;
    }

    if (IS_AFFECTED2(ch, AFF_PARALYSIS) )
    {
        send_to_char("You have been paralized! You couldn't possibly start a fight!\n\r",ch);
        return;
    }

    if (IS_AFFECTED3(ch, AFF_HOLDPERSON) )
    {
        send_to_char("You can't move at all, let alone start a fight!\n\r",ch);
        return;
    }

    if (ch->fight_pos > FIGHT_MIDDLE) 
    {
	if (ch->fighting)
	{
		send_to_char("You cannot engage your attacker from this range.\r\n",ch);
		return;
	}
	else
	{
		send_to_char("You advance out of the rear rank towards your enemy.\r\n", ch);
		ch->fight_pos--;
	}
    }

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Kill whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, NULL, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( !IS_NPC(victim)  && !IS_NPC(ch))
    {
        if ( !IS_SET(victim->act, PLR_KILLER)
        &&   !IS_SET(victim->act, PLR_THIEF) )
        {
            send_to_char( "You must MURDER a player.\n\r", ch );
            return;
        }
    }

    if ( victim == ch )
    {
	send_to_char( "You hit yourself.  Ouch!\n\r", ch );
	multi_hit( ch, ch, TYPE_UNDEFINED );
	return;
    }
    
    if( IS_AFFECTED2(victim, AFF_INVUL) || victim->position == POS_DEAD)
    {
        act( "$N is invulnerable.", ch, NULL, victim, TO_CHAR );
        act( "You are invulnerable to $n.", ch, NULL, victim, TO_VICT );
        return;
    }

    if ( is_safe( ch, victim ) )
	return;

    if ( victim->fighting != NULL && 
	!is_same_group(ch,victim->fighting))
    {
        send_to_char("Kill stealing is not permitted.\n\r",ch);
        return;
    }

    if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
    {
	act( "$N is your beloved master.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( ch->position == POS_FIGHTING )
    {
	send_to_char( "You do the best you can!\n\r", ch );
	return;
    }

    WAIT_STATE( ch, 1 * PULSE_VIOLENCE );
    check_killer( ch, victim );
    multi_hit( ch, victim, TYPE_UNDEFINED );
    return;
}



void do_murde( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to MURDER, spell it out.\n\r", ch );
    return;
}



void do_murder( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    if (IS_AFFECTED2(ch,AFF_RAINBOW_PATTERN))
    {
	send_to_char("You can't move at all, let alone murder anyone.\n\r",ch);
	return;
    }

    if (ch->class == class_lookup("blacksmith"))
    {
         send_to_char( "You are Blacksmith.. you couldn't fight your way out a paper bag.\n\r", ch );
        return;
    }

    if( IS_AFFECTED2(ch, AFF_INVUL) )
    {
        send_to_char( "You are invulnerable.\n\r", ch );
        return;
    }

    if ( IS_AFFECTED2( ch, AFF_FEAR ))
    {
	  send_to_char("Fear still courses through your body, making it impossible to start fights.\n\r", ch );
	return;
    }

    if (IS_AFFECTED2(ch, AFF_PARALYSIS) )
    {
      send_to_char("You have been paralized! You couldn't possibly start a fight!\n\r",ch);
      return;
    }

    if (IS_AFFECTED3(ch, AFF_HOLDPERSON) )
    {
        send_to_char("You can't move at all, let alone start fighting!\n\r",ch);
        return;
    }

    if (ch->fight_pos > FIGHT_MIDDLE) 
    {
	if (ch->fighting)
	{
		send_to_char("You cannot engage your attacker from this range.\r\n",ch);
		return;
	}
	else
	{
		send_to_char("You advance out of the rear rank towards your enemy.\r\n", ch);
		ch->fight_pos--;
	}
    }

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Murder whom?\n\r", ch );
	return;
    }

    if (IS_AFFECTED(ch,AFF_CHARM) || (IS_NPC(ch) && IS_SET(ch->act,ACT_PET)))
	return;

    if ( ( victim = get_char_room( ch, NULL, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "Yes, that's what we need.... more suicide!\n\r", ch );
	return;
    }
    
    if( IS_AFFECTED2(victim, AFF_INVUL) || victim->position == POS_DEAD)
    {
        act( "$N is invulnerable.", ch, NULL, victim, TO_CHAR );
        act( "You are invulnerable to $n.", ch, NULL, victim, TO_VICT );
        return;
    }

    if ( is_safe( ch, victim ) )
	return;

    if( IS_AFFECTED3(victim, AFF_SUBDUE)
    &&  IS_SET(ch->act,PLR_AUTOMERCY))
      return;

    if (!IS_NPC(ch) && !IS_NPC(victim)
    && (ABS(ch->level - victim->level) > 10)
    && (ch->level <= LEVEL_HERO)
    && IS_AFFECTED3(victim,AFF_SUBDUE))
      return;

    if (IS_NPC(victim)
    &&  victim->fighting != NULL
    && !is_same_group(ch,victim->fighting))
    {
        send_to_char("Kill stealing is not permitted.\n\r",ch);
        return;
    }

    if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
    {
	act( "$N is your beloved master.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( ch->position == POS_FIGHTING )
    {
	send_to_char( "You do the best you can!\n\r", ch );
	return;
    }

    WAIT_STATE( ch, 1 * PULSE_VIOLENCE );
    if (!can_see(victim,ch)) 
        do_yell(victim, "Help! Someone is attacking me!");
    else
    {
     if (IS_NPC(ch))
         sprintf(buf, "Help! %s is attacking me!",  ch->short_descr);
     else
         sprintf(buf, "Help! %s is attacking me!", 
         (is_affected(ch,gsn_doppelganger) 
         && (!IS_IMMORTAL(victim)))? ch->doppel->name :
	   (is_affected(ch,gsn_treeform) 
         && (!IS_IMMORTAL(victim)))? "A mighty oak tree" :
	   ch->name);
     do_yell( victim,buf );
    }
    check_killer( ch, victim );
    multi_hit( ch, victim, TYPE_UNDEFINED );
    return;
}



void do_backstab( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  int dam_type, fight, backstab_bonus = 0;

  if ( ch->riding )
  {
    send_to_char( "You can't get close enough while mounted.\n\r", ch );
    return;
  }

  one_argument( argument, arg );

  if (arg[0] == '\0')
  {
    send_to_char("Backstab whom?\n\r",ch);
    return;
  }

  if ( !IS_NPC( ch ) 
  && ch->level < skill_table[gsn_backstab].skill_level[ch->class] )
  {
    send_to_char( "You don't know how to do that.\n\r", ch );
    return;
  }

  if ( IS_AFFECTED2(ch, AFF_PARALYSIS) 
  ||   IS_AFFECTED3(ch, AFF_HOLDPERSON))
  {
    send_to_char("You can't move at all, there's no way you can do that.\n\r",ch);
    return;
  }

  if (ch->fighting != NULL)
  {
    send_to_char("You're facing the wrong end.\n\r",ch);
    return;
  }

  if (ch->fight_pos > FIGHT_FRONT)
  {
    send_to_char("You are too far away to backstab!\r\n", ch);
    return;
  }
  else
  if ((victim = get_char_room(ch, NULL, arg)) == NULL)
  {
    send_to_char("They aren't here.\n\r",ch);
    return;
  }

  if ( victim == ch )
  {
    send_to_char( "How can you sneak up on yourself?\n\r", ch );
    return;
  }

  if ( is_safe( ch, victim ) )
    return;

  if (IS_NPC(victim)
  && victim->fighting != NULL
  && !is_same_group(ch,victim->fighting))
  {
    send_to_char("Kill stealing is not permitted.\n\r",ch);
    return;
  }

  if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL)
  {
    send_to_char( "You need to wield a weapon to backstab.\n\r", ch );
    return;
  }

  if (obj->value[0] != WEAPON_DAGGER )
  {
    send_to_char ("You can only backstab with daggers.\n\r", ch );
    return;
  }

  if ( victim->fighting != NULL )
  {
    send_to_char( "You can't backstab a fighting person.\n\r", ch );
    return;
  }

  if ((victim->max_hit == 0 )
  ||  (victim->hit == 0))
  {
    act( "$N can not be backstabbed!.",ch,NULL,victim,TO_CHAR );
    return;
  }

  if ( (victim->hit * 100 / victim->max_hit) < 90 )
  {
    act( "$N is hurt and suspicious ... you can't sneak up.", ch, NULL, victim, TO_CHAR );
    return;
  }

  if (!IS_AFFECTED(ch, AFF_SNEAK))
  {
    send_to_char("You are not sneaky enough to backstab.\n\r",ch);
    return;
  }

  dam_type = attack_table[obj->value[3]].damage;
  if (victim->position == POS_SLEEPING)
  {
    backstab_bonus = 10;
  }

  fight = 0;

  if ( victim->position <= POS_SLEEPING )
    fight = 1;

  check_killer( ch, victim );
  WAIT_STATE( ch, skill_table[gsn_backstab].beats );
  if ( !IS_AWAKE(victim)
  ||   IS_NPC(ch)
  ||   number_percent( ) < (ch->pcdata->learned[gsn_backstab]+backstab_bonus) )
  {
    check_improve(ch,gsn_backstab,TRUE,1);
    one_hit( ch, victim, gsn_backstab,FALSE );
    if( (obj = get_eq_char(ch, WEAR_SECONDARY)) != NULL)
    {
      /* see if we can get a double one launched */
      if( !IS_NPC(ch)  /* pets can't get double */
      && number_percent () < ((ch->pcdata->learned[gsn_double_bs]+backstab_bonus) * 3) / 4 )
      {
        /* deal a 2nd blow */
        one_hit( ch, victim, gsn_double_bs,TRUE );
        check_improve(ch,gsn_double_bs,TRUE,1);
      }
    }
    if (!IS_NPC(victim) && fight != 1)
    {
      if (!can_see(victim,ch) ) 
        do_yell(victim, "Argh! Someone just plunged a blade in my back!");
      else
      {
      if (IS_NPC(ch))
        sprintf(buf, "Argh! %s just plunged a blade in my back!", ch->short_descr);
      else
        sprintf(buf, "Argh! %s just plunged a blade in my back!", 
        ((is_affected(ch,gsn_doppelganger)) && (!IS_IMMORTAL(victim)))? ch->doppel->name :
        ((is_affected(ch,gsn_treeform)) && (!IS_IMMORTAL(victim)))? "A mighty oak tree" : 
	    ch->name);
        do_yell( victim, buf );
      }
    }
  }
  else
  {
    if (!IS_NPC(victim) && fight != 1)
    {
      if (!can_see(victim,ch) ) 
        do_yell(victim, "Help! Someone tried to backstab me!");
      else
      {
        if (IS_NPC(ch))
          sprintf(buf, "%s, you will die you backstabbing scum!", ch->short_descr);
        else 
          sprintf(buf, "%s, you will die you backstabbing scum!", 
          ((is_affected(ch,gsn_doppelganger)) && (!IS_IMMORTAL(victim)))? ch->doppel->name :
          ((is_affected(ch,gsn_treeform)) && (!IS_IMMORTAL(victim)))? "A mighty oak tree" :
	      ch->name);
          do_yell( victim, buf );
      }
    }
    check_improve(ch,gsn_backstab,FALSE,1);
    damage( ch, victim, 0, gsn_backstab,DAM_NONE, TRUE );
  }

  if (IS_AFFECTED(ch, AFF_SNEAK))
  {
    affect_strip(ch,skill_lookup("sneak"));
    REMOVE_BIT( ch->affected_by, AFF_SNEAK );
  }

  return;
}

void do_circle( CHAR_DATA *ch, char *argument )
{
  OBJ_DATA  *obj;
  CHAR_DATA *victim;
  char arg [ MAX_INPUT_LENGTH ];

  if ( ch->riding )
  {
    send_to_char( "You can't circle while mounted.\n\r", ch );
    return;
  }

  if (!IS_NPC( ch ) 
  && ch->level < skill_table[gsn_circle].skill_level[ch->class] )
  {
    send_to_char( "You don't know how to do that.\n\r", ch );
    return;
  }

  if ( IS_NPC( ch ) && IS_AFFECTED( ch, AFF_CHARM ) )
    return;

  if ( !ch->fighting )
  {
    send_to_char( "You must be in a fight in order to do that.\n\r", ch );
    return;
  }

  one_argument( argument, arg );
    
  if ( arg[0] == '\0' )
    victim = ch->fighting;
  else
  if ( !( victim = get_char_room( ch, NULL, arg ) ) )
  {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  if ( victim == ch )
  {
    send_to_char( "You spin around in a circle.  Whee!\n\r", ch );
    return;
  }

  if ( is_safe( ch, victim ) )
    return;

  if ( victim != ch->fighting )
  {
    send_to_char( "One fight at a time.\n\r", ch );
    return;
  }

  if ( !victim->fighting )
  {
    act( "Why?  $E isn't bothering anyone.", ch, NULL, victim, TO_CHAR );
    return;
  }

  if ( !is_same_group( ch, victim->fighting ) )
  {
    send_to_char( "Why call attention to yourself?\n\r", ch );
    return;
  }

  /* Make sure they're in the middle line */
  if( ch->fight_pos == FIGHT_FRONT )
  {
    send_to_char("You're too close to the action to circlestab.\n\r", ch);
    return;
  }
  else if (ch->fight_pos == FIGHT_REAR)
  {
    send_to_char("You're too far away from the action to circlestab.\r\n",ch);
    return;
  }

  if( victim->fighting == ch )
  {
    send_to_char("You're too busy defending yourself to circlestab.\n\r",ch );
    return;
  }

  if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL)
  {
    send_to_char( "You need to wield a weapon to circle.\n\r", ch );
    return;
  }

  if ( obj->value[0] != WEAPON_DAGGER )
  {
    send_to_char ("You can only circlestab with daggers.\n\r",ch);
    return;
  }

  act( "You circle around behind $N...", ch, NULL, victim, TO_CHAR    );
  act( "$n circles around behind $N...", ch, NULL, victim, TO_NOTVICT );
    
  check_killer( ch, victim );
  WAIT_STATE( ch, skill_table[gsn_circle].beats );
  if ( number_percent( ) < get_skill(ch,gsn_circle)
  || ( get_skill(ch,gsn_circle) >= 2 && !IS_AWAKE(victim) ) )
  {
    check_improve(ch,gsn_circle,TRUE,1);
    multi_hit( ch, victim, gsn_circle );
  }
  else
  {
    act( "You failed to get around $M", ch, NULL, victim, TO_CHAR );
    check_improve( ch, gsn_circle, FALSE, 1 );
    damage( ch, victim, 0, gsn_circle,DAM_NONE,TRUE);
  }

    return;
}

void do_flee( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *was_in;
    ROOM_INDEX_DATA *now_in;
    CHAR_DATA *victim;
    int attempt;

    if ( ( victim = ch->fighting ) == NULL )
    {
        if ( ch->position == POS_FIGHTING )
            ch->position = POS_STANDING;
	send_to_char( "You aren't fighting anyone.\n\r", ch );
	return;
    }

    if (ch->riding)
    {
	send_to_char( "If you want to flee, dismount first.\n\r", ch );
      return;
    }
    
    if (IS_NPC(victim)
    && !IS_NPC(ch)
    && (victim->spec_fun == spec_lookup("spec_orgguard"))
    && (victim->org_id != ORG_NONE)
    && !SAME_ORG(ch,victim))
    {
	send_to_char( "Your attempts to flee have been thwarted!\n\r", ch );
      return;
    }

    if (IS_AFFECTED3(ch, AFF_QUICKSAND) )
    {
	if (number_percent() > 50)
      {

	 if ( ch->move < 50 )
	 {
	   send_to_char( "You are too exhausted.\n\r", ch );
	   return;
	 }

        act("You try to flee, but the quicksand holds you firmly in the ground!", ch,NULL,NULL,TO_CHAR);
        act("$n tries to flee, but the quicksand holds $m firmly in the ground!", ch,NULL,NULL,TO_ROOM);
	  WAIT_STATE( ch, 15 );
	  return;
      }
      else
      {
        act("You successfully break free of the quicksand, but feel extremely exhausted from the process.", ch,NULL,NULL,TO_CHAR);
        act("$n successfully breaks free of the quicksand, but now looks extremely exhausted from the process", ch,NULL,NULL,TO_ROOM);
	  affect_strip(ch,skill_lookup("quicksand"));
	  ch->move = ch->move/2;
      }
    }

    was_in = ch->in_room;
    for ( attempt = 0; attempt < 6; attempt++ )
    {
	EXIT_DATA *pexit;
	int door;

	door = number_door( );
	if ( ( pexit = was_in->exit[door] ) == 0
	||   pexit->u1.to_room == NULL
	||   IS_SET(pexit->exit_info, EX_CLOSED)
	||   number_range(0,ch->daze) != 0
	|| ( IS_NPC(ch)
	&&   IS_SET(pexit->u1.to_room->room_flags, ROOM_NO_MOB) ) )
	    continue;

	move_char( ch, door, FALSE );
	if ( ( now_in = ch->in_room ) == was_in )
	    continue;

	ch->in_room = was_in;
	act( "$n has fled!", ch, NULL, NULL, TO_ROOM );
	ch->in_room = now_in;

	if ( !IS_NPC(ch) )
	{
	    send_to_char( "You flee from combat!\n\r", ch );
	if( (ch->class == 2) 
	    && (number_percent() < 3*(ch->level/2) ) )
		send_to_char( "You snuck away safely.\n\r", ch);
	else
	    {
	    send_to_char( "Curse the Gods, you've lost 10 experience points.\n\r", ch); 
	    gain_exp( ch, -10 );
	    }

          if (ch->riding )
          {
             move_char( ch->riding, door, FALSE );  
             ch->riding->in_room = ch->in_room;
             stop_fighting(ch->riding,TRUE);
          }
	}
	stop_fighting( ch, TRUE );
	return;
    }

    send_to_char( "You attempt to flee from combat, but can't escape!\n\r", ch );
    return;
}



void do_rescue( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *fch;

    if ( ch->riding )
    {
        send_to_char( "You can't do that while mounted.\n\r", ch );
        return;
    }

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Rescue whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, NULL, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "What about fleeing instead?\n\r", ch );
	return;
    }

    if (IS_AFFECTED2(victim, AFF_INVUL))
    {
	send_to_char("They are invulnerable and need no assistance.",ch);
	return;
    }

    if ( !IS_NPC(ch) && IS_NPC(victim) )
    {
	send_to_char( "Doesn't need your help!\n\r", ch );
	return;
    }

    if ( ch->fighting == victim )
    {
	send_to_char( "Too late.\n\r", ch );
	return;
    }

    if ( ( fch = victim->fighting ) == NULL )
    {
	send_to_char( "That person is not fighting right now.\n\r", ch );
	return;
    }

    if ( IS_NPC(fch) && !is_same_group(ch,victim))
    {
        send_to_char("Kill stealing is not permitted.\n\r",ch);
        return;
    }

    WAIT_STATE( ch, skill_table[gsn_rescue].beats );
    if ( number_percent( ) > get_skill(ch,gsn_rescue))
    {
	send_to_char( "You fail the rescue.\n\r", ch );
	check_improve(ch,gsn_rescue,FALSE,1);
	return;
    }

    act( "You rescue $N!",  ch, NULL, victim, TO_CHAR    );
    act( "$n rescues you!", ch, NULL, victim, TO_VICT    );
    act( "$n rescues $N!",  ch, NULL, victim, TO_NOTVICT );
    check_improve(ch,gsn_rescue,TRUE,1);
    stop_fighting( fch, FALSE );
    stop_fighting( victim, FALSE );
    check_killer( ch, fch );
    if (!ch->fighting)
    {
    set_fighting( ch, fch );
    }
    if (!fch->fighting)
    {
    set_fighting( fch, ch );
    }
    ch->fight_pos = FIGHT_FRONT;
    if (ch->size >= victim->size ) 
    {
	if (ch->fight_pos != FIGHT_REAR)
	    victim->fight_pos = FIGHT_REAR;
    }
    return;
}



void do_kick( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    int damp;

    if ( ch->riding )
    {
        send_to_char( "You can't get close enough while mounted.\n\r", ch );
        return;
    }

    if ( ch->fight_pos > FIGHT_FRONT )
    {
       send_to_char( "You aren't close enough to kick!\n\r",ch);
       return;
    }
    if (!IS_NPC(ch) && !IS_SET(ch->parts, PART_LEGS))
    {
        send_to_char("You require the legs to support your balance", ch);
        return;
    } 

    if ( !IS_NPC(ch)
    &&   ch->level < skill_table[gsn_kick].skill_level[ch->class] )
    {
	send_to_char(
	    "You better leave the martial arts to fighters.\n\r", ch );
	return;
    }

    if ( IS_AFFECTED2(ch, AFF_PARALYSIS) 
    ||   IS_AFFECTED3(ch, AFF_HOLDPERSON))
    {
      send_to_char("You can't move at all, there's no way you can do that.\n\r",ch);
      return;
    }

    if (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_KICK))
	return;

    if ( ( victim = ch->fighting ) == NULL )
    {
	send_to_char( "You aren't fighting anyone.\n\r", ch );
	return;
    }

    WAIT_STATE( ch, skill_table[gsn_kick].beats );
    if ( get_skill(ch,gsn_kick) > number_percent())
    {
      damp=number_range(1,2);
      if ( damp == 1 )
      {
	  if (!IS_SET(victim->loc_hp2,BROKEN_LEG) && !IS_SET(victim->loc_hp2,LOST_LEG))
        {
	  SET_BIT(victim->loc_hp2,BROKEN_LEG);
	  act("Your powerful kick breaks $N's leg!",ch,NULL,victim,TO_CHAR);
	  act("$n's powerful kick breaks $N's leg!",ch,NULL,victim,TO_NOTVICT);
	  act("$n's powerful kick breaks your leg!",ch,NULL,victim,TO_VICT);
        }
      }
      else if ( damp == 2 )
      {
	if (!IS_SET(victim->loc_hp,LOST_TOOTH_16))
	    SET_BIT(victim->loc_hp,LOST_TOOTH_16);
	else if (!IS_SET(victim->loc_hp,LOST_TOOTH_8))
	    SET_BIT(victim->loc_hp,LOST_TOOTH_8);
	else if (!IS_SET(victim->loc_hp,LOST_TOOTH_4))
	    SET_BIT(victim->loc_hp,LOST_TOOTH_4);
	else if (!IS_SET(victim->loc_hp,LOST_TOOTH_2))
	    SET_BIT(victim->loc_hp,LOST_TOOTH_2);
	else if (!IS_SET(victim->loc_hp,LOST_TOOTH_1))
	    SET_BIT(victim->loc_hp,LOST_TOOTH_1);
	act("You kick $N in the face, knocking out one of $S teeth!",ch,NULL,victim,TO_CHAR);
	act("$n kicks $N in the face, knocking out one of $S teeth!",ch,NULL,victim,TO_NOTVICT);
	act("$n kicks you in the face, knocking out one of your teeth!",ch,NULL,victim,TO_VICT);
      make_part(victim,"tooth");
      }
            /* Lets give bonuses to people wearing boots */
      if (( obj = get_eq_char(ch,WEAR_FEET)) != NULL )
      {
	  if (is_plate(obj))
	    damage(ch,victim,number_range( 80,ch->level + 200), gsn_kick,DAM_BASH,TRUE);
        else
        if (is_chain(obj))
	    damage(ch,victim,number_range( 60,ch->level + 150), gsn_kick,DAM_BASH,TRUE);
        else
        if (is_leather(obj))
	    damage(ch,victim,number_range( 40,ch->level + 100), gsn_kick,DAM_BASH,TRUE);
        else
	    damage(ch,victim,number_range( 20,ch->level + 50), gsn_kick,DAM_BASH,TRUE);
	}
	else
      {
	 damage(ch,victim,number_range( 1, ch->level ), gsn_kick,DAM_BASH,TRUE);
      }
	check_improve(ch,gsn_kick,TRUE,1);
    }
    else
    {
	damage( ch, victim, 0, gsn_kick,DAM_BASH,TRUE);
	check_improve(ch,gsn_kick,FALSE,1);
    }
    check_killer(ch,victim);
    return;
}




void do_disarm( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    int chance,hth,ch_weapon,vict_weapon,ch_vict_weapon;

    hth = 0;

    if ((chance = get_skill(ch,gsn_disarm)) == 0)
    {
	send_to_char( "You don't know how to disarm opponents.\n\r", ch );
	return;
    }

    if ( get_eq_char( ch, WEAR_WIELD ) == NULL 
    &&   ((hth = get_skill(ch,gsn_hand_to_hand)) == 0
    ||    (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_DISARM))))
    {
	send_to_char( "You must wield a weapon to disarm.\n\r", ch );
	return;
    }

    if ( ( victim = ch->fighting ) == NULL )
    {
	send_to_char( "You aren't fighting anyone.\n\r", ch );
	return;
    }

    if ( ( obj = get_eq_char( victim, WEAR_SECONDARY ) ) == NULL )
    if ( ( obj = get_eq_char( victim, WEAR_WIELD ) ) == NULL )
    {
	send_to_char( "Your opponent is not wielding a weapon.\n\r", ch );
	return;
    }

    if ( (obj != NULL) && IS_OBJ_STAT(obj,ITEM_DEATH) )
    {
	act( "Your opponent's weapon does not seem to want to budge from $S hand!",  ch, NULL, victim, TO_CHAR    );
	return;
    }

    /* find weapon skills */
    ch_weapon = get_weapon_skill(ch,get_weapon_sn(ch));
    vict_weapon = get_weapon_skill(victim,get_weapon_sn(victim));
    ch_vict_weapon = get_weapon_skill(ch,get_weapon_sn(victim));

    /* modifiers */

    /* skill */
    if ( get_eq_char(ch,WEAR_WIELD) == NULL)
	chance = chance * hth/150;
    else
	chance = chance * ch_weapon/100;

    chance += (ch_vict_weapon/2 - vict_weapon) / 2; 

    /* dex vs. strength */
    chance += get_curr_stat(ch,STAT_DEX);
    chance -= 2 * get_curr_stat(victim,STAT_STR);

    /* level */
    chance += (ch->level - victim->level) * 2;
 
    /* and now the attack */
    if (number_percent() < chance)
    {
    	WAIT_STATE( ch, skill_table[gsn_disarm].beats );
	disarm( ch, victim );
	check_improve(ch,gsn_disarm,TRUE,1);
    }
    else
    {
	WAIT_STATE(ch,skill_table[gsn_disarm].beats);
	if (((is_affected(victim, gsn_doppelganger)))
      && (!IS_SET(ch->act, PLR_HOLYLIGHT)))
	{
	  sprintf(buf, "You fail to disarm %s.", victim->doppel->name);
        send_to_char(buf, ch);
	  sprintf(buf, "%s tries to disarm %s, but fails.",ch->name, victim->doppel->name);
        act(buf, ch, NULL, victim, TO_NOTVICT);
      }
	else
	if (((is_affected(victim, gsn_treeform)))
      && (!IS_SET(ch->act, PLR_HOLYLIGHT)))
	{
	  sprintf(buf, "You fail to disarm the mighty oak tree.");
        send_to_char(buf, ch);
	  sprintf(buf, "%s tries to disarm the mighty oak tree, but fails.",ch->name);
        act(buf, ch, NULL, victim, TO_NOTVICT);
      }
	else
	{
        act("You fail to disarm $N.",ch,NULL,victim,TO_CHAR);
	  act("$n tries to disarm $N, but fails.",ch,NULL,victim,TO_NOTVICT);
	}
      act("$n tries to disarm you, but fails.",ch,NULL,victim,TO_VICT);
      check_improve(ch,gsn_disarm,FALSE,1);
    }
    check_killer(ch,victim);
    return;
}

void do_sla( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to SLAY, spell it out.\n\r", ch );
    return;
}



void do_slay( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MAX_INPUT_LENGTH];

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Slay whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, NULL, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( ch == victim )
    {
	send_to_char( "Yes, that's what we need.... more suicide!\n\r", ch );
	return;
    }

    if ( !IS_NPC(victim) && victim->level >= get_trust(ch) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    act( "You slay $M in cold blood!",  ch, NULL, victim, TO_CHAR    );
    act( "$n slays you in cold blood!", ch, NULL, victim, TO_VICT    );
    act( "$n slays $N in cold blood!",  ch, NULL, victim, TO_NOTVICT );
    raw_kill( ch, victim );
    check_cpose(victim);
    return;
}

/* ATTENTION CODERS, Vlad's Change */

void do_break_weapon(CHAR_DATA *ch,char *argument) {
  CHAR_DATA *victim;
  int chance;
  OBJ_DATA *weapon;
  
  chance = get_skill(ch,gsn_break_weapon);
  if (chance == 0) {
    send_to_char("Huh?\n\r",ch);
    return;
  }
  
  victim = ch->fighting;
  
  if (victim == NULL) {
    send_to_char("But you aren't fighting anyone.\n\r",ch);
    return;
  }

  weapon = get_eq_char(ch,WEAR_WIELD);
  if (weapon == NULL) {
    send_to_char("You must be wielding an axe to break your enemy's weapon.\n\r", ch);
    return;
  }
  if (weapon->value[0] != WEAPON_AXE) {
    send_to_char("You must be wielding an axe to break your enemy's weapon.\n\r", ch);
    return;
  }
  
  weapon = get_eq_char(victim,WEAR_WIELD);
  if (weapon == NULL) {
    send_to_char("They aren't wielding a weapon.\n\r",ch);
    return;
  }
  chance /= 2;
  
  chance -= weapon->level;
  chance -= get_curr_stat(victim,STAT_DEX);
  chance += get_curr_stat(ch,STAT_STR);
  chance -= victim->level;
  chance += ch->level;
  chance = URANGE(5,chance,50);
  
    
  if (number_percent() > chance) {
    act("You try to smash $N's weapon to pieces, but fail.",ch,0,victim,TO_CHAR);
    act("$n tries to smash your weapon to pieces, but fails.",ch,0,victim,TO_VICT);
    act("$n tries to smash $N's weapon to pieces, but fails.",ch,0,victim,TO_NOTVICT);
    check_improve(ch,gsn_break_weapon,FALSE,1);
    WAIT_STATE(ch,12);
    return;
  }
  act("With a sharp crack $n's $p splinters and breaks!",victim,weapon,0,TO_ROOM);
  act("$n strikes your weapon and with a sharp crack it breaks!",ch,0,victim,TO_VICT);
  extract_obj(weapon);
  check_improve(ch,gsn_break_weapon,TRUE,2);
  WAIT_STATE(ch,12);
  return;
}

/*
 * Check for tumble.
 */
bool check_tumble( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance, skill;

    if (IS_NPC(victim))
	return FALSE;

    if (victim->level < skill_table[gsn_tumble].skill_level[victim->class])
       return FALSE;

    if ( !IS_AWAKE(victim) )
        return FALSE;

    skill = victim->pcdata->learned[gsn_tumble];
    if( skill <= 1 )
        return FALSE;

    if (victim->riding)
     return FALSE;

    chance  = victim->pcdata->learned[gsn_tumble] / 2;
    chance += get_curr_stat(victim, STAT_DEX) / 3;
    chance -= ch->hitroll /9 + (ch->level <=5) + (ch->level <= 10);  
    if ( number_percent( ) >= chance + victim->level - ch->level )
        return FALSE;
    act( "You tumble away from $n's blow.", ch, NULL, victim, TO_VICT    );
    act( "$N tumbles away from your blow.", ch, NULL, victim, TO_CHAR    );
    check_improve(victim,gsn_tumble,TRUE,6);
    return TRUE;
}


bool check_counter( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt)
{
        int chance;
        int dam_type;
        OBJ_DATA *wield;

        if (    ( get_eq_char(victim, WEAR_WIELD) == NULL ) ||
                ( !IS_AWAKE(victim) ) ||
                ( !can_see(victim,ch) ) ||
                ( get_skill(victim,gsn_counter) < 1 )
           )
           return FALSE;

        wield = get_eq_char(victim,WEAR_WIELD);

        chance = get_skill(victim,gsn_counter) / 6;
        chance += ( victim->level - ch->level ) / 4;
        chance += 2 * (get_curr_stat(victim,STAT_DEX) - get_curr_stat(ch,STAT_DEX));
        chance += get_weapon_skill(victim,get_weapon_sn(victim)) -
                        get_weapon_skill(ch,get_weapon_sn(ch));
        chance += (get_curr_stat(victim,STAT_STR) - get_curr_stat(ch,STAT_STR) );

    if ( number_percent( ) >= chance )
        return FALSE;

        dt = gsn_counter;

    if ( dt == TYPE_UNDEFINED )
    {
	dt = TYPE_HIT;
	if ( wield != NULL && wield->item_type == ITEM_WEAPON )
	    dt += wield->value[3];
	else 
            dt += ch->dam_type;
    }

    if (dt < TYPE_HIT)
    	if (wield != NULL)
    	    dam_type = attack_table[wield->value[3]].damage;
    	else
    	    dam_type = attack_table[ch->dam_type].damage;
    else
    	dam_type = attack_table[dt - TYPE_HIT].damage;

    if (dam_type == -1)
	dam_type = DAM_BASH;

    act( "You reverse $n's attack and counter with your own!", ch, NULL, victim, TO_VICT    );
    act( "$N reverses your attack!", ch, NULL, victim, TO_CHAR    );

    damage(victim,ch,dam/2, gsn_counter , dam_type ,TRUE ); /* DAM MSG NUMBER!! */

    check_improve(victim,gsn_counter,TRUE,6);

    return TRUE;
}

bool check_critical(CHAR_DATA *ch, CHAR_DATA *victim)
{
   OBJ_DATA *obj;
   int damp;

   obj = get_eq_char(ch,WEAR_WIELD);

   if (( get_eq_char(ch,WEAR_WIELD) == NULL )
   || ( get_skill(ch,gsn_critical)  <  1 )
   || ( get_weapon_skill(ch,get_weapon_sn(ch))  !=  100 )
   || ( number_range(0,100) > get_skill(ch,gsn_critical)))
       return FALSE;

    if ( number_range(0,100) > 25 )
       return FALSE;

    /* Now, if it passed all the tests... */
    damp=number_range(1,5);
    if ( damp == 1 )
    {
	if (!IS_SET(victim->loc_hp,LOST_EYE_L))
	    SET_BIT(victim->loc_hp,LOST_EYE_L);
	else if (!IS_SET(victim->loc_hp,LOST_EYE_R))
	    SET_BIT(victim->loc_hp,LOST_EYE_R);
      else return FALSE;
	act("Your skillful blow takes out $N's eye!",ch,NULL,victim,TO_CHAR);
	act("$n's skillful blow takes out $N's eye!",ch,NULL,victim,TO_NOTVICT);
	act("$n's skillful blow takes out your eye!",ch,NULL,victim,TO_VICT);
	make_part(victim,"eyeball");
    }
    else if ( damp == 2 )
    {
	if (!IS_SET(victim->loc_hp,LOST_EAR_L))
	    SET_BIT(victim->loc_hp,LOST_EAR_L);
	else if (!IS_SET(victim->loc_hp,LOST_EAR_R))
	    SET_BIT(victim->loc_hp,LOST_EAR_R);
      else return FALSE;
	act("Your skillful blow cuts off $N's ear!",ch,NULL,victim,TO_CHAR);
	act("$n's skillful blow cuts off $N's ear!",ch,NULL,victim,TO_NOTVICT);
	act("$n's skillful blow cuts off your ear!",ch,NULL,victim,TO_VICT);
	make_part(victim,"ear");
    }
    else if ( damp == 3 )
    {
	if (!IS_SET(victim->loc_hp,LOST_NOSE))
	    SET_BIT(victim->loc_hp,LOST_NOSE);
      else return FALSE;
	act("Your skillful blow cuts off $N's nose!",ch,NULL,victim,TO_CHAR);
	act("$n's skillful blow cuts off $N's nose!",ch,NULL,victim,TO_NOTVICT);
	act("$n's skillful blow cuts off your nose!",ch,NULL,victim,TO_VICT);
	make_part(victim,"nose");
    }
    else if ( damp == 4 )
    {
	if (!IS_SET(victim->loc_hp,BROKEN_JAW))
	    SET_BIT(victim->loc_hp,BROKEN_JAW);
      else return FALSE;
	act("Your skillful blow breaks $N's jaw!",ch,NULL,victim,TO_CHAR);
	act("$n's skillful blow breaks $N's jaw!",ch,NULL,victim,TO_NOTVICT);
	act("$n's skillful blow breaks your jaw!",ch,NULL,victim,TO_VICT);
    }
    else if ( damp == 5 )
    {
	if (!IS_SET(victim->loc_hp,LOST_NOSE) && !IS_SET(victim->loc_hp,BROKEN_NOSE))
	    SET_BIT(victim->loc_hp,BROKEN_NOSE);
	else return FALSE;
	act("Your skillful blow breaks $N's nose!",ch,NULL,victim,TO_CHAR);
	act("$n's skillful blow breaks $N's nose!",ch,NULL,victim,TO_NOTVICT);
	act("$n's skillful blow breaks your nose!",ch,NULL,victim,TO_VICT);
    }

  check_improve(ch,gsn_critical,TRUE,6);
  return TRUE;
}

void do_deathgrip( CHAR_DATA *ch, char *argument )
{
        int sn;
        OBJ_DATA *obj;

        sn = skill_lookup("deathgrip");

        if ( get_skill(ch,sn) < 1 )
        {
                send_to_char("What's that?\n\r",ch);
                return;
        }

        if (  get_skill(ch,sn)  <  (number_range(0, 100))  )
        {
                send_to_char("You failed to create a grip of death.\n",ch);
                check_improve(ch,sn,FALSE,1);
                return;
        }

        obj = get_eq_char(ch,WEAR_WIELD);

        if ( (obj != NULL) && !IS_OBJ_STAT(obj,ITEM_DEATH) )
        {
                SET_BIT( obj->extra_flags, ITEM_DEATH);
                act("$p flickers with dark power.",ch,obj,NULL,TO_ALL);
        }
}

void do_whirlwind( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *pChar;
  CHAR_DATA *pChar_next;
  OBJ_DATA *wield;
  int chance, dam_type;
  AFFECT_DATA af;

    if ( ch->riding )
    {
        send_to_char( "You can't whirl while mounted.\n\r", ch );
        return;
    }

    if ( IS_AFFECTED2(ch, AFF_PARALYSIS) 
    ||   IS_AFFECTED3(ch, AFF_HOLDPERSON))
    {
      send_to_char("You can't move at all, there's no way you can do that.\n\r",ch);
      return;
    }

  if (ch->fighting != NULL)
    {
     send_to_char("You cannot do that while in combat.\n\r",ch);
     return;
    }

  do_visible(ch,"");

  if (IS_NPC(ch))
  return;
   
  if ((ch->level < skill_table[gsn_whirlwind].skill_level[ch->class] )
  || (chance = get_skill(ch,gsn_whirlwind) == 1 ) ) 
   {
     act("$n loses $s balance and falls in a heap.",  ch,NULL,NULL,TO_ROOM);
     act("You lose your balance and fall in a heap.", ch,NULL,NULL,TO_CHAR);
     ch->position = POS_RESTING;
     WAIT_STATE(ch, skill_table[gsn_whirlwind].beats*3);
    return;
   }
 
   if ( ( wield = get_eq_char( ch, WEAR_WIELD ) ) == NULL )
   {
      send_to_char( "You need to wield a weapon first...\n\r", ch );
      return;
   }
   
   act( "$n holds $p firmly, and starts spinning round...", ch, wield, NULL, TO_ROOM );
   act( "You hold $p firmly, and start spinning round...",  ch, wield, NULL, TO_CHAR );
   
  if ((number_percent( )+number_fuzzy(10)) < ch->pcdata->learned[gsn_whirlwind])
  {
     WAIT_STATE(ch, skill_table[gsn_whirlwind].beats );
     dam_type = attack_table[wield->value[3]].damage;
     check_improve(ch,gsn_whirlwind,TRUE,1);
     pChar_next = NULL;
       for ( pChar = ch->in_room->people; pChar; pChar = pChar_next)
       {
        pChar_next = pChar->next_in_room;
         if (!IS_IMMORTAL(pChar) && (pChar != ch) && (!is_safe(ch,pChar))
         && (!is_same_group( pChar,ch)))
         {
           if (pChar->position !=POS_FIGHTING)
		{
            if (!can_see(pChar,ch))
                do_yell(pChar, "Help! Someone's slicing me open!");
            else
              {
            sprintf( buf, "Help! %s's slicing me open!", ch->name);
            do_yell( pChar, buf );
              }
		}

           act("$n turns, blades whirling, towards YOU!",ch,NULL,pChar,TO_VICT);
           damage(ch, pChar, number_range(1, ch->level ), gsn_whirlwind,dam_type,TRUE); 
        }
       }
     return;
   }

  WAIT_STATE(ch, skill_table[gsn_whirlwind].beats*2);
  act("$n looks dizzy.\n\r",ch,NULL,NULL,TO_ROOM);
  send_to_char("You feel dizzy.\n\r",ch);
  check_improve(ch,gsn_whirlwind,FALSE,1);

  af.where		= TO_AFFECTS;
  af.type         = gsn_whirlwind;
  af.level        = ch->level;
  af.duration     = 1;
  af.location     = APPLY_HITROLL;
  af.modifier     = -1;
  af.bitvector    = 0;
  affect_to_char(ch,&af);
  af.location     = APPLY_DEX;
  affect_to_char(ch,&af);
  return;
}


void do_lunge( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    int dam;

    if ( ch->riding )
    {
        send_to_char( "You can't get close enough while mounted.\n\r", ch );
        return;
    }

    one_argument( argument, arg );

    if (arg[0] == '\0')
    {
        send_to_char("Impale whom?\n\r",ch);
        return;
    }

    if (ch->fighting != NULL)
    {
	send_to_char("You're facing the wrong end.\n\r",ch);
	return;
    }
 
    else if ((victim = get_char_room(ch, NULL, arg)) == NULL)
    {
        send_to_char("They aren't here.\n\r",ch);
        return;
    }

    if ( victim == ch )
    {
	send_to_char( "How can you impale yourself?\n\r", ch );
	return;
    }

   if ( get_skill(ch,gsn_lunge) == 0 
   ||     (!IS_NPC(ch) 
   &&      ch->level < skill_table[gsn_lunge].skill_level[ch->class])) 
   { 
    send_to_char("Lunge? What's that?\n\r",ch); 
    return; 
   } 

    if ( IS_AFFECTED2(ch, AFF_PARALYSIS) 
    ||   IS_AFFECTED3(ch, AFF_HOLDPERSON))
    {
      send_to_char("You can't move at all, there's no way you can do that.\n\r",ch);
      return;
    }

    if ( is_safe( ch, victim ) )
      return;

    if (IS_NPC(victim) &&
	 victim->fighting != NULL && 
	!is_same_group(ch,victim->fighting))
    {
        send_to_char("Kill stealing is not permitted.\n\r",ch);
        return;
    }

   if ( !( obj = get_eq_char( ch, WEAR_WIELD ) ) 
    ||(obj->value[0] != 1 
    && obj->value[0] != WEAPON_SPEAR 
    && obj->value[0] != WEAPON_POLEARM
    && obj->value[0] != WEAPON_SWORD 
    && obj->value[0] != WEAPON_AXE )) 
   { 
       send_to_char( "Lunge with that?! Right..Find yourself an impaling weapon.\n\r", ch ); 
       return; 
   } 

    if ( victim->hit < victim->max_hit / 3)
    {
	act( "$N is hurt and suspicious ... you can't impale them.",
	    ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( ch->fight_pos == FIGHT_REAR )
    {
       send_to_char( "You aren't close enough to lunge!\n\r",ch);
       return;
    }

   WAIT_STATE( ch, skill_table[gsn_lunge].beats ); 
   if ( number_percent( ) < get_skill(ch,gsn_lunge) 
   || ( get_skill(ch,gsn_lunge) >= 2 && !IS_AWAKE(victim) ) ) 
   {
    	if ( ch->fight_pos == FIGHT_FRONT )
        {
        check_improve(ch,gsn_lunge,TRUE,1); 
        act( "$n impales you with a lunge.", ch, NULL, victim, TO_VICT); 
        act( "You impale $N.", ch, NULL, victim, TO_CHAR); 
        act( "$n impales $N.", ch, NULL, victim, TO_NOTVICT); 
        dam = dice(200 + 20, 15);
        damage( ch, victim, dam, gsn_lunge,DAM_PIERCE,TRUE);
        }
    	if ( ch->fight_pos == FIGHT_MIDDLE )
        {
        act( "$n lunges forward and skillfully impales you.", ch, NULL, victim, TO_VICT); 
        act( "You lunge forward and impale $N.", ch, NULL, victim, TO_CHAR); 
        act( "$n lunges forward and impales $N.", ch, NULL, victim, TO_NOTVICT);
        dam = dice(100 + 20, 15);
        damage( ch, victim, dam, gsn_lunge,DAM_PIERCE,TRUE);
	  }
   } 
   else 
   { 
    check_improve(ch,gsn_lunge,FALSE,1); 
    act( "$n tries to impale you with a skillful lunge.", ch, NULL, victim, 
TO_VICT); 
    act( "You try to impale $N with a skillful lunge.", ch, NULL, victim, TO_CHAR); 
    act( "$n tries to impale $N with a skillful lunge.", ch, NULL, victim, TO_NOTVICT); 
    damage( ch, victim, 0, gsn_lunge,DAM_PIERCE,TRUE); 
   } 

   return; 
}

void do_blackjack( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    int chance;

    if ( ch->riding )
    {
        send_to_char( "You can't get close enough while mounted.\n\r", ch );
        return;
    }

    one_argument(argument,arg);

    if ( arg[0] == '\0' )
    {
        send_to_char( "Blackjack whom?\n\r", ch );
        return;
    }

    if ( (chance = get_skill(ch,gsn_blackjack)) == 0)
    {
	send_to_char("You cant blackjack.\n\r",ch);
	return;
    }

    if ((victim = get_char_room(ch, NULL, arg)) == NULL)
    {
	send_to_char("They aren't here.\n\r",ch);
	return;
    }

    if (IS_AFFECTED2(victim,AFF_BLACKJACK))
    {
	act("$E's already been knocked out cold.",ch,NULL,victim,TO_CHAR);
	return;
    }

    if (victim == ch)
    {
	send_to_char("You can't blackjack yourself.\n\r",ch);
	return;
    }

    if ( IS_AFFECTED2(ch, AFF_PARALYSIS) 
    ||   IS_AFFECTED3(ch, AFF_HOLDPERSON))
    {
      send_to_char("You can't move at all, there's no way you can do that.\n\r",ch);
      return;
    }

    if (IS_AFFECTED(victim,AFF_SLEEP))
    {
        act("$e's already asleep.  Trying to wake them up?.", ch,NULL,victim,TO_CHAR);
        return;
    }

    if (is_safe(ch,victim))
	return;

    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
    {
	act("But $N is such a good friend!",ch,NULL,victim,TO_CHAR);
	return;
    }

  if (IS_AFFECTED3(victim, AFF_WATCHFUL)
  &&  IS_AWAKE(victim))
  {
    act( "$N is extremely alert to any movement around $M and catches you before\n\r"
	   "you have a chance to whack $M in the back of the head!", ch, NULL, victim, TO_CHAR );
    act( "Your supreme alertness spins you around in time to catch $n trying\n\r"
	   "to whack you in the back of the head!  In the process however, your\n\r"
	   "alertness wears off and you find yourself fighting $n.", ch, NULL, victim, TO_VICT );
    if (!ch->fighting)
      set_fighting(ch, victim);
    affect_strip(victim,skill_lookup("blessed watchfulness"));
    REMOVE_BIT( victim->affected3_by, AFF_WATCHFUL );
    return;
  }

    if (victim->riding)
    {
	act("$N is mounted and $S back is covered.\n\r",ch, NULL, victim,TO_CHAR);
	return;
    }


    /* level */
    chance += (ch->level - victim->level) * 2;

    /* sloppy hack to prevent false zeroes */
    if (chance % 5 == 0)
	chance += 1;

    if (chance == 0)
    {
	send_to_char("You failed horrbly!.\n\r",ch);
	return;
    }

    if (number_percent( ) < (ch->pcdata->learned[gsn_blackjack]) 
    && (victim->level <= (ch->level+3)))
    {
	AFFECT_DATA af;
	act("$n is knocked out cold!",victim,NULL,NULL,TO_ROOM);
	send_to_char("WHAM!! Your whacked upside the head and knocked out cold!\n\r",victim);
	check_improve(ch,gsn_blackjack,TRUE,2);

      victim->position 	= POS_SLEEPING;
	af.where		= TO_AFFECTS2;
	af.type 		= gsn_blackjack;
	af.level 		= ch->level;
	af.duration		= number_fuzzy(ch->level / 8);
	af.location		= APPLY_NONE;
	af.modifier		= 0;
	af.bitvector 	= AFF_BLACKJACK;
	affect_to_char(victim,&af);
      check_cpose(ch);
      check_cpose(victim);

      if ( !IS_NPC(victim) )
         {
             if ( !IS_IMMORTAL(ch) )
             {
                ch->quit_timer = -5;
                ch->lastfought = victim;
                if (!IS_NPC(victim))
                {
                   free_string(ch->last_pc_fought);
                   ch->last_pc_fought = str_dup(victim->name);
                }
             }
             victim->quit_timer = -5;
             victim->lastfought = ch;
             if (!IS_NPC(victim))
             {
                free_string(ch->last_pc_fought);
                ch->last_pc_fought = str_dup(victim->name);
             }
         }

    }
    else
    {
      act("$n tries to whack you over the head!",ch,NULL,victim,TO_VICT);
      send_to_char("Your blackjack attempt misses.\n\r",ch);
      act("$n tries to whack $N over the head, but misses.",ch,NULL,victim,TO_NOTVICT);
      if(!IS_NPC(ch))
      {     
        if (!can_see(victim,ch))
          do_yell(victim, "Help! Someone tried to blackjack me!");
        else
        {
          if (IS_NPC(ch))
            sprintf(buf, "Help! %s just tried to blackjack me!", ch->short_descr);
          else
            sprintf( buf, "Help! %s just tried to blackjack me!", 
            ((is_affected(ch,gsn_doppelganger)) && (!IS_IMMORTAL(victim)))? ch->doppel->name :
            ((is_affected(ch,gsn_treeform)) && (!IS_IMMORTAL(victim)))? "A mighty oak tree" :
            ch->name);
            do_yell( victim, buf );
        }
      }
	multi_hit( ch, victim, TYPE_UNDEFINED );
	check_improve(ch,gsn_blackjack,FALSE,2);
	return;
    }
    WAIT_STATE( ch, 2 * PULSE_VIOLENCE );
}

void do_ravage_bite( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    int dam;

    if ( ch->riding )
    {
        send_to_char( "You can't get close enough while mounted.\n\r", ch );
        return;
    }

    if ( !IS_NPC(ch)
    &&   ch->level < skill_table[gsn_ravage_bite].skill_level[ch->class] )
    {
	send_to_char(    "You had better leave the biting to animals.\n\r", ch );
	return;
    }

    if ( ( victim = ch->fighting ) == NULL )
    {
	send_to_char( "You aren't fighting anyone.\n\r", ch );
	return;
    }

    else

	act("$n leaps forth, ravagely biting you!",ch,NULL,victim,TO_VICT);
	act("You leap onto $N, and ravagely bite $M!",ch,NULL,victim,TO_CHAR);
	act("$n leaps onto $N and starts biting him like a wild beast.",ch,NULL,victim,TO_NOTVICT);
    dam = number_range( 1, ch->level+1000 );
    WAIT_STATE( ch, skill_table[gsn_ravage_bite].beats );

    if ( get_skill(ch,gsn_ravage_bite) > number_percent())
    {
	damage(ch,victim,number_range( dam, (ch->level*2.7) ), gsn_ravage_bite,DAM_PIERCE,TRUE);
	check_improve(ch,gsn_ravage_bite,TRUE,1);
    }
    else
    {
	damage( ch, victim, 0, gsn_ravage_bite,DAM_PIERCE,TRUE);
	check_improve(ch,gsn_ravage_bite,FALSE,1);
    }
    return;
}

bool check_displacement( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance;

    if (IS_NPC(victim))
	return FALSE;

    if ( !IS_AWAKE(victim) )
	return FALSE;

    if ( !IS_NPC(victim) && victim->level < skill_table[gsn_displacement].skill_level[victim->class] )
	return FALSE;

    chance = get_skill(victim,gsn_displacement) / 2;

    if (!can_see(victim,ch))
	chance /= 2;

    if ( number_percent( ) >= chance + victim->level - ch->level )
        return FALSE;

    act( "Your body displaces itself to avoid $n's attack.", ch, NULL, victim, TO_VICT    );
    act( "$N's body displaces itself to avoid your attack.", ch, NULL, victim, TO_CHAR    );
    check_improve(victim,gsn_displacement,TRUE,6);
    return TRUE;
}

void do_nerve_pinch(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];

  one_argument(argument,arg);

  if (ch->level < skill_table[gsn_nerve_pinch].skill_level[ch->class] )
  {
    send_to_char( "Huh?\n\r", ch );
    return;
  }

  if (ch->fighting == NULL)
  {
    send_to_char("You aren't fighting anyone.\n\r",ch);
    return;
  }

  victim = ch->fighting;

  if (is_safe(ch,victim)) return;

  if (is_affected(victim,gsn_nerve_pinch))
  {
    send_to_char("That person's nerves are already in kinks.\n\r",ch);
    WAIT_STATE( ch, skill_table[gsn_nerve_pinch].beats );
    return;
  }

  check_killer(ch,victim);
  WAIT_STATE( ch, skill_table[gsn_nerve_pinch].beats );

  if (IS_NPC(ch)
  ||  number_percent() < (ch->pcdata->learned[gsn_nerve_pinch] + ch->level + get_curr_stat(ch,STAT_DEX))/2)
  {
    AFFECT_DATA af;

    af.type   = gsn_nerve_pinch;
    af.level  = ch->level;
    af.duration = ch->level * PULSE_VIOLENCE/PULSE_TICK;
    af.location = APPLY_STR;
    af.modifier = -3;
    af.bitvector = 0;
    affect_to_char(victim,&af);
    act("$N screams in agony with your nerve pinch.",ch,NULL,victim,TO_CHAR);
    act("$n causes you great discomfort with $s nerve pinch.",ch,NULL,victim,TO_VICT);
    act("$n makes $N scream in agony with $s nerve pinch.",ch,NULL,victim,TO_NOTVICT);
    check_improve(ch,gsn_nerve_pinch,TRUE,1);
  }
  else
  {
    send_to_char("You attack, but don't pinch the right spots and fail.\n\r",ch);
    act("$n tries to attack you with a nerve pinch, but fails.", ch,NULL,victim,TO_VICT);
    act("$n tries to attack $N with a nerve pinch, but fails.",ch,NULL,victim,TO_NOTVICT);
    check_improve(ch,gsn_nerve_pinch,FALSE,1);
  }

  multi_hit(victim,ch,TYPE_UNDEFINED);
  return;
}

void do_caltrops(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim = ch->fighting;
  AFFECT_DATA af; 

   if (    !IS_NPC( ch ) 
        && ch->level < skill_table[gsn_caltrops].skill_level[ch->class] )
    {
      send_to_char("Huh?\n\r",ch);
      return;
    }

  if (victim == NULL)
    {
      send_to_char("You must be in combat.\n\r",ch);
      return;
    }

  if (is_safe(ch,victim))
    return;

  if (IS_AFFECTED(victim, AFF_FLYING))
    {
    send_to_char("They are flying.\r\n",ch);
    return;
    }

  act("You throw a handful of sharp spikes at the feet of $N.",
      ch,NULL,victim,TO_CHAR);
  act("$n throws a handful of sharp spikes at your feet!",
      ch,NULL,victim,TO_VICT);

  WAIT_STATE(ch,skill_table[gsn_caltrops].beats);

  if (!IS_NPC(ch) && number_percent() >= ch->pcdata->learned[gsn_caltrops])
    {
      damage(ch,victim,0,gsn_caltrops,DAM_PIERCE,0);
      check_improve(ch,gsn_caltrops,FALSE,1);
      return;
    }

  damage(ch,victim,ch->level / 3,gsn_caltrops,DAM_PIERCE,0);

  if (!is_affected(victim,gsn_caltrops))
  {


    af.type         = gsn_caltrops;
    af.level        = ch->level;
    af.duration     = 2;
    af.location     = APPLY_HITROLL;
    af.modifier     = -5;
    af.bitvector    = 0;
    affect_to_char(victim,&af);

    af.location     = APPLY_DEX;
    affect_to_char(victim,&af);

    af.location     = APPLY_DAMROLL;
    affect_to_char(victim,&af);

    act("$N starts limping.",ch,NULL,victim,TO_CHAR);
    act("You start to limp.",ch,NULL,victim,TO_VICT);
    check_improve(ch,gsn_caltrops,TRUE,1);
  }
}

void do_bear_call(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *gch;
  CHAR_DATA *bear;
  CHAR_DATA *bear2;
  AFFECT_DATA af;
  int i;
  
  if ( IS_NPC(ch) ||
	 ch->level < skill_table[gsn_bear_call].skill_level[ch->class] )
    {
      send_to_char("The bears won't listen to you.\n\r", ch);
      return;
    }

  if (ch->in_room->sector_type != SECT_FOREST &&
	ch->in_room->sector_type != SECT_HILLS &&
	ch->in_room->sector_type != SECT_POLAR &&
	ch->in_room->sector_type != SECT_FIELD &&
	ch->in_room->sector_type != SECT_MOUNTAIN)
    {
      send_to_char("There are no bears here to listen to you.\n\r", ch);
      return;
    }

  send_to_char("You call for bears to help you.\n\r",ch);
  act("$n shouts a bear call.",ch,NULL,NULL,TO_ROOM);

  if (is_affected(ch, gsn_bear_call))
    {
      send_to_char("You cannot summon the strength to handle more bears right now.\n\r", ch);
      return;
    }

  if (ch->move < skill_table[gsn_bear_call].min_mana)
    {
      send_to_char("You can't concentrate enough.\n\r", ch);
      return;
    }

  for (gch = char_list; gch != NULL; gch = gch->next)
    {
      if (IS_NPC(gch) && IS_AFFECTED(gch,AFF_CHARM) && gch->master == ch &&
	  gch->pIndexData->vnum == MOB_VNUM_BEAR)
	{
	  send_to_char("What's wrong with the bear you've got?",ch);
	  return;
	}
    }

  if (number_percent() > get_skill(ch, gsn_bear_call))
    {
      send_to_char("The bears fail to respond to your call.\n\r", ch);
      check_improve(ch, gsn_bear_call, FALSE, 2);
      ch->move -= skill_table[gsn_bear_call].min_mana / 2;
      return;
    }

  ch->move -= skill_table[gsn_bear_call].min_mana;

  bear = create_mobile( get_mob_index(MOB_VNUM_BEAR) );

  for (i=0;i < MAX_STATS; i++)
    {
      bear->perm_stat[i] = UMIN(25,2 * ch->perm_stat[i]);
    }

  bear->max_hit = IS_NPC(ch)? ch->max_hit : ch->pcdata->perm_hit / 2;
  bear->hit = bear->max_hit;
  bear->max_mana = IS_NPC(ch)? ch->max_mana : ch->pcdata->perm_mana / 4;
  bear->mana = bear->max_mana;
  bear->alignment = ch->alignment;
  bear->level = UMIN(70,ch->level);
  for (i=0; i < 3; i++)
   bear->armor[i] = interpolate(bear->level,100,-100);
  bear->armor[3] = interpolate(bear->level,100,0);
  bear->gold = 0;

  if (ch->in_room->sector_type == SECT_POLAR)
    {
    bear->name = str_dup("polar bear");
    bear->short_descr = str_dup("a polar bear");
    bear->long_descr = str_dup("A hulking polar bear is here.\n\r");
    bear->damroll += 10;
    bear->description = str_dup
("\n\rThis huge white polar bear has a bulky form and a ferocious temper.  It's\n\r"
"thick white and yellow coat shelters it from the cold and helps it blend\n\r"
"in with the stark whiteness of a blizzard.  Muscles are thick along the\n\r"
"bear's entire body.  Tiny brown eyes notice movement and scan for targets.\n\r"
"Another noticable feature are the large claws on its paws.\n\r"); 
    }
  else if (ch->level>70)
    {
    bear->name = str_dup("tough old grizzly bear");
    bear->short_descr = str_dup("a tough old grizzly bear");
    bear->long_descr = str_dup("A tough old grizzly bear is here.\n\r");
    bear->description = str_dup
("\n\rThis tough old grizzly has a bulky and unwieldy form, but is\n\r"
"nevertheless capable of rapid motion.  Muscles ripple along the bear's\n\r"
"legs.  Tiny brown eyes notice movement and scan for targets. Another\n\r"
"noticable feature are the large claws on its paws.\n\r"); 
    }
  else if (ch->level>50)
    {
    bear->name = str_dup("kodiak bear");
    bear->short_descr = str_dup("a kodiak bear");
    bear->long_descr = str_dup("A kodiak bear is here.\n\r");
    bear->description = str_dup
("\n\rThis kodiak bear has a bulky form, weighing in at close to 1500 stones.\n\r"
"Muscles ripple along the bear's legs.  Tiny brown eyes notice movement and\n\r"
"scan for targets. Another noticable feature are the large claws on its\n\r"
"paws.\n\r"); 
    }
  else if (ch->level>35)
    {
    bear->name = str_dup("brown bear");
    bear->short_descr = str_dup("a brown bear");
    bear->long_descr = str_dup("A brown bear is here.\n\r");
    bear->description = str_dup
("\n\rThis brown bear stands at roughly four and a half paces in length and\n\r"
"weighs in at close to 800 stones.  Muscles along the bear's body are\n\r"
"noticeable.  Tiny brown eyes notice movement and scan for targets. Streaks\n\r"
"of cinnamon highlight the shaggy brown fur of this large animal.  Another\n\r"
"noticable feature are the claws on its paws.\n\r"); 
    }
  else
    {
    bear->name = str_dup("baby black bear");
    bear->short_descr = str_dup("a baby black bear");
    bear->long_descr = str_dup("A baby black bear cub is here.\n\r");
    bear->description = str_dup
("\n\rThis bear cub stands at roughly a half pace in length and weighs in at\n\r"
"close to 100 stones.  Baby fat along with some developing muscle along the\n\r"
"bear's body are noticeable.  Tiny brown eyes notice movement and scan for\n\r"
"playthings.  A tiny black mohawk along its back and glistening black fur\n\r"
"show a well-cared for cub.  While it is a baby, a noticable feature are\n\r"
"the claws on its paws.\n\r"); 
    }

  bear2 = create_mobile(bear->pIndexData);
  clone_mobile(bear,bear2);
  
  SET_BIT(bear->affected_by, AFF_CHARM);
  SET_BIT(bear2->affected_by, AFF_CHARM);
  bear->master = bear2->master = ch;
  bear->leader = bear2->leader = ch;

  char_to_room(bear,ch->in_room);
  char_to_room(bear2,ch->in_room);
  send_to_char("Two bears come to your rescue!\n\r",ch);
  act("Two bears come to $n's rescue!",ch,NULL,NULL,TO_ROOM);

  af.type               = gsn_bear_call;
  af.level              = ch->level; 
  af.duration           = 24;
  af.bitvector          = 0;
  af.modifier           = 0;
  af.location           = APPLY_NONE;
  affect_to_char(ch, &af); 

  check_improve(ch, gsn_bear_call, TRUE, 1);
  return;
}

void do_ambush( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    int fight;
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Who are you ambushing?\n\r", ch );
        return;
    }

    if( !IS_NPC(ch) && ch->pcdata->learned[gsn_ambush] < 1 )
    {
        send_to_char( "You don't know how to ambush people.\n\r", ch );
        return;
    }


    if( !IS_NPC(ch) && !IS_AFFECTED2(ch, AFF_CAMOUFLAGE) )
    {
        send_to_char( "You are not camouflaged from sight.\n\r", ch );
        return;
    }
    
    if ( ( victim = get_char_room( ch, NULL, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( victim == ch )
    {
        send_to_char( "Just how do you ambush yourself?\n\r", ch );
        return;
    }

    if ( IS_AFFECTED2(ch, AFF_PARALYSIS) 
    ||   IS_AFFECTED3(ch, AFF_HOLDPERSON))
    {
      send_to_char("You can't move at all, there's no way you can do that.\n\r",ch);
      return;
    }
       
    if ( victim->hit < victim->max_hit * 3 / 5 )    
    {
        act("$N is in fear for $S life, and is too aware to be ambushed.",
            ch, NULL, victim, TO_CHAR );
        return;
    }
  
    if ( is_safe( ch, victim ) )
      return;

    if (IS_AFFECTED(ch, AFF_SLOW))
    {
        send_to_char( "You are moving to slowly to ambush someone.\n\r", ch);
        return;
    }

    if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL)
    {
        send_to_char( "You need to wield a weapon to ambush someone.\n\r", ch );
        return;
    }

    if (ch->fight_pos > FIGHT_FRONT)
    {
        send_to_char("You are too far away to ambush!\r\n", ch);
        return;
    }

    if (ch->fighting != NULL)
    {
      send_to_char("Your in the middle of a fight, you couldn't possibly ambush anyone.\n\r",ch);
      return;
    }

    fight =0;
    if ( victim->position <= POS_SLEEPING )
        fight = 1;
    check_killer( ch, victim );
    WAIT_STATE( ch, skill_table[gsn_ambush].beats );
    if ( !IS_AWAKE(victim)
    ||   IS_NPC(ch)
    ||   number_percent( ) < ch->pcdata->learned[gsn_ambush] )
    {
        check_improve(ch,gsn_ambush,TRUE,1);
        one_hit( ch, victim, gsn_ambush,FALSE );
    if (!IS_NPC(victim) && fight != 1)
    {
      if (!can_see(victim,ch) ) 
        do_yell(victim, "Help! Someone just ambushed me!");
      else
      {
        if (IS_NPC(ch))
          sprintf(buf, "Help! I've been ambushed by %s!", ch->short_descr);
        else
          sprintf(buf, "Help! I've been ambushed by %s!", 
          ((is_affected(ch,gsn_doppelganger)) && (!IS_IMMORTAL(victim)))? ch->doppel->name :
          ((is_affected(ch,gsn_treeform)) && (!IS_IMMORTAL(victim)))? "A mighty oak tree" :
          ch->name);
          do_yell( victim, buf );
      }
    }
    }
    else
    {
    if (!IS_NPC(victim) && fight != 1)
    {
        if (!can_see(victim,ch) ) 
            do_yell(victim, "Help! Someone just tried to ambush me!");
        else
            {
            sprintf(buf, "Duck! %s just leaped over my head!", ch->name);
            do_yell( victim,buf );
        }
    }
        check_improve(ch,gsn_ambush,FALSE,1);
        damage( ch, victim, 0, gsn_ambush ,DAM_NONE,0 );
    }
    return;
}


/*
 * Set condition of a victim.
 */
void update_cond( CHAR_DATA *victim )
{
  if ( IS_NPC(victim))
  return;

  victim->pcdata->condition[COND_THIRST] = 65;
  victim->pcdata->condition[COND_FULL]   = 65;
  return;

}

void do_fire( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim = NULL;
  OBJ_DATA *quiver;
  OBJ_DATA *bow;
  int dam;
  int chance;

  quiver = get_eq_char(ch, WEAR_ABOUT);
  if (quiver == NULL)
  {
    send_to_char( "{WYou aren't wearing a quiver to get arrows from.{x\n\r", ch );
    return;
  }

  if ( quiver->item_type != ITEM_QUIVER )
  {
    send_to_char( "{WYou can only draw arrows from a quiver.{x\n\r", ch );
    return;
  }

  bow = get_eq_char(ch, WEAR_WIELD);
  if (bow == NULL)
  {
    send_to_char("{WWhat are you going to do, throw the arrow at them?{x\n\r", ch);
    return;
  }

  if (bow->value[0] != WEAPON_BOW)
  {
    send_to_char("{WYou might want to use a bow to fire that arrow with.{x\n\r", ch);
    return;
  }    

  one_argument( argument, arg );
  if ( arg[0] == '\0' && ch->fighting == NULL )
  {
    send_to_char( "{WFire an arrow at who?{x\n\r", ch );
    return;
  }

  if (!str_cmp(arg, "none")
  || !str_cmp(arg, "self")
  || victim == ch)
  {
    send_to_char("{WHow exactly did you plan on firing an arrow at yourself?{x\n\r", ch );
    return;
  }
	
  if ( arg[0] == '\0' )
  {
    if ( ch->fighting != NULL )
    {
      victim = ch->fighting;
    }
    else
    {
      send_to_char( "{WFire at whom or what?{x\n\r", ch );
      return;
    }
  }

  if ( ( victim = get_char_room( ch, NULL, arg ) ) == NULL )
  {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  if ( !IS_NPC( ch ) 
  && ch->level < skill_table[gsn_bow].skill_level[ch->class] )
  {
    send_to_char( "You don't know how to do that.\n\r", ch );
    return;
  }

  if (ch->fight_pos == FIGHT_REAR )
  {
    WAIT_STATE( ch, 2 * PULSE_VIOLENCE );

    if ( victim != NULL &&  quiver->value[0] > 0  )
    {
      act( "{W$n quickly notches an arrow and {yfires {Wit at $N{W.{x", ch, NULL, victim, TO_NOTVICT );
      act( "{WYou quickly notch an arrow and {yfire {Wit at $N.{x", ch, NULL, victim, TO_CHAR );
      act( "{W$n quickly notches an arrow and {yfires {Wit at you.{x",ch, NULL, victim, TO_VICT );
      quiver->value[0] = quiver->value[0] - 1;
      if ( quiver->value[0] <= 0 )
      {
        act( "{WYour $p {Wis now out of arrows, you need to find another one.{x", ch, quiver, NULL, TO_CHAR );
        extract_obj(quiver);
      }
    }

    if (number_percent() >= 20 + get_skill(ch,gsn_bow) * 4/5 )
    {
      act( "{WYou fire an arrow and it {rmisses{W, landing harmlessly on the ground.{x", ch, NULL, NULL, TO_CHAR);
      act( "{W$n fires an arrow and it {rmisses{W, landing harmlessly on the ground.{x", ch, NULL, NULL, TO_ROOM);
      check_improve(ch,gsn_bow,FALSE,2);
    }
    else
    {
      if (check_arrow_defl(ch, victim))
        return;
      dam = dice(5, 25);
      damage( ch, victim, dam, gsn_bow,DAM_PIERCE,TRUE);
      check_improve(ch,gsn_bow,TRUE,2);
    }
    return;
  }
  else
  if (ch->fight_pos == FIGHT_MIDDLE )
  {

    if (((chance = get_skill(ch,gsn_point_blank)) == 0)
    || (!IS_NPC(ch)
    && ch->level < skill_table[gsn_point_blank].skill_level[ch->class]))
    {	
      send_to_char("You aren't skilled enough to fire arrows at point blank range!\n\r",ch);
      send_to_char("You should consider moving to the REAR of your group to fire arrows!\n\r",ch);
      return;
    }

    WAIT_STATE( ch, 2 * PULSE_VIOLENCE );

    if ( victim != NULL &&  quiver->value[0] > 0  )
    {
      act( "{W$n quickly notches an arrow and {yfires {Wpoint-blank range at $N{W.{x", ch, NULL, victim, TO_NOTVICT );
      act( "{WYou quickly notch an arrow and {yfire {Wpoint-blank range at $N.{x", ch, NULL, victim, TO_CHAR );
      act( "{W$n quickly notches an arrow and {yfires {Wpoint-blank range at you.{x",ch, NULL, victim, TO_VICT );
      quiver->value[0] = quiver->value[0] - 1;
      if ( quiver->value[0] <= 0 )
      {
        act( "{WYour $p {Wis now out of arrows, you need to find another one.{x", ch, quiver, NULL, TO_CHAR );
        extract_obj(quiver);
      }
    }

    if ((number_percent() >= 20 + get_skill(ch,gsn_bow) * 4/5 )
    ||  (number_percent() >= 20 + get_skill(ch,gsn_point_blank) * 4/5 ))
    {
      act( "{WYou fire an arrow and it {rmisses{W, landing harmlessly on the ground.{x", ch, NULL, NULL, TO_CHAR);
      act( "{W$n fires an arrow and it {rmisses{W, landing harmlessly on the ground.{x", ch, NULL, NULL, TO_ROOM);
      check_improve(ch,gsn_point_blank,FALSE,2);
    }
    else
    {
      if (check_arrow_defl(ch, victim))
        return;
      dam = dice(10, 35);
      damage( ch, victim, dam, gsn_point_blank,DAM_PIERCE,TRUE);
      check_improve(ch,gsn_point_blank,TRUE,2);
    }
    return;
  }
  else
  if (ch->fight_pos == FIGHT_FRONT )
  {
    send_to_char("You can not fire arrows while so close to your enemy!\n\r",ch);
    send_to_char("Try moving further back to the middle or rear position.\n\r",ch);
    return;
  }
  else
    return;
}

void do_battlecry ( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *pch;
    AFFECT_DATA af;
    int chance;
   
    if ( (chance = get_skill(ch,gsn_battlecry)) == 0
    ||   (!IS_NPC(ch)
    &&    ch->level < skill_table[gsn_battlecry].skill_level[ch->class]))
    {
        send_to_char("Go home and practice screaming at the furniture.\n\r",ch);
        return;
    }

    if (ch->fighting == NULL)
    {
        send_to_char("You aren't fighting anyone.\n\r",ch);
        return;
    }

    if (ch->position < POS_FIGHTING)
    {
        send_to_char("You'll have to get up first.\n\r", ch);
        return;
    }
  
    if (number_percent() < chance)
    {
        send_to_char("You unleash a raging battlecry!\n\r", ch);
        act("$n unleashes a raging battle cry!",ch, NULL, NULL, TO_ROOM);

        for (pch = ch->next_in_room; pch && pch->next_in_room != NULL; pch = pch->next_in_room)
        {
            if (pch == ch)
                continue;

            chance = 100;
            if (pch->level > ch->level)
                chance -= (pch->level - ch->level) * 2;
            if (pch->hit < (pch->max_hit / 2))
                chance += 10;

            if (number_percent() < chance)
            {
                act("Suddenly $N seems very intimidating - you PANIC as your ears pop from the battlecry!", pch, NULL, ch, TO_CHAR);
                do_flee(pch,"");
            }
            else
            {
                act("Your ears pop at the sound of $N's battlecry.", pch, NULL,
                    ch, TO_CHAR);
            }

            if (!IS_AFFECTED2(pch, AFF_DEAFNESS))
            {

		af.where		= TO_AFFECTS2;
            af.type      	= skill_lookup("deafness");
    		af.level     	= ch->level;
		af.duration  	= 2;
    		af.location  	= 0;
    		af.modifier  	= 0;
    		af.bitvector 	= AFF_DEAFNESS;
    		affect_to_char( pch, &af );
    		}

        }

        
        check_improve(ch,gsn_battlecry,TRUE,1);
        WAIT_STATE(ch,skill_table[gsn_battlecry].beats);
        return;
    }
    
    send_to_char("You make a series of feeble grunts.\n\r",ch);
    act("$n makes some feeble grunting noises.", ch, NULL, NULL, TO_ROOM);

    check_improve(ch,gsn_battlecry,FALSE,1);
    WAIT_STATE(ch,skill_table[gsn_battlecry].beats);
    return;
}

void do_bind( CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    AFFECT_DATA af;
    char arg[MSL];

    /* Grab argument to get victim's name */
    one_argument(argument, arg);

    if(IS_NPC(ch))
    {
	send_to_char( "You don't know any first aid.\n\r",ch );
	return;
    }

    /* Check for ch's skill_level in this skill */
    if (ch->level < skill_table[gsn_bind].skill_level[ch->class])
    {
      send_to_char( "You don't know any first aid.\n\r",ch );
       return;
    }
  
    /* Check if no arguments were supplied */
    if (arg[0] == '\0')
    {
	send_to_char("Who's wounds do you wish to bind?\n\r",ch);
	return;
    }

    /* Get victim pointer from victim's name supplied in 'arg' */
    if ((victim = get_char_room(ch, NULL, arg)) == NULL)
    {
	send_to_char("They are not here.\n\r",ch);
	return;
    }

    /* Make sure victim is not an NPC */
    if (IS_NPC(victim))
    {
	send_to_char("First aid cannot help that one.\n\r",ch);
	return;
    }
    if (ch == victim)
    {
        if (((victim->hit/victim->max_hit)*100 <= 10)
        && victim->position <= POS_RESTING
        && !is_affected(victim, gsn_bind))
        {
	      if ( number_percent() <ch->pcdata->learned[gsn_bind])
              { check_improve(ch, gsn_bind, TRUE, 1);
	      af.where		= TO_AFFECTS;
              af.type       	= gsn_bind;
              af.level      	= ch->level;
              af.modifier   	= 0;
              af.location 	= APPLY_NONE;
              af.duration 	= 2;
              af.bitvector 	= 0;
            affect_to_char(victim,&af);
	  send_to_char("You bind your wounds successfully, easing your pain.\n\r", ch);
		}
		else
		send_to_char("You fail to bind your wounds.\n\r", ch);
	WAIT_STATE(ch,skill_table[gsn_bind].beats);
	}
	else
	send_to_char("You are unable to do so to yourself right now.\n\r", ch);
	return;
    }

    /* Make sure victim is not already dead and is really dying or 
       stunned or mortally wounded */
    if (victim->position == POS_DEAD
    || victim->position > POS_STUNNED)
    {
	if (((victim->hit/victim->max_hit)*100 <= 10)
	&& victim->position <= POS_RESTING
	&& !is_affected(victim, gsn_bind))
	{
	   if ( number_percent() < ch->pcdata->learned[gsn_bind])
	      { check_improve(ch, gsn_bind, TRUE, 1);
	      af.where		= TO_AFFECTS;
	      af.type       	= gsn_bind;
	      af.level      	= ch->level;
	      af.modifier   	= 0;
	      af.location 	= APPLY_NONE;
	      af.duration 	= 2;
	      af.bitvector 	= 0;
  	    affect_to_char(victim,&af);
	 act("You have stopped $N's bleeding, securely binding $S wounds.", ch, NULL, victim, TO_CHAR);
	 send_to_char("Your pain eases as your wounds are bound.\n\r", victim);
	    }
	   else
	    {
        /* Skill failed, check improve, send fail message */
        check_improve(ch, gsn_bind, FALSE, 1);
        act("$n fails to stop the bleeding.",ch,NULL,NULL,TO_ROOM);
        act("You fail to stop the bleeding.",ch,NULL,NULL,TO_CHAR);
	    } 
	WAIT_STATE(ch,skill_table[gsn_bind].beats); 
	}
	else
       send_to_char( "First aid won't help that one now.\n\r",ch );
       return;
    }

    /* Compare ch skill with a random percentage */
    if ( number_percent() < ch->pcdata->learned[gsn_bind])
       check_improve(ch, gsn_bind, TRUE, 1);
    else
    {
	/* Skill failed, check improve, send fail message */
        check_improve(ch, gsn_bind, FALSE, 1);
        act("$n fails to stop the bleeding.",ch,NULL,NULL,TO_ROOM);
        act("You fail to stop the bleeding.",ch,NULL,NULL,TO_CHAR);
        return;
    }

    /* Skill succeeded, heal victim, update position */
    victim->hit = 1;
    update_pos(victim);

    /* Send messages to the appropriate people */
    act("You have stopped $N's bleeding, securely binding $S wounds.\n"
	"$N will now live.",
	ch, NULL, victim, TO_CHAR);
    send_to_char("You are no longer on the verge of traveling to the Abyss.\n\r", victim ); 
    act("$n has stopped the bleeding, securely binding $N's wounds.", 
	ch, NULL, victim, TO_NOTVICT );

    /* Return victim to sleeping for healing purposes */
    victim->position = POS_SLEEPING;
      af.where		= TO_AFFECTS;
      af.type       	= gsn_bind;
      af.level      	= ch->level;
      af.modifier   	= 0;
      af.location 	= APPLY_NONE;
      af.duration 	= 2;
      af.bitvector 	= 0;
      affect_to_char(victim,&af);
    return;
}

void do_breakneck( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *wield;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    AFFECT_DATA af;  
    int chance;
 
    one_argument( argument, arg );
    wield = get_eq_char( ch, WEAR_WIELD );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Break who's neck?\n\r", ch );
        return;
    }

    if ( !IS_NPC(ch) && ch->pcdata->learned[gsn_breakneck] < 1 )
    {
        send_to_char("You don't know that skill.\n\r",ch);
        return;
    }

    if ( ch->riding )
    {
        send_to_char( "You can't get close enough while mounted.\n\r", ch );
        return;
    }

    if ( wield == NULL)
    {  
     send_to_char("You must be wielding a hammer or mace in order to break someone's neck.\n\r",ch);
     return;
    }
    if ( wield->value[0] != WEAPON_MACE )
    {
     send_to_char("You must be wielding a hammer or mace in order to break someone's neck.\n\r",ch);
     return;
    }

    if ( ( victim = get_char_room( ch, NULL, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( victim == ch )
    {
        send_to_char( "You can't break your own neck.\n\r", ch );
        return;
    }

    if ( ch->lastfought != NULL && victim->lastfought != NULL && !IS_NPC(victim) )
    {
        if ( victim == ch->lastfought || ch == victim->lastfought )
        {
            send_to_char("They are wary of your attack!\n\r", ch );
            return;
        }
    }

    if (IS_AFFECTED(victim,AFF_SLEEP))
    {
        act("$e's is already asleep.  There's no point.",
            ch,NULL,victim,TO_CHAR);
        return;
    }

    if ( IS_AFFECTED(ch, AFF_CHARM) )
    {
        send_to_char("Your master doesn't agree with your will!",ch);
        return;
    }

    if ( is_safe( ch, victim ) )
      return;
  

    if ( victim->fighting != NULL )
    {
        send_to_char( "You can't break a fighting person's neck.\n\r", ch );
        return;
    }

    if (ch->fight_pos > FIGHT_FRONT)
    {
	send_to_char("You are too far away to break their neck!\r\n", ch);
	return;
    }

    if ( IS_AFFECTED2(ch, AFF_PARALYSIS) 
    ||   IS_AFFECTED3(ch, AFF_HOLDPERSON))
    {
      send_to_char("You can't move at all, there's no way you can do that.\n\r",ch);
      return;
    }

    if (victim->riding)
    {
	act("$N is mounted and $S back is covered.\n\r",ch, NULL, victim,TO_CHAR);
	return;
    }
     /* modifiers */

    chance = get_skill(ch, gsn_breakneck);

    switch(check_immune(victim,1))
    {
        case(IS_IMMUNE):
            send_to_char("Your breakneck has no effect!\n\r",ch);
            return;
            break;
        case(IS_RESISTANT):     
            chance -= (victim->level) * 4;
            break;
    }

    /* size */
    if (ch->size < victim->size)
        chance += (ch->size - victim->size) * 5;

    /* str */
    chance += get_curr_stat(ch, STAT_STR);
    chance -= get_curr_stat(victim, STAT_STR) * 6 / 5;

    /* dex */
    chance += get_curr_stat(ch, STAT_DEX);
    chance -= get_curr_stat(victim, STAT_DEX) * 3 / 2;

    /* level */
    if ( (victim->level - ch->level) > 7 )
        chance -= (victim->level) * 5;
    chance += (ch->level - victim->level) *2;
    
         
    if (number_percent( ) < chance )
    {   
  
        WAIT_STATE( ch, skill_table[gsn_breakneck].beats );
        act("$n hits you in the nape of the neck with a solid THUD!",ch,NULL,victim,TO_VICT);
        act(" ",ch,NULL,victim,TO_VICT);
        act("You feel dizzy as everything around you fades to black.",ch,NULL,victim,TO_VICT);
        act("You hit $N on the nape of the neck with a solid THUD!",ch,NULL,victim,TO_CHAR);
        act(" ",ch,NULL,victim,TO_CHAR);
        act("$N goes cross-eyed a moment, then slumps to the ground unconscious.",ch,NULL,
             victim,TO_CHAR);
        act("$n hits $N in the nape of the neck with a solid THUD!",ch,NULL,victim,TO_NOTVICT); 
        act(" ",ch,NULL,victim,TO_NOTVICT);
        act("$N goes cross-eyed a moment, then slumps to the ground unconscious.",
             ch,NULL,victim,TO_NOTVICT); 
        victim->position  = POS_SLEEPING;
	 af.where	  = TO_AFFECTS;
         af.type          = gsn_sleep;
         af.level         = ch->level;       
         af.duration      = 1;         
         af.location      = APPLY_NONE;
         af.modifier      = 0;
         af.bitvector     = AFF_SLEEP;
         affect_join(victim,&af);
         check_cpose(ch);  
         check_cpose(victim);

         check_improve(ch,gsn_breakneck,TRUE,1);
         if ( !IS_NPC(victim) )
         {
             if ( !IS_IMMORTAL(ch) )
             {
                ch->quit_timer = -5;
                ch->lastfought = victim;
                if (!IS_NPC(victim))
                {
                  free_string(ch->last_pc_fought);
                  ch->last_pc_fought = str_dup(victim->name);
                }

             }
             victim->quit_timer = -5;
             victim->lastfought = ch;
             if (!IS_NPC(victim))
             {
               free_string(ch->last_pc_fought);
               ch->last_pc_fought = str_dup(victim->name);
             }

         }
    }
    else
    {    
      act("$n tries to thump you in the nape of the neck!",ch,NULL,victim,TO_VICT);
      act("You attempt to thump $N in the nape of the neck, but miss.",ch,NULL,victim,TO_CHAR);
      act("$n tries to thump $N in the nape of the neck, but misses.",ch,NULL,victim,TO_NOTVICT); 

      if(!(IS_NPC(ch)))
      {     
        if (!can_see(victim,ch))
          do_yell(victim, "Help! Someone tried to break my neck!");
        else
        {
          if (IS_NPC(ch))
                sprintf(buf, "Help! %s just tried to break my neck!", ch->short_descr);
          else
            sprintf( buf, "Help! %s just tried to break my neck!", 
            ((is_affected(ch,gsn_doppelganger)) && (!IS_IMMORTAL(victim)))? ch->doppel->name :
            ((is_affected(ch,gsn_treeform)) && (!IS_IMMORTAL(victim)))? "A mighty oak tree" :
            ch->name);
            do_yell( victim, buf );
        }
      }
      check_improve(ch,gsn_breakneck,FALSE,1);
      multi_hit( ch, victim, TYPE_UNDEFINED );
      return;
    }
}

void do_strangle( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *wield;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    AFFECT_DATA af;  
    int chance;
 
    one_argument( argument, arg );
    wield = get_eq_char( ch, WEAR_WIELD );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Choke whom?\n\r", ch );
        return;
    }

    if ( !IS_NPC(ch) && ch->pcdata->learned[gsn_strangle] < 1 )
    {
        send_to_char("You don't know that skill.\n\r",ch);
        return;
    }

    if ( ch->riding )
    {
        send_to_char( "You can't get close enough while mounted.\n\r", ch );
        return;
    }

    if ( IS_AFFECTED2(ch, AFF_PARALYSIS) 
    ||   IS_AFFECTED3(ch, AFF_HOLDPERSON))
    {
      send_to_char("You can't move at all, there's no way you can do that.\n\r",ch);
      return;
    }

    if ( wield == NULL)
	{
	send_to_char("You must be wielding a whip in order to strangle someone.\n\r",ch);
	return;
	}
    if ( wield->value[0] != WEAPON_WHIP )
	{
	send_to_char("You must be wielding a whip in order to strangle someone.\n\r",ch);
	return;
	}

    if ( ( victim = get_char_room( ch, NULL, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( victim == ch )
    {
        send_to_char( "You can't strangle yourself.\n\r", ch );
        return;
    }

    if ( ch->lastfought != NULL && victim->lastfought != NULL && !IS_NPC(victim) )
    {
        if ( victim == ch->lastfought
	|| ch == victim->lastfought )
        {
            send_to_char("They are wary of your attack!\n\r", ch );
            return;
        }
    }

    if (IS_AFFECTED(victim,AFF_SLEEP))
    {
        act("$e's already asleep.  Trying to wake them up?.",
            ch,NULL,victim,TO_CHAR);
        return;
    }

    if ( IS_AFFECTED(ch, AFF_CHARM) )
    {
        send_to_char("Your master doesn't agree with your will!",ch);
        return;
    }

    if ( is_safe( ch, victim ) )
      return;
  
    if (victim->riding)
    {
	act("$N is mounted and $S back is covered.\n\r",ch, NULL, victim,TO_CHAR);
	return;
    }

    /* modifiers */

    chance = get_skill(ch, gsn_strangle);

    switch(check_immune(victim,1))
    {
        case(IS_IMMUNE):
            send_to_char("Your choking whip has no effect!\n\r",ch);
            return;
            break;
        case(IS_RESISTANT):     
            chance -= (victim->level) * 4;
            break;
    }

    /* str */
    chance += get_curr_stat(ch, STAT_STR);
    chance -= get_curr_stat(victim, STAT_STR) * 4 / 3;

    /* dex */
    chance += get_curr_stat(ch, STAT_DEX);
    chance -= get_curr_stat(victim, STAT_DEX) * 3 / 2;

    /* level */
    if ( (victim->level - ch->level) > 7 )
        chance -= (victim->level) * 5;
    chance += (ch->level - victim->level) *2;
    
         
    if (number_percent( ) < chance )
    {   
  
         WAIT_STATE( ch, skill_table[gsn_strangle].beats );
         act("$n coils a whip around your neck and yanks you to the ground!",
              ch,NULL,victim,TO_VICT);
         act(" ",ch,NULL,victim,TO_VICT);
         act("{rYOU CAN'T BREATHE!!!{x",ch,NULL,victim,TO_VICT);
         act(" ",ch,NULL,victim,TO_VICT);
         act(" ",ch,NULL,victim,TO_VICT);
         act("You pass out from asphyixiation.",ch,NULL,victim,TO_VICT);
         act("You swiftly coil your whip around $N's neck, bringing $m to the ground in one rapid stroke.",ch,NULL,
             victim,TO_CHAR);
         act("$n swiftly coils $s whip around $N's neck, bringing $m to the ground in one rapid stroke.",ch,NULL,victim,TO_NOTVICT); 
         victim->position  = POS_SLEEPING;
	   af.where	  	  = TO_AFFECTS;
         af.type          = gsn_sleep;
         af.level         = ch->level;       
         af.duration      = 1;         
         af.location      = APPLY_NONE;
         af.modifier      = 0;
         af.bitvector     = AFF_SLEEP;
         affect_join(victim,&af);
         check_cpose(ch);
         check_cpose(victim);

         check_improve(ch,gsn_strangle,TRUE,1);
         if ( !IS_NPC(victim) )
         {
             if ( !IS_IMMORTAL(ch) )
             {
                ch->quit_timer = -5;
                ch->lastfought = victim;
                if (!IS_NPC(victim))
                {
                  free_string(ch->last_pc_fought);
                  ch->last_pc_fought = str_dup(victim->name);
                }

             }
             victim->quit_timer = -5;
             victim->lastfought = ch;
             if (!IS_NPC(victim))
             {
                 free_string(ch->last_pc_fought);
                 ch->last_pc_fought = str_dup(victim->name);
             }

         }
    }
    else
    {    

        act("$n tries to strangle you with a coiled whip!",ch,NULL,victim,TO_VICT);
        act("Your strangle attempt misses.",ch,NULL,victim,TO_CHAR);
        act("$n tries to strangle $N with a coiled whip, but misses."
             ,ch,NULL,victim,TO_NOTVICT); 
       
        if(!(IS_NPC(ch)))
        {     
            if (!can_see(victim,ch))
                do_yell(victim, "Help! Someone tried to strangle me!");
            else
            {
            if (IS_NPC(ch))
                sprintf(buf, "Help! %s just tried to strangle me!", ch->short_descr);
            else
            sprintf( buf, "Help! %s just tried to strangle me!", 
            ((is_affected(ch,gsn_doppelganger)) && (!IS_IMMORTAL(victim)))? ch->doppel->name :
            ((is_affected(ch,gsn_treeform)) && (!IS_IMMORTAL(victim)))? "A mighty oak tree" :
            ch->name);
            do_yell( victim, buf );
          }
        }
        check_improve(ch,gsn_strangle,FALSE,1);
        multi_hit( ch, victim, TYPE_UNDEFINED );
        return;
    }
  }

/*
 * Check if guarded.
 */
bool check_guard( CHAR_DATA *ch, CHAR_DATA *victim )
{
 int chance;

 if ( victim->guarded == NULL ) 
   return FALSE;
 
 if ( victim->in_room != victim->guarded->in_room )
   return FALSE;

 if ( victim->guarded->position != POS_FIGHTING )
   return FALSE;
 
#if 0
 if( !IS_NPC(ch) && (ABS(victim->guarded->level - ch->level ) > 8)) 
   return FALSE;
#endif

 if( IS_AFFECTED2(victim->guarded, AFF_INVUL) )
   return FALSE;

 if( IS_AFFECTED3(victim->guarded, AFF_SUBDUE)
 &&  IS_SET(ch->act,PLR_AUTOMERCY))
   return FALSE;

 if (!IS_NPC(ch)
 && !IS_NPC(victim)
 && (ABS(ch->level - victim->level) > 10)
 && (ch->level <= 101)
 && IS_AFFECTED3(victim->guarded,AFF_SUBDUE))
   return FALSE;

 chance = get_skill(victim->guarded,gsn_guard) / 2;

 if ( number_percent() >= chance )
 {
  act("You fail to protect $N!", victim->guarded, NULL, victim, TO_CHAR);
  act("$n fails to protect you!", victim->guarded, NULL, victim, TO_VICT);
  return FALSE;
 }

 check_improve(victim->guarded,gsn_guard,TRUE,6);
 return TRUE;
}

/* Guard Skill Allows Target To Get Off Scott Free */
void do_guard( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );


    if (IS_NPC(ch))
    {
        return;
    }

    if ( arg[0] == '\0' )
    {
      if ( (ch->guarded == NULL) && (ch->guarding == NULL) )
	{
        send_to_char( "Guard whom?\n\r", ch );
        return;
	}
      if ( (ch->guarded != NULL) )
	{
        act("You will no longer be guarded.", ch, NULL, NULL, TO_CHAR);
        act("$n stops you from guarding them.", ch, NULL, ch->guarded, TO_VICT);
	  ch->guarded->guarding = NULL;         ch->guarded = NULL;
        return;
	}
      if ( (ch->guarding != NULL) )
	{
        act("You will no longer guard $N.", ch, NULL, ch->guarding, TO_CHAR);
        act("$n stops guarding you!", ch, NULL, ch->guarding, TO_VICT);
	  ch->guarding->guarded = NULL;
	  ch->guarding = NULL;
	  return;
	}
    }

    if ( ( victim = get_char_room( ch, NULL, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if (IS_NPC(victim))
    {
        send_to_char("They would be better off alone.\n\r",ch);
        return;
    }

 
    if ( get_skill(ch,gsn_guard) == 0
    ||   (IS_NPC(ch))
    ||   (!IS_NPC(ch)
    &&    ch->level < skill_table[gsn_guard].skill_level[ch->class]))
    {   
        send_to_char("They would be better off alone.\n\r",ch);
        return;
    }

    if ( ch->guarded != NULL )
    {
      act( 
   "You are blocked from helping by $N's superb job of protecting your hide!", 
      ch, NULL, ch->guarded, TO_CHAR);
      return;
    }

    if ( victim == ch )
    {
        send_to_char( "What about fleeing instead?\n\r", ch );
        return;
    }

    if ( victim == ch->guarding )
    {
        send_to_char( "You are already guarding them!\n\r", ch );
        return;
    }

    if ( victim->guarded != NULL )
    {
        send_to_char( "They seem to be under someone else's protection.\n\r", ch);
        return;
    }

    if ( victim == ch->fighting)
    {
        send_to_char("How can you guard somebody you're fighting?\n\r",ch);
        return;
    } 

    if ( ch->guarding != NULL )
    {
      act("$n stops guarding you!", ch, NULL, ch->guarding, TO_VICT);
      act("You stop guarding $N.", ch, NULL, ch->guarding, TO_CHAR);
      ch->guarding->guarded = NULL;
	ch->guarding = NULL;
    }
    ch->guarding = victim;
    ch->guarding->guarded = ch;
    act("$n starts guarding you!", ch, NULL, ch->guarding, TO_VICT);
    act("You will now guard $N from harm.", ch, NULL, ch->guarding, TO_CHAR);
    return;
}

void do_cutthroat( CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH]; 
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    int fight;
    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
        send_to_char( "Cut who's throat?\n\r", ch );
        return;
    }

    if (IS_NPC(ch))
    {
      return;
    }

    if( ch->pcdata->learned[gsn_cutthroat] <= 1 )
    {
        send_to_char( "You don't know that skill.\n\r", ch );
        return;
    }
    if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL)
    {
        send_to_char( "You need to wield a weapon to cutthroat.\n\r", ch );
        return;
    }
    
    if (obj->value[0] != WEAPON_DAGGER )
    {
        send_to_char ("You can only cutthroat with daggers.\n\r",
        ch );
        return;
    }

    if (ch->fight_pos > FIGHT_FRONT)
    {
        send_to_char("You are too far away to cutthroat!\r\n", ch);
        return;
    }

    if ( ( victim = get_char_room( ch, NULL, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( victim == ch )
    {
        send_to_char( "That's pointless\n\r", ch );
        return;
    }

    if ( IS_AFFECTED2(ch, AFF_PARALYSIS) 
    ||   IS_AFFECTED3(ch, AFF_HOLDPERSON))
    {
      send_to_char("You can't move at all, there's no way you can do that.\n\r",ch);
      return;
    }

    if ( is_safe( ch, victim ) )
      return;

    if ( victim->hit < victim->max_hit )
    {
        act( "$N is hurt, suspicious, and looking all around.",
            ch, NULL, victim, TO_CHAR );
        return;
    }

    if ( victim->position != POS_SLEEPING )
    {
     send_to_char("You can only cutthroat sleeping people!\n\r",ch);
     return;
    }
 
    fight = 0;


if ( number_percent( ) > ch->pcdata->learned[gsn_cutthroat] )
   {
     check_improve(ch,gsn_cutthroat,FALSE,1); 
    if (!IS_NPC(victim) && fight != 1)
    {
        if (!can_see(victim,ch) )
            do_yell(victim, "Help! Someone tried to cut my throat!");
        else
            {
            if (IS_NPC(ch))
              sprintf(buf, "Die, %s, you cutthroating fool!", ch->short_descr);
            else
              sprintf(buf, "Die, %s, you cutthroating fool!",
              ((is_affected(ch,gsn_doppelganger)) && (!IS_IMMORTAL(victim)))? ch->doppel->name :
              ((is_affected(ch,gsn_treeform)) && (!IS_IMMORTAL(victim)))? "A mighty oak tree" :
              ch->name);
              do_yell( victim, buf );
            }
    }
  return;
  }
    one_hit( ch, victim, gsn_cutthroat,FALSE );

    if (!IS_NPC(victim) && fight != 1)
    {
        if (!can_see(victim,ch) )
            do_yell(victim, "Argh! Someone just cut my throat!");
        else
            {
            if (IS_NPC(ch))
                sprintf(buf, "Argh! %s just cut my throat!",
                    ch->short_descr);
            else
                sprintf(buf, "Argh! %s just cut my throat!",
                ((is_affected(ch,gsn_doppelganger)) && (!IS_IMMORTAL(victim)))? ch->doppel->name :
                ((is_affected(ch,gsn_treeform)) && (!IS_IMMORTAL(victim)))? "A mighty oak tree" :
                ch->name);
            do_yell( victim, buf );
            }
     }


    check_improve(ch,gsn_cutthroat,TRUE,1);

return;
}

void do_head_butt( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int chance;
  int fight;
  int damp;
  one_argument(argument,arg);
 
  if ( (chance = get_skill(ch,gsn_head_butt)) == 0
  ||   (!IS_NPC(ch)
  &&    ch->level < skill_table[gsn_head_butt].skill_level[ch->class]))
  {   
      send_to_char("Head butting? What's that?\n\r",ch);
      return;
  }
  if (arg[0] == '\0')
  {
      victim = ch->fighting;
      if (victim == NULL)
      {
          send_to_char("But you aren't fighting anyone!\n\r",ch);
          return;
      }
  }
  else if ((victim = get_char_room(ch, NULL, arg)) == NULL)
  {
      send_to_char("They aren't here.\n\r",ch);
      return;
  }
  if (ch->riding)
  {
    send_to_char("You can't do that while mounted.\n\r",ch);
    return;
  }

  if (victim->position < POS_FIGHTING)
  {
      act("You'll have to let $M get back up first.",ch,NULL,victim,TO_CHAR);
      return;
  } 

  if (victim == ch)
  {
      send_to_char("Head butting yourself is very hard to do.\n\r",ch);
      return;
  }

  if ( IS_AFFECTED2(ch, AFF_PARALYSIS) 
  ||   IS_AFFECTED3(ch, AFF_HOLDPERSON))
  {
    send_to_char("You can't move at all, there's no way you can do that.\n\r",ch);
    return;
  }

  if (is_safe(ch,victim))
      return;

  if ( victim->fighting != NULL && !is_same_group(ch,victim->fighting))
  {
      send_to_char("Kill stealing is not permitted.\n\r",ch);
      return;
  }

  if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
  {
      act("But $N is your friend!",ch,NULL,victim,TO_CHAR);
      return;
  }
  /* MGD */
  if (ch->fight_pos > FIGHT_FRONT )
  {
      send_to_char("You are not close enough to head butt!\n\r", ch);
      return;
  }
  
  /* modifiers */
  /* size  and weight */
  chance += ch->carry_weight / 25;
  chance -= victim->carry_weight / 20;
  if (ch->size < victim->size)
      chance += (ch->size - victim->size) * 25;
  else
      chance += (ch->size - victim->size) * 10; 

  /* stats */
  chance += get_curr_stat(ch,STAT_STR);
  chance -= get_curr_stat(victim,STAT_DEX) * 4/3;
  /* speed */
  if (IS_SET(ch->off_flags,OFF_FAST))
      chance += 10;
  if (IS_SET(ch->off_flags,OFF_FAST))
      chance -= 20;
  /* level */
  chance += (ch->level - victim->level) * 2;

  fight = 0;
  if (ch->fighting == NULL)
      fight = 1;
  /* now the attack */
  if (number_percent() < chance)
  {
      act("$n grabs you by the neck and slams $s head into your face.",ch,NULL,victim,TO_VICT);
      act("You grab $N by the neck and slam your head into $S face.",ch,NULL,victim,TO_CHAR);
      act("$n grabs $N by the neck and slams $s head into $S face.",ch,NULL,victim,TO_NOTVICT);
      damp=number_range(1,3);
      if ( damp == 1 )
      {
	 if (!IS_SET(victim->loc_hp,BROKEN_NOSE) && !IS_SET(victim->loc_hp,LOST_NOSE))
	 act("Your nose shatters under the impact of the blow!",victim,NULL,NULL,TO_CHAR);
    	 act("$n's nose shatters under the impact of the blow!",victim,NULL,NULL,TO_ROOM);
	 SET_BIT(victim->loc_hp,BROKEN_NOSE);
      }
      else if ( damp == 2 )
      {
       if (!IS_SET(victim->loc_hp,BROKEN_JAW))
       act("Your jaw shatters under the impact of the blow!",victim,NULL,NULL,TO_CHAR);
    	 act("$n's jaw shatters under the impact of the blow!",victim,NULL,NULL,TO_ROOM);
	 SET_BIT(victim->loc_hp,BROKEN_JAW);
      }
      else if ( damp == 3 )
      {
	 if (!IS_SET(victim->loc_hp,BROKEN_NECK))
	 act("Your neck shatters under the impact of the blow!",victim,NULL,NULL,TO_CHAR);
    	 act("$n's neck shatters under the impact of the blow!",victim,NULL,NULL,TO_ROOM);
	 SET_BIT(victim->loc_hp,BROKEN_NECK);
      }
 
      check_improve(ch,gsn_head_butt,TRUE,1);
      WAIT_STATE(ch,skill_table[gsn_head_butt].beats);
      if (!victim->riding)
        victim->position = POS_RESTING;
 
     if (ch->race != race_lookup("minotaur"))
      damage(ch,victim,number_range(2,2 + 2 * ch->size + chance/20),gsn_head_butt,
          DAM_BASH, TRUE);

      if (ch->race == race_lookup("minotaur"))
      damage(ch,victim,(number_range(2,2 + 2 * ch->size + chance/20)*2),gsn_head_butt,
          DAM_PIERCE, TRUE);

      if (!IS_NPC(victim) && fight == 1 
         && victim->position > POS_SLEEPING)
      {
         if (!can_see(victim,ch))
            do_yell(victim, "Help! Someone is attacking me!");
         else
         {
            if (IS_NPC(ch))
                sprintf(buf, "Help! %s is attacking me!", ch->short_descr);
            else
               sprintf(buf, "Help! %s is attacking me!", 
               (is_affected(ch,gsn_doppelganger) && (!IS_IMMORTAL(victim)))? ch->doppel->name :
               ((is_affected(ch,gsn_treeform)) && (!IS_IMMORTAL(victim)))? "A mighty oak tree" :
	         ch->name);
            do_yell( victim, buf );
        }
    }
    }
    else
    {
    if (ch->fighting == NULL && !IS_NPC(victim))
    {
    if (!can_see(victim,ch))
        do_yell(victim, "Hey! Someone is attacking me!");
    else
        {
        if (IS_NPC(ch))
            sprintf(buf, "Hey! %s is attacking me!",
                ch->short_descr);
        else
            sprintf(buf, "Hey! %s is attacking me!", 
               (is_affected(ch,gsn_doppelganger) && (!IS_IMMORTAL(victim)))? ch->doppel->name :
               ((is_affected(ch,gsn_treeform)) && (!IS_IMMORTAL(victim)))? "A mighty oak tree" :
	         ch->name);
        do_yell( victim, buf);
        }
    }
        damage(ch,victim,0,gsn_head_butt,DAM_BASH, TRUE);
        act("Your head butt misses!",
            ch,NULL,victim,TO_CHAR);
        act("$n tries to head butt and misses.",
            ch,NULL,victim,TO_NOTVICT);
        act("You evade $n's head butt.",
            ch,NULL,victim,TO_VICT);
        check_improve(ch,gsn_head_butt,FALSE,1);
        ch->position = POS_RESTING;
        WAIT_STATE(ch,skill_table[gsn_head_butt].beats * 3/2); 
    }
 return;
}


void do_assassinate( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg );
    one_argument( argument, arg2 );

    if ( !IS_NPC( ch ) 
    && ch->level < skill_table[gsn_assassinate].skill_level[ch->class] )
    {
      send_to_char( "You don't know how to do that.\n\r", ch );
      return;
    }

    if ( arg[0] == '\0' )
    {
	send_to_char( "Assassinate whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, NULL, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( ch == victim )
    {
	send_to_char( "Yes, that's what we need.... more suicide!\n\r", ch );
	return;
    }

    if ( ch->fighting != NULL )
    {
	send_to_char( "Your too busy fighting.\n\r", ch );
	return;
    }

    if ( IS_AFFECTED2(ch, AFF_PARALYSIS) 
    ||   IS_AFFECTED3(ch, AFF_HOLDPERSON))
    {
      send_to_char("You can't move at all, there's no way you can do that.\n\r",ch);
      return;
    }

    if (ch->fight_pos > FIGHT_FRONT)
    {
	send_to_char("You are too far away to assassinate anyone!\r\n", ch);
	return;
    }

    if ( IS_IMMORTAL(victim) )
    {
	send_to_char( "You'll only piss them off if you try.\n\r", ch );
	return;
    }

    if ( is_safe(ch, victim) )
	return;

    if ( !str_cmp( arg2, "choke" ) )
    {
      act( "You wrap your meaty arms around $N's neck choking out the life.\n\r",  ch, NULL, victim, TO_CHAR    );
      act( "$n wrapped meaty arms around your neck and tries to choke you to death.\n\r", ch, NULL, victim, TO_VICT    );
      act( "$n wraps meaty arms around $N's neck choking out $S life.\n\r",  ch, NULL, victim, TO_NOTVICT );
    }

    else if ( !str_cmp( arg2, "poison" ) )
    {
      act( "You blow poison powder in $N's face causing them to convulse as\n\rtheir blood vessels explode.\n\r",  ch, NULL, victim, TO_CHAR    );
      act( "$n blows poison powder in your face, causing you to convulse as your\n\rblood vessels explode.\n\r", ch, NULL, victim, TO_VICT    );
      act( "$n blows poison powder in $N's face causing convulsions\n\ras $N's blood vessels explode.\n\r",  ch, NULL, victim, TO_NOTVICT );
    }
    else if (!str_cmp( arg2, "backstab" ) )
     {
      act( "Your backstab finds its mark between $N's shoulder blades.\n\r",  ch, NULL, victim, TO_CHAR );
      act( "$n backstabs you square between the shoulder blades, trying to end your life.\n\r",  ch, NULL, victim, TO_VICT );
      act( "$n backstabs $N square between the shoulder blades, trying to end $S life.\n\r",  ch, NULL, victim, TO_NOTVICT );
     }
    else
     { 
	send_to_char( "What kind of assassination attempt: poison, choke, backstab?\n\r", ch );
	return;
     } 

    if ( !IS_AWAKE(victim)
    || ( number_percent() < 10 ))
    {
      check_improve(ch,gsn_assassinate,TRUE,1);
      act("Your assassination attempt was successful! $N now lies dead!\n\r",ch,NULL,victim,TO_CHAR);
      act("$n has assassinated you! How do you like them apples?\n\r",ch,NULL,victim,TO_VICT);
      act("$n has assassinated $N! The horror of it all!\n\r",ch,NULL,victim,TO_NOTVICT);
      raw_kill( ch, victim );

      sprintf( log_buf, "%s was assassinated by %s at %s [room %d]",
      	(IS_NPC(victim) ? victim->short_descr : victim->name),
            (IS_NPC(ch) ? ch->short_descr : ch->name),
            ch->in_room->name, ch->in_room->vnum);
 
      if (IS_NPC(victim))
      	wiznet(log_buf,NULL,NULL,WIZ_MOBDEATHS,0,0);
      else
            wiznet(log_buf,NULL,NULL,WIZ_DEATHS,0,0);

	/* Begin Paladine's PK Logger code*/
	if(!IS_NPC(victim) && ch != victim)
	{
	  CHAR_DATA *iPked;

	  iPked = ch;

	  /* Follow charm thread to responsible character. */
    	  while ( IS_AFFECTED(iPked, AFF_CHARM) && iPked->master != NULL )
	  iPked = iPked->master;

	  /* Log both Cheap PKILL via your charmie and Regular PKILL */
	  if( !IS_NPC(iPked) || !IS_NPC(ch) )
	  { 
	    FILE *fp;
	    char *strtime = NULL;

  	    /* write to the pk log */
    	    fclose(fpReserve);
    	    if( (fp = fopen(PK_LOG, "a")) == NULL)
    	    {
		bug("Error opening pk_log.txt",0);
		fclose(fp);
		fpReserve = fopen(NULL_FILE, "r");
    	    }
	    else
	    {
		sprintf( log_buf, "%s was ASSASSINATED by %s at room %d",
      		victim->name, ch->name, ch->in_room->vnum);
    		strtime = ctime(&current_time);
    		strtime[strlen(strtime)-1] = '\0';
		sprintf(buf, "%s :: %s\n", strtime, log_buf);
    		fprintf(fp, "%s", buf);
    		fclose(fp);
    		fpReserve = fopen(NULL_FILE, "r");
	    }
	  }
	} /* End PK Logger */

	sprintf( log_buf, "%s assassinated by %s at %d",victim->name,
	  (IS_NPC(ch) ? ch->short_descr : ch->name),
	  ch->in_room->vnum );
	log_string( log_buf );
    }
    else
    {
	act("$n's assassination attempt on your life is a failure!\n\r",ch,NULL,victim,TO_VICT);
	act("Your assassination attempt on $N's life is a failure!\n\r",ch,NULL,victim,TO_CHAR);
	act("$n's assassination attempt on $N's life is a failure!\n\r",ch,NULL,victim,TO_NOTVICT);
      if (!ch->fighting)
        set_fighting(ch, victim);
	check_improve(ch,gsn_assassinate,FALSE,1);
    }
  return;
}

void do_crush( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;

    if ( !IS_NPC(ch)
    &&   ch->level < skill_table[gsn_tail].skill_level[ch->class] )
    {
	send_to_char( "Since when did you grow a tail?\n\r", ch );
      return;
    }

    if (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_TAIL))
	return;

    if ( ( victim = ch->fighting ) == NULL )
    {
	send_to_char( "You aren't fighting anyone.\n\r", ch );
	return;
    }

    WAIT_STATE( ch, skill_table[gsn_tail].beats );
    if ( get_skill(ch,gsn_tail) > number_percent())
    {
	damage(ch,victim,number_range( 1, ch->level ), gsn_tail,DAM_SLASH,TRUE);
	check_improve(ch,gsn_tail,TRUE,1);
    }
    else
    {
	damage( ch, victim, 0, gsn_tail,DAM_SLASH,TRUE);
	check_improve(ch,gsn_tail,FALSE,1);
    }
	check_killer(ch,victim);
    return;
}


void do_tail( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;

    if ( !IS_NPC(ch)
    &&   ch->level < skill_table[gsn_tail].skill_level[ch->class] )
    {
	send_to_char( "Since when did you grow a tail?\n\r", ch );
      return;
    }

    if (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_TAIL))
	return;

    if ( ( victim = ch->fighting ) == NULL )
    {
	send_to_char( "You aren't fighting anyone.\n\r", ch );
	return;
    }

    WAIT_STATE( ch, skill_table[gsn_tail].beats );
    if ( get_skill(ch,gsn_tail) > number_percent())
    {
	damage(ch,victim,number_range( 1, ch->level ), gsn_tail,DAM_SLASH,TRUE);
	check_improve(ch,gsn_tail,TRUE,1);
    }
    else
    {
	damage( ch, victim, 0, gsn_tail,DAM_SLASH,TRUE);
	check_improve(ch,gsn_tail,FALSE,1);
    }
	check_killer(ch,victim);
    return;
}

void do_backup( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *gch;
  CHAR_DATA *guard2;
  CHAR_DATA *guard;
  AFFECT_DATA af;
  int i;
  bool org_check = FALSE;

   if ((ORG(ch->org_id) == ORG_SOLAMNIC)
   || (ORG(ch->org_id)  == ORG_MERCENARY)
   || (ORG(ch->org_id)  == ORG_KOT))
       org_check = TRUE;

   if (org_check == FALSE)
   {
      send_to_char("You are not capable of receiving backup.\n\r",ch);
      return;
   }

   if (ch->in_room->sector_type != SECT_CITY)
   {
     if ((ORG(ch->org_id) == ORG_MERCENARY)
     &&  (ch->in_room->sector_type != SECT_MERCENARY))
     {
       send_to_char("The Mercenary Guild has no authority here, you cannot call for backup.\n\r", ch);
       return;
     }

     if ((ORG(ch->org_id) == ORG_SOLAMNIC)
     &&  (ch->in_room->sector_type != SECT_SOLAMNIC))
     {
       send_to_char("The Solamnic Knights have no authority here, you cannot call for backup.\n\r", ch);
       return;
     }

     if ((ORG(ch->org_id) == ORG_KOT)
     &&  (ch->in_room->sector_type != SECT_KOT))
     {
       send_to_char("The Knights of Takhisis have no authority here, you cannot call for backup.\r", ch);
       return;
     }
   }

  if ( IS_NPC(ch)
  || ( !IS_NPC( ch ) && get_skill(ch,gsn_backup) < 1 ))
   {
      send_to_char("You are not capable of receiving backup.\n\r",ch);
      return;
   }

  if  (ch->pcdata->condition[COND_DRUNK] > 5)
    {
	if (ORG(ch->org_id)  == ORG_MERCENARY)
	{
                send_to_char("The hounds can't understand you.\n\r", ch);
                return;
	}

	if (ORG(ch->org_id)  == ORG_SOLAMNIC)
	{
                send_to_char("The wardens can't understand your call.\n\r", ch);
                return;
	}

	if (ORG(ch->org_id)  == ORG_KOT)
	{
          send_to_char("The sentries can't understand your call.\r", ch);
          return;
	}
    }

  if (is_affected(ch, gsn_backup))
    {
        if (ORG(ch->org_id)  == ORG_MERCENARY)
        {
      	  send_to_char("The hounds don't listen to you.\n\r", ch);
      	  return;
        }

        if (ORG(ch->org_id)  == ORG_SOLAMNIC)
        {
      	  send_to_char("The wardens don't heed your call.\n\r", ch);
      	  return;
        }

	if (ORG(ch->org_id)  == ORG_KOT)
	{
      	  send_to_char("The sentries ignore you.\r", ch);
      	  return;
	}
    }

  if (ch->mana < skill_table[gsn_backup].min_mana)
    {
      send_to_char("You can't concentrate enough.\n\r", ch);
      return;
    }

  for (gch = char_list; gch != NULL; gch = gch->next)
    {
      if (IS_NPC(gch) && IS_AFFECTED(gch,AFF_CHARM) && gch->master == ch &&
	gch->pIndexData->vnum == MOB_VNUM_GUARD)
	{
             if (ORG(ch->org_id)  == ORG_MERCENARY)
             {		
      		send_to_char("The hounds refuse your summons.\n\r", ch);
      		return;
             }
		
      	 if (ORG(ch->org_id)  == ORG_SOLAMNIC)
	       {		
                send_to_char("The wardens don't heed your call.\n\r", ch);
      		return;
	       }

		 if (ORG(ch->org_id)  == ORG_KOT)
             {
      		send_to_char("The sentries ignore you.\r", ch);
      		return;
	       }
      }
    }

  send_to_char("You attempt to call for backup.\r\n", ch);
  act("$n attempts to call for backup.", ch, NULL, NULL, TO_ROOM);

  ch->mana -= skill_table[gsn_backup].min_mana;

  guard = create_mobile( get_mob_index(MOB_VNUM_GUARD) );

  for (i=0;i < MAX_STATS; i++)
    {
      guard->perm_stat[i] = UMIN(25,2 * ch->perm_stat[i]);
    }

  guard->max_hit = IS_NPC(ch)? ch->max_hit : ch->pcdata->perm_hit;
  guard->hit = guard->max_hit;
  guard->max_mana = IS_NPC(ch)? ch->max_mana : ch->pcdata->perm_mana;
  guard->mana = guard->max_mana;
  guard->alignment = ch->alignment;
  guard->level = UMIN(70,ch->level);
  for (i=0; i < 3; i++)
    guard->armor[i] = interpolate(guard->level,100,-100);
  guard->armor[3] = interpolate(guard->level,100,0);
  guard->sex = ch->sex;
  guard->gold = 0;

   if (ORG(ch->org_id)  == ORG_MERCENARY)
   {
  	do_yell(ch, "Release the Hounds!");
      guard->race	= race_lookup("wolf");

  	free_string(guard->name);
  	guard->name = str_dup("hound mercenary");

  	free_string(guard->short_descr);
  	guard->short_descr = str_dup("a vicious hound");
 
  	free_string(guard->long_descr);
  	guard->long_descr = str_dup("A hound snarls viciously\r\n");

  	free_string(guard->description);
  	guard->description = str_dup("The hound shows large red eyes, and vicious fangs.\r\n"
	"Standing by its master, the hound upholds that which\r\n"
      "its master wishes to uphold.\r\n");
 
  	send_to_char("You successfully summon two hounds to your side!\n\r",ch);
  	act("Two hounds appears at $n's side!",ch,NULL,NULL,TO_ROOM);
   }

   if (ORG(ch->org_id)  == ORG_SOLAMNIC)
   {
  	do_yell(ch, "Wardens!  To Arms!");

  	free_string(guard->name);
  	guard->name = str_dup("warden solamnic");
      guard->race	= race_lookup("human");

  	free_string(guard->short_descr);
  	guard->short_descr = str_dup("a towering warden");
 
  	free_string(guard->long_descr);
  	guard->long_descr = str_dup("A warden stands here, protecting the peace.\r\n");

  	free_string(guard->description);
  	guard->description = str_dup("A warden stands here stolidly, protecting those in harm.\r\n"
	"Standing by its master, the guard fights for the cause of\r\n"
      "upholding the law.\r\n");
  
  	send_to_char("You successfully summon two wardens to your side!\n\r",ch);
  	act("Two wardens appears at $n's side!",ch,NULL,NULL,TO_ROOM);
   }

   if (ORG(ch->org_id)  == ORG_KOT)
   {
  	do_yell(ch, "Come!  Sentries!");
      guard->race	= race_lookup("human");

  	free_string(guard->name);
  	guard->name = str_dup("sentry kot");

  	free_string(guard->short_descr);
  	guard->short_descr = str_dup("a strong sentry");
 
  	free_string(guard->long_descr);
  	guard->long_descr = str_dup("A strong sentry stands here, ready to fight.\r\n");

  	free_string(guard->description);
  	guard->description = str_dup("A sentry stands here stolidly, protecting it's master.\r\n"
	"Standing by its master, the sentry fights for the furtherment\r\n"
      "of the Knights of Takhisis.\r\n");
  
 	send_to_char("You successfully summon two sentries to your side!\n\r",ch);
  	act("Two sentries appear at $n's side!",ch,NULL,NULL,TO_ROOM);
   }

  guard2 = create_mobile(guard->pIndexData);
  clone_mobile(guard,guard2);
 
  SET_BIT(guard->affected_by, AFF_CHARM);
  SET_BIT(guard2->affected_by, AFF_CHARM);
  guard->master = guard2->master = ch;
  guard->leader = guard2->leader = ch;

  char_to_room(guard,ch->in_room);
  char_to_room(guard2,ch->in_room);

  af.where		= TO_AFFECTS;
  af.type               = gsn_backup;
  af.level              = ch->level; 
  af.duration           = 24;
  af.bitvector          = 0;
  af.modifier           = 0;
  af.location           = APPLY_NONE;
  affect_to_char(ch, &af); 
  return;
}

void do_impale( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    int fight, dam_type;
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Who do you wish to impale?\n\r", ch );
        return;
    }

    if( !IS_NPC(ch) && ch->pcdata->learned[gsn_impale] < 1 )
    {
        send_to_char( "You don't know that skill.\n\r", ch );
        return;
    }

    if ( ( victim = get_char_room( ch, NULL, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( victim == ch )
    {
        send_to_char( "You can't seem to impale yourself.\n\r", ch );
        return;
    }

    if ( IS_AFFECTED2(ch, AFF_PARALYSIS) 
    ||   IS_AFFECTED3(ch, AFF_HOLDPERSON))
    {
      send_to_char("You can't move at all, there's no way you can do that.\n\r",ch);
      return;
    }

    if ( is_safe( ch, victim ) )
      return;

    if ( victim->hit < victim->max_hit * 3 / 4)
    {
        act( "$N is hurt badly and is prepared for your surprise attack.",
            ch, NULL, victim, TO_CHAR );
        return;
    }

    if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL)
    {
        send_to_char( "You must wield a spear in order to impale someone.\n\r", ch );
        return;
    }
    dam_type = attack_table[obj->value[3]].damage;
    if ( obj->value[0] != WEAPON_SPEAR )
    {
        send_to_char("You must wield a spear in order to impale someone.\n\r",ch);
        return;
    }
    if ( victim->fighting != NULL )
    {
        send_to_char( "You cannot get enough distance to impale someone who is fighting.\n\r", ch );
        return;
    }
    /* MGD */
    if ( ch->fight_pos > FIGHT_FRONT )
    {
        send_to_char ( "You are too far away to attempt to impale your foe!\n\r", ch );
        return;
    }
    act("You extend your spear and thrust forward in an attempt to impale $N!", ch, NULL, victim, TO_CHAR);
    act("$n extends $s spear and thrusts forward in an attempt to impale you!",ch, NULL, victim, TO_VICT);
    act("$n extends $s spear and thrusts forward in an attempt to impale $N!", ch, NULL, victim, TO_NOTVICT);
    
    fight = 0;
    if ( victim->position <= POS_SLEEPING )
        fight = 1;
    check_killer( ch, victim );
    WAIT_STATE( ch, skill_table[gsn_impale].beats );
    if ( !IS_AWAKE(victim)
    ||   IS_NPC(ch)
    ||   number_percent( ) < ch->pcdata->learned[gsn_impale] )
    {
        check_improve(ch,gsn_impale,TRUE,1);
        one_hit( ch, victim, gsn_impale,FALSE );
    if (!IS_NPC(victim) && fight != 1)
    {
        if (!can_see(victim,ch) )
            do_yell(victim, "Ack! Someone just impaled me!");
        else
            {
            if (IS_NPC(ch))
                sprintf(buf, "Ack! %s just impaled me!",
                    ch->short_descr);
            else
                sprintf(buf, "Ack! %s just impaled me!",
               (is_affected(ch,gsn_doppelganger) && (!IS_IMMORTAL(victim)))? ch->doppel->name :
               ((is_affected(ch,gsn_treeform)) && (!IS_IMMORTAL(victim)))? "A mighty oak tree" :
	         ch->name);
          do_yell( victim, buf );
        }
    }
    }
    else
    {
    if (!IS_NPC(victim) && fight != 1)
    {
        if (!can_see(victim,ch) ) 
            do_yell(victim, "Help! Someone tried to impale me straight through the gut!");
        else
            {
            if (IS_NPC(ch))
                sprintf(buf, "Die, %s, you butchering fool!", ch->short_descr);
            else
                sprintf(buf, "Die, %s, you butchering fool!", 
               (is_affected(ch,gsn_doppelganger) && (!IS_IMMORTAL(victim)))? ch->doppel->name :
               ((is_affected(ch,gsn_treeform)) && (!IS_IMMORTAL(victim)))? "A mighty oak tree" :
	         ch->name);
            do_yell( victim, buf );
            }
    }
        check_improve(ch,gsn_impale,FALSE,1);
        damage( ch, victim, 0, gsn_impale,DAM_NONE,TRUE );
    }
    return;
}

void do_jab( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	OBJ_DATA *wield, *hold, *obj;

	one_argument(argument, arg);

	if( !IS_NPC(ch) && get_skill(ch, gsn_jab) < 1 )
	{
		send_to_char("You don't know how to do that.\n\r",ch);
		return;
	}

	if(arg[0] == '\0')
	{
		victim = ch->fighting;
	}
	else
	{
		victim = get_char_room(ch,NULL,arg);
	}
	
	if( victim == NULL )
	{
		send_to_char("Who do you wish to jab?\n\r",ch);
		return;
	}

	obj = NULL;
	wield = get_eq_char(ch, WEAR_WIELD);
	hold = get_eq_char(ch, WEAR_HOLD);

	if( wield == NULL && hold == NULL )
	{
		send_to_char("You need to hold or wield a staff to jab someone.\n\r",ch);
		return;
	}

	if(wield == NULL)
		obj = hold;

	if(hold == NULL)
		obj = wield;

	if(wield != NULL && hold != NULL)
	{
		if(hold->item_type == ITEM_STAFF)
			obj = hold;
		else
		{
			obj = wield;
		}
	}

	if(obj == NULL)
	{
		bugf("jab: NULL obj\n");
		return;
	}

	if ((obj->item_type != ITEM_STAFF)
	&&  (obj->value[0] != WEAPON_STAFF))
	{
		send_to_char("You must hold or wield a staff to jab someone.\n\r",ch);
		return;
	}

    	if(victim == ch)
    	{
      	send_to_char("Ouch..that hurt!\n\r",ch);
        	return;
    	}

    	if( is_safe(ch, victim) )
        return;

    	WAIT_STATE(ch, skill_table[gsn_jab].beats);
    	if (number_percent() > get_skill(ch, gsn_jab) )
    	{
        	damage( ch, victim, 0, gsn_jab,DAM_BASH,TRUE );
        	check_improve(ch,gsn_jab,FALSE,1);
		return;
    	}
    	else
    	{
        	damage( ch, victim, number_range( 1, ch->level ), gsn_jab,DAM_BASH,TRUE);
        	check_improve(ch,gsn_jab,TRUE,1);
		return;
    	}
   	return;
}

void do_cleave( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    int fight, dam_type;
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Who do you wish to cleave?\n\r", ch );
        return;
    }

    if( ( !IS_NPC(ch) && ch->pcdata->learned[gsn_cleave] < 1 )
    || (ch->level < skill_table[gsn_cleave].skill_level[ch->class]))
    {
        send_to_char( "You don't know that skill.\n\r", ch );
        return;
    }

    if ( ( victim = get_char_room( ch, NULL, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( victim == ch )
    {
        send_to_char( "You can't seem to cleave yourself.\n\r", ch );
        return;
    }

    if ( is_safe( ch, victim ) )
      return;

    if ( victim->hit < victim->max_hit * 3 / 4)
    {
        act( "$N is hurt badly and is prepared for your cleave attempt.",
            ch, NULL, victim, TO_CHAR );
        return;
    }

    if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL)
    {
        send_to_char( "You need to wield an axe in order to cleave someone.\n\r", ch );
        return;
    }

    dam_type = attack_table[obj->value[3]].damage;
    if ( obj->value[0] != WEAPON_AXE )
    {
        send_to_char("You cannot cleave your opponent with that weapon.\n\r",ch);
        return;
    }

    if ( victim->fighting != NULL )
    {
        send_to_char( "You cannot cleave someone who is fighting.\n\r", ch );
        return;
    }

    if ( IS_AFFECTED2(ch, AFF_PARALYSIS) 
    ||   IS_AFFECTED3(ch, AFF_HOLDPERSON))
    {
      send_to_char("You can't move at all, there's no way you can do that.\n\r",ch);
      return;
    }

    if ( ch->fight_pos > FIGHT_FRONT )
    {
        send_to_char ( "You are too far away to cleave!\n\r", ch );
        return;
    }

    if( !IS_NPC(ch) && (ch->pcdata->learned[gsn_axe] < 90 ))
    {
        send_to_char( "You are not yet skilled enough in wielding axes to cleave anyone.\n\r", ch );
        return;
    }
   
    act("You raise your axe high above your head and swing it towards $N with a massive cleave!", ch, NULL, victim, TO_CHAR);
    act("$n raises $s axe high above $s head and swings it towards you with a massive cleave!", ch, NULL, victim, TO_VICT);
    act("$n raises $s axe high above $s head and swings it towards $N with a massive cleave!", ch, NULL, victim, TO_NOTVICT);
 
    fight = 0;
    if ( victim->position <= POS_SLEEPING )
        fight = 1;
    check_killer( ch, victim );
    WAIT_STATE( ch, skill_table[gsn_cleave].beats );
    if ( !IS_AWAKE(victim)
    ||   IS_NPC(ch)
    ||   number_percent( ) < ch->pcdata->learned[gsn_cleave] )
    {
        check_improve(ch,gsn_cleave,TRUE,1);
        one_hit( ch, victim, gsn_cleave,FALSE );

	  /* Chance for a second cleave, provided they have weapon with right Specialty */
        if ( (obj->value[4] == WEAPON_MIGHTY_CLEAVING )
        && (ch->pcdata->learned[gsn_cleave] > 90))
	  {
	   act("You get a SECOND {RMighty Cleave{x in!", ch, NULL, NULL, TO_CHAR);
         one_hit( ch, victim, gsn_cleave,FALSE );
	  }
    if (!IS_NPC(victim) && fight != 1)
    {
        if (!can_see(victim,ch) )
            do_yell(victim, "Ack! Someone just sliced me into pieces!");
        else
            {
            if (IS_NPC(ch))
                sprintf(buf, "Ack! %s just sliced me into pieces!",
                    ch->short_descr);
            else
                sprintf(buf, "Ack! %s just sliced me into pieces!",
               (is_affected(ch,gsn_doppelganger) && (!IS_IMMORTAL(victim)))? ch->doppel->name :
               ((is_affected(ch,gsn_treeform)) && (!IS_IMMORTAL(victim)))? "A mighty oak tree" :
	         ch->name);
            do_yell( victim, buf );
        }
    }
    }
    else
    {
    if (!IS_NPC(victim) && fight != 1)
    {
        if (!can_see(victim,ch) ) 
            do_yell(victim, "Help! Someone tried to cleave me in two!");
        else
            {
            if (IS_NPC(ch))
                sprintf(buf, "Die, %s, you butchering fool!", ch->short_descr);
            else
                sprintf(buf, "Die, %s, you butchering fool!", 
               (is_affected(ch,gsn_doppelganger) && (!IS_IMMORTAL(victim)))? ch->doppel->name :
               ((is_affected(ch,gsn_treeform)) && (!IS_IMMORTAL(victim)))? "A mighty oak tree" :
	         ch->name);
            do_yell( victim, buf );
        }
    }
    check_improve(ch,gsn_cleave,FALSE,1);
    damage( ch, victim, 0, gsn_cleave,DAM_NONE,TRUE );
    }
    return;
}

void do_trammel( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *wield;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;
    int fight;
    one_argument(argument,arg);
    wield = get_eq_char( ch, WEAR_WIELD );

    if ( (chance = get_skill(ch,gsn_trammel)) == 0
    ||   (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_TRIP))
    ||   (!IS_NPC(ch) 
          && ch->level < skill_table[gsn_trammel].skill_level[ch->class]))
    {
        send_to_char("Hmm?\n\r",ch);
        return;
    }

    if ( wield == NULL)
    {
	send_to_char("You must be wielding a whip in order to trammel your opponant's feet.\n\r",ch);
	return;
    }

    if ( wield->value[0] != WEAPON_WHIP )
    {
	send_to_char("You must be wielding a whip in order to trammel your opponant's feet.\n\r",ch);
	return;
    }

    if ( IS_AFFECTED2(ch, AFF_PARALYSIS) 
    ||   IS_AFFECTED3(ch, AFF_HOLDPERSON))
    {
      send_to_char("You can't move at all, there's no way you can do that.\n\r",ch);
      return;
    }

    if (arg[0] == '\0')
    {
        victim = ch->fighting;
        if (victim == NULL)
        {
            send_to_char("But you aren't fighting anyone!\n\r",ch);
            return;
        }
    }
    else if ((victim = get_char_room(ch, NULL, arg)) == NULL)
    {
        send_to_char("They aren't here.\n\r",ch);
        return;
    }

    if (is_safe(ch,victim))
        return;

    if ( IS_NPC(victim)
    && victim->fighting != NULL
    && !is_same_group(ch,victim->fighting))
    {
        send_to_char("Kill stealing is not permitted.\n\r",ch);
        return;
    }
    
    if (victim->position < POS_FIGHTING)
    {
        act("$N is already down.",ch,NULL,victim,TO_CHAR);
        return;
    }

    if (victim == ch)
    {
        send_to_char("You foolishly trammel your own feet!\n\r",ch);
        WAIT_STATE(ch,2 * skill_table[gsn_trammel].beats);
        act("$n foolishly trammels $s own feet!",ch,NULL,NULL,TO_ROOM);
        return;
    }

    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
    {
        act("$N is your beloved master.",ch,NULL,victim,TO_CHAR);
        return;
    }

    if ( ch->fight_pos == FIGHT_REAR )
    {
        send_to_char("You aren't close enough to trammel!\n\r",ch);
        return;
    }

    if( !IS_NPC(ch) && (ch->pcdata->learned[gsn_whip] < 90 ))
    {
        send_to_char( "You are not yet skilled enough with whips to trammel anyone.\n\r", ch );
        return;
    }

    /* modifiers */
    /* size */
    if (ch->size < victim->size)
        chance -= (ch->size - victim->size) * 10;  /* bigger = easier to trammel */
    /* dex */
    chance += get_curr_stat(ch,STAT_DEX);
    chance -= get_curr_stat(victim,STAT_DEX) * 3 / 2;
    /* speed */
    if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE))
        chance += 10;
    if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,AFF_HASTE))
        chance -= 20;
    if (IS_AFFECTED(ch, AFF_SLOW))
        chance -= 20;
    if (IS_AFFECTED(victim, AFF_SLOW))
        chance += 20;
    /* level */
    chance += (ch->level - victim->level) * 2;
    fight = 0;
    if (ch->fighting == NULL)
        fight = 1;
    /* now the attack */
    if (number_percent() < chance)
    {
        act("$n lashes out and trammels your feet with $s whip, causing you to loose your balance!",ch,NULL,victim,TO_VICT);
        act("You lash out with your whip and trammel $N's feet, making $M stumble!",ch,NULL,victim,TO_CHAR);
        act("$n lashes out and trammels $N's feet with $s whip, causing $N to stumble.",ch,NULL,victim,TO_NOTVICT);
        check_improve(ch,gsn_trammel,TRUE,1);
        WAIT_STATE(victim,2 * PULSE_VIOLENCE);
        WAIT_STATE(ch,skill_table[gsn_trammel].beats);
        victim->position = POS_RESTING;
        damage(ch,victim,number_range(2,100),gsn_trammel,
            DAM_BASH, TRUE );
    if (fight == 1 && !IS_NPC(victim) && victim->position > POS_SLEEPING)
    {
        if (!can_see(victim,ch) ) 
            do_yell(victim, "Help! Someone just yanked my feet out from under me!");
        else
           {
            if (IS_NPC(ch))
                sprintf(buf, "Help! %s just yanked my feet out from under me!", 
                ch->short_descr);
            else
                sprintf(buf, "Help! %s just yanked my feet out from under me!",
               (is_affected(ch,gsn_doppelganger) && (!IS_IMMORTAL(victim)))? ch->doppel->name :
               ((is_affected(ch,gsn_treeform)) && (!IS_IMMORTAL(victim)))? "A mighty oak tree" :
	         ch->name);
            do_yell( victim, buf );
        }
    }    
    }
    else
    {
    if (ch->fighting == NULL && !IS_NPC(victim))
    {
        if (!can_see(victim,ch) ) 
            do_yell(victim, "Hey! Someone tried to trammel my feet with a whip!");
        else
            {
            if (IS_NPC(ch))
            sprintf(buf, "Hey! %s tried to trammel my feet with a whip!", ch->short_descr);
            else
                sprintf(buf, "Hey! %s tried to trammel my feet with a whip!", 
               (is_affected(ch,gsn_doppelganger) && (!IS_IMMORTAL(victim)))? ch->doppel->name :
               ((is_affected(ch,gsn_treeform)) && (!IS_IMMORTAL(victim)))? "A mighty oak tree" :
	         ch->name);
            do_yell( victim,buf );
        }
    }
        damage(ch,victim,0,gsn_trammel,DAM_BASH, TRUE);
        WAIT_STATE(ch,skill_table[gsn_trammel].beats*2/3);
        check_improve(ch,gsn_trammel,FALSE,1);
    } 
}

void do_sweep( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *wield;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;
    int fight;
    one_argument(argument,arg);
    wield = get_eq_char( ch, WEAR_WIELD );

    if ( (chance = get_skill(ch,gsn_sweep)) == 0
    ||   (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_TRIP))
    ||   (!IS_NPC(ch) 
          && ch->level < skill_table[gsn_sweep].skill_level[ch->class]))
    {
        send_to_char("Hmm?\n\r",ch);
        return;
    }

    if (ch->riding)
    {
      send_to_char("You can't do that while mounted!.\n\r",ch);
      return;
    }

    if ((wield == NULL) || ( wield->value[0] != WEAPON_SPEAR ))
    {
	send_to_char("You must be wielding a spear in order to execute a sweep attack.\n\r",ch);
	return;
    }

    if ( IS_AFFECTED2(ch, AFF_PARALYSIS) 
    ||   IS_AFFECTED3(ch, AFF_HOLDPERSON))
    {
      send_to_char("You can't move at all, there's no way you can do that.\n\r",ch);
      return;
    }

    if (arg[0] == '\0')
    {
        victim = ch->fighting;
        if (victim == NULL)
        {
            send_to_char("But you aren't fighting anyone!\n\r",ch);
            return;
        }
    }

    else if ((victim = get_char_room(ch, NULL, arg)) == NULL)
    {
        send_to_char("They aren't here.\n\r",ch);
        return;
    }

    if (victim->riding && !IS_NPC(victim))
    {
      act("$S feet are not on the ground.\n\r",ch,NULL,victim,TO_CHAR);
      return;
    }

    if (is_safe(ch,victim))
        return;

    if ( IS_NPC(victim)
    && victim->fighting != NULL
    && !is_same_group(ch,victim->fighting))
    {
        send_to_char("Kill stealing is not permitted.\n\r",ch);
        return;
    }
    
    if (victim->position < POS_FIGHTING)
    {
        act("$N is already down.",ch,NULL,victim,TO_CHAR);
        return;
    }

    if (IS_AFFECTED(victim,AFF_FLYING) || (victim->riding && !IS_NPC(victim)))   
    {
        act("$S feet aren't on the ground.",ch,NULL,victim,TO_CHAR);
        return;
    }

    if (victim == ch)
    {
        send_to_char("You foolishly sweep attack your own feet!\n\r",ch);
        WAIT_STATE(ch,2 * skill_table[gsn_sweep].beats);
        act("$n foolishly sweep attacks $s own feet!",ch,NULL,NULL,TO_ROOM);
        return;
    }

    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
    {
        act("$N is your beloved master.",ch,NULL,victim,TO_CHAR);
        return;
    }

    if ( ch->fight_pos > FIGHT_FRONT )
    {
        send_to_char("You aren't close enough to execute a sweep attack!\n\r",ch);
        return;
    }

    if( !IS_NPC(ch) && (ch->pcdata->learned[gsn_spear] < 90 ))
    {
        send_to_char( "You are not yet skilled enough with spears to sweep anyone.\n\r", ch );
        return;
    }

    /* modifiers */

    /* size */
    if (ch->size < victim->size)
        chance += (ch->size - victim->size) * 10;  /* bigger = harder to sweep */

    /* dex */
    chance += get_curr_stat(ch,STAT_DEX);
    chance -= get_curr_stat(victim,STAT_DEX) * 3 / 2;

    /* speed */
    if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE))
        chance += 10;
    if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,AFF_HASTE))
        chance -= 20;
    if (IS_AFFECTED(ch, AFF_SLOW))
        chance -= 20;
    if (IS_AFFECTED(victim, AFF_SLOW))
        chance += 20;

    /* level */
    chance += (ch->level - victim->level) * 2;
    fight = 0;
    if (ch->fighting == NULL)
        fight = 1;

    /* now the attack */
    if (number_percent() < chance)
    {
        act("$n swings $s staff in a low arc, sweeping your feet out from under you!",ch,NULL,victim,TO_VICT);
        act("You swing your staff in a low sweeping arc, causing $N to stumble!",ch,NULL,victim,TO_CHAR);
        act("$n swings out $s staff in a low sweeping arc, causing $N to stumble.",ch,NULL,victim,TO_NOTVICT);
        check_improve(ch,gsn_sweep,TRUE,1);
        WAIT_STATE(victim,2 * PULSE_VIOLENCE);
        WAIT_STATE(ch,skill_table[gsn_sweep].beats);
        victim->position = POS_RESTING;
        damage(ch,victim,number_range(8,100),gsn_sweep,
            DAM_BASH, TRUE );
    if (fight == 1 && !IS_NPC(victim) && victim->position > POS_SLEEPING)
    {
        if (!can_see(victim,ch) ) 
            do_yell(victim, "Help! Someone just swept my feet out from under me!");
        else
           {
            if (IS_NPC(ch))
                sprintf(buf, "Help! %s just swept my feet out from under me!", 
                ch->short_descr);
            else
                sprintf(buf, "Help! %s just swept my feet out from under me!",
               (is_affected(ch,gsn_doppelganger) && (!IS_IMMORTAL(victim)))? ch->doppel->name :
               ((is_affected(ch,gsn_treeform)) && (!IS_IMMORTAL(victim)))? "A mighty oak tree" :
	         ch->name);
            do_yell( victim, buf );
        }
    }    
    }
    else
    {
    if (ch->fighting == NULL && !IS_NPC(victim))
    {
        if (!can_see(victim,ch) ) 
            do_yell(victim, "Hey! Someone tried to sweep my feet out from under me!");
        else
            {
            if (IS_NPC(ch))
                sprintf(buf, "Hey! %s tried to sweep my feet out from under me!", ch->short_descr);
            else
                sprintf(buf, "Hey! %s tried to sweep my feet out from under me!", 
               (is_affected(ch,gsn_doppelganger) && (!IS_IMMORTAL(victim)))? ch->doppel->name :
               ((is_affected(ch,gsn_treeform)) && (!IS_IMMORTAL(victim)))? "A mighty oak tree" :
	         ch->name);
            do_yell( victim,buf );
        }
    }
        damage(ch,victim,0,gsn_sweep,DAM_BASH, TRUE);
        WAIT_STATE(ch,skill_table[gsn_sweep].beats*2/3);
        check_improve(ch,gsn_sweep,FALSE,1);
     }
}

void do_flank_attack( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    int chance, dam_type;
    
    /* make sure they're fighting */
    if( (victim = ch->fighting) == NULL )
    {
        send_to_char("You are not fighting anyone.\n\r", ch );
        return;
    }

    /* make sure they're not taking damage though */
    if( victim->fighting == ch )
    {
        send_to_char("The enemy onslaught is too aggressive right now.\n\r",ch );
        return;
    }
    
    /* Make sure they're in the rear line */
    if( ch->fight_pos == FIGHT_FRONT )
    {
        send_to_char("You cannot execute a flank attack while defending a frontal assault.\n\r", ch);
        return;
    }

    else if (ch->fight_pos == FIGHT_REAR)
    {
	send_to_char("There is no way you can evaluate and break through the enemy defenses from way back here.\r\n",ch);
	return;
    }
    
    /* make sure they have a weapon */
    if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL)
    {
        send_to_char( "You need to wield a weapon to execute a flank attack.\n\r", ch );
        return;
    }

    if ( IS_AFFECTED2(ch, AFF_PARALYSIS) 
    ||   IS_AFFECTED3(ch, AFF_HOLDPERSON))
    {
      send_to_char("You can't move at all, there's no way you can do that.\n\r",ch);
      return;
    }

    dam_type = attack_table[obj->value[3]].damage;
    if ( obj->value[0]!=WEAPON_SPEAR
    && obj->value[0] != WEAPON_POLEARM )
    {
        send_to_char ("You must be wielding a long-shafted weapon in order to flank attack.\n\r",ch);
        return;
    }

    /* see if they have a shot at it */
    if ( IS_NPC(ch)
    ||   (chance = get_skill(ch,gsn_flank_attack)) == 0
    ||   ch->level < skill_table[gsn_flank_attack].skill_level[ch->class])
    {
        send_to_char("Flank attack?  What's that?\n\r",ch);
        return;
    }

    if (victim->position < POS_FIGHTING)
    {
        act("$N is already down.",ch,NULL,victim,TO_CHAR);
        return;
    }

    /* modifiers */

    /* dex */
    chance += get_curr_stat(ch,STAT_DEX);
    chance -= get_curr_stat(victim,STAT_DEX) * 3 / 2;

    /* speed */
    if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE))
        chance += 10;
    if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,AFF_HASTE))
        chance -= 20;

    /* level */
    chance += (ch->level - victim->level) * 2;

    /* now the attack */
    act("$n quickly evaluates $N's defenses, then maneuvers in to execute a cunning flank attack.", ch, NULL, victim, TO_ROOM);
    act("You quickly evaluate $N's defenses as you move in to execute a cunning flank attack.", ch, NULL, victim, TO_CHAR);
    if (number_percent() < chance)
    {
        check_improve(ch,gsn_flank_attack,TRUE,1);
        WAIT_STATE(ch,skill_table[gsn_flank_attack].beats*3/2);
        one_hit(ch, victim, gsn_flank_attack,FALSE );
    }
    else
    {
        damage(ch,victim,0,gsn_flank_attack,DAM_PIERCE,TRUE);
        WAIT_STATE(ch,skill_table[gsn_flank_attack].beats*5/4);
        check_improve(ch,gsn_flank_attack,FALSE,1);
    } 
}

void do_squire_call( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *gch;
  CHAR_DATA *squire;
  AFFECT_DATA af;
  int i;
  
  if ( IS_NPC(ch)
  || ch->level < skill_table[gsn_squire_call].skill_level[ch->class] )
  {
      send_to_char("The squires won't answer you.\n\r", ch);
      return;
  }

  if( !IS_NPC(ch) && ch->pcdata->learned[gsn_squire_call] < 1 )
  {
      send_to_char("The squires won't answer you.\n\r", ch);
      return;
  }

  if (is_affected(ch, gsn_squire_call))
  {
      send_to_char("Your voice is too hoarse to summon squires.\n\r", ch);
      return;
  }

  if (ch->mana < skill_table[gsn_squire_call].min_mana)
  {
      send_to_char("You can't concentrate enough.\n\r", ch);
      return;
  }

  for (gch = char_list; gch != NULL; gch = gch->next)
  {
      if (IS_NPC(gch)
	&& IS_AFFECTED(gch,AFF_CHARM)
	&& gch->master == ch
	&& gch->pIndexData->vnum == MOB_VNUM_SQUIRE)
	{
	  send_to_char("What's wrong with the squire you've got?",ch);
	  return;
	}
  }

  send_to_char("You bark out an order, summoning a squire.\n\r",ch);
  act("$n barks out an order, summoning a squire.",ch,NULL,NULL,TO_ROOM);

  if (number_percent() > get_skill(ch, gsn_squire_call))
  {
      send_to_char("The squire fails to respond to your summons.\n\r", ch);
      check_improve(ch, gsn_squire_call, FALSE, 2);
      ch->mana -= skill_table[gsn_squire_call].min_mana / 2;
      return;
  }

  ch->mana -= skill_table[gsn_squire_call].min_mana;

  squire = create_mobile( get_mob_index(MOB_VNUM_SQUIRE) );

  for (i=0;i < MAX_STATS; i++)
  {
      squire->perm_stat[i] = UMIN(25,2 * ch->perm_stat[i]);
  }

  squire->max_hit = IS_NPC(ch)? ch->max_hit : ch->pcdata->perm_hit;
  squire->hit = squire->max_hit;
  squire->max_mana = IS_NPC(ch)? ch->max_mana : ch->pcdata->perm_mana;
  squire->mana = squire->max_mana;
  squire->alignment = ch->alignment;
  squire->level = UMIN(70,ch->level);
  for (i=0; i < 3; i++)
    squire->armor[i] = interpolate(squire->level,100,-100);
  squire->armor[3] = interpolate(squire->level,100,0);
  squire->gold = 0;
    
  SET_BIT(squire->affected_by, AFF_CHARM);
  squire->master = ch;
  squire->leader = ch;

  char_to_room(squire,ch->in_room);
  send_to_char("A young squire runs in and salutes you loyally.\n\r",ch);
  act("A young squire runs in and salutes $n loyally.",ch,NULL,NULL,TO_ROOM);

  af.where			= TO_AFFECTS;
  af.type               = gsn_squire_call;
  af.level              = ch->level; 
  af.duration           = 24;
  af.bitvector          = 0;
  af.modifier           = 0;
  af.location           = APPLY_NONE;
  affect_to_char(ch, &af); 

  check_improve(ch, gsn_squire_call, TRUE, 1);
  return;
}

void do_surge( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    
    if (IS_NPC(ch))
    {
         return;
    }

    if ( !IS_NPC(ch)
    &&   ch->level < skill_table[gsn_surge].skill_level[ch->class] )
    {
        send_to_char("You are unable to stir the mana in others.\n\r", ch );
        return;
    }

    if (ch->pcdata->learned[gsn_surge] < 1 )
    {
        send_to_char("You are unable to stir the mana in others.\n\r", ch );
        return;
    }

    if ( ( victim = ch->fighting ) == NULL )
    {
        send_to_char( "You aren't fighting anyone.\n\r", ch );
        return;
    }
    if ( ch->fight_pos > FIGHT_MIDDLE )
    {
       send_to_char( "You aren't near enough to inspire a mana surge!\n\r",ch);
       return;
    }
   
    if (victim->mana <= 0)
    {
       send_to_char( "There is not enough mana left to surge!\n\r",ch);
       return;
    }

    WAIT_STATE( ch, skill_table[gsn_surge].beats );
    if ( IS_NPC(ch) || number_percent( ) < ch->pcdata->learned[gsn_surge] )
    {
        damage( ch, victim, number_range( 1, ch->level ), gsn_surge,DAM_ENERGY,TRUE);
        check_improve(ch,gsn_surge,TRUE,1);
    }
    else
    {
        damage( ch, victim, 0, gsn_surge,DAM_ENERGY,TRUE );
        check_improve(ch,gsn_surge,FALSE,1);
    }

    victim->mana -= (number_range( 1, ch->level )/2) + 1;
    if (victim->mana < 0)
	victim->mana = 0;

    return;
}

void do_endure(CHAR_DATA *ch, char *arg)
{
  AFFECT_DATA af;

  if (IS_NPC(ch))
  {
      send_to_char("You have no endurance whatsoever.\n\r",ch);
      return;
  }

  if ( ch->level < skill_table[gsn_endure].skill_level[ch->class]
  || ch->pcdata->learned[gsn_endure] <= 1 )
  {
      send_to_char("You lack the concentration.\n\r",ch);
      return;
  }
       
  if (is_affected(ch,gsn_endure))
  {
      send_to_char("You cannot endure any more concentration.\n\r",ch);
      return;
  }
  
  WAIT_STATE( ch, skill_table[gsn_endure].beats );

  af.where		= TO_AFFECTS;
  af.type 		= gsn_endure;
  af.level 		= ch->level;
  af.duration 	= ch->level / 4;
  af.location 	= APPLY_SAVING_SPELL;
  af.modifier 	= -1 * (ch->pcdata->learned[gsn_endure] / 10); 
  af.bitvector 	= 0;
  affect_to_char(ch,&af);

  send_to_char("You prepare yourself for magical encounters.\n\r",ch);
  act("$n concentrates for a moment, then resumes $s position.",ch,NULL,NULL,TO_ROOM);
  check_improve(ch,gsn_endure,TRUE,1);
}

void do_serenade(CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *pChar;
  AFFECT_DATA af;
  CHAR_DATA *gch;
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  int chance =0;

    one_argument( argument, arg );

    if (IS_NPC(ch))
     return;

    if ( arg[0] == '\0' )
    {
        send_to_char( "Serenade whom?\n\r", ch );
        return;
    }

    if ( !IS_NPC(ch) && ch->pcdata->learned[gsn_serenade] < 1 )
    {
        send_to_char("You don't know that skill.\n\r",ch);
        return;
    }

    if ( ( pChar = get_char_room( ch, NULL, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( pChar == ch )
    {
        send_to_char( "You sing to yourself.\n\r", ch );
        return;
    }

    if (ch->sex == pChar->sex)
    {
     send_to_char("That person is more interested in suitors of the opposite gender.\n\r",ch);
     return;
    }
 
    if ( IS_AFFECTED(ch,AFF_CHARM))
    {
        send_to_char("You master does not agree with your will!",ch);
        return;
    }
   
    if ( is_safe(ch,pChar))
        return;

    if (pChar->level >= ch->level)
    {
     send_to_char("That person does not seem to be affected by your serenade.\n\r",ch);
     return;
    }

    chance = get_skill(ch,gsn_serenade)  - 25 + ch->perm_stat[STAT_CHR];

       if ( IS_AWAKE(pChar)
       && (!IS_IMMORTAL(pChar))
       && (!is_safe(ch,pChar)))        {
       if (saves_spell(ch->level,pChar, DAM_SOUND) 
       || (number_percent() > chance + ch->pcdata->learned[gsn_serenade])) 
       { 
        act("$n stumbles over the lyrics of a romantic tune.",ch,ch,NULL,TO_ROOM);
        act("You begin your serenade, only to stumble over the lyrics.",ch,NULL,NULL,TO_ROOM); 
        check_improve(ch,gsn_serenade,FALSE,1);

       if (!IS_NPC(pChar) && pChar->position != POS_FIGHTING)
        {
           if (!can_see(pChar,ch))
              do_yell(pChar,"Help! Someone is attacking me!");
           else
           {
              if (IS_NPC(ch))
                 sprintf(buf, "Help! %s is attacking me!",ch->short_descr);
              else
                 sprintf(buf,"Help! %s is attacking me!",
                 (is_affected(ch,gsn_doppelganger) && (!IS_IMMORTAL(pChar)))? ch->doppel->name :
                 (is_affected(ch,gsn_treeform) && (!IS_IMMORTAL(pChar)))? "A mighty oak tree" :
                 ch->name);
              do_yell(pChar,buf);
           }
         multi_hit(ch,pChar,TYPE_UNDEFINED);
         return; 
        } 
      }
     for (gch = char_list; gch != NULL; gch = gch->next)
     {
         if (is_affected(gch,gsn_serenade)
         &&  is_same_group(gch,ch))
         {
         send_to_char("Now, now, one should not grow greedy in affairs on the heart..\n\r",ch); 
         return;
         }
        
         if ((gch->sex == pChar->sex)
         &&   is_same_group(gch,ch))
         {
          act("In a jealous rage, $n is attacked by those who would have loved $m.",
            ch,NULL,NULL,TO_ROOM);
          act("In a jealous rage, you are attacked by those you would have loved you.",
            ch,NULL,NULL,TO_CHAR);
          if (IS_NPC(gch)) 
             multi_hit(ch,gch,TYPE_UNDEFINED);
          else 
             stop_follower(gch);
          if (IS_NPC(pChar))
              multi_hit(ch,pChar,TYPE_UNDEFINED);
          else
             stop_follower(pChar);
          return; 
         } 
      }

      act("$n beautifully sings a lilting serenade.",ch,NULL,NULL,TO_ROOM);
      act("You beautifully sing a lilting serenade.",ch,NULL,NULL,TO_CHAR);
      check_improve(ch,gsn_serenade,TRUE,1);
      if (pChar->master)
         stop_follower(pChar);
      add_follower(pChar,ch);
      pChar->leader = ch;

	if( !IS_NPC(pChar) && IS_SET(pChar->act, PLR_NOFOLLOW) )
		REMOVE_BIT(pChar->act, PLR_NOFOLLOW);

	af.where		= TO_AFFECTS;
      af.type       	= gsn_serenade;
      af.level      	= ch->level;
      af.modifier   	= 0;
      af.location 	= APPLY_NONE;
      af.duration 	= ch->level / 4;
      af.bitvector 	= AFF_CHARM;
      affect_to_char(pChar,&af);
      return; 
    }
send_to_char("Your lovely serenade is coldly ignored. \n\r",ch);
return;
}

void do_taunt( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *pChar;
  CHAR_DATA *pChar_next;
  AFFECT_DATA af;
  char buf[MAX_STRING_LENGTH];
  int chance;
  int fail_taunt = 0;

  if (IS_NPC(ch))
    return;

 if (ch->mana < 25)
 {
    send_to_char("You are not in the right state of mind to taunt your foes.\n\r",ch);
    return;
 }

    if ( IS_AFFECTED2(ch, AFF_PARALYSIS) 
    ||   IS_AFFECTED3(ch, AFF_HOLDPERSON))
    {
      send_to_char("You can't move at all, there's no way you can do that.\n\r",ch);
      return;
    }
 
 if ( (chance = get_skill(ch,gsn_taunt) == 0 )
 || (  ch->level < skill_table[gsn_taunt].skill_level[ch->class]))
 {
      send_to_char("You aren't witty enough to taunt your foes.\n\r",ch);
      WAIT_STATE( ch, skill_table[gsn_taunt].beats );
      return;
 }

 if (number_percent() > chance + ch->pcdata->learned[gsn_taunt])
  {
     act( "$n attempts to say something insulting, but just can't find the right words.\n\r",ch,ch,NULL,TO_ROOM);
     act( "You try to think up a scathing insult, but nothing comes to mind.\n\r",ch,NULL,NULL,TO_CHAR);
     ch->mana = ch->mana - 12;
     check_improve(ch,gsn_taunt,FALSE,1);
     fail_taunt = 1;
  }

 if (fail_taunt != 1)
  {
    act( "You cleverly deliver a harsh insult to anger your foes.",ch,NULL,NULL,TO_CHAR );
    act( "$n cleverly delivers a harsh and arrogant insult.",ch,NULL,NULL,TO_ROOM );
    check_improve(ch,gsn_taunt,TRUE,1); 
  } 

 pChar_next = NULL;
 for ( pChar = ch->in_room->people; pChar; pChar = pChar_next) 
   {
     pChar_next = pChar->next_in_room;
        if   (IS_AWAKE(pChar)
        &&   (!IS_IMMORTAL(pChar))
        &&   (!is_safe(ch,pChar))
        &&   ( !is_same_group( pChar, ch))
	  &&   (!is_affected(pChar, AFF_INVUL)))
         {

             if (!IS_NPC(pChar) && pChar->position != POS_FIGHTING)
              {
                if (!can_see(pChar,ch))
                    do_yell(pChar,"Help! Someone is attacking me!");
                else
                {
                  if (IS_NPC(ch))
                    sprintf(buf, "Help! %s is attacking me!",ch->short_descr);
                  else
                    sprintf(buf,"Help! %s is attacking me!",
                    (is_affected(ch,gsn_doppelganger) && (!IS_IMMORTAL(pChar)))? ch->doppel->name : 
                    (is_affected(ch,gsn_treeform) && (!IS_IMMORTAL(pChar)))? "A mighty oak tree" :
		        ch->name);
                    do_yell(pChar,buf);
                   }
               } 
 
     if (saves_spell(ch->level,pChar, DAM_MENTAL)
     || fail_taunt == 1 
     || is_affected(pChar,gsn_taunt))   
        continue;

	af.where		= TO_AFFECTS;
      af.type       	= gsn_taunt;
      af.level      	= ch->level;
      af.modifier   	= -1;
      if (ch->level > 29 )
          af.modifier 	= -2;
      if (ch->level > 40 )
          af.modifier 	= -3;  
      af.location 	= APPLY_HITROLL;
      af.duration 	= 3;
      af.bitvector 	= 0;
      affect_to_char(pChar,&af);
      af.location 	=  APPLY_DAMROLL;
      affect_to_char(pChar,&af);
      multi_hit( ch, pChar, TYPE_UNDEFINED);
     }
  continue; 
 }
     ch->mana = ch->mana - 25;
return;
}

void do_bravado( CHAR_DATA *ch, char *argument)
{

 CHAR_DATA *pChar;
 CHAR_DATA *pChar_next;
 AFFECT_DATA af;

 if (IS_NPC(ch))
  return;

 if (ch->level < skill_table[gsn_bravado].skill_level[ch->class])
 {
      send_to_char( "Huh?\n\r",ch );
      return;
 }

 if ( !IS_NPC(ch) && ch->pcdata->learned[gsn_bravado] < 1 )
 {
      send_to_char( "Huh?\n\r",ch );
      return;
 }

 if (IS_AFFECTED2(ch,AFF_BRAVADO))
 {
     send_to_char("You and your friends are already feeling courageous.\n\r",ch);
     return;
 }
 
 if ( number_percent( ) < ch->pcdata->learned[gsn_bravado])
    check_improve(ch,gsn_bravado,TRUE,1);
 else
 {
    check_improve(ch,gsn_bravado,FALSE,1);
    act("$n tries to increase morale, but fails.",ch,NULL,NULL,TO_ROOM);
    act("You try to increase the morale of your friends, but fail.",ch,NULL,NULL,TO_CHAR);
    return;
  } 

 pChar_next = NULL;
 for ( pChar = ch->in_room->people; pChar; pChar = pChar_next)
 {
  pChar_next = pChar->next_in_room;

    if (is_same_group(ch,pChar)
    && !IS_AFFECTED2(pChar,AFF_BRAVADO))
    {
	 af.where		= TO_AFFECTS2;
       af.type     	= gsn_bravado;
       af.level    	= ch->level;
       af.duration 	= ch->level/3;
       af.location 	= APPLY_HITROLL;
       af.modifier  	= 1;
    
      if ( ch->level > 19 )
          af.modifier  = 2;
       if ( ch->level > 29 )
          af.modifier  = 3;
       if ( ch->level > 39 )
          af.modifier = 4;
       if ( ch->level > 49 )
          af.modifier = 5;
       af.bitvector 	= AFF_BRAVADO;
       affect_to_char(pChar,&af);
       af.location 	= APPLY_DAMROLL;
       affect_to_char(pChar,&af); 
   
       if (pChar == ch)
        send_to_char("You raise your voice in a song of great deeds and ancient battles.\n\r",ch);
       else
        send_to_char("Your morale is raised by a battle-hymn.\n\r",pChar);
     } 
  }

act("$n raises $s voice in a song of great deeds and ancient battles.",ch,NULL,NULL,TO_ROOM);
return;
}

void do_pacify( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *pChar; 
  CHAR_DATA *pChar_next;
  int chance;

  if ( (chance = get_skill(ch,gsn_pacify)) == 0
  || (!IS_NPC(ch)
  &&   ch->level < skill_table[gsn_pacify].skill_level[ch->class]))
  {
      send_to_char("You make some grunting noise.\n\r",ch);
      act ( "$n makes some grunting noises.",ch,NULL,NULL,TO_ROOM); 
      return;
  }

  if (ch->mana < 25)
  {
    send_to_char("You do not have the strength to sing that song.\n\r",ch);
    return;
  }

  pChar_next = NULL; 

  for ( pChar = ch->in_room->people; pChar; pChar = pChar_next) 
  {
    pChar_next = pChar->next_in_room;
    if  (IS_AWAKE(pChar)
    &&  (IS_SET((pChar)->act,ACT_AGGRESSIVE))
    &&   IS_NPC(pChar)) 
    {
      /* harder for a person who is fighting to hear that song */
      if (pChar->position == POS_FIGHTING)       
      chance -= 10;   

      WAIT_STATE( ch, skill_table[gsn_pacify].beats );

      if (number_percent() > chance + ch->pcdata->learned[gsn_pacify])
      {
        act( "$n makes a croaking noise.\n\r",ch,NULL,NULL,TO_ROOM);
        act( "You make a croaking noise.\n\r",ch,NULL,NULL,TO_CHAR);
        ch->mana = ch->mana - 12;
        check_improve(ch,gsn_pacify,FALSE,1);
        return;
      }
      if  (saves_spell(ch->level,pChar, DAM_SOUND))
      {
        do_say( pChar,"Eeek! You sing terribly!");
        ch->mana = ch->mana - 25;
        check_improve(ch,gsn_pacify,TRUE,1);
        return;
      } 

      REMOVE_BIT(pChar->act,ACT_AGGRESSIVE);
      act( "You sing a soothing song.",ch,NULL,NULL,TO_CHAR );
      act( "$n sings a soothing song.",ch,NULL,NULL,TO_ROOM );
      ch->mana = ch->mana - 25;
      check_improve(ch,gsn_pacify,TRUE,1); 
      return; 
    } 
  }
  act( "You sing a soothing song, but nobody hears it.",ch,NULL,NULL,TO_CHAR );
  ch->mana = ch->mana - 25;
  return;
}

void do_lull ( CHAR_DATA *ch, char *argument )
{
 CHAR_DATA *victim; 
 AFFECT_DATA af;
 char arg[MAX_INPUT_LENGTH];
 char buf[MAX_STRING_LENGTH];
 int chance;
 one_argument (argument, arg);

 if ( arg[0] == '\0' )
 {
           send_to_char( "Lull whom?\n\r", ch );
           return;
 }

 if ((chance = get_skill(ch,gsn_lull)) == 0
 || (!IS_NPC(ch)
 && ch->level < skill_table[gsn_lull].skill_level[ch->class]))
 {
    send_to_char("You sing a lullaby, far too loudly to lull anyone to sleep.\n\r",ch);
    act ( "$n loudly sings a lullaby.",ch,NULL,NULL,TO_ROOM); 
    return;
 }

 if (ch->mana < 25)
 {
    send_to_char("You must gain more mental focus before you can attempt your lullaby.\n\r",ch);
    return;
 }

 if ((victim = get_char_room (ch, NULL, arg)) == NULL)
 {
    send_to_char("They aren't here.\n\r", ch);
    return;
 }

 if (victim == NULL)
 {
    send_to_char ("No one here fits that description.\n\r", ch);
    return;
 }

 if (victim == ch)
 {
    send_to_char("You try hard, but can't sing yourself to sleep.\n\r", ch);
    return;
 }

 if (IS_AFFECTED (victim, AFF_SLEEP) || is_affected (ch, gsn_lull))
 {
    send_to_char ("That person cannot hear you sing while sleeping.\n\r",ch);
    return;
 }

 if( IS_AFFECTED2(victim,AFF_SLEEPCURSE) )
 {
    send_to_char("Their mind has been cursed to prevent sleeping.\n\r",ch );
    return;
 }

 if (IS_SET(ch->in_room->room_flags, ROOM_SAFE)
 ||  IS_SET(victim->in_room->room_flags, ROOM_SAFE))
 {
    send_to_char("You can not LULL anyone to sleep here.\n\r",ch );
    return;
 }

 if (IS_AFFECTED(ch, AFF_CHARM))
 {
    send_to_char("Your master doesn't agree with your will!",ch);
    return;
 }

 if (victim->position == POS_FIGHTING)       
          chance -= 25;
 if (victim->race == ch->race)
	    chance += 10;
 if ((ch->perm_stat[STAT_CHR]+ch->mod_stat[STAT_CHR])>18)
	    chance += 15;
 if ((ch->perm_stat[STAT_CHR]+ch->mod_stat[STAT_CHR])<10)
	    chance -= 15;
 if (victim->position == POS_FIGHTING)       
          chance -= 25;
 if (!IS_NPC(victim))
 {
    if (victim->pcdata->condition[COND_FULL] < 10)
	    chance -= 10;
    if (victim->pcdata->condition[COND_THIRST] < 10)
	    chance -= 5;
    if (victim->pcdata->condition[COND_FULL] > 25)
	    chance += 10;
    if (victim->pcdata->condition[COND_THIRST] > 20)
	    chance += 5;
 }
	
	
	
     if ((number_percent() > chance + ch->pcdata->learned[gsn_lull])
     || (saves_spell(ch->level,victim, DAM_SOUND)))
     { 
         if (!IS_NPC(victim))
         {
           if (!can_see(victim,ch))
             do_yell(victim,"Help! Someone is attacking me!");
           else
           {
              if (IS_NPC(ch))
                sprintf(buf, "Help! %s is attacking me!",ch->short_descr);
              else
                sprintf(buf,"Help! %s is attacking me!",
                (is_affected(ch,gsn_doppelganger) && (!IS_IMMORTAL(victim)))? ch->doppel->name :
                (is_affected(ch,gsn_treeform) && (!IS_IMMORTAL(victim)))? "A mighty oak tree" : 
		    ch->name);
              do_yell(victim,buf);
           }
         }
	send_to_char("You maul your lullaby.\n\r",ch);
	send_to_char("\n\r",ch);
	act ("$n attempts to sing a lullaby, but it sounds mangled.",ch,NULL,NULL,TO_ROOM);
	check_improve(ch,gsn_lull,FALSE,1);
	multi_hit( ch, victim, TYPE_UNDEFINED );
	ch->mana = ch->mana - 12; 
	return;
	}

	act( "You sweetly sing a soft lullaby.",ch,NULL,NULL,TO_CHAR );
	act( "$n sweetly sings a soft lullaby.",ch,NULL,NULL,TO_ROOM );
	ch->mana = ch->mana - 25;
	check_improve(ch,gsn_lull,TRUE,1); 

        if   (IS_AWAKE(victim)
        &&   (!IS_IMMORTAL(victim))
        &&   (!is_safe(ch,victim))
        &&   (!is_same_group(victim, ch)))
         {
	   WAIT_STATE( ch, skill_table[gsn_lull].beats );
	   send_to_char ("The last thing you remember is a beautiful voice before drifting off to sleep.\n\r",victim);

         if (victim->riding)
         { 
           act("$n falls off $s mount.",victim,NULL,NULL,TO_ROOM);
           victim->riding->rider = NULL;
           victim->riding        = NULL;
         }

	   victim->position = POS_SLEEPING;

	   af.where			= TO_AFFECTS;
	   af.type       		= gsn_lull;
	   af.level      		= ch->level;
	   af.duration   		= 5;
	   if (ch->level > 39 )
	      af.duration 	= 6;	
	   if (ch->level > 59 )
	      af.duration 	= 8;
	   if (ch->level > 79 )
	      af.duration 	= 10;  
	   af.location 		= APPLY_NONE;
	   af.modifier 		= 0;
	   af.bitvector	 	= AFF_SLEEP;
	   affect_to_char(victim,&af);
	   return;
	}
}

/* Apprehend
 * Allows Solamnics to force a player into jail!
 * 
 */
void do_apprehend(CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *fch;
    CHAR_DATA *fch_next;
    CHAR_DATA *victim;
    int cbonus, vbonus, rchance, chstr, chdex, vicstr, vicdex;
    char buf[100];
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if (ORG(ch->org_id)  != ORG_SOLAMNIC)
    {
	send_to_char("You have no authority to apprehend anyone.\n\r", ch);
	return;
    }

    if((ch->in_room->area->control_flags != CONTROL_SOLAMNICS)
    &  (ch->in_room->sector_type != SECT_SOLAMNIC))
    {
      send_to_char("Your Government has no authority here, you can NOT apprehend people for a crime in these parts... only on Solamnic controlled grounds.\n\r", ch);
      return;
    }

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Syntax: apprehend <character> <assault|murder|theft|treason>\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, NULL, arg1 ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if (!can_see(ch, victim))
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( IS_NPC(victim) )
    {
        send_to_char( "You cannot apprehend them.\n\r", ch );
        return;
    }

    if (victim->position == POS_DEAD)
    {
	act("$E is dead and has no body.\n\r",ch, NULL, victim, TO_CHAR);
	return;
    }

    if (IS_IMMORTAL(victim))
    {
	send_to_char( "They are much too powerful to apprehend.\n\r", ch);
	return;
    }

    if (IS_AFFECTED( victim, AFF_SHACKLES ) )
    {
      act("$N is already shackled in cuffs.", ch, NULL,victim, TO_CHAR);
      return;
    }

    if ( IS_AFFECTED2(ch, AFF_PARALYSIS) 
    ||   IS_AFFECTED3(ch, AFF_HOLDPERSON))
    {
      send_to_char("You can't move at all, there's no way you can do that.\n\r",ch);
      return;
    }

    if (ORG(ch->org_id)  == ORG_SOLAMNIC)
    {
      if (IS_SET(victim->act, PLR_OUTLAW))
	   {
		send_to_char( "There is no jail sentence for Outlaws!\n\r", ch);
		return;
	   }
	else if ( !str_cmp( arg2, "assault"))
	   {         
		if (!IS_SET(victim->act, PLR_ASSAULT))
		{
		   send_to_char( "They are not wanted for assault.\n\r", ch);
		   return;
		}
	   }
	else if ( !str_cmp( arg2, "murder")) 
	   {           
		if (!IS_SET(victim->act, PLR_KILLER))
		{
		   send_to_char( "They are not wanted for murder.\n\r", ch);
		   return;
		}
	   }
	else if ( !str_cmp( arg2, "theft")) 
	   {           
		if (!IS_SET(victim->act, PLR_THIEF))
		{
		   send_to_char( "They are not wanted for theft.\n\r", ch);
		   return;
		}
	   }
	else if ( !str_cmp( arg2, "treason")) 
	   {
		if (!IS_SET(victim->act, PLR_TREASON))           
		{
		   send_to_char( "They are not wanted for treason.\n\r", ch);
		   return;		
		}
	   }
	 else
	   {
                 send_to_char( "Syntax: apprehend <character> <assault|murder|theft|treason>\n\r", ch );
		 return;
	   }	
     }
	
    cbonus = ch->level;
    vbonus = victim->level;

    chstr = ch->perm_stat[STAT_STR];
    vicstr = victim->perm_stat[STAT_STR];
    
    chdex = ch->perm_stat[STAT_DEX];
    vicdex = victim->perm_stat[STAT_DEX];

    if ( ( chstr - vicstr ) > 0 )
	cbonus += ( chstr - vicstr );
    else
	vbonus += ( vicstr - chstr );  

    if ( ( chdex - vicdex ) > 0 )
	cbonus += ( chdex - vicdex );
    else
	vbonus += ( vicdex - chdex );  

    rchance = number_range(-3, 3);
    cbonus += rchance;
    rchance = number_range(-3, 3);
    vbonus += rchance;

    if (IS_AFFECTED(victim, AFF_WEAKEN))
    cbonus += 10;

    for (fch = ch->in_room->people; fch != NULL; fch = fch_next)
    {
	fch_next = fch->next_in_room;
	if ((fch->master == ch) && (fch->level >= 20) && (!IS_NPC(fch)))
	{
	    cbonus += fch->level / 10;
	}
    }

    for (fch = victim->in_room->people; fch != NULL; fch = fch_next)
    {
	fch_next = fch->next_in_room;
	if ((fch->master == victim) && (fch->level >= 20))
	{
	    vbonus += fch->level / 10;
	}
    }

    if ((cbonus < vbonus) && !IS_AFFECTED3(victim,AFF_SUBDUE))
    {
	act("$n attempts to clasp shackles around $N's wrists, but fails.\n\r"
	    "", ch, NULL, victim, TO_NOTVICT);
	act("You attempt to clasp shackles around $N's wrists, but fail.", 
	ch, NULL, victim, TO_CHAR);
	send_to_char( "\n\r", ch);
	act("$n attempts to clasp shackles around your wrists, but fails.", 
	ch, NULL, victim, TO_VICT);
	send_to_char( "\n\r", victim);
	sprintf(buf, "You won't take me so easily, %s!\n\r", ch->name);
	do_yell(victim, buf);
	multi_hit( ch, victim, TYPE_UNDEFINED );
	return;
    }
    else
    {
	act("$n clasps shackles around $N's wrists.\n\r"
	    "", ch, NULL, victim, TO_NOTVICT);
	act("You clasp shackles around $N's wrists.", ch, NULL, victim, TO_CHAR);
	send_to_char( "\n\r", ch);
	act("$n clasps shackles around your wrists.",ch, NULL, victim, TO_VICT);
	send_to_char( "\n\r", victim);
	if (victim->sex == 1)
	{
	    do_yell(ch, "Take him away!");
	}
	else
	{
	    do_yell(ch, "Take her away!");
	}

      if (ORG(ch->org_id)  == ORG_SOLAMNIC)
      {

	act("\n\rA pair of wardens hastily arrive at your request, taking $N\n\r"
	    "by the arms and escorting $M off to the penitentiary.",
	ch, NULL, victim, TO_CHAR);
	send_to_char( "\n\r", ch);
	act("\n\rA pair of wardens hastily arrive at $n's request, taking $N\n\r"
	    "by the arms and escorting $M off to the penitentiary.\n\r"
	    "", ch, NULL, victim, TO_NOTVICT);
	act("\n\rA pair of wardens hastily arrive at $n's request, taking you\n\r"
	    "by the arms and escorting you off to the penitentiary!",
	ch, NULL, victim, TO_VICT);
	send_to_char( "\n\r", victim);
	sprintf(buf, "%s %d", victim->name, 1280);
	      if ( !str_cmp(arg2, "assault"))
		{
		    spell_shackles(skill_lookup("shackles"), 2, ch, victim, TARGET_CHAR);

		    REMOVE_BIT(victim->act, PLR_ASSAULT);
		    do_transfer(ch, buf);
		    return;
		}
		else if (!str_cmp(arg2, "theft"))
		{
		    spell_shackles(skill_lookup("shackles"), 6, ch, victim, TARGET_CHAR);

		    REMOVE_BIT(victim->act, PLR_THIEF);
		    do_transfer(ch, buf);
		    return;
		}
		else if (!str_cmp(arg2, "murder"))
		{
		    spell_shackles(skill_lookup("shackles"), 8, ch, victim, TARGET_CHAR);

		    REMOVE_BIT(victim->act, PLR_KILLER);
		    do_transfer(ch, buf);
		    return;
		}
		else if (!str_cmp(arg2, "treason"))
		{
		    spell_shackles(skill_lookup("shackles"), 14, ch, victim, TARGET_CHAR);
		    REMOVE_BIT(victim->act, PLR_TREASON);
		    do_transfer(ch, buf);
		    return;
		}
	}
     }
}

void do_rally(  CHAR_DATA *ch, char *argument)
{

 CHAR_DATA *pChar;
 CHAR_DATA *pChar_next;
 int chance;
 AFFECT_DATA af;

 if (IS_AFFECTED3(ch,AFF_RALLY)) 
  {
    send_to_char("You are still worn out from your last rally.\n\r",ch);
    return;
  }

 if ( ch->fighting == NULL)
  {
    send_to_char("You can only rally allies in the passion of battle.\n\r",ch);
    return;
  }
 
 if ( ch->mana < 200)
  {
    send_to_char("You can't muster enough energy to rally allies.\n\r",ch);
    return;
  }

 if ( ( ch->level < skill_table[gsn_rally].skill_level[ch->class] )
 || ( chance = get_skill(ch,gsn_rally) == 1 ))
  {
     act("$n makes a pathetic attempt to rally assistance from the locals.",ch,NULL,NULL,TO_ROOM);
     act("The locals laugh at your pathetic attempt to rally their aid.",ch,NULL,NULL,TO_CHAR);
     return;
  }

  act("$n thunders a brilliant monologue, requesting allies to join the battle.",ch,NULL,NULL,TO_ROOM);
  act("You thunder a brilliant monologue, requesting assistance of the locals.",ch,NULL,NULL,TO_CHAR);
 
  ch->mana -= 200;

  af.where     	= TO_AFFECTS3;
  af.type      	= gsn_rally;
  af.level     	= ch->level;
  af.duration  	= 15;
  af.location  	= APPLY_NONE;
  af.modifier  	= UMAX(1,ch->level/5);
  af.bitvector 	= AFF_RALLY;
  affect_to_char(ch,&af);

  pChar_next = NULL;
  for ( pChar = ch->in_room->people; pChar; pChar = pChar_next)
  {
    pChar_next = pChar->next_in_room;
   
    if (( pChar->fighting == NULL) 
    && (!IS_AFFECTED(pChar,AFF_CHARM)) && IS_NPC(pChar) )
    {
      if ( number_percent( ) < ch->pcdata->learned[gsn_rally] )
      {
        multi_hit( pChar, ch->fighting, TYPE_UNDEFINED );
        check_improve(ch,gsn_rally,TRUE,1);
        continue;
	}
      else 
      {
        check_improve(ch,gsn_rally,FALSE,1);
        act("$n fails to convince the locals to lend their aid in battle.",
          ch,NULL,NULL,TO_ROOM);
        act("You fail to convince the locals of the worth of your cause.",
          ch,NULL,NULL,TO_CHAR);
	  if (ch->mana >= 50)
	  {
          ch->mana -= 50;
	  }
	  else
	  {
	    ch->mana = 0;
	  }
        act("$n looks around and chuckles.",pChar,NULL,NULL,TO_ROOM);
        act("$n looks at you and chuckles.",pChar,NULL,NULL,TO_CHAR);
        continue;
      }
    }
  }
 return;
}

void entwine( CHAR_DATA *ch, CHAR_DATA *victim )
{
    OBJ_DATA *obj;

    if ( ( obj = get_eq_char( victim, WEAR_WIELD ) ) == NULL )
        return;

    if ( IS_OBJ_STAT(obj,ITEM_NOREMOVE))
    {
        act("You try to yank $N's weapon away with your whip, but it won't budge!",
             ch,NULL,victim,TO_CHAR);
        act("$n tries to yank your weapon away with $s whip, but it won't budge!",
            ch,NULL,victim,TO_VICT);
        act("$n tries to yank away $N's weapon with $s whip, but fails.",
             ch,NULL,victim,TO_NOTVICT);
        return;
    }

    act( "$n entwines $s whip around your weapon and yanks it into $s possession!", 
         ch, NULL, victim, TO_VICT    );
    act( "You entwine your whip around $N's weapon and yank it into your possession!",  
          ch, NULL, victim, TO_CHAR    );
    act( "$n entwines $s whip around $N's weapon and yanks it into $s possession!",  
          ch, NULL, victim, TO_NOTVICT );
    obj_from_char( obj );
    if ( IS_OBJ_STAT(obj,ITEM_NODROP) )
        obj_from_char( obj );
        obj_to_char( obj, ch );
    if (IS_NPC(victim) && victim->wait == 0 && can_see_obj(victim,obj))
    return;
}

void do_entwine( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    OBJ_DATA *wield;
    int chance,hth,ch_weapon,vict_weapon,ch_vict_weapon;
    wield = get_eq_char( ch, WEAR_WIELD );
    hth = 0;

    if ((chance = get_skill(ch,gsn_entwine)) == 0)
    {
      send_to_char( "You don't know how to entwine weapons.\n\r", ch );
      return;
    }

    if ( ( victim = ch->fighting ) == NULL )
    {
      send_to_char( "You aren't fighting anyone.\n\r", ch );
      return;
    }

    if ( wield == NULL )
    {
	send_to_char("You must be wielding a whip in order to entwine your opponant's weapon.\n\r",ch);
	return;
    }

    if ( wield->value[0] != WEAPON_WHIP )
    {
	send_to_char("You must be wielding a whip in order to entwine your opponant's weapon.\n\r",ch);
	return;
    }

    if ( ( obj = get_eq_char( victim, WEAR_WIELD ) ) == NULL )
    {
      send_to_char( "Your opponent is not wielding a weapon.\n\r", ch );
      return;
    }

    if ( IS_AFFECTED2(ch, AFF_PARALYSIS) 
    ||   IS_AFFECTED3(ch, AFF_HOLDPERSON))
    {
      send_to_char("You can't move at all, there's no way you can do that.\n\r",ch);
      return;
    }

    ch_weapon = get_weapon_skill(ch,get_weapon_sn(ch));
    vict_weapon = get_weapon_skill(victim,get_weapon_sn(victim));
    ch_vict_weapon = get_weapon_skill(ch,get_weapon_sn(victim));

    /* modifiers */
    if ( get_eq_char(ch,WEAR_WIELD) == NULL)
        chance = chance * hth/150;
    else
        chance = chance * ch_weapon/100;
    chance += (ch_vict_weapon/2 - vict_weapon) / 2; 
    /* dex vs. strength */
    chance += get_curr_stat(ch,STAT_DEX)/3;
    chance -= 2 * get_curr_stat(victim,STAT_STR);
    /* level */
    chance += (ch->level - victim->level);
 
    /* and now the attack */
    if (number_percent() < chance)
    {
        WAIT_STATE( ch, skill_table[gsn_entwine].beats );
        entwine( ch, victim );
        check_improve(ch,gsn_entwine,TRUE,1);
    }
    else
    {
        WAIT_STATE(ch,skill_table[gsn_entwine].beats);
        act("You try to use your whip to yank away $N's weapon, but fail.",
          ch,NULL,victim,TO_CHAR);
        act("$n tries to use $s whip to yank away your weapon, but fails.",
          ch,NULL,victim,TO_VICT);
        act("$n tries to use $s whip to yank away $N's weapon, but fails.",
          ch,NULL,victim,TO_NOTVICT);
        check_improve(ch,gsn_entwine,FALSE,1);
    }
    return;
}

void do_eyerub( CHAR_DATA *ch, char *argument )
{
  AFFECT_DATA af;
  int chance;

  if ( (chance = get_skill(ch,gsn_eyerub)) == 0
  ||   (ch->level < skill_table[gsn_eyerub].skill_level[ch->class]))
  {	
    send_to_char("You rub your eyes, but it didn't do anything.\n\r",ch);
    return;
  }

  if ( number_percent() > 24)
  {
    if (is_affected(ch,skill_lookup("fire breath")))
    {
      send_to_char("You rub the smoke out of your eyes.\n\r",ch);
      act("$n rubs the smoke out of $s eyes.",ch,NULL,NULL,TO_ROOM);
      affect_strip(ch,skill_lookup("fire breath"));
      check_improve(ch,gsn_eyerub,TRUE,2);
      WAIT_STATE(ch,skill_table[gsn_eyerub].beats);
      return;
    }
    else
    if (is_affected(ch,gsn_dirt))
    {
      send_to_char("You rub the dirt out of your eyes.\n\r",ch);
      act("$n rubs the dirt out of $s eyes.",ch,NULL,NULL,TO_ROOM);
      affect_strip( ch, gsn_dirt);
      check_improve(ch,gsn_eyerub,TRUE,2);
      WAIT_STATE(ch,skill_table[gsn_eyerub].beats);
      return;
    }
    else
      send_to_char("You rub your eyes.\n\r",ch);
      WAIT_STATE(ch,skill_table[gsn_eyerub].beats);
    return;
  }
  else
  {
    if (is_affected(ch,skill_lookup("fire breath")))
    {
      send_to_char("Rubbing your eyes only made matters worse.\n\r",ch);
      act("$n rubs $s eyes, making matters worse.",ch,NULL,NULL,TO_ROOM);
      affect_strip(ch,skill_lookup("fire breath"));
      af.where        = TO_AFFECTS;
      af.type         = skill_lookup("fire breath");
      af.level        = ch->level;
      af.duration     = number_range(5,10);
      af.location     = APPLY_HITROLL;
      af.modifier     = -6;
      af.bitvector    = AFF_BLIND;
      affect_to_char(ch,&af);
      check_improve(ch,gsn_eyerub,FALSE,2);
      WAIT_STATE(ch,skill_table[gsn_eyerub].beats);
      return;
    }
    else
    if (is_affected(ch,gsn_dirt))
    {
      send_to_char("Rubbing your eyes only made matters worse.\n\r",ch);
      act("$n rubs $s eyes, making matters worse.",ch,NULL,NULL,TO_ROOM);
      affect_strip( ch, gsn_dirt);
      af.where		= TO_AFFECTS;
      af.type 		= gsn_dirt;
      af.level 		= ch->level;
      af.duration		= 5;
      af.location		= APPLY_HITROLL;
      af.modifier		= -6;
      af.bitvector 	= AFF_BLIND;
      affect_to_char(ch,&af);
      check_improve(ch,gsn_eyerub,FALSE,2);
      WAIT_STATE(ch,skill_table[gsn_eyerub].beats);
      return;
    }
    else
      send_to_char("You rub your eyes.\n\r",ch);
      act("$n rubs $s eyes.",ch,NULL,NULL,TO_ROOM);
      WAIT_STATE(ch,skill_table[gsn_eyerub].beats);
    return;
  }
}

void do_fingerbreak( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    AFFECT_DATA af;
    int chance;

    if ( ch->riding )
    {
        send_to_char( "You can't get close enough while mounted.\n\r", ch );
        return;
    }

    if ( (chance = get_skill(ch,gsn_fingerbreak)) == 0
    ||   (ch->level < skill_table[gsn_fingerbreak].skill_level[ch->class]))
    {   
      send_to_char( "Your not skilled enough to break anyones fingers.\n\r", ch );
      return;
    }

    if ( IS_AFFECTED2(ch, AFF_PARALYSIS) 
    ||   IS_AFFECTED3(ch, AFF_HOLDPERSON))
    {
      send_to_char("You can't move at all, there's no way you can do that.\n\r",ch);
      return;
    }

    if ( ( victim = ch->fighting ) == NULL )
    {
      send_to_char("You aren't fighting anyone!\n\r",ch);
      return;
    }

    if (ch->fight_pos > FIGHT_FRONT)
    {
      send_to_char("You are too far away to break anyone's fingers!\r\n", ch);
      return;
    }

    if (victim->fight_pos > FIGHT_FRONT)
    {
      send_to_char("They are too far away to break their fingers!\r\n", ch);
      return;
    }

    if (!IS_SET(victim->parts,PART_FINGERS))
    {
      send_to_char("They don't appear to have any fingers to smash!\r\n", ch);
      return;
    }

    if (is_affected(victim,skill_lookup("finger break")))
    {
      send_to_char("Their fingers are already broken.\n\r",ch);
      return;
    }

    if ( is_safe( ch, victim ) )
      return;

    if (IS_NPC(victim)
    &&  victim->fighting != NULL
    &&  !is_same_group(ch,victim->fighting))
    {
        send_to_char("Kill stealing is not permitted.\n\r",ch);
        return;
    }

    if (number_percent( ) < (ch->pcdata->learned[gsn_fingerbreak]) )
    {
      check_improve(ch,gsn_fingerbreak,TRUE,1);

      if (!IS_NPC(victim))
      {
        if (!can_see(victim,ch) ) 
          do_yell(victim, "Argh! Someone just busted up my fingers!");
        else
        {
          if (IS_NPC(ch))
            sprintf(buf, "Argh! %s just busted up my fingers!",ch->short_descr);
          else
            sprintf(buf, "Argh! %s just busted up my fingers!", 
                (is_affected(ch,gsn_doppelganger) && (!IS_IMMORTAL(victim)))? ch->doppel->name :
                (is_affected(ch,gsn_treeform) && (!IS_IMMORTAL(victim)))? "A mighty oak tree" : 
		    ch->name);
            do_yell( victim, buf );
        }
      }
      act("You smash $N's fingers, busting them up pretty good!", ch, NULL, victim, TO_CHAR);
      act("$n smashes your fingers, busting them up pretty good!", ch, NULL, victim, TO_VICT);
      act("$n smashes $N's fingers, busting them up pretty good!", ch, NULL, victim, TO_NOTVICT);
      check_improve(ch,gsn_fingerbreak,FALSE,1);
      af.where        = TO_AFFECTS;
      af.type 	    = gsn_fingerbreak;
      af.level        = ch->level;
      af.duration     = number_range(1,5);
      af.location     = APPLY_HITROLL;
      af.modifier     = -2;
      af.bitvector    = 0;
      affect_to_char(victim,&af);
      damage( ch, victim, number_range( 1, ch->level ),gsn_fingerbreak,DAM_BASH,TRUE);
      WAIT_STATE( ch, skill_table[gsn_fingerbreak].beats );
      return;
    }
}

void do_execute( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    char *name;

    argument = one_argument( argument, arg );
    one_argument( argument, arg2 );

    if ( !IS_NPC( ch ) 
    && ch->level < skill_table[gsn_execute].skill_level[ch->class] )
    {
      send_to_char( "You don't know how to do that.\n\r", ch );
      return;
    }

    if ( arg[0] == '\0' )
    {
      send_to_char( "Who would you like to execute?\n\r", ch );
      return;
    }

    if ( ( victim = get_char_room( ch, NULL, arg ) ) == NULL )
    {
      send_to_char( "They aren't here.\n\r", ch );
      return;
    }

    if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL)
    {
      send_to_char( "You need to wield an axe in order to execute someone.\n\r", ch );
      return;
    }

    if ( obj->value[0] != WEAPON_AXE )
    {
      send_to_char("You can only execute someone while wielding an Axe.\n\r",ch);
      return;
    }

    if ( ch == victim )
    {
      send_to_char( "Yes, that's what we need.... more suicide!\n\r", ch );
      return;
    }

    if ( ch->fighting != NULL )
    {
      send_to_char( "Your too busy fighting.\n\r", ch );
      return;
    }

    if ( is_safe( ch, victim ) )
      return;

    if ( IS_AFFECTED2(ch, AFF_PARALYSIS) 
    ||   IS_AFFECTED3(ch, AFF_HOLDPERSON))
    {
      send_to_char("You can't move at all, there's no way you can do that.\n\r",ch);
      return;
    }

    if (ch->fight_pos > FIGHT_FRONT)
    {
      send_to_char("You are too far away to execute anyone!\r\n", ch);
      return;
    }

    if ( IS_IMMORTAL(victim) )
    {
      send_to_char( "You'd only piss them off trying that.\n\r", ch );
      return;
    }

    if ( !IS_AWAKE(victim)
    && ( number_percent() < 30 ))
    {
      check_improve(ch,gsn_execute,TRUE,1);
      act("You skillfully swing your axe and execute $N, lobbing off $S head!\n\r",ch,NULL,victim,TO_CHAR);
      act("$n has skillfully executed you, chopping off your head!\n\r",ch,NULL,victim,TO_VICT);
      act("$n has skillfully executed $N, lopping off $S head!\n\r",ch,NULL,victim,TO_NOTVICT);
      raw_kill( ch, victim );
      name = str_dup(victim->name);
      obj = create_object(get_obj_index(OBJ_VNUM_SEVERED_HEAD),0);

      obj->extra_flags |= ITEM_NODROP|ITEM_NOUNCURSE;
      obj->timer	= number_range( 15, 20 );

      sprintf(buf, "%s %s", "head", name);
      free_string( obj->name );
      obj->name = str_dup( buf);

      sprintf( buf, obj->short_descr, name );
      free_string( obj->short_descr );
      obj->short_descr = str_dup( buf );

      sprintf( buf, obj->description, name );
      free_string( obj->description );
      obj->description = str_dup( buf );

      obj_to_char(obj,ch);
      free_string(name);

      sprintf( log_buf, "%s was executed by %s at %s [room %d]",
      	(IS_NPC(victim) ? victim->short_descr : victim->name),
            (IS_NPC(ch) ? ch->short_descr : ch->name),
            ch->in_room->name, ch->in_room->vnum);
 
      if (IS_NPC(victim))
      	wiznet(log_buf,NULL,NULL,WIZ_MOBDEATHS,0,0);
      else
            wiznet(log_buf,NULL,NULL,WIZ_DEATHS,0,0);

	/* Begin Paladine's PK Logger code*/
	if(!IS_NPC(victim) && ch != victim)
	{
	  CHAR_DATA *iPked;

	  iPked = ch;

	  /* Follow charm thread to responsible character. */
    	  while ( IS_AFFECTED(iPked, AFF_CHARM) && iPked->master != NULL )
	  iPked = iPked->master;

	  /* Log both Cheap PKILL via your charmie and Regular PKILL */
	  if( !IS_NPC(iPked) || !IS_NPC(ch) )
	  { 
	    FILE *fp;
	    char *strtime = NULL;

  	    /* write to the pk log */
    	    fclose(fpReserve);
    	    if( (fp = fopen(PK_LOG, "a")) == NULL)
    	    {
		bug("Error opening pk_log.txt",0);
		fclose(fp);
		fpReserve = fopen(NULL_FILE, "r");
    	    }
	    else
	    {
		sprintf( log_buf, "%s was EXECUTED by %s at room %d",
      		victim->name, ch->name, ch->in_room->vnum);
    		strtime = ctime(&current_time);
    		strtime[strlen(strtime)-1] = '\0';
		sprintf(buf2, "%s :: %s\n", strtime, log_buf);
    		fprintf(fp, "%s", buf2);
    		fclose(fp);
    		fpReserve = fopen(NULL_FILE, "r");
	    }
	  }
	} /* End PK Logger */

	sprintf( log_buf, "%s executed by %s at %d",victim->name,
	  (IS_NPC(ch) ? ch->short_descr : ch->name),
	  ch->in_room->vnum );
	log_string( log_buf );

    }
    else
    {
	act("$n's attempt to execute you was a failure!\n\r",ch,NULL,victim,TO_VICT);
	act("Your execution attempt on $N's life was a failure!\n\r",ch,NULL,victim,TO_CHAR);
	act("$n's execution attempt on $N's life was a failure!\n\r",ch,NULL,victim,TO_NOTVICT);
      if (!ch->fighting)
        set_fighting(ch, victim);
	check_improve(ch,gsn_execute,FALSE,1);
    }
  return;
}

void do_earpunch( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    AFFECT_DATA af;
    int chance;

    if ( ch->riding )
    {
        send_to_char( "You can't get close enough while mounted.\n\r", ch );
        return;
    }

    if ( (chance = get_skill(ch,gsn_earpunch)) == 0
    ||   (ch->level < skill_table[gsn_earpunch].skill_level[ch->class]))
    {   
      send_to_char( "Your not skilled enough to break anyones fingers.\n\r", ch );
      return;
    }

    if ( IS_AFFECTED2(ch, AFF_PARALYSIS) 
    ||   IS_AFFECTED3(ch, AFF_HOLDPERSON))
    {
      send_to_char("You can't move at all, there's no way you can do that.\n\r",ch);
      return;
    }

    if ( ( victim = ch->fighting ) == NULL )
    {
      send_to_char("You aren't fighting anyone!\n\r",ch);
      return;
    }

    if (ch->fight_pos > FIGHT_FRONT)
    {
      send_to_char("You are too far away to clap punch their ears!\r\n", ch);
      return;
    }

    if (victim->fight_pos > FIGHT_FRONT)
    {
      send_to_char("They are too far away to clap punch their ears!\r\n", ch);
      return;
    }

    if (is_affected(victim,skill_lookup("earpunch")))
    {
      send_to_char("Their ears are already ringing and they're already dizzy.\n\r",ch);
      return;
    }

    if ( is_safe( ch, victim ) )
    {
      send_to_char("They dodged your attempted earpunch.\n\r",ch);
      return;
    }

    if ( get_eq_char( ch, WEAR_WIELD ) != NULL )
    {
      send_to_char( "You can't earpunch while wielding a weapon.\n\r", ch );
      return;
    }

    if (number_percent( ) < (ch->pcdata->learned[gsn_earpunch]) )
    {

      check_improve(ch,gsn_earpunch,TRUE,1);
      act("You clap punch $N's ears, making them deaf and dizzy!", ch, NULL, victim, TO_CHAR);
      act("$n clap punches your ears, making you deaf and dizzy!", ch, NULL, victim, TO_VICT);
      act("$n clap punches $N's ears, making $M deaf and dizzy!", ch, NULL, victim, TO_NOTVICT);
      af.where        = TO_AFFECTS;
      af.type 	    = gsn_earpunch;
      af.level        = ch->level;
      af.duration     = number_range(1,5);
      af.location     = APPLY_HITROLL;
      af.modifier     = -4;
      af.bitvector    = 0;
      affect_to_char(victim,&af);

      if (!IS_AFFECTED2(victim, AFF_DEAFNESS))
      {
        af.where		= TO_AFFECTS2;
    	  af.location  	= 0;
    	  af.modifier  	= 0;
    	  af.bitvector 	= AFF_DEAFNESS;
    	  affect_to_char(victim,&af);
      }

      damage( ch, victim, number_range( 1, ch->level ),gsn_earpunch,DAM_SOUND,TRUE);
      WAIT_STATE( ch, skill_table[gsn_earpunch].beats );
    }
  return;
}

void do_brainjar( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  int chance;

  if ( ch->riding )
  {
    send_to_char( "You can't get close enough while mounted.\n\r", ch );
    return;
  }

  one_argument(argument,arg);

  if ( arg[0] == '\0' )
  {
    send_to_char( "Jar who's brain?\n\r", ch );
     return;
  }

  if ( (chance = get_skill(ch,gsn_brainjar)) == 0
  ||   (ch->level < skill_table[gsn_brainjar].skill_level[ch->class]))
  {
    send_to_char("You aren't skilled enough to jar anyone's brain.\n\r",ch);
    return;
  }

  if ((victim = get_char_room(ch, NULL, arg)) == NULL)
  {
    send_to_char("They aren't here.\n\r",ch);
    return;
  }

  if (IS_AFFECTED2(victim,AFF_BLACKJACK))
  {
    act("$E has been knocked out, your blow will have no effect.",ch,NULL,victim,TO_CHAR);
    return;
  }

  if (is_affected(victim,gsn_brainjar))
  {
    act("$E has already had a severe blow to the head, jarring their brain.",ch,NULL,victim,TO_CHAR);
    return;
  }

  if (ch->fight_pos > FIGHT_FRONT)
  {
    send_to_char("You are too far away to place a blow to their head!\r\n", ch);
    return;
  }

  if (victim->fight_pos > FIGHT_FRONT)
  {
    send_to_char("They are too far away to place a blow to their head!\r\n", ch);
    return;
  }

  if (victim == ch)
  {
    send_to_char("You can't place a blow to your own head to jar it.\n\r",ch);
    return;
  }

  if ( IS_AFFECTED2(ch, AFF_PARALYSIS) 
  ||   IS_AFFECTED3(ch, AFF_HOLDPERSON))
  {
    send_to_char("You can't move at all, there's no way you can do that.\n\r",ch);
    return;
  }

  if (IS_AFFECTED(victim,AFF_SLEEP))
  {
    act("$e is sleeping.  Trying to wake them up?.", ch,NULL,victim,TO_CHAR);
    return;
  }

  if (is_safe(ch,victim))
  {
    send_to_char("They dodged your attempted blow to the head.\n\r",ch);
    return;
  }

  if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
  {
    act("But $N is such a good friend!",ch,NULL,victim,TO_CHAR);
    return;
  }

  if (victim->riding)
  {
    act("$N is mounted and too high up to land a proper blow to the head.\n\r",ch, NULL, victim,TO_CHAR);
    return;
  }

  if (number_percent( ) < (ch->pcdata->learned[gsn_brainjar]) 
  && (victim->level <= (ch->level+3)))
  {
    AFFECT_DATA af;
    act("$n has been hit hard in the head, jarring $s brain!",victim,NULL,NULL,TO_ROOM);
    send_to_char("WHAM!! You've been hit hard in the head, jarring your brain!\n\r",victim);

    check_improve(ch,gsn_brainjar,TRUE,2);

    af.where		= TO_AFFECTS;
    af.type 		= gsn_brainjar;
    af.level 		= ch->level;
    af.duration		= number_fuzzy(ch->level / 8);
    af.location		= APPLY_DEX;
    af.modifier		= -3;
    af.bitvector 		= 0;
    affect_to_char(victim,&af);

    af.where		= TO_AFFECTS2;
    af.location  		= APPLY_SPEECH_IMPAIR;
    af.modifier  		= MOD_SPEECH_GARBLE;
    af.bitvector 		= AFF_SPEECH_IMPAIR;
    affect_to_char( victim, &af );

    check_cpose(ch);
    check_cpose(victim);

    if ( !IS_NPC(victim) )
    {
      if ( !IS_IMMORTAL(ch) )
      {
        ch->quit_timer = -5;
        ch->lastfought = victim;
        if (!IS_NPC(victim))
        {
          free_string(ch->last_pc_fought);
          ch->last_pc_fought = str_dup(victim->name);
        }
      }
      victim->quit_timer = -5;
      victim->lastfought = ch;
      if (!IS_NPC(victim))
      {
        free_string(ch->last_pc_fought);
        ch->last_pc_fought = str_dup(victim->name);
      }
    }

    }
    else
    {
      act("$n tries to jar your brain with a powerful blow to the head!",ch,NULL,victim,TO_VICT);
      send_to_char("Your powerful blow to their head didn't do the trick.\n\r",ch);
      act("$n tries to jar $N's brain with a powerful blow to the head, but fails.",ch,NULL,victim,TO_NOTVICT);
      if(!IS_NPC(ch))
      {     
        if (!can_see(victim,ch))
          do_yell(victim, "Help! Someone tried to jar my brain!");
      else
      {
        if (IS_NPC(ch))
          sprintf(buf, "Help! %s just tried to jar my brain!", ch->short_descr);
        else
          sprintf( buf, "Help! %s just tried to jar my brain!", 
                (is_affected(ch,gsn_doppelganger) && (!IS_IMMORTAL(victim)))? ch->doppel->name :
                (is_affected(ch,gsn_treeform) && (!IS_IMMORTAL(victim)))? "A mighty oak tree" : 
		    ch->name);
        do_yell( victim, buf );
      }
    }
    multi_hit( ch, victim, TYPE_UNDEFINED );
    check_improve(ch,gsn_brainjar,FALSE,2);
    return;
  }
}

void do_rupture_organ( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim;
  char buf[MAX_STRING_LENGTH];
  int chance;

  if ( ch->riding )
  {
    send_to_char( "You can't get close enough while mounted.\n\r", ch );
    return;
  }

  if ( ( victim = ch->fighting ) == NULL )
  {
    send_to_char( "You aren't fighting anyone.\n\r", ch );
    return;
  }

  if ( get_eq_char( ch, WEAR_WIELD ) != NULL )
  {
    send_to_char( "You can't deliver a life threatening blow while wielding a primary weapon.\n\r", ch );
    return;
  }

  if ( ch->fight_pos > FIGHT_FRONT )
  {
    send_to_char( "You aren't close enough to deliver a fatal punch!\n\r",ch);
    return;
  }

  if ( victim->fight_pos > FIGHT_FRONT )
  {
    send_to_char( "They aren't close enough to deliver a fatal punch!\n\r",ch);
    return;
  }

  if ( (chance = get_skill(ch,gsn_rupture)) == 0
  ||   (ch->level < skill_table[gsn_rupture].skill_level[ch->class]))
  {
    send_to_char("You aren't skilled enough to rupture anyone's organs.\n\r",ch);
    return;
  }

  if ( (chance = get_skill(ch,gsn_hand_to_hand)) == 0
  ||   (ch->level < skill_table[gsn_hand_to_hand].skill_level[ch->class]))
  {
    send_to_char("You must be skilled in Hand to Hand combat to rupture anyone's organs.\n\r",ch);
    return;
  }

  if ( (chance = get_skill(ch,gsn_hand_to_hand)) <= 94)
  {
    send_to_char("You aren't yet skilled enough in Hand to Hand combat to rupture anyone's organs.\n\r",ch);
    return;
  }

  if (is_affected(victim,gsn_rupture))
  {
    act("$N is already bleeding internally, and is too weary of another blow to $S organs.",ch,NULL,victim,TO_CHAR);
    return;
  }

  if (is_safe(ch,victim))
  {
    send_to_char("They dodged your attempted organ rupture.\n\r",ch);
    return;
  }

  if (victim == ch)
  {
    send_to_char("You can't place a blow to your own body.\n\r",ch);
    return;
  }

  if (number_percent( ) < 10) 
  {
    act("WHAM!! You hit $N so hard, it ruptures $S organs, killing $M instantly.",
          ch, NULL, victim, TO_CHAR );
    act("WHAM!! $n hits $N so hard, it ruptures $S organs, killing $M instantly.", 
          ch, NULL, victim, TO_NOTVICT );
    act("WHAM!! $n punches you so hard, your organs rupture, killing you instantly.",
          ch, NULL, victim, TO_VICT );

    check_improve(ch,gsn_rupture,TRUE,1);
    raw_kill(ch,victim);

    sprintf( log_buf, "%s was organ ruptured by %s at %s [room %d]",
      (IS_NPC(victim) ? victim->short_descr : victim->name),
      (IS_NPC(ch) ? ch->short_descr : ch->name),
       ch->in_room->name, ch->in_room->vnum);
 
    if (IS_NPC(victim))
      wiznet(log_buf,NULL,NULL,WIZ_MOBDEATHS,0,0);
    else
      wiznet(log_buf,NULL,NULL,WIZ_DEATHS,0,0);

    /* Begin Paladine's PK Logger code*/
    if(!IS_NPC(victim) && ch != victim)
    {
	CHAR_DATA *iPked;

	iPked = ch;

	/* Follow charm thread to responsible character. */
     while ( IS_AFFECTED(iPked, AFF_CHARM) && iPked->master != NULL )
	iPked = iPked->master;

	/* Log both Cheap PKILL via your charmie and Regular PKILL */
	if( !IS_NPC(iPked) || !IS_NPC(ch) )
	{ 
	  FILE *fp;
	  char *strtime = NULL;

  	  /* write to the pk log */
    	  fclose(fpReserve);
    	  if( (fp = fopen(PK_LOG, "a")) == NULL)
    	  {
	    bug("Error opening pk_log.txt",0);
	    fclose(fp);
	    fpReserve = fopen(NULL_FILE, "r");
    	  }
	  else
	  {
	    sprintf( log_buf, "%s was ORGAN RUPTURED by %s at room %d",
      	victim->name, ch->name, ch->in_room->vnum);
    	    strtime = ctime(&current_time);
    	    strtime[strlen(strtime)-1] = '\0';
	    sprintf(buf, "%s :: %s\n", strtime, log_buf);
    	    fprintf(fp, "%s", buf);
    	    fclose(fp);
    	    fpReserve = fopen(NULL_FILE, "r");
	  }
	}
    } /* End PK Logger */

    sprintf( log_buf, "%s organ ruptured by %s at %d",victim->name,
	(IS_NPC(ch) ? ch->short_descr : ch->name), ch->in_room->vnum );
    log_string( log_buf );

    return;
  }
  else
  {
    if ( number_percent() < get_skill(ch,gsn_rupture))
    {
      AFFECT_DATA af;
      af.where		= TO_AFFECTS;
      af.type 		= gsn_rupture;
      af.level 		= ch->level;
      af.duration		= number_fuzzy(ch->level / 8);
      af.location		= 0;
      af.modifier		= 0;
      af.bitvector 		= 0;
      affect_to_char(victim,&af);

      act("Your powerful blow ruptured $N's organ, causing internal bleeding.",
            ch, NULL, victim, TO_CHAR );
      act("$n nails you with a hard punch, rupturing an organ and causing internal bleeding!",
            ch, NULL, victim, TO_VICT);
      act("$n nails $N with a powerful blow, causing internal bleeding.",
            ch, NULL, victim, TO_NOTVICT);

      damage(ch,victim,number_range( 10, ch->level * 2 ), gsn_rupture, DAM_BASH, TRUE);
      check_improve(ch, gsn_rupture, TRUE, 1);
    }
    else
    {
      send_to_char("Your powerful blow had no effect.\n\r",ch);
      damage( ch, victim, 0, gsn_rupture,DAM_BASH,TRUE);
      check_improve(ch, gsn_rupture, FALSE, 1);
    }
  WAIT_STATE( ch, skill_table[gsn_rupture].beats );
  }
  return;
}

/*
 * Heroic Demise skill. Leaves victim with 1/10th of their total HP,
 * unless already below that.
 */
bool check_heroic( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance;

    chance = get_skill(victim,gsn_heroic);

    if ( get_eq_char( victim, WEAR_WIELD ) == NULL )
    {
	if (IS_NPC(victim))
	    chance /= 2;
	else
	    return FALSE;
    }

    if (!can_see(ch,victim))
	chance /= 2;

    if ( number_percent( ) >= chance + victim->level - ch->level )
	return FALSE;

    act( "In one final desperate attempt to hurt $n, you lunge forth your weapon and",  ch, NULL, victim, TO_VICT    );
    act( "strike a critical blow, before you fall to your own death!",  ch, NULL, victim, TO_VICT    );
    act( "In one final heroic like move, $N strikes you with a critical blow before.", ch, NULL, victim, TO_CHAR    );
    act( "falling to $S death!", ch, NULL, victim, TO_CHAR    );
    ch->hit = (ch->max_hit/10);
    check_improve(victim,gsn_heroic,TRUE,2);
    return TRUE;
}

void do_yield( CHAR_DATA *ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  int yield;
  CHAR_DATA *rch; 
  OBJ_DATA *obj; 

  one_argument( argument, arg );

  if ( arg[0] != '\0' )
  {
    yield = atoi( arg );

    if ( yield < 0 )
    {
        send_to_char( "To yield with such cowardice bestows only shame.\n\r", ch);
        return;
    }

    if ( yield > ch->max_hit/3 )
    {
        send_to_char( "To yield with such cowardice bestows only shame.\n\r", ch);
        return;
    }

    ch->yield   = yield;
    sprintf( buf, "Yield set to %d hit points.\n\r", yield );
    send_to_char( buf, ch );
    return;
  }            

  if (ch->fighting == NULL)
  {
    send_to_char("You are not fighting anybody.\n\r",ch);
    return;
  }

  if ( !IS_NPC(ch->fighting) 
  &&   !IS_NPC(ch)
  &&   !IS_SET(ch->fighting->act,PLR_AUTOYIELD))
  {
    send_to_char("Your request to yield is refused.\n\r",ch);
    return;
  }

  if ( IS_NPC(ch->fighting) && ch->fighting->master == NULL)
  {
    send_to_char("Your request to yield is refused.\n\r",ch);
    return;
  }

  send_to_char("You yield in defeat.\n\r",ch);
  act("$n yields in defeat.",ch,NULL,NULL,TO_ROOM);  
 
  for ( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
      if(rch->fighting == ch)
         stop_fighting(rch,FALSE);

  stop_fighting(ch,TRUE);

  REMOVE_BIT(ch->act,PLR_AUTOASSIST);

  if (ch->riding)
  {
    act("You dismount $N.",ch,NULL,ch->riding,TO_CHAR); 
    act("$n dismounts from $N.",ch,NULL,ch->riding,TO_ROOM);
    ch->riding->rider = NULL;
    ch->riding        = NULL;
  }

  if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) != NULL 
  && !IS_OBJ_STAT(obj,ITEM_NOREMOVE))
  {
      obj_from_char( obj );
      if ( IS_OBJ_STAT(obj,ITEM_NODROP) || IS_OBJ_STAT(obj,ITEM_INVENTORY) )
      {
        obj_to_char( obj, ch );
        act("You stop using $p and sit in submission.",ch,obj,NULL,TO_CHAR);
        act("$n stops using $p and sits in submission.",ch,obj,NULL,TO_ROOM);
      }
      else
      {
        obj_to_room( obj, ch->in_room );
        act("You drop $p and sit in submission.",ch,obj,NULL,TO_CHAR);
        act("$n drops $p and sits in submission.",ch,obj,NULL,TO_ROOM);
      }
  }
  else
  {
    act("$n sits in submission.",ch,NULL,NULL,TO_ROOM);
    act("You sit in submission.",ch,NULL,NULL,TO_CHAR);
  }

  ch->position = POS_SITTING;

  WAIT_STATE(ch,2 * PULSE_VIOLENCE); 
return;
}

void do_shield_shatter(CHAR_DATA *ch,char *argument)
{
  CHAR_DATA *victim;
  int chance;
  OBJ_DATA *weapon;
  OBJ_DATA *shield;
  
  chance = get_skill(ch,gsn_shield_shatter);

  if (chance == 0)
  {
    send_to_char("Huh?\n\r",ch);
    return;
  }
  
  victim = ch->fighting;
  
  if (victim == NULL)
  {
    send_to_char("But you aren't fighting anyone.\n\r",ch);
    return;
  }

  weapon = get_eq_char(ch,WEAR_WIELD);

  if (weapon == NULL)
  {
    send_to_char("You must be wielding an axe, flail, or mace to shatter your enemy's shield.\n\r", ch);
    return;
  }

  if ((weapon->value[0] != WEAPON_AXE)
  &   (weapon->value[0] != WEAPON_MACE)
  &   (weapon->value[0] != WEAPON_FLAIL))
  {
    send_to_char("You must be wielding an axe, flail, or mace to shatter your enemy's shield.\n\r", ch);
    return;
  }

  if ((weapon->value[0] == WEAPON_AXE)
  &&  (get_skill(ch,gsn_axe) != 100))
  {
    send_to_char("You are not yet skilled enough in axe wielding to shatter an opponants shield. You must master your skill in using axes first.\n\r", ch);
    return;
  }

  if ((weapon->value[0] == WEAPON_MACE)
  &&  (get_skill(ch,gsn_mace) != 100))
  {
    send_to_char("You are not yet skilled enough in mace wielding to shatter an opponants shield. You must master your skill in using maces first.\n\r", ch);
    return;
  }

  if ((weapon->value[0] == WEAPON_FLAIL)
  &&  (get_skill(ch,gsn_flail) != 100))
  {
    send_to_char("You are not yet skilled enough in flail wielding to shatter an opponants shield. You must master your skill in using flails first.\n\r", ch);
    return;
  }
  
  shield = get_eq_char(victim,WEAR_SHIELD);

  if (shield == NULL)
  {
    send_to_char("They aren't holding a shield to shatter.\n\r",ch);
    return;
  }

  if(  str_cmp(shield->material, "redwood")
  &    str_cmp(shield->material, "bone")
  &    str_cmp(shield->material, "cedar")
  &    str_cmp(shield->material, "clay")
  &    str_cmp(shield->material, "ebony")
  &    str_cmp(shield->material, "hickory")
  &    str_cmp(shield->material, "ivory")
  &    str_cmp(shield->material, "mahogony")
  &    str_cmp(shield->material, "maple")
  &    str_cmp(shield->material, "oak")
  &    str_cmp(shield->material, "pine")
  &    str_cmp(shield->material, "rosewood")
  &    str_cmp(shield->material, "vallenwood")
  &    str_cmp(shield->material, "walnut")
  &    str_cmp(shield->material, "wax")
  &    str_cmp(shield->material, "wood") )
  {
    act("You try to shatter $N's shield to pieces, but the vibration knocks you back on your ass. Perhaps you should stick to shattering shields made of softer materials, like wood and ivory.",ch,0,victim,TO_CHAR);
    act("$n tries to shatter your shield to pieces, but fails and the vibrations from hitting your shield knocks $m back on $s ass.",ch,0,victim,TO_VICT);
    act("$n tries to shatter $N's shield to pieces, but fails and only causes a vibration that knocks $m back on $s ass. How embarrasing!",ch,0,victim,TO_NOTVICT);
    ch->position = POS_RESTING;
    return;
  }

  chance /= 2;
  
  chance += weapon->level;
  chance -= shield->level;
  chance -= get_curr_stat(victim,STAT_DEX);
  chance += get_curr_stat(ch,STAT_STR);
  chance -= victim->level;
  chance += ch->level;
  chance = URANGE(1,chance,40);
  
    
  if (number_percent() > chance)
  {
    act("You try to shatter $N's shield to pieces, but fail.",ch,0,victim,TO_CHAR);
    act("$n tries to shatter your shield to pieces, but fails.",ch,0,victim,TO_VICT);
    act("$n tries to shatter $N's shield to pieces, but fails.",ch,0,victim,TO_NOTVICT);
    check_improve(ch,gsn_shield_shatter,FALSE,1);
    WAIT_STATE(ch,24);
    return;
  }

  act("With a loud crack, $p that $n was holding to shield $mself, splinters and shatters!",victim,shield,0,TO_ROOM);
  act("$n strikes your shield hard and with a sharp crack it shatters!",ch,0,victim,TO_VICT);
  extract_obj(shield);
  check_improve(ch,gsn_shield_shatter,TRUE,2);
  WAIT_STATE(ch,18);
  return;
}

bool check_arrow_defl( CHAR_DATA *ch, CHAR_DATA *victim )
{
  OBJ_DATA *shield;
  int chance;

  shield = get_eq_char(victim,WEAR_SHIELD);

  if(IS_NPC(victim))
    return FALSE;

  if(shield == NULL)
    return FALSE;

  if(!victim->fighting)
    return FALSE;

  if(!IS_AWAKE(victim))
    return FALSE;

  chance  = victim->pcdata->learned[gsn_arrow_deflection] / 2;

  chance -= get_curr_stat(victim, STAT_DEX)/4;
  chance += ch->hitroll /9 + (ch->level <=5) + (ch->level <= 10);  
  if ( number_percent( ) >= chance )
    return FALSE;

  act( "You see an arrow coming towards you and quickly put your shield up, deflecting $n's arrow from harms way.", ch, NULL, victim, TO_VICT);
  act( "$N puts up $S shield in a very quick manner, deflecting your arrow harmlessly away from $M.", ch, NULL, victim, TO_CHAR);
  check_improve(victim,gsn_arrow_deflection,TRUE,6);
  return TRUE;
}

