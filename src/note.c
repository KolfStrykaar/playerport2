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
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "merc.h"
#include "recycle.h"
#include "tables.h"

#define PLINE   74              /* max line length for "pretty" lines */

/* globals from db.c for load_notes */
#if !defined(macintosh)
extern  int     _filbuf         args( (FILE *) );
#endif
extern FILE *                  fpArea;
extern char                    strArea[MAX_INPUT_LENGTH];

/* local procedures */
void load_thread(char *name, NOTE_DATA **list, int type, time_t free_time);
void parse_note(CHAR_DATA *ch, char *argument, int type);
bool hide_note(CHAR_DATA *ch, NOTE_DATA *pnote);
void desc_format     	args( ( CHAR_DATA *ch, int start, int lines ) );
void back_format     	args( ( CHAR_DATA *ch, int start, int lines ) );
void song_attach     	args( ( CHAR_DATA *ch ) );
void song_remove     	args( ( CHAR_DATA *ch, SONG_DATA *psong ) );
void song_expunge   	args( ( SONG_DATA *psong ) );
void song_show       	args( ( CHAR_DATA *ch, int start, int lines ) );
void song_delete     	args( ( CHAR_DATA *ch, int start, int lines ) );
void song_insert     	args( ( CHAR_DATA *ch, int start, char *text ) );
void song_pretty     	args( ( CHAR_DATA *ch, int start, int lines ) );
void pretty_proc     	args( ( char *buf, char *word ) );
void sing_channel    	args( ( CHAR_DATA *ch, char *argument ) );
void write_songs     	args( ( void ) );
void pretty_singing  	args( ( char * buf ) );

NOTE_DATA *note_list;
NOTE_DATA *idea_list;
NOTE_DATA *penalty_list;
NOTE_DATA *news_list;
NOTE_DATA *changes_list;

SONG_DATA * song_list;
SONG_DATA * song_free;

SINGING_DATA * singing_list;
SINGING_DATA * singing_free;


int count_spool(CHAR_DATA *ch, NOTE_DATA *spool)
{
    int count = 0;
    NOTE_DATA *pnote;

    for (pnote = spool; pnote != NULL; pnote = pnote->next)
	if (!hide_note(ch,pnote))
	    count++;

    return count;
}

int forward_spool(CHAR_DATA *ch, NOTE_DATA *spoolfrom, NOTE_DATA *spoolto)
{
    char buf[MAX_STRING_LENGTH];
    int count = 0;
    NOTE_DATA *pNoteFrom, *pNoteTo, *spoolto_last;

    for (pNoteFrom = spoolfrom; pNoteFrom != NULL; 
                                 pNoteFrom = pNoteFrom->next)
        if ( is_name( ch->name, pNoteFrom->to_list ) && 
                                                 !hide_note(ch,pNoteFrom) )
	{
	    
	    
	    for ( spoolto_last = spoolto; spoolto_last->next ; )	    
	    {
	        spoolto_last = spoolto_last->next;
	    }

	    spoolto_last->next = pNoteTo = new_note();
	    pNoteTo->next	= NULL;
	    pNoteTo->sender	= str_dup( pNoteFrom->sender );
	    pNoteTo->date	= str_dup( pNoteFrom->date );
	    pNoteTo->date_stamp = current_time;
	    pNoteTo->to_list	= str_dup( ch->name );
	    pNoteTo->subject	= str_dup( pNoteFrom->subject );
	    pNoteTo->text	= str_dup( pNoteFrom->text );
	    pNoteTo->type	= spoolto->type;
	    count++;
	}
       return count;
    sprintf(buf,"%d %s been added.\n\r",
    count, count > 1 ? "penalties have" : "penalty has");
    send_to_char(buf,ch);
}

void do_unread(CHAR_DATA *ch)
{
    char buf[MAX_STRING_LENGTH];

    if (IS_NPC(ch))
	return; 

    send_to_char("\n\r",ch);
    send_to_char("{WNum         Name Unread Description{x\n\r",ch);
    send_to_char("{w==== =========== ====== ==========={x\n\r",ch);
    sprintf( buf,"[ 1]       {CNotes {w[{R%4d{w] {WGeneral Discussion{x\n\r",
    count_spool(ch,note_list));
    send_to_char( buf, ch );
    sprintf( buf,"[ 2]     {cChanges {w[{R%4d{w] {WChanges to the Game{x\n\r",
    count_spool(ch,changes_list));
    send_to_char( buf, ch );
    sprintf( buf,"[ 3]       {CIdeas {w[{R%4d{w] {WIdeas to the Staff{x\n\r",
    count_spool(ch,idea_list));
    send_to_char( buf, ch );
    sprintf( buf,"[ 4]     {cStories {w[{R%4d{w] {WRoleplay stories{x\n\r",
    count_spool(ch,news_list));
    send_to_char( buf, ch );
    if (IS_TRUSTED(ch,ANGEL))
    {
    sprintf( buf,"[ 5]   {CPenalties {w[{R%4d{w] {WImposed Penalties{x\n\r",
    count_spool(ch,penalty_list));
    send_to_char( buf, ch );
    }
    send_to_char("\n\r",ch);

}

void do_note(CHAR_DATA *ch,char *argument)
{
    parse_note(ch,argument,NOTE_NOTE);
}

void do_idea(CHAR_DATA *ch,char *argument)
{
    parse_note(ch,argument,NOTE_IDEA);
}

void do_penalty(CHAR_DATA *ch,char *argument)
{
    parse_note(ch,argument,NOTE_PENALTY);
}

void do_news(CHAR_DATA *ch,char *argument)
{
    parse_note(ch,argument,NOTE_NEWS);
}

void do_changes(CHAR_DATA *ch,char *argument)
{
    parse_note(ch,argument,NOTE_CHANGES);
}

void save_notes(int type)
{
    FILE *fp;
    char *name;
    NOTE_DATA *pnote;

    switch (type)
    {
	default:
	    return;
	case NOTE_NOTE:
	    name = NOTE_FILE;
	    pnote = note_list;
	    break;
	case NOTE_IDEA:
	    name = IDEA_FILE;
	    pnote = idea_list;
	    break;
	case NOTE_PENALTY:
	    name = PENALTY_FILE;
	    pnote = penalty_list;
	    break;
	case NOTE_NEWS:
	    name = NEWS_FILE;
	    pnote = news_list;
	    break;
	case NOTE_CHANGES:
	    name = CHANGES_FILE;
	    pnote = changes_list;
	    break;
    }

    fclose( fpReserve );
    if ( ( fp = fopen( name, "w" ) ) == NULL )
    {
	perror( name );
    }
    else
    {
	for ( ; pnote != NULL; pnote = pnote->next )
	{
	    fprintf( fp, "Sender  %s~\n", pnote->sender);
	    fprintf( fp, "Date    %s~\n", pnote->date);
	    fprintf( fp, "Stamp   %ld\n", pnote->date_stamp);
	    fprintf( fp, "To      %s~\n", pnote->to_list);
	    fprintf( fp, "Subject %s~\n", pnote->subject);
	    fprintf( fp, "Text\n%s~\n",   pnote->text);
	}
	fclose( fp );
	fpReserve = fopen( NULL_FILE, "r" );
   	return;
    }
}
void load_notes(void)
{
    load_thread(NOTE_FILE,&note_list, NOTE_NOTE, 0); //C038
    load_thread(IDEA_FILE,&idea_list, NOTE_IDEA, 0); //C038
    load_thread(PENALTY_FILE,&penalty_list, NOTE_PENALTY, 0);
    load_thread(NEWS_FILE,&news_list, NOTE_NEWS, 0);
    load_thread(CHANGES_FILE,&changes_list,NOTE_CHANGES, 0);
}

void load_thread(char *name, NOTE_DATA **list, int type, time_t free_time)
{
    FILE *fp;
    NOTE_DATA *pnotelast;
 
    if ( ( fp = fopen( name, "r" ) ) == NULL )
	return;
	 
    pnotelast = NULL;
    for ( ; ; )
    {
	NOTE_DATA *pnote;
	char letter;
	 
	do
	{
	    letter = getc( fp );
            if ( feof(fp) )
            {
                fclose( fp );
                return;
            }
        }
        while ( isspace(letter) );
        ungetc( letter, fp );
 
        pnote           = alloc_perm( sizeof(*pnote) );
 
        if ( str_cmp( fread_word( fp ), "sender" ) )
            break;
        pnote->sender   = fread_string( fp );
 
        if ( str_cmp( fread_word( fp ), "date" ) )
            break;
        pnote->date     = fread_string( fp );
 
        if ( str_cmp( fread_word( fp ), "stamp" ) )
            break;
        pnote->date_stamp = fread_number(fp);
 
        if ( str_cmp( fread_word( fp ), "to" ) )
            break;
        pnote->to_list  = fread_string( fp );
 
        if ( str_cmp( fread_word( fp ), "subject" ) )
            break;
        pnote->subject  = fread_string( fp );
 
        if ( str_cmp( fread_word( fp ), "text" ) )
            break;
        pnote->text     = fread_string( fp );
 
        if (free_time && pnote->date_stamp < current_time - free_time)
        {
	    free_note(pnote);
            continue;
        }

	pnote->type = type;
 
        if (*list == NULL)
            *list           = pnote;
        else
            pnotelast->next     = pnote;
 
        pnotelast       = pnote;
    }
 
    strcpy( strArea, NOTE_FILE );
    fpArea = fp;
    bug( "Load_notes: bad key word.", 0 );
    exit( 1 );
    return;
}

void append_note(NOTE_DATA *pnote)
{
    FILE *fp;
    char *name;
    NOTE_DATA **list;
    NOTE_DATA *last;

    switch(pnote->type)
    {
	default:
	    return;
	case NOTE_NOTE:
	    name = NOTE_FILE;
	    list = &note_list;
	    break;
	case NOTE_IDEA:
	    name = IDEA_FILE;
	    list = &idea_list;
	    break;
	case NOTE_PENALTY:
	    name = PENALTY_FILE;
	    list = &penalty_list;
	    break;
	case NOTE_NEWS:
	     name = NEWS_FILE;
	     list = &news_list;
	     break;
	case NOTE_CHANGES:
	     name = CHANGES_FILE;
	     list = &changes_list;
	     break;
    }

    if (*list == NULL)
	*list = pnote;
    else
    {
	for ( last = *list; last->next != NULL; last = last->next);
	last->next = pnote;
    }

    fclose(fpReserve);
    if ( ( fp = fopen(name, "a" ) ) == NULL )
    {
        perror(name);
    }
    else
    {
        fprintf( fp, "Sender  %s~\n", pnote->sender);
        fprintf( fp, "Date    %s~\n", pnote->date);
        fprintf( fp, "Stamp   %ld\n", pnote->date_stamp);
        fprintf( fp, "To      %s~\n", pnote->to_list);
        fprintf( fp, "Subject %s~\n", pnote->subject);
        fprintf( fp, "Text\n%s~\n", pnote->text);
        fclose( fp );
    }
    fpReserve = fopen( NULL_FILE, "r" );
}

