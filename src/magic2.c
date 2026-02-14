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
#include <stat.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "merc.h"
#include "interp.h"
#include "magic.h"
#include "recycle.h"
#include "tables.h"
#include "lookup.h"
#include <unistd.h>
#include <stddef.h>
#include <dirent.h>

void    show_list_to_char  args((OBJ_DATA *list, CHAR_DATA *ch,
                                 bool fShort, bool fShowNothing));
void    show_char_to_char  args((CHAR_DATA *list, CHAR_DATA *ch));
void raw_kill		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
extern char *target_name;
extern  AFFECT_DATA	*affect_free;
void    add_memory_contents args ( ( CHAR_DATA *ch, EXTRA_DESCR_DATA *ed,  
				char *buf, CHAR_DATA *rch) );
bool check_dispel( int dis_level, CHAR_DATA *victim, int sn);

void spell_farsight( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (victim != ch)
	  return;


    if ( IS_AFFECTED2(victim, AFF_FARSIGHT) )
    {
        if (victim == ch)
        send_to_char("You can enchance your vision no further.\n\r",ch);
        return;
    }

    if (IS_AFFECTED(ch,AFF_BLIND))
    {
        send_to_char("Maybe it would help if you could see?\n\r",ch);
        return;
    }
 
    af.where	= TO_AFFECTS2;
    af.type      	= sn;
    af.level     	= level;
    af.duration  	= 24;
    af.location  	= APPLY_NONE;
    af.modifier  	= 0;
    af.bitvector 	= AFF_FARSIGHT;
    affect_to_char( victim, &af );
    send_to_char( "Your sight expands into the distance.\n\r", victim );
    return;
}


void spell_dimensional_door( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim;
    OBJ_DATA *portal, *stone;

        if ( ( victim = get_char_world( ch, target_name ) ) == NULL
    ||   victim == ch
    ||   ch->riding
    ||   victim->in_room == NULL
    ||   victim->in_room == ch->in_room
    ||   !can_see_room(ch,victim->in_room)
    ||   IS_SET(victim->in_room->room_flags, ROOM_SAFE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
    ||   IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
    ||   IS_SET(ch->in_room->room_flags,ROOM_NOMAGIC) //C076
    ||   IS_SET(victim->in_room->room_flags,ROOM_NOMAGIC)  //C076
    ||   IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
    ||   victim->level >= level + 3
    ||   (!IS_NPC(victim) && victim->level >= LEVEL_HERO)  /* NOT trust */
    ||   (IS_NPC(victim) && IS_SET(victim->imm_flags,IMM_SUMMON))
    ||   (IS_NPC(victim) && saves_spell( level, victim,DAM_NONE) ) 
    ||   (!IS_NPC(victim) && SAME_CLAN(ch,victim)) )
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }   

    stone = get_eq_char(ch,WEAR_HOLD);
    if (!IS_IMMORTAL(ch) 
    &&  (stone == NULL || stone->item_type != ITEM_WARP_STONE))
    {
	send_to_char("You lack the proper component for this spell.\n\r",ch);
	return;
    }

    if (stone != NULL && stone->item_type == ITEM_WARP_STONE)
    {
     	act("You draw upon the power of $p.",ch,stone,NULL,TO_CHAR);
     	act("It flares brightly and vanishes!",ch,stone,NULL,TO_CHAR);
     	extract_obj(stone);
    }

    portal = create_object(get_obj_index(OBJ_VNUM_PORTAL),0);
    portal->timer = 2 + level / 25; 
    portal->value[3] = victim->in_room->vnum;

    obj_to_room(portal,ch->in_room);

    act("$p rises up from the ground.",ch,portal,NULL,TO_ROOM);
    act("$p rises up before you.",ch,portal,NULL,TO_CHAR);
}

void spell_nexus( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim;
    OBJ_DATA *portal, *stone;
    ROOM_INDEX_DATA *to_room, *from_room;

    from_room = ch->in_room;
 
        if ( ( victim = get_char_world( ch, target_name ) ) == NULL
    ||   victim == ch
    ||   ch->riding
    ||   (to_room = victim->in_room) == NULL
    ||   victim->in_room == ch->in_room
    ||   !can_see_room(ch,to_room) || !can_see_room(ch,from_room)
    ||   IS_SET(to_room->room_flags, ROOM_SAFE)
    ||   IS_SET(from_room->room_flags,ROOM_SAFE)
    ||   IS_SET(to_room->room_flags, ROOM_PRIVATE)
    ||   IS_SET(to_room->room_flags, ROOM_SOLITARY)
    ||   IS_SET(to_room->room_flags, ROOM_NO_RECALL)
    ||   IS_SET(from_room->room_flags,ROOM_NO_RECALL)
    ||   IS_SET(from_room->room_flags,ROOM_NOMAGIC) //C076
    ||   IS_SET(to_room->room_flags,ROOM_NOMAGIC)  //C076
    ||   victim->level >= level + 3
    ||   (!IS_NPC(victim) && victim->level >= LEVEL_HERO))
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }   
 
    stone = get_eq_char(ch,WEAR_HOLD);
    if (!IS_IMMORTAL(ch)
    &&  (stone == NULL || stone->item_type != ITEM_WARP_STONE))
    {
        send_to_char("You lack the proper component for this spell.\n\r",ch);
        return;
    }
 
    if (stone != NULL && stone->item_type == ITEM_WARP_STONE)
    {
        act("You draw upon the power of $p.",ch,stone,NULL,TO_CHAR);
        act("It flares brightly and vanishes!",ch,stone,NULL,TO_CHAR);
        extract_obj(stone);
    }

    /* portal one */ 
    portal = create_object(get_obj_index(OBJ_VNUM_PORTAL),0);
    portal->timer = 1 + level / 10;
    portal->value[3] = to_room->vnum;
 
    obj_to_room(portal,from_room);
 
    act("$p rises up from the ground.",ch,portal,NULL,TO_ROOM);
    act("$p rises up before you.",ch,portal,NULL,TO_CHAR);

    /* no second portal if rooms are the same */
    if (to_room == from_room)
	return;

    /* portal two */
    portal = create_object(get_obj_index(OBJ_VNUM_PORTAL),0);
    portal->timer = 1 + level/10;
    portal->value[3] = from_room->vnum;

    obj_to_room(portal,to_room);

    if (to_room->people != NULL)
    {
	act("$p rises up from the ground.",to_room->people,portal,NULL,TO_ROOM);
	act("$p rises up from the ground.",to_room->people,portal,NULL,TO_CHAR);
    }
}

void spell_ethereal_warrior( int sn, int level, CHAR_DATA *ch, void *vo, int target ) 
{
  CHAR_DATA *gch;
  CHAR_DATA *warrior;
  AFFECT_DATA af;
  int i = 0;

  if (is_affected(ch,sn))
    {
      send_to_char("You lack the power to summon another ethereal warrior right now.\n\r",ch);
      return;
    }

  send_to_char("You attempt to summon an ethereal warrior.\n\r",ch);
  act("$n attempts to summon an ethereal warrior.",ch,NULL,NULL,TO_ROOM);

  for (gch = char_list; gch != NULL; gch = gch->next)
    {
      if (IS_NPC(gch) && IS_AFFECTED(gch,AFF_CHARM) && gch->master == ch &&
          ( gch->pIndexData->vnum == MOB_VNUM_ETHEREAL_WARRIOR ) )
	{
	    send_to_char("More ethereal warriors are more than you can control!\n\r",ch);
	    return;
	}
    }

  warrior = create_mobile( get_mob_index(MOB_VNUM_ETHEREAL_WARRIOR) );


  for (i = 0; i < MAX_STATS; i ++)
       warrior->perm_stat[i] = UMIN(25,15 + ch->level/10);
            
  warrior->perm_stat[STAT_STR] += 3;
  warrior->perm_stat[STAT_INT] -= 1;
  warrior->perm_stat[STAT_CON] += 2;

  warrior->max_hit =  level * 8.3;
  warrior->hit = warrior->max_hit;
  warrior->max_mana = level +200;
  warrior->mana = warrior->max_mana;
  warrior->level = ch->level - 2;
  for (i=0; i < 3; i++)
    warrior->armor[i] = interpolate(warrior->level,100,-100);
  warrior->armor[3] = interpolate(warrior->level,100,0);
  warrior->gold = 0;
  warrior->timer = 0;
  warrior->damage[DICE_NUMBER] = 13;   
  warrior->damage[DICE_TYPE] = 9;
  warrior->damage[DICE_BONUS] = ch->level / 2 + 10;

  char_to_room(warrior,ch->in_room);
  send_to_char("You summoned an ethereal warrior!\n\r",ch);
  act("$n summons an ethereal warrior!",ch,NULL,NULL,TO_ROOM);

  af.where			= TO_AFFECTS;
  af.type               = sn;
  af.level              = level; 
  af.duration           = 30;
  af.bitvector          = 0;
  af.modifier           = 0;
  af.location           = APPLY_NONE;
  affect_to_char(ch, &af);  

  SET_BIT(warrior->affected_by, AFF_CHARM);
  warrior->master = warrior->leader = ch;
  SET_BIT(warrior->act, ACT_PET);
  warrior->comm = COMM_NOTELL|COMM_NOSHOUT|COMM_NOCHANNELS;
  ch->pet = warrior;

}

void spell_shadow_walk ( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim;
    bool gate_pet;

    if (IS_AFFECTED(ch,AFF_BLIND))
    {
	send_to_char("Where do you think your going? Your blind as a bat.\n\r",ch);
	return;
    }

    if ( ch->quit_timer < 0
    && !IS_IMMORTAL(ch)
    && (number_percent() < 50 )) 
    {
	send_to_char("Your heart is beating so fast, you lose your concentration on the incantation.\n\r",ch);
	return;
    }

    if ( ( victim = get_char_world( ch, target_name ) ) == NULL
    ||   victim == ch
    ||   ch->riding
    ||   victim->in_room == ch->in_room
    ||   IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
    ||   IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
    ||   IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
    ||   victim->level >= ch->level + 3
    ||   (!IS_NPC(victim) && victim->level >= LEVEL_HERO))  /* NOT trust */
    {
        send_to_char( "The abysmal void shudders, then falters and closes.\n\r", ch );
        return;
    }
    if (( time_info.hour > 6) && (time_info.hour < 20))
    {
        send_to_char( "You may only traverse through the shadow plane at night.\n\r", ch );
        return;
    }
    if (ch->pet != NULL && ch->in_room == ch->pet->in_room && !ch->pet->tethered)
	gate_pet = TRUE;
    else
	gate_pet = FALSE;
    
    act("$n vanishes into an abysmal void of twisted shadows.",ch,NULL,NULL,TO_ROOM);
    send_to_char("You vanish into an abysmal void of twisted shadows.\n\r",ch);
    char_from_room(ch);
    char_to_room(ch,victim->in_room);
    act("$n emerges from an abysmal void of twisted shadows.",ch,NULL,NULL,TO_ROOM);
    do_function(ch, &do_look, "auto");

    if (gate_pet)
    {
      act("$n vanishes into an abysmal void of twisted shadows.",ch->pet,NULL,NULL,TO_ROOM);
      send_to_char("You vanish into an abysmal void of twisted shadows.\n\r",ch->pet);
      char_from_room(ch->pet);
      char_to_room(ch->pet,victim->in_room);
      act("$n emerges from an abysmal void of twisted shadows.",ch->pet,NULL,NULL,TO_ROOM);
	do_function(ch->pet, &do_look, "auto");
    }
}

void spell_animate_corpse( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    OBJ_DATA *obj;
    CHAR_DATA *mob, *gch;
    int i;
    int zombie_count = 0;

    obj = get_obj_here( ch, NULL, target_name );

    if ( obj == NULL )
    {
        send_to_char( "Animate what?\n\r", ch );
        return;
    }

    /* Nothing but NPC corpses. */
    if( obj->item_type != ITEM_CORPSE_NPC )
    {
        if( obj->item_type == ITEM_CORPSE_PC )
            send_to_char( "You can't ressurect players.\n\r", ch );
        else
            send_to_char( "It would serve no purpose...\n\r", ch );
        return;
    }

    if( obj->level > (ch->level + 2) )
    {
        send_to_char( "You couldn't call forth such a great spirit.\n\r", ch );
        return;
    } 

   if( ch->pet != NULL )
    {
        send_to_char( "You couldn't possibly animate a corpse while already controling other pets.\n\r", ch );
        return;
    }

   for (gch = char_list; gch != NULL; gch = gch->next)
   {
	if (IS_NPC(gch) && IS_AFFECTED(gch, AFF_CHARM) && gch->master == ch &&
          gch->pIndexData->vnum == MOB_VNUM_ZOMBIE)
	  		zombie_count++;
   }

   if (zombie_count >=1)
   {
	send_to_char("You cannot control more than one zombie at a time.\n\r",ch);
	return;
   }


    /* Chew on the zombie a little bit, recalculate level-dependant stats */
    mob 				= create_mobile( get_mob_index( MOB_VNUM_ZOMBIE ) );
    mob->level			= obj->level;
    mob->max_hit			= mob->level * 8 + number_range
    (mob->level * mob->level/4, mob->level * mob->level);
    mob->max_hit *= .9;
    mob->hit			= mob->max_hit;
    mob->max_mana			= 100 + dice(mob->level,10);
    mob->mana 			= mob->max_mana;
    for (i = 0; i < 3; i++)        
    mob->armor[i]			= interpolate(mob->level,100,-100);
    mob->armor[3]			= interpolate(mob->level,100,0);
    for (i = 0; i < MAX_STATS; i++)
    mob->perm_stat[i]		= 11 + mob->level/4;

    /* You rang? */
    char_to_room( mob, ch->in_room );
    act( "$p springs to life as a hideous zombie!", ch, obj, NULL, TO_ROOM );
    act( "$p springs to life as a hideous zombie!", ch, obj, NULL, TO_CHAR );
    extract_obj(obj);

    /* Yessssss, massssssster... */
    SET_BIT(mob->affected_by, AFF_CHARM);
    SET_BIT(mob->act, ACT_PET);
    mob->comm = COMM_NOTELL|COMM_NOSHOUT|COMM_NOCHANNELS;
    add_follower( mob, ch );
    mob->leader = ch;
    ch->pet = mob;

    /* For a little flavor... */
    do_say( mob, "Yessssss...massssssster...How may I serve you?" );
    return;
}

void spell_fear( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
  ROOM_INDEX_DATA *was_in;
  ROOM_INDEX_DATA *now_in;
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
  int attempt;

  if ( victim == NULL )
  {
    send_to_char( "Who would you like to put the fear of death into?\n\r", ch );
    return;
  }

  if (IS_AFFECTED(victim,AFF_BERSERK) 
  || is_affected(victim,skill_lookup("frenzy")))
  {
    act("$N is in a blind rage, it isn't going to work.\n\r",ch,NULL,victim,TO_CHAR);
    return;
  }

  if (IS_IMMORTAL(victim))
  {
    send_to_char("Immortals know no fear.\n\r",ch);
    return;
  }

  if ( IS_AFFECTED2( victim, AFF_FEAR ))
  {
    act("$N is already writhing in fear from your evil, black magic.",ch,NULL,victim,TO_CHAR);
    return;
  }

  if ( IS_AFFECTED3(victim, AFF_QUICKSAND) )
  {
    act("$N is waist deep in quicksand. Your illusion may scare $m, but won't frighten $m away.",ch,NULL,victim,TO_CHAR);
    return;
  }

  if ( !victim->in_room
  || victim == ch
  || IS_SET( victim->in_room->room_flags, ROOM_SAFE      )
  || IS_SET( victim->in_room->room_flags, ROOM_PRIVATE   )
  || IS_SET( victim->in_room->room_flags, ROOM_SOLITARY  )
  || IS_SET( victim->in_room->room_flags, ROOM_NO_RECALL )
  || victim->riding
  || victim->level >= level 
  || victim->in_room->area != ch->in_room->area )
  {
    send_to_char( "You failed.\n\r", ch );
    return;
  }

  was_in = victim->in_room;
  for ( attempt = 0; attempt < 6; attempt++ )
  {
    EXIT_DATA *pexit;
    int door;

    door = number_door( );
    if ( ( pexit = was_in->exit[door] ) == 0
    ||   pexit->u1.to_room == NULL
    ||   IS_SET(pexit->exit_info, EX_CLOSED)
    ||   number_range(0,ch->daze) != 0
    ||   IS_SET(pexit->u1.to_room->room_flags, ROOM_NO_MOB) )
      continue;

    act( "You utter a word of dark power and $N runs away in fear!", ch, NULL, victim, TO_CHAR    );
    act( "$n utters a word of dark power, unnerving fear grips your heart.",  ch, NULL, victim, TO_VICT    );

    move_char( victim, door, FALSE );
    if ( ( now_in = victim->in_room ) == was_in )
	continue;

    af.where	= TO_AFFECTS2;
    af.type		= sn;
    af.level	= level;
    af.duration  	= 0;
    af.modifier  	= 0;
    af.location 	= 0;
    af.bitvector 	= AFF_FEAR;
    affect_to_char( victim, &af );
    victim->in_room = was_in;
    victim->in_room = now_in;
    stop_fighting( victim, TRUE );
    return;
  }
}

void spell_horrid_wilting( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    OBJ_DATA *obj;

    obj = get_obj_here( ch, NULL, target_name );

    if ( obj == NULL )
    {
        send_to_char( "What would you like to draw power from?\n\r", ch );
        return;
    }

    /* Nothing but NPC corpses. */
    if( obj->item_type != ITEM_CORPSE_NPC )
    {
        if( obj->item_type == ITEM_CORPSE_PC )
            send_to_char( "You can not draw upon the power of other players.\n\r", ch );
        else
            send_to_char( "It would serve no purpose...\n\r", ch );
        return;
    } 

    ch->hit = UMIN( ch->hit + 50, ch->max_hit );
    ch->move = UMIN( ch->move + 20, ch->max_move );
    act( "$n sucks the soul out of $p, drawing in power!", ch, obj, NULL, TO_ROOM );
    act( "You suck the soul out of $p, drawing in the power!", ch, obj, NULL, TO_CHAR );
    extract_obj(obj);
    return;
}

void spell_paralysis( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    char buf[MAX_STRING_LENGTH];

    if (IS_NPC(victim))
    {
      act("You cannot paralyze that one.", ch, NULL, victim, TO_CHAR);
      return;
    }

    if ( ch == victim )
    {
        send_to_char( "Your body goes limp for a split second, perhaps you should try it on someone else?\n\r", ch );
        return;
    }

    if ( IS_AFFECTED2( victim, AFF_PARALYSIS ))
    {
        act("$N is already paralyzed.",ch,NULL,victim,TO_CHAR);
        return;
    }

    if (IS_AFFECTED3(victim, AFF_HOLDPERSON))
    {
      act("$N is already unable to act.", ch, NULL, victim, TO_CHAR);
      return;
    }

    if (victim->level <= (ch->level+4))
    {
      af.where		= TO_AFFECTS2;
      af.type      	= sn;
      af.level		= level + 5;
      af.duration  	= 4;
      af.modifier  	= 0;
      af.location  	= 0;
      af.bitvector 	= AFF_PARALYSIS;
      affect_to_char( victim, &af );
      act( "$N suddenly goes limp under your paralysis, leaving $m vulnerable to you!", ch, NULL, victim, TO_CHAR    );
      act( "Your entire body suddnely goes completely numb, rendering you paralyzed!",  ch, NULL, victim, TO_VICT    );
      act( "$n's dark powers have rendered $N completely paralyzed and vulnerable!",   ch, NULL, victim, TO_NOTVICT );
      return;
    }
    else
    {
      act("$n's magical attempt at paralyzing you failed!",ch,NULL,victim,TO_VICT);
      send_to_char("Your attempt at total paralysis failed horribly.\n\r",ch);
      if(!IS_NPC(ch))
      {     
        if (!can_see(victim,ch))
          do_yell(victim, "Help! Someone tried to paralyze me!");
        else
        {
          if (IS_NPC(ch))
            sprintf(buf, "Help! %s just tried to paralyze me!", ch->short_descr);
          else
            sprintf( buf, "Help! %s just tried to paralyze me!", 
            ((is_affected(ch,gsn_doppelganger)) && (!IS_IMMORTAL(victim)))? ch->doppel->name :
            ((is_affected(ch,gsn_treeform)) && (!IS_IMMORTAL(victim)))? "A mighty oak tree" :
            ch->name);
            do_yell( victim, buf );
        }
      }
	multi_hit( ch, victim, TYPE_UNDEFINED );
	return;
    }
}

void spell_vampiric_touch( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = number_range(1, 300);

    if ( ch->fight_pos > FIGHT_FRONT )  
    {
	send_to_char("You must be close enough to touch your victim in order to steal their life energy.\n\r", ch);
      act("$n fails to reach $s victim.", ch, NULL, NULL, TO_ROOM);
	return;
    }

    act("You feel your life being sucked away.",victim,NULL,NULL,TO_CHAR);
    damage(ch,victim,number_range( dam, (ch->level*1.7) ),sn,DAM_NEGATIVE,TRUE);
    ch->alignment = UMAX(-1000,ch->alignment - 1);
    ch->hit += dam *.2;

    return;
}

void spell_undead_disruption(int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    if ( !IS_NPC(ch) && !IS_EVIL(ch) )
    {
        victim = ch;
	send_to_char("The undead turn upon you!\n\r",ch);
    }

    ch->alignment = UMAX(-1000, ch->alignment - 50);

    if (victim != ch)
    {
      act("$n calls forth undead creatures from the ground upon $N!",ch,NULL,victim,TO_ROOM);
      act("$n points a finger at you, assailing you with undead creatures!",ch,NULL,victim,TO_VICT);
      send_to_char("You raise a few undead creatures to do your bidding!\n\r",ch);
    }
    dam = dice( level+45, 10 );
    if ( saves_spell( level, victim,DAM_NEGATIVE) )
        dam /= 2;
    damage( ch, victim, dam, sn, DAM_NEGATIVE ,TRUE);
}

void spell_nightmare( int sn, int level, CHAR_DATA *ch, void *vo, int target ) 
{
  CHAR_DATA *gch;
  CHAR_DATA *nightmare;
  int i = 0;

  send_to_char("You attempt to summon a nightmare.\n\r",ch);
  act("$n attempts to summon a frightful nightmare.",ch,NULL,NULL,TO_ROOM);

  for (gch = char_list; gch != NULL; gch = gch->next)
    {
      if (IS_NPC(gch) && IS_AFFECTED(gch,AFF_CHARM) && gch->master == ch &&
          ( gch->pIndexData->vnum == MOB_VNUM_NIGHTMARE ) )
	{
	    send_to_char("Summoning more than one nightmare is beyond your control!\n\r",ch);
	    return;
	}
    }

  nightmare = create_mobile( get_mob_index(MOB_VNUM_NIGHTMARE) );


  for (i = 0; i < MAX_STATS; i ++)
  nightmare->perm_stat[i] = UMIN(25,15 + ch->level/15);           
  nightmare->perm_stat[STAT_STR] += 1;
  nightmare->perm_stat[STAT_INT] -= 1;
  nightmare->perm_stat[STAT_CON] += 2;

  nightmare->max_hit 	*= 12.5;
  nightmare->hit		= nightmare->max_hit;
  nightmare->max_mana	= 100 + dice(nightmare->level,10);
  nightmare->mana		= nightmare->max_mana;
  nightmare->level 	= 20;
  for (i=0; i < 3; i++)
  nightmare->armor[i]  = interpolate(nightmare->level,10,-10);
  nightmare->armor[3] = interpolate(nightmare->level,10,0);
  nightmare->gold = 0;
  nightmare->timer = 0;
  nightmare->damage[DICE_NUMBER] = 2;   
  nightmare->damage[DICE_TYPE] = 9;
  nightmare->damage[DICE_BONUS] = ch->level / 4 + 5;

  char_to_room(nightmare,ch->in_room);
  send_to_char("Darkness clouds the area then leaves, leaving a fearful nightmare behind!\n\r",ch);
  act("Dark clouds enter then leave! A frightful nightmare now stands by $n!",ch,NULL,NULL,TO_ROOM);


  SET_BIT(nightmare->affected_by, AFF_CHARM);
  SET_BIT(nightmare->act, ACT_PET);
  nightmare->comm = COMM_NOTELL|COMM_NOSHOUT|COMM_NOCHANNELS;
  add_follower( nightmare, ch );
  nightmare->leader = ch;
  ch->pet = nightmare;

}

void spell_cavorting_bones( int sn, int level, CHAR_DATA *ch, void *vo, int target ) 
{
  CHAR_DATA *gch;
  CHAR_DATA *skeleton;
  AFFECT_DATA af;
  int i;
  int sect;
  int skeleton_count = 0;

  sect = ch->in_room->sector_type;

  if (sect != SECT_GRAVEYARD)
  {
    send_to_char("You do not see any graves suitable for your incantation here.\n\r",ch);
    return; 
  }

  if ( is_affected(ch, sn))
  {
	send_to_char("You must recover before you can raise another skeleton from the graves.\n\r", ch);
	return;
  }

  for (gch = char_list; gch != NULL; gch = gch->next)
  {
    if (IS_NPC(gch) && IS_AFFECTED(gch, AFF_CHARM)
    && gch->master == ch
    && gch->pIndexData->vnum == MOB_VNUM_SKELETON)
	  skeleton_count ++;
  }

  if (skeleton_count >=3)
  {
    send_to_char("You cannot control more than three skeletons at a time.\n\r",ch);
    return;
  }

  send_to_char("You gesture the symbol of unlife above a nearby grave.\n\r", ch);
  act("$n makes some necromatic gestures above a nearby grave.", ch, NULL, NULL, TO_ROOM);

  skeleton = create_mobile( get_mob_index(MOB_VNUM_SKELETON));

  for (i = 0; i < MAX_STATS; i ++)
       skeleton->perm_stat[i] = UMIN(25,15 + ch->level/10);
            
  skeleton->perm_stat[STAT_STR] += 3;
  skeleton->perm_stat[STAT_INT] -= 1;
  skeleton->perm_stat[STAT_CON] += 2;

  skeleton->max_hit =  level * 11.3;
  skeleton->hit = skeleton->max_hit;
  skeleton->max_mana = level +200;
  skeleton->mana = skeleton->max_mana;
  skeleton->level = number_range(ch->level - 6, ch->level - 1);
  for (i=0; i < 3; i++)
  skeleton->armor[i] = interpolate(skeleton->level,100,-100);
  skeleton->armor[3] = interpolate(skeleton->level,100,0);
  skeleton->gold = 0;
  skeleton->timer = 0;
  skeleton->sex = 0;
  skeleton->damage[DICE_NUMBER] = 4;   
  skeleton->damage[DICE_TYPE] = 6;
  skeleton->damage[DICE_BONUS] = ch->level / 2 + 20;   
  send_to_char("As you complete the gesture, a skeleton emerges from the cold, hard earth.\n\r",ch);
  act("$n completes the gesture as a skeleton emerges from the cold, hard earth.", ch, NULL, NULL, TO_ROOM);
  char_to_room(skeleton, ch->in_room);

  af.type               = sn;
  af.level              = level; 
  af.duration           = 24;
  af.bitvector          = 0;
  af.modifier           = 0;
  af.location           = APPLY_NONE;
  affect_to_char(ch, &af);  

  SET_BIT(skeleton->affected_by, AFF_CHARM);
  SET_BIT(skeleton->act, ACT_PET);
  skeleton->comm = COMM_NOTELL|COMM_NOSHOUT|COMM_NOCHANNELS;
  add_follower( skeleton, ch );
  skeleton->leader = ch;
  ch->pet = skeleton;
}

void spell_enervation(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    act("$n releases a black bolt of negative energy at $N.",ch,NULL,victim,TO_NOTVICT);
    act("$n releases a black bolt of negative energy at you!",ch,NULL,victim,TO_VICT);
    act("You release a black bolt of negative energy at $N.",ch,NULL,victim,TO_CHAR);
 
    dam = number_range( 30, 180 );
    if ( saves_spell( level, victim, DAM_NEGATIVE) )
        dam /= 2;
    damage( ch, victim, dam, sn, DAM_NEGATIVE ,TRUE);
    return;
}

void spell_undetectable_align( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( ch, sn ) )
    {
	send_to_char("Your already concealing your alignment.\n\r",ch); 
	return;
    }

    af.where	= TO_AFFECTS2;
    af.type      	= sn;
    af.level	 	= level;
    af.duration 	= 8;
    af.modifier 	= 0;
    af.location 	= 0;
    af.bitvector 	= AFF_UNDETECTABLE_ALIGNMENT;
    affect_to_char( victim, &af );
    send_to_char( "You conceal your alignment from others.\n\r", victim );
    return;
}

