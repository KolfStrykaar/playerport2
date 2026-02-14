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
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "merc.h"
#include "music.h"
#include "recycle.h"
#include "interp.h"

/*
 * Global functions.
 */

void raw_kill		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );

/*
 * These are the declares for the song_fun's
 * Add new declares for new songs you write here
 */
DECLARE_SONG_FUN(	song_of_swift_travel	);
DECLARE_SONG_FUN(	song_of_huma		);
DECLARE_SONG_FUN(	song_of_the_journeyman	);
DECLARE_SONG_FUN(	song_of_fortitude		);
DECLARE_SONG_FUN(	song_of_mass_holding	);
DECLARE_SONG_FUN( song_of_souls		);
DECLARE_SONG_FUN( song_of_silence		);
DECLARE_SONG_FUN( song_of_inebriation	);
DECLARE_SONG_FUN( song_of_revelation	);
DECLARE_SONG_FUN( song_of_serenity		);
DECLARE_SONG_FUN( song_of_clumsiness	);
DECLARE_SONG_FUN( song_of_mana		);
DECLARE_SONG_FUN(	song_of_rage		);
DECLARE_SONG_FUN(	song_of_diplomacy		);
DECLARE_SONG_FUN(	song_of_compassion	);
DECLARE_SONG_FUN(	song_of_sustenance	);
DECLARE_SONG_FUN(	song_of_idiocy		);
DECLARE_SONG_FUN(	song_of_barbarians	);
DECLARE_SONG_FUN(	song_of_nightmares	);
DECLARE_SONG_FUN(	song_of_wollen_feet	);
DECLARE_SONG_FUN(	song_of_the_kender	);
DECLARE_SONG_FUN(	song_of_the_rose		);


/*
 * Table for the songs of the MUD
 * add a new set of brackets for each new song you add, and dont forget
 * to increment MAX_SONGS in merc.h
 */
const   struct  song_type       song_table [MAX_SONGS] =
{
/*  {
	"Song name", "Song list name", level, song_fun,
	min position, min mana, beats
    }
*/

    {
	"song of barbarians", "Song of Barbarians", 5, song_of_barbarians,
	POS_STANDING, 60, 25
    },

    {
	"song of revelation", "Song of Revelation", 10, song_of_revelation,
	POS_STANDING, 20, 25
    },

    {
      "song of swift travel", "Song of Swift Travel", 15, song_of_swift_travel,
	POS_STANDING, 30, 25
    },

    {
      "song of huma", "Song of Huma", 20, song_of_huma,
	POS_RESTING, 40, 25
    },

    {
	"song of serenity", "Song of Serenity", 23, song_of_serenity,
	POS_FIGHTING, 25, 25
    },

    {
      "song of the journeyman", "Song of the Journeyman", 25, song_of_the_journeyman,
	POS_RESTING, 50, 25
    },

    {
      "song of compassion", "Song of the Compassion", 27, song_of_compassion,
	POS_RESTING, 30, 25
    },

    {
      "song of fortitude", "Song of Fortitude", 30, song_of_fortitude,
	POS_RESTING, 50, 25
    },

    {
      "song of nightmares", "Song of Nightmares", 32, song_of_nightmares,
	POS_FIGHTING, 70, 25
    },

    {
	"song of inebriation", "Song of Inebriation", 35, song_of_inebriation,
	POS_STANDING, 40, 25
    },

    {
	"song of the kender", "Song of the Kender", 38, song_of_the_kender,
	POS_STANDING, 30, 25
    },

    {
	"song of silence", "Song of Silence", 40, song_of_silence,
	POS_STANDING, 45, 25
    },

    {
	"song of sustenance", "Song of Sustenance", 45, song_of_sustenance,
	POS_RESTING, 105, 25
    },

    {
	"song of wollen feet", "Song of Wollen Feet", 48, song_of_wollen_feet,
	POS_RESTING, 80, 25
    },

    {
      "song of rage", "Song of Rage", 50, song_of_rage,
      POS_FIGHTING, 50, 25
    },

    {
      "song of diplomacy", "Song of Diplomacy", 55, song_of_diplomacy,
      POS_STANDING, 150, 25
    },

    {
      "song of mass holding", "Song of Mass Holding", 63, song_of_mass_holding,
	POS_STANDING, 75, 25
    },

    {
	"song of clumsiness", "Song of Clumsiness", 65, song_of_clumsiness,
	POS_FIGHTING, 35, 25
    },

    {
      "song of the rose", "Song of the Rose", 68, song_of_the_rose,
      POS_STANDING, 85, 25
    },

    {
	"song of mana", "Song of Mana", 70, song_of_mana,
	POS_STANDING, 65, 25
    },

    {
	"song of souls", "Song of Souls", 80, song_of_souls,
	POS_STANDING, 150, 25
    },

    {
      "song of idiocy", "Song of Idiocy", 95, song_of_idiocy,
      POS_FIGHTING, 200, 25
    },

};

/* This function lists songs and their levels...the {Y and {G stuff is for color
 * remove it if you dont have color
 */
