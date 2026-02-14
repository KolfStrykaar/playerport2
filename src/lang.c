
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "lang.h"


const struct lang_type lang_table[MAX_LANG] =
{
      { "common"      	}, /*0*/
      { "elven"       	}, /*1*/
	{ "drow"		}, /*2*/
      { "dwarven"    	}, /*3*/
      { "ogre"    	}, /*4*/
      { "kender"    	}, /*5*/
      { "minotaur"      }, /*6*/
	{ "silvanesti"	}, /*7*/
	{ "qualinesti"	}, /*8*/
	{ "kagonesti"	}, /*9*/
	{ "dargonesti"	}, /*10*/
	{ "dimernesti"	}, /*11*/
	{ "draconian"	}, /*12*/
	{ "solamnic"	}, /*13*/
	{ "thieves cant"  }  /*14*/
};

/*
        Replacement chars per each language
*/
const struct lang_chars_type lang_chars[MAX_LANG][350] =
{
    /* COMMON */
    {
        { "", "" }
    },

    /* ELVEN */
    {
     { "a", "ae" }, { "b", "'gil" }, { "c", "k" }, { "d", "d" },
     { "e", "eia" }, { "f", "fira'" }, { "g", "elin'" }, { "h", "h" },
     { "i", "ie" }, { "j", "y" }, { "k", "kria" }, { "l", "lia" },
     { "m", "mir'" }, { "n", "'nion" }, { "o", "u" }, { "p", "s" },
     { "q", "kir" }, { "r", "k" }, { "s", "sol'" }, { "t", "til'" },
     { "u", "a" }, { "v", "var" }, { "w", "s" }, { "x", "n" },
     { "y", "l" }, { "z", "s" },
      { "1", "1" }, { "2", "2" }, { "3", "3" }, { "4", "4" },
      { "5", "5" }, { "6", "6" }, { "7", "7" }, { "8", "8" },
      { "9", "9" }, { "0", "0" },
      { ",", "," }, { ".", "." }, { "!", "!" }, { "'", "'" },
      { "_", "_" }, { ";", ";" }, { ":", ":" }, { "=", "=" },
      { "\"", "\"" }, { ")", ")" }, { "(", "(" },
      { "*", "*" }, { "&", "&" }, { "^", "^" }, { "%", "%" },
      { "$", "$" }, { "#", "#" }, { "<", "<" }, 
      { "-", "-" },
     { "weapon", "darrellion" }, { "battle", "abirronian" },
     { "treasure", "thres'serion" }, { "friend", "mellion" },
     { "magician", "la'eerban" }, { "magical", "la'arn" },
     { "magic", "la'ar" }, { "strengh", "couriniel" },
     { "weak", "darsyiotha" }, { "be", "sil" }, { "they", "hara" },
     { "he",  "il" },  { "his", "ils" }, { "she", "lia" },
     { "hers", "lias" },  { "him", "ile" }, { "her", "lei" },
     { "the",  "la" }, { "and", "ai" }, { "father", "iltanils" },
     { "mother", "lianinia" },  { "dwarf", "morinif" },
     { "deurgar", "morinif'sha" }, { "warrior", "abirronis" }, 
     { "elf", "ellev'nia" }, { "elves", "ellev'niaie" },
     { "", "" }
    },

    /* DROW */
    {
     { "a", "aex" }, { "b", "gilx" }, { "c", "x" }, { "d", "dir'" },
     { "e", "xeia" }, { "f", "firax'" }, { "g", "xelin'" }, { "h", "h" },
     { "i", "ie" }, { "j", "y" }, { "k", "kria" }, { "l", "lia" },
     { "m", "mir'zia'" }, { "n", "''ix'nion" }, { "o", "u" }, { "p", "s" },
     { "q", "kir" }, { "r", "k" }, { "s", "s" }, { "t", "til'" },
     { "u", "a" }, { "v", "var" }, { "w", "s" }, { "x", "x" },
     { "y", "y" }, { "z", "z" },
      { "1", "1" }, { "2", "2" }, { "3", "3" }, { "4", "4" },
      { "5", "5" }, { "6", "6" }, { "7", "7" }, { "8", "8" },
      { "9", "9" }, { "0", "0" },
      { ",", "," }, { ".", "." }, { "!", "!" }, { "'", "'" },
      { "_", "_" }, { ";", ";" }, { ":", ":" }, { "=", "=" },
      { "\"", "\"" }, { ")", ")" }, { "(", "(" },
      { "*", "*" }, { "&", "&" }, { "^", "^" }, { "%", "%" },
      { "$", "$" }, { "#", "#" }, { "<", "<" }, 
      { "-", "-" },
     { "weapon", "darrellion'ix" }, { "battle", "zi'abirronian" },
     { "treasure", "thres'serion'ix" }, { "enemy", "zahadir'ix" },
     { "magician", "za'eerban" }, { "magical", "za'arn" },
     { "magic", "za'ar" }, { "strengh", "zouriniel" },
     { "weak", "darsyiothax" }, { "be", "sil'x" }, { "they", "xara" },
     { "he",  "il" },  { "his", "ils" }, { "she", "lia" },
     { "hers", "xias" },  { "him", "xile" }, { "her", "xei" },
     { "the",  "xhai" }, { "and", "x'ai" }, { "father", "iltanils" },
     { "mother", "xianinia" },  { "dwarf", "xai'morinif" },
     { "deurgar", "morinif'sha" }, { "warrior", "xabirronis" }, 
     { "elf", "ellez'nix" }, { "elves", "ellez'niaiex" },
	{ "", "" }
    },

    /* DWARVISH */
    {
      { "a", "kha-" }, { "b", "bha-" }, { "c", "q" }, { "d", "dhad-" },
      { "e", "e" }, { "f", "fha-" }, { "g", "gha-" }, { "h", "h" },
      { "i", "u" }, { "j", "y" }, { "k", "kh" }, { "l", "r" },
      { "m", "mher-" }, { "n", "i" }, { "o", "a" }, { "p", "s" },
      { "q", "d" }, { "r", "k" }, { "s", "r" }, { "t", "h" },
      { "u", "u" }, { "v", "vh" }, { "w", "wr" }, { "x", "n" },
      { "y", "l" }, { "z", "zha-" },
      { "1", "1" }, { "2", "2" }, { "3", "3" }, { "4", "4" },
      { "5", "5" }, { "6", "6" }, { "7", "7" }, { "8", "8" },
      { "9", "9" }, { "0", "0" },
      { ",", "," }, { ".", "." }, { "!", "!" }, { "'", "'" },
      { "_", "_" }, { ";", ";" }, { ":", ":" }, { "=", "=" },
      { "\"", "\"" }, { ")", ")" }, { "(", "(" },
      { "*", "*" }, { "&", "&" }, { "^", "^" }, { "%", "%" },
      { "$", "$" }, { "#", "#" }, { "<", "<" }, 
      { "-", "-" },
      { "weapon", "dhar-ak" }, { "battle", "bolta-rha" },
      { "treasure", "thres-sha" }, { "friend", "merro-nhin" },
      { "magician", "fa-eerban" }, { "magical", "fa-arn" },
      { "magic", "fa-ar" }, { "strength", "alt-run" },
      { "weak", "deur" }, { "be", "gah" }, { "they", "har" },
      { "he",  "tor" },  { "his", "tors" }, { "she", "kal" },
      { "hers", "kals" },  { "him", "til" }, { "her", "kil" },
      { "the",  "ra" }, { "and", "erh" }, { "father", "torm" },
      { "mother", "kahalla" },  { "Hiddukel", "Hitax the Flaw" },
      { "deurgar", "duer-gah-har" }, { "warrior", "jha-ra-ghra" }, 
      { "", "" }
    },

    /* OGRE */
    {
     { "a", "-ud" }, { "b", "bu" }, { "c", "k" }, { "d", "d" },
     { "e", "ed" }, { "f", "gu'" }, { "g", "f'" }, { "h", "du-" },
     { "i", "u" }, { "j", "l" }, { "k", "k" }, { "l", "n" },
     { "m", "r'" }, { "n", "'l" }, { "o", "u" }, { "p", "d" },
     { "q", "k" }, { "r", "rh" }, { "s", "bud-" }, { "t", "d'" },
     { "u", "a" }, { "v", "var" }, { "w", "s" }, { "x", "z" },
     { "y", "l" }, { "z", "z" },
        { "1", "1" }, { "2", "2" }, { "3", "3" }, { "4", "4" },
        { "5", "5" }, { "6", "6" }, { "7", "7" }, { "8", "8" },
        { "9", "9" }, { "0", "0" },
        { ",", "," }, { ".", "." }, { "!", "!" }, { "'", "'" },
        { "_", "_" }, { ";", ";" }, { ":", ":" }, { "=", "=" },
        { "\"", "\"" }, { ")", ")" }, { "(", "(" },
        { "*", "*" }, { "&", "&" }, { "^", "^" }, { "%", "%" },
        { "$", "$" }, { "#", "#" }, { "<", "<" }, 
        { "-", "-" },
     { "weapon", "thud-ud" }, { "battle", "batterud" },
     { "treasure", "hud-ruarud" }, { "friend", "gurh-ud" },
     { "magician", "rhagerud" }, { "magical", "ragerud-ud" },
     { "magic", "ragerd" }, { "strengh", "rhorud" },
     { "weak", "piffle-ud" }, { "be", "id" }, { "they", "borud" },
     { "he",  "il" },  { "his", "ils" }, { "she", "lia" },
     { "hers", "hru-dud" },  { "him", "hru" }, { "her", "hrud" },
     { "the",  "ud" }, { "and", "erh" }, { "father", "berga-hru" },
     { "mother", "berga-hrud" },  { "dwarf", "dorad" },
     { "deurgar", "merda-dorad" }, { "warrior", "horud" }, 
     { "giant", "hruderad" }, { "Hiddukel", "M'Fistos" },
	{ "", "" }
    },

    /* KENDER */
    {
     { "a", "ae" }, { "b", "b" }, { "c", "d" }, { "d", "c" },
     { "e", "itter" }, { "f", "g'" }, { "g", "f'" }, { "h", "l" },
     { "i", "atter" }, { "j", "l" }, { "k", "k" }, { "l", "n" },
     { "m", "r'" }, { "n", "'l" }, { "o", "utter" }, { "p", "d" },
     { "q", "q" }, { "r", "k" }, { "s", "sirit" }, { "t", "d'" },
     { "u", "otter" }, { "v", "var" }, { "w", "s" }, { "x", "z" },
     { "y", "ickity" }, { "z", "z" },
        { "1", "1" }, { "2", "2" }, { "3", "3" }, { "4", "4" },
        { "5", "5" }, { "6", "6" }, { "7", "7" }, { "8", "8" },
        { "9", "9" }, { "0", "0" },
        { ",", "," }, { ".", "." }, { "!", "!" }, { "'", "'" },
        { "_", "_" }, { ";", ";" }, { ":", ":" }, { "=", "=" },
        { "\"", "\"" }, { ")", ")" }, { "(", "(" },
        { "*", "*" }, { "&", "&" }, { "^", "^" }, { "%", "%" },
        { "$", "$" }, { "#", "#" }, { "<", "<" }, 
        { "-", "-" },
     { "weapon", "miskeratter" }, { "battle", "perickity" },
     { "treasure", "moneriffity" }, { "friend", "gippy" },
     { "magician", "sparkity" }, { "magical", "sparkality" },
     { "magic", "spark" }, { "strengh", "yipperif" },
     { "weak", "pittelit" }, { "be", "nify" }, { "they", "birry" },
     { "he",  "eky" },  { "his", "ekyi" }, { "she", "oky" },
     { "hers", "okyi" },  { "him", "ek" }, { "her", "ok" },
     { "the",  "yittery" }, { "and", "chittera" },
     { "father", "ekyya" },
     { "mother", "ikyya" },  { "kender", "dippity" },
     { "halfing", "ittybittyit" }, { "thief", "siparkle" }, 
     { "quickling", "chitterialit" }, { "gnome", "tipperio" },
	{ "", "" }
    },

    /* MINOTAUR */
    {
     { "a", "-udk" }, { "b", "bhuk" }, { "c", "k" }, { "d", "d" },
     { "e", "ed" }, { "f", "gu'" }, { "g", "f'" }, { "h", "du-" },
     { "i", "u" }, { "j", "l" }, { "k", "k" }, { "l", "n" },
     { "m", "rhan-'" }, { "n", "'l" }, { "o", "u" }, { "p", "d" },
     { "q", "k" }, { "r", "rh" }, { "s", "mhan" }, { "t", "dhuth'" },
     { "u", "a" }, { "v", "var" }, { "w", "s" }, { "x", "z" },
     { "y", "yhon" }, { "z", "z" },
     { "1", "1" }, { "2", "2" }, { "3", "3" }, { "4", "4" },
     { "5", "5" }, { "6", "6" }, { "7", "7" }, { "8", "8" },
     { "9", "9" }, { "0", "0" },
     { ",", "," }, { ".", "." }, { "!", "!" }, { "'", "'" },
     { "_", "_" }, { ";", ";" }, { ":", ":" }, { "=", "=" },
     { "\"", "\"" }, { ")", ")" }, { "(", "(" },
     { "*", "*" }, { "&", "&" }, { "^", "^" }, { "%", "%" },
     { "$", "$" }, { "#", "#" }, { "<", "<" }, 
     { "-", "-" },
     { "weapon", "thud-tor" }, { "battle", "mhantorak" },
     { "treasure", "kharantor" }, { "friend", "moragtor" },
     { "magician", "mahagerud" }, { "magical", "mahagerud-ud" },
     { "magic", "mahagerd" }, { "strengh", "mhorud" },
     { "weak", "piffle-ud" }, { "be", "id" }, { "they", "borud" },
     { "he",  "aegor" },  { "his", "aeg" }, { "she", "augor" },
     { "hers", "augore" },  { "him", "aega" }, { "her", "augora" },
     { "the",  "mhan" }, { "and", "mher" }, { "father", "aegoratha" },
     { "mother", "minolo" },  { "dwarf", "dhorad" },
     { "minotaur", "mhantorakar" }, { "warrior", "mhara-tuk" }, 
     { "giant", "hruderad" }, { "elves", "mhadderberud" },
  	{ "", "" }
    },

    /* SILVANESTI */
    {
     { "a", "ae" }, { "b", "'ril" }, { "c", "k" }, { "d", "d" },
     { "e", "eia" }, { "f", "fir'" }, { "g", "gin'" }, { "h", "h" },
     { "i", "ie" }, { "j", "y" }, { "k", "kia" }, { "l", "li" },
     { "m", "mi'" }, { "n", "'ni" }, { "o", "u" }, { "p", "s" },
     { "q", "ki" }, { "r", "k" }, { "s", "sil'" }, { "t", "te'" },
     { "u", "a" }, { "v", "var" }, { "w", "s" }, { "x", "n" },
     { "y", "l" }, { "z", "s" },
      { "1", "1" }, { "2", "2" }, { "3", "3" }, { "4", "4" },
      { "5", "5" }, { "6", "6" }, { "7", "7" }, { "8", "8" },
      { "9", "9" }, { "0", "0" },
      { ",", "," }, { ".", "." }, { "!", "!" }, { "'", "'" },
      { "_", "_" }, { ";", ";" }, { ":", ":" }, { "=", "=" },
      { "\"", "\"" }, { ")", ")" }, { "(", "(" },
      { "*", "*" }, { "&", "&" }, { "^", "^" }, { "%", "%" },
      { "$", "$" }, { "#", "#" }, { "<", "<" }, 
      { "-", "-" },
     { "Thera", "Gaia" }, { "Takhisis", "Drakanna" },
     { "Hiddukel", "Betrayer" }, { "Shinare", "Shillara" },
     { "Paladine", "E'li" }, { "Mishakal", "Quenesti Pah" },
     { "Branchala", "Astarin" }, { "Solinari", "Beacon" },
     { "Nuitari", "Nil'rean" }, { "Chemosh", "Aeleth" },
     { "Airius", "Drakan" }, { "magic", "ilde" },
     { "might", "drauul" }, { "arm", "gurrt" },
     { "strength", "grauul" }, { "attack", "daah" },
     { "dragon", "drakon" }, { "murder", "leucth" },
     { "sword", "lierakla" }, { "horny", "leanna" },
     { "thief", "lecutta" }, { "gnoll", "lutteah" },
     { "blade", "lierakla" }, { "leg", "lauck" },
     { "attractive", "shulamushula" }, { "slime", "sleta" },
     { "soft", "suvvt" }, { "sexy", "shulamushuli" },
     { "cleric", "shanan" }, { "head", "serreck" },
     { "war", "sutta" }, { "oompa", "sex" },
     { "have", "kallak" }, { "dung", "karnaloghtch" },
     { "damn", "kkuleh" }, { "tribe", "kullek" },
     { "take", "kallik" }, { "group", "kullok" },
     { "give", "khillosh" }, { "lust", "kellatat" },
     { "weapon", "tsittsa" }, { "eat", "chaach" },
     { "fool", "tchakl" }, { "that", "ta" },
     { "victory", "chaaug" }, { "gnome", "chaa" },
     { "me", "ra" }, { "will", "uh" },
     { "battle", "ralak" }, { "good", "rauuht" },
     { "joy", "nugnukkaah" }, { "no", "nug" },
     { "want", "aark" }, { "curse", "makkad" },
  	{ "", "" }
    },

    /* QUALINESTI */
    {
     { "a", "ai" }, { "b", "'ril" }, { "c", "k" }, { "d", "de" },
     { "e", "eea" }, { "f", "fi'rn" }, { "g", "gin'" }, { "h", "h'" },
     { "i", "ie" }, { "j", "y" }, { "k", "kia" }, { "l", "lilu" },
     { "m", "mi'" }, { "n", "'ni" }, { "o", "uu" }, { "p", "s" },
     { "q", "ki" }, { "r", "k" }, { "s", "sil'" }, { "t", "tei'" },
     { "u", "a" }, { "v", "var" }, { "w", "s" }, { "x", "n" },
     { "y", "lye" }, { "z", "s'c" },
      { "1", "1" }, { "2", "2" }, { "3", "3" }, { "4", "4" },
      { "5", "5" }, { "6", "6" }, { "7", "7" }, { "8", "8" },
      { "9", "9" }, { "0", "0" },
      { ",", "," }, { ".", "." }, { "!", "!" }, { "'", "'" },
      { "_", "_" }, { ";", ";" }, { ":", ":" }, { "=", "=" },
      { "\"", "\"" }, { ")", ")" }, { "(", "(" },
      { "*", "*" }, { "&", "&" }, { "^", "^" }, { "%", "%" },
      { "$", "$" }, { "#", "#" }, { "<", "<" }, 
      { "-", "-" },
     { "Thera", "Gaia" }, { "Takhisis", "Drakanna" },
     { "Hiddukel", "Betrayer" }, { "Shinare", "Shillara" },
     { "Paladine", "E'li" }, { "Mishakal", "Quenesti Pah" },
     { "Branchala", "Astarin" }, { "Solinari", "Beacon" },
     { "Nuitari", "Nil'rean" }, { "Chemosh", "Aeleth" },
     { "Airius", "Drakan" }, { "magic", "ilde" },
     { "might", "drauul" }, { "arm", "gurrt" },
     { "strength", "grauul" }, { "attack", "daah" },
     { "dragon", "drakon" }, { "murder", "leucth" },
     { "sword", "lierakla" }, { "horny", "leanna" },
     { "thief", "lecutta" }, { "gnoll", "lutteah" },
     { "blade", "lierakla" }, { "leg", "lauck" },
     { "attractive", "shulamushula" }, { "slime", "sleta" },
     { "mage", "errma" }, { "sexy", "shulamushuli" },
     { "cleric", "shanan" }, { "head", "serreck" },
     { "war", "sutta" }, { "oompa", "sex" },
     { "have", "kallak" }, { "dung", "karnaloghtch" },
     { "damn", "kkuleh" }, { "tribe", "kullek" },
     { "take", "kallik" }, { "group", "kullok" },
     { "give", "khillosh" }, { "lust", "kellatat" },
     { "weapon", "tsittsa" }, { "eat", "chaach" },
     { "fool", "tchakl" }, { "that", "ta" },
     { "victory", "chaaug" }, { "gnome", "chaa" },
     { "me", "ra" }, { "will", "uh" },
     { "battle", "ralak" }, { "good", "rauuht" },
     { "joy", "nugnukkaah" }, { "no", "nug" },
     { "want", "aark" }, { "curse", "makkad" },
  	{ "", "" }
    },

    /* KAGONESTI */
    {
     { "a", "a'a" }, { "b", "'ril" }, { "c", "ka" }, { "d", "di" },
     { "e", "e'e" }, { "f", "fe" }, { "g", "gin'" }, { "h", "eh" },
     { "i", "ei" }, { "j", "yu" }, { "k", "kia" }, { "l", "li" },
     { "m", "mi'" }, { "n", "'ni" }, { "o", "h'o" }, { "p", "pa" },
     { "q", "ki" }, { "r", "k" }, { "s", "sil'" }, { "t", "ti" },
     { "u", "a" }, { "v", "vi" }, { "w", "s" }, { "x", "n" },
     { "y", "l" }, { "z", "z" },
      { "1", "1" }, { "2", "2" }, { "3", "3" }, { "4", "4" },
      { "5", "5" }, { "6", "6" }, { "7", "7" }, { "8", "8" },
      { "9", "9" }, { "0", "0" },
      { ",", "," }, { ".", "." }, { "!", "!" }, { "'", "'" },
      { "_", "_" }, { ";", ";" }, { ":", ":" }, { "=", "=" },
      { "\"", "\"" }, { ")", ")" }, { "(", "(" },
      { "*", "*" }, { "&", "&" }, { "^", "^" }, { "%", "%" },
      { "$", "$" }, { "#", "#" }, { "<", "<" }, 
      { "-", "-" },
     { "Thera", "Gaia" }, { "Takhisis", "Drakanna" },
     { "Hiddukel", "Betrayer" }, { "Shinare", "Shillara" },
     { "Paladine", "E'li" }, { "Mishakal", "Quenesti Pah" },
     { "Branchala", "Astarin" }, { "Solinari", "Beacon" },
     { "Nuitari", "Nil'rean" }, { "Chemosh", "Aeleth" },
     { "Airius", "Drakan" }, { "magic", "ilde" },
     { "might", "drauul" }, { "arm", "gurrt" },
     { "strength", "grauul" }, { "attack", "daah" },
     { "dragon", "drakon" }, { "murder", "leucth" },
     { "sword", "lierakla" }, { "horny", "leanna" },
     { "thief", "lecutta" }, { "gnoll", "lutteah" },
     { "blade", "lierakla" }, { "leg", "lauck" },
     { "attractive", "shulamushula" }, { "slime", "sleta" },
     { "soft", "suvvt" }, { "sexy", "shulamushuli" },
     { "cleric", "shanan" }, { "head", "serreck" },
     { "war", "sutta" }, { "oompa", "sex" },
     { "have", "kallak" }, { "dung", "karnaloghtch" },
     { "damn", "kkuleh" }, { "tribe", "kullek" },
     { "take", "kallik" }, { "group", "kullok" },
     { "give", "khillosh" }, { "lust", "kellatat" },
     { "weapon", "tsittsa" }, { "eat", "chaach" },
     { "fool", "tchakl" }, { "that", "ta" },
     { "victory", "chaaug" }, { "gnome", "chaa" },
     { "me", "ra" }, { "will", "uh" },
     { "battle", "ralak" }, { "good", "rauuht" },
     { "joy", "nugnukkaah" }, { "no", "nug" },
     { "want", "aark" }, { "curse", "makkad" },
  	{ "", "" }
    },

    /* DARGONESTI */
    {
     { "a", "ae" }, { "b", "'ril" }, { "c", "k" }, { "d", "d" },
     { "e", "eia" }, { "f", "fir'" }, { "g", "gin'" }, { "h", "h" },
     { "i", "ie" }, { "j", "y" }, { "k", "kia" }, { "l", "li" },
     { "m", "mi'" }, { "n", "'ni" }, { "o", "u" }, { "p", "s" },
     { "q", "ki" }, { "r", "k" }, { "s", "sil'" }, { "t", "te'" },
     { "u", "a" }, { "v", "var" }, { "w", "s" }, { "x", "n" },
     { "y", "l" }, { "z", "s" },
      { "1", "1" }, { "2", "2" }, { "3", "3" }, { "4", "4" },
      { "5", "5" }, { "6", "6" }, { "7", "7" }, { "8", "8" },
      { "9", "9" }, { "0", "0" },
      { ",", "," }, { ".", "." }, { "!", "!" }, { "'", "'" },
      { "_", "_" }, { ";", ";" }, { ":", ":" }, { "=", "=" },
      { "\"", "\"" }, { ")", ")" }, { "(", "(" },
      { "*", "*" }, { "&", "&" }, { "^", "^" }, { "%", "%" },
      { "$", "$" }, { "#", "#" }, { "<", "<" }, 
      { "-", "-" },
     { "Thera", "Gaia" }, { "Takhisis", "Drakanna" },
     { "Hiddukel", "Betrayer" }, { "Shinare", "Shillara" },
     { "Paladine", "E'li" }, { "Mishakal", "Quenesti Pah" },
     { "Branchala", "Astarin" }, { "Solinari", "Beacon" },
     { "Nuitari", "Nil'rean" }, { "Chemosh", "Aeleth" },
     { "Airius", "Drakan" }, { "magic", "ilde" },
     { "might", "drauul" }, { "arm", "gurrt" },
     { "strength", "grauul" }, { "attack", "daah" },
     { "dragon", "drakon" }, { "murder", "leucth" },
     { "sword", "lierakla" }, { "horny", "leanna" },
     { "thief", "lecutta" }, { "gnoll", "lutteah" },
     { "blade", "lierakla" }, { "leg", "lauck" },
     { "attractive", "shulamushula" }, { "slime", "sleta" },
     { "soft", "suvvt" }, { "sexy", "shulamushuli" },
     { "cleric", "shanan" }, { "head", "serreck" },
     { "war", "sutta" }, { "oompa", "sex" },
     { "have", "kallak" }, { "dung", "karnaloghtch" },
     { "damn", "kkuleh" }, { "tribe", "kullek" },
     { "take", "kallik" }, { "group", "kullok" },
     { "give", "khillosh" }, { "lust", "kellatat" },
     { "weapon", "tsittsa" }, { "eat", "chaach" },
     { "fool", "tchakl" }, { "that", "ta" },
     { "victory", "chaaug" }, { "gnome", "chaa" },
     { "me", "ra" }, { "will", "uh" },
     { "battle", "ralak" }, { "good", "rauuht" },
     { "joy", "nugnukkaah" }, { "no", "nug" },
     { "want", "aark" }, { "curse", "makkad" },
  	{ "", "" }
    },

    /* DIMERNESTI */
    {
     { "a", "ae" }, { "b", "r'i" }, { "c", "k" }, { "d", "d" },
     { "e", "ey" }, { "f", "far'" }, { "g", "g'" }, { "h", "h" },
     { "i", "ii" }, { "j", "y" }, { "k", "ka" }, { "l", "li" },
     { "m", "mi'" }, { "n", "'ni" }, { "o", "u" }, { "p", "s" },
     { "q", "ku" }, { "r", "k" }, { "s", "sil'" }, { "t", "ty'" },
     { "u", "a" }, { "v", "var" }, { "w", "swe" }, { "x", "i'x" },
     { "y", "l" }, { "z", "s" },
      { "1", "1" }, { "2", "2" }, { "3", "3" }, { "4", "4" },
      { "5", "5" }, { "6", "6" }, { "7", "7" }, { "8", "8" },
      { "9", "9" }, { "0", "0" },
      { ",", "," }, { ".", "." }, { "!", "!" }, { "'", "'" },
      { "_", "_" }, { ";", ";" }, { ":", ":" }, { "=", "=" },
      { "\"", "\"" }, { ")", ")" }, { "(", "(" },
      { "*", "*" }, { "&", "&" }, { "^", "^" }, { "%", "%" },
      { "$", "$" }, { "#", "#" }, { "<", "<" }, 
      { "-", "-" },
     { "Thera", "Gaia" }, { "Takhisis", "Drakanna" },
     { "Hiddukel", "Betrayer" }, { "Shinare", "Shillara" },
     { "Paladine", "E'li" }, { "Mishakal", "Quenesti Pah" },
     { "Branchala", "Astarin" }, { "Solinari", "Beacon" },
     { "Nuitari", "Nil'rean" }, { "Chemosh", "Aeleth" },
     { "Airius", "Drakan" }, { "magic", "ilde" },
     { "might", "drauul" }, { "arm", "gurrt" },
     { "strength", "grauul" }, { "attack", "daah" },
     { "dragon", "drakon" }, { "murder", "leucth" },
     { "sword", "lierakla" }, { "horny", "leanna" },
     { "thief", "lecutta" }, { "gnoll", "lutteah" },
     { "blade", "lierakla" }, { "leg", "lauck" },
     { "attractive", "shulamushula" }, { "slime", "sleta" },
     { "soft", "suvvt" }, { "sexy", "shulamushuli" },
     { "cleric", "shanan" }, { "head", "serreck" },
     { "war", "sutta" }, { "oompa", "sex" },
     { "have", "kallak" }, { "dung", "karnaloghtch" },
     { "damn", "kkuleh" }, { "tribe", "kullek" },
     { "take", "kallik" }, { "group", "kullok" },
     { "give", "khillosh" }, { "lust", "kellatat" },
     { "weapon", "tsittsa" }, { "eat", "chaach" },
     { "fool", "tchakl" }, { "that", "ta" },
     { "victory", "chaaug" }, { "gnome", "chaa" },
     { "me", "ra" }, { "will", "uh" },
     { "battle", "ralak" }, { "good", "rauuht" },
     { "joy", "nugnukkaah" }, { "no", "nug" },
     { "want", "aark" }, { "curse", "makkad" },
  	{ "", "" }
    },

    /* DRACONIAN */
    {
     { "a", "aa" }, { "b", "k" }, { "c", "klik" }, { "d", "dhakin" },
     { "e", "eia" }, { "f", "firk" }, { "g", "glik'" }, { "h", "hak" },
     { "i", "icth" }, { "j", "y" }, { "k", "kria" }, { "l", "lia" },
     { "m", "mirk'" }, { "n", "nharak" }, { "o", "ok" }, { "p", "pk" },
     { "q", "kirk" }, { "r", "k" }, { "s", "s" }, { "t", "til" },
     { "u", "ahrak" }, { "v", "vhark" }, { "w", "wh" }, { "x", "nirax" },
     { "y", "y" }, { "z", "zhork" },
     { "1", "1" }, { "2", "2" }, { "3", "3" }, { "4", "4" },
     { "5", "5" }, { "6", "6" }, { "7", "7" }, { "8", "8" },
     { "9", "9" }, { "0", "0" },
     { ",", "," }, { ".", "." }, { "!", "!" }, { "'", "'" },
     { "_", "_" }, { ";", ";" }, { ":", ":" }, { "=", "=" },
     { "\"", "\"" }, { ")", ")" }, { "(", "(" },
     { "*", "*" }, { "&", "&" }, { "^", "^" }, { "%", "%" },
     { "$", "$" }, { "#", "#" }, { "<", "<" }, 
     { "-", "-" },
     { "weapon", "thoradrian" }, { "battle", "gnorashith" },
     { "treasure", "miradirashah" }, { "friend", "khadirith" },
     { "magician", "gnoraath" }, { "magical", "gnorathal" },
     { "magic", "gnoraah" }, { "strengh", "stilkeiatilhah" },
     { "weak", "kharsyiotha" }, { "be", "khil" }, { "they", "gnara" },
     { "he",  "atha" },  { "his", "arrah" }, { "she", "atha" },
     { "hers", "arrah" },  { "him", "atah" }, { "her", "athah" },
     { "the",  "aeh" }, { "and", "kai" }, { "father", "arrathana" },
     { "mother", "arrathanah" },  { "arcani", "arcahanai" },
     { "deurgar", "gnoratha" }, { "warrior", "khathkananan" }, 
     { "elf", "ellev'nia" }, { "elves", "ellev'niaie" },
	{ "", "" }
    },

    /* SOLAMNIC */
    {
     { "a", "ra" }, { "b", "ba" }, { "c", "k" }, { "d", "d" },
     { "e", "as" }, { "f", "th" }, { "g", "go" }, { "h", "h" },
     { "i", "i" }, { "j", "jo" }, { "k", "kia" }, { "l", "la" },
     { "m", "mi'" }, { "n", "n" }, { "o", "on" }, { "p", "s" },
     { "q", "que" }, { "r", "arus" }, { "s", "bi" }, { "t", "t" },
     { "u", "no" }, { "v", "var" }, { "w", "s" }, { "x", "ex" },
     { "y", "st" }, { "z", "s" },
      { "1", "1" }, { "2", "2" }, { "3", "3" }, { "4", "4" },
      { "5", "5" }, { "6", "6" }, { "7", "7" }, { "8", "8" },
      { "9", "9" }, { "0", "0" },
      { ",", "," }, { ".", "." }, { "!", "!" }, { "'", "'" },
      { "_", "_" }, { ";", ";" }, { ":", ":" }, { "=", "=" },
      { "\"", "\"" }, { ")", ")" }, { "(", "(" },
      { "*", "*" }, { "&", "&" }, { "^", "^" }, { "%", "%" },
      { "$", "$" }, { "#", "#" }, { "<", "<" }, 
      { "-", "-" },
     { "life", "mithas" }, { "honor", "sularus" },
     { "my", "est" }, { "is my", "oth" },
     { "Paladine", "Paladine" }, { "no", "non" },
     { "not", "non" }, { "us", "nobi" },
     { "our", "-ah" }, { "of", "mas" },
     { "and", "ai" }, { "is", "na" },
     { "grant", "coni" }, { "doubt", "nough" },
     { "death", "dix" }, { "grace", "parl" },
     { "sword", "paxum" }, { "heart", "saat" },
     { "now", "sac" }, { "solamnic", "solamnis" },
     { "solamnic knight", "solamnis" }, { "knighthood of solamnia", "solamnis" },
     { "warrior", "sularus" }, { "tarikan", "sularus" },
     { "honor", "sularus" }, { "warrior lord", "sularus" },
     { "is taken", "tarus" }, { "has been taken", "tarus" },
     { "taken", "tarus" }, { "wavers in", "vergi" },
     { "lord", "lor" }, { "call", "karai" },
     { "glory", "karamnes" }, { "broken", "kudak" },
     { "love", "fam" }, { "breath", "fai" },
     { "his", "ex" }, { "survive", "durvey" },
     { "son", "mac" }, { "dragon", "draco" },
     { "temper", "oparu" }, { "me with", "est" },
  	{ "", "" }
    },

    /* THIEVES CANT */
    {
     { "a", "a" }, { "b", "b" }, { "c", "c" }, { "d", "d" },
     { "e", "e" }, { "f", "f" }, { "g", "g" }, { "h", "h" },
     { "i", "i" }, { "j", "j" }, { "k", "k" }, { "l", "l" },
     { "m", "m" }, { "n", "n" }, { "o", "o" }, { "p", "p" },
     { "q", "q" }, { "r", "r" }, { "s", "s" }, { "t", "t" },
     { "u", "u" }, { "v", "v" }, { "w", "w" }, { "x", "x" },
     { "y", "y" }, { "z", "z" },
      { "1", "1" }, { "2", "2" }, { "3", "3" }, { "4", "4" },
      { "5", "5" }, { "6", "6" }, { "7", "7" }, { "8", "8" },
      { "9", "9" }, { "0", "0" },
      { ",", "," }, { ".", "." }, { "!", "!" }, { "'", "'" },
      { "_", "_" }, { ";", ";" }, { ":", ":" }, { "=", "=" },
      { "\"", "\"" }, { ")", ")" }, { "(", "(" },
      { "*", "*" }, { "&", "&" }, { "^", "^" }, { "%", "%" },
      { "$", "$" }, { "#", "#" }, { "<", "<" }, 
      { "-", "-" },
     { "mistress", "abbess" }, { "naked", "abram" },
     { "brothel", "Academy" }, { "widow", "Ace of Spades" },
     { "fling dust", "To amuse" }, { "delude", "amuse" },
     { "lawyer", "Ambidexter" }, { "flung dust", "Amuser" },
     { "begging", "angling" }, { "witness", "Affidavit" },
     { "associate", "Adam Tiler" }, { "hung", "acorned" },
     { "naked man", "abram cove" }, { "poor man", "abram cove" },
     { "malingering", "o sham abram" }, { "master thief", "Arch Rogue" },
     { "hot", "ard" }, { "stolen", "ard" },
     { "rogue in conjunction", "ark ruffian" }, { "hang", "dangle" },
     { "prostitute", "aunt" }, { "priest", "autem bawler" },
     { "church thief", "autem diver" }, { "female beggar", "autem mort" },
     { "criminals in the", "babes in the wood" }, { "burn", "badge" },
     { "sheriff", "bandog" }, { "bailiff", "bandog" },
     { "magic item", "bagged flash" }, { "transporter", "barrow man" },
     { "common", "bat" }, { "justice", "bawd" },
     { "criminal seller", "bawdy basket" }, { "alarm", "beef" },
     { "bartender", "beggar maker" }, { "good", "bene" },
     { "beer", "bowse" }, { "fellow", "cove" },
     { "night", "darkmans" }, { "counterfeiter", "bene feaker" },
     { "false passes", "gybes" }, { "fool", "ben" },
     { "crowbar", "bess" }, { "crow bar", "betty" },
     { "cheat", "bilk" }, { "con", "bilk" },
     { "jail", "bilboes" }, { "stocks", "bilboes" },
     { "go", "bing" }, { "escape", "bing" },
     { "brandy", "bingo" }, { "spirits", "bingo" },
     { "alchohal", "bingo" }, { "drinker", "bingo boy" },
     { "guild member", "bird of a feather" }, { "guildmember", "bird of a feather" },
     { "steal", "lift" }, { "pick", "black" },
     { "lock", "art" }, { "calf", "blater" },
     { "abandoned", "blasted" }, { "stealing", "bleating" },
     { "goods", "blow" }, { "cheated", "bleater" },
     { "sheep", "rig" }, { "innkeeper", "bluffer" },
     { "money", "blunt" }, { "prison", "boarding school" },
     { "shoplifter", "Bob" }, { "seized", "boned" },
     { "arrested", "boned" }, { "stolen", "borrowed" },
     { "cut", "boung" }, { "purse", "nipper" },
     { "risk", "boredom" }, { "safe", "box" },
     { "cracker", "man" }, { "woman", "brimstone" },
     { "serious injury", "broken finger" }, { "poor thief", "Bubber" },
     { "pickpoket", "buzman" }, { "flee", "burning" },
     { "constable", "bus napper" }, { "watchman", "kenchin" },
     { "fake coin", "button" }, { "inform", "cackle" },
     { "disguise", "calle" }, { "oath", "cap" },
     { "bully", "captain sharp" }, { "beggar", "canter" },
     { "lockpick", "charm" }, { "large sum", "caravan" },
     { "gypsies", "cattle" }, { "burned", "charactered" },
     { "gallow", "chatt" }, { "tankard", "clank" },
     { "tankard stealer", "clank napper" }, { "drunk", "clear" },
     { "to be hanged", "to climb up a ladder to bed" }, { "hit", "click" },
     { "thieves", "cloyes" }, { "whiped", "cly the jerk" },
     { "counterfeited", "confect" }, { "illegal", "contraband" },
     { "death sentence", "cramp word" }, { "a man", "cove" },
     { "house breaker", "cracksman" }, { "kill", "crash" },
     { "hide", "crawl in a hole" }, { "house", "crib" },
     { "honest", "cull" }, { "gullable", "cullability" },
     { "chains", "darbies" }, { "rope", "danglestuff" },
     { "dark", "darkee" }, { "hooded", "darkee" },
     { "bribe", "dawb" }, { "mugger", "ding boy" },
     { "hideout", "dive" }, { "innocent", "dodgies" },
     { "mute", "dommerer" }, { "follow", "drag" },
     { "bungling thief", "dromedary" }, { "losing", "dropping" },
     { "hand", "glove" }, { "tie", "dress" },
     { "pocket book", "dummy" }, { "counterfeiting coins", "drawing the king's picture" },
     { "rob", "dub" }, { "house", "lay" },
     { "silver", "dump" }, { "no fear", "die hard" },
     { "promise", "earnest" }, { "rich", "equipt" },
     { "gloves", "fams" }, { "dog", "fang" },
     { "reciever", "family man" }, { "palming", "fam lay" },
     { "alter", "figure" }, { "plan", "flag" },
     { "planning", "flag waving" }, { "magic", "flash" },
     { "thieves cant", "flash lingo" }, { "prison ship", "floating hell" },
     { "whip", "flog" }, { "common thief", "footpad" },
     { "a judge", "fortune teller" }, { "strangled", "frummagemmed" },
     { "trick", "fun" }, { "deceive", "gamon" },
     { "robbery", "game" }, { "highwayman", "gentleman's master" },
     { "die", "goin' legit" }, { "judge", "grandpa" },
     { "villainous", "hang gallows" }, { "assassin", "hitman" },
     { "accomplice", "hired help" }, { "informant", "hole in need of plugging" },
     { "blindfold", "hoodwink" }, { "captured", "hooked" },
     { "gold ring", "jem" }, { "riot", "hubbub" },
     { "staff", "jordain" }, { "stick", "jordain" },
     { "young thieves", "keddeys" }, { "shackles", "king's plate" },
     { "gold", "king's picture" }, { "hello", "knock-knock" },
     { "danger", "lay" }, { "door", "lid" },
     { "group", "mob" }, { "night watchmen", "moon pigs" },
     { "hemp", "neck weed" }, { "married", "noozed" },
     { "courthouse", "nubbing ken" }, { "cudgel", "oaken towel" },
     { "experienced", "old" }, { "seal", "onion" },
     { "signet ring", "onion" }, { "guard", "porker" },
     { "conceal", "paum" }, { "telescope", "peeper" },
     { "spy", "peeping tom" }, { "dagger", "pig poker" },
     { "shop", "broker" }, { "body", "quarromes" },
     { "share", "quota" }, { "coachman", "rattling cove" },
     { "paper", "rag" }, { "larceny", "racket" },
     { "goal", "repository" }, { "successful", "robert's men" },
     { "cheap ale", "rot gut" }, { "professional", "rounder" },
     { "fine", "rum" }, { "good", "rum" },
     { "valuable", "rum" }, { "secure", "rug" },
     { "clever trick", "rum bite" }, { "skilled", "rum" },
     { "invade", "rush" }, { "loot", "score" },
     { "execution", "scragg" }, { "injury", "scratch" },
     { "key", "screw" }, { "con man", "sharper" },
     { "hangman", "sheriff's journeyman" }, { "pilferer", "sneak" },
     { "quiet", "smooth" }, { "trial", "show" },
     { "backstab", "shoulder tap" }, { "organ", "squeeker" },
     { "booty", "swag" }, { "sell", "switch" },
     { "flogging", "teize" }, { "wall", "tilted floor" },
     { "killed", "used up" }, { "knock", "tuck" },
     { "inconscious", "in bed" }, { "pawn", "vamp" },
     { "criminals", "varlets" }, { "prison guard", "underdubber" },
     { "silver plate", "wedge" }, { "pirate", "waterpad" },
     { "day of execution", "wry neck day" }, { "cry", "yelp" },
     { "crooked", "zad" }, { "heavy", "fat lady" },
     { "light", "thin woman" }, { "big", "tall man" },
     { "small", "short man" }, { "sapphire", "berry wine" },
     { "jewel", "booze" }, { "ruby", "Chunk o' Brandy" },
     { "diamond", "Chunk o' Gin" }, { "jewelry", "fancy booze" },
     { "ring", "finger joint" }, { "emerald", "green beer" },
     { "earring", "lobe latch" }, { "pearl", "milk" },
     { "necklace", "noose" }, { "copper", "rust" },
     { "platinum", "shiny tin" }, { "noon", "six dirty fingers" },
     { "mignight", "thumbs" }, { "sunrise", "gettin' dirty" },
     { "sunset", "almost clean" }, { "lookout", "pair o' eyes" },
     { "thief", "gentleman" }, { "boring", "risky" },
  	{ "", "" }
    }
};