bool is_note_to( CHAR_DATA *ch, NOTE_DATA *pnote )
{

    if ( !str_cmp( ch->name, pnote->sender ) )
	return TRUE;

    if ( is_exact_name( "all", pnote->to_list ))
	return TRUE;

    if ( IS_IMMORTAL(ch) && is_name( "imm", pnote->to_list ))
	return TRUE;

    if ( IS_IMMORTAL(ch) && is_name( "imms", pnote->to_list ) )
	return TRUE;

    if ( IS_IMMORTAL(ch) && is_name( "immort", pnote->to_list ) )
	return TRUE;

    if ( IS_IMMORTAL(ch) && is_name( "immortal", pnote->to_list ) )
	return TRUE;

    if ( IS_IMMORTAL(ch) && is_name( "immortals", pnote->to_list ) )
	return TRUE;

    if (ch->org_id && is_name(org_table[ORG( ch->org_id )]. name,pnote->to_list))
	return TRUE;

    if ( IS_IMPLEMENTOR(ch) && is_name ( "imp", pnote->to_list ) )
        return TRUE;

    if ( IS_IMPLEMENTOR(ch) && is_name ( "imps", pnote->to_list ) )
        return TRUE;

    if ( IS_IMPLEMENTOR(ch) && is_name ( "implementor", pnote->to_list ) )
        return TRUE;

    if ( IS_IMPLEMENTOR(ch) && is_name ( "implementors", pnote->to_list ) )
	return TRUE;

    if ( IS_HERO(ch) && is_name( "hero", pnote->to_list ) )
	return TRUE;

    if (is_exact_name( ch->name, pnote->to_list ) )
	return TRUE;

    if ( ch->class == class_lookup("warrior")
    && is_name( "warrior", pnote->to_list ))
	return TRUE;

    if ( ch->class == class_lookup("thief")
    && is_name( "thief", pnote->to_list ))
	return TRUE;

    if ( ch->class == class_lookup("knight")
    && is_name( "knight", pnote->to_list ))
	return TRUE;

    if ( ch->class == class_lookup("battlemage")
    && is_name( "battlemage", pnote->to_list ))
	return TRUE;

    if ( ch->class == class_lookup("necromancer")
    && is_name( "necromancer", pnote->to_list ))
	return TRUE;

    if ( ch->class == class_lookup("illusionist")
    && is_name( "illusionist", pnote->to_list ))
	return TRUE;

    if ( ch->class == class_lookup("enchanter")
    && is_name( "enchanter", pnote->to_list ))
	return TRUE;

    if ( ch->class == class_lookup("conjuror")
    && is_name( "conjuror", pnote->to_list ))
	return TRUE;

    if ( ch->class == class_lookup("cleric")
    && is_name( "cleric", pnote->to_list ))
	return TRUE;

    if ( ch->class == class_lookup("assassin")
    && is_name( "assassin", pnote->to_list ))
	return TRUE;

    if ( ch->class == class_lookup("barbarian")
    && is_name( "barbarian", pnote->to_list ))
	return TRUE;

    if ( ch->class == class_lookup("bard")
    && is_name( "bard", pnote->to_list ))
	return TRUE;

    if ( ch->class == class_lookup("spellfilcher")
    && is_name( "spellfilcher", pnote->to_list ))
	return TRUE;

    if ( ch->class == class_lookup("druid")
    && is_name( "druid", pnote->to_list ))
	return TRUE;

    if ( ch->class == class_lookup("ranger")
    && is_name( "ranger", pnote->to_list ))
	return TRUE;

    if ( ch->class == class_lookup("blacksmith")
    && is_name( "blacksmith", pnote->to_list ))
	return TRUE;

    return FALSE;
}



void note_attach( CHAR_DATA *ch, int type )
{
    NOTE_DATA *pnote;

    if ( ch->pnote != NULL )
	return;

    pnote = new_note();

    pnote->next		= NULL;
    pnote->sender	= str_dup( ch->name );
    pnote->date		= str_dup( "" );
    pnote->to_list	= str_dup( "" );
    pnote->subject	= str_dup( "" );
    pnote->text		= str_dup( "" );
    pnote->type		= type;
    ch->pnote		= pnote;
    return;
}



void note_remove( CHAR_DATA *ch, NOTE_DATA *pnote, bool delete)
{
    char to_new[MAX_INPUT_LENGTH];
    char to_one[MAX_INPUT_LENGTH];
    NOTE_DATA *prev;
    NOTE_DATA **list;
    char *to_list;

    if (!delete)
    {
	/* make a new list */
        to_new[0]	= '\0';
        to_list	= pnote->to_list;
        while ( *to_list != '\0' )
        {
    	    to_list	= one_argument( to_list, to_one );
    	    if ( to_one[0] != '\0' && str_cmp( ch->name, to_one ) )
	    {
	        strcat( to_new, " " );
	        strcat( to_new, to_one );
	    }
        }
        /* Just a simple recipient removal? */
       if ( str_cmp( ch->name, pnote->sender ) && to_new[0] != '\0' )
       {
	   free_string( pnote->to_list );
	   pnote->to_list = str_dup( to_new + 1 );
	   return;
       }
    }
    /* nuke the whole note */

    switch(pnote->type)
    {
	default:
	    return;
	case NOTE_NOTE:
	    list = &note_list;
	    break;
	case NOTE_IDEA:
	    list = &idea_list;
	    break;
	case NOTE_PENALTY:
	    list = &penalty_list;
	    break;
	case NOTE_NEWS:
	    list = &news_list;
	    break;
	case NOTE_CHANGES:
	    list = &changes_list;
	    break;
    }

    /*
     * Remove note from linked list.
     */
    if ( pnote == *list )
    {
	*list = pnote->next;
    }
    else
    {
	for ( prev = *list; prev != NULL; prev = prev->next )
	{
	    if ( prev->next == pnote )
		break;
	}

	if ( prev == NULL )
	{
	    bug( "Note_remove: pnote not found.", 0 );
	    return;
	}

	prev->next = pnote->next;
    }

    save_notes(pnote->type);
    free_note(pnote);
    return;
}

bool hide_note (CHAR_DATA *ch, NOTE_DATA *pnote)
{
    time_t last_read;

    if (IS_NPC(ch))
	return TRUE;

    switch (pnote->type)
    {
	default:
	    return TRUE;
	case NOTE_NOTE:
	    last_read = ch->pcdata->last_note;
	    break;
	case NOTE_IDEA:
	    last_read = ch->pcdata->last_idea;
	    break;
	case NOTE_PENALTY:
	    last_read = ch->pcdata->last_penalty;
	    break;
	case NOTE_NEWS:
	    last_read = ch->pcdata->last_news;
	    break;
	case NOTE_CHANGES:
	    last_read = ch->pcdata->last_changes;
	    break;
    }
    
    if (pnote->date_stamp <= last_read)
	return TRUE;

    if (!str_cmp(ch->name,pnote->sender))
	return TRUE;

    if (!is_note_to(ch,pnote))
	return TRUE;

    return FALSE;
}

void update_read(CHAR_DATA *ch, NOTE_DATA *pnote)
{
    time_t stamp;

    if (IS_NPC(ch))
	return;

    stamp = pnote->date_stamp;

    switch (pnote->type)
    {
        default:
            return;
        case NOTE_NOTE:
	    ch->pcdata->last_note = UMAX(ch->pcdata->last_note,stamp);
            break;
        case NOTE_IDEA:
	    ch->pcdata->last_idea = UMAX(ch->pcdata->last_idea,stamp);
            break;
        case NOTE_PENALTY:
	    ch->pcdata->last_penalty = UMAX(ch->pcdata->last_penalty,stamp);
            break;
        case NOTE_NEWS:
	    ch->pcdata->last_news = UMAX(ch->pcdata->last_news,stamp);
            break;
        case NOTE_CHANGES:
	    ch->pcdata->last_changes = UMAX(ch->pcdata->last_changes,stamp);
            break;
    }
}

