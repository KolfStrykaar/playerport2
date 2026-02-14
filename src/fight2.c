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
bool is_safe		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void make_corpse	args( ( CHAR_DATA *ch ) );
void one_hit     	args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt, bool secondary ) );
void mob_hit		args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) );
void raw_kill		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void set_fighting	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void stop_fighting   	args( ( CHAR_DATA *ch, bool fBoth ) );
bool remove_obj		args( (CHAR_DATA *ch, int iWear, bool fReplace ) );
void multi_hit		args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) );


void do_tonguecut( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  int chance;

  if ( ch->riding )
  {
    send_to_char( "You can't get close enough while mounted.\n\r", ch );
    return;
  }

  one_argument(argument,arg);

  if ( arg[0] == '\0' )
  {
    send_to_char( "Cut who's tongue off?\n\r", ch );
     return;
  }

  if ( (chance = get_skill(ch,gsn_tonguecut)) == 0
  ||   (ch->level < skill_table[gsn_tonguecut].skill_level[ch->class]))
  {
    send_to_char("You aren't skilled enough to cut anyone's tongue off.\n\r",ch);
    return;
  }

  if ((victim = get_char_room(ch, NULL, arg)) == NULL)
  {
    send_to_char("They aren't here.\n\r",ch);
    return;
  }

  if ( ch->fighting != NULL )
  {
    send_to_char( "Your too busy fighting.\n\r", ch );
    return;
  }

  if (victim == ch)
  {
    send_to_char("Cut your own tongue off? Were you an abused child growing up?\n\r",ch);
    return;
  }

  if (IS_AWAKE(victim))
  {
    send_to_char( "They need to be sleeping or knocked out to cut their tongue off.\n\r", ch );
    return;
  }

  if (is_affected(victim,gsn_tonguecut))
  {
    act("$S tongue has already been cut off.",ch,NULL,victim,TO_CHAR);
    return;
  }

  if ( IS_AFFECTED2(ch, AFF_PARALYSIS) 
  ||   IS_AFFECTED3(ch, AFF_HOLDPERSON))
  {
    send_to_char("You can't move at all, there's no way you can do that.\n\r",ch);
    return;
  }

  if (is_safe(ch,victim))
  {
    send_to_char("Nope.\n\r",ch);
    return;
  }

  if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
  {
    act("But $N is such a good friend!",ch,NULL,victim,TO_CHAR);
    return;
  }

  if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL)
  {
    send_to_char( "You need to wield a weapon to cut off someone's tongue.\n\r", ch );
    return;
  }

  if (obj->value[0] != WEAPON_DAGGER )
  {
    send_to_char ("You can only cut their tongue off with a dagger.\n\r", ch );
    return;
  }

  if (number_percent( ) < (ch->pcdata->learned[gsn_tonguecut]) 
  && (victim->level <= (ch->level+3)))
  {
    AFFECT_DATA af;
    act("$n's tongue just got lopped off!",victim,NULL,NULL,TO_ROOM);
    send_to_char("Blood quickly fills your mouth, as your tongue has just been cut off!\n\r",victim);

    check_improve(ch,gsn_tonguecut,TRUE,2);

    af.where		= TO_AFFECTS;
    af.type 		= gsn_tonguecut;
    af.level 		= ch->level;
    af.duration		= number_fuzzy(ch->level / 8);
    af.location		= 0;
    af.modifier		= 0;
    af.bitvector 	= 0;
    affect_to_char(victim,&af);

    af.where		= TO_AFFECTS2;
    af.location  	= APPLY_SPEECH_IMPAIR;
    af.modifier  	= MOD_SPEECH_GARBLE;
    af.bitvector 	= AFF_SPEECH_IMPAIR;
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
    victim->position = POS_STANDING;
    send_to_char("You failed to cut their tongue off.\n\r",ch);
    act("$n tries to cut your tongue off but fails!",ch,NULL,victim,TO_VICT);
    act("$n tries to cut $N's tongue off, but fails.",ch,NULL,victim,TO_NOTVICT);
    if(!IS_NPC(ch))
    {     
      if (!can_see(victim,ch))
        do_yell(victim, "Help! Someone tried to cut my tongue off!");
      else
      {
        if (IS_NPC(ch))
          sprintf(buf, "Help! %s just tried to cut my tongue off!", ch->short_descr);
        else
          sprintf( buf, "Help! %s just tried to cut my tongue off!", 
          ((is_affected(ch,gsn_doppelganger)) && (!IS_IMMORTAL(victim)))? ch->doppel->name :
          ((is_affected(ch,gsn_treeform)) && (!IS_IMMORTAL(victim)))? "A mighty oak" : 
          ch->name);
        do_yell( victim, buf );
      }
    }
    multi_hit( ch, victim, TYPE_UNDEFINED );
    check_improve(ch,gsn_tonguecut,FALSE,2);
    return;
  }
}

void do_stalk( CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];
  int chance;
    
  if ( IS_NPC(ch) )
    return;

  argument = one_argument (argument, arg);

  if ( (chance = get_skill(ch,gsn_stalk)) == 0
  ||   (ch->level < skill_table[gsn_stalk].skill_level[ch->class]))
  {
    send_to_char("Huh?\n\r",ch);
    return;
  }

  if ( IS_AFFECTED2(ch, AFF_PARALYSIS) 
  ||   IS_AFFECTED3(ch, AFF_HOLDPERSON))
  {
    send_to_char("You can't move at all, there's no way you can do that.\n\r",ch);
    return;
  }

  if ( ch->riding )
  {
    send_to_char( "You can't stalk anyone while mounted.\n\r", ch );
    return;
  }

  if ( arg[0] == '\0' )
  {
    send_to_char( "Stalk whom?\n\r", ch );
    return;
  }

  if ( ( victim = get_char_world( ch, arg ) ) == NULL )
  {
    send_to_char( "You cannot find them.\n\r", ch );
    return;
  }
    
  if( ch->move < 400) 
  {
    send_to_char( "You don't have enough movement to stalk them.\n\r",ch);
    return;
  }

  if ( ch->fighting != NULL )
  {
    send_to_char( "Your too busy fighting.\n\r", ch );
    return;
  }

  if( IS_SET(victim->in_room->room_flags, ROOM_NOWHERE)
  ||   IS_SET(victim->in_room->room_flags, ROOM_SAFE)
  ||   IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
  ||   IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
  ||   IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
  ||   IS_SET(victim->in_room->room_flags, ROOM_IMP_ONLY)
  ||   IS_SET(victim->in_room->room_flags, ROOM_GODS_ONLY)
  ||   IS_SET(victim->in_room->room_flags, ROOM_HEROES_ONLY)
  ||   IS_SET(victim->in_room->room_flags, ROOM_NEWBIES_ONLY)
  ||   IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
  ||	 ( number_percent() > 50)
  ||   victim->level >= ch->level + 3
  ||   (!IS_NPC(victim) && victim->level >= LEVEL_HERO)
  ||   victim->in_room == NULL )
  { 
    send_to_char( "You can't find it's location.\n\r",ch);
    return; 
  }

  if (victim == ch)
  {
    send_to_char("But you're already at yourself!\n\r",ch);
    return;
  }

  if (victim->in_room == ch->in_room)
  {
    send_to_char("But you're already there!\n\r",ch);
    return;
  }

  check_improve(ch,gsn_stalk,TRUE,1);

  send_to_char("\n\r",ch);
  act("You pick up the trail of $N, and quickly find them.", ch, NULL, victim, TO_CHAR);
  send_to_char("\n\r",ch);
  act("$n disappears suddenly into the shadows.", ch, NULL, victim, TO_ROOM);
  char_from_room(ch);
  char_to_room(ch, victim->in_room);
  ch->move -= 400;
  do_look(ch,"auto");
  send_to_char("\n\r",ch);
  act("$n walks out of nowhere behind $N.", ch, NULL, victim, TO_NOTVICT);
  act("$n walks out of nowhere from behind.", ch, NULL, victim, TO_VICT);
  act("You walk out of nowhere up behind $N.", ch, NULL, victim, TO_CHAR);
  send_to_char("\n\r",ch);
  return;
}