void do_speak( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    int i;

    if( IS_NPC(ch) )
	return;

    if( argument[0] == '\0' )
    {
	sprintf(buf, "You currently speak the %s tongue.\n\r", 
	  lang_lookup(ch->language));
	send_to_char(buf, ch);
	return;
    }

    if (IS_AFFECTED2(ch, AFF_FORGET))
    {
	send_to_char("You can't seem to remember any language other than the one you currently speak.\n\r",ch);
	return;
    }

    for (i=0 ; i < MAX_LANG ; i++)
    { 
        if (!str_prefix(argument, lang_lookup(i)))
	{
	    ch->language = i;
	    sprintf(buf, "You formulate your thoughts to speak the %s tongue."
		"\r\n", lang_lookup(ch->language));
	    send_to_char(buf, ch);
	    return;
	}
    }
    sprintf(buf, "You know nothing about %s, and could not possible speak it."		"\r\n", argument);
    send_to_char(buf, ch);
    return;
}

char * lang_lookup( int lang )
{
    return lang_table[lang].lang;
}
int get_lang_skill(int lang)
{
    int language;

    switch(lang)
    {
    default:			return FALSE;        			break;
    case LANG_ELVEN:		language = gsn_lang_elven;		break;
    case LANG_DROW:		language = gsn_lang_drow;		break;
    case LANG_DWARVEN:		language = gsn_lang_dwarven;		break;
    case LANG_OGRE:		language = gsn_lang_ogre;		break;
    case LANG_KENDER:		language = gsn_lang_kender;		break;
    case LANG_MINOTAUR:		language = gsn_lang_minotaur;		break;
    case LANG_SILVANESTI:	language = gsn_lang_silvanesti;	break;
    case LANG_QUALINESTI:	language = gsn_lang_qualinesti;	break;
    case LANG_KAGONESTI:	language = gsn_lang_kagonesti;	break;
    case LANG_DARGONESTI:	language = gsn_lang_dargonesti;	break;
    case LANG_DIMERNESTI:	language = gsn_lang_dimernesti;	break;
    case LANG_DRACONIAN:	language = gsn_lang_draconian;	break;
    case LANG_SOLAMNIC:		language = gsn_lang_solamnic;		break;
    case LANG_THIEVES_CANT:	language = gsn_lang_thieves_cant; 	break;
    }

    return language;
}

