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
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "interp.h"
#include "magic.h"


void one_hit     		args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt, bool secondary ) );

/*
 * The following special functions are available for mobiles.
 */
DECLARE_SPEC_FUN( spec_questmaster		);
DECLARE_SPEC_FUN(	spec_breath_any		);
DECLARE_SPEC_FUN(	spec_breath_acid	);
DECLARE_SPEC_FUN(	spec_breath_fire	);
DECLARE_SPEC_FUN(	spec_breath_frost	);
DECLARE_SPEC_FUN(	spec_breath_gas		);
DECLARE_SPEC_FUN(	spec_breath_lightning	);
DECLARE_SPEC_FUN(	spec_cast_adept		);
DECLARE_SPEC_FUN(	spec_cast_cleric	);
DECLARE_SPEC_FUN(	spec_cast_judge		);
DECLARE_SPEC_FUN(	spec_cast_mage		);
DECLARE_SPEC_FUN(	spec_cast_undead	);
DECLARE_SPEC_FUN(	spec_executioner	);
DECLARE_SPEC_FUN(	spec_fido		);
DECLARE_SPEC_FUN(	spec_guard		);
DECLARE_SPEC_FUN(	spec_janitor		);
DECLARE_SPEC_FUN(	spec_mayor		);
DECLARE_SPEC_FUN(	spec_poison		);
DECLARE_SPEC_FUN(	spec_thief		);
DECLARE_SPEC_FUN(	spec_nasty		);
DECLARE_SPEC_FUN(	spec_troll_member	);
DECLARE_SPEC_FUN(	spec_ogre_member	);
DECLARE_SPEC_FUN(	spec_patrolman		);
DECLARE_SPEC_FUN(	spec_dark_magic		);
DECLARE_SPEC_FUN(	spec_angry		);
DECLARE_SPEC_FUN(	spec_animal		);
DECLARE_SPEC_FUN(	spec_belligerent	);
DECLARE_SPEC_FUN(	spec_dolphin		);
DECLARE_SPEC_FUN(	spec_evilguard		);
DECLARE_SPEC_FUN(	spec_ghost		);
DECLARE_SPEC_FUN(	spec_happy		);
DECLARE_SPEC_FUN(	spec_priesttakhisis	);
DECLARE_SPEC_FUN(	spec_jester		);
DECLARE_SPEC_FUN(	spec_merchant		);
DECLARE_SPEC_FUN( 	spec_prophet		);
DECLARE_SPEC_FUN(	spec_sad		);
DECLARE_SPEC_FUN( 	spec_rloader		);
DECLARE_SPEC_FUN(	spec_ferrysol		);
DECLARE_SPEC_FUN(	spec_ferrypal		);
DECLARE_SPEC_FUN( 	spec_orgguard		);
DECLARE_SPEC_FUN(	spec_hometownchanger	);
DECLARE_SPEC_FUN(	spec_jorge		);

/* the function table */
const   struct  spec_type    spec_table[] =
{
    { "spec_questmaster",           	spec_questmaster        },
    {	"spec_breath_any",		spec_breath_any		},
    {	"spec_breath_acid",		spec_breath_acid	},
    {	"spec_breath_fire",		spec_breath_fire	},
    {	"spec_breath_frost",		spec_breath_frost	},
    {	"spec_breath_gas",		spec_breath_gas		},
    {	"spec_breath_lightning",	spec_breath_lightning	},	
    {	"spec_cast_adept",		spec_cast_adept		},
    {	"spec_cast_cleric",		spec_cast_cleric	},
    {	"spec_cast_judge",		spec_cast_judge		},
    {	"spec_cast_mage",		spec_cast_mage		},
    {	"spec_cast_undead",		spec_cast_undead	},
    {	"spec_executioner",		spec_executioner	},
    {	"spec_fido",			spec_fido		},
    {	"spec_guard",			spec_guard		},
    {	"spec_janitor",			spec_janitor		},
    {	"spec_mayor",			spec_mayor		},
    {	"spec_poison",			spec_poison		},
    {	"spec_thief",			spec_thief		},
    {	"spec_nasty",			spec_nasty		},
    {	"spec_troll_member",		spec_troll_member	},
    {	"spec_ogre_member",		spec_ogre_member	},
    {	"spec_patrolman",		spec_patrolman		},
    {	"spec_dark_magic",		spec_dark_magic		},
    {	"spec_angry",			spec_angry		},
    { 	"spec_animal",			spec_animal		},
    { 	"spec_belligerent",		spec_belligerent	},
    { 	"spec_dolphin",			spec_dolphin		},
    { 	"spec_evilguard",		spec_evilguard		},
    {	"spec_ghost",			spec_ghost		},
    {	"spec_happy",			spec_happy		},
    {	"spec_priesttakhisis",		spec_priesttakhisis	},
    { 	"spec_jester",			spec_jester		},
    {	"spec_merchant",		spec_merchant		},
    {	"spec_prophet",			spec_prophet		},
    {	"spec_sad",			spec_sad		},
    { 	"spec_rloader",			spec_rloader		},
    { 	"spec_ferrysol",		spec_ferrysol		},
    { 	"spec_ferrypal",		spec_ferrypal		},
    { 	"spec_orgguard",		spec_orgguard		},
    { 	"spec_hometownchanger",		spec_hometownchanger	},
    {	"spec_jorge",			spec_jorge		},
    {	NULL,				NULL			}
};

/*
 * Given a name, return the appropriate spec fun.
 */
SPEC_FUN *spec_lookup( const char *name )
{
   int i;
 
   for ( i = 0; spec_table[i].name != NULL; i++)
   {
        if (LOWER(name[0]) == LOWER(spec_table[i].name[0])
        &&  !str_prefix( name,spec_table[i].name))
            return spec_table[i].function;
   }
 
    return 0;
}

char *spec_name( SPEC_FUN *function)
{
    int i;

    for (i = 0; spec_table[i].function != NULL; i++)
    {
	if (function == spec_table[i].function)
	    return spec_table[i].name;
    }

    return NULL;
}

bool spec_questmaster (CHAR_DATA *ch)
{
    if (ch->fighting != NULL)
     return spec_cast_mage( ch );
    return FALSE;
}

bool spec_troll_member( CHAR_DATA *ch)
{
    CHAR_DATA *vch, *victim = NULL;
    int count = 0;
    char *message;

    if (!IS_AWAKE(ch)
    || IS_AFFECTED(ch,AFF_CALM)
    || ch->in_room == NULL 
    ||  IS_AFFECTED(ch,AFF_CHARM)
    || IS_AFFECTED2(ch,AFF_RAINBOW_PATTERN)
    || ch->fighting != NULL)
	return FALSE;

    /* find an ogre to beat up */
    for (vch = ch->in_room->people;  vch != NULL;  vch = vch->next_in_room)
    {
	if (!IS_NPC(vch) || ch == vch)
	    continue;

	if (vch->pIndexData->vnum == MOB_VNUM_PATROLMAN)
	    return FALSE;

	if (vch->pIndexData->group == GROUP_VNUM_OGRES
	&&  ch->level > vch->level - 2 && !is_safe(ch,vch))
	{
	    if (number_range(0,count) == 0)
		victim = vch;

	    count++;
	}
    }

    if (victim == NULL)
	return FALSE;

    /* say something, then raise hell */
    switch (number_range(0,6))
    {
	default:  message = NULL; 	break;
	case 0:	message = "$n yells 'I've been looking for you, punk!'";
		break;
	case 1: message = "With a scream of rage, $n attacks $N.";
		break;
	case 2: message = 
		"$n says 'What's slimy Ogre trash like you doing around here?'";
		break;
	case 3: message = "$n cracks his knuckles and says 'Do ya feel lucky?'";
		break;
	case 4: message = "$n says 'There's no cops to save you this time!'";
		break;	
	case 5: message = "$n says 'Time to join your brother, spud.'";
		break;
	case 6: message = "$n says 'Let's rock.'";
		break;
    }

    if (message != NULL)
    	act(message,ch,NULL,victim,TO_ALL);
    multi_hit( ch, victim, TYPE_UNDEFINED );
    return TRUE;
}