void do_songs(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    int i;

    if ( ch->class != class_lookup("bard") && !(IS_IMMORTAL(ch)))
    {
        send_to_char( "Since when did you learn how to use Bard skills?\n\r",ch);
        return;
    }

    sprintf(buf, "{Y[%-20s %5s]{x\n\r", "Song Name", "Level");
    send_to_char(buf,ch);

    sprintf(buf, "----------------------------\n\r");
    send_to_char(buf,ch);

    for (i = 0; i < MAX_SONGS; i++)
    {
	sprintf(buf, "{Y[{G%-22s {C%3d{Y]{x\n\r", song_table[i].listname, song_table[i].level);
	send_to_char(buf,ch);
    }
    return;
}


/* looks up a song */
int song_lookup( const char *name )
{
    int songnum;

    for ( songnum = 0; songnum < MAX_SKILL; songnum++ )
    {
	if ( song_table[songnum].name == NULL )
	    break;
	if ( LOWER(name[0]) == LOWER(song_table[songnum].name[0])
	&&   !str_cmp( name, song_table[songnum].name ) )
	    return songnum;
    }

    return -1;
}


/* actually starts the playing process */
void do_play( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *instrument;
    int songnum, chance, mana, level;

    if ((chance = get_skill(ch,gsn_play)) == 0)
    {
	send_to_char("You have no clue how to play music.\n\r", ch);
	return;
    }

    if (IS_AFFECTED2( ch, AFF_FORGET))
    {
	send_to_char("You can't seem to remember the first thing about playing musical instruments.\n\r",ch);
	return;
    }

    if (is_affected(ch,gsn_fingerbreak) 
    && ( ch->religion_id == RELIGION_NONE) )
    {
	send_to_char("Your fingers are too smashed to properly play a musical instrument.\n\r", ch );
	return;
    }

    if ((instrument = get_eq_char(ch, WEAR_HOLD)) == NULL)
    {
	send_to_char("You aren't carrying an instrument.\n\r", ch);
	return;
    }

    if (instrument->item_type != ITEM_INSTRUMENT)
    {
	send_to_char("You aren't carrying an instrument.\n\r", ch);
	return;
    }

    if (argument[0] == '\0')
    {
	send_to_char("Play what?\n\r", ch);
	return;
    }

    songnum = song_lookup(argument);

    if (songnum == -1)
    {
	send_to_char("That isn't a song.\n\r", ch);
	return;
    }

    if (ch->level < song_table[songnum].level)
    {
	send_to_char("You haven't been able to master that song yet.\n\r", ch);
	return;
    }

    /*
	Below:  This basically lets there be songs that you can play while resting
	and songs that must be standing to play.  This way you cant play a damaging
	song to start a fight while sitting/resting
    */

    if (ch->position < song_table[songnum].minimum_position)
    {
	send_to_char("You need to be standing up to play that song.\n\r", ch);
	return;
    }

    /*
	Below: This can be changed to an algorith to allow for varied mana
	based on a comparison to level of spell vs players level, etc
    */

    mana = song_table[songnum].min_mana;

    if (!IS_NPC(ch) && ch->mana < mana)
    {
	send_to_char("You don't have enough mana.\n\r", ch);
	return;
    }

    act("$n plays a melody on $p.",ch,instrument,NULL,TO_ROOM);
    act("You play a melody on $p.",ch,instrument,NULL,TO_CHAR);

    WAIT_STATE( ch, song_table[songnum].beats );

    chance = chance - (20 / (1 + ch->level - song_table[songnum].level));

	/* average of level of player and level of instrument */
    level = (ch->level + instrument->level) / 2;

    if (number_percent() > chance)
    {
	ch->mana -= mana / 2;
	act("$n's fingers slip and the song ends abruptly.",ch,NULL,NULL,TO_ROOM);
	send_to_char("Your fingers slip and the song ends abruptly.\n\r", ch);
	check_improve(ch,gsn_play,FALSE,1);
	return;
    }
    else /* actually start playing the song */
    {
	ch->mana -= mana;
	if (IS_NPC(ch) || class_table[ch->class].fMana)
	    (*song_table[songnum].song_fun) ( songnum, level, ch );
	else
	    (*song_table[songnum].song_fun) ( songnum, 3 * level/4, ch );
	check_improve(ch,gsn_play,TRUE,1);
    }

    return;
}

void song_of_huma( int songnum, int level, CHAR_DATA *ch )
{
  CHAR_DATA *vch;
  AFFECT_DATA af1, af2;

  act("$n begins to sing the Song of Huma.",ch,NULL,NULL,TO_ROOM);
  send_to_char("You start singing the Song of Huma.\n\r", ch);

  af1.where = TO_AFFECTS;
  af1.type = skill_lookup("bless");
  af1.level = level;
  af1.duration = level/2;
  af1.location = APPLY_HITROLL;
  af1.modifier = level/8;
  af1.bitvector = 0;

  af2.where = TO_AFFECTS;
  af2.type = skill_lookup("armor");
  af2.level = level;
  af2.duration = level/2;
  af2.location = APPLY_AC;
  af2.modifier = -20;
  af2.bitvector = 0;

  if (!is_affected( ch, af1.type ))
  {
    act("$n glows briefly.",ch,NULL,NULL,TO_ROOM);
    send_to_char("You glow briefly.\n\r", ch);
    affect_to_char(ch,&af1);
  }

  if (!is_affected( ch, af2.type ))
  {
    act("$n is suddenly surrounded by a glowing suit of armor.",ch,NULL,NULL,TO_ROOM);
    send_to_char("You are suddenly surrounded by a glowing suit of armor.\n\r", ch);
    affect_to_char(ch,&af2);
  }

  for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room )
  {
    if (ch == vch)
      continue;

    if (!can_see (ch, vch))
      continue;

    if (!IS_NPC(vch))
    {
      if (number_percent() <= 80 && !is_affected( vch, af1.type ))
      {
	act("$n glows briefly.",vch,NULL,NULL,TO_ROOM);
	send_to_char("You glow briefly.\n\r", vch);
	affect_to_char( vch, &af1 );
      }
      if (number_percent() <= 80 && !is_affected( vch, af2.type ))
      {
	act("$n is suddenly surrounded by a glowing suit of armor.",vch,NULL,NULL,TO_ROOM);
	send_to_char("You are suddenly surrounded by a glowing suit of armor.\n\r", vch);
	affect_to_char( vch, &af2 );
      }
    }
  }
  return;
}

