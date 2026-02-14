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
#include <stat.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include "merc.h"
#include "interp.h"
#include "recycle.h"
#include "tables.h"
#include "lookup.h"
#include "olc.h"
#include <unistd.h>
#include <stddef.h>
#include <dirent.h>
/*
 * Local functions.
 */
void 	  append_note	args(	(NOTE_DATA *pnote) );
ROOM_INDEX_DATA *	find_location	args( ( CHAR_DATA *ch, char *arg ) );
extern bool write_to_descriptor( int desc, char *txt, int length );
extern bool check_parse_name( char *name );
extern char *initial( const char *str );
char *  size_name args( (int size) );

void do_wiznet( CHAR_DATA *ch, char *argument )
{
    int flag;
    char buf[MAX_STRING_LENGTH];

    if ( argument[0] == '\0' )
    {
      	if (IS_SET(ch->wiznet,WIZ_ON))
      	{
            send_to_char("Signing off of Wiznet.\n\r",ch);
            REMOVE_BIT(ch->wiznet,WIZ_ON);
      	}
      	else
      	{
            send_to_char("Welcome to Wiznet!\n\r",ch);
            SET_BIT(ch->wiznet,WIZ_ON);
      	}
      	return;
    }

    if (!str_prefix(argument,"on"))
    {
	send_to_char("Welcome to Wiznet!\n\r",ch);
	SET_BIT(ch->wiznet,WIZ_ON);
	return;
    }

    if (!str_prefix(argument,"off"))
    {
	send_to_char("Signing off of Wiznet.\n\r",ch);
	REMOVE_BIT(ch->wiznet,WIZ_ON);
	return;
    }

    /* show wiznet status */
    if (!str_prefix(argument,"status")) 
    {
	buf[0] = '\0';

	if (!IS_SET(ch->wiznet,WIZ_ON))
	    strcat(buf,"off ");

	for (flag = 0; wiznet_table[flag].name != NULL; flag++)
	    if (IS_SET(ch->wiznet,wiznet_table[flag].flag))
	    {
		strcat(buf,wiznet_table[flag].name);
		strcat(buf," ");
	    }

	strcat(buf,"\n\r");

	send_to_char("Wiznet status:\n\r",ch);
	send_to_char(buf,ch);
	return;
    }

    if (!str_prefix(argument,"show"))
    /* list of all wiznet options */
    {
	buf[0] = '\0';

	for (flag = 0; wiznet_table[flag].name != NULL; flag++)
	{
	    if (wiznet_table[flag].level <= get_trust(ch))
	    {
	    	strcat(buf,wiznet_table[flag].name);
	    	strcat(buf," ");
	    }
	}

	strcat(buf,"\n\r");

	send_to_char("Wiznet options available to you are:\n\r",ch);
	send_to_char(buf,ch);
	return;
    }
   
    flag = wiznet_lookup(argument);

    if (flag == -1 || get_trust(ch) < wiznet_table[flag].level)
    {
	send_to_char("No such option.\n\r",ch);
	return;
    }
   
    if (IS_SET(ch->wiznet,wiznet_table[flag].flag))
    {
	sprintf(buf,"You will no longer see %s on wiznet.\n\r",
	        wiznet_table[flag].name);
	send_to_char(buf,ch);
	REMOVE_BIT(ch->wiznet,wiznet_table[flag].flag);
    	return;
    }
    else
    {
    	sprintf(buf,"You will now see %s on wiznet.\n\r",
		wiznet_table[flag].name);
	send_to_char(buf,ch);
    	SET_BIT(ch->wiznet,wiznet_table[flag].flag);
	return;
    }

}

void wiznet(char *string, CHAR_DATA *ch, OBJ_DATA *obj,
	    long flag, long flag_skip, int min_level) 
{
    DESCRIPTOR_DATA *d;
    int loopy;
    int min_wiz_level = 0;

    /* min_wiz_level is the level required to even have the flag. Flags higher then
     * the immortal's current trust can be displayed if they previously had that trust,
     * and the flag was turned on. This usually happens in a Demotion scenario.
     */
    for(loopy = 0; wiznet_table[loopy].name != NULL; loopy++)
    {
	/* do not show us wiznet messages higher then our trust */
	if(wiznet_table[loopy].flag == flag)
		min_wiz_level = wiznet_table[loopy].level; 
    }

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        if (d->connected == CON_PLAYING
	&&  IS_IMMORTAL(d->character) 
	&&  IS_SET(d->character->wiznet,WIZ_ON) 
	&&  (!flag || IS_SET(d->character->wiznet,flag))
	&&  (!flag_skip || !IS_SET(d->character->wiznet,flag_skip))
	&&  get_trust(d->character) >= min_level
	&&  get_trust(d->character) >= min_wiz_level
	&&  d->character != ch)
        {
	    if (IS_SET(d->character->wiznet,WIZ_PREFIX))
	  	send_to_char("{Z--> ",d->character);
	    else
	  	send_to_char( "{Z", d->character );
            act_new(string,d->character,obj,ch,TO_CHAR,POS_DEAD);
			send_to_char( "{x", d->character );
        }
    }
 
    return;
}


/* equips a character */
void do_outfit ( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;

    if (ch->level > 5 || IS_NPC(ch))
    {
	send_to_char("Find it yourself!\n\r",ch);
	return;
    }

    if (ch->carry_number+4 > can_carry_n(ch))
    {
         send_to_char("You are carrying too much, try dropping some items.\n\r",ch);
         return;
    }

    if ( ( obj = get_eq_char( ch, WEAR_LIGHT ) ) == NULL )
    {
	if ( ch->class == class_lookup("necromancer"))
	{
        obj = create_object( get_obj_index(OBJ_VNUM_BLOOD_CANDLE), 0 );
	}
	else
	if ( ch->class == class_lookup("enchanter"))
	{
        obj = create_object( get_obj_index(OBJ_VNUM_SILVER_ORB), 0 );
	}
	else
	if ( ch->class == class_lookup("thief"))
	{
        obj = create_object( get_obj_index(OBJ_VNUM_HAND_LAMP), 0 );
	}
	else
	if ( ch->class == class_lookup("warrior")
	||   ch->class == class_lookup("knight")
	||   ch->class == class_lookup("barbarian"))
	{
        obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_LANTERN), 0 );
	}
	else
	if ( ch->class == class_lookup("cleric"))
	{
        obj = create_object( get_obj_index(OBJ_VNUM_CRYSTAL_ORB), 0 );
	}
	else
        obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_BANNER), 0 );

      obj_to_char( obj, ch );
      equip_char( ch, obj, WEAR_LIGHT );
    }
 
    if ( ( obj = get_eq_char( ch, WEAR_BODY ) ) == NULL )
    {
	if ( ch->class == class_lookup("necromancer"))
	{
        obj = create_object( get_obj_index(OBJ_VNUM_BLACK_ROBE), 0 );
	}
	else
	if ( ch->class == class_lookup("enchanter"))
	{
        obj = create_object( get_obj_index(OBJ_VNUM_SILVER_ROBE), 0 );
	}
	else
	if ( ch->class == class_lookup("thief"))
	{
        obj = create_object( get_obj_index(OBJ_VNUM_SILENCED_ARMOR), 0 );
	}
	else
	if ( ch->class == class_lookup("warrior")
	||   ch->class == class_lookup("knight")
	||   ch->class == class_lookup("barbarian"))
	{
        obj = create_object( get_obj_index(OBJ_VNUM_LOOSE_SHIRT), 0 );
	}
	else
	if ( ch->class == class_lookup("cleric"))
	{
        obj = create_object( get_obj_index(OBJ_VNUM_BLUE_ROBE), 0 );
	}
	else
	  obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_VEST), 0 );

      obj_to_char( obj, ch );
      equip_char( ch, obj, WEAR_BODY );
    }

    if ( ( obj = get_eq_char( ch, WEAR_FEET ) ) == NULL )
    { 
	if ( ch->class == class_lookup("necromancer"))
	{
        obj = create_object( get_obj_index(OBJ_VNUM_BLACK_BOOTS), 0 );
	}
	else
	if ( ch->class == class_lookup("enchanter"))
	{
        obj = create_object( get_obj_index(OBJ_VNUM_CLOTH_BOOTS), 0 );
	}
	else
	if ( ch->class == class_lookup("thief"))
	{
        obj = create_object( get_obj_index(OBJ_VNUM_CLAWED_SHOES), 0 );
	}
	else
	if ( ch->class == class_lookup("warrior")
	||   ch->class == class_lookup("knight")
	||   ch->class == class_lookup("barbarian"))
	{
        obj = create_object( get_obj_index(OBJ_VNUM_LEATHER_BOOTS), 0 );
	}
	else
	if ( ch->class == class_lookup("cleric"))
	{
        obj = create_object( get_obj_index(OBJ_VNUM_SANDALS), 0 );
	}
	else
	  obj = create_object( get_obj_index(OBJ_VNUM_NEWBIE_BOOTS), 0 );

      obj_to_char( obj, ch );
      equip_char( ch, obj, WEAR_FEET );
    }

    if( (obj = get_eq_char(ch, WEAR_WIELD) ) == NULL
    &&   ch->class == class_lookup("blacksmith") )
    {
	obj = create_object( get_obj_index(5710), 0 );
	obj_to_char(obj, ch);
	equip_char(ch, obj, WEAR_WIELD);
    }

    if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL
    &&     ch->class != class_lookup("blacksmith"))
    { 
        obj = create_object( get_obj_index(class_table[ch->class].weapon),0);
        obj_to_char( obj, ch );
        equip_char( ch, obj, WEAR_WIELD );
    }

    if ( ( obj = get_eq_char( ch, WEAR_SHIELD ) ) == NULL )
    {
        obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_SHIELD), 0 );
        obj->cost = 0;
        obj_to_char( obj, ch );
        equip_char( ch, obj, WEAR_SHIELD );
    }

    if ( ch->class == class_lookup("necromancer"))
    {
        obj = create_object( get_obj_index(OBJ_VNUM_BOOK_DEAD), 0 );
    	  obj_to_char( obj, ch );
    }

    if ( ch->class == class_lookup("enchanter"))
    {
        obj = create_object( get_obj_index(OBJ_VNUM_ENCHANT_BOOK), 0 );
    	  obj_to_char( obj, ch );
    }

    if ( ch->class == class_lookup("warrior")
    ||   ch->class == class_lookup("knight")
    ||   ch->class == class_lookup("barbarian"))
    {
        obj = create_object( get_obj_index(OBJ_VNUM_DISCIPLINE), 0 );
    	  obj_to_char( obj, ch );
    }

    if ( ch->class == class_lookup("cleric"))
    {
        obj = create_object( get_obj_index(OBJ_VNUM_SPIRITUALISM), 0 );
    	  obj_to_char( obj, ch );
    }

    obj = create_object( get_obj_index(OBJ_VNUM_MAP), 0 );
    obj_to_char( obj, ch );

    obj = create_object( get_obj_index(OBJ_VNUM_MAP2), 0 );
    obj_to_char( obj, ch );

    send_to_char( "You have been equipped by the Gods.", ch );
    return;
}

     
/* RT nochannels command, for those spammers */
void do_nochannels( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
 
    one_argument( argument, arg );
 
    if ( arg[0] == '\0' )
    {
        send_to_char( "Nochannel whom?\n\r", ch );
        return;
    }
 
    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }
 
    if ( get_trust( victim ) >= get_trust( ch ) )
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }
 
    if ( IS_SET(victim->comm, COMM_NOCHANNELS) )
    {
        REMOVE_BIT(victim->comm, COMM_NOCHANNELS);
        send_to_char( "The gods have restored your channel privileges.\n\r", victim );
        send_to_char( "NOCHANNELS removed.\n\r", ch );
	sprintf(buf,"$N restores channels to %s",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
    else
    {
        SET_BIT(victim->comm, COMM_NOCHANNELS);
        send_to_char( "The gods have revoked your channel privileges.\n\r", victim );
        send_to_char( "NOCHANNELS set.\n\r", ch );
	sprintf(buf,"$N revokes %s's channels.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
 
    return;
}

void do_bamfin( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( !IS_NPC(ch))
    {
	smash_tilde( argument );

	if (argument[0] == '\0')
	{
	    sprintf(buf,"Your poofin is %s\n\r",ch->pcdata->bamfin);
	    send_to_char(buf,ch);
	    return;
	}

	if ( strstr(argument,ch->name) == NULL)
	{
	    send_to_char("You must include your name.\n\r",ch);
	    return;
	}
	     
	free_string( ch->pcdata->bamfin );
	ch->pcdata->bamfin = str_dup( argument );

        sprintf(buf,"Your poofin is now %s\n\r",ch->pcdata->bamfin);
        send_to_char(buf,ch);
    }
    return;
}

void do_bamfout( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
 
    if ( !IS_NPC(ch) )
    {
        smash_tilde( argument );
 
        if (argument[0] == '\0')
        {
            sprintf(buf,"Your poofout is %s\n\r",ch->pcdata->bamfout);
            send_to_char(buf,ch);
            return;
        }
 
        if ( strstr(argument,ch->name) == NULL)
        {
            send_to_char("You must include your name.\n\r",ch);
            return;
        }
 
        free_string( ch->pcdata->bamfout );
        ch->pcdata->bamfout = str_dup( argument );
 
        sprintf(buf,"Your poofout is now %s\n\r",ch->pcdata->bamfout);
        send_to_char(buf,ch);
    }
    return;
}

void do_disconnect( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Disconnect whom?\n\r", ch );
	return;
    }

    if (is_number(arg))
    {
	int desc;

	desc = atoi(arg);
    	for ( d = descriptor_list; d != NULL; d = d->next )
    	{
            if ( d->descriptor == desc )
            {
            	close_socket( d );
            	send_to_char( "Ok.\n\r", ch );
            	return;
            }
	}
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->desc == NULL )
    {
	act( "$N doesn't have a descriptor.", ch, NULL, victim, TO_CHAR );
	return;
    }

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d == victim->desc )
	{
	    close_socket( d );
	    send_to_char( "Ok.\n\r", ch );
	    return;
	}
    }

    bug( "Do_disconnect: desc not found.", 0 );
    send_to_char( "Descriptor not found!\n\r", ch );
    return;
}



void do_pardon( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

   if ( !IS_NPC( ch ) && get_skill(ch,gsn_charge) < 1 )
    {
      send_to_char("You have no idea how to pardon people from a crime.\n\r", ch);
      return;
    }

    if (IS_NPC(ch))
    {
      send_to_char("You can't.\n\r", ch);
      return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Syntax: pardon <character> <murder|theft|assault|treason|outlaw>.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if (IS_IMMORTAL(victim) && (ch->level < victim->level))
    {
	send_to_char( "You don't have the power to pardon that person.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "murder" ) )
    {
	if ( IS_SET(victim->act, PLR_KILLER) )
	{
	    REMOVE_BIT( victim->act, PLR_KILLER );
	    send_to_char( "Murder has been pardoned.\n\r", ch );
	    send_to_char( "You are no longer wanted for MURDER.\n\r", victim );
          victim->pcdata->pardons[0][1]++;
	}
      else
	{
          send_to_char( "They were not flagged as a Murderer.\n\r", ch );
	}
	return;
    }

    if ( !str_cmp( arg2, "theft" ) )
    {
	if ( IS_SET(victim->act, PLR_THIEF) )
	{
	    REMOVE_BIT( victim->act, PLR_THIEF );
	    send_to_char( "The theft has been pardoned.\n\r", ch );
	    send_to_char( "You are no longer wanted for THEFT.\n\r", victim );
	    victim->pcdata->pardons[0][0]++;
	}
      else 
	{
           send_to_char( "They were not flagged as a Thief.\n\r", ch );
	}
	return;
    }

    if ( !str_cmp( arg2, "assault" ) )
    {
	if ( IS_SET(victim->act, PLR_ASSAULT) )
	{
	    REMOVE_BIT( victim->act, PLR_ASSAULT );
	    send_to_char( "The assault has been pardoned.\n\r", ch );
	    send_to_char( "You are no longer wanted for ASSAULT.\n\r", victim );
	    victim->pcdata->pardons[0][3]++;
	}
      else
	{
          send_to_char( "They were not wanted for assault.\n\r", ch );
	}
	return;
    }

    if ( !str_cmp( arg2, "treason" ) )
    {
	if ( IS_SET(victim->act, PLR_TREASON) )
	{
	    REMOVE_BIT( victim->act, PLR_TREASON );
	    send_to_char( "Treason has been pardoned.\n\r", ch );
	    send_to_char( "You are no longer a TREASONER of your clan.\n\r", victim );
	    victim->pcdata->pardons[0][2]++;
	}
      else
	{
          send_to_char( "They were not flagged as a TRAITOR.\n\r", ch );
	}
	return;
    }

    if ( !str_cmp( arg2, "outlaw" ) )
    {
	if ( IS_SET(victim->act, PLR_OUTLAW) )
	{
	    REMOVE_BIT( victim->act, PLR_OUTLAW );
	    send_to_char( "The Outlaw has been pardoned.\n\r", ch );
	    send_to_char( "You are no longer an OUTLAW.\n\r", victim );
	    victim->pcdata->pardons[0][4]++;
       }
       else
	 {
          send_to_char( "They were not flagged as an OUTLAW.\n\r", ch );
	 }
	return;
    }

    send_to_char( "Syntax: pardon <character> <killer|thief|assault|treason|outlaw>.\n\r", ch );
    return;
}



void do_echo( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;
    
    if ( argument[0] == '\0' )
    {
	send_to_char( "Global echo what?\n\r", ch );
	return;
    }
    
    for ( d = descriptor_list; d; d = d->next )
    {
	if ( d->connected == CON_PLAYING )
	{
	    if (get_trust(d->character) >= get_trust(ch))
		send_to_char( "global> ",d->character);
	    send_to_char( argument, d->character );
	    send_to_char( "\n\r",   d->character );
	}
    }

    return;
}



void do_recho( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;
    
    if ( argument[0] == '\0' )
    {
	send_to_char( "Local echo what?\n\r", ch );

	return;
    }

    for ( d = descriptor_list; d; d = d->next )
    {
	if ( d->connected == CON_PLAYING
	&&   d->character->in_room == ch->in_room )
	{
            if (get_trust(d->character) >= get_trust(ch))
                send_to_char( "local> ",d->character);
	    send_to_char( argument, d->character );
	    send_to_char( "\n\r",   d->character );
	}
    }

    return;
}

void do_zecho(CHAR_DATA *ch, char *argument)
{
    DESCRIPTOR_DATA *d;

    if (argument[0] == '\0')
    {
	send_to_char("Zone echo what?\n\r",ch);
	return;
    }

    for (d = descriptor_list; d; d = d->next)
    {
	if (d->connected == CON_PLAYING
	&&  d->character->in_room != NULL && ch->in_room != NULL
	&&  d->character->in_room->area == ch->in_room->area)
	{
	    if (get_trust(d->character) >= get_trust(ch))
		send_to_char("zone> ",d->character);
	    send_to_char(argument,d->character);
	    send_to_char("\n\r",d->character);
	}
    }
}

void do_pecho( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    argument = one_argument(argument, arg);
 
    if ( argument[0] == '\0' || arg[0] == '\0' )
    {
	send_to_char("Personal echo what?\n\r", ch); 
	return;
    }
   
    if  ( (victim = get_char_world(ch, arg) ) == NULL )
    {
	send_to_char("Target not found.\n\r",ch);
	return;
    }

    if (get_trust(victim) >= get_trust(ch) && get_trust(ch) != MAX_LEVEL)
        send_to_char( "personal> ",victim);

    send_to_char(argument,victim);
    send_to_char("\n\r",victim);
    send_to_char( "personal> ",ch);
    send_to_char(argument,ch);
    send_to_char("\n\r",ch);
}


ROOM_INDEX_DATA *find_location( CHAR_DATA *ch, char *arg )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    if ( is_number(arg) )
	return get_room_index( atoi( arg ) );

    if ( ( victim = get_char_world( ch, arg ) ) != NULL )
	return victim->in_room;

    if ( ( obj = get_obj_world( ch, arg ) ) != NULL )
	return obj->in_room;

    return NULL;
}



void do_transfer( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Transfer whom (and where)?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg1, "all" ) )
    {
	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->connected == CON_PLAYING
	    &&   d->character != ch
	    &&   d->character->in_room != NULL
	    &&   can_see( ch, d->character ) )
	    {
		char buf[MAX_STRING_LENGTH];
		sprintf( buf, "%s %s", d->character->name, arg2 );
		do_function(ch, &do_transfer, buf );
	    }
	}
	return;
    }

    /*
     * Thanks to Grodyn for the optional location parameter.
     */
    if ( arg2[0] == '\0' )
    {
	location = ch->in_room;
    }
    else
    {
	if ( ( location = find_location( ch, arg2 ) ) == NULL )
	{
	    send_to_char( "No such location.\n\r", ch );
	    return;
	}

	if ( !is_room_owner(ch,location) && room_is_private( location ) 
	&&  get_trust(ch) < MAX_LEVEL)
	{
	    send_to_char( "That room is private right now.\n\r", ch );
	    return;
	}
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->in_room == NULL )
    {
	send_to_char( "They are in limbo.\n\r", ch );
	return;
    }

    if ( victim->fighting != NULL )
	stop_fighting( victim, TRUE );

    if ( ch->desc->original == NULL )
    {
        if (ch->pcdata->transout[0] != '\0')
            act("$n is $t",victim,ch->pcdata->transout, NULL, TO_ROOM );
        else
            act( "$n disappears in a mushroom cloud.", victim, NULL, NULL, TO_ROOM );
    }
    else
        act( "$n disappears in a mushroom cloud.", victim, NULL, NULL, TO_ROOM );

    char_from_room( victim );
    char_to_room( victim, location );
    if (victim->riding && !IS_NPC(victim))
    {
	victim->riding->rider = NULL;
	victim->riding        = NULL;  
    }

    if (victim->rider && IS_NPC(victim))
    {
     victim->rider->riding	= NULL;
     victim->rider->position 	= POS_RESTING;
     victim->rider		= NULL;
    }

    if ( ch->desc->original == NULL ) //C068
    {
        if (ch->pcdata->transin[0] != '\0')
            act("$n $t",victim,ch->pcdata->transin, NULL, TO_ROOM );
        else
            act( "$n arrives from a puff of smoke.", victim, NULL, NULL, TO_ROOM );
    }
    else
        act( "$n arrives from a puff of smoke.", victim, NULL, NULL, TO_ROOM );

    if ( ch != victim )
	act( "$n has transferred you.", ch, NULL, victim, TO_VICT );
    do_function(victim, &do_look, "auto" );
    send_to_char( "Ok.\n\r", ch );
}



void do_at( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    ROOM_INDEX_DATA *original;
    OBJ_DATA *on;
    CHAR_DATA *wch;
    
    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "At where what?\n\r", ch );
	return;
    }

    if ( ( location = find_location( ch, arg ) ) == NULL )
    {
	send_to_char( "No such location.\n\r", ch );
	return;
    }

    if (!is_room_owner(ch,location) && room_is_private( location ) 
    &&  get_trust(ch) < MAX_LEVEL)
    {
	send_to_char( "That room is private right now.\n\r", ch );
	return;
    }

    original = ch->in_room;
    on = ch->on;
    char_from_room( ch );
    char_to_room( ch, location );
    interpret( ch, argument );

    /*
     * See if 'ch' still exists before continuing!
     * Handles 'at XXXX quit' case.
     */
     for ( wch = char_list; wch != NULL; wch = wch->next )
     {
	if ( wch == ch )
	{
	    OBJ_DATA *obj;

	    char_from_room( ch );
	    char_to_room( ch, original );

	    /* See if on still exists before setting ch->on back to it. */
	    for ( obj = original->contents; obj; obj = obj->next_content )
	    {
		if ( obj == on )
		{
		    ch->on = on;
		    break;
		}
	    }
	    break;
	}
     }

    return;
}



void do_goto( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *location;
    CHAR_DATA *rch;
    int count = 0;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Goto where?\n\r", ch );
	return;
    }

    if ( ( location = find_location( ch, argument ) ) == NULL )
    {
	send_to_char( "No such location.\n\r", ch );
	return;
    }

    count = 0;
    for ( rch = location->people; rch != NULL; rch = rch->next_in_room )
        count++;

    if (!is_room_owner(ch,location) && room_is_private(location) 
    &&  (count > 1 || get_trust(ch) < MAX_LEVEL))
    {
	send_to_char( "That room is private right now.\n\r", ch );
	return;
    }

    if ( ch->fighting != NULL )
	stop_fighting( ch, TRUE );

    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
	if (get_trust(rch) >= ch->invis_level)
	{
	    if (ch->pcdata != NULL && ch->pcdata->bamfout[0] != '\0')
		act("$t",ch,ch->pcdata->bamfout,rch,TO_VICT);
	    else
		act("$n leaves in a swirling mist.",ch,NULL,rch,TO_VICT);
	}
    }

    char_from_room( ch );
    char_to_room( ch, location );


    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
        if (get_trust(rch) >= ch->invis_level)
        {
            if (ch->pcdata != NULL && ch->pcdata->bamfin[0] != '\0')
                act("$t",ch,ch->pcdata->bamfin,rch,TO_VICT);
            else
                act("$n appears in a swirling mist.",ch,NULL,rch,TO_VICT);
        }
    }

    do_function(ch, &do_look, "auto" );
    return;
}

void do_violate( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *location;
    CHAR_DATA *rch;
 
    if ( argument[0] == '\0' )
    {
        send_to_char( "Goto where?\n\r", ch );
        return;
    }
 
    if ( ( location = find_location( ch, argument ) ) == NULL )
    {
        send_to_char( "No such location.\n\r", ch );
        return;
    }

    if (!room_is_private( location ))
    {
        send_to_char( "That room isn't private, use goto.\n\r", ch );
        return;
    }
 
    if ( ch->fighting != NULL )
        stop_fighting( ch, TRUE );
 
    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
        if (get_trust(rch) >= ch->invis_level)
        {
            if (ch->pcdata != NULL && ch->pcdata->bamfout[0] != '\0')
                act("$t",ch,ch->pcdata->bamfout,rch,TO_VICT);
            else
                act("$n leaves in a swirling mist.",ch,NULL,rch,TO_VICT);
        }
    }
 
    char_from_room( ch );
    char_to_room( ch, location );
 
 
    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
        if (get_trust(rch) >= ch->invis_level)
        {
            if (ch->pcdata != NULL && ch->pcdata->bamfin[0] != '\0')
                act("$t",ch,ch->pcdata->bamfin,rch,TO_VICT);
            else
                act("$n appears in a swirling mist.",ch,NULL,rch,TO_VICT);
        }
    }
 
    do_function(ch, &do_look, "auto" );
    return;
}

/* RT to replace the 3 stat commands */