int lang_skill(CHAR_DATA *ch, CHAR_DATA *vict)
{
    int language;
    int cskill = 0;
    int skill = 44;

    if (ch->language == LANG_COMMON || IS_NPC(ch)) 
	return FALSE;

    language = get_lang_skill(ch->language);

    cskill = ch->pcdata->learned[language];
    if (vict)
    {
        skill = IS_NPC(vict) ? 0 : vict->pcdata->learned[language];
	  if(IS_IMMORTAL(vict)) skill = 100;
    }

    check_improve(ch, language, TRUE, 1);
    if (skill < 33 || (skill >= 33 && (skill + cskill) <= 75))
	return 0;
    else
	return 1;
}

char * language_check( CHAR_DATA *ch, char *mesg, CHAR_DATA *vict )
{
    char *new_mesg = NULL;
    char buf[100];
    char buf2[MAX_STRING_LENGTH];
    sh_int language = 0;
    int skill, cskill;
    bool cHas = TRUE;

    if (ch->language == LANG_COMMON || IS_NPC(ch)) 
        return mesg;

    language = get_lang_skill(ch->language);

    if( ( cskill = ch->pcdata->learned[language]) == 0)
        cHas = FALSE;

    if(vict)
    {
	skill = IS_NPC(vict) ? 0 : vict->pcdata->learned[language];
	if(IS_IMMORTAL(vict)) skill = 100; //Immortals are 100% Language.
	new_mesg = lang_convert( mesg, ch->language, cskill, skill, FALSE );
	return(new_mesg);
    }
    else
    {
	if(!cHas)
	{
	    sprintf(buf, "You know nothing about the %s language, and decide to use common instead.\n\r", lang_lookup(ch->language));
	    ch->language = LANG_COMMON;
	    send_to_char(buf, ch);
	    sprintf(buf2, "%s", mesg);
            return(str_dup(buf2));
	}
        new_mesg = lang_convert( mesg, ch->language, 0, cskill, TRUE );
        return(new_mesg);
    }
}

