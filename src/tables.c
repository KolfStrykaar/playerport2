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
#include "tables.h"


/* for position */
const struct position_type position_table[] =
{
    {	"dead",			"dead"	},
    {	"mortally wounded",	"mort"	},
    {	"incapacitated",		"incap"	},
    {	"stunned",			"stun"	},
    {	"sleeping",			"sleep"	},
    {	"resting",			"rest"	},
    {  "sitting",			"sit"   	},
    {	"fighting",			"fight"	},
    {	"standing",			"stand"	},
    {	NULL,				NULL		}
};

/* for sex */
const struct sex_type sex_table[] =
{
   {	"none"	},
   {	"male"	},
   {	"female"	},
   {	"either"	},
   {	NULL		}
};

/* various flag tables */
const struct flag_type act_flags[] =
{
    {	"npc",		A,	FALSE	},
    {	"sentinel",		B,	TRUE	},
    {	"scavenger",	C,	TRUE	},
    {	"mountable", 	D, 	TRUE	}, 
    {	"aggressive",	F,	TRUE	},
    {	"stay_area",	G,	TRUE	},
    {	"wimpy",		H,	TRUE	},
    {	"pet",		I,	TRUE	},
    {	"train",		J,	TRUE	},
    {	"practice",		K,	TRUE	},
    { "bounty",         L,    TRUE  },
    { "track",		M,	TRUE	},
    { "no_xp_gain",	N,	TRUE	},
    {	"undead",		O,	TRUE	},
    {	"cleric",		Q,	TRUE	},
    {	"mage",		R,	TRUE	},
    {	"thief",		S,	TRUE	},
    {	"warrior",		T,	TRUE	},
    {	"noalign",		U,	TRUE	},
    {	"nopurge",		V,	TRUE	},
    {	"outdoors",		W,	TRUE	},
    {	"indoors",		Y,	TRUE	},
    {	"healer",		aa,	TRUE	},
    {	"gain",		bb,	TRUE	},
    {	"update_always",	cc,	TRUE	},
    {	"changer",		dd,	TRUE	},
    {	"banker",		ee,	TRUE	},
    {	NULL,			0,	FALSE	}
};

const struct flag_type plr_flags[] =
{
    {	"npc",		A,	FALSE	},
    { "nonote",		B,	FALSE	},
    {	"autoassist",	C,	FALSE	},
    {	"autoexit",		D,	FALSE	},
    {	"autoloot",		E,	FALSE	},
    {	"autosac",		F,	FALSE	},
    {	"autogold",		G,	FALSE	},
    {	"autosplit",	H,	FALSE	},
    {	"assault",		I,	FALSE	},
    {	"treason",		J,	FALSE	},
    {	"outlaw",		K,	FALSE	},
    { "automercy",	L,	FALSE	},
    { "autoyield",	M,	FALSE	},
    {	"holylight",	N,	FALSE	},
    {	"can_loot",		P,	FALSE	},
    { "autotitle",	Q,	FALSE	},
    {	"nofollow",		R,	FALSE	},
    {	"permit",		U,	TRUE	},
    {	"log",		W,	FALSE	},
    {	"deny",		X,	FALSE	},
    {	"freeze",		Y,	FALSE	},
    {	"thief",		Z,	FALSE	},
    {	"killer",		aa,	FALSE	},
    {	"notitle",		ee,	FALSE	},
    {	NULL,			0,	0	}
};

/* various flag tables */
const struct flag_type act2_flags[] =
{
    {	NULL,			0,	FALSE	}
};

const struct flag_type plr2_flags[] =
{
    {	"autodig",		A,	FALSE	},
    {	NULL,			0,	0	}
};

const struct flag_type affect_flags[] =
{
    {	"blind",			A,	TRUE	},
    {	"invisible",		B,	TRUE	},
    {	"detect_evil",		C,	TRUE	},
    {	"detect_invis",		D,	TRUE	},
    {	"detect_magic",		E,	TRUE	},
    {	"detect_hidden",		F,	TRUE	},
    {	"detect_good",		G,	TRUE	},
    {	"sanctuary",		H,	TRUE	},
    {	"incendiary_cloud",	I,	TRUE	},
    {	"infrared",			J,	TRUE	},
    {	"curse",			K,	TRUE	},
    {	"poison",			M,	TRUE	},
    {	"protect_evil",		N,	TRUE	},
    {	"protect_good",		O,	TRUE	},
    {	"sneak",			P,	TRUE	},
    {	"hide",			Q,	TRUE	},
    {	"sleep",			R,	TRUE	},
    {	"charm",			S,	TRUE	},
    {	"flying",			T,	TRUE	},
    {	"pass_door",		U,	TRUE	},
    {	"haste",			V,	TRUE	},
    {	"calm",			W,	TRUE	},
    {	"plague",			X,	TRUE	},
    {	"weaken",			Y,	TRUE	},
    {	"dark_vision",		Z,	TRUE	},
    {	"berserk",			aa,	TRUE	},
    {	"waterbreathing",		bb,	TRUE	},
    {	"regeneration",		cc,	TRUE	},
    {	"slow",			dd,	TRUE	},
    { "shackles",           	ee,   FALSE  },
    {	NULL,			0,	0	}
};

const struct flag_type affect2_flags[] =
{
    {	"forest_vision",		A,	FALSE	},
    {	"blackjack",		B,	FALSE	},
    {	"bounty",			C,	FALSE	},
    {	"glide",			D,	FALSE	},
    {	"silence",			E,	FALSE	},
    {	"fear",			F,	FALSE	},
    {	"paralysis",		G,	FALSE	},
    {	"undetectable_align",	H,	FALSE	},
    {	"repulsion",		I,	FALSE	},
    {	"sympathy",			J,	FALSE	},
    {	"rainbow_pattern",	K,	FALSE	},
    {	"wraithform",		L,	FALSE	},
    {	"nature",			M,	FALSE	},
    {	"warmth",			N,	FALSE	},
    {	"herbs",			O,	FALSE	},
    {	"enlightenment",		P,	FALSE	},
    {	"dark_taint",		Q,	FALSE	},
    {	"forget",			R,	FALSE	},
    {	"paradise",			S,	FALSE	},
    {	"invul",			T,	FALSE	},
    {	"camouflage",		U,	FALSE	},
    {	"looking_glass",		V,	FALSE	},
    {	"passtrace",		W,	FALSE	},
    { "sleepcurse",		X,	FALSE },
    { "mooncloak",		Y,	FALSE },
    { "bravado",			Z,	FALSE },
    { "wasting",			aa,	FALSE },
    { "farsight",			bb,	FALSE },
    { "speech_impair",		cc,	FALSE	},
    { "deafness",			dd,	FALSE	},
    { "swim",			ee,	FALSE	},
    {	NULL,			0,	0	}
};

const struct flag_type affect3_flags[] =
{
    { "rally",			A,	FALSE	},
    { "gaura",			B,	FALSE	},
    { "eaura",			C,	FALSE	},
    { "holdperson",		D,	FALSE	},
    { "immolation",		E,	FALSE	},
    { "pineneedles",		F,	FALSE	},
    { "fire",			G,	FALSE	},
    { "instruction",		H,	FALSE	},
    { "subdue",			I,	FALSE	},
    { "might",			J,	FALSE	},
    { "valor",			K,	FALSE	},
    { "honor",			L,	FALSE	},
    { "preach",			M,	FALSE	},
    {	"clearmind",		N,	FALSE	},
    {	"divine_favor",		O,	FALSE	},
    {	"quicksand",		P,	FALSE	},
    {	"beastspite_bear",	Q,	FALSE	},
    {	"beastspite_lizard",	R,	FALSE	},
    {	"beastspite_fox",		S,	FALSE	},
    {	"beastspite_wolf",	T,	FALSE	},
    {	"beastspite_wyvern",	U,	FALSE	},
    {	"beastspite_dragon",	V,	FALSE	},
    {	"beastspite_bat",		W,	FALSE	},
    {	"blessed_watchfulness",	X,	FALSE	},
    {	"blood_bond",		Y,	FALSE	},
    {	NULL,			0,	0	}
};

const struct flag_type off_flags[] =
{
    {	"area_attack",		A,	TRUE	},
    {	"backstab",			B,	TRUE	},
    {	"bash",			C,	TRUE	},
    {	"berserk",			D,	TRUE	},
    {	"disarm",			E,	TRUE	},
    {	"dodge",			F,	TRUE	},
    {	"fast",			H,	TRUE	},
    {	"kick",			I,	TRUE	},
    {	"dirt_kick",		J,	TRUE	},
    {	"parry",			K,	TRUE	},
    {	"rescue",			L,	TRUE	},
    {	"tail",			M,	TRUE	},
    {	"trip",			N,	TRUE	},
    {	"crush",			O,	TRUE	},
    {	"assist_all",		P,	TRUE	},
    {	"assist_align",		Q,	TRUE	},
    {	"assist_race",		R,	TRUE	},
    {	"assist_players",		S,	TRUE	},
    {	"assist_guard",		T,	TRUE	},
    {	"assist_vnum",		U,	TRUE	},
    {	NULL,				0,	0	}
};

const struct flag_type imm_flags[] =
{
    {	"summon",		A,	TRUE	},
    {	"charm",		B,	TRUE	},
    {	"magic",		C,	TRUE	},
    {	"weapon",		D,	TRUE	},
    {	"bash",		E,	TRUE	},
    {	"pierce",		F,	TRUE	},
    {	"slash",		G,	TRUE	},
    {	"fire",		H,	TRUE	},
    {	"cold",		I,	TRUE	},
    {	"lightning",	J,	TRUE	},
    {	"acid",		K,	TRUE	},
    {	"poison",		L,	TRUE	},
    {	"negative",		M,	TRUE	},
    {	"holy",		N,	TRUE	},
    {	"energy",		O,	TRUE	},
    {	"mental",		P,	TRUE	},
    {	"disease",		Q,	TRUE	},
    {	"drowning",		R,	TRUE	},
    {	"light",		S,	TRUE	},
    {	"sound",		T,	TRUE	},
    {	"wood",		X,	TRUE	},
    {	"silver",		Y,	TRUE	},
    {	"iron",		Z,	TRUE	},
    {	NULL,			0,	0	}
};