void parse_note( CHAR_DATA *ch, char *argument, int type )
{
    BUFFER *buffer;
    BUFFER *output;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    NOTE_DATA *pnote;
    NOTE_DATA *prev_note;
    NOTE_DATA **list;
    char *list_name;
    int vnum;
    int anum;

    if ( IS_NPC(ch) )
	return;

    switch(type)
    {
	default:
	    return;
        case NOTE_NOTE:
            list = &note_list;
	    list_name = "notes";
            break;
        case NOTE_IDEA:
            list = &idea_list;
	    list_name = "ideas";
            break;
        case NOTE_PENALTY:
            list = &penalty_list;
	    list_name = "penalties";
            break;
        case NOTE_NEWS:
            list = &news_list;
	    list_name = "stories"; //C065
            break;
        case NOTE_CHANGES:
            list = &changes_list;
	    list_name = "changes";
            break;
    }

    argument = one_argument( argument, arg );
    smash_tilde( argument );

    if ( arg[0] == '\0' || !str_prefix( arg, "read" ) )
    {
        bool fAll;
 
        if ( !str_cmp( argument, "all" ) )
        {
            fAll = TRUE;
            anum = 0;
        }
 
        else if ( argument[0] == '\0' || !str_prefix(argument, "next"))
        /* read next unread note */
        {
            vnum = 0;
            for ( pnote = *list; pnote != NULL; pnote = pnote->next)
            {
                if (!hide_note(ch,pnote))
                {
                    sprintf( buf, "[%3d] %s: %s\n\r%s\n\rTo: %s\n\r",
                        vnum,
                        pnote->sender,
                        pnote->subject,
                        pnote->date,
                        pnote->to_list);
                    send_to_char( buf, ch );
                    page_to_char( pnote->text, ch );
                    update_read(ch,pnote);
                    return;
                }
                else if (is_note_to(ch,pnote))
                    vnum++;
            }
	    sprintf(buf,"You have no unread %s.\n\r",list_name);
	    send_to_char(buf,ch);
            return;
        }
 
        else if ( is_number( argument ) )
        {
            fAll = FALSE;
            anum = atoi( argument );
        }
        else
        {
            send_to_char( "Read which number?\n\r", ch );
            return;
        }
 
        vnum = 0;
        for ( pnote = *list; pnote != NULL; pnote = pnote->next )
        {
            if ( is_note_to( ch, pnote ) && ( vnum++ == anum || fAll ) )
            {
                sprintf( buf, "[%3d] %s: %s\n\r%s\n\rTo: %s\n\r",
                    vnum - 1,
                    pnote->sender,
                    pnote->subject,
                    pnote->date,
                    pnote->to_list
                    );
                send_to_char( buf, ch );
                page_to_char( pnote->text, ch );
		update_read(ch,pnote);
                return;
            }
        }
 
	sprintf(buf,"There aren't that many %s.\n\r",list_name);
	send_to_char(buf,ch);
        return;
    }

    if ( !str_prefix( arg, "list" ) )
    {
	vnum = 0;
      buf[0] = '\0';
      output = new_buf();

      sprintf( buf, "\n\r");
      add_buf(output,buf);
	sprintf (buf,"                    {YDisplaying messages in your spool{x\n\r");
      add_buf(output,buf);
      sprintf( buf, "\n\r");
      add_buf(output,buf);
    	sprintf (buf,"{W- Number -    - Author -       - Subject -{x\n\r");
	add_buf(output,buf);
    	sprintf (buf,"-------------------------------------------------------------------------------\n\r");
      add_buf(output,buf);

	for ( pnote = *list; pnote != NULL; pnote = pnote->next )
	{
	    if ( is_note_to( ch, pnote ) )
	    {
		sprintf( buf, "  [%3d%s]       %-16s- %s\n\r",
		    vnum, hide_note(ch,pnote) ? " " : "N",
	  	    pnote->sender,
		    pnote->subject );
		add_buf(output,buf);
		vnum++;
	    }
	}
      page_to_char( buf_string(output), ch );
	if (!vnum)
	{
	    switch(type)
	    {
		case NOTE_NOTE:	
		    send_to_char("There are no notes for you.\n\r",ch);
		    break;
		case NOTE_IDEA:
		    send_to_char("There are no ideas for you.\n\r",ch);
		    break;
		case NOTE_PENALTY:
		    send_to_char("There are no penalties for you.\n\r",ch);
		    break;
		case NOTE_NEWS:
		    send_to_char("There are no stories for you.\n\r",ch);
		    break;
		case NOTE_CHANGES:
		    send_to_char("There are no changes for you.\n\r",ch);
		    break;
	    }
	}
	return;
    }

    if ( !str_prefix( arg, "remove" ) )
    {
        if ( !is_number( argument ) )
        {
            send_to_char( "Note remove which number?\n\r", ch );
            return;
        }
 
        anum = atoi( argument );
        vnum = 0;
        for ( pnote = *list; pnote != NULL; pnote = pnote->next )
        {
            if ( is_note_to( ch, pnote ) && vnum++ == anum )
            {
                note_remove( ch, pnote, FALSE );
                send_to_char( "Ok.\n\r", ch );
                return;
            }
        }
 
	sprintf(buf,"There aren't that many %s.",list_name);
	send_to_char(buf,ch);
        return;
    }
 
    if ( !str_prefix( arg, "delete" ) && get_trust(ch) >= MAX_LEVEL - 1)
    {
        if ( !is_number( argument ) )
        {
            send_to_char( "Note delete which number?\n\r", ch );
            return;
        }
 
        anum = atoi( argument );
        vnum = 0;
        for ( pnote = *list; pnote != NULL; pnote = pnote->next )
        {
            if ( is_note_to( ch, pnote ) && vnum++ == anum )
            {
                note_remove( ch, pnote,TRUE );
                send_to_char( "Ok.\n\r", ch );
                return;
            }
        }

 	sprintf(buf,"There aren't that many %s.",list_name);
	send_to_char(buf,ch);
        return;
    }

    if (!str_prefix(arg,"catchup"))
    {
	switch(type)
	{
	    case NOTE_NOTE:	
		ch->pcdata->last_note = current_time;
		break;
	    case NOTE_IDEA:
		ch->pcdata->last_idea = current_time;
		break;
	    case NOTE_PENALTY:
		ch->pcdata->last_penalty = current_time;
		break;
	    case NOTE_NEWS:
		ch->pcdata->last_news = current_time;
		break;
	    case NOTE_CHANGES:
		ch->pcdata->last_changes = current_time;
		break;
	}
	return;
    }

    /* below this point only certain people can edit notes */
    if /*((type == NOTE_NEWS && !IS_TRUSTED(ch,ANGEL))
    ||  */(type == NOTE_CHANGES && !IS_TRUSTED(ch,CREATOR))
    {
	sprintf(buf,"You aren't high enough level to write %s.",list_name);
	send_to_char(buf,ch);
	return;
    }

    /*Added for C007 by Sirrion*/
    if ( !str_cmp( arg, "edit" ) )
    {
	SET_BIT(ch->comm,COMM_WRITING);
	note_attach( ch,type );
	string_append( ch, &ch->pnote->text );
	return;
    }
    /*End of edit code*/
    
    if ( !str_cmp( arg, "+" ) )
    {
	note_attach( ch,type );
	if (ch->pnote->type != type)
	{
	    send_to_char(
		"You already have a different note in progress.\n\r",ch);
	    return;
	}

	if (strlen(ch->pnote->text)+strlen(argument) >= 4096)
	{
	    send_to_char( "Note too long.\n\r", ch );
	    return;
	}

 	buffer = new_buf();

	add_buf(buffer,ch->pnote->text);
	add_buf(buffer,argument);
	add_buf(buffer,"\n\r");
	free_string( ch->pnote->text );
	ch->pnote->text = str_dup( buf_string(buffer) );
	free_buf(buffer);
	send_to_char( "Ok.\n\r", ch );
	return;
    }

    if (!str_cmp(arg,"-"))
    {
 	int len;
	bool found = FALSE;

	note_attach(ch,type);
        if (ch->pnote->type != type)
        {
            send_to_char(
                "You already have a different note in progress.\n\r",ch);
            return;
        }

	if (ch->pnote->text == NULL || ch->pnote->text[0] == '\0')
	{
	    send_to_char("No lines left to remove.\n\r",ch);
	    return;
	}

	strcpy(buf,ch->pnote->text);

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
		    free_string(ch->pnote->text);
		    ch->pnote->text = str_dup(buf);
		    return;
		}
	    }
	}
	buf[0] = '\0';
	free_string(ch->pnote->text);
	ch->pnote->text = str_dup(buf);
	return;
    }

    if ( !str_prefix( arg, "subject" ) )
    {
	note_attach( ch,type );
        if (ch->pnote->type != type)
        {
            send_to_char(
                "You already have a different note in progress.\n\r",ch);
            return;
        }

        if ( strlen(argument) > 41 )
        {
            send_to_char("\n\r",ch);
            send_to_char("Your -SUBJECT- is TOO LARGE. Please limit text to 40 characters or less.\n\r",ch);
            return;
        }

	free_string( ch->pnote->subject );
	ch->pnote->subject = str_dup( argument );
	send_to_char( "Ok.\n\r", ch );
	return;
    }

    if ( !str_prefix( arg, "to" ) )
    {
	note_attach( ch,type );
        if (ch->pnote->type != type)
        {
            send_to_char(
                "You already have a different note in progress.\n\r",ch);
            return;
        }

        if ( IS_SET(ch->act, PLR_NONOTE )
	&& !str_cmp(argument, "all" ))
	{
    	send_to_char( "Your note privledges to send to everyone have been revoked.\n\r", ch);
    	send_to_char( "You may only send notes to individuals.\n\r", ch);
    	return;
    	}

        if ( strlen(argument) > 41 )
        {
            send_to_char("\n\r",ch);
            send_to_char("Your -TO- block is TOO LARGE. Please limit text to 40 characters or less.\n\r",ch);
            return;
        }

	free_string( ch->pnote->to_list );
	ch->pnote->to_list = str_dup( argument );
	send_to_char( "Ok.\n\r", ch );
	return;
    }

    if ( !str_prefix( arg, "clear" ) )
    {
	if ( ch->pnote != NULL )
	{
	    free_note(ch->pnote);
	    ch->pnote = NULL;
	}

	send_to_char( "Ok.\n\r", ch );
	return;
    }

    if ( !str_prefix( arg, "show" ) )
    {
	if ( ch->pnote == NULL )
	{
	    send_to_char( "You have no note in progress.\n\r", ch );
	    return;
	}

	if (ch->pnote->type != type)
	{
	    send_to_char("You aren't working on that kind of note.\n\r",ch);
	    return;
	}

	sprintf( buf, "%s: %s\n\rTo: %s\n\r",
	    ch->pnote->sender,
	    ch->pnote->subject,
	    ch->pnote->to_list
	    );
	send_to_char( buf, ch );
	send_to_char( ch->pnote->text, ch );
	return;
    }

    if ( !str_prefix( arg, "post" ) || !str_prefix(arg, "send"))
    {
	char *strtime;

	if ( ch->pnote == NULL )
	{
	    send_to_char( "You have no note in progress.\n\r", ch );
	    return;
	}

        if (ch->pnote->type != type)
        {
            send_to_char("You aren't working on that kind of note.\n\r",ch);
            return;
        }

	if (!str_cmp(ch->pnote->to_list,""))
	{
	    send_to_char(
		"You need to provide a recipient (name, all, or immortal).\n\r",
		ch);
	    return;
	}

	if (!str_cmp(ch->pnote->subject,""))
	{
	    send_to_char("You need to provide a subject.\n\r",ch);
	    return;
	}

	if (!str_cmp(ch->pnote->text,""))
	{
	  send_to_char("You need to provide some kind of message. You Can not leave it Blank.\n\r",ch);
	  send_to_char("Type NOTE EDIT to enter the text editor for your note. That's where you'll\n\r",ch);
	  send_to_char("enter in your message text. Type HELP NOTE if your confused as to how to\n\r",ch);
	  send_to_char("do this or don't know what to do when inside the editor.\n\r",ch);
	  return;
	}

	ch->pnote->next			= NULL;
	strtime				= ctime( &current_time );
	strtime[strlen(strtime)-1]	= '\0';
	ch->pnote->date			= str_dup( strtime );
	ch->pnote->date_stamp		= current_time;

	append_note(ch->pnote);
	ch->pnote = NULL;
	return;
    }

    if ( !str_prefix(arg, "forward") ) //note forward(arg) #(arg2) person(arg3)
    {
	char *strtime;
	argument = one_argument(argument, arg2);
	one_argument(argument, arg3);

	if(arg2[0] == '\0' || !is_number(arg2) || arg3[0] == '\0')
	{
		send_to_char("Syntax: <spool> forward # <person>, ex: note forward 5 paladine\n\r", ch);
		return;
	}
	
	anum = atoi(arg2);
	vnum = 0;
	for(prev_note = *list; prev_note != NULL; prev_note = prev_note->next)
	{
		if( is_note_to(ch, prev_note) && (vnum++ == anum) )
		{
			buffer = new_buf();
			ch->pnote = NULL;
			ch->pnote = new_note();
			ch->pnote->type = type;
			ch->pnote->sender = str_dup(ch->name);
			ch->pnote->to_list = str_dup(arg3);
			ch->pnote->subject = str_dup(prev_note->subject);
			ch->pnote->next = NULL;
			strtime = ctime( &current_time );
			strtime[strlen(strtime)-1] = '\0';
			ch->pnote->date = str_dup( strtime );
			ch->pnote->date_stamp = current_time;
			add_buf(buffer, "---Forwarded Message---\n\r");
                  sprintf( buf, "%s: %s\n\r%s\n\rTo: %s\n\r",
                      prev_note->sender, prev_note->subject,
                      prev_note->date, prev_note->to_list);
			add_buf(buffer, buf);
			add_buf(buffer, prev_note->text);
			add_buf(buffer, "\n\r");
			ch->pnote->text = str_dup( buf_string(buffer) );
			free_buf(buffer);
			append_note(ch->pnote);
			ch->pnote = NULL;
			send_to_char("Note has been forwarded.\n\r",ch);
			return;
		}
	}
	sprintf(buf, "There aren't that many %s.\n\r",list_name);
	send_to_char(buf, ch);
	return;
    }

    send_to_char( "You can't do that.\n\r", ch );
    return;
}

