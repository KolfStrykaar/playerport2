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
#include <stdio.h>
#include <time.h>
#include "merc.h"
#include "magic.h"
#include "interp.h"


/* item type list */
const struct item_type		item_table	[]	=
{
    {	ITEM_LIGHT,		"light"		},
    {	ITEM_SCROLL,	"scroll"		},
    {	ITEM_WAND,		"wand"		},
    { ITEM_STAFF,		"staff"		},
    { ITEM_WEAPON,	"weapon"		},
    { ITEM_TREASURE,	"treasure"		},
    { ITEM_ARMOR,		"armor"		},
    {	ITEM_POTION,	"potion"		},
    {	ITEM_CLOTHING,	"clothing"		},
    { ITEM_FURNITURE,	"furniture"		},
    {	ITEM_TRASH,		"trash"		},
    {	ITEM_CONTAINER,	"container"		},
    {	ITEM_DRINK_CON, 	"drink"		},
    {	ITEM_KEY,		"key"			},
    {	ITEM_FOOD,		"food"		},
    {	ITEM_MONEY,		"money"		},
    {	ITEM_BOAT,		"boat"		},
    {	ITEM_CORPSE_NPC,	"npc_corpse"	},
    {	ITEM_CORPSE_PC,	"pc_corpse"		},
    { ITEM_FOUNTAIN,	"fountain"		},
    {	ITEM_PILL,		"pill"		},
    {	ITEM_PROTECT,	"protect"		},
    {	ITEM_MAP,		"map"			},
    {	ITEM_PORTAL,	"portal"		},
    {	ITEM_WARP_STONE,	"warp_stone"	},
    {	ITEM_ROOM_KEY,	"room_key"		},
    {	ITEM_GEM,		"gem"			},
    {	ITEM_JEWELRY,	"jewelry"		},
    { ITEM_JUKEBOX,	"jukebox"		},
    {	ITEM_TATTOO,	"tattoo"		},
    { ITEM_WET_STONE,	"wetstone"		},
    {	ITEM_INSTRUMENT,	"instrument"	},
    {	ITEM_STONE,		"stone"		},
    {	ITEM_QUIVER,	"quiver"		},
    {	ITEM_ARROW,		"arrow"		},
    { ITEM_BHAMMER,	"bhammer"		},
    { ITEM_ANVIL,		"anvil"		},
    { ITEM_RAW_MATERIAL, "raw_material"	},
    { ITEM_TOOL_KIT,	"tool_kit"		},
    { ITEM_BOOBYTRAP,	"boobytrap"		},
    { ITEM_TRAP,		"trap",		},
    { ITEM_FURNACE,	"furnace"		},
    { ITEM_BARREL,	"barrel"		},
    { ITEM_GRINDER,	"grinder"		},
    {   0,		NULL		}
};

/*
 * Traps
 */
const	struct	trap_type	trap_table	[]	=
{
    /* name */
    { "explosive", },		/* 0 */
    { "snare", },
    { "fireseed", },
    { "alarm", },
    { "poison", },
    { "arrow", },			/* 5 */
    { "net", },
    { "flooding", },
    { "portcullis", },
    { "crushing", },
    { "pit", },			/* 10 */
    { "smoke", },
    { NULL }
};

const struct align_range_type align_change_table[3][3] =
 {
 /* CH */
 /* VICTIM */
 /* GOOD */
 /* NEUT */
 /* EVIL */
 /* GOOD */
   {
	 { -3, -250 },
	 { 1, -1 },
	 { 3, 2 },
   },
 
/* NEUT */
   {
	 { -2, -3 },
	 { 1, -2 },
	 { 3, 2 },
   },

 /* EVIL */
   {
	 { -1, -2 },
	 { 1, -1 },
	 { 2, 1 },
   }
 };

/* weapon selection table */
const	struct	weapon_type	weapon_table	[]	=
{
   { "sword",	OBJ_VNUM_SCHOOL_SWORD,	WEAPON_SWORD,	&gsn_sword		},
   { "mace",	OBJ_VNUM_SCHOOL_MACE,	WEAPON_MACE,	&gsn_mace 		},
   { "dagger",	OBJ_VNUM_SCHOOL_DAGGER,	WEAPON_DAGGER,	&gsn_dagger		},
   { "axe",		OBJ_VNUM_SCHOOL_AXE,	WEAPON_AXE,		&gsn_axe		},
   { "spear",	OBJ_VNUM_SCHOOL_SPEAR,	WEAPON_SPEAR,	&gsn_spear		},
   { "flail",	OBJ_VNUM_SCHOOL_FLAIL,	WEAPON_FLAIL,	&gsn_flail		},
   { "whip",	OBJ_VNUM_SCHOOL_WHIP,	WEAPON_WHIP,	&gsn_whip		},
   { "polearm",	OBJ_VNUM_SCHOOL_POLEARM,WEAPON_POLEARM,	&gsn_polearm	},
   { "bow",		OBJ_VNUM_SCHOOL_BOW,	WEAPON_BOW,		&gsn_bow		},
   { "staff",     OBJ_VNUM_SCHOOL_STAFF,	WEAPON_STAFF,	&gsn_staff		},
   { NULL,	0,				0,	NULL		}
};


 
/* wiznet table and prototype for future flag setting */
const   struct wiznet_type      wiznet_table    []              =
{
   {  "on",           	WIZ_ON,         	IM },
   {  "prefix",		WIZ_PREFIX,		IM },
   {  "ticks",        	WIZ_TICKS,      	IM },
   {  "logins",      	WIZ_LOGINS,     	IM },
   {  "sites",       	WIZ_SITES,      	L4 },
   {  "links",        	WIZ_LINKS,      	L7 },
   {	"newbies",		WIZ_NEWBIE,		IM },
   {	"spam",		WIZ_SPAM,		L5 },
   {  "deaths",       	WIZ_DEATHS,     	IM },
   {  "resets",       	WIZ_RESETS,     	L4 },
   {  "mobdeaths",    	WIZ_MOBDEATHS,  	L4 },
   {  "flags",		WIZ_FLAGS,		L5 },
   {	"penalties",	WIZ_PENALTIES,	L5 },
   {	"saccing",		WIZ_SACCING,	L5 },
   {	"levels",		WIZ_LEVELS,		IM },
   {	"load",		WIZ_LOAD,		L2 },
   {	"restore",		WIZ_RESTORE,	L2 },
   {	"snoops",		WIZ_SNOOPS,		L2 },
   {	"switches",		WIZ_SWITCHES,	L2 },
   {  "clan",	    	WIZ_CLAN,    	L5 },
   {  "memcheck",		WIZ_MEMCHECK,	L1 },
   {	"secure",		WIZ_SECURE,		L1 },
   {	"multi",		WIZ_MULTI,		L1 },
   {	"subdues",		WIZ_SUBDUES,	IM },
   {	NULL,			0,		0  }
};

/* attack table  -- not very organized :( */
const 	struct attack_type	attack_table	[MAX_DAMAGE_MESSAGE]	=
{
    {   "none",		"hit",		-1			},  /*  0 */
    {	  "slice",		"slice", 		DAM_SLASH		},	
    {   "stab",		"stab",		DAM_PIERCE		},
    {	  "slash",		"slash",		DAM_SLASH		},
    {	  "whip",		"whip",		DAM_SLASH		},
    {   "claw",		"claw",		DAM_SLASH		},  /*  5 */
    {	  "blast",		"blast",		DAM_BASH		},
    {   "pound",		"pound",		DAM_BASH		},
    {	  "crush",		"crush",		DAM_BASH		},
    {   "grep",		"grep",		DAM_SLASH		},
    {	  "bite",		"bite",		DAM_PIERCE		},  /* 10 */
    {   "pierce",		"pierce",		DAM_PIERCE		},
    {   "suction",	"suction",		DAM_BASH		},
    {	  "beating",	"beating",		DAM_BASH		},
    {   "digestion",	"digestion",	DAM_ACID		},
    {	  "charge",		"charge",		DAM_BASH		},  /* 15 */
    {   "slap",		"slap",		DAM_BASH		},
    {	  "punch",		"punch",		DAM_BASH		},
    {	  "wrath",		"wrath",		DAM_ENERGY		},
    {	  "magic",		"magic",		DAM_ENERGY		},
    {   "divine",		"divine power",	DAM_HOLY		},  /* 20 */
    {	  "cleave",		"cleave",		DAM_SLASH		},
    {	  "scratch",	"scratch",		DAM_PIERCE		},
    {   "peck",		"peck",		DAM_PIERCE		},
    {   "peckb",		"peck",		DAM_BASH		},
    {   "chop",		"chop",		DAM_SLASH		},  /* 25 */
    {   "sting",		"sting",		DAM_PIERCE		},
    {   "smash",		"smash",		DAM_BASH		},
    {   "shbite",		"shocking bite",	DAM_LIGHTNING	},
    {	  "flbite",		"flaming bite", 	DAM_FIRE		},
    {	  "frbite",		"freezing bite", 	DAM_COLD		},  /* 30 */
    {	  "acbite",		"acidic bite", 	DAM_ACID		},
    {	  "chomp",		"chomp",		DAM_PIERCE		},
    {   "drain",		"life drain",	DAM_NEGATIVE	},
    {   "thrust",		"thrust",		DAM_PIERCE		},
    {   "slime",		"slime",		DAM_ACID		},
    {	  "shock",		"shock",		DAM_LIGHTNING	},
    {   "thwack",		"thwack",		DAM_BASH		},
    {   "flame",		"flame",		DAM_FIRE		},
    {   "chill",		"chill",		DAM_COLD		},
    {	  "tail",	 	"thrash",	  	DAM_SLASH	 	},
    {   NULL,		NULL,		0		}
};

/* race table */
const 	struct	race_type	race_table	[]		=
{
/*
    {
	name,		desc,		pc_race?,
	act bits,	aff_by bits,	off bits,
	imm,		res,		vuln,
	form,		parts,	says
	Important to make sure you know the difference between aff and aff2
    },
*/
    { "none",	"",	FALSE, 0, 0, 0, 0, 0, 0, 0, 0, "says", }, //C072

    { 
	"human",	"Human       - Average creatures of diplomacy, ethics, and adventure",	TRUE, 
	0,		0, 		0,
	0, 		0,		0,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K, "says"
    },

   {
      "draconian",   "Draconian   - Evil lizardmen, spawned from corrupted Dragon eggs",    TRUE,
      0,             AFF_PROTECT_GOOD|AFF_REGENERATION,      0,
      IMM_MENTAL,             RES_NEGATIVE|RES_BASH,         0,
      A|H|M|V,                  A|B|C|D|E|F|G|H|J|K|P|Q|U|V|W|X, "gutturals",
   },

    {
      "half-elf",    "Half-elf    - Half breeds with both Human and Elven traits",   TRUE,
      0,             AFF_INFRARED,	    	0,
      0,             RES_CHARM,         	0,
      A|H|M|V,                  A|B|C|D|E|F|G|H|I|J|K,	"says"
    },

    {
       "silvanesti",   "Silvanesti  - The 'purest' and most traditional of all elven races",     TRUE,
       0,            AFF_INFRARED,      	0,
       0,            RES_CHARM,         	0,
       A|H|M|V,                 A|B|C|D|E|F|G|H|I|J|K,	"chants"
    },

    {
      "qualinesti",     "Qualinesti  - The more adventurous, 'high' elves of Krynn",     TRUE,
      0,             AFF_INFRARED,      	0,
      0,             RES_CHARM,         	0,
      A|H|M|V,                  A|B|C|D|E|F|G|H|I|J|K,	"sings"
    },

    {
      "kagonesti",     "Kagonesti   - Wilder elves who live in the large forests of Krynn",     TRUE,
       0,            AFF_INFRARED,      	0,
       0,            RES_COLD,          	0,
       A|H|M|V,                 A|B|C|D|E|F|G|H|I|J|K,	"warbles"

    },

    { "dargonesti",   "Dargonesti  - Civilized sea elves living off the eastern coasts of Ansalon",      TRUE,
       0,            AFF_INFRARED|AFF_WATERBREATHING,		0,
       0,            RES_CHARM,         	0,
       A|H|M|V,                 A|B|C|D|E|F|G|H|I|J|K,	"gargles"

    },

    { "dimernesti",     "Dimernesti  - The 'high' sea elves, less civilized than their cousins",     TRUE,
       0,            AFF_INFRARED|AFF_WATERBREATHING,		0,
       0,            RES_CHARM,         	0,
       A|H|M|V,                 A|B|C|D|E|F|G|H|I|J|K,	"garbles"

    },

    {
	"neidar",	  "Neidar      - Dwarves who live outside of caves and closer to Humans and Elves",      TRUE,
	 0,           AFF_INFRARED,       	0,
	 0,	            0,              	VULN_COLD|VULN_DROWNING,
       A|H|M|V,                 A|B|C|D|E|F|G|H|I|J|K,	"mutters" 
    },
    
    { 
      "hylar",    "Hylar       - The most noble, as well as the largest clan of all Dwarves",     TRUE,
	 0,           AFF_INFRARED,       	0,
	 0,	            0,              	VULN_COLD|VULN_DROWNING,
       A|H|M|V,                 A|B|C|D|E|F|G|H|I|J|K,	"grumbles"
    },

    {
      "theiwar",     "Theiwar     - Conniving and scheming dark dwarves with evil self interests",   TRUE,
	 0,           AFF_INFRARED,       	0,
	 0,	            0,        		VULN_COLD|VULN_DROWNING|VULN_LIGHT,
       A|H|M|V,                 A|B|C|D|E|F|G|H|I|J|K,	"grates"
    },

    {
      "aghar",    "Aghar       - Known as 'Gully Dwarves', who inherited nothing but bad genes",       TRUE,
	 0,           AFF_INFRARED,		0,
	 0,	     RES_POISON|RES_DISEASE,       VULN_COLD|VULN_DROWNING,
       A|H|M|V,                 A|B|C|D|E|F|G|H|I|J|K,	"mumbles"
    },

    {
      "daergar",    "Daergar     - Dummest and most violent of all the Dwarven races",       TRUE,
	 0,           AFF_INFRARED,		0,
	 0,	     RES_POISON|RES_DISEASE,       VULN_COLD|VULN_DROWNING|VULN_LIGHT,
       A|H|M|V,                 A|B|C|D|E|F|G|H|I|J|K,	"grumbles"
    },

    {
      "daewar",    "Daewar      - The Dwarven Craftsmen. Always looking for more wealth",       TRUE,
	 0,           AFF_INFRARED,		0,
	 0,	     RES_POISON|RES_DISEASE,       VULN_COLD|VULN_DROWNING,
       A|H|M|V,                 A|B|C|D|E|F|G|H|I|J|K,	"mumbles"
    },

    {
      "klar",    "Klar        - Performs mainly tasks of farming, mining and general construction",       TRUE,
	 0,           AFF_INFRARED,		0,
	 0,	     RES_POISON|RES_DISEASE,       VULN_COLD|VULN_DROWNING,
       A|H|M|V,                 A|B|C|D|E|F|G|H|I|J|K,	"mutters"
    },

    {
      "gnome",    "Gnome       - Highly intelligent creatures who lack in wisdom",     TRUE,
	 0,               0,               	0,
	 0,	         RES_MAGIC,           0,
       A|H|M|V,                 A|B|C|D|E|F|G|H|I|J|K,	"babbles"
    },
 
   {
     "kender",    "Kender      - Small children sized people plagued by wanderlust",     TRUE,
	 0,               0,               	0,
	 0,	            0,              VULN_POISON,
       A|H|M|V,                 A|B|C|D|E|F|G|H|I|J|K,	"chatters"
   },

   {
     "minotaur",   "Minotaur    - Half human, half bull monsters who thrive on flesh",     TRUE,
	0,                  0,            	0,
	0,	        RES_COLD|RES_FIRE,            VULN_ACID,
      A|H|M|V,                  A|B|C|D|E|F|G|H|I|J|K,	"snorts"
   },

   {
     "ogre",	"Ogre        - The biggest, tallest, fattest, and smelliest of all the races",       TRUE,
	0,                  0,			0,
	0,	        RES_COLD|RES_FIRE,            VULN_ACID,
      A|H|M|V,                  A|B|C|D|E|F|G|H|I|J|K,	"thunders"
   },

    {
	"elf",	"",		FALSE,
	0,		AFF_INFRARED,		0,
	0,		RES_CHARM,	VULN_IRON,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K,		"chants"
    },

    {
	"dwarf",	"",	FALSE,
	0,		AFF_INFRARED,		0,
	0,		RES_POISON|RES_DISEASE, VULN_DROWNING,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K,		"grunts"
    },

    {
	"giant",	"",	FALSE,
	0,		0,			0,
	0,		RES_FIRE|RES_COLD,	VULN_MENTAL|VULN_LIGHTNING,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K,		"booms"
    },

    {
	"bat",	"",		FALSE,
	0,		AFF_FLYING|AFF_DARK_VISION,	OFF_DODGE|OFF_FAST,
	0,		0,		VULN_LIGHT,
	A|G|V,		A|C|D|E|F|H|J|K|P,	"squeeks"
    },

    {
	"bear",	"",		FALSE,
	0,		0,		      	OFF_CRUSH|OFF_DISARM|OFF_BERSERK,
	0,		RES_BASH|RES_COLD,	0,
	A|G|V,		A|B|C|D|E|F|H|J|K|U|V,		"growls"
    },

    {
	"cat",	"",		FALSE,
	0,		AFF_DARK_VISION,		OFF_FAST|OFF_DODGE,
	0,		0,		0,
	A|G|V,		A|C|D|E|F|H|J|K|Q|U|V,		"meows"
    },

    {
	"centipede",	"",	FALSE,
	0,		AFF_DARK_VISION,		0,
	0,		RES_PIERCE|RES_COLD,	VULN_BASH,
 	A|B|G|O,		A|C|K,	"wriggles"
    },

    {
	"dog",	"",		FALSE,
	0,		0,		OFF_FAST,
	0,		0,		0,
	A|G|V,		A|C|D|E|F|H|J|K|U|V,	"barks"
    },

    {
	"doll",	"",		FALSE,
	0,		0,		0,
	IMM_COLD|IMM_POISON|IMM_HOLY|IMM_NEGATIVE|IMM_MENTAL|IMM_DISEASE
	|IMM_DROWNING,	RES_BASH|RES_LIGHT,
	VULN_SLASH|VULN_FIRE|VULN_ACID|VULN_LIGHTNING|VULN_ENERGY,
	E|J|M|cc,	A|B|C|G|H|K,	"says"	
    },

    { "dragon", 	"",	FALSE, 
	0, 			AFF_INFRARED|AFF_FLYING,	0,
	0,			RES_FIRE|RES_BASH|RES_CHARM, 
	VULN_PIERCE|VULN_COLD,
	A|H|Z,		A|C|D|E|F|H|J|K|P|Q|U|V|X,	"roars"
    },

    {
	"fido",	"",		FALSE,
	0,		0,		OFF_DODGE|ASSIST_RACE,
	0,		0,			VULN_MAGIC,
	A|B|G|V,	A|C|D|E|F|H|J|K|Q|V,	"growls"
    },		
   
    {
	"fox",	"",		FALSE,
	0,		AFF_DARK_VISION,		OFF_FAST|OFF_DODGE,
	0,		0,		0,
	A|G|V,		A|C|D|E|F|H|J|K|Q|V,	"yelps"
    },

    {
	"goblin",	"",	FALSE,
	0,		AFF_INFRARED,	0,
	0,		RES_DISEASE,	VULN_MAGIC,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K,		"cackles"
    },

    {
	"hobgoblin",	"",		FALSE,
	0,		AFF_INFRARED,		0,
	0,		RES_DISEASE|RES_POISON,	0,
	A|H|M|V,        A|B|C|D|E|F|G|H|I|J|K|Y,	"grumbles"
    },

    {
	"kobold",	"",	FALSE,
	0,		AFF_INFRARED,		0,
	0,		RES_POISON,	VULN_MAGIC,
	A|B|H|M|V,	A|B|C|D|E|F|G|H|I|J|K|Q,	"whines"
    },

    {
	"lizard",	"",	FALSE,
	0,		0,		0,
	0,		RES_POISON,	VULN_COLD,
	A|G|X|cc,	A|C|D|E|F|H|K|Q|V,	"flicks it's tongue"
    },

    {
	"modron",	"",	FALSE,
	0,		AFF_INFRARED,		ASSIST_RACE|ASSIST_ALIGN,
	IMM_CHARM|IMM_DISEASE|IMM_MENTAL|IMM_HOLY|IMM_NEGATIVE,
			RES_FIRE|RES_COLD|RES_ACID,	0,
	H,		A|B|C|G|H|J|K,		"screeches"
    },

    {
	"orc",	"",		FALSE,
	0,		AFF_INFRARED,		0,
	0,		RES_DISEASE,	VULN_LIGHT,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K,		"booms"
    },

    {
	"pig",	"",		FALSE,
	0,		0,		0,
	0,		0,		0,
	A|G|V,	 	A|C|D|E|F|H|J|K,		"oinks"
    },	

    {
	"rabbit",	"",	FALSE,
	0,		0,		OFF_DODGE|OFF_FAST,
	0,		0,		0,
	A|G|V,		A|C|D|E|F|H|J|K,		"wriggles it's nose"
    },
    
    {
	"school monster",	  "",		FALSE,
	ACT_NOALIGN,		0,		0,
	IMM_CHARM|IMM_SUMMON,	0,		VULN_MAGIC,
	A|M|V,		A|B|C|D|E|F|H|J|K|Q|U,	"thunders"
    },	

    {
	"snake",	"",	FALSE,
	0,		0,		0,
	0,		RES_POISON,	VULN_COLD,
	A|G|X|Y|cc,	A|D|E|F|K|L|Q|V|X,		"hisses"
    },
 
    {
	"song bird",	"",	FALSE,
	0,		AFF_FLYING,		OFF_FAST|OFF_DODGE,
	0,		0,		0,
	A|G|W,		A|C|D|E|F|H|K|P,		"chirps"
    },

    {
	"troll",	"",	FALSE,
	0,		AFF_REGENERATION|AFF_INFRARED|AFF_DETECT_HIDDEN,
	OFF_BERSERK,
 	0,	RES_CHARM|RES_BASH,	VULN_FIRE|VULN_ACID,
	A|B|H|M|V,		A|B|C|D|E|F|G|H|I|J|K|U|V,	"garbles"
    },

    {
	"water fowl",	"",	FALSE,
	0,		AFF_WATERBREATHING|AFF_FLYING,	0,
	0,		RES_DROWNING,		0,
	A|G|W,		A|C|D|E|F|H|K|P,		"sqwauks"
    },		
  
    {
	"wolf",	"",		FALSE,
	0,		AFF_DARK_VISION,		OFF_FAST|OFF_DODGE,
	0,		0,		0,	
	A|G|V,		A|C|D|E|F|J|K|Q|V,	"howls"
    },

    {
	"horse",	"",		FALSE,
	0,		AFF_DARK_VISION,		OFF_FAST|OFF_DODGE|OFF_KICK,
	0,		0,		0,	
	A|G|V,		A|C|D|E|F|J|K|L|Q,	"whinnies"
    },

    {
	"wyvern",	"",	FALSE,
	0,		AFF_FLYING|AFF_DETECT_INVIS|AFF_DETECT_HIDDEN,
	OFF_BASH|OFF_FAST|OFF_DODGE,
	IMM_POISON,	0,	VULN_LIGHT,
	A|B|G|Z,		A|C|D|E|F|H|J|K|Q|V|X,		"screeches"
    },

    {
	"unique",	"",	FALSE,
	0,		0,		0,
	0,		0,		0,		
	0,		0,	"says"
    },

    {
	NULL, 0, 0, 0, 0, 0, 0
    }
};

const	struct	pc_race_type	pc_race_table	[]	=
{
    { "null race", "", 0, { 100,100,100,100,100,100,100,100,100,100,100,100,100,100,100 },
      { "" }, { 13, 13, 13, 13, 13, 13 }, { 18, 18, 18, 18, 18, 18 }, 0 , 1, 1, 0 },
 
/*
    {
	"race name", 	short name, points,	{ class multipliers },
	{ bonus skills },
        { base stats },		{ max stats },		size, base_age, max_age,
        home town(vnum)
    },
*/
    {
	"human", "Human", 0, {100,100,100,100,100,100,100,150,100,100,100,100,100,100,100,130 },
	{ "common", "recall", "instruct" },
        { 15, 13, 13, 13, 13, 13 },	{ 32, 32, 32, 32, 32, 32 },	SIZE_MEDIUM, 17, 80,
        ROOM_VNUM_TEMPLE
    },

    {
    "draconian",  "Draconian", 10, {100,100,150,100,150,100,150,150,100,100,130,100,100,100,100,130 },
    {"glide", "draconian tongue", "common", "recall", "instruct" },
    { 16, 10, 13, 17, 11, 10 }, { 35, 32, 31, 34, 32, 18 }, SIZE_LARGE, 200, 1000,
	   ROOM_VNUM_TEMPLE
    },

    {
      "half-elf",  "Half-Elf", 8,{100,100,100,110,100,150,150,150,100,120,120,100,100,100,100,125 },
      { "elven tongue", "qualinesti tongue", "silvanesti tongue", "common", "recall", "instruct" },
      { 13, 15, 13, 13, 13, 16 }, { 32, 34, 32, 32, 32, 35 }, SIZE_MEDIUM, 34, 160,
        ROOM_VNUM_TEMPLE

    },

    { 	
	"silvanesti", "Silvanesti", 10, { 100,125,100,120,100,150,150,150,100,150,130,100,100,100,100,130 }, 
	{ "silvanesti tongue", "elven tongue", "common", "recall", "instruct" },
	{ 11, 15, 13, 13, 13, 16 },	{ 28, 34, 32, 32, 32, 35 }, SIZE_MEDIUM, 255, 1200,
        ROOM_VNUM_TEMPLE
    },

    {
      "qualinesti", "Qualinesti", 10,  {100,125,100,120,100,150,150,150,100,150,125,100,100,100,100,135 },
      { "qualinesti tongue", "elven tongue", "common", "recall", "instruct" },        
      { 11, 15, 13, 13, 13, 16 }, { 28, 34, 32, 32, 32, 35 }, SIZE_MEDIUM, 255, 1200,
        ROOM_VNUM_TEMPLE
    },

    {
      "kagonesti", "Kagonesti", 8, {150,120,100,125,100,150,100,150,100,125,125,100,100,100,100,145 },
      { "sneak", "kagonesti tongue", "elven tongue", "common", "recall", "instruct" },
      { 13, 11, 13, 15, 13, 16 }, { 30, 30, 32, 34, 32, 35 }, SIZE_MEDIUM, 255, 1200,
        ROOM_VNUM_TEMPLE
    },

    { "dargonesti", "Dargonesti", 10,  { 100,125,100,120,100,150,150,150,100,150,120,100,100,100,100,140 },
      { "dargonesti tongue", "elven tongue", "common", "recall", "instruct" },
      { 11, 15, 13, 13, 13, 16 }, { 30, 34, 32, 32, 32, 35 }, SIZE_MEDIUM, 255, 1200,
        ROOM_VNUM_TEMPLE
    },

    { "dimernesti", "Dimernesti", 10, {100,125,100,120,100,150,150,150,100,150,120,100,100,100,100,150 },
      { "dimernesti tongue", "elven tongue", "common", "recall", "instruct" },
      { 11, 15, 13, 13, 13, 16 }, { 30, 34, 32, 32, 32, 35 }, SIZE_MEDIUM, 255, 1200,
        ROOM_VNUM_TEMPLE
    },

    {
	"neidar", "Neidar", 8, {150,125,150,100,120,150,150,150,120,150,150,100,100,100,100,100 },
	{ "dwarven tongue", "common", "recall", "instruct" },
	{ 14, 12, 14, 10, 15, 10 },	{ 32, 31, 32, 30, 34, 31 }, SIZE_MEDIUM, 85, 400,
        ROOM_VNUM_TEMPLE
    },

   { 
     "hylar", "Hylar", 8, { 150,125,150,100,120,150,150,150,150,150,150,100,100,100,100,100 },
     { "dwarven tongue", "common", "recall", "instruct" },
     { 14, 12, 14, 10, 15, 10 },  { 32, 31, 32, 30, 34, 31 }, SIZE_MEDIUM, 85, 400,
        ROOM_VNUM_TEMPLE
   },

  {
    "theiwar",  "Theiwar", 8, {150,125,150,100,120,150,150,150,120,100,150,100,100,100,100,110 },
    { "sneak", "dwarven tongue", "common", "recall", "instruct" },
    { 14, 12, 10, 13, 15, 10 },   { 32, 31, 29, 32, 34, 31 }, SIZE_MEDIUM, 85, 400,
        ROOM_VNUM_TEMPLE
  },

  {
    "aghar",  "Aghar", 0, {150,120,100,100,130,150,100,150,175,150,150,100,100,100,100,115 },
    { "sneak", "hide", "dwarven tongue", "common", "recall", "instruct" },
    { 12, 8, 8, 13, 17, 10 },    { 33, 26, 27, 32, 34, 30 }, SIZE_SMALL, 85, 400,
        ROOM_VNUM_TEMPLE
  },

  {
    "daergar",  "Daergar", 0, {150,120,100,100,130,150,100,150,175,150,150,100,100,100,100,115 },
    { "bash", "handtohand", "dwarven tongue", "common", "recall", "instruct" },
    { 12, 8, 8, 13, 17, 10 },    { 33, 26, 27, 32, 34, 30 }, SIZE_SMALL, 85, 400,
        ROOM_VNUM_TEMPLE
  },

  {
    "daewar",  "Daewar", 0, {150,120,100,100,130,150,100,150,175,150,150,100,100,100,100,115 },
    { "smith", "masterwork", "dwarven tongue", "common", "recall", "instruct" },
    { 12, 8, 8, 13, 17, 10 },    { 33, 26, 27, 32, 34, 30 }, SIZE_SMALL, 85, 400,
        ROOM_VNUM_TEMPLE
  },

  {
    "klar",  "klar", 0, {150,120,100,100,130,150,100,150,175,150,150,100,100,100,100,115 },
    { "hide", "dwarven tongue", "common", "recall", "instruct" },
    { 12, 8, 8, 13, 17, 10 },    { 33, 26, 27, 32, 34, 30 }, SIZE_SMALL, 85, 400,
        ROOM_VNUM_TEMPLE
  },

  {
    "gnome",  "Gnome", 5, {125,130,150,175,150,150,150,100,175,150,150,100,100,100,100,150 },
    { "common", "recall", "instruct" }, 
    { 12, 15, 10, 11, 15, 10 },  { 31, 34, 29, 30, 33, 29 }, SIZE_SMALL, 65, 350,
        ROOM_VNUM_TEMPLE
  },
 
 {
   "kender",  "Kender", 7 , { 150,190,100,135,100,150,150,150,120,175,120,100,100,100,100,150 },
   { "sneak", "hide", "kender tongue", "taunt", "common", "recall", "instruct" }, 
   { 12, 12, 10, 15, 11, 13 },  { 31, 30, 28, 34, 32, 32 }, SIZE_SMALL, 25, 100,
        ROOM_VNUM_TEMPLE
 },

 {
   "minotaur", "Minotaur", 15, {130,175,150,100,150,150,100,150,175,100,150,100,100,100,100,135 },
   { "bash", "handtohand", "minotaur tongue", "common", "recall", "instruct" }, 
   { 15, 12, 11, 10, 15, 10 },  { 35, 30, 29, 28, 34, 18 }, SIZE_LARGE, 37, 175,
        ROOM_VNUM_TEMPLE
 },

 {
   "ogre", "Ogre", 6, {150,120,150,100,150,150,100,150,180,150,130,100,100,100,100,160 },
   { "bash", "ogre tongue", "common", "recall", "instruct" },
   { 16, 10, 13, 11, 15, 10 },	{ 35, 24, 30, 27, 32, 17 }, SIZE_LARGE, 128, 600,
        ROOM_VNUM_TEMPLE
 }
};
      	

/*
 * Class table.
 */
const	struct	class_type	class_table	[MAX_CLASS]	=
{
    {
	"battlemage", "Battlemage   - Formiddable warriors who've also taken to magic", "Bam",  STAT_INT,  OBJ_VNUM_SCHOOL_SWORD,
	{ 3345, 1683, 2652, 12142, 5778, 5912, 9717 },  75,  20, 6,  9,  14, 6, 12, TRUE,
        TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, TRUE, TRUE, FALSE,
        FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE
    },

    {
	"cleric", "Cleric       - Holy warriors who's powers excell in healing and defense", "Cle",  STAT_WIS,  OBJ_VNUM_SCHOOL_MACE,
	{ 3003, 2559, 11912, 5779, 5903, 964, 11223, 9718 },  75,  20, 2,  7, 11, 8, 18, TRUE,
        TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE,
        TRUE, TRUE, TRUE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE
    },

    {
	"thief", "Thief        - Specialists at thievery and covert actions", "Thi",  STAT_DEX,  OBJ_VNUM_SCHOOL_DAGGER,
	{ 3028, 2547, 1656, 11128, 5780, 10643, 5902, 9719 },  75,  20,  -4,  9, 14, 2, 4, TRUE,
        TRUE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE,
        FALSE, TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE
    },

    {
	"warrior", "Warrior      - Weapon masters and fighters of supperior strategic skill", "War",  STAT_STR,  OBJ_VNUM_SCHOOL_SWORD,
	{ 3022, 2595, 1628, 11184, 5781, 10672, 5898, 9720 },  75,  20,  -10,  13, 19, 2, 4, TRUE,
        TRUE, TRUE, TRUE, FALSE, FALSE, TRUE, FALSE, FALSE, TRUE,
        TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, TRUE, TRUE
    },

    {
       "bard", "Bard         - Tough fighters skilled in the magic of music and song", "Bar",  STAT_CHR,  OBJ_VNUM_SCHOOL_DAGGER,
       { 3028, 2545, 1682, 12028, 5782, 5904, 9721 }, 75, 20, 1, 8, 15, 5, 13, TRUE,
        TRUE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE,
        TRUE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE, TRUE, FALSE, FALSE
    },

    {
      "knight", "Knight       - Loyal warriors, well skilled in fighting and weapons", "Kni", STAT_STR, OBJ_VNUM_SCHOOL_SWORD,
      { 3146, 2587, 1627, 11119, 5783, 5913, 960, 9722 }, 75, 20, -15, 16, 22, 2, 4, TRUE,
        TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,
        FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE
    },

    {
      "barbarian", "Barbarian    - Brute fighters well skilled in hand to hand combat", "Bab", STAT_STR, OBJ_VNUM_SCHOOL_WHIP,
      { 3280, 1635, 2570, 11194, 5784, 10680, 5900, 9723 }, 75, 20, -7, 12, 20, 2, 4, TRUE,
        TRUE, FALSE, TRUE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE,
        FALSE, FALSE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE, TRUE, TRUE
    },

    {
      "druid", "Druid        - Mages specializing in the magic of Nature", "Dru", STAT_WIS, OBJ_VNUM_SCHOOL_STAFF,
      { 3003, 2477, 1684, 12147, 5785, 5901, 9725 }, 75, 18, 4, 5, 14, 8, 17, TRUE,
        TRUE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE,
        TRUE, TRUE, TRUE, FALSE, TRUE, TRUE, FALSE, TRUE, FALSE, FALSE
    },
 
    {
      "ranger", "Ranger       - Fighters excelling in wilderness tactics and survival", "Ran", STAT_DEX, OBJ_VNUM_SCHOOL_DAGGER,
      { 3022, 1658, 2654, 11982, 5786, 5899, 9726, 3857 }, 75, 20, -4, 11, 19, 4, 9, TRUE,
        TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE,
        FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, FALSE, TRUE, TRUE, TRUE
    },

    {
      "assassin", "Assassin     - Masters of stealth and hired killing", "Ass", STAT_DEX, OBJ_VNUM_SCHOOL_DAGGER,
     { 3036, 10965, 2653, 11940, 5787, 11217, 5906, 961, 9729 }, 75, 20, -2, 10, 14, 2, 4, TRUE,
        TRUE, TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,
        FALSE, TRUE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE, TRUE, TRUE
    },

    /* New Classes Written by Hiddukel */

    {
      "spellfilcher", "Spellfilcher - Rogues who master in spells of trickery and stealth ", "Spe", STAT_WIS, OBJ_VNUM_SCHOOL_DAGGER,
      { 3022, 2584, 1685, 12150, 5788, 5905, 9730 }, 75, 20, -2, 10, 20, 6, 12, TRUE,
        TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE,
        FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE
    },

    {
	"necromancer", "Necromancer  - Masters of death, unlife, pain, and disease", "Nec",  STAT_INT,  OBJ_VNUM_SCHOOL_DAGGER,
	{ 3245, 10958, 2648, 11964, 5775, 11370, 5908, 9727 },  75,  20, 6,  6,  12, 10, 20, TRUE,
        TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, TRUE, TRUE, FALSE,
        FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE
    },

    {
	"enchanter", "Enchanter    - Masters of alteration, enchantment, transmutation and charm", "Enc",  STAT_INT,  OBJ_VNUM_SCHOOL_DAGGER,
	{ 3244, 10958, 2649, 11964, 5776, 11370, 5910, 9728 },  75,  20, 6,  6,  12, 10, 20, TRUE,
        TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, TRUE, TRUE, FALSE,
        FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE
    },

    {
	"illusionist", "Illusionist  - Masters of illusion, phantasm, and illusory alteration", "Ill",  STAT_INT,  OBJ_VNUM_SCHOOL_DAGGER,
	{ 3243, 10958, 2650, 11964, 5777, 11370, 5909, 9731 },  75,  20, 6,  6,  12, 10, 20, TRUE,
        TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, TRUE, TRUE, FALSE,
        FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE
    },

    {
	"conjuror", "Conjuror     - Masters of evocation, invokation, summoning, and conjuration", "Con",  STAT_INT,  OBJ_VNUM_SCHOOL_DAGGER,
	{ 3018, 10958, 2651, 11964, 5774, 11370, 5911, 9732 },  75,  20, 6,  6,  12, 10, 20, TRUE,
        TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, TRUE, TRUE, FALSE,
        FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE
    },

    {
	"blacksmith", "Blacksmith   - A NON-FIGHTING Class - forge, repair armor and weapons", "Bla",  STAT_WIS,  0,
	{  5793, 9724 },  75,  20, 2,  7, 11, 8, 18, TRUE,
        TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE,
        TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE
    }
    
};

 char * const title_table [MAX_CLASS][MAX_LEVEL+1][2] =
 { 
    {
 	{ "Man", "Woman" },
 	{ "Reckless", "Reckless" },
 	{ "Brave", "Brave" },
 	{ "Foolhardy", "Foolhardy" },
 	{ "Learner of Secrets", "Learner of Secrets" },
 	{ "Learner of Secrets", "Learner of Secrets" },
 	{ "Applier of Secrets", "Applier of Secrets" },
 	{ "Applier of Secrets", "Applier of Secrets" },
 	{ "Initiate of the Body", "Initiate of the Body" },
 	{ "Initiate of the Body", "Initiate of the Body" },
 	{ "Initiate of the Mind", "Initiate of the Mind" },
 	{ "Initiate of the Mind", "Initiate of the Mind" },
 	{ "Initiate of the Physical", "Initiate of the Physical" },
 	{ "Initiate of the Physical", "Initiate of the Physical" },
 	{ "Initiate of the Mental", "Initiate of the Mental" },
 	{ "Initiate of the Mental", "Initiate of the Mental" },
 	{ "Initiate of the Void", "Initiate of the Void" },
 	{ "Initiate of the Void", "Initiate of the Void" },
 	{ "Learned of the Body", "Learned of the Body" },
 	{ "Learned of the Body", "Learned of the Body" },
 	{ "Learned of the Mind", "Learned of the Mind" },
 	{ "Learned of the Mind", "Learned of the Mind" },
 	{ "Learned of the Physical", "Learned of the Physical" },
 	{ "Learned of the Physical", "Learned of the Physical" },
 	{ "Learned of the Mental", "Learned of the Mental" },
 	{ "Learned of the Mental", "Learned of the Mental" },
 	{ "Student of the Void", "Student of the Void" },
 	{ "Student of the Void", "Student of the Void" },
 	{ "Master of the Body", "Mistress of the Body" },
 	{ "Master of the Body", "Mistress of the Body" },
 	{ "Master of the Mind", "Mistress of the Mind" },
 	{ "Master of the Mind", "Mistress of the Mind" },
 	{ "Master of the Physical", "Mistress of the Physical" },
 	{ "Master of the Physical", "Mistress of the Physical" },
 	{ "Lord of the Void", "Lady of the Void" },
 	{ "Lord of the Void", "Lady of the Void" },
 	{ "Tempter of Magics", "Tempter of Magics" },
 	{ "Tempter of Fate", "Tempter of Fate" },
 	{ "Tempter of Fate", "Tempter of Fate" },
 	{ "Wielder of Magic", "Wielder of Magic"},
 	{ "Initiate Battlemage", "Initiate Battlemage" },
 	{ "Student Battlemage", "Student Battlemage" },
 	{ "Lesser Battlemage", "Lesser Battlemage" },
 	{ "Battlemage", "Battlemage" },
 	{ "Greater Battlemage", "Greater Battlemage" },
 	{ "Greater Battlemage", "Greater Battlemage" },
 	{ "Elder Battlemage", "Elder Battlemage"},
 	{ "Elder Battlemage", "Elder Battlemage"},
 	{ "Courter of Fire", "Courter of Fire"},
 	{ "Courter of Acid", "Courter of Acid"},
 	{ "Bound of Fire", "Bound of Fire"},
 	{ "Bound of Acid", "Bound of Acid" },
 	{ "Master of Fire", "Mistress of Fire" },
 	{ "Master of Acid", "Mistress of Acid" },
 	{ "Greater Master of the Void", "Greater Mistress of the Void"},
 	{ "Master of the Siege", "Mistress of the Siege"},
 	{ "Weapon of Magic", "Weapon of Magic"},
 	{ "Magic of Weapons", "Magic of Weapons"},
 	{ "Combiner of Opposites", "Combiner of Opposites"},
 	{ "Strategist", "Strategist"},
 	{ "Learned Strategist", "Learned Strategist"},
 	{ "Grand Strategist", "Grand Strategist"},
 	{ "Flinger of Missiles", "Flinger of Missiles"},
 	{ "Flinger of Flame", "Flinger of Flame"},
 	{ "Archer of Acid", "Archer of Acid"},
 	{ "Initiate of Destruction", "Initiate of Destruction"},
 	{ "Initiate of Magical Weapons", "Initiate of Magical Weapons"},
 	{ "Learner of Destruction", "Learner of Destruction"},
 	{ "Learner of Magical Weapons", "Learner of Magical Weapons"},
 	{ "Master Destructor", "Mistress Destructor"},
 	{ "Master Destructor", "Mistress Destructor"},
 	{ "Master of Magical Weapons", "Mistress of Magical Weapons"},
 	{ "Master Lord of Destruction", "Mistress Lady of Destruction"},
 	{ "Master Lord of Destruction", "Mistress Lady of Destruction"},
 	{ "Master Lord of Magical Weapons", "Mistress Lady of Magical Weapons"},
 	{ "Scatterer of Spears", "Scatterer of Spears"},
 	{ "Discharger of Daggers", "Discharger of Daggers"},
 	{ "Rainer of Death From Above", "Rainer of Death From Above"},
 	{ "Rainer of Death From Above", "Rainer of Death From Above"},
 	{ "Punisher of Villages", "Punisher of Villages"},
 	{ "Punisher of Villages", "Punisher of Villages"},
 	{ "Ravager of Armies", "Ravager of Armies"},
 	{ "Ravager of Armies", "Ravager of Armies"},
 	{ "Master of Swords", "Mistress of Swords"},
 	{ "Weapon of Kings", "Weapon of Kings" },
 	{ "Weapon of Kings", "Weapon of Kings" },
 	{ "Savior of Cities", "Savior of Cities" },
 	{ "Savior of Cities", "Savior of Cities" },
 	{ "Guardian of the Countryside","Guardian of the Countryside "},
 	{ "Learned of the Disparate", "Learned of the Disparate"},
 	{ "Learned of the Disparate", "Learned of the Disparate"},
 	{ "Master of the Disparate", "Mistress of the Disparate" },
 	{ "Master of the Disparate", "Mistress of the Disparate" },
 	{ "Master of the Twofold", "Mistress of the Twofold"},
 	{ "Master of the Twofold", "Mistress of the Twofold"},
 	{ "Master of the Double Art", "Mistress of the Double Art"},
 	{ "Master of the Double Art", "Mistress of the Double Art"},
 	{ "Master of the Double Art", "Mistress of the Double Art"},
 	{ "Grand Master Battlemage", "Grand Master Battlemage"},
 	{ "Grand Master Battlemage", "Grand Master Battlemage"},
 	{ "Grand Master Battlemage", "Grand Master Battlemage"},
 	{ "Hero of the Double Art", "Hero of the Double Art"},
 	{ "Avatar", "Avatar" },
 	{ "Angel", "Angel" },
 	{ "Demigod", "Demigoddess", },
 	{ "Immortal", "Immortal" },
 	{ "God", "Goddess" },
 	{ "Deity", "Deity" },
 	{ "Supreme Master", "Supreme Mistress" },
 	{ "Creator", "Creator" },
 	{ "Implementor", "Implementor" } 
   }, 

   {
 	{ "Man", "Woman" },
	{ "Believer", "Believer" },
	{ "Believer", "Believer" },
	{ "Attendant", "Attendant" },
	{ "Attendant", "Attendant" },
	{ "Acolyte", "Acolyte" },
	{ "Acolyte", "Acolyte" },
	{ "Novice", "Novice" },
	{ "Novice", "Novice" },
	{ "Missionary", "Missionary" },
	{ "Missionary", "Missionary" },
	{ "Adept", "Adept" },
	{ "Adept", "Adept" },
	{ "Deacon", "Deacon" },
	{ "Deacon", "Deacon" },
	{ "Vicar", "Vicar" },
	{ "Vicar", "Vicar" },
	{ "Monsignor", "Monsignor" },
	{ "Monsignor", "Monsignor" },
	{ "Minister", "Minister" },
	{ "Minister", "Minister" },
	{ "Canon", "Canon" },
	{ "Canon", "Canon" },
	{ "Levite", "Levite" },
	{ "Levite", "Levite" },
	{ "Curate", "Curate" },
	{ "Curate", "Curate" },
	{ "Chaplain", "Chaplain" },
	{ "Chaplain", "Chaplain" },
	{ "Expositor", "Expositor" },
	{ "Expositor", "Expositor" },
	{ "Bishop", "Bishop" },
	{ "Bishop", "Bishop" },
	{ "Lesser Patriarch", "Lesser Matriarch" },
	{ "Lesser Patriarch", "Lesser Matriarch" },
	{ "Patriarch", "Matriarch" },
	{ "Patriarch", "Matriarch" },
	{ "Elder Patriarch", "Elder Matriarch" },
	{ "Elder Patriarch", "Elder Matriarch" },
	{ "Grand Patriarch", "Grand Matriarch" },
	{ "Grand Patriarch", "Grand Matriarch" },
	{ "Lesser Demon Killer", "Lesser Demon Killer" },
	{ "Lesser Demon Killer", "Lesser Demon Killer" },
	{ "Demon Killer", "Demon Killer" },
	{ "Demon Killer", "Demon Killer" },
	{ "Greater Demon Killer", "Greater Demon Killer" },
	{ "Greater Demon Killer", "Greater Demon Killer" },
	{ "Cardinal of the Water", "Cardinal of the Water" },
	{ "Cardinal of the Water", "Cardinal of the Water" },
	{ "Cardinal of the Earth", "Cardinal of the Earth" },
	{ "Cardinal of the Earth", "Cardinal of the Earth" }, 
	{ "Cardinal of the Wind", "Cardinal of the Wind" },
	{ "Cardinal of the Wind", "Cardinal of the Wind" }, 
	{ "Cardinal of the Fire", "Cardinal of the Fire" },
	{ "Cardinal of the Fire", "Cardinal of the Fire" },
	{ "Cardinal of the Sea", "Cardinal of the Sea" },
	{ "Cardinal of the Sea", "Cardinal of the Sea" },
	{ "Cardinal of the Mountains", "Cardinal of the Mountains" },
	{ "Cardinal of the Mountains", "Cardinal of the Mountains" },
	{ "Cardinal of the Sky", "Cardinal of the Sky" },
	{ "Cardinal of the Sky", "Cardinal of the Sky" },
	{ "Cardinal of the Ether", "Cardinal of the Ether" },
	{ "Cardinal of the Ether", "Cardinal of the Ether" },
	{ "Cardinal of the Heavens", "Cardinal of the Heavens" }, 
	{ "Cardinal of the Heavens", "Cardinal of the Heavens" },
	{ "Cardinal of the Gods", "Cardinal of the Gods" },
	{ "Cardinal of the Gods", "Cardinal of the Gods" },
	{ "Blessed", "Blessed" }, 
	{ "Divine", "Divine" }, 
	{ "Holy", "Holy" },
	{ "Holy", "Holy" }, 
	{ "Chosen", "Chosen" },
	{ "Chosen", "Chosen" }, 
	{ "Favored", "Favored" },
	{ "Favored", "Favored" },
	{ "Messiah", "Messiah" },
	{ "Messiah", "Messiah" }, 
	{ "High Cleric", "High Cleric" },
	{ "High Cleric", "High Cleric" },
	{ "Arch Cleric", "Arch Cleric" },
	{ "Arch Cleric", "Arch Cleric" },
	{ "Elder Cleric", "Elder Cleric" },
	{ "Elder Cleric", "Elder Cleric" },
	{ "Cleric of Hope", "Cleric of Hope" },
	{ "Cleric of Hope", "Cleric of Hope" },
	{ "Cleric of Life", "Cleric of Life" }, 
	{ "Cleric of Life", "Cleric of Life" }, 
	{ "Cleric of the Blatant", "Cleric of the Blatant" }, 
	{ "Cleric of the Visible", "Cleric of the Visible" }, 
	{ "Cleric of the Doubted", "Cleric of the Doubted" }, 
	{ "Cleric of the Hidden", "Cleric of the Hidden" },
 	{ "Cleric of the Hidden", "Cleric of the Hidden" }, 
	{ "Cleric of the Invisible", "Cleric of the Invisible" }, 
	{ "Cleric of the Invisible", "Cleric of the Invisible" }, 
	{ "Cleric of the Divine", "Cleric of the Divine" }, 
	{ "Cleric of the Divine", "Cleric of the Divine" },
	{ "Cleric of the Divine", "Cleric of the Divine" }, 
	{ "Master of All Divinity", "Mistress of All Divinity" }, 
	{ "Cleric Hero", "Cleric Heroine" }, 
	{ "Cleric Avatar", "Cleric Avatar" }, 
	{ "Angel", "Angel" }, 
	{ "Demigod", "Demigoddess", }, 
	{ "Immortal", "Immortal" }, 
	{ "God", "Goddess" }, 
	{ "Deity", "Deity" }, 
	{ "Supreme Master", "Supreme Mistress" }, 
	{ "Creator", "Creator" }, 
	{ "Implementor", "Implementor" } 
      }, 

   { 
 	{ "Man", "Woman" },
 	{ "Pilferer", "Pilferer" },
 	{ "Acrobat", "Acrobat" },
 	{ "Adventurer", "Adventurer" },
 	{ "Bandit", "Bandit" },
 	{ "Footpad", "Footpad" },
 	{ "Dervish", "Dervish" },
 	{ "Filcher", "Filcher" },
 	{ "Beggar", "Beggar" },
 	{ "Gambler", "Gambler" },
 	{ "Loanshark", "Loanshark" },
 	{ "Protector of Racketeering", "Protector of Racketeering" },
 	{ "Buccaneer", "Buccaneer" },
 	{ "Clod", "Clod" },
 	{ "Pick-Pocket", "Pick-Pocket" },
 	{ "Sneak", "Sneak" },
 	{ "Pincher", "Pincher" },
 	{ "Fence", "Fence" },
 	{ "Grifter", "Grifter" },
 	{ "Investigator", "Investigator" },
 	{ "Sewer Rat", "Sewer Rat" },
 	{ "Snatcher", "Snatcher" },
 	{ "Sharper", "Sharper" },
 	{ "Jester", "Jester" },
 	{ "Matrud", "Matrud" },
 	{ "Rogue", "Rogue" },
 	{ "Merchant Rogue", "Merchant Rogue" },
 	{ "Robber", "Robber" },
 	{ "Nightblade", "Nightblade" },
 	{ "Magsman", "Magswoman" },
 	{ "Highwayman", "Highwaywoman" },
 	{ "Burglar", "Burglar" },
 	{ "Cat Burglar", "Cat Burglar" },
 	{ "Planewalker Thief", "Planewalker Thief" },
 	{ "Thief", "Thief" },
 	{ "Scout", "Scout" },
 	{ "Counterfeiter", "Counterfeiter" },
 	{ "Seductor", "Seductress" },
 	{ "Knifer", "Knifer" },
 	{ "Quick-Blade", "Quick-Blade" },
 	{ "Killer", "Killer" },
 	{ "Shadow", "Shadow" },
 	{ "Smuggler", "Smuggler" },
 	{ "Brigand", "Brigand" },
 	{ "Cut-Throat", "Cut-Throat" },
 	{ "Lesser Spy", "Lesser Spy" },
 	{ "Spy", "Spy" },
 	{ "Swashbuckler", "Swashbuckler" },
 	{ "Extortionist", "Extortionist" },
 	{ "Blackmailer", "Blackmailer" },
 	{ "Swindler", "Swindler" },
 	{ "Thug", "Thug" },
 	{ "High Thief", "High Thief" },
 	{ "High Thief", "High Thief" },
 	{ "Greater Spy", "Greater Spy" },
 	{ "Greater Spy", "Greater Spy" },
 	{ "Grand Spy", "Grand Spy" },
 	{ "Grand Spy", "Grand Spy" },
 	{ "Troubadour", "Troubadour" },
 	{ "Troubleshooter", "Troubleshooter" },
 	{ "Warrior Rogue", "Warrior Rogue" },
 	{ "Master Spy", "Mistress Spy" },
 	{ "Master Spy", "Mistress Spy" },
 	{ "Master of Vision", "Mistress of Vision" },
 	{ "Master of Vision", "Mistress of Vision" },
 	{ "Master of Hearing", "Mistress of Hearing" },
 	{ "Master of Hearing", "Mistress of Hearing" },
 	{ "Master of Smell", "Mistress of Smell" },
 	{ "Master of Smell", "Mistress of Smell" },
 	{ "Master of Taste", "Mistress of Taste" },
 	{ "Master of Taste", "Mistress of Taste" },
 	{ "Master of Touch", "Mistress of Touch" },
 	{ "Master of Touch", "Mistress of Touch" },
 	{ "Crime Lord", "Crime Mistress" },
 	{ "Infamous Crime Lord", "Infamous Crime Mistress" },
 	{ "Greater Crime Lord", "Greater Crime Mistress" },
 	{ "Master Crime Lord", "Master Crime Mistress" },
 	{ "Supreme Crime Lord", "Supreme Crime Mistress" },
 	{ "Master of Shadows", "Mistress of Shadows" },
 	{ "Master of Stealth", "Mistress of Stealth" },
 	{ "Master of Treachery", "Mistress of Treachery" },
 	{ "Master of Pain", "Mistress of Pain" },
 	{ "Master of Shadows", "Mistress of Shadows" },
 	{ "Master of Death", "Mistress of Death" },
 	{ "Master Thief", "Mistress Thief" },
 	{ "Lord of Fear", "Lady of Fear" },
 	{ "Lord of Mystery", "Lady of Mystery" },
 	{ "Lord of Darkness", "Lady of Darkness" },
 	{ "Lord of the Unseen", "Lady of the Unseen" },
 	{ "Lord of the Underworld", "Lady of the Underworld" },
 	{ "Thief of Doom ", "Thief of Doom" },
 	{ "Thief of Doom ", "Thief of Doom" },
 	{ "Thief of Despair", "Thief of Despair" },
 	{ "Thief of Despair", "Thief of Despair" },
 	{ "Thief of Sorrow", "Thief of Sorrow" },
 	{ "Thief of Sorrow", "Thief of Sorrow" },
 	{ "Thief of Life", "Thief of Life" },
 	{ "Thief of Life", "Thief of Life" },
 	{ "Thief of Souls", "Thief of Souls" },
 	{ "Thief of Souls", "Thief of Souls" },
 	{ "Thief Hero", "Thief Heroine" },
 	{ "Master of all Thieves", "Mistress of all Thieves" },
 	{ "Avatar of Thieves", "Avatar of Thieves", },
 	{ "Angel of Thieves", "Angel of Thieves" },
 	{ "Demigod of Thieves", "Demigoddess of Thieves" },
 	{ "Immortal Thief", "Immortal Thief" },
 	{ "God of Thieves", "God of Thieves", },
 	{ "Deity of Thieves", "Deity of Thieves" },
 	{ "Supreme Master", "Supreme Mistress" },
 	{ "Creator", "Creator" },
 	{ "Implementor", "Implementor" }
   },

   {
 	{ "Man", "Woman" },
 	{ "Swordpupil", "Swordpupil" },
 	{ "Swordpupil", "Swordpupil" },
 	{ "Recruit", "Recruit" },
 	{ "Recruit", "Recruit" },
 	{ "Sentry", "Sentry" },
 	{ "Sentry", "Sentry" },
 	{ "Fighter", "Fighter" },
 	{ "Fighter", "Fighter" },
 	{ "Soldier", "Soldier" },
 	{ "Soldier", "Soldier" },
 	{ "Warrior", "Warrior" },
 	{ "Warrior", "Warrior" },
 	{ "Veteran", "Veteran" },
 	{ "Veteran", "Veteran" },
 	{ "Swordsman", "Swordswoman" },
 	{ "Swordsman", "Swordswoman" },
 	{ "Fencer", "Fencer" },
 	{ "Fencer", "Fencer" },
 	{ "Combatant", "Combatant" },
 	{ "Combatant", "Combatant" },
 	{ "Hero", "Heroine" },
 	{ "Hero", "Heroine" },
 	{ "Myrmidon", "Myrmidon" },
 	{ "Myrmidon", "Myrmidon" },
 	{ "Swashbuckler", "Swashbuckler" },
 	{ "Swashbuckler", "Swashbuckler" },
 	{ "Mercenary", "Mercenary" },
 	{ "Mercenary", "Mercenary" },
 	{ "Swordmaster", "Swordmistress" },
 	{ "Swordmaster", "Swordmistress" },
 	{ "Lieutenant", "Lieutenant" },
 	{ "Lieutenant", "Lieutenant" },
 	{ "Champion", "Lady Champion" },
 	{ "Champion", "Lady Champion" },
 	{ "Dragoon", "Lady Dragoon" },
 	{ "Dragoon", "Lady Dragoon" },
 	{ "Cavalier", "Lady Cavalier" },
 	{ "Cavalier", "Lady Cavalier" },
 	{ "Knight", "Lady Knight" },
 	{ "Knight", "Lady Knight" },
 	{ "Grand Knight", "Grand Knight" },
 	{ "Grand Knight", "Grand Knight" },
 	{ "Master Knight", "Master Knight" },
 	{ "Master Knight", "Master Knight" },
 	{ "Lesser Demon Slayer", "Lesser Demon Slayer" },
 	{ "Lesser Demon Slayer", "Lesser Demon Slayer" },
 	{ "Demon Slayer", "Demon Slayer" },
 	{ "Demon Slayer", "Demon Slayer" },
 	{ "Greater Demon Slayer", "Greater Demon Slayer" },
 	{ "Greater Demon Slayer", "Greater Demon Slayer" },
 	{ "Lesser Dragon Slayer", "Lesser Dragon Slayer" },
 	{ "Lesser Dragon Slayer", "Lesser Dragon Slayer" },
 	{ "Dragon Slayer", "Dragon Slayer" },
 	{ "Dragon Slayer", "Dragon Slayer" },
 	{ "Greater Dragon Slayer", "Greater Dragon Slayer" },
 	{ "Greater Dragon Slayer", "Greater Dragon Slayer" },
 	{ "Underlord", "Underlord" },
 	{ "Underlord", "Underlord" },
 	{ "Overlord", "Overlord" },
 	{ "Overlord", "Overlord" },
 	{ "Baron of Thunder", "Baroness of Thunder" },
 	{ "Baron of Thunder", "Baroness of Thunder" },
 	{ "Baron of Storms", "Baroness of Storms" },
 	{ "Baron of Storms", "Baroness of Storms" },
 	{ "Baron of Tornadoes", "Baroness of Tornadoes" },
 	{ "Baron of Tornadoes", "Baroness of Tornadoes" },
 	{ "Baron of Hurricanes", "Baroness of Hurricanes" },
 	{ "Baron of Hurricanes", "Baroness of Hurricanes" },
 	{ "Baron of Meteors", "Baroness of Meteors" },
 	{ "Baron of Meteors", "Baroness of Meteors" },
 	{ "Baron of Fire", "Baroness of Fire" },
 	{ "Baron of Fire", "Baroness of Fire" },
 	{ "Baron of Ice", "Baroness of Ice" },
 	{ "Baron of Ice", "Baroness of Ice" },
 	{ "Master of the Elements", "Mistress of the Elements" },
 	{ "Master of the Elements", "Mistress of the Elements" },
 	{ "Master of Copper", "Mistress of Copper" },
 	{ "Master of Copper", "Mistress of Copper" },
 	{ "Master of Brass", "Mistress of Brass" },
 	{ "Master of Brass", "Mistress of Brass" },
 	{ "Master of Iron", "Mistress of Iron" },
 	{ "Master of Iron", "Mistress of Iron" },
 	{ "General", "Lady General" },
 	{ "General", "Lady General" },
 	{ "Field Marshall", "Field Marshall" },
 	{ "Field Marshall", "Field Marshall" },
 	{ "Master of Tactics", "Mistress of Tactics" },
 	{ "Master of Tactics", "Mistress of Tactics" },
 	{ "Master of Weapons", "Mistress of Weapons" },
 	{ "Master of Weapons", "Mistress of Weapons" },
 	{ "Master of Might", "Mistress of Might" },
 	{ "Master of Might", "Mistress of Might" },
 	{ "Master of Power", "Mistress of Power" },
 	{ "Master of Power", "Mistress of Power" },
 	{ "Master of Combat", "Mistress of Combat" },
 	{ "Master of Combat", "Mistress of Combat" },
 	{ "Master of Battle", "Mistress of Battle" },
 	{ "Master of Battle", "Mistress of Battle" },
 	{ "Master of War", "Mistress of War" },
 	{ "Master of War", "Mistress of War" },
 	{ "Knight Hero", "Knight Heroine" },
 	{ "Avatar of War", "Avatar of War" },
 	{ "Angel of War", "Angel of War" },
 	{ "Demigod of War", "Demigoddess of War" },
 	{ "Immortal Warlord", "Immortal Warlord" },
 	{ "God of War", "Goddess of War" },
 	{ "Deity of War", "Deity of War" },
 	{ "Supreme Master of War", "Supreme Mistress of War" },
 	{ "Creator", "Creator" },
 	{ "Implementor", "Implementor" }
   },

   {
 	{ "Man", "Woman" },
 	{ "Fiddler", "Fiddler" },
 	{ "Fiddler", "Fiddler" },
 	{ "Lutist", "Lutist" },
 	{ "Lutist", "Lutist" },
 	{ "Harpist", "Harpist" },
 	{ "Harpist", "Harpist" },
 	{ "Piper", "Piper" },
 	{ "Piper", "Piper" },
 	{ "Fifer", "Fifer" },
 	{ "Fifer", "Fifer" },
 	{ "Chanter", "Chanter" },
 	{ "Chanter", "Chanter" },
 	{ "Warbler", "Warbler" },
 	{ "Warbler", "Warbler" },
 	{ "Crooner", "Crooner" },
 	{ "Crooner", "Crooner" },
 	{ "Serenader", "Serenader" },
 	{ "Serenader", "Serenader" },
 	{ "Songster", "Songster" },
 	{ "Songster", "Songster" },
 	{ "Poet", "Poet" },
 	{ "Poet", "Poet" },
 	{ "Bard", "Bard" },
 	{ "Bard", "Bard" },
 	{ "Versifier", "Versifier" },
 	{ "Versifier", "Versifier" },
 	{ "Rhymer", "Rhymer" },
 	{ "Rhymer", "Rhymer" },
 	{ "Musician", "Musician" },
 	{ "Musician", "Musician" },
 	{ "Instrumentalist", "Instrumentalist" },
 	{ "Instrumentalist", "Instrumentalist" },
 	{ "Artist", "Artist" },
 	{ "Artist", "Artist" },
 	{ "Performer", "Performer" },
 	{ "Performer", "Performer" },
 	{ "Jester", "Jester" },
 	{ "Jester", "Jester" },
 	{ "Virtuoso", "Virtuoso" },
 	{ "Virtuoso", "Virtuoso" },
 	{ "Master Bard", "Master Bard"},
 	{ "Master Bard", "Master Bard"},
 	{ "Composer", "Composer" },
 	{ "Composer", "Composer" },
 	{ "Arranger", "Arranger" },
 	{ "Arranger", "Arranger" },
 	{ "Melodist", "Melodist" },
 	{ "Melodist", "Melodist" },
 	{ "Tunesmith", "Tunesmith" },
 	{ "Tunesmith", "Tunesmith" },
 	{ "Lyricist", "Lyricist" },
 	{ "Lyricist", "Lyricist" },
 	{ "Ballader", "Ballader" },
 	{ "Ballader", "Ballader" },
 	{ "Historian", "Historian" },
 	{ "Historian", "Historian" },
 	{ "Musical Historian", "Musical Historian" },
 	{ "Musical Historian", "Musical Historian" },
 	{ "Minstrel", "Minstrel" },
 	{ "Minstrel", "Minstrel" },
 	{ "Player", "Player" },
 	{ "Player", "Player" },
 	{ "Lyric Performer", "Lyric Performer" },
 	{ "Lyric Performer", "Lyric Performer" },
 	{ "Comedic Performer", "Comedic Performer" },
 	{ "Comedic Performer", "Comedic Performer" },
 	{ "Romantic Performer", "Romantic Performer" },
 	{ "Romantic Performer", "Romantic Performer" },
 	{ "Epic Performer", "Epic Performer" },
 	{ "Epic Performer", "Epic Performer" },
 	{ "Tragic Performer", "Tragic Performer" },
 	{ "Tragic Performer", "Tragic Performer" },
 	{ "Dancer", "Dancer" },
 	{ "Dancer", "Dancer" },
 	{ "Tumbler", "Tumbler" },
 	{ "Tumbler", "Tumbler" },
 	{ "Acrobat", "Acrobat" },
 	{ "Acrobat", "Acrobat" },
 	{ "Entertainer", "Entertainer" },
 	{ "Entertainer", "Entertainer" },
 	{ "Pacifier", "Pacifier" },
 	{ "Pacifier", "Pacifier" },
 	{ "Enciter", "Enciter" },
 	{ "Enciter", "Enciter" },
 	{ "Entrancer", "Entrancer" },
 	{ "Entrancer", "Entrancer" },
 	{ "Mesmerizer", "Mesmerizer" },
 	{ "Mesmerizer", "Mesmerizer" },
 	{ "Master Entertainer", "Master Entertainer" },
 	{ "Master Entertainer", "Master Entertainer" },
 	{ "Master Pacifier", "Master Pacifier" },
 	{ "Master Pacifier", "Master Pacifier" },
 	{ "Master Enciter", "Master Enciter" },
 	{ "Master Enciter", "Master Enciter" },
 	{ "Master Entrancer", "Master Entrancer" },
 	{ "Master Entrancer", "Master Entrancer" },
 	{ "Master Mesmerizer", "Master Mesmerizer" },
 	{ "Master Mesmerizer", "Master Mesmerizer" },
 	{ "Master of the Fine Arts", "Master of the Fine Arts" },
 	{ "Master of the Fine Arts", "Master of the Fine Arts" },
 	{ "Hero of the Fine Arts", "Heroine of the Fine Arts" },
 	{ "Avatar of the Fine Arts", "Avatar of the Fine Arts" },
 	{ "Angel of the Fine Arts", "Angel of the Fine Arts" },
 	{ "Demigod of the Fine Arts", "Demigoddess of the Fine Arts", },
 	{ "Immortal of the Fine Arts", "Immortal of the Fine Arts" },
 	{ "God of the Fine Arts", "Goddess of the Fine Arts" },
 	{ "Deity of the Fine Arts", "Deity of the Fine Arts" },
 	{ "Supreme Master Troubadour", "Supreme Mistress Troubadour"},
 	{ "Creator", "Creator" },
 	{ "Implementor", "Implementor" } 
   },

   {
 	{ "Man", "Woman" },
 	{ "Farmer", "Farmer" },
 	{ "Farmer", "Farmer" },
 	{ "Sword Pupil", "Sword Pupil" },
 	{ "Sword Pupil", "Sword Pupil" },
 	{ "Junior Messenger Boy", "Junior Messenger Girl" },
 	{ "Junior Messenger Boy", "Junior Messenger Girl" },
 	{ "Messenger Boy", "Messenger Girl" },
 	{ "Messenger Boy", "Messenger Girl" },
 	{ "Junior Page", "Junior Page" },
 	{ "Junior Page", "Junior Page" },
 	{ "Page", "Page" },
 	{ "Page", "Page" },
 	{ "Junior Valet", "Junior Valet" },
 	{ "Junior Valet", "Junior Valet" },
 	{ "Valet", "Valet" },
 	{ "Valet", "Valet" },
 	{ "Junior Envoy", "Junior Envoy" },
 	{ "Junior Envoy", "Junior Envoy" },
 	{ "Envoy", "Lady Envoy" },
 	{ "Envoy", "Lady Envoy" },
 	{ "Apprentice Squire", "Apprentice Squire" },
 	{ "Apprentice Squire", "Apprentice Squire" },
 	{ "Junior Squire", "Junior Squire" },
 	{ "Junior Squire", "Junior Squire" },
 	{ "Squire", "Squire" },
 	{ "Squire", "Squire" },
 	{ "Senior Squire", "Senior Squire" },
 	{ "Senior Squire", "Senior Squire" },
 	{ "Standard Bearer", "Standard Bearer" },
 	{ "Standard Bearer", "Standard Bearer" },
 	{ "Senior Standard Bearer", "Senior Standard Bearer" },
 	{ "Senior Standard Bearer", "Senior Standard Bearer" },
 	{ "Shield Bearer", "Shield Bearer" },
	{ "Shield Bearer", "Shield Bearer" },
 	{ "Senior Shield Bearer", "Senior Shield Bearer" },
 	{ "Senior Shield Bearer", "Senior Shield Bearer" },
 	{ "Foresworn", "Foresworn" },
 	{ "Foresworn", "Foresworn" },
 	{ "Horseman", "Horsewoman" },
 	{ "Horseman", "Horsewoman" },
 	{ "Lancer", "Lancer" },
 	{ "Lancer", "Lancer" },
 	{ "Jouster", "Jouster" },
 	{ "Jouster", "Jouster" },
 	{ "Cavalier", "Cavalier" },
 	{ "Cavalier", "Cavalier" },
 	{ "Equerry", "Equerry" },
 	{ "Equerry", "Equerry" },
 	{ "Knight", "Knight" },
 	{ "Knight", "Knight" },
 	{ "Elder Knight", "Elder Knight" },
 	{ "Elder Knight", "Elder Knight" },
 	{ "High Knight", "High Knight" },
 	{ "High Knight", "High Knight" },
 	{ "Lord Knight", "Lady Knight" },
 	{ "Lord Knight", "Lady Knight" },
 	{ "Stronghold Knight", "Stronghold Knight" },
 	{ "Stronghold Knight", "Stronghold Knight" },
 	{ "Protector", "Protector" },
 	{ "Protector", "Protector" },
 	{ "Knight Protector", "Lady Knight Protector" },
 	{ "Knight Protector", "Lady Knight Protector" },
 	{ "Guardian", "Guardian" },
 	{ "Guardian", "Guardian" },
 	{ "Elder Guardian", "Elder Guardian" },
 	{ "Elder Guardian", "Elder Guardian" },
 	{ "Divinate", "Divinate" },
 	{ "Divinate", "Divinate" },
 	{ "Knight Errant", "Lady Knight Errant" },
 	{ "Knight Errant", "Lady Knight Errant" },
 	{ "Ghosthunter", "Ghosthunter" },
 	{ "Ghosthunter", "Ghosthunter" },
 	{ "Wyrmslayer", "Wyrmslayer" },
 	{ "Wyrmslayer", "Wyrmslayer" },
 	{ "Inquisitor", "Inquisitor" },
 	{ "Inquisitor", "Inquisitor" },
 	{ "Chosen One", "Chosen One" },
 	{ "Chosen One", "Chosen One" },
 	{ "Oathbound", "Oathbound" },
 	{ "Oathbound", "Oathbound" },
 	{ "Bringer of Hope", "Bringer of Hope" },
 	{ "Bringer of Hope", "Bringer of Hope" },
 	{ "Bringer of Joy", "Bringer of Joy" },
 	{ "Bringer of Joy", "Bringer of Joy" },
 	{ "Bringer of Innocence", "Bringer of Innocence" },
 	{ "Bringer of Innocence", "Bringer of Innocence" },
 	{ "Bringer of the Flame", "Bringer of the Flame" },
 	{ "Bringer of the Flame", "Bringer of the Flame" },
 	{ "Bringer of the Light", "Bringer of the Light" },
 	{ "Bringer of the Light", "Bringer of the Light" },
 	{ "Guardian of Hope", "Guardian of Hope" },
 	{ "Guardian of Hope", "Guardian of Hope" },
 	{ "Guardian of Joy", "Guardian of Joy" },
 	{ "Guardian of Joy", "Guardian of Joy" },
 	{ "Guardian of Innocence", "Guardian of Innocence" },
 	{ "Guardian of Innocence", "Guardian of Innocence" },
 	{ "Guardian of the Flame", "Guardian of the Flame" },
 	{ "Guardian of the Flame", "Guardian of the Flame" },
 	{ "Guardian of the Sword", "Guardian of the Sword" },
 	{ "Guardian of the Sword", "Guardian of the Sword" },
 	{ "Hero of the Sword", "Heroine of the Sword" },
 	{ "Avatar of the Sword", "Avatar of the Sword" },
 	{ "Angel of the Sword", "Angel of the Sword" },
 	{ "Demigod of the Sword", "Demigoddess of the Sword" },
 	{ "Immortal of the Sword", "Immortal of the Sword" },
 	{ "God of the Sword", "Goddess of the Sword" },
 	{ "Deity of the Sword", "Deity of the Sword" },
 	{ "Supreme Master of the Sword", "Supreme Mistress of the Sword" },
 	{ "Creator", "Creator" },
 	{ "Implementor", "Implementor" }
   }, 
   {
 	{ "Man", "Woman" },
 	{ "Rude", "Rude" },
 	{ "Wild", "Wild" },
 	{ "Blunt", "Blunt" },
 	{ "Rough", "Rough" },
 	{ "Harsh", "Harsh" },
 	{ "Noisy", "Noisy" },
	{ "Berserker", "Berserker" },
	{ "Reckless", "Reckless" },
	{ "Subduer", "Subduer" },
	{ "Adroit Butcher", "Adroit Butcher" },
	{ "Uncultivated", "Uncultivated" },
	{ "Undomesticated", "Undomesticated" },
	{ "Audacious", "Audacious" },
	{ "Bewildered", "Bewildered" },
	{ "Uncontrollable", "Uncontrollable" },
	{ "Furious", "Furious" },
	{ "Brutal", "Brutal" },
	{ "Barbaric", "Barbaric" },
	{ "Outlandish", "Outlandish" },
	{ "Hunnish", "Hunnish" },
	{ "Extravagant", "Extravagant" },
	{ "Unruly", "Unruly" },
	{ "Destructive", "Destructive" },
	{ "Bestial", "Bestial" },
	{ "Blood Thirsty", "Blood Thirsty" },
	{ "Wild Beast", "Wild Beast" },
 	{ "Primitive", "Primitive" },
 	{ "Gruff", "Gruff" },
 	{ "Craggy", "Craggy" },
 	{ "Rugged", "Rugged" },
 	{ "Rowdy", "Rowdy" },
 	{ "Coarse", "Coarse" },
 	{ "Riotous", "Riotous" },
 	{ "Brusque", "Brusque" },
 	{ "Savage", "Savage" },
 	{ "Churlish", "Churlish" },
 	{ "Boisterous", "Boisterous" },
 	{ "Primeval", "Primeval" },
 	{ "Unkempt", "Unkempt" },
 	{ "Barbarian", "Barbarian" },
 	{ "Unwrought", "Unwrought" },
 	{ "Unrefined", "Unrefined" },
 	{ "Unpolished", "Unpolished" },
 	{ "Unrestrained", "Unrestrained" },
 	{ "Uncivilized", "Uncivilized" },
 	{ "Rough-Hewn", "Rough-Hewn" },
 	{ "Fierce", "Fierce" },
 	{ "Uproarious", "Uproarious" },
 	{ "Ferocious", "Ferocious" },
 	{ "Raging Barbarian", "Raging Barbarian" },
 	{ "Untamed", "Untamed" },
 	{ "Native", "Native" },
 	{ "Voracious", "Voracious" },
 	{ "Unrelenting", "Unrelenting" },
 	{ "Inhumane Butcher", "Inhumane Butcher" },
 	{ "Archaic", "Archaic" },
 	{ "Gothic", "Gothic" },
 	{ "Ill-Tempered", "Ill-Tempered" },
 	{ "Vulgar", "Vulgar" },
 	{ "Bringer of Death", "Bringer of Death" },
 	{ "Initiate of Weapons", "Initiate of Weapons" },
 	{ "Master of Weapons", "Master of Weapons" },
 	{ "Master of Weapons", "Master of Weapons" },
 	{ "Keeper of Grasslands", "Keeper of Grasslands" },
 	{ "Primal", "Primal" },
 	{ "Primal Warrior", "Primal Warrior" },
 	{ "Primal Destructor", "Primal Destructor" },
 	{ "Unkempt Beast", "Unkempt Beast" },
 	{ "Sword King", "Sword Queen" },
 	{ "Tribal Guardian", "Tribal Guardian" },
 	{ "Tribal Sentinel", "Tribal Sentinel" },
 	{ "Tribal Leader", "Tribal Leader" },
 	{ "Seeker of Blood", "Seeker of Blood" },
 	{ "Seeker of Pain", "Seeker of Pain" },
 	{ "Seeker of Death", "Seeker of Death" },
 	{ "Master of Destruction", "Master of Destruction" },
 	{ "King of Destruction", "Queen of Destruction" },
 	{ "Relentless Silencer", "Relentless Silencer" },
 	{ "Contentious Slayer", "Contentious Slayer" },
 	{ "Lord Barbarian", "Lady Barbarian" },
 	{ "Initiate of the Brawl", "Initiate of the Brawl" },
 	{ "Initiate of the Fray", "Initiate of the Fray" },
 	{ "Initiate of the Fracas", "Initiate of the Fracas" },
 	{ "Initiate of the Charge", "Initiate of the Charge" },
 	{ "Initiate of the Melee", "Initiate of the Melee" },
 	{ "Master of the Brawl", "Mistress of the Brawl" },
 	{ "Master of the Fray", "Mistress of the Fray" },
 	{ "Master of the Fracas", "Mistress of the Fracas" },
 	{ "Master of the Charge", "Mistress of the Charge" },
 	{ "Master of the Melee", "Mistress of the Melee" },
 	{ "Initiate of Hand-to-Hand", "Initiate of Hand-to-Hand" },
 	{ "Initiate of Ground Fighting","Initiate of Ground Fighting" },
 	{ "Initiate of Brute Strength", "Initiate of Brute Strength" },
 	{ "Initiate of Combat Prowess", "Initiate of Combat Prowess" },
 	{ "Initiate of Pure Battle", "Initiate of Pure Battle" },
 	{ "Master of Hand-to-Hand", "Mistress of Hand-to-Hand" },
 	{ "Master of Ground Fighting", "Mistress of Ground Fighting" },
 	{ "Master of Brute Strength", "Mistress of Brute Strength" },
 	{ "Master of Combat Prowess", "Mistress of Combat Prowess" },
 	{ "Master of Pure Battle", "Mistress of Pure Battle" },
 	{ "Hero of Pure Battle", "Heroine of Pure Battle" },
 	{ "Avatar of Pure Battle", "Avatar of Pure Battle" },
 	{ "Angel of Pure Battle", "Angel of Pure Battle" },
 	{ "Demigod of Pure Battle", "Demigoddess of Pure Battle", },
 	{ "Immortal of Pure Battle", "Immortal of Pure Battle" },
 	{ "God of Pure Battle", "Goddess of Pure Battle" },
 	{ "Deity of Pure Battle", "Deity of Pure Battle" },
 	{ "Supreme Barbarian Master", "Supreme Barbarian Mistress" },
 	{ "Creator", "Creator" },
 	{ "Implementor", "Implementor" }
   }, 

   {
 	{ "Man", "Woman" },
 	{ "Gardener", "Gardener" },
 	{ "Cultivator", "Cultivator" },
 	{ "Horticulturist", "Horticulturist" },
 	{ "Herbalist", "Herbalist" },
 	{ "Shepard", "Shepard" },
 	{ "Naturalist", "Naturalist"},
 	{ "Elementalist", "Elementalist"},
 	{ "Servant of Nature", "Servant of Nature"},
 	{ "Sage", "Sage"},
 	{ "Savant", "Savant"},
 	{ "Shaman", "Shaman"},
 	{ "Forest Dweller", "Forest Dweller"},
 	{ "Pine Spirit", "Pine Spirit"},
 	{ "Ash Spirit", "Ash Spirit"},
 	{ "Oak Spirit", "Oak Spirit"},
 	{ "Forestwalker", "Forestwalker"},
 	{ "Forestwalker", "Forestwalker"},
 	{ "Islandwalker", "Islandwalker"},
 	{ "Islandwalker", "Islandwalker"},
 	{ "Plainswalker", "Plainswalker"},
 	{ "Plainswalker", "Plainswalker"},
 	{ "Desertwalker", "Desertwalker"},
 	{ "Desertwalker", "Desertwalker"},
 	{ "Swampwalker", "Swampwalker"},
 	{ "Swampwalker", "Swampwalker"},
 	{ "Junglewalker", "Junglewalker"},
 	{ "Junglewalker", "Junglewalker"},
 	{ "Windwalker", "Windwalker"},
 	{ "Windwalker", "Windwalker"},
 	{ "Earth Spirit", "Earth Spirit"},
 	{ "Snow Spirit", "Snow Spirit"},
 	{ "Druid", "Druid"},
 	{ "Druid", "Druid"},
 	{ "Acolyte of the Flora", "Acolyte of the Flora"},
 	{ "Acolyte of the Flora", "Acolyte of the Flora"},
 	{ "Acolyte of the Fauna", "Acolyte of the Fauna"},
 	{ "Acolyte of the Fauna", "Acolyte of the Fauna"},
 	{ "Beastcharmer", "Beastcharmer"},
 	{ "Hierophant", "Hierophant"},
 	{ "Master Druid", "Master Druid"},
 	{ "High Druid", "High Druid"},
 	{ "Archdruid", "Archdruid"},
 	{ "Great Druid", "Great Druid"},
 	{ "Woodland Lord", "Woodland Lady"},
 	{ "Spirit of the Grasslands", "Spirit of the Grasslands"},
 	{ "Spirit of the Hills", "Spirit of the Hills"},
 	{ "Spirit of the Springs", "Spirit of the Springs"},
 	{ "Spirit of the Wilderness", "Spirit of the Wilderness"},
 	{ "Speaker of the Grasslands", "Speaker of the Grasslands"},
 	{ "Speaker of the Hills", "Speaker of the Hills"},
 	{ "Speaker of the Springs", "Speaker of the Springs"},
 	{ "Speaker of the Wilderness", "Speaker of the Wilderness"},
 	{ "Guardian of the Grasslands", "Guardian of the Grasslands"},
 	{ "Guardian of the Hills", "Guardian of the Hills"},
 	{ "Guardian of the Springs", "Guardian of the Springs"},
 	{ "Guardian of the Wilderness", "Guardian of the Wilderness"},
 	{ "Caller of Lightning", "Caller fo Lightning"},
 	{ "Diverter of Lightning", "Diverter of Lightning"},
 	{ "Aspirant", "Aspirant"},
 	{ "Ovate", "Ovate"},
 	{ "Raindancer", "Raindancer"},
 	{ "Raindancer", "Raindancer"},
 	{ "Moondancer", "Moondancer"},
 	{ "Moondancer", "Moondancer"},
 	{ "Stardancer", "Stardancer"},
 	{ "Stardancer", "Stardancer"},
 	{ "Keeper of Fire", "Keeper of Fire"},
 	{ "Keeper of Fire", "Keeper of Fire"},
 	{ "Keeper of Water", "Keeper of Water"},
 	{ "Keeper of Water", "Keeper of Water"},
 	{ "Earthshaper", "Earthshaper"},
 	{ "Earthshaper", "Earthshaper"},
 	{ "Seashaper", "Seashaper"},
 	{ "Seashaper", "Seashaper"},
 	{ "Windshaper", "Windshaper"},
 	{ "Windshaper", "Windshaper"},
 	{ "Skyshaper", "Skyshaper"},
 	{ "Skyshaper", "Skyshaper"},
 	{ "Rainshaper", "Rainshaper"},
 	{ "Rainshaper", "Rainshaper"},
 	{ "Guardian of Earth", "Guardian of Earth"},
 	{ "Guardian of Earth", "Guardian of Earth"},
 	{ "Guardian of Sea", "Guardian of Sea" },
 	{ "Guardian of Sea", "Guardian of Sea" },
 	{ "Guardian of Wind", "Guardian of Wind" },
 	{ "Guardian of Wind", "Guardian of Wind" },
 	{ "Guardian of Sky", "Guardian of Sky" },
 	{ "Guardian of Sky", "Guardian of Sky" },
 	{ "Guardian of Rain", "Guardian of Rain "},
 	{ "Guardian of Rain", "Guardian of Rain "},
 	{ "Speaker of Names", "Speaker of Names" },
 	{ "Speaker of Names", "Speaker of Names" },
 	{ "Father of the Storm", "Mother of the Storm"},
 	{ "Father of the Storm", "Mother of the Storm"},
 	{ "Bringer of the Storm", "Bringer of the Storm"},
 	{ "Bringer of the Storm", "Bringer of the Storm"},
 	{ "Soother of the Storm", "Soother of the Storm"},
 	{ "Soother of the Storm", "Soother of the Storm"},
 	{ "Keeper of the Wild","Keeper of the Wild"},
 	{ "Hero of the Wild Places", "Hero of the Wild Places"},
 	{ "Avatar", "Avatar" },
 	{ "Angel", "Angel" },
 	{ "Demigod", "Demigoddess", },
 	{ "Immortal", "Immortal" },
 	{ "God", "Goddess" },
 	{ "Deity", "Deity" },
 	{ "Supreme Master", "Supreme Mistress" },
 	{ "Creator", "Creator" },
 	{ "Implementor", "Implementor" } 
   }, 

   {
 	{ "Man", "Woman" },
 	{ "Forest Recruit", "Forest Recruit", },
 	{ "Strider", "Strider", },
 	{ "Strider", "Strider", },
 	{ "Forest Pupil", "Forest Pupil" },
 	{ "Forest Pupil", "Forest Pupil" },
 	{ "Forester", "Forester" },
 	{ "Forester", "Forester" },
 	{ "Master Forester", "Master Forester" },
 	{ "Master Forester", "Master Forester" },
 	{ "Trapper", "Trapper" },
 	{ "Trapper", "Trapper" },
 	{ "Guide", "Guide" },
 	{ "Guide", "Guide" },
 	{ "Master Trapper", "Master Trapper" },
 	{ "Master Trapper", "Master Trapper" },
 	{ "Hunter", "Huntress" },
 	{ "Hunter", "Huntress" },
 	{ "Master Hunter", "Master Hunter" },
 	{ "Master Hunter", "Master Hunter" },
 	{ "Tracker", "Tracker" },
 	{ "Tracker", "Tracker" },
 	{ "Master Tracker", "Master Tracker" },
 	{ "Master Tracker", "Master Tracker" },
 	{ "Scout", "Scout" },
 	{ "Scout", "Scout" },
 	{ "Master Scout", "Mistress Scout" },
 	{ "Master Scout", "Mistress Scout" },
 	{ "Courser", "Courser" },
 	{ "Courser", "Courser" },
 	{ "Green Man", "Green Woman" },
 	{ "Green Man", "Green Woman" },
 	{ "Lesser Woodsman", "Lesser Woodsman" },
 	{ "Lesser Woodsman", "Lesser Woodsman" },
 	{ "Woodsman", "Woodswoman" },
 	{ "Woodsman", "Woodswoman" },
 	{ "Pathfinder", "Pathfinder" },
 	{ "Pathfinder", "Pathfinder" },
 	{ "Master Woodsman", "Master Woodswoman" },
 	{ "Master Woodsman", "Master Woodswoman" },
 	{ "Ranger Initiate", "Ranger Initiate" },
 	{ "Ranger Initiate", "Ranger Initiate" },
 	{ "Ranger Candidate", "Ranger Candidate" },
 	{ "Ranger Squire", "Ranger Squire" },
 	{ "Ranger", "Ranger" },
 	{ "Master Ranger", "Mistress Ranger" },
 	{ "Ranger Captain", "Ranger Captain" },
 	{ "Ranger Lord", "Ranger Lady" },
 	{ "Forestwalker", "Forestwalker" },
 	{ "Master Forestwalker", "Master Forestwalker" },
 	{ "Warder", "Warder" },
 	{ "Warder Captain", "Warder Captain" },
 	{ "Warder General", "Warder General" },
 	{ "Master of Warders", "Mistress of Warders" },
 	{ "Sword of the Forest", "Sword of the Forest" },
 	{ "Knight of the Forest", "Knight of the Forest" },
 	{ "Knight of the Forest", "Knight of the Forest" },
 	{ "Guardian of the Forest", "Guardian of the Forest" },
 	{ "Guardian of the Forest", "Guardian of the Forest" },
 	{ "Lord of the Forest", "Lady of the Forest" },
 	{ "Lord of the Forest", "Lady of the Forest" },
 	{ "Overlord of the Forest", "Overlady of the Forest" },
 	{ "Overlord of the Forest", "Overlady of the Forest" },
 	{ "Baron of the Forest", "Baroness of the Forest" },
 	{ "Baron of the Forest", "Baroness of the Forest" },
 	{ "Master of the Forest", "Master of the Forest" },
 	{ "Master of the Forest", "Master of the Forest" },
 	{ "Huntsman", "Huntswoman" },
 	{ "Huntsman", "Huntswoman" },
 	{ "Caller of the Pack", "Caller of the Pack" },
 	{ "Caller of the Pack", "Caller of the Pack" },
 	{ "Master of the Pack", "Mistress of the Pack" },
 	{ "Master of the Pack", "Mistress of the Pack" },
 	{ "Sylvan Knight Initiate", "Sylvan Knight Initiate" },
 	{ "Sylvan Knight Initiate", "Sylvan Knight Initiate" },
 	{ "Sylvan Knight", "Sylvan Knight" },
 	{ "Sylvan Knight", "Sylvan Knight" },
 	{ "Sylvan Lord", "Sylvan Lady" },
 	{ "Sylvan Lord", "Sylvan Lady" },
 	{ "Sylvan Baron", "Sylvan Baroness" },
 	{ "Sylvan Baron", "Sylvan Baroness" },
 	{ "Sylvan Prince", "Sylvan Princess" },
 	{ "Sylvan Prince", "Sylvan Princess" },
 	{ "Sylvan King", "Sylvan Queen" },
 	{ "Sylvan King", "Sylvan Queen" },
 	{ "Master Hunter", "Master Hunter" },
 	{ "Master Hunter", "Master Hunter" },
 	{ "Warder of the Glade", "Warder of the Glade" },
 	{ "Warder of the Glade", "Warder of the Glade" },
 	{ "Keeper of the Glade", "Keeper of the Glade" },
 	{ "Keeper of the Glade", "Keeper of the Glade" },
 	{ "Knight of the Glade", "Knight of the Glade" },
 	{ "Knight of the Glade", "Knight of the Glade" },
 	{ "Master of the Glade", "Mistress of the Glade" },
 	{ "Master of the Glade", "Mistress of the Glade" },
 	{ "King of the Glade", "Queen of the Glade" },
 	{ "King of the Glade", "Queen of the Glade" },
 	{ "Watcher of the Glade", "Watcher of the Glade" },
 	{ "Watcher of the Glade", "Watcher of the Glade" },
 	{ "Forest King", "Forest Queen" },
 	{ "Avatar of the Glade", "Avatar of the Glade" },
 	{ "Angel of the Glade", "Angel of the Glade" },
 	{ "Demigod of the Glade", "Demigoddess of the Glade", },
 	{ "Immortal of the Glade", "Immortal of the Glade" },
 	{ "God of the Glade", "Goddess of the Glade" },
 	{ "Deity of the Glade", "Deity of the Glade" },
 	{ "Supreme Master of the Glade","Supreme Mistress of the Glade" },
 	{ "Creator", "Creator" },
 	{ "Implementor", "Implementor" } 
   },

   {
 	{ "Man", "Woman" },
 	{ "Sturdy", "Sturdy" },
 	{ "Sturdy", "Sturdy" },
 	{ "Healthy", "Healthy" },
 	{ "Healthy", "Healthy" },
 	{ "Toned", "Toned" },
 	{ "Toned", "Toned" },
 	{ "Muscled", "Muscled" },
 	{ "Muscled", "Muscled" },
 	{ "Sleek", "Sleek" },
 	{ "Sleek", "Sleek" },
 	{ "Active", "Active" },
 	{ "Active", "Active" },
 	{ "Curious", "Curious" },
 	{ "Curious", "Curious" },
 	{ "Dangerous", "Dangerous" },
 	{ "Dangerous", "Dangerous" },
 	{ "Actor", "Actor" },
 	{ "Actor", "Actor" },
 	{ "Faceless", "Faceless" },
 	{ "Faceless", "Faceless" },
 	{ "Learner of Stealth", "Learner of Stealth"},
 	{ "Learner of Stealth", "Learner of Stealth"},
 	{ "Learner of Blades", "Learner of Blades"},
 	{ "Learner of Blades", "Learner of Blades"},
 	{ "Learner of Combat", "Learner of Combat" },
 	{ "Learner of Combat", "Learner of Combat" },
 	{ "Learner of Poison", "Learner of Poison"},
 	{ "Learner of Poison", "Learner of Poison"},
 	{ "Learner of Assassination", "Learner of Assassination"},
 	{ "Learner of Assassination", "Learner of Assassination"},
 	{ "Neophyte Assassin", "Neophyte Assassin"},
 	{ "Neophyte Assassin", "Neophyte Assassin"},
 	{ "Initiate Assassin", "Initiate Assassin"},
 	{ "Initiate Assassin", "Initiate Assassin"},
 	{ "Younger Assassin", "Younger Assassin"},
 	{ "Younger Assassin", "Younger Assassin"},
 	{ "Lesser Assassin", "Lesser Assassin"},
 	{ "Lesser Assassin", "Lesser Assassin"},
 	{ "Assassin", "Assassin"},
 	{ "Assassin", "Assassin"},
 	{ "Greater Assassin", "Greater Assassin"},
 	{ "Greater Assassin", "Greater Assassin"},
 	{ "Elder Assassin", "Elder Assassin" },
 	{ "Elder Assassin", "Elder Assassin" },
 	{ "Learned Assassin", "Learned Assassin"},
 	{ "Learned Assassin", "Learned Assassin"},
 	{ "Master Assassin", "Master Assassin"},
 	{ "Master Assassin", "Master Assassin"},
 	{ "Assassin of Many Voices", "Assassin of Many Voices"},
 	{ "Assassin of Many Voices", "Assassin of Many Voices"},
 	{ "Speaker of the Weak", "Speaker of the Weak"},
 	{ "Speaker of the Weak", "Speaker of the Weak"},
 	{ "Spirit of the Strong", "Spirit of the Strong"},
 	{ "Spirit of the Strong", "Spirit of the Strong"},
 	{ "Guardian of the Forgotten", "Guardian of the Forgotten"},
 	{ "Guardian of the Forgotten", "Guardian of the Forgotten"},
 	{ "Spirit of the Proud", "Spirit of the Proud"},
 	{ "Spirit of the Proud", "Spirit of the Proud"},
 	{ "Assassin of Many Faces", "Assassin of Many Faces"},
 	{ "Assassin of Many Faces", "Assassin of Many Faces"},
 	{ "Overlooked Son", "Overlooked Daughter"},
 	{ "Overlooked Son", "Overlooked Daughter"},
 	{ "Ancient Living Again", "Ancient Living Again"},
 	{ "Ancient Living Again", "Ancient Living Again"},
 	{ "Echo of Another Time", "Echo of Another Time"},
 	{ "Echo of Another Time", "Echo of Another Time"},
 	{ "Keeper of Other Values", "Keeper of Other Values"},
 	{ "Keeper of Other Values", "Keeper of Other Values"},
 	{ "Assassin of Many Races", "Assassin of Many Races"},
 	{ "Assassin of Many Races", "Assassin of Many Races"},
 	{ "Knife in Your Back", "Knife in Your Back"},
 	{ "Knife in Your Back", "Knife in Your Back"},
 	{ "Lover in Your Bed", "Lover in your Bed"},
 	{ "Lover in Your Bed", "Lover in your Bed"},
 	{ "Cry of a Sightless Man", "Cry of a Sightless Man"},
 	{ "Cry of a Sightless Man", "Cry of a Sightless Man"},
 	{ "Kiss of a Mute Woman", "Kiss of a Mute Woman"},
 	{ "Kiss of a Mute Woman", "Kiss of a Mute Woman"},
 	{ "Assassin of Many Lives", "Assassin of Many Lives"},
 	{ "Assassin of Many Lives", "Assassin of Many Lives"},
 	{ "Forgotten Soul", "Forgotten Soul" },
 	{ "Forgotten Soul", "Forgotten Soul" },
 	{ "Man in Black", "Woman in Black" },
 	{ "Man in Black", "Woman in Black" },
 	{ "Silent Wind", "Silent Wind" },
 	{ "Silent Wind", "Silent Wind" },
 	{ "Cheater of Death", "Cheater of Death" },
 	{ "Cheater of Death", "Cheater of Death" },
 	{ "Snatcher of Life", "Snatcher of Life "},
 	{ "Snatcher of Life", "Snatcher of Life "},
 	{ "Figure in the Shadows", "Figure in the Shadows" },
 	{ "Figure in the Shadows", "Figure in the Shadows" },
 	{ "Relative No One Knows", "Relative No One Knows"},
 	{ "Relative No One Knows", "Relative No One Knows"},
 	{ "Stranger in the House", "Stranger in the House"},
 	{ "Stranger in the House", "Stranger in the House"},
 	{ "Lurker in the Dark", "Lurker in the Dark"},
 	{ "Lurker in the Dark", "Lurker in the Dark"},
 	{ "Whisper in the Night", "Whisper in the Night"},
 	{ "Whisper in the Night", "Whisper in the Night"},
 	{ "Lord Assassin of the Night", "Lady Assassin of the Night"},
 	{ "Avatar", "Avatar" },
 	{ "Angel", "Angel" },
 	{ "Demigod", "Demigoddess", },
 	{ "Immortal", "Immortal" },
 	{ "God", "Goddess" },
 	{ "Deity", "Deity" },
 	{ "Supreme Master", "Supreme Mistress" },
 	{ "Creator", "Creator" },
 	{ "Implementor", "Implementor" } 
   },

   {
 	{ "Man", "Woman", },
 	{ "Street Urchin", "Street Urchin" },
 	{ "Street Urchin", "Street Urchin" },
 	{ "Beggar", "Beggar", },
 	{ "Beggar", "Beggar", },
 	{ "Drifter", "Drifter", },
 	{ "Drifter", "Drifter", },
 	{ "Vagabond", "Vagabond", },
 	{ "Vagabond", "Vagabond", },
 	{ "Wanderer", "Wanderer", },
 	{ "Wanderer", "Wanderer", },
 	{ "Outcast", "Outcast", },
 	{ "Outcast", "Outcast", },
 	{ "Nomad", "Nomad", },
 	{ "Nomad", "Nomad", },
 	{ "Pariah", "Pariah", },
 	{ "Pariah", "Pariah", },
 	{ "Crafter", "Crafter", },
 	{ "Crafter", "Crafter", },
 	{ "Handler", "Handler" },
 	{ "Handler", "Handler" },
 	{ "Celeritous", "Celeritous" },
 	{ "Celeritous", "Celeritous" },
 	{ "Quick-Witted", "Quick-Witted", },
 	{ "Quick-Witted", "Quick-Witted", },
 	{ "Swiftminded", "Swiftminded", },
 	{ "Swiftminded", "Swiftminded", },
 	{ "Dazzler", "Dazzler", },
 	{ "Dazzler", "Dazzler", },
 	{ "Gypsy", "Gypsy", },
 	{ "Gypsy", "Gypsy", },
 	{ "Spell Scavenger", "Spell Scavenger", },
 	{ "Spell Scavenger", "Spell Scavenger", },
 	{ "Bewitched", "Bewitched", },
 	{ "Bewitched", "Bewitched", },
 	{ "Dervish", "Dervish", },
 	{ "Dervish", "Dervish", },
 	{ "Dabbler of Magic", "Dabbler of Magic", },
 	{ "Dabbler of Magic", "Dabbler of Magic", },
 	{ "Spellfilcher", "Spellfilcher", },
 	{ "Spellfilcher", "Spellfilcher", },
 	{ "Initiate of Shadows", "Initiate of Shadows", },
 	{ "Initiate of Shadows", "Initiate of Shadows", },
 	{ "Shadowdweller", "Shadowdweller", },
 	{ "Shadowdweller", "Shadowdweller", },
 	{ "Seeker of Shadows", "Seeker of Shadows", },
 	{ "Seeker of Shadows", "Seeker of Shadows", },
 	{ "Shadowcaster", "Shadowcaster", },
 	{ "Shadowcaster", "Shadowcaster", },
 	{ "Master Shadowcaster", "Mistress Shadowcaster", },
 	{ "Master Shadowcaster", "Mistress Shadowcaster", },
 	{ "Seeker of Talismans", "Seeker of Talismans", },
 	{ "Seeker of Talismans", "Seeker of Talismans", },
 	{ "Handler of Wealth", "Handler of Wealth", },
 	{ "Handler of Wealth", "Handler of Wealth", },
 	{ "Appraiser of Gold", "Appraiser of Gold", },
 	{ "Appraiser of Gold", "Appraiser of Gold", },
 	{ "Collector of Artifacts", "Collector of Artifacts", },
 	{ "Collector of Artifacts", "Collector of Artifacts", },
 	{ "Greater Spellfilcher", "Greater Spellfilcher", },
 	{ "Greater Spellfilcher", "Greater Spellfilcher", },
 	{ "Dilettante of Illusions", "Dilettante of Illusions", },
 	{ "Dilettante of Illusions", "Dilettante of Illusions", },
 	{ "Crafter of Echoes", "Crafter of Echoes", },
 	{ "Crafter of Echoes", "Crafter of Echoes", },
 	{ "Dabbler of Cantrips", "Dabbler of Cantrips", },
 	{ "Dabbler of Cantrips", "Dabbler of Cantrips", },
 	{ "Initiate of Whispers", "Initiate of Whispers", },
 	{ "Initiate of Whispers", "Initiate of Whispers", },
 	{ "Caster of Deceptions", "Caster of Deceptions", },
 	{ "Caster of Deceptions", "Caster of Deceptions", },
 	{ "Dilettante of Enchantments", "Dilettante of Enchantments", },
 	{ "Dilettante of Enchantments", "Dilettante of Enchantments", },
 	{ "Bringer of Swiftness", "Bringer of Swiftness", },
 	{ "Bringer of Swiftness", "Bringer of Swiftness", },
 	{ "Beholder of Shadows", "Beholder of Shadows", },
 	{ "Beholder of Shadows", "Beholder of Shadows", },
 	{ "Enchanter of Thieves", "Enchantress of Thieves", },
 	{ "Enchanter of Thieves", "Enchantress of Thieves", },
 	{ "Master of the Unseen", "Mistress of the Unseen" },
 	{ "Master of the Unseen", "Mistress of the Unseen" },
 	{ "Seeker of the Unknown", "Seeker of the Unknown", },
 	{ "Seeker of the Unknown", "Seeker of the Unknown", },
 	{ "Warden of Jewels", "Warden of Jewels", },
 	{ "Warden of Jewels", "Warden of Jewels", },
 	{ "Master of Artifacts", "Mistress of Artifacts", },
 	{ "Master of Artifacts", "Mistress of Artifacts", },
 	{ "Possessor of Treasures", "Possessor of Treasures", },
 	{ "Possessor of Treasures", "Possessor of Treasures", },
 	{ "Keeper of the Seven Keys", "Keeper of the Seven Keys", },
 	{ "Keeper of the Seven Keys", "Keeper of the Seven Keys", },
 	{ "Lord of the Ten Bells", "Lady of the Ten Bells", },
 	{ "Lord of the Ten Bells", "Lady of the Ten Bells", },
 	{ "Guardian of Secrets", "Guardian of Secrets", },
 	{ "Guardian of Secrets", "Guardian of Secrets", },
 	{ "Master Spellbinder", "Mistress Spellbinder", },
 	{ "Master Spellbinder", "Mistress Spellbinder", },
 	{ "Master Dazzler", "Mistress Dazzler", },
 	{ "Master Dazzler", "Mistress Dazzler", },
 	{ "Grand Master Spellfilcher", "Grand Mistress Spellfilcher", },
 	{ "Grand Master Spellfilcher", "Grand Mistress Spellfilcher", },
 	{ "Hero of Dazzlers", "Heroine of Dazzlers" },
 	{ "Avatar of Dazzlers", "Avatar of Dazzlers" },
 	{ "Angel of Dazzlers", "Angel of Dazzlers" },
 	{ "Demigod of Dazzlers", "Demigoddess of Dazzlers", },
 	{ "Immortal of Dazzlers", "Immortal of Dazzlers" },
 	{ "God of Dazzlers", "Goddess of Dazzlers" },
 	{ "Deity of Dazzlers", "Deity of Dazzlers" },
 	{ "Supreme Master of Dazzlers", "Supreme Mistress of Dazzlers" },
 	{ "Creator", "Creator" },
 	{ "Implementor", "Implementor" } 
   },

   {
 	{ "Man", "Woman" },
 	{ "Scum", "Scum" },
 	{ "Scum", "Scum" },
 	{ "Brute", "Brute" },
 	{ "Brute", "Brute" },
 	{ "Bully", "Bully" },
 	{ "Bully", "Bully" },
 	{ "Bastard", "Bastard" },
 	{ "Bastard", "Bastard" },
 	{ "Cheating Slime", "Cheating Slime" },
 	{ "Cheating Slime", "Cheating Slime" },
 	{ "Villian", "Villian" },
 	{ "Villian", "Villian" },
 	{ "Infamous", "Infamous" },
 	{ "Infamous", "Infamous" },
 	{ "Murderous", "Murderous" },
 	{ "Murderous", "Murderous" },
 	{ "Dark", "Dark" },
 	{ "Dark", "Dark" },
 	{ "Evil", "Evil" },
 	{ "Evil", "Evil" },
 	{ "Saboteur", "Saboteur" },
 	{ "Saboteur", "Saboteur" },
 	{ "Hated", "Hated" },
 	{ "Hated", "Hated" },
 	{ "Wrecker", "Wrecker" },
 	{ "Wrecker", "Wrecker" },
 	{ "Cruel", "Cruel" },
 	{ "Cruel", "Cruel" },
 	{ "Dark Lord", "Dark Lady" },
 	{ "Dark Lord", "Dark Lady" },
 	{ "Black Sword" , "Black Sword" },
 	{ "Black Sword" , "Black Sword" },
 	{ "Crimson Sword", "Crimson Sword" },
 	{ "Crimson Sword", "Crimson Sword" },
 	{ "Black Knight", "Black Lady" },
 	{ "Black Knight", "Black Lady" },
 	{ "Crimson Knight", "Crimson Lady" },
 	{ "Crimson Knight", "Crimson Lady" },
 	{ "Avernon", "Avernon" },
 	{ "Avernon", "Avernon" },
 	{ "Knight of Evil", "Lady of Evil" },
 	{ "Knight of Evil", "Lady of Evil" },
 	{ "Knight of Pain", "Lady of Pain" },
 	{ "Knight of Pain", "Lady of Pain" },
 	{ "Knight of Suffering", "Lady of Suffering" },
 	{ "Knight of Suffering", "Lady of Suffering" },
 	{ "Knight of Despair", "Lady of Despair" },
 	{ "Knight of Despair", "Lady of Despair" },
 	{ "Knight of the Dark Mantle", "Lady of the Dark Mantle" },
 	{ "Knight of the Dark Mantle", "Lady of the Dark Mantle" },
 	{ "Obliterator of Altruism", "Obliterator of Altruism" },
 	{ "Obliterator of Altruism", "Obliterator of Altruism" },
 	{ "Decimator of Villages", "Decimator of Villages" },
 	{ "Decimator of Villages", "Decimator of Villages" },
 	{ "Pulverizer of Towns", "Pulverizer of Towns" },
 	{ "Pulverizer of Towns", "Pulverizer of Towns" },
 	{ "Spoiler of Purity", "Spoiler of Purity" },
 	{ "Spoiler of Purity", "Spoiler of Purity" },
 	{ "Ravager of Virgins", "Ravager of Virgins" },
 	{ "Ravager of Virgins", "Ravager of Virgins" },
 	{ "Mutilator of Babies", "Mutilator of Babies" },
 	{ "Mutilator of Babies", "Mutilator of Babies" },
 	{ "Disintigrator of Trust", "Disintigrator of Trust" },
 	{ "Disintigrator of Trust", "Disintigrator of Trust" },
 	{ "Annihilator of Virtue", "Annihilator of Virtue" },
 	{ "Annihilator of Virtue", "Annihilator of Virtue" },
 	{ "Exterminator of Goodness", "Exterminator of Goodness" },
 	{ "Exterminator of Goodness", "Exterminator of Goodness" },
 	{ "Son of the Darkness", "Daughter of the Darkness" },
 	{ "Son of the Darkness", "Daughter of the Darkness" },
 	{ "Unmaker", "Unmaker" },
 	{ "Unmaker", "Unmaker" },
 	{ "Unmaker", "Unmaker" },
 	{ "Subverter of Good Intentions","Subverter of Good Intentions" },
 	{ "Subverter of Good Intentions","Subverter of Good Intentions" },
 	{ "Subverter of Good Intentions","Subverter of Good Intentions" },
 	{ "Devourer of the Weak-Minded","Devourer of the Weak-Minded" },
 	{ "Devourer of the Weak-Minded","Devourer of the Weak-Minded" },
 	{ "Devourer of the Weak-Minded","Devourer of the Weak-Minded" },
 	{ "Quencher of Hope", "Quencher of Hope" },
 	{ "Quencher of Hope", "Quencher of Hope" },
 	{ "Quencher of Hope", "Quencher of Hope" },
 	{ "Destroyer of the Light", "Destroyer of the Light" },
 	{ "Destroyer of the Light", "Destroyer of the Light" },
 	{ "Destroyer of the Light", "Destroyer of the Light" },
 	{ "Extinguisher of the Flame", "Extinguisher of the Flame" },
 	{ "Extinguisher of the Flame", "Extinguisher of the Flame" },
 	{ "Extinguisher of the Flame", "Extinguisher of the Flame" },
 	{ "Harbringer of Death", "Harbringer of Death" },
 	{ "Harbringer of Death", "Harbringer of Death" },
 	{ "Harbringer of Death", "Harbringer of Death" },
 	{ "Bringer of Famine", "Bringer of Famine" },
 	{ "Bringer of Famine", "Bringer of Famine" },
 	{ "Bringer of Famine", "Bringer of Famine" },
 	{ "Spreader of Pestilence", "Spreader of Pestilence" },
 	{ "Spreader of Pestilence", "Spreader of Pestilence" },
 	{ "Spreader of Pestilence", "Spreader of Pestilence" },
 	{ "Corruptor of the Innocent", "Corruptor of the Innocent" },
 	{ "Corruptor of the Innocent", "Corruptor of the Innocent" },
 	{ "Corruptor of the Innocent", "Corruptor of the Innocent" },
 	{ "Almighty Lord of Darkness", "Almighty Lady of Darkness" },
 	{ "Avatar of Darkness", "Avatar of Darkness" },
 	{ "Angel of Darkness", "Angel of Darkness" },
 	{ "Demigod of Darkness", "Demigoddess of Darkness" },
 	{ "Immortal of Darkness", "Immortal of Darkness" },
 	{ "God of Darkness", "Goddess of Darkness" },
 	{ "Deity of Darkness", "Deity of Darkness" },
 	{ "Supreme Master of Darkness", "Supreme Mistress of Darkness" },
 	{ "Creator", "Creator" },
 	{ "Implementor", "Implementor" } 
   },

   {
 	{ "Man", "Woman" },
 	{ "Apprentice of Magic", "Apprentice of Magic" },
 	{ "Apprentice of Magic", "Apprentice of Magic" },
 	{ "Spell Student", "Spell Student" },
 	{ "Spell Student", "Spell Student" },
 	{ "Scholar of Magic", "Scholar of Magic" },
 	{ "Scholar of Magic", "Scholar of Magic" },
 	{ "Delver in Spells", "Delver in Spells" },
 	{ "Delver in Spells", "Delver in Spells" },
 	{ "Medium of Magic", "Medium of Magic" },
 	{ "Medium of Magic", "Medium of Magic" },
 	{ "Scribe of Magic", "Scribe of Magic" },
 	{ "Scribe of Magic", "Scribe of Magic" },
 	{ "Seer", "Seer" }, { "Sage", "Sage" },
 	{ "Seer", "Seer" }, { "Sage", "Sage" },
 	{ "Abjurer", "Abjurer" },
 	{ "Abjurer", "Abjurer" },
 	{ "Magician", "Magician" },
 	{ "Magician", "Magician" },
 	{ "Hopeful Enchanter", "Hopeful Enchantress" },
 	{ "Hopeful Enchanter", "Hopeful Enchantress" },
 	{ "Student Enchanter", "Student Enchantress" },
 	{ "Student Enchanter", "Student Enchantress" },
 	{ "Novice Enchanter", "Novice Enchantress" },
 	{ "Novice Enchanter", "Novice Enchantress" },
 	{ "Caster of Cantrips", "Caster of Cantrips" },
 	{ "Caster of Cantrips", "Caster of Cantrips" },
 	{ "Lesser Enchanter", "Lesser Enchantress" },
 	{ "Lesser Enchanter", "Lesser Enchantress" },
 	{ "Student of Charm", "Student of Charm" },
 	{ "Student of Charm", "Student of Charm" },
 	{ "Student of Alteration", "Student of Alteration" },
 	{ "Student of Alteration", "Student of Alteration" },
 	{ "Student of Buguiling", "Student of Beguiling" },
 	{ "Student of Buguiling", "Student of Beguiling" },
 	{ "Candidate Enchanter", "Candidate Enchantress" },
 	{ "Candidate Enchanter", "Candidate Enchantress" },
 	{ "Enchanter", "Enchantress" },
 	{ "Enchanter", "Enchantress" },
 	{ "Lesser Charmer", "Lesser Charmer" },
 	{ "Lesser Charmer", "Lesser Charmer" },
 	{ "Charmer", "Charmer" },
 	{ "Charmer", "Charmer" },
 	{ "Beast Charmer", "Beast Charmer" },
 	{ "Beast Charmer", "Beast Charmer" },
 	{ "Greater Charmer", "Greater Charmer" },
 	{ "Greater Charmer", "Greater Charmer" },
 	{ "Dragon Charmer", "Dragon Charmer" },
 	{ "Dragon Charmer", "Dragon Charmer" },
 	{ "Enchanter of Staves", "Enchantress of Staves" },
 	{ "Enchanter of Staves", "Enchantress of Staves" },
 	{ "Enchanter of Wands", "Enchantress of Wands" },
 	{ "Enchanter of Wands", "Enchantress of Wands" },
 	{ "Enchanter of Scrolls", "Enchantress of Scrolls" },
 	{ "Enchanter of Scrolls", "Enchantress of Scrolls" },
 	{ "Enchanter of Blades", "Enchantress of Blades" },
 	{ "Enchanter of Blades", "Enchantress of Blades" },
 	{ "Greater Enchanter", "Greater Enchantress" },
 	{ "Greater Enchanter", "Greater Enchantress" },
 	{ "Beguiler", "Beguiler" },
 	{ "Beguiler", "Beguiler" },
 	{ "Greater Beguiler", "Greater Beguiler" },
 	{ "Greater Beguiler", "Greater Beguiler" },
 	{ "Beguiler of Serpents", "Beguiler of Serpents" },
 	{ "Beguiler of Serpents", "Beguiler of Serpents" },
 	{ "Master Beguiler", "Master Beguiler" },
 	{ "Master Beguiler", "Master Beguiler" },
 	{ "Beguiler of the Stars", "Beguiler of the Stars" },
 	{ "Beguiler of the Stars", "Beguiler of the Stars" },
 	{ "Charmer of the Heavens", "Charmer of the Heavens" },
 	{ "Charmer of the Heavens", "Charmer of the Heavens" },
 	{ "Alterer of the Body", "Alterer of the Body" },
 	{ "Alterer of the Body", "Alterer of the Body" },
 	{ "Alterer of the Realms", "Alterer of the Realms" },
 	{ "Alterer of the Realms", "Alterer of the Realms" },
 	{ "Alterer of the Mind", "Alterer of the Mind" },
 	{ "Alterer of the Mind", "Alterer of the Mind" },
 	{ "Lesser Master Enchanter", "Lesser Master Enchantress" },
 	{ "Lesser Master Enchanter", "Lesser Master Enchantress" },
 	{ "Master Enchanter", "Master Enchantress" },
 	{ "Master Enchanter", "Master Enchantress" },
 	{ "Master of Charm", "Mistress of Charm" },
 	{ "Master of Charm", "Mistress of Charm" },
 	{ "Master of Beguiling", "Mistress of Beguiling" },
 	{ "Master of Beguiling", "Mistress of Beguiling" },
 	{ "Master of Bewitchery", "Mistress of Bewitchery" },
 	{ "Master of Bewitchery", "Mistress of Bewitchery" },
 	{ "Master of Enhancement", "Mistress of Enhancement" },
 	{ "Master of Enhancement", "Mistress of Enhancement" },
 	{ "Master of Alteration", "Mistress of Alteration", },
 	{ "Master of Alteration", "Mistress of Alteration", },
 	{ "Master of Captivation", "Mistress of Captivation" },
 	{ "Master of Captivation", "Mistress of Captivation" },
 	{ "Master of Mind Control", "Mistress of Mind Control" },
 	{ "Master of Mind Control", "Mistress of Mind Control" },
 	{ "Greater Master Enchanter", "Greater Master Enchantress" },
 	{ "Greater Master Enchanter", "Greater Master Enchantress" },
 	{ "High Wizard of Enchantment", "High Mistress of Enchantment" },
 	{ "High Wizard of Enchantment", "High Mistress of Enchantment" },
 	{ "Legendary Enchanter", "Legendary Enchantress" },
 	{ "Avatar", "Avatar of Enchantment" },
 	{ "Angel", "Angel" },
 	{ "Demigod", "Demigoddess" },
 	{ "Immortal Enchanter", "Immortal Enchantress" },
 	{ "God", "Goddess" },
 	{ "Diety", "Diety" },
 	{ "Supremicy", "Supremicy" },
 	{ "Creator", "Creator" },
 	{ "Implementor of Enchantment", "Implementor of Enchantment" } 
}, 

{
 	{ "Man", "Woman" },
 	{ "Apprentice of Magic", "Apprentice of Magic" },
 	{ "Apprentice of Magic", "Apprentice of Magic" },
 	{ "Spell Student", "Spell Student" },
 	{ "Spell Student", "Spell Student" },
 	{ "Scholar of Magic", "Scholar of Magic" },
 	{ "Scholar of Magic", "Scholar of Magic" },
 	{ "Delver in Spells", "Delver in Spells" },
 	{ "Delver in Spells", "Delver in Spells" },
 	{ "Medium of Magic", "Medium of Magic" },
 	{ "Medium of Magic", "Medium of Magic" },
 	{ "Scribe of Magic", "Scribe of Magic" },
 	{ "Scribe of Magic", "Scribe of Magic" },
 	{ "Seer", "Seer" }, { "Sage", "Sage" },
 	{ "Seer", "Seer" }, { "Sage", "Sage" },
 	{ "Abjurer", "Abjurer" },
 	{ "Abjurer", "Abjurer" },
 	{ "Magician", "Magician" },
 	{ "Magician", "Magician" },
 	{ "Hopeful Illusionist", "Hopeful Illusionist" },
 	{ "Hopeful Illusionist", "Hopeful Illusionist" },
 	{ "Student Illusionist", "Student Illusionist" },
 	{ "Student Illusionist", "Student Illusionist" },
 	{ "Novice Illusionist", "Novice Illusionist" },
 	{ "Novice Illusionist", "Novice Illusionist" },
 	{ "Caster of Cantrips", "Caster of Cantrips" },
 	{ "Caster of Cantrips", "Caster of Cantrips" },
 	{ "Lesser Illusionist", "Lesser Illusionist" },
 	{ "Lesser Illusionist", "Lesser Illusionist" },
 	{ "Student of Light", "Student of Light" },
 	{ "Student of Light", "Student of Light" },
 	{ "Student of Sound", "Student of Sound" },
 	{ "Student of Sound", "Student of Sound" },
 	{ "Student of Sight", "Student of Sight" },
 	{ "Student of Sight", "Student of Sight" },
 	{ "Candidate Illusionist", "Candidate Illusionist" },
 	{ "Candidate Illusionist", "Candidate Illusionist" },
 	{ "Illusionist", "Illusionist" },
 	{ "Illusionist", "Illusionist" },
 	{ "Adept of Light", "Adept of Light" },
 	{ "Adept of Light", "Adept of Light" },
 	{ "Caster of Stardust", "Caster of Stardust" },
 	{ "Caster of Stardust", "Caster of Stardust" },
 	{ "Wielder of Moonbeams", "Wielder of Moonbeams" },
 	{ "Wielder of Moonbeams", "Wielder of Moonbeams" },
 	{ "Twister of Sunshine", "Twister of Sunshine" },
 	{ "Twister of Sunshine", "Twister of Sunshine" },
 	{ "Maker of Rainbows", "Maker of Rainbows" },
 	{ "Maker of Rainbows", "Maker of Rainbows" },
 	{ "Trickster of Sound", "Trickster of Sound" },
 	{ "Trickster of Sound", "Trickster of Sound" },
 	{ "Bringer of Whispers", "Bringer of Whispers" },
 	{ "Bringer of Whispers", "Bringer of Whispers" },
 	{ "Cantor of Spells", "Cantor of Spells" },
 	{ "Cantor of Spells", "Cantor of Spells" },
 	{ "Manipulator of Voices", "Manipulator of Voices" },
 	{ "Manipulator of Voices", "Manipulator of Voices" },
 	{ "Keeper of Echoes", "Keeper of Echoes" },
 	{ "Keeper of Echoes", "Keeper of Echoes" },
 	{ "Sorcerer of Sight", "Sorceress of Sight" },
 	{ "Sorcerer of Sight", "Sorceress of Sight" },
 	{ "Adept of Mirrors", "Adept of Mirrors" },
 	{ "Adept of Mirrors", "Adept of Mirrors" },
 	{ "Speaker of Illusions", "Speaker of Illusions" },
 	{ "Speaker of Illusions", "Speaker of Illusions" },
 	{ "Wielder of Images", "Wielder of Images" },
 	{ "Wielder of Images", "Wielder of Images" },
 	{ "Mirage Maker", "Mirage Maker" },
 	{ "Mirage Maker", "Mirage Maker" },
 	{ "Dreamweaver", "Dreamweaver" },
 	{ "Dreamweaver", "Dreamweaver" },
 	{ "Invoker of Figments", "Invoker of Figments" },
 	{ "Invoker of Figments", "Invoker of Figments" },
 	{ "Caster of Shadows", "Caster of Shadows" },
 	{ "Caster of Shadows", "Caster of Shadows" },
 	{ "Swindler of the Mind", "Swindler of the Mind" },
 	{ "Swindler of the Mind", "Swindler of the Mind" },
 	{ "Spellbinder of Lies", "Spellbinder of Lies" },
 	{ "Spellbinder of Lies", "Spellbinder of Lies" },
 	{ "Master Illusionist", "Mistress Illusionist" },
 	{ "Master Illusionist", "Mistress Illusionist" },
 	{ "Master of Echoes", "Mistress of Echoes" },
 	{ "Master of Echoes", "Mistress of Echoes" },
 	{ "Master of Mirages", "Mistress of Mirages" },
 	{ "Master of Mirages", "Mistress of Mirages" },
 	{ "Master of Suggestion", "Mistress of Suggestion" },
 	{ "Master of Suggestion", "Mistress of Suggestion" },
 	{ "Master of Imagination", "Mistress of Imagination" },
 	{ "Master of Imagination", "Mistress of Imagination" },
 	{ "Master of Dreams", "Mistress of Dreams" },
 	{ "Master of Dreams", "Mistress of Dreams" },
 	{ "Master of Hallucinations", "Mistress of Hallucinations" },
 	{ "Master of Hallucinations", "Mistress of Hallucinations" },
 	{ "Master of Phantasm", "Mistress of Phantasm" },
 	{ "Master of Phantasm", "Mistress of Phantasm" },
 	{ "Greater Master Illusionist", "Greater Mistress Illusionist" },
 	{ "Greater Master Illusionist", "Greater Mistress Illusionist" },
 	{ "High Wizard of Illusions", "High Mistress of Illusions" },
 	{ "High Wizard of Illusions", "High Mistress of Illusions" },
 	{ "Legendary Illusionist", "Legendary Illusionist" },
 	{ "Avatar Illusionist", "Avatar Illusionist" },
 	{ "Illusionist Angel", "Illusionist Angel" },
 	{ "Illusionist Demigod", "Illusionist Demigoddess" },
 	{ "Immortal Illusionist", "Immortal Illusionist" },
 	{ "God of Illusions", "Goddess of Illusions" },
 	{ "Diety of Illusions", "Diety of Illusions" },
 	{ "Supremicy of Illusions", "Supremecy of Illusions" },
 	{ "Creator of Illusions", "Creator of Illusions" },
 	{ "Illusionist Implementor", "Illusionist Implementor" }
   }, 


    {
	{ "Man", "Woman" },
	{ "Apprentice of Magic", "Apprentice of Magic" }, 
	{ "Apprentice of Magic", "Apprentice of Magic" }, 
	{ "Spell Student", "Spell Student" },
	{ "Spell Student", "Spell Student" },
	{ "Scholar of Magic", "Scholar of Magic" },
	{ "Scholar of Magic", "Scholar of Magic" },
	{ "Delver in Spells", "Delver in Spells" },
 	{ "Delver in Spells", "Delver in Spells" },
	{ "Medium of Magic", "Medium of Magic" },
 	{ "Medium of Magic", "Medium of Magic" },
	{ "Scribe of Magic", "Scribe of Magic" },
 	{ "Scribe of Magic", "Scribe of Magic" },
	{ "Seer", "Seer" }, 
	{ "Seer", "Seer" },
	{ "Sage", "Sage" },
 	{ "Sage", "Sage" }, 
	{ "Abjurer", "Abjurer" }, 
	{ "Abjurer", "Abjurer" },
	{ "Magician", "Magician" }, 
	{ "Magician", "Magician" },
	{ "Hopeful Conjurer", "Hopeful Conjurer" },
	{ "Hopeful Conjurer", "Hopeful Conjurer" },
	{ "Student Conjurer", "Student Conjurer" },
 	{ "Student Conjurer", "Student Conjurer" },
	{ "Novice Conjurer", "Novice Conjurer" }, 
	{ "Novice Conjurer", "Novice Conjurer" },
	{ "Caster of Cantrips", "Caster of Cantrips" },
 	{ "Caster of Cantrips", "Caster of Cantrips" },
	{ "Lesser Conjurer", "Lesser Conjurer" },
 	{ "Lesser Conjurer", "Lesser Conjurer" }, 
	{ "Student of Invocation", "Student of Invocation" },
	{ "Student of Invocation", "Student of Invocation" }, 
	{ "Student of Summoning", "Student of Summoning" },
 	{ "Student of Summoning", "Student of Summoning" },
	{ "Student of Conjuration", "Student of Conjuration" }, 
	{ "Student of Conjuration", "Student of Conjuration" },
	{ "Candidate Conjurer", "Candidate Conjurer" },
 	{ "Candidate Conjurer", "Candidate Conjurer" }, 
	{ "Conjurer", "Conjurer" }, 
	{ "Conjurer", "Conjurer" },
	{ "Lesser Summoner", "Lesser Summoner" }, 
	{ "Lesser Summoner", "Lesser Summoner" },
	{ "Summoner", "Summoner" },
 	{ "Summoner", "Summoner" },
	{ "Summoner of Monsters", "Summoner of Monsters" },
 	{ "Summoner of Monsters", "Summoner of Monsters" },
	{ "Greater Summoner", "Greater Summoner" },
 	{ "Greater Summoner", "Greater Summoner" }, 
	{ "Summoner of Dragons", "Summoner of Dragons" },
 	{ "Summoner of Dragons", "Summoner of Dragons" },
	{ "Brandisher of Staves", "Brandisher of Staves" },
 	{ "Brandisher of Staves", "Brandisher of Staves" },
	{ "Keeper of Wands", "Keeper of Wands" },
 	{ "Keeper of Wands", "Keeper of Wands" },
	{ "Reciter of Scrolls", "Reciter of Scrolls" },
 	{ "Reciter of Scrolls", "Reciter of Scrolls" },
	{ "Master of Potions", "Mistress of Potions" },
 	{ "Master of Potions", "Mistress of Potions" },
	{ "Cantor of Spells", "Cantor of Spells" },
 	{ "Cantor of Spells", "Cantor of Spells" },
	{ "Lesser Invoker", "Lesser Invoker" }, 
	{ "Lesser Invoker", "Lesser Invoker" },  
	{ "Greater Invoker", "Greater Invoker" }, 
	{ "Greater Invoker", "Greater Invoker" }, 
	{ "Invoker of Fire", "Invoker of Fire" },
 	{ "Invoker of Fire", "Invoker of Fire" }, 
	{ "Invoker of Ice", "Invoker of Ice" }, 
	{ "Invoker of Ice", "Invoker of Ice" },
	{ "Invoker of the Elements", "Invoker of the Elements" },
 	{ "Invoker of the Elements", "Invoker of the Elements" },
	{ "Conjurer of Smoke", "Conjurer of Smoke" },
 	{ "Conjurer of Smoke", "Conjurer of Smoke" },
	{ "Caller of Lightning", "Caller of Lightning" },
 	{ "Caller of Lightning", "Caller of Lightning" },
	{ "Inciter of Power", "Inciter of Power" }, 
	{ "Inciter of Power", "Inciter of Power" },
	{ "Speaker of Incantations", "Speaker of Incantations" }, 
	{ "Speaker of Incantations", "Speaker of Incantations" },
	{ "Lesser Master Conjurer", "Lesser Mistress Conjurer" },
 	{ "Lesser Master Conjurer", "Lesser Mistress Conjurer" }, 
	{ "Master Conjurer", "Mistress Conjurer" },
 	{ "Master Conjurer", "Mistress Conjurer" }, 
	{ "Sorcerer of the Elements", "Sorceress of the Elements" },
 	{ "Sorcerer of the Elements", "Sorceress of the Elements" },
	{ "Sorcerer of Summoning", "Sorceress of Summoning" },
 	{ "Sorcerer of Summoning", "Sorceress of Summoning" },
	{ "Sorcerer of Invocation", "Sorceress of Invocation" },
 	{ "Sorcerer of Invocation", "Sorceress of Invocation" },
	{ "Master of Incantations", "Mistress of Incantations" },
 	{ "Master of Incantations", "Mistress of Incantations" },
	{ "Master of Invocation", "Master of Invocation" }, 
	{ "Master of Invocation", "Master of Invocation" },
	{ "Master of Summoning", "Mistress of Summoning" },
 	{ "Master of Summoning", "Mistress of Summoning" },
	{ "Master of Conjuration", "Mistress of Conjuration" },
 	{ "Master of Conjuration", "Mistress of Conjuration" },
	{ "Greater Master Conjurer", "Greater Mistress Conjurer" },
 	{ "Greater Master Conjurer", "Greater Mistress Conjurer" },
	{ "High Wizard of Conjuration", "High Mistress of Conjuration" },
	{ "High Wizard of Conjuration", "High Mistress of Conjuration" },
 	{ "Legendary Conjurer", "Legendary Conjurer" },
	{ "Avatar", "Avatar" }, 
	{ "Angel", "Angel" }, 
	{ "Demigod", "Demigoddess" }, 
	{ "Immortal", "Immortal" }, 
	{ "God", "Goddess" }, 
	{ "Diety", "Diety" }, 
	{ "Supremicy", "Supremicy" }, 
	{ "Creator", "Creator" }, 
	{ "Implementor", "Implementress" } 
  },

    {
 	{ "Man", "Woman" },
	{ "Servant", "Servant" },
	{ "Servant", "Servant" },
	{ "Laborer", "Laborer" },
	{ "Laborer", "Laborer" },
	{ "Toolkeeper", "Toolkeeper" },
	{ "Toolkeeper", "Toolkeeper" },
 	{ "Miner", "Miner" },
 	{ "Miner", "Miner" },
	{ "Prospector", "Prospector" },
	{ "Prospector", "Prospector" },
	{ "Student", "Student" },
	{ "Student", "Student" },
 	{ "Lodefinder", "Lodefinder" },
 	{ "Lodefinder", "Lodefinder" },
 	{ "Apprentice", "Apprentice" },
 	{ "Apprentice", "Apprentice" },
	{ "Carver", "Carver" },
	{ "Carver", "Carver" },
 	{ "Apprentice Stoneshaper", "Apprentice Stoneshaper" },
 	{ "Apprentice Stoneshaper", "Apprentice Stoneshaper" },
 	{ "Master Stoneshaper", "Master Stoneshaper" },
 	{ "Master Stoneshaper", "Master Stoneshaper" },
	{ "Apprentice Inventor", "Apprentice Inventor" },
	{ "Apprentice Inventor", "Apprentice Inventor" },
	{ "Master Inventor", "Master Inventor" },
	{ "Master Inventor", "Master Inventor" },
 	{ "Apprentice Metalshaper", "Apprentice Metalshaper" },
 	{ "Apprentice Metalshaper", "Apprentice Metalshaper" },
 	{ "Master Metalshaper", "Master Metalshaper" },
 	{ "Master Metalshaper", "Master Metalshaper" },
 	{ "Apprentice of the Anvil", "Apprentice of the Anvil" },
 	{ "Apprentice of the Anvil", "Apprentice of the Anvil" },
 	{ "Novice Engineer", "Novice Engineer" },
 	{ "Novice Engineer", "Novice Engineer" },
 	{ "Experienced Engineer", "Experienced Engineer" },
 	{ "Experienced Engineer", "Experienced Engineer" },
 	{ "Advanced Engineer", "Advanced Engineer" },
	{ "Advanced Engineer", "Advanced Engineer" },
	{ "Apprentice Crafter", "Apprentice Crafter" },
	{ "Apprentice Crafter", "Apprentice Crafter" },
	{ "Master Crafter", "Master Crafter"},
	{ "Master Crafter", "Master Crafter"},
 	{ "Craftsman", "Craftswoman" },
 	{ "Craftsman", "Craftswoman" },
	{ "Apprentice Oreshaper", "Apprentice Oreshaper" },
	{ "Apprentice Oreshaper", "Apprentice Oreshaper" },
	{ "Master Oreshaper", "Master Oreshaper" },
	{ "Master Oreshaper", "Master Oreshaper" },
 	{ "Apprentice Artisan", "Apprentice Artisan" },
 	{ "Apprentice Artisan", "Apprentice Artisan" },
 	{ "Master Artisan", "Master Artisan" },
 	{ "Master Artisan", "Master Artisan" },
	{ "Apprentice Engraver", "Apprentice Engraver" },
	{ "Apprentice Engraver", "Apprentice Engraver" },
	{ "Master Engraver", "Master Engraver" },
	{ "Master Engraver", "Master Engraver" },
 	{ "Apprentice Alchemist", "Apprentice Alchemist" },
 	{ "Apprentice Alchemist", "Apprentice Alchemist" },
 	{ "Master Alchemist", "Master Alchemist" },
 	{ "Master Alchemist", "Master Alchemist" },
	{ "Apprentice Melder", "Apprentice Melder" },
	{ "Apprentice Melder", "Apprentice Melder" },
	{ "Master Melder", "Master Melder", },
	{ "Master Melder", "Master Melder", },
 	{ "Apprentice Runescribe", "Apprentice Runescribe" },
 	{ "Apprentice Runescribe", "Apprentice Runescribe" },
 	{ "Master Runescribe", "Master Runescribe" },
 	{ "Master Runescribe", "Master Runescribe" },
 	{ "Apprentice Runesmith", "Apprentice Runesmith" },
 	{ "Apprentice Runesmith", "Apprentice Runesmith" },
 	{ "Master Runesmith", "Master Runesmith" },
 	{ "Master Runesmith", "Master Runesmith" },
	{ "Rune Lord", "Rune Mistress" },
	{ "Rune Lord", "Rune Mistress" },
 	{ "Novice Loremaster", "Novice Loremaster" },
 	{ "Novice Loremaster", "Novice Loremaster" },
 	{ "Loremaster", "Loremaster" },
 	{ "Loremaster", "Loremaster" },
 	{ "Apprentice of the Flame", "Apprentice of the Flame" },
 	{ "Apprentice of the Flame", "Apprentice of the Flame" },
 	{ "Master of the Flame", "Master of the Flame" },
 	{ "Master of the Flame", "Master of the Flame" },
	{ "Apprentice Stonemason", "Apprentice Stonemason" },
	{ "Apprentice Stonemason", "Apprentice Stonemason" },
 	{ "Master Stonemason", "Master Stonemason" },
 	{ "Master Stonemason", "Master Stonemason" },
 	{ "Armorer", "Armorer" },
 	{ "Armorer", "Armorer" },
	{ "Apprentice Weaponsmith", "Apprentice Weaponsmith" },
	{ "Apprentice Weaponsmith", "Apprentice Weaponsmith" },
 	{ "Weaponsmith", "Weaponsmith" },
 	{ "Weaponsmith", "Weaponsmith" },
 	{ "Apprentice of the Forge", "Apprentice of the Forge" },
 	{ "Apprentice of the Forge", "Apprentice of the Forge" },
 	{ "Master of the Forge", "Master of the Forge" },
 	{ "Master of the Forge", "Master of the Forge" },
 	{ "Master Blacksmith", "Master Blacksmith" },
 	{ "Master Blacksmith", "Master Blacksmith" },
 	{ "Grand Master Blacksmith", "Grand Master Blacksmith" },
 	{ "Grand Master Blacksmith", "Grand Master Blacksmith" },
 	{ "Legendary Blacksmith", "Legendary Blacksmith" },
 	{ "Avatar", "Avatar" },
 	{ "Angel", "Angel" },
 	{ "Demigod", "Demigoddess", },
 	{ "Immortal", "Immortal" },
 	{ "God", "Goddess" },
 	{ "Deity", "Deity" },
 	{ "Supreme Master", "Supreme Mistress" },
 	{ "Creator", "Creator" },
 	{ "Implementor", "Implementor" } 
   },

};

/*
 * Attribute bonus tables.
 */
const	struct	str_app_type	str_app		[36]		=
{
    { -5, -4,   0,  0 },  /* 0  */
    { -5, -4,   3,  1 },
    { -3, -2,   3,  2 },
    { -3, -1,  10,  3 },
    { -2, -1,  25,  4 },
    { -2, -1,  55,  5 },  /* 5  */
    { -1,  0,  80,  6 },
    { -1,  0,  90,  7 },
    {  0,  0, 100,  8 },
    {  0,  0, 100,  9 },
    {  0,  0, 115, 10 }, /* 10  */
    {  0,  0, 115, 11 },
    {  0,  0, 130, 12 },
    {  0,  0, 130, 13 }, /* 13  */
    {  0,  1, 140, 14 },
    {  1,  1, 150, 15 }, /* 15  */
    {  1,  2, 165, 16 },
    {  2,  3, 180, 22 },
    {  2,  3, 200, 25 }, /* 18  */
    {  3,  4, 225, 30 },
    {  3,  5, 250, 35 }, /* 20  */
    {  4,  6, 300, 40 },
    {  4,  6, 350, 45 },
    {  5,  7, 400, 50 },
    {  5,  8, 400, 55 },
    {  6,  8, 400, 60 }, /* 25   */
    {  6,  8, 450, 60 },
    {  6,  9, 450, 65 },
    {  7,  9, 450, 65 },
    {  7,  9, 450, 70 },
    {  7, 10, 500, 70 }, /* 30   */
    {  7, 10, 500, 75 },
    {  7, 10, 500, 75 },
    {  8, 10, 500, 80 },
    {  8, 11, 550, 80 },
    {  8, 11, 550, 85 }  /* 35   */
};



const	struct	int_app_type	int_app		[36]		=
{
    {  3 },			/*  0 */
    {  5 },	
    {  7 },
    {  8 },
    {  9 },
    { 10 },			/*  5 */
    { 11 },
    { 12 },
    { 13 },
    { 15 },
    { 17 },			/* 10 */
    { 19 },
    { 22 },
    { 25 },
    { 28 },
    { 31 },			/* 15 */
    { 34 },
    { 37 },
    { 40 },
    { 44 },
    { 49 },			/* 20 */
    { 55 },
    { 60 },
    { 70 },
    { 80 },
    { 80 },			/* 25 */
    { 80 },
    { 80 },
    { 80 },
    { 85 },
    { 85 },			/* 30 */
    { 85 },
    { 85 },
    { 90 },
    { 90 },
    { 90 }			/* 35 */
};



const	struct	wis_app_type	wis_app		[36]		=
{
    { 0 },			/*  0 */
    { 0 },
    { 0 },
    { 0 },
    { 0 },
    { 1 },			/*  5 */
    { 1 },
    { 1 },
    { 1 },
    { 1 },
    { 1 },			/* 10 */
    { 1 },
    { 1 },
    { 1 },
    { 1 },
    { 2 },			/* 15 */
    { 2 },
    { 2 },
    { 3 },
    { 3 },
    { 3 },			/* 20 */
    { 3 },
    { 4 },
    { 4 },
    { 4 },
    { 5 },			/* 25 */
    { 5 },
    { 5 },
    { 5 },
    { 6 },
    { 6 },			/* 30 */
    { 6 },
    { 6 },
    { 6 },
    { 7 },
    { 7 }			/* 35 */
};



const	struct	dex_app_type	dex_app		[36]		=
{
    {   60 },   		/* 0 */
    {   50 },
    {   50 },
    {   40 },
    {   30 },
    {   20 },   		/* 5 */
    {   10 },
    {    0 },
    {    0 },
    {    0 },
    {    0 },   		/* 10 */
    {    0 },
    {    0 },
    {    0 },
    {    0 },
    { - 10 },   		/* 15 */
    { - 15 },
    { - 20 },
    { - 30 },
    { - 40 },
    { - 50 },   		/* 20 */
    { - 60 },
    { - 75 },
    { - 90 },
    { -105 },
    { -110 },    		/* 25 */
    { -115 },
    { -120 },
    { -125 },
    { -130 },
    { -135 },    		/* 30 */
    { -140 },
    { -145 },
    { -150 },
    { -155 },
    { -160 }    		/* 35 */
};


const	struct	con_app_type	con_app		[36]		=
{
    { -4, 20 },   	/*  0 */
    { -3, 25 },
    { -2, 30 },
    { -2, 35 },
    { -1, 40 },
    { -1, 45 },   	/*  5 */
    { -1, 50 },
    {  0, 55 },
    {  0, 60 },
    {  0, 65 },
    {  0, 70 },   	/* 10 */
    {  0, 75 },
    {  0, 80 },
    {  0, 85 },
    {  0, 88 },
    {  1, 90 },   	/* 15 */
    {  2, 95 },
    {  2, 97 },
    {  3, 99 },
    {  3, 99 },
    {  4, 99 },   	/* 20 */
    {  4, 99 },
    {  5, 99 },
    {  6, 99 },
    {  7, 99 },
    {  8, 99 },    	/* 25 */
    {  8, 99 },
    {  9, 99 },
    {  9, 99 },
    {  9, 99 },
    {  9, 99 },    	/* 30 */
    { 10, 99 },
    { 10, 99 },
    { 10, 99 },
    { 10, 99 },
    { 10, 99 }    	/* 35 */
};



/*
 * Liquid properties.
 * Used in world.obj.
 */
const	struct	liq_type	liq_table	[]	=
{
/*    name			color	proof, full, thirst, food, ssize */
    { "water",			"clear",		{   0, 1, 10, 0, 16 }	},
    { "beer",			"amber",		{  12, 1,  8, 1, 12 }	},
    { "red wine",			"burgundy",		{  30, 1,  8, 1,  5 }	},
    { "ale",			"brown",		{  15, 1,  8, 1, 12 }	},
    { "dark ale",			"dark",		{  16, 1,  8, 1, 12 }	},
    { "whisky",			"golden",		{ 120, 1,  5, 0,  2 }	},
    { "lemonade",			"pink",		{   0, 1,  9, 2, 12 }	},
    { "firebreather",		"boiling",		{ 190, 0,  4, 0,  2 }	},
    { "local specialty",	"clear",		{ 151, 1,  3, 0,  2 }	},
    { "slime mold juice",	"green",		{   0, 2, -8, 1,  2 }	},
    { "milk",			"white",		{   0, 2,  9, 3, 12 }	},
    { "tea",			"tan",		{   0, 1,  8, 0,  6 }	},
    { "coffee",			"black",		{   0, 1,  8, 0,  6 }	},
    { "blood",			"red",		{   0, 2, -1, 2,  6 }	},
    { "salt water",		"clear",		{   0, 1, -2, 0,  1 }	},
    { "coke",			"brown",		{   0, 2,  9, 2, 12 }	}, 
    { "root beer",		"brown",		{   0, 2,  9, 2, 12 }   },
    { "elvish wine",		"green",		{  35, 2,  8, 1,  5 }   },
    { "white wine",		"golden",		{  28, 1,  8, 1,  5 }   },
    { "champagne",		"golden",		{  32, 1,  8, 1,  5 }   },
    { "mead",			"honey-colored",	{  34, 2,  8, 2, 12 }   },
    { "rose wine",		"pink",		{  26, 1,  8, 1,  5 }	},
    { "benedictine wine",	"burgundy",		{  40, 1,  8, 1,  5 }   },
    { "vodka",			"clear",		{ 130, 1,  5, 0,  2 }   },
    { "cranberry juice",	"red",		{   0, 1,  9, 2, 12 }	},
    { "orange juice",		"orange",		{   0, 2,  9, 3, 12 }   }, 
    { "absinthe",			"green",		{ 200, 1,  4, 0,  2 }	},
    { "brandy",			"golden",		{  80, 1,  5, 0,  4 }	},
    { "aquavit",			"clear",		{ 140, 1,  5, 0,  2 }	},
    { "schnapps",			"clear",		{  90, 1,  5, 0,  2 }   },
    { "icewine",			"purple",		{  50, 2,  6, 1,  5 }	},
    { "amontillado",		"burgundy",		{  35, 2,  8, 1,  5 }	},
    { "sherry",			"red",		{  38, 2,  7, 1,  5 }   },	
    { "framboise",		"red",		{  50, 1,  7, 1,  5 }   },
    { "rum",			"amber",		{ 151, 1,  4, 0,  2 }	},
    { "cordial",			"clear",		{ 100, 1,  5, 0,  2 }   },
    { NULL,				NULL,			{   0, 0,  0, 0,  0 }	}
};

 /* direction mapping */
 int regular_dir[6] = { 0,1,2,3,4,5 };
 int oppossite_dir[6] = { 2,3,0,1,5,4 };


/*
 * The skill and spell table.
 * Slot numbers must never be changed as they appear in #OBJECTS sections.
 */
#define SLOT(n)	n

const	struct	skill_type	skill_table	[MAX_SKILL]	=
{

/*
 * Magic spells.
 */

{
	"reserved", { 111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111 },
                    { 111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111 },
	0,					TAR_IGNORE,					POS_STANDING,
	NULL,					SLOT( 0),	 				0,	 		0,
	"",					"",						""
},

{
	"acid blast", { 29,102,102,102,102,102,102,102,102,102,36,28,28,28,28,102 },
                      { 1,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0 },
	spell_acid_blast,			TAR_CHAR_OFFENSIVE,			POS_FIGHTING,
	NULL,					SLOT(1),					20,			12,
	"acid blast",			"!Acid Blast!",				""
},

{
	"armor", {  7,2,102,102,102,102,102,102,102,102,102,7,7,7,7,102 },     
                 { 1,1,0,0,0,0,0,0,0,0,0,1,1,1,1,0 },
	spell_armor,			TAR_CHAR_DEFENSIVE,			POS_STANDING,
	NULL,					SLOT(2),	 				5,			12,
	"",					"You feel less armored.",		""
},

{
	"bless", { 102,7,102,102,102,102,102,102,102,102,102,102,102,102,102,102 },
                 { 0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	spell_bless,			TAR_OBJ_CHAR_DEF,	POS_STANDING,
	NULL,					SLOT(3),	 				5,			12,
	"",					"You feel less righteous.", 		"$p's holy aura fades."
},

{
	"blindness", {  102,15,102,102,102,102,102,102,102,102,102,12,12,12,12,102  },
                     { 0,1,0,0,0,0,0,0,0,0,0,1,1,1,1,0 },
	spell_blindness,			TAR_CHAR_SEMIOFFENSIVE,			POS_FIGHTING,
	&gsn_blindness,			SLOT(4),	 				5,			12,
	"",					"You can see again.",			""
},

{
	"burning hands", { 7,102,102,102,102,102,102,102,102,102,102,7,7,7,7,102  },
                         { 1,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0 },
	spell_burning_hands,		TAR_CHAR_OFFENSIVE,			POS_FIGHTING,
	NULL,					SLOT( 5),					15,			12,
	"burning hands",			"!Burning Hands!", 			""
},

{
    	"call lightning", { 102,102,102,102,102,102,102,102,102,102,102,102,102,102,26,102  },
                          { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0 },
	spell_call_lightning,		TAR_IGNORE,					POS_FIGHTING,
	&gsn_lightning,			SLOT( 6),					15,			12,
	"lightning bolt",			"!Call Lightning!",			""
},

{
	"calm",	{ 18,16,18,18,18,18,18,18,18,18,18,18,18,18,18,18  },
                { 0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	spell_calm,		TAR_IGNORE,		POS_FIGHTING,
	NULL,			SLOT(7),	30,	12,
	"",			"You have lost your peace of mind.",	""
},

{
	"cancellation",	{ 102,34,102,102,102,102,102,102,102,102,102,102,18,102,102,102  },
                        { 0,1,0,0,0,0,0,0,0,0,0,0,2,0,0,0 },
	spell_cancellation,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(8),	20,	12,
	""			"!cancellation!",	""
},

{
	"cause critical", { 102,19,102,102,102,102,102,102,102,102,102,102,102,102,102,102  },
                          { 0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	spell_cause_critical,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(9),	20,	12,
	"spell",		"!Cause Critical!",	""
},

{
	"cause light", { 102,1,102,102,102,102,102,102,102,102,102,102,102,102,102,102  },
                       { 0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	spell_cause_light,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(10),	15,	12,
	"spell",		"!Cause Light!",	""
},

{
	"cause serious", { 102,7,102,102,102,102,102,102,102,102,102,102,17,102,102,102  },
                         { 0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0 },
	spell_cause_serious,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(11),	17,	12,
	"spell",		"!Cause Serious!",	""
},

{   
	"chain lightning", { 102,102,102,102,102,102,102,102,102,102,102,102,102,102,33,102 }, 
                           { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0 },
	spell_chain_lightning,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(12),	125,	24,
	"lightning",		"!Chain Lightning!",	""
}, 

{
	"change sex", { 15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15  },
                      { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	spell_change_sex,	TAR_CHAR_SEMIOFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(13),	15,	12,
	"",			"Your body feels familiar again.",	""
},

{
	"charm person",	{ 102,102,102,102,102,102,102,102,102,102,102,102,20,102,102,102  },
                        { 0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0 },
	spell_charm_person,	TAR_CHAR_SEMIOFFENSIVE,	POS_STANDING,
	&gsn_charm_person,	SLOT( 14),	 5,	12,
	"",			"You feel more self-confident.",	""
},

{
	"chill touch", {  4,102,102,102,102,102,102,102,102,102,102,4,102,102,102,102  },
                       { 1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0 },
	spell_chill_touch,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT( 15),	15,	12,
	"chilling touch",	"You feel less cold.",	""
},

{
	"colour spray",	{ 102,102,102,102,102,102,102,102,102,102,102,102,102,16,102,102  },
                        { 0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0 },
	spell_colour_spray,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(16),	15,	12,
	"colour spray",		"!Colour Spray!",	""
},

{
	"continual light", { 102,102,102,102,102,102,102,102,102,102,102,102,102,8,102,102  },
                           { 0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0 },
	spell_continual_light,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(17),	 7,	12,
	"",			"!Continual Light!",	""
},

{
	"control weather", { 102,102,102,102,102,102,102,102,102,102,102,102,102,102,15,102  },
                           { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0 },
	spell_control_weather,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(18),	25,	12,
	"",			"!Control Weather!",	""
},

{
	"create food", { 102,5,102,102,102,102,102,102,102,102,102,102,102,102,10,102  },
                       { 0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0 },
	spell_create_food,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(19),	 5,	12,
	"",			"!Create Food!",	""
},

{
	"create rose", { 102,102,102,102,102,102,102,24,102,102,102,102,102,16,102,102  }, 	
                       { 0,0,0,0,0,0,0,2,0,0,0,0,0,1,0,0 },
	spell_create_rose,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(20),	30, 	12,
	"",			"!Create Rose!",	""
},  

{
	"create spring", { 102,17,102,102,102,102,102,102,102,102,102,102,102,102,14,102  },  
                         { 0,2,0,0,0,0,0,0,0,0,0,0,0,0,1,0 },
	spell_create_spring,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(21),	20,	12,
	"",			"!Create Spring!",	""
},

{
	"create water",	{ 102,102,102,102,102,102,102,102,102,102,102,102,102,102,8,102  },    
                        { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0 },
	spell_create_water,	TAR_OBJ_INV,		POS_STANDING,
	NULL,			SLOT(22),	 5,	12,
	"",			"!Create Water!",	""
},

{
	"cure blindness", { 102,6,102,102,102,102,102,102,102,102,102,102,102,102,102,102  },   
                          { 0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	spell_cure_blindness,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(23),	 5,	12,
	"",			"!Cure Blindness!",	""
},

{
	"cure critical", { 102,13,102,102,102,102,102,102,102,102,102,102,102,102,102,102  },   
                        { 0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	spell_cure_critical,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(24),	20,	12,
	"",			"!Cure Critical!",	""
},

{
	"cure disease",	{ 102,13,102,102,102,102,102,102,102,102,102,102,102,102,102,102  },  
                        { 0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	spell_cure_disease,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(25),	20,	12,
	"",			"!Cure Disease!",	""
},

{
	"cure light", { 102,1,102,102,102,102,102,102,102,102,102,102,102,102,102,102  },   
                      { 0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	spell_cure_light,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(26),	10,	12,
	"",			"!Cure Light!",		""
},

{
	"cure poison", { 102,14,102,102,102,102,102,102,102,102,102,102,102,102,102,102  }, 
                       { 0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	spell_cure_poison,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(27),	 5,	12,
	"",			"!Cure Poison!",	""
},

{
	"cure serious",	{ 102,7,102,102,102,102,102,102,102,102,102,102,102,102,102,102  }, 
                        { 0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	spell_cure_serious,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(28),	15,	12,
	"",			"!Cure Serious!",	""
},

{
	"curse", { 102,18,102,102,102,102,102,102,102,102,102,19,102,102,102,102  },  
                 { 0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0 },
	spell_curse,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	&gsn_curse,		SLOT(29),	20,	12,
	"curse",		"The curse wears off.", 
	"$p is no longer impure."
},

{
	"demonfire", { 102,34,102,102,102,102,102,102,102,102,102,43,102,102,102,102  },
                     { 0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0 },
	spell_demonfire,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(30),	45,	12,
	"torments",		"!Demonfire!",		""
},	

{
	"detect evil", { 102,4,102,102,102,102,102,102,102,102,102,102,102,102,102,102  }, 
                       { 0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	spell_detect_evil,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,			SLOT(31),	 5,	12,
	"",			"The red in your vision disappears.",	""
},

{
      "detect good", { 11,4,102,102,102,102,102,102,102,102,102,11,11,11,11,102  },     
                       { 1,1,0,0,0,0,0,0,0,0,0,1,1,1,1,0 },
      spell_detect_good,      TAR_CHAR_SELF,          POS_STANDING,
      NULL,                   SLOT(32),        5,     12,
	"",                     "The gold in your vision disappears.",	""
},

{
	"detect hidden", { 102,102,12,102,102,102,102,102,80,12,60,102,102,102,102,102  },    
                         { 0,0,2,0,0,0,0,0,1,1,1,0,0,0,0,0 },
	spell_detect_hidden,	TAR_CHAR_SELF,		POS_STANDING,
	&gsn_detect_hidden,			SLOT(33),	 5,	12,
	"",			"You feel less aware of your surroundings.",	
	""
},

{
      "forest vision", { 102,102,102,102,102,102,102,102,15,102,102,102,102,102,102,102  },
                       { 0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0 },
	spell_forest_vision,    TAR_CHAR_SELF,          POS_STANDING,
	&gsn_forest_vision,             SLOT(34),	 5,	12,  
      "",               "Your eyes become less coherent to the forest.",    ""
},

{
	"detect invis",	{  3,8,102,102,102,102,102,102,102,102,102,102,3,3,3,102  },     
                        { 1,1,0,0,0,0,0,0,0,0,0,0,1,1,1,0 },
	spell_detect_invis,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,			SLOT(35),	 5,	12,
	"",			"You no longer see invisible objects.",
	""
},

{
	"detect magic",	{ 2,102,102,102,102,102,102,102,102,102,102,2,2,2,2,102  },     
                        { 1,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0 },
	spell_detect_magic,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,			SLOT(36),	 5,	12,
	"",			"The detect magic wears off.",	""
},

{
	"detect poison", { 15,7,9,102,102,102,102,102,102,9,102,15,15,15,15,102  },  
                         { 1,1,1,0,0,0,0,0,0,1,0,1,1,1,1,0},
	spell_detect_poison,	TAR_OBJ_INV,		POS_STANDING,
	NULL,			SLOT(37),	 5,	12,
	"",			"!Detect Poison!",	""
},

{
	"dispel evil", { 102,15,102,102,102,102,102,102,102,102,102,102,102,102,102,102  },   
                       { 0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	spell_dispel_evil,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(38),	15,	12,
	"dispel evil",		"!Dispel Evil!",	""
},

{
      "dispel good", { 102,15,102,102,102,102,102,102,102,102,102,102,102,102,102,102  },
                       { 0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
      spell_dispel_good,      TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
      NULL,                   SLOT(39),      15,     12,
      "dispel good",          "!Dispel Good!",	""
},

{
	"dispel magic",	{ 16,24,102,102,20,102,102,102,102,102,102,16,16,16,16,102  },   
                        { 1,1,0,0,1,0,0,0,0,0,0,1,1,1,1,0 },
	spell_dispel_magic,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(40),	15,	12,
	"",			"!Dispel Magic!",	""
},

{
	"earthquake", { 102,10,102,102,102,102,102,102,102,102,102,102,26,102,102,102  },  
                      { 0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0 },
	spell_earthquake,	TAR_IGNORE,		POS_FIGHTING,
	NULL,			SLOT(41),	15,	12,
	"earthquake",		"!Earthquake!",		""
},

{
	"enchant armor", { 102,102,102,102,102,102,102,102,102,102,102,102,16,102,102,102  },
                         { 0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0 },
	spell_enchant_armor,	TAR_OBJ_INV,		POS_STANDING,
	NULL,			SLOT(42),	100,	24,
	"",			"!Enchant Armor!",	""
},

{
	"enchant weapon", { 53,102,102,102,102,102,102,102,102,102,102,102,17,102,102,102 }, 
                          { 1,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0 },
	spell_enchant_weapon,	TAR_OBJ_INV,		POS_STANDING,
	NULL,			SLOT(43),	100,	24,
	"",			"!Enchant Weapon!",	""
},

{
	"energy drain",	{ 102,40,102,102,102,102,102,102,102,102,102,19,102,102,102,102  },    
                        { 0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0 },
	spell_energy_drain,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(44),	35,	12,
	"energy drain",		"!Energy Drain!",	""
},

{
	"incendiary cloud", {  102,102,102,102,102,102,102,102,102,102,10,102,102,102,6,102  },   
                       { 0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0 },
	spell_incendiary_cloud,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(45),	 5,	12,
	"incendiary cloud",		"The green aura around you fades away.",
	""
},

{
	"glitterdust", { 102,102,102,102,102,102,102,102,102,102,102,102,102,102,20,102  },
                      { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0 },
	spell_glitterdust,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(46),	24,	12,
	"glitterdust",		"You rub the glitterdust out of your eyes!",		""
},

{
	"farsight", { 102,102,102,102,102,102,102,102,16,102,102,102,14,102,102,102  },
                    { 0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0 },
	spell_farsight,		TAR_CHAR_SELF,		POS_STANDING,
	NULL,			SLOT(47),	36,	20,
	"farsight",		"Your expanded vision returns to normal!",		""
},	

{
	"fireball", { 24,102,102,102,102,102,102,102,102,102,102,102,102,22,102,102  },
                    { 2,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0 },
	spell_fireball,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(48),	15,	12,
	"fireball",		"!Fireball!",		""
},
  
{
	"fireproof", { 102,12,102,102,102,102,102,19,102,102,102,102,102,102,102,102  },
	             { 0,1,0,0,0,0,0,2,0,0,0,0,0,0,0,0 },
	spell_fireproof,	TAR_OBJ_INV,		POS_STANDING,
	NULL,			SLOT(49),	10,	12,
	"",			"",	"$p's protective aura fades."
},

{
	"flamestrike", { 102,20,102,102,102,102,102,102,102,102,102,102,31,102,102,102  },
                       { 0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0 },
	spell_flamestrike,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(50),	20,	12,
	"flamestrike",		"!Flamestrike!",		""
},

{
	"levitate", { 10,102,102,102,102,102,102,102,102,102,15,102,10,102,102,102  },    
               { 1,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0 },
	spell_levitate,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(51),	10,	18,
	"",			"You slowly float to the ground.",	""
},

{
	"orb of containment", { 4,10,102,16,102,16,16,6,16,7,102,4,4,4,4,102  },
        	         { 1,1,0,2,0,2,2,1,2,2,0,1,1,1,1,0 },
	spell_orb_of_containment,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(52),	40,	24,
	"",			"!Orb of Containment!",	""
},

{
        "frenzy", { 102,102,102,26,102,26,20,102,26,102,102,102,102,102,102,102  },  
                  { 0,0,0,2,0,2,2,0,2,0,0,0,0,0,0,0 },
        spell_frenzy,           TAR_CHAR_SEMIOFFENSIVE,     POS_STANDING,
        NULL,                   SLOT(53),      30,     24,
        "",                     "Your rage ebbs.",	""
},

{
	"gate",	{102,102,102,102,102,102,102,102,102,102,62,102,37,39,25,102  },
                { 0,0,0,0,0,0,0,0,0,0,1,0,1,1,1,0 },
	spell_gate,		TAR_IGNORE,		POS_FIGHTING,
	NULL,			SLOT(54),	80,	24,
	"",			"!Gate!",		""
},

{
	"giant strength", {  31,102,102,102,102,102,102,102,102,102,21,102,16,102,102,102  },
                          { 2,0,0,0,0,0,0,0,0,0,2,0,2,0,0,0 },
	spell_giant_strength,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(55),	20,	12,
	"",			"You feel weaker.",	""
},

{
      "ethereal warrior", { 102,102,102,102,102,102,102,102,102,102,102,102,102,75,102,102  },
					{ 0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0 },
        spell_ethereal_warrior,     TAR_IGNORE,             POS_STANDING,
        NULL,           SLOT(56),      250,            32,
        "",     "You re-gained the power to summon more ethereal warriors.",	""    
},

{
	"harm",	{ 102,23,102,102,102,102,102,102,102,102,102,29,102,102,102,102  },
                { 0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0 },
	spell_harm,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(57),	35,	12,
	"harm spell",		"!Harm!,		"""
},
  
{
	"haste", { 102,102,102,102,102,102,102,102,102,26,50,102,21,102,102,102  },    
                 { 0,0,0,0,0,0,0,0,0,2,2,0,2,0,0,0 },
	spell_haste,		TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(58),	30,	12,
	"",			"You feel yourself slow down.",	""
},

{
	"heal", { 102,21,102,102,102,102,102,102,102,102,102,102,102,102,102,102  },    
                { 0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	spell_heal,		TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(59),	50,	12,
	"",			"!Heal!",		""
},

{
	"life transfer", { 102,21,102,102,102,102,102,70,102,102,102,40,40,40,40,102  },    
                		{ 0,1,0,0,0,0,0,1,0,0,0,1,1,1,1,0 },
	spell_life_transfer,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(60),	50,	12,
	"",			"!Life Transfer!",		""
},

{
	"mind shatter", { 102,102,102,102,102,102,102,102,102,102,64,102,51,102,102,102  },    
                		{ 0,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0 },
	spell_mind_shatter,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(61),	50,	12,
	"",			"Your mind mends back together again.",		""
},
  
{
	"heat metal", { 102,102,102,102,102,102,102,102,102,102,102,102,34,102,102,102  },
           	      { 0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0 },
	spell_heat_metal,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(62), 	25,	18,
	"spell",		"!Heat Metal!",		""
},

{
	"holy word", { 102,36,102,102,102,102,102,102,102,102,102,102,102,102,102,102  },
 	             { 0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	spell_holy_word,	TAR_IGNORE,	POS_FIGHTING,
	NULL,			SLOT(63), 	200,	24,
	"divine wrath",		"!Holy Word!",		""
},

{
	"identify", { 25,25,25,25,25,25,25,25,25,25,38,25,29,25,25,25  },
                    { 0,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0 },
	spell_identify,		TAR_OBJ_INV,		POS_STANDING,
	NULL,			SLOT(64),	12,	24,
	"",			"!Identify!",		""
},

{
	"infravision", {  9,102,10,102,102,102,102,102,102,10,1,9,9,9,9,102  },
                       { 1,0,2,0,0,0,0,0,0,2,1,1,1,1,1,0 },
	spell_infravision,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	&gsn_infravision,			SLOT(65),	 5,	18,
	"",			"You no longer see in the dark.",	""
},

{
	"invisibility",	{  5,102,102,102,102,102,102,102,102,102,85,5,5,5,5,102  },    
                        { 1,0,0,0,0,0,0,0,0,0,2,1,1,1,1,0 },
	spell_invis,		TAR_OBJ_CHAR_DEF,	POS_STANDING,
	&gsn_invis,		SLOT(66),	 5,	12,
	"",			"You are no longer invisible.",		
	"$p fades into view."
},

{
	"know alignment", {  12,9,102,102,102,102,102,102,102,102,102,12,12,12,12,102  },   
                          { 1,1,0,0,0,0,0,0,0,0,0,1,1,1,1,0 },
	spell_know_alignment,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(67),	 9,	12,
	"",			"!Know Alignment!",	""
},

{
	"storm of vengeance", {  102,102,102,102,102,102,102,17,102,102,102,102,102,102,13,102  },
                          { 0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0 },
	spell_storm_of_vengeance,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(68),	15,	12,
	"storm of vengeance",	"!Storm of Vengeance!",	""
},

{
	"locate object", { 102,102,102,102,102,102,102,102,102,102,102,102,27,102,102,102  },
                         { 0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0 },
	spell_locate_object,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(69),	20,	18,
	"",			"!Locate Object!",	""
},

{
	"magic missile", { 1,102,102,102,102,102,102,102,102,102,3,1,1,1,1,102  },    
                         { 1,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0 },
	spell_magic_missile,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(70),	15,	12,
	"magic missile",	"!Magic Missile!",	""
},

{
	"mass healing",	{ 102,38,102,102,102,102,102,102,102,102,102,102,102,102,102,102  },
                        { 0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	spell_mass_healing,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(71),	100,	36,
	"",			"!Mass Healing!",	""
},

{
	"mass invis", { 102,102,102,102,102,102,102,102,102,102,102,102,102,22,102,102  },
                      { 0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0 },
	spell_mass_invis,	TAR_IGNORE,		POS_STANDING,
	&gsn_mass_invis,	SLOT(72),	20,	24,
	"",			"You are no longer invisible.",		""
},

{
	"nexus", { 102,102,102,102,102,102,102,102,102,102,102,102,102,102,91,102  },
                 { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0 },
      spell_nexus,            TAR_IGNORE,             POS_STANDING,
      NULL,                   SLOT(73),       150,   36,
      "",                     "!Nexus!",		""
},

{
	"pass door", { 102,102,102,102,102,102,102,102,102,102,102,102,24,102,102,102  },
                     { 0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0},
	spell_pass_door,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,			SLOT(74),	20,	12,
	"",			"You feel solid again.",	""
},

{
	"aid", {102,40,102,102,102,102,102,102,102,102,102,53,53,53,53,102  },
			{0,1,0,0,0,0,0,0,0,0,0,1,1,1,1,0 },
	spell_aid,		TAR_CHAR_DEFENSIVE,		POS_STANDING,
	NULL,			SLOT(75),	15,	12,
	"",			"You don't feel so aided anymore.",		""
},

{
	"plague", { 102,17,102,102,102,102,102,102,102,102,102,23,102,102,102,102  },
                  { 0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0 },
	spell_plague,		TAR_CHAR_SEMIOFFENSIVE,	POS_FIGHTING,
	&gsn_plague,		SLOT(76),	20,	12,
	"sickness",		"Your sores vanish.",	""
},

{
	"poison", { 102,102,102,102,102,102,102,102,102,102,102,17,102,102,102,102  },
                  { 0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0 },
	spell_poison,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	&gsn_poison,		SLOT(77),	10,	12,
	"poison",		"You feel less sick.",	
	"The poison on $p dries up."
},

{
        "dimensional door", { 102,102,102,102,102,102,102,102,102,102,102,102,102,102,50,102  },
                  { 0,0,0,0,0,0,0,3,0,0,0,0,0,0,2,0 }, 
        spell_dimensional_door,           TAR_IGNORE,             POS_STANDING,
        NULL,                   SLOT(78),       100,     24,
        "",                     "!Dimensional Door!",		""
},

{
	"protection evil", { 102,9,102,102,102,30,102,102,102,102,102,102,12,12,12,102  },    
                           { 0,1,0,0,0,2,0,0,0,0,0,1,1,1,1,0 },
	spell_protection_evil,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,			SLOT(79), 	5,	12,
	"",			"You feel less protected.",	""
},

{
        "protection good", { 102,102,102,102,102,102,102,102,102,102,102,12,102,102,102,102  },
                           { 0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0 },
        spell_protection_good,  TAR_CHAR_SELF,          POS_STANDING,
        NULL,                   SLOT(80),       5,     12,
        "",                     "You feel less protected.",	""
},

{
        "ray of truth", { 102,35,102,102,102,102,102,102,102,102,102,102,102,102,102,102  },
                        { 0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
        spell_ray_of_truth,     TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
        NULL,                   SLOT(81),      20,     12,
        "ray of truth",         "!Ray of Truth!",	""
},

{
	"recharge", { 102,102,102,102,102,102,102,102,102,102,102,102,9,102,9,102  }, 	
                    { 0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0 },
	spell_recharge,		TAR_OBJ_INV,		POS_STANDING,
	NULL,			SLOT(82),	60,	24,
	"",			"!Recharge!",		""
},

{
	"refresh", {  102,5,102,102,102,102,102,102,102,102,102,102,8,8,8,102  },     
                   { 0,1,0,0,0,0,0,0,0,0,0,0,1,1,1,0 },
	spell_refresh,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(83),	12,	18,
	"refresh",		"!Refresh!",		""
},

{
	"remove curse", { 102,18,102,102,102,102,102,102,102,102,102,102,102,102,102,102  },
                        { 0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	spell_remove_curse,	TAR_OBJ_CHAR_DEF,	POS_STANDING,
	NULL,			SLOT(84),	 5,	12,
	"",			"!Remove Curse!",	""
},

{
	"sanctuary", { 102,36,102,102,102,102,102,102,102,102,102,102,102,102,102,102  },
                     { 0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	spell_sanctuary,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	&gsn_sanctuary,		SLOT(85),	75,	12,
	"",			"The white aura around your body fades.",
	""
},

{
	"shield", { 18,102,102,102,102,102,102,102,102,102,22,102,102,20,102,102  },    
                  { 2,0,0,0,0,0,0,0,0,0,1,0,0,1,0,0 },
	spell_shield,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(86),	12,	18,
	"",			"Your force shield shimmers then fades away.",
	""
},

{
	"shocking grasp", {  10,102,102,102,102,102,102,102,102,102,102,102,102,10,102,102  },
                          { 1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0 },
	spell_shocking_grasp,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(87),	15,	12,
	"shocking grasp",	"!Shocking Grasp!",	""
},

{
	"sleep", { 102,102,102,102,102,102,102,102,102,102,102,102,10,102,102,102  },
                 { 0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0 },
	spell_sleep,		TAR_CHAR_SEMIOFFENSIVE,	POS_STANDING,
	&gsn_sleep,		SLOT(88),	15,	12,
	"",			"You feel less tired.",	""
},

{
        "slow", { 102,102,102,102,102,102,102,102,102,102,102,102,23,102,102,102  },
                { 0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0 },
        spell_slow,             TAR_CHAR_SEMIOFFENSIVE,     POS_FIGHTING,
        NULL,                   SLOT(89),      30,     12,
        "",                     "You feel yourself speed up.",	""
},

{
	"flesh to stone", { 102,102,102,102,102,102,102,102,102,102,102,102,25,102,102,102  },
                      		{ 0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0 },
	spell_flesh_to_stone,		TAR_CHAR_SELF,				POS_STANDING,
	NULL,					SLOT(90),					12,			18,
	"",					"Your flesh loses it's stone like texture.",	""
},

{
	"summon", { 102,102,102,102,20,102,102,102,102,102,102,102,102,102,30,102  },
                  { 0,0,0,0,1,0,0,0,0,0,0,0,0,0,2,0 },
	spell_summon,		TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(91),	50,	12,
	"",			"!Summon!",		""
},

{
	"teleport", {  13,22,102,102,102,102,102,40,102,102,102,13,13,13,13,102  },
                    { 1,1,0,0,0,0,0,2,0,0,0,1,1,1,1,0 },
	spell_teleport,		TAR_CHAR_SELF,		POS_FIGHTING,
	NULL,	 		SLOT( 92),	35,	12,
	"",			"!Teleport!",		""
},

{
	"ventriloquate", {  102,102,102,102,102,102,102,102,102,102,102,102,102,3,102,102  },
                         { 0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0 },
	spell_ventriloquate,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(93),	 5,	12,
	"",			"!Ventriloquate!",	""
},

{
	"weaken", {  102,102,102,102,102,102,102,102,102,102,18,11,11,11,11,102  },    
                  { 0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0 },
	spell_weaken,		TAR_CHAR_SEMIOFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(94),	20,	12,
	"spell",		"You feel stronger.",	""
},

{
	"word of recall", { 74,25,102,102,102,102,102,102,102,102,70,60,60,60,60,102  },
                          { 1,1,0,0,0,0,0,0,0,0,1,1,1,1,1,0 },
	spell_word_of_recall,	TAR_CHAR_SELF,		POS_RESTING,
	NULL,			SLOT(95),	 25,	12,
	"",			"!Word of Recall!",	""
},

{
	"acid breath", { 102,102,102,102,102,102,102,102,102,102,102,65,102,102,102,102  },    
                       { 0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0 },
	spell_acid_breath,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(96),	100,	24,
	"blast of acid",	"!Acid Breath!",	""
},

{
	"fire breath", { 102,102,102,102,102,102,102,102,102,102,102,102,102,67,67,102  },
                       { 0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0 },
	spell_fire_breath,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(97),	125,	24,
	"blast of flame",	"The smoke leaves your eyes.",	""
},

{
	"frost breath",	{ 102,102,102,102,102,102,102,102,102,102,102,102,102,68,68,102  },
                        { 0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0 },
	spell_frost_breath,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(98),	125,	24,
	"blast of frost",	"!Frost Breath!",	""
},

{
	"gas breath", { 102,102,102,102,102,102,102,102,102,102,102,102,102,78,78,102  },
                      { 0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0 },
	spell_gas_breath,	TAR_IGNORE,		POS_FIGHTING,
	NULL,			SLOT(99),	150,	24,
	"blast of gas",		"!Gas Breath!",		""
},

{
	"lightning breath", { 102,102,102,102,102,102,102,102,102,102,102,102,102,84,84,102 },
                            { 0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0},
	spell_lightning_breath,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(100),	150,	24,
	"blast of lightning",	"!Lightning Breath!",	""
},

{
      "general purpose", {102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102 },
                           { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
      spell_general_purpose,  TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
      NULL,                   SLOT(101),      0,      12,
      "general purpose ammo", "!General Purpose Ammo!",	""
},
 
{
	"high explosive", { 102,102,102,102,102,102,102,102,102,102,102,102,102,102,102 },
                          { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
      spell_high_explosive,   TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
      NULL,                   SLOT(102),      0,      12,
      "high explosive ammo",  "!High Explosive Ammo!",	""
},

{
	"flaming shield",  {102,102,102,102,102,102,102,102,102,102,102,102,30,102,102,102 },
					{ 0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0},
	spell_flame_shield,  	TAR_CHAR_SELF,  	POS_STANDING,
	&gsn_flame_shield,	SLOT(103),	100,	12,
	"",	"The flaming shield around you dies out.",		""
},

{
	"axe", { 102,102,1,1,102,1,1,102,1,1,102,102,102,102,102,102 },
               { 0,0,2,2,0,2,2,0,2,2,0,0,0,0,0,0},
      spell_null,             TAR_IGNORE,             POS_FIGHTING,
      &gsn_axe,            	SLOT( 104),       0,      0,
      "",                     "!Axe!",		""
},

{
      "dagger", {  1,102,1,1,1,102,1,1,1,1,1,1,1,1,1,102 },
                  { 2,0,2,2,2,0,2,2,2,2,2,2,2,2,2,0},
      spell_null,             TAR_IGNORE,             POS_FIGHTING,
      &gsn_dagger,            SLOT( 105),       	0,      		0,
      "",                     "!Dagger!",			""
},
 
{
	"flail", { 102,1,1,1,1,1,1,102,1,1,102,102,102,102,102,102  },
                 { 0,2,2,2,2,2,2,0,2,2,0,0,0,0,0,0},
      spell_null,             TAR_IGNORE,             POS_FIGHTING,
      &gsn_flail,            	SLOT( 106),       	0,      		0,
      "",                     "!Flail!",			""
},

{
	"mace",	{ 1,1,1,1,1,1,1,102,1,1,102,102,102,102,102,102 },
        		{ 3,2,3,3,2,3,3,0,3,3,0,0,0,0,0,0},
      spell_null,             TAR_IGNORE,             POS_FIGHTING,
      &gsn_mace,            	SLOT( 107),       	0,      		0,
      "",                     "!Mace!",			""
},

{
	"polearm", { 1,102,1,1,1,1,1,102,1,1,102,102,102,102,102,102 },
        	   { 2,0,2,2,2,2,2,0,2,2,0,0,0,0,0,0},
      spell_null,             TAR_IGNORE,             POS_FIGHTING,
      &gsn_polearm,           SLOT( 108),       0,      0,
      "",                     "!Polearm!",		""
},
    
{
	"shield block",	{ 4,102,3,1,15,1,1,14,1,3,102,102,102,102,102,102 },
         	        { 4,0,6,2,6,2,2,4,2,6,0,0,0,0,0,0},
	spell_null,		TAR_IGNORE,		POS_FIGHTING,
	&gsn_shield_block,	SLOT(109),	0,	0,
	"",			"!Shield!",		""
},
 
{
	"spear", {  1,102,1,1,102,1,1,1,1,1,1,102,102,102,102,102 },
	         { 2,0,2,2,0,2,2,2,2,2,4,0,0,0,0,0},
      spell_null,             TAR_IGNORE,             POS_FIGHTING,
      &gsn_spear,            	SLOT( 110),       0,      0,
      "",                     "!Spear!",		""
},

{
	"sword", { 1,102,1,1,1,1,1,102,1,1,1,102,102,102,102,102 },
	         { 2,0,3,2,2,2,2,0,2,3,4,0,0,0,0,0},
      spell_null,             TAR_IGNORE,             POS_FIGHTING,
      &gsn_sword,            	SLOT( 111),       0,      0,
      "",                     "!sword!",		""
},

{
	"whip",	{ 1,102,1,1,1,1,1,102,1,1,1,1,1,1,1,102 },
                { 2,0,2,2,2,2,2,0,2,2,2,2,2,2,2,0},
      spell_null,             TAR_IGNORE,             POS_FIGHTING,
      &gsn_whip,            	SLOT( 112),       0,      0,
      "",                     "!Whip!",	""
},

{
      "backstab", { 102,102,1,102,102,102,102,102,102,1,8,102,102,102,102,102 },
                    { 0,0,5,0,0,0,0,0,0,5,6,0,0,0,0,0},
      spell_null,             TAR_IGNORE,             POS_STANDING,
      &gsn_backstab,          SLOT( 113),        0,     12,
      "backstab",             "!Backstab!",		""
},

{
	"bash",	{ 3,102,102,2,102,2,1,102,2,102,102,102,102,102,102,102 },
         	{ 4,0,0,4,0,4,4,0,4,0,0,0,0,0,0,0},
      spell_null,             TAR_IGNORE,             POS_FIGHTING,
      &gsn_bash,            	SLOT( 114),       0,      24,
      "bash",                 "!Bash!",		""
},

{
	"berserk", { 102,102,102,18,102,18,10,102,18,102,102,102,102,102,102,102},
           	   { 0,0,0,5,0,5,3,0,5,0,0,0,0,0,0,0 },
      spell_null,             TAR_IGNORE,             POS_FIGHTING,
      &gsn_berserk,        	SLOT( 115),       0,      24,
      "",                     "You feel your pulse slow down.",	""
},

{
       "camouflage", { 102,102,102,102,102,102,102,102,18,102,102,102,102,102,102,102  },
               { 0,0,0,0,0,0,0,0,5,0,0,0,0,0,0,0 },
       spell_null,             TAR_IGNORE,             POS_STANDING,
       &gsn_camouflage,         SLOT( 116),     0,	24,
       "",                     "!Camouflage!",        	"",
},

{
	"dirt kicking",	{ 102,102,3,3,102,3,3,7,3,3,102,102,102,102,102,102  },
	                { 0,0,4,4,0,4,4,4,4,4,0,0,0,0,0,0 }, 
	spell_null,		TAR_IGNORE,		POS_FIGHTING,
	&gsn_dirt,		SLOT( 117),	0,	24,
	"kicked dirt",		"You rub the dirt out of your eyes.",	""
},

{
      "disarm", { 19,102,12,11,15,11,11,20,11,12,102,102,102,102,102,102  },
                  { 6,0,6,4,6,4,4,6,4,6,0,0,0,0,0,0 },
      spell_null,             TAR_IGNORE,             POS_FIGHTING,
      &gsn_disarm,            SLOT( 118),        0,     24,
      "",                     "!Disarm!",		""
},
 
{
      "dodge", { 5,22,1,1,1,1,1,1,1,1,10,20,20,20,20,102  },
                 { 4,8,4,6,7,6,6,7,6,4,7,8,8,8,8,0 },
      spell_null,             TAR_IGNORE,             POS_FIGHTING,
      &gsn_dodge,             SLOT( 119),        0,     0,
      "",                     "!Dodge!",		""
},
 
{
      "enhanced damage", { 38,54,25,2,20,2,2,32,2,25,41,54,54,54,54,102  },
                           { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0 },
      spell_null,             TAR_IGNORE,             POS_FIGHTING,
      &gsn_enhanced_damage,   SLOT( 120),        0,     0,
      "",                     "!Enhanced Damage!",	""
},

{
	"envenom", { 20,20,10,20,20,20,15,20,35,10,20,47,47,47,47,30 },
	           { 0,0,4,0,0,0,8,8,10,4,0,6,6,6,6,0 },
	spell_null,		TAR_IGNORE,	  	POS_RESTING,
	&gsn_envenom,		SLOT(121),	0,	12,
	"",			"!Envenom!",		""
},

{
	"hand to hand",	{ 8,57,15,6,1,6,3,10,6,15,102,102,102,102,102,3 },
                  	{ 6,7,6,4,3,4,3,5,4,6,0,0,0,0,0,1 },
	spell_null,		TAR_IGNORE,		POS_FIGHTING,
	&gsn_hand_to_hand,	SLOT( 122),	0,	0,
	"",			"!Hand to Hand!",	""
},

{
      "kick", { 13,102,14,8,4,8,8,10,8,14,102,102,102,102,102,102  },
                { 3,0,6,3,2,3,3,5,3,6,0,0,0,0,0,0 },
	spell_null,             TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
      &gsn_kick,              SLOT( 123),        0,     12,
      "kick",                 "!Kick!",		""
},

{
      "parry", { 49,45,102,1,5,1,1,7,1,102,33,45,45,45,45,102  },
                 { 6,8,0,4,4,4,4,5,4,0,6,8,8,8,8,0 },
      spell_null,             TAR_IGNORE,             POS_FIGHTING,
      &gsn_parry,             SLOT( 124),        0,     0,
      "",                     "!Parry!",		""
},

{
      "rescue", { 102,102,102,1,102,1,102,102,102,102,102,102,102,102,102,102  },    
                  { 0,0,0,4,0,4,0,0,0,0,0,0,0,0,0,0 },
      spell_null,             TAR_IGNORE,             POS_FIGHTING,
      &gsn_rescue,            SLOT( 125),        0,     12,
      "",                     "!Rescue!",		""
},

{
	"trip",	{ 28,102,1,15,1,15,8,8,10,1,102,102,102,102,102,102  },
          		{ 4,0,4,8,4,8,6,6,7,4,0,0,0,0,0,0 },
	spell_null,		TAR_IGNORE,		POS_FIGHTING,
	&gsn_trip,		SLOT( 126),	0,	24,
	"trip",			"!Trip!",		""
},

{
	"second attack", { 21,52,12,5,10,5,5,15,5,12,35,52,52,52,52,102  },
                         { 4,10,5,3,4,3,3,6,3,5,6,10,10,10,10,0 },
	spell_null,             TAR_IGNORE,             POS_FIGHTING,
	&gsn_second_attack,     SLOT( 0),        0,     0,
	"",                     "!Second Attack!",	""
},

{
	"third attack", { 67,102,58,12,102,12,12,102,12,58,102,102,102,102,102,102  },
                        { 4,0,10,4,0,4,4,0,4,10,0,0,0,0,0,0 },
	spell_null,             TAR_IGNORE,             POS_FIGHTING,
	&gsn_third_attack,      SLOT( 0),        0,     0,
	"",                     "!Third Attack!",	""
},

{
	"fourth attack", {102,102,102,65,102,48,84,102,74,102,102,102,102,102,102,102  },
                        {0,0,0,4,0,4,4,0,4,0,0,0,0,0,0,0 },
	spell_null,             TAR_IGNORE,             POS_FIGHTING,
	&gsn_fourth_attack,     SLOT( 0),        0,     0,
	"",                     "!Fourth Attack!",    ""
},

{
	"track", { 30,30,85,30,30,30,30,30,25,50,30,30,30,30,30,30 },
                   {0,0,4,0,0,0,0,0,4,4,0,0,0,0,0,0 },
	spell_null,			TAR_IGNORE,			POS_STANDING,
	&gsn_track,			SLOT( 130),       	0,       		0,
	"",			      "!Track!",     		""
},


{ 
	"break weapon",
  	{ 102,102,102,65,102,57,80,102,102,102,102,102,102,102,102,102 },
  	{0,0,0,1,0,1,1,0,0,0,0,0,0,0,0,0},
  	spell_null,			TAR_CHAR_SELF, 			POS_STANDING,
  	&gsn_break_weapon,	SLOT(131),      			25,    		12,
  	"",                     "",        				""
},


{
	"counter",   { 102,102,74,40,65,45,48,102,70,75,102,102,102,102,102,102 },
                         { 0,0,8,6,6,6,6,0,8,8,0,0,0,0,0,0},
	spell_null,             TAR_IGNORE,             	POS_FIGHTING,
	&gsn_counter,           SLOT( 132),       		0,      		0,
	"counterattack",        "!Counter!",   			""
},

{         
	"critical strike",  { 102,102,102,48,102,53,35,102,102,70,102,102,102,102,102,102 },
                             { 0,0,0,1,0,1,1,0,0,2,0,0,0,0,0,0}, 
	spell_null,             TAR_IGNORE,             	POS_FIGHTING,
	&gsn_critical,          SLOT( 133),       		0,      		0,
	"",                     "!Critical Strike!",   		""     
},

{
	"sharpen",   { 102,102,102,95,102,102,102,75,102,102,102,102,102,102,102,8 },
                      { 0,0,0,6,0,0,0,6,0,0,0,0,0,0,0,1},
	spell_null,             TAR_IGNORE,             	POS_RESTING,
	&gsn_sharpen,           SLOT( 134),        		0,      		0,
	"",                     "!Sharpen!",            	""
},

{
	"deathgrip",  { 102,95,102,102,90,95,102,60,102,73,102,102,102,102,102,102 },
			       { 0,4,0,0,5,5,0,3,0,4,0,0,0,0,0,0},
	spell_null,             TAR_IGNORE,             	POS_RESTING,
	NULL,                   SLOT( 135),       		0,      		12,
	"",                     "The dark shroud leaves your hands.",   ""
},

{
	"demand",       { 102,53,45,102,102,102,102,102,102,53,102,102,102,102,102,102 },
				   { 0,2,1,0,0,0,0,0,0,2,0,0,0,0,0,0},
	spell_null,     		TAR_IGNORE,    			POS_STANDING,
	&gsn_demand,    		SLOT(136),        		0,      		12,
	"",				"!Demand!",				""
},

{
	  "convoke swarm",{ 102,102,102,102,102,102,102,102,102,102,102,102,102,102,95,102 },
					{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0},
	  spell_convoke_swarm,	TAR_CHAR_OFFENSIVE,		POS_FIGHTING,
	  NULL,			SLOT( 137),				150,			24,
	  "swarm of insects",	"!Convoke Swarm!",		""
},

    {
        "circlestab", { 102,102,54,102,102,102,102,102,102,54,85,102,102,102,102,102 },
                    { 0,0,1,0,0,0,0,0,0,1,2,0,0,0,0,0},
        spell_null,           TAR_IGNORE,             	POS_STANDING,
        &gsn_circle,		SLOT( 138),        		0,     		24,
        "circlestab",         "!Circlestab!",			""
    },

    {
        "whirlwind",	{ 102,102,102,52,102,29,45,102,102,102,102,102,102,102,102,102 },
			  { 0,0,0,1,0,1,1,0,0,0,0,0,0,0,0,0},
        spell_null,		TAR_IGNORE,				POS_STANDING,
        &gsn_whirlwind,		SLOT( 139),				0,			12,
        "whirlwind", 		"You regain your balance.",		""
    },

    {
        "lunge",	{ 102,102,102,45,102,42,102,102,102,102,102,102,102,102,102,102 },
			  { 0,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0},
        spell_null,		TAR_IGNORE,				POS_FIGHTING,
        &gsn_lunge,		SLOT( 140),				0,			24,
        "lunge",			"!Lunge!",				""
    },

{
	 "blackjack", { 102,102,18,102,102,102,102,102,102,102,102,102,102,102,102,102 },
					{ 0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0},
	 spell_null,		TAR_CHAR_OFFENSIVE,		POS_STANDING,
	 &gsn_blackjack,		SLOT( 141),				0,			12,
	 "blackjack", 		"The pain in your head subsides. You may rise to your feet again.",		""
},

{
	"butcher",	{ 102,102,102,102,102,102,42,102,33,102,102,102,102,102,102,102 },
				{ 0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0},
	spell_null,			TAR_IGNORE,				POS_RESTING,
	&gsn_butcher,		SLOT(142),				0,			12,
	"",				"!Butcher!",			""
},

{
      "dragon skin",  { 102,102,102,102,102,102,102,102,102,102,102,102,102,35,102,102 },
				{ 0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0},
        spell_dragon_skin,    TAR_CHAR_DEFENSIVE,          	POS_STANDING,
        NULL,           	SLOT(143),      			30,            	12,
        "",     			"Your skin softens as your dragon armor fades away.",	""
},

{
	"glide", { 102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102 },
				{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	  spell_null,		TAR_CHAR_SELF,			POS_STANDING,
	  &gsn_glide,		SLOT( 144),				0,			0,
        "",				"You fold in your wings and land back on the ground.",	""
},

{
	"ravage bite",  { 102,102,102,102,102,102,74,102,102,102,102,102,102,102,102,102 },
				{ 0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0},
	 spell_null,		TAR_IGNORE,				POS_FIGHTING,
	 &gsn_ravage_bite,	SLOT( 145),				0,			24,
	 "ravaging bite",		"!Ravage Bite!",			""
},

{
	"dual wield",   { 102,102,35,25,30,32,27,102,45,36,102,102,102,102,102,102 },
                      { 0,0,4,4,4,4,4,0,4,4,0,0,0,0,0,0},
       spell_null,           	TAR_IGNORE,             	POS_STANDING,
       &gsn_dual_wield,      	SLOT( 146),        		0,      		0,
       "",                   	"!Dual Wield!",           	""
},

{
      "silence",  { 102,85,102,102,102,102,102,102,102,102,102,102,102,55,102,102 },
			{ 0,6,0,0,0,0,0,0,0,0,0,0,0,3,0,0},
       spell_silence,     	TAR_CHAR_SEMIOFFENSIVE,       POS_STANDING,
       NULL,           		SLOT( 147),      			50,            	24,
       "",     			"You regain the ability to speak again.",		""
},

{
      "traject",  { 102,30,102,102,102,102,102,102,102,102,63,35,35,35,35,102 },
			{ 0,4,0,0,0,0,0,0,0,0,4,3,3,3,3,0},
       spell_null,     		TAR_CHAR_DEFENSIVE,           POS_STANDING,
       &gsn_traject,          SLOT(148),      			0,            	12,
       "",     			"!Traject!",			""
},

{
      "play music",  { 102,102,102,102,5,102,102,102,102,102,102,102,102,102,102,102 },
			{ 0,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0},
       spell_null,     TAR_IGNORE,             POS_RESTING,
       &gsn_play,           SLOT(149),      0,            12,
       "",     "!Play Music!",	""
},

{
      "mount",  { 23,35,23,13,18,13,13,26,11,23,30,25,25,25,25,13},
			{ 2,2,2,1,2,1,1,2,1,2,2,2,2,2,2,1},
       spell_null,     TAR_IGNORE,             POS_STANDING,
       &gsn_mount,           SLOT(150),      0,            5,
       "",     "!Mount!",	""
},

{
	"shadow walk",	{102,102,102,102,102,102,102,102,102,102,102,34,102,102,102,102  },
                { 0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0 },
	spell_shadow_walk,		TAR_IGNORE,		POS_FIGHTING,
	NULL,			SLOT(151),	80,	24,
	"",			"!Shadow Walk!",		""
},

{
      "animate corpse", { 80,80,80,80,80,80,80,80,80,80,80,39,80,80,80,80  },
					{ 0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0 },
        spell_animate_corpse,     TAR_IGNORE,             POS_STANDING,
        NULL,           SLOT(152),      250,            32,
        "",     "You re-gained the power to raise the dead once more.",	""    
},

{
	"fear", {  102,102,102,102,102,102,102,102,102,102,102,26,102,102,102,102  },     
                 { 0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0 },
	spell_fear,		TAR_CHAR_SEMIOFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT( 153),	 25,	12,
	"",			"Your skin returns to its normal color and your nerves feel more at ease.",	""
},

{
      "horrid wilting", { 102,102,102,102,102,102,102,102,102,102,102,31,102,102,102,102  },
					{ 0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0 },
        spell_horrid_wilting,     TAR_IGNORE,             POS_STANDING,
        NULL,           SLOT(154),      15,            12,
        "",     "!Horrid Wilting!",	""    
},

{
      "paralysis",  { 102,102,102,102,102,102,102,102,102,102,102,48,102,102,102,102 },
			{ 0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0},
        spell_paralysis,     TAR_CHAR_SEMIOFFENSIVE,             POS_STANDING,
        NULL,           SLOT(155),      50,            24,
        "",     "The numbness leaves your body and you regain the ability to move again.",	""
},

{
	"vampiric touch", {  102,102,102,102,102,102,102,102,102,102,102,33,102,102,102,102  },
                       { 0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0 },
	spell_vampiric_touch,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT( 156),	35,	24,
	"vampiric touch",	     "!Vampiric Touch!",	""
},

{
	"undead disruption", { 102,102,102,102,102,102,102,102,102,102,102,57,102,102,102,102  },
                     { 0,1,0,0,0,0,0,0,0,0,0,2,0,0,0,0 },
	spell_undead_disruption,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(157),	65,	12,
	"evil bidding",		"!Undead Disruption!",		""
},

{
      "nightmare", { 102,102,102,102,102,102,102,102,102,102,102,63,102,102,102,102  },
					{ 0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0 },
        spell_nightmare,     TAR_IGNORE,             POS_STANDING,
        NULL,           SLOT(158),      150,            24,
        "",     "!Nightmare!",	""    
},

{
      "cavorting bones", { 102,102,102,102,102,102,102,102,102,102,102,72,102,102,102,102  },
					{ 0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0},
        spell_cavorting_bones,     TAR_IGNORE,             POS_STANDING,
        NULL,           SLOT(159),      250,            32,
        "",     "The dead will answer to your call again.",	""    
},

{
        "enervation", { 102,102,102,102,102,102,102,102,102,102,102,15,102,102,102,102 },
                          { 0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0},
        spell_enervation,   TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
        NULL,                   SLOT(160),      30,      12,
        "ray of negative energy",  "!Enervation!",	""
},

{
	"remove paralysis",	{ 102,50,102,102,102,102,102,102,102,102,102,50,102,102,102,102  },
                        { 0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0 },
	spell_remove_paralysis,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(161),	35,	12,
	""			"!Remove Paralysis!",	""
},

{
	"undetectable align", { 102,30,102,102,102,102,102,102,102,102,102,102,102,102,102,102  },
                      { 0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	spell_undetectable_align,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,			SLOT(162),	22,	12,
	"",			"Your alignment becomes noticable once again to others.",	""
},

{
	"ice shield",  {102,102,102,102,102,102,102,102,102,102,102,102,39,102,102,102 },
					{ 0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0},
	spell_ice_shield,  	TAR_CHAR_SELF,  	POS_STANDING,
	NULL,			SLOT(163),	100,	12,
	"",	"The icy shield surrounding you melts away into the ground.",	""
},

{
	"repulsion", {  102,37,102,102,102,102,102,102,102,102,102,102,102,102,102,102  },     
                 { 0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	spell_repulsion,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT( 164),	 36,	12,
	"",			"The iron bars and statuettes around you disappear.",	""
},

{   
	"firestorm", { 102,76,102,102,102,102,102,102,102,102,102,102,102,102,102,102 }, 
                           { 0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	spell_firestorm,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(165),	100,	12,
	"firestorm",		"!Firestorm!",	""
},

{
	"break enchantment",	{ 102,102,102,102,102,102,102,102,102,102,102,102,71,102,102,102  },
                        { 0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0 },
	spell_break_enchantment,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT( 166),	 75,	24,
	"",			"!Break Enchantment!",	""
},

{
	"sympathy", {  1,1,1,1,1,1,1,1,1,1,1,1,47,1,1,40  },     
                 { 0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0 },
	spell_sympathy,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT( 167),	 100,	12,
	"",			"The magical vibrations surrounding you disappear.",	""
},

{
        "displacement", { 102,102,102,102,102,102,102,102,102,102,102,102,102,65,102,102  },
				{ 0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_displacement,             SLOT( 168),       0,      0,
        "",                     "!Displacement!",   ""
},

{
        "wind wall", { 102,102,102,102,102,102,102,102,102,102,102,102,102,102,36,102  },
                { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0 },
        spell_wind_wall,             TAR_CHAR_DEFENSIVE,     POS_FIGHTING,
        NULL,                   SLOT(169),      40,     12,
        "",                     "The wall of wind surrounding you finally lightens up.",	""
},

{
	"gust of wind", { 102,102,102,102,102,102,102,102,102,102,102,102,102,102,18,102  },    
               { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0 },
	spell_gust_of_wind,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(170),	25,	18,
	"",			"You gently land on the ground as the gust of wind dies out.",	""
},

{
      "unseen servant", { 102,102,102,102,102,102,102,102,102,102,102,102,102,102,70,102  },
					{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0 },
        spell_unseen_servant,     TAR_IGNORE,             POS_STANDING,
        NULL,           SLOT(171),      250,            32,
        "",     "Unseen servants once again will come to your beck and call",	""    
},

{
	"rainbow pattern", {102,102,102,102,102,102,102,102,102,102,102,102,102,45,102,102  },
                { 0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0 },
	spell_rainbow_pattern,		TAR_IGNORE,		POS_FIGHTING,
	NULL,			SLOT(172),	45,	12,
	"",			"The rainbow disappears! You re-gain your concentration once again!",	""
},

{   
	"sunbeam", { 102,60,102,102,102,102,102,102,102,102,102,102,102,102,102,102 }, 
                           { 0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	spell_sunbeam,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(174),	100,	12,
	"sunbeam",		"!Sunbeam!",	""
},

{
	"cantrip", { 1,102,102,102,102,102,102,102,102,102,1,1,1,1,1,102  }, 
 			{ 1,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0 },
 	spell_cantrip,   TAR_IGNORE,	 POS_STANDING,
 	NULL,			SLOT(175),	 5,	 12,
 	"", 			"Your colorful aura dissolves away.",	  ""
},

{
	"disintegrate", { 102,102,102,102,102,102,102,102,102,102,102,56,102,102,102,102  },
                       { 0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0 },
	spell_disintegrate,	TAR_IGNORE,		POS_FIGHTING,
	NULL,			SLOT(176),	60, 	12,
	"disintegration",			"!Disintegrate!",		""
},

{   
	"spiritual hammer", { 102,45,102,102,102,102,102,102,102,102,102,102,102,102,102,102 }, 
                           { 0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	spell_spiritual_hammer,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(177),	30,	12,
	"spiritual hammer",		"!Spiritual Hammer!",	""
},

{
 	"darkness",	 { 102,102,102,102,102,102,102,102,102,102,102,102,102,31,102,102  },
  		 { 0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0 },
	spell_darkness, 		TAR_IGNORE, 		POS_STANDING,
 	NULL, 		SLOT(178), 65, 12,
 	"",				 "!Darkness!",		""
},

{
 	"unholy armor", { 45,45,45,45,45,45,45,45,45,45,45,45,45,45,45,45  },
			 { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
 	spell_unholy_armor,	 TAR_CHAR_SELF,	 POS_STANDING,
 	NULL, 		SLOT(179),	 20,	 12,
 	"", 			"Your unholy armor slowly dissolves.", 		""
},

{
 	"wraithform", { 102,102,102,102,102,102,102,102,102,102,102,36,102,102,102,102  },
			 { 0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0 },
 	spell_wraithform,	 	TAR_CHAR_SELF,	 POS_STANDING,
 	&gsn_wraithform,		 SLOT(180),	 20,	 12,
 	"",			 "You feel solid and heavy again.",		""
},

{
 	"nerve pinch", { 102,102,102,20,102,21,20,102,102,102,102,102,102,102,102,102  },
			 { 0,0,0,1,0,1,1,0,0,0,0,0,0,0,0,0 },
 	spell_null, 		TAR_IGNORE, 	POS_FIGHTING,
 	&gsn_nerve_pinch, 	SLOT(181),	 0,	 16,
 	"", 		"The pain from your pinched nerve leaves you.",		""
},

{
	 "pickpocket", { 102,102,27,102,102,102,102,102,102,102,102,102,102,102,102,102  },
 			{ 0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	spell_null,	 		TAR_IGNORE,		 POS_STANDING,
	&gsn_pickpocket, 		SLOT(182),	 0,	 24,
	 "",		 "!Pickpocket!",		""
},

{
	 "caltrops", { 102,102,102,24,102,25,25,102,102,102,102,102,102,102,102,102  },
			{ 0,0,0,2,0,1,2,0,0,0,0,0,0,0,0,0 },
	 spell_null,	 	TAR_IGNORE, 	POS_FIGHTING,
	 &gsn_caltrops,		 SLOT(183),	 0,	 20,
	 "caltrops",	 "You stop limping.",		""
},

{
	 "campfire", { 102,102,102,102,102,102,102,102,21,102,102,102,102,102,102,102  },
	 		{ 0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0 },
	 spell_null,	 TAR_IGNORE,	 POS_RESTING,
	 &gsn_campfire,	 SLOT(184),	 0,	 12,
	 "",			 "!Campfire!",		""
},

{
	 "search water", { 102,102,102,102,102,102,102,102,23,102,102,102,102,102,102,102  },
				  { 0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0 },
	 spell_null,	 TAR_IGNORE,	 POS_STANDING,
	 &gsn_search_water,	 SLOT(185), 0, 12,
	 "",			 "!Search Water!",		""
},

{
	 "feeblemind", { 102,102,102,102,102,102,102,102,102,102,102,102,42,102,102,102  }, 				{ 0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0 },
	 spell_feeblemind, 	TAR_CHAR_OFFENSIVE, 	POS_FIGHTING,
	 NULL,	 	SLOT(186),	 65,	 12,
	 "spell",	 "You feel your mental focus return.",		""
},

{
 	"iceblast", { 47,102,102,102,102,102,102,102,102,102,102,102,102,102,36,102  },
 			{ 1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0 },
 	spell_iceblast,		 TAR_IGNORE,	 POS_FIGHTING,
 	NULL,			 SLOT(187),	 15,	 16,
 	"iceblast",			 "!Iceblast!",			""
},

{
	 "binding", { 102,102,102,102,102,102,102,102,102,102,102,102,36,102,102,102  },
 			{ 0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0 },
 	spell_binding, 		TAR_OBJ_INV,	 POS_STANDING,
 	NULL, 		SLOT(188),		 100,		 24,
	"",		 		"!Bind Item!",			""
},

{
      "badberry", { 102,102,102,102,102,102,102,6,102,102,102,102,102,102,102,102  },
                   { 0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0 },
        spell_badberry,        TAR_IGNORE,       POS_STANDING,
        NULL,        SLOT(189),      5,       12,
        "",                "!Badberry!",                ""
},

{
 	"goodberry", { 102,102,102,102,102,102,102,3,102,102,102,102,102,102,102,102  },
			 { 0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0 },
 	spell_goodberry,		TAR_IGNORE,	 POS_STANDING,
 	NULL,			 	SLOT(190),		 5,		 12,
 	"",				 "!Goodberry!",			""
},

{ 
 	"breath of nature", { 102,102,102,102,102,102,102,32,102,102,102,102,102,102,102,102  },
 				{ 0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0},
	spell_breath_of_nature, 	TAR_CHAR_DEFENSIVE,	 POS_FIGHTING, 
	NULL,				 	SLOT(191),	 50,	 12, 
	"", 					"!Breath of Nature!",			""
}, 

{
 	"druidstaff", { 102,102,102,102,102,102,102,25,102,102,102,102,102,102,102,102  },
			 { 0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0 }, 
	spell_druidstaff,		 TAR_IGNORE,	 POS_STANDING, 
	NULL, 			 SLOT(192),	 30,	 12,
	"",				 "!Druidstaff!",				"" 
}, 

{
 	"earthmeld", { 102,102,102,102,102,102,102,34,102,102,102,102,102,102,102,102  },
 			{ 0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0 },
	spell_earthmeld,		 TAR_IGNORE,	 POS_STANDING,
	NULL,				 SLOT(193),		80,	 16,
	"",		 		"!Earthmeld!",	""
},

{
 	"windspirit", { 102,102,102,102,102,102,102,14,102,102,102,102,102,102,102,102  },
 			{ 0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0 },
	spell_windspirit, 	TAR_CHAR_DEFENSIVE, 	POS_STANDING,
	NULL, 			SLOT(194),	 20,	 12,
	"",				"The spirit of the wind flies from your body.",	""
},

{
 	"stonespirit", { 102,102,102,102,102,102,102,16,102,102,102,102,102,102,102,102  },
		 { 0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0 },
	spell_stonespirit, 	TAR_CHAR_DEFENSIVE, 	POS_STANDING,
	NULL, 			SLOT(195), 	20, 	12,
	"", 			"The stonespirit drops from you to rejoin the land.",	""
},

{
 	"summon insects", { 102,102,102,102,102,102,102,47,102,102,102,102,102,102,102,102  },
				{ 0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0 },
	spell_summon_insects,	 TAR_IGNORE,	 POS_FIGHTING,
	NULL, 			SLOT(196),	 50,	 24,
	"insect's bite", 	"The insects fly off, tormenting you no longer.",	""
},

{
 	"natures embrace", { 102,102,102,102,102,102,102,22,102,102,102,102,102,102,102,102  },
				 { 0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0 },
	spell_natures_embrace,	 TAR_CHAR_SELF,	 POS_STANDING,
	NULL,				 SLOT(197),	 75,	 24,
	"",		 "Nature's protective embrace leaves your body.",		""
},

{
 	"natures warmth", { 102,102,102,102,102,102,102,21,102,102,102,102,102,102,102,102  },
			{ 0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0 },
	spell_natures_warmth, TAR_CHAR_SELF, POS_STANDING,
	NULL, 			SLOT(198), 75, 40,
	"", 		"You feel the warm touch of nature leave you.",			""
},

{
 	"bearcall", { 102,102,102,102,120,102,102,102,23,102,102,102,102,102,102,102  },
			{ 0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0 }, 
	spell_null, 	TAR_IGNORE, 	POS_STANDING,
	&gsn_bear_call, 	SLOT(199), 	30, 	12,
	"", 			"You regain your voice to call bears.",		""
},

{
 	"ambush", { 102,102,102,102,120,102,102,102,37,102,102,102,102,102,102,102  },
		 { 0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,0 }, 
	spell_null, 	TAR_IGNORE, 	POS_STANDING,
	&gsn_ambush, 	SLOT(200), 	0, 	24,
	"surprise attack", 	"!Ambush!",			""
},

 {
      "pillify", { 102,40,102,102,120,102,102,28,102,102,102,102,102,102,43,102  },
		   { 0,2,0,0,0,0,0,2,0,0,0,0,0,0,3,0 }, 
      spell_null,             TAR_IGNORE,             POS_SLEEPING,
      &gsn_pillify,            SLOT( 201),       100,      25,
      "failure",                     "!Pillify!",           ""
 },

{
 	"herbal discern", { 102,102,102,102,102,102,102,34,20,102,102,102,102,102,102,102  },
		 	{ 0,0,0,0,0,0,0,2,4,0,0,0,0,0,0,0 },
	spell_null, 	TAR_CHAR_DEFENSIVE, 	POS_RESTING,
	&gsn_herbal_discern, 		SLOT(202),	 0,	 12,
	"",		 "You may now find herbs again.",			""
},

{
 	"enlightenment", { 102,43,102,102,102,102,102,102,102,102,102,102,102,102,102,102  },
			{ 0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	spell_enlightenment, 	TAR_CHAR_DEFENSIVE, 	POS_STANDING,
	NULL, 		SLOT(203),	 20, 	12,
	"",		 "You feel your new insight slip away.",		""
},

{
 	"bad luck", { 102,102,102,102,102,102,102,102,102,102,27,102,102,102,102,102  },
			{ 0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0 },
	spell_bad_luck, 		TAR_CHAR_SEMIOFFENSIVE,	 POS_FIGHTING,
	NULL, 		SLOT(204),	 30,	 12,
	"", 		"Your luck returns to normal.",		""
},

{
      "mountjack", { 102,102,63,102,102,102,102,102,102,102,85,102,102,102,102,102 },
                    { 0,0,1,0,0,0,0,0,0,0,2,0,0,0,0,0},
      spell_null,             TAR_IGNORE,             POS_STANDING,
      &gsn_mountjack,          SLOT( 205),        0,     12,
      "",             "!Mountjack!",		""
},

{
 	"word of death", { 102,102,102,102,102,102,102,102,102,102,102,95,102,102,102,102 },
			 { 0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0},
	spell_word_of_death,	 TAR_CHAR_OFFENSIVE,	 POS_STANDING,
	NULL,			 SLOT(206),	 75,	 22,
	"",			 "!Word of Death!",				""
},

{
 	"blade barrier", { 26,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102},
 			{ 2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	spell_blade_barrier,	 TAR_CHAR_OFFENSIVE,	 POS_FIGHTING,
	NULL,			 SLOT(207),	 15,	 16,
	"blade barrier",	 "!Blade Barrier!",		""
},

{
 	"flyingswords", { 39,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102 },
 			{ 2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	spell_flyingswords,	 TAR_CHAR_OFFENSIVE,	 POS_FIGHTING,
	NULL,			 SLOT(208),	 20,	 12,
	"sword's fury",	 "!Flyingswords!",	""
},

{
 	"aura cleanse", { 102,90,102,102,102,102,102,102,102,102,102,102,102,102,102,102 },
			{ 0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	spell_aura_cleanse,	 TAR_OBJ_INV,	 POS_STANDING,
	NULL,			 SLOT(209),	 100,	 12,
	"",		 "!Aura Cleanse!",			""
},

{
 	"bark skin", { 102,102,102,102,102,102,102,10,102,102,102,102,102,102,102,102 },
 			{ 0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0},
	spell_bark_skin,		 TAR_CHAR_SELF,	 POS_STANDING,
	NULL,			 SLOT(210),	 20,	 18,
	"",		 "Your skin loses its rough texture.",		""
},

{
 	"coldfire", { 102,102,102,102,102,102,102,102,102,102,102,2,102,102,102,102 },
 			{ 0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0},
	spell_coldfire,		 TAR_CHAR_OFFENSIVE,	 POS_FIGHTING,
	NULL,			 SLOT(211),	 15,	 8,
	"coldfire",		 "!Coldfire!",		""
},

{
 	"brew", { 86,27,102,102,102,102,102,43,102,102,102,41,43,39,40,102 },
 		{ 6,6,0,0,0,0,0,4,0,0,0,4,4,4,4,0},
	spell_null,			 TAR_IGNORE,	 POS_STANDING,
	&gsn_brew,		 SLOT(212),	 0,	 12,
	"",			 "!Brew!",			""
},

{
 	"scribe", { 84,25,102,102,102,102,102,41,102,102,102,39,41,37,38,102 },
		 { 6,6,0,0,0,0,0,4,0,0,0,4,4,4,4,0},
	spell_null,			 TAR_IGNORE,	 POS_STANDING,
	&gsn_scribe,	 SLOT(213),	 0,	 12,
	"",			 "!Scribe!",		""
},

{
 	"battle tactics", { 12,14,13,6,6,8,6,7,15,8,17,21,21,21,21,102 },
			{ 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0 },
	spell_null, 		TAR_IGNORE, 	POS_STANDING,
	&gsn_fight_pos,	 SLOT(214),		 0,		 0,
	"", 			"!Fight Positioning!",			""
},

{
 	"trophy", { 35,35,35,35,35,35,35,35,29,35,35,35,35,35,35,35},
                   {0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0 },
	spell_null,		 	TAR_IGNORE,	 	POS_STANDING,
	&gsn_trophy, 	SLOT(215),	 0,	 12,
	"" 			"!Trophy!",			""
},

{
 	"battlestaff", { 42,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102},
                   {4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	spell_battlestaff,	 TAR_IGNORE,	 POS_STANDING,
	NULL, 		SLOT(216),	 10,	 12,
	"",			 "!Battlestaff!",			""
},

{
 	"wear chain", { 1,1,102,1,1,1,1,102,1,102,102,102,102,102,102,1},
                   {4,4,0,2,3,1,2,0,2,0,0,0,0,0,0,1 },
	spell_null,		 TAR_IGNORE,	 POS_RESTING,
	&gsn_wear_chain,	 SLOT(217),	 0,	 0,
	"",			 "!wear chain!",		""
}, 

{
 	"wear leather", { 1,1,1,1,1,1,1,1,1,1,1,102,102,102,102,1},
                   {1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,1 },
	spell_null,		 TAR_IGNORE,	 POS_RESTING,
	&gsn_wear_leather, SLOT(218),	 0,	 0,
	"",		 "!wear leather!",		""
}, 

{
 	"wear plate", { 102,102,102,4,102,2,102,102,102,102,102,102,102,102,102,1},
                   {0,0,0,2,0,1,0,0,0,0,0,0,0,0,0,1 },
	spell_null,		 TAR_IGNORE,	 POS_RESTING,
	&gsn_wear_plate,	 SLOT(219),	 0,	 0,
	"",		 "!wear plate!",			""
},

{
 	"knock", { 60,102,102,102,102,102,102,102,102,102,102,21,45,25,23,102},
                   {2,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0 },
	spell_knock,	 TAR_IGNORE,	 POS_STANDING,
	NULL,			 SLOT(220), 	5, 	12,
	"",				 "!Knock!",			""
},

{
	"flame rune", { 102,102,102,102,102,102,102,102,102,102,102,102,63,102,102,102},
                   {0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0 },
	spell_flame_rune,		TAR_OBJ_INV,	POS_STANDING,
	NULL,			SLOT(221),	100,	24,
	"",			"!Flame Rune!",			""
},

{
	"frost rune", { 102,102,102,102,102,102,102,102,102,102,102,102,67,102,102,102},
                   {0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0 },
	spell_frost_rune,		TAR_OBJ_INV,	POS_STANDING,
	NULL,			SLOT(222),	100,	24,
	"",			"!Frost Rune!",			""
},

{
	"shocking rune", { 102,102,102,102,102,102,102,102,102,102,102,102,73,102,102,102},
                   {0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0 },
	spell_shocking_rune,		TAR_OBJ_INV,	POS_STANDING,
	NULL,			SLOT(223),	100,	24,
	"",			"!Shocking Rune!",			""
},

{
	"drain rune", { 102,102,102,102,102,102,102,102,102,102,102,45,102,102,102,102},
                   {0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0 },
	spell_drain_rune,		TAR_OBJ_INV,	POS_STANDING,
	NULL,			SLOT(224),	100,	24,
	"",			"!Drain Rune!",			""
},

{ 
	"fast healing",	{ 12,9,16,6,16,6,6,3,6,16,14,12,12,12,12,12},
         	        { 4,5,6,4,6,4,4,3,4,6,4,4,4,4,4,1},
	spell_null,		TAR_IGNORE,		POS_SLEEPING,
	&gsn_fast_healing,	SLOT( 225),	0,	0,
	"",			"!Fast Healing!",	""
},

{
	"haggle", { 102,102,1,14,1,14,14,10,14,1,24,102,102,102,102,1},
         	  { 0,0,3,6,3,6,6,5,6,3,4,0,0,0,0,1},
	spell_null,		TAR_IGNORE,		POS_RESTING,
	&gsn_haggle,		SLOT( 226),	0,	0,
	"",			"!Haggle!",		""
},

{
	"hide",	{ 102, 102,1,102,10,102,102,102,27,1,47,102,102,102,102,102},
        		{ 0,0,4,0,4,0,0,0,5,4,4,0,0,0,0,0},
	spell_null,		TAR_IGNORE,		POS_RESTING,
	&gsn_hide,		SLOT( 227),	 0,	12,
	"",			"!Hide!",		""
},
 #ifdef LANGUAGES
{
      "common",{  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
		{ 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
      spell_null,             TAR_IGNORE,             POS_STANDING,
      &gsn_lang_common,           SLOT( 228),       0,      0,
      "",                     "!Common!",            ""
},
{
      "elven tongue",{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
		{ 0,0,0,0,0,0,0,2,0,0,0,2,2,2,1,1},
      spell_null,             TAR_IGNORE,             POS_STANDING,
      &gsn_lang_elven,            SLOT( 229),       0,      0,
      "",                     "!Elven!",             ""
},
{
      "dwarven tongue",{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
		{ 0,0,0,0,0,0,0,0,0,0,0,2,2,2,1,1},
      spell_null,             TAR_IGNORE,             POS_STANDING,
      &gsn_lang_dwarven,            SLOT( 230),       0,      0,
      "",                     "!Dwarven!",             ""
},
{
      "drow tongue",{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
		{0,0,0,0,0,0,0,1,0,0,0,1,1,1,1,1},
      spell_null,             TAR_IGNORE,             POS_STANDING,
      &gsn_lang_drow,            SLOT( 231),       0,      0,
      "",                     "!Drow!",             ""
},

{
      "silvanesti tongue",{  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
		{0,0,0,0,0,0,0,1,0,0,0,1,1,1,1,1},
      spell_null,             TAR_IGNORE,             POS_STANDING,
      &gsn_lang_silvanesti,           SLOT( 232),       0,      0,
      "",                     "!Silvanesti!",            ""
},
{
      "qualinesti tongue",{  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
		{0,0,0,0,0,0,0,1,0,0,0,1,1,1,1,1},
      spell_null,             TAR_IGNORE,             POS_STANDING,
      &gsn_lang_qualinesti,           SLOT( 233),       0,      0,
      "",                     "!Qualinesti!",            ""
},
{
      "kagonesti tongue",{  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
		{0,0,0,0,0,0,0,1,0,0,0,1,1,1,1,1},
      spell_null,             TAR_IGNORE,             POS_STANDING,
      &gsn_lang_kagonesti,           SLOT( 234),       0,      0,
      "",                     "!Kagonesti!",            ""
},
{
      "dargonesti tongue",{  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
		{0,0,0,0,0,0,0,1,0,0,0,1,1,1,1,1},
      spell_null,             TAR_IGNORE,             POS_STANDING,
      &gsn_lang_dargonesti,           SLOT( 235),       0,      0,
      "",                     "!Dargonesti!",            ""
},

{
      "dimernesti tongue",{  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
		{0,0,0,0,0,0,0,1,0,0,0,1,1,1,1,1},
      spell_null,             TAR_IGNORE,             POS_STANDING,
      &gsn_lang_dimernesti,           SLOT( 236),       0,      0,
      "",                     "!Dimernesti!",            ""
},
{
      "kender tongue",{  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
			{0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1},
      spell_null,             TAR_IGNORE,             POS_STANDING,
      &gsn_lang_kender,           SLOT( 237),       0,      0,
      "",                     "!Kender!",            ""
},
{
      "minotaur tongue",{  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
			{0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1},
      spell_null,             TAR_IGNORE,             POS_STANDING,
      &gsn_lang_minotaur,           SLOT( 238),       0,      0,
      "",                     "!Minotaur!",            ""
},
{
      "ogre tongue",{  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
				{0,0,0,0,0,1,0,0,0,0,0,1,1,1,1,1 },
      spell_null,             TAR_IGNORE,             POS_STANDING,
      &gsn_lang_ogre,           SLOT( 239),       0,      0,
      "",                     "!Ogre!",            ""
},

{
      "draconian tongue",{  1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,1},
				{0,0,0,0,0,1,0,0,0,0,0,2,2,2,2,1 },
      spell_null,             TAR_IGNORE,             POS_STANDING,
      &gsn_lang_draconian,           SLOT( 240),       0,      0,
      "",                     "!Draconian!",            ""
},

{
      "solamnic tongue",{  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
				{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 },
	spell_null,             TAR_IGNORE,             POS_STANDING,
      &gsn_lang_solamnic,           SLOT( 241),       0,      0,
      "",                     "!Solamnic!",            ""
},

{
      "thieves cant",{  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
				{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 },
	spell_null,             TAR_IGNORE,             POS_STANDING,
      &gsn_lang_thieves_cant,           SLOT( 242),       0,      0,
      "",                     "!Thieves Cant!",            ""
},
#endif

{
	"lore",	{ 37,102,15,102,5,27,34,3,102,102,102,102,102,102,102,2 },
           		{2,0,1,0,3,10,10,2,0,0,0,0,0,0,0,1 },
	spell_null,			TAR_IGNORE,				POS_RESTING,
	&gsn_lore,			SLOT( 243),				0,			36,
	"",				"!Lore!",				""
},

{
	"meditation", {  6,6,102,102,102,50,102,102,102,102,19,6,6,6,6,102 },
                      { 5,5,0,0,0,10,0,0,0,0,8,5,5,5,5,0 },
	spell_null,		TAR_IGNORE,		POS_SLEEPING,
	&gsn_meditation,	SLOT( 244),	0,	0,
	"",			"Meditation",		""
},

{
	"peek",	{  102,102,1,102,102,102,102,102,102,1,102,102,102,102,102,102
},
        		{0,0,3,0,0,0,0,0,0,3,0,0,0,0,0,0 },
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_peek,		SLOT( 245),	 0,	 0,
	"",			"!Peek!",		""
},

{
	"pick lock", { 20,20,7,20,20,20,20,20,20,50,41,20,20,20,20,20 },
	             { 0,0,1,0,1,0,0,1,0,1,1,0,0,0,0,0 },
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_pick_lock,		SLOT( 246),	 0,	12,
	"",			"!Pick!",		""
},

{
	"sneak", { 102,102,1,102,9,102,102,102,30,1,8,102,102,102,102,102 },
         	 { 0,0,4,0,4,0,6,0,6,4,4,0,0,0,0,0 },
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_sneak,		SLOT( 247),	 0,	12,
	"",			"You no longer feel stealthy.",	""
},

{
	"steal", { 1,1,5,1,1,1,1,1,1,1,18,1,1,1,1,102 },
        	 { 0,0,4,0,0,0,0,0,0,0,2,0,0,0,0,0 },
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_steal,		SLOT( 248),	 0,	24,
	"",			"!Steal!",		""
},

{
	"scrolls", {  1,1,1,1,13,1,1,38,1,1,1,1,1,1,1,102 },
        	   { 1,1,0,0,1,0,0,4,0,0,4,1,1,1,1,0 },
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_scrolls,	SLOT( 249),		20,	24,
	"",			"!Scrolls!",		""
},

{
	"staves", {  1,1,102,102,102,102,102,102,102,102,102,1,1,1,1,102 },
          	  { 2,3,0,0,0,0,0,0,0,0,0,2,2,2,2,0 },
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_staves,	SLOT( 250),		20,	12,
	"",			"!Staves!",		""
},
    
{
	"wands", {  1,1,102,102,102,102,102,102,102,102,102,1,1,1,1,102 },
           	 { 2,3,0,0,0,0,0,0,0,0,0,2,2,2,2,0 },
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_wands,		SLOT( 251),		20,	12,
	"",			"!Wands!",		""
},

{
	"recall", {  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 },
         	  { 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 },
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_recall,	SLOT( 252),		0,	12,
	"",			"!Recall!",		""
},

{
	"wedge", {   },
		 {   },
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_wedge,		SLOT( 253),	0,	12,
	"",			"!Wedge!",		""
},

{
	"pry",	{},
		{},
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_pry,		SLOT( 254),	0,	12,
	"",			"!Pry!",		""
},

{
	"chant of battle", { 102,102,102,102,32,102,102,102,102,102,102,102,102,102,102,102 },
                      { 0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0 },
	spell_chant_of_battle,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,			SLOT( 255),	45,	12,
	"",			"The power of the battle chant fades away.",	""
},

{
	"chords of dissonance", { 102,102,102,102,12,102,102,102,102,102,102,102,102,102,102,102 },  
                      { 0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0 },
	spell_chords_of_dissonance,	TAR_IGNORE,		POS_FIGHTING,
	NULL,			SLOT( 256),	15,	12,
	"discordant sounds",		"!Chords of Dissonance!",		""
},

{
	"bow", { 102,102,102,1,102,1,102,1,1,102,102,102,102,102,102,102 },
                { 0,0,0,2,0,2,0,2,2,0,0,0,0,0,0,0 },
	spell_null,             TAR_IGNORE,             POS_FIGHTING,
	&gsn_bow,     			SLOT( 257),       0,      0,
	"arrow",                     "!Bow!",		""
},

{
      "grasping roots",  {102,102,102,102,102,102,102,102,17,102,102,102,102,102,102,102},
                   {0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0 },
        spell_grasping_roots,     TAR_CHAR_OFFENSIVE,             POS_STANDING,
        NULL,           SLOT(258),      40,            24,
        "",     "The roots grasping your feet disappear back into the ground.",	""
},

{
	"cloak of starlight", {20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20},
                   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	spell_cloak_starlight, 		TAR_CHAR_SELF, 	POS_STANDING, 
	NULL, 		SLOT(259), 		15, 		12,
	"",		 "Your cloak of starlight returns to the sky.",		""
},

{
 	"gaseous form", {18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18},
                   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }, 
	spell_gaseous_form, 		TAR_CHAR_SELF, 	POS_STANDING, 
	NULL, 		SLOT(260), 		45, 		12,
	"", 		"The gas surrounding you fades away.",		""
},

{
 	"dark taint", {15,15,15,15,15,15,15,15,15,15,15,10,15,15,15,15},
                   {0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0 }, 
	spell_dark_taint, 		TAR_CHAR_SEMIOFFENSIVE,	POS_FIGHTING,
	NULL, 		SLOT( 261), 		50, 		12,
	"", 		"You are no longer host to dark energies.", 		""
},

{
      "battlecry", {10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
                   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
      spell_null,                  TAR_CHAR_OFFENSIVE,      POS_FIGHTING, 
      &gsn_battlecry,   SLOT( 262),            0,           12,
      "battlecry",        "!Battlecry!",             ""
},

{
 	"drain blood", {18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18},
                   {0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0 },
	spell_null, 		TAR_IGNORE, 	POS_STANDING,
	&gsn_drain, 	SLOT(263), 		11, 		12,
	"", 			"!Drain!",			""
},

{
 	"warding", {30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30},
                   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	spell_ward, 		TAR_CHAR_SELF, 	POS_STANDING,
	NULL, 		SLOT( 264), 	20, 		30,
	"", 			"The warding energies subside.",		""
},

{
 	"forget", {102,102,102,102,102,102,102,102,102,102,102,82,102,102,102,102},
                   {0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0 },
	spell_forget, 	TAR_CHAR_SEMIOFFENSIVE, 	POS_STANDING,
	NULL, 		SLOT( 265), 	30, 		16,
	"", 		"The void surrounding your memories departs.",		""
},

{
 	"truefire", {50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50},
                   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	spell_truefire, 	TAR_IGNORE, 		POS_STANDING,
	NULL, 		SLOT(266), 		15, 		12,
	"", 			"!Truefire!",			""
},

{
 	"forgedeath", {1,1,1,1,1,1,1,1,1,1,1,40,1,1,1,102},
                   {0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0 },
  	spell_forgedeath,      TAR_CHAR_SELF,          POS_STANDING,
  	NULL,                  SLOT(267),      25,     18,
  	"",                    "You feel death leave you.",    ""
},

{
 	"bind wounds", {102,102,102,30,102,36,35,102,102,102,102,102,102,102,102,102},
                   {0,0,0,2,0,2,2,0,0,0,0,0,0,0,0,0 },
	spell_null, 		TAR_IGNORE, 		POS_STANDING,
	&gsn_bind, 			SLOT(268), 	0, 	14,
	"", 		"Your bindings have staunched the bleeding.",		""
},

{
 	"breakneck", {102,102,102,35,102,102,39,102,102,102,102,102,102,102,102,102},
                   {0,0,0,2,0,0,2,0,0,0,0,0,0,0,0,0 },
	spell_null, 		TAR_IGNORE, 		POS_STANDING,
	&gsn_breakneck, 		SLOT(269), 	0, 	24,
	"pounding neckbreak", 		"Your pounding headache finally subsides.",	""
},

{
 	"strangle", {102,102,102,102,102,102,102,102,102,40,102,102,102,102,102,102},
                   {0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0 },
	spell_null, 		TAR_IGNORE, 		POS_STANDING,
	&gsn_strangle, 		SLOT(270), 	0, 	24,
	"strangling whip", 	"You are able to breathe normally again.",	""
},

{
 	"guard", {102,102,102,6,102,6,6,102,9,102,102,102,102,102,102,102},
                   {0,0,0,1,0,1,1,0,1,0,0,0,0,0,0,0 },
	spell_null, 		TAR_IGNORE, 		POS_FIGHTING,
	&gsn_guard, 		SLOT(271), 	0, 	12,
	"", 				"!Guard!",			""
},

{
 	"cutthroat", {102,102,102,102,102,102,102,102,102,45,102,102,102,102,102,102},
                   {0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0 },
	spell_null, 		TAR_IGNORE, 		POS_STANDING,
	&gsn_cutthroat, 		SLOT(272), 	0, 	24,
	"cutthroat", 		"Your neck stops bleeding.",		""
},

{
 	"headbutt", {102,102,102,4,102,102,4,102,102,102,102,102,102,102,102,102},
                   {0,0,0,1,0,0,1,0,0,0,0,0,0,0,0,0 },
	spell_null, 		TAR_IGNORE, 		POS_RESTING,
	&gsn_head_butt, 		SLOT(273), 	0, 	12,
	"headbutt", 		"!Headbutt!",		""
},

{
      "hunt",{102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102},
                   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
      spell_null,             TAR_IGNORE,             POS_RESTING,
      &gsn_hunt,              SLOT( 274),        0,     12,
      "",                     "!Hunt!"			""
},

{
	"closing weave", {102,102,102,102,102,102,102,102,102,102,102,102,102,102,51,102},
                   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0 },
	spell_closing_weave,	        TAR_IGNORE,	  	POS_STANDING,
	NULL,		        	SLOT(275),	       50,		24,
	"",			        "!Closing Weave!",	""
},

{
      "cause discord", {102,102,102,102,102,102,102,102,102,102,102,102,102,57,102,102},
                   {0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0 },
      spell_cause_discord,   TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
      NULL,                   SLOT(276),      50,      12,
	"",  			"!Cause Discord!",        ""
},

{
	"shield rush", {102,102,102,102,102,102,55,102,102,102,102,102,102,102,102,102},
                   {0,0,0,0,0,0,3,0,0,0,0,0,0,0,0,0 },
	spell_null,			TAR_CHAR_OFFENSIVE,	POS_STANDING,
	&gsn_shield_rush,		SLOT(277),		0,	36,
	"shield rush",		"!Shield Rush!",		""
},

{
      "shifting shadows", {102,102,102,102,102,102,102,102,102,102,102,102,102,48,102,102},
                   {0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0 },
      spell_shifting_shadows,   TAR_CHAR_SELF,     POS_STANDING,
      NULL,                   SLOT(278),      75,      12,
	"",  	"The shadows surrounding you shift back to their normal state!",       ""
},

{
 	"assassinate", {102,102,102,102,102,102,102,102,102,95,102,102,102,102,102,102},
                   {0,0,0,0,0,0,0,0,0,8,0,0,0,0,0,0 },
	spell_null, 		TAR_IGNORE, 		POS_STANDING,
	&gsn_assassinate, 		SLOT(279), 	0, 	24,
	"assassination", 		"!Assassinate!",		""
},

{
      "crush",{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
                   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
      spell_null,             TAR_IGNORE,             POS_FIGHTING,
      &gsn_crush,      		SLOT( 280),        0,     0,
      "crushing tail",                   	"!crush!",	""
},

{
      "tail", {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
                   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
      spell_null,             TAR_IGNORE,             POS_FIGHTING,
      &gsn_tail,      		SLOT( 281),        0,     0,
      "thrashing tail",                     "!Tail!",		""
},

{
 	"alter reality", {102,102,102,102,102,102,102,102,102,102,102,102,102,90,102,102},
                   {0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0 },
	spell_alter_reality, 		TAR_CHAR_DEFENSIVE, 	POS_STANDING,
	NULL, 			SLOT( 282), 		100, 		12,
	"", 		"The illusion of nirvana fades away from your eyes.",		""
},

{
 	"invul", {102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102},
                   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	spell_null, 		TAR_CHAR_DEFENSIVE, 	POS_RESTING,
	&gsn_invul, 		SLOT(283), 	10, 	12, 
	"", 		"You are no longer invulnerable.",		""
},

{
 	"doppelganger", {102,102,102,102,102,102,102,102,102,102,102,102,102,62,102,102},
                   {0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0 },
	spell_doppelganger, 	TAR_CHAR_SEMIOFFENSIVE, 	POS_STANDING,
	&gsn_doppelganger, 	SLOT(284), 	75, 	12,
	"", 			"Your disguise wears off.",		""
},

{
 	"disguise", {102,102,50,102,102,102,102,102,102,102,102,102,102,102,102,102},
                   {0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	spell_null, 		TAR_IGNORE, 		POS_STANDING,
	&gsn_disguise, 		SLOT(285), 	0, 	24,
	"", 			"Your disguise wears off.",		""
},

{
      "charge", {40,40,40,40,40,40,40,40,40,40,40,40,40,40,40,40},
                   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
      spell_null,                  TAR_IGNORE,      POS_STANDING, 
      &gsn_charge,   		SLOT( 286),            0,           12,
      "",        "!Charge!",             ""
},

{
      "pardon", {40,40,40,40,40,40,40,40,40,40,40,40,40,40,40,40},
                   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
      spell_null,                  TAR_IGNORE,      POS_STANDING, 
      &gsn_pardon,   		SLOT( 287),            0,           12,
      "",        "!Pardon!",             ""
},

{
      "backup", {15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15},
                   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
      spell_null,                  TAR_IGNORE,      POS_STANDING, 
      &gsn_backup,   		SLOT( 288),            15,           12,
      "",        "You may now call for backup again!",             ""
},

{
 	"shackles", {110,110,110,110,110,110,110,110,110,110,110,110,110,110,110,110},
                   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	spell_shackles, 		TAR_CHAR_DEFENSIVE, 	POS_STANDING,
	&gsn_shackles, 		SLOT(289), 			5, 		12,
	"", 		"A guard enters the room and removes your shackles.",		""
},

{
	"apprehend", {40,40,40,40,40,40,40,40,40,40,40,40,40,40,40,40},
                   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	spell_null, 		TAR_IGNORE, 		POS_RESTING,
	&gsn_apprehend, 		SLOT(290), 			0, 		12,
	"", 				"!apprehend!",		""
},

{
 	"mounted combat", {26,102,102,26,102,26,26,102,26,102,102,102,102,102,102,102},
                   {3,0,0,3,0,3,3,0,3,0,0,0,0,0,0,0 },
	spell_null, 		TAR_IGNORE, 		POS_FIGHTING,
	&gsn_mounted_combat, 	SLOT(291), 			0, 		12,
	"", 				"You dismount.",		""
},

{
 	"preservation", {102,102,102,102,102,102,102,102,102,102,102,18,102,102,102,102},
                   {0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0 },
	spell_preserve_part, 	TAR_OBJ_INV, 		POS_STANDING,
	NULL, 			SLOT(292), 			25, 		14,
	"", 				"!Preserve Part!",	""
},

{
 	"looking glass", {102,102,102,102,102,102,102,102,102,102,102,102,102,37,102,102},
                   {0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0 },
	spell_looking_glass, 	TAR_CHAR_SEMIOFFENSIVE, POS_STANDING,
	NULL, 			SLOT(293), 			25, 		36,
	"", 			"Uoy ssap kcab hguorht eht gnikool ssalg.",	""
},

{
 	"seal door", {102,102,102,102,102,102,102,102,102,102,102,32,32,32,32,102},
                   {0,0,0,0,0,0,0,0,0,0,0,2,2,2,2,0 },
	spell_seal_door, 	TAR_IGNORE, 		POS_STANDING,
	NULL, 			SLOT(293), 			40, 		18,
	"", 			"!Seal Door!",			""
},

{
 	"unseal door", {102,102,102,102,102,102,102,102,102,102,102,32,32,32,32,102},
                   {0,0,0,0,0,0,0,0,0,0,0,2,2,2,2,0 },
	spell_unseal_door, 	TAR_IGNORE, 		POS_STANDING,
	NULL, 			SLOT(293), 			50, 		18,
	"", 			"!Unseal Door!",			""
},

{
 	"impale", {102,102,102,7,102,102,102,102,102,102,102,102,102,102,102,102},
                   {0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0 },
	spell_null, 		TAR_IGNORE, 		POS_STANDING,
	&gsn_impale, 		SLOT(294), 			0, 		28,
	"thrusting spear", 	"!Impale!",			""
},

{
 	"sate", {102,28,102,102,102,102,102,102,102,102,102,102,102,102,102,102},
                   {0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	spell_sate, 		TAR_CHAR_DEFENSIVE, 	POS_STANDING,
	NULL, 			SLOT(295), 			5, 		12,
	"", 				"!Sate!",			""
},

{
 	"attunement", {102,102,102,102,102,102,102,102,102,102,102,102,19,102,102,102},
                   {0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0 },
	spell_attunement, 	TAR_CHAR_DEFENSIVE, 	POS_STANDING,
	NULL, 			SLOT(296), 			20, 		12,
	"", 		"You are no longer attuned to the web of life.",	""
},

{
 	"invigorate", {102,28,102,102,102,102,102,102,102,102,102,102,102,102,102,102},
                   {0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	spell_invigorate, 	TAR_CHAR_DEFENSIVE, 	POS_STANDING,
	NULL, 			SLOT(297), 			25, 		50,
	"", 				"!Invigorate!",		""
},

{
 	"pass without trace", {102,102,102,102,102,102,102,53,102,102,102,102,102,102,102,102},
                   {0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0 },
	spell_pass_without_trace, TAR_CHAR_SELF, 		POS_STANDING,
	NULL, 			SLOT(298), 			10, 		12,
	"", 				"You feel your fluid movement subside.", 	""
},

{
 	"luck", {102,102,102,102,102,102,102,102,102,102,24,102,102,102,102,102},
                   {0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0},
	spell_luck, 		TAR_CHAR_DEFENSIVE, 	POS_FIGHTING,
	NULL, 			SLOT(299), 			30, 		12,
	"", 				"You do not seem to feel as lucky.",	""
},

{
 	"delusions of grandeur", {102,102,102,102,102,102,102,102,102,102,102,102,102,6,102,102},
                   {0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0 },
	spell_grandeur, 		TAR_CHAR_SEMIOFFENSIVE, POS_STANDING,
	NULL, 			SLOT(300), 			20, 		12,
	"", 				"Your regular self-image returns.",		""
},

{
 	"illusion", {102,102,102,102,102,102,102,102,102,102,102,102,102,50,102,102},
                   {0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0 },
	spell_illusion, 		TAR_IGNORE, 		POS_STANDING,
	NULL, 			SLOT(301), 			5, 		12,
	"", 				"!Illusion!",		""
},

{
 	"mirror", {102,102,102,102,102,102,102,102,102,102,102,102,102,18,102,102},
                   {0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0 },
	spell_mirror, 		TAR_CHAR_DEFENSIVE, 	POS_STANDING,
	&gsn_mirror, 		SLOT(302), 			30, 		12,
	"", 			"Your mirrored image disappears.",		""
},

{
 	"glamour", {102,102,102,102,102,102,102,102,102,102,102,102,6,14,102,102},
                   {0,0,0,0,0,0,0,0,0,0,0,0,2,2,0,0 },
	spell_glamour, 		TAR_CHAR_DEFENSIVE, 	POS_STANDING,
	NULL, 			SLOT(303), 			20, 		12,
	"", 			"You feel less glamourous.",		""
},

{
 	"purify", {102,26,102,102,102,102,102,8,102,102,102,102,102,102,102,102},
                   {0,2,0,0,0,0,0,2,0,0,0,0,0,0,0,0 },
	spell_purify, 		TAR_OBJ_INV, 		POS_STANDING,
	NULL, 			SLOT(304), 			10, 		24,
	"", 				"!Purify!",			""
},

{
 	"jab", {102,102,102,102,102,102,102,5,102,102,102,8,8,8,8,102},
                   {0,0,0,0,0,0,0,2,0,0,0,2,2,2,2,0 },
	spell_null, 		TAR_IGNORE, 		POS_FIGHTING,
	&gsn_jab, 			SLOT(305),	 		0, 		12,
	"striking jab", 		"!Jab!",			""
},

{
 	"cleave", {102,102,102,28,102,28,31,102,102,102,102,102,102,102,102,102},
                   {0,0,0,2,0,2,2,0,0,0,0,0,0,0,0,0 },
	spell_null, 		TAR_IGNORE, 		POS_STANDING,
	&gsn_cleave, 		SLOT(306), 			0, 		28,
	"massive cleave", 	"!Cleave!",			""
},

{
 	"tumble", {102,102,11,102,11,102,102,102,102,11,102,102,102,102,102,102},
                   {0,0,4,0,3,0,0,0,0,4,0,0,0,0,0,0 },
	spell_null, 		TAR_IGNORE, 		POS_FIGHTING,
	&gsn_tumble, 		SLOT(307), 			0, 		0,
	"", 				"!Tumble!",			""
},

{
 	"trammel", {102,102,102,17,14,16,102,102,102,102,102,102,102,102,102,102},
                   {0,0,0,2,3,2,0,0,0,0,0,0,0,0,0,0 },
	spell_null, 		TAR_IGNORE, 		POS_FIGHTING,
	&gsn_trammel, 		SLOT(308), 			0, 		20,
	"trammel", 			"!Trammel!",		""
},

{
 	"sweep attack", {16,102,102,102,102,102,102,16,102,102,102,102,102,102,102,102},
                   {3,0,0,0,0,0,0,3,0,0,0,0,0,0,0,0 },
	spell_null, 		TAR_IGNORE, 		POS_FIGHTING,
	&gsn_sweep, 		SLOT(309), 			0, 		20,
	"sweep attack", 		"!Sweep!",			""
},

{
 	"flank attack", {102,102,102,33,102,22,102,102,102,102,102,102,102,102,102,102},
                   {0,0,0,3,0,2,0,0,0,0,0,0,0,0,0,0 },
	spell_null, 		TAR_IGNORE, 		POS_FIGHTING,
	&gsn_flank_attack, 	SLOT(310), 			0, 		12,
	"cunning flank attack", 	"!Flank Attack!",		""
},

{
 	"vermin plague", {102,102,102,102,102,102,102,27,102,102,102,102,102,102,102,102},
                   {0,0,0,0,0,0,0,3,0,0,0,0,0,0,0,0 },
	spell_vermin_plague, 	TAR_CHAR_OFFENSIVE, 	POS_FIGHTING,
	NULL, 			SLOT(311), 			35, 		12,
	"vermin", 			"!vermin!",				""
}, 

{
 	"vortex", {102,102,102,102,102,102,102,12,102,102,102,102,102,102,102,15},
                   {0,0,0,0,0,0,0,3,0,0,0,0,0,0,0,0 },
	spell_vortex, 		TAR_IGNORE, 		POS_STANDING,
	NULL, 			SLOT(312), 			30, 		12,
	"",	 			"!Vortex!",			""
},

{
 	"detect presence", {102,102,102,102,102,102,102,20,102,102,102,20,20,20,20,102},
                   {0,0,0,0,0,0,0,1,0,0,0,1,1,1,1,0 },
	spell_detect_presence, TAR_CHAR_SELF, 		POS_STANDING,
	NULL, 			SLOT(313), 			25, 		12,
	"", 				"!presence!",		""
},

{
 	"scout", {102,102,9,102,102,102,102,12,7,9,9,102,102,102,102,102},
                   {0,0,2,0,0,0,0,2,2,2,2,0,0,0,0,0 },
	spell_null, 		TAR_IGNORE, 		POS_STANDING,
	&gsn_scout, 		SLOT(314), 			0, 		12,
	"", 				"!Scout!",			""
},

{
	"sticks to snakes", {102,102,102,102,102,102,102,23,102,102,102,102,102,102,102,102},
                   {0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0 },
	spell_sticks_to_snakes, TAR_IGNORE, 		POS_FIGHTING,
	NULL, 			SLOT(315), 			15, 		12,
	"snake bite", 		"You don't feel so ill.",	""
},

{
	"sleepless curse", {102,102,102,102,102,102,102,102,102,102,102,102,76,102,102,102},
                   {0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0 },
	spell_sleepless_curse, 	TAR_CHAR_SEMIOFFENSIVE, POS_STANDING,
	&gsn_sleepless_curse, 	SLOT(316), 			50, 		12,
	"", 			"You feel as though you can finally rest.",		""

},

{
 	"second backstab", {102,102,38,102,102,102,102,102,102,48,102,102,102,102,102,102},
                   {0,0,4,0,0,0,0,0,0,4,0,0,0,0,0,0 },
	spell_null, 		TAR_IGNORE, 		POS_STANDING,
	&gsn_double_bs, 		SLOT(317), 			0, 		16,
	"second backstab", 	"!2nd Backstab!",			""
},

{
 	"quiet movement", {25,25,25,25,25,25,25,4,17,25,25,25,25,25,25,25},
                   {0,0,0,0,0,0,0,3,3,0,0,0,0,0,0,0 },
	spell_null, 		TAR_IGNORE, 		POS_STANDING,
	&gsn_quiet_movement, 	SLOT(318), 			0, 		12,
	"", 		"You can no longer hide among the bushes.",	""
},

{
	"sequester", {102,102,102,102,102,102,102,102,102,102,102,102,102,81,102,102},
                   {0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0 },
	spell_sequester, 		TAR_OBJ_CHAR_DEF, 	POS_STANDING,
	NULL, 			SLOT(319), 			35, 		18,
	"", 	"You feel pins and needles as the feeling comes back into your limbs.",		""
},

{
 	"strip", {102,102,71,102,102,102,102,102,102,102,102,102,102,102,102,102},
                   {0,0,6,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	spell_null, 		TAR_IGNORE, 		POS_STANDING,
	&gsn_strip, 		SLOT(320), 			0, 		48,
	"", 				"!Strip!",			""
},

{
 	"preserve dead", {102,102,102,102,102,102,102,102,102,102,102,68,102,102,102,102},
                   {0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0 },
	spell_preserve_dead, 	TAR_IGNORE, 		POS_STANDING,
	NULL, 			SLOT(321), 			40, 		16,
	"", 				"!Preserve Dead!",	""
},

{
	"ray of fatigue", { 102,102,102,102,102,102,102,102,102,102,102,76,102,102,102,102 },
                  { 0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0 },
	spell_ray_of_fatigue,   TAR_CHAR_SEMIOFFENSIVE,	POS_FIGHTING,
	&gsn_ray_of_fatigue,    SLOT(322),     		40,     16,
	"",          "Your body feels more rejuvenated and less fatigued.",	""
},

{
	"mooncloak", { 102,102,102,102,102,102,102,102,102,102,102,102,102,102,45,102 },
                     { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0 },
	spell_mooncloak,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,				SLOT(323),			50,		20,
	"",	"The moon's energy leaves you, making you feeling more vulnerable.", ""
},

{
 	"squire call", { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 },
                     { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	spell_null, 		TAR_IGNORE, 		POS_STANDING,
	&gsn_squire_call, 	SLOT(324), 			0, 		12,
	"", 		"You are able to summon your squire again.",	""
},

{
 	"surge", { 10,102,102,102,102,102,102,11,102,102,10,4,4,4,4,102 },
                     { 2,0,0,0,0,0,0,2,0,0,2,1,1,1,1,0 },
	spell_null, 		TAR_CHAR_OFFENSIVE, 	POS_FIGHTING,
	&gsn_surge, 		SLOT(325), 			0, 		12,
	"mana surge", 		"!Surge!",			""
},

{
 	"endure", { 11,102,102,102,102,102,102,12,102,102,11,5,5,5,5,102 },
                     { 2,0,0,0,0,0,0,2,0,0,2,1,1,1,1,0 },
	spell_null, 		TAR_IGNORE, 		POS_STANDING,
	&gsn_endure, 		SLOT(326), 			0, 		20,
	"endure", 		"You are less prepared for magical encounters.",	""
},

{
 	"serenade", { 102,102,102,102,25,102,102,102,102,102,102,102,102,102,102,102 },
                     { 0,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0 },
	spell_null, 		TAR_IGNORE, 		POS_RESTING,
	&gsn_serenade, 		SLOT(327), 			0, 		12,
	"", 			"The love spell wears off you!",	""
},

{
 	"taunt", { 102,65,21,102,35,102,102,102,102,30,52,102,37,102,102,102 },
                     { 0,4,3,0,2,0,0,0,0,2,3,0,4,0,0,0 },
	spell_null, 		TAR_IGNORE, 		POS_RESTING,
	&gsn_taunt, 		SLOT(328), 			0, 		12,
	"", 			"You calm down.",				""
},

{
 	"bravado", { 102,102,102,102,8,102,102,102,102,102,102,102,102,102,102,102 },
                     { 0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0 },
	spell_null, 		TAR_IGNORE, 		POS_STANDING,
 	&gsn_bravado, 		SLOT(329), 			0, 		12,
 	"", 			"Your courage fades a little.",	""
},

{
 	"lull", { 102,102,102,102,40,102,102,102,102,102,102,102,102,102,102,102 },
                     { 0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0 },
	spell_null, 		TAR_IGNORE, 		POS_RESTING,
	&gsn_lull, 			SLOT(330), 			0, 		12,
	"", 			"You are less tired!",			""
},

{
 	"pacify", { 102,102,102,102,50,102,102,102,102,102,102,102,102,102,102,102 },
                     { 0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0 },
	spell_null, 		TAR_IGNORE, 		POS_RESTING,
	&gsn_pacify, 		SLOT(331), 			0, 		12,
	"", 			"!Pacify!",					""
},

{
 	"boil blood", { 102,102,102,102,102,102,102,102,102,102,102,14,102,102,102,102 },
                     { 0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0 },
	spell_blood_boil, 	TAR_CHAR_OFFENSIVE, 	POS_FIGHTING,
	NULL, 			SLOT(332), 			15, 		12, 
	"blood magic", 		"!boiling blood!",		""
},

{
 	"god fire", { 102,55,102,102,102,102,102,102,102,102,102,102,102,102,102,102 },
                     { 0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	spell_god_fire, 		TAR_CHAR_OFFENSIVE, 	POS_FIGHTING,
	NULL,				 SLOT(333), 		25, 		12,
	"burns", 		"The fires of the gods flare out.",	""
}, 

{
 	"wrath", { 102,64,102,102,102,102,102,102,102,102,102,102,102,102,102,102 },
                     { 0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	spell_wrath, 		TAR_CHAR_OFFENSIVE, 	POS_FIGHTING,
	NULL, 			SLOT(	334), 		20, 		12,
	"heavenly wrath", 	"!Wrath!",				""
},

{
 	"great wasting", { 102,102,102,102,102,102,102,102,102,102,102,24,102,102,102,102 },
                     { 0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0 },
	spell_great_wasting, 	TAR_CHAR_SEMIOFFENSIVE, POS_STANDING,
	&gsn_wasting, 		SLOT(336), 			20, 		20,
	"lays waste", 		"Your vigor is restored.",	""
},

{
 	"fumble", { 102,102,102,102,102,102,102,102,102,102,102,102,49,102,102,102 },
                     { 0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0 },
	spell_fumble, 		TAR_CHAR_OFFENSIVE, 	POS_FIGHTING,
	NULL, 			SLOT(337), 			16, 		20,
	"", 				"!fumble!",				""
}, 

{
 	"regeneration", { 102,70,102,102,102,102,102,102,102,102,102,102,102,102,102,102 },
                     { 0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	spell_regeneration, 	TAR_CHAR_DEFENSIVE, 	POS_STANDING,
	NULL, 			SLOT(338), 			75, 		16,
	"", 		"You feel your healing rate return to normal.",	""
}, 

{
 	"sparkshot", { 102,102,102,102,102,102,102,1,102,102,102,102,102,102,102,102 },
                     { 0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0 },
	spell_sparkshot, 		TAR_CHAR_OFFENSIVE, 	POS_FIGHTING,
	NULL, 			SLOT(339), 			15, 		6,
	"sparkshot", 		"!Sparkshot!",		""
}, 

{
 	"quench", { 102,27,102,102,102,102,102,102,102,102,102,102,102,102,102,102 },
                     { 0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	spell_quench, 		TAR_CHAR_DEFENSIVE, 	POS_STANDING,
	NULL, 			SLOT(340), 			5, 		12,
	"", 				"!Quench!",				""
}, 

{
 	"rabbit summon", { 102,102,102,102,102,102,102,102,102,102,102,102,102,102,1,102 },
                     { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0 },
	spell_rabbit_summon, 	TAR_IGNORE, 		POS_STANDING,
	NULL, 			SLOT(341), 			15, 		12,
	"", 		"Your ability to summon a rabbit returns.",	""
}, 

{
 	"banish", { 35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35 },
                     { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	spell_banish, 		TAR_CHAR_OFFENSIVE, 	POS_FIGHTING,
	NULL, 			SLOT(342), 			35, 		12,
	"", 				"!Banish!",			""
}, 

{
 	"salvation", { 25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25 },
                     { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	spell_salvation, 		TAR_CHAR_DEFENSIVE, 	POS_STANDING,
	NULL, 			SLOT(343), 			150, 		20,
	"", 				"!Salvation!",		""
},

{
 	"scrye", { 102,102,102,102,102,102,102,102,102,102,102,102,102,30,102,102 },
                     { 0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0 },
	spell_scrye, 		TAR_IGNORE, 		POS_STANDING,
	NULL, 			SLOT(344), 			35, 		18,
	"", 				"!scrye!",			""
},


{
 	"flash", { 102,102,102,102,102,102,102,102,102,102,31,102,102,42,102,102 },
                     { 0,0,0,0,0,0,0,0,0,0,2,0,0,2,0,0 },
	spell_flash, 		TAR_CHAR_SEMIOFFENSIVE, POS_STANDING,
	NULL, 			SLOT(346), 			20, 		12,
	"", 		"White stars stop dancing before your eyes.",	""
},

{
 	"empowerment", {102,102,102,102,102,102,102,102,102,102,55,102,82,102,102,102},
                   {0,0,0,0,0,0,0,0,0,0,2,0,1,0,0,0 },
	spell_empowerment, 	TAR_CHAR_DEFENSIVE, 	POS_STANDING,
	NULL, 			SLOT(347), 			20, 		12,
	"", 			"You feel less robust.",		""
},

{
 	"sunray", { 102,102,102,102,102,102,102,57,102,102,102,102,102,102,102,102 },
                     { 0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0 },
	spell_sunray, 		TAR_CHAR_OFFENSIVE, 	POS_FIGHTING,
	NULL, 			SLOT(348), 			35, 		12,
	"sunray", 	"The bright spots blocking your vision clear!",	""
},

{
 	"eavesdrop", { 102,102,22,102,102,102,102,102,102,33,28,102,102,102,102,102 },
                     { 0,0,1,0,0,0,0,0,0,1,1,0,0,0,0 },
	spell_null, 		TAR_IGNORE, 		POS_RESTING,
	&gsn_eavesdrop, 		SLOT(349), 			15, 		12,
	"eavesdrop", 		"!Eavesdrop!",				""
},

{
 	"garble", { 102,102,102,102,102,102,102,102,102,102,102,102,38,102,102,102 },
                     { 0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0 },
	spell_garble, 		TAR_CHAR_SEMIOFFENSIVE, POS_STANDING, 
	NULL, 			SLOT(350), 			25, 		24,
	"", 			"Your tongue returns to normal.",		""
},

{
	"coldlight", { 102,102,102,102,102,102,102,102,102,102,102,3,102,102,102,102 },
                           { 0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0 },
	spell_coldlight,		TAR_IGNORE,			POS_STANDING,
	NULL,				SLOT(351),	 		7,		12,
	"",			"!Continual Light!",	""
},

{
	"resist cold",  {102,102,102,102,102,102,102,102,102,102,102,28,102,102,102,102},
					{ 0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0},
	spell_resist_cold,  	TAR_CHAR_SELF,  		POS_STANDING,
	NULL,				SLOT(352),			45,		12,
	"",	"Your body once again becomes less resistant to cold based attacks.",	""
},

{
	"deadeye",	{  102,102,102,102,102,102,102,102,102,102,102,3,102,102,102,102 },     
                        { 0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0 },
	spell_deadeye,		TAR_CHAR_SELF,		POS_STANDING,
	NULL,				SLOT(353),	 		15,		12,
	"",		"You no longer see the invisible nor see in the dark.",     ""
},

{
	"renew bones", { 102,102,102,102,102,102,102,102,102,102,102,29,102,102,102,102 },    
                { 0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0 },
	spell_renew_bones,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,				SLOT(354),			50,		12,
	"",				"!Renew Bones!",		""
},

{
 	"breath of the dead", { 102,102,102,102,102,102,102,102,102,102,102,54,102,102,102,102 },
 			{ 0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0},
	spell_breath_of_the_dead,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			 	SLOT(355),	 		45,	 	8,
	"breath of the dead",	"!Breath of the Dead!",		""
},

{
	"intensify death",  {50,50,50,50,50,50,50,50,50,50,50,28,50,50,50,50},
					{ 0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0},
	spell_intensify_death,  TAR_CHAR_SELF,  		POS_STANDING,
	NULL,				SLOT(356),			60,		12,
	"",	"Your body becomes vulnerable to the living once again.",	""
},

{
 	"shallow breath", { 102,102,102,102,102,102,102,102,102,102,102,102,4,102,102,102 },
 			{ 0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0},
	spell_shallow_breath,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			 	SLOT(357),	 		15,	 	8,
	"shallow breath spell",		 "!Shallow Breath!",		""
},

{
	"chaos flux", { 102,102,102,102,102,102,102,102,102,102,102,102,22,102,102,102},
                          { 0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0},
      spell_chaos_flux,   	TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
      NULL,                   SLOT(358),      		35,      12,
      "chaotic flux",  		"!Chaotic Flux!",			""
},

{
	"gravity flux", { 102,102,102,102,102,102,102,102,102,102,102,102,37,102,102,102 },  
                      { 0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0 },
	spell_gravity_flux,	TAR_IGNORE,		POS_FIGHTING,
	NULL,				SLOT(359),			30,		12,
	"gravitational flux",		"!Gravity Flux!",			""
},

{
	"torment",	{ 102,102,102,102,102,102,102,102,102,102,102,102,63,102,102,102 },
                { 0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0 },
	spell_torment,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,				SLOT(360),			35,		12,
	"torments",			"!Torment!",		""
},

{
 	"cure deafness", { 102,12,102,102,102,102,102,102,102,102,102,102,102,102,102,102 },
                { 0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	spell_cure_deafness, 	TAR_CHAR_DEFENSIVE, 	POS_STANDING,
	NULL, 			SLOT(361), 			15, 		12,
	"", 				"!Cure Deafness!",	""
},

{
 	"deafness", { 102,102,102,102,102,102,102,102,102,102,12,38,102,33,102,102 },
                { 0,0,0,0,0,0,0,0,0,0,1,1,0,1,0,0 },
 	spell_deafness, 		TAR_CHAR_SEMIOFFENSIVE, POS_STANDING,
	NULL, 			SLOT(362), 			25, 		16,
	"", 		"The sounds of the world return to your ears.",	""
},

{
 	"thunderclap", { 102,102,102,102,102,102,102,102,102,102,102,102,102,102,36,102 },
                { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0 },
	spell_thunder_clap, 	TAR_CHAR_SEMIOFFENSIVE, POS_STANDING,
	NULL, 			SLOT(363), 			20, 		12,
	"", 		"The ringing in your ears subsides.",		""
}, 

{
 	"riposte", { 50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50 },
                { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	spell_null, 		TAR_IGNORE, 		POS_FIGHTING,
	&gsn_riposte, 		SLOT(364),			0,	 	0,
	"thrusting riposte", 	"!Riposte!",		""
},

{
 	"redirect", { 102,102,102,102,22,102,102,102,102,102,102,102,102,102,102,102 },
                { 0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0 },
	spell_null, 		TAR_IGNORE,	 		POS_FIGHTING,
	&gsn_redirect, 		SLOT(365), 			0, 		0,
	"", 				"!Redirect!",		""
},

{
 	"evade", { 31,102,19,102,102,102,102,102,102,102,102,102,102,102,102,102 },
                { 1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	spell_null, 		TAR_IGNORE, 		POS_FIGHTING,
	&gsn_evade, 		SLOT(366), 			0, 		0,
	"", 				"!Evade!",			""
},

{
 	"pivot block", { 30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30 },
                { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	spell_null, 		TAR_IGNORE, 		POS_FIGHTING,
	&gsn_pivot, 		SLOT(367), 			0, 		0,
	"pivot block", 		"!Pivot!",			""
},

{
 	"sidestep", { 102,102,102,102,102,102,23,102,102,102,102,102,102,102,102,102 },
                { 0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0 },
	spell_null, 		TAR_IGNORE, 		POS_FIGHTING,
	&gsn_sidestep, 		SLOT(368), 			0, 		0,
	"", 				"!Sidestep!",		""
},

{
 	"acrobatics", { 102,102,102,102,33,102,102,102,102,102,102,102,102,102,102,102 },
                { 0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0 },
	spell_null, 		TAR_IGNORE, 		POS_FIGHTING,
	&gsn_acrobatics, 		SLOT(369), 			0, 		0,
	"", 				"!Acrobatics!",		""
},

{
 	"trance", { 102,102,102,102,102,102,102,23,102,102,102,14,14,14,14,102 },
                { 0,0,0,0,0,0,0,1,0,0,0,1,1,1,1,0 },
	spell_null, 		TAR_IGNORE, 		POS_SLEEPING,
	&gsn_trance, 		SLOT(370), 			0, 		0,
	"", 				"!Trance!",			""
},

{
 	"waterlungs", { 102,102,102,102,102,102,102,5,102,102,102,102,33,102,102,102 },
                { 0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0 },
	spell_waterlungs, 	TAR_CHAR_DEFENSIVE, 	POS_STANDING,
	NULL, 			SLOT(371), 			10, 		18,
	"", 			"Your lungs return to normal.",		""
}, 

{
 	"mend", { 102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,10 },
                { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3 },
	spell_null, 		TAR_IGNORE, 		POS_STANDING,
	&gsn_mend,	 		SLOT(372), 			0, 		20,
	"", 				"!Mend!",		""
},

{
 	"repair shield", { 102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,2 },
                { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2 },
	spell_null, 		TAR_IGNORE, 		POS_STANDING,
	&gsn_repair_shield,	SLOT(373), 			0, 		20,
	"", 				"!Repair Shield!",		""
},

{
 	"leather repair", { 102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,1 },
                { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 },
	spell_null, 		TAR_IGNORE, 		POS_STANDING,
	&gsn_leather_repair,	SLOT(374), 			0, 		20,
	"", 				"!Leather Repair!",		""
},

{
 	"plate repair", { 102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,5 },
                { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2 },
	spell_null, 		TAR_IGNORE, 		POS_STANDING,
	&gsn_plate_repair,	SLOT(375), 			0, 		20,
	"", 				"!Plate Repair!",		""
},

{
 	"chain repair", { 102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,4 },
                { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2 },
	spell_null, 		TAR_IGNORE, 		POS_STANDING,
	&gsn_chain_repair,	SLOT(376), 			0, 		20,
	"", 				"!Chain Repair!",		""
},

{
 	"weapon repair", { 102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,6 },
                { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4 },
	spell_null, 		TAR_IGNORE, 		POS_STANDING,
	&gsn_weapon_repair,	SLOT(377), 			0, 		20,
	"", 				"!Weapon Repair!",		""
},

{
 	"deathblade", { 102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,12 },
                { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3 },
	spell_null, 		TAR_IGNORE, 		POS_STANDING,
	&gsn_deathblade,		SLOT(378), 			0, 		20,
	"", 				"!Deathblade!",		"The sharp Deathblade on $p becomes dull."
},

{
 	"flameblade", { 102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,11 },
                { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4 },
	spell_null, 		TAR_IGNORE, 		POS_STANDING,
	&gsn_flameblade,		SLOT(379), 			0, 		20,
	"", 				"!Flameblade!",		"The Flameblade on $p goes out."
},

{
 	"runeblade", { 102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,9 },
                { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3 },
	spell_null, 		TAR_IGNORE, 		POS_STANDING,
	&gsn_runeblade,		SLOT(380), 			0, 		20,
	"", 				"!Runeblade!",		"The magical Runeblade on $p fades."
},

{
 	"fast working", { 102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,16 },
                { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5 },
	spell_null, 		TAR_IGNORE, 		POS_STANDING,
	&gsn_fast_working,	SLOT(381), 			0, 		0,
	"", 				"!Fast Working!",		""
},

{
 	"stainless", { 102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,18 },
                { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4 },
	spell_null, 		TAR_IGNORE, 		POS_STANDING,
	&gsn_stainless,		SLOT(382), 			0, 		30,
	"", 		"!Stainless!",		 "The protective film on $p disintegrates."
},

{
 	"eversharp", { 102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,20 },
                { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4 },
	spell_null, 		TAR_IGNORE, 		POS_STANDING,
	&gsn_eversharp,		SLOT(383), 			0, 		20,
	"", 				"!Eversharp!",		""
},

{
 	"purifying fire", { 102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,25 },
                { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2 },
	spell_null, 		TAR_IGNORE, 		POS_STANDING,
	&gsn_purifying_fire,	SLOT(384), 			0, 		25,
	"", 				"!Purifying Fire!",		""
},

{
 	"fletching", { 102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,23 },
                { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2 },
	spell_null, 		TAR_IGNORE, 		POS_STANDING,
	&gsn_fletching,		SLOT(385), 			0, 		20,
	"", 				"!Fletching!",		""
},

{
 	"boobytrap", { 102,102,17,102,102,102,102,102,102,102,102,102,102,102,102,102 },
                { 0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	spell_null, 		TAR_IGNORE, 		POS_STANDING,
	&gsn_boobytrap, 		SLOT(386), 			0, 		12,
	"", 				"!Boobytrap!",		""
},

{
 	"disarm traps", { 102,102,24,102,102,102,102,102,36,102,32,102,102,102,102,102 },
                { 0,0,1,0,0,0,0,0,1,0,1,0,0,0,0,0 },
	spell_null, 		TAR_IGNORE, 		POS_STANDING,
	&gsn_disarm_traps, 	SLOT(387), 			0, 		12,
	"clumsiness", 				"!Disarm Traps!",		""
},

{
 	"set traps", { 102,102,102,102,102,102,102,102,31,102,102,102,102,102,102,102 },
                { 0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0 },
	spell_null, 		TAR_IGNORE, 		POS_STANDING,
	&gsn_set_traps, 		SLOT(388), 			0, 		12,
	"", 				"!Set Traps!",		""
},

{
 	"alarm", { 102,102,102,102,102,102,102,102,102,102,40,102,102,102,22,102 },
                { 0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0 },
	spell_alarm, 		TAR_IGNORE, 		POS_STANDING,
	NULL, 			SLOT(389), 			5, 		12,
	"", 		"You may now call upon another alarm.", 	""
},

{
 	"fireseed", { 102,102,102,102,102,102,102,38,102,102,102,102,102,102,102,102 },
                { 0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0 },
	spell_fire_seed, 		TAR_IGNORE, 		POS_FIGHTING,
	NULL, 			SLOT(390), 			25, 		16,
	"fireseed", 		"!Fireseed!",		""
},

{
 	"gift", { 21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21 },
                { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0 },
	spell_gift, 		TAR_OBJ_INV,		POS_STANDING,
	NULL, 			SLOT(391), 			50, 		12,
	"", 		"The ability to reach others returns.",		""
},

{
 	"rally", { 102,102,102,102,17,102,102,102,102,102,102,102,102,102,102,102 },
                { 0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0 },
	spell_null, 		TAR_IGNORE, 		POS_RESTING,
	&gsn_rally, 		SLOT(392),	 		0, 		24,
	"", 			"You can rally again.",		""
},

{
 	"entwine", { 102,102,102,102,102,102,60,102,102,102,102,102,102,102,102,102 },
                { 0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0 },
	spell_null, 		TAR_IGNORE, 		POS_FIGHTING,
	&gsn_entwine, 		SLOT(393), 			0, 		20,
	"entwine", 			"!entwine!",		""
},

{
 	"cloak of evil", { 102,102,102,102,102,102,102,102,102,102,102,102,102,17,102,102 },
                { 0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0 },
	spell_evil_cloak, 	TAR_CHAR_SELF, 		POS_STANDING,
	NULL, 			SLOT(394), 			10, 		12,
	"", 		"The black mist surrounding you dissipates!",		""
},

{
 	"hypnotic pattern", { 102,102,102,102,102,102,102,102,102,102,102,102,102,29,102,102 },
                { 0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0 },
	spell_dazzle, 		TAR_CHAR_SEMIOFFENSIVE, POS_STANDING,
	NULL, 			SLOT(395), 			40, 		16,
	"", 	"The patterns dissipate, and your mind is your own again.",	""
},

{
 	"smith", { 102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,35 },
                { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6 },
	spell_null, 		TAR_IGNORE, 		POS_STANDING,
	&gsn_smith,			SLOT(396), 			0, 		0,
	"", 				"!Smith!",			""
},

{
	"create furnace", { 102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,32 },
                     { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 },
	spell_create_furnace,	TAR_IGNORE,			POS_STANDING,
	NULL,				SLOT(397),			30,		20,
	"",				"!Create Furnace!",	""
},

{
	"create barrel", { 102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,28 },
                     { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 },
	spell_create_barrel,	TAR_IGNORE,			POS_STANDING,
	NULL,				SLOT(398),			30,		20,
	"",				"!Create Barrel!",	""
},

{
	"create grinder", { 102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,32 },
                     { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 },
	spell_create_grinder,	TAR_IGNORE,			POS_STANDING,
	NULL,				SLOT(399),			30,		20,
	"",				"!Create Grinder!",	""
},

{
	"create anvil", { 102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,5 },
                     { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 },
	spell_create_anvil,	TAR_IGNORE,			POS_STANDING,
	NULL,				SLOT(400),			30,		20,
	"",				"!Create Anvil!",		""
},

{
 	"immolation", { 102,102,102,102,102,102,102,61,102,102,102,102,102,102,102,102 },
 			{ 0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0},
	spell_immolation,		TAR_CHAR_SELF,		POS_STANDING,
	NULL,			 	SLOT(401),	 		30,	 	12,
	"",		 "Your immolating flames return to the fire realm.",		""
},

{
 	"pine needles", { 102,102,102,102,102,102,102,42,102,102,102,102,102,102,102,102 },
 			{ 0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0},
	spell_pine_needles,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,			 	SLOT(402),	 		20,	 	12,
	"",		 "The pine needles surrounding you drop to the ground.",		""
},

{
	"masterwork", { 102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,45 },
                     { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7},
	spell_null,			TAR_IGNORE,			POS_STANDING,
	&gsn_masterwork,		SLOT(403),			0,		55,
	"",				"!Masterwork!"		""
},

{
	"create flame", { 102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,23 },
                     { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 },
	spell_create_flame,	TAR_IGNORE,			POS_STANDING,
	NULL,				SLOT(404),			30,		20,
	"",				"!Create Flame!",		""
},

{
	"thors hammer", { 102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,3 },
                     { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3 },
	spell_thors_hammer,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,				SLOT(405),			20,		30,
	"gigantic hammer",   	"!Thor's Hammer!",	""
},

{
	"dismantle", { 102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,4 },
                     { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4 },
	spell_null,			TAR_IGNORE,			POS_STANDING,
	&gsn_dismantle,		SLOT(406),			0,		35,
	"",		 	  	"!Dismantle!",		""
},

{
	"staff", { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,102 },
	         { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0},
      spell_null,             TAR_IGNORE,             POS_FIGHTING,
      &gsn_staff,            	SLOT( 407),       	0,      	0,
      "",                     "!Staff!",		""
},

{
	"unholyrite", { 60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60 },
	         { 0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0 },
      spell_null,             TAR_IGNORE,			POS_STANDING,
      &gsn_unholyrite,        SLOT( 408),      	 	0,      	0,
      "",                     "!Unholyrite!",		""
},

{
 	"phantom armor", { 102,102,102,102,102,102,102,102,102,102,102,102,102,93,102,102 },
 			{ 0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0},
	spell_phantom_armor,	TAR_CHAR_SELF,	 	POS_STANDING,
	NULL,			 	SLOT(409),	 		120,	 	18,
	"",		 "Your illusionary plate mail disappears.",		""
},

{
  	"spook", {  102,102,102,102,102,102,102,102,102,102,102,102,102,70,102,102  },     
                 { 0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0 },
  	spell_spook,		TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,				SLOT(410),	 		25,		24,
	"",			"The illusion fades and your nerves return to normal.",	""
},

{
	"eyerub", { 16,17,18,15,18,20,17,18,19,19,20,22,23,24,21,20 },
				{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
	spell_null,			TAR_IGNORE,			POS_FIGHTING,
      &gsn_eyerub,		SLOT(411),			0,		24,
      "",			"!Eyerub!",				""
},

{
 	"finger break", {102,102,102,70,102,70,65,102,102,102,102,102,102,102,102,102},
                   {0,0,0,1,0,1,1,0,0,0,0,0,0,0,0,0 },
	spell_null, 		TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	&gsn_fingerbreak, 	SLOT(412), 			0, 		24,
	"finger smash", 	"Your fingers heal up enough to use them again!",  ""
},

{
 	"execute", {102,102,102,102,102,102,91,102,102,102,102,102,102,102,102,102},
                   {0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0 },
	spell_null, 		TAR_IGNORE, 		POS_STANDING,
	&gsn_execute, 		SLOT(413), 			0, 		24,
	"execution", 		"!Execute!",		""
},

{
      "earpunch", { 102,102,102,80,102,102,70,102,102,102,102,102,102,102,102,102  },
                {0,0,0,1,0,0,1,0,0,0,0,0,0,0,0,0 },
	spell_null,             TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
      &gsn_earpunch,          SLOT(414),        	0,     	24,
      "earpunch",  "You regain some of your mental senses back.",  ""
},

{
	 "brainjar", { 102,102,102,102,102,102,95,102,102,102,102,102,102,102,102,102 },
			{ 0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0},
	 spell_null,		TAR_CHAR_OFFENSIVE,	POS_STANDING,
	 &gsn_brainjar,		SLOT(415),			0,		12,
	 "brainjar",	"The pain in your head subsides.",		""
},

{
      "rupture organ", { 102,102,102,102,102,102,77,102,102,102,102,102,102,102,102,102 },
			{ 0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0},
	spell_null,             TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
      &gsn_rupture,           SLOT(416),        	0,     	30,
      "organ rupturing punch",   "Your internal bleeding resides",	""
},

{
      "tonguecut", { 102,102,102,85,102,102,88,102,102,80,102,102,102,102,102,102 },
			{ 0,0,0,1,0,0,1,0,0,1,0,0,0,0,0,0},
	spell_null,             TAR_IGNORE,     		POS_STANDING,
      &gsn_tonguecut,           SLOT(417),        	0,     	24,
      "attempted tonguecut",   "The Gods have bestowed you with a new tongue.",	""
},

{
	"cultural artistry", { 102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,50 },
				{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5 },
	spell_null,			TAR_IGNORE,			POS_STANDING,
	&gsn_cultural_artistry,	SLOT(418),			0,		34,
	"",				"!Cultural Artistry!",	""
},

{
	"wrath of nature", {102,102,102,102,102,102,102,70,102,102,102,102,102,102,102,102},
                   {0,0,0,0,0,0,0,3,0,0,0,0,0,0,0,0 },
	spell_wrath_of_nature, 	TAR_CHAR_OFFENSIVE, 	POS_FIGHTING,
	NULL, 			SLOT(419), 			30, 		12,
	"wrath of nature",	"!Wrath of Nature!",	""
},

{
 	"stalk", {102,102,102,102,102,102,102,102,102,85,102,102,102,102,102,102},
                   {0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0 },
	spell_null, 		TAR_IGNORE, 		POS_STANDING,
	&gsn_stalk, 		SLOT(420), 			0, 		24,
	"", 				"!Stalk!",			""
},

{
	"smother", { 102,102,102,67,45,62,63,102,102,102,102,102,102,102,102,102  },
                  { 0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0 },
	spell_null,			TAR_IGNORE,			POS_STANDING,
	&gsn_smother,		SLOT(421),			0,		24,
	"",				"!Smother!",		""
},

{
	"firetongue", { 102,102,102,102,102,102,102,102,102,102,75,102,102,102,102,102  },
                    { 0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0 },
	spell_firetongue,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,				SLOT(422),			40,		36,
	"tongue of fire",		"!Firetongue!",		""
},

{
	"instruct", { 50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50  },
                  { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 },
	spell_null,			TAR_IGNORE,			POS_STANDING,
	&gsn_instruct,		SLOT(423),			0,		36,
	"",  "You may instruct others the ways of your skills again.", ""
},

{
	"eagle eye", { 60,60,60,60,60,60,60,60,58,60,60,60,60,60,60,60  },
                { 0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0 },
	spell_eagle_eye,		TAR_IGNORE,			POS_STANDING,
	NULL,				SLOT(424),			80,		24,
	"",		"You regained the ability to call on another Eagle.",	""
},

{
	"call of the wild", { 102,102,102,102,102,102,102,68,65,102,102,102,102,102,102,102  },  
                      { 0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0 },
	spell_call_of_the_wild,	TAR_IGNORE,			POS_FIGHTING,
	NULL,				SLOT(425),			90,		12,
	"",		"Your hearing returns to normal",		""
},

{
	"tame", { 102,102,102,102,102,102,102,80,80,102,102,102,102,102,102,102  },
                { 0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0 },
	spell_null,		TAR_CHAR_SEMIOFFENSIVE,		POS_STANDING,
	&gsn_tame,			SLOT(426),			0,		24,
	"",			"You regain your free will!",		""
},

{
 	"entangle", { 102,102,102,102,102,102,102,75,102,102,102,102,102,102,102,102  },
                { 0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0 },
	spell_entangle, 	TAR_CHAR_SEMIOFFENSIVE, 	POS_STANDING,
	NULL, 			SLOT(427), 			35, 		12,
	"", 			"The vines binding you loosen and fall away.",	""
},

{
      "heroic demise", { 91,102,102,78,102,80,102,102,102,102,102,102,102,102,102,102  },
                 { 1,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0 },
      spell_null,             TAR_IGNORE,             POS_FIGHTING,
      &gsn_heroic,            SLOT( 428),        	0,     	0,
      "",                     "!Heroic Demise!",		""
},

{
      "evade spell", { 102,102,92,102,102,102,102,102,102,102,102,102,102,102,102,102  },
                 { 0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0 },
      spell_null,             TAR_IGNORE,             	POS_FIGHTING,
      &gsn_evade_spell,		SLOT( 429),        	0,     	0,
      "",                     "!Evade Spell!",		""
},

{
	"endure disease", { 65,73,102,102,102,102,102,102,102,102,47,70,102,102,102,102  },    
                  { 1,1,0,0,0,0,0,0,0,0,1,1,0,0,0,0 },
	spell_endure_disease,	TAR_CHAR_SELF,			POS_STANDING,
	NULL,				SLOT(430),			60,		18,
	"",				"You are no longer resistant to diseases.",	""
},

{
 	"joust", {45,102,102,37,102,34,102,102,102,102,102,102,102,102,102,102},
                   {1,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0 },
	spell_null, 		TAR_IGNORE, 			POS_STANDING,
	&gsn_joust, 		SLOT(431), 			0, 		28,
	"jousting weapon", 	"!Joust!",			""
},

{
 	"subdue", { 102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102 },    
                  { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	spell_null, 		TAR_CHAR_OFFENSIVE, 		POS_RESTING,
	&gsn_subdue, 		SLOT(432), 			0, 		12,
	"", 				"You are no longer subdued.",			""
},

{
	"polish", { 102,102,102,102,102,38,102,102,102,102,102,102,102,102,102,102 },
	           { 0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0 },
	spell_null,			TAR_IGNORE,	  			POS_RESTING,
	&gsn_polish,		SLOT(433),			0,		36,
	"",				"!Polish!",			""
},

{ 
	"shatter shield",{ 94,82,102,92,102,75,102,102,102,102,102,102,102,102,102,102 },
  			{1,1,0,1,0,1,0,0,0,0,0,0,0,0,0,0},
  	spell_null,			TAR_CHAR_OFFENSIVE, 		POS_STANDING,
  	&gsn_shield_shatter,	SLOT(434),      		25,    	18,
  	"",                     "!Shield Shatter!",	""
},

{
	"key origin", { 102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,7 },
                   { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	spell_null,			TAR_IGNORE,				POS_RESTING,
	&gsn_key_origin,		SLOT(435),			0,		12,
	"",				"!Key Origin!",		""
},

{
	"plant growth",{ 102,102,102,102,102,102,102,82,102,102,102,102,102,102,102,102},
			{ 0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0 },
	spell_plant_growth,	TAR_IGNORE,	 			POS_RESTING,
	NULL,	 			SLOT(436),	 		85,	 	12,
	"",			 	"",				""
},

{
 	"company might", {102,102,102,102,102,64,102,102,102,102,102,102,102,102,102,102},
                   {0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,0 },
	spell_null, 		TAR_IGNORE, 			POS_STANDING,
	&gsn_company_might, 	SLOT(437), 			50, 		12,
	"", 		"The inspiration of Might drains from your body and mind!",	""
},

{
 	"company valor", {102,102,102,102,102,68,102,102,102,102,102,102,102,102,102,102},
                   {0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,0 },
	spell_null, 		TAR_IGNORE, 			POS_STANDING,
	&gsn_company_valor, 	SLOT(438), 			75, 		12,
	"", 		"The inspiration of Valor drains from your body and mind!",	""
},

{
 	"company honor", {102,102,102,102,102,73,102,102,102,102,102,102,102,102,102,102},
                   {0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,0 },
	spell_null, 		TAR_IGNORE, 			POS_STANDING,
	&gsn_company_honor, 	SLOT(439), 			100, 		12,
	"", 		"The inspiration of Honor drains from your body and mind!",	""
},

{
 	"arrow deflection", {89,97,102,83,102,82,85,102,87,102,102,102,102,102,102,102},
                		{ 3,2,0,1,0,1,1,0,1,0,0,0,0,0,0,0 },
	spell_null, 		TAR_IGNORE, 			POS_FIGHTING,
	&gsn_arrow_deflection, 	SLOT(440),			0,	 	0,
	"", 				"!Arrow Deflection!",			""
},

{
      "enlarge", { 102,102,102,102,102,102,102,102,102,102,68,102,56,102,102,102},
				{ 0,0,0,0,0,0,0,0,0,0,2,0,1,0,0,0},
      spell_enlarge,          TAR_CHAR_DEFENSIVE,     	POS_RESTING,
      NULL,             	SLOT(441),       		65,      	18,
      "",            "You suddenly feel back to your normal size again",   ""
},

{
      "shrink", { 87,102,102,102,102,102,102,102,102,102,102,102,57,102,102,102},
				{ 2,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0},
      spell_shrink,          TAR_CHAR_DEFENSIVE,     		POS_RESTING,
      NULL,             	SLOT(442),       		65,      	18,
      "",            "You suddenly feel back to your normal size again",   ""
},

{
 	"preach", {102,63,102,102,102,102,102,102,102,102,102,102,102,102,102,102},
                   {0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	spell_null, 		TAR_IGNORE, 			POS_STANDING,
	&gsn_preach, 		SLOT(443), 			10, 		18,
	"", 		"The inspiration of Religous Preaching drains from your soul and mind!",	""
},

{
 	"singing", {102,102,102,102,70,102,102,102,102,102,102,102,102,102,102,102},
                   {0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0 },
	spell_null, 		TAR_IGNORE, 			POS_RESTING,
	&gsn_sing, 			SLOT(444), 			0, 		12,
	"", 				"!Sing!",			""
},

{
	"clear thoughts", { 100,95,102,102,102,102,102,102,102,102,102,102,102,102,102,102  },    
                  { 1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	spell_clear_thoughts,	TAR_CHAR_SELF,			POS_RESTING,
	NULL,				SLOT(445),			150,		18,
	"",		"You are no longer resistant to mind shattering spells.",	""
},

{
 	"divine favor", {102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102},
                   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	spell_divine_favor, 	TAR_CHAR_DEFENSIVE, 		POS_STANDING,
	NULL, 			SLOT(446), 				0, 		0,
	"", 		"You may call upon the Gods for divine favor once more.",	""
},

{
 	"lock", { 62,102,102,102,102,102,102,102,102,102,43,27,48,27,27,102},
                   {2,0,0,0,0,0,0,0,0,0,2,1,1,1,1,0 },
	spell_lock,	 		TAR_IGNORE,	 			POS_STANDING,
	NULL,			 	SLOT(447), 				10, 		12,
	"",				 "!Lock!",				""
},

{
 	"mark of origin", { 102,102,102,102,102,102,102,102,102,102,102,101,101,101,101,102 },
                { 0,0,0,0,0,0,0,0,0,0,0,2,2,2,2,0 },
	spell_mark_of_origin, 		TAR_IGNORE,				POS_RESTING,
	NULL, 				SLOT(448), 				250, 		12,
	"", 					"The memory of your mark of origin slowly fades away.",		""
},

{
	"conjure food", 	{ 102,102,102,102,102,102,102,102,102,102,102,102,102,102,57,102  },
                      		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0 },
	spell_conjure_food,		TAR_IGNORE,					POS_STANDING,
	NULL,					SLOT(449),	 				25,			12,
	"",					"!Create Food!",				""
},

{
 	"treeform", 	{ 102,102,102,102,102,102,102,55,102,102,102,102,102,102,102,102 },
                   		{0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0 },
	spell_null, 			TAR_IGNORE, 				POS_STANDING,
	&gsn_treeform, 			SLOT(450), 					0, 			18,
	"", 					"Your treeform fades and you revert back to your normal body.",		""
},

{
      "quicksand", 	{ 102,102,102,102,102,102,102,90,102,102,102,102,102,102,102,102  },
                  		{ 0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0 },
      spell_quicksand,        	TAR_CHAR_SEMIOFFENSIVE, 		POS_STANDING,
      NULL,                  		SLOT(451),      				60,     		24,
      "",                     	"The quicksand around you disappears.",			""
},

{
      "windtalk", 	{ 102,102,102,102,102,102,102,72,102,102,102,102,102,102,102,102  },
                  		{ 0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0 },
      spell_null,				TAR_CHAR_DEFENSIVE, 			POS_STANDING,
      &gsn_windtalk,			SLOT(452),      				0,     		12,
      "",                   		"!Windtalk!",				""
},

{
      "dancing strike", { 102,102,102,102,81,102,102,102,102,102,102,102,102,102,102,102  },
                  		{ 0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0 },
      spell_null,				TAR_CHAR_OFFENSIVE,     		POS_FIGHTING,
      &gsn_dancing_strike,		SLOT(453),      				0,    		24,
      "dancing strike",       	"!Dancing Strike!",			""
},

{
	"point-blank shot", { 102,102,102,102,102,102,102,102,55,102,102,102,102,102,102,102 },
                			{ 0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0 },
	spell_null,             	TAR_IGNORE,             		POS_FIGHTING,
	&gsn_point_blank,     		SLOT(454),       				0,      		0,
	"point-blank shot",		"!Bow!",					""
},

{
	"wilderness lore", { 102,102,102,102,102,102,102,90,102,102,102,102,102,102,102,102 },
           				{0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0 },
	spell_null,				TAR_IGNORE,					POS_RESTING,
	&gsn_wilderness,			SLOT(455),					0,			24,
	"",					"!Wilderness Lore!",			""
},

/*
{
	"blood bond",	{ 102,102,102,102,102,102,102,102,102,102,95,102,90,102,102,102 },     
                 			{ 0,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0 },
	spell_blood_bond,			TAR_CHAR_DEFENSIVE,			POS_STANDING,
	NULL,					SLOT(456),	 				25,			18,
	"",					"Your blood bond dissipates.",	""
},
*/

{
	"beastspite", 	{ 102,102,102,102,102,102,102,95,102,102,102,102,102,102,102,102 },     
                 			{ 0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0 },
	spell_null,				TAR_CHAR_SEMIOFFENSIVE,			POS_STANDING,
	&gsn_beastspite,			SLOT(457),	 				0,			18,
	"",					"Selective beasts will no longer spite you.",		""
},

/*
{
	"bestow curse", 	{ 102,75,102,102,102,102,102,102,102,102,102,102,102,102,102,102 },     
                 			{ 0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	spell_null,				TAR_CHAR_OFFENSIVE,			POS_STANDING,
	&gsn_bestow_curse,		SLOT(458),	 				0,			24,
	"",					"The bestowed curse has finally worn off.",		""
},
*/

{
	"black cairn", 	{ 102,48,102,102,102,102,102,102,102,102,102,102,102,102,102,102 },     
                 			{ 0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	spell_black_cairn,		TAR_IGNORE,					POS_STANDING,
	NULL,					SLOT(459),	 				50,			24,
	"",					"!Black Cairn!",		""
},

{
	"blessed warmth", { 102,68,102,102,102,102,102,102,102,102,102,102,102,102,102,102 },     
                 			{ 0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	spell_blessed_warmth,		TAR_CHAR_DEFENSIVE,			POS_STANDING,
	NULL,					SLOT(460),	 				45,			18,
	"",					"You feel less protected against the cold.",		""
},

{
	"blessed watchfulness", { 102,87,102,102,102,102,102,102,102,102,102,102,102,102,102,102 },     
                 			{ 0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	spell_blessed_watchfulness,	TAR_CHAR_DEFENSIVE,			POS_STANDING,
	NULL,					SLOT(461),	 				50,			18,
	"",					"Your exceptional awareness affect fades, returning to normal",		""
},

{
	"battletide", 	{  67,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102 },     
                 			{ 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	spell_battletide,			TAR_CHAR_SEMIOFFENSIVE,			POS_FIGHTING,
	NULL,					SLOT(462),	 				40,			18,
	"",					"You can once again move faster in battle.",		""
},

{
 	"relegate", {102,70,102,102,102,102,102,102,102,102,102,102,102,102,102,102},
                   		{0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	spell_null, 			TAR_IGNORE, 				POS_STANDING,
	&gsn_relegate, 			SLOT(463), 					100, 			24,
	"", 					"!Relegate!",	""
}

/*

{
	"battlefate", 	{  90,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102 },     
                 			{ 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	spell_battlefate,			TAR_CHAR_OFFENSIVE,			POS_FIGHTING,
	NULL,					SLOT(463),	 				75,			18,
	"",					"The fate of battle is once again in your hands.",	""
}
*/

};

const   struct  group_type      group_table     [MAX_GROUP]     =
{

{
  /*BATTLEMAGES*/
  {
  "sword", "dagger", "polearm", "sword", "wear chain", "whip", "mace", "spear",
  "wear leather", "staves", "scrolls", "wands", "bash", "shield block", "dodge",
  "meditation", "hand to hand", "third attack", "second attack", "brew", "scribe",
  "parry", "lore", "enhanced damage", "mount", "mounted combat", "trip", "disarm",
  "sweep attack", "endure", "surge", "battle tactics", "kick", "fast healing",
  "cantrip", "magic missile", "detect magic", "detect invis", "chill touch",
  "invisibility", "armor", "orb of containment", "burning hands", "infravision",
  "levitate", "detect good", "know alignment", "teleport", "detect poison",
  "shocking grasp", "dispel magic", "sheild", "fireball", "blade barrier", "acid blast",
  "giant strength", "flyingswords", "battlestaff", "iceblast", "enchant weapon", "knock",
  "word of recall", "evade", "staff", "eyerub", "instruct", "heroic demise",
  "endure disease", "joust", "shatter shield", "arrow deflection", "shrink",
  "clear thoughts", "lock", "battletide"
  }
},

{
   /*CLERICS*/
   {
   "flail", "wear chain", "wear leather", "staves", "wands", "scrolls", "mace",
   "meditation", "fast healing", "battle tactics", "dodge", "scribe", "brew",
   "traject", "mount", "pillify", "parry", "second attack", "demand", "enhanced damage",
   "hand to hand", "taunt", "deathgrip", "cause light", "cure light", "armor",
   "detect good", "detect evil", "create food", "refresh", "cure blindness", "bless",
   "cure serious", "cause serious", "detect poison", "detect invis", "know alignment",
   "earthquake", "protection evil", "orb of containment", "fireproof", "cure critical",
   "cure poison", "cure disease", "blindness", "dispel good", "calm", "dispel evil",
   "create spring", "curse", "cause critical", "flamestrike", "heal", "teleport",
   "harm", "dispel magic", "word of recall", "purify", "quench", "sate",
   "undetectable align", "cancellation", "ray of truth", "holy word", "repulsion",
   "mass healing", "energy drain", "enlightenment", "spiritual hammer", "invigorate",
   "demonfire", "sanctuary", "aid", "plague", "remove curse", "life transfer",
   "remove paralysis", "god fire", "sunbeam", "wrath", "regeneration", "firestorm",
   "silence", "aura cleanse", "cure deafness", "staff", "eyerub", "instruct",
   "endure disease", "shatter shield", "arrow deflection", "preach", "clear thoughts",
   "black cairn", "blessed warmth", "blessed watchfulness", "relegate"
   }
},

{
  /*THIEVES*/ 
  {
  "dagger", "sword", "whip", "wear leather", "haggle", "backstab", "dodge", "peek",
  "wear leather", "sneak", "dirt kicking", "trip", "hide", "shield block", "steal",
  "pick lock", "scout", "envenom", "tumble", "disarm", "second attack", "battle tactics",
  "kick", "hand to hand", "fast healing", "blackjack", "taunt", "mount",
  "enhanced damage", "pickpocket", "dual wield", "second backstab", "demand",
  "disguise", "circlestab", "lore", "third attack", "mountjack", "strip", "counter",
  "track", "detect hidden", "infravision", "detect poison", "eavesdrop", "evade",
  "boobytrap", "disarm traps", "staff", "eyerub", "instruct", "evade spell"
  }
},

{
  /*WARRIORS*/
  { 
  "frenzy", "axe", "sword", "dagger", "wear chain", "wear leather", "rescue", "bow",
  "enhanced damage", "whip", "spear", "parry", "dodge", "shield block", "mace",
  "flail", "polearm", "bash", "dirt kicking", "wear plate", "second attack",
  "headbutt", "hand to hand", "fast healing", "battle tactics", "guard", "impale",
  "kick", "disarm", "third attack", "mount", "trip", "trammel", "berserk", "nerve pinch",
  "caltrops", "dual wield", "mounted combat", "cleave", "bind wounds", "flank attack",
  "breakneck", "counter", "lunge", "critical strike", "whirlwind", "fourth attack",
  "break weapon", "sharpen", "staff", "eyerub", "finger break", "earpunch", "tonguecut",
  "instruct", "smother", "heroic demise", "joust", "shatter shield", "arrow deflection"
  }
},

{
  /*BARDS*/
  {
  "dagger", "mace", "sword", "flail", "polearm", "whip", "dodge", "trip", "haggle",
  "hand to hand", "wear chain", "wear leather", "kick", "parry", "lore", "play music",
  "battle tactics", "bravado", "sneak", "second attack", "tumble", "trammel", "hide",
  "disarm", "shield block", "fast healing", "mount", "enhanced damage", "serenade",
  "dual wield", "taunt", "lull", "pacify", "counter", "deathgrip", "chords of dissonance",
  "summon", "chant of battle", "dispel magic", "redirect", "acrobatics", "rally",
  "staff", "scrolls", "eyerub", "instruct", "smother", "singing", "dancing strike"
  }
},

{
  /*KNIGHTS*/
  {
  "axe", "sword", "flail", "polearm", "mace", "shield block", "parry",
  "dodge", "wear chain", "bow", "wear plate", "bash", "wear leather", "rescue",
  "enhanced damage", "whip", "spear", "dirt kicking", "second attack", "hand to hand",
  "guard", "fast healing", "kick", "battle tactics", "disarm", "third attack", "mount",
  "haggle", "trip", "trammel", "berserk", "nerve pinch", "caltrops", "mounted combat",
  "flank attack", "lore", "cleave", "whirlwind", "dual wield", "bind wounds", "lunge",
  "counter", "fourth attack", "meditation", "critical strike", "break weapon", "deathgrip",
  "frenzy", "protection evil", "staff", "eyerub", "finger break", "instruct",
  "smother", "heroic demise", "joust", "polish", "shatter shield", "company might",
  "company valor", "company honor", "arrow deflection"
  }
},

{
  /*BARBARIANS*/
  {
  "axe", "flail", "polearm", "spear", "whip", "dodge", "parry", "wear leather", "dagger",
  "mace", "shield block", "sword", "bash", "enhanced damage", "wear chain", "hand to hand",
  "dirt kicking", "headbutt", "second attack", "battle tactics", "fast healing", "guard",
  "kick", "trip", "berserk", "disarm", "third attack", "mount", "haggle", "envenom",
  "nerve pinch", "caltrops", "mounted combat", "dual wield", "cleave", "lore",
  "critical strike", "bind wounds", "breakneck", "butcher", "whirlwind", "counter",
  "shield rush", "ravage bite", "break weapon", "fourth attack", "frenzy", "sidestep",
  "entwine", "staff", "eyerub", "finger break", "execute", "earpunch", "brainjar",
  "rupture organ", "tonguecut", "instruct", "smother", "arrow deflection"
  }
},

{
  /*DRUIDS*/
  {
  "dagger", "dodge", "spear", "wear leather", "elven tongue", "drow tongue", "bow",
  "kagonesti tongue", "fast healing", "lore", "quiet movement", "jab", "dirt kicking",
  "battle tactics", "parry", "trip", "hand to hand", "haggle", "kick", "surge", "scout",
  "endure", "shield block", "second attack", "sweep attack", "disarm", "envenom", "mount",
  "pillify", "enhanced damage", "herbal discern", "pick lock", "scribe", "brew",
  "deathgrip", "sharpen", "sparkshot", "goodberry", "badberry", "orb of containment",
  "purify", "bark skin", "vortex", "windspirit", "stonespirit", "storm of vengeance",
  "fireproof", "detect presence", "natures warmth", "natures embrace", "sticks to snakes",
  "create rose", "druidstaff", "vermin plague", "breath of nature", "earthmeld", "teleport",
  "summon insects", "pass without trace", "sunray", "life transfer", "trance", "waterlungs",
  "fireseed", "immolation", "pine needles", "staff", "scrolls", "eyerub", "wrath of nature",
  "instruct", "call of the wild", "tame", "entangle", "plant growth", "treeform", "quicksand",
  "windtalk", "wilderness lore", "beastspite"
  }
},

{
  /*RANGERS*/
  {
  "axe", "flail", "dagger", "mace", "shield block", "sword", "dodge", "parry", "polearm",
  "spear", "whip", "enhanced damage", "wear chain", "wear leather", "bow", "bash",
  "dirt kicking", "second attack", "hand to hand", "fast healing", "scout", "kick",
  "guard", "trip", "disarm", "third attack", "haggle", "mount", "battle tactics",
  "quiet movement", "berserk", "camouflage", "herbal discern", "campfire", "search water",
  "bearcall", "track", "mounted combat", "hide", "trophy", "sneak", "butcher", "envenom",
  "ambush", "dual wield", "counter", "fourth attack", "forest vision", "farsight",
  "orb of containment", "grasping roots", "frenzy", "detect hidden", "disarm traps",
  "set traps", "staff", "eyerub", "instruct", "eagle eye", "call of the wild", "tame",
  "arrow deflection", "point-blank shot"
  }
},

{
  /*ASSASSINS*/
  {
  "detect poison", "infravision", "detect hidden", "haste", "axe", "flail", "polearm",
  "dagger", "mace", "spear", "whip", "dodge", "wear leather", "hide", "sword", "backstab",
  "trip", "haggle", "peek", "shield block", "pick lock", "dirt kicking", "sneak", "scout",
  "battle tactics", "envenom", "tumble", "disarm", "second attack", "kick", "hand to hand",
  "fast healing", "mount", "enhanced damage", "taunt", "dual wield", "strangle",
  "track", "demand", "circlestab", "third attack", "critical strike", "deathgrip",
  "counter", "assassinate", "eavesdrop", "second backstab", "staff", "eyerub", "tonguecut",
  "instruct", "stalk", "cutthroat"
  }
},

{
  /*SPELLFILCHERS*/ 
  {
  "infravision", "magic missile", "incendiary cloud", "levitate", "weaken",
  "giant strength", "shield", "luck", "bad luck", "flash", "acid blast", "identify",
  "haste", "empowerment", "detect hidden", "gate", "mind shatter", "word of recall",
  "invisibility", "dagger", "sword", "spear", "whip", "wear leather", "steal", "sneak",
  "backstab", "scout", "dodge", "surge", "endure", "fast healing", "battle tactics",
  "meditation", "haggle", "mount", "parry", "second attack", "enhanced damage", "hide",
  "pick lock", "taunt", "traject", "circlestab", "mountjack", "cantrip", "eavesdrop",
  "deafness", "disarm traps", "alarm", "staff", "eyerub", "instruct", "firetongue",
  "endure disease", "scrolls", "enlarge", "lock"
  }
},

{
  /*NECROMANCERS*/
  {
  "dagger", "whip", "elven tongue", "drow tongue", "dwarven tongue", "qualinesti tongue",
  "dargonesti tongue", "kender tongue", "ogre tongue", "scrolls", "wands", "staves",
  "minotaur tongue", "dimernesti tongue", "kagonesti tongue", "silvanesti tongue",
  "draconian tongue", "surge", "endure", "meditation", "jab", "fast healing", "dodge",
  "battle tactics", "mount", "traject", "scribe", "brew", "parry", "envenom",
  "second attack", "enhanced damage", "magic missile", "detect magic", "dark taint",
  "cantrip", "forgedeath", "coldfire", "deadeye", "chill touch",
  "orb of containment", "burning hands", "enervation", "invisibility", "armor", "infravision",
  "detect good", "weaken", "know alignment", "blindness", "protection good", "teleport",
  "boil blood", "detect poison", "dispel magic", "poison", "preservation", "energy drain",
  "curse", "detect presence", "knock", "plague", "great wasting", "fear", "acid blast",
  "harm", "horrid wilting", "seal door", "unseal door", "vampiric touch", "shadow walk",
  "wraithform", "animate corpse", "life transfer", "demonfire", "drain rune", "paralysis",
  "remove paralysis", "aid", "disintegrate", "undead disruption", "word of recall",
  "nightmare", "acid breath", "preserve dead", "cavorting bones", "ray of fatigue",
  "forget", "word of death", "coldlight", "resist cold", "renew bones", "intensify death",
  "breath of the dead", "deafness", "trance", "staff", "unholyrite", "drain blood", "eyerub",
  "instruct", "endure disease", "lock", "mark of origin"
  }
},

{
  /*ENCHANTERS*/
  {
  "dagger", "whip", "elven tongue", "drow tongue", "dwarven tongue", "qualinesti tongue",
  "dargonesti tongue", "kender tongue", "ogre tongue", "scrolls", "wands", "staves",
  "minotaur tongue", "dimernesti tongue", "kagonesti tongue", "silvanesti tongue",
  "draconian tongue", "surge", "endure", "meditation", "jab", "fast healing", "dodge",
  "battle tactics", "mount", "traject", "scribe", "brew", "parry", "envenom",
  "second attack", "enhanced damage", "taunt", "cantrip", "magic missile", "detect magic",
  "detect invis", "orb of containment", "invisibility", "glamour", "armor", "refresh", "sleep",
  "burning hands", "recharge", "weaken", "know alignment", "infravision", "levitate",
  "detect good", "blindness", "protection evil", "teleport", "farsight", "detect poison",
  "dispel magic", "enchant armor", "giant strength", "cancellation", "enchant weapon",
  "attunement", "cause serious", "charm person", "detect presence", "flesh to stone",
  "slow", "haste", "pass door", "earthquake", "locate object", "acid blast", "identify",
  "flaming shield", "flamestrike", "seal door", "unseal door", "heat metal", "binding",
  "ice sheild", "life transfer", "feeblemind", "knock", "sympathy", "fumble", "aid",
  "mind shatter", "word of recall", "flame rune", "frost rune", "break enchantment",
  "shocking rune", "sleepless curse", "empowerment", "garble", "shallow breath",
  "chaos flux", "gravity flux", "torment", "gate", "trance", "waterlungs", "staff",
  "eyerub", "instruct", "enlarge", "shrink", "lock", "mark of origin"
  }
},

{
  /*ILLUSIONISTS*/
  {
  "dagger", "whip", "elven tongue", "drow tongue", "dwarven tongue", "qualinesti tongue",
  "dargonesti tongue", "kender tongue", "ogre tongue", "scrolls", "wands", "staves",
  "minotaur tongue", "dimernesti tongue", "kagonesti tongue", "silvanesti tongue",
  "draconian tongue", "surge", "endure", "meditation", "jab", "fast healing", "dodge",
  "battle tactics", "mount", "traject", "scribe", "brew", "parry", "envenom", "cantrip",
  "second attack", "enhanced damage", "displacement", "magic missile", "detect magic",
  "detect invis", "orb of containment", "invisibility", "ventriloquate", "burning hands",
  "refresh", "weaken", "know alignment", "delusions of grandeur", "armor", "continual light",
  "infravision", "shocking grasp", "detect good", "blindness", "protection evil",
  "teleport", "glamour", "detect poison", "colour spray", "create rose", "dispel magic",
  "mirror", "shield", "fireball", "detect presence", "mass invis", "unseal door", "knock",
  "acid blast", "scrye", "darkness", "seal door", "dragon skin", "looking glass", "flash",
  "life transfer", "rainbow pattern", "shifting shadows", "illusion", "aid", "silence",
  "cause discord", "word of recall", "doppelganger", "fire breath", "frost breath",
  "ethereal warrior", "gas breath", "sequester", "lightning breath", "alter reality",
  "gate", "deafness", "trance", "cloak of evil", "hypnotic pattern", "staff",
  "phantom armor", "spook", "eyerub", "instruct", "lock", "mark of origin"
  }
},

{
  /*CONJURORS*/
  {
  "dagger", "whip", "elven tongue", "drow tongue", "dwarven tongue", "qualinesti tongue",
  "dargonesti tongue", "kender tongue", "ogre tongue", "scrolls", "wands", "staves",
  "minotaur tongue", "dimernesti tongue", "kagonesti tongue", "silvanesti tongue",
  "draconian tongue", "surge", "endure", "meditation", "jab", "fast healing", "dodge",
  "battle tactics", "mount", "traject", "scribe", "brew", "parry", "envenom", "cantrip",
  "second attack", "enhanced damage", "detect magic", "detect invis", "orb of containment",
  "invisibility", "incendiary cloud", "armor", "create water", "burning hands", "refresh",
  "recharge", "weaken", "know alignment", "teleport", "infravision", "create food",
  "detect good", "blindness", "protection evil", "storm of vengeance", "create spring",
  "detect poison", "dispel magic", "gust of wind", "glitterdust", "detect presence", 
  "iceblast", "closing weave", "knock", "gate", "call lightning", "wind wall",
  "life transfer", "mooncloak", "dimensional door", "aid", "word of recall",
  "fire breath", "frost breath", "unseen servant", "gas breath", "lightning breath",
  "convoke swarm", "magic missile", "thunderclap", "trance", "alarm", "nexus", "staff",
  "summon", "control weather", "rabbit summon", "eyerub", "instruct", "acid blast",
  "lock", "mark of origin", "conjure food"
  }
},

{
  /*BLACKSMITHS*/
  {
  "elven tongue", "drow tongue", "dwarven tongue", "qualinesti tongue",
  "dargonesti tongue", "kender tongue", "ogre tongue", "minotaur tongue",
  "dimernesti tongue", "kagonesti tongue", "silvanesti tongue", "draconian tongue",
  "mend", "repair shield", "leather repair", "chain repair", "plate repair",
  "weapon repair", "deathblade", "runeblade", "eversharp", "fast working",
  "stainless", "smith", "fletching", "flameblade", "purifying fire", "create furnace",
  "create barrel", "create grinder", "create anvil", "hand to hand", "haggle", "mount",
  "wear plate", "wear chain", "wear leather", "sharpen", "fast healing", "masterwork",
  "create flame", "thors hammer", "lore", "dismantle", "eyerub", "cultural artistry",
  "instruct", "key origin"
  }
}

};

const struct  religion_type	religion_table[MAX_RELIGION] =
{
  {
    "none", "none", "none", "God Name", 0, 0, 0, SECT_NONE
  },

 /*
  { Religion Short Name, Religion Long Name, Mark Description, God Name,
    Alignment, Mark VNUM, Devotee Vnum, Altar VNUM, Sector Type
  }
  */

  {
    "Corruption", "Corruption", "Earing of Corruption", "Hiddukel",
    -1000, 1231, 1232, SECT_CORRUPTION
  },

  {
    "Death", "Death", "Ashes of Death", "Chemosh",
    -1000, 1233, 1234, SECT_DEATH
  },

  {
    "Blessing", "Blessing", "Holy Symbol of Mishakal", "Mishakal",
    1000, 1235, 1236, SECT_BLESSING
  },

  {
    "Knowledge", "Knowledge", "Mark of Knowledge", "Gilean",
    0, 1237, 1238, SECT_KNOWLEDGE
  },

  {
    "Virtue", "Virtue", "Tattoo of a Platinum Dragon", "Paladine",
    1000, 1239, 1240, SECT_VIRTUE
  },

  {
    "Chivalry", "Chivalry", "Mark of Martial Valor", "Habbakuk",
    1000, 1241, 1242, SECT_CHIVALRY
  },

  {
    "Wealth", "Wealth", "Earing of Prosperity", "Shinare",
    0, 1243, 1244, SECT_WEALTH
  },

  {
    "Wisdom", "Wisdom", "Pendant of Wisdom", "Majere",
    1000, 1245, 1246, SECT_WISDOM
  },

  {
    "Beauty", "Beauty", "Branch of Raving Beauty", "Branchala",
    1000, 1247, 1248, SECT_BEAUTY
  },

  {
    "Disease", "Disease", "Boils of Disease", "Morgion",
    -1000, 1249, 1250, SECT_DISEASE
  },

  {
    "Nature", "Nature", "Golden Branch of Nature", "Chislev",
    0, 1251, 1252, SECT_NATURE
  },

  {
    "Flame", "Flame", "Searing Scorch of Flame", "Sirrion",
    0, 1253, 1254, SECT_FLAME
  },

  {
    "Destruction", "Destruction", "Embodiment of Destruction", "Sargonnas",
    0, 1255, 1256, SECT_DESTRUCTION
  },

  {
    "Justice", "Justice", "Mark of Holy Justice", "Kiri",
    1000, 1257, 1258, SECT_JUSTICE
  },

  {
    "Life", "Life", "Golden Bracelet of Life", "Zivilyn",
    0, 1259, 1260, SECT_LIFE
  },

  {
    "Hatred", "Hatred", "Scorch of Hatred", "Takhisis",
    -1000, 1261, 1262, SECT_HATRED
  },

  {
    "Storm", "Storm", "Rage of a Storm", "Zeboim",
    -1000, 1263, 1264, SECT_STORM
  },

  {
    "Battle", "Battle", "Scar of Battle", "Reorx",
    0, 1265, 1266, SECT_BATTLE
  },

  {
    "Rmagic", "RedMagic", "Essence of Red Magic", "Lunitari",
    0, 1267, 1268, SECT_RMAGIC
  },

  {
    "Bmagic", "BlackMagic", "Shadow of Black Magic", "Nuitari",
    -1000, 1269, 1270, SECT_BMAGIC
  },

  {
    "Wmagic", "WhiteMagic", "Radiance of White Magic", "Solinari",
    1000, 1271, 1272, SECT_WMAGIC
  }

};
