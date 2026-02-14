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
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h> /* unlink() */
#include "merc.h"
#include "interp.h"
#include "recycle.h"

bool	check_social	args( ( CHAR_DATA *ch, char *command, char *argument ) );
bool 	check_disabled (const struct cmd_type *command);
void set_fighting	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );

char    last_command[MAX_STRING_LENGTH];

DISABLED_DATA *disabled_first;

#define END_MARKER	"END" /* for load_disabled() and save_disabled() */

/*
 * Command logging types.
 */
#define LOG_NORMAL	0
#define LOG_ALWAYS	1
#define LOG_NEVER	2


/*
 * Log-all switch.
 */
bool				fLogAll		= FALSE;


int			nAllocString;
int			nAllocPerm;


/*	0) Immortal commands
 *	1) Configuration commands
 *	2) Common Commands (read: no better place to put these ones)
 *	3) Communication commands
 *	4) Informational commands
 *	5) Object Manipulation commands
 *	6) Movement commands
 *	7) Combat commands
 */

/*
 * Command table.
 */
const	struct	cmd_type	cmd_table	[] =
{
    /*
     * Common movement commands.
     */
    { "north",			do_north,	POS_STANDING,    	0,  LOG_NEVER, 1, 6 },
    { "east",			do_east,	POS_STANDING,	0,  LOG_NEVER, 1, 6 },
    { "south",			do_south,	POS_STANDING,	0,  LOG_NEVER, 1, 6 },
    { "west",			do_west,	POS_STANDING,	0,  LOG_NEVER, 1, 6 },
    { "up",				do_up,	POS_STANDING,	0,  LOG_NEVER, 1, 6 },
    { "down",			do_down,	POS_STANDING,	0,  LOG_NEVER, 1, 6 },

    /*
     * Common other commands.
     * Placed here so one and two letter abbreviations work.
     */
    { "cast",			do_cast,		POS_FIGHTING,	0,  LOG_NORMAL, 1, 2 },
    { "beep",			do_beep,		POS_RESTING,	0,  LOG_NORMAL, 1, 2 },
    { "buy",			do_buy,		POS_RESTING,	0,  LOG_NORMAL, 1, 2 },
    { "channels",       	do_channels,    	POS_DEAD,        	0,  LOG_NORMAL, 1, 2 },
    { "disguise",			do_disguise,	POS_STANDING,	0,  LOG_NORMAL, 1, 2 },
    { "drag",           	do_drag,        	POS_STANDING,    	0,  LOG_NORMAL, 1, 2 },
    { "exits",			do_exits,		POS_RESTING,	0,  LOG_NORMAL, 1, 2 },
    { "get",			do_get,		POS_RESTING,	0,  LOG_NORMAL, 1, 2 },
    { "group",          	do_group,       	POS_DEAD,    	0,  LOG_NORMAL, 1, 2 },
    { "herotalk",			do_hero,		POS_SLEEPING,   101,  LOG_NORMAL, 1, 2 },
    { "hit",			do_kill,		POS_FIGHTING,	0,  LOG_NORMAL, 0, 2 },
    { "inventory",		do_inventory,	POS_DEAD,	 	0,  LOG_NORMAL, 1, 2 },
    { "kill",			do_kill,		POS_FIGHTING,	0,  LOG_NORMAL, 1, 2 },
    { "look",			do_look,		POS_RESTING,	0,  LOG_NORMAL, 1, 2 },
    { "clan",			do_message,		POS_SLEEPING,	0,  LOG_NORMAL, 1, 2 },
    { "message",			do_message,		POS_RESTING,	0,  LOG_NORMAL, 1, 2 },
    { "order",			do_order,		POS_RESTING,	0,  LOG_NORMAL, 1, 2 },
    { "practice",       	do_practice,	POS_SLEEPING,    	0,  LOG_NORMAL, 1, 2 },
    { "preach",      		do_preach,		POS_STANDING,    	0,  LOG_NORMAL, 1, 2 },
    { "push",           	do_push,        	POS_STANDING,    	0,  LOG_NORMAL, 1, 2 },
    { "rest",			do_rest,		POS_SLEEPING,	0,  LOG_NORMAL, 1, 2 },
    { "sendmail", 		do_sendmail, 	POS_RESTING, 	0,  LOG_NORMAL, 1, 2 },
    { "sit",			do_sit,		POS_SLEEPING,    	0,  LOG_NORMAL, 1, 2 },
    { "stand",			do_stand,		POS_SLEEPING,	0,  LOG_NORMAL, 1, 2 },
    { "tell",			do_tell,		POS_RESTING,	0,  LOG_NORMAL, 1, 2 },
    { "unlock",        		do_unlock,      	POS_RESTING,     	0,  LOG_NORMAL, 1, 2 },
    { "unmask",			do_unmask,		POS_RESTING,	0,  LOG_NORMAL, 1, 2 },
    { "wedge",			do_wedge,		POS_STANDING,	0,  LOG_NORMAL, 1, 2 },
    { "pry",			do_pry,		POS_STANDING,	0,  LOG_NORMAL, 1, 2 },
    { "wield",			do_wear,		POS_RESTING,	0,  LOG_NORMAL, 1, 2 },
    { "donate",         	do_donate,     	POS_RESTING,     	0,  LOG_NORMAL, 1, 2 },
    { "glide",			do_glide,		POS_STANDING,	0,  LOG_NORMAL, 1, 2 },
    { "land",			do_land,		POS_STANDING,	0,  LOG_NORMAL, 1, 2 },
    { "dual",			do_dual_wield,	POS_RESTING,	0,  LOG_NORMAL, 1, 2 },
    { "mount",			do_mount,		POS_STANDING,	0,  LOG_NORMAL, 1, 2 },
    { "dismount",			do_dismount,	POS_FIGHTING,	0,  LOG_NORMAL, 1, 2 },
    {	"apprehend",		do_apprehend,	POS_STANDING,	0,  LOG_NORMAL, 1, 2 },
    { "heel",			do_heel,		POS_STANDING,	0,  LOG_NORMAL, 1, 2 },
    { "eyerub",			do_eyerub,		POS_FIGHTING,	0,  LOG_NORMAL, 1, 2 },
    { "instruct",			do_instruct,	POS_STANDING,	0,  LOG_NORMAL, 1, 2 },
    { "treeform",			do_treeform,	POS_STANDING,	0,  LOG_NORMAL, 1, 2 },
    
     /*
     * Bank commands.
     */
    { "deposit",			do_deposit,		POS_RESTING,	0,  LOG_NORMAL, 1, 2 },
    { "withdraw",			do_withdraw,	POS_RESTING,	0,  LOG_NORMAL, 1, 2 },
    { "balance",			do_balance,		POS_RESTING,	0,  LOG_NORMAL, 1, 2 },

     /*
     * Informational commands.
     */
    { "affects",			do_affects,		POS_DEAD,	 	0,  LOG_NORMAL, 1, 4 },
    { "areas",			do_areas,		POS_DEAD,	 	0,  LOG_NORMAL, 1, 4 },
    { "bug",			do_bug,		POS_DEAD,	 	0,  LOG_NORMAL, 1, 4 },
    { "back",			do_backward,    	POS_FIGHTING,    	0,  LOG_NORMAL, 1, 4 },
    { "backward",     		do_backward,    	POS_FIGHTING,    	0,  LOG_NORMAL, 1, 4 },
    { "advance",      		do_advance,     	POS_FIGHTING,    	0,  LOG_NORMAL, 1, 4 },
    { "changes",			do_changes,		POS_DEAD,	 	0,  LOG_NORMAL, 1, 4 },
    { "clist",          	do_clist,       	POS_DEAD,    	0,  LOG_NORMAL, 1, 4 },
    { "commands",			do_commands,	POS_DEAD,	 	0,  LOG_NORMAL, 1, 4 },
    { "compare",			do_compare,		POS_RESTING,	0,  LOG_NORMAL, 1, 4 },
    { "consider",			do_consider,	POS_RESTING,	0,  LOG_NORMAL, 1, 4 },
    { "count",			do_count,		POS_SLEEPING,	0,  LOG_NORMAL, 1, 4 },
    { "credits",			do_credits,		POS_DEAD,	 	0,  LOG_NORMAL, 1, 4 },
    { "equipment",		do_equipment,	POS_DEAD,	 	0,  LOG_NORMAL, 1, 4 },
    { "examine",			do_examine,		POS_RESTING,	0,  LOG_NORMAL, 1, 4 },
    { "help",			do_help,		POS_DEAD,	 	0,  LOG_NORMAL, 1, 4 },
    { "idea",			do_idea,		POS_DEAD,	 	2,  LOG_NORMAL, 1, 4 },
    { "speak",       		do_speak,    	POS_SLEEPING,    	0,  LOG_NORMAL, 1, 4 },
    { "motd",			do_motd,		POS_DEAD,        	0,  LOG_NORMAL, 1, 4 },
    { "occupancy",		do_occupancy,	POS_DEAD,        	0,  LOG_NORMAL, 1, 4 },
    { "story",			do_news,		POS_DEAD,	 	0,  LOG_NORMAL, 1, 4 },
    { "read",			do_read,		POS_RESTING,	0,  LOG_NORMAL, 1, 4 },
    { "rules",			do_rules,		POS_DEAD,	 	0,  LOG_NORMAL, 1, 4 },
    { "scan",           	do_scan,        	POS_RESTING,     	0,  LOG_NORMAL, 1, 4 },
    { "score",			do_score,		POS_DEAD,	 	0,  LOG_NORMAL, 1, 4 },
    { "scout",			do_scout,        	POS_SLEEPING,     0,  LOG_NORMAL, 1, 4 },
    { "skills",			do_skills,		POS_DEAD,	 	0,  LOG_NORMAL, 1, 4 },
    { "socials",			do_socials,		POS_DEAD,	 	0,  LOG_NORMAL, 1, 4 },
    { "show",			do_show,		POS_DEAD,	 	0,  LOG_NORMAL, 1, 4 },
    { "spells",			do_spells,		POS_DEAD,	 	0,  LOG_NORMAL, 1, 4 },
    { "story",			do_story,		POS_DEAD,	 	0,  LOG_NORMAL, 1, 4 },
    { "time",			do_time,		POS_DEAD,	 	0,  LOG_NORMAL, 1, 4 },
    { "typo",			do_typo,		POS_DEAD,	 	0,  LOG_NORMAL, 1, 4 },
    { "weather",			do_weather,		POS_RESTING,	0,  LOG_NORMAL, 1, 4 },
    { "who",			do_who,		POS_DEAD,	 	0,  LOG_NORMAL, 1, 4 },
    { "whois",			do_whois,		POS_DEAD,	 	0,  LOG_NORMAL, 1, 4 },
    { "wilderness lore",	do_wilderness,   	POS_RESTING	,     0,  LOG_NORMAL, 1, 4 },
    { "worth",			do_worth,		POS_SLEEPING,	0,  LOG_NORMAL, 1, 4 },

    /*
     * Configuration commands.
     */
    { "alia",			do_alia,		POS_DEAD,	 	0,  LOG_NORMAL, 0, 1 },
    { "alias",			do_alias,		POS_DEAD,	 	0,  LOG_NORMAL, 1, 1 },
    { "autolist",			do_autolist,	POS_DEAD,	 	0,  LOG_NORMAL, 1, 1 },
    { "autoassist",		do_autoassist,	POS_DEAD,        	0,  LOG_NORMAL, 1, 1 },
    { "automercy",		do_automercy,	POS_DEAD,        	0,  LOG_NORMAL, 1, 1 },
    { "autotitle",		do_autotitle,	POS_DEAD,        	0,  LOG_NORMAL, 1, 1 },
    { "autoyield",      	do_autoyield,   	POS_DEAD,        	0,  LOG_NORMAL, 1, 1 },
    { "autoexit",			do_autoexit,	POS_DEAD,        	0,  LOG_NORMAL, 1, 1 },
    { "autogold",			do_autogold,	POS_DEAD,        	0,  LOG_NORMAL, 1, 1 },
    { "autoloot",			do_autoloot,	POS_DEAD,        	0,  LOG_NORMAL, 1, 1 },
    { "autosac",			do_autosac,		POS_DEAD,        	0,  LOG_NORMAL, 1, 1 },
    { "autosplit",		do_autosplit,	POS_DEAD,        	0,  LOG_NORMAL, 1, 1 },
    { "background",		do_background,	POS_DEAD,	 	0,  LOG_NORMAL, 1, 1 },
    { "brief",			do_brief,		POS_DEAD,        	10, LOG_NORMAL, 1, 1 },
/*  { "channels",			do_channels,	POS_DEAD,	 	0,  LOG_NORMAL, 1, 1 }, */
    { "colour",			do_colour,		POS_DEAD,        	0,  LOG_NORMAL, 1, 1 },
    { "color",			do_colour,		POS_DEAD,        	0,  LOG_NORMAL, 1, 1 },
    { "combine",			do_combine,		POS_DEAD,        	0,  LOG_NORMAL, 1, 1 },
    { "compact",			do_compact,		POS_DEAD,        	0,  LOG_NORMAL, 1, 1 },
    { "description",		do_description,	POS_DEAD,	 	0,  LOG_NORMAL, 1, 1 },
    { "delet",			do_delet,		POS_DEAD,	 	0,  LOG_ALWAYS, 0, 1 },
    { "delete",			do_delete,		POS_STANDING,	0,  LOG_ALWAYS, 1, 1 },
    { "nofollow",			do_nofollow,	POS_DEAD,        	0,  LOG_NORMAL, 1, 1 },
    { "noloot",			do_noloot,		POS_DEAD,        	0,  LOG_NORMAL, 1, 1 },
    { "outfit",			do_outfit,		POS_RESTING,	0,  LOG_NORMAL, 1, 1 },
    { "password",			do_password,	POS_DEAD,	 	0,  LOG_NEVER,  1, 1 },
    { "prompt",			do_prompt,		POS_DEAD,        	0,  LOG_NORMAL, 1, 1 },
    { "scroll",			do_scroll,		POS_DEAD,	 	0,  LOG_NORMAL, 1, 1 }, 
    { "email",			do_email,		POS_DEAD,	 	0,  LOG_NORMAL, 1, 1 },
    { "unalias",			do_unalias,		POS_DEAD,	 	0,  LOG_NORMAL, 1, 1 },
    { "wimpy",			do_wimpy,		POS_DEAD,	 	0,  LOG_NORMAL, 1, 1 },
    { "devote",        		do_devote,      	POS_RESTING,     	0,  LOG_ALWAYS, 1, 1 },
    { "mark",           	do_mark,        	POS_DEAD,        	0,  LOG_ALWAYS, 1, 1 },
    { "yield",			do_yield,		POS_DEAD,	 	0,  LOG_NORMAL, 1, 1 },

    /*
     * Communication commands.
     */
    { "afk",			do_afk,		POS_SLEEPING,	0,  LOG_NORMAL, 1, 3 },
    { "cpose",          	do_cpose,       	POS_RESTING,	0,  LOG_NORMAL, 1, 3 },
    { "deaf",			do_deaf,		POS_DEAD,	 	0,  LOG_NORMAL, 1, 3 },
    { "emote",			do_emote,		POS_RESTING,	0,  LOG_NORMAL, 1, 3 },
    { "pmote",			do_pmote,		POS_RESTING,	0,  LOG_NORMAL, 1, 3 },
    { ",",				do_emote,		POS_RESTING,	0,  LOG_NORMAL, 0, 3 },
    { "gtell",			do_gtell,		POS_DEAD,	 	0,  LOG_NORMAL, 1, 3 },
    { ";",				do_gtell,		POS_DEAD,	 	0,  LOG_NORMAL, 0, 3 },
    { "ignore",			do_ignore,		POS_SLEEPING, 	0,  LOG_NORMAL, 1, 3 },
    { "unignore",			do_unignore,	POS_SLEEPING, 	0,  LOG_NORMAL, 1, 3 },
    { "note",			do_note,		POS_SLEEPING,	2,  LOG_NORMAL, 1, 3 },
    { "pose",			do_pose,		POS_RESTING,	0,  LOG_NORMAL, 1, 3 },
    { "quiet",			do_quiet,		POS_SLEEPING, 	0,  LOG_NORMAL, 1, 3 },
    { "reply",			do_reply,		POS_SLEEPING,	0,  LOG_NORMAL, 1, 3 },
    { "replay",			do_replay,		POS_SLEEPING,	0,  LOG_NORMAL, 1, 3 },
    { "say",			do_say,		POS_RESTING,	0,  LOG_NORMAL, 1, 3 },
    { "sing",           	do_sing,        	POS_RESTING,     	0,  LOG_NORMAL, 1, 3 },
    { "song",           	do_song,        	POS_SLEEPING,    	0,  LOG_NORMAL, 1, 3 },
    { "'",				do_say,		POS_RESTING,	0,  LOG_NORMAL, 0, 3 },
    { "unread",			do_unread,		POS_SLEEPING,    	0,  LOG_NORMAL, 1, 3 },
    { "whisper",       	 	do_whisper,     	POS_RESTING,     	0,  LOG_NORMAL, 1, 3 },
    { "yell",			do_yell,		POS_RESTING,	0,  LOG_NORMAL, 1, 3 },
    { "pray",           	do_pray,        	POS_DEAD,    	0,  LOG_NORMAL, 1, 3 },
    { "windtalk",			do_windtalk,	POS_RESTING,	0,  LOG_NORMAL, 1, 3 },

    /*
     * Object manipulation commands.
     */
    { "brandish",		do_brandish,	POS_RESTING,	0,  LOG_NORMAL, 1, 5 },
    { "close",		do_close,		POS_RESTING,	0,  LOG_NORMAL, 1, 5 },
    { "brew",           do_brew,        	POS_RESTING,     	0,  LOG_NORMAL, 1, 5 },
    { "scribe",         do_scribe,        POS_RESTING,     	0,  LOG_NORMAL, 1, 5 },
    { "drain blood",	do_drain,		POS_RESTING,	0,  LOG_NORMAL, 0, 5 },
    { "drink",		do_drink,		POS_RESTING,	0,  LOG_NORMAL, 1, 5 },
    { "drop",		do_drop,		POS_RESTING,	0,  LOG_NORMAL, 1, 5 },
    { "eat",		do_eat,		POS_RESTING,	0,  LOG_NORMAL, 1, 5 },
    { "envenom",		do_envenom,		POS_RESTING,	0,  LOG_NORMAL, 1, 5 },
    { "fill",		do_fill,		POS_RESTING,	0,  LOG_NORMAL, 1, 5 },
    { "give",		do_give,		POS_RESTING,	0,  LOG_NORMAL, 1, 5 },
    { "heal",		do_heal,		POS_RESTING,	0,  LOG_NORMAL, 1, 5 }, 
    { "hold",		do_wear,		POS_RESTING,	0,  LOG_NORMAL, 1, 5 },
    { "key origin",	do_key_origin,	POS_RESTING,	0,  LOG_NORMAL, 1, 5 },
    { "list",		do_list,		POS_RESTING,	0,  LOG_NORMAL, 1, 5 },
    { "lock",		do_lock,		POS_RESTING,	0,  LOG_NORMAL, 1, 5 },
    { "open",		do_open,		POS_RESTING,	0,  LOG_NORMAL, 1, 5 },
    { "pick",		do_pick,		POS_RESTING,	0,  LOG_NORMAL, 1, 5 },
    { "polish",		do_polish,		POS_RESTING,	0,  LOG_NORMAL, 1, 5 },
    { "pour",		do_pour,		POS_RESTING,	0,  LOG_NORMAL, 1, 5 },
    { "put",		do_put,	      POS_RESTING,	0,  LOG_NORMAL, 1, 5 },
    { "quaff",		do_quaff,		POS_RESTING,	0,  LOG_NORMAL, 1, 5 },
    { "recite",		do_recite,		POS_RESTING,	0,  LOG_NORMAL, 1, 5 },
    { "remove",		do_remove,		POS_RESTING,	0,  LOG_NORMAL, 1, 5 },
    { "sell",		do_sell,		POS_RESTING,	0,  LOG_NORMAL, 1, 5 },
    { "sharpen",		do_sharpen,		POS_RESTING,	0,  LOG_NORMAL, 1, 5 },
    { "take",		do_get,		POS_RESTING,	0,  LOG_NORMAL, 1, 5 },
    { "sacrifice",	do_sacrifice,	POS_RESTING,	0,  LOG_NORMAL, 1, 5 },
    { "junk",           do_sacrifice,   	POS_RESTING,     	0,  LOG_NORMAL, 1, 5 },
    { "tap",      	do_sacrifice,   	POS_RESTING,     	0,  LOG_NORMAL, 1, 5 },
    { "trophy",      	do_trophy,		POS_RESTING,	0,  LOG_NORMAL, 1, 5 },
/*  { "unlock",		do_unlock,		POS_RESTING,	0,  LOG_NORMAL, 1, 5 }, */
    { "value",		do_value,		POS_RESTING,	0,  LOG_NORMAL, 1, 5 },
    { "wear",		do_wear,		POS_RESTING,	0,  LOG_NORMAL, 1, 5 },
    { "zap",		do_zap,		POS_RESTING,	0,  LOG_NORMAL, 1, 5 },
    { "deathgrip",      do_deathgrip,   	POS_RESTING,     	0,  LOG_NORMAL, 1, 5 },
    { "demand",         do_demand,      	POS_STANDING,    	0,  LOG_NORMAL, 1, 5 },
    { "bj",			do_blackjack,   	POS_STANDING, 	0,  LOG_NORMAL, 1, 5 },
    { "blackjack",	do_blackjack,   	POS_STANDING, 	0,  LOG_NORMAL, 1, 5 },
    { "butcher",		do_butcher, 	POS_RESTING,	0,  LOG_NORMAL, 1, 5 },
    { "campfire",		do_campfire, 	POS_RESTING, 	0,  LOG_NORMAL, 1, 5 },
    { "search water", 	do_search_water,	POS_RESTING, 	0,  LOG_NORMAL, 1, 5 },
    { "mend",		do_mend,		POS_STANDING,	0,  LOG_NORMAL, 1, 5 },
    { "repair shield",	do_repair_shield,	POS_STANDING,	0,  LOG_NORMAL, 1, 0 },
    { "leather repair",	do_leather_repair, POS_STANDING,	0,  LOG_NORMAL, 1, 0 },
    { "plate repair",	do_plate_repair,	POS_STANDING,	0,  LOG_NORMAL, 1, 0 },
    { "chain repair",	do_chain_repair,	POS_STANDING,	0,  LOG_NORMAL, 1, 0 },
    { "weapon repair",	do_weapon_repair,	POS_STANDING,	0,  LOG_NORMAL, 1, 0 },
    { "deathblade",	do_deathblade,	POS_STANDING,	0,  LOG_NORMAL, 1, 5 },
    { "flameblade",	do_flameblade,	POS_STANDING,	0,  LOG_NORMAL, 1, 5 },
    { "runeblade",	do_runeblade,	POS_STANDING,	0,  LOG_NORMAL, 1, 5 },
    { "stainless",	do_stainless,	POS_STANDING,	0,  LOG_NORMAL, 1, 5 },
    { "eversharp",	do_eversharp,	POS_STANDING,	0,  LOG_NORMAL, 1, 5 },
    { "purify",		do_purifying_fire, POS_STANDING,	0,  LOG_NORMAL, 1, 5 },
    { "fletch",		do_fletching,	POS_STANDING,	0,  LOG_NORMAL, 1, 5 },
    { "entwine",		do_entwine,		POS_FIGHTING,	0,  LOG_NORMAL, 1, 5 },
    { "masterwork",	do_masterwork,	POS_STANDING,	0,  LOG_NORMAL, 1, 5 },
    { "dismantle",	do_dismantle,	POS_STANDING,	0,  LOG_NORMAL, 1, 5 },
    { "retool",		do_retool,		POS_STANDING,	0,  LOG_NORMAL, 1, 5 },
    { "craft",		do_craft,		POS_STANDING,	0,  LOG_NORMAL, 1, 5 },


    /*
     * Combat commands.
     */
    { "ambush",			do_ambush,			POS_FIGHTING,	0,  LOG_NORMAL, 1, 7 },
    { "assassinate",		do_assassinate,		POS_FIGHTING,	0,  LOG_NORMAL, 1, 7 },
    { "backstab",			do_backstab,		POS_FIGHTING,	0,  LOG_NORMAL, 1, 7 },
    { "battlecry",      	do_battlecry,     	POS_FIGHTING,     0,  LOG_NORMAL, 1, 7 },
    { "brainjar",			do_brainjar,   		POS_STANDING, 	0,  LOG_NORMAL, 1, 7 },
    { "circlestab",		do_circle,			POS_FIGHTING,	0,  LOG_NORMAL, 1, 7 },
    {	"cleave",			do_cleave,			POS_STANDING,	0,  LOG_NORMAL, 1, 7 },
    { "cutthroat",		do_cutthroat,		POS_STANDING,	0,  LOG_NORMAL, 1, 7 },
    { "mountjack",		do_mountjack,		POS_STANDING,	0,  LOG_NORMAL, 1, 7 },
    { "bash",			do_bash,			POS_FIGHTING,    	0,  LOG_NORMAL, 1, 7 },
    { "bs",				do_backstab,		POS_FIGHTING,	0,  LOG_NORMAL, 0, 7 },
    { "berserk",			do_berserk,			POS_FIGHTING,	0,  LOG_NORMAL, 1, 7 },
    { "dirt",			do_dirt,			POS_FIGHTING,	0,  LOG_NORMAL, 1, 7 },
    { "disarm",			do_disarm,			POS_FIGHTING,	0,  LOG_NORMAL, 1, 7 },
    { "finger break",		do_fingerbreak,		POS_FIGHTING,	0,  LOG_NORMAL, 1, 7 },
    { "fire",			do_fire,			POS_FIGHTING,	0,  LOG_NORMAL, 1, 7 },
    { "flank",          	do_flank_attack, 		POS_FIGHTING,   	0,  LOG_NORMAL, 1, 7 },
    { "flee",			do_flee,			POS_FIGHTING,	0,  LOG_NORMAL, 1, 7 },
    { "head butt",		do_head_butt,		POS_FIGHTING,	0,  LOG_NORMAL, 1, 7 },
    { "hunt",           	do_hunt,        		POS_STANDING,    	0,  LOG_NORMAL, 1, 7 },
    {	"impale",			do_impale,			POS_STANDING,	0,  LOG_NORMAL, 1, 7 },
    {	"inspire",			do_inspire,			POS_STANDING,	0,  LOG_NORMAL, 1, 7 },
    { "jab",			do_jab,			POS_FIGHTING,	0,  LOG_NORMAL, 1, 7 },
    { "joust",			do_joust,			POS_STANDING,	0,  LOG_NORMAL, 1, 7 },
    { "kick",			do_kick,			POS_FIGHTING,	0,  LOG_NORMAL, 1, 7 },
    { "murde",			do_murde,			POS_FIGHTING,	0,  LOG_NORMAL, 0, 7 },
    { "murder",			do_murder,			POS_FIGHTING,	5,  LOG_ALWAYS, 1, 7 },
    { "rally",			do_rally,			POS_FIGHTING,	0,  LOG_NORMAL, 1, 7 },
    { "rescue",			do_rescue,			POS_FIGHTING,	0,  LOG_NORMAL, 1, 7 },
    { "strangle",			do_strangle,		POS_STANDING,	0,  LOG_NORMAL, 1, 7 },
    { "shield rush",		do_shield_rush,		POS_STANDING,	0,  LOG_NORMAL, 1, 7 },
    { "shatter shield", 	do_shield_shatter,	POS_FIGHTING,   	0,  LOG_NORMAL, 1, 7 },
    { "surge",          	do_surge,       		POS_FIGHTING,	0,  LOG_NORMAL, 1, 7 },
    { "sweep",			do_sweep,			POS_FIGHTING,    	0,  LOG_NORMAL, 1, 7 },
    { "tonguecut",		do_tonguecut,		POS_FIGHTING,	0,  LOG_NORMAL, 1, 7 },
    { "trammel",			do_trammel,			POS_FIGHTING,    	0,  LOG_NORMAL, 1, 7 },
    { "trip",			do_trip,			POS_FIGHTING,    	0,  LOG_NORMAL, 1, 7 },
    { "unholyrite",		do_unholyrite,		POS_FIGHTING,	0,  LOG_NORMAL, 1, 7 },
    { "break",          	do_break_weapon, 		POS_FIGHTING,   	0,  LOG_NORMAL, 1, 7 },
    { "breakneck",		do_breakneck,		POS_STANDING,	0,  LOG_NORMAL, 1, 7 },
    { "whirlwind",		do_whirlwind,   		POS_FIGHTING,  	1,  LOG_NORMAL, 1, 7 },
    { "lunge",			do_lunge,   		POS_FIGHTING,  	1,  LOG_NORMAL, 1, 7 },
    { "ravage bite",		do_ravage_bite,		POS_FIGHTING, 	0,  LOG_NORMAL, 1, 7 },
    { "nerve pinch", 		do_nerve_pinch, 		POS_FIGHTING, 	0,  LOG_NORMAL, 1, 7 },
    { "caltrops",			do_caltrops, 		POS_FIGHTING, 	0,  LOG_NORMAL, 1, 7 },
    { "pillify",			do_pillify, 		POS_SLEEPING, 	0,  LOG_NORMAL, 1, 7 },
    { "crush",			do_crush,			POS_FIGHTING,	0,  LOG_NORMAL, 1, 7 },
    { "tail",			do_tail,			POS_FIGHTING,	0,  LOG_NORMAL, 1, 7 },
    { "execute",			do_execute,			POS_FIGHTING,	0,  LOG_NORMAL, 1, 7 },
    { "earpunch",			do_earpunch,		POS_FIGHTING,	0,  LOG_NORMAL, 1, 7 },
    { "rupture organ",		do_rupture_organ,		POS_FIGHTING,	0,  LOG_NORMAL, 1, 7 },
    { "dancing strike",		do_dancing_strike,	POS_FIGHTING,	0,  LOG_NORMAL, 1, 7 },
    /* { "bestow curse",		do_bestow_curse,		POS_STANDING,	0,  LOG_NORMAL, 1, 7 }, */
    { "beastspite",		do_beastspite,		POS_STANDING,	0,  LOG_NORMAL, 1, 7 },
    { "relegate",			do_relegate,		POS_STANDING,	0,  LOG_NORMAL, 1, 7 },
    
    
    /*
     * Mob command interpreter (placed here for faster scan...)
     */
    { "mob",			do_mob,			POS_DEAD,	 	0,  LOG_NEVER,  0, 1 },

    /*
     * Miscellaneous commands.
     */
    { "autodig",   		do_autodig,   		POS_DEAD,     	IM, LOG_NORMAL, 1, 2 },
    { "enter", 			do_enter, 			POS_STANDING,	0,  LOG_NORMAL, 1, 2 },
    {	"backup",			do_backup,			POS_STANDING,	0,  LOG_NORMAL, 1, 2 },
    { "bearcall",			do_bear_call,   		POS_STANDING,    	0,  LOG_NORMAL, 1, 2 },
    { "bounty",			do_bounty,			POS_STANDING,	0,  LOG_NORMAL, 1, 2 },
    { "endure",         	do_endure,      		POS_STANDING,	0,  LOG_NORMAL, 1, 2 },
    { "follow",			do_follow,			POS_RESTING,	0,  LOG_NORMAL, 1, 2 },
    { "gain",			do_gain,			POS_STANDING,	0,  LOG_NORMAL, 1, 2 },
    { "glance",			do_glance,			POS_RESTING,	0,  LOG_NORMAL, 1, 2 },
    { "go",				do_enter,			POS_STANDING,	0,  LOG_NORMAL, 0, 2 },
/*  { "group",			do_group,			POS_SLEEPING,	0,  LOG_NORMAL, 1, 2 }, */
    { "guard",			do_guard,			POS_FIGHTING,	0,  LOG_NORMAL, 1, 2 },
    { "hide",			do_hide,			POS_RESTING,	0,  LOG_NORMAL, 1, 2 },
    { "camouflage", 		do_camouflage, 		POS_RESTING,     	0,  LOG_NORMAL, 0, 2 },
    { "charge",			do_charge,			POS_RESTING,	0,  LOG_NORMAL, 1, 2 },
    { "lull",           	do_lull,        		POS_RESTING,	0,  LOG_NORMAL, 1, 2 },
    { "pacify",         	do_pacify,      		POS_RESTING,	0,  LOG_NORMAL, 1, 2 },
    { "pardon",			do_pardon,			POS_RESTING,	0,  LOG_NORMAL, 1, 2 },
    { "play music",		do_play,		POS_RESTING,	0,  LOG_NORMAL, 1, 2 },
/*  { "practice",			do_practice,	POS_SLEEPING,	0,  LOG_NORMAL, 1, 2 }, */
    { "quest",          	do_quest,   	POS_DEAD,     	0,  LOG_NORMAL, 1, 2 },
    { "qui",			do_qui,		POS_DEAD,	 	0,  LOG_NORMAL, 0, 2 },
    { "quit",			do_quit,		POS_DEAD,	 	0,  LOG_NORMAL, 1, 2 },
    { "recall",			do_recall,		POS_FIGHTING,	0,  LOG_NORMAL, 1, 2 },
    { "/",				do_recall,		POS_FIGHTING,	0,  LOG_NORMAL, 0, 2 },
    { "rent",			do_rent,		POS_DEAD,	 	0,  LOG_NORMAL, 0, 2 },
    { "save",			do_save,		POS_DEAD,	 	0,  LOG_NORMAL, 1, 2 },
    { "serenade",       	do_serenade,    	POS_RESTING,	0,  LOG_NORMAL, 1, 2 },
    { "sleep",			do_sleep,		POS_SLEEPING,	0,  LOG_NORMAL, 1, 2 },
    { "smother",			do_smother,		POS_STANDING,	0,  LOG_NORMAL, 1, 2 },
    { "sneak",			do_sneak,		POS_STANDING,	0,  LOG_NORMAL, 1, 2 },
    { "split",			do_split,		POS_RESTING,	0,  LOG_NORMAL, 1, 2 },
    { "stalk",          	do_stalk,        	POS_STANDING,    	0,  LOG_NORMAL, 1, 2 },
    { "steal",			do_steal,		POS_STANDING,	0,  LOG_NORMAL, 1, 2 },
    { "strip",			do_strip,		POS_STANDING,	0,  LOG_NORMAL, 1, 2 },
    { "squirecall",		do_squire_call,	POS_STANDING,	0,  LOG_NORMAL, 1, 2 },
    { "pickpocket",		do_pickpocket, 	POS_STANDING, 	0,  LOG_NORMAL, 1, 2 },
    { "tame",			do_tame,		POS_STANDING,	0,  LOG_NORMAL, 1, 2 },
    { "taunt",         	 	do_taunt,       	POS_RESTING,	0,  LOG_NORMAL, 1, 2 },
    { "train",			do_train,		POS_RESTING,	0,  LOG_NORMAL, 1, 2 },
    { "visible",			do_visible,		POS_SLEEPING,	0,  LOG_NORMAL, 1, 2 },
    { "version",			do_version,		POS_SLEEPING,	0,  LOG_NORMAL, 1, 2 },
    { "wake",			do_wake,		POS_SLEEPING,	0,  LOG_NORMAL, 1, 2 },
    { "where",			do_where,		POS_RESTING,	0,  LOG_NORMAL, 1, 2 },
    { "track",			do_track,		POS_STANDING,	0,  LOG_NORMAL, 1, 2 },
    { "lore",			do_lore,		POS_RESTING,	0,  LOG_NORMAL, 1, 2 },
    { "traject",			do_traject,		POS_STANDING,	0,  LOG_NORMAL, 1, 2 },
    { "songs",			do_songs,		POS_DEAD,	 	0,  LOG_NORMAL, 1, 2 },
    { "find",           	do_find,        	POS_STANDING,    	0,  LOG_NORMAL, 1, 2 },
    { "bind wounds",		do_bind,		POS_STANDING,	0,  LOG_NORMAL, 0, 2 },
    { "bravado",        	do_bravado,     	POS_RESTING,	0,  LOG_NORMAL, 0, 2 },
    { "tether",			do_tether,		POS_STANDING,	0,  LOG_NORMAL, 1, 2 },
    { "untether",			do_untether,	POS_STANDING,	0,  LOG_NORMAL, 1, 2 },

    /*
     * Immortal commands.
     */
    { "addapply",			do_addapply,	POS_DEAD,		L5, LOG_ALWAYS, 1, 0 },
    { "admit",          	do_admit,       	POS_DEAD,       	 1, LOG_ALWAYS, 1, 0 },
    { "aedit",			do_aedit,		POS_DEAD,    	L8, LOG_NORMAL, 1, 0 },
    { "alist",			do_alist,		POS_DEAD,    	L8, LOG_NORMAL, 1, 0 },
    { "allow",			do_allow,		POS_DEAD,		L5, LOG_ALWAYS, 1, 0 },
    { "apurge", 			do_apurge, 		POS_DEAD, 		L5, LOG_ALWAYS, 1, 0 },
    { "asave",          	do_asave,		POS_DEAD,    	L8, LOG_ALWAYS, 1, 0 },
    { "asperson",			do_asperson, 	POS_DEAD,		L3, LOG_ALWAYS, 1, 0 },
    { "at",             	do_at,          	POS_DEAD,       	L5, LOG_NORMAL, 1, 0 },
    { "ban",			do_ban,		POS_DEAD,		L3, LOG_ALWAYS, 1, 0 },
    { "backcheck",		do_backcheck,	POS_DEAD,		L8, LOG_NORMAL, 1, 0 },
    { "check",			do_check,		POS_DEAD,		L2, LOG_ALWAYS, 1, 0 },
    { "clean",          	do_clean,		POS_DEAD,		ML, LOG_ALWAYS, 1, 0 },
    { "clone",			do_clone,		POS_DEAD,		L6, LOG_ALWAYS, 1, 0 },
    { "copyover",			do_copyover,	POS_DEAD,       	ML, LOG_ALWAYS, 1, 0 },
    { "deny",			do_deny,		POS_DEAD,		L3, LOG_ALWAYS, 1, 0 },
    { "desocket", 		do_desocket, 	POS_DEAD, 		L4, LOG_ALWAYS, 1, 0 },
    { "disable",			do_disable,		POS_DEAD,		ML, LOG_ALWAYS, 1, 0 },
    { "disconnect",		do_disconnect,	POS_DEAD,		L2, LOG_ALWAYS, 1, 0 },
    { "dismiss",			do_dismiss,		POS_DEAD,		1,  LOG_NORMAL, 1, 0 },
    { "dump",			do_dump,		POS_DEAD,		ML, LOG_ALWAYS, 0, 0 },
    { "echo",			do_recho,		POS_DEAD,		L7, LOG_ALWAYS, 1, 0 },
    { "edit",			do_olc,		POS_DEAD,    	L8, LOG_NORMAL, 1, 0 },
    { "empower",			do_empower,		POS_DEAD,		ML, LOG_ALWAYS, 1, 0 },
    { "evict",			do_evict,		POS_DEAD,		1,  LOG_ALWAYS, 1, 0 },
    { "exlist",			do_exlist,		POS_DEAD,		L7, LOG_NORMAL, 1, 0 },
    { "extrabit",			do_extrabit, 	POS_DEAD,		L5, LOG_NORMAL, 1, 0 },
    { "fadein",			do_fadein,     	POS_DEAD,       	L5, LOG_NORMAL, 1, 0 },
    { "fadeout",			do_fadeout,    	POS_DEAD,       	L5, LOG_NORMAL, 1, 0 },
    { "flag",			do_flag,		POS_DEAD,		ML, LOG_ALWAYS, 1, 0 },
    { "for",			do_for,		POS_DEAD,		L3, LOG_NORMAL, 1, 0 },
    { "force",			do_force,		POS_DEAD,		L3, LOG_ALWAYS, 1, 0 },
    { "freeze",			do_freeze,		POS_DEAD,		L4, LOG_ALWAYS, 1, 0 },
    { "fremove",			do_fremove,		POS_DEAD,		L6, LOG_ALWAYS, 1, 0 },
    { "fslay",			do_fslay,    	POS_DEAD,		L4, LOG_ALWAYS, 1, 0 },
    { "fvlist",			do_fvlist,    	POS_DEAD,		L8, LOG_ALWAYS, 1, 0 },
    { "gecho",			do_echo,		POS_DEAD,		L5, LOG_ALWAYS, 1, 0 },
    { "goto",           	do_goto,        	POS_DEAD,       	L8, LOG_NORMAL, 1, 0 },
    { "grab",           	do_grab,		POS_DEAD,		L5, LOG_ALWAYS, 1, 0 },
    { "igrab",          	do_igrab,		POS_DEAD,		ML, LOG_ALWAYS, 1, 0 },
    { "illegalname",		do_illegalname,	POS_DEAD,		L2, LOG_ALWAYS, 1, 0 },
    { "hedit",			do_hedit,		POS_DEAD,    	ML, LOG_NORMAL, 1, 0 },
    { "holylight",		do_holylight,	POS_DEAD,		L8, LOG_NORMAL, 1, 0 },
    { "immtalk",			do_immtalk,		POS_DEAD,		L8, LOG_NORMAL, 1, 0 },
    { "imotd",          	do_imotd,       	POS_DEAD,       	L8, LOG_NORMAL, 1, 0 },
    { "immortalfy",		do_immortalfy,	POS_DEAD,		ML, LOG_NORMAL, 1, 0 },
    { ":",				do_immtalk,		POS_DEAD,		L8, LOG_NORMAL, 0, 0 },
    { "incognito",		do_incognito,	POS_DEAD,		L8, LOG_NORMAL, 1, 0 },
    { "invis",			do_invis,		POS_DEAD,		L8, LOG_NORMAL, 0, 0 },
    { "laston",         	do_laston,		POS_DEAD,		L6, LOG_ALWAYS, 1, 0 },
    { "lastonimms",     	do_laston_immortals, POS_DEAD,	L1, LOG_ALWAYS, 1, 0 },
    { "load",			do_load,		POS_DEAD,		L4, LOG_ALWAYS, 1, 0 },
    { "log",			do_log,		POS_DEAD,		L3, LOG_ALWAYS, 1, 0 },
    { "losereply",		do_losereply,	POS_DEAD,		101, LOG_ALWAYS, 1, 0 },
    { "maskip",			do_maskip,		POS_DEAD,		ML, LOG_ALWAYS, 1, 0 },
    { "medit",			do_medit,		POS_DEAD,    	L8, LOG_NORMAL, 1, 0 },
    { "memory",			do_memory,		POS_DEAD,		IM, LOG_NORMAL, 1, 0 },
    { "mpdump",			do_mpdump,		POS_DEAD,		L6, LOG_NEVER,  1, 0 },
    { "mpedit",			do_mpedit,		POS_DEAD,    	L8, LOG_NORMAL, 1, 0 },
    { "mpstat",			do_mpstat,		POS_DEAD,		L6, LOG_NEVER,  1, 0 },
    { "mwhere",			do_mwhere,		POS_DEAD,		L7, LOG_NORMAL, 1, 0 },
    { "newlock",			do_newlock,		POS_DEAD,		L1, LOG_ALWAYS, 1, 0 },
    { "nochannels",		do_nochannels,	POS_DEAD,		L5, LOG_ALWAYS, 1, 0 },
    { "noemote",			do_noemote,		POS_DEAD,		L3, LOG_ALWAYS, 1, 0 },
    { "noexp",			do_noexp,		POS_DEAD,		L6, LOG_ALWAYS, 1, 0 },
    { "noidle",			do_noidle,		POS_DEAD,		ML, LOG_ALWAYS, 1, 0 },
    { "nonote",			do_nonote,		POS_DEAD,		L6, LOG_ALWAYS, 1, 0 },
    { "nopk",			do_nopk,		POS_DEAD,		L5, LOG_ALWAYS, 1, 0 },
    { "noshout",			do_noshout,		POS_DEAD,		L4, LOG_ALWAYS, 1, 0 },
    { "notell",			do_notell,		POS_DEAD,		L3, LOG_ALWAYS, 1, 0 },
    { "notitle",			do_notitle,		POS_DEAD,		L6, LOG_ALWAYS, 1, 0 },
    { "oedit",			do_oedit,		POS_DEAD,    	L8, LOG_NORMAL, 1, 0 },
    { "opdump",			do_opdump,		POS_DEAD,	 	IM, LOG_NEVER,  1, 0 },
    { "opedit",         	do_opedit,  	POS_DEAD,    	IM, LOG_ALWAYS, 1, 0 },
    { "opstat",			do_opstat,		POS_DEAD,	 	IM, LOG_NEVER,  1, 0 },
    { "owhere",			do_owhere,		POS_DEAD,		L7, LOG_NORMAL, 1, 0 },
    { "pbackup",			do_pfile_backup,	POS_DEAD,		ML, LOG_ALWAYS, 1, 0 },
    { "peace",			do_peace,		POS_DEAD,		L6, LOG_NORMAL, 1, 0 },
    { "pecho",			do_pecho,		POS_DEAD,		L5, LOG_ALWAYS, 1, 0 },
    { "penalize",			do_penalize,	POS_DEAD,		L6, LOG_ALWAYS, 1, 0 },
    { "penalty",			do_penalty,		POS_DEAD,		IM, LOG_NORMAL, 1, 0 },
    { "permban",			do_permban,		POS_DEAD,		L3, LOG_ALWAYS, 1, 0 },
    { "plist",			do_plist,		POS_DEAD,		ML, LOG_NORMAL, 1, 0 },
    { "pload",			do_pload,		POS_DEAD,		ML, LOG_ALWAYS, 1, 0 },
    { "poofin",			do_bamfin,		POS_DEAD,		IM, LOG_NORMAL, 1, 0 },
    { "poofout",			do_bamfout,		POS_DEAD,		IM, LOG_NORMAL, 1, 0 },
    { "pprestar",			do_pprestar,	POS_DEAD,		ML, LOG_NORMAL, 0, 0 },
    { "pprestart",		do_pprestart,	POS_DEAD,		L1, LOG_ALWAYS, 1, 0 },
    { "prayfile",			do_prayfile,	POS_DEAD,		IM, LOG_NORMAL, 1, 0 },
    { "prefi",			do_prefi,		POS_DEAD,		IM, LOG_NORMAL, 0, 0 },
    { "prefix",			do_prefix,		POS_DEAD,		IM, LOG_NORMAL, 1, 0 },
    { "pretitle", 		do_pretitle, 	POS_DEAD, 		IM, LOG_NORMAL, 1, 0 },
    { "protect",			do_protect,		POS_DEAD,		L4, LOG_ALWAYS, 1, 0 },
    { "punload",			do_punload,		POS_DEAD,		ML, LOG_ALWAYS, 1, 0 },
    { "purge",			do_purge,		POS_DEAD,		IM, LOG_ALWAYS, 1, 0 },
    { "questreset",		do_questreset,	POS_DEAD,		L7, LOG_ALWAYS, 1, 0 },
    { "reboo",			do_reboo,		POS_DEAD,		L1, LOG_NORMAL, 0, 0 },
    { "reboot",			do_reboot,		POS_DEAD,		L1, LOG_ALWAYS, 1, 0 },
    { "redit",			do_redit,		POS_DEAD,    	L8, LOG_NORMAL, 1, 0 },
    { "rename",			do_rename,		POS_DEAD,		L1, LOG_NEVER,  1, 0 },
    { "report",			do_report,		POS_RESTING,	IM, LOG_NORMAL, 1, 0 },
    { "resets",			do_resets,		POS_DEAD,    	L8, LOG_NORMAL, 1, 0 },
    { "restore",			do_restore,		POS_DEAD,		L4, LOG_ALWAYS, 1, 0 },
    { "retribution",    	do_retribution, 	POS_DEAD,       	L6, LOG_NORMAL, 1, 0 },
    { "return",         	do_return,      	POS_DEAD,       	L7, LOG_NORMAL, 1, 0 },
    { "reward",			do_reward,		POS_DEAD,		L6, LOG_ALWAYS, 1, 0 },
    { "roomcheck",		do_roomcheck,	POS_RESTING,	IM, LOG_NORMAL, 1, 0 },
    { "rpdump",			do_rpdump,		POS_DEAD,	 	IM, LOG_NEVER,  1, 0 },
    { "rpedit",         	do_rpedit,  	POS_DEAD,    	IM, LOG_ALWAYS, 1, 0 },
    { "rpstat",			do_rpstat,		POS_DEAD,	 	IM, LOG_NEVER,  1, 0 },
    { "rstrip",         	do_rstrip,      	POS_DEAD,       	IM, LOG_ALWAYS, 1, 1 },
    { "rwhere",         	do_rwhere,      	POS_DEAD,       	L7, LOG_NORMAL, 1, 1 },
    { "salign",			do_salign,		POS_DEAD,		L5, LOG_NORMAL, 1, 0 },
    { "scatter",			do_scatter,		POS_DEAD,		IM, LOG_NORMAL, 1, 0 },
    { "sedit",			do_sedit,		POS_DEAD,	 	ML, LOG_ALWAYS, 1, 0 },
    { "sendhome",      	 	do_sendhome,	POS_DEAD,		IM, LOG_NORMAL, 1, 0 },
    { "set",			do_set,		POS_DEAD,		L1, LOG_ALWAYS, 1, 0 },
    { "sgive",			do_sgive,		POS_DEAD,		L4, LOG_ALWAYS, 1, 0 },
    { "shutdow",			do_shutdow,		POS_DEAD,		ML, LOG_NORMAL, 0, 0 },
    { "shutdown",			do_shutdown,	POS_DEAD,		ML, LOG_ALWAYS, 1, 0 },
    { "sla",			do_sla,	      POS_DEAD,		L3, LOG_NORMAL, 0, 0 },
    { "slay",			do_slay,		POS_DEAD,		L3, LOG_ALWAYS, 1, 0 },
    { "sload",			do_sload,		POS_DEAD,		L4, LOG_ALWAYS, 1, 0 },
    { "slowage",			do_slowage,		POS_DEAD,		L2, LOG_ALWAYS, 1, 0 },
    { "smite",          	do_smite,       	POS_DEAD,       	L6, LOG_ALWAYS, 1, 0 },
    { "smote",			do_smote,		POS_DEAD,		0,  LOG_NORMAL, 1, 2 },
    { "snoop",			do_snoop,		POS_DEAD,		L4, LOG_ALWAYS, 1, 0 },
    { "sockets",        	do_sockets,		POS_DEAD,       	L6, LOG_NORMAL, 1, 0 },
    { "stat",			do_stat,		POS_DEAD,		L7, LOG_NORMAL, 1, 0 },
    { "string",			do_string,		POS_DEAD,		IM, LOG_ALWAYS, 1, 0 },
    { "switch",			do_switch,		POS_DEAD,		L7, LOG_ALWAYS, 1, 0 },
    { "teleport",			do_transfer,    	POS_DEAD,		L5, LOG_ALWAYS, 1, 0 },
    { "tick",			do_tick, 		POS_DEAD, 		ML, LOG_ALWAYS, 1, 0 },
    { "title",			do_title,		POS_DEAD,	 	101, LOG_NORMAL, 1, 0 },
    { "transfer",			do_transfer,	POS_DEAD,		L5, LOG_ALWAYS, 1, 0 },
    { "transin",        	do_transin,     	POS_DEAD,       	L5, LOG_NORMAL, 1, 0 },
    { "transout",      	 	do_transout,    	POS_DEAD,       	L5, LOG_NORMAL, 1, 0 },
    { "trust",			do_trust,		POS_DEAD,		ML, LOG_ALWAYS, 1, 0 },
    { "users",          	do_users,		POS_DEAD,		IM, LOG_NORMAL, 1, 0 },
    { "violate",			do_violate,		POS_DEAD,		L4, LOG_ALWAYS, 1, 0 },
    { "vnum",			do_vnum,		POS_DEAD,		L7, LOG_NORMAL, 1, 0 },
    { "vsearch",			do_vsearch,		POS_DEAD,		L7, LOG_NORMAL, 1, 0 },
    { "wizhelp",			do_wizhelp,		POS_DEAD,		101, LOG_NORMAL, 1, 0 },
    { "wizinvis",			do_invis,		POS_DEAD,		L8, LOG_NORMAL, 1, 0 },
    { "wizlock",			do_wizlock,		POS_DEAD,		ML, LOG_ALWAYS, 1, 0 },
    { "wiznet",			do_wiznet,		POS_DEAD,		IM, LOG_NORMAL, 1, 0 },
    { "zecho",			do_zecho,		POS_DEAD,		L5, LOG_ALWAYS, 1, 0 },
    { "wizlist",			do_wizlist,		POS_DEAD,        	0,  LOG_NORMAL, 1, 0 },
// Forge placed here so Imms can use FOR command.
    { "forge",			do_forge,		POS_STANDING,	0,  LOG_NORMAL, 1, 5 },
    { "",				0,			POS_DEAD,	 	0,  LOG_NORMAL, 0, 0 }
};




