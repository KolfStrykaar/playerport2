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
*       ROM 2.4 is copyright 1993-1995 Russ Taylor                         *
*       ROM has been brought to you by the ROM consortium                  *
*           Russ Taylor (rtaylor@pacinfo.com)                              *
*           Gabrielle Taylor (gtaylor@pacinfo.com)                         *
*           Brian Moore (rom@rom.efn.org)                                  *
*       By using this code, you have agreed to follow the terms of the     *
*       ROM license, in the file Rom24/doc/rom.license                     *
***************************************************************************/

/****************************************************************************
*  Automated Quest code written by Vassago of MOONGATE, moongate.ams.com    *
*  4000. Copyright (c) 1996 Ryan Addams, All Rights Reserved. Use of this   *
*  code is allowed provided you add a credit line to the effect of:         *
*  "Quest Code (c) 1996 Ryan Addams" to your logon screen with the rest     *
*  of the standard diku/rom credits. If you use this or a modified version  *
*  of this code, let me know via email: moongate@moongate.ams.com. Further  *
*  updates will be posted to the rom mailing list. If you'd like to get     *
*  the latest version of quest.c, please send a request to the above add-   *
*  ress. Quest Code v2.03. Please do not remove this notice from this file. *
****************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"

DECLARE_DO_FUN( do_say );
void	quest_sell	args( (CHAR_DATA *ch, CHAR_DATA *qm, char *argument) );

/* Object vnums for Quest Rewards */

#define QUEST_ITEM1 1206
#define QUEST_ITEM2 1207
#define QUEST_ITEM3 1208
#define QUEST_ITEM4 1209
#define QUEST_ITEM5 1211
#define QUEST_ITEM6 1218

/* Object vnums for object quest 'tokens'. In Moongate, the tokens are
   things like 'the Shield of Moongate', 'the Sceptre of Moongate'. These
   items are worthless and have the rot-death flag, as they are placed
   into the world when a player receives an object quest. */

#define QUEST_OBJQUEST1  1201
#define QUEST_OBJQUEST2  1202
#define QUEST_OBJQUEST3  1203
#define QUEST_OBJQUEST4  1204
#define QUEST_OBJQUEST5  1205

/* Local functions */

void generate_quest     args(( CHAR_DATA *ch, CHAR_DATA *questman ));
void quest_update       args(( void ));
bool quest_level_diff   args(( int clevel, int mlevel));
bool chance             args(( int num ));
ROOM_INDEX_DATA         *find_location( CHAR_DATA *ch, char *arg );

/* CHANCE function. I use this everywhere in my code, very handy :> */

bool chance(int num)
{
    if (number_range(1,100) <= num) return TRUE;
    else return FALSE;
}

/* The main quest function */

