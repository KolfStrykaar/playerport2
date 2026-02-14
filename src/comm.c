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
 *  Thanks to abaddon for proof-reading our comm.c and pointing out bugs.  *
 *  Any remaining bugs are, of course, our work, not his.  :)              *
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

/*
 * This file contains all of the OS-dependent stuff:
 *   startup, signals, BSD sockets for tcp/ip, i/o, timing.
 *
 * The data flow for input is:
 *    Game_loop ---> Read_from_descriptor ---> Read
 *    Game_loop ---> Read_from_buffer
 *
 * The data flow for output is:
 *    Game_loop ---> Process_Output ---> Write_to_descriptor -> Write
 *
 * The OS-dependent functions are Read_from_descriptor and Write_to_descriptor.
 * -- Furey  26 Jan 1993
 */

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h> /* OLC -- for close read write etc */
#include <stdarg.h> /* printf_to_char */

#include "merc.h"
#include "interp.h"
#include "recycle.h"
#include "tables.h"

/*
 * Malloc debugging stuff.
 */
#if defined(sun)
#undef MALLOC_DEBUG
#endif

#if defined(MALLOC_DEBUG)
#include <malloc.h>
extern	int	malloc_debug	args( ( int  ) );
extern	int	malloc_verify	args( ( void ) );
#endif



/*
 * Signal handling.
 * Apollo has a problem with __attribute(atomic) in signal.h,
 *   I dance around it.
 */
#if defined(apollo)
#define __attribute(x)
#endif

#if defined(unix)
#include <signal.h>
#endif

#if defined(apollo)
#undef __attribute
#endif



/*
 * Socket and TCP/IP stuff.
 */
#if	defined(macintosh) || defined(MSDOS)
const	char	echo_off_str	[] = { '\0' };
const	char	echo_on_str	[] = { '\0' };
const	char 	go_ahead_str	[] = { '\0' };
#endif

#if	defined(unix)
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "telnet.h"
const	char	echo_off_str	[] = { IAC, WILL, TELOPT_ECHO, '\0' };
const	char	echo_on_str	[] = { IAC, WONT, TELOPT_ECHO, '\0' };
const	char 	go_ahead_str	[] = { IAC, GA, '\0' };
#endif



/*
 * OS-dependent declarations.
 */
#if	defined(_AIX)
#include <sys/select.h>
int	accept		args( ( int s, struct sockaddr *addr, int *addrlen ) );
int	bind		args( ( int s, struct sockaddr *name, int namelen ) );
void	bzero		args( ( char *b, int length ) );
int	getpeername	args( ( int s, struct sockaddr *name, int *namelen ) );
int	getsockname	args( ( int s, struct sockaddr *name, int *namelen ) );
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
int	listen		args( ( int s, int backlog ) );
int	setsockopt	args( ( int s, int level, int optname, void *optval,
			    int optlen ) );
int	socket		args( ( int domain, int type, int protocol ) );
#endif

#if	defined(apollo)
#include <unistd.h>
void	bzero		args( ( char *b, int length ) );
#endif

#if	defined(__hpux)
int	accept		args( ( int s, void *addr, int *addrlen ) );
int	bind		args( ( int s, const void *addr, int addrlen ) );
void	bzero		args( ( char *b, int length ) );
int	getpeername	args( ( int s, void *addr, int *addrlen ) );
int	getsockname	args( ( int s, void *name, int *addrlen ) );
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
int	listen		args( ( int s, int backlog ) );
int	setsockopt	args( ( int s, int level, int optname,
 				const void *optval, int optlen ) );
int	socket		args( ( int domain, int type, int protocol ) );
#endif

#if	defined(interactive)
#include <net/errno.h>
#include <sys/fnctl.h>
#endif

#if	defined(linux)
/* 
    Linux shouldn't need these. If you have a problem compiling, try
    uncommenting these functions.
*/
/*
int	getpeername	args( ( int s, struct sockaddr *name, int *namelen ) );
int	getsockname	args( ( int s, struct sockaddr *name, int *namelen ) );
int	listen		args( ( int s, int backlog ) );
*/

int	close		args( ( int fd ) );
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
/* int	read		args( ( int fd, char *buf, int nbyte ) ); */
int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
int	socket		args( ( int domain, int type, int protocol ) );
/* int	write		args( ( int fd, char *buf, int nbyte ) ); */ /* read,write in unistd.h */
#endif

#if	defined(macintosh)
#include <console.h>
#include <fcntl.h>
#include <unix.h>


struct	timeval
{
	time_t	tv_sec;
	time_t	tv_usec;
};
#if	!defined(isascii)
#define	isascii(c)		( (c) < 0200 )
#endif
static	long			theKeys	[4];

int	gettimeofday		args( ( struct timeval *tp, void *tzp ) );
#endif

#if	defined(MIPS_OS)
extern	int		errno;
#endif

#if	defined(MSDOS)
int	gettimeofday	args( ( struct timeval *tp, void *tzp ) );
int	kbhit		args( ( void ) );
#endif

#if	defined(NeXT)
int	close		args( ( int fd ) );
int	fcntl		args( ( int fd, int cmd, int arg ) );
#if	!defined(htons)
u_short	htons		args( ( u_short hostshort ) );
#endif
#if	!defined(ntohl)
u_long	ntohl		args( ( u_long hostlong ) );
#endif
int	read		args( ( int fd, char *buf, int nbyte ) );
int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
int	write		args( ( int fd, char *buf, int nbyte ) );
#endif

#if	defined(sequent)
int	accept		args( ( int s, struct sockaddr *addr, int *addrlen ) );
int	bind		args( ( int s, struct sockaddr *name, int namelen ) );
int	close		args( ( int fd ) );
int	fcntl		args( ( int fd, int cmd, int arg ) );
int	getpeername	args( ( int s, struct sockaddr *name, int *namelen ) );
int	getsockname	args( ( int s, struct sockaddr *name, int *namelen ) );
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
#if	!defined(htons)
u_short	htons		args( ( u_short hostshort ) );
#endif
int	listen		args( ( int s, int backlog ) );
#if	!defined(ntohl)
u_long	ntohl		args( ( u_long hostlong ) );
#endif
int	read		args( ( int fd, char *buf, int nbyte ) );
int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
int	setsockopt	args( ( int s, int level, int optname, caddr_t optval,
			    int optlen ) );
int	socket		args( ( int domain, int type, int protocol ) );
int	write		args( ( int fd, char *buf, int nbyte ) );
#endif

/* This includes Solaris Sys V as well */
#if defined(sun)
int	accept		args( ( int s, struct sockaddr *addr, int *addrlen ) );
int	bind		args( ( int s, struct sockaddr *name, int namelen ) );
void	bzero		args( ( char *b, int length ) );
int	close		args( ( int fd ) );
int	getpeername	args( ( int s, struct sockaddr *name, int *namelen ) );
int	getsockname	args( ( int s, struct sockaddr *name, int *namelen ) );
int	listen		args( ( int s, int backlog ) );
int	read		args( ( int fd, char *buf, int nbyte ) );
int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );

#if !defined(__SVR4)
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );

#if defined(SYSV)
int setsockopt		args( ( int s, int level, int optname,
			    const char *optval, int optlen ) );
#else
int	setsockopt	args( ( int s, int level, int optname, void *optval,
			    int optlen ) );
#endif
#endif
int	socket		args( ( int domain, int type, int protocol ) );
int	write		args( ( int fd, char *buf, int nbyte ) );
#endif

#if defined(ultrix)
int	accept		args( ( int s, struct sockaddr *addr, int *addrlen ) );
int	bind		args( ( int s, struct sockaddr *name, int namelen ) );
void	bzero		args( ( char *b, int length ) );
int	close		args( ( int fd ) );
int	getpeername	args( ( int s, struct sockaddr *name, int *namelen ) );
int	getsockname	args( ( int s, struct sockaddr *name, int *namelen ) );
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
int	listen		args( ( int s, int backlog ) );
int	read		args( ( int fd, char *buf, int nbyte ) );
int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
int	setsockopt	args( ( int s, int level, int optname, void *optval,
			    int optlen ) );
int	socket		args( ( int domain, int type, int protocol ) );
int	write		args( ( int fd, char *buf, int nbyte ) );
#endif



/*
 * Global variables.
 */
DESCRIPTOR_DATA *   descriptor_list;	/* All open descriptors		*/
DESCRIPTOR_DATA *   d_next;		/* Next descriptor in loop	*/
FILE *		    fpReserve;		/* Reserved file handle		*/
bool		    god;		/* All new chars are gods!	*/
bool		    merc_down;		/* Shutdown			*/
bool		    wizlock;		/* Game is wizlocked		*/
bool		    newlock;		/* Game is newlocked		*/
char		    str_boot_time[MAX_INPUT_LENGTH];
time_t		    current_time;	/* time of this pulse */	
bool		    MOBtrigger = TRUE;  /* act() switch                 */


/*
 * OS-dependent local functions.
 */
#if defined(macintosh) || defined(MSDOS)
void	game_loop_mac_msdos	args( ( void ) );
bool	read_from_descriptor	args( ( DESCRIPTOR_DATA *d ) );
bool	write_to_descriptor	args( ( int desc, char *txt, int length ) );
#endif

#if defined(unix)
void	game_loop_unix		args( ( int control ) );
int	init_socket			args( ( int port ) );
void	init_descriptor		args( ( int control ) );
bool	read_from_descriptor	args( ( DESCRIPTOR_DATA *d ) );
bool	write_to_descriptor	args( ( int desc, char *txt, int length ) );
void  send_to_desc2		args( ( const char *txt, DESCRIPTOR_DATA *d ) );
#endif




/*
 * Other local functions (OS-independent).
 */
char *initial( const char *str );
bool	check_parse_name		args( ( char *name ) );
bool  check_parse_surname     args( ( char *name ) );
bool	check_illegal_name	args( ( char *name ) );
bool	check_reconnect		args( ( DESCRIPTOR_DATA *d, char *name,bool fConn ) );
bool	check_playing		args( ( DESCRIPTOR_DATA *d, char *name ) );
int	main				args( ( int argc, char **argv ) );
void	nanny				args( ( DESCRIPTOR_DATA *d, char *argument ) );
bool	process_output		args( ( DESCRIPTOR_DATA *d, bool fPrompt ) );
void	read_from_buffer		args( ( DESCRIPTOR_DATA *d ) );
void	stop_idling			args( ( CHAR_DATA *ch ) );
void  bust_a_prompt           args( ( CHAR_DATA *ch ) );
void  init_signals            args( ( void ) );
void  do_auto_shutdown        args( ( void ) );
void  reverse_word            args( ( char *w, int n ) );


/* Needs to be global because of do_copyover */
int port, control;

int main( int argc, char **argv )
{
    struct timeval now_time;

    bool fCopyOver = FALSE;

    /*
     * Memory debugging if needed.
     */
#if defined(MALLOC_DEBUG)
    malloc_debug( 2 );
#endif

    /*
     * Init time.
     */

    gettimeofday( &now_time, NULL );
    current_time 	= (time_t) now_time.tv_sec-21600;
    strcpy( str_boot_time, ctime( &current_time ) );

    /*
     * Macintosh console initialization.
     */
#if defined(macintosh)
    console_options.nrows = 31;
    cshow( stdout );
    csetmode( C_RAW, stdin );
    cecho2file( "log file", 1, stderr );
#endif

    /*
     * Reserve one channel for our use.
     */
    if ( ( fpReserve = fopen( NULL_FILE, "r" ) ) == NULL )
    {
	perror( NULL_FILE );
	exit( 1 );
    }

    /*
     * Get the port number.
     */
    port = 4000;
    if ( argc > 1 )
    {
	if ( !is_number( argv[1] ) )
	{
	    fprintf( stderr, "Usage: %s [port #]\n", argv[0] );
	    exit( 1 );
	}
	else if ( ( port = atoi( argv[1] ) ) <= 1024 )
	{
	    fprintf( stderr, "Port number must be above 1024.\n" );
	    exit( 1 );
	}
	
  	/* Are we recovering from a copyover? */
  	if (argv[2] && argv[2][0])
  	{
  		fCopyOver = TRUE;
  		control = atoi(argv[3]);
  	}
  	else
  		fCopyOver = FALSE;
 	
    }

    /*
     * Run the game.
     */
#if defined(macintosh) || defined(MSDOS)
    boot_db();
    log_string( "Merc is ready to rock." );
    game_loop_mac_msdos( );
#endif

#if defined(unix)
 
 	if (!fCopyOver)
 	    control = init_socket( port );
 	    
     boot_db();
    sprintf( log_buf, "ROM is ready to rock on port %d.", port );
    log_string( log_buf );
     
	/* Check to see if we need to backup pfiles */
	do_function(NULL, &do_pfile_backup, "");

     if (fCopyOver)
     	copyover_recover();
     
    game_loop_unix( control );
    close (control);
#endif

    /*
     * That's all, folks.
     */
    log_string( "Normal termination of game." );
    exit( 0 );
    return 0;
}