const struct flag_type form_flags[] =
{
    {	"edible",		FORM_EDIBLE,		TRUE	},
    {	"poison",		FORM_POISON,		TRUE	},
    {	"magical",		FORM_MAGICAL,		TRUE	},
    {	"instant_decay",	FORM_INSTANT_DECAY,	TRUE	},
    {	"other",		FORM_OTHER,			TRUE	},
    {	"animal",		FORM_ANIMAL,		TRUE	},
    {	"sentient",		FORM_SENTIENT,		TRUE	},
    {	"undead",		FORM_UNDEAD,		TRUE	},
    {	"construct",	FORM_CONSTRUCT,		TRUE	},
    {	"mist",		FORM_MIST,			TRUE	},
    {	"intangible",	FORM_INTANGIBLE,		TRUE	},
    {	"biped",		FORM_BIPED,			TRUE	},
    {	"centaur",		FORM_CENTAUR,		TRUE	},
    {	"insect",		FORM_INSECT,		TRUE	},
    {	"spider",		FORM_SPIDER,		TRUE	},
    {	"crustacean",	FORM_CRUSTACEAN,		TRUE	},
    {	"worm",		FORM_WORM,			TRUE	},
    {	"blob",		FORM_BLOB,			TRUE	},
    {	"mammal",		FORM_MAMMAL,		TRUE	},
    {	"bird",		FORM_BIRD,			TRUE	},
    {	"reptile",		FORM_REPTILE,		TRUE	},
    {	"snake",		FORM_SNAKE,			TRUE	},
    {	"dragon",		FORM_DRAGON,		TRUE	},
    {	"amphibian",	FORM_AMPHIBIAN,		TRUE	},
    {	"fish",		FORM_FISH ,			TRUE	},
    {	"cold_blood",	FORM_COLD_BLOOD,		TRUE	},
    {	NULL,			0,				0	}
};

const struct flag_type part_flags[] =
{
    {	"head",		PART_HEAD,			TRUE	},
    {	"arms",		PART_ARMS,			TRUE	},
    {	"legs",		PART_LEGS,			TRUE	},
    {	"heart",		PART_HEART,			TRUE	},
    {	"brains",		PART_BRAINS,		TRUE	},
    {	"guts",		PART_GUTS,			TRUE	},
    {	"hands",		PART_HANDS,			TRUE	},
    {	"feet",		PART_FEET,			TRUE	},
    {	"fingers",		PART_FINGERS,		TRUE	},
    {	"ear",		PART_EAR,			TRUE	},
    {	"eye",		PART_EYE,			TRUE	},
    {	"long_tongue",	PART_LONG_TONGUE,		TRUE	},
    {	"eyestalks",	PART_EYESTALKS,		TRUE	},
    {	"tentacles",	PART_TENTACLES,		TRUE	},
    {	"fins",		PART_FINS,			TRUE	},
    {	"wings",		PART_WINGS,			TRUE	},
    {	"tail",		PART_TAIL,			TRUE	},
    {	"claws",		PART_CLAWS,			TRUE	},
    {	"fangs",		PART_FANGS,			TRUE	},
    {	"horns",		PART_HORNS,			TRUE	},
    {	"scales",		PART_SCALES,		TRUE	},
    {	"tusks",		PART_TUSKS,			TRUE	},
    {	NULL,			0,			0	}
};

const struct flag_type comm_flags[] =
{
    {   "quiet",			COMM_QUIET,				TRUE	},
    {   "deaf",			COMM_DEAF,				TRUE	},
    {   "nowiz",			COMM_NOWIZ,				TRUE	},
    {   "nopkill",		COMM_NOPKILL,			TRUE	},
    {   "noooc",			COMM_NOOOC,				TRUE	},
    {   "noic", 			COMM_NOIC,      			TRUE	},
    {   "nomusic",		COMM_NOMUSIC,			TRUE	},
    {   "noclan",			COMM_NOCLAN,			TRUE	},
    {   "noquote",		COMM_NOQUOTE,			TRUE	},
    {   "shoutsoff",		COMM_SHOUTSOFF,			TRUE	},
    {   "compact",		COMM_COMPACT,			TRUE	},
    {   "brief",			COMM_BRIEF,				TRUE	},
    {   "prompt",			COMM_PROMPT,			TRUE	},
    {   "combine",		COMM_COMBINE,			TRUE	},
    {   "telnet_ga",		COMM_TELNET_GA,			TRUE	},
    {   "show_affects",		COMM_SHOW_AFFECTS,		TRUE	},
    {   "nograts",		COMM_NOGRATS,			TRUE	},
    {	  "editing",		COMM_EDITOR,			TRUE	},
    {   "noemote",		COMM_NOEMOTE,			FALSE	},
    {   "noshout",		COMM_NOSHOUT,			FALSE	},
    {   "notell",			COMM_NOTELL,			FALSE	},
    {   "nochannels",		COMM_NOCHANNELS,			FALSE	},
    {   "snoop_proof",		COMM_SNOOP_PROOF,			FALSE	},
    {   "afk",			COMM_AFK,				TRUE	},
    {   "nopray",             COMM_NOPRAY,            	TRUE  },
    {	  "writing",		COMM_WRITING,			TRUE	},
    {	  "nohero",			COMM_NOHERO,			TRUE	},
    {   "noidle",			COMM_NOIDLE,			FALSE	},
    {   "slowage",		COMM_SLOWAGE,			FALSE	},
    {   "maskedip",		COMM_MASKEDIP,			FALSE	},
    {	NULL,				0,					0	}
};

const struct flag_type mprog_flags[] =
{
    {	"act",	TRIG_ACT,		TRUE	},
    {	"bribe",	TRIG_BRIBE,		TRUE 	},
    {	"death",	TRIG_DEATH,		TRUE  },
    {	"entry",	TRIG_ENTRY,		TRUE	},
    {	"fight",	TRIG_FIGHT,		TRUE	},
    {	"give",	TRIG_GIVE,		TRUE	},
    {	"greet",	TRIG_GREET,		TRUE  },
    {	"grall",	TRIG_GRALL,		TRUE	},
    {	"kill",	TRIG_KILL,		TRUE	},
    {	"hpcnt",	TRIG_HPCNT,		TRUE  },
    {	"random",	TRIG_RANDOM,	TRUE	},
    {	"speech",	TRIG_SPEECH,	TRUE	},
    {	"exit",	TRIG_EXIT,		TRUE  },
    {	"exall",	TRIG_EXALL,		TRUE  },
    {	"delay",	TRIG_DELAY,		TRUE  },
    {	"surr",	TRIG_SURR,		TRUE  },
    {	NULL,			0,		TRUE	}
};

const struct flag_type area_flags[] =
{
    { "none",		AREA_NONE,		FALSE	},
    { "changed",	AREA_CHANGED,		TRUE	},
    { "added",		AREA_ADDED,		TRUE	},
    { "loading",	AREA_LOADING,		FALSE	},
    { NULL,		0,			0	}
};

const struct flag_type control_flags[] =
{
    { "nobody",		CONTROL_NOBODY,		TRUE	},
    { "solamnics",	CONTROL_SOLAMNICS,	TRUE	},
    { "kot",		CONTROL_KOT,		TRUE	},
    { NULL,		0,			0	}
};

const struct flag_type sex_flags[] =
{
    { "male",		SEX_MALE,		TRUE	},
    { "female",		SEX_FEMALE,		TRUE	},
    { "neutral",	SEX_NEUTRAL,		TRUE	},
    { "random",		3,          		TRUE	},   /* ROM */
    { "none",		SEX_NEUTRAL,		TRUE	},
    { NULL,		0,			0	}
};



const struct flag_type exit_flags[] =
{
    { "door",		EX_ISDOOR,		TRUE  	},
    { "closed",		EX_CLOSED,		TRUE	},
    { "locked",		EX_LOCKED,		TRUE	},
    { "pickproof",	EX_PICKPROOF,		TRUE	},
    { "nopass",		EX_NOPASS,		TRUE	},
    { "easy",		EX_EASY,		TRUE	},
    { "hard",		EX_HARD,		TRUE	},
    { "infuriating",	EX_INFURIATING,		TRUE	},
    { "noclose",	EX_NOCLOSE,		TRUE	},
    { "nolock",		EX_NOLOCK,		TRUE	},
    { "magicseal",	EX_MAGICSEAL,		TRUE	},
    { NULL,		0,			0	}
};



const struct flag_type door_resets[] =
{
    { "open and unlocked",	0,		TRUE	},
    { "closed and unlocked",	1,		TRUE	},
    { "closed and locked",	2,		TRUE	},
    { NULL,			0,		0	}
};



