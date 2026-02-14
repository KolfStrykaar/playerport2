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
 **************************************************************************/

/* Add changes here between the Diku and Rom credits: format should be
 * similar to the first comment: Initials of your name, month/year, and the
 * change(s) you made.  Use real name initials, not Immortal character
 */

/* CMW - 11/2013 - Changed all of the truemark speaches so that no matter what
 * happens, the victim is always the character calling upon the truemark.  It was
 * originally set with whom the character was fighting (if null, defaults to self).
 */


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
#include "lang.h"
#include "interp.h"
#include "recycle.h"
#include "tables.h"

#define PLINE   74              /* max line length for "pretty" lines */

extern void translate(CHAR_DATA * ch, CHAR_DATA * victim, char * message, char *output);
void    pretty_proc     args( ( char *buf, char *word ) );
void    mesg_garble     args( ( char *mesg ) );
char  * speech_impair   args( ( CHAR_DATA *ch, char *mesg ) );
void    sing_channel    args( ( CHAR_DATA *ch, char *argument ) );

/*
        pretty_proc
        add words wrapping lines as necessary.
        invoked with a NULL word ptr, it merely inits its counters.
*/
void pretty_proc( char *buf, char *word )
{
    static char *pbuf;
    static int index;
    int i;
    /* special cue to do inits */
    if( word == NULL)
    {
        pbuf = buf;
        index = 0;
        return;
    }
    /* forced newline */
    if( !str_cmp( "\n\r", word ) )
    {
        /* strip trailing spaces */
        for( i = strlen(pbuf) - 1; i >= 0 && pbuf[i] == ' '; i--)
            pbuf[i] = 0;
        strcat( pbuf, word );
        index = 0;
        return;
    }
    /* see if it's a soft space */
    if( !str_cmp( " ", word ) )
    {
        if(index == 0)
            return;
    }
    /* add a word */
    if( strlen(word) > PLINE )
        word[PLINE] = 0;
    if( strlen(word) + index > PLINE )
    {
        /* strip trailing spaces */
        for( i = strlen(pbuf) - 1; i >= 0 && pbuf[i] == ' '; i--)
            pbuf[i] = 0;
        strcat(pbuf,"\n\r");
        index = 0;
        while(*word == ' ')
            word++;
    }
    strcat( pbuf, word );
    index += strlen(word);
}

/* RT code to delete yourself */
void do_delet( CHAR_DATA *ch, char *argument)
{
    send_to_char("You must type the full command to delete yourself.\n\r",ch);
}

void do_delete( CHAR_DATA *ch, char *argument)
{
   DESCRIPTOR_DATA *d;
   char strsave[MAX_INPUT_LENGTH];
   int id;

   if (IS_NPC(ch))
	return;

   if (ch->pcdata->confirm_delete)
   {
	if (argument[0] != '\0')
	{
	    send_to_char("Delete status removed.\n\r",ch);
	    ch->pcdata->confirm_delete = FALSE;
	    return;
	}
	else
	{
	    /* Remove name from the Clan Roster, if we were in a clan. */
	    if(ch->org_id != ORG_NONE)
	    {
		rem_name_clan(ch->name, ch->org_id, ch->level);
	    }

    	    sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( ch->name ) );
	    wiznet("$N turns $Mself into line noise.",ch,NULL,0,0,0);
	    stop_fighting(ch,TRUE);
          save_char_obj( ch );
          id = ch->id;
          d = ch->desc;
          extract_char( ch, TRUE );
          if ( d != NULL )
          close_socket( d );
	    unlink(strsave);
	    return;
 	}
    }

    if (argument[0] != '\0')
    {
	send_to_char("Just type delete. No argument.\n\r",ch);
	return;
    }

    send_to_char("Type delete again to confirm this command.\n\r",ch);
    send_to_char("WARNING: this command is irreversible.\n\r",ch);
    send_to_char("Typing delete with an argument will undo delete status.\n\r",
	ch);
    ch->pcdata->confirm_delete = TRUE;
    wiznet("$N is contemplating deletion.",ch,NULL,0,0,get_trust(ch));
}	    

/* RT code to display channel status */

void do_channels( CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];

    /* lists all channels and their status */
    send_to_char("   channel     status\n\r",ch);
    send_to_char("---------------------\n\r",ch);

    if (IS_IMMORTAL(ch))
    {
      send_to_char("{igod channel{x    ",ch);
      if(!IS_SET(ch->comm,COMM_NOWIZ))
        send_to_char("ON\n\r",ch);
      else
        send_to_char("OFF\n\r",ch);
    }

    send_to_char("{ktells{x          ",ch);
    if (!IS_SET(ch->comm,COMM_DEAF))
	send_to_char("ON\n\r",ch);
    else
	send_to_char("OFF\n\r",ch);

    send_to_char("{tquiet mode{x     ",ch);
    if (IS_SET(ch->comm,COMM_QUIET))
      send_to_char("ON\n\r",ch);
    else
      send_to_char("OFF\n\r",ch);

    if (IS_SET(ch->comm,COMM_AFK))
	send_to_char("You are AFK.\n\r",ch);

    if (IS_SET(ch->comm,COMM_SNOOP_PROOF))
	send_to_char("You are immune to snooping.\n\r",ch);
   
    if (ch->lines != PAGELEN)
    {
	if (ch->lines)
	{
	    sprintf(buf,"You display %d lines of scroll.\n\r",ch->lines+2);
	    send_to_char(buf,ch);
 	}
	else
	    send_to_char("Scroll buffering is off.\n\r",ch);
    }

    if (ch->prompt != NULL)
    {
	sprintf(buf,"Your current prompt is: %s\n\r",ch->prompt);
	send_to_char(buf,ch);
    }

    if (IS_SET(ch->comm,COMM_NOSHOUT))
      send_to_char("You cannot shout.\n\r",ch);
  
    if (IS_SET(ch->comm,COMM_NOTELL))
      send_to_char("You cannot use tell.\n\r",ch);
 
    if (IS_SET(ch->comm,COMM_NOCHANNELS))
     send_to_char("You cannot use channels.\n\r",ch);

    if (IS_SET(ch->comm,COMM_NOEMOTE))
      send_to_char("You cannot show emotions.\n\r",ch);

}

/* RT deaf blocks out all shouts */

void do_deaf( CHAR_DATA *ch, char *argument)
{
    
   if (IS_SET(ch->comm,COMM_DEAF))
   {
     send_to_char("You can now hear tells again.\n\r",ch);
     REMOVE_BIT(ch->comm,COMM_DEAF);
   }
   else 
   {
     send_to_char("From now on, you won't hear tells.\n\r",ch);
     SET_BIT(ch->comm,COMM_DEAF);
   }
}

/* RT quiet blocks out all communication */

void do_quiet ( CHAR_DATA *ch, char * argument)
{
    if (IS_SET(ch->comm,COMM_QUIET))
    {
      send_to_char("Quiet mode removed.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_QUIET);
    }
   else
   {
     send_to_char("From now on, you will only hear says and emotes.\n\r",ch);
     SET_BIT(ch->comm,COMM_QUIET);
   }
}

/* afk command */

void do_afk ( CHAR_DATA *ch, char * argument)
{

    if ( ch->desc->original != NULL )
    {
	send_to_char( "You can not go into or out of AFK while switched.\n\r", ch );
	send_to_char( "Please RETURN to your original self to go into or out of AFK mode.\n\r", ch );
	return;
    }

    if (IS_SET(ch->comm,COMM_AFK))
    {
        send_to_char("AFK mode removed.\n\r",ch);
        REMOVE_BIT(ch->comm,COMM_AFK);
        if(buf_string(ch->pcdata->buffer)[0] != '\0')
        {
            send_to_char("You have missed tells, please type 'replay' to see them.\n\r",ch);
        }
        else
        {
            send_to_char("You have no missed tells.\n\r",ch);
        }
    }
   else
   {
     send_to_char("You are now in AFK mode.\n\r",ch);
     SET_BIT(ch->comm,COMM_AFK);
   }
}

void do_replay (CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
    {
	send_to_char("You can't replay.\n\r",ch);
	return;
    }

    if (buf_string(ch->pcdata->buffer)[0] == '\0')
    {
	send_to_char("You have no tells to replay.\n\r",ch);
	return;
    }

    page_to_char(buf_string(ch->pcdata->buffer),ch);
    clear_buf(ch->pcdata->buffer);
}

/* clan channels */
void do_clantalk( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char wiz_buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    int head_god = FALSE;

    head_god = !str_cmp(ch->name, HEAD_GOD) 
            || !str_cmp(ch->name,HEAD_GOD2) 
            || !str_cmp(ch->name, HEAD_GOD3);

    if ( ch->org_id == CLAN_NONE )
    {
	send_to_char("You aren't in a clan.\n\r",ch);
	return;
    }

    if ( argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,COMM_NOCLAN))
      {
        send_to_char("Clan channel is now ON\n\r",ch);
        REMOVE_BIT(ch->comm,COMM_NOCLAN);
      }

      else
      {
        send_to_char("Clan channel is now OFF\n\r",ch);
        SET_BIT(ch->comm,COMM_NOCLAN);
      }
      return;
    }

        if(!IS_NPC(ch) && IS_SET(ch->comm,COMM_NOCHANNELS))
        {
         send_to_char("The gods have revoked your channel privileges.\n\r",ch);
          return;
        }

        REMOVE_BIT(ch->comm,COMM_NOCLAN);

    if (((!str_infix("mud", argument))
    ||  (!str_infix("MUD", argument))
    ||  (!str_infix("MuD", argument))
    ||  (!str_infix("MUd", argument))
    ||  (!str_infix("mUD", argument))
    ||  (!str_infix("muD", argument))
    ||  (!str_infix("Mud", argument))
    ||  (!str_infix("mUd", argument))
    ||  (!str_suffix("mud", argument))
    ||  (!str_suffix("MUD", argument))
    ||  (!str_suffix("Mud", argument))
    ||  (!str_suffix("mud.", argument))
    ||  (!str_suffix("MUD.", argument))
    ||  (!str_suffix("Mud.", argument))
    ||  (!str_suffix("mud!", argument))
    ||  (!str_suffix("MUD!", argument))
    ||  (!str_suffix("Mud!", argument))
    ||  (!str_cmp("mud", argument))
    ||  (!str_cmp("mud,", argument))
    ||  (!str_cmp("MUD,", argument))
    ||  (!str_cmp("Mud,", argument))
    ||  (!str_cmp("mud.", argument))
    ||  (!str_cmp("MUD.", argument))
    ||  (!str_cmp("Mud.", argument))
    ||  (!str_cmp("mud!", argument))
    ||  (!str_cmp("MUD!", argument))
    ||  (!str_cmp("Mud!", argument))
    ||  (!str_prefix("Mud", argument))
    ||  (!str_prefix("MUD", argument))
    ||  (!str_prefix("mud", argument)))
    &&  !head_god)
    {
    sprintf( log_buf, "[WARNING] [%s] %s clans: %s",
	org_table[ORG(ch->org_id)].upper_name,
 	ch->name, argument);
    log_string( log_buf );
    wiznet(log_buf,NULL,NULL,WIZ_MULTI,WIZ_CLAN,0);
    }

    if (!IS_NPC(ch))
    {
    sprintf(wiz_buf, "[%s] %s clans: %s",
	org_table[ORG(ch->org_id)].upper_name,
	ch->name, argument );
    wiznet(wiz_buf,NULL,NULL,WIZ_CLAN,0,0);
    }

      sprintf( buf, "{CYou clan '%s'{x\n\r", argument );
      send_to_char( buf, ch );

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        if ( d->connected == CON_PLAYING
	  && SAME_ORG(ch,d->character) 
	  && !IS_SET(d->character->comm,COMM_NOCLAN)
	  && !IS_SET(d->character->comm,COMM_QUIET)
	  && d->character != ch)
        {
            sprintf( buf, "{C%s clans '%s'\n\r",
             can_see( d->character, ch ) ? ch->name : "someone", argument );
            send_to_char(buf, d->character);
        }
    }

    return;
}

void do_immtalk( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;
    int head_god = FALSE;

    head_god = !str_cmp(ch->name, HEAD_GOD) 
            || !str_cmp(ch->name,HEAD_GOD2) 
            || !str_cmp(ch->name, HEAD_GOD3);

    if ( argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,COMM_NOWIZ))
      {
	send_to_char("Immortal channel is now ON\n\r",ch);
	REMOVE_BIT(ch->comm,COMM_NOWIZ);
      }
      else
      {
	send_to_char("Immortal channel is now OFF\n\r",ch);
	SET_BIT(ch->comm,COMM_NOWIZ);
      } 
      return;
    }

    if ((!str_infix("o.o", argument))
    ||  (!str_suffix("o.o", argument))
    ||  (!str_cmp("o.o,", argument))
    ||  (!str_prefix("o.o", argument)))
    {
      send_to_char("What in the living FUCK is o.o ? Use some english language Fuckhead!\n\r",ch);
      return;
    }

    if ((!str_infix("o.0", argument))
    ||  (!str_suffix("o.0", argument))
    ||  (!str_cmp("o.0,", argument))
    ||  (!str_prefix("o.0", argument)))
    {
      send_to_char("What in the living FUCK is o.0 ? Use some english language Fuckhead!\n\r",ch);
      return;
    }

    if ((!str_infix("0.o", argument))
    ||  (!str_suffix("0.o", argument))
    ||  (!str_cmp("0.o,", argument))
    ||  (!str_prefix("0.o", argument)))
    {
      send_to_char("What in the living FUCK is 0.o ? Use some english language Fuckhead!\n\r",ch);
      return;
    }

    if ((!str_infix("0.0", argument))
    ||  (!str_suffix("0.0", argument))
    ||  (!str_cmp("0.0,", argument))
    ||  (!str_prefix("0.0", argument)))
    {
      send_to_char("What in the living FUCK is 0.0 ? Use some english language Fuckhead!\n\r",ch);
      return;
    }

    if ((!str_infix("o_o", argument))
    ||  (!str_suffix("o_o", argument))
    ||  (!str_cmp("o_o,", argument))
    ||  (!str_prefix("o_o", argument)))
    {
      send_to_char("What in the living FUCK is o_o ? Use some english language Fuckhead!\n\r",ch);
      return;
    }

    if ((!str_infix("o_0", argument))
    ||  (!str_suffix("o_0", argument))
    ||  (!str_cmp("o_0,", argument))
    ||  (!str_prefix("o_0", argument)))
    {
      send_to_char("What in the living FUCK is o_0 ? Use some english language Fuckhead!\n\r",ch);
      return;
    }

    if ((!str_infix("0_o", argument))
    ||  (!str_suffix("0_o", argument))
    ||  (!str_cmp("0_o,", argument))
    ||  (!str_prefix("0_o", argument)))
    {
      send_to_char("What in the living FUCK is 0_o ? Use some english language Fuckhead!\n\r",ch);
      return;
    }

    if ((!str_infix("0_0", argument))
    ||  (!str_suffix("0_0", argument))
    ||  (!str_cmp("0_0,", argument))
    ||  (!str_prefix("0_0", argument)))
    {
      send_to_char("What in the living FUCK is o_o ? Use some english language Fuckhead!\n\r",ch);
      return;
    }

    if (((!str_infix("mud", argument))
    ||  (!str_infix("MUD", argument))
    ||  (!str_infix("MuD", argument))
    ||  (!str_infix("MUd", argument))
    ||  (!str_infix("mUD", argument))
    ||  (!str_infix("muD", argument))
    ||  (!str_infix("Mud", argument))
    ||  (!str_infix("mUd", argument))
    ||  (!str_suffix("mud", argument))
    ||  (!str_suffix("MUD", argument))
    ||  (!str_suffix("Mud", argument))
    ||  (!str_suffix("mud.", argument))
    ||  (!str_suffix("MUD.", argument))
    ||  (!str_suffix("Mud.", argument))
    ||  (!str_suffix("mud!", argument))
    ||  (!str_suffix("MUD!", argument))
    ||  (!str_suffix("Mud!", argument))
    ||  (!str_cmp("mud", argument))
    ||  (!str_cmp("mud,", argument))
    ||  (!str_cmp("MUD,", argument))
    ||  (!str_cmp("Mud,", argument))
    ||  (!str_cmp("mud.", argument))
    ||  (!str_cmp("MUD.", argument))
    ||  (!str_cmp("Mud.", argument))
    ||  (!str_cmp("mud!", argument))
    ||  (!str_cmp("MUD!", argument))
    ||  (!str_cmp("Mud!", argument))
    ||  (!str_prefix("Mud", argument))
    ||  (!str_prefix("MUD", argument))
    ||  (!str_prefix("mud", argument)))
    &&  !head_god)
    {
    sprintf( log_buf, "[WARNING] %s imms: %s", ch->name, argument);
    log_string( log_buf );
    wiznet(log_buf,NULL,NULL,WIZ_MULTI,0,0);
    }

  REMOVE_BIT(ch->comm,COMM_NOWIZ);

  act_new("{i[{y$n{i]: $t{x",ch,argument,NULL,TO_CHAR,POS_DEAD);
  for ( d = descriptor_list; d != NULL; d = d->next )
  {
    if (d->connected == CON_PLAYING
    && IS_IMMORTAL(d->character)
    && IS_SET(d->character->comm,COMM_NOWIZ)
    && !IS_IMPLEMENTOR(ch))
      continue;

    if ( d->connected == CON_PLAYING
    && IS_IMMORTAL(d->character))
    {
	act_new("{i[{y$n{i]: $t{x",ch,argument,d->character,TO_VICT,POS_DEAD);
    }
  }
  return;
}

/* RT chat replaced with ROM gossip */
void do_hero( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    int head_god = FALSE;

    head_god = !str_cmp(ch->name, HEAD_GOD) 
            || !str_cmp(ch->name,HEAD_GOD2) 
            || !str_cmp(ch->name, HEAD_GOD3);
 
    if (argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,COMM_NOHERO))
      {
        send_to_char("HERO channel is now ON. Kick Ass!\n\r",ch);
        REMOVE_BIT(ch->comm,COMM_NOHERO);
      }
      else
      {
        send_to_char("HERO channel is now OFF. That Sucks!\n\r",ch);
        SET_BIT(ch->comm,COMM_NOHERO);
      }
    }
    else
    {
      if (IS_SET(ch->comm,COMM_QUIET))
      {
          send_to_char("You must turn off quiet mode first.\n\r",ch);
          return;
      }
 
      if(!IS_NPC(ch) && IS_SET(ch->comm,COMM_NOCHANNELS))
      {
          send_to_char("The gods have revoked your channel privileges.\n\r",ch);
          return;
 
      }

    if (((!str_infix("mud", argument))
    ||  (!str_infix("MUD", argument))
    ||  (!str_infix("MuD", argument))
    ||  (!str_infix("MUd", argument))
    ||  (!str_infix("mUD", argument))
    ||  (!str_infix("muD", argument))
    ||  (!str_infix("Mud", argument))
    ||  (!str_infix("mUd", argument))
    ||  (!str_suffix("mud", argument))
    ||  (!str_suffix("MUD", argument))
    ||  (!str_suffix("Mud", argument))
    ||  (!str_suffix("mud.", argument))
    ||  (!str_suffix("MUD.", argument))
    ||  (!str_suffix("Mud.", argument))
    ||  (!str_suffix("mud!", argument))
    ||  (!str_suffix("MUD!", argument))
    ||  (!str_suffix("Mud!", argument))
    ||  (!str_cmp("mud", argument))
    ||  (!str_cmp("mud,", argument))
    ||  (!str_cmp("MUD,", argument))
    ||  (!str_cmp("Mud,", argument))
    ||  (!str_cmp("mud.", argument))
    ||  (!str_cmp("MUD.", argument))
    ||  (!str_cmp("Mud.", argument))
    ||  (!str_cmp("mud!", argument))
    ||  (!str_cmp("MUD!", argument))
    ||  (!str_cmp("Mud!", argument))
    ||  (!str_prefix("Mud", argument))
    ||  (!str_prefix("MUD", argument))
    ||  (!str_prefix("mud", argument)))
    &&  !head_god)
    {
    sprintf( log_buf, "[WARNING] %s heros: %s", ch->name, argument);
    log_string( log_buf );
    wiznet(log_buf,NULL,NULL,WIZ_MULTI,0,0);
    }

      REMOVE_BIT(ch->comm,COMM_NOHERO);

      sprintf( buf, "{MYou HERO '{M%s{M'{x\n\r", argument );
      send_to_char( buf, ch );
      for ( d = descriptor_list; d != NULL; d = d->next )
      {
        CHAR_DATA *victim;
 
        victim = d->original ? d->original : d->character;
 
        if ( d->connected == CON_PLAYING
	  &&   d->character != ch
	  &&   !IS_SET(victim->comm,COMM_NOHERO)
	  &&   !IS_SET(victim->comm,COMM_QUIET)
	  && ( victim->level >= 101) )
           {
           if (!check_ignore(victim,ch))
           act_new( "{M$n HEROs '{M$t{M'{x",
		   ch,argument, d->character, TO_VICT,POS_SLEEPING );
           }
      }
    }
}