void do_devote( CHAR_DATA *ch, char *argument, int type )
{
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    char *strtime;
    int religion;
    FILE *fp;
    OBJ_DATA *obj;
    NOTE_DATA *pnote;

    argument = one_argument(argument, arg);

    if ( IS_NPC(ch))
    {
	send_to_char("You must be a regular player to devote yourself.\n\r",ch);
	return;
    }

    if ( IS_IMMORTAL(ch))
    {
      send_to_char( "You have no need to devote yourself to a god.\n\r",ch);
      return;
    }

    if ( IS_SET(ch->comm, COMM_NOPRAY) )
    {
      send_to_char( "The gods refuse to hear your prayers.\n\r", ch );
      return;
    }

    if (!IS_SET(ch->in_room->room_flags, ROOM_TEMPLE ) )
    {
      send_to_char(
           "You try to devote, but find that you are not inside of a temple of devotion.\n\r", ch);  
      return;
    }
    if ( arg[0] == '\0' )
    {
      send_to_char( "What deity do you wish to devote yourself to?\n\r", ch );
      return;
    }

    if( ch->religion_id != RELIGION_NONE )
    {
      send_to_char( "Trying to change Gods already? Well, You must be stripped of your mark\n\rbefore you can devote yourself to any other God.\n\r", ch );
      return;
    }

    for ( religion = 1; ((religion_table[religion].godname != NULL) 
    && (religion !=  MAX_RELIGION)) ; religion++ )
    {
      if ( (!str_cmp (arg, religion_table[religion].godname))
      && (ch->in_room->sector_type == religion_table[religion].sector_type) )
      {

        if (ch->devotee_id == religion)
	{
	  send_to_char("You cannot re-devote yourself to that God. Once is enough.\n\r",ch);
	  return;
	}

	sprintf( buf, "You offer your life to %s, praying for divine favor.\n\r",
        religion_table[religion].godname);
    	send_to_char(buf,ch);

        if( ch->religion_id == RELIGION_NONE )
        {
          if ( ( obj = get_eq_char( ch, WEAR_MARK ) ) != NULL )
            extract_obj ( obj );
            obj = create_object( get_obj_index(religion_table[religion].devoteevnum), 0 );
            obj->cost = 0;
            obj_to_char( obj, ch);
            equip_char( ch, obj, WEAR_MARK);                
            ch->devotee_id = religion;
        } 

    	  for ( d = descriptor_list; d != NULL; d = d->next )
    	  {
          CHAR_DATA *victim;
	    char buf1[100];
          victim = d->original ? d->original : d->character;
          if ( d->connected == CON_PLAYING
          && IS_IMMORTAL(d->character) ) 
        	{
	    	  sprintf(buf1,"%s is devoting their life to %s.\n\r",
                   ch->name,religion_table[religion].godname);  
	    	  send_to_char(buf1, d->character);
        	}
    	  }

        note_attach ( ch, type );
        free_string( ch->pnote->to_list );
        ch->pnote->to_list = str_dup( arg );

        note_attach ( ch, type );
        sprintf ( buf, "DEVOTION: %s", ch->name);
        free_string( ch->pnote->subject );
        ch->pnote->subject = str_dup( buf );   
 
        note_attach ( ch, type );
    	sprintf ( buf, "%s has devoted to you, in your temple.\n\r",ch->name);
    	ch->pnote->text 			= str_dup ( buf ) ;
    	ch->pnote->next                   = NULL;
    	strtime                           = ctime( &current_time );
    	strtime[strlen(strtime)-1]  	  = '\0';
    	ch->pnote->date                   = str_dup( strtime );
    	ch->pnote->date_stamp             = current_time;
        if ( note_list == NULL )
        {
          note_list       = ch->pnote;
        }
        else
        {
         for ( pnote = note_list; pnote->next != NULL; pnote = pnote->next);
         pnote->next     = ch->pnote;
        }
        pnote               = ch->pnote;
        ch->pnote           = NULL;
        fclose( fpReserve );
        if ( ( fp = fopen( NOTE_FILE, "a" ) ) == NULL )
        {
          perror( NOTE_FILE );
        }
        else
        {
            fprintf( fp, "Sender  %s~\n",       pnote->sender);
            fprintf( fp, "Date    %s~\n",  pnote->date);
            fprintf( fp, "Stamp   %ld\n",  pnote->date_stamp);
            fprintf( fp, "To      %s~\n",  pnote->to_list);
            fprintf( fp, "Subject %s~\n",  pnote->subject);
            fprintf( fp, "Text\n%s~\n",  pnote->text);
            fclose( fp );
        }
        pnote->type = NOTE_NOTE;
        fpReserve = fopen( NULL_FILE, "r" );
        return;
     }
  }
  /* if an inappropriate name of a god is used, it defaults to here */
  send_to_char("You cannot devote yourself to that god here.\n\r",ch);
  return;
}

/*
 * do_mark
 *
 * Bring someone into a religion by marking them.  Available for
 * immortal heads of religions and also the head god and high priest(ess)
 * credentials.
 *
 * Usage
 *   mark [immortal] [religion]  (head god only)
 *   mark [mortal]
 */
void do_mark( CHAR_DATA *ch, char *argument, int type )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    char buf[100];
    CHAR_DATA *gch;
    OBJ_DATA *obj;
    DESCRIPTOR_DATA *d;
    FILE *fp;
    NOTE_DATA *pnote;
    char *strtime;
    int religion = 0;
    int leader = 0;
    int head_god = FALSE; 
    int marktype;
    char *religionname;
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );
    /* check qualifications. must be leader or head god */
    head_god = !str_cmp(ch->name, HEAD_GOD) || !str_cmp(ch->name,HEAD_GOD2)
               || !str_cmp(ch->name, HEAD_GOD3);

    if( ch->level < LEVEL_IMMORTAL )
    {
        if( ch->class != class_lookup("cleric"))
        {
            send_to_char("Only a Cleric can grant the devotee mark of a god(ess).\n\r",ch);
            return;
        }

        if( ch->pcdata->highpriest == 0 )
        {
            send_to_char("Only a High Priest(ess) can grant the devotee mark of a god(ess).\n\r",ch);
            return;
        }
        if( arg1[0] == 0 )
        {
            send_to_char("Usage: mark <char>\n\r",ch);
            return;
        }

        if( ( gch = get_char_room( ch,NULL, arg1 ) ) == NULL )
        {
            send_to_char("They must be present during the ceremony.\n\r",ch);
            return;
        }

        if( IS_NPC(gch) || gch->level >= LEVEL_IMMORTAL )
        {
            send_to_char("Only mortals can wear a devotee mark.\n\r",ch);
            return;
        }
        religion = ch->religion_id;
        religionname = religion_table[religion].longname;

        if( gch->religion_id != RELIGION_NONE )
        {
            send_to_char("You cannot mark one who follows a religion already.\n\r",ch);
            return;
        }

        sprintf(buf,"You now wear the devoted mark of %s.\n\r",
                   religion_table[religion].longname);
        send_to_char(buf,gch);
        sprintf(buf,"%s now wears the devoted mark of %s.\n\r",
                   gch->name,
                   religion_table[religion].longname);
        send_to_char(buf,ch);

        if( ( obj = get_eq_char(gch,WEAR_MARK) ) != NULL )
            extract_obj(obj);

        marktype = religion; 
        obj = create_object(get_obj_index(religion_table[marktype].devoteevnum), 0);

        if( obj == NULL )
        {
            send_to_char("No mark found, please inform immortal.\n\r",ch);
            return;
        }

        gch->devotee_id = religion;
        obj->cost = 0;
        obj_to_char(obj,gch);
        equip_char(gch,obj,WEAR_MARK);

	  if (gch->devotee_id == 1
        ||  gch->devotee_id == 2
        ||  gch->devotee_id == 10
        ||  gch->devotee_id == 13
        ||  gch->devotee_id == 16
        ||  gch->devotee_id == 17
        ||  gch->devotee_id == 20 )
        {
          gch->alignment = -1000;
        }

	  if (gch->devotee_id == 4
        ||  gch->devotee_id == 7
        ||  gch->devotee_id == 11
        ||  gch->devotee_id == 12
        ||  gch->devotee_id == 15
        ||  gch->devotee_id == 18
        ||  gch->devotee_id == 19 )
        {
          gch->alignment = 0;
        }

	  if (gch->devotee_id == 3
        ||  gch->devotee_id == 5
        ||  gch->devotee_id == 6
        ||  gch->devotee_id == 8
        ||  gch->devotee_id == 9
        ||  gch->devotee_id == 14
        ||  gch->devotee_id == 21 )
        {
          gch->alignment = 1000;
        }


        for ( d = descriptor_list; d != NULL; d = d->next )
        {
          CHAR_DATA *victim;
          char buf1[100];
          victim = d->original ? d->original : d->character;
          if ( d->connected == CON_PLAYING
          && IS_IMMORTAL(d->character) )
          {
            sprintf(buf1,"%s is devoting their life to %s.\n\r", 
            gch->name,religion_table[religion].godname);
            send_to_char(buf1, d->character);
          } 
	  }

        note_attach ( ch, type );
        free_string( ch->pnote->to_list );
        ch->pnote->to_list = str_dup (religion_table[religion].godname);

        note_attach ( ch, type );
        sprintf ( buf, "DEVOTION: %s", ch->name);
        free_string( ch->pnote->subject );
        ch->pnote->subject = str_dup( buf );   
 
        note_attach ( ch, type );
    	  sprintf ( buf, "%s has devoted to you, in worship with %s.\n\r",
        gch->name, ch->name);
    	  ch->pnote->text 			= str_dup ( buf ) ;
    	  ch->pnote->next                   = NULL;
    	  strtime                           = ctime( &current_time );
    	  strtime[strlen(strtime)-1]  	= '\0';
    	  ch->pnote->date                   = str_dup( strtime );
    	  ch->pnote->date_stamp             = current_time;
        if ( note_list == NULL )
        {
          note_list       = ch->pnote;
        }
        else
        {
         for ( pnote = note_list; pnote->next != NULL; pnote = pnote->next);
         pnote->next     = ch->pnote;
        }
        pnote               = ch->pnote;
        ch->pnote           = NULL;
        fclose( fpReserve );
        if ( ( fp = fopen( NOTE_FILE, "a" ) ) == NULL )
        {
          perror( NOTE_FILE );
        }
        else
        {
            fprintf( fp, "Sender  %s~\n",       pnote->sender);
            fprintf( fp, "Date    %s~\n",  pnote->date);
            fprintf( fp, "Stamp   %ld\n",  pnote->date_stamp);
            fprintf( fp, "To      %s~\n",  pnote->to_list);
            fprintf( fp, "Subject %s~\n",  pnote->subject);
            fprintf( fp, "Text\n%s~\n",  pnote->text);
            fclose( fp );
        }
        pnote->type = NOTE_NOTE;
        fpReserve = fopen( NULL_FILE, "r" );
        return;
    }

    if ( !head_god && !( ch->religion_id & RELIGION_LEADER ) )
    {
        send_to_char( "You cannot possibly have mortal followers.\n\r", ch );
        return;
    }

    /* make sure they spec'd an inductee */
    if( arg1[0] == '\0' )
    {
      send_to_char( "Usage: mark <char> <religionname> [leader]or[high]or[devotee]\n\r",ch);
        return;
    }

    if( arg2[0] == '\0' )
    {
      char buf[MAX_STRING_LENGTH];
      BUFFER *buffer;

      buffer = new_buf();
      add_buf(buffer, "Available Religions Are:");
      add_buf(buffer, "\n\r");
      for ( religion = 0; religion_table[religion].longname != NULL; religion++ )
      {
	if ( (religion % 2) == 0 )
        add_buf(buffer, "\n\r");
	sprintf( buf, "%-15s - %-15s",
        religion_table[religion].longname,
	religion_table[religion].godname);
	add_buf(buffer, buf);
      }
      add_buf(buffer, "\n\r");
      page_to_char( buf_string(buffer), ch );
      return;
    }

    if ( ( gch = get_char_world( ch, arg1 ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if( IS_NPC(gch) )
    {
        send_to_char( "Only players can become your followers.\n\r", ch );
        return;
    }
    /* get religion name */
    for( religion = 0, religionname = NULL; religion < MAX_RELIGION; religion++) 
    {
        if( !str_cmp( arg2, religion_table[religion].longname ) )
        {
            religionname = religion_table[religion].longname;
            break;
        }
    }
    if( religionname == NULL )
    {
        send_to_char( "That is not a valid religion.\n\r", ch );
        return;
    }

    if (!str_cmp( arg2, "religion none" ) )
    {
        send_to_char( "That is not a valid religion.\n\r", ch );
        return;
    }

    if (!str_cmp( arg2, "none" ) )
    {
        send_to_char( "That is not a valid religion.\n\r", ch );
        return;
    }

    /* see if they're making a leader */
    if( arg3[0] != '\0' && !str_cmp( arg3, "leader" ) )
    {
        if( gch->level < LEVEL_IMMORTAL )
        {
            send_to_char( "Only immortals may lead a religion.\n\r", ch );
            return;
        }
        leader = RELIGION_LEADER;
    }

    if( arg3[0] != 0 && !str_cmp(arg3,"devotee") )
    {

      if ( ( gch = get_char_world( ch, arg1 ) ) == NULL )
      {
        send_to_char( "They aren't here.\n\r", ch );
        return;
      }

      if( IS_NPC(gch) || gch->level >= LEVEL_IMMORTAL )
      {
        send_to_char("Only mortal players can be marked.\n\r",ch);
        return;
      }

      if( gch->religion_id != RELIGION_NONE )
      {
        send_to_char("You cannot mark one who follows a religion already.\n\r",ch);
        return;
      }

	if( ( obj = get_eq_char(gch,WEAR_MARK) ) != NULL )
	{
	    sprintf( buf, "%s no longer devotes to %s.\n\r",
            gch->name,
            religion_table[RELIGION(gch->devotee_id)].longname);
            send_to_char( buf, ch );	
	    sprintf( buf, "You no longer devote to %s.\n\r",
			religion_table[RELIGION(gch->devotee_id)].longname);
	    send_to_char( buf, gch);
            extract_obj(obj);
	}

      marktype = religion; 
      obj = create_object(get_obj_index(religion_table[marktype].devoteevnum), 0);

      if( obj == NULL )
      {
        send_to_char("No mark found, please inform immortal.\n\r",ch);
        return;
      }

      gch->devotee_id = religion;
      obj->cost = 0;
      obj_to_char(obj,gch);
      equip_char(gch,obj,WEAR_MARK);
	sprintf( buf, "%s is now devoted to you.\n\r",gch->name);
	send_to_char(buf, ch);
	sprintf( buf, "You are now a devotee of %s.\n\r",
		    religion_table[RELIGION(gch->devotee_id)].longname);
	send_to_char(buf, gch);
        return;
    }

    if( arg3[0] != 0 && !str_cmp(arg3,"high") )
    {
      if( gch->level >= LEVEL_IMMORTAL )
      {
        send_to_char("Only mortals can be your High Priest(ess).\n\r",ch);
        return;
      }

      if( gch->class != class_lookup("cleric"))
      {
        send_to_char("Only Clerics can actually be a High Priest(ess).\n\r",ch);
        return;
      }
      gch->pcdata->highpriest = 1;
    }

    /* see if they're an immort (only HEAD_GOD can do 'em) */
    if( gch->level >= LEVEL_IMMORTAL )
    {
      if( !head_god )
      {
        send_to_char( "You are not qualified to select immortals to lead a religion.\n\r",ch );
        return;
      }
      /* immortals are always leaders */
      leader = RELIGION_LEADER;
    }
    /* see if they're trying to mark into religion other than own */
    if( religion != RELIGION( ch->religion_id) && !head_god)
    {
      send_to_char( "You are not qualified to choose the followers of others.\n\r",ch );
      return;
    }

    /* see if they're already in another religion */
    if( gch->religion_id != RELIGION_NONE )
    {
      if( (gch->level >= LEVEL_AVATAR && !head_god) || !head_god)
      {
        send_to_char(
          "They are in another religion.  You are not qualified to\n\r", ch);
        send_to_char( "lead simultaneous religions.\n\r", ch );
        return;
      }
    }
    /* we're cleared do the markring */
    /* if they're already in a religion, inform of departure */
    if( gch->religion_id != RELIGION_NONE ) 
    {
      sprintf( buf, "%s changes religions from %s and now follows %s!\n\r",
        gch->name,
        religion_table[RELIGION(gch->religion_id)].longname,
        religion_table[RELIGION(religion)].longname );
      send_to_char( buf, ch );
    }

    /* Removes mark from character upon change of org if needed */
    obj = get_eq_char( gch, WEAR_MARK );
    if ( ( obj = get_eq_char( gch, WEAR_MARK ) ) != NULL )
      extract_obj ( obj );

    marktype = religion;
    obj = create_object( get_obj_index(religion_table[marktype].markvnum), 0 );

    if (obj == NULL)
    {
     send_to_char("No mark found\n\r",ch);
     return;
    }
    gch->devotee_id = 0;
    obj->cost = 0;
    obj_to_char( obj, gch);
    equip_char( gch, obj, WEAR_MARK);


    if (IS_IMMORTAL(ch))
    {
      if (IS_GOOD(ch))
        gch->alignment = 1000;

      if (IS_EVIL(ch))
        gch->alignment = -1000;

      if (IS_NEUTRAL(ch))
        gch->alignment = 0;
    }

    /* tell of new religion */
    gch->religion_id = religion | leader;
    arg1[0] = 0;
    if (gch->level >= LEVEL_IMMORTAL )
    {
      strcpy( arg1, " as head Immortal" );
        sprintf( buf, "You mark %s with the %s%s!\n\r",
        gch->name, religion_table[RELIGION(religion)].markdesc, arg1 );
        send_to_char( buf, ch );
      sprintf(buf, "%s marks you with the %s%s!\n\r",
        ch->name,
        religion_table[RELIGION(religion)].markdesc, arg1 );
        send_to_char( buf, gch );
    }

    if (gch ->level < LEVEL_IMMORTAL )
    {
      if( gch->pcdata->highpriest == 1 )
      {
        sprintf( buf, "You mark %s with the %s, as a High Priest(ess)!\n\r",
          gch->name, religion_table[RELIGION(religion)].markdesc);
          send_to_char( buf, ch );
        sprintf(buf, "%s marks you with the %s, as a High Priest(ess)!\n\r",
          ch->name,
          religion_table[RELIGION(religion)].markdesc);
          send_to_char( buf, gch );
        }
        else
        {
          sprintf( buf, "You mark %s with the %s!\n\r",
            gch->name, religion_table[RELIGION(religion)].markdesc);
            send_to_char( buf, ch );
          sprintf(buf, "%s marks you with the %s!\n\r",
            ch->name,
            religion_table[RELIGION(religion)].markdesc);
            send_to_char( buf, gch );
        }
    }
} 

void do_description( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];

  if (!str_cmp(argument, "format"))
  {
    desc_format(ch, 1, 100);
    return;
  }

  if ( argument[0] != '\0' )
  {
    buf[0] = '\0';
    smash_tilde( argument );

    if (argument[0] == '-')
    {
      int len;
      bool found = FALSE;
 
      if (ch->description == NULL || ch->description[0] == '\0')
      {
        send_to_char("No lines left to remove.\n\r",ch);
        return;
      }
	
  	strcpy(buf,ch->description);
 
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
		free_string(ch->description);
		ch->description = str_dup(buf);
		send_to_char( "Your description is:\n\r", ch );
		send_to_char( ch->description ? ch->description : 
		"(None).\n\r", ch );
            return;
          }
        }
      }
      buf[0] = '\0';
	free_string(ch->description);
	ch->description = str_dup(buf);
	send_to_char("Description cleared.\n\r",ch);
	return;
    }

    if ( !str_cmp( argument, "edit" ) )
    {
      SET_BIT(ch->comm,COMM_EDITOR);
	string_append( ch, &ch->description );
	return;
    }

    if ( argument[0] == '+' )
    {
	if ( ch->description != NULL )
      strcat( buf, ch->description );
	argument++;
	while ( isspace(*argument) )
	argument++;
      strcat( buf, argument );
      strcat( buf, "\n\r" );
      free_string( ch->description );
      ch->description = str_dup( buf );
      send_to_char( "Your description is:\n\r", ch );
      send_to_char( ch->description ? ch->description : "(None).\n\r", ch );
      return;
    }

    if ( strlen(buf) >= 2048)
    {
	send_to_char( "Description too long.\n\r", ch );
	return;
    }
  }

  send_to_char( "Your description is:\n\r", ch );
  send_to_char( ch->description ? ch->description : "(None).\n\r", ch );
  return;
}