char * lang_convert( char *mesg, int lang, int cskill, int skill, bool Ischar )
{
    char buf[MSL];
    char extra1[2];
    char *extra = extra1;
    char mpart[2];
    int i, len=0, percent;
    bool Mesg = FALSE;
    bool Cap = FALSE;
    int j = 1;

    buf[0] = '\0';
    for( ; *mesg != '\0'; mesg += len ) 
    {
	if(!Mesg && !Ischar)
	{
	    if (*mesg == '\'')
	    {
	        len=1;
	        continue;
 	    }
 	    else
 	        Mesg = TRUE;
	}
	if( *mesg < 'A' || *mesg > 'z' )
	{
	    extra[0] = *mesg;
	    extra[1] = '\0';
	    sprintf(mpart, "%s", extra);
	    strcat(buf, mpart);
	    len=1;
	    continue;
	}
	if( *mesg >= 'A' && *mesg <= 'Z' )
	    Cap = TRUE;
	if( *mesg >= 'a' && *mesg <= 'z' )
	    Cap = FALSE;
	for( i=26 ; j && (len = strlen(lang_chars[lang][i].old)) != 0 ; i++)
	{
	    if(!str_prefix(lang_chars[lang][i].old, mesg))
	    {
		percent = number_percent();
		if( Ischar && percent > skill )
		{
		    if( Cap )
			strcat(buf, capitalize(lang_chars[lang][number_range(0,25)].new));
		    else
		        strcat(buf, lang_chars[lang][number_range(0,25)].new);
	 	}
		else if( percent > skill )
		{
		    if( Cap )
			strcat(buf, capitalize(lang_chars[lang][i].new));
		    else
		        strcat(buf, lang_chars[lang][i].new);
		}
		else  
		{
		    if( Cap )
			strcat(buf, capitalize(lang_chars[lang][i].old));
		    else
		        strcat(buf, lang_chars[lang][i].old);
		}
		break;
	    }
	
	    if ((len = strlen(lang_chars[lang][i+1].old)) == 0)
	        i = -1;

	    if (i == 25)
	        j = 0; 
	}
	if( len == 0 )
	    len = 1;
    }
    return(str_dup(buf));
}
