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
#include <time.h>
#include "merc.h"
#include "interp.h"
#include "music.h"
#include "recycle.h"

/*
 * Local functions.
 */
int   hit_gain	 	args( ( CHAR_DATA *ch ) );
int   mana_gain	 	args( ( CHAR_DATA *ch ) );
int   move_gain	 	args( ( CHAR_DATA *ch ) );
void  mobile_update	args( ( void ) );
void  health_dam      	args( ( CHAR_DATA *ch) );
void  weather_update	args( ( void ) );
void  char_update	args( ( void ) );
void  obj_update	args( ( void ) );
void  check_death	args( ( CHAR_DATA *ch ) );
void  aggr_update	args( ( void ) );
void  quest_update 	args( ( void ) );
void  to_temple		args( (CHAR_DATA *ch ) );
void  raw_kill		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void  reg_mend		args( (CHAR_DATA *ch) );
void  regen_limb	args( (CHAR_DATA *ch) );
void  are_balance	args( (CHAR_DATA *ch) );
void  are_corrupt	args( (CHAR_DATA *ch) );
void  are_sanctify	args( (CHAR_DATA *ch) );

/* used for saving */

int	save_number = 0;

/*
 * Advancement stuff.
 */
void advance_level( CHAR_DATA *ch, bool hide )
{
    char buf[MAX_STRING_LENGTH];
    int add_hp;
    int add_mana;
    int add_move;
    int add_prac;

    ch->pcdata->last_level = 
	( ch->played + (int) (current_time - ch->logon) ) / 3600;

    add_hp	= con_app[get_curr_stat(ch,STAT_CON)].hitp + number_range(
		    class_table[ch->class].hp_min,
		    class_table[ch->class].hp_max );
    add_mana	= int_app[get_curr_stat(ch,STAT_INT)].mana_bonus + number_range(
		    class_table[ch->class].mana_min,
		    class_table[ch->class].mana_max );

    if (!class_table[ch->class].fMana)
	add_mana /= 2;
    add_move	= number_range( 1, (get_curr_stat(ch,STAT_CON)
				  + get_curr_stat(ch,STAT_DEX))/6 );
    add_prac	= wis_app[get_curr_stat(ch,STAT_WIS)].practice;

    add_hp = add_hp * 9/10;
    add_mana = add_mana * 9/10;
    add_move = add_move * 9/10;

    add_hp	= UMAX(  2, add_hp   );
    add_mana	= UMAX(  2, add_mana );
    add_move	= UMAX(  6, add_move );

    ch->max_hit 	+= add_hp;
    ch->max_mana	+= add_mana;
    ch->max_move	+= add_move;
    ch->practice	+= add_prac;

    /* only give them a train every 5 levels */
    if((ch->level % 5) == 0)
    	ch->train		+= 1;

    ch->pcdata->perm_hit	+= add_hp;
    ch->pcdata->perm_mana	+= add_mana;
    ch->pcdata->perm_move	+= add_move;

    if (IS_SET(ch->act, PLR_NOTITLE))
    {
    sprintf( buf, "the %s",
    title_table [ch->class] [ch->level] [ch->pcdata->true_sex == SEX_FEMALE ? 1 : 0] );
    set_title( ch, buf );
    }

    if (!hide)
    {
    	sprintf(buf,
	    "You gain %d hit point%s, %d mana, %d move, and %d practice%s.\n\r",
	    add_hp, add_hp == 1 ? "" : "s", add_mana, add_move,
	    add_prac, add_prac == 1 ? "" : "s");
        send_to_char( buf, ch );
    }

    ch->hit 	= ch->max_hit;
    ch->mana	= ch->max_mana;
    ch->move	= ch->max_move;

    if(ch->level > 5 && ch->in_room != NULL 
    && IS_SET(ch->in_room->room_flags, ROOM_NEWBIES_ONLY) )
    {
	send_to_char("The gods whisk you away to your hometown.\n\r",ch);

	if( get_room_index(hometown_table[ch->hometown].recall) != NULL )
		sprintf( buf, "%s %d", ch->name, hometown_table[ch->hometown].recall );
	else
		sprintf( buf, "%s %d", ch->name, 3054 );

	do_function(ch, &do_transfer, buf);
    }

    return;
}   



void gain_exp( CHAR_DATA *ch, int gain )
{
    char buf[MAX_STRING_LENGTH];

    if ( IS_SET(ch->act, PLR_NOEXP))
      return;

    if ( IS_NPC(ch) || ch->level >= LEVEL_HERO || ch->level == LEVEL_HERO -1 )
	return;

    /* keep xp from dropping below starting value */
    ch->exp = UMAX( exp_at_level(ch, 1), ch->exp + gain );

    /* now see if they raise up */
    while ( ch->level < LEVEL_HERO - 1
     && ch->exp >= exp_at_level(ch, ch->level + 1) )
    {
	send_to_char( "You raise a level!!  ", ch );
	ch->level += 1;
	sprintf(buf,"%s gained level %d",ch->name,ch->level);
	log_string(buf);
	sprintf(buf,"$N has attained level %d!",ch->level);
	wiznet(buf,ch,NULL,WIZ_LEVELS,0,0);
	advance_level(ch,FALSE);
	save_char_obj(ch);
    }

    return;
}



/*
 * Regeneration stuff.
 */
int hit_gain( CHAR_DATA *ch )
{
    int gain;
    int number;
    OBJ_DATA *obj;

    if (ch->in_room == NULL)
	return 0;

    if ( IS_NPC(ch) )
    {
	gain =  5 + ch->level;
 	if (IS_AFFECTED(ch,AFF_REGENERATION))
	    gain *= 2;

	switch(ch->position)
	{
	    default : 		gain /= 2;			break;
	    case POS_SLEEPING: 	gain = 3 * gain/2;		break;
	    case POS_RESTING:  					break;
	    case POS_FIGHTING:	gain /= 3;		 	break;
 	}

	
    }
    else
    {
	gain = UMAX(3,get_curr_stat(ch,STAT_CON) - 3 + ch->level/2); 
	gain += class_table[ch->class].hp_max - 10;
 	number = number_percent();
	if (number < get_skill(ch,gsn_fast_healing))
	{
	    gain += number * gain / 100;
	    if (ch->hit < ch->max_hit)
		check_improve(ch,gsn_fast_healing,TRUE,8);
	}

	switch ( ch->position )
	{
	    default:	   	gain /= 4;			break;
	    case POS_SLEEPING: 
		if (ch->on && ch->on->item_type == ITEM_FURNITURE)
		{
		  gain += (ch->on->value[3] * 12);
		}
		else
		{
              gain /=1;
		}
		break;
	    case POS_RESTING:  	
		if (ch->on && ch->on->item_type == ITEM_FURNITURE)
		{
		  gain += (ch->on->value[3] * 6);
		}
		else
		{
		  gain /= 2;
		}
		break;
	    case POS_FIGHTING: 	gain /= 6;			break;
	}

	if ( (ch->pcdata->condition[COND_FULL] <= 10)
      &&   (ch->pcdata->condition[COND_FULL] > 0) )
	    gain /= 2;

	if ( (ch->pcdata->condition[COND_THIRST] <= 10)
      &&   (ch->pcdata->condition[COND_THIRST] > 0) )
	    gain /= 2;

    }

    gain = gain * ch->in_room->heal_rate / 100;

    if ( IS_AFFECTED(ch, AFF_POISON) )
	gain /= 4;

    if (IS_AFFECTED(ch, AFF_PLAGUE))
	gain /= 8;

    if (ch->in_room != NULL)
     {

    if (( IS_AFFECTED(ch, AFF_WARMTH)
    && (( ch->in_room->sector_type == SECT_FOREST)
    || (  ch->in_room->sector_type == SECT_FIELD)
    || (  ch->in_room->sector_type == SECT_DESERT)
    || (  ch->in_room->sector_type == SECT_HILLS)
    || (  ch->in_room->sector_type == SECT_MOUNTAIN)))
    && (!IS_AFFECTED(ch, AFF_REGENERATION)))
        gain *=2 ;

    }

    if (IS_AFFECTED(ch,AFF_HASTE))
	gain /=2 ;

    if (IS_AFFECTED(ch,AFF_SLOW))
	gain *=2 ;

    if (IS_AFFECTED(ch,AFF_REGENERATION))
	gain *=2 ;

   if (IS_AFFECTED2(ch, AFF_WASTING))
       gain /= 2;   

   if (ch->in_room != NULL)
   {
    for (obj = ch->in_room->contents; obj != NULL; obj = obj->next_content)    
    {
       if (obj->pIndexData->vnum == OBJ_VNUM_CAMPFIRE)
       {
          gain += gain/2;
          break;
       }
    }
   }

   if ( (ch->in_room->sector_type == SECT_UNDERWATER)
   && (!IS_AFFECTED(ch, AFF_WATERBREATHING))
   && (!IS_SET(ch->imm_flags, IMM_DROWNING))
   && (ch->level <= LEVEL_HERO )
   && !IS_NPC(ch))
   {
	send_to_char("You swallow a huge gulp of water!\n\r", ch);
	act("$n tries to take a breath of water.", ch, NULL, NULL, TO_ROOM);
	if(IS_SET(ch->vuln_flags, VULN_DROWNING))
		return (0 - gain*2);
	if(IS_SET(ch->res_flags, RES_DROWNING))
		return (0 - gain/2);
	return (0 - gain * 3 / 2);
    }

    if ( ch->in_room && (ch->in_room->sector_type == SECT_WATER_NOSWIM)
    && !IS_AFFECTED(ch,AFF_FLYING)
    && !IS_AFFECTED2(ch, AFF_WRAITHFORM))
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
      if ( !found
      && (!IS_AFFECTED(ch, AFF_WATERBREATHING))
      && (!IS_SET(ch->imm_flags, IMM_DROWNING))
      && (ch->level <= LEVEL_HERO)
	&& !IS_NPC(ch))
      {
	send_to_char("You swallow a huge gulp of water!\n\r", ch);
	act("$n tries to take a breath of water.", ch, NULL, NULL, TO_ROOM);
	if(IS_SET(ch->vuln_flags, VULN_DROWNING))
		return (0 - gain*2);
	if(IS_SET(ch->res_flags, RES_DROWNING))
		return (0 - gain/2);
	return (0 - gain * 3 / 2);
       }
    }

    if (IS_AFFECTED3(ch, AFF_SUBDUE)
    && gain < 0)
	return 0;

    return UMIN(gain, ch->max_hit - ch->hit);
}



int mana_gain( CHAR_DATA *ch )
{
    int gain;
    int number;

    if (ch->in_room == NULL)
	return 0;

    if ( IS_NPC(ch) )
    {
	gain = 5 + ch->level;
	switch (ch->position)
	{
	    default:		gain /= 2;		break;
	    case POS_SLEEPING:	gain = 3 * gain/2;	break;
   	    case POS_RESTING:				break;
	    case POS_FIGHTING:	gain /= 3;		break;
    	}
    }
    else
    {
	gain = (get_curr_stat(ch,STAT_WIS) 
	      + get_curr_stat(ch,STAT_INT) + ch->level) / 2;
	number = number_percent();
	if (number < ch->pcdata->learned[gsn_meditation]
	   && number > ch->pcdata->learned[gsn_trance])
	{
	    gain += number * gain / 100;
	    if (ch->mana < ch->max_mana)
	        check_improve(ch,gsn_meditation,TRUE,8);
	}
	if (number < ch->pcdata->learned[gsn_meditation]
	   && number < ch->pcdata->learned[gsn_trance])
 	{
	    gain += number * gain /50;
	    if (ch->mana < ch->max_mana)
		check_improve(ch,gsn_trance,TRUE,8);
		check_improve(ch,gsn_meditation,TRUE,8);
	}
	if (!class_table[ch->class].fMana)
	    gain /= 2;

	if (class_table[ch->class].mana_min < 3)
	    gain /= 2;

	switch ( ch->position )
	{
	    default:		gain /= 4;			break;
	    case POS_SLEEPING: 
		if (ch->on && ch->on->item_type == ITEM_FURNITURE)
		{
		  gain += (ch->on->value[3] * 10);
		}
		else
		{
              gain /=1;
		}
		break;
	    case POS_RESTING:  	
		if (ch->on && ch->on->item_type == ITEM_FURNITURE)
		{
		  gain += (ch->on->value[3] * 5);
		}
		else
		{
		  gain /= 2;
		}
		break;
	    case POS_FIGHTING:	gain /= 6;			break;
	}

	if ( (ch->pcdata->condition[COND_FULL] <= 10)
      &&   (ch->pcdata->condition[COND_FULL] > 0) )
	    gain /= 2;

	if ( (ch->pcdata->condition[COND_THIRST] <= 10)
      &&   (ch->pcdata->condition[COND_THIRST] > 0) )
	    gain /= 2;

    }

    gain = gain * ch->in_room->mana_rate / 100;

    if ( IS_AFFECTED( ch, AFF_POISON ) )
	gain /= 4;

    if (IS_AFFECTED(ch, AFF_PLAGUE))
        gain /= 8;

    if (IS_AFFECTED(ch,AFF_HASTE) || IS_AFFECTED(ch,AFF_SLOW))
        gain /=2 ;

    if (IS_AFFECTED(ch,AFF_REGENERATION))
	gain *=2 ;

   if (IS_AFFECTED2(ch, AFF_WASTING))
       gain /= 2; 

   if (IS_AFFECTED3(ch, AFF_SUBDUE)
   && gain < 0)
     return 0;

   return UMIN(gain, ch->max_mana - ch->mana);
}