const struct flag_type room_flags[] =
{
    { "dark",		ROOM_DARK,		TRUE	},
    { "no_mob",		ROOM_NO_MOB,		TRUE	},
    { "indoors",	ROOM_INDOORS,		TRUE	},
    { "balance",	ROOM_BALANCE,		TRUE	},
    { "corrupt",	ROOM_CORRUPT,		TRUE	},
    { "sanctify",	ROOM_SANCTIFY,		TRUE	},
    { "private",	ROOM_PRIVATE,		TRUE  	},
    { "safe",		ROOM_SAFE,		TRUE	},
    { "solitary",	ROOM_SOLITARY,		TRUE	},
    { "pet_shop",	ROOM_PET_SHOP,		TRUE	},
    { "no_recall",	ROOM_NO_RECALL,		TRUE	},
    { "imp_only",	ROOM_IMP_ONLY,		TRUE  	},
    { "gods_only",	ROOM_GODS_ONLY,		TRUE  	},
    { "heroes_only",	ROOM_HEROES_ONLY,	TRUE	},
    { "newbies_only",	ROOM_NEWBIES_ONLY,	TRUE	},
    { "law",		ROOM_LAW,		TRUE	},
    { "nowhere",	ROOM_NOWHERE,		TRUE	},
    { "bank",		ROOM_BANK,		TRUE	}, // C036
    { "nomagic",	ROOM_NOMAGIC,		TRUE	}, // C076
    { "underground",	ROOM_UNDERGROUND,	TRUE	},
    { "temple",		ROOM_TEMPLE,		TRUE	},
    { "quiet",		ROOM_QUIET,		TRUE	},
    { "light",		ROOM_LIGHT,		TRUE	}, //Perm Light in Room
    { NULL,		0,			0	}
};



const struct flag_type sector_flags[] =
{
    { "forest", 	SECT_FOREST,		TRUE	},
    { "field", 		SECT_FIELD,		TRUE	},
    { "hills", 		SECT_HILLS,		TRUE	},
    { "mountain",	SECT_MOUNTAIN,		TRUE	},
    { "swim", 		SECT_WATER_SWIM,	TRUE	},
    { "noswim", 	SECT_WATER_NOSWIM,	TRUE	},
    { "unused", 	SECT_UNUSED,		TRUE	},
    { "air", 		SECT_AIR,		TRUE	},
    { "desert", 	SECT_DESERT,		TRUE	},
    { "inside", 	SECT_INSIDE,		TRUE	},
    { "city", 		SECT_CITY,		TRUE	},
    { "graveyard",	SECT_GRAVEYARD,		TRUE	},
    { "polar",		SECT_POLAR,		TRUE	},
    { "swamp",		SECT_SWAMP,		TRUE	},
    { "transport",	SECT_TRANSPORT,		TRUE	},
    { "underwater",	SECT_UNDERWATER,	TRUE	},
    { "kot",		SECT_KOT,		TRUE	},
    { "solamnic",	SECT_SOLAMNIC,		TRUE	},
    { "conclave",	SECT_CONCLAVE,		TRUE	},
    { "aesthetic",	SECT_AESTHETIC,		TRUE	},
    { "thieves",	SECT_THIEVES,		TRUE	},
    { "holyorder",	SECT_HOLYORDER,		TRUE	},
    { "mercenary",	SECT_MERCENARY,		TRUE	},
    { "undead",		SECT_UNDEAD,		TRUE	},
    { "forester",	SECT_FORESTER,		TRUE	},
    { "artisans",	SECT_ARTISANS,		TRUE	},
    { "blackorder",	SECT_BLACKORDER,	TRUE	},
    { "corruption",	SECT_CORRUPTION,	TRUE 	},
    { "death",		SECT_DEATH,		TRUE 	},
    { "blessing",	SECT_BLESSING,		TRUE 	},
    { "knowledge",	SECT_KNOWLEDGE,		TRUE 	},
    { "virtue",		SECT_VIRTUE,		TRUE 	},
    { "chivalry",	SECT_CHIVALRY,		TRUE 	},
    { "wealth",		SECT_WEALTH,		TRUE 	},
    { "wisdom",		SECT_WISDOM,		TRUE 	},
    { "beauty",		SECT_BEAUTY,		TRUE 	},
    { "disease",	SECT_DISEASE,		TRUE 	},
    { "nature",		SECT_NATURE,		TRUE 	},
    { "flame",		SECT_FLAME,		TRUE 	},
    { "rage",		SECT_DESTRUCTION,	TRUE 	},
    { "justice",	SECT_JUSTICE,		TRUE 	},
    { "life",		SECT_LIFE,		TRUE 	},
    { "hatred",		SECT_HATRED,		TRUE 	},
    { "storm",		SECT_STORM,		TRUE 	},
    { "battle",		SECT_BATTLE,		TRUE 	},
    { "redmagic",	SECT_RMAGIC,		TRUE 	},
    { "blackmagic",	SECT_BMAGIC,		TRUE 	},
    { "whitemagic",	SECT_WMAGIC,		TRUE 	},
    { "none",		SECT_NONE,		FALSE	},
    { NULL,		0,			0	}
};




const struct flag_type type_flags[] =
{
    {	"light",		ITEM_LIGHT,			TRUE	},
    {	"scroll",		ITEM_SCROLL,		TRUE	},
    {	"wand",		ITEM_WAND,			TRUE	},
    {	"staff",		ITEM_STAFF,			TRUE	},
    {	"weapon",		ITEM_WEAPON,		TRUE	},
    {	"treasure",		ITEM_TREASURE,		TRUE	},
    {	"armor",		ITEM_ARMOR,			TRUE	},
    {	"potion",		ITEM_POTION,		TRUE	},
    {	"furniture",	ITEM_FURNITURE,		TRUE	},
    {	"trash",		ITEM_TRASH,			TRUE	},
    {	"container",	ITEM_CONTAINER,		TRUE	},
    {	"drinkcontainer",	ITEM_DRINK_CON,		TRUE	},
    {	"key",		ITEM_KEY,			TRUE	},
    {	"food",		ITEM_FOOD,			TRUE	},
    {	"money",		ITEM_MONEY,			TRUE	},
    {	"boat",		ITEM_BOAT,			TRUE	},
    {	"npccorpse",	ITEM_CORPSE_NPC,		TRUE	},
    {	"pc corpse",	ITEM_CORPSE_PC,		FALSE	},
    {	"fountain",		ITEM_FOUNTAIN,		TRUE	},
    {	"pill",		ITEM_PILL,			TRUE	},
    {	"protect",		ITEM_PROTECT,		TRUE	},
    {	"map",		ITEM_MAP,			TRUE	},
    { "portal",		ITEM_PORTAL,		TRUE	},
    { "warpstone",	ITEM_WARP_STONE,		TRUE	},
    {	"roomkey",		ITEM_ROOM_KEY,		TRUE	},
    { "gem",		ITEM_GEM,			TRUE	},
    {	"jewelry",		ITEM_JEWELRY,		TRUE	},
    {	"jukebox",		ITEM_JUKEBOX,		TRUE	},
    {	"tattoo",		ITEM_TATTOO,		TRUE	},
    {	"wetstone",		ITEM_WET_STONE,		TRUE	},
    {	"instrument",	ITEM_INSTRUMENT,		TRUE	},
    {	"stone",		ITEM_STONE,			TRUE	},
    {	"quiver",		ITEM_QUIVER,		TRUE	},
    {	"arrow",		ITEM_ARROW,			TRUE	},
    { "bhammer",		ITEM_BHAMMER,		TRUE	},
    { "anvil",		ITEM_ANVIL,			TRUE  },
    { "raw material",	ITEM_RAW_MATERIAL,	TRUE  },
    { "tool kit",		ITEM_TOOL_KIT,		TRUE	},
    { "boobytrap",	ITEM_BOOBYTRAP,		TRUE	},
    { "trap",		ITEM_TRAP,			TRUE	},
    { "furnace",		ITEM_FURNACE,		TRUE	},
    { "barrel",		ITEM_BARREL,		TRUE	},
    { "grinder",		ITEM_GRINDER,		TRUE	},
    {	NULL,			0,			0	}
};


const struct flag_type extra_flags[] =
{
    {	"glow",		ITEM_GLOW,			TRUE	},
    {	"hum",		ITEM_HUM,			TRUE	},
    {	"dark",		ITEM_DARK,			TRUE	},
    {	"masterwork",	ITEM_MASTERWORK,		FALSE	},
    {	"evil",		ITEM_EVIL,			TRUE	},
    {	"invis",		ITEM_INVIS,			TRUE	},
    {	"magic",		ITEM_MAGIC,			TRUE	},
    {	"nodrop",		ITEM_NODROP,		TRUE	},
    {	"bless",		ITEM_BLESS,			TRUE	},
    {	"antigood",		ITEM_ANTI_GOOD,		TRUE	},
    {	"antievil",		ITEM_ANTI_EVIL,		TRUE	},
    {	"antineutral",	ITEM_ANTI_NEUTRAL,	TRUE	},
    {	"noremove",		ITEM_NOREMOVE,		TRUE	},
//  {	"inventory",	ITEM_INVENTORY,		TRUE	}, //C074
    {	"nopurge",		ITEM_NOPURGE,		TRUE	},
    {	"rotdeath",		ITEM_ROT_DEATH,		TRUE	},
    {	"visdeath",		ITEM_VIS_DEATH,		TRUE	},
    { "nonmetal",		ITEM_NONMETAL,		TRUE	},
    {	"meltdrop",		ITEM_MELT_DROP,		TRUE	},
    {	"hadtimer",		ITEM_HAD_TIMER,		TRUE	},
    {	"sellextract",	ITEM_SELL_EXTRACT,	TRUE	},
    {	"burnproof",	ITEM_BURN_PROOF,		TRUE	},
    {	"nouncurse",	ITEM_NOUNCURSE,		TRUE	},
    {	"nolocate",		ITEM_NOLOCATE,		TRUE	},
    {	"noident",		ITEM_NOIDENT,		TRUE	},
    { "questobj",		ITEM_QUESTOBJ,		TRUE	},
    { "stainless",	ITEM_STAINLESS,		FALSE	},
    {	NULL,			0,			0	}
};

