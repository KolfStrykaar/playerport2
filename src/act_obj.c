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
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "interp.h"
#include "magic.h"
#include "lang.h"

/*
 * Local functions.
 */
#define CD CHAR_DATA
#define OD OBJ_DATA
bool	remove_obj	args( (CHAR_DATA *ch, int iWear, bool fReplace ) );
void	wear_obj	args( (CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace ) );
CD *	find_keeper	args( (CHAR_DATA *ch ) );
int	get_cost	args( (CHAR_DATA *keeper, OBJ_DATA *obj, bool fBuy ) );
void 	obj_to_keeper	args( (OBJ_DATA *obj, CHAR_DATA *ch ) );
OD *	get_obj_keeper	args( (CHAR_DATA *ch,CHAR_DATA *keeper,char *argument));


#undef OD
#undef	CD

/* RT part of the corpse looting code */

bool can_loot(CHAR_DATA *ch, OBJ_DATA *obj)
{
    CHAR_DATA *owner, *wch;

    if (IS_IMMORTAL(ch))
	return TRUE;

    if (!obj->owner || obj->owner == NULL)
	return TRUE;

    owner = NULL;
    for ( wch = char_list; wch != NULL ; wch = wch->next )
        if (!str_cmp(wch->name,obj->owner))
            owner = wch;

    if (owner == NULL)
	return TRUE;

    if (!str_cmp(ch->name,owner->name))
	return TRUE;

    if (!IS_NPC(owner) && IS_SET(owner->act,PLR_CANLOOT))
	return TRUE;

    if (is_same_group(ch,owner))
	return TRUE;

    return FALSE;
}

/*checks for leather armor*/
bool is_leather(OBJ_DATA *obj)
{
    if (obj->value[4] == 1)
	return TRUE;

    if (is_name("leather",obj->name) ||
        is_name("banded",obj->name) ||
        is_name("hide",obj->name))
        return TRUE;
    else
        return FALSE;
}

bool is_chain(OBJ_DATA *obj)
{
    if (obj->value[4] == 2)
	return TRUE;

    if (is_name("chain",obj->name) ||
        is_name("scale",obj->name) ||
        is_name("chainmail",obj->name) ||
        is_name("scalemail",obj->name) ||
        is_name("ringmail",obj->name) ||
        is_name("ring",obj->name) )
           return TRUE;
        else
           return FALSE;
}

bool is_plate(OBJ_DATA *obj)
{
    if (obj->value[4] == 4)
	return TRUE;

      if (is_name("plate",obj->name) ||
          is_name("plates",obj->name) ||
          is_name("breastplate",obj->name) ||
          is_name("platemail",obj->name) ||
          is_name("leafmail",obj->name) ||
          is_name("fullplate",obj->name) ||
          is_name("brigandine",obj->name))
           return TRUE;
        else
           return FALSE;
}

void get_obj( CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *container )
{
    /* variables for AUTOSPLIT */
    CHAR_DATA *gch;
    int members;
    char buffer[100];

    if ( !CAN_WEAR(obj, ITEM_TAKE) )
    {
	send_to_char( "You can't take that.\n\r", ch );
	return;
    }

    if ( ch->carry_number + get_obj_number( obj ) > can_carry_n( ch ) )
    {
	act( "$d: you can't carry that many items.",
	    ch, NULL, obj->name, TO_CHAR );
	return;
    }

    if ((!obj->in_obj || obj->in_obj->carried_by != ch)
    &&  (get_carry_weight(ch) + get_obj_weight(obj) > can_carry_w(ch)))
    {
	act( "$d: you can't carry that much weight.",
	    ch, NULL, obj->name, TO_CHAR );
	return;
    }

    if (!can_loot(ch,obj))
    {
	act("Corpse looting is not permitted.",ch,NULL,NULL,TO_CHAR );
	return;
    }

    if (obj->in_room != NULL)
    {
	for (gch = obj->in_room->people; gch != NULL; gch = gch->next_in_room)
	    if (gch->on == obj)
	    {
		act("$N appears to be using $p.",
		    ch,obj,gch,TO_CHAR);
		return;
	    }
    }
		

    if ( container != NULL )
    {
    	if ( (container->pIndexData->vnum == OBJ_VNUM_PIT 
		|| container->pIndexData->vnum == hometown_table[ch->hometown].pit_vnum)
	&&  get_trust(ch) < (obj->level - 20))
	{
	    send_to_char("You are not powerful enough to use it.\n\r",ch);
	    return;
	}

    	if ( (container->pIndexData->vnum == OBJ_VNUM_PIT
		|| container->pIndexData->vnum == hometown_table[ch->hometown].pit_vnum)
	&&  !CAN_WEAR(container, ITEM_TAKE)
	&&  !IS_OBJ_STAT(obj,ITEM_HAD_TIMER))
	    obj->timer = 0;
	/* if it's low level eq, clear the cleanup timer */
      clr_low_eq_tmr( obj );
	act( "You get $p from $P.", ch, obj, container, TO_CHAR );
	if ( !IS_AFFECTED(ch,AFF_SNEAK))
	act( "$n gets $p from $P.", ch, obj, container, TO_ROOM );
	REMOVE_BIT(obj->extra_flags,ITEM_HAD_TIMER);
	obj_from_obj( obj );
    }
    else
    {
      /* if it's low level eq, clear the cleanup timer */
      clr_low_eq_tmr( obj );
	act( "You get $p.", ch, obj, container, TO_CHAR );
	if ( !IS_AFFECTED(ch,AFF_SNEAK))
	act( "$n gets $p.", ch, obj, container, TO_ROOM );
	obj_from_room( obj );
    }

    if ( obj->item_type == ITEM_MONEY)
    {
	ch->gold += obj->value[0];
	ch->steel += obj->value[1];
        if (IS_SET(ch->act,PLR_AUTOSPLIT))
        { /* AUTOSPLIT code */
    	  members = 0;
    	  for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    	  {
            if (!IS_AFFECTED(gch,AFF_CHARM) && is_same_group( gch, ch ) )
              members++;
    	  }

	  if ( members > 1 && (obj->value[0] > 1 || obj->value[1]))
	  {
	    sprintf(buffer,"%d %d",obj->value[0],obj->value[1]);
	    do_function(ch, &do_split, buffer);	
	  }
        }
 
	extract_obj( obj );
    }
    else
    {
	obj_to_char( obj, ch );
	if ( HAS_TRIGGER_OBJ( obj, TRIG_GET ) )
	    p_give_trigger( NULL, obj, NULL, ch, obj, TRIG_GET );
	if ( HAS_TRIGGER_ROOM( ch->in_room, TRIG_GET ) )
	    p_give_trigger( NULL, NULL, ch->in_room, ch, obj, TRIG_GET );
    }

    return;
}



void do_get( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    OBJ_DATA *container;
    bool found;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if (!str_cmp(arg2,"from"))
	argument = one_argument(argument,arg2);

    /* Get type. */
    if ( arg1[0] == '\0' )
    {
	send_to_char( "Get what?\n\r", ch );
	return;
    }

    if ( arg2[0] == '\0' )
    {
	if ( str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) )
	{
	    /* 'get obj' */
	    obj = get_obj_list( ch, arg1, ch->in_room->contents );
	    if ( obj == NULL )
	    {
		act( "I see no $T here.", ch, NULL, arg1, TO_CHAR );
		return;
	    }

	    get_obj( ch, obj, NULL );
	}
	else
	{
	    /* 'get all' or 'get all.obj' */
	    found = FALSE;
	    for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next )
	    {
		obj_next = obj->next_content;
		if ( ( arg1[3] == '\0' || is_name( &arg1[4], obj->name ) )
		&&   can_see_obj( ch, obj ) )
		{
		    found = TRUE;
		    get_obj( ch, obj, NULL );
		}
	    }

	    if ( !found ) 
	    {
		if ( arg1[3] == '\0' )
		    send_to_char( "I see nothing here.\n\r", ch );
		else
		    act( "I see no $T here.", ch, NULL, &arg1[4], TO_CHAR );
	    }
	}
    }
    else
    {
	/* 'get ... container' */
	if ( !str_cmp( arg2, "all" ) || !str_prefix( "all.", arg2 ) )
	{
	    send_to_char( "You can't do that.\n\r", ch );
	    return;
	}

	if ( ( container = get_obj_here( ch, NULL, arg2 ) ) == NULL )
	{
	    act( "I see no $T here.", ch, NULL, arg2, TO_CHAR );
	    return;
	}

	switch ( container->item_type )
	{
	default:
	    send_to_char( "That's not a container.\n\r", ch );
	    return;

	case ITEM_CONTAINER:
	case ITEM_CORPSE_NPC:
	    break;

	case ITEM_CORPSE_PC:
	 {
	  /*if (!can_loot(ch,container))
	  {
	   send_to_char( "\n\r{CA godly force freezes your outstretched hand.{x\n\r", ch );
         return;
	  }
        if  (!IS_NPC( ch ) && !IS_IMMORTAL(ch)
	  &&   str_cmp( container->owner, ch->name )
	  &&   str_cmp( container->owner2, ch->name ) )
	  {
	   send_to_char( "You did not inflict the death blow upon this corpse.\n\r", ch );
	   return;
	  }*/
	 }
	}

	if ( IS_SET(container->value[1], CONT_CLOSED) )
	{
	    act( "The $d is closed.", ch, NULL, container->name, TO_CHAR );
	    return;
	}

	if ( str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) )
	{
	    /* 'get obj container' */
	    obj = get_obj_list( ch, arg1, container->contains );
	    if ( obj == NULL )
	    {
		act( "I see nothing like that in the $T.",
		    ch, NULL, arg2, TO_CHAR );
		return;
	    }
	    get_obj( ch, obj, container );
	}
	else
	{
	    /* 'get all container' or 'get all.obj container' */
	    found = FALSE;
	    for ( obj = container->contains; obj != NULL; obj = obj_next )
	    {
		obj_next = obj->next_content;
		if ( ( arg1[3] == '\0' || is_name( &arg1[4], obj->name ) )
		&&   can_see_obj( ch, obj ) )
		{
		    found = TRUE;
		    if ( (container->pIndexData->vnum == OBJ_VNUM_PIT
				|| container->pIndexData->vnum == hometown_table[ch->hometown].pit_vnum)
		    &&  !IS_IMMORTAL(ch))
		    {
			send_to_char("The gods frown upon such wanton greed!\n\r",ch);
			return;
		    }
		    get_obj( ch, obj, container );
		}
	    }

	    if ( !found )
	    {
		if ( arg1[3] == '\0' )
		    act( "I see nothing in the $T.",
			ch, NULL, arg2, TO_CHAR );
		else
		    act( "I see nothing like that in the $T.",
			ch, NULL, arg2, TO_CHAR );
	    }
	}
    }

    return;
}



void do_put( CHAR_DATA *ch, char *argument )
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  OBJ_DATA *container;
  OBJ_DATA *obj;
  OBJ_DATA *obj1;
  OBJ_DATA *obj_next;
  OBJ_DATA *obj1_next;
  int count;

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  if (!str_cmp(arg2,"in") || !str_cmp(arg2,"on"))
    argument = one_argument(argument,arg2);

  if ( arg1[0] == '\0' || arg2[0] == '\0' )
  {
    send_to_char( "Put what in what?\n\r", ch );
    return;
  }

  if ((!str_cmp(arg1,"coin")
  ||  !str_cmp(arg1,"coins")
  ||  !str_cmp(arg1,"gold"))
  && !str_cmp(arg2,"fountain"))
  {

    if ( ( obj = get_obj_here( ch, NULL, arg2 ) ) == NULL )
    {
	send_to_char( "You do not see a fountain here.\n\r", ch );
	return;
    }

    if ( ch->gold <= 0 )
    {
	send_to_char( "Your pockets appear to be empty right now.\n\r", ch );
	return;
    }

    if ( obj->item_type != ITEM_FOUNTAIN )
    {
	send_to_char( "That's not a fountain.\n\r", ch );
	return;
    }

    ch->gold -= 1;
    act("You fling a gold coin into the fountain and make a wish.",ch,obj,NULL,TO_CHAR);
    act("$n flings a gold coin into the fountain and makes a wish.",ch,obj,NULL,TO_ROOM);
    obj_cast_spell(skill_lookup("luck"),10,ch,ch,NULL);
    WAIT_STATE(ch,2*PULSE_VIOLENCE);
    return;
  }

  if ( !str_cmp( arg2, "all" ) || !str_prefix( "all.", arg2 ) )
  {
    send_to_char( "You can't do that.\n\r", ch );
    return;
  }

  if ( ( container = get_obj_here( ch, NULL, arg2 ) ) == NULL )
  {
    act( "I see no $T here.", ch, NULL, arg2, TO_CHAR );
    return;
  }

  if ( container->item_type != ITEM_CONTAINER )
  {
    send_to_char( "That's not a container.\n\r", ch );
    return;
  }

  if ( IS_SET(container->value[1], CONT_CLOSED) )
  {
    act( "The $d is closed.", ch, NULL, container->name, TO_CHAR );
    return;
  }

  if ( str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) )
  {
	/* 'put obj container' */
    if ( ( obj = get_obj_carry( ch, arg1, ch ) ) == NULL )
    {
      send_to_char( "You do not have that item.\n\r", ch );
      return;
    }

    if ( obj == container )
    {
      send_to_char( "You can't fold it into itself.\n\r", ch );
	return;
    }

    if ( !can_drop_obj( ch, obj ) )
    {
	send_to_char( "You can't let go of it.\n\r", ch );
	return;
    }

    if (WEIGHT_MULT(obj) != 100)
    {
      send_to_char("You have a feeling that would be a bad idea.\n\r",ch);
      return;
    }

    if (get_obj_weight( obj ) + get_true_weight( container ) > (container->value[0] * 10) 
    ||  get_obj_weight(obj) > (container->value[3] * 10))
    {
	send_to_char( "It won't fit.\n\r", ch );
	return;
    }
	
    if (( container->pIndexData->vnum == OBJ_VNUM_PIT 
    ||    container->pIndexData->vnum == hometown_table[ch->hometown].pit_vnum)
    &&   !CAN_WEAR(container,ITEM_TAKE))
    {
	if (obj->timer)
	  SET_BIT(obj->extra_flags,ITEM_HAD_TIMER);
	else
	  obj->timer = number_range(100,200);
    }

    count = 0;
    if (( container->pIndexData->vnum == OBJ_VNUM_PIT 
    ||    container->pIndexData->vnum == hometown_table[ch->hometown].pit_vnum))
    {
      for ( obj1 = container->contains; obj1 != NULL; obj1 = obj1_next )
      {
        obj1_next = obj1->next_content;
        if(obj1->pIndexData->vnum == obj->pIndexData->vnum)
          count++;

        if ( count > 9 )
        {
          send_to_char( "You can't put any more of those items into the pit!\n\r", ch );
          return;
        }
      }
    }

    obj_from_char( obj );
    obj_to_obj( obj, container );

    if (IS_SET(container->value[1],CONT_PUT_ON))
    {
	act("$n puts $p on $P.",ch,obj,container, TO_ROOM);
	act("You put $p on $P.",ch,obj,container, TO_CHAR);
    }
    else
    {
	act( "$n puts $p in $P.", ch, obj, container, TO_ROOM );
	act( "You put $p in $P.", ch, obj, container, TO_CHAR );
    }
  }
  else
  {
    /* 'put all container' or 'put all.obj container' */
    for ( obj = ch->carrying; obj != NULL; obj = obj_next )
    {
	obj_next = obj->next_content;

	if ( ( arg1[3] == '\0' || is_name( &arg1[4], obj->name ) )
	&&   can_see_obj( ch, obj )
	&&   WEIGHT_MULT(obj) == 100
	&&   obj->wear_loc == WEAR_NONE
	&&   obj != container
	&&   can_drop_obj( ch, obj )
	&&   get_obj_weight( obj ) + get_true_weight( container )
		 <= (container->value[0] * 10) 
	&&   get_obj_weight(obj) <= (container->value[3] * 10))
	{
	  if (( container->pIndexData->vnum == OBJ_VNUM_PIT
	  ||    container->pIndexData->vnum == hometown_table[ch->hometown].pit_vnum)
	  &&   !CAN_WEAR(obj, ITEM_TAKE) )
        {
	    if (obj->timer)
	      SET_BIT(obj->extra_flags,ITEM_HAD_TIMER);
	    else
            obj->timer = number_range(100,200);
        }
	  obj_from_char( obj );
        obj_to_obj( obj, container );

        if (IS_SET(container->value[1],CONT_PUT_ON))
        {
          act("$n puts $p on $P.",ch,obj,container, TO_ROOM);
          act("You put $p on $P.",ch,obj,container, TO_CHAR);
        }
	  else
	  {
          act( "$n puts $p in $P.", ch, obj, container, TO_ROOM );
	    act( "You put $p in $P.", ch, obj, container, TO_CHAR );
	  }
	}
    }
  }
  return;
}

void do_drop( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *blood;
    OBJ_DATA *obj_next;
    bool found;
    char buf[MAX_STRING_LENGTH];

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Drop what?\n\r", ch );
	return;
    }

    if ( is_number( arg ) )
    {
	/* 'drop NNNN coins' */
	int amount, steel = 0, gold = 0;

	amount   = atoi(arg);
	argument = one_argument( argument, arg );
	if ( amount <= 0
	|| ( str_cmp( arg, "coins" ) && str_cmp( arg, "coin" ) && 
	     str_cmp( arg, "steel"  ) && str_cmp( arg, "gold") ) )
	{
	    send_to_char( "Sorry, you can't do that.\n\r", ch );
	    return;
	}

	if ( !str_cmp( arg, "coins") || !str_cmp(arg,"coin") 
	||   !str_cmp( arg, "gold"))
	{
	    if (ch->gold < amount)
	    {
		send_to_char("You don't have that much gold.\n\r",ch);
		return;
	    }

	    ch->gold -= amount;
	    gold = amount;
	}

	else
	{
	    if (ch->steel < amount)
	    {
		send_to_char("You don't have that much steel.\n\r",ch);
		return;
	    }

	    ch->steel -= amount;
  	    steel = amount;
	}

	for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;

	    switch ( obj->pIndexData->vnum )
	    {
	    case OBJ_VNUM_GOLD_ONE:
		gold += 1;
		extract_obj(obj);
		break;

	    case OBJ_VNUM_STEEL_ONE:
		steel += 1;
		extract_obj( obj );
		break;

	    case OBJ_VNUM_GOLD_SOME:
		gold += obj->value[0];
		extract_obj(obj);
		break;

	    case OBJ_VNUM_STEEL_SOME:
		steel += obj->value[1];
		extract_obj( obj );
		break;

	    case OBJ_VNUM_COINS:
		gold += obj->value[0];
		steel += obj->value[1];
		extract_obj(obj);
		break;
	    }
	}

	obj_to_room( create_money( steel, gold ), ch->in_room );
	act( "$n drops some coins.", ch, NULL, NULL, TO_ROOM );
	send_to_char( "OK.\n\r", ch );
	return;
    }

    if ( str_cmp( arg, "all" ) && str_prefix( "all.", arg ) )
    {
	/* 'drop obj' */
	if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL )
	{
	    send_to_char( "You do not have that item.\n\r", ch );
	    return;
	}

	if ( !can_drop_obj( ch, obj ) )
	{
	    send_to_char( "You can't let go of it.\n\r", ch );
	    return;
	}

	obj_from_char( obj );

      if (( ch->in_room->sector_type == SECT_WATER_SWIM )
      ||  ( ch->in_room->sector_type == SECT_WATER_NOSWIM ))
      {
          act( "$p vanishes into the water.", ch, obj, NULL, TO_ROOM );
	  act( "$p vanishes into the water.", ch, obj, NULL, TO_CHAR );
	  extract_obj( obj );
          return; // exit out of the program when extracting object, else triggers below will cause a crash
      }
      else
      {
        /* see if low-level eq cleanup timer needs to be set */
        set_low_eq_tmr( obj );
        obj_to_room( obj, ch->in_room );
      }

      if (( obj->pIndexData->vnum == OBJ_VNUM_JAR )
      && (( ch->in_room->sector_type != SECT_WATER_SWIM )
      ||  ( ch->in_room->sector_type != SECT_WATER_NOSWIM ))
      &&  ( number_percent() < 51 ) )
      {
        send_to_char( "The jar of blood shatters on the ground, creating a messy pool of blood.\n\r", ch );
	  extract_obj( obj );
        blood =  create_object( get_obj_index(OBJ_VNUM_BLOOD),ch->level);
        obj_to_room( blood, ch->in_room );
        sprintf(buf,"There's a large messy pool of blood here.");
        free_string( blood->description );
        blood->description = str_dup( buf );
        sprintf(buf,"a messy pool of blood");
        free_string( blood->short_descr );
        blood->short_descr = str_dup( buf );
        sprintf(buf,"messy pool blood");
        free_string( blood->name );
        blood->name = str_dup( buf );
        blood->timer = 2;
        return;
      }

      if ( obj->item_type == ITEM_TRAP )
      {
        switch( ch->in_room->sector_type )
        {
          case SECT_FOREST:
          case SECT_HILLS:
    	    case SECT_MOUNTAIN:
   	    case SECT_SWAMP:
          if (!IS_NPC(ch) && number_percent( ) < ch->pcdata->learned[gsn_set_traps])
          {
            check_improve(ch,gsn_set_traps,TRUE,3);
	      REMOVE_BIT(obj->wear_flags, ITEM_TAKE);
            act("$n sets $p on the ground and skillfully arms it.",
              ch, obj, NULL, TO_ROOM);
            act("You set $p on the ground and skillfully arm it.",
              ch, obj, NULL, TO_CHAR);
            return;
          }
          else
          {
            extract_obj(obj);
            check_improve(ch,gsn_set_traps,FALSE,3);
            act("$n botches an attempt to arm $p and destroys it.",
              ch, obj, NULL, TO_ROOM);
            act("You botch an attempt to arm $p and destroy it.",
              ch, obj, NULL, TO_CHAR);
            return;
          }
          default:
            act("You don't think you could properly set $p in such surroundings.",
              ch, obj, NULL, TO_CHAR);
            get_obj(ch, obj, NULL);
          return;
        }
      }

      if ( obj->item_type == ITEM_BOOBYTRAP )
      {
        switch( ch->in_room->sector_type )
        {
          case SECT_CITY:
          case SECT_INSIDE:
          if (!IS_NPC(ch)
          && number_percent( ) < ch->pcdata->learned[gsn_boobytrap])
          {
            check_improve(ch,gsn_boobytrap,TRUE,3);
	      REMOVE_BIT(obj->wear_flags, ITEM_TAKE);
            act("$n sets $p on the ground and activates it.",
              ch, obj, NULL, TO_ROOM);
            act("You set $p on the ground and activate it.",
              ch, obj, NULL, TO_CHAR);
	      obj->timer = 20;
	      obj->level = ch->level;
            return;
          }
          else
          {
            extract_obj(obj);
            check_improve(ch,gsn_boobytrap,FALSE,3);
            act("$n botches an attempt to boobytrap the area with $p and destroys it.",
              ch, obj, NULL, TO_ROOM);
            act("You botch your attempt to boobytrap the area with $p and destroy it.",
              ch, obj, NULL, TO_CHAR);
            return;
          }
          default:
            act("This is not a confined enough area for you to properly boobytrap it.",
              ch, NULL, NULL, TO_CHAR);
            get_obj(ch, obj, NULL);
            return;
        }
      }

	act( "$n releases $p from $s grasp.", ch, obj, NULL, TO_ROOM );
	act( "You release $p from your grasp.", ch, obj, NULL, TO_CHAR );

      if (( ch->in_room->sector_type != SECT_WATER_SWIM )
	&&  ( ch->in_room->sector_type != SECT_WATER_NOSWIM ))
      {
	 act( "$p hits the ground.", ch, obj, NULL, TO_ROOM );
	 act( "$p hits the ground.", ch, obj, NULL, TO_CHAR );
      }
      else
      {
      }

	if ( HAS_TRIGGER_OBJ( obj, TRIG_DROP ) )
	    p_give_trigger( NULL, obj, NULL, ch, obj, TRIG_DROP );
	if ( HAS_TRIGGER_ROOM( ch->in_room, TRIG_DROP ) )
	    p_give_trigger( NULL, NULL, ch->in_room, ch, obj, TRIG_DROP );

	if (IS_OBJ_STAT(obj,ITEM_MELT_DROP))
	{
	    act("$p dissolves into smoke.",ch,obj,NULL,TO_ROOM);
	    act("$p dissolves into smoke.",ch,obj,NULL,TO_CHAR);
	    extract_obj(obj);
	}
    }
    else
    {
	/* 'drop all' or 'drop all.obj' */
	found = FALSE;
	for ( obj = ch->carrying; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;

	    if ( ( arg[3] == '\0' || is_name( &arg[4], obj->name ) )
	    &&   can_see_obj( ch, obj )
	    &&   obj->wear_loc == WEAR_NONE
	    &&   can_drop_obj( ch, obj ) )
	    {
		found = TRUE;
		obj_from_char( obj );
            /* see if low-level eq cleanup timer needs to be set */
            set_low_eq_tmr( obj );
		obj_to_room( obj, ch->in_room );
	      act( "$n releases $p from $s grasp.", ch, obj, NULL, TO_ROOM );
	      act( "You release $p from your grasp.", ch, obj, NULL, TO_CHAR );
	if ( HAS_TRIGGER_OBJ( obj, TRIG_DROP ) )
	    p_give_trigger( NULL, obj, NULL, ch, obj, TRIG_DROP );
	if ( HAS_TRIGGER_ROOM( ch->in_room, TRIG_DROP ) )
	    p_give_trigger( NULL, NULL, ch->in_room, ch, obj, TRIG_DROP );
	    
	if ( obj && IS_OBJ_STAT(obj,ITEM_MELT_DROP))
	     {
	     act("$p dissolves into smoke.",ch,obj,NULL,TO_ROOM);
	     act("$p dissolves into smoke.",ch,obj,NULL,TO_CHAR);
	     extract_obj(obj);
	     }
	    }
	}

	if ( !found )
	{
	    if ( arg[3] == '\0' )
		act( "You are not carrying anything.",
		    ch, NULL, arg, TO_CHAR );
	    else
		act( "You are not carrying any $T.",
		    ch, NULL, &arg[4], TO_CHAR );
	}
    }

    return;
}