void song_of_the_journeyman( int songnum, int level, CHAR_DATA *ch )
{
    CHAR_DATA *vch;

   act("$n begins to sing the Song of the Journeyman.",ch,NULL,NULL,TO_ROOM);
   send_to_char("You begin singing the Song of the Journeyman.\n\r", ch);

    ch->move = UMIN( ch->move + level, ch->max_move );

    act("$n looks less tired and more energetic.",ch,NULL,NULL,TO_ROOM);
    send_to_char("Your feet feel more relieved and less sore.\n\r", ch);

    for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room )
    {
	if (ch == vch)
	    continue;

	if (!IS_NPC(vch) && is_same_group( vch, ch ) )
	{
	    if (number_percent() <= 80)
	    {
		act("$n looks less tired and more energetic.",vch,NULL,NULL,TO_ROOM);
		send_to_char("Your feet feel more relieved and less sore.\n\r", vch);
            vch->move = UMIN( vch->move + level, vch->max_move );
	    }
	}
    }

    return;
}

void song_of_swift_travel( int songnum, int level, CHAR_DATA *ch )
{
    AFFECT_DATA af;

   act("$n begins to sing the Song of Swift Travel.",ch,NULL,NULL,TO_ROOM);
   send_to_char("You start singing the Song of Swift Travel.\n\r", ch);

    af.where	= TO_AFFECTS;
    af.type	= skill_lookup("levitate");
    af.level	= level;
    af.duration = level/4;
    af.location = 0;
    af.modifier = 0;
    af.bitvector = AFF_FLYING;

    if (!is_affected( ch, af.type ))
    {
	act("$n's feet begin to slowly lift off the ground.",ch,NULL,NULL,TO_ROOM);
	send_to_char("The musical notes slowly lift you into the air.\n\r", ch);
	affect_to_char(ch,&af);
    }

    return;
}

void song_of_fortitude( int songnum, int level, CHAR_DATA *ch )
{
    AFFECT_DATA af1, af2;

   act("$n begins to sing the Song of Fortitude.",ch,NULL,NULL,TO_ROOM);
   send_to_char("You begin singing the Song of Fortitude.\n\r", ch);

    af1.where 		= TO_AFFECTS;
    af1.type 		= skill_lookup("haste");
    af1.level 		= level;
    af1.duration 	= level/4;
    af1.location 	= APPLY_HITROLL;
    af1.modifier 	= level/8;
    af1.bitvector 	= 0;

    af2.where 		= TO_AFFECTS;
    af2.type 		= skill_lookup("armor");
    af2.level 		= level;
    af2.duration 	= level/4;
    af2.location 	= APPLY_AC;
    af2.modifier 	= -100;
    af2.bitvector 	= 0;

    if (!is_affected( ch, af1.type ))
    {
	send_to_char("You feel a tingling sensation.\n\r", ch);
	affect_to_char(ch,&af1);
    }

    if (!is_affected( ch, af2.type ))
    {
	act("$n's skin thickens, becoming harder like stone.",ch,NULL,NULL,TO_ROOM);
	send_to_char("Your skin hardens, becoming less vulnerable to attacks.\n\r", ch);
	affect_to_char(ch,&af2);
    }

    return;
}

void song_of_mass_holding( int songnum, int level, CHAR_DATA *ch )
{
    CHAR_DATA *vch;
    AFFECT_DATA af;

   act("$n begins to sing a captivating song.",ch,NULL,NULL,TO_ROOM);
   send_to_char("You begin singing the Song of Mass Holding.\n\r", ch);

    af.where 	= TO_AFFECTS2;
    af.type	= skill_lookup("paralysis");
    af.level 	= level;
    af.duration = 1;
    af.location = 0;
    af.modifier = 0;
    af.bitvector = AFF_PARALYSIS;

    for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room )
    {
	if (ch == vch)
	    continue;

    	if (!IS_NPC(vch) && is_same_group( vch, ch ) )
            continue;

	if (!IS_NPC(vch))
	{
	    if (number_percent() <= 60 && !IS_AFFECTED2(vch, AFF_PARALYSIS ))
	    {
		act("$n can not move, briefly awe struck by the captivating magic of the song.",vch,NULL,NULL,TO_ROOM);
		send_to_char("The music captivates your mind, putting you in a temporary paralysis.\n\r", vch);
		affect_to_char( vch, &af );
	    }
	}
    }

    return;
}