void spell_ice_shield( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
	send_to_char("You are already shielded from fire attacks.\n\r",ch);
	return;
    }

    af.where	= TO_RESIST;
    af.type      	= sn;
    af.level     	= level;
    af.duration  	= level / 8;
    af.location  	= APPLY_NONE;
    af.modifier  	= 0;
    af.bitvector 	= RES_FIRE;
    affect_to_char( victim, &af );
    send_to_char( "You are surrounded by a shield of ice.\n\r", victim );
    act( "$n is surrounded by a shield of ice.", victim, NULL, NULL, TO_ROOM );
    return;

}

void spell_repulsion( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED2( victim, AFF_REPULSION ))
    {
	if (victim == ch)
	  send_to_char("Your already surrounded by iron bars inset in statuettes.\n\r",ch);
	else
	  act("$N is already protected from aggressive monsters.",ch,NULL,victim,TO_CHAR);
        return;
    }

    af.where		= TO_AFFECTS2;
    af.type      		= sn;
    af.level	 	= level - 5;
    af.duration  		= 5;
    af.modifier  		= 0;
    af.location  		= 0;
    af.bitvector 		= AFF_REPULSION;
    affect_to_char( victim, &af );
    send_to_char( "A pair of small iron bars attached to two small canine statuettes surround you.\n\r", victim );
    act( "$n is surrounded by iron bars inset in two small canine statuettes.", victim, NULL, NULL, TO_ROOM );
    return;
}

void spell_firestorm(int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    if (ch == victim)
      {
        send_to_char( "Suicide is a mortal sin. Do you really want to meet Hiddukel so soon?\n\r", ch );
        return;
      }

    send_to_char( "You call down a storm of fire!\n\r", ch );
    act( "$n calls down a storm of fire!", ch, NULL, NULL, TO_ROOM );

    dam = dice(40 + 20, 20);

  spell_incendiary_cloud(skill_lookup("incendiary cloud"), level,ch,(void *) victim,TARGET_CHAR);
  damage( ch, victim, dam, sn, DAM_FIRE,TRUE );

    dam = dice(25 + 20, 20);

  spell_incendiary_cloud(skill_lookup("incendiary cloud"), level,ch,(void *) victim,TARGET_CHAR);
  damage( ch, victim, dam, sn, DAM_FIRE,TRUE );

    dam = dice(15 + 20, 20);

  damage( ch, victim, dam, sn, DAM_FIRE,TRUE );

    return;
}

void spell_break_enchantment( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;

    if ( victim == ch )
    {
	send_to_char( "Break the enchantment NOT on you? Get real!\n\r", ch );
	return;
    }

    if  (!IS_NPC(victim))
    {
	send_to_char( "They probably wouldn't like that too much!\n\r", ch );
	return;
    }

    if ( !IS_AFFECTED( victim, AFF_CHARM ))
    {
	send_to_char( "They don't seem to be under any kind of enchantment spell!\n\r", ch );
	return;
    }

    act("$n breaks the enchantment $N was under!",ch,NULL,victim,TO_ROOM);
    if ( victim->fighting == ch )
	stop_fighting( victim, TRUE );
    if ( victim->master )
        	stop_follower( victim );
    act( "You break the enchantment spell $N was under!", ch, NULL, victim, TO_CHAR    );
    	return;
}

void spell_sympathy( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED2( victim, AFF_SYMPATHY ))
    {
	if (victim == ch)
      {
	  send_to_char("Magical vibrations already eminate around you.\n\r",ch);
        return;
      }
	else
      {
	  act("$N is already eminating with magical vibrations.",ch,NULL,victim,TO_CHAR);
        return;
      }
    }

    af.where	= TO_AFFECTS2;
    af.type      	= sn;
    af.level	= level - 10;
    af.duration  	= 3;
    af.modifier  	= 0;
    af.location  	= 0;
    af.bitvector 	= AFF_SYMPATHY;
    affect_to_char( victim, &af );
    send_to_char( "Magical vibrations begin to eminate from within you.\n\r", victim );
    act( "$n begins to eminate magical vibrations.", victim, NULL, NULL, TO_ROOM );
    return;
}

void spell_wind_wall( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_AFFECTED(ch,AFF_BLIND))
    {
	send_to_char("Cast the spell on whom? Your blind as a bat.\n\r",ch);
	return;
    }
 
    if ( is_affected( victim, sn ) || IS_AFFECTED(victim,AFF_SLOW))
    {
        if (victim == ch)
          send_to_char("The forces of nature seem to ignore your request!\n\r",ch);
        else
          act("$N already appears to be moving slow enough.",ch,NULL,victim,TO_CHAR);
        return;
    }
 
    if (saves_spell(level,victim,DAM_OTHER) 
    ||  IS_SET(victim->imm_flags,IMM_MAGIC))
    {
	if (victim != ch)
            send_to_char("Nothing seemed to happen.\n\r",ch);
        send_to_char("You feel a slight breeze.\n\r",victim);
        return;
    }
 
    if (IS_AFFECTED(victim,AFF_HASTE))
    {
        act("$n becomes engulfed in strong winds, making $m move a little slower.",victim,NULL,NULL,TO_ROOM);
        return;
    }
 

    af.where     	= TO_AFFECTS;
    af.type      	= sn;
    af.level     	= level;
    af.duration 	= 2;
    af.location  	= APPLY_DEX;
    af.modifier 	= -5;
    af.bitvector 	= AFF_SLOW;
    affect_to_char( victim, &af );
    send_to_char( "A great wall of wind blows around you, slowing your movements down.\n\r", victim );
    act("$n becomes engulfed in a wall of wind, slowing $m down.",victim,NULL,NULL,TO_ROOM);
    return;
}

void spell_gust_of_wind( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

   if ( victim->position == POS_FIGHTING)
    {
        if (victim == ch)
          send_to_char("You can not seem to get airborne while fighting.\n\r",ch);
        else
          act("$N can not start flying in the heat of battle.",ch,NULL,victim,TO_CHAR);
        return;
    }

    if ( IS_AFFECTED(victim, AFF_FLYING) )
    {
	if (victim == ch)
	  send_to_char("Your already floating in the air.\n\r",ch);
	else
	  act("$N doesn't need your help to float in the air.",ch,NULL,victim,TO_CHAR);
	return;
    }

    if ( IS_AFFECTED3(victim, AFF_QUICKSAND) )
    {
	send_to_char("Your waist deep in quicksand, you couldn't possibly get airborne!\n\r",ch);
      return;
    }

    af.where    	= TO_AFFECTS;
    af.type      	= sn;
    af.level     	= level;
    af.duration  	= level / 3;
    af.location  	= 0;
    af.modifier  	= 0;
    af.bitvector 	= AFF_FLYING;
    affect_to_char( victim, &af );
    send_to_char( "A great gust of wind blows in and lifts your feet off the ground.\n\r", victim );
    act( "A great gust of wind blows in and lifts $n's feet off the ground.", victim, NULL, NULL, TO_ROOM );
    return;
}

void spell_unseen_servant( int sn, int level, CHAR_DATA *ch, void *vo, int target ) 
{
  CHAR_DATA *gch;
  CHAR_DATA *servant;
  AFFECT_DATA af;
  int i = 0;

  if (is_affected(ch,sn))
    {
      send_to_char("You lack the power to conjure another servant.\n\r",ch);
      return;
    }

  send_to_char("You attempt to conjure forth an unseen servant.\n\r",ch);
  act("$n attempts to conjure forth an unseen servant.",ch,NULL,NULL,TO_ROOM);

  for (gch = char_list; gch != NULL; gch = gch->next)
    {
      if (IS_NPC(gch) && IS_AFFECTED(gch,AFF_CHARM) && gch->master == ch &&
          ( gch->pIndexData->vnum == MOB_VNUM_SERVANT ) )
	{
	    send_to_char("Any more servants would be beyond your control!\n\r",ch);
	    return;
	}
    }

  servant = create_mobile( get_mob_index(MOB_VNUM_SERVANT) );


  for (i = 0; i < MAX_STATS; i ++)
       servant->perm_stat[i] = UMIN(25,15 + ch->level/10);
            
  servant->perm_stat[STAT_STR] += 3;
  servant->perm_stat[STAT_INT] -= 1;
  servant->perm_stat[STAT_CON] += 2;

  servant->max_hit =  level * 11.5;
  servant->hit = servant->max_hit;
  servant->max_mana = level +200;
  servant->mana = servant->max_mana;
  servant->level = number_range(ch->level - 10, ch->level - 1);
  for (i=0; i < 3; i++)
  servant->armor[i] = interpolate(servant->level,100,-100);
  servant->armor[3] = interpolate(servant->level,100,0);
  servant->gold = 0;
  servant->timer = 0;
  servant->damage[DICE_NUMBER] = 14;   
  servant->damage[DICE_TYPE] = 11;
  servant->damage[DICE_BONUS] = ch->level / 2 + 19;

  char_to_room(servant,ch->in_room);
    act("The fabric of time seems to rip open as $n conjures forth an unseen servant.",ch,NULL,NULL,TO_ROOM);
    send_to_char("Your conjuration of an unseen servant to do your bidding is successful.\n\r",ch);

  af.where	= TO_AFFECTS;
  af.type		= sn;
  af.level		= level; 
  af.duration	= 25;
  af.bitvector	= 0;
  af.modifier	= 0;
  af.location	= APPLY_NONE;
  affect_to_char(ch, &af);  

  SET_BIT(servant->affected_by, AFF_CHARM);
  SET_BIT(servant->act, ACT_PET);
  servant->comm = COMM_NOTELL|COMM_NOSHOUT|COMM_NOCHANNELS;
  add_follower( servant, ch );
  servant->leader = ch;
  ch->pet = servant;

}

void spell_rainbow_pattern( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *vch;
    int mlevel = 0;
    int count = 0;
    int high_level = 0;    
    int chance;
    AFFECT_DATA af;

    /* get sum of all mobile levels in the room */
    for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
    {
	if (vch->position == POS_FIGHTING)
	{
	    count++;
	    if (IS_NPC(vch))
	      mlevel += vch->level;
	    else
	      mlevel += vch->level/2;
	    high_level = UMAX(high_level,vch->level);
	}
    }

    /* compute chance of stopping combat */
    chance = 4 * level - high_level + 2 * count;

    if (IS_IMMORTAL(ch)) /* always works */
      mlevel = 0;

    send_to_char("You wave your hand in the air, creating a brilliant light shimmering with rainbow-hued colors, in an attempt to captivate everyone's attention around you.\n\r",ch);

    if (number_range(0, chance) >= mlevel)  /* hard to stop large fights */
    {
	for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
   	{
	    if (IS_NPC(vch)
	    && (IS_SET(vch->imm_flags,IMM_MAGIC)
	    ||  IS_SET(vch->act,ACT_UNDEAD)))
	      return;

	    if (IS_AFFECTED(vch,AFF_CALM)
	    ||  IS_AFFECTED2(vch,AFF_RAINBOW_PATTERN)
	    ||  IS_AFFECTED(vch,AFF_BERSERK)
	    ||  is_affected(vch,skill_lookup("frenzy")))
	      return;

	    if (vch->fighting || vch->position == POS_FIGHTING)
	      stop_fighting(vch,FALSE);

	    if ((vch != ch)
	    &&  !IS_IMMORTAL(vch))
	    {
	      send_to_char("A glowing, rainbow-hued pattern of internal weaving colors captivates you.\n\r",vch);

	      af.where 		= TO_AFFECTS2;
	      af.type 		= sn;
  	      af.level 	      = level;
	      af.duration 	= 2;
	      af.location 	= APPLY_HITROLL;
	      if (!IS_NPC(vch))
	        af.modifier 	= -5;
	      else
	        af.modifier 	= -2;
	      af.bitvector 	= AFF_RAINBOW_PATTERN;
	      affect_to_char(vch,&af);

	      af.location	= APPLY_DAMROLL;
	      affect_to_char(vch,&af);
	    }
	}
    }
}

void spell_sunbeam( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;

  if ( !IS_OUTSIDE(ch) )
    {
      send_to_char( "You must be out of doors.\n\r", ch );
      return;
    }

  if ( weather_info.sky > SKY_CLOUDLESS )
    {
      send_to_char( "You need good sunny weather to cast this spell.\n\r", ch );
      return;
    }

    act("$n directs a dazzling beam of intensely hot light at $N!",ch,NULL,victim,TO_NOTVICT);
    act("$n directs a dazzling beam of intensely hot light at you!",ch,NULL,victim,TO_VICT);
    act("You direct a dazzling beam of intensely hot light at $N!",ch,NULL,victim,TO_CHAR);
    dam = dice(40 + 20, 20);
  if ( saves_spell( level, victim, DAM_LIGHT ) )
    dam /= 2;
  damage( ch, victim, dam, sn,DAM_LIGHT,TRUE);
  return;
}


void spell_cantrip( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    char type[MAX_INPUT_LENGTH];
    char argument[MAX_INPUT_LENGTH];
    AFFECT_DATA af;
    OBJ_DATA *flower;
    OBJ_DATA *scroll;
    OBJ_DATA *bouquet;
    OBJ_DATA *light;
    OBJ_DATA *food; 

    target_name = one_argument( target_name, type );
    target_name = one_argument( target_name, argument );
    send_to_char("\n\r",ch);

    if( !str_cmp(type, "flower") )
    {
        flower = create_object( get_obj_index( OBJ_VNUM_FLOWER ), 0 );
        flower->timer = level;
        act("With a flash of light, $p appears in $n's hand.", ch, flower, NULL, TO_ROOM );
        act("You create $p.", ch, flower, NULL, TO_CHAR );
        obj_to_char( flower, ch );
        return;
    }
    else if( !str_cmp(type, "scroll") )
    {
        scroll = create_object( get_obj_index( OBJ_VNUM_SCROLL ), 0 );
        act("In $n's open hand, $p suddenly appears.", ch, scroll, NULL,TO_ROOM );
        act("You create $p.", ch, scroll, NULL, TO_CHAR );
        obj_to_char( scroll, ch );
        return;
    }
    else if( !str_cmp( type, "aura") )
    {
	if (IS_AFFECTED(ch, AFF_SANCTUARY))
	{
	    send_to_char("Your current aura already shines brightly enough.\n\r",ch);
	    return;
	}
    	  af.where     = TO_AFFECTS;
    	  af.type      = sn;
    	  af.level     = level;
    	  af.duration  = 0;
    	  af.location  = APPLY_NONE;
    	  af.modifier  = 0;
    	  af.bitvector = AFF_SANCTUARY;
        affect_to_char( ch, &af );
        send_to_char( "An aura of oscillating color surrounds you.\n\r",ch );
        act( "An aura of oscillating color surrounds $n.", ch, NULL,NULL, TO_ROOM );   
        return;
    }
    else if( !str_cmp(type, "bouquet") )
    {
        bouquet = create_object( get_obj_index( OBJ_VNUM_BOUQUET ), 0 );
        bouquet->timer = level;
        act("$n sweeps $s arms in a grand motion, and a colorful bouquet appears.", 
          ch, bouquet, NULL, TO_ROOM );
        act("You create $p.", ch, bouquet, NULL, TO_CHAR );
        obj_to_char( bouquet, ch );
        return;
    }
    else if( !str_cmp(type, "food") )
    {
        food = create_object( get_obj_index( OBJ_VNUM_FOOD ), 0 );
        food->timer = level;
        act("$n snaps $s fingers and a mouthwatering pear appears out of thin air.", 
          ch, food, NULL, TO_ROOM );
        act("You create $p.", ch, food, NULL, TO_CHAR );
        obj_to_char( food, ch );
        return;
    }
    else if( !str_cmp(type, "fireworks") )
    {
        act( "You make a stunning burst of flashing colors appear.", ch, NULL, NULL,TO_CHAR );
        act( "A stunning burst of flashing colors appears all around you.", 
           ch, NULL, NULL,TO_ROOM );
        return;
    }
    else if( !str_cmp(type, "juggle") )
    {
       act( "You make seven glowing sticks of wood juggle themselves.", ch, NULL, NULL,TO_CHAR );
       act( "Seven glowing sticks of wood juggle themselves.", ch, NULL, NULL, TO_ROOM );
       return;
    }
    else if( !str_cmp(type, "music") )
    {
      act("You make a haunting melody to entertain all who hear it.", ch, NULL, NULL,TO_CHAR );
      act("A haunting melody of no physical origin tickles your ears.", ch, NULL, NULL,TO_ROOM );
      return;
    }
    else if( !str_cmp(type, "glow") )
    {
        act( "With a wave of your hand, your body takes on a shimmering golden glow.", 
         ch,NULL, NULL, TO_CHAR );
        act( "With wave of $s hand, $n's body takes on a shimmering golden glow.", 
         ch, NULL,NULL, TO_ROOM );
        return;
    }
    else if( !str_cmp(type, "smoke") )
    {
        act( "Bursts of purple smoke appear all around you.", ch, NULL, NULL, TO_CHAR );
        act( "Bursts of purple smoke appear all around $n.", ch, NULL, NULL, TO_ROOM );
        return;
    }
    else if( !str_cmp(type, "warm") )
    {
        act( "You make the room heat up considerably.", ch, NULL, NULL, TO_CHAR );
        act( "Suddenly you're sweating, as the air has gotten much hotter.", 
         ch, NULL, NULL, TO_ROOM );
        return;
    }
    else if( !str_cmp(type, "rainbow") )
    {
        act( "You gesture in a semi-circle, and a rainbow appears across the sky.", 
          ch, NULL, NULL, TO_CHAR );
        act( "$n gestures in a semi-circle, and a rainbow arcs across the sky.", 
         ch, NULL, NULL, TO_ROOM );
        return;
    }
    else if( !str_cmp(type, "dizzy") )
    {
        act( "As you recite the word, the world seems to spin quickly for a moment.", 
         ch, NULL, NULL, TO_CHAR );
        act( "With a low voice $n recites a word and for a moment, the world seems to spin.", 
         ch, NULL, NULL, TO_ROOM );
        return;
    }
    else if( !str_cmp(type, "shadows") )
    {
        act( "Shadows swirl and shimmer violently around you.", 
         ch, NULL, NULL, TO_CHAR );
        act( "Shadows swirl and shimmer violently making your eyes hurt.", 
         ch, NULL, NULL, TO_ROOM );
        return;
    }
    else if( !str_cmp(type, "wind") )
    {
        act( "Spinning the air with a finger, you cause a great wind to enter the room.", 
          ch, NULL, NULL, TO_CHAR );
        act( "Suddenly, a blustery wind whips by, tearing at your clothes, and then dies away.",
          ch, NULL, NULL, TO_ROOM );
        return;
    }
    else if( !str_cmp(type, "laugh") )
    {
      act( "As the words leave your mouth, the air is filled with haunting laughter.", 
          ch, NULL, NULL, TO_CHAR );
      act("The air is suddenly filled with haunting jovial laughter.", ch, NULL, NULL, TO_ROOM );
      return;
    }
    else if( !str_cmp(type, "cool") )
    {
        act( "You make the room cool down considerably.", ch, NULL, NULL, TO_CHAR );
        act( "The surrounding air has gotten much cooler.", ch, NULL,NULL, TO_ROOM );
        return;
    }
    else if( !str_cmp(type, "fright") )
    {
        act( "The air suddenly blares with loud atonal music.", ch, NULL, NULL, TO_CHAR );
        act( "Loud atonal music suddenly blares up, causing you to jump.", 
          ch, NULL, NULL, TO_ROOM );
        return;
    }
    else if( !str_cmp(type, "hue") )
    {
        act( "You cause the air to shimmer with a wide spectrum of colors.", 
          ch, NULL, NULL, TO_CHAR );
        act( "A scope of color bursts forth, shimmering for one dazzling moment.", 
          ch, NULL, NULL, TO_ROOM );
        return;
    }
    else if( !str_cmp(type, "love") )
    {
       act( "With the words, you magically embrace those in the room lovingly.", 
          ch, NULL, NULL, TO_CHAR );
       act( "A tender loving sensation magically caresses your body.", ch, NULL, NULL, TO_ROOM );
       return;
    }
    else if( !str_cmp(type, "waterdance") )
    {
        act( "Little streams and beams of water shoot from one hand to another.", 
          ch, NULL, NULL, TO_CHAR );
        act( "Little streams and beams of water shoot from one of $s's hands to another.", 
          ch, NULL, NULL, TO_ROOM );
        return;
    }
    else if( !str_cmp(type, "stench") )
    {
        act( "Plugging your nose, you create a nasty smelling cloud.", ch, NULL, NULL, TO_CHAR );
        act( "An overwhelming odour drifts by, causing you to gag.", ch, NULL, NULL, TO_ROOM );
        return;
    }
    else if( !str_cmp(type, "drench") )
    {
        act( "Thrusting your hand forward, you let loose a high-pressure stream of water.", 
          ch, NULL, NULL, TO_CHAR );
        act( "You are drenched to the bone as $n shoots a stream of water from $s palm.", 
         ch, NULL, NULL, TO_ROOM );
        return;
    }
    else if( !str_cmp(type, "flare") )
    {
        act( "With your hand extended, a blue flame flares up from your fingertips, then dies.",
          ch, NULL, NULL, TO_CHAR );
        act( "$n extends $s palm, and a blue flame flares from $s fingertips, then dies.", 
          ch, NULL, NULL, TO_ROOM );
        return;
    }
    else if( !str_cmp(type, "leaves") )
    {
        act( "A cloud of colorful leaves spiral through the air around you before vanishing.", 
         ch, NULL, NULL, TO_CHAR );
        act( "A cloud of colorful leaves spiral through the air around $n before vanishing.", 
         ch, NULL, NULL, TO_ROOM );
        return;
    }
    else if( !str_cmp(type, "whirlwind") )
    {
        act( "You conjure a tiny whirlwind which spins about harmlessly on the ground.", 
          ch, NULL, NULL, TO_CHAR );
        act( "A tiny whirlwind spins about on the ground, terrorizing little bugs.", 
          ch, NULL, NULL, TO_ROOM );
        return;
    }
    else if( !str_cmp(type, "flamedance") )
    {
        act( "A tiny human-shaped flame appears in your palm, and dances for a moment.", 
          ch, NULL, NULL, TO_CHAR );
        act( "A tiny human-shaped flame appears in $n's palm, and dances for a moment.", 
          ch, NULL, NULL, TO_ROOM );
        return;
    }

    else if( !str_cmp(type, "cook") )
    {
        act( "You hold your hand flat forming a green flame that cooks the food.",
          ch, NULL, NULL, TO_CHAR );
        act( "With a flaming hand, $n cooks $s food, filling the area with a tantalizing odor.",
          ch, NULL, NULL, TO_ROOM );
        return;
    }
    else if( !str_cmp(type, "flamefinger") )
    {
        act( "Your index finger produces a flame when stroked with your thumb while saying the ancient word, 'Zippo.'", ch, NULL, NULL, TO_CHAR );
        act( "$n produces a flame off the tip of $s index finger.", ch, NULL, NULL, TO_ROOM );
        return;
    }
    else if( !str_cmp(type, "halo") )
    {
        act( "Holding your hands together, you look slightly up and to the left.  A bright halo appears above your head.", ch, NULL, NULL, TO_CHAR );
        act( "You hear a choir singing in the distance.  A bright halo appears above $s head.", ch, NULL, NULL, TO_ROOM );
        return;
    }

    else if( !str_cmp(type, "spotlight") )
    {   
    	act( "A bright light appears just above you, letting all know you are here.",
         ch,NULL, NULL, TO_CHAR );
    	act( "A very bright spot of light appears above $n's head.", ch, NULL, NULL, TO_ROOM );
    	return;
    }
    
    else if( !str_cmp(type, "fanfare") )
    {
      act( "A loud trumpet seems to herald something significant.", ch, NULL, NULL, TO_CHAR );
      act( "A loud trumpet seems to herald something significant.", ch, NULL, NULL, TO_ROOM );
      return;
    }
    else if( !str_cmp(type, "thunder") )
    {
      act( "A rumble of thunder echos in the distance.", ch, NULL, NULL, TO_CHAR );
      act( "You look around for storm as you hear the sound of thunder in the distance.", 
       ch, NULL, NULL, TO_ROOM );
       return;
    }
    else if( !str_cmp(type, "freshen") )
    {
    act( "You breathe deeply as the scent of pine fills the air.", ch, NULL, NULL, TO_CHAR );
    act( "A blast of fresh air fills the room with the scent of pine trees.", 
        ch, NULL, NULL, TO_ROOM );
    return;
    }
    else if( !str_cmp(type, "flash") )
    {
       act( "You shield your eyes as a bright flash of light fills the room.", 
         ch, NULL, NULL, TO_CHAR );
       act( "A bright flash of light fills the room for a moment and then fades away.", 
         ch, NULL, NULL, TO_ROOM );
	return;
    }

    else if( !str_cmp(type, "dragon") )
    {
      act( "You make the image of a fire-breathing dragon appear in the room.", 
         ch, NULL, NULL, TO_CHAR );
      act( "A roaring dragon appears in the air and breathes fire, it flickers and disappears.",
         ch, NULL, NULL, TO_ROOM);
        return;
    }

    else if( !str_cmp(type, "jasmine") )
    {
        act( "You conjure forth the sweet smell of jasmine.", ch, NULL,NULL, TO_CHAR );
        act( "The sweet smell of jasmine wafts to your nose.", ch, NULL,NULL, TO_ROOM);
        return;
    }

    else if( !str_cmp(type, "icedance") )
    {
        act( "You twirl your finger and a delicate snowflake dances in the air for a moment.", 
          ch, NULL, NULL, TO_CHAR );
        act( "A delicate snowflake dances in the air.", ch, NULL, NULL, TO_ROOM );
        return;
    }
    else if (!str_cmp(type, "kiss") )
    {
        act("You wave your hand in the air, creating a gust of wind that blows kisses to the area.", ch, NULL, NULL, TO_CHAR );
        act( "$n waves a hand in the air, a small gust of wind picks up and you feel an affectionate kiss.", ch, NULL, NULL, TO_ROOM ); 
        return;
    }
    else if (!str_cmp(type, "rose") )
    {
        act("You rub your hands together forming a fiery ball that bursts into a flaming rose that sparkles and then goes out.",ch,NULL,NULL,TO_CHAR);
        act("$n rubs $s hands together forming a fiery ball that bursts into a flaming rose that sparkles and then goes out.", ch, NULL, NULL,TO_ROOM );
        return;
    }

    else if( !str_cmp(type, "sneeze") )
    {
        act( "You grin, knowing that anyone around you now has the urge to sneeze.", 
           ch, NULL, NULL, TO_CHAR );
        act( "Suddenly, your nose tickles furiously and you let loose a huge sneeze.", 
           ch, NULL, NULL, TO_ROOM );
        return;
    }
    else if( !str_cmp(type, "faces") )
    {
        act( "You make the shadows twist into menacing faces.", ch, NULL, NULL, TO_CHAR );
        act( "The shadows twist into menacing faces at $n's command.", ch, NULL, NULL, TO_ROOM );
        return;
    }
    else if( !str_cmp(type, "banquet") )
    {
        act( "You make it seem as if a banquet has appeared.", ch, NULL, NULL,TO_CHAR );
        act( "At $n's word, a sumptuous banquet is laid out by unseen hands.", 
          ch,NULL, NULL, TO_ROOM );
        return;
    }
    else if( !str_cmp(type, "ghost") )
    {
        act( "You create a thin reflection of your essence; it lasts only a moment.", 
           ch, NULL, NULL, TO_CHAR );
        act( "For an instant, you see a ghostly apparition that resembles $n.", 
          ch, NULL, NULL, TO_ROOM );
        return;
    }
    else if( !str_cmp(type, "light") )
    {
        light = create_object( get_obj_index( OBJ_VNUM_BLUELIGHT ), 0 );
        light->timer = 2;
        act("The room seems dingy until $n calls a ball of blue light to appear.", 
            ch, light, NULL, TO_ROOM );
        act("You call $p to appear.", ch, light, NULL, TO_CHAR );
        obj_to_room( light, ch->in_room );
        return;
    }
    else if( !str_cmp(type, "clean") )
    {
        act( "You cause unseen hands to tidy the room for you.", ch, NULL, NULL, TO_CHAR );
        act( "Objects seem to float of their own accord as unseen hands tidy the room.", 
          ch, NULL, NULL, TO_ROOM );
        return;
    }
    else if( !str_cmp(type, "dirty") )
    {
        act( "At your word, everything becomes dingy, covered in a layer of dirt.", 
          ch, NULL, NULL, TO_CHAR );
        act( "At $n's word, everything becomes dingy, covered in a layer of dirt.", 
          ch, NULL, NULL, TO_ROOM );
        return;
    }
    else if( !str_cmp(type, "dust") )
    {
        act( "You cause foreboding words to write themselves into the thick dust.", 
          ch, NULL, NULL, TO_CHAR );
        act( "Foreboding words write themselves into the thick dust of this place.", 
         ch, NULL, NULL, TO_ROOM );
        return;
    }
    else if( !str_cmp(type, "drunk") )
    {
        act( "You appear to flush bright red, as if inebriated to the core.", 
          ch, NULL, NULL, TO_CHAR );
        act( "$n flushes bright red and begins to sweat, as if inebriated to the core.", 
          ch, NULL, NULL, TO_ROOM );
        return;
    }
    else if( !str_cmp(type, "sober") )
    {
        act( "You look as if you've never partaken of the fruit of the vine.", 
          ch, NULL, NULL, TO_CHAR );
        act( "$n looks as if the fruit of the vine has never passed $s lips.", 
          ch, NULL, NULL, TO_ROOM );
        return;
    }

    else if( !str_cmp(type, "age") )
    {
        act( "You make it seem as if you have aged considerably for a moment.", 
          ch, NULL, NULL, TO_CHAR );
        act( "At $n's word, small lines and wrinkles appear upon $s face.", 
          ch, NULL, NULL, TO_ROOM );
        return;
    }        
    else if( !str_cmp(type, "crimson") )
    {
        act( "An arc of crimson light surrounds you as you speak the arcane words.", 
          ch, NULL, NULL, TO_CHAR );
        act( "$n is enveloped in a light crimson aura.",ch, NULL, NULL, TO_ROOM );
        return;
    }                                    
    else if( !str_cmp(type, "snow") )
    {
        act( "You raise your hands in an upward motion as the small flakes of snow gently begin to drift downwards.", ch, NULL, NULL, TO_CHAR );
        act( "Small flakes of snow begin to fall lightly about the room.", ch, NULL, NULL, TO_ROOM );
        return;
    }                                    
 
    else if( !str_cmp(type, "whispering") )
    {
        act( "At your command, the wind begin a soft chant into everyone's ears.", 
             ch, NULL, NULL, TO_CHAR );
        act( "The winds pick up as you hear a faint whispering cross your ears.", 
             ch, NULL, NULL, TO_ROOM );
        return;
    }     
     else if( !str_cmp(type, "shade") )
    {
        act( "A small shield forms over your head, shading you from the elements.", 
            ch, NULL, NULL, TO_CHAR );
        act( "$n utters a few arcane words as a shield forms above $s head, protecting them from the weather.", ch, NULL, NULL, TO_ROOM );
        return;
    }
    else if( !str_cmp(type, "redeye") )
    {
      act("As you speak the arcane words, your eyes glow an eerie red.",ch,NULL, NULL, TO_CHAR );
      act("As $n speaks the arcane words, $s eyes glow an eerie red.", ch, NULL, NULL, TO_ROOM );
        return;
    }

    else if( !str_cmp(type, "bats") )
    {
      act("As you throw a black dust into the air, it turns into a swarm of bats.",ch,NULL, NULL, TO_CHAR );
      act("As $n throws a black dust into the air, it turns into a swarm of bats .", ch, NULL, NULL, TO_ROOM );
        return;
    }
    else if( !str_cmp(type, "disappear") )
    {
      act("You perform a small illusion and make yourself appear to have moved by magic.",ch,NULL, NULL, TO_CHAR );
      act("$n snaps $s fingers and disappears, $e reappears on the other side of the room.", ch, NULL, NULL, TO_ROOM );
        return;
    }
    else if( !str_cmp(type, "butterflies") )
    {
      act("You twirl and open your robe, causing a mass of butterflies to flutter from them.",ch,NULL, NULL, TO_CHAR );
      act("$n twirls and opens $s robe, releasing dozens of butterflies.",ch, NULL, NULL, TO_ROOM );
        return;
    }
    else if( !str_cmp(type, "waterform") )
    {
      act("You focus on pulling water around you to form the appearance of a water being.",ch,NULL, NULL, TO_CHAR);
      act("$n turns momentarily into a watery shape.",ch, NULL, NULL, TO_ROOM );
        return;
    }
    else if( !str_cmp(type, "fairy") )
    {
        act( "You summon the fairy-folk in your sing-song voice.", 
          ch, NULL, NULL, TO_CHAR );
        act( "The room is filled with little sparks of quickly moving light, taking a closer look you notice they are fairies!", 
          ch, NULL, NULL, TO_ROOM );
        return;
    }
    else 
    {
        send_to_char( "You don't know a cantrip like that.\n\r", ch);
        return;
    }
}

