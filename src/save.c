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
#include <string.h>
#include <time.h>
#include <malloc.h>
#include "merc.h"
#include "recycle.h"
#include "tables.h"
#include "lookup.h"

#define PFILE_VERSION 10
 
#if !defined(macintosh)
extern  int     _filbuf         args( (FILE *) );
#endif

void do_reform       args( (CHAR_DATA *ch) );

/* int rename(const char *oldfname, const char *newfname); viene en stdio.h */

char *print_flags(int flag)
{
    int count, pos = 0;
    static char buf[52];


    for (count = 0; count < 32;  count++)
    {
        if (IS_SET(flag,1<<count))
        {
            if (count < 26)
                buf[pos] = 'A' + count;
            else
                buf[pos] = 'a' + (count - 26);
            pos++;
        }
    }

    if (pos == 0)
    {
        buf[pos] = '0';
        pos++;
    }

    buf[pos] = '\0';

    return buf;
}


/*
 * Array of containers read for proper re-nesting of objects.
 */
#define MAX_NEST	100
static	OBJ_DATA *	rgObjNest	[MAX_NEST];



/*
 * Local functions.
 */
void	fwrite_char	args( ( CHAR_DATA *ch,  FILE *fp ) );
void	fwrite_obj	args( ( CHAR_DATA *ch,  OBJ_DATA  *obj,
			    FILE *fp, int iNest ) );
void	fwrite_pet	args( ( CHAR_DATA *pet, FILE *fp) );
void	fread_char	args( ( CHAR_DATA *ch,  FILE *fp ) );
void    fread_pet	args( ( CHAR_DATA *ch,  FILE *fp ) );
void	fread_obj	args( ( CHAR_DATA *ch,  FILE *fp ) );



/*
 * Save a character and inventory.
 * Would be cool to save NPC's too for quest purposes,
 *   some of the infrastructure is provided.
 */
void save_char_obj( CHAR_DATA *ch )
{
    char strsave[MAX_INPUT_LENGTH];
    FILE *fp;

    if ( IS_NPC(ch) )
	return;

    //C073 Added from Edwin's fix list
    //
    // Don't save if the character is invalidated.
    // This might happen during the auto-logoff of players.
    // (or other places not yet found out)
    //
    if ( !IS_VALID(ch)) {
        bugf("save_char_obj: Trying to save an invalidated character.\n");
        return;
    }

    if ( ch->desc != NULL && ch->desc->original != NULL )
	ch = ch->desc->original;

#if defined(unix)
    /* create god log */
    if (IS_IMMORTAL(ch) || ch->level >= LEVEL_IMMORTAL)
    {
	fclose(fpReserve);
	sprintf(strsave, "%s%s",GOD_DIR, capitalize(ch->name));
	if ((fp = fopen(strsave,"w")) == NULL)
	{
	    bug("Save_char_obj: fopen",0);
	    perror(strsave);
 	}

	fprintf(fp,"Lev %2d Trust %2d  %s%s\n",
	    ch->level, get_trust(ch), ch->name, ch->pcdata->title);
	fclose( fp );
	fpReserve = fopen( NULL_FILE, "r" );
    }
#endif

    fclose( fpReserve );
    sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( ch->name ) );
    if ( ( fp = fopen( TEMP_FILE, "w" ) ) == NULL )
    {
	bug( "Save_char_obj: fopen", 0 );
	perror( strsave );
    }
    else
    {
	fwrite_char( ch, fp );
	if ( ch->carrying != NULL )
	    fwrite_obj( ch, ch->carrying, fp, 0 );
	/* save the pets */
	if (ch->pet != NULL && ch->pet->in_room == ch->in_room)
	    fwrite_pet(ch->pet,fp);
	fprintf( fp, "#END\n" );
    }
    fclose( fp );
    rename(TEMP_FILE,strsave);
    fpReserve = fopen( NULL_FILE, "r" );
    return;
}



/*
 * Write the char.
 */