void song_of_souls( int songnum, int level, CHAR_DATA *ch )
{
	CHAR_DATA *vch;
	int mhp = 0;

	if(ch->hit < 100 || ch->move < 100)
	{
		send_to_char("Rest a while before trying to sing such an energetic song.\n\r",ch);
		return;
	}

	ch->hit /= 2;
	ch->move = 0;
	act("$n begins to sing a song which revives your spirit.",ch,NULL,NULL,TO_ROOM);
	send_to_char("You begin singing the Song of Souls.\n\r",ch);

	for(vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room )
	{
		mhp = vch->max_hit*3/4;

		if(ch == vch
		|| IS_NPC(vch)
		|| !is_same_group(ch,vch)
		|| vch->hit > mhp 
		|| number_percent() <= 15 )
			continue;

		if( IS_AFFECTED(vch, AFF_POISON) )
		{
			affect_strip(vch, gsn_poison);
			REMOVE_BIT(vch->affected_by, AFF_POISON);
		      send_to_char("A warm feeling runs through your body.\n\r",vch);
        		act("$n looks much better.",vch,NULL,NULL,TO_ROOM);
		}
		
		if( IS_AFFECTED(vch, AFF_PLAGUE) )
		{
			affect_strip(vch, gsn_plague);
			REMOVE_BIT(vch->affected_by, AFF_PLAGUE);
			send_to_char("Your sores vanish.\n\r",vch);
			act("$n looks relieved as $s sores vanish.",vch,NULL,NULL,TO_ROOM);
		}

		vch->hit = vch->max_hit;
		send_to_char("Your spirit is filled with energy, as you listen to the song!\n\r",vch);
		act("$n's spirit is filled with energy, as $e listens to the song!",vch,NULL,NULL,TO_ROOM);
	}
	send_to_char("You feel tired after singing this energetic song.\n\r",ch);
	return;
}

void song_of_silence( int songnum, int level, CHAR_DATA *ch )
{
    CHAR_DATA *vch;
    AFFECT_DATA af;
    int chance = 0;

   act("$n begins to sing a song which is very beautiful.",ch,NULL,NULL,TO_ROOM);
   send_to_char("You begin singing the Song of Silence.\n\r", ch);

   af.where	 = TO_AFFECTS2;
   af.type   = skill_lookup("silence");
   af.level	 = level;
   af.duration  = 2;
   af.modifier  = 0;
   af.location  = 0;
   af.bitvector = AFF_SILENCE;

   for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room )
   {
	if( ch == vch
	 || IS_NPC(vch)
	 || (!IS_NPC(vch) && is_same_group(vch, ch))
	 || IS_IMMORTAL(vch)
	 || is_safe(ch,vch)
	 || ch->level > (vch->level + 12)
	 || IS_AFFECTED2(vch,AFF_SILENCE) )
		continue;

	if( ch->level >= vch->level + 5)
		chance = 50;
	else
		chance = 30;

	if (number_percent() <= chance)
	{
		act("$n is stricken speechless by the beauty of the song.",vch,NULL,NULL,TO_ROOM);
		send_to_char("You are so captivated by the beautiful song, that you are rendered speechless!\n\r",vch);
		affect_to_char( vch, &af );
	}

    }
    return;
}

void song_of_inebriation( int songnum, int level, CHAR_DATA *ch )
{
    CHAR_DATA *vch;
    int chance = 0;

   act("$n begins to sing a tavern song.",ch,NULL,NULL,TO_ROOM);
   send_to_char("You begin singing the Song of Inebriation.\n\r", ch);

   for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room )
   {
	if( ch == vch
	 || IS_NPC(vch)
	 || (!IS_NPC(vch) && is_same_group(vch, ch))
	 || IS_IMMORTAL(vch)
	 || is_safe(ch,vch)
	 || ch->level > (vch->level + 12)
	 || vch->pcdata->condition[COND_DRUNK] > 5 )
		continue;

	if( ch->level >= vch->level + 5)
		chance = 60;
	else
		chance = 30;

	if (number_percent() <= chance)
	{
		gain_condition(vch, COND_DRUNK, number_range(10,15));
		act("$n seems a bit drunk as he goes for another drink... and orders water!",vch,NULL,NULL,TO_ROOM);
		send_to_char("You begin to feel tipsy.. Perhaps another drink? *hic*\n\r",vch);
	}

    }
    return;
}