void do_smother( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];
  int chance;


  if ( IS_NPC(ch) )
    return;

  one_argument( argument, arg );

  if ( (chance = get_skill(ch,gsn_smother)) == 0
  ||   (ch->level < skill_table[gsn_smother].skill_level[ch->class]))
  {
    send_to_char("You don't know the first thing on how to put out human torches.\n\r",ch);
    return;
  }

  if ( arg[0] == '\0')
  {
    send_to_char( "Smother whom?\n\r", ch );
    return;
  }

  if ( ch->fighting != NULL )
  {
    send_to_char( "Your too busy fighting.\n\r", ch );
    return;
  }
    
  if ( ( victim = get_char_room( ch, NULL, arg ) ) == NULL )
  {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  if ( victim == ch )
  {
    send_to_char( "You try rolling on the ground, but can't seem to put out the flames.\n\r", ch );
    return;
  }

  if (!IS_AFFECTED3(victim, AFF_FIRE))
  {
    send_to_char( "But they aren't on fire!\n\r", ch );
    return;
  }

  if ( number_percent() < 15) 
  {
    act("You try to smother the flames around $N but fail!", ch,NULL,victim,TO_CHAR);
    act("$n tries to smother the flames around you but fails!", ch,NULL,victim,TO_VICT);
    act("$n tries to smother the flames around $N but fails!", ch,NULL,victim,TO_NOTVICT);
    check_improve(ch,gsn_smother,FALSE,1);
    return;
  }
  else
  if ( number_percent() > 96 && !IS_AFFECTED3(ch,AFF_FIRE) )
  {
    act("A spark of flame from $N's body sets you on fire!", ch,NULL,victim,TO_CHAR);
    act("A spark of flame from your body sets $n on fire!", ch,NULL,victim,TO_VICT);
    act("A spark of flame from $N's body sets $n on fire!", ch,NULL,victim,TO_NOTVICT);
    SET_BIT(ch->affected3_by, AFF_FIRE);
    return;
  }
  else
    act( "You manage to smother the flames around $M!", ch,NULL,victim,TO_CHAR);
    act( "$n manages to smother the flames around you!", ch,NULL,victim,TO_VICT);
    act( "$n manages to smother the flames around $N!", ch,NULL,victim,TO_NOTVICT);
    REMOVE_BIT(victim->affected3_by, AFF_FIRE);
    check_improve(ch,gsn_smother,TRUE,1);
    return;
}

void do_tame( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    int chance;
    CHAR_DATA *victim;
    CHAR_DATA *gch;
    AFFECT_DATA af;
    int charm_count = 0;

    one_argument( argument, arg );

    if ( (chance = get_skill(ch,gsn_tame)) == 0
    ||   (ch->level < skill_table[gsn_tame].skill_level[ch->class]))
    {
      send_to_char("You don't know the first thing about taming a creature.\n\r",ch);
      return;
    }

    if ( IS_AFFECTED2(ch, AFF_PARALYSIS) 
    ||   IS_AFFECTED3(ch, AFF_HOLDPERSON))
    {
      send_to_char("You can't move at all, there's no way you can do that.\n\r",ch);
      return;
    }

    if ( arg[0] == '\0')
    {
      send_to_char( "Tame who or what?\n\r", ch );
      return;
    }

    if ( ch->fighting != NULL )
    {
      send_to_char( "Your too busy fighting.\n\r", ch );
      return;
    }

    if ( ( victim = get_char_room( ch, NULL, arg ) ) == NULL )
    {
      send_to_char( "They aren't here.\n\r", ch );
      return;
    }

    if (!IS_NPC(victim))
    {
     send_to_char( "Players can not be tamed.\n\r", ch );
     return;
    }

    if ( IS_AFFECTED2(victim, AFF_INVUL)
    ||   IS_AFFECTED3(victim, AFF_SUBDUE))
    {
     send_to_char( "$n is untamable to you.\n\r", ch );
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
      send_to_char( "You can't tame animals that aren't out in the wild and on land.\n\r", ch );
      return;
    }

    if ( IS_AFFECTED2(ch, AFF_INVUL)
    ||   IS_AFFECTED3(victim, AFF_SUBDUE))
    {
        send_to_char( "No way!\n\r", ch );
        return; 
    }

    if((victim->race == race_lookup("human"))
    || (victim->race == race_lookup("draconian"))
    || (victim->race == race_lookup("half-elf"))
    || (victim->race == race_lookup("silvanesti"))
    || (victim->race == race_lookup("qualinesti"))
    || (victim->race == race_lookup("dargonesti"))
    || (victim->race == race_lookup("dimernesti"))
    || (victim->race == race_lookup("neidar"))
    || (victim->race == race_lookup("hylar"))
    || (victim->race == race_lookup("theiwar"))
    || (victim->race == race_lookup("aghar"))
    || (victim->race == race_lookup("daergar"))
    || (victim->race == race_lookup("daewar"))
    || (victim->race == race_lookup("klar"))
    || (victim->race == race_lookup("gnome"))
    || (victim->race == race_lookup("kender"))
    || (victim->race == race_lookup("minotaur"))
    || (victim->race == race_lookup("ogre"))
    || (victim->race == race_lookup("elf"))
    || (victim->race == race_lookup("dwarf"))
    || (victim->race == race_lookup("giant"))
    || (victim->race == race_lookup("centipede"))
    || (victim->race == race_lookup("doll"))
    || (victim->race == race_lookup("goblin"))
    || (victim->race == race_lookup("hobgoblin"))
    || (victim->race == race_lookup("kobold"))
    || (victim->race == race_lookup("orc"))
    || (victim->race == race_lookup("troll")))
    {
	send_to_char( "\n\r", ch );
      act("You can only TAME animals! You only managed to upset $N!",ch,0,victim,TO_CHAR);
	send_to_char( "\n\r", ch );
        multi_hit( ch, victim, TYPE_UNDEFINED );
	return;
    }

    if (is_safe(ch,victim))
    {
	send_to_char( "You failed to tame them!\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "You can't tame yourself!\n\r", ch );
	return;
    }

    if ( victim->position == POS_SLEEPING )
    {
      send_to_char( "You can not seem to get your victim's attention.\n\r", ch );
      return;
    }

    if ( IS_AFFECTED(victim, AFF_CHARM)
    ||   IS_AFFECTED(ch, AFF_CHARM)
    ||   ch->level < victim->level
    ||   IS_SET(victim->imm_flags,IMM_CHARM)
    ||   saves_spell( ch->level, victim,DAM_CHARM) )
    {
	send_to_char( "You failed to tame them!\n\r", ch );
	return;
    }


    if (IS_SET(victim->in_room->room_flags,ROOM_LAW))
    {	send_to_char(
	    "The mayor does not allow taming in the city limits.\n\r",ch);
	return;
    }
  
    for(gch = char_list; gch != NULL; gch = gch->next)
    {
      if( IS_NPC(gch) && IS_AFFECTED(gch,AFF_CHARM) 
      && gch->master == ch )
	charm_count++;
    }

    if(charm_count >= 1) //Total 2 Charmies (1, 2)
    {
      send_to_char("Your influence can only stretch so far!\n\r",ch);
      return;
    }

    chance /= 2;
 
    chance -= get_curr_stat(victim,STAT_WIS);
    chance += get_curr_stat(ch,STAT_CHR);
    chance -= victim->level;
    chance += ch->level;
    chance = URANGE(5,chance,50);


    if (number_percent() > chance)
    {
      act("You try to tame $N, but fail.",ch,0,victim,TO_CHAR);
      act("$n tries to tame you, but fails.",ch,0,victim,TO_VICT);
      act("$n tries to tame $N, but fails.",ch,0,victim,TO_NOTVICT);
      check_improve(ch,gsn_tame,FALSE,1);
      WAIT_STATE(ch,10);
      return;
    }

    check_improve(ch,gsn_tame,TRUE,1);

    if ( victim->master )
	stop_follower( victim );
    add_follower( victim, ch );
    victim->leader 	= ch;
    af.where     	= TO_AFFECTS;
    af.type      	= gsn_tame;
    af.level	 	= ch->level;
    af.duration  	= number_fuzzy( ch->level / 4 );
    af.location  	= 0;
    af.modifier  	= 0;
    af.bitvector 	= AFF_CHARM;
    affect_to_char( victim, &af );

    act( "Isn't $n just so nice?", ch, NULL, victim, TO_VICT );

    if ( ch != victim )
      act("$N looks at you with adoring eyes.",ch,NULL,victim,TO_CHAR);

    if( !IS_NPC(victim) && IS_SET(victim->act, PLR_NOFOLLOW) )
      REMOVE_BIT(victim->act, PLR_NOFOLLOW);

    if (IS_NPC(victim))
    {
      SET_BIT(victim->act, ACT_PET);
      victim->comm = COMM_NOTELL|COMM_NOSHOUT|COMM_NOCHANNELS;
      ch->pet = victim;
    }

    if (IS_NPC(victim) && IS_SET(victim->act,ACT_STAY_AREA))
    {
      REMOVE_BIT(victim->act,ACT_STAY_AREA);
      SET_BIT(victim->act, ACT_SENTINEL);
    }
    return;
}

void do_joust( CHAR_DATA *ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  int dam;
  int chance;
  one_argument( argument, arg );

  if ( get_skill(ch,gsn_mounted_combat) == 0
  || (!IS_NPC(ch)
  && ch->level < skill_table[gsn_mounted_combat].skill_level[ch->class]))
  {	
    send_to_char("You need to be trained in Mounted Combat in order to execute this skill!\n\r",ch);
    return;
  }

  if (((chance = get_skill(ch,gsn_joust)) == 0)
  || (!IS_NPC(ch)
  && ch->level < skill_table[gsn_joust].skill_level[ch->class]))
  {	
    send_to_char("Joust? What's that?\n\r",ch);
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
    send_to_char("Joust who exactly? The air?\n\r",ch);
    return;
  }

  if ( ( victim = get_char_room( ch, NULL, arg ) ) == NULL )
  {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  if (!ch->riding)
  {
    send_to_char("You can't do that if your not mounted.\n\r",ch);
    return;
  }

  if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL)
  {
    send_to_char( "You need to wield a weapon to do any jousting.\n\r", ch );
    return;
  }

  if ((obj->value[0] != WEAPON_POLEARM )
  &&  (obj->value[0] != WEAPON_SPEAR ))
  {
    send_to_char ("You can only perform a jousting move using polearm or spear type weapons.\n\r",ch );
    return;
  }

  if (IS_NPC(ch))
  {
    return;
  }

  if (victim == ch)
  {
    send_to_char("Very funny. Planning to bend the shaft of your weapon so it points back at you?\n\r",ch);
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

  if (ch->position == POS_STUNNED)
  {
    send_to_char("You're still a little woozy.\n\r",ch);
    return;
  }

  if (ch->fighting != NULL )
  {
    send_to_char("You're too busy fighting.\n\r", ch);
    return;
  }

  if ( victim->fighting != NULL )
  {
    send_to_char( "You can't joust someone who is fighting, you might hit the person they are fighting.\n\r", ch );
    return;
  }

  /* modifiers */

  /* stats */
  chance += get_curr_stat(ch,STAT_STR);
  chance -= (get_curr_stat(victim,STAT_DEX) * 4)/3;
  chance -= GET_AC(victim,AC_BASH) /25;

  /* speed */
  if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,AFF_HASTE))
    chance -= 30;

  /* level */
  chance += (ch->level - victim->level);

  WAIT_STATE( ch, skill_table[gsn_joust].beats );

  if (number_percent() > get_skill(ch,gsn_mounted_combat))
  {	
    send_to_char("You kick your mount, making it lunge towards your victim, but alas,\n\r",ch);
    send_to_char("your mount makes an awkward move, like it was spooked, throwing you to the ground.\n\r",ch);
    act( "$n's mount moves awkwardly while in a full run, sending $n flying to the ground.",
    ch, NULL, NULL, TO_NOTVICT );
    ch->riding->rider = NULL;
    ch->riding    = NULL;
    ch->position  = POS_RESTING;
    return;
  }

  if ( number_percent( ) < chance
  || ( get_skill(ch,gsn_joust) >= 2 && !IS_AWAKE(victim) ) )
  {
    check_improve(ch,gsn_joust,TRUE,2);
    act( "$n kicks $s mount, making it lunge at you, allowing $m to joust you with $s weapon.", ch, NULL, victim, TO_VICT);
    act( "You kick your mount, making it lunge towards $N, allowing you to joust $M with your weapon.", ch, NULL, victim, TO_CHAR);
    act( "$n runs $s mount to $N, jousting $M with $s weapon.", ch, NULL, victim, TO_NOTVICT);
    dam = dice( (ch->level/2) + 25, 25 );
    damage( ch, victim, dam, gsn_joust,DAM_BASH,TRUE);
  }
  else
  {
    check_improve(ch,gsn_joust,FALSE,2);
    act( "$n lunges at you with $s mount and tries to joust you with $s weapon, but thankfully $e misses.", ch, NULL, victim, TO_VICT);
    act( "You try to joust $N with your weapon, but you miss.", ch, NULL, victim, TO_CHAR);
    act( "$n tries to joust $N with $s weapon, but alas $e misses.", ch, NULL, victim,  TO_NOTVICT);
    damage( ch, victim, 0, gsn_joust, DAM_BASH, TRUE);
  }

  if (ch->fighting == NULL)
    set_fighting(ch, victim);

  ch->fight_pos = FIGHT_FRONT;
  ch->riding->fight_pos = FIGHT_FRONT;

  return;
}

