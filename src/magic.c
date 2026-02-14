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
#include "interp.h"
#include "magic.h"
#include "recycle.h"

/*
 * Local functions.
 */
void say_spell		args( ( CHAR_DATA *ch, int sn ) );
void gesture_spell   	args( ( CHAR_DATA *ch, int sn, void *vo, int target ) );

/* imported functions */
bool remove_obj      	args( ( CHAR_DATA *ch, int iWear, bool fReplace ) );
void wear_obj		args( ( CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace ) );
void check_advance		( CHAR_DATA *ch, CHAR_DATA *victim );



/*
 * Lookup a skill by name.
 */
int skill_lookup( const char *name )
{
    int sn;

    for ( sn = 0; sn < MAX_SKILL; sn++ )
    {
	if ( skill_table[sn].name == NULL )
	    break;
	if ( LOWER(name[0]) == LOWER(skill_table[sn].name[0])
	&&   !str_prefix( name, skill_table[sn].name ) )
	    return sn;
    }

    return -1;
}

int find_spell( CHAR_DATA *ch, const char *name )
{
    /* finds a spell the character can cast if possible */
    int sn, found = -1;

    if (IS_NPC(ch))
	return skill_lookup(name);

    for ( sn = 0; sn < MAX_SKILL; sn++ )
    {
	if (skill_table[sn].name == NULL)
	    break;
	if (LOWER(name[0]) == LOWER(skill_table[sn].name[0])
	&&  !str_prefix(name,skill_table[sn].name))
	{
	    if ( found == -1)
		found = sn;
	    if (ch->level >= skill_table[sn].skill_level[ch->class]
	    &&  ch->pcdata->learned[sn] > 0)
		    return sn;
	}
    }
    return found;
}



/*
 * Lookup a skill by slot number.
 * Used for object loading.
 */
int slot_lookup( int slot )
{
    extern bool fBootDb;
    int sn;

    if ( slot <= 0 )
	return -1;

    for ( sn = 0; sn < MAX_SKILL; sn++ )
    {
	if ( slot == skill_table[sn].slot )
	    return sn;
    }

    if ( fBootDb )
    {
	bug( "Slot_lookup: bad slot %d.", slot );
	abort( );
    }

    return -1;
}



/*
 * Utter mystical words for an sn.
 */
void say_spell( CHAR_DATA *ch, int sn )
{
    char buf  [MAX_STRING_LENGTH];
    char buf2 [MAX_STRING_LENGTH];
    CHAR_DATA *rch;
    char *pName;
    int iSyl;
    int length;

    struct syl_type
    {
	char *	old;
	char *	new;
    };

    static const struct syl_type syl_table[] =
    {
	{ " ",		" "						},
	{ "ar",		"abra"					},
	{ "au",		"kada"					},
	{ "bless",		"fido"					},
	{ "blind",		"nose"					},
      { "silence",    	"shhh.."        				},
      { "waterlungs",   "aquaticus gillus" 			},
      { "curse",		"blasphemous fool!"			},
      { "disintegrate", "ashes to ashes, dust to dust..."	},
	{ "bur",		"mosa"					},
	{ "cu",		"judi"					},
	{ "de",		"oculo"					},
	{ "en",		"unso"					},
	{ "light",		"dies"					},
	{ "lo",		"hi"						},
	{ "mor",		"zak"						},
	{ "move",		"sido"					},
	{ "ness",		"lacri"					},
	{ "ning",		"illa"					},
	{ "per",		"duda"					},
	{ "ra",		"gru"						},
	{ "fresh",		"ima"						},
	{ "re",		"candus"					},
	{ "son",		"sabru"					},
	{ "tect",		"infra"					},
	{ "tri",		"cula"					},
	{ "ven",		"nofo"					},
	{ "a", "a" }, { "b", "b" }, { "c", "q" }, { "d", "e" },
	{ "e", "z" }, { "f", "y" }, { "g", "o" }, { "h", "p" },
	{ "i", "u" }, { "j", "y" }, { "k", "t" }, { "l", "r" },
	{ "m", "w" }, { "n", "i" }, { "o", "a" }, { "p", "s" },
	{ "q", "d" }, { "r", "f" }, { "s", "g" }, { "t", "h" },
	{ "u", "j" }, { "v", "z" }, { "w", "x" }, { "x", "n" },
	{ "y", "l" }, { "z", "k" },
	{ "", "" }
    };

    buf[0]	= '\0';
    for ( pName = skill_table[sn].name; *pName != '\0'; pName += length )
    {
	for ( iSyl = 0; (length = strlen(syl_table[iSyl].old)) != 0; iSyl++ )
	{
	    if ( !str_prefix( syl_table[iSyl].old, pName ) )
	    {
		strcat( buf, syl_table[iSyl].new );
		break;
	    }
	}

	if ( length == 0 )
	    length = 1;
    }

    sprintf( buf2, "$n utters the words, '%s'.", buf );
    sprintf( buf,  "$n utters the words, '%s'.", skill_table[sn].name );

    for ( rch = ch->in_room->people; rch; rch = rch->next_in_room )
    {
	if ( rch != ch )
	    act((!IS_NPC(rch) && ch->class==rch->class) ? buf : buf2,
	        ch, NULL, rch, TO_VICT );
    }

    return;
}

void gesture_spell( CHAR_DATA *ch, int sn, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    char *spellName;
    int i;
    struct gesture_type
    {
        char *  name;
        char *  ch;
	char *  vict;
	char *  room;
    };
static const struct gesture_type gesture_table[] =
{
 { "fireball",
   "You thrust your hands forward as you conjure up a terrifying ball of fire!",
   "$n thrusts $s arms forward, sending a terrifying ball of fire spiraling towards you!",
   "$n thrusts $s arms foward, conjuring up a terrifying ball of fire!" },

 { "lightning bolt",
   "Lightning crackles from your body as you send a deadly bolt cascading towards $N!",
   "Lightning crackles from $n's body and sends a deadly bolt cascading towards you!",
   "Lightning crackles from $n's body and sends a deadly bolt cascading towards $N!" },

 { "acid blast",
   "With a wave of your arm, you conjure forth a column of acid and send it hurling towards $N!",
   "With a wave of $s arm, $n conjures forth a column of acid and sends it hurling towards you!",
   "With a wave of $s arm, $n conjures forth a column of acid and sends it hurling towards $N!" },

 { "shocking grasp",
   "You attempt to grab $N firmly by the throat as lightning crackles around you.",
   "$n attempts to grab you firmly by the throat as a painful shock shoots down your spine!", 
   "$n attempts to grab $N firmly by the throat as lightning bolts crackle around them!", },

{ "waterlungs",
   "You make a fluid gesture towards $N's chest.",
   "$n makes a fluid gesture towards your chest.", 
   "$n makes a fluid gesture towards $N's chest.", },

 { "luck",
   "With an enigmatic smile, you wink at $N.",
   "$n smiles enigmatically.", 
   "With an enigmatic smile, $n winks at $N.", },

 { "bad luck",
   "With an evil grin, you draw a hex in the air before $N.",
   "$n grins evilly, and draws something in the air.", 
   "With an evil grin, $n gestures something before $N.", },

 { "chill touch",
   "You reach towards $N with a deathly cold hand.",
   "$n reaches towards you with a deathly cold hand.", 
   "$n reaches out towards $N with a deathly cold hand.", },

 { "shadow walk",
   "In one motion, you twist the shadows in preparation for your shadow walk.",
   "In one motion, $n twists the shadows to form an abysmal void.", 
   "In one motion, $n twists the shadows to form an abysmal void.", },

 { "iceblast",
   "You extend your arms together and conjure forth an impressive mass of ice!",
   "$n extends $s arms together and conjures forth an impressive mass of ice!", 
   "$n extends $s arms together and conjures forth an impressive mass of ice!", },

 { "feeblemind",
   "You put a finger on your temples as you glare deeply into $N's eyes.",
   "$n puts a finger to $s temples and glares deeply into your eyes.", 
   "$n puts a finger to $s temples and glares deeply into the eyes of $N.",    },

 { "haste",
   "You make a series of rapid gestures in preparation for your enchantment.",
   "$n makes a series of rapid gestures in preparation for $s enchantment.", 
   "$n makes a series of rapid gestures in preparation for $s enchantment.",  },

 { "forget",
   "You wave a pale hand in front of $N's eyes as you begin your incantation.",
   "$n waves a pale hand in front of your eyes and begins $s incantation.",
   "$n waves a pale hand in front of $N's eyes and begins $s incantation.",  },

 { "disintegrate",
   "You extend your arm and release a handful of black ashes into the wind.",
   "$n extends $s arm and releases a handful of black ashes into the wind.", 
   "$n extends $s arm and releases a handful of black ashes into the wind.", },

 { "word of death",
   "You point a chilling bony finger at $N and whisper the word 'DIE!'.",
   "$n points a chilling bony finger at you and whispers the word 'DIE!'.", 
   "$n points a chilling bony finger at $N and whispers the word 'DIE!'.", },

 { "paralysis",
   "You make a gripping motion with your bony hands as you glare at $N.",
   "$n makes a gripping motion with $s bony hands and glares at you.", 
   "$n makes a gripping motion with $s bony hands and glares at $N.",  },

 { "ice shield",
   "You magically produce some shimmering frost which quickly expands into a freezing shield of ice.",
   "$n magically produces some shimmering frost which quickly expands into a freezing shield of ice.", 
   "$n magically produces some shimmering frost which quickly expands into a freezing shield of ice.", },

 { "flaming shield",
   "You conjure up a small flame which quickly expands into a flaming shield of magical fire.",
   "$n conjures up a small flame which quickly expands into a flaming shield of magical fire.", 
   "$n conjures up a small flame which quickly expands into a flaming shield of magical fire.", },

 { "slow",
   "You make a series of languid gestures in preparation for your enchantment.",
   "$n makes a series of languid gestures in preparation for $s enchantment.", 
   "$n makes a series of languid gestures in preparation for $s enchantment.",  },

 { "sleep",
   "You raise your hand then let it drop.",
   "$n raises $s hand, then lets it drop.", 
   "$n raises $s hand, then lets it drop.",  },

 { "looking glass",
   "You wiggle your fingers and an illusory looking-glass appears.",
   "$n wiggles $s fingers and an illusory looking-glass appears.", 
   "$n wiggles $s fingers and an illusory looking-glass appears.",  },

 { "curse",
   "You narrow your eyes and glare at $N.",
   "$n narrows $s eyes and glares at you.", 
   "$n narrows $s eyes and glares at $N.", },

 { "weaken",
   "You allow your body to go limp as you point towards $N with an emaciated gesture.",
   "$n allows $s body to go limp as $s points towards you with an emaciated gesture.",
   "$n allows $s body to go limp as $s points towards $N with an emaciated gesture.", },

 { "giant strength",
   "You clench your fist as you channel the power of strength for your enchantment.",
   "$n clentches $s fist and channels the power of strength for $s enchantment.",
   "$n clentches $s fist and channels the power of strength for $s enchantment.", },

 { "create nightmare",
   "You stretch your hands toward the shadows, beckoning them to you.",
   "$n stretches $s hands toward the shadows, and beckons them.",
   "$n stretches $s hands toward the shadows, and beckons them.",},

        { "", "" }
    };

    spellName = skill_table[sn].name;

    for ( i=0 ; gesture_table[i].name[0] != '\0' ; i++ )
    {
	if (!str_cmp(spellName, gesture_table[i].name))
	{
		if (gesture_table[i].ch[0] != '\0')
		{
			act(gesture_table[i].ch, ch, NULL, victim, TO_CHAR);
		}
		if (gesture_table[i].vict[0] != '\0' && victim)
		{
			act(gesture_table[i].vict, ch, NULL, victim, TO_VICT);
		}
		if (gesture_table[i].room[0] != '\0')
		{
			act(gesture_table[i].room, ch, NULL, victim, TO_NOTVICT);
		}
        }
    }
    return;
}


/*
 * Compute a saving throw.
 * Negative apply's make saving throw better.
 */
bool saves_spell( int level, CHAR_DATA *victim, int dam_type )
{
    int save;

    save = 30 + ( victim->level - level - victim->saving_throw) * 4;

    if (IS_AFFECTED(victim,AFF_BERSERK))
	save += victim->level/2;

    switch(check_immune(victim,dam_type))
    {
	case IS_IMMUNE:		return TRUE;
	case IS_RESISTANT:	save += 2;	break;
	case IS_VULNERABLE:	save -= 2;	break;
    }

    if (!IS_NPC(victim) && class_table[victim->class].fMana)
	save = 9 * save / 10;
    save = URANGE( 5, save, 95 );
    return number_percent( ) < save;
}

/* RT save for dispels */

bool saves_dispel( int dis_level, int spell_level, int duration)
{
    int save;
    
    if (duration == -1)
      spell_level += 5;  
      /* very hard to dispel permanent effects */

    save = 50 + (spell_level - dis_level) * 5;
    save = URANGE( 5, save, 95 );
    return number_percent( ) < save;
}

/* co-routine for dispel magic and cancellation */

bool check_dispel( int dis_level, CHAR_DATA *victim, int sn)
{
    AFFECT_DATA *af;

    if (is_affected(victim, sn))
    {
        for ( af = victim->affected; af != NULL; af = af->next )
        {
            if ( af->type == sn )
            {
                if (!saves_dispel(dis_level,af->level,af->duration))
                {
                    affect_strip(victim,sn);
        	    if ( skill_table[sn].msg_off )
        	    {
            		send_to_char( skill_table[sn].msg_off, victim );
            		send_to_char( "\n\r", victim );
        	    }
		    return TRUE;
		}
		else
		    af->level--;
            }
        }
    }
    return FALSE;
}

/* for finding mana costs -- temporary version */
int mana_cost (CHAR_DATA *ch, int min_mana, int level)
{
    if (ch->level + 2 == level)
	return 1000;
    return UMAX(min_mana,(100/(2 + ch->level - level)));
}



/*
 * The kludgy global is for spells who want more stuff from command line.
 */
char *target_name;

void do_cast( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    void *vo;
    int mana;
    int sn;
    int target;
    int penalty = 0;
    int aggravated = 0;

    /*
     * Switched NPC's can cast spells, but others can't.
     */
    if ( IS_NPC(ch) && ch->desc == NULL)
	return;

    if ( IS_AFFECTED2( ch, AFF_RAINBOW_PATTERN ))
    {
        send_to_char( "You can't seem to concentrate enough with that rainbow in the sky.\n\r", ch );
        return;
    }

    if ((number_percent() < 26)
    &&   is_affected(ch,gsn_earpunch))
    {
        send_to_char( "Being deaf and dizzy, the magic words don't come out right and you fail.\n\r", ch );
        return;
    }

    if (is_affected(ch,gsn_tonguecut))
    {
      send_to_char( "You have no tongue, hence you can not pronounce any words.\n\r", ch );
      return;
    }

    if( IS_AFFECTED2(ch, AFF_INVUL) )
    {
        send_to_char( "You are invulnerable.\n\r", ch );
        return;
    }

    if ((ch->in_room->sector_type == SECT_UNDERWATER)
    && (!IS_AFFECTED(ch, AFF_WATERBREATHING))
    && (!IS_SET(ch->imm_flags, IMM_DROWNING))
    && (ch->level <= LEVEL_HERO ))
    {
      send_to_char( "Despite your best efforts to inhale air underwater, you inhale water instead which prevents the magic words from being spoken at all.\n\r", ch );
      return;
    }

    if   (!str_cmp(class_table[ch->class].name, "Necromancer") && !(IS_EVIL(ch)))
    {
       send_to_char("You have strayed too close to the path of the light, and can no longer cast your spells!\n\r",ch);
        return;
    }

    if ( IS_AFFECTED2( ch, AFF_FEAR ))
    {
	  send_to_char("Fear still courses through your body, making it impossible to cast spells.\n\r", ch );
	return;
    }

    if (!IS_IMMORTAL(ch) && ch->pcdata->condition[COND_DRUNK] > 15)
    {
     send_to_char("You try to recite your spell, but can't pronounce the words in your drunken state.\n\r",ch);
     return;
    }

    if (IS_AFFECTED2(ch, AFF_PARALYSIS) )
    {
        send_to_char("You have been paralized! You couldn't possibly cast any magic!\n\r",ch);
        return;
    }

    if( IS_AFFECTED2(ch, AFF_SPEECH_IMPAIR) && !IS_IMMORTAL(ch) )
    {
       send_to_char("Your speech is impaired... you mispronounce the words.\n\r", ch );
       return;
    }

    target_name = one_argument( argument, arg1 );
    one_argument( target_name, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Cast which what where?\n\r", ch );
	return;
    }

    if ((sn = find_spell(ch,arg1)) < 1
    ||  skill_table[sn].spell_fun == spell_null
    || (!IS_NPC(ch) && (ch->level < skill_table[sn].skill_level[ch->class]
    ||   		 ch->pcdata->learned[sn] == 0)))
    {
	send_to_char( "You don't know any spells of that name.\n\r", ch );
	return;
    }

    if ( ch->position < skill_table[sn].minimum_position )
    {
	send_to_char( "You can't concentrate enough.\n\r", ch );
	return;
    }

    if (is_affected(ch,gsn_fingerbreak) 
    && (ch->class != class_lookup("cleric")
    || ch->religion_id == RELIGION_NONE)
    && skill_table[sn].spell_fun != spell_acid_breath
    && skill_table[sn].spell_fun != spell_breath_of_nature
    && skill_table[sn].spell_fun != spell_breath_of_the_dead
    && skill_table[sn].spell_fun != spell_fire_breath
    && skill_table[sn].spell_fun != spell_frost_breath
    && skill_table[sn].spell_fun != spell_gas_breath
    && skill_table[sn].spell_fun != spell_lightning_breath
    && skill_table[sn].spell_fun != spell_shallow_breath)
    {
	send_to_char("Your fingers are too smashed to properly channel magic through them.\n\r", ch );
	return;
    }

    if (ch->level + 2 == skill_table[sn].skill_level[ch->class])
	mana = 50;
    else
    	mana = UMAX(
	    skill_table[sn].min_mana,
	    100 / ( 2 + ch->level - skill_table[sn].skill_level[ch->class] ) );

    if (IS_SET(ch->in_room->room_flags,ROOM_NOMAGIC) && !(IS_IMMORTAL(ch)))
    {
        send_to_char( "You find that you are just too feebleminded to manage casting here.\n\r",ch);
        ch->mana -= mana;
        return;
    }

    if (IS_AFFECTED2(ch, AFF_SILENCE) )
    {
        send_to_char("You have been silenced! Your tongue is no longer responsive.\n\r",ch);
        ch->mana -= mana;
        act("$n tries to speak magic, but $s tongue doesn't seem to be working properly.",ch,NULL,NULL,TO_ROOM);
        return;
    }

    if( IS_AFFECTED( ch, AFF_SHACKLES) && !IS_IMMORTAL(ch) )
    {
	send_to_char("You can not properly cast spells while bound in shackles.\n\r", ch);
	return;
    }

    if( IS_AFFECTED2(ch, AFF_FORGET) && !IS_IMMORTAL(ch) )
    {
        send_to_char("You begin to recite your spell, but you just can't seem to remember how it goes...\n\r", ch );
        return;
    }


    /*
     * Locate targets.
     */
    victim	= NULL;
    obj		= NULL;
    vo		= NULL;
    target	= TARGET_NONE;
      
    switch ( skill_table[sn].target )
    {
    default:
	bug( "Do_cast: bad target for sn %d.", sn );
	return;

    case TAR_IGNORE:
	break;

    case TAR_CHAR_OFFENSIVE:
	if ( arg2[0] == '\0' )
	{
	    if ( ( victim = ch->fighting ) == NULL )
	    {
		send_to_char( "Cast the spell on whom?\n\r", ch );
		return;
	    }
	}
	else
	{
	    if ( ( victim = get_char_room( ch, NULL, target_name ) ) == NULL )
	    {
		send_to_char( "They aren't here.\n\r", ch );
		return;
	    }
	}
/*
        if ( ch == victim )
        {
            send_to_char( "You can't do that to yourself.\n\r", ch );
            return;
        }
*/


	if ( !IS_NPC(ch) )
	{
        if (is_safe(ch,victim) && victim != ch)
	  {
	    send_to_char("Your spoken magic fails to coalesce.\n\r",ch);
	    return; 
	  }
	  check_killer(ch,victim);

        if( IS_AFFECTED2(victim, AFF_INVUL)
        || victim->position == POS_DEAD
        ||  IS_AFFECTED3(victim, AFF_SUBDUE))
        {
          act( "$N is invulnerable.", ch, NULL, victim, TO_CHAR );
          act( "You are invulnerable to $n.", ch, NULL, victim, TO_VICT );
          return;
        }
	}

      if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
	{
	    send_to_char( "You can't do that on your own follower.\n\r",ch );
	    return;
	}

/*
      if ( !IS_NPC(victim)
	&& !IS_NPC(ch)
	&& !IS_IMMORTAL(ch)
	&& ABS(ch->level - victim->level) > 12 )
	{
	    act("$N is out of your league.\n\r",ch, NULL, victim, TO_CHAR);
	    return;
      }
*/
      if (victim->position == POS_DEAD)
      {
        send_to_char("You cannot cast that spell on a dead person.\n\r",ch);
        return;
      }

      if (( victim != ch)
      &&  ( !IS_NPC(victim))
      &&  ( get_skill(victim, gsn_evade_spell) > 1 )
      &&  (number_percent( ) < 26 ))
      {
        say_spell( ch, sn );
        gesture_spell( ch, sn, vo, target );
        act( "You nimbly evade $n's spell, stepping out of the way just in time.", ch, NULL, victim, TO_VICT);
        act( "$N was too well prepared and nimbly evades your spell.", ch, NULL, victim, TO_CHAR    );
        check_improve(victim,gsn_evade_spell,TRUE,1);
        ch->mana -= mana;
        return;
      }

      if (IS_AFFECTED2(ch, AFF_DEAFNESS))
	penalty += 40;

	vo = (void *) victim;
	target = TARGET_CHAR;
	break;

    case TAR_CHAR_SEMIOFFENSIVE:
      if ( arg2[0] == '\0' )
	   {
           (victim = ch);
	   }
        else
         {
            if ( ( victim = get_char_room( ch, NULL, target_name ) ) == NULL )
            {
                send_to_char( "They aren't here.\n\r", ch );
                return;
            }
         }

	if ( !IS_NPC(ch) )
	{

          if (is_safe(ch,victim) && victim != ch)
	    {
		send_to_char("Your spoken magic fails to coalesce.\n\r",ch);
		return; 
	    }
	    check_killer(ch,victim);

          if( IS_AFFECTED2(victim, AFF_INVUL)
          || victim->position == POS_DEAD
          || IS_AFFECTED3(victim, AFF_SUBDUE))
          {
            act( "$N is invulnerable.", ch, NULL, victim, TO_CHAR );
            act( "You are invulnerable to $n.", ch, NULL, victim, TO_VICT );
            return;
          }
	}

/*
      if ( !IS_NPC(victim)
	&& !IS_NPC(ch)
	&& !IS_IMMORTAL(ch)
	&& ABS(ch->level - victim->level) > 12 )
	{
	    act("$n is out of your league.\n\r",ch, NULL, victim, TO_CHAR);
	    return;
      }
*/
      if (victim->position == POS_DEAD)
      {
         send_to_char("You cannot cast that spell on a dead person.\n\r",ch);
         return;
      }

      if (IS_AFFECTED2(ch, AFF_DEAFNESS))
	penalty += 40;

      vo = (void *) victim;
	aggravated = (number_range(1,80) + number_fuzzy(get_curr_stat(victim, STAT_INT)) - number_fuzzy(get_curr_stat(ch, STAT_CHR)));
      break;


    case TAR_CHAR_DEFENSIVE:
	if ( arg2[0] == '\0' )
	{
	    victim = ch;
	}
	else
	{
	    if ( ( victim = get_char_room( ch, NULL, target_name ) ) == NULL )
	    {
		send_to_char( "They aren't here.\n\r", ch );
		return;
	    }

          if( IS_AFFECTED2(victim, AFF_INVUL)
          || victim->position == POS_DEAD
          ||  IS_AFFECTED3(victim, AFF_SUBDUE))
          {
            act( "$N is invulnerable.", ch, NULL, victim, TO_CHAR );
            act( "You are invulnerable to $n.", ch, NULL, victim, TO_VICT );
            return;
          }
	}

      if (IS_AFFECTED2(ch, AFF_DEAFNESS))
	penalty += 40;

	vo = (void *) victim;
	target = TARGET_CHAR;
	break;

    case TAR_CHAR_SELF:
	if ( arg2[0] != '\0' && !is_name( target_name, ch->name ) )
	{
	    send_to_char( "You cannot cast this spell on another.\n\r", ch );
	    return;
	}

      if (IS_AFFECTED2(ch, AFF_DEAFNESS))
	penalty += 40;

	vo = (void *) ch;
	target = TARGET_CHAR;
	break;

    case TAR_OBJ_INV:
	if ( arg2[0] == '\0' )
	{
	    send_to_char( "What should the spell be cast upon?\n\r", ch );
	    return;
	}

	if ( ( obj = get_obj_carry( ch, arg2, ch ) ) == NULL )
	{
	    send_to_char( "You are not carrying that.\n\r", ch );
	    return;
	}

      if (IS_AFFECTED2(ch, AFF_DEAFNESS))
	penalty += 40;

      vo = (void *) obj;
      break;

    case TAR_OBJ_CHAR_OFF:
	if (arg2[0] == '\0')
	{
	    if ((victim = ch->fighting) == NULL)
	    {
		send_to_char("Cast the spell on whom or what?\n\r",ch);
		return;
	    }
	
	    target = TARGET_CHAR;
	}
	else if ((victim = get_char_room(ch, NULL, target_name)) != NULL)
	{
	    target = TARGET_CHAR;
	}

	if (target == TARGET_CHAR) /* check the sanity of the attack */
	{
	    if(is_safe_spell(ch,victim,FALSE) && victim != ch)
	    {
		send_to_char("Not on that target.\n\r",ch);
		return;
	    }

          if( IS_AFFECTED2(victim, AFF_INVUL)
          || victim->position == POS_DEAD
          || IS_AFFECTED3(victim, AFF_SUBDUE))
          {
            act( "$N is invulnerable.", ch, NULL, victim, TO_CHAR );
            act( "You are invulnerable to $n.", ch, NULL, victim, TO_VICT );
            return;
          }

            if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
            {
                send_to_char( "You can't do that on your own follower.\n\r",
                    ch );
                return;
            }

	    if (!IS_NPC(ch))
		check_killer(ch,victim);

	    vo = (void *) victim;
 	}
	else if ((obj = get_obj_here(ch, NULL, target_name)) != NULL)
	{
	    vo = (void *) obj;
	    target = TARGET_OBJ;
	}
	else
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}

      if (IS_AFFECTED2(ch, AFF_DEAFNESS))
	penalty += 40;

	break; 

    case TAR_OBJ_CHAR_DEF:
      if (arg2[0] == '\0')
      {
        vo = (void *) ch;
        target = TARGET_CHAR;                                                 
      }
      else if ((victim = get_char_room(ch, NULL, target_name)) != NULL)
      {
        vo = (void *) victim;
        target = TARGET_CHAR;
	}
	else if ((obj = get_obj_carry(ch,target_name,ch)) != NULL)
	{
	    vo = (void *) obj;
	    target = TARGET_OBJ;
	}
	else
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
	break;
    }
	    
    if ( !IS_NPC(ch) && ch->mana < mana )
    {
	send_to_char( "You don't have enough mana.\n\r", ch );
	return;
    }
      
    if ( str_cmp( skill_table[sn].name, "ventriloquate" )
    &&   str_cmp( skill_table[sn].name, "illusion" )
    &&   str_cmp( skill_table[sn].name, "doppelganger"))
      say_spell( ch, sn );

    if (!IS_IMPLEMENTOR(ch))
    {
      WAIT_STATE( ch, skill_table[sn].beats );
    }
      
    if ( !IS_NPC(ch) && number_percent( ) > get_skill(ch,sn) - penalty )
    {
	send_to_char( "You lost your concentration.\n\r", ch );
	check_improve(ch,sn,FALSE,1);
	ch->mana -= mana / 2;
    }
    else
    {
      ch->mana -= mana;
      gesture_spell( ch, sn, vo, target );

      if (IS_NPC(ch) || class_table[ch->class].fMana)
	/* class has spells */
        (*skill_table[sn].spell_fun) ( sn, ch->level, ch, vo,target);
      else
        (*skill_table[sn].spell_fun) (sn, 3 * ch->level/4, ch, vo,target);
      check_improve(ch,sn,TRUE,1);
    }

    if ((skill_table[sn].target == TAR_CHAR_OFFENSIVE
    ||  (skill_table[sn].target == TAR_CHAR_SEMIOFFENSIVE && aggravated > 80)
    ||  (skill_table[sn].target == TAR_OBJ_CHAR_OFF && target == TARGET_CHAR))
    &&   victim != ch
    &&   victim->master != ch)
    {
      CHAR_DATA *vch;
      CHAR_DATA *vch_next;

	if (ch->fight_pos == FIGHT_REAR)
	{
        bool Fighting = FALSE;

	  for (vch = ch->in_room->people ; vch ; vch = vch->next)
	  {
	    if (vch == ch || IS_NPC(vch))
		continue;

	    if (is_same_group(ch, vch))
	    {
		if (vch->fighting != NULL)
		Fighting = TRUE;
	    }
	  }
	  if (Fighting == FALSE)
	  {
	    //bug("Advancing from rear rank during cast.", 0);
	    ch->fight_pos = FIGHT_MIDDLE;
	    send_to_char("You advance towards the monster.\r\n",ch);
	  }
	  else
	  {
	    //bug("Stop fighting after cast.", 0);
	    stop_fighting(ch, FALSE);
	  }
	}

	for ( vch = ch->in_room->people; vch; vch = vch_next )
	{
        vch_next = vch->next_in_room;
        if ( victim == vch && victim->fighting == NULL
        && !IS_AFFECTED3(victim,AFF_HOLDPERSON) )
	  {
	    check_killer(victim,ch);
	    multi_hit( victim, ch, TYPE_UNDEFINED );
	    break;
	  }
	}
    }
    return;
}