void do_quest(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *questman;
    OBJ_DATA *obj=NULL, *obj_next;
    OBJ_INDEX_DATA *questinfoobj;
    MOB_INDEX_DATA *questinfo;
    char buf [MAX_STRING_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (arg1[0] == '\0')
    {
      send_to_char("\n\rQUEST commands: POINTS|INFO|TIME|REQUEST|COMPLETE|LIST|BUY|QUIT|SELL.\n\r",ch);
      send_to_char("\n\rFor more information, type 'HELP QUEST'.\n\r",ch);
      return;
    }
    if (!strcmp(arg1, "points"))
    {
	sprintf(buf, "You have %d Quest Points.\n\r",ch->questpoints);
	send_to_char(buf, ch);
	return;
    }
    else if (!strcmp(arg1, "info"))
    {
      if (IS_SET(ch->act, PLR_QUESTOR))
      {
        if (ch->questmob == -1 && ch->questgiver->short_descr != NULL)
        {
          sprintf(buf, "\n\rYour quest is ALMOST complete!\n\r");
          send_to_char(buf, ch);
          sprintf(buf, "\n\rGet back to %s before your time runs out!\n\r",
          ch->questgiver->short_descr);
          send_to_char(buf, ch);
        }
        else if (ch->questobj > 0)
        {
          questinfoobj = get_obj_index(ch->questobj);
          if (questinfoobj != NULL)
          {
            sprintf(buf, "\n\rYou are on a quest to recover the fabled %s!\n\r",
            questinfoobj->name);
            send_to_char(buf, ch);
            send_to_char("It is now unknown where it may be!\n\r",ch);
          }
          else 
            send_to_char("You aren't currently on a quest.\n\r",ch);
            return;
        }
        else if (ch->questmob > 0)
        {
          questinfo = get_mob_index(ch->questmob);
          if (questinfo != NULL)
          {
            sprintf(buf, "\n\rYou are on a quest to slay the dreaded %s!\n\r",
            questinfo->short_descr);
            send_to_char(buf, ch);
            sprintf(buf, "%s was last seen in the vicinity of %s!\n\r",
	    questinfo->short_descr,
            questinfo->area->name);
            send_to_char(buf, ch);
          }
          else
            send_to_char("You aren't currently on a quest.\n\r",ch);
          return;
          }
        }
        else
            send_to_char("You aren't currently on a quest.\n\r",ch);
        return;
    }
    else if (!strcmp(arg1, "time"))
    {
      if (!IS_SET(ch->act, PLR_QUESTOR))
      {
        send_to_char("You aren't currently on a quest.\n\r",ch);
        if (ch->nextquest > 1)
        {
          sprintf(buf, "There are %d minutes remaining until you can go on another quest.\n\r",
          ch->nextquest);
          send_to_char(buf, ch);
        }
        else
        if (ch->nextquest == 1)
        {
          sprintf(buf, "There is less than a minute remaining until you can go on another quest.\n\r");
          send_to_char(buf, ch);
        }
      }
      else
      if (ch->countdown > 0)
      {
        sprintf(buf, "Time left for current quest: %d\n\r",
        ch->countdown);
        send_to_char(buf, ch);
      }
      return;
    }

    for ( questman = ch->in_room->people; questman != NULL; questman = questman->next_in_room )
    {
        if (!IS_NPC(questman)) continue;
        if (questman->spec_fun == spec_lookup( "spec_questmaster" )) break;
    }

    if (questman == NULL || questman->spec_fun != spec_lookup( "spec_questmaster" ))
    {
        send_to_char("You can't do that here.\n\r",ch);
        return;
    }

    if ( questman->fighting != NULL)
    {
        send_to_char("Wait until the fighting stops.\n\r",ch);
        return;
    }

    ch->questgiver = questman;

    if( !str_prefix( arg1, "sell" ) )
    {
      if (ch->position < POS_RESTING)
      {
        send_to_char( "In your dreams, or what?\n\r", ch );
        return;
      }
	quest_sell(ch,questman,arg2);
	return;
    }

    if (!strcmp(arg1, "list"))
    {

      if (ch->position < POS_RESTING)
      {
        send_to_char( "In your dreams, or what?\n\r", ch );
        return;
      }

      act( "$n asks $N for a list of quest items.", ch, NULL, questman, TO_ROOM);
      act ("You ask $N for a list of quest items.",ch, NULL, questman, TO_CHAR);
      sprintf(buf, "Current Quest Items available for Purchase:\n\r\
              1000qp.........2000 Experience Points\n\r\
              700qp..........350,000 Gold Pieces\n\r\
              500qp..........A Sleeping Bag\n\r\
              500qp..........A Make Over coupon\n\r\
              200qp..........A True Faith coupon\n\r\
              100qp..........A Quest note\n\r\
              50qp...........A Home coupon\n\r\
              35qp...........A Restring coupon\n\r\
        ** To buy an item, type 'QUEST BUY <item>'.\n\r");
        send_to_char(buf, ch);
        return;
    }
    else 
    if (!strcmp(arg1, "buy"))
    {

      if (ch->position < POS_RESTING)
      {
        send_to_char( "In your dreams, or what?\n\r", ch );
        return;
      }

      if (arg2[0] == '\0')
      {
        send_to_char("To buy an item, type 'QUEST BUY <item>'.\n\r",ch);
        return;
      }

      if (is_name(arg2, "restring"))
      {
        if (ch->questpoints >= 35)
        {
          ch->questpoints -= 35;
          obj = create_object(get_obj_index(QUEST_ITEM1),ch->level);
          send_to_char("You successfully purchase a RESTRING coupon.\n\r",ch);
	    send_to_char("\n\r",ch);
        }
        else
        {
          sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.",
          ch->name);
          do_say(questman,buf);
          return;
        }
      }
      else
      if (is_name(arg2, "home"))
      {
        if (ch->questpoints >= 50)
        {
          ch->questpoints -= 50;
          obj = create_object(get_obj_index(QUEST_ITEM2),ch->level);
	    send_to_char("You successfully purchase a HOME coupon.\n\r",ch);
	    send_to_char("\n\r",ch);
        }
        else
        {
          sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.",
          ch->name);
          do_say(questman,buf);
          return;
        }
      }
      else
      if (is_name(arg2, "true faith"))
      {
        if (ch->questpoints >= 200)
        {
          ch->questpoints -= 200;
          obj = create_object(get_obj_index(QUEST_ITEM3),ch->level);
	    send_to_char("You successfully purchase a TRUE FAITH coupon.\n\r",ch);
	    send_to_char("\n\r",ch);
        }
        else
        {
          sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.",
          ch->name);
          do_say(questman,buf);
          return;
        }
      }
      else
      if (is_name(arg2, "make over"))
      {
        if (ch->questpoints >= 500)
        {
          ch->questpoints -= 500;
          obj = create_object(get_obj_index(QUEST_ITEM4),ch->level);
	    send_to_char("You successfully purchase a MAKE OVER coupon.\n\r",ch);
	    send_to_char("\n\r",ch);
        }
        else
        {
          sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.",
          ch->name);
          do_say(questman,buf);
          return;
        }
      }
      else if (is_name(arg2, "quest note"))
      {
        if (ch->questpoints >= 100)
        {
          ch->questpoints -= 100;
          obj = create_object(get_obj_index(QUEST_ITEM5),ch->level);
	    send_to_char("You successfully purchase a QUEST NOTE.\n\r",ch);
	    send_to_char("\n\r",ch);
        }
        else
        {
          sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.",
          ch->name);
          do_say(questman,buf);
          return;
        }
      }
      else if (is_name(arg2, "sleeping bag"))
      {
        if (ch->questpoints >= 500)
        {
          ch->questpoints -= 500;
          obj = create_object(get_obj_index(QUEST_ITEM6),ch->level);
	    send_to_char("You successfully purchase a SLEEPING BAG.\n\r",ch);
	    send_to_char("\n\r",ch);
        }
        else
        {
          sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.",
          ch->name);
          do_say(questman,buf);
          return;
        }
      }
      else if (is_name(arg2, "gold pieces"))
      {
        if (ch->questpoints >= 700)
        {
          ch->questpoints -= 700;
          ch->gold += 350000;
          act( "$N gives 350,000 gold pieces to $n.", ch, NULL, questman, TO_ROOM );
          act( "$N has 350,000 in gold transfered from $s account to your balance.",
          ch,NULL,questman, TO_CHAR );
          return;
        }
        else
        {
          sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.",
          ch->name);
          do_say(questman,buf);
          return;
        }
      }
      else if (is_name(arg2, "experience points"))
      {
        if (ch->questpoints >= 1000)
        {
          ch->questpoints -= 1000;
          gain_exp(ch, 2000);
	    send_to_char("You have gained an additional 2000 experience points with your purchase.\n\r",ch);
        }
        else
        {
          sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.",
          ch->name);
          do_say(questman,buf);
          return;
        }
      }
      else
      {
        sprintf(buf, "I don't have that item, %s.",ch->name);
        do_say(questman, buf);
      }

      if (obj != NULL)
      {
            act( "$N gives $p to $n.", ch, obj, questman, TO_ROOM );
            act( "$N gives you $p.",   ch, obj, questman, TO_CHAR );
            obj_to_char(obj, ch);
      }
      return;
    }
    else
    if (!strcmp(arg1, "quit"))
    {
      if (!IS_SET(ch->act,PLR_QUESTOR))
      {
	send_to_char("You aren't currently on a quest.\n\r",ch);
	return;
      }
      sprintf(buf,"\n\rYou admit defeat and give up on your quest.\n\r");
      send_to_char(buf, ch);
      sprintf(buf,"\n\rYou may quest again in 1 minute.\n\r");
      send_to_char(buf, ch);
      REMOVE_BIT(ch->act,PLR_QUESTOR);
      ch->questgiver 	= NULL;
      ch->countdown 	= 0;
      ch->questmob 	= 0;
      ch->questobj 	= 0;
      ch->nextquest 	= 1;
      return;
    }
    else
    if (!strcmp(arg1, "request"))
    {

      if (ch->position < POS_RESTING)
      {
        send_to_char( "In your dreams, or what?\n\r", ch );
        return;
      }

      act( "$n asks $N for a quest.", ch, NULL, questman, TO_ROOM);
      act ("You ask $N for a quest.",ch, NULL, questman, TO_CHAR);

      if (IS_SET(ch->act, PLR_QUESTOR))
      {
        sprintf(buf, "But you're already on a quest!");
        do_say(questman, buf);
        return;
      }

      if (ch->nextquest > 0)
      {
        sprintf(buf, "You're very brave, %s, but let someone else have a chance.",
        ch->name);
        do_say(questman, buf);
        sprintf(buf, "Come back later.");
        do_say(questman, buf);
        return;
      }

      sprintf(buf, "Thank you, brave %s!",ch->name);
      do_say(questman, buf);
      ch->questmob = 0;
      ch->questobj = 0;

      generate_quest(ch, questman);

      if (ch->questmob > 0 || ch->questobj > 0)
      {
        ch->countdown = number_range(20,40);
        SET_BIT(ch->act, PLR_QUESTOR);
        sprintf(buf, "You have %d minutes to complete this quest.",
        ch->countdown);
        do_say(questman, buf);
        sprintf(buf, "May the gods go with you!");
        do_say(questman, buf);
      }
      return;
    }
    else
    if (!strcmp(arg1, "complete"))
    {

      if (ch->position < POS_RESTING)
      {
        send_to_char( "In your dreams, or what?\n\r", ch );
        return;
      }

      act( "$n informs $N $e has completed $s quest.", ch, NULL, questman, TO_ROOM);
      act ("You inform $N you have completed $s quest.",ch, NULL, questman, TO_CHAR);

      if (ch->questgiver != questman)
      {
        sprintf(buf, "I never sent you on a quest! Perhaps you're thinking of someone else.");
        do_say(questman,buf);
        return;
      }

      if (IS_SET(ch->act, PLR_QUESTOR))
      {
        if (ch->questmob == -1 && ch->countdown > 0)
        {
          int reward, pointreward;

          reward = number_range(100,700);
          pointreward = number_range(25,75);

          sprintf(buf, "Congratulations on completing your quest!");
          do_say(questman,buf);
          sprintf(buf,"As a reward, I am giving you %d quest points, and %d steel.",
          pointreward,
          reward);
          do_say(questman,buf);

          REMOVE_BIT(ch->act, PLR_QUESTOR);
          ch->questgiver 	= NULL;
          ch->countdown 	= 0;
          ch->questmob 		= 0;
          ch->questobj 		= 0;
          ch->nextquest 	= 10;
          ch->steel 		+= reward;
          ch->questpoints 	+= pointreward;

          return;
        }
        else
        if (ch->questobj > 0 && ch->countdown > 0)
        {
          bool obj_found = FALSE;

          for (obj = ch->carrying; obj != NULL; obj= obj_next)
          {
            obj_next = obj->next_content;

            if (obj != NULL && obj->pIndexData->vnum == ch->questobj)
            {
              obj_found = TRUE;
              break;
            }
          }

        if (obj_found == TRUE)
        {
          int reward, pointreward;

          reward = number_range(100,700);
          pointreward = number_range(25,75);

          act("You hand $p to $N.",ch, obj, questman, TO_CHAR);
          act("$n hands $p to $N.",ch, obj, questman, TO_ROOM);

          sprintf(buf, "Congratulations on completing your quest!");
          do_say(questman,buf);
          sprintf(buf,"As a reward, I am giving you %d quest points, and %d steel.",
          pointreward,
          reward);
          do_say(questman,buf);

          REMOVE_BIT(ch->act, PLR_QUESTOR);
          ch->questgiver = NULL;
          ch->countdown = 0;
          ch->questmob = 0;
          ch->questobj = 0;
          ch->nextquest = 5;
          ch->steel += reward;
          ch->questpoints += pointreward;
          extract_obj(obj);
          return;
        }
        else
        {
          sprintf(buf, "You haven't completed the quest yet, but there is still time!");
          do_say(questman, buf);
          return;
        }
        return;
      }
      else
      if ((ch->questmob > 0 || ch->questobj > 0) && ch->countdown >0)
      {
        sprintf(buf, "You haven't completed the quest yet, but there is still time!");
        do_say(questman, buf);
        return;
      }
    }
    if (ch->nextquest > 0)
      sprintf(buf,"But you didn't complete your quest in time!");
    else 
      sprintf(buf, "You have to REQUEST a quest first, %s.",
      ch->name);
      do_say(questman, buf);
      return;
  }

  send_to_char("\n\rQUEST commands: POINTS|INFO|TIME|REQUEST|COMPLETE|LIST|BUY|QUIT|SELL.\n\r",ch);
  send_to_char("\n\rFor more information, type 'HELP QUEST'.\n\r",ch);
  return;
}