void do_inspire( CHAR_DATA *ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  AFFECT_DATA af;
  int members;

  if (IS_NPC(ch))
  return;

  argument = one_argument( argument, arg1 );

  members = 0;

  for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
  {
    vch_next = vch->next_in_room;

    if (!IS_AFFECTED(vch,AFF_CHARM) && is_same_group( vch, ch ) )
      members++;
  }

  if ( members <= 1 )
  {
    send_to_char("There's nobody around for you to inspire.\n\r",ch);
    return;
  }

  if ( arg1[0] == '\0' )
  {
    send_to_char( "Syntax: inspire <might|valor|honor>.\n\r", ch );
    return;
  }

  if ( !str_prefix( arg1, "might") )
  {
    if ((ch->level < skill_table[gsn_company_might].skill_level[ch->class])
    ||  (!IS_NPC(ch) && ch->pcdata->learned[gsn_company_might] < 1 ))
    {
      send_to_char( "Huh? You don't know the first thing about inspiration!\n\r",ch );
      return;
    }

    if ( number_percent( ) < ch->pcdata->learned[gsn_company_might])
      check_improve(ch,gsn_company_might,TRUE,1);
    else
    {
      check_improve(ch,gsn_company_might,FALSE,1);
      act("$n tries to increase morale, but fails.",
      ch,NULL,vch,TO_ROOM);
      act("You try to increase the morale of your friends, but fail.",
      ch,NULL,vch,TO_CHAR);
      return;
    }

    for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
    {
      vch_next = vch->next_in_room;
      if(!IS_NPC(vch))
      {
	  if (is_same_group(ch,vch)
        &&  IS_AFFECTED3(ch,AFF_MIGHT))
        {
          send_to_char("You can't inspire anyone else around you just yet.\n\r",ch);
          return;
        }

        if (is_same_group(ch,vch)
        && !IS_AFFECTED3(vch,AFF_MIGHT))
        {
	    af.where	= TO_AFFECTS3;
          af.type     	= gsn_company_might;
          af.level    	= ch->level;
          af.duration 	= ch->level/3;
          af.location 	= APPLY_HITROLL;
          af.modifier  	= 4;
          af.bitvector 	= AFF_MIGHT;
          affect_to_char(vch,&af);

          af.location   = APPLY_STR;
          af.modifier   = 2;
          af.bitvector  = 0;
          affect_to_char(vch,&af);
   
	    act("$n raises $s voice in an inspiring cry of honor.",ch,NULL,vch,TO_NOTVICT );
          if (vch == ch)
           send_to_char("You raise your voice into an inspiring cry of might.\n\r",ch);
          else
           send_to_char("Your morale is raised by an inspiration of might.\n\r",vch);
        } 
      }
    }
    return;
  }
  else
  if ( !str_prefix( arg1, "valor") )
  {
    if ((ch->level < skill_table[gsn_company_valor].skill_level[ch->class])
    ||  (!IS_NPC(ch) && ch->pcdata->learned[gsn_company_valor] < 1 ))
    {
      send_to_char( "Huh? You don't know the first thing about inspiration!\n\r",ch );
      return;
    }

    if ( number_percent( ) < ch->pcdata->learned[gsn_company_valor])
      check_improve(ch,gsn_company_valor,TRUE,1);
    else
    {
      check_improve(ch,gsn_company_valor,FALSE,1);
      act("$n tries to increase morale, but fails.",
      ch,NULL,NULL,TO_ROOM);
      act("You try to increase the morale of your friends, but fail.",
      ch,NULL,NULL,TO_CHAR);
      return;
    }

    for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
    {
      vch_next = vch->next_in_room;
      if(!IS_NPC(vch))
      {
	  if (is_same_group(ch,vch)
        &&  IS_AFFECTED3(ch,AFF_VALOR))
        {
          send_to_char("You can't inspire anyone else around you just yet.\n\r",ch);
          return;
        }

        if (is_same_group(ch,vch)
        && !IS_AFFECTED3(vch,AFF_VALOR))
        {
	    af.where	= TO_AFFECTS3;
          af.type     	= gsn_company_valor;
          af.level    	= ch->level;
          af.duration 	= ch->level/3;
          af.location 	= APPLY_DAMROLL;
          af.modifier  	= 2;
          af.bitvector 	= AFF_VALOR;
          affect_to_char(vch,&af);

          af.location   = APPLY_DEX;
          af.modifier   = 2;
          af.bitvector  = 0;
          affect_to_char(vch,&af);
   
	    act("$n raises $s voice in an inspiring cry of honor.",ch,NULL,vch,TO_NOTVICT );
          if (vch == ch)
           send_to_char("You raise your voice into an inspiring cry of valor.\n\r",ch);
          else
           send_to_char("Your morale is raised by an inspiration of valor.\n\r",vch);
        } 
      }
    }
    return;
  }
  else
  if ( !str_prefix( arg1, "honor") )
  {
    if ((ch->level < skill_table[gsn_company_honor].skill_level[ch->class])
    ||  (!IS_NPC(ch) && ch->pcdata->learned[gsn_company_honor] < 1 ))
    {
      send_to_char( "Huh? You don't know the first thing about inspiration!\n\r",ch );
      return;
    }

    if ( number_percent( ) < ch->pcdata->learned[gsn_company_honor])
      check_improve(ch,gsn_company_honor,TRUE,1);
    else
    {
      check_improve(ch,gsn_company_honor,FALSE,1);
      act("$n tries to increase morale, but fails.",
      ch,NULL,NULL,TO_ROOM);
      act("You try to increase the morale of your friends, but fail.",
      ch,NULL,NULL,TO_CHAR);
      return;
    }

    for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
    {
      vch_next = vch->next_in_room;
      if(!IS_NPC(vch))
      {
	  if (is_same_group(ch,vch)
        &&  IS_AFFECTED3(ch,AFF_HONOR))
        {
          send_to_char("You can't inspire anyone else around you just yet.\n\r",ch);
          return;
        }

        if (is_same_group(ch,vch)
        && !IS_AFFECTED3(vch,AFF_HONOR))
        {
	    af.where	= TO_AFFECTS3;
          af.type     	= gsn_company_honor;
          af.level    	= ch->level;
          af.duration 	= ch->level/3;
          af.location 	= APPLY_AC;
          af.modifier  	= -30;
          af.bitvector 	= AFF_HONOR;
          affect_to_char(vch,&af);

          af.location   = APPLY_INT;
          af.modifier   = 1;
          af.bitvector  = 0;
          affect_to_char(vch,&af);

          af.location   = APPLY_WIS;
          af.modifier   = 2;
          af.bitvector  = 0;
          affect_to_char(vch,&af);
   
	    act("$n raises $s voice in an inspiring cry of honor.",ch,NULL,vch,TO_NOTVICT );
          if (vch == ch)
           send_to_char("You raise your voice into an inspiring cry of honor.\n\r",ch);
          else
           send_to_char("Your morale is raised by an inspiration of honor.\n\r",vch);
        } 
      }
    }
    return;
  }
  else
  {
    send_to_char( "Syntax: inspire <might|valor|honor>.\n\r", ch );
    return;
  }
}