void song_of_revelation( int songnum, int level, CHAR_DATA *ch )
{
    CHAR_DATA *vch;
    AFFECT_DATA af;
    AFFECT_DATA af2;

   act("$n begins to sing a song which provokes your mind.",ch,NULL,NULL,TO_ROOM);
   send_to_char("You begin singing the Song of Revelation.\n\r", ch);

   af.where		= TO_AFFECTS;
   af.type   	= skill_lookup("detect invis");
   af.level		= level;
   af.duration  	= level/3;
   af.modifier  	= 0;
   af.location  	= APPLY_NONE;
   af.bitvector 	= AFF_DETECT_INVIS;

   af2.where  	= TO_AFFECTS;
   af2.type   	= skill_lookup("detect hidden");
   af2.level  	= level;
   af2.duration  	= level/3;
   af2.modifier  	= 0;
   af2.location  	= APPLY_NONE;
   af2.bitvector 	= AFF_DETECT_HIDDEN;

   for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room )
   {
     if( IS_NPC(vch)
      || (!IS_NPC(vch) && !is_same_group(ch, vch))
      || (IS_AFFECTED(vch,AFF_DETECT_INVIS) && IS_AFFECTED(vch,AFF_DETECT_HIDDEN)) )
       continue;

     act("$n's eyes glint for a moment as $e listens to the song.",vch,NULL,NULL,TO_ROOM);
     send_to_char("You have gained more insight about your surroundings after hearing the song.\n\r",vch);

     if( IS_AFFECTED(vch,AFF_DETECT_INVIS) && !IS_AFFECTED(vch,AFF_DETECT_HIDDEN) )
     {
       affect_to_char( vch, &af2 );
       continue;
     }
     else if( !IS_AFFECTED(vch,AFF_DETECT_INVIS) && IS_AFFECTED(vch,AFF_DETECT_HIDDEN) )
     {
       affect_to_char( vch, &af );
       continue;
     }
     else if( !IS_AFFECTED(vch,AFF_DETECT_INVIS) && !IS_AFFECTED(vch,AFF_DETECT_HIDDEN) )
     {
       affect_to_char( vch, &af );
       affect_to_char( vch, &af2 );
       continue;
     }
     else
     {
       send_to_char("Oops. Please report this to Paladine.\n\r",ch);
       continue;
     }

   }
   return;
}

void song_of_serenity( int songnum, int level, CHAR_DATA *ch )
{
  CHAR_DATA *vch;
  AFFECT_DATA af;
  int chance = 0;

  act("$n begins to sing a peaceful song.",ch,NULL,NULL,TO_ROOM);
  send_to_char("You begin singing the Song of Serenity.\n\r", ch);

  af.where 	= TO_AFFECTS;
  af.type	= skill_lookup("calm");
  af.level 	= level;
  af.duration 	= 3;
  af.location 	= 0;
  af.modifier 	= 0;
  af.bitvector 	= AFF_CALM;

  for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room )
  {
    if(ch == vch
    || is_affected(vch, skill_lookup("frenzy")) )
    	continue;

    if( ch->level >= vch->level + 5)
      chance = 60;
    else
      chance = 40;

    if (number_percent() <= chance)
    {
      if (vch->fighting || vch->position == POS_FIGHTING)
      {
	act("The peaceful song soothes $n, causing $m to stop fighting.",vch,NULL,NULL,TO_ROOM);
	send_to_char("The peaceful song soothes you, taking away the urge to fight.\n\r", vch);
	stop_fighting(vch,TRUE);

	if( !is_same_group(ch, vch) && !IS_AFFECTED(vch,AFF_CALM) )
	  affect_to_char( vch, &af );

      }
    }

  }

  return;
}

void song_of_clumsiness( int songnum, int level, CHAR_DATA *ch )
{
    CHAR_DATA *vch;
    AFFECT_DATA af;
    int chance = 0;

   act("$n begins to sing a slow, sad song of times long lost.",ch,NULL,NULL,TO_ROOM);
   send_to_char("You begin singing the Song of Clumsiness.\n\r", ch);

    af.where     = TO_AFFECTS;
    af.type      = skill_lookup("slow");
    af.level     = level;
    af.duration  = level/2;
    af.location  = APPLY_DEX;
    af.modifier  = -5;
    af.bitvector = AFF_SLOW;

   for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room )
   {
	if( ch == vch
	 || (!IS_NPC(vch) && is_same_group(vch, ch))
       || (IS_NPC(vch) && is_same_group(vch, ch))
	 || IS_IMMORTAL(vch)
	 || is_safe(ch,vch)
	 || IS_AFFECTED(vch,AFF_SLOW) )
		continue;

	if( ch->level >= vch->level + 5 )
		chance = 65;
	else
		chance = 40;

	if( IS_AFFECTED(vch,AFF_HASTE) )
		chance = 25;

	if (number_percent() <= chance)
	{
		act("$n is really moved by the song, as $e looks a bit lethargic after hearing it.",vch,NULL,NULL,TO_ROOM);
		send_to_char("You feel lethargic as you listen to the song.\n\r",vch);
		if( IS_AFFECTED(vch,AFF_HASTE) )
		{
			affect_strip( vch, skill_lookup("haste") );
			REMOVE_BIT(vch->affected_by, AFF_HASTE);
		}
		affect_to_char( vch, &af );
	}

    }
    return;
}

void song_of_mana( int songnum, int level, CHAR_DATA *ch )
{
    CHAR_DATA *vch;
    AFFECT_DATA af;

   act("$n begins to sing a song which fills your spirit.",ch,NULL,NULL,TO_ROOM);
   send_to_char("You begin singing the Song of Mana.\n\r", ch);

   af.where	 = TO_AFFECTS;
   af.type   = skill_lookup("regeneration");
   af.level	 = level;
   af.duration  = 45;
   af.location  = 0;
   af.modifier  = 0;
   af.bitvector = AFF_REGENERATION;

   for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room )
   {
	if( IS_NPC(vch)
	 || (!IS_NPC(vch) && !is_same_group(ch, vch))
       || IS_AFFECTED(vch,AFF_REGENERATION) )
		continue;

	act("a light blue outline surrounds $n momentarily.",vch,NULL,NULL,TO_ROOM);
	send_to_char("Your spirit feels full of energy after listining to the song.\n\r",vch);
	affect_to_char( vch, &af );

    }
    return;
}