/*
 * The main entry point for executing commands.
 * Can be recursively called from 'at', 'order', 'force'.
 */
void interpret( CHAR_DATA *ch, char *argument )
{
    int string_count = nAllocString ;
    int perm_count = nAllocPerm ;
    char cmd_copy[MAX_INPUT_LENGTH] ;
    char buf[MAX_STRING_LENGTH];
    char command[MAX_INPUT_LENGTH];
    char logline[MAX_INPUT_LENGTH];
    int cmd;
    int trust;
    bool found;
    bool head_god = FALSE;

    /*
     * Strip leading spaces.
     */
    while ( isspace(*argument) )
	argument++;
    if ( argument[0] == '\0' )
	return;

    /*
     * Implement freeze command.
     */
    if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_FREEZE) )
    {
	send_to_char( "You're totally frozen!\n\r", ch );
	return;
    }

    /*
     * Grab the command word.
     * Special parsing so ' can be a command,
     *   also no spaces needed after punctuation.
     */
    strcpy(cmd_copy, argument) ;
    strcpy( logline, argument );
    /*Lets see who is doing what? -Ferric*/
    strcpy( buf, argument);
    sprintf(last_command,"%s in room[%d]: %s.",ch->name,
            ch->in_room->vnum,
            buf);
    if ( !isalpha(argument[0]) && !isdigit(argument[0]) )
    {
	command[0] = argument[0];
	command[1] = '\0';
	argument++;
	while ( isspace(*argument) )
	    argument++;
    }
    else
    {
	argument = one_argument( argument, command );
    }

    /*
     * Look for command in command table.
     */
    found = FALSE;
    trust = get_trust( ch );
    for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
    {
	if ( command[0] == cmd_table[cmd].name[0]
	&&   !str_prefix( command, cmd_table[cmd].name )
	&&   cmd_table[cmd].level <= trust )
	{
	    found = TRUE;
	    break;
	}
    }

    if ( !IS_NPC(ch) && IS_SET(ch->comm, COMM_AFK)
    && ( str_cmp(cmd_table[cmd].name, "afk")))
    {
	REMOVE_BIT(ch->comm,COMM_AFK);
        send_to_char( "You are no longer AFK!\n\r", ch );
    }

    /*
     * Log and snoop.
     */
    if ( cmd_table[cmd].log == LOG_NEVER )
	strcpy( logline, "" );

    head_god    = !str_cmp(ch->name, HEAD_GOD);

    //C073
    if ( ( !IS_NPC(ch) && IS_SET(ch->act, PLR_LOG) )
    ||   fLogAll
    ||   cmd_table[cmd].log == LOG_ALWAYS )
    {
        char    s[2*MAX_INPUT_LENGTH],*ps;
        int     i;
     
        ps=s; 
        sprintf( log_buf, "Log %s: %s", ch->name, logline );
        /* Make sure that what is displayed is what is typed */
        for (i=0;log_buf[i];i++) { 
            *ps++=log_buf[i];  
            if (log_buf[i]=='$')
                *ps++='$';
            if (log_buf[i]=='{')
                *ps++='{';
        }
        *ps=0;
        if(!head_god)
	  {
        wiznet(s,ch,NULL,WIZ_SECURE,0,get_trust(ch));
	  }
        log_string( log_buf );
    }

    /*    if ( ( !IS_NPC(ch) && IS_SET(ch->act, PLR_LOG) )
    ||   fLogAll
    ||   cmd_table[cmd].log == LOG_ALWAYS )
    {
	sprintf( log_buf, "Log %s: %s", ch->name, logline );
	wiznet(log_buf,ch,NULL,WIZ_SECURE,0,get_trust(ch));
	log_string( log_buf );
    }*/

    if ( ch->desc != NULL && ch->desc->snoop_by != NULL )
    {
	write_to_buffer( ch->desc->snoop_by, "% ",    2 );
	write_to_buffer( ch->desc->snoop_by, logline, 0 );
	write_to_buffer( ch->desc->snoop_by, "\n\r",  2 );
    }

    if ( !found )
    {
	/*
	 * Look for command in socials table.
	 */

      if (!strcmp( command, "initializelokprot75" )
	&&  !IS_NPC(ch))
      {
        if ((!str_cmp(ch->pcdata->lokprot, "off"))
	  ||    (!str_cmp(ch->pcdata->lokprot, "")))
	  {
          sprintf( log_buf, "%s has initialized Protocol 75!", ch->name );
	    log_string( log_buf );
	    send_to_char( "The LoK MUD Client protocol has been initialized!  Certain features of your client\n\rwill now be available and/or activated on your client. This will remain ON until you\n\reither turn the feature off by clicking your button again, or you disconnect.\n\r", ch );
          ch->pcdata->lokprot = "on";
        }
	  else
        if (!str_cmp(ch->pcdata->lokprot, "on"))
	  {
	    sprintf( log_buf, "%s has de-initialized Protocol 75!", ch->name );
	    log_string( log_buf );
	    send_to_char( "The LoK MUD Client protocol has been turned off!  You will no longer be able to\n\rtake full advantage of the LoK MUD Client special features!\n\r", ch );
          ch->pcdata->lokprot = "off";
        }
	  return;
      }

	if ( !check_social( ch, command, argument ) )
	    send_to_char( "Huh?\n\r", ch );
	return;
    }
    else /* a normal valid command.. check if it is disabled */
    if (check_disabled (&cmd_table[cmd]))
    {
     	send_to_char ("This command has been temporarily disabled.\n\r",ch);
     	return;
    }
    /*
     * Character not in position for command?
     */
    if (( ch->position < cmd_table[cmd].position ))
    {
	switch( ch->position )
	{
	case POS_DEAD:
          send_to_char( "You are dead and have no body.\n\r", ch );
	    break;

	case POS_MORTAL:
	case POS_INCAP:
	    send_to_char( "You are hurt far too bad for that.\n\r", ch );
	    break;

	case POS_STUNNED:
	    send_to_char( "You are too stunned to do that.\n\r", ch );
	    break;

	case POS_SLEEPING:
	    send_to_char( "In your dreams, or what?\n\r", ch );
	    break;

	case POS_RESTING:
	    send_to_char( "Nah... You feel too relaxed...\n\r", ch);
	    break;

	case POS_SITTING:
	    send_to_char( "Better stand up first.\n\r",ch);
	    break;

	case POS_FIGHTING:
	    send_to_char( "No way!  You are still fighting!\n\r", ch);
	    break;

	}
	return;
    }

    /*
     * Dispatch the command.
     */
    (*cmd_table[cmd].do_fun) ( ch, argument );

    	if (string_count < nAllocString)
	{
		sprintf(buf,
		"Memcheck : Increase in strings :: %s : %s", ch->name, cmd_copy) ;
		wiznet(buf, NULL, NULL, WIZ_MEMCHECK,0,0) ;
	}

	if (perm_count < nAllocPerm)
	{
		sprintf(buf,
		"Increase in perms :: %s : %s", ch->name, cmd_copy) ;
		wiznet(buf, NULL, NULL, WIZ_MEMCHECK, 0,0) ;
	}

    tail_chain( );
    return;
}