int move_gain( CHAR_DATA *ch )
{
    int gain;

    if (ch->in_room == NULL)
	return 0;

    if ( IS_NPC(ch) )
    {
	gain = ch->level;
    }
    else
    {
	gain = UMAX( 15, ch->level );

	switch ( ch->position )
	{
	case POS_SLEEPING: gain += get_curr_stat(ch,STAT_DEX);		break;
	case POS_RESTING:  gain += get_curr_stat(ch,STAT_DEX) / 2;	break;
	}

	if ( ch->pcdata->condition[COND_FULL]   <= 10 )
	    gain /= 2;

	if ( ch->pcdata->condition[COND_THIRST] <= 10 )
	    gain /= 2;
    }

    gain = gain * ch->in_room->heal_rate/100;

    if ( IS_AFFECTED(ch, AFF_POISON) )
	gain /= 4;

    if (IS_AFFECTED(ch, AFF_PLAGUE))
        gain /= 8;

    if (IS_AFFECTED(ch,AFF_HASTE) || IS_AFFECTED(ch,AFF_SLOW))
        gain /=2 ;

    if (IS_AFFECTED(ch,AFF_REGENERATION))
	gain *=2 ;

    if (IS_AFFECTED2(ch, AFF_WASTING))
       gain /= 2;

    if (IS_AFFECTED3(ch, AFF_SUBDUE)
    && gain < 0)
      return 0;

    return UMIN(gain, ch->max_move - ch->move);
}


void gain_condition( CHAR_DATA *ch, int iCond, int value )
{
    int condition;
    int dam;

    if ( value == 0 
    || IS_NPC(ch) 
    || ch->level >= LEVEL_AVATAR 
    || IS_SET(ch->comm, COMM_NOIDLE)
    || (ch->in_room == get_room_index(1280)))
	return;

    condition				= ch->pcdata->condition[iCond];
    if (condition == -1)
        return;
    ch->pcdata->condition[iCond]	= URANGE( 0, condition + value, 65 );

    if ( ch->pcdata->condition[iCond] == 0 )
    {
	switch ( iCond )
	{
	case COND_DRUNK:
	    if ( condition != 0 )
		send_to_char( "You are sober.\n\r", ch );
	    break;

	case COND_FULL:
          if (!IS_AFFECTED(ch, AFF_SUBDUE))
          {
	      dam = ch->level / 2;
	      send_to_char( "You are starving to death.\n\r",  ch );
	      damage(ch,ch,dam,0,DAM_NONE,FALSE);
	    }
	    break;

	case COND_THIRST:
          if (!IS_AFFECTED(ch, AFF_SUBDUE))
          {
	      dam = ch->level / 2;
	      send_to_char( "You are dying of thirst.\n\r", ch );
	      damage(ch,ch,dam,0,DAM_NONE,FALSE);
          }
	    break;

	}
    }

    return;
}



/*
 * Mob autonomous action.
 * This function takes 25% to 35% of ALL Merc cpu time.
 * -- Furey
 */
void mobile_update( void )
{
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;
    EXIT_DATA *pexit;
    int door;

    /* Examine all mobs. */
    for ( ch = char_list; ch != NULL; ch = ch_next )
    {
	ch_next = ch->next;

      if (IS_NPC(ch)
      &&  !IS_AFFECTED( ch, AFF_SLEEP)
      &&  !IS_AFFECTED( ch, AFF_CHARM)
      &&  ch->hunting != NULL
      &&  (ch->position == POS_RESTING || ch->position == POS_SITTING))
         ch->position = POS_STANDING;

      if ( !IS_NPC(ch) || !ch->in_room /*|| IS_AFFECTED(ch,AFF_CHARM)*/)
	    continue;

      if (ch->in_room->area->empty && !IS_SET(ch->act,ACT_UPDATE_ALWAYS))
	    continue;

	/* Examine call for special procedure */
      if ( ch->spec_fun != 0 )
      {
	    if(ch->spec_fun == spec_lookup("spec_rloader"))
		continue;

	    if ( (*ch->spec_fun) ( ch ) )
		continue;
      }

	if (ch->pIndexData->pShop != NULL) /* give him some steel */
	    if ((ch->steel * 100 + ch->gold) < ch->pIndexData->wealth)
	{
		ch->steel += ch->pIndexData->wealth * number_range(1,20)/5000000;
		ch->gold += ch->pIndexData->wealth * number_range(1,20)/50000;
	}
	 
	/*
	 * Check triggers only if mobile still in default position
	 */
	if ( ch->position == ch->pIndexData->default_pos )
	{
	    /* Delay */
	    if ( HAS_TRIGGER_MOB( ch, TRIG_DELAY) 
	    &&   ch->mprog_delay > 0 )
	    {
		if ( --ch->mprog_delay <= 0 )
		{
		    p_percent_trigger( ch, NULL, NULL, NULL, NULL, NULL, TRIG_DELAY );
		    continue;
		}
	    } 
	    if ( HAS_TRIGGER_MOB( ch, TRIG_RANDOM) )
	    {
		if( p_percent_trigger( ch, NULL, NULL, NULL, NULL, NULL, TRIG_RANDOM ) )
		continue;
	    }
	}

	/* That's all for sleeping / busy monster, and empty zones */
	if ( ch->position != POS_STANDING )
	    continue;

      if ( !IS_AFFECTED(ch, AFF_CHARM)
      && ( ch->leader != NULL)
      && ( ch->leader != ch))
      {
           stop_follower(ch);
           if ( ch->position == POS_FIGHTING)
              ch->position = POS_STANDING;
      }

    	if  ( ch->position == POS_SLEEPING 
    	&&  !IS_AFFECTED( ch, AFF_SLEEP)
    	&&  !IS_AFFECTED( ch, AFF_CHARM))
        	do_stand(ch, "");

        /*
         * Update tracking mobs
         */
	if (ch->hunting != NULL && ch->fighting == NULL
	&& number_bits(1) == 0)
	{
	    CHAR_DATA *vict;

            do_stand(ch," ");

	    if (ch->hit < ch->max_hit * .2)
	    {
		ch->hunting = NULL;
	    }
	    else if ((vict = get_char_world(ch, ch->hunting->name)) == NULL)
	    {
		if (number_percent() < 10)
		    ch->hunting = NULL;
	    }
	    else if (ch->hunting != NULL)
	    {
            if ( ch->in_room == vict->in_room )
            {
              if( IS_AFFECTED2(vict, AFF_INVUL) 
              ||  vict->position == POS_DEAD
              ||  IS_AFFECTED3(vict, AFF_SUBDUE))
              {
                ch->hunting = NULL;
              }
            }
	        hunt_victim(ch);
	    	if (vict->in_room == ch->in_room && ch->hunting != NULL)
		{
		    char buf[MSL];

		    sprintf(buf, "glare %s", vict->name);
		    interpret(ch, buf);
		    do_kill(ch, vict->name);
		}
		else
		{
		    interpret(ch, "peer");
		}
	    }
	    else
	    {
	        ch->hunting = NULL;
	    }
	}

	/* Scavenge */
	if ( IS_SET(ch->act, ACT_SCAVENGER)
	&&   ch->in_room->contents != NULL
      && !IS_AFFECTED(ch,AFF_BLIND)
	&&   number_bits( 6 ) == 0 )
	{
	    CHAR_DATA *gch;
	    OBJ_DATA *obj;
	    OBJ_DATA *obj_best;
	    int max;

	    max         = 1;
	    obj_best    = 0;
	    for ( obj = ch->in_room->contents; obj; obj = obj->next_content )
	    {

            if (obj->in_room != NULL)
            {
	        for (gch = obj->in_room->people; gch != NULL; gch = gch->next_in_room)
	        if (gch->on == obj)
		    continue;
            }

		if ( CAN_WEAR(obj, ITEM_TAKE)
            && can_loot(ch, obj)
		&& obj->cost > max
            && !IS_SET(obj->extra_flags, ITEM_QUESTOBJ)
		&& (obj->item_type != ITEM_CORPSE_PC)
            && obj->cost > 0)
		{
		    obj_best    = obj;
		    max         = obj->cost;
		}
	    }

	    if ( obj_best )
	    {
		obj_from_room( obj_best );
		obj_to_char( obj_best, ch );
		act( "$n gets $p.", ch, obj_best, NULL, TO_ROOM );
	    }
	}

	/* Wander */
	if ( number_bits(4) == 0
	&& ( door = number_bits( 5 ) ) <= 5
	&& ( pexit = ch->in_room->exit[door] ) 
	&&   pexit->u1.to_room 
	&&   !IS_SET(pexit->exit_info, EX_CLOSED)
	&&   !IS_SET(pexit->u1.to_room->room_flags, ROOM_NO_MOB)
	&& (((IS_SET(pexit->u1.to_room->sector_type, SECT_UNDERWATER) ||
	      IS_SET(pexit->u1.to_room->sector_type, SECT_WATER_SWIM) ||
	      IS_SET(pexit->u1.to_room->sector_type, SECT_WATER_NOSWIM))
	&&   (IS_SET(ch->act, ACT_INDOORS)))
	||  (!IS_SET(ch->act, ACT_INDOORS)))
	&&  (!IS_SET(ch->act, ACT_STAY_AREA)
	||   pexit->u1.to_room->area == ch->in_room->area ) 
	&& ( !IS_SET(ch->act, ACT_SENTINEL) ) 
	&& (!ch->tethered)
      && (ch->fighting == NULL)
	&& !IS_SET(pexit->u1.to_room->sector_type, SECT_SOLAMNIC)
	&& !IS_SET(pexit->u1.to_room->sector_type, SECT_KOT)
	&& !IS_SET(pexit->u1.to_room->sector_type, SECT_CONCLAVE)
	&& !IS_SET(pexit->u1.to_room->sector_type, SECT_AESTHETIC)
	&& !IS_SET(pexit->u1.to_room->sector_type, SECT_THIEVES)
	&& !IS_SET(pexit->u1.to_room->sector_type, SECT_HOLYORDER)
	&& !IS_SET(pexit->u1.to_room->sector_type, SECT_MERCENARY)
	&& !IS_SET(pexit->u1.to_room->sector_type, SECT_UNDEAD)
	&& !IS_SET(pexit->u1.to_room->sector_type, SECT_FORESTER)
	&& !IS_SET(pexit->u1.to_room->sector_type, SECT_ARTISANS)
	&& !IS_SET(pexit->u1.to_room->sector_type, SECT_BLACKORDER))
	{
	    move_char( ch, door, FALSE );
	}

    }

    return;
}

/*
 * Update the weather.
 */