void spell_disintegrate( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{

    CHAR_DATA *victim = (CHAR_DATA *) vo;
    OBJ_DATA *obj;
    OBJ_DATA *obj_lose;
    OBJ_DATA *obj_next;    
    OBJ_DATA *t_obj, *next_obj;
    int dam;
    bool found;
    found = FALSE;

  if (ch->fighting)
    victim = ch->fighting;
  if(victim || (victim = get_char_room(ch, NULL, target_name)) != NULL)
  {
    if (victim == ch)
        return;

   dam = dice( level, 10 );
    if ( saves_spell( level, victim,DAM_NEGATIVE ) )
    dam /= 2;
      damage( ch, victim, dam, sn, DAM_NEGATIVE, 0 );
      send_to_char("You scream as you feel some of your flesh rot and decompose as it disintegrates.\n\r",victim);
      act("$N screams as some of $S flesh rots and decomposes as it is disintegrated.", ch, NULL, victim, TO_NOTVICT);
      act("$N screams as some of $S flesh rots and decomposes as it is disintegrated.", ch, NULL, victim, TO_CHAR);
	damage( ch, victim, dam, sn, DAM_NEGATIVE ,TRUE);
	
	if ( number_percent( ) < 2 * level && !saves_spell( level-10, victim, DAM_NEGATIVE ) )
    	{
        for ( obj_lose = victim->carrying; obj_lose != NULL; obj_lose = obj_next )
        {
            int iWear, i;
            obj_next = obj_lose->next_content;
            if ( (number_range( 1, 20 ) != 1) || (found))
                continue;
            switch ( obj_lose->item_type )
            {
            case ITEM_ARMOR:
                if ( (obj_lose->value[0] > 0) && (obj_lose->wear_loc != -1))
                {
                    act( "$p disappears, swallowed by a black void of decay!",
                        victim, obj_lose, NULL, TO_CHAR );
                    if ( ( iWear = obj_lose->wear_loc ) != WEAR_NONE )
                        for (i = 0; i < 4; i ++)
                            victim->armor[i] -= apply_ac( obj_lose, iWear, i );
                    for (i = 0; i < 4; i ++)
                        obj_lose->value[i] -= 1;
                    obj_lose->cost      = 0;
                    if ( iWear != WEAR_NONE )
                        for (i = 0; i < 4; i++)
                            victim->armor[i] += apply_ac( obj_lose, iWear, i );
		    found = TRUE;
                }
                break;
            case ITEM_CONTAINER:
		if (obj_lose->wear_loc == -1)
		{
                act( "$p fumes and dissolves, destroying some of the contents.",
                    victim, obj_lose, NULL, TO_CHAR );
                /* save some of  the contents */
                for (t_obj = obj_lose->contains; t_obj != NULL; t_obj = next_obj)
                {
                    next_obj = t_obj->next_content;
                    obj_from_obj(t_obj);
                    if (number_bits(2) == 0 || victim->in_room == NULL)
                        extract_obj(t_obj);
                    else 
                        obj_to_room(t_obj,victim->in_room);
                }
                extract_obj( obj_lose );
                }
		found = TRUE;
                break; 
            }
         }
        }
      return;
  }
    if (target_name[0] == '\0')
	return;

        if ((obj = get_obj_here(ch, NULL, target_name)) == NULL)
	{
	  send_to_char("You cannot find that here.\n\r", ch);
          return;
        }

     if (obj->wear_loc != -1)
     {
	send_to_char("That must be carried to disintegrate.\n\r",ch);
	return;
     }

       if ( !CAN_WEAR(obj, ITEM_TAKE) )
        {
        send_to_char("You cannot disintegrate that!\n\r",ch);
        return;
        }

      if ((obj->item_type == ITEM_CORPSE_PC) || 
	  (obj->item_type == ITEM_CORPSE_NPC) )
      {
        act("Thousands of black worms emerge from the rotting flesh of $p and rapidly decompose it.",ch, obj, NULL, TO_ROOM);
        act("Thousands of black worms emerge from the rotting flesh of $p and rapidly decompose it.",ch, obj, NULL, TO_CHAR);
        for (t_obj = obj->contains; t_obj ; t_obj = next_obj)
	{
           next_obj = t_obj->next_content;   
           obj_from_obj(t_obj);
 
           if (obj->in_obj) /* in another object */
             obj_to_obj(t_obj,obj->in_obj);
 
           if (obj->carried_by)  /* carried */
             obj_to_char(t_obj,obj->carried_by);
 
           if (!obj->in_room)  /* destroy it */
             extract_obj(t_obj);
           else /* to a room */
             obj_to_room(t_obj,obj->in_room);
        }
        extract_obj( obj );
        return;
      }
      else
      {
        act("$p disintegrates before your very eyes!", ch, obj, NULL, TO_ROOM);
        act("You disintegrate $p.", ch, obj, NULL, TO_CHAR);
        extract_obj( obj );
        return;
      }
    
 send_to_char("You cannot find that here.\n\r", ch);
 return;
}

void spell_spiritual_hammer( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam, wait;

    if (victim == ch)
        return;

    dam = dice(level,4);
    if (saves_spell(level,victim,DAM_HOLY))
	dam /= 2;
    else
        {
        act("$n's spell knocks you to the ground.\n\r",ch,NULL,victim,TO_VICT);
        act ("$n is knocked to the ground by a divine force.\n\r",victim, NULL,NULL, TO_ROOM);
	  damage( ch, victim, dam, sn, DAM_HOLY ,TRUE);
        wait = number_range(1,3);
        if (victim->level < LEVEL_HERO+1) 
        WAIT_STATE(victim, wait + PULSE_VIOLENCE);
        victim -> position = POS_RESTING;
	  return;
        }
    act("$n hits you with a divine force.\n\r",ch,NULL,victim,TO_VICT);
    act("$n is hit with a divine force.\n\r",victim,NULL,NULL,TO_ROOM);
    damage( ch, victim, dam, sn, DAM_HOLY,TRUE );
    return;
}

void spell_darkness( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  CHAR_DATA *vch;
  OBJ_DATA  *light;
  
  for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
    for (light = vch->carrying; light != NULL; light = light->next_content)
      {
	if (light->item_type == ITEM_LIGHT && light->value[2] != 0
	&& !is_same_group(ch, vch) && !IS_IMMORTAL(vch)
        && !IS_NPC(vch) )
         {  
	  if (light->wear_loc <= 0 && !saves_spell(level, vch,DAM_NEGATIVE))
          {
	    act("$p flickers and goes out!",ch,light,NULL,TO_CHAR);
	    act("$p flickers and goes out!",ch,light,NULL,TO_ROOM);
	    light->value[2] = 0; 
	    ch->in_room->light--;
	  }
	  else
          {
	    act("$p momentarily dims.",ch,light,NULL,TO_CHAR);
	    act("$p momentarily dims.",ch,light,NULL,TO_ROOM);
	  }
	}
      }

  for (light = ch->in_room->contents;light != NULL; light=light->next_content)
    if ((light->item_type == ITEM_LIGHT) && (light->wear_loc <= 0)) 
    {  
      act("$p flickers and goes out!",ch,light,NULL,TO_CHAR);
      act("$p flickers and goes out!",ch,light,NULL,TO_ROOM);
      light->value[2] = 0; 
      ch->in_room->light--;
    }
}

void spell_unholy_armor( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    if ( is_affected( victim, sn ) )
    {
        if (victim == ch)
          send_to_char("You are already clothed in unholy armor.\n\r",ch);
        else
          act("$N is already clothed in unholy armor.",ch,NULL,victim,TO_CHAR);
        return;
    }
    af.where	= TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 24;
    af.modifier  = -65;
    af.location  = APPLY_AC;
    af.bitvector = 0;
    affect_to_char( victim, &af );

    if ( !IS_AFFECTED2( victim, AFF_REPULSION ) )
    {
    af.where		= TO_AFFECTS2;
    af.type      		= sn;
    af.modifier  		= 0;
    af.location  		= 0;
    af.bitvector 		= AFF_REPULSION;
    affect_to_char( victim, &af );
    }

    send_to_char( "You wrap yourself in unholy armor.\n\r", victim );
    if ( ch != victim )
        act("$N is wrapped in your unholy armor.",ch,NULL,victim,TO_CHAR);
    return;
}

void spell_wraithform( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED2( victim, AFF_WRAITHFORM ) )
    {
        if (victim == ch)
          send_to_char("You are already empowered by the essence of wraiths.\n\r",ch);
        else
          act("$N is already affected by the essence of wraiths.",ch,NULL,victim,TO_CHAR);
        return;
    }
    af.where	= TO_AFFECTS2;
    af.type      	= sn;
    af.level     	= level;
    af.duration  	= number_fuzzy( level/3);
    af.location  	= APPLY_NONE;
    af.modifier  	= 0;
    af.bitvector 	= AFF_WRAITHFORM;
    affect_to_char( victim, &af );

    if (!is_affected( victim, skill_lookup("unholy armor")))
    {
    af.type      	= skill_lookup("unholy armor");
    af.modifier  	= (-15+(-1*(level/3)));
    af.location  	= APPLY_AC;
    af.bitvector 	= 0;
    affect_to_char( victim, &af);
    }

    send_to_char( "You flicker and fade as you become transformed into a shimmering wraith.\n\r", victim );
    if ( ch != victim )
        act("$N flickers and fades as $E becomes transformed into a shimmering wraith.",ch,NULL,victim,TO_CHAR);
}

void spell_feeblemind( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (victim == ch)
	{
	send_to_char("You cannot focus this spell on yourself.\n\r",ch);
	return;
	}

    if (IS_IMMORTAL(victim))
	{
	send_to_char("You spell only creates a few sparks and a cool light show.\n\r",ch);
	return;
	}

    if (is_affected( victim, sn ) )
	{
	act("$N is already enfeebled.\n\r",ch,NULL,victim,TO_CHAR);
        return;
	}

    if (saves_spell( level, victim, DAM_MENTAL ) )
	{
	act("$N resists your attempts to enfeeble $M.\n\r",ch,NULL,victim,TO_CHAR);
	act("$n attempts to enfeeble $N, but $E resists.\n\r",ch,NULL,victim,TO_ROOM);
        return;
	}

    af.where		= TO_AFFECTS;
    af.type      		= sn;
    af.level     		= level;
    af.duration  		= level/3;
    af.location  		= APPLY_MANA;
    af.modifier  		= -200;
    af.bitvector 		= 0;
    affect_to_char( victim, &af );

    af.location  		= APPLY_INT;
    af.modifier  		= -7;
    af.bitvector 		= 0;
    affect_to_char( victim, &af );
    send_to_char( "You feel your mental focus slip away.\n\r", victim );
    act("$n appears to be less focused.",victim,NULL,NULL,TO_ROOM);
    return;
}

void spell_iceblast( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
         CHAR_DATA *vch;
         CHAR_DATA *vch_next;
         int dam;
         char buf[MAX_STRING_LENGTH];

	 sprintf( buf,"Help! %s just hit me with a blast of ice!", ch->name);

         for ( vch = ch->in_room->people; vch != NULL; vch = vch_next)
         {
           vch_next = vch->next_in_room;
           if (!IS_NPC(ch) && vch != ch && !is_same_group(vch,ch) 
           && !IS_IMMORTAL(vch))
            {
             if (ch->fighting == NULL)
	       do_function( vch, &do_yell, buf );

             dam = dice( level, 12 );
             if (saves_spell( level, vch, DAM_COLD))
                 dam /= 2;
             damage( ch, vch, dam, sn, DAM_COLD, TRUE);
            }
        }
  return;
}

void spell_binding ( int sn, int level, CHAR_DATA *ch,void *vo, int target)
{
  OBJ_DATA *obj = (OBJ_DATA *) vo;
  int i;  
  int fail, result;


  /* Make sure the character has the object */
  if (obj->wear_loc != -1)
  {
    send_to_char("An item to be bound must be held, wielded or worn.\n\r",ch);
    return;
  }
  
 
i = ch->level - obj->level;
fail = 80;
fail = fail - i;

 if (IS_OBJ_STAT(obj,ITEM_BLESS))
   fail -= 15;
 if (IS_OBJ_STAT(obj,ITEM_GLOW)) 
     fail -= 5; 
 result = number_percent();

/* Item is destroyed 1 out of three failures */

 if (result < (fail/2))
{
 act("Tendrils of light tighten around $p and shatter it!",ch,obj,NULL,TO_CHAR);
 act("Tendrils of light tighten around $p and shatter it!",ch,obj,NULL,TO_ROOM); 
 extract_obj(obj);
 return;
 } 

 /* Normal failure does nothing */
 
if (result < fail )
 {
 act("Tendrils of light tighten around $p and then dissipate.",ch,obj,NULL,TO_CHAR);
 act("Tendrils of light tighten around $p and then dissipate.",ch,obj,NULL,TO_ROOM);
 return;
 }

 /* Success adds ITEM_NOREMOVE flag to the item */ 

{
 act("Tendrils of light tighten around $p, causing it to glow briefly.",ch,obj,NULL,TO_CHAR);
 act("Tendrils of light tighten around $p, causing it to glow briefly.",ch,obj,NULL,TO_ROOM);
 SET_BIT(obj->extra_flags, ITEM_NOREMOVE);
 SET_BIT(obj->extra_flags, ITEM_NODROP); 
return;
}
}

void spell_badberry( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    OBJ_DATA *berry;
    berry = create_object( get_obj_index( OBJ_VNUM_BERRY ), 0 );
    berry->value[0] = level/4;
    berry->value[3] = 1;
    obj_to_room( berry, ch->in_room );
    act( "$n has created $p.", ch, berry, NULL, TO_ROOM );
    act( "You have created $p.", ch, berry, NULL, TO_CHAR );
    return;
}

void spell_goodberry( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    OBJ_DATA *berry;
    berry = create_object( get_obj_index( OBJ_VNUM_BERRY ), 0 );
    berry->value[0] = 5 + level;
    berry->value[1] = 5;
    obj_to_room( berry, ch->in_room );
    act( "$n has created $p.", ch, berry, NULL, TO_ROOM );
    act( "You have created $p.", ch, berry, NULL, TO_CHAR );
    return;
}

void spell_breath_of_nature( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int heal, sect;
  OBJ_DATA *obj;

   sect = ch->in_room->sector_type;

  if (ch->in_room != NULL)
  {
    for (obj = ch->in_room->contents; obj != NULL; obj = obj->next_content)
    {
     /* Added for plant growth. This will still allow the caster to cast
      * their spell, but the affect will be less.
      */
      if ((obj->in_room != NULL)
      &&  (obj->pIndexData->vnum == 1820))
      {
        heal = dice(3, 8) + level - 14;
        victim->hit = UMIN( victim->hit + heal, victim->max_hit );
        check_subdue(victim);
        update_pos( victim );
        send_to_char( "Nature caresses your soul with her rejuvenating breath.\n\r", victim);
        if ( ch != victim )
        send_to_char( "Ok.\n\r", ch );
        return;
      }
    }
  }


   if ( (sect == SECT_INSIDE)
   || (sect == SECT_CITY)
   || (IS_SET(ch->in_room->room_flags, ROOM_INDOORS) ) )
    {
    send_to_char( "The soft whisper of Nature's breath cannot be heard here.\n\r", ch);
    return;
    }
   else 
    {
    heal = dice(3, 8) + level - 7;
    victim->hit = UMIN( victim->hit + heal, victim->max_hit );
    check_subdue(victim);
    update_pos( victim );
    send_to_char( "Nature caresses your soul with her rejuvenating breath.\n\r", victim);
    if ( ch != victim )
       send_to_char( "Ok.\n\r", ch );
    }
}

void spell_druidstaff( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
  OBJ_DATA *staff;

  if ( ch->carry_number >= can_carry_n( ch) )
  {
    send_to_char("You cannot carry that many items.\n\r",ch);
    return;
  }

  if ( ch->carry_weight >= can_carry_w(ch) )
  {
  send_to_char("You cannot carry that much weight.\n\r",ch);
  return;
  }

  staff = create_object( get_obj_index(OBJ_VNUM_DRUIDSTAFF),level);
  send_to_char("You create a druid staff!\n\r",ch);
  act("$n creates a druid staff!",ch,NULL,NULL,TO_ROOM);
 
  staff->value[0] = 10;	/*makes this a staff*/
  staff->value[1] = (level * 2)/10;
  staff->value[2] = 5;

  if  (ch->level > 39) 
   {
    if (IS_GOOD(ch))
     staff->extra_flags = ITEM_ANTI_EVIL|ITEM_ANTI_NEUTRAL|ITEM_INVENTORY|ITEM_ROT_DEATH;
    else if (IS_EVIL(ch))
     staff->extra_flags = ITEM_ANTI_GOOD|ITEM_ANTI_NEUTRAL|ITEM_INVENTORY|ITEM_ROT_DEATH;
   }
  else 
   {
   if (IS_GOOD(ch))
    staff->extra_flags = ITEM_ANTI_EVIL|ITEM_ANTI_NEUTRAL|ITEM_ROT_DEATH;
   else if (IS_EVIL(ch))
    staff->extra_flags = ITEM_ANTI_NEUTRAL|ITEM_ANTI_GOOD|ITEM_ROT_DEATH;
   }

  staff->timer = level/2;
  
  obj_to_char(staff,ch);
}