void quest_sell( CHAR_DATA *ch, CHAR_DATA *qm, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number;
    int cost;

    number = mult_argument(argument,arg);
    obj  = get_obj_carry( ch,arg, ch );

    if (obj == NULL)
    {
      send_to_char( "Sell what?\n\r", ch );
      return;
    }

    cost = obj->cost/2;

    if (number < 1)
    {
      act("$n tells you 'Get real!",qm,NULL,ch,TO_VICT);
      return;
    }

    if (number > 1)
    {
      act("$n tells you 'One object at a time please",qm,NULL,ch,TO_VICT);
      return;
    }

    if ( cost < 0 || !can_see_obj( qm, obj ) )
    {
      act( "$n tells you 'I can't see that that object'.", qm, NULL, ch, TO_VICT );
      ch->reply = qm;
      return;
    }

    if ( cost == 0 || cost == 1 )
    {
      act( "$n tells you 'I'm sorry, but the cost of that item is worthless'.", qm, NULL, ch, TO_VICT );
      ch->reply = qm;
      return;
    }

    if (!IS_SET(obj->extra_flags, ITEM_QUESTOBJ))
    {
      sprintf(buf, "But %s isn't a quest object!!\n\r", obj->short_descr);
      send_to_char(buf,ch);
      return;
    }

    sprintf(buf,"$n sells $p.\n\r");
    act(buf,ch,obj,NULL,TO_ROOM);
    sprintf(buf,"You sell $p for %d questpoints.\n\r",
    cost);
    act(buf,ch,obj,NULL,TO_CHAR);
    sprintf(buf, "\n\r");
    send_to_char(buf, ch);
    sprintf (buf, "Since this is a used %s, I shall only give you half of the purchase price for it.",
    obj->short_descr);
    do_say(qm, buf);
    sprintf(buf, "\n\r");
    send_to_char(buf, ch);
    sprintf (buf, "You shall receive %d questpoints.",
    cost);
    do_say(qm, buf);
    sprintf(buf, "\n\r");
    send_to_char(buf, ch);

    ch->questpoints += cost;
    obj_from_char(obj);
}