bool spec_ogre_member( CHAR_DATA *ch)
{
    CHAR_DATA *vch, *victim = NULL;
    int count = 0;
    char *message;
 
    if (!IS_AWAKE(ch)
    || IS_AFFECTED(ch,AFF_CALM)
    || ch->in_room == NULL
    ||  IS_AFFECTED(ch,AFF_CHARM)
    || IS_AFFECTED2(ch,AFF_RAINBOW_PATTERN)
    || ch->fighting != NULL)
        return FALSE;

    /* find an troll to beat up */
    for (vch = ch->in_room->people;  vch != NULL;  vch = vch->next_in_room)
    {
        if (!IS_NPC(vch) || ch == vch)
            continue;
 
        if (vch->pIndexData->vnum == MOB_VNUM_PATROLMAN)
            return FALSE;
 
        if (vch->pIndexData->group == GROUP_VNUM_TROLLS
        &&  ch->level > vch->level - 2 && !is_safe(ch,vch))
        {
            if (number_range(0,count) == 0)
                victim = vch;
 
            count++;
        }
    }
 
    if (victim == NULL)
        return FALSE;
 
    /* say something, then raise hell */
    switch (number_range(0,6))
    {
	default: message = NULL;	break;
        case 0: message = "$n yells 'I've been looking for you, punk!'";
                break;
        case 1: message = "With a scream of rage, $n attacks $N.'";
                break;
        case 2: message =
                "$n says 'What's Troll filth like you doing around here?'";
                break;
        case 3: message = "$n cracks his knuckles and says 'Do ya feel lucky?'";
                break;
        case 4: message = "$n says 'There's no cops to save you this time!'";
                break;
        case 5: message = "$n says 'Time to join your brother, spud.'";
                break;
        case 6: message = "$n says 'Let's rock.'";
                break;
    }
 
    if (message != NULL)
    	act(message,ch,NULL,victim,TO_ALL);
    multi_hit( ch, victim, TYPE_UNDEFINED );
    return TRUE;
}

bool spec_patrolman(CHAR_DATA *ch)
{
    CHAR_DATA *vch,*victim = NULL;
    OBJ_DATA *obj;
    char *message;
    int count = 0;

    if (!IS_AWAKE(ch)
    || IS_AFFECTED(ch,AFF_CALM)
    || ch->in_room == NULL
    ||  IS_AFFECTED(ch,AFF_CHARM)
    || IS_AFFECTED2(ch,AFF_RAINBOW_PATTERN)
    || ch->fighting != NULL)
        return FALSE;

    /* look for a fight in the room */
    for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
    {
	if (vch == ch)
	    continue;

	if (vch->fighting != NULL)  /* break it up! */
	{
	    if (number_range(0,count) == 0)
	        victim = (vch->level > vch->fighting->level) 
		    ? vch : vch->fighting;
	    count++;
	}
    }

    if (victim == NULL || (IS_NPC(victim) && victim->spec_fun == ch->spec_fun))
	return FALSE;

    if (((obj = get_eq_char(ch,WEAR_NECK_1)) != NULL 
    &&   obj->pIndexData->vnum == OBJ_VNUM_WHISTLE)
    ||  ((obj = get_eq_char(ch,WEAR_NECK_2)) != NULL
    &&   obj->pIndexData->vnum == OBJ_VNUM_WHISTLE))
    {
	act("You blow down hard on $p.",ch,obj,NULL,TO_CHAR);
	act("$n blows on $p, ***WHEEEEEEEEEEEET***",ch,obj,NULL,TO_ROOM);

    	for ( vch = char_list; vch != NULL; vch = vch->next )
    	{
            if ( vch->in_room == NULL )
            	continue;

            if (vch->in_room != ch->in_room 
	    &&  vch->in_room->area == ch->in_room->area)
            	send_to_char( "You hear a shrill whistling sound.\n\r", vch );
    	}
    }

    switch (number_range(0,6))
    {
	default:	message = NULL;		break;
	case 0:	message = "$n yells 'All roit! All roit! break it up!'";
		break;
	case 1: message = "$n says 'Society's to blame, but what's a bloke to do?'";
		break;
	case 2: message = "$n mumbles 'bloody kids will be the death of us all.'";
		break;
	case 3: message = "$n shouts 'Stop that! Stop that!' and attacks.";
		break;
	case 4: message = "$n pulls out his billy and goes to work.";
		break;
	case 5: message = "$n sighs in resignation and proceeds to break up the fight.";
		break;
	case 6: message = "$n says 'Settle down, you hooligans!'";
		break;
    }

    if (message != NULL)
	act(message,ch,NULL,NULL,TO_ALL);

    multi_hit(ch,victim,TYPE_UNDEFINED);

    return TRUE;
}
	

bool spec_nasty( CHAR_DATA *ch )
{
    CHAR_DATA *victim, *v_next;
    long steel;
 
    if (!IS_AWAKE(ch)) {
       return FALSE;
    }
 
    if (ch->position != POS_FIGHTING) {
       for ( victim = ch->in_room->people; victim != NULL; victim = v_next)
       {
          v_next = victim->next_in_room;
          if (!IS_NPC(victim)
             && (victim->level > ch->level)
             && (victim->level < ch->level + 10))
          {
	     do_function(ch, &do_backstab, victim->name);
             if (ch->position != POS_FIGHTING)
	     {
                 do_function(ch, &do_murder, victim->name);
	     }

             /* should steal some coins right away? :) */
             return TRUE;
          }
       }
       return FALSE;    /*  No one to attack */
    }
 
    /* okay, we must be fighting.... steal some coins and flee */
    if ( (victim = ch->fighting) == NULL)
        return FALSE;   /* let's be paranoid.... */
 
    switch ( number_bits(2) )
    {
        case 0:  act( "$n rips apart your coin purse, spilling your steel coins!",
                     ch, NULL, victim, TO_VICT);
                 act( "You slash apart $N's coin purse and gather his steel coins.",
                     ch, NULL, victim, TO_CHAR);
                 act( "$N's coin purse is ripped apart!",
                     ch, NULL, victim, TO_NOTVICT);
                 steel = victim->steel / 10;  /* steal 10% of his steel */
                 victim->steel -= steel;
                 ch->steel     += steel;
                 return TRUE;
 
        case 1:  do_function(ch, &do_flee, "");
                 return TRUE;
 
        default: return FALSE;
    }
}

/*
 * Core procedure for dragons.
 */
bool dragon( CHAR_DATA *ch, char *spell_name )
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    int sn;

    if ( ch->position != POS_FIGHTING )
	return FALSE;

    for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
    {
	v_next = victim->next_in_room;
	if ( victim->fighting == ch && number_bits( 3 ) == 0 )
	    break;
    }

    if ( victim == NULL )
	return FALSE;

    if ( ( sn = skill_lookup( spell_name ) ) < 0 )
	return FALSE;
    (*skill_table[sn].spell_fun) ( sn, ch->level, ch, victim, TARGET_CHAR);
    return TRUE;
}



/*
 * Special procedures for mobiles.
 */
bool spec_breath_any( CHAR_DATA *ch )
{
    if ( ch->position != POS_FIGHTING )
	return FALSE;

    switch ( number_bits( 3 ) )
    {
    case 0: return spec_breath_fire		( ch );
    case 1:
    case 2: return spec_breath_lightning	( ch );
    case 3: return spec_breath_gas		( ch );
    case 4: return spec_breath_acid		( ch );
    case 5:
    case 6:
    case 7: return spec_breath_frost		( ch );
    }

    return FALSE;
}



bool spec_breath_acid( CHAR_DATA *ch )
{
    return dragon( ch, "acid breath" );
}



bool spec_breath_fire( CHAR_DATA *ch )
{
    return dragon( ch, "fire breath" );
}



bool spec_breath_frost( CHAR_DATA *ch )
{
    return dragon( ch, "frost breath" );
}



bool spec_breath_gas( CHAR_DATA *ch )
{
    int sn;

    if ( ch->position != POS_FIGHTING )
	return FALSE;

    if ( ( sn = skill_lookup( "gas breath" ) ) < 0 )
	return FALSE;
    (*skill_table[sn].spell_fun) ( sn, ch->level, ch, NULL,TARGET_CHAR);
    return TRUE;
}



bool spec_breath_lightning( CHAR_DATA *ch )
{
    return dragon( ch, "lightning breath" );
}



