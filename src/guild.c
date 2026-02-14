/* -----------------------------------------------------------------------
The following snippet was written by Gary McNickle (dharvest) for
Rom 2.4 specific MUDs and is released into the public domain. My thanks to
the originators of Diku, and Rom, as well as to all those others who have
released code for this mud base.  Goes to show that the freeware idea can
actually work. ;)  In any case, all I ask is that you credit this code
properly, and perhaps drop me a line letting me know it's being used.

from: gary@dharvest.com
website: http://www.dharvest.com
or http://www.dharvest.com/resource.html (rom related)

Send any comments, flames, bug-reports, suggestions, requests, etc... 
to the above email address.
----------------------------------------------------------------------- */

/***************************************************************************
*       ROM 2.4 is copyright 1993-1995 Russ Taylor			   *
*       ROM has been brought to you by the ROM consortium		   *
*           Russ Taylor (rtaylor@pacinfo.com)				   *
*           Gabrielle Taylor (gtaylor@pacinfo.com)			   *
*           Brian Moore (rom@rom.efn.org)				   *
*       By using this code, you have agreed to follow the terms of the     *
*       ROM license, in the file Rom24/doc/rom.license                     *
***************************************************************************/

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


/**  file   : guild.c
  *  author : Gary McNickle (gary@dharvest.com)
  *  purpose: Meant to provide a guild system configurable without 
  *           recompiling. This is done via an ASCII file called "guild.dat"  
  *           that you can edit or create with any text editor 
  *           (vi/pico/jed/jaret/etc).  All gulid/clan related commands from 
  *           ROM 2.4 have been moved to this file in an effort to keep 
  *           related code located all in one spot.
  */

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "recycle.h"

struct org_type org_table[MAX_ORG] = 
{
    { "None",
	"NONE",
	"Nothing",
	{"",	NULL},
	"",
	0, 0, 0
    },

    { "Thieves",
	"  THIEVES   ",
	"The Thieves Guild",
	{"pick lock", "envenom", "thieves cant", NULL },
	"Hiddukel",
	0, 0, 0
    },

    {	"Conclave",
	"  CONCLAVE  ",
	"The Conclave of Mages",
	{ "cloak of starlight", "change sex", "gift", NULL },
	"Lunitari",
	0, 0, 0
    },

    { "Solamnic",
	"  SOLAMNIC  ",	
	"The Solamnic Knighthood",
	{"charge", "pardon", "battlecry", "backup", "solamnic tongue",
         "pivot block", "riposte",  NULL },
	"Paladine",
	0, 0, 0
    },

    {
	"KoT",
	"    KOT     ",
	"The Knights of Takhisis",
	{"backup", "riposte", "pivot block", "battlecry", NULL },
	"Sargonnas",
	0, 0, 0
    },

    {
	"Holyorder",
	" HOLYORDER  ",
	"The Holy Order of the Stars",
	{"warding", "banish", "salvation", "truefire", NULL },
	"Majere",
	0, 0, 0
    },

    {
	"Mercenary",
	" MERCENARY  ",
	"The Mercenaries of Krynn",
	{"backup", "trophy", "track", NULL },
	"Hiddukel",
	0, 0, 0
    },

    {
	"Artisans",
	"  ARTISANS  ",
	"The Artisans of Krynn",
	{ "vortex", "calm", "sympathy", NULL },
	"Reorx",
	0, 0, 0
    },

    {
	"Forester",
	"  FORESTER  ",
	"The Forest People of Nature",
	{ "quiet movement", "riposte", "pivot block", "eagle eye", NULL },
	"Chislev",
	0, 0, 0
    },

    {
	"Aesthetic",
	" AESTHETIC  ",
	"The Order of the Book",
	{"truefire", "lore", "scrolls", "sympathy", "identify", "calm",
	 "minotaur tongue", "ogre tongue", "kender tongue", "elven tongue",
	 "dwarven tongue", NULL},
	"Gilean",
	0, 0, 0
    },

    {
        "Blackorder",
        " BLACKORDER ",
        "The Order of Evil",
	{"unholy armor", "gaseous form", "dark taint", "unholyrite", "drain blood", NULL},
        "Morgion",
        0, 0, 0
    },

    {
        "Undead",
        "   UNDEAD   ",
        "The Undead Armies",
	{"drain blood", "dark taint", "unholy armor", "animate corpse", "intensify death", "unholyrite",
	  NULL},
        "Chemosh",
        0, 0, 0
    },         

};

/* Power item vnum, Room of power item, pointer to power item,
  pointer to clan data, filename for clan_data */ 