void generate_quest(CHAR_DATA *ch, CHAR_DATA *questman)
{
    CHAR_DATA *victim;
    ROOM_INDEX_DATA *room;
    ROOM_INDEX_DATA *pRoomIndex;
    OBJ_DATA *questitem;
    char buf [MAX_STRING_LENGTH];

    /*  Randomly selects a mob from the world mob list. If you don't
        want a mob to be selected, make sure it is immune to summon.
        Or, you could add a new mob flag called ACT_NOQUEST. The mob
        is selected for both mob and obj quests, even tho in the obj
        quest the mob is not used. This is done to assure the level
        of difficulty for the area isn't too great for the player. */

    for (victim = char_list; victim != NULL; victim = victim->next)
    {
      if (!IS_NPC(victim)) 
        continue;

      if (quest_level_diff(ch->level, victim->level) == TRUE
      && !IS_SET(victim->imm_flags, IMM_SUMMON)
	&& !IS_SET(victim->imm_flags, IMM_CHARM)
	&& !IS_SET(victim->imm_flags, IMM_MAGIC)
	&& !IS_SET(victim->imm_flags, IMM_WEAPON)
	&& ch->in_room->area->continent == victim->in_room->area->continent
      && victim->pIndexData != NULL
      && victim->pIndexData->pShop == NULL
      && !IS_SET(victim->in_room->room_flags, ROOM_SAFE)
      && (victim->in_room->sector_type != SECT_SOLAMNIC)
      && (victim->in_room->sector_type != SECT_KOT)
      && (victim->in_room->sector_type != SECT_CONCLAVE)
      && (victim->in_room->sector_type != SECT_THIEVES)
      && (victim->in_room->sector_type != SECT_BLACKORDER)
      && (victim->in_room->sector_type != SECT_HOLYORDER)
      && (victim->in_room->sector_type != SECT_AESTHETIC)
      && (victim->in_room->sector_type != SECT_MERCENARY)
      && (victim->in_room->sector_type != SECT_FORESTER)
      && (victim->in_room->sector_type != SECT_UNDEAD)
      && (victim->in_room->sector_type != SECT_ARTISANS)
      && victim->spec_fun != spec_lookup("spec_orgguard")
      && !IS_SET(victim->act, ACT_PET)
      && !IS_SET(victim->affected_by, AFF_CHARM)
	&& (victim->pIndexData != get_mob_index(MOB_VNUM_MIRROR))
	&& (victim->in_room != get_room_index(number_range(9400,9499)))
      && (victim->in_room != get_room_index(number_range(3700,3799))))
        break;
    }

    if ( victim == NULL  )
    {
      do_say(questman, "I'm sorry, but I don't have any quests for you at this time.");
      do_say(questman, "Try again later.");
      ch->nextquest = 1;
      return;
    }

    if ( ( room = find_location( ch, victim->name ) ) == NULL )
    {
      sprintf(buf, "I'm sorry, but I don't have any quests for you at this time.");
      do_say(questman, buf);
      sprintf(buf, "Try again later.");
      do_say(questman, buf);
      ch->nextquest = 1;
      return;
    }

    /*  40% chance it will send the player on a 'recover item' quest. */

    if (chance(40) || ch->class == class_lookup("blacksmith") )
    {
      long objvnum = 0;

      switch(number_range(0,4))
      {
        case 0:
        objvnum = QUEST_OBJQUEST1;
        break;

        case 1:
        objvnum = QUEST_OBJQUEST2;
        break;

        case 2:
        objvnum = QUEST_OBJQUEST3;
        break;

        case 3:
        objvnum = QUEST_OBJQUEST4;
        break;

        case 4:
        objvnum = QUEST_OBJQUEST5;
        break;
      }

      questitem = create_object( get_obj_index(objvnum), ch->level );
      questitem->timer = 40;
      for ( ; ; )
      {
        pRoomIndex = get_room_index( number_range( 700, 17900 ) );

        if (pRoomIndex != NULL)
        if (CAN_ACCESS(pRoomIndex))
            break;
      }

      if ((pRoomIndex->vnum >= 3700)
	&&  (pRoomIndex->vnum <= 3799))
      {
        sprintf(buf, "I'm sorry, but I don't have any quests for you at this time.");
        do_say(questman, buf);
        sprintf(buf, "Try again later.");
        do_say(questman, buf);
        ch->nextquest = 1;
        return;
      }

      if ((pRoomIndex->vnum >= 9400)
	&&  (pRoomIndex->vnum <= 9499))
      {
        sprintf(buf, "I'm sorry, but I don't have any quests for you at this time.");
        do_say(questman, buf);
        sprintf(buf, "Try again later.");
        do_say(questman, buf);
        ch->nextquest = 1;
        return;
      }

      obj_to_room(questitem, pRoomIndex);
      ch->questobj = questitem->pIndexData->vnum;

      sprintf(buf, "Vile pilferers have stolen %s from the royal treasury!",
      questitem->short_descr);
      do_say(questman, buf);
      do_say(questman, "My court wizardess, with her magic mirror, has pinpointed its location.");
      sprintf(buf, "Look in the general area of %s!",
      pRoomIndex->area->name);
      do_say(questman, buf);
      return;
    }

    /* Quest to kill a mob */

    else
    {
      switch(number_range(0,1))
      {
        case 0:
        sprintf(buf, "An enemy of mine, %s, is making vile threats against the crown.",
        victim->short_descr);
        do_say(questman, buf);
        sprintf(buf, "This threat must be eliminated!");
        do_say(questman, buf);
        break;

        case 1:
        sprintf(buf, "Krynn's most heinous criminal, %s, has escaped from the dungeon!",
        victim->short_descr);
        do_say(questman, buf);
        sprintf(buf, "Since the escape, %s has murdered %d civillians!",
        victim->short_descr,
        number_range(2,20));
        do_say(questman, buf);
        do_say(questman,"The penalty for this crime is death, and you are to deliver the sentence!");
        break;
      }

      if (victim->in_room != NULL)
      {
        sprintf(buf, "Seek %s out somewhere in the vicinity of %s!",
        victim->short_descr,
        victim->in_room->name);
        do_say(questman, buf);

        sprintf(buf, "That location is in the general area of %s.",
	victim->in_room->area->name);
        do_say(questman, buf);
      }
      ch->questmob = victim->pIndexData->vnum;
    }
  return;
}