void do_say( CHAR_DATA *ch, char *argument )
{
#ifdef LANGUAGES
    CHAR_DATA *vict;
    char buf[MSL];
    int language = 0;
#endif
    int head_god = FALSE;
    char *mesg;

    head_god = !str_cmp(ch->name, HEAD_GOD) 
            || !str_cmp(ch->name,HEAD_GOD2) 
            || !str_cmp(ch->name, HEAD_GOD3);

    if (IS_AFFECTED2(ch, AFF_SILENCE) )
    {
        send_to_char("You have been silenced! Your tongue is no longer responsive!\n\r",ch);
        act("$n tries to speak, but $s tongue doesn't seem to be working properly.",ch,NULL,NULL,TO_ROOM);
        return;
    }

    if(!IS_NPC(ch) && IS_SET(ch->comm,COMM_NOCHANNELS) )
    {
	send_to_char("Your channel privileges have been revoked.\n\r",ch);
	return;
    }

    if (IS_AFFECTED(ch, AFF_SLEEP)
    ||  IS_AFFECTED2(ch,AFF_BLACKJACK))
    {
	send_to_char("You murmur in your sleep.\n\r",ch);
	act("$n murmurs in $s sleep.",ch,NULL,NULL,TO_ROOM);
	return;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char( "Say what?\n\r", ch );
	return;
    }
    if (((!str_infix("mud", argument))
    ||  (!str_infix("MUD", argument))
    ||  (!str_infix("MuD", argument))
    ||  (!str_infix("MUd", argument))
    ||  (!str_infix("mUD", argument))
    ||  (!str_infix("muD", argument))
    ||  (!str_infix("Mud", argument))
    ||  (!str_infix("mUd", argument))
    ||  (!str_suffix("mud", argument))
    ||  (!str_suffix("MUD", argument))
    ||  (!str_suffix("Mud", argument))
    ||  (!str_suffix("mud.", argument))
    ||  (!str_suffix("MUD.", argument))
    ||  (!str_suffix("Mud.", argument))
    ||  (!str_suffix("mud!", argument))
    ||  (!str_suffix("MUD!", argument))
    ||  (!str_suffix("Mud!", argument))
    ||  (!str_cmp("mud", argument))
    ||  (!str_cmp("mud,", argument))
    ||  (!str_cmp("MUD,", argument))
    ||  (!str_cmp("Mud,", argument))
    ||  (!str_cmp("mud.", argument))
    ||  (!str_cmp("MUD.", argument))
    ||  (!str_cmp("Mud.", argument))
    ||  (!str_cmp("mud!", argument))
    ||  (!str_cmp("MUD!", argument))
    ||  (!str_cmp("Mud!", argument))
    ||  (!str_prefix("Mud", argument))
    ||  (!str_prefix("MUD", argument))
    ||  (!str_prefix("mud", argument)))
    &&  !head_god)
    {
    sprintf( log_buf, "[WARNING] %s says: %s", ch->name, argument);
    log_string( log_buf );
    wiznet(log_buf,NULL,NULL,WIZ_MULTI,0,0);
    }

    if (IS_AFFECTED(ch, AFF_HIDE))   
    {
        REMOVE_BIT( ch->affected_by, AFF_HIDE );
        act("You step out of the shadows.", ch, NULL, NULL, TO_CHAR);
        act("$n steps out of the shadows.", ch, NULL, NULL, TO_ROOM);
    }

    /* check for speech impairment */
    argument = speech_impair( ch, argument );
#ifdef LANGUAGES
    mesg = language_check( ch, argument, NULL );
    if ((language = lang_skill(ch, NULL)) != 0)
	sprintf(buf, "You say in %s {w'$T'{x",capitalize(lang_lookup(ch->language)));
    else
	sprintf(buf, "You say {w'$T'{x");
    act( buf, ch, NULL, mesg, TO_CHAR );

    for ( vict = ch->in_room->people ; vict != NULL; vict = vict->next_in_room)
    {
	if( vict == ch )
	    continue;

    if (IS_AFFECTED2(vict, AFF_DEAFNESS))
    {
	act("$n moves $s lips, but you can't understand what $e's saying.", ch, NULL, vict, TO_VICT);
	continue;
    }

	if (lang_skill(ch, vict))
	 sprintf(buf, "$n %s in %s {w'$t'{x", race_table[ch->race].say, capitalize(lang_lookup(ch->language)));
	else	
	if (ch->race == vict->race)
	    sprintf(buf, "$n %s {w'$t'{x", race_table[ch->race].say);
	else
	    sprintf(buf, "$n %s {w'$t'{x", race_table[ch->race].say);
        act( buf, ch, language_check(ch, mesg, vict), vict, TO_VICT);
	
    }
#else
    act( "$n says {w'$T'{x", ch, NULL, argument, TO_ROOM );
    act( "You say {w'$T'{x", ch, NULL, argument, TO_CHAR );
    mesg = argument;
#endif
    if ( !IS_NPC(ch) )
    {
	CHAR_DATA *mob, *mob_next;
	OBJ_DATA *obj, *obj_next;
	for ( mob = ch->in_room->people; mob != NULL; mob = mob_next )
	{
	    mob_next = mob->next_in_room;
	    if ( IS_NPC(mob) && HAS_TRIGGER_MOB( mob, TRIG_SPEECH )
	    &&   mob->position == mob->pIndexData->default_pos )
		p_act_trigger( argument, mob, NULL, NULL, ch, NULL, NULL, TRIG_SPEECH );
	    for ( obj = mob->carrying; obj; obj = obj_next )
	    {
		obj_next = obj->next_content;
		if ( HAS_TRIGGER_OBJ( obj, TRIG_SPEECH ) )
		    p_act_trigger( argument, NULL, obj, NULL, ch, NULL, NULL, TRIG_SPEECH );
	    }

     /* Speech Prog that will allow people with the appropriate mark, to be able
      * call upon their God, using the appropriate defined speach here, to grant
      * them a sort of special power.
      */

      

          /* Gilean */
      if (!str_cmp("Enlighten me, Great God of Knowledge!", argument))
      {
        if (( obj = get_eq_char(ch, WEAR_MARK)) == NULL )
	    return;
    
        if ((obj != NULL)
        && (obj->pIndexData->vnum == 1237))
        {
          if ((ch->alignment > 50)
	  ||  (ch->alignment < -50))
	  {
	    send_to_char("You have strayed too far off the path of True Balance to gain Gilean's favor. He isn't very pleased with your progress at maintaining balance throughout Krynn.", ch);
	    send_to_char("", ch);
	    return;
	  }

          if (ch->mana < 21)
	  {
	    send_to_char("You do not have enough mana.\n\r",ch);
	    return;
	  }

          if (IS_AFFECTED3(ch, AFF_DIVINE_FAVOR))
	  {
	    send_to_char("You may not call upon your God again for Divine Favor yet.\n\r",ch);
	    return;
	  }

	  send_to_char("Your Eternal Light of Knowledge glows as it fills you with an inner light.\n\r",ch);
	  act("The Eternal Light of Knowledge upon $n's forearm begins to glow with an inner light.",
          ch, NULL, NULL, TO_ROOM);
	  obj_cast_spell(skill_lookup("enlightenment"),ch->level,ch,ch,NULL);
	  obj_cast_spell(skill_lookup("divine favor"),ch->level,ch,ch,NULL);
	  WAIT_STATE(ch,2*PULSE_VIOLENCE);
	  ch->mana -=21;
	  return;
        }
      }

             /* Mishakal */
      if (!str_cmp("Blessed Lady of the Heart, have mercy upon my soul and cast my body into the heavens!", argument))
      {
        if (( obj = get_eq_char(ch, WEAR_MARK)) == NULL )
	    return;
    
        if ((obj != NULL)
        && (obj->pIndexData->vnum == 1235))
        {

          if (ch->alignment < 850)
	    {
	      send_to_char("You have strayed too far off the path of Righteousness to gain Mishakal's favor. She isn't very pleased with your progress at spreading kindness and good deeds throughout Krynn.", ch);
	      send_to_char("", ch);
	      return;
	    }

          if (ch->mana < 21)
	    {
		send_to_char("You do not have enough mana.\n\r",ch);
		return;
	    }

          if (IS_AFFECTED3(ch, AFF_DIVINE_FAVOR))
	    {
		send_to_char("You may not call upon your Goddess again for Divine Favor yet.\n\r",ch);
		return;
	    }

	    send_to_char("Your shimmering Holy Symbol flashes brightly, casting a myriad of rainbow hues throughout the area. You feel her divine essence flow within you.\n\r",ch);
	    act("$n's shimmering Holy Symbol flashes brightly, casting a myriad of rainbow hues throughout the room.",ch, NULL, NULL, TO_ROOM);
	    obj_cast_spell(skill_lookup("bless"),ch->level,ch,ch,NULL);
	    obj_cast_spell(skill_lookup("sanctuary"),ch->level,ch,ch,NULL);
	    obj_cast_spell(skill_lookup("divine favor"),ch->level,ch,ch,NULL);
	    WAIT_STATE(ch,2*PULSE_VIOLENCE);
	    ch->mana -= 25;
	    return;
        }
      }

      /* Chislev */
      if (!str_cmp("Noble Goddess of Nature, I have need of swift travel!", argument))
      {
        CHAR_DATA *gch;
        CHAR_DATA *unicorn;
        int i;

        if (( obj = get_eq_char(ch, WEAR_MARK)) == NULL )
	    return;
    
        if ((obj != NULL)
        && (obj->pIndexData->vnum == 1251))
        {
          if ((ch->alignment < -50)
          ||  (ch->alignment >  50))
	  {
	    send_to_char("You do not have a perfect balance of alignment to call upon this noble steed.\n\r", ch);
	    send_to_char("", ch);
	    return;
	  }

          for (gch = char_list; gch != NULL; gch = gch->next)
          {
            if (IS_NPC(gch)
            && IS_AFFECTED(gch, AFF_CHARM)
            && gch->master == ch
            && gch->pIndexData->vnum == MOB_VNUM_UNICORN)
            {
              send_to_char("You cannot control two unicorns at once.\n\r",ch);
              return;
            }	
          }

          if (ch->mana < 100)
	  {
	    send_to_char("You do not have enough mana.\n\r",ch);
	    return;
	  }

          if (IS_AFFECTED3(ch, AFF_DIVINE_FAVOR))
	  {
	    send_to_char("You may not call upon your Goddess again for Divine Favor yet.\n\r",ch);
	    return;
	  }

	  send_to_char("You call upon Chislev to bring you a swift mount.\n\r",ch);
	  send_to_char("The Golden Branch of Nature transforms into a pure white unicorn.\n\r",ch);
	  act("$n calls upon the divine force of Chislev.", ch, NULL, NULL, TO_ROOM);
	  act("The Golden Branch of Nature upon $n glows with a blinding light and a pure white unicorn materializes.",ch,NULL,NULL,TO_ROOM);
	  unicorn = create_mobile(get_mob_index(MOB_VNUM_UNICORN));
	  if (unicorn == NULL)
	    act("Unicorn not found.",ch, NULL, NULL, TO_ROOM);

	  for (i=0; i < MAX_STATS; i++)
	  unicorn->perm_stat[i] = ch->perm_stat[i];
	  unicorn->max_hit=URANGE(ch->pcdata->perm_hit,2*ch->pcdata->perm_hit,2000);
  	  unicorn->hit = unicorn->max_hit;
  	  unicorn->level = ch->level;
	  for (i=0; i < 3; i++)
	  unicorn->armor[i] = interpolate(unicorn->level, 100, -100);
	  unicorn->armor[3] = interpolate(unicorn->level, 100, 0);
	  unicorn->master = ch;
	  unicorn->leader = ch;
	  char_to_room(unicorn,ch->in_room); 
	  WAIT_STATE(ch,2*PULSE_VIOLENCE);
	  ch->mana -=100;
	  obj_cast_spell(skill_lookup("divine favor"),ch->level,ch,ch,NULL);
	  return;
        }
      }

      /* Takhisis */
      if (!str_cmp("I have given myself over to Hatred, empower me to unleash my wrath!", argument))
      {
        if (( obj = get_eq_char(ch, WEAR_MARK)) == NULL )
	    return;
    
        if ((obj != NULL)
        && (obj->pIndexData->vnum == 1261))
        {
          if (ch->alignment > -850)
	  {
	    send_to_char("You have strayed too far off the path of Darkness to gain Takhisis's favor. She isn't very pleased with your progress at spreading evil throughout Krynn.\n\r", ch);
	    send_to_char("", ch);
	    return;
	  }

          if (ch->mana < 40)
	  {
	    send_to_char("You do not have enough mana.\n\r",ch);
	    return;
	  }

          if (IS_AFFECTED3(ch, AFF_DIVINE_FAVOR))
	  {
	    send_to_char("You may not call upon your Goddess again for Divine Favor yet.\n\r",ch);
	    return;
	  }

	  send_to_char("You are bathed in the firey breath of Hatred, your Scorch of Hatred begins to burn and torment your body with pain.\n\r",ch);
	  act("$n is engulfed in fire, $s Scorch of Hatred begins to burn and wrack $s body in painful torment.",ch, NULL, NULL, TO_ROOM);
	  obj_cast_spell(skill_lookup("protection good"),ch->level,ch,ch,NULL);
	  obj_cast_spell(skill_lookup("frenzy"),ch->level,ch,ch,NULL);
	  obj_cast_spell(skill_lookup("divine favor"),ch->level,ch,ch,NULL);
	  WAIT_STATE(ch,2*PULSE_VIOLENCE);
	  ch->mana -= 40;
	  return;
        }
      }

             /* Solinari */
      if (!str_cmp("Radiance of light, bathe me in your righteous magic!", argument))
      {
        if (( obj = get_eq_char(ch, WEAR_MARK)) == NULL )
	    return;
    
        if ((obj != NULL)
        && (obj->pIndexData->vnum == 1271))
        {

          if (ch->alignment < 850)
	    {
	      send_to_char("You have strayed too far off the path of Righteousness to gain Solinari's favor. He isn't very pleased with your progress at spreading kindness and good deeds throughout Krynn.\n\r", ch);
	      send_to_char("", ch);
	      return;
	    }

          if (ch->mana < 70)
	    {
		send_to_char("You do not have enough mana.\n\r",ch);
		return;
	    }

          if (IS_AFFECTED3(ch, AFF_DIVINE_FAVOR))
	    {
		send_to_char("You may not call upon your God again for Divine Favor yet.\n\r",ch);
		return;
	    }

	    send_to_char("For a brief moment, a brilliant white light eminates from your entire body.\n\r",ch);
	    act("$n is engulfed in a brilliant white light.",ch, NULL, NULL, TO_ROOM);
	    obj_cast_spell(skill_lookup("protection evil"),ch->level,ch,ch,NULL);
	    obj_cast_spell(skill_lookup("cure light"),ch->level,ch,ch,NULL);
	    obj_cast_spell(skill_lookup("divine favor"),ch->level,ch,ch,NULL);
	    WAIT_STATE(ch,2*PULSE_VIOLENCE);
	    ch->mana -= 70;
	    return;
        }
      }

             /* Nuitari */
      if (!str_cmp("Cloak of darkness, shelter me in your dark embrace!", argument))
      {
        if (( obj = get_eq_char(ch, WEAR_MARK)) == NULL )
	    return;
    
        if ((obj != NULL)
        && (obj->pIndexData->vnum == 1269))
        {

          if (ch->alignment > -850)
	    {
	      send_to_char("You have strayed too far off the path of Darkness to gain Nuitari's favor. He isn't very pleased with your progress at spreading evil throughout Krynn.\n\r", ch);
	      send_to_char("", ch);
	      return;
	    }

          if (ch->mana < 70)
	    {
		send_to_char("You do not have enough mana.\n\r",ch);
		return;
	    }

          if (IS_AFFECTED3(ch, AFF_DIVINE_FAVOR))
	    {
		send_to_char("You may not call upon your God again for Divine Favor yet.\n\r",ch);
		return;
	    }

	    send_to_char("A darkness radiates out from the Shadow of Black Magic burned on your chest, engulfing your entire body in it's evil mist, before dissipating back into your chest.\n\r",ch);
	    act("A darkness radiates out from the Shadow of Black Magic burned on $n's chest, engulfing $s entire body in it's evil mist, before dissipating back into $s chest.",ch, NULL, NULL, TO_ROOM);
	    obj_cast_spell(skill_lookup("protection good"),ch->level,ch,ch,NULL);
	    obj_cast_spell(skill_lookup("intensify death"),ch->level,ch,ch,NULL);
	    obj_cast_spell(skill_lookup("divine favor"),ch->level,ch,ch,NULL);
	    WAIT_STATE(ch,2*PULSE_VIOLENCE);
	    ch->mana -= 70;
	    return;
        }
      }

             /* Lunitari */
      if (!str_cmp("May the crimson rays of Lunitari empower my magic!", argument))
      {
        if (( obj = get_eq_char(ch, WEAR_MARK)) == NULL )
	    return;
    
        if ((obj != NULL)
        && (obj->pIndexData->vnum == 1267))
        {

          if ((ch->alignment > 50)
	    ||  (ch->alignment < -50))
	    {
	      send_to_char("You have strayed too far off the path of True Balance to gain Lunitari's favor. She isn't very pleased with your progress at maintaining balance throughout Krynn.\n\r", ch);
	      send_to_char("", ch);
	      return;
	    }

          if (ch->mana < 70)
	    {
		send_to_char("You do not have enough mana.\n\r",ch);
		return;
	    }

          if (IS_AFFECTED3(ch, AFF_DIVINE_FAVOR))
	    {
		send_to_char("You may not call upon your Goddess again for Divine Favor yet.\n\r",ch);
		return;
	    }

	    send_to_char("Crimson energy radiates out from the Essence of Red Magic burned on your chest, engulfing your entire body in it's brilliant mist, before dissipating back into your chest.\n\r",ch);
	    act("A crimson energy radiates out from the Essence of Red Magic burned on $n's chest, engulfing $s entire body in it's brilliant mist, before dissipating back into $s chest.",ch, NULL, NULL, TO_ROOM);
	    obj_cast_spell(skill_lookup("shifting shadows"),ch->level,ch,ch,NULL);
	    obj_cast_spell(skill_lookup("mirror"),ch->level,ch,ch,NULL);
	    obj_cast_spell(skill_lookup("divine favor"),ch->level,ch,ch,NULL);
	    WAIT_STATE(ch,2*PULSE_VIOLENCE);
	    ch->mana -= 70;
	    return;
        }
      }

             /* Majere */
      if (!str_cmp("Master of the mind, guide me with your disciplines!", argument))
      {
        if (( obj = get_eq_char(ch, WEAR_MARK)) == NULL )
	    return;

        if ((obj != NULL)
        && (obj->pIndexData->vnum == 1245))
        {

          if (ch->alignment < 850)
	    {
	      send_to_char("You have strayed too far off the path of Righteousness to gain Majere's favor. He isn't very pleased with your progress at spreading kindness and good deeds throughout Krynn.\n\r", ch);
	      send_to_char("", ch);
	      return;
	    }

          if (ch->mana < 25)
	    {
		send_to_char("You do not have enough mana.\n\r",ch);
		return;
	    }

          if (IS_AFFECTED3(ch, AFF_DIVINE_FAVOR))
	    {
		send_to_char("You may not call upon your God again for Divine Favor yet.\n\r",ch);
		return;
	    }

	    send_to_char("The Pendant of Wisdom around your neck begins to glow a mysterious color.\n\r",ch);
	    act("The Pendant of Wisdom around $n's neck begins to glow a mysterious color before fading back to it's normal state.",ch, NULL, NULL, TO_ROOM);
	    obj_cast_spell(skill_lookup("clear thoughts"),ch->level,ch,ch,NULL);
	    obj_cast_spell(skill_lookup("divine favor"),ch->level,ch,ch,NULL);
	    WAIT_STATE(ch,2*PULSE_VIOLENCE);
	    ch->mana -= 25;
	    return;
        }
      }

             /* Branchala */
      if (!str_cmp("May the music of life, wash our sins away!", argument))
      {
        if (( obj = get_eq_char(ch, WEAR_MARK)) == NULL )
	    return;

        if ((obj != NULL)
        && (obj->pIndexData->vnum == 1247))
        {

          if (ch->alignment < 850)
	    {
	      send_to_char("You have strayed too far off the path of Righteousness to gain Branchala's favor. He isn't very pleased with your progress at spreading kindness and good deeds throughout Krynn.\n\r", ch);
	      send_to_char("", ch);
	      return;
	    }

          if (ch->mana < 35)
	    {
		send_to_char("You do not have enough mana.\n\r",ch);
		return;
	    }

          if (IS_AFFECTED3(ch, AFF_DIVINE_FAVOR))
	    {
		send_to_char("You may not call upon your God again for Divine Favor yet.\n\r",ch);
		return;
	    }

	    send_to_char("The Branch of Raving Beauty you have been marked with, begins to eminate a soft white glow.\n\r",ch);
	    act("$n's Branch of Raving Beauty begins to eminate a soft white light.",ch, NULL, NULL, TO_ROOM);
	    obj_cast_spell(skill_lookup("glamour"),ch->level,ch,ch,NULL);
	    obj_cast_spell(skill_lookup("empowerment"),ch->level,ch,ch,NULL);
	    obj_cast_spell(skill_lookup("divine favor"),ch->level,ch,ch,NULL);
	    WAIT_STATE(ch,2*PULSE_VIOLENCE);
	    ch->mana -= 35;
	    return;
        }
      }

             /* Chemosh */
      if (!str_cmp("Ruler of the Undead, spare your humble servant and grant me eternal life!", argument))
      {
        if (( obj = get_eq_char(ch, WEAR_MARK)) == NULL )
	    return;

        if ((obj != NULL)
        && (obj->pIndexData->vnum == 1233))
        {

          if (ch->alignment > -850)
	    {
	      send_to_char("You have strayed too far off the path of Darkness to gain Chemosh's favor. He isn't very pleased with your progress at spreading evil throughout Krynn.\n\r", ch);
	      send_to_char("", ch);
	      return;
	    }

          if (ch->mana < 50)
	    {
		send_to_char("You do not have enough mana.\n\r",ch);
		return;
	    }

          if (IS_AFFECTED3(ch, AFF_DIVINE_FAVOR))
	    {
		send_to_char("You may not call upon your God again for Divine Favor yet.\n\r",ch);
		return;
	    }

	    send_to_char("The Ashes of Death begin to give off a foul odor... the distinct smell of death and decaying flesh.\n\r",ch);
	    act("The air begins to fill with the distinct odor of death and decaying flesh.",ch, NULL, NULL, TO_ROOM);
	    obj_cast_spell(skill_lookup("repulsion"),ch->level,ch,ch,NULL);
	    obj_cast_spell(skill_lookup("intensify death"),ch->level,ch,ch,NULL);
	    obj_cast_spell(skill_lookup("divine favor"),ch->level,ch,ch,NULL);
	    WAIT_STATE(ch,2*PULSE_VIOLENCE);
	    ch->mana -= 50;
	    return;
        }
      }

             /* Paladine */
      if (!str_cmp("Father of Dragons, I repent and ask for forgiveness!", argument))
      {
        if (( obj = get_eq_char(ch, WEAR_MARK)) == NULL )
	    return;

        if ((obj != NULL)
        && (obj->pIndexData->vnum == 1239))
        {

          if (ch->alignment < 850)
	    {
	      send_to_char("You have strayed too far off the path of Righteousness to gain Paladine's favor. He isn't very pleased with your progress at spreading kindness and good deeds throughout Krynn.\n\r", ch);
	      send_to_char("", ch);
	      return;
	    }

          if (ch->mana < 50)
	    {
		send_to_char("You do not have enough mana.\n\r",ch);
		return;
	    }

          if (IS_AFFECTED3(ch, AFF_DIVINE_FAVOR))
	    {
		send_to_char("You may not call upon your God again for Divine Favor yet.\n\r",ch);
		return;
	    }

	    send_to_char("The Tattoo of a Platinum Dragon begins to glow, before an intense beam of golden light shoots out for a brief moment.\n\r",ch);
	    act("The Tattoo of a Platinum Dragon branded upon $n, begins to glow, before an intense beam of golden light shoots out for a brief moment.",ch, NULL, NULL, TO_ROOM);
	    obj_cast_spell(skill_lookup("dragon skin"),ch->level,ch,ch,NULL);
	    obj_cast_spell(skill_lookup("aid"),ch->level,ch,ch,NULL);
	    obj_cast_spell(skill_lookup("divine favor"),ch->level,ch,ch,NULL);
	    WAIT_STATE(ch,2*PULSE_VIOLENCE);
	    ch->mana -= 50;
	    return;
        }
      }

             /* Kiri */
      if (!str_cmp("In your name, may I be glorious in battle!", argument))
      {
        if (( obj = get_eq_char(ch, WEAR_MARK)) == NULL )
	    return;

        if ((obj != NULL)
        && (obj->pIndexData->vnum == 1257))
        {

          if (ch->alignment < 850)
	    {
	      send_to_char("You have strayed too far off the path of Righteousness to gain Kiri-Jolith's favor. He isn't very pleased with your progress at spreading kindness and good deeds throughout Krynn.\n\r", ch);
	      send_to_char("", ch);
	      return;
	    }

          if (ch->mana < 45)
	    {
		send_to_char("You do not have enough mana.\n\r",ch);
		return;
	    }

          if (IS_AFFECTED3(ch, AFF_DIVINE_FAVOR))
	    {
		send_to_char("You may not call upon your God again for Divine Favor yet.\n\r",ch);
		return;
	    }

	    send_to_char("Your Mark of Holy Justice begins to vibrate, then slowly starts to hum loudly.\n\r",ch);
	    act("$n's Mark of Holy Justice begins to vibrate, then slowly starts to hum, before once again going quiet and still.",ch, NULL, NULL, TO_ROOM);
	    obj_cast_spell(skill_lookup("chant of battle"),ch->level,ch,ch,NULL);
	    obj_cast_spell(skill_lookup("divine favor"),ch->level,ch,ch,NULL);
	    WAIT_STATE(ch,2*PULSE_VIOLENCE);
	    ch->mana -= 45;
	    return;
        }
      }

             /* Zeboim */
      if (!str_cmp("Mistress of the Sea, grant me safe passage!", argument))
      {
        if (( obj = get_eq_char(ch, WEAR_MARK)) == NULL )
	    return;

        if ((obj != NULL)
        && (obj->pIndexData->vnum == 1263))
        {

          if (ch->alignment > -850)
	    {
	      send_to_char("You have strayed too far off the path of Darkness to gain Zeboim's favor. She isn't very pleased with your progress at spreading evil throughout Krynn.\n\r", ch);
	      send_to_char("", ch);
	      return;
	    }

          if (ch->mana < 25)
	    {
		send_to_char("You do not have enough mana.\n\r",ch);
		return;
	    }

          if (IS_AFFECTED3(ch, AFF_DIVINE_FAVOR))
	    {
		send_to_char("You may not call upon your Goddess again for Divine Favor yet.\n\r",ch);
		return;
	    }

	    send_to_char("The sound of water crashing against a rocky shore thunders from within your mark, the Rage of a Storm.\n\r",ch);
	    act("The thundering sound of water crashing up against a rocky shore nearly deafens you as $n's Rage of a Storm begins to unleash it's power.",ch, NULL, NULL, TO_ROOM);
	    obj_cast_spell(skill_lookup("waterlungs"),ch->level,ch,ch,NULL);
	    obj_cast_spell(skill_lookup("divine favor"),ch->level,ch,ch,NULL);
	    WAIT_STATE(ch,2*PULSE_VIOLENCE);
	    ch->mana -= 25;
	    return;
        }
      }

             /* Hiddukel */
      if (!str_cmp("I shall betray my brothers in your name!", argument))
      {
        if (( obj = get_eq_char(ch, WEAR_MARK)) == NULL )
	    return;

        if ((obj != NULL)
        && (obj->pIndexData->vnum == 1231))
        {

          if (ch->alignment > -850)
	    {
	      send_to_char("You have strayed too far off the path of Darkness to gain Hiddukel's favor. He isn't very pleased with your progress at spreading evil throughout Krynn.\n\r", ch);
	      send_to_char("", ch);
	      return;
	    }

          if (ch->mana < 75)
	    {
		send_to_char("You do not have enough mana.\n\r",ch);
		return;
	    }

          if (IS_AFFECTED3(ch, AFF_DIVINE_FAVOR))
	    {
		send_to_char("You may not call upon your God again for Divine Favor yet.\n\r",ch);
		return;
	    }

	    send_to_char("The sound of laughter suddenly fills the air as your Earing of Corruption begins to unleash it's Unholy power.\n\r",ch);
	    act("The sound of laughter suddenly fills your ears as $n's Earing of Corruption begins to unleash it's Unholy power.",ch, NULL, NULL, TO_ROOM);
	    obj_cast_spell(skill_lookup("orb_of_containment"),ch->level,ch,ch,NULL);
	    obj_cast_spell(skill_lookup("divine favor"),ch->level,ch,ch,NULL);
	    WAIT_STATE(ch,2*PULSE_VIOLENCE);
          ch->gold += 1000;
	    ch->mana -= 75;
	    return;
        }
      }

             /* Shinare */
      if (!str_cmp("Grant me a prosperous journey!", argument))
      {
        if (( obj = get_eq_char(ch, WEAR_MARK)) == NULL )
	    return;

        if ((obj != NULL)
        && (obj->pIndexData->vnum == 1243))
        {

          if ((ch->alignment > 50)
	    ||  (ch->alignment < -50))
	    {
	      send_to_char("You have strayed too far off the path of True Balance to gain Shinare's favor. She isn't very pleased with your progress at maintaining balance throughout Krynn.\n\r", ch);
	      send_to_char("", ch);
	      return;
	    }

          if (ch->mana < 75)
	    {
		send_to_char("You do not have enough mana.\n\r",ch);
		return;
	    }

          if (IS_AFFECTED3(ch, AFF_DIVINE_FAVOR))
	    {
		send_to_char("You may not call upon your Goddess again for Divine Favor yet.\n\r",ch);
		return;
	    }

	    send_to_char("Your Earing of Prosperity begins to glow a soft, golden color.\n\r",ch);
	    act("$n's Earing of Prosperity begins to glow a soft, golden color.",ch, NULL, NULL, TO_ROOM);
	    obj_cast_spell(skill_lookup("luck"),ch->level,ch,ch,NULL);
	    obj_cast_spell(skill_lookup("divine favor"),ch->level,ch,ch,NULL);
	    WAIT_STATE(ch,2*PULSE_VIOLENCE);
          ch->gold += 1000;
	    ch->mana -= 75;
	    return;
        }
      }

             /* Sargonnas */
      if (!str_cmp("May your vengeance guide my blade to strike thy enemies dead!", argument))
      {
        if (( obj = get_eq_char(ch, WEAR_MARK)) == NULL )
	    return;

        if ((obj != NULL)
        && (obj->pIndexData->vnum == 1255))
        {

          if (ch->alignment > -850)
	    {
	      send_to_char("You have strayed too far off the path of Darkness to gain Sargonnas's favor. He isn't very pleased with your progress at spreading evil throughout Krynn.\n\r", ch);
	      send_to_char("", ch);
	      return;
	    }

          if (ch->mana < 80)
	    {
		send_to_char("You do not have enough mana.\n\r",ch);
		return;
	    }

          if (IS_AFFECTED3(ch, AFF_DIVINE_FAVOR))
	    {
		send_to_char("You may not call upon your God again for Divine Favor yet.\n\r",ch);
		return;
	    }

	    send_to_char("Your Embodiment of Destruction turns black, as it begins to unleash it's raw, Unholy power.\n\r",ch);
	    act("$n's Embodiment of Destruction turns black, as it begins to unleash it's raw, Unholy power.",ch, NULL, NULL, TO_ROOM);
	    obj_cast_spell(skill_lookup("unholy armor"),ch->level,ch,ch,NULL);
	    obj_cast_spell(skill_lookup("forgedeath"),ch->level,ch,ch,NULL);
	    obj_cast_spell(skill_lookup("divine favor"),ch->level,ch,ch,NULL);
	    WAIT_STATE(ch,2*PULSE_VIOLENCE);
	    ch->mana -= 80;
	    return;
        }
      }

             /* Reorx */
      if (!str_cmp("By Reorx's Hammer, I shall never give up!", argument))
      {
        if (( obj = get_eq_char(ch, WEAR_MARK)) == NULL )
	    return;

        if ((obj != NULL)
        && (obj->pIndexData->vnum == 1265))
        {

          if ((ch->alignment > 50)
	    ||  (ch->alignment < -50))
	    {
	      send_to_char("You have strayed too far off the path of True Balance to gain Reorx's favor. He isn't very pleased with your progress at maintaining balance throughout Krynn.\n\r", ch);
	      send_to_char("", ch);
	      return;
	    }

          if (ch->mana < 50)
	    {
		send_to_char("You do not have enough mana.\n\r",ch);
		return;
	    }

          if (IS_AFFECTED3(ch, AFF_DIVINE_FAVOR))
	    {
		send_to_char("You may not call upon your God again for Divine Favor yet.\n\r",ch);
		return;
	    }

	    send_to_char("Your Scar of Battle starts to bleed, like a fresh wound, as it begins to unleash it's power.\n\r",ch);
	    act("$n's Scar of Battle starts to bleed, like a fresh open wound, as it begins to unleash it's power.",ch, NULL, NULL, TO_ROOM);
	    obj_cast_spell(skill_lookup("haste"),ch->level,ch,ch,NULL);
	    obj_cast_spell(skill_lookup("giant strength"),ch->level,ch,ch,NULL);
	    obj_cast_spell(skill_lookup("divine favor"),ch->level,ch,ch,NULL);
	    WAIT_STATE(ch,2*PULSE_VIOLENCE);
	    ch->mana -= 50;
	    return;
        }
      }

             /* Sirrion */
      if (!str_cmp("Bathe me in your eternal fire Lord Sirrion!", argument))
      {
        if (( obj = get_eq_char(ch, WEAR_MARK)) == NULL )
	    return;

        if ((obj != NULL)
        && (obj->pIndexData->vnum == 1253))
        {

          if ((ch->alignment > 50)
	    ||  (ch->alignment < -50))
	    {
	      send_to_char("You have strayed too far off the path of True Balance to gain Sirrion's favor. He isn't very pleased with your progress at maintaining balance throughout Krynn.\n\r", ch);
	      send_to_char("", ch);
	      return;
	    }

          if (ch->mana < 90)
	    {
		send_to_char("You do not have enough mana.\n\r",ch);
		return;
	    }

          if (IS_AFFECTED3(ch, AFF_DIVINE_FAVOR))
	    {
		send_to_char("You may not call upon your God again for Divine Favor yet.\n\r",ch);
		return;
	    }

	    send_to_char("Your Searing Scorch of Flame bursts into a magical fire, engulfing your entire body for a brief moment before dissipating.\n\r",ch);
	    act("$n's Searing Scorch of Flame suddenly bursts into a magical fire, engulfing $s entire body for a brief moment before dissipating.",ch, NULL, NULL, TO_ROOM);
	    obj_cast_spell(skill_lookup("flaming shield"),ch->level,ch,ch,NULL);
	    obj_cast_spell(skill_lookup("truefire"),ch->level,ch,ch,NULL);
	    obj_cast_spell(skill_lookup("divine favor"),ch->level,ch,ch,NULL);
	    WAIT_STATE(ch,2*PULSE_VIOLENCE);
	    ch->mana -= 90;
	    return;
        }
      }

             /* Habbakuk */
      if (!str_cmp("May your passion for life grant me strength and courage!", argument))
      {
        if (( obj = get_eq_char(ch, WEAR_MARK)) == NULL )
	    return;

        if ((obj != NULL)
        && (obj->pIndexData->vnum == 1241))
        {

          if (ch->alignment < 850)
	    {
	      send_to_char("You have strayed too far off the path of Righteousness to gain Habbakuk's favor. He isn't very pleased with your progress at spreading kindness and good deeds throughout Krynn.\n\r", ch);
	      send_to_char("", ch);
	      return;
	    }

          if (ch->mana < 100)
	    {
		send_to_char("You do not have enough mana.\n\r",ch);
		return;
	    }

          if (IS_AFFECTED3(ch, AFF_DIVINE_FAVOR))
	    {
		send_to_char("You may not call upon your God again for Divine Favor yet.\n\r",ch);
		return;
	    }

	    send_to_char("Your Mark of Martial Valor begins to unleash it's Holy energies.\n\r",ch);
	    act("$n's Mark of Martial Valor begins to unleash it's Holy energies.",ch, NULL, NULL, TO_ROOM);
	    obj_cast_spell(skill_lookup("ethereal warrior"),ch->level,ch,ch,NULL);
	    obj_cast_spell(skill_lookup("divine favor"),ch->level,ch,ch,NULL);
	    WAIT_STATE(ch,2*PULSE_VIOLENCE);
	    ch->mana -= 100;
	    return;
        }
      }

             /* Morgion */
      if (!str_cmp("Give me strength to make those around me suffer with disease and plague!", argument))
      {
        if (( obj = get_eq_char(ch, WEAR_MARK)) == NULL )
	    return;

        if ((obj != NULL)
        && (obj->pIndexData->vnum == 1249))
        {

          if (ch->alignment > -850)
	    {
	      send_to_char("You have strayed too far off the path of Darkness to gain Morgion's favor. He isn't very pleased with your progress at spreading evil throughout Krynn.\n\r", ch);
	      send_to_char("", ch);
	      return;
	    }

          if (ch->mana < 65)
	    {
		send_to_char("You do not have enough mana.\n\r",ch);
		return;
	    }

          if (IS_AFFECTED3(ch, AFF_DIVINE_FAVOR))
	    {
		send_to_char("You may not call upon your God again for Divine Favor yet.\n\r",ch);
		return;
	    }

	    send_to_char("Your Boils of Disease begin to crackle and pop, giving off toxic vapors.\n\r",ch);
	    act("$n's Boils of Disease begin to crackle and pop, giving off toxic vapors.",ch, NULL, NULL, TO_ROOM);
	    obj_cast_spell(skill_lookup("cloak of evil"),ch->level,ch,ch,NULL);
	    obj_cast_spell(skill_lookup("endure disease"),ch->level,ch,ch,NULL);
	    obj_cast_spell(skill_lookup("divine favor"),ch->level,ch,ch,NULL);
	    WAIT_STATE(ch,2*PULSE_VIOLENCE);
	    ch->mana -= 65;
	    return;
        }
      }

             /* Zivilyn */
      if (!str_cmp("Master of insight, enlighten me with your great wisdom!", argument))
      {
        if (( obj = get_eq_char(ch, WEAR_MARK)) == NULL )
	    return;

        if ((obj != NULL)
        && (obj->pIndexData->vnum == 1259))
        {

          if ((ch->alignment > 50)
	    ||  (ch->alignment < -50))
	    {
	      send_to_char("You have strayed too far off the path of True Balance to gain Zivilyn's favor. He isn't very pleased with your progress at maintaining balance throughout Krynn.\n\r", ch);
	      send_to_char("", ch);
	      return;
	    }

          if (ch->mana < 25)
	    {
		send_to_char("You do not have enough mana.\n\r",ch);
		return;
	    }

          if (IS_AFFECTED3(ch, AFF_DIVINE_FAVOR))
	    {
		send_to_char("You may not call upon your God again for Divine Favor yet.\n\r",ch);
		return;
	    }

	    send_to_char("Your Golden Bracelet of Life begins to sooth you, radiating it's a powerful energy.\n\r",ch);
	    act("$n's Golden Bracelet of Life begins to radiate a soothing energy.",ch, NULL, NULL, TO_ROOM);
	    obj_cast_spell(skill_lookup("attune"),ch->level,ch,ch,NULL);
	    obj_cast_spell(skill_lookup("divine favor"),ch->level,ch,ch,NULL);
	    WAIT_STATE(ch,2*PULSE_VIOLENCE);
	    ch->mana -= 25;
	    return;
        }
      }

     /* Spec that changes a person's hometown when they say the appropriate
      * message. I'm feeling lazy on the checks, so I'm doing a simple VNUM
      * check on crossreferencing the area, since area name can't be checked
      * by it's string, without adding in more DEF's in merc.h and I said fuck
      * the dumb shit, let's just be lazy. VNUM range instead of JUST the actual
      * VNUM the Registrar is in, has been done just in case the ROOM gets moved
      * to a different VNUM. If outside of the appropriate range, then it won't
      * work at all. So, make sure if a builder changes the room for the office,
      * they use an appropriate VNUM within the RANGE of the actual hometown area.
      */
      if (mob->spec_fun == spec_lookup("spec_hometownchanger"))
      {
        if (!str_cmp("I wish to live in Palanthas", argument)
        ||  !str_cmp("I wish to live in Palanthas.", argument))
        {
               /* Palanthas */
          if ((mob->in_room->vnum <= 3384)
          &&  (mob->in_room->vnum >= 3000))
          {

            if (ch->hometown == 0)
            {
              send_to_char("\n\r",ch);
              do_say(mob,"But your already a citizen of Palanthas! But hey, if you wanna give me your loot anyways, I'll take it");
              send_to_char("\n\r",ch);
              interpret(mob, "rub");
              return;
            }

            if (ch->steel < 5000)
            {
              send_to_char("\n\r",ch);
              do_say(mob,"I'm sorry, but the cost to live here is 5,000 steel coins and you don't quite have enough!");
              return;
            }

            send_to_char("\n\r",ch);
            do_say(mob,"Hurray! You are now a prestige citizen of Palanthas! Welcome!");
            send_to_char("\n\r",ch);
            interpret(mob, "smile");
            ch->steel -= 5000;
            ch->hometown = 0;
            ch->recall_point = hometown_table[ch->hometown].recall;
            return;
          } 
	    else
          {
            send_to_char("\n\r",ch);
            do_say(mob,"Then you must go to Palanthas to plea your case, not here!");
            send_to_char("\n\r",ch);
            interpret(mob, "mutter");
            return;
          }
        }
        if (!str_cmp("I wish to live in Neraka", argument)
        ||  !str_cmp("I wish to live in Neraka.", argument))
        {
              /* Neraka and Neraka2 */
          if (((mob->in_room->vnum <= 11200)
          &&  (mob->in_room->vnum >= 11000))
          || ((mob->in_room->vnum <= 12300)
          &&  (mob->in_room->vnum >= 11901)))
          {

            if (ch->hometown == 5)
            {
              send_to_char("\n\r",ch);
              do_say(mob,"But your already a citizen of Neraka! But hey, if you wanna give me your loot anyways, I'll take it");
              send_to_char("\n\r",ch);
              interpret(mob, "rub");
              return;
            }

            if (ch->steel < 5000)
            {
              send_to_char("\n\r",ch);
              do_say(mob,"I'm sorry, but the cost to live here is 5,000 steel coins and you don't quite have enough!");
              return;
            }

            send_to_char("\n\r",ch);
            do_say(mob,"Hurray! You are now a prestige citizen of Neraka! Welcome!");
            send_to_char("\n\r",ch);
            interpret(mob, "smile");
            ch->steel -= 5000;
            ch->hometown = 5;
            ch->recall_point = hometown_table[ch->hometown].recall;
            return;
          } 
	    else
          {
            send_to_char("\n\r",ch);
            do_say(mob,"Then you must go to Neraka to plea your case, not here!");
            send_to_char("\n\r",ch);
            interpret(mob, "mutter");
            return;
          }
        }
        if (!str_cmp("I wish to live in Solace", argument)
        ||  !str_cmp("I wish to live in Solace.", argument))
        {
              /* Solace */
          if ((mob->in_room->vnum <= 2700)
          &&  (mob->in_room->vnum >= 2400))
          {

            if (ch->hometown == 2)
            {
              send_to_char("\n\r",ch);
              do_say(mob,"But your already a citizen of Solace! But hey, if you wanna give me your loot anyways, I'll take it");
              send_to_char("\n\r",ch);
              interpret(mob, "rub");
              return;
            }

            if (ch->steel < 5000)
            {
              send_to_char("\n\r",ch);
              do_say(mob,"I'm sorry, but the cost to live here is 5,000 steel coins and you don't quite have enough!");
              return;
            }

            send_to_char("\n\r",ch);
            do_say(mob,"Hurray! You are now a prestige citizen of Solace! Welcome!");
            send_to_char("\n\r",ch);
            interpret(mob, "smile");
            ch->steel -= 5000;
            ch->hometown = 2;
            ch->recall_point = hometown_table[ch->hometown].recall;
            return;
          } 
	    else
          {
            send_to_char("\n\r",ch);
            do_say(mob,"Then you must go to Solace to plea your case, not here!");
            send_to_char("\n\r",ch);
            interpret(mob, "mutter");
            return;
          }
        }
        if (!str_cmp("I wish to live in Solanthus", argument)
        ||  !str_cmp("I wish to live in Solanthus.", argument))
        {
              /* Solanthus */
          if ((mob->in_room->vnum <= 5799)
          &&  (mob->in_room->vnum >= 5600))
          {

            if (ch->hometown == 1)
            {
              send_to_char("\n\r",ch);
              do_say(mob,"But your already a citizen of Solanthus! But hey, if you wanna give me your loot anyways, I'll take it");
              send_to_char("\n\r",ch);
              interpret(mob, "rub");
              return;
            }

            if (ch->steel < 5000)
            {
              send_to_char("\n\r",ch);
              do_say(mob,"I'm sorry, but the cost to live here is 5,000 steel coins and you don't quite have enough!");
              return;
            }

            send_to_char("\n\r",ch);
            do_say(mob,"Hurray! You are now a prestige citizen of Solanthus! Welcome!");
            send_to_char("\n\r",ch);
            interpret(mob, "smile");
            ch->steel -= 5000;
            ch->hometown = 1;
            ch->recall_point = hometown_table[ch->hometown].recall;
            return;
          } 
	    else
          {
            send_to_char("\n\r",ch);
            do_say(mob,"Then you must go to Solanthus to plea your case, not here!");
            send_to_char("\n\r",ch);
            interpret(mob, "mutter");
            return;
          }
        }
        if (!str_cmp("I wish to live in Thorbardin", argument)
        ||  !str_cmp("I wish to live in Thorbardin.", argument))
        {
              /* Thorbardin */
          if (((mob->in_room->vnum <= 10799)
          &&  (mob->in_room->vnum >= 10600))
          || ((mob->in_room->vnum <= 16449)
          &&  (mob->in_room->vnum >= 16000)))
          {
            if (ch->hometown == 4)
            {
              send_to_char("\n\r",ch);
              do_say(mob,"But ye is already a citizen of Thorbardin! But hey, if ye wanna give me ye loot anyways, I'll take it");
              send_to_char("\n\r",ch);
              interpret(mob, "rub");
              return;
            }

            if (ch->steel < 5000)
            {
              send_to_char("\n\r",ch);
              do_say(mob,"I'm sorry, but the cost to live here is 5,000 steel coins and you don't quite have enough!");
              return;
            }

            send_to_char("\n\r",ch);
            do_say(mob,"Praise Reorx! Ye are now a prestige citizen of Thorbardin! Welcome!");
            send_to_char("\n\r",ch);
            interpret(mob, "smile");
            ch->steel -= 5000;
            ch->hometown = 4;
            ch->recall_point = hometown_table[ch->hometown].recall;
            return;
          } 
	    else
          {
            send_to_char("\n\r",ch);
            do_say(mob,"Then you must go to Thorbardin to plea your case, not here!");
            send_to_char("\n\r",ch);
            interpret(mob, "mutter");
            return;
          }
        }
        if (!str_cmp("I wish to live in Mt. Nevermind", argument)
        ||  !str_cmp("I wish to live in Mt. Nevermind.", argument))
        {
              /* Mt. Nevermind */
          if (((mob->in_room->vnum <= 1799)
          &&  (mob->in_room->vnum >= 1600))
          || ((mob->in_room->vnum <= 10999)
          &&  (mob->in_room->vnum >= 10800)))
          {
            if (ch->hometown == 7)
            {
              send_to_char("\n\r",ch);
              do_say(mob,"ButyouarealreadyacitizenofMountNevermind!Buthey,ifyouwannagivemeyourlootanyways,I'lltakeit");
              send_to_char("\n\r",ch);
              interpret(mob, "rub");
              return;
            }

            if (ch->steel < 5000)
            {
              send_to_char("\n\r",ch);
              do_say(mob,"I'msorry,butthecosttolivehereis5,000steelcoinsandyoudon'tquitehaveenough!");
              return;
            }

            send_to_char("\n\r",ch);
            do_say(mob,"Goody! YouarenowaprestigecitizenofMountNevermind!Welcome!");
            send_to_char("\n\r",ch);
            interpret(mob, "smile");
            ch->steel -= 5000;
            ch->hometown = 7;
            ch->recall_point = hometown_table[ch->hometown].recall;
            return;
          } 
	    else
          {
            send_to_char("\n\r",ch);
            do_say(mob,"Then you must go to Mt. Nevermind to plea your case, not here!");
            send_to_char("\n\r",ch);
            interpret(mob, "mutter");
            return;
          }
        }
        if (!str_cmp("I wish to live in Kendermore", argument)
        ||  !str_cmp("I wish to live in Kendermore.", argument))
        {
              /* Kendermore */
          if ((mob->in_room->vnum <= 14999)
          &&  (mob->in_room->vnum >= 14900))
          {

            if (ch->hometown == 6)
            {
              send_to_char("\n\r",ch);
              do_say(mob,"But your already a citizen of Kendermore! But hey, if you wanna give me your loot anyways, I'll take it");
              send_to_char("\n\r",ch);
              interpret(mob, "rub");
              return;
            }

            if (ch->steel < 5000)
            {
              send_to_char("\n\r",ch);
              do_say(mob,"I'm sorry, but the cost to live here is 5,000 steel coins and you don't quite have enough!");
              return;
            }

            send_to_char("\n\r",ch);
            do_say(mob,"Hurray! You are now a prestige citizen of Kendermore! Welcome!");
            send_to_char("\n\r",ch);
            interpret(mob, "smile");
            ch->steel -= 5000;
            ch->hometown = 6;
            ch->recall_point = hometown_table[ch->hometown].recall;
            return;
          } 
	    else
          {
            send_to_char("\n\r",ch);
            do_say(mob,"Then you must go to Kendermore to plea your case, not here!");
            send_to_char("\n\r",ch);
            interpret(mob, "mutter");
            return;
          }
        }
      }


     /*Speech Prog that allows org members to direct guards*/
      if ((!str_cmp("Go off duty", argument)
      ||   !str_cmp("Go off duty.", argument))
      &&  (mob->spec_fun == spec_lookup("spec_orgguard")))
      {
        if (IS_AFFECTED(mob, AFF_SLEEP))
	  {
	    act("$N is sleeping on the job.\n\r",ch,NULL,mob,TO_NOTVICT);
	    return;
	  }

        if( ORG(mob->org_id) != ORG(ch->org_id) )
        {
          do_say(mob,"I do not take orders from the likes of you!");
          return;
        }

        if(mob->spec_state == 2)
        {
          do_say(mob, "I am already off duty!");
          return;
        }

        mob->spec_state = 2;
        free_string(mob->long_descr);

        switch(ORG(mob->org_id))
        {
          case ORG_HOLYORDER:
           if(ch->sex == 1)
           do_say(mob, "Yes Sir! Our allies may now enter the Citidel.");
          else
           do_say(mob, "Yes Milady! Our allies may now enter the Citidel.");
           mob->long_descr = str_dup("The guardian is here resting after a long duty shift.\r\n");
          return;

          case ORG_BLACKORDER:
           if(ch->sex == 1)
           do_say(mob, "Yes Sir! Our allies may now enter the keep.");
          else
           do_say(mob, "Yes Milady! Our allies may now enter the keep.");
           mob->long_descr = str_dup("The guardian is here resting after a long duty shift.\r\n");
          return;

          case ORG_AESTHETIC:
           if(ch->sex == 1)
           do_say(mob, "Yes Sir! Our allies may now enter the keep.");
          else
           do_say(mob, "Yes Milady! Our allies may now enter the keep.");
           mob->long_descr = str_dup("The guardian is here resting after a long duty shift.\r\n");
          return;

          case ORG_CONCLAVE:
           if(ch->sex == 1)
           do_say(mob, "As you command, my distinguished master.");
          else
           do_say(mob, "As you command, my radiant mistress.");
           mob->long_descr = str_dup("A mysterious spirit is here, with eyes closed.\r\n");
          return;

          case ORG_FORESTER:
           if(ch->sex == 1)
           do_say(mob, "Certainly, my brother!");
          else
           do_say(mob, "As you wish, my sister!");
           mob->long_descr = str_dup("The guardian is here resting.\r\n");
          return;

          case ORG_THIEVES:
           if(ch->sex == 1)
           do_say(mob, "Very well, Sire.  Your associates are now welcome.");
          else
           do_say(mob, "Very well, Mistress.  Your associates are now welcome.");
           mob->long_descr = str_dup("A shadowed figure stands silently in the corner.\r\n");
          return;

          case ORG_KOT:
           if(ch->sex == 1)
           do_say(mob, "At ease as you command, Lord!");
          else
           do_say(mob, "At ease as you command, Mistress!");
           mob->long_descr = str_dup("A Knight of Takhisis is standing here at ease.\r\n");
          return;

          case ORG_MERCENARY:
           if(ch->sex == 1)
           do_say(mob, "I shall remain at ease until further notice, Sir!");
          else
           do_say(mob, "I shall remain at ease until further notice, Ma'am!");
           mob->long_descr = str_dup("A Master Assassin is standing here at ease.\r\n");
          return;

          case ORG_UNDEAD:
           if(ch->sex == 1)
           do_say(mob, "At ease as you command my Lord!");
          else
           do_say(mob, "At ease as you command M'Lady!");
           mob->long_descr = str_dup("An Undead Zombie is standing here at ease.\r\n");
          return;

          case ORG_SOLAMNIC:
           if(ch->sex == 1)
           do_say(mob, "As you command, Lord.  Foreign guests may now pass freely.");
          else
           do_say(mob, "As you command, my Lady.  Foreign guests may now pass freely.");
           mob->long_descr = str_dup("A Solamnic Warden stands cordially aside.\r\n");
          return;

          case ORG_ARTISANS:
           if(ch->sex == 1)
           do_say(mob, "Ah, I thank thee, lad.  Methinks tis a good time to rest my old bones.");
          else
           do_say(mob, "Ah, I thank thee, lass.  These old bones could use a rest.");
           mob->long_descr = str_dup("A dwarven miner is here, resting his weary bones.\r\n");
          return;
        }
      }

      if ((!str_cmp("Go on alert", argument)
      ||   !str_cmp("Go on alert.", argument))
      &&  (mob->spec_fun == spec_lookup("spec_orgguard")))
      {
        if (IS_AFFECTED(mob, AFF_SLEEP))
	  {
	    act("$N is sleeping on the job.\n\r",ch,NULL,mob,TO_NOTVICT);
	    return;
	  }

        if( ORG(mob->org_id) != ORG(ch->org_id) )
        {
          do_say(mob,"I do not take orders from the likes of you!");
          return;
        }

        if(mob->spec_state == 1)
        {
          do_say(mob, "I am already on alert!");
          return;
        }

        mob->spec_state = 1;
        free_string(mob->long_descr);

        switch(ORG(mob->org_id))
        {
          case ORG_HOLYORDER:
          if(ch->sex == 1)
            do_say(mob, "Yes Sir! Intruders shall be dealt with harshly!");
          else
            do_say(mob, "Yes Milady! Intruders shall be dealt with harshly!");
          mob->long_descr = str_dup("A silver-glad griffon is here, vigilant for any foes.\r\n");
          return;

          case ORG_BLACKORDER:
          if(ch->sex == 1)
            do_say(mob, "Yes Sir! Intruders shall be dealt with harshly!");
          else
            do_say(mob, "Yes Milady! Intruders shall be dealt with harshly!");
          mob->long_descr = str_dup("A demon is here, vigilant for any foes.\r\n");
          return;

          case ORG_AESTHETIC:
          if(ch->sex == 1)
            do_say(mob, "Yes Sir! Intruders shall be dealt with harshly!");
          else
            do_say(mob, "Yes Milady! Intruders shall be dealt with harshly!");
          mob->long_descr = str_dup("A cleric stands here, vigilant for any foes.\r\n");
          return;

          case ORG_CONCLAVE:
          if(ch->sex == 1)
            do_say(mob, "Intruders shall face my wrath, distinguished master!");
          else
            do_say(mob, "Intruders shall face my wrath, radiant mistress!");
          mob->long_descr = str_dup("A spirit is guarding the tower, it's eyes burning with white fire.\r\n");
          return;

          case ORG_FORESTER:
          if(ch->sex == 1)
            do_say(mob, "Yes Sir!");
          else
            do_say(mob, "Yes Milady!");
          mob->long_descr = str_dup("The guardian is here looking sharp.\r\n");
          return;

          case ORG_THIEVES:
          if(ch->sex == 1)
            do_say(mob, "Intruders shall be met with lethal force, Sire.");
          else
            do_say(mob, "Intruders shall be met with lethal force, Mistress.");
          mob->long_descr = str_dup("A shadowy cloaked figure glares out from the darkness.\r\n");
          return;

          case ORG_KOT:
          if(ch->sex == 1)
            do_say(mob, "No mercy shall be shown to those who attempt to breach the Stronghold!");
          else
            do_say(mob, "No mercy shall be shown to those who attempt to breach the Stronghold!");
          mob->long_descr = str_dup("A Knight of Takhisis is here at arms, alertly guarding the Stronghold.\r\n");
          return;

          case ORG_MERCENARY:
          if(ch->sex == 1)
            do_say(mob, "Trespassers shall be executed as you command, Lord.");
          else
            do_say(mob, "Trespassers shall be executed as you command, Mistress.");
          mob->long_descr= str_dup("A Master Assassin is prepared to strike at any intruders.\r\n");
          return;

          case ORG_UNDEAD:
           if(ch->sex == 1)
           do_say(mob, "Intruders shall join us in death M'Lord!");
          else
           do_say(mob, "Intruders shall join us in death M'Lady!");
           mob->long_descr = str_dup("An Undead Zombie is prepared to welcome death to any intruders.\r\n");
          return;

          case ORG_SOLAMNIC:
          if(ch->sex == 1)
            do_say(mob, "I am prepared to fight for the defense of our oath, my Lord!");
          else
            do_say(mob, "I am prepared to fight in the defense of our oath, my Lady!");
          mob->long_descr = str_dup("A Solamnic Warden is here, prepared to fight in defense of the Headquarters.\r\n");
         return;

          case ORG_ARTISANS:
          if(ch->sex == 1)
            do_say(mob, "Yes lad, I'll be sure ye are informed of any activity.");
          else
            do_say(mob, "Yes lass, I'll be sure ye are informed of any activity.");
          mob->long_descr = str_dup("An alert dwarven miner is here, discouraging outsiders.\r\n");
          return;
        }
      }

      if ((!str_cmp("Go on duty", argument)
      ||   !str_cmp("Go on duty.", argument))
      &&  (mob->spec_fun == spec_lookup("spec_orgguard")))
      {
        if (IS_AFFECTED(mob, AFF_SLEEP))
	  {
	    act("$N is sleeping on the job.\n\r",ch,NULL,mob,TO_NOTVICT);
	    return;
	  }

        if( ORG(mob->org_id) != ORG(ch->org_id) )
        {
          do_say(mob,"I do not take orders from the likes of you!");
          return;
        }

        if(mob->spec_state == 0)
        {
          do_say(mob, "I am already on alert!");
          return;
        }

        mob->spec_state = 0;
        free_string(mob->long_descr);

        switch(ORG(mob->org_id))
        {
          case ORG_HOLYORDER:
          if(ch->sex == 1)
            do_say(mob, "Yes Sir! The Citidel shall be kept free of intruders.");
          else
           do_say(mob, "Yes Milady! The Citidel shall be kept free of intruders.");
          mob->long_descr = str_dup("A silver-glad griffon is here, guarding the entrance to the Citidel.\r\n");
          return;

          case ORG_BLACKORDER:
          if(ch->sex == 1)
            do_say(mob, "Yes Sir! The Citidel shall be kept free of intruders.");
          else
           do_say(mob, "Yes Milady! The Citidel shall be kept free of intruders.");
          mob->long_descr = str_dup("A demon is here, guarding the entrance to the Citidel.\r\n");
          return;

          case ORG_AESTHETIC:
          if(ch->sex == 1)
            do_say(mob, "Yes Sir! The Library shall be kept free of intruders.");
          else
           do_say(mob, "Yes Milady! The Library shall be kept free of intruders.");
          mob->long_descr = str_dup("A cleric is here, guarding the entrance to the Library.\r\n");
          return;

          case ORG_CONCLAVE:
          if(ch->sex == 1)
            do_say(mob, "As you command, my distinguished master.");
          else
            do_say(mob, "As you command, my radiant mistress.");
          mob->long_descr = str_dup("A spirit is here, guarding the Tower of High Sorcery.\r\n");
          return;

          case ORG_FORESTER:
          if(ch->sex == 1)
            do_say(mob, "Yes Sir!");
          else
            do_say(mob, "Yes Milady!");
          mob->long_descr = str_dup("The guardian is here.\r\n");
          return;

          case ORG_THIEVES:
          if(ch->sex == 1)
            do_say(mob,"Yes, Sire.  Rest assured I shall entertain unwelcome guests appropriately.");
          else
            do_say(mob,"Rest assured I shall entertain unwelcome guests appropriately, Mistress.");
          mob->long_descr = str_dup("A shadowed figure stands here, grinning deviously.\r\n");
          return;

          case ORG_KOT:
          if(ch->sex == 1)
            do_say(mob, "As you decree, Lord!");
          else
            do_say(mob, "As you decree, Mistress!");
          mob->long_descr = str_dup("A Knight of Takhisis is standing here on duty.\r\n");
          return;

          case ORG_MERCENARY:
          if(ch->sex == 1)
            do_say(mob, "As you wish, Lord!");
          else
            do_say(mob, "As you wish, Mistress!");
          mob->long_descr = str_dup("A Master Assassin is standing here on duty.\r\n");
          return;

          case ORG_UNDEAD:
           if(ch->sex == 1)
           do_say(mob, "As you command M'Lord!");
          else
           do_say(mob, "As you command M'Lady!");
           mob->long_descr = str_dup("An Undead Zombie is stands guard.\r\n");
          return;

          case ORG_SOLAMNIC:
          if(ch->sex == 1)
            do_say(mob, "Resuming standard duty as you command, my Lord.");
          else
            do_say(mob, "Resuming standard duty as you command, my Lady!");
          mob->long_descr = str_dup("A Solamnic Warden is here, guarding the Solamnic Headquarters.\r\n");
          return;

          case ORG_ARTISANS:
          if(ch->sex == 1)
            do_say(mob, "Aye, lad.  Trust that the mine be well guarded.");
          else
            do_say(mob, "Aye, lass.  Trust that the mine be well guarded.");
          mob->long_descr = str_dup("A dwarven miner is here, discouraging outsiders.\r\n");
        return;
        }
      }

	}
	for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;
	    if ( HAS_TRIGGER_OBJ( obj, TRIG_SPEECH ) )
		p_act_trigger( argument, NULL, obj, NULL, ch, NULL, NULL, TRIG_SPEECH );
	}
	
	if ( HAS_TRIGGER_ROOM( ch->in_room, TRIG_SPEECH ) )
	    p_act_trigger( argument, NULL, NULL, ch->in_room, ch, NULL, NULL, TRIG_SPEECH );
    }
    return;
}