const struct flag_type extra2_flags[] =
{
    {	"unique",		ITEM_UNIQUE,		TRUE	},
    {	"artistry",		ITEM_ARTISTRY,		FALSE	},
    {	NULL,			0,			0	}
};



const struct flag_type wear_flags[] =
{
    {	"take",		ITEM_TAKE,			TRUE	},
    {	"finger",		ITEM_WEAR_FINGER,		TRUE	},
    {	"neck",		ITEM_WEAR_NECK,		TRUE	},
    {	"body",		ITEM_WEAR_BODY,		TRUE	},
    {	"head",		ITEM_WEAR_HEAD,		TRUE	},
    {	"legs",		ITEM_WEAR_LEGS,		TRUE	},
    {	"feet",		ITEM_WEAR_FEET,		TRUE	},
    {	"hands",		ITEM_WEAR_HANDS,		TRUE	},
    {	"arms",		ITEM_WEAR_ARMS,		TRUE	},
    {	"shield",		ITEM_WEAR_SHIELD,		TRUE	},
    {	"about",		ITEM_WEAR_ABOUT,		TRUE	},
    {	"waist",		ITEM_WEAR_WAIST,		TRUE	},
    {	"wrist",		ITEM_WEAR_WRIST,		TRUE	},
    {	"wield",		ITEM_WIELD,			TRUE	},
    {	"hold",		ITEM_HOLD,			TRUE	},
    { "nosac",		ITEM_NO_SAC,		TRUE	},
    {	"wearfloat",	ITEM_WEAR_FLOAT,		TRUE	},
    {	"tattoo",		ITEM_WEAR_TATTOO,		TRUE	},
/*  { "twohands",     	ITEM_TWO_HANDS,		TRUE  }, */
    { "shoulders",	ITEM_WEAR_SHOULDERS,	TRUE	},
    { "ankle",		ITEM_WEAR_ANKLE,		TRUE	},
    { "face",		ITEM_WEAR_FACE,		TRUE	},
    { "mark",		ITEM_WEAR_MARK,		TRUE	},
    { "tiny",		ITEM_SIZE_TINY,		TRUE	},
    { "small",		ITEM_SIZE_SMALL,		TRUE	},
    { "medium",		ITEM_SIZE_MEDIUM,		TRUE	},
    { "large",		ITEM_SIZE_LARGE,		TRUE	},
    { "giant",		ITEM_SIZE_GIANT,		TRUE	},
    { "huge",		ITEM_SIZE_HUGE,		TRUE	},
    { "ignoresize",	ITEM_SIZE_IGNORE,		TRUE	},
    {	NULL,			0,			0	}
};

/*
 * Used when adding an affect to tell where it goes.
 * See addaffect and delaffect in act_olc.c
 */
const struct flag_type apply_flags[] =
{
    {	"none",		APPLY_NONE,			TRUE	},
    {	"strength",		APPLY_STR,			TRUE	},
    {	"dexterity",	APPLY_DEX,			TRUE	},
    {	"intelligence",	APPLY_INT,			TRUE	},
    {	"wisdom",		APPLY_WIS,			TRUE	},
    {	"constitution",	APPLY_CON,			TRUE	},
    {	"sex",		APPLY_SEX,			TRUE	},
    {	"class",		APPLY_CLASS,		TRUE	},
    {	"level",		APPLY_LEVEL,		TRUE	},
    {	"age",		APPLY_AGE,			TRUE	},
    {	"height",		APPLY_HEIGHT,		TRUE	},
    {	"weight",		APPLY_WEIGHT,		TRUE	},
    {	"mana",		APPLY_MANA,			TRUE	},
    {	"hp",			APPLY_HIT,			TRUE	},
    {	"move",		APPLY_MOVE,			TRUE	},
    {	"steel",		APPLY_STEEL,		TRUE	},
    {	"experience",	APPLY_EXP,			TRUE	},
    {	"ac",			APPLY_AC,			TRUE	},
    {	"hitroll",		APPLY_HITROLL,		TRUE	},
    {	"damroll",		APPLY_DAMROLL,		TRUE	},
    {	"saves",		APPLY_SAVES,		TRUE	},
    {	"savingpara",	APPLY_SAVING_PARA,	TRUE	},
    {	"savingrod",	APPLY_SAVING_ROD,		TRUE	},
    {	"savingpetri",	APPLY_SAVING_PETRI,	TRUE	},
    {	"savingbreath",	APPLY_SAVING_BREATH,	TRUE	},
    {	"savingspell",	APPLY_SAVING_SPELL,	TRUE	},
    {	"spellaffect",	APPLY_SPELL_AFFECT,	FALSE	},
    { "charisma",		APPLY_CHR,			TRUE	},
    { "size",		APPLY_SIZE,			TRUE	},
    {	NULL,			0,			0	}
};



/*
 * What is seen.
 */
const struct flag_type wear_loc_strings[] =
{
    {	"in the inventory",	WEAR_NONE,			TRUE	},
    {	"as a light",		WEAR_LIGHT,			TRUE	},
    {	"on the left finger",	WEAR_FINGER_L,		TRUE	},
    {	"on the right finger",	WEAR_FINGER_R,		TRUE	},
    {	"around the neck (1)",	WEAR_NECK_1,		TRUE	},
    {	"around the neck (2)",	WEAR_NECK_2,		TRUE	},
    {	"on the body",		WEAR_BODY,			TRUE	},
    {	"over the head",		WEAR_HEAD,			TRUE	},
    { "over the face",		WEAR_FACE,			TRUE	},
    {	"on the legs",		WEAR_LEGS,			TRUE	},
    { "on the left ankle",	WEAR_ANKLE_L,		TRUE	},
    { "on the right ankle",	WEAR_ANKLE_R,		TRUE	},
    {	"on the feet",		WEAR_FEET,			TRUE	},
    {	"on the hands",		WEAR_HANDS,			TRUE	},
    {	"on the arms",		WEAR_ARMS,			TRUE	},
    { "on the shoulders",	WEAR_SHOULDERS,		TRUE	},
    {	"as a shield",		WEAR_SHIELD,		TRUE	},
    {	"about the shoulders",	WEAR_ABOUT,			TRUE	},
    {	"around the waist",	WEAR_WAIST,			TRUE	},
    {	"on the left wrist",	WEAR_WRIST_L,		TRUE	},
    {	"on the right wrist",	WEAR_WRIST_R,		TRUE	},
    {	"wielded",			WEAR_WIELD,			TRUE	},
    {	"held in the hands",	WEAR_HOLD,			TRUE	},
    {	"floating nearby",	WEAR_FLOAT,			TRUE	},
    {	"tattooed",			WEAR_TATTOO,		TRUE	},
    {	"secondary",          	WEAR_SECONDARY,        	TRUE	},
    {   "marked",		WEAR_MARK,		TRUE	},
    {	NULL,				0,				0	}
};


const struct flag_type wear_loc_flags[] =
{
    {	"none",		WEAR_NONE,			TRUE	},
    {	"light",		WEAR_LIGHT,			TRUE	},
    {	"lfinger",		WEAR_FINGER_L,		TRUE	},
    {	"rfinger",		WEAR_FINGER_R,		TRUE	},
    {	"neck1",		WEAR_NECK_1,		TRUE	},
    {	"neck2",		WEAR_NECK_2,		TRUE	},
    {	"body",		WEAR_BODY,			TRUE	},
    {	"head",		WEAR_HEAD,			TRUE	},
    { "face",		WEAR_FACE,			TRUE	},
    {	"legs",		WEAR_LEGS,			TRUE	},
    { "lankle",		WEAR_ANKLE_L,		TRUE	},
    { "rankle",		WEAR_ANKLE_R,		TRUE	},
    {	"feet",		WEAR_FEET,			TRUE	},
    {	"hands",		WEAR_HANDS,			TRUE	},
    {	"arms",		WEAR_ARMS,			TRUE	},
    { "shoulders",	WEAR_SHOULDERS,		TRUE	},
    {	"shield",		WEAR_SHIELD,		TRUE	},
    {	"about",		WEAR_ABOUT,			TRUE	},
    {	"waist",		WEAR_WAIST,			TRUE	},
    {	"lwrist",		WEAR_WRIST_L,		TRUE	},
    {	"rwrist",		WEAR_WRIST_R,		TRUE	},
    {	"wielded",		WEAR_WIELD,			TRUE	},
    {	"hold",		WEAR_HOLD,			TRUE	},
    {	"floating",		WEAR_FLOAT,			TRUE	},
    {	"tattoo",		WEAR_TATTOO,		TRUE	},
    {	"secondary",	WEAR_SECONDARY,    	TRUE	},
    {   "mark",		WEAR_MARK,		TRUE	},
    {	NULL,		0,		0	}
};

const struct flag_type container_flags[] =
{
    {	"closeable",	1,		TRUE	},
    {	"pickproof",	2,		TRUE	},
    {	"closed",		4,		TRUE	},
    {	"locked",		8,		TRUE	},
    {	"puton",		16,		TRUE	},
    {	NULL,			0,		0	}
};

const struct flag_type guild_flags[] =
{
   {	"Independent",	GUILD_INDEPENDENT,	TRUE 	},
   {	"Modified",		GUILD_CHANGED,		FALSE	},
   {	"Delete",		GUILD_DELETED,		TRUE	},
   {	"Immortal",		GUILD_IMMORTAL,		TRUE	},
   {	NULL,			0,				FALSE	}
};


/*****************************************************************************
                      ROM - specific tables:
 ****************************************************************************/




const struct flag_type ac_type[] =
{
    {   "pierce",        AC_PIERCE,            	TRUE    },
    {   "bash",          AC_BASH,              	TRUE    },
    {   "slash",         AC_SLASH,             	TRUE    },
    {   "exotic",        AC_EXOTIC,            	TRUE    },
    {   NULL,            0,                     0       }
};