void spell_earthmeld( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim;
    bool gate_pet;

    if ( ch->quit_timer < 0
    && !IS_IMMORTAL(ch)
    && (number_percent() < 50 )) 
    {
	send_to_char("Your heart is beating so fast, you lose your concentration on the incantation.\n\r",ch);
	return;
    }
   
    if ( ( victim = get_char_world( ch, target_name ) ) == NULL
    ||   victim == ch
    ||   victim->in_room == NULL
    ||   !can_see_room(ch,victim->in_room)
    ||   victim->in_room == ch->in_room
    ||   IS_SET(victim->in_room->room_flags, ROOM_SAFE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
    ||   IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
    ||   IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
    ||   victim->level >= level + 5
    ||   (!IS_NPC(victim) && victim->level >= LEVEL_HERO)  /* NOT trust */ 
    ||   (IS_NPC(victim) && saves_spell( level, victim,DAM_OTHER) ) )
    {
        send_to_char( "Your attempt to earthmeld failed.\n\r", ch );
        return;
    }

    if (ch->riding)
    {
      send_to_char("You cannot earthmeld while mounted.\n\r",ch);
      return;
    }

    if (ch->pet != NULL && ch->in_room == ch->pet->in_room && !ch->pet->tethered)
        gate_pet = TRUE;
    else
        gate_pet = FALSE;

    switch(ch->in_room->sector_type)
    {
	case SECT_FIELD:
	  act("The grass below $n opens up and swallows $m up.",ch,NULL,NULL,TO_ROOM);
        send_to_char("The grass below you opens wide and swallows you.\n\r",ch);
	  break;
	case SECT_FOREST:
	  act("$n melds into a tree and vanishes.",ch,NULL,NULL,TO_ROOM);
        send_to_char("You step into a tree and vanish.\n\r",ch);
	  break;
	case SECT_HILLS:
	case SECT_MOUNTAIN:
	  act("$n suddenly sinks into the rocky soil and is gone.",ch,NULL,NULL,TO_ROOM);
        send_to_char("Suddenly the rocky soil splits open and you sink down.\n\r",ch);
	  break;
	case SECT_AIR:
	  act("$n fades into the air and is gone.",ch,NULL,NULL,TO_ROOM);
        send_to_char("Your body becomes light, melding you with the wind and air.\n\r",ch);
	  break;
	case SECT_SWAMP:
	  act("The swampy ground under $n bubbles... $e sinks and then is gone.",
           ch,NULL,NULL,TO_ROOM);
        send_to_char("The ground under your feet swallows you up.\n\r",ch);
	  break;
	case SECT_DESERT:
	  act("A sandy whirlpool is created under $n and suddenly $e disappears.",
           ch,NULL,NULL,TO_ROOM);
        send_to_char("You flush yourself into the sand and vanish.\n\r",ch);
	  break;
	case SECT_GRAVEYARD:
	  act("$n is sucked into the ground and is no longer before you.",ch,NULL,NULL,TO_ROOM);
        send_to_char("The ground sucks you deep into the bowels of the earth.\n\r",ch);
	  break;
	default:
        act("$n lays down on the ground and melds with it.",ch,NULL,NULL,TO_ROOM);
        send_to_char("You meld with the ground.\n\r",ch);
	  break;
    }
    char_from_room(ch);
    char_to_room(ch,victim->in_room);
    switch ( ch->in_room->sector_type )
    {
      case SECT_FIELD:
        act("$n rises up from the grass.",ch,NULL,NULL,TO_ROOM);
	  break;
	case SECT_FOREST:
        act("$n steps out of a tree.",ch,NULL,NULL,TO_ROOM);
	  break;
	case SECT_HILLS:
	case SECT_MOUNTAIN:
        act("$n rises up from the rocky soil.",ch,NULL,NULL,TO_ROOM);
	  break;
	case SECT_SWAMP:
        act("$n rises up from the swampy ground.",ch,NULL,NULL,TO_ROOM);
	  break;
	case SECT_DESERT:
        act("$n materializes from the sand.",ch,NULL,NULL,TO_ROOM);
	  break;
	case SECT_GRAVEYARD:
        act("$n appears from the ground.",ch,NULL,NULL,TO_ROOM);
	  break;
	default:
        act("The ground opens up and $n appears.",ch,NULL,NULL,TO_ROOM);
	  break;
    }
    do_look(ch,"auto");
    if (gate_pet)
    {
	act("$n melds with the ground.",ch->pet,NULL,NULL,TO_ROOM);
	send_to_char("You meld with the ground.\n\r",ch->pet);
	char_from_room(ch->pet);
	char_to_room(ch->pet,victim->in_room);
	act("The ground opens up and $n appears.",ch->pet,NULL,NULL,TO_ROOM);
      do_look(ch->pet,"auto");
    }
      if(IS_AFFECTED2 (ch, AFF_CAMOUFLAGE ) )
      {
        send_to_char("You leave the safety of your cover.", ch);
        act( "$n leaves $s cover.", ch, NULL, NULL, TO_ROOM );
        REMOVE_BIT (ch->affected2_by, AFF_CAMOUFLAGE);
	}
	if(IS_AFFECTED (ch, AFF_HIDE ) )
      {
        send_to_char("You step out of the shadows.", ch);
        act( "$n steps out of the shadows.", ch, NULL, NULL, TO_ROOM);
        REMOVE_BIT (ch->affected_by, AFF_HIDE);
	}
}

void spell_windspirit( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

  act( "$n softly whispers into a passing breeze.",  ch, NULL, NULL, TO_ROOM);
  act( "You softly whisper into a passing breeze.", ch, NULL, NULL, TO_CHAR); 

    if ( is_affected( victim, sn ) )
    {
        if (victim == ch)
          send_to_char("You already hold the spirit of the wind within you! \n\r", ch);
        else
          act("$N already houses the spirit of the wind.", ch, NULL, victim, TO_CHAR);
        return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.location  = APPLY_DEX;
    af.modifier  = 1 + (level >= 40) + (level >= 51);
    af.bitvector = 0;
    affect_to_char( victim, &af );
    send_to_char( "You feel the spirit of the wind enter your body!\n\r", victim );
    act("The spirit of the wind finds its way to $N and enters $s body.", 
       ch, NULL, victim,TO_NOTVICT);
    if ( ch != victim )
        send_to_char( "Ok.\n\r", ch );
    return;
}

void spell_stonespirit( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    act( "$n gently fingers a stone in $s palm and concentrates.",  ch, NULL, NULL, TO_ROOM);
    act( "You gently finger a stone in your palm and concentrate.", ch, NULL, NULL, TO_CHAR); 
    if ( is_affected( victim, sn ) )
    {
        if (victim == ch)
          send_to_char("You already have a stonespirit within you! \n\r", ch);
        else
          act("$N already houses a stonespirit.", ch, NULL, victim, TO_CHAR);
        return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.location  = APPLY_CON;
    af.modifier  = 1 + (level >= 40) + (level >= 51);
    af.bitvector = 0;
    affect_to_char( victim, &af );
    send_to_char( "You feel a stonespirit bind with your flesh! \n\r", victim );
    act("A stonespirit rises up and joins with $N's body.", ch, NULL, victim, TO_NOTVICT);
    if ( ch != victim )
        send_to_char( "Ok.\n\r", ch );
    return;
}

void spell_summon_insects(int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  AFFECT_DATA af;
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  bool Hit = FALSE;
  int sect;

  sect = ch->in_room->sector_type;

  if ( ( ( sect == SECT_FIELD )
  || ( sect == SECT_FOREST )
  || ( sect == SECT_HILLS )
  || ( sect == SECT_AIR ) 
  || ( sect == SECT_MOUNTAIN )
  || ( sect == SECT_GRAVEYARD )) &&
     (!IS_SET(ch->in_room->room_flags, ROOM_INDOORS) ) )
  {
    send_to_char("You summon a great swarm of biting insects!\n\r", ch);
    act( "$n summons a great swarm of biting insects!",ch, NULL, NULL, TO_ROOM);
    for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
    {
        vch_next = vch->next_in_room;
        if (is_safe_spell(ch,vch,DAM_OTHER) )
            continue;

        Hit = TRUE;
        if ( is_same_group( vch, ch ) )
        {
            act("The insects don't seem interested in $N.", ch,NULL,vch,TO_CHAR);
            act("The insects don't seem interested in $N.", ch, NULL, vch, TO_NOTVICT);
            send_to_char("The insects don't seem intested in you.\n\r",vch);
            continue;
        }
        if ( saves_spell( level, vch, DAM_OTHER ) )
        {
            send_to_char( "You feel a needle-like pinch.\n\r", vch );
            act("A stray insect bites $N.", ch, NULL, vch, TO_CHAR);
            act("A stray insect bites $N.", ch, NULL, vch, TO_NOTVICT);
            damage( ch, vch, level/2, sn, DAM_PIERCE, 0 );
            continue;
        }
      if ( !is_affected( vch, sn ) )
        {
        af.where     = TO_AFFECTS;
        af.type      = sn;
        af.level     = level;
        af.duration  = level;
        af.modifier  = -level/10;
        af.location  = APPLY_DEX;
        af.bitvector = 0;
        affect_to_char( vch, &af );

	  af.modifier  = -level/15;
        af.location  = APPLY_STR;
        affect_to_char( vch, &af );
	  af.location  = APPLY_HITROLL;
	  affect_to_char( vch, &af );
        }
        send_to_char( "The swarm descends upon you, buzzing and biting!\n\r", vch );
        act("$N swats frantically as insects swarm around $M, buzzing and biting.",
          ch, NULL, vch, TO_CHAR);
        act("$N swats frantically as insects swarm around $M, buzzing and biting.",
         ch, NULL, vch, TO_NOTVICT);
        damage( ch, vch,dice(22 + 20, 20), sn, DAM_PIERCE, TRUE );
      }
      if ( !Hit)
      {
        send_to_char("The swarm dissipates.\n\r",ch);
        act("The swarm dissipates.",ch,NULL,NULL,TO_ROOM);
      }
    }
    else
    {
        act ("There aren't enough insects here.", ch, NULL, NULL, TO_CHAR );
        return;
    }
}

void spell_natures_embrace( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
  int sect;
  OBJ_DATA *obj;

  sect = ch->in_room->sector_type;

  if (ch->in_room != NULL)
  {
    for (obj = ch->in_room->contents; obj != NULL; obj = obj->next_content)
    {
     /* Added for plant growth. This will still allow the caster to cast
      * their spell, but the duration of the affect of the spell will be shorter
      */
      if ((obj->in_room != NULL)
      &&  (obj->pIndexData->vnum == 1820))
      {
        if ( IS_AFFECTED2(victim, AFF_NATURE) )
        {
         if (victim == ch)
          send_to_char("Nature already embraces you with her protection.\n\r",ch);
         else
          act("You cannot convince the spirit of nature to protect someone else.",
          ch,NULL,victim,TO_CHAR);
         return;
        }

        af.where     = TO_AFFECTS2;
        af.type      = sn;
        af.level     = level;
        af.duration  = level / 7;
        af.location  = APPLY_NONE;
        af.modifier  = 0;
        af.bitvector = AFF_NATURE;
        affect_to_char( victim, &af );
        send_to_char("You kneel to the ground and etch a small rune of protection.\n\r",victim);
        do_say (ch, "Gods of nature guard my travels with your embrace!");  
        send_to_char( "You feel a warmth pass over your body as nature holds you in its protective embrace.\n\r", victim );
        act("$N etches a runic symbol in the ground.",ch,NULL,victim,TO_ROOM);
        act("As $N finishes the incantation a green aura envelops $m.",
        ch,NULL,victim,TO_ROOM);
        act("The green aura flashes brightly for a moment, then fades.",
        ch,NULL,victim,TO_ROOM);
        return;
      }
    }
  }

  if ((sect == SECT_INSIDE)
  || (sect == SECT_CITY)
  || (IS_SET(ch->in_room->room_flags, ROOM_INDOORS)))
  {
    send_to_char( "The powers of nature cannot protect you here!\n\r", ch);
    return;
  }

  if ( IS_AFFECTED2(victim, AFF_NATURE) )
  {
    if (victim == ch)
      send_to_char("Nature already embraces you with her protection.\n\r",ch);
    else
      act("You cannot convince the spirit of nature to protect someone else.",ch,NULL,victim,TO_CHAR);
    return;
  }

  af.where     = TO_AFFECTS2;
  af.type      = sn;
  af.level     = level;
  af.duration  = level / 5;
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.bitvector = AFF_NATURE;
  affect_to_char( victim, &af );
  send_to_char("You kneel to the ground and etch a small rune of protection.\n\r",victim);
  do_say (ch, "Gods of nature guard my travels with your embrace!");  
  send_to_char( "You feel a warmth pass over your body as nature holds you in its protective embrace.\n\r", victim );
  act("$N etches a runic symbol in the ground.",ch,NULL,victim,TO_ROOM);
  act("As $N finishes the incantation a green aura envelops $m.",ch,NULL,victim,TO_ROOM);
  act("The green aura flashes brightly for a moment, then fades.",ch,NULL,victim,TO_ROOM);
  return;
}

void spell_natures_warmth(  int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
  int sect;
  OBJ_DATA *obj;

  sect = ch->in_room->sector_type;

  if (ch->in_room != NULL)
  {
    for (obj = ch->in_room->contents; obj != NULL; obj = obj->next_content)
    {
     /* Added for plant growth. This will still allow the caster to cast
      * their spell, but the duration of the affect of the spell will be shorter
      */
      if ((obj->in_room != NULL)
      &&  (obj->pIndexData->vnum == 1820))
      {
        if ( IS_AFFECTED2(victim, AFF_WARMTH) )
        {
          send_to_char("You already feel the warmth of nature!\n\r",ch);
          return;
        }
   
        af.where     = TO_AFFECTS2;
        af.type      = sn;
        af.level     = level;
    	  af.duration  = level / 7;
    	  af.location  = APPLY_NONE;
    	  af.modifier  = 0;
    	  af.bitvector = AFF_WARMTH;
    	  affect_to_char( victim, &af );
    	  send_to_char("You begin an arcane chant to the forces of nature!\n\r",victim); 
    	  act("$N begins an arcane chant to the forces of nature.",ch,NULL,victim,TO_ROOM);   
    	  act("The plants surrounding $n seem to glow for a brief moment.",
        ch,NULL,victim,TO_ROOM);
    	  send_to_char("You feel the warmth of nature flow within you!\n\r",victim);
    	  return;
      }
    }
  }

  if ((sect == SECT_INSIDE)
  || (sect == SECT_CITY)
  || (IS_SET(ch->in_room->room_flags, ROOM_INDOORS) ) )
  {
   send_to_char( "You cannot feel the warmth of nature in these surroundings!\n\r",ch);
   return;
  }

  if ( IS_AFFECTED2(victim, AFF_WARMTH) )
    {
          send_to_char("You already feel the warmth of nature!\n\r",ch);
          return;
    }
   
    af.where     = TO_AFFECTS2;
    af.type      = sn;
    af.level     = level;
    af.duration  = level / 5;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_WARMTH;
    affect_to_char( victim, &af );
    send_to_char("You begin an arcane chant to the forces of nature!\n\r",victim); 
    act("$N begins an arcane chant to the forces of nature.",ch,NULL,victim,TO_ROOM);   
    act("The plants surrounding $n seem to glow for a brief moment.",ch,NULL,victim,TO_ROOM);
    send_to_char("You feel the warmth of nature flow within you!\n\r",victim);
    return;
}

void spell_enlightenment( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
      if (victim == ch)
        send_to_char("You are already as enlightened as you can get!\n\r",ch);
      else
        send_to_char("\n\r",ch);
        act("$N can not be enlightened any further.", ch, NULL, victim, TO_CHAR);
      return;
    }
   
    if ( victim == ch )
    {
	  af.where     = TO_AFFECTS2;
        af.type      = sn;
        af.level     = level;
        af.duration  = level;
        af.location  = APPLY_WIS;
        af.modifier  = 1 + (level >= 40) + (level >= 51);
        af.bitvector = AFF_ENLIGHTENMENT;
        affect_to_char( victim, &af );
        send_to_char("You feel your god's wisdom flow into you.\n\r",ch);
        return;
    }

   if (!IS_IMMORTAL(ch))
	{
	act("You share the profound wisdom of your deity with $N.\n\r",ch, NULL, victim, TO_CHAR);
	act("$n shares the profound wisdom of $s diety with you.\n\r", ch,NULL, victim, TO_VICT);
	act("$n shares the profound wisdom of $s diety with $N.\n\r", ch,NULL, victim, TO_NOTVICT);
	}
   else
	{
	act("You share your divine wisdom with $N.\n\r", ch, NULL, victim,TO_CHAR);
	act("$n shares $s divine wisdom with you.\n\r", ch, NULL,victim,TO_VICT);
	act("$n shares divine wisdom with $N.\n\r", ch, NULL, victim,TO_NOTVICT);
	}	

    af.where     = TO_AFFECTS2;
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.location  = APPLY_WIS;
    af.modifier  = 1 + (level >= 40) + (level >= 51);
    af.bitvector = 0;
    affect_to_char( victim, &af );
    send_to_char( "You are able to grasp profundities as never before!\n\r", victim );
    act("$N has been enlightened by $n's preaching.", ch, NULL, victim, TO_NOTVICT);
    return;
}

void spell_bad_luck( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (victim == ch || is_affected( victim, sn ) )
    {
        if (victim == ch)
          send_to_char("Your day is already unlucky enough!\n\r",ch);
        else
          act("Things couldn't possibly go worse for $N!",ch,NULL,victim,TO_CHAR);
        return;
    }
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.location  = APPLY_HITROLL;
    af.modifier  = -5;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    send_to_char( "Your body groans as you feel your luck go sour.\n\r", victim );
    act("$n slumps slightly, and lets out a pained sigh.",victim,NULL,NULL,TO_ROOM);
    return;
}

void spell_word_of_death( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    char buf[MAX_STRING_LENGTH];

    if ((victim == ch) || IS_IMMORTAL(victim))
        return;
    
    if (victim->fighting != ch)
        victim->fighting = ch;

    if (ch->fighting || (ch->quit_timer < 0) )
    {
	send_to_char("You can't concentrate enough.\n\r",ch);
	return;
    }

    if( is_safe(ch, victim) )
    {
	send_to_char("Your spell had no effect on them.\n\r",ch);
	return;
    }

    if (victim->fighting == NULL && !IS_NPC(victim) )
    {
      if ((!can_see(victim,ch)) && (!is_same_group(victim,ch))) 
        do_yell(victim, "Help! Someone cast a spell on me!");
      else 
      {
        sprintf(buf, "Aaaaagh! %s cast a spell on me!", ch->name);
        do_yell( victim, buf);
      }
    }

    /* set quit timer so they cannot spam the command. (see above check) */
    if ( ch->lastfought != NULL 
    &&  !IS_NPC(ch->lastfought) )
    {
      ch->quit_timer = 0;
      ch->quit_timer = -6; 
    }

    if ( number_percent() > 10)
	{
      act("$N grows deathly pale for a moment, but maintains $S composure.", ch, NULL, victim, TO_CHAR    );
	act("$N grows deathly pale for a moment, but maintains $S composure.", 
          ch, NULL, victim, TO_NOTVICT);
      act("The shadow of death clouds your mind for a moment, but you regain your composure.", ch, NULL, victim, TO_VICT);
	ch->hit -= 15;
	return;
	}
    else
	{
      act("$N slumps to the ground! DEAD!", ch, NULL, victim, TO_CHAR );
	act("$N slumps to the ground lifelessly as a dark shadow passes over $M.", 
         ch, NULL, victim, TO_NOTVICT);
	act("A dark shadow passes over you, your body slumps lifelessly to the ground! DEAD!",ch, NULL, victim, TO_VICT);
	raw_kill(ch,victim);
	}
}

void spell_blade_barrier( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const sh_int dam_each[] =
    {
          0,
          0,   0,   0,   0,   0,          0,   0,   0,   0,   0,
          0,   0,   0,   0,  30,         33,  35,  37,  40,  43,
         45,  47,  50,  53,  55,         57,  60,  63,  65,  67,
         70,  73,  75,  77,  80,         82,  84,  86,  88,  90,
         92,  94,  96,  98, 100,        102, 104, 106, 108, 110,
        112, 114, 116, 118, 120,        122, 124, 126, 128, 130,
	  132, 134, 136, 138, 140,	    142, 144, 146, 148, 150,
	  152, 154, 156, 158, 160,	    162, 164, 166, 168, 170,
	  172, 174, 176, 178, 180,	    182, 184, 186, 188, 190,
	  192, 194, 196, 198, 200,	    202, 204, 206, 208, 210
    };

    int dam;
    if (victim == ch)
        return;

    level       = UMIN(level, sizeof(dam_each)/sizeof(dam_each[0]) - 1);
    level       = UMAX(0, level);
    dam         = number_range( dam_each[level] / 2, dam_each[level] * 2 );

    send_to_char("You conjure daggers that hurl themselves at your enemy.\n\r", ch);
    act("$n conjures daggers that hurl themselves at you.", ch, NULL,victim, TO_VICT);
    act("$n conjures daggers that hurl themselves at $N.", ch, NULL,victim, TO_NOTVICT);

    if ( saves_spell( level, victim, DAM_PIERCE) )
        dam /= 2;
    damage( ch, victim, dam+15, sn, DAM_SLASH, TRUE );
    return;
}

void spell_flyingswords( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const sh_int dam_each[] =
    {
          0,
          0,   0,   0,   0,   0,          0,   0,   0,   0,   0,
          0,   0,   0,   0,  30,         35,  40,  45,  50,  55,
         60,  65,  70,  75,  80,         82,  84,  86,  88,  90,
         92,  94,  96,  98, 100,        102, 104, 106, 108, 110,
        112, 114, 116, 118, 120,        122, 124, 126, 128, 130,
	  132, 134, 136, 138, 140,	    142, 144, 146, 148, 150,
	  152, 154, 156, 158, 160,	    162, 164, 166, 168, 170,
	  172, 174, 176, 178, 180,	    182, 184, 186, 188, 190,
	  192, 194, 196, 198, 200,	    202, 204, 206, 208, 210,
	  212, 214, 216, 218, 220,	    222, 224, 226, 228, 230
    };
    int dam;

    if (victim == ch)
        return;

    level       = UMIN(level, sizeof(dam_each)/sizeof(dam_each[0]) - 1);
    level       = UMAX(0, level);
    dam         = number_range( dam_each[level] / 1.5, dam_each[level] * 2.5 );

    send_to_char("You conjure swords that slice furiously at your enemy.\n\r", ch); 
    act("$n conjures swords that slice furiously at you.", ch, NULL, victim, TO_VICT);
    act("$n conjures swords that slice furiously at $N.", ch, NULL, victim, TO_NOTVICT); 

    if ( saves_spell( level, victim, DAM_PIERCE) )
        dam /= 2;
    damage( ch, victim, dam+20, sn, DAM_SLASH, TRUE );
    return;
}

void spell_aura_cleanse( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  OBJ_DATA *obj = (OBJ_DATA *) vo;
  AFFECT_DATA *paf;
  int result, fail;

  fail = 35;
  fail -= 3 * level/2;

  if (IS_OBJ_STAT(obj,ITEM_BLESS))
     fail -= 15;
  if (IS_OBJ_STAT(obj,ITEM_GLOW))
     fail -= 5;
  fail = URANGE(5,fail,95);
  result = number_percent();

  if (result - (obj->level - level) < (fail / 2 )) 
   {
    act("$p glows lightly then fades into nothingness!",ch,obj,NULL,TO_CHAR);
    act("$p glows lightly then fades into nothingness!",ch,obj,NULL,TO_ROOM);
    extract_obj(obj);
    return;
   }
  if (result - (obj->level - level)  < (fail / 1.5))
   {
    AFFECT_DATA *paf_next;
    act("$p vibrates faintly, then ceases to move.",ch,obj,NULL,TO_CHAR);
    act("$p vibrates faintly, then ceases to move.",ch,obj,NULL,TO_ROOM);
    for (paf = obj->affected;  paf != NULL; paf = paf_next)
    {
          paf_next = paf->next;
          paf_next = affect_free;
          affect_free = paf;
    }
    obj->affected = NULL;
    obj->extra_flags = 0;
    return;
    }
 if ( result - (obj->level - level) <= fail )
  {
   send_to_char("Nothing seemed to happen.\n\r",ch);
   return;
   }
 if (IS_OBJ_STAT(obj,ITEM_EVIL))
     REMOVE_BIT(obj->extra_flags,ITEM_EVIL);
 
 if (IS_OBJ_STAT(obj,ITEM_ANTI_EVIL))
     REMOVE_BIT(obj->extra_flags,ITEM_ANTI_EVIL);

 if (IS_OBJ_STAT(obj,ITEM_DARK))
     REMOVE_BIT(obj->extra_flags,ITEM_DARK);

 if (IS_OBJ_STAT(obj,ITEM_ANTI_NEUTRAL))
     REMOVE_BIT(obj->extra_flags,ITEM_ANTI_NEUTRAL);

 
 if (IS_OBJ_STAT(obj, ITEM_ANTI_GOOD));
     REMOVE_BIT(obj->extra_flags, ITEM_ANTI_GOOD);
       
  act("You speak a solemn prayer to purge $p!",ch,obj,NULL,TO_CHAR);
  act("$n speaks a solemn prayer to purge $p!",ch,obj,NULL,TO_ROOM);
  act("$p glows with an reverant light.",ch,obj,NULL,TO_CHAR);
  act("$p glows with an reverant light.",ch,obj,NULL,TO_ROOM);
  return;
}

void spell_bark_skin( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    if ( is_affected( ch, sn ) )
    {
        if (victim == ch)
          send_to_char("Your skin is already covered in bark.\n\r",ch); 
        else
          act("$N's skin is already as hard as tree bark.",ch,NULL,victim,TO_CHAR);
        return;
    }
    af.type      = sn;
    af.level     = level;
    af.duration  = level/2;
    af.location  = APPLY_AC;
    af.modifier  = (-20+(-1*(level/2)));
    af.bitvector = 0;
    affect_to_char( victim, &af );
    act( "$n's skin becomes covered in bark.", victim, NULL, NULL, TO_ROOM );
    send_to_char( "Your skin becomes covered in bark.\n\r", victim );
    return;
}

void spell_coldfire( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const sh_int dam_each[] = 
    {
         0,
         4,  4,  5,  5,  6,      7,  7,  7,  7,  7,
         8,  8,  8,  8,  8,      9,  9,  9,  9,  9,
        10, 10, 10, 10, 10,     11, 11, 11, 11, 11,
        12, 12, 12, 12, 12,     13, 13, 13, 13, 13,
        14, 14, 14, 14, 14,     15, 15, 15, 15, 15,
        16, 16, 16, 16, 16,     17, 17, 17, 17, 17,
        18, 18, 18, 18, 18,     19, 19, 19, 19, 19,
        20, 20, 20, 20, 20,     21, 21, 21, 21, 21,
        22, 22, 22, 22, 22,     23, 23, 23, 23, 23,
        24, 24, 24, 24, 24,     25, 25, 25, 25, 25
    };
    int dam;
    if (victim == ch)
        return;

    send_to_char("A cool aura of energy begins to surround you.\n\r", ch); 
    send_to_char("You extend your arm, ordering the energy to strike out at your enemy.\n\r", ch); 
    act( "$n begins an arcane chant, a cool blue energy begins to surround $m.", ch, NULL, NULL, TO_ROOM);
    act("$n extends his arm and orders the cool blue energy to strike you.", ch, NULL, victim, TO_VICT);
    act("$n extends his arm, ordering the energy to strike out at $N.", ch, NULL, victim, TO_NOTVICT); 

    level       = UMIN(level, sizeof(dam_each)/sizeof(dam_each[0]) - 1);
    level       = UMAX(0, level);
    dam         = number_range( dam_each[level] / 2, dam_each[level] * 2 );
    if ( saves_spell( level, victim, DAM_COLD ) )
        dam /= 2;
    damage( ch, victim, dam, sn, DAM_COLD,TRUE );
    return;
}

void spell_battlestaff( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
  OBJ_DATA *staff;

  if ( ch->carry_number >= can_carry_n( ch) )
  {
    send_to_char("You cannot carry that many items.\n\r",ch);
    return;
  }

  if ( ch->carry_weight >= can_carry_w(ch) )
  {
  send_to_char("You cannot carry that much weight.\n\r",ch);
  return;
  }

  staff = create_object( get_obj_index(OBJ_VNUM_BATTLESTAFF),level);
  send_to_char("You create a battle staff!\n\r",ch);
  act("$n creates a battle staff!",ch,NULL,NULL,TO_ROOM);

  staff->value[1] = (level * 2)/10;
  staff->value[2] = 4;
  if  (ch->level > 39)
   {
    if (IS_GOOD(ch))
     staff->extra_flags = ITEM_ANTI_EVIL|ITEM_ANTI_NEUTRAL|ITEM_INVENTORY|ITEM_ROT_DEATH;
    else if (IS_EVIL(ch))
     staff->extra_flags = ITEM_ANTI_GOOD|ITEM_ANTI_NEUTRAL|ITEM_INVENTORY|ITEM_ROT_DEATH;
   }
  else
   {
   if (IS_GOOD(ch))
    staff->extra_flags = ITEM_ANTI_EVIL|ITEM_ANTI_NEUTRAL|ITEM_ROT_DEATH;
   else if (IS_EVIL(ch))
    staff->extra_flags = ITEM_ANTI_NEUTRAL|ITEM_ANTI_GOOD|ITEM_ROT_DEATH;
   }

  staff->timer = level/2;

  obj_to_char(staff,ch);
}

void spell_knock( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{

   char pick_obj[MAX_INPUT_LENGTH];  
   OBJ_DATA *obj;
   int door;
  
   obj = NULL;

   target_name = one_argument(target_name, pick_obj);

   if (pick_obj[0] == '\0')
   {
     send_to_char("What would you like to try to unlock?\n\r",ch);
     return;
   }
  
   if ( (obj = get_obj_here( ch, NULL, pick_obj)) != NULL)
   {
       if (obj->item_type != ITEM_CONTAINER)
       {
          send_to_char("That's not a container.\n\r",ch);
          return;
       }
       if (!IS_SET(obj->value[1], CONT_CLOSED))
       {
          send_to_char( "It's not closed.\n\r",ch);
          return;
       }
       if ( obj->value[2] < 0)
       {
          send_to_char( "It can't be unlocked.\n\r",ch);
          return;
       }
       if ( !IS_SET(obj->value[1], CONT_LOCKED))
       {
          send_to_char( "It's already unlocked.\n\r",ch);
          return;
       }
       if ( IS_SET(obj->value[1], CONT_PICKPROOF))
       {
          send_to_char( "spell failed.\n\r",ch);
          return;
       }
       REMOVE_BIT(obj->value[1], CONT_LOCKED);
       send_to_char( "*Click*\n\r",ch);
       return;
     }

     if ( ( door = find_door( ch, pick_obj) ) >= 0)
     {
       ROOM_INDEX_DATA *to_room;
       EXIT_DATA *pexit;
       EXIT_DATA *pexit_rev;
       pexit = ch->in_room->exit[door];
       if ( !IS_SET(pexit->exit_info, EX_CLOSED))
       {
          send_to_char( "It's not closed.\n\r",ch);
          return;
       }
       if ( pexit->key < 0 && !IS_IMMORTAL(ch))
       {
          send_to_char("That lock can't be opened.\n\r",ch);
          return;
       }
       if ( !IS_SET(pexit->exit_info, EX_LOCKED))
       {
          send_to_char("It's already unlocked.\n\r",ch);
          return;
       }
       if ( IS_SET(pexit->exit_info, EX_PICKPROOF) && !IS_IMMORTAL(ch))
       {
          send_to_char("Spell failed.\n\r",ch);
          return;
       }
       REMOVE_BIT(pexit->exit_info, EX_LOCKED);
       send_to_char("*Click*\n\r",ch);
       
       /* pick lock on other side of door as well */
       if ( (  to_room = pexit->u1.to_room ) != NULL
       &&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
       &&   pexit_rev->u1.to_room == ch->in_room)
       {
            REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED);
       }
     }
     return;
}

void spell_flame_rune(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
  OBJ_DATA *obj = (OBJ_DATA *) vo;
  int fail, result;
  
  if (obj->item_type != ITEM_WEAPON)
    {
      send_to_char("That isn't a weapon.\n\r",ch);
      return;
    }
  
  if (obj->wear_loc != -1)
    {
      send_to_char("The item must be carried.\n\r",ch);
      return;
    }
  if(obj->value[4] & WEAPON_FLAMING) /*Already flaming*/
    {
      send_to_char("That weapon is already flaming!\n\r",ch);
      return;
    }
  if(IS_WEAPON_STAT(obj,WEAPON_FROST))
    {
      send_to_char("That weapon is too cold to accept the magic.\n\r", ch);
	return ;
    }

  fail = 50; /*Base chance of failure*/
  if(IS_WEAPON_STAT(obj,WEAPON_VAMPIRIC))   /*Any Extra flags will make*/
    fail +=35;
  if(IS_WEAPON_STAT(obj,WEAPON_SHARP))      /*The enchant harder*/
    fail +=35;
  if(IS_WEAPON_STAT(obj,WEAPON_VORPAL))
    fail +=35;
  if(IS_WEAPON_STAT(obj,WEAPON_SHOCKING))
    fail +=35;
  if(IS_WEAPON_STAT(obj,WEAPON_POISON))
    fail +=35;
  if(obj->enchanted)
    fail +=25;
  if (IS_OBJ_STAT(obj,ITEM_BLESS))
    fail-=10;
  if (IS_OBJ_STAT(obj,ITEM_GLOW))
    fail-=3;
  fail = URANGE(5,fail,95);
  fail -= level;
  result = number_percent();
  

  if(result < (fail/2)) /*Destroyed*/
    {
      act("In a flash of extreme heat, mysterious writing appears on and eats through $p!",ch,obj,NULL,TO_CHAR);
      act("In a flash of extreme heat, mysterious writing appears on and eats through $p!",ch,obj,NULL,TO_ROOM);
      extract_obj(obj);
      return;
    }
  if(result <=fail)
    {
      send_to_char("Nothing seemed to happen.\n\r",ch);
      return;
    }
  if (result > fail)  /* success! */
    {
      act("A rune of bright yellow appears on $p, radiating intense heat!",ch,obj,NULL,TO_CHAR);
      act("A rune of bright yellow appears on $p, radiating intense heat!",ch,obj,NULL,TO_ROOM);
      SET_BIT(obj->value[4], WEAPON_FLAMING);
	if(obj->level < 51)
      	obj->level = UMIN(obj->level + 3, 51);
    }
  return;
}

void spell_frost_rune(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
  OBJ_DATA *obj = (OBJ_DATA *) vo;
  int fail, result;
  
  if (obj->item_type != ITEM_WEAPON)
    {
      send_to_char("That isn't a weapon.\n\r",ch);
      return;
    }
  
  if (obj->wear_loc != -1)
    {
      send_to_char("The item must be carried.\n\r",ch);
      return;
    }
  if(obj->value[4] & WEAPON_FROST) /*Already frost*/
    {
      send_to_char("That weapon is already cold!\n\r",ch);
      return;
    }

 if(IS_WEAPON_STAT(obj,WEAPON_FLAMING))
    {
	send_to_char("That weapon is too hot to accept the magic.\n\r", ch);
	return ;
    }

  fail = 50; /*Base chance of failure*/
  if(IS_WEAPON_STAT(obj,WEAPON_VAMPIRIC))     /*Any Extra flags will make*/
    fail +=35;
  if(IS_WEAPON_STAT(obj,WEAPON_SHARP))      /*The enchant harder*/
    fail +=35;
  if(IS_WEAPON_STAT(obj,WEAPON_VORPAL)) 
    fail +=35;
  if(IS_WEAPON_STAT(obj,WEAPON_SHOCKING)) 
    fail +=35;
  if(IS_WEAPON_STAT(obj,WEAPON_POISON))
    fail +=35;
  if(obj->enchanted)
    fail += 25;
  if (IS_OBJ_STAT(obj,ITEM_BLESS))
    fail-=10;
  if (IS_OBJ_STAT(obj,ITEM_GLOW))
    fail-=3;
  fail = URANGE(5,fail,95);
  fail -= level;
  result = number_percent();
  
  if(result < (fail/2)) /*Destroyed*/
    {
      act("In a burst of freezing cold $p shatters into pieces!",ch,obj,NULL,TO_CHAR);
      act("In a burst of freezing cold $p shatters into pieces!",ch,obj,NULL,TO_ROOM);
      extract_obj(obj);
      return;
    }
  if(result <=fail)
    {
      send_to_char("Nothing seemed to happen.\n\r",ch);
      return;
    }
  if (result > fail)  /* success! */
    {
      act("A rune of deep blue turns $p deathly cold!",ch,obj,NULL,TO_CHAR);
      act("A rune of deep blue turns $p deathly cold!",ch,obj,NULL,TO_ROOM);
      SET_BIT(obj->value[4], WEAPON_FROST);
	if(obj->level < 51)
      	obj->level = UMIN(obj->level + 3, 51);
    }
  return;
}

void spell_shocking_rune(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
  OBJ_DATA *obj = (OBJ_DATA *) vo;
  int fail, result;
  
  if (obj->item_type != ITEM_WEAPON)
    {
      send_to_char("That isn't a weapon.\n\r",ch);
      return;
    }
  
  if (obj->wear_loc != -1)
    {
      send_to_char("The item must be carried.\n\r",ch);
      return;
    }
  if(IS_WEAPON_STAT(obj,WEAPON_SHOCKING))  /*Already flaming*/
    {
      send_to_char("That weapon is already electrified!\n\r",ch);
      return;
    }
  fail = 50; /*Base chance of failure*/
  if(IS_WEAPON_STAT(obj,WEAPON_VAMPIRIC))    /*Any Extra flags will make*/
    fail +=35;
  if(IS_WEAPON_STAT(obj,WEAPON_SHARP))      /*The enchant harder*/
    fail +=35;
  if(IS_WEAPON_STAT(obj,WEAPON_VORPAL)) 
    fail +=35;
  if(IS_WEAPON_STAT(obj,WEAPON_FLAMING)) 
    fail +=35;
  if(IS_WEAPON_STAT(obj,WEAPON_FROST))
    fail +=35;
  if(IS_WEAPON_STAT(obj,WEAPON_POISON))
    fail +=35;
  if(obj->enchanted)
    fail +=25;
  if (IS_OBJ_STAT(obj,ITEM_BLESS))
    fail-=10;
  if (IS_OBJ_STAT(obj,ITEM_GLOW))
    fail-=3;
  fail = URANGE(5,fail,95);
  fail -= level;
  result = number_percent();
  
  if(result < (fail/2)) /*Destroyed*/
    {
      act("In a bolt of white light, $p is ruined!",ch,obj,NULL,TO_CHAR);
      act("In a bolt of white light, $p is ruined!",ch,obj,NULL,TO_ROOM);
      extract_obj(obj);
      return;
    }
  if(result <=fail)
    {
      send_to_char("Nothing seemed to happen.\n\r",ch);
      return;
    }
  if (result > fail)  /* success! */
	{
	  act("A white rune appears on $p, sending crackles of electricity down it!",ch,obj,NULL,TO_CHAR);
	  act("A white rune appears on $p, sending crackles of electricity down it!",ch,obj,NULL,TO_ROOM);
	  SET_BIT(obj->value[4], WEAPON_SHOCKING);
	  if(obj->level < 51)
      	obj->level = UMIN(obj->level + 3, 51);
	}
  return;
}

void spell_drain_rune(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
  OBJ_DATA *obj = (OBJ_DATA *) vo;
  int fail, result;

  if(IS_GOOD(ch))
    {
      send_to_char("You are far too nice to use such magic.\n\r",ch);
      return ;
    }

  if(!IS_EVIL(ch))
    {
      send_to_char("You are not quite wicked enough to do that.\n\r",ch);
      return ;
    }

  if (obj->item_type != ITEM_WEAPON)
    {
      send_to_char("That isn't a weapon.\n\r",ch);
      return;
    }

  if (obj->wear_loc != -1)
    {
      send_to_char("The item must be carried.\n\r",ch);
      return;
    }
  if(IS_OBJ_STAT(obj,ITEM_BLESS))
    {
	send_to_char("That weapon is too holy to be touched by your magic.\n\r", ch);
	return ;
    }
  if(IS_WEAPON_STAT(obj, WEAPON_VAMPIRIC))
    {
      send_to_char("That weapon is already vampiric!\n\r",ch);
      return;
    }
  fail = 50;
  if(IS_WEAPON_STAT(obj,WEAPON_VORPAL))    /*Any Extra flags will make*/
    fail +=35;
  if(IS_WEAPON_STAT(obj,WEAPON_SHARP))      /*The enchant harder*/
    fail +=35;
  if(IS_WEAPON_STAT(obj,WEAPON_POISON)) 
    fail +=35;
  if (IS_WEAPON_STAT(obj, WEAPON_FLAMING)) 
    fail +=35;
  if (IS_WEAPON_STAT(obj, WEAPON_FROST))
    fail +=35;
  if(IS_WEAPON_STAT(obj,WEAPON_SHOCKING))
    fail +=35;
  if(obj->enchanted)
    fail +=25;
  if (IS_OBJ_STAT(obj,ITEM_BLESS))
    fail-=10;
  if (IS_OBJ_STAT(obj,ITEM_GLOW))
    fail-=3;
  fail = URANGE(5,fail,95);
  fail -= level;
  result = number_percent();
  if(result < (fail/3)) /*Destroyed*/
    {
      act("The power is sucked from $p, destroying it!",ch,obj,NULL,TO_CHAR);
      act("The power is sucked from $p, destroying it!",ch,obj,NULL,TO_ROOM);
      extract_obj(obj);
      return;
    }
  if(result <=fail)
    {
      send_to_char("Nothing seemed to happen.\n\r",ch);
      return;
    }
  if (result > fail)  /* success! */
    {
      act("$p feels like it'll suck the life from you as a rune of black appears on it!",ch,obj,NULL,TO_CHAR);
      act("$p feels like it'll suck the life from you as a rune of black appears on it!",ch,obj,NULL,TO_ROOM);
      SET_BIT(obj->value[4], WEAPON_VAMPIRIC);
	if(obj->level < 51)
      	obj->level = UMIN(obj->level + 3, 51);
      if(!IS_OBJ_STAT(obj,ITEM_EVIL))
	  SET_BIT(obj->extra_flags, ITEM_EVIL);
	if(!IS_OBJ_STAT(obj,ITEM_ANTI_GOOD))
	  SET_BIT(obj->extra_flags, ITEM_ANTI_GOOD);
	if(!IS_OBJ_STAT(obj,ITEM_ANTI_NEUTRAL))
	  SET_BIT(obj->extra_flags, ITEM_ANTI_NEUTRAL);
    }
  return;
}

void spell_chant_of_battle( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    act( "$n begins to chant a song of great battle.",  ch, NULL, NULL, TO_ROOM);
    act( "You begin to chant a song of great battle.", ch, NULL, NULL, TO_CHAR); 
    if ( is_affected( victim, sn ) )
    {
        if (victim == ch)
          send_to_char("The song doesn't appear to have any effects on you! \n\r", ch);
        return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level / 2;
    af.duration  = level / 4;
    af.location  = APPLY_STR;
    af.modifier  = 2;
    af.bitvector = 0;
    affect_to_char( victim, &af );

    af.location  = APPLY_DEX;
    af.modifier  = 2;
    af.bitvector = 0;
    affect_to_char( victim, &af );

    af.location  = APPLY_AC;
    af.modifier  = -10;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    send_to_char( "You feel a power from within the song, begin to strengthen your battle skills! \n\r", victim );
    act("A power hidden deep within the song's words, begins to surround $N.", ch, NULL, victim, TO_NOTVICT);
    return;
}

void spell_chords_of_dissonance( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;

  send_to_char( "The clashing of musical chords fills the room!\n\r", ch );
  act( "$n fills the room with musical chords clashing together.", ch,NULL,NULL,TO_ROOM );

  for ( vch = char_list; vch != NULL; vch = vch_next )
  {
    vch_next	= vch->next;

    if ( vch->in_room == NULL )
      continue;

    if ( vch->in_room == ch->in_room )
    {
      if (!IS_NPC(vch) && is_same_group( vch, ch ))
        continue;

      if ( vch != ch && !is_safe_spell(ch,vch,TRUE))
      {
        if (IS_AFFECTED(vch,AFF_FLYING))
          damage(ch,vch,0,sn,DAM_SOUND,TRUE);
	  else
          damage(ch,vch,level + dice(1, 15), sn, DAM_SOUND,TRUE);
      }
	continue;
    }
  }
  return;
}

void spell_grasping_roots( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (victim == ch)
        return;

    if ( is_affected( victim, sn ) )
    {
        act("$N is already being detained from wild roots.", ch, NULL, victim, TO_CHAR);
        return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level / 2;
    af.duration  = 3;
    af.location  = APPLY_DEX;
    af.modifier  = -3;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    act( "Hundreds of roots pierce through the ground and grasp at $N's feet!", ch, NULL, victim, TO_CHAR    );
    act( "Hundreds of roots come out of the ground and grasp at your feet, slowing you down!",  ch, NULL, victim, TO_VICT    );
    act( "$n's control over the earth's roots, entangle $N!",   ch, NULL, victim, TO_NOTVICT );
    return;
}

void spell_cloak_starlight( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    if ( is_affected( victim, sn ) )
    {
        if (victim == ch)
          send_to_char("You are already cloaked in shimmering starlight. \n\r",ch);
        else
          act("$N is already cloaked in shimmering starlight.", ch, NULL,victim, TO_CHAR);
        return;
    }
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level/2;
    af.modifier  = -30 -level/2;
    af.location  = APPLY_AC;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    send_to_char( "The faint light of the stars forms a cloak about you.\n\r", victim);
    act("$N is cloaked in the faint light of the stars.", ch, NULL, victim, TO_NOTVICT );
    return;
}

void spell_gaseous_form( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    if ( is_affected ( victim, sn ) )
	{
        send_to_char("You are already surrounded by gas.\n\r", victim );
        return;
	}

    act( "A cloud of gas suddenly appears and surrounds $n.\n\r", victim, NULL, NULL, TO_ROOM );
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level / 2;
    af.location  = APPLY_AC;
    af.modifier  = -25;
    af.bitvector = 0;
    affect_to_char( victim, &af );

    if ( level <= 40 )
    {
    af.location  = APPLY_HITROLL;
    af.modifier  = 3;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    }
else
    if ( level > 40 && level <= 75 )
    {
    af.location  = APPLY_HITROLL;
    af.modifier  = 5;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    }
else
    if ( level > 75 )
    {
    af.location  = APPLY_HITROLL;
    af.modifier  = 7;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    }
    send_to_char( "You conjure a cloud of gas.\n\r", victim );
    return;
}

void spell_dark_taint( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (victim == ch)
        return;

    if ( IS_AFFECTED2(victim, AFF_DARK_TAINT) )
	{
        act("$N is already a suitable host to your dark energies.",ch,NULL,victim,TO_CHAR);
        return;
	}

    if (victim->fighting == NULL && !IS_NPC(victim) )
	spell_yell(ch, victim, NULL);

    af.where     = TO_AFFECTS2;
    af.type      = sn;
    af.level     = level;
    af.duration  = number_fuzzy( level / 6 );
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_DARK_TAINT;
    affect_to_char( victim, &af );
    send_to_char( "You feel dark forces stir within you.\n\r", victim );
    if ( ch != victim )
        act("$N is now a suitable host for dark energies.",ch,NULL,victim,TO_CHAR);
    return;
}

void spell_ward( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( ch, sn ) )
    {
          send_to_char("You are already warded.\n\r",ch);
           return;
    }
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level/4;
    af.modifier  = -1* (level + (level/2));
    af.location  = APPLY_AC;
    af.bitvector = 0;
    affect_to_char( ch, &af );
    send_to_char( "Energies swirl and weave around you, sealing you away from harm.\n\r", ch);
    act( "Energies swirl and weave around $n, sealing $m away from harm.\n\r", victim, NULL, NULL, TO_ROOM );
    return;
}

void spell_forget( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_NPC(victim))
    {
      act("You can only incite forgetfulness in sentient targets.", ch, NULL, victim, TO_CHAR);
      return;
    }

    if (victim == ch)
    {
      act("You prefer to keep your delicate memory intact.", ch, NULL, victim, TO_CHAR);
      return;
    }

    if ( IS_AFFECTED2( victim, AFF_FORGET ) )
    {
      act("$N is already forgetful.", ch, NULL, victim, TO_CHAR);
      return;
    }

    if ( (saves_spell(level, victim, DAM_NEGATIVE)))
    {
      act("You fail to wither the memories of $N.", ch, NULL, victim, TO_CHAR);
      return;
    }
    af.where	= TO_AFFECTS2;
    af.type      	= sn;
    af.level     	= level / 5;
    af.duration  	= level / 8;
    af.location  	= APPLY_NONE;
    af.modifier  	= 0;
    af.bitvector 	= AFF_FORGET;
    affect_to_char( victim, &af );
    send_to_char( "A cold void enters your mind as your memories slip away. \n\r", victim );
    act( "A blank stare suddenly spreads across $N's face.", ch, NULL, victim, TO_NOTVICT );
    act("A blank stare suddenly spreads across $N's face as you wither $S memories with your magic.", ch, NULL, victim, TO_CHAR); 
    return;
}

void spell_truefire( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  OBJ_DATA *flame;
  AFFECT_DATA *paf;
  int added;

  if ( ch->carry_number >= can_carry_n( ch ) )
  {
    send_to_char("You can't carry that many items.\n\r",ch);
    return;
  }

  if ( ch->carry_weight >= can_carry_w(ch) )
  {
  send_to_char("You can't carry that much weight.\n\r",ch);
  return;
  }

  flame = create_object( get_obj_index(OBJ_VNUM_FLAME),level);
  send_to_char("You bring forth a shimmering white flame!\n\r",ch);
  act("$n brings forth a shimmering white flame!",ch,NULL,NULL,TO_ROOM);

  added = level / 10;

  if (affect_free == NULL)
    paf = alloc_perm(sizeof(*paf));
  else
   {
   paf = affect_free;
   affect_free = affect_free->next;
   }
        paf->type       = sn;
        paf->level      = level;
        paf->duration   = -1;
        paf->location   = APPLY_DAMROLL;
        paf->modifier   = added;
        paf->bitvector  = 0;
        paf->next       = flame->affected;
        flame->affected = paf;

  if (affect_free == NULL)
    paf = alloc_perm(sizeof(*paf));
  else
    {
    paf = affect_free;
    affect_free = affect_free->next;
    }

        paf->type       = sn;
        paf->level      = level;
        paf->duration   = -1;
        paf->location   = APPLY_HITROLL;
        paf->modifier   = added;
        paf->bitvector  = 0;
        paf->next       = flame->affected;
        flame->affected = paf;

  flame->timer = level;

  obj_to_char(flame,ch);
}

void spell_forgedeath( int sn, int level, CHAR_DATA *ch, void *vo,int target ) 
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
  OBJ_DATA *obj;

  if ( is_affected( ch, sn ) )
 {
  if (victim == ch)
     send_to_char("You have already forged a death edge on your weapon.\n\r",ch);
  else 
     act("$N already has a deathblade.",ch,NULL,victim,TO_CHAR); return; 
 }
  obj = get_eq_char(ch, WEAR_WIELD); /* list what obj equals-Kracus*/
  if ( get_eq_char(ch,WEAR_WIELD) == NULL) /* make sure they are wielding */ 
 {
  send_to_char("You have to be wielding the weapon to forge it in death.\n\r",ch); 
  return;
 }
 if (IS_OBJ_STAT(obj,ITEM_DEATHBLADE)) /*already is a deathblade! */ 
 {                                     
  send_to_char("Your blade is already forged in death.\n\r",ch);
  return;
 }
  SET_BIT(obj->extra_flags, ITEM_DEATHBLADE);
  obj->timer = 50; /* keep this number the same as the af.duration number */

    af.where      	= TO_CHAR;
    af.type        	= sn;
    af.level        	= level;
    af.duration		= 50; /* keep number same as "obj->timer" number*/
    af.location   	= APPLY_DAMROLL; 
    af.modifier  		= 4;
    af.bitvector 		= 0;
    affect_to_char( ch, &af );
    act( "$n forges a weapon into a deathblade.", victim, NULL, NULL,TO_ROOM ); 
    send_to_char( "You forge a death edge on your weapon.\n\r", victim ); return;
}

void spell_closing_weave( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    OBJ_DATA* src;
    OBJ_DATA* dst;

    if ((IS_SET(ch->in_room->room_flags, ROOM_SAFE) // Kracus- noclose in safe rooms
      ||(IS_SET(ch->in_room->room_flags, ROOM_PRIVATE))))
    {
        send_to_char("You fail to close the gateway.\n\r",ch);
        return;
    } 

    src = get_obj_here(ch, NULL, target_name); // Kracus-defines portal location

    if (src == NULL)
    {
       send_to_char("Channel a closing weave on what?", ch);
       return;
    }

    if ( src->item_type != ITEM_PORTAL ) // Kracus- has to be a portal
    {
        send_to_char( "You cannot close that.\n\r", ch );
        return;
    }

    if ( src->timer <= 0 ) // Set to make sure permanent or 0 portals nonclose
    {
        send_to_char("You cannot close a permanent gateway.",ch);
        return;
    }

    // Find second portal
    for (dst = get_room_index(src->value[3])->contents; dst != NULL; dst = dst->next_content)
    {
        if (is_name(target_name, dst->name) &&
	    dst->item_type == ITEM_PORTAL &&
	    dst->value[3] == ch->in_room->vnum) break;
    }
    // Set to make sure permanent or 0 portals nonclose
    if (dst != NULL && dst->timer <= 0 )
    {
        send_to_char("You cannot close a permanent gateway.",ch);
        return;
    }

    act("$p shimmers in a white light, then closes.", ch, src, NULL, TO_ROOM);
    act("$p shimmers in a white light, then closes.", ch, src, NULL, TO_CHAR);
    extract_obj(src); // Close the source portal
    if (dst != NULL) // Check for one-sided portal
    {
        extract_obj(dst); // Close the destination portal
    }

    return;
}

void spell_cause_discord( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    CHAR_DATA *fch ;
    
    if(ch == victim)
    { 
    send_to_char("If you want to fight your own group ... use the kill command.\n\r",ch);
    return;
    }

    act("$N tries to spread discord in your group!",victim,NULL,ch,TO_CHAR);
    act("You try to spread discord in $N's group!",ch,NULL,victim,TO_CHAR);
    
    log_string("Cause discord: parcours des char de la piece:");
    for(fch = victim->in_room->people ; fch != NULL ; fch = fch->next_in_room)
    {
        log_string(fch->name);

    	if( fch->master == victim 
    	 && ( fch->position > POS_RESTING )
    	 && can_see( fch, victim ) 
    	 && !saves_spell(level, fch, DAM_NEGATIVE) 
    	 && !( !IS_NPC(fch) && ( fch->clan_id != CLAN_NONE ) )
	 && !fch->riding )
    	 {
    	  if( (IS_NPC(fch) && IS_AFFECTED(fch,AFF_CHARM))
    	  || (!IS_NPC(fch) && !fch->riding) )
    	  {
    	    log_string("...attack his master!");
    	    stop_fighting(fch,FALSE);
    	    stop_follower(fch);
    	    send_to_char("You scream and attack your group leader!!\n\r",fch);
	    act("$n screams and suddenly attacks YOU!",fch,NULL,victim,TO_VICT);
	    act("$n screams and suddenly attacks $N!",fch,NULL,victim,TO_NOTVICT);
	    multi_hit(fch,victim,TYPE_UNDEFINED);    	    
   	  }
   	  else if( IS_NPC(fch) ) /* Pet or Mercenary */
   	  {
   	    log_string("...stop following his master!");
   	    stop_fighting(fch,FALSE);
   	    stop_follower(fch);
   	  }
   	  else
   	  {
	    log_string("BUG: cause_discord : should never log his :P");
	    return;
   	  }  	        	
    	 }
    	 else
    	   log_string("..isn't concern.");
    }
    return;
}

void spell_shifting_shadows( int sn, int level, CHAR_DATA *ch, void *vo, int target ) 
{ 
   CHAR_DATA *victim = (CHAR_DATA *) vo; 
   AFFECT_DATA af; 

   if ( is_affected( victim, sn ) ) 
   { 
    if (victim == ch) 
      send_to_char("You are already protected.\n\r",ch); 
    else 
      act("$N is already protected.",ch,NULL,victim,TO_CHAR); 
    return; 
   } 

   af.where     = TO_IMMUNE; 
   af.type      = sn; 
   af.level     = level; 
   af.duration  = 12; 
   af.location  = APPLY_NONE; 
   af.modifier  = 0; 
   af.bitvector = IMM_NEGATIVE; 
   affect_to_char( victim, &af ); 
   act( "$n is surrounded by shifting shadows.", victim, NULL, NULL, TO_ROOM ); 
   send_to_char( "Shadows begin to take form out of nowhere and surround you.\n\r", victim ); 
   return; 
}

void spell_alter_reality( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_NPC(victim))
    {
      act("$N is already in $S own little world.", ch, NULL, victim,TO_CHAR); 
      return;
    }

    if (victim == ch)
    {
      act("You prefer to stay in this reality.", ch, NULL, victim, TO_CHAR);
      return;
    }
    
    if ( IS_AFFECTED2( victim, AFF_PARADISE ) )
    {
          act("$N is already lost in $S illusions of nirvana.", ch, NULL,victim, TO_CHAR);
          return;
    }

    if (saves_spell(level,victim,DAM_OTHER))
    {
      act("You failed to trap $N in $S illusions of nirvana.", ch, NULL, victim, TO_CHAR);
      return;
    }

    if( is_safe_spell(ch, victim, FALSE) )
    {
	send_to_char("Your illusion of nirvana wasn't convincing.\n\r",ch);
	return;
    }

    af.where	= TO_AFFECTS2;
    af.type		= sn;
    af.level	= level;
    af.duration  	= level / 4;
    af.modifier  	= 0;
    af.location 	= APPLY_NONE;
    af.bitvector 	= AFF_PARADISE;
    affect_to_char( victim, &af );
    send_to_char( "You lose yourself in a dream of nirvana.\n\r", victim );
    act( "$n gets a faraway look in $s eyes.", victim, NULL, NULL, TO_ROOM );
    return;
}