void weather_update( void )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
   // OBJ_DATA *puddle;
    int diff;

    buf[0] = '\0';

    switch ( ++time_info.hour )
    {

    case  6:
	weather_info.sunlight = SUN_LIGHT;
	strcat( buf, "The day has begun.\n\r" );
	break;

    case  7:
	weather_info.sunlight = SUN_RISE;
	strcat( buf, "The sun rises in the east just as the black moon, Nuitari, disappears.\n\r" );
	break;

    case 19:
	weather_info.sunlight = SUN_SET;
	strcat( buf, "The sun slowly disappears in the west.\n\r" );
	break;

    case 20:
	weather_info.sunlight = SUN_DARK;
	strcat( buf, "The night has begun, as the Silver rays of Solinari fill the sky.\n\r" );
	break;

    case 24:
	time_info.hour = 0;
	time_info.day++;
	break;
    }

    if ( time_info.day   >= 35 )
    {
	time_info.day = 0;
	time_info.month++;
    }

    if ( time_info.month >= 12 )
    {
	time_info.month = 0;
	time_info.year++;
    }

    /*
     * Weather change.
     */
    if ( time_info.month >= 9 && time_info.month <= 11 )
	diff = weather_info.mmhg >  985 ? -2 : 2;
    else
	diff = weather_info.mmhg > 1015 ? -2 : 2;

    weather_info.change   += diff * dice(1, 4) + dice(2, 6) - dice(2, 6);
    weather_info.change    = UMAX(weather_info.change, -12);
    weather_info.change    = UMIN(weather_info.change,  12);

    weather_info.mmhg += weather_info.change;
    weather_info.mmhg  = UMAX(weather_info.mmhg,  960);
    weather_info.mmhg  = UMIN(weather_info.mmhg, 1040);

    switch ( weather_info.sky )
    {
    default: 
	bug( "Weather_update: bad sky %d.", weather_info.sky );
	weather_info.sky = SKY_CLOUDLESS;
	break;

    case SKY_CLOUDLESS:
	if ( weather_info.mmhg <  990
	|| ( weather_info.mmhg < 1010 && number_bits( 2 ) == 0 ) )
	{
	    strcat( buf, "The sky is getting cloudy.\n\r" );
	    weather_info.sky = SKY_CLOUDY;
	}
	break;

    case SKY_CLOUDY:
	if ( weather_info.mmhg <  970
	|| ( weather_info.mmhg <  990 && number_bits( 2 ) == 0 ) )
	{
         // for ( d = descriptor_list; d != NULL; d = d->next )
         // {
         //   if((number_percent() > 25)
         //   &&  IS_OUTSIDE(d->character))
         //   {
         //   puddle =  create_object( get_obj_index(OBJ_VNUM_PUDDLE), d->character->level + 10);
         //   obj_to_room( puddle, d->character->in_room );
         //   puddle->timer = 4;
         //   }
         // }
	    strcat( buf, "It starts to rain.\n\r" );
	    weather_info.sky = SKY_RAINING;
	}

	if ( weather_info.mmhg > 1030 && number_bits( 2 ) == 0 )
	{
	    strcat( buf, "The clouds disappear.\n\r" );
	    weather_info.sky = SKY_CLOUDLESS;
	}
	break;

    case SKY_RAINING:
	if ( weather_info.mmhg <  970 && number_bits( 2 ) == 0 )
	{
	    strcat( buf, "Lightning flashes in the sky.\n\r" );
	    weather_info.sky = SKY_LIGHTNING;
	}

	if ( weather_info.mmhg > 1030
	|| ( weather_info.mmhg > 1010 && number_bits( 2 ) == 0 ) )
	{
	    strcat( buf, "The rain stopped.\n\r" );
	    weather_info.sky = SKY_CLOUDY;
	}
	break;

    case SKY_LIGHTNING:
	if ( weather_info.mmhg > 1010
	|| ( weather_info.mmhg >  990 && number_bits( 2 ) == 0 ) )
	{
	    strcat( buf, "The lightning has stopped.\n\r" );
	    weather_info.sky = SKY_RAINING;
	    break;
	}
	break;
    }

    if ( buf[0] != '\0' )
    {
	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->connected == CON_PLAYING
	    &&   IS_OUTSIDE(d->character)
	    &&   IS_AWAKE(d->character) )
		send_to_char( buf, d->character );
	}
    }

    return;
}

