/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,	   *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *									   *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael	   *
 *  Chastain, Michael Quan, and Mitchell Tse.				   *
 *									   *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc	   *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.						   *
 *									   *
 *  Much time and thought has gone into this software and you are	   *
 *  benefitting.  We hope that you share your changes too.  What goes	   *
 *  around, comes around.						   *
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
#include <stdlib.h>
#include "merc.h"
#include "interp.h"
#include "magic.h"
#include "recycle.h"

/* local function protos */
static void init_level_spacing( int scale );
static int exp_delta_level( int base, int level );

/* used to get new skills */
void do_gain(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *trainer;

    if (IS_NPC(ch))
	return;

    /* find a trainer */
    for ( trainer = ch->in_room->people; 
	  trainer != NULL; 
	  trainer = trainer->next_in_room)
	if (IS_NPC(trainer) && IS_SET(trainer->act,ACT_GAIN))
	    break;

    if (trainer == NULL || !can_see(ch,trainer))
    {
	send_to_char("You can't do that here.\n\r",ch);
	return;
    }

    if ((ch->in_room == get_room_index(ROOM_VNUM_ALTAR))
    &&  (ch->level >= 11))
    {
      send_to_char( "You cannot use GAIN here anymore beyond level 10.\n\r", ch );
      send_to_char( "You must now seek out the local trainer.\n\r", ch );
      return;
    }

    one_argument(argument,arg);

    if (arg[0] == '\0')
    {
	do_function(trainer, &do_say, "Pardon me?");
	return;
    }

    if (!str_prefix(arg,"convert"))
    {
	if (ch->practice < 10)
	{
	    act("$N tells you 'You are not yet ready.'",
		ch,NULL,trainer,TO_CHAR);
	    return;
	}

	act("$N helps you apply your practice to training.",
		ch,NULL,trainer,TO_CHAR);
	ch->practice -= 10;
	ch->train +=1 ;
	return;
    }

/* Gain Study Command by Hiddukel */
if (!str_prefix(arg,"revert"))
    {
	if (ch->train < 1)
	{
	    act("$N tells you 'You are not yet ready.'",
		ch,NULL,trainer,TO_CHAR);
	    return;
	}

	act("$N helps you trade in your training session for practice sessions.",
		ch,NULL,trainer,TO_CHAR);
	ch->practice += 10;
	ch->train -=1;
	return;
    }

    act("$N tells you 'I do not understand...'",ch,NULL,trainer,TO_CHAR);
}
    



/* RT spells and skills show the players spells (or skills) */