void spell_shackles( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    af.where	= TO_AFFECTS;
    af.type      	= sn;
    af.level     	= level;
    af.duration  	= 24*level;
    af.location  	= APPLY_HITROLL;
    af.modifier  	= -100;
    af.bitvector 	= AFF_SHACKLES;
    affect_to_char( victim, &af );
    send_to_char( "\n\r", ch);
    send_to_char( "Guards approach you and cuff iron shackles around your wrists.\n\r", victim );
    send_to_char( "\n\r", victim); 
    if ( ch != victim )
    act("Guards approach $N and cuff iron shackles around $S wrists.\n\r"
	  "",ch,NULL,victim,TO_NOTVICT);
    return;
}

/*
 *    Applies a temporary effect to turn everything around.
 */
void spell_looking_glass( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;


    if (IS_IMMORTAL(victim))
    {
	send_to_char("Yeah right!",ch);
	return;
    }

    if (victim->fighting == NULL && !IS_NPC(victim) )
	 spell_yell(ch, victim, NULL);

    if ( IS_AFFECTED2(victim, AFF_SPEECH_IMPAIR ) )
    {
        act( "$S speech is already impaired.\n\r",ch,NULL,victim,TO_CHAR);
        return;
    }

    if( saves_spell( level, victim, DAM_NEGATIVE ) )
        return;
   
    if( is_safe_spell(ch, victim, FALSE) )
    {
	send_to_char("The looking glass has no affect.\n\r",ch);
	return;
    }

    /* add the speech impairment */
    af.where	= TO_AFFECTS2;
    af.type      	= sn;
    af.level     	= level;
    af.duration  	= number_range(3,7);
    af.location  	= APPLY_SPEECH_IMPAIR;
    af.modifier  	= MOD_SPEECH_REVERSE;
    af.bitvector 	= AFF_SPEECH_IMPAIR;
    affect_to_char( victim, &af );

    /* and add the screen reverse */
    af.location  	= APPLY_PERCEPTION;
    af.modifier  	= 0;
    af.bitvector 	= AFF_LOOKING_GLASS;
    affect_to_char( victim, &af );
    send_to_char( "Uoy ssap hguorht a gnikool ssalg dna gnihtyreve smees ylneddus egnarts.\n\r", victim );
    act("$N passed through the looking glass to the world beyond.",ch, NULL, victim, TO_CHAR);
    return;
}

void spell_seal_door( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
   char arg[MAX_INPUT_LENGTH];  
   int door;

   target_name = one_argument(target_name, arg);

   if (arg[0] == '\0')
   {
     send_to_char("What would you like to place a magical seal upon?\n\r",ch);
     return;
   }

   if ( ( door = find_door( ch, arg ) ) >= 0 )
   {
        /* seal door */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit = ch->in_room->exit[door];

	if ( IS_SET(pexit->exit_info, EX_MAGICSEAL) )
	{
           send_to_char( "It's already magically sealed shut.\n\r",  ch ); 
           return;
	}

	/* Do not seal doors to Ship Progs
       * or the ship's entrances, it screws up the spec */
	if( ch->in_room->vnum == 17784
	||  ch->in_room->vnum == 17788
	||  ch->in_room->vnum == 17900
	||  ch->in_room->vnum == 3162
	||  ch->in_room->vnum == 3163
	||  ch->in_room->vnum == 3164 )
	{
		send_to_char("Your magic fails to coalesce.\n\r",ch);
		return;
	}

	SET_BIT( pexit->exit_info, EX_MAGICSEAL );
	act( "$n places a magical seal upon the $d.", ch, NULL, pexit->keyword, TO_ROOM );
	send_to_char( "You succeeded at placing a magical seal upon the door.\n\r",  ch );

	if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
	    SET_BIT(pexit->exit_info, EX_CLOSED);

	/* seal the other side */
	if ( ( to_room   = pexit->u1.to_room            ) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
	&&   pexit_rev->u1.to_room == ch->in_room )
	{
	    SET_BIT( pexit_rev->exit_info, EX_MAGICSEAL );

	    if ( !IS_SET(pexit_rev->exit_info, EX_CLOSED) )
	    SET_BIT(pexit_rev->exit_info, EX_CLOSED);
	}
    }
}

void spell_unseal_door( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
   char arg[MAX_INPUT_LENGTH];  
   int door;

   target_name = one_argument(target_name, arg);

   if (arg[0] == '\0')
   {
     send_to_char("What would you like to place a magical seal upon?\n\r",ch);
     return;
   }

   if ( ( door = find_door( ch, arg ) ) >= 0 )
   {
        /* seal door */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit = ch->in_room->exit[door];

	if ( !IS_SET(pexit->exit_info, EX_MAGICSEAL) )
	{
           send_to_char( "It isn't even magically sealed shut though.\n\r",  ch ); 
           return;
	}

	REMOVE_BIT( pexit->exit_info, EX_MAGICSEAL );
	act( "$n removes a magical seal upon the $d.", ch, NULL, pexit->keyword, TO_ROOM );
	send_to_char( "You succeeded at removing the magical seal.\n\r",  ch );

	/* unseal the other side */
	if ( ( to_room   = pexit->u1.to_room            ) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
	&&   pexit_rev->u1.to_room == ch->in_room )
	{
	    REMOVE_BIT( pexit_rev->exit_info, EX_MAGICSEAL );
	}
    }
}