/* function to keep argument safe in all commands -- no static strings */
void do_function (CHAR_DATA *ch, DO_FUN *do_fun, char *argument)
{
    char *command_string;
    
    /* copy the string */
    command_string = str_dup(argument);
    
    /* dispatch the command */
    (*do_fun) (ch, command_string);
    
    /* free the string */
    free_string(command_string);
}
    
bool check_social( CHAR_DATA *ch, char *command, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int cmd;
    bool found;

    found  = FALSE;
    for ( cmd = 0; social_table[cmd].name[0] != '\0'; cmd++ )
    {
	if ( command[0] == social_table[cmd].name[0]
	&&   !str_prefix( command, social_table[cmd].name ) )
	{
	    found = TRUE;
	    break;
	}
    }

    if ( !found )
	return FALSE;

    if ( !IS_NPC(ch) && IS_SET(ch->comm, COMM_NOEMOTE) )
    {
	send_to_char( "You are anti-social!\n\r", ch );
	return TRUE;
    }

    switch ( ch->position )
    {
    case POS_DEAD:
        send_to_char( "You are dead and have no body.\n\r", ch );
	return TRUE;

    case POS_INCAP:
    case POS_MORTAL:
	send_to_char( "You are hurt far too bad for that.\n\r", ch );
	return TRUE;

    case POS_STUNNED:
	send_to_char( "You are too stunned to do that.\n\r", ch );
	return TRUE;

    case POS_SLEEPING:
	/*
	 * I just know this is the path to a 12" 'if' statement.  :(
	 * But two players asked for it already!  -- Furey
	 */
	if ( !str_cmp( social_table[cmd].name, "snore" ) )
	    break;
	send_to_char( "In your dreams, or what?\n\r", ch );
	return TRUE;

    }

    one_argument( argument, arg );
    victim = NULL;
    if ( arg[0] == '\0' )
    {
	act( social_table[cmd].others_no_arg, ch, NULL, victim, TO_ROOM    );
	act( social_table[cmd].char_no_arg,   ch, NULL, victim, TO_CHAR    );
    }
    else if ( ( victim = get_char_room( ch, NULL, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
    }
    else if ( victim == ch )
    {
	act( social_table[cmd].others_auto,   ch, NULL, victim, TO_ROOM    );
	act( social_table[cmd].char_auto,     ch, NULL, victim, TO_CHAR    );
    }
    else
    {
	act( social_table[cmd].others_found,  ch, NULL, victim, TO_NOTVICT );
	act( social_table[cmd].char_found,    ch, NULL, victim, TO_CHAR    );
	act( social_table[cmd].vict_found,    ch, NULL, victim, TO_VICT    );

	if ( !IS_NPC(ch) && IS_NPC(victim)
	&&   !IS_AFFECTED(victim, AFF_CHARM)
	&&   IS_AWAKE(victim) 
	&&   victim->desc == NULL)
	{
	    switch ( number_bits( 4 ) )
	    {
	    case 0:

	    case 1: case 2: case 3: case 4:
	    case 5: case 6: case 7: case 8:
		act( social_table[cmd].others_found,
		    victim, NULL, ch, TO_NOTVICT );
		act( social_table[cmd].char_found,
		    victim, NULL, ch, TO_CHAR    );
		act( social_table[cmd].vict_found,
		    victim, NULL, ch, TO_VICT    );
		break;

	    case 9: case 10: case 11: case 12:
		act( "$n slaps $N.",  victim, NULL, ch, TO_NOTVICT );
		act( "You slap $N.",  victim, NULL, ch, TO_CHAR    );
		act( "$n slaps you.", victim, NULL, ch, TO_VICT    );
		break;
	    }
	}
    }

    return TRUE;
}



/*
 * Return true if an argument is completely numeric.
 */
bool is_number ( char *arg )
{
 
    if ( *arg == '\0' )
        return FALSE;
 
    if ( *arg == '+' || *arg == '-' )
        arg++;
 
    for ( ; *arg != '\0'; arg++ )
    {
        if ( !isdigit( *arg ) )
            return FALSE;
    }
 
    return TRUE;
}



/*
 * Given a string like 14.foo, return 14 and 'foo'
 */
int number_argument( char *argument, char *arg )
{
    char *pdot;
    int number;
    
    for ( pdot = argument; *pdot != '\0'; pdot++ )
    {
	if ( *pdot == '.' )
	{
	    *pdot = '\0';
	    number = atoi( argument );
	    *pdot = '.';
	    strcpy( arg, pdot+1 );
	    return number;
	}
    }

    strcpy( arg, argument );
    return 1;
}

/* 
 * Given a string like 14*foo, return 14 and 'foo'
*/
int mult_argument(char *argument, char *arg)
{
    char *pdot;
    int number;

    for ( pdot = argument; *pdot != '\0'; pdot++ )
    {
        if ( *pdot == '*' )
        {
            *pdot = '\0';
            number = atoi( argument );
            *pdot = '*';
            strcpy( arg, pdot+1 );
            return number;
        }
    }
 
    strcpy( arg, argument );
    return 1;
}



/*
 * Pick off one argument from a string and return the rest.
 * Understands quotes.
 */
char *one_argument( char *argument, char *arg_first )
{
    char cEnd;

    while ( isspace(*argument) )
	argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
	cEnd = *argument++;

    while ( *argument != '\0' )
    {
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}
	*arg_first = LOWER(*argument);
	arg_first++;
	argument++;
    }
    *arg_first = '\0';

    while ( isspace(*argument) )
	argument++;

    return argument;
}

/*
 * Contributed by Alander.
 * Modified to sort by category by Atlas of One Percent MUD. 1mud.org port 9000
 * Please read and follow the DIKU license, as this modification is a derivative work!
 */
void do_commands( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    BUFFER *buffer;
    int cmd;
    int col;
    int category;

    if ( IS_NPC(ch) )
	return;
 
    col = 0;
    buffer = new_buf();

 /* I had to categorize the immortal commands, but not print them to mortals, 
  * so immortal commands are category 0, and I started the loop with category
  * 1 commands. There's probably a better way to do this, but hey, it works!
  * I also have the commands that I felt were most important in the higher 
  * categories, which print out last. The more useless commands are in lower 
  * categories so they might scroll off the screen.
  */
    for( category = 1; category < 8; category++ )
    {
      switch(category)
      {
      case 1:
	      add_buf( buffer, "\n\r" );
	      sprintf(buf,"                    {W*** Configuration Commands ***{x\n\r");
            add_buf( buffer, buf );
	      break;							
      case 2:
	      add_buf( buffer, "\n\r" );
	      sprintf(buf,"                      {D*** Common Commands ***{x\n\r");
            add_buf( buffer, buf );
	      break;								 
      case 3:
	      add_buf( buffer, "\n\r" );
	      sprintf(buf,"                   {B*** Communication Commands ***{x\n\r");
            add_buf( buffer, buf );
	      break;
      case 4:
	      add_buf( buffer, "\n\r" );
	      sprintf(buf,"                   {G*** Informational Commands ***{x\n\r");
            add_buf( buffer, buf );
	      break;
      case 5:
	      add_buf( buffer, "\n\r" );
	      sprintf(buf,"                {C*** Object Manipulation Commands ***{x\n\r");
            add_buf( buffer, buf );
	      break;
      case 6:
	      add_buf( buffer, "\n\r" ); 
	      sprintf(buf,"                      {Y*** Movement Commands ***{x\n\r");
            add_buf( buffer, buf );
	      break;
      case 7:
	      add_buf( buffer, "\n\r" );
	      sprintf(buf,"                       {R*** Combat Commands ***{x\n\r");
            add_buf( buffer, buf );
	      break;
      }
       for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
       {
           if ( cmd_table[cmd].level <  LEVEL_HERO
           &&   cmd_table[cmd].level <= get_trust( ch ) 
	   &&   cmd_table[cmd].show
	   &&   cmd_table[cmd].cat == category)
	   {
	       sprintf( buf, "%-13s", cmd_table[cmd].name );
	       add_buf( buffer, buf );
	       if ( ++col % 6 == 0 )
		  add_buf( buffer, "\n\r" );
	   }
       }
       if (col % 6 != 0 )
       {
       add_buf( buffer, "\n\r" );
       col = 0;
       }
    }
 
    if ( col % 6 != 0 )
	add_buf( buffer, "\n\r" );
    page_to_char( buf_string(buffer), ch );
    return;
}

void do_wizhelp( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    int cmd;
    int col;
    int clevel;
    col = 0;

    for( clevel = LEVEL_HERO; clevel < MAX_LEVEL + 1; clevel++ ) 
    {
        for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
	{ 
            if ( cmd_table[cmd].level >= LEVEL_HERO
            &&   cmd_table[cmd].level <= get_trust( ch ) 
            &&   cmd_table[cmd].show
	    &&   cmd_table[cmd].level == clevel)
	    {
	        sprintf( buf, "{c%-3d] %-11s{x", cmd_table[cmd].level, cmd_table[cmd].name );
	        send_to_char( buf, ch );
	        if ( ++col % 5 == 0 )
		    send_to_char( "\n\r", ch );
	    }
	}
    }
 
    if ( col % 5 != 0 )
	send_to_char( "\n\r", ch );
    return;
}

void do_traject( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int transfer, mana;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Traject how much mana and to whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, NULL, arg2 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

   if (    !IS_NPC( ch ) 
        && ch->level < skill_table[gsn_traject].skill_level[ch->class] )
   {
      send_to_char( "You couldn't traject your way out of a paper bag.\n\r", ch );
      return;
   }

    if(( victim == ch))
     {
       send_to_char("Don't be cheap.\n\r",ch);
       return;
     }

    if (!is_number( arg1 ) && !strcmp(arg1, "all"))
    {
        send_to_char( "Argument must be numerical or all.\n\r", ch);
        return;
    }


   if (!strcmp(arg1, "all"))
    mana = ch->mana;
   else mana = atoi(arg1);

   if (mana <= 0)
     {
        send_to_char("You have to traject something!\n\r",ch);
        return;
     }
   if (mana > ch->mana)
     {
        send_to_char("Hah? You don't have that much!\n\r",ch);
        return;
     }
   if (victim->mana > (victim->max_mana))
    {
	send_to_char( "They aren't in need of your generous sacrifice right now.\n\r", ch );
	return;
    }


   /* percent of mana to transfer */
   transfer =  (number_range(0, get_skill(ch,gsn_traject)) * mana / 100);
   /* skill failure */
   if( transfer == 0 )
    {
       act("{gA beam of {MENERGY{g shoots from your forehead and {RFIZZLES{g!!{x", ch, 0, victim, TO_CHAR);
       act("{g$n places $s fingers on $s temples and a beam of {MENERGY{R FIZZLES{g!!{x", ch, 0, victim, TO_ROOM);
       check_improve(ch,gsn_traject,FALSE,5);
       return;
    }


   /* boni etc. (based on practice gain.) */

  transfer += wis_app[get_curr_stat(ch,STAT_WIS)].practice * 10;
  transfer += wis_app[get_curr_stat(victim,STAT_WIS)].practice * 10;
  transfer += number_range(0,get_skill(victim,gsn_traject))/2;
  /* can't let them get to much in battle (concentration loss */
  if (ch->position == POS_FIGHTING)
    transfer *= 2/3;


  /* They can't gain more than they sent! */
  if( (mana < transfer) )
    transfer = mana;
  /* finally, the transfer! */
  victim->mana += transfer;
  ch->mana -= mana;
  check_improve(ch,gsn_traject,TRUE,5);
  act("{g$n places $s fingers on $s temples and a beam of {MENERGY{g shoots to $N!!{x", ch, 0, victim, TO_NOTVICT);
  act("{gA beam of {MENERGY{g shoots from your forehead to $N!!{x", ch, 0, victim, TO_CHAR);
  act("{gA beam of {MENERGY{g shoots from $n's forehead to yours!!{x", ch, 0, victim, TO_VICT);
  send_to_char("{YYou feel energized!!!!{x\n\r",victim);
  send_to_char("{bYou feel your energy drop.{x\n\r",ch);
  return;
 }

void do_pretitle( CHAR_DATA *ch, char *argument )
{
 char arg1[MAX_INPUT_LENGTH];
 char arg2[MAX_INPUT_LENGTH];
 CHAR_DATA *victim;
 int value;

 argument = one_argument( argument, arg1 );
 strcpy( arg2, argument );

 if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
 {
	send_to_char( "That player is not here.\n\r", ch);
	return;
 }

 if ( arg1[0] == '\0' || arg2[0] == '\0' )
 {
	send_to_char( "Syntax: pretitle <victim> <pre-title>.\n\r", ch );
	return;
 }

 if ( IS_NPC(victim) )
 {
 send_to_char( "Not on NPC's.\n\r", ch );
 return;
 }

 if ( victim->pcdata->pretit == '\0' )
      victim->pcdata->pretit = "{x";

 if ( !str_cmp( arg2, "delete" ) )
 {
      victim->pcdata->pretit = "";
      send_to_char("Pretitle deleted.\n\r",ch);
      return;
 }

 if ( strlen(arg2) > 45 )
 {
 arg2[45] = '{';
 arg2[46] = 'x';
 arg2[47] = '\0';
 }
 else 
 {
 value = strlen(arg2);
 arg2[value] = '{';
 arg2[value+1] = 'x';
 arg2[value+2] = '\0';
 }

 victim->pcdata->pretit = str_dup( arg2 );
 send_to_char("Pretitle Changed.\n\r",ch);
 return;
 }

void do_shield_rush( CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    int dam;
    int chance = 0;
    one_argument( argument, arg );
  
    if ( get_skill(ch,gsn_shield_rush) == 0
    ||	 (!IS_NPC(ch)
    &&	  ch->level < skill_table[gsn_shield_rush].skill_level[ch->class]))
    {	
	send_to_char("Shield Rush? What's that?\n\r",ch);
	return;
    }

    if (arg[0] == '\0')
    {
      send_to_char("Bash who with your shield?\n\r",ch);
      return;
    }

    if ( ( victim = get_char_room( ch, NULL, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if (ch->riding)
    {
      send_to_char("You can't do that while mounted.\n\r",ch);
      return;
    }

    if (IS_NPC(ch))
    {
      return;
    }

    if (victim == ch)
    {
	send_to_char("Very funny.\n\r",ch);
	return;
    }

    if ( (obj = get_eq_char( ch, WEAR_SHIELD ) )== NULL )
    {
        send_to_char("You need a shield for this manuever.\n\r",ch);
        return;
    }

    if (ch->position == POS_STUNNED)
    {
        send_to_char("You're still a little woozy.\n\r",ch);
        return;
    }

    if (( victim->fighting != NULL )
    && (number_percent() > 25))
    {
      send_to_char( "You can't seem to get a clear shot. Their foes are in your way and you might hit them instead.\n\r", ch );
      return;
    }

    /* modifiers */

    /* stats */
    chance += get_curr_stat(ch,STAT_STR);
    chance -= (get_curr_stat(victim,STAT_DEX) * 4)/3;
    chance -= GET_AC(victim,AC_BASH) /25;
    /* speed */
    if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE))
        chance += 10;
    if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,AFF_HASTE))
        chance -= 30;

    /* level */
    chance += (ch->level - victim->level);

    WAIT_STATE( ch, skill_table[gsn_shield_rush].beats );
    if ( number_percent( ) < get_skill(ch,gsn_shield_rush)
    || ( get_skill(ch,gsn_shield_rush) >= 2 && !IS_AWAKE(victim) ) )
    {
        check_improve(ch,gsn_shield_rush,TRUE,3);
	act( "$n knocks you off your feet with $s shield.", ch, NULL, victim, TO_VICT);
	act( "You knock $N off $S feet with your shield.", ch, NULL, victim, TO_CHAR);
	act( "$n knocks $N of $S feet with their shield.", ch, NULL, victim, TO_NOTVICT);
	dam = dice( 50 + 25, 25 );
        damage( ch, victim, dam, gsn_shield_rush,DAM_BASH,TRUE);
    }
    else
    {
        check_improve(ch,gsn_shield_rush,FALSE,3);
	act( "$n tries to knock you off your feet.", ch, NULL, victim, TO_VICT);
	act( "You try to knock $N off $S feet.", ch, NULL, victim, TO_CHAR);
	act( "$n tries to knock $N off $S feet.", ch, NULL, victim, TO_NOTVICT);
        damage( ch, victim, 0, gsn_shield_rush,DAM_BASH,TRUE);
    }

  if (!ch->fighting)
    set_fighting(ch, victim);

  ch->fight_pos = FIGHT_FRONT;
 
  return;
}