void fwrite_char( CHAR_DATA *ch, FILE *fp )
{
    AFFECT_DATA *paf;
    int sn, pos; 
    int x,y;

    fprintf( fp, "#%s\n", 			IS_NPC(ch) ? "MOB" : "PLAYER"		);
    fprintf( fp, "Name %s~\n",		ch->name		);
    if (ch->surname != NULL)
        fprintf( fp, "Surname %s~\n",	ch->surname		);
    if (ch->maskedip != NULL)
        fprintf( fp, "MaskedIp %s~\n",	ch->maskedip	);
    if (!IS_NPC(ch) && (ch->pcdata->lokprot != NULL))
        fprintf( fp, "Lokprot %s~\n",	ch->pcdata->lokprot	);
    fprintf( fp, "Email %s~\n", 		ch->pcdata->email	);
    fprintf( fp, "Id   %ld\n", 		ch->id		);
    fprintf( fp, "LogO %ld\n",		current_time	);
    fprintf( fp, "Vers %d\n",   		PFILE_VERSION	);
    if (ch->act != 0)
	fprintf( fp, "Act  %s\n",   		print_flags(ch->act)); //C070
    if (ch->act2 != 0)
	fprintf( fp, "Act2  %s\n",   		print_flags(ch->act2));
    if (ch->short_descr[0] != '\0')
      	fprintf( fp, "ShD  %s~\n",	ch->short_descr	);
    if( ch->long_descr[0] != '\0')
	fprintf( fp, "LnD  %s~\n",		ch->long_descr	);
    if (ch->description[0] != '\0')
    	fprintf( fp, "Desc %s~\n",		ch->description	);
    if (ch->background[0] != '\0')
    	fprintf( fp, "Backg %s~\n",		ch->background	);
    if( ch->prompt != NULL
     || !str_cmp( ch->prompt,	"<%hhp %mm %vmv> " )
     || !str_cmp( ch->prompt,"{r<%hhp %mm %vmv>{x " ) )
        fprintf( fp, "Prom %s~\n",      	ch->prompt  	);
    fprintf( fp, "Race %s~\n", 		pc_race_table[ch->race].name );
    if (ch->clan_id != 0)
	fprintf( fp, "Clan %d\n",		ch->clan_id		);
    if (ch->org_id != 0)
	fprintf( fp, "Org %d\n",		ch->org_id		);
    if (ch->religion_id != 0)
	fprintf( fp, "Religion %d\n",   	ch->religion_id 	);
    if (ch->devotee_id != 0)
	fprintf( fp, "Devotee %d\n",   	ch->devotee_id 	);
    if (ch->pcdata->highpriest != 0)
	fprintf( fp, "Highpriest %d\n",   	ch->pcdata->highpriest );
    if (ch->language != 0)
	fprintf( fp, "Language %d\n",		ch->language	);
    fprintf( fp, "Sex  %d\n",			ch->sex		);
    fprintf( fp, "Cla  %d\n",			ch->class		);
    fprintf( fp, "Hmtown %d\n", 		ch->hometown	);
    fprintf( fp, "Levl %d\n",			ch->level		);
    fprintf( fp, "recall_point %d\n", 	ch->recall_point 	);
    fprintf( fp, "Levscale %d\n", 		LEVEL_SCALE		);
    fprintf( fp, "Xptolev  %d\n",
	exp_at_level( ch, ch->level + 1) - ch->exp		);
    if (ch->deathstat > 0)
      fprintf( fp, "Deathstat %d\n",ch->deathstat       	);
    if (ch->trust != 0)
	fprintf( fp, "Tru  %d\n",		ch->trust		);
    fprintf( fp, "Sec  %d\n",    		ch->pcdata->security	);	/* OLC */
    fprintf( fp, "Plyd %d\n",
	ch->played + (int) (current_time - ch->logon)		);
    fprintf( fp, "Not  %ld %ld %ld %ld %ld\n",		
	ch->pcdata->last_note,ch->pcdata->last_idea,ch->pcdata->last_penalty,
	ch->pcdata->last_news,ch->pcdata->last_changes		);
    fprintf( fp, "Scro %d\n", 		ch->lines		);
    fprintf( fp, "Room %d\n",
        (  ch->in_room == get_room_index( ROOM_VNUM_LIMBO )
        && ch->was_in_room != NULL )
            ? ch->was_in_room->vnum
            : ch->in_room == NULL ? 3001 : ch->in_room->vnum );
    if (ch->loc_hp != 0)
	fprintf( fp, "LocationHP %s\n", print_flags(ch->loc_hp));
    if (ch->loc_hp2 != 0)
	fprintf( fp, "LocationHP2 %s\n", print_flags(ch->loc_hp2));
    fprintf( fp, "HMV  %d %d %d %d %d %d\n",
	ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move, ch->max_move );
    if (ch->steel > 0)
      fprintf( fp, "Steel %ld\n",		ch->steel		);
    else
      fprintf( fp, "Steel %d\n", 		0			); 
    if (ch->gold > 0)
	fprintf( fp, "Gold %ld\n",		ch->gold		);
    else
	fprintf( fp, "Gold %d\n",		0			);
    if (ch->bank > 0)
	fprintf( fp, "Bank %ld\n",		ch->bank		);
    else
	fprintf( fp, "Bank %ld\n", (long int) 0			);
    fprintf( fp, "Exp  %d\n",			ch->exp		);
    if (ch->affected_by != 0)
	fprintf( fp, "AfBy %s\n",   print_flags(ch->affected_by));
    if (ch->affected2_by != 0)
	fprintf( fp, "AfBy2 %s\n",   print_flags(ch->affected2_by));
    if (ch->affected3_by != 0)
	fprintf( fp, "AfBy3 %s\n",   print_flags(ch->affected3_by));
    if (ch->pcdata->deaths)
	fprintf( fp, "Deaths %d\n",		ch->pcdata->deaths);
    fprintf( fp, "FlagTot"						);
    for (x=0;x < 3; x++)
     for (y=0;y < 6; y++)
      fprintf(fp, " %d",     			ch->pcdata->flags[x][y]	);
    fprintf( fp, "\n"							);
    fprintf( fp, "FParTot"						);
    for (x=0;x < 3; x++)
     for (y=0;y < 6; y++)
      fprintf(fp, " %d",     			ch->pcdata->pardons[x][y]	);
     fprintf( fp, "\n"							);
    fprintf( fp, "Comm %s\n",       	print_flags(ch->comm));
    if (ch->wiznet)
    	fprintf( fp, "Wizn %s\n",   		print_flags(ch->wiznet));
    if (ch->invis_level)
	fprintf( fp, "Invi %d\n", 		ch->invis_level	);
    if (ch->incog_level)
	fprintf(fp,"Inco %d\n",			ch->incog_level	);
    fprintf( fp, "Pos  %d\n",	
	ch->position == POS_FIGHTING ? POS_STANDING : ch->position );
 /*   fprintf( fp, "Time %d\n",		ch->quit_timer ); */
    if (ch->practice != 0)
    	fprintf( fp, "Prac %d\n",		ch->practice	);
    if (ch->train != 0)
	fprintf( fp, "Trai %d\n",		ch->train		);
    if (ch->penalize_timer != 0)
	fprintf( fp, "Penal_timer %d\n",	ch->penalize_timer);
    if (ch->reward_timer != 0)
	fprintf( fp, "Reward_timer %d\n",	ch->reward_timer	);
    if (ch->saving_throw != 0)
	fprintf( fp, "Save  %d\n",		ch->saving_throw	);
    fprintf( fp, "Alig  %d\n",		ch->alignment	);
    fprintf( fp, "Ethos %d\n",		ch->ethos		);
    if (ch->hitroll != 0)
	fprintf( fp, "Hit   %d\n",		ch->hitroll		);
    if (ch->damroll != 0)
	fprintf( fp, "Dam   %d\n",		ch->damroll		);
    fprintf( fp, "ACs %d %d %d %d\n",	
	ch->armor[0],ch->armor[1],ch->armor[2],ch->armor[3]);
    if (ch->wimpy !=0 )
	fprintf( fp, "Wimp  %d\n",		ch->wimpy		);
    if (ch->yield !=0 )
      fprintf( fp, "Yield  %d\n",      	ch->yield       	);
    fprintf( fp, "Attr %d %d %d %d %d %d\n",
	ch->perm_stat[STAT_STR],
	ch->perm_stat[STAT_INT],
	ch->perm_stat[STAT_WIS],
	ch->perm_stat[STAT_DEX],
	ch->perm_stat[STAT_CON],
	ch->perm_stat[STAT_CHR] );

    fprintf (fp, "AMod %d %d %d %d %d %d\n",
	ch->mod_stat[STAT_STR],
	ch->mod_stat[STAT_INT],
	ch->mod_stat[STAT_WIS],
	ch->mod_stat[STAT_DEX],
	ch->mod_stat[STAT_CON],
	ch->mod_stat[STAT_CHR] );

    if ( IS_NPC(ch) )
    {
	fprintf( fp, "Vnum %d\n",		ch->pIndexData->vnum	);
    }
    else
    {
	fprintf( fp, "Pass %s~\n",		ch->pcdata->pwd		);
	if (ch->pcdata->bamfin[0] != '\0')
	    fprintf( fp, "Bin  %s~\n",	ch->pcdata->bamfin);
	if (ch->pcdata->bamfout[0] != '\0')
		fprintf( fp, "Bout %s~\n",	ch->pcdata->bamfout);
        if (ch->pcdata->smite[0] != '\0')
                fprintf( fp, "Smite %s~\n",     ch->pcdata->smite);
        if (ch->pcdata->fadein[0] != '\0')
                fprintf( fp, "Fin %s~\n",       ch->pcdata->fadein);
        if (ch->pcdata->fadeout[0] != '\0')
                fprintf( fp, "Fout %s~\n",      ch->pcdata->fadeout);
        if (ch->pcdata->transin[0] != '\0')
                fprintf( fp, "Tin %s~\n",       ch->pcdata->transin);
        if (ch->pcdata->transout[0] != '\0')
                fprintf( fp, "Tout %s~\n",      ch->pcdata->transout);  
	fprintf( fp, "Titl %s~\n",		ch->pcdata->title		);
	fprintf( fp, "Pretit %s~\n", 		ch->pcdata->pretit 	);
    	fprintf( fp, "Pnts %d\n",   		ch->pcdata->points      );
	fprintf( fp, "TSex %d\n",		ch->pcdata->true_sex	);
	fprintf( fp, "LLev %d\n",		ch->pcdata->last_level	);
	fprintf( fp, "HMVP %d %d %d\n", 	ch->pcdata->perm_hit, 
						   	ch->pcdata->perm_mana,
						   	ch->pcdata->perm_move);
	fprintf( fp, "Cnd  %d %d %d %d\n",
	    ch->pcdata->condition[0],
	    ch->pcdata->condition[1],
	    ch->pcdata->condition[2],
	    ch->pcdata->condition[3] );

      if (ch->questpoints != 0)
          fprintf( fp, "QuestPnts %d\n",  ch->questpoints 	);
      if (ch->nextquest != 0)
          fprintf( fp, "QuestNext %d\n",  ch->nextquest   	);
      else if (ch->countdown != 0)
          fprintf( fp, "QuestNext %d\n",  2              	);

/*        //C028
	fprintf( fp, "RaceText %s~\n",	ch->pcdata->racetext);
        fprintf( fp, "ClanTitle %s~\n",	ch->pcdata->clantitle);
*/

      //C039
      //Only write Racetext or clantitle if it doesnt have a value and not @
      if (ch->pcdata->racetext != NULL && str_cmp(ch->pcdata->racetext,"@"))
          fprintf( fp, "RaceText %s~\n", ch->pcdata->racetext);
      if (ch->pcdata->clantitle != NULL && str_cmp(ch->pcdata->clantitle,"@"))
          fprintf( fp, "ClanTitle %s~\n", ch->pcdata->clantitle);

	/*
	 * Write Colour Config Information.
	 */
	fprintf( fp, "Coloura     %d%d%d %d%d%d %d%d%d %d%d%d %d%d%d\n",   
		ch->pcdata->text[2],
		ch->pcdata->text[0],
		ch->pcdata->text[1],
		ch->pcdata->auction[2],
		ch->pcdata->auction[0],
		ch->pcdata->auction[1],
		ch->pcdata->gossip[2],
		ch->pcdata->gossip[0],
		ch->pcdata->gossip[1],
		ch->pcdata->music[2],
		ch->pcdata->music[0],
		ch->pcdata->music[1],
		ch->pcdata->ask[2],
		ch->pcdata->ask[0],
		ch->pcdata->ask[1] );
	fprintf( fp, "Colourb     %d%d%d %d%d%d %d%d%d %d%d%d %d%d%d\n",   
		ch->pcdata->answer[2],
		ch->pcdata->answer[0],
		ch->pcdata->answer[1],
		ch->pcdata->quote[2],
		ch->pcdata->quote[0],
		ch->pcdata->quote[1],
		ch->pcdata->quote_text[2],
		ch->pcdata->quote_text[0],
		ch->pcdata->quote_text[1],
		ch->pcdata->immtalk_text[2],
		ch->pcdata->immtalk_text[0],
		ch->pcdata->immtalk_text[1],
		ch->pcdata->immtalk_type[2],
		ch->pcdata->immtalk_type[0],
		ch->pcdata->immtalk_type[1] );
	fprintf( fp, "Colourc     %d%d%d %d%d%d %d%d%d %d%d%d %d%d%d\n",   
		ch->pcdata->info[2],
		ch->pcdata->info[0],
		ch->pcdata->info[1],
		ch->pcdata->tell[2],
		ch->pcdata->tell[0],
		ch->pcdata->tell[1],
		ch->pcdata->reply[2],
		ch->pcdata->reply[0],
		ch->pcdata->reply[1],
		ch->pcdata->gtell_text[2],
		ch->pcdata->gtell_text[0],
		ch->pcdata->gtell_text[1],
		ch->pcdata->gtell_type[2],
		ch->pcdata->gtell_type[0],
		ch->pcdata->gtell_type[1] );
	fprintf( fp, "Colourd     %d%d%d %d%d%d %d%d%d %d%d%d %d%d%d\n",   
		ch->pcdata->room_title[2],
		ch->pcdata->room_title[0],
		ch->pcdata->room_title[1],
		ch->pcdata->room_text[2],
		ch->pcdata->room_text[0],
		ch->pcdata->room_text[1],
		ch->pcdata->room_exits[2],
		ch->pcdata->room_exits[0],
		ch->pcdata->room_exits[1],
		ch->pcdata->room_things[2],
		ch->pcdata->room_things[0],
		ch->pcdata->room_things[1],
		ch->pcdata->prompt[2],
		ch->pcdata->prompt[0],
		ch->pcdata->prompt[1] );
	fprintf( fp, "Coloure     %d%d%d %d%d%d %d%d%d %d%d%d %d%d%d\n",   
		ch->pcdata->fight_death[2],
		ch->pcdata->fight_death[0],
		ch->pcdata->fight_death[1],
		ch->pcdata->fight_yhit[2],
		ch->pcdata->fight_yhit[0],
		ch->pcdata->fight_yhit[1],
		ch->pcdata->fight_ohit[2],
		ch->pcdata->fight_ohit[0],
		ch->pcdata->fight_ohit[1],
		ch->pcdata->fight_thit[2],
		ch->pcdata->fight_thit[0],
		ch->pcdata->fight_thit[1],
		ch->pcdata->fight_skill[2],
		ch->pcdata->fight_skill[0],
		ch->pcdata->fight_skill[1] );
	fprintf( fp, "Colourf     %d%d%d %d%d%d %d%d%d %d%d%d %d%d%d\n",   
		ch->pcdata->wiznet[2],
		ch->pcdata->wiznet[0],
		ch->pcdata->wiznet[1],
		ch->pcdata->say[2],
		ch->pcdata->say[0],
		ch->pcdata->say[1],
		ch->pcdata->say_text[2],
		ch->pcdata->say_text[0],
		ch->pcdata->say_text[1],
		ch->pcdata->tell_text[2],
		ch->pcdata->tell_text[0],
		ch->pcdata->tell_text[1],
		ch->pcdata->reply_text[2],
		ch->pcdata->reply_text[0],
		ch->pcdata->reply_text[1] );
	fprintf( fp, "Colourg     %d%d%d %d%d%d %d%d%d %d%d%d %d%d%d\n",   
		ch->pcdata->auction_text[2],
		ch->pcdata->auction_text[0],
		ch->pcdata->auction_text[1],
		ch->pcdata->gossip_text[2],
		ch->pcdata->gossip_text[0],
		ch->pcdata->gossip_text[1],
		ch->pcdata->music_text[2],
		ch->pcdata->music_text[0],
		ch->pcdata->music_text[1],
		ch->pcdata->ask_text[2],
		ch->pcdata->ask_text[0],
		ch->pcdata->ask_text[1],
		ch->pcdata->answer_text[2],
		ch->pcdata->answer_text[0],
		ch->pcdata->answer_text[1] );



	/* write alias */
        for (pos = 0; pos < MAX_ALIAS; pos++)
	{
	    if (ch->pcdata->alias[pos] == NULL
	    ||  ch->pcdata->alias_sub[pos] == NULL)
		break;

	    fprintf(fp,"Alias %s %s~\n",ch->pcdata->alias[pos],
		    ch->pcdata->alias_sub[pos]);
	}

	for ( sn = 0; sn < MAX_SKILL; sn++ )
	{
	    if ( skill_table[sn].name != NULL 
	      && ch->pcdata->learned[sn] > 0 
	      && legal_skill( ch, sn ) )
	    {
		fprintf( fp, "Sk %d '%s'\n",
		    ch->pcdata->learned[sn], skill_table[sn].name );
	    }
	}
    }

    for ( paf = ch->affected; paf != NULL; paf = paf->next )
    {
	if (paf->type < 0 || paf->type >= MAX_SKILL || paf->type == gsn_doppelganger)
	    continue;
	
	fprintf( fp, "Affc '%s' %3d %3d %3d %3d %3d %10d\n",
	    skill_table[paf->type].name,
	    paf->where,
	    paf->level,
	    paf->duration,
	    paf->modifier,
	    paf->location,
	    paf->bitvector
	    );
    }

    fprintf( fp, "End\n\n" );
    return;
}