void spell_sate(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA*)vo;
  int amount;

  if(!IS_NPC(victim))
  {
  amount = number_fuzzy(5+ch->level);
  gain_condition( victim, COND_FULL, amount);
  send_to_char("Your hunger subsides.\n\r",victim);
  if( victim != ch )
  act("You have sated $S hunger.\n\r",ch,NULL,victim,TO_CHAR);
  return;
  }

  act("Let $M find $S own nourishment.\n\r",ch,NULL,victim,TO_CHAR);
  return;
}

void spell_attunement( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
        if (victim == ch)
          send_to_char("You are already as attuned as you can get!\n\r",ch);
        else
          act("$N can't get any more attuned to the web of life.", ch, NULL, victim, TO_CHAR);
        return;
    }
   
    if ( victim == ch )
    {
	  af.where		= TO_AFFECTS;
        af.type      	= sn;
        af.level     	= level;
        af.duration  	= level;
        af.location  	= APPLY_WIS;
        af.modifier  	= 1 + (level >= 40) + (level >= 51);
        af.bitvector 	= 0;
        affect_to_char( victim, &af );
        af.location  	= APPLY_INT;
        af.modifier  	= 1 + (level >= 40) + (level >= 51);
        af.bitvector 	= 0;
        affect_to_char( victim, &af );
        send_to_char("You feel more closely attuned to the web of life.\n\r", ch);
        return;
    }

    af.where	= TO_AFFECTS;
    af.type      	= sn;
    af.level     	= level;
    af.duration  	= level;
    af.location  	= APPLY_WIS;
    af.modifier  	= 1 + (level >= 40) + (level >= 51);
    af.bitvector 	= 0;
    affect_to_char( victim, &af );

    af.where	= TO_AFFECTS;
    af.location  	= APPLY_INT;
    af.modifier  	= 1 + (level >= 40) + (level >= 51);
    af.bitvector 	= 0;
    affect_to_char( victim, &af );
    send_to_char( "You are magically attuned to the web of life!\n\r", victim );
    act("$N has become attuned to the web of life through $n's magic.", 
        ch, NULL, victim,TO_NOTVICT); 
    if ( ch != victim )
        send_to_char( "Ok.\n\r", ch );
    return;
}

void spell_invigorate( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int add;

  if (victim == ch)
  {
     send_to_char("You cannot cast that spell on yourself.\n\r",ch);  
     return;  
  }

  if (ch->level > 80)
    add = 40;
  else if (ch->level > 59)
    add = 35;
  else if (ch->level > 39)
    add = 30;
  else add = 25;

  victim->mana = victim->mana + add;
  if (victim->mana > victim->max_mana)
     victim->mana = victim->max_mana;

  send_to_char("A warm glow passes through you.\n\r",victim);  
  act("A warm glow passes through $N.", ch, NULL, victim, TO_CHAR);
  act("A warm glow passes through $N",  ch, NULL, victim,TO_NOTVICT);
  return;
}

void spell_pass_without_trace( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
        if (victim == ch)
          send_to_char("You are already moving gracefully through nature.\n\r",ch);
        else
          act("$N is already moving gracefully through nature.",ch,NULL,victim,TO_CHAR);
        return;
    }

    af.where	= TO_AFFECTS2;
    af.type      	= sn;
    af.level     	= level;
    af.duration  	= 24;
    af.modifier  	= 0;
    af.location  	= APPLY_NONE;
    af.bitvector 	= AFF_PASSTRACE;
    affect_to_char( victim, &af );
    send_to_char( "You begin to tread with a supernatural grace.\n\r", victim );
    if ( ch != victim )
       act("$N begins to tread with a supernatural grace.",ch,NULL,victim,TO_CHAR);
}

void spell_luck( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
        if (victim == ch)
          send_to_char("You are already as lucky as you can be!\n\r",ch);
        else
          act("$N is as lucky as can be!",ch,NULL,victim,TO_CHAR);
        return;
    }
    af.where	= TO_AFFECTS;
    af.type      	= sn;
    af.level     	= level;
    af.duration  	= level;
    af.location  	= APPLY_HITROLL;
    af.modifier  	= 5;
    af.bitvector 	= 0;
    affect_to_char( victim, &af );
    send_to_char( "You feel as if this is your lucky day!\n\r", victim );
    act("$n gets a lucky gleam in $s eyes.",victim,NULL,NULL,TO_ROOM);
    return;
}

void spell_grandeur( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
        if (victim == ch)
          send_to_char("You are already too full of yourself!\n\r",ch);
        else
          act("$N is already wallowing in pride.", ch, NULL, victim, TO_CHAR);
        return;
    }

    af.where	= TO_AFFECTS;
    af.type      	= sn;
    af.level     	= level;
    af.duration  	= 2;
    af.location  	= APPLY_INT;
    af.modifier  	= 1;
    af.bitvector 	= 0;
    affect_to_char( victim, &af );

    af.location  	= APPLY_STR;
    af.modifier  	= 2;
    af.bitvector 	= 0;
    affect_to_char( victim, &af );

    af.location  	= APPLY_CON;
    af.modifier  	= 1;
    af.bitvector 	= 0;
    affect_to_char( victim, &af );

    af.location  	= APPLY_DEX;
    af.modifier  	= 1;
    af.bitvector 	= 0;
    affect_to_char( victim, &af );

    af.duration  	= 6;
    af.location  	= APPLY_WIS;
    af.modifier  	= -4;
    af.bitvector 	= 0;
    affect_to_char( victim, &af );

    send_to_char( "You suddenly feel invincible: perfect in all ways!\n\r",victim );
    act("$n seems to grow larger, as $e swells with pride.", victim, NULL,NULL, TO_ROOM);
    return;
}

void spell_illusion ( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *vch;

    if (target_name[0] == '\0')
    {	
	send_to_char("What illusion do you wish to create?\r\n", ch);
	return;
    }

	if( IS_SET(ch->comm, COMM_NOCHANNELS) )
	{
		send_to_char("Your channel privileges have been revoked.\n\r",ch);
		return;
	}

    send_to_char("You attempt to create a convincing illusion.\r\n", ch);
    for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room )
    {
    	if (saves_spell(level+20, vch, DAM_NONE) && vch != ch)
    	{
	    
        act_new( "Illusory and faint, $T", vch, NULL, target_name, TO_CHAR, POS_RESTING);
    	}
    	else if (vch != ch) 
    	{ 
        act_new( "$T", vch, NULL, target_name, TO_CHAR, POS_RESTING);
    	}
    }
    return;
}

void spell_mirror( int sn, int level, CHAR_DATA *ch, void *vo, int target )	
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
  int mirrors,new_mirrors;
  CHAR_DATA *gch;
  CHAR_DATA *tmp_vict;
  char long_buf[MAX_STRING_LENGTH];
  char short_buf[20];
  int order;

  if (IS_NPC(victim))
  {
    act("You cannot mirror $N.\n\r",ch,NULL,victim,TO_CHAR);
    return;
  }

  if (is_affected (victim, gsn_treeform))
  {
    send_to_char("As hard as you try, you just can't seem to create mirror images of an oak tree.\n\r",ch);
    return;
  }

  for (mirrors = 0, gch = char_list; gch != NULL; gch = gch->next)
  {
    if (IS_NPC(gch)
    && is_affected(gch,gsn_mirror)
    && gch->doppel->name == victim->name)
      mirrors++;
  }

  /* Code above is replica of the one below, except that we're removing the Doppel affect check
   * since Doppel affect removes automatically if the victim logs off, which then bypasses
   * this count check and allows the spell caster to cast most mirror images.  Put back in if
   * anything seems to go haywire as a result of this removal.  May have to just alter the doppel
   * affect itself so that it doesn't remove from MOBs if and when a player logs off.
   *
 * for (mirrors = 0, gch = char_list; gch != NULL; gch = gch->next)
 * {
 *   if (IS_NPC(gch)
 *   && is_affected(gch,gsn_mirror)
 *   && is_affected(gch,gsn_doppelganger) 
 *   && gch->doppel->name == victim->name)
 *     mirrors++;
 * }
   */

  if (mirrors >= level/9)
  {
    if (ch==victim) 
      send_to_char("You cannot focus any more mirror images.\n\r",ch);
    else
      act("You cannot focus any more mirror images on $N.",ch,NULL,victim,TO_CHAR);
      return;
  }

  af.level     	= level;
  af.modifier  	= 0;
  af.location  	= 0;
  af.bitvector 	= 0;
  /* Find the permanent description of the mirror for after the mirrored
     person quits */

  for (tmp_vict = victim; is_affected(tmp_vict,gsn_doppelganger);
       tmp_vict = tmp_vict->doppel);

  sprintf(long_buf, "%s%s is here.\n\r", tmp_vict->name, tmp_vict->pcdata->title);
  sprintf(short_buf, tmp_vict->name );

  order = number_range(0,level/9 - mirrors); /* randomly place the real one */

  for (new_mirrors=0; mirrors + new_mirrors < level/9;new_mirrors++) 
  {
    gch = create_mobile( get_mob_index(MOB_VNUM_MIRROR) );

    free_string(gch->name);
    free_string(gch->short_descr);
    free_string(gch->long_descr);
    free_string(gch->description);
    gch->name 		= str_dup(tmp_vict->name);
    gch->short_descr 	= str_dup(short_buf);
    gch->long_descr 	= str_dup(long_buf);
    gch->description 	= (tmp_vict->description == NULL)? 
                       NULL : str_dup(tmp_vict->description);
    gch->sex 		= tmp_vict->sex;
    
    af.type 		= gsn_doppelganger;
    af.duration 		= level;
    affect_to_char(gch,&af);

    af.type 		= gsn_mirror;
    af.duration 		= -1;
    affect_to_char(gch,&af);

    gch->max_hit 		= gch->hit = 1;
    gch->level 		= 1;
    SET_BIT(gch->act, ACT_SENTINEL);
    gch->doppel 		= victim;
    gch->master 		= victim;
    char_to_room(gch,victim->in_room);

    if (new_mirrors == order)
    {
      char_from_room(victim);
      char_to_room(victim,gch->in_room);
    }
      

    if (ch==victim)
    {
      send_to_char("Your mirror image appears next to you!\n\r",ch);
      act("A mirror image of $N appears beside $M!",ch,NULL,victim,TO_ROOM);
    }
    else
    {
      act("A mirror of $N appears beside $M!",ch,NULL,victim,TO_CHAR);
      act("A mirror of $N appears beside $M!",ch,NULL,victim,TO_NOTVICT);
      send_to_char("Your mirror image appears next to you!\n\r",victim);
    }
  }
}

void spell_glamour( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
        if (victim == ch)
          send_to_char("You are already as beautiful as you can get!\n\r",ch);
        else
          act("$N can't get any more attractive.",ch,NULL,victim,TO_CHAR);
        return;
    }

    af.where	= TO_AFFECTS;
    af.type      	= sn;
    af.level     	= level;
    af.duration  	= level;
    af.location  	= APPLY_CHR;
    af.modifier  	= 1 + (level >= 18) + (level >= 25) + (level >= 32);
    af.bitvector 	= 0;
    affect_to_char( victim, &af );
    send_to_char( "You suddenly feel glamourous as beautiful sparkles cascade around you.\n\r", victim );
    act("Beautiful sparkles cascade around $n, enhancing $s appearance.",victim,NULL,NULL,TO_ROOM);
    return;
}