void do_stat ( CHAR_DATA *ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char *string;
   OBJ_DATA *obj;
   ROOM_INDEX_DATA *location;
   CHAR_DATA *victim;

   string = one_argument(argument, arg);
   if ( arg[0] == '\0')
   {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  stat <name>\n\r",ch);
	send_to_char("  stat obj <name>\n\r",ch);
	send_to_char("  stat mob <name>\n\r",ch);
 	send_to_char("  stat room <number>\n\r",ch);
	return;
   }

   if (!str_cmp(arg,"room"))
   {
	do_function(ch, &do_rstat, string);
	return;
   }
  
   if (!str_cmp(arg,"obj"))
   {
	do_function(ch, &do_ostat, string);
	return;
   }

   if(!str_cmp(arg,"char")  || !str_cmp(arg,"mob"))
   {
	do_function(ch, &do_mstat, string);
	return;
   }
   
   /* do it the old way */

   obj = get_obj_world(ch,argument);
   if (obj != NULL)
   {
     do_function(ch, &do_ostat, argument);
     return;
   }

  victim = get_char_world(ch,argument);
  if (victim != NULL)
  {
    do_function(ch, &do_mstat, argument);
    return;
  }

  location = find_location(ch,argument);
  if (location != NULL)
  {
    do_function(ch, &do_rstat, argument);
    return;
  }

  send_to_char("Nothing by that name found anywhere.\n\r",ch);
}

void do_rstat( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    OBJ_DATA *obj;
    CHAR_DATA *rch;
    int door;

    one_argument( argument, arg );
    location = ( arg[0] == '\0' ) ? ch->in_room : find_location( ch, arg );
    if ( location == NULL )
    {
	send_to_char( "No such location.\n\r", ch );
	return;
    }

    if (!is_room_owner(ch,location) && ch->in_room != location 
    &&  room_is_private( location ) && !IS_TRUSTED(ch,IMPLEMENTOR))
    {
	send_to_char( "That room is private right now.\n\r", ch );
	return;
    }

    sprintf( buf, "Name: '%s'\n\rArea: '%s'\n\r",
	location->name,
	location->area->name );
    send_to_char( buf, ch );

    sprintf( buf,
	"Vnum: %d  Sector: %d  Light: %d  Healing: %d  Mana: %d\n\r",
	location->vnum,
	location->sector_type,
	location->light,
	location->heal_rate,
	location->mana_rate );
    send_to_char( buf, ch );

    sprintf( buf,
	"Room flags: %d.\n\rDescription:\n\r%s",
	location->room_flags,
	location->description );
    send_to_char( buf, ch );

    if ( location->extra_descr != NULL )
    {
	EXTRA_DESCR_DATA *ed;

	send_to_char( "Extra description keywords: '", ch );
	for ( ed = location->extra_descr; ed; ed = ed->next )
	{
	    send_to_char( ed->keyword, ch );
	    if ( ed->next != NULL )
		send_to_char( " ", ch );
	}
	send_to_char( "'.\n\r", ch );
    }

    send_to_char( "Characters:", ch );
    for ( rch = location->people; rch; rch = rch->next_in_room )
    {
	if (can_see(ch,rch))
        {
	    send_to_char( " ", ch );
	    one_argument( rch->name, buf );
	    send_to_char( buf, ch );
	}
    }

    send_to_char( ".\n\rObjects:   ", ch );
    for ( obj = location->contents; obj; obj = obj->next_content )
    {
	send_to_char( " ", ch );
	one_argument( obj->name, buf );
	send_to_char( buf, ch );
    }
    send_to_char( ".\n\r", ch );

    for ( door = 0; door <= 5; door++ )
    {
	EXIT_DATA *pexit;

	if ( ( pexit = location->exit[door] ) != NULL )
	{
	    sprintf( buf,
		"Door: %d.  To: %d.  Key: %d.  Exit flags: %d.\n\rKeyword: '%s'.  Description: %s",

		door,
		(pexit->u1.to_room == NULL ? -1 : pexit->u1.to_room->vnum),
	    	pexit->key,
	    	pexit->exit_info,
	    	pexit->keyword,
	    	pexit->description[0] != '\0'
		    ? pexit->description : "(none).\n\r" );
	    send_to_char( buf, ch );
	}
    }

    return;
}



void do_ostat( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    AFFECT_DATA *paf;
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Stat what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_world( ch, argument ) ) == NULL )
    {
	send_to_char( "Nothing like that in hell, earth, or heaven.\n\r", ch );
	return;
    }

    sprintf( buf, "Name(s): %s\n\r",
	obj->name );
    send_to_char( buf, ch );

    sprintf( buf, "Vnum: %d  Format: %s  Type: %s  Resets: %d\n\r",
	obj->pIndexData->vnum, obj->pIndexData->new_format ? "new" : "old",
	item_name(obj->item_type), obj->pIndexData->reset_num );
    send_to_char( buf, ch );

    sprintf( buf, "Short description: %s\n\rLong description: %s\n\r",
	obj->short_descr, obj->description );
    send_to_char( buf, ch );

    sprintf( buf, "Wear bits: %s\n\rExtra bits: %s\n\r",
	wear_bit_name(obj->wear_flags), extra_bit_name( obj->extra_flags ) );
    send_to_char( buf, ch );

    sprintf( buf, "Extra2 bits: %s\n\r",
	extra2_bit_name( obj->extra2_flags ) );
    send_to_char( buf, ch );

    sprintf( buf, "Number: %d/%d  Weight: %d stones  Material: %s\n\r",
	1,
      get_obj_number( obj ),
	obj->weight,
	obj->material );
    send_to_char( buf, ch );

    sprintf( buf, "Level: %d  Cost: %d  Condition: %d  Timer: %d\n\r",
	obj->level, obj->cost, obj->condition, obj->timer );
    send_to_char( buf, ch );

    sprintf( buf, "Size: %s\n\r",
	size_name( obj->size ) );
    send_to_char( buf, ch );

    sprintf( buf,
	"In room: %d  In object: %s  Carried by: %s  Wear_loc: %d\n\r",
	obj->in_room    == NULL    ?        0 : obj->in_room->vnum,
	obj->in_obj     == NULL    ? "(none)" : obj->in_obj->short_descr,
	obj->carried_by == NULL    ? "(none)" : 
	    can_see(ch,obj->carried_by) ? obj->carried_by->name
				 	: "someone",
	obj->wear_loc );
    send_to_char( buf, ch );
    
    sprintf( buf, "Values: %d %d %d %d %d\n\r",
	obj->value[0], obj->value[1], obj->value[2], obj->value[3],
	obj->value[4] );
    send_to_char( buf, ch );
    
    /* now give out vital statistics as per identify */
    
    switch ( obj->item_type )
    {
    	case ITEM_SCROLL: 
    	case ITEM_POTION:
    	case ITEM_PILL:
	    sprintf( buf, "Level %d spells of:", obj->value[0] );
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

	    if (obj->value[4] >= 0 && obj->value[4] < MAX_SKILL)
	    {
		send_to_char(" '",ch);
		send_to_char(skill_table[obj->value[4]].name,ch);
		send_to_char("'",ch);
	    }

	    send_to_char( ".\n\r", ch );
	break;

    	case ITEM_WAND: 
    	case ITEM_STAFF: 
	    sprintf( buf, "Has %d(%d) charges of level %d",
	    	obj->value[1], obj->value[2], obj->value[0] );
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

	case ITEM_BOOBYTRAP:
	    sprintf(buf,"A %s boobytrap.\n\r",
		trap_table[obj->value[1]].trap_name);
	    send_to_char(buf,ch);
	    break;

	case ITEM_TRAP:
	    sprintf(buf,"A %s trap.\n\r",
		trap_table[obj->value[1]].trap_name);
	    send_to_char(buf,ch);
	    break;
      
    	case ITEM_WEAPON:
 	    send_to_char("Weapon type is ",ch);
	    switch (obj->value[0])
	    {
	    	case(WEAPON_EXOTIC): 
		    send_to_char("exotic\n\r",ch);
		    break;
	    	case(WEAPON_SWORD): 
		    send_to_char("sword\n\r",ch);
		    break;	
	    	case(WEAPON_DAGGER): 
		    send_to_char("dagger\n\r",ch);
		    break;
	    	case(WEAPON_SPEAR):
		    send_to_char("spear\n\r",ch);
		    break;
	    	case(WEAPON_MACE): 
		    send_to_char("mace/club\n\r",ch);	
		    break;
	   	case(WEAPON_AXE): 
		    send_to_char("axe\n\r",ch);	
		    break;
	    	case(WEAPON_FLAIL): 
		    send_to_char("flail\n\r",ch);
		    break;
	    	case(WEAPON_WHIP): 
		    send_to_char("whip\n\r",ch);
		    break;
	    	case(WEAPON_POLEARM): 
		    send_to_char("polearm\n\r",ch);
		    break;
		case(WEAPON_BOW):
		    send_to_char("bow\n\r",ch);
		    break;
		case(WEAPON_STAFF):
		    send_to_char("staff\n\r",ch);
		    break;
	    	default: 
		    send_to_char("unknown\n\r",ch);
		    break;
 	    }
	    if (obj->pIndexData->new_format)
	    	sprintf(buf,"Damage is %dd%d (average %d)\n\r",
		    obj->value[1],obj->value[2],
		    (1 + obj->value[2]) * obj->value[1] / 2);
	    else
	    	sprintf( buf, "Damage is %d to %d (average %d)\n\r",
	    	    obj->value[1], obj->value[2],
	    	    ( obj->value[1] + obj->value[2] ) / 2 );
	    send_to_char( buf, ch );

	    sprintf(buf,"Damage noun is %s.\n\r",
		(obj->value[3] > 0 && obj->value[3] < MAX_DAMAGE_MESSAGE) ?
		    attack_table[obj->value[3]].noun : "undefined");
	    send_to_char(buf,ch);
	    
	    if (obj->value[4])  /* weapon flags */
	    {
	        sprintf(buf,"Weapons flags: %s\n\r",
		    weapon_bit_name(obj->value[4]));
	        send_to_char(buf,ch);
            }
	break;

    	case ITEM_ARMOR:
	    sprintf( buf, 
	    "Armor class is %d pierce, %d bash, %d slash, and %d vs. magic\n\r",
	        obj->value[0], obj->value[1], obj->value[2], obj->value[3] );
	    send_to_char( buf, ch );
	break;

    case ITEM_QUIVER:
	    sprintf(buf,"{WIt holds {R%d %d{Wd{R%d {Warrows.{x\n\r",
	    obj->value[0], obj->value[1], obj->value[2]);
	    send_to_char(buf,ch);
	    break;

    case ITEM_ARROW:
	    sprintf(buf,"{WThis arrow will do {R%d{Wd{R%d {Wdamage for an average of {R%d{W.{x\n\r",
	    obj->value[1], obj->value[2], ( obj->value[1] + obj->value[2] ) / 2 );
	    send_to_char(buf,ch);
	    break;

      case ITEM_CONTAINER:
            sprintf(buf,"Capacity: %d#  Maximum weight: %d#  flags: %s\n\r",
                obj->value[3], obj->value[0], cont_bit_name(obj->value[1]));
            send_to_char(buf,ch);
            if (obj->value[4] != 100)
            {
                sprintf(buf,"Weight multiplier: %d%%\n\r",
		    obj->value[4]);
                send_to_char(buf,ch);
            }
        break;
    }


    if ( obj->extra_descr != NULL || obj->pIndexData->extra_descr != NULL )
    {
	EXTRA_DESCR_DATA *ed;

	send_to_char( "Extra description keywords: '", ch );

	for ( ed = obj->extra_descr; ed != NULL; ed = ed->next )
	{
	    send_to_char( ed->keyword, ch );
	    if ( ed->next != NULL )
	    	send_to_char( " ", ch );
	}

	for ( ed = obj->pIndexData->extra_descr; ed != NULL; ed = ed->next )
	{
	    send_to_char( ed->keyword, ch );
	    if ( ed->next != NULL )
		send_to_char( " ", ch );
	}

	send_to_char( "'\n\r", ch );
    }

    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
	sprintf( buf, "Affects %s by %d, level %d",
	    affect_loc_name( paf->location ), paf->modifier,paf->level );
	send_to_char(buf,ch);
	if ( paf->duration > -1)
	    sprintf(buf,", %d hours.\n\r",paf->duration);
	else
	    sprintf(buf,".\n\r");
	send_to_char( buf, ch );
	if (paf->bitvector)
	{
	    switch(paf->where)
	    {
		case TO_AFFECTS:
		    sprintf(buf,"Adds %s affect.\n",
			affect_bit_name(paf->bitvector));
		    break;
                case TO_WEAPON:
                    sprintf(buf,"Adds %s weapon flags.\n",
                        weapon_bit_name(paf->bitvector));
		    break;
		case TO_OBJECT:
		    sprintf(buf,"Adds %s object flag.\n",
			extra_bit_name(paf->bitvector));
		    break;
		case TO_OBJECT2:
		    sprintf(buf,"Adds %s object flag.\n",
			extra2_bit_name(paf->bitvector));
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

    if (!obj->enchanted)
    for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
    {
	sprintf( buf, "Affects %s by %d, level %d.\n\r",
	    affect_loc_name( paf->location ), paf->modifier,paf->level );
	send_to_char( buf, ch );
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
                case TO_OBJECT2:
                    sprintf(buf,"Adds %s object flag.\n",
                        extra2_bit_name(paf->bitvector));
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

    return;
}



void do_mstat( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    AFFECT_DATA *paf;
    CHAR_DATA *victim;
    int xp_to_lev;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Stat whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, argument ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

	sprintf(buf,
	 "%s=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n\r",
	  IS_SET(ch->act, PLR_COLOUR) ? C_GREEN : "");
	send_to_char(buf, ch);

    if (IS_NPC(victim))
    {
    sprintf( buf, "Name:  %s\n\r",
	victim->name );
	send_to_char(buf,ch);
    }

    if ( !IS_NPC(victim) )
    {
    sprintf( buf, "Name:  %s %s%s\n\r",
	victim->name,
	victim->surname,
	victim->pcdata->title );
    send_to_char( buf, ch );
    }

    if (IS_NPC(victim))
    {
    sprintf( buf, "Level: %-12d  \n\r",
	victim->level );
    send_to_char(buf,ch);
    }


    if ( !IS_NPC(victim) )
    {
    sprintf( buf, "Level:   %3d                Age:    %-12d   Hours: %d \n\r",
	victim->level,
	get_age(victim), 
	(int) (victim->played + current_time - victim->logon) / 3600 );
    send_to_char( buf, ch );
    }

    sprintf( buf, 
	"Vnum:      %-10d       Format: %s \n\r",
	IS_NPC(victim) ? victim->pIndexData->vnum : 0,
	IS_NPC(victim) ? victim->pIndexData->new_format ? "new" : "old" : "pc"
	);
    send_to_char( buf, ch );

    if (IS_NPC(victim))
    {
	sprintf(buf,"Count: %-14d  Killed: %d\n\r",
	    victim->pIndexData->count,victim->pIndexData->killed);
	send_to_char(buf,ch);
    }

    if (IS_NPC(victim))
    {
	sprintf( buf, "Hunting: %s\n\r",
        victim->hunting    ? victim->hunting->name : "(none)");
      send_to_char( buf, ch );
    }

    if (!IS_NPC(victim) )
    {
	sprintf(buf, "Deathstat: %-10d       Deaths: %d\n\r", 
	    victim->deathstat,
	    victim->pcdata->deaths );
	send_to_char(buf, ch);
    }

    if (!IS_NPC(victim) ) 
    {
	sprintf( buf, "Lastfought:     %-13s\n\r",
	victim->lastfought ? victim->lastfought->name : "(none)" );
   	send_to_char( buf, ch );
    }

    if (!IS_NPC(victim) ) 
    {
	sprintf( buf, "Last_PC_Fought: %-20s               Quit Timer: %d\n\r",
      victim->last_pc_fought,  
	victim->quit_timer );
   	send_to_char( buf, ch );
    }

    if ( victim->clan_id != CLAN_NONE )
    {
    	  sprintf( buf,
	    "A member of the %s Clan%s\n\r",
		org_table[CLAN( victim->clan_id )].name,
		victim->clan_id & CLAN_LEADER ? ", and is it's Leader." : "." );
	  send_to_char( buf, ch );
    }

    if ( victim->org_id != ORG_NONE )
    {
    	  sprintf( buf,
	    "A member of the %s Clan%s\n\r",
		org_table[ ORG(victim->org_id) ].name,
		victim->org_id & ORG_LEADER ? ", and is it's Leader." : "." );
	  send_to_char( buf, ch );
    }

    if(!IS_NPC(victim))
    {
      if (victim->devotee_id && !victim->religion_id)
	  sprintf( buf, "%s (devotee)\n\r",
	  religion_table[victim->devotee_id].longname);
      else
	  sprintf( buf, "Religion: %s %s\n\r",
	  religion_table[RELIGION(victim->religion_id)].longname,
	  victim->religion_id & RELIGION_LEADER ? "(leader)" :
        ( victim->pcdata->highpriest == 1 ? "(high)" : "" ) );
      send_to_char( buf, ch );
    }

	sprintf(buf,
	 "%s-----------------------------------------------------------------\n\r",
	  IS_SET(ch->act, PLR_COLOUR) ? C_GREEN : "");
	send_to_char(buf, ch);

    if (IS_NPC(victim))
    {
    sprintf( buf, "HP:   %d/%-6d                                   Str: %d(%d) \n\r",
	victim->hit,
	victim->max_hit,
	victim->perm_stat[STAT_STR],
	get_curr_stat(victim,STAT_STR) );
    send_to_char(buf,ch);
    }


    if ( !IS_NPC(victim) )
    {
    sprintf( buf, "Hp:    %5d/%-5d                                 Str: %d(%d) \n\r",
	victim->hit,
	victim->max_hit,
	victim->perm_stat[STAT_STR],
	get_curr_stat(victim,STAT_STR) );
    send_to_char( buf, ch );
    }

    sprintf( buf, "Class: %-13s   Mana:   %5d/%-5d         Int: %d(%d) \n\r",
	IS_NPC(victim) ? "mobile" : class_table[victim->class].name, 
	victim->mana,
	victim->max_mana,
	victim->perm_stat[STAT_INT],
	get_curr_stat(victim,STAT_INT) );
    send_to_char(buf,ch);

    sprintf( buf, "Race:  %-13s   Move:   %5d/%-5d         Wis: %d(%d) \n\r",
	race_table[victim->race].name,
	victim->move,
	victim->max_move,
	victim->perm_stat[STAT_WIS],
	get_curr_stat(victim,STAT_WIS) );
    send_to_char(buf,ch);

    if (IS_NPC(victim))
    {
    sprintf( buf, "Sex:   %-13s   Items:  %-17d  Dex: %d(%d) \n\r",
	sex_table[victim->sex].name,
	victim->carry_number,
	victim->perm_stat[STAT_DEX],
	get_curr_stat(victim,STAT_DEX) );
    send_to_char(buf,ch);
    }

    if ( !IS_NPC(victim) )
    {
    sprintf( buf, "Sex:   %-14s  Items:  %5d/%-5d         Dex: %d(%d) \n\r",
	sex_table[victim->sex].name,
	victim->carry_number,
	can_carry_n(victim),
	victim->perm_stat[STAT_DEX],
	get_curr_stat(victim,STAT_DEX) );
    send_to_char( buf, ch );
    }

    if (IS_NPC(victim))
    {
    sprintf( buf, "Weight: %-13ld  Con: %d(%d) \n\r",
      get_carry_weight(victim) / 10,
	victim->perm_stat[STAT_CON],
	get_curr_stat(victim,STAT_CON) );
    send_to_char(buf,ch);
    }

    if ( !IS_NPC(victim) )
    {
    sprintf( buf, "Qp:    %-14d  Weight:%6ld/%-7d       Con: %d(%d) \n\r",
      victim->questpoints,
      get_carry_weight(victim)/10,
      can_carry_w(victim)/10,
	victim->perm_stat[STAT_CON],
	get_curr_stat(victim,STAT_CON) );
    send_to_char( buf, ch );
    }

    if ( !IS_NPC(victim) )
    {
    sprintf( buf, "                                                   Chr: %d(%d) \n\r",
        victim->perm_stat[STAT_CHR],
        get_curr_stat(victim,STAT_CHR));
    send_to_char( buf, ch );
    }

	sprintf(buf,
	 "%s-----------------------------------------------------------------\n\r",
	  IS_SET(ch->act, PLR_COLOUR) ? C_GREEN : "");
	send_to_char(buf, ch);

    sprintf( buf, "Steel:   %-12ld  Gold:   %ld \n\r",
	victim->steel, 
	victim->gold );
    send_to_char(buf,ch);

    if (IS_NPC(victim) && victim->pIndexData->new_format)
    {
	sprintf(buf, "Damage: %dd%-10d  Message:  %s\n\r",
	    victim->damage[DICE_NUMBER],victim->damage[DICE_TYPE],
	    attack_table[victim->dam_type].noun);
	send_to_char(buf,ch);
    }

    if (IS_NPC(victim))
    {
    sprintf( buf, "Pierce: %-13d  Room:   %d \n\r",
	    GET_AC(victim,AC_PIERCE),
	    victim->in_room == NULL    ?        0 : victim->in_room->vnum );
    send_to_char(buf,ch);
    }

    if ( !IS_NPC(victim) )
    {
    sprintf( buf, "Pierce: %-13d  Room:   %d \n\r",
	    GET_AC(victim,AC_PIERCE),
	    victim->in_room == NULL    ?        0 : victim->in_room->vnum );
    send_to_char( buf, ch );
    }

    if (IS_NPC(victim))
    {
    sprintf( buf, "Bash:   %-13d  Saves: %d \n\r",
	GET_AC(victim,AC_BASH),
	victim->saving_throw );
    send_to_char(buf,ch);
    }

    if ( !IS_NPC(victim) )
    {
    sprintf( buf, "Bash:   %-13d  Thirst: %-2d     Saves:      %d \n\r",
	GET_AC(victim,AC_BASH),
	victim->pcdata->condition[COND_THIRST],
	victim->saving_throw );
    send_to_char( buf, ch );
    }

    if (IS_NPC(victim))
    {
    sprintf( buf, "Slash:  %-13d  Group: %d \n\r",
	GET_AC(victim,AC_SLASH),
	IS_NPC(victim) ? victim->group : 0 );
    send_to_char(buf,ch);
    }

    if ( !IS_NPC(victim) )
    {
    sprintf( buf, "Slash:  %-13d  Full:   %d     Last Level: %-6d     Group: %d\n\r",
	GET_AC(victim,AC_SLASH),
	victim->pcdata->condition[COND_FULL],
	victim->pcdata->last_level, 
	IS_NPC(victim) ? victim->group : 0 );
    send_to_char( buf, ch );
    }

    if (IS_NPC(victim))
    {
    sprintf( buf, "Magic:  %-13d  Size: %s\n\r",
	GET_AC(victim,AC_EXOTIC),
	size_name(victim->size));
    send_to_char(buf,ch);
    }

    if ( !IS_NPC(victim) )
    {
    sprintf( buf, "Magic:  %-13d  Drunk:  %-3d    Size:      %-6s     Timer: %d\n\r",
	GET_AC(victim,AC_EXOTIC),
	victim->pcdata->condition[COND_DRUNK],
	size_name(victim->size),
	victim->timer );
    send_to_char( buf, ch );
    }

	sprintf(buf,
	 "%s-----------------------------------------------------------------\n\r",
	  IS_SET(ch->act, PLR_COLOUR) ? C_GREEN : "");
	send_to_char(buf, ch);

    if (IS_NPC(victim))
    {
    sprintf( buf, "Wimpy:    %-7d  Position: %s\n\r",
	victim->wimpy,
	position_table[victim->position].name );
    send_to_char(buf,ch);
    }

    if ( !IS_NPC(victim) )
    {
    sprintf( buf, "Wimpy:    %-7d      Trains:   %-4d         Practices:  %d\n\r",
	victim->wimpy,
	victim->train,
      victim->practice );
    send_to_char( buf, ch );
    }

    if (IS_NPC(victim))
    {
    sprintf( buf, "Hit Roll: %-8d     Dam Roll: %-5d        Alignment:  %d\n\r",
	GET_HITROLL(victim),
	GET_DAMROLL(victim),
	victim->alignment );
    send_to_char(buf,ch);
    }

    if ( !IS_NPC(victim) )
    {
    sprintf( buf, "Hit Roll: %-8d     Dam Roll: %-5d        Alignment:  %d\n\r",
	GET_HITROLL(victim),
	GET_DAMROLL(victim),
	victim->alignment );
    send_to_char( buf, ch );
    }

    xp_to_lev = exp_at_level( victim, victim->level + 1) - victim->exp;
    if ( !IS_NPC(victim) )
    {
    sprintf( buf, "Exp:      %-10d   To Level: %-11d  Ethos:      %s\n\r",
	victim->exp,
	xp_to_lev,
	get_ethos(victim) );
    send_to_char( buf, ch );
    }

    if (!IS_NPC(victim))
    {
    sprintf( buf, "Security: %-7d      Trust:    %-3d          Position:   %s\n\r",
	victim->pcdata->security,
	victim->trust,
      position_table[victim->position].name );
    send_to_char( buf, ch );
    }

	sprintf(buf,
	 "%s-----------------------------------------------------------------\n\r",
	  IS_SET(ch->act, PLR_COLOUR) ? C_GREEN : "");
	send_to_char(buf, ch);

    if (victim->comm)
    {
    	sprintf(buf,"Comm:  %s\n\r",comm_bit_name(victim->comm));
    	send_to_char(buf,ch);
    }

    sprintf(buf, "Act:  %s\n\r",act_bit_name(victim->act));
    send_to_char(buf,ch);

    if (victim->imm_flags)
    {
	sprintf(buf, "Immune:  %s\n\r",imm_bit_name(victim->imm_flags));
	send_to_char(buf,ch);
    }
 
    if (victim->res_flags)
    {
	sprintf(buf, "Resist:  %s\n\r", imm_bit_name(victim->res_flags));
	send_to_char(buf,ch);
    }

    if (victim->vuln_flags)
    {
      sprintf(buf, "Vulnerable: %s\n\r", imm_bit_name(victim->vuln_flags));
	send_to_char(buf,ch);
    }

    if (IS_NPC(victim))
    {
    sprintf(buf, "Form:  %s\n\rParts:       %s\n\r", 
	form_bit_name(victim->form), part_bit_name(victim->parts));
    send_to_char(buf,ch);
    }

    if (IS_NPC(victim))
    {
      sprintf( buf, "Spec_state: %d\n\r", victim->spec_state);
      send_to_char( buf,ch );
    }

    if (victim->affected_by)
    {
	sprintf(buf, "Affected by %s\n\r", 
	    affect_bit_name(victim->affected_by));
	send_to_char(buf,ch);
    }

    if (victim->affected2_by)
    {
         sprintf(buf, "Affected2 by %s\n\r",
            affect2_bit_name(victim->affected2_by) );
        send_to_char(buf,ch);
    }

    if (victim->affected3_by)
    {
         sprintf(buf, "Affected3 by %s\n\r",
            affect3_bit_name(victim->affected3_by) );
        send_to_char(buf,ch);
    }

    if ( IS_NPC(victim) && victim->spec_fun != 0 )
    {
	sprintf(buf,"Mobile has special procedure %s.\n\r",
		spec_name(victim->spec_fun));
	send_to_char(buf,ch);
    }

    if ( IS_NPC(victim)
    && (!victim->pIndexData->new_format))
    {
	sprintf(buf,"** This MOB is written in the OLD FORMAT **.\n\r");
	send_to_char(buf,ch);
    }

    for ( paf = victim->affected; paf != NULL; paf = paf->next )
    {
	sprintf( buf,
	    "Spell: '%s' modifies %s by %d for %d hours, level %d.\n\r",
	    skill_table[(int) paf->type].name,
	    affect_loc_name( paf->location ),
	    paf->modifier,
	    paf->duration,
	    paf->level
	    );
	send_to_char( buf, ch );
    }

	sprintf(buf,
	 "%s=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n\r",
	  IS_SET(ch->act, PLR_COLOUR) ? C_GREEN : "");
	send_to_char(buf, ch);

    return;
}

/* ofind and mfind replaced with vnum, vnum skill also added */

void do_vnum(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char *string;

    string = one_argument(argument,arg);
 
    if (arg[0] == '\0')
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  vnum obj <name>\n\r",ch);
	send_to_char("  vnum mob <name>\n\r",ch);
	send_to_char("  vnum skill <skill or spell>\n\r",ch);
	return;
    }

    if (!str_cmp(arg,"obj"))
    {
	do_function(ch, &do_ofind, string);
 	return;
    }

    if (!str_cmp(arg,"mob") || !str_cmp(arg,"char"))
    { 
	do_function(ch, &do_mfind, string);
	return;
    }

    if (!str_cmp(arg,"skill") || !str_cmp(arg,"spell"))
    {
	do_function (ch, &do_slookup, string);
	return;
    }
    /* do both */
    do_function(ch, &do_mfind, argument);
    do_function(ch, &do_ofind, argument);
}


void do_mfind( CHAR_DATA *ch, char *argument )
{
    extern int top_mob_index;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    MOB_INDEX_DATA *pMobIndex;
    int vnum;
    int nMatch;
    bool fAll;
    bool found;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Find whom?\n\r", ch );
	return;
    }

    fAll	= FALSE; /* !str_cmp( arg, "all" ); */
    found	= FALSE;
    nMatch	= 0;

    /*
     * Yeah, so iterating over all vnum's takes 10,000 loops.
     * Get_mob_index is fast, and I don't feel like threading another link.
     * Do you?
     * -- Furey
     */
    for ( vnum = 0; nMatch < top_mob_index; vnum++ )
    {
	if ( ( pMobIndex = get_mob_index( vnum ) ) != NULL )
	{
	    nMatch++;
	    if ( fAll || is_name( argument, pMobIndex->player_name ) )
	    {
		found = TRUE;
		sprintf( buf, "[%5d] %s\n\r",
		    pMobIndex->vnum, pMobIndex->short_descr );
		send_to_char( buf, ch );
	    }
	}
    }

    if ( !found )
	send_to_char( "No mobiles by that name.\n\r", ch );

    return;
}