void do_spells(CHAR_DATA *ch, char *argument)
{
    char buffer[4*MSL];
    char arg[MAX_INPUT_LENGTH];
    char spell_list[LEVEL_HERO + 1][MAX_STRING_LENGTH];
    char spell_columns[LEVEL_HERO + 1];
    int sn, level, min_lev = 1, max_lev = LEVEL_HERO, mana;
    bool found = FALSE;
    char buf[MAX_STRING_LENGTH];
 
    if (IS_NPC(ch))
      return;

    max_lev = (ch->level > 11) ? MAX_LEVEL : ch->level+20;
    if (argument[0] != '\0')
    {
	if (argument[0] != '\0')
	{
	    argument = one_argument(argument,arg);
	    if (!is_number(arg))
	    {
		send_to_char("Arguments must be numerical or none.\n\r",ch);
		return;
	    }
	    max_lev = atoi(arg);

	    if (max_lev < 1 || max_lev > ch->level)
	    {
		sprintf(buf,"Levels must be between 1 and %d.\n\r",ch->level);
		send_to_char(buf,ch);
		return;
	    }

	    if (argument[0] != '\0')
	    {
		argument = one_argument(argument,arg);
		if (!is_number(arg))
		{
		    send_to_char("Arguments must be numerical or none.\n\r",ch);
		    return;
		}
		min_lev = max_lev;
		max_lev = atoi(arg);

		if (max_lev < 1 || max_lev > ch->level)
		{
		    sprintf(buf,
			"Levels must be between 1 and %d.\n\r",ch->level);
		    send_to_char(buf,ch);
		    return;
		}

		if (min_lev > max_lev)
		{
		    send_to_char("That would be silly.\n\r",ch);
		    return;
		}
	    }
	}
    }

    /* initialize data */
    for (level = 0; level < LEVEL_HERO + 1; level++)
    {
        spell_columns[level] = 0;
        spell_list[level][0] = '\0';
    }
 
    for (sn = 0; sn < MAX_SKILL; sn++)
    {
        if (skill_table[sn].name == NULL )
	    break;

	if ((level = skill_table[sn].skill_level[ch->class]) < LEVEL_HERO+ 1
	&&  level >= min_lev && level <= max_lev
	&&  skill_table[sn].spell_fun != spell_null
	&&  ch->pcdata->learned[sn] > 0)
        {
	    found = TRUE;
	    level = skill_table[sn].skill_level[ch->class];
	    if (ch->level < level)
	    	sprintf(buf,"%-18s n/a      ", skill_table[sn].name);
	    else
	    {
		mana = UMAX(skill_table[sn].min_mana,
		    100/(2 + ch->level - level));
	        sprintf(buf,"%-18s  %3d mana  ",skill_table[sn].name,mana);
	    }
 
	    if (spell_list[level][0] == '\0')
          	sprintf(spell_list[level],"\n\rLevel %2d: %s",level,buf);
	    else /* append */
	    {
          	if ( ++spell_columns[level] % 2 == 0)
		    strcat(spell_list[level],"\n\r          ");
          	strcat(spell_list[level],buf);
	    }
	}
    }
 
    /* return results */
 
    if (!found)
    {
      	send_to_char("No spells found.\n\r",ch);
      	return;
    }

    buffer[0] = '\0';
    for (level = 0; level < LEVEL_HERO + 1; level++)
      	if (spell_list[level][0] != '\0')
	    strcat(buffer,spell_list[level]);
    strcat(buffer,"\n\r");
    page_to_char(buffer,ch);
}

void do_skills(CHAR_DATA *ch, char *argument)
{
    BUFFER *buffer;
    char arg[MAX_INPUT_LENGTH];
    char skill_list[LEVEL_HERO + 1][MAX_STRING_LENGTH];
    char skill_columns[LEVEL_HERO + 1];
    int sn, level, min_lev = 1, max_lev = LEVEL_HERO;
    bool found = FALSE;
    char buf[MAX_STRING_LENGTH];
 
    if (IS_NPC(ch))
      return;
    max_lev = (IS_IMMORTAL (ch)) ? ch->level + 20 : LEVEL_HERO;
    if (argument[0] != '\0')
    {
	argument = one_argument(argument,arg);
        if (!is_number(arg))
	{
		send_to_char("Arguments must be numerical or none.\n\r",ch);
		return;
	}
	max_lev = atoi(arg);

	if (max_lev < 1 || max_lev > ch->level)
	{
		sprintf(buf,"Levels must be between 1 and %d.\n\r",ch->level);
		send_to_char(buf,ch);
		return;
	}

	if (argument[0] != '\0')
	{
	    argument = one_argument(argument,arg);
	    if (!is_number(arg))
	    {
		    send_to_char("Arguments must be numerical or none.\n\r",ch);
		    return;
	    }
	    min_lev = max_lev;
	    max_lev = atoi(arg);

	    if (max_lev < 1 || max_lev > ch->level)
	    {
		    sprintf(buf,
			"Levels must be between 1 and %d.\n\r",ch->level);
		    send_to_char(buf,ch);
		    return;
	    }

	    if (min_lev > max_lev)
	    {
		    send_to_char("That would be silly.\n\r",ch);
		    return;
	    }
	}
    }


    /* initialize data */
    for (level = 0; level < LEVEL_HERO + 1; level++)
    {
        skill_columns[level] = 0;
        skill_list[level][0] = '\0';
    }
 
    for (sn = 0; sn < MAX_SKILL; sn++)
    {
        if (skill_table[sn].name == NULL )
	    break;

	if ((level = skill_table[sn].skill_level[ch->class]) < LEVEL_HERO + 1
	&&  level >= min_lev && level <= max_lev
	&&  skill_table[sn].spell_fun == spell_null
	&&  ch->pcdata->learned[sn] > 0)
        {
	    found = TRUE;
	    level = skill_table[sn].skill_level[ch->class];
	    if (ch->level < level)
	    	sprintf(buf,"%-18s n/a      ", skill_table[sn].name);
	    else
	    	sprintf(buf,"%-18s %3d%%      ",skill_table[sn].name,
		    ch->pcdata->learned[sn]);
 
	    if (skill_list[level][0] == '\0')
          	sprintf(skill_list[level],"\n\rLevel %2d: %s",level,buf);
	    else /* append */
	    {
          	if ( ++skill_columns[level] % 2 == 0)
		    strcat(skill_list[level],"\n\r          ");
          	strcat(skill_list[level],buf);
	    }
	}
    }
 
    /* return results */
 
    if (!found)
    {
      	send_to_char("No skills found.\n\r",ch);
      	return;
    }

    buffer = new_buf();
    for (level = 0; level < LEVEL_HERO + 1; level++)
      	if (skill_list[level][0] != '\0')
	    add_buf(buffer,skill_list[level]);
    add_buf(buffer,"\n\r");
    page_to_char(buf_string(buffer),ch);
    free_buf(buffer);
}