/*
 * Update all chars, including mobs.
*/
void char_update( void )
{   
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;
    CHAR_DATA *ch_quit;
    int xp_to_lev;

    bool is_obj;

    ch_quit	= NULL;

    /* update save counter */
    save_number++;

    if (save_number > 29)
	save_number = 0;

    for ( ch = char_list; ch != NULL; ch = ch_next )
    {
	AFFECT_DATA *paf;
	AFFECT_DATA *paf_next;

	ch_next = ch->next;

	if (!IS_NPC(ch) && (IS_SET(ch->loc_hp,LOST_HEAD)))
	  is_obj = TRUE;
	else
	  is_obj = FALSE;

      if ( ch->timer > 20 )
        ch_quit = ch;

      reg_mend(ch);

	if ( ch->position >= POS_STUNNED )
	{
            /* check to see if we need to go home */
        if (IS_NPC(ch)
        && ch->zone != NULL
        && ch->zone != ch->in_room->area
        && ch->desc == NULL 
        &&  ch->fighting == NULL 
	  && !IS_AFFECTED(ch,AFF_CHARM)
        && number_percent() < 5)
        {
          act("$n wanders on home.",ch,NULL,NULL,TO_ROOM);
          extract_char(ch,TRUE);
          continue;
        }

	  if ( ch->hit  < ch->max_hit )
	    ch->hit  += hit_gain(ch);
	  else
	    ch->hit = ch->max_hit;

	  if ( ch->mana < ch->max_mana )
	    ch->mana += mana_gain(ch);
	  else
	    ch->mana = ch->max_mana;

	  if ( ch->move < ch->max_move )
	    ch->move += move_gain(ch);
	  else
	    ch->move = ch->max_move;
	}

      if (!IS_NPC(ch))
        health_dam(ch);

	if ( ch->position == POS_STUNNED )
	  update_pos( ch );

	ch->quit_timer++;

  if(ch->desc != NULL && ch->desc->connected == CON_PLAYING)
  {
      if (IS_SET(ch->in_room->room_flags, ROOM_CORRUPT))
	  are_corrupt( ch );

      if (IS_SET(ch->in_room->room_flags, ROOM_SANCTIFY))
	  are_sanctify( ch );

      if (IS_SET(ch->in_room->room_flags, ROOM_BALANCE))
	  are_balance( ch );
  }

      if (!IS_NPC(ch))
          check_death(ch);

 	if (ch->mesg_delay && ch->mesg_buf != NULL)
	{
  	    CHAR_DATA *gch;
	    char *buf = ch->mesg_buf;
            char arg[512];
	    int org = 0;

	    ch->mesg_delay--;

	    if ((IS_FORESTER(ch)) && (ch->mesg_delay > 0 ))
		for (gch = char_list; gch != NULL; gch = gch->next )
		   if ((IS_FORESTER(gch)) && (gch != ch))
		   {
			if (IS_SET(gch->in_room->room_flags, ROOM_INDOORS))
			   act("One of your contacts signals that a {ymessage is en route.{x",
			   gch, NULL, NULL, TO_CHAR);
			else
			   act("You see a {ypigeon circling overhead.{x",
			   gch, NULL, NULL, TO_CHAR);
		   }

	    if ((IS_HOLYORDER(ch)) && (ch->mesg_delay > 0 ))
		for (gch = char_list; gch != NULL; gch = gch->next )
		   if ((IS_HOLYORDER(gch)) && (gch != ch))
		   {
			if (IS_SET(gch->in_room->room_flags, ROOM_INDOORS))
			   act("One of your contacts signals that a {ymessage is en route.{x",
			   gch, NULL, NULL, TO_CHAR);
			else
			   act("You see a {yyoung cleric running towards you.{x",
			   gch, NULL, NULL, TO_CHAR);
		   }

	    if ((IS_UNDEAD(ch)) && (ch->mesg_delay > 0 ))
		for (gch = char_list; gch != NULL; gch = gch->next )
		   if ((IS_UNDEAD(gch)) && (gch != ch))
		   {
			if (IS_SET(gch->in_room->room_flags, ROOM_INDOORS))
			   act("One of your contacts signals that a {ymessage is en route.{x",
			   gch, NULL, NULL, TO_CHAR);
			else
			   act("You see a {ydemonic knight stepping out of the shadows.{x",
			   gch, NULL, NULL, TO_CHAR);
		   }

	    if ((IS_ARTISANS(ch)) && (ch->mesg_delay > 0 ))
		for (gch = char_list; gch != NULL; gch = gch->next )
		   if ((IS_ARTISANS(gch)) && (gch != ch))
		   {
			if (IS_SET(gch->in_room->room_flags, ROOM_INDOORS))
			   act("One of your contacts signals that a {ymessage is en route.{x",
			   gch, NULL, NULL, TO_CHAR);
			else
			   act("You see a {yblacksmith running towards you.{x",
			   gch, NULL, NULL, TO_CHAR);
		   }

	    if ((IS_AESTHETIC(ch)) && (ch->mesg_delay > 0 ))
		for (gch = char_list; gch != NULL; gch = gch->next )
		   if ((IS_AESTHETIC(gch)) && (gch != ch))
		   {
			if (IS_SET(gch->in_room->room_flags, ROOM_INDOORS))
			   act("One of your contacts signals that a {ymessage is en route.{x",
			   gch, NULL, NULL, TO_CHAR);
			else
			   act("You see a {yhawk circling overhead.{x",
			   gch, NULL, NULL, TO_CHAR);
		   }

	    if ((IS_BLACKORDER(ch)) && (ch->mesg_delay > 0 ))
		for (gch = char_list; gch != NULL; gch = gch->next )
		   if ((IS_BLACKORDER(gch)) && (gch != ch))
		   {
			if (IS_SET(gch->in_room->room_flags, ROOM_INDOORS))
			   act("One of your contacts signals that a {ymessage is en route.{x",
			   gch, NULL, NULL, TO_CHAR);
			else
			   act("You see an {yunholy monk walking briskly towards you.{x",
			   gch, NULL, NULL, TO_CHAR);
		   }

	    if (ch->mesg_delay == 0)
	    {
		buf = one_argument(buf, arg);

		if (!str_cmp(arg, "thieves"))
		    org = ORG_THIEVES;
		else if (!str_cmp(arg, "kot"))
		    org = ORG_KOT;
            else if (!str_cmp(arg, "mercenary")) 
		    org = ORG_MERCENARY;
            else if (!str_cmp(arg, "solamnic")) 
		    org = ORG_SOLAMNIC;
            else if (!str_cmp(arg, "forester")) 
		    org = ORG_FORESTER;
            else if (!str_cmp(arg, "conclave")) 
		    org = ORG_CONCLAVE;
            else if (!str_cmp(arg, "holyorder")) 
		    org = ORG_HOLYORDER;
            else if (!str_cmp(arg, "artisans")) 
		    org = ORG_ARTISANS;
            else if (!str_cmp(arg, "aesthetic")) 
		    org = ORG_AESTHETIC;
            else if (!str_cmp(arg, "blackorder")) 
		    org = ORG_BLACKORDER;
            else if (!str_cmp(arg, "undead")) 
		    org = ORG_UNDEAD;

		for (gch = char_list ; gch != NULL ; gch = gch->next)
	 	{
		    if (!org)
			break;

		    if (IS_NPC(gch) || gch->position <= POS_SLEEPING)
			continue;

		    if (org == ORG_THIEVES 
		    && (ORG(gch->org_id) == ORG_THIEVES))
		    {
			if ( buf == NULL )
			    break;

			one_argument( buf, arg );
			arg[0] = UPPER(arg[0]);

			if ( !str_cmp(gch->name, arg))
			    continue;

			act("A shadowy figure approaches you.", gch, NULL, NULL, TO_CHAR);
			act("He whispers {y'I bear a message from $t'{x", gch, arg, NULL, TO_CHAR);
			act("It reads: {G$t{x", gch, buf+strlen(arg)+1, NULL, TO_CHAR);
			act("The figure turns away and disappears.",gch, NULL, NULL, TO_CHAR);
		    }
		    else if (org == ORG_KOT
		    && (ORG(gch->org_id) == ORG_KOT))
		    {
			if ( buf == NULL )
			    break;

			one_argument( buf, arg );
			arg[0] = UPPER(arg[0]);

			if ( !str_cmp(gch->name, arg))
			    continue;

			act("A courier on horseback approaches $n.",
			gch, NULL, NULL, TO_ROOM);
			act("He shouts {y'I bear a message for $n!'{x",
			gch, NULL, NULL, TO_ROOM);
			act("The courier salutes $n, and gallops away.",
			gch, NULL, NULL, TO_ROOM);

			act("A courier on horseback approaches you.", 
			gch, NULL, NULL, TO_CHAR);
			act("He shouts {y'I bear a message for $n!'{x",
			gch, NULL, NULL, TO_CHAR);
			act("The courier salutes you, and gallops away.",
			gch, NULL, NULL, TO_CHAR);
			act("From $T, it reads: {G$t{x",
			gch, buf+strlen(arg)+1, arg, TO_CHAR);
		    }
		    else if ( (org == ORG_MERCENARY) 
		    && (ORG(gch->org_id) == ORG_MERCENARY))
		    {	
		        if ( buf == NULL )
			break;

		    	one_argument( buf, arg );
	            	arg[0] = UPPER(arg[0]);

			if ( !str_cmp(gch->name, arg))
			    continue;
			
			act("A winged demon flies into the room and grunts at $n.",
			gch, NULL, NULL, TO_ROOM);
			act("The demon hisses, {r'A message comes for you.'{x",
			gch, NULL, NULL, TO_ROOM);
			act("The demon hands $n a scrawled parchment and flies away with a sharp hiss.",
			gch, NULL, NULL, TO_ROOM);

			act("A winged demon flies into the room and grunts at you.",
			gch, NULL, NULL, TO_CHAR);
			act("The demon hisses, {r'A message comes for you.'{x",
			gch, NULL, NULL, TO_CHAR);
			act("The demon hands you a scrawled parchment and flies away with a sharp hiss.",
			gch, NULL, NULL, TO_CHAR);
			act("From $T, the scroll reads: {R$t{x",
			gch, buf+strlen(arg)+1, arg, TO_CHAR);

		    }   
		    else if ( (org == ORG_SOLAMNIC) 
		    && (ORG(gch->org_id) == ORG_SOLAMNIC))
		    {	
		        if ( buf == NULL )
			break;

		    	one_argument( buf, arg );
	            	arg[0] = UPPER(arg[0]);

			if ( !str_cmp(gch->name, arg))
			    continue;
			
			act("A rider on horseback approaches $n and salutes formally.",
			gch, NULL, NULL, TO_ROOM);
			if(gch->sex == 2)
			   act("He says, {y'A message for you, my lady.'{x",
			   gch, NULL, NULL, TO_ROOM);
			else
			   act("He says, {y'A message for you, sir.'{x",
			   gch, NULL, NULL, TO_ROOM);
			act("The rider hands $n a scroll, then nods politely and quickly gallops away.",
			gch, NULL, NULL, TO_ROOM);

			act("A rider on horseback approaches you and salutes formally.",
			gch, NULL, NULL, TO_CHAR);
			act("The rider dismounts and bows formally.",
			gch, NULL, NULL, TO_CHAR);
			if(gch->sex == 2)
			   act("He says, {y'A message for you, my lady.'{x",
			   gch, NULL, NULL, TO_CHAR);
			else
			   act("He says, {y'A message for you, sir.'{x",
			   gch, NULL, NULL, TO_CHAR);
			act("The rider hands you a scroll, then nods politely and quickly gallops away.",
			gch, NULL, NULL, TO_CHAR);
			act("From $T, the scroll reads: {Y$t{x",
			gch, buf+strlen(arg)+1, arg, TO_CHAR);

		    }   
		    else if ( (org == ORG_FORESTER) 
		    && (ORG(gch->org_id) == ORG_FORESTER))
		    {
			if ( buf == NULL )
			    break;

		    	one_argument( buf, arg );
	            	arg[0] = UPPER(arg[0]);

			if ( !str_cmp(gch->name, arg))
			    continue;

			act("A white carrier pigeon flies in and drops a message for $n.",
			gch, NULL, NULL, TO_ROOM);
			act("A white carrier pigeon circles around $n, and flies away.",
			gch, NULL, NULL, TO_ROOM);

			act("A white carrier pigeon flies in and drops you a message.", 
			gch, NULL, NULL, TO_CHAR);
			act("A white carrier pigeon circles above you, and flies away.",
			gch, NULL, NULL, TO_CHAR);
			act("The message is from $T, and it reads: {Y$t{x",
			gch, buf+strlen(arg)+1, arg, TO_CHAR);
		    }
		    else if ( (org == ORG_HOLYORDER) 
		    && (ORG(gch->org_id) == ORG_HOLYORDER))
		    {	
		        if ( buf == NULL )
			break;

		    	one_argument( buf, arg );
	            	arg[0] = UPPER(arg[0]);

			if ( !str_cmp(gch->name, arg))
			    continue;
			
			act("A cleric in ceremonial garb jogs up to $n.",
			gch, NULL, NULL, TO_ROOM);
			act("He gathers his breath and says, {y'I am commanded to bring you this scroll, $n.'{x",
			gch, NULL, NULL, TO_ROOM);
			act("The cleric hands $n a scroll, then nods curtly and jogs steadily away.",
			gch, NULL, NULL, TO_ROOM);

			act("A cleric in ceremonial garb jogs up to you.",
			gch, NULL, NULL, TO_CHAR);
			act("He gathers his breath and says, {y'I am commanded to bring you this scroll, $n.'{x",
			gch, NULL, NULL, TO_CHAR);
			act("The cleric hands you a scroll, then nods curtly and jogs steadily away.",
			gch, NULL, NULL, TO_CHAR);
			act("From $T, the scroll reads: {Y$t{x",
			gch, buf+strlen(arg)+1, arg, TO_CHAR);
		    }
		    else if ( (org == ORG_ARTISANS) 
		    && (ORG(gch->org_id) == ORG_ARTISANS))
		    {
			if ( buf == NULL )
			    break;

		    	one_argument( buf, arg );
	            	arg[0] = UPPER(arg[0]);

			if ( !str_cmp(gch->name, arg))
			    continue;

			act("A blacksmith runs in and gives $n a messege.",
			gch, NULL, NULL, TO_ROOM);
			act("A blacksmith bows his head, then runs away.",
			gch, NULL, NULL, TO_ROOM);

			act("A blacksmith runs in and hands you a message.", 
			gch, NULL, NULL, TO_CHAR);
			act("A blacksmith bows his head, and runs away.",
			gch, NULL, NULL, TO_CHAR);
			act("The message is from $T, and it reads: {Y$t{x",
			gch, buf+strlen(arg)+1, arg, TO_CHAR);
		    }
		    else if ( (org == ORG_AESTHETIC) 
		    && (ORG(gch->org_id) == ORG_AESTHETIC))
		    {
			if ( buf == NULL )
			    break;

		    	one_argument( buf, arg );
	            	arg[0] = UPPER(arg[0]);

			if ( !str_cmp(gch->name, arg))
			    continue;

			act("A large brown hawk flies in and drops a message for $n.",
			gch, NULL, NULL, TO_ROOM);
			act("A large brown hawk circles around $n, and flies away.",
			gch, NULL, NULL, TO_ROOM);

			act("A large brown hawk flies in and drops you a message.", 
			gch, NULL, NULL, TO_CHAR);
			act("A large brown hawk circles above you, and flies away.",
			gch, NULL, NULL, TO_CHAR);
			act("The message is from $T, and it reads: {Y$t{x",
			gch, buf+strlen(arg)+1, arg, TO_CHAR);
		    }
		    else if ( (org == ORG_BLACKORDER) 
		    && (ORG(gch->org_id) == ORG_BLACKORDER))
		    {
			if ( buf == NULL )
			    break;

		    	one_argument( buf, arg );
	            	arg[0] = UPPER(arg[0]);

			if ( !str_cmp(gch->name, arg))
			    continue;

			act("An unholy monk runs in and hands $n a black scroll.",
			gch, NULL, NULL, TO_ROOM);
			act("An unholy monk bows his head in reverence, and then departs.",
			gch, NULL, NULL, TO_ROOM);

			act("An unholy monk runs in and hands you a black scoll.", 
			gch, NULL, NULL, TO_CHAR);
			act("An unholy monk bows his head in reverence, and then departs.",
			gch, NULL, NULL, TO_CHAR);
			act("The black scroll is from $T, and it reads: {Y$t{x",
			gch, buf+strlen(arg)+1, arg, TO_CHAR);
		    }
		    else if ( (org == ORG_UNDEAD) 
		    && (ORG(gch->org_id) == ORG_UNDEAD))
		    {
			if ( buf == NULL )
			    break;

		    	one_argument( buf, arg );
	            	arg[0] = UPPER(arg[0]);

			if ( !str_cmp(gch->name, arg))
			    continue;

			act("A demonic knight steps out of the shadows, handing $n a message.",
			gch, NULL, NULL, TO_ROOM);
			act("A demonic knight nods briefly, then steps back into the shadows.",
			gch, NULL, NULL, TO_ROOM);

			act("A demonic knight steps out of the shadows and hands you a message.", 
			gch, NULL, NULL, TO_CHAR);
			act("A demonic knight nods briefly, then steps back into the shadows.",
			gch, NULL, NULL, TO_CHAR);
			act("The message is from $T, and it reads: {Y$t{x",
			gch, buf+strlen(arg)+1, arg, TO_CHAR);
		    }
            } 
		free_string(ch->mesg_buf);
		ch->mesg_buf = NULL;
	    }
 	}

	/* see if fight end timer is running */
	if( ch->fight_end_timer )
	    ch->fight_end_timer--;

      /*
       * These next 2 timers are there to prevent Immortals from SPAM rewarding or
       * SPAM penalizing. They have LIMIT CAPS for a fucking reason. Can't wait to
	 * see their faces drop when they first run into this heh. The timer is set
       * currently as 100 ticks. Defined in act_wiz.c. Adjust if needed.
       */

	/* New timer for REWARD */
	if( ch->reward_timer )
	    ch->reward_timer--;

	/* New timer for Penalize */
	if( ch->penalize_timer )
	    ch->penalize_timer--;

        /* New death update: Hiddukel */
	if ( !IS_NPC( ch ) && ch->position == POS_DEAD )
	{
	  if( ch->fight_end_timer == 1 )
	    send_to_char( "Your new body is on its way!\n\r", ch );
	  else if(ch->fight_end_timer == 0)
        {
          to_temple(ch);
          if (!str_cmp(class_table[ch->class].name, "Necromancer"))
            send_to_char("You return from the spirits more powerful than before.\n\r",ch);
	  }
	}

     /*
      * Autosave and autoquit for IMMORTALS below level 110 who are link dead.
      * Getting tired of seeing Immortals dropping link instead of logging off all the time.
      */

	if ( !IS_NPC(ch)
      && ch->level > 101
      && ch->level < 110
	&& ch->desc == NULL )
      {
	  if ( ++ch->timer >= 12 )
	  {
          if (!IS_SET(ch->comm,COMM_AFK) )
          {
            send_to_char( "You have been put in AFK mode.\n\r", ch );
	  	SET_BIT(ch->comm, COMM_AFK);
          }
	  }
      }

	if ( !IS_NPC(ch) && ch->level < LEVEL_IMMORTAL )
	{
	    OBJ_DATA *obj;

	    if ( ( obj = get_eq_char( ch, WEAR_LIGHT ) ) != NULL
	    &&   obj->item_type == ITEM_LIGHT
	    &&   obj->value[2] > 0 )
	    {
		if ( --obj->value[2] == 0 && ch->in_room != NULL )
		{
		    --ch->in_room->light;
		    act( "$p goes out.", ch, obj, NULL, TO_ROOM );
		    act( "$p flickers and goes out.", ch, obj, NULL, TO_CHAR );
		    extract_obj( obj );
		}
	 	else if ( obj->value[2] <= 5 && ch->in_room != NULL)
		    act("$p flickers.",ch,obj,NULL,TO_CHAR);
	    }

	    if (IS_IMPLEMENTOR(ch)
	    || IS_SET( ch->comm, COMM_NOIDLE )
	    || IS_AFFECTED (ch, AFF_SLEEP)
	    || IS_AFFECTED2 (ch, AFF_BLACKJACK))
		ch->timer = 0;

	    if ( ++ch->timer >= 12 )
	    {
            if (!IS_SET(ch->comm,COMM_AFK) )
            {
            send_to_char( "Due to being idle for so long, you have been put in AFK mode.\n\r", ch );
            send_to_char( "If a pattern results, you will be dealt with severe punishment.\n\r", ch );
		SET_BIT(ch->comm, COMM_AFK);
            }
	    }

          if ( !IS_NPC(ch)
	    && ch->desc != NULL
	    && !IS_SET(ch->comm, COMM_AFK)
	    && !IS_SET(ch->comm, COMM_WRITING) )
	    {
	      gain_condition( ch, COND_DRUNK,  -1 );

	      if (ch->level > 6)
            {
	        gain_condition( ch, COND_THIRST, -1 );

              if (!IS_UNDEAD(ch))
	        {
	          gain_condition( ch, COND_FULL, -1);
              }
            }
          }

	   if ( (ch->pcdata->condition[COND_FULL] <= 10)
         &&   (ch->pcdata->condition[COND_FULL] > 0) )
	     send_to_char("You are feeling hungry.\n\r",ch);

	   if ( (ch->pcdata->condition[COND_THIRST] <= 10)
         &&   (ch->pcdata->condition[COND_THIRST] > 0) )
	     send_to_char("You are feeling thirsty.\n\r",ch);
	}

	for ( paf = ch->affected; paf != NULL; paf = paf_next )
	{
	    paf_next	= paf->next;
	    if ( paf->duration > 0 )
	    {
		paf->duration--;
		if (number_range(0,4) == 0 && paf->level > 0)
		  paf->level--;  /* spell strength fades with time */
            }
	    else if ( paf->duration < 0 )
		;
	    else
	    {
		if ( paf_next == NULL
		||   paf_next->type != paf->type
		||   paf_next->duration > 0 )
		{
		    if ( paf->type > 0 && skill_table[paf->type].msg_off )
		    {
			send_to_char( skill_table[paf->type].msg_off, ch );
			send_to_char( "\n\r", ch );
		    }
		}
	  
		affect_remove( ch, paf );
	    }
	}

	/*
	 * Run up passive exp for Blacksmiths
	 * Added anti-cheater code, after the top two checks
	 */
	if ( (ch->quit_timer > 20)
      &&   ch->class == class_lookup("blacksmith")
      &&   ch->desc != NULL	//Go LD, get xp... NOT.
	&&  !IS_SET(ch->comm, COMM_AFK) //seen them afk for hours on end... FANUS.
	&&   ch->timer < 2 ) //Seen them sit idle and trigger. FUCK I want anti-trigger code.
	{
	    if (ch->level < 15)
	    {
	      gain_exp(ch, 18);
	    }
	    else
	    if (ch->level < 25)
	    {
		gain_exp(ch, 36);
	    }
	    else
	    if (ch->level < 40)
	    {
		gain_exp(ch, 72);
	    }
	    else
	    if (ch->level < 60)
	    {
		gain_exp(ch, 144);
	    }
	    else
	    if (ch->level < 75)
	    {
		gain_exp(ch, 180);
	    }
	    else
	    gain_exp(ch, 216);
	}

	/*
	 * Run up passive exp for all OTHER classes
	 */

      xp_to_lev = exp_at_level(ch, ch->level + 1) - ch->exp;

	if((ch->quit_timer > 20)
      &&  ch->class != class_lookup("blacksmith")
      &&  ch->desc != NULL
	&&  !IS_SET(ch->comm, COMM_AFK)
	&& (ch->timer < 2)
	&& (ch->level > 19))
	{
	    if((ch->level < 25)
	    && (xp_to_lev >= 1500))
	    {
		gain_exp(ch, 36);
	    }
	    else
	    if((ch->level < 40)
	    && (xp_to_lev >= 2500))
	    {
		gain_exp(ch, 54);
	    }
	    else
	    if((ch->level < 60)
	    && (xp_to_lev >= 5000))
	    {
		gain_exp(ch, 72);
	    }
	    else
	    if((ch->level < 75)
	    && (xp_to_lev >= 10000))
	    {
		gain_exp(ch, 90);
	    }
	    else
	    if((ch->level < 101)
	    && (xp_to_lev >= 10000))
	    {
	      gain_exp(ch, 108);
	    }
	}

	/*
	 * Careful with the damages here,
	 *   MUST NOT refer to ch after damage taken,
	 *   as it may be lethal damage (on NPC).
	 */

	if ( (ch->loc_hp != 0 || ch->loc_hp2 != 0) && !is_obj && ch->in_room != NULL  )
	{
	  int dam = 0;

	   if (!IS_NPC(ch))
         {
	    if (IS_SET(ch->loc_hp2,BLEEDING_HEAD) && (ch->hit > ch->max_hit/3) )
	    {
		act( "A spray of blood shoots from the stump of $n's neck.", ch, NULL, NULL, TO_ROOM );
		send_to_char( "A spray of blood shoots from the stump of your neck.\n\r", ch );
            REMOVE_BIT(ch->loc_hp2,BLEEDING_HEAD);
		dam += number_range(20,50);
	    }
	    if (IS_SET(ch->loc_hp2,BLEEDING_THROAT) && (ch->hit > ch->max_hit/3) )
	    {
		act( "Blood pours from the slash in $n's throat.", ch, NULL, NULL, TO_ROOM );
		send_to_char( "Blood pours from the slash in your throat.\n\r", ch );
		dam += number_range(10,20);
	    }
	    else if (IS_SET(ch->loc_hp2,BLEEDING_HAND) && (ch->hit > ch->max_hit/3) )
	    {
		act( "A spray of blood shoots from the stump of $n's wrist.", ch, NULL, NULL, TO_ROOM );
		send_to_char( "A spray of blood shoots from the stump of your wrist.\n\r", ch );
		dam += number_range(5,10);
	    }
	    if (IS_SET(ch->loc_hp2,BLEEDING_ARM) && (ch->hit > ch->max_hit/3) )
	    {
		act( "A spray of blood shoots from the stump of $n's arm.", ch, NULL, NULL, TO_ROOM );
		send_to_char( "A spray of blood shoots from the stump of your arm.\n\r", ch );
		dam += number_range(10,20);
	    }
	    if (IS_SET(ch->loc_hp2,BLEEDING_LEG) && (ch->hit > ch->max_hit/3) )
	    {
		act( "A spray of blood shoots from the stump of $n's leg.", ch, NULL, NULL, TO_ROOM );
		send_to_char( "A spray of blood shoots from the stump of your leg.\n\r", ch );
		dam += number_range(10,20);
	    }
	    else if (IS_SET(ch->loc_hp2,BLEEDING_FOOT) && (ch->hit > ch->max_hit/3) )
	    {
		act( "A spray of blood shoots from the stump of $n's ankle.", ch, NULL, NULL, TO_ROOM );
		send_to_char( "A spray of blood shoots from the stump of your ankle.\n\r", ch );
		dam += number_range(5,10);
	    }
	    update_pos(ch);
         }
        }

      if (is_affected(ch, gsn_plague) 
      && (ch != NULL)
      && (!IS_AFFECTED3(ch,AFF_SUBDUE)))
      {
        AFFECT_DATA *af, plague;
        CHAR_DATA *vch;
        int dam;

	  if (ch->in_room == NULL)
          continue;
            
	  act("$n writhes in agony as plague sores erupt from $s skin.",
		  ch, NULL, NULL, TO_ROOM);
	  send_to_char("You writhe in agony from the plague.\n\r",ch);

        for ( af = ch->affected; af != NULL; af = af->next )
        {
          if (af->type == gsn_plague)
            break;
        }
        
        if (af == NULL)
        {
          REMOVE_BIT(ch->affected_by,AFF_PLAGUE);
          continue;
        }
        
        if (af->level == 1)
          continue;
        
	  plague.where		= TO_AFFECTS;
        plague.type 		= gsn_plague;
        plague.level 		= af->level - 1; 
        plague.duration 	= number_range(1,2 * plague.level);
        plague.location		= APPLY_STR;
        plague.modifier 	= -5;
        plague.bitvector 	= AFF_PLAGUE;
        
        for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
        {
          if (!saves_spell(plague.level - 2,vch,DAM_DISEASE) 
	    &&  !IS_IMMORTAL(vch)
          &&  !IS_AFFECTED(vch,AFF_PLAGUE) && number_bits(4) == 0)
          {
            send_to_char("You feel hot and feverish.\n\r",vch);
            act("$n shivers and looks very ill.",vch,NULL,NULL,TO_ROOM);
            affect_join(vch,&plague);
          }
        }

	  dam = UMIN(ch->level,af->level/5+1);
	  ch->mana -= dam;
	  ch->move -= dam;
	  damage( ch, ch, dam, gsn_plague,DAM_DISEASE,FALSE);
      }
	else 
      if ( IS_AFFECTED(ch, AFF_POISON)
      &&   ch != NULL
	&&  !IS_AFFECTED(ch,AFF_SLOW)
      && (!IS_AFFECTED3(ch,AFF_SUBDUE)))
	{
	  AFFECT_DATA *poison;

	  poison = affect_find(ch->affected,gsn_poison);

	  if (poison != NULL)
	  {
	    act( "$n shivers and suffers.", ch, NULL, NULL, TO_ROOM );
	    send_to_char( "You shiver and suffer.\n\r", ch );
	    damage(ch,ch,poison->level/10 + 1,gsn_poison,DAM_POISON,FALSE);
	  }
	}
      else
      if ( is_affected(ch,gsn_rupture)
      &&   ch != NULL )
      {
	  act( "$n doubles over in pain, internal bleeding slowly injuring $m.",
                ch, NULL, NULL, TO_ROOM );
	  send_to_char( "You double over in pain from internal bleeding.\n\r", ch );
	  damage(ch, ch, number_range( 10, 40 ), gsn_rupture, DAM_BASH, FALSE);
      }
	else
	if ( IS_AFFECTED3(ch, AFF_FIRE)
      && ch != NULL
	&& !IS_NPC(ch))
	{
	  act( "$n screams in agony as the flames around $m burn $m.", ch, NULL, NULL, TO_ROOM );
	  send_to_char( "Your on fire, and the flames are burning you severely!\n\r", ch );
	  damage(ch, ch, number_range( 10, 40 ), gsn_fire, DAM_FIRE, FALSE);
	}
	else
      if ( ch->position == POS_INCAP && number_range(0,1) == 0)
	{
	  damage( ch, ch, 1, TYPE_UNDEFINED, DAM_NONE,FALSE);
	}
	else
      if ( ch->position == POS_MORTAL )
	{
	  damage( ch, ch, 1, TYPE_UNDEFINED, DAM_NONE,FALSE);
	}
    }

    /*
     * Autosave and autoquit.
     * Check that these chars still exist.
     */
    for ( ch = char_list; ch != NULL; ch = ch_next )
    {

        //C073 Added from Edwin's fix list
        if (!IS_VALID(ch))
        {
            bugf("update_char: Trying to work with an invalidated character.\n");
            break;
        }

        ch_next = ch->next;

	if (ch->desc != NULL && ch->desc->descriptor % 30 == save_number)
	{
	    save_char_obj(ch);
	}

        if (ch == ch_quit)
	{
            do_function(ch, &do_quit, "" );
	}
    }

    return;
}