void do_shout( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;
    int head_god = FALSE;

    head_god = !str_cmp(ch->name, HEAD_GOD) 
            || !str_cmp(ch->name,HEAD_GOD2) 
            || !str_cmp(ch->name, HEAD_GOD3);

    if (argument[0] == '\0' )
    {
      	if (IS_SET(ch->comm,COMM_SHOUTSOFF))
      	{
            send_to_char("You can hear shouts again.\n\r",ch);
            REMOVE_BIT(ch->comm,COMM_SHOUTSOFF);
      	}
      	else
      	{
            send_to_char("You will no longer hear shouts.\n\r",ch);
            SET_BIT(ch->comm,COMM_SHOUTSOFF);
      	}
      	return;
    }

    if (((!str_infix("mud", argument))
    ||  (!str_infix("MUD", argument))
    ||  (!str_infix("MuD", argument))
    ||  (!str_infix("MUd", argument))
    ||  (!str_infix("mUD", argument))
    ||  (!str_infix("muD", argument))
    ||  (!str_infix("Mud", argument))
    ||  (!str_infix("mUd", argument))
    ||  (!str_suffix("mud", argument))
    ||  (!str_suffix("MUD", argument))
    ||  (!str_suffix("Mud", argument))
    ||  (!str_suffix("mud.", argument))
    ||  (!str_suffix("MUD.", argument))
    ||  (!str_suffix("Mud.", argument))
    ||  (!str_suffix("mud!", argument))
    ||  (!str_suffix("MUD!", argument))
    ||  (!str_suffix("Mud!", argument))
    ||  (!str_cmp("mud", argument))
    ||  (!str_cmp("mud,", argument))
    ||  (!str_cmp("MUD,", argument))
    ||  (!str_cmp("Mud,", argument))
    ||  (!str_cmp("mud.", argument))
    ||  (!str_cmp("MUD.", argument))
    ||  (!str_cmp("Mud.", argument))
    ||  (!str_cmp("mud!", argument))
    ||  (!str_cmp("MUD!", argument))
    ||  (!str_cmp("Mud!", argument))
    ||  (!str_prefix("Mud", argument))
    ||  (!str_prefix("MUD", argument))
    ||  (!str_prefix("mud", argument)))
    &&  !head_god)
    {
    sprintf( log_buf, "[WARNING] %s shouts: %s", ch->name, argument);
    log_string( log_buf );
    wiznet(log_buf,NULL,NULL,WIZ_MULTI,0,0);
    }

    if (IS_AFFECTED2(ch, AFF_SILENCE) )
    {
        send_to_char("You have been silenced! Your tongue is no longer responsive!\n\r",ch);
        act("$n tries to speak, but $s tongue doesn't seem to be working properly.",ch,NULL,NULL,TO_ROOM);
        return;
    }

    if ( IS_SET(ch->comm, COMM_NOSHOUT) )
    {
        send_to_char( "You can't shout.\n\r", ch );
        return;
    }
 
    REMOVE_BIT(ch->comm,COMM_SHOUTSOFF);


    WAIT_STATE( ch, 12 );

    act( "{RYou shout '$T'{R", ch, NULL, argument, TO_CHAR );
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	CHAR_DATA *victim;

	victim = d->original ? d->original : d->character;

	if ( d->connected == CON_PLAYING &&
	     d->character != ch &&
	     !IS_SET(victim->comm, COMM_SHOUTSOFF) &&
	     !IS_SET(victim->comm, COMM_QUIET) ) 
	{
        if (!check_ignore(victim,ch))
	    act("{R$n shouts '$t'{R",ch,argument,d->character,TO_VICT);
	}
    }

    return;
}