void desc_format( CHAR_DATA *ch, int start, int lines )
{
    char buf[MAX_STRING_LENGTH];
    char wordbuf[MAX_INPUT_LENGTH];
    char *p, *bp, *wp;
    int i,inword;
    /* find starting line to pretty-ify */
    for( i = 1, p = ch->description, bp = buf; *p != 0 && i < start; p++ )
    {
        *bp++ = *p;
        if( *p == '\r' )
            i++;
    }
    *bp = 0;
    /* now build pretty lines from raw ones */
    pretty_proc( bp, NULL );            /* init pretty processor */
    for( i = inword = 0, wp = wordbuf; i < lines && *p != 0; p++ )
    {
        if( *p == ' ' )
        {
            if( inword )
            {
                inword = 0;
                *wp = 0;
                pretty_proc( NULL, wordbuf );
                wp = wordbuf;
            }
            *wp++ = ' ';
        }
        else if( *p == '\r' )
        {
            i++;                /* inc line count */
            if( inword )
            {
                inword = 0;
                *wp = 0;
                pretty_proc( NULL, wordbuf );
                wp = wordbuf;
                if( p[1] == '\n' || p[1] == ' ' || p[1] == 0)
                    pretty_proc( NULL, "\n\r" );
                else
                    pretty_proc( NULL, " " );
            }
            else
            {
                pretty_proc( NULL, "\n\r" );
                wp = wordbuf;
            }
        }
        else if( *p == '\n' )
            continue;
        else {
            inword = 1;
            *wp++ = *p;
        }
    }
    /* and append any leftover lines directly */
    strcat( buf, p );
    /* and swap in the new editted description */
    free_string( ch->description );
    ch->description = str_dup( buf );
    send_to_char( "Ok.\n\r", ch );
}

void back_format( CHAR_DATA *ch, int start, int lines )
{
  char buf[MAX_STRING_LENGTH];
  char wordbuf[MAX_INPUT_LENGTH];
  char *p, *bp, *wp;
  int i,inword;
  /* find starting line to pretty-ify */
  for( i = 1, p = ch->background, bp = buf; *p != 0 && i < start; p++ )
  {
    *bp++ = *p;
    if( *p == '\r' )
      i++;
  }
  *bp = 0;
  /* now build pretty lines from raw ones */
  pretty_proc( bp, NULL );            /* init pretty processor */
  for( i = inword = 0, wp = wordbuf; i < lines && *p != 0; p++ )
  {
    if( *p == ' ' )
    {
      if( inword )
      {
        inword = 0;
        *wp = 0;
        pretty_proc( NULL, wordbuf );
        wp = wordbuf;
      }
      *wp++ = ' ';
    }
    else
    if( *p == '\r' )
    {
      i++;                /* inc line count */
      if( inword )
      {
        inword = 0;
        *wp = 0;
        pretty_proc( NULL, wordbuf );
        wp = wordbuf;
        if( p[1] == '\n' || p[1] == ' ' || p[1] == 0)
          pretty_proc( NULL, "\n\r" );
        else
          pretty_proc( NULL, " " );
      }
      else
      {
        pretty_proc( NULL, "\n\r" );
        wp = wordbuf;
      }
    }
    else
    if( *p == '\n' )
      continue;
    else
    {
      inword = 1;
      *wp++ = *p;
    }
  }
  /* and append any leftover lines directly */
  strcat( buf, p );
  /* and swap in the new editted background */
  free_string( ch->background );
  ch->background = str_dup( buf );
  send_to_char( "Ok.\n\r", ch );
}


void do_background( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];

  if (!str_cmp(argument, "format"))
  {
    back_format(ch, 1, 100);
    return;
  }

  if ( argument[0] != '\0' )
  {
    buf[0] = '\0';
    smash_tilde( argument );

    if (argument[0] == '-')
    {
      int len;
      bool found = FALSE;
 
      if (ch->background == NULL || ch->background[0] == '\0')
      {
        send_to_char("No lines left to remove.\n\r",ch);
        return;
      }
	
  	strcpy(buf,ch->background);
 
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
		free_string(ch->background);
		ch->background = str_dup(buf);
		send_to_char( "Your background is:\n\r", ch );
		send_to_char( ch->background ? ch->background : 
		"(None).\n\r", ch );
            return;
          }
        }
      }
      buf[0] = '\0';
	free_string(ch->background);
	ch->background = str_dup(buf);
	send_to_char("Background cleared.\n\r",ch);
	return;
    }

    if ( !str_cmp( argument, "edit" ) )
    {
      SET_BIT(ch->comm,COMM_EDITOR);
	string_append( ch, &ch->background );
	return;
    }

    if ( argument[0] == '+' )
    {
	if ( ch->background != NULL )
      strcat( buf, ch->background );
	argument++;
	while ( isspace(*argument) )
	argument++;
      strcat( buf, argument );
      strcat( buf, "\n\r" );
      free_string( ch->background );
      ch->background = str_dup( buf );
      send_to_char( "Your background is:\n\r", ch );
      send_to_char( ch->background ? ch->background : "(None).\n\r", ch );
      return;
    }

    if ( strlen(buf) >= 2048)
    {
	send_to_char( "Background too long.\n\r", ch );
	return;
    }
  }

  send_to_char( "Your background is:\n\r", ch );
  send_to_char( ch->background ? ch->background : "(None).\n\r", ch );
  return;
}