void do_ofind( CHAR_DATA *ch, char *argument )
{
    extern int top_obj_index;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA *pObjIndex;
    int vnum;
    int nMatch;
    bool fAll;
    bool found;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Find what?\n\r", ch );
	return;
    }

    fAll	= FALSE; /* !str_cmp( arg, "all" ); */
    found	= FALSE;
    nMatch	= 0;

    /*
     * Yeah, so iterating over all vnum's takes 10,000 loops.
     * Get_obj_index is fast, and I don't feel like threading another link.
     * Do you?
     * -- Furey
     */
    for ( vnum = 0; nMatch < top_obj_index; vnum++ )
    {
	if ( ( pObjIndex = get_obj_index( vnum ) ) != NULL )
	{
	    nMatch++;
	    if ( fAll || is_name( argument, pObjIndex->name ) )
	    {
		found = TRUE;
		sprintf( buf, "[%5d] %s\n\r",
		    pObjIndex->vnum, pObjIndex->short_descr );
		send_to_char( buf, ch );
	    }
	}
    }

    if ( !found )
	send_to_char( "No objects by that name.\n\r", ch );

    return;
}


void do_owhere(CHAR_DATA *ch, char *argument )
{
    char buf[MAX_INPUT_LENGTH];
    BUFFER *buffer;
    OBJ_DATA *obj;
    OBJ_DATA *in_obj;
    bool found;
    int number = 0, max_found;

    found = FALSE;
    number = 0;
    max_found = 200;

    buffer = new_buf();

    if (argument[0] == '\0')
    {
	send_to_char("Find what?\n\r",ch);
	return;
    }
 
    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
        if (!can_see_obj( ch, obj )
	  ||  !is_name( argument, obj->name )
        ||   ch->level < obj->level
	  ||  ((obj->pIndexData->vnum < 1273 && obj->pIndexData->vnum > 1230)
	  &&	 (ch->level < 110)))
            continue;
 
        found = TRUE;
        number++;
 
        for ( in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj )
            ;
 
        if ( in_obj->carried_by != NULL && can_see(ch,in_obj->carried_by)
	  &&   in_obj->carried_by->in_room != NULL)
            sprintf( buf, "%3d) %s is carried by %s [Room %d]\n\r",
            number, 
		obj->short_descr,PERS(in_obj->carried_by, ch),
		in_obj->carried_by->in_room->vnum );

        else if (in_obj->in_room != NULL && can_see_room(ch,in_obj->in_room))
            sprintf( buf, "%3d) %s is in %s [Room %d]\n\r",
            number,
		obj->short_descr,
		in_obj->in_room->name, 
	   	in_obj->in_room->vnum);

	  else
            sprintf( buf, "%3d) %s is somewhere\n\r",number, obj->short_descr);
 
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
}


void do_mwhere( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    BUFFER *buffer;
    CHAR_DATA *victim;
    bool found;
    int count = 0;

    if ( argument[0] == '\0' )
    {
	DESCRIPTOR_DATA *d;

	/* show characters logged */

	buffer = new_buf();
	for (d = descriptor_list; d != NULL; d = d->next)
	{
	    if (d->character != NULL && d->connected == CON_PLAYING
	    &&  d->character->in_room != NULL && can_see(ch,d->character)
	    &&  can_see_room(ch,d->character->in_room))
	    {
		victim = d->character;
		count++;
		if (d->original != NULL)
		    sprintf(buf,"%3d) %s (in the body of %s) is in %s [%d]\n\r",
			count, d->original->name,victim->short_descr,
			victim->in_room->name,victim->in_room->vnum);
		else
		    sprintf(buf,"%3d) %s is in %s [%d]\n\r",
			count, victim->name,victim->in_room->name,
			victim->in_room->vnum);
		add_buf(buffer,buf);
	    }
	}

        page_to_char(buf_string(buffer),ch);
	free_buf(buffer);
	return;
    }

    found = FALSE;
    buffer = new_buf();
    for ( victim = char_list; victim != NULL; victim = victim->next )
    {
	if ( victim->in_room != NULL
	&&   is_name( argument, victim->name ) 
	&&   can_see(ch,victim) )
	{
	    found = TRUE;
	    count++;
	    sprintf( buf, "%3d) [%5d] %-28s [%5d] %s\n\r", count,
		IS_NPC(victim) ? victim->pIndexData->vnum : 0,
		IS_NPC(victim) ? victim->short_descr : victim->name,
		victim->in_room->vnum,
		victim->in_room->name );
	    add_buf(buffer,buf);
	}
    }

    /* all the mobs without a room */
    if (!str_cmp(argument,"nowhere"))
    {
        buffer = new_buf();
        found=FALSE;
        count=0;
        for ( victim = char_list; victim != NULL; victim = victim->next ) {
            if (victim->in_room==NULL) {
                found = TRUE;
                count++;
                sprintf( buf, "%3d) [%5d] %-28s %lx\n\r", count,
                    IS_NPC(victim) ? victim->pIndexData->vnum : 0,
                    IS_NPC(victim) ? victim->short_descr : victim->name,
                    (unsigned long)victim);
                add_buf(buffer,buf);
            }
        }
        if (found)
            page_to_char(buf_string(buffer),ch);
        else
            send_to_char("No mobs without rooms found.\n\r",ch);
        free_buf(buffer);
        return;
    }

    if ( !found )
	act( "You didn't find any $T.", ch, NULL, argument, TO_CHAR );
    else
    	page_to_char(buf_string(buffer),ch);

    free_buf(buffer);

    return;
}



void do_reboo( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to REBOOT, spell it out.\n\r", ch );
    return;
}



void do_reboot( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    extern bool merc_down;
    DESCRIPTOR_DATA *d,*d_next;
    CHAR_DATA *vch;

    if (ch->invis_level < LEVEL_HERO)
    {
    	sprintf( buf, "*** Reboot by %s ***.", ch->name );
    	do_function(ch, &do_echo, buf );
    }

    merc_down = TRUE;
    for ( d = descriptor_list; d != NULL; d = d_next )
    {
	d_next = d->next;
	vch = d->original ? d->original : d->character;
	if (vch != NULL)
	    save_char_obj(vch);
    	close_socket(d);
    }
    
    return;
}

void do_shutdow( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to SHUTDOWN, spell it out.\n\r", ch );
    return;
}

void do_shutdown( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    extern bool merc_down;
    DESCRIPTOR_DATA *d,*d_next;
    CHAR_DATA *vch;

    if (ch->invis_level < LEVEL_HERO)
    sprintf( buf, "*** Shutdown by %s ***.", ch->name );
    append_file2( ch, SHUTDOWN_FILE, buf );
    strcat( buf, "\n\r" );
    if (ch->invis_level < LEVEL_HERO)
    {
    	do_function(ch, &do_echo, buf );
    }
    merc_down = TRUE;
    for ( d = descriptor_list; d != NULL; d = d_next)
    {
	d_next = d->next;
	vch = d->original ? d->original : d->character;
	if (vch != NULL)
	    save_char_obj(vch);
	close_socket(d);
    }
    return;
}

void do_protect( CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim;

  if (argument[0] == '\0')
  {
    send_to_char("Protect whom from snooping?\n\r",ch);
    return;
  }

  if ((victim = get_char_world(ch,argument)) == NULL)
  {
    send_to_char("You can't find them.\n\r",ch);
    return;
  }

  if (IS_SET(victim->comm,COMM_SNOOP_PROOF))
  {
    act_new("$N is no longer snoop-proof.",ch,NULL,victim,TO_CHAR,POS_DEAD);
    REMOVE_BIT(victim->comm,COMM_SNOOP_PROOF);
  }
  else
  {
    act_new("$N is now snoop-proof.",ch,NULL,victim,TO_CHAR,POS_DEAD);
    SET_BIT(victim->comm,COMM_SNOOP_PROOF);
  }
}
  


void do_snoop( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Snoop whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->desc == NULL )
    {
	send_to_char( "No descriptor to snoop.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "Cancelling all snoops.\n\r", ch );
	wiznet("$N stops being such a snoop.",
		ch,NULL,WIZ_SNOOPS,WIZ_SECURE,get_trust(ch));
	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->snoop_by == ch->desc )
		d->snoop_by = NULL;
	}
	return;
    }

    if ( victim->desc->snoop_by != NULL )
    {
	send_to_char( "Busy already.\n\r", ch );
	return;
    }

    if (!is_room_owner(ch,victim->in_room)
    && ch->in_room != victim->in_room 
    &&  room_is_private(victim->in_room)
    && !IS_TRUSTED(ch,IMPLEMENTOR))
    {
      send_to_char("That character is in a private room.\n\r",ch);
      return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) 
    ||   IS_SET(victim->comm,COMM_SNOOP_PROOF))
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( ch->desc != NULL )
    {
	for ( d = ch->desc->snoop_by; d != NULL; d = d->snoop_by )
	{
	    if ( d->character == victim || d->original == victim )
	    {
		send_to_char( "No snoop loops.\n\r", ch );
		return;
	    }
	}
    }

    victim->desc->snoop_by = ch->desc;
    sprintf(buf,"$N starts snooping on %s",
	(IS_NPC(ch) ? victim->short_descr : victim->name));
    wiznet(buf,ch,NULL,WIZ_SNOOPS,WIZ_SECURE,get_trust(ch));
    send_to_char( "Ok.\n\r", ch );
    return;
}



void do_switch( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );
    
    if ( arg[0] == '\0' )
    {
	send_to_char( "Switch into whom?\n\r", ch );
	return;
    }

    if ( ch->desc == NULL )
	return;
    
    if ( ch->desc->original != NULL )
    {
	send_to_char( "You are already switched.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "Ok.\n\r", ch );
	return;
    }

    if (!IS_NPC(victim))
    {
	send_to_char("You can only switch into mobiles.\n\r",ch);
	return;
    }

    if (!is_room_owner(ch,victim->in_room) && ch->in_room != victim->in_room 
    &&  room_is_private(victim->in_room) && !IS_TRUSTED(ch,IMPLEMENTOR))
    {
	send_to_char("That character is in a private room.\n\r",ch);
	return;
    }

    if ( victim->desc != NULL )
    {
	send_to_char( "Character in use.\n\r", ch );
	return;
    }

    sprintf(buf,"$N switches into %s",victim->short_descr);
    wiznet(buf,ch,NULL,WIZ_SWITCHES,WIZ_SECURE,get_trust(ch));

    ch->desc->character = victim;
    ch->desc->original  = ch;
    victim->desc        = ch->desc;
    ch->desc            = NULL;
    /* change communications to match */
    if (ch->prompt != NULL)
        victim->prompt = str_dup(ch->prompt);
    victim->comm = ch->comm;
    victim->lines = ch->lines;
    send_to_char( "Ok.\n\r", victim );
    return;
}



void do_return( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( ch->desc == NULL )
	return;

    if ( ch->desc->original == NULL )
    {
	send_to_char( "You aren't switched.\n\r", ch );
	return;
    }

    send_to_char( 
"You return to your original body. Type replay to see any missed tells.\n\r", 
	ch );
    if (ch->prompt != NULL)
    {
	free_string(ch->prompt);
	ch->prompt = NULL;
    }

    sprintf(buf,"$N returns from %s.",ch->short_descr);
    wiznet(buf,ch->desc->original,0,WIZ_SWITCHES,WIZ_SECURE,get_trust(ch));
    ch->desc->character       = ch->desc->original;
    ch->desc->original        = NULL;
    ch->desc->character->desc = ch->desc; 
    ch->desc                  = NULL;
    return;
}

/* trust levels for load and clone */
bool obj_check (CHAR_DATA *ch, OBJ_DATA *obj)
{
    if (IS_TRUSTED(ch,GOD)
	|| (IS_TRUSTED(ch,IMMORTAL) && obj->level <= 20 && obj->cost <= 1000)
	|| (IS_TRUSTED(ch,DEMI)	    && obj->level <= 10 && obj->cost <= 500)
	|| (IS_TRUSTED(ch,ANGEL)    && obj->level <=  5 && obj->cost <= 250)
	|| (IS_TRUSTED(ch,AVATAR)   && obj->level ==  0 && obj->cost <= 100))
	return TRUE;
    else
	return FALSE;
}

/* for clone, to insure that cloning goes many levels deep */
void recursive_clone(CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *clone)
{
    OBJ_DATA *c_obj, *t_obj;


    for (c_obj = obj->contains; c_obj != NULL; c_obj = c_obj->next_content)
    {
	if (obj_check(ch,c_obj))
	{
	    t_obj = create_object(c_obj->pIndexData,0);
	    clone_object(c_obj,t_obj);
	    obj_to_obj(t_obj,clone);
	    recursive_clone(ch,c_obj,t_obj);
	}
    }
}

/* command that is similar to load */
void do_clone(CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char *rest;
    CHAR_DATA *mob;
    OBJ_DATA  *obj;

    rest = one_argument(argument,arg);

    if (arg[0] == '\0')
    {
	send_to_char("Clone what?\n\r",ch);
	return;
    }

    if (!str_prefix(arg,"object"))
    {
	mob = NULL;
	obj = get_obj_here(ch, NULL, rest);
	if (obj == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
    }
    else if (!str_prefix(arg,"mobile") || !str_prefix(arg,"character"))
    {
	obj = NULL;
	mob = get_char_room(ch, NULL, rest);
	if (mob == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
    }
    else /* find both */
    {
	mob = get_char_room(ch, NULL, argument);
	obj = get_obj_here(ch, NULL, argument);
	if (mob == NULL && obj == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
    }

    /* clone an object */
    if (obj != NULL)
    {
	OBJ_DATA *clone;

	if (!obj_check(ch,obj))
	{
	    send_to_char(
		"Your powers are not great enough for such a task.\n\r",ch);
	    return;
	}

	clone = create_object(obj->pIndexData,0); 
	clone_object(obj,clone);
	if (obj->carried_by != NULL)
	    obj_to_char(clone,ch);
	else
	    obj_to_room(clone,ch->in_room);
 	recursive_clone(ch,obj,clone);

	act("$n has created $p.",ch,clone,NULL,TO_ROOM);
	act("You clone $p.",ch,clone,NULL,TO_CHAR);
	wiznet("$N clones $p.",ch,clone,WIZ_LOAD,WIZ_SECURE,get_trust(ch));
	return;
    }
    else if (mob != NULL)
    {
	CHAR_DATA *clone;
	OBJ_DATA *new_obj;
	char buf[MAX_STRING_LENGTH];

	if (!IS_NPC(mob))
	{
	    send_to_char("You can only clone mobiles.\n\r",ch);
	    return;
	}

	if ((mob->level > 20 && !IS_TRUSTED(ch,GOD))
	||  (mob->level > 10 && !IS_TRUSTED(ch,IMMORTAL))
	||  (mob->level >  5 && !IS_TRUSTED(ch,DEMI))
	||  (mob->level >  0 && !IS_TRUSTED(ch,ANGEL))
	||  !IS_TRUSTED(ch,AVATAR))
	{
	    send_to_char(
		"Your powers are not great enough for such a task.\n\r",ch);
	    return;
	}

	clone = create_mobile(mob->pIndexData);
	clone_mobile(mob,clone); 
	
	for (obj = mob->carrying; obj != NULL; obj = obj->next_content)
	{
	    if (obj_check(ch,obj))
	    {
		new_obj = create_object(obj->pIndexData,0);
		clone_object(obj,new_obj);
		recursive_clone(ch,obj,new_obj);
		obj_to_char(new_obj,clone);
		new_obj->wear_loc = obj->wear_loc;
	    }
	}
	char_to_room(clone,ch->in_room);
        act("$n has created $N.",ch,NULL,clone,TO_ROOM);
        act("You clone $N.",ch,NULL,clone,TO_CHAR);
	sprintf(buf,"$N clones %s.",clone->short_descr);
	wiznet(buf,ch,NULL,WIZ_LOAD,WIZ_SECURE,get_trust(ch));
        return;
    }
}

/* RT to replace the two load commands */

void do_load(CHAR_DATA *ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];

    argument = one_argument(argument,arg);

    if (arg[0] == '\0')
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  load mob <vnum>\n\r",ch);
	send_to_char("  load obj <vnum> <level>\n\r",ch);
	return;
    }

    if (!str_cmp(arg,"mob") || !str_cmp(arg,"char"))
    {
	do_function(ch, &do_mload, argument);
	return;
    }

    if (!str_cmp(arg,"obj"))
    {
	do_function(ch, &do_oload, argument);
	return;
    }
    /* echo syntax */
    do_function(ch, &do_load, "");
}

void do_sload(CHAR_DATA *ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];

    argument = one_argument(argument,arg);

    if (arg[0] == '\0')
    {
	send_to_char("  SLOAD: Loads an object into the room invis to all mortals.\n\r",ch);
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  sload mob <vnum>\n\r",ch);
	send_to_char("  sload obj <vnum> <level>\n\r",ch);
	return;
    }

    if (!str_cmp(arg,"mob") || !str_cmp(arg,"char"))
    {
	do_function(ch, &do_smload, argument);
	return;
    }

    if (!str_cmp(arg,"obj"))
    {
	do_function(ch, &do_soload, argument);
	return;
    }
    /* echo syntax */
    do_function(ch, &do_sload, "");
}



void do_mload( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    MOB_INDEX_DATA *pMobIndex;
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];
    
    one_argument( argument, arg );

    if ( arg[0] == '\0' || !is_number(arg) )
    {
	send_to_char( "Syntax: load mob <vnum>.\n\r", ch );
	return;
    }

    if ( ( pMobIndex = get_mob_index( atoi( arg ) ) ) == NULL )
    {
	send_to_char( "No mob has that vnum.\n\r", ch );
	return;
    }

    victim = create_mobile( pMobIndex );
    char_to_room( victim, ch->in_room );
    act( "$n has created $N!", ch, NULL, victim, TO_ROOM );
    sprintf(buf,"$N loads %s.",victim->short_descr);
    wiznet(buf,ch,NULL,WIZ_LOAD,WIZ_SECURE,get_trust(ch));
    send_to_char( "Ok.\n\r", ch );
    return;
}

void do_smload( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    MOB_INDEX_DATA *pMobIndex;
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];
    
    one_argument( argument, arg );

    if ( arg[0] == '\0' || !is_number(arg) )
    {
	send_to_char( "Syntax: sload mob <vnum>.\n\r", ch );
	return;
    }

    if ( ( pMobIndex = get_mob_index( atoi( arg ) ) ) == NULL )
    {
	send_to_char( "No mob has that vnum.\n\r", ch );
	return;
    }

    victim = create_mobile( pMobIndex );
    char_to_room( victim, ch->in_room );
    sprintf(buf,"$N loads %s.",victim->short_descr);
    wiznet(buf,ch,NULL,WIZ_LOAD,WIZ_SECURE,get_trust(ch));
    send_to_char( "Ok! All loaded.\n\r", ch );
    return;
}

void do_oload( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH] ,arg2[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA *obj;
    int level;
    
    argument = one_argument( argument, arg1 );
    one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || !is_number(arg1))
    {
	send_to_char( "Syntax: load obj <vnum> <level>.\n\r", ch );
	return;
    }
    
    level = get_trust(ch); /* default */
  
    if ( arg2[0] != '\0')  /* load with a level */
    {
	if (!is_number(arg2))
        {
	  send_to_char( "Syntax: oload <vnum> <level>.\n\r", ch );
	  return;
	}
        level = atoi(arg2);
        if (level < 0 || level > get_trust(ch))
	{
	  send_to_char( "Level must be be between 0 and your level.\n\r",ch);
  	  return;
	}
    }

    if ( ( pObjIndex = get_obj_index( atoi( arg1 ) ) ) == NULL )
    {
	send_to_char( "No object has that vnum.\n\r", ch );
	return;
    }

    obj = create_object( pObjIndex, level );
    if ( CAN_WEAR(obj, ITEM_TAKE) )
	obj_to_char( obj, ch );
    else
	obj_to_room( obj, ch->in_room );
    act( "$n has created $p!", ch, obj, NULL, TO_ROOM );
    wiznet("$N loads $p.",ch,obj,WIZ_LOAD,WIZ_SECURE,get_trust(ch));
    send_to_char( "Ok.\n\r", ch );
    return;
}

void do_soload( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH] ,arg2[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA *obj;
    int level;
    
    argument = one_argument( argument, arg1 );
    one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || !is_number(arg1))
    {
	send_to_char( "Syntax: sload obj <vnum> <level>.\n\r", ch );
	return;
    }
    
    level = get_trust(ch); /* default */
  
    if ( arg2[0] != '\0')  /* load with a level */
    {
	if (!is_number(arg2))
        {
	  send_to_char( "Syntax: soload <vnum> <level>.\n\r", ch );
	  return;
	}
        level = atoi(arg2);
        if (level < 0 || level > get_trust(ch))
	{
	  send_to_char( "Level must be be between 0 and your level.\n\r",ch);
  	  return;
	}
    }

    if ( ( pObjIndex = get_obj_index( atoi( arg1 ) ) ) == NULL )
    {
	send_to_char( "No object has that vnum.\n\r", ch );
	return;
    }

    obj = create_object( pObjIndex, level );
    if ( CAN_WEAR(obj, ITEM_TAKE) )
	obj_to_char( obj, ch );
    else
	obj_to_room( obj, ch->in_room );
    wiznet("$N loads $p.",ch,obj,WIZ_LOAD,WIZ_SECURE,get_trust(ch));
    send_to_char( "Ok! All loaded.\n\r", ch );
    return;
}

void do_purge( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	/* 'purge' */
	CHAR_DATA *vnext;
	OBJ_DATA  *obj_next;

	for ( victim = ch->in_room->people; victim != NULL; victim = vnext )
	{
	    vnext = victim->next_in_room;
	    if ( IS_NPC(victim) && !IS_SET(victim->act,ACT_NOPURGE) 
	    &&   victim != ch /* safety precaution */ )
		extract_char( victim, TRUE );
	}

	for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;
	    if (!IS_OBJ_STAT(obj,ITEM_NOPURGE))
	      extract_obj( obj );
	}

	act( "$n purges the room!", ch, NULL, NULL, TO_ROOM);
	send_to_char( "Ok.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( !IS_NPC(victim) )
    {
      send_to_char("Sorry, but you can not purge Players\n\r",ch);
	return;
    }

    act( "$n purges $N.", ch, NULL, victim, TO_NOTVICT );
    extract_char( victim, TRUE );
    return;
}



void do_empower( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    int level;
    int iLevel;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) )
    {
	send_to_char( "Syntax: empower <char> <level>.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "That player is not here.\n\r", ch);
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( ( level = atoi( arg2 ) ) < 1 || level > MAX_LEVEL )
    {
	sprintf(buf,"Level must be 1 to %d.\n\r", MAX_LEVEL);
	send_to_char(buf, ch);
	return;
    }

    if ( level == victim->level)
    {
    sprintf( buf, "%s is already level %d.  Re-advancing...\n\r", victim->name, level );
    send_to_char(buf,ch);
    send_to_char( "Deja vu!  Your mind reels as you re-live your past levels!\n\r", victim );
    }

    if ( level > get_trust( ch ) )
    {
	send_to_char( "Limited to your trust level.\n\r", ch );
	return;
    }

    /*
     * Lower level:
     *   Reset to level 1.
     *   Then raise again.
     *   Currently, an imp can lower another imp.
     *   -- Swiftest
     */
    if ( level < victim->level )
    {
        int temp_prac;

	send_to_char( "Lowering a player's level!\n\r", ch );
      sprintf(buf,"Demoting %s from level %d to level %d!\n\r", victim->name, victim->level, level );
      send_to_char(buf,ch);
	send_to_char( "Cursed and forsaken!  The Gods have lowered your level...\n\r", victim );
	temp_prac 		= victim->practice;
	victim->level    	= 1;
	victim->exp      	= exp_at_level(victim, level);
	victim->max_hit  	= 10;
	victim->max_mana 	= 100;
	victim->max_move 	= 100;
	victim->practice 	= 0;
	victim->hit      	= victim->max_hit;
	victim->mana     	= victim->max_mana;
	victim->move     	= victim->max_move;
	advance_level( victim, TRUE );
	victim->practice = temp_prac;
    }
    else
    {
	send_to_char( "Raising a player's level!\n\r", ch );
      sprintf(buf,"Raising %s from level %d to level %d!\n\r", victim->name, victim->level, level );
      send_to_char(buf,ch);
	send_to_char( "**** OOOOHHHHHHHHHH  YYYYEEEESSS ****\n\r", victim );
    }

    for ( iLevel = victim->level ; iLevel < level; iLevel++ )
    {
	victim->level += 1;
	advance_level( victim,TRUE);
    }

    if ( (victim->religion_id != RELIGION_NONE) && level >= 103 )
    {
	victim->religion_id = RELIGION_NONE;
	obj = ( get_eq_char( victim, WEAR_MARK ) );
	extract_obj( obj );
    }

    sprintf(buf,"You are now level %d.\n\r",victim->level);
    send_to_char(buf,victim);
    victim->exp   = exp_at_level(victim, UMAX( 1, victim->level ) );
    victim->trust = 0;
    save_char_obj(victim);
    return;
}

void do_trust( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    int level;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) )
    {
	send_to_char( "Syntax: trust <char> <level>.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "That player is not here.\n\r", ch);
	return;
    }

    if ( ( level = atoi( arg2 ) ) < 0 || level > MAX_LEVEL )
    {
	sprintf(buf, "Level must be 0 (reset) or 1 to %d.\n\r",MAX_LEVEL);
	send_to_char(buf, ch);
	return;
    }

    if ( level > get_trust( ch ) )
    {
	send_to_char( "Limited to your trust.\n\r", ch );
	return;
    }

    sprintf(buf,"Trust on %s has been changed from %d to %d!\n\r", victim->name, victim->trust, level );
    send_to_char(buf, ch);
    victim->trust = level;
    return;
}



