/********************************************************************************
 * Email code copyright 1999-2001                                               *
 * Markanth : markanth@spaceservices.net                                        *
 * Devil's Lament : spaceservices.net port 3778                                 *
 * Web Page : http://spaceservices.net/~markanth/                               *
 *                                                                              *
 * Provides ability to send an email with your character from within a mud.     *
 * String parsing code based off roguemud.                                      *
 *                                                                              *
 * All I ask in return is that you give me credit on your mud somewhere         *
 * or email me if you use it, or if you make any changes.                       *
 ********************************************************************************/

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

const char *szEmailPrompt = "\n\r(V)iew, (S)end or (F)orget it?";
char *string_linedel( char *, int );
char *string_lineadd( char *, char *, int );
char *numlineas( char * );

bool valid_email ( char *argument )
{
    int i;
    bool pAnd = FALSE, pDot = FALSE;

    for ( i = 0; argument[i] != '\0'; i++ )
    {
        if ( argument[i] == '@' )
            pAnd = TRUE;
        else if ( argument[i] == '.' )
            pDot = TRUE;
    }
    if ( !pAnd || !pDot )
        return FALSE;
    else
        return TRUE;
}

void handle_email_to ( DESCRIPTOR_DATA * d, char *argument )
{
    CHAR_DATA *ch = ch ( d );

    if ( !ch || !ch->pcdata )
    {
        bug ( "handle_email_to: NULL pcdata", 0 );
        return;
    }
    if ( argument[0] == '\0' )
    {
        write_to_buffer ( d,
        " Please enter a valid e-mail address.\n\rType 'cancel' to cancel this letter.\n\r",
                          0 );
        return;
    }
    else if ( !str_cmp ( argument, "cancel" ) )
    {
        write_to_buffer ( d, "E-Mail cancelled.\n\r", 0 );
        free_email_data ( ch->pcdata->email_data );
        ch->pcdata->email_data = NULL;
        d->connected = CON_PLAYING;
        return;
    }
    else if ( !valid_email ( argument ) || strlen ( argument ) > 50 )
    {
        write_to_buffer ( d,
        " Please enter a valid e-mail address.\n\r\n\r",0 );
        write_to_buffer ( d,
        " Type 'cancel' to cancel this letter.\n\r",0 );
        return;
    }
    free_string( ch->pcdata->email_data->to );
	ch->pcdata->email_data->to = str_dup( argument );
    write_to_buffer ( d,
                      "\n\r E-Mail address set.\n\r", 0 );
    write_to_buffer ( d,
                      "\n\r Please enter a subject for your letter:\n\r", 0 );
    d->connected = CON_EMAIL_SUBJECT;
    return;
}

void handle_email_subject ( DESCRIPTOR_DATA * d, char *argument )
{
    CHAR_DATA *ch = ch ( d );

    if ( !ch || !ch->pcdata )
    {
        bug ( "handle_email_subject: NULL pcdata", 0 );
        return;
    }
    if ( argument[0] == '\0' )
    {
        write_to_buffer ( d,
                          " Please enter a subject for your E-Mail.\n\rType 'cancel' to cancel this letter.\n\r",
                          0 );
        return;
    }
    else if ( !str_cmp ( argument, "cancel" ) )
    {
        write_to_buffer ( d, "E-Mail cancelled.\n\r", 0 );
        free_email_data ( ch->pcdata->email_data );
        ch->pcdata->email_data = NULL;
        d->connected = CON_PLAYING;
        return;
    }
    else if ( strlen ( argument ) > 50 )
    {
        write_to_buffer ( d,
        "No, no. This is just the Subject. You're not writing the note yet.\n\r",0 );
        return;
    }
    free_string( ch->pcdata->email_data->subject );
	ch->pcdata->email_data->subject = str_dup( argument );
    write_to_buffer ( d,
                      "   Subject set.\n\r\n\r** Please enter the body of your e-mail.\n\r",0 );
    write_to_buffer ( d, "\n\r"
    "   You have entered the e-mail editor. This can be confusing,          \n\r"
    "   especially if you haven't even figured out how to write notes       \n\r"
    "   properly. If that's the case, type .q and then hit RETURN to        \n\r"
    "   exit this editor, then type F to EXPUNGE your email. If you         \n\r"
    "   wish to continue, this is a quick DESC on how it works. Also        \n\r"
    "   BE sure to read 'HELP SENDMAIL' for more detailed instructions      \n\r"
    "   On a fresh line, hit the RETURN key to enter the text editor.       \n\r"
    "   This is where you will write your message. Once you are finished    \n\r"
    "   writing your email, type @ on a fresh line to EXIT the email        \n\r"
    "   text editor. This will then bring you directly back to here.        \n\r"
    "   Commands here are .h/.c/.f/.s/.q  or just hit RETURN. ALWAYS        \n\r"
    "   remember to type .q if here to exit completely. Again, read         \n\r"
    "   the help file before you use this feature.              	    \n\r"
    "\n\r"
    "   ( Hit RETURN to entire email text editor, or type .q to quit! )\n\r"
    "=----=----=----=----=----=----=----=----=----=----=----=----=----=----=\n\r", 0 );
    d->connected = CON_EMAIL_BODY;
    return;
}