void song_of_rage( int songnum, int level, CHAR_DATA *ch )
{
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  int dam;

  if (ch->fight_pos == FIGHT_REAR)
  {
    send_to_char("The sounds of battle is too loud for anyone to hear your song from this\r\n",ch);
    send_to_char("far away. Try moving closer, either to the Middle or Front ranks.\r\n",ch);
    return;
  }

  if (ch->fighting == NULL)
  {
    send_to_char("You need to be fighting someone for this song to be effective.\n\r", ch);
    return;
  }

  act("$n strikes a bad chord and shrieks out a horrible sound.",ch,NULL,NULL,TO_ROOM);
  send_to_char("You shriek out horrible, shrieking sounds.\n\r", ch);

  for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
  {

    vch_next = vch->next_in_room;

    if (ch == vch)
      continue;

    if (!IS_NPC(vch) && is_same_group( vch, ch ) )
      continue;

    if (ch->fighting != vch)
      continue;

    if (vch->in_room == NULL )
      continue;

    dam = dice( level * 12 / 11, 10 );

    act("$n is hit hard by the force of sonic waves.",vch,NULL,NULL,TO_ROOM);
    sprintf(buf,"Your sonic waves impale %s with a raging force!\n\r", IS_NPC(vch) ? vch->short_descr : vch->name);
    send_to_char( buf, ch );
    sprintf(buf,"Sonic waves hit you with a raging force!\n\r");
    send_to_char( buf, vch );
    damage(ch, vch, dam*4, -8, DAM_SOUND, TRUE);
  }
  return;
}

void song_of_diplomacy( int songnum, int level, CHAR_DATA *ch )
{
  CHAR_DATA *vch;
  AFFECT_DATA af;

  act("$n begins to sing a song with diplomatic harmonies.",ch,NULL,NULL,TO_ROOM);
  send_to_char("You begin singing the Song of Diplomacy.\n\r", ch);

  for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room )
  {
    if( IS_NPC(vch)
    || IS_AFFECTED2(vch,AFF_SYMPATHY)
    || (!IS_NPC(vch)
    && !is_same_group(ch, vch)))
      continue;

    act("$n focuses on the lyrics of the song.",vch,NULL,NULL,TO_ROOM);
    send_to_char("You have gained better diplomatic attributes.\n\r",vch);

    af.where		= TO_AFFECTS2;
    af.type   		= skill_lookup("sympathy");
    af.level		= level - 5;
    af.duration  		= level/4;
    af.modifier  		= 0;
    af.location  		= 0;
    af.bitvector 		= AFF_SYMPATHY;
    affect_to_char( vch, &af );

    af.where		= TO_AFFECTS;
    af.location  		= APPLY_CHR;
    af.modifier  		= 1 + (level >= 18) + (level >= 25) + (level >= 32);
    af.bitvector 		= 0;
    affect_to_char( vch, &af );
  }
  return;
}

void song_of_compassion( int songnum, int level, CHAR_DATA *ch )
{
  CHAR_DATA *vch;
  AFFECT_DATA af;

  act("$n begins to sing a song with a compassionate melody.",ch,NULL,NULL,TO_ROOM);
  send_to_char("You begin singing the Song of Compassion.\n\r", ch);

  for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room )
  {
    if( IS_NPC(vch)
    || (IS_AFFECTED(vch, AFF_PROTECT_EVIL)
    &&  IS_AFFECTED(vch, AFF_PROTECT_GOOD))
    || (!IS_NPC(vch)
    && !is_same_group(ch, vch)))
      continue;

    act("$n smiles with compassion to the lyrics of the song.",vch,NULL,NULL,TO_ROOM);
    send_to_char("You have become more compassionate to both Evil and Good.\n\r",vch);

    if (!IS_AFFECTED(vch, AFF_PROTECT_EVIL))
    {
      af.where     		= TO_AFFECTS;
      af.type      		= skill_lookup("protection evil");
      af.level     		= level;
      af.duration  		= 36;
      af.location  		= APPLY_SAVING_SPELL;
      af.modifier  		= -6;
      af.bitvector 		= AFF_PROTECT_EVIL;
      affect_to_char( vch, &af );
    }

    if (!IS_AFFECTED(vch, AFF_PROTECT_GOOD))
    {
      af.where     		= TO_AFFECTS;
      af.type      		= skill_lookup("protection good");
      af.level     		= level;
      af.duration  		= 36;
      af.location  		= APPLY_SAVING_SPELL;
      af.modifier  		= -6;
      af.bitvector 		= AFF_PROTECT_GOOD;
      affect_to_char( vch, &af );
    }
  }
  return;
}

void song_of_sustenance( int songnum, int level, CHAR_DATA *ch )
{
  CHAR_DATA *vch;
  int amount;

  act("$n begins to sing a warming and fulfilling melody.",ch,NULL,NULL,TO_ROOM);
  send_to_char("You begin singing the Song of Sustenance.\n\r", ch);

  for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room )
  {
    if( IS_NPC(vch)
    || (!IS_NPC(vch)
    && !is_same_group(ch, vch)))
      continue;

    act("$n smiles and pats $s belly.",vch,NULL,NULL,TO_ROOM);
    send_to_char("Your thirst has been quenched and your hunger sated.\n\r",vch);

    amount = number_fuzzy(5+ch->level);
    gain_condition( vch, COND_FULL, amount);
    gain_condition( vch, COND_THIRST, amount);
  }
  return;
}