bool spec_cast_adept( CHAR_DATA *ch )
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;

    if ( !IS_AWAKE(ch) )
	return FALSE;

    for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
    {
	v_next = victim->next_in_room;
	if ( victim != ch && can_see( ch, victim ) && number_bits( 1 ) == 0 
	     && !IS_NPC(victim) && victim->level < 11)
	    break;
    }

    if ( victim == NULL )
	return FALSE;

    switch ( number_bits( 4 ) )
    {
    case 0:
	act( "$n utters the word 'abrazak'.", ch, NULL, NULL, TO_ROOM );
	spell_armor( skill_lookup( "armor" ), ch->level,ch,victim,TARGET_CHAR);
	return TRUE;

    case 1:
	act( "$n utters the word 'fido'.", ch, NULL, NULL, TO_ROOM );
	spell_bless( skill_lookup( "bless" ), ch->level,ch,victim,TARGET_CHAR);
	return TRUE;

    case 2:
	act("$n utters the words 'judicandus noselacri'.",ch,NULL,NULL,TO_ROOM);
	spell_cure_blindness( skill_lookup( "cure blindness" ),
	    ch->level, ch, victim,TARGET_CHAR);
	return TRUE;

    case 3:
	act("$n utters the words 'judicandus dies'.", ch,NULL, NULL, TO_ROOM );
	spell_cure_light( skill_lookup( "cure light" ),
	    ch->level, ch, victim,TARGET_CHAR);
	return TRUE;

    case 4:
	act( "$n utters the words 'judicandus sausabru'.",ch,NULL,NULL,TO_ROOM);
	spell_cure_poison( skill_lookup( "cure poison" ),
	    ch->level, ch, victim,TARGET_CHAR);
	return TRUE;

    case 5:
	act("$n utters the word 'candusima'.", ch, NULL, NULL, TO_ROOM );
	spell_refresh( skill_lookup("refresh"),ch->level,ch,victim,TARGET_CHAR);
	return TRUE;

    case 6:
	act("$n utters the words 'judicandus eugzagz'.",ch,NULL,NULL,TO_ROOM);
	spell_cure_disease(skill_lookup("cure disease"),
	    ch->level,ch,victim,TARGET_CHAR);
    }

    return FALSE;
}



bool spec_cast_cleric( CHAR_DATA *ch )
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    char *spell;
    int sn;

    if ( ch->position != POS_FIGHTING )
	return FALSE;

    for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
    {
	v_next = victim->next_in_room;
	if ( victim->fighting == ch && number_bits( 2 ) == 0 )
	    break;
    }

    if ( victim == NULL )
	return FALSE;

    for ( ;; )
    {
	int min_level;

	switch ( number_bits( 4 ) )
	{
	case  0: min_level =  0; spell = "blindness";      break;
	case  1: min_level =  3; spell = "cause serious";  break;
	case  2: min_level =  7; spell = "earthquake";     break;
	case  3: min_level =  9; spell = "cause critical"; break;
	case  4: min_level = 10; spell = "dispel evil";    break;
	case  5: min_level = 12; spell = "curse";          break;
	case  6: min_level = 12; spell = "change sex";     break;
	case  7: min_level = 13; spell = "flamestrike";    break;
	case  8:
	case  9:
	case 10: min_level = 15; spell = "harm";           break;
	case 11: min_level = 15; spell = "plague";	   break;
	default: min_level = 16; spell = "dispel magic";   break;
	}

	if ( ch->level >= min_level )
	    break;
    }

    if ( ( sn = skill_lookup( spell ) ) < 0 )
	return FALSE;
    (*skill_table[sn].spell_fun) ( sn, ch->level, ch, victim,TARGET_CHAR);
    return TRUE;
}

bool spec_cast_judge( CHAR_DATA *ch )
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    char *spell;
    int sn;
 
    if ( ch->position != POS_FIGHTING )
        return FALSE;
 
    for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
    {
        v_next = victim->next_in_room;
        if ( victim->fighting == ch && number_bits( 2 ) == 0 )
            break;
    }
 
    if ( victim == NULL )
        return FALSE;
 
    spell = "high explosive";
    if ( ( sn = skill_lookup( spell ) ) < 0 )
        return FALSE;
    (*skill_table[sn].spell_fun) ( sn, ch->level, ch, victim,TARGET_CHAR);
    return TRUE;
}



bool spec_cast_mage( CHAR_DATA *ch )
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    char *spell;
    int sn;

    if ( ch->position != POS_FIGHTING )
	return FALSE;

    for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
    {
	v_next = victim->next_in_room;
	if ( victim->fighting == ch && number_bits( 2 ) == 0 )
	    break;
    }

    if ( victim == NULL )
	return FALSE;

    for ( ;; )
    {
	int min_level;

	switch ( number_bits( 4 ) )
	{
	case  0: min_level =  0; spell = "blindness";      break;
	case  1: min_level =  3; spell = "chill touch";    break;
	case  2: min_level =  7; spell = "weaken";         break;
	case  3: min_level =  8; spell = "teleport";       break;
	case  4: min_level = 11; spell = "colour spray";   break;
	case  5: min_level = 12; spell = "change sex";     break;
	case  6: min_level = 13; spell = "energy drain";   break;
	case  7:
	case  8:
	case  9: min_level = 15; spell = "fireball";       break;
	case 10: min_level = 20; spell = "plague";	   break;
	default: min_level = 20; spell = "acid blast";     break;
	}

	if ( ch->level >= min_level )
	    break;
    }

    if ( ( sn = skill_lookup( spell ) ) < 0 )
	return FALSE;
    (*skill_table[sn].spell_fun) ( sn, ch->level, ch, victim,TARGET_CHAR);
    return TRUE;
}



bool spec_cast_undead( CHAR_DATA *ch )
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    char *spell;
    int sn;

    if ( ch->position != POS_FIGHTING )
	return FALSE;

    for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
    {
	v_next = victim->next_in_room;
	if ( victim->fighting == ch && number_bits( 2 ) == 0 )
	    break;
    }

    if ( victim == NULL )
	return FALSE;

    for ( ;; )
    {
	int min_level;

	switch ( number_bits( 4 ) )
	{
	case  0: min_level =  0; spell = "curse";          break;
	case  1: min_level =  3; spell = "weaken";         break;
	case  2: min_level =  6; spell = "chill touch";    break;
	case  3: min_level =  9; spell = "blindness";      break;
	case  4: min_level = 12; spell = "poison";         break;
	case  5: min_level = 15; spell = "energy drain";   break;
	case  6: min_level = 18; spell = "harm";           break;
	case  7: min_level = 21; spell = "teleport";       break;
	case  8: min_level = 20; spell = "plague";	   break;
	default: min_level = 18; spell = "harm";           break;
	}

	if ( ch->level >= min_level )
	    break;
    }

    if ( ( sn = skill_lookup( spell ) ) < 0 )
	return FALSE;
    (*skill_table[sn].spell_fun) ( sn, ch->level, ch, victim,TARGET_CHAR);
    return TRUE;
}


bool spec_executioner( CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    char *crime;

    if ( !IS_AWAKE(ch) || ch->fighting != NULL )
	return FALSE;

    crime = "";
    for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
    {
	v_next = victim->next_in_room;

	if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_KILLER) 
	&&   can_see(ch,victim))
	    { crime = "a KILLER"; break; }

	if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_THIEF) 
	&&   can_see(ch,victim))
	    { crime = "a THIEF"; break; }

	if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_ASSAULT) 
	&&   can_see(ch,victim))
	    { crime = "wanted for ASSAULT"; break; }

	if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_TREASON) 
	&&   can_see(ch,victim))
	    { crime = "wanted for TREASON"; break; }

	if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_OUTLAW) 
	&&   can_see(ch,victim))
	    { crime = "an OUTLAW"; break; }
    }

    if ( victim == NULL )
	return FALSE;

    sprintf( buf, "%s is %s!  PROTECT THE INNOCENT!  SLAY THE CRIMINAL!!!",
	victim->name, crime );
    REMOVE_BIT(ch->comm,COMM_NOSHOUT);
    do_function(ch, &do_yell, buf );
    multi_hit( ch, victim, TYPE_UNDEFINED );
    return TRUE;
}

			

bool spec_fido( CHAR_DATA *ch )
{
    OBJ_DATA *corpse;
    OBJ_DATA *c_next;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;

    if ( !IS_AWAKE(ch) )
	return FALSE;

    for ( corpse = ch->in_room->contents; corpse != NULL; corpse = c_next )
    {
	c_next = corpse->next_content;
	if ( corpse->item_type != ITEM_CORPSE_NPC )
	    continue;

	act( "$n savagely devours a corpse.", ch, NULL, NULL, TO_ROOM );
	for ( obj = corpse->contains; obj; obj = obj_next )
	{
	    obj_next = obj->next_content;
	    obj_from_obj( obj );
	    obj_to_room( obj, ch->in_room );
	}
	extract_obj( corpse );
	return TRUE;
    }

    return FALSE;
}