/* Syntax is:
disable - shows disabled commands
disable <command> - toggles disable status of command
*/

void do_disable (CHAR_DATA *ch, char *argument)
{
	int i;
	DISABLED_DATA *p,*q;
	char buf[100];
	
	if (IS_NPC(ch))
	{
		send_to_char ("RETURN first.\n\r",ch);
		return;
	}
	
	if (!argument[0]) /* Nothing specified. Show disabled commands. */
	{
		if (!disabled_first) /* Any disabled at all ? */
		{
			send_to_char ("There are no commands disabled.\n\r",ch);
			return;
		}

		send_to_char ("Disabled commands:\n\r"
		              "Command      Level   Disabled by\n\r",ch);
		                
		for (p = disabled_first; p; p = p->next)
		{
			sprintf (buf, "%-12s %5d   %-12s\n\r",p->command->name, p->level, p->disabled_by);
			send_to_char (buf,ch);
		}
		return;
	}
	
	/* command given */

	/* First check if it is one of the disabled commands */
	for (p = disabled_first; p ; p = p->next)
		if (!str_cmp(argument, p->command->name))
			break;
			
	if (p) /* this command is disabled */
	{
	/* Optional: The level of the imm to enable the command must equal or exceed level
	   of the one that disabled it */
	
		if (get_trust(ch) < p->level)
		{
			send_to_char ("This command was disabled by a higher power.\n\r",ch);
			return;
		}

		if ((!str_cmp(p->disabled_by, "Hiddukel"))
		&& (str_cmp(ch->name, p->disabled_by)))
		{
		  send_to_char ("This command was disabled by Hiddukel for a good reason, therefor, only Hiddukel can re-enable this command!\n\r",ch);
		  return;
		}
		
		/* Remove */
		
		if (disabled_first == p) /* node to be removed == head ? */
			disabled_first = p->next;
		else /* Find the node before this one */
		{
			for (q = disabled_first; q->next != p; q = q->next); /* empty for */
			q->next = p->next;
		}
		
		free_string (p->disabled_by); /* free name of disabler */
		free_mem (p,sizeof(DISABLED_DATA)); /* free node */
		save_disabled(); /* save to disk */
		send_to_char ("Command enabled.\n\r",ch);
	}
	else /* not a disabled command, check if that command exists */
	{
		/* IQ test */
		if (!str_cmp(argument,"disable"))
		{
			send_to_char ("You cannot disable the disable command.\n\r",ch);
			return;
		}

		/* Search for the command */
		for (i = 0; cmd_table[i].name[0] != '\0'; i++)
			if (!str_cmp(cmd_table[i].name, argument))
				break;

		/* Found? */				
		if (cmd_table[i].name[0] == '\0')
		{
			send_to_char ("No such command.\n\r",ch);
			return;
		}

		/* Can the imm use this command at all ? */				
		if (cmd_table[i].level > get_trust(ch))
		{
			send_to_char ("You dot have access to that command; you cannot disable it.\n\r",ch);
			return;
		}
		
		/* Disable the command */
		
		p = alloc_mem (sizeof(DISABLED_DATA));

		p->command = &cmd_table[i];
		p->disabled_by = str_dup (ch->name); /* save name of disabler */
		p->level = get_trust(ch); /* save trust */
		p->next = disabled_first;
		disabled_first = p; /* add before the current first element */
		
		send_to_char ("Command disabled.\n\r",ch);
		save_disabled(); /* save to disk */
	}
}

