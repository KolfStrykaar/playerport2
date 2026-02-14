/* LoK (The Legends of Krynn) Blacksmith class coded by Shaun Marquardt 2003-2004
 * with serious modifications and additions by Craig Walters 2005
 * Blacksmith Skills and Spells are to be coded in this C file - Shaun
 */

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
#include "tables.h"

extern  AFFECT_DATA	*affect_free;

bool is_chain	args( ( OBJ_DATA *obj ) );
bool is_plate	args( ( OBJ_DATA *obj ) );
bool is_leather	args( ( OBJ_DATA *obj ) );
int fast_working	args( ( CHAR_DATA *ch, int sn ) );
bool bs_exp		args( ( CHAR_DATA *ch, int sn ) );

int fast_working( CHAR_DATA *ch, int sn )
{
	int wait = 0;

	if( get_skill(ch,gsn_fast_working) >= 1)
	{
		if( number_percent() > get_skill(ch,gsn_fast_working) )
		{
			check_improve(ch,gsn_fast_working,FALSE,1);
			wait = skill_table[sn].beats;
		}
		else
		{
			check_improve(ch,gsn_fast_working,TRUE,1);
			wait = skill_table[sn].beats*3/4;
		}
	}
	else
	{
		wait = skill_table[sn].beats;
	}

	if(wait == 0) wait = skill_table[sn].beats;

	return wait;

}

/* Function to award smiths who work more, with more experience.
 * After all, a smith that works all the time, is more experienced then a smith
 * who just sits around on his duff all day. HOWEVER; we do NOT give exp
 * if the smith has the skill at 100% because there's nothing left to learn.
 */
bool bs_exp( CHAR_DATA* ch, int sn )
{
	if( IS_NPC(ch) )
		return FALSE;

	if( ch->pcdata->learned[sn] == 100 )
		return FALSE;

	if( number_percent() < 20 )
		return TRUE;

	return FALSE;
}

void do_mend( CHAR_DATA *ch, char *argument )
{
  DESCRIPTOR_DATA *d;
  char arg[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH]; 
  char buf[MAX_STRING_LENGTH];
  OBJ_DATA *obj, *raw_mat, *hold;
  OBJ_DATA *obj2;
  OBJ_DATA *anvil = NULL;
  int cond, steel, wait;

  argument = one_argument(argument, arg);
  one_argument(argument, arg2);

  if(IS_NPC(ch))
  {
    send_to_char("Mobs cannot repair items!\n\r",ch);
    return;
  }

  if ( get_skill(ch, gsn_mend) < 1 )
  {
    send_to_char("You don't know how to do that.\n\r", ch );
    return;
  }

  if ( arg[0] == '\0' )
  {
    send_to_char("Syntax: mend <object> <raw material>\n\r",ch);
    send_to_char("Example: mend sword steel\n\r",ch);
    return;
  }

  hold = get_eq_char(ch, WEAR_WIELD);
  if( (hold == NULL) || (hold->item_type != ITEM_BHAMMER) )
  {
    send_to_char("You must be wielding your blacksmithing hammer to mend an object.\n\r",ch);
    return;
  }
  
  for (obj2 = ch->in_room->contents; obj2 != NULL; obj2 = obj2->next_content)
  {
    if( obj2->item_type == ITEM_ANVIL )
      anvil = obj2;
  }

  if (anvil == NULL)
  {
    send_to_char("You require the need for an Anvil to mend items.\n\r",ch);
    send_to_char("Make sure there's one here in the room with you and it's on the ground.\n\r",ch);
    return;
  }

  if( (obj = get_obj_carry(ch, arg, ch)) == NULL )
  {
    send_to_char("You do not have that item to mend.\n\r",ch);
    return;
  }

  if( obj->item_type != ITEM_WEAPON && obj->item_type != ITEM_ARMOR )
  {
    send_to_char("You can only mend weapons and armor.\n\r",ch);
    return;
  }

  if( arg2[0] == '\0' )
  {
    act("Mend $p with what?",ch,obj,NULL,TO_CHAR);	
    return;
  }

  if( (raw_mat = get_obj_carry(ch, arg2, ch)) == NULL )
  {
    send_to_char("You do not have those raw materials.\n\r",ch);
    return;
  }

  if( raw_mat->item_type != ITEM_RAW_MATERIAL)
  {
    act("You cannot use $p for raw materials.",ch,raw_mat,NULL,TO_CHAR);
    return;
  }

  if( str_cmp(obj->material, raw_mat->material) )
  {
    sprintf( buf, "You cannot use %s to mend %s,\n\ras they are made of different materials.\n\r",
    raw_mat->short_descr, obj->short_descr);
    send_to_char(buf, ch);
    return;
  }

  cond = obj->condition;

  if(cond == 100)
  {
    act("$p isn't in any need of mending.\n\r",ch,obj,NULL,TO_CHAR);
    return;
  }

  if(cond >= 90) steel = 10;
  else if( (cond >= 80) && (cond <= 89) ) steel = 20;
  else if( (cond >= 70) && (cond <= 79) ) steel = 40;
  else if( (cond >= 60) && (cond <= 69) ) steel = 60;
  else if( (cond >= 50) && (cond <= 59) ) steel = 80;
  else if( (cond >= 40) && (cond <= 49) ) steel = 100;
  else if( (cond >= 30) && (cond <= 39) ) steel = 120;
  else if( (cond >= 21) && (cond <= 29) ) steel = 140;
  else if( (cond >= 11) && (cond <= 20) ) steel = 160;
    else steel = 100;

  if(ch->steel < steel)
  {
    send_to_char("You do not have enough steel money to repair this object.\n\r",ch);
    return;
  }

/* check for fast working */
  wait = fast_working(ch, gsn_mend);

/* Ok! Now, we got everything! Let's get the show on the road. */

  act("$n begins to make repairs on $p.",ch,obj,NULL,TO_ROOM);
  WAIT_STATE(ch, wait);

  for ( d = descriptor_list; d != NULL; d = d->next )
  {
    if ( d->connected == CON_PLAYING
    &&   d->character != ch
    &&   d->character->in_room != NULL
    &&   d->character->in_room != ch->in_room
    &&   d->character->in_room->area == ch->in_room->area
    &&   !IS_SET(d->character->in_room->room_flags, ROOM_QUIET)
    &&   !IS_AFFECTED2(d->character, AFF_DEAFNESS) 
    &&   number_percent() < 5 )
    {
      act("The sounds of hammering can be heard in the distance.",ch,argument,d->character,TO_VICT);
    }
  }

  if ( number_percent() > get_skill(ch,gsn_mend))
  {
    act("You fail to mend $p.",ch,obj,NULL,TO_CHAR);
    ch->steel -= steel;
    extract_obj(raw_mat);
    act("$n fails to mend $p.",ch,obj,NULL,TO_ROOM);

    if( number_percent() <= 10 )
    {
	send_to_char("Oops! you broke it, instead of fixing it! It's gone now!!\n\r",ch);
	act("$n accidentally breaks $p!",ch,obj,NULL,TO_ROOM);
	extract_obj(obj);
    }
    check_improve(ch,gsn_mend,FALSE,1);
    return;
  }
  else
  {
	act("You successfully repair $p.",ch,obj,NULL,TO_CHAR);
	extract_obj(raw_mat);
	ch->steel -= steel;
	obj->condition = 100;
	check_improve(ch,gsn_mend,TRUE,1);
	if( bs_exp(ch, gsn_mend) ) gain_exp(ch, 50);
	act("$n successfully makes repairs to $p.",ch,obj,NULL,TO_ROOM);
	return;
  }
  return;
}