bool spec_guard( CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    CHAR_DATA *ech;
    char *crime;
    int max_evil;

    if ( !IS_AWAKE(ch) || ch->fighting != NULL )
	return FALSE;

    max_evil = 300;
    ech      = NULL;
    crime    = "";

    for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
    {
	v_next = victim->next_in_room;

	if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_KILLER) 
	&&   can_see(ch,victim))
	    { crime = "a KILLER"; break; }

	if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_THIEF) 
	&&   can_see(ch,victim))
	    { crime = "a THIEF"; break; }

	if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_ASSAULT) 
	&&   can_see(ch,victim))
	    { crime = "wanted for ASSAULT"; break; }

	if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_TREASON) 
	&&   can_see(ch,victim))
	    { crime = "wanted for TREASON"; break; }

	if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_OUTLAW) 
	&&   can_see(ch,victim))
	    { crime = "an OUTLAW"; break; }

	if ( victim->fighting != NULL
	&&   victim->fighting != ch
	&&   victim->alignment < max_evil )
	{
	    max_evil = victim->alignment;
	    ech      = victim;
	}
    }

    if ( victim != NULL && victim->level < 102)
    {
	sprintf( buf, "%s is %s!  PROTECT THE INNOCENT!!  SLAY THE CRIMINAL!!",
	    victim->name, crime );
 	REMOVE_BIT(ch->comm,COMM_NOSHOUT);
	do_function(ch, &do_yell, buf );
	multi_hit( ch, victim, TYPE_UNDEFINED );
	return TRUE;
    }

    if ( ech != NULL )
    {
	act( "$n screams 'TIME TO DIE CRIMINAL!!",
	    ch, NULL, NULL, TO_ROOM );
	multi_hit( ch, ech, TYPE_UNDEFINED );
	return TRUE;
    }

    return FALSE;
}



bool spec_janitor( CHAR_DATA *ch )
{
    OBJ_DATA *trash;
    OBJ_DATA *trash_next;

    if ( !IS_AWAKE(ch) )
	return FALSE;

    for ( trash = ch->in_room->contents; trash != NULL; trash = trash_next )
    {
	trash_next = trash->next_content;
	if ( !IS_SET( trash->wear_flags, ITEM_TAKE ) || !can_loot(ch,trash))
	    continue;
	if ( trash->item_type == ITEM_DRINK_CON
	||   trash->item_type == ITEM_TRASH
	||   trash->cost < 10 )
	{
	    act( "$n picks up some trash.", ch, NULL, NULL, TO_ROOM );
	    obj_from_room( trash );
	    obj_to_char( trash, ch );
	    return TRUE;
	}
    }

    return FALSE;
}



bool spec_mayor( CHAR_DATA *ch )
{
    static const char open_path[] =
	"W3a3003b33000c111d0d111Oe333333Oe22c222112212111a1S.";

    static const char close_path[] =
	"W3a3003b33000c111d0d111CE333333CE22c222112212111a1S.";

    static const char *path;
    static int pos;
    static bool move;

    if ( !move )
    {
	if ( time_info.hour ==  6 )
	{
	    path = open_path;
	    move = TRUE;
	    pos  = 0;
	}

	if ( time_info.hour == 20 )
	{
	    path = close_path;
	    move = TRUE;
	    pos  = 0;
	}
    }

    if ( ch->fighting != NULL )
	return spec_cast_mage( ch );
    if ( !move || ch->position < POS_SLEEPING )
	return FALSE;

    switch ( path[pos] )
    {
    case '0':
    case '1':
    case '2':
    case '3':
	move_char( ch, path[pos] - '0', FALSE );
	break;

    case 'W':
	ch->position = POS_STANDING;
	act( "$n awakens and groans loudly.", ch, NULL, NULL, TO_ROOM );
	break;

    case 'S':
	ch->position = POS_SLEEPING;
	act( "$n lies down and falls asleep.", ch, NULL, NULL, TO_ROOM );
	break;

    case 'a':
	act( "$n says 'Hello Honey!'", ch, NULL, NULL, TO_ROOM );
	break;

    case 'b':
	act( "$n says 'What a view!  I must do something about that dump!'",
	    ch, NULL, NULL, TO_ROOM );
	break;

    case 'c':
	act( "$n says 'Vandals!  Youngsters have no respect for anything!'",
	    ch, NULL, NULL, TO_ROOM );
	break;

    case 'd':
	act( "$n says 'Good day, citizens of Palanthas!'", ch, NULL, NULL, TO_ROOM );
	break;

    case 'e':
	act( "$n says 'I hereby declare the city of Palanthas open!'",
	    ch, NULL, NULL, TO_ROOM );
	break;

    case 'E':
	act( "$n says 'I hereby declare the city of Palanthas closed!'",
	    ch, NULL, NULL, TO_ROOM );
	break;

    case 'O':
/*	do_function(ch, &do_unlock, "gate" ); */
	do_function(ch, &do_open, "gate" );
	break;

    case 'C':
	do_function(ch, &do_close, "gate" );
/*	do_function(ch, &do_lock, "gate" ); */
	break;

    case '.' :
	move = FALSE;
	break;
    }

    pos++;
    return FALSE;
}



bool spec_poison( CHAR_DATA *ch )
{
    CHAR_DATA *victim;

    if ( ch->position != POS_FIGHTING
    || ( victim = ch->fighting ) == NULL
    ||   number_percent( ) > 2 * ch->level )
	return FALSE;

    act( "You bite $N!",  ch, NULL, victim, TO_CHAR    );
    act( "$n bites $N!",  ch, NULL, victim, TO_NOTVICT );
    act( "$n bites you!", ch, NULL, victim, TO_VICT    );
    spell_poison( gsn_poison, ch->level, ch, victim,TARGET_CHAR);
    return TRUE;
}



bool spec_thief( CHAR_DATA *ch )
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    long steel,gold;

    if ( ch->position != POS_STANDING )
	return FALSE;

    for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
    {
	v_next = victim->next_in_room;

	if ( IS_NPC(victim)
	||   victim->level >= LEVEL_IMMORTAL
	||   number_bits( 5 ) != 0 
	||   !can_see(ch,victim))
	    continue;

	if ( IS_AWAKE(victim) && number_range( 0, ch->level ) == 0 )
	{
	    act( "You discover $n's hands in your wallet!",
		ch, NULL, victim, TO_VICT );
	    act( "$N discovers $n's hands in $S wallet!",
		ch, NULL, victim, TO_NOTVICT );
	    return TRUE;
	}
	else
	{
	    steel = victim->steel * UMIN(number_range(1,20),ch->level / 2) / 100;
	    steel = UMIN(steel, ch->level * ch->level * 10 );
	    ch->steel     += steel;
	    victim->steel -= steel;
	    gold = victim->gold * UMIN(number_range(1,20),ch->level/2)/100;
	    gold = UMIN(gold,ch->level*ch->level * 25);
	    ch->gold	+= gold;
	    victim->gold -= gold;
	    return TRUE;
	}
    }

    return FALSE;
}