#if defined(unix)
int init_socket( int port )
{
    static struct sockaddr_in sa_zero;
    struct sockaddr_in sa;
    int x = 1;
    int fd;

    if ( ( fd = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 )
    {
	perror( "Init_socket: socket" );
	exit( 1 );
    }

    if ( setsockopt( fd, SOL_SOCKET, SO_REUSEADDR,
    (char *) &x, sizeof(x) ) < 0 )
    {
	perror( "Init_socket: SO_REUSEADDR" );
	close(fd);
	exit( 1 );
    }

#if defined(SO_DONTLINGER) && !defined(SYSV)
    {
	struct	linger	ld;

	ld.l_onoff  = 1;
	ld.l_linger = 1000;

	if ( setsockopt( fd, SOL_SOCKET, SO_DONTLINGER,
	(char *) &ld, sizeof(ld) ) < 0 )
	{
	    perror( "Init_socket: SO_DONTLINGER" );
	    close(fd);
	    exit( 1 );
	}
    }
#endif

    sa		    = sa_zero;
    sa.sin_family   = AF_INET;
    sa.sin_port	    = htons( port );
    sa.sin_addr.s_addr = inet_addr("176.9.151.147");

    if ( bind( fd, (struct sockaddr *) &sa, sizeof(sa) ) < 0 )
    {
	perror("Init socket: bind" );
	close(fd);
	exit(1);
    }


    if ( listen( fd, 3 ) < 0 )
    {
	perror("Init socket: listen");
	close(fd);
	exit(1);
    }

    return fd;
}
#endif



#if defined(macintosh) || defined(MSDOS)
void game_loop_mac_msdos( void )
{
    struct timeval last_time;
    struct timeval now_time;
    static DESCRIPTOR_DATA dcon;

    gettimeofday( &last_time, NULL );
    current_time = (time_t) last_time.tv_sec;

    /*
     * New_descriptor analogue.
     */
    dcon.descriptor	= 0;
    dcon.connected	= CON_ANSI;
    dcon.host		= str_dup( "localhost" );
    dcon.outsize		= 2000;
    dcon.outbuf		= alloc_mem( dcon.outsize );
    dcon.next		= descriptor_list;
    dcon.showstr_head	= NULL;
    dcon.showstr_point	= NULL;
    dcon.pEdit		= NULL;			/* OLC */
    dcon.pString		= NULL;			/* OLC */
    dcon.editor		= 0;				/* OLC */
    descriptor_list	= &dcon;

    /*
     * Send the greeting.
     */
    {
	extern char * help_greeting;
	if ( help_greeting[0] == '.' )
	    write_to_buffer( &dcon, help_greeting+1, 0 );
	else
	    write_to_buffer( &dcon, help_greeting  , 0 );
    }

    /* Main loop */
    while ( !merc_down )
    {
	DESCRIPTOR_DATA *d;

	/*
	 * Process input.
	 */
	for ( d = descriptor_list; d != NULL; d = d_next )
	{
	    d_next	= d->next;
	    d->fcommand	= FALSE;

#if defined(MSDOS)
	    if ( kbhit( ) )
#endif
	    {
		if ( d->character != NULL )
		    d->character->timer = 0;
		if ( !read_from_descriptor( d ) )
		{
		    if ( d->character != NULL && d->connected == CON_PLAYING)
			save_char_obj( d->character );
		    d->outtop	= 0;
		    close_socket( d );
		    continue;
		}
	    }

	    if (d->character != NULL && d->character->daze > 0)
		--d->character->daze;

	    if ( d->character != NULL && d->character->wait > 0 )
	    {
		--d->character->wait;
		continue;
	    }

	    read_from_buffer( d );
	    if ( d->incomm[0] != '\0' )
	    {
		d->fcommand	= TRUE;
		stop_idling( d->character );

	        /* OLC */
	        if ( d->showstr_point )
	            show_string( d, d->incomm );
	        else
	        if ( d->pString )
	            string_add( d->character, d->incomm );
	        else
	            switch ( d->connected )
	            {
	                case CON_PLAYING:
			    if ( !run_olc_editor( d ) )
				substitute_alias( d, d->incomm );
			    break;
	                default:
			    nanny( d, d->incomm );
			    break;
	            }

		d->incomm[0]	= '\0';
	    }
	}



	/*
	 * Autonomous game motion.
	 */
	update_handler( );



	/*
	 * Output.
	 */
	for ( d = descriptor_list; d != NULL; d = d_next )
	{
	    d_next = d->next;

	    if ( ( d->fcommand || d->outtop > 0 ) )
	    {
		if ( !process_output( d, TRUE ) )
		{
		    if ( d->character != NULL && d->connected == CON_PLAYING)
			save_char_obj( d->character );
		    d->outtop	= 0;
		    close_socket( d );
		}
	    }
	}



	/*
	 * Synchronize to a clock.
	 * Busy wait (blargh).
	 */
	now_time = last_time;
	for ( ; ; )
	{
	    int delta;

#if defined(MSDOS)
	    if ( kbhit( ) )
#endif
	    {
		if ( dcon.character != NULL )
		    dcon.character->timer = 0;
		if ( !read_from_descriptor( &dcon ) )
		{
		    if ( dcon.character != NULL && d->connected == CON_PLAYING)
			save_char_obj( d->character );
		    dcon.outtop	= 0;
		    close_socket( &dcon );
		}
#if defined(MSDOS)
		break;
#endif
	    }

	    gettimeofday( &now_time, NULL );
	    delta = ( now_time.tv_sec  - last_time.tv_sec  ) * 1000 * 1000
		  + ( now_time.tv_usec - last_time.tv_usec );
	    if ( delta >= 1000000 / PULSE_PER_SECOND )
		break;
	}
	last_time    = now_time;
	current_time = (time_t) last_time.tv_sec;
    }

    return;
}
#endif



#if defined(unix)
void game_loop_unix( int control )
{
    static struct timeval null_time;
    struct timeval last_time;

    signal( SIGPIPE, SIG_IGN );
    gettimeofday( &last_time, NULL );
    current_time = (time_t) last_time.tv_sec;

    init_signals();

    /* Main loop */
    while ( !merc_down )
    {
	fd_set in_set;
	fd_set out_set;
	fd_set exc_set;
	DESCRIPTOR_DATA *d;
	int maxdesc;

#if defined(MALLOC_DEBUG)
	if ( malloc_verify( ) != 1 )
	    abort( );
#endif

	/*
	 * Poll all active descriptors.
	 */
	FD_ZERO( &in_set  );
	FD_ZERO( &out_set );
	FD_ZERO( &exc_set );
	FD_SET( control, &in_set );
	maxdesc	= control;
	for ( d = descriptor_list; d; d = d->next )
	{
	    maxdesc = UMAX( maxdesc, d->descriptor );
	    FD_SET( d->descriptor, &in_set  );
	    FD_SET( d->descriptor, &out_set );
	    FD_SET( d->descriptor, &exc_set );
	}

	if ( select( maxdesc+1, &in_set, &out_set, &exc_set, &null_time ) < 0 )
	{
	    perror( "Game_loop: select: poll" );
	    exit( 1 );
	}

	/*
	 * New connection?
	 */
	if ( FD_ISSET( control, &in_set ) )
	    init_descriptor( control );

	/*
	 * Kick out the freaky folks.
	 */
	for ( d = descriptor_list; d != NULL; d = d_next )
	{
	    d_next = d->next;   
	    if ( FD_ISSET( d->descriptor, &exc_set ) )
	    {
		FD_CLR( d->descriptor, &in_set  );
		FD_CLR( d->descriptor, &out_set );
		if ( d->character && d->connected == CON_PLAYING)
		    save_char_obj( d->character );
		d->outtop	= 0;
		close_socket( d );
	    }
	}

	/*
	 * Process input.
	 */
	for ( d = descriptor_list; d != NULL; d = d_next )
	{
	    d_next	= d->next;
	    d->fcommand	= FALSE;

	    if ( FD_ISSET( d->descriptor, &in_set ) )
	    {
		if ( d->character != NULL )
		    d->character->timer = 0;
		if ( !read_from_descriptor( d ) )
		{
		    FD_CLR( d->descriptor, &out_set );
		    if ( d->character != NULL && d->connected == CON_PLAYING)
			save_char_obj( d->character );
		    d->outtop	= 0;
		    close_socket( d );
		    continue;
		}
	    }

	    if (d->character != NULL && d->character->daze > 0)
		--d->character->daze;

	    if ( d->character != NULL && d->character->wait > 0 )
	    {
		--d->character->wait;
		continue;
	    }

	    read_from_buffer( d );
	    if ( d->incomm[0] != '\0' )
	    {
		d->fcommand	= TRUE;
		stop_idling( d->character );

	/* OLC */
	if ( d->showstr_point )
	    show_string( d, d->incomm );
	else
	if ( d->pString )
	    string_add( d->character, d->incomm );
	else
	    switch ( d->connected )
	    {
	        case CON_PLAYING:
		    if ( !run_olc_editor( d ) )
    		        substitute_alias( d, d->incomm );
		    break;
	        default:
		    nanny( d, d->incomm );
		    break;
	    }

		d->incomm[0]	= '\0';
	    }
	}



	/*
	 * Autonomous game motion.
	 */
	update_handler( );



	/*
	 * Output.
	 */
	for ( d = descriptor_list; d != NULL; d = d_next )
	{
	    d_next = d->next;

	    if ( ( d->fcommand || d->outtop > 0 )
	    &&   FD_ISSET(d->descriptor, &out_set) )
	    {
		if ( !process_output( d, TRUE ) )
		{
		    if ( d->character != NULL && d->connected == CON_PLAYING)
			save_char_obj( d->character );
		    d->outtop	= 0;
		    close_socket( d );
		}
	    }
	}



	/*
	 * Synchronize to a clock.
	 * Sleep( last_time + 1/PULSE_PER_SECOND - now ).
	 * Careful here of signed versus unsigned arithmetic.
	 */
	{
	    struct timeval now_time;
	    long secDelta;
	    long usecDelta;

	    gettimeofday( &now_time, NULL );
	    usecDelta	= ((int) last_time.tv_usec) - ((int) now_time.tv_usec)
			+ 1000000 / PULSE_PER_SECOND;
	    secDelta	= ((int) last_time.tv_sec ) - ((int) now_time.tv_sec );
	    while ( usecDelta < 0 )
	    {
		usecDelta += 1000000;
		secDelta  -= 1;
	    }

	    while ( usecDelta >= 1000000 )
	    {
		usecDelta -= 1000000;
		secDelta  += 1;
	    }

	    if ( secDelta > 0 || ( secDelta == 0 && usecDelta > 0 ) )
	    {
		struct timeval stall_time;

		stall_time.tv_usec = usecDelta;
		stall_time.tv_sec  = secDelta;
		if ( select( 0, NULL, NULL, NULL, &stall_time ) < 0 )
		{
		    perror( "Game_loop: select: stall" );
		    exit( 1 );
		}
	    }
	}

	gettimeofday( &last_time, NULL );
	current_time = (time_t) last_time.tv_sec;
    }

    return;
}
#endif



#if defined(unix)
void init_descriptor( int control )
{
    char color[] = "Color?";
    static char * codes [] = { "M", "B", "G", "Y", "C", "X" };
    int a;
    long pulse_color_anim = 1;


    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *dnew;
    struct sockaddr_in sock;
    struct hostent *from;
    int desc;
    unsigned int size;

    size = sizeof(sock);
    getsockname( control, (struct sockaddr *) &sock, &size );
    if ( ( desc = accept( control, (struct sockaddr *) &sock, &size) ) < 0 )
    {
	perror( "New_descriptor: accept" );
	return;
    }

#if !defined(FNDELAY)
#define FNDELAY O_NDELAY
#endif

    if ( fcntl( desc, F_SETFL, FNDELAY ) == -1 )
    {
	perror( "New_descriptor: fcntl: FNDELAY" );
	return;
    }

    /*
     * Cons a new descriptor.
     */
    dnew = new_descriptor(); /* new_descriptor now also allocates things */
    dnew->descriptor 	= desc;
    dnew->connected	= CON_ANSI;
    dnew->pEdit		= NULL;			/* OLC */
    dnew->pString		= NULL;			/* OLC */
    dnew->editor		= 0;				/* OLC */


    size = sizeof(sock);
    if ( getpeername( desc, (struct sockaddr *) &sock, &size ) < 0 )
    {
	perror( "New_descriptor: getpeername" );
	dnew->host = str_dup( "(unknown)" );
    }
    else
    {
	/*
	 * Would be nice to use inet_ntoa here but it takes a struct arg,
	 * which ain't very compatible between gcc and system libraries.
	 */
	int addr;

	addr = ntohl( sock.sin_addr.s_addr );
	sprintf( buf, "%d.%d.%d.%d",
	    ( addr >> 24 ) & 0xFF, ( addr >> 16 ) & 0xFF,
	    ( addr >>  8 ) & 0xFF, ( addr       ) & 0xFF
	    );
	sprintf( log_buf, "Sock.sinaddr:  %s", buf );
	log_string( log_buf );
	from = gethostbyaddr( (char *) &sock.sin_addr,
	    sizeof(sock.sin_addr), AF_INET );
	dnew->host = str_dup( from ? from->h_name : buf );
    }

    /*
     * Swiftest: I added the following to ban sites.  I don't
     * endorse banning of sites, but Copper has few descriptors now
     * and some people from certain sites keep abusing access by
     * using automated 'autodialers' and leaving connections hanging.
     *
     * Furey: added suffix check by request of Nickel of HiddenWorlds.
     */
    if ( check_ban(dnew->host,BAN_ALL))
    {
	write_to_descriptor( desc,
	    "Your site has been banned from this mud.\n\r", 0 );
	close( desc );
	free_descriptor(dnew);
	return;
    }
    /*
     * Init descriptor data.
     */
    dnew->next			= descriptor_list;
    descriptor_list		= dnew;

    /*
     *  Animated ANSI Color login prompt by Shaun Marquardt
     */
	dnew->ansi = TRUE;
	pulse_color_anim = PULSE_COLOR_ANIM;

	sprintf(buf, "{WDo you see{X ");
	send_to_desc2(buf, dnew);

	update_handler();
for(a=0;a<6;a++)
{
	while(PULSE_COLOR_ANIM)
	{
		if(a==5) break;

    		if( --pulse_color_anim <= 0 )
		{
			update_handler();
			pulse_color_anim = PULSE_COLOR_ANIM;
			break;
		}
	}

	sprintf(buf, "{%s%c", codes[a], color[a]);
	send_to_desc2(buf, dnew);

	if(a==5)
	{
		send_to_desc2(" ", dnew);
		if( read_from_descriptor(dnew) ) break;
	}

}

//    write_to_buffer( dnew, "Do you wish to play using ANSI color? [Y]es or [N]o ", 0 );
    return;
}
#endif



void close_socket( DESCRIPTOR_DATA *dclose )
{
    CHAR_DATA *ch;

    if ( dclose->outtop > 0 )
	process_output( dclose, FALSE );

    if ( dclose->snoop_by != NULL )
    {
	write_to_buffer( dclose->snoop_by,
	    "Your victim has left the game.\n\r", 0 );
    }

    {
	DESCRIPTOR_DATA *d;

	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->snoop_by == dclose )
		d->snoop_by = NULL;
	}
    }

    if ( ( ch = dclose->character ) != NULL )
    {
	sprintf( log_buf, "Closing link to %s.", ch->name );
	log_string( log_buf );
	/* cut down on wiznet spam when rebooting */
	if ( dclose->connected == CON_PLAYING && !merc_down)
	{
	    act( "$n has lost $s link.", ch, NULL, NULL, TO_ROOM );
	    wiznet("Net death has claimed $N.",ch,NULL,WIZ_LINKS,0,0);
	    ch->desc = NULL;
	    if (!IS_NPC(ch) && (!str_cmp(ch->pcdata->lokprot, "on")))
	    {
            ch->pcdata->lokprot = "off";
          }
	}
	else
	{
	    free_char(dclose->original ? dclose->original : 
		dclose->character );
	    if (!IS_NPC(ch) && (!str_cmp(ch->pcdata->lokprot, "on")))
	    {
            ch->pcdata->lokprot = "off";
          }
	}
    }

    if ( d_next == dclose )
	d_next = d_next->next;   

    if ( dclose == descriptor_list )
    {
	descriptor_list = descriptor_list->next;
    }
    else
    {
	DESCRIPTOR_DATA *d;

	for ( d = descriptor_list; d && d->next != dclose; d = d->next )
	    ;
	if ( d != NULL )
	    d->next = dclose->next;
	else
	    bug( "Close_socket: dclose not found.", 0 );
    }

    close( dclose->descriptor );
    free_descriptor(dclose);
#if defined(MSDOS) || defined(macintosh)
    exit(1);
#endif
    return;
}



bool read_from_descriptor( DESCRIPTOR_DATA *d )
{
    int iStart;

    /* Hold horses if pending command already. */
    if ( d->incomm[0] != '\0' )
	return TRUE;

    /* Check for overflow. */
    iStart = strlen(d->inbuf);
    if ( iStart >= sizeof(d->inbuf) - 10 )
    {
	sprintf( log_buf, "%s input overflow!", d->host );
	log_string( log_buf );
	write_to_descriptor( d->descriptor,
	    "\n\r*** PUT A LID ON IT ALREADY!!! ***\n\r", 0 );
	return FALSE;
    }

    /* Snarf input. */
#if defined(macintosh)
    for ( ; ; )
    {
	int c;
	c = getc( stdin );
	if ( c == '\0' || c == EOF )
	    break;
	putc( c, stdout );
	if ( c == '\r' )
	    putc( '\n', stdout );
	d->inbuf[iStart++] = c;
	if ( iStart > sizeof(d->inbuf) - 10 )
	    break;
    }
#endif

#if defined(MSDOS) || defined(unix)
    for ( ; ; )
    {
	int nRead;

	nRead = read( d->descriptor, d->inbuf + iStart,
	    sizeof(d->inbuf) - 10 - iStart );
	if ( nRead > 0 )
	{
	    iStart += nRead;
	    if ( d->inbuf[iStart-1] == '\n' || d->inbuf[iStart-1] == '\r' )
		break;
	}
	else if ( nRead == 0 )
	{
	    log_string( "EOF encountered on read." );
	    return FALSE;
	}
	else if ( errno == EWOULDBLOCK )
	    break;
	else
	{
	    perror( "Read_from_descriptor" );
	    return FALSE;
	}
    }
#endif

    d->inbuf[iStart] = '\0';
    return TRUE;
}



/*
 * Transfer one line from input buffer to input line.
 */