void do_dancing_strike( CHAR_DATA *ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int chance;
  one_argument( argument, arg );

  if (((chance = get_skill(ch,gsn_dancing_strike)) == 0)
  || (!IS_NPC(ch)
  && ch->level < skill_table[gsn_dancing_strike].skill_level[ch->class]))
  {	
    send_to_char("Dancing Strike? What's that?\n\r",ch);
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
    send_to_char( "You aren't fighting anyone.\n\r", ch );
    return;
  }

  if (ch->riding)
  {
    send_to_char("You can't deliver a dancing strike while mounted.\n\r",ch);
    return;
  }

  if (IS_NPC(ch))
  {
    return;
  }

  if (victim == ch)
  {
    send_to_char("You dance around the room like a ballerina, all the while kicking your own leg.\n\r",ch);
    act( "$n puts $s hand over $s head and begins to twirl like a ballerina, simultaniously kicking $mself in the leg.",
    ch, NULL, NULL, TO_NOTVICT );
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

  if (ch->position == POS_STUNNED)
  {
    send_to_char("You're still a little woozy.\n\r",ch);
    return;
  }

  /* modifiers */

  /* stats */
  chance += get_curr_stat(ch,STAT_STR);
  chance += get_curr_stat(ch,STAT_DEX);
  chance -= (get_curr_stat(victim,STAT_DEX) * 4)/3;
  chance -= GET_AC(victim,AC_BASH) /25;

  /* speed */
  if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,AFF_HASTE))
    chance -= 30;

  /* level */
  chance += (ch->level - victim->level);

  WAIT_STATE( ch, skill_table[gsn_dancing_strike].beats );

  if ( number_percent( ) < chance
  || ( get_skill(ch,gsn_dancing_strike) >= 2 && !IS_AWAKE(victim) ) )
  {
    check_improve(ch,gsn_dancing_strike,TRUE,2);

    switch (number_range(0,3))
    {
	default:
        act( "In a swift, fluid like motion, $n breaks into a small dance, twirling", ch, NULL, victim, TO_VICT);
        act( "around you as to avoid any counter and delivers a severe strike to your head!", ch, NULL, victim, TO_VICT);
        act( "In a swift, fluid like motion, you break into a small dance, twirling\n\r"
              "around $N to avoid any possible counter and deliver a severe\n\rstrike to $S head!", ch, NULL, victim, TO_CHAR);
        act( "$n breaks into a dance and delivers a severe strike to $N's head!", ch, NULL, victim, TO_NOTVICT);
        damage(ch,victim,number_range( 90,ch->level + 260), gsn_dancing_strike,DAM_BASH,TRUE);
        break;

	case 0:
        act( "In a swift, fluid like motion, $n breaks into a small dance, twirling", ch, NULL, victim, TO_VICT);
        act( "around you as to avoid any counter and delivers a severe strike to your head!", ch, NULL, victim, TO_VICT);
        act( "In a swift, fluid like motion, you break into a small dance, twirling\n\r"
              "around $N to avoid any possible counter and deliver a severe\n\rstrike to $S head!", ch, NULL, victim, TO_CHAR);
        act( "$n breaks into a dance and delivers a severe strike to $N's head!", ch, NULL, victim, TO_NOTVICT);
        damage(ch,victim,number_range( 90,ch->level + 260), gsn_dancing_strike,DAM_BASH,TRUE);
	  break;

	case 1:
        act( "$n breaks away from your assault for a brief moment and engages in a dance.", ch, NULL, victim, TO_VICT);
        act( "Mesmerized for a brief moment at the sight, $e quickly dances up to your side", ch, NULL, victim, TO_VICT);
        act( "and delivers a painful strike to the side of your body.", ch, NULL, victim, TO_VICT);
        act( "You break away from the onslaught for a brief moment and engage in a dance.", ch, NULL, victim, TO_CHAR);
        act( "Mesmerized for a bried moment at the sight of you dancing, you quickly dance", ch, NULL, victim, TO_CHAR);
        act( "up to $N and deliver a painful strike to the side of $S body!", ch, NULL, victim, TO_CHAR);
        act( "$n breaks into a dance and delivers a painful strike to the side of $N's body!", ch, NULL, victim, TO_NOTVICT);
        damage(ch,victim,number_range( 85,ch->level + 240), gsn_dancing_strike,DAM_BASH,TRUE);
	  break;

	case 2:
        act( "$n breaks into an eratic dance, as if you were in a bar listening to music,", ch, NULL, victim, TO_VICT);
        act( "but quickly uses his dancing moves to deliver crushing strike to your leg!", ch, NULL, victim, TO_VICT);
        act( "You break into an erotic dance, as if you were in a bar and music rang\n\r"
             "throughout the air. With $N distracted, you quickly deliver\n\r"
             "a crushing strike to $S leg!", ch, NULL, victim, TO_CHAR);
        act( "$n breaks into a dance and delivers a crushing strike to $N's leg!", ch, NULL, victim, TO_NOTVICT);
        damage(ch,victim,number_range( 80,ch->level + 220), gsn_dancing_strike,DAM_BASH,TRUE);
        break;
    }
  }
  else
  {
    check_improve(ch,gsn_dancing_strike,FALSE,2);

    act( "$n tries to break into a dance, but collapses to the ground in $s futile attempt.", ch, NULL, victim, TO_VICT);
    act( "You try to break into a dance to deliver a strike, but your feet get entangled,", ch, NULL, victim, TO_CHAR);
    act( "causing you to trip over yourself and collapse to the ground!", ch, NULL, victim, TO_CHAR);
    act( "$n tries to break into a dance, but collapses to the ground in $s futile attempt.", ch, NULL, victim,  TO_NOTVICT);
    if (ch->position != POS_RESTING)
      ch->position = POS_RESTING;
    damage( ch, victim, 0, gsn_dancing_strike, DAM_BASH, TRUE);
  }

  if (ch->fighting == NULL)
    set_fighting(ch, victim);

  ch->fight_pos = FIGHT_FRONT;
  check_killer(ch,victim);

  return;
}