void do_restore( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *vch;
    DESCRIPTOR_DATA *d;

    one_argument( argument, arg );
    if (arg[0] == '\0' || !str_cmp(arg,"room"))
    {
    /* cure room */
    	
        for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
        {
            affect_strip(vch,gsn_plague);
            affect_strip(vch,gsn_poison);
            affect_strip(vch,gsn_blindness);
            affect_strip(vch,gsn_sleep);
            affect_strip(vch,gsn_curse);
            
            vch->hit 	= vch->max_hit;
            vch->mana	= vch->max_mana;
            vch->move	= vch->max_move;
		check_subdue(vch);
		update_cond( vch );
            update_pos( vch);
            act("The power of $n surges through you, restoring you to perfect health.",ch,NULL,vch,TO_VICT);
        }

        sprintf(buf,"$N restored room %d.",ch->in_room->vnum);
        wiznet(buf,ch,NULL,WIZ_RESTORE,WIZ_SECURE,get_trust(ch));
        
        send_to_char("Room restored.\n\r",ch);
        return;

    }
    
    if ( get_trust(ch) >=  MAX_LEVEL - 1 && !str_cmp(arg,"all"))
    {
    /* cure all */
    	
        for (d = descriptor_list; d != NULL; d = d->next)
        {
	    victim = d->character;

	    if (victim == NULL || IS_NPC(victim))
		continue;
                
            affect_strip(victim,gsn_plague);
            affect_strip(victim,gsn_poison);
            affect_strip(victim,gsn_blindness);
            affect_strip(victim,gsn_sleep);
            affect_strip(victim,gsn_curse);
            
            victim->hit 	= victim->max_hit;
            victim->mana	= victim->max_mana;
            victim->move	= victim->max_move;
		update_cond( victim );
            update_pos( victim);
	    if (victim->in_room != NULL)
                act("The power of $n surges through you, restoring you to perfect health.",ch,NULL,victim,TO_VICT);
        }
	send_to_char("All active players restored.\n\r",ch);
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    affect_strip(victim,gsn_plague);
    affect_strip(victim,gsn_poison);
    affect_strip(victim,gsn_blindness);
    affect_strip(victim,gsn_sleep);
    affect_strip(victim,gsn_curse);
    victim->hit  = victim->max_hit;
    victim->mana = victim->max_mana;
    victim->move = victim->max_move;
    update_cond( victim );
    update_pos( victim );
    act( "The power of $n surges through you, restoring you to perfect health.", ch, NULL, victim, TO_VICT );
    sprintf(buf,"$N restored %s",
	IS_NPC(victim) ? victim->short_descr : victim->name);
    wiznet(buf,ch,NULL,WIZ_RESTORE,WIZ_SECURE,get_trust(ch));
    send_to_char( "Ok.\n\r", ch );
    return;
}

 	
void do_freeze( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Freeze whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed, and they saw...\n\r", ch );
      sprintf( buf, "%s is attempting to freeze you.\n\r", ch->name );
      send_to_char(buf,victim);
	return;
    }

    if ( IS_SET(victim->act, PLR_FREEZE) )
    {
	REMOVE_BIT(victim->act, PLR_FREEZE);
      send_to_char( "Your frozen form suddenly thaws.\n\r", victim );
	send_to_char( "You can play again.\n\r", victim );
	send_to_char( "FREEZE removed.\n\r", ch );
	sprintf(buf,"$N thaws %s.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
    else
    {
	SET_BIT(victim->act, PLR_FREEZE);
      send_to_char( "A Godly force turns your body to ice!\n\r", victim );
	send_to_char( "You can't do ANYthing!\n\r", victim );
	send_to_char( "FREEZE set.\n\r", ch );
	sprintf(buf,"$N puts %s in the deep freeze.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }

    save_char_obj( victim );

    return;
}



void do_log( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Log whom?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	if ( fLogAll )
	{
	    fLogAll = FALSE;
	    send_to_char( "Log ALL off.\n\r", ch );
	}
	else
	{
	    fLogAll = TRUE;
	    send_to_char( "Log ALL on.\n\r", ch );
	}
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    /*
     * No level check, gods can log anyone.
     */
    if ( IS_SET(victim->act, PLR_LOG) )
    {
	REMOVE_BIT(victim->act, PLR_LOG);
	send_to_char( "LOG removed.\n\r", ch );
    }
    else
    {
	SET_BIT(victim->act, PLR_LOG);
	send_to_char( "LOG set.\n\r", ch );
    }

    return;
}

void do_noemote( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Noemote whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }


    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->comm, COMM_NOEMOTE) )
    {
	REMOVE_BIT(victim->comm, COMM_NOEMOTE);
	send_to_char( "You can emote again.\n\r", victim );
	send_to_char( "NOEMOTE removed.\n\r", ch );
	sprintf(buf,"$N restores emotes to %s.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
    else
    {
	SET_BIT(victim->comm, COMM_NOEMOTE);
	send_to_char( "You can't emote!\n\r", victim );
	send_to_char( "NOEMOTE set.\n\r", ch );
	sprintf(buf,"$N revokes %s's emotes.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }

    return;
}



void do_noshout( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Noshout whom?\n\r",ch);
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->comm, COMM_NOSHOUT) )
    {
	REMOVE_BIT(victim->comm, COMM_NOSHOUT);
	send_to_char( "You can shout again.\n\r", victim );
	send_to_char( "NOSHOUT removed.\n\r", ch );
	sprintf(buf,"$N restores shouts to %s.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
    else
    {
	SET_BIT(victim->comm, COMM_NOSHOUT);
	send_to_char( "You can't shout!\n\r", victim );
	send_to_char( "NOSHOUT set.\n\r", ch );
	sprintf(buf,"$N revokes %s's shouts.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }

    return;
}



void do_notell( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Notell whom?", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->comm, COMM_NOTELL) )
    {
	REMOVE_BIT(victim->comm, COMM_NOTELL);
	send_to_char( "You can tell again.\n\r", victim );
	send_to_char( "NOTELL removed.\n\r", ch );
	sprintf(buf,"$N restores tells to %s.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
    else
    {
	SET_BIT(victim->comm, COMM_NOTELL);
	send_to_char( "You can't tell!\n\r", victim );
	send_to_char( "NOTELL set.\n\r", ch );
	sprintf(buf,"$N revokes %s's tells.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }

    return;
}

void do_noidle( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Noidle whom?", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->level > 101 )
    {
	send_to_char( "You can not NOIDLE Immortals. They have absolutely no need to be noidled.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->comm, COMM_NOIDLE) )
    {
	REMOVE_BIT(victim->comm, COMM_NOIDLE);
	send_to_char( "You can't idle anymore.\n\r", victim );
	send_to_char( "NOIDLE removed.\n\r", ch );
	sprintf(buf,"$N restores idling to %s.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
    else
    {
	SET_BIT(victim->comm, COMM_NOIDLE);
	send_to_char( "You can idle now without accumulating an idle timer!\n\r", victim );
	send_to_char( "NOIDLE set.\n\r", ch );
	sprintf(buf,"$N removes %s's idle timer.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }

    return;
}

void do_peace( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *rch;

    for ( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
    {
	if ( rch->fighting != NULL )
	    stop_fighting( rch, TRUE );
	if (IS_NPC(rch) && IS_SET(rch->act,ACT_AGGRESSIVE))
	    REMOVE_BIT(rch->act,ACT_AGGRESSIVE);
      if (rch->hunting != NULL)
          rch->hunting = NULL;
    }

    send_to_char( "Ok.\n\r", ch );
    return;
}

void do_wizlock( CHAR_DATA *ch, char *argument )
{
    extern bool wizlock;
    wizlock = !wizlock;

    if ( wizlock )
    {
	wiznet("$N has wizlocked the game.",ch,NULL,0,0,0);
	send_to_char( "Game wizlocked.\n\r", ch );
    }
    else
    {
	wiznet("$N removes wizlock.",ch,NULL,0,0,0);
	send_to_char( "Game un-wizlocked.\n\r", ch );
    }

    return;
}

/* RT anti-newbie code */

void do_newlock( CHAR_DATA *ch, char *argument )
{
    extern bool newlock;
    newlock = !newlock;
 
    if ( newlock )
    {
	wiznet("$N locks out new characters.",ch,NULL,0,0,0);
        send_to_char( "New characters have been locked out.\n\r", ch );
    }
    else
    {
	wiznet("$N allows new characters back in.",ch,NULL,0,0,0);
        send_to_char( "Newlock removed.\n\r", ch );
    }
 
    return;
}


void do_slookup( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int sn;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Lookup which skill or spell?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	for ( sn = 0; sn < MAX_SKILL; sn++ )
	{
	    if ( skill_table[sn].name == NULL )
		break;
	    sprintf( buf, "Sn: %3d  Slot: %3d  Skill/spell: '%s'\n\r",
		sn, skill_table[sn].slot, skill_table[sn].name );
	    send_to_char( buf, ch );
	}
    }
    else
    {
	if ( ( sn = skill_lookup( arg ) ) < 0 )
	{
	    send_to_char( "No such skill or spell.\n\r", ch );
	    return;
	}

	sprintf( buf, "Sn: %3d  Slot: %3d  Skill/spell: '%s'\n\r",
	    sn, skill_table[sn].slot, skill_table[sn].name );
	send_to_char( buf, ch );
    }

    return;
}

/* RT set replaces sset, mset, oset, and rset */

void do_set( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];

    argument = one_argument(argument,arg);

    if (arg[0] == '\0')
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  set mob   <name> <field> <value>\n\r",ch);
	send_to_char("  set obj   <name> <field> <value>\n\r",ch);
	send_to_char("  set room  <room> <field> <value>\n\r",ch);
      send_to_char("  set skill <name> <spell or skill> <value>\n\r",ch);
	return;
    }

    if (!str_prefix(arg,"mobile") || !str_prefix(arg,"character"))
    {
	do_function(ch, &do_mset, argument);
	return;
    }

    if (!str_prefix(arg,"skill") || !str_prefix(arg,"spell"))
    {
	do_function(ch, &do_sset, argument);
	return;
    }

    if (!str_prefix(arg,"object"))
    {
	do_function(ch, &do_oset, argument);
	return;
    }

    if (!str_prefix(arg,"room"))
    {
	do_function(ch, &do_rset, argument);
	return;
    }

    /* echo syntax */
    do_function(ch, &do_set, "");
}

void do_sset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int value;
    int sn;
    bool fAll;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char( "Syntax:\n\r",ch);
	send_to_char("  set skill <name> <spell or skill> <value>\n\r", ch);
	send_to_char("  set skill <name> all <value>\n\r",ch);  
	send_to_char("   (use the name of the skill, not the number)\n\r",ch);
	send_to_char("   (use QUOTATION marks for multi worded spells and skills)\n\r",ch);
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    fAll = !str_cmp( arg2, "all" );
    sn   = 0;
    if ( !fAll && ( sn = skill_lookup( arg2 ) ) < 0 )
    {
	send_to_char( "No such skill or spell.\n\r", ch );
	return;
    }

    /*
     * Snarf the value.
     */
    if ( !is_number( arg3 ) )
    {
	send_to_char( "Value must be numeric.\n\r", ch );
	return;
    }

    value = atoi( arg3 );
    if ( value < 0 || value > 100 )
    {
	send_to_char( "Value range is 0 to 100.\n\r", ch );
	return;
    }

    if ( fAll )
    {
	for ( sn = 0; sn < MAX_SKILL; sn++ )
	{
	    if ( skill_table[sn].name != NULL )
		victim->pcdata->learned[sn]	= value;
	}
    }
    else
    {
	victim->pcdata->learned[sn] = value;
    }

    return;
}


void do_mset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    char buf[100];
    CHAR_DATA *victim;
    int value;

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  set char <name> <field> <value>\n\r",ch); 
	send_to_char( "  Field being one of:\n\r",ch );
	send_to_char( "    str int wis dex con chr sex class level\n\r",ch );
	send_to_char( "    race group steel gold hp mana move prac\n\r",ch);
	send_to_char( "    align train thirst drunk full hunt\n\r",ch );
	send_to_char( "    security deathcount surname questpoints\n\r",ch );
	send_to_char( "    ethos questtime hometown\n\r",ch );
	send_to_char( "\n\r",ch );
	send_to_char( " ** Note: If setting surname to blank, use the argument RESET after\n\r",ch );
	send_to_char( "          the argument SURNAME.\n\r",ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    /* clear zones for mobs */
    victim->zone = NULL;

    /*
     * Snarf the value (which need not be numeric).
     */
    value = is_number( arg3 ) ? atoi( arg3 ) : -1;

    /*
     * Set something.
     */
    if ( !str_cmp( arg2, "str" ) )
    {
	if ( value < 3 || value > get_max_train(victim,STAT_STR) )
	{
	    sprintf(buf,
		"Strength range is 3 to %d\n\r.",
		get_max_train(victim,STAT_STR));
	    send_to_char(buf,ch);
	    return;
	}

	victim->perm_stat[STAT_STR] = value;
	return;
    }

    if ( !str_cmp( arg2, "security" ) )	/* OLC */
    {
	if ( IS_NPC(ch) )
	{
		send_to_char( "Si, claro.\n\r", ch );
		return;
	}

        if ( IS_NPC( victim ) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

	if ( value > ch->pcdata->security || value < 0 )
	{
	    if ( ch->pcdata->security != 0 )
	    {
		sprintf( buf, "Valid security is 0-%d.\n\r",
		    ch->pcdata->security );
		send_to_char( buf, ch );
	    }
	    else
	    {
		send_to_char( "Valid security is 0 only.\n\r", ch );
	    }
	    return;
	}
	victim->pcdata->security = value;
	return;
    }

    if ( !str_cmp( arg2, "int" ) )
    {
        if ( value < 3 || value > get_max_train(victim,STAT_INT) )
        {
            sprintf(buf,
		"Intelligence range is 3 to %d.\n\r",
		get_max_train(victim,STAT_INT));
            send_to_char(buf,ch);
            return;
        }
 
        victim->perm_stat[STAT_INT] = value;
        return;
    }

    if ( !str_cmp( arg2, "wis" ) )
    {
	if ( value < 3 || value > get_max_train(victim,STAT_WIS) )
	{
	    sprintf(buf,
		"Wisdom range is 3 to %d.\n\r",get_max_train(victim,STAT_WIS));
	    send_to_char( buf, ch );
	    return;
	}

	victim->perm_stat[STAT_WIS] = value;
	return;
    }

    if ( !str_cmp( arg2, "dex" ) )
    {
	if ( value < 3 || value > get_max_train(victim,STAT_DEX) )
	{
	    sprintf(buf,
		"Dexterity range is 3 to %d.\n\r",
		get_max_train(victim,STAT_DEX));
	    send_to_char( buf, ch );
	    return;
	}

	victim->perm_stat[STAT_DEX] = value;
	return;
    }

    if ( !str_cmp( arg2, "con" ) )
    {
	if ( value < 3 || value > get_max_train(victim,STAT_CON) )
	{
	    sprintf(buf,
		"Constitution range is 3 to %d.\n\r",
		get_max_train(victim,STAT_CON));
	    send_to_char( buf, ch );
	    return;
	}

	victim->perm_stat[STAT_CON] = value;
	return;
    }

    if ( !str_cmp( arg2, "chr" ) )
    {
        if ( value < 3 || value > 35 )
        {
            send_to_char("Charisma range is 3 to 35.\n\r", ch );
            return;
        }
        victim->perm_stat[STAT_CHR] = value;
        return;
    }


    if ( !str_prefix( arg2, "sex" ) )
    {
	if ( value < 0 || value > 2 )
	{
	    send_to_char( "Sex range is 0 to 2.\n\r", ch );
	    return;
	}
	victim->sex = value;
	if (!IS_NPC(victim))
	    victim->pcdata->true_sex = value;
	return;
    }

    if ( !str_prefix( arg2, "class" ) )
    {
	int class;

	if (IS_NPC(victim))
	{
	    send_to_char("Mobiles have no class.\n\r",ch);
	    return;
	}

	class = class_lookup(arg3);
	if ( class == -1 )
	{
	    char buf[MAX_STRING_LENGTH];

        	strcpy( buf, "Possible classes are: " );
        	for ( class = 0; class < MAX_CLASS; class++ )
        	{
            	    if ( class > 0 )
                    	strcat( buf, " " );
            	    strcat( buf, class_table[class].name );
        	}
            strcat( buf, ".\n\r" );

	    send_to_char(buf,ch);
	    return;
	}

	victim->class = class;
	return;
    }

    if ( !str_prefix( arg2, "level" ) )
    {
	if ( !IS_NPC(victim) )
	{
	    send_to_char( "Not on PC's.\n\r", ch );
	    return;
	}

	if ( value < 0 || value > MAX_LEVEL )
	{
	    sprintf(buf, "Level range is 0 to %d.\n\r", MAX_LEVEL);
	    send_to_char(buf, ch);
	    return;
	}
	victim->level = value;
	return;
    }

    if ( !str_prefix( arg2, "steel" ) )
    {
	victim->steel = value;
	return;
    }

    if ( !str_prefix(arg2, "gold" ) )
    {
	victim->gold = value;
	return;
    }

    if ( !str_prefix( arg2, "questtime" ) )
    {
	victim->nextquest = value;
	return;
    }

    if( !str_prefix(arg2, "hometown") )
    {
		int hometown;

		if (IS_NPC(victim))
		{
			send_to_char("Mobiles don't have hometowns.\n\r",ch);
			return;
		}

		hometown = get_hometown(arg3);
		if(hometown == -1)
		{
			char buf[MAX_STRING_LENGTH];

        	strcpy(buf, "Invalid hometown. Valid hometowns are:\n\r");
        	for(hometown = 0; hometown_table[hometown].name != NULL; hometown++ )
        	{
            	    if (hometown > 0) strcat(buf, " ");

            	    strcat(buf, hometown_table[hometown].name );
        	}
            strcat(buf, "\n\r");

			send_to_char(buf,ch);
			return;
		}

		victim->hometown = hometown;
	      victim->recall_point = hometown_table[victim->hometown].recall;

		send_to_char("Hometown set.\n\r",ch);
		return;
    }
	
    if ( !str_prefix( arg2, "deathcount"))
    {
       if (ch->level < 109)
       {
         send_to_char("You don't have the power to do that.\n\r",ch);
         return;
       }

       victim->pcdata->deaths = value;
       return;
    }

    if ( !str_prefix( arg2, "ethos"))
    {		 
        if (IS_NPC(victim))		 
        {		  
           send_to_char("Mobiles do not have an ethos.\n\r",ch);
           return;		  
        }
	if (is_number(arg3))
	{
	    send_to_char("Value should be lawful, neutral, or chaotic.\n\r",ch);
	    return;
	}
	if (!str_cmp(arg3, "lawful"))
	   victim->ethos = ETHOS_LAWFUL;
	else if (!str_cmp(arg3, "neutral"))
	   victim->ethos = ETHOS_NEUTRAL;
	else if (!str_cmp(arg3, "chaotic"))
	   victim->ethos = ETHOS_CHAOTIC;
	else
	   {
	   send_to_char("Value should be lawful, neutral, or chaotic.\n\r",ch);
	   return;
	   }
	send_to_char("Ethos set.\n\r",ch);
      return;	
    }

    if ( !str_prefix( arg2, "hp" ) )
    {
	if ( value < -10 || value > 30000 )
	{
	    send_to_char( "Hp range is -10 to 30,000 hit points.\n\r", ch );
	    return;
	}
	victim->max_hit = value;
        if (!IS_NPC(victim))
            victim->pcdata->perm_hit = value;
	return;
    }

    if ( !str_prefix( arg2, "mana" ) )
    {
	if ( value < 0 || value > 30000 )
	{
	    send_to_char( "Mana range is 0 to 30,000 mana points.\n\r", ch );
	    return;
	}
	victim->max_mana = value;
        if (!IS_NPC(victim))
            victim->pcdata->perm_mana = value;
	return;
    }

    if ( !str_prefix( arg2, "move" ) )
    {
	if ( value < 0 || value > 30000 )
	{
	    send_to_char( "Move range is 0 to 30,000 move points.\n\r", ch );
	    return;
	}
	victim->max_move = value;
        if (!IS_NPC(victim))
            victim->pcdata->perm_move = value;
	return;
    }

    if ( !str_prefix( arg2, "practice" ) )
    {
	if ( value < 0 || value > 250 )
	{
	    send_to_char( "Practice range is 0 to 250 sessions.\n\r", ch );
	    return;
	}
	victim->practice = value;
	return;
    }

    if ( !str_prefix( arg2, "train" ))
    {
	if (value < 0 || value > 100 )
	{
	    send_to_char("Training session range is 0 to 100 sessions.\n\r",ch);
	    return;
	}
	victim->train = value;
	return;
    }

    if ( !str_prefix( arg2, "questpoints" ))
    {
	if (value < 0 || value > 1000 )
	{
	    send_to_char("Quest Points range is 0 to 1000 sessions.\n\r",ch);
	    return;
	}
	victim->questpoints = value;
	return;
    }

    if ( !str_prefix( arg2, "align" ) )
    {
	if ( value < -1000 || value > 1000 )
	{
	    send_to_char( "Alignment range is -1000 to 1000.\n\r", ch );
	    return;
	}
	victim->alignment = value;
	return;
    }

    if ( !str_prefix( arg2, "thirst" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( value < -1 || value > 100 )
	{
	    send_to_char( "Thirst range is -1 to 100.\n\r", ch );
	    return;
	}

	victim->pcdata->condition[COND_THIRST] = value;
	return;
    }

    if ( !str_prefix( arg2, "drunk" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( value < -1 || value > 100 )
	{
	    send_to_char( "Drunk range is -1 to 100.\n\r", ch );
	    return;
	}

	victim->pcdata->condition[COND_DRUNK] = value;
	return;
    }

    if ( !str_prefix( arg2, "full" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( value < -1 || value > 100 )
	{
	    send_to_char( "Full range is -1 to 100.\n\r", ch );
	    return;
	}

	victim->pcdata->condition[COND_FULL] = value;
	return;
    }

    if (!str_prefix(arg2, "hunt"))
    {
        CHAR_DATA *hunted = 0;

        if ( !IS_NPC(victim) )
        {
            send_to_char( "Not on PC's.\n\r", ch );
            return;
        }

        if ( str_prefix( arg3, "." ) )
          if ( (hunted = get_char_area(victim, arg3)) == NULL )
            {
              send_to_char("Mob couldn't locate the victim to hunt.\n\r", ch);
              return;
            }

        victim->hunting = hunted;
        return;
    }

    if ( !str_prefix( arg2, "surname") )	 
    {		 
      if (IS_NPC(victim))		 
      {		  
        send_to_char("Mobiles do not have a surname.\n\r",ch);
        return;		  
      }

      if ( !str_prefix( arg3, "reset" ) )
      {
        victim->surname = str_dup ("");	  
        send_to_char("Surname has been set to nothing.\n\r",ch);
        return;		  
      }
	else
      {
        victim->surname = str_dup (capitalize(arg3));
        capitalize( victim->surname );
	  send_to_char("New Surname set.\n\r",ch);
        return;
      }
    }

    if (!str_prefix( arg2, "race" ) )
    {
	int race;

	race = race_lookup(arg3);

	if ( race == 0)
	{
	    send_to_char("That is not a valid race.\n\r",ch);
	    return;
	}

	if (!IS_NPC(victim) && !race_table[race].pc_race)
	{
	    send_to_char("That is not a valid player race.\n\r",ch);
	    return;
	}

	victim->race = race;
	return;
    }
   
    if (!str_prefix(arg2,"group"))
    {
	if (!IS_NPC(victim))
	{
	    send_to_char("Only on NPCs.\n\r",ch);
	    return;
	}
	victim->group = value;
	return;
    }

    if (!str_prefix(arg2,"cweight"))
    {
	if (IS_NPC(victim))
	{
	    send_to_char("Not on NPCs.\n\r",ch);
	    return;
	}
	victim->carry_weight = value;
	return;
    }

    if (!str_prefix(arg2,"nextquest"))
    {
	if (IS_NPC(victim))
	{
	    send_to_char("Not on NPCs.\n\r",ch);
	    return;
	}
        ch->nextquest = value;
	return;
    }

    if (!str_prefix(arg2,"qps"))
    {
	if (IS_NPC(victim))
	{
	    send_to_char("Not on NPCs.\n\r",ch);
	    return;
	}
        ch->questpoints = value;
	return;
    }

    /*
     * Generate usage message.
     */
    do_function(ch, &do_mset, "" );
    return;
}

void do_string( CHAR_DATA *ch, char *argument )
{
    char type [MAX_INPUT_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    char buf[MSL];

    smash_tilde( argument );
    argument = one_argument( argument, type );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( type[0] == '\0' || arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  string char <name> <field> <string>\n\r",ch);
	send_to_char("    fields: name short long desc title spec racetext\n\r",ch);
	send_to_char("  string obj  <name> <field> <string>\n\r",ch);
	send_to_char("    fields: name short long extended\n\r",ch);
	return;
    }
    
    if (!str_prefix(type,"character") || !str_prefix(type,"mobile"))
    {
    	if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
    	}

	/* clear zone for mobs */
	victim->zone = NULL;

	/* string something */

     	if ( !str_prefix( arg2, "name" ) )
    	{
	    if ( !IS_NPC(victim) )
	    {
	    	send_to_char( "Not on PC's.\n\r", ch );
	    	return;
	    }
	    free_string( victim->name );
	    victim->name = str_dup( arg3 );
	    return;
    	}
    	
    	if ( !str_prefix( arg2, "description" ) )
    	{
    	    free_string(victim->description);
    	    victim->description = str_dup(arg3);
    	    return;
    	}

    	if ( !str_prefix( arg2, "short" ) )
    	{
	    free_string( victim->short_descr );
	    victim->short_descr = str_dup( arg3 );
	    return;
    	}

    	if ( !str_prefix( arg2, "long" ) )
    	{
	    free_string( victim->long_descr );
	    strcat(arg3,"\n\r");
	    victim->long_descr = str_dup( arg3 );
	    return;
    	}

    	if ( !str_prefix( arg2, "title" ) )
    	{
	    if ( IS_NPC(victim) )
	    {
	    	send_to_char( "Not on NPC's.\n\r", ch );
	    	return;
	    }

          if (IS_SET(victim->act, PLR_NOTITLE))
	    {
	    	send_to_char( "You need to first, remove their NOTITLE flag.\n\r", ch );
	    	send_to_char( "To learn how, see 'help notitle'.\n\r", ch );
	    	return;
	    }

	    set_title( victim, arg3 );
	    return;
    	}

    	if ( !str_prefix( arg2, "spec" ) )
    	{
	    if ( !IS_NPC(victim) )
	    {
	    	send_to_char( "Not on PC's.\n\r", ch );
	    	return;
	    }

	    if ( ( victim->spec_fun = spec_lookup( arg3 ) ) == 0 )
	    {
	    	send_to_char( "No such spec fun.\n\r", ch );
	    	return;
	    }

	    return;
      }

      if ( !str_prefix( arg2, "racetext" ) )
    	{
	    if ( IS_NPC(victim) )
	    {
	    	send_to_char( "Not on NPC's.\n\r", ch );
	    	return;
	    }

            if(strlen_color(arg3)>MAX_WHO_COL)
            {
                sprintf(buf,"The racetext must be less than %d characters without colours.",MAX_WHO_COL);
                send_to_char(buf,ch);
                if (!IS_IMMORTAL(victim))
                    return;
            }

            free_string( victim->pcdata->racetext );
            if(!str_cmp(arg3,"default"))
            {
//                victim->pcdata->racetext = "@";
                victim->pcdata->racetext = NULL; //C039
            }
            else
            {
                victim->pcdata->racetext = str_dup( arg3 );
            }
	    return;
    	}
    }
    if (!str_prefix(type,"object"))
    {
    	/* string an obj */
    	
   	if( (obj = get_obj_carry(ch, arg1,ch)) == NULL )
    	{
	    send_to_char("You do not have that item.\n\r", ch);
	    return;
    	}
    	
        if ( !str_prefix( arg2, "name" ) )
    	{
	    free_string( obj->name );
	    obj->name = str_dup( arg3 );
	    return;
    	}

    	if ( !str_prefix( arg2, "short" ) )
    	{
	    free_string( obj->short_descr );
	    obj->short_descr = str_dup( arg3 );
	    return;
    	}

    	if ( !str_prefix( arg2, "long" ) )
    	{
	    free_string( obj->description );
	    obj->description = str_dup( arg3 );
	    return;
    	}

    	if ( !str_prefix( arg2, "ed" ) || !str_prefix( arg2, "extended"))
    	{
	    EXTRA_DESCR_DATA *ed;

	    argument = one_argument( argument, arg3 );
	    if ( argument == NULL )
	    {
	    	send_to_char( "Syntax: oset <object> ed <keyword> <string>\n\r",
		    ch );
	    	return;
	    }

 	    strcat(argument,"\n\r");

	    ed = new_extra_descr();

	    ed->keyword		= str_dup( arg3     );
	    ed->description	= str_dup( argument );
	    ed->next		= obj->extra_descr;
	    obj->extra_descr	= ed;
	    return;
    	}
    }
    
    	
    /* echo bad use message */
    do_function(ch, &do_string, "");
}



void do_oset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int value;

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  set obj <object> <field> <value>\n\r",ch);
	send_to_char("  Field being one of:\n\r",				ch );
	send_to_char("    value0 value1 value2 value3 value4 (v1-v4)\n\r",	ch );
	send_to_char("    extra wear level weight cost timer condition\n\r",		ch );
	return;
    }

    if ( ( obj = get_obj_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "Nothing like that in heaven or earth.\n\r", ch );
	return;
    }

    /*
     * Snarf the value (which need not be numeric).
     */
    value = atoi( arg3 );

    /*
     * Set something.
     */
    if ( !str_cmp( arg2, "value0" ) || !str_cmp( arg2, "v0" ) )
    {
	obj->value[0] = UMIN(50,value);
	return;
    }

    if ( !str_cmp( arg2, "value1" ) || !str_cmp( arg2, "v1" ) )
    {
	obj->value[1] = value;
	return;
    }

    if ( !str_cmp( arg2, "value2" ) || !str_cmp( arg2, "v2" ) )
    {
	obj->value[2] = value;
	return;
    }

    if ( !str_cmp( arg2, "value3" ) || !str_cmp( arg2, "v3" ) )
    {
	obj->value[3] = value;
	return;
    }

    if ( !str_cmp( arg2, "value4" ) || !str_cmp( arg2, "v4" ) )
    {
	obj->value[4] = value;
	return;
    }

    if ( !str_prefix( arg2, "extra" ) )
    {
      if ( ( value = flag_value( extra_flags, arg3 ) ) != NO_FLAG)
      {
          TOGGLE_BIT(obj->extra_flags, value);
      }
      return;
    }

    if ( !str_prefix( arg2, "extra2" ) )
    {
      if ( ( value = flag_value( extra2_flags, arg3 ) ) != NO_FLAG)
      {
          TOGGLE_BIT(obj->extra2_flags, value);
      }
      return;
    }

    if ( !str_prefix( arg2, "wear" ) )
    {
	obj->wear_flags = value;
	return;
    }

    if ( !str_prefix( arg2, "level" ) )
    {
	obj->level = value;
	return;
    }
	
    if ( !str_prefix( arg2, "weight" ) )
    {
	obj->weight = value;
	return;
    }

    if ( !str_prefix( arg2, "cost" ) )
    {
	obj->cost = value;
	return;
    }

    if ( !str_prefix( arg2, "timer" ) )
    {
	obj->timer = value;
	return;
    }

    if ( !str_prefix( arg2, "condition" ) )
    {
    	if ( value < 0 || value > 100 )
    	{
		send_to_char( "Value range is 0 to 100.\n\r", ch );
		return;
    	}

	obj->condition = value;
	return;
    }

    /*
     * Generate usage message.
     */
    do_function(ch, &do_oset, "" );
    return;
}



void do_rset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    int value;

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char( "Syntax:\n\r",ch);
	send_to_char( "  set room <location> <field> <value>\n\r",ch);
	send_to_char( "  Field being one of:\n\r",			ch );
	send_to_char( "    flags sector\n\r",				ch );
	return;
    }

    if ( ( location = find_location( ch, arg1 ) ) == NULL )
    {
	send_to_char( "No such location.\n\r", ch );
	return;
    }

    if (!is_room_owner(ch,location) && ch->in_room != location 
    &&  room_is_private(location) && !IS_TRUSTED(ch,IMPLEMENTOR))
    {
        send_to_char("That room is private right now.\n\r",ch);
        return;
    }

    /*
     * Snarf the value.
     */
    if ( !is_number( arg3 ) )
    {
	send_to_char( "Value must be numeric.\n\r", ch );
	return;
    }
    value = atoi( arg3 );

    /*
     * Set something.
     */
    if ( !str_prefix( arg2, "flags" ) )
    {
	location->room_flags	= value;
	return;
    }

    if ( !str_prefix( arg2, "sector" ) )
    {
	location->sector_type	= value;
	return;
    }

    /*
     * Generate usage message.
     */
    do_function(ch, &do_rset, "");
    return;
}

void do_sockets( CHAR_DATA *ch, char *argument )
{
     CHAR_DATA       *vch;
     DESCRIPTOR_DATA *d;
     char            buf  [ MAX_STRING_LENGTH ];
     char            buf2 [ MAX_STRING_LENGTH ];
     int             count;
     char *          st;
     char            s[100];
     char            idle[10];
     bool head_god = FALSE;


     count       = 0;
     buf[0]      = '\0';
     buf2[0]     = '\0';
     head_god    = !str_cmp(ch->name, HEAD_GOD);

strcat( buf2, "\n\r[Num Connected_State Login@ Idl] Player       Host\n\r" );
strcat( buf2, "--------------------------------------------------------------------------\n\r");

 for ( d = descriptor_list; d; d = d->next )
     {
         if ( d->character && can_see( ch, d->character ) )
         {
            /* NB: You may need to edit the CON_ values */
            switch( d->connected )
            {
               case CON_PLAYING:              st = "    PLAYING    ";    break;
               case CON_GET_NAME:             st = "   Get Name    ";    break;
               case CON_GET_OLD_PASSWORD:     st = "Get Old Passwd ";    break;
               case CON_CONFIRM_NEW_NAME:     st = " Confirm Name  ";    break;
               case CON_GET_NEW_PASSWORD:     st = "Get New Passwd ";    break;
               case CON_CONFIRM_NEW_PASSWORD: st = "Confirm Passwd ";    break;
		   case CON_GET_SURNAME:	    st = "  Get Surname  ";	 break;
               case CON_GET_NEW_RACE:         st = "  Get New Race ";    break;
               case CON_GET_NEW_SEX:          st = "  Get New Sex  ";    break;
               case CON_GET_NEW_CLASS:        st = " Get New Class ";    break;
		   case CON_GET_HOMETOWN:	    st = "  Get Hometown ";	 break;
		   case CON_GET_ALIGNMENT:	    st = " Get Alignment ";	 break;
		   case CON_GET_ETHOS:		    st = "   Get Ethos   ";	 break;
               case CON_READ_MOTD:            st = "  Reading MOTD ";    break;
		   case CON_BREAK_CONNECT:	    st = "   Link Dead   ";	 break;
		   case CON_GET_EMAIL:		    st = "   Get Email   ";	 break;
		   case CON_CONFIRM_COMPLY:	    st = "Confirm comply ";	 break;
		   case CON_GET_NEXT:	    	    st = "    Get next   ";	 break;
		   case CON_GET_NEXT2:	    	    st = "    Get next2  ";	 break;
		   case CON_PREAD_MOTD:	    	    st = "  Newbie Help  ";	 break;
               default:                       st = "   !UNKNOWN!   ";    break;
            }
            count++;

            /* Format "login" value... */
            vch = d->original ? d->original : d->character;
            strftime( s, 100, "%I:%M%p", localtime( &vch->logon ) );

            if ( vch->level <= LEVEL_HERO && vch->timer > 0 )
               sprintf( idle, "%-2d", vch->timer );
            else
               sprintf( idle, "  " );

            sprintf( buf, "[%3d %s %7s %2s] %-12s %s%-32.32s%s\n\r",
               d->descriptor,
               st,
               s,
               idle,
               ( d->original ) ? d->original->name
                               : ( d->character )  ? d->character->name
                                                   : "(None!)",
!head_god ? "" : IS_SET(d->character->comm, COMM_MASKEDIP) ? C_B_RED : "",
               IS_SET(d->character->comm, COMM_MASKEDIP) ? d->character->maskedip : d->host,
               CLEAR );

            strcat( buf2, buf );
          }
     }

     sprintf( buf, "\n\r%d user%s\n\r", count, count == 1 ? "" : "s" );
     strcat( buf2, buf );
     send_to_char( buf2, ch );
     return; 
}

/*
 * Thanks to Grodyn for pointing out bugs in this function.
 */
void do_force( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "\n\rsyntax: force <player|mob> <command for them to execute>\n\r", ch );
      if (get_trust(ch) < MAX_LEVEL - 2)
      {
	send_to_char( "syntax: force all <command for all to execute>\n\r", ch );
	send_to_char( "syntax: force players <command for all players to execute>\n\r", ch );
	send_to_char( "syntax: force immortals <command for all immortals below your level to execute>\n\r", ch );
      }
	return;
    }

    one_argument(argument,arg2);
  
    if (!str_cmp(arg2,"delete") || !str_prefix(arg2,"mob") || !str_cmp(arg2, "afk"))
    {
	send_to_char("That will NOT be done.\n\r",ch);
	return;
    }

    sprintf( buf, "$n forces you to '%s'.", argument );

    if ( !str_cmp( arg, "all" ) )
    {
        DESCRIPTOR_DATA *desc,*desc_next;

        if (get_trust(ch) < MAX_LEVEL - 2)
        {
            send_to_char("Not at your level!\n\r",ch);
            return;
        }

        for ( desc = descriptor_list; desc != NULL; desc = desc_next ) {
            desc_next = desc->next;

            if (desc->connected==CON_PLAYING
            &&  get_trust( desc->character ) < get_trust( ch ) )
            {
                act( buf, ch, NULL, desc->character, TO_VICT );
                interpret( desc->character, argument );
            }
        }
    }
    else if (!str_cmp(arg,"players"))
    {
        DESCRIPTOR_DATA *desc,*desc_next;

        if (get_trust(ch) < MAX_LEVEL - 2)
        {
            send_to_char("Not at your level!\n\r",ch);
            return;
        }

        for ( desc = descriptor_list; desc != NULL; desc = desc_next ) {
            desc_next = desc->next;

            if (desc->connected==CON_PLAYING
            &&  get_trust( desc->character ) < get_trust( ch )
            &&  desc->character->level < LEVEL_HERO)
            {
                act( buf, ch, NULL, desc->character, TO_VICT );
                interpret( desc->character, argument );
            }
        }
    }
    else if (!str_cmp(arg,"immortals"))
    {
        DESCRIPTOR_DATA *desc,*desc_next;

        if (get_trust(ch) < MAX_LEVEL - 2)
        {
            send_to_char("Not at your level!\n\r",ch);
            return;
        }

        for ( desc = descriptor_list; desc != NULL; desc = desc_next ) {
            desc_next = desc->next;

            if (desc->connected==CON_PLAYING
            &&  get_trust( desc->character ) < get_trust( ch )
	      &&  desc->character->level >= LEVEL_HERO)
 	      {
                act( buf, ch, NULL, desc->character, TO_VICT );
                interpret( desc->character, argument );
            }
        }
    }
    else
    {
	CHAR_DATA *victim;

	if ( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
	}

	if ( victim == ch )
	{
	    send_to_char( "Aye aye, right away!\n\r", ch );
	    return;
	}

    	if (!is_room_owner(ch,victim->in_room) 
	&&  ch->in_room != victim->in_room 
      &&  room_is_private(victim->in_room)
	&& !IS_TRUSTED(ch,IMPLEMENTOR))
    	{
            send_to_char("That character is in a private room.\n\r",ch);
            return;
      }

	if ( get_trust( victim ) >= get_trust( ch ) )
	{
	    send_to_char( "Do it yourself!\n\r", ch );
	    return;
	}

	if ( !IS_NPC(victim) && get_trust(ch) < MAX_LEVEL -3)
	{
	    send_to_char("Not at your level!\n\r",ch);
	    return;
	}

	act( buf, ch, NULL, victim, TO_VICT );
	interpret( victim, argument );
    }

    send_to_char( "Ok.\n\r", ch );
    return;
}



/*
 * New routines by Dionysos.
 */
void do_invis( CHAR_DATA *ch, char *argument )
{
    int level;
    char arg[MAX_STRING_LENGTH];
    if ( IS_NPC(ch) )
        return;
    /* RT code for taking a level argument */
    one_argument( argument, arg );
    if ( arg[0] == '\0' ) 
    /* take the default path */
      if ( IS_SET(ch->act, PLR_WIZINVIS) )
      {
        REMOVE_BIT(ch->act, PLR_WIZINVIS);
        ch->invis_level = 0;
        if (ch->pcdata->fadein[0] != '\0')
            {
              act("$t",ch,ch->pcdata->fadein, NULL, TO_ROOM );
		}
        else
            {
              act( "$n slowly fades into existence.", ch, NULL, NULL, TO_ROOM );
            }
        send_to_char( "You slowly fade back into existence.\n\r", ch );
      }
      else
      {
        SET_BIT(ch->act, PLR_WIZINVIS);
        ch->invis_level = get_trust(ch);
        if (ch->pcdata->fadeout[0] != '\0')
            {
              act("$t",ch,ch->pcdata->fadeout, NULL, TO_ROOM );
		}
        else
            {
              act( "$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM );
            }

        send_to_char( "You slowly vanish into thin air.\n\r", ch );
      }
    else
    /* do the level thing */
    {
      level = atoi(arg);
      if (level < 2 || level > get_trust(ch))
      {
        send_to_char("Invis level must be between 2 and your level.\n\r",ch);
        return;
      }
      else
      {
          ch->reply = NULL;
          SET_BIT(ch->act, PLR_WIZINVIS);
          ch->invis_level = level;
        if (ch->pcdata->fadeout[0] != '\0')
            {
              act("$t",ch,ch->pcdata->fadeout, NULL, TO_ROOM );
		}
        else
            {
              act( "$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM );
            }
          send_to_char( "You slowly vanish into thin air.\n\r", ch );
      }
    }
    return;
}

void do_incognito( CHAR_DATA *ch, char *argument )
{
    int level;
    char arg[MAX_STRING_LENGTH];
 
    /* RT code for taking a level argument */
    one_argument( argument, arg );
 
    if ( arg[0] == '\0' )
    /* take the default path */
 
      if ( ch->incog_level)
      {
          ch->incog_level = 0;
          act( "$n is no longer cloaked.", ch, NULL, NULL, TO_ROOM );
          send_to_char( "You are no longer cloaked.\n\r", ch );
      }
      else
      {
          ch->incog_level = get_trust(ch);
          act( "$n cloaks $s presence.", ch, NULL, NULL, TO_ROOM );
          send_to_char( "You cloak your presence.\n\r", ch );
      }
    else
    /* do the level thing */
    {
      level = atoi(arg);
      if (level < 2 || level > get_trust(ch))
      {
        send_to_char("Incog level must be between 2 and your level.\n\r",ch);
        return;
      }
      else
      {
          ch->reply = NULL;
          ch->incog_level = level;
          act( "$n cloaks $s presence.", ch, NULL, NULL, TO_ROOM );
          send_to_char( "You cloak your presence.\n\r", ch );
      }
    }
 
    return;
}



void do_holylight( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) )
	return;

    if ( IS_SET(ch->act, PLR_HOLYLIGHT) )
    {
	REMOVE_BIT(ch->act, PLR_HOLYLIGHT);
	send_to_char( "Holy light mode off.\n\r", ch );
    }
    else
    {
	SET_BIT(ch->act, PLR_HOLYLIGHT);
	send_to_char( "Holy light mode on.\n\r", ch );
    }

    return;
}

/* prefix command: it will put the string typed on each line typed */

void do_prefi (CHAR_DATA *ch, char *argument)
{
    send_to_char("You cannot abbreviate the prefix command.\r\n",ch);
    return;
}

void do_prefix (CHAR_DATA *ch, char *argument)
{
    char buf[MAX_INPUT_LENGTH];

    if (argument[0] == '\0')
    {
	if (ch->prefix[0] == '\0')
	{
	    send_to_char("You have no prefix to clear.\r\n",ch);
	    return;
	}

	send_to_char("Prefix removed.\r\n",ch);
	free_string(ch->prefix);
	ch->prefix = str_dup("");
	return;
    }

    if (ch->prefix[0] != '\0')
    {
	sprintf(buf,"Prefix changed to %s.\r\n",argument);
	free_string(ch->prefix);
    }
    else
    {
	sprintf(buf,"Prefix set to %s.\r\n",argument);
    }

    ch->prefix = str_dup(argument);
}
 
#define CH(descriptor)  ((descriptor)->original ? \
 (descriptor)->original : (descriptor)->character)
 
 /* This file holds the copyover data */
 #define COPYOVER_FILE "copyover.data"
 
 /* This is the executable file */
 #define EXE_FILE	  "../src/rom"
 
 
 /*  Copyover - Original idea: Fusion of MUD++
  *  Adapted to Diku by Erwin S. Andreasen, <erwin@pip.dknet.dk>
  *  http://pip.dknet.dk/~pip1773
  *  Changed into a ROM patch after seeing the 100th request for it :)
  */
 void do_copyover (CHAR_DATA *ch, char * argument)
 {
 	FILE *fp;
 	DESCRIPTOR_DATA *d, *d_next;
 	char buf [100], buf2[100];
 	extern int port,control; /* db.c */
 	bool  write_to_descriptor     args( ( int desc, char *txt, int length));
 	fp = fopen (COPYOVER_FILE, "w");
 	
 	if (!fp)
 	{
 		send_to_char ("Copyover file not writeable, aborted.\n\r",ch);
 		logpf ("Could not write to copyover file: %s", COPYOVER_FILE);
 		perror ("do_copyover:fopen");
 		return;
 	}
 	
 	/* Consider changing all saved areas here, if you use OLC */
 	
 	/* do_asave (NULL, ""); - autosave changed areas */
 	
 	
 	sprintf (buf, "\n\r           *** Copyover brought to you in part by -= %s =- ***\n\r\n\r The Legends of Krynn is undergoing a Hot-Reboot. This is to incorporate a few\n\r new changes that were made to the MUD. Please stand by! Thank You!\n\r", ch->name);
 	
 	/* For each playing descriptor, save its state */
 	for (d = descriptor_list; d ; d = d_next)
 	{
 		CHAR_DATA * och = CH (d);
 		d_next = d->next; /* We delete from the list , so need to save this */

            if( IS_SET(d->character->comm, COMM_MASKEDIP) )
            {
              REMOVE_BIT(d->character->comm, COMM_MASKEDIP);
              d->character->maskedip = NULL;
            }

 		if (!d->character || d->connected > CON_PLAYING) /* drop those logging on */
 		{
 			write_to_descriptor (d->descriptor, "\n\rSorry, we are rebooting. Come back in a few minutes.\n\r", 0);
 			close_socket (d); /* throw'em out */
 		}
 		else
 		{
 			fprintf (fp, "%d %s %s\n", d->descriptor, och->name, d->host);
 
 #if 0			 /* This is not necessary for ROM */
 			if (och->level == 1)
 			{
 				write_to_descriptor (d->descriptor, "Since you are level one, and level one characters do not save, you gain a free level!\n\r", 0);
 				advance_level (och);
 				och->level++; /* Advance_level doesn't do that */
 			}
 #endif			
 			save_char_obj (och);
 			
 			write_to_descriptor (d->descriptor, buf, 0);
 		}
 	}
 	
 	fprintf (fp, "-1\n");
 	fclose (fp);
 	
 	/* Close reserve and other always-open files and release other resources */
 	
 	fclose (fpReserve);
 	
 	/* exec - descriptors are inherited */
 	
 	sprintf (buf, "%d", port);
 	sprintf (buf2, "%d", control);
 	execl (EXE_FILE, "rom", buf, "copyover", buf2, (char *) NULL);
 
 	/* Failed - sucessful exec will not return */
 	
 	perror ("do_copyover: execl");
 	send_to_char ("Copyover FAILED!\n\r",ch);
 	
 	/* Here you might want to reopen fpReserve */
 	fpReserve = fopen (NULL_FILE, "r");
 }
 
 /* Recover from a copyover - load players */
 void copyover_recover ()
 {
 	DESCRIPTOR_DATA *d;
 	FILE *fp;
 	char name [100];
 	char host[MSL];
 	int desc;
 	bool fOld;
 	
 	logpf ("Copyover recovery initiated");
 	
 	fp = fopen (COPYOVER_FILE, "r");
 	
 	if (!fp) /* there are some descriptors open which will hang forever then ? */
 	{
 		perror ("copyover_recover:fopen");
 		logpf ("Copyover file not found. Exitting.\n\r");
 		exit (1);
 	}
 
 	unlink (COPYOVER_FILE); /* In case something crashes - doesn't prevent reading	*/
 	
 	for (;;)
 	{
 		fscanf (fp, "%d %s %s\n", &desc, name, host);
 		if (desc == -1)
 			break;
 
 		/* Write something, and check if it goes error-free */		
 		if (!write_to_descriptor (desc, "\n\rRestoring from copyover... almost finished!\n\r",0))
 		{
 			close (desc); /* nope */
 			continue;
 		}
 		
 		d = new_descriptor();
 		d->descriptor = desc;
 		
 		d->host = str_dup (host);
 		d->next = descriptor_list;
 		descriptor_list = d;
 		d->connected = CON_COPYOVER_RECOVER; /* -15, so close_socket frees the char */
 		
 	
 		/* Now, find the pfile */
 		
 		fOld = load_char_obj (d, name);
 		
 		if (!fOld) /* Player file not found?! */
 		{
 			write_to_descriptor (desc, "\n\rSomehow, your character was lost in the copyover. Sorry.\n\r", 0);
 			close_socket (d);			
 		}
 		else /* ok! */
 		{
 			write_to_descriptor (desc, "\n\rThank you for your patience. Copyover is complete. You may now resume playing.\n\r",0);
 	
 			/* Just In Case */
 			if (!d->character->in_room)
 				d->character->in_room = get_room_index (ROOM_VNUM_TEMPLE);
 
 			/* Insert in the char_list */
 			d->character->next = char_list;
 			char_list = d->character;
 
 			char_to_room (d->character, d->character->in_room);
 			do_look (d->character, "auto");
 			act ("$n materializes!", d->character, NULL, NULL, TO_ROOM);
 			d->connected = CON_PLAYING;
 
 			if (d->character->pet != NULL)
 			{
 			    char_to_room(d->character->pet,d->character->in_room);
 			    act("$n materializes!.",d->character->pet,NULL,NULL,TO_ROOM);
 			}
 		}
 		
 	}
    fclose (fp);
 	
 	
}
void do_transin( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( !IS_NPC(ch) )
    {
	smash_tilde( argument );

	if (argument[0] == '\0')
	{
	    sprintf(buf,"Your transin is %s\n\r",ch->pcdata->transin);
	    send_to_char(buf,ch);
	    return;
	}
	     
	free_string( ch->pcdata->transin );
	ch->pcdata->transin = str_dup( argument );

        sprintf(buf,"Your transin is now %s\n\r",ch->pcdata->transin);
        send_to_char(buf,ch);
    }
    return;
}

void do_transout( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
 
    if ( !IS_NPC(ch) )
    {
        smash_tilde( argument );
 
        if (argument[0] == '\0')
        {
            sprintf(buf,"Your transout is %s\n\r",ch->pcdata->transout);
            send_to_char(buf,ch);
            return;
        }
          
        free_string( ch->pcdata->transout );
        ch->pcdata->transout = str_dup( argument );
 
        sprintf(buf,"Your transout is now %s\n\r",ch->pcdata->transout);
        send_to_char(buf,ch);
    }
    return;
}

void do_fadein( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( !IS_NPC(ch) )
    {
	smash_tilde( argument );

	if (argument[0] == '\0')
	{
	    sprintf(buf,"Your fadein is: %s\n\r",ch->pcdata->fadein);
	    send_to_char(buf,ch);
	    return;
	}
	     
	free_string( ch->pcdata->fadein );
	ch->pcdata->fadein = str_dup( argument );

        sprintf(buf,"Your fadein is now: %s\n\r",ch->pcdata->fadein);
        send_to_char(buf,ch);
    }
    return;
}

void do_fadeout( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
 
    if ( !IS_NPC(ch) )
    {
        smash_tilde( argument );
 
        if (argument[0] == '\0')
        {
            sprintf(buf,"Your fadeout is: %s\n\r",ch->pcdata->fadeout);
            send_to_char(buf,ch);
            return;
        }
          
        free_string( ch->pcdata->fadeout );
        ch->pcdata->fadeout = str_dup( argument );
 
        sprintf(buf,"Your fadeout is now: %s\n\r",ch->pcdata->fadeout);
        send_to_char(buf,ch);
    }
    return;
}


/* This code was developed and made by Bloodbath, with the help and ideas of
   Guyver. */

void do_retribution( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( IS_NPC(ch) )
        return;

    if ( argument[0] != '\0' )
    {
	buf[0] = '\0';
	smash_tilde( argument );

    	if (argument[0] == '-')
    	{
            int len;
            bool found = FALSE;
 
            if (ch->pcdata->smite == NULL || ch->pcdata->smite[0] == '\0')
            {
                send_to_char("No lines left to remove.\n\r",ch);
                return;
            }
	
            strcpy(buf,ch->pcdata->smite);
 
            for (len = strlen(buf); len > 0; len--)
            {
                if (buf[len] == '\r')
                {
                    if (!found)  /* back it up */
                    {
                        if (len > 0)
                            len--;
                        found = TRUE;
                    }
                    else /* found the second one */
                    {
                        buf[len + 1] = '\0';
                        free_string(ch->pcdata->smite);
                        ch->pcdata->smite = str_dup(buf);
                        send_to_char( "Your divine fury takes the form of:\n\r", ch );
                        send_to_char( ch->pcdata->smite ? ch->pcdata->smite : 
			    "(None).\n\r", ch );
                        return;
                    }
                }
            }
            buf[0] = '\0';
            free_string(ch->pcdata->smite);
            ch->pcdata->smite = str_dup(buf);
            send_to_char("Mortals no longer need fear your mark upon them.\n\r",ch);
	    return;
        }
	if ( argument[0] == '+' )
	{
            if ( ch->pcdata->smite != NULL )
                strcat( buf, ch->pcdata->smite );
	    argument++;
	    while ( isspace(*argument) )
		argument++;
	}

        if ( strlen(buf) >= 1024)
	{
            send_to_char( "Your fury is great indeed, too great.  Use less lines.\n\r", ch );
	    return;
	}

	strcat( buf, argument );
	strcat( buf, "\n\r" );
        free_string( ch->pcdata->smite );
        ch->pcdata->smite = str_dup( buf );
    }

    send_to_char( "Your divine fury takes the form of:\n\r", ch );
    send_to_char( ch->pcdata->smite ? ch->pcdata->smite : "(None).\n\r", ch );
    return;
}


void do_smite( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];  
//    char buf[MAX_INPUT_LENGTH]; //Commented out 21 Jan as it is not needed
    CHAR_DATA *victim;
    
    argument = one_argument( argument, arg1 );  /* Combine the arguments */
    

    if ( IS_NPC(ch) )
        return;

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Syntax:\n\r",ch);
        send_to_char( "Smite <victim>\n\r",ch);
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
        send_to_char( "They are saved only through their abscence.\n\r",ch);
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Your an Immortal for Bejesus sakes. Just kill the FUCKER!!\n\r",ch);
	return;
    }

    if (!IS_NPC(victim) && victim->level >= get_trust(ch))
    {
        send_to_char("Your reach exceeds your grasp.\n\r",ch);
        return;
    }

    
    /* Customize stuff by alignment */

    if (ch->alignment > 350)
    {                                                                                                                                   
        act_new("Your actions have brought the holy power of $n upon you!",ch,NULL,victim,TO_VICT,POS_DEAD);
        act_new("$N has brought the holy power of $n upon themselves!",ch,NULL,victim,TO_NOTVICT,POS_DEAD);                                              
    }
    if (ch->alignment > -351 && ch->alignment < 351)
    {
        act_new("Your actions have called the divine fury of $n upon you!",ch,NULL,victim,TO_VICT,POS_DEAD);
        act_new("$N has called the divine fury of $n upon themselves!",ch,NULL,victim,TO_NOTVICT,POS_DEAD);
    }
    if (ch->alignment < -350)
    {
        act_new("You are struck down by the infernal power of $n!!",ch,NULL,victim,TO_VICT,POS_DEAD);
        act_new("The hellspawned, infernal power of $n has struck down $N!!",ch,NULL,victim,TO_NOTVICT,POS_DEAD);
    }

    /* This is where the thing we did in retribution is used */

    if ( ch->pcdata->smite[0] != '\0' )
    {
        //C072
        act_new(ch->pcdata->smite,ch,NULL,victim,TO_VICT,POS_DEAD);
//        send_to_char( ch->pcdata->smite, victim );
    }

    /* If it REALLY hurt */

    send_to_char( "{RThat really did HURT!{x\n\r", victim );

    if (!IS_NPC(victim)) //C064
    {
        char_from_room( victim );
        char_to_room( victim, get_home(victim)); //C058
    }
    victim->position = 5;
    victim->hit = 1;
    victim->mana = 1;
    victim->move = 1;

    send_to_char("Your will is done, your power felt.\n\r",ch);
    return;
}

void do_rename (CHAR_DATA* ch, char* argument)
{
	char old_name[MAX_INPUT_LENGTH],
		  new_name[MAX_INPUT_LENGTH],
		  strsave [MAX_INPUT_LENGTH];

	CHAR_DATA* victim;
	FILE* file;

	argument = one_argument(argument, old_name); /* find new/old name */
	one_argument (argument, new_name);

	/* Trivial checks */
	if (!old_name[0])
	{
		send_to_char ("Rename who?\n\r",ch);
		return;
	}
	
	victim = get_char_world (ch, old_name);
	
	if (!victim)
	{
		send_to_char ("There is no such a person online.\n\r",ch);
		return;
	}
	
	if (IS_NPC(victim))
	{   
		send_to_char ("You cannot use Rename on NPCs.\n\r",ch);
		return;
	}

	/* allow rename self new_name,but otherwise only lower level */	
	if ( (victim != ch) && (get_trust (victim) >= get_trust (ch)) )
	{
		send_to_char ("You failed.\n\r",ch);
		return;
	}
	
	if (!victim->desc || (victim->desc->connected != CON_PLAYING) )
	{
		send_to_char ("This player has lost his link or is inside a pager or the like.\n\r",ch);
		return;
	}

	if (!new_name[0])
	{
		send_to_char ("Rename to what new name?\n\r",ch);
		return;
	}
	
	if (!check_parse_name(new_name))
	{
		send_to_char ("The new name is illegal.\n\r",ch);
		return;
	}

	/* First, check if there is a player named that off-line */
#if !defined(machintosh) && !defined(MSDOS)
    sprintf( strsave, "%s%s%s%s", PLAYER_DIR, initial( new_name ),
	    	 "/", capitalize( new_name ) );
#else
    sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( new_name ) );
#endif

	fclose (fpReserve); /* close the reserve file */
	file = fopen (strsave, "r"); /* attempt to to open pfile */
	if (file)
	{
		send_to_char ("A player with that name already exists!\n\r",ch);
		fclose (file);
    	fpReserve = fopen( NULL_FILE, "r" ); /* is this really necessary these days? */
		return;		
	}
   	fpReserve = fopen( NULL_FILE, "r" );  /* reopen the extra file */

	/* Check .gz file ! */
#if !defined(machintosh) && !defined(MSDOS)
    sprintf( strsave, "%s%s%s%s.gz", PLAYER_DIR, initial( new_name ),
	    	 "/", capitalize( new_name ) );
#else
    sprintf( strsave, "%s%s.gz", PLAYER_DIR, capitalize( new_name ) );
#endif

	fclose (fpReserve); /* close the reserve file */
	file = fopen (strsave, "r"); /* attempt to to open pfile */
	if (file)
	{
		send_to_char ("A player with that name already exists in a compressed file!\n\r",ch);
		fclose (file);
    	fpReserve = fopen( NULL_FILE, "r" ); 
		return;		
	}
		fpReserve = fopen( NULL_FILE, "r" );  /* reopen the extra file */

	if (get_char_world(ch,new_name)) /* check for playing level-1 non-saved */
	{
		send_to_char ("A player with the name you specified already exists!\n\r",ch);
		return;
	}

	/* Save the filename of the old name */

#if !defined(machintosh) && !defined(MSDOS)
    sprintf( strsave, "%s%s%s%s", PLAYER_DIR, initial( victim->name ),
	    	 "/", capitalize( victim->name ) );
#else
    sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( victim->name ) );
#endif


	/* Rename the character and save him to a new file */
	/* NOTE: Players who are level 1 do NOT get saved under a new name */

	free_string (victim->name);
	victim->name = str_dup (capitalize(new_name));
	
	save_char_obj (victim);
	
	/* unlink the old file */
	unlink (strsave); /* unlink does return a value.. but we do not care */

	/* That's it! */
	
	send_to_char ("Character renamed.\n\r",ch);

	victim->position = POS_STANDING; /* I am laaazy */
	act ("$n has renamed you to $N!",ch,NULL,victim,TO_VICT);
			
} /* do_rename */

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
/** Function: do_pload
  * Descr   : Loads a player object into the mud, bringing them (and their
  *           pet) to you for easy modification.  Player must not be connected.
  *           Note: be sure to send them back when your done with them.
  * Returns : (void)
  * Syntax  : pload (who)
  * Written : v1.0 12/97
  * Author  : Gary McNickle <gary@dharvest.com>
  */
void do_pload( CHAR_DATA *ch, char *argument )
{
  DESCRIPTOR_DATA d;
  bool isChar = FALSE;
  char name[MAX_INPUT_LENGTH];

  if (argument[0] == '\0')
  {
    send_to_char("Load who?\n\r", ch);
    return;
  }

  if(!str_cmp(argument,"hiddukel"))
  {
    send_to_char("Hiddukel's character can NOT be ploaded! Your transaction has been logged!\n\r", ch);
    return;
  }

  argument[0] = UPPER(argument[0]);
  argument = one_argument(argument, name);

  /* Dont want to load a second copy of a player who's allready online! */
  if ( get_char_world( ch, name ) != NULL )
  {
    send_to_char( "That person is allready connected!\n\r", ch );
    return;
  }

  isChar = load_char_obj(&d, name); /* char pfile exists? */

  if (!isChar)
  {
    send_to_char("Load Who? Are you sure? I cant seem to find them.\n\r", ch);
    return;
  }

  d.character->desc     = NULL;
  d.character->next     = char_list;
  char_list             = d.character;
  d.connected           = CON_PLAYING;
  reset_char(d.character);

  /* bring player to imm */
  if ( d.character->in_room != NULL )
  {
    char_to_room( d.character, ch->in_room); /* put in room imm is in */
  }

  act( "Player $N has been loaded into the room.", ch, NULL, d.character, TO_CHAR );
  act( "$n has pulled $N from the pattern!", ch, NULL, d.character, TO_ROOM );
  act( "$N appears from out of nowhere, eyes glazed over.", ch, NULL, d.character, TO_ROOM );

  if (d.character->pet != NULL)
   {
     char_to_room(d.character->pet,d.character->in_room);
     act("$n has entered the game.",d.character->pet,NULL,NULL,TO_ROOM);
   }

}

/** Function: do_punload
  * Descr   : Returns a player, previously 'ploaded' back to the void from
  *           whence they came.  This does not work if the player is actually 
  *           connected.
  * Returns : (void)
  * Syntax  : punload (who)
  * Written : v1.0 12/97
  * Author  : Gary McNickle <gary@dharvest.com>
  */
void do_punload( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim;
  char who[MAX_INPUT_LENGTH];

  argument = one_argument(argument, who);

  if(!str_cmp(argument,"hiddukel"))
  {
    send_to_char("Hiddukel's character can NOT be ploaded! Your transaction has been logged!\n\r", ch);
    return;
  }

  if ( ( victim = get_char_world( ch, who ) ) == NULL )
  {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  /** Person is legitametly logged on... was not ploaded.
   */
  if (victim->desc != NULL)
  {
    send_to_char("I dont think that would be a good idea...\n\r", ch);
    return;
  }

  if (victim->was_in_room != NULL) /* return player and pet to orig room */
  {
    char_to_room(victim, victim->was_in_room);
    if (victim->pet != NULL)
      char_to_room(victim->pet, victim->was_in_room);
  }

  save_char_obj(victim);
  do_quit(victim,"");

  act( "Player $N has been unloaded from the MUD.", ch, NULL, victim, TO_CHAR );
  act("$n has released $N back to the Pattern.", ch, NULL, victim, TO_ROOM);
}

void do_check( CHAR_DATA *ch, char *argument ) 
{     
char buf[MAX_STRING_LENGTH];     
char arg[MAX_INPUT_LENGTH];     
BUFFER *buffer;     
CHAR_DATA *victim;     
int count = 1;         

one_argument( argument, arg );          

if (arg[0] == '\0'|| !str_prefix(arg,"stats"))     
{         
	buffer = new_buf();     	
      for (victim = char_list; 
	victim != NULL; 
	victim = victim->next)     	
  {     	    
    if (IS_NPC(victim) || !can_see(ch,victim))
      continue;
     	    if (victim->desc == NULL)
 	    {
 	    	sprintf(buf,"%3d) %s is linkdead.\n\r", count, victim->name);
 	      add_buf(buffer, buf);
 	    	count++;
 	    	continue;
	    }

 	   if (victim->desc->connected >= CON_GET_NEW_RACE
 	   && victim->desc->connected <= CON_PICK_WEAPON)
 	    { 	    	
		sprintf(buf,"%3d) %s is being created.\n\r",count, victim->name); 	    	      add_buf(buffer, buf);
 	    	count++;
 	    	continue;
 	    }

 	   if ( (victim->desc->connected == CON_GET_OLD_PASSWORD
 	   || victim->desc->connected >= CON_READ_IMOTD)
 	   && get_trust(victim) <= get_trust(ch) )
 	    {
 	    	sprintf(buf,"%3d) %s is connecting.\n\r",count, victim->name);
 	    	add_buf(buffer, buf);
 	    	count++;
 	    	continue;
 	    		  	    }
 	  if (victim->desc->connected == CON_PLAYING)
 	    {
 	     if (get_trust(victim) > get_trust(ch))
 	     sprintf(buf,"%3d) %s.\n\r", count, victim->name);
 	     else
 	      {
           sprintf(buf,"%3d) %s, Level %d connected since %d hours (%d total hours)\n\r",
		count,
		victim->name,
		victim->level,
		((int)(current_time - victim->logon)) /3600,
            (victim->played + (int)(current_time - victim->logon)) /3600 );
 		add_buf(buffer, buf);

 	if (arg[0]!='\0' && !str_prefix(arg,"stats"))
 	{
 	sprintf(buf,"  %d HP %d Mana (%d %d %d %d %d %d) %ld steels %d Tr %d Pr %d Qpts.\n\r", 	victim->max_hit,
	victim->max_mana,
	victim->perm_stat[STAT_STR],
 	victim->perm_stat[STAT_INT],
	victim->perm_stat[STAT_WIS],
	victim->perm_stat[STAT_DEX],
	victim->perm_stat[STAT_CON],
	victim->perm_stat[STAT_CHR],
 	victim->steel + victim->gold/100,
	victim->train,
	victim->practice,
	victim->questpoints);
 	add_buf(buffer, buf);
 		    }
 		    count++;
 		}
 	        continue;
 	    }
 	     	 sprintf(buf,"%3d) bug (oops)...please report to Hiddukel: %s %d\n\r", 	    				count,
			victim->name,
			victim->desc->connected);
		 	add_buf(buffer, buf);
		 	count++;
        	}
     	page_to_char(buf_string(buffer),ch);
     	free_buf(buffer);
     	return;
     }

     if (!str_prefix(arg,"eq"))
     {
      buffer = new_buf();
     	for (victim = char_list;
 	victim != NULL;
 	victim = victim->next)
     	{
     	if (IS_NPC(victim)
      || victim->desc->connected != CON_PLAYING
     	|| !can_see(ch,victim)
     	|| get_trust(victim) > get_trust(ch) )
     	   continue;

 sprintf(buf,"%3d) %s, %d items (weight %d) Hit:%d Dam:%d Save:%d AC:%d %d %d %d.\n\r",     	    	count,
		victim->name,
 		victim->carry_number,
 		victim->carry_weight,
      	victim->hitroll,
 		victim->damroll,
 		victim->saving_throw,
     	    	victim->armor[AC_PIERCE],
 		victim->armor[AC_BASH],
     	    	victim->armor[AC_SLASH],
 		victim->armor[AC_EXOTIC]);
     	    	add_buf(buffer, buf);
     	    	count++;
       	}
     	page_to_char(buf_string(buffer),ch);
     	free_buf(buffer);
    	     	return;
     }

    if ( !str_prefix( arg, "process" ) )
    {
      FILE *fp;
      char proc[24];
      char line[80];
      char buf[MAX_STRING_LENGTH];
      BUFFER *output;
      output = new_buf();

      sprintf( proc, "ps xu > ../data/ps.txt" );
      system( proc );

      /* open file */
      fclose( fpReserve );
      fp = fopen( "../data/ps.txt","r" );

      while( fgets( line, 80, fp ) != NULL )
      {
	sprintf(buf, "%s\n\r", line);
	add_buf(output,buf);
      }

      fclose(fp);
      fpReserve = fopen( NULL_FILE, "r" );
      page_to_char( buf_string(output), ch );
      free_buf(output);
      return;
    }

    if (!str_prefix(arg,"snoop"))
    {
      char bufsnoop [100];

      if(ch->level < MAX_LEVEL )
      {
         send_to_char("You can't use this check option.\n\r",ch);
         return;
      }
         buffer = new_buf();

          for (victim = char_list; victim != NULL; victim = victim->next)
         {
              if (IS_NPC(victim)
              || victim->desc->connected != CON_PLAYING
              || !can_see(ch,victim)
              || get_trust(victim) > get_trust(ch) )
                 continue;

            if(victim->desc->snoop_by != NULL)
            sprintf(bufsnoop," %15s .",victim->desc->snoop_by->character->name);
        else
            sprintf(bufsnoop,"     (none)      ." );
            sprintf(buf,"%3d %15s : %s \n\r",
		count,
		victim->name,
		bufsnoop);
            add_buf(buffer, buf);
            count++;
         }
         page_to_char(buf_string(buffer),ch);
         free_buf(buffer);
         return;
     }
     send_to_char("Syntax: 'check'         display info about players\n\r",ch);
     send_to_char("        'check stats'   display info and resume stats\n\r",ch);
     send_to_char("        'check eq'      resume eq of all players\n\r",ch);
     send_to_char("        'check process' check processes of the MUD\n\r",ch);
     send_to_char("        'check snoop'   check to see who's snooping who\n\r",ch);
     send_to_char("Use the stat command in case of doubt about someone...\n\r",ch);
     return;
 }

/* Hiddukel's being mean to characters section... */

void do_notitle(CHAR_DATA *ch, char *argument)
{
   char      arg[MAX_INPUT_LENGTH];
   char      buf[MAX_STRING_LENGTH];
   CHAR_DATA *victim;

   argument = one_argument(argument, arg);
   victim   = get_char_world(ch, arg);

   if (arg[0] == '\0')
      {
      send_to_char("Whose Title Priviliges do you wish to change?\n\r", ch);
      return;
      }

	if ((victim = get_char_world( ch, arg ))==NULL )
      {
      send_to_char("They aren't of this world!\n\r", ch);
      return;
      }
   else
      {
      if (IS_NPC(victim))
         {
         send_to_char("Not on NPC's!\n\r", ch);
         return;
         }
	else if (IS_SET(victim->act, PLR_NOTITLE))
 	   {
	   REMOVE_BIT(victim->act, PLR_NOTITLE);
         sprintf(buf, "You allow %s the ability to recieve a special title!\n\r", victim->name);
         sprintf(buf, "A title which will now remain with them forever! Ha Ha!\n\r");
         send_to_char(buf, ch);
         return;
         }
      else
         {
	   SET_BIT(victim->act, PLR_NOTITLE);
         sprintf(buf, "You have made it so %s will now recieve a pre-set title for each level!\n\r", victim->name);
         send_to_char(buf, ch);
         sprintf( buf, "the %s",
         title_table [victim->class] [victim->level] [victim->pcdata->true_sex == SEX_FEMALE ? 1 : 0] );
         set_title( victim, buf );
         return;
         }
      }
}

void do_noexp(CHAR_DATA *ch, char *argument)
{
   char      arg[MAX_INPUT_LENGTH];
   char      buf[MAX_STRING_LENGTH];
   CHAR_DATA *victim;

   argument = one_argument(argument, arg);

   if (arg[0] == '\0')
      {
      send_to_char("Whose EXP Priviliges do you wish to change?\n\r", ch);
      return;
      }

	if ((victim = get_char_world( ch, arg ) )==NULL )
      {
      send_to_char("They aren't of this world!\n\r", ch);
      return;
      }
   else
      {
      if (IS_NPC(victim))
         {
         send_to_char("Not on NPC's!\n\r", ch);
         return;
         }
      else if (IS_SET(victim->act, PLR_NOEXP))
         {
         REMOVE_BIT(victim->act, PLR_NOEXP);
         sprintf(buf, "You restore %s's Exp priviliges!\n\r", victim->name);
         send_to_char(buf, ch);
         return;
         }
      else
         {
         SET_BIT(victim->act, PLR_NOEXP);
         sprintf(buf, "You remove %s's Exp priviliges!\n\r", victim->name);
         send_to_char(buf, ch);
         return;
         }
      }
}

void do_extrabit( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];
  char target_object[MAX_INPUT_LENGTH];
  char extrabits[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  int x;
  int y;

  //Dave said to put it in but not necessary, better safe than sorry >:)
  smash_tilde(argument);  
  
  //Gets the object in question.
  argument = one_argument(argument, target_object);

  //Gets the flags that should be put on the item.
  //argument = one_argument(argument, extrabits);
  strcpy (extrabits, argument);

  //Check to see if there is a substring.  If not then print out
  //the syntax help file type thingermabobber.
  if (target_object[0] == '\0' || extrabits[0] == '\0') 
  {
    send_to_char ("Syntax: extrabit <item name> <flags>\n\r",ch);
    send_to_char ("Flag Listings:\n\r\n\r",ch);
    send_to_char ("A - Glow           B - Hum           C - Dark\n\r",ch);
    send_to_char ("D - Masterwork     E - Evil          F - Invis\n\r",ch);
    send_to_char ("G - Magic          H - No Drop       I - Bless\n\r",ch);
    send_to_char ("J - Anti Good      K - Anti Evil     L - Anti Neutral\n\r",ch);
    send_to_char ("M - No Remove      N - Inventory     O - No Purge\n\r",ch);
    send_to_char ("P - Rot Death      Q - Vis Death     S - Non Metal\n\r",ch);
    send_to_char ("T - No Locate      U - Melt Drop     V - Had Timer\n\r",ch);
    send_to_char ("W - Sell Extract   X - Stainless     Y - Burn Proof\n\r",ch);
    send_to_char ("Z - No Uncurse\n\r\n\r",ch);
    send_to_char ("To remove all flags type 'extrabit <target object> 0'\n\r",ch);
    return;
  }

  //Check to see if object exsists in the world.
  if ((obj = get_obj_world (ch,target_object)) == NULL)
  {
    send_to_char ("The specified object is not in exsistance.\n\r",ch);
    return;
  }

  //Remove all flags from object.
  obj->extra_flags = 0;
  obj->extra2_flags = 0;

  //Check to see if it was just an object wipe.
  if (extrabits[0] == '\0')
  {
    send_to_char ("All extrabits removed from target item\n\r",ch);
    return;
  }
  //Extrabits are needed to be set.
  else 
  {
    //Reads in each letter of the Extrabits string and adds the flags
    //accordingly.
    for (x = 0; x < strlen(extrabits); x++)
    {  
       extrabits[x] = UPPER(extrabits[x]);
       y = 0;
       while (extrabits_table[y].bitname != NULL)
       {
	    //Checks to see if letter matches table.
	    if (extrabits[x] == extrabits_table[y].letter)
	    //If yes add the bit
          {
	       SET_BIT(obj->extra_flags, extrabits_table[y].bitnumber);
		 sprintf (buf,"%s bit added.\n\r",extrabits_table[y].bitname);
		 send_to_char (buf, ch);
		 break;
          }
	    //If no then go up another letter.
          y++;
       }
    } //End For
  } //Else ending
  return;
}

void do_asperson(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim;
  DESCRIPTOR_DATA *tmp;
  char arg[MAX_INPUT_LENGTH];
  argument = one_argument(argument, arg);
  if (IS_NPC(ch)) return;
  if ((victim = get_char_world(ch, arg)) == NULL)
  {
    send_to_char("They are not here.\n\r", ch );
    return;
  }
  if (victim == ch)
  {
    send_to_char("That would be a bad idea.\n\r", ch );
    return;
  }
  if (IS_NPC(victim))
  {
    send_to_char("Not on mobiles.\n\r",ch);
    return;
  }

  if(!str_cmp(victim->name, "Hiddukel"))
  {
    send_to_char("That would be a bad idea.\n\r", ch );
    return;
  }

  if(!str_cmp(argument, "quit")
  || !str_cmp(argument, "mob")
  || !str_cmp(argument, "delete") )
  {
	send_to_char("That will NOT be done.\n\r",ch);
	return;
  }

  tmp = victim->desc;
  victim->desc = ch->desc;
  interpret(victim, argument);
  victim->desc = tmp;
  return;
}

void do_fslay( CHAR_DATA *ch, char *argument )
{
    {
    CHAR_DATA *victim;
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg );
    one_argument( argument, arg2 );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Syntax: fslay <Char> <Type>\n\r", ch );
	send_to_char( "Types: Skin, Slit, Immolate, Demon, Shatter, 9mm, Deheart, Pounce, Fslay.\n\r", ch);
	return;
    }

    if ( ( victim = get_char_room( ch, NULL, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( ch == victim )
    {
	send_to_char( "Suicide is a mortal sin.\n\r", ch );
	return;
    }

    if ( !IS_NPC(victim) && get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "skin" ) )
    {
    act( "You rip the flesh from $N and send his soul to the fiery depths of the Abyss.", ch, NULL, victim, TO_CHAR );
    act( "Your flesh has been torn from your bones and your bodyless soul now watches your bones incenerate in the fires of the Abyss.", ch, NULL, victim,TO_VICT );
    act( "$n rips the flesh off of $N, releasing his soul into the fiery depths of the Abyss.", ch, NULL, victim, TO_NOTVICT );
	}
/* This one is dedicated to my Head Builder */

	else if ( !str_cmp( arg2, "9mm" ) && get_trust(ch) == MAX_LEVEL )
	{
	act( "You pull out your 9mm and bust a cap in $N's ass.", ch, NULL, victim, TO_CHAR );
	act( "$n pulls out $s 9mm and busts a cap in your ass.", ch, NULL, victim, TO_VICT );
	act( "$n pulls out $s 9mm and busts a cap in $N's ass.", ch, NULL, victim, TO_NOTVICT );
	}

	else if ( !str_cmp( arg2, "deheart" ) )
	{
	act( "You rip through $N's chest and pull out $S beating heart in your hand.", ch, NULL, victim, TO_CHAR );
	act( "You feel a sharp pain as $n rips into your chest and pulls our your beating heart in $s hand.", ch, NULL, victim, TO_VICT );
	act( "Specks of blood hit your face as $n rips through $N's chest pulling out $S beating heart.", ch, NULL, victim, TO_NOTVICT );
	}

    else if ( !str_cmp( arg2, "immolate" ) )
    {
      act( "Your fireball turns $N into a blazing inferno.",  ch, NULL, victim, TO_CHAR    );
      act( "$n releases a searing fireball in your direction.", ch, NULL, victim, TO_VICT    );
      act( "$n points at $N, who bursts into a flaming inferno.",  ch, NULL, victim, TO_NOTVICT );
    }

    else if ( !str_cmp( arg2, "shatter" ) )
    {
      act( "You freeze $N with a glance and shatter the frozen corpse into tiny shards.",  ch, NULL, victim, TO_CHAR    );
      act( "$n freezes you with a glance and shatters your frozen body into tiny shards.", ch, NULL, victim, TO_VICT    );
      act( "$n freezes $N with a glance and shatters the frozen body into tiny shards.",  ch, NULL, victim, TO_NOTVICT );
    }

    else if ( !str_cmp( arg2, "demon" ) )
    {
      act( "You gesture, and a slavering demon appears.  With a horrible grin, the foul creature turns on $N, who screams in panic before being eaten alive.",  ch, NULL, victim, TO_CHAR );
      act( "$n gestures, and a slavering demon appears.  The foul creature turns on you with a horrible grin.   You scream in panic before being eaten alive.",  ch, NULL, victim, TO_VICT );
      act( "$n gestures, and a slavering demon appears.  With a horrible grin, the foul creature turns on $N, who screams in panic before being eaten alive.",  ch, NULL, victim, TO_NOTVICT );
    }

    else if ( !str_cmp( arg2, "pounce" ) )
    {
      act( "Leaping upon $N with bared fangs, you tear open $S throat and toss the corpse to the ground...",  ch, NULL, victim, TO_CHAR );
      act( "In a heartbeat, $n rips $s fangs through your throat!  Your blood sprays and pours to the ground as your life ends...", ch, NULL, victim, TO_VICT );
      act( "Leaping suddenly, $n sinks $s fangs into $N's throat.  As blood sprays and gushes to the ground, $n tosses $N's dying body away.",  ch, NULL, victim, TO_NOTVICT );
    }
 
    else if ( !str_cmp( arg2, "slit" ))
    {
      act( "You calmly slit $N's throat.", ch, NULL, victim, TO_CHAR );
      act( "$n reaches out with a clawed finger and calmly slits your throat.", ch, NULL, victim, TO_VICT );
      act( "A claw extends from $n's hand as $M calmly slits $N's throat.", ch, NULL, victim, TO_NOTVICT );
    }

    else if ( !str_cmp( arg2, "fslay" ))
    {
      act( "You point at $N and fall down laughing.", ch, NULL, victim, TO_CHAR );
      act( "$n points at you and falls down laughing. How embaressing!.", ch, NULL, victim, TO_VICT );
      act( "$n points at $N and falls down laughing.", ch, NULL, victim, TO_NOTVICT );
      return;
    }

    else
    {
      act( "You slay $N in cold blood!",  ch, NULL, victim, TO_CHAR    );
      act( "$n slays you in cold blood!", ch, NULL, victim, TO_VICT    );
      act( "$n slays $N in cold blood!",  ch, NULL, victim, TO_NOTVICT );
    }

    if (!IS_NPC(victim)) //C064
    {
        char_from_room( victim );
        char_to_room( victim, get_home(victim)); //C058
    }
    victim->position = 5;
    victim->hit 	= victim->max_hit - victim->max_hit + 20;
    victim->mana	= victim->mana;
    victim->move	= victim->move;
    return;
 }
}

void do_desocket(CHAR_DATA *ch, char *argument)
{
    DESCRIPTOR_DATA *d;
    int socket;
    char arg[MAX_INPUT_LENGTH];

    one_argument(argument, arg);

    if (!is_number(arg))
    {
        send_to_char("The argument must be a number.\n\r", ch);
        return;
    }

    if (arg[0] == '\0')
    {
        send_to_char("Disconnect which socket?\n\r", ch);
        return;
    }
    else
    {
        socket = atoi(arg);
        for ( d = descriptor_list; d != NULL; d = d->next )      
	{
	    if (d->descriptor == socket)
	    {
	        if (d->character == ch)
		{
		    send_to_char("It would be foolish to disconnect yourself.\n\r", ch);
		    return;
		}
	        if (d->connected == CON_PLAYING)
		{
		    send_to_char("Why don't you just use disconnect?\n\r", ch);
		    return;
		}
	        write_to_descriptor(d->descriptor,
                "\n\r        You are being disconnected from the Legends of Krynn.\n\r"
		    "                    Have a nice day! Buh Bye!",0);
	        close_socket(d);
	        send_to_char("Done.\n\r", ch);
	        return;
	    }
	}
        send_to_char("No such socket is connected.\n\r", ch);
        return;
    }
}

/*Purges all affects from target*/
void do_apurge( CHAR_DATA *ch, char *argument )
{
  AFFECT_DATA *tempaf;
  CHAR_DATA   *victim;
  char arg[MAX_INPUT_LENGTH];
  one_argument( argument, arg );

  if ( arg[0] == '\0' )
  {
    victim = ch;
  } 
  else if ( ( victim = get_char_world( ch, arg ) ) == NULL )
  {
    send_to_char("They are not here.\r\n", ch);
    return;
  }

  if (get_trust(ch) < get_trust(victim))
  {
    act("$n tried to purge all of your affects.", ch, NULL, victim, TO_VICT);
    act("Nice try.", ch, NULL, victim, TO_CHAR);
    return;
  }

  if ( victim->affected == NULL)
  {
    send_to_char("They are not affected by anything.\r\n", ch);
    return;
  }

  while(victim->affected != NULL)
  {
    tempaf = victim->affected;
    affect_remove(victim, tempaf);
  }

  act("$n purges all of your affects.", ch, NULL, victim, TO_VICT);
  act("You purge all of $N's affects.", ch, NULL, victim, TO_CHAR);
  act("$n purges $N of all affects.", ch, NULL, victim, TO_NOTVICT);
}

void display_roomcheck_syntax(CHAR_DATA *ch)
{
  send_to_char("SYNTAX:\n",ch);
  send_to_char("roomcheck <command>\n\r",ch);
  send_to_char("    names   - Check rooms for lack of names.\n\r",ch);
  send_to_char("    white   - Check Room names for white.\n\r",ch);
  send_to_char("    link    - Check Room's for links.\n\r",ch);
  send_to_char("    private - Check Room for Private w/o SAFE.\n\r",ch);
  send_to_char("    descr   - Check Room for BLANK descriptions.\n\r",ch);
  send_to_char("    delete  - Check DELETED status on rooms.\n\r",ch);
}

void do_roomcheck( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_INPUT_LENGTH];
  BUFFER *buffer;
  char arg1[MIL];
  int first = TRUE;
  OBJ_INDEX_DATA *pObjIndex;
  AREA_DATA *area;
  ROOM_INDEX_DATA *room, *room2;
  int number = 0, max_found = 200, vnum=0, tvnum=0, exit = 0;

  argument = one_argument( argument, arg1 );

  if (!str_cmp(arg1,"\0")) {
    display_roomcheck_syntax(ch);
    return;
  }

  buffer = new_buf();
  for ( area = area_first; area; area = area->next )
    {
      for ( vnum = area->min_vnum; vnum <= area->max_vnum; vnum++ )
	{
	if ( !( room = get_room_index( vnum ) ) )
	  continue;
	  if (!strcmp(arg1,"names"))
        {
         if (room->name!=&str_empty[0]) 
          continue;

	    ++number; /*count it if we found a match */
 
	    sprintf(buf, "%3d [%5d] (%s)\n\r",
		     number, vnum, area->name );
	    add_buf( buffer, buf );

	    if ( number >= max_found )
	      break;
	  }
        else if (!strcmp(arg1,"white")) {
	    if ( strstr( room->name,"{W" ) )
	      continue;

	    ++number; /*count it if we found a match */
 
	    sprintf(buf, "%3d [%5d] %s (%s)\n\r",
		     number, vnum, room->name, area->name );
	    add_buf( buffer, buf );

	    if ( number >= max_found )
	      break;
	  } else if (!strcmp(arg1,"private")) {
	    if ( !IS_SET(room->room_flags, ROOM_PRIVATE) )
	      continue;
	    if ( IS_SET(room->room_flags, ROOM_SAFE) )
	      continue;

	    ++number; /*count it if we found a match */
 
	    sprintf(buf, "%3d [%5d] %s (%s)\n\r",
		     number, vnum, room->name, area->name );
	    add_buf( buffer, buf );

	    if ( number >= max_found )
	      break;
	  } else if (!strcmp(arg1,"delete")) {
	    if (!is_name("delete",room->name))
	      continue;
	    
	    ++number; /*count it if we found a match */
 
	    sprintf(buf, "%3d [%5d] %s (%s)\n\r",
		     number, vnum, room->name, area->name );
	    add_buf( buffer, buf );

	    if ( number >= max_found )
	      break;
	  } else if (!strcmp(arg1,"descr")) {
	    if (room->description[0] != '\0') 
	      continue;

	    ++number; /*count it if we found a match */
	    sprintf(buf, "%3d [%5d] %s (%s)\n\r",
		     number, vnum, room->name, area->name );
	    add_buf( buffer, buf );

	    if ( number >= max_found )
	      break;
	  } else if (!strcmp(arg1, "link")) {

	    if (!first) {
	      /* Clear the Linked Flag */
	      for(tvnum=0; tvnum<=top_vnum_room; tvnum++) 
		{
		  if( (room2 = get_room_index(tvnum)) == NULL)
		    continue;
		  room2->linked = FALSE;
		}

	      /* Find all the rooms each room is linked and set the flag */
	      for(tvnum=0; tvnum<=top_vnum_room; tvnum++) 
		{
      
		  if( (room2 = get_room_index(tvnum)) == NULL)
		    continue;
		  for(exit=0; exit<MAX_EXITS; exit++) {
		    if( room2->exit[exit]) {
		      room2->linked = TRUE;
		      room2->exit[exit]->u1.to_room->linked = TRUE;
		    }
		  }
		}
	      /* Find all the portal endings from the objects */
	      for (tvnum = 0; tvnum < top_obj_index; tvnum++)
		if ((pObjIndex = get_obj_index(tvnum)) != NULL)
		  {
		    if (pObjIndex->item_type != ITEM_PORTAL)
		      continue;
		    room2 = get_room_index(pObjIndex->value[3]);
		    if (room2)
		      room2->linked = TRUE;
		  }
	 
	    
	      /* Find all the pet storage rooms */
	      for(tvnum=0; tvnum<=top_vnum_room; tvnum++) 
		{
      
		  if( (room2 = get_room_index(tvnum)) == NULL)
		    continue;

		  if ( !IS_SET(room2->room_flags, ROOM_PET_SHOP) )
		    continue;
      
		  if( (room2 = get_room_index(tvnum+1)) == NULL)
		    continue;
      
		  room2->linked = TRUE;
		}
	      first = FALSE;
	    }

	    if (room->linked)
	      continue;
	    ++number; /*count it if we found a match */
	    sprintf(buf, "%3d [%5d] %s (%s)\n\r",
		     number, vnum, room->name, area->name );
	    add_buf( buffer, buf );

	    if ( number >= max_found )
	      break;
	  
	  }else {
	    display_roomcheck_syntax(ch);
	    return;
	  }
	  if ( number >= max_found )
	    break;
	}
    }
  if ( !number )
    send_to_char( "No matching criteria.\n\r", ch );
  else
    page_to_char(buf_string(buffer),ch);
    
  free_buf(buffer);

}

void do_losereply (CHAR_DATA * ch, char *argument)
{

  char arg1[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  char buf[MAX_INPUT_LENGTH];
  argument = one_argument (argument, arg1);
  if (arg1[0] == '\0')
    {
      send_to_char ("Who do you want to lose their reply?\n\r", ch);
      return;
    }

  if ((victim = get_char_world (ch, arg1)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }
  victim->reply = NULL;
  sprintf (buf, "Reply lost for %s.\n\r", victim->name);
  send_to_char (buf, ch);
}

void do_addapply(CHAR_DATA *ch, char *argument)
{
	OBJ_DATA *obj;
	AFFECT_DATA paf;
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_INPUT_LENGTH];
	int affect_modify = 0, bit = 0, enchant_type, pos, i;
	
	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	argument = one_argument( argument, arg3 );
	
	if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
	{
	send_to_char("Syntax for applies: addapply <object> <apply type> <value>\n\r",ch);
	send_to_char("Apply Types: hp str dex int wis con mana\n\r", ch);
	send_to_char("             ac move hitroll damroll saves\n\r\n\r", ch);
	send_to_char("Syntax for affects: addapply <object> affect <affect name>\n\r",ch);
	send_to_char("Affect Names: blind invisible detect_evil detect_invis detect_magic\n\r",ch);
	send_to_char("              detect_hidden detect_good sanctuary infrared\n\r",ch);
	send_to_char("              curse poison protect_evil protect_good sneak hide sleep charm\n\r", ch);
	send_to_char("              flying pass_door haste calm plague weaken dark_vision berserk\n\r", ch);
	send_to_char("              regeneration slow\n\r", ch);
	return;
	}
	
	if ((obj = get_obj_here(ch, NULL, arg1)) == NULL)
	{
		send_to_char("No such object exists!\n\r",ch);
		return;
	}
	
	if (!str_prefix(arg2,"hp"))
		enchant_type=APPLY_HIT;
	else if (!str_prefix(arg2,"str"))
		enchant_type=APPLY_STR;
	else if (!str_prefix(arg2,"dex"))
		enchant_type=APPLY_DEX;
	else if (!str_prefix(arg2,"int"))
		enchant_type=APPLY_INT;
	else if (!str_prefix(arg2,"wis"))
		enchant_type=APPLY_WIS;
	else if (!str_prefix(arg2,"con"))
		enchant_type=APPLY_CON;
	else if (!str_prefix(arg2,"mana"))
		enchant_type=APPLY_MANA;
	else if (!str_prefix(arg2,"move"))
		enchant_type=APPLY_MOVE;
	else if (!str_prefix(arg2,"ac"))
		enchant_type=APPLY_AC;
	else if (!str_prefix(arg2,"hitroll"))
		enchant_type=APPLY_HITROLL;
	else if (!str_prefix(arg2,"damroll"))
		enchant_type=APPLY_DAMROLL;
	else if (!str_prefix(arg2,"saves"))
		enchant_type=APPLY_SAVING_SPELL;
	else if (!str_prefix(arg2,"affect"))
		enchant_type=APPLY_SPELL_AFFECT;
	else
	{
		send_to_char("That apply is not possible!\n\r",ch);
		return;
	}
	
	if (enchant_type==APPLY_SPELL_AFFECT)
	{
		for (pos = 0; affect_flags[pos].name != NULL; pos++)
			if (!str_cmp(affect_flags[pos].name,arg3))
				bit = affect_flags[pos].bit;
	}
	else
	{
		if ( is_number(arg3) )
			affect_modify=atoi(arg3);  
		else
		{
			send_to_char("Applies require a value.\n\r", ch);
			return;
		}
	}
		
	affect_enchant(obj);

	/* create the affect */
    	paf.where	= TO_AFFECTS;
    	paf.type	= 0;
	paf.level	= ch->level;
	paf.duration	= -1;
	paf.location	= enchant_type;
	paf.modifier	= affect_modify;
	paf.bitvector	= bit;
	
	if ( enchant_type == APPLY_SPELL_AFFECT )
	{	
        	for ( i=0 ; arg3[i] != '\0'; i++ )
        	{
            		if ( arg3[i] == '_' )
                		arg3[i] = ' ';
        	}	
        	paf.type      = skill_lookup(arg3);
	}
	affect_to_obj(obj,&paf);
	send_to_char("Ok.\n\r", ch);
}

void do_nonote(CHAR_DATA *ch, char *argument)
{
   char      arg[MAX_INPUT_LENGTH];
   char      buf[MAX_STRING_LENGTH];
   CHAR_DATA *victim;

   argument = one_argument(argument, arg);
   victim   = get_char_world(ch, arg);

   if (arg[0] == '\0')
      {
      send_to_char("Whose Note Priviliges do you wish to alter?\n\r", ch);
      return;
      }

	if ((victim = get_char_world( ch, arg ))==NULL )
      {
      send_to_char("They aren't of this world!\n\r", ch);
      return;
      }
   else
      {
      if (IS_NPC(victim))
         {
         send_to_char("Not on NPC's!\n\r", ch);
         return;
         }
	else if (IS_SET(victim->act, PLR_NONOTE))
 	   {
	   REMOVE_BIT(victim->act, PLR_NONOTE);
         sprintf(buf, "You have allowed %s to write notes to everyone!\n\r", victim->name);
         send_to_char(buf, ch);
         return;
         }
      else
         {
	   SET_BIT(victim->act, PLR_NONOTE);
         sprintf(buf, "You have made it so %s will no longer be able to write notes to everyone!\n\r", victim->name);
         send_to_char(buf, ch);
         return;
         }
      }
}

void do_fvlist (CHAR_DATA *ch, char *argument)
{
  int i,j = 0;
  char arg[MAX_INPUT_LENGTH];
  char *string;

  string = one_argument(argument,arg);
 
  if (arg[0] == '\0')
  {
      send_to_char("Syntax:\n\r",ch);
      send_to_char("  fvlist obj\n\r",ch);
      send_to_char("  fvlist mob\n\r",ch);
      send_to_char("  fvlist room\n\r",ch);
      return;
  }

  if (!str_cmp(arg,"obj"))
  {
      printf_to_char(ch,"{WFree {C%s{W vnum listing for area {C%s{x\n\r",arg,
		     ch->in_room->area->name);
      printf_to_char(ch,"{Y=============================================================================={C\n\r");
      for (i = ch->in_room->area->min_vnum; i <= ch->in_room->area->max_vnum; i++)
	{
	  if (get_obj_index(i) == NULL) 
        {
	    printf_to_char(ch,"%8d, ",i);
	    j++;
	  }
     	}
      send_to_char("{x\n\r",ch);
      return;
  }

  if (!str_cmp(arg,"mob"))
  { 
      printf_to_char(ch,"{WFree {C%s {Wvnum listing for area {C%s{x\n\r",arg,
		     ch->in_room->area->name);
      printf_to_char(ch,"{Y=============================================================================={C\n\r");
      for (i = ch->in_room->area->min_vnum; i <= ch->in_room->area->max_vnum; i++)
      {
	  if (get_mob_index(i) == NULL)
        {
	    printf_to_char(ch,"%8d, ",i);
	    j++;
	  }
      }
      send_to_char("{x\n\r",ch);
      return;
  }

  if (!str_cmp(arg,"room"))
  { 
      printf_to_char(ch,"{WFree {C%s {Wvnum listing for area {C%s{x\n\r",arg,
		     ch->in_room->area->name);
      printf_to_char(ch,"{Y=============================================================================={C\n\r");
      for (i = ch->in_room->area->min_vnum; i <= ch->in_room->area->max_vnum; i++)
	{
	  if (get_room_index(i) == NULL)
	  {
	    printf_to_char(ch,"%8d, ",i);
	    j++;
	  }
    }
    send_to_char("{x\n\r",ch);
    return;
  }
  send_to_char("WHAT??? \n\r",ch);
  send_to_char("Syntax:\n\r",ch);
  send_to_char("  fvlist obj\n\r",ch);
  send_to_char("  fvlist mob\n\r",ch);
  send_to_char("  fvlist room\n\r",ch);
}

void do_prayfile(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_STRING_LENGTH];
    FILE *fp;
    char buf[MSL];
    int x;

    argument = one_argument( argument, arg );


    if ( arg[0] == '\0')
    {
	if (get_trust(ch) >= MAX_LEVEL - 1)
      {
        send_to_char( "Syntax: prayfile <delete/read>\n\r", ch );
        return;
	}
	else
      {
	  send_to_char( "Syntax: prayfile <read>\n\r", ch );
	  return;
	}
    }


    if (!str_prefix(arg,"delete"))
    {
	if (get_trust(ch) >= MAX_LEVEL - 1)
      {
        fclose(fopen("pray.txt","w"));
        send_to_char("Pray logfile has been cleaned.\n\r", ch);
        return;
      }
      else
      {
	  send_to_char( "You do not have the authority to delete the prayfile\n\r", ch );
	  return;
	}
    }
    else if (!str_prefix(arg,"read"))
    {
        if ( (fp = fopen("pray.txt","r")) != NULL )
        {
                while (!feof(fp))
                {
                        if ((x = fread(&buf, 1, MSL, fp)) == 0) break;
                        buf[x] = 0;
                        send_to_char(buf, ch);
                }
                fclose(fp);
        }
    return;
    }
    else
    {
	if (get_trust(ch) >= MAX_LEVEL - 1)
      {
        send_to_char( "Syntax: prayfile <delete/read>\n\r", ch );
        return;
	}
	else
      {
	  send_to_char( "Syntax: prayfile <read>\n\r", ch );
	  return;
	}
    }

    return;
}

void do_vsearch(CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];
    BUFFER *buffer;
    OBJ_DATA *obj;
    OBJ_DATA *in_obj;
    bool found;
    int number = 0, max_found;
    int argi;

    found = FALSE;
    number = 0;
    max_found = 200;

    buffer = new_buf();

    one_argument( argument, arg );

    if (argument[0] == '\0')
    {
        send_to_char( "Syntax:  vsearch <vnum>\n\r", ch );
	return;
    }

    argi = atoi(arg);

    if (argi < 0 && argi > 20000)
    {
	send_to_char( "Vnum out of range.\n\r", ch);
	return;
    }

    if ((argi < 1273 && argi > 1230)
    && (ch->level < 110))
    {
	send_to_char( "You may not do a VSEARCH on Immortal Marks, especially when trying to exploit those Immortals above you who are WIZINVIS.\n\r", ch);
	return;
    }
 
    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
        if ( !can_see_obj( ch, obj )
	|| !( argi == obj->pIndexData->vnum ))
            continue;
 
        found = TRUE;
        number++;
 
     for ( in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj );
        if ( in_obj->carried_by != NULL
	&& can_see(ch,in_obj->carried_by)
	&& in_obj->carried_by->in_room != NULL)
            sprintf( buf, "%3d) %s is carried by %s [Room %d]\n\r",
                number,
		obj->short_descr,
		PERS(in_obj->carried_by, ch),
		in_obj->carried_by->in_room->vnum );
        else
	if (in_obj->in_room != NULL
	&& can_see_room(ch,in_obj->in_room))
            sprintf( buf, "%3d) %s is in %s [Room %d]\n\r",
                number,
		obj->short_descr,
		in_obj->in_room->name, 
	   	in_obj->in_room->vnum);
	else
            sprintf( buf, "%3d) %s is somewhere\n\r",
	    number,
	    obj->short_descr);
 
        buf[0] = UPPER(buf[0]);
        add_buf(buffer,buf);
 
        if (number >= max_found)
            break;
    }
 
    if ( !found )
        send_to_char( "Nothing like that in Heaven, Hell or Earth.\n\r", ch );
    else
        page_to_char(buf_string(buffer),ch);

    free_buf(buffer);
}

void do_reward( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int amount;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    amount = atoi(arg2);

    if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) 
    ||   arg3[0] == '\0' )
    {
        send_to_char( "Syntax: reward <char> <experience> <roleplay,nice,behavior,help>.\n\r", ch );
        send_to_char( "\n\r", ch );
        send_to_char( "{rRoleplay{x - Their excellent Roleplay.\n\r", ch );
        send_to_char( "{rNice{x     - Your just feeling Nice.\n\r", ch );
        send_to_char( "{rBehavior{x - They are on very good behavior.\n\r", ch );
        send_to_char( "{rHelp{x     - They offer to help other players, especially newbies.\n\r", ch );
        return;
    }

    if ( ( victim = get_char_room( ch, NULL, arg1 ) ) == NULL )
    {
        send_to_char( "That player is not here.\n\r", ch);
        return;
    }

    if ( IS_NPC(victim) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }

    if ( ch == victim )
    {
        send_to_char("You cannot reward yourself.\n\r",ch);
        return;
    }

    if ( IS_HERO( victim ))
    {
        send_to_char( "You cannot reward heros and above.\n\r", ch );
        return;
    }

    if (amount < 1)
    {
	send_to_char( "You cannot take away from their experience.\n\r", ch );
	return;
    }

    if((victim->reward_timer) && !IS_IMPLEMENTOR(ch))
    {
	send_to_char( "They can NOT be rewarded again so soon. Players are only allowed to be rewarded\n\r", ch );
	send_to_char( "once, and then it must be a while before they can be rewarded again. If you\n\r", ch );
	send_to_char( "feel they deserve a higher reward than what you can give them, seek a higher Immortal!\n\r", ch );
	return;
    }

    switch ( ch->level )
    {
	case MAX_LEVEL:
	    if (amount > 100000)
	    {
		act("You can only reward up to 100,000 experience points.",victim,NULL,ch,TO_VICT);
		return;
	    }
	break;
	case(MAX_LEVEL-1):
	    if (amount > 5000)
	    {
		act("You can only reward up to 5,000 experience points.",victim,NULL,ch,TO_VICT);
		return;
	    }
	break;
	case(MAX_LEVEL-2):
	    if (amount > 3500)
	    {
		act("You can only reward up to 3,500 experience points.",victim,NULL,ch,TO_VICT);
		return;
	    }
	break;
	case(MAX_LEVEL-3):
	    if (amount > 2500)
	    {
		act("You can only reward up to 2,500 experience points.",victim,NULL,ch,TO_VICT);
		return;
	    }
	break;
	case(MAX_LEVEL-4):
	    if (amount > 1500)
	    {
		act("You can only reward up to 1,500 experience points.",victim,NULL,ch,TO_VICT);
		return;
	    }
	break;
	case(MAX_LEVEL-5):
	    if (amount > 1000)
	    {
		act("You can only reward up to 1,000 experience points.",victim,NULL,ch,TO_VICT);
		return;
	    }
	break;
	case(MAX_LEVEL-6):
	    if (amount > 500)
	    {
		act("You can only reward up to 500 experience.",victim,NULL,ch,TO_VICT);
		return;
	    }
	break;
	default:
	    if (amount > 100)
	    {
		act("You can only reward up to 100 experience.",victim,NULL,ch,TO_VICT);
		return;
	    }
	break;
	
    }

    if (!str_cmp( arg3, "roleplay" ))
    {
    act("{c$N rewards you $t experience for your roleplaying!{x",
        victim,arg2,ch,TO_CHAR);
    act("{cYou reward $n $t experience for their roleplaying!{x",
        victim,arg2,ch,TO_VICT);
    }
    else
    if (!str_cmp( arg3, "nice" ))
    {
    act("{c$N rewards you $t experience JUST because $E's feeling chirpy and nice!{x",
        victim,arg2,ch,TO_CHAR);
    act("{cYou reward $n $t experience JUST because your feeling all chirpy and nice!{x",
        victim,arg2,ch,TO_VICT);
    }
    else
    if (!str_cmp( arg3, "behavior" ))
    {
    act("{c$N rewards you $t experience for your outstanding behavior!{x",
        victim,arg2,ch,TO_CHAR);
    act("{cYou reward $n $t experience for their outstanding behavior!{x",
        victim,arg2,ch,TO_VICT);
    }
    else
    if (!str_cmp( arg3, "help" ))
    {
    act("{c$N rewards you $t experience for helping out other player(s)!{x",
        victim,arg2,ch,TO_CHAR);
    act("{cYou reward $n $t experience for helping out other player(s)!{x",
        victim,arg2,ch,TO_VICT);
    }
    else
    {
      send_to_char("Valid reasons are: Roleplay, Nice, Behavior, and Help.\n\r",ch);
      return;
    }

    victim->reward_timer = 100;
    gain_exp(victim, amount);
    return;
}

void do_penalize( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int amount;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    amount = atoi(arg2);

    if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) )
    {
        send_to_char( "Syntax: penalize <char> <experience>.\n\r", ch );
        return;
    }

    if ( ( victim = get_char_room( ch, NULL, arg1 ) ) == NULL )
    {
        send_to_char( "That player is not here.\n\r", ch);
        return;
    }

    if ( IS_NPC(victim) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }

    if ( ch == victim )
    {
        send_to_char("You cannot penalize yourself.\n\n\r.",ch);
        return;
    }

    if ( IS_HERO( victim ))
    {
        send_to_char( "You cannot reward heros and above.\n\r", ch );
        return;
    }

    if (amount < 1)
    {
	send_to_char( "You can penalize someone with MORE experience.\n\r", ch );
	return;
    }

    if(victim->penalize_timer)
    {
	send_to_char( "They can NOT be penalized again so soon. Players are only allowed to be penalized\n\r", ch );
	send_to_char( "once, and then it must be a while before they can be penalized again. If you\n\r", ch );
	send_to_char( "feel they deserve a higher penalty than what you can give them, seek a higher Immortal!\n\r", ch );
	return;
    }

    switch ( ch->level )
    {
	case MAX_LEVEL:
	    if (amount > 100000)
	    {
		act("You can only penalize up to 100,000 experience.",victim,NULL,ch,TO_VICT);
		return;
	    }
	break;
	case(MAX_LEVEL-1):
	    if (amount > 5000)
	    {
		act("You can only penalize up to 5,000 experience.",victim,NULL,ch,TO_VICT);
		return;
	    }
	break;
	case(MAX_LEVEL-2):
	    if (amount > 3500)
	    {
		act("You can only penalize up to 3,500 experience.",victim,NULL,ch,TO_VICT);
		return;
	    }
	break;
	case(MAX_LEVEL-3):
	    if (amount > 2500)
	    {
		act("You can only penalize up to 2,500 experience.",victim,NULL,ch,TO_VICT);
		return;
	    }
	break;
	case(MAX_LEVEL-4):
	    if (amount > 1500)
	    {
		act("You can only penalize up to 1,500 experience.",victim,NULL,ch,TO_VICT);
		return;
	    }
	break;
	case(MAX_LEVEL-5):
	    if (amount > 1000)
	    {
		act("You can only penalize up to 1,000 experience.",victim,NULL,ch,TO_VICT);
		return;
	    }
	break;
	case(MAX_LEVEL-6):
	    if (amount > 500)
	    {
		act("You can only penalize up to 500 experience.",victim,NULL,ch,TO_VICT);
		return;
	    }
	break;
	default:
	    if (amount > 100)
	    {
		act("You can only penalize 100 experience.",victim,NULL,ch,TO_VICT);
		return;
	    }
	break;
	
    }
		 
    act("{R$N penalizes you $t experience for your poor roleplaying.{x",
        victim,arg2,ch,TO_CHAR);
    act("{RYou penalize $n $t experience for their poor roleplaying.{x",
        victim,arg2,ch,TO_VICT);
 
    victim->penalize_timer = 100;
    victim->exp -= amount;
    return;
}

void do_immortalfy( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int sn;

    argument = one_argument( argument, arg1 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Syntax: immortalfy <char>.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "That player is not here.\n\r", ch);
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( !IS_IMMORTAL(victim) )
    {
	send_to_char( "You can only set up Immortals with these stats.\n\r", ch );
	return;
    }

    sprintf(buf,"Setting %s up with general Immortal stats!\n\r", victim->name );
    send_to_char(buf,ch);
    send_to_char( "Make sure you remove their NOTITLE flag and give them a custom title.\n\r", ch );
    victim->max_hit  			= 30000;
    victim->max_mana 			= 30000;
    victim->max_move 			= 30000;
    victim->practice 			= 0;
    victim->hit      			= victim->max_hit;
    victim->mana     			= victim->max_mana;
    victim->move     			= victim->max_move;
    victim->pcdata->security		= 1;
    victim->perm_stat[STAT_STR]	= 35;
    victim->perm_stat[STAT_INT]	= 35;
    victim->perm_stat[STAT_DEX]	= 35;
    victim->perm_stat[STAT_CON]	= 35;
    victim->perm_stat[STAT_WIS]	= 35;
    victim->perm_stat[STAT_CHR]	= 35;
    victim->steel				= 20000;
    for ( sn = 0; sn < MAX_SKILL; sn++ )
    {
      if ( skill_table[sn].name != NULL )
      victim->pcdata->learned[sn]   = 100;
    }

    sprintf(buf,"You have been setup with basic Immortal stats.\n\r");
    send_to_char(buf,victim);
    save_char_obj(victim);
    return;
}

void do_questreset(CHAR_DATA *ch,char *argument)
{
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];

  argument = one_argument(argument,arg);
  if (arg[0] == '\0')
  { 
    send_to_char("Syntax: questreset <player>\n\r",ch);
    return; 
  }                            

  if ((victim = get_char_world(ch,arg)) == NULL)
  { 
    send_to_char("They aren't here.\n\r",ch);
    return; 
  }

  if (IS_NPC(victim))
  { send_to_char("Mobs dont quest.\n\r",ch);
    return; }

  if (IS_SET(victim->act,PLR_QUESTOR))
  REMOVE_BIT(victim->act,PLR_QUESTOR);
  victim->questgiver    = NULL;
  victim->countdown     = 0;
  victim->questmob      = 0;
  victim->questobj      = 0;
  victim->nextquest     = 0;                                         

  printf_to_char(victim,"%s has cleared your quest.\n\r",PERS(ch,victim));
  printf_to_char(ch,"You clear %s's quest.\n\r",victim->name);
  return;
}