/*
 * Cast spells at targets using a magical object.
 */
void obj_cast_spell( int sn, int level, CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj )
{
    void *vo=NULL;
    int target = TARGET_NONE;
    int aggravated=0;

    if ( sn <= 0 )
	return;

    if  (IS_SET(ch->in_room->room_flags, ROOM_NOMAGIC) 
    && !IS_IMMORTAL(ch))
    {
        send_to_char("Your spell fizzles out and fails.\n\r",ch);
        act("$n's spell fizzles out and fails.\n\r", ch,NULL,NULL, TO_ROOM);
        return;
    }

    if ( sn >= MAX_SKILL || skill_table[sn].spell_fun == 0 )
    {
	bug( "Obj_cast_spell: bad sn %d.", sn );
	return;
    }

    switch ( skill_table[sn].target )
    {
    default:
	bug( "Obj_cast_spell: bad target for sn %d.", sn );
	return;

    case TAR_IGNORE:
	vo = NULL;
	break;

    case TAR_CHAR_OFFENSIVE:
	if ( victim == NULL )
	    victim = ch->fighting;
	if ( victim == NULL )
	{
	    send_to_char( "You can't do that.\n\r", ch );
	    return;
	}
	if (is_safe(ch,victim) && ch != victim)
	{
	    send_to_char("Something isn't right...\n\r",ch);
	    return;
	}
	vo = (void *) victim;
	break;

    case TAR_CHAR_SEMIOFFENSIVE:

      if (victim == NULL )
          victim = ch->fighting;

	if (victim == NULL)
	    victim = ch;

	if (is_safe(ch,victim) && ch != victim)
	{
	    send_to_char("The area is shielded from harm.\n\r",ch);
	    return;
	}

        vo = (void *) victim;
	aggravated = (number_range(1,80) + number_fuzzy(get_curr_stat(victim, STAT_INT)) - number_fuzzy(get_curr_stat(ch, STAT_WIS)));
        break;

    case TAR_CHAR_DEFENSIVE:
        if ( victim == NULL )
            victim = ch;
        vo = (void *) victim;
        break;
    case TAR_CHAR_SELF:
        vo = (void *) ch;
        break;
    case TAR_OBJ_INV:
        if ( obj == NULL )
        {
            send_to_char( "You can't do that.\n\r", ch );
            return;
        }
        vo = (void *) obj;
        break;

    case TAR_OBJ_CHAR_OFF:
        if ( victim == NULL && obj == NULL)
        {
	    if (ch->fighting != NULL)
		victim = ch->fighting;
	    else
	    {
		send_to_char("You can't do that.\n\r",ch);
		return;
	    }

	    if (victim != NULL)
	    {
		if (is_safe_spell(ch,victim,FALSE) && ch != victim)
		{
		    send_to_char("Something isn't right...\n\r",ch);
		    return;
		}

		vo = (void *) victim;
		target = TARGET_CHAR;
	    }
	    else
	    {
	    	vo = (void *) obj;
	    	target = TARGET_OBJ;
            }
        }
        break;


    case TAR_OBJ_CHAR_DEF:
	if (victim == NULL && obj == NULL)
	{
	    vo = (void *) ch;
	    target = TARGET_CHAR;
	}
	else if (victim != NULL)
	{
	    vo = (void *) victim;
	    target = TARGET_CHAR;
	}
	else
	{
	    vo = (void *) obj;
	    target = TARGET_OBJ;
	}
	
	break;
    }

    target_name = "";
    (*skill_table[sn].spell_fun) ( sn, level, ch, vo,target);

    

    if ( (skill_table[sn].target == TAR_CHAR_OFFENSIVE
    ||   ((skill_table[sn].target == TAR_CHAR_SEMIOFFENSIVE) && (aggravated > 80))
    ||   (skill_table[sn].target == TAR_OBJ_CHAR_OFF && target == TARGET_CHAR))
    &&   victim != ch
    &&   victim->master != ch )
    {
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;

	for ( vch = ch->in_room->people; vch; vch = vch_next )
	{
	    vch_next = vch->next_in_room;
	    if ( victim == vch && victim->fighting == NULL )
	    {
		check_killer(victim,ch);
		multi_hit( victim, ch, TYPE_UNDEFINED );
		break;
	    }
	}
    }

    return;
}



/*
 * Spell functions.
 */
void spell_acid_blast( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = dice( level, 8 );
    if ( saves_spell( level, victim, DAM_ACID ) )
	dam /= 2;
    damage( ch, victim, dam, sn, DAM_ACID, TRUE);
    return;
}



void spell_armor( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected(victim, skill_lookup("phantom armor")))
    {
	if (victim == ch)
	  send_to_char("This will not work on you when already protected by Phantom Armor.\n\r",ch);
	else
	  act("$N is protected by an Illusory armor, your spell failed.",ch,NULL,victim,TO_CHAR);
	return;
    }

    if ( is_affected( victim, sn ) )
    {
	if (victim == ch)
	  send_to_char("You are already armored.\n\r",ch);
	else
	  act("$N is already armored.",ch,NULL,victim,TO_CHAR);
	return;
    }
    af.where	 	= TO_AFFECTS;
    af.type      		= sn;
    af.level	 	= level;
    af.duration  		= 24;
    af.modifier  		= -20;
    af.location  		= APPLY_AC;
    af.bitvector 		= 0;
    affect_to_char( victim, &af );
    send_to_char( "You feel someone protecting you.\n\r", victim );
    if ( ch != victim )
	act("$N is protected by your magic.",ch,NULL,victim,TO_CHAR);
    return;
}

void spell_bless( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    AFFECT_DATA af;

    /* deal with the object case first */
    if (target == TARGET_OBJ)
    {
	obj = (OBJ_DATA *) vo;
	if (IS_OBJ_STAT(obj,ITEM_BLESS))
	{
	    act("$p is already blessed.",ch,obj,NULL,TO_CHAR);
	    return;
	}

	if (IS_OBJ_STAT(obj,ITEM_EVIL))
	{
	    AFFECT_DATA *paf;

	    paf = affect_find(obj->affected,gsn_curse);
	    if (!saves_dispel(level,paf != NULL ? paf->level : obj->level,0))
	    {
		if (paf != NULL)
		    affect_remove_obj(obj,paf);
		act("$p glows a pale blue.",ch,obj,NULL,TO_ALL);
		REMOVE_BIT(obj->extra_flags,ITEM_EVIL);
		return;
	    }
	    else
	    {
		act("The evil of $p is too powerful for you to overcome.",
		    ch,obj,NULL,TO_CHAR);
		return;
	    }
	}
	
	af.where	= TO_OBJECT;
	af.type		= sn;
	af.level	= level;
	af.duration	= 6 + level;
	af.location	= APPLY_SAVES;
	af.modifier	= -1;
	af.bitvector	= ITEM_BLESS;
	affect_to_obj(obj,&af);

	act("$p glows with a holy aura.",ch,obj,NULL,TO_ALL);

	if (obj->wear_loc != WEAR_NONE)
	    ch->saving_throw -= 1;
	return;
    }

    /* character target */
    victim = (CHAR_DATA *) vo;

    if ( is_affected( victim, sn ) )
    {
        if (victim == ch)
          send_to_char("You are already blessed.\n\r",ch);
        else
          act("$N already has divine favor.",ch,NULL,victim,TO_CHAR);
        return;
    }

   if ( victim->position == POS_FIGHTING)
    {
        if (victim == ch)
          send_to_char("You can not bless yourself while fighting.\n\r",ch);
        else
          act("$N can not be blessed in the heat of battle.",ch,NULL,victim,TO_CHAR);
        return;
    }

	if ((victim != ch) && (!IS_IMMORTAL(ch)))
        {
          if ( ch->alignment >= 300 )
            do_say (ch, "May the grace of the light bless you, my friend.");
          else if ( ch->alignment <= -300 )
            do_say (ch, "May the dark gods favor you in your journeys.");
          else 
           do_say (ch, "May these blessings guide you in your destiny.");
        }
	else if IS_IMMORTAL(ch)
        {
          if IS_GOOD(ch)
            do_say (ch, "The grace of the light smiles upon you.");
          else if IS_EVIL(ch)
            do_say (ch, "Darkness casts its shadows upon you.");
          else 
           do_say (ch, "May these blessings guide your destiny.");
        }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = 6+level;
    af.location  = APPLY_HITROLL;
    af.modifier  = level / 8;
    af.bitvector = 0;
    affect_to_char( victim, &af );

    af.location  = APPLY_SAVING_SPELL;
    af.modifier  = 0 - level / 8;
    affect_to_char( victim, &af );
    send_to_char( "You feel righteous.\n\r", victim );
    if (IS_IMMORTAL(ch) && (ch != victim))
	{
	act("You place your favor upon $N.\n\r",ch,NULL,victim,TO_CHAR);
	act("$n gives $N divine blessing.\n\r",ch,NULL,victim,TO_NOTVICT);
	act("You have received the divine blessing of $n.\n\r",ch,NULL,victim,TO_VICT);
	return;
	}
    if (ch != victim )
        act("You grant $N the favor of your god.",ch,NULL,victim,TO_CHAR);
    return;
}



void spell_blindness( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_AFFECTED(ch,AFF_BLIND))
    {
	send_to_char("Cast the spell on whom? Your blind as a bat.\n\r",ch);
	return;
    }

    if ( IS_AFFECTED(victim, AFF_BLIND) || saves_spell(level,victim,DAM_OTHER))
	return;


    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.location  = APPLY_HITROLL;
    af.modifier  = -4;
    af.duration  = 1+level;
    af.bitvector = AFF_BLIND;
    affect_to_char( victim, &af );
    send_to_char( "You are blinded!\n\r", victim );
    act("$n appears to be blinded.",victim,NULL,NULL,TO_ROOM);
    return;
}



void spell_burning_hands(int sn,int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const sh_int dam_each[] = 
    {
	 0,
	 0,  0,  0,  0,	14,	17, 20, 23, 26, 29,
	29, 29, 30, 30,	31,	31, 32, 32, 33, 33,
	34, 34, 35, 35,	36,	36, 37, 37, 38, 38,
	39, 39, 40, 40,	41,	41, 42, 42, 43, 43,
	44, 44, 45, 45,	46,	46, 47, 47, 48, 48
    };
    int dam;

    level	= UMIN(level, sizeof(dam_each)/sizeof(dam_each[0]) - 1);
    level	= UMAX(0, level);
    dam		= number_range( dam_each[level] / 2, dam_each[level] * 2 );
    if ( saves_spell( level, victim,DAM_FIRE) )
	dam /= 2;
    damage( ch, victim, dam, sn, DAM_FIRE,TRUE);
    return;
}



void spell_call_lightning( int sn, int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    int dam;
    char buf[MSL];

    if ( !IS_OUTSIDE(ch) )
    {
	send_to_char( "You must be out of doors.\n\r", ch );
	return;
    }

    if ( weather_info.sky < SKY_RAINING )
    {
	send_to_char( "You need bad weather.\n\r", ch );
	return;
    }

    dam = dice(level/2, 8);

    send_to_char( "Your Gods lightning strikes your foes.\n\r", ch );
    sprintf(buf,"$n calls $s God's lightning to strike $s foes!\n\r");
    act( buf,ch, NULL, NULL, TO_ROOM );

    for ( vch = char_list; vch != NULL; vch = vch_next )
    {
	vch_next	= vch->next;
	if ( vch->in_room == NULL )
	    continue;

	if( is_safe_spell(ch, vch, FALSE) )
		continue;

	if ( vch->in_room == ch->in_room )
	{
	    if ( vch != ch && ( IS_NPC(ch) ? !IS_NPC(vch) : IS_NPC(vch) ) )
		damage( ch, vch, saves_spell( level,vch,DAM_LIGHTNING) 
		? dam / 2 : dam, sn,DAM_LIGHTNING,TRUE);
	    continue;
	}

	if ( vch->in_room->area == ch->in_room->area
	&&   IS_OUTSIDE(vch)
	&&   IS_AWAKE(vch) )
	    send_to_char( "Lightning flashes in the sky.\n\r", vch );
    }

    return;
}

/* RT calm spell stops all fighting in the room */

void spell_calm( int sn, int level, CHAR_DATA *ch, void *vo,int target)
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

    if (number_range(0, chance) >= mlevel)  /* hard to stop large fights */
    {
	for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
   	{
	    if (IS_NPC(vch) && (IS_SET(vch->imm_flags,IMM_MAGIC) ||
				IS_SET(vch->act,ACT_UNDEAD)))
	      return;

	    if (IS_AFFECTED(vch,AFF_CALM) || IS_AFFECTED(vch,AFF_BERSERK)
	    || is_affected(vch,skill_lookup("frenzy"))
	    || IS_AFFECTED2(vch,AFF_RAINBOW_PATTERN))
	      return;
	    
	    send_to_char("A wave of calm passes over you.\n\r",vch);

	    if (vch->fighting || vch->position == POS_FIGHTING)
	      stop_fighting(vch,FALSE);


	    af.where = TO_AFFECTS;
	    af.type = sn;
  	    af.level = level;
	    af.duration = level/8;
	    af.location = APPLY_HITROLL;
	    if (!IS_NPC(vch))
	      af.modifier = -5;
	    else
	      af.modifier = -2;
	    af.bitvector = AFF_CALM;
	    affect_to_char(vch,&af);

	    af.location = APPLY_DAMROLL;
	    affect_to_char(vch,&af);
	}
    }
}