void do_give( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA  *obj;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Give what to whom?\n\r", ch );
	return;
    }

    if ( is_number( arg1 ) )
    {
	/* 'give NNNN coins victim' */
	int amount;
	bool gold;

	amount   = atoi(arg1);
	if ( amount <= 0
	|| ( str_cmp( arg2, "coins" ) && str_cmp( arg2, "coin" ) && 
	     str_cmp( arg2, "steel"  ) && str_cmp( arg2, "gold")) )
	{
	    send_to_char( "Sorry, you can't do that.\n\r", ch );
	    return;
	}

	gold = str_cmp(arg2,"steel");

	argument = one_argument( argument, arg2 );
	if ( arg2[0] == '\0' )
	{
	    send_to_char( "Give what to whom?\n\r", ch );
	    return;
	}

	if ( ( victim = get_char_room( ch, NULL, arg2 ) ) == NULL )
	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
	}

	if ( (!gold && ch->steel < amount) || (gold && ch->gold < amount) )
	{
	    send_to_char( "You haven't got that much.\n\r", ch );
	    return;
	}

	if (!IS_SET(victim->act,ACT_IS_CHANGER))
      {
	  if (gold)
	  {
	    ch->gold		-= amount;
	    victim->gold  	+= amount;
	  }
	  else
	  {
	    ch->steel		-= amount;
	    victim->steel 	+= amount;
	  }

	  sprintf(buf,"$n gives you %d %s.",amount, gold ? "gold" : "steel");
	  act( buf, ch, NULL, victim, TO_VICT    );
	  act( "$n gives $N some coins.",  ch, NULL, victim, TO_NOTVICT );
	  sprintf(buf,"You give $N %d %s.",amount, gold ? "gold" : "steel");
	  act( buf, ch, NULL, victim, TO_CHAR    );
	}

	/*
	 * Bribe trigger
	 */
	if ( IS_NPC(victim) && HAS_TRIGGER_MOB( victim, TRIG_BRIBE ) )
	    p_bribe_trigger( victim, ch, gold ? amount : amount * 100 );

	if (IS_NPC(victim) && IS_SET(victim->act,ACT_IS_CHANGER))
	{
	    int change;

          if (!can_see(victim,ch))
          {
		act("$n says, 'I'm sorry, is someone there? Show yourself! I don't do business with folk I can't see!!'", victim,NULL,ch,TO_VICT);
            return;
          }

	    if (is_affected(ch,gsn_doppelganger))
	    {
		act("$n says, 'Hrm... I don't know.  You look a little suspect!  If your disguised, you can forget about doing business here!!'", victim,NULL,ch,TO_VICT);
            return;
	    }

	    if (gold)
	    {
	      ch->gold		-= amount;
	      victim->gold  	+= amount;
	    }
	    else
	    {
	      ch->steel		-= amount;
	      victim->steel 	+= amount;
	    }

	    sprintf(buf,"$n gives you %d %s.",amount, gold ? "gold" : "steel");
	    act( buf, ch, NULL, victim, TO_VICT    );
	    act( "$n gives $N some coins.",  ch, NULL, victim, TO_NOTVICT );
	    sprintf(buf,"You give $N %d %s.",amount, gold ? "gold" : "steel");
	    act( buf, ch, NULL, victim, TO_CHAR    );

	    change = (gold ? 95 * amount / 100 / 100 
		 	     : 95 * amount);

	    if (!gold && change > victim->gold)
	    	victim->gold += change;

	    if (gold && change > victim->steel)
		victim->steel += change;

	    if (change < 1 && can_see(victim,ch))
	    {
		act( "$n tells you 'I'm sorry, you did not give me enough to change.'",
            victim,NULL,ch,TO_VICT);
		ch->reply = victim;
		sprintf(buf,"%d %s %s",amount, gold ? "gold" : "steel",ch->name);
		do_function(victim, &do_give, buf);
	    }
	    else if (can_see(victim,ch))
	    {
		sprintf(buf,"%d %s %s", change, gold ? "steel" : "gold",ch->name);
		do_function(victim, &do_give, buf);
		if (gold)
		{
		    sprintf(buf,"%d gold %s", (95 * amount / 100 - change * 100),ch->name);
		    do_function(victim, &do_give, buf);
		}
		act("$n tells you 'Thank you, come again.'", victim,NULL,ch,TO_VICT);
		ch->reply = victim;
	    }
	}
	return;
    }

    if ( ( obj = get_obj_carry( ch, arg1, ch ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }

    if ( obj->wear_loc != WEAR_NONE )
    {
	send_to_char( "You must remove it first.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, NULL, arg2 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if (IS_NPC(victim) && victim->pIndexData->pShop != NULL)
    {
	act("$N tells you 'Sorry, you'll have to sell that.'", ch,NULL,victim,TO_CHAR);
	ch->reply = victim;
	return;
    }

    if ( !can_drop_obj( ch, obj ) )
    {
	send_to_char( "You can't let go of it.\n\r", ch );
	return;
    }

    if ( victim->carry_number + get_obj_number( obj ) > can_carry_n( victim ) )
    {
      act( "$n tried to give you $p, but your carrying too many items.", ch,obj,victim,TO_VICT);
	act( "$N has $S hands full.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if (get_carry_weight(victim) + get_obj_weight(obj) > can_carry_w( victim ) )
    {
      act( "$n tried to give you $p, but you are carrying too much weight!",ch,obj,victim,TO_VICT);
	act( "$N can't carry that much weight.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( !can_see_obj( victim, obj ) )
    {
	act( "$N can't see it.", ch, NULL, victim, TO_CHAR );
	return;
    }

    obj_from_char( obj );
    obj_to_char( obj, victim );
    MOBtrigger = FALSE;
    act( "$n gives $p to $N.", ch, obj, victim, TO_NOTVICT );
    act( "$n gives you $p.",   ch, obj, victim, TO_VICT    );
    act( "You give $p to $N.", ch, obj, victim, TO_CHAR    );
    MOBtrigger = TRUE;
    if ( HAS_TRIGGER_OBJ( obj, TRIG_GIVE ) )
	p_give_trigger( NULL, obj, NULL, ch, obj, TRIG_GIVE );
    if ( HAS_TRIGGER_ROOM( ch->in_room, TRIG_GIVE ) )
	p_give_trigger( NULL, NULL, ch->in_room, ch, obj, TRIG_GIVE );
    /*
     * Give trigger
     */
    if ( IS_NPC(victim) && HAS_TRIGGER_MOB( victim, TRIG_GIVE ) )
	p_give_trigger( victim, NULL, NULL, ch, obj, TRIG_GIVE );

    return;
}


/* for poisoning weapons and food/drink */
void do_envenom(CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;
    AFFECT_DATA af;
    int percent,skill;

    /* find out what */
    if (argument[0] == '\0')
    {
	send_to_char("Envenom what item?\n\r",ch);
	return;
    }

    obj =  get_obj_list(ch,argument,ch->carrying);

    if (obj== NULL)
    {
	send_to_char("You don't have that item.\n\r",ch);
	return;
    }

    if ((skill = get_skill(ch,gsn_envenom)) < 1)
    {
	send_to_char("Are you crazy? You'd poison yourself!\n\r",ch);
	return;
    }

    if (obj->item_type == ITEM_FOOD || obj->item_type == ITEM_DRINK_CON)
    {
	if (IS_OBJ_STAT(obj,ITEM_BLESS) || IS_OBJ_STAT(obj,ITEM_BURN_PROOF))
	{
	    act("You fail to poison $p.",ch,obj,NULL,TO_CHAR);
	    return;
	}

	if (number_percent() < skill)  /* success! */
	{
	    act("$n treats $p with deadly poison.",ch,obj,NULL,TO_ROOM);
	    act("You treat $p with deadly poison.",ch,obj,NULL,TO_CHAR);
	    if (!obj->value[3])
	    {
		obj->value[3] = 1;
		check_improve(ch,gsn_envenom,TRUE,4);
	    }
	    WAIT_STATE(ch,skill_table[gsn_envenom].beats);
	    return;
	}

	act("You fail to poison $p.",ch,obj,NULL,TO_CHAR);
	if (!obj->value[3])
	    check_improve(ch,gsn_envenom,FALSE,4);
	WAIT_STATE(ch,skill_table[gsn_envenom].beats);
	return;
     }

    if (obj->item_type == ITEM_WEAPON)
    {
        if (IS_WEAPON_STAT(obj,WEAPON_FLAMING)
        ||  IS_WEAPON_STAT(obj,WEAPON_FROST)
        ||  IS_WEAPON_STAT(obj,WEAPON_VAMPIRIC)
        ||  IS_WEAPON_STAT(obj,WEAPON_SHARP)
        ||  IS_WEAPON_STAT(obj,WEAPON_VORPAL)
        ||  IS_WEAPON_STAT(obj,WEAPON_SHOCKING)
        ||  IS_WEAPON_STAT(obj,WEAPON_BRILLIANT)
        ||  IS_WEAPON_STAT(obj,WEAPON_CHAOTIC)
        ||  IS_WEAPON_STAT(obj,WEAPON_DISRUPTION)
        ||  IS_WEAPON_STAT(obj,WEAPON_FLAMING_BURST)
        ||  IS_WEAPON_STAT(obj,WEAPON_SHOCKING_BURST)
        ||  IS_WEAPON_STAT(obj,WEAPON_ICY_BURST)
        ||  IS_WEAPON_STAT(obj,WEAPON_HOLY)
        ||  IS_WEAPON_STAT(obj,WEAPON_UNHOLY)
        ||  IS_WEAPON_STAT(obj,WEAPON_LAWFUL)
        ||  IS_WEAPON_STAT(obj,WEAPON_MIGHTY_CLEAVING)
        ||  IS_WEAPON_STAT(obj,WEAPON_THUNDERING)
	  ||  IS_WEAPON_STAT(obj,WEAPON_BSTAFF)
	  ||  IS_WEAPON_STAT(obj,WEAPON_DSTAFF)
	  ||  IS_OBJ_STAT(obj,ITEM_DEATHBLADE)
        ||  IS_OBJ_STAT(obj,ITEM_BLESS) 
	  || IS_OBJ_STAT(obj,ITEM_BURN_PROOF))
        {
            act("You can't seem to envenom $p.",ch,obj,NULL,TO_CHAR);
            return;
        }

	if (obj->value[3] < 0 
	||  attack_table[obj->value[3]].damage == DAM_BASH)
	{
	    send_to_char("You can only envenom edged weapons.\n\r",ch);
	    return;
	}

        if (IS_WEAPON_STAT(obj,WEAPON_POISON))
        {
            act("$p is already envenomed.",ch,obj,NULL,TO_CHAR);
            return;
        }

	percent = number_percent();
	if (percent < skill)
	{
 
            af.where     = TO_WEAPON;
            af.type      = gsn_poison;
            af.level     = ch->level * percent / 100;
            af.duration  = ch->level/2 * percent / 100;
            af.location  = 0;
            af.modifier  = 0;
            af.bitvector = WEAPON_POISON;
            affect_to_obj(obj,&af);
 
            act("$n coats $p with deadly venom.",ch,obj,NULL,TO_ROOM);
	    act("You coat $p with venom.",ch,obj,NULL,TO_CHAR);
	    check_improve(ch,gsn_envenom,TRUE,3);
	    WAIT_STATE(ch,skill_table[gsn_envenom].beats);
            return;
        }
	else
	{
	    act("You fail to envenom $p.",ch,obj,NULL,TO_CHAR);
	    check_improve(ch,gsn_envenom,FALSE,3);
	    WAIT_STATE(ch,skill_table[gsn_envenom].beats);
	    return;
	}
    }
 
    act("You can't poison $p.",ch,obj,NULL,TO_CHAR);
    return;
}

void do_fill( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *fountain;
    bool found;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Fill what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }

    found = FALSE;
    for ( fountain = ch->in_room->contents; fountain != NULL;
	fountain = fountain->next_content )
    {
	if ( fountain->item_type == ITEM_FOUNTAIN )
	{
	    found = TRUE;
	    break;
	}
    }

    if ( !found )
    {
	send_to_char( "There is no fountain here!\n\r", ch );
	return;
    }

    if ( obj->item_type != ITEM_DRINK_CON )
    {
	send_to_char( "You can't fill that.\n\r", ch );
	return;
    }

    if ( obj->value[1] != 0 && obj->value[2] != fountain->value[2] )
    {
	send_to_char( "There is already another liquid in it.\n\r", ch );
	return;
    }

    if ( obj->value[1] >= obj->value[0] )
    {
	send_to_char( "Your container is full.\n\r", ch );
	return;
    }

    if ((fountain->value[1] <= 0)
    &&  (fountain->value[1] != -1))
    {
	sprintf(buf, "There isn't enough %s left to fill $p from $P.",
      liq_table[fountain->value[2]].liq_name);
      act( buf, ch, obj,fountain, TO_CHAR );
	return;
    }

    sprintf(buf, "You fill $p with %s from $P.",
	liq_table[fountain->value[2]].liq_name);
    act( buf, ch, obj,fountain, TO_CHAR );
    sprintf(buf,"$n fills $p with %s from $P.",
	liq_table[fountain->value[2]].liq_name);
    act(buf,ch,obj,fountain,TO_ROOM);
    obj->value[2] = fountain->value[2];
    obj->value[1] = obj->value[0];

    if (fountain->value[0] > 0)
        fountain->value[1] -= 1;

    return;
}

void do_pour (CHAR_DATA *ch, char *argument)
{
    char arg[MAX_STRING_LENGTH],buf[MAX_STRING_LENGTH];
    OBJ_DATA *out, *in;
    CHAR_DATA *vch = NULL;
    int amount;

    argument = one_argument(argument,arg);
    
    if (arg[0] == '\0' || argument[0] == '\0')
    {
	send_to_char("Pour what into what?\n\r",ch);
	return;
    }
    

    if ((out = get_obj_carry(ch,arg, ch)) == NULL)
    {
	send_to_char("You don't have that item.\n\r",ch);
	return;
    }

    if (out->item_type != ITEM_DRINK_CON)
    {
	send_to_char("That's not a drink container.\n\r",ch);
	return;
    }

    if (!str_cmp(argument,"out"))
    {
	if (out->value[1] == 0)
	{
	    send_to_char("It's already empty.\n\r",ch);
	    return;
	}

	out->value[1] = 0;
	out->value[3] = 0;
	sprintf(buf,"You invert $p, spilling %s all over the ground.",
		liq_table[out->value[2]].liq_name);
	act(buf,ch,out,NULL,TO_CHAR);
	
	sprintf(buf,"$n inverts $p, spilling %s all over the ground.",
		liq_table[out->value[2]].liq_name);
	act(buf,ch,out,NULL,TO_ROOM);
	return;
    }

    if ((in = get_obj_here(ch, NULL, argument)) == NULL)
    {
	vch = get_char_room(ch, NULL, argument);

	if (vch == NULL)
	{
	    send_to_char("Pour into what?\n\r",ch);
	    return;
	}

	in = get_eq_char(vch,WEAR_HOLD);

	if (in == NULL)
	{
	    send_to_char("They aren't holding anything.",ch);
 	    return;
	}
    }

    if (in->item_type != ITEM_DRINK_CON)
    {
	send_to_char("You can only pour into other drink containers.\n\r",ch);
	return;
    }
    
    if (in == out)
    {
	send_to_char("You cannot change the laws of physics!\n\r",ch);
	return;
    }

    if (in->value[1] != 0 && in->value[2] != out->value[2])
    {
	send_to_char("They don't hold the same liquid.\n\r",ch);
	return;
    }

    if (out->value[1] == 0)
    {
	act("There's nothing in $p to pour.",ch,out,NULL,TO_CHAR);
	return;
    }

    if (in->value[1] >= in->value[0])
    {
	act("$p is already filled to the top.",ch,in,NULL,TO_CHAR);
	return;
    }

    amount = UMIN(out->value[1],in->value[0] - in->value[1]);

    in->value[1] += amount;
    out->value[1] -= amount;
    in->value[2] = out->value[2];
    
    if (vch == NULL)
    {
    	sprintf(buf,"You pour %s from $p into $P.",
	    liq_table[out->value[2]].liq_name);
    	act(buf,ch,out,in,TO_CHAR);
    	sprintf(buf,"$n pours %s from $p into $P.",
	    liq_table[out->value[2]].liq_name);
    	act(buf,ch,out,in,TO_ROOM);
    }
    else
    {
        sprintf(buf,"You pour some %s for $N.",
            liq_table[out->value[2]].liq_name);
        act(buf,ch,NULL,vch,TO_CHAR);
	sprintf(buf,"$n pours you some %s.",
	    liq_table[out->value[2]].liq_name);
	act(buf,ch,NULL,vch,TO_VICT);
        sprintf(buf,"$n pours some %s for $N.",
            liq_table[out->value[2]].liq_name);
        act(buf,ch,NULL,vch,TO_NOTVICT);
	
    }
}

void do_drink( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  OBJ_DATA *obj;
  int amount;
  int liquid;

  one_argument( argument, arg );

  if ( arg[0] == '\0' )
  {
    for ( obj = ch->in_room->contents; obj; obj = obj->next_content )
    {
	if ( obj->item_type == ITEM_FOUNTAIN )
      break;
    }

    if ( obj == NULL )
    {
	send_to_char( "Drink what?\n\r", ch );
	return;
    }
  }
  else
  {
    if ( ( obj = get_obj_here( ch, NULL, arg ) ) == NULL )
    {
	send_to_char( "You can't find it.\n\r", ch );
	return;
    }
  }

  if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10 )
  {
    send_to_char( "You fail to reach your mouth.  *Hic*\n\r", ch );
    return;
  }

    switch ( obj->item_type )
    {
    default:
	send_to_char( "You can't drink from that.\n\r", ch );
	return;

    case ITEM_FOUNTAIN:
      if ( ( liquid = obj->value[2] )  < 0 )
      {
        bug( "Do_drink: bad liquid number %d.", liquid );
        liquid = obj->value[2] = 0;
      }

	if ((obj->value[1] <= 0)
      &&  (obj->value[1] != -1))
	{
	  sprintf(buf, "There isn't enough %s left to drink from $P.",
        liq_table[obj->value[2]].liq_name);
        act( buf, ch, NULL,obj, TO_CHAR );
        return;
	}

	amount = liq_table[liquid].liq_affect[4] * 3;
	break;

    case ITEM_DRINK_CON:
	if ( obj->value[1] <= 0 )
	{
	    send_to_char( "It is already empty.\n\r", ch );
	    return;
	}

      if (IS_UNDEAD(ch)
	&&  obj->value[2] != 13 )
	{
	  send_to_char( "You may only quench your thirst with {rBlood{x.\n\r", ch );
	  return;
	}

	if ( ( liquid = obj->value[2] )  < 0 )
	{
	    bug( "Do_drink: bad liquid number %d.", liquid );
	    liquid = obj->value[2] = 0;
	}

        amount = liq_table[liquid].liq_affect[4];
        amount = UMIN(amount, obj->value[1]);
	break;
     }
    if (!IS_NPC(ch) && !IS_IMMORTAL(ch) 
    &&  ch->pcdata->condition[COND_THIRST] > 64)
    {
	send_to_char("You couldn't possibly drink another drop.\n\r",ch);
	return;
    }

    act( "$n drinks $T from $p.",
	ch, obj, liq_table[liquid].liq_name, TO_ROOM );
    act( "You drink $T from $p.",
	ch, obj, liq_table[liquid].liq_name, TO_CHAR );

    gain_condition( ch, COND_DRUNK,
	amount * liq_table[liquid].liq_affect[COND_DRUNK] / 36 );
    if IS_UNDEAD(ch)
    {
      gain_condition( ch, COND_THIRST, +5 );
    }
    else
    {
      gain_condition( ch, COND_THIRST,
	  amount * liq_table[liquid].liq_affect[COND_THIRST] / 10 );
    }

    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK]  > 10 )
	send_to_char( "You feel drunk.\n\r", ch );
    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_THIRST] > 40 )
	send_to_char( "Your thirst is quenched.\n\r", ch );
	
    if ( obj->value[3] != 0 )
    {
	/* The drink was poisoned ! */
	AFFECT_DATA af;

	act( "$n chokes and gags.", ch, NULL, NULL, TO_ROOM );
	send_to_char( "You choke and gag.\n\r", ch );
	af.where     = TO_AFFECTS;
	af.type      = gsn_poison;
	af.level	 = number_fuzzy(amount); 
	af.duration  = 3 * amount;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = AFF_POISON;
	affect_join( ch, &af );
    }
	
    if (obj->value[0] > 0)
        obj->value[1] -= amount;

    return;
}



void do_eat( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if ( ch->fighting )
    {
	send_to_char( "Perhaps you should wait until AFTER the fight.\n\r",ch);
	return;
    }

    if ( arg[0] == '\0' )
    {
	send_to_char( "Eat what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }

    if ( !IS_IMMORTAL(ch) )
    {
	if ( obj->item_type != ITEM_FOOD && obj->item_type != ITEM_PILL )
	{
	    send_to_char( "That's not edible.\n\r", ch );
	    return;
	}

	if ( !IS_NPC(ch) 
	&& (ch->pcdata->condition[COND_FULL] > 64)
	&& (obj->item_type != ITEM_PILL) )
	{   
	    send_to_char( "You are stuffed to the gills and couldn't possibly take another bite!\n\r", ch );
	    return;
	}
    }

    act( "$n eats $p.",  ch, obj, NULL, TO_ROOM );
    act( "You eat $p.", ch, obj, NULL, TO_CHAR );

    switch ( obj->item_type )
    {

    case ITEM_FOOD:
	if ( !IS_NPC(ch) )
	{
	    int condition;

	    condition = ch->pcdata->condition[COND_FULL];
	    gain_condition( ch, COND_FULL, obj->value[0] );
	    if ( ch->pcdata->condition[COND_FULL] < 0 )
		send_to_char( "You are no longer hungry.\n\r", ch );
	    else if ( ch->pcdata->condition[COND_FULL] > 20 )
		send_to_char( "You are getting full.\n\r", ch );
	}

      if (is_name("juicy",obj->name)
      ||  is_name("juice",obj->name))
	{
	    int condition;

	    condition = ch->pcdata->condition[COND_THIRST];
	    gain_condition( ch, COND_THIRST, 1 );
	    sprintf(buf, "The juices from %s also quench your thirst a little.\n\r",
          obj->short_descr);
	    send_to_char( buf, ch );
	}

	if ( obj->value[3] != 0 )
	{
	    /* The food was poisoned! */
	    AFFECT_DATA af;

	    act( "$n chokes and gags.", ch, 0, 0, TO_ROOM );
	    send_to_char( "You choke and gag.\n\r", ch );

	    af.where	 = TO_AFFECTS;
	    af.type      = gsn_poison;
	    af.level 	 = number_fuzzy(obj->value[0]);
	    af.duration  = 2 * obj->value[0];
	    af.location  = APPLY_NONE;
	    af.modifier  = 0;
	    af.bitvector = AFF_POISON;
	    affect_join( ch, &af );
	}
	break;

    case ITEM_PILL:
	obj_cast_spell( obj->value[1], obj->value[0], ch, ch, NULL );
	obj_cast_spell( obj->value[2], obj->value[0], ch, ch, NULL );
	obj_cast_spell( obj->value[3], obj->value[0], ch, ch, NULL );
	break;
    }

	if ( HAS_TRIGGER_OBJ( obj, TRIG_EAT ) )
	p_percent_trigger( NULL, obj, NULL, ch, NULL, NULL, TRIG_EAT );

    extract_obj( obj );
    return;
}



/*
 * Remove an object.
 */
bool remove_obj( CHAR_DATA *ch, int iWear, bool fReplace )
{
    OBJ_DATA *obj;

    if ( ( obj = get_eq_char( ch, iWear ) ) == NULL )
	return TRUE;

    if ( !fReplace )
	return FALSE;

    if ( IS_SET(obj->extra_flags, ITEM_NOREMOVE) )
    {
	act( "You can't remove $p.", ch, obj, NULL, TO_CHAR );
	return FALSE;
    }

    if ( (obj->item_type == ITEM_WEAPON) && IS_OBJ_STAT(obj,ITEM_DEATH) )
    {
        act( "The black mist around $p fades away.",ch,obj,NULL,TO_ALL);
        REMOVE_BIT(obj->extra_flags,ITEM_DEATH);
    }

/* -Removed for Stainless and Blacksmith skills 
    if(IS_SET(obj->extra_flags, ITEM_DEATHBLADE 
    && !IS_OBJ_STAT(obj, ITEM_HAD_TIMER)))
    {
    obj->timer = 0;
    REMOVE_BIT(obj->extra_flags,ITEM_HAD_TIMER);
    }
*/

    unequip_char( ch, obj );
    act( "$n stops using $p.", ch, obj, NULL, TO_ROOM );
    act( "You stop using $p.", ch, obj, NULL, TO_CHAR );
    return TRUE;
}



/*
 * Wear one object.
 * Optional replacement of existing objects.
 * Big repetitive code, ick.
 */
void wear_obj( CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace )
{
    char buf[MAX_STRING_LENGTH];
    int skill = 0;


    skill = 101;
    if ( ch->level < obj->level - 20 )
    {
	sprintf( buf, "You must be level %d to use this object.\n\r",
	    obj->level - 20 );
	send_to_char( buf, ch );
	act( "$n tries to use $p, but is too inexperienced.",
	    ch, obj, NULL, TO_ROOM );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_TATTOO ) )
    {
        if ( !remove_obj( ch, WEAR_TATTOO, fReplace ) )
            return;
        act( "$n tattoos $p on $s body.",   ch, obj, NULL, TO_ROOM );
        act( "You tattoo $p on your body.", ch, obj, NULL, TO_CHAR );
        equip_char( ch, obj, WEAR_TATTOO );
        return;
    }

    if ( obj->item_type == ITEM_LIGHT )
    {

      if ( (obj->size > (ch->size +1 ))
      && (obj->size != SIZE_IGNORE) 
      && !IS_IMMORTAL(ch))
	{
	  send_to_char("That is much to large for you to handle.\n\r", ch);
	  return;
	}

	if ( !remove_obj( ch, WEAR_LIGHT, fReplace ) )
	    return;
      if (obj->value[2] != 0)
      {
	  act( "$n lights $p and holds it.", ch, obj, NULL, TO_ROOM );
	  act( "You light $p and hold it.",  ch, obj, NULL, TO_CHAR );
      }
	else
	{
	  act( "$n holds $p, but it appears to be burned out.", ch, obj, NULL, TO_ROOM );
        act( "You hold and try to light $p, but it won't light anymore.",ch,obj,NULL,TO_CHAR );
	}
	equip_char( ch, obj, WEAR_LIGHT );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_FINGER ) )
    {

        /*Characters can only wear rings their own size*/
      if ((obj->size != ch->size)
      && (obj->size != SIZE_IGNORE) 
      && !IS_IMMORTAL(ch))  
	{
	  if( obj->size > ch->size )
	    send_to_char("It falls right off your finger.\n\r", ch);
	  else
	    send_to_char("It won't even fit on your pinky!\n\r", ch);
	  return;
	}

	if ( get_eq_char( ch, WEAR_FINGER_L ) != NULL
	&&   get_eq_char( ch, WEAR_FINGER_R ) != NULL
	&&   !remove_obj( ch, WEAR_FINGER_L, fReplace )
	&&   !remove_obj( ch, WEAR_FINGER_R, fReplace ) )
	    return;

	if ( get_eq_char( ch, WEAR_FINGER_L ) == NULL )
	{
	    act( "$n wears $p on $s left finger.",    ch, obj, NULL, TO_ROOM );
	    act( "You wear $p on your left finger.",  ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_FINGER_L );
	    return;
	}

	if ( get_eq_char( ch, WEAR_FINGER_R ) == NULL )
	{
	    act( "$n wears $p on $s right finger.",   ch, obj, NULL, TO_ROOM );
	    act( "You wear $p on your right finger.", ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_FINGER_R );
	    return;
	}

	bug( "Wear_obj: no free finger.", 0 );
	send_to_char( "You already wear two rings.\n\r", ch );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_NECK ) )
    {

        /*Characters can only wear neckwear their size and one size larger*/
      if (((obj->size > (ch->size+1))
      || (obj->size < ch->size))
      && (obj->size != SIZE_IGNORE)
      && !IS_IMMORTAL(ch))  
	{
	  if( obj->size > ch->size )
	    send_to_char("It falls right off.\n\r", ch);
	  else
	    send_to_char("It won't fit around your neck.\n\r", ch);
	  return;
	}

	if ( get_eq_char( ch, WEAR_NECK_1 ) != NULL
	&&   get_eq_char( ch, WEAR_NECK_2 ) != NULL
	&&   !remove_obj( ch, WEAR_NECK_1, fReplace )
	&&   !remove_obj( ch, WEAR_NECK_2, fReplace ) )
	    return;

	if ( get_eq_char( ch, WEAR_NECK_1 ) == NULL )
	{
	    act( "$n wears $p around $s neck.",   ch, obj, NULL, TO_ROOM );
	    act( "You wear $p around your neck.", ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_NECK_1 );
	    return;
	}

	if ( get_eq_char( ch, WEAR_NECK_2 ) == NULL )
	{
	    act( "$n wears $p around $s neck.",   ch, obj, NULL, TO_ROOM );
	    act( "You wear $p around your neck.", ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_NECK_2 );
	    return;
	}

	bug( "Wear_obj: no free neck.", 0 );
	send_to_char( "You already wear two neck items.\n\r", ch );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_BODY ) )
    {

        /*Characters can only wear bodyware their own size*/
    if ((obj->size != ch->size)
    && (obj->size != SIZE_IGNORE) 
    && !IS_IMMORTAL(ch))  
    {
	if( obj->size > ch->size )
	  send_to_char("It is way too big.\n\r", ch);
	else
	  send_to_char("It is way too small.\n\r", ch);
	return;
    }

	if ( !remove_obj( ch, WEAR_BODY, fReplace ) )
	    return;
    
    if (is_leather(obj))
	skill = get_skill(ch,gsn_wear_leather);

    if (is_chain(obj))
	skill = get_skill(ch,gsn_wear_chain);
    
    if (is_plate(obj))
	skill = get_skill(ch,gsn_wear_plate);
    
      if (skill < 101)
      {
          if ( skill < 1 )
          {
            act ( "You cannot move at all in $p and immediately take it off.",ch,obj,NULL,TO_CHAR );
    	    return;
          }
          if ( (skill > -1) && (skill < 25) )
            act ( "You can hardly move your body in $p.",ch,obj,NULL,TO_CHAR );

          if ( (skill > 24) && (skill < 50) )
            act ( "You feel very uncomfortable wearing $p.",ch,obj,NULL,TO_CHAR );

          if ( (skill > 49) && (skill < 75) )
            act ( "You are slightly hindered in $p.",ch,obj,NULL,TO_CHAR );

          if ( (skill > 74) && (skill < 100) )
            act ( "You feel relatively at ease wearing $p.",ch,obj,NULL,TO_CHAR );

          if ( skill == 100 )
            act ( "You can move about effortlessly in $p.",ch,obj,NULL,TO_CHAR );
      }

	act( "$n wears $p on $s torso.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p on your torso.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_BODY );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_HEAD ) )
    {

        /*Minotaurs cannot wear headwear unless they have the ignore size*/
      if ( (obj->size != SIZE_IGNORE ) && ch->race == race_lookup("minotaur"))
      {
        send_to_char("You cannot fit that object over your horns.\n\r",ch);
        return;
      }

        /*Characters can only wear headwear their own size*/
      if ((obj->size != ch->size)
      && (obj->size != SIZE_IGNORE) 
      && !IS_IMMORTAL(ch))  
	{
	  if( obj->size > ch->size )
	    send_to_char("It is way too large.\n\r", ch);
	  else
	    send_to_char("It is way too small.\n\r", ch);
	  return;
	}

	if ( ch->class == class_lookup("barbarian")
	&& ((is_plate(obj))
	|| (is_chain(obj))))
	{
	    sprintf(buf, "You know you could never move properly wearing %s.\n\r", obj->short_descr);
	    send_to_char(buf, ch);
	    return;
	}
	if ( !remove_obj( ch, WEAR_HEAD, fReplace ) )
	    return;
	act( "$n wears $p on $s head.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p on your head.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_HEAD );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_FACE ) )
    {

        /*Characters can only wear face gear their own size*/
      if ( (obj->size != ch->size)
      && (obj->size != SIZE_IGNORE) 
      && !IS_IMMORTAL(ch))  
	{
	  if( obj->size > ch->size )
	    send_to_char("It is way too large.\n\r", ch);
	  else
	    send_to_char("It is way too small.\n\r", ch);
	  return;
	}

	if ( !remove_obj( ch, WEAR_FACE, fReplace ) )
	    return;

	act( "$n wears $p on $s face.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p on your face.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_FACE );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_LEGS ) )
    {

        /*Characters can only wear legware their own size*/
      if ((obj->size != ch->size)
      && (obj->size != SIZE_IGNORE) 
      && !IS_IMMORTAL(ch))  
	{
	  if( obj->size > ch->size )
	    send_to_char("It is way too large.\n\r", ch);
	  else
	    send_to_char("It is way too small.\n\r", ch);
	  return;
	}

    if ( !remove_obj( ch, WEAR_LEGS, fReplace ) )
	return;

    if (is_leather(obj))
	skill = get_skill(ch,gsn_wear_leather);

    if (is_chain(obj))
	skill = get_skill(ch,gsn_wear_chain);
    
    if (is_plate(obj))
	skill = get_skill(ch,gsn_wear_plate);
    
      if (skill < 101)
      {
          if ( skill < 1 )
          {
            act ( "You cannot move at all in $p and immediately take it off.",ch,obj,NULL,TO_CHAR );
    	    return;
          }
          if ( (skill > -1) && (skill < 25) )
            act ( "You can hardly move your body in $p.",ch,obj,NULL,TO_CHAR );

          if ( (skill > 24) && (skill < 50) )
            act ( "You feel very uncomfortable wearing $p.",ch,obj,NULL,TO_CHAR );

          if ( (skill > 49) && (skill < 75) )
            act ( "You are slightly hindered in $p.",ch,obj,NULL,TO_CHAR );

          if ( (skill > 74) && (skill < 100) )
            act ( "You feel relatively at ease wearing $p.",ch,obj,NULL,TO_CHAR );

          if ( skill == 100 )
            act ( "You can move about effortlessly in $p.",ch,obj,NULL,TO_CHAR );
      }

	act( "$n wears $p on $s legs.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p on your legs.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_LEGS );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_FEET ) )
    {

        /*Characters can only wear footware their own size*/
    if ((obj->size != ch->size)
    && (obj->size != SIZE_IGNORE) 
    && !IS_IMMORTAL(ch))  
    {
	  if( obj->size > ch->size )
	    send_to_char("They are way too big.\n\r", ch);
	  else
	    send_to_char("You could never squeeze your feet into those!\n\r", ch);
	  return;
    }

    if ( !remove_obj( ch, WEAR_FEET, fReplace ) )
	    return;

    if (is_chain(obj))
	skill = get_skill(ch,gsn_wear_chain);
    
    if (is_plate(obj))
	skill = get_skill(ch,gsn_wear_plate);
    
      if (skill < 101)
      {
          if ( skill < 1 )
          {
            act ( "You cannot move at all in $p and immediately take it off.",ch,obj,NULL,TO_CHAR );
    	    return;
          }
          if ( (skill > -1) && (skill < 25) )
            act ( "You can hardly move your body in $p.",ch,obj,NULL,TO_CHAR );

          if ( (skill > 24) && (skill < 50) )
            act ( "You feel very uncomfortable wearing $p.",ch,obj,NULL,TO_CHAR );

          if ( (skill > 49) && (skill < 75) )
            act ( "You are slightly hindered in $p.",ch,obj,NULL,TO_CHAR );

          if ( (skill > 74) && (skill < 100) )
            act ( "You feel relatively at ease wearing $p.",ch,obj,NULL,TO_CHAR );

          if ( skill == 100 )
            act ( "You can move about effortlessly in $p.",ch,obj,NULL,TO_CHAR );
      }
	act( "$n wears $p on $s feet.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p on your feet.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_FEET );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_HANDS ) )
    {

        /*Characters can only wear handware their own size*/
      if ((obj->size != ch->size)
      && (obj->size != SIZE_IGNORE) 
      && !IS_IMMORTAL(ch))  
	{
	  if( obj->size > ch->size )
	    send_to_char("They fall right off your hands.\n\r", ch);
	  else
	    send_to_char("You could never squeeze your hands into those.\n\r", ch);
	  return;
	}

	if ( ch->class == class_lookup("barbarian")
	&& ((is_plate(obj))
	|| (is_chain(obj))))
	{
	    sprintf(buf, "You know you could never move properly wearing %s.\n\r", obj->short_descr);
	    send_to_char(buf, ch);
	    return;
	}
	if ( !remove_obj( ch, WEAR_HANDS, fReplace ) )
	    return;
    
    if (is_leather(obj))
	skill = get_skill(ch,gsn_wear_leather);

    if (is_chain(obj))
	skill = get_skill(ch,gsn_wear_chain);
    
    if (is_plate(obj))
	skill = get_skill(ch,gsn_wear_plate);
    
      if (skill < 101)
      {
          if ( skill < 1 )
          {
            act ( "You cannot move at all in $p and immediately take it off.",ch,obj,NULL,TO_CHAR );
    	    return;
          }
          if ( (skill > -1) && (skill < 25) )
            act ( "You can hardly move your body in $p.",ch,obj,NULL,TO_CHAR );

          if ( (skill > 24) && (skill < 50) )
            act ( "You feel very uncomfortable wearing $p.",ch,obj,NULL,TO_CHAR );

          if ( (skill > 49) && (skill < 75) )
            act ( "You are slightly hindered in $p.",ch,obj,NULL,TO_CHAR );

          if ( (skill > 74) && (skill < 100) )
            act ( "You feel relatively at ease wearing $p.",ch,obj,NULL,TO_CHAR );

          if ( skill == 100 )
            act ( "You can move about effortlessly in $p.",ch,obj,NULL,TO_CHAR );
      }

	act( "$n wears $p on $s hands.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p on your hands.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_HANDS );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_ARMS ) )
    {

        /*Characters can only wear armware their own size*/
      if ((obj->size != ch->size)
      && (obj->size != SIZE_IGNORE) 
      && !IS_IMMORTAL(ch))  
      {
	  if( obj->size > ch->size )
	    send_to_char("It is way too large.\n\r", ch);
	  else
	    send_to_char("It is way too small.\n\r", ch);
	  return;
      }


      if ( !remove_obj( ch, WEAR_ARMS, fReplace ) )
	  return;
    
      if (is_leather(obj))
	  skill = get_skill(ch,gsn_wear_leather);

      if (is_chain(obj))
	  skill = get_skill(ch,gsn_wear_chain);
    
      if (is_plate(obj))
	  skill = get_skill(ch,gsn_wear_plate);
    
      if (skill < 101)
      {
          if ( skill < 1 )
          {
            act ( "You cannot move at all in $p and immediately take it off.",ch,obj,NULL,TO_CHAR );
    	    return;
          }
          if ( (skill > -1) && (skill < 25) )
            act ( "You can hardly move your body in $p.",ch,obj,NULL,TO_CHAR );

          if ( (skill > 24) && (skill < 50) )
            act ( "You feel very uncomfortable wearing $p.",ch,obj,NULL,TO_CHAR );

          if ( (skill > 49) && (skill < 75) )
            act ( "You are slightly hindered in $p.",ch,obj,NULL,TO_CHAR );

          if ( (skill > 74) && (skill < 100) )
            act ( "You feel relatively at ease wearing $p.",ch,obj,NULL,TO_CHAR );

          if ( skill == 100 )
            act ( "You can move about effortlessly in $p.",ch,obj,NULL,TO_CHAR );
      }

	act( "$n wears $p on $s arms.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p on your arms.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_ARMS );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_SHOULDERS ) )
    {

        /*Characters can only wear shoulderware their own size*/
      if ((obj->size != ch->size)
      && (obj->size != SIZE_IGNORE) 
      && !IS_IMMORTAL(ch))  
	{
	  if( obj->size > ch->size )
	    send_to_char("It is way too large.\n\r", ch);
	  else
	    send_to_char("It is way too small.\n\r", ch);
	  return;
	}

	if ( !remove_obj( ch, WEAR_SHOULDERS, fReplace ) )
	    return;
    
      if (is_leather(obj))
	  skill = get_skill(ch,gsn_wear_leather);

      if (is_chain(obj))
	  skill = get_skill(ch,gsn_wear_chain);
    
      if (is_plate(obj))
	  skill = get_skill(ch,gsn_wear_plate);
    
      if (skill < 101)
      {
          if ( skill < 1 )
          {
            act ( "You cannot move at all in $p and immediately take it off.",ch,obj,NULL,TO_CHAR );
    	    return;
          }
          if ( (skill > -1) && (skill < 25) )
            act ( "You can hardly move your body in $p.",ch,obj,NULL,TO_CHAR );

          if ( (skill > 24) && (skill < 50) )
            act ( "You feel very uncomfortable wearing $p.",ch,obj,NULL,TO_CHAR );

          if ( (skill > 49) && (skill < 75) )
            act ( "You are slightly hindered in $p.",ch,obj,NULL,TO_CHAR );

          if ( (skill > 74) && (skill < 100) )
            act ( "You feel relatively at ease wearing $p.",ch,obj,NULL,TO_CHAR );

          if ( skill == 100 )
            act ( "You can move about effortlessly in $p.",ch,obj,NULL,TO_CHAR );
      }

	act( "$n wears $p on $s shoulders.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p on your shoulders.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_SHOULDERS );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_ABOUT ) )
    {

        /*Characters can only wear cloaks, etc. one size above or below */
      if (((obj->size > (ch->size+1))
      || (obj->size < (ch->size-1)))
      && (obj->size != SIZE_IGNORE)
      && !IS_IMMORTAL(ch))  
	{
	  if( obj->size > ch->size )
	    send_to_char("It is way too large.\n\r", ch);
	  else
	    send_to_char("It is way too small.\n\r", ch);
	  return;
	}

	if ( !remove_obj( ch, WEAR_ABOUT, fReplace ) )
	    return;

	act( "$n wears $p about $s torso.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p about your torso.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_ABOUT );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_WAIST ) )
    {

        /*Characters can only wear belts their size and one size larger */
      if (((obj->size > (ch->size+1))
      || (obj->size < ch->size))
      && (obj->size != SIZE_IGNORE)
      && !IS_IMMORTAL(ch))   
	{
	  if( obj->size > ch->size )
	    send_to_char("You cannot tighten it enough.\n\r", ch);
	  else
	    send_to_char("It won't fit over your waist.\n\r", ch);
	  return;
	}

	if ( !remove_obj( ch, WEAR_WAIST, fReplace ) )
	    return;
	act( "$n wears $p about $s waist.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p about your waist.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_WAIST );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_WRIST ) )
    {

        /*Characters can only wear wrist ware their size and one size larger*/
      if ( ((obj->size > (ch->size+1))
      || (obj->size < ch->size)) 
      && (obj->size != SIZE_IGNORE)
      && !IS_IMMORTAL(ch))   
	{
	  if( obj->size > ch->size )
	    send_to_char("It falls right off.\n\r", ch);
	  else
	    send_to_char("It won't fit over your wrist.\n\r", ch);
	  return;
	}

	if ( get_eq_char( ch, WEAR_WRIST_L ) != NULL
	&&   get_eq_char( ch, WEAR_WRIST_R ) != NULL
	&&   !remove_obj( ch, WEAR_WRIST_L, fReplace )
	&&   !remove_obj( ch, WEAR_WRIST_R, fReplace ) )
	    return;

	if ( get_eq_char( ch, WEAR_WRIST_L ) == NULL )
	{
	    act( "$n wears $p around $s left wrist.",
		ch, obj, NULL, TO_ROOM );
	    act( "You wear $p around your left wrist.",
		ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_WRIST_L );
	    return;
	}

	if ( get_eq_char( ch, WEAR_WRIST_R ) == NULL )
	{
	    act( "$n wears $p around $s right wrist.",
		ch, obj, NULL, TO_ROOM );
	    act( "You wear $p around your right wrist.",
		ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_WRIST_R );
	    return;
	}

	bug( "Wear_obj: no free wrist.", 0 );
	send_to_char( "Your already wearing two wrist items.\n\r", ch );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_ANKLE ) )
    {

        /*Characters can only wear ankle ware their size and one size larger*/
      if ( ((obj->size > (ch->size+1))
      || (obj->size < ch->size)) 
      && (obj->size != SIZE_IGNORE)
      && !IS_IMMORTAL(ch))   
	{
	  if( obj->size > ch->size )
	    send_to_char("It falls right off.\n\r", ch);
	  else
	    send_to_char("It won't fit over your ankle.\n\r", ch);
	  return;
	}

	if ( get_eq_char( ch, WEAR_ANKLE_L ) != NULL
	&&   get_eq_char( ch, WEAR_ANKLE_R ) != NULL
	&&   !remove_obj( ch, WEAR_ANKLE_L, fReplace )
	&&   !remove_obj( ch, WEAR_ANKLE_R, fReplace ) )
	    return;

	if ( get_eq_char( ch, WEAR_ANKLE_L ) == NULL )
	{
	    act( "$n wears $p around $s left ankle.",
		ch, obj, NULL, TO_ROOM );
	    act( "You wear $p around your left ankle.",
		ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_ANKLE_L );
	    return;
	}

	if ( get_eq_char( ch, WEAR_ANKLE_R ) == NULL )
	{
	    act( "$n wears $p around $s right ankle.",
		ch, obj, NULL, TO_ROOM );
	    act( "You wear $p around your right ankle.",
		ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_ANKLE_R );
	    return;
	}

	bug( "Wear_obj: no free ankle.", 0 );
	send_to_char( "Your already wearing two ankle items.\n\r", ch );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_SHIELD ) )
    {
	OBJ_DATA *weapon;

        /*Characters can only use shields etc one step away */
      if ( ( (obj->size > (ch->size+1))
      || (obj->size < (ch->size-1))) 
      && (obj->size != SIZE_IGNORE)
      && !IS_IMMORTAL(ch))  
	{
	  if( obj->size > ch->size )
	    send_to_char("It is way too large, it would just get in your way.\n\r", ch);
	  else
	    send_to_char("It doesn't cover anything!\n\r", ch);
	  return;
	}

      if (get_eq_char (ch, WEAR_SECONDARY) != NULL)
      {
          send_to_char ("You cannot use a shield while using 2 weapons.\n\r",ch);
          return;
      }

	if ( !remove_obj( ch, WEAR_SHIELD, fReplace ) )
	    return;

	weapon = get_eq_char(ch,WEAR_WIELD);
	if (weapon != NULL && ch->size < SIZE_LARGE 
	&&  IS_WEAPON_STAT(weapon,WEAPON_TWO_HANDS))
	{
	    send_to_char("Your hands are tied up with your weapon!\n\r",ch);
	    return;
	}

	act( "$n wears $p as a shield.", ch, obj, NULL, TO_ROOM );
	act( "You wear $p as a shield.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_SHIELD );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WIELD ) )
    {
	int sn;
	OBJ_DATA *hold;

        /*Characters can only wield one step away */
      if ( ((obj->size > (ch->size+1))
      || (obj->size < (ch->size-1))) 
      && (obj->size != SIZE_IGNORE)
      && !IS_IMMORTAL(ch))  
	{
	  if( obj->size > ch->size )
	    send_to_char("It is way too large for you to properly wield.\n\r", ch);
	  else
	    send_to_char("It is way too small for you to properly wield.\n\r", ch);
	  return;
	}

	if ( !remove_obj( ch, WEAR_WIELD, fReplace ) )
	    return;

	if ( !IS_NPC(ch) 
	&& get_obj_weight(obj) > (str_app[get_curr_stat(ch,STAT_STR)].wield  
		* 10))
	{
	    send_to_char( "It is too heavy for you to wield.\n\r", ch );
	    return;
	}

      if ( (get_eq_char( ch, WEAR_SECONDARY ) ) != NULL )
      {
          send_to_char( "Remove your secondary weapon first.\n\r", ch );
          return;
      }

      hold = get_eq_char(ch,WEAR_HOLD);
	if (hold != NULL && ch->size < SIZE_LARGE 
	&&  IS_WEAPON_STAT(obj,WEAPON_TWO_HANDS))
	{
	  send_to_char("You need two hands free for that weapon.\n\r",ch);
	  return;
	}

	if (!IS_NPC(ch) && ch->size < SIZE_LARGE 
	&&  IS_WEAPON_STAT(obj,WEAPON_TWO_HANDS)
 	&&  get_eq_char(ch,WEAR_SHIELD) != NULL)
	{
	    send_to_char("You need two hands free for that weapon.\n\r",ch);
	    return;
	}
      if (IS_WEAPON_STAT(obj,WEAPON_BSTAFF)
	&&  (ch->class != class_lookup("battlemage")))
      {
        act("You are not in tune enough with the mysteries of battle magic to wear that.", ch, obj, NULL, TO_CHAR);
        act("Emitting a flash of red light, your staff disappears.", ch, NULL, NULL,TO_CHAR);
        act("Emitting a flash of red light, $n's staff disappears.", ch, NULL, NULL,TO_ROOM);
        extract_obj(obj);
        return;
      }

      if (IS_WEAPON_STAT(obj,WEAPON_DSTAFF)
	&&  (ch->class != class_lookup("druid")))
      {
        act("You are not in tune enough with the mysteries of nature to wear that.",
         ch, obj, NULL, TO_CHAR);
        act("Emitting a flash of green light, your druid staff disappears.", ch, NULL, NULL, TO_CHAR);
        act("Emitting a flash of green light, $n's druid staff disappears.", ch, NULL, NULL, TO_ROOM);
        extract_obj(obj);
        return;
      }

	act( "$n wields $p.", ch, obj, NULL, TO_ROOM );
	act( "You wield $p.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_WIELD );

        sn = get_weapon_sn(ch);

	if (sn == gsn_hand_to_hand)
	   return;

        skill = get_weapon_skill(ch,sn);
 
        if (skill >= 100)
            act("$p feels like a part of you!",ch,obj,NULL,TO_CHAR);
        else if (skill > 85)
            act("You feel quite confident with $p.",ch,obj,NULL,TO_CHAR);
        else if (skill > 70)
            act("You are skilled with $p.",ch,obj,NULL,TO_CHAR);
        else if (skill > 50)
            act("Your skill with $p is adequate.",ch,obj,NULL,TO_CHAR);
        else if (skill > 25)
            act("$p feels a little clumsy in your hands.",ch,obj,NULL,TO_CHAR);
        else if (skill > 1)
            act("You fumble and almost drop $p.",ch,obj,NULL,TO_CHAR);
        else
            act("You don't even know which end is up on $p.",
                ch,obj,NULL,TO_CHAR);

	return;
    }

    if ( CAN_WEAR( obj, ITEM_HOLD ) )
    {
	  OBJ_DATA *weapon;

        if (get_eq_char (ch, WEAR_SECONDARY) != NULL)
        {
            send_to_char ("You cannot hold an item while using 2 weapons.\n\r",ch);
            return;
        }

	  weapon = get_eq_char(ch,WEAR_WIELD);
	  if (weapon != NULL && ch->size < SIZE_LARGE 
	  &&  IS_WEAPON_STAT(weapon,WEAPON_TWO_HANDS))
	  {
	    send_to_char("Your hands are tied up with your weapon!\n\r",ch);
	    return;
	  }

        if ( !remove_obj( ch, WEAR_HOLD, fReplace ) )
            return;
        act( "$n holds $p in $s hands.",   ch, obj, NULL, TO_ROOM );
        act( "You hold $p in your hands.", ch, obj, NULL, TO_CHAR );
        equip_char( ch, obj, WEAR_HOLD );
        return;
    }

    if ( CAN_WEAR(obj,ITEM_WEAR_FLOAT) )
    {
	if (!remove_obj(ch,WEAR_FLOAT, fReplace) )
	    return;
	act("$n releases $p to float next to $m.",ch,obj,NULL,TO_ROOM);
	act("You release $p and it floats next to you.",ch,obj,NULL,TO_CHAR);
	equip_char(ch,obj,WEAR_FLOAT);
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_MARK ) )
    {
	if ( !remove_obj( ch, WEAR_MARK, fReplace ) )
	    return;

	act( "$n marks $mself with $p.",   ch, obj, NULL, TO_ROOM );
	act( "You mark yourself with $p.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_MARK );
	return;
    }

    if ( fReplace )
	send_to_char( "You can't wear, wield, or hold that.\n\r", ch );

    return;
}



void do_wear( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int Light_Num = 0;
    int iWear; 
    bool found = FALSE;

    if (IS_AFFECTED2(ch, AFF_PARALYSIS) )
    {
        send_to_char("You have been paralized! You couldn't even bat an eyelash!\n\r",ch);
        return;
    }

    if (IS_AFFECTED3(ch, AFF_HOLDPERSON) )
    {
        send_to_char("You can not move at all!\n\r",ch);
        return;
    }

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Wear, wield, or hold what?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	OBJ_DATA *obj_next;

      send_to_char( "{YYou equip yourself with all possible items at hand.{x\n\r", ch );
      act("{Y$n equips $mself with all possible items at hand.{x", ch, NULL, NULL, TO_ROOM);
	for ( obj = ch->carrying; obj != NULL; obj = obj_next )
	{
	  obj_next = obj->next_content;
	  if ( obj->wear_loc == WEAR_NONE && can_see_obj( ch, obj ) )
	    wear_obj( ch, obj, FALSE );

   	  if (IS_AFFECTED(ch,AFF_HIDE))
   	  {
          Light_Num = 0;
          for ( iWear = 0; (iWear < MAX_WEAR && !found); iWear ++)
          {
            if ((IS_OBJ_STAT(obj,ITEM_GLOW))
            && (obj->wear_loc != WEAR_MARK))
              Light_Num++;

            if ((Light_Num >= 3
            || (obj->item_type == ITEM_LIGHT))
            && IS_AFFECTED(ch,AFF_HIDE))
            {
              act("You are wearing too much light to hide and end up revealing yourself to everyone around you.",ch,NULL,NULL,TO_CHAR);
              act("$n steps out of the shadows.",ch,NULL,NULL,TO_ROOM);
              REMOVE_BIT( ch->affected_by, AFF_HIDE ); 
            }
          }
        }
	}
	return;
    }
    else
    {
	if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL )
	{
	    send_to_char( "You do not have that item.\n\r", ch );
	    return;
	}

	wear_obj( ch, obj, TRUE );
    }

    if (IS_AFFECTED(ch,AFF_HIDE))
    {
      for ( iWear = 0; (iWear < MAX_WEAR && !found); iWear ++)
      {
        if ((obj = get_eq_char(ch,iWear)) == NULL)       
          continue; 
   
        if (IS_OBJ_STAT(obj,ITEM_GLOW)    
        && (obj->wear_loc != WEAR_MARK)) 
          Light_Num++;
      
        if ((Light_Num >= 3)   
        || (obj->item_type == ITEM_LIGHT))  
        { 
            act("You are wearing too much light to hide and end up revealing yourself to everyone around you.",ch,NULL,NULL,TO_CHAR);
            act("$n steps out of the shadows.",ch,NULL,NULL,TO_ROOM);
            REMOVE_BIT( ch->affected_by, AFF_HIDE ); 
            return;
	  }
      }
    }
    return;
}

void do_remove( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int iWear;
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Remove what?\n\r", ch );
        return;
    }

    if ( !strcmp(arg,"all") ) 
    {
      send_to_char( "{YYou remove all possible equipment.{x\n\r", ch );
      act("{Y$n removes all possible equipment.{x", ch, NULL, NULL, TO_ROOM);
        for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
            remove_obj( ch, iWear, TRUE );
        return;
    }
        
    if ( ( obj = get_obj_wear( ch, arg, TRUE ) ) == NULL )
    {
        send_to_char( "You do not have that item.\n\r", ch );
        return;
    }
    remove_obj( ch, obj->wear_loc, TRUE );
    return;
}

void do_sacrifice( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    int gold;
    
    /* variables for AUTOSPLIT */
    CHAR_DATA *gch;
    int members;
    char buffer[100];


    one_argument( argument, arg );

    if ( arg[0] == '\0' || !str_cmp( arg, ch->name ) )
    {
	act( "$n offers $mself to the Gods, who graciously decline.",
	    ch, NULL, NULL, TO_ROOM );
      send_to_char("The Gods appreciate your offer and may accept it later.\n\r",ch);
	return;
    }

    if ( ch->fighting )
    {
	send_to_char("The Gods refuse to accept any sacrifice while you are in combat.\n\r",ch);
	return;
    }

    if (!str_cmp("all",arg) || !str_prefix("all.",arg))
    {
        OBJ_DATA *obj_next;
        bool found = FALSE;

        for (obj = ch->in_room->contents;obj;obj = obj_next)
        { 
          obj_next = obj->next_content;

          if (arg[3] != '\0' && !is_name(&arg[4],obj->name))
          continue;

          if( !CAN_WEAR(obj,ITEM_TAKE)
          ||  CAN_WEAR(obj,ITEM_NO_SAC)
          ||  (obj->item_type == ITEM_CORPSE_PC && obj->contains))
          continue;

          if (obj->in_room != NULL)
          {
	      for (gch = obj->in_room->people; gch != NULL; gch = gch->next_in_room)
	        if (gch->on == obj)
	        {
		    act("$N appears to be using $p.", ch,obj,gch,TO_CHAR);
		    return;
	        }
          }

        gold = UMAX(1,obj->level * 3);
        if (obj->item_type != ITEM_CORPSE_NPC
        && obj->item_type != ITEM_CORPSE_PC)

        gold = UMIN(gold,obj->cost);
        found = TRUE;
        printf_to_char( ch, "You recieve %d gold for your sacrifice of %s.\n\r", gold, obj->short_descr );
        act( "$n sacrifices $p to the Gods.", ch, obj, NULL, TO_ROOM );
        ch->gold += gold;
        extract_obj( obj );

        if (IS_SET(ch->act,PLR_AUTOSPLIT))
        { 
          members = 0;
          for (gch = ch->in_room->people;gch;gch = gch->next_in_room)
          if (is_same_group(ch,gch))
            members++;
          if (members > 1 && gold > 1)
          { 
            sprintf(buf,"%d",gold);
            do_function( ch, &do_split, buf);
          }
        }
      }
      if (found)
      wiznet("$N sends up everything in that room as a burnt offering.",
        ch,obj,WIZ_SACCING,0,0);    
      else
        send_to_char("There is nothing sacrificable in this room.\n\r",ch);
        return;
    }

    obj = get_obj_list( ch, arg, ch->in_room->contents );
    if ( obj == NULL )
    {
	send_to_char( "You can't find it.\n\r", ch );
	return;
    }

    if ( obj->item_type == ITEM_CORPSE_PC )
    {
	if (obj->contains)
      {
        send_to_char( "The Gods wouldn't like that.\n\r", ch );
	  return;
      }
    }

    if ( !CAN_WEAR(obj, ITEM_TAKE) || CAN_WEAR(obj, ITEM_NO_SAC))
    {
	act( "$p is not an acceptable sacrifice.", ch, obj, 0, TO_CHAR );
	return;
    }

    if (obj->in_room != NULL)
    {
	for (gch = obj->in_room->people; gch != NULL; gch = gch->next_in_room)
	if (gch->on == obj)
	{
	  act("$N appears to be using $p.",ch,obj,gch,TO_CHAR);
        return;
	}
    }
		
    gold = UMAX(1,obj->level * 3);

    if (obj->item_type != ITEM_CORPSE_NPC && obj->item_type != ITEM_CORPSE_PC)
    	gold = UMIN(gold,obj->cost);

    if (gold == 1)
	send_to_char( "The Gods give you one gold coin for your sacrifice.\n\r", ch );
    else
    {
	sprintf(buf,"The Gods gives you %d gold coins for your sacrifice.\n\r",gold);
	send_to_char(buf,ch);
    }
    
    ch->gold += gold;
    
    if (IS_SET(ch->act,PLR_AUTOSPLIT) )
    { /* AUTOSPLIT code */
    	members = 0;
	for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    	{
    	    if ( is_same_group( gch, ch ) )
            members++;
    	}

	if ( members > 1 && gold)
	{
	    sprintf(buffer,"%d",gold);
	    do_function(ch, &do_split, buffer);	
	}
    }

    act( "$n sacrifices $p to the Gods.", ch, obj, NULL, TO_ROOM );
    wiznet("$N sends up $p as a burnt offering.",
	   ch,obj,WIZ_SACCING,0,0);
    extract_obj( obj );
    return;
}



void do_quaff( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if ( ch->fighting )
    {
	send_to_char("You have better things to do with your hands right now.\n\r",ch);
	return;
    }

    if( IS_AFFECTED( ch, AFF_SHACKLES) && !IS_IMMORTAL(ch) )
    {
	send_to_char("You cannot seem to get it to your lips with your hands bound.\n\r",ch);
	return;
    }

    if ( arg[0] == '\0' )
    {
	send_to_char( "Quaff what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL )
    {
	send_to_char( "You do not have that potion.\n\r", ch );
	return;
    }

    if ( obj->item_type != ITEM_POTION )
    {
	send_to_char( "You can quaff only potions.\n\r", ch );
	return;
    }

    if (ch->level < obj->level - 20)
    {
	send_to_char("This liquid is too powerful for you to drink.\n\r",ch);
	return;
    }

    act( "$n quaffs $p.", ch, obj, NULL, TO_ROOM );
    act( "You quaff $p.", ch, obj, NULL ,TO_CHAR );

    obj_cast_spell( obj->value[1], obj->value[0], ch, ch, NULL );
    obj_cast_spell( obj->value[2], obj->value[0], ch, ch, NULL );
    obj_cast_spell( obj->value[3], obj->value[0], ch, ch, NULL );

    extract_obj( obj );
    WAIT_STATE( ch, 2 * PULSE_VIOLENCE );
    return;
}



void do_recite( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *scroll;
    OBJ_DATA *obj;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if(IS_NPC(ch))
	return;

    if ( ( scroll = get_obj_carry( ch, arg1, ch ) ) == NULL )
    {
	send_to_char( "You do not have that scroll.\n\r", ch );
	return;
    }

    if (IS_AFFECTED2(ch, AFF_SILENCE) )
    {
	send_to_char( "You can't speak the words.\n\r", ch);
	return;
    }

    if (IS_AFFECTED2( ch, AFF_FORGET))
    {
	send_to_char("For the life of you, you cannot remember how to read scrolls.\n\r",ch);
	return;
    }

    if (IS_AFFECTED2(ch, AFF_PARALYSIS) )
    {
        send_to_char("You have been paralized! You couldn't even bat an eyelash!\n\r",ch);
        return;
    }

        /* cant recite from front row */
    if (( ch->fighting )
    && ( ch->fight_pos == TRUE))
    {
	send_to_char("Are you kidding? You're WAY too busy to read a scroll.\n\r", ch );
	return;
    }

    if( IS_AFFECTED( ch, AFF_SHACKLES) && !IS_IMMORTAL(ch) )
    {
	send_to_char( "You can't seem to hold it up properly with your hands shackled.\n\r", ch );
	return;
    }

    if ( scroll->item_type != ITEM_SCROLL )
    {
	send_to_char( "You can recite only scrolls.\n\r", ch );
	return;
    }

    if ( !IS_NPC( ch )
    && ch->level < skill_table[gsn_scrolls].skill_level[ch->class] )
    {
      send_to_char("You do not know how to recite a scroll.\n\r", ch);
      return;
    }

    if ( (number_percent() > ch->pcdata->learned[gsn_lang_elven])
    &&   (scroll->value[4] == skill_lookup("elven tongue")) )
    {
         sprintf(buf,"You fail to decipher the elven language of %s.\n\r",
         scroll->short_descr);
         send_to_char(buf,ch);
         sprintf(buf,"\n\rFlames engulf %s, burning it to ash.\n\r",
         scroll->short_descr);
         send_to_char(buf,ch);
         extract_obj(scroll); 
         return;
    }

    if ( (number_percent() > ch->pcdata->learned[gsn_lang_drow])
    &&   (scroll->value[4] == skill_lookup("drow tongue")) )
    {
         sprintf(buf,"You fail to decipher the dark language of %s.\n\r",
         scroll->short_descr);
         send_to_char(buf,ch);
         sprintf(buf,"\n\rFlames engulf %s, burning it to ash.\n\r",
         scroll->short_descr);
         send_to_char(buf,ch);
         extract_obj(scroll);
         return;
    }

    if ( (number_percent() > ch->pcdata->learned[gsn_lang_dwarven])
    &&   (scroll->value[4] == skill_lookup("dwarven tongue")) )
    {
         sprintf(buf,"You fail to decipher the dwarven language of %s.\n\r",
         scroll->short_descr);
         send_to_char(buf,ch);
         sprintf(buf,"\n\rFlames engulf %s, burning it to ash.\n\r",
         scroll->short_descr);
         send_to_char(buf,ch);
         extract_obj(scroll);
         return;
    }

    if ( (number_percent() > ch->pcdata->learned[gsn_lang_ogre])
    &&   (scroll->value[4] == skill_lookup("ogre tongue")) )
    {
         sprintf(buf,"You fail to decipher the ogre language of %s.\n\r",
         scroll->short_descr);
         send_to_char(buf,ch);
         sprintf(buf,"\n\rFlames engulf %s, burning it to ash.\n\r",
         scroll->short_descr);
         send_to_char(buf,ch);
         extract_obj(scroll);
         return;
    }

    if ( (number_percent() > ch->pcdata->learned[gsn_lang_kender])
    &&   (scroll->value[4] == skill_lookup("kender tongue")) )
    {
         sprintf(buf,"You fail to decipher the kender language of %s.\n\r",
         scroll->short_descr);
         send_to_char(buf,ch);
         sprintf(buf,"\n\rFlames engulf %s, burning it to ash.\n\r",
         scroll->short_descr);
         send_to_char(buf,ch);
         extract_obj(scroll);
         return;
    }

    if ( (number_percent() > ch->pcdata->learned[gsn_lang_silvanesti])
    &&   (scroll->value[4] == skill_lookup("silvanesti tongue")) )
    {
         sprintf(buf,"You fail to decipher the silvanesti language of %s.\n\r",
         scroll->short_descr);
         send_to_char(buf,ch);
         sprintf(buf,"\n\rFlames engulf %s, burning it to ash.\n\r",
         scroll->short_descr);
         send_to_char(buf,ch);
         extract_obj(scroll);
         return;
    }

    if ( (number_percent() > ch->pcdata->learned[gsn_lang_qualinesti])
    &&   (scroll->value[4] == skill_lookup("qualinesti tongue")) )
    {
         sprintf(buf,"You fail to decipher the qualinesti language of %s.\n\r",
         scroll->short_descr);
         send_to_char(buf,ch);
         sprintf(buf,"\n\rFlames engulf %s, burning it to ash.\n\r",
         scroll->short_descr);
         send_to_char(buf,ch);
         extract_obj(scroll);
         return;
    }

    if ( (number_percent() > ch->pcdata->learned[gsn_lang_kagonesti])
    &&   (scroll->value[4] == skill_lookup("kagonesti tongue")) )
    {
         sprintf(buf,"You fail to decipher the kagonesti language of %s.\n\r",
         scroll->short_descr);
         send_to_char(buf,ch);
         sprintf(buf,"\n\rFlames engulf %s, burning it to ash.\n\r",
         scroll->short_descr);
         send_to_char(buf,ch);
         extract_obj(scroll);
         return;
    }

    if ( (number_percent() > ch->pcdata->learned[gsn_lang_dargonesti])
    &&   (scroll->value[4] == skill_lookup("dargonesti tongue")) )
    {
         sprintf(buf,"You fail to decipher the dargonesti language of %s.\n\r",
         scroll->short_descr);
         send_to_char(buf,ch);
         sprintf(buf,"\n\rFlames engulf %s, burning it to ash.\n\r",
         scroll->short_descr);
         send_to_char(buf,ch);
         extract_obj(scroll);
         return;
    }

    if ( (number_percent() > ch->pcdata->learned[gsn_lang_dimernesti])
    &&   (scroll->value[4] == skill_lookup("dimernesti tongue")) )
    {
         sprintf(buf,"You fail to decipher the dimernesti language of %s.\n\r",
         scroll->short_descr);
         send_to_char(buf,ch);
         sprintf(buf,"\n\rFlames engulf %s, burning it to ash.\n\r",
         scroll->short_descr);
         send_to_char(buf,ch);
         extract_obj(scroll);
         return;
    }

    if ( (number_percent() > ch->pcdata->learned[gsn_lang_draconian])
    &&   (scroll->value[4] == skill_lookup("draconian tongue")) )
    {
         sprintf(buf,"You fail to decipher the draconian language of %s.\n\r",
         scroll->short_descr);
         send_to_char(buf,ch);
         sprintf(buf,"\n\rFlames engulf %s, burning it to ash.\n\r",
         scroll->short_descr);
         send_to_char(buf,ch);
         extract_obj(scroll);
         return;
    }

    if ( (number_percent() > ch->pcdata->learned[gsn_lang_minotaur])
    &&   (scroll->value[4] == skill_lookup("minotaur tongue")) )
    {
         sprintf(buf,"You fail to decipher the minotaur language of %s.\n\r",
         scroll->short_descr);
         send_to_char(buf,ch);
         sprintf(buf,"\n\rFlames engulf %s, burning it to ash.\n\r",
         scroll->short_descr);
         send_to_char(buf,ch);
         extract_obj(scroll);
         return;
    }

    if ( ch->level < scroll->level - 20 )
    {
	send_to_char(
		"This scroll is too complex for you to comprehend.\n\r",ch);
	return;
    }

    obj = NULL;
    if ( arg2[0] == '\0' )
    {
	victim = ch;
    }
    else
    {
	if ( ( victim = get_char_room ( ch, NULL, arg2 ) ) == NULL
	&&   ( obj    = get_obj_here  ( ch, NULL, arg2 ) ) == NULL )
	{
	    send_to_char( "You can't find it.\n\r", ch );
	    return;
	}
    }

    act( "$n recites $p.", ch, scroll, NULL, TO_ROOM );
    act( "You recite $p.", ch, scroll, NULL, TO_CHAR );

    if ((number_percent() >= 20 + get_skill(ch,gsn_scrolls) * 4/5)
    || ((!str_cmp(class_table[ch->class].name, "Spellfilcher"))
    && (number_percent() > 50))) 
    {
	send_to_char("You mispronounce a syllable.\n\r",ch);
	check_improve(ch,gsn_scrolls,FALSE,2);
    }
    else
    {
    	obj_cast_spell( scroll->value[1], scroll->value[0], ch, victim, obj );
    	obj_cast_spell( scroll->value[2], scroll->value[0], ch, victim, obj );
    	obj_cast_spell( scroll->value[3], scroll->value[0], ch, victim, obj );
	check_improve(ch,gsn_scrolls,TRUE,2);
    }

    extract_obj( scroll );
    WAIT_STATE( ch, 2 * PULSE_VIOLENCE );
    return;
}



void do_brandish( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    OBJ_DATA *staff;
    int sn;

    if(IS_NPC(ch))
	return;

    if ( ( staff = get_eq_char( ch, WEAR_HOLD ) ) == NULL )
    {
	send_to_char( "You hold nothing in your hand.\n\r", ch );
	return;
    }

    if (IS_AFFECTED2( ch, AFF_FORGET))
    {
	send_to_char("For the life of you, you cannot remember how to use magical devices.\n\r",ch);
	return;
    }

    if( IS_AFFECTED( ch, AFF_SHACKLES) && !IS_IMMORTAL(ch) )
    {
	send_to_char( "You can't hold it properly with your hands shackled.\n\r", ch );
	return;
    }

    if ( staff->item_type != ITEM_STAFF )
    {
	send_to_char( "You can brandish only with a staff.\n\r", ch );
	return;
    }

    if ( ( sn = staff->value[3] ) < 0
    ||   sn >= MAX_SKILL
    ||   skill_table[sn].spell_fun == 0 )
    {
	bug( "Do_brandish: bad sn %d.", sn );
	return;
    }

    WAIT_STATE( ch, 2 * PULSE_VIOLENCE );

    if ( staff->value[2] > 0 )
    {
	act( "$n brandishes $p.", ch, staff, NULL, TO_ROOM );
	act( "You brandish $p.",  ch, staff, NULL, TO_CHAR );
	if ( ch->level < (staff->level - 20 ) 
	||   number_percent() >= 20 + get_skill(ch,gsn_staves) * 4/5)
 	{
	    act ("You fail to invoke $p.",ch,staff,NULL,TO_CHAR);
	    act ("...and nothing happens.",ch,NULL,NULL,TO_ROOM);
	    check_improve(ch,gsn_staves,FALSE,2);
	}
	
	else for ( vch = ch->in_room->people; vch; vch = vch_next )
	{
	    vch_next	= vch->next_in_room;

	    switch ( skill_table[sn].target )
	    {
	    default:
		bug( "Do_brandish: bad target for sn %d.", sn );
		return;

	    case TAR_IGNORE:
		if ( vch != ch )
		    continue;
		break;

	    case TAR_CHAR_OFFENSIVE:
		if ( IS_NPC(ch) ? IS_NPC(vch) : !IS_NPC(vch) )
		    continue;
		break;

          case TAR_CHAR_SEMIOFFENSIVE:
            if ( IS_NPC(ch) ? IS_NPC(vch) : !IS_NPC(vch) )
                continue;
            break;
		
	    case TAR_CHAR_DEFENSIVE:
		if ( IS_NPC(ch) ? !IS_NPC(vch) : IS_NPC(vch) )
		    continue;
		break;

	    case TAR_CHAR_SELF:
		if ( vch != ch )
		    continue;
		break;
	    }

	    obj_cast_spell( staff->value[3], staff->value[0], ch, vch, NULL );
	    check_improve(ch,gsn_staves,TRUE,2);
	}
    }

    if ( --staff->value[2] <= 0 )
    {
	act( "$n's $p blazes bright and is gone.", ch, staff, NULL, TO_ROOM );
	act( "Your $p blazes bright and is gone.", ch, staff, NULL, TO_CHAR );
	extract_obj( staff );
    }
    WAIT_STATE( ch, 2 * PULSE_VIOLENCE );
    return;
}



void do_zap( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *wand;
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if(IS_NPC(ch))
	return;

    if (IS_AFFECTED2( ch, AFF_FORGET))
    {
	send_to_char("For the life of you, you cannot remember how to use magical devices.\n\r",ch);
	return;
    }

    if( IS_AFFECTED( ch, AFF_SHACKLES) && !IS_IMMORTAL(ch) )
    {
	send_to_char( "You can't seem to hold it properly with your hands shackled.\n\r", ch );
	return;
    }

    if ( arg[0] == '\0' && ch->fighting == NULL )
    {
	send_to_char( "Zap whom or what?\n\r", ch );
	return;
    }

    if ( ( wand = get_eq_char( ch, WEAR_HOLD ) ) == NULL )
    {
	send_to_char( "You hold nothing in your hand.\n\r", ch );
	return;
    }

    if ( wand->item_type != ITEM_WAND )
    {
	send_to_char( "You can zap only with a wand.\n\r", ch );
	return;
    }

    obj = NULL;
    if ( arg[0] == '\0' )
    {
	if ( ch->fighting != NULL )
	{
	    victim = ch->fighting;
	}
	else
	{
	    send_to_char( "Zap whom or what?\n\r", ch );
	    return;
	}
    }
    else
    {
	if ( ( victim = get_char_room ( ch, NULL, arg ) ) == NULL
	&&   ( obj    = get_obj_here  ( ch, NULL, arg ) ) == NULL )
	{
	    send_to_char( "You can't find it.\n\r", ch );
	    return;
	}
    }

    WAIT_STATE( ch, 2 * PULSE_VIOLENCE );

    if ( wand->value[2] > 0 )
    {
	if ( victim != NULL )
	{
	    act( "$n zaps $N with $p.", ch, wand, victim, TO_NOTVICT );
	    act( "You zap $N with $p.", ch, wand, victim, TO_CHAR );
	    act( "$n zaps you with $p.",ch, wand, victim, TO_VICT );
	}
	else
	{
	    act( "$n zaps $P with $p.", ch, wand, obj, TO_ROOM );
	    act( "You zap $P with $p.", ch, wand, obj, TO_CHAR );
	}

 	if (ch->level < (wand->level - 20 ) 
	||  number_percent() >= 20 + get_skill(ch,gsn_wands) * 4/5) 
	{
	    act( "Your efforts with $p produce only smoke and sparks.",
		 ch,wand,NULL,TO_CHAR);
	    act( "$n's efforts with $p produce only smoke and sparks.",
		 ch,wand,NULL,TO_ROOM);
	    check_improve(ch,gsn_wands,FALSE,2);
	}
	else
	{
	    obj_cast_spell( wand->value[3], wand->value[0], ch, victim, obj );
	    check_improve(ch,gsn_wands,TRUE,2);
	}
    }

    if ( --wand->value[2] <= 0 )
    {
	act( "$n's $p explodes into fragments.", ch, wand, NULL, TO_ROOM );
	act( "Your $p explodes into fragments.", ch, wand, NULL, TO_CHAR );
	extract_obj( wand );
    }

    WAIT_STATE( ch, 2 * PULSE_VIOLENCE );
    return;
}



void do_steal( CHAR_DATA *ch, char *argument )
{
  char buf  [MAX_STRING_LENGTH];
  char arg1 [MAX_INPUT_LENGTH];
  char arg2 [MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  CHAR_DATA *tmp_ch;
  OBJ_DATA *obj;
  int percent;
  bool worn = FALSE;

  if (!IS_NPC( ch ) 
  && ch->level < skill_table[gsn_steal].skill_level[ch->class] )
  {
    send_to_char( "Stealing? Better leave that one to the Thieves.\n\r", ch );
    return;
  }

  if (!IS_NPC( ch )
  && ( get_skill(ch,gsn_steal) < 1 ))
  {
    send_to_char( "Stealing? Better leave that one to the Thieves.\n\r", ch );
    return;
  }

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  if ( arg1[0] == '\0' || arg2[0] == '\0' )
  {
    send_to_char( "Steal what from whom?\n\r", ch );
    return;
  }

  if (ch->riding && IS_NPC(ch))
  {
    send_to_char("You can't do that while mounted.\n\r",ch);
    return;
  }

  if ( ( victim = get_char_room( ch, NULL, arg2 ) ) == NULL )
  {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

    if ( victim == ch )
    {
	send_to_char( "That's pointless.\n\r", ch );
	return;
    }

    if (is_safe(ch,victim))
	return;
  
    if ( IS_NPC(victim) 
	  && victim->position == POS_FIGHTING)
    {
	send_to_char(  "Kill stealing is not permitted.\n\r"
		       "You'd better not -- you might get hit.\n\r",ch);
	return;
    }

    tmp_ch = (is_affected(ch,gsn_doppelganger) && !IS_IMMORTAL(victim)) ?
     ch->doppel : ch;

    WAIT_STATE( ch, skill_table[gsn_steal].beats );
    percent  = number_percent( ) + ( IS_AWAKE(victim) ? 10 : -50 );

    if ( ch->level + 5 < victim->level
    ||   victim->position == POS_FIGHTING
    || IS_AFFECTED(ch,AFF_CHARM)
    || (IS_NPC(ch) && IS_SET(ch->act,ACT_PET))
    || ( !IS_NPC(ch) && percent > ch->pcdata->learned[gsn_steal] ) 
    || (victim->level > LEVEL_HERO && ch->level <= LEVEL_HERO))
    {
	/*
	 * Failure.
	 */
	if (!(IS_AFFECTED(victim, AFF_SLEEP))
      &&  !(IS_AFFECTED2(victim, AFF_BLACKJACK)))
         victim->position = POS_STANDING; 

        send_to_char( "Oops, you failed.\n\r", ch );
        if ((IS_AFFECTED(ch,AFF_HIDE)) || (IS_AFFECTED(ch,AFF_INVISIBLE)));
           do_visible(ch,"");

        act( "$n tried to steal from you.\n\r",ch,NULL,victim,TO_VICT);
        act( "$n tried to steal from $N.\n\r", ch,NULL,victim,TO_NOTVICT);
	switch(number_range(0,3))
	{
	case 0 :
	   if (can_see(victim, ch))
           {
             sprintf( buf, "%s is a lousy thief!", tmp_ch->name );
	     }
	   else
           {
	     sprintf( buf, "Someone is a lousy thief!" );
           }
         break;
      case 1 :
	   if (can_see(victim, ch))
           {
             sprintf( buf, "%s couldn't rob %s way out of a paper bag!",
                    tmp_ch->name,(tmp_ch->sex == 2) ? "her" : "his");
	     }
	   else 
           {
	     sprintf( buf, "Someone couldn't rob their way out of a paper bag!" );
           }
	   break;
	case 2 :
	   if (can_see(victim, ch))
	     {
             sprintf( buf,"%s tried to rob me!",tmp_ch->name );
	     }
	   else 
	     {
	       sprintf( buf, "Someone tried to rob me!" );
           }
           break;
	case 3 :
	   if (can_see(victim, ch))
	     {
             sprintf(buf,"Keep your hands out of there, %s!",tmp_ch->name);
	     }
	   else 
	     {
	       sprintf( buf, "Keep your hands out of there!" );
           }
           break;
        }
        if (IS_AFFECTED(victim, AFF_SLEEP)
	  ||  IS_AFFECTED2(victim, AFF_BLACKJACK))
           act("$n murmurs in $s sleep.",victim,NULL,NULL,TO_ROOM);
	  else
	   {
	  if (!IS_NPC(victim))
	   do_yell( victim, buf );
	   }
	if ( !IS_NPC(ch) )
	{
	    if ( IS_NPC(victim) )
	    {
	        check_improve(ch,gsn_steal,FALSE,2);
		multi_hit( victim, ch, TYPE_UNDEFINED );
	    }
	    else
	    {
		sprintf(buf,"$N tried to steal from %s.",victim->name);
		wiznet(buf,ch,NULL,WIZ_FLAGS,0,0);
	    }
	}

	return;
    }

    if ( !str_cmp( arg1, "coin"  )
    ||   !str_cmp( arg1, "coins" )
    ||   !str_cmp( arg1, "steel"  ) 
    ||   !str_cmp( arg1, "gold"))
    {
        int amount;
        amount = victim->steel * number_range(1, 10) / 100;
        if ( amount <= 0 )
        {
            send_to_char( "You couldn't get any steel.\n\r", ch );
            return;
        }
        ch->steel     += amount;
        victim->steel -= amount;
        sprintf( buf, "Bingo!  You got %d steel coins.\n\r", amount );
        send_to_char( buf, ch );
        check_improve(ch,gsn_steal,TRUE,2);
        return;
    }


    if ( ( obj = get_obj_carry( victim, arg1, ch ) ) == NULL )
    {
	send_to_char( "You can't find it.\n\r", ch );
	return;
    }
	
    if ( !can_drop_obj( ch, obj )
    ||   IS_SET(obj->extra_flags, ITEM_INVENTORY)
    ||   IS_SET(obj->extra_flags, ITEM_NODROP)
    ||   obj->level > ch->level )
    {
	send_to_char( "You can't pry it away.\n\r", ch );
	return;
    }

    if ( ch->carry_number + get_obj_number( obj ) > can_carry_n( ch ) )
    {
	send_to_char( "You have your hands full.\n\r", ch );
	return;
    }

    if ( ch->carry_weight + get_obj_weight( obj ) > can_carry_w( ch ) )
    {
	send_to_char( "You can't carry that much weight.\n\r", ch );
	return;
    }

    if ( get_obj_weight( obj ) > 50)
    {
      send_to_char( "They may notice something that large missing from their possessions,\n\r", ch);
      return;
    }

    if ( worn == TRUE ) 
    {
        if (IS_SET(obj->extra_flags,ITEM_NOREMOVE)) 
        {  
            send_to_char( "You can't remove it.\n\r", ch );
            return;
        }
    }

    if ( (worn == TRUE) && (ch->position > POS_SLEEPING) )
    {
        send_to_char("You can't get it off them while they're wide awake!\n\r",ch);
        return;
    }

    if ( worn == TRUE )
    unequip_char(ch,obj);

    obj_from_char( obj );
    obj_to_char( obj, ch );
    act("You pocket $p.",ch,obj,NULL,TO_CHAR);
    check_improve(ch,gsn_steal,TRUE,2);
    send_to_char( "Got it!\n\r", ch );
    return;
}



/*
 * Shopping commands.
 */
CHAR_DATA *find_keeper( CHAR_DATA *ch )
{
    /*char buf[MAX_STRING_LENGTH];*/
    CHAR_DATA *keeper;
    SHOP_DATA *pShop;

    pShop = NULL;
    for ( keeper = ch->in_room->people; keeper; keeper = keeper->next_in_room )
    {
	if ( IS_NPC(keeper) && (pShop = keeper->pIndexData->pShop) != NULL )
	    break;
    }

    if ( pShop == NULL )
    {
	send_to_char( "You can't do that here.\n\r", ch );
	return NULL;
    }

    /*
     * Undesirables.
     *
    if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_KILLER) )
    {
	do_function(keeper, &do_say, "Killers are not welcome!");
	sprintf(buf, "GUARDS! %s the KILLER is over here!\n\r", ch->name);
	do_function(keeper, &do_yell, buf );
	return NULL;
    }

    if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_THIEF) )
    {
	do_function(keeper, &do_say, "Thieves are not welcome!");
	sprintf(buf, "GUARDS! %s the THIEF is over here!\n\r", ch->name);
	do_function(keeper, &do_yell, buf );
	return NULL;
    }

    if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_ASSAULT) )
    {
	do_function(keeper, &do_say, "People who assault others are not welcome!");
	sprintf(buf, "GUARDS! %s the ASSAULTER is over here!\n\r", ch->name);
	do_function(keeper, &do_yell, buf );
	return NULL;
    }

    if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_TREASON) )
    {
	do_function(keeper, &do_say, "Treasoners are not welcome!");
	sprintf(buf, "GUARDS! %s the TREASONER is over here!\n\r", ch->name);
	do_function(keeper, &do_yell, buf );
	return NULL;
    }

    if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_OUTLAW) )
    {
	do_function(keeper, &do_say, "Outlaws are not welcome!");
	sprintf(buf, "GUARDS! %s the OUTLAW is over here!\n\r", ch->name);
	do_function(keeper, &do_yell, buf );
	return NULL;
    }

	*/
    /*
     * Shop hours.
     */
    if ( time_info.hour < pShop->open_hour )
    {
	do_function(keeper, &do_say, "Sorry, I am closed. Come back later.");
	return NULL;
    }
    
    if ( time_info.hour > pShop->close_hour )
    {
	do_function(keeper, &do_say, "Sorry, I am closed. Come back tomorrow.");
	return NULL;
    }

    /*
     * Invisible or hidden people.
     */
    if ( !can_see( keeper, ch ) && !IS_IMMORTAL( ch ) )
    {
	do_function(keeper, &do_say, "I don't trade with folks I can't see.");
	return NULL;
    }

    return keeper;
}

/* insert an object at the right spot for the keeper */
void obj_to_keeper( OBJ_DATA *obj, CHAR_DATA *ch )
{
    OBJ_DATA *t_obj, *t_obj_next;

    /* see if any duplicates are found */
    for (t_obj = ch->carrying; t_obj != NULL; t_obj = t_obj_next)
    {
	t_obj_next = t_obj->next_content;

	if (obj->pIndexData == t_obj->pIndexData 
	&&  !str_cmp(obj->short_descr,t_obj->short_descr))
	{
	    /* if this is an unlimited item, destroy the new one */
	    if (IS_OBJ_STAT(t_obj,ITEM_INVENTORY))
	    {
		extract_obj(obj);
		return;
	    }
	    obj->cost = t_obj->cost; /* keep it standard */
	    break;
	}
    }

    if (t_obj == NULL)
    {
	obj->next_content = ch->carrying;
	ch->carrying = obj;
    }
    else
    {
	obj->next_content = t_obj->next_content;
	t_obj->next_content = obj;
    }

    obj->carried_by      = ch;
    obj->in_room         = NULL;
    obj->in_obj          = NULL;
    ch->carry_number    += get_obj_number( obj );
    ch->carry_weight    += get_obj_weight( obj );
}

/* get an object from a shopkeeper's list */
OBJ_DATA *get_obj_keeper( CHAR_DATA *ch, CHAR_DATA *keeper, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number;
    int count;
 
    number = number_argument( argument, arg );
    count  = 0;
    for ( obj = keeper->carrying; obj != NULL; obj = obj->next_content )
    {
        if (obj->wear_loc == WEAR_NONE
        &&  can_see_obj( keeper, obj )
	&&  can_see_obj(ch,obj)
        &&  is_name( arg, obj->name ) )
        {
            if ( ++count == number )
                return obj;
	
	    /* skip other objects of the same name */
	    while (obj->next_content != NULL
	    && obj->pIndexData == obj->next_content->pIndexData
	    && !str_cmp(obj->short_descr,obj->next_content->short_descr))
		obj = obj->next_content;
        }
    }
 
    return NULL;
}

int get_cost( CHAR_DATA *keeper, OBJ_DATA *obj, bool fBuy )
{
    SHOP_DATA *pShop;
    int cost;

    if ( obj == NULL || ( pShop = keeper->pIndexData->pShop ) == NULL )
	return 0;

    if ( fBuy )
    {
	cost = obj->cost * pShop->profit_buy  / 100;
    }
    else
    {
	OBJ_DATA *obj2;
	int itype;

	cost = 0;
	for ( itype = 0; itype < MAX_TRADE; itype++ )
	{
	    if ( obj->item_type == pShop->buy_type[itype] )
	    {
		cost = obj->cost * pShop->profit_sell / 100;
		break;
	    }
	}

	if (!IS_OBJ_STAT(obj,ITEM_SELL_EXTRACT))
	    for ( obj2 = keeper->carrying; obj2; obj2 = obj2->next_content )
	    {
                if ( obj->pIndexData == obj2->pIndexData && !str_cmp(obj->short_descr,obj2->short_descr) )
                {
	 	    if (IS_OBJ_STAT(obj2,ITEM_INVENTORY))
			cost /= 2;
		    else
                        cost = cost * 3 / 4;
                }
	    }
    }

    if ( obj->item_type == ITEM_STAFF || obj->item_type == ITEM_WAND )
    {
	if (obj->value[1] == 0)
	    cost /= 4;
	else
	    cost = cost * obj->value[2] / obj->value[1];
    }

    return cost;
}



void do_buy( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    int cost,roll;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Buy what?\n\r", ch );
	return;
    }

    if ( IS_SET(ch->in_room->room_flags, ROOM_PET_SHOP) )
    {
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *pet;
	ROOM_INDEX_DATA *pRoomIndexNext;
	ROOM_INDEX_DATA *in_room;

	smash_tilde(argument);

	if ( IS_NPC(ch) )
	    return;

	argument = one_argument(argument,arg);

	/* hack to make new thalos pets work */
	if (ch->in_room->vnum == 9621)
	    pRoomIndexNext = get_room_index(9706);
	else
	    pRoomIndexNext = get_room_index( ch->in_room->vnum + 1 );
	if ( pRoomIndexNext == NULL )
	{
	    bug( "Do_buy: bad pet shop at vnum %d.", ch->in_room->vnum );
	    send_to_char( "Sorry, you can't buy that here.\n\r", ch );
	    return;
	}

	in_room     = ch->in_room;
	ch->in_room = pRoomIndexNext;
	pet         = get_char_room( ch, NULL, arg );
	ch->in_room = in_room;

	if ( pet == NULL || !IS_SET(pet->act, ACT_PET) )
	{
	    send_to_char( "Sorry, you can't buy that here.\n\r", ch );
	    return;
	}

	if ( ch->pet != NULL )
	{
	    send_to_char("You already own a pet.\n\r",ch);
	    return;
	}

 	cost = 10 * pet->level * pet->level;

	if ( (ch->gold + 100 * ch->steel) < cost )
	{
	    send_to_char( "You can't afford it.\n\r", ch );
	    return;
	}

	if ( ch->level < (pet->level - 20 ) )
	{
	    send_to_char(
		"You're not powerful enough to master this pet.\n\r", ch );
	    return;
	}

	/* haggle */
	roll = number_percent();
	if (roll < get_skill(ch,gsn_haggle))
	{
	    cost -= cost / 2 * roll / 100;
	    sprintf(buf,"You haggle the price down to %d coins.\n\r",cost);
	    send_to_char(buf,ch);
	    check_improve(ch,gsn_haggle,TRUE,4);
	
	}

	deduct_cost(ch,cost);
	pet			= create_mobile( pet->pIndexData );
	SET_BIT(pet->act, ACT_PET);
	SET_BIT(pet->affected_by, AFF_CHARM);
	pet->comm = COMM_NOTELL|COMM_NOSHOUT|COMM_NOCHANNELS;

	argument = one_argument( argument, arg );
	if ( arg[0] != '\0' )
	{
	    sprintf( buf, "%s %s", pet->name, arg );
	    free_string( pet->name );
	    pet->name = str_dup( buf );
	}

	sprintf( buf, "%sA neck tag says 'I belong to %s'.\n\r",
	    pet->description, ch->name );
	free_string( pet->description );
	pet->description = str_dup( buf );
	pet->hometown = ch->hometown;
	char_to_room( pet, ch->in_room );
	add_follower( pet, ch );
	pet->leader = ch;
	ch->pet = pet;
	send_to_char( "Enjoy your pet.\n\r", ch );
	act( "$n bought $N as a pet.", ch, NULL, pet, TO_ROOM );
	return;
    }
    else
    {
	CHAR_DATA *keeper;
	OBJ_DATA *obj,*t_obj;
	char arg[MAX_INPUT_LENGTH];
	int number, count = 1;

	if ( ( keeper = find_keeper( ch ) ) == NULL )
	    return;

	number = mult_argument(argument,arg);
	obj  = get_obj_keeper( ch,keeper, arg );
	cost = get_cost( keeper, obj, TRUE );

	if (number < 1 || number > 99)
	{
	    act("$n tells you 'Get real!",keeper,NULL,ch,TO_VICT);
	    return;
	}

	if ( cost <= 0 || !can_see_obj( ch, obj ) )
	{
	    act( "$n tells you 'I don't sell that -- try 'list''.",
		keeper, NULL, ch, TO_VICT );
	    ch->reply = keeper;
	    return;
	}

	if (!IS_OBJ_STAT(obj,ITEM_INVENTORY))
	{
	    for (t_obj = obj->next_content;
	     	 count < number && t_obj != NULL; 
	     	 t_obj = t_obj->next_content) 
	    {
	    	if (t_obj->pIndexData == obj->pIndexData
	    	&&  !str_cmp(t_obj->short_descr,obj->short_descr))
		    count++;
	    	else
		    break;
	    }

	    if (count < number)
	    {
	    	act("$n tells you 'I don't have that many in stock.",
		    keeper,NULL,ch,TO_VICT);
	    	ch->reply = keeper;
	    	return;
	    }
	}

	if ( (ch->gold + ch->steel * 100) < cost * number )
	{
	    if (number > 1)
		act("$n tells you 'You can't afford to buy that many.",
		    keeper,obj,ch,TO_VICT);
	    else
	    	act( "$n tells you 'You can't afford to buy $p'.",
		    keeper, obj, ch, TO_VICT );
	    ch->reply = keeper;
	    return;
	}
	
	if ( (obj->level - 20) > ch->level )
	{
	    act( "$n tells you 'You can't use $p yet'.",
		keeper, obj, ch, TO_VICT );
	    ch->reply = keeper;
	    return;
	}

	if (ch->carry_number +  number * get_obj_number(obj) > can_carry_n(ch))
	{
	    send_to_char( "You can't carry that many items.\n\r", ch );
	    return;
	}

	if ( ch->carry_weight + number * get_obj_weight(obj) > can_carry_w(ch))
	{
	    send_to_char( "You can't carry that much weight.\n\r", ch );
	    return;
	}

	/* haggle */
	roll = number_percent();
	if (!IS_OBJ_STAT(obj,ITEM_SELL_EXTRACT) 
	&& roll < get_skill(ch,gsn_haggle))
	{
	    cost -= obj->cost / 2 * roll / 100;
	    act("You haggle with $N.",ch,NULL,keeper,TO_CHAR);
	    check_improve(ch,gsn_haggle,TRUE,4);
	}

	if (number > 1)
	{
	    sprintf(buf,"$n buys $p[%d].",number);
	    act(buf,ch,obj,NULL,TO_ROOM);
	    sprintf(buf,"You buy $p[%d] for %d gold.",number,cost * number);
	    act(buf,ch,obj,NULL,TO_CHAR);
	}
	else
	{
	    act( "$n buys $p.", ch, obj, NULL, TO_ROOM );
	    sprintf(buf,"You buy $p for %d gold.",cost);
	    act( buf, ch, obj, NULL, TO_CHAR );
	}
	deduct_cost(ch,cost * number);
	keeper->steel += cost * number/100;
	keeper->gold += cost * number - (cost * number/100) * 100;

	for (count = 0; count < number; count++)
	{
	    if ( IS_SET( obj->extra_flags, ITEM_INVENTORY ) )
	    	t_obj = create_object( obj->pIndexData, obj->level );
	    else
	    {
		t_obj = obj;
		obj = obj->next_content;
	    	obj_from_char( t_obj );
	    }

	    if (t_obj->timer > 0 && !IS_OBJ_STAT(t_obj,ITEM_HAD_TIMER))
	    	t_obj->timer = 0;
	    REMOVE_BIT(t_obj->extra_flags,ITEM_HAD_TIMER);
	    obj_to_char( t_obj, ch );
	    if (cost < t_obj->cost)
	    	t_obj->cost = cost;
	}
    }
}



void do_list( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( IS_SET(ch->in_room->room_flags, ROOM_PET_SHOP) )
    {
        ROOM_INDEX_DATA *pRoomIndexNext;
	CHAR_DATA *pet;
	bool found;

        /* hack to make new thalos pets work */
        if (ch->in_room->vnum == 9621)
            pRoomIndexNext = get_room_index(9706);
        else
            pRoomIndexNext = get_room_index( ch->in_room->vnum + 1 );

	if ( pRoomIndexNext == NULL )
	{
	    bug( "Do_list: bad pet shop at vnum %d.", ch->in_room->vnum );
	    send_to_char( "You can't do that here.\n\r", ch );
	    return;
	}

	found = FALSE;
	for ( pet = pRoomIndexNext->people; pet; pet = pet->next_in_room )
	{
	    if ( IS_SET(pet->act, ACT_PET) )
	    {
		if ( !found )
		{
		  found = TRUE;
		  send_to_char( "Pets For Sale:\n\r\n\r", ch );
                  send_to_char( "Level | Price in Gold | Name of Pet\n\r", ch );
                  send_to_char( "-----------------------------------\n\r", ch );
		}
		sprintf( buf, "[%2d] %8d - %s\n\r",
		    pet->level,
		    10 * pet->level * pet->level,
		    pet->short_descr );
		send_to_char( buf, ch );
	    }
	}
	if ( !found )
	    send_to_char( "Sorry, we're out of pets right now.\n\r", ch );
	return;
    }
    else
    {
	CHAR_DATA *keeper;
	OBJ_DATA *obj;
	int cost,count;
	bool found;
	char arg[MAX_INPUT_LENGTH];

	if ( ( keeper = find_keeper( ch ) ) == NULL )
	    return;
        one_argument(argument,arg);

	found = FALSE;
	for ( obj = keeper->carrying; obj; obj = obj->next_content )
	{
	    if ( obj->wear_loc == WEAR_NONE
	    &&   can_see_obj( ch, obj )
	    &&   ( cost = get_cost( keeper, obj, TRUE ) ) > 0 
	    &&   ( arg[0] == '\0'  
 	       ||  is_name(arg,obj->name) ))
	    {
		if ( !found )
		{
		    found = TRUE;
		    send_to_char( "[Lv Price Qty] Item\n\r", ch );
		}

		if (IS_OBJ_STAT(obj,ITEM_INVENTORY))
		    sprintf(buf,"[%2d %5d -- ] %s\n\r",
			obj->level,cost,obj->short_descr);
		else
		{
		    count = 1;

		    while (obj->next_content != NULL 
		    && obj->pIndexData == obj->next_content->pIndexData
		    && !str_cmp(obj->short_descr,
			        obj->next_content->short_descr))
		    {
			obj = obj->next_content;
			count++;
		    }
		    sprintf(buf,"[%2d %5d %2d ] %s\n\r",
			obj->level,cost,count,obj->short_descr);
		}
		send_to_char( buf, ch );
	    }
	}

	if ( !found )
	    send_to_char( "You can't buy anything here.\n\r", ch );
	return;
    }
}



void do_sell( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *keeper;
    OBJ_DATA *obj;
    int cost,roll;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Sell what?\n\r", ch );
	return;
    }

    if ( ( keeper = find_keeper( ch ) ) == NULL )
	return;

    if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL )
    {
	act( "$n tells you 'You don't have that item'.",
	    keeper, NULL, ch, TO_VICT );
	ch->reply = keeper;
	return;
    }

    if ( !can_drop_obj( ch, obj ) )
    {
	send_to_char( "You can't let go of it.\n\r", ch );
	return;
    }

    if (!can_see_obj(keeper,obj))
    {
	act("$n doesn't see what you are offering.",keeper,NULL,ch,TO_VICT);
	return;
    }

    if ( ( cost = get_cost( keeper, obj, FALSE ) ) <= 0 )
    {
	act( "$n looks uninterested in $p.", keeper, obj, ch, TO_VICT );
	return;
    }
    if ( cost > (keeper-> gold + 100 * keeper->steel) )
    {
	act("$n tells you 'I'm afraid I don't have enough wealth to buy $p.",
	    keeper,obj,ch,TO_VICT);
	return;
    }

    act( "$n sells $p.", ch, obj, NULL, TO_ROOM );
    /* haggle */
    roll = number_percent();
    if (!IS_OBJ_STAT(obj,ITEM_SELL_EXTRACT) && roll < get_skill(ch,gsn_haggle))
    {
        send_to_char("You haggle with the shopkeeper.\n\r",ch);
        cost += obj->cost / 2 * roll / 100;
        cost = UMIN(cost,95 * get_cost(keeper,obj,TRUE) / 100);
	cost = UMIN(cost,(keeper->gold + 100 * keeper->steel));
        check_improve(ch,gsn_haggle,TRUE,4);
    }
    sprintf( buf, "You sell $p for %d gold and %d steel piece%s.",
	cost - (cost/100) * 100, cost/100, cost == 1 ? "" : "s" );
    act( buf, ch, obj, NULL, TO_CHAR );
    ch->steel     += cost/100;
    ch->gold 	 += cost - (cost/100) * 100;
    deduct_cost(keeper,cost);
    if ( keeper->steel < 0 )
	keeper->steel = 0;
    if ( keeper->gold< 0)
	keeper->gold = 0;

    if ( obj->item_type == ITEM_TRASH || IS_OBJ_STAT(obj,ITEM_SELL_EXTRACT))
    {
	extract_obj( obj );
    }
    else
    {
	obj_from_char( obj );
	if (obj->timer)
	    SET_BIT(obj->extra_flags,ITEM_HAD_TIMER);
	else
	    obj->timer = number_range(50,100);
	obj_to_keeper( obj, keeper );
    }

    return;
}



void do_value( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *keeper;
    OBJ_DATA *obj;
    int cost;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Value what?\n\r", ch );
	return;
    }

    if ( ( keeper = find_keeper( ch ) ) == NULL )
	return;

    if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL )
    {
	act( "$n tells you 'You don't have that item'.",
	    keeper, NULL, ch, TO_VICT );
	ch->reply = keeper;
	return;
    }

    if (!can_see_obj(keeper,obj))
    {
        act("$n doesn't see what you are offering.",keeper,NULL,ch,TO_VICT);
        return;
    }

    if ( !can_drop_obj( ch, obj ) )
    {
	send_to_char( "You can't let go of it.\n\r", ch );
	return;
    }

    if ( ( cost = get_cost( keeper, obj, FALSE ) ) <= 0 )
    {
	act( "$n looks uninterested in $p.", keeper, obj, ch, TO_VICT );
	return;
    }

    sprintf( buf, 
	"$n tells you 'I'll give you %d gold and %d steel coins for $p'.", 
	cost - (cost/100) * 100, cost/100 );
    act( buf, keeper, obj, ch, TO_VICT );
    ch->reply = keeper;

    return;
}

void do_push_drag( CHAR_DATA *ch, char *argument, char *verb )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    ROOM_INDEX_DATA *in_room;
    ROOM_INDEX_DATA *to_room;
    CHAR_DATA *victim;
    EXIT_DATA *pexit;
    OBJ_DATA *obj;
    int door;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    victim = get_char_room(ch,NULL,arg1);
    obj = get_obj_list( ch, arg1, ch->in_room->contents );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	sprintf( buf, "%s whom or what where?\n\r", capitalize(verb));
	send_to_char( buf, ch );
	return;
    }

    if ( (!victim || !can_see(ch,victim))
    && (!obj || !can_see_obj(ch,obj)) )
    {
	sprintf(buf,"%s whom or what where?\n\r", capitalize(verb));
	send_to_char( buf, ch );
        return;
    }

         if ( !str_cmp( arg2, "n" ) || !str_cmp( arg2, "north" ) ) door = 0;
    else if ( !str_cmp( arg2, "e" ) || !str_cmp( arg2, "east"  ) ) door = 1;
    else if ( !str_cmp( arg2, "s" ) || !str_cmp( arg2, "south" ) ) door = 2;
    else if ( !str_cmp( arg2, "w" ) || !str_cmp( arg2, "west"  ) ) door = 3;
    else if ( !str_cmp( arg2, "u" ) || !str_cmp( arg2, "up"    ) ) door = 4;
    else if ( !str_cmp( arg2, "d" ) || !str_cmp( arg2, "down"  ) ) door = 5;
    else
    {
      sprintf( buf, "Alas, you cannot %s in that direction.\n\r", verb );
      send_to_char( buf, ch );
      return;
    }

  if ( obj )
  {
    in_room = obj->in_room;
    if ( ( pexit   = in_room->exit[door] ) == NULL
    ||   ( to_room = pexit->u1.to_room   ) == NULL 
    ||	 !can_see_room(ch,pexit->u1.to_room))
    {
        sprintf( buf, "Alas, you cannot %s in that direction.\n\r", verb );
        send_to_char( buf, ch );
	return;
    }

    if ( IS_SET(pexit->exit_info, EX_CLOSED)
    || IS_SET(pexit->exit_info,EX_NOPASS) )
    {
	act( "You cannot $t it through the $d.", ch, verb, pexit->keyword, TO_CHAR );
	act( "$n decides to $t $P around!", ch, verb, obj, TO_ROOM );
	return;
    }

    act( "You attempt to $T $p out of the room.", ch, obj, verb, TO_CHAR );
    act( "$n is attempting to $T $p out of the room.", ch, obj, verb, TO_ROOM );

    if ( obj->weight >  (2 * can_carry_w (ch)) )
    {
      act( "$p is too heavy to $T.\n\r", ch, obj, verb, TO_CHAR);
      act( "$n attempts to $T $p, but it is too heavy.\n\r", ch, obj, verb, TO_ROOM);
      return;
    }
    if 	 ( !IS_IMMORTAL(ch)
    ||   IS_SET(ch->in_room->room_flags, ROOM_SAFE)
    ||   IS_SET(ch->in_room->room_flags, ROOM_PRIVATE)
    ||   IS_SET(ch->in_room->room_flags, ROOM_SOLITARY) )
    {
	send_to_char( "It won't budge.\n\r", ch );
	return;
    }

    if ( ch->move > 10 )
    {
	ch->move -= 10;
	send_to_char( "You succeed!\n\r", ch );
      send_to_char( "\n\r", ch );
	act( "$n succeeds!", ch, NULL, NULL, TO_ROOM );
	if (!str_cmp( verb, "drag" ))
	{
	act( "$n drags $p $T!", ch, obj, dir_name[door], TO_ROOM );
	char_from_room( ch );
	char_to_room( ch, pexit->u1.to_room );
	do_look( ch, "auto" );
	obj_from_room( obj );
	obj_to_room( obj, to_room );
	act( "$n drags $p into the room.", ch, obj, dir_name[door], TO_ROOM );
	}
	else if (!str_cmp( verb, "push" ))
	{
	act( "$p {Wflies{x $T!", ch, obj, dir_name[door], TO_ROOM );
	act( "$p {Wflies{x $T!", ch, obj, dir_name[door], TO_CHAR );
	char_from_room( ch );
	char_to_room( ch, pexit->u1.to_room );
	act( "$p {Wflies{x into the room!", ch, obj, dir_name[door], TO_ROOM );
	char_from_room( ch );
	char_to_room( ch, in_room );
	obj_from_room( obj );
	obj_to_room( obj, to_room );
	}
    }
    else
    {
      sprintf( buf, "You are too tired to %s anything around!\n\r", verb );
      send_to_char( buf, ch );
    }
  }
  else
  {
    if ( ch == victim )
    {
	act( "You $t yourself about the room and look very silly.", ch, verb, NULL, TO_CHAR );
	act( "$n decides to be silly and $t $mself about the room.", ch, verb, NULL, TO_ROOM );
	return;
    }

    in_room = victim->in_room;
    if ( ( pexit   = in_room->exit[door] ) == NULL
    ||   ( to_room = pexit->u1.to_room   ) == NULL 
    ||	 !can_see_room(victim,pexit->u1.to_room))
    {
        sprintf( buf, "Alas, you cannot %s them that way.\n\r", verb );
        send_to_char( buf, ch );
	return;
    }

    if (IS_SET(pexit->exit_info, EX_CLOSED)
    &&  (!IS_AFFECTED(victim, AFF_PASS_DOOR) || IS_SET(pexit->exit_info,EX_NOPASS)))
    {
	act( "You try to $t them through the $d.", ch, verb, pexit->keyword, TO_CHAR );
	act( "$n decides to $t you around!", ch, verb, victim, TO_VICT );
	act( "$n decides to $t $N around!", ch, verb, victim, TO_NOTVICT );
	return;
    }

    if (((ch->class == class_lookup("assassin"))
    ||   (ch->class == class_lookup("thief")))
    &&   ((IS_AFFECTED2(victim, AFF_BLACKJACK))
    ||    (IS_AFFECTED(victim, AFF_SLEEP)))
    &&   (!str_cmp( verb, "drag" )))
    {
	if(victim->size > (ch->size + 1))
	{
	  send_to_char( "They are too big for you to drag anywhere!\n\r", ch );
	  return;
	}

	if(victim->size == (ch->size + 1))
	{
	  if (ch->move > 300 )
	  {
          act( "You attempt to $t $N out of the room.", ch, verb, victim, TO_CHAR );
	    ch->move -= 300;
	    send_to_char( "You succeed!\n\r", ch );
	    move_char( ch, door, FALSE );
          act( "$n is dragged $T!", victim, NULL, dir_name[door], TO_ROOM );
	    char_from_room( victim );
	    char_to_room( victim, pexit->u1.to_room );
	    act( "$N drags $n into the room.", victim, NULL, ch, TO_NOTVICT );
          return;
	  }
	  else
	  {
	    send_to_char( "Your too tired to drag an unconscious body around!\n\r", ch );
	    return;
	  }
	}

	if(victim->size == ch->size)
	{
	  if (ch->move > 200 )
	  {
          act( "You attempt to $t $N out of the room.", ch, verb, victim, TO_CHAR );
	    ch->move -= 200;
	    send_to_char( "You succeed!\n\r", ch );
	    move_char( ch, door, FALSE );
          act( "$n is dragged $T!", victim, NULL, dir_name[door], TO_ROOM );
	    char_from_room( victim );
	    char_to_room( victim, pexit->u1.to_room );
	    act( "$N drags $n into the room.", victim, NULL, ch, TO_NOTVICT );
          return;
	  }
	  else
	  {
	    send_to_char( "Your too tired to drag an unconscious body around!\n\r", ch );
	    return;
	  }
	}

	if(victim->size < ch->size)
	{
	  if (ch->move > 150 )
	  {
          act( "You attempt to $t $N out of the room.", ch, verb, victim, TO_CHAR );
	    ch->move -= 150;
	    send_to_char( "You succeed!\n\r", ch );
	    move_char( ch, door, FALSE );
          act( "$n is dragged $T!", victim, NULL, dir_name[door], TO_ROOM );
	    char_from_room( victim );
	    char_to_room( victim, pexit->u1.to_room );
	    act( "$N drags $n into the room.", victim, NULL, ch, TO_NOTVICT );
          return;
	  }
	  else
	  {
	    send_to_char( "Your too tired to drag an unconscious body around!\n\r", ch );
	    return;
	  }
	}
    }

    act( "You attempt to $t $N out of the room.", ch, verb, victim, TO_CHAR );
    act( "$n is attempting to $t you out of the room!", ch, verb, victim, TO_VICT );
    act( "$n is attempting to $t $N out of the room.", ch, verb, victim, TO_NOTVICT );

/*
 * Immortals
 */
    if (IS_IMMORTAL(ch))
    {
      if ((victim->in_room == NULL)
      ||  IS_SET(victim->in_room->room_flags, ROOM_SAFE)
      ||  IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
      ||  IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
      || (IS_NPC(victim)
      &&  (IS_SET(victim->act,ACT_TRAIN)
      ||  IS_SET(victim->act,ACT_PRACTICE)
      ||  IS_SET(victim->act,ACT_IS_HEALER)
      ||  IS_SET(victim->act,ACT_IS_CHANGER)
      ||  IS_SET(victim->imm_flags,IMM_SUMMON)
      ||  victim->pIndexData->pShop) ))
      {
	  send_to_char( "They won't budge.\n\r", ch );
        return;
      }

	send_to_char( "You succeed!\n\r", ch );
	act( "$n succeeds!", ch, NULL, NULL, TO_ROOM );
	if (!str_cmp( verb, "drag" ))
	{
	  move_char( ch, door, FALSE );
	  act( "$n is dragged $T!", victim, NULL, dir_name[door], TO_ROOM );
	  act( "You are dragged $T!\n\r", victim, NULL, dir_name[door], TO_CHAR );
	  char_from_room( victim );
	  char_to_room( victim, pexit->u1.to_room );
	  do_look( victim, "auto" );
	  act( "$N drags $n into the room.", victim, NULL, ch, TO_NOTVICT );
	}
	else if (!str_cmp( verb, "push" ))
	{
	  act( "$n {Wflies{x $T!", victim, NULL, dir_name[door], TO_ROOM );
	  act( "You {Wfly{x $T!\n\r", victim, NULL, dir_name[door], TO_CHAR );
	  char_from_room( victim );
	  char_to_room( victim, pexit->u1.to_room );
	  do_look( victim, "auto" );
	  act( "$n {Wflies{x into the room!", victim, NULL, NULL, TO_ROOM );
	}
    }
    else
    {
/*
 * Mortals
 */

    if ((IS_NPC(victim)
    && (IS_SET(victim->act,ACT_TRAIN)
    ||  IS_SET(victim->act,ACT_PRACTICE)
    ||  IS_SET(victim->act,ACT_IS_HEALER)
    ||  IS_SET(victim->act,ACT_IS_CHANGER)
    ||  IS_SET(victim->imm_flags,IMM_SUMMON)
    ||  victim->pIndexData->pShop))
    ||  victim->in_room == NULL
    ||  IS_SET(victim->in_room->room_flags, ROOM_SAFE)
    ||  IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
    ||  IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
    || (!str_cmp( verb, "push" ) && victim->position != POS_STANDING)
    ||  is_safe(ch,victim)
    || (number_percent() > 50)
    || (victim->max_hit > (ch->max_hit + (get_curr_stat(ch,STAT_STR)*20))) 
    || ((victim->position == POS_SLEEPING )
    &&  (!IS_AFFECTED2(victim, AFF_BLACKJACK))
    &&  (!IS_AFFECTED(victim, AFF_SLEEP))) )
    {
	send_to_char( "They won't budge.\n\r", ch );
	return;
    }

    if ( ch->move > 200 )
    {
	ch->move -= 200;
	send_to_char( "You succeed!\n\r", ch );
	act( "$n succeeds!", ch, NULL, NULL, TO_ROOM );
	if (!str_cmp( verb, "drag" ))
	{
	move_char( ch, door, FALSE );
	act( "$n is dragged $T!", victim, NULL, dir_name[door], TO_ROOM );
	act( "You are dragged $T!\n\r", victim, NULL, dir_name[door], TO_CHAR );
	char_from_room( victim );
	char_to_room( victim, pexit->u1.to_room );
	do_look( victim, "auto" );
	act( "$N drags $n into the room.", victim, NULL, ch, TO_NOTVICT );
	}
	else if (!str_cmp( verb, "push" ))
	{
	act( "$n {Wflies{x $T!", victim, NULL, dir_name[door], TO_ROOM );
	act( "You {Wfly{x $T!\n\r", victim, NULL, dir_name[door], TO_CHAR );
	char_from_room( victim );
	char_to_room( victim, pexit->u1.to_room );
	do_look( victim, "auto" );
	act( "$n {Wflies{x into the room!", victim, NULL, NULL, TO_ROOM );
	}
    }
    else
    {
      sprintf( buf, "You are too tired to %s anybody around!\n\r", verb );
      send_to_char( buf, ch );
    }
  }
  }
  return;
}
               
void do_push( CHAR_DATA *ch, char *argument )
{
    do_push_drag( ch, argument, "push" );
    return;
}

void do_drag( CHAR_DATA *ch, char *argument )
{
    do_push_drag( ch, argument, "drag" );
    return;
}

void do_sharpen( CHAR_DATA *ch, char *argument )
{
  OBJ_DATA *obj,*stone;
  AFFECT_DATA af;
  int percent,skill;

  /* find out what */
  if (argument == '\0')
  {
    send_to_char("Sharpen what weapon?\n\r",ch);
    return;
  }

  obj =  get_obj_list(ch,argument,ch->carrying);

  if (obj== NULL)
  {
    send_to_char("You don't have that item.\n\r",ch);
    return;
  }

  if ((skill = get_skill(ch,gsn_sharpen)) < 1)
  {
    send_to_char("You have no idea how to sharpen weapons.\n\r",ch);
    return;
  }

  if (obj->item_type == ITEM_WEAPON)
  {
    if (IS_WEAPON_STAT(obj,WEAPON_SHARP))
    {
      act("$p has already been sharpened.",ch,obj,NULL,TO_CHAR);
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

    stone = get_eq_char(ch,WEAR_HOLD);

    if (!IS_IMMORTAL(ch) 
    &&  (stone == NULL || stone->item_type != ITEM_WET_STONE))
    {
	send_to_char("You lack the proper tools to sharpen weapons.\n\r",ch);
	return;
    }

    if (stone != NULL && stone->item_type == ITEM_WET_STONE)
    {
     	act("You use a $p to sharpen your weapon.",ch,stone,NULL,TO_CHAR);
     	act("It suddenly crubles into dust!",ch,stone,NULL,TO_CHAR);
     	extract_obj(stone);
    }

	percent = number_percent();
	if (percent < skill)
	{
 
            af.where     = TO_WEAPON;
            af.type      = gsn_sharpen;
            af.level     = ch->level * percent / 100;
            af.duration  = ch->level/2 * percent / 100;
            af.location  = 0;
            af.modifier  = 0;
            af.bitvector = WEAPON_SHARP;
            affect_to_obj(obj,&af);
 
            act("$n pulls out a piece of stone and begins sharpening $p.",ch,obj,NULL,TO_ROOM);
	    act("You sharpen $p.",ch,obj,NULL,TO_CHAR);
	    check_improve(ch,gsn_sharpen,TRUE,3);
	    WAIT_STATE(ch,skill_table[gsn_sharpen].beats);
            return;
        }
	else
	{
	    act("You fail to sharpen $p.",ch,obj,NULL,TO_CHAR);
	    check_improve(ch,gsn_sharpen,FALSE,3);
	    WAIT_STATE(ch,skill_table[gsn_sharpen].beats);
	    return;
	}

    act("You can only sharpen weapons.",ch,obj,NULL,TO_CHAR);
    return;
   }
  return;
}

void do_demand(CHAR_DATA *ch,char *argument)
{
    CHAR_DATA  *victim;
    OBJ_DATA *obj;
    char i_name[MAX_INPUT_LENGTH];
    char m_name[MAX_INPUT_LENGTH];
    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char *target_name;
    int chance, vlevel;

    target_name = one_argument(argument,i_name);
    one_argument(target_name,m_name);

    chance = ch->pcdata->learned[gsn_demand];
    if (chance == 0
    || ch->level < skill_table[gsn_demand].skill_level[ch->class])
    {
        send_to_char("You are hardly intimidating enough to demand off others.\n\r",ch);
        return;
    }

    if (IS_NPC(ch))
        return;

    if ((victim = get_char_room(ch, NULL, m_name)) == NULL )
    {
        send_to_char("They aren't here.\n\r",ch);
        return;
    }

        /* Prevent people from demanding items from Shopkeepers */
    if (IS_NPC(victim)
    && (victim->pIndexData->pShop != NULL))
    {
	send_to_char("That wouldn't be a very wise decision.\n\r",ch);
	return;
    }

        /* Prevent people from demanding items from Info type MOBs */
    if (IS_NPC(victim)
    && (IS_SET(victim->act,ACT_TRAIN)
    ||  IS_SET(victim->act,ACT_PRACTICE)
    ||  IS_SET(victim->act,ACT_IS_HEALER)
    ||  IS_SET(victim->act,ACT_IS_CHANGER)
    ||  IS_SET(victim->act,ACT_BANKER)))
    {        
      send_to_char("That wouldn't be a very wise decision.\n\r",ch);
      return;
    }

    if (!IS_NPC(victim))
    {
        send_to_char("Why not just threaten them in person?\n\r",ch);
        return;
    }
    vlevel = victim->level;
    chance /= 2;
    chance += (3*ch->level);
    chance -= 2*vlevel;

    if (IS_GOOD(victim))
        chance -= 4*vlevel;
    else if (IS_EVIL(victim))
        chance -= 2*vlevel;
    else
        chance -= 3*vlevel;

    vlevel += 8;

    if ((obj = get_obj_list(victim, i_name, victim->carrying)) == NULL)
    {
        send_to_char("They do not have that object.\n\r",ch);
        return;
    }
    if (!can_see_obj(ch,obj) )
    {
        send_to_char("They do not have that object.\n\r",ch);
        return;
    }

    if (victim->position != POS_STANDING)
    {
        send_to_char("They must be standing up in order to give you anything.\n\r",ch);
        return;
    }

    if (vlevel > ch->level
    || number_percent() > chance)
    {
        check_improve(ch,gsn_demand,FALSE,2);
        sprintf(buf1,"I don't think I'd give my belongings to one as weak as you!");
        sprintf(buf2,"Help! I'm being attacked by %s!",victim->short_descr);
        do_say(victim,buf1);
        do_yell(ch,buf2);
        multi_hit(victim,ch,TYPE_UNDEFINED);
        return;
    }

    if (!can_see(victim,ch))
    {
        act("$N tells you, 'I can't give to those I can't see.'",ch,0,victim,TO_CHAR);
        return;
    }

    if (!can_see_obj(victim,obj))
    {
        act("$N tells you, 'I can't see such an object.'",ch,0,victim,TO_CHAR);
        return;
    }

    if (obj->level > ch->level + 8)
    {
        do_say(victim, "That item is far to precious to hand over to scum like you!");
        sprintf(buf1,"Help! I'm being attacked by %s!",victim->short_descr);
        do_yell(ch,buf1);
        multi_hit(victim,ch,TYPE_UNDEFINED);
        return;
    }

    if (ch->move < obj->level)
    {
        act("$N tells you, 'Hah! You couldn't even get away if I chased you!'.",ch,0,victim,TO_CHAR);
        sprintf(buf1,"Help! I'm being attacked by %s!",victim->short_descr);
        do_yell(ch,buf1);
        multi_hit(victim,ch,TYPE_UNDEFINED);
        return;
    }

    if (ch->hit < (ch->max_hit*3/7))
    {
        do_say(victim,"Hah! You look weak enough that even I could kill you!");
        sprintf(buf1,"Help! I'm being attacked by %s!",victim->short_descr);
        do_yell(ch,buf1);
        multi_hit(victim,ch,TYPE_UNDEFINED);
        return;
    }

    if (ch->mana < 30)
    {
        send_to_char("You don't have the mana.\n\r",ch);
        return;
    }

    if ((obj->wear_loc != WEAR_NONE) && IS_SET(obj->extra_flags, ITEM_NOREMOVE) )
    {
        act("$N tells you, 'I'm unable to release $p'.",ch,obj,victim,TO_CHAR);
        act("$N cowers back from you in fright.",ch,0,victim,TO_CHAR);
        act("$N cowers back from $n in fright.",ch,0,victim,TO_NOTVICT);
        return;
    }
    if (IS_SET(obj->extra_flags,ITEM_NODROP) )
    {
        act("$N tells you, 'I'm unable to release $p'.",ch,obj,victim,TO_CHAR);
        act("$N cowers back from you in fright.",ch,0,victim,TO_CHAR);
        act("$N cowers back from $n in fright.",ch,0,victim,TO_NOTVICT);
        return;
    }

    if ( ch->carry_weight + get_obj_weight(obj)  > can_carry_w(ch) )
    {
        act("$N tells you, 'You can't carry the weight $n.'",ch,0,victim,TO_CHAR);
        return;
    }
    if ( ch->carry_number + 1 > can_carry_n(ch) )
    {
        act("$N tells you, 'You can't carry that many items $n.'",ch,0,victim,TO_CHAR);
        return;
    }

    act("$N caves in to the bully tactics of $n.",ch,0,victim,TO_NOTVICT);
    act("$N shivers in fright and caves in to your bully tactics.",ch,0,victim,TO_CHAR);

    if (obj->wear_loc != WEAR_NONE)
    {
        act("$n stops using $p.",victim,obj,NULL,TO_ROOM);
        act("You stop using $p.",victim,obj,NULL,TO_CHAR);
    }
    act("$N gives $p to $n.",ch,obj,victim,TO_NOTVICT);
    act("$N gives you $p.",ch,obj,victim,TO_CHAR);
    check_improve(ch,gsn_demand,TRUE,2);
    WAIT_STATE(ch,24);
    obj_from_char(obj);
    obj_to_char(obj,ch);
    ch->mana -= 30;
    ch->hit -= 30;
    return;
}

void do_lore( CHAR_DATA *ch, char *argument)
{
   char arg[MAX_INPUT_LENGTH];
   OBJ_DATA *obj;
   char buf[MAX_STRING_LENGTH];
   argument = one_argument(argument,arg);

   if (IS_AFFECTED2 (ch, AFF_FORGET))
	{
	send_to_char("You can't seem to remember anything about the value or worth of objects.\n\r",ch);
	return;
	}
   if (get_skill(ch,gsn_lore) == 0
   || ch->level < skill_table[gsn_lore].skill_level[ch->class] )
   {
      send_to_char("You fail to learn anything.\n\r",ch);
      return;
   }


   if (arg[0] == '\0')
   {
     send_to_char("Lore what?\n\r",ch);
     return;
   }

   if ( ( obj = get_obj_carry(ch,arg,ch)) == NULL)
   {
     send_to_char("You are not carrying that item.\n\r",ch);
     return;
   }

   if ( number_percent( ) < ch->pcdata->learned[gsn_lore] ) 
    check_improve(ch,gsn_lore,TRUE,1); 
   else
   {
    send_to_char("You fail to learn anything.\n\r",ch);
    check_improve(ch,gsn_lore,FALSE,1);
    return;
   }

   sprintf(buf,"You carefully examine %s.\n\r",obj->short_descr);
   send_to_char(buf,ch);
   buf[0] = '\0';

   switch(obj->item_type)
   {
     case (ITEM_SCROLL): sprintf(buf,"It appears to have letters on it.\n\r"); break;
     case (ITEM_POTION): sprintf(buf,"It appears to be some sort of magic potion.\n\r");break;
     case (ITEM_PILL):   sprintf(buf,"It appears to be edible.\n\r");break;
     case (ITEM_FOOD):   sprintf(buf,"It appears to be an edible food product.\n\r");break;
     case (ITEM_WAND):   sprintf(buf,"It appears to be a magical wand.\n\r");break;
     case (ITEM_STAFF):  sprintf(buf,"It appears to be a magical staff.\n\r");break;
     case (ITEM_INSTRUMENT): sprintf(buf,"It appears to be a playable instrument.\n\r");break;
     case (ITEM_WEAPON):
        switch(obj->value[0])
        {
        case(WEAPON_EXOTIC): sprintf(buf,"It appears to be an exotic weapon.\n\r");break;
        case(WEAPON_SWORD):  sprintf(buf,"It appears to be a sword.\n\r");         break;
       case(WEAPON_DAGGER):  sprintf(buf,"It appears to be a dagger.\n\r");        break;
       case(WEAPON_SPEAR):   sprintf(buf,"It appears to be a spear.\n\r");         break; 
       case(WEAPON_MACE):    sprintf(buf,"It appears to be a mace.\n\r");          break;
       case(WEAPON_AXE):     sprintf(buf,"It appears to be an axe.\n\r");          break;
       case(WEAPON_FLAIL):   sprintf(buf,"It appears to be a flail\n\r");           break;
       case(WEAPON_WHIP):    sprintf(buf,"It appears to be a whip.\n\r");          break;
       case(WEAPON_POLEARM): sprintf(buf,"It appears to be a polearm.\n\r");       break;
       case(WEAPON_BOW):     sprintf(buf,"It appears to be a bow.\n\r");           break;
       case(WEAPON_STAFF):   sprintf(buf,"It appears to be a staff.\n\r");         break;
       default:              sprintf(buf,"It appears to be a weapon of unkown use.\n\r");
       break;  
       }
     break;
     case (ITEM_LIGHT):sprintf(buf,"It appears to be used as a light.\n\r");break;
     case (ITEM_TREASURE):sprintf(buf,"It appears to be an unusual treasure.\n\r");break;
     case (ITEM_CLOTHING):sprintf(buf,"It appears to be clothing.\n\r");break;
     case (ITEM_FURNITURE):sprintf(buf,"It appears to be funiture.\n\r");break;
     case (ITEM_DRINK_CON):sprintf(buf,"It appears to be used to hold liquids.\n\r""It appears to hold %s-colored %s.\n\r",
                liq_table[obj->value[2]].liq_color,
                liq_table[obj->value[2]].liq_name);break;
     case (ITEM_KEY):sprintf(buf,"It appears to be a key.\n\r");break;
     case (ITEM_MONEY):sprintf(buf,"It appears to be money.\n\r");break; 
     case (ITEM_BOAT): sprintf(buf,"It appears to be a boat.\n\r");break;
     case (ITEM_CORPSE_NPC):
     case (ITEM_CORPSE_PC):sprintf(buf,"It appears to be a corpse.\n\r");break;
     case (ITEM_FOUNTAIN): sprintf(buf,"It appears to be a fountain.\n\r");break;
     case (ITEM_MAP):sprintf(buf,"It appears to be a map.\n\r");break;
     case (ITEM_PROTECT):sprintf(buf,"It appears to be a protectable item.\n\r");break;
   send_to_char(buf,ch);
     }

   if (CAN_WEAR(obj,ITEM_TAKE))
   {
    send_to_char("By the make, you would gather it is meant to be ",ch);
 
  if (CAN_WEAR (obj, ITEM_WEAR_FINGER))
          send_to_char("worn as a ring.\n\r",ch);
  else if (CAN_WEAR(obj,ITEM_WEAR_NECK))
          send_to_char("used as a necklace.\n\r",ch);
  else if (CAN_WEAR(obj,ITEM_WEAR_BODY))
          send_to_char("worn about the torso.\n\r",ch);
  else if (CAN_WEAR(obj,ITEM_WEAR_HEAD))
          send_to_char("worn on the head.\n\r",ch);
  else if (CAN_WEAR(obj,ITEM_WEAR_LEGS))
          send_to_char("used as leggings.\n\r",ch);
  else if (CAN_WEAR(obj,ITEM_WEAR_FEET))
          send_to_char("worn on the feet.\n\r",ch);
  else if (CAN_WEAR(obj,ITEM_WEAR_HANDS))
          send_to_char("worn on your hands.\n\r",ch);
  else if (CAN_WEAR(obj, ITEM_WEAR_ARMS))
          send_to_char("worn on the arms.\n\r",ch);
  else if (CAN_WEAR(obj,ITEM_WEAR_SHIELD))
          send_to_char("used as a shield.\n\r",ch);
  else if (CAN_WEAR(obj,ITEM_WEAR_ABOUT))
          send_to_char("worn about the body.\n\r",ch);
  else if (CAN_WEAR(obj,ITEM_WEAR_WAIST))
          send_to_char("worn about the waist.\n\r",ch);
  else if (CAN_WEAR(obj,ITEM_WEAR_WRIST))
          send_to_char("worn as a bracelet.\n\r",ch);
  else if (CAN_WEAR(obj,ITEM_WEAR_ANKLE))
          send_to_char("worn on your ankle.\n\r",ch);
  else if (CAN_WEAR(obj,ITEM_WEAR_SHOULDERS))
          send_to_char("worn on your shoulders.\n\r",ch);
  else if (CAN_WEAR(obj,ITEM_WEAR_FACE))
          send_to_char("worn on your face.\n\r",ch);
  else if (CAN_WEAR(obj,ITEM_WEAR_MARK))
          send_to_char("worn as a mark.\n\r",ch);
  else if (CAN_WEAR(obj,ITEM_HOLD)) 
          send_to_char("held in your hands.\n\r",ch);
  else if (CAN_WEAR(obj,ITEM_WIELD))
          send_to_char("wielded.\n\r",ch);
  else
          send_to_char("be unwearable.\n\r",ch);
  }
  send_to_char(buf,ch);
  sprintf(buf,"It seems to weigh approximately %d stones.\n\r", obj->weight);
  send_to_char(buf,ch);
  sprintf(buf,"You would estimate it's value to be about %d gold pieces.\n\r", obj->cost);
  send_to_char(buf,ch);

  if(ch->class == class_lookup("blacksmith"))
  {
	sprintf(buf, "That it is in %d%c condition, and made of a %s material.\n\r",
	   obj->condition, '\x25', obj->material);
   	send_to_char(buf,ch);
  }

  return;
}

void do_fremove( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj_next;
    OBJ_DATA *obj;

         argument = one_argument( argument, arg1 );

 if ( arg1[0] == '\0' )
    {
        send_to_char( "Syntax: fremove <char>\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
        send_to_char( "That player is not here.\n\r", ch);
        return;
    }

   if ( victim->level > get_trust( ch) )
    {
        send_to_char( "Limited to your trust level.\n\r", ch );
        return;
    }

  for (obj = victim->carrying; obj; obj = obj_next)
    {
    obj_next = obj->next_content;

    if (obj->wear_loc != WEAR_NONE)
    remove_obj (victim, obj->wear_loc, TRUE);

    }
save_char_obj(victim);
    return;
}

void do_butcher(CHAR_DATA *ch, char *argument)
{
	 char buf[MAX_STRING_LENGTH];
	 char arg[MAX_STRING_LENGTH];
	 int numst = 0;
	 OBJ_DATA *steak;
	 OBJ_DATA *obj;

	 one_argument(argument, arg);
	 if(get_skill(ch,gsn_butcher)==0)	 {
		 send_to_char("Butchering is beyond your skills.\n\r",ch);
		 return;
	 }

	 if(arg[0]=='\0')
	 {
		 send_to_char("Butcher what?\n\r",ch);
		 return;
	 }

	 obj = get_obj_list( ch, arg, ch->in_room->contents );
	 if ( obj == NULL )
	 {
		  send_to_char( "It's not here.\n\r", ch );
		  return;
	 }

	 if( (obj->item_type != ITEM_CORPSE_NPC)
		  && (obj->item_type!=ITEM_CORPSE_PC) )
	 {
		  send_to_char( "You can only butcher corpses.\n\r", ch );
		  return;
 	 }

	 /* create and rename the steak */
	 buf[0]='\0';
	 strcat(buf,"A steak of ");
	 strcat(buf,str_dup(obj->short_descr));
	 strcat(buf," is here.");
	 steak = create_object( get_obj_index( OBJ_VNUM_STEAK ), 0 );
	 steak->description=str_dup(buf);
	 steak->value[0] = ch->level / 2;
	 steak->value[1] = ch->level;
	 buf[0]='\0';
	 strcat(buf,"A steak of ");
	 strcat(buf,str_dup(obj->short_descr));
	 steak->short_descr=str_dup(buf);

	 /* Check the skill roll, and put a random ammount of steaks here. */
	 if(number_percent( ) < get_skill(ch,gsn_butcher))
	 {
		 numst = dice(1,4);
		 switch(numst)
		 {
		 case 1:

			steak = create_object( get_obj_index( OBJ_VNUM_STEAK ), 0 );
			obj_to_room( steak, ch->in_room );
			act( "$n butchers a corpse and creates a steak.\n\r", ch, steak, NULL, TO_ROOM );
			act( "You butcher a corpse and create a steak.\n\r",  ch, steak, NULL, TO_CHAR );
			break;

		 case 2:
			steak = create_object( get_obj_index( OBJ_VNUM_STEAK ), 0 );
			obj_to_room( steak, ch->in_room );
			steak = create_object( get_obj_index( OBJ_VNUM_STEAK ), 0 );
			obj_to_room( steak, ch->in_room );
			act( "$n butchers a corpse and creates two steaks.\n\r", ch, steak, NULL, TO_ROOM );
			act( "You butcher a corpse and create two steaks.\n\r",  ch, steak, NULL, TO_CHAR );
			break;

		 case 3:
			steak = create_object( get_obj_index( OBJ_VNUM_STEAK ), 0 );
			obj_to_room( steak, ch->in_room );
			steak = create_object( get_obj_index( OBJ_VNUM_STEAK ), 0 );
			obj_to_room( steak, ch->in_room );
			steak = create_object( get_obj_index( OBJ_VNUM_STEAK ), 0 );
			obj_to_room( steak, ch->in_room );
			act( "$n butchers a corpse and creates three steaks.\n\r", ch, steak, NULL, TO_ROOM );
			act( "You butcher a corpse and create three steaks.\n\r",  ch, steak, NULL, TO_CHAR );
			break;

		 case 4:
			steak = create_object( get_obj_index( OBJ_VNUM_STEAK ), 0 );			obj_to_room( steak, ch->in_room );
			steak = create_object( get_obj_index( OBJ_VNUM_STEAK ), 0 );
			obj_to_room( steak, ch->in_room );
			steak = create_object( get_obj_index( OBJ_VNUM_STEAK ), 0 );
			obj_to_room( steak, ch->in_room );
			steak = create_object( get_obj_index( OBJ_VNUM_STEAK ), 0 );
			obj_to_room( steak, ch->in_room );
			act( "$n butchers a corpse and creates four steaks.\n\r", ch, steak, NULL, TO_ROOM );
			act( "You butcher a corpse and create four steaks.\n\r",  ch, steak, NULL, TO_CHAR );
			break;

		}
		check_improve(ch,gsn_butcher,TRUE,1);
	 }
	 else
	 {
		 act( "$n fails to butcher a corpse, and destroys it.\n\r", ch, steak, NULL, TO_ROOM );
		 act( "You fail to butcher a corpse, and destroy it.\n\r",   ch, steak, NULL, TO_CHAR );
		 check_improve(ch,gsn_butcher,FALSE,1);
	 }

	 /* dump items caried */
	 /* Taken from the original ROM code and added into here. */
	 if ( obj->item_type == ITEM_CORPSE_PC )
	 {   /* save the contents */
		 {
				OBJ_DATA *t_obj, *next_obj;
				for (t_obj = obj->contains; t_obj != NULL; t_obj = next_obj)
				{
					 next_obj = t_obj->next_content;
					 obj_from_obj(t_obj);
					 if (obj->in_obj) /* in another object */
						  obj_to_obj(t_obj,obj->in_obj);
					 else if (obj->carried_by) /* carried */
						  if (obj->wear_loc == WEAR_FLOAT)
								if (obj->carried_by->in_room == NULL)
									 extract_obj(t_obj);
								else
									 obj_to_room(t_obj,obj->carried_by->in_room);
						  else
								obj_to_char(t_obj,obj->carried_by);
					else if (obj->in_room == NULL) /* destroy it */
						  extract_obj(t_obj);
					 else /* to a room */
						  obj_to_room(t_obj,obj->in_room);
			  }
		}
  }
	 if ( obj->item_type == ITEM_CORPSE_NPC )
	 {
		 {
				OBJ_DATA *t_obj, *next_obj;
				for (t_obj = obj->contains; t_obj != NULL; t_obj = next_obj)
				{
					 next_obj = t_obj->next_content;
					 obj_from_obj(t_obj);
					 if (obj->in_obj) /* in another object */
						  obj_to_obj(t_obj,obj->in_obj);
					 else if (obj->carried_by) /* carried */
						  if (obj->wear_loc == WEAR_FLOAT)
								if (obj->carried_by->in_room == NULL)
									 extract_obj(t_obj);
								else
									obj_to_room(t_obj,obj->carried_by->in_room);
						  else
								obj_to_char(t_obj,obj->carried_by);
					 else if (obj->in_room == NULL) /* destroy it */
						  extract_obj(t_obj);
					 else /* to a room */
						  obj_to_room(t_obj,obj->in_room);
			}
	  }
  }
	 /* Now remove the corpse */
	 extract_obj(obj);
	 return;
}

void do_donate( CHAR_DATA *ch, char *argument)
{
   OBJ_DATA *pit = NULL;
   OBJ_DATA *hometownpit;
   OBJ_DATA *obj;
   OBJ_DATA *obj1;
   OBJ_DATA *obj1_next;
   ROOM_INDEX_DATA *original;
   char arg[MAX_INPUT_LENGTH];
   int count;
   
   argument = one_argument(argument, arg);


   if (arg[0] == '\0' )
   {
      send_to_char("Donate what?\n\r",ch);
      return;
   }

   original = ch->in_room;
   if (ch->position == POS_FIGHTING)
   {
      send_to_char(" You're fighting!\n\r",ch);
      return;
   }

   if ( (obj = get_obj_carry (ch, arg, ch)) == NULL)
   {
      send_to_char("You do not have that!\n\r",ch);
      return;
   }
   else
   {
      if (!can_drop_obj(ch, obj) && ch->level < 91)
      {
         send_to_char("Its stuck to you.\n\r",ch);
         return;
      }
      if ((obj->item_type == ITEM_CORPSE_NPC) ||
         (obj->item_type == ITEM_CORPSE_PC))
      {
         send_to_char("You cannot donate that!\n\r",ch);
         return;
      }
      if (obj->timer > 0)
      {
         send_to_char("You cannot donate that.\n\r",ch);
         return;
      }

	for(hometownpit = object_list; hometownpit != NULL; hometownpit = hometownpit->next)
	{
		if( hometownpit->pIndexData->vnum != hometown_table[ch->hometown].pit_vnum )
			continue;

		if(hometownpit->in_room->vnum == 0) 
			continue;

		pit = hometownpit;
		break;
	}

      count = 0;
      if (( hometownpit->pIndexData->vnum == OBJ_VNUM_PIT 
      ||    hometownpit->pIndexData->vnum == hometown_table[ch->hometown].pit_vnum))
      {
        for ( obj1 = hometownpit->contains; obj1 != NULL; obj1 = obj1_next )
        {
          obj1_next = obj1->next_content;
          if(obj1->pIndexData->vnum == obj->pIndexData->vnum)
          count++;

          if ( count > 9 )
          {
            send_to_char( "You can't donate any more of those items into the pit!\n\r", ch );
            return;
          }
        }
      }

      if (ch->in_room != get_room_index(ROOM_VNUM_ALTAR))
         act("$n donates $p.",ch,obj,NULL,TO_ROOM);
      act("You donate $p.",ch,obj,NULL,TO_CHAR);

      char_from_room(ch);
      char_to_room(ch, get_room_index(hometownpit->in_room->vnum) );
	pit = get_obj_here(NULL, ch->in_room, hometownpit->name);

	if(pit == NULL)
	{
		pit = create_object( get_obj_index(hometown_table[ch->hometown].pit_vnum), 0 );
		obj_to_room(pit, ch->in_room);
	}

      obj_from_char(obj);
      obj_to_obj(obj, pit);
      char_from_room(ch);
      char_to_room(ch,original);
      return;
   }   
}

void do_dual_wield (CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;
    OBJ_DATA *obj2;
    char buf[MAX_STRING_LENGTH];

    if (    !IS_NPC( ch ) 
    && ch->level < skill_table[gsn_dual_wield].skill_level[ch->class] )
    {
      send_to_char( "You don't know how to wield two weapons at the same time.\n\r", ch );
      return;
    }

    if (argument[0] == '\0') /* empty */
    {
        send_to_char ("Wear which weapon in your off-hand?\n\r",ch);
        return;
    }

    obj = get_obj_carry (ch, argument, ch);

    if (obj == NULL)
    {
        send_to_char ("You have no such weapon.\n\r",ch);
        return;
    }

    if (get_eq_char (ch,WEAR_SHIELD) != NULL)
    {
        send_to_char ("You cannot use a secondary weapon while using a shield.\n\r",ch);
        return;
    }

    if( get_eq_char (ch,WEAR_HOLD) != NULL)
    {
	send_to_char("You cannot use another weapon while holding something in your hands.\n\r",ch);
	return;
    }

    if  (IS_WEAPON_STAT(obj,WEAPON_TWO_HANDS))
    {
	  send_to_char("This weapon requires two hands, hence it cannot be dual wielded!\n\r",ch);
	  return;
    }

    if (obj->item_type != ITEM_WEAPON )
    {
    send_to_char("You can only dual wield weapons.\n\r",ch);
    return;
    }

    if ( ch->level < obj->level )
    {
        sprintf( buf, "You must be level %d to use this object.\n\r",
            obj->level );
        send_to_char( buf, ch );
        act( "$n tries to use $p, but is too inexperienced.",
            ch, obj, NULL, TO_ROOM );
        return;
    }

    /*Characters can only dual wield the same size */
    if ( ((obj->size > (ch->size))
    || (obj->size < (ch->size))) 
    && (obj->size != SIZE_IGNORE)
    && !IS_IMMORTAL(ch))  
    {
      if( obj->size > ch->size )
	  send_to_char("It is way too large for you to properly wield.\n\r", ch);
      else
	  send_to_char("It is way too small for you to properly wield.\n\r", ch);
      return;
    }

    if (get_eq_char (ch, WEAR_WIELD) == NULL)
    {
        send_to_char ("You need to wield a primary weapon, before using a secondary one!\n\r",ch);
        return;
    }

    obj2 = get_eq_char(ch,WEAR_WIELD);
    if( (obj->value[0] != WEAPON_DAGGER
    || obj2->value[0] != WEAPON_DAGGER)
    && (ch->class == class_lookup("thief"))) 
    {
       send_to_char("You can only dual wield two daggers.\n\r",ch);
       return;
    }

    if (obj->value[0]!=WEAPON_DAGGER || obj2->value[0]!=WEAPON_DAGGER)
    if ( (get_obj_weight (obj)*2) > get_obj_weight(get_eq_char(ch,WEAR_WIELD)))
    {
        send_to_char ("Your secondary weapon has to be considerably lighter than the primary one.\n\r",ch);
        return;
    }

	if (obj2 != NULL && ch->size < SIZE_LARGE 
	&&  IS_WEAPON_STAT(obj2,WEAPON_TWO_HANDS))
	{
	    send_to_char("Your hands are tied up with your primary weapon!\n\r",ch);
	    return;
	}

    if (!remove_obj(ch, WEAR_SECONDARY, TRUE))
        return;
   {
    act ("$n wields $p in $s off-hand.",ch,obj,NULL,TO_ROOM);
    act ("You wield $p in your off-hand.",ch,obj,NULL,TO_CHAR);
    equip_char ( ch, obj, WEAR_SECONDARY);
    return;
   }
}

/* Hiddukel - steal items from conatiners */
void do_pickpocket( CHAR_DATA *ch, char *argument )
{
    char buf [MAX_STRING_LENGTH]; 
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *tmp_ch;
    OBJ_DATA *obj;
    OBJ_DATA *item;
    OBJ_DATA *item_next;
    int bonus; 
    int percent;

    argument = one_argument( argument, arg1);
    argument = one_argument( argument, arg2);

    if IS_NPC(ch)
	{
	send_to_char("This feature is unavailable in switched mode.\n\r", ch);
	return;
	}

    if (!IS_NPC(ch)
    && ch->level < skill_table[gsn_pickpocket].skill_level[ch->class] )
    {
       send_to_char("Hmm?\n\r",ch);
       return;
    }

    if (arg1[0] == '\0' || arg2[0] == '\0' )
    {
        send_to_char("Pickpocket what from whom?\n\r",ch);
        return; 
    }

    if (ch->riding)
    {
      send_to_char("You can't do that while mounted.\n\r",ch);
      return;
    }
    if ( ( victim = get_char_room( ch, NULL, arg2 ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }
    if ( victim == ch )
    {
        send_to_char( "That's pointless.\n\r", ch );
        return;
    }

    if (ch->carry_number >= can_carry_n(ch))
    {
        send_to_char("Your hands are full.\n\r",ch);
        return;
     }

    if (is_safe(ch,victim))
        return;

    if (victim->position == POS_FIGHTING)
    {
        send_to_char("You'd better not -- you might get hit.\n\r",ch);
        return;
    }

    tmp_ch = (is_affected(ch,gsn_doppelganger) && !IS_IMMORTAL(victim)) ?
     ch->doppel : ch;
  
   if ((( obj = get_obj_wear(victim, arg1, TRUE)) == NULL)
   && ((  obj = get_obj_carry(victim,arg1,ch)) == NULL))
      {
              send_to_char("You can't find it.\n\r",ch);
              return;
       }

     switch (obj->item_type)
     {
       case ITEM_CONTAINER:
       case ITEM_CORPSE_NPC:
       case ITEM_CORPSE_PC:
           break;
       default:
            send_to_char("That's not a container.\n\r",ch);
            return;
      } 
 
   WAIT_STATE(ch,skill_table[gsn_pickpocket].beats);

   bonus = (get_curr_stat(ch,STAT_DEX) - 21) * 2;

   if (victim->position <= POS_SLEEPING)
       bonus =+ 5;

   if (IS_AFFECTED(victim,AFF_SLEEP))
       bonus =+ 100;

   if ( IS_SET(obj->value[1], CONT_CLOSED))
	bonus =- 40;
 
   percent = number_percent() - bonus;

   if (percent > ch->pcdata->learned[gsn_pickpocket]
	|| (victim->level > LEVEL_HERO && ch->level <= LEVEL_HERO))
   {
         send_to_char("Oops!\n\r",ch);
         if (IS_AFFECTED(ch,AFF_HIDE))
           {
            REMOVE_BIT( ch->affected_by, AFF_HIDE);
            act("You leave the shadows.",ch,NULL,NULL,TO_CHAR);
            act("$n leaves the shadows.",ch,NULL,NULL,TO_ROOM);
           } 
	 if (can_see(victim, ch))
             act("$n tried to pickpocket you.",ch,NULL,victim,TO_VICT);
	 else
             act("Someone tried to pickpocket you.",ch,NULL,victim,TO_VICT);

         if (!IS_AFFECTED(victim,AFF_SLEEP) 
         && (victim->position == POS_SLEEPING))
             victim->position = POS_STANDING;

         if (victim->position != POS_SLEEPING
	|| (!IS_NPC(victim)))
         {
	   if (can_see(victim, ch)) {
             sprintf(buf,"%s tried to pickpocket me!",tmp_ch->name);
             do_yell(victim,buf);
	   }
	   else {
             sprintf(buf,"Someone tried to pickpocket me!");
             do_yell(victim,buf);
           }
         }
         check_improve(ch,gsn_pickpocket,FALSE,1);
      return;
   } 

   if (IS_SET(obj->value[1],CONT_LOCKED))
   {
      act("$P is closed and locked.",ch,NULL,obj,TO_CHAR);
      return;
   } 

   if ( IS_SET(obj->value[1],CONT_CLOSED))
   {
      act("You manage to open $P.",ch,NULL,obj,TO_CHAR);      
      REMOVE_BIT(obj->value[1],CONT_CLOSED);
      return;
   }

   if (obj->contains == NULL)
   {
       act("You find nothing in $P.",ch,NULL,obj,TO_CHAR);
       return;
    } 

   for ( item = obj->contains; item != NULL; item = item_next)
   {

       if ( ch->carry_weight + get_obj_weight( item ) > can_carry_w( ch ))
       {
          send_to_char("You cannot carry that much weight.\n\r",ch);
          return;
        }

       obj_from_obj(item);
       obj_to_char(item,ch); 
       act("You get $p from $P.",ch,item,obj,TO_CHAR); 
       check_improve(ch,gsn_pickpocket,TRUE,1);
       return; 
    }

return;
}

void do_campfire( CHAR_DATA *ch, char *argument )
{

   OBJ_DATA *obj;
   OBJ_DATA *campfire; 
   int chance;

   if (ch->riding)
   {
      send_to_char("You can't do that while mounted.\n\r",ch);
      return;
   }

   if (ch->in_room->contents != NULL)    
   for (obj = ch->in_room->contents; obj != NULL; obj = obj->next_content)
   {
      if (obj->pIndexData->vnum == OBJ_VNUM_CAMPFIRE)
      {
         send_to_char("You rekindle the campfire.\n\r",ch);
         act("$n rekindles the campfire.\n\r",ch,NULL,NULL,TO_ROOM);
         obj->timer = 5;
         return;
       }
   }

   if ( (chance = get_skill(ch,gsn_campfire)) == 0
   || (!IS_NPC(ch)
   && ch->level < skill_table[gsn_campfire].skill_level[ch->class]))
   {
        send_to_char("The skill of making campfires is unkown to you.\n\r",ch);
        return;
   }

   switch(ch->in_room->sector_type)
   {
    case (SECT_FOREST):
    case (SECT_HILLS) :
    case (SECT_MOUNTAIN):
    case (SECT_FIELD) :
    {
      act("$n gathers wood and makes a campfire.",ch,NULL,ch,TO_ROOM);
      send_to_char("You forage for some wood then start a small campfire.\n\r",ch);
      break;
    }
   default:
   {
     send_to_char("You are unable to start a fire here.\n\r",ch);
     return;
   }
  }


  if ( number_percent( ) < ch->pcdata->learned[gsn_campfire] )
     check_improve(ch,gsn_campfire,TRUE,1);
  else
  {
    check_improve(ch,gsn_campfire,FALSE,1);
    send_to_char("You fail to start a fire.\n\r",ch);
    act("$n fails to start a fire.",ch,NULL,ch,TO_ROOM);
    return;
  }


  campfire = create_object( get_obj_index(OBJ_VNUM_CAMPFIRE),0);
  campfire->timer = 5;
  obj_to_room(campfire,ch->in_room);


return;
}

void do_search_water( CHAR_DATA *ch, char *argument )
{

   OBJ_DATA *obj;
   OBJ_DATA *spring; 
   int chance;

   if (ch->riding)
   {
      send_to_char("You might want to step off of the mount before you do that.\n\r",ch);
      return;
   }

   if (ch->in_room->contents != NULL)    
   for (obj = ch->in_room->contents; obj != NULL; obj = obj->next_content)
   {
      if (obj->pIndexData->vnum == OBJ_VNUM_SPRING)
      {
         send_to_char("There is already a spring here.\n\r",ch);
         return;
       }
   }
   if ( (chance = get_skill(ch,gsn_search_water)) == 0
   || (!IS_NPC(ch)
   && ch->level < skill_table[gsn_search_water].skill_level[ch->class]))
   {
        send_to_char("You  should consider yourself lucky to find the fountain in the city on occasion.\n\r",ch);
        return;
   }

   switch(ch->in_room->sector_type)
   {
    case (SECT_FOREST):
    case (SECT_HILLS) :
    case (SECT_MOUNTAIN):
    case (SECT_FIELD) :
    {
      act("$n begins searching for a natural spring.",ch,NULL,ch,TO_ROOM);
      send_to_char("You search the area, in hopes of finding a spring to quench your thirst.\n\r",ch);
      act("$n digs a small hole with $s hands as a small spring bubbles forth from the ground.",ch,NULL,ch,TO_ROOM);
      send_to_char("You dig a small hole in the ground, allowing a natural spring to flow.\n\r",ch);
      break;
    }
   default:
   {
     send_to_char("You are not going to find a spring here.\n\r",ch);
     return;
   }
  }


  if ( number_percent( ) < ch->pcdata->learned[gsn_search_water] )
     check_improve(ch,gsn_search_water,TRUE,1);
  else
  {
    check_improve(ch,gsn_search_water,FALSE,1);
    send_to_char("You search for a spring only to find nothing.\n\r",ch);
    act("$n fails to find a natural spring in the surrounding area.",ch,NULL,ch,TO_ROOM);
    return;
  }


  spring = create_object( get_obj_index(OBJ_VNUM_SPRING),0);
  spring->timer = 15;
  spring->value[0]=50;
  spring->value[1]=50;
  spring->extra_flags = 0;
  free_string(spring->name);
  spring->name=str_dup("spring");
  free_string(spring->short_descr);
  spring->short_descr=str_dup("a natural sparkling spring");
  free_string(spring->description);
  spring->description=str_dup("A small natural spring flows forth from the hole dug into the ground.");
  obj_to_room(spring,ch->in_room);


return;
}

void do_dreamevent( CHAR_DATA *ch, char *argument )
{
 int dreamevent;
 int number;

 if (!IS_NPC(ch) && ch->position == POS_SLEEPING)
 {
   dreamevent = dice(1,40);

    if (dreamevent == 1)
    {
    send_to_char("...ZZzzzzZZzzZzzZZZzzZZZzzZZZzZzzZzZZzzzZZzzzZZzzZZ\n\r",ch);
    return;
    }
    else if (dreamevent == 2)
    {
    send_to_char("ZZzzzzZZzzZzzZZZzzZZZzzZZZzZZZZzzzZZzzzZZzzZZ....\n\r",ch);
    return;
    }
    else if (dreamevent == 3)
    {
    send_to_char("ZZzzzzZZzzZzzzzZZZzzzZZZzzZzZZzZZzZZzzzZZzzzZZzzZZ....\n\r",ch);
    return;
    }
    else if (dreamevent == 4)
    {
    send_to_char("ZZzzzzZZzzZzzZZZzzZZZzzZZZzZZZZzzzZZzzzZZzzZZ....\n\r",ch);
    return;
    }
    else if (dreamevent == 5)
    {
    send_to_char("ZZzzzzZZzzZzzZZZzzZZZzzZZZzZZZZzzzZZzzzZZzzZZ....\n\r",ch);
    return;
    }
    else if (dreamevent == 6)
    {
    send_to_char("ZZzzzzZZzzZzzZZZzzZZZzzZZZzZZZZzzzZZzzzZZzzZZ....\n\r",ch);
    return;
    }
    else if (dreamevent == 7)
    {
    send_to_char("You're walking on a green path in the woods.. which woods?..\n\r",ch);
    send_to_char("Nothing looks familiar to you, yet this image is clear.\n\r",ch);
    send_to_char("Wherever you are, you must get out. they're watching you....\n\r",ch);
    send_to_char("They know who you are, and know where you've come from.\n\r",ch);
    send_to_char("You hear a voice and it's telling you to be careful and prepare.\n\r",ch);
    send_to_char("But prepare for what???....\n\r",ch);
    return;
    }
    else if (dreamevent == 8)
    {
    send_to_char("ZZzzzzZZzzZzzZZZzzZZZzzZZZzZZZZzzzZZzzzZZzzZZ....\n\r",ch);
    send_to_char("You sure snore a lot.\n\r",ch);
    act( "$n snores loudly .. how rude.\n\r", ch, NULL, NULL,TO_ROOM);
    return;
    }
    else if (dreamevent == 9)
    {
    send_to_char("You are awoken from sleep when you..\n\r",ch);
    send_to_char("notice there is a sharp stick protruding from the ground\n\r",ch);
    send_to_char("and it is jabbing you. Ouch..\n\r",ch);
    send_to_char("You go back to sleep.\n\r",ch);
    act( "$n shouts 'Ouch' and pulls a stick out from under $m.\n\r", ch, NULL, NULL,TO_ROOM);
    act( "$n curses bitterly, and goes back to sleep.\n\r", ch, NULL, NULL,TO_ROOM);
    return;
    }
    else if (dreamevent == 10)
    {
    send_to_char("ZZzzzzZZzzZzzZZZzzZZZzzZZZzZZZZzzzZZzzzZZzzZZ....\n\r",ch);
    }
    else if (dreamevent == 11)
    {
    send_to_char("ZZzzzzZZzzZzzZZZzzZZZzzZZZzZZZZzzzZZzzzZZzzZZ....\n\r",ch);
    send_to_char("You feel cold.\n\r",ch);
    return;
    }
    else if (dreamevent == 12)
    {
    send_to_char("Whoa! You thought you were sleeping, but now you're standing\n\r",ch);
    send_to_char("in the center of a stage! The lights shine down on you,\n\r",ch);
    send_to_char("the crowd cheers as you are standing there dressed in your armor\n\r",ch);
    send_to_char("the crowd settles..awaiting your next move.. you dont know what to do!\n\r",ch);
    send_to_char("an overwelming effect of stage fright washes over you! you panic\n\r",ch);
    send_to_char("risking everyone laughing at you, you start to sing a song..\n\r",ch);
    send_to_char("the crowd starts booing at your performance! you run off the stage..\n\r",ch);
    send_to_char("thank the gods this is only a dream..\n\r",ch);
    return;
    }
    else if (dreamevent == 13)
    {
    send_to_char("Masses are moving in a void. They are flowing though it. They are\n\r",ch);
    send_to_char("closing in on you. You look closer. You start manuvering the masses.\n\r",ch);
    send_to_char("You clench your fist, but it feels smaller than usual. You open it\n\r",ch);
    send_to_char("and reclench it, but it still feels small.. and hard, it feels\n\r",ch);
    send_to_char("like a rock. You manuver the masses with the power of your mind.\n\r",ch);
    send_to_char("It's a game. The masses are closing in. You look in closer. There\n\r",ch);
    send_to_char("must be an escape. They're getting closer...you can't win, you can't.\n\r",ch);
    send_to_char("Your mind is falling away.. you pass into a deeper sleep.\n\r",ch);
    return;
    }
    else if (dreamevent == 14)
    {
    send_to_char("You think about vampires and one appears.\n\r",ch);
    send_to_char("It reaches for you and you scream in terror as\n\r",ch);
    send_to_char("it's fangs bite into your neck!\n\r",ch);
    return;
    }
    else if (dreamevent == 15)
    {
    send_to_char("You are walking in a forest. You don't know where anything is.\n\r",ch);
    send_to_char("You've never been here before, but it seems so familiar to you..\n\r",ch);
    send_to_char("As you're walking, you come accross a pond. You see your reflection..\n\r",ch);
    send_to_char("Or do you?.. It doesnt look like you.. but it has to be..\n\r",ch);
    send_to_char("You feel a bit better about your abilities!\n\r",ch);
    ch->max_hit = ch->max_hit + 10;
    act( "$n glows a strange color momentarily...\n\r", ch, NULL, NULL,TO_ROOM);
    act( "$n passes into a deeper sleep...\n\r", ch, NULL, NULL,TO_ROOM);
    return;
    }
    else if (dreamevent == 16)
    {
    send_to_char("You shudder from the cold as you see a scene of a vast frozen wasteland\n\r",ch);
    send_to_char("you begin to stumble around in the cold snow drifts and frozen ponds\n\r",ch);
    send_to_char("the cold begins to reach your fingers and toes as you begin to loose\n\r",ch);
    send_to_char("feeling in them. All of a sudden, a man dressed in black appears before you.\n\r",ch);
    send_to_char("In one hand he is holding a dagger and in the other...\n\r",ch);
    send_to_char("YOUR HEAD!?!! As you begin to scream it slowly fades away as you feel the frost\n\r",ch);
    send_to_char("from a short blade against your neck.\n\r",ch);
    act( "$n lets out a cry and a bit of blood drips from $s hand!\n\r", ch, NULL, NULL,TO_ROOM);
    act( "Mumbling strange words, $n passes into a deeper sleep.\n\r", ch, NULL, NULL,TO_ROOM);
    return;
    }
    else if (dreamevent == 17)
    {
    send_to_char("Wait, is that pipe shaking?  Yes, it is! All of the sudden\n\r",ch);
    send_to_char("you, hear the shrill scream of alarms! You run as fast as you\n\r",ch);
    send_to_char("can, trying to get out, but it's just too big! Oh, no!\n\r",ch);
    send_to_char("BOOOOOOOOMMMMMMMMMM!!!!!!!!!!!!!!!\n\r",ch);
    send_to_char("You wake up, with grease in your hair.\n\r",ch);
    do_wake(ch,"");
    return;
    }
    else if (dreamevent == 18)
    {
    send_to_char("You find yourself in a labyrinth of pipes and gears. All around you, you\n\r",ch);
    send_to_char("hear clanks, bangs, and the rattle of chains. You smell the oil used to\n\r",ch);
    send_to_char("lubricate the gears.  As you walk through this place, you find it\n\r",ch);
    send_to_char("looks familiar. You wonder why. Then you realize, you've\n\r",ch);
    send_to_char("seen it before! A gnome had been working on the plans a while back!\n\r",ch);
    return;
    }
    else if (dreamevent == 19)
    {
    send_to_char("*snore* *weeze* *cough* *choke*\n\r",ch);
    return;
    }
    else if (dreamevent == 20)
    {
    send_to_char("You hear them comming\n\r",ch);
    send_to_char("They're comming, there's too many of them. What will you do?\n\r",ch);
    send_to_char("Suddenly, they crest the ridge, and you are bowled over by\n\r",ch);
    send_to_char("2000 puppies who want to lick your face.\n\r",ch);
    send_to_char("They're all over. Well, at least they're housebroken.\n\r",ch);
    send_to_char("Aren't they?\n\r",ch);
    return;
    }
    else if (dreamevent == 21)
    {
    send_to_char("Your walking down a grey road, and the scenes all about\n\r",ch);
    send_to_char("you are blue, thats all you see, greyish blue for as far as\n\r",ch);
    send_to_char("the eye can see.\n\r",ch);
    send_to_char("You keep walking down the road, and you come upon a bench,\n\r",ch);
    send_to_char("with 2 people sitting on it reading newspapers, you\n\r",ch);
    send_to_char("notice the papers are close to their faces.\n\r",ch);
    send_to_char("All of a sudden their faces morph into the newspaper, and\n\r",ch);
    send_to_char("they put their hands down. They look at you then start\n\r",ch);
    send_to_char("talking to each other.\n\r",ch);
    send_to_char("You run as fast as you can down the road, till you see a\n\r",ch);
    send_to_char("woman, made of bones. She is taking a walk, with her baby in a cart\n\r",ch);
    send_to_char("she is pushing along. You decide to look closer, so you\n\r",ch);
    send_to_char("walk over to the cart made of bones...\n\r",ch);
    send_to_char("You look inside and the baby screams so loud it knocks\n\r",ch);
    send_to_char("you back a few feet.\n\r",ch);
    send_to_char("You run as fast as you can till you come to a cliff.\n\r",ch);
    return;
    }
    else if (dreamevent == 22)
    {
    send_to_char("You're resting peacefully, dreaming about beautiful green fields,\n\rfull of colorful flowers.. the ",ch);
    send_to_char("Sun shines brightly overhead,\n\rwhen an odd sensation awakes you.\n\r",ch);
    send_to_char("You wake up to feel an odd burning sensation on your left hand, and look down to\n\r",ch);
    send_to_char("find your hand literally on fire! Bright flames and greasy orange smoke\n\r",ch);
    send_to_char("rise from the palm of your\n\r",ch);
    send_to_char("hand, as you sit helplessly and watch the skin drip and crackle,\n\r",ch);
    send_to_char("peeling away from the bones as you begin to scream in agony..\n\r",ch);
    send_to_char("You sit bolt upright in bed, rocked to the core of your being,\n\ras you realize this was only.. a ",ch);
    send_to_char("dream..?\n\r",ch);
    return;
    }
    else if (dreamevent == 23)
    {
    send_to_char("You are being led down a corridor.\n\r",ch);
    send_to_char("You are completely shackled and each arm is fiercely gripped by a decaying\n\r",ch);
    send_to_char("zombie...They force you into a large dark throne room. Before you sits\n\r",ch);
    send_to_char("Chemosh on a throne of chipped bone and patchworked flesh. He is in deep\n\r",ch);
    send_to_char("conversation with Morgion, who stands to his left. Morgion leans over and\n\r",ch);
    send_to_char("whispers something into Chemosh's ear, and Chemosh turns to glare at you. He\n\r",ch);
    send_to_char("extends his arm, makes a fist, and points his thumb down to the floor.\n\r",ch);
    send_to_char("\n\r",ch);
    send_to_char("Excruciating pain fills your entire body, and you look down to see your\n\r",ch);
    send_to_char("skin dissolve into dust. You open your mouth, and a scream of hopelessness\n\r",ch);
    send_to_char("escapes........\n\r",ch);
    send_to_char("\n\r",ch);
    send_to_char("All goes dark.......\n\r",ch);
    act( "$n rolls around frantically appearing to be in pain!!\n\r", ch, NULL, NULL,TO_ROOM);
    act( "$n screams over and over... \n\r No!! It hurts!! Make it stop!!!\"\n\r", ch, NULL, NULL,TO_ROOM);
    return;
    }
    else if (dreamevent == 24)
    {
    send_to_char("You hear flutes in the distance. As the fog clears you realize that you\n\r",ch);
    send_to_char("are standing in a quaint village filled with small happy fauns. A small\n\r",ch);
    send_to_char("male approaches you, smiles, puts a pipe to his lips, and begins to play\n\r",ch);
    send_to_char("a tune. He walks away - and you begin to follow. At first you are enchanted\n\r",ch);
    send_to_char("...but you slowly come to the realization you can't stop! You struggle\n\r",ch);
    send_to_char("from the sound...throwing your hands over your ears...but it solves\n\r",ch);
    send_to_char("nothing. He leads you away from the village into a dark, ominous forest.\n\r",ch);
    send_to_char("The sound begins to drill into your eardrums...then your brain...\n\r",ch);
    send_to_char("You awake with a start!\n\r",ch);
    do_wake(ch,"");
    return;
    }
    else if (dreamevent == 25)
    {
    send_to_char("You feel a soft brush at your neck, and ignore it.\n\r",ch);
    send_to_char("Then you feel the fangs PIERCE your skin and your life blood being taken!\n\r",ch);
    send_to_char("\n\r",ch);
    send_to_char("You sit bolt upright, your heart pounding!\n\r",ch);
    send_to_char("You look about but see no one near you.\n\r",ch);
    send_to_char("You calm yourself and drift back to sleep, realizing it was only a dream.\n\r",ch);
    send_to_char("Or was it...........?\n\r",ch);
    ch->hit= ch->hit - 5;
    act( "$n's neck is bleeding!!\n\r", ch, NULL, NULL,TO_ROOM);
    act( "$n rolls around and rubs the blood on $s hands..\n\r", ch, NULL, NULL,TO_ROOM);
    return;
    }
    else if (dreamevent == 26)
    {
    send_to_char("You start to wake up, but then fall back to sleep.\n\r",ch);
    send_to_char("zzzZzzzZZZZzzZZ...\n\r",ch);
    return;
    }
    else if (dreamevent == 27)
    {
    send_to_char("A bright beam catches you on the shoulder and you\n\rscream in pain as ",ch);
    send_to_char("your flesh rips open. You fumble with your sword and \n\rtry to draw it out of it's sheath.\n\r",ch);
    send_to_char("Still unable to get the sword out, you throw it\n\rto the ground in ",ch);
    send_to_char("disgust. Holding your injured shoulder with one hand,\n\r you reach with the ",ch);
    send_to_char("other into your pack, pulling out a small blinking sphere.\n\r",ch);
    send_to_char("You close your eyes and say a silent good-bye to your fleet, only\n\r",ch);
    send_to_char("praying they made it out in time.  You can hear the strange foreign\n\r",ch);
    send_to_char("shouts of the Draconians coming up quickly, and you shudder in fear.\n\r",ch);
    send_to_char("Soon they are nearing the gate you're in and you know it is time. You\n\r",ch);
    send_to_char("push a glowing button on the sphere and it begins to beep loudly.\n\r",ch);
    send_to_char("'10 seconds until utter destruction' a voice intones.\n\r",ch);
    send_to_char("You clip the sphere onto the wall as the Draconians enter screaming.\n\r",ch);
    send_to_char("There is a sudden white light and then nothing....\n\r",ch);
    return;
    }
    else if (dreamevent == 28)
    {
    send_to_char("A hand grabs your shoulder and a grizzled old man\n\r",ch);
    send_to_char("peers at your wounds.\n\r",ch);
    send_to_char("'Hmmmm, let me see.'\n\r",ch);
    send_to_char("He digs in his pouch and pulls out a smelly potion.\n\r",ch);
    send_to_char("'Here, drink this.'\n\r",ch);
    send_to_char("As you raise yourself from your slumbers, he disappears.\n\r",ch);
    send_to_char("All that remains is a vial that reeks even from arms length.\n\r",ch);
    return;
    }
    else if (dreamevent == 29)
    {
    send_to_char("You feel yourself floating...\n\r",ch);
    send_to_char("as you are floating around you come across a tree.\n\r",ch);
    send_to_char("you grasp for the branch and hold on tight, you pull yourself\n\r",ch);
    send_to_char("down to the ground. once your feet are firmly pressed against the ground\n\r",ch);
    send_to_char("you take a piece of fruit from the tree... it is a",ch);
    number = number_range(0,3);

    switch(number) 
     {
     case (0) :
     send_to_char(" Mystical Berry.\n\r",ch);
     send_to_char("You eat the Mystical Berry and you feel your power has increased!\n\r",ch);
     ch->max_mana = ch->max_mana + 90;
     act( "$n glows a strange color momentarily...\n\r", ch, NULL, NULL,TO_ROOM);
     act( "$n passes into a deeper sleep...\n\r", ch, NULL, NULL,TO_ROOM);
     return;

     case (1) :
     send_to_char("n orange.\n\r",ch);
     send_to_char("You eat the orange and feel rejuvinated!\n\r",ch);
     ch->hit = ch->hit + 60;
     act( "$n glows a strange color momentarily...\n\r", ch, NULL, NULL,TO_ROOM);
     act( "$n passes into a deeper sleep...\n\r", ch, NULL, NULL,TO_ROOM);
     return;

     case (2) :
     send_to_char("n apple.\n\r",ch);
     send_to_char("You eat the apple and notice you can take more damage than before!!\n\r",ch);
     ch->max_hit = ch->max_hit + 30;
     act( "$n glows a strange color momentarily...\n\r", ch, NULL, NULL,TO_ROOM);
     act( "$n passes into a deeper sleep...\n\r", ch, NULL, NULL,TO_ROOM);
     return;

     case (3) :
     send_to_char(" cherry.\n\r",ch);
     send_to_char("You eat the cherry and notice it has a sweet taste, you reach for\n\r",ch);
     send_to_char("another...\n\r",ch);
     return;
	} 
    }
    else if (dreamevent == 30)
    {
    send_to_char("Ahhhh...  Asleep at last, it's been a long time since you've had a good\n\r",ch);
    send_to_char("night's sleep, and you figure that tonight should be different.. Or should\n\r",ch);
    send_to_char("it!  All of a sudden you feel a burst of energy run through your body, and\n\r",ch);
    send_to_char("you break out in a cold sweat!  You feel as if someone or some THING is\n\r",ch);
    send_to_char("pulling your body, down, down, down, your pulled until your at the center\n\r",ch);
    send_to_char("of the earth.  Then a deep voice speaks, 'I have been waiting your arrival'\n\r",ch);
    send_to_char("it booms. Looking around franticlly you see a dark figure with horns dressed\n\r",ch);
    send_to_char("in a blood-stained cloak. 'You have been very bad...' it continues 'for this\n\r",ch);
    send_to_char("I will punish you!' With that you feel a draining of strength and you fall\n\r",ch);
    send_to_char("to the ground.  Trying to get up you realize that you can't move! Then the\n\r",ch);
    send_to_char("voice booms 'And now, you DIE!'  ZZZZZZZZZZaaaaaaaaaaaaappppppp! A bolt \n\r",ch);
    send_to_char("lightning hits your body...\n\r",ch);
    send_to_char("\n\r",ch);
    send_to_char("\n\r",ch);
    send_to_char("                                  Everything falls silent.\n\r",ch);
    number = number_range(0,4);

    switch(number) 
     {
     case (0) :
     act( "A bolt of {WLIGHTNING{x falls from the sky and stikes $n, $n is unharmed.", ch, NULL, NULL, TO_ROOM);
     return;

     case (1) :
     ch->hit = ch->hit * 0.95;
     act( "A bolt of {WLIGHTNING{x falls from the sky and stikes $n, doing little or no damage.\n\r", ch, NULL, NULL, TO_ROOM);
     return;

     case (2) :
     ch->hit = ch->hit * 0.70;
     act( "A bolt of {WLIGHTNING{x falls from the sky and stikes $n causing a fair bit of damage!!", ch, NULL, NULL, TO_ROOM);
     return;

     case (3) :
     ch->hit = ch->hit * 0.50;
     act( "A bolt of {WLIGHTNING{x falls from the sky and stikes $n causing major damage!!!", ch, NULL, NULL, TO_ROOM);
     return;

     case (4) :
     ch->hit = ch->hit * 0.40;
     act( "A bolt of {WLIGHTNING{x falls from the sky and stikes $n causing critical damage!!!!", ch, NULL, NULL, TO_ROOM);
     return;
    }
   }
    else if (dreamevent == 31)
    {
    send_to_char("You are walking down the street on a bright and sunny morning.\n\r",ch);
    send_to_char("People are laughing and smiling happily as you walk by and you smile in\n\r",ch);
    send_to_char("return.  It sure is nice to see people happy for once.\n\r",ch);
    send_to_char("You continue your walk down the street and stop to look in a store window\n\r",ch);
    send_to_char("at beautiful dishes. Looking more closely in the window you gasp in horror.\n\r",ch);
    send_to_char("YOU FORGOT TO PUT YOUR CLOTHES ON THIS MORNING!!!!\n\r",ch);
    send_to_char("You turn deep red from embarassment and hide behind a bush.\n\r",ch);
    return;
    }
    else if (dreamevent == 32)
    {
    send_to_char("Creak...\n\r",ch);
    send_to_char("Someone ... or something is approaching.\n\r",ch);
    send_to_char("You struggle to awaken as you hear the whisper of a sword being drawn.\n\r",ch);
    send_to_char("Red eyes gleem evilly at you as you frantically roll out of bed reaching \n\r",ch);
    send_to_char("in vain for your weapon.\n\r",ch);
    send_to_char("Swish.\n\r",ch);
    send_to_char("Darkness.\n\r",ch);
    return;
    }
    else if (dreamevent == 33)
    {
    send_to_char("A warm incredible feeling of happiness washes over you.\n\r",ch);
    send_to_char("It feels like you are floating on a bed of air and a \n\r",ch);
    send_to_char("thousand butterflies are fluttering over you gently cooling\n\r",ch);
    send_to_char("you with the beating of their wings.  A far off harp\n\r",ch);
    send_to_char("peacefully fills the air with soul-stirring music.  As you\n\r",ch);
    send_to_char("open your eyes, you realize the sweet aroma filling the air comes\n\r",ch);
    send_to_char("from the hundreds of flowers on and around you with still\n\r",ch);
    send_to_char("more floating down from the sky above.  You close your eyes\n\r",ch);
    send_to_char("hoping to retain these feelings forever.\n\r",ch);
    act( "The air is filled with sweetness as hundreds of flower petals float\n\rdown from the sky to cover $n as $e lies sleeping.", ch, NULL, NULL, TO_ROOM);
    return;
    }
    else if (dreamevent == 34)
    {
    send_to_char("You see an add on the television .. wait, whats a television?!?\n\r",ch);
    send_to_char("They dont exist yet! hmm anyway back to your dream..\n\r",ch);
    send_to_char("You see an add on the television about a new lottery..\n\r",ch);
    send_to_char("You run down to the ticket place and buy 4 tickets..\n\r",ch);
    send_to_char("\n\r",ch);
    send_to_char("It is now a few days later, the draw is about to begin!!\n\r",ch);
    send_to_char("\n\r",ch);
    number = number_range(0,3);

    switch(number) 
     {
     case (0) :
     send_to_char("You win the jackpot!!!!!!!\n\r",ch);
     send_to_char("You are rich rich! RICH!!!\n\r",ch);
     send_to_char("Or so you feel anyway..\n\r",ch);
     ch->steel = ch->steel + 10;
     send_to_char("You get 10 steel coins!!!\n\r",ch);
     act( "$n is rolling around in steel coins!", ch, NULL, NULL, TO_ROOM);
     return;

     case (1) :
     send_to_char("You win second prize!!!!\n\r",ch);
     send_to_char("You see the world in a whole new aspect!!\n\r",ch);
     ch->steel = ch->steel + 4;
     send_to_char("You win 4 steel coins!!!\n\r",ch);
     act( "$n is clutching 4 steel coins in $s hand!!", ch, NULL, NULL, TO_ROOM);
     return;

     case (2) :
     send_to_char("You win third prize!!!\n\r",ch);
     send_to_char("Not bad for your first lottery..=)\n\r",ch);
     ch->gold = ch->gold + 4;
     send_to_char("You win 4 gold coins!!\n\r",ch);
     act( "$n clutches 4 gold coins in $s hand!!", ch, NULL, NULL, TO_ROOM);
     return;

     case (3) :
     send_to_char("None of your tickets win.. bummer..\n\r",ch);
     return;
    }
   }
    else if (dreamevent == 35)
    {
    send_to_char("...ZZzzzzZZzzZzzZZZzzZZZzzZZZzZzzZzZZzzzZZzzzZZzzZZ\n\r",ch);
    return;
    }
    else if (dreamevent == 36)
    {
    send_to_char("...ZZzzzzZZzzZzzZZZzzZZZzzZZZzZzzZzZZzzzZZzzzZZzzZZ\n\r",ch);
    return;
    }
    else if (dreamevent == 37)
    {
    send_to_char("You find yourself standing before your school of training *sigh*\n\r",ch);
    send_to_char("a dreary morning and only 1/3 into the school year.\n\r",ch);
    send_to_char("All of a sudden you are soaring high above the street!\n\r",ch);
    send_to_char("You swoop down and topple over a couple trash cans!\n\r",ch);
    send_to_char("All your fellow students turn green with envy.\n\r",ch);
    send_to_char("You begin to doubt your abilities! LOOK OUT!!\n\r",ch);
    send_to_char("You are falling into a deep, black hole.....\n\r",ch);
    return;
    }
    else
    {
    send_to_char("...ZZzzzzZZzzZzzZZZzzZZZzzZZZzZzzZzZZzzzZZzzzZZzzZZ\n\r",ch);
    return;
    }
  }
}

bool number_chance(int num)
{
    if (number_range(1,100) <= num) return TRUE;
    else return FALSE;
}

void imprint_spell( int sn, int level, CHAR_DATA * ch, void *vo )
{
    static const int    sucess_rate[] = { 80, 25, 10 };

    char      buf [ MAX_STRING_LENGTH ];
    OBJ_DATA *obj = ( OBJ_DATA * ) vo;
    int       free_slots;
    int       i;
    int       mana;
    int       class;
    int       snlev = 0;

    for ( free_slots = i = 1; i < 4; i++ )
        if ( obj->value[i] != 0 )
            free_slots++;

    if ( free_slots > 3 )
    {
        act( "$p cannot contain any more spells.", ch, obj, NULL, TO_CHAR
);
        return;
    }

    mana = 40;
    mana += skill_table[sn].min_mana;
    if ( !IS_NPC( ch ) && ch->mana < mana )
    {
        send_to_char( "You don't have enough mana.\n\r", ch );
        return;
    }

    if ( number_percent( ) > ch->pcdata->learned[sn] )
    {
     send_to_char( "You lost your concentration.\n\r", ch );
        ch->mana -= mana / 2;
        return;
    }

    ch->mana -= mana;
    obj->value[free_slots] = sn;

    if ( number_percent( ) > sucess_rate[free_slots - 1] )
    {
        sprintf( buf, "The magic enchantment has failed: the %s vanishes.\n\r",
        item_name( obj->item_type ) );
        send_to_char( buf, ch );
        extract_obj( obj );
        return;
    }


    free_string( obj->short_descr );
    sprintf( buf, "a %s of ", item_name( obj->item_type ) );
    for ( i = 1; i <= free_slots; i++ )
        if ( obj->value[ i ] != -1 )
        {
            strcat( buf, skill_table[ obj->value[ i ] ].name );
            ( i != free_slots ) ? strcat( buf, ", " ) : strcat( buf, "" );
    }
    obj->short_descr = str_dup( buf );

    sprintf( buf, "%s %s %s", obj->name, item_name( obj->item_type ), skill_table[sn].name );
    free_string( obj->name );
    obj->name = str_dup( buf );

     for (class = 0;class < MAX_CLASS;class++)
     {
     if(skill_table[sn].skill_level[class] < snlev)
      {
      snlev = skill_table[sn].skill_level[class];
      }
     }
    if(obj->level < snlev) {obj->level = snlev;
    }
    sprintf( buf, "You have imbued a new spell to the %s.\n\r",
    item_name( obj->item_type ) );
    send_to_char( buf, ch );
    return;
}

void do_pillify( CHAR_DATA * ch,char *argument )
{
  OBJ_DATA  *pill;
  char       arg [ MAX_INPUT_LENGTH ];
  int        sn;

  if ( !IS_AWAKE(ch) )
  {
	    send_to_char( "In your dreams, or what?\n\r", ch );
	    return;
  }

  if ( ch->desc->original != NULL )
  {
	send_to_char( "You can not NOT pillify a spell while switched.\n\r", ch );
	send_to_char( "Please RETURN to your original self if you wish to pillify something.\n\r", ch );
	return;
  }

  if(IS_NPC(ch))
  {
  send_to_char("You don't have any need for pills.\n\r",ch);
  return;
  }

  if(get_skill(ch,gsn_pillify) < 1)
  {
  send_to_char("Huh?\n\r",ch);
  return;
  }

  argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Make a pill out of what spell?\n\r", ch );
        return;
    }

  if ( ( sn = skill_lookup( arg ) ) < 0 )
    {
        send_to_char( "You don't know any spells by that name.\n\r", ch );
        return;
    }

    if(!number_chance(get_skill(ch,sn)))
    {
         send_to_char("You don't know that spell well enough to make a pill of it!\n\r",ch);
         return;
    }

    if ( (skill_table[sn].target != TAR_CHAR_DEFENSIVE) && 
         (skill_table[sn].target != TAR_CHAR_SEMIOFFENSIVE) &&
         (skill_table[sn].target != TAR_CHAR_SELF)              ) 
    {
        send_to_char( "You cannot make a pill of that.\n\r", ch );
        return;
    }
  pill = create_object( get_obj_index( OBJ_VNUM_PILL ), 0 );
  if(!pill)
  {
  send_to_char("Could not find the pill object, please notify an IMP!\n\r",ch);
  return;
  }
  obj_to_char(pill,ch);
  send_to_char("You begin focusing your magical energy.\n\r",ch);
  act( "$n begins focusing their magical energy.", ch, NULL, NULL, TO_ROOM );
    WAIT_STATE( ch, skill_table[ gsn_pillify ].beats );
  act( "$p suddenly pops into existence.",ch, pill, NULL, TO_CHAR);
  act( "$p suddenly pops into existence.",ch, pill, NULL, TO_ROOM);
   if ( !IS_NPC( ch )
        && ( number_percent( ) > ch->pcdata->learned[gsn_pillify] ||
             number_percent( ) > ( ( get_curr_stat(ch,STAT_INT) - 13 ) * 5 +
                                   ( get_curr_stat(ch,STAT_WIS) - 13 ) * 3 ) ) )
    {
        act( "$p {Yexplodes {Rviolently{x!", ch, pill, NULL, TO_CHAR );
        act( "$p {Yexplodes {Rviolently{x!", ch, pill, NULL, TO_ROOM );
        extract_obj( pill );
        damage( ch, ch, ch->max_hit / 10, gsn_pillify, WEAR_NONE, DAM_ENERGY );
        check_improve(ch,gsn_pillify,FALSE,1);
        return;
    }

    pill->level       = ch->level / 2;
    pill->value[0]    = ch->level / 2;
    imprint_spell( sn, ch->level, ch, pill );
    check_improve(ch,gsn_pillify,TRUE,1);
    return;
}

void do_find( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char buf[MSL];
    AFFECT_DATA af;
    int i;
    OBJ_DATA *herb;
    int Number_ofRoots = 15; /*max_herbs*/ 
 
    struct flora_type
    {
	char *name;
	int   vnum;
  	int   sect;
	int   timer;
	short start;
	short stop;
	char *to_char;
	char *to_room;
    };

    static const struct flora_type flora_table [] =
    {
	{"seaweed", OBJ_VNUM_HERB_SEAWEED, SECT_WATER_NOSWIM|SECT_WATER_SWIM|SECT_UNDERWATER, 35, 23, 1,
	"You look out onto the water, searching for any floating seaweed.",
	"$n begins to look out onto the water, looking for any floating seaweed."	
	},
	{"garlic", OBJ_VNUM_HERB_GARLIC, SECT_FOREST, 5, 23, 1,
	"You search for a fresh garlic bulb in the undergrowth.",
	"$n begins to search for an herb in the undergrowth."	
	},
	{"mint", OBJ_VNUM_HERB_MINT, SECT_FOREST, 5, 23, 1,
      "You search for some fresh mint leaves in the undergrowth.", 
      "$n begins to search for some leaves in the undergrowth.",
	},
	{"tateesha", OBJ_VNUM_HERB_TATEESHA, SECT_SWAMP, 5 ,  23, 1,
      "You search the swamp vegetation for some tateesha nuts.",
      "$n searches through the swamp vegitation for some nuts."
	},
	{"suaeysit", OBJ_VNUM_HERB_SUAEYSIT, SECT_FOREST, 15, 23, 1,
	"You search the forest floor for a dark black mushroom.",
      "$n searches the forest floor for something.",
	},
	{"fungaboru", OBJ_VNUM_HERB_FUNGABORU, SECT_HILLS, 15, 23, 1,
      "You search for fungaboru in the damp hillside.", 
      "$n begins searching for mushrooms in the damp hillside.",
	},
	{"ghostroot", OBJ_VNUM_HERB_GHOSTROOT, SECT_GRAVEYARD, 15, 23, 1,
      "You search for a fresh ghostroot in the soil of the nearby graves.", 
      "$n begins to dig through the soil of the nearby graves.", 
	},
      {"acorn", OBJ_VNUM_HERB_ACORN, SECT_FOREST, 10, 23, 1,
      "You search the forest for a fresh acorn.",
      "$n begins searching the forest area for something.",
      },
      {"zulsendra", OBJ_VNUM_HERB_ZULSENDRA, SECT_GRAVEYARD, 5 , 23, 1,
      "You search about the graves for some zulsendra mushrooms.",
      "$n searches about the graves for something.",
      },
      {"yavethalion", OBJ_VNUM_HERB_YAVETHALION, SECT_FOREST, 5, 23, 1,
       "You search about for some yavethalion fruit.",
       "$n looks about in the forest for something.",
      },
      {"winclamit", OBJ_VNUM_HERB_WINCLAMIT, SECT_FOREST, 5, 23, 1,
      "You search the forest trees for a fine winclamit berry.",
      "$n is looking at several small berries.",
      },
	{"stonewort", OBJ_VNUM_HERB_STONEWORT, SECT_MOUNTAIN, 10, 23, 1,
	 "You toss over a few stones looking for a sprout of stonewort.",
	"$n begins to search for an herb under the stones of the rocky land.",
	},
	{"woodsorrel", OBJ_VNUM_HERB_WOODSORREL, SECT_DESERT, 10, 23, 1,
	 "You search through the sands for a small sample of woodsorrel.",
	 "$n sifts through the sand in search of a desert flower.",
	},
	{"iceroot", OBJ_VNUM_HERB_ICEROOT, SECT_MOUNTAIN, 10, 23, 1,
	 "You dig through the cold mountain soil in search of iceroot.",
	 "$n digs into the mountain soil in search of a root.",
	},
	{"goldbloom", OBJ_VNUM_HERB_GOLDBLOOM, SECT_HILLS, 20, 23,1,
	 "You search for a small flower of goldbloom in the hills.",
         "$n searches through the grass, looking for a small goldenflower.",
        }
         
    };
 
    argument = one_argument( argument, arg1);
 
    if (IS_AFFECTED2( ch, AFF_FORGET))
    {
	send_to_char("You can't seem to remember how to identify certain plants.\n\r",ch);
	return;
    }

    if (get_skill(ch,gsn_herbal_discern) == 0  
    || ch->level < skill_table[gsn_herbal_discern].skill_level[ch->class] )
    {
     send_to_char("You do not know how to find herbs on your own!\n\r",ch);
     return;
    }
    if ( ch->fighting != NULL) 
    {
	send_to_char( "You are too busy to search for any herbs right now!\n\r",ch);
        return;
    }
    if ( arg1[0] == '\0' ) 
    {
        send_to_char("What herb do you wish to search for?\n\r",ch);
        return;
    }
    for (i=0 ; i < Number_ofRoots; i++)
    {
	if (!str_cmp(arg1, flora_table[i].name))
		break;
    }
    if  ( i == Number_ofRoots )
    { 
    	send_to_char("There is no known herb by that name.\n\r",ch);
        return;
    }

    if ( IS_AFFECTED2( ch, AFF_HERBS )) 
    {
        send_to_char( "You can not find anymore herbs at the moment.\n\r", ch);
        return;
    }


    if (ch->in_room->sector_type == flora_table[i].sect)
    {
	act(flora_table[i].to_char, ch, NULL, NULL, TO_CHAR);
	act(flora_table[i].to_room, ch, NULL, NULL, TO_ROOM);

	if (number_percent() > ch->pcdata->learned[gsn_herbal_discern])
	{
		sprintf(buf, "You fail to find any fresh %s.\r\n",
		flora_table[i].name);
		send_to_char(buf, ch);
		check_improve(ch, gsn_herbal_discern, FALSE, 1);
		return;
	}
	else
	{
 	      check_improve(ch,gsn_herbal_discern,TRUE,1);
    		herb = create_object(get_obj_index(flora_table[i].vnum), 0);
         	obj_to_room( herb, ch->in_room);
		if (flora_table[i].timer >= 15)
		herb->timer = flora_table[i].timer*3;
		else
		herb->timer = flora_table[i].timer*5;
              	act("You find $p.", ch, herb, NULL, TO_CHAR);
              	act("$n carefully uncovers $p.",ch,herb,NULL,TO_ROOM);

		    af.where     = TO_AFFECTS2;
                af.type      = gsn_herbal_discern;
                af.level     = ch->level;
                af.duration  = flora_table[i].timer;
                af.location  = APPLY_NONE;
                af.modifier  = 0;
                af.bitvector = AFF_HERBS;
                affect_to_char( ch, &af );
	}
    }
    else
    {
	sprintf(buf, "You cannot find %s in this type of terrain.\r\n", 
	flora_table[i].name);
	send_to_char(buf, ch);
    }
}

void spell_imprint(int sn, int level, CHAR_DATA *ch, void *vo, char *argument )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    int       sp_slot, i, mana;
    char      buf[ MAX_STRING_LENGTH ];
    char arg[MAX_INPUT_LENGTH];

  if ( ch->desc->original != NULL )
  {
	send_to_char( "You can not NOT imprint a spell while switched.\n\r", ch );
	send_to_char( "Please RETURN to your original self if you wish to imprint something.\n\r", ch );
	return;
  }

    if (skill_table[sn].spell_fun == spell_null )
        {
        send_to_char("That is not a spell.\n\r",ch);
        return;
        }

    one_argument(argument, arg);

    /* counting the number of spells contained within */

    for (sp_slot = i = 1; i < 4; i++)
        if (obj->value[i] != -1)
            sp_slot++;

    if (sp_slot > 3)
    {
        act ("$p cannot contain any more spells.", ch, obj, NULL, TO_CHAR);
        return;
    }

   /* scribe/brew costs 4 times the normal mana required to cast the
      spell */
    mana = skill_table[sn].min_mana * 4;

    if ( !IS_NPC(ch) && ch->mana < mana )
    {
        send_to_char( "You don't have enough mana.\n\r", ch );
        return;
    }


    if ( number_percent( ) > ch->pcdata->learned[sn] )
    {
        send_to_char( "You lost your concentration.\n\r", ch );
        ch->mana -= mana / 2;
        return;
    }

    /* executing the imprinting process */
    ch->mana -= mana;
    obj->value[sp_slot] = sn;

    /* Making it successively harder to pack more spells into potions or
       scrolls - JH */

    switch( sp_slot )
    {

    default:
        bug( "sp_slot has more than %d spells.", sp_slot );
        return;

    case 1:
        if ( number_percent() > 80 )
        {
	  send_to_char("\n\r", ch);
        sprintf( buf, "The magic enchantment has failed and the %s vanishes.\n\r",
        item_name( obj->item_type ) );
        send_to_char( buf, ch );
          extract_obj( obj );
          return;
        }
        break;
    case 2:
        if ( number_percent() > 25 )
        {
	  send_to_char("\n\r", ch);
        sprintf( buf, "The magic enchantment has failed and the %s vanishes.\n\r",
        item_name( obj->item_type ) );
        send_to_char( buf, ch );
          extract_obj( obj );
          return;
        }
        break;

    case 3:
        if ( number_percent() > 10 )
        {
	  send_to_char("\n\r", ch);
        sprintf( buf, "The magic enchantment has failed and the %s vanishes.\n\r",
        item_name( obj->item_type ) );
        send_to_char( buf, ch );
          extract_obj( obj );
          return;
        }
        break;
    }


    /* labeling the item */

    free_string (obj->short_descr);
    if ( arg[0] == '\0' )
        {
        sprintf ( buf, "a %s of ", item_name( obj->item_type ) );
        for (i = 1; i <= sp_slot ; i++)
                if (obj->value[i] != -1)
                        {
                        strcat (buf, skill_table[obj->value[i]].name);
                        (i != sp_slot ) ? strcat (buf, ", ") : strcat (buf, "") ;
                        }
        }
    else
    sprintf( buf, "a %s of %s ", item_name( obj->item_type ), skill_table[sn].name );
    obj->short_descr = str_dup(buf);
    sprintf( buf, "%s %s %s", obj->name, item_name( obj->item_type ), skill_table[sn].name );
    free_string( obj->name );
    obj->name = str_dup( buf );
    send_to_char("\n\r", ch);
    send_to_char("You have successfully enscribed a new spell onto the parchment.\n\r", ch);
    sprintf( buf, "You carefully roll the %s up and seal it with hot wax,\n\r",
    item_name( obj->item_type ) );
    send_to_char( buf, ch );
    send_to_char("then write the name of the spell upon the seal.\n\r", ch);

    return;
}

void spell_mix(int sn, int level, CHAR_DATA *ch, void *vo, char *argument )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    int       sp_slot, i, mana;
    char      buf[ MAX_STRING_LENGTH ];
    char arg[MAX_INPUT_LENGTH];

  if ( ch->desc->original != NULL )
  {
	send_to_char( "You can not NOT mix a spell while switched.\n\r", ch );
	send_to_char( "Please RETURN to your original self if you wish to mix something.\n\r", ch );
	return;
  }

    if (skill_table[sn].spell_fun == spell_null )
        {
        send_to_char("That is not a spell.\n\r",ch);
        return;
        }

    one_argument(argument, arg);

    /* counting the number of spells contained within */

    for (sp_slot = i = 1; i < 4; i++)
        if (obj->value[i] != -1)
            sp_slot++;

    if (sp_slot > 3)
    {
        act ("$p cannot contain any more spells.", ch, obj, NULL, TO_CHAR);
        return;
    }

   /* brew costs 4 times the normal mana required to cast the
      spell */
    mana = skill_table[sn].min_mana * 4;

    if ( !IS_NPC(ch) && ch->mana < mana )
    {
        send_to_char( "You don't have enough mana.\n\r", ch );
        return;
    }


    if ( number_percent( ) > ch->pcdata->learned[sn] )
    {
        send_to_char( "You lost your concentration.\n\r", ch );
        ch->mana -= mana / 2;
        return;
    }

    /* executing the imprinting process */
    ch->mana -= mana;
    obj->value[sp_slot] = sn;

    /* Making it successively harder to pack more spells into potions or
       scrolls - JH */

    switch( sp_slot )
    {

    default:
        bug( "sp_slot has more than %d spells.", sp_slot );
        return;

    case 1:
        if ( number_percent() > 80 )
        {
	  send_to_char("\n\r", ch);
        sprintf( buf, "The magic enchantment has failed and the %s vanishes.\n\r",
        item_name( obj->item_type ) );
        send_to_char( buf, ch );
          extract_obj( obj );
          return;
        }
        break;
    case 2:
        if ( number_percent() > 25 )
        {
	  send_to_char("\n\r", ch);
        sprintf( buf, "The magic enchantment has failed and the %s vanishes.\n\r",
        item_name( obj->item_type ) );
        send_to_char( buf, ch );
          extract_obj( obj );
          return;
        }
        break;

    case 3:
        if ( number_percent() > 10 )
        {
	  send_to_char("\n\r", ch);
        sprintf( buf, "The magic enchantment has failed and the %s vanishes.\n\r",
        item_name( obj->item_type ) );
        send_to_char( buf, ch );
          extract_obj( obj );
          return;
        }
        break;
    }


    /* labeling the item */

    free_string (obj->short_descr);
    if ( arg[0] == '\0' )
        {
        sprintf ( buf, "a %s of ", item_name( obj->item_type ) );
        for (i = 1; i <= sp_slot ; i++)
                if (obj->value[i] != -1)
                        {
                        strcat (buf, skill_table[obj->value[i]].name);
                        (i != sp_slot ) ? strcat (buf, ", ") : strcat (buf, "") ;
                        }
        }
    else
    sprintf( buf, "a %s of %s ", item_name( obj->item_type ), skill_table[sn].name );
    obj->short_descr = str_dup(buf);
    sprintf( buf, "%s %s %s", obj->name, item_name( obj->item_type ), skill_table[sn].name );
    free_string( obj->name );
    obj->name = str_dup( buf );
    send_to_char("\n\r", ch);
    send_to_char("You have successfully mixed in the ingredients, forming a new\n\r", ch);
    sprintf( buf, "spell into the %s. You then carefully write the name of the spell\n\r",
    item_name( obj->item_type ) );
    send_to_char( buf, ch );
    send_to_char("onto the side of the glass.\n\r", ch);

    return;
}


void do_brew ( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int sn;

    if ( !IS_NPC( ch )                                                  
	&& ch->level < skill_table[gsn_brew].skill_level[ch->class] )
    {                                          
	send_to_char( "You do not know how to brew potions.\n\r", ch );
	return;
    }

  if ( ch->desc->original != NULL )
  {
	send_to_char( "You can not NOT brew a spell while switched.\n\r", ch );
	send_to_char( "Please RETURN to your original self if you wish to brew something.\n\r", ch );
	return;
  }

    /* Do we have a vial to brew potions? */
    for ( obj = ch->carrying; obj; obj = obj->next_content )
    {
	if ( obj->item_type == ITEM_POTION && obj->wear_loc == WEAR_HOLD )
	    break;
    }

    if ( !obj )
    {
	send_to_char( "You are not holding a vial.\n\r", ch );
	return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Brew what spell?\n\r", ch );
	return;
    }

    if ( ( sn = skill_lookup(arg1) )  < 0)
    {
	send_to_char( "You don't know any spells by that name.\n\r", ch );
	return;
    }

    if ( ( sn = skill_lookup(arg1)) == skill_lookup("summon"))
    {
	send_to_char( "You can not brew that particular spell.\n\r", ch );
	return;
    }

    if ( ( sn = skill_lookup(arg1)) == skill_lookup("beastspite"))
    {
	send_to_char( "You can not scribe that particular spell.\n\r", ch );
	return;
    }

    if ( ( sn = skill_lookup(arg1)) == skill_lookup("blood bond"))
    {
	send_to_char( "You can not scribe that particular spell.\n\r", ch );
	return;
    }

    if ( ( sn = skill_lookup(arg1)) == skill_lookup("black cairn"))
    {
	send_to_char( "You can not scribe that particular spell.\n\r", ch );
	return;
    }

    if ( ( sn = skill_lookup(arg1)) == skill_lookup("blessed watchfulness"))
    {
	send_to_char( "You can not scribe that particular spell.\n\r", ch );
	return;
    }

    if ( ( sn = skill_lookup(arg1)) == skill_lookup("sequester"))
    {
	send_to_char( "You can not brew that particular spell.\n\r", ch );
	return;
    }

    if ( ( sn = skill_lookup(arg1)) == skill_lookup("gate"))
    {
	send_to_char( "You can not brew that particular spell.\n\r", ch );
	return;
    }

    if ( ( sn = skill_lookup(arg1)) == skill_lookup("shadow walk"))
    {
	send_to_char( "You can not brew that particular spell.\n\r", ch );
	return;
    }

    if ( ( sn = skill_lookup(arg1)) == skill_lookup("earthmeld"))
    {
	send_to_char( "You can not brew that particular spell.\n\r", ch );
	return;
    }

    if ( ( sn = skill_lookup(arg1)) == skill_lookup("cantrip"))
    {
	send_to_char( "You can not brew that particular spell.\n\r", ch );
	return;
    }

    if ( ( sn = skill_lookup(arg1)) == skill_lookup("nexus"))
    {
	send_to_char( "You can not brew that particular spell.\n\r", ch );
	return;
    }

    if ( ( sn = skill_lookup(arg1)) == skill_lookup("dimensional door"))
    {
	send_to_char( "You can not brew that particular spell.\n\r", ch );
	return;
    }

    if ( arg2[0] == '\0' )
	send_to_char("You carefully begin to pour in some carefully chosen ingredients into\n\r", ch);
	send_to_char("the vial and begin to stir carefully.\n\r", ch);


    /* preventing potions of gas breath, acid blast, etc. It doesn't 
       make sense when you quaff a gas breath potion, and then the mobs 
       in the room are hurt. Those TAR_IGNORE spells are a mixed 
       blessing.  --- JH                                              */
  
    if ( (skill_table[sn].target != TAR_CHAR_DEFENSIVE) && 
         (skill_table[sn].target != TAR_CHAR_SEMIOFFENSIVE) &&
         (skill_table[sn].target != TAR_CHAR_SELF)              ) 
    {
	send_to_char( "You cannot brew that type of spell.\n\r", ch );
	return;
    }

    act( "$n mixes in some carefully chosen ingredients into an empty", ch, obj, NULL, TO_ROOM );
    act( "vial and begins to stir them, preparing to make a new potion.", ch, obj, NULL, TO_ROOM );
    WAIT_STATE( ch, skill_table[gsn_brew].beats );

    /* Check the skill percentage, fcn(wis,int,skill) */
    if ( !IS_NPC(ch) 
         && ( number_percent( ) > ch->pcdata->learned[gsn_brew] ||
              number_percent( ) > ((get_curr_stat(ch,STAT_INT) -13)*5 + 
                                   (get_curr_stat(ch,STAT_WIS) -13)*3) ))
    {
	act( "$p explodes violently, nearly burning you in the process!", ch, obj, NULL, TO_CHAR );
	act( "$p explodes violently!", ch, obj, NULL, TO_ROOM );
	extract_obj( obj );
	check_improve(ch, gsn_brew, FALSE,1);
	return;
    }

    obj->level = ch->level/2;
    obj->value[0] = ch->level/4;
    spell_mix(sn, ch->level, ch, obj, arg2); 
    check_improve(ch, gsn_brew, TRUE,1);
}

void do_scribe ( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int sn, chance, reqint, reqwis;

    chance = reqint = reqwis = 0;
    if ( !IS_NPC( ch )                                                  
	&& ch->level < skill_table[gsn_scribe].skill_level[ch->class] )
    {                                          
	send_to_char( "You do not know how to scribe scrolls.\n\r", ch );
	return;
    }

  if ( ch->desc->original != NULL )
  {
	send_to_char( "You can not NOT scribe a spell while switched.\n\r", ch );
	send_to_char( "Please RETURN to your original self if you wish to scribe something.\n\r", ch );
	return;
  }

    /* Do we have a parchment to scribe spells? */
    for ( obj = ch->carrying; obj; obj = obj->next_content )
    {
	if ( obj->item_type == ITEM_SCROLL && obj->wear_loc == WEAR_HOLD )
	    break;
    }
    if ( !obj )
    {
	send_to_char( "You are not holding a parchment.\n\r", ch );
	return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Scribe what spell?\n\r", ch );
	return;
    }

    if ( ( sn = skill_lookup(arg1) )  < 0)
    {
	send_to_char( "You don't know any spells by that name.\n\r", ch );
	return;
    }

    if ( ( sn = skill_lookup(arg1)) == skill_lookup("summon"))
    {
	send_to_char( "You can not scribe that particular spell.\n\r", ch );
	return;
    }

    if ( ( sn = skill_lookup(arg1)) == skill_lookup("beastspite"))
    {
	send_to_char( "You can not scribe that particular spell.\n\r", ch );
	return;
    }

    if ( ( sn = skill_lookup(arg1)) == skill_lookup("blood bond"))
    {
	send_to_char( "You can not scribe that particular spell.\n\r", ch );
	return;
    }

    if ( ( sn = skill_lookup(arg1)) == skill_lookup("black cairn"))
    {
	send_to_char( "You can not scribe that particular spell.\n\r", ch );
	return;
    }

    if ( ( sn = skill_lookup(arg1)) == skill_lookup("blessed watchfulness"))
    {
	send_to_char( "You can not scribe that particular spell.\n\r", ch );
	return;
    }

    if ( ( sn = skill_lookup(arg1)) == skill_lookup("sequester"))
    {
	send_to_char( "You can not scribe that particular spell.\n\r", ch );
	return;
    }

    if ( ( sn = skill_lookup(arg1)) == skill_lookup("gate"))
    {
	send_to_char( "You can not scribe that particular spell.\n\r", ch );
	return;
    }

    if ( ( sn = skill_lookup(arg1)) == skill_lookup("shadow walk"))
    {
	send_to_char( "You can not scribe that particular spell.\n\r", ch );
	return;
    }

    if ( ( sn = skill_lookup(arg1)) == skill_lookup("earthmeld"))
    {
	send_to_char( "You can not scribe that particular spell.\n\r", ch );
	return;
    }

    if ( ( sn = skill_lookup(arg1)) == skill_lookup("cantrip"))
    {
	send_to_char( "You can not scribe that particular spell.\n\r", ch );
	return;
    }

    if ( ( sn = skill_lookup(arg1)) == skill_lookup("nexus"))
    {
	send_to_char( "You can not scribe that particular spell.\n\r", ch );
	return;
    }

    if ( ( sn = skill_lookup(arg1)) == skill_lookup("dimensional door"))
    {
	send_to_char( "You can not scribe that particular spell.\n\r", ch );
	return;
    }

    if ( arg2[0] == '\0' )
	send_to_char("You begin to write the complex words of the spell onto the scroll.\n\r", ch);
    
    reqint = ((get_curr_stat(ch,STAT_INT)-13)*5);
    reqwis = ((get_curr_stat(ch,STAT_WIS)-13)*3);

    act( "$n begins to write the complex words of a spell onto a scroll.", ch, obj, NULL, TO_ROOM );
    WAIT_STATE( ch, skill_table[gsn_scribe].beats );

    /* Check the skill percentage, fcn(int,wis,skill) */
    if ((!IS_NPC(ch)) && 
	(number_percent( ) > (ch->pcdata->learned[gsn_scribe] + chance) ||
	(number_percent( ) > (reqint + reqwis)) ))
    {
	act( "$p bursts in flames, nearly burning you in the process!", ch, obj, NULL, TO_CHAR );
	act( "$p bursts in flames!", ch, obj, NULL, TO_ROOM );
	extract_obj( obj );
	check_improve(ch, gsn_scribe,FALSE,1); 
	return;
    }

    /* basically, making scrolls more potent than potions; also, scrolls
       are not limited in the choice of spells, i.e. scroll of enchant  
       weapon has no analogs in potion forms --- JH */

    obj->level = ch->level*2/3;
    obj->value[0] = ch->level/3;
    spell_imprint(sn, ch->level, ch, obj, arg2); 
    check_improve(ch, gsn_scribe,TRUE,1);
}

void do_trophy( CHAR_DATA *ch, char *argument )
{
/*
    static char *headers[] = { "the corpse of the ", "the corpse of The ",
                               "the corpse of an ", "the corpse of An ",
                               "the corpse of a ", "the corpse of A ",
                               "corpse of " }; // (This one must be last)
*/
    static char *headers[] = { "the corpse of ", "corpse of " }; //(This one must be last)

    char arg1[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    char buf[MAX_STRING_LENGTH];
    int i, skillper;

    if (ch->riding)
    {
      send_to_char("You cannot reach the corpse while mounted.\n\r",ch);
      return;
    }

    one_argument ( argument, arg1);

    if ( arg1[0] == '\0' )
    {
        send_to_char ( "Make a trophy bag out of what?\n\r", ch);
        return;
    }
    
    if ( (obj = get_obj_list( ch,arg1,ch->in_room->contents)) == NULL )
    {
        if ( (obj = get_obj_carry( ch, arg1, ch )) == NULL )
        {
            send_to_char ("You can't find it.\n\r", ch);
            return;
        }
    }
    
    if ( obj->item_type != ITEM_CORPSE_NPC && obj->item_type != ITEM_CORPSE_PC)
	return;

    skillper = (get_skill(ch,gsn_trophy));
    if ( ( skillper == 0) || (!IS_NPC(ch)
    &&  ch->level < skill_table[gsn_trophy].skill_level[ch->class]))
       {
        send_to_char("The skill of making trophy bags is unknown to you.\n\r",ch);
        return;
        }

    if (ch->fighting != NULL)
       {
        send_to_char("You are to busy to make a trophy bag now!\n\r",ch);
        return;
       }

    
    if ( number_percent() > skillper )
    {
        send_to_char("You can't seem to get it to form a bag.\n\r",ch);
        check_improve(ch, gsn_trophy, FALSE,1);
        return;
    }
       
    check_improve(ch, gsn_trophy, TRUE,1);

    /* dump items that are in the corpse onto the ground */
    /* Taken from the original ROM code and added into here. */

    if ( obj->item_type == ITEM_CORPSE_PC )
    {   /* save the contents */
       {

            OBJ_DATA *t_obj, *next_obj;
            for (t_obj = obj->contains; t_obj != NULL; t_obj = next_obj)
            {
                next_obj = t_obj->next_content;
                obj_from_obj(t_obj);
                if (obj->in_obj) /* in another object */
                    obj_to_obj(t_obj,obj->in_obj);
                else if (obj->carried_by) /* carried */
                    if (obj->wear_loc == WEAR_TATTOO)
                        if (obj->carried_by->in_room == NULL)
                            extract_obj(t_obj);
                        else
                            obj_to_room(t_obj,obj->carried_by->in_room);
                    else
                        obj_to_char(t_obj,obj->carried_by);
               else if (obj->in_room == NULL) /* destroy it */
                    extract_obj(t_obj);
                else /* to a room */
                    obj_to_room(t_obj,obj->in_room);
           }
      }

  }

    if ( obj->item_type == ITEM_CORPSE_NPC )
    {
       {
            OBJ_DATA *t_obj, *next_obj;
            for (t_obj = obj->contains; t_obj != NULL; t_obj = next_obj)
            {
                next_obj = t_obj->next_content;
                obj_from_obj(t_obj);
                if (obj->in_obj) /* in another object */
                    obj_to_obj(t_obj,obj->in_obj);
                else if (obj->carried_by) /* carried */
                    if (obj->wear_loc == WEAR_TATTOO)
                        if (obj->carried_by->in_room == NULL)
                            extract_obj(t_obj);
                        else
                            obj_to_room(t_obj,obj->carried_by->in_room);
                    else
                        obj_to_char(t_obj,obj->carried_by);
                else if (obj->in_room == NULL) /* destroy it */
                    extract_obj(t_obj);
                else /* to a room */
                    obj_to_room(t_obj,obj->in_room);
         }
     }
  }
       
    for (i = 0; i < 2; i++)
       {
       int len = strlen(headers[i]);
/*       if ( memcmp(obj->short_descr, headers[i], len) == 0 ) */
       if ( !str_prefix(headers[i], obj->short_descr) )
          {
           sprintf( buf, "bag" );
           free_string( obj->name );
           obj->name = str_dup(buf);
          sprintf( buf, "A bag of fine %s hide catches your eye.",
             obj->short_descr+len );
          free_string( obj->description );
          obj->description = str_dup( buf );

          sprintf( buf, "bag made from %s's hide", obj->short_descr+len );
          free_string( obj->short_descr );
          obj->short_descr = str_dup( buf );

          break;
          }
	else 
           { 
            sprintf( buf, "bag" );
           free_string( obj->name );
           obj->name = str_dup(buf);
	     sprintf( buf, "A bag of fine %s hide catches your eye.",
	     obj->short_descr ); 
             free_string( obj->description ); 
             obj->description = str_dup( buf ); 
    
             sprintf( buf, "bag made from %s's hide", obj->short_descr ); 
             free_string( obj->short_descr ); 
             obj->short_descr = str_dup( buf ); 
              
             break; 
            } 

       }

    obj->item_type = ITEM_CONTAINER;
    obj->wear_flags = ITEM_HOLD|ITEM_TAKE;
    obj->timer = 0;
    obj->weight = 5;
    obj->level = ch->level/3;
    obj->cost = ch->level * 50;
    obj->value[0] = ch->level * 10;             /* Weight capacity */
    obj->value[1] = 1;                          /* Closeable */
    obj->value[2] = -1;                         /* No key needed */
    obj->value[3] = 100;
    obj->pIndexData = get_obj_index( OBJ_VNUM_TROPHY );

    act( "Your new $p turned out well.", ch, obj, NULL, TO_CHAR );
    act( "$n's new $p looks like it turned out well.", ch, obj, NULL, TO_ROOM );

    send_to_char( "Ok.\n\r", ch );
    return;
}

void do_drain(CHAR_DATA *ch, char *argument)
{
/*
    static char *headers[] = { "the corpse of the ", "the corpse of The ",
                               "the corpse of an ", "the corpse of An ",
                               "the corpse of a ", "the corpse of A ",
                               "corpse of " }; // (This one must be last)
*/
    static char *headers[] = { "the corpse of ", "corpse of " }; //(This one must be last)

    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char arg[MAX_STRING_LENGTH];
    int i, numst = 0;
    OBJ_DATA *jar = NULL;
    OBJ_DATA *obj;

    one_argument(argument, arg);

    if(get_skill(ch,gsn_drain)==0)
    {
       send_to_char("You have no knowledge of that skill.\n\r",ch);
       return;
    }
   
    if (ch->riding)
    {
      send_to_char("You can't reach the corpse while mounted.\n\r",ch);
      return;
    }

    if(arg[0]=='\0')
    {
       send_to_char("Drain what?\n\r",ch);
       return;
    }

    obj = get_obj_list( ch, arg, ch->in_room->contents );
    if ( obj == NULL )
    {
        send_to_char( "It's not here.\n\r", ch );
        return;
    }

    if( (obj->item_type != ITEM_CORPSE_NPC)
        && (obj->item_type!=ITEM_CORPSE_PC) )
    {
        send_to_char( "You can only drain corpses.\n\r", ch );
        return;
    }

    /* create and rename the jar */
   for (i = 0; i < 2; i++)
   {
     int len = strlen(headers[i]);
/*   if ( memcmp(obj->short_descr, headers[i], len) == 0 )*/
     if ( !str_prefix(headers[i], obj->short_descr) )
     {
       sprintf( buf, "A jar of %s's blood has been forgotten here.", obj->short_descr+len
);
       free_string( obj->description );
       obj->description = str_dup( buf );
       sprintf( buf2, "a jar of %s's blood", obj->short_descr+len );
       free_string( obj->short_descr );
       obj->short_descr = str_dup( buf2 );
       break;
     }
     else 
     { 
       sprintf( buf, "A jar of %s's blood has been forgotten here.", obj->short_descr ); 
       free_string( obj->description ); 
       obj->description = str_dup( buf ); 
       sprintf( buf2, "a jar of %s's blood", obj->short_descr ); 
       free_string( obj->short_descr ); 
       obj->short_descr = str_dup( buf2 );
       break; 
     } 
   }

   /* Check the skill roll, and have the fullness of the jar differ. */

   if(number_percent( ) < get_skill(ch,gsn_drain))
   {
     numst = dice(1,4);
     switch(numst)
     {
       case 1:
         jar = create_object( get_obj_index( OBJ_VNUM_JAR ), 0 );
         jar->description=str_dup(buf);
         jar->short_descr=str_dup(buf2);
         jar->value[0] = 5;
         jar->value[1] = 5;
         obj_to_char( jar, ch );
         act( "$n drains the blood from the corpse into a jar.", ch, jar, NULL, TO_ROOM );
         act( "You drain the blood from the corpse into a jar.", ch, jar, NULL, TO_CHAR );
         break;

       case 2:
         jar = create_object( get_obj_index( OBJ_VNUM_JAR ), 0 );
         jar->description=str_dup(buf);
         jar->short_descr=str_dup(buf2);
         jar->value[0] = ch->level;
         jar->value[1] = ch->level;
         obj_to_char( jar, ch );
         act( "$n drains the blood from the corpse into a jar.", ch, jar, NULL, TO_ROOM );
         act( "You drain the blood from the corpse into a jar.", ch, jar, NULL, TO_CHAR );
         break;

       case 3:
         jar = create_object( get_obj_index( OBJ_VNUM_JAR ), 0 );
         jar->description=str_dup(buf);
         jar->short_descr=str_dup(buf2);
         jar->value[0] = ch->level * 2;
         jar->value[1] = ch->level * 2;
         obj_to_char( jar, ch );
         act( "$n drains the blood from the corpse into a jar.", ch, jar, NULL, TO_ROOM );
         act( "You drain the blood from the corpse into a jar.", ch, jar, NULL, TO_CHAR );
         break;

       case 4:
         jar = create_object( get_obj_index( OBJ_VNUM_JAR ), 0 );
         jar->description=str_dup(buf);
         jar->short_descr=str_dup(buf2);
         jar->value[0] = ch->level * 2;
         jar->value[1] = ch->level * 2;
         jar->value[3] = 1;
         obj_to_char( jar, ch );
         act( "$n drains the blood from the corpse into a jar.", ch, jar, NULL, TO_ROOM );
         act( "You drain the blood from the corpse into a jar.", ch, jar, NULL, TO_CHAR );
         break;
     }
     check_improve(ch,gsn_drain,TRUE,1);

   }
   else
   {
     act( "$n fails to drain the corpse and destroys it.", ch, jar, NULL, TO_ROOM );
     act( "You fail to drain the corpse and destroy it.", ch, jar, NULL, TO_CHAR );
     check_improve(ch,gsn_drain,FALSE,1);
   }
   /* dump items that are in the corpse onto the ground */
   /* Taken from the original ROM code and added into here. */

   if ( obj->item_type == ITEM_CORPSE_PC )
   {   /* save the contents */
     {
       OBJ_DATA *t_obj, *next_obj;
       for (t_obj = obj->contains; t_obj != NULL; t_obj = next_obj)
       {
         next_obj = t_obj->next_content;
         obj_from_obj(t_obj);
         if (obj->in_obj) /* in another object */
           obj_to_obj(t_obj,obj->in_obj);
         else if (obj->carried_by) /* carried */
           obj_to_char(t_obj,obj->carried_by);
         else if (obj->in_room == NULL) /* destroy it */
           extract_obj(t_obj);
         else /* to a room */
           obj_to_room(t_obj,obj->in_room);
       }
     }
   }

   if ( obj->item_type == ITEM_CORPSE_NPC )
   {
     {
       OBJ_DATA *t_obj, *next_obj;
       for (t_obj = obj->contains; t_obj != NULL; t_obj = next_obj)
       {
         next_obj = t_obj->next_content;
         obj_from_obj(t_obj);
         if (obj->in_obj) /* in another object */
           obj_to_obj(t_obj,obj->in_obj);
         else if (obj->carried_by) /* carried */
           obj_to_char(t_obj,obj->carried_by);
         else if (obj->in_room == NULL) /* destroy it */
           extract_obj(t_obj);
         else /* to a room */
           obj_to_room(t_obj,obj->in_room);
       }
     }
   }

    /* Now remove the corpse */
    extract_obj(obj);
    return;
}

void do_scatter( CHAR_DATA *ch, char *argument )
{
  OBJ_DATA *obj;
  OBJ_DATA *obj_next;
  ROOM_INDEX_DATA *pRoomIndex;

  for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next )
  {
   obj_next = obj->next_content;
   if ( !IS_SET(obj->extra_flags, ITEM_NOPURGE) )
   {
    for ( ; ; )
    {
        pRoomIndex = get_room_index( number_range( 0, 17900 ) );
        if ( pRoomIndex != NULL )
        if ( CAN_ACCESS(pRoomIndex) )
            break;
    }
    act("$p vanishes in a puff of smoke!",ch,obj,NULL,TO_ROOM);
    obj_from_room(obj);
    obj_to_room(obj, pRoomIndex);
   }
  }
  return;
}

void do_strip( CHAR_DATA *ch, char *argument)
{

    char buf  [MAX_STRING_LENGTH];
    char arg1 [MAX_STRING_LENGTH];
    char arg2 [MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *tmp_ch;
    OBJ_DATA *obj = NULL;

    if (IS_NPC(ch))
	return;

    if ( !IS_NPC( ch ) 
    && ch->level < skill_table[gsn_strip].skill_level[ch->class] )
    {
      send_to_char( "Stripping? Better leave that one to the Thieves.\n\r", ch );
      return;
    }

    argument = one_argument( argument, arg1);
    argument = one_argument( argument, arg2);

    if (arg1[0] == '\0' || arg2[0] == '\0' )
    {
        send_to_char( "Strip what from whom?\n\r",ch);
        return;
    }
    if ( ( victim = get_char_room( ch, NULL, arg2)) == NULL ) 
    {
        send_to_char( "They aren't here.\n\r",ch);
        return;
    }

    if (victim == ch)
    {
       send_to_char( "That's pointless.\n\r",ch);
       return;
    }

    if (is_safe(ch,victim))
        return;

    if (IS_NPC(victim))
    {
	send_to_char( "Not on that one you don't.\n\r", ch);
	return;
    }
 
    if (victim->position == POS_FIGHTING)
    {
        send_to_char("You'd better not -- you might get hit.\n\r",ch);
        return;
    }

    if (ch->riding)
    {
      send_to_char("You can't do that while mounted.\n\r",ch);
      return;
    }

    tmp_ch = (is_affected(ch,gsn_doppelganger) && !IS_IMMORTAL(victim)) ? ch->doppel : ch;

    WAIT_STATE(ch, 20);
    if (IS_AWAKE(victim))
    {
       /* fail skill */
       send_to_char("Oops, you failed.\n\r",ch); 
       if (!IS_AFFECTED(victim,AFF_SLEEP))   
       {
          victim->position = POS_STANDING;

	  if (can_see(victim, ch))
	  {
           act( "$n tried to strip you.\n\r", ch, NULL, victim, TO_VICT);
           sprintf(buf,"Help!! %s is trying to undress me!",tmp_ch->name);
	     if (!IS_NPC(victim))
             do_yell( victim, buf ); 
	  }
	  else
	  {
            act( "Someone tried to strip you.\n\r", ch, NULL, victim, TO_VICT);
            sprintf(buf,"Help!! Someone is trying to undress me!");
            if (!IS_NPC(victim)) 
            do_yell( victim, buf ); 
        }
       }
        if (ch->pcdata->learned[gsn_strip] > 1)
            check_improve(ch,gsn_strip,FALSE,2);
        return;
     }

    obj = NULL;

    if ( ( obj = get_obj_wear( victim, arg1, TRUE)) == NULL)
    {
        send_to_char("You can't find it.\n\r",ch);
        return;
    }

    if ( !can_drop_obj( ch, obj )
    ||   IS_SET(obj->extra_flags, ITEM_NOREMOVE))
    {
       send_to_char( "You can't pry it away.\n\r", ch );
       return;
    }

    if ( ch->carry_number + get_obj_number( obj ) > can_carry_n( ch ) )
    {
        send_to_char( "You have your hands full.\n\r", ch );
        return;
    }

    if ( ch->carry_weight + get_obj_weight( obj ) > can_carry_w( ch ) )
    {
        send_to_char( "You can't carry that much weight.\n\r", ch );
        return;
    }

    if ( IS_AFFECTED(ch,AFF_HIDE))
    {
        REMOVE_BIT( ch->affected_by, AFF_HIDE );
        act("You step out of the shadows.", ch, NULL, NULL, TO_CHAR); 
        if ( !IS_AFFECTED(ch,AFF_SNEAK)) 
        act("$n steps out of the shadows.", ch, NULL, NULL, TO_ROOM);
    }
      

    if ( !IS_AFFECTED(ch,AFF_SNEAK))
    act("$n strips $p from the sleeping body of $N.\n\r",ch,obj,victim,TO_ROOM);
    act("You strip $p from the sleeping body of $N.\n\r",ch,obj,victim,TO_CHAR);
    obj_from_char( obj );
    obj_to_char(obj,ch);
    WAIT_STATE(ch, 20);
    check_improve(ch,gsn_strip,TRUE,2);
    return;
}

void do_polish(CHAR_DATA *ch, char *argument)
{
  OBJ_DATA *obj;
  AFFECT_DATA af;
  AFFECT_DATA *paf;
  int skill;

  /* find out what */
  if (argument[0] == '\0')
  {
    send_to_char("What would you like to polish?\n\r",ch);
    return;
  }

  obj =  get_obj_list(ch,argument,ch->carrying);

  if (obj== NULL)
  {
    send_to_char("You don't have that item.\n\r",ch);
    return;
  }

  if ((skill = get_skill(ch,gsn_polish)) < 1)
  {
    send_to_char("You smear some polish around, but give up in frustration.\n\r",ch);
    return;
  }

  if (obj->item_type != ITEM_ARMOR
  &&  obj->item_type != ITEM_WEAPON)
  {
    send_to_char("You can only polish armor and weapons.\n\r",ch);
    return;
  }

  if(str_cmp(obj->material, "iron")
  && str_cmp(obj->material, "silver")
  && str_cmp(obj->material, "mithril")
  && str_cmp(obj->material, "platinum")
  && str_cmp(obj->material, "titanium")
  && str_cmp(obj->material, "copper")
  && str_cmp(obj->material, "bronze")
  && str_cmp(obj->material, "gold")
  && str_cmp(obj->material, "pewter")
  && str_cmp(obj->material, "tin")
  && str_cmp(obj->material, "steel"))
  {
    send_to_char("The object your trying to polish is not made from a metal material, hence you can not polish it.\n\r",ch);
    return;
  }

  for(paf = obj->affected; paf != NULL; paf = paf->next)
  {
    if(paf->type == gsn_polish)
    {
	act("$p is already polished to a gleam, and requires no further polishing at this time.",ch,obj,NULL,TO_CHAR);
	return;
    }
  }

  if (number_percent() < skill)  /* success! */
  {
    act("$n polishes $p to a brilliant shine.",ch,obj,NULL,TO_ROOM);
    act("You polish $p to a brilliant shine.",ch,obj,NULL,TO_CHAR);
    af.where		= TO_OBJECT;
    af.type			= gsn_polish;
    af.level		= ch->level;
    af.duration 		= number_range(5,15);
    af.location 		= APPLY_CHR;
    af.modifier 		= 3;
    af.bitvector 		= 0;
    affect_to_obj(obj,&af);
    WAIT_STATE(ch,skill_table[gsn_polish].beats);
    check_improve(ch,gsn_polish,TRUE,1);
    return;
  }
  else
  {
    act("You fail to polish $p to a brilliant shine.",ch,obj,NULL,TO_CHAR);
    check_improve(ch,gsn_polish,FALSE,1);
    WAIT_STATE(ch,skill_table[gsn_polish].beats);
    return;
  }

  act("You can't polish $p.",ch,obj,NULL,TO_CHAR);
  return;
}