/*
void do_bestow_curse( CHAR_DATA *ch, char *argument )
{
  char arg1[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  AFFECT_DATA af;

  if (IS_NPC(ch))
  return;

  argument = one_argument( argument, arg1 );

  if ( ( victim = get_char_room( ch, NULL, arg ) ) == NULL )
  {
	send_to_char( "They aren't here.\n\r", ch );
	return;
  }

  if ( arg1[0] == '\0' )
  {

    send_to_char( "Syntax: bestow <curse> <victim>.\n\r\n\r", ch );
    send_to_char( "        Available Curses: alter gender\n\r", ch );

    if (ch->level < 23 )
    {
      send_to_char( "                          growth\n\r", ch );
      return;
    }
    else
    if (ch->level < 26 )
    {
      send_to_char( "                          growth\n\r", ch );
      send_to_char( "                          weariness\n\r", ch );
      return;
    }
    else
    if (ch->level < 32 )
    {
      send_to_char( "                          growth\n\r", ch );
      send_to_char( "                          weariness\n\r", ch );
      send_to_char( "                          \n\r", ch );
      return;
    }
    else
    if (ch->level < 37 )
    {
      send_to_char( "                          growth\n\r", ch );
      send_to_char( "                          weariness\n\r", ch );
      send_to_char( "                          \n\r", ch );
      send_to_char( "                          \n\r", ch );
      return;
    }
    else
    if (ch->level < 45 )
    {
      send_to_char( "                          growth\n\r", ch );
      send_to_char( "                          weariness\n\r", ch );
      send_to_char( "                          \n\r", ch );
      send_to_char( "                          \n\r", ch );
      send_to_char( "                          \n\r", ch );
      return;
    }
    else
    if (ch->level < 53 )
    {
      send_to_char( "                          growth\n\r", ch );
      send_to_char( "                          weariness\n\r", ch );
      send_to_char( "                          \n\r", ch );
      send_to_char( "                          \n\r", ch );
      send_to_char( "                          \n\r", ch );
      send_to_char( "                          \n\r", ch );
      return;
    }
    else
    if (ch->level < 59 )
    {
      send_to_char( "                          growth\n\r", ch );
      send_to_char( "                          weariness\n\r", ch );
      send_to_char( "                          \n\r", ch );
      send_to_char( "                          \n\r", ch );
      send_to_char( "                          \n\r", ch );
      send_to_char( "                          \n\r", ch );
      send_to_char( "                          \n\r", ch );
      return;
    }
    else
    if (ch->level < 66 )
    {
      send_to_char( "                          growth\n\r", ch );
      send_to_char( "                          weariness\n\r", ch );
      send_to_char( "                          \n\r", ch );
      send_to_char( "                          \n\r", ch );
      send_to_char( "                          \n\r", ch );
      send_to_char( "                          \n\r", ch );
      send_to_char( "                          \n\r", ch );
      send_to_char( "                          \n\r", ch );
      return;
    }
    else
    if (ch->level < 70 )
    {
      send_to_char( "                          growth\n\r", ch );
      send_to_char( "                          weariness\n\r", ch );
      send_to_char( "                          \n\r", ch );
      send_to_char( "                          \n\r", ch );
      send_to_char( "                          \n\r", ch );
      send_to_char( "                          \n\r", ch );
      send_to_char( "                          \n\r", ch );
      send_to_char( "                          \n\r", ch );
      send_to_char( "                          \n\r", ch );
      return;
    }
    else
    if (ch->level < 75 )
    {
      send_to_char( "                          growth\n\r", ch );
      send_to_char( "                          weariness\n\r", ch );
      send_to_char( "                          \n\r", ch );
      send_to_char( "                          \n\r", ch );
      send_to_char( "                          \n\r", ch );
      send_to_char( "                          \n\r", ch );
      send_to_char( "                          \n\r", ch );
      send_to_char( "                          \n\r", ch );
      send_to_char( "                          \n\r", ch );
      send_to_char( "                          \n\r", ch );
      return;
    }
    else
    if (ch->level < 84 )
    {
      send_to_char( "                          growth\n\r", ch );
      send_to_char( "                          weariness\n\r", ch );
      send_to_char( "                          \n\r", ch );
      send_to_char( "                          \n\r", ch );
      send_to_char( "                          \n\r", ch );
      send_to_char( "                          \n\r", ch );
      send_to_char( "                          \n\r", ch );
      send_to_char( "                          \n\r", ch );
      send_to_char( "                          \n\r", ch );
      send_to_char( "                          \n\r", ch );
      send_to_char( "                          \n\r", ch );
      return;
    }
    else
    if (ch->level < 95 )
    {
      send_to_char( "                          growth\n\r", ch );
      send_to_char( "                          weariness\n\r", ch );
      send_to_char( "                          \n\r", ch );
      send_to_char( "                          \n\r", ch );
      send_to_char( "                          \n\r", ch );
      send_to_char( "                          \n\r", ch );
      send_to_char( "                          \n\r", ch );
      send_to_char( "                          \n\r", ch );
      send_to_char( "                          \n\r", ch );
      send_to_char( "                          \n\r", ch );
      send_to_char( "                          \n\r", ch );
      send_to_char( "                          \n\r", ch );
      return;
    }
    else
    return;
  }

  if ( !str_prefix( arg1, "might") )
  {
    if ((ch->level < skill_table[gsn_company_might].skill_level[ch->class])
    ||  (!IS_NPC(ch) && ch->pcdata->learned[gsn_company_might] < 1 ))
    {
      send_to_char( "Huh? You don't know the first thing about inspiration!\n\r",ch );
      return;
    }

    if ( number_percent( ) < ch->pcdata->learned[gsn_company_might])
      check_improve(ch,gsn_company_might,TRUE,1);
    else
    {
      check_improve(ch,gsn_company_might,FALSE,1);
      act("$n tries to increase morale, but fails.",
      ch,NULL,vch,TO_ROOM);
      act("You try to increase the morale of your friends, but fail.",
      ch,NULL,vch,TO_CHAR);
      return;
    }

    for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
    {
      vch_next = vch->next_in_room;
      if(!IS_NPC(vch))
      {
	  if (is_same_group(ch,vch)
        &&  IS_AFFECTED3(ch,AFF_MIGHT))
        {
          send_to_char("You can't inspire anyone else around you just yet.\n\r",ch);
          return;
        }

        if (is_same_group(ch,vch)
        && !IS_AFFECTED3(vch,AFF_MIGHT))
        {
	    af.where	= TO_AFFECTS3;
          af.type     	= gsn_company_might;
          af.level    	= ch->level;
          af.duration 	= ch->level/3;
          af.location 	= APPLY_HITROLL;
          af.modifier  	= 4;
          af.bitvector 	= AFF_MIGHT;
          affect_to_char(vch,&af);

          af.location   = APPLY_STR;
          af.modifier   = 2;
          af.bitvector  = 0;
          affect_to_char(vch,&af);
   
	    act("$n raises $s voice in an inspiring cry of honor.",ch,NULL,vch,TO_NOTVICT );
          if (vch == ch)
           send_to_char("You raise your voice into an inspiring cry of might.\n\r",ch);
          else
           send_to_char("Your morale is raised by an inspiration of might.\n\r",vch);
        } 
      }
    }
    return;
  }
  else
  if ( !str_prefix( arg1, "valor") )
  {
    if ((ch->level < skill_table[gsn_company_valor].skill_level[ch->class])
    ||  (!IS_NPC(ch) && ch->pcdata->learned[gsn_company_valor] < 1 ))
    {
      send_to_char( "Huh? You don't know the first thing about inspiration!\n\r",ch );
      return;
    }

    if ( number_percent( ) < ch->pcdata->learned[gsn_company_valor])
      check_improve(ch,gsn_company_valor,TRUE,1);
    else
    {
      check_improve(ch,gsn_company_valor,FALSE,1);
      act("$n tries to increase morale, but fails.",
      ch,NULL,NULL,TO_ROOM);
      act("You try to increase the morale of your friends, but fail.",
      ch,NULL,NULL,TO_CHAR);
      return;
    }

    for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
    {
      vch_next = vch->next_in_room;
      if(!IS_NPC(vch))
      {
	  if (is_same_group(ch,vch)
        &&  IS_AFFECTED3(ch,AFF_VALOR))
        {
          send_to_char("You can't inspire anyone else around you just yet.\n\r",ch);
          return;
        }

        if (is_same_group(ch,vch)
        && !IS_AFFECTED3(vch,AFF_VALOR))
        {
	    af.where	= TO_AFFECTS3;
          af.type     	= gsn_company_valor;
          af.level    	= ch->level;
          af.duration 	= ch->level/3;
          af.location 	= APPLY_DAMROLL;
          af.modifier  	= 2;
          af.bitvector 	= AFF_VALOR;
          affect_to_char(vch,&af);

          af.location   = APPLY_DEX;
          af.modifier   = 2;
          af.bitvector  = 0;
          affect_to_char(vch,&af);
   
	    act("$n raises $s voice in an inspiring cry of honor.",ch,NULL,vch,TO_NOTVICT );
          if (vch == ch)
           send_to_char("You raise your voice into an inspiring cry of valor.\n\r",ch);
          else
           send_to_char("Your morale is raised by an inspiration of valor.\n\r",vch);
        } 
      }
    }
    return;
  }
  else
  if ( !str_prefix( arg1, "honor") )
  {
    if ((ch->level < skill_table[gsn_company_honor].skill_level[ch->class])
    ||  (!IS_NPC(ch) && ch->pcdata->learned[gsn_company_honor] < 1 ))
    {
      send_to_char( "Huh? You don't know the first thing about inspiration!\n\r",ch );
      return;
    }

    if ( number_percent( ) < ch->pcdata->learned[gsn_company_honor])
      check_improve(ch,gsn_company_honor,TRUE,1);
    else
    {
      check_improve(ch,gsn_company_honor,FALSE,1);
      act("$n tries to increase morale, but fails.",
      ch,NULL,NULL,TO_ROOM);
      act("You try to increase the morale of your friends, but fail.",
      ch,NULL,NULL,TO_CHAR);
      return;
    }

    for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
    {
      vch_next = vch->next_in_room;
      if(!IS_NPC(vch))
      {
	  if (is_same_group(ch,vch)
        &&  IS_AFFECTED3(ch,AFF_HONOR))
        {
          send_to_char("You can't inspire anyone else around you just yet.\n\r",ch);
          return;
        }

        if (is_same_group(ch,vch)
        && !IS_AFFECTED3(vch,AFF_HONOR))
        {
	    af.where	= TO_AFFECTS3;
          af.type     	= gsn_company_honor;
          af.level    	= ch->level;
          af.duration 	= ch->level/3;
          af.location 	= APPLY_AC;
          af.modifier  	= -30;
          af.bitvector 	= AFF_HONOR;
          affect_to_char(vch,&af);

          af.location   = APPLY_INT;
          af.modifier   = 1;
          af.bitvector  = 0;
          affect_to_char(vch,&af);

          af.location   = APPLY_WIS;
          af.modifier   = 2;
          af.bitvector  = 0;
          affect_to_char(vch,&af);
   
	    act("$n raises $s voice in an inspiring cry of honor.",ch,NULL,vch,TO_NOTVICT );
          if (vch == ch)
           send_to_char("You raise your voice into an inspiring cry of honor.\n\r",ch);
          else
           send_to_char("Your morale is raised by an inspiration of honor.\n\r",vch);
        } 
      }
    }
    return;
  }
  else
  {
    send_to_char( "Syntax: inspire <might|valor|honor>.\n\r", ch );
    return;
  }
}

*/

