/*
From: "Judson E. Knott" <jek@conga.oit.unc.edu>

Beep, and Balance will work without any modification execept to interp.c 
(command table) and merc.h (DECLARE_DO_FUN).

Deposit, Withdraw will work with mods to the above to files, and the 
addition of #define ROOM_BANK, and #define ACT_BANKER in merc.h ...Also, 
you will need to change an area file to have your first bank.

Wedge and Pry, will take the most mods to files, they are skills.  find 
your open anyother commands that you do not want to work when a door is 
wedged shut.  Also, #define EX_WEDGE needs added, and #define 
EX_WEDGPROOF needs added.

Just give me some credit for what I wrote, maybe just 1/2 of a line.

jek@conga.oit.unc.edu
-----=============================-----
*/

#if defined( macintosh )
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
/* #include "devel.h" */
#include "merc.h"
#include "interp.h"

const     int     rev_dir1  [] =
{
    2, 3, 0, 1, 5, 4
};

void do_beep ( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];

    if (IS_NPC(ch))
	return;

    argument = one_argument( argument, arg );

    if  ( arg[0] == '\0' )
    {
	send_to_char( "Beep who?\n\r", ch );
	return;
    }

    if ( !( victim = get_char_world( ch, arg ) ) )
    {
	send_to_char( "They are not here.\n\r", ch );
	return;
    }


    if ( IS_NPC(victim))
    {
	send_to_char( "They are not beepable.\n\r", ch );
	return;
    }

    sprintf( buf, "\aYou beep %s.\n\r", victim->name );
    send_to_char( buf, ch );

    sprintf( buf, "\a%s has beeped you.\n\r", ch->name );
    send_to_char( buf, victim );
    return;
}

void do_wedge( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int door;
    EXIT_DATA *pexit; 
    EXIT_DATA *pexit_rev;
    ROOM_INDEX_DATA *to_room;

    one_argument( argument, arg );

    if ( ch->level < skill_table[gsn_wedge].skill_level[ch->class] )
    {
	sprintf( buf, "Your inexperience has hindered your use of wedge.\n\r" );
	send_to_char( buf, ch );
	return;
    }

    if ( arg[0] == '\0' )
    {
	sprintf( buf, "But you cannot wedge nothing.\n\r" ); 
	send_to_char( buf, ch );
	return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	pexit = ch->in_room->exit[door];

	if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
	{
	    sprintf( buf, "But it is not closed.\n\r" );
	    send_to_char( buf, ch );
	    return;
	}

	if ( IS_SET(pexit->exit_info, EX_BASHED) )
	{
	    sprintf( buf, "The $d has been bashed and cannot be wedged closed." );
	    act( buf, ch, NULL, pexit->keyword, TO_CHAR );
	    return;
	}

	if ( IS_SET(pexit->exit_info, EX_WEDGPROOF) )
	{
	    sprintf( buf, "This door is unable to be wedged shut.\n\r" );
	    send_to_char( buf, ch );
	    return;
	}

	if ( IS_SET(pexit->exit_info, EX_WEDGE) )
	{
	    sprintf( buf, "This door is already wedged shut.\n\r" );
	    send_to_char( buf, ch );
	    return;
	}

        if ( number_percent( ) > ch->pcdata->learned[gsn_wedge] )
        {
	    sprintf( buf, "You failed.\n\r" );
	    send_to_char( buf, ch );
	    return;
    	}

	SET_BIT( pexit->exit_info, EX_WEDGE );
	act( "$n wedges the $d closed.", ch, NULL, pexit->keyword, TO_ROOM );
	sprintf( buf, "Ok.\n\r" );
	send_to_char( buf, ch );

	if ( ( to_room = pexit->u1.to_room )
	&& ( pexit_rev = to_room->exit[rev_dir1[door]] )
	&& pexit_rev->u1.to_room == ch->in_room )
	{
	    CHAR_DATA *rch;

	    SET_BIT( pexit_rev->exit_info, EX_WEDGE );
	    for ( rch = to_room->people; rch; rch = rch->next_in_room )
	    {
		/*if ( rch->deleted )
		    continue;*/
		act( "The $d is rammed shut from the other side.", rch, NULL, pexit_rev->keyword, TO_CHAR );
	    } 
	}
    } 
    return;
}