void do_preach( CHAR_DATA *ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  AFFECT_DATA af;
  int count;

  if (IS_NPC(ch))
  return;

  count = 0;

  if ((ch->level < skill_table[gsn_preach].skill_level[ch->class])
  ||  (!IS_NPC(ch) && ch->pcdata->learned[gsn_preach] < 1 ))
  {
    send_to_char( "Huh? You don't know the first thing about preaching the word of God!\n\r",ch  );
    return;
  }

  if (ch->religion_id == RELIGION_NONE)
  { 
    send_to_char("You worship no God yet! How could you possibly preach about them?\n\r", ch);
    return;
  }

  if( ch->pcdata->highpriest == 0 )
  {
    send_to_char("Only a High Priest(ess) can preach the word of their God(ess).\n\r",ch);
    return;
  }

  if (IS_AFFECTED3(ch,AFF_PREACH))
  {
    send_to_char("Your still too exhausted from the last sermon you gave, to preach to others about your God just yet.\n\r",ch);
    return;
  }

  for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
  {
    vch_next = vch->next_in_room;

    if (!IS_AFFECTED3(vch,AFF_PREACH)
    && !IS_NPC(vch)
    && can_see( ch, vch ))
      count++;
  }

  if ( count <= 1 )
  {
    sprintf(buf,"There's nobody around for you to preach the word of your God, %s to.\n\r", religion_table[ch->religion_id].godname);
    send_to_char(buf,ch);
    return;
  }

  for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
  {
    vch_next = vch->next_in_room;

    if ( number_percent( ) < ch->pcdata->learned[gsn_preach])
      check_improve(ch,gsn_preach,TRUE,1);
    else
    {
      check_improve(ch,gsn_preach,FALSE,1);
      sprintf(buf,"%s tries to preach the word of God, %s, but suddenly goes blank and gives up.\n\r", ch->name, religion_table[ch->religion_id].godname);
      send_to_char(buf,vch);
      send_to_char("You try to preach the word of your God, but suddenly go blank.\n\r",ch);
      send_to_char("You give up and bow your head in disgrace.\n\r",ch);
      return;
    }

    if(!IS_NPC(vch)
    && (vch != ch)
    && can_see( ch, vch ))
    {
      if (!IS_AFFECTED3(vch,AFF_PREACH))
      {
        if (!IS_AFFECTED3(ch,AFF_PREACH))
        {
          af.where	= TO_AFFECTS3;
          af.type     	= gsn_preach;
          af.level    	= ch->level;
          af.duration 	= ch->level/3;
          af.location 	= 0;
          af.modifier  	= 0;
          af.bitvector 	= AFF_PREACH;
          affect_to_char(ch,&af);
        }

	  if ((ch->religion_id == 3
        ||  ch->religion_id == 5
        ||  ch->religion_id == 6
        ||  ch->religion_id == 8
        ||  ch->religion_id == 9
        ||  ch->religion_id == 14
        ||  ch->religion_id == 21 )
        &&  ch->religion_id != RELIGION_NONE)
        {
          if(vch->alignment <= -750)
          {
            sprintf(buf,"%s has strayed too far towards the path of Darkness for your\n\rpreaching to have any effect on them.\n\r", vch->name);
            send_to_char(buf,ch);
            continue;
          }

          if(vch->alignment >= 1000)
          {
            sprintf(buf,"%s is already as close to the Gods of Light as one can be.\n\rYour preaching has no effect on them.\n\r", vch->name);
            send_to_char(buf,ch);
            continue;
          }

	    af.where	= TO_AFFECTS3;
          af.type     	= gsn_preach;
          af.level    	= ch->level;
          af.duration 	= ch->level/3;
          af.location 	= 0;
          af.modifier  	= 0;
          af.bitvector 	= AFF_PREACH;
          affect_to_char(vch,&af);

          if(ch->level >= 63)
            vch->alignment += 35;
          else
          if(ch->level >= 70)
            vch->alignment += 55;
          else
          if(ch->level >= 80)
            vch->alignment += 70;
          else
          if(ch->level >= 90)
            vch->alignment += 85;

          if(vch->alignment > 1000)
            vch->alignment = 1000;
   
          sprintf(buf,"%s raises their voice and begins to preach the Holy Word of %s.\n\r",
                 ch->name,
                 religion_table[ch->religion_id].godname);
          sprintf(buf,"You raise your voice and begin to preach the Holy Word of %s to %s.\n\r", religion_table[ch->religion_id].godname, vch->name);
          send_to_char(buf,ch);
          send_to_char("You feel closer to the Gods of Light.\n\r",vch);
        } 

	  if((ch->religion_id == 1
        ||  ch->religion_id == 2
        ||  ch->religion_id == 10
        ||  ch->religion_id == 13
        ||  ch->religion_id == 16
        ||  ch->religion_id == 17
        ||  ch->religion_id == 20 )
        &&  ch->religion_id != RELIGION_NONE)
        {
          if(vch->alignment >= 750)
          {
            sprintf(buf,"%s has strayed too far into the Light for your preaching of\n\rEvil Ways to have any effect on them.\n\r", vch->name);
            send_to_char(buf,ch);
            continue;
          }

          if(vch->alignment <= -1000)
          {
            sprintf(buf,"%s is already as close to the Gods of Darkness as one can be.\n\rYour preaching has no effect on them.\n\r", vch->name);
            send_to_char(buf,ch);
            continue;
          }

	    af.where	= TO_AFFECTS3;
          af.type     	= gsn_preach;
          af.level    	= ch->level;
          af.duration 	= ch->level/3;
          af.location 	= 0;
          af.modifier  	= 0;
          af.bitvector 	= AFF_PREACH;
          affect_to_char(vch,&af);

          if(ch->level >= 63)
           vch->alignment -= 35;
          else
          if(ch->level >= 70)
           vch->alignment -= 55;
          else
          if(ch->level >= 80)
           vch->alignment -= 70;
          else
          if(ch->level >= 90)
           vch->alignment -= 85;

          if(vch->alignment < -1000)
            vch->alignment = -1000;
   
          sprintf(buf,"%s raises their voice and begins to preach the UnHoly Word of %s.\n\r",
                 ch->name,
                 religion_table[ch->religion_id].godname);
          sprintf(buf,"You raise your voice and begin to preach the UnHoly Word of %s to %s.\n\r", religion_table[ch->religion_id].godname, vch->name);
          send_to_char(buf,ch);
          send_to_char("You feel closer to the Gods of Darkness.\n\r",vch);
        }

	  if((ch->religion_id == 4
        ||  ch->religion_id == 7
        ||  ch->religion_id == 11
        ||  ch->religion_id == 12
        ||  ch->religion_id == 15
        ||  ch->religion_id == 18
        ||  ch->religion_id == 19 )
        &&  ch->religion_id != RELIGION_NONE)
        {
          if(vch->alignment >= 750)
          {
            sprintf(buf,"%s has strayed too far into the Light for your preaching of\n\rBalance and Neutrality to have any effect on them.\n\r", vch->name);
            send_to_char(buf,ch);
            continue;
          }

          if(vch->alignment <= -750)
          {
            sprintf(buf,"%s has strayed too far into the Darkness for your preaching of\n\rBalance and Neutrality to have any effect on them.\n\r", vch->name);
            send_to_char(buf,ch);
            continue;
          }

          if(vch->alignment == 0)
          {
            sprintf(buf,"%s is already as close to the Gods of Neutrality as one can be.\nYour preaching has no effect on them.\n\r", vch->name);
            send_to_char(buf,ch);
            continue;
          }

	    af.where	= TO_AFFECTS3;
          af.type     	= gsn_preach;
          af.level    	= ch->level;
          af.duration 	= ch->level/3;
          af.location 	= 0;
          af.modifier  	= 0;
          af.bitvector 	= AFF_PREACH;
          affect_to_char(vch,&af);

          if(vch->alignment >= 85)
          {
            if(ch->level >= 63)
             vch->alignment -= 35;
            else
            if(ch->level >= 70)
             vch->alignment -= 55;
            else
            if(ch->level >= 80)
             vch->alignment -= 70;
            else
            if(ch->level >= 90)
             vch->alignment -= 85;
          }
          else
          if(vch->alignment <= -85)
          {
            if(ch->level >= 63)
             vch->alignment += 35;
            else
            if(ch->level >= 70)
             vch->alignment += 55;
            else
            if(ch->level >= 80)
             vch->alignment += 70;
            else
            if(ch->level >= 90)
             vch->alignment += 85;
          }
          else
          if((vch->alignment >= -85)
	    && (vch->alignment <= -1))
	    {
		vch->alignment = 0;
	    }
          else
          if((vch->alignment <= 85)
	    && (vch->alignment >= 1))
	    {
		vch->alignment = 0;
	    }
   
          sprintf(buf,"%s raises their voice and begins to preach the Word of %s.\n\r",
                 ch->name,
                 religion_table[ch->religion_id].godname);
          send_to_char(buf,vch);
          sprintf(buf,"You raise your voice and begin to preach the Word of %s to %s.\n\r",
                 religion_table[ch->religion_id].godname, vch->name);
          send_to_char(buf,ch);
          send_to_char("You feel closer to the Gods of Balance and Neutrality.\n\r",vch);
        }
      }
    }
  }
  return;
}

void write_songs( void ) /* MGD-S */
{
    FILE *fp;
    SONG_DATA *psong;

    fclose( fpReserve );

    if ( ( fp = fopen( SONG_FILE, "w" ) ) == NULL )
    {
        perror( SONG_FILE );
    }
    else
    {
        for ( psong = song_list; psong != NULL; psong = psong->next )
        {
            fprintf( fp, "Composer  %s~\n",  psong->composer);
            fprintf( fp, "Date      %s~\n",  psong->date);
            fprintf( fp, "Stamp     %ld\n",  psong->date_stamp);
            fprintf( fp, "To        %s~\n",  psong->to_list);
            fprintf( fp, "Title     %s~\n",  psong->title);
            fprintf( fp, "Recitals  %d\n",   psong->recitals);
            fprintf( fp, "Text\n%s~\n",      psong->text);
        }
        fclose( fp );
    }
    fpReserve = fopen( NULL_FILE, "r" );
    return;
}