/*
 * Update all objs.
 * This function is performance sensitive.
 */
void obj_update( void )
{   
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    AFFECT_DATA *paf, *paf_next;

    for ( obj = object_list; obj != NULL; obj = obj_next )
    {
	CHAR_DATA *rch;
	char *message;

	obj_next = obj->next;

	/* go through affects and decrement */
        for ( paf = obj->affected; paf != NULL; paf = paf_next )
        {
            paf_next    = paf->next;
            if ( paf->duration > 0 )
            {
                paf->duration--;
                if (number_range(0,4) == 0 && paf->level > 0)
                  paf->level--;  /* spell strength fades with time */
            }
            else if ( paf->duration < 0 )
                ;
            else
            {
                if ( paf_next == NULL
                ||   paf_next->type != paf->type
                ||   paf_next->duration > 0 )
                {
                    if ( paf->type > 0 && skill_table[paf->type].msg_obj )
                    {
			if (obj->carried_by != NULL)
			{
			    rch = obj->carried_by;
			    act(skill_table[paf->type].msg_obj,
				rch,obj,NULL,TO_CHAR);
			}
			if (obj->in_room != NULL 
			&& obj->in_room->people != NULL)
			{
			    rch = obj->in_room->people;
			    act(skill_table[paf->type].msg_obj,
				rch,obj,NULL,TO_ALL);
			}
                    }
                }

                affect_remove_obj( obj, paf );
            }
        }
	/*
	 * Oprog triggers!
	 */
	if ( obj->in_room || (obj->carried_by && obj->carried_by->in_room))
	{
	    if ( HAS_TRIGGER_OBJ( obj, TRIG_DELAY )
	      && obj->oprog_delay > 0 )
	    {
	        if ( --obj->oprog_delay <= 0 )
		    p_percent_trigger( NULL, obj, NULL, NULL, NULL, NULL, TRIG_DELAY );
	    }
	    else if ( ((obj->in_room && !obj->in_room->area->empty)
	    	|| obj->carried_by ) && HAS_TRIGGER_OBJ( obj, TRIG_RANDOM ) )
		    p_percent_trigger( NULL, obj, NULL, NULL, NULL, NULL, TRIG_RANDOM );
	 }
	/* Make sure the object is still there before proceeding */
	if ( !obj )
	    continue;

	if ( obj->timer <= 0 || --obj->timer > 0 )
	    continue;

	if (obj->pIndexData->vnum == 1894
      ||  obj->pIndexData->vnum == 1864)
	{
	  message = "$p soaks into the ground.";
	}
	else
	if (obj->pIndexData->vnum == 1820)
	{
	  message = "$p wither and die, decomposing into dust.";
	}
	else
	if (obj->pIndexData->vnum == 22
	&& !str_cmp(obj->name,"geyser water"))
	{
	  message = "The City Plumbers come out and fix the ruptured water main, stopping the geyser from wasting any more water.";
	}
	else
	{
	switch ( obj->item_type )
	 {
	default:              message = "$p crumbles into dust.";  break;
	case ITEM_FOUNTAIN:   message = "$p dries up.";         break;
	case ITEM_CORPSE_NPC: message = "$p decays into dust."; break;
	case ITEM_CORPSE_PC:  message = "$p decays into dust."; break;
	case ITEM_FOOD:       message = "$p decomposes.";	break;
	case ITEM_POTION:     message = "$p has evaporated from disuse.";	
								break;
	case ITEM_PORTAL:     message = "$p fades out of existence."; break;
	case ITEM_CONTAINER: 
	    if (CAN_WEAR(obj,ITEM_WEAR_FLOAT))
		if (obj->contains)
		    message = 
		"$p flickers and vanishes, spilling its contents on the floor.";
		else
		    message = "$p flickers and vanishes.";
	    else
		message = "$p crumbles into dust.";
	    break;
	 }
        }

	if ( obj->carried_by != NULL )
	{
	    if (IS_NPC(obj->carried_by) 
	    &&  obj->carried_by->pIndexData->pShop != NULL)
		obj->carried_by->gold += obj->cost/5;
	    else
	    {
	    	act( message, obj->carried_by, obj, NULL, TO_CHAR );
		if ( obj->wear_loc == WEAR_FLOAT)
		    act(message,obj->carried_by,obj,NULL,TO_ROOM);
	    }
	}
	else if ( obj->in_room != NULL
	&&      ( rch = obj->in_room->people ) != NULL )
	{
	    if (! (obj->in_obj 
	    && (obj->in_obj->pIndexData->vnum == OBJ_VNUM_PIT 
			|| obj->in_obj->pIndexData->vnum == hometown_table[rch->hometown].pit_vnum)
	    && !CAN_WEAR(obj->in_obj,ITEM_TAKE)))
	    {
	    	act( message, rch, obj, NULL, TO_ROOM );
	    	act( message, rch, obj, NULL, TO_CHAR );
	    }
	}

        if ((obj->item_type == ITEM_CORPSE_PC || obj->wear_loc == WEAR_FLOAT)
	&&  obj->contains)
	{   /* save the contents */
     	    OBJ_DATA *t_obj, *next_obj;

	    for (t_obj = obj->contains; t_obj != NULL; t_obj = next_obj)
	    {
		next_obj = t_obj->next_content;
		obj_from_obj(t_obj);

		if (obj->in_obj) /* in another object */
		    obj_to_obj(t_obj,obj->in_obj);

		else if (obj->carried_by)  /* carried */
		    if (obj->wear_loc == WEAR_FLOAT)
			if (obj->carried_by->in_room == NULL)
			    extract_obj(t_obj);
			else
			    obj_to_room(t_obj,obj->carried_by->in_room);
		    else
		    	obj_to_char(t_obj,obj->carried_by);

		else if (obj->in_room == NULL)  /* destroy it */
		    extract_obj(t_obj);

		else /* to a room */
		    obj_to_room(t_obj,obj->in_room);
	    }
	}

	extract_obj( obj );
    }

    return;
}



