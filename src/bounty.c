/***************************************************************************
*  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,         *
*  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.    *
*                                                                          *
*  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael           *
*  Chastain, Michael Quan, and Mitchell Tse.                               *
*                                                                          *
*  In order to use any part of this Merc Diku Mud, you must comply with    *
*  both the original Diku license in 'license.doc' as well the Merc        *
*  license in 'license.txt'.  In particular, you may not remove either of  *
*  these copyright notices.                                                *
*                                                                          *
*  Much time and thought has gone into this software and you are           *
*  benefitting.  We hope that you share your changes too.  What goes       *
*  around, comes around.                                                   *
****************************************************************************/

/***************************************************************************
*       ROM 2.4 is copyright 1993-1998 Russ Taylor                         *
*       ROM has been brought to you by the ROM consortium                  *
*           Russ Taylor (rtaylor@hypercube.org)                            *
*           Gabrielle Taylor (gtaylor@hypercube.org)                       *
*           Brian Moore (zump@rom.org)                                     *
*       By using this code, you have agreed to follow the terms of the     *
*       ROM license, in the file Rom24/doc/rom.license                     *
****************************************************************************/

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "merc.h"
#include "interp.h"
#include "recycle.h"

/* Global variables */
BOUNTY_DATA	*bounty_list;

/* Command functions needed */
DECLARE_DO_FUN(do_say	);

/* And some local defines */
#define BOUNTY_PLACE_CHARGE	3
#define BOUNTY_REMOVE_CHARGE	100
#define BOUNTY_ADD_CHARGE	5
#define MIN_BOUNTY		1000

void save_bounties()
{
  FILE *fp;
  BOUNTY_DATA *bounty;
  bool found = FALSE;

  fclose(fpReserve);
  if ( ( fp = fopen( BOUNTY_FILE, "w" ) ) == NULL )
  {
    perror( BOUNTY_FILE );
  }

  for( bounty = bounty_list; bounty != NULL; bounty = bounty->next)
  {
    found = TRUE;
    fprintf(fp,"%-12s %d\n",bounty->name,bounty->amount);
  }

  fclose(fp);
  fpReserve = fopen( NULL_FILE, "r" );
  if(!found)
    unlink(BOUNTY_FILE);
}

void load_bounties()
{
  FILE *fp;
  BOUNTY_DATA *blist;

  if ( ( fp = fopen( BOUNTY_FILE, "r" ) ) == NULL )
	return;

  blist = NULL;
  for( ; ; )
  {
    BOUNTY_DATA *bounty;
    if ( feof(fp) )
    {
      fclose(fp);
      return;
    }

    bounty = new_bounty();

    bounty->name = str_dup(fread_word(fp));
    bounty->amount = fread_number(fp);
    fread_to_eol(fp);

    if(bounty_list == NULL)
	bounty_list = bounty;
    else
	blist->next = bounty;
    blist = bounty;
  }
}

bool is_bountied(CHAR_DATA *ch)
{
  BOUNTY_DATA *bounty;
  bool found = FALSE;

  if(IS_NPC(ch))
    return FALSE;

  for(bounty = bounty_list; bounty != NULL; bounty = bounty->next)
  {
    if( !str_cmp(ch->name,bounty->name) )
      found = TRUE;
  }

  return found;
}