void parse_email_text ( int command, char *string, CHAR_DATA * ch )
{
    switch ( command )
    {
        case PARSE_HELP:
            send_to_char
             ( "=======================================================\n\r", ch );
            send_to_char ( "Edit help (commands on blank line):\n\r", ch );
            send_to_char ( ".h               - get help (this info)\n\r", ch );
            send_to_char ( ".s               - show string so far\n\r", ch );
            send_to_char ( ".f               - (word wrap) string\n\r", ch );
            send_to_char ( ".c               - clear string so far\n\r", ch );
            send_to_char ( ".q               - Exit e-mail editor to send\n\r", ch );
            send_to_char ( "Or, just hit RETURN to edit your email message!\n\r", ch );
            send_to_char
                ( "=======================================================\n\r", ch );
            break;
        case PARSE_LIST_NORM:
            send_to_char ( "String so far:\n\r", ch );
            page_to_char ( ch->pcdata->email_data->body, ch );
            break;
        default:
            send_to_char ( "Invalid command.\n\r", ch );
            bugf ( "invalid command passed to parse_email_text" );
            break;
    }
}

void handle_email_body ( DESCRIPTOR_DATA * d, char *argument )
{
    bool terminate = FALSE;
    int i = 2, j = 0, action = 0;
    CHAR_DATA *ch = d->character;
    char buf[MAX_STRING_LENGTH];
    char actions[MAX_INPUT_LENGTH];

    if ( !ch->pcdata->email_data )
    {
        d->connected = CON_PLAYING;
        bugf ( "nanny: In CON_EMAIL_TEXT, buf no email in progress." );
        return;
    }

    if ( ( action = ( *argument == '.' ) ) )
    {
        while ( argument[i] != '\0' )
        {
            actions[j] = argument[i];
            i++;
            j++;
        }
        actions[j] = '\0';
        *argument = '\0';
        switch ( argument[1] )
        {
            case 'q':
                terminate = TRUE;
                *argument = '\0';
                break;
            case 'c':
                send_to_char ( "String cleared.\n\r", ch );
                free_string( ch->pcdata->email_data->body );
	        ch->pcdata->email_data->body = str_dup( "" );
                return;
            case 's':
                parse_email_text ( PARSE_LIST_NORM, actions, ch );
                return;
            case 'f':
                ch->pcdata->email_data->body =
                    format_string ( ch->pcdata->email_data->body );
                send_to_char ( "String formatted.\n\r", ch );
                return;
            case 'h':
                parse_email_text ( PARSE_HELP, actions, ch );
                return;
            default:
                send_to_char ( "Invalid command.\n\r", ch );
                return;
        }
    }

    if ( terminate )
    {
        printf_to_char ( ch, "\n\r%s", szEmailPrompt );
        d->connected = CON_EMAIL_FINISH;
        return;
    }

    if ( ( strlen ( argument ) + strlen ( buf ) ) > MAX_EMAIL_TEXT )
    {
        send_to_char ( "E-mail too long, bailing out!\n\r", ch );
        free_email_data ( ch->pcdata->email_data );
        ch->pcdata->email_data = NULL;
        d->connected = CON_PLAYING;
        return;
    }

    SET_BIT(ch->comm,COMM_EDITOR);
    string_append( ch, &ch->pcdata->email_data->body );

}