void spell_purify( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{                    

 OBJ_DATA *obj = (OBJ_DATA *) vo;
 
 if (obj->wear_loc != -1)
 {
  send_to_char("The item to be purified must be held.\n\r",ch);
  return;
 }

 if (obj->item_type != ITEM_FOOD && obj->item_type != ITEM_DRINK_CON)
 {
  send_to_char("The item cannot be purfied.\n\r",ch);
  return;
 }
 
 if (obj->value[3] != 0)
 {
   obj->value[3] = 0; 
   act("You rid $p of all its impurities.", ch, obj, NULL, TO_CHAR);
 }
 else
   act("$p does not contain any impurities.", ch, obj, NULL, TO_CHAR); 

 return;
}

void spell_vermin_plague( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    static const sh_int dam_each[] =
    {
          0,
          0,   0,   0,   0,   0,          0,   0,   0,   0,   0,
         13,  18,  23,  28,  33,         38,  43,  48,  53,  58,
         63,  68,  73,  78,  83,         85,  87,  89,  91,  93,
        100, 102, 104, 106, 108,        110, 112, 113, 115, 117,
        122, 124, 126, 128, 130,        132, 134, 136, 138, 140

    };

    int dam;
    if (victim == ch)
        return;

    if (victim->fighting == NULL && !IS_NPC(victim) )
	spell_yell(ch, victim, NULL);

    level       = UMIN(level, sizeof(dam_each)/sizeof(dam_each[0]) - 1);
    level       = UMAX(0, level);
    dam         = number_range( dam_each[level] / 2, (dam_each[level] *2)+15);

                         
    send_to_char("You send out a call for aid to rodents.\n\r",ch);
    act("$n calls forth an army of rodents against you.", ch, NULL,victim,TO_VICT);
    act("$n calls forth an army of rodents to $s aid.", ch, NULL, ch,TO_NOTVICT);


     if (IS_AFFECTED(victim,AFF_FLYING)) 
        dam = dam*.75;

    if ( saves_spell( level, victim, DAM_SLASH ) )
        dam /= 2;
    damage( ch, victim, dam, sn, DAM_SLASH, TRUE );
    return;
}

void spell_vortex( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{

  ROOM_INDEX_DATA *to_room;
  int lvnum;
  int uvnum;

  if ( IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
  || (ch->riding)) 
  {
   send_to_char("spell failed.\n\r",ch);
   return;
  }

    if ( ch->quit_timer < 0
    && !IS_IMMORTAL(ch)
    && (number_percent() < 50 ))
    {
    send_to_char("Your heart is beating so fast, you lose your concentration on the incantation.\n\r",ch);
    return;
  }

  lvnum = ch->in_room->area->min_vnum;
  uvnum = ch->in_room->area->max_vnum;
 
  for ( ; ;)
  {
     to_room = get_room_index( number_range( lvnum,uvnum ) );  

     if ( to_room != NULL )
     if ( can_see_room(ch,to_room)
     &&   !IS_SET(to_room->room_flags, ROOM_PRIVATE)
     &&   !IS_SET(to_room->room_flags, ROOM_SAFE)
     &&   !IS_SET(to_room->room_flags, ROOM_PET_SHOP)
     &&   !IS_SET(to_room->room_flags, ROOM_NO_RECALL)
     &&   !IS_SET(to_room->room_flags, ROOM_IMP_ONLY)
     &&   !IS_SET(to_room->room_flags, ROOM_GODS_ONLY)
     &&   !IS_SET(to_room->room_flags, ROOM_HEROES_ONLY)
     &&   !IS_SET(to_room->room_flags, ROOM_NEWBIES_ONLY)
     &&   !IS_SET(to_room->room_flags, ROOM_NOWHERE)
     &&   !IS_SET(to_room->room_flags, ROOM_NOMAGIC)
     &&   !IS_SET(to_room->room_flags, ROOM_SOLITARY) )
        break;
  }


  act( "$n disappears in a vortex of swirling wind!", ch, NULL, NULL, TO_ROOM );
  char_from_room(ch);
  char_to_room(ch,to_room);
  act( "$n appears in a vortex of swirling wind.", ch, NULL, NULL, TO_ROOM );
  do_look( ch, "auto" );
}

void spell_detect_presence( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{

  char buf[MAX_STRING_LENGTH]; 
  CHAR_DATA *victim;
  DESCRIPTOR_DATA *d;
  bool found = FALSE;

  if (IS_AFFECTED(ch,AFF_BLIND) && !IS_SET(ch->act,PLR_HOLYLIGHT))
  {
      send_to_char( "You can't see a thing!\n\r",ch);
      return;
  }

 {
 	if  (!str_cmp(class_table[ch->class].name, "Conjurer"))
 	{
 	send_to_char("You conjure forth a small scrying mirror and look deeply into it.\n\r",ch);
 	act("As $N speaks the words of magic, a small mirror appears in $s hand!\n\r",ch,NULL,ch,TO_ROOM);
 	send_to_char("You look into the mirror, scrying the location of all those whom are near!\n\r",ch);
        }
        else if (!str_cmp(class_table[ch->class].name, "Druid"))
        {
         send_to_char("You gather some water and pour it into a small wooden bowl in hopes to scrye your surroundings.\n\r",ch);
         act("$N pours a small amount of water into a simple wooden bowl.\n\r",ch,NULL,ch,TO_ROOM);
         act("$N looks into the still waters of the bowl.\n\r",ch,NULL,ch,TO_ROOM);
         send_to_char("Your magics reflect a faint image of those around you onto the water.\n\r",ch);
        }
        else if (!str_cmp(class_table[ch->class].name, "Enchanter"))
        {
         send_to_char("You take a small figurine from a pouch and cast a scrying spell on it.\n\r",ch);
         act("$n produces a small figurine and casts a spell on it!\n\r",ch,NULL,ch,TO_ROOM);
         act("The lips on the figurine begin to move as if speaking to $N.\n\r",ch,NULL,ch,TO_ROOM);
         send_to_char("The figurine informs you of:\n\r",ch);
        }
        
        else if (!str_cmp(class_table[ch->class].name, "Illusionist"))
        {
         send_to_char("You speak the words of a scrying spell.\n\r",ch);
         send_to_char("Faint images of these people dance before your eyes:\n\r",ch);
        }
        else if (!str_cmp(class_table[ch->class].name, "Necromancer"))
        {
         send_to_char("You grab a handful of finger bones from a pouch and cast them into a bowl.\n\r",ch);
         act("$n throws some bones into a bowl and stares at them.\n\r",ch,NULL,ch,TO_ROOM);
         send_to_char("The bones give you the knowledge of:\n\r",ch);
        }
        else 
        {
        	send_to_char("You feel the presence of:\n\r",ch);
        	
        }
 }



  for ( d = descriptor_list; d; d = d->next )
  {

    if ( d->connected == CON_PLAYING
    && ( victim = d->character ) != NULL
    &&   !IS_NPC(victim)
    &&   victim->in_room->sector_type != SECT_INSIDE
    &&   victim->in_room != NULL
    &&   victim->in_room->area == ch->in_room->area
    &&   can_see( ch, victim ) )
    {
      found = TRUE;
      sprintf( buf, "%-28s %s\n\r",
      (is_affected(victim,gsn_doppelganger) && !IS_SET(ch->act,PLR_HOLYLIGHT)) ? victim->doppel->name :
      (is_affected(victim,gsn_treeform) && !IS_SET(ch->act,PLR_HOLYLIGHT)) ? "A mighty oak tree" :
      victim->name,
      victim->in_room->name );
      send_to_char( buf, ch );
    }
  }
return;
}

void spell_sticks_to_snakes(int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  AFFECT_DATA af;
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  bool Hit = FALSE;
  int sect;

  sect = ch->in_room->sector_type;

  if ( ( ( sect == SECT_FIELD )
  || ( sect == SECT_FOREST )
  || ( sect == SECT_HILLS )
  || ( sect == SECT_MOUNTAIN ) 
  || ( sect == SECT_SWAMP ) )
  && (!IS_SET(ch->in_room->room_flags, ROOM_INDOORS) ) )
  {
    send_to_char("You transform all the sticks in your vicinity into venomous snakes!\n\r", ch);
    act( "$n transforms some nearby sticks into venomous snakes!", ch, NULL, NULL, TO_ROOM);
    for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
    {
        vch_next = vch->next_in_room;
        if (is_safe_spell(ch,vch,TRUE) )
	    continue;

	  Hit = TRUE;
        if ( is_same_group( vch, ch ) )
        {
	    act("$N fights off the snakes attacking $M.",ch,NULL,vch,TO_CHAR);
            act("$N fights off the snakes attacking $M.",ch,NULL,vch,TO_NOTVICT);
            send_to_char("You fight off the snakes attacking you.\n\r",vch);
            continue;
        }

        if ( saves_spell( level, vch, DAM_POISON ) )
        {
            send_to_char( "One of the snakes bites you!\n\r", vch );
            act("One of the snakes bites $N!", ch, NULL, vch, TO_CHAR);
            act("One of the snakes bites $N!", ch, NULL, vch, TO_NOTVICT);
            damage( ch, vch, 5, sn, DAM_PIERCE, TRUE );
            continue;
        }

        if (!is_affected(vch, sn))
        {
	  af.where		= TO_AFFECTS;
        af.type      	= sn;
        af.level     	= level;
        af.duration  	= level;
        af.modifier  	= -2;
        af.location  	= APPLY_STR;
        af.bitvector 	= 0;
        affect_join( vch, &af );
        send_to_char( "One of the snakes bites you!\n\r", vch );
        act("One of the snakes bites $N!", ch, NULL, vch, TO_CHAR);
        act("One of the snakes bites $N!", ch, NULL, vch, TO_NOTVICT);
	  }
        if (!is_affected(vch, gsn_poison))
        {
	  af.where		= TO_AFFECTS;
        af.type      	= gsn_poison;
        af.level     	= level/2;
        af.duration  	= level/2;
        af.modifier  	= 0;
        af.location  	= 0;
        af.bitvector 	= AFF_POISON;
        affect_join( vch, &af );
        send_to_char( "You feel very sick.\n\r", vch );
        act("$N looks very ill.", ch, NULL, vch, TO_CHAR);
        act("$N looks very ill.", ch, NULL, vch, TO_NOTVICT);
        }
        damage( ch, vch, (level*1.25 + number_range(10,20)), sn,DAM_PIERCE, TRUE );
      }
      if ( !Hit)
      {
	send_to_char("The snakes wander off.\n\r",ch);
	act("The snakes wander off.\n\r",ch,NULL,NULL, TO_ROOM);
      }
    }
    else
    {
      act ("There aren't enough sticks here.", ch, NULL, NULL, TO_CHAR );
      return;
    }
}

void spell_sleepless_curse( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
	
  if( ch == victim )
  {
    send_to_char("Why would you want to cast this on yourself?\n\r",ch);
    return;
  }

  if( IS_AFFECTED2(victim,AFF_SLEEPCURSE) )
  {
    send_to_char("Their mind has already been cursed to prevent sleep.\n\r",ch );
    return;
  }

  if( IS_AFFECTED(victim,AFF_SLEEP) )
  {
    send_to_char("They have been forced to sleep through magic. Your spell fizzles out and dies.\n\r",ch );
    return;
  }

  if ( !IS_AWAKE(victim) )
  {
    send_to_char("You can only cast this spell on people who are already awake.\n\r",ch );
    return;
  }

  if( saves_spell(level,victim,DAM_NEGATIVE) )
  {
    send_to_char("Your spell didn't seem to affect them.\n\r",ch );
    return;
  }

  af.where		= TO_AFFECTS2;
  af.type     	= sn;
  af.level		= level;
  af.duration 	= level/4;
  af.modifier 	= 0;
  af.location 	= 0;
  af.bitvector 	= AFF_SLEEPCURSE;
  affect_to_char( victim, &af );
  send_to_char("You have been inflicted with a sleeping curse!\n\r", victim );
  act("You have successfully inflicted $N with a sleeping curse.", ch, NULL, victim, TO_CHAR);
  act("$n looks very exhausted and weak.",victim,NULL,NULL,TO_ROOM);
  return;
}

void spell_sequester( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    AFFECT_DATA af;

    /* deal with the object case first */
    if (target == TARGET_OBJ)
    {
	obj = (OBJ_DATA *) vo;
	if( obj->timer > 0 )
	{
	  act("$p isn't in any condition for this spell to be cast upon it.",ch,obj,NULL,TO_CHAR);
	  return;
	}

      if ( number_percent() > 50)
      {
        act("$p flashes out of sight briefly then returns to normal.",ch,obj,NULL,TO_CHAR);
        return;
      }
      else
      {
	  REMOVE_BIT( obj->extra_flags, ITEM_INVIS );
	  REMOVE_BIT( obj->extra_flags, ITEM_NOLOCATE );
	  SET_BIT( obj->extra_flags, ITEM_NOLOCATE );
	  SET_BIT( obj->extra_flags, ITEM_INVIS);
	  act("$p fades out of sight.",ch,obj,NULL,TO_CHAR);
      }
    }
  else
  {
    /* character target */
    victim = (CHAR_DATA *) vo;

    if ( ch == victim )
    {
      send_to_char( "Your body goes limp for a split second, perhaps you should try it on someone else?\n\r", ch );
      return;
    }

    if ( IS_AFFECTED2( victim, AFF_PARALYSIS ))
    {
      act("$N is already in a state of suspended animation.",ch,NULL,victim,TO_CHAR);
      return;
    }

    af.where	= TO_AFFECTS2;
    af.type      	= sn;
    af.level	= level;
    af.duration  	= 2;
    af.modifier  	= 0;
    af.location  	= 0;
    af.bitvector 	= AFF_PARALYSIS;
    affect_to_char( victim, &af );
    act( "$n renders you comatose, leaving you in a state of suspended animation", ch, NULL, victim, TO_VICT    );
    act( "$N becomes comatose, leaving $M in a state of suspended animation.", ch, NULL, victim, TO_NOTVICT );
    return;
  }
}

void spell_preserve_dead( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    int result, fail;

	if ((obj = get_obj_here(ch, NULL, target_name)) == NULL)
	{
	send_to_char("Preserve what???\n\r",ch);
	return;
	}

   if( (obj->pIndexData->vnum == OBJ_VNUM_CORPSE_NPC) 
    || (obj->pIndexData->vnum == OBJ_VNUM_CORPSE_PC) )
  {
    if (obj->timer == 0)
    {
	act("$p is too far gone to save it from decay.",ch,obj,NULL,TO_CHAR);
	return;
    }
	    
    fail = 10;  /* base 10% chance of failure */
    if (obj->timer < 10)
	fail += 40;  /*harder for further decay*/
    fail -= level/5;
    fail += (obj->timer / 4);
    fail = URANGE(5,fail,95);
    result = number_percent();
    

    if (result < (fail / 3))  
    {
        act("$p shrivels up and decays in a cloud of ash!",ch,obj,NULL,TO_CHAR);
        act("$p shrivels up and decays in a cloud of ash!",ch,obj,NULL,TO_ROOM);
        extract_obj(obj);
        return;
    }
    if (result < (fail / 2)) 
    {
        act("$p begins to show signs of further decay.",ch,obj,NULL,TO_CHAR);
        act("$p begins to show signs of further decay.",ch,obj,NULL,TO_ROOM);
        obj->timer /= 2;
        return;
    }
    if ( result <= fail )  /* failed, no bad result */
    {
        send_to_char("Nothing seemed to happen.\n\r",ch);
        return;
    }

    obj->timer = 40;
    act("$p absorbs the shadows around you, slowing its decay.",ch,obj,NULL,TO_CHAR);
    act("$p absorbs the shadows around you, slowing its decay.",ch,obj,NULL,TO_ROOM);
    return;
  }
  else
  {
     send_to_char("Hrmm. Got corpse?\n\r",ch);
     return;
  }

}

void spell_ray_of_fatigue( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
	
	if( ch == victim )
	{
		send_to_char("You feel yourself beginning to move slower... prehaps you should cast this on\n\rsomebody else?\n\r",ch);
		return;
	}

	if( saves_spell(level,victim,DAM_NEGATIVE)
	|| (IS_AFFECTED(victim,AFF_SLOW) && (victim->move < 50)) )
	{
		send_to_char("Your spell didn't seem to affect them.\n\r",ch );
		return;
	}

    if( !IS_AFFECTED(victim,AFF_SLOW) )
    {
      af.where		= TO_AFFECTS;
      af.type     	= sn;
      af.level		= level;
      af.duration 	= 10;
      af.location 	= APPLY_DEX;
      af.modifier  	= -1 - (level >= 18) - (level >= 25) - (level >= 32);
      af.bitvector 	= AFF_SLOW;
      affect_to_char( victim, &af );
    }

    if( victim->move > 50 ) victim->move /=3;

	send_to_char("You suddenly feel very fatigued and tired!\n\r", victim );
	act("$n appears very fatigued and tired.",victim,NULL,NULL,TO_ROOM);
	return;
}

void spell_mooncloak( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    	CHAR_DATA *victim = (CHAR_DATA *) vo;
    	AFFECT_DATA af;

    	if (( time_info.hour > 6) && (time_info.hour < 20))
    	{
      	send_to_char( "The moon cannot offer it's protective powers during the day.\n\r", ch );
      	return;
    	}

	if( !IS_OUTSIDE(ch) )
	{
		send_to_char("You must be outside, underneath the moon.\n\r",ch);
		return;
	}

	if( IS_AFFECTED2(victim,AFF_MOONCLOAK) )
      {
		send_to_char("The powers of the moon already protect them.\n\r", ch);
		return;
      }

	if( is_affected(victim, skill_lookup("shield"))
	   || is_affected(victim, skill_lookup("sanctuary"))
	   || is_affected(victim, skill_lookup("armor")) )
	{
	   if( ch == victim)
	   {
		send_to_char("Your spell had no affect on you!\n\r",ch);
		return;
	   }
	   else
	   {
		send_to_char("Your spell didn't seem to affect them.\n\r",ch );
		return;
	   }
	}

      af.where		= TO_AFFECTS2;
      af.type     	= sn;
      af.level		= level;
      af.duration 	= level/3;
      af.modifier 	= 0;
      af.location 	= 0;
      af.bitvector 	= AFF_MOONCLOAK;
      affect_to_char( victim, &af );

	send_to_char("You have been protected by the powers of the moon!\n\r",victim);
      act("$n raises $s hand, and a beam of energy from the sky surrounds $N.", ch, NULL, victim, TO_NOTVICT);
      act("The energies of the moon swirl around $n, protecting $m from harm!",victim,NULL,NULL,TO_ROOM);
      return;
}

void spell_blood_boil( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int percent;
    static const sh_int dam_each[] = 
    {
         0,
         0,  0,  0,  0,  0,      0,  0,  0,  0,  0,
         0,  0, 25, 26, 28,     30, 30, 30, 31, 31,
        32, 32, 33, 33, 34,     34, 35, 36, 36, 37,
        37, 38, 39, 40, 40,     41, 41, 42, 42, 43,
        44, 44, 45, 45, 46,     47, 47, 48, 49, 50
    };
    int dam;
    percent = 100;
    if (victim == ch)
        return;
    
    
    if (victim->fighting == NULL && !IS_NPC(victim))
        spell_yell(ch, victim, NULL);

    level       = UMIN(level, sizeof(dam_each)/sizeof(dam_each[0]) - 1);
    level       = UMAX(0, level);
    dam         = number_range( dam_each[level] / 2,  dam_each[level] * 2 );
    if ( saves_spell( level, victim, DAM_NEGATIVE ) )
        dam /= 2;

    if ( victim->max_hit > 0 )
	percent = ( 100 * victim->hit ) / victim->max_hit;
    else
	return;

   if (percent <=  50) 
	{
    	send_to_char("Your blood boils in your open wounds!\n\r", victim);
	damage( ch, victim, dam, sn, DAM_NEGATIVE, TRUE );
	return;
	}
   send_to_char("There aren't any open wounds to boil the blood.\n\r", ch);
   return;
}

void spell_god_fire(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
    if (victim == ch)
	{
        send_to_char("The gods will send their own punishment if they feel it necessary!\n\r",ch);
        return;
	}
    
    if (victim->fighting == NULL && !IS_NPC(victim) )
        spell_yell(ch, victim, NULL);

    act("$n calls forth the holy fires of the gods upon $N!", ch,NULL,victim,TO_NOTVICT );
    act("$n has assailed you with the scathing fires of the gods!",ch,NULL,victim,TO_VICT);
    act("You assail $N with the holy fires of the gods!\n\r",ch,NULL,victim,TO_CHAR);

    dam = dice( level, 10 );
    if ( saves_spell( level, victim, DAM_HOLY ) )
        dam /= 2;
    damage( ch, victim, dam, sn, DAM_HOLY, TRUE );
}

void spell_wrath(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
    if (victim == ch)
        return;
    if (victim->fighting == NULL && !IS_NPC(victim) )
        spell_yell(ch, victim, NULL);

    if (!IS_NPC(ch) && (victim->alignment < 249 ) )
    {
        act("$n calls down the wrath of the gods on $N!\n\r",ch,NULL,victim,TO_NOTVICT );
        act("$n punishes you with the wrath of the gods!\n\r",ch,NULL,victim,TO_VICT);
        act("You call down the wrath of the gods on $N!\n\r",ch,NULL,victim,TO_CHAR);
    
        dam = dice( level * 12 / 11, 10 );
        if ( saves_spell( level, victim, DAM_HOLY ) )
            dam /= 2;
        damage( ch, victim, dam, sn, DAM_HOLY, TRUE );
    }
    if (!IS_NPC(ch) && (victim->alignment > 250) )
    {
        send_to_char("The gods do not enhance your wrath.\n\r",ch);
        dam = dice(level / 10, 1);
        if ( saves_spell( level, victim, DAM_HOLY ) )
            dam /= 5;
        damage( ch, victim, dam, sn, DAM_HOLY, TRUE );
    }
}


void spell_great_wasting( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    if (victim == ch)
        return;
    if (victim->fighting == NULL && !IS_NPC(victim) )
	spell_yell(ch, victim, NULL);

    if (IS_AFFECTED2(victim, AFF_WASTING))
    {
	act("$N is already suffering from that curse.",ch,NULL,victim,TO_CHAR);
	return;
    }

    if (saves_spell(level,victim, DAM_NEGATIVE) || 
        (IS_NPC(victim) && IS_SET(victim->act,ACT_UNDEAD)))
    {
        if (ch == victim)
          send_to_char("You seem a bit dizzy, but it passes.\n\r",ch);
        else
          act("$N seems to be unaffected.",ch,NULL,victim,TO_CHAR);
        return;
    }

    af.where	= TO_AFFECTS2;
    af.type       = sn;
    af.level      = level;
    af.duration  	= level * 3/4;
    af.location  	= APPLY_NONE;
    af.modifier  	= 0; 
    af.bitvector 	= AFF_WASTING;
    affect_join(victim,&af);
   
    send_to_char
      ("Your breath is knocked out of you and you feel more tired.\n\r",victim);
    act("$n slumps over as $s health deteriorates.\n\r",victim,NULL,NULL,TO_ROOM);
}

bool fumble_obj( CHAR_DATA *victim, OBJ_DATA *obj_drop, int level, bool drop )
{
    if ( drop )
    {
        if ( !can_drop_obj( victim, obj_drop ) )
            return FALSE;
    }
    else
    {
        if ( IS_OBJ_STAT( obj_drop, ITEM_NOREMOVE ) )
            return FALSE;
    }

    if ( saves_spell( level, victim, DAM_BASH ) )
    {
        act( "You nearly $T $p, but manage to keep your grip.",
            victim, obj_drop, drop ? "drop" : "lose hold of", TO_CHAR );
        act( "$n nearly $T $p, but manages to keep $s grip.",
            victim, obj_drop, drop ? "drops" : "loses hold of", TO_ROOM );
        return FALSE;
    }

    if ( drop )
    {
        obj_from_char( obj_drop );
        obj_to_room( obj_drop, victim->in_room );
    }
    else
    {
        unequip_char( victim, obj_drop );
    }

    act( "You fumble and $T $p!",
        victim, obj_drop, drop ? "drop" : "lose hold of", TO_CHAR );
    act( "$n fumbles and $T $p!",
        victim, obj_drop, drop ? "drops" : "loses hold of", TO_ROOM );
    return TRUE;
}


void spell_fumble( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    OBJ_DATA   *obj;
    OBJ_DATA   *obj_drop;
    int         carry;
    int         check;
    int         count;
    int         drop;
    bool        fumbled = FALSE;

    if ( !IS_AWAKE( victim ) )
        return;

    carry = 0;
    for ( obj = victim->carrying; obj; obj = obj->next_content )
        carry++;

    drop = carry - can_carry_n( victim ) + 5;

    for ( check = 0; check < drop; check++ )
    {
        obj_drop = NULL;
        count = 0;

        for ( obj = victim->carrying; obj; obj = obj->next_content )
        {
            if ( obj->wear_loc == WEAR_NONE
            && number_range( 0, count++ ) == 0 )
                obj_drop = obj;
        }

        if ( !obj_drop )
            break;

        fumbled = fumble_obj( victim, obj_drop, level, TRUE ) || fumbled;
    }

    if ( ( obj_drop = get_eq_char( victim, WEAR_HOLD ) ) )
        fumbled = fumble_obj( victim, obj_drop, level, FALSE ) || fumbled;

    if ( ( obj_drop = get_eq_char( victim, WEAR_LIGHT ) ) )
        fumbled = fumble_obj( victim, obj_drop, level, FALSE ) || fumbled;

     if ( ( obj_drop = get_eq_char( victim, WEAR_WIELD ) ) )
        fumbled = fumble_obj( victim, obj_drop, level, FALSE ) || fumbled;

    if ( !fumbled )
    {
        send_to_char( "You stumble momentarily, but quickly recover.\n\r",
            victim );
        act( "$n stumbles momentarily, but quickly recovers.",
            victim, NULL, NULL, TO_ROOM );
    }

    return;
}

void spell_regeneration( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    if ( is_affected( victim, sn ) )
    {
        if (victim == ch)
          send_to_char("You are already regenerating.\n\r",ch);
        else
          act("$N is already regenerating.",ch,NULL,victim,TO_CHAR);
        return;
	}

    af.where	= TO_AFFECTS;
    af.type    	= sn;
    af.level     	= level;
    af.duration	= number_fuzzy( level / 2 );
    af.location  	= APPLY_NONE;
    af.modifier  	= 0;
    af.bitvector 	= AFF_REGENERATION;
    affect_to_char( victim, &af );
    send_to_char( "You attune yourself with regenerative life forces.\n\r", ch );
    act( "$n's wounds begin to slowly heal before your eyes.", victim, NULL, NULL, TO_ROOM );
    send_to_char( "You feel your body begin to slowly regenerate.\n\r", victim );
    if ( ch != victim )
    return;
}

void spell_sparkshot( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const sh_int dam_each[] = 
    {
         0,
         4,  4,  5,  5,  6,      7,  7,  7,  7,  7,
         8,  8,  8,  8,  8,      9,  9,  9,  9,  9,
        10, 10, 10, 10, 10,     11, 11, 11, 11, 11,
        12, 12, 12, 12, 12,     13, 13, 13, 13, 13,
        14, 14, 14, 14, 14,     15, 15, 15, 15, 15
    };
    int dam;
    if (victim == ch)
        return;
    if (victim->fighting == NULL && !IS_NPC(victim) )
	spell_yell(ch, victim, NULL);

    level       = UMIN(level, sizeof(dam_each)/sizeof(dam_each[0]) - 1);
    level       = UMAX(0, level);
    dam         = number_range( dam_each[level] / 2, dam_each[level] * 2);
    if ( saves_spell( level, victim, DAM_ENERGY ) )
        dam /= 2;
    damage( ch, victim, dam, sn, DAM_FIRE,TRUE );
    return;
}

void spell_quench(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA*)vo;
  int amount;

  if(!IS_NPC(victim))
  {
  amount = number_fuzzy(5 + ch->level);
  gain_condition( victim, COND_THIRST, amount);
  send_to_char("Your thirst subsides.\n\r",victim);
  if( victim != ch )
  act("You quench $S thirst.\n\r",ch,NULL,victim,TO_CHAR);
  return;
  }

  act("Let $M drink for $Mself.\n\r",ch,NULL,victim,TO_CHAR);
  return;
}

void spell_banish( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    ROOM_INDEX_DATA *pRoomIndex;
    char buf[MAX_STRING_LENGTH];
    if (victim == ch)
    {
        send_to_char( "You can't banish yourself.\n\r", ch );
        return;
    }

    if ( victim->quit_timer < 0
    && !IS_IMMORTAL(ch)
    && (number_percent() < 50 )) 
    {
	send_to_char("They can not be banished in an attempt to escape from fighting.\n\r",ch);
	return;
    }

    if ( victim->alignment > 250 )
    {
        send_to_char( "You can't banish someone so good.\n\r", ch );
        return;
    }

    if (IS_NPC(victim) )
    {
      send_to_char("You cannot banish a non-player.\n\r",ch);
      return;
    }
   
    if (IS_IMMORTAL(victim))
    {
      send_to_char("You cannot banish one so powerful.\n\r",ch);
      return;
    }

    if ( victim->in_room == NULL
    || IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
    || IS_NPC(ch)
    || ( !IS_NPC(ch) && victim->fighting != NULL )
    || ( victim != ch
    && ( saves_spell( level, victim, DAM_MENTAL ) 
    || saves_spell( level, victim, DAM_ENERGY ) ) ) )
    {
        send_to_char( "You failed.\n\r", ch );
     if ( !IS_NPC(victim) )
        spell_yell(ch, victim, NULL);
     return;
    }
    for ( ; ; )
    {
        pRoomIndex = get_room_index( number_range( 0, 18051 ) );
        if ( pRoomIndex != NULL )
        if ( can_see_room(ch,pRoomIndex)
        &&   !IS_SET(pRoomIndex->room_flags, ROOM_PRIVATE)
        &&   !IS_SET(pRoomIndex->room_flags, ROOM_SAFE)
        &&   !IS_SET(pRoomIndex->room_flags, ROOM_PET_SHOP)
        &&   !IS_SET(pRoomIndex->room_flags, ROOM_NO_RECALL)
        &&   !IS_SET(pRoomIndex->room_flags, ROOM_IMP_ONLY)
        &&   !IS_SET(pRoomIndex->room_flags, ROOM_GODS_ONLY)
        &&   !IS_SET(pRoomIndex->room_flags, ROOM_HEROES_ONLY)
        &&   !IS_SET(pRoomIndex->room_flags, ROOM_NEWBIES_ONLY)
        &&   !IS_SET(pRoomIndex->room_flags, ROOM_NOWHERE)
        &&   !IS_SET(pRoomIndex->room_flags, ROOM_NOMAGIC)
        &&   !IS_SET(pRoomIndex->room_flags, ROOM_SOLITARY) )
            break;
    }
    if ( ( victim != ch) && !IS_NPC(victim) )
    {
      sprintf(buf, "Begone, vile %s!", 
      (is_affected(victim, gsn_doppelganger)&& (!IS_SET(ch->act,PLR_HOLYLIGHT)))? victim->doppel->name :
      (is_affected(victim, gsn_doppelganger)&& (!IS_SET(ch->act,PLR_HOLYLIGHT)))? "oak tree" :
       victim->name);
	do_yell( ch, buf);
     send_to_char("You have been banished!\n\r",victim);
     act( "$n is hurled from the room with great force!", victim, NULL, NULL, TO_ROOM );
     char_from_room( victim );
     char_to_room( victim, pRoomIndex );
     if (victim->riding && !IS_NPC(victim))
     {
	victim->riding->rider = NULL;
	victim->riding        = NULL;  
     }
     act( "Out of nowhere, $n suddenly appears.", victim, NULL, NULL, TO_ROOM );
     do_look( victim, "auto" );
     return;
    }

   if ( ( victim != ch) && IS_NPC(victim) )
   {
     sprintf(buf, "Begone, vile one!");
     do_yell( ch, buf);
     send_to_char("You have been banished!\n\r",victim);
     act( "$n is hurled from the room with great force!", victim, NULL, NULL, TO_ROOM );
     char_from_room( victim );
     char_to_room( victim, pRoomIndex );
     if (victim->riding)
     {
	victim->riding->rider = NULL;
	victim->riding        = NULL;  
     }
     if (victim->rider)
     {
     victim->rider->riding	= NULL;
     victim->rider->position 	= POS_RESTING;
     victim->rider		= NULL;
     }
     act( "Out of nowhere, $n suddenly appears.", victim, NULL, NULL, TO_ROOM );
     do_look( victim, "auto" );
     return;
   }
}

void spell_salvation( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    ROOM_INDEX_DATA *pRoomIndex;

    pRoomIndex = get_room_index( 3054 );

    if (victim == ch)
    {
        send_to_char( "You can't be your own salvation.\n\r", ch );
        return;
    }

    if ( victim->alignment < -249 )
    {
        send_to_char( "You can't save someone so evil.\n\r", ch );
        return;
    }

    if (victim->in_room == NULL
    ||  IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
    ||  IS_NPC(ch)
    ||  IS_NPC(victim)
    || ( victim->level >= 102)
    || ( victim == ch )
    || ( victim->riding ) )
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }

    if ( victim != ch ) 
    {
    ch->move = -10;
    send_to_char("A Crusader of the Holy Order has been your salvation!\n\r", victim);
    act( "$n disappears into a white flame!", victim, NULL, NULL, TO_ROOM );
    char_from_room( victim );
    char_to_room( victim, pRoomIndex );
    victim->move = victim->max_move;
    victim->mana = victim->max_mana;
    victim->hit = victim->max_hit;
    act( "$n suddenly appears from a white flame!", victim, NULL, NULL, TO_ROOM );
    do_look( victim, "auto" );
    return;
    }
}

/*
void spell_dark_whisper( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    char argument[MAX_INPUT_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *wch;
    argument = one_argument( ch, victim );
   
    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
        send_to_char( "Convey what message to whom?\n\r", ch );
        return;
    }
	
	if( IS_SET(ch->comm, COMM_NOCHANNELS) )
	{
		send_to_char("Your channel privileges have been revoked.\n\r".ch);
		return;
	}

    victim = NULL;
    if( ch->in_room != NULL )
    {
        for( wch = ch->in_room->people; wch != NULL; wch = wch->next_in_room )
        {
            if( IS_NPC( wch ) )
            {
                if( is_name( arg, wch->name ) )
                {
                    victim = wch;
                    break;
                }
            }
            else
            {
                if( !str_prefix( arg, wch->name ) )
                {
                    victim = wch;
                    break;
		}
	    }
         }
    }
    victim = NULL;
    if( ch->in_room != NULL )
    {
        for( wch = ch->in_room->people; wch != NULL; wch = wch->next_in_room )
        {
            if( IS_NPC( wch ) )
            {
                if( is_name( arg, wch->name ) )
                {
                    victim = wch;
                    break;
                }
            }
            else
            {
                if( !str_prefix( arg, wch->name ) )
                {
                    victim = wch;
                    break;
                }
            }
        }
    }
    if( victim == NULL )
    {
        for( wch = char_list; wch != NULL; wch = wch->next )
        {
            if( IS_NPC( wch ) )
                continue;
            if( !str_prefix( arg, wch->name ) )
            {
                victim = wch;
                break;
            }
        }
    }
    if( victim == NULL
    || !can_see (ch, victim) )
    {
        send_to_char( "They are not here.\n\r", ch );
        return;
    }
 
	if( IS_SET(victim->comm, COMM_NOCHANNELS) )
	{
		send_to_char("They are not privileged enough to hear your message.\n\r",ch);
		return;
	}

    if ( victim->desc == NULL && !IS_NPC(victim))
    {
        char buf[MAX_STRING_LENGTH];
 
        act("You sense an emptiness that tells you $N's mind cannot receive your message."
            "You may wish to try your incantation again when $E returns.",
            ch,NULL,victim,TO_CHAR);
 
        buf[0] = UPPER(buf[0]);
        add_buf(victim->pcdata->buffer,buf);
        return;
    }
    act( "With the aid of dark sorcery, you convey your message.", ch, NULL, NULL, TO_CHAR );
    act_new("A cold, dark voice in your mind whispers '$t'",
       NULL,argument,victim,TO_VICT,POS_DEAD);

    return;
}
*/

void spell_flash(int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  AFFECT_DATA af;
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;

  send_to_char(
    "With a wave of your hand, a blindingly bright flash of light streaks through the room!\n\r",ch);
  act( "With a gesture, $n speaks an incantation and a blindingly bright flash of light streaks through the room!", ch, NULL, NULL, TO_ROOM);

  for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
  {
    vch_next = vch->next_in_room;
    if (is_safe_spell(ch,vch,TRUE) )
      continue;

    if ( IS_AFFECTED(vch, AFF_BLIND)
    || saves_spell(level,vch,DAM_LIGHT))
    {
      continue;
    }

    if ( is_same_group( vch, ch ) )
    {
      continue;
    }

    af.where	= TO_AFFECTS;
    af.type      	= sn;
    af.level     	= level;
    af.duration  	= 1;
    af.modifier  	= 0;
    af.location  	= 0;
    af.bitvector 	= AFF_BLIND;
    affect_to_char( vch, &af );
    send_to_char( "You are momentarily blinded!\n\r", vch );
    act("The bright flash of light blinds $N!.", ch, NULL, vch, TO_CHAR);
    act("The bright flash of light blinds $N!.", ch, NULL, vch, TO_NOTVICT);
  }
 return;
}

void spell_rabbit_summon( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *gch;
  CHAR_DATA *rabbit;
  AFFECT_DATA af;
  int rabbit_count=0;

  if ( is_affected(ch, sn))
  {
      send_to_char("You have not yet recovered enough to summon another rabbit.\n\r", ch);
      return;
  }

  send_to_char("You try to summon a rabbit.\n\r", ch);
  act("$n tries to summon a rabbit.", ch, NULL, NULL, TO_ROOM);

  for (gch = char_list; gch != NULL; gch = gch->next)
  {
      if (IS_NPC(gch)
	&& IS_AFFECTED(gch, AFF_CHARM)
	&& gch->master == ch
	&& gch->pIndexData->vnum == MOB_VNUM_RABBIT)
	  rabbit_count ++;
  }

  if (rabbit_count >=3)
	{
	send_to_char("You cannot control more than three rabbits at a time.\n\r",ch);
	return;
	}

  rabbit = create_mobile( get_mob_index(MOB_VNUM_RABBIT));

  send_to_char("You summon a rabbit.\n\r",ch);
  act("$n reaches into a fold of $s garment and produces a rabbit.", 
     ch, NULL, NULL, TO_ROOM);
  char_to_room(rabbit, ch->in_room);

  af.where			= TO_AFFECTS;
  af.type               = sn;
  af.level              = level; 
  af.duration           = 10;
  af.bitvector          = 0;
  af.modifier           = 0;
  af.location           = APPLY_NONE;
  affect_to_char(ch, &af);  

  rabbit->master 		= ch;
  rabbit->leader 		= ch;
  SET_BIT(rabbit->act, ACT_PET);
  rabbit->comm = COMM_NOTELL|COMM_NOSHOUT|COMM_NOCHANNELS;
  ch->pet = rabbit;
  return;
}

void spell_empowerment( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
        if (victim == ch)
          send_to_char("You are already as robust as you can get!\n\r",ch);
        else
          act("$N is already as robust as possible.", ch, NULL, victim, TO_CHAR);
        return;
    }

    af.where	= TO_AFFECTS;
    af.type      	= sn;
    af.level     	= level;
    af.duration  	= level;
    af.location  	= APPLY_CON;
    af.modifier  	= 1 + (level >= 40) + (level >= 51);
    af.bitvector 	= 0;
    affect_to_char( victim, &af );
    send_to_char( "You feel more healthy and robust!\n\r", victim );
    act("$N looks more robust, almost overflowing with good health.", 
      ch, NULL, victim,TO_NOTVICT);
    if ( ch != victim )
        send_to_char( "Ok.\n\r", ch );
    return;
}