/*
 * Aggress.
 *
 * for each mortal PC
 *     for each mob in room
 *         aggress on some random PC
 *
 * This function takes 25% to 35% of ALL Merc cpu time.
 * Unfortunately, checking on each PC move is too tricky,
 *   because we don't the mob to just attack the first PC
 *   who leads the party into the room.
 *
 * -- Furey
 */
void aggr_update( void )
{
  CHAR_DATA *wch;
  CHAR_DATA *wch_next;
  CHAR_DATA *ch;
  CHAR_DATA *ch_next;
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  CHAR_DATA *victim;

  for ( wch = char_list; wch != NULL; wch = wch_next )
  {
    wch_next = wch->next;

    if ( IS_NPC(wch)
    ||   wch->level >= LEVEL_IMMORTAL
    ||   IS_AFFECTED2(wch, AFF_INVUL)
    ||   IS_AFFECTED3(wch, AFF_SUBDUE)
    ||   wch->class == class_lookup("blacksmith")
    ||   wch->in_room == NULL 
    ||   wch->riding != NULL
    ||   wch->in_room->area->empty)
	continue;

    for ( ch = wch->in_room->people; ch != NULL; ch = ch_next )
    {
	int count;

	ch_next = ch->next_in_room;

     /*
	* First set of checks are to see if a player is affected by any
	* of the BEASTSPITE affects, and if so, attack the player.
	*/

      if (IS_AFFECTED3(wch, AFF_BEASTSPITE_BEAR))
	{
	  if ( !IS_NPC(ch)
	  ||   IS_SET(ch->in_room->room_flags,ROOM_SAFE)
	  ||   IS_AFFECTED(ch,AFF_CALM)
	  ||   IS_AFFECTED2(ch,AFF_RAINBOW_PATTERN)
	  ||   ch->fighting != NULL
	  ||   IS_AFFECTED(ch, AFF_CHARM)
	  ||   !IS_AWAKE(ch)
	  ||   ( IS_SET(ch->act, ACT_WIMPY) && IS_AWAKE(wch) )
	  ||   number_bits(1) == 0)
	    continue;

	  count	= 0;
	  victim	= NULL;
	  for ( vch = wch->in_room->people; vch != NULL; vch = vch_next )
	  {
	    vch_next = vch->next_in_room;

	    if ( !IS_NPC(vch)
	    &&   vch->level < 102
	    && (ch->race == race_lookup("bear")))
	    {
		if ( number_range( 0, count ) == 0 )
		  victim = vch;
		count++;
	    }
	  }
        if (!victim)
	    continue;

        interpret(ch,"growl");
        act( "$n looks at $N with spite before attacking $M!",ch,NULL,victim, TO_NOTVICT );
        act( "$n looks at you with spite, before attacking you!",ch,NULL,victim,TO_VICT);
	  multi_hit( ch, victim, TYPE_UNDEFINED );
	}
	else
      if (IS_AFFECTED3(wch, AFF_BEASTSPITE_LIZARD))
	{
	  if ( !IS_NPC(ch)
	  ||   IS_SET(ch->in_room->room_flags,ROOM_SAFE)
	  ||   IS_AFFECTED(ch,AFF_CALM)
	  ||   IS_AFFECTED2(ch,AFF_RAINBOW_PATTERN)
	  ||   ch->fighting != NULL
	  ||   IS_AFFECTED(ch, AFF_CHARM)
	  ||   !IS_AWAKE(ch)
	  ||   ( IS_SET(ch->act, ACT_WIMPY) && IS_AWAKE(wch) )
	  ||   number_bits(1) == 0)
	    continue;

	  count	= 0;
	  victim	= NULL;
	  for ( vch = wch->in_room->people; vch != NULL; vch = vch_next )
	  {
	    vch_next = vch->next_in_room;

	    if ( !IS_NPC(vch)
	    &&   vch->level < 102
	    && (ch->race == race_lookup("lizard")))
	    {
		if ( number_range( 0, count ) == 0 )
		  victim = vch;
		count++;
	    }
	  }
        if (!victim)
	    continue;

        interpret(ch,"glare");
        act( "$n looks at $N with spite before attacking $M!",ch,NULL,victim, TO_NOTVICT );
        act( "$n looks at you with spite, before attacking you!",ch,NULL,victim,TO_VICT);
	  multi_hit( ch, victim, TYPE_UNDEFINED );
	}
	else
      if (IS_AFFECTED3(wch, AFF_BEASTSPITE_FOX))
	{
	  if ( !IS_NPC(ch)
	  ||   IS_SET(ch->in_room->room_flags,ROOM_SAFE)
	  ||   IS_AFFECTED(ch,AFF_CALM)
	  ||   IS_AFFECTED2(ch,AFF_RAINBOW_PATTERN)
	  ||   ch->fighting != NULL
	  ||   IS_AFFECTED(ch, AFF_CHARM)
	  ||   !IS_AWAKE(ch)
	  ||   ( IS_SET(ch->act, ACT_WIMPY) && IS_AWAKE(wch) )
	  ||   number_bits(1) == 0)
	    continue;

	  count	= 0;
	  victim	= NULL;
	  for ( vch = wch->in_room->people; vch != NULL; vch = vch_next )
	  {
	    vch_next = vch->next_in_room;

	    if ( !IS_NPC(vch)
	    &&   vch->level < 102
	    && (ch->race == race_lookup("fox")))
	    {
		if ( number_range( 0, count ) == 0 )
		  victim = vch;
		count++;
	    }
	  }
        if (!victim)
	    continue;

        interpret(ch,"snarl");
        act( "$n looks at $N with spite before attacking $M!",ch,NULL,victim, TO_NOTVICT );
        act( "$n looks at you with spite, before attacking you!",ch,NULL,victim,TO_VICT);
	  multi_hit( ch, victim, TYPE_UNDEFINED );
	}
	else
      if (IS_AFFECTED3(wch, AFF_BEASTSPITE_WOLF))
	{
	  if ( !IS_NPC(ch)
	  ||   IS_SET(ch->in_room->room_flags,ROOM_SAFE)
	  ||   IS_AFFECTED(ch,AFF_CALM)
	  ||   IS_AFFECTED2(ch,AFF_RAINBOW_PATTERN)
	  ||   ch->fighting != NULL
	  ||   IS_AFFECTED(ch, AFF_CHARM)
	  ||   !IS_AWAKE(ch)
	  ||   ( IS_SET(ch->act, ACT_WIMPY) && IS_AWAKE(wch) )
	  ||   number_bits(1) == 0)
	    continue;

	  count	= 0;
	  victim	= NULL;
	  for ( vch = wch->in_room->people; vch != NULL; vch = vch_next )
	  {
	    vch_next = vch->next_in_room;

	    if ( !IS_NPC(vch)
	    &&   vch->level < 102
	    && (ch->race == race_lookup("wolf")))
	    {
		if ( number_range( 0, count ) == 0 )
		  victim = vch;
		count++;
	    }
	  }
        if (!victim)
	    continue;

        interpret(ch,"howl");
        act( "$n looks at $N with spite before attacking $M!",ch,NULL,victim, TO_NOTVICT );
        act( "$n looks at you with spite, before attacking you!",ch,NULL,victim,TO_VICT);
	  multi_hit( ch, victim, TYPE_UNDEFINED );
	}
	else
      if (IS_AFFECTED3(wch, AFF_BEASTSPITE_WYVERN))
	{
	  if ( !IS_NPC(ch)
	  ||   IS_SET(ch->in_room->room_flags,ROOM_SAFE)
	  ||   IS_AFFECTED(ch,AFF_CALM)
	  ||   IS_AFFECTED2(ch,AFF_RAINBOW_PATTERN)
	  ||   ch->fighting != NULL
	  ||   IS_AFFECTED(ch, AFF_CHARM)
	  ||   !IS_AWAKE(ch)
	  ||   ( IS_SET(ch->act, ACT_WIMPY) && IS_AWAKE(wch) )
	  ||   number_bits(1) == 0)
	    continue;

	  count	= 0;
	  victim	= NULL;
	  for ( vch = wch->in_room->people; vch != NULL; vch = vch_next )
	  {
	    vch_next = vch->next_in_room;

	    if ( !IS_NPC(vch)
	    &&   vch->level < 102
	    && (ch->race == race_lookup("wyvern")))
	    {
		if ( number_range( 0, count ) == 0 )
		  victim = vch;
		count++;
	    }
	  }
        if (!victim)
	    continue;

        interpret(ch,"glare");
        act( "$n looks at $N with spite before attacking $M!",ch,NULL,victim, TO_NOTVICT );
        act( "$n looks at you with spite, before attacking you!",ch,NULL,victim,TO_VICT);
	  multi_hit( ch, victim, TYPE_UNDEFINED );
	}
	else
      if (IS_AFFECTED3(wch, AFF_BEASTSPITE_DRAGON))
	{
	  if ( !IS_NPC(ch)
	  ||   IS_SET(ch->in_room->room_flags,ROOM_SAFE)
	  ||   IS_AFFECTED(ch,AFF_CALM)
	  ||   IS_AFFECTED2(ch,AFF_RAINBOW_PATTERN)
	  ||   ch->fighting != NULL
	  ||   IS_AFFECTED(ch, AFF_CHARM)
	  ||   !IS_AWAKE(ch)
	  ||   ( IS_SET(ch->act, ACT_WIMPY) && IS_AWAKE(wch) )
	  ||   number_bits(1) == 0)
	    continue;

	  count	= 0;
	  victim	= NULL;
	  for ( vch = wch->in_room->people; vch != NULL; vch = vch_next )
	  {
	    vch_next = vch->next_in_room;

	    if ( !IS_NPC(vch)
	    &&   vch->level < 102
	    && (ch->race == race_lookup("dragon")))
	    {
		if ( number_range( 0, count ) == 0 )
		  victim = vch;
		count++;
	    }
	  }
        if (!victim)
	    continue;

        interpret(ch,"roar");
        act( "$n looks at $N with spite before attacking $M!",ch,NULL,victim, TO_NOTVICT );
        act( "$n looks at you with spite, before attacking you!",ch,NULL,victim,TO_VICT);
	  multi_hit( ch, victim, TYPE_UNDEFINED );
	}
	else
      if (IS_AFFECTED3(wch, AFF_BEASTSPITE_BAT))
	{
	  if ( !IS_NPC(ch)
	  ||   IS_SET(ch->in_room->room_flags,ROOM_SAFE)
	  ||   IS_AFFECTED(ch,AFF_CALM)
	  ||   IS_AFFECTED2(ch,AFF_RAINBOW_PATTERN)
	  ||   ch->fighting != NULL
	  ||   IS_AFFECTED(ch, AFF_CHARM)
	  ||   !IS_AWAKE(ch)
	  ||   ( IS_SET(ch->act, ACT_WIMPY) && IS_AWAKE(wch) )
	  ||   number_bits(1) == 0)
	    continue;

	  count	= 0;
	  victim	= NULL;
	  for ( vch = wch->in_room->people; vch != NULL; vch = vch_next )
	  {
	    vch_next = vch->next_in_room;

	    if ( !IS_NPC(vch)
	    &&   vch->level < 102
	    && (ch->race == race_lookup("bat")))
	    {
		if ( number_range( 0, count ) == 0 )
		  victim = vch;
		count++;
	    }
	  }
        if (!victim)
	    continue;

        interpret(ch,"snarl");
        act( "$n looks at $N with spite before attacking $M!",ch,NULL,victim, TO_NOTVICT );
        act( "$n looks at you with spite, before attacking you!",ch,NULL,victim,TO_VICT);
	  multi_hit( ch, victim, TYPE_UNDEFINED );
	}
	else
	{
	  if ( !IS_NPC(ch)
	  ||   !IS_SET(ch->act, ACT_AGGRESSIVE)
	  ||   IS_SET(ch->in_room->room_flags,ROOM_SAFE)
	  ||   IS_AFFECTED(ch,AFF_CALM)
	  ||   ch->level >= wch->level + 5
	  ||   IS_AFFECTED2(ch,AFF_RAINBOW_PATTERN)
	  ||   ch->fighting != NULL
	  ||   IS_AFFECTED(ch, AFF_CHARM)
	  ||   !IS_AWAKE(ch)
	  ||   ( IS_SET(ch->act, ACT_WIMPY) && IS_AWAKE(wch) )
	  ||   !can_see( ch, wch ) 
	  ||   number_bits(1) == 0)
	    continue;

	  count	= 0;
	  victim	= NULL;
	  for ( vch = wch->in_room->people; vch != NULL; vch = vch_next )
	  {
	    vch_next = vch->next_in_room;

	    if ( !IS_NPC(vch)
	    &&   vch->level < LEVEL_IMMORTAL
	    &&   ( !IS_SET(ch->act, ACT_WIMPY) || !IS_AWAKE(vch) )
	    &&   can_see( ch, vch )
	    &&   !IS_AFFECTED2(vch, AFF_SYMPATHY)
	    &&   !IS_AFFECTED2(vch, AFF_REPULSION))
	    {
		    if ( number_range( 0, count ) == 0 )
			victim = vch;
		    count++;
	    }
	  }
        if (!victim)
	    return;

	  multi_hit( ch, victim, TYPE_UNDEFINED );
      }
    }
  }
  return;
}

 void sleep_update(void)
 {
     SLEEP_DATA *temp = first_sleep, *temp_next;
     int type = 0; // 1 = mob, 2 = obj, 3= room, 0 = NULL

     for( ; temp != NULL; temp = temp_next)
     {
         bool delete = FALSE;
         
         temp_next = temp->next;
         
	if(temp->mob)
		type = 1;
	else if(temp->obj)
		type = 2;
	else if(temp->room)
		type = 3;
	else
		type = 0;

         /* checks to make sure the mob still exists*/
/*         if(!temp->mob) //this was before modififications for o/rprogs also */
		if(type == 0)
               delete = TRUE;
         /*checks to make sure the character is still in the same room as the mob*/
         if(temp->mob && temp->ch && temp->mob->in_room != temp->ch->in_room)
            delete = TRUE;
	   else if(temp->obj && !get_obj_here(temp->ch, NULL, temp->obj->name) )
		delete = TRUE;
	   else if(temp->room && !get_char_room(NULL, temp->room, temp->ch->name) )
		delete = TRUE;
	   else
		delete = FALSE;

          if(delete)
         {
             /* some slick linked list manipulation */
             if(temp->prev)
                 temp->prev->next = temp->next;
             if(temp->next)
                 temp->next->prev = temp->prev;
             if( temp == first_sleep && (temp->next == NULL || temp->prev == NULL) )
                 first_sleep = temp->next;
             free_sleep_data(temp);
             continue;
         }
         
         if(--temp->timer <= 0)
         {
		if(type == 1)
             	program_flow(temp->vnum, temp->prog->code, temp->mob, NULL, NULL, temp->ch, NULL, NULL, temp->line);
            if(type ==2)
	            program_flow(temp->vnum, temp->prog->code, NULL, temp->obj, NULL, temp->ch, NULL, NULL, temp->line);
		if(type == 3)
	            program_flow(temp->vnum, temp->prog->code, NULL, NULL, temp->room, temp->ch, NULL, NULL, temp->line);

             /* more slick linked list manipulation */
             if(temp->prev)
                 temp->prev->next = temp->next;
             if(temp->next)
                 temp->next->prev = temp->prev;
             if( temp == first_sleep && (temp->next == NULL || temp->prev == NULL) )
                 first_sleep = temp->next;
             free_sleep_data(temp);
         }
     }
 }