void do_sing( CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *ich;
  char buf[MAX_STRING_LENGTH];
  int anum;
  SONG_DATA *psong;
  SINGING_DATA *new_singing;
  char text[MAX_INPUT_LENGTH];
  int vnum;
    
  buf[0]='\0';
  text[0]='\0';
   
  if ( ch->class != class_lookup("bard") && !IS_IMMORTAL(ch) )
  {
    send_to_char("You are not trained in the art of singing.\n\r",ch);
    return;
  };
  
  if ( is_number( argument ) )
  {
    anum = atoi( argument );
    vnum = 0;

    for ( psong = song_list; psong != NULL; psong = psong->next )
    {
      if ( knows_song( ch, psong ) && ( vnum++ == anum  ) )
      {        
        /* new_singing = alloc_perm(sizeof(ch->psinging)); */

	  if ( singing_free == NULL )
	  {
	    new_singing     = alloc_perm( sizeof(*ch->psinging) );
	  }
	  else
	  {
	    new_singing     = singing_free;
	    singing_free    = singing_free->next;
	  }

        if (singing_list == NULL)
        {
          singing_list    = new_singing;
        }       
	  else
	  {
	    new_singing->next 	  = singing_list;
	    singing_list          = new_singing;
        }

	  new_singing->singer       = ch;
	  new_singing->title        = str_dup( psong->title );
	  new_singing->text         = str_dup( psong->text );
        new_singing->composer     = str_dup( psong->composer );
        new_singing->pos          = 0;

        psong->recitals++;

        strcat(buf, psong->to_list);
        psong->to_list = NULL;
                
        for ( ich = char_list ; ich != NULL ; ich = ich->next)
        {
          if ( ich->class == class_lookup("bard") 
          && ich->in_room->area == ch->in_room->area 
          && ich != ch 
          && !is_name(ich->name, buf) )
          {
            strcat( buf, " " );
            strcat( buf, ich->name );
          }
        }      
        psong->to_list = str_dup(buf);
        write_songs();

	  if (!IS_AWAKE(ch))
	    return;

	  if ((IS_AFFECTED2(ch, AFF_SPEECH_IMPAIR))
        ||  (IS_AFFECTED2(ch, AFF_SILENCE)))
	   return;

        sprintf(text,"May the Gods favor my voice in singing '%s', by %s",
		        new_singing->title,
		        new_singing->composer);
	  sing_channel(ch,text);
	  return;
      }   
    }
    send_to_char("You don't know that song.\n\r", ch);
    return;
  }
  if (!str_cmp("stop", argument))
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
    send_to_char("Ok.\n\r", ch);
    return;
  }
  send_to_char("Usage: sing <number> / stop\n\r",ch);
}

void singing_update(void)
{
  SINGING_DATA *psong;
  SINGING_DATA *psong2;
  CHAR_DATA *ch;
  int pos; 
  int count;
  char letter;
  char buf[MAX_STRING_LENGTH];
  buf[0]='\0';
    
  for (psong = singing_list; psong != NULL; psong = psong->next)
  {
    ch = psong->singer;
    for ( pos = psong->pos, count = 0 ; count < PLINE; pos++, count++ )
    {
      letter = psong->text[pos];
            
      if ( ch == NULL || ch->desc == NULL || letter == '\0' )
      {
        if ( singing_list == psong )
        {
          singing_list = psong->next;
          free_string(psong->title);
          free_string(psong->text);
          free_string(psong->composer);
          singing_free = psong;
        }
        else
        {
          for (psong2 = singing_list ; psong2 != NULL ; psong2 = psong2->next)
          {
            if (psong2->next == psong)
            break;
          }
                    
          psong2->next = psong->next;
          free_string(psong->title);
          free_string(psong->text);
          free_string(psong->composer);
          singing_free = psong;                         
        }

        count--;
        break;
      }  
          
      /* counts the amount of characters.  When it gets above 60 in the line it
	 * looks for the next SPACE.  When it reaches that next space, it stops,
	 * goes back 1 count to get rid of the SPACE, then marsk that as the
	 * cut-off to stop on that line and start the new one
	 */
      if ( ( count > ( (60)) && (letter == ' ' )) )
	{
	  buf[count] = letter;
        count--;
        break;
      }
            
      if ( letter == '\n' )
      {
       letter = ' ';
      }

      if ( letter == '\r' )
      {
        letter = ' ';
        count--;
      }

      if ( letter == '/' )
      {
        letter = '.';
        /* buf[count] = letter; */
	  /*removing the COUNT-- to keep it a space in order to properly
	   * allocate the letter count to go to next line.  Otherwise it
	   * won't be treated as a space, and the letters will look like
	   * a run-on sentence, and a new line won't be accounted for until
	   * too long into the sentence, making the output look fugly
	   */
        /* count--; */
      }
            
      if ( letter == '.' 
      ||   letter == '!'
      ||   letter == '?' )
      {
        buf[count] = letter;
        break;
      }
                         
      buf[count] = letter;
    }
    if ( psong != NULL ) psong->pos = pos + 1;
        
    buf[count+1] = '\0';

    pretty_singing(buf);

    if ( buf[0] != '\0' )
    sing_channel(ch,buf);       
  }         
  return;
}    

void pretty_singing( char * buf)
{
  char buf2[MAX_INPUT_LENGTH];
  int i;
  int count;
    
  for ( i=0 ; buf[i] == ' ' ; i++);
    
  for ( count =0 ; 1 ; i++, count++)
  {
    buf2[count] = buf[i];
    if ( buf[i] == '\0' ) break;
  }
    
  for ( i=0 ; 1 ; i++ )
  {
    buf[i] = buf2[i];
    if (buf2[i] == '\0') break;
  }
}

bool knows_song( CHAR_DATA *ch, SONG_DATA *psong )
{
    if ( !str_cmp( ch->name, psong->composer ) )
        return TRUE;
    if ( is_name( "all", psong->to_list ) )
        return TRUE;
    if ( IS_IMMORTAL(ch) )
	return TRUE;
    if ( IS_HERO(ch) )
	return TRUE;
    if ( is_name( ch->name, psong->to_list ) )
	return TRUE;

    return FALSE;
}

void song_attach( CHAR_DATA *ch )
{
  SONG_DATA *psong;

  if ( ch->psong != NULL )
    return;

  if ( song_free == NULL )
  {
    psong     = alloc_perm( sizeof(*ch->psong) );
  }
  else
  {
    psong     = song_free;
    song_free = song_free->next;
  }

  psong->next         = NULL;
  psong->composer     = str_dup( ch->name );
  psong->date         = str_dup( "" );
  psong->to_list      = str_dup( "" );
  psong->title        = str_dup( "" );
  psong->text         = str_dup( "" );
  ch->psong           = psong;
  return;
}

void song_remove( CHAR_DATA *ch, SONG_DATA *psong )
{
  char to_new[MAX_INPUT_LENGTH];
  char to_one[MAX_INPUT_LENGTH];
  FILE *fp;
  SONG_DATA *prev;
  char *to_list;

 /*
  * Build a new to_list.
  * Strip out this recipient.
  */
  to_new[0]   = '\0';
  to_list     = psong->to_list;
  while ( *to_list != '\0' )
  {
    to_list = one_argument( to_list, to_one );
    if ( to_one[0] != '\0' && str_cmp( ch->name, to_one ) )
    {
      strcat( to_new, " " );
      strcat( to_new, to_one );
    }
  }

  if ( str_cmp( ch->name, psong->composer ) && to_new[0] != '\0' )
  {
    free_string( psong->to_list );
    psong->to_list = str_dup( to_new + 1 );
    return;
  }

  if ( psong == song_list )
  {
    song_list = psong->next;
  }
  else
  {
    for ( prev = song_list; prev != NULL; prev = prev->next )
    {
      if ( prev->next == psong )
        break;
    }
    if ( prev == NULL )
    {
      bug( "Song_remove: psong not found.", 0 );
      return;
    }
    prev->next = psong->next;
  }
  free_string( psong->text     );
  free_string( psong->title    );
  free_string( psong->to_list  );
  free_string( psong->date     );
  free_string( psong->composer );
  psong->next = song_free;
  song_free   = psong;

 /*
  * Rewrite entire list.
  */

  fclose( fpReserve );
  if ( ( fp = fopen( SONG_FILE, "w" ) ) == NULL )
  {
        perror( SONG_FILE );
  }
  else
  {
    for ( psong = song_list; psong != NULL; psong = psong->next )
    {
      fprintf( fp, "Composer  %s~\n", psong->composer);
      fprintf( fp, "Date      %s~\n", psong->date);
      fprintf( fp, "Stamp     %ld\n",  psong->date_stamp);
      fprintf( fp, "To        %s~\n", psong->to_list);
      fprintf( fp, "Title     %s~\n", psong->title);
      fprintf( fp, "Recitals  %d\n", psong->recitals);
      fprintf( fp, "Text\n%s~\n",   psong->text);
    }
        fclose( fp );
  }
  fpReserve = fopen( NULL_FILE, "r" );
  return;
}

/* used by imps to nuke a song for good */
void song_expunge( SONG_DATA *psong )
{
    FILE *fp;
    SONG_DATA *prev;
 
 /*
  * Remove song from linked list.
  */
  if ( psong == song_list )
  {
    song_list = psong->next;
  }
  else
  {
    for ( prev = song_list; prev != NULL; prev = prev->next )
    {
      if ( prev->next == psong )
      break;
    }
 
    if ( prev == NULL )
    {
      bug( "Song_expunge: psong not found.", 0 );
      return;
    }
    prev->next = psong->next;
  }
 
  free_string( psong->text     );
  free_string( psong->title    );
  free_string( psong->to_list  );
  free_string( psong->date     );
  free_string( psong->composer );
  psong->next = song_free;
  song_free   = psong;
 
 /*
  * Rewrite entire list.
  */
  fclose( fpReserve );

  if ( ( fp = fopen( SONG_FILE, "w" ) ) == NULL )
  {
    perror( SONG_FILE );
  }
  else
  {
    for ( psong = song_list; psong != NULL; psong = psong->next )
    {
      fprintf( fp, "Composer  %s~\n", psong->composer);
      fprintf( fp, "Date      %s~\n", psong->date);
      fprintf( fp, "Stamp     %ld\n",  psong->date_stamp);
      fprintf( fp, "To        %s~\n", psong->to_list);
      fprintf( fp, "Title     %s~\n", psong->title);
      fprintf( fp, "Recitals  %d\n", psong->recitals);
      fprintf( fp, "Text\n%s~\n",   psong->text);
    }
    fclose( fp );
  }
  fpReserve = fopen( NULL_FILE, "r" );
  return;
}