void do_bounty(CHAR_DATA *ch, char *argument)
{
  FILE *fp;
  CHAR_DATA *vch;
  BOUNTY_DATA *bounty;
  BOUNTY_DATA *prev=NULL;
  char arg[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  char strsave[MAX_STRING_LENGTH];
  int cash, amount, surcharge;

  if( IS_NPC(ch) )
    return;

  /* Check for the bounty collector */
  for(vch = ch->in_room->people; vch ; vch = vch->next_in_room)
  {
    if(IS_NPC(vch) && IS_SET(vch->act, ACT_BOUNTY))
      break;
  }

  if(vch == NULL)
  {
    send_to_char("You cannot do that here.\n\r",ch);
    return;
  }

  /* Only clanned chars/pkillers can place bounties. */
  if( ch->org_id == ORG_NONE )
  {
    do_say(vch,"You must be in a clan to place or collect a bounty.");
    return;
  }

  if( IS_SET(ch->comm, COMM_NOPKILL) )
  {
	send_to_char("You find yourself unable to set a bounty per your PK Restrictions.\n\r",ch);
	return;
   }

  argument = one_argument(argument, arg);
  one_argument(argument, arg2);

  if(arg[0] == '\0')
  {
    do_say(vch,"These are the bounties I will honor:");
    send_to_char("  Name              Bounty\n\r",ch);
    send_to_char("  ============      ===============\n\r",ch);
    for(bounty = bounty_list;bounty != NULL;bounty=bounty->next)
    {
      sprintf(buf,"  %-12s      %d\n\r",bounty->name,bounty->amount);
      send_to_char(buf,ch);
    }

    return;
  }

  if( arg2[0] == '\0' 
  ||  (str_cmp(arg,"remove") && str_cmp(arg,"collect") && !is_number(arg2)) )
  {
    send_to_char("Syntax: bounty\n\r",ch);
    send_to_char("        bounty remove <player>\n\r",ch);
    send_to_char("        bounty collect <player>\n\r",ch);
    send_to_char("        bounty <player> <amount>\n\r",ch);
    return;
  }

  /* For collecting bounties. */
  if( !str_cmp(arg,"collect") )
  {
    OBJ_DATA *obj;

    for(bounty = bounty_list; bounty != NULL; prev=bounty, bounty=bounty->next)
    {
      if( !str_cmp(arg2,bounty->name) )
	break;
    }

    if( bounty == NULL )
    {
      do_say(vch,"That individual is not in the books.");
      return;
    }

    for(obj = ch->carrying; obj != NULL; obj = obj->next)
    {
      if( is_name(arg2,obj->name)
      &&  (obj->pIndexData->vnum == OBJ_VNUM_SEVERED_HEAD) )
	break;
    }

    if( obj == NULL
    &&  bounty != NULL
    && !IS_IMMORTAL(ch))
    {
      do_say(vch,"I'm afraid I'll need proof of the death.");
      return;
    }

    if (!IS_IMMORTAL(ch))
    {
      extract_obj(obj);
      do_say(vch,"Very well, here is your payment.");
      ch->steel += bounty->amount;
    }
    else
    {
      do_say(vch,"Yes all powerful one.  I shall remove them from the books right away!");
    }

    /*
     * OK, we've got a valid bounty and they've
     * been paid.  Time to remove the bounty.
     */
    if(prev == NULL)
      bounty_list = bounty_list->next;
    else
      prev->next = bounty->next;

    free_bounty(bounty);
    save_bounties();
    return;
  }

  /* For buying off bounties. */
  if( !str_cmp(arg,"remove") )
  {
    for(bounty = bounty_list; bounty != NULL; prev=bounty, bounty=bounty->next)
    {
      if( !str_cmp(arg2,bounty->name) )
	break;
    }

    if( bounty == NULL )
    {
      do_say(vch,"That individual is not in the books.");
      return;
    }

    surcharge = BOUNTY_REMOVE_CHARGE;
    amount = bounty->amount+(bounty->amount*surcharge/100);
    cash = ch->steel + (ch->gold/100);

    if(!IS_IMMORTAL(ch))
    {
      if(cash < amount)
      {
        do_say(vch,"You cannot afford to remove that bounty.");
        return;
      }

      if(ch->gold < (amount*100))
      {
        amount -= (ch->gold/100);
        ch->gold %= 100;
      }
      else
      {
        ch->gold -= amount*100;
        amount = 0;
      }

      if(ch->steel < amount)
      {
        amount -= ch->steel;
        ch->steel = 0;
      }
      else
      {
        ch->steel -= amount;
        amount = 0;
      }
    }

   /*
    * OK, we've got a valid bounty and they can
    * pay.  Time to remove the bounty.
    */
    if(prev == NULL)
      bounty_list = bounty_list->next;
    else
      prev->next = bounty->next;

    free_bounty(bounty);
    do_say(vch,"The name has been removed from the books.");
    save_bounties();
    return;
  }

  /*
   * This code allows you to bounty those
   * offline as well as those not visible.
   */

  fclose(fpReserve);
  sprintf(strsave, "%s%s", PLAYER_DIR, capitalize(arg));
  if ( ( fp = fopen( strsave, "r" ) ) == NULL )
  {
    do_say(vch,"I am afraid there is no such person.");
    fpReserve = fopen( NULL_FILE, "r" );
    return;
  }

  strcpy(arg,capitalize(arg));

  fclose( fp );
  fpReserve = fopen( NULL_FILE, "r" );

  /* Minimum bounty, change as desired */
  if( (amount = atoi(arg2)) < MIN_BOUNTY)
  {
    sprintf(buf,"I do not accept bounties under %d steel.",MIN_BOUNTY);
    do_say(vch,buf);
    return;
  }

  /*
   * Char's cash total in gold, bounties
   * are always set in gold. Notice the
   */
  cash = ch->steel + (ch->gold/100);

  /*
   * Does the bounty already exist?
   * Let's find out.
   */
  for(bounty = bounty_list;bounty != NULL;bounty = bounty->next)
  {
    if( !str_cmp(bounty->name,arg) )
      break;
  }

  /*
   * Either create a new bounty
   * or add to an existing one.
   * Notice the charge added to
   * place a bounty and the
   * additional one to add to a
   * bounty.  Don't want to make
   * it too cheap.  Again, change
   * as desired.
   */

  if(bounty == NULL)
  {
    surcharge = BOUNTY_PLACE_CHARGE;
    if(cash < (amount+(amount*surcharge/100)))
    {
      do_say(vch,"You cannot afford to place that bounty.");
      return;
    }
    bounty = new_bounty();
    bounty->name = str_dup(arg);
    bounty->amount = amount;
    bounty->next = bounty_list;
    bounty_list = bounty;
    amount += amount*surcharge/100;
  }
  else
  {
    sprintf(buf,"There is a %d%% surcharge to add to an existing bounty.",BOUNTY_ADD_CHARGE);
    do_say(vch,buf);
    surcharge = BOUNTY_PLACE_CHARGE + BOUNTY_ADD_CHARGE;
    amount += amount*surcharge/100;
    if(cash < amount)
    {
      do_say(vch,"You cannot afford to place that bounty.");
      return;
    }
    bounty->amount += amount;
  }

  save_bounties();

  /*
   * Now make them pay, start with bank
   * then go to cash on hand.  Also use
   * up that annoying silver first.
   */
  if(ch->gold < (amount*100))
  {
    amount -= (ch->gold/100);
    ch->gold %= 100;
  }
  else
  {
    ch->gold -= amount*100;
    amount = 0;
  }

  if(ch->steel < amount)
  {
    amount -= ch->gold;
    ch->steel = 0;
  }
  else
  {
    ch->steel -= amount;
    amount = 0;
  }

  do_say(vch,"Your bounty has been recorded in the books.");
}