bool spec_dark_magic( CHAR_DATA *ch ) 
{ 
   CHAR_DATA *victim; 
   CHAR_DATA *v_next; 

   if ( ch->position != POS_FIGHTING ) 
    return FALSE; 

   for ( victim = ch->in_room->people; victim != NULL; victim = v_next ) 
   { 
    v_next = victim->next_in_room; 
    if ( victim->fighting == ch && number_bits( 2 ) == 0 ) 
        break; 
   } 

   if ( victim == NULL ) 
    return FALSE; 

   if (ch->position == POS_STUNNED) 
       return FALSE; 


    switch ( number_bits( 4 ) ) 
    { 

   case 0: 
    if (IS_AFFECTED(ch, AFF_BLIND)) 
    { 
    act("$n's eyes flare bright {bblue{x for a moment.", 
                  ch,NULL,NULL,TO_ROOM); 
    spell_cure_blindness( skill_lookup( "cure blindness" ), 
                  ch->level, ch, ch, TAR_CHAR_SELF ); 
       } 
       else if (IS_AFFECTED(ch, AFF_CURSE)) 
       { 
           act("$n's eyes flare black for a moment.", 
                                        ch,NULL,NULL,TO_ROOM); 
           spell_remove_curse( skill_lookup("remove curse" ), 
                                         ch->level, ch, ch, TAR_CHAR_SELF ); 
        } 
        else if (!IS_AFFECTED(ch, AFF_SANCTUARY)) 
        { 
            act("$n's eyes glow bright {Wwhite{x for a moment.", 
                                          ch,NULL, NULL, TO_ROOM); 
            spell_sanctuary( skill_lookup("sanctuary" ), 
                                          ch->level, ch, ch, TAR_CHAR_SELF ); 
        } 
        else if (!IS_AFFECTED(ch, AFF_HASTE)) 
        { 
             act("$n focuses on endurance for a moment.", ch, NULL, NULL, TO_ROOM); 
             spell_haste( skill_lookup("haste" ), 
                                           ch->level, ch, ch, TAR_CHAR_SELF ); 
        } 

        return TRUE; 

        case  1: 
             if (!IS_AFFECTED(victim, AFF_POISON)) 
             { 
                 act("$n says, '!Your blood shall be a pleasant sacrifice!", 
                                             ch, NULL, NULL, TO_ROOM); 
                 spell_poison( skill_lookup("poison" ), 
                                             ch->level, ch, victim, TAR_CHAR_SELF ); 
              } 
              else if (!IS_AFFECTED(victim, AFF_CURSE) ) 
              { 
                  act("$n's eyes flash {dblack{x for a moment.", 
                                              ch, NULL, NULL, TO_ROOM); 
                  spell_curse( skill_lookup("curse" ), 
                                              ch->level, ch, victim, TAR_CHAR_SELF ); 
              } 
              else if (IS_AFFECTED(victim, AFF_SANCTUARY)) 
              { 
                 act("$n says, '!Your deaths are at hand, flee while you still can!", 
                                              ch, NULL, NULL, TO_ROOM); 
                 spell_dispel_magic( skill_lookup("dispel magic" ), 
                                              ch->level, ch, victim, TAR_CHAR_SELF ); 
               } 
               return TRUE; 

               case 2: 
                   if (ch->hit < (ch->max_hit * .50 ) ) 
                   { 
                       act("$n says, !Pathetic fool, you cannot hurt me!", 
                                               ch, NULL, NULL, TO_ROOM); 
                       spell_heal( skill_lookup("heal" ), 
                                               ch->level, ch, ch, TAR_CHAR_SELF); 
                    } 
                    else if (IS_AFFECTED(ch, AFF_PLAGUE)) 
                    { 
                    act("$n says, !Disease means nothing to me fool!", 
                                                ch, NULL, NULL, TO_ROOM); 
                    spell_cure_disease( skill_lookup("cure disease" ), 
                                                ch->level, ch, ch, TAR_CHAR_SELF ); 
                    } 
                    if (ch->hit < (ch->max_hit * .25 ) ) 
                    { 
                        act("$n says, !Arrgghhh you are indead powerfull!", 
                                                  ch, NULL, NULL, TO_ROOM); 
                        spell_colour_spray( skill_lookup("colour spray" ), 
                                                  ch->level, ch, victim, TAR_CHAR_SELF ); 
                     } 
                     return TRUE; 

                   case 3: 
                       if (!IS_AFFECTED(victim, AFF_PLAGUE)) 
                       { 
                           act("$n's eyes flash a brilliant {Ggreen{x", 
                                                  ch, NULL, NULL, TO_ROOM); 
                           spell_plague( skill_lookup("plague" ), 
                                                  ch->level, ch, victim, TAR_CHAR_SELF ); 
                        } 
                        else if (ch->hit < (ch->max_hit * .10 ) ) 
                        { 
                            do_flee(ch,""); 
                        } 
                        else if (IS_AFFECTED(ch, AFF_POISON)) 
                        { 
                            act("$n's eyes flash {Yyellow{x", ch, NULL, NULL, TO_ROOM); 
                            spell_cure_poison( skill_lookup("cure poison" ), 
                                                   ch->level, ch, ch, TAR_CHAR_SELF ); 
                        } 
                        else if (!IS_AFFECTED(victim, AFF_BLIND)) 
                        { 
                            act("$n's eyes glaze over", ch, NULL, NULL, TO_ROOM); 
                            spell_blindness( skill_lookup("blindness" ), 
                                                   ch->level, ch, victim, TAR_CHAR_SELF ); 
                        } 


                        return TRUE; 


        } 

       return FALSE; 
}

bool spec_angry( CHAR_DATA *ch )
{


    if ( ch->fighting != NULL )
	return FALSE;

    if ( ch->position != POS_STANDING )
	return FALSE;

    switch(number_bits(7))
    {
      case 0:
        act("$n grits $s teeth and shakes a clenched fist in the air.", ch, NULL,
NULL, TO_ROOM );
        break;
      case 1:
        act("$n mutters a curse under $s breath.", ch, NULL, NULL, TO_ROOM );
        break;
      case 2:
        act("$n throws back $s head and screams in wordless fury.", ch, NULL, NULL, TO_ROOM );
        break;
      case 3:
        act("$n turns red and curses the gods who have wronged $m.", ch, NULL, NULL, TO_ROOM );
        break;
      default:
        break;
    }

    return FALSE;
}

bool spec_animal( CHAR_DATA *ch )
{


    if ( ch->fighting != NULL )
	return FALSE;

    if ( ch->position != POS_STANDING )
	return FALSE;

  switch(number_bits(7))
  {
      case 0:
        act("$n sniffs the ground, looking for something good to eat.", ch, NULL, NULL, TO_ROOM );
        break;
      case 1:
        act("$n licks $s paws." , ch, NULL, NULL, TO_ROOM );
        break;
      case 2:
        act("$n looks at you curiously.", ch, NULL, NULL, TO_ROOM );
        break;
      case 3:
        act("$n scratches behind $s left ear.", ch, NULL, NULL, TO_ROOM );
        break;
      default:
          break;
    }

    return FALSE;
}

bool spec_belligerent( CHAR_DATA *ch )
{
  char buf[100], buf2[100];

  if ( ch->fighting != NULL )
	return FALSE;

  if ( ch->position != POS_STANDING )
	return FALSE;
  
  switch(number_bits(8))
    {
      case 1:
        break;
      case 2:
        sprintf(buf, "What do you think you're looking at?");
        act("$n scowls at you.", ch, NULL, NULL, TO_ROOM);
        do_say(ch, buf);
        break;
      case 3:
        sprintf(buf, "Are you looking at me?");
        sprintf(buf2, "I don't see anyone else, so you must be looking at me.");
        do_say(ch, buf);
        act("$n looks around.", ch, NULL, NULL, TO_ROOM);
        do_say(ch, buf2);
        break;
      case 4:
        sprintf(buf, "Why don't you watch where you're going?!");
        act("$n lunges in front of you.",  ch, NULL, NULL, TO_ROOM);
        do_say(ch, buf);
        break;
      case 5:
        act("$n glances at you and mutters something under $s breath.", ch, NULL, NULL, TO_ROOM);
        break;
    }

    return FALSE;
}

bool spec_dolphin( CHAR_DATA *ch )
{


    if ( ch->fighting != NULL )
	return FALSE;

    if ( ch->position != POS_STANDING )
	return FALSE;

    switch(number_bits(7))
    {
      case 0:
        act("$n jumps out of the water.", ch, NULL, NULL, TO_ROOM );
        break;
      case 1:
        act("$n clicks and chatters." , ch, NULL, NULL, TO_ROOM );
        break;
      case 2:
        act("$n quickly swims in a circle.", ch, NULL, NULL, TO_ROOM );
        break;
      case 3:
        act("$n swims gracefully in the water.", ch, NULL, NULL, TO_ROOM );
        break;
      default:
          break;
    }

    return FALSE;
}

bool spec_evilguard( CHAR_DATA *ch )
{


    if ( ch->fighting != NULL )
	return FALSE;

    if ( ch->position != POS_STANDING )
	return FALSE;

    switch(number_bits(7))
    {
      case 0:
        act("$n discreetly nods and looks away as a well-known thug slips some coins into $s hand.", ch, NULL, NULL, TO_ROOM );
        break;
      case 1:
        act("$n glares at a passerby, uncertain of their loyalties.", ch, NULL, NULL, TO_ROOM );
        break;
      case 2:
        act("$n scans the area, looking for foolish rebels.", ch, NULL, NULL, TO_ROOM );
        break;
      case 3:
        do_say( ch, "I will slit the throat of any who dares to disrespect our laws.");
        break;
      default:
        break;
    }

    return FALSE;
}