/* checks for skill improvement */
void check_improve( CHAR_DATA *ch, int sn, bool success, int multiplier )
{
    int chance;
    char buf[100];

    if (IS_NPC(ch))
	return;

    if (ch->level < skill_table[sn].skill_level[ch->class]
    ||  skill_table[sn].rating[ch->class] == 0
    ||  ch->pcdata->learned[sn] == 0
    ||  ch->pcdata->learned[sn] == 100)
	return;  /* skill is not known */ 

    /* check to see if the character has a chance to learn */
    chance = 10 * int_app[get_curr_stat(ch,STAT_INT)].learn;
    chance /= (		multiplier
		*	skill_table[sn].rating[ch->class] 
		*	1);	/* stock used to be 4 */
    chance += ch->level * 1.4;	/* stock used to be 1 */

    if (number_range(1,850) > chance)	/* stock was 1000 */
	return;

    /* now that the character has a CHANCE to learn, see if they really have */	

    if (success)
    {
	chance = URANGE(5,100 - ch->pcdata->learned[sn], 95);
	if (number_percent() < chance)
	{
	    sprintf(buf,"You have become better at %s!\n\r",
		    skill_table[sn].name);
	    send_to_char(buf,ch);
	    ch->pcdata->learned[sn]++;
	    gain_exp(ch,2 * skill_table[sn].rating[ch->class]);
	}
    }

    else
    {
	chance = URANGE(5,ch->pcdata->learned[sn]/2,30);
	if (number_percent() < chance)
	{
	    sprintf(buf,
		"You learn from your mistakes, and your %s skill improves.\n\r",
		skill_table[sn].name);
	    send_to_char(buf,ch);
	    ch->pcdata->learned[sn] += number_range(1,3);
	    ch->pcdata->learned[sn] = UMIN(ch->pcdata->learned[sn],100);
	    gain_exp(ch,2 * skill_table[sn].rating[ch->class]);
	}
    }
}

/*
 * base_exp_per_level
 *
 * Returns starting experience gain needed to level.  Replaced
 * mostly by 'exp_gain_level' which calc's varying gain needed
 * at higher levels.
 */
int base_exp_per_level(CHAR_DATA *ch)
{
    if (IS_NPC(ch))
	return 1000; 

    return 1000 * pc_race_table[ch->race].class_mult[ch->class]/100;
}