void do_pry( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int door;
    EXIT_DATA *pexit; 
    EXIT_DATA *pexit_rev;
    ROOM_INDEX_DATA *to_room;

    one_argument( argument, arg );

    if ( ch->level < skill_table[gsn_pry].skill_level[ch->class] )
    {
	 sprintf( buf, "Your inexperience has hindered your use of pry.\n\r" );
	 send_to_char( buf, ch );
	 return;
    }

    if ( arg[0] == '\0' )
    {
	 sprintf( buf, "But you cannot pry nothing open.\n\r" ); 
	 send_to_char( buf, ch );
	 return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	pexit = ch->in_room->exit[door];

	if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
	{
	    sprintf( buf, "But it is not closed.\n\r" );
	    send_to_char( buf, ch );
	    return;
	}

	if ( !IS_SET(pexit->exit_info, EX_WEDGE) )
	{
	    sprintf( buf, "This door is not wedged shut.\n\r" );
	    send_to_char( buf, ch );
	    return;
	}

	if ( IS_SET(pexit->exit_info, EX_BASHED) )
	{
	    sprintf( buf, "The $d has been bashed and cannot be pried open." );
	    act( buf, ch, NULL, pexit->keyword, TO_CHAR );
	    return;
	}

	if ( IS_SET(pexit->exit_info, EX_WEDGPROOF) )
	{
	    sprintf( buf, "This door is unable to be wedged shut.\n\r" );
	    send_to_char( buf, ch );
	    return;
	}

        if ( number_percent( ) > ch->pcdata->learned[gsn_pry] )
        {
	    sprintf( buf, "You failed.\n\r" );
	    send_to_char( buf, ch );
	    return;
    	}

	REMOVE_BIT( pexit->exit_info, EX_WEDGE );
	act( "$n pries the $d open.", ch, NULL, pexit->keyword, TO_ROOM );
	sprintf( buf, "Ok.\n\r" );
	send_to_char( buf, ch );

	if ( ( to_room = pexit->u1.to_room )
	&& ( pexit_rev = to_room->exit[rev_dir1[door]] )
	&& pexit_rev->u1.to_room == ch->in_room )
	{
	    CHAR_DATA *rch;

	    REMOVE_BIT( pexit_rev->exit_info, EX_WEDGE );
	    for ( rch = to_room->people; rch; rch = rch->next_in_room )
	    {
		/*if ( rch->deleted )
		    continue;*/
		act( "The $d is pried open from the other side.", rch, NULL, pexit_rev->keyword, TO_CHAR );
	    } 
	}
    } 
    return;
}

void do_balance ( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if (IS_NPC(ch))
	return;

    sprintf( buf, "You have %ld coins in the bank.\n\r", ch->bank );
    send_to_char( buf, ch );
    return;
}

void do_deposit ( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *banker;
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int amnt;

    if (IS_NPC(ch))
	return;

    if (!IS_SET(ch->in_room->room_flags, ROOM_BANK) ) 
    {
	sprintf( buf, "But you are not in a bank.\n\r" );
	send_to_char( buf, ch );
	return;
    }

    banker = NULL;
    for ( banker = ch->in_room->people; banker; banker = banker->next_in_room )
    {
	if ( IS_NPC( banker ) && IS_SET(banker->pIndexData->act, ACT_BANKER) )
	    break;
    }

    if ( !banker )
    {
	sprintf( buf, "The banker is currently not available.\n\r" );
	send_to_char( buf, ch );
	return;
    }
 
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	sprintf( buf, "How much steel do you wish to deposit?\n\r" );
	send_to_char( buf, ch );
	return;
    }

    amnt = atoi( arg );
    
    if ( amnt >= (ch->steel + 1) )
    {
	sprintf( buf, "%s, you do not have %d steel coins.", ch->name, amnt );
	do_say( banker, buf );
	return;
    }

    ch->bank += amnt;
    ch->steel -= amnt;
    sprintf( buf, "%s, your account now contains: %ld coins,", ch->name, ch->bank );
    do_say( banker, buf );
    sprintf( buf, "after depositing: %d coins.", amnt );
    do_say( banker, buf );
    return;
}

void do_withdraw ( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *banker;
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int amnt;

    if (IS_NPC(ch))
	return;

    if (!IS_SET(ch->in_room->room_flags, ROOM_BANK) ) 
    {
	sprintf( buf, "But you are not in a bank.\n\r" );
	send_to_char( buf, ch );
	return;
    }

    banker = NULL;
    for ( banker = ch->in_room->people; banker; banker = banker->next_in_room )
    {
	if ( IS_NPC( banker ) && IS_SET(banker->pIndexData->act, ACT_BANKER) )
	    break;
    }

    if ( !banker )
    {
	 sprintf( buf, "The banker is currently not available.\n\r" );
	 send_to_char( buf, ch );
	 return;
    }
 
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	 sprintf( buf, "How much steel do you wish to withdraw?\n\r" );
	 send_to_char( buf, ch );
	 return;
    }

    amnt = atoi( arg );
    
    if ( amnt >= (ch->bank + 1) )
    {
	sprintf( buf, "%s, you do not have %d steel coins in the bank.", ch->name, amnt );
	do_say( banker, buf );
	return;
    }

    ch->steel += amnt;
    ch->bank -= amnt;
    sprintf( buf, "%s, your account now contains: %ld coins,", ch->name, ch->bank );
    do_say( banker, buf );
    sprintf( buf, "after withdrawing: %d coins.", amnt );
    do_say( banker, buf );
    return;
}


/*
 =============================================================================
/   ______ _______ ____   _____   ___ __    _ ______    ____  ____   _____   /
\  |  ____|__   __|  _ \ / ____\ / _ \| \  / |  ____|  / __ \|  _ \ / ____\  \
/  | |__     | |  | |_| | |     | |_| | |\/| | |___   | |  | | |_| | |       /
/  | ___|    | |  | ___/| |   __|  _  | |  | | ____|  | |  | |  __/| |   ___ \
\  | |       | |  | |   | |___| | | | | |  | | |____  | |__| | |\ \| |___| | /
/  |_|       |_|  |_|  o \_____/|_| |_|_|  |_|______|o \____/|_| \_|\_____/  \
\                                                                            /
 ============================================================================

------------------------------------------------------------------------------
ftp://ftp.game.org/pub/mud      FTP.GAME.ORG      http://www.game.org/ftpsite/
------------------------------------------------------------------------------

  This file came from FTP.GAME.ORG, the ultimate source for MUD resources.

------------------------------------------------------------------------------

*/