bool spec_ghost( CHAR_DATA *ch )
{


    if ( ch->fighting != NULL )
	return FALSE;

    if ( ch->position != POS_STANDING )
	return FALSE;

    switch(number_bits(7)) 
    {
      case 0:
        interpret(ch, "groan");
        break;
      case 1:
        interpret(ch, "threat");
        break;
      case 2:
        interpret(ch, "moan");
        break;
      default:
        break;
    }

    return FALSE;
}

bool spec_happy( CHAR_DATA *ch )
{
  char buf[MAX_STRING_LENGTH];

    if ( ch->fighting != NULL )
	return FALSE;

    if ( ch->position != POS_STANDING )
	return FALSE;

    switch(number_bits(7))
    {
      case 0:
        act("$n smiles happily.", ch, NULL, NULL, TO_ROOM );
        break;
      case 1:
        act("$n greets you warmly.", ch, NULL, NULL, TO_ROOM );
        break;
      case 2:
        act("$n laughs merrily.", ch, NULL, NULL, TO_ROOM );
        break;
      case 3:
        sprintf( buf, "Good journeys to you, stranger.");
        do_say (ch, buf);
        break;
      default:
        break;
    }

    return FALSE;
}

bool spec_priesttakhisis( CHAR_DATA *ch )
{

    if ( ch->fighting != NULL )
	return FALSE;

    if ( ch->position != POS_STANDING )
	return FALSE;

  switch(number_bits(8)) 
  {
 	case 0:
 	     REMOVE_BIT(ch->comm, COMM_NOSHOUT);
 	     do_yell(ch, "Never will you escape Her, you are Her children!");
 	     break;

 	case 1:
  	     do_say(ch, "I hear Her calling me from the Abyss.  I go to obey.");
 	     break;

  	case 2:
   	     REMOVE_BIT(ch->comm, COMM_NOSHOUT);
   	     do_yell(ch, "You will be a slave to her will forever!");
   	     break;

  	case 3:
   	     do_say(ch, "Every bit of me belongs to Her.");
   	     break;

  	case 4:
   	     do_say(ch, "The things She offers cannot be refused.");
   	     break;

  	case 5:
    	     do_say(ch, "To be in Her presence is all I desire now.");
    	     break;
  
  	case 6:
    	     do_say(ch, "She has fulfilled all of my secret dreams.");
   	     break;

  	case 7:
   	     do_say(ch, "If you doubt me, visit Her temple, seek out Her followers.");
   	     break;

  	case 8:
   	     do_say(ch, "The scars on my body and in my mind have been a small price to pay.");
   	     break;

  	case 9:
   	    REMOVE_BIT(ch->comm, COMM_NOSHOUT);
   	    do_yell(ch, "Go to Her, for She loves all of Her little ones.");
   	    break;

   	default:
	    break;
      }

    return FALSE;
}

bool spec_jester( CHAR_DATA *ch )
{
  char buf[100], buf2[100], buf3[100], buf4[100];

    if ( ch->fighting != NULL )
	return FALSE;

    if ( ch->position != POS_STANDING )
	return FALSE;

    switch(number_bits(8))
    {
      case 0:
        break;
      case 1:
        act("$n sings a bawdy song.", ch, NULL, NULL, TO_ROOM);
        break;
      case 2:
        /* joke: */ /*this code line wraps, nothing to do about it */
        sprintf(buf, "Why may a caudled fillhorse be deemed the brother to a hiren");
        sprintf(buf2, "candle in the night?");
        /* and its wacky punchline */
        sprintf(buf3, 
            "Because a candle may be greased, yet a fillhorse be without");
        sprintf(buf4, "a fat argier.");
        do_say(ch, buf);
        do_say(ch, buf2);
        act("$n pauses dramatically.", ch, NULL, NULL, TO_ROOM);
        do_say(ch, buf3);
        do_say(ch, buf4);
        break;
      case 3: 
        act("$n chuckles nervously.", ch, NULL, NULL, TO_ROOM);
        break;
      case 4:
        act("$n capers around the room.", ch, NULL, NULL, TO_ROOM);
        break;
      case 5:
        act("$n sobs quietly, silently hating $s job.", ch, NULL, NULL, TO_ROOM);
        break; 
   }

    return FALSE;
}

bool spec_merchant( CHAR_DATA *ch )
{

    if ( ch->fighting != NULL )
	return FALSE;

    if ( ch->position != POS_STANDING )
	return FALSE;

    switch(number_bits(8)) 
    {
      case 0:
        REMOVE_BIT(ch->comm, COMM_NOSHOUT);
        do_yell(ch, "Quality merchandise!  Get your merchandise here!");
        break;
      case 1:
        REMOVE_BIT(ch->comm, COMM_NOSHOUT);
        do_yell(ch, "Merchandise!  Get your merchandise here, cheap!");
        break;
      default:
        break;
    }

    return FALSE;
}

bool spec_prophet( CHAR_DATA *ch )
{

    if ( ch->fighting != NULL )
	return FALSE;

    if ( ch->position != POS_STANDING )
	return FALSE;

    switch(number_bits(8))
    {
      case 0:
        act("$n offers you literature about the end of the world.", ch, NULL, NULL, TO_ROOM );
        break;
      case 1:
        act("$n screams at you about the impending doom of all the Realms.", ch, NULL, NULL,TO_ROOM );
        break;
      case 2:
        act("$n points at you.", ch, NULL, NULL, TO_ROOM );
        do_say(ch, "Repent, sinner!  Soon you will look upon the face of your god!");
        break;
      case 3:
        REMOVE_BIT(ch->comm, COMM_NOSHOUT);
        do_yell(ch, "The end is near!  Repent, sinners!");
        break;
      default:
        break;
    }

    return FALSE;
}

bool spec_sad( CHAR_DATA *ch )
{

    if ( ch->fighting != NULL )
	return FALSE;

    if ( ch->position != POS_STANDING )
	return FALSE;

     switch(number_bits(7))
     {
       case 0:
         act("$n sighs sadly.", ch, NULL, NULL, TO_ROOM );
         break;
       case 1:
         act("$n hangs $s head in misery.", ch, NULL, NULL, TO_ROOM );
         break;
       case 2:
         act("$n sniffs pitifully.", ch, NULL, NULL, TO_ROOM );
         break;
       case 3:
         act("$n wipes a solitary tear from $s eye.",  ch, NULL, NULL, TO_ROOM );
         break;
       case 4:
         act("$n sobs quietly.", ch, NULL, NULL, TO_ROOM );
         break;
       default:
         break;
    }

    return FALSE;
}