void read_from_buffer( DESCRIPTOR_DATA *d )
{
    int i, j, k;

    /*
     * Hold horses if pending command already.
     */
    if ( d->incomm[0] != '\0' )
	return;

    /*
     * Look for at least one new line.
     */
    for ( i = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++ )
    {
	if ( d->inbuf[i] == '\0' )
	    return;
    }

    /*
     * Canonical input processing.
     */
    for ( i = 0, k = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++ )
    {
	if ( k >= MAX_INPUT_LENGTH - 2 )
	{
	    write_to_descriptor( d->descriptor, "Line too long.\n\r", 0 );

	    /* skip the rest of the line */
	    for ( ; d->inbuf[i] != '\0'; i++ )
	    {
		if ( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
		    break;
	    }
	    d->inbuf[i]   = '\n';
	    d->inbuf[i+1] = '\0';
	    break;
	}

	if ( d->inbuf[i] == '\b' && k > 0 )
	    --k;
	else if ( isascii(d->inbuf[i]) && isprint(d->inbuf[i]) )
	    d->incomm[k++] = d->inbuf[i];
    }

    /*
     * Finish off the line.
     */
    if ( k == 0 )
	d->incomm[k++] = ' ';
    d->incomm[k] = '\0';

    /*
     * Deal with bozos with #repeat 1000 ...
     */


    if ( k > 1 || d->incomm[0] == '!' )
    {
    	if ( d->incomm[0] != '!' && strcmp( d->incomm, d->inlast ) )
	{
	    d->repeat = 0;
	}
	else
	{
	    if (++d->repeat >= 25 && d->character
	    &&  d->connected == CON_PLAYING)
	    {
		sprintf( log_buf, "%s input spamming!", d->host );
		log_string( log_buf );
		wiznet("Spam spam spam $N spam spam spam spam spam!",
		       d->character,NULL,WIZ_SPAM,0,get_trust(d->character));
		if (d->incomm[0] == '!')
		    wiznet(d->inlast,d->character,NULL,WIZ_SPAM,0,
			get_trust(d->character));
		else
		    wiznet(d->incomm,d->character,NULL,WIZ_SPAM,0,
			get_trust(d->character));

		d->repeat = 0;
/*
		write_to_descriptor( d->descriptor,
		    "\n\r*** PUT A LID ON IT ALREADY!!! ***\n\r", 0 );
		strcpy( d->incomm, "quit" );
*/
	    }
	}
    }


    /*
     * Do '!' substitution.
     */
    if ( d->incomm[0] == '!' )
	strcpy( d->incomm, d->inlast );
    else
	strcpy( d->inlast, d->incomm );

    /*
     * Shift the input buffer.
     */
    while ( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
	i++;
    for ( j = 0; ( d->inbuf[j] = d->inbuf[i+j] ) != '\0'; j++ )
	;
    return;
}

/*
 * Low level output function.
 */
bool process_output( DESCRIPTOR_DATA *d, bool fPrompt )
{
    extern bool merc_down;
    int percent, percent2, percent3;

    /*
     * Bust a prompt.
     */
    if ( !merc_down )
    {
	if ( d->showstr_point )
	    write_to_buffer( d, "\n\r[Hit Return to continue]\n\r", 0 );
	else if ( fPrompt && d->pString && d->connected == CON_PLAYING )
	    write_to_buffer( d, "> ", 2 );
	else if ( fPrompt && d->connected == CON_PLAYING )
	{
	    CHAR_DATA *ch;
	CHAR_DATA *victim;

	ch = d->character;

        /* battle prompt */
        if ((victim = ch->fighting) != NULL && can_see(ch,victim))
        {
            int percent;
            char wound[100];
	    char buf[MAX_STRING_LENGTH];
 
            if (victim->max_hit > 0)
                percent = victim->hit * 100 / victim->max_hit;
            else
                percent = -1;
 
            if (percent >= 100)
                sprintf(wound,"is in perfect health.");
            else if (percent >= 90)
                sprintf(wound,"is slightly scratched.");
            else if (percent >= 80)
                sprintf(wound,"has a few bruises.");
            else if (percent >= 70)
                sprintf(wound,"has some small wounds and bruises.");
            else if (percent >= 60)
                sprintf(wound,"has several wounds.");
            else if (percent >= 50)
                sprintf(wound,"has many nasty wounds.");
            else if (percent >= 40)
                sprintf(wound,"is bleeding freely.");
            else if (percent >= 30)
                sprintf(wound,"is covered in blood.");
            else if (percent >= 20)
                sprintf(wound,"is vomiting blood.");
            else if (percent >= 10)
                sprintf(wound,"is barely clinging to life.");
            else if (percent >= 0)
                sprintf(wound,"is about to die.");
            else
                sprintf(wound,"is bleeding to death.");
 
            if (!can_see(ch,victim))
                sprintf(buf, "Someone %s \n\r", wound);
            else
                sprintf(buf,"%s %s \n\r", 
/* before doppel *    IS_NPC(victim) ? victim->short_descr : victim->name,wound); */
           (is_affected(victim,gsn_doppelganger)&& !IS_SET(ch->act,PLR_HOLYLIGHT)) ?
            PERS(victim->doppel,ch) :
 	      (is_affected(victim,gsn_treeform)&& !IS_SET(ch->act,PLR_HOLYLIGHT)) ? 
		"A mighty oak tree" : PERS(victim,ch),wound);
            buf[0] = UPPER(buf[0]);
		send_to_desc(buf, d);
        }

	ch = d->original ? d->original : d->character;
	if (!IS_SET(ch->comm, COMM_COMPACT) )
	    write_to_buffer( d, "\n\r", 2 );


        if ( IS_SET(ch->comm, COMM_PROMPT) )
            bust_a_prompt( d->character );

	if (IS_SET(ch->comm,COMM_TELNET_GA))
	    write_to_buffer(d,go_ahead_str,0);

	if (!IS_NPC(ch) && (!str_cmp(ch->pcdata->lokprot, "on")))
      {
	  char buf[MAX_STRING_LENGTH];
        percent = ch->hit * 100 / ch->max_hit;
        percent2 = ch->mana * 100 / ch->max_mana;
	  percent3 = ch->move * 100 / ch->max_move;
        sprintf(buf, "locprothp/mana/move%dlocprothp/mana/move%dlocprothp/mana/move%d", percent, percent2, percent3);
	  send_to_desc(buf, d);

	  /* percent2 = ch->mana * 100 / ch->max_mana;
        sprintf(buf, "locprotmana%d", percent2);
	  send_to_desc(buf, d); */
      }
    }
    }
    /*
     * Short-circuit if nothing to write.
     */
    if ( d->outtop == 0 )
	return TRUE;

    /*
     * Snoop-o-rama.
     */
    if ( d->snoop_by != NULL )
    {
	if (d->character != NULL)
	    write_to_buffer( d->snoop_by, d->character->name,0);
	write_to_buffer( d->snoop_by, "> ", 2 );
	write_to_buffer( d->snoop_by, d->outbuf, d->outtop );
    }

    /*
     * OS-dependent output.
     */
    if ( !write_to_descriptor( d->descriptor, d->outbuf, d->outtop ) )
    {
	d->outtop = 0;
	return FALSE;
    }
    else
    {
	d->outtop = 0;
	return TRUE;
    }
}

/*
 * Bust a prompt (player settable prompt)
 * coded by Morgenes for Aldara Mud
 */
void bust_a_prompt( CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    const char *str;
    const char *i=NULL;
    char *point;
    char *pbuff;
    char buffer[ MAX_STRING_LENGTH*2 ];
    char doors[MAX_STRING_LENGTH];
    EXIT_DATA *pexit;
    bool found;
    const char *dir_name[] = {"N","E","S","W","U","D"};
    char wound[MAX_STRING_LENGTH];
    int percent;
    int door;
 
    point = buf;
    str = ch->prompt;
    if (str == NULL || str[0] == '\0')
    {
/*        sprintf( buf, "{p<%dhp %dm %dmv> {x %s",
	    ch->hit,ch->mana,ch->move,ch->prefix);
	send_to_char(buf,ch);
        return;*/
        strcpy(ch->prompt,"<%hhp %mm %vmv>%f");
    }

   if (IS_SET(ch->comm,COMM_AFK))
   {
	send_to_char("{p<AFK>{x ",ch);
	return;
   }

   while( *str != '\0' )
   {
      if( *str != '%' )
      {
         *point++ = *str++;
         continue;
      }
      ++str;
      switch( *str )
      {
        default :
          i = " "; break;

	  case 'e':
	    found = FALSE;
	    doors[0] = '\0';
	    for (door = 0; door < 6; door++)
	    {
		if ((pexit = ch->in_room->exit[door]) != NULL
		&&  pexit ->u1.to_room != NULL
		&&  (can_see_room(ch,pexit->u1.to_room)
		||   (IS_AFFECTED(ch,AFF_INFRARED) 
		&&    !IS_AFFECTED(ch,AFF_BLIND)))
		&&  !IS_SET(pexit->exit_info,EX_CLOSED))
		{
		    found = TRUE;
		    strcat(doors,dir_name[door]);
		}
	    }

	    if (!found)
            strcat(doors,"none");

	    sprintf(buf2,"%s",doors);
	    i = buf2;
          break;

	case 'b' :
        if ((victim = ch->fighting) != NULL)
        {
          if (victim->max_hit > 0)
                percent = victim->hit * 100 / victim->max_hit;
          else
                percent = -1;
          if (percent >= 100)
                sprintf(wound,"{wEnemy: [{r+++{Y+++{g++++{x]");
          else if (percent >= 90)
                sprintf(wound,"{wEnemy: [{r+++{Y+++{g+++ {x]");
          else if (percent >= 80)
                sprintf(wound,"{wEnemy: [{r+++{Y+++{g++  {x]");
          else if (percent >= 70)
                sprintf(wound,"{wEnemy: [{r+++{Y+++{g+   {x]");
          else if (percent >= 58)
                sprintf(wound,"{wEnemy: [{r+++{Y+++    {x]");
          else if (percent >= 45)
                sprintf(wound,"{wEnemy: [{r+++{Y++     {x]");
          else if (percent >= 30)
                sprintf(wound,"{wEnemy: [{r+++{Y+      {x]");
          else if (percent >= 28)
                sprintf(wound,"{wEnemy: [{r+++{x         ]");
          else if (percent >= 15)
                sprintf(wound,"{wEnemy: [{r++{x          ]");
          else if (percent >= 8)
                sprintf(wound,"{wEnemy: [{r+{x           ]");
          else
                sprintf(wound,"{wEnemy: [          {x]");
            sprintf(buf2," %s",wound);
            i = buf2;
        }
        else
           sprintf( buf2, " " );
        i = buf2;       
        break;

	case 'B' :
           if ((victim = ch->fighting) != NULL)
           {
             if (victim->max_hit > 0)
                percent = victim->hit * 100 / victim->max_hit;
             else
                percent = -1;
             if(percent >= 65)
               sprintf(buf2," {wEnemy: {g%d{x%%",percent);
             else if(percent >= 25 && percent < 65)
               sprintf(buf2," {wEnemy: {Y%d{x%%",percent);
             else
               sprintf(buf2," {wEnemy: {r%d{x%%",percent);
             i = buf2;
           }
           else
             sprintf( buf2, " " );
           i = buf2;         
           break;   

 	 case 'c' :
	    sprintf(buf2,"%s","\n\r");
            i = buf2; break;
       case 'h' :
            sprintf( buf2, "%d", ch->hit );
            i = buf2; break;
       case 'H' :
            sprintf( buf2, "%d", ch->max_hit );
            i = buf2; break;
       case 'm' :
            sprintf( buf2, "%d", ch->mana );
            i = buf2; break;
       case 'M' :
            sprintf( buf2, "%d", ch->max_mana );
            i = buf2; break;
       case 'v' :
            sprintf( buf2, "%d", ch->move );
            i = buf2; break;
       case 'V' :
            sprintf( buf2, "%d", ch->max_move );
            i = buf2; break;
       case 'x' :
            sprintf( buf2, "%d", ch->exp );
            i = buf2; break;
	 case 'X' :
	    sprintf(buf2, "%d", IS_NPC(ch) ? 0 :
	    exp_at_level(ch, ch->level + 1) - ch->exp);
	    i = buf2; break;
         case 'g' :
            sprintf( buf2, "%ld", ch->steel);
            i = buf2; break;
	 case 's' :
	    sprintf( buf2, "%ld", ch->gold);
	    i = buf2; break;
         case 'a' :
            if( ch->level > 9 )
               sprintf( buf2, "%d", ch->alignment );
            else
               sprintf( buf2, "%s", IS_GOOD(ch) ? "good" : IS_EVIL(ch) ?
                "evil" : "neutral" );
            i = buf2; break;
         case 'r' :
            if( ch->in_room != NULL )
               sprintf( buf2, "%s", 
		((!IS_NPC(ch) && IS_SET(ch->act,PLR_HOLYLIGHT)) ||
		 (!IS_AFFECTED(ch,AFF_BLIND) && !room_is_dark( ch->in_room )))
		? ch->in_room->name : "darkness");
            else
               sprintf( buf2, " " );
            i = buf2; 
            break;
      case 'R' :
            if( IS_IMMORTAL( ch ) && ch->in_room != NULL )
               sprintf( buf2, "%d", ch->in_room->vnum );
            else
               sprintf( buf2, " " );
            i = buf2; break;
         case 'z' :
            if( IS_IMMORTAL( ch ) && ch->in_room != NULL )
               sprintf( buf2, "%s", ch->in_room->area->name );
            else
               sprintf( buf2, " " );
            i = buf2; break;
         case '%' :
            sprintf( buf2, "%%" );
            i = buf2; break;
         case 'o' :
            sprintf( buf2, "%s", olc_ed_name(ch) );
            i = buf2; break;
	 case 'O' :
	    sprintf( buf2, "%s", olc_ed_vnum(ch) );
	    i = buf2; break;
	 case 'W' ://C068
             if(ch->invis_level) sprintf( buf2, " %d", ch->invis_level);
		 else sprintf(buf2, " ");

		 i = buf2;
             break;
	 case 'w' ://C068
             if(ch->incog_level) sprintf( buf2, " %d", ch->incog_level);
		 else sprintf(buf2, " ");

		 i = buf2;
             break;
	 case 'D' ://C068
             sprintf( buf2, "%s", ch->name);
	       i = buf2; break;

      }
      ++str;
      while( (*point = *i) != '\0' )
         ++point, ++i;
   }
   *point	= '\0';
   pbuff	= buffer;
   colourconv( pbuff, buf, ch );
   send_to_char( "{p", ch );
   write_to_buffer( ch->desc, buffer, 0 );
   if (IS_SET (ch->act2, PLR_AUTODIG))
        send_to_char ("{c[{WA{wuto{WD{wig{c]{x ", ch);
   send_to_char( "{x", ch );

   if (ch->prefix[0] != '\0')
        write_to_buffer(ch->desc,ch->prefix,0);
   return;
}

/*
                                reverse_txt
    
    Reverse a words in a text blob.  Used by reverse screen effect.
*/
void reverse_txt( char *txt, int length )
{
    int i, state, c;
    char *word_ptr = NULL;
#define WALKTHRU        0
#define INWORD          1
#define INESC           2
    /* walk thru line looking for words, ignore color codes */
    for( i = 0, state = WALKTHRU; i < length; i++, txt++ )
    {
        c = *txt;
        switch(state)
        {
        case WALKTHRU:                  /* just leave in place */
            if( isalnum(c) )            /* start of word? */
            {
                word_ptr = txt;
                state = INWORD;
            }
            else if( c == '\033' )      /* start of color code escape seq? */
                state = INESC;
            break;
        case INESC:                     /* ignore contents of color code seq */
            if( c == 'm' )              /* 'm' marks end of code */
                state = WALKTHRU;
            break;
        
        case INWORD:                    /* traversing a word */
            if( isalnum(c) 
             || (c == '-'  && isalnum(txt[1]) )         /* hypenated */
             || (c == '\'' && isalnum(txt[1]) ) )       /* contraction */
                break;                  /* still in word */
            
            /* end of word, do the swap */
            reverse_word( word_ptr, txt - word_ptr );
            if( c == '\033' )
                state = INESC;
            else
                state = WALKTHRU;
            word_ptr = NULL;
        }
    }
}

/*
 * reverse_word
 * Just swap one word.
 */
void reverse_word( char *w, int n )
{
    int i, hlen, left, right, up_left, up_right;
    hlen = n / 2;
    n--;
    for( i = 0; i < hlen; i++ )
    {
        left = w[i];
        right = w[n - i];
        up_left = isupper(left) ? TRUE : FALSE;
        up_right = isupper(right) ? TRUE : FALSE;
        if( up_left != up_right )
        {
            if( isupper(left) )
                left += 'a' - 'A';
            else
                left -= 'a' - 'A';
            if( isupper(right) )
                right += 'a' - 'A';
            else
                right -= 'a' - 'A';
        }
        w[i] = right;
        w[n - i] = left;
    }
}



/*
 * Append onto an output buffer.
 */
void write_to_buffer( DESCRIPTOR_DATA *d, const char *txt, int length )
{
    int reverse;

    /* check for special screen effect */
    if( d->character != NULL 
     && IS_AFFECTED2( d->character, AFF_LOOKING_GLASS ) 
     && d->connected == CON_PLAYING )
        reverse = TRUE;
    else
        reverse = FALSE;

    /*
     * Find length in case caller didn't.
     */
    if ( length <= 0 )
	length = strlen(txt);

    /*
     * Initial \n\r if needed.
     */
    if ( d->outtop == 0 && !d->fcommand )
    {
	d->outbuf[0]	= '\n';
	d->outbuf[1]	= '\r';
	d->outtop	= 2;
    }

    /*
     * Expand the buffer as needed.
     */
    while ( d->outtop + length >= d->outsize )
    {
	char *outbuf;

        if (d->outsize >= 32000)
	{
	    bug("Buffer overflow. Closing.\n\r",0);
	    close_socket(d);
	    return;
 	}
	outbuf      = alloc_mem( 2 * d->outsize );
	strncpy( outbuf, d->outbuf, d->outtop );
	free_mem( d->outbuf, d->outsize );
	d->outbuf   = outbuf;
	d->outsize *= 2;
    }

    /*
     * Copy.
     */
    strncpy( d->outbuf + d->outtop, txt, length );
    if( reverse )
        reverse_txt( d->outbuf + d->outtop, length );
    d->outtop += length;
    return;
}



/*
 * Lowest level output function.
 * Write a block of text to the file descriptor.
 * If this gives errors on very long blocks (like 'ofind all'),
 *   try lowering the max block size.
 */
bool write_to_descriptor( int desc, char *txt, int length )
{
    int iStart;
    int nWrite;
    int nBlock;

#if defined(macintosh) || defined(MSDOS)
    if ( desc == 0 )
	desc = 1;
#endif

    if ( length <= 0 )
	length = strlen(txt);

    for ( iStart = 0; iStart < length; iStart += nWrite )
    {
	nBlock = UMIN( length - iStart, 4096 );
	if ( ( nWrite = write( desc, txt + iStart, nBlock ) ) < 0 )
	    { perror( "Write_to_descriptor" ); return FALSE; }
    } 

    return TRUE;
}



/*
 * Deal with sockets that haven't logged in yet.
 */
void nanny( DESCRIPTOR_DATA *d, char *argument )
{
    DESCRIPTOR_DATA *d_old, *d_next;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *ch;
    char *pwdnew;
    char *p;
    int iClass,race,i;
    bool fOld;
    OBJ_DATA *obj;
    char pwdbuf[MAX_STRING_LENGTH];
    CHAR_DATA *gch;

    while ( isspace(*argument) )
	argument++;

    ch = d->character;

    switch ( d->connected )
    {

    default:
	bug( "Nanny: bad d->connected %d.", d->connected );
	close_socket( d );
	return;

    case CON_ANSI:
        if ( argument[0] == '\0' || UPPER(argument[0]) == 'Y' )
        {
            d->ansi = TRUE;
            send_to_desc( "{RAnsi enabled!{x\n\r",d);
            d->connected = CON_GET_NAME;
            {
                extern char * help_greeting;
                if ( help_greeting[0] == '.' )
                  send_to_desc( help_greeting+1, d );
                else
                  send_to_desc( help_greeting , d );
            }
            break;
        }
  
        else
        if (UPPER(argument[0]) == 'N')
        {
            d->ansi = FALSE;
            send_to_desc("Ansi disabled!\n\r",d);
            d->connected = CON_GET_NAME;
            {
                extern char * help_greeting;
                if ( help_greeting[0] == '.' )
                  send_to_desc( help_greeting+1, d );
                else
                  send_to_desc( help_greeting , d );
            }
            break;
        }
        else
        {
	      send_to_desc( "Would you like to play using ANSI colors? [Y]es or [N]o ",d);
            return;
        }

      case CON_GET_NAME:
	if ( argument[0] == '\0' )
	{
	    close_socket( d );
	    return;
	}

	argument[0] = UPPER(argument[0]);
	if ( !check_parse_name( argument ) )
	{
	    write_to_buffer( d, "\n\rThe MUD Administrators have found the name to be unacceptable.\n\r\n\rName: ", 0 );
	    return;
	}

       /* Bug Fix for char creation of same name at same time */
        {
	  DESCRIPTOR_DATA *wd;
	  char *name = argument;
	  
	  for( wd = descriptor_list; wd; wd = wd->next )
	  {
	    if( wd->character == NULL
	        || wd->character->name == NULL
	        || wd->character->name[0] == '\0')
	      continue;
	    if( wd->character->level > 1 )
	      continue;
	    if( is_name(name, wd->character->name))
	    {
	      write_to_buffer( d,"\n\rThere is another player creating a character with this name.\n\rWhat is your name: ",0);
	      return;
	    }
	  }
	}
       /* end bug fix */

	fOld = load_char_obj( d, argument );
	ch   = d->character;

	if (IS_SET(ch->act, PLR_DENY))
	{
	 sprintf( log_buf, "Denying access to %s@%s.", argument, d->host );
	 log_string( log_buf );
	 write_to_buffer( d, "\n\r", 0 );
	 write_to_buffer( d, "The actions you have taken have been deemed grossly unacceptable\n\r", 0 );
	 write_to_buffer( d, "to the administration of this mud. You have been DENIED access from\n\r", 0 );
	 write_to_buffer( d, "this MUD with the current character your using. Feel free to start a\n\r", 0 );
	 write_to_buffer( d, "new character though. If you continue to perform the same actions\n\r", 0 );
	 write_to_buffer( d, "or any actions like them, you will not only lose your new character\n\r", 0 );
	 write_to_buffer( d, "but you will also lose access to this MUD, your site will be banned.\n\r", 0 );
	 write_to_buffer( d, "\n\r", 0 );
	 close_socket( d );
	 return;
	}

	if (check_ban(d->host,BAN_PERMIT) && !IS_SET(ch->act,PLR_PERMIT))
	{
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d," ** Your site has been banned from this mud. **\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    close_socket(d);
	    return;
	}

        if (ch->deathstat >= MAX_DEATH)
        {
           write_to_buffer(d,"\n\rThis character has undergone age death.\n\r",0); 
           write_to_buffer(d,"The name is unavailable until it is deleted due to inactivity.\n\n\r",0);
               if (d->character->pet)
                {
               CHAR_DATA *pet=d->character->pet;

               char_to_room(pet,get_room_index( ROOM_VNUM_LIMBO));
               stop_follower(pet);
               extract_char(pet,TRUE);
                }
           close_socket(d);
           return;
        }


	if ( check_reconnect( d, argument, FALSE ) )
	{
	    fOld = TRUE;
	}
	else
	{
	    if ( wizlock && !IS_IMMORTAL(ch)) 
	    {
		write_to_buffer( d, "The game is wizlocked.\n\r", 0 );
		close_socket( d );
		return;
	    }
	}

	if ( fOld )
	{
	    /* Old player */
	    write_to_buffer( d, "Password: ", 0 );
	    write_to_buffer( d, echo_off_str, 0 );
	    d->connected = CON_GET_OLD_PASSWORD;
	    return;
	}
	else
	{
	    /* New player */
 	    if (newlock)
	    {
	      write_to_buffer(d,"\n\r",0);
	      write_to_buffer(d,"\n\r",0);
            write_to_buffer( d, "The game is newlocked.\n\r", 0 );
	      write_to_buffer(d,"\n\r",0);
	      write_to_buffer(d,"Please come back at another time.\n\r",0);
	      write_to_buffer(d,"\n\r",0);
            close_socket( d );
            return;
          }

	    if (check_ban(d->host,BAN_NEWBIES))
	    {
	      write_to_buffer(d,"\n\r",0);
	      write_to_buffer(d,"\n\r",0); 
		write_to_buffer(d,"New players are not allowed from your site.\n\r",0);
	      write_to_buffer(d,"\n\r",0);
	      write_to_buffer(d,"\n\r",0);
		close_socket(d);
		return;
	    }
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d," -----------------------------------------------------------------------------\n\r",0);
	    write_to_buffer
(d," : + - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < - + :\n\r",0);
	    write_to_buffer
(d," : | <-------------------------------------------------------------------> | :\n\r",0);
	    write_to_buffer
(d," : + :        Welcome! We are glad you have taken interest in the        : + :\n\r",0);
	    write_to_buffer
(d," : - :  Legends of Krynn, but before you enter the realms, a valid name  : - :\n\r",0);
	    write_to_buffer
(d," : + :  is required. As the realm is medieval themed, you cannot enter  : + :\n\r",0);
	    write_to_buffer
(d," : | :   with names like Kevin, Bob, Joe, or Smith, from our every day   : | :\n\r",0);
	    write_to_buffer
(d," : + :  lives, nor can you go with names from movies, television shows,  : + :\n\r",0);
	    write_to_buffer
(d," : - :  magazines, or video games, such as Seinfeld, Gades, HanSolo, or  : - :\n\r",0);
	    write_to_buffer
(d," : + :                             Jeopardy.                             : + :\n\r",0);
	    write_to_buffer
(d," : | :                                                                   : | :\n\r",0);
	    write_to_buffer
(d," : + :          Biblical names such as Angel/Devil are illegal.          : + :\n\r",0);
	    write_to_buffer
(d," : - :    Two words making up a name is illegal (such as SwiftBlade).    : - :\n\r",0);
	    write_to_buffer
(d," : + :  Names that do not apply to these guidlines will be asked to pick : + :\n\r",0);
	    write_to_buffer
(d," : + :   a new name in the game. Not complying with this will result in  : | :\n\r",0);
	    write_to_buffer
(d," : - : the names being deleted/locked so they cannot use that name again.: + :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : | :\n\r",0);
	    write_to_buffer
(d," : | <-------------------------------------------------------------------> | :\n\r",0);
	    write_to_buffer
(d," : + - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < - + :\n\r",0);
	    write_to_buffer
(d," -----------------------------------------------------------------------------\n\r",0);
	 write_to_buffer(d,"\n\r",0);
	 write_to_buffer(d,"\n\r",0);
	 sprintf( buf, "Do you comply with these guidlines %s (Y/N)? ", argument );
	 write_to_buffer( d, buf, 0 );
	 d->connected = CON_CONFIRM_COMPLY;
	 return;
	}
	break;

    case CON_CONFIRM_COMPLY:
	switch ( *argument )
	{
	case 'y': case 'Y':
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d," -----------------------------------------------------------------------------\n\r",0);
	    write_to_buffer
(d," : + - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < - + :\n\r",0);
	    write_to_buffer
(d," : | <-------------------------------------------------------------------> | :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : - :                                                                   : - :\n\r",0);
	    write_to_buffer
(d," : + :                Outstanding! Thank you for complying!              : + :\n\r",0);
	    write_to_buffer
(d," : | :                                                                   : | :\n\r",0);
	    write_to_buffer
(d," : + :      As you step forward into the light you notice an old man     : + :\n\r",0);
	    write_to_buffer
(d," : - :    approaching you. He wears the long robes of one of the magic   : - :\n\r",0);
	    write_to_buffer
(d," : + :     users of the White Order, although his have dulled into a     : + :\n\r",0);
	    write_to_buffer
(d," : | :    mouse-grey.  He plays with his long white beard with one of    : | :\n\r",0);
	    write_to_buffer
(d," : + :    his wrinkled hands as he examines you with his piercing blue   : + :\n\r",0);
	    write_to_buffer
(d," : - :    eyes.  He walks slowly up to you and holds one hand out and    : - :\n\r",0);
	    write_to_buffer
(d," : + :                          says to you:                             : + :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : | :\n\r",0);
	    write_to_buffer
(d," : | <-------------------------------------------------------------------> | :\n\r",0);
	    write_to_buffer
(d," : + - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < - + :\n\r",0);
	    write_to_buffer
(d," -----------------------------------------------------------------------------\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    sprintf( buf, " 'Greetings!  You're %s are you not!?'  Y or N  ", ch->name);
	    write_to_buffer( d, buf, 0 );
	    d->connected = CON_CONFIRM_NEW_NAME;
	    break;

	case 'n': case 'N':
	    write_to_buffer( d,"\n\r",0);
	    write_to_buffer( d, "Too bad! Come back when you're ready to comply! ", 0 );
          close_socket( d );
	    break;

	default:
	    write_to_buffer( d, "Please type Yes or No! ", 0 );
	    break;
	}
	break;

    case CON_GET_OLD_PASSWORD:
#if defined(unix)
	write_to_buffer( d, "\n\r", 2 );
#endif

	if ( strcmp( crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd ))
	{
	    write_to_buffer( d, "Wrong password.\n\r", 0 );
	    sprintf( log_buf, "%s. Bad Password. IP: %s> %s", ch->name,d->host,argument);
	    log_string( log_buf );
          sprintf( log_buf, "%s@%s bad password.", ch->name, d->host );
          wiznet(log_buf,NULL,NULL,WIZ_LOGINS,0,get_trust(ch));
           if ((gch = get_char_world(ch,ch->name)) != NULL)
           {
           send_to_char(   "{R                 {R>{r>{D> {wBAD {WPASSWORD {wATTEMPT {D<{r<{R<{x\n\r{x",gch);
           sprintf( pwdbuf,"{w            %s tried to log in with a bad password.\n\r{x",d->host);
           send_to_char(pwdbuf,gch);
           }
           if (d->character->pet) 
           {
               CHAR_DATA *pet=d->character->pet;
               char_to_room(pet,get_room_index( ROOM_VNUM_LIMBO));
               stop_follower(pet);
               extract_char(pet,TRUE);
           }
	    close_socket( d );
	    return;
	}
 
	write_to_buffer( d, echo_on_str, 0 );

	if (check_playing(d,ch->name))
	    return;

	if ( check_reconnect( d, ch->name, TRUE ) )
	    return;

	sprintf( log_buf, "Look out, %s@%s has connected.", ch->name, d->host );
	log_string( log_buf );
	wiznet(log_buf,NULL,NULL,WIZ_SITES,0,get_trust(ch));

	if (ch->desc->ansi)
          SET_BIT(ch->act, PLR_COLOUR);
      else REMOVE_BIT(ch->act, PLR_COLOUR);

      if ( IS_IMMORTAL(ch) )
      {
	write_to_buffer( d, " Would you like to login (W)izinvis, (I)ncognito, or(N)ormal? ", 0 );
      d->connected = CON_WIZ;
      }
	else
	{
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer
(d," -----------------------------------------------------------------------------\n\r",0);
	    write_to_buffer
(d," : + - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < - + :\n\r",0);
	    write_to_buffer
(d," : | <-------------------------------------------------------------------> | :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : - :                      MOTD (Message of the Day)                    : - :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : | :  * You are responsible for knowing the rules (type 'rules').      : | :\n\r",0);
	    write_to_buffer
(d," : + :    Ignorance of the rules is no excuse.                           : + :\n\r",0);
	    write_to_buffer
(d," : - :                                                                   : - :\n\r",0);
	    write_to_buffer
(d," : + :  * Important commands: HELP, COMMANDS, NOTES, IDEAS, CHANGES,     : + :\n\r",0);
	    write_to_buffer
(d," : | :    RULES                                                          : | :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : - :  * Use the DELETE command to erase unwanted characters            : - :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : | :  * You MUST have a description for your character by level 10!     : | :\n\r",0);
	    write_to_buffer
(d," : + :    Failure to do so MAY result in imprisonment until one is      : + :\n\r",0);
	    write_to_buffer
(d," : - :    written!                                                       : - :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : + :  * Roleplaying is not only encouraged here, but without it you    : | :\n\r",0);
	    write_to_buffer
(d," : - :    will be denied, and then banned if no improvement is found.    : + :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : - :\n\r",0);
	    write_to_buffer
(d," : | :  * Make sure you Read and Understand HELP TENETS and HELP MISSION : | :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : - :  * English is the only language allowed to be used here for       : - :\n\r",0);
	    write_to_buffer
(d," : + :    Communicating In Character. NO EXCEPTIONS!                     : + :\n\r",0);
	    write_to_buffer
(d," : | <-------------------------------------------------------------------> | :\n\r",0);
	    write_to_buffer
(d," : + - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < - + :\n\r",0);
	    write_to_buffer
(d," -----------------------------------------------------------------------------\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"                          [ Hit RETURN to Continue ]\n\r",0);
	    d->connected = CON_READ_MOTD;
	}
	break;

	case CON_WIZ:
	    write_to_buffer( d, "\n\r", 2 ); 
	    switch ( *argument )
	    {
		case 'w': case 'W':
		    SET_BIT(ch->act, PLR_WIZINVIS);
		    (ch->invis_level = ch->level);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer
(d," -----------------------------------------------------------------------------\n\r",0);
	    write_to_buffer
(d," : + - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < - + :\n\r",0);
	    write_to_buffer
(d," : | <-------------------------------------------------------------------> | :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : - :               IMOTD (Immortal Message of the Day)                 : - :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : | :                         Welcome Immortal!                         : | :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : - :                                                                   : - :\n\r",0);
	    write_to_buffer
(d," : + :  * You are responsible for following the laws of Immortal         : + :\n\r",0);
	    write_to_buffer
(d," : | :    behavior. These can be found by typing 'HELP LAWS' or          : | :\n\r",0);
	    write_to_buffer
(d," : + :    'HELP COMMANDMENTS'.                                           : + :\n\r",0);
	    write_to_buffer
(d," : - :                                                                   : - :\n\r",0);
	    write_to_buffer
(d," : + :  * Excessive loading is not allowed, and is grounds for deletion. : + :\n\r",0);
	    write_to_buffer
(d," : | :                                                                   : | :\n\r",0);
	    write_to_buffer
(d," : + :  * Asking for promotion is grounds for instant deletion.          : + :\n\r",0);
	    write_to_buffer
(d," : - :                                                                   : - :\n\r",0);
	    write_to_buffer
(d," : + :  * Type 'HELP JOBS' to get some idea of what you should be doing. : + :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : | :\n\r",0);
	    write_to_buffer
(d," : - :  * Reading 'HELP TENETS' is a MUST! You must know everything about: + :\n\r",0);
	    write_to_buffer
(d," : + :    it nearly by memory.                                           : - :\n\r",0);
	    write_to_buffer
(d," : | :                                                                   : | :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : | <-------------------------------------------------------------------> | :\n\r",0);
	    write_to_buffer
(d," : + - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < - + :\n\r",0);
	    write_to_buffer
(d," -----------------------------------------------------------------------------\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"                          [ Hit RETURN to Continue ]\n\r",0);
		    d->connected=CON_READ_IMOTD;
		    break;
		case 'i': case 'I':
		    REMOVE_BIT(ch->act, PLR_WIZINVIS);
		    (ch->incog_level = ch->level);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer
(d," -----------------------------------------------------------------------------\n\r",0);
	    write_to_buffer
(d," : + - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < - + :\n\r",0);
	    write_to_buffer
(d," : | <-------------------------------------------------------------------> | :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : - :               IMOTD (Immortal Message of the Day)                 : - :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : | :                         Welcome Immortal!                         : | :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : - :                                                                   : - :\n\r",0);
	    write_to_buffer
(d," : + :  * You are responsible for following the laws of Immortal         : + :\n\r",0);
	    write_to_buffer
(d," : | :    behavior. These can be found by typing 'HELP LAWS' or          : | :\n\r",0);
	    write_to_buffer
(d," : + :    'HELP COMMANDMENTS'.                                           : + :\n\r",0);
	    write_to_buffer
(d," : - :                                                                   : - :\n\r",0);
	    write_to_buffer
(d," : + :  * Excessive loading is not allowed, and is grounds for deletion. : + :\n\r",0);
	    write_to_buffer
(d," : | :                                                                   : | :\n\r",0);
	    write_to_buffer
(d," : + :  * Asking for promotion is grounds for instant deletion.          : + :\n\r",0);
	    write_to_buffer
(d," : - :                                                                   : - :\n\r",0);
	    write_to_buffer
(d," : + :  * Type 'HELP JOBS' to get some idea of what you should be doing. : + :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : | :\n\r",0);
	    write_to_buffer
(d," : - :  * Reading 'HELP TENETS' is a MUST! You must know everything about: + :\n\r",0);
	    write_to_buffer
(d," : + :    it nearly by memory.                                           : - :\n\r",0);
	    write_to_buffer
(d," : | :                                                                   : | :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : | <-------------------------------------------------------------------> | :\n\r",0);
	    write_to_buffer
(d," : + - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < - + :\n\r",0);
	    write_to_buffer
(d," -----------------------------------------------------------------------------\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"                          [ Hit RETURN to Continue ]\n\r",0);
		    d->connected=CON_READ_IMOTD;
		    break;
		case 'n': case 'N':
		    REMOVE_BIT(ch->act, PLR_WIZINVIS);
		    (ch->incog_level = 0);
		    (ch->invis_level = 0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer
(d," -----------------------------------------------------------------------------\n\r",0);
	    write_to_buffer
(d," : + - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < - + :\n\r",0);
	    write_to_buffer
(d," : | <-------------------------------------------------------------------> | :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : - :               IMOTD (Immortal Message of the Day)                 : - :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : | :                         Welcome Immortal!                         : | :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : - :                                                                   : - :\n\r",0);
	    write_to_buffer
(d," : + :  * You are responsible for following the laws of Immortal         : + :\n\r",0);
	    write_to_buffer
(d," : | :    behavior. These can be found by typing 'HELP LAWS' or          : | :\n\r",0);
	    write_to_buffer
(d," : + :    'HELP COMMANDMENTS'.                                           : + :\n\r",0);
	    write_to_buffer
(d," : - :                                                                   : - :\n\r",0);
	    write_to_buffer
(d," : + :  * Excessive loading is not allowed, and is grounds for deletion. : + :\n\r",0);
	    write_to_buffer
(d," : | :                                                                   : | :\n\r",0);
	    write_to_buffer
(d," : + :  * Asking for promotion is grounds for instant deletion.          : + :\n\r",0);
	    write_to_buffer
(d," : - :                                                                   : - :\n\r",0);
	    write_to_buffer
(d," : + :  * Type 'HELP JOBS' to get some idea of what you should be doing. : + :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : | :\n\r",0);
	    write_to_buffer
(d," : - :  * Reading 'HELP TENETS' is a MUST! You must know everything about: + :\n\r",0);
	    write_to_buffer
(d," : + :    it nearly by memory.                                           : - :\n\r",0);
	    write_to_buffer
(d," : | :                                                                   : | :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : | <-------------------------------------------------------------------> | :\n\r",0);
	    write_to_buffer
(d," : + - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < - + :\n\r",0);
	    write_to_buffer
(d," -----------------------------------------------------------------------------\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"                          [ Hit RETURN to Continue ]\n\r",0);
		    d->connected=CON_READ_IMOTD;
		    break;
		default :
		    write_to_buffer (d, " That is not a choice. What IS your choice? ", 0 );
		    break;
		}
	break;

/* RT code for breaking link */
 
    case CON_BREAK_CONNECT:
	switch( *argument )
	{
	case 'y' : case 'Y':
          for ( d_old = descriptor_list; d_old != NULL; d_old = d_next )
	    {
		d_next = d_old->next;
		if (d_old == d || d_old->character == NULL)
		    continue;

		if (str_cmp(ch->name,d_old->original ?
		    d_old->original->name : d_old->character->name))
		    continue;

            if (!IS_NPC(ch) && (!str_cmp(ch->pcdata->lokprot, "on")))
	      {
              ch->pcdata->lokprot = "off";
            }
		close_socket(d_old);
	    }
	    if (check_reconnect(d,ch->name,TRUE))
	    	return;
	    write_to_buffer(d," Reconnect attempt failed.\n\rName: ",0);
          if ( d->character != NULL )
          {
		nuke_pets( d->character );
            free_char( d->character );
            d->character = NULL;
          }
	    d->connected = CON_GET_NAME;
	    break;

	case 'n' : case 'N':
	    write_to_buffer(d,"Name: ",0);
            if ( d->character != NULL )
            {
		    nuke_pets( d->character );
                free_char( d->character );
                d->character = NULL;
            }
	    d->connected = CON_GET_NAME;
	    break;

	default:
	    write_to_buffer(d," Please type [Y]es or [N]o! ",0);
	    break;
	}
	break;

    case CON_CONFIRM_NEW_NAME:
	switch ( *argument )
	{
	case 'y': case 'Y':
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer
(d," -----------------------------------------------------------------------------\n\r",0);
	    write_to_buffer
(d," : + - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < - + :\n\r",0);
	    write_to_buffer
(d," : | <-------------------------------------------------------------------> | :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : - :                                                                   : - :\n\r",0);
	    write_to_buffer
(d," : + :    The old man smiles and then says 'Welcome to uh...  Well...    : + :\n\r",0);
	    write_to_buffer
(d," : | :   Here! Yes...  That's it... 'Here', IS an appropriate name for   : | :\n\r",0);
	    write_to_buffer
(d," : + :                           this place...'                          : + :\n\r",0);
	    write_to_buffer
(d," : - :                                                                   : - :\n\r",0);
	    write_to_buffer
(d," : + :         The old man then gets a serious look in his eyes.         : + :\n\r",0);
	    write_to_buffer
(d," : | :                                                                   : | :\n\r",0);
	    write_to_buffer
(d," : + :    Well, if your going to stay here for a while I will have to    : + :\n\r",0);
	    write_to_buffer
(d," : - :    ask you to give me a password so that I really know it's you   : - :\n\r",0);
	    write_to_buffer
(d," : + :    and not one of those doorknob black robes trying to fool an    : + :\n\r",0);
	    write_to_buffer
(d," : + :                      old man with illusions!'                     : | :\n\r",0);
	    write_to_buffer
(d," : - :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : + :    ( Note: Passwords must be between 5 and 8 characters long )    : - :\n\r",0);
	    write_to_buffer
(d," : - : Only up to 8 characters are recognized, so don't make them longer : + :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : - :\n\r",0);
	    write_to_buffer
(d," : - :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : - :\n\r",0);
	    write_to_buffer
(d," : | <-------------------------------------------------------------------> | :\n\r",0);
	    write_to_buffer
(d," : + - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < - + :\n\r",0);
	    write_to_buffer
(d," -----------------------------------------------------------------------------\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
sprintf( buf, " - %s", echo_off_str );
	    write_to_buffer( d, buf, 0 );
	    d->connected = CON_GET_NEW_PASSWORD;
	    break;

	case 'n': case 'N':
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer( d, " Ok, what IS it, then? ", 0 );
	    nuke_pets( d->character );
	    free_char( d->character );
	    d->character = NULL;
	    d->connected = CON_GET_NAME;
	    break;

	default:
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer( d, " Please type Yes or No! ", 0 );
	    break;
	}
	break;

    case CON_GET_NEW_PASSWORD:
#if defined(unix)
	write_to_buffer( d, "\n\r", 2 );
#endif

	if ( strlen(argument) < 5 )
	{
	write_to_buffer( d," Password must be at least five characters long.\n\rPassword: ",0 );
	return;
	}

	pwdnew = crypt( argument, ch->name );
	for ( p = pwdnew; *p != '\0'; p++ )
	{
	    if ( *p == '~' )
	    {
		write_to_buffer( d,
		    "New password not acceptable, try again.\n\rPassword: ",
		    0 );
		return;
	    }
	}

	free_string( ch->pcdata->pwd );
	ch->pcdata->pwd	= str_dup( pwdnew );
	write_to_buffer( d, " 'Hmm, run that by me again to make sure you said it right'\n\r\n\r", 0 );
      write_to_buffer( d, " - ", 0 );
	d->connected = CON_CONFIRM_NEW_PASSWORD;
	break;

    case CON_CONFIRM_NEW_PASSWORD:
#if defined(unix)
	write_to_buffer( d, "\n\r", 2 );
#endif

	if ( strcmp( crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd ) )
	{
	    write_to_buffer( d, " Passwords don't match.\n\rRetype password: ",
		0 );
	    d->connected = CON_GET_NEW_PASSWORD;
	    return;
	}

/*Get Surname*/
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer
(d," -----------------------------------------------------------------------------\n\r",0);
	    write_to_buffer
(d," : + - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < - + :\n\r",0);
	    write_to_buffer
(d," : | <-------------------------------------------------------------------> | :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : - :                                                                   : - :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : | :                                                                   : | :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : - :                                                                   : - :\n\r",0);
	    write_to_buffer
(d," : + :     The old man begins to smile again until suddenly he stops,    : + :\n\r",0);
	    write_to_buffer
(d," : | :    gets a confused look in his eyes and says to you 'Heh, well,   : | :\n\r",0);
	    write_to_buffer
(d," : + :      er, what was your last name?  I can't seem to remember!'     : + :\n\r",0);
	    write_to_buffer
(d," : - :                                                                   : - :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : - :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : - :\n\r",0);
	    write_to_buffer
(d," : | <-------------------------------------------------------------------> | :\n\r",0);
	    write_to_buffer
(d," : + - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < - + :\n\r",0);
	    write_to_buffer
(d," -----------------------------------------------------------------------------\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
          write_to_buffer( d, " - ", 0 );
        d->connected = CON_GET_SURNAME;
        break;
    case CON_GET_SURNAME:
	switch ( argument[0] )
	{
	case '\0':
	case '\r':
	case '\n':
            sprintf(buf, "\n\n\r Welcome to Krynn, %s!\n\r", ch->name);
            write_to_buffer( d, buf, 0);
	    break;
	default:
	    if( !isalpha(argument[0]))
	    {
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer
(d," -----------------------------------------------------------------------------\n\r",0);
	    write_to_buffer
(d," : + - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < - + :\n\r",0);
	    write_to_buffer
(d," : | <-------------------------------------------------------------------> | :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : - :                                                                   : - :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : | :                                                                   : | :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : - :                                                                   : - :\n\r",0);
	    write_to_buffer
(d," : + :     The old man begins to smile again until suddenly he stops,    : + :\n\r",0);
	    write_to_buffer
(d," : | :    gets a confused look in his eyes and says to you 'Heh, well,   : | :\n\r",0);
	    write_to_buffer
(d," : + :      er, what was your last name?  I can't seem to remember!'     : + :\n\r",0);
	    write_to_buffer
(d," : - :                                                                   : - :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : - :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : - :\n\r",0);
	    write_to_buffer
(d," : | <-------------------------------------------------------------------> | :\n\r",0);
	    write_to_buffer
(d," : + - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < - + :\n\r",0);
	    write_to_buffer
(d," -----------------------------------------------------------------------------\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
          write_to_buffer( d, " - ", 0 );
        	return;
	    }
	    argument[0] = UPPER(argument[0]);
        if ( !check_parse_surname( argument ) )
        {
            write_to_buffer( d, " Illegal Surname, try another.\n\rSurname: ", 0 );
            return;
        }
	    ch->surname = str_dup(argument);
            sprintf(buf, "\n\n\r 'Oh, that's right! Welcome to Krynn, %s %s!'\n\r", ch->name, ch->surname);
            write_to_buffer( d, buf, 0);
	    break;
	}

	write_to_buffer(d,"\n\r",2);
	write_to_buffer( d, echo_on_str, 0 );
      sprintf(buf, " 'Oh, that's right! Welcome to Krynn, %s %s!'\n\r\n\r", ch->name, ch->surname);
      write_to_buffer( d, buf, 0);
	write_to_buffer(d," The old man's confused face turns to that of one of pondering and \n\r",0);
	write_to_buffer(d," asks you 'What manner of creature ARE you?  I've seen many creatures \n\r",0);
	write_to_buffer(d," here on Krynn. Here's a list of them in no particular order:\n\r",0);
	write_to_buffer(d,
	 "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n\r",0);
	write_to_buffer(d,"\n\r",0);
	for ( race = 1; race_table[race].desc != NULL; race++ )
	{
	    if (!race_table[race].pc_race)
		break;
	    write_to_buffer(d,race_table[race].desc,0);
	    write_to_buffer(d,"\n\r",0);
	}
	write_to_buffer(d,"\n\r",0);
	write_to_buffer(d," What is your race? (Type help <RACE> for more information)\n\r\n\r",0);
      write_to_buffer( d, " - ", 0 );
	d->connected = CON_GET_NEW_RACE;
	break;

    case CON_GET_NEW_RACE:
	one_argument(argument,arg);

	if (!strcmp(arg,"help"))
	{
	    argument = one_argument(argument,arg);
	    if (argument[0] == '\0')
		do_function(ch, &do_help, "race help");
	    else
		do_function(ch, &do_help, argument);
	      write_to_buffer(d,"\n\r",0);
            write_to_buffer(d,
		" What is your race (Type help <RACE> for more information)?\n\r\n\r",0);
            write_to_buffer( d, " - ", 0 );
	    break;
  	}

	race = race_lookup(argument);

	if (race == 0 || !race_table[race].pc_race)
	{
	    write_to_buffer(d,"\n\r",2);
	    write_to_buffer(d," That is not a valid race.\n\r",0);
          write_to_buffer(d," The following races are available:\n\r",0);
	    write_to_buffer(d,
	 "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n\r",0);
	    write_to_buffer(d,"\n\r",0);
            for ( race = 1; race_table[race].desc != NULL; race++ )
            {
            	if (!race_table[race].pc_race)
                    break;
            	write_to_buffer(d,race_table[race].desc,0);
            	write_to_buffer(d,"\n\r",0);
            }
            write_to_buffer(d,"\n\r",0);
            write_to_buffer(d,
		"What is your race? (help <RACE> for more information) ",0);
	    break;
	}

        ch->race = race;
	/* initialize stats */
	for (i = 0; i < MAX_STATS; i++)
	ch->perm_stat[i] 	= pc_race_table[race].stats[i];
	ch->affected_by 	= ch->affected_by|race_table[race].aff;
	ch->imm_flags	= ch->imm_flags|race_table[race].imm;
	ch->res_flags	= ch->res_flags|race_table[race].res;
	ch->vuln_flags	= ch->vuln_flags|race_table[race].vuln;
	ch->form		= race_table[race].form;
	ch->parts		= race_table[race].parts;
	ch->pcdata->learned[ gsn_lang_common ] = 100;
	/* add skills */
	for (i = 0; i < 5; i++)
	{
            if (pc_race_table[race].skills[i] == NULL)
	 	break;
          ch->pcdata->learned[skill_lookup(pc_race_table[race].skills[i])] = 100;
            
	}
	/* add cost */
	ch->pcdata->points = pc_race_table[race].points;
	ch->size = pc_race_table[race].size;

	/*  send_to_desc ("\x01B[2J", d); */
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer
(d," -----------------------------------------------------------------------------\n\r",0);
	    write_to_buffer
(d," : + - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < - + :\n\r",0);
	    write_to_buffer
(d," : | <-------------------------------------------------------------------> | :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : - :                                                                   : - :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : | :                                                                   : | :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : - :   The old man begins to chuckle and says to you 'While we're on   : - :\n\r",0);
	    write_to_buffer
(d," : + :   the subject of 'What manner of creature are you?' You'd better  : + :\n\r",0);
	    write_to_buffer
(d," : | :   tell an old man...  Umm.. Well' The old man begins to blush at  : | :\n\r",0);
	    write_to_buffer
(d," : + :   this point. 'I do have a hard time seeing and I've forgotten my : + :\n\r",0);
	    write_to_buffer
(d," : - :   reading spectecles about here somewhere, Would you be a MALE or : - :\n\r",0);
	    write_to_buffer
(d," : + :                            a FEMALE?'                             : + :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : - :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : - :\n\r",0);
	    write_to_buffer
(d," : | <-------------------------------------------------------------------> | :\n\r",0);
	    write_to_buffer
(d," : + - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < - + :\n\r",0);
	    write_to_buffer
(d," -----------------------------------------------------------------------------\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
          write_to_buffer( d, " - ", 0 );
        d->connected = CON_GET_NEW_SEX;
        break;
        

    case CON_GET_NEW_SEX:
	switch ( argument[0] )
	{
	case 'm': case 'M': ch->sex = SEX_MALE;    
			    ch->pcdata->true_sex = SEX_MALE;
			    break;
	case 'f': case 'F': ch->sex = SEX_FEMALE; 
			    ch->pcdata->true_sex = SEX_FEMALE;
			    break;
	default:
	    write_to_buffer( d, "\n\r", 0 );
	    write_to_buffer( d, " That's not a gender.\n\r", 0 );
	    write_to_buffer( d, " Which gender do you wish to be?\n\r", 0 );
          write_to_buffer( d, " [M]ale or [F]emale? ", 0 );
	    return;
	}
	write_to_buffer(d,"\n\r",2);
	write_to_buffer( d, " The old man smiles again and says 'Ah thank you for helping an old man!\n\r", 0 );
	write_to_buffer( d, " By the way, what do you do for a living? \n\r", 0 );
	write_to_buffer( d, "\n\r", 0 );
	strcpy( buf, "The following classes are available to you:\n\r" );
	write_to_buffer(d,
	 "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n\r",0);
	write_to_buffer(d,"\n\r",0);
	for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
	{
	    if(
               (class_table[iClass].human  && ch->race == 1)
            || (class_table[iClass].dracon && ch->race == 2)
            || (class_table[iClass].helf   && ch->race == 3)
            || (class_table[iClass].silvan && ch->race == 4)
            || (class_table[iClass].qualin && ch->race == 5)
            || (class_table[iClass].kagon  && ch->race == 6)
            || (class_table[iClass].dargon && ch->race == 7)
            || (class_table[iClass].dimern && ch->race == 8)
            || (class_table[iClass].neidar && ch->race == 9)
            || (class_table[iClass].hylar  && ch->race == 10)
            || (class_table[iClass].theiwa && ch->race == 11)
            || (class_table[iClass].aghar  && ch->race == 12)
            || (class_table[iClass].daerga && ch->race == 13)
            || (class_table[iClass].daewar && ch->race == 14)
            || (class_table[iClass].klar   && ch->race == 15)
            || (class_table[iClass].gnome  && ch->race == 16)
            || (class_table[iClass].kender && ch->race == 17)
            || (class_table[iClass].mino   && ch->race == 18)
            || (class_table[iClass].ogre   && ch->race == 19))
            {    
                strcat( buf,"\n\r");
                strcat( buf, class_table[iClass].desc );
            }
	}
      strcat( buf,"\n\r");
	strcat( buf, "\n\rWhat is your profession?: ");
	write_to_buffer( d, buf, 0 );
	d->connected = CON_GET_NEW_CLASS;
	break;

    case CON_GET_NEW_CLASS:
	iClass = class_lookup(argument);

	if ( iClass == -1 
            || (!class_table[iClass].human  && ch->race == 1)
            || (!class_table[iClass].dracon && ch->race == 2)
            || (!class_table[iClass].helf   && ch->race == 3)
            || (!class_table[iClass].silvan && ch->race == 4)
            || (!class_table[iClass].qualin && ch->race == 5)
            || (!class_table[iClass].kagon  && ch->race == 6)
            || (!class_table[iClass].dargon && ch->race == 7)
            || (!class_table[iClass].dimern && ch->race == 8)
            || (!class_table[iClass].neidar && ch->race == 9)
            || (!class_table[iClass].hylar  && ch->race == 10)
            || (!class_table[iClass].theiwa && ch->race == 11)
            || (!class_table[iClass].aghar  && ch->race == 12)
            || (!class_table[iClass].daerga && ch->race == 13)
            || (!class_table[iClass].daewar && ch->race == 14)
            || (!class_table[iClass].klar   && ch->race == 15)
            || (!class_table[iClass].gnome  && ch->race == 16)
            || (!class_table[iClass].kender && ch->race == 17)
            || (!class_table[iClass].mino   && ch->race == 18)
            || (!class_table[iClass].ogre   && ch->race == 19))
	{
           	write_to_buffer(d,"\n\r",2);
	      write_to_buffer( d, "That's not a class.\n\r", 0 );
		strcpy( buf, "The following classes are available to you:\n\r" );
	write_to_buffer(d,
	 "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n\r",0);
        	for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
        	{
	           if(
                      (class_table[iClass].human  && ch->race == 1)
                   || (class_table[iClass].dracon && ch->race == 2)
                   || (class_table[iClass].helf   && ch->race == 3)
                   || (class_table[iClass].silvan && ch->race == 4)
                   || (class_table[iClass].qualin && ch->race == 5)
                   || (class_table[iClass].kagon  && ch->race == 6)
                   || (class_table[iClass].dargon && ch->race == 7)
                   || (class_table[iClass].dimern && ch->race == 8)
                   || (class_table[iClass].neidar && ch->race == 9)
                   || (class_table[iClass].hylar  && ch->race == 10)
                   || (class_table[iClass].theiwa && ch->race == 11)
                   || (class_table[iClass].aghar  && ch->race == 12)
               	 || (class_table[iClass].daerga && ch->race == 13)
            	 || (class_table[iClass].daewar && ch->race == 14)
           	 	 || (class_table[iClass].klar   && ch->race == 15)
            	 || (class_table[iClass].gnome  && ch->race == 16)
            	 || (class_table[iClass].kender && ch->race == 17)
            	 || (class_table[iClass].mino   && ch->race == 18)
            	 || (class_table[iClass].ogre   && ch->race == 19))
                   {    
                		strcat( buf,"\n\r");
                		strcat( buf, class_table[iClass].desc );
                   }
	        }
            strcat( buf,"\n\r");
		strcat( buf, "\n\rWhat is your choice?: ");
        	write_to_buffer( d, buf, 0 );
            
	    return;
	}

        ch->class = iClass;

	sprintf( log_buf, "%s@%s new player.", ch->name, d->host );
	log_string( log_buf );
	wiznet("Newbie alert!  $N sighted.",ch,NULL,WIZ_NEWBIE,0,0);
        wiznet(log_buf,NULL,NULL,WIZ_SITES,0,get_trust(ch));

	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer
(d," -----------------------------------------------------------------------------\n\r",0);
	    write_to_buffer
(d," : + - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < - + :\n\r",0);
	    write_to_buffer
(d," : | <-------------------------------------------------------------------> | :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : - :      The old man nods his head and says 'Ah, yes yes, a fine      : - :\n\r",0);
	    write_to_buffer
(d," : + :     profession that is!  Now, tell an old man where your from!    : + :\n\r",0);
	    write_to_buffer
(d," : | :                      Let's not be strangers!'                     : | :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : - :    There are many cities on Krynn. Here are just a few that are   : - :\n\r",0);
	    write_to_buffer
(d," : + :   available to you, for choosing a permanant place of living. We  : + :\n\r",0);
	    write_to_buffer
(d," : | :    understand that most cities are inhabited by specific races,   : | :\n\r",0);
	    write_to_buffer
(d," : + :    however, most cities on Krynn have races of ALL types living   : + :\n\r",0);
	    write_to_buffer
(d," : - :  within their city walls. With that, we do not restrict any race  : - :\n\r",0);
	    write_to_buffer
(d," : + :    to any specific hometown. These cities are open to everyone.   : + :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : | :\n\r",0);
	    write_to_buffer
(d," : - :                      Please select a Hometown:                    : + :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : - :\n\r",0);
	    write_to_buffer
(d," : | <-------------------------------------------------------------------> | :\n\r",0);
	    write_to_buffer
(d," : + - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < - + :\n\r",0);
	    write_to_buffer
(d," -----------------------------------------------------------------------------\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	for (i=0;hometown_table[i].name != NULL; ++i)
	{
		sprintf(buf," [%-15s]\n\r", hometown_table[i].name );
		write_to_buffer( d, buf, 0 );
	}
	write_to_buffer( d, "\n\r",0);
	write_to_buffer( d, "\n\r What's your hometown going to be? ", 0);
	d->connected = CON_GET_HOMETOWN;
	break;

    case CON_GET_HOMETOWN:

    	if (get_hometown(argument) == -1)
    	{
      	write_to_buffer( d, "\n\r",2);
    		write_to_buffer( d, "\n\rThat's not a valid selection.\n\r",0);
    		write_to_buffer( d, "Valid selections are:\n\r",0);
      	write_to_buffer( d, "\n\r",2);
		for (i=0;hometown_table[i].name != NULL; ++i)
		{
			sprintf(buf," [%-15s]\n\r", hometown_table[i].name );
			write_to_buffer( d, buf, 0 );
		}
		write_to_buffer( d, "\n\r",2);
    		write_to_buffer( d, "\n\r What's your hometown going to be? ", 0);
    		return;
    	}
	ch->hometown = get_hometown(argument);
      ch->recall_point = hometown_table[ch->hometown].recall;

	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer
(d," -----------------------------------------------------------------------------\n\r",0);
	    write_to_buffer
(d," : + - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < - + :\n\r",0);
	    write_to_buffer
(d," : | <-------------------------------------------------------------------> | :\n\r",0);
	    write_to_buffer
(d," : + : 'A fine place that is!'  The old man tells you.  'Been there many : + :\n\r",0);
	    write_to_buffer
(d," : - :     a time in my wide travels!  Now then I'm a very *snicker*     : - :\n\r",0);
	    write_to_buffer
(d," : + :  religous old man, and I'd like to know which of the three faiths : + :\n\r",0);
	    write_to_buffer
(d," : | :                            you follow!                            : | :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : - :  -Good aligns should regard life as sacred and seek only to kill  : - :\n\r",0);
	    write_to_buffer
(d," : + :   those who are evil or who threaten them.                        : + :\n\r",0);
	    write_to_buffer
(d," : | :                                                                   : | :\n\r",0);
	    write_to_buffer
(d," : + :  -Neutrals are stand-ins between good and evil, and base their    : + :\n\r",0);
	    write_to_buffer
(d," : - :   actions upon the needs of the moment without going to extremes  : - :\n\r",0);
	    write_to_buffer
(d," : + :   of selflessness or selfishness.                                 : + :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : | :\n\r",0);
	    write_to_buffer
(d," : - :  -Evils regard the lives of others as irrelevant and do not base  : + :\n\r",0);
	    write_to_buffer
(d," : + :   their actions on any real concern regarding the lives of others.: - :\n\r",0);
	    write_to_buffer
(d," : | <-------------------------------------------------------------------> | :\n\r",0);
	    write_to_buffer
(d," : + - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < - + :\n\r",0);
	    write_to_buffer
(d," -----------------------------------------------------------------------------\n\r",0);

      write_to_buffer( d, "\n\r",0);
      write_to_buffer( d, "\n\r",0);
      if (( ch->class == class_lookup("necromancer"))
	||  ( ch->class == class_lookup("assassin"))
      ||  ( ch->race == race_lookup("draconian")))
      {
	write_to_buffer( d, "You may be only of EVIL alignment.\n\r",0);
      write_to_buffer( d, "\n\r",0);
	write_to_buffer( d, "Choose your alignment: [E]vil ",0);
      }
	else
      {
	write_to_buffer( d, "You may be good, neutral, or evil.\n\r",0);
      write_to_buffer( d, "\n\r",0);
	write_to_buffer( d, "Choose your alignment: [G]ood | [N]eutral | [E]vil ",0);
      }
	d->connected = CON_GET_ALIGNMENT;
	break;

case CON_GET_ALIGNMENT:
	switch( argument[0])
	{
	    case 'g' :
	    case 'G' :
      	    if (( ch->class == class_lookup("necromancer"))
		    ||  ( ch->class == class_lookup("assassin"))
		    ||  ( ch->race  == race_lookup ("draconian")))
      	    {
		      write_to_buffer( d, "You may be only of EVIL alignment.\n\r",0);
      		write_to_buffer( d, "\n\r",0);
			write_to_buffer( d, "Choose your alignment: [E]vil ",0);
			return;
		    }
                ch->alignment = 750;  break;
	    case 'n' :
	    case 'N' :
      	    if (( ch->class == class_lookup("necromancer"))
		    ||  ( ch->class == class_lookup("assassin"))
		    ||  ( ch->race  == race_lookup ("draconian")))
      	    {
		      write_to_buffer( d, "You may be only of EVIL alignment.\n\r",0);
      		write_to_buffer( d, "\n\r",0);
			write_to_buffer( d, "Choose your alignment: [E]vil ",0);
			return;
		    }
		    ch->alignment = 0;	break;
	    case 'e' :
	    case 'E' :
		    ch->alignment = -750; break;
	    default:
	      write_to_buffer(d,"\n\r",0);
		write_to_buffer(d,"That's not a valid alignment.\n\r",0);
	      write_to_buffer(d,"\n\r",0);
      	if (( ch->class == class_lookup("necromancer"))
		||  ( ch->class == class_lookup("assassin"))
		||  ( ch->race  == race_lookup ("draconian")))
      	{
		write_to_buffer( d, "You may be only of EVIL alignment.\n\r",0);
      	write_to_buffer( d, "\n\r",0);
		write_to_buffer( d, "Choose your alignment: [E]vil ",0);
      	}
		else
      	{
      write_to_buffer( d, "\n\r",0);
      	write_to_buffer( d, "\n\r",0);
		write_to_buffer( d, "Choose your alignment: [G]ood | [N]eutral | [E]vil ",0);
      	}
		return;
	}
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer
(d," -----------------------------------------------------------------------------\n\r",0);
	    write_to_buffer
(d," : + - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < - + :\n\r",0);
	    write_to_buffer
(d," : | <-------------------------------------------------------------------> | :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : + :\n\r",0);
	if (ch->alignment == 750)
	{
        write_to_buffer
(d," : - :    'Ahh, a kind-hearted soul!  I knew it when I first saw you!    : - :\n\r",0);
	}
	else if (ch->alignment == 0)
	{
	   write_to_buffer
(d," : - :          'Ah yes, always a need for balance you know!             : - :\n\r",0);
	}
	else
	{
	   write_to_buffer
(d," : - : 'Hmm, a dangerous faith in these times, you'd best watch yourself!: - :\n\r",0);
	}
	    write_to_buffer
(d," : + :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : | :     Now onto the politics, how would you describe yourself?'      : | :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : - : ETHOS is an indication of your character's reaction towards laws. : - :\n\r",0);
	    write_to_buffer
(d," : + :  It has three values: lawful, neutral, and chaotic.  It is quite  : + :\n\r",0);
	    write_to_buffer
(d," : | :  acceptable for evil characters to be lawful and good characters  : | :\n\r",0);
	    write_to_buffer
(d," : + :  to be chaotic. These represent each character's understanding of : + :\n\r",0);
	    write_to_buffer
(d," : - :                   society and relationships.                      : - :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : + :                      Choose your Ethos:                           : | :\n\r",0);
	    write_to_buffer
(d," : - :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : + :          You can be (L)awful, (N)eutral, or (C)haotic.            : - :\n\r",0);
	    write_to_buffer
(d," : | <-------------------------------------------------------------------> | :\n\r",0);
	    write_to_buffer
(d," : + - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < - + :\n\r",0);
	    write_to_buffer
(d," -----------------------------------------------------------------------------\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
      write_to_buffer( d, " Choice: ",0);
      d->connected = CON_GET_ETHOS;
      break;

    case CON_GET_ETHOS:
        switch( argument[0])
        {
            case 'l' : case 'L' :
                ch->ethos = ETHOS_LAWFUL;  
		break;
            case 'n' : case 'N' :
                ch->ethos = ETHOS_NEUTRAL;      
		break;
            case 'c' : case 'C' :
                ch->ethos = ETHOS_CHAOTIC; 
		break;
            default:
	        write_to_buffer(d,"\n\r",0);
              write_to_buffer( d," That's not a valid ethos.",0);
	        write_to_buffer( d,"\n\n\n\r Choose your ethos:\n\r",0);
	        write_to_buffer( d,"\n\r You can be (L)awful, (N)eutral, or (C)haotic.",0);
       	  write_to_buffer( d,"\n\r Choice: ",0);
              d->connected = CON_GET_ETHOS;
		return;
        }

	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer
(d," -----------------------------------------------------------------------------\n\r",0);
	    write_to_buffer
(d," : + - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < - + :\n\r",0);
	    write_to_buffer
(d," : | <-------------------------------------------------------------------> | :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : - :                                                                   : - :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : | :   The old man looks at you with his piercing eyes once more and   : | :\n\r",0);
	    write_to_buffer
(d," : + :    says to you 'Well now, don't let an old man keep you here!     : + :\n\r",0);
	    write_to_buffer
(d," : - :  You've great things to do! I can feel it!  Goodbye for now, I'm  : - :\n\r",0);
	    write_to_buffer
(d," : + :    sure we'll meet again later.  What's that?  You want to know   : + :\n\r",0);
	    write_to_buffer
(d," : | :   about me?  Now now, you are rather nosey aren't you! Go along   : | :\n\r",0);
	    write_to_buffer
(d," : + :  now, shoo!  You have things to do!'  Grinning mischeviously the  : + :\n\r",0);
	    write_to_buffer
(d," : - :  old man walks away, you watch him walk away, shake your head and : - :\n\r",0);
	    write_to_buffer
(d," : + :                 turn back to what you were doing...               : + :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : - :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : - :\n\r",0);
	    write_to_buffer
(d," : | <-------------------------------------------------------------------> | :\n\r",0);
	    write_to_buffer
(d," : + - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < - + :\n\r",0);
	    write_to_buffer
(d," -----------------------------------------------------------------------------\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
      write_to_buffer(d,"                          [ Hit RETURN to Continue ] ",0);
	d->connected = CON_GET_NEXT;
      break;

    case CON_GET_NEXT:
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer
(d," -----------------------------------------------------------------------------\n\r",0);
	    write_to_buffer
(d," : + - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < - + :\n\r",0);
	    write_to_buffer
(d," : | <-------------------------------------------------------------------> | :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : - :                                                                   : - :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : | :                                                                   : | :\n\r",0);
	    write_to_buffer
(d," : + :   The Legends of Krynn does not offer God selection in creation   : + :\n\r",0);
	    write_to_buffer
(d," : - :  anymore. If you wish to worship a Diety, throughout the MUD you  : - :\n\r",0);
	    write_to_buffer
(d," : + :  will find Temples built in an Immortal's name. Find the Temple   : + :\n\r",0);
	    write_to_buffer
(d," : | :  of the Immortal you wish to worship, find an Altar of some sort  : | :\n\r",0);
	    write_to_buffer
(d," : + :   and DEVOTE yourself to that God, using the command DEVOTE. It   : + :\n\r",0);
	    write_to_buffer
(d," : - :            would look something like, DEVOTE HIDDUKEL.            : - :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : - :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : - :\n\r",0);
	    write_to_buffer
(d," : | <-------------------------------------------------------------------> | :\n\r",0);
	    write_to_buffer
(d," : + - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < - + :\n\r",0);
	    write_to_buffer
(d," -----------------------------------------------------------------------------\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
      write_to_buffer(d,"                          [ Hit RETURN to Continue ] ",0);
	d->connected = CON_GET_NEXT2;
	break;


    case CON_GET_NEXT2:
	write_to_buffer(d,"\n\r",2);
      if (ch->desc->ansi)
          SET_BIT(ch->act, PLR_COLOUR);
      else REMOVE_BIT(ch->act, PLR_COLOUR);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer
(d," -----------------------------------------------------------------------------\n\r",0);
	    write_to_buffer
(d," : + - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < - + :\n\r",0);
	    write_to_buffer
(d," : | <-------------------------------------------------------------------> | :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : - :                                                                   : - :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : | :                                                                   : | :\n\r",0);
	    write_to_buffer
(d," : + :   Please enter in an e-mail address at this time. If you do not   : + :\n\r",0);
	    write_to_buffer
(d," : - :   wish to enter one in now, you may enter one in later on in the  : - :\n\r",0);
	    write_to_buffer
(d," : + :    game using our EMAIL command. You will never be required to    : + :\n\r",0);
	    write_to_buffer
(d," : | :  enter in an e-mail address here, however, entering one makes it  : | :\n\r",0);
	    write_to_buffer
(d," : + : a lot easier for us to inform you of any changes that takes place : + :\n\r",0);
	    write_to_buffer
(d," : - : here at Legends of Krynn. If you do not wish to enter one in now, : - :\n\r",0);
	    write_to_buffer
(d," : + :                        simply hit RETURN.                         : + :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : - :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : - :\n\r",0);
	    write_to_buffer
(d," : | <-------------------------------------------------------------------> | :\n\r",0);
	    write_to_buffer
(d," : + - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < - + :\n\r",0);
	    write_to_buffer
(d," -----------------------------------------------------------------------------\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
      write_to_buffer(d,": ",0);
	d->connected = CON_GET_EMAIL;
	break;

    case CON_GET_EMAIL:
		{		
		int pos;
		int approved=FALSE;
		if (IS_NULLSTR(argument))
		{
			write_to_buffer(d, "No email address will be on file for you then.\n\r", 0);
		  d->character->pcdata->email = str_dup("none");
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer
(d," -----------------------------------------------------------------------------\n\r",0);
	    write_to_buffer
(d," : + - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < - + :\n\r",0);
	    write_to_buffer
(d," : | <-------------------------------------------------------------------> | :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : - :                      MOTD (Message of the Day)                    : - :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : | :  * You are responsible for knowing the rules (type 'rules').      : | :\n\r",0);
	    write_to_buffer
(d," : + :    Ignorance of the rules is no excuse.                           : + :\n\r",0);
	    write_to_buffer
(d," : - :                                                                   : - :\n\r",0);
	    write_to_buffer
(d," : + :  * Important commands: HELP, COMMANDS, NOTES, IDEAS, CHANGES,     : + :\n\r",0);
	    write_to_buffer
(d," : | :    RULES                                                          : | :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : - :  * Use the DELETE command to erase unwanted characters            : - :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : | :  * You MUST have a description for your character by level 10!     : | :\n\r",0);
	    write_to_buffer
(d," : + :    Failure to do so may result in imprisonment until one is      : + :\n\r",0);
	    write_to_buffer
(d," : - :    written!                                                       : - :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : + :  * Roleplaying is not only encouraged here, but without it you    : | :\n\r",0);
	    write_to_buffer
(d," : - :    will be denied, and then banned if no improvement is found.    : + :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : - :\n\r",0);
	    write_to_buffer
(d," : | :  * Make sure you Read and Understand HELP TENETS and HELP MISSION : | :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : | <-------------------------------------------------------------------> | :\n\r",0);
	    write_to_buffer
(d," : + - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < - + :\n\r",0);
	    write_to_buffer
(d," -----------------------------------------------------------------------------\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"                          [ Hit RETURN to Continue ]\n\r",0);
            if (ch->level == 0)
		  d->connected = CON_PREAD_MOTD;
            else
	        d->connected = CON_READ_MOTD;

              break;
		}
		if (!strchr(argument, '@'))
		{
			write_to_buffer(d, "\n\r", 0);
			write_to_buffer(d, "I suspect your email address to be invalid.\n\r", 0);
			write_to_buffer(d, "Please enter a valid address: \n\r", 0);
			return;
		}
		if (strchr(argument, '~'))
		{
			write_to_buffer(d, "\n\r", 0);
			write_to_buffer(d, "No tilde allowed for political reasons. Get a real email account.\n\r", 0);
			write_to_buffer(d, "Please enter a valid address: \n\r", 0);
			return;
		}
		sprintf(buf, "%s", argument);
		for(pos = strlen(argument)-2; pos >= 0; pos--)
		{
			if (buf[pos] == '.')
			{	if (isdigit(buf[pos+1])) {approved = TRUE; break;}
				else
				{
					if (pos+2 < strlen(argument))
					{
						if (isalpha(buf[pos+1]) && isalpha(buf[pos+2]))
						{
							approved = TRUE; break;
						}
					}
				}
			}
		}
		if (approved!=TRUE)
		{
		write_to_buffer(d, "\n\r", 0);
		write_to_buffer(d, "Your domain name was deemed invalid by the parser. Try and be a little more creative.\n\r", 0);
		write_to_buffer(d, "Please enter a valid address: \n\r", 0);
		return;
		}
		d->character->pcdata->email = str_dup(argument);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer
(d," -----------------------------------------------------------------------------\n\r",0);
	    write_to_buffer
(d," : + - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < - + :\n\r",0);
	    write_to_buffer
(d," : | <-------------------------------------------------------------------> | :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : - :                      MOTD (Message of the Day)                    : - :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : | :  * You are responsible for knowing the rules (type 'rules').      : | :\n\r",0);
	    write_to_buffer
(d," : + :    Ignorance of the rules is no excuse.                           : + :\n\r",0);
	    write_to_buffer
(d," : - :                                                                   : - :\n\r",0);
	    write_to_buffer
(d," : + :  * Important commands: HELP, COMMANDS, NOTES, IDEAS, CHANGES,     : + :\n\r",0);
	    write_to_buffer
(d," : | :    RULES                                                          : | :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : - :  * Use the DELETE command to erase unwanted characters            : - :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : | :  * You MUST have a description for your character by level 5!     : | :\n\r",0);
	    write_to_buffer
(d," : + :    Failure to do so will result in imprisonment until one is      : + :\n\r",0);
	    write_to_buffer
(d," : - :    written!                                                       : - :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : + :  * Roleplaying is not only encouraged here, but without it you    : | :\n\r",0);
	    write_to_buffer
(d," : - :    will be denied, and then banned if no improvement is found.    : + :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : - :\n\r",0);
	    write_to_buffer
(d," : | :  * Make sure you Read and Understand HELP TENETS and HELP MISSION : | :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : | <-------------------------------------------------------------------> | :\n\r",0);
	    write_to_buffer
(d," : + - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < - + :\n\r",0);
	    write_to_buffer
(d," -----------------------------------------------------------------------------\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"                          [ Hit RETURN to Continue ]\n\r",0);
		if (ch->level == 0)
		  d->connected = CON_PREAD_MOTD;
            else
	        d->connected = CON_READ_MOTD;
		}
		break;

    case CON_READ_IMOTD:
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer
(d," -----------------------------------------------------------------------------\n\r",0);
	    write_to_buffer
(d," : + - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < - + :\n\r",0);
	    write_to_buffer
(d," : | <-------------------------------------------------------------------> | :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : - :                      MOTD (Message of the Day)                    : - :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : | :  * You are responsible for knowing the rules (type 'rules').      : | :\n\r",0);
	    write_to_buffer
(d," : + :    Ignorance of the rules is no excuse.                           : + :\n\r",0);
	    write_to_buffer
(d," : - :                                                                   : - :\n\r",0);
	    write_to_buffer
(d," : + :  * Important commands: HELP, COMMANDS, NOTES, IDEAS, CHANGES,     : + :\n\r",0);
	    write_to_buffer
(d," : | :    RULES                                                          : | :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : - :  * Use the DELETE command to erase unwanted characters            : - :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : | :  * You MUST have a description for your character by level 5!     : | :\n\r",0);
	    write_to_buffer
(d," : + :    Failure to do so will result in imprisonment until one is      : + :\n\r",0);
	    write_to_buffer
(d," : - :    written!                                                       : - :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : + :  * Roleplaying is not only encouraged here, but without it you    : | :\n\r",0);
	    write_to_buffer
(d," : - :    will be denied, and then banned if no improvement is found.    : + :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : - :\n\r",0);
	    write_to_buffer
(d," : | :  * Make sure you Read and Understand HELP TENETS and HELP MISSION : | :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : | <-------------------------------------------------------------------> | :\n\r",0);
	    write_to_buffer
(d," : + - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < - + :\n\r",0);
	    write_to_buffer
(d," -----------------------------------------------------------------------------\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"                          [ Hit RETURN to Continue ]\n\r",0);
        d->connected = CON_READ_MOTD;
	break;

    case CON_EMAIL_TO:
        handle_email_to ( d, argument );
        break;

    case CON_EMAIL_SUBJECT:
        handle_email_subject ( d, argument );
        break;

    case CON_EMAIL_BODY:
        handle_email_body ( d, argument );
        break;

    case CON_EMAIL_FINISH:
        handle_email_finish ( d, argument );
        break;

    case CON_PREAD_MOTD:
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer
(d," -----------------------------------------------------------------------------\n\r",0);
	    write_to_buffer
(d," : + - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < - + :\n\r",0);
	    write_to_buffer
(d," : | <-------------------------------------------------------------------> | :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : - :   Ah!  Another mortal, seeking adventure.  You should go through  : - :\n\r",0);
	    write_to_buffer
(d," : + :  mudschool first, to gain experience to face the trials outside.  : + :\n\r",0);
	    write_to_buffer
(d," : | :    Be sure to practice a weapon in the guild room, or your stay   : | :\n\r",0);
	    write_to_buffer
(d," : + :  with us shall be very short indeed. Type 'EQUIPMENT' to see what : + :\n\r",0);
	    write_to_buffer
(d," : - :   you're wielding, 'SPELLS' to see your spells, 'SKILLS' to see   : - :\n\r",0);
	    write_to_buffer
(d," : + :   your skills, and 'COMMANDS' for a list of commands.  Help is    : + :\n\r",0);
	    write_to_buffer
(d," : | :             available on most commands and abilities.             : | :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : - :    This is a roleplaying mud that does allow playerkilling, but   : - :\n\r",0);
	    write_to_buffer
(d," : + :   requires roleplaying reasons to do so.  If you are interested   : + :\n\r",0);
	    write_to_buffer
(d," : | : in roleplaying, you are welcome here.  If you are only interested : | :\n\r",0);
	    write_to_buffer
(d," : + :      in powergaming and playerkilling, try a different mud.       : + :\n\r",0);
	    write_to_buffer
(d," : - :                                                                   : - :\n\r",0);
	    write_to_buffer
(d," : + :       A note for newbies who have never played a MUD before:      : + :\n\r",0);
	    write_to_buffer
(d," : + : Greater and less than signs (<>) have been used above to set apart: | :\n\r",0);
	    write_to_buffer
(d," : - :  the commands and your personalized input from the explanations   : + :\n\r",0);
	    write_to_buffer
(d," : + : of them.  The shorthand <text> is used for a line of text of your : - :\n\r",0);
	    write_to_buffer
(d," : | :                             choosing.                             : | :\n\r",0);
	    write_to_buffer
(d," : + :                                                                   : + :\n\r",0);
	    write_to_buffer
(d," : - :   Everything inside the quotation marks ("")denotes exactly what  : - :\n\r",0);
	    write_to_buffer
(d," : + :               you need to type for any given command.             : + :\n\r",0);
	    write_to_buffer
(d," : | :                                                                   : | :\n\r",0);
	    write_to_buffer
(d," : + :       Please type 'HELP RULES' to see the Rules of this MUD.      : + :\n\r",0);
	    write_to_buffer
(d," : | <-------------------------------------------------------------------> | :\n\r",0);
	    write_to_buffer
(d," : + - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < | > - < - + :\n\r",0);
	    write_to_buffer
(d," -----------------------------------------------------------------------------\n\r",0);
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"\n\r",0);
      write_to_buffer(d,"                          [ Hit RETURN to Continue ] ",0);
	d->connected = CON_READ_MOTD;
	break;

    case CON_READ_MOTD:
	/* See if they are a devote already */
	obj = get_eq_char(ch, WEAR_MARK);
	if (obj && !ch->religion_id)
	{
	  for ( i = 1 ; i < MAX_RELIGION ; i++ )
	  {
	    if (religion_table[i].devoteevnum == obj->pIndexData->vnum )
	    {
		ch->devotee_id = i;
		  break;
	    }
	  }
	}
      if ( ch->pcdata == NULL || ch->pcdata->pwd[0] == '\0')
      {
        write_to_buffer( d, "Warning! Null password!\n\r",0 );
        write_to_buffer( d, "Please report old password with bug.\n\r",0);
        write_to_buffer( d, "Type 'password null <new password>' to fix.\n\r",0);
      }
	write_to_buffer(d,"\n\r",2);
	write_to_buffer(d,"\n\r                      Welcome to the Legends of Krynn!\n\r",0);
	write_to_buffer(d,"\n\r",0);
	ch->next		= char_list;
	char_list		= ch;
	d->connected	= CON_PLAYING;
	reset_char(ch);

      if (ch->recall_point == 0)
	{
        ch->recall_point = hometown_table[ch->hometown].recall;
        sprintf( log_buf, "** Adjusting recall point for %s. It was NULL.", ch->name );
        log_string( log_buf );
	}

	if ( ch->level == 0 )
	{

	    ch->perm_stat[class_table[ch->class].attr_prime] += 3;

	    ch->level	= 1;
	    ch->exp       = exp_at_level(ch, 1);
	    ch->hit		= ch->max_hit;
	    ch->mana	= ch->max_mana;
	    ch->move	= ch->max_move;
	    ch->train	= 5;
	    ch->practice 	= 8;
	    ch->fight_pos = FIGHT_FRONT;
	    ch->steel 	= 20;
          ch->gold	= 200;
	    SET_BIT(ch->act, PLR_NOTITLE);
	    SET_BIT(ch->act, PLR_AUTOEXIT);
	    SET_BIT(ch->act, PLR_AUTOGOLD);
	    SET_BIT(ch->act, PLR_AUTOASSIST);
	    SET_BIT(ch->act, PLR_AUTOTITLE);
          sprintf( buf, "the %s",
          title_table [ch->class] [ch->level] [ch->sex == SEX_FEMALE ? 1 : 0] );
          set_title( ch, buf );

          if ( ch->class == class_lookup("battlemage"))
          {
          ch->pcdata->learned[gsn_sword]= 40;
          }

          if ( ch->class == class_lookup("cleric"))
          {
          ch->pcdata->learned[gsn_mace]= 40;
          }

          if ( ch->class == class_lookup("thief"))
          {
          ch->pcdata->learned[gsn_dagger]= 40;
          }

          if ( ch->class == class_lookup("warrior"))
          {
          ch->pcdata->learned[gsn_sword]= 40;
          }

          if ( ch->class == class_lookup("bard"))
          {
          ch->pcdata->learned[gsn_dagger]= 40;
          }

          if ( ch->class == class_lookup("knight"))
          {
          ch->pcdata->learned[gsn_sword]= 40;
          }

          if ( ch->class == class_lookup("barbarian"))
          {
          ch->pcdata->learned[gsn_whip]= 40;
          }

          if ( ch->class == class_lookup("druid"))
          {
          ch->pcdata->learned[gsn_staff]= 40;
          }

          if ( ch->class == class_lookup("ranger"))
          {
          ch->pcdata->learned[gsn_dagger]= 40;
          }

          if ( ch->class == class_lookup("assassin"))
          {
          ch->pcdata->learned[gsn_dagger]= 40;
          }

          if ( ch->class == class_lookup("spellfilcher"))
          {
          ch->pcdata->learned[gsn_dagger]= 40;
          }

          if ( ch->class == class_lookup("necromancer"))
          {
          ch->pcdata->learned[gsn_dagger]= 40;
          }

          if ( ch->class == class_lookup("enchanter"))
          {
          ch->pcdata->learned[gsn_dagger]= 40;
          }

          if ( ch->class == class_lookup("illusionist"))
          {
          ch->pcdata->learned[gsn_dagger]= 40;
          }

          if ( ch->class == class_lookup("conjuror"))
          {
          ch->pcdata->learned[gsn_dagger]= 40;
          }

          ch->pcdata->learned[gsn_recall] = 50;
	    char_to_room( ch, get_room_index( hometown_table[ch->hometown].school ) );
	    send_to_char("\n\r",ch);

        }
        //C053
	else if ( ch->in_room != NULL )
	{
	    char_to_room( ch, ch->in_room );
	}
	else 
	{
	    char_to_room( ch, get_home(ch) );
	}

        /* auto-level scale adjustments here */
        if(( ch->level != 1 )
	  &&( ch->level < 100))
        {
            /* if level scale was different last time they played */
            if( ch->level_scale != LEVEL_SCALE ) {
                /* adjust so they have same xp needed to level */
                sprintf( log_buf,
                 "Adjusting xp for %s: xp=%d level %d scale=%d xp_to_lev=%d",
                    ch->name,
                    ch->exp, ch->level, ch->level_scale, ch->xp_to_lev );
                log_string( log_buf );
                /* if they've never saved with this new format, give 'em 1/2 */
                if( ch->xp_to_lev == 0 )
                {
                    ch->xp_to_lev = exp_gain_level( ch, ch->level) / 2;
                }
                ch->exp = exp_at_level( ch, ch->level + 1) - ch->xp_to_lev;
                sprintf( log_buf,
                 "New xp=%d", ch->exp );
                log_string( log_buf );
            }
        }

        /* make sure they're in sync with latest skill/spell rules */
        for(i = 1; i < MAX_SKILL; i++ )
        {
            if( skill_table[i].name == NULL ) break;
            if( legal_skill( ch, i ) )
            { 
                if( ch->pcdata->learned[i] == 0 )
                {
                    ch->pcdata->learned[i] = 1;
                    if( ch->level != 1)
                    {
                        sprintf( log_buf, "%s gets skill %s",
                                    ch->name, skill_table[i].name );
                        log_string( log_buf );
			ch->practice++;
                    }
                }
            }
            else
            {
                if( ch->pcdata->learned[i] != 0 )
                {
                    ch->pcdata->learned[i] = 0;
                    sprintf( log_buf, "%s loses skill %s",
                            ch->name, skill_table[i].name );
                    log_string( log_buf );
                }
            }
        }

      ch->pcdata->learned[ gsn_instruct ] = 100;

	/* If we are in a clan, make sure we are on the Roster */
	if(ch->org_id != ORG_NONE)
	{
		/* add_name_clan will automatically check to see if they are already on the roster. */
		add_name_clan(ch->name, ch->org_id, ch->level);
	}

	act( "$n has entered the realms of Krynn.", ch, NULL, NULL, TO_ROOM );
	do_function(ch, &do_look, "auto" );

	wiznet("Look out, $N has left real life behind.",ch,NULL,
		WIZ_LOGINS,WIZ_SITES,get_trust(ch));

      if (ch->steel > 100000 && !IS_IMMORTAL(ch))
      {
       sprintf(buf,
       "You are taxed %ld steel to pay for town improvements.\n\r",(ch->steel - 100000) / 2);
       send_to_char(buf,ch);
       ch->steel -= (ch->steel - 100000) / 2;
      }

      if (IS_IMMORTAL(ch)
	&& IS_SET(ch->comm, COMM_NOIDLE) )
      {
	  REMOVE_BIT(ch->comm, COMM_NOIDLE);
      }

      if (ch->level == 1)
	{
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

	}

	if (ch->pet != NULL)
	{
	    char_to_room(ch->pet,ch->in_room);
	    act("$n has entered the realms of Krynn.",ch->pet,NULL,NULL,TO_ROOM);
	}
	do_function(ch, &do_unread, "");
	break;
    }

    return;
}



/*
 * Parse a name for acceptability.
 */
bool check_parse_name( char *name )
{

    /*
     * Reserved words.
     */
    if (is_exact_name(name,
	"all auto immortal self someone something the you loner none legend lord "
" death raistlin raist rastlin tanis tanthalas kit tika verminaard huma king dragon "
" dark night slayer kitiara sturm brightblade tas tasselhoff taselhoff tasselhof "
" taselhof burfoot burrfoot caramon flint goldmoon riverwind lauralanthalasa "
" gilthanus laurana illegal sweetie realm mater unlinked theros toede fewmaster "
" dracon magi shit fuck damn dam shitlicker cock cunt pussy dick bitch fucker "
" fuckstick highbulp bupu kithkanan kith gilthaus lady porthios skull god "
" fistandantalis fistandantalus fist fizban poontang money green blue red black "
" necro necromancer moon child sun guest newbie new luck leader killer kill hack "
" lover mortal raiden rayden poonanny amethyst gold steel silver argargarg "
" create test tester testie dalamar quality equality hellbender hell abyss "
" link lostsoul lost soul ogopogo predetor predator predeter quick rabbit "
" horse donkey ass queen king silence trip uncle father mother aunt sister "
" brother raistlyn raistlun rastlyn start slugman trapspringer jugwine "
" sanjaysanjay chewy scale log logspur logspar beauty serpant serpent phantom "
" shithole fucko sheep sheepfucker"))
    {
	return FALSE;
    }
	
    if (str_cmp(capitalize(name),"Alander") && (!str_prefix("Alan",name)
    || !str_suffix("Alander",name)))
	return FALSE;

    if (is_exact_name(name,"kiri-jolith"))
    {
	return TRUE;
    }

    if (check_illegal_name( name ))
	return FALSE;

    /*
     * Length restrictions.
     */
     
    if ( strlen(name) <  2 )
	return FALSE;

#if defined(MSDOS)
    if ( strlen(name) >  8 )
	return FALSE;
#endif

#if defined(macintosh) || defined(unix)
    if ( strlen(name) > 12 )
	return FALSE;
#endif

    /*
     * Alphanumerics only.
     * Lock out IllIll twits.
     */
    {
	char *pc;
	bool fIll,adjcaps = FALSE,cleancaps = FALSE;
 	int total_caps = 0;

	fIll = TRUE;
	for ( pc = name; *pc != '\0'; pc++ )
	{
	    if ( !isalpha(*pc) )
		return FALSE;

	    if ( isupper(*pc)) /* ugly anti-caps hack */
	    {
		if (adjcaps)
		    cleancaps = TRUE;
		total_caps++;
		adjcaps = TRUE;
	    }
	    else
		adjcaps = FALSE;

	    if ( LOWER(*pc) != 'i' && LOWER(*pc) != 'l' )
		fIll = FALSE;
	}

	if ( fIll )
	    return FALSE;

	if (cleancaps || (total_caps > (strlen(name)) / 2 && strlen(name) < 3))
	    return FALSE;
    }

    /*
     * Prevent players from naming themselves after mobs.
     */
    {
	extern MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];
	MOB_INDEX_DATA *pMobIndex;
	int iHash;

	for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
	{
	    for ( pMobIndex  = mob_index_hash[iHash];
		  pMobIndex != NULL;
		  pMobIndex  = pMobIndex->next )
	    {
		if ( is_name( name, pMobIndex->player_name ) )
		    return FALSE;
	    }
	}
    }

    return TRUE;
}

/*
 * Adapted from Parse name.
 */
bool check_parse_surname( char *name )
{

    /*
     * Reserved words.
     */
    if (is_exact_name(name,
	"all auto immortal self someone something the you loner none legend lord "
" death raistlin raist rastlin tanis tanthalas kit tika verminaard huma king dragon "
" dark night slayer kitiara sturm brightblade tas tasselhoff taselhoff tasselhof "
" taselhof burfoot burrfoot caramon flint goldmoon riverwind lauralanthalasa "
" gilthanus laurana illegal sweetie realm mater unlinked theros toede fewmaster "
" dracon magi shit fuck damn dam shitlicker cock cunt pussy dick bitch fucker "
" fuckstick highbulp bupu kithkanan kith gilthaus lady porthios skull god "
" fistandantalis fistandantalus fist fizban poontang money green blue red black "
" necro necromancer moon child sun guest newbie new luck leader killer kill hack "
" lover mortal raiden rayden poonanny amethyst gold steel silver argargarg "
" create test tester testie dalamar quality equality hellbender hell abyss "
" link lostsoul lost soul ogopogo predetor predator predeter quick rabbit "
" horse donkey ass queen king silence trip uncle father mother aunt sister "
" brother raistlyn raistlun rastlyn start slugman trapspringer jugwine "
" sanjaysanjay chewy scale log logspur logspar beauty serpant serpent phantom "
" shithole fucko sheep sheepfucker"))
    {
	return FALSE;
    }
	
    if (str_cmp(capitalize(name),"Alander") && (!str_prefix("Alan",name)
    || !str_suffix("Alander",name)))
	return FALSE;

    /*
     * Length restrictions.
     */
     
    if ( strlen(name) <  2 )
	return FALSE;

#if defined(MSDOS)
    if ( strlen(name) >  8 )
	return FALSE;
#endif

#if defined(macintosh) || defined(unix)
    if ( strlen(name) > 12 )
	return FALSE;
#endif

    /*
     * Alphanumerics only.
     * Lock out IllIll twits.
     */
    {
	char *pc;
	bool fIll,adjcaps = FALSE,cleancaps = FALSE;
 	int total_caps = 0;

	fIll = TRUE;
	for ( pc = name; *pc != '\0'; pc++ )
	{
	    if ( !isalpha(*pc) )
		return FALSE;

	    if ( isupper(*pc)) /* ugly anti-caps hack */
	    {
		if (adjcaps)
		    cleancaps = TRUE;
		total_caps++;
		adjcaps = TRUE;
	    }
	    else
		adjcaps = FALSE;

	    if ( LOWER(*pc) != 'i' && LOWER(*pc) != 'l' )
		fIll = FALSE;
	}

	if ( fIll )
	    return FALSE;

	if (cleancaps || (total_caps > (strlen(name)) / 2 && strlen(name) < 3))
	    return FALSE;
    }

    /*
     * Prevent players from naming themselves after mobs.
     */
    {
	extern MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];
	MOB_INDEX_DATA *pMobIndex;
	int iHash;

	for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
	{
	    for ( pMobIndex  = mob_index_hash[iHash];
		  pMobIndex != NULL;
		  pMobIndex  = pMobIndex->next )
	    {
		if ( is_name( name, pMobIndex->player_name ) )
		    return FALSE;
	    }
	}
    }

    return TRUE;
}


/*
 * Look for link-dead player to reconnect.
 */
bool check_reconnect( DESCRIPTOR_DATA *d, char *name, bool fConn )
{
    CHAR_DATA *ch;

    for ( ch = char_list; ch != NULL; ch = ch->next )
    {
	if ( !IS_NPC(ch)
	&&   (!fConn || ch->desc == NULL)
	&&   !str_cmp( d->character->name, ch->name ) )
	{
	    if ( fConn == FALSE )
	    {
		free_string( d->character->pcdata->pwd );
		d->character->pcdata->pwd = str_dup( ch->pcdata->pwd );
	    }
	    else
	    {
		nuke_pets( d->character );
		free_char( d->character );
		d->character = ch;
		ch->desc	 = d;
		ch->timer	 = 0;
		send_to_char(
		    "Reconnecting. Type replay to see missed tells.\n\r", ch );
		act( "$n has reconnected.", ch, NULL, NULL, TO_ROOM );

		sprintf( log_buf, "%s@%s reconnected.", ch->name, d->host );
		log_string( log_buf );
		wiznet("$N groks the fullness of $S link.",
		    ch,NULL,WIZ_LINKS,0,0);
		d->connected = CON_PLAYING;
	    }
	    return TRUE;
	}
    }

    return FALSE;
}



/*
 * Check if already playing.
 */
bool check_playing( DESCRIPTOR_DATA *d, char *name )
{
    DESCRIPTOR_DATA *dold;

    for ( dold = descriptor_list; dold; dold = dold->next )
    {
	if ( dold != d
	&&   dold->character != NULL
	&&   dold->connected != CON_GET_NAME
	&&   dold->connected != CON_GET_OLD_PASSWORD
	&&   !str_cmp( name, dold->original
	         ? dold->original->name : dold->character->name ) )
	{
	    write_to_buffer( d, "That character is already playing.\n\r",0);
	    write_to_buffer( d, "Do you wish to connect anyway (Y/N)?",0);
	    d->connected = CON_BREAK_CONNECT;
	    return TRUE;
	}
    }

    return FALSE;
}



void stop_idling( CHAR_DATA *ch )
{
    if ( ch == NULL
    ||   ch->desc == NULL
    ||   ch->desc->connected != CON_PLAYING
    ||   ch->was_in_room == NULL 
    ||   ch->in_room != get_room_index(ROOM_VNUM_LIMBO))
	return;

    ch->timer = 0;
    char_from_room( ch );
    char_to_room( ch, ch->was_in_room );
    ch->was_in_room	= NULL;
    act( "$n has returned from the void.", ch, NULL, NULL, TO_ROOM );
    return;
}



/*
 * Write to one char.
 */
void send_to_char_bw( const char *txt, CHAR_DATA *ch )
{
    if ( txt != NULL && ch->desc != NULL )
        write_to_buffer( ch->desc, txt, strlen(txt) );
    return;
}

/*
 * Write to one char, new colour version, by Lope.
 */
void send_to_char( const char *txt, CHAR_DATA *ch )
{
    const	char 	*point;
    		char 	*point2;
    		char 	buf[ MAX_STRING_LENGTH*4 ];
		int	skip = 0;

    buf[0] = '\0';
    point2 = buf;
    if( txt && ch->desc )
	{
	    if( IS_SET( ch->act, PLR_COLOUR ) )
	    {
		for( point = txt ; *point ; point++ )
	        {
		    if( *point == '{' )
		    {
			point++;
			skip = colour( *point, ch, point2 );
			while( skip-- > 0 )
			    ++point2;
			continue;
		    }
		    *point2 = *point;
		    *++point2 = '\0';
		}			
		*point2 = '\0';
        	write_to_buffer( ch->desc, buf, point2 - buf );
	    }
	    else
	    {
		for( point = txt ; *point ; point++ )
	        {
		    if( *point == '{' )
		    {
			point++;
			continue;
		    }
		    *point2 = *point;
		    *++point2 = '\0';
		}
		*point2 = '\0';
        	write_to_buffer( ch->desc, buf, point2 - buf );
	    }
	}
    return;
}

/*
 * Write to one desc, new colour version by Lope.
 */
void send_to_desc(const char *txt, DESCRIPTOR_DATA *d )
{
    const   char   *point;
            char   *point2;
            char   buf[ MAX_STRING_LENGTH*4 ];
            int    skip = 0;

    buf[0] = '\0';
    point2 = buf;
    if( txt && d )
      {
         if( d->ansi == TRUE )
         {
           for( point = txt ; *point ; point++ )
             {
               if( *point == '{' )
               {
                 point++;
                 skip = colour( *point, NULL, point2 );
                 while( skip-- > 0 )
                    ++point2;
                 continue;
               }
               *point2 = *point;
               *++point2 = '\0';
             }
             *point2 = '\0';
             write_to_buffer( d, buf, point2 - buf );
         }
         else
         {
           for( point = txt; *point ; point++ )
             {
               if( *point == '{' )
               {
                 point++;
                 continue;
               }
               *point2 = *point;
               *++point2 = '\0';
            }
            *point2 = '\0';
            write_to_buffer( d, buf, point2 - buf );
         }
      }
    return;
}

/*
 * Send a page to one char.
 */
void page_to_char_bw( const char *txt, CHAR_DATA *ch )
{
    if ( txt == NULL || ch->desc == NULL)
	return;

    if (ch->lines == 0 )
    {
	send_to_char_bw( txt, ch );
	return;
    }
	
#if defined(macintosh)
	send_to_char_bw(txt,ch);
#else
    ch->desc->showstr_head = alloc_mem(strlen(txt) + 1);
    strcpy(ch->desc->showstr_head,txt);
    ch->desc->showstr_point = ch->desc->showstr_head;
    show_string(ch->desc,"");
#endif
}

/*
 * Page to one char, new colour version, by Lope.
 */
void page_to_char( const char *txt, CHAR_DATA *ch )
{
    const	char	*point;
    		char	*point2;
    		char	buf[ MAX_STRING_LENGTH * 4 ];
		int	skip = 0;

#if defined(macintosh)
    send_to_char( txt, ch );
#else
    buf[0] = '\0';
    point2 = buf;
    if( txt && ch->desc )
	{
	    if( IS_SET( ch->act, PLR_COLOUR ) )
	    {
		for( point = txt ; *point ; point++ )
	        {
		    if( *point == '{' )
		    {
			point++;
			skip = colour( *point, ch, point2 );
			while( skip-- > 0 )
			    ++point2;
			continue;
		    }
		    *point2 = *point;
		    *++point2 = '\0';
		}			
		*point2 = '\0';
		ch->desc->showstr_head  = alloc_mem( strlen( buf ) + 1 );
		strcpy( ch->desc->showstr_head, buf );
		ch->desc->showstr_point = ch->desc->showstr_head;
		show_string( ch->desc, "" );
	    }
	    else
	    {
		for( point = txt ; *point ; point++ )
	        {
		    if( *point == '{' )
		    {
			point++;
			continue;
		    }
		    *point2 = *point;
		    *++point2 = '\0';
		}
		*point2 = '\0';
		ch->desc->showstr_head  = alloc_mem( strlen( buf ) + 1 );
		strcpy( ch->desc->showstr_head, buf );
		ch->desc->showstr_point = ch->desc->showstr_head;
		show_string( ch->desc, "" );
	    }
	}
#endif
    return;
}


/* string pager */
void show_string(struct descriptor_data *d, char *input)
{
    char buffer[4*MAX_STRING_LENGTH];
    char buf[MAX_INPUT_LENGTH];
    register char *scan, *chk;
    int lines = 0, toggle = 1;
    int show_lines;

    one_argument(input,buf);
    if (buf[0] != '\0')
    {
	if (d->showstr_head)
	{
	    free_mem(d->showstr_head,strlen(d->showstr_head));
	    d->showstr_head = 0;
	}
    	d->showstr_point  = 0;
	return;
    }

    if (d->character)
	show_lines = d->character->lines;
    else
	show_lines = 0;

    for (scan = buffer; ; scan++, d->showstr_point++)
    {
	if (((*scan = *d->showstr_point) == '\n' || *scan == '\r')
	    && (toggle = -toggle) < 0)
	    lines++;

	else if (!*scan || (show_lines > 0 && lines >= show_lines))
	{
	    *scan = '\0';
	    write_to_buffer(d,buffer,strlen(buffer));
	    for (chk = d->showstr_point; isspace(*chk); chk++);
	    {
		if (!*chk)
		{
		    if (d->showstr_head)
        	    {
            		free_mem(d->showstr_head,strlen(d->showstr_head));
            		d->showstr_head = 0;
        	    }
        	    d->showstr_point  = 0;
    		}
	    }
	    return;
	}
    }
    return;
}
	

/* quick sex fixer */
void fix_sex(CHAR_DATA *ch)
{
    if (ch->sex < 0 || ch->sex > 2)
    	ch->sex = IS_NPC(ch) ? 0 : ch->pcdata->true_sex;
}

void act_new(const char *format, CHAR_DATA *ch, const void *arg1,
             const void *arg2, int type, int min_pos)
{
  /* to be compatible with older code */
  act_new1(format, ch, arg1, arg2, type, min_pos, FALSE);
}

void act_new1( const char *format, CHAR_DATA *ch, const void *arg1, 
	      const void *arg2, int type, int min_pos, bool fDoppel)
{
    static char * const he_she  [] = { "it",  "he",  "she" };
    static char * const him_her [] = { "it",  "him", "her" };
    static char * const his_her [] = { "its", "his", "her" };
 
    CHAR_DATA 		*to;
    CHAR_DATA 		*vch = ( CHAR_DATA * ) arg2;
    OBJ_DATA 		*obj1 = ( OBJ_DATA  * ) arg1;
    OBJ_DATA 		*obj2 = ( OBJ_DATA  * ) arg2;
    const 	char 	*str;
    char 		*i = NULL;
    char 		*point;
    char 		*pbuff;
    char 		buffer[ MAX_STRING_LENGTH*2 ];
    char 		buf[ MAX_STRING_LENGTH   ];
    char 		fname[ MAX_INPUT_LENGTH  ];
    bool		fColour = FALSE;

 
    /*
     * Discard null and zero-length messages.
     */
    if( !format || !*format )
        return;

    /* discard null rooms and chars */
    if( !ch || !ch->in_room )
	return;

    to = ch->in_room->people;
    if( type == TO_VICT )
    {
        if( !vch )
        {
            bug( "Act: null vch with TO_VICT.", 0 );
            return;
        }

	if( !vch->in_room )
	    return;

        to = vch->in_room->people;
    }
 
    for( ; to ; to = to->next_in_room )
    {
/*
             * MOBProgram fix to allow MOBs to see acts()
             *   -- Thunder (thunder1@fast.net)
             */
        if ( (to->desc == NULL
        /*&&    (!IS_NPC(to) || !HAS_TRIGGER_MOB(to, TRIG_ACT))*/) //C065
        ||   to->position < min_pos )
            continue; 
 
        if( ( type == TO_CHAR ) && to != ch )
            continue;
        if( type == TO_VICT && ( to != vch || to == ch ) )
            continue;
        if( type == TO_ROOM && to == ch )
            continue;
        if( type == TO_NOTVICT && (to == ch || to == vch) )
            continue;
 
        point   = buf;
        str     = format;
        while( *str != '\0' )
        {
            if( *str != '$' )
            {
                *point++ = *str++;
                continue;
            }
            fColour = TRUE;
    	    ++str;
    	    i = " <@@@> ";
            
			if( !arg2 && *str >= 'A' && *str <= 'Z' )
            {
                bug( "Act: missing arg2 for code %d.", *str );
                i = " <@@@> ";
            }
            else
            {
                switch ( *str )
                {
                default:  bug( "Act: bad code %d.", *str );
                          i = " <@@@> ";                                break;
                /* Thx alex for 't' idea */
   		    case 't': if (arg1) i=(char *)arg1;
                          else bug("Act: bad code $t for 'arg1'",0);
                          break;
                case 'T': if (arg2) i=(char *)arg2;
                          else bug("Act: bad code $T for 'arg2'",0);
                          break;
/* Before Doppel *
                case 'n': i = PERS( ch,  to  );                         break;
                case 'N': i = PERS( vch, to  );                         break;
                case 'e': i = he_she  [URANGE(0, ch  ->sex, 2)];        break;
                case 'E': i = he_she  [URANGE(0, vch ->sex, 2)];        break;
                case 'm': i = him_her [URANGE(0, ch  ->sex, 2)];        break;
                case 'M': i = him_her [URANGE(0, vch ->sex, 2)];        break;
                case 's': i = his_her [URANGE(0, ch  ->sex, 2)];        break;
                case 'S': i = his_her [URANGE(0, vch ->sex, 2)];        break;
*/

                case 'n':
			if (is_affected(ch, gsn_doppelganger) &&
                        !IS_SET(to->act,PLR_HOLYLIGHT) && !IS_NPC(ch))
                        {
                        i=      ( can_see( to, ch ) ?
                                ( IS_NPC(ch) ? ch->doppel->short_descr
                                : ch->doppel->name ) : "someone" );
                        }
                        else
				if (ch&&to) i=PERS(ch,to);
             		else bug("Act: bad code $n for 'ch' or 'to'",0);
             		break;
                case 'N': 
                	if (is_affected(vch, gsn_doppelganger) &&
                        !IS_SET(to->act,PLR_HOLYLIGHT) && !IS_NPC(ch))
                        {
                        i=      ( can_see( to, vch ) ?
                                ( IS_NPC(vch) ? vch->short_descr
                                : vch->doppel->name ) : "someone" );
			}
			else
			if (vch&&to) i=PERS(vch,to);
             	else bug("Act: bad code $N for 'ch' or 'to'",0);
             	break;
                case 'e': i = 
                  (is_affected(ch, gsn_doppelganger) &&
                    !IS_SET(to->act,PLR_HOLYLIGHT)) ?
                    he_she [URANGE(0,ch->doppel->sex,2)] :
                    he_she  [URANGE(0, ch  ->sex, 2)];    
                  break;
                case 'E': i = 
                  (is_affected(vch, gsn_doppelganger) &&
                    !IS_SET(to->act,PLR_HOLYLIGHT)) ?
                    he_she  [URANGE(0, vch->doppel->sex, 2)] :
                    he_she  [URANGE(0, vch->sex, 2)];
                  break;
                case 'm': i = 
                  (is_affected(ch, gsn_doppelganger) &&
                    !IS_SET(to->act,PLR_HOLYLIGHT)) ?
                    him_her [URANGE(0,ch->doppel->sex,2)] :
                    him_her [URANGE(0, ch->sex, 2)];    
                  break;
                case 'M': i = 
                  (is_affected(vch, gsn_doppelganger) &&
                    !IS_SET(to->act,PLR_HOLYLIGHT)) ?
                    him_her  [URANGE(0, vch->doppel->sex, 2)] :
                    him_her  [URANGE(0, vch->sex, 2)];
                  break;
                case 's': i = 
                  (is_affected(ch, gsn_doppelganger) &&
                    !IS_SET(to->act,PLR_HOLYLIGHT)) ?
                    his_her [URANGE(0,ch->doppel->sex,2)] :
                    his_her [URANGE(0, ch  ->sex, 2)];    
                  break;
                case 'S': i = 
                  (is_affected(vch, gsn_doppelganger) &&
                    !IS_SET(to->act,PLR_HOLYLIGHT)) ?
                    his_her  [URANGE(0, vch->doppel->sex, 2)] :
                    his_her  [URANGE(0, vch->sex, 2)];
                  break;
 
                case 'p':
                    i = can_see_obj( to, obj1 )
                            ? obj1->short_descr
                            : "something";
                    break;
 
                case 'P':
                    i = can_see_obj( to, obj2 )
                            ? obj2->short_descr
                            : "something";
                    break;
 
                case 'd':
                    if ( arg2 == NULL || ((char *) arg2)[0] == '\0' )
                    {
                        i = "door";
                    }
                    else
                    {
                        one_argument( (char *) arg2, fname );
                        i = fname;
                    }
                    break;
                }
            }
 
            ++str;
            while ( ( *point = *i ) != '\0' )
                ++point, ++i;
        }
 
        *point++ = '\n';
        *point++ = '\r';
	*point   = '\0';
        buf[0]   = UPPER(buf[0]);
	if ( to->desc != NULL )
        {
        pbuff	 = buffer;
	colourconv( pbuff, buf, to );
        write_to_buffer( to->desc, buffer, 0 );
        }
	else
	if ( MOBtrigger )
	    p_act_trigger( buf, to, NULL, NULL, ch, arg1, arg2, TRIG_ACT );
    }
    if ( type == TO_ROOM || type == TO_NOTVICT )
    {
	OBJ_DATA *obj, *obj_next;
	CHAR_DATA *tch, *tch_next;

	 point   = buf;
	 str     = format;
	 while( *str != '\0' )
	 {
	     *point++ = *str++;
	 }
	 *point   = '\0';
	 
	for( obj = ch->in_room->contents; obj; obj = obj_next )
	{
	    obj_next = obj->next_content;
	    if ( HAS_TRIGGER_OBJ( obj, TRIG_ACT ) )
		p_act_trigger( buf, NULL, obj, NULL, ch, NULL, NULL, TRIG_ACT );
	}

	for( tch = ch; tch; tch = tch_next )
	{
	    tch_next = tch->next_in_room;

	    for ( obj = tch->carrying; obj; obj = obj_next )
	    {
		obj_next = obj->next_content;
		if ( HAS_TRIGGER_OBJ( obj, TRIG_ACT ) )
		    p_act_trigger( buf, NULL, obj, NULL, ch, NULL, NULL, TRIG_ACT );
	    }
	}

	 if ( HAS_TRIGGER_ROOM( ch->in_room, TRIG_ACT ) )
	     p_act_trigger( buf, NULL, NULL, ch->in_room, ch, NULL, NULL, TRIG_ACT );
    }
    return;
}


/*
 * Macintosh support functions.
 */
#if defined(macintosh)
int gettimeofday( struct timeval *tp, void *tzp )
{
    tp->tv_sec  = time( NULL );
    tp->tv_usec = 0;
}
#endif

 
 void logpf (char * fmt, ...)
 {
 	char buf [2*MSL];
 	va_list args;
 	va_start (args, fmt);
 	vsprintf (buf, fmt, args);
 	va_end (args);
 	
 	log_string (buf);
 }
 
int colour( char type, CHAR_DATA *ch, char *string )
{
    PC_DATA	*col;
    char	code[ 20 ];
    char	*p = '\0';

    if(ch && IS_NPC( ch ) )
	return( 0 );

    col = ch ? ch->pcdata : NULL;

    switch( type )
    {
	default:
	    strcpy( code, CLEAR );
	    break;
	case 'x':
	    strcpy( code, CLEAR );
	    break;
	case 'p':
    if( col->prompt[2] )
		sprintf( code, "\e[%d;3%dm%c", col->prompt[0], col->prompt[1], '\a' );
	    else
		sprintf( code, "\e[%d;3%dm", col->prompt[0], col->prompt[1] );
	    break;
	case 's':
	    if( col->room_title[2] )
		sprintf( code, "\e[%d;3%dm%c",
		    col->room_title[0], col->room_title[1], '\a' );
	    else
		sprintf( code, "\e[%d;3%dm", col->room_title[0], col->room_title[1] );
	    break;
	case 'S':
	    if( col->room_text[2] )
		sprintf( code, "\e[%d;3%dm%c",
		    col->room_text[0], col->room_text[1], '\a' );
	    else
		sprintf( code, "\e[%d;3%dm", col->room_text[0], col->room_text[1] );
	    break;
	case 'd':
	    if( col->gossip[2] )
		sprintf( code, "\e[%d;3%dm%c",
		    col->gossip[0], col->gossip[1], '\a' );
	    else
		sprintf( code, "\e[%d;3%dm", col->gossip[0], col->gossip[1] );
	    break;
	case '9':
	    if( col->gossip_text[2] )
		sprintf( code, "\e[%d;3%dm%c",
		    col->gossip_text[0], col->gossip_text[1], '\a' );
	    else
		sprintf( code, "\e[%d;3%dm", col->gossip_text[0], col->gossip_text[1] );
	    break;
	case 'Z':
	    if( col->wiznet[2] )
		sprintf( code, "\e[%d;3%dm%c",
		    col->wiznet[0], col->wiznet[1], '\a' );
	    else
		sprintf( code, "\e[%d;3%dm", col->wiznet[0], col->wiznet[1] );
	    break;
	case 'o':
	    if( col->room_exits[2] )
		sprintf( code, "\e[%d;3%dm%c",
		    col->room_exits[0], col->room_exits[1], '\a' );
	    else
		sprintf( code, "\e[%d;3%dm", col->room_exits[0], col->room_exits[1] );
	    break;
	case 'O':
	    if( col->room_things[2] )
		sprintf( code, "\e[%d;3%dm%c",
		    col->room_things[0], col->room_things[1], '\a' );
	    else
		sprintf( code, "\e[%d;3%dm", col->room_things[0], col->room_things[1] );
	    break;
	case 'i':
	    if( col->immtalk_text[2] )
		sprintf( code, "\e[%d;3%dm%c",
		    col->immtalk_text[0], col->immtalk_text[1], '\a' );
	    else
		sprintf( code, "\e[%d;3%dm",
		    col->immtalk_text[0], col->immtalk_text[1] );
	    break;
	case 'I':
	    if( col->immtalk_type[2] )
		sprintf( code, "\e[%d;3%dm%c",
		    col->immtalk_type[0], col->immtalk_type[1], '\a' );
	    else
		sprintf( code, "\e[%d;3%dm",
		    col->immtalk_type[0], col->immtalk_type[1] );
	    break;
	case '2':
	    if( col->fight_yhit[2] )
		sprintf( code, "\e[%d;3%dm%c",
		    col->fight_yhit[0], col->fight_yhit[1], '\a' );
	    else
		sprintf( code, "\e[%d;3%dm", col->fight_yhit[0], col->fight_yhit[1] );
	    break;
	case '3':
	    if( col->fight_ohit[2] )
		sprintf( code, "\e[%d;3%dm%c",
		    col->fight_ohit[0], col->fight_ohit[1], '\a' );
	    else
		sprintf( code, "\e[%d;3%dm", col->fight_ohit[0], col->fight_ohit[1] );
	    break;
	case '4':
	    if( col->fight_thit[2] )
		sprintf( code, "\e[%d;3%dm%c",
		    col->fight_thit[0], col->fight_thit[1], '\a' );
	    else
		sprintf( code, "\e[%d;3%dm", col->fight_thit[0], col->fight_thit[1] );
	    break;
	case '5':
	    if( col->fight_skill[2] )
		sprintf( code, "\e[%d;3%dm%c",
		    col->fight_skill[0], col->fight_skill[1], '\a' );
	    else
		sprintf( code, "\e[%d;3%dm", col->fight_skill[0], col->fight_skill[1] );
	    break;
	case '1':
	    if( col->fight_death[2] )
		sprintf( code, "\e[%d;3%dm%c",
		    col->fight_death[0], col->fight_death[1], '\a' );
	    else
		sprintf( code, "\e[%d;3%dm", col->fight_death[0], col->fight_death[1] );
	    break;
	case '6':
	    if( col->say[2] )
		sprintf( code, "\e[%d;3%dm%c", col->say[0], col->say[1], '\a' );
	    else
		sprintf( code, "\e[%d;3%dm", col->say[0], col->say[1] );
	    break;
	case '7':
	    if( col->say_text[2] )
		sprintf( code, "\e[%d;3%dm%c",
		    col->say_text[0], col->say_text[1], '\a' );
	    else
		sprintf( code, "\e[%d;3%dm", col->say_text[0], col->say_text[1] );
	    break;
	case 'k':
	    if( col->tell[2] )
		sprintf( code, "\e[%d;3%dm%c", col->tell[0], col->tell[1], '\a' );
	    else
		sprintf( code, "\e[%d;3%dm", col->tell[0], col->tell[1] );
	    break;
	case 'K':
	    if( col->tell_text[2] )
		sprintf( code, "\e[%d;3%dm%c",
		    col->tell_text[0], col->tell_text[1], '\a' );
	    else
		sprintf( code, "\e[%d;3%dm", col->tell_text[0], col->tell_text[1] );
	    break;
	case 'l':
	    if( col->reply[2] )
		sprintf( code, "\e[%d;3%dm%c", col->reply[0], col->reply[1], '\a' );
	    else
		sprintf( code, "\e[%d;3%dm", col->reply[0], col->reply[1] );
	    break;
	case 'L':
	    if( col->reply_text[2] )
		sprintf( code, "\e[%d;3%dm%c",
		    col->reply_text[0], col->reply_text[1], '\a' );
	    else
		sprintf( code, "\e[%d;3%dm", col->reply_text[0], col->reply_text[1] );
	    break;
	case 'n':
	    if( col->gtell_text[2] )
		sprintf( code, "\e[%d;3%dm%c",
		    col->gtell_text[0], col->gtell_text[1], '\a' );
	    else
		sprintf( code, "\e[%d;3%dm", col->gtell_text[0], col->gtell_text[1] );
	    break;
	case 'N':
	    if( col->gtell_type[2] )
		sprintf( code, "\e[%d;3%dm%c",
		    col->gtell_type[0], col->gtell_type[1], '\a' );
	    else
		sprintf( code, "\e[%d;3%dm", col->gtell_type[0], col->gtell_type[1] );
	    break;
	case 'a':
	    if( col->auction[2] )
		sprintf( code, "\e[%d;3%dm%c",
		    col->auction[0], col->auction[1], '\a' );
	    else
		sprintf( code, "\e[%d;3%dm", col->auction[0], col->auction[1] );
	    break;
      case 'A':
	    if( col->auction_text[2] )
		sprintf( code, "\e[%d;3%dm%c",
		    col->auction_text[0], col->auction_text[1], '\a' );
	    else
		sprintf( code, "\e[%d;3%dm", col->auction_text[0], col->auction_text[1] );
	    break;
      case 'q':
          if( col->ask[2] )
		sprintf( code, "\e[%d;3%dm%c",
		    col->ask[0], col->ask[1], '\a' );
	    else
		sprintf( code, "\e[%d;3%dm", col->ask[0], col->ask[1] );
	    break;
      case 'Q':
	    if( col->ask_text[2] )
		sprintf( code, "\e[%d;3%dm%c",
		    col->ask_text[0], col->ask_text[1], '\a' );
	    else
		sprintf( code, "\e[%d;3%dm",
		    col->ask_text[0], col->ask_text[1] );
	    break;
	case 'f':
	    if( col->answer[2] )
		sprintf( code, "\e[%d;3%dm%c",
		    col->answer[0], col->answer[1], '\a' );
	    else
		sprintf( code, "\e[%d;3%dm", col->answer[0], col->answer[1] );
	    break;
	case 'F':
	    if( col->answer_text[2] )
		sprintf( code, "\e[%d;3%dm%c",
		    col->answer_text[0], col->answer_text[1], '\a' );
	    else
		sprintf( code, "\e[%d;3%dm", col->answer_text[0], col->answer_text[1] );
	    break;
	case 'e':
	    if( col->music[2] )
 	sprintf( code, "\e[%d;3%dm%c",
		    col->music[0], col->music[1], '\a' );
	    else
		sprintf( code, "\e[%d;3%dm", col->music[0], col->music[1] );
	    break;
	case 'E':
	    if( col->music_text[2] )
		sprintf( code, "\e[%d;3%dm%c",
		    col->music_text[0], col->music_text[1], '\a' );
	    else
		sprintf( code, "\e[%d;3%dm", col->music_text[0], col->music_text[1] );
	    break;
	case 'h':
	    if( col->quote[2] )
		sprintf( code, "\e[%d;3%dm%c",
		    col->quote[0], col->quote[1], '\a' );
	    else
		sprintf( code, "\e[%d;3%dm", col->quote[0], col->quote[1] );
	    break;
	case 'H':
	    if( col->quote_text[2] )
		sprintf( code, "\e[%d;3%dm%c",
		    col->quote_text[0], col->quote_text[1], '\a' );
	    else
		sprintf( code, "\e[%d;3%dm", col->quote_text[0], col->quote_text[1] );
	    break;
	case 'j':
	    if( col->info[2] )
		sprintf( code, "\e[%d;3%dm%c", col->info[0], col->info[1], '\a' );
	    else
		sprintf( code, "\e[%d;3%dm", col->info[0], col->info[1] );
	    break;
	case 'b':
	    strcpy( code, C_BLUE );
	    break;
	case 'c':
	    strcpy( code, C_CYAN );
	    break;
	case 'g':
	    strcpy( code, C_GREEN );
	    break;
	case 'm':
	    strcpy( code, C_MAGENTA );
	    break;
	case 'r':
	    strcpy( code, C_RED );
	    break;
	case 'w':
	    strcpy( code, C_WHITE );
	    break;
	case 'y':
	    strcpy( code, C_YELLOW );
	    break;
	case 'B':
	    strcpy( code, C_B_BLUE );
	    break;
	case 'C':
	    strcpy( code, C_B_CYAN );
	    break;
	case 'G':
	    strcpy( code, C_B_GREEN );
	    break;
	case 'M':
	    strcpy( code, C_B_MAGENTA );
	    break;
	case 'R':
	    strcpy( code, C_B_RED );
	    break;
	case 'W':
	    strcpy( code, C_B_WHITE );
	    break;
	case 'Y':
	    strcpy( code, C_B_YELLOW );
	    break;
	case 'D':
	    strcpy( code, C_D_GREY );
	    break;
	case '*':
	    sprintf( code, "%c", '\a' );
	    break;
	case '/':
	    strcpy( code, "\n\r" );
	    break;
	case '-':
	    sprintf( code, "%c", '~' );
	    break;
	case '{':
	    sprintf( code, "%c", '{' );
	    break;
    }

    p = code;
    while( *p != '\0' )
    {
	*string = *p++;
	*++string = '\0';
    }

    return( strlen( code ) );
}

void colourconv( char *buffer, const char *txt, CHAR_DATA *ch )
{
    const	char	*point;
		int	skip = 0;

    if( ch->desc && txt )
    {
	if( IS_SET( ch->act, PLR_COLOUR ) )
	{
	    for( point = txt ; *point ; point++ )
	    {
		if( *point == '{' )
		{
		    point++;
		    skip = colour( *point, ch, buffer );
		    while( skip-- > 0 )
			++buffer;
		    continue;
		}
		*buffer = *point;
		*++buffer = '\0';
	    }			
	    *buffer = '\0';
	}
	else
	{
	    for( point = txt ; *point ; point++ )
	    {
		if( *point == '{' )
		{
		    point++;
		    continue;
		}
		*buffer = *point;
		*++buffer = '\0';
	    }
	    *buffer = '\0';
	}
    }
    return;
}



/* source: EOD, by John Booth <???> */

void printf_to_char (CHAR_DATA *ch, char *fmt, ...)
{
	char buf [MAX_STRING_LENGTH];
	va_list args;
	va_start (args, fmt);
	vsprintf (buf, fmt, args);
	va_end (args);
	
	send_to_char (buf, ch);
}

void bugf (char * fmt, ...)
{
	char buf [2*MSL];
	va_list args;
	va_start (args, fmt);
	vsprintf (buf, fmt, args);
	va_end (args);

	bug (buf, 0);
}

void sig_handler(int sig)
{
  switch(sig)
  {
    case SIGBUS:
      bug("Sig handler SIGBUS.",0);
      do_auto_shutdown();
      break;
    case SIGTERM:
      bug("Sig handler SIGTERM.",0);
      do_auto_shutdown();
      break;
    case SIGABRT:
      bug("Sig handler SIGABRT",0);
      do_auto_shutdown();             
     case SIGSEGV:
      bug("Sig handler SIGSEGV",0);
      do_auto_shutdown();
      break;
  }
}

void init_signals()
{
  signal(SIGBUS,sig_handler);
  signal(SIGTERM,sig_handler);
  signal(SIGABRT,sig_handler);
  signal(SIGSEGV,sig_handler);
}

bool check_illegal_name( char *name )
{
  char strsave[MIL],
  nameread[MSL];
  FILE *fp;
  fclose( fpReserve );

  sprintf( strsave, "%s", ILLEGAL_NAME_FILE );
  if ( (fp = fopen( strsave, "r" ) ) != NULL )
  {
    for ( ; ; )
    {
      fscanf (fp, " %s", nameread);
      if ( !str_cmp( nameread, "END" ) )
        break;  
      else if (is_name(name,nameread))
	  return TRUE;
    }
  }
  else
    fp = fopen( NULL_FILE, "r" );
    fclose( fp );
    fpReserve = fopen( NULL_FILE, "r" );  
    return FALSE;  
}

/*
 * This is send_to_desc but with write_to_descriptor
 * instead of write_to_buffer.
 */
void send_to_desc2(const char *txt, DESCRIPTOR_DATA *d )
{
    const   char   *point;
            char   *point2;
            char   buf[ MAX_STRING_LENGTH*4 ];
            int    skip = 0;

    buf[0] = '\0';
    point2 = buf;

	if(d == NULL) return;

    if( txt && d )
      {
         if( d->ansi == TRUE )
         {
           for( point = txt ; *point ; point++ )
             {
               if( *point == '{' )
               {
                 point++;
                 skip = colour( *point, NULL, point2 );
                 while( skip-- > 0 )
                    ++point2;
                 continue;
               }
               *point2 = *point;
               *++point2 = '\0';
             }
             *point2 = '\0';
             write_to_descriptor( d->descriptor, buf, point2 - buf );
         }
         else
         {
           for( point = txt; *point ; point++ )
             {
               if( *point == '{' )
               {
                 point++;
                 continue;
               }
               *point2 = *point;
               *++point2 = '\0';
            }
            *point2 = '\0';
            write_to_descriptor( d->descriptor, buf, point2 - buf );
         }
      }
    return;
}