void spell_sunray( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{ 
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af; 
    static const sh_int dam_each[] = 
    {
         0,
         0,  0,  0,  0,  0,      0,  0,  0,  0,  0,
        25, 30, 35, 40, 45,     50, 55, 60, 65, 70,
        75, 80, 85, 90, 95,     97, 99,101,103,105,
       107,109,111,113,115,    117,119,121,123,125,
       127,129,131,133,135,    137,139,141,143,145
    };
    int dam;

    act("$n opens $s hand as a bright beam of sunlight strikes at $N with incredible force.",ch,NULL,victim,TO_NOTVICT);
    act("$n opens $s hand as a bright beam of sunlight strikes you with incredible force!",ch,NULL,victim,TO_VICT);
    act("You direct a bright beam of intensely hot light at $N!",ch,NULL,victim,TO_CHAR);
    level       = UMIN(level, sizeof(dam_each)/sizeof(dam_each[0]) - 1);
    level       = UMAX(0, level);
    dam         = number_range( dam_each[level] / 2,  dam_each[level] * 2 );

    if ( saves_spell( level, victim, DAM_LIGHT) )
        dam /= 2;

    else if (!IS_AFFECTED(victim,AFF_BLIND))
    {
      act ("$n is blinded by the bright beam of sun.",victim,NULL,NULL,TO_ROOM);

	af.where		= TO_AFFECTS;
      af.type      	= sn;
      af.level     	= level;
      af.location  	= APPLY_HITROLL;
      af.modifier  	= -4;
      af.duration  	= (level/4)+5;
      af.bitvector 	= AFF_BLIND;
      affect_to_char( victim, &af );

    }
    damage( ch, victim, dam, sn, DAM_LIGHT, TRUE);
    return;
}

void spell_scrye( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim;
    ROOM_INDEX_DATA *location;
    char buf  [MAX_STRING_LENGTH];
    // char arg1 [MAX_INPUT_LENGTH];
    EXIT_DATA *pexit;
    bool found;
    bool fAuto = FALSE;
    int door;
    int *dirtab;

    if (((victim = get_char_world(ch, target_name)) == NULL)
    ||   !can_see_room(ch,victim->in_room)) 
    {
	send_to_char("You are unable to find your target.\n\r",ch);
	return;
    }

    if ((victim == ch)
    ||  (victim->in_room == NULL))
    {
	send_to_char("You already have a view of your own surroundings.\n\r",ch);
	return;
    }

    if ((victim->level >= level + 9)
    || (!IS_NPC(victim)
    && (victim->level > LEVEL_HERO)
    && (victim->level > ch->level))
    || (IS_NPC(victim) && IS_SET(victim->imm_flags,IMM_SUMMON))
    || ( IS_NPC(victim)
    && saves_spell(level, victim, DAM_MENTAL)))
    {
	send_to_char("You cannot focus on someone resistant to your scrye.",ch);
	return;
    }

    if ((IS_SET(victim->in_room->room_flags, ROOM_SAFE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
    ||   IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
    ||   IS_SET(victim->in_room->room_flags, ROOM_GODS_ONLY)
    ||   IS_SET(victim->in_room->room_flags, ROOM_IMP_ONLY)
    ||   IS_SET(ch->in_room->room_flags,     ROOM_NO_RECALL))
    &&   !IS_IMMORTAL(ch))

    {
        send_to_char("Your vision cannot extend to them.\n\r", ch );
        return;
    }

    if ((location = victim->in_room) == NULL )
    {
        send_to_char( "No such location.\n\r", ch );
        return;
    }

    send_to_char("You send your vision outward.\n\r",ch);
    if (saves_spell(level, victim, DAM_MENTAL))
	send_to_char("You get the feeling someone is watching you.\n\r",victim);

    send_to_char(victim->in_room->name, ch );

    if(IS_IMMORTAL(ch) && (IS_NPC(ch) || IS_SET(ch->act,PLR_HOLYLIGHT)))
    {
      sprintf(buf, " [Room %d]", victim->in_room->vnum);
      send_to_char(buf,ch);
    }
    send_to_char( "\n\r", ch );

    //if (arg1[0] == '\0' || (!IS_NPC(ch) && !IS_SET(ch->comm, COMM_BRIEF) ) )
    //{
      send_to_char( "  ",ch);
      send_to_char( victim->in_room->description, ch );
   // }

    send_to_char("\n\r",ch);

    if ( IS_NPC(ch) || IS_SET(ch->act, PLR_AUTOEXIT) )
    {
	fAuto = TRUE;
    }

    if(IS_AFFECTED2(ch, AFF_LOOKING_GLASS))
      dirtab = oppossite_dir;
    else
      dirtab = regular_dir;

	strcpy( buf, fAuto ? "[Exits:" : "Obvious exits:\n\r" );
	found = FALSE;
	for (door = 0; door <= 5; door++ )
	{
		if ((pexit = victim->in_room->exit[dirtab[door]] ) != NULL &&
		     pexit->u1.to_room != NULL &&
		     !IS_SET(pexit->exit_info, EX_CLOSED) )
		{
		   found = TRUE;
		   if ( fAuto )
			{
			   strcat( buf, " " );
			   strcat( buf, dir_name[dirtab[door]] );
			}
			else
			{
			   sprintf( buf + strlen(buf), "%-5s - %s\n\r",
			   capitalize( dir_name[dirtab[door]] ),
			   room_is_dark( pexit->u1.to_room )
                       ?  "Too dark to tell"
                       : pexit->u1.to_room->name);
			}
		}
	}

	send_to_char(buf,ch);

	if (!found)
	{
	   strcat(buf, fAuto ? " none" : "None.\n\r" );
	   send_to_char(buf,ch);
	}

	sprintf(buf, "]\n\r");
	send_to_char(buf,ch);

      show_list_to_char( victim->in_room->contents, ch, FALSE, FALSE );
      show_char_to_char( victim->in_room->people,  ch );

    return;
}

/*
 *  Applies a temporary effect to their speech.
 */
void spell_garble( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_IMMORTAL(victim))
    {
	send_to_char("Yeah right!",ch);
	return;
    }

    if (victim == ch)
    {
	send_to_char("Garbling yourself would be foolishness.\n\r",ch);
	return;
    }

    if (victim->fighting == NULL && !IS_NPC(victim) )
	spell_yell(ch, victim, NULL);
    
    if ( IS_AFFECTED2(victim, AFF_SPEECH_IMPAIR ) )
    {
        act( "$S speech is already impaired.\n\r",ch,NULL,victim,TO_CHAR);
        return;
    }
    
    if( saves_spell( level, victim, DAM_MENTAL ) )
        return;
    
    af.where	= TO_AFFECTS2;
    af.type      	= sn;
    af.level     	= level;
    af.duration  	= number_range(3,7);
    af.location  	= APPLY_SPEECH_IMPAIR;
    af.modifier  	= MOD_SPEECH_GARBLE;
    af.bitvector 	= AFF_SPEECH_IMPAIR;
    affect_to_char( victim, &af );
    send_to_char( "Your tongue suddenly feels very strange.\n\r", victim );
    act("$N loses control of $S tongue.",ch, NULL, victim, TO_CHAR);
    return;
}

void spell_create_furnace( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
	OBJ_DATA *obj;
	bool is_here = FALSE;

	for(obj = ch->in_room->contents; obj != NULL; obj = obj->next_content)
	{
		if( obj->item_type == ITEM_FURNACE )
			is_here = TRUE;
	}

	if(is_here)
	{
		send_to_char("There is already a furnace in the room.\n\r",ch);
		return;
	}

	obj = create_object( get_obj_index(OBJ_VNUM_FURNACE), ch->level );
	obj_to_room(obj, ch->in_room);
	obj->timer = ch->level;
	act("$n strikes a flint, and ignites a furnace, so that it's fires are blazing.",ch, NULL, NULL, TO_ROOM);
	act("You strike a flint, and ignite a furnace, so that it's fires are blazing.",ch, NULL, NULL, TO_CHAR);
	return;
}

void spell_create_barrel( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
	OBJ_DATA *obj;
	bool is_here = FALSE;

	for(obj = ch->in_room->contents; obj != NULL; obj = obj->next_content)
	{
		if( obj->item_type == ITEM_BARREL )
			is_here = TRUE;
	}

	if(is_here)
	{
		send_to_char("There is already a cooling barrel in the room.\n\r",ch);
		return;
	}

	obj = create_object( get_obj_index(OBJ_VNUM_BARREL), ch->level );
	obj_to_room(obj, ch->in_room);
	obj->timer = ch->level;
	act("$n puts together some wood to make a cooling barrel, and fills it with water.",ch, NULL, NULL, TO_ROOM);
	act("You put together some wood to make a cooling barrel, and fill it with water.",ch, NULL, NULL, TO_CHAR);
	return;
}

void spell_create_grinder( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
	OBJ_DATA *obj;
	bool is_here = FALSE;

	for(obj = ch->in_room->contents; obj != NULL; obj = obj->next_content)
	{
		if( obj->item_type == ITEM_GRINDER )
			is_here = TRUE;
	}

	if(is_here)
	{
		send_to_char("There is already a grinder in the room.\n\r",ch);
		return;
	}

	obj = create_object( get_obj_index(OBJ_VNUM_GRINDER), ch->level );
	obj_to_room( obj, ch->in_room );
	obj->timer = ch->level;
	act("$n rounds off some stones to form a rudimentary grinder.",ch, NULL, NULL, TO_ROOM);
	act("You round off some stones to form a rudimentary grinder.",ch, NULL, NULL, TO_CHAR);
	return;
}

void spell_create_anvil( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
	OBJ_DATA *obj;
	bool is_here = FALSE;

	for(obj = ch->in_room->contents; obj != NULL; obj = obj->next_content)
	{
		if( obj->item_type == ITEM_ANVIL )
			is_here = TRUE;
	}

	if(is_here)
	{
		send_to_char("There is already an anvil in the room.\n\r",ch);
		return;
	}

	obj = create_object( get_obj_index(OBJ_VNUM_ANVIL), ch->level );
	obj_to_room( obj, ch->in_room );
	obj->timer = ch->level;
	act("$n creates a large, black, heavy-looking anvil.",ch, NULL, NULL, TO_ROOM);
	act("You create a large, black, heavy-looking anvil.",ch, NULL, NULL, TO_CHAR);
	return;
}

void spell_pine_needles( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  AFFECT_DATA af;

  if ( ch->in_room->sector_type != SECT_FOREST )
  {
    send_to_char( "You can only cast this spell in a forest.\n\r", ch );
    return;
  }

  if (is_affected(ch, skill_lookup("phantom armor")))
  {
    send_to_char("This will not work on you when already protected by Phantom Armor.\n\r",ch);
    return;
  }

  if ( IS_AFFECTED3( ch, AFF_PINENEEDLES ))
  {
    send_to_char( "Pine needles already swirl about you.\n\r", ch );
    return;
  }

  af.where		= TO_AFFECTS3;
  af.type		= sn;
  af.level		= level;
  af.duration	= 24;
  af.modifier	= -(level / 5 + 1);
  af.location	= APPLY_AC;
  af.bitvector	= AFF_PINENEEDLES;
  affect_to_char( ch, &af );

  send_to_char( "A swirl of pine needles surround you.\n\r", ch );
  act( "A swirl of pine needles surround $n.", ch, NULL, NULL, TO_ROOM );
  return;
}

void spell_create_flame( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
	OBJ_DATA *obj;
	bool is_here = FALSE;

	for(obj = ch->in_room->contents; obj != NULL; obj = obj->next_content)
	{
		if( !str_cmp(obj->material, "magical fire") )
			is_here = TRUE;
	}

	if(is_here)
	{
		send_to_char("There is already magical fire in the room.\n\r",ch);
		return;
	}

	obj = create_object( get_obj_index(OBJ_VNUM_MAGICAL_FIRE), ch->level );
	obj_to_room(obj, ch->in_room);
	obj->timer = ch->level;
	act("$n waves $s hand, and $p appears in the room.",ch, obj, NULL, TO_ROOM);
	act("You wave your hand, and $p appears in the room.",ch, obj, NULL, TO_CHAR);
	return;
}

void spell_thors_hammer( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
	CHAR_DATA *victim;
	int dam;		 //Quick defensive damage spell (prevent gankage of smith)
				 //Also useful against the aggro/lowpower mob...

	victim = ch->fighting;
	if(victim == NULL
	|| victim == ch 
	|| ch->fighting != victim
	|| ch->fighting == NULL)
	{
		send_to_char("But you are not fighting anyone.\n\r",ch);
		return;
	}

	dam = number_fuzzy(ch->level);
	if( saves_spell(level,victim,DAM_ENERGY) )
		dam = dam*3/4;

	damage(ch, victim, dam, sn, DAM_ENERGY, TRUE);
	act("$n creates a gigantic hammer which swings down upon $N!",ch,NULL,victim,TO_NOTVICT);
	return;
}

void spell_phantom_armor( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( ch, sn ) )
    {
        if (victim == ch)
          send_to_char("Your already wearing an illusionary suit of plate mail.\n\r",ch); 
        else
          act("$N already has an illusionary plate mail.",ch,NULL,victim,TO_CHAR);
        return;
    }

    if ( IS_AFFECTED(victim, AFF_SANCTUARY )
    ||   IS_AFFECTED3(victim, AFF_PINENEEDLES)
    ||   is_affected(victim, skill_lookup("shield"))
    ||   is_affected(victim, skill_lookup("armor")) )
    {
      send_to_char("Your illusion will not work while under the protection of other magic.\n\r",ch);
      return;
    }

    af.type      = sn;
    af.level     = level;
    af.duration  = level/2;
    af.location  = APPLY_AC;
    af.modifier  = -150;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    act( "A semi-illusory suit of armor covers $n.", victim, NULL, NULL, TO_ROOM );
    send_to_char( "A semi-illusory suit of armor covers your entire body.\n\r", victim );
    return;
}

void spell_wrath_of_nature( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	if( victim == NULL )
	{
		send_to_char("Cast this spell on whom?\n\r",ch);
		return;
	}

	dam = dice( level+10, 10 );

	switch(ch->in_room->sector_type)
	{
		case SECT_FIELD:
			act("The ground underneath $n rolls fiercly, and throws $m around.",victim, NULL, NULL, TO_ROOM);
			break;
		case SECT_FOREST:
			act("The forest roars with anger and strikes out at $n.",victim, NULL, NULL, TO_ROOM);
			break;
		case SECT_HILLS:
			act("The hills suddenly form a steep incline under $n, then shrink, hurling $m to the ground.",victim, NULL, NULL, TO_ROOM);
			break;
		case SECT_MOUNTAIN:
			act("A landslide rumbles down the mountain towards $n, hitting $m with rocks.",victim, NULL, NULL, TO_ROOM);
			break;
		case SECT_WATER_SWIM:
		case SECT_WATER_NOSWIM:
		case SECT_UNDERWATER:
			act("The water engulfs $n in a swirling whirlpool, consuming $m!",victim, NULL, NULL, TO_ROOM);
			break;
		case SECT_AIR:
			act("A whirlwind kicks up and consumes $n with it's rage!",victim, NULL, NULL, TO_ROOM);
			break;
		case SECT_GRAVEYARD:
			act("Dead tendrils and branches reach out towards $n, lashing across $m body.",victim, NULL, NULL, TO_ROOM);
			break;
		case SECT_SWAMP:
			act("The bog opens up and swallows $n, then spits $m out.",victim, NULL, NULL, TO_ROOM);
			break;
		case SECT_DESERT:
			act("A searing wind blasts $n with sand, pelting $m hard!",victim, NULL, NULL, TO_ROOM);
			break;
		default:
			act("The ground opens up, and several green vines with thorns emerge, slashing at $n.",victim, NULL, NULL, TO_ROOM);
			break;
	}

	damage(ch, victim, dam*2, sn, DAM_OTHER, TRUE);
	return;
}

void spell_firetongue(int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    int dam;

    if( victim == NULL )
    {
	send_to_char("Cast the spell on whom?\n\r",ch);
	return;
    }

    if (ch == victim)
    {
      send_to_char( "Suicide is a mortal sin.\n\r", ch );
      return;
    }

    send_to_char( "At the end of the incantation, you open your mouth, spewing forth a tongue of fire!\n\r", ch );
    act( "$n opens $s mouth, spewing forth a tongue of fire!",ch,NULL,NULL,TO_ROOM );
    dam = dice(45 + 20, 20);
    damage( ch, victim, dam, sn, DAM_FIRE,TRUE );

    if ( number_percent() > 50
    && !IS_AFFECTED3 (victim, AFF_FIRE)
    && !IS_NPC (victim))
    {
      af.where     	= TO_AFFECTS3;
      af.type      	= sn;
      af.level     	= level;
      af.duration  	= level;
      af.location  	= 0;
      af.modifier  	= 0;
      af.bitvector 	= AFF_FIRE;
      affect_join( victim, &af );
      act("$N becomes engulfed in a blanket of flames, turning $M into a living torch.",ch,NULL,victim,TO_CHAR);
	act("$N becomes engulfed in a blanket of flames, turning $M into a living torch.",ch,NULL,victim,TO_NOTVICT);
      act("The flames engulf your entire body, turning you into a living torch. Ouch!",ch, NULL, victim, TO_VICT);
    }

    return;
}

void spell_eagle_eye( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{

  AFFECT_DATA af;
  CHAR_DATA *victim;
  char buf[MAX_STRING_LENGTH];
  bool found;
  DESCRIPTOR_DATA *d;

  if ( is_affected(ch, sn))
  {
    send_to_char("You don't have the energy to call for another Eagle to scout around.\n\r", ch);
    return;
  }

  if (ch->fighting != NULL)
  {
    send_to_char("Your too busy fighting to try to call for an Eagle right now.\n\r", ch);
    return;
  }

  if (IS_SET(ch->in_room->room_flags,ROOM_INDOORS))
  {
    send_to_char("You need to be outdoors in order to cast this spell!\n\r",ch);
    return;
  }

  found = FALSE;

  af.where		      = TO_AFFECTS;
  af.type               = sn;
  af.level              = ch->level; 
  af.duration           = 9;
  af.bitvector          = 0;
  af.modifier           = 0;
  af.location           = APPLY_NONE;
  affect_to_char(ch, &af);

    send_to_char( "You transmit mental energy for an Eagle nearby to scout the area.\n\r", ch );
    send_to_char( "\n\r", ch );
    act("$n looks skywards, in hopes of contacting an Eagle to scout the area for $m.", ch, NULL, NULL, TO_ROOM);
    send_to_char( "Through telepathy, the Eagle reports that the following people are nearby:\n\r", ch );
    send_to_char( "\n\r", ch );

  for ( d = descriptor_list; d; d = d->next )
  {
    if ( d->connected == CON_PLAYING
    && ( victim = d->character ) != NULL
    &&   !IS_NPC(victim)
    &&   victim != ch
    &&   victim->in_room != NULL
    &&   !IS_SET(victim->in_room->room_flags,ROOM_NOWHERE)
    &&   !IS_SET(victim->in_room->room_flags,ROOM_INDOORS)
    &&   !IS_SET(ch->in_room->room_flags,ROOM_INDOORS)
    &&   (!room_is_private(victim->in_room))
    &&   victim->in_room->area == ch->in_room->area
    &&   can_see( ch, victim ) )
    {
	found = TRUE;
	sprintf( buf, "%-28s %s\n\r",
      victim->name, victim->in_room->name );
	send_to_char( buf, ch );
    }
  }
  if ( !found )
  send_to_char("None.\n\r", ch);
  return;
}

void spell_call_of_the_wild( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  AFFECT_DATA af;

  if (ch->fighting != NULL)
  {
    send_to_char( "Your too busy fighting to call on the wild!.\n\r", ch );
    return;
  }

  if ((ch->in_room->sector_type != SECT_FOREST)
  & ( ch->in_room->sector_type != SECT_FIELD)
  & ( ch->in_room->sector_type != SECT_DESERT)
  & ( ch->in_room->sector_type != SECT_HILLS)
  & ( ch->in_room->sector_type != SECT_POLAR)
  & ( ch->in_room->sector_type != SECT_SWAMP)
  & ( ch->in_room->sector_type != SECT_MOUNTAIN))
  {
    send_to_char( "There aren't enough wild animals in this area to heed your call.\n\r", ch );
    return;
  }

  send_to_char( "You disperse a mental energy, making every wild creature in the immediate vicinity\n\r", ch );
  send_to_char( "to go wild, screaming in an aweful and deafening shriek!\n\r", ch );
  act( "$n makes all the wild creatures around scream in an aweful and deafening shriek.", ch, NULL, NULL, TO_ROOM );

  for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
  {
    vch_next = vch->next_in_room; 
    
    if (!IS_NPC(ch) 
    && vch != ch 
    && ch->fighting != vch
    && vch->fighting != ch
    && !IS_NPC(vch))
    {

      if (IS_IMMORTAL(vch)
      || vch == ch
      || is_same_group(vch,ch))
        continue;

      if ( vch->in_room == NULL )
        continue;

      if (IS_AFFECTED2(vch, AFF_DEAFNESS) || saves_spell( level, vch, DAM_SOUND ) )
	  continue;

      if (!IS_AFFECTED2(vch, AFF_DEAFNESS))
      {
        af.where		= TO_AFFECTS2;
        af.type      	= sn;
        af.level     	= ch->level;
	  af.duration  	= 4;
	  af.location  	= 0;
        af.modifier  	= 0;
	  af.bitvector 	= AFF_DEAFNESS;
	  affect_to_char( vch, &af );
        send_to_char("The frantic screaming of wild animals becomes too much to bear, leaving your hearing\n\r", vch);
        send_to_char("with a constant ring.\n\r", vch);
	  act("$n reels back and holds $s ears.",vch,NULL,NULL,TO_ROOM);
        continue;
      }
    }
  }
  return;
}

void spell_endure_disease( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (is_affected(victim, skill_lookup("endure disease")))
  {
    if (victim == ch)
      send_to_char("Your ability to endure diseases more effectively has already been heightened.\n\r",ch);
    else
      act("$N is already able to endure diseases more effectively.",ch,NULL,victim,TO_CHAR);
      return;
  }

  af.type      	= skill_lookup("endure disease");
  af.level     	= level;
  af.duration  	= level/4;
  af.modifier  	= 0;
  af.location  	= APPLY_NONE;
  af.bitvector 	= RES_DISEASE;
  affect_to_char( victim, &af );

  af.modifier  	= 0;
  af.location  	= APPLY_NONE;
  af.bitvector 	= RES_POISON;
  affect_to_char( victim, &af );

  send_to_char( "A misty green fog surrounds you, then enters your body through your mouth.\n\r",victim);
  act("A misty green fog surrounds $N, then enters $S body through $S mouth.", ch,NULL,victim,TO_NOTVICT);
  return;
}

void spell_lock( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int door;

    target_name = one_argument(target_name, arg);

    if ( arg[0] == '\0' )
    {
	send_to_char( "Lock what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_here( ch, NULL, arg ) ) != NULL )
    {

	/* 'lock object' */
	if ( obj->item_type != ITEM_CONTAINER )
	{
	  send_to_char( "That's not a container.\n\r", ch );
	  return;
	}

	if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	{
          send_to_char( "It's not closed.\n\r", ch );
          return;
        }

	if ( obj->value[2] < 0 )
	{ 
          send_to_char( "It can't be locked.\n\r", ch );
          return;
        }

	if ( IS_SET(obj->value[1], CONT_LOCKED) )
	{
          send_to_char( "It's already locked.\n\r", ch );
          return;
        }

	SET_BIT(obj->value[1], CONT_LOCKED);
	act("You lock $p.",ch,obj,NULL,TO_CHAR);
	act( "$n's magic spell locks $p.", ch, obj, NULL, TO_ROOM );
	return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
      /* 'lock door' */
      ROOM_INDEX_DATA *to_room;
      EXIT_DATA *pexit;
      EXIT_DATA *pexit_rev;

      pexit = ch->in_room->exit[door];

      if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
      {
        send_to_char( "It's not closed.\n\r", ch );
        return;
      }

      if ( pexit->key < 0 )
      {
        send_to_char( "It can't be locked.\n\r", ch );
        return;
      }

      if ( IS_SET(pexit->exit_info, EX_LOCKED) )
      {
        send_to_char( "It's already locked.\n\r",ch );
        return;
      }

      SET_BIT(pexit->exit_info, EX_LOCKED);
      send_to_char( "*Click*\n\r", ch );
      act( "$n's magic spell locks the $d.", ch, NULL, pexit->keyword, TO_ROOM );

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

void spell_quicksand( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    int sect;

    if (IS_AFFECTED(ch,AFF_BLIND))
    {
	send_to_char("Cast the spell on whom? Your blind as a bat.\n\r",ch);
	return;
    }

    sect = ch->in_room->sector_type;

    if (sect != SECT_DESERT)
    {
	send_to_char("You can only cast this spell while in a desert.\n\r",ch);
	return; 
    }

    if (victim == ch)
    {
	send_to_char("You can not cast this spell on yourself!\n\r",ch);
      return;
    }
 
    if ( IS_AFFECTED3(victim,AFF_QUICKSAND))
    {
      act("$N is already buried in quicksand.",ch,NULL,victim,TO_CHAR);
      return;
    }

    if ( IS_AFFECTED(victim, AFF_FLYING) )
    {
	send_to_char("You can not cast this spell on someone who is flying!\n\r",ch);
      return;
    }
    
    if (IS_SET(victim->imm_flags,IMM_MAGIC))
    {
	if (victim != ch)
        send_to_char("Nothing seemed to happen.\n\r",ch);
      send_to_char("You feel momentarily lethargic.\n\r",victim);
      return;
    }
 
    if (IS_AFFECTED(victim,AFF_HASTE))
    {
      if (!check_dispel(level,victim,skill_lookup("haste")))
      {
	  if (victim != ch)
          send_to_char("Spell failed.\n\r",ch);
        send_to_char("The ground below your feet momentarily shifts.\n\r",victim);
        return;
      }

      act("The ground below $n's feet shifts momentarily.",victim,NULL,NULL,TO_ROOM);
      return;
    }

    send_to_char("You raise your arm in the air and begin to make swirling motions while\n\rpointing at the ground below your vitim's feet with your other arm!\n\r\n\r",ch);

    af.where     = TO_AFFECTS3;
    af.type      = sn;
    af.level     = level;
    af.duration  = level/5;
    af.location  = APPLY_DEX;
    af.modifier  = -1 - (level >= 18) - (level >= 25) - (level >= 32);
    af.bitvector = AFF_QUICKSAND;
    affect_to_char( victim, &af );

    act("$n raises $s arms in the air and begins to make a swirling motion.",ch,NULL,victim,TO_VICT);
    send_to_char( "Within moments the ground below your feet turns to quicksand, sucking you down\n\runtil your legs are completely buried!\n\r", victim );
    act("The ground below $n's feet turns to quicksand, sucking $m down until his\n\rlegs are completely buried.",victim,NULL,NULL,TO_ROOM);
    return;
}

void spell_battletide( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  AFFECT_DATA af;
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;

  if (IS_AFFECTED(ch,AFF_BLIND))
  {
    send_to_char("Your blind and can not see your target!\n\r",ch);
    return;
  }

  for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
  {
    vch_next        = vch->next_in_room;

    if (IS_IMMORTAL(vch) || vch == ch || is_same_group(vch,ch))
      continue;

    if (saves_spell(level,vch,DAM_OTHER) 
    ||  IS_SET(vch->imm_flags,IMM_MAGIC))
    {
      send_to_char("Nothing seemed to happen.\n\r",ch);
      send_to_char("You feel momentarily lethargic.\n\r",vch);
      return;
    }
     
    if (!is_safe_spell(ch,vch,TRUE) )
    {
      if ( vch->in_room == NULL )
        continue;

      if ( is_affected( vch, sn ) || IS_AFFECTED(vch,AFF_SLOW))
      {
        act("$N can't get any slower.", ch, NULL, vch, TO_CHAR);
      }
      else
	{
        if (IS_AFFECTED(vch,AFF_HASTE))
        {
          if (!check_dispel(level,vch,skill_lookup("haste")))
          {
            send_to_char("Spell failed.\n\r",ch);
            send_to_char("You feel momentarily slower.\n\r",vch);
            return;
          }
          act("$n is moving less quickly.",vch,NULL,NULL,TO_ROOM);
        }      
        af.where     = TO_AFFECTS;
        af.type      = sn;
        af.level     = level;
        af.duration  = level/2;
        af.location  = APPLY_DEX;
        af.modifier  = -2 - (level >= 18) - (level >= 25) - (level >= 32);
        af.bitvector = AFF_SLOW;
        affect_to_char( vch, &af );
        send_to_char( "You feel yourself slowing d o w n...\n\r", vch );
        act("$n starts to move in slow motion.",vch,NULL,NULL,TO_ROOM);
	}

      if ( vch->in_room == NULL )
        continue;
    }
  }
  return;
}

/*
void spell_battlefate( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  return;
}
*/

void spell_blessed_watchfulness( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  CHAR_DATA  *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ((victim != ch)
  &  !is_same_group(ch, victim))
  {
    send_to_char("You can only cast this spell on yourself or your fellow group members.\n\r",ch);
    return;
  }

  if ( is_affected( victim, sn ) )
  {
    if (victim == ch)
	send_to_char("Your already as watchful as can be!\n\r",ch);
    else
	act("Your clerical magic is already aiding $N in maintaining supreme alertness!",ch,NULL,victim,TO_CHAR);
    return;
  }

    af.where	= TO_AFFECTS3;
    af.type      	= sn;
    af.level     	= level;
    af.duration  	= 10;
    af.location  	= APPLY_NONE;
    af.modifier  	= 0;
    af.bitvector 	= AFF_WATCHFUL;
    affect_to_char( victim, &af );

    af.duration  	= 10;
    af.location  	= APPLY_WIS;
    af.modifier  	= 2;
    af.bitvector 	= 0;
    affect_to_char( victim, &af );

    send_to_char( "Magic courses through your body and mind, making you more alert to your surroundings!\n\r", victim );
    act( "$n appears to be more alert to $s surroundings!", victim, NULL, NULL, TO_ROOM );

    return;
}

void spell_blessed_warmth( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  CHAR_DATA  *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ((victim != ch)
  &  !is_same_group(ch, victim))
  {
    send_to_char("You can only cast this spell on yourself or your fellow group members.\n\r",ch);
    return;
  }

  if ( is_affected( victim, sn ) )
  {
    if (victim == ch)
	send_to_char("You are already immune to {bCold{x attacks.\n\r",ch);
    else
	act("Your clerical magic is already protecting $N against Cold attacks.",ch,NULL,victim,TO_CHAR);
    return;
  }

    af.where	= TO_IMMUNE;
    af.type      	= sn;
    af.level     	= level;
    af.duration  	= level / 8;
    af.location  	= APPLY_NONE;
    af.modifier  	= 0;
    af.bitvector 	= IMM_COLD;
    affect_to_char( victim, &af );
    send_to_char( "A narrow shaft of holy light beams down directly upon you,\n\rprotecting you in its warmth\n\r", victim );
    act( "$n is protected by the warmth of a holy light.", victim, NULL, NULL, TO_ROOM );

    return;
}

void spell_black_cairn( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  char buf[MAX_INPUT_LENGTH];
  BUFFER *buffer;
  OBJ_DATA *obj;
  OBJ_DATA *in_obj;
  bool found;
  int number = 0, max_found;

  found = FALSE;
  number = 0;
  max_found = 20;

  buffer = new_buf();

  if (number_percent() > 80 )
  {
    send_to_char("The Gods failed to honor your request!\n\r",ch);
    return;
  }
 
  for ( obj = object_list; obj != NULL; obj = obj->next )
  {
    if ((obj->pIndexData->vnum != OBJ_VNUM_CORPSE_NPC) 
    &   (obj->pIndexData->vnum != OBJ_VNUM_CORPSE_PC ))
      continue;
 
    found = TRUE;
    number++;
 
    for ( in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj );

      if (in_obj->in_room != NULL && can_see_room(ch,in_obj->in_room))
        sprintf( buf, "%s is in %s over in %s\n\r",
        obj->short_descr,
        in_obj->in_room->name,
	  in_obj->in_room->area->name);
 
      buf[0] = UPPER(buf[0]);
      add_buf(buffer,buf);
 
      if (number >= max_found)
        break;
  }
 
  if ( !found )
  {
    send_to_char( "The Gods inform you that there are no corpses to be found at this time!\n\r", ch );
    free_buf(buffer);
    return;
  }
  else
  {
    send_to_char( "\n\rThrough your magic, you ask the Gods to enlighten you as to the\n\r", ch );
    send_to_char( "location of corpses throughout the land.\n\r", ch );
    sprintf( buf, "\n\rVisions begin to take form, revealing to you the location of %s\n\r",
        number > 1 ? "several corpses:\n\r" : "a corpse:\n\r");
    send_to_char( buf, ch );
    page_to_char(buf_string(buffer),ch);

    free_buf(buffer);
  }
  return;
}


/*
void spell_blood_bond( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  CHAR_DATA  *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if(victim == ch)
  {
    send_to_char("You can't for a lasting blood bond with yourself!\n\r",ch);
    return;
  }

  if ( is_affected( victim, sn ) )
  {
    act("$N already has a lasting blood bond with someone!",ch,NULL,victim,TO_CHAR);
    return;
  }

    af.where	= TO_AFFECTS3;
    af.type      	= sn;
    af.level     	= level;
    af.duration  	= 10;
    af.location  	= APPLY_NONE;
    af.modifier  	= 0;
    af.bitvector 	= AFF_BLOOD_BOND;
    affect_to_char( victim, &af );

    af.where	= TO_AFFECTS3;
    af.type      	= sn;
    af.level     	= level;
    af.duration  	= 10;
    af.location  	= APPLY_NONE;
    af.modifier  	= 0;
    af.bitvector 	= AFF_BLOOD_BOND;
    affect_to_char( ch, &af );

    act( "$n forms a lasting blood bond with you!", ch, NULL, NULL, TO_VICT );
    act( "Your form a lasting blood bond with $N!", ch, NULL, victim, TO_CHAR );

  return;
}
*/