bool spec_rloader( CHAR_DATA *ch )
{
	OBJ_DATA *corpse;
	OBJ_DATA *c_next;
	OBJ_DATA *v_corpse = NULL;
	OBJ_DATA *obj;
	int chance = 0;
	int cha = 0;

	if(ch == NULL)
		return FALSE;

      for ( corpse = ch->in_room->contents; corpse != NULL; corpse = c_next )
      {
		c_next = corpse->next_content;
		if ( corpse->item_type != ITEM_CORPSE_NPC )
	    		continue;
		v_corpse = corpse;
      }

	if(v_corpse == NULL)
		return FALSE;

	cha = get_curr_stat(ch,STAT_CHR);

	if(cha == 25)
		chance = 80;
	else if( (cha >= 20) && (cha <= 24) )
		chance = 70;
	else if( (cha >= 15) && (cha <= 19) )
		chance = 60;
	else if( (cha >= 10) && (cha <= 14) )
		chance = 50;
	else if( (cha >= 6) && (cha <= 9) )
		chance = 40;
	else
		chance = 20;

   if(number_percent() >= chance)
   {
	switch(number_range(0,40))
	{
		case 1:
			obj = create_object( get_obj_index(OBJ_VNUM_LOADER_APPLE), ch->level + 10 );
			obj_to_obj( obj, v_corpse );
			break;
		case 2:
			obj = create_object( get_obj_index(OBJ_VNUM_LOADER_PAPER), ch->level + 10 );
			obj_to_obj( obj, v_corpse );
			break;
		case 3:
			obj = create_object( get_obj_index(OBJ_VNUM_LOADER_POUCH), ch->level + 10 );
			obj_to_obj( obj, v_corpse );
			break;
		case 4:
			obj = create_object( get_obj_index(OBJ_VNUM_LOADER_SUNROD), ch->level + 10 );
			obj_to_obj( obj, v_corpse );
			break;
		case 5:
			obj = create_object( get_obj_index(OBJ_VNUM_LOADER_BILE), ch->level + 10 );
			obj_to_obj( obj, v_corpse );
			break;
		case 6:
			obj = create_object( get_obj_index(OBJ_VNUM_LOADER_DAGGER), ch->level + 10 );
			obj_to_obj( obj, v_corpse );
			break;
		case 7:
			obj = create_object( get_obj_index(OBJ_VNUM_LOADER_BREAD), ch->level + 10 );
			obj_to_obj( obj, v_corpse );
			break;
		case 8:
			obj = create_object( get_obj_index(OBJ_VNUM_LOADER_WATER), ch->level + 10 );
			obj_to_obj( obj, v_corpse );
			break;
		case 9:
			obj = create_object( get_obj_index(OBJ_VNUM_LOADER_BBERRIES), ch->level + 10 );
			obj_to_obj( obj, v_corpse );
			break;
		case 10:
			obj = create_object( get_obj_index(OBJ_VNUM_LOADER_CLOTHES), ch->level + 10 );
			obj_to_obj( obj, v_corpse );
			break;
		case 11:
			obj = create_object( get_obj_index(OBJ_VNUM_LOADER_ROPE), ch->level + 10 );
			obj_to_obj( obj, v_corpse );
			break;
		case 12:
			obj = create_object( get_obj_index(OBJ_VNUM_LOADER_ORB), ch->level + 10 );
			obj_to_obj( obj, v_corpse );
			break;
		case 13:
			obj = create_object( get_obj_index(OBJ_VNUM_LOADER_PIPE), ch->level + 10 );
			obj_to_obj( obj, v_corpse );
			break;
		case 14:
			obj = create_object( get_obj_index(OBJ_VNUM_LOADER_GEAR), ch->level + 10 );
			obj_to_obj( obj, v_corpse );
			break;
		case 15:
			obj = create_object( get_obj_index(OBJ_VNUM_LOADER_CHEESE), ch->level + 10 );
			obj_to_obj( obj, v_corpse );
			break;
		case 16:
			obj = create_object( get_obj_index(OBJ_VNUM_LOADER_BEEF), ch->level + 10 );
			obj_to_obj( obj, v_corpse );
			break;
		case 17:
			obj = create_object( get_obj_index(OBJ_VNUM_LOADER_RUNEST), ch->level + 10 );
			obj_to_obj( obj, v_corpse );
			break;
		case 18:
			obj = create_object( get_obj_index(OBJ_VNUM_LOADER_SCROLL), ch->level + 10 );
			obj_to_obj( obj, v_corpse );
			break;
		case 19:
			obj = create_object( get_obj_index(OBJ_VNUM_LOADER_POTION), ch->level + 10 );
			obj_to_obj( obj, v_corpse );
			break;
		case 20:
			obj = create_object( get_obj_index(OBJ_VNUM_LOADER_POLEARM), ch->level + 10 );
			obj_to_obj( obj, v_corpse );
			break;
		case 21:
			obj = create_object( get_obj_index(OBJ_VNUM_LOADER_GOLD), ch->level + 10 );
			obj_to_obj( obj, v_corpse );
			break;
		case 22:
			obj = create_object( get_obj_index(OBJ_VNUM_LOADER_RAGS), ch->level + 10 );
			obj_to_obj( obj, v_corpse );
			break;
		case 23:
			obj = create_object( get_obj_index(OBJ_VNUM_LOADER_CRYSTAL), ch->level + 10 );
			obj_to_obj( obj, v_corpse );
			break;
		case 24:
			obj = create_object( get_obj_index(OBJ_VNUM_LOADER_BEER), ch->level + 10 );
			obj_to_obj( obj, v_corpse );
			break;
		case 25:
			obj = create_object( get_obj_index(OBJ_VNUM_LOADER_JACKET), ch->level + 10 );
			obj_to_obj( obj, v_corpse );
			break;
		case 26:
			obj = create_object( get_obj_index(OBJ_VNUM_LOADER_BERRY), ch->level + 10 );
			obj_to_obj( obj, v_corpse );
			break;
		case 27:
			obj = create_object( get_obj_index(OBJ_VNUM_LOADER_BOX), ch->level + 10 );
			obj_to_obj( obj, v_corpse );
			break;
		case 28:
			obj = create_object( get_obj_index(OBJ_VNUM_LOADER_GLOVES), ch->level +  10 );
			obj_to_obj( obj, v_corpse );
			break;
		case 29:
			obj = create_object( get_obj_index(OBJ_VNUM_LOADER_SYHIDD), ch->level +  10 );
			obj_to_obj( obj, v_corpse );
			break;
		case 30:
			obj = create_object( get_obj_index(OBJ_VNUM_LOADER_RAYD), ch->level +  10 );
			obj_to_obj( obj, v_corpse );
			break;
		case 31:
			obj = create_object( get_obj_index(OBJ_VNUM_LOADER_STEEL), ch->level +  10 );
			obj_to_obj( obj, v_corpse );
			break;
		case 32:
			obj = create_object( get_obj_index(OBJ_VNUM_LOADER_PALSURP), ch->level +  10 );
			obj_to_obj( obj, v_corpse );
			break;
		case 33:
			obj = create_object( get_obj_index(OBJ_VNUM_LOADER_LIGHT), ch->level +  10 );
			obj_to_obj( obj, v_corpse );
			break;
		case 34:
			obj = create_object( get_obj_index(OBJ_VNUM_LOADER_RATIONS), ch->level +  10 );
			obj_to_obj( obj, v_corpse );
			break;
		case 35:
			obj = create_object( get_obj_index(OBJ_VNUM_LOADER_HOOK), ch->level +  10 );
			obj_to_obj( obj, v_corpse );
			break;
		case 36:
			obj = create_object( get_obj_index(OBJ_VNUM_LOADER_TRASH), ch->level +  10 );
			obj_to_obj( obj, v_corpse );
			break;
		case 37:
			obj = create_object( get_obj_index(OBJ_VNUM_LOADER_BANANA), ch->level +  10 );
			obj_to_obj( obj, v_corpse );
			break;
		case 38:
			obj = create_object( get_obj_index(OBJ_VNUM_LOADER_BELT), ch->level +  10 );
			obj_to_obj( obj, v_corpse );
			break;
		case 39:
			obj = create_object( get_obj_index(OBJ_VNUM_LOADER_PANTS), ch->level +  10 );
			obj_to_obj( obj, v_corpse );
			break;
		case 40:
			obj = create_object( get_obj_index(OBJ_VNUM_LOADER_WETST), ch->level +  10 );
			obj_to_obj( obj, v_corpse );
			break;
		default:
			break;
	}
	return TRUE;
   }
   return FALSE;
}