void do_song( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];
  char output[MAX_STRING_LENGTH*5]; //increase this as song list grows
  char arg[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char arg3[MAX_INPUT_LENGTH];
  SONG_DATA *psong = NULL;
  int start,lines;
  int vnum;
  int anum;
  output[0] = '\0';
  buf[0] = '\0';
  if ( IS_NPC(ch) )
        return;

  if ( ch->class != class_lookup("bard") && !IS_IMMORTAL(ch) )
  {
    send_to_char("You are not trained in the art of singing.\n\r",ch);
    return;
  }
 
  argument = one_argument( argument, arg );
  smash_tilde( argument );
  if (arg[0] == '\0')
  {
    send_to_char("Type 'help song' for information on this command.\n\r",ch);
    return;
  }
  if ( !str_prefix( arg, "list" ) )
  {
    vnum = 0;
    for ( psong = song_list; psong != NULL; psong = psong->next )
    {
      if ( knows_song( ch, psong ) )
      {
        if (IS_IMMORTAL(ch))
        {
          sprintf( buf, "[%3d R: %d] %s: %s\n\r",
            vnum,
            psong->recitals, 
            psong->composer, 
            psong->title);
          if (!vnum)
		sprintf(output, buf);
	    else
		strcat(output, buf);
            vnum++;
          }  
          else  
          {
            sprintf( buf, "[%3d] %s: %s\n\r",
              vnum, 
              psong->composer, 
              psong->title );
		if (!vnum)
              sprintf(output, buf);
            else
            strcat(output, buf);
            vnum++;
          }
        }
      }
	page_to_char(output, ch);
      return;
    }
    if ( !str_prefix( arg, "read" ) )
    {
      bool fAll;
      if ( !str_cmp( argument, "all" ) )
      {
        fAll = TRUE;
        anum = 0;
      }
        
      else if ( is_number( argument ) )
      {
        fAll = FALSE;
        anum = atoi( argument );
      }
      else
      {
        send_to_char( "Song read which number?\n\r", ch );
        return;
      }
      vnum = 0;
      for ( psong = song_list; psong != NULL; psong = psong->next )
      {
        if ( knows_song( ch, psong ) && ( vnum++ == anum || fAll ) )
        {
          sprintf( buf, "[%3d] %s: %s\n\r",
            vnum - 1,
            psong->composer,
            psong->title);
          send_to_char( buf, ch );
          send_to_char( psong->text, ch );
          return;
        }
      }
      send_to_char( "No such song.\n\r", ch );
      return;
    }
    if ( !str_cmp( arg, "+" ) )
    {
        song_attach( ch );
        strcpy( buf, ch->psong->text );
        if ( strlen(buf) + strlen(argument) >= MAX_STRING_LENGTH - 200 )
        {
            send_to_char( "Song too long.\n\r", ch );
            return;
        }
        /* let '.' be a marker for leading white space */
        if( argument[0] == '.' )
            argument++;
        /* make sure they didn't use tabs */
        for( start = 0; argument[start] != 0; start++ )
        {
            if (argument[start] == '\t')
            {
                send_to_char( "Sorry, TABs not allowed.\n\r", ch );
                return;
            }
        }
        strcat( buf, argument );
        strcat( buf, "\n\r" );
        free_string( ch->psong->text );
        ch->psong->text = str_dup( buf );
        send_to_char( "Ok.\n\r", ch );
        return;
    }
    if ( !str_prefix( arg, "title" ) )
    {
        song_attach( ch );
        free_string( ch->psong->title );
        ch->psong->title = str_dup( argument );
        send_to_char( "Ok.\n\r", ch );
        return;
    }
    if ( !str_prefix( arg, "clear" ) )
    {
        if ( ch->psong != NULL )
        {
            free_string( ch->psong->text );
            free_string( ch->psong->title );
            free_string( ch->psong->to_list );
            free_string( ch->psong->date );
            free_string( ch->psong->composer );
            ch->psong->next     = song_free;
            song_free           = ch->psong;
            ch->psong           = NULL;
        }
        send_to_char( "Ok.\n\r", ch );
        return;
    }
    if ( !str_prefix( arg, "show" ) || !str_prefix( arg, "pretty" ) )
    {
        if ( ch->psong == NULL )
        {
            send_to_char( "You have no song in progress.\n\r", ch );
            return;
        }
        argument = one_argument( argument, arg2 );
        argument = one_argument( argument, arg3 );
        start = 1;
        lines = 100;
        /* see if starting line spec'd */
        if( arg2[0] != 0 )
        {
            start = atoi(arg2);
            if( start < 1 ) start = 1;
        }
        /* see if number of lines spec'd */
        if( arg3[0] != 0 )
        {
            lines = atoi(arg3);
            if( lines < 1 ) lines = 1;
        }
        if( !str_prefix( arg, "show" ) )
            song_show( ch, start, lines );
        else
            song_pretty( ch, start, lines );
                
        return;
    }
    if ( !str_prefix( arg, "delete" ) )
    {
        if ( ch->psong == NULL )
        {
            send_to_char( "You have no song in progress.\n\r", ch );
            return;
        }
        lines = 1;
        argument = one_argument( argument, arg2 );
        argument = one_argument( argument, arg3 );
        if( arg2[0] == 0 )
        {
            send_to_char( "Usage: song delete <line> <<# to delete>>\n\r",
                ch );
            return;
        }
        start = atoi(arg2);
        if( start < 1 ) start = 1;
        if( arg3[0] != 0 )
        {
            lines = atoi(arg3);
            if( lines < 1 ) lines = 1;
        }
        song_delete( ch, start, lines );
        return;
    }
    if ( !str_prefix( arg, "insert" ) )
    {
        if ( ch->psong == NULL )
        {
            send_to_char( "You have no song in progress.\n\r", ch );
            return;
        }
        argument = one_argument( argument, arg2 );
        if( arg2[0] == 0 )
        {
            send_to_char( "Usage: song insert <line #> <<text>>\n\r", ch );
            return;
        }
        start = atoi(arg2);
        if( start < 1 || start > 100 )
        {
            send_to_char( "Invalid insert point.\n\r", ch );
            return;
        }
        if( *argument == '.' )
            argument++;
        song_insert( ch, start, argument );
        return;
  }
  if ( !str_prefix( arg, "compose" ) )
  {
    FILE *fp;
    char *strtime;

    if ( ch->psong == NULL )
    {
      send_to_char( "You have no song in progress.\n\r", ch );
      return;
    }

    ch->psong->to_list = str_dup(ch->name);

    if (!str_cmp(ch->psong->title,""))
    {
      send_to_char("You need to provide a title.\n\r",ch);
      return;
    }

    ch->psong->next                 = NULL;
    strtime                         = ctime( &current_time );
    strtime[strlen(strtime)-1]      = '\0';
    ch->psong->date                 = str_dup( strtime );
    ch->psong->date_stamp           = current_time;

    if ( song_list == NULL )
    {
      song_list   = ch->psong;
    }
    else
    {
      for ( psong = song_list; psong->next != NULL; psong = psong->next )
               ;
      psong->next = ch->psong;
    }

    psong           = ch->psong;
    ch->psong       = NULL;
    fclose( fpReserve );
    if ( ( fp = fopen( SONG_FILE, "a" ) ) == NULL )
    {
      perror( SONG_FILE );
    }
    else
    {
      fprintf( fp, "Composer  %s~\n", psong->composer);
      fprintf( fp, "Date      %s~\n", psong->date);
      fprintf( fp, "Stamp     %ld\n", psong->date_stamp);
      fprintf( fp, "To        %s~\n", psong->to_list);
      fprintf( fp, "Title     %s~\n", psong->title);
      fprintf( fp, "Recitals  0\n");
      fprintf( fp, "Text\n%s~\n", psong->text);
      fclose( fp );
    }
    fpReserve = fopen( NULL_FILE, "r" );
    send_to_char( "Ok.\n\r", ch );
    return;
  }
  if ( !str_prefix( arg, "remove" ) )
  {
    if ( !is_number( argument ) )
    {
      send_to_char( "Song remove which number?\n\r", ch );
      return;
    }
    anum = atoi( argument );
    vnum = 0;
    for ( psong = song_list; psong != NULL; psong = psong->next )
    {
      if ( knows_song( ch, psong ) && vnum++ == anum )
      {
        song_remove( ch, psong );
        send_to_char( "Ok.\n\r", ch );
        return;
      }
    }
    send_to_char( "No such song.\n\r", ch );
    return;
  }

  if ( !str_prefix( arg, "expunge" ) && get_trust(ch) >= MAX_LEVEL - 1)
  {
    if (!IS_IMPLEMENTOR(ch) )
    {
      send_to_char( "You do not have the ability to remove songs.\n\r",ch);
      return;
    }

    if ( !is_number( argument ) )
    {
      send_to_char( "Song expunge which number?\n\r", ch );
      return;
    }
 
    anum = atoi( argument );
    vnum = 0;
    for ( psong = song_list; psong != NULL; psong = psong->next )
    {
      if ( knows_song( ch, psong ) && vnum++ == anum )
      {
        song_expunge( psong );
        send_to_char( "Ok.\n\r", ch );
        return;
      }
    }
 
    send_to_char( "No such song.\n\r", ch );
    return;
  }
  send_to_char( "Huh?  Type 'help song' for usage.\n\r", ch );
  return;
}
/*
 *   song_show
 *   display song in progress with line numbers.
 */
void song_show( CHAR_DATA *ch, int start, int lines )
{
  char buf[MAX_STRING_LENGTH];
  char *p, *bp;
  int i;
  /* if showing at start, show all note info */
  if( start == 1 )
  {
    sprintf( buf, "%s: %s\n\r",
      ch->psong->composer,
      ch->psong->title);
    send_to_char( buf, ch );
  }
  /* find starting line to show */
  for( i = 1, p = ch->psong->text; *p != 0 && i < start; p++ )
  {
    if( *p == '\n' )
    {
      i++;
      p++;   /* skip '\r' */
    }
  }
  /* found starting line, show req'd number */
  for( i = 0; i < lines && *p != 0; i++)
  {
    sprintf( buf, "% 2d: ", i + start);
    bp = buf + 4;
    /* copy one line */
    while( *p && *p != '\r')
      *bp++ = *p++;
    if( *p == '\r' )
      *bp++ = *p++;
    *bp = 0;
    send_to_char( buf, ch );
  }
}
/*
 *   song_delete
 *   delete one or more lines of a song in progress.
 */
void song_delete( CHAR_DATA *ch, int start, int lines )
{
    char buf[MAX_STRING_LENGTH];
    char *p, *bp;
    int i;
    /* find starting line to delete */
    for( i = 1, p = ch->psong->text, bp = buf; *p != 0 && i < start; p++ )
    {
        *bp++ = *p;
        if( *p == '\r' )
        {
            i++;
        }
    }
    /* found starting line, delete req'd number */
    for( i = 0; *p != 0 && i < lines; p++)
        if( *p == '\r' ) i++;
    
    /* and now copy remainder of note */
    while( *p != 0 )
        *bp++ = *p++;
    *bp = 0;
    /* and swap in the new editted note */
    free_string( ch->psong->text );
    ch->psong->text = str_dup( buf );
    sprintf( buf, "Removed %d lines of song starting at line %d.\n\r",
        i, start );
    send_to_char( buf, ch );
}
/*  song_insert
 *  insert a line of text at a spec'd line.
 */
void song_insert( CHAR_DATA *ch, int start, char *text )
{
    char buf[MAX_STRING_LENGTH];
    char *p, *bp;
    int i;
    /* find line to insert at */
    for( i = 1, p = ch->psong->text, bp = buf; *p != 0 && i < start; p++ )
    {
        *bp++ = *p;
        if( *p == '\r' )
        {
            i++;
        }
    }
    /* found line, append new text */
    strcpy( bp, text );
    strcat( bp, "\n\r" );
    
    /* and now copy remainder of note */
    strcat( bp, p );
    /* and swap in the new edited note */
    free_string( ch->psong->text );
    ch->psong->text = str_dup( buf );
    send_to_char( "Ok.\n\r", ch );
}
/*
 *   song_pretty
 *   pretty-ify a song by filling or wrapping lines.  recognizes
 *   leading space or extra newline/cr as paragraph delimiter.
 */
void song_pretty( CHAR_DATA *ch, int start, int lines )
{
    char buf[MAX_STRING_LENGTH];
    char wordbuf[MAX_INPUT_LENGTH];
    char *p, *bp, *wp;
    int i,inword;
    /* find starting line to pretty-ify */
    for( i = 1, p = ch->psong->text, bp = buf; *p != 0 && i < start; p++ )
    {
        *bp++ = *p;
        if( *p == '\r' )
            i++;
    }
    *bp = 0;
    /* now build pretty lines from raw ones */
    pretty_proc( bp, NULL );            /* init pretty processor */
    for( i = inword = 0, wp = wordbuf; i < lines && *p != 0; p++ )
    {
        if( *p == ' ' )
        {
            if( inword )
            {
                inword = 0;
                *wp = 0;
                pretty_proc( NULL, wordbuf );
                wp = wordbuf;
            }
            *wp++ = ' ';
        }
        else if( *p == '\r' )
        {
            i++;                /* inc line count */
            if( inword )
            {
                inword = 0;
                *wp = 0;
                pretty_proc( NULL, wordbuf );
                wp = wordbuf;
                if( p[1] == '\n' || p[1] == ' ' || p[1] == 0)
                    pretty_proc( NULL, "\n\r" );
                else
                    pretty_proc( NULL, " " );
            }
            else
            {
                pretty_proc( NULL, "\n\r" );
                wp = wordbuf;
            }
        }
        else if( *p == '\n' )
            continue;
        else {
            inword = 1;
            *wp++ = *p;
      }
    }
    /* and append any leftover lines directly */
    strcat( buf, p );
    /* and swap in the new editted note */
    free_string( ch->psong->text );
    ch->psong->text = str_dup( buf );
    send_to_char( "Ok.\n\r", ch );
}