void do_repair_shield( CHAR_DATA *ch, char *argument )
{
	DESCRIPTOR_DATA *d;
	char arg[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	OBJ_DATA *obj, *raw_mat, *hold;
      OBJ_DATA *obj2;
      OBJ_DATA *anvil = NULL;
	int cond, steel, wait;

	argument = one_argument(argument, arg);
	one_argument(argument, arg2);

  	if(IS_NPC(ch))
  	{
  		send_to_char("Mobs cannot repair items!\n\r",ch);
  		return;
  	}

	if ( get_skill(ch, gsn_repair_shield) < 1 )
	{
		send_to_char("You don't know how to do that.\n\r", ch );
		return;
	}

	if ( arg[0] == '\0' )
	{
		send_to_char("Syntax: repair <shield> <raw material>\n\r",ch);
		send_to_char("Example: repair shield steel\n\r",ch);
		return;
	}

	hold = get_eq_char(ch, WEAR_WIELD);
	if( (hold == NULL) || (hold->item_type != ITEM_BHAMMER) )
	{
		send_to_char("You must be wielding your blacksmithing hammer to repair shields.\n\r",ch);
		return;
	}

      for (obj2 = ch->in_room->contents; obj2 != NULL; obj2 = obj2->next_content)
      {
        if( obj2->item_type == ITEM_ANVIL )
		  anvil = obj2;
      }

      if (anvil == NULL)
      {
        send_to_char("You require the need for an Anvil to repair items.\n\r",ch);
        send_to_char("Make sure there's one here in the room with you and it's on the ground.\n\r",ch);
        return;
      }

	if( (obj = get_obj_carry(ch, arg, ch)) == NULL )
	{
		send_to_char("You do not have that shield to mend.\n\r",ch);
		return;
	}

	if( obj->item_type != ITEM_ARMOR || !IS_SET(obj->wear_flags, ITEM_WEAR_SHIELD) )
	{
		send_to_char("You can only repair a shield.\n\r",ch);
		return;
	}

	if( arg2[0] == '\0' )
	{
		act("Repair $p with what?",ch,obj,NULL,TO_CHAR);
		return;
	}

	if( (raw_mat = get_obj_carry(ch, arg2, ch)) == NULL )
	{
		send_to_char("You do not have those raw materials.\n\r",ch);
		return;
	}

	if( raw_mat->item_type != ITEM_RAW_MATERIAL)
	{
		act("You cannot use $p for raw materials.",ch,raw_mat,NULL,TO_CHAR);
		return;
	}

	if( str_cmp(obj->material, raw_mat->material) )
	{
		sprintf( buf, "You cannot use %s to repair %s,\n\ras they are made of different materials.\n\r",raw_mat->short_descr, obj->short_descr);
		send_to_char(buf, ch);
		return;
	}

	cond = obj->condition;

	if(cond == 100)
	{
		act("$p isn't in any need of repariing.",ch,obj,NULL,TO_CHAR);
		return;
	}

	if(cond >= 90) steel = 1;
	else if( (cond >= 80) && (cond <= 89) ) steel = 2;
	else if( (cond >= 70) && (cond <= 79) ) steel = 4;
	else if( (cond >= 60) && (cond <= 69) ) steel = 6;
	else if( (cond >= 50) && (cond <= 59) ) steel = 8;
	else if( (cond >= 40) && (cond <= 49) ) steel = 10;
	else if( (cond >= 30) && (cond <= 39) ) steel = 12;
	else if( (cond >= 21) && (cond <= 29) ) steel = 14;
	else if( (cond >= 11) && (cond <= 20) ) steel = 16;
	else steel = 10;

	if(ch->steel < steel)
	{
		send_to_char("You do not have enough steel to repair this object.\n\r",ch);
		return;
	}

/* check for fast working */
	wait = fast_working(ch, gsn_repair_shield);

/* Ok! Now, we got everything! Let's get the show on the road. */

  for ( d = descriptor_list; d != NULL; d = d->next )
  {
    if ( d->connected == CON_PLAYING
    &&   d->character != ch
    &&   d->character->in_room != NULL
    &&   d->character->in_room != ch->in_room
    &&   d->character->in_room->area == ch->in_room->area
    &&   !IS_SET(d->character->in_room->room_flags, ROOM_QUIET)
    &&   !IS_AFFECTED2(d->character, AFF_DEAFNESS)
    &&   number_percent() < 5 )
    {
      act("The sounds of hammering can be heard in the distance.",ch,argument,d->character,TO_VICT);
    }
  }

	act("$n begins to make repairs to $p.",ch,obj,NULL,TO_ROOM);
	WAIT_STATE(ch, wait);
	if ( number_percent() > get_skill(ch,gsn_repair_shield))
	{
		act("You fail to repair $p.",ch,obj,NULL,TO_CHAR);
		ch->steel -= steel;
		extract_obj(raw_mat);
		act("$n fails to repair $p.",ch,obj,NULL,TO_ROOM);

			if( number_percent() <= 10 )
			{
				send_to_char("Oops! you broke it, instead of fixing it! It's gone now!!\n\r",ch);
				act("$n accidentally breaks $p!",ch,obj,NULL,TO_ROOM);
				extract_obj(obj);
			}
		check_improve(ch,gsn_repair_shield,FALSE,1);
		return;
	}
	else
	{
		act("You successfully repair $p.",ch,obj,NULL,TO_CHAR);
		extract_obj(raw_mat);
		ch->steel -= steel;
		obj->condition = 100;
		check_improve(ch,gsn_repair_shield,TRUE,1);
		if( bs_exp(ch, gsn_repair_shield) ) gain_exp(ch, 50);
		act("$n successfully makes repairs to $p.",ch,obj,NULL,TO_ROOM);
		return;
	}
	return;
}

void do_leather_repair( CHAR_DATA *ch, char *argument )
{
	DESCRIPTOR_DATA *d;
	char arg[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	OBJ_DATA *obj, *raw_mat, *hold;
      OBJ_DATA *obj2;
      OBJ_DATA *anvil = NULL;
	int cond, steel, wait;

	argument = one_argument(argument, arg);
	one_argument(argument, arg2);

  	if(IS_NPC(ch))
  	{
  		send_to_char("Mobs cannot repair items!\n\r",ch);
  		return;
  	}

	if ( get_skill(ch, gsn_leather_repair) < 1 )
	{
		send_to_char("You don't know how to do that.\n\r", ch );
		return;
	}

	if ( arg[0] == '\0' )
	{
		send_to_char("Syntax: leather <object> <raw material>\n\r",ch);
		send_to_char("Example: leather vest leather\n\r",ch);
		return;
	}

	hold = get_eq_char(ch, WEAR_WIELD);
	if( (hold == NULL) || (hold->item_type != ITEM_BHAMMER) )
	{
		send_to_char("You must be wielding your blacksmithing hammer to make repairs.\n\r",ch);
		return;
	}

      for (obj2 = ch->in_room->contents; obj2 != NULL; obj2 = obj2->next_content)
      {
        if( obj2->item_type == ITEM_ANVIL )
		  anvil = obj2;
      }

      if (anvil == NULL)
      {
        send_to_char("You require the need for an Anvil to repair items.\n\r",ch);
        send_to_char("Make sure there's one here in the room with you and it's on the ground.\n\r",ch);
        return;
      }

	if( (obj = get_obj_carry(ch, arg, ch)) == NULL )
	{
		send_to_char("You do not have that to repair.\n\r",ch);
		return;
	}

	if( obj->item_type != ITEM_ARMOR || str_infix("leather", obj->material) || !is_leather(obj) )
	{
		send_to_char("You can only repair armor made of leather.\n\r",ch);
		return;
	}

	if( arg2[0] == '\0' )
	{
		act("Repair $p with what?",ch,obj,NULL,TO_CHAR);
		return;
	}

	if( (raw_mat = get_obj_carry(ch, arg2, ch)) == NULL )
	{
		send_to_char("You do not have those raw materials.\n\r",ch);
		return;
	}

	if( raw_mat->item_type != ITEM_RAW_MATERIAL)
	{
		act("You cannot use $p for raw materials.",ch,obj,NULL,TO_CHAR);
		return;
	}

	if( str_cmp(obj->material, raw_mat->material) )
	{
		sprintf( buf, "You cannot use %s to repair %s,\n\ras they are made of different materials.\n\r",raw_mat->short_descr, obj->short_descr);
		send_to_char(buf, ch);
		return;
	}
/* steel table, where worse condition equals more money */
	cond = obj->condition;

	if(cond == 100)
	{
		act("$p isn't in any need of repairing.",ch,obj,NULL,TO_CHAR);
		return;
	}

	if(cond >= 90) steel = 1;
	else if( (cond >= 80) && (cond <= 89) ) steel = 2;
	else if( (cond >= 70) && (cond <= 79) ) steel = 4;
	else if( (cond >= 60) && (cond <= 69) ) steel = 6;
	else if( (cond >= 50) && (cond <= 59) ) steel = 8;
	else if( (cond >= 40) && (cond <= 49) ) steel = 10;
	else if( (cond >= 30) && (cond <= 39) ) steel = 12;
	else if( (cond >= 21) && (cond <= 29) ) steel = 14;
	else if( (cond >= 11) && (cond <= 20) ) steel = 16;
	else steel = 10;

	if(ch->steel < steel)
	{
		send_to_char("You do not have enough steel to repair this object.\n\r",ch);
		return;
	}

/* check for fast working */
	wait = fast_working(ch, gsn_leather_repair);

/* Ok! Now, we got everything! Let's get the show on the road. */

  for ( d = descriptor_list; d != NULL; d = d->next )
  {
    if ( d->connected == CON_PLAYING
    &&   d->character != ch
    &&   d->character->in_room != NULL
    &&   d->character->in_room != ch->in_room
    &&   d->character->in_room->area == ch->in_room->area
    &&   !IS_SET(d->character->in_room->room_flags, ROOM_QUIET)
    &&   !IS_AFFECTED2(d->character, AFF_DEAFNESS) 
    &&   number_percent() < 5 )
    {
      act("The sounds of hammering can be heard in the distance.",ch,argument,d->character,TO_VICT);
    }
  }

	act("$n begins to make repairs to $p.",ch,obj,NULL,TO_ROOM);
	WAIT_STATE(ch, wait);
	if ( number_percent() > get_skill(ch,gsn_leather_repair))
	{
		act("You fail to repair $p.",ch,obj,NULL,TO_CHAR);
		ch->steel -= steel;
		extract_obj(raw_mat);
		act("$n fails to repair $p.",ch,obj,NULL,TO_ROOM);

			if( number_percent() <= 10 )
			{
				send_to_char("Oops! you broke it, instead of fixing it! It's gone now!!\n\r",ch);
				act("$n accidentally breaks $p!",ch,obj,NULL,TO_ROOM);
				extract_obj(obj);
			}
		check_improve(ch,gsn_leather_repair,FALSE,1);
		return;
	}
	else
	{
		act("You successfully repair $p.",ch,obj,NULL,TO_CHAR);
		extract_obj(raw_mat);
		ch->steel -= steel;
		obj->condition = 100;
		check_improve(ch,gsn_leather_repair,TRUE,1);
		if( bs_exp(ch, gsn_leather_repair) ) gain_exp(ch, 50);
		act("$n successfully makes repairs to $p.",ch,obj,NULL,TO_ROOM);
		return;
	}
	return;
}

void do_plate_repair( CHAR_DATA *ch, char *argument )
{
	DESCRIPTOR_DATA *d;
	char arg[MAX_INPUT_LENGTH]; 
	char arg2[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	OBJ_DATA *obj, *raw_mat, *hold;
      OBJ_DATA *obj2;
      OBJ_DATA *anvil = NULL;
	int cond, steel, wait;

	argument = one_argument(argument, arg);
	one_argument(argument, arg2);

  	if(IS_NPC(ch))
  	{
  		send_to_char("Mobs cannot repair items!\n\r",ch);
  		return;
  	}

	if ( get_skill(ch, gsn_plate_repair) < 1 )
	{
		send_to_char("You don't know how to do that.\n\r", ch );
		return;
	}

	if ( arg[0] == '\0' )
	{
		send_to_char("Syntax: plate <object> <raw material>\n\r",ch);
		send_to_char("Example: plate platemail steel\n\r",ch);
		return;
	}

	hold = get_eq_char(ch, WEAR_WIELD);
	if( (hold == NULL) || (hold->item_type != ITEM_BHAMMER) )
	{
		send_to_char("You must be wielding your blacksmithing hammer to make repairs.\n\r",ch);
		return;
	}

      for (obj2 = ch->in_room->contents; obj2 != NULL; obj2 = obj2->next_content)
      {
        if( obj2->item_type == ITEM_ANVIL )
		  anvil = obj2;
      }

      if (anvil == NULL)
      {
        send_to_char("You require the need for an Anvil to repair items.\n\r",ch);
        send_to_char("Make sure there's one here in the room with you and it's on the ground.\n\r",ch);
        return;
      }

	if( (obj = get_obj_carry(ch, arg, ch)) == NULL )
	{
		send_to_char("You do not have that to repair.\n\r",ch);
		return;
	}

	if( obj->item_type != ITEM_ARMOR || !is_plate(obj) )
	{
		send_to_char("You can only repair plate type armor.\n\r",ch);
		return;
	}

	if( arg2[0] == '\0' )
	{
		act("Repair $p with what?",ch,obj,NULL,TO_CHAR);
		return;
	}

	if( (raw_mat = get_obj_carry(ch, arg2, ch)) == NULL )
	{
		send_to_char("You do not have those raw materials.\n\r",ch);
		return;
	}

	if( raw_mat->item_type != ITEM_RAW_MATERIAL)
	{
		act("You cannot use $p for raw materials.",ch,raw_mat,NULL,TO_CHAR);
		return;
	}

	if( str_cmp(obj->material, raw_mat->material) )
	{
		sprintf( buf, "You cannot use %s to repair %s,\n\ras they are made of different materials.\n\r",raw_mat->short_descr, obj->short_descr);
		send_to_char(buf, ch);
		return;
	}

	cond = obj->condition;

	if(cond == 100)
	{
		act("$p isn't in any need of repairing.",ch,obj,NULL,TO_CHAR);
		return;
	}

	if(cond >= 90) steel = 1;
	else if( (cond >= 80) && (cond <= 89) ) steel = 2;
	else if( (cond >= 70) && (cond <= 79) ) steel = 4;
	else if( (cond >= 60) && (cond <= 69) ) steel = 6;
	else if( (cond >= 50) && (cond <= 59) ) steel = 8;
	else if( (cond >= 40) && (cond <= 49) ) steel = 10;
	else if( (cond >= 30) && (cond <= 39) ) steel = 12;
	else if( (cond >= 21) && (cond <= 29) ) steel = 14;
	else if( (cond >= 11) && (cond <= 20) ) steel = 16;
	else steel = 10;

	if(ch->steel < steel)
	{
		send_to_char("You do not have enough steel to repair this object.\n\r",ch);
		return;
	}

/* check for fast working */
	wait = fast_working(ch, gsn_plate_repair);

/* Ok! Now, we got everything! Let's get the show on the road. */

  for ( d = descriptor_list; d != NULL; d = d->next )
  {
    if ( d->connected == CON_PLAYING
    &&   d->character != ch
    &&   d->character->in_room != NULL
    &&   d->character->in_room != ch->in_room
    &&   d->character->in_room->area == ch->in_room->area
    &&   !IS_SET(d->character->in_room->room_flags, ROOM_QUIET)
    &&   !IS_AFFECTED2(d->character, AFF_DEAFNESS)
    &&   number_percent() < 5 )
    {
      act("The sounds of hammering can be heard in the distance.",ch,argument,d->character,TO_VICT);
    }
  }

	act("$n begins to make repairs to $p.",ch,obj,NULL,TO_ROOM);
	WAIT_STATE(ch, wait);
	if ( number_percent() > get_skill(ch,gsn_plate_repair))
	{
		act("You fail to repair $p.",ch,obj,NULL,TO_CHAR);
		ch->steel -= steel;
		extract_obj(raw_mat);
		act("$n fails to repair $p.",ch,obj,NULL,TO_ROOM);

			if( number_percent() <= 10 )
			{
				send_to_char("Oops! you broke it, instead of fixing it! It's gone now!!\n\r",ch);
				act("$n accidentally breaks $p!",ch,obj,NULL,TO_ROOM);
				extract_obj(obj);
			}
		check_improve(ch,gsn_plate_repair,FALSE,1);
		return;
	}
	else
	{
		act("You successfully repair $p.",ch,obj,NULL,TO_CHAR);
		extract_obj(raw_mat);
		ch->steel -= steel;
		obj->condition = 100;
		check_improve(ch,gsn_plate_repair,TRUE,1);
		if( bs_exp(ch, gsn_plate_repair) ) gain_exp(ch, 50);
		act("$n successfully makes repairs to $p.",ch,obj,NULL,TO_ROOM);
		return;
	}
	return;
}

void do_chain_repair( CHAR_DATA *ch, char *argument )
{
	DESCRIPTOR_DATA *d;
	char arg[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	OBJ_DATA *obj, *raw_mat, *hold;
      OBJ_DATA *obj2;
      OBJ_DATA *anvil = NULL;
	int cond, steel, wait;

	argument = one_argument(argument, arg);
	one_argument(argument, arg2);

  	if(IS_NPC(ch))
  	{
  		send_to_char("Mobs cannot repair items!\n\r",ch);
  		return;
  	}

	if ( get_skill(ch, gsn_chain_repair) < 1 )
	{
		send_to_char("You don't know how to do that.\n\r", ch );
		return;
	}

	if ( arg[0] == '\0' )
	{
		send_to_char("Syntax: chain <object> <raw material>\n\r",ch);
		send_to_char("Example: chain chainmail steel\n\r",ch);
		return;
	}

	hold = get_eq_char(ch, WEAR_WIELD);
	if( (hold == NULL) || (hold->item_type != ITEM_BHAMMER) )
	{
		send_to_char("You must be wielding your blacksmithing hammer to make repairs.\n\r",ch);
		return;
	}

      for (obj2 = ch->in_room->contents; obj2 != NULL; obj2 = obj2->next_content)
      {
        if( obj2->item_type == ITEM_ANVIL )
		  anvil = obj2;
      }

      if (anvil == NULL)
      {
        send_to_char("You require the need for an Anvil to repair items.\n\r",ch);
        send_to_char("Make sure there's one here in the room with you and it's on the ground.\n\r",ch);
        return;
      }

	if( (obj = get_obj_carry(ch, arg, ch)) == NULL )
	{
		send_to_char("You do not have that to repair.\n\r",ch);
		return;
	}
/* check to see for ARMOR and chain types only. */
	if( obj->item_type != ITEM_ARMOR || !is_chain(obj) )
	{
		send_to_char("You can only repair chain type armor.\n\r",ch);
		return;
	}

	if( arg2[0] == '\0' )
	{
		act("Repair $p with what?",ch,obj,NULL,TO_CHAR);
		return;
	}

	if( (raw_mat = get_obj_carry(ch, arg2, ch)) == NULL )
	{
		send_to_char("You do not have those raw materials.\n\r",ch);
		return;
	}

	if( raw_mat->item_type != ITEM_RAW_MATERIAL)
	{
		act("You cannot use $p for raw materials.",ch,raw_mat,NULL,TO_CHAR);
		return;
	}

	if( str_cmp(obj->material, raw_mat->material) )
	{
		sprintf( buf, "You cannot use %s to repair %s,\n\ras they are made of different materials.\n\r",raw_mat->short_descr, obj->short_descr);
		send_to_char(buf, ch);
		return;
	}

	cond = obj->condition;

	if(cond == 100)
	{
		act("$p isn't in any need of repairing.",ch,obj,NULL,TO_CHAR);
		return;
	}

	if(cond >= 90) steel = 1;
	else if( (cond >= 80) && (cond <= 89) ) steel = 2;
	else if( (cond >= 70) && (cond <= 79) ) steel = 4;
	else if( (cond >= 60) && (cond <= 69) ) steel = 6;
	else if( (cond >= 50) && (cond <= 59) ) steel = 8;
	else if( (cond >= 40) && (cond <= 49) ) steel = 10;
	else if( (cond >= 30) && (cond <= 39) ) steel = 12;
	else if( (cond >= 21) && (cond <= 29) ) steel = 14;
	else if( (cond >= 11) && (cond <= 20) ) steel = 16;
	else steel = 10;

	if(ch->steel < steel)
	{
		send_to_char("You do not have enough steel to repair this object.\n\r",ch);
		return;
	}

/* check for fast working */
	wait = fast_working(ch, gsn_chain_repair );

/* Ok! Now, we got everything! Let's get the show on the road. */

  for ( d = descriptor_list; d != NULL; d = d->next )
  {
    if ( d->connected == CON_PLAYING
    &&   d->character != ch
    &&   d->character->in_room != NULL
    &&   d->character->in_room != ch->in_room
    &&   d->character->in_room->area == ch->in_room->area
    &&   !IS_SET(d->character->in_room->room_flags, ROOM_QUIET)
    &&   !IS_AFFECTED2(d->character, AFF_DEAFNESS)
    &&   number_percent() < 5 )
    {
      act("The sounds of hammering can be heard in the distance.",ch,argument,d->character,TO_VICT);
    }
  }

	act("$n begins to make repairs to $p.",ch,obj,NULL,TO_ROOM);
	WAIT_STATE(ch, wait);
	if ( number_percent() > get_skill(ch,gsn_chain_repair))
	{
		act("You fail to repair $p.",ch,obj,NULL,TO_CHAR);
		ch->steel -= steel;
		extract_obj(raw_mat);
		act("$n fails to repair $p.",ch,obj,NULL,TO_ROOM);

			if( number_percent() <= 10 )
			{
				send_to_char("Oops! you broke it, instead of fixing it! It's gone now!!\n\r",ch);
				act("$n accidentally breaks $p!",ch,obj,NULL,TO_ROOM);
				extract_obj(obj);
			}
		check_improve(ch,gsn_chain_repair,FALSE,1);
		return;
	}
	else
	{
		act("You successfully repair $p.",ch,obj,NULL,TO_CHAR);
		extract_obj(raw_mat);
		ch->steel -= steel;
		obj->condition = 100;
		check_improve(ch,gsn_chain_repair,TRUE,1);
		if( bs_exp(ch, gsn_chain_repair) ) gain_exp(ch, 50);
		act("$n successfully makes repairs to $p.",ch,obj,NULL,TO_ROOM);
		return;
	}
	return;
}

void do_weapon_repair( CHAR_DATA *ch, char *argument )
{
	DESCRIPTOR_DATA *d;
	char arg[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	OBJ_DATA *obj, *raw_mat, *hold;
      OBJ_DATA *obj2;
      OBJ_DATA *anvil = NULL;
	int cond, steel, wait;

	argument = one_argument(argument, arg);
	one_argument(argument, arg2);

  	if(IS_NPC(ch))
  	{
  		send_to_char("Mobs cannot repair items!\n\r",ch);
  		return;
  	}

	if ( get_skill(ch, gsn_weapon_repair) < 1 )
	{
		send_to_char("You don't know how to do that.\n\r", ch );
		return;
	}

	if ( arg[0] == '\0' )
	{
		send_to_char("Syntax: weapon <object> <raw material>\n\r",ch);
		send_to_char("Example: weapon axe steel\n\r",ch);
		return;
	}

	hold = get_eq_char(ch, WEAR_WIELD);
	if( (hold == NULL) || (hold->item_type != ITEM_BHAMMER) )
	{
		send_to_char("You must be wielding your blacksmithing hammer to make repairs.\n\r",ch);
		return;
	}

      for (obj2 = ch->in_room->contents; obj2 != NULL; obj2 = obj2->next_content)
      {
        if( obj2->item_type == ITEM_ANVIL )
		  anvil = obj2;
      }

      if (anvil == NULL)
      {
        send_to_char("You require the need for an Anvil to repair items.\n\r",ch);
        send_to_char("Make sure there's one here in the room with you and it's on the ground.\n\r",ch);
        return;
      }

	if( (obj = get_obj_carry(ch, arg, ch)) == NULL )
	{
		send_to_char("You do not have that weapon to repair.\n\r",ch);
		return;
	}

	if( obj->item_type != ITEM_WEAPON )
	{
		send_to_char("You can only repair weapons.\n\r",ch);
		return;
	}

	if( arg2[0] == '\0' )
	{
		act("Repair $p with what?",ch,obj,NULL,TO_CHAR);
		return;
	}

	if( (raw_mat = get_obj_carry(ch, arg2, ch)) == NULL )
	{
		send_to_char("You do not have those raw materials.\n\r",ch);
		return;
	}

	if( raw_mat->item_type != ITEM_RAW_MATERIAL)
	{
		act("You cannot use $p for raw materials.",ch,raw_mat,NULL,TO_CHAR);
		return;
	}

	if( str_cmp(obj->material, raw_mat->material) )
	{
		sprintf( buf, "You cannot use %s to repair %s,\n\ras they are made of different materials.\n\r",raw_mat->short_descr, obj->short_descr);
		send_to_char(buf, ch);
		return;
	}

	cond = obj->condition;

	if(cond == 100)
	{
		act("$p isn't in any need of repairing.",ch,obj,NULL,TO_CHAR);
		return;
	}

	if(cond >= 90) steel = 1;
	else if( (cond >= 80) && (cond <= 89) ) steel = 2;
	else if( (cond >= 70) && (cond <= 79) ) steel = 4;
	else if( (cond >= 60) && (cond <= 69) ) steel = 6;
	else if( (cond >= 50) && (cond <= 59) ) steel = 8;
	else if( (cond >= 40) && (cond <= 49) ) steel = 10;
	else if( (cond >= 30) && (cond <= 39) ) steel = 12;
	else if( (cond >= 21) && (cond <= 29) ) steel = 14;
	else if( (cond >= 11) && (cond <= 20) ) steel = 16;
	else steel = 10;

	if(ch->steel < steel)
	{
		send_to_char("You do not have enough steel to repair this object.\n\r",ch);
		return;
	}

/* check for fast working */
	wait = fast_working(ch, gsn_weapon_repair);

/* Ok! Now, we got everything! Let's get the show on the road. */

  for ( d = descriptor_list; d != NULL; d = d->next )
  {
    if ( d->connected == CON_PLAYING
    &&   d->character != ch
    &&   d->character->in_room != NULL
    &&   d->character->in_room != ch->in_room
    &&   d->character->in_room->area == ch->in_room->area
    &&   !IS_SET(d->character->in_room->room_flags, ROOM_QUIET)
    &&   !IS_AFFECTED2(d->character, AFF_DEAFNESS)
    &&   number_percent() < 5 )
    {
      act("The sounds of hammering can be heard in the distance.",ch,argument,d->character,TO_VICT);
    }
  }

	act("$n begins to make repairs to $p.",ch,obj,NULL,TO_ROOM);
	WAIT_STATE(ch, wait);
	if ( number_percent() > get_skill(ch,gsn_weapon_repair))
	{
		act("You fail to repair $p.",ch,obj,NULL,TO_CHAR);
		ch->steel -= steel;
		extract_obj(raw_mat);
		act("$n fails to repair $p.",ch,obj,NULL,TO_ROOM);

			if( number_percent() <= 10 )
			{
				send_to_char("Oops! you broke it, instead of fixing it! It's gone now!!\n\r",ch);
				act("$n accidentally breaks $p!",ch,obj,NULL,TO_ROOM);
				extract_obj(obj);
			}
		check_improve(ch,gsn_weapon_repair,FALSE,1);
		return;
	}
	else
	{
		act("You successfully repair $p.",ch,obj,NULL,TO_CHAR);
		extract_obj(raw_mat);
		ch->steel -= steel;
		obj->condition = 100;
		check_improve(ch,gsn_weapon_repair,TRUE,1);
		if( bs_exp(ch, gsn_weapon_repair) ) gain_exp(ch, 50);
		act("$n successfully makes repairs to $p.",ch,obj,NULL,TO_ROOM);
		return;
	}
	return;
}

void do_deathblade( CHAR_DATA *ch, char *argument )
{
	DESCRIPTOR_DATA *d;
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj, *hold;
	AFFECT_DATA af;
	bool blade = FALSE;
      OBJ_DATA *obj2;
      OBJ_DATA *anvil = NULL;
	int wait;

	one_argument(argument, arg);

  	if( IS_NPC(ch) )
  	{
  		send_to_char("Mobs cannot forge a deathblade on a weapon!\n\r",ch);
  		return;
  	}

	if ( get_skill(ch, gsn_deathblade) < 1 )
	{
		send_to_char("You don't know how to do that.\n\r", ch );
		return;
	}

	if( arg[0] == '\0' )
	{
		send_to_char("Forge a deathblade on what?\n\r",ch);
		return;
	}

	hold = get_eq_char(ch, WEAR_WIELD);
	if( (hold == NULL) || (hold->item_type != ITEM_BHAMMER) )
	{
		send_to_char("You must be wielding your blacksmithing hammer to forge a deathblade.\n\r",ch);
		return;
	}

      for (obj2 = ch->in_room->contents; obj2 != NULL; obj2 = obj2->next_content)
      {
        if( obj2->item_type == ITEM_ANVIL )
		  anvil = obj2;
      }

      if (anvil == NULL)
      {
        send_to_char("You require the need for an Anvil to imbue items.\n\r",ch);
        send_to_char("Make sure there's one here in the room with you and it's on the ground.\n\r",ch);
        return;
      }

	if( (obj = get_obj_carry(ch, arg, ch)) == NULL )
	{
		send_to_char("You do not have that weapon to forge a deathblade upon.\n\r",ch);
		return;
	}

	if( obj->item_type != ITEM_WEAPON )
	{
		send_to_char("You cannot forge a deathblade onto something which is not a weapon.",ch);
		return;
	}

	if( obj->value[0] == WEAPON_SWORD
	 || obj->value[0] == WEAPON_DAGGER
	 || obj->value[0] == WEAPON_AXE )
		blade = TRUE;

	if(!blade)
	{
		send_to_char("There is no blade on that weapon, so a deathblade cannot be forged onto it.\n\r",ch);
		return;
	}

	if( IS_OBJ_STAT(obj,ITEM_DEATHBLADE) )
	{
		act("$p already has a deathblade forged upon it.",ch,obj,NULL,TO_CHAR);
		return;
	}

	if( obj->timer > 0 )
	{
		act("$p isn't in any condition to have a deathblade forged upon it.",ch,obj,NULL,TO_CHAR);
		return;
	}
	
	if( ch->steel < ((250 + ch->level)/2))
	{
		send_to_char("You do not have enough steel to forge a deathblade.\n\r",ch);
		return;
	}

/* check for fast working */
	wait = fast_working(ch, gsn_deathblade);

  for ( d = descriptor_list; d != NULL; d = d->next )
  {
    if ( d->connected == CON_PLAYING
    &&   d->character != ch
    &&   d->character->in_room != NULL
    &&   d->character->in_room != ch->in_room
    &&   d->character->in_room->area == ch->in_room->area
    &&   !IS_SET(d->character->in_room->room_flags, ROOM_QUIET)
    &&   !IS_AFFECTED2(d->character, AFF_DEAFNESS)
    &&   number_percent() < 5 )
    {
      act("The sounds of hammering can be heard in the distance.",ch,argument,d->character,TO_VICT);
    }
  }

	act("$n begins to forge a deathblade upon $p.",ch,obj,NULL,TO_ROOM);
	WAIT_STATE(ch, wait);
	if ( number_percent() > get_skill(ch,gsn_deathblade))
	{
		act("You fail to forge a deathblade upon $p.",ch,obj,NULL,TO_CHAR);
		ch->steel -= ch->level;
		act("$n fails to forge a deathblade upon $p.",ch,obj,NULL,TO_ROOM);
		check_improve(ch,gsn_deathblade,FALSE,1);
	}
	else
	{
		af.where		= TO_OBJECT;
		af.type		= gsn_deathblade;
		af.level		= ch->level;
		af.duration 	= (number_range(150,250) + ch->level);
		af.location 	= 0;
		af.modifier 	= 0;
		af.bitvector 	= ITEM_DEATHBLADE;
		affect_to_obj(obj,&af);

		act("You successfully forge a deathblade upon $p.",ch,obj,NULL,TO_CHAR);
		ch->steel -= ((250 + ch->level)/2);
		check_improve(ch,gsn_deathblade,TRUE,1);
		if( bs_exp(ch, gsn_deathblade) ) gain_exp(ch, 50);
		act("$n successfully forges a deathblade upon $p.",ch,obj,NULL,TO_ROOM);
		return;
	}
	return;
}

void do_flameblade(  CHAR_DATA *ch, char *argument )
{
	DESCRIPTOR_DATA *d;
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj, *hold;
	AFFECT_DATA af;
	bool flame = FALSE;
      OBJ_DATA *obj2;
      OBJ_DATA *anvil = NULL;
	int wait;

	one_argument(argument, arg);

  	if( IS_NPC(ch) )
  	{
  		send_to_char("Mobs cannot imbue flame into a weapon!\n\r",ch);
  		return;
  	}

	if ( get_skill(ch, gsn_flameblade) < 1 )
	{
		send_to_char("You don't know how to do that.\n\r", ch );
		return;
	}

	if( arg[0] == '\0' )
	{
		send_to_char("Imbue flame onto what weapon?\n\r",ch);
		return;
	}

	hold = get_eq_char(ch, WEAR_WIELD);
	if( (hold == NULL) || (hold->item_type != ITEM_BHAMMER) )
	{
		send_to_char("You must be wielding your blacksmithing hammer to imbue flames.\n\r",ch);
		return;
	}

      for (obj2 = ch->in_room->contents; obj2 != NULL; obj2 = obj2->next_content)
      {
        if( obj2->item_type == ITEM_ANVIL )
		  anvil = obj2;
      }

      if (anvil == NULL)
      {
        send_to_char("You require the need for an Anvil to imbue items.\n\r",ch);
        send_to_char("Make sure there's one here in the room with you and it's on the ground.\n\r",ch);
        return;
      }

	if( (obj = get_obj_carry(ch, arg, ch)) == NULL )
	{
		send_to_char("You do not have that weapon to imbue flame upon.\n\r",ch);
		return;
	}

	if( obj->item_type != ITEM_WEAPON )
	{
		send_to_char("You cannot imbue flames onto something which is not a weapon.\n\r",ch);
		return;
	}

	if( !str_cmp(obj->material, "stone")
	 || !str_cmp(obj->material, "crystal")
	 || !str_cmp(obj->material, "marble")
	 || !str_cmp(obj->material, "obsidian")
	 || !str_cmp(obj->material, "iron")
	 || !str_cmp(obj->material, "silver")
	 || !str_cmp(obj->material, "mithril")
	 || !str_cmp(obj->material, "diamond")
	 || !str_cmp(obj->material, "platinum")
	 || !str_cmp(obj->material, "titanium")
	 || !str_cmp(obj->material, "ceramic")
	 || !str_cmp(obj->material, "granite")
	 || !str_cmp(obj->material, "velium")
	 || !str_cmp(obj->material, "slate")
	 || !str_cmp(obj->material, "adamantite")
	 || !str_cmp(obj->material, "steel") )
		flame = TRUE;

	if(!flame)
	{
		act("You cannot imbue flame onto $p, as it would burn right up!",ch,obj,NULL,TO_CHAR);
		return;
	}

	if( IS_WEAPON_STAT(obj,WEAPON_FLAMING) )
	{
		send_to_char("That weapon is already flaming!\n\r",ch);
		return;
	}

	if( IS_WEAPON_STAT(obj, WEAPON_FROST)
	 || IS_WEAPON_STAT(obj, WEAPON_ICY_BURST) )
	{
		send_to_char("That weapon seems too cold to imbue flames into it.\n\r",ch);
		return;
	}

	if( IS_OBJ_STAT(obj, ITEM_BURN_PROOF) )
	{
		send_to_char("Try as you might, the flame will not ignite on this weapon.\n\r",ch);
		return;
	}

	if( obj->timer > 0 )
	{
		act("$p isn't in any condition to be imbued with flames.",ch,obj,NULL,TO_CHAR);
		return;
	}

	if(ch->steel < ((300+(ch->level * 2))/2))
	{
		send_to_char("You do not have enough steel to imbue flames.\n\r",ch);
		return;
	}

/* check for fast working */
	wait = fast_working(ch, gsn_flameblade);

  for ( d = descriptor_list; d != NULL; d = d->next )
  {
    if ( d->connected == CON_PLAYING
    &&   d->character != ch
    &&   d->character->in_room != NULL
    &&   d->character->in_room != ch->in_room
    &&   d->character->in_room->area == ch->in_room->area
    &&   !IS_SET(d->character->in_room->room_flags, ROOM_QUIET)
    &&   !IS_AFFECTED2(d->character, AFF_DEAFNESS)
    &&   number_percent() < 5 )
    {
      act("The sounds of hammering can be heard in the distance.",ch,argument,d->character,TO_VICT);
    }
  }

	act("$n begins to imbue fire onto $p.",ch,obj,NULL,TO_ROOM);
	WAIT_STATE( ch, wait );
	if ( (number_percent() + 20)  > get_skill(ch,gsn_flameblade) )
	{
		act("You melt $p trying to imbue the hot flames onto it.",ch,obj,NULL,TO_CHAR);
		ch->steel -= 150;
		extract_obj(obj);
		act("$n melts $p trying to imbue it with hot flames.",ch,obj,NULL,TO_ROOM);
		check_improve(ch,gsn_flameblade,FALSE,1);
	}
	else
	{
		af.where		= TO_WEAPON;
		af.type		= gsn_flameblade;
		af.level		= ch->level;
		af.duration 	= (number_range(150,300) + ch->level);
		af.location 	= 0;
		af.modifier 	= 0;
		af.bitvector 	= WEAPON_FLAMING;
		affect_to_obj(obj,&af);

		act("You successfully imbue flames into $p.",ch,obj,NULL,TO_CHAR);
		ch->steel -= ((300+(ch->level * 2))/2);
		check_improve(ch,gsn_flameblade,TRUE,1);
		if( bs_exp(ch, gsn_flameblade) ) gain_exp(ch, 50);
		act("$n successfully imbues flames into $p.",ch,obj,NULL,TO_ROOM);
		return;
	}
	return;
}

void do_runeblade( CHAR_DATA *ch, char *argument )
{
	DESCRIPTOR_DATA *d;
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj, *hold;
	AFFECT_DATA af;
      OBJ_DATA *obj2;
	OBJ_DATA *anvil = NULL;
	int wait;

	one_argument(argument, arg);

  	if( IS_NPC(ch) )
  	{
  		send_to_char("Mobs cannot forge a runeblade onto a weapon!\n\r",ch);
  		return;
  	}

	if ( get_skill(ch, gsn_runeblade) < 1 )
	{
		send_to_char("You don't know how to do that.\n\r", ch );
		return;
	}

	if( arg[0] == '\0' )
	{
		send_to_char("Forge a runeblade onto what weapon?\n\r",ch);
		return;
	}

	hold = get_eq_char(ch, WEAR_WIELD);
	if( (hold == NULL) || (hold->item_type != ITEM_BHAMMER) )
	{
		send_to_char("You must be wielding your blacksmithing hammer to forge a runeblade.\n\r",ch);
		return;
	}

      for (obj2 = ch->in_room->contents; obj2 != NULL; obj2 = obj2->next_content)
      {
        if( obj2->item_type == ITEM_ANVIL )
		  anvil = obj2;
      }

      if (anvil == NULL)
      {
        send_to_char("You require the need for an Anvil to imbue items.\n\r",ch);
        send_to_char("Make sure there's one here in the room with you and it's on the ground.\n\r",ch);
        return;
      }

	if( (obj = get_obj_carry(ch, arg, ch)) == NULL )
	{
		send_to_char("You do not have that weapon on which to forge a runeblade.\n\r",ch);
		return;
	}

	if( obj->item_type != ITEM_WEAPON )
	{
		send_to_char("You cannot forge a runeblade onto something which is not a weapon.\n\r",ch);
		return;
	}

	if( IS_WEAPON_STAT(obj,WEAPON_BRILLIANT) )
	{
		send_to_char("You cannot forge a runeblade on this weapon.\n\r",ch);
		return;
	}

	if( obj->timer > 0 )
	{
		act("$p isn't in any condition to have a runeblade forged upon it.",ch,obj,NULL,TO_CHAR);
		return;
	}

	if(ch->steel < ((250 + ch->level)/2))
	{
		send_to_char("You do not have enough steel to forge a runeblade.\n\r",ch);
		return;
	}

/* check for fast working */
	wait = fast_working(ch, gsn_runeblade);

  for ( d = descriptor_list; d != NULL; d = d->next )
  {
    if ( d->connected == CON_PLAYING
    &&   d->character != ch
    &&   d->character->in_room != NULL
    &&   d->character->in_room != ch->in_room
    &&   d->character->in_room->area == ch->in_room->area
    &&   !IS_SET(d->character->in_room->room_flags, ROOM_QUIET)
    &&   !IS_AFFECTED2(d->character, AFF_DEAFNESS)
    &&   number_percent() < 5 )
    {
      act("The sounds of hammering can be heard in the distance.",ch,argument,d->character,TO_VICT);
    }
  }

	act("$n begins to forge runes upon $p.",ch,obj,NULL,TO_ROOM);
	WAIT_STATE(ch, wait);
	if ( number_percent() > get_skill(ch,gsn_runeblade) )
	{
		act("You fail to forge a runeblade onto $p.",ch,obj,NULL,TO_CHAR);
		ch->steel -= ch->level;
		act("$n fails to forge a runeblade onto $p.",ch,obj,NULL,TO_ROOM);
		check_improve(ch,gsn_runeblade,FALSE,1);
	}
	else
	{
		af.where		= TO_WEAPON;
		af.type		= gsn_runeblade;
		af.level		= ch->level;
		af.duration 	= (number_range(150,250) + ch->level);
		af.location 	= 0;
		af.modifier		= 0;
		af.bitvector 	= WEAPON_BRILLIANT;
		affect_to_obj(obj,&af);

		act("You successfully forge a runeblade onto $p.",ch,obj,NULL,TO_CHAR);
		ch->steel -= ((250 + ch->level)/2);
		check_improve(ch,gsn_runeblade,TRUE,1);
		if( bs_exp(ch, gsn_runeblade) ) gain_exp(ch, 50);
		act("$n successfully forges a runeblade onto $p.",ch,obj,NULL,TO_ROOM);
		return;
	}
	return;
}

void do_stainless( CHAR_DATA *ch, char *argument )
{
	DESCRIPTOR_DATA *d;
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj, *hold;
	AFFECT_DATA af;
      OBJ_DATA *obj2;
	OBJ_DATA *anvil = NULL;
	int wait;

	one_argument(argument, arg);

	if( IS_NPC(ch) )
	{
		send_to_char("Mobs cannot use this skill.\n\r",ch);
		return;
	}

	if( get_skill(ch, gsn_stainless) < 1 )
	{
		send_to_char("You don't know how to do that.\n\r",ch);
		return;
	}

	if( arg[0] == '\0' )
	{
		send_to_char("Make what weapon stainless?\n\r",ch);
		return;
	}

	hold = get_eq_char(ch, WEAR_WIELD);
	if( (hold == NULL) || (hold->item_type != ITEM_BHAMMER) )
	{
		send_to_char("You must be wielding your blacksmithing hammer to make a weapon stainless.\n\r",ch);
		return;
	}

      for (obj2 = ch->in_room->contents; obj2 != NULL; obj2 = obj2->next_content)
      {
        if( obj2->item_type == ITEM_ANVIL )
		  anvil = obj2;
      }

      if (anvil == NULL)
      {
        send_to_char("You require the need for an Anvil to make items stainless.\n\r",ch);
        send_to_char("Make sure there's one here in the room with you and it's on the ground.\n\r",ch);
        return;
      }

	if( (obj = get_obj_carry(ch, arg, ch)) == NULL )
	{
		send_to_char("You do not have that weapon.\n\r",ch);
		return;
	}

	if( obj->item_type != ITEM_WEAPON && obj->item_type != ITEM_ARMOR )
	{
		send_to_char("You cannot make that object resistant to corrosion.\n\r",ch);
		return;
	}

	if( IS_SET(obj->extra_flags, ITEM_STAINLESS) )
	{
		act("$p is already protected from corrosion.",ch,obj,NULL,TO_CHAR);
		return;
	}

	if( obj->timer > 0 )
	{
		act("$p is too far corroded to become stainless.",ch,obj,NULL,TO_CHAR);
		return;
	}

	if( ch->steel < ch->level )
	{
		act("You do not have enough steel to make $p stainless.",ch,obj,NULL,TO_CHAR);
		return;
	}

/* check for fast working */
	wait = fast_working(ch, gsn_stainless);

  for ( d = descriptor_list; d != NULL; d = d->next )
  {
    if ( d->connected == CON_PLAYING
    &&   d->character != ch
    &&   d->character->in_room != NULL
    &&   d->character->in_room != ch->in_room
    &&   d->character->in_room->area == ch->in_room->area
    &&   !IS_SET(d->character->in_room->room_flags, ROOM_QUIET)
    &&   !IS_AFFECTED2(d->character, AFF_DEAFNESS)
    &&   number_percent() < 5 )
    {
      act("The sounds of hammering can be heard in the distance.",ch,argument,d->character,TO_VICT);
    }
  }

	act("$n begins to forge a protective film upon $p.",ch,obj,NULL,TO_ROOM);
	WAIT_STATE(ch, wait);
	if( number_percent() > get_skill(ch, gsn_stainless) )
	{
		act("You fail to protect $p from corrosion.",ch,obj,NULL,TO_CHAR);
		ch->steel -= ch->level;
		act("$n fails to forge a protective film upon $p.",ch,obj,NULL,TO_ROOM);
		check_improve(ch,gsn_stainless,FALSE,1);
	}
	else
	{
	
		af.where	= TO_OBJECT;
		af.type		= gsn_stainless;
		af.level	= ch->level;
		af.duration = (number_range(100,250) + ch->level);
		af.location = 0;
		af.modifier = 0;
		af.bitvector = ITEM_STAINLESS;
		affect_to_obj(obj,&af);

		act("You successfully protect $p from corrosion.",ch,obj,NULL,TO_CHAR);
		ch->steel -= ch->level;
		check_improve(ch,gsn_stainless,TRUE,1);
		if( bs_exp(ch, gsn_stainless) ) gain_exp(ch, 50);
		act("$n successfully protects $p from corrosion.",ch,obj,NULL,TO_ROOM);
		return;
	}
	return;
}

void do_eversharp( CHAR_DATA *ch, char *argument )
{
	DESCRIPTOR_DATA *d;
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj, *stone, *wield;
	int steel, wait;

	one_argument( argument, arg);

	if( IS_NPC(ch) )
	{
		send_to_char("Mobs cannot make a weapon forever sharp!\n\r",ch);
		return;
	}

	if( get_skill(ch, gsn_eversharp) < 1 )
	{
		send_to_char("You don't know how to do that.\n\r",ch);
		return;
	}

	if (arg[0] == '\0')
	{
		send_to_char("Make what weapon forever sharp?\n\r",ch);
		return;
	}

	wield = get_eq_char(ch, WEAR_WIELD);
	if( (wield == NULL) || (wield->item_type != ITEM_BHAMMER) )
	{
		send_to_char("You must be wielding your blacksmithing hammer to sharpen a weapon with.\n\r",ch);
		return;
	}

	stone = get_eq_char(ch,WEAR_HOLD);
	if ( (stone == NULL) || (stone->item_type != ITEM_WET_STONE) )
	{
		send_to_char("You must be holding a wetstone to sharpen weapons upon.\n\r",ch);
		return;
	}

	if( (obj = get_obj_carry(ch, arg, ch)) == NULL )
	{
		send_to_char("You don't have that item.\n\r",ch);
		return;
	}

	if(obj->item_type != ITEM_WEAPON)
	{
		send_to_char("You may only sharpen weapons.\n\r",ch);
		return;
	}

      if ((obj->value[0] != 1 
      && obj->value[0] != WEAPON_SWORD 
      && obj->value[0] != WEAPON_DAGGER
      && obj->value[0] != WEAPON_SPEAR 
      && obj->value[0] != WEAPON_AXE )) 
      { 
        send_to_char( "You can only sharpen weapons with a bladed edge.\n\r", ch ); 
        return; 
      } 

	if(IS_WEAPON_STAT(obj,WEAPON_SHARP))
	{
		act("$p has already been sharpened.",ch,obj,NULL,TO_CHAR);
		return;
	}

	steel = obj->level*5;
	if(ch->steel < steel)
	{
		send_to_char("You do not have enough steel to sharpen a weapon.\n\r",ch);
		return;
	}

	if(stone != NULL && stone->item_type == ITEM_WET_STONE)
	{
		act("You use a $p to sharpen your weapon.",ch,stone,NULL,TO_CHAR);
		act("It suddenly crubles into dust!",ch,stone,NULL,TO_CHAR);
		extract_obj(stone);
	}

/* check for fast working */
	wait = fast_working(ch, gsn_eversharp);

  for ( d = descriptor_list; d != NULL; d = d->next )
  {
    if ( d->connected == CON_PLAYING
    &&   d->character != ch
    &&   d->character->in_room != NULL
    &&   d->character->in_room != ch->in_room
    &&   d->character->in_room->area == ch->in_room->area
    &&   !IS_SET(d->character->in_room->room_flags, ROOM_QUIET)
    &&   !IS_AFFECTED2(d->character, AFF_DEAFNESS)
    &&   number_percent() < 5 )
    {
      act("The sounds of hammering can be heard in the distance.",ch,argument,d->character,TO_VICT);
    }
  }

	act("$n begins to sharpen $p.",ch,obj,NULL,TO_ROOM);
	WAIT_STATE(ch, wait);
	if ( number_percent() > get_skill(ch, gsn_eversharp) )
	{
		act("You fail to sharpen $p.",ch,obj,NULL,TO_CHAR);
		act("$n fails to sharpen $p.",ch,obj,NULL,TO_ROOM);
		ch->steel -= steel/2;
		check_improve(ch,gsn_eversharp,FALSE,1);
		return;
	}
	else
	{
		act("You successfully make $p forever sharp.",ch,obj,NULL,TO_CHAR);
		SET_BIT(obj->value[4], WEAPON_SHARP); 
		ch->steel -= steel;
		check_improve(ch,gsn_eversharp,TRUE,1);
		if( bs_exp(ch, gsn_eversharp) ) gain_exp(ch, 50);
		act("Using a blacksmith hammer with a stone, $n makes $p forever sharp.",ch,obj,NULL,TO_ROOM);
		return;
	}
  	return;
}

void do_purifying_fire( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj, *fire;
	bool fire_here = FALSE;
	bool bExtraOneFlags = TRUE, bExtraTwoFlags = TRUE;
	int wait;
	one_argument(argument, arg);

	if( IS_NPC(ch) )
	{
		send_to_char("Mobs cannot purify a weapon.\n\r",ch);
		return;
	}

	if( get_skill(ch, gsn_purifying_fire) < 1 )
	{
		send_to_char("You don't know how to do that.\n\r",ch);
		return;
	}

	if(arg[0] == '\0')
	{
		send_to_char("Bathe what in the purifying fire?\n\r",ch);
		return;
	}

	if( (obj = get_obj_carry(ch, arg, ch)) == NULL )
	{
		send_to_char("You don't have that item.\n\r",ch);
		return;
	}

	if( obj->item_type != ITEM_WEAPON && obj->item_type != ITEM_ARMOR )
	{
		send_to_char("You may only purify weapons or armor.\n\r",ch);
		return;
	}
	
	for(fire = ch->in_room->contents; fire != NULL; fire = fire->next_content)
	{
		if( str_cmp(fire->material, "magical fire") )
			continue;

		fire_here = TRUE;
	}

	if(!fire_here) 
	{
		send_to_char("There isn't any magical fire here to purify armor or a weapon with.\n\r",ch);
		return;
	}

	if(obj->extra_flags == 0 
	|| obj->extra_flags != obj->pIndexData->extra_flags)
	{
		bExtraOneFlags = FALSE;
	}

	if(obj->extra2_flags == 0
	|| obj->extra2_flags != obj->pIndexData->extra2_flags)
	{
		bExtraTwoFlags = FALSE;
	}

	/* We cannot remove any affects. */
	if( !bExtraOneFlags && !bExtraTwoFlags )
	{
		act("$p cannot be purified of any magical properties.",ch,obj,NULL,TO_CHAR);
		return;
	}

	if( obj->timer > 0 )
	{
		act("$p isn't in any condition to be subjected to the magical flames.",ch,obj,NULL,TO_CHAR);
		return;
	}

	if(ch->steel < 500)
	{
	  send_to_char("You do not have enough steel to purify armor or a weapon.\n\r",ch);
	  send_to_char("The total cost is 500 steel coins.\n\r",ch);
	  return;
	}

	/* check for fast working */
	wait = fast_working(ch, gsn_purifying_fire);
	
	act("$n bathes $p in a magical blue fire.",ch,obj,NULL,TO_ROOM);
	WAIT_STATE(ch, wait);
	if ( number_percent() > get_skill(ch, gsn_purifying_fire) )
	{
		act("You fail to purify the magical properties of $p.",ch,obj,NULL,TO_CHAR);
		act("$n fails to purify $p of it's magical properties.",ch,obj,NULL,TO_ROOM);
		ch->steel -= 20;
		check_improve(ch,gsn_purifying_fire,FALSE,1);
		return;
	}
	else
	{
		act("You pull $p out of the fire, it's magical properties purified.",ch,obj,NULL,TO_CHAR);

	/* Remove these following extra_flags */
	if(bExtraOneFlags)
	{
		if(IS_SET(obj->extra_flags, ITEM_GLOW))			REMOVE_BIT(obj->extra_flags, ITEM_GLOW);
		if(IS_SET(obj->extra_flags, ITEM_HUM))			REMOVE_BIT(obj->extra_flags, ITEM_HUM);
		if(IS_SET(obj->extra_flags, ITEM_DARK)) 		REMOVE_BIT(obj->extra_flags, ITEM_DARK);
		if(IS_SET(obj->extra_flags, ITEM_EVIL)) 		REMOVE_BIT(obj->extra_flags, ITEM_EVIL);
		if(IS_SET(obj->extra_flags, ITEM_INVIS)) 		REMOVE_BIT(obj->extra_flags, ITEM_INVIS);
		if(IS_SET(obj->extra_flags, ITEM_MAGIC)) 		REMOVE_BIT(obj->extra_flags, ITEM_MAGIC);
		if(IS_SET(obj->extra_flags, ITEM_NODROP))		REMOVE_BIT(obj->extra_flags, ITEM_NODROP);
		if(IS_SET(obj->extra_flags, ITEM_BLESS)) 		REMOVE_BIT(obj->extra_flags, ITEM_BLESS);
		if(IS_SET(obj->extra_flags, ITEM_ANTI_GOOD)) 	REMOVE_BIT(obj->extra_flags, ITEM_ANTI_GOOD);
		if(IS_SET(obj->extra_flags, ITEM_ANTI_EVIL)) 	REMOVE_BIT(obj->extra_flags, ITEM_ANTI_EVIL);
		if(IS_SET(obj->extra_flags, ITEM_ANTI_NEUTRAL)) REMOVE_BIT(obj->extra_flags, ITEM_ANTI_NEUTRAL);
		if(IS_SET(obj->extra_flags, ITEM_NOREMOVE)) 	REMOVE_BIT(obj->extra_flags, ITEM_NOREMOVE);
		if(IS_SET(obj->extra_flags, ITEM_STAINLESS)) 	REMOVE_BIT(obj->extra_flags, ITEM_STAINLESS);
		if(IS_SET(obj->extra_flags, ITEM_BURN_PROOF)) 	REMOVE_BIT(obj->extra_flags, ITEM_BURN_PROOF);
		if(IS_SET(obj->extra_flags, ITEM_NOUNCURSE)) 	REMOVE_BIT(obj->extra_flags, ITEM_NOUNCURSE);
	}

/* Remove these following extra2_flags - Currently there is none we want to remove.
	if(bExtraTwoFlags)
	{
	}
*/

/* Unfortunately, this doesn't seem to work. If you wanna figure it out,
 * go right ahead. -Shaun 

		// remove all affects
		for (paf = obj->affected;  paf != NULL; paf = paf_next)
		{
			paf_next = paf->next;
			if(paf->type == gsn_cultural_artistry) continue;

			paf_next = affect_free;
			affect_free = paf;
		}
		obj->affected = NULL;
*/
		ch->steel -= 500;
		check_improve(ch,gsn_purifying_fire,TRUE,1);
		if( bs_exp(ch, gsn_purifying_fire) ) gain_exp(ch, 50);
		act("$n pulls $p out of the fire, it's magical properties purified.",ch,obj,NULL,TO_ROOM);
		return;
	}
	return;
}

void do_fletching( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA *obj, *inv, *hold;
	OBJ_DATA *feathers = NULL;
	OBJ_DATA *iron_tips = NULL;
	int wait;

	if( IS_NPC(ch) )
	{
		send_to_char("Mobs cannot create a quiver of arrows.\n\r",ch);
		return;
	}

	if( get_skill(ch, gsn_fletching) < 1 )
	{
		send_to_char("You don't know how to do that.\n\r",ch);
		return;
	}

	hold = get_eq_char(ch, WEAR_HOLD);
	if( (hold == NULL) || (hold->item_type != ITEM_TOOL_KIT) )
	{
		send_to_char("You must be holding a fletching toolkit to create arrows with.\n\r",ch);
		return;
	}

	for(inv = ch->carrying; inv != NULL; inv = inv->next_content)
	{
		if( !str_cmp(inv->material, "feathers") )
			feathers = inv;

		if( !str_cmp(inv->material, "iron tips") )
			iron_tips = inv;
	}

	if( (feathers == NULL) || (iron_tips == NULL) )
	{
		send_to_char("You must have feathers and iron tips to create arrows.\n\r",ch);
		return;
	}

	if(ch->steel < 60)
	{
		send_to_char("You do not have enough steel to create a quiver of arrows.\n\r",ch);
		return;
	}

/* check for fast working */
	wait = fast_working(ch, gsn_fletching);

	act("$n begins to carve out a series of arrows.",ch,NULL,NULL,TO_ROOM);
	WAIT_STATE(ch, wait);
	if( number_percent() > get_skill(ch,gsn_fletching) )
	{
		act("You fail to create a quiver of arrows.",ch,NULL,NULL,TO_CHAR);
		act("$n fails to create a quiver of arrows.",ch,NULL,NULL,TO_ROOM);
		ch->steel -= 60;
		extract_obj(hold);
		extract_obj(feathers);
		extract_obj(iron_tips);
		check_improve(ch,gsn_fletching,FALSE,1);
		return;
	}
	else
	{
		act("You successfully create a quiver of arrows.",ch,NULL,NULL,TO_CHAR);
		ch->steel -= 60;
		extract_obj(hold);
		extract_obj(feathers);
		extract_obj(iron_tips);
		obj = create_object( get_obj_index(OBJ_VNUM_FLETCHING_QUIVER), ch->level + 10 );
		obj_to_char( obj, ch );
		check_improve(ch,gsn_fletching,TRUE,1);
		if( bs_exp(ch, gsn_fletching) ) gain_exp(ch, 50);
		act("$n successfully creates a quiver of arrows.",ch,NULL,NULL,TO_ROOM);
		return;
	}
	return;
}

void do_forge( CHAR_DATA *ch, char *argument )
{
	DESCRIPTOR_DATA *d;
	char arg[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_INPUT_LENGTH];
	char arg4[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	OBJ_DATA *obj, *inv, *room_obj, *raw_mat;
	OBJ_DATA *anvil = NULL;
	OBJ_DATA *furnace = NULL;
	OBJ_DATA *barrel = NULL;
	OBJ_DATA *grinder = NULL;
	int steel, item, wait, material_count, show;
      int value;
      int wvone;
      int wvtwo;
	int acp;
	int acb;
	int acs;
	int ace;

	smash_tilde(argument);
	argument = one_argument( argument, arg );
	argument = one_argument( argument, arg2 );
	argument = one_argument( argument, arg3 );
      one_argument(argument, arg4);

	if( IS_NPC(ch) )
	{
	  send_to_char("The Forge skill is not available to MOBs.\n\r",ch);
	  return;
	}

	if( get_skill(ch, gsn_smith) < 1 )
	{
	  send_to_char("You don't know how to do that.\n\r",ch);
	  return;
	}

	if( arg[0] == '\0' || arg2[0] == '\0' || str_cmp(arg2, "into"))
	{
	  send_to_char("Syntax: forge <raw material> into <level # you want it> <object>\n\r",ch);
	  send_to_char("   Example: forge steel into 10 sword\n\r",ch);
	  return;
	}

	raw_mat = get_obj_carry(ch, arg, ch);

	if( raw_mat == NULL )
	{
	  send_to_char("You do not have those raw materials.\n\r",ch);
	  return;
	}

	if( raw_mat->item_type != ITEM_RAW_MATERIAL )
	{
	  act("You cannot use $p for raw materials.",ch,raw_mat,NULL,TO_CHAR);
	  return;
	}

      if ( !is_number( arg3 ) || arg3[0] == '\0')
      {
	  send_to_char( "Oops! Please specify a level you wish to forge the item as!\n\r", ch );
	  sprintf(buf, "The value may range from 10 to %d.\n\r",
        ch->level+10);
	  send_to_char(buf, ch);
	  return;
      }

      value = atoi( arg3 );

      if ( value > (ch->level+10) || value < 10 )
      {
	  sprintf(buf, "Sorry! The values you are authorized to use range from 10 to %d only!\n\r",
        ch->level+10);
	  send_to_char(buf, ch);
	  return;
      }

	item = forge_lookup(arg4);
	show = 0;
	if(arg4[0] == '\0' || item == -1 )
	{
	  send_to_char("You do not know how to forge that item.\n\r",ch);
	  sprintf(buf, "Here is a list of the items you can forge with %s material.\n\r",
        raw_mat->material);
	  send_to_char(buf, ch);
	  send_to_char("More items are available at higher levels.\n\r\n\r",ch);

	  for(item = 0; forge_items_table[item].name != NULL; item++)
	  {
	    if( ch->level < (forge_items_table[item].level - 20) 
	    || str_cmp(forge_items_table[item].material, raw_mat->material) )
		continue;

	    sprintf(buf, "%-15s", forge_items_table[item].name);
	    show++;
	    send_to_char(buf, ch);

	    if( (show % 4) == 0 )
	    send_to_char("\n\r",ch);
	  }

	  if(show == 0)
	  {
	    sprintf(buf, "There are no objects you can forge with %s material.\n\r",
          raw_mat->material);
	    send_to_char(buf, ch);
	    return;
	  }

	  send_to_char("\n\r",ch);
	  return;
	}

	inv = get_eq_char(ch, WEAR_WIELD);
	if( (inv == NULL) || (inv->item_type != ITEM_BHAMMER) )
	{
	  send_to_char("You must be wielding your blacksmithing hammer to forge something.\n\r",ch);
	  return;
	}

	if( ch->level < (forge_items_table[item].level - 20) )
	{
	  send_to_char("You are too inexperienced to forge that item.\n\r",ch);
	  return;
	}

	if( str_cmp(forge_items_table[item].material, raw_mat->material) )
	{
	  sprintf( buf, "%s is made of a %s material,\n\rso, you cannot use %s to forge %s.\n\r",
	  capitalize(forge_items_table[item].short_descr),forge_items_table[item].material,
	  raw_mat->material, forge_items_table[item].short_descr );
	  send_to_char(buf, ch);
	  return;
	}

	material_count = 0;
	for(inv = ch->carrying; inv != NULL; inv = inv->next_content)
	{
	  if( inv->item_type == ITEM_RAW_MATERIAL
        && !str_cmp(inv->material, forge_items_table[item].material) )
	    material_count++;
	}

	if( material_count < forge_items_table[item].weight )
	{
	  sprintf(buf, "You need %d total pieces of raw material to forge that.\n\r",
        forge_items_table[item].weight);
	  send_to_char(buf, ch);
	  return;
	}

	if( (ch->carry_weight + (forge_items_table[item].weight * 10)) > can_carry_w(ch) )
	{
	  send_to_char("You are carrying too much weight to handle the newly forged item.\n\r",ch);
	  return;
	}

	steel = forge_items_table[item].cost*2;
	steel = steel/100;
	if( steel > ch->steel )
	{
	  sprintf(buf, "You need %d steel coins to forge that object.\n\r",steel);
	  send_to_char(buf, ch);
	  return;
	}

	for(room_obj = ch->in_room->contents; room_obj != NULL; room_obj = room_obj->next_content)
	{
	  if( room_obj->item_type == ITEM_FURNACE )
	    furnace = room_obj;

	  if( room_obj->item_type == ITEM_BARREL )
	    barrel = room_obj;

	  if( room_obj->item_type == ITEM_GRINDER )
	    grinder = room_obj;

	  if( room_obj->item_type == ITEM_ANVIL )
	    anvil = room_obj;
	}

	if( furnace == NULL || barrel == NULL || grinder == NULL || anvil ==NULL )
	{
	  send_to_char("You must have the following items in the same room with you to Forge.\n\r",ch);
	  send_to_char(" 1) A Furnace.\t2) A Cooling Barrel\t3) A Grinder.\t4) An anvil.\n\r",ch);
	  return;
	}

	if( get_skill(ch,gsn_fast_working) >= 1)
	{
	  if( number_percent() > get_skill(ch,gsn_fast_working) )
	  {
	    check_improve(ch,gsn_fast_working,FALSE,1);
	    wait = forge_items_table[item].beats*3;
	  }
	  else
	  {
	    check_improve(ch,gsn_fast_working,TRUE,1);
	    wait = forge_items_table[item].beats*3;
          wait = wait*3/4;
	  }
	}
	else
	{
	  wait = forge_items_table[item].beats*3;
	}

	if(wait == 0)
	{
	  bugf("forge: wait = 0, item = %s. Defaulting to 35 wait.",
        forge_items_table[item].name);
	  wait = 35;
	}

/*woot. End result time. Let's make some funky voodoo magic.*/

  for ( d = descriptor_list; d != NULL; d = d->next )
  {
    if ( d->connected == CON_PLAYING
    &&   d->character != ch
    &&   d->character->in_room != NULL
    &&   d->character->in_room != ch->in_room
    &&   d->character->in_room->area == ch->in_room->area
    &&   !IS_SET(d->character->in_room->room_flags, ROOM_QUIET)
    &&   !IS_AFFECTED2(d->character, AFF_DEAFNESS)
    &&   number_percent() < 5 )
    {
      act("The sounds of hammering can be heard in the distance.",ch,argument,d->character,TO_VICT);
    }
  }

	sprintf(buf, "$n begins to forge %s.", forge_items_table[item].short_descr);
	act(buf, ch, NULL, NULL, TO_ROOM);
	WAIT_STATE(ch, wait);
	if( number_percent() > get_skill(ch, gsn_smith) )
	{
	  sprintf(buf, "You fail to forge %s.\n\r",
        forge_items_table[item].short_descr);
	  send_to_char(buf, ch);
	  ch->steel -= steel;

	  for(material_count = 0; material_count < forge_items_table[item].weight; material_count++)
	  {
	    inv = get_obj_carry(ch, arg, ch);

	    if( inv->item_type == ITEM_RAW_MATERIAL
	    && !str_cmp(inv->material, forge_items_table[item].material) )
	      extract_obj(inv);
	  }

	  sprintf(buf, "$n fails to forge %s.", forge_items_table[item].short_descr);
	  act(buf, ch, NULL, NULL, TO_ROOM);
	  check_improve(ch, gsn_smith, FALSE, 1);
	  return;
	}
	else //Woo hoo!
	{
	  obj = create_object( get_obj_index(OBJ_VNUM_FORGE_LOADER), forge_items_table[item].level );

	  free_string(obj->name);
	  obj->name = str_dup(forge_items_table[item].name);

	  obj->item_type = 0;
	  obj->item_type = forge_items_table[item].item_type;

	  obj->level = 0;
	  obj->level = value;

	  obj->wear_flags = 0;
	  obj->wear_flags = forge_items_table[item].wear_flags|ITEM_SIZE_IGNORE;

	  free_string(obj->material);
	  obj->material = str_dup(forge_items_table[item].material);

	  obj->weight = 0;
	  obj->weight = forge_items_table[item].weight;

	  obj->cost = 0;
	  obj->cost = value * 10;

	  free_string(obj->short_descr);
	  obj->short_descr = str_dup(forge_items_table[item].short_descr);

	  free_string(obj->description);
	  obj->description = str_dup(forge_items_table[item].long_descr);

  /* Clear all object values */
	  obj->value[0] = 0;
	  obj->value[1] = 0;
	  obj->value[2] = 0;
	  obj->value[3] = 0;
	  obj->value[4] = 0;

  /* Now set them per object type */

        /* This can be written in a more simplified way, but it's 0200, I'm tired and
         * doing the first easiest thing that comes to mind
         */
        if(value == 0  || value == 1  || value == 2  || value == 3  || value == 4  ||
	     value == 7  || value == 8  || value == 9  || value == 10 || value == 15 ||
           value == 16 || value == 39 || value == 40 || value == 51 || value == 52 ||
           value == 75 || value == 76 || value == 99)
          wvone = 2;
        else
        if(value == 5  || value == 6  || value == 11 || value == 12 || value == 19 ||
	     value == 20 || value == 31 || value == 32 || value == 79 || value == 80 )
	    wvone = 3;
        else
        if(value == 13 || value == 14 || value == 17 || value == 18 || value == 21 ||
	     value == 22 || value == 25 || value == 26 || value == 29 || value == 30 ||
	     value == 45 || value == 46 || value == 61 || value == 62 || value == 85 ||
           value == 86 )
          wvone = 4;
        else
        if(value == 23 || value == 24 || value == 27 || value == 28 || value == 33 ||
	     value == 34 || value == 37 || value == 38 || value == 43 || value == 44 ||
	     value == 57 || value == 58 || value == 67 || value == 68 || value == 77 ||
	     value == 78 || value == 87 || value == 88 || value == 93 || value == 94 )
          wvone = 5;
        else
        if(value == 35 || value == 36 || value == 41 || value == 42 || value == 47 ||
	     value == 48 || value == 53 || value == 54 || value == 59 || value == 60 ||
	     value == 71 || value == 72 || value == 95 || value == 96 )
          wvone = 6;
        else
        if(value == 49 || value == 50 || value == 55 || value == 56 || value == 63 ||
	     value == 64 || value == 69 || value == 70 )
          wvone = 7;
        else
        if(value == 65 || value == 66 || value == 73 || value == 74 || value == 81 ||
	     value == 82 || value == 89 || value == 90 || value == 97 || value == 98 )
          wvone = 8;
        else
        if(value == 83 || value == 84 || value == 91 || value == 92 )
          wvone = 9;
        else
          wvone = 10;

        if(value == 1 || value == 2 || value == 5 || value == 6 )
          wvtwo = 3;
        else
        if(value == 3 || value == 4 || value == 13 || value == 14 )
          wvtwo = 4;
        else
        if(value == 11 || value == 12 || value == 17 || value == 18 || value == 23 ||
 	     value == 24 )
          wvtwo = 5;
        else
        if(value == 7  || value == 8  || value == 21 || value == 22 || value == 27 ||
 	     value == 28 || value == 35 || value == 36 )
          wvtwo = 6;
        else
        if(value == 9  || value == 10 || value == 25 || value == 26 || value == 33 ||
 	     value == 34 || value == 41 || value == 42 || value == 49 || value == 50  )
          wvtwo = 7;
        else
        if(value == 19 || value == 20 || value == 29 || value == 30 || value == 37 ||
 	     value == 38 || value == 47 || value == 48 || value == 55 || value == 56 ||
           value == 65 || value == 66 || value == 50  )
          wvtwo = 8;
        else
        if(value == 43 || value == 44 || value == 53 || value == 54 || value == 63 ||
 	     value == 64 || value == 73 || value == 74 || value == 83 || value == 84  )
          wvtwo = 9;
        else
        if(value == 15 || value == 16 || value == 59 || value == 60 || value == 69 ||
 	     value == 70 || value == 81 || value == 82 || value == 91 || value == 92 ||
 	     value == 100 )
          wvtwo = 10;
        else
        if(value == 89 || value == 90 )
          wvtwo = 11;
        else
        if(value == 31 || value == 32 || value == 45 || value == 46 || value == 57 ||
 	     value == 58 || value == 71 || value == 72 || value == 97 || value == 98 )
          wvtwo = 12;
        else
        if(value == 67 || value == 68 )
          wvtwo = 14;
        else
        if(value == 61 || value == 62 || value == 77 || value == 78 || value == 95 ||
	  value == 96 )
          wvtwo = 16;
        else
        if(value == 87 || value == 88 )
          wvtwo = 18;
        else
        if(value == 93 || value == 94 )
          wvtwo = 19;
        else
        if(value == 39 || value == 40 || value == 85 || value == 86 )
          wvtwo = 22;
        else
        if(value == 51 || value == 52 || value == 79 || value == 80 )
          wvtwo = 28;
        else
        if(value == 75 || value == 76 )
          wvtwo = 40;
        else
        if(value == 99 )
          wvtwo = 52;
        else
          wvtwo = 16;

	  if( obj->item_type == ITEM_WEAPON )
	  {
	    obj->value[0] = forge_items_table[item].weapon_class;
	    obj->value[1] = wvone;
	    obj->value[2] = wvtwo;
	    obj->value[3] = attack_lookup( forge_items_table[item].dam_noun );
	  }

        if( value <= 4 )
        {
          acp = 1;
          acb = 1;
          acs = 1;
          ace = 0;
        }
        else
        if( value <= 9 )
        {
          acp = 2;
          acb = 2;
          acs = 2;
          ace = 0;
	  }
        else
        if( value <= 14 )
        {
          acp = 3;
          acb = 3;
          acs = 3;
          ace = 0;
	  }
        else
        if( value <= 19 )
        {
          acp = 4;
          acb = 4;
          acs = 4;
          ace = 0;
	  }
        else
        if( value <= 24 )
        {
          acp = 5;
          acb = 5;
          acs = 5;
          ace = 1;
	  }
        else
        if( value <= 29 )
        {
          acp = 6;
          acb = 6;
          acs = 6;
          ace = 2;
	  }
        else
        if( value <= 34 )
        {
          acp = 7;
          acb = 7;
          acs = 7;
          ace = 3;
	  }
        else
        if( value <= 39 )
        {
          acp = 8;
          acb = 8;
          acs = 8;
          ace = 4;
	  }
        else
        if( value <= 44 )
        {
          acp = 9;
          acb = 9;
          acs = 9;
          ace = 5;
	  }
        else
        if( value <= 49 )
        {
          acp = 10;
          acb = 10;
          acs = 10;
          ace = 6;
	  }
        else
        if( value <= 54 )
        {
          acp = 11;
          acb = 11;
          acs = 11;
          ace = 7;
	  }
        else
        if( value <= 59 )
        {
          acp = 12;
          acb = 12;
          acs = 12;
          ace = 8;
	  }
        else
        if( value <= 64 )
        {
          acp = 13;
          acb = 13;
          acs = 13;
          ace = 9;
	  }
        else
        if( value <= 69 )
        {
          acp = 14;
          acb = 14;
          acs = 14;
          ace = 10;
	  }
        else
        if( value <= 74 )
        {
          acp = 15;
          acb = 15;
          acs = 15;
          ace = 11;
	  }
        else
        if( value <= 79 )
        {
          acp = 16;
          acb = 16;
          acs = 16;
          ace = 12;
	  }
        else
        if( value <= 84 )
        {
          acp = 17;
          acb = 17;
          acs = 17;
          ace = 13;
	  }
        else
        if( value <= 89 )
        {
          acp = 18;
          acb = 18;
          acs = 18;
          ace = 14;
	  }
        else
        if( value <= 94 )
        {
          acp = 19;
          acb = 19;
          acs = 19;
          ace = 15;
	  }
        else
        if( value <= 99 )
        {
          acp = 20;
          acb = 20;
          acs = 20;
          ace = 16;
	  }
        else
        if( value <= 104 )
        {
          acp = 21;
          acb = 21;
          acs = 21;
          ace = 17;
	  }
        else
        if( value <= 109 )
        {
          acp = 22;
          acb = 22;
          acs = 22;
          ace = 18;
	  }
        else
        {
          acp = 23;
          acb = 23;
          acs = 23;
          ace = 19;
	  }

	  if( obj->item_type == ITEM_ARMOR )
	  {
	    obj->value[0] = acp;
	    obj->value[1] = acb;
	    obj->value[2] = acs;
	    obj->value[3] = ace;
	    SET_BIT(obj->value[4], forge_items_table[item].armor_type);
	  }

	  ch->steel -= steel;

	  for(material_count = 0; material_count < forge_items_table[item].weight; material_count++)
	  {
	    inv = get_obj_carry(ch, arg, ch);

	    if( inv->item_type == ITEM_RAW_MATERIAL
	    && !str_cmp(inv->material, forge_items_table[item].material) )
	      extract_obj(inv);
	  }

	  obj_to_char(obj, ch);
	  act("You successfully create $p in your forge.",ch, obj, NULL, TO_CHAR);
	  check_improve(ch, gsn_smith, TRUE, 1);
	  if(number_percent() <= 25) gain_exp(ch, 50);
	  act("$n has successfully created $p in $s forge.",ch, obj, NULL, TO_ROOM);
	  return;
  }
  return;
}

void do_masterwork( CHAR_DATA *ch, char *argument )
{
  DESCRIPTOR_DATA *d;
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *vch;
  CHAR_DATA *smith2 = NULL;
  OBJ_DATA *obj, *wield;
  bool can_masterwork = FALSE;
  int wait, wait2, cost;
  int number, sk1, sk2;

  smash_tilde(argument);
  one_argument(argument, arg);

  if( IS_NPC(ch) )
  {
    send_to_char("Not with MOBs.\n\r",ch);
    return;
  }

  if( get_skill(ch,gsn_masterwork) < 1 )
  {
    send_to_char("You don't know how to do that.\n\r",ch);
    return;
  }

  if( arg[0] == '\0' )
  {
    send_to_char("Masterwork what?\n\r",ch);
    return;
  }

  wield = get_eq_char(ch, WEAR_WIELD);
  if( (wield == NULL) || (wield->item_type != ITEM_BHAMMER) )
  {
    send_to_char("You must be wielding your blacksmithing hammer to make something perfect.\n\r",ch);
    return;
  }

  obj = get_obj_carry(ch, arg, ch);
  if(obj == NULL)
  {
    send_to_char("You do not have that item.\n\r",ch);
    return;
  }

  if( obj->item_type == ITEM_WEAPON || obj->item_type == ITEM_ARMOR )
    can_masterwork = TRUE;

  if(!can_masterwork)
  {
    send_to_char("You can only masterwork a weapon or a piece of armor.\n\r",ch);
    return;
  }

  if( IS_SET(obj->extra_flags, ITEM_MASTERWORK) )
  {
    act("$p already looks like a masterwork piece of gear!",ch,obj,NULL,TO_CHAR);
    return;
  }

  if(obj->timer > 0)
  {
    act("$p is in no condition to be perfected.",ch,obj,NULL,TO_CHAR);
    return;
  }

  for(vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
  {
    if(ch == vch
    || IS_NPC(vch)
    || vch->class != class_lookup("blacksmith")
    || get_skill(vch, gsn_masterwork) < 1 )
	continue;

    smith2 = vch;
  }

  if(smith2 == NULL)
  {
    send_to_char("There must be a second blacksmith in the room who knows how to Masterwork\n\rto help with such a task.\n\r",ch);
    return;
  }

  wield = get_eq_char(smith2, WEAR_WIELD);
  if( (wield == NULL) || (wield->item_type != ITEM_BHAMMER) )
  {
    send_to_char("The second smith must also be wielding a blacksmithing hammer.\n\r",ch);
    send_to_char("You must be wielding your blacksmithing hammer to make something perfect.\n\r",smith2);
    return;
  }

  if( obj->pIndexData->vnum != OBJ_VNUM_FORGE_LOADER
  &&  obj->cost < 500 )
  {
    send_to_char("Try perfecting something worthwhile, and not a threepence item.\n\r",ch);
    return;
  }

  if( obj->pIndexData->vnum == OBJ_VNUM_FORGE_LOADER
  &&  obj->level < 35 )
  {
    send_to_char("Try perfecting something worthwhile, and not a threepence item.\n\r",ch);
    return;
  }
	
  if( obj->item_type == ITEM_WEAPON 
  &&( (1 + obj->value[2]) * obj->value[1] / 2) > 75 ) //Average > 75
    can_masterwork = FALSE;

  if( obj->item_type == ITEM_ARMOR 
  &&(obj->value[0] > 30
  || obj->value[1] > 30 
  || obj->value[2] > 30
  || obj->value[3] > 30) )
    can_masterwork = FALSE;

  if(!can_masterwork)
  {
    act("$p looks near perfect already!",ch,obj,NULL,TO_CHAR);
    return;
  }

  if(obj->enchanted)
  {
    send_to_char("You cannot Masterwork an object so enchanted.\n\r",ch);
    return;
  }

  if(obj->pIndexData->vnum == OBJ_VNUM_FORGE_LOADER)
  {
    cost = (obj->cost*10) / 100;
  }
  else
  {
    cost = (obj->cost*5) / 100;
  }

  if(ch->steel < cost)
  {
    sprintf(buf, "You need %d steel pieces to perfect that object.\n\r",cost);
    send_to_char(buf, ch);
    return;
  }

  wait = fast_working(ch, gsn_masterwork);
  wait2 = fast_working(smith2, gsn_masterwork);

  WAIT_STATE(ch, wait);
  WAIT_STATE(smith2, wait2);

  for ( d = descriptor_list; d != NULL; d = d->next )
  {
    if ( d->connected == CON_PLAYING
    &&   d->character != ch
    &&   d->character->in_room != NULL
    &&   d->character->in_room != ch->in_room
    &&   d->character->in_room->area == ch->in_room->area
    &&   !IS_SET(d->character->in_room->room_flags, ROOM_QUIET)
    &&   !IS_AFFECTED2(d->character, AFF_DEAFNESS)
    &&   number_percent() < 5 )
    {
      act("The sounds of hammering can be heard in the distance.",ch,argument,d->character,TO_VICT);
    }
  }

  act("$n and $N being to craft $p into perfect shape.",ch,obj,smith2,TO_NOTVICT);
  sprintf(buf, "You begin to craft %s into perfect shape.\n\r",obj->short_descr);
  send_to_char(buf,ch);
  send_to_char(buf,smith2);

  number = number_percent();
  number += 15;
  sk1 = get_skill(ch,gsn_masterwork);
  sk2 = get_skill(smith2, gsn_masterwork);

  if( number_range(100,250) > (sk1 + sk2) ) //complete failure
  {
    act("You fail to re-craft $p.",ch,obj,NULL,TO_CHAR);
    act("You fail to re-craft $p.",smith2,obj,NULL,TO_CHAR);
    ch->steel -= (cost)/2;
    check_improve(ch,gsn_masterwork,FALSE,1);
    check_improve(smith2,gsn_masterwork,FALSE,1);
    act("$n and $N fail to re-craft $p.",ch,obj,smith2,TO_NOTVICT);
	return;
  }
  else
  {
    SET_BIT(obj->extra_flags, ITEM_MASTERWORK);
    ch->steel -= cost;

    if(number < sk1 && number < sk2) //both pass
    {
	act("You successfully re-craft $p.",ch,obj,NULL,TO_CHAR);
	act("You successfully re-craft $p.",smith2,obj,NULL,TO_CHAR);

      if(obj->item_type == ITEM_WEAPON)
	  obj->value[2] += 4; //add more sides to the dam dice..

      if(obj->item_type == ITEM_ARMOR) //Add a small amount to all ac values..
      {
	  obj->value[0] += 6;
	  obj->value[1] += 6;
	  obj->value[2] += 6;
	  obj->value[3] += 6;
      }

	check_improve(ch,gsn_masterwork,TRUE,1);
	check_improve(smith2,gsn_masterwork,TRUE,1);
			
	if( bs_exp(ch, gsn_masterwork) )
	  gain_exp(ch,50);

	if( bs_exp(smith2, gsn_masterwork) )
	  gain_exp(smith2,50);

	act("$n and $N successfully re-craft $p.",ch,obj,smith2,TO_NOTVICT);
	return;
    }

    if(number > sk1 && number > sk2) //both FAIL, yet top PASS
    {
	act("You successfully re-craft $p, although it didn't turn out so great...",
        ch,obj,NULL,TO_CHAR);
	act("You successfully re-craft $p, although it didn't turn out so great...",
        smith2,obj,NULL,TO_CHAR);
	if(obj->item_type == ITEM_WEAPON) obj->value[2] += 1;

	if(obj->item_type == ITEM_ARMOR)
	{
	  obj->value[0] += 1;
	  obj->value[1] += 1;
	  obj->value[2] += 1;
	  obj->value[3] += 1;
	}

	check_improve(ch,gsn_masterwork,FALSE,1);
	check_improve(smith2,gsn_masterwork,FALSE,1);
	act("$n and $N successfully re-craft $p, although it didn't turn out so great...",
        ch,obj,smith2,TO_NOTVICT);
	return;
    }

    if(number < sk1 && number > sk2) //smith2 FAIL
    {
	act("You successfully re-craft $p, although it didn't turn out so great because your partner failed.",ch,obj,NULL,TO_CHAR);
	sprintf(buf, "You try to aid %s, but fail.\n\r",ch->name);
	send_to_char(buf,smith2);

	if(obj->item_type == ITEM_WEAPON) obj->value[2] += 3;

	if(obj->item_type == ITEM_ARMOR)
	{
	  obj->value[0] += 4;
	  obj->value[1] += 4;
	  obj->value[2] += 4;
	  obj->value[3] += 4;
	}

	check_improve(ch,gsn_masterwork,TRUE,1);
	check_improve(smith2,gsn_masterwork,FALSE,1);

	if( bs_exp(ch, gsn_masterwork) ) gain_exp(ch,50);

	act("$n successfully re-crafts $p, but it didn't turn out so great....",ch,obj,NULL,TO_ROOM);
	return;
    }

    if(number > sk1 && number < sk2) //ch (smith1) FAIL
    {
	act("You successfully re-craft $p, but it didn't turn out so great because your partner failed.",smith2,obj,NULL,TO_CHAR);
	sprintf(buf, "You try to aid %s, but fail.\n\r",smith2->name);
	send_to_char(buf,ch);

	if(obj->item_type == ITEM_WEAPON) obj->value[2] += 2;

	if(obj->item_type == ITEM_ARMOR)
	{
	  obj->value[0] += 2;
	  obj->value[1] += 2;
	  obj->value[2] += 2;
	  obj->value[3] += 2;
	}

	check_improve(smith2,gsn_masterwork,TRUE,1);
	check_improve(ch,gsn_masterwork,FALSE,1);
	if( bs_exp(smith2, gsn_masterwork) ) gain_exp(smith2,50);
	act("$n successfully re-crafts $p, but it didn't turn out that great....",smith2,obj,NULL,TO_ROOM);
	return;
    } 
    return;
  } 
  return;
}

void do_dismantle( CHAR_DATA *ch, char *argument )
{
	DESCRIPTOR_DATA *d;
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	OBJ_DATA *obj, *obj2;
	int num_materials = 0, a = 0;
	int wait, steel;

	one_argument(argument, arg);

	if( get_skill(ch, gsn_dismantle) < 1 )
	{
		send_to_char("You don't know how to dismantle gear.\n\r",ch);
		return;
	}

	if(IS_NPC(ch))
	{
		send_to_char("Mobs can't do that.\n\r",ch);
		return;
	}

	if( arg[0] == '\0' )
	{
		send_to_char("Dismantle what?\n\r",ch);
		return;
	}

	if( (obj = get_obj_carry(ch, arg, ch)) == NULL )
	{
		send_to_char("You do not have that item.\n\r",ch);
		return;
	}

	if( obj->item_type != ITEM_WEAPON && obj->item_type != ITEM_ARMOR )
	{
		send_to_char("You can only dismantle weapons or armor.\n\r",ch);
		return;
	}

	if(!str_cmp(obj->material, "unknown")
	|| (material_lookup(obj->material) == -1) )
	{
		send_to_char("You don't know what material this is made out of, so you cannot dismantle it.\n\r",ch);
		return;
	}

	num_materials = obj->weight;

	if(num_materials == 0)
	{
		act("$p does not have enough usable materials to be dismantled.",ch,obj,NULL,TO_CHAR);
		return;
	}

	if ( ch->carry_number + num_materials > can_carry_n(ch) )
	{
		send_to_char("You are carrying too many things to handle additional raw materials.\n\r",ch);
		return;
	}

	steel = obj->weight;
	if(steel > ch->steel)
	{
	  send_to_char("You have insufficient funds to dismantle that.\n\r",ch);
	  return;
	}

	wait = fast_working(ch, gsn_dismantle);

  for ( d = descriptor_list; d != NULL; d = d->next )
  {
    if ( d->connected == CON_PLAYING
    &&   d->character != ch
    &&   d->character->in_room != NULL
    &&   d->character->in_room != ch->in_room
    &&   d->character->in_room->area == ch->in_room->area
    &&   !IS_SET(d->character->in_room->room_flags, ROOM_QUIET)
    &&   !IS_AFFECTED2(d->character, AFF_DEAFNESS)
    &&   number_percent() < 5 )
    {
      act("The sounds of hammering can be heard in the distance.",ch,argument,d->character,TO_VICT);
    }
  }

	act("$n begins to dismantle $p.",ch,obj,NULL,TO_ROOM);
	WAIT_STATE(ch, wait);
	if( number_percent() > get_skill(ch, gsn_dismantle) || num_materials == 0 )
	{
		act("You fail to dismantle $p, breaking it into unusable trash.",ch,obj,NULL,TO_CHAR);
		ch->steel -= steel;
		extract_obj(obj);
		if(num_materials > 0) check_improve(ch, gsn_dismantle, FALSE, 1);
		act("$n fails to dismantle $p, breaking it into unusable trash.",ch,obj,NULL,TO_ROOM);
		return;
	}
	else
	{
		act("You successfully dismantle $p, breaking it into raw materials.",ch,obj,NULL,TO_CHAR);
		act("$n successfully dismantles $p, breaking it into raw materials.",ch,obj,NULL,TO_ROOM);
		ch->steel -= steel;
		sprintf(buf, "%s", obj->material);
		extract_obj(obj);

		for(a=0; a < num_materials; a++)
		{
			obj2 = create_object( get_obj_index(OBJ_VNUM_DISMANTLE), 0 );
			obj_to_char(obj2, ch);

			free_string(obj2->material);
			obj2->material = str_dup(buf);

			free_string(obj2->name);
			sprintf( buf2, "piece %s raw material", buf);
			obj2->name = str_dup(buf2);

			free_string(obj2->short_descr);
			sprintf( buf2, "a piece of %s material",buf);
			obj2->short_descr = str_dup(buf2);

			free_string(obj2->description);
			sprintf( buf2, "This piece of %s material looks good for raw materials.", buf);
			obj2->description = str_dup(buf2);
		}

		check_improve(ch,gsn_dismantle,TRUE,1);

/* This success EXP is commented out because
 * people were spamming this command for the EXP
		if( bs_exp(ch, gsn_dismantle) ) gain_exp(ch, 50);
*/

		return;
	} //end success
	return;
} //end dismantle

/* Cultural Artistry Weapons - Retool */
void do_retool( CHAR_DATA *ch, char *argument )
{
	DESCRIPTOR_DATA *d;
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj, *hold;
	int wait, race, affect_number;
	AFFECT_DATA af;
	AFFECT_DATA *paf;
	bool Aff = TRUE;
	int mod = 0;
      int steel = 300;

	struct gnome_affect_type
	{
		int bit;
		int af_where;
	};

	static const struct gnome_affect_type gnome_affect [] =
	{
	  { ITEM_GLOW,		TO_OBJECT },
	  { ITEM_INVIS,		TO_OBJECT },
	  { ITEM_ANTI_GOOD,	TO_OBJECT },
	  { ITEM_ANTI_NEUTRAL, 	TO_OBJECT },
	  { ITEM_ANTI_EVIL,	TO_OBJECT },
	  { ITEM_HUM,		TO_OBJECT },
	  { ITEM_NODROP,		TO_OBJECT },
	  { ITEM_NOIDENT,		TO_OBJECT },
	  { ITEM_NOREMOVE,	TO_OBJECT },
	  { ITEM_BURN_PROOF,	TO_OBJECT },
	  { WEAPON_VORPAL,	TO_WEAPON },
	  { WEAPON_FROST,		TO_WEAPON },
	  { WEAPON_SHOCKING,	TO_WEAPON },
	  { WEAPON_DISRUPTION, 	TO_WEAPON }
	};

	one_argument(argument, arg);

	if(IS_NPC(ch))
	{
		send_to_char("NPCs can't do this.\n\r",ch);
		return;
	}

	if(ch->pcdata->learned[gsn_cultural_artistry] < 1)
	{
		send_to_char("You don't know how to retool weapons with cultural art.\n\r",ch);
		return;
	}

	if(ch->race == race_lookup("aghar"))
	{
		send_to_char("Your race doesn't retool weapons.\n\r",ch);
		return;
	}

	if(arg[0] == '\0')
	{
		send_to_char("Retool what weapon with your cultural art?\n\r",ch);
		return;
	}

	hold = get_eq_char(ch, WEAR_WIELD);
	if((hold == NULL) || (hold->item_type != ITEM_BHAMMER))
	{
		send_to_char("You must be wielding your blacksmithing hammer to retool weapons.\n\r",ch);
		return;
	}

	if((obj = get_obj_carry(ch, arg, ch)) == NULL)
	{
		send_to_char("You do not have that item.\n\r",ch);
		return;
	}

  if(obj->item_type != ITEM_WEAPON)
  {
    send_to_char("You may only retool weapons with your cultural art.\n\r",ch);
    return;
  }

  if(obj->pIndexData->vnum != OBJ_VNUM_FORGE_LOADER)
  {
    send_to_char("You may only apply your cultural art to items created in your Forge.\n\r",ch);
    return;
  }

  if( IS_SET(obj->extra2_flags, ITEM_ARTISTRY) )
  {
    act("$p already has cultural art upon it.",ch,obj,NULL,TO_CHAR);
    return;
  }

  for(paf = obj->affected; paf!= NULL; paf = paf->next)
  {
    if(paf->type == gsn_cultural_artistry)
    {
	act("$p already has cultural art upon it.",ch,obj,NULL,TO_CHAR);
	return;
    }
  }

  if(obj->enchanted)
  {
    send_to_char("You cannot Retool a weapon so enchanted.\n\r",ch);
    return;
  }

      race = race_lookup(pc_race_table[ch->race].name);

	switch(race)
	{
	  case 0:
	    steel = 300;
          break;

	  case 1:
          if(ch->level >= 50)  steel = 300;
          if(ch->level >= 60)  steel = 400;
          if(ch->level >= 70)  steel = 500;
          if(ch->level >= 80)  steel = 600;
          if(ch->level >= 90)  steel = 700;
          break;

	  case 2:
	    steel = 300;
          break;

	  case 3:
          if(ch->level >= 50)  steel = 300;
          if(ch->level >= 60)  steel = 510;
          if(ch->level >= 70)  steel = 580;
          if(ch->level >= 80)  steel = 720;
          if(ch->level >= 90)  steel = 860;
          if(ch->level >= 95)  steel = 1000;
          break;

	  case 4:
	    steel = 300;
          break;

	  case 5:
          if(ch->level >= 50)  steel = 300;
          if(ch->level >= 60)  steel = 400;
          if(ch->level >= 70)  steel = 500;
          if(ch->level >= 80)  steel = 600;
          if(ch->level >= 90)  steel = 700;
          break;

	  case 6:
          if(ch->level >= 50)  steel = 300;
          if(ch->level >= 70)  steel = 500;
          if(ch->level >= 90)  steel = 700;
          break;

	  case 7:
	  case 8:
	    steel = 300;
          break;

	  case 9:
          if(ch->level >= 50)  steel = 300;
          if(ch->level >= 70)  steel = 500;
          if(ch->level >= 90)  steel = 700;
          break;

	  case 10:
	  case 11:
	    steel = 300;
          break;

	  case 12:
	    steel = 0;
          break;

	  case 13:
	  case 14:
	  case 15:
	    steel = 300;
          break;

	  case 16:
	    steel = 500;
          break;

	  case 17:
          if(ch->level >= 50)  steel = 300;
          if(ch->level >= 70)  steel = 500;
          if(ch->level >= 90)  steel = 700;
          break;

	  case 18:
	    steel = 300;
          break;

	  case 19:
          if(ch->level >= 50)  steel = 300;
          if(ch->level >= 70)  steel = 500;
          if(ch->level >= 90)  steel = 700;
          break;

	  default: //default
	    steel = 300;
          break;
	}

	if(ch->steel < steel)
	{
		send_to_char("You do not have enough steel money to retool a weapon with your cultural art.\n\r",ch);
		return;
	}

  wait = fast_working(ch, gsn_cultural_artistry);

  WAIT_STATE(ch, wait);

  for ( d = descriptor_list; d != NULL; d = d->next )
  {
    if ( d->connected == CON_PLAYING
    &&   d->character != ch
    &&   d->character->in_room != NULL
    &&   d->character->in_room != ch->in_room
    &&   d->character->in_room->area == ch->in_room->area
    &&   !IS_SET(d->character->in_room->room_flags, ROOM_QUIET)
    &&   !IS_AFFECTED2(d->character, AFF_DEAFNESS)
    &&   number_percent() < 5 )
    {
      act("The sounds of hammering can be heard in the distance.",ch,argument,d->character,TO_VICT);
    }
  }

  act("You begin to retool $p with your cultural art.",ch,obj,NULL,TO_CHAR);
  act("$n begins to retool $p with $s cultural art.",ch,obj,NULL,TO_ROOM);

  if(number_percent() > ch->pcdata->learned[gsn_cultural_artistry])
  {
    act("You fail to craft your cultural art.",ch,NULL,NULL,TO_CHAR);
    act("$n fails to craft $s cultural art.",ch,NULL,NULL,TO_ROOM);
    ch->steel -= steel;
    check_improve(ch, gsn_cultural_artistry, FALSE, 1);
    return;
  }
  else
  {
    act("You succeed at crafting your cultural art on $p.",ch,obj,NULL,TO_CHAR);
    act("$n succeeds at crafting $s cultural art on $p.",ch,obj,NULL,TO_ROOM);
    SET_BIT(obj->extra2_flags, ITEM_ARTISTRY);
    ch->steel -= steel;
    check_improve(ch,gsn_cultural_artistry,TRUE,1);
    if( bs_exp(ch, gsn_cultural_artistry) ) gain_exp(ch, 50);

    //Get Race # for Character's text race
	race = race_lookup(pc_race_table[ch->race].name);

	af.where 	= TO_OBJECT;
	af.type  	= gsn_cultural_artistry;
	af.level 	= ch->level;
	af.duration	= -1;
	af.location	= 0;
	af.modifier = 0;
	af.bitvector = 0;

	switch(race)
	{
	  case 0:  //NULL
	    Aff = FALSE;
	    bugf("Character %s race is NULL.",ch->name);
	    break;

	  case 1:  //Human
	    af.where 	= TO_OBJECT;
	    af.location = APPLY_DAMROLL;
	    af.modifier = 1 + (ch->level >= 50) + (ch->level >= 60) + (ch->level >= 70)+ (ch->level >= 80) + (ch->level >= 90);
	    break;

	  case 2:  //Draconian
				af.where = TO_WEAPON;
				af.bitvector = WEAPON_POISON;
				break;

        case 3:  //Half-Elf
				af.where = TO_AFFECTS;
				af.location = APPLY_HIT;

				if(ch->level >= 50) mod = 10;
				if(ch->level >= 60) mod = 15;
				if(ch->level >= 70) mod = 20;
				if(ch->level >= 80) mod = 30;
				if(ch->level >= 90) mod = 40;
				if(ch->level >= 95) mod = 50;

				af.modifier = mod;
				break;

	  case 4:  //Silvanesti
				af.where = TO_WEAPON;
				af.bitvector = WEAPON_HOLY;
				break;

	  case 5:  //Qualinesti
				af.where = TO_OBJECT;
				af.location = APPLY_HITROLL;
				af.modifier = 1 + (ch->level >= 50) + (ch->level >= 60) + (ch->level >= 70)+ (ch->level >= 80) + (ch->level >= 90);
				break;

			case 6:  //Kagonesti
				af.where = TO_OBJECT;
				af.location = APPLY_STR;
				af.modifier = 1 + (ch->level >= 50) + (ch->level >= 70) + (ch->level >= 90);
				break;

			case 7:  //Dargonesti
			case 8:  //Dimernesti
				af.where = TO_WEAPON;
				af.bitvector = WEAPON_SHOCKING;
				break;

			case 9:  //Neidar
				af.where = TO_OBJECT;
				af.location = APPLY_CHR;
				af.modifier = 1 + (ch->level >= 50) + (ch->level >= 70) + (ch->level >= 90);
				break;
				
			case 10: //Hylar
				if ((ch->ethos == 0)
				&& (obj->value[4] != WEAPON_THUNDERING))
                          SET_BIT(obj->value[4], WEAPON_THUNDERING);

				if ((ch->ethos == 1)
				&& (obj->value[4] != WEAPON_LAWFUL))
                          SET_BIT(obj->value[4], WEAPON_LAWFUL);


				if ((ch->ethos == 2)
				&& (obj->value[4] != WEAPON_CHAOTIC))
                          SET_BIT(obj->value[4], WEAPON_CHAOTIC);

				break;

			case 11: //Theiwar
				af.where = TO_WEAPON;
				af.bitvector = WEAPON_VAMPIRIC;
				break;

			case 12: //Aghar
				Aff = FALSE;
				send_to_char("You can't do this...\n\r",ch);
				break;

			case 13: //Daergar
				af.where = TO_WEAPON;
				af.bitvector = WEAPON_UNHOLY;
				break;

			case 14: //Daewar
				af.where = TO_WEAPON;
				af.bitvector = WEAPON_VORPAL;
				break;

			case 15: //Klar
				af.where = TO_OBJECT;
				af.bitvector = ITEM_STAINLESS;
				break;

			case 16: //Gnome
			affect_number = number_range(0,13);
			af.where = gnome_affect[affect_number].af_where;
			af.bitvector = gnome_affect[affect_number].bit;

			break;

			case 17: //Kender
				af.where = TO_OBJECT;
				af.location = APPLY_DEX;
				af.modifier = 1 + (ch->level >= 50) + (ch->level >= 70) + (ch->level >= 90);
				break;

			case 18: //Minotaur
				af.where = TO_WEAPON;
				af.bitvector = WEAPON_MIGHTY_CLEAVING;
				break;

			case 19: //Ogre
				af.where = TO_AFFECTS;
				af.location = APPLY_CON;
				af.modifier = 1 + (ch->level >= 50) + (ch->level >= 70) + (ch->level >= 90);
				break;

			default: //New Race?
				Aff = FALSE;
				bugf("Character %s race not recognized.",ch->name);
				break;

		}

		if(Aff) affect_to_obj(obj, &af);
		return;
	}
	return;
}

/* Cultural Artistry Armor and Jewelry - Craft */
void do_craft( CHAR_DATA *ch, char *argument )
{
  DESCRIPTOR_DATA *d;
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj, *hold;
  int wait, race, affect_number;
  AFFECT_DATA af;
  AFFECT_DATA *paf;
  bool Aff = TRUE;
  int mod = 0;
  int steel = 300;

  struct gnome_affect_type
  {
    int bit;
    int af_where;
    int location;
    int modifier;
  };

  static const struct gnome_affect_type gnome_affect [] =
  {
    { ITEM_BLESS,		TO_OBJECT,  0, 0 },
    { ITEM_EVIL,		TO_OBJECT,  0, 0 },
    { ITEM_INVIS,		TO_OBJECT,  0, 0 },
    { ITEM_BURN_PROOF,	TO_OBJECT,  0, 0 },
    { ITEM_NOREMOVE,	TO_OBJECT,  0, 0 },
    { ITEM_NONMETAL,	TO_OBJECT,  0, 0 },
    { ITEM_GLOW,		TO_OBJECT,  0, 0 },
    { ITEM_HUM,		TO_OBJECT,  0, 0 },
    { ITEM_MELT_DROP,	TO_OBJECT,  0, 0 },
    { ITEM_NOIDENT,	TO_OBJECT,  0, 0 },
    { 0,			TO_OBJECT,  APPLY_HITROLL, 5 },
    { 0,			TO_AFFECTS, APPLY_SAVES,  -2 },
    { 0,			TO_AFFECTS, APPLY_MANA,	  25 },
    { 0,			TO_AFFECTS, APPLY_HIT,	  25 },
    { 0,			TO_OBJECT,  APPLY_DAMROLL, 5 }
  };

  one_argument(argument, arg);

  if(IS_NPC(ch))
  {
    send_to_char("NPCs can't do this.\n\r",ch);
    return;
  }

  if(ch->pcdata->learned[gsn_cultural_artistry] < 1)
  {
    send_to_char("You don't know how to craft ANYTHING with cultural art.\n\r",ch);
    return;
  }

  if(ch->race == race_lookup("aghar"))
  {
    send_to_char("Your race doesn't craft anything.\n\r",ch);
    return;
  }

	if(arg[0] == '\0')
	{
	  send_to_char("Craft what item with your cultural art?\n\r",ch);
	  return;
	}

	hold = get_eq_char(ch, WEAR_WIELD);
	if((hold == NULL) || (hold->item_type != ITEM_BHAMMER))
	{
	  send_to_char("You must be wielding your blacksmithing hammer to craft something.\n\r",ch);
	  return;
	}

	if((obj = get_obj_carry(ch, arg, ch)) == NULL)
	{
	  send_to_char("You do not have that item.\n\r",ch);
	  return;
	}

	if(obj->item_type != ITEM_ARMOR && obj->item_type != ITEM_JEWELRY)
	{
	  send_to_char("You may only craft armor or jewelry with your cultural art.\n\r",ch);
	  return;
	}

	if(obj->pIndexData->vnum != OBJ_VNUM_FORGE_LOADER)
	{
	  send_to_char("You may only apply your cultural art to items created in your Forge.\n\r",ch);
	  return;
	}

	/* These following races have incompatible affects for jewelry */
	if(obj->item_type == ITEM_JEWELRY
	&&(ch->race == race_lookup("human")
	|| ch->race == race_lookup("daewar")
	|| ch->race == race_lookup("draconian")
	|| ch->race == race_lookup("neidar")
	|| ch->race == race_lookup("ogre")) )
	{
	  send_to_char("You cannot apply your Cultural Art to jewelry.\n\r",ch);
	  return;
	}

	if( IS_SET(obj->extra2_flags, ITEM_ARTISTRY) )
	{
	  act("$p already has cultural art upon it.",ch,obj,NULL,TO_CHAR);
	  return;
	}

	for(paf = obj->affected; paf!= NULL; paf = paf->next)
	{
	  if(paf->type == gsn_cultural_artistry)
	  {
	    act("$p already has cultural art upon it.",ch,obj,NULL,TO_CHAR);
	    return;
	  }
	}

	if(obj->enchanted)
	{
	  send_to_char("You cannot Craft an object so enchanted.\n\r",ch);
	  return;
	}

      race = race_lookup(pc_race_table[ch->race].name);

	switch(race)
	{
	  case 0:
	    steel = 300;
          break;

	  case 1:
          if(ch->level >= 50)  steel = 300;
          if(ch->level >= 60)  steel = 440;
          if(ch->level >= 70)  steel = 580;
          if(ch->level >= 80)  steel = 720;
          if(ch->level >= 90)  steel = 860;
          if(ch->level >= 100) steel = 1000;
          break;

	  case 2:
          if(ch->level >= 50)  steel = 300;
          if(ch->level >= 60)  steel = 440;
          if(ch->level >= 70)  steel = 580;
          if(ch->level >= 80)  steel = 720;
          if(ch->level >= 90)  steel = 860;
          if(ch->level >= 100) steel = 1000;
          break;

	  case 3:
          if(ch->level >= 50)  steel = 300;
          if(ch->level >= 75)  steel = 590;
          break;

	  case 4:
	    steel = 300;
          break;

	  case 5:
	    if(ch->level >= 50)  steel = 300;
	    if(ch->level >= 52)  steel = 328;
	    if(ch->level >= 54)  steel = 356;
	    if(ch->level >= 56)  steel = 384;
	    if(ch->level >= 58)  steel = 412;
	    if(ch->level >= 60)  steel = 440;
	    if(ch->level >= 62)  steel = 468;
	    if(ch->level >= 64)  steel = 496;
	    if(ch->level >= 66)  steel = 524;
	    if(ch->level >= 68)  steel = 552;
	    if(ch->level >= 70)  steel = 580;
	    if(ch->level >= 72)  steel = 608;
	    if(ch->level >= 74)  steel = 636;
	    if(ch->level >= 76)  steel = 664;
	    if(ch->level >= 78)  steel = 692;
	    if(ch->level >= 80)  steel = 720;
	    if(ch->level >= 82)  steel = 748;
	    if(ch->level >= 84)  steel = 776;
	    if(ch->level >= 86)  steel = 804;
	    if(ch->level >= 88)  steel = 832;
	    if(ch->level >= 90)  steel = 860;
	    if(ch->level >= 92)  steel = 888;
	    if(ch->level >= 94)  steel = 916;
	    if(ch->level >= 96)  steel = 944;
	    if(ch->level >= 98)  steel = 972;
	    if(ch->level >= 100) steel = 1000;
          break;

	  case 6:
	  case 7:
	    steel = 300;
          break;

	  case 8:
          if(ch->level >= 50)  steel = 300;
          if(ch->level >= 70)  steel = 517;
          if(ch->level >= 90)  steel = 734;
          break;

	  case 9:
          if(ch->level >= 50)  steel = 300;
          if(ch->level >= 60)  steel = 440;
          if(ch->level >= 70)  steel = 580;
          if(ch->level >= 80)  steel = 720;
          if(ch->level >= 90)  steel = 860;
          if(ch->level >= 100) steel = 1000;
          break;

	  case 10:
	    steel = 300;
          break;

	  case 11:
	    if(ch->level >= 50)  steel = 300;
	    if(ch->level >= 52)  steel = 328;
	    if(ch->level >= 54)  steel = 356;
	    if(ch->level >= 56)  steel = 384;
	    if(ch->level >= 58)  steel = 412;
	    if(ch->level >= 60)  steel = 440;
	    if(ch->level >= 62)  steel = 468;
	    if(ch->level >= 64)  steel = 496;
	    if(ch->level >= 66)  steel = 524;
	    if(ch->level >= 68)  steel = 552;
	    if(ch->level >= 70)  steel = 580;
	    if(ch->level >= 72)  steel = 608;
	    if(ch->level >= 74)  steel = 636;
	    if(ch->level >= 76)  steel = 664;
	    if(ch->level >= 78)  steel = 692;
	    if(ch->level >= 80)  steel = 720;
	    if(ch->level >= 82)  steel = 748;
	    if(ch->level >= 84)  steel = 776;
	    if(ch->level >= 86)  steel = 804;
	    if(ch->level >= 88)  steel = 832;
	    if(ch->level >= 90)  steel = 860;
	    if(ch->level >= 92)  steel = 888;
	    if(ch->level >= 94)  steel = 916;
	    if(ch->level >= 96)  steel = 944;
	    if(ch->level >= 98)  steel = 972;
	    if(ch->level >= 100) steel = 1000;
          break;

	  case 12:
	    steel = 0;
          break;

	  case 13:
	    steel = 300;
          break;

	  case 14:
          if(ch->level >= 50)  steel = 300;
          if(ch->level >= 60)  steel = 440;
          if(ch->level >= 70)  steel = 580;
          if(ch->level >= 80)  steel = 720;
          if(ch->level >= 90)  steel = 860;
          if(ch->level >= 100) steel = 1000;
          break;

	  case 15:
	    steel = 300;
          break;

	  case 16:
	    steel = 500;
          break;

	  case 17:
          if(ch->level >= 50)  steel = 300;
          if(ch->level >= 70)  steel = 517;
          if(ch->level >= 90)  steel = 734;
          break;

	  case 18:
	    if(ch->level >= 50)  steel = 300;
	    if(ch->level >= 52)  steel = 328;
	    if(ch->level >= 54)  steel = 356;
	    if(ch->level >= 56)  steel = 384;
	    if(ch->level >= 58)  steel = 412;
	    if(ch->level >= 60)  steel = 440;
	    if(ch->level >= 62)  steel = 468;
	    if(ch->level >= 64)  steel = 496;
	    if(ch->level >= 66)  steel = 524;
	    if(ch->level >= 68)  steel = 552;
	    if(ch->level >= 70)  steel = 580;
	    if(ch->level >= 72)  steel = 608;
	    if(ch->level >= 74)  steel = 636;
	    if(ch->level >= 76)  steel = 664;
	    if(ch->level >= 78)  steel = 692;
	    if(ch->level >= 80)  steel = 720;
	    if(ch->level >= 82)  steel = 748;
	    if(ch->level >= 84)  steel = 776;
	    if(ch->level >= 86)  steel = 804;
	    if(ch->level >= 88)  steel = 832;
	    if(ch->level >= 90)  steel = 860;
	    if(ch->level >= 92)  steel = 888;
	    if(ch->level >= 94)  steel = 916;
	    if(ch->level >= 96)  steel = 944;
	    if(ch->level >= 98)  steel = 972;
	    if(ch->level >= 100) steel = 1000;
          break;

	  case 19:
          if(ch->level >= 50)  steel = 300;
          if(ch->level >= 60)  steel = 440;
          if(ch->level >= 70)  steel = 580;
          if(ch->level >= 80)  steel = 720;
          if(ch->level >= 90)  steel = 860;
          if(ch->level >= 100) steel = 1000;
          break;

    default: //default
      steel = 300;
    break;
  }

  if(ch->steel < steel)
  {
    send_to_char("You do not have enough steel money to craft it with your cultural art.\n\r",ch);
    return;
  }

  wait = fast_working(ch, gsn_cultural_artistry);

  WAIT_STATE(ch, wait);

  for ( d = descriptor_list; d != NULL; d = d->next )
  {
    if ( d->connected == CON_PLAYING
    &&   d->character != ch
    &&   d->character->in_room != NULL
    &&   d->character->in_room != ch->in_room
    &&   d->character->in_room->area == ch->in_room->area
    &&   !IS_SET(d->character->in_room->room_flags, ROOM_QUIET)
    &&   !IS_AFFECTED2(d->character, AFF_DEAFNESS)
    &&   number_percent() < 5 )
    {
      act("The sounds of hammering can be heard in the distance.",ch,argument,d->character,TO_VICT);
    }
  }

  act("You begin to craft $p with your cultural art.",ch,obj,NULL,TO_CHAR);
  act("$n begins to craft $p with $s cultural art.",ch,obj,NULL,TO_ROOM);

  if(number_percent() > ch->pcdata->learned[gsn_cultural_artistry])
  {
	  act("You fail to craft your cultural art.",ch,NULL,NULL,TO_CHAR);
	  act("$n fails to craft $s cultural art.",ch,NULL,NULL,TO_ROOM);
	  ch->steel -= steel;
	  check_improve(ch, gsn_cultural_artistry, FALSE, 1);
	  return;
  }
  else
  {
	  act("You succeed at crafting your cultural art on $p.",ch,obj,NULL,TO_CHAR);
	  act("$n succeeds at crafting $s cultural art on $p.",ch,obj,NULL,TO_ROOM);
	  SET_BIT(obj->extra2_flags, ITEM_ARTISTRY);
	  ch->steel -= steel;
	  check_improve(ch,gsn_cultural_artistry,TRUE,1);
	  if( bs_exp(ch, gsn_cultural_artistry) ) gain_exp(ch, 50);

	//Get Race # for Character's text race
	  race = race_lookup(pc_race_table[ch->race].name);

	  af.where 		= TO_OBJECT;
	  af.type  		= gsn_cultural_artistry;
	  af.level 		= ch->level;
	  af.duration	= -1;
	  af.location	= 0;
	  af.modifier 	= 0;
	  af.bitvector 	= 0;

		switch(race)
		{
			case 0:  //NULL
				Aff = FALSE;
				bugf("Character %s race is NULL.",ch->name);
				break;

			case 1:  //Human
				Aff = FALSE;

				if(ch->level >= 50) mod = 5;
				if(ch->level >= 60) mod = 6;
				if(ch->level >= 70) mod = 7;
				if(ch->level >= 80) mod = 8;
				if(ch->level >= 90) mod = 9;
				if(ch->level >= 100) mod = 10;

				obj->value[AC_SLASH] += mod; //adds to SLASHING AC
				break;

			case 2:  //Draconian
				Aff = FALSE;

				if(ch->level >= 50) mod = 5;
				if(ch->level >= 60) mod = 6;
				if(ch->level >= 70) mod = 7;
				if(ch->level >= 80) mod = 8;
				if(ch->level >= 90) mod = 9;
				if(ch->level >= 100) mod = 10;

				obj->value[AC_EXOTIC] += mod; //adds to EXOTIC AC
				break;

			case 3:  //Half-Elf
				af.where = TO_OBJECT;
				af.location = APPLY_HITROLL;
				af.modifier = 1 + (ch->level >= 50) + (ch->level >= 75);
				break;

			case 4:  //Silvanesti
				af.where = TO_OBJECT;
				af.bitvector = ITEM_ANTI_EVIL;
				break;

			case 5:  //Qualinesti
				af.where = TO_AFFECTS;
				af.location = APPLY_MOVE;

				if(ch->level >= 50) mod = 10;
				if(ch->level >= 52) mod = 11;
				if(ch->level >= 54) mod = 12;
				if(ch->level >= 56) mod = 13;
				if(ch->level >= 58) mod = 14;
				if(ch->level >= 60) mod = 15;
				if(ch->level >= 62) mod = 16;
				if(ch->level >= 64) mod = 17;
				if(ch->level >= 66) mod = 18;
				if(ch->level >= 68) mod = 19;
				if(ch->level >= 70) mod = 20;
				if(ch->level >= 72) mod = 21;
				if(ch->level >= 74) mod = 22;
				if(ch->level >= 76) mod = 23;
				if(ch->level >= 78) mod = 24;
				if(ch->level >= 80) mod = 25;
				if(ch->level >= 82) mod = 26;
				if(ch->level >= 84) mod = 27;
				if(ch->level >= 86) mod = 28;
				if(ch->level >= 88) mod = 29;
				if(ch->level >= 90) mod = 30;
				if(ch->level >= 92) mod = 31;
				if(ch->level >= 94) mod = 32;
				if(ch->level >= 96) mod = 33;
				if(ch->level >= 98) mod = 34;
				if(ch->level >= 100) mod = 35;

				af.modifier = mod;
				break;

			case 6:  //Kagonesti
				af.where = TO_OBJECT;
				af.bitvector = ITEM_INVIS;
				break;

			case 7:  //Dargonesti
				af.where = TO_OBJECT;
				af.bitvector = ITEM_BURN_PROOF;
				break;

			case 8:  //Dimernesti
				af.where = TO_AFFECTS;
				af.location = APPLY_SAVES;
				af.modifier = -1 - (ch->level >= 50) - (ch->level >= 70) - (ch->level >= 90);
				break;

			case 9:  //Neidar
				Aff = FALSE;

				if(ch->level >= 50) mod = 5;
				if(ch->level >= 60) mod = 6;
				if(ch->level >= 70) mod = 7;
				if(ch->level >= 80) mod = 8;
				if(ch->level >= 90) mod = 9;
				if(ch->level >= 100) mod = 10;

				obj->value[AC_PIERCE] += mod; //adds to PIERCING AC
				break;
				
			case 10: //Hylar
				af.where = TO_OBJECT;
				af.bitvector = ITEM_BLESS;
				break;

			case 11: //Theiwar
				af.where = TO_AFFECTS;
				af.location = APPLY_MANA;

				if(ch->level >= 50) mod = 5;
				if(ch->level >= 52) mod = 6;
				if(ch->level >= 54) mod = 7;
				if(ch->level >= 56) mod = 8;
				if(ch->level >= 58) mod = 9;
				if(ch->level >= 60) mod = 10;
				if(ch->level >= 62) mod = 11;
				if(ch->level >= 64) mod = 12;
				if(ch->level >= 66) mod = 13;
				if(ch->level >= 68) mod = 14;
				if(ch->level >= 70) mod = 15;
				if(ch->level >= 72) mod = 16;
				if(ch->level >= 74) mod = 17;
				if(ch->level >= 76) mod = 18;
				if(ch->level >= 78) mod = 19;
				if(ch->level >= 80) mod = 20;
				if(ch->level >= 82) mod = 21;
				if(ch->level >= 84) mod = 22;
				if(ch->level >= 86) mod = 23;
				if(ch->level >= 88) mod = 24;
				if(ch->level >= 90) mod = 25;
				if(ch->level >= 92) mod = 26;
				if(ch->level >= 94) mod = 27;
				if(ch->level >= 96) mod = 28;
				if(ch->level >= 98) mod = 29;
				if(ch->level >= 100) mod = 30;

				af.modifier = mod;
				break;

			case 12: //Aghar
				Aff = FALSE;
				send_to_char("You can't do this...\n\r",ch);
				break;

			case 13: //Daergar
				af.where = TO_OBJECT;
				af.bitvector = ITEM_EVIL;
				break;

			case 14: //Daewar
				if(ch->level >= 50)  mod = -1;
				if(ch->level >= 60)  mod = -2;
				if(ch->level >= 70)  mod = -3;
				if(ch->level >= 80)  mod = -4;
				if(ch->level >= 90)  mod = -5;
				if(ch->level >= 100) mod = -6;

				af.where     = TO_OBJECT;
				af.location  = APPLY_SAVING_SPELL;
                        af.modifier  = mod;
				break;

			case 15: //Klar
				af.where = TO_OBJECT;
				af.bitvector = ITEM_STAINLESS;
				break;

			case 16: //Gnome
				affect_number = number_range(0,14);
				af.where = gnome_affect[affect_number].af_where;
				af.location = gnome_affect[affect_number].location;
				af.modifier = gnome_affect[affect_number].modifier;
				af.bitvector = gnome_affect[affect_number].bit;
				break;

			case 17: //Kender
				af.where = TO_OBJECT;
				af.location = APPLY_DEX;
				af.modifier = 1 + (ch->level > 50) + (ch-> level > 75) + (ch->level > 90);
				break;

			case 18: //Minotaur
				af.where = TO_AFFECTS;
				af.location = APPLY_HIT;
				
				if(ch->level >= 50) mod = 15;
				if(ch->level >= 52) mod = 16;
				if(ch->level >= 54) mod = 17;
				if(ch->level >= 56) mod = 18;
				if(ch->level >= 58) mod = 19;
				if(ch->level >= 60) mod = 20;
				if(ch->level >= 62) mod = 21;
				if(ch->level >= 64) mod = 22;
				if(ch->level >= 66) mod = 23;
				if(ch->level >= 68) mod = 24;
				if(ch->level >= 70) mod = 25;
				if(ch->level >= 72) mod = 26;
				if(ch->level >= 74) mod = 27;
				if(ch->level >= 76) mod = 28;
				if(ch->level >= 78) mod = 29;
				if(ch->level >= 80) mod = 30;
				if(ch->level >= 82) mod = 31;
				if(ch->level >= 84) mod = 32;
				if(ch->level >= 86) mod = 33;
				if(ch->level >= 88) mod = 34;
				if(ch->level >= 90) mod = 35;
				if(ch->level >= 92) mod = 36;
				if(ch->level >= 94) mod = 37;
				if(ch->level >= 96) mod = 38;
				if(ch->level >= 98) mod = 39;
				if(ch->level >= 100) mod = 40;

				af.modifier = mod;
				break;

			case 19: //Ogre
				Aff = FALSE;

				if(ch->level >= 50) mod = 5;
				if(ch->level >= 60) mod = 6;
				if(ch->level >= 70) mod = 7;
				if(ch->level >= 80) mod = 8;
				if(ch->level >= 90) mod = 9;
				if(ch->level >= 100) mod = 10;

				obj->value[AC_BASH] += mod; //adds to BASHING AC
				break;

			default: //New Race?
				Aff = FALSE;
				bugf("Character %s race not recognized.",ch->name);
				break;

		} //end switch

		if(Aff) affect_to_obj(obj, &af);
		return;
	} //end else
	return;
} //end craft

void do_key_origin( CHAR_DATA *ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  OBJ_DATA *obj;
  argument = one_argument(argument,arg);

  if (IS_AFFECTED2 (ch, AFF_FORGET))
  {
    send_to_char("You can't seem to remember anything about the value or worth of objects.\n\r",ch);
    return;
  }

  if (get_skill(ch,gsn_key_origin) == 0
  || ch->level < skill_table[gsn_key_origin].skill_level[ch->class] )
  {
    send_to_char("Huh?.\n\r",ch);
    return;
  }

  if (arg[0] == '\0')
  {
    send_to_char("What key would you like to learn the geographical origin of?\n\r",ch);
    return;
  }

  if ( ( obj = get_obj_carry(ch,arg,ch)) == NULL)
  {
    send_to_char("You are not carrying that item.\n\r",ch);
    return;
  }

  if (obj->item_type != ITEM_KEY)
  {
    send_to_char("You can only determine the geographical origin of keys. Nothing else!\n\r",ch);
    return;
  }

  sprintf(buf,"You carefully examine %s.\n\r",obj->short_descr);
  send_to_char(buf,ch);
  send_to_char("\n\r",ch);

  if ( number_percent( ) < ch->pcdata->learned[gsn_key_origin] ) 
    check_improve(ch,gsn_key_origin,TRUE,1); 
  else
  {
    send_to_char("You fail to learn anything.\n\r",ch);
    check_improve(ch,gsn_key_origin,FALSE,1);
    return;
  }

  sprintf(buf,"It appears to be a key with a geographical origin that would come from %s\n\r", obj->pIndexData->area->name );
  send_to_char(buf,ch);

  return;

}