/*
 * Handle all kinds of updates.
 * Called once per pulse from game loop.
 * Random times to defeat tick-timing clients and players.
 */

void update_handler( void )
{
    static  int     pulse_area;
    static  int     pulse_mobile;
    static  int     pulse_violence;
    static  int     pulse_point;
    extern  int 	  port;
    static  int     pulse_music;

    if ( --pulse_music    <= 0 )
    {
        pulse_music = PULSE_MUSIC;
        singing_update();
    }

    if ( --pulse_area     <= 0 )
    {
	pulse_area	= PULSE_AREA;
	/* number_range( PULSE_AREA / 2, 3 * PULSE_AREA / 2 ); */
	area_update	( );
      quest_update     ( );
    }

    if ( --pulse_mobile   <= 0 )
    {
	pulse_mobile	= PULSE_MOBILE;
	mobile_update	( );
    }

    if ( --pulse_violence <= 0 )
    {
	pulse_violence	= PULSE_VIOLENCE;
	violence_update	( );
    }

    if ( --pulse_point    <= 0 )
    {
	wiznet("TICK!",NULL,NULL,WIZ_TICKS,0,0);
	pulse_point     = PULSE_TICK;
/* number_range( PULSE_TICK / 2, 3 * PULSE_TICK / 2 ); */
	weather_update( );
	char_update	  ( );
	obj_update	  ( );
    }
    if (IS_PP(port))
    sleep_update();
    aggr_update( );
    tail_chain( );
    return;
}

void do_tick( CHAR_DATA *ch, char *argument )
{   DESCRIPTOR_DATA *d;
    area_update	( );
    mobile_update	( );
    violence_update	( );
    weather_update( );
    char_update	( );
    obj_update	( );
    send_to_char("You forced time to fly by! Tick!\n\r", ch);
       for (d = descriptor_list; d != NULL; d = d->next)
       {
        if (d == NULL)
         continue;

           ch->timer++;
        if (ch->timer > 20)
        {
             if (d->character != NULL && d->connected == CON_PLAYING)
                   save_char_obj(d->character );
                d->outtop = 0;
                close_socket ( d );
	}
       }
    return;
}

void check_death( CHAR_DATA *ch)
{

  DESCRIPTOR_DATA *d;  
  int age;

  if (IS_IMMORTAL(ch))
    return;

  age = get_age(ch);

  if (age < pc_race_table[ch->race].max_age)
   return;

  ch->deathstat++;
  send_to_char("You feel old and tired.\n\r",ch); 

  if (ch->deathstat == 1 || ch->deathstat == 50)
  {
    ch->max_hit  -= ch->max_hit  /3; 
    ch->max_move -= ch->max_move /3;
    ch->perm_stat[STAT_CON] -= 3;
    ch->perm_stat[STAT_DEX] -= 3;
    ch->perm_stat[STAT_STR] -= 3;

 }

  
 if (ch->deathstat > MAX_DEATH)
 {
   send_to_char("Alas, all things end. You shall be remembered.\n\r",ch);
   act("$n lays down $s life and in $s final moments, $s spirit goes to walk with the immortals.\n\r",ch,NULL,NULL,TO_ROOM);
   act("You lay down your life, and your spirit goes to walk with the immortals.\n\r",ch, NULL, NULL, TO_CHAR);
   send_to_char("The final pages in this life have been written.\n\r",ch);
   save_char_obj( ch );
   d = ch->desc;
   extract_char( ch, TRUE ); 
   if ( d != NULL )
       close_socket( d );
   return;
 }

 if (ch->deathstat > 90)
  send_to_char("You are nearing your final death. Prepare yourself.\n\r",ch);


return;
}

void to_temple(CHAR_DATA *ch)
{

  if (ch->alignment < -250)
  {
    act ("Demons ascend out of the Abyss with a new body for $n.",ch,NULL,NULL,TO_ROOM);  
    send_to_char( "Demons ascend out of the Abyss with your new body!\n\r", ch );   
  }
  else
  {
    act ("Angels descend from the heavens with a new body for $n.",ch,NULL,NULL,TO_ROOM);     
    send_to_char( "Angels descend from the heavens with your new body!\n\r", ch );
  }

  send_to_char( "It appears to be a pretty good one.\n\r", ch );
  ch->position = POS_STANDING;


return;
}

void health_dam( CHAR_DATA *ch )
{

  if ( ch->desc == NULL)
    return;

  if (ch->move<10)
    ch->move +=8;
  if (ch->level < 10)
    return;

  if (IS_AFFECTED(ch, AFF_SHACKLES)) 
   return;

 if (ch->hit < 0)
 {
   send_to_char("\n\rYou are {rDEAD!!{x\n\r\n\r",ch);
   raw_kill(ch, ch);
   ch->pcdata->deaths++;

   act( "A spirit appears to rise from the lifeless corpse of $n.\n\r",ch, 0, 0, TO_ROOM );
   send_to_char("Your spirit rises and hovers above your lifeless corpse while you\n\r",ch);  
   send_to_char("wait for the gods to grant you a new body.\n\r",ch);  

   ch->hit = 1;
   ch->position = POS_DEAD;
   ch->fight_end_timer = 3; 
 }

return;
}

void regen_limb(CHAR_DATA *ch)
{
  if (ch->loc_hp != 0 || ch->loc_hp2 !=0)
    reg_mend(ch);
  return;
}