const struct flag_type size_flags[] =
{
    {   "tiny",          SIZE_TINY,            TRUE    },
    {   "small",         SIZE_SMALL,           TRUE    },
    {   "medium",        SIZE_MEDIUM,          TRUE    },
    {   "large",         SIZE_LARGE,           TRUE    },
    {   "giant",         SIZE_GIANT,           TRUE    },
    {   "huge",          SIZE_HUGE,            TRUE    },
    {    NULL,	       0,			     0	 }
};


const struct flag_type weapon_class[] =
{
    {   "exotic",		WEAPON_EXOTIC,		TRUE	},
    {   "sword",		WEAPON_SWORD,		TRUE	},
    {   "dagger",		WEAPON_DAGGER,		TRUE	},
    {   "spear",		WEAPON_SPEAR,		TRUE	},
    {   "mace",		WEAPON_MACE,		TRUE	},
    {   "axe",		WEAPON_AXE,			TRUE	},
    {   "flail",		WEAPON_FLAIL,		TRUE	},
    {   "whip",		WEAPON_WHIP,		TRUE	},
    {   "polearm",	WEAPON_POLEARM,		TRUE	},
    {   "bow",		WEAPON_BOW,			TRUE	},
    {   "staff",	WEAPON_STAFF,		TRUE	},
    {   NULL,		0,				0	}
};


const struct flag_type weapon_type2[] =
{
    {   "flaming",		WEAPON_FLAMING,       		TRUE	},
    {   "frost",			WEAPON_FROST,         		TRUE	},
    {   "vampiric",		WEAPON_VAMPIRIC,      		TRUE	},
    {   "sharp",			WEAPON_SHARP,         		TRUE	},
    {   "vorpal",			WEAPON_VORPAL,        		TRUE	},
    {   "twohands",		WEAPON_TWO_HANDS,     		TRUE	},
    {   "shocking",		WEAPON_SHOCKING,      		TRUE	},
    {   "poison",			WEAPON_POISON,			TRUE	},
    {   "brilliant",		WEAPON_BRILLIANT,			TRUE	},
    {   "chaotic",		WEAPON_CHAOTIC,			TRUE	},
    {   "disruption",		WEAPON_DISRUPTION,		TRUE	},
    {   "flamingburst",		WEAPON_FLAMING_BURST,		TRUE	},
    {   "shockingburst",	WEAPON_SHOCKING_BURST,		TRUE	},
    {   "icyburst",		WEAPON_ICY_BURST,			TRUE	},
    {   "holy",			WEAPON_HOLY,			TRUE	},
    {   "unholy",			WEAPON_UNHOLY,			TRUE	},
    {   "lawful",			WEAPON_LAWFUL,			TRUE	},
    {   "mightycleaving",	WEAPON_MIGHTY_CLEAVING,		TRUE	},
    {   "thundering",		WEAPON_THUNDERING,		TRUE	},
    {	  "bstaff",			WEAPON_BSTAFF,			TRUE	},
    {	  "dstaff",			WEAPON_DSTAFF,			TRUE	},
    {   NULL,              	0,                   		0	}
};

const struct flag_type res_flags[] =
{
    {   "summon",	 		RES_SUMMON,			TRUE	},
    {   "charm",         	RES_CHARM,            	TRUE	},
    {   "magic",         	RES_MAGIC,            	TRUE	},
    {   "weapon",       	RES_WEAPON,           	TRUE	},
    {   "bash",          	RES_BASH,             	TRUE	},
    {   "pierce",        	RES_PIERCE,           	TRUE	},
    {   "slash",         	RES_SLASH,            	TRUE	},
    {   "fire",          	RES_FIRE,             	TRUE	},
    {   "cold",          	RES_COLD,             	TRUE	},
    {   "lightning",     	RES_LIGHTNING,        	TRUE	},
    {   "acid",          	RES_ACID,             	TRUE	},
    {   "poison",        	RES_POISON,           	TRUE	},
    {   "negative",      	RES_NEGATIVE,         	TRUE	},
    {   "holy",          	RES_HOLY,             	TRUE	},
    {   "energy",        	RES_ENERGY,           	TRUE	},
    {   "mental",        	RES_MENTAL,           	TRUE	},
    {   "disease",      	RES_DISEASE,          	TRUE	},
    {   "drowning",      	RES_DROWNING,         	TRUE	},
    {   "light",         	RES_LIGHT,            	TRUE	},
    {   "sound",			RES_SOUND,			TRUE	},
    {   "wood",			RES_WOOD,			TRUE	},
    {   "silver",			RES_SILVER,			TRUE	},
    {   "iron",			RES_IRON,			TRUE	},
    {   NULL,          		0,            		0	}
};


const struct flag_type vuln_flags[] =
{
    {   "summon",		VULN_SUMMON,		TRUE	},
    {   "charm",		VULN_CHARM,			TRUE	},
    {   "magic",		VULN_MAGIC,           	TRUE	},
    {   "weapon",		VULN_WEAPON,          	TRUE	},
    {   "bash",		VULN_BASH,            	TRUE	},
    {   "pierce",		VULN_PIERCE,          	TRUE	},
    {   "slash",		VULN_SLASH,           	TRUE	},
    {   "fire",		VULN_FIRE,            	TRUE	},
    {   "cold",		VULN_COLD,            	TRUE	},
    {   "lightning",	VULN_LIGHTNING,       	TRUE	},
    {   "acid",		VULN_ACID,            	TRUE	},
    {   "poison",		VULN_POISON,          	TRUE	},
    {   "negative",	VULN_NEGATIVE,        	TRUE	},
    {   "holy",		VULN_HOLY,            	TRUE	},
    {   "energy",		VULN_ENERGY,          	TRUE	},
    {   "mental",		VULN_MENTAL,          	TRUE	},
    {   "disease",	VULN_DISEASE,         	TRUE	},
    {   "drowning",	VULN_DROWNING,        	TRUE	},
    {   "light",		VULN_LIGHT,           	TRUE	},
    {   "sound",		VULN_SOUND,			TRUE	},
    {   "wood",		VULN_WOOD,            	TRUE	},
    {   "silver",		VULN_SILVER,          	TRUE	},
    {   "iron",		VULN_IRON,            	TRUE	},
    {   NULL,              0,                   0	}
};

const struct flag_type position_flags[] =
{
  {   "dead",		POS_DEAD,            	FALSE   },
  {   "mortal",		POS_MORTAL,          	FALSE   },
  {   "incap",		POS_INCAP,           	FALSE   },
  {   "stunned",	POS_STUNNED,         	FALSE   },
  {   "sleeping",	POS_SLEEPING,        	TRUE    },
  {   "resting",	POS_RESTING,         	TRUE    },
  {   "sitting",	POS_SITTING,         	TRUE    },
  {   "fighting",	POS_FIGHTING,        	FALSE   },
  {   "standing",	POS_STANDING,        	TRUE    },
  {   NULL,             0,                   	0       }
};

const struct flag_type portal_flags[]=
{
  {   "normal_exit",	GATE_NORMAL_EXIT,	TRUE	},
  {   "no_curse",	GATE_NOCURSE,		TRUE	},
  {   "go_with",	GATE_GOWITH,		TRUE	},
  {   "buggy",		GATE_BUGGY,		TRUE	},
  {   "random",		GATE_RANDOM,		TRUE	},
  {   NULL,		0,			0	}
};

const struct flag_type furniture_flags[]=
{
  {	"stand_at",	STAND_AT,	TRUE	},
  {	"stand_on",	STAND_ON,	TRUE	},
  {	"stand_in",	STAND_IN,	TRUE	},
  {	"sit_at",	SIT_AT,		TRUE	},
  {	"sit_on",	SIT_ON,		TRUE	},
  {	"sit_in",	SIT_IN,		TRUE	},
  {	"rest_at",	REST_AT,	TRUE	},
  {	"rest_on",	REST_ON,	TRUE	},
  {	"rest_in",	REST_IN,	TRUE	},
  {	"sleep_at",	SLEEP_AT,	TRUE	},
  {	"sleep_on",	SLEEP_ON,	TRUE	},
  {	"sleep_in",	SLEEP_IN,	TRUE	},
  {	"put_at",	PUT_AT,		FALSE	}, //Don't use it and Don't need it for now.
  {	"put_on",	PUT_ON,		FALSE	}, //Don't use it and Don't need it for now
  {	"put_in",	PUT_IN,		FALSE	}, //Don't use it and Don't need it for now
  {	"put_inside",	PUT_INSIDE,	FALSE	}, //Don't use it and Don't need it for now
  {	NULL,		0,		0	}
};

const	struct	flag_type	apply_types	[]	=
{
  {	"affects",	TO_AFFECTS,	TRUE	},
  {	"object",	TO_OBJECT,	TRUE	},
  {	"immune",	TO_IMMUNE,	TRUE	},
  {	"resist",	TO_RESIST,	TRUE	},
  {	"vuln",		TO_VULN,	TRUE	},
  {	"weapon",	TO_WEAPON,	TRUE	},
  {	NULL,		0,		TRUE	}
};

const	struct	bit_type	bitvector_type	[]	=
{
  {	affect_flags,	"affect"	},
  {	apply_flags,	"apply"		},
  {	imm_flags,	"imm"		},
  {	res_flags,	"res"		},
  {	vuln_flags,	"vuln"		},
  {	weapon_type2,	"weapon"	}
};