bool quest_level_diff(int clevel, int mlevel)
{
    if (clevel < 6 && mlevel < 6) return TRUE;
    else if (clevel < 10 && mlevel < 10) return TRUE;
    else if (clevel >  9 && clevel < 15 && mlevel > 9 && mlevel < 15) return TRUE;
    else if (clevel > 14 && clevel < 20 && mlevel > 14 && mlevel < 20) return TRUE;
    else if (clevel > 19 && clevel < 25 && mlevel > 19 && mlevel < 25) return TRUE;
    else if (clevel > 24 && clevel < 30 && mlevel > 24 && mlevel < 30) return TRUE;
    else if (clevel > 29 && clevel < 35 && mlevel > 29 && mlevel < 35) return TRUE;
    else if (clevel > 34 && clevel < 40 && mlevel > 34 && mlevel < 40) return TRUE;
    else if (clevel > 39 && clevel < 45 && mlevel > 39 && mlevel < 45) return TRUE;
    else if (clevel > 44 && clevel < 50 && mlevel > 44 && mlevel < 50) return TRUE;
    else if (clevel > 49 && clevel < 55 && mlevel > 49 && mlevel < 55) return TRUE;
    else if (clevel > 54 && clevel < 60 && mlevel > 54 && mlevel < 60) return TRUE;
    else if (clevel > 59 && clevel < 65 && mlevel > 59 && mlevel < 65) return TRUE;
    else if (clevel > 64 && clevel < 70 && mlevel > 64 && mlevel < 70) return TRUE;
    else if (clevel > 69 && clevel < 75 && mlevel > 69 && mlevel < 75) return TRUE;
    else if (clevel > 74 && clevel < 80 && mlevel > 74 && mlevel < 80) return TRUE;
    else if (clevel > 79 && clevel < 85 && mlevel > 79 && mlevel < 85) return TRUE;
    else if (clevel > 84 && clevel < 90 && mlevel > 84 && mlevel < 90) return TRUE;
    else if (clevel > 89 && clevel < 95 && mlevel > 89 && mlevel < 95) return TRUE;
    else if (clevel > 94 && clevel < 100 && mlevel > 94 && mlevel < 100) return TRUE;
    else if (clevel > 99 && clevel < 110 && mlevel > 99 && mlevel < 110) return TRUE;
    else return FALSE;
}
                