void do_whisper( CHAR_DATA *ch, char *argument )
{
#ifdef LANGUAGES
    int language;
    char buf[MSL];
    char *mesg;
#endif
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *wch;
    int chance;
    int head_god = FALSE;

    argument = one_argument( argument, arg );
    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
        send_to_char( "Whisper whom what?\n\r", ch );
        return;
    }
    victim = NULL;

	if(!IS_NPC(ch) && IS_SET(ch->comm,COMM_NOCHANNELS) )
	{
		send_to_char("Your channel privileges have been revoked.\n\r",ch);
		return;
	}

    head_god = !str_cmp(ch->name, HEAD_GOD) 
            || !str_cmp(ch->name,HEAD_GOD2) 
            || !str_cmp(ch->name, HEAD_GOD3);

    if( ch->in_room != NULL )
    {
        for( wch = ch->in_room->people; wch != NULL; wch = wch->next_in_room )
        {
            if(!can_see(ch, wch))
	    	continue;

            if (is_affected(wch,gsn_doppelganger) 
            && IS_THIEVES(wch))
                continue;

            if((IS_NPC(wch) && !str_prefix(arg, wch->short_descr))
	      || (!IS_NPC(wch) && !str_prefix(arg, wch->name)))
            {
                victim = wch;
                break;
            }
        }
    }

    /* see if we found em */
    if( victim == NULL )
    {
      send_to_char( "They are not here.\n\r", ch );
      return;
    }

    if (((!str_infix("mud", argument))
    ||  (!str_infix("MUD", argument))
    ||  (!str_infix("MuD", argument))
    ||  (!str_infix("MUd", argument))
    ||  (!str_infix("mUD", argument))
    ||  (!str_infix("muD", argument))
    ||  (!str_infix("Mud", argument))
    ||  (!str_infix("mUd", argument))
    ||  (!str_suffix("mud", argument))
    ||  (!str_suffix("MUD", argument))
    ||  (!str_suffix("Mud", argument))
    ||  (!str_suffix("mud.", argument))
    ||  (!str_suffix("MUD.", argument))
    ||  (!str_suffix("Mud.", argument))
    ||  (!str_suffix("mud!", argument))
    ||  (!str_suffix("MUD!", argument))
    ||  (!str_suffix("Mud!", argument))
    ||  (!str_cmp("mud", argument))
    ||  (!str_cmp("mud,", argument))
    ||  (!str_cmp("MUD,", argument))
    ||  (!str_cmp("Mud,", argument))
    ||  (!str_cmp("mud.", argument))
    ||  (!str_cmp("MUD.", argument))
    ||  (!str_cmp("Mud.", argument))
    ||  (!str_cmp("mud!", argument))
    ||  (!str_cmp("MUD!", argument))
    ||  (!str_cmp("Mud!", argument))
    ||  (!str_prefix("Mud", argument))
    ||  (!str_prefix("MUD", argument))
    ||  (!str_prefix("mud", argument)))
    &&  !head_god)
    {
    sprintf( log_buf, "[WARNING] %s whispers to %s: %s", ch->name, victim->name, argument);
    log_string( log_buf );
    wiznet(log_buf,NULL,NULL,WIZ_MULTI,0,0);
    }

    if (victim == ch)
    {
       send_to_char("You whisper to yourself.\n\r",ch);
       return;
    }
   
    if (( victim->desc == NULL)
    || (!IS_IMMORTAL(ch) && !IS_AWAKE(victim) ))
    {
       	act( "They cannot hear you.", ch, 0, victim, TO_CHAR );
       	return;
    }

    if (IS_AFFECTED2(ch, AFF_SILENCE))
    {
     send_to_char("The silence that affects you prevents your message from getting through.\n\r", ch);
    return;
    } 

    /* check for speech impairment */
    argument = speech_impair( ch, argument );
#ifdef LANGUAGES
    mesg = language_check(ch, argument, NULL);
    if ((language = lang_skill(ch, NULL)) != 0)
        sprintf(buf, "You whisper to $N in %s {b'$t'{x",capitalize(lang_lookup(ch->language)));
    else
	sprintf(buf, "You whisper to $N {b'$t'{x");

    act( buf, ch, mesg, victim, TO_CHAR );
#else
    act( "You whisper to $n {b'$t'{x", ch, argument, victim, TO_CHAR);
#endif

    for( wch = ch->in_room->people; wch != NULL; wch = wch->next_in_room )
    {
        if(!can_see(wch, ch) && wch != victim)
    	   continue;

	if(wch!= ch && wch!=victim && !IS_NPC(wch))	
	   {
	   sprintf(buf,"%s whispers to %s.",ch->name, can_see(wch,victim) ? victim->name : "someone");
	   act(buf,wch,NULL,victim,TO_CHAR);
	   }

	if(wch!= ch && wch!=victim && IS_NPC(wch))	
	   {
	   sprintf(buf,"%s whispers to %s.",ch->name, wch->short_descr);
	   act(buf,wch,NULL,victim,TO_CHAR);
	   }

#ifdef LANGUAGES
	chance = get_skill(wch,gsn_eavesdrop);
        if((victim == wch) && !IS_AFFECTED2(wch, AFF_DEAFNESS))
        {
	    if (lang_skill(ch, victim))
		sprintf(buf, "{w$n whispers in %s {b'$t'{x",capitalize(lang_lookup(ch->language)));
	    else
		sprintf(buf, "{w$n whispers {b'$t'{x");

	    act( buf, ch, language_check(ch, mesg, victim), victim, TO_VICT);
        }
	else if (IS_NPC(wch) || (chance > 0))
	{
	    if (!IS_NPC(wch) 
	    && (chance < number_percent()))
		continue;

	  if (!IS_AFFECTED(wch, AFF_DEAFNESS))
        {
	    if (lang_skill(ch, wch))
		sprintf(buf, "You Eavesdrop:\n{w$n whispers to %s in %s {b'$t'{x",can_see(wch,victim)? victim->name : "someone", capitalize(lang_lookup(ch->language)));
	    else
		sprintf(buf, "You Eavesdrop:\n{w$n whispers to %s {b'$t'{x", can_see(wch,victim)? victim->name : "someone" );

	    act( buf, ch, language_check(ch, mesg, wch), wch, TO_VICT);
	    check_improve(wch, gsn_eavesdrop, TRUE, 2);
        }
	}
#else
      if(victim == wch))
        act("{w$n whispers {b'$t'{x",ch,argument,victim,TO_VICT,);
	else if (IS_NPC(wch) || (chance = get_skill(wch,gsn_eavesdrop)) > 0)
	{
	  if (!IS_NPC(wch) && chance < number_percent())
		continue;
        act("You Eavesdrop:\n{w$n whispers {b'$t'{x",ch,argument,wch,TO_VICT,);
	}
#endif

    }
    return;
}

void do_tell( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    int head_god = FALSE;

    head_god = !str_cmp(ch->name, HEAD_GOD) 
            || !str_cmp(ch->name, HEAD_GOD2) 
            || !str_cmp(ch->name, HEAD_GOD3);


    if ( IS_SET(ch->comm, COMM_NOTELL) || IS_SET(ch->comm,COMM_DEAF))
    {
	send_to_char( "Your message didn't get through.\n\r", ch );
	return;
    }

    if ( IS_SET(ch->comm, COMM_QUIET) 
    &&   str_cmp(ch->name, HEAD_GOD))
    {
	send_to_char( "You must turn off quiet mode first.\n\r", ch);
	return;
    }

    if (IS_SET(ch->comm,COMM_DEAF)
    &&  str_cmp(ch->name, HEAD_GOD))
    {
	send_to_char("You must turn off deaf mode first.\n\r",ch);
	return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Tell whom what?\n\r", ch );
	return;
    }

    /*
     * Can tell to PC's anywhere, but NPC's only in same room.
     * -- Furey
     */
    if ( ( victim = get_char_world( ch, arg ) ) == NULL
    || ( IS_NPC(victim) && victim->in_room != ch->in_room ) )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if(!IS_NPC(ch)
    && !IS_IMMORTAL(victim)
    && IS_SET(ch->comm, COMM_NOCHANNELS) )
    {
	send_to_char("Your channel privileges have been revoked.\n\r",ch);
	return;
    }

    if ( victim->desc == NULL && !IS_NPC(victim))
    {
	act("$N seems to have misplaced $S link...try again later.",
	    ch,NULL,victim,TO_CHAR);
      sprintf(buf,"{k%s tells you '{K%s{k'{x\n\r",PERS(ch,victim),argument);
      buf[0] = UPPER(buf[0]);
      add_buf(victim->pcdata->buffer,buf);
	return;
    }

    if ( !(IS_IMMORTAL(ch) && ch->level > LEVEL_IMMORTAL) && !IS_AWAKE(victim) )
    {
	if (IS_NPC(victim))
	{
	  act("$E is can't hear you.",ch,NULL,victim,TO_CHAR);
	  return;
	}

	act( "$E can't hear you.", ch, 0, victim, TO_CHAR );
	sprintf(buf,"{k%s tells you '{K%s{k'{x\n\r",PERS(ch,victim),argument);
	buf[0] = UPPER(buf[0]);
	add_buf(victim->pcdata->buffer,buf);
	return;
    }
  
    if ((IS_SET(victim->comm,COMM_QUIET) || IS_SET(victim->comm,COMM_DEAF))
    && !IS_IMMORTAL(ch))
    {
	act( "$E is not receiving tells.", ch, 0, victim, TO_CHAR );
  	return;
    }

    if ((IS_SET(victim->comm,COMM_QUIET))
    && !str_cmp(victim->name, HEAD_GOD))
    {
	act( "$E is not receiving tells.", ch, 0, victim, TO_CHAR );
  	return;
    }

    if (check_ignore(victim,ch))
    {
	act( "$E is ignoring you! Stop bothering them!", ch, 0, victim, TO_CHAR );
  	return;
    }

    if (IS_SET(victim->comm,COMM_WRITING)
    && !IS_IMPLEMENTOR(ch))
    {
	act("$E is writing a note, and is not receiving any tells right now.",ch,NULL,victim,TO_CHAR);
	return;
    }

    if (IS_SET(victim->comm,COMM_EDITOR)
    && !IS_IMPLEMENTOR(ch))
    {
	act("$E is in a text editor right now, and is not receiving any tells.",ch,NULL,victim,TO_CHAR);
	return;
    }

    if (((!str_infix("mud", argument))
    ||  (!str_infix("MUD", argument))
    ||  (!str_infix("MuD", argument))
    ||  (!str_infix("MUd", argument))
    ||  (!str_infix("mUD", argument))
    ||  (!str_infix("muD", argument))
    ||  (!str_infix("Mud", argument))
    ||  (!str_infix("mUd", argument))
    ||  (!str_suffix("mud", argument))
    ||  (!str_suffix("MUD", argument))
    ||  (!str_suffix("Mud", argument))
    ||  (!str_suffix("mud.", argument))
    ||  (!str_suffix("MUD.", argument))
    ||  (!str_suffix("Mud.", argument))
    ||  (!str_suffix("mud!", argument))
    ||  (!str_suffix("MUD!", argument))
    ||  (!str_suffix("Mud!", argument))
    ||  (!str_cmp("mud", argument))
    ||  (!str_cmp("mud,", argument))
    ||  (!str_cmp("MUD,", argument))
    ||  (!str_cmp("Mud,", argument))
    ||  (!str_cmp("mud.", argument))
    ||  (!str_cmp("MUD.", argument))
    ||  (!str_cmp("Mud.", argument))
    ||  (!str_cmp("mud!", argument))
    ||  (!str_cmp("MUD!", argument))
    ||  (!str_cmp("Mud!", argument))
    ||  (!str_prefix("Mud", argument))
    ||  (!str_prefix("MUD", argument))
    ||  (!str_prefix("mud", argument)))
    &&  !head_god)
    {
      sprintf( log_buf, "[WARNING] %s tells %s: %s", ch->name, victim->name, argument);
      log_string( log_buf );
      wiznet(log_buf,NULL,NULL,WIZ_MULTI,0,0);
    }

    if (IS_SET(victim->comm,COMM_AFK))
    {
	if (IS_NPC(victim))
	{
	  act("$E is AFK, and not receiving tells.",ch,NULL,victim,TO_CHAR);
	  return;
	}

	act("$E is AFK, but your tell will go through when $E returns.",
	    ch,NULL,victim,TO_CHAR);
	sprintf(buf,"{k%s tells you '{K%s{k'{x\n\r",PERS(ch,victim),argument);
	buf[0] = UPPER(buf[0]);
	add_buf(victim->pcdata->buffer,buf);
	return;
    }

    act( "{kYou tell $N '{K$t{k'{x", ch,argument, victim, TO_CHAR );
    act_new("{k$n tells you '{K$t{k'{x",ch,argument,victim,TO_VICT,POS_DEAD);
    victim->reply	= ch;

    if ( !IS_NPC(ch) && IS_NPC(victim) && HAS_TRIGGER_MOB(victim,TRIG_SPEECH) )
	p_act_trigger( argument, victim, NULL, NULL, ch, NULL, NULL, TRIG_SPEECH );

    return;
}



void do_reply( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];
    int head_god = FALSE;

    if ( IS_SET(ch->comm, COMM_NOTELL) )
    {
	send_to_char( "Your message didn't get through.\n\r", ch );
	return;
    }

    head_god = !str_cmp(ch->name, HEAD_GOD) 
            || !str_cmp(ch->name,HEAD_GOD2) 
            || !str_cmp(ch->name, HEAD_GOD3);

    if ( ( victim = ch->reply ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

	if(!IS_NPC(ch) && !IS_IMMORTAL(victim) && IS_SET(ch->comm, COMM_NOCHANNELS))
	{
		send_to_char("Your channel privileges have been revoked.\n\r",ch);
		return;
	}

    if (((!str_infix("mud", argument))
    ||  (!str_infix("MUD", argument))
    ||  (!str_infix("MuD", argument))
    ||  (!str_infix("MUd", argument))
    ||  (!str_infix("mUD", argument))
    ||  (!str_infix("muD", argument))
    ||  (!str_infix("Mud", argument))
    ||  (!str_infix("mUd", argument))
    ||  (!str_suffix("mud", argument))
    ||  (!str_suffix("MUD", argument))
    ||  (!str_suffix("Mud", argument))
    ||  (!str_suffix("mud.", argument))
    ||  (!str_suffix("MUD.", argument))
    ||  (!str_suffix("Mud.", argument))
    ||  (!str_suffix("mud!", argument))
    ||  (!str_suffix("MUD!", argument))
    ||  (!str_suffix("Mud!", argument))
    ||  (!str_cmp("mud", argument))
    ||  (!str_cmp("mud,", argument))
    ||  (!str_cmp("MUD,", argument))
    ||  (!str_cmp("Mud,", argument))
    ||  (!str_cmp("mud.", argument))
    ||  (!str_cmp("MUD.", argument))
    ||  (!str_cmp("Mud.", argument))
    ||  (!str_cmp("mud!", argument))
    ||  (!str_cmp("MUD!", argument))
    ||  (!str_cmp("Mud!", argument))
    ||  (!str_prefix("Mud", argument))
    ||  (!str_prefix("MUD", argument))
    ||  (!str_prefix("mud", argument)))
    &&  !head_god)
    {
    sprintf( log_buf, "[WARNING] %s replies to %s: %s", ch->name, victim->name, argument);
    log_string( log_buf );
    wiznet(log_buf,NULL,NULL,WIZ_MULTI,0,0);
    }

    if ( victim->desc == NULL && !IS_NPC(victim))
    {
        act("$N seems to have misplaced $S link...try again later.",
            ch,NULL,victim,TO_CHAR);
        sprintf(buf,"{k%s tells you '{K%s{k'{x\n\r",PERS(ch,victim),argument);
        buf[0] = UPPER(buf[0]);
        add_buf(victim->pcdata->buffer,buf);
        return;
    }

    if (check_ignore(victim,ch))
    {
	act( "$E is ignoring you! Stop bothering them!", ch, 0, victim, TO_CHAR );
  	return;
    }

    if ( !IS_IMMORTAL(ch) && !IS_AWAKE(victim) )
    {
	act( "$E can't hear you.", ch, 0, victim, TO_CHAR );
	return;
    }

    if ((IS_SET(victim->comm,COMM_QUIET) || IS_SET(victim->comm,COMM_DEAF))
    &&  !IS_IMMORTAL(ch))
    {
        act_new( "$E is not receiving tells.", ch, 0, victim, TO_CHAR,POS_DEAD);
        return;
    }

    if ((IS_SET(victim->comm,COMM_QUIET))
    && !str_cmp(victim->name, HEAD_GOD))
    {
	act( "$E is not receiving tells.", ch, 0, victim, TO_CHAR );
  	return;
    }

    if (!IS_IMMORTAL(victim) && !IS_AWAKE(ch))
    {
	send_to_char( "In your dreams, or what?\n\r", ch );
	return;
    }

    if (IS_SET(victim->comm,COMM_WRITING)
    && !IS_IMPLEMENTOR(ch))
    {
       act_new("$E is writing a note, and is not receiving any tells.",ch,NULL,victim,TO_CHAR,POS_DEAD);
       return;
    }

    if (IS_SET(victim->comm,COMM_EDITOR)
    && !IS_IMPLEMENTOR(ch))
    {
	act("$E is in a text editor right now, and is not receiving any tells.",ch,NULL,victim,TO_CHAR);
	return;
    }

    if (IS_SET(victim->comm,COMM_AFK))
    {
        if (IS_NPC(victim))
        {
            act_new("$E is AFK, and not receiving tells.",
		ch,NULL,victim,TO_CHAR,POS_DEAD);
            return;
        }
 
        act_new("$E is AFK, but your tell will go through when $E returns.",
            ch,NULL,victim,TO_CHAR,POS_DEAD);
        sprintf(buf,"{k%s tells you '{K%s{k'{x\n\r",PERS(ch,victim),argument);
	buf[0] = UPPER(buf[0]);
        add_buf(victim->pcdata->buffer,buf);
        return;
    }

    act_new("{kYou tell $N '{K$t{k'{x",ch,argument,victim,TO_CHAR,POS_DEAD);
    act_new("{k$n tells you '{K$t{k'{x",ch,argument,victim,TO_VICT,POS_DEAD);
    victim->reply	= ch;

    return;
}



void do_yell( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;
    int head_god = FALSE;

    if ( IS_SET(ch->comm, COMM_NOSHOUT) )
    {
        send_to_char( "You can't yell.\n\r", ch );
        return;
    }

    if(!IS_NPC(ch) && IS_SET(ch->comm,COMM_NOCHANNELS) )
    {
	send_to_char("Your channel privileges have been revoked.\n\r",ch);
	return;
    }

    if (IS_AFFECTED2(ch, AFF_SILENCE) )
    {
        send_to_char("You have been silenced! Your tongue is no longer responsive!\n\r",ch);
        act("$n tries to speak, but $s tongue doesn't seem to be working properly.",ch,NULL,NULL,TO_ROOM);
        return;
    }

    if ( IS_SET(ch->in_room->room_flags, ROOM_QUIET))
    {
        send_to_char( "You can't yell in this room.\n\r", ch );
        return;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char( "Yell what?\n\r", ch );
	return;
    }

    head_god = !str_cmp(ch->name, HEAD_GOD) 
            || !str_cmp(ch->name,HEAD_GOD2) 
            || !str_cmp(ch->name, HEAD_GOD3);

    if (((!str_infix("mud", argument))
    ||  (!str_infix("MUD", argument))
    ||  (!str_infix("MuD", argument))
    ||  (!str_infix("MUd", argument))
    ||  (!str_infix("mUD", argument))
    ||  (!str_infix("muD", argument))
    ||  (!str_infix("Mud", argument))
    ||  (!str_infix("mUd", argument))
    ||  (!str_suffix("mud", argument))
    ||  (!str_suffix("MUD", argument))
    ||  (!str_suffix("Mud", argument))
    ||  (!str_suffix("mud.", argument))
    ||  (!str_suffix("MUD.", argument))
    ||  (!str_suffix("Mud.", argument))
    ||  (!str_suffix("mud!", argument))
    ||  (!str_suffix("MUD!", argument))
    ||  (!str_suffix("Mud!", argument))
    ||  (!str_cmp("mud", argument))
    ||  (!str_cmp("mud,", argument))
    ||  (!str_cmp("MUD,", argument))
    ||  (!str_cmp("Mud,", argument))
    ||  (!str_cmp("mud.", argument))
    ||  (!str_cmp("MUD.", argument))
    ||  (!str_cmp("Mud.", argument))
    ||  (!str_cmp("mud!", argument))
    ||  (!str_cmp("MUD!", argument))
    ||  (!str_cmp("Mud!", argument))
    ||  (!str_prefix("Mud", argument))
    ||  (!str_prefix("MUD", argument))
    ||  (!str_prefix("mud", argument)))
    &&  !head_god)
    {
    sprintf( log_buf, "[WARNING] %s yells: %s", ch->name, argument);
    log_string( log_buf );
    wiznet(log_buf,NULL,NULL,WIZ_MULTI,0,0);
    }

    act("{RYou yell '$t'{x",ch,argument,NULL,TO_CHAR);
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d->connected == CON_PLAYING
	&&   d->character != ch
	&&   d->character->in_room != NULL
	&&   d->character->in_room->area == ch->in_room->area 
      &&   !IS_SET(d->character->comm,COMM_QUIET)
      &&   !IS_SET(d->character->in_room->room_flags, ROOM_QUIET)
	&&   !IS_AFFECTED2(d->character, AFF_DEAFNESS) )
	{
	    act("{R$n yells '$t'{x",ch,argument,d->character,TO_VICT);
	}
    }

    return;
}


void do_emote( CHAR_DATA *ch, char *argument )
{
    int head_god = FALSE;

    if ( !IS_NPC(ch) && IS_SET(ch->comm, COMM_NOEMOTE) )
    {
        send_to_char( "You can't show your emotions.\n\r", ch );
        return;
    }

	if(!IS_NPC(ch) && IS_SET(ch->comm, COMM_NOCHANNELS) )
	{
		send_to_char("Your channel privileges have been revoked.\n\r",ch);
		return;
	}

    if (IS_AFFECTED2(ch, AFF_PARALYSIS) )
    {
        send_to_char("You have been paralized! You couldn't even emote a twitch!\n\r",ch);
        return;
    }

    if (IS_AFFECTED3(ch, AFF_HOLDPERSON) )
    {
        send_to_char("Your can't move, hence you can not emote!\n\r",ch);
        return;
    }
 
    if ( argument[0] == '\0' )
    {
        send_to_char( "Emote what?\n\r", ch );
        return;
    }

    head_god = !str_cmp(ch->name, HEAD_GOD) 
            || !str_cmp(ch->name,HEAD_GOD2) 
            || !str_cmp(ch->name, HEAD_GOD3);

    if (((!str_infix("mud", argument))
    ||  (!str_infix("MUD", argument))
    ||  (!str_infix("MuD", argument))
    ||  (!str_infix("MUd", argument))
    ||  (!str_infix("mUD", argument))
    ||  (!str_infix("muD", argument))
    ||  (!str_infix("Mud", argument))
    ||  (!str_infix("mUd", argument))
    ||  (!str_suffix("mud", argument))
    ||  (!str_suffix("MUD", argument))
    ||  (!str_suffix("Mud", argument))
    ||  (!str_suffix("mud.", argument))
    ||  (!str_suffix("MUD.", argument))
    ||  (!str_suffix("Mud.", argument))
    ||  (!str_suffix("mud!", argument))
    ||  (!str_suffix("MUD!", argument))
    ||  (!str_suffix("Mud!", argument))
    ||  (!str_cmp("mud", argument))
    ||  (!str_cmp("mud,", argument))
    ||  (!str_cmp("MUD,", argument))
    ||  (!str_cmp("Mud,", argument))
    ||  (!str_cmp("mud.", argument))
    ||  (!str_cmp("MUD.", argument))
    ||  (!str_cmp("Mud.", argument))
    ||  (!str_cmp("mud!", argument))
    ||  (!str_cmp("MUD!", argument))
    ||  (!str_cmp("Mud!", argument))
    ||  (!str_prefix("Mud", argument))
    ||  (!str_prefix("MUD", argument))
    ||  (!str_prefix("mud", argument)))
    &&  !head_god)
    {
    sprintf( log_buf, "[WARNING] %s emotes: %s", ch->name, argument);
    log_string( log_buf );
    wiznet(log_buf,NULL,NULL,WIZ_MULTI,0,0);
    }

    if (IS_AFFECTED(ch, AFF_HIDE))    
    {
        REMOVE_BIT( ch->affected_by, AFF_HIDE );
        act("You step out of the shadows.", ch, NULL, NULL, TO_CHAR);
        act("$n steps out of the shadows.", ch, NULL, NULL, TO_ROOM);
    }
 
    MOBtrigger = FALSE;
    act( "$n $T", ch, NULL, argument, TO_ROOM );
    act( "$n $T", ch, NULL, argument, TO_CHAR );
    MOBtrigger = TRUE;
    return;
}