void song_of_idiocy( int songnum, int level, CHAR_DATA *ch )
{
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  AFFECT_DATA af;

  if (ch->fight_pos == FIGHT_REAR)
  {
    send_to_char("The sounds of battle is too loud for anyone to hear your song from this\r\n",ch);
    send_to_char("far away. Try moving closer, either to the Middle or Front ranks.\r\n",ch);
    return;
  }

  if (ch->fighting == NULL)
  {
    send_to_char("You need to be fighting someone for this song to be effective.\n\r", ch);
    return;
  }

  act("$n strikes a bad chord and shrieks out a mind shattering sound.",ch,NULL,NULL,TO_ROOM);
  send_to_char("You shriek out horrible, mind shattering sounds.\n\r", ch);

  for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
  {
    vch_next = vch->next_in_room;

    if (ch == vch)
      continue;

    if (!IS_NPC(vch) && is_same_group( vch, ch ) )
      continue;

    if (ch->fighting != vch)
      continue;

    if (vch->in_room == NULL )
      continue;

    if (is_affected(vch, skill_lookup("mind shatter")))
      continue;

    af.where	 	= TO_VULN;
    af.type      		= skill_lookup("mind shatter");
    af.level	 	= level;
    af.duration 	 	= 2;
    af.location  		= APPLY_NONE;
    af.modifier  		= 0;     
    af.bitvector 		= VULN_MAGIC;
    affect_to_char( vch, &af );

    af.where     		= TO_VULN;
    af.location  		= APPLY_NONE;
    af.modifier  		= 0;
    af.bitvector 		= VULN_BASH;
    affect_to_char( vch, &af );

    af.where     		= TO_AFFECTS;
    af.location  		= APPLY_SAVING_SPELL;
    af.modifier 	 	= 25;
    af.bitvector 		= 0;
    affect_to_char( vch, &af );

    af.where     		= TO_AFFECTS;
    af.location  		= APPLY_AC;
    af.modifier  		= 100;
    af.bitvector 		= 0;
    affect_to_char( vch, &af );

    act("$n's mind begins to tear apart!",vch,NULL,NULL,TO_ROOM);
    sprintf(buf,"Your mind begins to tear apart!\n\r");
    send_to_char( buf, vch );
  }
  return;
}

void song_of_barbarians( int songnum, int level, CHAR_DATA *ch )
{
  CHAR_DATA *vch;
  AFFECT_DATA af;

  act("$n begins to sing a song with a motivational tune.",ch,NULL,NULL,TO_ROOM);
  send_to_char("You begin singing the Song of Barbarians.\n\r", ch);

  for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room )
  {
    if( IS_NPC(vch)
    ||  is_affected(vch, skill_lookup("frenzy"))
    ||  IS_AFFECTED(vch,AFF_BERSERK)
    || (!IS_NPC(vch)
    && !is_same_group(ch, vch)))
      continue;

    act("$n becomes enraged with a Barbarian like frenzy.",vch,NULL,NULL,TO_ROOM);
    send_to_char("You suddenly become enraged with a Barbarian like frenzy.\n\r",vch);

    af.where     		= TO_AFFECTS;
    af.type      		= skill_lookup("frenzy");
    af.level	 	= level;
    af.duration	 	= level / 4;
    af.modifier  		= level / 6;
    af.bitvector 		= 0;

    af.location  		= APPLY_HITROLL;
    affect_to_char(vch,&af);

    af.location  		= APPLY_DAMROLL;
    affect_to_char(vch,&af);

    af.modifier  		= 10 * (level / 12);
    af.location  		= APPLY_AC;
    affect_to_char(vch,&af);
  }
  return;
}
void song_of_nightmares( int songnum, int level, CHAR_DATA *ch )
{
  ROOM_INDEX_DATA *was_in;
  ROOM_INDEX_DATA *now_in;
  CHAR_DATA *vch;
  AFFECT_DATA af;

  if (ch->fight_pos == FIGHT_REAR)
  {
    send_to_char("Your too far away for anyone to hear your song from this\r\n",ch);
    send_to_char("distance. Try moving closer, either to the Middle or Front ranks.\r\n",ch);
    return;
  }

  if (ch->fighting == NULL)
  {
    send_to_char("You need to be fighting someone for this song to be effective.\n\r", ch);
    return;
  }

  act("$n strikes a bad chord and shrieks out a nightmarish sound.",ch,NULL,NULL,TO_ROOM);
  send_to_char("You shriek out horrible, nightmarish sound.\n\r", ch);

  for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room )
  {

    if (!IS_NPC(vch) && is_same_group( vch, ch ) )
      continue;

    if (IS_AFFECTED(vch,AFF_BERSERK) 
    || is_affected(vch,skill_lookup("frenzy")))
      continue;

    if (IS_AFFECTED2( vch, AFF_FEAR ))
      continue;

    if (ch->fighting != vch)
      continue;

    if ( !vch->in_room
    || IS_SET( vch->in_room->room_flags, ROOM_SAFE      )
    || IS_SET( vch->in_room->room_flags, ROOM_PRIVATE   )
    || IS_SET( vch->in_room->room_flags, ROOM_SOLITARY  )
    || IS_SET( vch->in_room->room_flags, ROOM_NO_RECALL )
    || vch->riding
    || vch->level >= ch->level 
    || vch->in_room->area != ch->in_room->area )
      continue;

    if (IS_IMMORTAL(vch))
      continue;

    was_in = vch->in_room;

   if (vch != ch)
   {
    EXIT_DATA *pexit;
    int door;

	door = number_door( );
	if ( ( pexit = was_in->exit[door] ) == 0
	||   pexit->u1.to_room == NULL
	||   IS_SET(pexit->exit_info, EX_CLOSED)
	||   IS_SET(pexit->u1.to_room->room_flags, ROOM_NO_MOB) )
	    continue;

      act("$n runs away in fear!",vch,NULL,NULL,TO_ROOM);
      send_to_char("An unnerving nightmare infiltrates your mind and you run away.\n\r",vch);

	move_char( vch, door, FALSE );
	if ( ( now_in = vch->in_room ) == was_in )
	    continue;

      af.where		= TO_AFFECTS2;
      af.type		= skill_lookup("fear");
      af.level		= level;
      af.duration  	= 1;
      af.modifier  	= 0;
      af.location 	= 0;
      af.bitvector 	= AFF_FEAR;
      affect_to_char( vch, &af );
      vch->in_room = was_in;
      vch->in_room = now_in;
      stop_fighting( vch, TRUE );
    }
  }
  return;
}