const struct extrabits_type  extrabits_table [] =
{
  {"ITEM_GLOW", A, 'A'},
  {"ITEM_HUM", B, 'B'},
  {"ITEM_DARK", C, 'C'},
  {"ITEM_MASTERWORK", D, 'D'},
  {"ITEM_EVIL", E, 'E'},
  {"ITEM_INVIS", F, 'F'},
  {"ITEM_MAGIC", G, 'G'},
  {"ITEM_NODROP", H, 'H'},
  {"ITEM_BLESS", I, 'I'},
  {"ITEM_ANTI_GOOD", J, 'J'},
  {"ITEM_ANTI_EVIL", K, 'K'},
  {"ITEM_ANTI_NEUTRAL", L, 'L'},
  {"ITEM_NOREMOVE", M, 'M'},
  {"ITEM_INVENTORY", N, 'N'},
  {"ITEM_NOPURGE", O, 'O'},
  {"ITEM_ROT_DEATH", P, 'P'},
  {"ITEM_VIS_DEATH", Q, 'Q'},
  {"ITEM_NONMETAL", S, 'S'},
  {"ITEM_NOLOCATE", T, 'T'},
  {"ITEM_MELT_DROP", U, 'U'},
  {"ITEM_HAD_TIMER", V, 'V'},
  {"ITEM_SELL_EXTRACT", W, 'W'},
  {"ITEM_STAINLESS", X, 'X'},
  {"ITEM_BURN_PROOF", Y, 'Y'},
  {"ITEM_NOUNCURSE", Z, 'Z'},
  {NULL, 0, '0'}
};

const struct flag_type oprog_flags[] =
{
    {	"act",			TRIG_ACT,		TRUE	},
    {	"fight",			TRIG_FIGHT,		TRUE	},
    {	"give",			TRIG_GIVE,		TRUE	},
    { "greet",			TRIG_GREET,		TRUE	},
    {	"random",			TRIG_RANDOM,	TRUE	},
    { "speech",			TRIG_SPEECH,	TRUE	},
    {	"exall",			TRIG_EXALL,		TRUE	},
    {	"delay",			TRIG_DELAY,		TRUE	},
    {	"drop",			TRIG_DROP,		TRUE	},
    {	"get",			TRIG_GET,		TRUE	},
    {	"sit",			TRIG_SIT,		TRUE	},
    { "eat",			TRIG_EAT,		TRUE	},
    {	NULL,				0,			TRUE	}
};

const struct flag_type rprog_flags[] =
{
    {	"act",			TRIG_ACT,		TRUE	},
    {	"fight",			TRIG_FIGHT,		TRUE	},
    {	"drop",			TRIG_DROP,		TRUE	},
    {	"greet",			TRIG_GREET,		TRUE	},
    {	"random",			TRIG_RANDOM,	TRUE	},
    {	"speech",			TRIG_SPEECH,	TRUE	},
    {	"exall",			TRIG_EXALL,		TRUE	},
    {	"delay",			TRIG_DELAY,		TRUE	},
    {	NULL,				0,			TRUE	}
};

const struct flag_type armor_flags[] =
{
    {	"leather",		ARMOR_LEATHER,		TRUE	},
    {	"chain",		ARMOR_CHAIN,		TRUE	},
    {	"plate",		ARMOR_PLATE,		TRUE	},
    {	NULL,			0,				0	}
};

const	struct	hometown_type	hometown_table [] =
{
/*	{	"name",		recall,	school,	morgue,	death,	pit_vnum	},	*/
	{	"Palanthas",	3001,		3700,		3036,		3054,		3237	},
	{	"Solanthus",	5737,		3700,		3036,		5737,		5737	},
	{	"Solace",		2659,		3700,		3036,		2659,		2659	},
	{	"Silvanost",	17650,	3700,		3036,		17650,	17651	},
/*	{	"Qualinost",	3001,		3700,		3036,		3054,		3237	},	*/
	{	"Thorbardin",	10664,	3700,		3036,		10664,	10677	},
	{	"Neraka",		11962,	3700,		3036,		11962,	11962	},
	{	"Kendermore",	14920,	3700,		3036,		14920,	14975	},
	{	"Mt Nevermind",	10896,	3700,		3036,		10895,	10896	},
/*	{	"Sanction",		3001,		3700,		3036,		3054,		3237	},	*/
/*	{	"Mithas",		3001,		3700,		3036,		3054,		3237	},	*/
/*	{	"Istar",		5892,		3700,		3036,		5892,		3237	},	*/
	{	NULL,			3001,		3700,		3001,		3001,		3237	}
};

const struct material_known material_list [] =
{
/* { "material" },*/
{ "adamantite" },
{ "air" },
{ "amber" },
{ "amethyst" },
{ "ash" },
{ "banded leather" },
{ "black opal" },
{ "black pearl" },
{ "blood" },
{ "bone" },
{ "brass" },
{ "bronze" },
{ "burlap" },
{ "canvas" },
{ "cedar" },
{ "centaur fur" },
{ "ceramic" },
{ "clay" },
{ "cloth" },
{ "copper" },
{ "coral" },
{ "crystal" },
{ "diamond" },
{ "dragon skin" },
{ "dwarf skin" },
{ "dyed leather" },
{ "fire opal" },
{ "ebony" },
{ "electrum" },
{ "elf skin" },
{ "elm" },
{ "embroidered cloth" },
{ "emerald" },
{ "energy" },
{ "feathers" },
{ "fine parchment" },
{ "flesh" },
{ "freshwater pearl" },
{ "fur" },
{ "garnet" },
{ "gemstone" },
{ "glass" },
{ "gold" },
{ "granite" },
{ "hair" },
{ "hematite" },
{ "hickory" },
{ "homespun" },
{ "human skin" },
{ "ice" },
{ "ink" },
{ "iron" },
{ "ironwood" },
{ "iron tips" },
{ "ivory" },
{ "jade" },
{ "kender skin" },
{ "lace" },
{ "lead" },
{ "leather" },
{ "leopard fur" },
{ "limestone" },
{ "linen" },
{ "magic" },
{ "magical fire" },
{ "magnetite" },
{ "mahogony" },
{ "malachite" },
{ "maple" },
{ "marble" },
{ "mica" },
{ "minotaur fur" },
{ "mithril" },
{ "mother of pearl" },
{ "muslin" },
{ "obsidian" },
{ "oak" },
{ "onyx" },
{ "opal" },
{ "panther fur" },
{ "paper" },
{ "papyrus" },
{ "parchment" },
{ "pearl" },
{ "pewter" },
{ "pine" },
{ "plant" },
{ "platinum" },
{ "quartz" },
{ "redwood" },
{ "rose gold" },
{ "rose quartz" },
{ "rosewood" },
{ "ruby" },
{ "sapphire" },
{ "sandstone" },
{ "shark skin" },
{ "shell" },
{ "silk" },
{ "silver" },
{ "skin" },
{ "slate" },
{ "smoky quartz" },
{ "snake skin" },
{ "soft leather" },
{ "soil" },
{ "steel" },
{ "stone" },
{ "studded leather" },
{ "suede" },
{ "tiger fur" },
{ "tiger skin" },
{ "tin" },
{ "titanium" },
{ "tooled leather" },
{ "topaz" },
{ "tourmaline" },
{ "turquoise" },
{ "unknown" }, //"Unknown" MUST be in the list for compatibility.
{ "vallenwood" },
{ "velium" },
{ "vellum" },
{ "velvet" },
{ "walnut" },
{ "wax" },
{ "white tiger fur" },
{ "white gold" },
{ "willow" },
{ "wood" },
{ "wool" },
{ "zebra fur" },
{ NULL }
};