/* Called from update_handler() by pulse_area */

void quest_update(void)
{
  DESCRIPTOR_DATA *d;
  CHAR_DATA *ch;

  for ( d = descriptor_list; d != NULL; d = d->next )
  {
    if (d->character != NULL && d->connected == CON_PLAYING)
    {
      ch = d->character;

      if (ch->nextquest > 0)
      {
        ch->nextquest--;
        if (ch->nextquest == 0)
        {
          send_to_char("You may now quest again.\n\r",ch);
          return;
        }
      }
      else if (IS_SET(ch->act,PLR_QUESTOR))
      {
        if (--ch->countdown <= 0)
        {
          char buf [MAX_STRING_LENGTH];

          ch->nextquest = 2;
          sprintf(buf, "\n\rYou have run out of time for your quest!\n\r");
          send_to_char(buf, ch);
          sprintf(buf, "\n\rYou may quest again in %d minutes.\n\r",
          ch->nextquest);
          send_to_char(buf, ch);
          REMOVE_BIT(ch->act, PLR_QUESTOR);
          ch->questgiver = NULL;
          ch->countdown = 0;
          ch->questmob = 0;
        }

        if (ch->countdown > 0 && ch->countdown < 6)
        {
          send_to_char("\n\rBetter hurry, you're almost out of time for your quest!\n\r",ch);
          return;
        }
      }
    }
  }
  return;
}