void song_of_wollen_feet( int songnum, int level, CHAR_DATA *ch )
{
  ROOM_INDEX_DATA *to_room;
  int lvnum;
  int uvnum;

  act("$n begins to sing a song which fills the air with whirling winds.",ch,NULL,NULL,TO_ROOM);
  send_to_char("You begin singing the Song of Wollen Feet.\n\r", ch);

  if ( IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
  || (ch->riding)) 
  {
   send_to_char("Your song failed to do anything at all.\n\r",ch);
   return;
  }

  if ( ch->quit_timer < 0
  && !IS_IMMORTAL(ch)
  && (number_percent() < 50 ))
  {
    send_to_char("Your heart is beating so fast, you lose your concentration on the song.\n\r",ch);
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
    &&   !IS_SET(to_room->room_flags, ROOM_SOLITARY) )
       break;
  }

  act( "$n disappears in a vortex of swirling wind!", ch, NULL, NULL, TO_ROOM );
  char_from_room(ch);
  char_to_room(ch,to_room);
  act( "$n appears in a vortex of swirling wind.", ch, NULL, NULL, TO_ROOM );
  do_look( ch, "auto" );
  return;
}

void song_of_the_kender( int songnum, int level, CHAR_DATA *ch )
{
  CHAR_DATA *vch;
  AFFECT_DATA af;

  act("$n begins to sing a song with an energizing and inspiring tune.",ch,NULL,NULL,TO_ROOM);
  send_to_char("You begin singing the Song of the Kender.\n\r", ch);

  for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room )
  {
    if( IS_NPC(vch)
    ||  is_affected(vch, skill_lookup("haste"))
    ||  IS_AFFECTED(vch,AFF_HASTE)
    || (!IS_NPC(vch)
    && !is_same_group(ch, vch)))
      continue;

    act("$n becomes energized with Kender like speed.",vch,NULL,NULL,TO_ROOM);
    send_to_char("You suddenly become energized with Kender like speed.\n\r",vch);

    af.where     = TO_AFFECTS;
    af.type      = skill_lookup("haste");
    af.level     = level;
    af.duration  = level/4;
    af.location  = APPLY_DEX;
    af.modifier  = 2 + (ch->level >= 35) + (ch->level >= 55) + (ch->level >= 75);
    af.bitvector = AFF_HASTE;
    affect_to_char(vch,&af);
  }
  return;
}

void song_of_the_rose( int songnum, int level, CHAR_DATA *ch )
{
  CHAR_DATA *vch;
  AFFECT_DATA af;

  act("$n begins to sing a tune of great knowledge and enlightenment.",ch,NULL,NULL,TO_ROOM);
  send_to_char("You begin singing the Song of the Rose.\n\r", ch);

  for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room )
  {
    if( IS_NPC(vch)
    ||  is_affected(vch, skill_lookup("enlightenment"))
    ||  IS_AFFECTED2(vch, AFF_ENLIGHTENMENT)
    || (!IS_NPC(vch)
    && !is_same_group(ch, vch)))
      continue;

    act("$n becomes enlightened with vast knowledge of the almighty Majere.",vch,NULL,NULL,TO_ROOM);
    send_to_char("You are able to grasp profundities as never before!\n\r",vch);

    af.where     = TO_AFFECTS2;
    af.type      = skill_lookup("enlightenment");
    af.level     = level;
    af.duration  = level/3;
    af.location  = APPLY_WIS;
    af.modifier  = 1 + (ch->level >= 70) + (ch->level >= 81);
    af.bitvector = AFF_ENLIGHTENMENT;
    affect_to_char(vch,&af);
  }
  return;
}