const   struct  forge_items_type forge_items_table [] =
{	
/*	{ "name", item_type, level, wear_flags, "material", weight, cost,
	   "short_descr", "long_descr",
 		weapon_class, w_v1, w_v2, "dam_noun", ac_pierce, ac_bash, ac_slash, ac_exotic, armor_type, beats },
*/
/*SWORDS*/
	{ "shortsword", ITEM_WEAPON, 45, ITEM_TAKE|ITEM_WIELD, "steel", 2, 450,
	   "a shortsword", "a shortsword lies on the floor.",
		WEAPON_SWORD, 4, 12, "slice", 0, 0, 0, 0, 0, 11 },
	{ "leaf-sword", ITEM_WEAPON, 46, ITEM_TAKE|ITEM_WIELD, "plant", 1, 460,
	   "a leaf-sword", "This leaf-sword looks very light.", 
		WEAPON_SWORD, 4, 12, "slice", 0, 0, 0, 0, 0, 10 },
	{ "gladius", ITEM_WEAPON, 50, ITEM_TAKE|ITEM_WIELD, "steel", 3, 500,
	   "a gladius", "A large gladius stands in the corner.",
		WEAPON_SWORD, 7, 7, "stab", 0, 0, 0, 0, 0, 12 },
	{ "cutlass", ITEM_WEAPON, 50, ITEM_TAKE|ITEM_WIELD, "steel", 2, 500, "a cutlass",
	    "This cutlass looks like it belongs to a pirate.",
		WEAPON_SWORD, 7, 7, "slice", 0, 0, 0, 0, 0, 11 },
	{ "sabre", ITEM_WEAPON, 50, ITEM_TAKE|ITEM_WIELD, "steel", 3, 500,
	   "sabre", "A long sabre sits here.",
		WEAPON_SWORD, 7, 7, "slice", 0, 0, 0, 0, 0, 11 },
	{ "rapier", ITEM_WEAPON, 52, ITEM_TAKE|ITEM_WIELD, "silver", 2, 520,
	   "a small rapier", "A small rapier lies covered in dust.",
		WEAPON_SWORD, 2, 28, "pierce", 0, 0, 0, 0, 0, 10 },
	{ "falchion", ITEM_WEAPON, 55, ITEM_TAKE|ITEM_WIELD, "steel", 4, 550,
	   "a large falchion", "A large falchion lies here, it's blade menacing.",
		WEAPON_SWORD, 7, 8, "slash", 0, 0, 0, 0, 0, 12 },
	{ "longsword", ITEM_WEAPON, 60, ITEM_TAKE|ITEM_WIELD, "steel", 4, 600,
	   "a longsword", "A sheathed longsword lies in the room.",
		WEAPON_SWORD, 6, 10, "slash", 0, 0, 0, 0, 0, 12 },
	{ "backsword", ITEM_WEAPON, 62, ITEM_TAKE|ITEM_WIELD, "steel", 3, 620,
	   "a backsword", "A backsword sits in the far corner.",
		WEAPON_SWORD, 4, 16, "slash", 0, 0, 0, 0, 0, 11 },
	{ "broadsword", ITEM_WEAPON, 65, ITEM_TAKE|ITEM_WIELD, "steel", 4, 650,
	   "a broadsword", "A long, thick broadsword lies here.",
		WEAPON_SWORD, 8, 8, "slash", 0, 0, 0, 0, 0, 13 },
	{ "flamberge", ITEM_WEAPON, 68, ITEM_TAKE|ITEM_WIELD, "steel", 4, 680,
	   "a flamberge", "A two-bladed flamberge sits in the room.",
		WEAPON_SWORD, 5, 14, "slash", 0, 0, 0, 0, 0, 13 },
	{ "bastard sword", ITEM_WEAPON, 75, ITEM_TAKE|ITEM_WIELD, "steel",  5, 4000,
	   "a bastard sword", "A large bastard sword sits here, daunting.",
	      WEAPON_SWORD, 2, 40, "chop", 0, 0, 0, 0, 0, 14 },
	{ "greatsword", ITEM_WEAPON, 80, ITEM_TAKE|ITEM_WIELD, "steel", 6, 5000,
	   "a greatsword", "A large greatsword sits in the corner of the room.",
		WEAPON_SWORD, 3, 28, "smash", 0, 0, 0, 0, 0, 14 },
	{ "warsword", ITEM_WEAPON, 85, ITEM_TAKE|ITEM_WIELD, "steel", 7, 6000,
	   "a warsword", "A giant warsword lies on the floor.",
		WEAPON_SWORD, 4, 22, "cleave", 0, 0, 0, 0, 0, 15 },
	{ "claymore", ITEM_WEAPON, 90, ITEM_TAKE|ITEM_WIELD, "steel", 8, 7000,
	   "a claymore", "a wicked-looking claymore sits here.",
		WEAPON_SWORD, 8, 11, "grep", 0, 0, 0, 0, 0, 15 },
/*DAGGERS*/
	{ "knife", ITEM_WEAPON, 30, ITEM_TAKE|ITEM_WIELD, "steel", 1, 300,
	   "a knife", "A knife lies on the ground.",
		WEAPON_DAGGER, 4, 8, "pierce", 0, 0, 0, 0, 0, 10 },
	{ "cock-spur", ITEM_WEAPON, 30, ITEM_TAKE|ITEM_WIELD, "steel", 2, 300, 
	   "a cock-spur", "A cock-spur lies here.",
		WEAPON_DAGGER, 4, 8, "pierce", 0, 0, 0, 0, 0, 10 },
	{ "stiletto", ITEM_WEAPON, 35, ITEM_TAKE|ITEM_WIELD, "steel", 2, 350,
	   "a stiletto", "A small, thin stiletto sits in the corner.",
		WEAPON_DAGGER, 6, 6, "stab", 0, 0, 0, 0, 0, 11 },
	{ "dagger", ITEM_WEAPON, 40, ITEM_TAKE|ITEM_WIELD, "steel", 1, 400,
	   "a jagged dagger", "A dagger with a jagged blade stands upright in the ground.",
		WEAPON_DAGGER, 2, 22, "pierce", 0, 0, 0, 0, 0, 10 },
	{ "dirk", ITEM_WEAPON, 42, ITEM_TAKE|ITEM_WIELD, "steel", 2, 420,
	   "a small dirk", "A sheathed dirk lies on the floor.",
		WEAPON_DAGGER, 6, 7, "thrust", 0, 0, 0, 0, 0, 12 },
	{ "long knife", ITEM_WEAPON, 45, ITEM_TAKE|ITEM_WIELD, "steel", 3, 450,
	   "a long knife", "A long knife lies sheathed on the floor.",
		WEAPON_DAGGER, 4, 12, "thrust", 0, 0, 0, 0, 0, 12 },
	{ "stone dagger", ITEM_WEAPON, 50, ITEM_TAKE|ITEM_WIELD, "stone", 2, 500,
	   "a stone dagger", "There appears to be a dagger rudimentaly cut from stone.",
		WEAPON_DAGGER, 7, 7, "stab", 0, 0, 0, 0, 0, 15 },
/*SPEARS*/
	{ "spear", ITEM_WEAPON, 54, ITEM_TAKE|ITEM_WIELD, "elm", 4, 540,
	   "a spear", "A spear sits upright in the corner.",
		WEAPON_SPEAR, 6, 9, "pierce", 0, 0, 0, 0, 0, 10 },
	{ "javelin", ITEM_WEAPON, 59, ITEM_TAKE|ITEM_WIELD, "maple", 4, 590,
	   "a long javelin", "This long javelin has a pointy tip.",
		WEAPON_SPEAR, 6, 10, "pierce", 0, 0, 0, 0, 0, 11 },
	{ "lance", ITEM_WEAPON, 65, ITEM_TAKE|ITEM_WIELD, "ironwood", 5, 650,
	   "a lance", "Here lies a short lance with a wooden handle.",
		WEAPON_SPEAR, 8, 8, "pierce", 0, 0, 0, 0, 0, 12 },
/*MACES*/
	{ "warhammer", ITEM_WEAPON, 50, ITEM_TAKE|ITEM_WIELD, "wood", 3, 500,
	   "a warhammer", "a huge warhammer sits on the ground.",
		WEAPON_MACE, 7, 7, "beating", 0, 0, 0, 0, 0, 11 },
	{ "morningstar", ITEM_WEAPON, 50, ITEM_TAKE|ITEM_WIELD, "onyx", 3, 500,
	   "a morningstar", "This huge, spiked morningstar looks menacing.",
		WEAPON_MACE, 7, 7, "pound", 0, 0, 0, 0, 0, 11 },
	{ "battle hammer", ITEM_WEAPON, 50, ITEM_TAKE|ITEM_WIELD, "lead", 3, 500,
	   "a battle hammer", "A large battlehammer sits here, forgotten.",
		WEAPON_MACE, 7, 7, "crush", 0, 0, 0, 0, 0, 11 },
	{ "footmans mace", ITEM_WEAPON, 52, ITEM_TAKE|ITEM_WIELD, "steel", 2, 520,
	   "a footmans mace", "This mace looks like it once belonged to a footman.",
		WEAPON_MACE, 2, 28, "thwack", 0, 0, 0, 0, 0, 10 },
	{ "horsemans mace", ITEM_WEAPON, 60, ITEM_TAKE|ITEM_WIELD, "wood", 4, 600,
	   "a horseman's mace", "This mace looks like it would belong to a horseman.",
	      WEAPON_MACE, 6, 10, "pound", 0, 0, 0, 0, 0, 10 },
/*AXES*/
	{ "hand axe", ITEM_WEAPON, 52, ITEM_TAKE|ITEM_WIELD, "steel", 2, 520,
	   "a hand axe", "a small hand axe sits here.",
		WEAPON_AXE, 2, 28, "chop", 0, 0, 0, 0, 0, 10 },
	{ "hatchet", ITEM_WEAPON, 54, ITEM_TAKE|ITEM_WIELD, "stone", 1, 540,
	   "a hatchet", "This small hatchet looks like it belongs to a farmer.",
		WEAPON_AXE, 6, 9, "chop", 0, 0, 0, 0, 0, 10 },
	{ "francisca axe", ITEM_WEAPON, 60, ITEM_TAKE|ITEM_WIELD, "iron", 2, 600,
         "a small axe called 'francisca'", "This small axe has the name 'Francisca' engraved on it's head.",
		WEAPON_AXE, 6, 10, "cleave", 0, 0, 0, 0, 0, 10 },
	{ "battleaxe", ITEM_WEAPON, 65, ITEM_TAKE|ITEM_WIELD, "steel", 3, 650,
	   "a battleaxe", "There is a sharp battleaxe lying on the ground.",
		WEAPON_AXE, 8, 8, "cleave", 0, 0, 0, 0, 0, 11 },
	{ "broad axe", ITEM_WEAPON, 70, ITEM_TAKE|ITEM_WIELD, "steel", 4, 700,
	   "a broad-axe", "An axe with a broad head lies on the ground.",
		WEAPON_AXE, 7, 10, "cleave", 0, 0, 0, 0, 0, 11 },
	{ "war axe", ITEM_WEAPON, 75, ITEM_TAKE|ITEM_WIELD, "steel", 6, 4000,
	   "a war axe", "A huge axe which looks like it's seen war lies here.",
		WEAPON_AXE, 2, 40, "grep", 0, 0, 0, 0, 0, 12 },
/*FLAILS*/
	{ "heavy flail", ITEM_WEAPON, 75, ITEM_TAKE|ITEM_WIELD, "tooled leather", 10, 4000,
	   "a heavy flail", "A heavy-looking flail sits here, sporting a razor-sharp end.",
		WEAPON_FLAIL, 2, 40, "bite", 0, 0, 0, 0, 0, 13 },
/*WHIPS*/
	{ "torture whip", ITEM_WEAPON, 55, ITEM_TAKE|ITEM_WIELD, "leather", 7, 550,
	   "a torture whip", "This whip looks excellent for use in torture.",
		WEAPON_WHIP, 7, 8, "whip", 0, 0, 0, 0, 0, 9 },
/*POLEARMS*/
	{ "dragonlance", ITEM_WEAPON, 120, ITEM_TAKE|ITEM_WIELD, "silver", 20, 30000,
	   "a dragonlance", "This is one of the legendary Silver DragonLances!",
		WEAPON_POLEARM, 19, 6, "thrust", 0, 0, 0, 0, 0, 23 },
/*BOWS*/
	{ "dark bow", ITEM_WEAPON, 85, ITEM_TAKE|ITEM_WIELD, "redwood", 5, 6500,
	   "a dark bow", "Here lies a bow, made from a dark red Redwood.",
		WEAPON_BOW, 4, 22, "thwack", 0, 0, 0, 0, 0, 15 },
/*STAFFS*/
	{ "quarterstaff", ITEM_WEAPON, 60, ITEM_TAKE|ITEM_WIELD, "ash", 3, 600,
	   "a quarterstaff", "A quarterstaff carved from Ash wood stands against a wall.",
		WEAPON_STAFF, 6, 10, "smash", 0, 0, 0, 0, 0, 11 },
/*ARMOR*/
/*WEAR_NECK*/
	{ "medallion", ITEM_ARMOR, 55, ITEM_TAKE|ITEM_WEAR_NECK, "gold", 1, 550,
	   "a medallion", "A mediallion of the stars lies here, shining.",
		0, 0, 0, NULL, 12, 12, 12, 8, 0, 8 },
	{ "amulet", ITEM_ARMOR, 60, ITEM_TAKE|ITEM_WEAR_NECK, "silver", 1, 600,
	   "an amulet", "Here lies a small silver amulet, dangling from it's chain.",
		0, 0, 0, NULL, 13, 13, 13, 9, 0, 7 },
/*WEAR_BODY*/
 	{ "scale tunic", ITEM_ARMOR, 95, ITEM_TAKE|ITEM_WEAR_BODY, "steel", 8, 8000,
	   "a scale tunic", "A tunic made of interlacing scale lies here.",
		0, 0, 0, NULL, 20, 20, 20, 16, ARMOR_PLATE, 14 },
	{ "chainmail", ITEM_ARMOR, 116, ITEM_TAKE|ITEM_WEAR_BODY, "iron", 10, 9000,
	   "a suit of chainmail", "A suit of chainmail sits on the ground.",
		0, 0, 0, NULL, 24, 24, 24, 20, ARMOR_CHAIN, 16 },
	{ "scalemail", ITEM_ARMOR, 117, ITEM_TAKE|ITEM_WEAR_BODY,"copper", 10, 10000,
	   "a suit of scalemail", "Here lies a suit of armor made of scale design.",
		0, 0, 0, NULL, 24, 24, 24, 20, 0, 17 },
	{ "platemail", ITEM_ARMOR, 118, ITEM_TAKE|ITEM_WEAR_BODY, "steel", 13, 11000,
	   "a suit of platemail", "A heavy-looking suit of platemail sits here.",
		0, 0, 0, NULL, 24, 24, 24, 21, ARMOR_PLATE, 18 },
	{ "full platemail", ITEM_ARMOR, 120, ITEM_TAKE|ITEM_WEAR_BODY, "steel", 15, 20000,
	   "a full suit of platemail", "This suit of full platemail sits upright",
		0, 0, 0, NULL, 26, 26, 26, 22, ARMOR_PLATE, 20 },
/*WEAR_HEAD*/
	{ "chain headpiece", ITEM_ARMOR, 73, ITEM_TAKE|ITEM_WEAR_HEAD, "steel", 4, 3000,
	   "a chain headpiece", "Here lies a headpiece made with interlacing chains.",
		0, 0, 0, NULL, 15, 15, 15, 11, ARMOR_CHAIN, 13 },
	{ "iron skullcap", ITEM_ARMOR, 80, ITEM_TAKE|ITEM_WEAR_HEAD, "iron", 9, 5000,
	   "an iron skullcap", "A large and heavy-looking iron skullcap lies upside down.",
		0, 0, 0, NULL, 17, 17, 17, 13, 0, 15 },
	{ "tiara", ITEM_ARMOR, 110, ITEM_TAKE|ITEM_WEAR_HEAD, "crystal", 4, 18000,
	   "a woman's tiara", "A lovely tiara sits daintily on the ground.",
		0, 0, 0, NULL, 23, 23, 23, 19, 0, 17 },
/*WEAR_LEGS*/
	{ "golden greaves", ITEM_ARMOR, 30, ITEM_TAKE|ITEM_WEAR_LEGS, "gold", 3, 300,
	   "golden greaves", "A pair of golden greaves shimmers in the light.",
		0, 0, 0, NULL, 7, 7, 7, 3, 0, 11 },
/*WEAR_FEET*/
	{ "leather boots", ITEM_ARMOR, 55, ITEM_TAKE|ITEM_WEAR_FEET, "tooled leather", 3, 550,
	   "leather boots", "These pair of Tooled Leather boots seem comfy.",
		0, 0, 0, NULL, 11, 11, 11, 7, ARMOR_LEATHER, 12 },
/*WEAR_HANDS*/
	{ "homespun gloves", ITEM_ARMOR, 40, ITEM_TAKE|ITEM_WEAR_HANDS, "homespun", 1, 400,
	   "homespun gloves", "Here lies a pair of carefully homespun gloves.",
		0, 0, 0, NULL, 9, 9, 9, 5, 0, 10 },
/*WEAR_ARMS*/
	{ "armband", ITEM_ARMOR, 55, ITEM_TAKE|ITEM_WEAR_ARMS, "wool", 1, 550,
	   "an armband", "A pair of loose armbands lie unused.",
		0, 0, 0, NULL, 12, 12, 12, 8, 0, 10 },
/*WEAR_SHIELD*/
	{ "mahogony buckler", ITEM_ARMOR, 70, ITEM_TAKE|ITEM_WEAR_SHIELD, "mahogony", 3, 700,
	   "a mahogony buckler", "A buckler made of fine mahogony lies here.",
		0, 0, 0, NULL, 15, 15, 15, 11, 0, 15 },
/*WEAR_ABOUT*/
	{ "cloak", ITEM_ARMOR, 60, ITEM_TAKE|ITEM_WEAR_ABOUT, "bone", 4, 600,
	   "a bone cloak", "A cloak made of bones woven together is strewn in the corner.",
		0, 0, 0, NULL, 13, 13, 13, 9, 0, 17 },
/*WEAR_WAIST*/
	{ "buckle", ITEM_ARMOR, 92, ITEM_TAKE|ITEM_WEAR_WAIST, "silver", 2, 7500,
	   "a buckle", "A shiny buckle sits on the ground",
		0, 0, 0, NULL, 19, 19, 19, 15, 0, 7 },
	{ "girdle", ITEM_ARMOR, 100, ITEM_TAKE|ITEM_WEAR_WAIST, "leather", 3, 9000,
	   "a girdle", "A tight leather girdle lies here, forgotten.",
		0, 0, 0, NULL, 21, 21, 21, 17, ARMOR_LEATHER, 9 },
/*WEAR_WRIST*/
	{ "cufflink", ITEM_ARMOR, 75, ITEM_TAKE|ITEM_WEAR_WRIST, "jade", 1, 4000,
	   "a cufflink", "This cufflink looks like it would go good with your shirt.",
		0, 0, 0, NULL, 16, 16, 16, 12, 0, 7 },
/*WEAR_SHOULDERS*/
	{ "brass pauldron", ITEM_ARMOR, 55, ITEM_TAKE|ITEM_WEAR_SHOULDERS, "brass", 2, 950,
	   "a set of brass pauldrons", "A set of brass pauldrons sit here, waiting for use.",
		0, 0, 0, NULL, 12, 12, 12, 8, 0, 9 },
/*WEAR_ANKLE*/
	{ "spurs", ITEM_ARMOR, 60, ITEM_TAKE|ITEM_WEAR_ANKLE, "iron", 1, 800,
	   "a set of spurs", "A set of spurs sits here, awaiting to be attatched to your boots.",
		0, 0, 0, NULL, 13, 13, 13, 9, 0, 10 },
/*WEAR_FACE*/
	{ "leather visor", ITEM_ARMOR, 60, ITEM_TAKE|ITEM_WEAR_FACE, "leather", 1, 600,
	   "a leather visor", "A simple but efficient leather visor rests here.",
		0, 0, 0, NULL, 13, 13, 13, 9, 0, 10 },
	{ "mask", ITEM_ARMOR, 118, ITEM_TAKE|ITEM_WEAR_FACE, "wood", 4, 20000,
	   "a wooden mask", "An exquisite wooden mask sits on the floor.",
		0, 0, 0, NULL, 24, 24, 24, 20, 0, 18 },
/*JEWELRY (by popular demand)*/
/*WEAR_FINGER*/
	{ "wedding ring", ITEM_JEWELRY, 20, ITEM_TAKE|ITEM_WEAR_FINGER, "diamond", 1, 2000,
	   "a diamond wedding ring", "Look! Someone dropped their diamond wedding ring.",
		0, 0, 0, NULL, 0, 0, 0, 0, 0, 6 },
/*WEAR_NECK*/
	{ "necklace", ITEM_JEWELRY, 20, ITEM_TAKE|ITEM_WEAR_NECK, "gold", 1, 1500,
	   "a golden necklace", "A 24-Karat Gold necklace lays on the ground.",
		0, 0, 0, NULL, 0, 0, 0, 0, 0, 7 },

	{ NULL, 0, 0, 0,NULL, 0, 0,
	   NULL, NULL,
		0, 0, 0, NULL, 0, 0, 0, 0, 0, 0 },
};