void do_beastspite( CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  AFFECT_DATA af;
  int chance;

  argument = one_argument( argument, arg );
  one_argument( argument, arg2 );

  if (((chance = get_skill(ch,gsn_beastspite)) == 0)
  || (!IS_NPC(ch)
  && ch->level < skill_table[gsn_beastspite].skill_level[ch->class]))
  {	
    send_to_char( "You don't know how to do that.\n\r", ch );
    return;
  }

  if ( arg[0] == '\0' )
  {
    send_to_char( "Who would you like to beastspite?\n\r", ch );
    return;
  }

  if ( ( victim = get_char_room( ch, NULL, arg ) ) == NULL )
  {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  if ( ch->fighting != NULL )
  {
    send_to_char( "Your too busy fighting.\n\r", ch );
    return;
  }

  if ( IS_IMMORTAL(victim) )
  {
    send_to_char( "You'll only piss them off if you try.\n\r", ch );
    return;
  }

  if ( IS_AFFECTED2(ch, AFF_PARALYSIS) 
  ||   IS_AFFECTED3(ch, AFF_HOLDPERSON))
  {
    send_to_char("You can't move at all, there's no way you can do that.\n\r",ch);
    return;
  }

  if (IS_NPC(victim))
    return;

  if (ch == victim)
  {
    send_to_char("That wouldn't accomplish much!\n\r", ch);
    return;
  }

  if(ch->mana < 45 )
  {
    send_to_char("You lack the neccessary mental energy to perform this task!\n\r", ch);
    return;
  }

  if (is_safe(ch,victim))
  {
    send_to_char("You can't put a curse on anyone around here, there are protections in place!\n\r", ch);
    return;
  }

  if (number_percent() < chance )
  {
    if (!str_cmp(arg2,"bear"))
    {
      if ( IS_AFFECTED3(victim, AFF_BEASTSPITE_BEAR)
      ||   IS_AFFECTED3(victim, AFF_BEASTSPITE_LIZARD)
      ||   IS_AFFECTED3(victim, AFF_BEASTSPITE_FOX)
      ||   IS_AFFECTED3(victim, AFF_BEASTSPITE_WOLF)
      ||   IS_AFFECTED3(victim, AFF_BEASTSPITE_WYVERN)
      ||   IS_AFFECTED3(victim, AFF_BEASTSPITE_DRAGON)
      ||   IS_AFFECTED3(victim, AFF_BEASTSPITE_BAT))
      {
        act("$N is already spited by certain beasts.  Additional spites would only\n\r"
		"ensure $S death!",ch,NULL,victim,TO_CHAR);
        return;
      }

      act("$n has cast a curse-like spell upon you.  From this moment on, any time\n\r"
	    "you come in contact with a bear, the bear will spite you and attack on site!\n\r"
	    "You will have to wait until the spell wears off before you can approach bears\n\r"
	    "without being attacked for being spiteful of your presence.\n\r",ch,NULL,victim,TO_VICT);

      act("Your spell has ensured that $N will be spited by any bear $E comes\n\r" 
          "in contact with and will be attacked on site, until the spell wears off!\n\r",ch,NULL,victim,TO_CHAR);

      af.where     = TO_AFFECTS3;
      af.type      = gsn_beastspite;
      af.level     = ch->level;
      af.duration  = ch->level/5;
      af.bitvector = AFF_BEASTSPITE_BEAR;
      affect_to_char( victim, &af );
      ch->mana -= 45;
      check_improve(ch,gsn_beastspite,TRUE,1);
	return;
    }
    else
    if (!str_cmp(arg2,"lizard"))
    {
      if ( IS_AFFECTED3(victim, AFF_BEASTSPITE_BEAR)
      ||   IS_AFFECTED3(victim, AFF_BEASTSPITE_LIZARD)
      ||   IS_AFFECTED3(victim, AFF_BEASTSPITE_FOX)
      ||   IS_AFFECTED3(victim, AFF_BEASTSPITE_WOLF)
      ||   IS_AFFECTED3(victim, AFF_BEASTSPITE_WYVERN)
      ||   IS_AFFECTED3(victim, AFF_BEASTSPITE_DRAGON)
      ||   IS_AFFECTED3(victim, AFF_BEASTSPITE_BAT))
      {
        act("$N is already spited by certain beasts.  Additional spites would only\n\r"
		"ensure $S death!",ch,NULL,victim,TO_CHAR);
        return;
      }

      act("$n has cast a curse-like spell upon you.  From this moment on, any time\n\r"
	    "you come in contact with a lizard, the lizard will spite you and attack on site!\n\r"
	    "You will have to wait until the spell wears off before you can approach lizards\n\r"
	    "without being attacked for being spiteful of your presence.\n\r",ch,NULL,victim,TO_VICT);

      act("Your spell has ensured that $N will be spited by any lizard $E comes\n\r" 
          "in contact with and will be attacked on site, until the spell wears off!\n\r",ch,NULL,victim,TO_CHAR);

      af.where     = TO_AFFECTS3;
      af.type      = gsn_beastspite;
      af.level     = ch->level;
      af.duration  = ch->level/5;
      af.bitvector = AFF_BEASTSPITE_LIZARD;
      affect_to_char( victim, &af );
      ch->mana -= 45;
      check_improve(ch,gsn_beastspite,TRUE,1);
	return;
    }
    else
    if (!str_cmp(arg2,"fox"))
    {
      if ( IS_AFFECTED3(victim, AFF_BEASTSPITE_BEAR)
      ||   IS_AFFECTED3(victim, AFF_BEASTSPITE_LIZARD)
      ||   IS_AFFECTED3(victim, AFF_BEASTSPITE_FOX)
      ||   IS_AFFECTED3(victim, AFF_BEASTSPITE_WOLF)
      ||   IS_AFFECTED3(victim, AFF_BEASTSPITE_WYVERN)
      ||   IS_AFFECTED3(victim, AFF_BEASTSPITE_DRAGON)
      ||   IS_AFFECTED3(victim, AFF_BEASTSPITE_BAT))
      {
        act("$N is already spited by certain beasts.  Additional spites would only\n\r"
		"ensure $S death!",ch,NULL,victim,TO_CHAR);
        return;
      }

      act("$n has cast a curse-like spell upon you.  From this moment on, any time\n\r"
	    "you come in contact with a fox, the fox will spite you and attack on site!\n\r"
	    "You will have to wait until the spell wears off before you can approach a fox\n\r"
	    "without being attacked for being spiteful of your presence.\n\r",ch,NULL,victim,TO_VICT);

      act("Your spell has ensured that $N will be spited by any fox $E comes\n\r" 
          "in contact with and will be attacked on site, until the spell wears off!\n\r",ch,NULL,victim,TO_CHAR);

      af.where     = TO_AFFECTS3;
      af.type      = gsn_beastspite;
      af.level     = ch->level;
      af.duration  = ch->level/5;
      af.bitvector = AFF_BEASTSPITE_FOX;
      affect_to_char( victim, &af );
      ch->mana -= 45;
      check_improve(ch,gsn_beastspite,TRUE,1);
	return;
    }
    else
    if (!str_cmp(arg2,"wolf"))
    {
      if ( IS_AFFECTED3(victim, AFF_BEASTSPITE_BEAR)
      ||   IS_AFFECTED3(victim, AFF_BEASTSPITE_LIZARD)
      ||   IS_AFFECTED3(victim, AFF_BEASTSPITE_FOX)
      ||   IS_AFFECTED3(victim, AFF_BEASTSPITE_WOLF)
      ||   IS_AFFECTED3(victim, AFF_BEASTSPITE_WYVERN)
      ||   IS_AFFECTED3(victim, AFF_BEASTSPITE_DRAGON)
      ||   IS_AFFECTED3(victim, AFF_BEASTSPITE_BAT))
      {
        act("$N is already spited by certain beasts.  Additional spites would only\n\r"
		"ensure $S death!",ch,NULL,victim,TO_CHAR);
        return;
      }

      act("$n has cast a curse-like spell upon you.  From this moment on, any time\n\r"
	    "you come in contact with a wolf, the wolf will spite you and attack on site!\n\r"
	    "You will have to wait until the spell wears off before you can approach wolves\n\r"
	    "without being attacked for being spiteful of your presence.\n\r",ch,NULL,victim,TO_VICT);

      act("Your spell has ensured that $N will be spited by any wolf $E comes\n\r" 
          "in contact with and will be attacked on site, until the spell wears off!\n\r",ch,NULL,victim,TO_CHAR);

      af.where     = TO_AFFECTS3;
      af.type      = gsn_beastspite;
      af.level     = ch->level;
      af.duration  = ch->level/5;
      af.bitvector = AFF_BEASTSPITE_WOLF;
      affect_to_char( victim, &af );
      ch->mana -= 45;
      check_improve(ch,gsn_beastspite,TRUE,1);
	return;
    }
    else
    if (!str_cmp(arg2,"wyvern"))
    {
      if ( IS_AFFECTED3(victim, AFF_BEASTSPITE_BEAR)
      ||   IS_AFFECTED3(victim, AFF_BEASTSPITE_LIZARD)
      ||   IS_AFFECTED3(victim, AFF_BEASTSPITE_FOX)
      ||   IS_AFFECTED3(victim, AFF_BEASTSPITE_WOLF)
      ||   IS_AFFECTED3(victim, AFF_BEASTSPITE_WYVERN)
      ||   IS_AFFECTED3(victim, AFF_BEASTSPITE_DRAGON)
      ||   IS_AFFECTED3(victim, AFF_BEASTSPITE_BAT))
      {
        act("$N is already spited by certain beasts.  Additional spites would only\n\r"
		"ensure $S death!",ch,NULL,victim,TO_CHAR);
        return;
      }

      act("$n has cast a curse-like spell upon you.  From this moment on, any time\n\r"
	    "you come in contact with a wyvern, the wyvern will spite you and attack on site!\n\r"
	    "You will have to wait until the spell wears off before you can approach wyverns\n\r"
	    "without being attacked for being spiteful of your presence.\n\r",ch,NULL,victim,TO_VICT);

      act("Your spell has ensured that $N will be spited by any wyvern $E comes\n\r" 
          "in contact with and will be attacked on site, until the spell wears off!\n\r",ch,NULL,victim,TO_CHAR);

      af.where     = TO_AFFECTS3;
      af.type      = gsn_beastspite;
      af.level     = ch->level;
      af.duration  = ch->level/5;
      af.bitvector = AFF_BEASTSPITE_WYVERN;
      affect_to_char( victim, &af );
      ch->mana -= 45;
      check_improve(ch,gsn_beastspite,TRUE,1);
	return;
    }
    else
    if (!str_cmp(arg2,"dragon"))
    {
      if ( IS_AFFECTED3(victim, AFF_BEASTSPITE_BEAR)
      ||   IS_AFFECTED3(victim, AFF_BEASTSPITE_LIZARD)
      ||   IS_AFFECTED3(victim, AFF_BEASTSPITE_FOX)
      ||   IS_AFFECTED3(victim, AFF_BEASTSPITE_WOLF)
      ||   IS_AFFECTED3(victim, AFF_BEASTSPITE_WYVERN)
      ||   IS_AFFECTED3(victim, AFF_BEASTSPITE_DRAGON)
      ||   IS_AFFECTED3(victim, AFF_BEASTSPITE_BAT))
      {
        act("$N is already spited by certain beasts.  Additional spites would only\n\r"
		"ensure $S death!",ch,NULL,victim,TO_CHAR);
        return;
      }

      act("$n has cast a curse-like spell upon you.  From this moment on, any time\n\r"
	    "you come in contact with a dragon, the dragon will spite you and attack on site!\n\r"
	    "You will have to wait until the spell wears off before you can approach dragons\n\r"
	    "without being attacked for being spiteful of your presence.\n\r",ch,NULL,victim,TO_VICT);

      act("Your spell has ensured that $N will be spited by any dragon $E comes\n\r" 
          "in contact with and will be attacked on site, until the spell wears off!\n\r",ch,NULL,victim,TO_CHAR);

      af.where     = TO_AFFECTS3;
      af.type      = gsn_beastspite;
      af.level     = ch->level;
      af.duration  = ch->level/5;
      af.bitvector = AFF_BEASTSPITE_DRAGON;
      affect_to_char( victim, &af );
      ch->mana -= 45;
      check_improve(ch,gsn_beastspite,TRUE,1);
	return;
    }
    else
    if (!str_cmp(arg2,"bat"))
    {
      if ( IS_AFFECTED3(victim, AFF_BEASTSPITE_BEAR)
      ||   IS_AFFECTED3(victim, AFF_BEASTSPITE_LIZARD)
      ||   IS_AFFECTED3(victim, AFF_BEASTSPITE_FOX)
      ||   IS_AFFECTED3(victim, AFF_BEASTSPITE_WOLF)
      ||   IS_AFFECTED3(victim, AFF_BEASTSPITE_WYVERN)
      ||   IS_AFFECTED3(victim, AFF_BEASTSPITE_DRAGON)
      ||   IS_AFFECTED3(victim, AFF_BEASTSPITE_BAT))
      {
        act("$N is already spited by certain beasts.  Additional spites would only\n\r"
		"ensure $S death!",ch,NULL,victim,TO_CHAR);
        return;
      }

      act("$n has cast a curse-like spell upon you.  From this moment on, any time\n\r"
	    "you come in contact with a bat, the bat will spite you and attack on site!\n\r"
	    "You will have to wait until the spell wears off before you can approach bats\n\r"
	    "without being attacked for being spiteful of your presence.\n\r",ch,NULL,victim,TO_VICT);

      act("Your spell has ensured that $N will be spited by any bat $E comes\n\r" 
          "in contact with and will be attacked on site, until the spell wears off!\n\r",ch,NULL,victim,TO_CHAR);

      af.where     = TO_AFFECTS3;
      af.type      = gsn_beastspite;
      af.level     = ch->level;
      af.duration  = ch->level/5;
      af.bitvector = AFF_BEASTSPITE_BAT;
      affect_to_char( victim, &af );
      ch->mana -= 45;
      check_improve(ch,gsn_beastspite,TRUE,1);
	return;
    }
    else
    {
      send_to_char( "What beast would you like to spite your victim?\n\r",ch);
      send_to_char( "Your options are:\n\r",ch);
      send_to_char( "- bear\n\r",ch);
      send_to_char( "- lizard\n\r",ch);
      send_to_char( "- fox\n\r",ch);
      send_to_char( "- wolf\n\r",ch);
      send_to_char( "- wyvern\n\r",ch);
      send_to_char( "- dragon\n\r",ch);
      send_to_char( "- bat\n\r",ch);
      return;
    }
  }
  else
  {
    send_to_char( "You failed!\n\r",ch);
  }
  return;
}

void do_relegate( CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim;
  ROOM_INDEX_DATA *pRoomIndex;
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  int chance;
  int lvnum;
  int uvnum;
    
  if ( IS_NPC(ch) )
    return;

  argument = one_argument (argument, arg);

  if ( (chance = get_skill(ch,gsn_relegate)) == 0
  ||   (ch->level < skill_table[gsn_relegate].skill_level[ch->class]))
  {
    send_to_char("You have no idea how to relegate someone!\n\r",ch);
    return;
  }

  if ( IS_AFFECTED2(ch, AFF_PARALYSIS) 
  ||   IS_AFFECTED3(ch, AFF_HOLDPERSON))
  {
    send_to_char("You can't move at all, there's no way you can do that.\n\r",ch);
    return;
  }

  if ( arg[0] == '\0' )
  {
    send_to_char( "Relegate whom?\n\r", ch );
    return;
  }

  if ( ( victim = get_char_world( ch, arg ) ) == NULL )
  {
    send_to_char( "You cannot find them.\n\r", ch );
    return;
  }

  if (IS_NPC(victim) )
  {
      send_to_char("You cannot relegate a non-player.\n\r",ch);
      return;
  }

  if (IS_IMMORTAL(victim))
  {
    send_to_char("You cannot relegate one so powerful.\n\r",ch);
    return;
  }
    
  if( ch->mana < 100) 
  {
    send_to_char( "You do not have enough mental energy to relegate anyone.\n\r",ch);
    return;
  }

  if ( ch->fighting != NULL )
  {
    send_to_char( "Your too busy fighting.\n\r", ch );
    return;
  }

  if (victim == ch)
  {
    send_to_char("You can't relegate yourself!\n\r",ch);
    return;
  }

  if ( victim->in_room == NULL )
  {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  if ( !IS_SET(ch->in_room->room_flags, ROOM_TEMPLE) )
  {
    send_to_char( "You can only relegate someone while in a divine Temple.\n\r", ch );
    return;
  }

  if (!IS_NPC(ch) && victim->fighting != NULL )
  {
    send_to_char( "You can't relegate someone while they're fighting.\n\r", ch );
    return;
  }

  if ( victim->class == class_lookup("cleric"))
  {
    send_to_char( "You can not relegate fellow Clerics.\n\r", ch );
    return;
  }

  if( ch->religion_id == RELIGION_NONE )
  {
    send_to_char( "You must follow a religion and be marked by a God in order to relegate someone.\n\r", ch );
    return;
  }

  check_improve(ch,gsn_relegate,TRUE,1);

  lvnum = ch->in_room->area->min_vnum;
  uvnum = ch->in_room->area->max_vnum;

    for ( ; ; )
    {
        pRoomIndex = get_room_index( number_range( lvnum,uvnum ) );
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
      sprintf(buf, "You've disgraced this Temple %s, now begone!", 
      (is_affected(victim, gsn_doppelganger)&& (!IS_SET(ch->act,PLR_HOLYLIGHT)))? victim->doppel->name :
      (is_affected(victim, gsn_doppelganger)&& (!IS_SET(ch->act,PLR_HOLYLIGHT)))? "oak tree" :
       victim->name);
	do_yell( ch, buf);

     act("You open up a small dimensional rift and push $N into it.\n\r", ch,NULL, victim, TO_CHAR);
     act("$n opens up a small dimensional rift and pushes you into it.\n\r", ch,NULL, victim, TO_VICT);

     act( "$n is pushed into a small dimensional rift and disappears!", victim, NULL, NULL, TO_ROOM );

     char_from_room( victim );
     char_to_room( victim, pRoomIndex );
     if (victim->riding && !IS_NPC(victim))
     {
	 victim->riding->rider = NULL;
	 victim->riding        = NULL;
     }
     victim->position = POS_STANDING; 
     act( "Out of nowhere, $n suddenly appears.", victim, NULL, NULL, TO_ROOM );
     do_look( victim, "auto" );
     return;
    }

}