/* 
 * legal_skill
 *
 * Returns TRUE if the char is allowed to possess this skill.
 */
bool legal_skill( CHAR_DATA *ch, int sn )
{
    int i, org;
    char *name, *s_name;

    /* imms can have all skills */
    if( ch->level >= 102 ) 
      return TRUE;

    name = skill_table[sn].name;
    if( name == NULL )
	return FALSE;
    if( strlen(name) == 0 )
	return FALSE;

    /* look for skill allowed by class */
    for(i = 0; i < MAX_IN_GROUP; i++)
    {
	s_name = group_table[ch->class].spells[i];
	if(s_name == NULL)
	    break;
	if( !strcmp(s_name, name) )
	    return TRUE;
    }

    /* look for skill allowed by race */
    for(i = 0; i < 6; i++)
    {
	s_name = pc_race_table[ch->race].skills[i];
	if(s_name == NULL)
	    break;
	if( !strcmp(s_name, name) )
	    return TRUE;
    }

    /* look for skill allowed by clan */
    org = ORG(ch->org_id);
    for(i = 0; i < 10; i++)
    {
	s_name = org_table[org].skills[i];
	if(s_name == NULL)
	    break;
	if( !strcmp(s_name, name) )
	    return TRUE;
    }
    return FALSE;
}


/* write a pet */
void fwrite_pet( CHAR_DATA *pet, FILE *fp)
{
    AFFECT_DATA *paf;
    
    fprintf(fp,"#PET\n");
    
    fprintf(fp,"Vnum %d\n",pet->pIndexData->vnum);
    
    fprintf(fp,"Name %s~\n", pet->name);
    fprintf(fp,"LogO %ld\n", current_time);
    if (pet->short_descr != pet->pIndexData->short_descr)
    	fprintf(fp,"ShD  %s~\n", pet->short_descr);
    if (pet->long_descr != pet->pIndexData->long_descr)
    	fprintf(fp,"LnD  %s~\n", pet->long_descr);
    if (pet->description != pet->pIndexData->description)
    	fprintf(fp,"Desc %s~\n", pet->description);
    if (pet->race != pet->pIndexData->race)
    	fprintf(fp,"Race %s~\n", race_table[pet->race].name);
    fprintf(fp,"Sex  %d\n", pet->sex);
    if (pet->level != pet->pIndexData->level)
    	fprintf(fp,"Levl %d\n", pet->level);
    fprintf(fp, "HMV  %d %d %d %d %d %d\n",
    	pet->hit, pet->max_hit, pet->mana, pet->max_mana, pet->move, pet->max_move);
    if (pet->steel > 0)
    	fprintf(fp,"Steel %ld\n",pet->steel);
    if (pet->gold > 0)
	fprintf(fp,"Gold %ld\n",pet->gold);
    if (pet->exp > 0)
    	fprintf(fp, "Exp  %d\n", pet->exp);
    if (pet->act != pet->pIndexData->act)
    	fprintf(fp, "Act  %s\n", print_flags(pet->act));
    if (pet->act2 != pet->pIndexData->act2)
    	fprintf(fp, "Act2  %s\n", print_flags(pet->act2));
    if (pet->affected_by != pet->pIndexData->affected_by)
    	fprintf(fp, "AfBy %s\n", print_flags(pet->affected_by));
    if (pet->affected2_by != pet->pIndexData->affected_by)
    	fprintf(fp, "AfBy2 %s\n", print_flags(pet->affected2_by));
    if (pet->affected3_by != pet->pIndexData->affected_by)
    	fprintf(fp, "AfBy3 %s\n", print_flags(pet->affected3_by));
    if (pet->comm != 0)
    	fprintf(fp, "Comm %s\n", print_flags(pet->comm));
    fprintf(fp,"Pos  %d\n", pet->position = POS_FIGHTING ? POS_STANDING : pet->position);
    if (pet->saving_throw != 0)
    	fprintf(fp, "Save %d\n", pet->saving_throw);
    if (pet->alignment != pet->pIndexData->alignment)
    	fprintf(fp, "Alig %d\n", pet->alignment);
    if (pet->hitroll != pet->pIndexData->hitroll)
    	fprintf(fp, "Hit  %d\n", pet->hitroll);
    if (pet->damroll != pet->pIndexData->damage[DICE_BONUS])
    	fprintf(fp, "Dam  %d\n", pet->damroll);
    fprintf(fp, "ACs  %d %d %d %d\n",
    	pet->armor[0],pet->armor[1],pet->armor[2],pet->armor[3]);
    fprintf(fp, "Attr %d %d %d %d %d %d\n",
    	pet->perm_stat[STAT_STR], pet->perm_stat[STAT_INT],
    	pet->perm_stat[STAT_WIS], pet->perm_stat[STAT_DEX],
    	pet->perm_stat[STAT_CON], pet->perm_stat[STAT_CHR]);
    fprintf(fp, "AMod %d %d %d %d %d %d\n",
    	pet->mod_stat[STAT_STR], pet->mod_stat[STAT_INT],
    	pet->mod_stat[STAT_WIS], pet->mod_stat[STAT_DEX],
    	pet->mod_stat[STAT_CON], pet->mod_stat[STAT_CHR]);
    
    for ( paf = pet->affected; paf != NULL; paf = paf->next )
    {
    	if (paf->type < 0 || paf->type >= MAX_SKILL || paf->type == gsn_doppelganger)
    	    continue;
    	    
    	fprintf(fp, "Affc '%s' %3d %3d %3d %3d %3d %10d\n",
    	    skill_table[paf->type].name,
    	    paf->where, paf->level, paf->duration, paf->modifier,paf->location,
    	    paf->bitvector);
    }
    
    fprintf(fp,"End\n");
    return;
}
    
/*
 * Write an object and its contents.
 */
void fwrite_obj( CHAR_DATA *ch, OBJ_DATA *obj, FILE *fp, int iNest )
{
    EXTRA_DESCR_DATA *ed;
    AFFECT_DATA *paf;

    /*
     * Slick recursion to write lists backwards,
     *   so loading them will load in forwards order.
     */
    if ( obj->next_content != NULL )
	fwrite_obj( ch, obj->next_content, fp, iNest );

    /*
     * Castrate storage characters.
     */
    if ( (ch->level < obj->level - 20 && obj->item_type != ITEM_CONTAINER)
    ||   obj->item_type == ITEM_KEY )
	return;

    fprintf( fp, "#O\n" );
    fprintf( fp, "Vnum %d\n",   obj->pIndexData->vnum        );
    if (!obj->pIndexData->new_format)
	fprintf( fp, "Oldstyle\n");
    if (obj->enchanted)
	fprintf( fp,"Enchanted\n");
    fprintf( fp, "Nest %d\n",	iNest	  	     );

    /* these data are only used if they do not match the defaults */

    if ( obj->name != obj->pIndexData->name)
    	fprintf( fp, "Name %s~\n",	obj->name		     );
    if ( obj->short_descr != obj->pIndexData->short_descr)
        fprintf( fp, "ShD  %s~\n",	obj->short_descr	     );
    if ( obj->description != obj->pIndexData->description)
        fprintf( fp, "Desc %s~\n",	obj->description	     );
    if ( obj->extra_flags != obj->pIndexData->extra_flags)
        fprintf( fp, "ExtF %d\n",	obj->extra_flags	     );
    if ( obj->extra2_flags != obj->pIndexData->extra2_flags)
        fprintf( fp, "ExtF2 %d\n",	obj->extra2_flags	     );
    if ( obj->wear_flags != obj->pIndexData->wear_flags)
        fprintf( fp, "WeaF %d\n",	obj->wear_flags		     );
    if ( obj->item_type != obj->pIndexData->item_type)
        fprintf( fp, "Ityp %d\n",	obj->item_type		     );
    if ( obj->weight != obj->pIndexData->weight)
        fprintf( fp, "Wt   %d\n",	obj->weight		     );
    if ( obj->condition != 0 && obj->condition != obj->pIndexData->condition )
	fprintf( fp, "Cond %d\n",	obj->condition		     );
    else
	fprintf( fp, "Cond %d\n",	obj->pIndexData->condition   );

    if( obj->material != obj->pIndexData->material )
	fprintf( fp, "Mat  %s~\n",	obj->material	);

    /* variable data */

    fprintf( fp, "Wear %d\n",   obj->wear_loc                );
    if (obj->level != obj->pIndexData->level)
        fprintf( fp, "Lev  %d\n",	obj->level		     );
    if (obj->timer != 0)
        fprintf( fp, "Time %d\n",	obj->timer	     );
    fprintf( fp, "Cost %d\n",	obj->cost		     );
    if (obj->value[0] != obj->pIndexData->value[0]
    ||  obj->value[1] != obj->pIndexData->value[1]
    ||  obj->value[2] != obj->pIndexData->value[2]
    ||  obj->value[3] != obj->pIndexData->value[3]
    ||  obj->value[4] != obj->pIndexData->value[4]) 
    	fprintf( fp, "Val  %d %d %d %d %d\n",
	    obj->value[0], obj->value[1], obj->value[2], obj->value[3],
	    obj->value[4]	     );

    switch ( obj->item_type )
    {
    case ITEM_POTION:
    case ITEM_SCROLL:
    case ITEM_PILL:
	if ( obj->value[1] > 0 )
	{
	    fprintf( fp, "Spell 1 '%s'\n", 
		skill_table[obj->value[1]].name );
	}

	if ( obj->value[2] > 0 )
	{
	    fprintf( fp, "Spell 2 '%s'\n", 
		skill_table[obj->value[2]].name );
	}

	if ( obj->value[3] > 0 )
	{
	    fprintf( fp, "Spell 3 '%s'\n", 
		skill_table[obj->value[3]].name );
	}

	break;

    case ITEM_STAFF:
    case ITEM_WAND:
	if ( obj->value[3] > 0 )
	{
	    fprintf( fp, "Spell 3 '%s'\n", 
		skill_table[obj->value[3]].name );
	}

	break;
    }

    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
	if (paf->type < 0 || paf->type >= MAX_SKILL)
	    continue;
        fprintf( fp, "Affc '%s' %3d %3d %3d %3d %3d %10d\n",
            skill_table[paf->type].name,
            paf->where,
            paf->level,
            paf->duration,
            paf->modifier,
            paf->location,
            paf->bitvector
            );
    }

    for ( ed = obj->extra_descr; ed != NULL; ed = ed->next )
    {
	fprintf( fp, "ExDe %s~ %s~\n",
	    ed->keyword, ed->description );
    }

    fprintf( fp, "End\n\n" );

    if ( obj->contains != NULL )
	fwrite_obj( ch, obj->contains, fp, iNest + 1 );

    return;
}