void do_pmote( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *vch;
    char *letter,*name;
    char last[MAX_INPUT_LENGTH], temp[MAX_STRING_LENGTH];
    int matches = 0;
    int head_god = FALSE;

    if ( !IS_NPC(ch) && IS_SET(ch->comm, COMM_NOEMOTE) )
    {
        send_to_char( "You can't show your emotions.\n\r", ch );
        return;
    }
 
 	if(!IS_NPC(ch) && IS_SET(ch->comm,COMM_NOCHANNELS) )
	{
		send_to_char("Your channel privileges have been revoked.\n\r",ch);
		return;
	}

    if ( argument[0] == '\0' )
    {
        send_to_char( "Emote what?\n\r", ch );
        return;
    }

    head_god = !str_cmp(ch->name, HEAD_GOD) 
            || !str_cmp(ch->name,HEAD_GOD2) 
            || !str_cmp(ch->name, HEAD_GOD3);

    if (((!str_infix("mud", argument))
    ||  (!str_infix("MUD", argument))
    ||  (!str_infix("MuD", argument))
    ||  (!str_infix("MUd", argument))
    ||  (!str_infix("mUD", argument))
    ||  (!str_infix("muD", argument))
    ||  (!str_infix("Mud", argument))
    ||  (!str_infix("mUd", argument))
    ||  (!str_suffix("mud", argument))
    ||  (!str_suffix("MUD", argument))
    ||  (!str_suffix("Mud", argument))
    ||  (!str_suffix("mud.", argument))
    ||  (!str_suffix("MUD.", argument))
    ||  (!str_suffix("Mud.", argument))
    ||  (!str_suffix("mud!", argument))
    ||  (!str_suffix("MUD!", argument))
    ||  (!str_suffix("Mud!", argument))
    ||  (!str_cmp("mud", argument))
    ||  (!str_cmp("mud,", argument))
    ||  (!str_cmp("MUD,", argument))
    ||  (!str_cmp("Mud,", argument))
    ||  (!str_cmp("mud.", argument))
    ||  (!str_cmp("MUD.", argument))
    ||  (!str_cmp("Mud.", argument))
    ||  (!str_cmp("mud!", argument))
    ||  (!str_cmp("MUD!", argument))
    ||  (!str_cmp("Mud!", argument))
    ||  (!str_prefix("Mud", argument))
    ||  (!str_prefix("MUD", argument))
    ||  (!str_prefix("mud", argument)))
    &&  !head_god)
    {
    sprintf( log_buf, "[WARNING] %s pmotes: %s", ch->name, argument);
    log_string( log_buf );
    wiznet(log_buf,NULL,NULL,WIZ_MULTI,0,0);
    }
 
    act( "$n $t", ch, argument, NULL, TO_CHAR );

    for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
    {
	if (vch->desc == NULL || vch == ch)
	    continue;

	if ((letter = strstr(argument,vch->name)) == NULL)
	{
	    MOBtrigger = FALSE;
	    act("$N $t",vch,argument,ch,TO_CHAR);
	    MOBtrigger = TRUE;
	    continue;
	}

	strcpy(temp,argument);
	temp[strlen(argument) - strlen(letter)] = '\0';
   	last[0] = '\0';
 	name = vch->name;
	
	for (; *letter != '\0'; letter++)
	{ 
	    if (*letter == '\'' && matches == strlen(vch->name))
	    {
		strcat(temp,"r");
		continue;
	    }

	    if (*letter == 's' && matches == strlen(vch->name))
	    {
		matches = 0;
		continue;
	    }
	    
 	    if (matches == strlen(vch->name))
	    {
		matches = 0;
	    }

	    if (*letter == *name)
	    {
		matches++;
		name++;
		if (matches == strlen(vch->name))
		{
		    strcat(temp,"you");
		    last[0] = '\0';
		    name = vch->name;
		    continue;
		}
		strncat(last,letter,1);
		continue;
	    }

	    matches = 0;
	    strcat(temp,last);
	    strncat(temp,letter,1);
	    last[0] = '\0';
	    name = vch->name;
	}

	MOBtrigger = FALSE;
	act("$N $t",vch,temp,ch,TO_CHAR);
	MOBtrigger = TRUE;
    }
	
    return;
}

void do_smote(CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *vch;
    char *letter,*name;
    char last[MAX_INPUT_LENGTH], temp[MAX_STRING_LENGTH];
    int matches = 0;
	int head_god = FALSE;

    if ( !IS_NPC(ch) && IS_SET(ch->comm, COMM_NOEMOTE) )
    {
        send_to_char( "You can't show your emotions.\n\r", ch );
        return;
    }
 
	if(!IS_NPC(ch) && IS_SET(ch->comm,COMM_NOCHANNELS) )
	{
		send_to_char("Your channel privileges have been revoked.\n\r",ch);
		return;
	}

    if ( argument[0] == '\0' )
    {
        send_to_char( "Emote what?\n\r", ch );
        return;
    }
 
     head_god = !str_cmp(ch->name, HEAD_GOD) 
            || !str_cmp(ch->name,HEAD_GOD2) 
            || !str_cmp(ch->name, HEAD_GOD3);

    if (((!str_infix("mud", argument))
    ||  (!str_infix("MUD", argument))
    ||  (!str_infix("MuD", argument))
    ||  (!str_infix("MUd", argument))
    ||  (!str_infix("mUD", argument))
    ||  (!str_infix("muD", argument))
    ||  (!str_infix("Mud", argument))
    ||  (!str_infix("mUd", argument))
    ||  (!str_suffix("mud", argument))
    ||  (!str_suffix("MUD", argument))
    ||  (!str_suffix("Mud", argument))
    ||  (!str_suffix("mud.", argument))
    ||  (!str_suffix("MUD.", argument))
    ||  (!str_suffix("Mud.", argument))
    ||  (!str_suffix("mud!", argument))
    ||  (!str_suffix("MUD!", argument))
    ||  (!str_suffix("Mud!", argument))
    ||  (!str_cmp("mud", argument))
    ||  (!str_cmp("mud,", argument))
    ||  (!str_cmp("MUD,", argument))
    ||  (!str_cmp("Mud,", argument))
    ||  (!str_cmp("mud.", argument))
    ||  (!str_cmp("MUD.", argument))
    ||  (!str_cmp("Mud.", argument))
    ||  (!str_cmp("mud!", argument))
    ||  (!str_cmp("MUD!", argument))
    ||  (!str_cmp("Mud!", argument))
    ||  (!str_prefix("Mud", argument))
    ||  (!str_prefix("MUD", argument))
    ||  (!str_prefix("mud", argument)))
    &&  !head_god)
    {
    sprintf( log_buf, "[WARNING] %s smotes: %s", ch->name, argument);
    log_string( log_buf );
    wiznet(log_buf,NULL,NULL,WIZ_MULTI,0,0);
    }
   
    if (strstr(argument,ch->name) == NULL)
    {
	send_to_char("You must include your name in an smote.\n\r",ch);
	return;
    }
   
    send_to_char(argument,ch);
    send_to_char("\n\r",ch);
 
    for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
    {
        if (vch->desc == NULL || vch == ch)
            continue;
 
        if ((letter = strstr(argument,vch->name)) == NULL)
        {
	    send_to_char(argument,vch);
	    send_to_char("\n\r",vch);
            continue;
        }
 
        strcpy(temp,argument);
        temp[strlen(argument) - strlen(letter)] = '\0';
        last[0] = '\0';
        name = vch->name;
 
        for (; *letter != '\0'; letter++)
        {
            if (*letter == '\'' && matches == strlen(vch->name))
            {
                strcat(temp,"r");
                continue;
            }
 
            if (*letter == 's' && matches == strlen(vch->name))
            {
                matches = 0;
                continue;
            }
 
            if (matches == strlen(vch->name))
            {
                matches = 0;
            }
 
            if (*letter == *name)
            {
                matches++;
                name++;
                if (matches == strlen(vch->name))
                {
                    strcat(temp,"you");
                    last[0] = '\0';
                    name = vch->name;
                    continue;
                }
                strncat(last,letter,1);
                continue;
            }
 
            matches = 0;
            strcat(temp,last);
            strncat(temp,letter,1);
            last[0] = '\0';
            name = vch->name;
        }
 
	send_to_char(temp,vch);
	send_to_char("\n\r",vch);
    }
 
    return;
}


/*
 * All the posing stuff.
 */
struct	pose_table_type
{
    char *	message[2*MAX_CLASS];
};

const	struct	pose_table_type	pose_table	[]	=
{
    {
	{
	    "You draw a battle-ward in the sand, to bring good fortune in war.",
	    "$n draws some sort of arcane ward in the sand with $s blade.",
	    "You mumble a prayer of Mishakal, the Goddess of Healing.",
	    "$n mumbles a prayer, speaking the name of Mishakal.",
	    "You perform a small card trick.",
	    "$n performs a small card trick.",
          "You recount an old dwarven battle-hymn you learned in Thorbardin.",
          "$n recounts an old battle-hymn that sounds dwarvish in origin.",
	    "You work your audience, attempting to be your usual, charming, self.",
	    "You find $n to be a terribly charming individual.", 
          "You brandish your family crest as you swear allegiance to god and country.",
          "$n brandishes $s family crest and swears allegiance to god and country.",
          "You stretch gracefully, showing off your lean physique.",
          "$n stretches gracefully, showing off $s lean physique.",
	    "You show your bulging muscles.",
	    "$n shows $s bulging muscles.",
	    "You brandish a sprig of mistletoe in preparation for your next encounter.",
	    "$n brandishes a sprig of mistletoe in preparation for $s next encounter.",
	    "You begin pondering who will die by your blade next.",
	    "$n begins contemplating as to who will die by $s blade next. Will it be you?",
          "You wave your dagger in the pattern of a magical ward.",
          "$n waves $s dagger in the pattern of a magical ward.",
	    "You cast bones from a small animal onto the ground and read them.  ",
	    "$n casts bones from a small animal onto the ground and reads them.",
          "You ponder the physics involved in your next incantation.",
          "$n seems to be deep on thought on the intricacies of magic.",
	    "You create a small illusion with a gold coin, pulling it out of nowhere.",
	    "$n pulls a gold coin seemingly out of nowhere.",
          "You make a few notes in your spellbook.",
          "$n makes some notes in $s spellbook.",
	    "You carefully clean and polish your tools.",
	    "$n carefully cleans and polishes $s tools.",
	}
    },

    {
	{
          "You chant an ancient, arcane battle mantra.",
          "$n chants an ancient, arcane battle mantra.",
	    "You ponder your destiny as you watch the grey clouds shift overhead.",
	    "$n gets a faraway look in $s eyes as the grey clouds shift overhead.",
	    "You wiggle your ears alternately.",
	    "$n wiggles $s ears alternately.",
	    "You crack nuts between your fingers.",
	    "$n cracks nuts between $s fingers.",
 	    "You idly flip through the pages of your songbook.",
	    "$n idly flips through the pages of $s songbook.",
          "You loudly proclaim your loyalty to truth and justice.",
          "$n loudly proclaim $s loyalty to truth and justice.",
	    "You fiercely stress the importance of strength in both mind and body.",
	    "$n fiercely stresses the importance of strength in both mind and body.",
	    "You crack nuts between your fingers.",
	    "$n cracks nuts between $s fingers.",
	    "You lick your finger and test the air for wind direction.",
	    "$n licks $s finger and tests the air for wind direction.",
          "You slowly trace a finger along one of your old battle scars.",
          "$n slowly traces a finger along one of $s old battle scars.",
	    "You wiggle your ears alternately.",
	    "$n wiggles $s ears alternately.",
	    "You begin mixing ashes with blood in a bowl, preparing for your next encounter.",
	    "$n prepares for $s next encounter, mixing blood with ashes into a bowl.",
	    "You write carefully in your spellbook.",
	    "$n writes carefully in $s spellbook.",
	    "You turn into a butterfly, then return to your normal shape.",
	    "$n turns into a butterfly, then returns to $s normal shape.",
	    "You trace a conjuring circle around you.",
	    "$n traces a conjuring circle around $mself.",
	    "You grab a polishing rag and begin buffing some armor plates.",
	    "$n grabs a polishing rag and begins to buff some armor plates.",
	}
    },

    {
	{
	    "You mumble some arcane words of magic in preparation for battle.",
	    "$n mumbles some arcane words of magic in preparation for battle.",
	    "You gently claim that comfort comes to those with faith.",
	    "$n gently claims that comfort comes to those with faith.",
	    "You pull the hood of your cloak deeply over your eyes.",
	    "$n pulls the hood of $s cloak deeply over $s eyes.",
          "You carefully check your food and water rations.",
          "$n carefully checks $s food and water rations.",
	    "You suddenly break into a song and dance.",
	    "$n suddenly breaks into a song and dance.",
	    "You grizzle your teeth and look mean.",
	    "$n grizzles $s teeth and looks mean.",
	    "You swear allegiance to your clan and tribe in your native tongue.",
	    "$n swears allegiance to $s clan and tribe in $s native tongue.",
	    "A halo appears over your head.",
	    "A halo appears over $n's head.",
	    "You kneel down and begin chatting with a small rabbit.",
	    "$n bends down and begins to chat with a small rabbit. Is $e senile?",
	    "You grizzle your teeth and look mean.",
	    "$n grizzles $s teeth and looks mean.",
	    "You nimbly tie yourself into a knot.",
	    "$n nimbly ties $mself into a knot.",
	    "You extract a skull from your pocket and measure its dimensions.",
	    "$n extracts a skull from $s pocket and measures its dimensions.",
	    "You close your eyes and commune with the elemental forces of Krynn.",
          "$n seems to be at one with the elemental forces of Krynn.",
	    "You stand quietly.",
	    "$n stands quietly.",
          "You smile mysteriously, your eyes sparking with the flame of magic.",
          "$n smiles mysteriously, $s eyes sparking with the flame of magic.",
	    "You carefully oil and condition some leather scraps.",
	    "$n carefully oils and conditions some leather scraps.",
	}
    },

    {
	{
	    "Little red lights dance in your eyes.",
	    "Little red lights dance in $n's eyes.",
	    "You recite words of wisdom.",
	    "$n recites words of wisdom.",
	    "You juggle with daggers, apples, and eyeballs.",
	    "$n juggles with daggers, apples, and eyeballs.",
	    "You hit your head, and your eyes roll.",
	    "$n hits $s head, and $s eyes roll.",
	    "In a flash of inspiration, you quickly jot down a brief verse.",
	    "In a flash of inspiration, $n quickly jots down a brief verse.",
          "You skillfully clean the blood from your weapon.",
          "$n skillfully cleans the blood from $s weapon.",
          "You loudly dismiss the use of body armor as weak and cowardly.",
          "$n loudly dismisses the use of body armor as weak and cowardly.",
	    "You hit your head, and your eyes roll.",
	    "$n hits $s head, and $s eyes roll.",
	    "You take a delightful sip of a hearty, herbal tea.",
	    "An uplifting scent of herbs wafts past you as $n sips $s tea.",
	    "You ponder your destiny as you watch the grey clouds shift overhead.",
	    "$n gets a faraway look in $s eyes as the grey clouds shift overhead.",
	    "Little red lights dance in your eyes.",
	    "Little red lights dance in $n's eyes.",
	    "You chop off the head of a bat. Yum Yum!",
	    "$n chops off the head of a bat. Yum Yum!",
	    "You smile mysteriously, your eyes sparking with the flames of magic.",
	    "$n smiles mysteriously, $s eyes sparking witht he flames of magic.",
	    "You smile mysteriously, your eyes sparking with the flames of magic.",
	    "$n smiles mysteriously, $s eyes sparking witht he flames of magic.",
          "You smooth out the wrinkles in your flawless, flowing robes.",
          "$n smoothes out the wrinkles in $s flawless, flowing robes.",
	    "Grabbing a stack of shields, you begin beating the dents out.",
	    "Grabbing a stack of shields, $n begins beating out the dents.",
	}
    },

    {
	{
	    "A slimy green monster appears before you and bows.",
	    "A slimy green monster appears before $n and bows.",
	    "Deep in prayer, you levitate.",
	    "Deep in prayer, $n levitates.",
	    "You steal the underwear off every person in the room.",
	    "Your underwear is gone!  $n stole it!",
	    "Crunch, crunch -- you munch a bottle.",
	    "Crunch, crunch -- $n munches a bottle.",
	    "You ponder your destiny as you watch the grey clouds shift overhead.",
	    "$n gets a faraway look in $s eyes as the grey clouds shift overhead.",
	    "You skillfully wipe off the blood from your armour.",
	    "$n skillfully wipes off the blood from $s armour.",
	    "You stand quietly.",
	    "$n stands quietly.",
	    "Crunch, crunch -- you munch a bottle.",
	    "Crunch, crunch -- $n munches a bottle.",
	    "You take a delightful sip of a hearty, herbal tea.",
	    "An uplifting scent of herbs wafts past you as $n sips $s tea.",
	    "You steal the underwear off every person in the room.",
	    "Your underwear is gone!  $n stole it!",
	    "A slimy green monster appears before you and bows.",
	    "A slimy green monster appears before $n and bows.",
	    "A slimy green monster appears before you and bows.",
	    "A slimy green monster appears before $n and bows.",
	    "A slimy green monster appears before you and bows.",
	    "A slimy green monster appears before $n and bows.",
	    "A slimy green monster appears before you and bows.",
	    "A slimy green monster appears before $n and bows.",
          "You quietly review a rumpled scroll before tucking it back in your robe.",
          "$n quietly reviews a rumpled scroll before tucking it back in $s robe.",
	    "You polish a shield to a mirror shine.",
	    "$n polishes a shield to a mirror shine.",
	}
    },

    {
	{
          "You carefully etch mystical runes into your armor.",
          "$n carefully etches mystical runes into $s armor.",
	    "An angel consults you.",
	    "An angel consults $n.",
          "You attempt to get a glimpse of everyone's magical items.",
          "You notice $n greedily appraising your magical items.",
          "You flex proudly, showing off your muscular physique.",
          "$n flexes proudly, showing off $s muscular physique..",
          "You recite a few verses of romantic poetry.",
          "$n recites a few verses of romantic poetry.",
          "You polish your already glowing armor.",
          "$n polish $s already glowing armor.",
	    "You grin as you lick the dried blood off your fingers.",
	    "$n grins as $e licks the dried blood off $s fingers.",
	    "... 98, 99, 100 ... you do pushups.",
	    "... 98, 99, 100 ... $n does pushups.",
	    "You sort through all your healing salves and herbal remedies.",
	    "$n sorts through all $s healing salves and herbal remedies.",
          "You appraise those around you with unbiased eyes.",
          "$n appraises you calmly, with unbiased eyes.",
	    "You turn everybody into a little pink elephant.",
	    "You are turned into a little pink elephant by $n.",
	    "You pull out a tattered scroll inked in blood and read quietly.",
	    "$n pulls out a tattered scroll inked in blood and reads quietly.",
	    "You turn everybody into a little pink elephant.",
	    "You are turned into a little pink elephant by $n.",
	    "You turn everybody into a little pink elephant.",
	    "You are turned into a little pink elephant by $n.",
          "You ponder the physics involved in your next incantation.",
          "$n seems to be deep on thought on the intricacies of magic.",
	    "You begin beating some boiled leather sheets into shape.",
	    "$n begins beating some boiled leather sheets into shape.",
	}
    },

    {
	{
	    "A small ball of light dances on your fingertips.",
	    "A small ball of light dances on $n's fingertips.",
          "Deep in prayer, you commune with your diety.",
          "Deep in prayer, $n communes with $s diety.",
	    "You count the money in everyone's pockets.",
	    "Check your money, $n is counting it.",
          "You chant an ancient, arcane battle mantra.",
          "$n chants an ancient, arcane battle mantra.",
	    "A small ball of light dances on your fingertips.",
	    "A small ball of light dances on $n's fingertips.",
          "You move your weapon slowly in a graceful practice maneuver.",
          "$n moves $s weapon slowly in a graceful practice maneuver.",
	    "You add another notch in your belt of death.",
	    "With an evil grin, $n adds another notch to $s belt.",
	    "Arnold Schwarzenegger admires your physique.",
	    "Arnold Schwarzenegger admires $n's physique.",
	    "You examine the nearby flora, and gather a specimen for your later use.",
	    "$n examines the nearby flora and collects a small sample.",
	    "Your body glows with an unearthly light.",
	    "$n's body glows with an unearthly light.",
	    "A small ball of light dances on your fingertips.",
	    "A small ball of light dances on $n's fingertips.",
	    "You chant softly in demonic tongues in an attempt to gain unholy favor.",
	    "$n chants softly in demonic tongues in an attempt to gain unholy favor.",
	    "You work your audience, attempting to be your usual, charming, self.",
	    "You find $n to be a terribly charming individual.", 
          "You ponder the physics involved in your next incantation.",
          "$n seems to be deep on thought on the intricacies of magic.",
	    "You stand quietly.",
	    "$n stands quietly.",
	    "You grab a knife and begin cutting a pattern into a side of leather.",
	    "$n grabs a knife and begins to cut a pattern into a side of leather.",
	}
    },

    {
	{
	    "Smoke and fumes leak from your nostrils.",
	    "Smoke and fumes leak from $n's nostrils.",
	    "You kneel briefly, praying for forgivness for your sins.",
	    "$n suddenly kneels, praying for forgiveness for $s sins.",
	    "You balance a pocket knife on your tongue.",
	    "$n balances a pocket knife on your tongue.",
	    "Watch your feet, you are juggling granite boulders.",
	    "Watch your feet, $n is juggling granite boulders.",
          "You humbly acclaim yourself as one of the best storysingers in all of Krynn.",
	    "$n brags about being the best storysinger in all of Krynn.", 
          "You pray for the Gods of War to favor your cause.",
          "$n prays for the Gods of War to favor $s cause.",
	    "You balance a pocket knife on your tongue.",
	    "$n balances a pocket knife on your tongue.",
	    "Watch your feet, you are juggling granite boulders.",
	    "Watch your feet, $n is juggling granite boulders.",
	    "You bend down and touch the ground, thanking her for all she provides.",
	    "$n bends down and touches the ground, thanking her for all she provides.",
	    "You idly whittle an animal bone into a keen dagger.",
	    "$n idly whittles an animal bone into a keen dagger.",
	    "Smoke and fumes leak from your nostrils.",
	    "Smoke and fumes leak from $n's nostrils.",
	    "You take a dagger and slice your finger, dripping the blood into a vial.",
	    "$n takes a dagger and slices $s finger, dripping the blood into a vial.",
	    "Smoke and fumes leak from your nostrils.",
	    "Smoke and fumes leak from $n's nostrils.",
	    "You fold your hands in a moment of silent reflection.",
	    "$n folds $s hands in a moment of silent reflection.",
	    "Smoke and fumes leak from your nostrils.",
	    "Smoke and fumes leak from $n's nostrils.",
	    "Your anvil rings as you restore a suit of armor to it's former glory.",
	    "$n's anvil rings as $e restores a suit of armor to it's former glory.",
	}
    },

    {
	{
	    "The light flickers as you rap in magical languages.",
	    "The light flickers as $n raps in magical languages.",
	    "Everyone levitates as you pray.",
	    "You levitate as $n prays.",
	    "You produce a coin from everyone's ear.",
	    "$n produces a coin from your ear.",
	    "You crudely apply war paint in preparation of battle.",
          "$n crudely applies war paint on $s face in primitive designs.",
	    "In a flash of inspiration, you quickly jot down a brief verse.",
	    "In a flash of inspiration, $n quickly jots down a brief verse.",
	    "You check to make sure your weapon is properly balanced.",
	    "$n checks to make sure $s weapon is properly balanced.",
	    "You crudely apply war paint in preparation of battle.",
          "$n crudely applies war paint on $s face in primitive designs.",
	    "Oomph!  You squeeze water out of a granite boulder.",
	    "Oomph!  $n squeezes water out of a granite boulder.",
	    "You offer some food to a passing rodent before it skitters off.",
	    "A passing rodent stops to nibble some food offered by $n, then skitters off.",
          "You slowly run your finger along the blade of your dagger.",
          "$n slowly runs $s finger along the blade of $s dagger.",
	    "You mumble some arcane words of magic in preparation for battle.",
	    "$n mumbles some arcane words of magic in preparation for battle.",
	    "You softly chant dark vows and sign mystic runes in the air.",
	    "$n softly chants and makes mystic symbols in the air.",
          "You recite words of wisdom.",
          "$n recites words of wisdom.",
          "You smooth out the wrinkles in your flawless, flowing robes.",
          "$n smoothes out the wrinkles in $s flawless, flowing robes.",
          "You smooth out the wrinkles in your flawless, flowing robes.",
          "$n smoothes out the wrinkles in $s flawless, flowing robes.",
	    "You carefully recondition the straps on a set of greaves.",
	    "$n carefully reconditions the straps on a set of greaves.",
	}
    },

    {
	{
          "You place runestones along the seams of your armour.",
          "$n places runestones along the seams of $s armour.",
	    "A cool breeze refreshes you.",
	    "A cool breeze refreshes $n.",
	    "You step behind your shadow.",
	    "$n steps behind $s shadow.",
          "You vow to banish all evil from the fair land of Krynn.",
          "$n vows to banish all evil from the fair land of Krynn.",
 	    "You idly flip through the pages of your songbook.",
	    "$n idly flips through the pages of $s songbook.",
	    "A cool breeze refreshes you.",
	    "A cool breeze refreshes $n.",
          "You skillfully clean the blood from your weapon.",
          "$n skillfully cleans the blood from $s weapon.",
	    "You pick your teeth with a spear.",
	    "$n picks $s teeth with a spear.",
	    "You snap a twig, determining the recent drought conditions.",
	    "$n snaps a twig in order to determine recent drought conditions.",
	    "You pick your teeth with a spear.",
	    "$n picks $s teeth with a spear.",
	    "Your head disappears.",
	    "$n's head disappears.",
	    "You grin eerily, your face a mask devoid of all emotion.",
          "$n grins eerily, $s face a mask devoid of all emotion.",
	    "Your head disappears.",
	    "$n's head disappears.",
	    "Your head disappears.",
	    "$n's head disappears.",
          "You idly shuffle a deck of tarot cards.",
          "$n idly shuffles a deck of tarot cards.",
	    "You carefully examine the hinge plates on a new set of pauldrons.",
	    "$n carefully examines the hinge plates on a new set of pauldrons.",
	}
    },

    {
	{
	    "A fire elemental singes your hair.",
	    "A fire elemental singes $n's hair.",
	    "The sun pierces through the clouds to illuminate you.",
	    "The sun pierces through the clouds to illuminate $n.",
	    "Your eyes dance with greed.",
	    "$n's eyes dance with greed.",
	    "Everyone is swept off their feet by your hug.",
	    "You are swept off your feet by $n's hug.",
	    "A fire elemental singes your hair.",
	    "A fire elemental singes $n's hair.",
	    "You ponder your destiny as you watch the grey clouds shift overhead.",
	    "$n gets a faraway look in $s eyes as the grey clouds shift overhead.",
	    "Your eyes dance with greed.",
	    "$n's eyes dance with greed.",
	    "Everyone is swept off their foot by your hug.",
	    "You are swept off your feet by $n's hug.",
	    "You take a delightful sip of a hearty, herbal tea.",
	    "An uplifting scent of herbs wafts past you as $n sips $s tea.",
	    "Everyone is swept off their foot by your hug.",
	    "You are swept off your feet by $n's hug.",
	    "A fire elemental singes your hair.",
	    "A fire elemental singes $n's hair.",
	    "You slice your skin, offering your blood to the dark gods.",
	    "$n slits $s skin, offering blood to the dark gods.",
	    "A fire elemental singes your hair.",
	    "A fire elemental singes $n's hair.",
	    "A fire elemental singes your hair.",
	    "A fire elemental singes $n's hair.",
	    "A fire elemental singes your hair.",
	    "A fire elemental singes $n's hair.",
	    "You remove the rust from a chainmail shirt with a stout wire brush.",
	    "$n removes the rust from a chainmail shirt with a stout wire brush.",
	}
    },

    {
	{
	    "The sky changes color to match your eyes.",
	    "The sky changes color to match $n's eyes.",
	    "You pray to the pantheon as an entity, that each fulfill his destiny.",
	    "$n prays for the destiny of all gods to be fulfilled.",
	    "You deftly steal everyone's weapon.",
	    "$n deftly steals your weapon.",
	    "Your karate chop splits a tree.",
	    "$n's karate chop splits a tree.",
	    "The sky changes color to match your eyes.",
	    "The sky changes color to match $n's eyes.",
	    "The ocean parts before you.",
	    "The ocean parts before $n.",
          "You gruffly declare that civilization is the bane of independence.",
          "With a crude grunt, $n declares that civilization is the bane of independence.",
	    "Your karate chop splits a tree.",
	    "$n's karate chop splits a tree.",
	    "You close your eyes for a moment, absorbing the pulse of nature.",
	    "$n seems to be lost for a moment in the rythm of nature.",
	    "Your karate chop splits a tree.",
	    "$n's karate chop splits a tree.",
	    "The sky changes color to match your eyes.",
	    "The sky changes color to match $n's eyes.",
	    "You chant softly in demonic tongues in an attempt to gain unholy favor.",
	    "$n chants softly in demonic tongues in an attempt to gain unholy favor.",
          "You ponder the physics involved in your next incantation.",
          "$n seems to be deep on thought on the intricacies of magic.",
	    "The sky changes color to match your eyes.",
	    "The sky changes color to match $n's eyes.",
          "You ponder the physics involved in your next incantation.",
          "$n seems to be deep on thought on the intricacies of magic.",
	    "With a set of pliers, you set about replacing the broken links of a chain coif.",
	    "With a set of pliers, $n sets about replacing the broken links of a chain coif.",
	}
    },

    {
	{
	    "The stones dance to your command.",
	    "The stones dance to $n's command.",
	    "A thunder cloud kneels to you.",
	    "A thunder cloud kneels to $n.",
          "You greedily count your gold pieces.",
          "$n greedily counts $s gold pieces.",
	    "A strap of your armor breaks over your mighty thews.",
	    "A strap of $n's armor breaks over $s mighty thews.",
	    "The stones dance to your command.",
	    "The stones dance to $n's command.",
	    "A thunder cloud kneels to you.",
	    "A thunder cloud kneels to $n.",
	    "The Grey Mouser buys you a beer.",
	    "The Grey Mouser buys $n a beer.",
	    "A strap of your armor breaks over your mighty thews.",
	    "A strap of $n's armor breaks over $s mighty thews.",
	    "The stones dance to your command.",
	    "The stones dance to $n's command.",
	    "A thunder cloud kneels to you.",
	    "A thunder cloud kneels to $n.",
	    "The stones dance to your command.",
	    "The stones dance to $n's command.",
	    "The stones dance to your command.",
	    "The stones dance to $n's command.",
	    "The stones dance to your command.",
	    "The stones dance to $n's command.",
	    "The stones dance to your command.",
	    "The stones dance to $n's command.",
	    "The stones dance to your command.",
	    "The stones dance to $n's command.",
	    "After a liberal dose of oiling, you test a suit of chainmail for flexibility.",
	    "After a liberal dose of oiling, $n tests a suit of chainmail for flexibility.",
	}
    },

    {
	{
	    "The heavens and grass change colour as you smile.",
	    "The heavens and grass change colour as $n smiles.",
	    "The Burning Man speaks to you.",
	    "The Burning Man speaks to $n.",
	    "Everyone's pocket explodes with your fireworks.",
	    "Your pocket explodes with $n's fireworks.",
	    "A boulder cracks at your frown.",
	    "A boulder cracks at $n's frown.",
	    "The heavens and grass change colour as you smile.",
	    "The heavens and grass change colour as $n smiles.",
	    "You curse Hiddukel as nothing more than a filthy beast of burden.",
	    "$n curses Hiddukel as nothing more than a filthy beast of burden.",
	    "You chant a tribal battle-hymn.",
	    "A somber, tribal battle-chant emerges from $n.",
	    "A boulder cracks at your frown.",
	    "A boulder cracks at $n's frown.",
	    "You look off into the wild blue yonder.",
	    "$n looks off into the wild blue yonder.",
	    "A boulder cracks at your frown.",
	    "A boulder cracks at $n's frown.",
	    "The heavens and grass change colour as you smile.",
	    "The heavens and grass change colour as $n smiles.",
	    "The heavens and grass change colour as you smile.",
	    "The heavens and grass change colour as $n smiles.",
	    "The heavens and grass change colour as you smile.",
	    "The heavens and grass change colour as $n smiles.",
	    "The heavens and grass change colour as you smile.",
	    "The heavens and grass change colour as $n smiles.",
	    "The heavens and grass change colour as you smile.",
	    "The heavens and grass change colour as $n smiles.",
	    "Sparks fly as you grind the rough edges off of a well-used blade.",
	    "Sparks fly as $n grinds the rough edges off of a well-used blade.",
	}
    },

    {
	{
          "You carefully etch mystical runes into your armor.",
          "$n carefully etches mystical runes into $s armor.",
          "You proudly display the heraldry of your diety.",
          "$n proudly displays the heraldry of $s diety.",
	    "Everyone discovers your dagger a centimeter from their eye.",
	    "You discover $n's dagger a centimeter from your eye.",
	    "You curse Hiddukel as nothing more than a filthy beast of burden.",
	    "$n curses Hiddukel as nothing more than a filthy beast of burden.",
	    "You hum an old village folk song.",
	    "$n hums an old village folk song.",
          "You polish your already glowing armor.",
          "$n polish $s already glowing armor.",
          "You flex proudly, showing off your muscular physique.",
          "$n flexes proudly, showing off $s muscular physique..",
	    "Mercenaries arrive to do your bidding.",
	    "Mercenaries arrive to do $n's bidding.",
	    "You close your eyes for a moment, absorbing the pulse of nature.",
	    "$n seems to be lost for a moment in the rythm of nature.",
	    "Mercenaries arrive to do your bidding.",
	    "Mercenaries arrive to do $n's bidding.",
	    "Everyone's clothes are transparent, and you are laughing.",
	    "Your clothes are transparent, and $n is laughing.",
	    "You quietly examine finger bones.",
	    "$n quietly examine finger bones.",
	    "You carefully brush the dust off your spellbook.",
	    "$n carefully brushes the dust off $s spellbook.",
	    "You write carefully in your spellbook.",
	    "$n writes carefully in $s spellbook.",
	    "You write carefully in your spellbook.",
	    "$n writes carefully in $s spellbook.",
	    "You wipe the sweat from your brow.",
	    "$n wipes the sweat from $s brow.",
	}
    },

    {
	{
	    "A black hole swallows you.",
	    "A black hole swallows $n.",
          "You vow to spread the word and truth of your diety.",
          "$n vows to spread the word and truth of $s diety.",
          "You meticulously examine your quality lock picks.",
          "$n meticulously examines $s quality lock picks.",
	    "Four matched Percherons bring in your chariot.",
	    "Four matched Percherons bring in $n's chariot.",
 	    "You idly flip through the pages of your songbook.",
	    "$n idly flips through the pages of $s songbook.",
          "You loudly proclaim your loyalty to truth and justice.",
          "$n loudly proclaim $s loyalty to truth and justice.",
          "You slowly trace a finger along one of your old battle scars.",
          "$n slowly traces a finger along one of $s old battle scars.",
	    "Four matched Percherons bring in your chariot.",
	    "Four matched Percherons bring in $n's chariot.",
	    "Where did you go?",
	    "Where did $n go?",
	    "Four matched Percherons bring in your chariot.",
	    "Four matched Percherons bring in $n's chariot.",
          "You make a few notes in your spellbook.",
          "$n makes some notes in $s spellbook.",
	    "A black hole swallows you.",
	    "A black hole swallows $n.",
	    "You boast the superiority of the school of enchantment.",
  	    "$n boasts the superiority of the school of enchantment.",
	    "You talk about the principle that life is an illusion.",
	    "$n talks about the principle that life is an illusion.",
	    "A black hole swallows you.",
	    "A black hole swallows $n.",
	    "You begin to pump your bellows furiously.",
	    "$n begins to pump $s bellows furiosly.",
	}
    },

    {
	{
	    "The world shimmers in time with your whistling.",
	    "The world shimmers in time with $n's whistling.",
	    "The gods give you a staff.",
	    "The gods give $n a staff.",
          "You slowly run your finger along the blade of your dagger.",
          "$n slowly runs $s finger along the blade of $s dagger.",
	    "You fiercely stress the importance of strength in both mind and body.",
	    "$n fiercely stresses the importance of strength in both mind and body.",
	    "The world shimmers in time with your whistling.",
	    "The world shimmers in time with $n's whistling.",
	    "The gods give you a staff.",
	    "The gods give $n a staff.",
	    "Click.",
	    "Click.",
	    "Atlas asks you to relieve him.",
	    "Atlas asks $n to relieve him.",
          "You carefully check your food and water rations.",
          "$n carefully checks $s food and water rations.",
          "You loudly dismiss the use of body armor as weak and cowardly.",
          "$n loudly dismisses the use of body armor as weak and cowardly.",
          "You toss a gold coin over your shoulder for good luck.",
          "$n tosses a gold coin over $s shoulder for good luck.",
	    "The world shimmers in time with your whistling.",
	    "The world shimmers in time with $n's whistling.",
	    "You pull out a tattered scroll inked in blood and read quietly.",
	    "$n pulls out a tattered scroll inked in blood and reads quietly.",
	    "The world shimmers in time with your whistling.",
	    "The world shimmers in time with $n's whistling.",
	    "You pull out a tattered scroll inked in blood and read quietly.",
	    "$n pulls out a tattered scroll inked in blood and reads quietly.",
	    "You hold up your newest weapon and show it off.",
	    "$n holds up $s newest weapon and shows it off.",
	}
    }
};



