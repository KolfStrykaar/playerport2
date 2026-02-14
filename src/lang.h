/*
 * Language declarations
 */

#define MAX_LANG                     15  /*races languages + common tongue */

#define LANG_COMMON			0
#define LANG_ELVEN			1
#define LANG_DROW				2
#define LANG_DWARVEN			3
#define LANG_OGRE				4
#define LANG_KENDER			5
#define LANG_MINOTAUR			6
#define LANG_SILVANESTI			7
#define LANG_QUALINESTI			8
#define LANG_KAGONESTI			9
#define LANG_DARGONESTI			10
#define LANG_DIMERNESTI			11
#define LANG_DRACONIAN			12
#define LANG_SOLAMNIC			13
#define LANG_THIEVES_CANT		14



int     get_lang_skill	args( ( int lang ) );
int     lang_skill	args( ( CHAR_DATA *ch, CHAR_DATA *vict ) );
char  * language_check  args( ( CHAR_DATA *ch, char *mesg, CHAR_DATA *vict ) );
char  *	lang_convert	args( ( char *mesg, int lang, int cskill, int skill, bool Ischar ) );
char  * lang_lookup	args( ( int lang ) );


struct lang_type
{
    char * lang;
};

struct lang_chars_type
{
    char * old;
    char * new;
};

extern  const   struct  lang_chars_type lang_chars[MAX_LANG][350];
extern  const   struct  lang_type       lang_table      [MAX_LANG];