/*
 * Load a char and inventory into a new ch structure.
 */
bool load_char_obj( DESCRIPTOR_DATA *d, char *name )
{
    char strsave[MAX_INPUT_LENGTH];
    char buf[100];
    CHAR_DATA *ch;
    FILE *fp;
    bool found;
    int stat;
    int skill=-1;

    ch = new_char();
    ch->pcdata = new_pcdata();

    d->character				= ch;
    ch->desc				= d;
    ch->name				= str_dup( name );
    ch->surname				= str_dup( "" );
    ch->maskedip				= str_dup( "" );
    ch->pcdata->lokprot			= str_dup( "" );
    ch->id					= get_pc_id();
    ch->race				= race_lookup("human");
    ch->comm				= COMM_COMBINE | COMM_PROMPT;
    ch->prompt 				= str_dup("<%hhp %mm %vmv> ");
    ch->wimpy			 	= 0;
    ch->yield				= 0;
    ch->pcdata->confirm_delete	= FALSE;
    ch->pcdata->pwd			= str_dup( "" );
    ch->pcdata->bamfin			= str_dup( "" );
    ch->pcdata->bamfout			= str_dup( "" );
    ch->pcdata->hometown		= 0;
    ch->pcdata->smite			= str_dup( "" );
    ch->pcdata->fadein			= str_dup( "" );
    ch->pcdata->fadeout			= str_dup( "" );
    ch->pcdata->transin			= str_dup( "" );
    ch->pcdata->transout		= str_dup( "" );
    ch->pcdata->title			= str_dup( "" );
    ch->pcdata->pretit 			= str_dup( "" ); 
    ch->pcdata->email 			= str_dup( "" );
    ch->cpose                       = NULL;
    ch->deathstat                   = 0;
    ch->last_pc_fought              = str_dup("(None)");
    ch->recall_point                = 0;
    for (stat =0; stat < MAX_STATS; stat++)
	ch->perm_stat[stat]		= 13;
    ch->pcdata->condition[COND_THIRST]	= 65; 
    ch->pcdata->condition[COND_FULL]	= 65;
    ch->pcdata->security		= 0;	/* OLC */
    ch->pcdata->text[0]		      = ( NORMAL );
    ch->pcdata->text[1]			= ( WHITE );
    ch->pcdata->text[2]			= 0;
    ch->pcdata->auction[0]		= ( NORMAL );
    ch->pcdata->auction[1]		= ( YELLOW );
    ch->pcdata->auction[2]		= 0;
    ch->pcdata->auction_text[0]	= ( NORMAL );
    ch->pcdata->auction_text[1]	= ( YELLOW );
    ch->pcdata->auction_text[2]	= 0;
    ch->pcdata->gossip[0]		= ( NORMAL );
    ch->pcdata->gossip[1]		= ( MAGENTA );
    ch->pcdata->gossip[2]		= 0;
    ch->pcdata->gossip_text[0]	= ( NORMAL );
    ch->pcdata->gossip_text[1]	= ( MAGENTA );
    ch->pcdata->gossip_text[2]	= 0;
    ch->pcdata->music[0]		= ( NORMAL );
    ch->pcdata->music[1]		= ( RED );
    ch->pcdata->music[2]		= 0;
    ch->pcdata->music_text[0]		= ( NORMAL );
    ch->pcdata->music_text[1]		= ( RED );
    ch->pcdata->music_text[2]		= 0;
    ch->pcdata->ask[0]			= ( BRIGHT );
    ch->pcdata->ask[1]			= ( YELLOW );
    ch->pcdata->ask[2]			= 0;
    ch->pcdata->ask_text[0] 		= ( BRIGHT );
    ch->pcdata->ask_text[1] 		= ( YELLOW );
    ch->pcdata->ask_text[2] 		= 0;
    ch->pcdata->answer[0]		= ( BRIGHT );
    ch->pcdata->answer[1]		= ( GREEN );
    ch->pcdata->answer[2]		= 0;
    ch->pcdata->answer_text[0]	= ( BRIGHT );
    ch->pcdata->answer_text[1]	= ( GREEN );
    ch->pcdata->answer_text[2]	= 0;
    ch->pcdata->quote[0]		= ( NORMAL );
    ch->pcdata->quote[1]		= ( GREEN );
    ch->pcdata->quote[2]		= 0;
    ch->pcdata->quote_text[0]		= ( NORMAL );
    ch->pcdata->quote_text[1]		= ( GREEN );
    ch->pcdata->quote_text[2]		= 0;
    ch->pcdata->immtalk_text[0]	= ( NORMAL );
    ch->pcdata->immtalk_text[1]	= ( CYAN );
    ch->pcdata->immtalk_text[2]	= 0;
    ch->pcdata->immtalk_type[0]	= ( NORMAL );
    ch->pcdata->immtalk_type[1]	= ( CYAN );
    ch->pcdata->immtalk_type[2]	= 0;
    ch->pcdata->info[0]			= ( BRIGHT );
    ch->pcdata->info[1]			= ( YELLOW );
    ch->pcdata->info[2]			= 1;
    ch->pcdata->say[0]			= ( NORMAL );
    ch->pcdata->say[1]			= ( GREEN );
    ch->pcdata->say[2]			= 0;
    ch->pcdata->say_text[0]		= ( NORMAL );
    ch->pcdata->say_text[1]		= ( GREEN );
    ch->pcdata->say_text[2]		= 0;
    ch->pcdata->tell[0]			= ( NORMAL );
    ch->pcdata->tell[1]			= ( YELLOW );
    ch->pcdata->tell[2]			= 0;
    ch->pcdata->tell_text[0]		= ( NORMAL );
    ch->pcdata->tell_text[1]		= ( YELLOW );
    ch->pcdata->tell_text[2]		= 0;
    ch->pcdata->reply[0]		= ( NORMAL );
    ch->pcdata->reply[1]		= ( YELLOW );
    ch->pcdata->reply[2]		= 0;
    ch->pcdata->reply_text[0]		= ( NORMAL );
    ch->pcdata->reply_text[1]		= ( YELLOW );
    ch->pcdata->reply_text[2]		= 0;
    ch->pcdata->gtell_text[0]		= ( NORMAL );
    ch->pcdata->gtell_text[1]		= ( BLUE );
    ch->pcdata->gtell_text[2]		= 0;
    ch->pcdata->gtell_type[0]		= ( NORMAL );
    ch->pcdata->gtell_type[1]		= ( BLUE );
    ch->pcdata->gtell_type[2]		= 0;
    ch->pcdata->wiznet[0]		= ( NORMAL );
    ch->pcdata->wiznet[1]		= ( GREEN );
    ch->pcdata->wiznet[2]		= 0;
    ch->pcdata->room_title[0]		= ( BRIGHT );
    ch->pcdata->room_title[1]		= ( GREEN );
    ch->pcdata->room_title[2]		= 0;
    ch->pcdata->room_text[0]		= ( NORMAL );
    ch->pcdata->room_text[1]		= ( WHITE );
    ch->pcdata->room_text[2]		= 0;
    ch->pcdata->room_exits[0]		= ( NORMAL );
    ch->pcdata->room_exits[1]		= ( GREEN );
    ch->pcdata->room_exits[2]		= 0;
    ch->pcdata->room_things[0]	= ( NORMAL );
    ch->pcdata->room_things[1]	= ( WHITE );
    ch->pcdata->room_things[2]	= 0;
    ch->pcdata->prompt[0]		= ( NORMAL );
    ch->pcdata->prompt[1]		= ( RED );
    ch->pcdata->prompt[2]		= 0;
    ch->pcdata->fight_death[0]	= ( NORMAL );
    ch->pcdata->fight_death[1]	= ( WHITE );
    ch->pcdata->fight_death[2]	= 0;
    ch->pcdata->fight_yhit[0]		= ( NORMAL );
    ch->pcdata->fight_yhit[1]		= ( WHITE );
    ch->pcdata->fight_yhit[2]		= 0;
    ch->pcdata->fight_ohit[0]		= ( NORMAL );
    ch->pcdata->fight_ohit[1]		= ( WHITE );
    ch->pcdata->fight_ohit[2]		= 0;
    ch->pcdata->fight_thit[0]		= ( NORMAL );
    ch->pcdata->fight_thit[1]		= ( WHITE );
    ch->pcdata->fight_thit[2]		= 0;
    ch->pcdata->fight_skill[0]	= ( BRIGHT );
    ch->pcdata->fight_skill[1]	= ( BLUE );
    ch->pcdata->fight_skill[2]	= 0;
/*  ch->pcdata->racetext        	= strdup("@"); //C030
    ch->pcdata->clantitle       	= strdup("@"); //C030*/
    ch->pcdata->racetext        	= NULL; //C039
    ch->pcdata->clantitle       	= NULL; //C039
    ch->pcdata->deaths			= 0;

    found = FALSE;
    fclose( fpReserve );
    
    #if defined(unix)
    /* decompress if .gz file exists */
    sprintf( strsave, "%s%s%s", PLAYER_DIR, capitalize(name),".gz");
    if ( ( fp = fopen( strsave, "r" ) ) != NULL )
    {
	fclose(fp);
	sprintf(buf,"gzip -dfq %s",strsave);
	system(buf);
    }
    #endif

    sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( name ) );
    if ( ( fp = fopen( strsave, "r" ) ) != NULL )
    {
	int iNest;

	for ( iNest = 0; iNest < MAX_NEST; iNest++ )
	    rgObjNest[iNest] = NULL;

	found = TRUE;
	for ( ; ; )
	{
	    char letter;
	    char *word;

	    letter = fread_letter( fp );
	    if ( letter == '*' )
	    {
		fread_to_eol( fp );
		continue;
	    }

	    if ( letter != '#' )
	    {
		bug( "Load_char_obj: # not found.", 0 );
		break;
	    }

	    word = fread_word( fp );
	    if      ( !str_cmp( word, "PLAYER" ) ) fread_char ( ch, fp );
	    else if ( !str_cmp( word, "OBJECT" ) ) fread_obj  ( ch, fp );
	    else if ( !str_cmp( word, "O"      ) ) fread_obj  ( ch, fp );
	    else if ( !str_cmp( word, "PET"    ) ) fread_pet  ( ch, fp );
	    else if ( !str_cmp( word, "END"    ) ) break;
	    else
	    {
		bug( "Load_char_obj: bad section.", 0 );
		break;
	    }
	}
	fclose( fp );
    }

    fpReserve = fopen( NULL_FILE, "r" );


    /* initialize race */
    if (found)
    {
	int i;

	if (ch->race == 0)
	    ch->race = race_lookup("human");

	ch->size = pc_race_table[ch->race].size;
	ch->dam_type = 17; /*punch */

	for (i = 0; i < 5; i++)
	{
	    if (pc_race_table[ch->race].skills[i] == NULL)
		break;
	}
	ch->affected_by 	= ch->affected_by|race_table[ch->race].aff;
	ch->imm_flags	= ch->imm_flags | race_table[ch->race].imm;
	ch->res_flags	= ch->res_flags | race_table[ch->race].res;
	ch->vuln_flags	= ch->vuln_flags | race_table[ch->race].vuln;
	ch->form		= race_table[ch->race].form;
	ch->parts		= race_table[ch->race].parts;
    }

	
    /* RT initialize skills */

    if (found && ch->version < 2)  /* need to add the new skills */
    {
	ch->pcdata->learned[gsn_recall] = 50;
    }
 
    /* fix levels */
    if (found && ch->version < 3 && (ch->level > 35 || ch->trust > 35))
    {
	switch (ch->level)
	{
	    case(40) : ch->level = 110;	break;  /* imp -> imp */
	    case(39) : ch->level = 108; break;	/* god -> supreme*/
	    case(38) : ch->level = 106; break;	/* deity -> god */
	    case(37) : ch->level = 103; break;	/* angel -> demigod */
	}

        switch (ch->trust)
        {
            case(40) : ch->trust = 110;  break;	/* imp -> imp */
            case(39) : ch->trust = 108;  break;	/* god -> supreme */
            case(38) : ch->trust = 106;  break;	/* deity -> god */
            case(37) : ch->trust = 103;  break;	/* angel -> demigod */
            case(36) : ch->trust = 101;  break;	/* hero -> hero */
        }
    }

    /* ream steel */
    if (found && ch->version < 4)
    {
	ch->steel   /= 100;
    }

    if ( found && ch->version <6)
    {
        if(strcmp(pc_race_table[ch->race].name,"gnome")==0)
        {
            skill=skill_lookup("gnomish");
        }
        else
        {
            if(strcmp(pc_race_table[ch->race].name,"half-elf")==0)
            {
                skill=skill_lookup("elven");
            }
            else
            {
                skill=skill_lookup(pc_race_table[ch->race].name);
            }
        }

        if (skill>0)
        {
            ch->pcdata->learned[skill]=100;
            //logpf("Just updated %s's race lang skill %d to 100",ch->name, skill);
        }

    }

    //C028
    if ( found && ch->version <8)
    {
        ch->pcdata->racetext = "@";
    }
    //C028
    if ( found && ch->version <9)
    {
        ch->pcdata->clantitle = "@";
    }

    //C039
    if ( found && ch->version <10)
    {

        if (!str_cmp(ch->pcdata->clantitle,"@"))
            ch->pcdata->clantitle=NULL;
        if (!str_cmp(ch->pcdata->racetext,"@"))
            ch->pcdata->racetext=NULL;
    }

    return found;
}