void reg_mend( CHAR_DATA *ch )
{

    if (IS_SET(ch->loc_hp,BROKEN_RIBS_1 ))
    {
	act("One of $n's ribs snap back into place.",ch,NULL,NULL,TO_ROOM);
	act("One of your ribs snap back into place.",ch,NULL,NULL,TO_CHAR);
	REMOVE_BIT(ch->loc_hp,BROKEN_RIBS_1);
    }
    else if (IS_SET(ch->loc_hp,BROKEN_RIBS_2 ))
    {
	act("One of $n's ribs snap back into place.",ch,NULL,NULL,TO_ROOM);
	act("One of your ribs snap back into place.",ch,NULL,NULL,TO_CHAR);
	REMOVE_BIT(ch->loc_hp,BROKEN_RIBS_2);
    }
    else if (IS_SET(ch->loc_hp,BROKEN_RIBS_4 ))
    {
	act("One of $n's ribs snap back into place.",ch,NULL,NULL,TO_ROOM);
	act("One of your ribs snap back into place.",ch,NULL,NULL,TO_CHAR);
	REMOVE_BIT(ch->loc_hp,BROKEN_RIBS_4);
    }
    else if (IS_SET(ch->loc_hp,BROKEN_RIBS_8 ))
    {
	act("One of $n's ribs snap back into place.",ch,NULL,NULL,TO_ROOM);
	act("One of your ribs snap back into place.",ch,NULL,NULL,TO_CHAR);
	REMOVE_BIT(ch->loc_hp,BROKEN_RIBS_8);
    }
    else if (IS_SET(ch->loc_hp,BROKEN_RIBS_16))
    {
	act("One of $n's ribs snap back into place.",ch,NULL,NULL,TO_ROOM);
	act("One of your ribs snap back into place.",ch,NULL,NULL,TO_CHAR);
      REMOVE_BIT(ch->loc_hp,BROKEN_RIBS_16);
    }
    else if (IS_SET(ch->loc_hp,LOST_EYE_L))
    {
	act("An eyeball appears in $n's left eye socket.",ch,NULL,NULL,TO_ROOM);
	act("An eyeball appears in your left eye socket.",ch,NULL,NULL,TO_CHAR);
	REMOVE_BIT(ch->loc_hp,LOST_EYE_L);
    }
    else if (IS_SET(ch->loc_hp,LOST_EYE_R))
    {
	act("An eyeball appears in $n's right eye socket.",ch,NULL,NULL,TO_ROOM);
	act("An eyeball appears in your right eye socket.",ch,NULL,NULL,TO_CHAR);
	REMOVE_BIT(ch->loc_hp,LOST_EYE_R);
    }
    else if (IS_SET(ch->loc_hp,LOST_EAR_L))
    {
	act("An ear grows on the left side of $n's head.",ch,NULL,NULL,TO_ROOM);
	act("An ear grows on the left side of your head.",ch,NULL,NULL,TO_CHAR);
	REMOVE_BIT(ch->loc_hp,LOST_EAR_L);
    }
    else if (IS_SET(ch->loc_hp,LOST_EAR_R))
    {
	act("An ear grows on the right side of $n's head.",ch,NULL,NULL,TO_ROOM);
	act("An ear grows on the right side of your head.",ch,NULL,NULL,TO_CHAR);
	REMOVE_BIT(ch->loc_hp,LOST_EAR_R);
    }
    else if (IS_SET(ch->loc_hp,LOST_NOSE))
    {
	act("A nose grows on the front of $n's face.",ch,NULL,NULL,TO_ROOM);
	act("A nose grows on the front of your face.",ch,NULL,NULL,TO_CHAR);
	REMOVE_BIT(ch->loc_hp,LOST_NOSE);
	REMOVE_BIT(ch->loc_hp,BROKEN_NOSE);
    }
    else if (IS_SET(ch->loc_hp,LOST_TOOTH_1 ))
    {
	act("A missing tooth grows in your mouth.",ch,NULL,NULL,TO_CHAR);
	act("A missing tooth grows in $n's mouth.",ch,NULL,NULL,TO_ROOM);
	REMOVE_BIT(ch->loc_hp,LOST_TOOTH_1);
    }
    else if (IS_SET(ch->loc_hp,LOST_TOOTH_2 ))
    {
	act("A missing tooth grows in your mouth.",ch,NULL,NULL,TO_CHAR);
	act("A missing tooth grows in $n's mouth.",ch,NULL,NULL,TO_ROOM);
	REMOVE_BIT(ch->loc_hp,LOST_TOOTH_2);
    }
    else if (IS_SET(ch->loc_hp,LOST_TOOTH_4 ))
    {
	act("A missing tooth grows in your mouth.",ch,NULL,NULL,TO_CHAR);
	act("A missing tooth grows in $n's mouth.",ch,NULL,NULL,TO_ROOM);
	REMOVE_BIT(ch->loc_hp,LOST_TOOTH_4);
    }
    else if (IS_SET(ch->loc_hp,LOST_TOOTH_8 ))
    {
	act("A missing tooth grows in your mouth.",ch,NULL,NULL,TO_CHAR);
	act("A missing tooth grows in $n's mouth.",ch,NULL,NULL,TO_ROOM);
	REMOVE_BIT(ch->loc_hp,LOST_TOOTH_8);
    }
    else if (IS_SET(ch->loc_hp,LOST_TOOTH_16))
    {
	act("A missing tooth grows in your mouth.",ch,NULL,NULL,TO_CHAR);
	act("A missing tooth grows in $n's mouth.",ch,NULL,NULL,TO_ROOM);
	REMOVE_BIT(ch->loc_hp,LOST_TOOTH_16);
    }
    else if (IS_SET(ch->loc_hp,BROKEN_NOSE))
    {
	act("$n's nose snaps back into place.",ch,NULL,NULL,TO_ROOM);
	act("Your nose snaps back into place.",ch,NULL,NULL,TO_CHAR);
	REMOVE_BIT(ch->loc_hp,BROKEN_NOSE);
    }
    else if (IS_SET(ch->loc_hp,BROKEN_JAW))
    {
	act("$n's jaw snaps back into place.",ch,NULL,NULL,TO_ROOM);
	act("Your jaw snaps back into place.",ch,NULL,NULL,TO_CHAR);
	REMOVE_BIT(ch->loc_hp,BROKEN_JAW);
    }
    else if (IS_SET(ch->loc_hp,BROKEN_SKULL))
    {
	act("$n's skull knits itself back together.",ch,NULL,NULL,TO_ROOM);
	act("Your skull knits itself back together.",ch,NULL,NULL,TO_CHAR);
	REMOVE_BIT(ch->loc_hp,BROKEN_SKULL);
    }
    else if (IS_SET(ch->loc_hp,BROKEN_SPINE))
    {
	act("$n's spine knits itself back together.",ch,NULL,NULL,TO_ROOM);
	act("Your spine knits itself back together.",ch,NULL,NULL,TO_CHAR);
	REMOVE_BIT(ch->loc_hp,BROKEN_SPINE);
    }
    else if (IS_SET(ch->loc_hp,BROKEN_NECK))
    {
	act("$n's neck snaps back into place.",ch,NULL,NULL,TO_ROOM);
	act("Your neck snaps back into place.",ch,NULL,NULL,TO_CHAR);
      REMOVE_BIT(ch->loc_hp,BROKEN_NECK);
    }
    else if (IS_SET(ch->loc_hp2,LOST_ARM_L))
    {
	act("An arm grows from the stump of $n's left shoulder.",ch,NULL,NULL,TO_ROOM);
	act("An arm grows from the stump of your left shoulder.",ch,NULL,NULL,TO_CHAR);
	REMOVE_BIT(ch->loc_hp2,LOST_ARM_L);
	REMOVE_BIT(ch->loc_hp,BROKEN_ARM);
      REMOVE_BIT(ch->loc_hp2,BLEEDING_ARM);
	SET_BIT(ch->loc_hp,LOST_HAND);
    }
    else if (IS_SET(ch->loc_hp,LOST_ARM_R))
    {
	act("An arm grows from the stump of $n's right shoulder.",ch,NULL,NULL,TO_ROOM);
	act("An arm grows from the stump of your right shoulder.",ch,NULL,NULL,TO_CHAR);
	REMOVE_BIT(ch->loc_hp,LOST_ARM_R);
	REMOVE_BIT(ch->loc_hp,BROKEN_ARM);
      REMOVE_BIT(ch->loc_hp2,BLEEDING_ARM);
	SET_BIT(ch->loc_hp,LOST_HAND);
    }
    else if (IS_SET(ch->loc_hp2,LOST_LEG))
    {
	act("A leg grows from the stump of $n's hip.",ch,NULL,NULL,TO_ROOM);
	act("A leg grows from the stump of your hip.",ch,NULL,NULL,TO_CHAR);
	REMOVE_BIT(ch->loc_hp2,LOST_LEG);
	REMOVE_BIT(ch->loc_hp2,BROKEN_LEG);
      REMOVE_BIT(ch->loc_hp2,BLEEDING_LEG);
	SET_BIT(ch->loc_hp2,LOST_FOOT);
    }
    else if (IS_SET(ch->loc_hp,BROKEN_ARM))
    {
	act("$n's arm snaps back into place.",ch,NULL,NULL,TO_ROOM);
	act("Your arm snaps back into place.",ch,NULL,NULL,TO_CHAR);
	REMOVE_BIT(ch->loc_hp,BROKEN_ARM);
    }
    else if (IS_SET(ch->loc_hp2,BROKEN_LEG))
    {
	act("$n's left leg snaps back into place.",ch,NULL,NULL,TO_ROOM);
	act("Your left leg snaps back into place.",ch,NULL,NULL,TO_CHAR);
	REMOVE_BIT(ch->loc_hp2,BROKEN_LEG);
    }
    else if (IS_SET(ch->loc_hp,LOST_HAND))
    {
	act("A hand grows from the stump of $n's wrist.",ch,NULL,NULL,TO_ROOM);
	act("A hand grows from the stump of your wrist.",ch,NULL,NULL,TO_CHAR);
	REMOVE_BIT(ch->loc_hp,LOST_HAND);
	SET_BIT(ch->loc_hp2,LOST_THUMB);
	SET_BIT(ch->loc_hp,LOST_FINGER_I);
	SET_BIT(ch->loc_hp,LOST_FINGER_M);
	SET_BIT(ch->loc_hp,LOST_FINGER_R);
	SET_BIT(ch->loc_hp2,LOST_FINGER_L);
    }
    else if (IS_SET(ch->loc_hp2,LOST_FOOT))
    {
	act("A foot grows from the stump of $n's ankle.",ch,NULL,NULL,TO_ROOM);
	act("A foot grows from the stump of your ankle.",ch,NULL,NULL,TO_CHAR);
	REMOVE_BIT(ch->loc_hp2,LOST_FOOT);
      REMOVE_BIT(ch->loc_hp2,BLEEDING_FOOT);
    }
    else if (IS_SET(ch->loc_hp2,LOST_THUMB))
    {
	act("A thumb slides out of $n's hand.",ch,NULL,NULL,TO_ROOM);
	act("A thumb slides out of your hand.",ch,NULL,NULL,TO_CHAR);
	REMOVE_BIT(ch->loc_hp2,LOST_THUMB);
	REMOVE_BIT(ch->loc_hp2,BROKEN_THUMB);
    }
    else if (IS_SET(ch->loc_hp2,BROKEN_THUMB))
    {
	act("$n's thumb snaps back into place.",ch,NULL,NULL,TO_ROOM);
	act("Your thumb snaps back into place.",ch,NULL,NULL,TO_CHAR);
	REMOVE_BIT(ch->loc_hp2,BROKEN_THUMB);
    }
    else if (IS_SET(ch->loc_hp,LOST_FINGER_I))
    {
	act("An index finger slides out of $n's hand.",ch,NULL,NULL,TO_ROOM);
	act("An index finger slides out of your hand.",ch,NULL,NULL,TO_CHAR);
	REMOVE_BIT(ch->loc_hp,LOST_FINGER_I);
	REMOVE_BIT(ch->loc_hp2,BROKEN_FINGER_I);
    }
    else if (IS_SET(ch->loc_hp2,BROKEN_FINGER_I))
    {
	act("$n's index finger snaps back into place.",ch,NULL,NULL,TO_ROOM);
	act("Your index finger snaps back into place.",ch,NULL,NULL,TO_CHAR);
	REMOVE_BIT(ch->loc_hp2,BROKEN_FINGER_I);
    }
    else if (IS_SET(ch->loc_hp,LOST_FINGER_M))
    {
	act("A middle finger slides out of $n's hand.",ch,NULL,NULL,TO_ROOM);
	act("A middle finger slides out of your hand.",ch,NULL,NULL,TO_CHAR);
	REMOVE_BIT(ch->loc_hp,LOST_FINGER_M);
	REMOVE_BIT(ch->loc_hp2,BROKEN_FINGER_M);
    }
    else if (IS_SET(ch->loc_hp2,BROKEN_FINGER_M))
    {
	act("$n's middle finger snaps back into place.",ch,NULL,NULL,TO_ROOM);
	act("Your middle finger snaps back into place.",ch,NULL,NULL,TO_CHAR);
	REMOVE_BIT(ch->loc_hp2,BROKEN_FINGER_M);
    }
    else if (IS_SET(ch->loc_hp,LOST_FINGER_R))
    {
	act("A ring finger slides out of $n's hand.",ch,NULL,NULL,TO_ROOM);
	act("A ring finger slides out of your hand.",ch,NULL,NULL,TO_CHAR);
	REMOVE_BIT(ch->loc_hp,LOST_FINGER_R);
	REMOVE_BIT(ch->loc_hp2,BROKEN_FINGER_R);
    }
    else if (IS_SET(ch->loc_hp2,BROKEN_FINGER_R))
    {
	act("$n's ring finger snaps back into place.",ch,NULL,NULL,TO_ROOM);
	act("Your ring finger snaps back into place.",ch,NULL,NULL,TO_CHAR);
	REMOVE_BIT(ch->loc_hp2,BROKEN_FINGER_R);
    }
    else if ( IS_SET(ch->loc_hp2,LOST_FINGER_L))
    {
	act("A little finger slides out of $n's hand.",ch,NULL,NULL,TO_ROOM);
	act("A little finger slides out of your hand.",ch,NULL,NULL,TO_CHAR);
	REMOVE_BIT(ch->loc_hp2,LOST_FINGER_L);
	REMOVE_BIT(ch->loc_hp2,BROKEN_FINGER_L);
    }
    else if ( IS_SET(ch->loc_hp2,BROKEN_FINGER_L))
    {
	act("$n's little finger snaps back into place.",ch,NULL,NULL,TO_ROOM);
	act("Your little finger snaps back into place.",ch,NULL,NULL,TO_CHAR);
	REMOVE_BIT(ch->loc_hp2,BROKEN_FINGER_L);
    }
    else if (IS_SET(ch->loc_hp,CUT_THROAT))
    {
	act("The wound in $n's throat closes up.",ch,NULL,NULL,TO_ROOM);
	act("The wound in your throat closes up.",ch,NULL,NULL,TO_CHAR);
	REMOVE_BIT(ch->loc_hp,CUT_THROAT);
      REMOVE_BIT(ch->loc_hp2,BLEEDING_THROAT);
    }
    return;
}


/* ROOM PROGS */
/* These progs are activated per tic. */

void are_corrupt(CHAR_DATA *ch)
{
  if (IS_NPC(ch) || IS_IMMORTAL(ch)) 
    return;

  if ((ch->alignment > -1000) && (ch->alignment < 1000))
  {
    ch->alignment--;
    send_to_char("You feel evil corruption seep into your bones.\n\r",ch);
  }

#if 0 
  if ((IS_GOOD(ch) && ch->alignment > 250)
  || (IS_NEUTRAL(ch) && ch->alignment > -250)
  || (IS_EVIL(ch) && ch->alignment > -1000))
    ch->alignment--;
#endif

}

void are_sanctify(CHAR_DATA *ch)
{

  if ( IS_IMMORTAL(ch) || IS_NPC(ch))
    return;

  if ( ch->alignment < 1000 )
  {
    ch->alignment++;
    send_to_char("A feeling of sanctity warms your heart.\n\r",ch);
  }
}

void are_balance(CHAR_DATA *ch)
{

  if (IS_NPC(ch) || IS_IMMORTAL(ch) || (ch->alignment==0))
    return;

  if ((ch->alignment > 0) && (ch->alignment <1000))
  {
    ch->alignment--;
    send_to_char("You feel darkness return balance to your soul.\n\r",ch);
  }

  if ((ch->alignment < 0) && (ch->alignment > -1000))
  {
    ch->alignment++;
    send_to_char("You feel the light return balance to your soul.\n\r",ch);
  }
}