void handle_email_finish ( DESCRIPTOR_DATA * d, char *argument )
{
    CHAR_DATA *ch = d->character;

    if ( !ch->pcdata->email_data )
    {
        d->connected = CON_PLAYING;
        bugf ( "nanny: In CON_EMAIL_FINISH, but no email in progress." );
        return;
    }

    switch ( argument[0] )
    {
        case 'V':
        case 'v':              /* view note so far */
            if ( ch->pcdata->email_data->body )
            {
                send_to_char ( "Your note so far:\n\r", ch );
                printf_to_char ( ch, "To: %s\n\r", ch->pcdata->email_data->to );
                printf_to_char ( ch, "SUbject: %s\n\r",
                ch->pcdata->email_data->subject );
                page_to_char ( ch->pcdata->email_data->body, ch );
                write_to_buffer ( d, "\r\n", 0 );
            }
            else
                write_to_buffer ( d, "You haven't written a thing!\n\r\n\r",0 );
            send_to_char ( szEmailPrompt, ch );
            break;
        case 'S':
        case 's':
            if ( emailf ( ch ) )
                write_to_buffer ( d, "E-Mail sent.\n\r", 0 );
            else
                write_to_buffer ( d,
                "There was a problem sending your e-mail, contact an Imm.\n\r", 0 );
            d->connected = CON_PLAYING;
            free_email_data ( ch->pcdata->email_data );
            ch->pcdata->email_data = NULL;
            act ( "$n finishes sending $s e-mail.", ch, NULL, NULL,TO_ROOM );
            break;
        case 'F':
        case 'f':
            write_to_buffer ( d, "Email cancelled!\n\r", 0 );
            free_email_data ( ch->pcdata->email_data );
            ch->pcdata->email_data = NULL;
            d->connected = CON_PLAYING;
            act ( "$n cancels $s e-mail.", ch, NULL, NULL, TO_ROOM );
            break;
        default:
            write_to_buffer ( d, "Huh? Valid answers are:\n\r\n\r", 0 );
            send_to_char ( szEmailPrompt, ch );
    }
}

void do_sendmail (CHAR_DATA *ch, char *argument)
{
    if ( !ch->desc || !ch->pcdata )
        return;

    free_email_data ( ch->pcdata->email_data );
    send_to_char ( "Please enter the E-Mail address you wish to send to:\n\r",ch );
    ch->pcdata->email_data = new_email_data (  );
    ch->desc->connected = CON_EMAIL_TO;
    return;
}

#define MUD_NAME "The Legends of Krynn"

bool emailf ( CHAR_DATA * ch )
{
    FILE *mail;
    char buf[MAX_INPUT_LENGTH];
    EMAIL_DATA *pMail;

    if ( !ch || !ch->pcdata )
        return FALSE;

    if ( !( pMail = ch->pcdata->email_data ) )
        return FALSE;

    if ( IS_NULLSTR ( pMail->to ) || IS_NULLSTR ( pMail->body ) )
        return FALSE;

    sprintf ( buf, "/usr/sbin/sendmail -t -i -F%s", ch->name );

    if ( ( mail = popen ( buf, "w" ) ) == NULL )
    {
        bug ( "Error opening sendmail", 0 );
        return FALSE;
    }

    fprintf ( mail, "To: %s\n", pMail->to );
    fprintf ( mail, "Reply-to: %s\n",
    IS_NULLSTR ( ch->pcdata->email ) ? "waltz@velocity.wolfpaw.net" : ch->pcdata->email );
    fprintf ( mail, "Subject: %s\n\n",
              !IS_NULLSTR ( pMail->subject ) ? pMail->
              subject : "Mail from " MUD_NAME "" );
    fprintf ( mail, "%s", pMail->body );
    fprintf ( mail, "\n------[ " MUD_NAME " ]------\n" );
    fprintf ( mail, "telnet://legendsofkrynn.wolfpaw.net:6100\n" );
    fprintf ( mail, "http://legendsofkrynn.wolfpaw.net\n" );
    fprintf ( mail, "-------------------------------------------------\n" );
    pclose ( mail );
    return TRUE;
}