void do_pose( CHAR_DATA *ch, char *argument )
{
    int level;
    int pose;

    if ( IS_NPC(ch) )
	return;

    level = UMIN( ch->level, sizeof(pose_table) / sizeof(pose_table[0]) - 1 );
    pose  = number_range(0, level);

    act( pose_table[pose].message[2*ch->class+0], ch, NULL, NULL, TO_CHAR );
    act( pose_table[pose].message[2*ch->class+1], ch, NULL, NULL, TO_ROOM );

    return;
}



void do_bug( CHAR_DATA *ch, char *argument )
{
    if ( argument[0] == '\0' )
    {
     send_to_char( "\n\rUsage: bug <message> (your location is automatically recorded)\n\r", ch );
     return;
    }
    append_file( ch, BUG_FILE, argument );
    send_to_char( "Thank you, your BUG notice has been recorded.\n\r", ch );
    return;
}

void do_typo( CHAR_DATA *ch, char *argument )
{
    if ( argument[0] == '\0' )
    {
      send_to_char( "\n\rUsage: typo <message> (your location is automatically recorded)\n\r", ch );
      return;
    }
    append_file( ch, TYPO_FILE, argument );
    send_to_char( "Thank you, your TYPO notice has been recorded.\n\r", ch );
    return;
}

void do_rent( CHAR_DATA *ch, char *argument )
{
    send_to_char( "There is no rent here.  Just save and quit.\n\r", ch );
    return;
}


void do_qui( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to QUIT, you have to spell it out.\n\r", ch );
    return;
}

void do_quit( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d,*d_next;
    char strsave[MAX_INPUT_LENGTH];
    CHAR_DATA *vch;
    int id;

    if ( IS_NPC(ch) )
	return;

    if ( ch->position == POS_FIGHTING )
    {
	send_to_char( "No way! You are fighting.\n\r", ch );
	return;
    }

  if (ch->level < 2)
   {
     if (ch->pcdata->confirm_delete)
     {
	if (argument[0] != '\0')
	  {
	    send_to_char("Quit status removed.\n\r",ch);
	    ch->pcdata->confirm_delete = FALSE;
	    return;
	}
	else
	{
    	    sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( ch->name ) );
	    wiznet("$N turns $Mself into line noise.",ch,NULL,0,0,0);
	    stop_fighting(ch,TRUE);
    	    save_char_obj( ch );
   	    id = ch->id;
    	    d = ch->desc;
   	    extract_char( ch, TRUE );
   	    if ( d != NULL )
    	    close_socket( d );
	    unlink(strsave);
	    return;
 	}
    }

    if (argument[0] != '\0')
    {
	send_to_char("Just type quit. No argument.\n\r",ch);
	return;
    }

    send_to_char("Remember, your character won't be saved until you reach level two.\n\r",ch);
    send_to_char("If you are sure you want to quit and lose this character, type quit again.\n\r",ch);
    send_to_char("Type 'quit no' or any other argument, to cancel quit.\n\r",ch);
    ch->pcdata->confirm_delete = TRUE;
    wiznet("$N is contemplating deletion.",ch,NULL,0,0,get_trust(ch));
    return;
  }

    if ( ch->quit_timer < 0 && !IS_IMMORTAL(ch)) 
    {
	send_to_char("Your heart is beating too fast after the fight to quit now.\n",ch);
	return;
    }

    if ( ch->position  < POS_STUNNED  )
    {
	send_to_char( "You're not DEAD yet.\n\r", ch );
	return;
    }

    if ( IS_AFFECTED2(ch, AFF_CAMOUFLAGE) )
    {
        REMOVE_BIT( ch->affected2_by, AFF_CAMOUFLAGE );
        act("You leave your cover.", ch, NULL, NULL, TO_CHAR);
        act("$n leaves their cover.", ch, NULL, NULL, TO_ROOM);
    }

    if (IS_AFFECTED(ch, AFF_HIDE) && !IS_AFFECTED(ch, AFF_SNEAK) )
    {
        REMOVE_BIT( ch->affected_by, AFF_HIDE );
        act("You step out of the shadows.", ch, NULL, NULL, TO_CHAR);
        act("$n steps out of the shadows.", ch, NULL, NULL, TO_ROOM);
    }

    if( IS_SET(ch->comm, COMM_MASKEDIP) )
    {
      REMOVE_BIT(ch->comm, COMM_MASKEDIP);
      ch->maskedip = NULL;
    }

    if ( is_affected(ch,skill_lookup("blessed warmth")) )
    {
      affect_strip(ch,skill_lookup("blessed warmth"));
    }

    
    if (!IS_NPC(ch)
    && (!str_cmp(ch->pcdata->lokprot, "on")))
    {
      ch->pcdata->lokprot = "off";
    }

/* Stop gaurding quitters */
    if ( (ch->guarding != NULL)
    || (ch->guarded != NULL) )
      do_guard( ch, "\0");

    send_to_char( "Your vision becomes hazy as the legendary realm of Krynn slips away.\n\r",ch);
    act( "In a foggy haze, $n departs the legendary realm of Krynn.", ch, NULL, NULL, TO_ROOM );
    sprintf( log_buf, "%s has quit.", ch->name );
    log_string( log_buf );
    wiznet("$N rejoins the real world.",ch,NULL,WIZ_LOGINS,0,get_trust(ch));
    if ( IS_SET(ch->act, PLR_WIZINVIS) )
    {
        REMOVE_BIT(ch->act, PLR_WIZINVIS);
        ch->invis_level = 0;
    }
    for (vch=char_list;vch != NULL;vch = vch->next) 
    if (is_affected(vch,gsn_doppelganger) && vch->doppel == ch) 
    {
     send_to_char("Your victim has left, you are no longer doppelganged.\n\r", vch);
     affect_strip(vch,gsn_doppelganger);
    }

    /*
     * After extract_char the ch is no longer valid!
     */
    save_char_obj( ch );
    id = ch->id;
    d = ch->desc;
    extract_char( ch, TRUE );
    if ( d != NULL )
    close_socket( d );

    /* toast evil cheating bastards */
    for (d = descriptor_list; d != NULL; d = d_next)
    {
	CHAR_DATA *tch;

	d_next = d->next;
	tch = d->original ? d->original : d->character;
	if (tch && tch->id == id)
	{
	    extract_char(tch,TRUE);
	    close_socket(d);
	} 
    }

    return;
}

void do_save( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) )
	return;

    if ( ch->level < 2 )
    {
       send_to_char("You must be level 2 to save!\n\r", ch);
       return;
    }
    else
    {
       save_char_obj( ch );
       send_to_char("Astinus logs your journeys into the book of Krynn.\n\r", ch);
       WAIT_STATE(ch,4 * PULSE_VIOLENCE);
       return;
    }
}



void do_follow( CHAR_DATA *ch, char *argument )
{
/* RT changed to allow unlimited following and follow the NOFOLLOW rules */
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Follow whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, NULL, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( (IS_AFFECTED(ch, AFF_CHARM) && ch->master != NULL)
	|| (is_affected(ch, gsn_serenade) && ch->master != NULL) )
    {
	act( "But you'd rather follow $N!", ch, NULL, ch->master, TO_CHAR );
	return;
    }

    if ( victim == ch )
    {
	if ( ch->master == NULL )
	{
	    send_to_char( "You already follow yourself.\n\r", ch );
	    return;
	}
	stop_follower(ch);
	return;
    }

    if (!IS_NPC(victim) && IS_SET(victim->act,PLR_NOFOLLOW) && !IS_IMMORTAL(ch))
    {
	act("$N doesn't seem to want any followers.\n\r",
             ch,NULL,victim, TO_CHAR);
        return;
    }

    REMOVE_BIT(ch->act,PLR_NOFOLLOW);
    
    if ( ch->master != NULL )
	stop_follower( ch );

    add_follower( ch, victim );
    return;
}


void add_follower( CHAR_DATA *ch, CHAR_DATA *master )
{
    if ( ch->master != NULL )
    {
	bug( "Add_follower: non-null master.", 0 );
	return;
    }

    ch->master        = master;
    ch->leader        = NULL;

    if ( can_see( master, ch ) )
	act( "$n now follows you.", ch, NULL, master, TO_VICT );

    act( "You now follow $N.",  ch, NULL, master, TO_CHAR );

    return;
}



void stop_follower( CHAR_DATA *ch )
{
    if ( ch->master == NULL )
    {
	bug( "Stop_follower: null master.", 0 );
	return;
    }

   if (ch->fight_pos != FIGHT_FRONT)
       ch->fight_pos = FIGHT_FRONT;

    if ( IS_AFFECTED(ch, AFF_CHARM) )
    {
	REMOVE_BIT( ch->affected_by, AFF_CHARM );
	affect_strip( ch, gsn_charm_person );
    }

    if ( can_see( ch->master, ch ) && ch->in_room != NULL)
    {
	act( "$n stops following you.",     ch, NULL, ch->master, TO_VICT    );
    	act( "You stop following $N.",      ch, NULL, ch->master, TO_CHAR    );
    }
    if (ch->master->pet == ch)
	ch->master->pet = NULL;

    ch->master = NULL;
    ch->leader = NULL;
    return;
}

/* nukes charmed monsters and pets */
void nuke_pets( CHAR_DATA *ch )
{    
    CHAR_DATA *pet;

    if ((pet = ch->pet) != NULL)
    {
    	stop_follower(pet);
    	if (pet->in_room != NULL)
    	    act("$N slowly fades away.",ch,NULL,pet,TO_NOTVICT);
    	extract_char(pet,TRUE);
    }
    ch->pet = NULL;

    return;
}



void die_follower( CHAR_DATA *ch )
{
    CHAR_DATA *fch;

    if ( ch->master != NULL )
    {
    	if (ch->master->pet == ch)
    	    ch->master->pet = NULL;
	stop_follower( ch );
    }

    ch->leader = NULL;

    for ( fch = char_list; fch != NULL; fch = fch->next )
    {
	if ( fch->master == ch )
	    stop_follower( fch );
	if ( fch->leader == ch )
	    fch->leader = fch;
    }

    return;
}



void do_order( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH],arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *och;
    CHAR_DATA *och_next;
    bool found;
    bool fAll;

    argument = one_argument( argument, arg );
    one_argument(argument,arg2);

    if (!str_cmp(arg2,"delete") || !str_cmp(arg2,"mob"))
    {
        send_to_char("That will NOT be done.\n\r",ch);
        return;
    }

    if ((ch->riding)
    && (!str_cmp(arg2,"advance")
    || !str_cmp(arg2,"advanc")
    || !str_cmp(arg2,"advan")
    || !str_cmp(arg2,"adva")
    || !str_cmp(arg2,"adv")
    || !str_cmp(arg2,"ad")))
    {
        send_to_char("You need to get your ass off your mount first.\n\r",ch);
        return;
    }

    if (!str_cmp(arg2,"dirt")
    || !str_cmp(arg2,"dir")
    || !str_cmp(arg2,"di")
    || !str_cmp(arg2,"score")
    || !str_cmp(arg2,"scor")
    || !str_cmp(arg2,"sco")
    || !str_cmp(arg2,"sc")
    || !str_cmp(arg2,"trip")
    || !str_cmp(arg2,"tri")
    || !str_cmp(arg2,"tr")
    || !str_cmp(arg2,"drop")
    || !str_cmp(arg2,"dro")
    || !str_cmp(arg2,"dr")
    || !str_cmp(arg2,"bs")
    || !str_cmp(arg2,"backstab")
    || !str_cmp(arg2,"backsta")
    || !str_cmp(arg2,"backst")
    || !str_cmp(arg2,"backs")
    || !str_cmp(arg2,"back")
    || !str_cmp(arg2,"bac")
    || !str_cmp(arg2,"ambush")
    || !str_cmp(arg2,"circle")
    || !str_cmp(arg2,"description")
    || !str_cmp(arg2,"descriptio")
    || !str_cmp(arg2,"descripti")
    || !str_cmp(arg2,"descript")
    || !str_cmp(arg2,"descrip")
    || !str_cmp(arg2,"descri")
    || !str_cmp(arg2,"descr")
    || !str_cmp(arg2,"desc")
    || !str_cmp(arg2,"des")
    || !str_cmp(arg2,"de")
    || !str_cmp(arg2,"flag")
    || !str_cmp(arg2,"bash")
    || !str_cmp(arg2,"pardon")
    || !str_cmp(arg2,"murder")
    || !str_cmp(arg2,"murde")
    || !str_cmp(arg2,"murd")
    || !str_cmp(arg2,"mur")
    || !str_cmp(arg2,"mu")
    || !str_cmp(arg2,"m")
    || !str_cmp(arg2,"kill")
    || !str_cmp(arg2,"kil")
    || !str_cmp(arg2,"ki")
    || !str_cmp(arg2,"k")
    || !str_cmp(arg2,"note")
    || !str_cmp(arg2,"not")
    || !str_cmp(arg2,"no")
    || !str_cmp(arg2,"give")
    || !str_cmp(arg2,"giv")
    || !str_cmp(arg2,"gi")
    || !str_cmp(arg2,"cleave")
    || !str_cmp(arg2,"put")
    || !str_cmp(arg2,"pu")
    || !str_cmp(arg2,"p")
    || !str_cmp(arg2,"hit")
    || !str_cmp(arg2,"hi")
    || !str_cmp(arg2,"af")
    || !str_cmp(arg2,"deman")
    || !str_cmp(arg2,"dema")
    || !str_cmp(arg2,"dem")
    || !str_cmp(arg2,"de")
    || !str_cmp(arg2,"bount")
    || !str_cmp(arg2,"boun")
    || !str_cmp(arg2,"bou")
    || !str_cmp(arg2,"quit")
    || !str_cmp(arg2,"qui")
    || !str_cmp(arg2,"qu")
    || !str_cmp(arg2,"h"))
    {
        send_to_char("That will NOT be done.\n\r",ch);
        return;
    }

/* Persons with NOCHANNELS can't 'speak' through pet. */
	if (IS_SET(ch->comm, COMM_NOCHANNELS)
	&& (!str_cmp(arg2,"emo")
	||	!str_cmp(arg2,"emot")
	||	!str_cmp(arg2,"emote")
	||	!str_cmp(arg2,",")
	||	!str_cmp(arg2,"pm")
	||	!str_cmp(arg2,"pmo")
	||	!str_cmp(arg2,"pmot")
	||	!str_cmp(arg2,"pmote")
	||	!str_cmp(arg2,"sm")
	||	!str_cmp(arg2,"smo")
	||	!str_cmp(arg2,"smot")
	||	!str_cmp(arg2,"smote")
	||	!str_cmp(arg2,"gt")
	||	!str_cmp(arg2,"gte")
	||	!str_cmp(arg2,"gtel")
	||	!str_cmp(arg2,"gtell")
	||	!str_cmp(arg2,";")
	||	!str_cmp(arg2,"rep")
	||	!str_cmp(arg2,"repl")
	||	!str_cmp(arg2,"reply")
	||	!str_cmp(arg2,"t")
	||	!str_cmp(arg2,"te")
	||	!str_cmp(arg2,"tel")
	||	!str_cmp(arg2,"tell")
	||	!str_cmp(arg2,"sa")
	||	!str_cmp(arg2,"say")
	||	!str_cmp(arg2,"'")
	||	!str_cmp(arg2,"whi")
	||	!str_cmp(arg2,"whis")
	||	!str_cmp(arg2,"whisp")
	||	!str_cmp(arg2,"whispe")
	||	!str_cmp(arg2,"whisper")
	||	!str_cmp(arg2,"y")
	||	!str_cmp(arg2,"ye")
	||	!str_cmp(arg2,"yel")
	||	!str_cmp(arg2,"yell")
	||	!str_cmp(arg2,"cl")
	||	!str_cmp(arg2,"cla")
	||	!str_cmp(arg2,"clan")
	||	!str_cmp(arg2,"m")
	||	!str_cmp(arg2,"me")
	||	!str_cmp(arg2,"mes")
	||	!str_cmp(arg2,"mess")
	||	!str_cmp(arg2,"messa")
	||	!str_cmp(arg2,"messag")
	||	!str_cmp(arg2,"message")
	||	!str_cmp(arg2,"h")
	||	!str_cmp(arg2,"he")
	||	!str_cmp(arg2,"her")
	||	!str_cmp(arg2,"hero")) )
	{
		send_to_char("Trying to get around your revoked privileges, are we??\n\r",ch);
		return;
	}

	if (!str_cmp(arg2,"cp")
	||	!str_cmp(arg2,"cpo")
	||	!str_cmp(arg2,"cpos")
	||	!str_cmp(arg2,"cpose"))
	{
		send_to_char("That will NOT be done.\n\r",ch);
		return;
	}

    if (!str_cmp(arg2,"demand"))
    {
        send_to_char("That will NOT be done.\n\r",ch);
        return;
    }

    if (!str_cmp(arg2,"afk"))
    {
        send_to_char("That will NOT be done.\n\r",ch);
        return;
    }

    if (!str_cmp(arg2,"bounty"))
    {
        send_to_char("That will NOT be done.\n\r",ch);
        return;
    }

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Order whom to do what?\n\r", ch );
	return;
    }

    if( IS_AFFECTED2( ch, AFF_SPEECH_IMPAIR )  && !IS_IMMORTAL( ch ) )
    {
      send_to_char( "Your speech is impaired... they can't understand you right now.\n\r", ch);
      return;
    }

    if ( IS_AFFECTED( ch, AFF_CHARM ) )
    {
	send_to_char( "You feel like taking, not giving, orders.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	fAll   = TRUE;
	victim = NULL;
    }
    else
    {
	fAll   = FALSE;
	if ( ( victim = get_char_room( ch, NULL, arg ) ) == NULL )
	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
	}

	if ( victim == ch )
	{
	    send_to_char( "Aye aye, right away!\n\r", ch );
	    return;
	}

	if (!IS_AFFECTED(victim, AFF_CHARM) || victim->master != ch 
	||  (IS_IMMORTAL(victim) && victim->trust >= ch->trust))
	{
	    send_to_char( "Do it yourself!\n\r", ch );
	    return;
	}
    }

    found = FALSE;
    for ( och = ch->in_room->people; och != NULL; och = och_next )
    {
	och_next = och->next_in_room;

	if ( IS_AFFECTED(och, AFF_CHARM)
	&&   och->master == ch
	&& ( fAll || och == victim ) )
	{
	    found = TRUE;
	    sprintf( buf, "$n orders you to '%s'.", argument );
	    act( buf, ch, NULL, och, TO_VICT );
	    interpret( och, argument );
	}
    }

    if ( found )
    {
	WAIT_STATE(ch,PULSE_VIOLENCE);
	send_to_char( "Ok.\n\r", ch );
    }
    else
	send_to_char( "You have no followers here.\n\r", ch );
    return;
}