struct clan_type clan_table[MAX_CLAN] =
{
	{ 0,0,NULL,NULL,"" 	   		},	/* no clan */
	{ 0,0,NULL,NULL,"../data/clan.thieves" 	},	/* Thieves */
	{ 0,0,NULL,NULL,"../data/clan.conclave"  	},	/* Conclave */
	{ 0,0,NULL,NULL,"../data/clan.solamnic"	},	/* Solamnic */
	{ 0,0,NULL,NULL,"../data/clan.kot"		},	/* KOT */
	{ 0,0,NULL,NULL,"../data/clan.holyorder" 	},	/* Holyorder */
	{ 0,0,NULL,NULL,"../data/clan.mercenary" 	},	/* Mercenary */
      { 0,0,NULL,NULL,"../data/clan.artisans"   },    /* Artisans */
      { 0,0,NULL,NULL,"../data/clan.forester"   },    /* Forester */
      { 0,0,NULL,NULL,"../data/clan.aestheticorder" },  /* AestheticOrder */
      { 0,0,NULL,NULL,"../data/clan.blackorder"	},    /* Blackorder */
      { 0,0,NULL,NULL,"../data/clan.undead"    	},    /* Undead */
};

char * const clan_skills[MAX_CLAN][11] =
{
        { NULL },								/* no clan */
        { "pick lock", "envenom", NULL },				/* Thieves */
        { NULL },								/* Conclave */
        { "backup", "track", NULL },				/* Solamnic */
        { "backup", NULL },						/* KOT */
        { NULL },								/* Holyorder */
        { "backup", NULL },						/* Mercenary */
        { NULL },								/* Artisans */
        { NULL },								/* Forester */
        { NULL },								/* AestheticOrder */
        { NULL },								/* Blackorder */
        { NULL },								/* Undead */
};


void load_orgs()
{
}

void load_clans()
{
}

/*
 * do_admit
 *
 * Bring someone into a organization.
 * Also available to mortals with org leader
 * credentials.
 *
 * Usage
 *   admit [immortal] [org]  (head god only)
 *   admit [mortal] [[leader]]
 */