void spell_cancellation( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    bool found = FALSE;
 
    level += 2;

    if ((!IS_NPC(ch) && IS_NPC(victim)
    && !(IS_AFFECTED(ch, AFF_CHARM)
    && ch->master == victim) )
    || (IS_NPC(ch)
    && IS_NPC(victim)))
    {
      send_to_char("You failed, try dispel magic.\n\r",ch);
      return;
    }

    if (IS_AFFECTED(ch,AFF_BLIND))
    {
      send_to_char("Cast the spell on whom? Your blind as a bat.\n\r",ch);
      return;
    }

    if ((victim != ch)
     &  !is_same_group(ch, victim))
    {
      send_to_char("You can only cast this spell on yourself or your fellow group members.\n\r",ch);
      return;
    }

    if ((victim->fighting != NULL)
    &&  (number_percent() < 50))
    {
      send_to_char("Your cancellation spell failed.\n\r",ch);
      return;
    }

    /* unlike dispel magic, the victim gets NO save */
 
    /* begin running through the spells */
 
    if (check_dispel(level,victim,skill_lookup("armor")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("bless")))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("feeblemind")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("blindness")))
    {
        found = TRUE;
        act("$n is no longer blinded.",victim,NULL,NULL,TO_ROOM);
    }

    if (check_dispel(level,victim,skill_lookup("bark skin")))
    {
        act("$n's skin loses its rough nature.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }

    if (check_dispel(level,victim,skill_lookup("calm")))
    {
	found = TRUE;
	act("$n no longer looks so peaceful...",victim,NULL,NULL,TO_ROOM);
    }
 
    if (check_dispel(level,victim,skill_lookup("change sex")))
    {
        found = TRUE;
        act("$n looks more like $mself again.",victim,NULL,NULL,TO_ROOM);
    }

    if (check_dispel(level,victim,skill_lookup("repulsion")))
    {
        found = TRUE;
        act("$n is no longer feared by aggressive monsters.",victim,NULL,NULL,TO_ROOM);
    }
 
    if (check_dispel(level,victim,skill_lookup("charm person")))
    {
        found = TRUE;
        act("$n regains $s free will.",victim,NULL,NULL,TO_ROOM);
    }
 
    if (check_dispel(level,victim,skill_lookup("chill touch")))
    {
        found = TRUE;
        act("$n looks warmer.",victim,NULL,NULL,TO_ROOM);
    }
 
    if (check_dispel(level,victim,skill_lookup("curse")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("detect evil")))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("detect good")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("detect hidden")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("detect invis")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("forest vision")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("detect magic")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("incendiary cloud")))
    {
        act("$n's outline fades.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("levitate")))
    {
        act("$n falls to the ground!",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }

    if (check_dispel(level,victim,skill_lookup("windspirit")))
    {
        found = TRUE;
    }

    if (check_dispel(level,victim,skill_lookup("stonespirit")))
    {
        found = TRUE;
    }

    if (check_dispel(level,victim,skill_lookup("frenzy")))
    {
	act("$n no longer looks so wild.",victim,NULL,NULL,TO_ROOM);;
	found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("giant strength")))
    {
        act("$n no longer looks so mighty.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }

    if (check_dispel(level,victim,skill_lookup("haste")))
    {
	act("$n is no longer moving so quickly.",victim,NULL,NULL,TO_ROOM);
	found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("infravision")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("invis")))
    {
        act("$n fades into existance.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("mass invis")))
    {
        act("$n fades into existance.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("pass door")))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("protection evil")))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("flaming shield")))
	  found = TRUE;

    if (check_dispel(level,victim,skill_lookup("ice shield")))
	  found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("sanctuary")))
    {
        act("The white aura around $n's body vanishes.",
            victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("shield")))
    {
        act("The shield protecting $n vanishes.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("sleep")))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("blackjack")))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("slow")))
    {
        act("$n is no longer moving so slowly.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("flesh to stone")))
    {
        act("$n's skin regains its normal texture.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("weaken")))
    {
        act("$n looks stronger.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }

    if (check_dispel(level,victim,skill_lookup("dragon skin")))
    {
        found = TRUE;
        act("$n's skin softens to it's normal texture.",victim,NULL,NULL,TO_ROOM);
    }

    if (check_dispel(level,victim,skill_lookup("silence")))
    {
        found = TRUE;
        act("$n regains the ability to speak again.",victim,NULL,NULL,TO_ROOM);
    }

    if (check_dispel(level,victim,skill_lookup("dark taint")))
    {
        act("The taint from $n flees into the darkness from which it came.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }

    if (check_dispel(level,victim,skill_lookup("great wasting")))
    {
        act("Health returns to $n as vigor is restored.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    } 

    if (check_dispel(level,victim,skill_lookup("alter reality")))
    {
        found = TRUE;
        act("$n comes back from $s altered state of reality.",victim,NULL,NULL,TO_ROOM);
    }
 
    if (check_dispel(level,victim,skill_lookup("mooncloak")))
    {
	  found = TRUE;
	  act("The energies of the moon leave $n and return to the sky.",victim,NULL,NULL,TO_ROOM);
    }

    if (found)
        send_to_char("Ok.\n\r",ch);
    else
        send_to_char("Spell failed.\n\r",ch);
}

void spell_cause_light( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    damage( ch, (CHAR_DATA *) vo, dice(1, 8) + level / 3, sn,DAM_HARM,TRUE);
    return;
}



void spell_cause_critical(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    damage( ch, (CHAR_DATA *) vo, dice(3, 8) + level - 6, sn,DAM_HARM,TRUE);
    return;
}



void spell_cause_serious(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    damage( ch, (CHAR_DATA *) vo, dice(2, 8) + level / 2, sn,DAM_HARM,TRUE);
    return;
}

void spell_chain_lightning(int sn,int level,CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    CHAR_DATA *tmp_vict,*last_vict,*next_vict;
    bool found;
    int dam;

    /* first strike */

    act("A lightning bolt leaps from $n's hand and arcs to $N.",
        ch,NULL,victim,TO_ROOM);
    act("A lightning bolt leaps from your hand and arcs to $N.",
	ch,NULL,victim,TO_CHAR);
    act("A lightning bolt leaps from $n's hand and hits you!",
	ch,NULL,victim,TO_VICT);  

    dam = dice(level,6);
    if (saves_spell(level,victim,DAM_LIGHTNING))
 	dam /= 3;
    damage(ch,victim,dam,sn,DAM_LIGHTNING,TRUE);
    last_vict = victim;
    level -= 4;   /* decrement damage */

    /* new targets */
    while (level > 0)
    {
	found = FALSE;
	for (tmp_vict = ch->in_room->people; 
	     tmp_vict != NULL; 
	     tmp_vict = next_vict) 
	{
	  next_vict = tmp_vict->next_in_room;
	  if (!is_safe_spell(ch,tmp_vict,TRUE) && tmp_vict != last_vict)
	  {
	    found = TRUE;
	    last_vict = tmp_vict;
	    act("The bolt arcs to $n!",tmp_vict,NULL,NULL,TO_ROOM);
	    act("The bolt hits you!",tmp_vict,NULL,NULL,TO_CHAR);
	    dam = dice(level,6);
	    if (saves_spell(level,tmp_vict,DAM_LIGHTNING))
		dam /= 3;
	    damage(ch,tmp_vict,dam,sn,DAM_LIGHTNING,TRUE);
	    level -= 4;  /* decrement damage */
	  }
	}   /* end target searching loop */
	
	if (!found) /* no target found, hit the caster */
	{
	  if (ch == NULL)
     	    return;

	  if (last_vict == ch) /* no double hits */
	  {
	    act("The bolt seems to have fizzled out.",ch,NULL,NULL,TO_ROOM);
	    act("The bolt grounds out through your body.",
		ch,NULL,NULL,TO_CHAR);
	    return;
	  }
	
	  last_vict = ch;
	  act("The bolt arcs to $n...whoops!",ch,NULL,NULL,TO_ROOM);
	  send_to_char("You are struck by your own lightning!\n\r",ch);
	  dam = dice(level,6);
	  if (saves_spell(level,ch,DAM_LIGHTNING))
	    dam /= 3;
	  damage(ch,ch,dam,sn,DAM_LIGHTNING,TRUE);
	  level -= 4;  /* decrement damage */
	  if (ch == NULL) 
	    return;
	}
    /* now go back and find more targets */
    }
}
	  

void spell_change_sex( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ))
    {
	if (victim == ch)
	  send_to_char("You've already been changed.\n\r",ch);
	else
	  act("$N has already had $s(?) sex changed.",ch,NULL,victim,TO_CHAR);
	return;
    }
    if (saves_spell(level , victim,DAM_OTHER))
	return;	
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 2 * level;
    af.location  = APPLY_SEX;
    do
    {
	af.modifier  = number_range( 0, 2 ) - victim->sex;
    }
    while ( af.modifier == 0 );
    af.bitvector = 0;
    affect_to_char( victim, &af );
    send_to_char( "You feel different.\n\r", victim );
    act("$n doesn't look like $mself anymore...",victim,NULL,NULL,TO_ROOM);
    return;
}



void spell_charm_person( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    CHAR_DATA *gch;
    AFFECT_DATA af;
    int charm_count = 0;

    if (IS_AFFECTED2(victim, AFF_INVUL) 
    ||  IS_AFFECTED3(ch, AFF_SUBDUE))
    {
      send_to_char( "$n is untouchable to you.\n\r", ch );
      return;
    }

    if (IS_AFFECTED2(ch, AFF_INVUL)
    ||  IS_AFFECTED3(ch, AFF_SUBDUE))
    {
      send_to_char( "No way!\n\r", ch );
      return; 
    }

    if (is_safe(ch,victim)) return;

    if ( victim == ch )
    {
	send_to_char( "You like yourself even better!\n\r", ch );
	return;
    }

    if ( victim->position == POS_SLEEPING )
    {
        send_to_char( "You can not seem to get your victim's attention.\n\r", ch );
        send_to_char( "Your slumbers are briefly troubled.\n\r", victim );
        return;
    }

    if ( IS_AFFECTED(victim, AFF_CHARM)
    ||   IS_AFFECTED(ch, AFF_CHARM)
    ||   level < victim->level
    ||   IS_SET(victim->imm_flags,IMM_CHARM)
    ||   saves_spell( level, victim,DAM_CHARM) )
	return;


    if (IS_SET(victim->in_room->room_flags,ROOM_LAW))
    {	send_to_char(
	    "The mayor does not allow charming in the city limits.\n\r",ch);
	return;
    }
  
    for(gch = char_list; gch != NULL; gch = gch->next)
    {
	if( IS_NPC(gch) && IS_AFFECTED(gch,AFF_CHARM) 
	&& gch->master == ch )
		charm_count++;
    }

    if(charm_count >= 2) //Total 2 Charmies (1, 2)
    {
	send_to_char("Your influence can only stretch so far!\n\r",ch);
	return;
    }

    if ( victim->master )
	stop_follower( victim );
    add_follower( victim, ch );
    victim->leader = ch;
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = number_fuzzy( level / 4 );
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = AFF_CHARM;
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



void spell_chill_touch( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const sh_int dam_each[] = 
    {
	 0,
	 0,  0,  6,  7,  8,	 9, 12, 13, 13, 13,
	14, 14, 14, 15, 15,	15, 16, 16, 16, 17,
	17, 17, 18, 18, 18,	19, 19, 19, 20, 20,
	20, 21, 21, 21, 22,	22, 22, 23, 23, 23,
	24, 24, 24, 25, 25,	25, 26, 26, 26, 27
    };
    int dam;

  if ( ch->fight_pos > FIGHT_FRONT )  
    {
    send_to_char("You are not close enough to give a chilling touch to your victim.\n\r", ch );
    act("$n fails to reach $s victim.", ch, NULL, NULL, TO_ROOM);
    return;
    }

    level	= UMIN(level, sizeof(dam_each)/sizeof(dam_each[0]) - 1);
    level	= UMAX(0, level);
    dam	= number_range( dam_each[level] / 2, dam_each[level] * 2 );
    if ( saves_spell( level, victim,DAM_COLD ) )
    {
	dam /= 2;
    }
    act("$n turns blue and shivers.",victim,NULL,NULL,TO_ROOM);
    damage( ch, victim, dam, sn, DAM_COLD,TRUE );
    return;
}



void spell_colour_spray( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const sh_int dam_each[] = 
    {
	 0,
	 0,  0,  0,  0,  0,	 0,  0,  0,  0,  0,
	30, 35, 40, 45, 50,	55, 55, 55, 56, 57,
	58, 58, 59, 60, 61,	61, 62, 63, 64, 64,
	65, 66, 67, 67, 68,	69, 70, 70, 71, 72,
	73, 73, 74, 75, 76,	76, 77, 78, 79, 79
    };
    int dam;

    level	= UMIN(level, sizeof(dam_each)/sizeof(dam_each[0]) - 1);
    level	= UMAX(0, level);
    dam		= number_range( dam_each[level] / 2,  dam_each[level] * 2 );
    if ( saves_spell( level, victim,DAM_LIGHT) )
	dam /= 2;
    else 
	spell_blindness(skill_lookup("blindness"),
	    level/2,ch,(void *) victim,TARGET_CHAR);

    damage( ch, victim, dam, sn, DAM_LIGHT,TRUE );
    return;
}



void spell_continual_light(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    OBJ_DATA *light;

    light = create_object( get_obj_index( OBJ_VNUM_LIGHT_BALL ), 0 );
    obj_to_room( light, ch->in_room );
    act( "$n twiddles $s thumbs and $p appears.",   ch, light, NULL, TO_ROOM );
    act( "You twiddle your thumbs and $p appears.", ch, light, NULL, TO_CHAR );
    return;
}

void spell_control_weather(int sn,int level,CHAR_DATA *ch,void *vo,int target) 
{
    if ( !str_cmp( target_name, "better" ) )
	weather_info.change += dice( level / 3, 4 );
    else if ( !str_cmp( target_name, "worse" ) )
	weather_info.change -= dice( level / 3, 4 );
    else
	send_to_char ("Do you want it to get better or worse?\n\r", ch );

    send_to_char( "Ok.\n\r", ch );
    return;
}

void spell_conjure_food( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
  OBJ_DATA *bloodvial1;
  OBJ_DATA *bloodvial2;
  OBJ_DATA *bloodvial3;
  OBJ_DATA *bloodvial4;
  OBJ_DATA *bloodvial5;
  OBJ_DATA *bloodvial6;
  OBJ_DATA *rabbit;
  OBJ_DATA *turkey;
  OBJ_DATA *lettuce;
  OBJ_DATA *lamb;
  OBJ_DATA *apple;
  OBJ_DATA *almonds;

  if (IS_UNDEAD(ch))
  {
    switch ( number_range(0,6) )
    {
      case 0:
        bloodvial1 = create_object( get_obj_index( OBJ_VNUM_BLOODVIAL1 ), 0 );
        act("$n utters the arcane words as a small vial of ogre blood appears.", ch, bloodvial1, NULL, TO_ROOM );
        act("You conjure $p.", ch, bloodvial1, NULL, TO_CHAR );
        obj_to_room( bloodvial1, ch->in_room );
        break;

      case 1:
        bloodvial2 = create_object( get_obj_index( OBJ_VNUM_BLOODVIAL2 ), 0 );
        act("$n utters the arcane words as a small vial of human blood appears.", ch, bloodvial2, NULL, TO_ROOM );
        act("You conjure $p.", ch, bloodvial2, NULL, TO_CHAR );
        obj_to_room( bloodvial2, ch->in_room );
        break;  

      case 2:
        bloodvial3 = create_object( get_obj_index( OBJ_VNUM_BLOODVIAL3 ), 0 );
        act("$n utters the arcane words as a small vial of elven blood appears.", ch, bloodvial3, NULL, TO_ROOM );
        act("You conjure $p.", ch, bloodvial3, NULL, TO_CHAR );
        obj_to_room( bloodvial3, ch->in_room );
        break; 

      case 3:
        bloodvial4 = create_object( get_obj_index( OBJ_VNUM_BLOODVIAL4 ), 0 );
        act("$n utters the arcane words as a small vial of dwarven blood appears.", ch, bloodvial4, NULL, TO_ROOM );
        act("You conjure $p.", ch, bloodvial4, NULL, TO_CHAR );
        obj_to_room( bloodvial4, ch->in_room );
        break;  

      case 4:
        bloodvial5 = create_object( get_obj_index( OBJ_VNUM_BLOODVIAL5 ), 0 );
        act("$n utters the arcane words as a small vial of kender blood appears.", ch, bloodvial5, NULL, TO_ROOM );
        act("You conjure $p.", ch, bloodvial5, NULL, TO_CHAR );
        obj_to_room( bloodvial5, ch->in_room );
        break; 

      case 5:
        bloodvial6 = create_object( get_obj_index( OBJ_VNUM_BLOODVIAL6 ), 0 );
        act("$n utters the arcane words as a small vial of dragon blood appears.", ch, bloodvial6, NULL, TO_ROOM );
        act("You conjure $p.", ch, bloodvial6, NULL, TO_CHAR );
        obj_to_room( bloodvial6, ch->in_room );
        break; 
    }
  return;
  }
  else
  {
    switch ( number_range(0,6) )
    {
      case 0:
        rabbit = create_object( get_obj_index( OBJ_VNUM_RABBIT ), 0 );
        act( "$p appears as $n speaks magical words.",ch, rabbit, NULL, TO_ROOM );
        act("You conjure $p.", ch, rabbit, NULL, TO_CHAR );
        obj_to_room( rabbit, ch->in_room );
        break;

      case 1:
        turkey = create_object( get_obj_index( OBJ_VNUM_TURKEY ), 0 );
        act( "$p appears as $n speaks magical words.",ch, turkey, NULL, TO_ROOM );
        act ("You conjure forth $p.",ch, turkey, NULL, TO_CHAR );
        obj_to_room( turkey, ch->in_room );
        break;  

      case 2:
        lettuce = create_object( get_obj_index( OBJ_VNUM_LETTUCE ), 0 );
        act("$n conjures forth $p.", ch, lettuce, NULL, TO_ROOM );
        act ("You conjure forth $p.",ch, lettuce, NULL, TO_CHAR );
        obj_to_room( lettuce, ch->in_room );
        break;  

      case 3:
        lamb = create_object( get_obj_index( OBJ_VNUM_LAMB ), 0 );
        act("$n conjures forth $p.", ch, lamb, NULL, TO_ROOM );
        act ("You conjure forth $p.",ch, lamb, NULL, TO_CHAR );
        obj_to_room( lamb, ch->in_room );
        break;  

      case 4:
        apple = create_object( get_obj_index( OBJ_VNUM_APPLE ), 0 );
        act( " $n conjures forth $p.", ch, apple, NULL, TO_ROOM );
        act ("You conjure forth $p.",ch, apple, NULL, TO_CHAR );
        obj_to_room( apple, ch->in_room );
        break;  

      case 5:
        almonds = create_object( get_obj_index( OBJ_VNUM_ALMONDS ), 0 );
        act("$n conjures forth $p.", ch, almonds, NULL, TO_ROOM );
        act ("You conjure forth $p.",ch, almonds, NULL, TO_CHAR );
        obj_to_room( almonds, ch->in_room );
        break;  
    }
  return;
  }
  return;
}

void spell_create_food( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    OBJ_DATA *mushroom;

    mushroom = create_object( get_obj_index( OBJ_VNUM_MUSHROOM ), 0 );
    obj_to_room( mushroom, ch->in_room );
    act( "$p suddenly appears.", ch, mushroom, NULL, TO_ROOM );
    act( "$p suddenly appears.", ch, mushroom, NULL, TO_CHAR );
    return;
}

void spell_create_rose( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    OBJ_DATA *rose;
    rose = create_object(get_obj_index(OBJ_VNUM_ROSE), 0);
    act("$n has created a beautiful red rose.",ch,rose,NULL,TO_ROOM);
    send_to_char("You create a beautiful red rose.\n\r",ch);
    if ( ch->carry_number + get_obj_number( rose ) > can_carry_n( ch ) )
    {
      obj_to_room( rose, ch->in_room );
    }
    else
    {
      obj_to_char(rose,ch);
    }
    return;
}

void spell_create_spring(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
  OBJ_DATA *spring;
  OBJ_DATA *obj;
  ROOM_INDEX_DATA *location;
  int count;
  int sprchk;
  int sprchk1;

/*if they cast it inside*/
  if (ch->in_room->sector_type == SECT_INSIDE)
  {
    send_to_char("A faint wet spot appears on the floor. Maybe it would\n\r",ch);
    send_to_char("be better to cast that outside?\n\r",ch);
    return;
  }

  if (IS_SET(ch->in_room->room_flags,ROOM_INDOORS))
  {
    send_to_char("A faint wet spot appears on the floor. Maybe it would\n\r",ch);
    send_to_char("be better to cast that outside?\n\r",ch);
    return;
  }

/*if they cast it in water*/
  if (ch->in_room->sector_type == SECT_WATER_SWIM)
  {
    send_to_char("The water around you sparkles slightly for a second.\n\r",ch);
    return;
  }

/*yet again in water*/
  if (ch->in_room->sector_type == SECT_WATER_NOSWIM)
  {
    send_to_char("The water around you sparkles slightly for a second.\n\r",ch);
    return;
  }

/*make a spring in the air?*/
  if (ch->in_room->sector_type == SECT_AIR)
  {
    send_to_char("A few drops of liquid fall down through the air to the ground below you.\n\r",ch);
    return;
  }

/*springs in the streets*/
  if (ch->in_room->sector_type == SECT_CITY
  && !IS_SET(ch->in_room->room_flags,ROOM_INDOORS))
  {
    location = ch->in_room;
    count = 0;
    sprchk = 0;

    /*check to see if there is already a spring created in that room*/
    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
      if((obj->pIndexData->vnum == OBJ_VNUM_SPRING)
	&& (obj->in_room->vnum == ch->in_room->vnum))
        sprchk++;
    }

    /*if there is already a spring in that room, can't create another*/
    if (sprchk >= 1)
    {
      send_to_char("The main pipe has already been ruptured here. You couldn't possibly rupture it here again!\n\r",ch);
      return;
    }

    /*check to see how many springs were created in a city and count them*/
    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
      if((obj->pIndexData->vnum == OBJ_VNUM_SPRING)
	&& (obj->in_room->area == ch->in_room->area))
        count++;
    }

    /*if there are at least 2 springs in the city, no more can be created*/
    if (count >= 2)
    {
      send_to_char("There have been too many ruptures in the city's main pipes.\n\r",ch);
      send_to_char("There isn't enough pressure left in the pipes to rupture them anywhere else.\n\r",ch);
      return;
    }

    send_to_char("You rupture a water main! Water shoots up into the air flooding the streets!\n\r",ch);
    act( "Water shoots up into the air, flooding the streets.", ch, NULL, NULL, TO_ROOM );
    spring = create_object( get_obj_index( OBJ_VNUM_SPRING ), 0 );
    spring->timer = level/2;
    obj_to_room( spring, ch->in_room );
    free_string(spring->name);
    spring->name=str_dup("geyser water");
    free_string(spring->short_descr);
    spring->short_descr=str_dup("a spouting geyser of water");
    free_string(spring->description);
    spring->description=str_dup("A huge geyser sprays up into the air here.");
    spring->wear_flags=0;
    spring->extra_flags=16384; 
    return;
  }

/*springs anywhere else*/

    sprchk1 = 0;

    /*check to see if there is already a spring created in that room*/
    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
      if((obj->pIndexData->vnum == OBJ_VNUM_SPRING)
	&& (obj->in_room->vnum == ch->in_room->vnum))
        sprchk1++;
    }

    /*if there is already a spring in that room, can't create another*/
    if (sprchk1 >= 1)
    {
      send_to_char("There already is a sparkling spring bubbling here!\n\r",ch);
      return;
    }

    spring = create_object( get_obj_index( OBJ_VNUM_SPRING ), 0 );
    spring->timer = level;
    free_string(spring->name);
    spring->name=str_dup("spring.");
    free_string(spring->short_descr);
    spring->short_descr=str_dup("a sparkling spring");
    free_string(spring->description);
    spring->description=str_dup("A sparkling spring bubbles up from the ground.");
    obj_to_room( spring, ch->in_room );
    act( "Mist coalesces, swirls and suddenly $p begins to flow.",ch,spring,NULL,TO_ROOM);
    act( "Mist coalesces, swirls and suddenly $p begins to flow.",ch,spring,NULL,TO_CHAR);
    return;
}



void spell_create_water( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    int water;

    if ( obj->item_type != ITEM_DRINK_CON )
    {
	send_to_char( "It is unable to hold water.\n\r", ch );
	return;
    }

    if ( obj->value[2] != LIQ_WATER && obj->value[1] != 0 )
    {
	send_to_char( "It contains some other liquid.\n\r", ch );
	return;
    }

    water = UMIN(
		level * (weather_info.sky >= SKY_RAINING ? 4 : 2),
		obj->value[0] - obj->value[1]
		);
  
    if ( water > 0 )
    {
	obj->value[2] = LIQ_WATER;
	obj->value[1] += water;
	if ( !is_name( "water", obj->name ) )
	{
	    char buf[MAX_STRING_LENGTH];

	    sprintf( buf, "%s water", obj->name );
	    free_string( obj->name );
	    obj->name = str_dup( buf );
	}
	act( "$p is filled.", ch, obj, NULL, TO_CHAR );
    }

    return;
}



void spell_cure_blindness(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if ( !is_affected( victim, gsn_blindness ) )
    {
        if (victim == ch)
          send_to_char("You aren't blind.\n\r",ch);
        else
          act("$N doesn't appear to be blinded.",ch,NULL,victim,TO_CHAR);
        return;
    }
 
    if (check_dispel(level,victim,gsn_blindness))
    {
        send_to_char( "Your vision returns!\n\r", victim );
        act("$n is no longer blinded.",victim,NULL,NULL,TO_ROOM);
    }
    else
        send_to_char("Spell failed.\n\r",ch);
}



void spell_cure_critical( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA *af;
    int heal;

    for (af = victim->affected ; af ; af = af->next)
    {
	if (af->type == gsn_subdue)
	{
	  affect_remove(victim, af);
	}	
    }

    heal = dice(3, 8) + level - 6;
    victim->hit = UMIN( victim->hit + heal, victim->max_hit );
    update_pos( victim );
    send_to_char( "You feel better!\n\r", victim );
    if ( ch != victim )
	send_to_char( "Ok.\n\r", ch );
    return;
}

/* RT added to cure plague */
void spell_cure_disease( int sn, int level, CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if ( !is_affected( victim, gsn_plague ) )
    {
        if (victim == ch)
          send_to_char("You aren't ill.\n\r",ch);
        else
          act("$N doesn't appear to be diseased.",ch,NULL,victim,TO_CHAR);
        return;
    }
    
    if (check_dispel(level,victim,gsn_plague))
    {
	send_to_char("Your sores vanish.\n\r",victim);
	act("$n looks relieved as $s sores vanish.",victim,NULL,NULL,TO_ROOM);
    }
    else
	send_to_char("Spell failed.\n\r",ch);
}



void spell_cure_light( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA *af;
    int heal;

    for (af = victim->affected ; af ; af = af->next)
    {
	if (af->type == gsn_subdue)
	{
	  affect_remove(victim, af);
	}	
    }

    heal = dice(1, 8) + level / 3;
    victim->hit = UMIN( victim->hit + heal, victim->max_hit );
    update_pos( victim );
    send_to_char( "You feel better!\n\r", victim );
    if ( ch != victim )
	send_to_char( "Ok.\n\r", ch );
    return;
}



void spell_cure_poison( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
 
    if ( !is_affected( victim, gsn_poison ) )
    {
        if (victim == ch)
          send_to_char("You aren't poisoned.\n\r",ch);
        else
          act("$N doesn't appear to be poisoned.",ch,NULL,victim,TO_CHAR);
        return;
    }
 
    if (check_dispel(level,victim,gsn_poison))
    {
        send_to_char("A warm feeling runs through your body.\n\r",victim);
        act("$n looks much better.",victim,NULL,NULL,TO_ROOM);
    }
    else
        send_to_char("Spell failed.\n\r",ch);
}

void spell_cure_serious( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA *af;
    int heal;

    for (af = victim->affected ; af ; af = af->next)
    {
	if (af->type == gsn_subdue)
	{
	  affect_remove(victim, af);
	}	
    }

    heal = dice(2, 8) + level /2 ;
    victim->hit = UMIN( victim->hit + heal, victim->max_hit );
    update_pos( victim );
    send_to_char( "You feel better!\n\r", victim );
    if ( ch != victim )
	send_to_char( "Ok.\n\r", ch );
    return;
}



void spell_curse( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim;
    AFFECT_DATA af;
    victim = (CHAR_DATA *) vo;

    if (IS_AFFECTED(ch,AFF_BLIND))
    {
	send_to_char("Cast the spell on whom? Your blind as a bat.\n\r",ch);
	return;
    }

    if (IS_AFFECTED(victim,AFF_CURSE) || saves_spell(level,victim,DAM_NEGATIVE))
	return;
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 2*level;
    af.location  = APPLY_HITROLL;
    af.modifier  = -1 * (level / 8);
    af.bitvector = AFF_CURSE;
    affect_to_char( victim, &af );

    af.location  = APPLY_SAVING_SPELL;
    af.modifier  = level / 8;
    affect_to_char( victim, &af );

    send_to_char( "You feel unclean.\n\r", victim );
    if ( ch != victim )
	act("$N looks very uncomfortable.",ch,NULL,victim,TO_CHAR);
    return;
}

/* RT replacement demonfire spell */

void spell_demonfire(int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    if ( !IS_NPC(ch) && !IS_EVIL(ch) )
    {
        victim = ch;
	send_to_char("The demons turn upon you!\n\r",ch);
    }

    ch->alignment = UMAX(-1000, ch->alignment - 50);

    if (victim != ch)
    {
	act("$n calls forth demons from the Abyss upon $N!",
	    ch,NULL,victim,TO_ROOM);
        act("$n has assailed you with the demons of Hell!",
	    ch,NULL,victim,TO_VICT);
	send_to_char("You conjure forth demons from the Abyss!\n\r",ch);
    }
    dam = dice( level, 10 );
    if ( saves_spell( level, victim,DAM_NEGATIVE) )
        dam /= 2;
    damage( ch, victim, dam, sn, DAM_NEGATIVE ,TRUE);
    spell_curse(gsn_curse, 3 * level / 4, ch, (void *) victim,TARGET_CHAR);
}

void spell_detect_evil( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_DETECT_EVIL) )
    {
	if (victim == ch)
	  send_to_char("You can already sense evil.\n\r",ch);
	else
	  act("$N can already detect evil.",ch,NULL,victim,TO_CHAR);
	return;
    }
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = level;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_DETECT_EVIL;
    affect_to_char( victim, &af );
    send_to_char( "Your eyes tingle.\n\r", victim );
    if ( ch != victim )
	send_to_char( "Ok.\n\r", ch );
    return;
}


void spell_detect_good( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
 
    if ( IS_AFFECTED(victim, AFF_DETECT_GOOD) )
    {
        if (victim == ch)
          send_to_char("You can already sense good.\n\r",ch);
        else
          act("$N can already detect good.",ch,NULL,victim,TO_CHAR);
        return;
    }
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_DETECT_GOOD;
    affect_to_char( victim, &af );
    send_to_char( "Your eyes tingle.\n\r", victim );
    if ( ch != victim )
        send_to_char( "Ok.\n\r", ch );
    return;
}



void spell_detect_hidden(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_DETECT_HIDDEN) )
    {
        if (victim == ch)
          send_to_char("You are already as alert as you can be. \n\r",ch);
        else
          act("$N can already sense hidden lifeforms.",ch,NULL,victim,TO_CHAR);
        return;
    }
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_DETECT_HIDDEN;
    affect_to_char( victim, &af );
    send_to_char( "Your awareness improves.\n\r", victim );
    if ( ch != victim )
	send_to_char( "Ok.\n\r", ch );
    return;
}

void spell_forest_vision(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED2(victim, AFF_FOREST_VISION) )
    {
       if (victim == ch)
         send_to_char("You can already see into the forest blendings. \n\r",ch);
       else
	   act("$N already has forest vision.",ch,NULL,victim,TO_CHAR);
       return;
     }
     af.where        = TO_AFFECTS2;
     af.type         = sn;
     af.level        = level;
     af.duration     = level;
     af.location     = APPLY_NONE;
     af.modifier     = 0;
     af.bitvector    = AFF_FOREST_VISION;
     affect_to_char( victim, &af );
     send_to_char( "Your eyes adjust to the under-growth of the forest.\n\r", victim );
     if (ch != victim )
       send_to_char( "Ok.\n\r", ch );
     return;
}



void spell_detect_invis( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_DETECT_INVIS) )
    {
        if (victim == ch)
          send_to_char("You can already see invisible.\n\r",ch);
        else
          act("$N can already see invisible things.",ch,NULL,victim,TO_CHAR);
        return;
    }

    af.where     		= TO_AFFECTS;
    af.type      		= sn;
    af.level     		= level;
    af.duration  		= level;
    af.modifier  		= 0;
    af.location  		= APPLY_NONE;
    af.bitvector 		= AFF_DETECT_INVIS;
    affect_to_char( victim, &af );
    send_to_char( "Your eyes tingle.\n\r", victim );
    if ( ch != victim )
	send_to_char( "Ok.\n\r", ch );
    return;
}

void spell_detect_magic( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_DETECT_MAGIC) )
    {
        if (victim == ch)
          send_to_char("You can already sense magical auras.\n\r",ch);
        else
          act("$N can already detect magic.",ch,NULL,victim,TO_CHAR);
        return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = level;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_DETECT_MAGIC;
    affect_to_char( victim, &af );
    send_to_char( "Your eyes tingle.\n\r", victim );
    if ( ch != victim )
	send_to_char( "Ok.\n\r", ch );
    return;
}



void spell_detect_poison( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;

    if ( obj->item_type == ITEM_DRINK_CON || obj->item_type == ITEM_FOOD )
    {
	if ( obj->value[3] != 0 )
	    send_to_char( "You smell poisonous fumes.\n\r", ch );
	else
	    send_to_char( "It looks delicious.\n\r", ch );
    }
    else
    {
	send_to_char( "It doesn't look poisoned.\n\r", ch );
    }

    return;
}



void spell_dispel_evil( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
  
    if ( !IS_NPC(ch) && IS_EVIL(ch) )
	victim = ch;
  
    if ( IS_GOOD(victim) )
    {
      act( "The Gods protect $N.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( IS_NEUTRAL(victim) )
    {
	act( "$N does not seem to be affected.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if (victim->hit > (ch->level * 4))
      dam = dice( level, 4 );
    else
      dam = UMAX(victim->hit, dice(level,4));
    if ( saves_spell( level, victim,DAM_HOLY) )
	dam /= 2;
    damage( ch, victim, dam, sn, DAM_HOLY ,TRUE);
    return;
}


void spell_dispel_good( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
 
    if ( !IS_NPC(ch) && IS_GOOD(ch) )
        victim = ch;
 
    if ( IS_EVIL(victim) )
    {
        act( "$N is protected by $S evil.", ch, NULL, victim, TO_ROOM );
        return;
    }
 
    if ( IS_NEUTRAL(victim) )
    {
        act( "$N does not seem to be affected.", ch, NULL, victim, TO_CHAR );
        return;
    }
 
    if (victim->hit > (ch->level * 4))
      dam = dice( level, 4 );
    else
      dam = UMAX(victim->hit, dice(level,4));
    if ( saves_spell( level, victim,DAM_NEGATIVE) )
        dam /= 2;
    damage( ch, victim, dam, sn, DAM_NEGATIVE ,TRUE);
    return;
}


/* modified for enhanced use */

void spell_dispel_magic( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    bool found = FALSE;

    if (saves_spell(level, victim,DAM_OTHER))
    {
	send_to_char( "You feel a brief tingling sensation.\n\r",victim);
	send_to_char( "You failed.\n\r", ch);
	return;
    }

    /* begin running through the spells */ 

    if (check_dispel(level,victim,skill_lookup("armor")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("bless")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("blindness")))
    {
        found = TRUE;
        act("$n is no longer blinded.",victim,NULL,NULL,TO_ROOM);
    }
 
    if (check_dispel(level,victim,skill_lookup("calm")))
    {
        found = TRUE;
        act("$n no longer looks so peaceful...",victim,NULL,NULL,TO_ROOM);
    }
 
    if (check_dispel(level,victim,skill_lookup("change sex")))
    {
        found = TRUE;
        act("$n looks more like $mself again.",victim,NULL,NULL,TO_ROOM);
    }
 
    if (check_dispel(level,victim,skill_lookup("charm person")))
    {
        found = TRUE;
        act("$n regains $s free will.",victim,NULL,NULL,TO_ROOM);
    }
 
    if (check_dispel(level,victim,skill_lookup("chill touch")))
    {
        found = TRUE;
        act("$n looks warmer.",victim,NULL,NULL,TO_ROOM);
    }
 
    if (check_dispel(level,victim,skill_lookup("curse")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("detect evil")))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("detect good")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("detect hidden")))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("forest vision")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("detect invis")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("detect magic")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("incendiary cloud")))
    {
        act("$n's outline fades.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("levitate")))
    {
        act("$n falls to the ground!",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("frenzy")))
    {
        act("$n no longer looks so wild.",victim,NULL,NULL,TO_ROOM);;
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("giant strength")))
    {
        act("$n no longer looks so mighty.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("haste")))
    {
        act("$n is no longer moving so quickly.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("infravision")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("invis")))
    {
        act("$n fades into existance.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("mass invis")))
    {
        act("$n fades into existance.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("pass door")))
        found = TRUE;
 

    if (check_dispel(level,victim,skill_lookup("protection evil")))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("protection good")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("sanctuary")))
    {
        act("The white aura around $n's body vanishes.",
            victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }

    if (IS_AFFECTED(victim,AFF_SANCTUARY) 
	&& !saves_dispel(level, victim->level,-1)
	&& !is_affected(victim,skill_lookup("sanctuary")))
    {
	REMOVE_BIT(victim->affected_by,AFF_SANCTUARY);
        act("The white aura around $n's body vanishes.",
            victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("shield")))
    {
        act("The shield protecting $n vanishes.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("sleep")))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("slow")))
    {
        act("$n is no longer moving so slowly.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("flesh to stone")))
    {
        act("$n's skin regains its normal texture.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("weaken")))
    {
        act("$n looks stronger.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (found)
        send_to_char("Ok.\n\r",ch);
    else
        send_to_char("Spell failed.\n\r",ch);
	return;
}

void spell_earthquake( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;

    send_to_char( "The earth trembles beneath your feet!\n\r", ch );
    act( "$n makes the earth tremble and shiver.", ch, NULL, NULL, TO_ROOM );
    for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
    {
        vch_next        = vch->next_in_room;
        if (IS_IMMORTAL(vch) || vch == ch || is_same_group(vch,ch))
          continue;     
        if (!is_safe_spell(ch,vch,TRUE) )
        {
          if (!IS_NPC(ch) 
          && vch != ch 
          && ch->fighting != vch
          && vch->fighting != ch
          && (IS_AFFECTED(vch,AFF_CHARM)
          || !IS_NPC(vch)))
          {
                spell_yell(ch, vch, NULL);
          }

          if ( vch->in_room == NULL )
            continue;
          {
            if (IS_AFFECTED(vch,AFF_FLYING)) 
              damage(ch,vch,0,sn,DAM_BASH,TRUE);
            else
              damage( ch, vch, level + dice(2, 8), sn, DAM_BASH,TRUE );
          }
          if ( vch->in_room == NULL )
              continue;
          if ( vch->in_room->area == ch->in_room->area )
          {
            send_to_char( "The earth trembles and shivers.\n\r", vch );
          }
       }
    }
    return;
}
void spell_enchant_armor( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA *paf; 
    int result, fail;
    int ac_bonus, added;
    bool ac_found = FALSE;

    if (obj->item_type != ITEM_ARMOR)
    {
	send_to_char("That isn't an armor.\n\r",ch);
	return;
    }

    if (obj->wear_loc != -1)
    {
	send_to_char("The item must be carried to be enchanted.\n\r",ch);
	return;
    }

    if( IS_SET(obj->extra_flags, ITEM_MASTERWORK) )
    {
		send_to_char("This masterworked item cannot be further enchanted.\n\r",ch);
		return;
    }

	if( IS_SET(obj->extra2_flags, ITEM_ARTISTRY) )
	{
		send_to_char("This item has a Cultural Art placed upon it, and cannot be further enchanted.\n\r",ch);
		return;
	}

    /* this means they have no bonus */
    ac_bonus = 0;
    fail = 25;	/* base 25% chance of failure */

    /* find the bonuses */

    if (!obj->enchanted)
    	for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
    	{
            if ( paf->location == APPLY_AC )
            {
	    	ac_bonus = paf->modifier;
		ac_found = TRUE;
	    	fail += 5 * (ac_bonus * ac_bonus);
 	    }

	    else  /* things get a little harder */
	    	fail += 20;
    	}
 
    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
	if ( paf->location == APPLY_AC )
  	{
	    ac_bonus = paf->modifier;
	    ac_found = TRUE;
	    fail += 5 * (ac_bonus * ac_bonus);
	}

	else /* things get a little harder */
	    fail += 20;
    }

    /* apply other modifiers */
    fail -= level;

    if (IS_OBJ_STAT(obj,ITEM_BLESS))
	fail -= 15;
    if (IS_OBJ_STAT(obj,ITEM_GLOW))
	fail -= 5;

    fail = URANGE(5,fail,85);

    result = number_percent();

    /* the moment of truth */
    if (result < (fail / 5))  /* item destroyed */
    {
	act("$p flares blindingly... and evaporates!",ch,obj,NULL,TO_CHAR);
	act("$p flares blindingly... and evaporates!",ch,obj,NULL,TO_ROOM);
	extract_obj(obj);
	return;
    }

    if (result < (fail / 3)) /* item disenchanted */
    {
	AFFECT_DATA *paf_next;

	act("$p glows brightly, then fades...oops.",ch,obj,NULL,TO_CHAR);
	act("$p glows brightly, then fades.",ch,obj,NULL,TO_ROOM);
	obj->enchanted = TRUE;

	/* remove all affects */
	for (paf = obj->affected; paf != NULL; paf = paf_next)
	{
	    paf_next = paf->next; 
	    free_affect(paf);
	}
	obj->affected = NULL;

	/* clear all flags */
	obj->extra_flags = 0;
	return;
    }

    if ( result <= fail )  /* failed, no bad result */
    {
	send_to_char("Nothing seemed to happen that time.\n\r",ch);
	return;
    }

    /* okay, move all the old flags into new vectors if we have to */
    if (!obj->enchanted)
    {
	AFFECT_DATA *af_new;
	obj->enchanted = TRUE;

	for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next) 
	{
	    af_new = new_affect();
	
	    af_new->next = obj->affected;
	    obj->affected = af_new;

	    af_new->where	= paf->where;
	    af_new->type 	= UMAX(0,paf->type);
	    af_new->level	= paf->level;
	    af_new->duration	= paf->duration;
	    af_new->location	= paf->location;
	    af_new->modifier	= paf->modifier;
	    af_new->bitvector	= paf->bitvector;
	}
    }

    if (result <= (90 - level/5))  /* success! */
    {
	act("$p shimmers with a golden aura.",ch,obj,NULL,TO_CHAR);
	act("$p shimmers with a golden aura.",ch,obj,NULL,TO_ROOM);
	SET_BIT(obj->extra_flags, ITEM_MAGIC);
	added = -1;
    }
    
    else  /* exceptional enchant */
    {
	act("$p glows a brilliant gold!",ch,obj,NULL,TO_CHAR);
	act("$p glows a brilliant gold!",ch,obj,NULL,TO_ROOM);
	SET_BIT(obj->extra_flags,ITEM_MAGIC);
	SET_BIT(obj->extra_flags,ITEM_GLOW);
	added = -2;
    }
		
    /* now add the enchantments */ 

    if (obj->level < LEVEL_HERO)
	obj->level = UMIN(LEVEL_HERO - 1,obj->level + 1);

    if (ac_found)
    {
	for ( paf = obj->affected; paf != NULL; paf = paf->next)
	{
	    if ( paf->location == APPLY_AC)
	    {
		paf->type = sn;
		paf->modifier += added;
		paf->level = UMAX(paf->level,level);
	    }
	}
    }
    else /* add a new affect */
    {
 	paf = new_affect();

	paf->where	= TO_OBJECT;
	paf->type	= sn;
	paf->level	= level;
	paf->duration	= -1;
	paf->location	= APPLY_AC;
	paf->modifier	=  added;
	paf->bitvector  = 0;
    	paf->next	= obj->affected;
    	obj->affected	= paf;
    }

}




void spell_enchant_weapon(int sn,int level,CHAR_DATA *ch, void *vo,int target)
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA *paf; 
    int result, fail;
    int hit_bonus, dam_bonus, added;
    bool hit_found = FALSE, dam_found = FALSE;

    if (obj->item_type != ITEM_WEAPON)
    {
	send_to_char("That isn't a weapon.\n\r",ch);
	return;
    }

    if (obj->wear_loc != -1)
    {
	send_to_char("The item must be carried to be enchanted.\n\r",ch);
	return;
    }

    if( IS_SET(obj->extra_flags, ITEM_MASTERWORK) )
    {
		send_to_char("This masterworked item cannot be further enchanced.\n\r",ch);
		return;
    }

	if( IS_SET(obj->extra2_flags, ITEM_ARTISTRY) )
	{
		send_to_char("This item has a Cultural Art placed upon it, and cannot be further enchanted.\n\r",ch);
		return;
	}

    /* this means they have no bonus */
    hit_bonus = 0;
    dam_bonus = 0;
    fail = 25;	/* base 25% chance of failure */

    /* find the bonuses */

    if (!obj->enchanted)
    	for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
    	{
            if ( paf->location == APPLY_HITROLL )
            {
	    	hit_bonus = paf->modifier;
		hit_found = TRUE;
	    	fail += 2 * (hit_bonus * hit_bonus);
 	    }

	    else if (paf->location == APPLY_DAMROLL )
	    {
	    	dam_bonus = paf->modifier;
		dam_found = TRUE;
	    	fail += 2 * (dam_bonus * dam_bonus);
	    }

	    else  /* things get a little harder */
	    	fail += 25;
    	}
 
    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
	if ( paf->location == APPLY_HITROLL )
  	{
	    hit_bonus = paf->modifier;
	    hit_found = TRUE;
	    fail += 2 * (hit_bonus * hit_bonus);
	}

	else if (paf->location == APPLY_DAMROLL )
  	{
	    dam_bonus = paf->modifier;
	    dam_found = TRUE;
	    fail += 2 * (dam_bonus * dam_bonus);
	}

	else /* things get a little harder */
	    fail += 25;
    }

    /* apply other modifiers */
    fail -= 3 * level/2;

    if (IS_OBJ_STAT(obj,ITEM_BLESS))
	fail -= 15;
    if (IS_OBJ_STAT(obj,ITEM_GLOW))
	fail -= 5;

    fail = URANGE(5,fail,95);

    result = number_percent();

    /* the moment of truth */
    if (result < (fail / 5))  /* item destroyed */
    {
	act("$p shivers violently and explodes into tiny pieces!",ch,obj,NULL,TO_CHAR);
	act("$p shivers violently and explodes into tiny pieces!",ch,obj,NULL,TO_ROOM);
	extract_obj(obj);
	return;
    }

    if (result < (fail / 2)) /* item disenchanted */
    {
	AFFECT_DATA *paf_next;

	act("$p glows brightly, then fades...oops.",ch,obj,NULL,TO_CHAR);
	act("$p glows brightly, then fades.",ch,obj,NULL,TO_ROOM);
	obj->enchanted = TRUE;

	/* remove all affects */
	for (paf = obj->affected; paf != NULL; paf = paf_next)
	{
	    paf_next = paf->next; 
	    free_affect(paf);
	}
	obj->affected = NULL;

	/* clear all flags */
	obj->extra_flags = 0;
	return;
    }

    if ( result <= fail )  /* failed, no bad result */
    {
	send_to_char("Nothing seemed to happen.\n\r",ch);
	return;
    }

    /* okay, move all the old flags into new vectors if we have to */
    if (!obj->enchanted)
    {
	AFFECT_DATA *af_new;
	obj->enchanted = TRUE;

	for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next) 
	{
	    af_new = new_affect();
	
	    af_new->next = obj->affected;
	    obj->affected = af_new;

	    af_new->where	= paf->where;
	    af_new->type 	= UMAX(0,paf->type);
	    af_new->level	= paf->level;
	    af_new->duration	= paf->duration;
	    af_new->location	= paf->location;
	    af_new->modifier	= paf->modifier;
	    af_new->bitvector	= paf->bitvector;
	}
    }

    if (result <= (100 - level/5))  /* success! */
    {
	act("$p glows blue.",ch,obj,NULL,TO_CHAR);
	act("$p glows blue.",ch,obj,NULL,TO_ROOM);
	SET_BIT(obj->extra_flags, ITEM_MAGIC);
	added = 1;
    }
    
    else  /* exceptional enchant */
    {
	act("$p glows a brilliant blue!",ch,obj,NULL,TO_CHAR);
	act("$p glows a brilliant blue!",ch,obj,NULL,TO_ROOM);
	SET_BIT(obj->extra_flags,ITEM_MAGIC);
	SET_BIT(obj->extra_flags,ITEM_GLOW);
	added = 2;
    }
		
    /* now add the enchantments */ 

    if (obj->level < LEVEL_HERO - 1)
	obj->level = UMIN(LEVEL_HERO - 1,obj->level + 1);

    if (dam_found)
    {
	for ( paf = obj->affected; paf != NULL; paf = paf->next)
	{
	    if ( paf->location == APPLY_DAMROLL)
	    {
		paf->type = sn;
		paf->modifier += added;
		paf->level = UMAX(paf->level,level);
		if (paf->modifier > 4)
		    SET_BIT(obj->extra_flags,ITEM_HUM);
	    }
	}
    }
    else /* add a new affect */
    {
	paf = new_affect();

	paf->where	= TO_OBJECT;
	paf->type	= sn;
	paf->level	= level;
	paf->duration	= -1;
	paf->location	= APPLY_DAMROLL;
	paf->modifier	=  added;
	paf->bitvector  = 0;
    	paf->next	= obj->affected;
    	obj->affected	= paf;
    }

    if (hit_found)
    {
        for ( paf = obj->affected; paf != NULL; paf = paf->next)
	{
            if ( paf->location == APPLY_HITROLL)
            {
		paf->type = sn;
                paf->modifier += added;
                paf->level = UMAX(paf->level,level);
                if (paf->modifier > 4)
                    SET_BIT(obj->extra_flags,ITEM_HUM);
            }
	}
    }
    else /* add a new affect */
    {
        paf = new_affect();
 
        paf->type       = sn;
        paf->level      = level;
        paf->duration   = -1;
        paf->location   = APPLY_HITROLL;
        paf->modifier   =  added;
        paf->bitvector  = 0;
        paf->next       = obj->affected;
        obj->affected   = paf;
    }

}



/*
 * Drain XP, MANA, HP.
 * Caster gains HP.
 */
void spell_energy_drain( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    if (victim != ch)
        ch->alignment = UMAX(-1000, ch->alignment - 50);

    if ( saves_spell( level, victim,DAM_NEGATIVE) )
    {
	send_to_char("You feel a momentary chill.\n\r",victim);  	
	return;
    }


    if ( victim->level <= 2 )
    {
	dam = ch->hit + 1;
      send_to_char("You feel your life slipping away!\n\r",victim);
      send_to_char("What a waste of time. They are so weak compared to you, your magic did a HUGE amount of damage!\n\r",ch);
      damage( ch, victim, dam, sn, DAM_NEGATIVE ,TRUE);
      return;
    }
    else
    {
	gain_exp( victim, 0 - number_range( level/2, 3 * level / 2 ) );
	victim->mana	/= 2;
	victim->move	/= 2;
	dam		 = dice(1, level);
	ch->hit		+= dam;
    }

    send_to_char("You feel your life slipping away!\n\r",victim);
    send_to_char("Wow....what a rush!\n\r",ch);
    damage( ch, victim, dam, sn, DAM_NEGATIVE ,TRUE);

    return;
}



void spell_fireball( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    static const sh_int dam_each[] = 
    {
	  0,
	  0,   0,   0,   0,   0,	  0,   0,   0,   0,   0,
	  0,   0,   0,   0,  30,	 35,  40,  45,  50,  55,
	 60,  65,  70,  75,  80,	 82,  84,  86,  88,  90,
	 92,  94,  96,  98, 100,	102, 104, 106, 108, 110,
	112, 114, 116, 118, 120,	122, 124, 126, 128, 130
    };
    int dam;

    if ( (ch->in_room->sector_type) == SECT_UNDERWATER )
    {
   	 send_to_char( "Your fireball harmlessly fizzles out in the water.\n\r", ch );
    	 act( "$n's fireball harmlessly fizzles out in the water.", ch, NULL, NULL, TO_ROOM );
   	 return;
    }

    level	= UMIN(level, sizeof(dam_each)/sizeof(dam_each[0]) - 1);
    level	= UMAX(0, level);
    dam		= number_range( dam_each[level] / 2, dam_each[level] * 2 );
    if ( saves_spell( level, victim, DAM_FIRE) )
	dam /= 2;
    damage( ch, victim, dam, sn, DAM_FIRE ,TRUE);
    return;
}


void spell_fireproof(int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA af;
 
    if (IS_OBJ_STAT(obj,ITEM_BURN_PROOF))
    {
        act("$p is already protected from burning.",ch,obj,NULL,TO_CHAR);
        return;
    }
 
    af.where     = TO_OBJECT;
    af.type      = sn;
    af.level     = level;
    af.duration  = number_fuzzy(level / 4);
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = ITEM_BURN_PROOF;
 
    affect_to_obj(obj,&af);
 
    act("You protect $p from fire.",ch,obj,NULL,TO_CHAR);
    act("$p is surrounded by a protective aura.",ch,obj,NULL,TO_ROOM);
}



void spell_flamestrike( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = dice(6 + level / 2, 8);
    if ( saves_spell( level, victim,DAM_FIRE) )
	dam /= 2;
    damage( ch, victim, dam, sn, DAM_FIRE ,TRUE);
    return;
}



void spell_incendiary_cloud( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_AFFECTED(ch,AFF_BLIND))
    {
	send_to_char("Cast the spell on whom? Your blind as a bat.\n\r",ch);
	return;
    }

    if ( IS_AFFECTED(victim, AFF_INCENDIARY_CLOUD) )
	return;

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.location  = APPLY_AC;
    af.modifier  = 2 * level;
    af.bitvector = AFF_INCENDIARY_CLOUD;
    affect_to_char( victim, &af );
    send_to_char( "You are surrounded by an incendiary cloud of fire.\n\r", victim );
    act( "$n is surrounded by an incendiary cloud of fire.", victim, NULL, NULL, TO_ROOM );
    return;
}



void spell_glitterdust( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *ich;
    AFFECT_DATA af;

    act( "The room is suddenly filled with blinding glitterdust.", ch, NULL, NULL, TO_ROOM );
    send_to_char( "You conjure forth blinding glitterdust into the room.\n\r", ch );

    for ( ich = ch->in_room->people; ich != NULL; ich = ich->next_in_room )
    {
	if (ich->invis_level > 0)
	    continue;

	if ( ich == ch || saves_spell( level, ich,DAM_OTHER) )
	    continue;

	affect_strip ( ich, gsn_wraithform		);
	affect_strip ( ich, gsn_invis			);
	affect_strip ( ich, gsn_mass_invis		);
	affect_strip ( ich, gsn_sneak			);
	REMOVE_BIT	 ( ich->affected2_by, AFF_WRAITHFORM	);
	REMOVE_BIT	 ( ich->affected2_by, AFF_WRAITHFORM	);
	REMOVE_BIT   ( ich->affected_by, AFF_HIDE	);
      REMOVE_BIT   ( ich->affected2_by, AFF_CAMOUFLAGE );
	REMOVE_BIT   ( ich->affected_by, AFF_INVISIBLE	);
	REMOVE_BIT   ( ich->affected_by, AFF_SNEAK	);
    	af.where		= TO_AFFECTS;
    	af.type      	= sn;
    	af.level	 	= level;
    	af.duration  	= 0;
    	af.modifier  	= 0;
    	af.location  	= 0;
    	af.bitvector 	= AFF_BLIND;
    	affect_to_char( ich, &af );
	act( "$n has been revealed!", ich, NULL, NULL, TO_ROOM );
	send_to_char( "You have been revealed and blinded!\n\r", ich );
    }

    return;
}

void spell_orb_of_containment( int sn, int level,CHAR_DATA *ch,void *vo,int target )
{
    OBJ_DATA *disc, *floating;

    floating = get_eq_char(ch,WEAR_FLOAT);
    if (floating != NULL && IS_OBJ_STAT(floating,ITEM_NOREMOVE))
    {
	act("You can't remove $p.",ch,floating,NULL,TO_CHAR);
	return;
    }

    disc = create_object(get_obj_index(OBJ_VNUM_DISC), 0);
    disc->value[0]	= ch->level * 10; /* 10 pounds per level capacity */
    disc->value[3]	= ch->level * 5; /* 5 pounds per level max per item */
    disc->timer		= ch->level * 3 - number_range(0,level / 2); 

    act("$n has created an orb of containment.",ch,NULL,NULL,TO_ROOM);
    send_to_char("You created an orb of containment.\n\r",ch);
    obj_to_char(disc,ch);
    wear_obj(ch,disc,TRUE);
    return;
}


void spell_levitate( int sn, int level, CHAR_DATA *ch, void *vo,int target )
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
	  send_to_char("You are already levitating.\n\r",ch);
	else
	  act("$N doesn't need your help to levitate.",ch,NULL,victim,TO_CHAR);
	return;
    }

    if ( IS_AFFECTED3(victim, AFF_QUICKSAND) )
    {
	send_to_char("Your waist deep in quicksand, you couldn't possibly get airborne!\n\r",ch);
      return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level + 3;
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = AFF_FLYING;
    affect_to_char( victim, &af );
    send_to_char( "Your feet rise off the ground.\n\r", victim );
    act( "$n's feet rise off the ground.", victim, NULL, NULL, TO_ROOM );
    return;
}

/* RT clerical berserking spell */

void spell_frenzy(int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected(victim,sn) || IS_AFFECTED(victim,AFF_BERSERK))
    {
	if (victim == ch)
	  send_to_char("You are already in a frenzy.\n\r",ch);
	else
	  act("$N is already in a frenzy.",ch,NULL,victim,TO_CHAR);
	return;
    }

    if (is_affected(victim,skill_lookup("calm")))
    {
	if (victim == ch)
	  send_to_char("Why don't you just relax for a while?\n\r",ch);
	else
	  act("$N doesn't look like $e wants to fight anymore.",
	      ch,NULL,victim,TO_CHAR);
	return;
    }

    if (((IS_GOOD(ch) && !IS_GOOD(victim))
    ||  (IS_NEUTRAL(ch) && !IS_NEUTRAL(victim))
    ||  (IS_EVIL(ch) && !IS_EVIL(victim)))
    &&  !IS_IMMORTAL(ch))
    {
	act("Your god doesn't seem to like $N",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     	= TO_AFFECTS;
    af.type 	= sn;
    af.level	= level;
    af.duration	= level / 3;
    af.modifier  	= level / 6;
    af.bitvector 	= 0;

    af.location  	= APPLY_HITROLL;
    affect_to_char(victim,&af);

    af.location  	= APPLY_DAMROLL;
    affect_to_char(victim,&af);

    af.modifier  	= 10 * (level / 12);
    af.location  	= APPLY_AC;
    affect_to_char(victim,&af);

    send_to_char("You are filled with a holy wrath!\n\r",victim);
    act("$n gets a wild look in $s eyes!",victim,NULL,NULL,TO_ROOM);
}

/* RT ROM-style gate */
    
void spell_gate( int sn, int level, CHAR_DATA *ch, void *vo,int target )
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
    ||   victim->in_room == NULL
    ||   victim->in_room == ch->in_room
    ||   !can_see_room(ch,victim->in_room)
    ||   IS_SET(victim->in_room->room_flags, ROOM_SAFE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
    ||   IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
    ||   IS_SET(victim->in_room->room_flags, ROOM_IMP_ONLY)
    ||   IS_SET(victim->in_room->room_flags, ROOM_GODS_ONLY)
    ||   IS_SET(victim->in_room->room_flags, ROOM_HEROES_ONLY)
    ||   IS_SET(victim->in_room->room_flags, ROOM_NEWBIES_ONLY)
    ||   IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
    ||   victim->level >= level + 3
    ||   (!IS_NPC(victim) && victim->level >= LEVEL_HERO)  /* NOT trust */ 
    ||   (IS_NPC(victim) && saves_spell( level, victim,DAM_OTHER)))
    {
        send_to_char( "Your gate fails and snaps shut quickly.\n\r", ch );
        return;
    }

    if (ch->riding)
    {
      send_to_char("You cannot gate while mounted.\n\r",ch);
      return;
    }

    if (ch->pet != NULL && ch->in_room == ch->pet->in_room && !ch->pet->tethered)
	gate_pet = TRUE;
    else
	gate_pet = FALSE;
    
    act("$n steps through a gate and vanishes.",ch,NULL,NULL,TO_ROOM);
    send_to_char("You step through a gate and vanish.\n\r",ch);
    char_from_room(ch);
    char_to_room(ch,victim->in_room);

    act("$n has arrived through a gate.",ch,NULL,NULL,TO_ROOM);
    do_function(ch, &do_look, "auto");

    if (gate_pet)
    {
	act("$n steps through a gate and vanishes.",ch->pet,NULL,NULL,TO_ROOM);
	send_to_char("You step through a gate and vanish.\n\r",ch->pet);
	char_from_room(ch->pet);
	char_to_room(ch->pet,victim->in_room);
	act("$n has arrived through a gate.",ch->pet,NULL,NULL,TO_ROOM);
	do_function(ch->pet, &do_look, "auto");
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



void spell_giant_strength(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
	if (victim == ch)
	  send_to_char("You are already as buff as you can get!\n\r",ch);
	else
	  act("$N can't get any stronger.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = level;
    af.location  = APPLY_STR;
    af.modifier  = 1 + (level >= 18) + (level >= 25) + (level >= 32);
    af.bitvector = 0;
    affect_to_char( victim, &af );
    send_to_char( "Your muscles surge with heightened power!\n\r", victim );
    act("$n's muscles surge with heightened power.",victim,NULL,NULL,TO_ROOM);
    return;
}



void spell_harm( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = UMAX(  20, victim->hit - dice(1,4) );
    if ( saves_spell( level, victim,DAM_HARM) )
	dam = UMIN( 50, dam / 2 );
    dam = UMIN( 100, dam );
    damage( ch, victim, dam, sn, DAM_HARM ,TRUE);
    return;
}

/* RT haste spell */

void spell_haste( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
 
    if ( is_affected( victim, sn ) || IS_AFFECTED(victim,AFF_HASTE)
    ||   IS_SET(victim->off_flags,OFF_FAST))
    {
	if (victim == ch)
	  send_to_char("You can't move any faster!\n\r",ch);
 	else
	  act("$N is already moving as fast as $E can.",
	      ch,NULL,victim,TO_CHAR);
        return;
    }

    if (IS_AFFECTED(victim,AFF_SLOW))
    {
	if (!check_dispel(level,victim,skill_lookup("slow")))
	{
	    if (victim != ch)
	        send_to_char("Spell failed.\n\r",ch);
	    send_to_char("You feel momentarily faster.\n\r",victim);
	    return;
	}
        act("$n is moving less slowly.",victim,NULL,NULL,TO_ROOM);
        return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    if (victim == ch)
      af.duration  = level/2;
    else
      af.duration  = level/4;
    af.location  = APPLY_DEX;
    af.modifier  = 1 + (level >= 18) + (level >= 25) + (level >= 32);
    af.bitvector = AFF_HASTE;
    affect_to_char( victim, &af );
    send_to_char( "You feel yourself moving more quickly.\n\r", victim );
    act("$n is moving more quickly.",victim,NULL,NULL,TO_ROOM);
    if ( ch != victim )
        send_to_char( "Ok.\n\r", ch );
    return;
}



void spell_heal( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    victim->hit = UMIN( victim->hit + 100, victim->max_hit );
    update_pos( victim );
    send_to_char( "A warm feeling fills your body.\n\r", victim );
    if ( ch != victim )
	send_to_char( "Ok.\n\r", ch );
    return;
}

void spell_heat_metal( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    OBJ_DATA *obj_lose, *obj_next;
    int dam = 0;
    bool fail = TRUE;

    if (IS_AFFECTED(ch,AFF_BLIND))
    {
	send_to_char("Cast the spell on whom? Your blind as a bat.\n\r",ch);
	return;
    }
 
   if (!saves_spell(level + 2,victim,DAM_FIRE) 
   &&  !IS_SET(victim->imm_flags,IMM_FIRE))
   {
        for ( obj_lose = victim->carrying;
	      obj_lose != NULL; 
	      obj_lose = obj_next)
        {
	    obj_next = obj_lose->next_content;
            if ( number_range(1,2 * level) > obj_lose->level 
	    &&   !saves_spell(level,victim,DAM_FIRE)
	    &&   !IS_OBJ_STAT(obj_lose,ITEM_NONMETAL)
	    &&   !IS_OBJ_STAT(obj_lose,ITEM_BURN_PROOF))
            {
                switch ( obj_lose->item_type )
                {
               	case ITEM_ARMOR:
		if (obj_lose->wear_loc != -1) /* remove the item */
		{
		    if (can_drop_obj(victim,obj_lose)
		    &&  (obj_lose->weight / 10) < 
			number_range(1,2 * get_curr_stat(victim,STAT_DEX))
		    &&  remove_obj( victim, obj_lose->wear_loc, TRUE ))
		    {
		        act("$n yelps and throws $p to the ground!",
			    victim,obj_lose,NULL,TO_ROOM);
		        act("You remove and drop $p before it burns you.",
			    victim,obj_lose,NULL,TO_CHAR);
			dam += (number_range(1,obj_lose->level) / 3);
                        obj_from_char(obj_lose);
                        obj_to_room(obj_lose, victim->in_room);
                        fail = FALSE;
                    }
		    else /* stuck on the body! ouch! */
		    {
			act("Your skin is seared by $p!",
			    victim,obj_lose,NULL,TO_CHAR);
			dam += (number_range(1,obj_lose->level));
			fail = FALSE;
		    }

		}
		else /* drop it if we can */
		{
		    if (can_drop_obj(victim,obj_lose))
		    {
                        act("$n yelps and throws $p to the ground!",
                            victim,obj_lose,NULL,TO_ROOM);
                        act("You and drop $p before it burns you.",
                            victim,obj_lose,NULL,TO_CHAR);
                        dam += (number_range(1,obj_lose->level) / 6);
                        obj_from_char(obj_lose);
                        obj_to_room(obj_lose, victim->in_room);
			fail = FALSE;
                    }
		    else /* cannot drop */
		    {
                        act("Your skin is seared by $p!",
                            victim,obj_lose,NULL,TO_CHAR);
                        dam += (number_range(1,obj_lose->level) / 2);
			fail = FALSE;
                    }
		}
                break;
                case ITEM_WEAPON:
		if (obj_lose->wear_loc != -1) /* try to drop it */
		{
		    if (IS_WEAPON_STAT(obj_lose,WEAPON_FLAMING))
			continue;

		    if (can_drop_obj(victim,obj_lose) 
		    &&  remove_obj(victim,obj_lose->wear_loc,TRUE))
		    {
			act("$n is burned by $p, and throws it to the ground.",
			    victim,obj_lose,NULL,TO_ROOM);
			send_to_char(
			    "You throw your red-hot weapon to the ground!\n\r",
			    victim);
			dam += 1;
			obj_from_char(obj_lose);
			obj_to_room(obj_lose,victim->in_room);
			fail = FALSE;
		    }
		    else /* YOWCH! */
		    {
			send_to_char("Your weapon sears your flesh!\n\r",
			    victim);
			dam += number_range(1,obj_lose->level);
			fail = FALSE;
		    }
		}
                else /* drop it if we can */
                {
                    if (can_drop_obj(victim,obj_lose))
                    {
                        act("$n throws a burning hot $p to the ground!",
                            victim,obj_lose,NULL,TO_ROOM);
                        act("You and drop $p before it burns you.",
                            victim,obj_lose,NULL,TO_CHAR);
                        dam += (number_range(1,obj_lose->level) / 6);
                        obj_from_char(obj_lose);
                        obj_to_room(obj_lose, victim->in_room);
                        fail = FALSE;
                    }
                    else /* cannot drop */
                    {
                        act("Your skin is seared by $p!",
                            victim,obj_lose,NULL,TO_CHAR);
                        dam += (number_range(1,obj_lose->level) / 2);
                        fail = FALSE;
                    }
                }
                break;
		}
	    }
	}
    } 
    if (fail)
    {
        send_to_char("Your spell had no effect.\n\r", ch);
	send_to_char("You feel momentarily warmer.\n\r",victim);
    }
    else /* damage! */
    {
	if (saves_spell(level,victim,DAM_FIRE))
	    dam = 2 * dam / 3;
	damage(ch,victim,dam,sn,DAM_FIRE,TRUE);
    }
}

/* RT really nasty high-level attack spell */
void spell_holy_word(int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    int dam;
    int bless_num, curse_num, frenzy_num;
   
    bless_num = skill_lookup("bless");
    curse_num = skill_lookup("curse"); 
    frenzy_num = skill_lookup("frenzy");

    act("$n utters a word of divine power!",ch,NULL,NULL,TO_ROOM);
    send_to_char("You utter a word of divine power.\n\r",ch);
 
    for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
    {
        vch_next = vch->next_in_room;

	if ((IS_GOOD(ch) && IS_GOOD(vch)) ||
	    (IS_EVIL(ch) && IS_EVIL(vch)) ||
	    (IS_NEUTRAL(ch) && IS_NEUTRAL(vch)) )
	{
	  spell_frenzy(frenzy_num,level,ch,(void *) vch,TARGET_CHAR); 
	  spell_bless(bless_num,level,ch,(void *) vch,TARGET_CHAR);
	}

	else if ((IS_GOOD(ch) && IS_EVIL(vch)) ||
		 (IS_EVIL(ch) && IS_GOOD(vch)) )
	{
	  if (!is_safe_spell(ch,vch,TRUE))
	  {
            spell_curse(curse_num,level,ch,(void *) vch,TARGET_CHAR);
	    send_to_char("You are struck down!\n\r",vch);
	    dam = dice(level,6);
	    damage(ch,vch,dam,sn,DAM_ENERGY,TRUE);
	  }
	}

        else if (IS_NEUTRAL(ch))
	{
	  if (!is_safe_spell(ch,vch,TRUE))
	  {
            spell_curse(curse_num,level/2,ch,(void *) vch,TARGET_CHAR);
	    send_to_char("You are struck down!\n\r",vch);
	    dam = dice(level,4);
	    damage(ch,vch,dam,sn,DAM_ENERGY,TRUE);
   	  }
	}
    }  
    
    send_to_char("You feel drained after casting this powerful spell.\n\r",ch);
    ch->move = 0;
    ch->hit /= 2;
}
 
void spell_identify( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    char buf[MAX_STRING_LENGTH];
    AFFECT_DATA *paf;

    if (IS_OBJ_STAT(obj,ITEM_NOIDENT)) //C074
    {
        sprintf(buf,"You fail to grasp the basic concept of what it might be.\n\r");
        send_to_char(buf,ch);
    }
    else
    {

        sprintf( buf,
            "A small cloud of ashen smoke appears around it, revealing to you...\n\r\n\rThat it is '%s', and is a type of %s.\n\rIt weighs approximately %d stones, and is valued at %d gold.\n\rThe level at which it holds is %d.\n\r",
    
            obj->short_descr,
            item_name(obj->item_type),
            obj->weight,
            obj->cost,
            obj->level
            );
        send_to_char( buf, ch );
    
        switch ( obj->item_type )
        {
        case ITEM_SCROLL: 
        case ITEM_POTION:
        case ITEM_PILL:
            sprintf( buf, "It contains level %d spells of:", obj->value[0] );
            send_to_char( buf, ch );
    
            if ( obj->value[1] >= 0 && obj->value[1] < MAX_SKILL )
            {
                send_to_char( " '", ch );
                send_to_char( skill_table[obj->value[1]].name, ch );
                send_to_char( "'", ch );
            }
    
            if ( obj->value[2] >= 0 && obj->value[2] < MAX_SKILL )
            {
                send_to_char( " '", ch );
                send_to_char( skill_table[obj->value[2]].name, ch );
                send_to_char( "'", ch );
            }
    
            if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL )
            {
                send_to_char( " '", ch );
                send_to_char( skill_table[obj->value[3]].name, ch );
                send_to_char( "'", ch );
            }
            send_to_char( ".\n\r", ch );
            if (obj->item_type == ITEM_SCROLL)
            {
             send_to_char("It is written in ",ch);
             if ( obj->value[4] >= 0 && obj->value[4] < MAX_SKILL)
             {
               if (obj->value[4] == skill_lookup("dwarven tongue"))
               {
	           send_to_char("the language of the dwarves.\n\r",ch);
		   }
		 else
               if (obj->value[4] == skill_lookup("elven tongue"))
               {
	           send_to_char("elven tongue.\n\r",ch);
		   }
		 else
               if (obj->value[4] == skill_lookup("drow tongue"))
               {
	           send_to_char("the drow tongue.\n\r",ch);
		   }
		 else
               if (obj->value[4] == skill_lookup("ogre tongue"))
               {
	           send_to_char("the ogre tongue.\n\r",ch);
		   }
		 else
               if (obj->value[4] == skill_lookup("kender tongue"))
               {
	           send_to_char("the kender tongue.\n\r",ch);
		   }
		 else
               if (obj->value[4] == skill_lookup("draconian tongue"))
               {
	           send_to_char("the draconian tongue.\n\r",ch);
		   }
		 else
               if (obj->value[4] == skill_lookup("silvanesti tongue"))
               {
	           send_to_char("the language of the silvanesti.\n\r",ch);
		   }
		 else
               if (obj->value[4] == skill_lookup("qualinesti tongue"))
               {
	           send_to_char("the language of the qualinesti.\n\r",ch);
		   }
		 else
               if (obj->value[4] == skill_lookup("kagonesti tongue"))
               {
	           send_to_char("the language of the kagonesti.\n\r",ch);
		   }
		 else
               if (obj->value[4] == skill_lookup("dargonesti tongue"))
               {
	           send_to_char("the language of the dargonesti.\n\r",ch);
		   }
		 else
               if (obj->value[4] == skill_lookup("dimernesti tongue"))
               {
	           send_to_char("the language of the dimernesti.\n\r",ch);
		   }
		 else
               if (obj->value[4] == skill_lookup("minotaur tongue"))
               {
	           send_to_char("minotaur tongue.\n\r",ch);
		   }
		 else
		   {
                 send_to_char("common tongue.\n\r",ch);   
               }
             }
            }
        break;
    
        case ITEM_WAND: 
        case ITEM_STAFF: 
            sprintf( buf, "It has %d charges of level %d",
                obj->value[2], obj->value[0] );
            send_to_char( buf, ch );
          
            if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL )
            {
                send_to_char( " '", ch );
                send_to_char( skill_table[obj->value[3]].name, ch );
                send_to_char( "'", ch );
            }
    
            send_to_char( ".\n\r", ch );
            break;
    
        case ITEM_DRINK_CON:
            sprintf(buf,"It holds %s-colored %s.\n\r",
                liq_table[obj->value[2]].liq_color,
                liq_table[obj->value[2]].liq_name);
            send_to_char(buf,ch);
            break;
    
        case ITEM_CONTAINER:
            sprintf(buf,"It can hold up to as many as: %d items.\n\rIt can carry a maximum weight of: %d stones.\n\rLastly, it is: %s.\n\r",
                obj->value[3], obj->value[0], cont_bit_name(obj->value[1]));
            send_to_char(buf,ch);
            break;
                    
        case ITEM_WEAPON:
            send_to_char("The type of weapon it is, is ",ch);
            switch (obj->value[0])
            {
                case(WEAPON_EXOTIC) : send_to_char("exotic.\n\r",ch);		break;
                case(WEAPON_SWORD)  : send_to_char("a sword.\n\r",ch);		break;	
                case(WEAPON_DAGGER) : send_to_char("a dagger.\n\r",ch);		break;
                case(WEAPON_SPEAR)	: send_to_char("a spear.\n\r",ch);	break;
                case(WEAPON_MACE) 	: send_to_char("a mace/club.\n\r",ch);	break;
                case(WEAPON_AXE)	: send_to_char("an axe.\n\r",ch);		break;
                case(WEAPON_FLAIL)	: send_to_char("a flail.\n\r",ch);		break;
                case(WEAPON_WHIP)	: send_to_char("a whip.\n\r",ch);		break;
                case(WEAPON_POLEARM): send_to_char("a polearm.\n\r",ch);	break;
		    case(WEAPON_BOW)	: send_to_char("a bow.\n\r",ch);		break;
		    case(WEAPON_STAFF)	: send_to_char("a staff.\n\r",ch);		break;
                default			: send_to_char("unknown.\n\r",ch);		break;
            }
            if (obj->pIndexData->new_format)
                sprintf(buf,"This weapon has an average damage of %d.\n\r",
                    (1 + obj->value[2]) * obj->value[1] / 2);
            else
                sprintf( buf, "This weapon has an average damage of %d.\n\r",
                    ( obj->value[1] + obj->value[2] ) / 2 );
            send_to_char( buf, ch );
            if (obj->value[4])  /* weapon flags */
            {
                sprintf(buf,"This weapon is special in being: %s\n\r",weapon_bit_name(obj->value[4]));
                send_to_char(buf,ch);
            }
            break;
    
        case ITEM_ARMOR:
            sprintf( buf, 
            "The <AC> or Armor Class is %d pierce, %d bash, %d slash, and %d vs. magic.\n\r", 
                obj->value[0], obj->value[1], obj->value[2], obj->value[3] );
            send_to_char( buf, ch );
            break;
        }
    
        if (!obj->enchanted)
        for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
        {
            if ( paf->location != APPLY_NONE && paf->modifier != 0 )
            {
                sprintf( buf, "Affects %s by %d.\n\r",
                    affect_loc_name( paf->location ), paf->modifier );
                send_to_char(buf,ch);
                if (paf->bitvector)
                {
                    switch(paf->where)
                    {
                        case TO_AFFECTS:
                            sprintf(buf,"Adds %s affect.\n",
                                affect_bit_name(paf->bitvector));
                            break;
                        case TO_OBJECT:
                            sprintf(buf,"Adds %s object flag.\n",
                                extra_bit_name(paf->bitvector));
                            break;
                        case TO_IMMUNE:
                            sprintf(buf,"Adds immunity to %s.\n",
                                imm_bit_name(paf->bitvector));
                            break;
                        case TO_RESIST:
                            sprintf(buf,"Adds resistance to %s.\n\r",
                                imm_bit_name(paf->bitvector));
                            break;
                        case TO_VULN:
                            sprintf(buf,"Adds vulnerability to %s.\n\r",
                                imm_bit_name(paf->bitvector));
                            break;
                        default:
                            sprintf(buf,"Unknown bit %d: %d\n\r",
                                paf->where,paf->bitvector);
                            break;
                    }
                    send_to_char( buf, ch );
                }
            }
        }
    
        for ( paf = obj->affected; paf != NULL; paf = paf->next )
        {
            if ( paf->location != APPLY_NONE && paf->modifier != 0 )
            {
                sprintf( buf, "Affects %s by %d",
                    affect_loc_name( paf->location ), paf->modifier );
                send_to_char( buf, ch );
                if ( paf->duration > -1)
                    sprintf(buf,", %d hours.\n\r",paf->duration);
                else
                    sprintf(buf,".\n\r");
                send_to_char(buf,ch);
                if (paf->bitvector)
                {
                    switch(paf->where)
                    {
                        case TO_AFFECTS:
                            sprintf(buf,"Adds %s affect.\n",
                                affect_bit_name(paf->bitvector));
                            break;
                        case TO_OBJECT:
                            sprintf(buf,"Adds %s object flag.\n",
                                extra_bit_name(paf->bitvector));
                            break;
                        case TO_WEAPON:
                            sprintf(buf,"Adds %s weapon flags.\n",
                                weapon_bit_name(paf->bitvector));
                            break;
                        case TO_IMMUNE:
                            sprintf(buf,"Adds immunity to %s.\n",
                                imm_bit_name(paf->bitvector));
                            break;
                        case TO_RESIST:
                            sprintf(buf,"Adds resistance to %s.\n\r",
                                imm_bit_name(paf->bitvector));
                            break;
                        case TO_VULN:
                            sprintf(buf,"Adds vulnerability to %s.\n\r",
                                imm_bit_name(paf->bitvector));
                            break;
                        default:
                            sprintf(buf,"Unknown bit %d: %d\n\r",
                                paf->where,paf->bitvector);
                            break;
                    }
                    send_to_char(buf,ch);
                }
            }
        }
    }
    return;
}



void spell_infravision( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_INFRARED) )
    {
	if (victim == ch)
	  send_to_char("You can already see in the dark.\n\r",ch);
	else
	  act("$N already has infravision.\n\r",ch,NULL,victim,TO_CHAR);
	return;
    }
    act( "$n's eyes glow red.\n\r", ch, NULL, NULL, TO_ROOM );

    af.where	 	= TO_AFFECTS;
    af.type      		= sn;
    af.level	 	= level;
    af.duration  		= 2 * level;
    af.location  		= APPLY_NONE;
    af.modifier  		= 0;
    af.bitvector 		= AFF_INFRARED;
    affect_to_char( victim, &af );
    send_to_char( "Your eyes glow red.\n\r", victim );
    return;
}



void spell_invis( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    AFFECT_DATA af;

    /* object invisibility */
    if (target == TARGET_OBJ)
    {
	obj = (OBJ_DATA *) vo;	

	if (IS_OBJ_STAT(obj,ITEM_INVIS))
	{
	    act("$p is already invisible.",ch,obj,NULL,TO_CHAR);
	    return;
	}
	
	af.where	= TO_OBJECT;
	af.type		= sn;
	af.level	= level;
	af.duration	= level + 12;
	af.location	= APPLY_NONE;
	af.modifier	= 0;
	af.bitvector	= ITEM_INVIS;
	affect_to_obj(obj,&af);

	act("$p fades out of sight.",ch,obj,NULL,TO_ALL);
	return;
    }

    /* character invisibility */
    victim = (CHAR_DATA *) vo;

    if ( IS_AFFECTED(victim, AFF_INVISIBLE) )
	return;

    act( "$n fades out of existence.", victim, NULL, NULL, TO_ROOM );

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level + 12;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_INVISIBLE;
    affect_to_char( victim, &af );
    send_to_char( "You fade out of existence.\n\r", victim );
    return;
}



void spell_know_alignment(int sn,int level,CHAR_DATA *ch,void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    char *msg;
    int ap;

    if ( IS_AFFECTED2( victim, AFF_UNDETECTABLE_ALIGNMENT ))
    {
	  act("Their alignement appears to be shielded by something.",ch,NULL,victim,TO_CHAR);
                return;
    }

    ap = victim->alignment;
	
         if ( ap >  700 ) msg = "$N has a pure and good aura.";
    else if ( ap >  350 ) msg = "$N is of excellent moral character.";
    else if ( ap >  100 ) msg = "$N is often kind and thoughtful.";
    else if ( ap > -100 ) msg = "$N doesn't have a firm moral commitment.";
    else if ( ap > -350 ) msg = "$N lies to $S friends.";
    else if ( ap > -700 ) msg = "$N is a black-hearted murderer.";
    else msg = "$N is the embodiment of pure evil!.";
	{
    act( msg, ch, NULL, victim, TO_CHAR );
    return;
	}
}



void spell_storm_of_vengeance(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const sh_int dam_each[] = 
    {
	 0,
	 0,  0,  0,  0,  0,	 0,  0,  0, 25, 28,
	31, 34, 37, 40, 40,	41, 42, 42, 43, 44,
	44, 45, 46, 46, 47,	48, 48, 49, 50, 50,
	51, 52, 52, 53, 54,	54, 55, 56, 56, 57,
	58, 58, 59, 60, 60,	61, 62, 62, 63, 64
    };
    int dam;

    level	= UMIN(level, sizeof(dam_each)/sizeof(dam_each[0]) - 1);
    level	= UMAX(0, level);
    dam		= number_range( dam_each[level] / 2, dam_each[level] * 2 );
    if ( saves_spell( level, victim,DAM_LIGHTNING) )
	dam /= 2;
    damage( ch, victim, dam, sn, DAM_LIGHTNING ,TRUE);
    return;
}



void spell_locate_object( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    char buf[MAX_INPUT_LENGTH];
    BUFFER *buffer;
    OBJ_DATA *obj;
    OBJ_DATA *in_obj;
    bool found;
    int number = 0, max_found;

    found = FALSE;
    number = 0;
    max_found = IS_IMMORTAL(ch) ? 200 : 2 * level;

    buffer = new_buf();
 
    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
	if ( !can_see_obj( ch, obj )
	|| !is_name( target_name, obj->name ) 
    	|| IS_OBJ_STAT(obj,ITEM_NOLOCATE)
	|| number_percent() > 2 * level
	|| ch->level < obj->level)
	    continue;

	found = TRUE;
        number++;

	for ( in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj )
	    ;

	if ( in_obj->carried_by != NULL && can_see(ch,in_obj->carried_by))
	{
	    sprintf( buf, "one is carried by %s\n\r",
		PERS(in_obj->carried_by, ch) );
	}
	else
	{
	    if (IS_IMMORTAL(ch) && in_obj->in_room != NULL)
		sprintf( buf, "one is in %s [Room %d]\n\r",
		    in_obj->in_room->name, in_obj->in_room->vnum);
	    else 
	    	sprintf( buf, "one is in %s\n\r",
		    in_obj->in_room == NULL
		    	? "somewhere" : in_obj->in_room->name );
	}

	buf[0] = UPPER(buf[0]);
	add_buf(buffer,buf);

    	if (number >= max_found)
	    break;
    }

    if ( !found )
	send_to_char( "Nothing like that in heaven or earth.\n\r", ch );
    else
	page_to_char(buf_string(buffer),ch);

    free_buf(buffer);

    return;
}



void spell_magic_missile( int sn, int level, CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const sh_int dam_each[] = 
    {
	 0,
	 3,  3,  4,  4,  5,	 6,  6,  6,  6,  6,
	 7,  7,  7,  7,  7,	 8,  8,  8,  8,  8,
	 9,  9,  9,  9,  9,	10, 10, 10, 10, 10,
	11, 11, 11, 11, 11,	12, 12, 12, 12, 12,
	13, 13, 13, 13, 13,	14, 14, 14, 14, 14
    };
    int dam;

    level	= UMIN(level, sizeof(dam_each)/sizeof(dam_each[0]) - 1);
    level	= UMAX(0, level);
    dam		= number_range( dam_each[level] / 2, dam_each[level] * 2 );
    if ( saves_spell( level, victim,DAM_ENERGY) )
	dam /= 2;
    damage( ch, victim, dam, sn, DAM_ENERGY ,TRUE);
    return;
}

void spell_mass_healing(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *gch;
    int heal_num, refresh_num;
    
    heal_num = skill_lookup("heal");
    refresh_num = skill_lookup("refresh"); 

    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	if ((IS_NPC(ch) && IS_NPC(gch)) ||
	    (!IS_NPC(ch) && !IS_NPC(gch)))
	{
	    spell_heal(heal_num,level,ch,(void *) gch,TARGET_CHAR);
	    spell_refresh(refresh_num,level,ch,(void *) gch,TARGET_CHAR);  
	}
    }
}
	    

void spell_mass_invis( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    AFFECT_DATA af;
    CHAR_DATA *gch;

    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	if ( !is_same_group( gch, ch ) || IS_AFFECTED(gch, AFF_INVISIBLE) )
	    continue;
	act( "$n slowly fades out of existence.", gch, NULL, NULL, TO_ROOM );
	send_to_char( "You slowly fade out of existence.\n\r", gch );

	af.where     = TO_AFFECTS;
	af.type      = sn;
    	af.level     = level/2;
	af.duration  = 24;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = AFF_INVISIBLE;
	affect_to_char( gch, &af );
    }
    send_to_char( "Ok.\n\r", ch );

    return;
}



void spell_null( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    send_to_char( "That's not a spell!\n\r", ch );
    return;
}



void spell_pass_door( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_PASS_DOOR) )
    {
	if (victim == ch)
	  send_to_char("You are already out of phase.\n\r",ch);
	else
	  act("$N is already shifted out of phase.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = number_fuzzy( level / 4 );
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_PASS_DOOR;
    affect_to_char( victim, &af );
    act( "$n turns translucent.", victim, NULL, NULL, TO_ROOM );
    send_to_char( "You turn translucent.\n\r", victim );
    return;
}

void spell_aid( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
	if ( victim == ch )
	    send_to_char("You are already aided.\n\r", ch);
	else
	    act("$N is already aided.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where	 = TO_AFFECTS;
    af.type	 	 = sn;
    af.level	 = level;
    af.duration	 = number_fuzzy( level / 4 );
    af.location	 = APPLY_HITROLL;
    if (ch->level < 20)
    {
    af.modifier	 = 1;
    }
    else
    if (ch->level < 40)
    {
    af.modifier	 = 2;
    }
    else
    if (ch->level < 60)
    {
    af.modifier	 = 4;
    }
    else
    if (ch->level < 80)
    {
    af.modifier	 = 6;
    }
    else
    {
    af.modifier	 = 10;
    }
    af.bitvector = 0;
    affect_to_char( victim, &af );
    send_to_char("You feel more prepared for battle!\n\r", victim);
    if ( ch != victim )
	act("$N looks more prepared for battle.",ch,NULL,victim,TO_CHAR);
    return;
}


/* RT plague spell, very nasty */

void spell_plague( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (IS_AFFECTED(ch,AFF_BLIND))
  {
    send_to_char("Cast the spell on whom? Your blind as a bat.\n\r",ch);
    return;
  }

  if (is_affected(victim,skill_lookup("endure disease"))
  && (number_percent() <= 65))
  {
    send_to_char("Your spell fizzles out, producing only a small cloud of green smoke.\n\r",ch);
    act("$n's feeble attempt to plague you has been thwarted by your endurance spell.",ch,NULL,victim,TO_VICT);
    return;
  }

  if (saves_spell(level,victim,DAM_DISEASE)
  || (IS_NPC(victim)
  && IS_SET(victim->act,ACT_UNDEAD)))
  {
    if (ch == victim)
	send_to_char("You feel momentarily ill, but it passes.\n\r",ch);
    else
	act("$N seems to be unaffected.",ch,NULL,victim,TO_CHAR);
    return;
  }

  af.where     		= TO_AFFECTS;
  af.type 	  		= sn;
  af.level	  		= level * 3/4;
  af.duration  		= level;
  af.location  		= APPLY_STR;
  af.modifier  		= -5; 
  af.bitvector 		= AFF_PLAGUE;
  affect_join(victim,&af);
   
  send_to_char("You scream in agony as plague sores erupt from your skin.\n\r",victim);
  act("$n screams in agony as plague sores erupt from $s skin.",victim,NULL,NULL,TO_ROOM);
}

void spell_poison( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (is_affected(victim, sn))
    return;

  if (is_affected(victim,skill_lookup("endure disease"))
  && (number_percent() <= 65))
  {
    send_to_char("Your spell fizzles out, producing only a small cloud of green smoke.\n\r",ch);
    act("$n's feeble attempt to poison you has been thwarted by your endurance spell.",ch,NULL,victim,TO_VICT);
    return;
  }

  if ( saves_spell( level, victim, DAM_POISON ) )
  {
    act("$n turns slightly green, but it passes.",victim,NULL,NULL,TO_ROOM);
    send_to_char("You feel momentarily ill, but it passes.\n\r",victim);
    return;
  }

  af.where     		= TO_AFFECTS;
  af.type      		= sn;
  af.level     		= level;
  af.duration  		= level;
  af.location  		= APPLY_STR;
  af.modifier  		= -2;
  af.bitvector 		= AFF_POISON;
  affect_join( victim, &af );
  send_to_char( "You feel very sick.\n\r", victim );
  act("$n looks very ill.",victim,NULL,NULL,TO_ROOM);
  return;
}

void spell_protection_evil(int sn,int level,CHAR_DATA *ch,void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
 
    if ( IS_AFFECTED(victim, AFF_PROTECT_EVIL) 
    ||   IS_AFFECTED(victim, AFF_PROTECT_GOOD))
    {
        if (victim == ch)
          send_to_char("You are already protected.\n\r",ch);
        else
          act("$N is already protected.",ch,NULL,victim,TO_CHAR);
        return;
    }

    if (IS_EVIL(ch))
    {
    send_to_char("What would be the point in protecting you from your own evil ways?\n\r",ch);
    return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 24;
    af.location  = APPLY_SAVING_SPELL;
    af.modifier  = -1;
    af.bitvector = AFF_PROTECT_EVIL;
    affect_to_char( victim, &af );
    send_to_char( "You feel holy and pure.\n\r", victim );
    if ( ch != victim )
        act("$N is protected from evil.",ch,NULL,victim,TO_CHAR);
    return;
}
 
void spell_protection_good(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
 
    if ( IS_AFFECTED(victim, AFF_PROTECT_GOOD) 
    ||   IS_AFFECTED(victim, AFF_PROTECT_EVIL))
    {
        if (victim == ch)
          send_to_char("You are already protected.\n\r",ch);
        else
          act("$N is already protected.",ch,NULL,victim,TO_CHAR);
        return;
    }

    if (IS_GOOD(ch))
    {
    send_to_char("Why would you want to protect yourself from others who walk with the light?\n\r",ch);
    return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 24;
    af.location  = APPLY_SAVING_SPELL;
    af.modifier  = -1;
    af.bitvector = AFF_PROTECT_GOOD;
    affect_to_char( victim, &af );
    send_to_char( "You feel aligned with darkness.\n\r", victim );
    if ( ch != victim )
        act("$N is protected from good.",ch,NULL,victim,TO_CHAR);
    return;
}


void spell_ray_of_truth (int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam, align;
 
    if (IS_EVIL(ch) )
    {
        victim = ch;
        send_to_char("The energy explodes inside you!\n\r",ch);
    }
 
    if (victim != ch)
    {
        act("$n raises $s hand, and a blinding ray of light shoots forth!",
            ch,NULL,NULL,TO_ROOM);
        send_to_char(
	   "You raise your hand and a blinding ray of light shoots forth!\n\r",
	   ch);
    }

    if (IS_GOOD(victim))
    {
	act("$n seems unharmed by the light.",victim,NULL,victim,TO_ROOM);
	send_to_char("The light seems powerless to affect you.\n\r",victim);
	return;
    }

    dam = dice( level, 10 );
    if ( saves_spell( level, victim,DAM_HOLY) )
        dam /= 2;

    align = victim->alignment;
    align -= 350;

    if (align < -1000)
	align = -1000 + (align + 1000) / 3;

    dam = (dam * align * align) / 1000000;

    damage( ch, victim, dam, sn, DAM_HOLY ,TRUE);
    spell_blindness(gsn_blindness, 
	3 * level / 4, ch, (void *) victim,TARGET_CHAR);
}


void spell_recharge( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    int chance, percent;

    if (obj->item_type != ITEM_WAND && obj->item_type != ITEM_STAFF)
    {
	send_to_char("That item does not carry charges.\n\r",ch);
	return;
    }

    if (obj->value[0] >= 3 * level / 2)
    {
	send_to_char("Your skills are not great enough for that.\n\r",ch);
	return;
    }

    if (obj->value[1] == 0)
    {
	send_to_char("That item has already been recharged once.\n\r",ch);
	return;
    }

    chance = 40 + 2 * level;

    chance -= obj->value[0]; /* harder to do high-level spells */
    chance -= (obj->value[1] - obj->value[2]) *
	      (obj->value[1] - obj->value[2]);

    chance = UMAX(level/2,chance);

    percent = number_percent();

    if (percent < chance / 2)
    {
	act("$p glows softly.",ch,obj,NULL,TO_ROOM);
	act("$p glows softly.",ch,obj,NULL,TO_CHAR);
	obj->value[2] = UMAX(obj->value[1],obj->value[2]);
	obj->value[1] = 0;
	return;
    }

    else if (percent <= chance)
    {
	int chargeback,chargemax;

	act("$p glows softly.",ch,obj,NULL,TO_ROOM);
	act("$p glows softly.",ch,obj,NULL,TO_CHAR);

	chargemax = obj->value[1] - obj->value[2];
	
	if (chargemax > 0)
	    chargeback = UMAX(1,chargemax * percent / 100);
	else
	    chargeback = 0;

	obj->value[2] += chargeback;
	obj->value[1] = 0;
	return;
    }	

    else if (percent <= UMIN(95, 3 * chance / 2))
    {
	send_to_char("Nothing seems to happen.\n\r",ch);
	if (obj->value[1] > 1)
	    obj->value[1]--;
	return;
    }

    else /* whoops! */
    {
	act("$p glows brightly and explodes!",ch,obj,NULL,TO_CHAR);
	act("$p glows brightly and explodes!",ch,obj,NULL,TO_ROOM);
	extract_obj(obj);
    }
}

void spell_refresh( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    victim->move = UMIN( victim->move + level, victim->max_move );
    if (victim->max_move == victim->move)
        send_to_char("You feel fully refreshed!\n\r",victim);
    else
        send_to_char( "You feel less tired.\n\r", victim );
    if ( ch != victim )
        send_to_char( "Ok.\n\r", ch );
    return;
}

void spell_remove_curse( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    bool found = FALSE;

    /* do object cases first */
    if (target == TARGET_OBJ)
    {
	obj = (OBJ_DATA *) vo;

	if (IS_OBJ_STAT(obj,ITEM_NODROP) || IS_OBJ_STAT(obj,ITEM_NOREMOVE))
	{
	    if (!IS_OBJ_STAT(obj,ITEM_NOUNCURSE)
	    &&  !saves_dispel(level + 2,obj->level,0))
	    {
		REMOVE_BIT(obj->extra_flags,ITEM_NODROP);
		REMOVE_BIT(obj->extra_flags,ITEM_NOREMOVE);
		act("$p glows blue.",ch,obj,NULL,TO_ALL);
		return;
	    }

	    act("The curse on $p is beyond your power.",ch,obj,NULL,TO_CHAR);
	    return;
	}
	act("There doesn't seem to be a curse on $p.",ch,obj,NULL,TO_CHAR);
	return;
    }

    /* characters */
    victim = (CHAR_DATA *) vo;

    if (check_dispel(level,victim,gsn_curse))
    {
	send_to_char("You feel better.\n\r",victim);
	act("$n looks more relaxed.",victim,NULL,NULL,TO_ROOM);
    }

   if ( check_dispel(level,victim,gsn_sleepless_curse) )
    {
	send_to_char("Your curse has been lifted, you can sleep again.\n\r",victim);
	act("$n looks more relieved now that $e can sleep again.",victim,NULL,NULL,TO_ROOM);
    }

   for (obj = victim->carrying; (obj != NULL && !found); obj = obj->next_content)
   {
        if ((IS_OBJ_STAT(obj,ITEM_NODROP) || IS_OBJ_STAT(obj,ITEM_NOREMOVE))
	&&  !IS_OBJ_STAT(obj,ITEM_NOUNCURSE))
        {   /* attempt to remove curse */
            if (!saves_dispel(level,obj->level,0))
            {
                found = TRUE;
                REMOVE_BIT(obj->extra_flags,ITEM_NODROP);
                REMOVE_BIT(obj->extra_flags,ITEM_NOREMOVE);
                act("Your $p glows blue.",victim,obj,NULL,TO_CHAR);
                act("$n's $p glows blue.",victim,obj,NULL,TO_ROOM);
            }
         }
    }
}

void spell_sanctuary( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected(victim, skill_lookup("phantom armor")))
    {
	if (victim == ch)
	  send_to_char("This will not work on you when already protected by Phantom Armor.\n\r",ch);
	else
	  act("$N is protected by an Illusory armor, your spell failed.",ch,NULL,victim,TO_CHAR);
	return;
    }

    if ( IS_AFFECTED(victim, AFF_SANCTUARY) )
    {
	if (victim == ch)
	  send_to_char("You are already in sanctuary.\n\r",ch);
	else
	  act("$N is already in sanctuary.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level / 6;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_SANCTUARY;
    affect_to_char( victim, &af );
    act( "$n is surrounded by a white aura.", victim, NULL, NULL, TO_ROOM );
    send_to_char( "You are surrounded by a white aura.\n\r", victim );
    return;
}



void spell_shield( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected(victim, skill_lookup("phantom armor")))
    {
	if (victim == ch)
	  send_to_char("This will not work on you when already protected by Phantom Armor.\n\r",ch);
	else
	  act("$N is protected by an Illusory armor, your spell failed.",ch,NULL,victim,TO_CHAR);
	return;
    }

    if ( is_affected( victim, sn ) )
    {
	if (victim == ch)
	  send_to_char("You are already shielded from harm.\n\r",ch);
	else
	  act("$N is already protected by a shield.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 8 + level;
    af.location  = APPLY_AC;
    af.modifier  = -20;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    act( "$n is surrounded by a force shield.", victim, NULL, NULL, TO_ROOM );
    send_to_char( "You are surrounded by a force shield.\n\r", victim );
    return;
}



void spell_shocking_grasp(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const int dam_each[] = 
    {
	 0,
	 0,  0,  0,  0,  0,	 0, 20, 25, 29, 33,
	36, 39, 39, 39, 40,	40, 41, 41, 42, 42,
	43, 43, 44, 44, 45,	45, 46, 46, 47, 47,
	48, 48, 49, 49, 50,	50, 51, 51, 52, 52,
	53, 53, 54, 54, 55,	55, 56, 56, 57, 57
    };
    int dam;

 if ( ch->fight_pos > FIGHT_FRONT )  
    {
      send_to_char("You must be within touch range in order to deliver a shocking grasp.\n\r", ch);
        act("$n fails to reach $s victim.", ch, NULL, NULL, TO_ROOM);
      return;
    }

    level	= UMIN(level, sizeof(dam_each)/sizeof(dam_each[0]) - 1);
    level	= UMAX(0, level);
    dam		= number_range( dam_each[level] / 2, dam_each[level] * 2 );
    if ( saves_spell( level, victim,DAM_LIGHTNING) )
	dam /= 2;
    damage( ch, victim, dam, sn, DAM_LIGHTNING ,TRUE);
    return;
}



void spell_sleep( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (ch == victim)
    {
	send_to_char("Why not just go to sleep instead?\n\r",ch);
	return;
    }

    if (IS_AFFECTED(ch,AFF_BLIND))
    {
	send_to_char("Cast the spell on whom? Your blind as a bat.\n\r",ch);
	return;
    }

    if ( ch->lastfought != NULL 
    && victim->lastfought != NULL
    && !IS_NPC(victim) 
    && !IS_IMMORTAL(ch))
    {
      if ( victim == ch->lastfought
	||   ch == victim->lastfought )
      {
        send_to_char("They are temporarily immune to your sleep spell!\n\r", ch );
        return;
      }
    }
  
    if( IS_AFFECTED2(victim,AFF_SLEEPCURSE) )
    {
	  act("$N has a sleepless curse upon $M, therefore, your spell will not work.",ch,NULL,victim,TO_CHAR);
	  return;
    }

    if ( IS_AFFECTED(victim, AFF_SLEEP)
    ||   (IS_NPC(victim) && IS_SET(victim->act,ACT_UNDEAD))
    ||   (level + 2) < victim->level
    ||   saves_spell( level-4, victim,DAM_CHARM) )
	return;

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = (level/4) + 10;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_SLEEP;
    affect_join( victim, &af );

    if ( IS_AWAKE(victim) )
    {
	send_to_char( "You feel very sleepy ..... zzzzzz.\n\r", victim );
	act( "$n goes to sleep.", victim, NULL, NULL, TO_ROOM );

	if( victim->riding ) //dismount
	{
		stop_fighting(victim->riding, TRUE);
		victim->riding->rider = NULL;
		victim->riding = NULL;
		victim->position = POS_STANDING;
	}

	victim->position = POS_SLEEPING;
    }

    if ( !IS_NPC(victim) 
    &&   !IS_IMMORTAL(ch))
    {
      ch->lastfought = victim;
      victim->lastfought = ch;
      if (!IS_NPC(victim))
      {
          free_string(ch->last_pc_fought);
          ch->last_pc_fought = str_dup(victim->name);
      }
    }

    check_cpose(victim);
    return;
}

void spell_slow( int sn, int level, CHAR_DATA *ch, void *vo,int target )
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
          send_to_char("You can't move any slower!\n\r",ch);
        else
          act("$N can't get any slower than that.",
              ch,NULL,victim,TO_CHAR);
        return;
    }
 
    if (saves_spell(level,victim,DAM_OTHER) 
    ||  IS_SET(victim->imm_flags,IMM_MAGIC))
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
            send_to_char("You feel momentarily slower.\n\r",victim);
            return;
        }

        act("$n is moving less quickly.",victim,NULL,NULL,TO_ROOM);
        return;
    }
 

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level/2;
    af.location  = APPLY_DEX;
    af.modifier  = -1 - (level >= 18) - (level >= 25) - (level >= 32);
    af.bitvector = AFF_SLOW;
    affect_to_char( victim, &af );
    send_to_char( "You feel yourself slowing d o w n...\n\r", victim );
    act("$n starts to move in slow motion.",victim,NULL,NULL,TO_ROOM);
    return;
}




void spell_flesh_to_stone( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    OBJ_DATA *stone;

    if ( is_affected( ch, sn ) )
    {
	if (victim == ch)
	  send_to_char("Your skin is already as hard as a rock.\n\r",ch); 
	else
	  act("$N is already as hard as can be.",ch,NULL,victim,TO_CHAR);
	return;
    }

    stone = get_eq_char(ch,WEAR_HOLD);
    if (!IS_IMMORTAL(ch) 
    &&  (stone == NULL || stone->item_type != ITEM_STONE))
    {
	send_to_char("You lack the proper component for this spell.\n\r",ch);
	return;
    }

    if (stone != NULL && stone->item_type == ITEM_STONE)
    {
     	act("You draw upon the power of $p.",ch,stone,NULL,TO_CHAR);
     	act("It flares brightly and vanishes!",ch,stone,NULL,TO_CHAR);
     	extract_obj(stone);
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.location  = APPLY_AC;
    af.modifier  = -40;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    act( "$n's flesh turns to stone.", victim, NULL, NULL, TO_ROOM );
    send_to_char( "Your flesh turns to stone.\n\r", victim );
    return;
}



void spell_summon( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim;

    if (IS_AFFECTED(ch,AFF_BLIND))
    {
	send_to_char("How do plan to summon someone when your blind as a bat.\n\r",ch);
	return;
    }

    if ( ( victim = get_char_world( ch, target_name ) ) == NULL
    ||   victim == ch
    ||   victim->in_room == NULL
    ||   IS_SET(ch->in_room->room_flags, ROOM_SAFE)
    ||   (ch->in_room->sector_type == SECT_UNDERWATER)
    ||   IS_SET(victim->in_room->room_flags, ROOM_SAFE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
    ||   IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
    ||   (IS_NPC(victim) && IS_SET(victim->act,ACT_AGGRESSIVE))
    ||   victim->level >= ch->level + 3
    ||   (!IS_NPC(victim) && victim->level >= LEVEL_IMMORTAL)
    ||   victim->fighting != NULL
    ||   victim->riding
    ||   victim->rider
    ||   (IS_NPC(victim) && IS_SET(victim->imm_flags,IMM_SUMMON))
    ||   (IS_NPC(victim) && victim->pIndexData->pShop != NULL)
    ||   (IS_NPC(victim) && saves_spell( level, victim,DAM_OTHER)) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if (((victim->quit_timer < 0 && !IS_IMMORTAL(ch))
    ||  (ch->quit_timer < 0 && !IS_IMMORTAL(ch)))
    && (number_percent() < 50 )) 
    {
      send_to_char("Your attempt to cast this spell failed!\n\r",ch);
      return;
    }

    check_cpose(victim);
    act( "$n disappears suddenly.", victim, NULL, NULL, TO_ROOM );
    char_from_room( victim );
    char_to_room( victim, ch->in_room );
    act( "$n arrives suddenly.", victim, NULL, NULL, TO_ROOM );
    act( "$n has summoned you to do $s bidding!", ch, NULL, victim,   TO_VICT );
    do_function(victim, &do_look, "auto" );
    return;
}



void spell_teleport( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    ROOM_INDEX_DATA *pRoomIndex;

    if ( victim->in_room == NULL
    ||   IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
    || ( victim != ch && IS_SET(victim->imm_flags,IMM_SUMMON))
    || ( !IS_NPC(ch) && victim->fighting != NULL )
    || ( victim != ch
    && ( saves_spell( level - 5, victim,DAM_OTHER))))
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if (((victim->quit_timer < 0 && !IS_IMMORTAL(ch))
    ||  (ch->quit_timer < 0 && !IS_IMMORTAL(ch)))
    && (number_percent() < 50 )) 
    {
      send_to_char("Your attempt to cast this spell failed!\n\r",ch);
      return;
    }

    if (ch->riding)
    {
      send_to_char("You cannot teleport while mounted.\n\r",ch);
      return;
    }

    pRoomIndex = get_random_room(victim);

    check_cpose(victim);
    if (victim != ch)
	send_to_char("You have been teleported!\n\r",victim);
    if (victim->riding && !IS_NPC(victim))
    {
	victim->riding->rider = NULL;
	victim->riding        = NULL;
    }
    act( "$n vanishes!", victim, NULL, NULL, TO_ROOM );
    char_from_room( victim );
    char_to_room( victim, pRoomIndex );
    act( "$n slowly fades into existence.", victim, NULL, NULL, TO_ROOM );
    do_function(victim, &do_look, "auto" );
    return;
}

void spell_ventriloquate( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  char speaker[MAX_INPUT_LENGTH];
  CHAR_DATA *vch;
  target_name = one_argument( target_name, speaker );
  speaker[0] = UPPER(speaker[0]);
    
  if( IS_SET(ch->comm, COMM_NOCHANNELS) )
  {
	send_to_char("Trying to get around your revoked privileges, are we??\n\r",ch);
	return;
  }

  for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room )
  {
    if( IS_SET(vch->comm, COMM_NOCHANNELS) ) continue;

    if (is_exact_name(speaker, vch->name) && vch != ch)
    {
	if (saves_spell(level+20, vch, DAM_MENTAL))
	{
        act_new( "Someone makes $t say '$T'",vch,speaker,target_name,TO_CHAR,POS_RESTING);
        continue;
	}
    }
    else
    if ((!is_exact_name( speaker, vch->name ) 
    && saves_spell( level+20, vch, DAM_MENTAL))
    && (vch != ch))
    {
      act_new( "Someone makes $t say '$T'",vch,speaker,target_name,TO_CHAR,POS_RESTING);
      continue;
    }
    else if (vch != ch) 
    {
      act_new( "$t says '$T'", vch, speaker, target_name, TO_CHAR, POS_RESTING);
	continue;
    }
    else 
    act_new( "You make $t say '$T'", vch,speaker,target_name,TO_CHAR,POS_RESTING);
  }
  return;
}

void spell_weaken( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_AFFECTED(ch,AFF_BLIND))
    {
	send_to_char("Cast the spell on whom? Your blind as a bat.\n\r",ch);
	return;
    }

    if ( is_affected( victim, sn ) || saves_spell( level, victim,DAM_OTHER) )
	return;

    if (ch == victim)
    {
	send_to_char("Making yourself weak would be funny, but it's not gonna happen.\n\r",ch);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level / 2;
    af.location  = APPLY_STR;
    af.modifier  = -1 * (level / 5);
    af.bitvector = AFF_WEAKEN;
    affect_to_char( victim, &af );
    send_to_char( "You feel your strength slip away.\n\r", victim );
    act("$n looks tired and weak.",victim,NULL,NULL,TO_ROOM);
    return;
}

void spell_flame_shield( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
	if (victim == ch)
	  send_to_char("You are already shielded from Cold attacks.\n\r",ch);
	else
	  act("$N is already protected by a flaming shield.",ch,NULL,victim,TO_CHAR);
	return;
    }


    af.where	= TO_RESIST;
    af.type      	= sn;
    af.level     	= level;
    af.duration  	= level / 8;
    af.location  	= APPLY_NONE;
    af.modifier  	= 0;
    af.bitvector 	= RES_COLD;
    affect_to_char( victim, &af );
    send_to_char( "You are surrounded by a flaming shield.\n\r", victim );
    act( "$n is surrounded by a flaming shield.", victim, NULL, NULL, TO_ROOM );
    return;
}

void spell_word_of_recall( int sn, int level, CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    ROOM_INDEX_DATA *location;
    int recl_room;
    
    if (IS_NPC(victim))
      return;

    if (IS_SET(victim->in_room->room_flags,ROOM_NO_RECALL)
    ||  IS_AFFECTED(victim,AFF_CURSE))
    {
	send_to_char("Spell failed.\n\r",victim);
	return;
    }

    if ( victim->quit_timer < 0
    && !IS_IMMORTAL(ch)
    && (number_percent() < 50 )) 
    {
	send_to_char("Your heart is beating so fast, you lose your concentration on the incantation.\n\r",ch);
	return;
    }

    if (victim->riding )
    {
	send_to_char("Spell Failed. You need to dismount first.\n\r",victim);
	return;
    }

    if ( IS_AFFECTED( victim, AFF_SHACKLES))
    {
	send_to_char("You are chained to the prison floor, and can not be sent home.\n\r",victim);
	return;
    }

    recl_room = victim->recall_point;

    if ( ( location = get_room_index(  recl_room ) ) == NULL )
    {
        send_to_char( "You are completely lost.\n\r", victim );
        return;
    }

    if ( victim->in_room == NULL )
        return;

    if (victim->fighting != NULL)
    {
	if ( number_percent() < 80 )
	{
          send_to_char( "You failed!!\n\r", victim );
	    return;
	}
      else
	stop_fighting(victim,TRUE);
    }
    
    check_cpose(victim);
    ch->move /= 2;
    act("$n disappears.",victim,NULL,NULL,TO_ROOM);
    char_from_room(victim);
    char_to_room(victim,location);
    act("$n appears in the room.",victim,NULL,NULL,TO_ROOM);
    do_function(victim, &do_look, "auto");

    if ((victim->pet != NULL) && !victim->pet->tethered)
    {
      char_from_room( victim->pet );
      char_to_room( victim->pet, location );
    }
}

/*
 * NPC spells.
 */
void spell_acid_breath( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam,hp_dam,dice_dam,hpch;

  act("$n spits acid at $N.",ch,NULL,victim,TO_NOTVICT);
  act("$n spits a stream of corrosive acid at you.",ch,NULL,victim,TO_VICT);
  act("You spit acid at $N.",ch,NULL,victim,TO_CHAR);

  hpch = UMAX(12,ch->hit);
  hp_dam = number_range(hpch/11 + 1, hpch/6);
  dice_dam = dice(level,16);

  dam = UMAX(hp_dam + dice_dam/10,dice_dam + hp_dam/10);
    
  if (saves_spell(level,victim,DAM_ACID))
  {
    acid_effect(victim,level/2,dam/4,TARGET_CHAR);
    damage(ch,victim,dam/2,sn,DAM_ACID,TRUE);
  }
  else
  {
    acid_effect(victim,level,dam,TARGET_CHAR);
    damage(ch,victim,dam,sn,DAM_ACID,TRUE);
  }
  return;
}



void spell_fire_breath( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  CHAR_DATA *vch, *vch_next;
  int dam,hp_dam,dice_dam;
  int hpch;

  act("$n breathes forth a cone of fire.",ch,NULL,victim,TO_NOTVICT);
  act("$n breathes a cone of hot fire over you!",ch,NULL,victim,TO_VICT);
  act("You breathe forth a cone of fire.",ch,NULL,NULL,TO_CHAR);

  hpch = UMAX( 10, ch->hit );
  hp_dam  = number_range( hpch/9+1, hpch/5 );
  dice_dam = dice(level,20);

  dam = UMAX(hp_dam + dice_dam /10, dice_dam + hp_dam / 10);
  fire_effect(victim->in_room,level,dam/2,TARGET_ROOM);

  for (vch = victim->in_room->people; vch != NULL; vch = vch_next)
  {
    vch_next = vch->next_in_room;

    if (is_safe_spell(ch,vch,TRUE) 
    ||  (IS_NPC(vch) && IS_NPC(ch) 
    &&   (ch->fighting != vch || vch->fighting != ch)))
	continue;

    if (!IS_NPC(vch) && is_same_group( vch, ch ) )
      continue;

    if (vch == victim) /* full damage */
    {
	if (saves_spell(level,vch,DAM_FIRE))
	{
	  fire_effect(vch,level/2,dam/4,TARGET_CHAR);
	  damage(ch,vch,dam/2,sn,DAM_FIRE,TRUE);
	}
	else
	{
	  fire_effect(vch,level,dam,TARGET_CHAR);
	  damage(ch,vch,dam,sn,DAM_FIRE,TRUE);
	}
    }
    else /* partial damage */
    {
	if (saves_spell(level - 2,vch,DAM_FIRE))
	{
	  fire_effect(vch,level/4,dam/8,TARGET_CHAR);
	  damage(ch,vch,dam/4,sn,DAM_FIRE,TRUE);
	}
	else
	{
	  fire_effect(vch,level/2,dam/4,TARGET_CHAR);
	  damage(ch,vch,dam/2,sn,DAM_FIRE,TRUE);
	}
    }
  }
  return;
}

void spell_frost_breath( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  CHAR_DATA *vch, *vch_next;
  int dam,hp_dam,dice_dam, hpch;

  act("$n breathes out a freezing cone of frost!",ch,NULL,victim,TO_NOTVICT);
  act("$n breathes a freezing cone of frost over you!",
	ch,NULL,victim,TO_VICT);
  act("You breathe out a cone of frost.",ch,NULL,NULL,TO_CHAR);

  hpch = UMAX(12,ch->hit);
  hp_dam = number_range(hpch/11 + 1, hpch/6);
  dice_dam = dice(level,16);

  dam = UMAX(hp_dam + dice_dam/10,dice_dam + hp_dam/10);
  cold_effect(victim->in_room,level,dam/2,TARGET_ROOM); 

  for (vch = victim->in_room->people; vch != NULL; vch = vch_next)
  {
    vch_next = vch->next_in_room;

    if (is_safe_spell(ch,vch,TRUE)
    ||  (IS_NPC(vch) && IS_NPC(ch) 
    &&   (ch->fighting != vch || vch->fighting != ch)))
	continue;

    if (!IS_NPC(vch) && is_same_group( vch, ch ) )
      continue;

    if (vch == victim) /* full damage */
    {
	if (saves_spell(level,vch,DAM_COLD))
	{
	  cold_effect(vch,level/2,dam/4,TARGET_CHAR);
	  damage(ch,vch,dam/2,sn,DAM_COLD,TRUE);
	}
	else
	{
	  cold_effect(vch,level,dam,TARGET_CHAR);
	  damage(ch,vch,dam,sn,DAM_COLD,TRUE);
	}
    }
    else
    {
	if (saves_spell(level - 2,vch,DAM_COLD))
	{
	  cold_effect(vch,level/4,dam/8,TARGET_CHAR);
	  damage(ch,vch,dam/4,sn,DAM_COLD,TRUE);
	}
	else
	{
	  cold_effect(vch,level/2,dam/4,TARGET_CHAR);
	  damage(ch,vch,dam/2,sn,DAM_COLD,TRUE);
	}
    }
  }
  return;
}

    
void spell_gas_breath( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  int dam,hp_dam,dice_dam,hpch;

  act("$n breathes out a cloud of poisonous gas!",ch,NULL,NULL,TO_ROOM);
  act("You breathe out a cloud of poisonous gas.",ch,NULL,NULL,TO_CHAR);

  hpch = UMAX(16,ch->hit);
  hp_dam = number_range(hpch/15+1,8);
  dice_dam = dice(level,12);

  dam = UMAX(hp_dam + dice_dam/10,dice_dam + hp_dam/10);
  poison_effect(ch->in_room,level,dam,TARGET_ROOM);

  for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
  {
    vch_next = vch->next_in_room;

    if (is_safe_spell(ch,vch,TRUE)
    ||  (IS_NPC(ch) && IS_NPC(vch) 
    &&   (ch->fighting == vch || vch->fighting == ch)))
	    continue;

    if (!IS_NPC(vch) && is_same_group( vch, ch ) )
      continue;

    if (is_affected(vch,skill_lookup("endure disease"))
    && (number_percent() <= 65))
    {
	poison_effect(vch,level/2,dam/4,TARGET_CHAR);
	damage(ch,vch,dam/2,sn,DAM_POISON,TRUE);
    }
    else
    if (saves_spell(level,vch,DAM_POISON))
    {
	poison_effect(vch,level/2,dam/4,TARGET_CHAR);
	damage(ch,vch,dam/2,sn,DAM_POISON,TRUE);
    }
    else
    {
	poison_effect(vch,level,dam,TARGET_CHAR);
	damage(ch,vch,dam,sn,DAM_POISON,TRUE);
    }
  }
  return;
}

void spell_lightning_breath(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam,hp_dam,dice_dam,hpch;

  act("$n breathes a bolt of lightning at $N.",ch,NULL,victim,TO_NOTVICT);
  act("$n breathes a bolt of lightning at you!",ch,NULL,victim,TO_VICT);
  act("You breathe a bolt of lightning at $N.",ch,NULL,victim,TO_CHAR);

  hpch = UMAX(10,ch->hit);
  hp_dam = number_range(hpch/9+1,hpch/5);
  dice_dam = dice(level,20);

  dam = UMAX(hp_dam + dice_dam/10,dice_dam + hp_dam/10);

  if (saves_spell(level,victim,DAM_LIGHTNING))
  {
    shock_effect(victim,level/2,dam/4,TARGET_CHAR);
    damage(ch,victim,dam/2,sn,DAM_LIGHTNING,TRUE);
  }
  else
  {
    shock_effect(victim,level,dam,TARGET_CHAR);
    damage(ch,victim,dam,sn,DAM_LIGHTNING,TRUE); 
  }
  return;
}

/*
 * Spells for mega1.are from Glop/Erkenbrand.
 */
void spell_general_purpose(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
 
    dam = number_range( 25, 100 );
    if ( saves_spell( level, victim, DAM_PIERCE) )
        dam /= 2;
    damage( ch, victim, dam, sn, DAM_PIERCE ,TRUE);
    return;
}

void spell_high_explosive(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
 
    dam = number_range( 30, 120 );
    if ( saves_spell( level, victim, DAM_PIERCE) )
        dam /= 2;
    damage( ch, victim, dam, sn, DAM_PIERCE ,TRUE);
    return;
}

void spell_convoke_swarm( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  int dam;

  if ( IS_SET(ch->in_room->room_flags, ROOM_NOMAGIC) )
  {
    send_to_char( "Your magic fizzles and dies.\n\r", ch);
    return;
  }

  send_to_char( "You summon a swarm of insects!\n\r", ch );
  act( "$n summons a swarm of insects!", ch, NULL, NULL, TO_ROOM );

  dam = dice(200 + 20, 15);
  plague_effect( ch->in_room, level, dam, TARGET_ROOM );

  for ( vch = char_list; vch != NULL; vch = vch_next )
  {
    vch_next	= vch->next;
    if ( vch->in_room == NULL )
	continue;
    if ( vch->in_room == ch->in_room )
    {
	if ( vch != ch && !is_safe_spell(ch,vch,TRUE))
	{
	  if ( ( ch->fighting == NULL )
	  && ( !IS_NPC( ch ) )
	  && ( !IS_NPC( vch ) ) )
	  {
	    vch->fighting = FALSE;
	  }

        if (is_same_group(vch,ch) )
        {
          continue;
        }

        if ( saves_spell( level, vch, DAM_ENERGY ) )
        {
          plague_effect( vch, level/2, dam/4, TARGET_CHAR );
          damage( ch, vch, dam, sn, DAM_ENERGY, TRUE );
        }
        else
        {
          plague_effect( vch, level, dam, TARGET_CHAR );
          damage( ch, vch, dam, sn, DAM_ENERGY, TRUE );
	  }
	}
	continue;
    }

    if ( vch->in_room->area == ch->in_room->area )
	send_to_char( "From somewhere nearby you hear a buzzing sound.\n\r", vch );
  }
  return;
}

void spell_life_transfer( int sn, int level, CHAR_DATA *ch, void *vo, int target)	
{
  CHAR_DATA *victim;

  if (( victim = get_char_room( ch, NULL, target_name ) ) == NULL)
  {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  if (ch->hit < (ch->max_hit / 1.5))
  {
    send_to_char( "You don't have enough life to give.\n\r", ch );
    return;
  }

  if (victim->hit > (victim->max_hit / 1.5))
  {
    send_to_char( "They aren't in need of your generous sacrifice right now.\n\r", ch );
    return;
  }

  ch->hit /= 2;
  victim->hit += ch->hit;
  act( "You give half your life to $N.\n\r", ch, NULL, victim, TO_CHAR );
  act( "$n gives you half of their life.\n\r",  ch, NULL, victim, TO_VICT);
  act( "$n gives half of their life to $N.\n\r", ch, NULL, victim, TO_NOTVICT);
  return;
}

void spell_mind_shatter( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_AFFECTED(ch,AFF_BLIND))
    {
	send_to_char("Cast the spell on whom? Your blind as a bat.\n\r",ch);
	return;
    }

    if ( is_affected( victim, sn ) )
    {
	if (victim == ch)
	  send_to_char("Your mind is already broken.\n\r",ch);
	else
	  act("$N is already reeling.",ch,NULL,victim,TO_CHAR);
	return;
    }

    if (IS_AFFECTED3(victim, AFF_CLEARMIND)
    && (number_percent() < 80 ))
    {
      send_to_char("Their mind is too focused, thoughts too clear, for your spell to have any effect!\n\r",ch);
      return;
    }

    af.where	 	= TO_VULN;
    af.type      		= sn;
    af.level	 	= level;
    af.duration 	 	= 1;
    af.location  		= APPLY_NONE;
    af.modifier  		= 0;     
    af.bitvector 		= VULN_MAGIC;
    affect_to_char( victim, &af );

    af.where     		= TO_VULN;
    af.type      		= sn;
    af.level     		= level;
    af.duration  		= 1;
    af.location  		= APPLY_NONE;
    af.modifier  		= 0;
    af.bitvector 		= VULN_BASH;
    affect_to_char( victim, &af );

    af.where     		= TO_AFFECTS;
    af.type      		= sn;
    af.level     		= level;
    af.duration  		= 1;
    af.location  		= APPLY_SAVING_SPELL;
    af.modifier 	 	= 25;
    af.bitvector 		= 0;
    affect_to_char( victim, &af );

    af.where     		= TO_AFFECTS;
    af.type      		= sn;
    af.level     		= level;
    af.duration  		= 1;
    af.location  		= APPLY_AC;
    af.modifier  		= 100;
    af.bitvector 		= 0;
    affect_to_char( victim, &af );

    send_to_char( "Your mind begins to tear apart!\n\r", victim );
    if ( ch != victim )
	act("$N's mind begins to tear apart!",ch,NULL,victim,TO_CHAR);
    return;
}

void spell_dragon_skin( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
	if (victim == ch)
	  send_to_char("Your skin is already as hard as dragon scales.\n\r",ch);
	else
	  act("$N's skin is already has hard as dragon scales.",ch,NULL,victim,TO_CHAR);
	return;
    }
    af.where	 = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = 15;
    af.modifier  = -40;
    af.location  = APPLY_AC;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    send_to_char( "Your skin turns as hard as dragon scales.\n\r", victim );
    act( "$n's skin turns hard as dragon scales.",victim,NULL,NULL,TO_ROOM );
    if ( ch != victim )
	act("$N is protected by your magic.",ch,NULL,victim,TO_CHAR);
    return;
}

void spell_silence( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( ch == victim )
    {
        send_to_char( "Silencing yourself would pretty stupid now wouldn't it?\n\r", ch );
        return;
    }

    if ( IS_AFFECTED2( victim, AFF_SILENCE ))
    {
	  act("$N has already been silenced.",ch,NULL,victim,TO_CHAR);
        return;
    }

    if (number_percent() > 15)
    {
      af.where	 = TO_AFFECTS2;
      af.type      = sn;
      af.level	 = level + 5;
      af.duration  = 10;
      af.modifier  = 0;
      af.location  = 0;
      af.bitvector = AFF_SILENCE;
      affect_to_char( victim, &af );
      act( "You have silenced $N!", ch, NULL, victim, TO_CHAR    );
      act( "Your tongue goes numb, as $n has silenced you!",  ch, NULL, victim, TO_VICT    );
      act( "$n has silenced $N!",   ch, NULL, victim, TO_NOTVICT );
      return;
    }
    else
    {
	send_to_char("You failed to silence your victim.\n\r",ch);
	return;
    }
}

void spell_remove_paralysis( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    bool found = FALSE;
 
    level += 2;

    if (IS_AFFECTED(ch,AFF_BLIND))
    {
	send_to_char("Cast the spell on whom? Your blind as a bat.\n\r",ch);
	return;
    }

    if (check_dispel(level,victim,skill_lookup("paralysis")))
    {
        found = TRUE;
        act("$n is no longer paralized.",victim,NULL,NULL,TO_ROOM);
    }

    if (check_dispel(level,victim,skill_lookup("hypnotic pattern")))
    {
        found = TRUE; 
        act("$n is no longer entranced by a hypnotic pattern.",victim,NULL,NULL,TO_ROOM);
        return;
    }

    if (found)
        send_to_char("Ok.\n\r",ch);
    else
        send_to_char("Spell failed.\n\r",ch);
}

void spell_doppelganger( int sn, int level, CHAR_DATA *ch, void *vo, int target )   
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( (ch == victim)
  || (is_affected(ch, sn) && (ch->doppel == victim)) )
  {
    act("But you already look like $M.",ch,NULL,victim,TO_CHAR);
    return;
  }

  if ( IS_NPC(victim))
  {
    act("You cannot doppel $N.",ch,NULL,victim,TO_CHAR);
    return;
  }

  if ( IS_IMMORTAL(victim))
  {
    send_to_char("Your puny self could never look that good!\n\r",ch);
    return;
  }

  if( is_affected( victim, gsn_treeform ) )
  {
    send_to_char("You couldn't possibly doppel a tree!\n\r",ch);
    return;
  }

  act("You change form to look like $N.",ch,NULL,victim,TO_CHAR);
  act("Someone who looks exactly like you is here.",ch,NULL,victim,TO_VICT);
  act("$n changes form to look like $N!",ch,NULL,victim,TO_NOTVICT);

  af.type               = sn;
  af.level              = level; 
  af.duration           = (2 * level)/3;
  af.location           = APPLY_NONE;
  af.modifier           = 0;
  af.bitvector          = 0;

  affect_to_char(ch,&af);  
  if( is_affected( victim, gsn_doppelganger ) )
    ch->doppel = victim->doppel;
  else
    ch->doppel = victim;

}

void spell_preserve_part( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    int result, fail;
    bool part;
    int vnum = obj->pIndexData->vnum;
	part = FALSE;
	switch(vnum)
	{
	case OBJ_VNUM_GUTS:
		 part = TRUE;
		 break;
	case OBJ_VNUM_SEVERED_HEAD:
		 part = TRUE;
		 break;
	case OBJ_VNUM_TORN_HEART:
		 part = TRUE;
		 break;
	case OBJ_VNUM_SLICED_ARM:
		 part = TRUE;
		 break;
	case OBJ_VNUM_SLICED_LEG:
		 part = TRUE;
		 break;
	case OBJ_VNUM_BRAINS:
		 part = TRUE;
		 break;
	default:
            part = FALSE;
	    break;
	}
    if (!part)
	{
	send_to_char("You cannot preserve that.\n\r",ch);
	return;
	}

    if (obj->wear_loc != -1)
    {
        send_to_char("The part must be carried to be preserved.\n\r",ch);
        return;
    }

    if (obj->timer <= -1)
    {
	send_to_char("This cannot be perserved more so than it is.\n\r", ch);
	return;
    }

    if (obj->timer == 0)
    {
	act("$p is too far gone to save it from decay.",ch,obj,NULL,TO_CHAR);
	return;
    }
	    
    fail = 10;  /* base 10% chance of failure */
    if (obj->value[3] == 1)
	fail += 40;  /*harder for poisoned limbs*/
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
        act("$p begins to show signs of decay, and turns a shade of green!",ch,obj,NULL,TO_CHAR);
        act("$p begins to show signs of decay, and turns a shade of green!",ch,obj,NULL,TO_ROOM);
        obj->timer /= 2;
	obj->value[3] = 1;
        return;
    }
    if ( result <= fail )  /* failed, no bad result */
    {
        send_to_char("Nothing seemed to happen.\n\r",ch);
        return;
    }
    obj->timer += number_fuzzy(level);
    act("$p absorbs the shadows around you, slowing its decay.",ch,obj,NULL,TO_CHAR);
    act("$p absorbs the shadows around you, slowing its decay.",ch,obj,NULL,TO_ROOM);
return;
}

void spell_resist_cold( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
	send_to_char("You are already resitant to the cold.\n\r",ch);
	return;
    }

    af.where	= TO_RESIST;
    af.type      	= sn;
    af.level     	= level;
    af.duration  	= level / 8;
    af.location  	= APPLY_NONE;
    af.modifier  	= 0;
    af.bitvector 	= RES_COLD;
    affect_to_char( victim, &af );
    send_to_char( "You become more resistant to cold based attacks.\n\r", victim );
    act( "$n looks more resistant to the cold.", victim, NULL, NULL, TO_ROOM );
    return;

}

void spell_coldlight(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    OBJ_DATA *light;

    light = create_object( get_obj_index( OBJ_VNUM_LIGHT_COLD ), 0 );
    obj_to_room( light, ch->in_room );
    act( "$n conjures forth a small dim light.", ch, NULL, NULL, TO_ROOM );
    act( "You twiddle your thumbs and $p appears.", ch, light, NULL, TO_CHAR );
    return;
}

void spell_renew_bones( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if (!IS_NPC(victim))
    {
        send_to_char( "You can't renew those with skin on their bones.\n\r", ch );
        return;
    }

    if (victim == ch)
    {
        send_to_char( "You can't renew your own health.\n\r", ch );
        return;
    }

    if (IS_NPC(victim) && (victim->pIndexData->vnum != MOB_VNUM_SKELETON ))
    {
        send_to_char( "You can only renew a skeleton's health.\n\r", ch );
        return;
    }

    victim->hit = UMIN( victim->hit + 100, victim->max_hit );
    update_pos( victim );
    send_to_char( "A warm feeling fills renews your bones.\n\r", victim );
    send_to_char( "Your skeletal fiend is renewed with strength and power.\n\r", ch );
    return;
}

void spell_intensify_death( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
	if (victim == ch)
	  send_to_char("Death has already intensified you as much as it will.\n\r",ch);
	else
	  act("$N is already protected from the living.",ch,NULL,victim,TO_CHAR);
	return;
    }
    af.where	 	= TO_AFFECTS;
    af.type      		= sn;
    af.level	 	= level;
    af.duration  		= 12;
    af.modifier  		= -40;
    af.location  		= APPLY_AC;
    af.bitvector 		= 0;
    affect_to_char( victim, &af );

    af.modifier  		= 1 + (level >= 18) + (level >= 25) + (level >= 32);
    af.location  		= APPLY_STR;
    af.bitvector 		= 0;
    affect_to_char( victim, &af );

    send_to_char( "Your body becomes less vulnerable to the living.\n\r", victim );
    if ( ch != victim )
	act("$N is protected by your magic.",ch,NULL,victim,TO_CHAR);
    act( "$n is surrounded by the shadow of death.", ch, NULL, NULL, TO_ROOM );
    return;
}

void spell_breath_of_the_dead( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  int dam,hp_dam,dice_dam,hpch;

  act("$n spews out a {ddeath cloud{x from $s mouth!",ch,NULL,NULL,TO_ROOM);
  act("You breathe out a cloud of {ddeath{x.",ch,NULL,NULL,TO_CHAR);

  hpch = UMAX(16,ch->hit);
  hp_dam = number_range(hpch/15+1,8);
  dice_dam = dice(level,12);

  dam = UMAX(hp_dam + dice_dam/13,dice_dam + hp_dam/13);
  poison_effect(ch->in_room,level,dam,TARGET_ROOM);

  for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
  {
    vch_next = vch->next_in_room;

    if (is_safe_spell(ch,vch,TRUE)
    ||  (IS_NPC(ch) && IS_NPC(vch) 
    &&   (ch->fighting == vch || vch->fighting == ch)))
	continue;

    if (!IS_NPC(vch) && is_same_group( vch, ch ) )
      continue;

    if (saves_spell(level,vch,DAM_POISON))
    {
	poison_effect(vch,level/2,dam/4,TARGET_CHAR);
	damage(ch,vch,dam/2,sn,DAM_POISON,TRUE);
    }
    else
    {
	poison_effect(vch,level,dam,TARGET_CHAR);
	damage(ch,vch,dam,sn,DAM_POISON,TRUE);
    }
  }
  return;
}

void spell_deadeye( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_DETECT_INVIS) )
    {
       send_to_char("You already see as the dead would see.\n\r",ch);
       return;
    }

    af.where     		= TO_AFFECTS;
    af.type      		= sn;
    af.level     		= level;
    af.duration  		= level;
    af.modifier  		= 0;
    af.location  		= APPLY_NONE;
    af.bitvector 		= AFF_DETECT_INVIS;
    affect_to_char( victim, &af );

    af.location  		= APPLY_NONE;
    af.modifier  		= 0;
    af.bitvector 		= AFF_INFRARED;
    affect_to_char( victim, &af );

    send_to_char( "Your eyes tingle, granting you the vision of the dead.\n\r", victim );
    return;
}

void spell_shallow_breath( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const sh_int dam_each[] = 
    {
	 0,
	 0,  0,  6,  7,  8,	 9, 12, 13, 13, 13,
	14, 14, 14, 15, 15,	15, 16, 16, 16, 17,
	17, 17, 18, 18, 18,	19, 19, 19, 20, 20,
	20, 21, 21, 21, 22,	22, 22, 23, 23, 23,
	24, 24, 24, 25, 25,	25, 26, 26, 26, 27
    };
    int dam;

    if ( ch->fight_pos > FIGHT_FRONT )  
    {
    send_to_char("You are not close enough to deliver the blow to your victim.\n\r", ch );
    act("$n fails to reach $s victim.", ch, NULL, NULL, TO_ROOM);
    return;
    }

    level	= UMIN(level, sizeof(dam_each)/sizeof(dam_each[0]) - 1);
    level	= UMAX(0, level);
    dam	= number_range( dam_each[level] / 2, dam_each[level] * 2 );
    if ( saves_spell( level, victim,DAM_BASH ) )
    {
	dam /= 2;
    }
    act("$n doubles over, the wind knocked out of $m.",victim,NULL,NULL,TO_ROOM);
    damage( ch, victim, dam, sn, DAM_BASH,TRUE );
    return;
}

void spell_chaos_flux(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
 
    dam = number_range( 130, 150 );
    if ( saves_spell( level, victim, DAM_ENERGY) )
        dam /= 2;
    damage( ch, victim, dam, sn, DAM_ENERGY ,TRUE);
    return;
}

void spell_gravity_flux( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;

    send_to_char( "The gravity in the room disappears!\n\r", ch );
    act( "$n makes the gravity give way.", ch, NULL, NULL, TO_ROOM );
    for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
    {
        vch_next        = vch->next_in_room;
        if (IS_IMMORTAL(vch) || vch == ch || is_same_group(vch,ch))
          continue;     
        if (!is_safe_spell(ch,vch,TRUE) )
        {
          if (!IS_NPC(ch) 
          && vch != ch 
          && ch->fighting != vch
          && vch->fighting != ch
          && (IS_AFFECTED(vch,AFF_CHARM)
          || !IS_NPC(vch)))
          {
                spell_yell(ch, vch, NULL);
          }

          if ( vch->in_room == NULL )
            continue;
          {
            if (IS_AFFECTED(vch,AFF_FLYING)) 
              damage(ch,vch,0,sn,DAM_BASH,TRUE);
            else
              damage( ch, vch, (level + dice(2, 8))*4, sn, DAM_BASH,TRUE );
    send_to_char( "Gravity gives way, throwing you up in the air then sucking you back down!\n\r", vch );
          }
          if ( vch->in_room == NULL )
              continue;
       }
    }
    return;
}

void spell_torment(int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    if (victim != ch)
    {
	act("$n calls forth demons from the Abyss upon $N!",
	    ch,NULL,victim,TO_ROOM);
        act("$n has assailed you with the demons of Hell!",
	    ch,NULL,victim,TO_VICT);
	send_to_char("You conjure forth demons from the Abyss!\n\r",ch);
    }
    dam = dice( level, 10 );
    if ( saves_spell( level, victim,DAM_HARM) )
        dam /= 2;
    damage( ch, victim, dam * 2, sn, DAM_HARM ,TRUE);
}

void spell_thunder_clap( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  AFFECT_DATA af;

        
  act( "$n slams $s hands together and a thunderous roar pierces the air.", ch, NULL, NULL, TO_ROOM);
  send_to_char( "You clap your hands together and a thunderous roar pierces the air!\n\r", ch );

  for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
  {
    vch_next = vch->next_in_room;
    if (vch == ch || is_same_group(vch,ch) || IS_IMMORTAL(vch))
    	continue;

    if (!is_safe_spell(ch,vch,TRUE) ) 
    {
      if (!IS_NPC(ch)
	&& vch != ch
	&& ch->fighting != vch
	&& vch->fighting != ch 
	&& (IS_AFFECTED(vch,AFF_CHARM)
	|| !IS_NPC(vch)))
      {
        spell_yell(ch, vch, NULL);
      }
    }

    if (vch == ch || is_same_group(vch,ch))
      continue;
        	
    if (is_affected( vch, sn ) || saves_spell( level, vch, DAM_SOUND ) )
	continue;

    if (!IS_AFFECTED2(vch, AFF_DEAFNESS))
    {
      af.where		= TO_AFFECTS2;
      af.type      	= sn;
      af.level     	= level;
	af.duration  	= level / 3;
	af.location  	= 0;
      af.modifier  	= 0;
	af.bitvector 	= AFF_DEAFNESS;
	affect_to_char( vch, &af );
      send_to_char("You're ears explode with a clash, leaving your hearing with a constant ring.\n\r", vch);
	act("$n reels back and holds $s ears.",vch,NULL,NULL,TO_ROOM);
    }
  }
  return;        	
}

void spell_deafness( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    act( "$n covers $s ears and nods.", ch, victim, victim, TO_ROOM);
    act( "You cover your ears and nod at $M.", victim, NULL, ch, TO_CHAR);
    if (victim->fighting == NULL && !IS_NPC(victim) )
	spell_yell(ch, victim, NULL);

    if ( is_affected( victim, sn ) || saves_spell( level, victim, DAM_SOUND ) )
        return;

    af.where		= TO_AFFECTS2;
    af.type      		= sn;
    af.level     		= level;
    af.duration  		= level / 2;
    af.location  		= 0;
    af.modifier  		= 0;
    af.bitvector 		= AFF_DEAFNESS;
    affect_to_char( victim, &af );
    send_to_char( "A cold, impenetrable silence surrounds you.\n\r", victim );
    act("$n looks less alert as $s sense of hearing departs $m.",victim,NULL,NULL,TO_ROOM);
    return;
}

void spell_cure_deafness(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  bool found = FALSE;

  if (check_dispel(level,victim,skill_lookup("deafness")))
  {
    act("$n is able to hear again.",victim,NULL,NULL,TO_ROOM);
    found = TRUE;
    return;
  }

  if (check_dispel(level,victim,skill_lookup("thunderclap")))
  {
    act("$n seems to shake off the affects of being deaf.", victim,NULL,NULL,TO_ROOM);
    found = TRUE;
    return;
  }

  send_to_char("Spell failed.\n\r",ch);
  return;
}

void spell_waterlungs( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    if ( IS_AFFECTED(victim, AFF_WATERBREATHING) )
    {
        if (victim == ch)
          send_to_char("You are already able to breathe freely underwater.\n\r",ch);
        else
          act("$N can already breathe freely underwater.",ch,NULL,victim,TO_CHAR);
        return;
    }

    af.where	= TO_AFFECTS;
    af.type      	= sn;
    af.level     	= level;
    af.duration  	= level + 3;
    af.location  	= 0;
    af.modifier  	= 0;
    af.bitvector 	= AFF_WATERBREATHING;
    affect_to_char( victim, &af );
    send_to_char( "You feel a strange sensation in your chest.\n\r", victim );
    return;
}

void spell_alarm(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{

    OBJ_DATA *alarm;
    AFFECT_DATA af;

    if ( is_affected(ch, sn))
    {
      send_to_char("You may control only one such alarm at a time.\n\r",ch);
      return;
    }
    alarm = create_object( get_obj_index( OBJ_VNUM_SPELL_ALARM ), 0);
    obj_to_room(alarm, ch->in_room);
    alarm->timer = ch->level/5;

    af.where		= TO_AFFECTS;
    af.type			= sn;
    af.level		= level; 
    af.duration		= level/5;
    af.bitvector		= 0; 
    af.modifier		= 0;
    af.location		= APPLY_NONE;
    affect_to_char(ch, &af);
    act("A small translucent bell fades into being.\n\r",ch,NULL,NULL,TO_ROOM);
    send_to_char("A small translucent bell fades into being.\n\r",ch);
    return;
}

void spell_fire_seed( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = ( CHAR_DATA *) vo;
    OBJ_DATA *obj;
    OBJ_DATA *fireseed;
    OBJ_DATA *acorn = NULL;
    int dam;

    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
        if ( obj->pIndexData->vnum == OBJ_VNUM_HERB_ACORN)
        {
          acorn = obj; 
          break;
        }   
    }
  
    if ( acorn == NULL )
    {
      send_to_char( "You require a specific acorn on your person to complete this spell.\n\r", ch);
      return;
    }
    
    if ( ( victim = get_char_room( ch, NULL, target_name ) ) != NULL )
    {
      if (victim == ch || target_name[0] == '\0')
      {
       if ( ch->fighting != NULL )
       {
        victim = ch->fighting;
        dam = dice( level, 5);
        if ( saves_spell( level, victim, DAM_FIRE) )
         dam /= 2;
        act( "Fire bursts about you as $n throws a flaming object at you!\n\r",
         ch, victim, victim, TO_VICT); 
        act( "Fire bursts about $N as a small flaming object hits $M!\n\r",
         ch, NULL, victim, TO_NOTVICT);
        act( "Fire bursts about $N as you hit $M with your fire seed!\n\r",
         ch, NULL, victim, TO_CHAR);
        damage( ch, victim, dam, sn, DAM_FIRE, 0 );
        extract_obj(acorn);
        return;
       }
       else
       {          
        fireseed = create_object( get_obj_index( OBJ_VNUM_SPELL_FIRESEED ), 0);
        obj_to_room( fireseed, ch->in_room );
        act("$n holds out a small acorn, which ignites as $e drops it to the ground.\n\r", ch, NULL, NULL, TO_ROOM);
         act("You recall the words of power and the acorn ignites.\n\rYou drop it to the ground.\n\r", ch, NULL, NULL, TO_CHAR);
        extract_obj(acorn);
        return;
       }
     } 

    if(is_safe_spell(ch,victim,TRUE))
    {
      send_to_char("Your magic seems to falter with that target.\n\r",ch);
      return;
    }

    if(is_same_group(victim,ch))
    {
      send_to_char("That would make for an interesting traveling companion.\n\r",ch);
      return;
    }

    dam = dice( level, 5);
    if ( saves_spell( level, victim, DAM_FIRE) )
       dam /= 2;
    act( "Fire bursts about you as $n throws a flaming object at you!\n\r",
      ch, victim, victim, TO_VICT);
    act( "Fire bursts about $N as a small flaming object hits $M!\n\r",
      ch, NULL, victim, TO_NOTVICT);
    act( "Fire bursts about $N as you hit $M with your fire seed!\n\r",
      ch, NULL, victim, TO_CHAR);
    damage( ch, victim, dam, sn, DAM_FIRE, TRUE );
    extract_obj(acorn);
    return;
    }
    else
    if (target_name[0] != '\0')
    {
      send_to_char( "Your target isn't here!\n\r", ch);
      return;
    }
    else
    {          
      fireseed = create_object( get_obj_index( OBJ_VNUM_SPELL_FIRESEED ), 0);
      obj_to_room( fireseed, ch->in_room );
      act("$n holds out a small acorn, which ignites as $e drops it to the ground.\n\r", ch, NULL, NULL, TO_ROOM);
      act("You recall the words of power and the acorn ignites.\n\rYou drop it to the ground.\n\r", ch, NULL, NULL, TO_CHAR);
      extract_obj(acorn);
      return;
     }
}

void spell_gift( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    CHAR_DATA *victim;
    char arg[MSL];

    target_name = one_argument(target_name, arg);

    if ( target_name[0] == '\0' )
    {
        send_to_char("Who do you wish to send your gift to?\n\r", ch);
        return;
    }

    if ( (victim = get_char_world(ch, target_name)) == NULL)
    {
        send_to_char("You cannot reach that person to gift them at this time.\n\r", ch);
        return;
    }

    if (IS_NPC(victim))
    return;

    if ( (victim->level > ch->level + 10)
    ||   IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
    ||   IS_SET(victim->in_room->room_flags, ROOM_GODS_ONLY)
    ||   IS_SET(victim->in_room->room_flags, ROOM_IMP_ONLY))
    {
        send_to_char("You cannot gift that person with anything right now.\n\r", ch);
        return;
    }

    if ( victim->carry_number + get_obj_number( obj ) > can_carry_n( victim ) )
    {
	act( "$N has $S hands full and cannot receive any gifts at the moment.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if (ch == victim)
      send_to_char("It would be pointless to gift yourself.\n\r", ch);
    else
    {
        /* Get obj from char & give it to the victim */
        obj_from_char(obj);
        obj_to_char(obj, victim);
        send_to_char("Your gift has been delivered through your magic.\n\r", ch);
	send_to_char("Your pack feels heavier. A gift has been sent to you!\n\r", victim);
    }
 return;
}

void spell_evil_cloak( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    
    if ( IS_AFFECTED3(victim, AFF_GAURA) )
    {
          send_to_char("You can either percieve yourself as good or evil, not both!\n\r",ch);
          return;
    }
    
    if ( IS_AFFECTED3(victim, AFF_EAURA) )
    {
          send_to_char("You already use your illusions to cloak yourself with an Evil aura!\n\r",ch);
          return;
    }

    af.where	= TO_AFFECTS3;
    af.type      	= sn;
    af.level     	= level;
    af.duration  	= number_fuzzy( level / 2 );
    af.location  	= APPLY_SAVING_SPELL;
    af.modifier  	= -10;
    af.bitvector 	= AFF_EAURA;
    affect_to_char( victim, &af );
    send_to_char("You speak the arcane words of your spell.\n\r",ch);
    act( "$n is surrounded with a swirling black mist.", victim, NULL, NULL, TO_ROOM );
    send_to_char( "You feel the mist embrace you, covering your intentions with those most evil!\n\r", victim );
    return;
}

void spell_dazzle( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

   if (victim == ch)
    {
      act("You cannot dazzle yourself.", ch, NULL, victim, TO_CHAR);
      return;
    }
    
    if (IS_NPC(victim))
    {
      act("Your patterns cannot dazzle that one.", ch, NULL, victim, TO_CHAR);
      return;
    }

    if ( is_affected( victim, sn ) )
    {
      act("$N is already dazzled beyond the strength of more dazzling.",
        ch,NULL,victim,TO_CHAR);
      return;
    }

    if (saves_spell(level, victim, DAM_MENTAL))
    {
      act("You fail to dazzle $N with your patterns.", ch, NULL, victim, TO_CHAR);
      return;
    }

    af.where	= TO_AFFECTS3;
    af.type      	= sn;
    af.level     	= level / 2;
    af.duration  	= level / 8;
    af.location  	= APPLY_NONE;
    af.modifier  	= 0;
    af.bitvector 	= AFF_HOLDPERSON;
    affect_to_char( victim, &af );
    send_to_char( "You stare at the magical patterns before you, dazzled by them. \n\r", victim );
    act( "$N is motionless, dazzled by $n's magical patterns.",
      ch, NULL, victim, TO_NOTVICT);
    act("You have successfully dazzled $N with your magical patterns.",
      ch, NULL, victim, TO_CHAR);
    return;
}

void spell_immolation( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  AFFECT_DATA af;

  if (IS_AFFECTED3(ch,AFF_IMMOLATION))
  {
    send_to_char("You are already engulfed in flames of wizardry!\n\r",ch);
    return;
  }

  send_to_char( "You are engulfed within the flames of wizardry.\n\r", ch );
  act( "$n is engulfed in fire.", ch, NULL, NULL, TO_ROOM );

  af.where     	= TO_AFFECTS3;
  af.type      	= sn;
  af.level	= level;
  af.duration  	= level / 4;
  af.location  	= APPLY_HITROLL;
  af.modifier  	= level / 6;
  af.bitvector 	= AFF_IMMOLATION;
  affect_to_char( ch, &af );

  af.location  	= APPLY_DAMROLL;
  af.modifier  	= level / 6;
  affect_to_char( ch, &af );

  af.location  	= APPLY_AC;
  af.modifier  	= 0 - get_skill(ch, gsn_endure ) / 2;
  affect_to_char( ch, &af );

  send_to_char( "Tongues of flame from your body fly forth.\n\r",ch);
  act( "Fire from $n reaches out, threatening to consume you.", ch, NULL, NULL, TO_ROOM );
  return;
}

void spell_spook( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
  ROOM_INDEX_DATA *was_in;
  ROOM_INDEX_DATA *now_in;
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
  int attempt;

  if ( victim == NULL )
  {
    send_to_char( "Who would you like to spook?\n\r", ch );
    return;
  }

  if (victim == ch)
  {
    send_to_char( "You couldn't possibly spook yourself with your own illusions.\n\r", ch );
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
    act("$N is already writhing in fear, your spell has no effect.",ch,NULL,victim,TO_CHAR);
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
    send_to_char( "Your spell failed, perhaps these aren't the right conditions.\n\r", ch );
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

    act( "You advance on $N with a spooky illusion!", ch, NULL, victim, TO_CHAR    );
    act( "$n advances on you with a spooky illusion, unnerving fear grips your heart.",  ch, NULL, victim, TO_VICT    );

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

void spell_entangle( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (IS_NPC(victim))
  {
    act("Your vines cannot entangle that one.", ch, NULL, victim, TO_CHAR);
    return;
  }

  if (victim == ch)
  {
    act("You cannot entangle yourself.", ch, NULL, victim, TO_CHAR);
    return;
  }

  if ( is_affected( victim, sn ) )
  {
    act("$N is already entangled by your vines.", ch, NULL, victim, TO_CHAR);
    return;
  }

  if ( IS_AFFECTED2( victim, AFF_PARALYSIS ))
  {
    act("$N is paralyzed. Your spell failed.",ch,NULL,victim,TO_CHAR);
    return;
  }

  if (IS_AFFECTED3(victim, AFF_HOLDPERSON))
  {
    act("$N is already unable to move.", ch, NULL, victim, TO_CHAR);
    return;
  }

  switch (ch->in_room->sector_type)
  {
  case SECT_FOREST:
  case SECT_HILLS:
  case SECT_MOUNTAIN:
  case SECT_SWAMP:
  case SECT_FIELD:
  case SECT_UNDERWATER:
  case SECT_FORESTER:
    if ( saves_spell(level, victim, DAM_OTHER))
    {
      act("You fail to entangle $N with your vines.", ch, NULL, victim, TO_CHAR);
      return;
    }

    af.where	= TO_AFFECTS3;
    af.type      	= sn;
    af.level     	= level;
    af.duration  	= level / 6;
    af.location  	= APPLY_NONE;
    af.modifier  	= 0;
    af.bitvector 	= AFF_HOLDPERSON;
    affect_to_char( victim, &af );
    act( "Vines sprout from the ground and wrap tightly around you!", ch, NULL, victim,TO_VICT );
    act( "Vines sprout from the ground and wrap tightly around $N.", ch, NULL, victim,TO_NOTVICT );
    act("You call forth vines to entangle $N in their grasp.", ch, NULL, victim, TO_CHAR);
    return;

  default:
    send_to_char("You cannot call any vines in this place.\n\r",ch);
    return;
    break;
    }
    return;
}

void spell_plant_growth ( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{ 
  OBJ_DATA *plants;
  int sect;
 
  sect = ch->in_room->sector_type;
  plants = create_object(get_obj_index(OBJ_VNUM_PLANTS), 0);

  if (ch->fighting)
  {
    send_to_char( "Your too busy fighting!\n\r", ch);
    return;
  }

  if ((sect == SECT_FOREST)
  || (sect == SECT_SWAMP))
  {
    send_to_char( "This land is already quite lush and fertile. Your magic is not needed here.\n\r", ch);
    return;
  }

  if (((sect != SECT_CITY)
  & (sect != SECT_FIELD)
  & (sect != SECT_HILLS)
  & (sect != SECT_DESERT)
  & (sect != SECT_GRAVEYARD)
  & (sect != SECT_POLAR)
  & (sect != SECT_MOUNTAIN))
  ||(IS_SET(ch->in_room->room_flags, ROOM_INDOORS) ))
  {
    send_to_char("Not even your magic can coax plants to grow or flowers to bud in this place.\n\r", ch);
    return;
  }

  send_to_char( "You call upon the plants to spring forth from this barren place.\n\r", ch);
  obj_to_room( plants, ch->in_room );
  plants->timer = 50;
  act( "$n speaks an incantation which calls the plants to spring forth from this barren place.",ch, NULL, NULL, TO_ROOM );
  return;
}

void spell_enlarge( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected(victim, skill_lookup("enlarge"))
    || (victim->size == 5))
    {
        if (victim == ch)
          send_to_char("You are already as large as you can get! \n\r", ch);
        else
          act("$N is already as large as $E can get.",ch,NULL,victim,TO_CHAR);
        return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.location  = APPLY_SIZE;
    af.modifier  = 1;
    af.bitvector = 0;
    affect_to_char( victim, &af );

    send_to_char( "Suddenly, the world seems much smaller!\n\r", victim );
    act("$N grows several paces right before your eyes!", ch, NULL,victim,TO_NOTVICT);
    act("$N grows several paces right before your eyes!", ch, NULL,victim,TO_CHAR);

    return;
}

void spell_shrink( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected(victim, skill_lookup("shrink"))
    || (victim->size == 0))
    {
        if (victim == ch)
          send_to_char("You are already as small as you can get! \n\r", ch);
        else
          act("$N is already as small as $E can get.",ch,NULL,victim,TO_CHAR);
        return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.location  = APPLY_SIZE;
    af.modifier  = -1;
    af.bitvector = 0;
    affect_to_char( victim, &af );

    send_to_char( "Suddenly, the world seems much larger!\n\r", victim );
    act("$N shrinks several paces right before your eyes!",ch,NULL,victim,TO_NOTVICT);
    act("$N shrinks several paces right before your eyes!",ch,NULL,victim,TO_CHAR);

    return;
}

void spell_clear_thoughts( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
    
  if ( IS_AFFECTED3(victim, AFF_CLEARMIND) )
  {
    send_to_char("Your mind is as clear as it's going to get!\n\r",ch);
    return;
  }

  af.where		= TO_AFFECTS3;
  af.type      	= sn;
  af.level     	= level;
  af.duration  	= number_fuzzy( level / 3 );
  af.location  	= 0;
  af.modifier  	= 0;
  af.bitvector 	= AFF_CLEARMIND;
  affect_to_char( victim, &af );
  act( "$n whispers a few arcane words.", victim, NULL, NULL, TO_ROOM );
  send_to_char( "Your mind becomes more clear and focused, your thoughts almost impenetrable!\n\r", victim );
  return;
}

void spell_divine_favor( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED3(victim, AFF_DIVINE_FAVOR))
    {
	return;
    }

    af.where	 	= TO_AFFECTS3;
    af.type      		= sn;
    af.level	 	= victim->level;
    af.duration  		= 23;
    af.modifier  		= 0;
    af.location  		= 0;
    af.bitvector 		= AFF_DIVINE_FAVOR;
    affect_to_char( victim, &af );
    return;
}

void spell_mark_of_origin( int sn, int level, CHAR_DATA *ch,void *vo,int target)
{
    
    if (IS_NPC(ch))
      return;

    if ( IS_SET(ch->in_room->room_flags,ROOM_NO_RECALL)
    ||   ch->in_room == NULL
    ||   ch->desc == NULL
    ||   IS_SET(ch->in_room->room_flags, ROOM_PRIVATE)
    ||   IS_SET(ch->in_room->room_flags, ROOM_SOLITARY)
    ||   IS_SET(ch->in_room->room_flags, ROOM_IMP_ONLY)
    ||   IS_SET(ch->in_room->room_flags, ROOM_GODS_ONLY)
    ||   IS_SET(ch->in_room->room_flags, ROOM_HEROES_ONLY)
    ||   IS_SET(ch->in_room->room_flags, ROOM_NEWBIES_ONLY)
    ||   IS_AFFECTED(ch,AFF_CURSE))
    {
	send_to_char("You fail to submit a visual reference of this place to memory!\n\r",ch);
	return;
    }

    if  (!IS_SET(ch->act, PLR_HOLYLIGHT)
    &&   room_is_dark( ch->in_room ))
    {
	send_to_char("You can't see a thing!  How could you possibly remember your surroundings?\n\r",ch);
	return;
    }

    if ( IS_AFFECTED(ch, AFF_BLIND) )
    {
	send_to_char("You can't see a thing!  How could you possibly remember your surroundings?\n\r",ch);
	return;
    }

    if (IS_AFFECTED2(ch, AFF_PARADISE))
    {
	send_to_char("Your caught in an illusion and therefor can not submit this place to memory.\n\r",ch);
	return;
    }

    if (ch->fighting != NULL)
    {
        send_to_char( "Your too busy fighting to memorize your surroundings.\n\r", ch );
        return;
    }

    if ( ch->recall_point == ch->in_room->vnum )
    {
        send_to_char( "You've already commited this place to memory.\n\r", ch );
        return;
    }

    if ( ch->exp < 1000 )
    {
        send_to_char( "You no longer have enough experience to cast this spell.\n\r", ch );
        return;
    }
    
    send_to_char( "You hold out a rose and let it drop to the ground, then slowly look up\n\r", ch );
    send_to_char( "and begin to examine your surroundings. Paying close attention to detail,\n\r", ch );
    send_to_char( "you commit this place to memory for future reference.\n\r", ch );
    send_to_char( "\n\r", ch );
    send_to_char( "This place is now marked as your origin of recall.\n\r", ch );
    act("$n holds out a single rose, then lets it drop to the ground.",ch,NULL,NULL,TO_ROOM);
    act("$e then looks up and slowly looks around, as if commiting a visual reference of this place to memory.",ch,NULL,NULL,TO_ROOM);
    ch->recall_point = ch->in_room->vnum;
    ch->exp -= 1000;

  return;
}