/*
 * Read in a char.
 */

#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )					\
				if ( !str_cmp( word, literal ) )	\
				{					\
				    field  = value;			\
				    fMatch = TRUE;			\
				    break;				\
				}

/* provided to free strings */
#if defined(KEYS)
#undef KEYS
#endif

#define KEYS( literal, field, value )					\
				if ( !str_cmp( word, literal ) )	\
				{					\
				    free_string(field);			\
				    field  = value;			\
				    fMatch = TRUE;			\
				    break;				\
				}

/* provided to free mem leaked with lookup routines */
/* -- Seronis */
#if defined(KEYL)
#undef KEYL
#endif

#define KEYL( literal, field, value, lookup ) \
if ( !str_cmp( word, literal ) ) \
{ \
char * temp = value; \
field = lookup(temp); \
free_string(temp); \
fMatch = TRUE; \
break; \
}

void fread_char( CHAR_DATA *ch, FILE *fp )
{
    char buf[MAX_STRING_LENGTH];
    char *word;
    bool fMatch;
    int x,y;
    int count = 0;
    int lastlogoff = current_time;
    int percent;
    ch->fight_pos = FIGHT_FRONT; /* MGD */

    sprintf(buf,"Loading %s.",ch->name);
    log_string(buf);

    for ( ; ; )
    {
	word   = feof( fp ) ? "End" : fread_word( fp );
	fMatch = FALSE;

	switch ( UPPER(word[0]) )
	{
	case '*':
	    fMatch = TRUE;
	    fread_to_eol( fp );
	    break;

	case 'A':
	    KEY( "Act",		ch->act,			fread_flag( fp ) );
	    KEY( "Act2",		ch->act2,			fread_flag( fp ) );
	    KEY( "AffectedBy",	ch->affected_by,		fread_flag( fp ) );
	    KEY( "AfBy",		ch->affected_by,		fread_flag( fp ) );
	    KEY( "AffectedBy2",	ch->affected2_by,		fread_flag( fp ) );
	    KEY( "AfBy2",		ch->affected2_by,		fread_flag( fp ) );
	    KEY( "AffectedBy3",	ch->affected3_by,		fread_flag( fp ) );
	    KEY( "AfBy3",		ch->affected3_by,		fread_flag( fp ) );
	    KEY( "Alignment",	ch->alignment,		fread_number( fp ) );
	    KEY( "Alig",		ch->alignment,		fread_number( fp ) );

	    if (!str_cmp( word, "Alia"))
	    {
		if (count >= MAX_ALIAS)
		{
		    fread_to_eol(fp);
		    fMatch = TRUE;
		    break;
		}

		ch->pcdata->alias[count] 	= str_dup(fread_word(fp));
		ch->pcdata->alias_sub[count]	= str_dup(fread_word(fp));
		count++;
		fMatch = TRUE;
		break;
	    }

            if (!str_cmp( word, "Alias"))
            {
                if (count >= MAX_ALIAS)
                {
                    fread_to_eol(fp);
                    fMatch = TRUE;
                    break;
                }
 
                ch->pcdata->alias[count]        = str_dup(fread_word(fp));
                ch->pcdata->alias_sub[count]    = fread_string(fp);
                count++;
                fMatch = TRUE;
                break;
            }

	    if (!str_cmp( word, "AC") || !str_cmp(word,"Armor"))
	    {
		fread_to_eol(fp);
		fMatch = TRUE;
		break;
	    }

	    if (!str_cmp(word,"ACs"))
	    {
		int i;

		for (i = 0; i < 4; i++)
		    ch->armor[i] = fread_number(fp);
		fMatch = TRUE;
		break;
	    }

	    if (!str_cmp(word, "AffD"))
	    {
		AFFECT_DATA *paf;
		int sn;

		paf = new_affect();

		sn = skill_lookup(fread_word(fp));
		if (sn < 0)
		    bug("Fread_char: unknown skill.",0);
		else
		    paf->type = sn;

		paf->level		= fread_number( fp );
		paf->duration	= fread_number( fp );
		paf->modifier	= fread_number( fp );
		paf->location	= fread_number( fp );
		paf->bitvector	= fread_number( fp );
		paf->next		= ch->affected;
		ch->affected	= paf;
		fMatch = TRUE;
		break;
	    }

            if (!str_cmp(word, "Affc"))
            {
                AFFECT_DATA *paf;
                int sn;
 
                paf = new_affect();
 
                sn = skill_lookup(fread_word(fp));
                if (sn < 0)
                    bug("Fread_char: unknown skill.",0);
                else
                    paf->type = sn;
 
                paf->where  	= fread_number(fp);
                paf->level      = fread_number( fp );
                paf->duration   = fread_number( fp );
                paf->modifier   = fread_number( fp );
                paf->location   = fread_number( fp );
                paf->bitvector  = fread_number( fp );
                paf->next       = ch->affected;
                ch->affected    = paf;
                fMatch = TRUE;
                break;
            }

	    if ( !str_cmp( word, "AttrMod"  ) || !str_cmp(word,"AMod"))
	    {
		int stat;
		for (stat = 0; stat < MAX_STATS; stat ++)
		   ch->mod_stat[stat] = fread_number(fp);
		fMatch = TRUE;
		break;
	    }

	    if ( !str_cmp( word, "AttrPerm" ) || !str_cmp(word,"Attr"))
	    {
		int stat;

		for (stat = 0; stat < MAX_STATS; stat++)
		    ch->perm_stat[stat] = fread_number(fp);
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'B':
	    KEY( "Background",	ch->background,		fread_string( fp ) );
	    KEY( "Backg",		ch->background,		fread_string( fp ) );
	    KEY( "Bamfin",	ch->pcdata->bamfin,	fread_string( fp ) );
	    KEY( "Bamfout",	ch->pcdata->bamfout,	fread_string( fp ) );
	    KEY( "Bank",	ch->bank,		fread_number( fp ) );
	    KEY( "Bin",		ch->pcdata->bamfin,	fread_string( fp ) );
	    KEY( "Bout",	ch->pcdata->bamfout,	fread_string( fp ) );
	    break;

	case 'C':
	    KEY( "Class",	ch->class,		fread_number( fp ) );
	    KEY( "Cla",	ch->class,		fread_number( fp ) );
	    KEY( "Clan",	ch->clan_id,	fread_number( fp ) );
          if (!str_cmp(word,"ClanTitle"))
          {
                KEYS( "ClanTitle",ch->pcdata->clantitle,fread_string( fp ) );
		fMatch = TRUE;
		break;
          }

          KEY( "Comm",	ch->comm,		fread_flag( fp ) );

	    if ( !str_cmp( word, "Condition" ) || !str_cmp(word,"Cond"))
	    {
		ch->pcdata->condition[0] = fread_number( fp );
		ch->pcdata->condition[1] = fread_number( fp );
		ch->pcdata->condition[2] = fread_number( fp );
		fMatch = TRUE;
		break;
	    }
            if (!str_cmp(word,"Cnd"))
            {
                ch->pcdata->condition[0] = fread_number( fp );
                ch->pcdata->condition[1] = fread_number( fp );
                ch->pcdata->condition[2] = fread_number( fp );
		ch->pcdata->condition[3] = fread_number( fp );
                fMatch = TRUE;
                break;
            }
	    
		if( !str_cmp( word, "Coloura" ) )
	    {
		LOAD_COLOUR( text )
		LOAD_COLOUR( auction )
		LOAD_COLOUR( gossip )
		LOAD_COLOUR( music )
		LOAD_COLOUR( ask )
		fMatch = TRUE;
		break;
	    }
	    if( !str_cmp( word, "Colourb" ) )
	    {
		LOAD_COLOUR( answer )
		LOAD_COLOUR( quote )
		LOAD_COLOUR( quote_text )
		LOAD_COLOUR( immtalk_text )
		LOAD_COLOUR( immtalk_type )
		fMatch = TRUE;
		break;
	    }
	    if( !str_cmp( word, "Colourc" ) )
	    {
		LOAD_COLOUR( info )
		LOAD_COLOUR( tell )
		LOAD_COLOUR( reply )
		LOAD_COLOUR( gtell_text )
		LOAD_COLOUR( gtell_type )
		fMatch = TRUE;
		break;
	    }
	    if( !str_cmp( word, "Colourd" ) )
	    {
		LOAD_COLOUR( room_title )
		LOAD_COLOUR( room_text )
		LOAD_COLOUR( room_exits )
		LOAD_COLOUR( room_things )
		LOAD_COLOUR( prompt )
		fMatch = TRUE;
		break;
	    }
	    if( !str_cmp( word, "Coloure" ) )
	    {
		LOAD_COLOUR( fight_death )
		LOAD_COLOUR( fight_yhit )
		LOAD_COLOUR( fight_ohit )
		LOAD_COLOUR( fight_thit )
		LOAD_COLOUR( fight_skill )
		fMatch = TRUE;
		break;
	    }
	    if( !str_cmp( word, "Colourf" ) )
	    {
		LOAD_COLOUR( wiznet )
		LOAD_COLOUR( say )
		LOAD_COLOUR( say_text )
		LOAD_COLOUR( tell_text )
		LOAD_COLOUR( reply_text )
		fMatch = TRUE;
		break;
	    }
	    if( !str_cmp( word, "Colourg" ) )
	    {
		LOAD_COLOUR( auction_text )
		LOAD_COLOUR( gossip_text )
		LOAD_COLOUR( music_text )
		LOAD_COLOUR( ask_text )
		LOAD_COLOUR( answer_text )
		fMatch = TRUE;
		break;
	    }
          
	    break;

	case 'D':
	    KEY( "Damroll",	ch->damroll,		fread_number( fp ) );
	    KEY( "Dam",		ch->damroll,		fread_number( fp ) );
	    KEY( "Description",	ch->description,		fread_string( fp ) );
	    KEY( "Deaths",	ch->pcdata->deaths,	fread_number( fp ) );
          KEY( "Deathstat",   ch->deathstat,          fread_number( fp ) );
	    KEY( "Desc",		ch->description,		fread_string( fp ) );
          KEY( "Devotee",	ch->devotee_id,	      fread_number( fp ) );
	    break;

	case 'E':
	    KEYS( "email",	ch->pcdata->email,	fread_string( fp ) );
	    KEY ( "Ethos",	ch->ethos,			fread_number( fp ) );
	    if ( !str_cmp( word, "End" ) )
	    {
    		/* adjust hp mana move up  -- here for speed's sake */
    		percent = (current_time - lastlogoff) * 25 / ( 2 * 60 * 60);

		percent = UMIN(percent,100);
 
    		if (percent > 0 && !IS_AFFECTED(ch,AFF_POISON)
    		&&  !IS_AFFECTED(ch,AFF_PLAGUE))
    		{
        	    ch->hit	+= (ch->max_hit - ch->hit) * percent / 100;
        	    ch->mana    += (ch->max_mana - ch->mana) * percent / 100;
        	    ch->move    += (ch->max_move - ch->move)* percent / 100;
    		}
		return;
	    }
	    KEY( "Exp",		ch->exp,		fread_number( fp ) );
	    break;

	case 'F':
          KEY( "Fin",		ch->pcdata->fadein,	fread_string( fp ) );
          KEY( "Fadein",	ch->pcdata->fadein,	fread_string( fp ) );
          KEY( "Fout",        ch->pcdata->fadeout,	fread_string( fp ) );
          KEY( "Fadeout",	ch->pcdata->fadeout,	fread_string( fp ) );
	    if ( !str_cmp( word, "FlagTot" ) )
	    {
	      for (x=0;x < 3; x++)
	        for (y=0;y < 4; y++)
		  ch->pcdata->flags[x][y]   =	fread_number( fp );  
	      fMatch = TRUE;
	      break;
	    }
	    if ( !str_cmp( word, "FParTot" ) )
	    {
	      for (x=0;x < 3; x++)
	        for (y=0;y < 4; y++)
		  ch->pcdata->pardons[x][y]  =	fread_number( fp );  
	      fMatch = TRUE;
	      break;
	    }
	    break;

	case 'G':
            KEY( "Gold",        ch->gold,             fread_number( fp ) );
	    break;

	case 'H':
	    KEY( "Highpriest",	ch->pcdata->highpriest,	fread_number( fp ) );
	    KEY( "Hitroll",	ch->hitroll,		fread_number( fp ) );
	    KEY( "Hit",		ch->hitroll,		fread_number( fp ) );
	    KEY( "Hmtown",	ch->hometown,		fread_number( fp ) );

	    if ( !str_cmp( word, "HpManaMove" ) || !str_cmp(word,"HMV"))
	    {
		ch->hit		= fread_number( fp );
		ch->max_hit	= fread_number( fp );
		ch->mana	= fread_number( fp );
		ch->max_mana	= fread_number( fp );
		ch->move	= fread_number( fp );
		ch->max_move	= fread_number( fp );
		fMatch = TRUE;
		break;
	    }

            if ( !str_cmp( word, "HpManaMovePerm" ) || !str_cmp(word,"HMVP"))
            {
                ch->pcdata->perm_hit	= fread_number( fp );
                ch->pcdata->perm_mana   = fread_number( fp );
                ch->pcdata->perm_move   = fread_number( fp );
                fMatch = TRUE;
                break;
            }
      
	    break;

	case 'I':
	    KEY( "Id",		ch->id,			fread_number( fp ) );
	    KEY( "InvisLevel",	ch->invis_level,		fread_number( fp ) );
	    KEY( "Inco",		ch->incog_level,		fread_number( fp ) );
	    KEY( "Invi",		ch->invis_level,		fread_number( fp ) );
	    break;

	case 'L':
	    KEY( "LastLevel",	ch->pcdata->last_level, fread_number(fp) );
//        KEY( "Language",	ch->language,		fread_number( fp ) );
	    KEY( "LLev",		ch->pcdata->last_level, fread_number( fp ) );
	    KEY( "Level",		ch->level,			fread_number( fp ) );
	    KEY( "Lev",		ch->level,			fread_number( fp ) );
	    KEY( "Levl",		ch->level,			fread_number( fp ) );
	    KEY( "Levscale",	ch->level_scale,		fread_number( fp ) );
	    KEY( "LocationHP",	ch->loc_hp,			fread_flag( fp ) );
	    KEY( "LocationHP2",	ch->loc_hp2,		fread_flag( fp ) );
	    KEY( "LogO",		lastlogoff,			fread_number( fp ) );
	    KEY( "LongDescr",	ch->long_descr,		fread_string( fp ) );
	    KEY( "LnD",		ch->long_descr,		fread_string( fp ) );
	    KEY( "Lokprot",	ch->pcdata->lokprot,	fread_string( fp ) );
	    break;

      case 'M':
          KEY( "MaskeIp",	ch->maskedip,		fread_string( fp ) );

	case 'N':
	    KEYS( "Name",	ch->name,		fread_string( fp ) );
	    KEY( "Note",	ch->pcdata->last_note,	fread_number( fp ) );
	    if (!str_cmp(word,"Not"))
	    {
		ch->pcdata->last_note			= fread_number(fp);
		ch->pcdata->last_idea			= fread_number(fp);
		ch->pcdata->last_penalty		= fread_number(fp);
		ch->pcdata->last_news			= fread_number(fp);
		ch->pcdata->last_changes		= fread_number(fp);
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'O':
	    KEY( "Org",		ch->org_id,		fread_number( fp ) );

	case 'P':
	    KEY( "Password",	ch->pcdata->pwd,		fread_string( fp ) );
	    KEY( "Pass",		ch->pcdata->pwd,		fread_string( fp ) );
	    KEY( "Penal_timer",	ch->penalize_timer,	fread_number( fp ) );
	    KEY( "Played",	ch->played,			fread_number( fp ) );
	    KEY( "Plyd",		ch->played,			fread_number( fp ) );
	    KEY( "Points",	ch->pcdata->points,	fread_number( fp ) );
	    KEY( "Pnts",		ch->pcdata->points,	fread_number( fp ) );
	    KEY( "Position",	ch->position,		fread_number( fp ) );
	    KEY( "Pos",		ch->position,		fread_number( fp ) );
	    KEY( "Practice",	ch->practice,		fread_number( fp ) );
	    KEY( "Prac",		ch->practice,		fread_number( fp ) );
	    if ( !str_cmp( word, "Ptit" ) || !str_cmp( word, "pretit")) 
 	    {
  		ch->pcdata->pretit = fread_string( fp );

   		if (ch->pcdata->pretit[0] != '.' && ch->pcdata->pretit[0] != ','
		&&  ch->pcdata->pretit[0] != '!' && ch->pcdata->pretit[0] != '?')
   		  {
   		  sprintf( buf, "%s", ch->pcdata->pretit );
   		  free_string( ch->pcdata->pretit );
   		  ch->pcdata->pretit = str_dup( buf );
   		  }
 		fMatch = TRUE;
 		break;
	    } 
          KEYS("Prompt",      ch->prompt,             fread_string( fp ) );
 	    KEYS("Prom",		ch->prompt,			fread_string( fp ) );
	    break;

      case 'Q':
          KEY( "QuestPnts",   ch->questpoints,        fread_number( fp ) );
          KEY( "QuestNext",   ch->nextquest,          fread_number( fp ) );
          break;

	case 'R':
          KEY( "Religion",	 ch->religion_id,	      fread_number( fp ) );
          KEY( "Recall_point", ch->recall_point,      fread_number( fp) );
	    KEY( "Reward_timer", ch->reward_timer,	fread_number( fp ) );

	    if ( !str_cmp( word, "Race" ) )
          {
     		char *tmp = fread_string(fp);
     		ch->race = race_lookup(tmp);
     		free_string(tmp);
     		fMatch = TRUE;
     		break;
	    }
          KEY( "Rank",        ch->rank, fread_number( fp ) );
            //C028
            if (!str_cmp(word,"RaceText"))
            {
/*              if (ch->version>7)
                {
                    logpf("Getting RaceText");
                    KEYS( "RaceText",ch->pcdata->racetext,fread_string( fp ) );
                }
*/
                //C039
                KEYS( "RaceText",ch->pcdata->racetext,fread_string( fp ) );
		fMatch = TRUE;
		break;
            }

	    if ( !str_cmp( word, "Room" ) )
	    {
		ch->in_room = get_room_index( fread_number( fp ) );
		if ( ch->in_room == NULL )
		    ch->in_room = get_room_index( ROOM_VNUM_LIMBO );
		fMatch = TRUE;
		break;
	    }

	    break;

	case 'S':
	    KEY( "SavingThrow",	ch->saving_throw,		fread_number( fp ) );
	    KEY( "Save",		ch->saving_throw,		fread_number( fp ) );
	    KEY( "Scro",		ch->lines,			fread_number( fp ) );
	    KEY( "Sex",		ch->sex,			fread_number( fp ) );
	    KEY( "ShortDescr",	ch->short_descr,		fread_string( fp ) );
	    KEY( "ShD",		ch->short_descr,		fread_string( fp ) );
	    KEY( "Sec",         ch->pcdata->security,	fread_number( fp ) );	/* OLC */
	    KEY( "Surname",	ch->surname,		fread_string( fp ) );
	    KEY( "Steel",		ch->steel,			fread_number( fp ) );
          KEY( "Smite",       ch->pcdata->smite,      fread_string( fp ) );

	    if ( !str_cmp( word, "Skill" ) || !str_cmp(word,"Sk"))
	    {
		int sn;
		int value;
		char *temp;

		value = fread_number( fp );
		temp = fread_word( fp ) ;
		sn = skill_lookup(temp);
		/* sn    = skill_lookup( fread_word( fp ) ); */
		if ( sn < 0 )
		{
		    fprintf(stderr,"%s",temp);
		    bug( "Fread_char: unknown skill. ", 0 );
		}
		else
		    ch->pcdata->learned[sn] = value;
		fMatch = TRUE;
	    }

	    break;

	case 'T':
	    KEY( "Time",	  	ch->quit_timer,		fread_number( fp ) );
          KEY( "TrueSex",     ch->pcdata->true_sex,  	fread_number( fp ) );
	    KEY( "TSex",		ch->pcdata->true_sex,   fread_number( fp ) );
	    KEY( "Trai",		ch->train,			fread_number( fp ) );
	    KEY( "Trust",		ch->trust,			fread_number( fp ) );
	    KEY( "Tru",		ch->trust,			fread_number( fp ) );
          KEY( "Tin",         ch->pcdata->transin,    fread_string( fp ) );
          KEY( "Transin",     ch->pcdata->transin,    fread_string( fp ) );
          KEY( "Tout",        ch->pcdata->transout,   fread_string( fp ) );
          KEY( "Transout",    ch->pcdata->transout,   fread_string( fp ) );

	    if ( !str_cmp( word, "Title" )  || !str_cmp( word, "Titl"))
	    {
		ch->pcdata->title = fread_string( fp );
    		if (ch->pcdata->title[0] != '.' && ch->pcdata->title[0] != ',' 
		&&  ch->pcdata->title[0] != '!' && ch->pcdata->title[0] != '?')
		{
		    sprintf( buf, " %s", ch->pcdata->title );
		    free_string( ch->pcdata->title );
		    ch->pcdata->title = str_dup( buf );
		}
		fMatch = TRUE;
		break;
	    }

	    break;

	case 'V':
	    KEY( "Version",     ch->version,		fread_number ( fp ) );
	    KEY( "Vers",	ch->version,		fread_number ( fp ) );
	    if ( !str_cmp( word, "Vnum" ) )
	    {
		ch->pIndexData = get_mob_index( fread_number( fp ) );
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'W':
	    KEY( "Wimpy",	ch->wimpy,		fread_number( fp ) );
	    KEY( "Wimp",	ch->wimpy,		fread_number( fp ) );
	    KEY( "Wizn",	ch->wiznet,		fread_flag( fp ) );
	    break;

	case 'X':
	    KEY( "Xptolev",	ch->xp_to_lev,		fread_number( fp ) );
	    break;

	case 'Y':
          KEY( "Yield",       ch->yield,              fread_number( fp ) );
          break; 
	}

	if ( !fMatch )
	{
	    /* Spam control */
	   /* bug( "Fread_char: no match.", 0 );
		bug( word, 0 ); */
	    fread_to_eol( fp );
	}
    }
}

/* load a pet from the forgotten reaches */
void fread_pet( CHAR_DATA *ch, FILE *fp )
{
    char *word;
    CHAR_DATA *pet;
    bool fMatch;
    int lastlogoff = current_time;
    int percent;

    /* first entry had BETTER be the vnum or we barf */
    word = feof(fp) ? "END" : fread_word(fp);
    if (!str_cmp(word,"Vnum"))
    {
    	long vnum;
    	
    	vnum = fread_number(fp);
    	if (get_mob_index(vnum) == NULL)
	{
    	    bug("Fread_pet: bad vnum %d.",vnum);
	    pet = create_mobile(get_mob_index(MOB_VNUM_FIDO));
	}
    	else
    	    pet = create_mobile(get_mob_index(vnum));
    }
    else
    {
        bug("Fread_pet: no vnum in file.",0);
        pet = create_mobile(get_mob_index(MOB_VNUM_FIDO));
    }
    
    for ( ; ; )
    {
    	word 	= feof(fp) ? "END" : fread_word(fp);
    	fMatch = FALSE;
    	
    	switch (UPPER(word[0]))
    	{
    	case '*':
    	    fMatch = TRUE;
    	    fread_to_eol(fp);
    	    break;
    		
    	case 'A':
    	    KEY( "Act",		pet->act,			fread_flag(fp));
    	    KEY( "Act2",		pet->act2,			fread_flag(fp));
    	    KEY( "AfBy",		pet->affected_by,		fread_flag(fp));
    	    KEY( "AfBy2",		pet->affected2_by,	fread_flag(fp));
    	    KEY( "AfBy3",		pet->affected3_by,	fread_flag(fp));
    	    KEY( "Alig",		pet->alignment,		fread_number(fp));
    	    
    	    if (!str_cmp(word,"ACs"))
    	    {
    	    	int i;
    	    	
    	    	for (i = 0; i < 4; i++)
    	    	    pet->armor[i] = fread_number(fp);
    	    	fMatch = TRUE;
    	    	break;
    	    }
    	    
    	    if (!str_cmp(word,"AffD"))
    	    {
    	    	AFFECT_DATA *paf;
    	    	int sn;
    	    	
    	    	paf = new_affect();
    	    	
    	    	sn = skill_lookup(fread_word(fp));
    	     	if (sn < 0)
    	     	    bug("Fread_char: unknown skill.",0);
    	     	else
    	     	   paf->type = sn;
    	     	   
    	     	paf->level		= fread_number(fp);
    	     	paf->duration	= fread_number(fp);
    	     	paf->modifier	= fread_number(fp);
    	     	paf->location	= fread_number(fp);
    	     	paf->bitvector	= fread_number(fp);
    	     	paf->next		= pet->affected;
    	     	pet->affected	= paf;
    	     	fMatch		= TRUE;
    	     	break;
    	    }

            if (!str_cmp(word,"Affc"))
            {
                AFFECT_DATA *paf;
                int sn;
 
                paf = new_affect();
 
                sn = skill_lookup(fread_word(fp));
                if (sn < 0)
                    bug("Fread_char: unknown skill.",0);
                else
                   paf->type = sn;
 
		    paf->where		= fread_number(fp);
                paf->level      	= fread_number(fp);
                paf->duration   	= fread_number(fp);
                paf->modifier   	= fread_number(fp);
                paf->location   	= fread_number(fp);
                paf->bitvector  	= fread_number(fp);
                paf->next       	= pet->affected;
                pet->affected   	= paf;
                fMatch          	= TRUE;
                break;
            }
    	     
    	    if (!str_cmp(word,"AMod"))
    	    {
    	     	int stat;
    	     	
    	     	for (stat = 0; stat < MAX_STATS; stat++)
    	     	    pet->mod_stat[stat] = fread_number(fp);
    	     	fMatch = TRUE;
    	     	break;
    	    }
    	     
    	    if (!str_cmp(word,"Attr"))
    	    {
    	         int stat;
    	         
    	         for (stat = 0; stat < MAX_STATS; stat++)
    	             pet->perm_stat[stat] = fread_number(fp);
    	         fMatch = TRUE;
    	         break;
    	    }
    	    break;
    	     
    	 case 'C':
    	     KEY( "Comm",	pet->comm,		fread_flag(fp));
    	     break;
    	     
    	 case 'D':
    	     KEY( "Dam",	pet->damroll,		fread_number(fp));
    	     KEY( "Desc",	pet->description,	fread_string(fp));
    	     break;
    	     
    	 case 'E':
    	     if (!str_cmp(word,"End"))
	     {
		pet->leader = ch;
		pet->master = ch;
		ch->pet = pet;
    		/* adjust hp mana move up  -- here for speed's sake */
    		percent = (current_time - lastlogoff) * 25 / ( 2 * 60 * 60);
 
    		if (percent > 0 && !IS_AFFECTED(ch,AFF_POISON)
    		&&  !IS_AFFECTED(ch,AFF_PLAGUE))
    		{
		    percent = UMIN(percent,100);
    		    pet->hit	+= (pet->max_hit - pet->hit) * percent / 100;
        	    pet->mana   += (pet->max_mana - pet->mana) * percent / 100;
        	    pet->move   += (pet->max_move - pet->move)* percent / 100;
    		}
    	     	return;
	     }
    	     KEY( "Exp",	pet->exp,		fread_number(fp));
    	     break;
    	     
    	 case 'G':
            KEY( "Gold",        pet->gold,            fread_number( fp ) );
    	     break;
    	     
    	 case 'H':
    	     KEY( "Hit",	pet->hitroll,		fread_number(fp));
    	     
    	     if (!str_cmp(word,"HMV"))
    	     {
    	     	pet->hit	= fread_number(fp);
    	     	pet->max_hit	= fread_number(fp);
    	     	pet->mana	= fread_number(fp);
    	     	pet->max_mana	= fread_number(fp);
    	     	pet->move	= fread_number(fp);
    	     	pet->max_move	= fread_number(fp);
    	     	fMatch = TRUE;
    	     	break;
    	     }
    	     break;
    	     
     	case 'L':
    	     KEY( "Levl",	pet->level,		fread_number(fp));
    	     KEY( "LnD",	pet->long_descr,	fread_string(fp));
	     KEY( "LogO",	lastlogoff,		fread_number(fp));
    	     break;
    	     
    	case 'N':
    	     KEY( "Name",	pet->name,		fread_string(fp));
    	     break;
    	     
    	case 'P':
    	     KEY( "Pos",	pet->position,		fread_number(fp));
    	     break;
    	     
	case 'R':
    	    KEY( "Race",	pet->race, race_lookup(fread_string(fp)));
    	    break;
 	    
    	case 'S' :
    	    KEY( "Save",	pet->saving_throw,	fread_number(fp));
    	    KEY( "Sex",		pet->sex,		fread_number(fp));
    	    KEY( "ShD",		pet->short_descr,	fread_string(fp));
    	     KEY( "Steel",	pet->steel,		fread_number(fp));
    	    break;
    	    
    	if ( !fMatch )
    	{
    	    bug("Fread_pet: no match.",0);
    	    fread_to_eol(fp);
    	}
    	
    	}
    }
}

extern	OBJ_DATA	*obj_free;

void fread_obj( CHAR_DATA *ch, FILE *fp )
{
    OBJ_DATA *obj;
    char *word;
    int iNest;
    bool fMatch;
    bool fNest;
    bool fVnum;
    bool first;
    bool new_format;  /* to prevent errors */
    bool make_new;    /* update object */
    
    fVnum = FALSE;
    obj = NULL;
    first = TRUE;  /* used to counter fp offset */
    new_format = FALSE;
    make_new = FALSE;

    word   = feof( fp ) ? "End" : fread_word( fp );
    if (!str_cmp(word,"Vnum" ))
    {
        long vnum;
	first = FALSE;  /* fp will be in right place */
 
        vnum = fread_number( fp );
        if (  get_obj_index( vnum )  == NULL )
	{
            bug( "Fread_obj: bad vnum %d.", vnum );
	}
        else
	{
	    obj = create_object(get_obj_index(vnum),-1);
	    new_format = TRUE;
	}
	    
    }

    if (obj == NULL)  /* either not found or old style */
    {
    	obj = new_obj();
    	obj->name		= str_dup( "" );
    	obj->short_descr	= str_dup( "" );
    	obj->description	= str_dup( "" );
    }

    fNest		= FALSE;
    fVnum		= TRUE;
    iNest		= 0;

    for ( ; ; )
    {
	if (first)
	    first = FALSE;
	else
	    word   = feof( fp ) ? "End" : fread_word( fp );
	fMatch = FALSE;

	switch ( UPPER(word[0]) )
	{
	case '*':
	    fMatch = TRUE;
	    fread_to_eol( fp );
	    break;

	case 'A':
	    if (!str_cmp(word,"AffD"))
	    {
		AFFECT_DATA *paf;
		int sn;

		paf = new_affect();

		sn = skill_lookup(fread_word(fp));
		if (sn < 0)
		    bug("Fread_obj: unknown skill.",0);
		else
		    paf->type = sn;

		paf->level		= fread_number( fp );
		paf->duration	= fread_number( fp );
		paf->modifier	= fread_number( fp );
		paf->location	= fread_number( fp );
		paf->bitvector	= fread_number( fp );
		paf->next		= obj->affected;
		obj->affected	= paf;
		fMatch		= TRUE;
		break;
	    }
            if (!str_cmp(word,"Affc"))
            {
                AFFECT_DATA *paf;
                int sn;
 
                paf = new_affect();
 
                sn = skill_lookup(fread_word(fp));
                if (sn < 0)
                    bug("Fread_obj: unknown skill.",0);
                else
                    paf->type = sn;
 
		paf->where	= fread_number( fp );
                paf->level      = fread_number( fp );
                paf->duration   = fread_number( fp );
                paf->modifier   = fread_number( fp );
                paf->location   = fread_number( fp );
                paf->bitvector  = fread_number( fp );
                paf->next       = obj->affected;
                obj->affected   = paf;
                fMatch          = TRUE;
                break;
            }
	    break;

	case 'C':
	    KEY( "Cond",	obj->condition,		fread_number( fp ) );
	    KEY( "Cost",	obj->cost,		fread_number( fp ) );
	    break;

	case 'D':
	    KEY( "Description",	obj->description,	fread_string( fp ) );
	    KEY( "Desc",	obj->description,	fread_string( fp ) );
	    break;

	case 'E':

	    if ( !str_cmp( word, "Enchanted"))
	    {
		obj->enchanted = TRUE;
	 	fMatch 	= TRUE;
		break;
	    }

	    KEY( "ExtraFlags",	obj->extra_flags,	fread_number( fp ) );
	    KEY( "ExtF",	obj->extra_flags,	fread_number( fp ) );
	    KEY( "ExtraFlags2",	obj->extra2_flags,	fread_number( fp ) );
	    KEY( "ExtF2",	obj->extra2_flags,	fread_number( fp ) );

	    if ( !str_cmp( word, "ExtraDescr" ) || !str_cmp(word,"ExDe"))
	    {
		EXTRA_DESCR_DATA *ed;

		ed = new_extra_descr();

		ed->keyword		= fread_string( fp );
		ed->description		= fread_string( fp );
		ed->next		= obj->extra_descr;
		obj->extra_descr	= ed;
		fMatch = TRUE;
	    }

	    if ( !str_cmp( word, "End" ) )
	    {
		if ( !fNest || ( fVnum && obj->pIndexData == NULL ) )
		{
		    bug( "Fread_obj: incomplete object.", 0 );
		    free_obj(obj);
		    return;
		}
		else
	        {
		    if ( !fVnum )
		    {
			free_obj( obj );
			obj = create_object( get_obj_index( OBJ_VNUM_DUMMY ), 0 );
		    }

		    if (!new_format)
		    {
		    	obj->next	= object_list;
		    	object_list	= obj;
		    	obj->pIndexData->count++;
		    }

		    if (!obj->pIndexData->new_format 
		    && obj->item_type == ITEM_ARMOR
		    &&  obj->value[1] == 0)
		    {
			obj->value[1] = obj->value[0];
			obj->value[2] = obj->value[0];
		    }
		    if (make_new)
		    {
			int wear;
			
			wear = obj->wear_loc;
			extract_obj(obj);

			obj = create_object(obj->pIndexData,0);
			obj->wear_loc = wear;
		    }
		    if ( iNest == 0 || rgObjNest[iNest] == NULL )
			obj_to_char( obj, ch );
		    else
			obj_to_obj( obj, rgObjNest[iNest-1] );

		    return;
		}
	    }
	    break;

	case 'I':
	    KEY( "ItemType",	obj->item_type,		fread_number( fp ) );
	    KEY( "Ityp",	obj->item_type,		fread_number( fp ) );
	    break;

	case 'L':
	    KEY( "Level",	obj->level,		fread_number( fp ) );
	    KEY( "Lev",		obj->level,		fread_number( fp ) );
	    break;

	case 'M':
	    KEYS( "Mat",	obj->material,	fread_string( fp ) );
	    break;

	case 'N':
	    KEY( "Name",	obj->name,		fread_string( fp ) );

	    if ( !str_cmp( word, "Nest" ) )
	    {
		iNest = fread_number( fp );
		if ( iNest < 0 || iNest >= MAX_NEST )
		{
		    bug( "Fread_obj: bad nest %d.", iNest );
		}
		else
		{
		    rgObjNest[iNest] = obj;
		    fNest = TRUE;
		}
		fMatch = TRUE;
	    }
	    break;

   	case 'O':
	    if ( !str_cmp( word,"Oldstyle" ) )
	    {
		if (obj->pIndexData != NULL && obj->pIndexData->new_format)
		    make_new = TRUE;
		fMatch = TRUE;
	    }
	    break;
		    

	case 'S':
	    KEY( "ShortDescr",	obj->short_descr,	fread_string( fp ) );
	    KEY( "ShD",		obj->short_descr,	fread_string( fp ) );

	    if ( !str_cmp( word, "Spell" ) )
	    {
		int iValue;
		int sn;

		iValue = fread_number( fp );
		sn     = skill_lookup( fread_word( fp ) );
		if ( iValue < 0 || iValue > 3 )
		{
		    bug( "Fread_obj: bad iValue %d.", iValue );
		}
		else if ( sn < 0 )
		{
		    bug( "Fread_obj: unknown skill.", 0 );
		}
		else
		{
		    obj->value[iValue] = sn;
		}
		fMatch = TRUE;
		break;
	    }

	    break;

	case 'T':
	    KEY( "Timer",	obj->timer,		fread_number( fp ) );
	    KEY( "Time",	obj->timer,		fread_number( fp ) );
	    break;

	case 'V':
	    if ( !str_cmp( word, "Values" ) || !str_cmp(word,"Vals"))
	    {
		obj->value[0]	= fread_number( fp );
		obj->value[1]	= fread_number( fp );
		obj->value[2]	= fread_number( fp );
		obj->value[3]	= fread_number( fp );
		if (obj->item_type == ITEM_WEAPON && obj->value[0] == 0)
		   obj->value[0] = obj->pIndexData->value[0];
		fMatch		= TRUE;
		break;
	    }

	    if ( !str_cmp( word, "Val" ) )
	    {
		obj->value[0] 	= fread_number( fp );
	 	obj->value[1]	= fread_number( fp );
	 	obj->value[2] 	= fread_number( fp );
		obj->value[3]	= fread_number( fp );
		obj->value[4]	= fread_number( fp );
		fMatch = TRUE;
		break;
	    }

	    if ( !str_cmp( word, "Vnum" ) )
	    {
		long vnum;

		vnum = fread_number( fp );
		if ( ( obj->pIndexData = get_obj_index( vnum ) ) == NULL )
		    bug( "Fread_obj: bad vnum %d.", vnum );
		else
		    fVnum = TRUE;
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'W':
	    KEY( "WearFlags",	obj->wear_flags,	fread_number( fp ) );
	    KEY( "WeaF",	obj->wear_flags,	fread_number( fp ) );
	    KEY( "WearLoc",	obj->wear_loc,		fread_number( fp ) );
	    KEY( "Wear",	obj->wear_loc,		fread_number( fp ) );
	    KEY( "Weight",	obj->weight,		fread_number( fp ) );
	    KEY( "Wt",		obj->weight,		fread_number( fp ) );
	    break;

	}

	if ( !fMatch )
	{
	    bug( "Fread_obj: no match.", 0 );
	    fread_to_eol( fp );
	}
    }
}

char *initial( const char *str )
{
    static char strint [ MAX_STRING_LENGTH ];

    strint[0] = LOWER( str[ 0 ] );
    return strint;

}