/* table based on factor =scale ^ (1/48) */
static double level_factors[] = {
    1.000000, /* illegal */
    1.000000, /* 1 */
    1.014545, /* 2 */
    1.023152, /* 3 */
    1.029302, /* 4 */
    1.034098, /* 5 */
    1.038034, /* 6 */
    1.041373, /* 7 */
    1.044274, /* 8 */
    1.046839, /* 9 */
    1.049140, /* 10 */
    1.051225, /* 11 */
    1.053132, /* 12 */
    1.054890, /* 13 */
    1.056520, /* 14 */
    1.058040, /* 15 */
    1.059463, /* 16 */
    1.060802, /* 17 */
    1.062066, /* 18 */
    1.063263, /* 19 */
    1.064400, /* 20 */
    1.065482, /* 21 */
    1.066515, /* 22 */
    1.067504, /* 23 */
    1.068450, /* 24 */
    1.069360, /* 25 */
    1.070234, /* 26 */
    1.071075, /* 27 */
    1.071887, /* 28 */
    1.072671, /* 29 */
    1.073429, /* 30 */
};

/* lookup table calc'd to figure level exp deltas */
static double level_delta_tab[MAX_LEVEL];

/*
 * init_level_spacing
 *
 * Calculates lookup table for converting base exp per level into
 * actual exp delta between two levels.  'scale' determines the
 * max spacing between levels, e.g. scale == 6 means at level 48
 * or so, the exp gain you need is 6 times your base.  A logarithmic
 * increase leads gradually up to that spacing.  scale == 1 sets
 * uniform level spacing.
 */
static
void init_level_spacing( int scale )
{
    int i;
    double factor, delta;
    static char initd;

    if(initd) return;
    if( scale < 1 || scale >= sizeof(level_factors) / sizeof(double)) {
	fprintf(stderr, "init_level_spacing: illegal scale %d\n", scale );
	exit(1);
    }
    factor = level_factors[scale];
    for( i = 0, delta = 1.0; i < MAX_LEVEL; i++)
    {
	level_delta_tab[i] = delta;
	delta *= factor;
    }
    initd = 1;
}

/*
 * exp_gain_level
 *
 * Calc's the exp changes needed to go up a level at a spec'd level.
 * If 'level' == 21, returns the exp gain needed to achieve level 22.
 */
int exp_gain_level( CHAR_DATA *ch, int level )
{
    int base;

    /* make sure we're initd */
    init_level_spacing( LEVEL_SCALE );

    /* get player's base xp */
    base = base_exp_per_level( ch );

    /* range checks */
    if(level < 1) level = 1;
    if(level >= MAX_LEVEL) level = MAX_LEVEL - 1;

    /* call local function to do actual work */
    return( exp_delta_level( base, level ) );
}

/*
 * exp_delta_level
 *
 * Used by 'exp_gain_level' and 'exp_at_level' to find spacing
 * up to next level.
 */
static
int exp_delta_level( int base, int level )
{
    int  xp, lxp, hxp, ret;

    /* calc raw xp */
    xp = (int) (level_delta_tab[level - 1] * base);

    /* now round to nearest 50 */
    lxp = xp - (xp % 50);
    hxp = lxp + 50;
    ret =  xp - lxp < hxp - xp ? lxp : hxp;
	
    return( ret );
}

/*
 * exp_at_level
 *
 * Calc's the exp needed to acquire the spec'd level using char's base
 * exp per level value.
 */
int exp_at_level( CHAR_DATA *ch, int level )
{
    int i, total, base;

    /* make sure we're initd */
    init_level_spacing( LEVEL_SCALE );

    if( level > MAX_LEVEL ) level = MAX_LEVEL - 1;
    base = base_exp_per_level( ch );

    for( i = total = 0; i < level; i++)
	total += exp_delta_level( base, i + 1 );
    return(total);
}