void do_auto_shutdown()
{
   FILE *fp;
   DESCRIPTOR_DATA *d, *d_next;
   char buf [100], buf2[100];
   char *strtime;
   extern int port,control;

   strtime = ctime(&current_time);
   strtime[strlen(strtime)-1] = '\0';

   /* This is to write to the file. */
   fclose(fpReserve);
   if((fp = fopen(LAST_COMMAND,"a")) == NULL)
   bug("Error in do_auto_save opening last_command.txt",0);
   
   fprintf(fp,"Last Command: %s : %s\n", strtime, last_command);
   fclose( fp );
   fpReserve = fopen( NULL_FILE, "r" );
	
   fp = fopen (COPYOVER_FILE, "w");   

   if (!fp)
   {
       for (d = descriptor_list;d != NULL;d = d_next)
       {
           if(d->character)
           {
              do_function( d->character, &do_save, "" );
              send_to_char("Ok I tried but we're crashing anyway sorry!\n\r",d->character);
           }

           d_next=d->next;
           close_socket(d);
       }
         
       exit(1);
   }
	
   do_function( NULL, &do_asave, "changed" );

   sprintf(buf,"\n\r\n\r          We are experiencing technical difficulties! Please stand by!\n\r\n\r");
   
   for (d = descriptor_list; d ; d = d_next)
   {
	CHAR_DATA * och = CH (d);
	d_next = d->next; /* We delete from the list , so need to save this */
		
	if (!d->character || d->connected > CON_PLAYING) 
	{
	    write_to_descriptor (d->descriptor, "\n\rSorry, we are rebooting. Come back in a few minutes.\n\r", 0);
	    close_socket (d); /* throw'em out */
	}
		
        else
	{
	    fprintf (fp, "%d %s %s\n", d->descriptor, och->name, d->host);
	    save_char_obj (och);
	    write_to_descriptor (d->descriptor, buf, 0);
	}
   }
	
   fprintf (fp, "-1\n");
   fclose(fp);
   fclose (fpReserve);
   sprintf (buf, "%d", port);
   sprintf (buf2, "%d", control);
   execl (EXE_FILE, "rom", buf, "copyover", buf2, (char *) NULL);   
   exit(1);
}