void do_admit( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    char buf[100];
    CHAR_DATA *gch;
    int org = 0;
    int leader = 0;
    int head_god = FALSE;
    char *orgname;
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    if ( !IS_IMMORTAL(ch) && ch->org_id == ORG_NONE )
    {
    	 sprintf( buf, "You need to be a member of a clan to admit anyone!\n\r");
	 send_to_char( buf, ch );
         return;
    }

    /* check qualifications. must be leader or IMP */
    head_god = !str_cmp(ch->name, HEAD_GOD) || !str_cmp(ch->name,HEAD_GOD2) || !str_cmp(ch->name, HEAD_GOD3);
    if ( !IS_IMMORTAL(ch) && !( ch->org_id & ORG_LEADER ) )
    {
        send_to_char( "You are not qualified to induct.\n\r", ch );
        return;
    }
  
    /* make sure they spec'd an inductee */
    if( arg1[0] == 0 || arg2[0] == 0 )
    {
        send_to_char( "Usage: admit <char> <orgname> [ leader ]\n\r", ch );
        return;
    }
    /* see who we're trying to induct */
    if ( ( gch = get_char_world( ch, arg1 ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }
    /* make sure they're a player */
    if( IS_NPC(gch) )
    {
        send_to_char( "Only regular players can join clans.\n\r", ch );
        return;
    }

    /* get org name */
    for( org = 0, orgname = NULL; org < MAX_ORG; org++) {
        if( !str_cmp( arg2, org_table[org].name ) )
        {
            orgname = org_table[org].name;
            break;
        }
    }
    if( orgname == NULL )
    {
        send_to_char( "That is not a valid name for any organization.\n\r", ch );
        return;
    }
    /* see if they're making a leader */
    if( arg3[0] != 0 && !str_cmp( arg3, "leader" ) )
    {
        if( ch->level < LEVEL_AVATAR )
        {
            send_to_char( "Only immortals may induct leaders.\n\r", ch );
            return;
        }
        leader = ORG_LEADER;
    }
    /* see if they're an immort (only HEAD_GOD can do 'em) */
    if( gch->level > LEVEL_HERO )
    {
        if( !head_god )
        {
            send_to_char( "You are not qualified to induct an immortal.\n\r",
                 ch );
            return;
        }
    /* Imms not always leaders anymore */
    }
    /* see if they're trying to induct into org other than own */
    if( org != ORG( ch->org_id) && !head_god)
    {
        send_to_char( "You are not qualified to induct into other clans.\n\r",
            ch );
        return;
    }
        
    /* see if they're already in another org */
    if( gch->org_id != ORG_NONE )
    {
        if( (gch->level > LEVEL_HERO && !head_god) || ch->level < LEVEL_AVATAR)
        {
            send_to_char(
              "They are in another clan.  You are not qualified to\n\r", ch );
            send_to_char( "perform simultaneous eviction/induction.\n\r", ch );
            return;
        }
    }
    /* we're cleared do induction */
    /* if they're already in a org, inform of departure */
    if( gch->org_id != ORG_NONE )
    {
        sprintf( buf, "%s leaves the %s clan!\n\r", 
            gch->name,
            org_table[ORG(gch->org_id)].name);
        tell_org( buf, gch->org_id, NULL, NULL );
        send_to_char( buf, ch );
        admin_org_skills( gch, "lose" );
	  rem_name_clan(gch->name, gch->org_id, gch->level);
    }
    /* tell new org */
    gch->org_id = org | leader;
    arg1[0] = 0;
    if( gch->level >= LEVEL_AVATAR && leader)
        strcpy( arg1, " as your Immortal Leader" );
    else if( leader )
        strcpy( arg1, " as a Mortal Leader" );
    sprintf( buf, "%s inducts %s into the %s clan%s!\n\r",
        ch->name,
        gch->name, 
        org_table[ORG(org)].name, arg1 );
      tell_org(buf, org, NULL, NULL );
    admin_org_skills( gch, "gain" );
    add_name_clan(gch->name, gch->org_id, gch->level);
}

/*
 * do_evict
 *
 * Remove someone from a clan.  Also available to mortals with org leader
 * credentials.
 *
 * Usage
 *   evict [immortal]  (head god only)
 *   evict [mortal] 
 */
void do_evict( CHAR_DATA *ch, char *argument )
{
    char buf[100];
    CHAR_DATA *gch;
    int org = 0;
    int head_god = FALSE;

    if ( !IS_IMMORTAL(ch) && ch->org_id == ORG_NONE )
    {
    	 sprintf( buf, "You need to be a member of a clan to evict someone!\n\r");
	 send_to_char( buf, ch ); 
         return;
    }

    /* check qualifications. must be leader or IMP */
    head_god = !str_cmp(ch->name, HEAD_GOD) || !str_cmp(ch->name,HEAD_GOD2) || !str_cmp(ch->name, HEAD_GOD3);
    if ( !IS_IMMORTAL(ch) && !( ch->org_id & ORG_LEADER ) )
    {
        send_to_char( "You are not qualified to evict.\n\r", ch );
        return;
    }
  
    /* make sure they spec'd an inductee */
    if( argument[0] == 0)
    {
        send_to_char( "Evict who?\n\r", ch );
        return;
    }

    /* see who we're trying to evict */
    if ( ( gch = get_char_world( ch, argument ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    /* make sure they're a player */
    if( IS_NPC(gch) )
    {
        send_to_char( "You can only evict regular players.\n\r", ch );
        return;
    }

    /* make sure they're in a org */
    if( gch->org_id == ORG_NONE )
    {
        send_to_char( "They are not in any clan or form of government.\n\r", ch );
        return;
    }

    /* see if they're an immoral */
    if( gch->level >= LEVEL_AVATAR )
    {
        if( !head_god )
        {
            send_to_char( "You are not qualified to evict an immortal.\n\r",
                 ch );
            return;
        }
    }

    /* mortal eviction */
    else 
    {
        /* make sure they're in proper org */
        if( ORG(gch->org_id) != ORG( ch->org_id) && !head_god )
        {
            send_to_char( "You cannot possibly evict them.\n\r", ch );
            return;
        }
        /* check for leader eviction */
        if( gch->org_id & ORG_LEADER )
        {
            if( ch->level < LEVEL_AVATAR ) 
		{
                send_to_char( "Only immortals can evict leaders.\n\r", ch );
                return;
            }
        }
    }

    /* we're cleared to do eviction */
    /* evict and inform of departure */
    org = ORG( gch->org_id );
    sprintf( buf, "%s evicts you from the %s clan.\n\r", 
        ch->name,
        org_table[org].name );
    send_to_char( buf, gch );
    send_to_char("You will be able to temporarily use your clan skills.\n\r",gch);
    sprintf( buf, "%s evicts %s from the %s clan.\n\r", 
        ch->name,
        ch == gch ? "themself" : gch->name,
        org_table[org].name );
    admin_org_skills( gch, "lose" );
    rem_name_clan(gch->name, gch->org_id, gch->level);
    gch->org_id = ORG_NONE;
      tell_org( buf, org, NULL, NULL );
    /* make sure evictor gets notice */
    if( ORG( ch->org_id ) != org )
    {
        sprintf( buf, "You evict %s from the %s clan.\n\r",
            ch == gch ? "yourself" : gch->name,
            org_table[org].name );
        send_to_char( buf, ch );
    }
}

/*
 *    admin_org_skills
 *   used to "gain" for "lose" org skills.
 */
void admin_org_skills( CHAR_DATA *ch, char *action )
{
    int i,sn;
    char *skill;
    int org;
    char buf[100];
    org = ORG( ch->org_id );
    if( org == ORG_NONE )
    {
        bug("admin_org_skills: no org",0);
        return;
    }
    /* look up skills */
    for( i = 0; i < 10; i++ )
    {
        if( (skill = org_table[org].skills[i]) == NULL )
            break;
        if( (sn = skill_lookup( skill ) ) < 0 ) continue;
        if ( ch->level > skill_table[sn].skill_level[ch->class] 
	      || ch->pcdata->learned[sn] > 1 /* skill is known, do not overwrite % */)
		{
		if (!str_cmp(action, "lose"))
		continue;
                if (ch->pcdata->learned[sn] > 70)
		continue;
		}
		
        sprintf( buf, "You %s a skill: %s\n\r", action, skill );
        send_to_char( buf, ch );
        ch->pcdata->learned[sn] = strcmp(action,"gain") ? 0 : 70;
    }
}

void do_clist( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];

    argument = one_argument(argument, arg1);

    if ( IS_NPC( ch ) ) return;

    if ( IS_IMPLEMENTOR(ch) && arg1[0] == '\0')
    {
	send_to_char("What clan's info do you want to list?\n\r",ch);
	return;
    }

    /* if we are not an imp, show listing only for the clan we are in */
    if( !IS_IMPLEMENTOR(ch) && ch->org_id == ORG_NONE )
    {
	send_to_char("You have no organization to list info for.\n\r",ch);
	return;
    }

    /* non-imp defaults to own org. */
    if( !IS_IMPLEMENTOR(ch) )
    {
	sprintf(buf, "%s", org_table[ORG(ch->org_id)].name);
	one_argument(buf, arg1);
    }

    if ( !str_prefix( arg1, "thieves") )
    {
	if ( !IS_THIEVES(ch) && !IS_IMPLEMENTOR(ch) )
	{
	    send_to_char("You do not have the authority for that clan.\n\r",ch);
	    return;
	}
	show_clan(ch, CLAN_THIEVES);
    }
    else if ( !str_prefix( arg1, "conclave") )
    {
	if ( !IS_CONCLAVE(ch) && !IS_IMPLEMENTOR(ch) )
	{
	    send_to_char("You do not have the authority for that clan.\n\r",ch);
	    return;
	}
	show_clan(ch, CLAN_CONCLAVE);
    }
    else if ( !str_prefix( arg1, "solamnic") )
    {
	if ( !IS_SOLAMNIC(ch) && !IS_IMPLEMENTOR(ch) )
	{
	    send_to_char("You do not have the authority for that clan.\n\r",ch);
	    return;
	}
	show_clan(ch, CLAN_SOLAMNIC);
    }
    else if ( !str_prefix( arg1, "kot") )
    {
	if ( !IS_KOT(ch) && !IS_IMPLEMENTOR(ch) )
	{
	    send_to_char("You do not have the authority for that clan.\n\r",ch);
	    return;
	}
	show_clan(ch, CLAN_KOT);
    }
    else if ( !str_prefix( arg1, "holyorder") )
    {
	if ( !IS_HOLYORDER(ch) && !IS_IMPLEMENTOR(ch) )
	{
	    send_to_char("You do not have the authority for that clan.\n\r",ch);
	    return;
	}
	show_clan(ch, CLAN_HOLYORDER);
    }
    else if ( !str_prefix( arg1, "mercenary") )
    {
	if ( !IS_MERCENARY(ch) && !IS_IMPLEMENTOR(ch) )
	{
	    send_to_char("You do not have the authority for that clan.\n\r",ch);
	    return;
	}
	show_clan(ch, CLAN_MERCENARY);
    }
    else if ( !str_prefix( arg1, "artisans") )
    {
	if ( !IS_ARTISANS(ch) && !IS_IMPLEMENTOR(ch) )
	{
	    send_to_char("You do not have the authority for that clan.\n\r",ch);
	    return;
	}
	show_clan(ch, CLAN_ARTISANS);
    }
    else if ( !str_prefix( arg1, "forester") )
    {
	if ( !IS_FORESTER(ch) && !IS_IMPLEMENTOR(ch) )
	{
	    send_to_char("You do not have the authority for that clan.\n\r",ch);
	    return;
	}
	show_clan(ch, CLAN_FORESTER);
    }
    else if ( !str_prefix( arg1, "aesthetic") )
    {
	if ( !IS_AESTHETIC(ch) && !IS_IMPLEMENTOR(ch) )
	{
	    send_to_char("You do not have the authority for that clan.\n\r",ch);
	    return;
	}
	show_clan(ch, CLAN_AESTHETIC);
    }
    else if ( !str_prefix( arg1, "blackorder") )
    {
	if ( !IS_BLACKORDER(ch) && !IS_IMPLEMENTOR(ch) )
	{
	    send_to_char("You do not have the authority for that clan.\n\r",ch);
	    return;
	}
	show_clan(ch, CLAN_BLACKORDER);
    }
    else if ( !str_prefix( arg1, "undead") )
    {
	if ( !IS_UNDEAD(ch) && !IS_IMPLEMENTOR(ch) )
	{
	    send_to_char("You do not have the authority for that clan.\n\r",ch);
	    return;
	}
	show_clan(ch, CLAN_UNDEAD);
    }
    else
    {
	send_to_char("No such clan to list info for.\n\r",ch);
	return;
    }
}

/*
 * Flaging people for crimes....
 *
 * 
 */
void do_charge( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

   if ( !IS_NPC( ch ) && get_skill(ch,gsn_charge) < 1 )
    {
      send_to_char("You have no idea how to charge people for a crime.\n\r", ch);
      return;
    }

    if ( IS_NPC(ch) )
    {
        send_to_char( "You can't.\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
       send_to_char( "Syntax: charge <character> <murder|theft|assault|treason|outlaw>.\n\r", ch );
       send_to_char( "        charge <character> <check>\r\n", ch);
       return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
        send_to_char("They are not here.\n\r",ch);
        return;
    }

    if ( !str_cmp( arg2, "check" ) )
    {
        if ((victim = get_char_world( ch, arg1 ) ) == NULL )
	  {
	  send_to_char("You don't have any information about that person right now.\n\r", ch);
	  return;
	  }	
        if (IS_NPC(victim))
          {
          send_to_char( "No one keeps information about them.\n\r", ch );
          return;
          }
    	stat_sheet(ch, victim);
        return;
    }

    if ( IS_NPC(victim) )
    {
        send_to_char( "Can't charge non-players.\n\r", ch );
        return;
    }

    if (IS_IMMORTAL(victim) && (ch->level < victim->level))
    {
	send_to_char( "You don't have the power to flag that person.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "theft" ) )
    	{
        	if ( IS_SET(victim->act, PLR_THIEF) )
        	{
            		send_to_char( "They are already flagged for THEFT.\n\r", ch );
            		return;
        	}
        	SET_BIT( victim->act, PLR_THIEF );
		victim->pcdata->flags[0][0]++;
        	act( "$N is now a wanted {rTHIEF{x.\n\r", 
		ch, NULL, victim, TO_CHAR );
        	act_new( "You are now an accomplished and very most wanted {rTHIEF{x", 
            	victim, 0, 0, TO_CHAR, POS_DEAD);
		return;
	}

    if ( !str_cmp( arg2, "murder" ) )
    	{
        	if ( IS_SET(victim->act, PLR_KILLER) )
        	{
            		send_to_char( "They are already flagged for MURDER.\n\r", ch );
            		return;
        	}
        	SET_BIT( victim->act, PLR_KILLER );
		victim->pcdata->flags[0][1]++;
        	act( "$N is now a wanted {rMURDERER{x.\n\r", 
		ch, NULL, victim, TO_CHAR );
        	act_new( "You are now wanted for {rMURDER{x!", 
            	victim, 0, 0, TO_CHAR, POS_DEAD);
		return;
    	}

    if ( !str_cmp( arg2, "treason" ) )
    	{
           if ( IS_SET(victim->act, PLR_TREASON) )
	   {
            send_to_char( "They are already flagged as a TRAITOR.\n\r", ch );
            return;
	   }

	   if (victim->org_id != ORG_SOLAMNIC)
	   {
	   act("$N is not governed by the Solamnics, $E cannot be charged with treason.\n\r",ch,NULL,victim,TO_CHAR);
	   return;
	   }

        	SET_BIT( victim->act, PLR_TREASON );
		victim->pcdata->flags[0][2]++;
        	act( "$N is now wanted for {rTREASON{x.\n\r", 
		ch, NULL, victim, TO_CHAR );
        	act_new( "You have been accused and found guilty of {rTREASON{x!", 
            	victim, 0, 0, TO_CHAR, POS_DEAD);
		return;
	}

    if ( !str_cmp( arg2, "assault" ) )
    	{
        	if ( IS_SET(victim->act, PLR_ASSAULT) )
        	{
            		send_to_char( "They are already flagged for ASSAULT.\n\r", ch );
            		return;
        	}
        	SET_BIT( victim->act, PLR_ASSAULT );
		victim->pcdata->flags[0][3]++;
        	act( "$N is now wanted for {rASSAULT{x.\n\r", 
		ch, NULL, victim, TO_CHAR );
        	act_new( "You are now wanted for {rASSAULT{x upon an upstanding citizen!", 
            	victim, 0, 0, TO_CHAR, POS_DEAD);
		return;
	}

    if ( !str_cmp( arg2, "outlaw" ) )
    	{
        	if ( IS_SET(victim->act, PLR_OUTLAW) )
        	{
            		send_to_char( "They are already flagged as an OUTLAW.\n\r", ch );
            		return;
        	}
        	SET_BIT( victim->act, PLR_OUTLAW );
		victim->pcdata->flags[0][4]++;
        	act( "$N has been flagged an {rOUTLAW{x.\n\r", 
		ch, NULL, victim, TO_CHAR );
        	act_new( "You have been accused and found guilty of being an {rOUTLAW{x!", 
            	victim, 0, 0, TO_CHAR, POS_DEAD);
		return;
	}

       return;
}

void stat_sheet(CHAR_DATA *ch, CHAR_DATA *victim)
{
    char out[MSL];
    char buf[MSL];
    char tmp[MSL];
    char tmp2[MSL];
    int i;

    out[0] = '\0';
    sprintf(buf, "Criminal Record for %s, Citizen of %s\r\n", 
	victim->name, 
	hometown_table[victim->hometown].name); 
    strcat(out, buf);
    for (i=0 ; i < (strlen(buf)-2) ; i++)
        strcat(out, "-");
    strcat(out, "\r\n");

    if (IS_SET(victim->act, PLR_OUTLAW))
    {
	sprintf(buf, "%s is an OUTLAW in %s\r\n", victim->name, 
	    hometown_table[victim->hometown].name);
	strcat(out, buf);
    }

    sprintf(buf, "\r\nCurrently wanted for: ");
    tmp[0] = '\0';
    if (IS_SET(victim->act, PLR_KILLER))
    {
	sprintf(tmp2, "MURDER  by the Solamnic Knights\r\n");
	strcat(tmp, tmp2);
	for (i=0 ; i < (strlen(buf)-2) ; i++)
	    strcat(tmp, " ");
    }
    if (IS_SET(victim->act, PLR_THIEF))
    {
	sprintf(tmp2, "THEFT   by the Solamnic Knights\r\n");
	strcat(tmp, tmp2);
	for (i=0 ; i < (strlen(buf)-2) ; i++)
	    strcat(tmp, " ");
    }
    if (IS_SET(victim->act, PLR_TREASON))
    {
	sprintf(tmp2, "TREASON  by the Solamnic Knights\r\n");
	strcat(tmp, tmp2);
	for (i=0 ; i < (strlen(buf)-2) ; i++)
	    strcat(tmp, " ");
    }
    if (IS_SET(victim->act, PLR_ASSAULT))
    {
	sprintf(tmp2, "ASSAULT in %s\r\n", hometown_table[victim->hometown].name);
	strcat(tmp, tmp2);
	for (i=0 ; i < (strlen(buf)-2) ; i++)
	    strcat(tmp, " ");
    }

    if (tmp[0] == '\0')
	strcat(tmp, "Nothing.\r\n");

    strcat(out, buf);
    strcat(out, tmp);
    strcat(out, "\r\n");

    sprintf(buf, "By the Solamnic Knights, %s has been:\r\n", victim->name);
    strcat(out, buf);

    sprintf(buf, "   Flagged for THEFT   %3d time(s), pardoned %3d time(s).\r\n", victim->pcdata->flags[0][0], victim->pcdata->pardons[0][0]);
    strcat(out, buf);

    sprintf(buf, "   Flagged for MURDER  %3d time(s), pardoned %3d time(s).\r\n", victim->pcdata->flags[0][1], victim->pcdata->pardons[0][1]);
    strcat(out, buf);

    sprintf(buf, "   Flagged for TREASON %3d time(s), pardoned %3d time(s).\r\n", victim->pcdata->flags[0][2], victim->pcdata->pardons[0][2]);
    strcat(out, buf);
    
    sprintf(buf, "   Flagged for ASSAULT %3d time(s), pardoned %3d time(s).\r\n", victim->pcdata->flags[0][3], victim->pcdata->pardons[0][3]);
    strcat(out, buf);
    
    strcat(out, "\r\n");
    page_to_char(out, ch);
    return;
}

int org_lookup(const char *name)
{
    int org;

    for (org = 0; org < MAX_ORG; org++)
    {
	if (!str_prefix(name, org_table[org].name))
	    return org;
    }

    return 0;
}

void do_unholyrite (CHAR_DATA *ch, char *argument)
{
  OBJ_DATA *blood;

  if (IS_NPC(ch))
    return;

  if (!IS_NPC(ch)
  &&  get_skill(ch,gsn_unholyrite)  <  1 )
  {
    send_to_char("The God of Death declines your sacrifice.\n\r",ch);
    return;
  }

  if (IS_AFFECTED(ch,AFF_CALM))
  {
    send_to_char("You're feeling too mellow to make the rite.\n\r",ch);
    return;
  }

  if (IS_AFFECTED2(ch,AFF_RAINBOW_PATTERN))
  {
    send_to_char("You're too fascinated with the rainbow in the sky to make the rite.\n\r",ch);
    return;
  }

  blood = get_eq_char(ch,WEAR_HOLD);

  if (!IS_IMMORTAL(ch) 
  && (blood == NULL
  ||  blood->pIndexData->vnum != OBJ_VNUM_JAR ))
  {
    send_to_char("You need to be holding a jar of blood.\n\r",ch);
    return;
  }

  if (blood != NULL && blood->pIndexData->vnum == OBJ_VNUM_JAR )
  {
    act("You pour the jar of blood onto the floor.",ch,blood,NULL,TO_CHAR);
    extract_obj(blood);
  }

  if (ch->mana < 300)
  {
    send_to_char("You don't have the mystical energies to make the rite.\n\r",ch);
    return;
  }

  if (ch->hit < ch->max_hit)
  {
    ch->hit += number_range(500,1000);
    send_to_char("You make a blood sacrifice to the God of Death.\n\r",ch);
    check_improve(ch,gsn_unholyrite,TRUE,1);
  }

  if (ch->hit >= ch->max_hit)
  {
    ch->hit=ch->max_hit;
    send_to_char("Your already in good health, your sacrifice was a waste.\n\r",ch);
    check_improve(ch,gsn_unholyrite,FALSE,1);
  }

  ch->mana -= 300;
  WAIT_STATE(ch,18);
  return;
}

/* NEW ROSTER CRAP - Shaun Marquardt */
void add_name_clan( char *name, int clan, int level)
{
	FILE *fp;
	char buf[MAX_STRING_LENGTH];
	char name2[80];
	int clan_id;
	int c;
	bool NameOnRoster = FALSE;

	/* Init values */
	buf[0] = '\0';
	name2[0] = '\0';
	clan_id = CLAN(clan);
	fclose(fpReserve);

	if( (fp = fopen(clan_table[clan_id].file, "r")) == NULL)
	{
		perror(clan_table[clan_id].file);
		exit(1);
	}

	/* Check to see if they are on the roster. */
	while(!feof(fp))
	{
		c = getc(fp);
		if(c == EOF)
		{
			ungetc(c, fp);
			break;
		}
		ungetc(c, fp);

		fgets(name2, 80, fp);
		if(name2 == NULL) break;

		name2[strlen(name2)-1] = '\0';
		if( is_name(name, name2) )
		{
			NameOnRoster = TRUE;
			break;
		}
	}
	fclose(fp);

	if(NameOnRoster) /* If the name is on the roster, we don't do anything. */
	{
		fpReserve = fopen(NULL_FILE, "r");
		return;
	}

	/* Add them to the Roster. */
	fp = fopen(clan_table[clan_id].file, "a");

	if(level > LEVEL_HERO)
	{
		sprintf(buf, "%s\n", name);
	}
	else if(clan & CLAN_LEADER)
	{
		sprintf(buf, "%s ({MLEADER{x)\n", name);
	}
	else
	{
		sprintf(buf, "%s\n", name);
	}

	fprintf(fp, "%s", buf);
	fclose(fp);
	fpReserve = fopen(NULL_FILE, "r");
	return;
}

void rem_name_clan( char *name, int clan, int level)
{
	FILE *fp;
	char buf[MAX_STRING_LENGTH];
	char name2[80];
	int clan_id;
	int c;
	bool NameOnRoster = FALSE;

	/* Init values. */
	buf[0] = '\0';
	name2[0] = '\0';
	clan_id = CLAN(clan);
	fclose(fpReserve);

	if( (fp = fopen(clan_table[clan_id].file, "r")) == NULL)
	{
		perror(clan_table[clan_id].file);
		exit(1);
	}

	/* Check to see if they are on the roster. */
	while(!feof(fp))
	{
		c = getc(fp);
		if(c == EOF)
		{
			ungetc(c, fp);
			break;
		}
		ungetc(c, fp);

		fgets(name2, 80, fp);
		if(name2 == NULL) break;

		name2[strlen(name2)-1] = '\0';
		if( is_name(name, name2) ) /* If they are on the roster, skip their name, and do not put */
		{				   /* it into the new roster buffer. Continue reading roster. */
			NameOnRoster = TRUE;
			continue;
		}

		strcat(buf, name2);
		strcat(buf, "\n");
	}
	fclose(fp);

	if(!NameOnRoster)
	{
		fpReserve = fopen(NULL_FILE, "r");
		return;
	}

	if( (fp = fopen(clan_table[clan_id].file, "w")) == NULL)
	{
		perror(clan_table[clan_id].file);
		exit(1);
	}

	/* Save the new roster, that doesn't have their name. */
	fprintf(fp, "%s", buf);	
	fclose(fp);
	fpReserve = fopen(NULL_FILE, "r");
	return;
}

void show_clan(CHAR_DATA *ch, int clan_id)
{
	BUFFER *buffer;
	FILE *fp;
	char buf[MAX_STRING_LENGTH];
	char name2[80];
	int c, god, total_mortals;
	bool skip = FALSE;

	buf[0] = '\0';
	name2[0] = '\0';
	total_mortals = 0;
	clan_id = CLAN(clan_id);
	buffer = new_buf();
	fclose(fpReserve);

	if( (fp = fopen(clan_table[clan_id].file, "r")) == NULL)
	{
		perror(clan_table[clan_id].file);
		exit(1);
	}

	sprintf(buf, "Information for the %s clan.\n\r", org_table[clan_id].name);
	add_buf(buffer, buf);
	add_buf(buffer, "Immortals:\n\r");
	while(!feof(fp)) /* Immortals */
	{
		c = getc(fp);
		if(c == EOF)
		{
			ungetc(c, fp);
			break;
		}
		ungetc(c, fp);

		fgets(name2, 80, fp);
		if(name2 == NULL) break;

		name2[strlen(name2)-1] = '\0';
		for(god = 1; god < MAX_RELIGION-1; god++) /* Only put God names as listed in the RELIGION */
		{							/* table (Paladine, Hiddukel, etc) into the Imm */
			if( !str_cmp(religion_table[god].godname, name2) ) /* (continued) section */
			{
				sprintf(buf, "%s\n\r", name2);
				add_buf(buffer, buf);
			}
		}
	}
	/* Close file & Re-Open it, because after every read, we end up at eof. */
	fclose(fp);
	fp = fopen(clan_table[clan_id].file, "r");

	add_buf(buffer, "\n\rMortal Leaders:\n\r");
	while(!feof(fp)) /* Mortal Leaders */
	{
		skip = FALSE;
		c = getc(fp);
		if(c == EOF)
		{
			ungetc(c, fp);
			break;
		}
		ungetc(c, fp);

		fgets(name2, 80, fp);
		if(name2 == NULL) break;

		name2[strlen(name2)-1] = '\0';
		for(god = 1; god < MAX_RELIGION-1; god++) //skip gods in formatting
		{
			if( !str_cmp(religion_table[god].godname, name2) )
				skip = TRUE;

			if(skip)
				break;
		}

		if(skip)
		{
			skip = FALSE;
			continue;
		}

		if( !is_name("({MLEADER{x)", name2) ) /* Skip names who don't have the LEADER text. */
			continue;

		sprintf(buf, "%s\n\r", name2);
		add_buf(buffer, buf);
		total_mortals++;
	}
	/* Close file & Re-Open it, because after every read, we end up at eof. */
	fclose(fp);
	fp = fopen(clan_table[clan_id].file, "r");

	add_buf(buffer, "\n\rMortals:\n\r");
	while(!feof(fp)) /* Mortals */
	{
		skip = FALSE;
		c = getc(fp);
		if(c == EOF)
		{
			ungetc(c, fp);
			break;
		}
		ungetc(c, fp);

		fgets(name2, 80, fp);
		if(name2 == NULL) break;

		name2[strlen(name2)-1] = '\0';
		for(god = 1; god < MAX_RELIGION-1; god++) //skip gods in formatting
		{
			if( !str_cmp(religion_table[god].godname, name2) )
				skip = TRUE;

			if(skip)
				break;
		}

		if(skip)
		{
			skip = FALSE;
			continue;
		}

		if( is_name("({MLEADER{x)", name2) ) /* Skip names who DO have the LEADER text */
			continue;

		sprintf(buf, "%s\n\r", name2);
		add_buf(buffer, buf);
		total_mortals++;
	}
	fclose(fp);
	fpReserve = fopen(NULL_FILE, "r");

	sprintf(buf, "\n\rTotal Mortals: %d\n\r", total_mortals);
	add_buf(buffer, buf);
	page_to_char(buf_string(buffer), ch);
	return;
}