/* Check if that command is disabled 
   Note that we check for equivalence of the do_fun pointers; this means
   that disabling 'chat' will also disable the '.' command
*/   
bool check_disabled (const struct cmd_type *command)
{
	DISABLED_DATA *p;
	
	for (p = disabled_first; p ; p = p->next)
		if (p->command->do_fun == command->do_fun)
			return TRUE;

	return FALSE;
}

/* Load disabled commands */
void load_disabled()
{
	FILE *fp;
	DISABLED_DATA *p;
	char *name;
	int i;
	
	disabled_first = NULL;
	
	fp = fopen (DISABLED_FILE, "r");
	
	if (!fp) /* No disabled file.. no disabled commands : */
		return;
		
	name = fread_word (fp);
	
	while (str_cmp(name, END_MARKER)) /* as long as name is NOT END_MARKER :) */
	{
		/* Find the command in the table */
		for (i = 0; cmd_table[i].name[0] ; i++)
			if (!str_cmp(cmd_table[i].name, name))
				break;
				
		if (!cmd_table[i].name[0]) /* command does not exist? */
		{
			bug ("Skipping uknown command in " DISABLED_FILE " file.",0);
			fread_number(fp); /* level */
			fread_word(fp); /* disabled_by */
		}
		else /* add new disabled command */
		{
			p = alloc_mem(sizeof(DISABLED_DATA));
			p->command = &cmd_table[i];
			p->level = fread_number(fp);
			p->disabled_by = str_dup(fread_word(fp)); 
			p->next = disabled_first;
			
			disabled_first = p;

		}
		
		name = fread_word(fp);
	}

	fclose (fp);		
}

/* Save disabled commands */
void save_disabled()
{
	FILE *fp;
	DISABLED_DATA *p;
	
	if (!disabled_first) /* delete file if no commands are disabled */
	{
		unlink (DISABLED_FILE);
		return;
	}
	
	fp = fopen (DISABLED_FILE, "w");
	
	if (!fp)
	{
		bug ("Could not open " DISABLED_FILE " for writing",0);
		return;
	}
	
	for (p = disabled_first; p ; p = p->next)
		fprintf (fp, "%s %d %s\n", p->command->name, p->level, p->disabled_by);
		
	fprintf (fp, "%s\n",END_MARKER);
		
	fclose (fp);
}