bool spec_ferrysol( CHAR_DATA *ch )
{

  switch(ch->spec_state)
  {
    case 0:
      do_say(ch, "All aboard! The ship departs in one minute!");
      do_unlock(ch, "north");
      do_open(ch, "north");
      break;
    case 1: case 2: case 3: case 4:
      break;
    case 5:
	do_say(ch, "The ship is departing soon for Solamnia! All aboard!");
	do_unlock(ch, "north");
	do_open(ch, "north");
	break;   
    case 6: case 7: case 8: case 9:
      break;
    case 10:
       do_say(ch, "Last call! The ship is departing now!");
       do_unlock(ch, "north");
       do_open(ch,"north");
       do_north(ch, "");
       break;
    case 11:
       do_say(ch, "Prepare the sails and batten down the hatches.");
       do_close(ch, "south");
       do_lock(ch, "south");
       break;
    case 12: case 13: case 14: case 15: case 16:
    case 17: case 18: case 19: case 20: case 21:
    case 22: case 23: case 24: case 25: case 26:
    case 27: case 28: case 29: case 30: case 31:
        switch(number_bits(4))
        {
          case 0:
            act("The boat rocks gently on the waves.", ch, NULL, NULL, TO_ROOM);
            break;
          case 1:
            act("A crisp ocean breeze ruffles your hair.", ch, NULL, NULL, TO_ROOM);
            break;          
          case 2:
            act("$n silently stares towards the horizon.", ch, NULL, NULL, TO_ROOM);
            break;
          case 3:
            act("The boat rocks gently on the waves.", ch, NULL, NULL, TO_ROOM);
            break;
          case 4:
            act("A large wave CRASHES on the ship's prow.", ch, NULL, NULL, TO_ROOM);
            break;
          default:
	    break;
          }
       break;
       case 32:
        do_say(ch, "We have arrived, travelers. Welcome to Solamnia." );
        do_unlock(ch, "north");
        do_open(ch, "north");
	break;
       case 33:
        do_unlock(ch, "north");
	do_open(ch, "north");
        do_north(ch, "");
        break;
       case 34:
        do_say(ch, "All aboard! The ship departs in one minute!");
        do_unlock(ch, "south");
        do_open(ch, "south");
        break;
       case 35: case 36: case 37: case 38:
        break;
       case 39:
        do_say(ch, "All aboard! The ship departs for Abanasinia soon!");
	do_unlock(ch, "south");
	do_open(ch, "south");
        break;
       case 40: case 41: case 42: case 43:
        break;
       case 44:
        do_say(ch, "Last call! The ship is departing!");
        do_unlock(ch, "south");
        do_open(ch, "south");
        do_south(ch, "");
	break;
       case 45:
        do_close(ch, "north");
        do_lock(ch, "north");
       break;
     case 46: case 47: case 48: case 49: case 50:
     case 51: case 52: case 53: case 54: case 55:
     case 56: case 57: case 58: case 59: case 60:
     case 61: case 62: case 63: case 64: case 65:
        switch(number_bits(4))
     {  
        case 0:
            act("The boat rocks gently on the waves.", ch, NULL, NULL, TO_ROOM);
            break;
          case 1:
            act("A crisp ocean breeze ruffles your hair.", ch, NULL, NULL, TO_ROOM);
            break;          
          case 2:
            act("$n silently stares towards the horizon.", ch, NULL, NULL, TO_ROOM);
            break;
          case 3:
            act("The boat rocks gently on the waves.", ch, NULL, NULL, TO_ROOM);
            break;
          case 4:
            act("A large wave CRASHES on the ship's prow.", ch, NULL, NULL, TO_ROOM);
            break;
          default:
           break;
          }  
          break;
        case 66:
          do_say(ch, "We have arrived, travelers! Welcome to Abanasinia.");
          do_unlock(ch, "south");
          do_open(ch, "south");
          break;
        case 67:
          do_unlock(ch, "south");
	  do_open(ch, "south");
          do_south(ch, "");
          ch->spec_state = -1;
          break;
        default:
          bug("Bad ferrysol spec_state", 0);
          break;
	  break;
     }
    ch->spec_state++;
    return FALSE;
}

bool spec_ferrypal( CHAR_DATA *ch )
{

  switch(ch->spec_state)
  {
    case 0:
      do_say(ch, "All aboard! The ship departs in one minute!");
      do_unlock(ch, "east");
      do_open(ch, "east");
      break;
    case 1: case 2: case 3: case 4:
      break;
    case 5:
	do_say(ch, "The ship is departing soon for Mt. Nevermind! All aboard!");
	do_unlock(ch, "east");
	do_open(ch, "east");
	break;   
    case 6: case 7: case 8: case 9:
      break;
    case 10:
       do_say(ch, "Last call! The ship is departing now!");
       do_unlock(ch, "east");
       do_open(ch,"east");
       do_east(ch, "");
       break;
    case 11:
       do_say(ch, "Prepare the sails and batten down the hatches.");
       do_close(ch, "west");
       do_lock(ch, "west");
       break;
    case 12: case 13: case 14: case 15: case 16:
    case 17: case 18: case 19: case 20: case 21:
    case 22: case 23: case 24: case 25: case 26:
    case 27: case 28: case 29: case 30: case 31:
        switch(number_bits(4))
        {
          case 0:
            act("The boat slices through the water.", ch, NULL, NULL, TO_ROOM);
            break;
          case 1:
            act("A flurry of silver streaks the ocean's blue.", ch, NULL, NULL, TO_ROOM);
            break;          
          case 2:
            act("$n paces restlessly and then looks up towards the sky.", ch, NULL, NULL, TO_ROOM);
            break;
          case 3:
            act("The pointy bow breaks through a large wave.", ch, NULL, NULL, TO_ROOM);
            break;
          case 4:
            act("Seagulls call out loudly and swoop through the air.", ch, NULL, NULL, TO_ROOM);
            break;
          default:
	    break;
          }
       break;
       case 32:
        do_say(ch, "We have arrived, travelers. Welcome to Mt. Nevermind." );
        do_unlock(ch, "east");
        do_open(ch, "east");
	break;
       case 33:
        do_unlock(ch, "east");
	  do_open(ch, "east");
        do_east(ch, "");
        break;
       case 34:
        do_say(ch, "All aboard! The ship departs in one minute!");
        do_unlock(ch, "west");
        do_open(ch, "west");
        break;
       case 35: case 36: case 37: case 38:
        break;
       case 39:
        do_say(ch, "All aboard! The ship departs for Palanthas soon!");
	  do_unlock(ch, "west");
	  do_open(ch, "west");
        break;
       case 40: case 41: case 42: case 43:
        break;
       case 44:
        do_say(ch, "Last call! The ship is departing!");
        do_unlock(ch, "west");
        do_open(ch, "west");
        do_west(ch, "");
	break;
       case 45:
        do_close(ch, "east");
        do_lock(ch, "east");
       break;
     case 46: case 47: case 48: case 49: case 50:
     case 51: case 52: case 53: case 54: case 55:
     case 56: case 57: case 58: case 59: case 60:
     case 61: case 62: case 63: case 64: case 65:
        switch(number_bits(4))
     {  
        case 0:
            act("Sailors swab the decks and tend to the mast.", ch, NULL, NULL, TO_ROOM);
            break;
          case 1:
            act("The sun beats down relentlessly, hot and unforgiving.", ch, NULL, NULL, TO_ROOM);
            break;          
          case 2:
            act("$n silently stares towards the horizon.", ch, NULL, NULL, TO_ROOM);
            break;
          case 3:
            act("The waves lap along the side of the boat.", ch, NULL, NULL, TO_ROOM);
            break;
          case 4:
            act("A large wave CRASHES on the ship's prow.", ch, NULL, NULL, TO_ROOM);
            break;
          default:
           break;
          }  
          break;
        case 66:
          do_say(ch, "We have arrived, travelers! Welcome to Palanthas.");
          do_unlock(ch, "west");
          do_open(ch, "west");
          break;
        case 67:
          do_unlock(ch, "west");
	    do_open(ch, "west");
          do_west(ch, "");
          ch->spec_state = -1;
          break;
        default:
          bug("Bad ferrysol spec_state", 0);
          break;
	  break;
     }
    ch->spec_state++;
    return FALSE;
}

/*Spec that prevents non orgmembers from leaving into an org sector 
 *as long as a guard is present. This function has no guts, as code is
 *actually defined in move_char located in the act_move.c file.
 */

bool spec_orgguard (CHAR_DATA *ch)
{
 return FALSE;
}

/* Spec that allows players to change their hometown.
 * Code function located in act_comm.c in DO_SAY
 */
bool spec_hometownchanger (CHAR_DATA *ch)
{
 return FALSE;
}


/* Jorge spec code to mess with Celeste and Jen
*/
bool spec_jorge( CHAR_DATA *ch )
{
  char buf[100], buf2[100];

  if ( ch->fighting != NULL )
	return FALSE;

  if ( ch->position != POS_STANDING )
	return FALSE;
  
  switch(number_bits(8))
    {
      case 1:
        break;
      case 2:
        sprintf(buf, "cerrar la puerta del establo después de que se robaron el caballo");
        do_say(ch, buf);
        break;
      case 3:
        sprintf(buf, "Did you hear about the two car pile-up in the Walmart parking lot?");
        sprintf(buf2, "50 Mexicans died.");
        do_say(ch, buf);
        act("$n looks around.", ch, NULL, NULL, TO_ROOM);
        do_say(ch, buf2);
        break;
      case 4:
        sprintf(buf, "Hey Puto!");
        sprintf(buf2, "Follar Gringo!");
        do_say(ch, buf);
        act("$n smiles at you then mutters under his breath.", ch, NULL, NULL, TO_ROOM);
        do_say(ch, buf2);
        break;
      case 5:
        sprintf(buf, "Why were there only 5,000 mexican soldiers at the battle of Alamo?");
        sprintf(buf2, "They only had 2 vans.");
        do_say(ch, buf);
        act("$n looks down at his watch.", ch, NULL, NULL, TO_ROOM);
        do_say(ch, buf2);
        break;
      case 6:
        act("Jorge starts picking weeds out from the vegetable crop.", ch, NULL, NULL, TO_ROOM);
        break;
    }

    return FALSE;
}