void do_illegalname( CHAR_DATA *ch, char *argument)
{
  char strsave[2*MIL],
  namelist[2*MSL],
  nameread[2*MSL],
  name[2*MSL];
  FILE *fp;

  if ( argument[0] == '\0' )
  {
	send_to_char( "syntax: illegalname <name>.\n\r", ch );
	return;
  }

  if( !check_parse_name(argument) )
  {
	send_to_char("That name is already illegal.\n\r",ch);
	return;
  }

  fclose( fpReserve );
  sprintf( strsave, "%s", ILLEGAL_NAME_FILE );
  sprintf(name, "%s\n", argument);
  sprintf(namelist,"%s","");
  if ( (fp = fopen( strsave, "r" ) ) != NULL )
  {
    for ( ; ; )
    {
      fscanf (fp, "%s", nameread);
	if ( !str_cmp( nameread, "END" ) )
        break;
	else
	{
	  strcat(namelist, nameread);
	  strcat(namelist,"\n");
	}
    }
  }
  else
    fp = fopen( NULL_FILE, "r" );
    fclose( fp );
    fp = fopen( strsave, "w" );
    strcat( namelist, name );
    fprintf( fp, "%s", namelist );
    fprintf( fp, "END" );
    fclose( fp );
    fpReserve = fopen( NULL_FILE, "r" );
    send_to_char( "All set, that name is now illegal.\n\r",ch);
}