void do_group( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    char temphp[10],tempmana[10],tempmove[10];

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	CHAR_DATA *gch;
	CHAR_DATA *leader;

	leader = (ch->leader != NULL) ? ch->leader : ch;
	sprintf( buf, "%s's group:\n\r", PERS(leader, ch) );
	send_to_char( buf, ch );

        for ( gch = char_list; gch != NULL; gch = gch->next )
        {
            if ( is_same_group( gch, ch ) )
            {
		int tmp;
		int max;
		
		tmp = gch->hit;
		max = gch->max_hit;

		if (gch->hit == 0)
		   tmp = 1;
		if (gch->max_hit == 0)
		   max = 1;

		if ((tmp * 100 / max) < 10) 
                {
		  sprintf( temphp, "%d", tmp);	
		  sprintf( temphp, "%3d",(tmp * 100 / max) );  
		}
	        sprintf( temphp, "%3d", tmp  * 100 / max );

		tmp = gch->mana;
		max = gch->max_mana;

		if (gch->mana == 0)
		   tmp = 1;
		if (gch->max_mana == 0)
		   max = 1;

                sprintf( tempmana, "%3d", tmp * 100 / max );

		tmp = gch->move;
		max = gch->max_move;

		if (gch->move == 0)
		   tmp = 1;
		if (gch->max_move == 0)
		   max = 1;

		sprintf( tempmove, "%3d", tmp * 100 / max );

                sprintf( buf,
                "[%-5s] %-12s %-3s%% hp  %-3s%% mana  %-3s%% mv  [Position: %s]\n\r",
                    IS_NPC(gch) ? "Mob" : pc_race_table[gch->race].who_name,
                    capitalize( PERS(gch, ch) ),
                    temphp,
                    tempmana,
                    tempmove,
                    ( gch->fight_pos == FIGHT_FRONT ? "Front" : 
		      gch->fight_pos == FIGHT_MIDDLE ? "Mid" : "Back" )    );
		//act( buf, ch, NULL, NULL, TO_CHAR );
            send_to_char(buf, ch);
            }
        }
	return;
    }

    if ( ( victim = get_char_room( ch, NULL, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( ch->master != NULL || ( ch->leader != NULL && ch->leader != ch ) )
    {
	send_to_char( "But you are following someone else!\n\r", ch );
	return;
    }

    if ( victim->master != ch && ch != victim )
    {
	act_new("$N isn't following you.",ch,NULL,victim,TO_CHAR,POS_SLEEPING);
	return;
    }

    if (IS_IMMORTAL(victim) && !IS_IMMORTAL(ch))
    {
        send_to_char( "You are not worthy of joining forces with an immortal.\n\r",ch);
        return;
    }
    
    if (IS_AFFECTED(victim,AFF_CHARM))
    {
        send_to_char("Your follower finds you far too charming to be easily dismissed.\n\r",ch);
        return;
    }
    
    if (IS_AFFECTED(ch,AFF_CHARM))
    {
    	act_new("You like your master too much to leave $m!",
	    ch,NULL,victim,TO_VICT,POS_SLEEPING);
    	return;
    }

    if ( is_same_group( victim, ch ) && ch != victim )
    {
        victim->leader = NULL;
        act( "$n removes $N from $s group.",   ch, NULL, victim, TO_NOTVICT );
        act( "$n removes you from $s group.",  ch, NULL, victim, TO_VICT    );
        act( "You remove $N from your group.", ch, NULL, victim, TO_CHAR    );
        return;
    }
    if( ABS( ch->level - victim->level) > GROUP_RANGE )
    {
        act( "Traveling together, but not learning together, $N joins $n's group.", ch, NULL, victim, TO_NOTVICT);
        act( "You will not learn much by joining $n's group.", ch, NULL, victim, TO_VICT);
        act( "No one will learn much by $N joining the group.", ch, NULL, victim, TO_CHAR);
    }

    victim->leader = ch;
    act_new("$N joins $n's group.",ch,NULL,victim,TO_NOTVICT,POS_RESTING);
    act_new("You join $n's group.",ch,NULL,victim,TO_VICT,POS_SLEEPING);
    act_new("$N joins your group.",ch,NULL,victim,TO_CHAR,POS_SLEEPING);
    return;
}



/*
 * 'Split' originally by Gnort, God of Chaos.
 */
void do_split( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH],arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *gch;
    int members;
    int amount_steel = 0, amount_gold = 0;
    int share_steel, share_gold;
    int extra_steel, extra_gold;

    argument = one_argument( argument, arg1 );
	       one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Split how much?\n\r", ch );
	return;
    }
    
    amount_gold = atoi( arg1 );

    if (arg2[0] != '\0')
	amount_steel = atoi(arg2);

    if ( amount_steel < 0 || amount_gold < 0)
    {
	send_to_char( "Your group wouldn't like that.\n\r", ch );
	return;
    }

    if ( amount_steel == 0 && amount_gold == 0 )
    {
	send_to_char( "You hand out zero coins, but no one notices.\n\r", ch );
	return;
    }

    if ( ch->steel <  amount_steel || ch->gold < amount_gold)
    {
	send_to_char( "You don't have that much to split.\n\r", ch );
	return;
    }
  
    members = 0;
    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	if ( is_same_group( gch, ch ) && !IS_AFFECTED(gch,AFF_CHARM))
	    members++;
    }

    if ( members < 2 )
    {
	send_to_char( "Just keep it all.\n\r", ch );
	return;
    }
	    
    share_gold = amount_gold / members;
    extra_gold = amount_gold % members;

    share_steel   = amount_steel / members;
    extra_steel   = amount_steel % members;

    if ( share_steel == 0 && share_gold == 0 )
    {
	send_to_char( "Don't even bother, cheapskate.\n\r", ch );
	return;
    }

    ch->gold	-= amount_gold;
    ch->gold	+= share_gold + extra_gold;
    ch->steel 	-= amount_steel;
    ch->steel 	+= share_steel + extra_steel;

    if (share_gold > 0)
    {
	sprintf(buf,
	    "You split %d gold coins. Your share is %d gold.\n\r",
 	    amount_gold,share_gold + extra_gold);
	send_to_char(buf,ch);
    }

    if (share_steel > 0)
    {
	sprintf(buf,
	    "You split %d steel coins. Your share is %d steel.\n\r",
	     amount_steel,share_steel + extra_steel);
	send_to_char(buf,ch);
    }

    if (share_steel == 0)
    {
	sprintf(buf,"$n splits %d gold coins. Your share is %d gold.",
		amount_gold,share_gold);
    }
    else if (share_gold == 0)
    {
	sprintf(buf,"$n splits %d steel coins. Your share is %d steel.",
		amount_steel,share_steel);
    }
    else
    {
	sprintf(buf,
"$n splits %d gold and %d steel coins, giving you %d gold and %d steel.\n\r",
	 amount_gold,amount_steel,share_gold,share_steel);
    }

    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	if ( gch != ch && is_same_group(gch,ch) && !IS_AFFECTED(gch,AFF_CHARM))
	{
	    act( buf, ch, NULL, gch, TO_VICT );
	    gch->steel += share_steel;
	    gch->gold += share_gold;
	}
    }

    return;
}



void do_gtell( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *gch;
	int head_god = FALSE;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Tell your group what?\n\r", ch );
	return;
    }

    if ( IS_SET( ch->comm, COMM_NOTELL ) )
    {
	send_to_char( "Your message didn't get through!\n\r", ch );
	return;
    }

	if(!IS_NPC(ch) && IS_SET(ch->comm,COMM_NOCHANNELS) )
	{
		send_to_char("Your channel privileges have been revoked.\n\r",ch);
		return;
	}

    head_god = !str_cmp(ch->name, HEAD_GOD) 
            || !str_cmp(ch->name,HEAD_GOD2) 
            || !str_cmp(ch->name, HEAD_GOD3);

    if (((!str_infix("mud", argument))
    ||  (!str_infix("MUD", argument))
    ||  (!str_infix("MuD", argument))
    ||  (!str_infix("MUd", argument))
    ||  (!str_infix("mUD", argument))
    ||  (!str_infix("muD", argument))
    ||  (!str_infix("Mud", argument))
    ||  (!str_infix("mUd", argument))
    ||  (!str_suffix("mud", argument))
    ||  (!str_suffix("MUD", argument))
    ||  (!str_suffix("Mud", argument))
    ||  (!str_suffix("mud.", argument))
    ||  (!str_suffix("MUD.", argument))
    ||  (!str_suffix("Mud.", argument))
    ||  (!str_suffix("mud!", argument))
    ||  (!str_suffix("MUD!", argument))
    ||  (!str_suffix("Mud!", argument))
    ||  (!str_cmp("mud", argument))
    ||  (!str_cmp("mud,", argument))
    ||  (!str_cmp("MUD,", argument))
    ||  (!str_cmp("Mud,", argument))
    ||  (!str_cmp("mud.", argument))
    ||  (!str_cmp("MUD.", argument))
    ||  (!str_cmp("Mud.", argument))
    ||  (!str_cmp("mud!", argument))
    ||  (!str_cmp("MUD!", argument))
    ||  (!str_cmp("Mud!", argument))
    ||  (!str_prefix("Mud", argument))
    ||  (!str_prefix("MUD", argument))
    ||  (!str_prefix("mud", argument)))
    &&  !head_god)
    {
    sprintf( log_buf, "[WARNING] %s gtells: %s", ch->name, argument);
    log_string( log_buf );
    wiznet(log_buf,NULL,NULL,WIZ_MULTI,0,0);
    }

	for ( gch = char_list; gch != NULL; gch = gch->next )
    {
	if ( gch == ch )
	    sprintf(buf, "You tell the group '$t'");
	else
    	    sprintf( buf, "%s tells the group '$t'", ch->name );
        if ( is_same_group( gch, ch ) )
	{
	    act_new( buf, gch, argument, 0, TO_CHAR, POS_DEAD );
	}
    }

    return;
}



/*
 * It is very important that this be an equivalence relation:
 * (1) A ~ A
 * (2) if A ~ B then B ~ A
 * (3) if A ~ B  and B ~ C, then A ~ C
 */
bool is_same_group( CHAR_DATA *ach, CHAR_DATA *bch )
{
    if ( ach == NULL || bch == NULL)
	return FALSE;

    if ( ach->leader != NULL ) ach = ach->leader;
    if ( bch->leader != NULL ) bch = bch->leader;
    return ach == bch;
}

/*
 * ColoUr setting and unsetting, way cool, Ant Oct 94
 *        revised to include config colour, Ant Feb 95
 */
void do_colour( CHAR_DATA *ch, char *argument )
{
    char 	arg[ MAX_STRING_LENGTH ];

    if( IS_NPC( ch ) )
    {
	send_to_char_bw( "ColoUr is not ON, Way Moron!\n\r", ch );
	return;
    }

    argument = one_argument( argument, arg );

    if( !*arg )
    {
	if( !IS_SET( ch->act, PLR_COLOUR ) )
	{
	    SET_BIT( ch->act, PLR_COLOUR );
	    send_to_char( "ColoUr is now ON, Way Cool!\n\r"
		"Further syntax:\n\r   colour {c<{xfield{c> <{xcolour{c>{x\n\r"
		"   colour {c<{xfield{c>{x {cbeep{x|{cnobeep{x\n\r"
		"Type help {ccolour{x and {ccolour2{x for details.\n\r"
		"ColoUr is brought to you by Lope, ant@solace.mh.se.\n\r", ch );
	}
	else
	{
	    send_to_char_bw( "ColoUr is now OFF, <sigh>\n\r", ch );
	    REMOVE_BIT( ch->act, PLR_COLOUR );
	}
	return;
    }

    if( !str_cmp( arg, "default" ) )
    {
	default_colour( ch );
	send_to_char_bw( "ColoUr setting set to default values.\n\r", ch );
	return;
    }

    if( !str_cmp( arg, "all" ) )
    {
	all_colour( ch, argument );
	return;
    }

    /*
     * Yes, I know this is ugly and unnessessary repetition, but its old
     * and I can't justify the time to make it pretty. -Lope
     */
    if( !str_cmp( arg, "text" ) )
    {
	ALTER_COLOUR( text )
    }
    else if( !str_cmp( arg, "auction" ) )
    {
	ALTER_COLOUR( auction )
    }
    else if( !str_cmp( arg, "auction_text" ) )
    {
	ALTER_COLOUR( auction_text )
    }
    else if( !str_cmp( arg, "gossip" ) )
    {
	ALTER_COLOUR( gossip )
    }
    else if( !str_cmp( arg, "gossip_text" ) )
    {
	ALTER_COLOUR( gossip_text )
    }
    else if( !str_cmp( arg, "music" ) )
    {
	ALTER_COLOUR( music )
    }
    else if( !str_cmp( arg, "music_text" ) )
    {
	ALTER_COLOUR( music_text )
    }
    else if( !str_cmp( arg, "ask" ) )
    {
	ALTER_COLOUR( ask )
    }
    else if( !str_cmp( arg, "ask_text" ) )
    {
	ALTER_COLOUR( ask_text )
    }
    else if( !str_cmp( arg, "answer" ) )
    {
	ALTER_COLOUR( answer )
    }
    else if( !str_cmp( arg, "answer_text" ) )
    {
	ALTER_COLOUR( answer_text )
    }
    else if( !str_cmp( arg, "quote" ) )
    {
	ALTER_COLOUR( quote )
    }
    else if( !str_cmp( arg, "quote_text" ) )
    {
	ALTER_COLOUR( quote_text )
    }
    else if( !str_cmp( arg, "immtalk_text" ) )
    {
	ALTER_COLOUR( immtalk_text )
    }
    else if( !str_cmp( arg, "immtalk_type" ) )
    {
	ALTER_COLOUR( immtalk_type )
    }
    else if( !str_cmp( arg, "info" ) )
    {
	ALTER_COLOUR( info )
    }
    else if( !str_cmp( arg, "say" ) )
    {
	ALTER_COLOUR( say )
    }
    else if( !str_cmp( arg, "say_text" ) )
    {
	ALTER_COLOUR( say_text )
    }
    else if( !str_cmp( arg, "tell" ) )
    {
	ALTER_COLOUR( tell )
    }
    else if( !str_cmp( arg, "tell_text" ) )
    {
	ALTER_COLOUR( tell_text )
    }
    else if( !str_cmp( arg, "reply" ) )
    {
	ALTER_COLOUR( reply )
    }
    else if( !str_cmp( arg, "reply_text" ) )
    {
	ALTER_COLOUR( reply_text )
    }
    else if( !str_cmp( arg, "gtell_text" ) )
    {
	ALTER_COLOUR( gtell_text )
    }
    else if( !str_cmp( arg, "gtell_type" ) )
    {
	ALTER_COLOUR( gtell_type )
    }
    else if( !str_cmp( arg, "wiznet" ) )
    {
	ALTER_COLOUR( wiznet )
    }
    else if( !str_cmp( arg, "room_title" ) )
    {
	ALTER_COLOUR( room_title )
    }
    else if( !str_cmp( arg, "room_text" ) )
    {
	ALTER_COLOUR( room_text )
    }
    else if( !str_cmp( arg, "room_exits" ) )
    {
	ALTER_COLOUR( room_exits )
    }
    else if( !str_cmp( arg, "room_things" ) )
    {
	ALTER_COLOUR( room_things )
    }
    else if( !str_cmp( arg, "prompt" ) )
    {
	ALTER_COLOUR( prompt )
    }
    else if( !str_cmp( arg, "fight_death" ) )
    {
	ALTER_COLOUR( fight_death )
    }
    else if( !str_cmp( arg, "fight_yhit" ) )
    {
	ALTER_COLOUR( fight_yhit )
    }
    else if( !str_cmp( arg, "fight_ohit" ) )
    {
	ALTER_COLOUR( fight_ohit )
    }
    else if( !str_cmp( arg, "fight_thit" ) )
    {
	ALTER_COLOUR( fight_thit )
    }
    else if( !str_cmp( arg, "fight_skill" ) )
    {
	ALTER_COLOUR( fight_skill )
    }
    else
    {
	send_to_char_bw( "Unrecognised Colour Parameter Not Set.\n\r", ch );
	return;
    }

    send_to_char_bw( "New Colour Parameter Set.\n\r", ch );
    return;
}

void do_pray( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;
    if ( IS_SET(ch->comm, COMM_NOPRAY) )
    {
        send_to_char( "The gods refuse to hear your prayers.\n\r", ch );
        return;
    }
    if ( IS_NPC(ch))
    {
	return;
    }
    if ( IS_IMMORTAL(ch))
    {
        send_to_char( "You have no need for prayers.\n\r",ch);
        return;
    }
    if ( argument[0] == '\0' )
    {
        send_to_char( "Pray to whom, and for what purpose?\n\r", ch );
        return;
    }
    append_pray (ch, PRAY_FILE, argument);

    /* changed from an act message to a printf message.  Act messages do not
     * appear to character if in a DEAD state, but printf does
     */
    printf_to_char (ch,"\n\rYou Pray: {B'%s'{x\n\r", argument);
   // act("You pray: {B'$t'{x\n\r", ch, argument, NULL, TO_CHAR);
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        CHAR_DATA *victim;
        victim = d->original ? d->original : d->character;
        if ( d->connected == CON_PLAYING &&
            (IS_IMMORTAL(d->character) ||
	    (d->character->level > 101)) &&
	    !IS_NPC(victim)) 
        {
            act_new("$N Prays: {B'$t'{x", d->character, argument,ch, TO_CHAR, POS_DEAD);
        }
    }
    return;
}

bool check_ignore(CHAR_DATA *victim, CHAR_DATA *ch)
{
        int pos;
	if(!IS_NPC(victim) && !IS_NPC(ch))
	{
        for (pos =0; pos <= MAX_IGNORE; pos++)
        {
        if (victim->pcdata->ignore[pos] == NULL) break;

        if (!str_cmp(victim->pcdata->ignore[pos], ch->name))
        return TRUE;
        }
	}
return FALSE;
}

void do_ignore(CHAR_DATA *rch, char *argument)
{
    CHAR_DATA *victim;
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int pos=0;

    argument = one_argument(argument, arg);

/* Mobs can't ignore players. */

    if (IS_NPC(rch))
        return;

    if (arg[0] == '\0') /* 'ignore' typed with no arguments. */
  {

        if (rch->pcdata->ignore[0] == NULL)
        {
            send_to_char("You are not ignoring anyone.\n\r",rch);
            return;
        }
        send_to_char("You are currently ignoring:\n\r",rch);

        for (pos = 0; pos < MAX_IGNORE; pos++)
        {
            if (rch->pcdata->ignore[pos] == NULL)
                break;

            sprintf(buf,"%s\n\r",rch->pcdata->ignore[pos]);
            send_to_char(buf,rch);
        }
return;


   }


/* Can only set ignore for people currently online
(the list maintenance is based loosely on the alias command) */

if ((victim = get_char_world(rch,arg)) == NULL )
	{
	send_to_char("They aren't online.\n\r",rch);
	return;
	}

if (victim->desc == NULL && !IS_NPC(victim))
	{
	send_to_char("You can't ignore linkdead players.\n\r",rch);
	return;
	}

if (IS_NPC(victim) )
	{
	send_to_char("Sorry, you can't ignore NPC's\n\r", rch);
	return;
	}

if (IS_IMMORTAL(victim))
	{
	send_to_char("You can't ignore the powers that be.\n\r",rch);
	return;
	}

if (victim == rch)
	{
	send_to_char("You can't ignore yourself, silly!\n\r",rch);
	return;
	}

for(pos=0; pos < MAX_IGNORE; pos++)

	/* make sure you don't list someone twice, no matter how
	much they annoy you... */

	{
	if      (rch->pcdata->ignore[pos] != NULL
		&& !str_cmp(capitalize(arg),rch->pcdata->ignore[pos]))
   	   {
		sprintf(buf,"You are already ignoring %s.\n\r",capitalize(arg));
		send_to_char(buf,rch);
		return;
	   }
	}

for (pos = 0; pos < MAX_IGNORE; pos++)
        {
	/* basically, set the value for pos to use in the next section */

            if (rch->pcdata->ignore[pos] == NULL)
                break;
         }

if (pos >= MAX_IGNORE)
     {
        send_to_char("Sorry, you have reached the ignore limit.\n\r",rch);
        return;
     }

	/* New Ignore entry */

     rch->pcdata->ignore[pos]            = str_dup(victim->name);

     sprintf(buf,"You are now ignoring %s.\n\r",victim->name);
     send_to_char(buf,rch);
     return;

}

/* remove an entry from your ignore list. May be done whether person is on or
not */

void do_unignore(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *rch;
    char arg[MAX_INPUT_LENGTH];
    int pos;
    bool found = FALSE;

   if (ch->desc == NULL)
        rch = ch;
    else
        rch = ch->desc->original ? ch->desc->original : ch;

    if (IS_NPC(rch))
        return;

    argument = one_argument(argument,arg);

    if (arg[0] == '\0')
    {
        send_to_char("Unignore whom?\n\r",ch);
        return;
    }

    for (pos = 0; pos < MAX_IGNORE; pos++)
    {
        if (rch->pcdata->ignore[pos] == NULL)
            break;

        if (found)
        {
            rch->pcdata->ignore[pos-1]           = rch->pcdata->ignore[pos];
            rch->pcdata->ignore[pos]             = NULL;
            continue;
        }
       if(!str_cmp(capitalize(arg),rch->pcdata->ignore[pos]))
        {
            send_to_char("You are no longer ignoring them.\n\r",ch);
            free_string(rch->pcdata->ignore[pos]);
            rch->pcdata->ignore[pos] = NULL;
            found = TRUE;
        }
    }

    if (!found)
	{
	send_to_char("No entry of that name to remove.\n\r",ch);
	}
return;

}

void do_cpose( CHAR_DATA *ch, char *argument )
{
 
 if (ch->position == POS_FIGHTING  
 ||  ch->position <= POS_SLEEPING)
 {
   send_to_char("You cannot change your pose right now.\n\r",ch);
   return;
 }
 
 if ( strlen(argument) < 3) 
 {
  send_to_char("Usage: cpose <text>\n\r",ch);
  return;
 }
 
 free_string(ch->cpose);
 
 if (!strcmp(argument,"clear"))
 {
  ch->cpose = NULL;
  send_to_char("cpose cleared.\n\r",ch);
  return;
 }

 ch->cpose = str_dup(argument);
 send_to_char("Cpose set to: ",ch);
 send_to_char(argument,ch);
 send_to_char("\n\r",ch);

return;
}

void check_cpose( CHAR_DATA *ch)
{

  if (ch->cpose != NULL)
  {
    free_string(ch->cpose);   
    ch->cpose = NULL;
  }
return;
}

/*
 * tell_org - utility routine tells all in spec'd clan
 */
void tell_org( char *msg, int org, char *argument, CHAR_DATA *ch)
{
    CHAR_DATA *gch;
    char buf[MAX_STRING_LENGTH];
    org = ORG( org );

    if ( ch == NULL )
	sprintf(buf, "{c[%s] {W%s{x", org_table[org].upper_name, msg);
    else
    	sprintf( buf, "%s", msg );
    /*
     * Note use of act_new, so it works on sleepers.
     */

    for ( gch = char_list; gch != NULL; gch = gch->next )
    {
        if ( org == ORG( gch->org_id ) )
        {
#if 0
	    if ( ch != NULL && !can_see(gch,ch) )
		act_new(no, gch, argument, 0, TO_CHAR, POS_DEAD);
	    else
#endif
          act_new(buf, gch, argument, 0, TO_CHAR, POS_DEAD );
        }
    }
}

/*
 * do_message -- send a message to your organization
 */