void do_sgive(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	OBJ_DATA *obj;

	argument = one_argument(argument, arg);
	one_argument(argument, arg2);

	if(arg[0] == '\0' || arg2[0] == '\0')
	{
		send_to_char("Syntax: sgive <item> <player> - Silently give an item to a player in the world.\n\r",ch);
		return;
	}

	if( (obj = get_obj_carry(ch, arg, ch)) == NULL )
	{
		send_to_char("You do not have that item.\n\r",ch);
		return;
	}

	if( (victim = get_char_world(ch, arg2)) == NULL )
	{
		send_to_char("They aren't here.\n\r",ch);
		return;
	}

	if( IS_NPC(victim) )
	{
		send_to_char("You may only sgive to players. Slap yourself!\n\r",ch);
		return;
	}

	obj_from_char(obj);
	obj_to_char(obj, victim);
	act("You silently give $p to $N.",ch,obj,victim,TO_CHAR);
	return;
}

void do_pprestar( CHAR_DATA *ch, char *argument )
{
	send_to_char("If you want to restart the Playerport, you must SPELL out pprestart.\n\r",ch);
	return;
}

//For the sake of all sanity... use check process first!
void do_pprestart( CHAR_DATA *ch, char *argument ) 
{
	char cmd[MAX_STRING_LENGTH];
	bool head_god = FALSE;
	extern int port;

	head_god = !str_cmp(ch->name, HEAD_GOD) 
	 	  || !str_cmp(ch->name, HEAD_GOD2)
	 	  || !str_cmp(ch->name, HEAD_GOD3);

	if(!head_god)
	{
		send_to_char("You are not authorized to use this command.\n\r",ch);
		return;
	}

	if(IS_PP(port))
	{
		send_to_char("This is the PlayerPort, you cannot use that command here.\n\r",ch);
		return;
	}

	if(argument[0] != '\0')
	{
		send_to_char("No argument. Just use the command.\n\r",ch);
		return;
	}

	sprintf(cmd, "csh ../../playerport/src/startup &");
	system(cmd);

	send_to_char("Playerport restart complete.\n\r",ch);
      return;
}

/*
 * do_rstrip
 *
 * Remove someone from a religion.  Mainly for Head God 
 *
 * Usage
 *   rstrip [immortal]  (head god only)
 *   rstrip [mortal] 
 */
void do_rstrip( CHAR_DATA *ch, char *argument )
{
    char buf[100];
    CHAR_DATA *gch;
    OBJ_DATA *obj;
    int religion = 0;
    int head_god = FALSE;
    /* check qualifications. must be leader or head god */
    head_god = !str_cmp(ch->name, HEAD_GOD) || !str_cmp(ch->name,HEAD_GOD2)
            || !str_cmp(ch->name, HEAD_GOD3);

    if ( !head_god && !( ch->religion_id & RELIGION_LEADER ) )
    {
      send_to_char( "You are not qualified to strip religions.\n\r", ch );
      return;
    }
  
    /* make sure they spec'd an inductee */
    if( argument[0] == 0)
    {
      send_to_char( "Strip who's religion?\n\r", ch );
      return;
    }

    if ( ( gch = get_char_world( ch, argument ) ) == NULL )
    {
      send_to_char( "They aren't here.\n\r", ch );
      return;
    }

    if( IS_NPC(gch) )
    {
      send_to_char( "You can only strip player's religions.\n\r", ch );
      return;
    }

    if( gch->devotee_id == 0 
    &&  gch->religion_id == RELIGION_NONE )
    {
      send_to_char( "They are not in any religion nor devoted to any God\n\r", ch );
      return;
    }

    /* see if they're an immort (only HEAD_GOD can do 'em) */
    if( gch->level >= LEVEL_IMMORTAL )
    {
      if( !head_god )
      {
        send_to_char( "You are not qualified to strip other religions.\n\r",ch );
        return;
      }
    }

    /* mortal strip-age */
    else 
    {
      /* make sure they're in proper religion */
      if( RELIGION(gch->religion_id) != RELIGION( ch->religion_id) && !head_god )
      {
        send_to_char( "You cannot choose the followers of other gods.\n\r",ch );
        return;
      }

      /* check for leader eviction */
      if( gch->religion_id & RELIGION_LEADER )
      {
        if( !head_god )
        {
          send_to_char( "Only the Head God can alter set religions.\n\r", ch );
          return;
        }
      }
    }

    /* we're cleared to do a religion strip */
    /* strip and inform of departure */
    religion = RELIGION( gch->religion_id );
    sprintf( buf, "%s strips you of your religion!\n\r", ch->name );
    send_to_char( buf, gch );
    sprintf( buf, "%s strips %s of their religion!\n\r", ch->name, gch->name );
    gch->religion_id = RELIGION_NONE;
    gch->pcdata->highpriest = 0;
    gch->devotee_id = 0;
    /* make sure evictor gets notice */
    sprintf( buf, "You strip %s of their religion!\n\r", gch->name );
    send_to_char( buf, ch );
    /* Removes mark from character upon strip-age */
    obj = get_eq_char( gch, WEAR_MARK );
    if ( ( obj = get_eq_char( gch, WEAR_MARK ) ) == NULL )
      return;
    else
      extract_obj ( obj );
}

int days_since_last_file_mod( char *filename )
{
    int days;
    struct stat buf;
    extern time_t current_time;

    if (!stat(filename, &buf))
    {
	days = (current_time - buf.st_mtime)/86400;
    }
    else
	days = 0;

    return days;
}

void do_plist( CHAR_DATA *ch, char *argument )
{
    DIR *dp;
    struct dirent *ep;
    char buf[MAX_STRING_LENGTH];
    BUFFER *buffer;
    int days;
    bool fAll = TRUE, fImmortal = FALSE;
    int col = 0;
    buffer = new_buf();

    if ( argument[0] == '\0' || !str_cmp( argument, "all" ) )
    {
	fAll = TRUE;
    }
    else
    if ( !str_prefix( argument, "immortal" ) )
    {
	fImmortal = TRUE;
    }
    else
    {
	send_to_char( "Syntax: PLIST [ALL/IMMORTAL]\n\r", ch );
	return;
    }

    if ( fImmortal )
    {
	/*
	 * change the string literals to GODS_DIR or where
	 * you keep your gods folder
	 */
	dp = opendir ("../gods");
    }
    else
    {
	/*
	 * change the string literals to PLAYER_DIR or where
	 * you keep your gods folder
	 */
	dp = opendir ("../player");
    }

    if (dp != NULL)
    {

 	while ( (ep = readdir (dp)) )
      {
	  if ( ep->d_name[0] == '.' )
	    continue;

	  sprintf( buf, "%s%s",
          fImmortal ? "../gods/" : "../player/", ep->d_name );

	  days = days_since_last_file_mod( buf );

	/*
	 * color coding using Lopes comaptable color to highlight
	 * inactivity. green = active, red = innactive
	 * Just remove the colors if they cause problems.
	 */
	 sprintf( buf, "%-15s %s%-3d{x days\n\r",
	   ep->d_name,
         days > 30 ? "{r" : days > 20 ? "{Y" : days > 10 ? "{g" : "{G", days );
	   add_buf( buffer, buf );
       }
       closedir (dp);
    }
    else
    	perror ("Couldn't open the directory");

    if ( col % 3 != 0 )
	add_buf( buffer, "\n\r" );

    page_to_char( buf_string(buffer), ch );

  return;
}

/* Backup all pfiles to players.tar.gz
 * This is also used automatically when the game is started. */
void do_pfile_backup( CHAR_DATA *ch, char *arguments )
{
	DIR *dir;
	struct dirent *file;
	int days = 0;
	bool backup = FALSE;

	dir = opendir("../");

	if(dir == NULL)
	{
		log_string("Couldn't open parent directory.");
		return;
	}

	while( (file = readdir(dir)) )
	{
		if(file->d_name[0] == '.') /* Don't touch Current or Parent Directory */
			continue;

		if( !str_cmp(file->d_name, "players.tar.gz") ) /* Look for backup file */
		{
			/* Now, we have out backup file. Check when it was last modified. */
			days = days_since_last_file_mod("../players.tar.gz");

			if(!ch && days < 30) //30 days between automatic backups.
			{
				backup = TRUE;
				break;
			}

			if(!ch) log_string("Automatic Player Backup is commencing now.");

			/* Remove previous backup, and create anew. */
			system("rm ../players.tar.gz");
			system("tar -cf ../players.tar ../player/*");
			system("gzip ../players.tar");
			
			if(ch) send_to_char("Player backup created successfully.\n\r",ch);
			log_string("Player backup created successfully.");
			backup = TRUE;
			break;
		}
	}
	closedir(dir);
	if(backup) return;

	if(ch) send_to_char("Could not backup players. Please report to Paladine.\n\r",ch);
	log_string("Could not backup players. Please report to Paladine.");
	return;
}

void do_nopk( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];

	one_argument(argument, arg);

	if(arg[0] == '\0')
	{
		send_to_char("Alter who's PK Privileges?\n\r",ch);
		return;
	}

	if( (victim = get_char_world(ch, arg)) == NULL)
	{
		send_to_char("No such player online.\n\r",ch);
		return;
	}

	if( victim->level > get_trust(ch) )
	{
		send_to_char("They wouldn't like that.\n\r",ch);
		return;
	}

	/* We are clear. */
	if( !IS_SET(victim->comm, COMM_NOPKILL) )
	{
		SET_BIT(victim->comm, COMM_NOPKILL);
		sprintf(buf,"You have removed PK Privileges from %s.\n\r",victim->name);
		send_to_char(buf,ch);
		send_to_char("Your ability to PK has been removed by the Gods!\n\r",victim);
		sprintf(buf,"%s revokes PK Privileges of %s",ch->name, victim->name);
		wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
		return;
	}
	else
	{
		REMOVE_BIT(victim->comm, COMM_NOPKILL);
		sprintf(buf, "You have restored PK Privileges to %s.\n\r",victim->name);
		send_to_char(buf,ch);
		send_to_char("You find yourself able to PK again.\n\r",victim);
		sprintf(buf,"%s restores PK Privileges of %s",ch->name, victim->name);
		wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
		return;
	}
	bug("Uh, something's not right in do_nopk.",0);
	return;
}

void do_rwhere( CHAR_DATA *ch, char *argument)
{
	AREA_DATA *pArea;
	ROOM_INDEX_DATA *room;
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	int parsevnum, num_rooms;
	BUFFER *buffer;

	buffer = new_buf();
	one_argument(argument,arg);
	parsevnum = num_rooms = 0;

	if(arg[0] == '\0')
	{
		send_to_char("Syntax: rwhere <room>\n\r",ch);
		return;
	}

	add_buf(buffer, "[Vnum ] Room Name\n\r"); 
	add_buf(buffer, "\n\r");
	for(pArea = area_first; pArea != NULL; pArea = pArea->next)
	{
		if(buffer->size >= 16000)
		{
			add_buf(buffer, "\n\rBuffer Overflow. Stopping.\n\r");
			break;
		}

		for(parsevnum = pArea->min_vnum; parsevnum <= pArea->max_vnum; parsevnum++)
		{
			if(buffer->size >= 16000)
				break;

			if( (room = get_room_index(parsevnum)) == NULL)
				continue;

			if( !is_name(arg, room->name) )
				continue;

			sprintf(buf, "[%-5d] %s\n\r",room->vnum, room->name);
			add_buf(buffer, buf);
			num_rooms++;
		}
	}

	if(num_rooms == 0)
		add_buf(buffer, "No rooms found.\n\r");

	page_to_char(buf_string(buffer), ch);
	free_buf(buffer);
	return;
}

void do_slowage( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];

	one_argument(argument, arg);

	if(arg[0] == '\0')
	{
		send_to_char("Slow down the aging process by half on whom?\n\r",ch);
		return;
	}

	if( (victim = get_char_world(ch, arg)) == NULL)
	{
		send_to_char("No such player online.\n\r",ch);
		return;
	}

	if( victim->level > get_trust(ch) )
	{
		send_to_char("They wouldn't like that.\n\r",ch);
		return;
	}

	if(IS_IMMORTAL(victim))
	{
		send_to_char("Immortals don't need their age slowed down.\n\r",ch);
		return;
	}

	/* We are clear. */
	if( !IS_SET(victim->comm, COMM_SLOWAGE) )
	{
		SET_BIT(victim->comm, COMM_SLOWAGE);
		sprintf(buf,"You have slowed down %s's aging process by half.\n\r",victim->name);
		send_to_char(buf,ch);
		send_to_char("You suddenly feel more youthful! You begin to age much more slowly than normal!\n\r",victim);
		sprintf(buf,"%s slowed %s's aging process!",ch->name, victim->name);
		wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
		return;
	}
	else
	{
		REMOVE_BIT(victim->comm, COMM_SLOWAGE);
		sprintf(buf, "You have restored %s's aging process back to normal. Warning, this can throw a victim into instant Age Death by doing this!\n\r",victim->name);
		send_to_char(buf,ch);
		send_to_char("You suddenly feel old and senile. Your near Age Death!\n\r",victim);
		sprintf(buf,"%s restores %s's aging process back to normal!",ch->name, victim->name);
		wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
		return;
	}
	bug("Uh, something's not right in do_slowage.",0);
	return;
}

void do_maskip( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim;
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  int addr1, addr2, addr3, addr4;

  one_argument(argument, arg);

  addr1 = (number_range(1,80));
  addr2 = (number_range(40,200));
  addr3 = (number_range(1,180));
  addr4 = (number_range(1,160));

  if(arg[0] == '\0')
  {
		send_to_char("Mask who's IP?\n\r",ch);
		return;
  }

  if( (victim = get_char_world(ch, arg)) == NULL)
  {
		send_to_char("No such player online.\n\r",ch);
		return;
  }

  if( victim->level > get_trust(ch) )
  {
		send_to_char("They wouldn't like that.\n\r",ch);
		return;
  }

  if(IS_IMMORTAL(victim))
  {
		send_to_char("Immortals should never have masked IP's.\n\r",ch);
		return;
  }

  /* We are clear. */
  if( !IS_SET(victim->comm, COMM_MASKEDIP) )
  {
    SET_BIT(victim->comm, COMM_MASKEDIP);
    sprintf(buf,"You have masked %s's IP address.\n\r",victim->name);
    send_to_char(buf,ch);
    sprintf(buf,"%s masked %s's IP!",ch->name, victim->name);
    wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    sprintf( buf, "%d.%d.%d.%d",
	       addr1,
             addr2,
	       addr3,
             addr4);
     victim->maskedip = str_dup (buf);
     return;
    }
    else
    {
	REMOVE_BIT(victim->comm, COMM_MASKEDIP);
	sprintf(buf, "You have restored %s's IP address back to normal.\n\r",victim->name);
	send_to_char(buf,ch);
	sprintf(buf,"%s restores %s's IP address back to normal!",ch->name, victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
      victim->maskedip = NULL;
      save_char_obj( victim );
	return;
    }
    bug("Uh, something's not right in do_maskip.",0);
    return;
}

void do_autodig(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
 
    if (IS_SET(ch->act2,PLR_AUTODIG))
    {
      send_to_char("Autodig is now disabled.\n\r",ch);
      REMOVE_BIT(ch->act2,PLR_AUTODIG);
    }
    else
    {
      send_to_char("Autodig is now enabled.\n\r",ch);
      SET_BIT(ch->act2,PLR_AUTODIG);
    }
}

void do_salign( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[100];
    CHAR_DATA *victim;
    int value;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) )
    {
	send_to_char( "Syntax: salign <char> <alignment>.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
      send_to_char( "That player is not here.\n\r", ch);
      return;
    }

    if ( IS_NPC ( victim ) )
    {
      send_to_char( "Not on NPCs.\n\r", ch);
      return;
    }

    value = atoi( arg2 );

    if ( value < -1000 || value > 1000 )
    {
      send_to_char( "Alignment range is -1000 to 1000.\n\r", ch );
      return;
    }

    sprintf(buf,"Alignment on %s has been changed from %d to %d!\n\r", victim->name, victim->alignment, value );
    send_to_char(buf, ch);
    victim->alignment = value;
    return;
}

void do_backcheck (CHAR_DATA * ch, char *argument)
{

  char arg1[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  char buf[MAX_INPUT_LENGTH];
  argument = one_argument (argument, arg1);

  if (arg1[0] == '\0')
  {
      send_to_char ("Who's background would you like to check?\n\r", ch);
      return;
  }

  if ((victim = get_char_world (ch, arg1)) == NULL)
  {
      send_to_char ("They aren't here.\n\r", ch);
      return;
  }

  if (IS_NPC(victim))
  {
    send_to_char("Not on mobiles.\n\r",ch);
    return;
  }

  if(!str_cmp(victim->name, "Hiddukel"))
  {
    send_to_char("That would be a bad idea.\n\r", ch );
    return;
  }

  if (victim->background == NULL)
  {
    send_to_char ("They currently do not have a background set.\n\r", ch);
    return;
  }
  else
  {
    sprintf(buf,"\n\r                ++ {R%s's Background ++{x\n\r", victim->name);
    send_to_char(buf, ch);
    sprintf(buf,"{y____________________________________________________________________{x\n\r");
    send_to_char(buf, ch);
    send_to_char( "\n\r", ch );
    send_to_char( victim->background, ch );
  }
  return;
}