void do_message( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char wiz_buf[MAX_STRING_LENGTH];
    CHAR_DATA *mob;
	int head_god = FALSE;

    if ( ch->org_id == ORG_NONE )
    {
        send_to_char( "You have no-one to send a message to.\n\r", ch );
        return;
    }

	if(!IS_NPC(ch) && IS_SET(ch->comm,COMM_NOCHANNELS) )
	{
		send_to_char("Your channel privileges have been revoked.\n\r",ch);
		return;
	}

    if( argument[0] == '\0' )
    {
	send_to_char("What message do you wish to report?\n\r",ch);
	return;
    }

    head_god = !str_cmp(ch->name, HEAD_GOD) 
            || !str_cmp(ch->name,HEAD_GOD2) 
            || !str_cmp(ch->name, HEAD_GOD3);

    if (((!str_infix("mud", argument))
    ||  (!str_infix("MUD", argument))
    ||  (!str_infix("MuD", argument))
    ||  (!str_infix("MUd", argument))
    ||  (!str_infix("mUD", argument))
    ||  (!str_infix("muD", argument))
    ||  (!str_infix("Mud", argument))
    ||  (!str_infix("mUd", argument))
    ||  (!str_suffix("mud", argument))
    ||  (!str_suffix("MUD", argument))
    ||  (!str_suffix("Mud", argument))
    ||  (!str_suffix("mud.", argument))
    ||  (!str_suffix("MUD.", argument))
    ||  (!str_suffix("Mud.", argument))
    ||  (!str_suffix("mud!", argument))
    ||  (!str_suffix("MUD!", argument))
    ||  (!str_suffix("Mud!", argument))
    ||  (!str_cmp("mud", argument))
    ||  (!str_cmp("mud,", argument))
    ||  (!str_cmp("MUD,", argument))
    ||  (!str_cmp("Mud,", argument))
    ||  (!str_cmp("mud.", argument))
    ||  (!str_cmp("MUD.", argument))
    ||  (!str_cmp("Mud.", argument))
    ||  (!str_cmp("mud!", argument))
    ||  (!str_cmp("MUD!", argument))
    ||  (!str_cmp("Mud!", argument))
    ||  (!str_prefix("Mud", argument))
    ||  (!str_prefix("MUD", argument))
    ||  (!str_prefix("mud", argument)))
	&&  !head_god)
    {
    sprintf( log_buf, "[WARNING] [%s] %s clans: %s",
	org_table[ORG(ch->org_id)].upper_name,
 	ch->name, argument);
    log_string( log_buf );
    wiznet(log_buf,NULL,NULL,WIZ_MULTI,WIZ_CLAN,0);
    }

    if (!IS_NPC(ch))
    {
    sprintf(wiz_buf, "[%s] %s clans: %s",
	org_table[ORG(ch->org_id)].upper_name,
      ch->name,
      argument );
    wiznet(wiz_buf,NULL,NULL,WIZ_CLAN,0,0);
    }

    switch(ORG(ch->org_id)) 
    {
	case ORG_KOT: case ORG_MERCENARY: case ORG_SOLAMNIC:
	    if (argument[0] != '\0') 
	    {
            if (!IS_NPC(ch))
            {
		  /* check for speech impairment */
        	  argument = speech_impair( ch, argument );
		  switch(ORG(ch->org_id)) 
		  {
		   case ORG_KOT:
			mob = get_char_room(ch, NULL, "sentry");
			break;
		   case ORG_MERCENARY:
			mob = get_char_room(ch, NULL, "hound");
			break;
		   case ORG_SOLAMNIC:
			mob = get_char_room(ch, NULL, "warden");
			break;
		   default:
			return;
			break; 
		  }
		  if (mob == NULL)
		  {
	    	    send_to_char("No one here will send a message for you. You need to call for BACKUP.\n\r",ch);
	    	    return;
		  }
		  if (!IS_NPC(mob))
		  {
	    	    send_to_char("Why don't you just ask them?\n\r",ch);
	    	    return;
		  }
		  if (mob->mesg_buf != NULL)
		  {
	    	    act("$N cannot send another messenger so soon.",ch, NULL, mob, TO_CHAR);
	    	    return;
		  }	
		  switch(ORG(ch->org_id))
              {
		   case ORG_KOT:
			sprintf(buf, "kot %s ", ch->name);
			break;		
		   case ORG_MERCENARY:
			sprintf(buf, "mercenary %s ", ch->name);
			break;
		   case ORG_SOLAMNIC:
			sprintf(buf, "solamnic %s ", ch->name);
			break;
              }
		  strcat(buf, argument);
		  mob->mesg_buf = str_dup( buf );
		
		  send_to_char("You pass your message into worthy hands.\n\r",ch);
		  switch(ORG(ch->org_id)) 
              {
		  case ORG_KOT:
	          act("A loyal sentry takes a scroll from his master.",ch, NULL, NULL, TO_ROOM);
		    interpret(mob, "bow");
		    act("A sentry whistles and a courier on horseback gallops in.",ch, NULL, NULL, TO_ROOM);
		    act("A courier salutes as he grabs the message and gallops off.",ch, NULL, NULL, TO_ROOM);
		    act("A sentry whistles and a courier on horseback gallops in.",ch, NULL, NULL, TO_CHAR);
		    act("A courier salutes as he grabs the message and gallops off.",ch, NULL, NULL, TO_CHAR);
		    break;
		  case ORG_MERCENARY:
		    interpret(mob, "bark");
		    interpret(mob, "emote howls loudly, summoning a winged messenger demon.");
		    act("The demon grasps a message from his master in his claw and flies off with a sharp hiss.", ch, NULL, NULL, TO_ROOM);
		    act("The demon grasps a message from you in his claw and flies off with a sharp hiss.", ch, NULL, NULL, TO_CHAR);
		    break;
		  case ORG_SOLAMNIC:
		    interpret(mob, "bow");
		    interpret(mob, "emote calls in a horseback courier.");
		    interpret(mob, "emote quickly writes a message and hands it to the courier.");
		    act("The courier salutes, then rides off at a quick gallop.", ch, NULL, NULL, TO_ROOM);
		    act("The courier salutes, then rides off at a quick gallop.", ch, NULL, NULL, TO_CHAR);
		    break;
		  }
		  /* sets tick delay -- tune as appropriate */
		  mob->mesg_delay = 1;
		  return;
	      }
              else
              {
	        argument = speech_impair(ch, argument);
                ch->short_descr[0] = UPPER(ch->short_descr[0]);
    	        sprintf( buf, "{Y%s informs: {x'$t'", ch->short_descr);
    	        tell_org( buf, ch->org_id, argument, ch );
	        return;
              }
            }
	    else
            {
		send_to_char("What message do you wish to report?\r\n", ch);
		return;
	    }
	    return;
	    break;
	case ORG_CONCLAVE:
	    if (argument[0] == '\0')
	    {
	      send_to_char("What do you wish to say?\r\n", ch);
	      return;
	    }
	    argument = speech_impair(ch, argument);
            if (!IS_NPC(ch))
            {
    	      sprintf( buf, "{Y%s mentally projects: {x'$t'", ch->name);
            }
            else
            {
              ch->short_descr[0] = UPPER(ch->short_descr[0]);
              sprintf( buf, "{Y%s mentally projects: {x'$t'", ch->short_descr);
            }
    	    tell_org( buf, ch->org_id, argument, ch );
	    return;
	    break;
        case ORG_FORESTER:
            if (argument[0] == '\0')
            {
 	      send_to_char("What do you want your message to say?\n\r",ch);
	      return;
	    }
	    if (ch->mesg_delay != 0)
    	    {
	      send_to_char("You still have a pigeon en route.\n\r",ch);
	      return;
 	    }	

           if (!IS_NPC(ch))
           { 
	     sprintf(buf, "forester %s ", ch->name);           
	     strcat(buf, argument);
  	     ch->mesg_buf = str_dup(buf);
 	     ch->mesg_delay=1; 
             send_to_char("You quickly write your message on a small piece of parchment.\n\r",ch);	
	     send_to_char("You give the message to a carrier pigeon.\n\r",ch);
             act("A carrier pigeon tightly grips a message in his talons.",ch, NULL, NULL, TO_ROOM);
	     act("A carrier pigeon flaps its wings as it begins its hurried flight.",ch, NULL, NULL, TO_ROOM);
	     return;
           }
           else
           {
	     argument = speech_impair(ch, argument);
             ch->short_descr[0] = UPPER(ch->short_descr[0]);
    	     sprintf( buf, "{Y%s informs: {x'$t'", ch->short_descr);
    	     tell_org( buf, ch->org_id, argument, ch );
	     return;
           }  
	   break;
    	case ORG_THIEVES:

	   if (argument[0] == '\0')
	   {
		send_to_char("Why send any empty message?\r\n", ch);
		return;
	   }

	   if (ch->mesg_delay != 0)
    	   {
		send_to_char("You still have a cloaked messenger en route.\n\r",ch);
	    	return;
 	   }

/* commented out the code below because if we wanted to use the short description for
* mobs, it would require a second buffer. .  If we want to eventually have the lag for
* NPCs as well, then we'll beed to add a second message buf to store to be used in
* char_update.  Only leaving the THieves code in tact but commented out for future use
* if need be.  Other guild areas I've deleted.
*           if (!IS_NPC(ch))
*           { 
*	     sprintf(buf, "thieves %s ", ch->name);
*           }
*           else
*           {
*             sprintf(buf, "thieves %s ", ch->short_descr);
*           }
*	   strcat(buf, argument);
*  	   ch->mesg_buf = str_dup(buf);
*	   
* 	   ch->mesg_delay=1; 
*           send_to_char("You quickly write your message on a small piece of parchment.\n\r",ch);
*	   send_to_char("You give the message to a cloaked informant.\n\r",ch);
*           act("A cloaked informant takes a message and puts it in his cloak.",ch, NULL, NULL, TO_ROOM);
*	   act("A cloaked informant hurries off to deliver the message.",ch, NULL, NULL, TO_ROOM);
*	   return;
*/

           if (!IS_NPC(ch))
           { 
	     sprintf(buf, "thieves %s ", ch->name);
	     strcat(buf, argument);
  	     ch->mesg_buf = str_dup(buf);
 	     ch->mesg_delay=1; 
             send_to_char("You quickly write your message on a small piece of parchment.\n\r",ch);
	     send_to_char("You give the message to a cloaked informant.\n\r",ch);
             act("A cloaked informant takes a message and puts it in his cloak.",ch, NULL, NULL, TO_ROOM);
	     act("A cloaked informant hurries off to deliver the message.",ch, NULL, NULL, TO_ROOM);
	     return;
           }
           else
           {
	     argument = speech_impair(ch, argument);
             ch->short_descr[0] = UPPER(ch->short_descr[0]);
    	     sprintf( buf, "{Y%s informs: {x'$t'", ch->short_descr);
    	     tell_org( buf, ch->org_id, argument, ch );
	     return;
           }             
	   break;
        case ORG_HOLYORDER:
           if (argument[0] == '\0')
           {
 		send_to_char("What do you want your message to say?\n\r",ch);
		return;
	   }
	   if (ch->mesg_delay != 0)
    	   {
		send_to_char("You still have a cleric en route.\n\r",ch);
	    	return;
 	   }	

           if (!IS_NPC(ch))
           { 
	     sprintf(buf, "holyorder %s ", ch->name);
	     strcat(buf, argument);
  	     ch->mesg_buf = str_dup(buf);
 	     ch->mesg_delay=1; 
             send_to_char("You quickly write your message on a small piece of parchment.\n\r",ch);	
	     send_to_char("You give the message to a cleric.\n\r",ch);
             act("A cleric tightly grasps a message in his hands.",ch, NULL, NULL, TO_ROOM);
	     act("A cleric turns on his heels and runs off to deliver the message.",ch, NULL, NULL, TO_ROOM);
	     return;
           }
           else
           {
	     argument = speech_impair(ch, argument);
             ch->short_descr[0] = UPPER(ch->short_descr[0]);
    	     sprintf( buf, "{Y%s informs: {x'$t'", ch->short_descr);
    	     tell_org( buf, ch->org_id, argument, ch );
	     return;
           }  
	   break;
        case ORG_UNDEAD:
           if (argument[0] == '\0')
           {
 		send_to_char("What do you want your message to say?\n\r",ch);
		return;
	   }
	   if (ch->mesg_delay != 0)
    	   {
		send_to_char("You still have a demonic knight en route.\n\r",ch);
	    	return;
 	   }	

           if (!IS_NPC(ch))
           { 
	     sprintf(buf, "undead %s ", ch->name);
	     strcat(buf, argument);
  	     ch->mesg_buf = str_dup(buf);
	     
 	     ch->mesg_delay=1; 
             send_to_char("You quickly write your message on a small piece of parchment.\n\r",ch);	
	     send_to_char("You give the message to a demonic knight.\n\r",ch);
             act("A demonic knight grasps a message in his white, skeletal hands.",ch, NULL, NULL, TO_ROOM);
	     act("A demonic knight fades into a shroud of darkness, disappearing from sight.",ch, NULL, NULL, TO_ROOM);
	     return;
           }
           else
           {
	     argument = speech_impair(ch, argument);
             ch->short_descr[0] = UPPER(ch->short_descr[0]);
    	     sprintf( buf, "{Y%s informs: {x'$t'", ch->short_descr);
    	     tell_org( buf, ch->org_id, argument, ch );
	     return;
           }  
	   break;
        case ORG_ARTISANS:
           if (argument[0] == '\0')
           {
 		send_to_char("What do you want your message to say?\n\r",ch);
		return;
	   }
	   if (ch->mesg_delay != 0)
    	   {
		send_to_char("You still have a blacksmith en route.\n\r",ch);
	    	return;
 	   }	

           if (!IS_NPC(ch))
           { 
	     sprintf(buf, "artisans %s ", ch->name);
	     strcat(buf, argument);
  	     ch->mesg_buf = str_dup(buf);
	   
 	     ch->mesg_delay=1; 
             send_to_char("You quickly write your message on a small piece of parchment.\n\r",ch);	
	     send_to_char("You give the message to a blacksmith.\n\r",ch);
             act("A blacksmith grasps a message in his large hands.",ch, NULL, NULL, TO_ROOM);
	     act("A blacksmith grunts once before departing with a message.",ch, NULL, NULL, TO_ROOM);
	     return;
           }
           else
           {
	     argument = speech_impair(ch, argument);
             ch->short_descr[0] = UPPER(ch->short_descr[0]);
    	     sprintf( buf, "{Y%s informs: {x'$t'", ch->short_descr);
    	     tell_org( buf, ch->org_id, argument, ch );
	     return;
           }  
	   break;
        case ORG_AESTHETIC:
           if (argument[0] == '\0')
           {
 		send_to_char("What do you want your message to say?\n\r",ch);
		return;
	   }
	   if (ch->mesg_delay != 0)
    	   {
		send_to_char("You still have a hawk en route.\n\r",ch);
	    	return;
 	   }	

           if (!IS_NPC(ch))
           { 
	     sprintf(buf, "aesthetic %s ", ch->name);
	     strcat(buf, argument);
  	     ch->mesg_buf = str_dup(buf);
 	     ch->mesg_delay=1; 
             send_to_char("You quickly write your message on a small piece of parchment.\n\r",ch);	
	     send_to_char("You give the message to a hawk.\n\r",ch);
             act("A hawk tightly grips a message in his talons.",ch, NULL, NULL, TO_ROOM);
	     act("A hawk flaps its wings as it begins its hurried flight.",ch, NULL, NULL, TO_ROOM);
	     return;
           }
           else
           {
	     argument = speech_impair(ch, argument);
             ch->short_descr[0] = UPPER(ch->short_descr[0]);
    	     sprintf( buf, "{Y%s informs: {x'$t'", ch->short_descr);
    	     tell_org( buf, ch->org_id, argument, ch );
	     return;
           }  
	   break;
        case ORG_BLACKORDER:
           if (argument[0] == '\0')
           {
 		send_to_char("What do you want your message to say?\n\r",ch);
		return;
	   }
	   if (ch->mesg_delay != 0)
    	   {
		send_to_char("You still have an unholy monk en route.\n\r",ch);
	    	return;
 	   }	

           if (!IS_NPC(ch))
           { 
	     sprintf(buf, "blackorder %s ", ch->name);
	     strcat(buf, argument);
  	     ch->mesg_buf = str_dup(buf);
 	     ch->mesg_delay=2; 
             send_to_char("You quickly write your message on a small piece of parchment.\n\r",ch);	
	     send_to_char("You give the message to an unholy monk.\n\r",ch);
             act("An unholy monk grasps a message in his hands.",ch, NULL, NULL, TO_ROOM);
	     act("An unholy monk bows his head in reverence and quickly departs.",ch, NULL, NULL, TO_ROOM);
	     return;
           }
           else
           {
	     argument = speech_impair(ch, argument);
             ch->short_descr[0] = UPPER(ch->short_descr[0]);
    	     sprintf( buf, "{Y%s informs: {x'$t'", ch->short_descr);
    	     tell_org( buf, ch->org_id, argument, ch );
	     return;
           }  
	   break;
	default:
	    break;
    }
    if ( IS_SET( ch->comm, COMM_NOTELL ) )
    {
        send_to_char( "Your message didn't get through!\n\r", ch );
        return;
    }

    /* check for speech impairment */
    argument = speech_impair( ch, argument );
    /* tell all clan members */
    sprintf( buf, ": '$t'");
    tell_org( buf, ch->org_id, argument, ch );
}

/*
 *  speech_impair
 *   
 *  Checks for an affect that alters speech.  Makes a static copy
 *  of mesg so mobs and const strings can be handled.  Careful,
 *  buffer is overwritten by next to this routine.
 *  If no impairment, simply returns passed argument.
 */
char * speech_impair( CHAR_DATA *ch, char *mesg )
{
    AFFECT_DATA *paf;
    int mod = MOD_NONE;
    static char new_mesg[MAX_INPUT_LENGTH];
    if( IS_IMMORTAL(ch) )
        return( mesg );
   
    if( !IS_AFFECTED2( ch, AFF_SPEECH_IMPAIR ) )
        return( mesg );
    
    /* figure out what the speech disorder is */
    for ( paf = ch->affected; paf != NULL; paf = paf->next )
    {
        if ( paf->location == APPLY_SPEECH_IMPAIR )
        {
            mod = paf->modifier;
            break;
        }
    }
    switch( mod )
    {
    default:
        bug( "speech impairment w/ bad modifier", mod);
        return( mesg );
    case MOD_SPEECH_REVERSE:
        strcpy( new_mesg, mesg );
        reverse_txt( new_mesg, strlen(new_mesg) );
        break;
    case MOD_SPEECH_GARBLE:
        strcpy( new_mesg, mesg );
        mesg_garble( new_mesg );
        break;
    }
    return( new_mesg );
}

/*
 *  mesg_reverse  
 *  Simply reverses mesg text.
 */
void mesg_reverse( char *mesg )
{
    int hlen,len;
    int i,tmp;
    len = strlen(mesg);
    if( len == 0 ) return;
    hlen = len / 2;
    len--;
    for(i = 0; i < hlen; i++ )
    {
        tmp = mesg[i];
        mesg[i] = mesg[len - i];
        mesg[len - i] = tmp;
    }
}

/*
 *  mesg_garble
 *  Substitutes letters to make mesg text a bit unreadable.
 */
void mesg_garble( char *mesg )
{
    static char *scram = "vdhgxpbjynzaotcuiwmfklqers";
    static int last_char;
    int i, len, c;
    len = strlen(mesg);
    for(i = 0; i < len; i++)
    {
        c = mesg[i];
        if( c >= 'a' && c <= 'z' )
        {
            c -= 'a';
            c = scram[(c + last_char) % 26];
            last_char = c;
        }
        else if( c >= 'A' && c <= 'Z' )
        {
            c -= 'A';
            c = scram[(c + last_char) % 26];
            last_char = c;
        }
        mesg[i] = c;
    }
}

void do_dismiss( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
       send_to_char( "Dismiss who?\n\r", ch );
       return;
    }

    if ( ( victim = get_char_room( ch, NULL, arg ) ) == NULL )
    {
       send_to_char( "They aren't here.\n\r", ch );
       return;
    }

    if ( victim == ch )
    {
       send_to_char( "Dismiss yourself? Slap yourself!\n\r", ch );
       return;
    }

    if ( !IS_NPC(victim))
    {
       send_to_char( "You can't dismiss players! Slap yourself!\n\r", ch );
       return;
    }

    if (victim == ch->riding) //are we mounted on it?
    {
	 send_to_char( "Get your ass off of it first!\n\r",ch );
	 return;
    }

    if (!str_cmp("a polar bear", victim->short_descr))
    {

      if (victim->master == NULL)
	   return;

      if (victim->master == ch)
      {
         if ((ch->in_room->sector_type == SECT_POLAR))
	     act("$N looks out into the snow and ice.\n\r",ch,NULL,victim,TO_CHAR);
         else
         {
	      act("$N refuses to leave in this terrain.\n\r",ch,NULL,victim,TO_CHAR);
	      return;
	   }
	   do_emote(victim, "raises it's front paw, and then wanders off.");
	   extract_char(victim, TRUE);
	   return;
      }
    else
	{
	   do_say(victim, "I do not listen to you.");
	   return;
	}
    }
    else
    if (!str_cmp("a tough old grizzly bear", victim->short_descr)
    ||  !str_cmp("a brown bear", victim->short_descr)
    ||  !str_cmp("a kodiak bear", victim->short_descr)
    ||  !str_cmp("a baby black bear", victim->short_descr))
    {

      if (victim->master == NULL)
	   return;

      if (victim->master == ch)
      {
         do_say (ch, "Return to nature!");
         if ((ch->in_room->sector_type == SECT_FOREST)
	   ||  (ch->in_room->sector_type == SECT_FIELD)
	   ||  (ch->in_room->sector_type == SECT_HILLS)
	   ||  (ch->in_room->sector_type == SECT_MOUNTAIN))
	     act("$N looks around peacefully.",ch,NULL,victim,TO_CHAR);
       else
         {
	      act("$N refuses to leave in this terrain.\n\r",ch,NULL,victim,TO_CHAR);
	      return;
	   }
	   do_emote(victim, "raises it's front paw, and then wanders off.");
	   extract_char(victim, TRUE);
	   return;
      }
    else
	{
	   do_say(victim, "I do not listen to you.");
	   return;
	}
    }
    else
    if (!str_cmp("a vicious hound", victim->short_descr))
    {
      if ( IS_MERCENARY(ch))
      {
      do_say (ch, "Hounds! Back to the kennel with you!");
      do_emote(victim, "howls loyally to its master, before bounding away.");
      extract_char(victim, TRUE);
      return;
      }
      else
      {
      do_emote(victim, "snarls savagely, yet maintains its well-trained disposition.");
      return;
      }
    }
    else
    if (!str_cmp("a strong sentry", victim->short_descr))
    {
      if ( IS_KOT(ch))
      {
      do_say (ch, "Leave me. I have political matters to attend to!");
      do_emote(victim, "raises his blade for the glory of the Dark Queen and marches away.");
      extract_char(victim, TRUE);
      return;
      }  
      else
      {
      do_say(victim, "Only those who work in the Dark Queen's name may give me orders.");
      return;
      }
    }
    else
    if (!str_cmp("a towering warden", victim->short_descr))
    {
      if ( IS_SOLAMNIC(ch))
      {
      do_say (ch, "You are dismissed!");
      do_emote(victim, "gives a crisp salute, and obediently departs.");
      extract_char(victim, TRUE);
      return;
      }
      else
      {
      do_say(victim, "I may only be dismissed by my superior officers.");
      return;
      }
    }
    else
    if (!str_cmp("a skeleton warrior", victim->short_descr))
    {
    int mana_cost=10;

	if (victim->master == NULL)
      {
	   do_say(victim, "I do not listen to you. Get lost punk!");
	   return;
      }

	if (victim->master == ch)
	{
	   if (ch->mana <= mana_cost)
	   {
	   send_to_char("You do not have the energy to guide the spirit to the grave.",ch);
	   return;
	   }
	   if (ch->in_room->sector_type != SECT_GRAVEYARD)
	   {
	   act("$N must be laid to rest in a graveyard.\n\r",ch,NULL,victim,TO_CHAR);
	   return;
	   }
         do_say (ch, "Return to thy grave!");
	   act("$n gestures to $N and then to a grave.\n\r",ch,NULL,victim,TO_ROOM);
	   act("You gesture to $N and guide $M into a grave.",ch,NULL,victim,TO_CHAR);
	   do_emote(victim, "lays in the grave and finds eternal rest.");
         extract_char(victim, TRUE);
	   ch->mana -= mana_cost;
         return;
      }
	else
	{
	   do_say(victim, "I only obey my one true master.");
	   return;
	}
    }
    else
    if (!str_cmp("a hideous zombie", victim->short_descr))
    {
    int mana_cost=10;

	if (victim->master == NULL)
      {
	   do_say(victim, "I do not listen to you. Get lost punk!");
	   return;
      }

	if (victim->master == ch)
	{
	   if (ch->mana <= mana_cost)
	   {
	   send_to_char("You do not have the energy to guide the spirit to the grave.",ch);
	   return;
	   }
	   if (ch->in_room->sector_type != SECT_GRAVEYARD)
	   {
	   act("$N must be laid to rest in a graveyard.\n\r",ch,NULL,victim,TO_CHAR);
	   return;
	   }
         do_say (ch, "Return to thy grave!");
	   act("$n gestures to $N and then to a grave.\n\r",ch,NULL,victim,TO_ROOM);
	   act("You gesture to $N and guide $M into a grave.",ch,NULL,victim,TO_CHAR);
	   do_emote(victim, "lays in the grave and finds eternal rest.");
         extract_char(victim, TRUE);
	   ch->mana -= mana_cost;
         return;
      }
	else
	{
	   do_say(victim, "I only obey my one true master.");
	   return;
	}
    }
    else
    if (!str_cmp("a nightmare", victim->short_descr))
    {
    int mana_cost=15;

	if (victim->master == NULL)
      {
	   do_say(victim, "I do not listen to you. Get lost punk!");
	   return;
      }

	if (victim->master == ch)
	{
	   if (ch->mana <= mana_cost)
	   {
	   send_to_char("You do not have the energy to send this one home.",ch);
	   return;
	   }
         do_say (ch, "Return to your home!");
	   act("$n draws a conjuring circle and gestures to $N.",ch,NULL,victim,TO_ROOM);
	   act("You draw a conjuring circle and gesture to $N.",ch,NULL,victim,TO_CHAR);
	   do_emote(victim, "steps into the circle and disappears.");
         extract_char(victim, TRUE);
	   ch->mana -= mana_cost;
         return;
      }
	else
	{
	   do_say(victim, "I only obey my one true master.");
	   return;
	}
    }
    else
    if (!str_cmp("an ethereal warrior", victim->short_descr))
    {
    int mana_cost=15;

	if (victim->master == NULL)
      {
	   do_say(victim, "I do not listen to you. Get lost punk!");
	   return;
      }

	if (victim->master == ch)
	{
	   if (ch->mana <= mana_cost)
	   {
	   send_to_char("You do not have the energy to send this one home.",ch);
	   return;
	   }
         do_say (ch, "Return to your home!");
	   act("$n draws a conjuring circle and gestures to $N.",ch,NULL,victim,TO_ROOM);
	   act("You draw a conjuring circle and gesture to $N.",ch,NULL,victim,TO_CHAR);
	   do_emote(victim, "steps into the circle and disappears.");
         extract_char(victim, TRUE);
	   ch->mana -= mana_cost;
         return;
      }
	else
	{
	   do_say(victim, "I only obey my one true master.");
	   return;
	}
    }
    else
    if (!str_cmp("a small rabbit", victim->short_descr))
    {
    int mana_cost=15;

	if (victim->master == NULL)
      {
	   do_say(victim, "I do not listen to you. Get lost punk!");
	   return;
      }

	if (victim->master == ch)
	{
	   if (ch->mana <= mana_cost)
	   {
	   send_to_char("You do not have the energy to send this one home.",ch);
	   return;
	   }
         do_say (ch, "Return to your home!");
	   act("$n draws a conjuring circle and gestures to $N.",ch,NULL,victim,TO_ROOM);
	   act("You draw a conjuring circle and gesture to $N.",ch,NULL,victim,TO_CHAR);
	   do_emote(victim, "steps into the circle and disappears.");
         extract_char(victim, TRUE);
	   ch->mana -= mana_cost;
         return;
      }
	else
	{
	   do_say(victim, "I only obey my one true master.");
	   return;
	}
    }
    else
    if (!str_cmp("a young squire", victim->short_descr))
    {

	if (victim->master == NULL)
      {
	   do_say(victim, "I do not listen to you. Get lost punk!");
	   return;
      }

	if (victim->master == ch)
	{
         do_say (ch, "You are dismissed!");
         do_emote(victim, "gives a crisp salute, and obediently departs.");
         extract_char(victim, TRUE);
         return;
      }
	else
	{
	   do_say(victim, "I only obey my one true master.");
	   return;
	}
    }
    else
    if (!str_cmp("an unseen servant", victim->short_descr))
    {
    int mana_cost=15;

	if (victim->master == NULL)
      {
	   do_say(victim, "I do not listen to you. Get lost punk!");
	   return;
      }

	if (victim->master == ch)
	{
	   if (ch->mana <= mana_cost)
	   {
	   send_to_char("You do not have the energy to send this one home.",ch);
	   return;
	   }
         do_say (ch, "Return to your home!");
	   act("$n draws a conjuring circle and gestures to $N.",ch,NULL,victim,TO_ROOM);
	   act("You draw a conjuring circle and gesture to $N.",ch,NULL,victim,TO_CHAR);
	   do_emote(victim, "steps into the circle and disappears.");
         extract_char(victim, TRUE);
	   ch->mana -= mana_cost;
         return;
      }
	else
	{
	   do_say(victim, "I only obey my one true master.");
	   return;
	}
    }
    else
    {
    do_say(victim, "I do not listen to you. Get lost punk!");
    return; 
    }
}

void sing_channel(CHAR_DATA *ch, char *argument)
{
 #ifdef LANGUAGES
    CHAR_DATA *vict;
    char buf[MSL];
    int language = 0;
#endif
    char *mesg;

  if (!IS_AWAKE(ch))
  {
    SINGING_DATA *psing;
    for (psing = singing_list; psing != NULL; psing = psing->next)
    {
      if (ch == psing->singer)
      {     
        singing_list = psing->next;
        free_string(psing->title);
        free_string(psing->text);
        free_string(psing->composer);
        singing_free = psing;
      } 
    }
    send_to_char("You must be awake to sing.\n\r",ch);
    return;
  }

  if ((IS_AFFECTED2(ch, AFF_SPEECH_IMPAIR))
  ||  (IS_AFFECTED2(ch, AFF_SILENCE))
  ||  (ch->position == POS_DEAD))
  {
    SINGING_DATA *psing;
    for (psing = singing_list; psing != NULL; psing = psing->next)
    {
      if (ch == psing->singer)
      {     
        singing_list = psing->next;
        free_string(psing->title);
        free_string(psing->text);
        free_string(psing->composer);
        singing_free = psing;
      } 
    }
    send_to_char("You could not possibly sing right now.",ch);
    return;
  }

  if ( argument[0] == '\0' )
  {
    return;
  }

  /* check for speech impairment */

  argument = speech_impair( ch, argument );

#ifdef LANGUAGES
    mesg = language_check( ch, argument, NULL );
    if ((language = lang_skill(ch, NULL)) != 0)
	sprintf(buf, "You sing in %s '$T'",capitalize(lang_lookup(ch->language)));
    else
	sprintf(buf, "You sing '$T'");
    act( buf, ch, NULL, mesg, TO_CHAR );
    if (ch->in_room == NULL)
    return;
    for ( vict = ch->in_room->people ; vict != NULL; vict = vict->next_in_room)
    {
	if( vict == ch )
	    continue;

        if (IS_AFFECTED2(vict, AFF_DEAFNESS))
        return;
	

    if (lang_skill(ch, vict))
	sprintf(buf, "$n sings in %s '$t'",capitalize(lang_lookup(ch->language)));
    else
      sprintf(buf, "$n sings '$t'");

    act( buf, ch, language_check(ch, mesg, vict), vict, TO_VICT);
  }
  #else
    act( "$n sings '$T'", ch, NULL, argument, TO_ROOM );
    act( "You sing '$T'", ch, NULL, argument, TO_CHAR );
    mesg = argument;
  #endif

  return;
}

void do_windtalk( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  int chance;
  CHAR_DATA *victim;

  argument = one_argument( argument, arg );

  /* added skill check.  Without it, people were able to execute - CMW 11/2013 */ 
  chance = get_skill(ch,gsn_windtalk);
  if (chance == 0) {
    send_to_char("Huh?\n\r",ch);
    return;
  }

  if ( arg[0] == '\0' )
  {
    send_to_char("Who should the wind carry your message to?\n\r", ch);
    return;
  }

  if ( argument[0] == '\0' )
  {
    send_to_char("What message would you like the wind to deliver to your intended target?\n\r", ch);
    return;
  }

  if ( ( victim = get_char_world( ch, arg ) ) == NULL )
  {
    send_to_char("The wind carries your message high into the air, but quickly dissipates,\n\rfailing to deliver your message to its intended target!\n\r", ch);
    return;
  }

  if (ch->mana < 30)
  {
    send_to_char("You do not have enough mental power to deliver a magical message at this time!\n\r", ch);
    return;
  }

  if (IS_AFFECTED2(ch, AFF_PARALYSIS) )
  {
    send_to_char("You have been paralized! You couldn't possibley conjure up the wind!\n\r",ch);
    return;
  }

  if (IS_AFFECTED3(ch, AFF_HOLDPERSON) )
  {
    send_to_char("Your can't move, hence you couldn't possibley conjure up the wind!\n\r",ch);
    return;
  }

  if ( IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
  ||   IS_SET(victim->in_room->room_flags, ROOM_GODS_ONLY)
  ||   IS_SET(victim->in_room->room_flags, ROOM_IMP_ONLY))
  {
    send_to_char("Your message can not get through to that person right now.\n\r", ch);
    return;
  }

  if (ch == victim)
    send_to_char("It would be pointless to send messages to yourself.\n\r", ch);
  else
  {
    ch->mana -= 30;

    act( "$n raises $s hands in the air, uttering an incantation that creates", ch, NULL, NULL, TO_ROOM );
    act( "a gusting wind that swirls around $m as $e then whispers a few words", ch, NULL, NULL, TO_ROOM );
    act( "into the wind and waves $s hand, as if dismissing its presence.  The", ch, NULL, NULL, TO_ROOM );
    act( "wind suddenly disappears as quickly as it had been conjured.", ch, NULL, NULL, TO_ROOM );

    send_to_char("\n\rYou raise your hands in the air, creating a whirling wind to surround you.\n\rAs the wind picks up momentum, you look up and whisper your message into\n\rthe swirling air.  The air then twirls above your head and into the sky,\n\rcarrying forth your message to its intended recipient.\n\r", ch);

    act_new("A strong wind begins to pick up around you, whispering words into your ear.",ch,argument,victim,TO_VICT,POS_DEAD);
    act_new("The wind whispers, 'I have been summoned by {c$n{x to bring you a message!'",ch,argument,victim,TO_VICT,POS_DEAD);
    act_new("The wind continues, 'The message is: {y$t{x'",ch,argument,victim,TO_VICT,POS_DEAD);

    WAIT_STATE(ch,2*PULSE_VIOLENCE);
  }
  return;
}

