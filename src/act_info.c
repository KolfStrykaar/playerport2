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
#include <time.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include "merc.h"
#include "interp.h"
#include "magic.h"
#include "recycle.h"
#include "tables.h"
#include "lookup.h"


char *	const	where_name	[] =
{
    "<used as light>     ",
    "<worn on finger>    ",
    "<worn on finger>    ",
    "<worn around neck>  ",
    "<worn around neck>  ",
    "<worn on torso>     ",
    "<worn on head>      ",
    "<worn on face>      ",
    "<worn on legs>      ",
    "<worn around ankle> ",
    "<worn around ankle> ",
    "<worn on feet>      ",
    "<worn on hands>     ",
    "<worn on arms>      ",
    "<worn on shoulders> ",
    "<worn as shield>    ",
    "<worn about body>   ",
    "<worn about waist>  ",
    "<worn around wrist> ",
    "<worn around wrist> ",
    "<primary weapon>    ",
    "<held>              ",
    "<floating nearby>   ",
    "<tattooed>          ",
    "<secondary weapon>  ",
    "<mark>              ",
};


/* for  keeping track of the player count */
int max_on = 0;

/*
 * Local functions.
 */
char *	format_obj_to_char	args( ( OBJ_DATA *obj, CHAR_DATA *ch,
				    bool fShort ) );
void	show_list_to_char	args( ( OBJ_DATA *list, CHAR_DATA *ch,
				    bool fShort, bool fShowNothing ) );
void	show_char_to_char_0	args( ( CHAR_DATA *victim, CHAR_DATA *ch ) );
void	show_char_to_char_1	args( ( CHAR_DATA *victim, CHAR_DATA *ch ) );
void	show_char_to_char	args( ( CHAR_DATA *list, CHAR_DATA *ch ) );
bool	check_blind		args( ( CHAR_DATA *ch ) );
char *    report_condition2 args( ( CHAR_DATA *ch, int iCond ) );
static void lokprot_send_room_token args( ( CHAR_DATA *ch ) );


char *format_obj_to_char( OBJ_DATA *obj, CHAR_DATA *ch, bool fShort )
{
    static char buf[MAX_STRING_LENGTH];
    AFFECT_DATA *paf;

    buf[0] = '\0';

    if ((fShort && (obj->short_descr == NULL || obj->short_descr[0] == '\0'))
    ||  (obj->description == NULL || obj->description[0] == '\0'))
	return buf;

    if ( IS_OBJ_STAT(obj, ITEM_INVIS)     )   strcat( buf, "({YInvis{x) ");
    if ( IS_AFFECTED(ch, AFF_DETECT_EVIL)
         && IS_OBJ_STAT(obj, ITEM_EVIL)   )   strcat( buf, "({RRed Aura{x) " );
    if (IS_AFFECTED(ch, AFF_DETECT_GOOD)
    &&  IS_OBJ_STAT(obj,ITEM_BLESS))	    strcat(buf,"({BBlue Aura{x) " );
    if ( IS_AFFECTED(ch, AFF_DETECT_MAGIC)
         && IS_OBJ_STAT(obj, ITEM_MAGIC)  )   strcat( buf, "({MMagical{x) " );
    if ( IS_OBJ_STAT(obj, ITEM_GLOW)      )   strcat( buf, "({YGlowing{x) " );
    if ( IS_OBJ_STAT(obj, ITEM_HUM)       )   strcat( buf, "({CHumming{x) " );
    if ( IS_OBJ_STAT(obj, ITEM_DEATHBLADE))   strcat( buf, "({RDeathblade{x) " );
    if ( IS_OBJ_STAT(obj, ITEM_MASTERWORK))   strcat( buf, "({BMasterwork{x) " );
    if ( obj->item_type == ITEM_WEAPON
	   && IS_OBJ_STAT2(obj, ITEM_ARTISTRY)) strcat( buf, "({GRetooled{x) " );
    if ( (obj->item_type == ITEM_ARMOR || obj->item_type == ITEM_JEWELRY)
	   && IS_OBJ_STAT2(obj, ITEM_ARTISTRY) ) strcat( buf, "({GCrafted{x) " );


    for(paf = obj->affected; paf != NULL; paf = paf->next)
    {
      if(paf->type == gsn_polish) strcat( buf, "({CPolished{x) " );
    }

    if ( fShort )
    {
	if ( obj->short_descr != NULL )
	    strcat( buf, obj->short_descr );
    }
    else
    {
	if ( obj->description != NULL)
	    strcat( buf, obj->description );
    }

    return buf;
}



/*
 * Show a list to a character.
 * Can coalesce duplicated items.
 */
void show_list_to_char( OBJ_DATA *list, CHAR_DATA *ch, bool fShort, bool fShowNothing )
{
    char buf[MAX_STRING_LENGTH];
    BUFFER *output;
    char **prgpstrShow;
    int *prgnShow;
    char *pstrShow;
    OBJ_DATA *obj;
    int nShow;
    int iShow;
    int count;
    bool fCombine;

    if ( ch->desc == NULL )
	return;

    /*
     * Alloc space for output lines.
     */
    output = new_buf();

    count = 0;
    for ( obj = list; obj != NULL; obj = obj->next_content )
	count++;
    if ( count > 500 )
    {
       send_to_char( "You have WAY too much junk!  Drop some of it!\n\r", ch );
       return;
    }
    prgpstrShow	= alloc_mem( count * sizeof(char *) );
    prgnShow    = alloc_mem( count * sizeof(int)    );
    nShow	= 0;

    /*
     * Format the list of objects.
     */
    for ( obj = list; obj != NULL; obj = obj->next_content )
    { 
	if ( obj->wear_loc == WEAR_NONE && can_see_obj( ch, obj )) 
	{
	    pstrShow = format_obj_to_char( obj, ch, fShort );

	    fCombine = FALSE;

	    if ( IS_NPC(ch) || IS_SET(ch->comm, COMM_COMBINE) )
	    {
		/*
		 * Look for duplicates, case sensitive.
		 * Matches tend to be near end so run loop backwords.
		 */
		for ( iShow = nShow - 1; iShow >= 0; iShow-- )
		{
		    if ( !strcmp( prgpstrShow[iShow], pstrShow ) )
		    {
			prgnShow[iShow]++;
			fCombine = TRUE;
			break;
		    }
		}
	    }

	    /*
	     * Couldn't combine, or didn't want to.
	     */
	    if ( !fCombine )
	    {
		prgpstrShow [nShow] = str_dup( pstrShow );
		prgnShow    [nShow] = 1;
		nShow++;
	    }
	}
    }

    /*
     * Output the formatted list.
     */
    for ( iShow = 0; iShow < nShow; iShow++ )
    {
	if (prgpstrShow[iShow][0] == '\0')
	{
	    free_string(prgpstrShow[iShow]);
	    continue;
	}

	if ( IS_NPC(ch) || IS_SET(ch->comm, COMM_COMBINE) )
	{
	    if ( prgnShow[iShow] != 1 )
	    {
		sprintf( buf, "(%2d) ", prgnShow[iShow] );
		add_buf(output,buf);
	    }
	    else
	    {
		add_buf(output,"     ");
	    }
	}
	add_buf(output,prgpstrShow[iShow]);
	add_buf(output,"\n\r");
	free_string( prgpstrShow[iShow] );
    }

    if ( fShowNothing && nShow == 0 )
    {
	if ( IS_NPC(ch) || IS_SET(ch->comm, COMM_COMBINE) )
	    send_to_char( "     ", ch );
	send_to_char( "Nothing.\n\r", ch );
    }
    page_to_char(buf_string(output),ch);

    /*
     * Clean up.
     */
    free_buf(output);
    free_mem( prgpstrShow, count * sizeof(char *) );
    free_mem( prgnShow,    count * sizeof(int)    );

    return;
}



void show_char_to_char_0( CHAR_DATA *victim, CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH], message[MAX_STRING_LENGTH];

    buf[0] = '\0';
    if (IS_NPC(victim) &&ch->questmob > 0 && victim->pIndexData->vnum == ch->questmob)
        strcat( buf, "[TARGET] ");
    if ( IS_SET(victim->comm,COMM_AFK)   		) strcat( buf, "{M[AFK]{x "		);
    if ( IS_SET(victim->comm,COMM_WRITING)   	) strcat( buf, "{M[Writing]{x "	);
    if ( IS_SET(victim->comm,COMM_EDITOR)   	) strcat( buf, "{M[Editing]{x "	);
    if ( victim->desc == NULL && !IS_NPC(victim)) strcat( buf, "{Y(LD){x " 		);
    if ( IS_AFFECTED(victim, AFF_INVISIBLE)   	) strcat( buf, "{Y(Invis){x " 	);
    if ( victim->invis_level >= LEVEL_HERO    	) strcat( buf, "{C(Wizi){x "		);
    if ( IS_AFFECTED(victim, AFF_HIDE)        	) strcat( buf, "{D(Hidden){x "	);
    if ( IS_AFFECTED(victim, AFF_FLYING)      	) strcat( buf, "{W(Flying){x " 	);
    if ( IS_AFFECTED2(victim, AFF_CAMOUFLAGE)   ) strcat( buf, "{G(Camo){x "		);
    if ( IS_AFFECTED(victim, AFF_CHARM)       	) strcat( buf, "{Y(Charmed){x " 	);
    if ( IS_AFFECTED(victim, AFF_PASS_DOOR)   	) strcat( buf, "{C(Translucent){x "	);
    if ( IS_AFFECTED(victim, AFF_INCENDIARY_CLOUD) ) strcat( buf, "{R(Flaming Aura){x " );
    if ((IS_EVIL(victim)
    ||   IS_AFFECTED3(victim, AFF_EAURA))
    &&  !IS_AFFECTED3(victim, AFF_GAURA)
    &&   IS_AFFECTED(ch, AFF_DETECT_EVIL)     	) strcat( buf, "{R(Red Aura){x "   	);
    if ((IS_GOOD(victim) 
    ||   IS_AFFECTED3(victim, AFF_GAURA))
    &&  !IS_AFFECTED3(victim, AFF_EAURA)
    &&   IS_AFFECTED(ch, AFF_DETECT_GOOD)     	) strcat( buf, "{Y(Golden Aura){x "	);
    if ( IS_AFFECTED(victim, AFF_SANCTUARY)   	) strcat( buf, "{W(White Aura){x "	);
    if ( IS_AFFECTED2(victim, AFF_WRAITHFORM) 	) strcat( buf, "{W(Ghostly){x "	);
    if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_KILLER ) )
								  strcat( buf, "{R(KILLER){x  " 	);
    if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_THIEF  ) )
								  strcat( buf, "{R(THIEF){x " 	);
    if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_ASSAULT  ) )
								  strcat( buf, "{R(ASSAULT){x " 	);
    if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_TREASON  ) )
								  strcat( buf, "{R(TREASON){x " 	);
    if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_OUTLAW  ) )
								  strcat( buf, "{R(OUTLAW){x " 	);
    if ( IS_SET(ch->act,PLR_HOLYLIGHT) && is_affected(victim,gsn_doppelganger))  
    {
        strcat( buf, "("); strcat(buf, PERS_DOPPEL(victim,ch)); strcat(buf, ") ");
    }
    if ( IS_SET(ch->act,PLR_HOLYLIGHT) && is_affected(victim,gsn_treeform))  
    {
        strcat( buf, "("); strcat(buf, victim->name); strcat(buf, ") ");
    }
/* Before Doppel *
    if ( victim->position == victim->start_pos && victim->long_descr[0] != '\0' )
    {
	strcat( buf, victim->long_descr );
	send_to_char( buf, ch );
	return;
    }
*/
    if (is_affected(victim,gsn_doppelganger)  && 
        victim->doppel->long_descr[0] != '\0') 
      {
      strcat(buf, victim->doppel->name);
      send_to_char(buf, ch);

      if (IS_NPC(ch))
      sprintf( buf, " %s",victim->long_descr);
      else
      sprintf( buf, " %s",victim->doppel->long_descr);
      send_to_char(buf, ch);
      return;
      }

    /* Treeform */
    if (is_affected(victim,gsn_treeform)
    && !is_affected(victim,gsn_doppelganger) 
    &&  victim->position != POS_SLEEPING )
    {
      if (IS_NPC(ch))
	{
        sprintf( buf, " %s",victim->long_descr);
	}
      else
	if (IS_SET(ch->act,PLR_HOLYLIGHT))
	{
	  sprintf( buf, " ({r%s{x) A mighty oak tree stands tall before you.\n\r",victim->name);
	}
	else
	{
        sprintf( buf, " A mighty oak tree stands tall before you.\n\r");
	}
      send_to_char(buf, ch);
      return;
    }

/* Tether */
    if (victim->long_descr[0] != '\0'
    && !is_affected(victim,gsn_doppelganger) 
    &&  victim->position != POS_SLEEPING
    && victim->tethered)
    {
      sprintf( buf, "%s%s is tethered here.\n\r", buf, victim->short_descr );
      send_to_char(buf, ch);
      return;
    }

    if (victim->long_descr[0] != '\0' && 
        !is_affected(victim,gsn_doppelganger) 
    &&  victim->position != POS_SLEEPING)
    {
      strcat( buf, victim->long_descr );
      send_to_char(buf, ch);
      return;
    }

    if (victim->cpose)
    {
    	if ( !IS_SET(ch->act,PLR_HOLYLIGHT)
	&& is_affected(victim,gsn_doppelganger)
	&& !IS_NPC(victim))
     	  strcat(buf,victim->doppel->name);
     	else
      if (is_affected(victim,gsn_treeform))
        strcat(buf," A mighty oak tree stands tall before you.");
      else
      strcat(buf, victim->name);
      strcat(buf," ");
      strcat(buf,victim->cpose);
      strcat(buf,"\n\r");
      send_to_char(buf,ch);
      return;
    }

    if (is_affected(victim, gsn_doppelganger)
    && (!is_affected(victim, gsn_treeform))
    &&  !IS_SET(ch->act,PLR_HOLYLIGHT))
    {
	if (IS_NPC(victim))
        strcat(buf, PERS(victim, ch ));
	else
	  strcat(buf, PERS_DOPPEL(victim, ch));

      if (!IS_NPC(victim->doppel) && !IS_SET(ch->comm, COMM_BRIEF))
        strcat(buf, victim->doppel->pcdata->title);
    }
    else
    {
      strcat( buf, PERS( victim, ch ) );
      if ( !IS_NPC(victim) && !IS_SET(ch->comm, COMM_BRIEF) 
      &&   victim->position == POS_STANDING && ch->on == NULL )
	  strcat( buf, victim->pcdata->title );
    }

    if( (!victim->riding) && (!victim->rider))
    switch ( victim->position )
    {
    case POS_DEAD:     strcat( buf, " is hovering here in spirit form." ); break;
    case POS_MORTAL:   strcat( buf, " is mortally wounded." );   break;
    case POS_INCAP:    strcat( buf, " is incapacitated." );      break;
    case POS_STUNNED:  strcat( buf, " is lying here stunned." ); break;
    case POS_SLEEPING: 
	if (victim->on != NULL)
	{
	    if (IS_SET(victim->on->value[2],SLEEP_AT))
  	    {
		sprintf(message," is sleeping at %s.",
		    victim->on->short_descr);
		strcat(buf,message);
	    }
	    else if (IS_SET(victim->on->value[2],SLEEP_ON))
	    {
		sprintf(message," is sleeping on %s.",
		    victim->on->short_descr); 
		strcat(buf,message);
	    }
	    else
	    {
		sprintf(message, " is sleeping in %s.",victim->on->short_descr);
		strcat(buf,message);
	    }
	}
	else
	    if ( IS_AFFECTED2(victim, AFF_BLACKJACK) )
	    {
		strcat(buf," is here, knocked out cold.");
	    }
	else 
	    strcat(buf," is sleeping here.");
	break;
    case POS_RESTING:  
        if (victim->on != NULL)
	{
            if (IS_SET(victim->on->value[2],REST_AT))
            {
                sprintf(message," is resting at %s.",
                    victim->on->short_descr);
                strcat(buf,message);
            }
            else if (IS_SET(victim->on->value[2],REST_ON))
            {
                sprintf(message," is resting on %s.",
                    victim->on->short_descr);
                strcat(buf,message);
            }
            else 
            {
                sprintf(message, " is resting in %s.",
                    victim->on->short_descr);
                strcat(buf,message);
            }
	}
        else
	    strcat( buf, " is resting here." );       
	break;
    case POS_SITTING:  
        if (victim->on != NULL)
        {
            if (IS_SET(victim->on->value[2],SIT_AT))
            {
                sprintf(message," is sitting at %s.",
                    victim->on->short_descr);
                strcat(buf,message);
            }
            else if (IS_SET(victim->on->value[2],SIT_ON))
            {
                sprintf(message," is sitting on %s.",
                    victim->on->short_descr);
                strcat(buf,message);
            }
            else
            {
                sprintf(message, " is sitting in %s.",
                    victim->on->short_descr);
                strcat(buf,message);
            }
        }
        else
	    strcat(buf, " is sitting here.");
	break;
    case POS_STANDING: 
	if (victim->on != NULL)
	{
	    if (IS_SET(victim->on->value[2],STAND_AT))
	    {
		sprintf(message," is standing at %s.",
		    victim->on->short_descr);
		strcat(buf,message);
	    }
	    else if (IS_SET(victim->on->value[2],STAND_ON))
	    {
		sprintf(message," is standing on %s.",
		   victim->on->short_descr);
		strcat(buf,message);
	    }
	    else
	    {
		sprintf(message," is standing in %s.",
		    victim->on->short_descr);
		strcat(buf,message);
	    }
	}
	else
	    strcat( buf, " is here." );               
	break;
    case POS_FIGHTING:
	strcat( buf, " is here, fighting " );
	if ( victim->fighting == NULL )
	    strcat( buf, "thin air??" );
	else if ( victim->fighting == ch )
	    strcat( buf, "YOU!" );
	else if ( victim->in_room == victim->fighting->in_room )
	{
	  strcat( buf, PERS( victim->fighting, ch ) );
	  strcat( buf, "." );
	}
	else
	    strcat( buf, "someone who left??" );
	break;
    }
    else
    {
      send_to_char(buf,ch);
      sprintf(buf," is here, mounted on %s", can_see( ch, victim->riding) ? victim->riding->short_descr : "someone");
      switch ( victim->position )
      {
        case POS_STANDING:  strcat(buf,"."); break; 
        case POS_FIGHTING:
          strcat( buf, ", fighting " );
          if ( victim->fighting == NULL )
            strcat( buf, "thin air??" );
          else
          if ( victim->fighting == ch )
            strcat( buf, "YOU!" );
          else
          if ( victim->in_room == victim->fighting->in_room )
          {
            if (is_affected(victim, gsn_doppelganger)
		&& !IS_SET(ch->act,PLR_HOLYLIGHT)
		&& !IS_NPC(ch))
		{
	        strcat( buf, PERS_DOPPEL( victim->fighting, ch ) );
		}
	      else
            if (is_affected(victim, gsn_treeform)
		&& !IS_SET(ch->act,PLR_HOLYLIGHT)
		&& !IS_NPC(ch))
		{
              strcat( buf, " A might oak tree" );
		}
            else
		{
	        strcat( buf, PERS( victim->fighting, ch ) );
		}
            strcat( buf, "." );
          }
          else
            strcat( buf, "someone who left??");
        break;
      }
   }

   strcat( buf, "\n\r" );
   buf[0] = UPPER(buf[0]);
   send_to_char( buf, ch );
   return;
}



void show_char_to_char_1( CHAR_DATA *victim, CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    CHAR_DATA *tmp_vict;
    int iWear;
    int percent;
    bool found;

    tmp_vict = is_affected(victim,gsn_doppelganger) ? victim->doppel : victim;

    if ( can_see( victim, ch ) )
    {
	if (ch == victim)
	{
        if (is_affected(ch,gsn_treeform))
	  {
          act( "A mighty oak tree looks at itself.",ch,NULL,NULL,TO_ROOM);
	  }
	  else
	  {
	    act( "$n looks at $mself.",ch,NULL,NULL,TO_ROOM);
	  }
	}
	else
	{
        if (is_affected(victim,gsn_treeform))
	  {
	    act( "$n looks at you.", ch, NULL, victim, TO_VICT    );
	    act( "$n looks at the mighty oak tree.",  ch, NULL, victim, TO_NOTVICT );
	  }
	  else
	  {
	    act( "$n looks at you.", ch, NULL, victim, TO_VICT    );
	    act( "$n looks at $N.",  ch, NULL, victim, TO_NOTVICT );
	  }
	}
    }

    if ( tmp_vict->description[0] != '\0' )
    {
      if (is_affected(tmp_vict,gsn_treeform))
	{
        strcat(buf, "Before you stands a mighty oak tree.  Its leaves are large and shimmer with a\n\rdeep green color.  The bark of the tree appears to be very thick and sturdy,\n\renough to dull even the sharpest of blades after only a few swings.  The\n\rmighty oak almost seems magical in a way, as if truely alive and watching your\n\revery move.\n\r");
	  send_to_char( buf, ch );
	}
	else
	{
	  send_to_char( tmp_vict->description, ch );
	}
	send_to_char( "\n\r", ch );
    }
    else
    {
	act( "You see nothing special about $M.", ch, NULL, victim, TO_CHAR );
    }

    if ( victim->max_hit > 0 )
	percent = ( 100 * victim->hit ) / victim->max_hit;
    else
	percent = -1;

    if (is_affected(victim, gsn_doppelganger)
    && !IS_SET(ch->act,PLR_HOLYLIGHT)
    && !IS_NPC(ch))
    {
	strcpy( buf, PERS_DOPPEL( victim, ch ) );
    }
    else
    if (is_affected(victim, gsn_treeform)
    && !IS_SET(ch->act,PLR_HOLYLIGHT)
    && !IS_NPC(ch))
    {
	strcpy( buf, "A mighty oak tree" );
    }
    else
    {
      strcpy( buf, PERS( victim, ch ) );
    }

    if (percent >= 100) 
	strcat( buf, " is in perfect health.\n\r");
    else if (percent >= 90) 
	strcat( buf, " is slightly scratched.\n\r");
    else if (percent >= 80) 
	strcat( buf, " has a few bruises.\n\r");
    else if (percent >= 70) 
	strcat( buf, " has some small wounds and bruises.\n\r");
    else if (percent >= 60) 
	strcat( buf, " has several wounds.\n\r");
    else if (percent >= 50)
	strcat( buf, " has many nasty wounds.\n\r");
    else if (percent >= 40) 
	strcat( buf, " is bleeding freely.\n\r");
    else if (percent >= 30)
	strcat( buf, " is covered in blood.\n\r");
    else if (percent >= 20) 
	strcat( buf, " is vomiting blood.\n\r");
    else if (percent >= 10)
	strcat( buf, " is barely clinging to life.\n\r");
    else if (percent >= 0 )
	strcat( buf, " is about to die.\n\r");
    else
	strcat(buf, " is bleeding to death.\n\r");

    if (victim->desc == NULL && !IS_NPC(victim))
    {
        strcat(buf, PERS(victim, ch));
        strcat(buf, " is {Ylink dead{x.\n\r");
    }

    buf[0] = UPPER(buf[0]);
    send_to_char( buf, ch );

    found = FALSE;
    for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
    {
	if ( ( obj = get_eq_char( victim, iWear ) ) != NULL
	&&   can_see_obj( ch, obj ) )
	{
	    if ( !found )
	    {
		send_to_char( "\n\r", ch );
		act( "$N is using:", ch, NULL, victim, TO_CHAR );
		found = TRUE;
	    }
	    send_to_char( where_name[iWear], ch );
	    send_to_char( format_obj_to_char( obj, ch, TRUE ), ch );
	    send_to_char( "\n\r", ch );
	}
    }

    if ( victim != ch
    && !IS_NPC(ch)
    && !IS_IMMORTAL(victim)
    && number_percent( ) < get_skill(ch,gsn_peek))
    {
	send_to_char( "\n\rYou peek at the inventory:\n\r", ch );
	check_improve(ch,gsn_peek,TRUE,4);
	show_list_to_char( victim->carrying, ch, TRUE, TRUE );
    }

    return;
}



void show_char_to_char( CHAR_DATA *list, CHAR_DATA *ch )
{
    CHAR_DATA *rch;

    for ( rch = list; rch != NULL; rch = rch->next_in_room )
    {
	if ( rch == ch )
	    continue;

      if ( rch->rider != NULL) 
          continue;

	if ( get_trust(ch) < rch->invis_level)
	    continue;

	if ( can_see( ch, rch ) )
	{
	    show_char_to_char_0( rch, ch );
	}
	else if ( room_is_dark( ch->in_room )
	&&        IS_AFFECTED(rch, AFF_INFRARED ) )
	{
	    send_to_char( "{RYou see glowing red eyes watching YOU!{x\n\r", ch );
	}
    }

    return;
} 



bool check_blind( CHAR_DATA *ch )
{

    if (!IS_NPC(ch) && IS_SET(ch->act,PLR_HOLYLIGHT))
	return TRUE;

    if ( IS_AFFECTED(ch, AFF_BLIND) )
    { 
	send_to_char( "You can't see a thing!\n\r", ch ); 
	return FALSE; 
    }

    return TRUE;
}

/* changes your scroll */
void do_scroll(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char buf[100];
    int lines;

    one_argument(argument,arg);
    
    if (arg[0] == '\0')
    {
	if (ch->lines == 0)
	    send_to_char("You do not page long messages.\n\r",ch);
	else
	{
	    sprintf(buf,"You currently display %d lines per page.\n\r",
		    ch->lines + 2);
	    send_to_char(buf,ch);
	}
	return;
    }

    if (!is_number(arg))
    {
	send_to_char("You must provide a number.\n\r",ch);
	return;
    }

    lines = atoi(arg);

    if (lines == 0)
    {
        send_to_char("Paging disabled.\n\r",ch);
        ch->lines = 0;
        return;
    }

    if (lines < 10 || lines > 100)
    {
	send_to_char("You must provide a reasonable number.\n\r",ch);
	return;
    }

    sprintf(buf,"Scroll set to %d lines.\n\r",lines);
    send_to_char(buf,ch);
    ch->lines = lines - 2;
}

/* RT does socials */
void do_socials(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    int iSocial;
    int col;
     
    col = 0;
   
    for (iSocial = 0; social_table[iSocial].name[0] != '\0'; iSocial++)
    {
	sprintf(buf,"%-12s",social_table[iSocial].name);
	send_to_char(buf,ch);
	if (++col % 6 == 0)
	    send_to_char("\n\r",ch);
    }

    if ( col % 6 != 0)
	send_to_char("\n\r",ch);
    return;
}


 
/* RT Commands to replace news, motd, imotd, etc from ROM */

void do_motd(CHAR_DATA *ch, char *argument)
{
    do_function(ch, &do_help, "motd");
}

void do_imotd(CHAR_DATA *ch, char *argument)
{  
    do_function(ch, &do_help, "imotd");
}

void do_rules(CHAR_DATA *ch, char *argument)
{
    do_function(ch, &do_help, "rules");
}

void do_story(CHAR_DATA *ch, char *argument)
{
    do_function(ch, &do_help, "story");
}

void do_wizlist(CHAR_DATA *ch, char *argument)
{
    do_function(ch, &do_help, "wizlist");
}

/* RT this following section holds all the auto commands from ROM, as well as
   replacements for config */

void do_autolist(CHAR_DATA *ch, char *argument)
{
    /* lists most player flags */
    if (IS_NPC(ch))
      return;

    send_to_char("   action     status\n\r",ch);
    send_to_char("---------------------\n\r",ch);
 
    send_to_char("autoassist     ",ch);
    if (IS_SET(ch->act,PLR_AUTOASSIST))
        send_to_char("ON\n\r",ch);
    else
        send_to_char("OFF\n\r",ch);

    send_to_char("automercy      ",ch);
    if (IS_SET(ch->act,PLR_AUTOMERCY))
        send_to_char("ON\n\r",ch);
    else
        send_to_char("OFF\n\r",ch);

    send_to_char("autoyield      ",ch);
    if (IS_SET(ch->act,PLR_AUTOYIELD))
        send_to_char("ON\n\r",ch);
    else
        send_to_char("OFF\n\r",ch);

    send_to_char("autoexit       ",ch);
    if (IS_SET(ch->act,PLR_AUTOEXIT))
        send_to_char("ON\n\r",ch);
    else
        send_to_char("OFF\n\r",ch);

    send_to_char("autogold       ",ch);
    if (IS_SET(ch->act,PLR_AUTOGOLD))
        send_to_char("ON\n\r",ch);
    else
        send_to_char("OFF\n\r",ch);

    send_to_char("autoloot       ",ch);
    if (IS_SET(ch->act,PLR_AUTOLOOT))
        send_to_char("ON\n\r",ch);
    else
        send_to_char("OFF\n\r",ch);

    send_to_char("autosac        ",ch);
    if (IS_SET(ch->act,PLR_AUTOSAC))
        send_to_char("ON\n\r",ch);
    else
        send_to_char("OFF\n\r",ch);

    send_to_char("autosplit      ",ch);
    if (IS_SET(ch->act,PLR_AUTOSPLIT))
        send_to_char("ON\n\r",ch);
    else
        send_to_char("OFF\n\r",ch);

    send_to_char("autotitle      ",ch);
    if (IS_SET(ch->act,PLR_AUTOTITLE))
        send_to_char("ON\n\r",ch);
    else
        send_to_char("OFF\n\r",ch);

    if(IS_IMMORTAL(ch))
    {
    send_to_char("Autodig        ",ch);
    if (IS_SET(ch->act2,PLR_AUTODIG))
	send_to_char("ON\n\r",ch);
    else
	send_to_char("OFF\n\r",ch);
    }

    send_to_char("compact mode   ",ch);
    if (IS_SET(ch->comm,COMM_COMPACT))
        send_to_char("ON\n\r",ch);
    else
        send_to_char("OFF\n\r",ch);

    send_to_char("prompt         ",ch);
    if (IS_SET(ch->comm,COMM_PROMPT))
	send_to_char("ON\n\r",ch);
    else
	send_to_char("OFF\n\r",ch);

    send_to_char("combine items  ",ch);
    if (IS_SET(ch->comm,COMM_COMBINE))
	send_to_char("ON\n\r",ch);
    else
	send_to_char("OFF\n\r",ch);

    send_to_char("Your corpse may always be looted from thieves, so be careful.\n\r",ch);
   
    if (IS_SET(ch->act,PLR_NOFOLLOW))
	send_to_char("You do not welcome followers.\n\r",ch);
    else
	send_to_char("You accept followers.\n\r",ch);
}

void do_automercy(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;

    if (IS_SET(ch->act,PLR_AUTOMERCY))
    {
      send_to_char("Automercy removed.\n\r",ch);
      REMOVE_BIT(ch->act,PLR_AUTOMERCY);
    }
    else
    {
      send_to_char("You will bestow mercy upon others in fight.\n\r",ch);
      SET_BIT(ch->act,PLR_AUTOMERCY);
    }
}

void do_autoyield(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;

    if (IS_SET(ch->act,PLR_AUTOYIELD))
    {
      send_to_char("Autoyield removed.\n\r",ch);
      REMOVE_BIT(ch->act,PLR_AUTOYIELD);
    }  
    else
    {
      send_to_char("You will now accept others to yield in combat.\n\r",ch);
      SET_BIT(ch->act,PLR_AUTOYIELD);
    }
}

void do_autoassist(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
    
    if (IS_SET(ch->act,PLR_AUTOASSIST))
    {
      send_to_char("Autoassist removed.\n\r",ch);
      REMOVE_BIT(ch->act,PLR_AUTOASSIST);
    }
    else
    {
      send_to_char("You will now assist when needed.\n\r",ch);
      SET_BIT(ch->act,PLR_AUTOASSIST);
    }
}

void do_autotitle(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;

    if (IS_IMMORTAL(ch))
      send_to_char("\n\rPlayers will still be able to see your title, no matter what!\n\r\n\r",ch);
    
    if (IS_SET(ch->act,PLR_AUTOTITLE))
    {
      send_to_char("Autotitle removed. People will no longer see your title on the WHO list!\n\r",ch);
      REMOVE_BIT(ch->act,PLR_AUTOTITLE);
    }
    else
    {
      send_to_char("People will now be able to see your title on the WHO list!\n\r",ch);
      SET_BIT(ch->act,PLR_AUTOTITLE);
    }
}

void do_autoexit(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
 
    if (IS_SET(ch->act,PLR_AUTOEXIT))
    {
      send_to_char("Exits will no longer be displayed.\n\r",ch);
      REMOVE_BIT(ch->act,PLR_AUTOEXIT);
    }
    else
    {
      send_to_char("Exits will now be displayed.\n\r",ch);
      SET_BIT(ch->act,PLR_AUTOEXIT);
    }
}

void do_autogold(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
 
    if (IS_SET(ch->act,PLR_AUTOGOLD))
    {
      send_to_char("Autogold removed.\n\r",ch);
      REMOVE_BIT(ch->act,PLR_AUTOGOLD);
    }
    else
    {
      send_to_char("Automatic steel and gold looting set.\n\r",ch);
      SET_BIT(ch->act,PLR_AUTOGOLD);
    }
}

void do_autoloot(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
 
    if (IS_SET(ch->act,PLR_AUTOLOOT))
    {
      send_to_char("Autolooting removed.\n\r",ch);
      REMOVE_BIT(ch->act,PLR_AUTOLOOT);
    }
    else
    {
      send_to_char("Automatic corpse looting set.\n\r",ch);
      SET_BIT(ch->act,PLR_AUTOLOOT);
    }
}

void do_autosac(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
 
    if (IS_SET(ch->act,PLR_AUTOSAC))
    {
      send_to_char("Autosacrificing removed.\n\r",ch);
      REMOVE_BIT(ch->act,PLR_AUTOSAC);
    }
    else
    {
      send_to_char("Automatic corpse sacrificing set.\n\r",ch);
      SET_BIT(ch->act,PLR_AUTOSAC);
    }
}

void do_autosplit(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
 
    if (IS_SET(ch->act,PLR_AUTOSPLIT))
    {
      send_to_char("Autosplitting removed.\n\r",ch);
      REMOVE_BIT(ch->act,PLR_AUTOSPLIT);
    }
    else
    {
      send_to_char("Automatic steel and gold splitting set.\n\r",ch);
      SET_BIT(ch->act,PLR_AUTOSPLIT);
    }
}

void do_brief(CHAR_DATA *ch, char *argument)
{
    if (IS_SET(ch->comm,COMM_BRIEF))
    {
      send_to_char("Full descriptions activated.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_BRIEF);
    }
    else
    {
      send_to_char("Short descriptions activated.\n\r",ch);
      SET_BIT(ch->comm,COMM_BRIEF);
    }
}

void do_compact(CHAR_DATA *ch, char *argument)
{
    if (IS_SET(ch->comm,COMM_COMPACT))
    {
      send_to_char("Compact mode removed.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_COMPACT);
    }
    else
    {
      send_to_char("Compact mode set.\n\r",ch);
      SET_BIT(ch->comm,COMM_COMPACT);
    }
}

void do_show(CHAR_DATA *ch, char *argument)
{
    if (IS_SET(ch->comm,COMM_SHOW_AFFECTS))
    {
      send_to_char("Affects will no longer be shown in score.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_SHOW_AFFECTS);
    }
    else
    {
      send_to_char("Affects will now be shown in score.\n\r",ch);
      SET_BIT(ch->comm,COMM_SHOW_AFFECTS);
    }
}

void do_prompt(CHAR_DATA *ch, char *argument)
{
   char buf[MAX_STRING_LENGTH];
 
   if ( argument[0] == '\0' )
   {
	if (IS_SET(ch->comm,COMM_PROMPT))
   	{
        send_to_char("You will no longer see prompts.\n\r",ch);
        REMOVE_BIT(ch->comm,COMM_PROMPT);
    	}
    	else
    	{
        send_to_char("You will now see prompts.\n\r",ch);
        SET_BIT(ch->comm,COMM_PROMPT);
    	}
       return;
   }

   if( !strcmp( argument, "show" ) )
   {
     sprintf(buf,"Your current prompt is set to %s\n\r",ch->prompt );
     send_to_char(buf,ch);
     return;
   }
 
   if( !strcmp( argument, "all" ) )
      strcpy( buf, "<%hhp %mm %vmv>%f ");
   else
   {
      if ( strlen(argument) > 50 )
         argument[50] = '\0';
      strcpy( buf, argument );
      smash_tilde( buf );
      if (str_suffix("%c",buf))
	strcat(buf," ");
	
   }
 
   free_string( ch->prompt );
   ch->prompt = str_dup( buf );
   sprintf(buf,"Prompt set to %s\n\r",ch->prompt );
   send_to_char(buf,ch);
   return;
}

void do_combine(CHAR_DATA *ch, char *argument)
{

    if (IS_SET(ch->comm,COMM_COMBINE))
    {
      send_to_char("Long inventory selected.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_COMBINE);
    }
    else
    {
      send_to_char("Combined inventory selected.\n\r",ch);
      SET_BIT(ch->comm,COMM_COMBINE);
    }
}

void do_noloot(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
 
    if (IS_SET(ch->act,PLR_CANLOOT))
    {
      send_to_char("Your corpse is now safe from thieves.\n\r",ch);
      REMOVE_BIT(ch->act,PLR_CANLOOT);
    }
    else
    {
      send_to_char("Your corpse may now be looted.\n\r",ch);
      SET_BIT(ch->act,PLR_CANLOOT);
    }
}

void do_nofollow(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;

    if (ch->riding)
    {
      send_to_char("You need to dismount first.\n\r",ch);
      return;
    }

    if ( (IS_AFFECTED(ch, AFF_CHARM) && ch->master != NULL)
	|| (is_affected(ch, gsn_serenade) && ch->master != NULL) )
    {
	act( "But you'd rather follow $N!", ch, NULL, ch->master, TO_CHAR );
	return;
    }

    if(ch->fight_pos != FIGHT_FRONT) 
    ch->fight_pos = FIGHT_FRONT;
 
    if (IS_SET(ch->act,PLR_NOFOLLOW))
    {
      send_to_char("You now accept followers.\n\r",ch);
      REMOVE_BIT(ch->act,PLR_NOFOLLOW);
    }
    else
    {
      send_to_char("You no longer accept followers.\n\r",ch);
      SET_BIT(ch->act,PLR_NOFOLLOW);
      die_follower( ch );
    }
}

static void lokprot_send_room_token( CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];

    if (ch == NULL || ch->in_room == NULL || IS_NPC(ch) || ch->pcdata == NULL)
	return;

    if (str_cmp(ch->pcdata->lokprot, "on"))
	return;

    sprintf(buf, "lokprotroomvnum%d", ch->in_room->vnum);
    send_to_char(buf, ch);
}

void do_look( CHAR_DATA *ch, char *argument )
{
    char buf  [MAX_STRING_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    EXIT_DATA *pexit;
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    OBJ_DATA *obj2;
    bool found;
    char *pdesc;
    int door;
    int number,count;
    int *dirtab;

    if ( ch->desc == NULL )
	return;

    if ( ch->position < POS_SLEEPING )
    {
	send_to_char( "You can't see anything but stars!\n\r", ch );
	return;
    }

    if ( ch->position == POS_SLEEPING )
    {
	send_to_char( "You can't see anything, you're sleeping!\n\r", ch );
	return;
    }

    if ( !check_blind( ch ) )
	return;

   found = FALSE;

   if (ch->in_room != NULL)
   {
    for (obj2 = ch->in_room->contents; obj2 != NULL; obj2 = obj2->next_content)    
    {
       if (obj2->pIndexData->vnum == OBJ_VNUM_CAMPFIRE)
       {
          found = TRUE;
          break;
       }
    }
   }

    if ( !IS_NPC(ch)
    &&   !IS_SET(ch->act, PLR_HOLYLIGHT)
    &&   room_is_dark( ch->in_room )
    &&   !found)
    {
      send_to_char( "It is pitch black ... \n\r", ch );
	show_char_to_char( ch->in_room->people, ch );
	if ( IS_AFFECTED(ch, AFF_INFRARED))
	  show_list_to_char(ch->in_room->contents, ch, FALSE, FALSE);
	return;
    }

    if (IS_AFFECTED2(ch, AFF_PARADISE)
    && !IS_NPC(ch)
    && !IS_SET(ch->act, PLR_HOLYLIGHT))
    {
      send_to_char("Nirvana......\n\r", ch);
      send_to_char("  ", ch);
      send_to_char("You are in that place where you go when you close your eyes\n\r", ch); 
      send_to_char("to sleep or to daydream.  As you look around, everything is as\n\r", ch);
      send_to_char("it was your last time here.  Maybe, just maybe, the gods will\n\r", ch);
      send_to_char("let you stay here, with all that you love, and all that you\n\r", ch); 
      send_to_char("desire.  You don't need to wake up from your dream.  Indeed,\n\r", ch); 
      send_to_char("it may very well be that time, for however hard you try to\n\r", ch); 
      send_to_char("to open your eyes to the drab world that you inhabit, nothing\n\r", ch); 
      send_to_char("meets your eyes except the glories of your dream world.\n\r", ch);
      return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    number = number_argument(arg1,arg3);
    count = 0;

    if ( arg1[0] == '\0' || !str_cmp( arg1, "auto" ) )
    {
	/* 'look' or 'look auto' */
	sprintf( buf, "{s%s", ch->in_room->name );
	send_to_char( buf, ch );

	if ( (IS_IMMORTAL(ch) && (IS_NPC(ch) || IS_SET(ch->act,PLR_HOLYLIGHT)))
	||   IS_BUILDER(ch, ch->in_room->area) )
	{
	    sprintf(buf," {r[{RRoom %d{r]",ch->in_room->vnum);
	    send_to_char(buf,ch);
	}

	send_to_char( "{x\n\r", ch );

	if ( arg1[0] == '\0'
	|| ( !IS_NPC(ch) && !IS_SET(ch->comm, COMM_BRIEF) ) )
	{
	    sprintf( buf, "{S  %s{x", ch->in_room->description );
	    send_to_char( buf, ch );
	}

        if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_AUTOEXIT) )
	{
	    send_to_char("\n\r",ch);
            do_function(ch, &do_exits, "auto" );
	}

	show_list_to_char( ch->in_room->contents, ch, FALSE, FALSE );
	show_char_to_char( ch->in_room->people,   ch );
        lokprot_send_room_token( ch );
	return;
    }

    if ( !str_cmp( arg1, "i" ) || !str_cmp(arg1, "in")  || !str_cmp(arg1,"on"))
    {
	/* 'look in' */
	if ( arg2[0] == '\0' )
	{
	    send_to_char( "Look in what?\n\r", ch );
	    return;
	}

	if ( ( obj = get_obj_here( ch, NULL, arg2 ) ) == NULL )
	{
	    send_to_char( "You do not see that here.\n\r", ch );
	    return;
	}

	switch ( obj->item_type )
	{
	default:
	    send_to_char( "That is not a container.\n\r", ch );
	    break;

	case ITEM_DRINK_CON:
	    if ( obj->value[1] <= 0 )
	    {
		send_to_char( "It is empty.\n\r", ch );
		break;
	    }

	    sprintf( buf, "It's %sfilled with a %s liquid.\n\r",
		obj->value[1] <     obj->value[0] / 4
		    ? "less than half-" :
		obj->value[1] < 3 * obj->value[0] / 4
		    ? "about half-"     : "more than half-",
		liq_table[obj->value[2]].liq_color
		);

	    send_to_char( buf, ch );
	    break;

	case ITEM_CONTAINER:
	case ITEM_CORPSE_NPC:
	case ITEM_CORPSE_PC:
	    if ( IS_SET(obj->value[1], CONT_CLOSED) )
	    {
		send_to_char( "It is closed.\n\r", ch );
		break;
	    }

	    act( "$p holds:", ch, obj, NULL, TO_CHAR );
	    show_list_to_char( obj->contains, ch, TRUE, TRUE );
	    break;
	case ITEM_QUIVER:
	    if ( obj->value[0] <= 0 )
	    {
		send_to_char( "{WThe quiver is out of arrows.{x\n\r", ch );
		break;
	    }
		
	    if (obj->value[0] == 1 )
	    {
		send_to_char( "{WThe quiver has 1 arrow remaining in it.{x\n\r", ch );
		break;
	    }
		
	    if (obj->value[0] > 1 )
	    {
		sprintf( buf, "{WThe quiver has %d arrows in it.{x\n\r", obj->value[0]);
	    }
	    send_to_char( buf, ch);
	    break;
	}
	return;
    }

    if ( ( victim = get_char_room( ch, NULL, arg1 ) ) != NULL )
    {
	show_char_to_char_1( victim, ch );
	return;
    }

    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
	if ( can_see_obj( ch, obj ) )
	{  /* player can see object */
	    pdesc = get_extra_descr( arg3, obj->extra_descr );
	    if ( pdesc != NULL )
            {
	    	if (++count == number)
	    	{
		    send_to_char( pdesc, ch );
		    return;
	    	}
	    	else continue;
            }

 	    pdesc = get_extra_descr( arg3, obj->pIndexData->extra_descr );
 	    if ( pdesc != NULL )
            {
 	    	if (++count == number)
 	    	{	
		    send_to_char( pdesc, ch );
		    return;
	     	}
		else continue;
            }

	    if ( is_name( arg3, obj->name ) )
	    	if (++count == number)
	    	{
	    	    send_to_char( obj->description, ch );
	    	    send_to_char( "\n\r",ch);
		    return;
		  }
	  }
    }

    for ( obj = ch->in_room->contents; obj != NULL; obj = obj->next_content )
    {
	if ( can_see_obj( ch, obj ) )
	{
	    pdesc = get_extra_descr( arg3, obj->extra_descr );
	    if ( pdesc != NULL )
	    	if (++count == number)
	    	{
		    send_to_char( pdesc, ch );
		    return;
	    	}

	    pdesc = get_extra_descr( arg3, obj->pIndexData->extra_descr );
	    if ( pdesc != NULL )
	    	if (++count == number)
	    	{
		    send_to_char( pdesc, ch );
		    return;
	    	}

	    if ( is_name( arg3, obj->name ) )
		if (++count == number)
		{
		    send_to_char( obj->description, ch );
		    send_to_char("\n\r",ch);
		    return;
		}
	}
    }

    pdesc = get_extra_descr(arg3,ch->in_room->extra_descr);
    if (pdesc != NULL)
    {
	if (++count == number)
	{
	    send_to_char(pdesc,ch);
	    return;
	}
    }
    
    if (count > 0 && count != number)
    {
    	if (count == 1)
    	    sprintf(buf,"You only see one %s here.\n\r",arg3);
    	else
    	    sprintf(buf,"You only see %d of those here.\n\r",count);
    	
    	send_to_char(buf,ch);
    	return;
    }

         if ( !str_cmp( arg1, "n" ) || !str_cmp( arg1, "north" ) ) door = 0;
    else if ( !str_cmp( arg1, "e" ) || !str_cmp( arg1, "east"  ) ) door = 1;
    else if ( !str_cmp( arg1, "s" ) || !str_cmp( arg1, "south" ) ) door = 2;
    else if ( !str_cmp( arg1, "w" ) || !str_cmp( arg1, "west"  ) ) door = 3;
    else if ( !str_cmp( arg1, "u" ) || !str_cmp( arg1, "up"    ) ) door = 4;
    else if ( !str_cmp( arg1, "d" ) || !str_cmp( arg1, "down"  ) ) door = 5;
    else
    {
	send_to_char( "You do not see that here.\n\r", ch );
	return;
    }

    if( IS_AFFECTED2(ch, AFF_LOOKING_GLASS) )
	dirtab = oppossite_dir;
    else
	dirtab = regular_dir;

    /* 'look direction' */
    if ( ( pexit = ch->in_room->exit[dirtab[door]] ) == NULL )
    {
	send_to_char( "Nothing special there.\n\r", ch );
	return;
    }

    if ( pexit->description != NULL && pexit->description[0] != '\0' )
	send_to_char( pexit->description, ch );
    else
	send_to_char( "Nothing special there.\n\r", ch );

    if ( pexit->keyword    != NULL
    &&   pexit->keyword[0] != '\0'
    &&   pexit->keyword[0] != ' ' )
    {
	if ( IS_SET(pexit->exit_info, EX_CLOSED) )
	{
	    act( "The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR );
	}
	else if ( IS_SET(pexit->exit_info, EX_ISDOOR) )
	{
	    act( "The $d is open.",   ch, NULL, pexit->keyword, TO_CHAR );
	}
    }

    return;
}

/* RT added back for the hell of it */
void do_read (CHAR_DATA *ch, char *argument )
{
    do_function(ch, &do_look, argument);
}

void do_examine( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Examine what?\n\r", ch );
	return;
    }

    do_function(ch, &do_look, arg );

    if ( ( obj = get_obj_here( ch, NULL, arg ) ) != NULL )
    {
	switch ( obj->item_type )
	{
	default:
	    break;

	case ITEM_ARMOR: case ITEM_WEAPON:
	    strcpy( buf, "As you look more closely, you notice that it is ");
	    if (obj->condition >= 100) strcat( buf, "in superb condition.");
       else if (obj->condition >=  90) strcat( buf, "in very good condition.");
       else if (obj->condition >=  80) strcat( buf, "in good shape.");
       else if (obj->condition >=  70) strcat( buf, "showing a bit of wear.");
       else if (obj->condition >=  60) strcat( buf, "a little run down.");
       else if (obj->condition >=  50) strcat( buf, "in need of repair.");
       else if (obj->condition >=  40) strcat( buf, "in great need of repair.");
       else if (obj->condition >=  30) strcat( buf, "in dire need of repair.");
       else if (obj->condition >=  20) strcat( buf, "very badly worn.");
       else if (obj->condition >=  10) strcat( buf, "practically worthless.");
       else if (obj->condition >=  1) strcat( buf, "nearly falling apart.");
       else if (obj->condition <=  0) strcat( buf, "broken.");
	    strcat( buf, "\n\r" );
	    send_to_char( buf, ch );
	    break;

	case ITEM_FOOD:
	    if ( obj->timer > 0 )
	      obj->condition = (obj->timer * 2);
	    else
	      obj->condition = 100;	
	      strcpy( buf, "As you examine it carefully you notice that it " );
	    if (obj->condition >= 100) strcat( buf, "is fresh.");
       else if (obj->condition >=  90) strcat( buf, "is nearly fresh.");
       else if (obj->condition >=  80) strcat( buf, "is perfectly fine.");
       else if (obj->condition >=  70) strcat( buf, "looks good.");
       else if (obj->condition >=  60) strcat( buf, "looks ok.");
       else if (obj->condition >=  50) strcat( buf, "is a little stale.");
       else if (obj->condition >=  40) strcat( buf, "is a bit stale.");
       else if (obj->condition >=  30) strcat( buf, "smells slightly off.");
       else if (obj->condition >=  20) strcat( buf, "smells quite rank.");
       else if (obj->condition >=  10) strcat( buf, "smells revolting!");
       else if (obj->condition >=  0) strcat( buf, "is crawling with maggots!");
	    strcat( buf, "\n\r" );
	    send_to_char( buf, ch );
	    break;
	
	case ITEM_JUKEBOX:
	    do_function(ch, &do_play, "list");
	    break;

	case ITEM_MONEY:
	    if (obj->value[0] == 0)
	    {
	        if (obj->value[1] == 0)
		    sprintf(buf,"Odd...there's no coins in the pile.\n\r");
		else if (obj->value[1] == 1)
		    sprintf(buf,"Wow. One steel coin.\n\r");
		else
		    sprintf(buf,"There are %d steel coins in the pile.\n\r",
			obj->value[1]);
	    }
	    else if (obj->value[1] == 0)
	    {
		if (obj->value[0] == 1)
		    sprintf(buf,"Wow. One gold coin.\n\r");
		else
		    sprintf(buf,"There are %d gold coins in the pile.\n\r",
			obj->value[0]);
	    }
	    else
		sprintf(buf,
		    "There are %d steel and %d gold coins in the pile.\n\r",
		    obj->value[1],obj->value[0]);
	    send_to_char(buf,ch);
	    break;

	case ITEM_DRINK_CON:
	case ITEM_CONTAINER:
	case ITEM_CORPSE_NPC:
	case ITEM_CORPSE_PC:
	    sprintf(buf,"in %s",argument);
	    do_function(ch, &do_look, buf );
	}
    }

    return;
}



/*
 * Thanks to Zrin for auto-exit part.
 */
void do_exits( CHAR_DATA *ch, char *argument )
{
    extern char * const dir_name[];
    char buf[MAX_STRING_LENGTH];
    EXIT_DATA *pexit;
    bool found;
    bool fAuto;
    int door;
    int *dirtab;

    fAuto  = !str_cmp( argument, "auto" );

    if ( !check_blind( ch ) )
	return;

    if( IS_AFFECTED2(ch, AFF_LOOKING_GLASS) )
	dirtab = oppossite_dir;
    else
	dirtab = regular_dir;

    if (IS_AFFECTED2(ch, AFF_PARADISE)
    && !IS_NPC(ch)
    && !IS_SET(ch->act, PLR_HOLYLIGHT))
    {
      send_to_char("  ", ch);
      send_to_char("Nirvana......\n\r", ch);
      send_to_char("  ", ch);
      send_to_char("All you see is an altered state of reality everywhere you look!\n\r", ch); 
      return;
    }

    if (fAuto)
	sprintf(buf,"[Exits:");
    else if (IS_IMMORTAL(ch))
	sprintf(buf,"Obvious exits from room %d:\n\r",ch->in_room->vnum);
    else
	sprintf(buf,"Obvious exits:\n\r");

    found = FALSE;
    for ( door = 0; door <= 5; door++ )
    {
	if ( ( pexit = ch->in_room->exit[door] ) != NULL
	&&   pexit->u1.to_room != NULL
	&&   can_see_room(ch,pexit->u1.to_room) )
	{
	    found = TRUE;
	    if ( fAuto )
	    {
		strcat( buf, " " );
		if IS_SET(pexit->exit_info, EX_CLOSED)
			strcat( buf, "(" );
		strcat( buf, dir_name[dirtab[door]] );
		if IS_SET(pexit->exit_info, EX_CLOSED)
			strcat( buf, ")" );
	    }
	    else
	    {
		sprintf( buf + strlen(buf), "%-5s - %s",
		    capitalize( dir_name[dirtab[door]] ),
		    room_is_dark( pexit->u1.to_room )
			?  "Too dark to tell"
			: pexit->u1.to_room->name
		    );
		if (IS_IMMORTAL(ch))
		    sprintf(buf + strlen(buf), 
			" (room %d)\n\r",pexit->u1.to_room->vnum);
		else
		    sprintf(buf + strlen(buf), "\n\r");
	    }
	}
    }

    if ( !found )
	strcat( buf, fAuto ? " none" : "None.\n\r" );

    if ( fAuto )
	strcat( buf, "]\n\r" );

    send_to_char( buf, ch );
    return;
}

void do_worth( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    int xp_to_lev;

    if (IS_NPC(ch))
    {
	sprintf(buf,"You have %ld steel and %ld gold.\n\r",
	    ch->steel,ch->gold);
	send_to_char(buf,ch);
	return;
    }

    xp_to_lev = exp_at_level(ch, ch->level + 1) - ch->exp;
    sprintf(buf, 
    "You have %ld steel, %ld gold, and %d experience (%d exp to level).\n\r",
	ch->steel, ch->gold,ch->exp,xp_to_lev);

    send_to_char(buf,ch);

    /* DEBUG */
    if( xp_to_lev < 0 && ch->level < 100)
    {
	sprintf(buf, "Worth, negative xp to level %d, char %s\n\r",
		xp_to_lev,ch->name);
	bug(buf,0);
	sprintf(buf, "ch->exp=%d ch->level=%d\n\r",
	    ch->exp, ch->level );
	bug(buf,0);
     } /* end DEBUG */

    return;
}

/*
Less Verbose report_condition for do_score
*/

char * report_condition2( CHAR_DATA *ch, int iCond )
{
    int condition, degree;
    static char *cond_mesgs[3][7] = {
        {
            "Sober",
            "Tipsy",
            "Inebriated",
            "Drunk",
            "Very Drunk",
            "Reeling Drunk",
            "Severly Drunk",
        },
        {
            "Starving",
            "Hungry",
            "Quite Hungry",
            "Somewhat Hungry",
            "Well Fed",
            "Full",
            "Stuffed"
        },
        {
            "Parched",
            "Very Thirsty",
            "Thirsty",
            "Somewhat Thirsty",
            "Barely Thirsty",
            "Not Thirsty",
            "Quenched"
        }
    };
    if( IS_NPC( ch ) || ch->position < POS_SLEEPING )
	return NULL;

    if( iCond > 2 )
    {
        bug("Unknown condition type %d", iCond );
        return NULL;
    }
    condition = ch->pcdata->condition[iCond];
    if( condition < 0 ) 
    {
        bug("Condition %d less than zero", iCond );
        return NULL;
    }
    else if( condition == 0 )
        degree = 0;
    else if( condition < 4 )
        degree = 1;
    else if( condition < 8 )
        degree = 2;
    else if( condition < 24 )
        degree = 3;
    else if( condition < 40 )
        degree = 4;
    else if( condition < 45 )
        degree = 5;
    else 
        degree = 6;
    
    return  cond_mesgs[iCond][degree];
}

/*Airius Modified */
void do_score( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];
  int xp_to_lev;
  int hours=0;

  if ( ch->desc->original != NULL )
  {
	send_to_char( "You can not display the SCORE of a mob.\n\r", ch );
	send_to_char( "Please RETURN to your original self to see a SCORE display.\n\r", ch );
	return;
  }

  hours = (int) ((ch->played + current_time - ch->logon) / 3600);

  sprintf(buf,
	 "{y    ___________________________________   _________________________________{x\n\r");
  send_to_char(buf, ch);

  sprintf(buf,"{y.-/|  {R78   {y~~**~~                      {y\\{x {y/                    ~~**~~   {R79  {y|{y\\{x{y-{x{y.{x\n\r");
  send_to_char(buf, ch);

  sprintf(buf,"{y||||                                    :                                  ||||{x\n\r");
  send_to_char(buf, ch);

  sprintf(buf,"{y||||   {RName:       {D%-18s   {y:   {RSurname:       {D%-16s{y||||{x\n\r",
  ch->name,
  IS_NPC(ch) ? "" : ch->surname);
  send_to_char(buf, ch);

  sprintf(buf,"{y||||____________________________________:__________________________________||||{x\n\r");
  send_to_char(buf, ch);

  sprintf(buf,"{y||||   {cRace:       {W%-14s       {y:   {cClass:         {W%-14s  {y||||{x\n\r",
  race_table[ch->race].name,
  class_table[ch->class].name);
  send_to_char(buf, ch);

  sprintf(buf,"{y||||   {cGender:     {W%-7s              {y:                                  {y||||{x\n\r",
  ch->sex==0 ? "sexless" : ch->sex==1 ? "male" : "female");
  send_to_char(buf, ch);

  sprintf(buf,"{y||||   {cLevel:      {W%-3d                  {y:   {cHours:         {W%-4d            {y||||{x\n\r",
  ch->level,
  hours);
  send_to_char(buf, ch);

if (!IS_NPC(ch) && ch->level < 100)
{
  xp_to_lev = exp_at_level(ch, ch->level + 1) - ch->exp;
  sprintf(buf,"{y||||   {cExperience: {W%-10d           {y:   {cTo Level:      {W%-10d      {y||||{x\n\r",
  ch->exp,
  xp_to_lev);
  send_to_char(buf, ch);
	/* DEBUG */
  if( xp_to_lev < 0 && ch->level < 100)
  {
	 sprintf(buf, "Score, negative xp to level %d, char %s\n\r",
	 xp_to_lev,ch->name);
	 bug(buf,0);
	 sprintf(buf, "ch->exp=%d ch->level=%d\n\r",
	 ch->exp, ch->level );
       bug(buf,0);
  } /* end DEBUG */
}

  sprintf(buf,"{y||||   {cPractices:  {W%-4d                 {y:   {cTrains:        {W%-4d            {y||||{x\n\r",
  ch->practice,
  ch->train);
  send_to_char(buf, ch);

  sprintf(buf,"{y||||   {cHometown:   {W%-12s         {y:   {cQuest Points:  {W%-5d           {y||||{x\n\r",
  hometown_table[ch->hometown].name,
  ch->questpoints);
  send_to_char(buf, ch);

  sprintf(buf,"{y||||   {cAlign:      {W%-6d {D%-11s     {y:   {cEthos:         {W%-11s     {y||||{x\n\r",
  ch->alignment,
  ch->alignment > 900 ? "{W(angelic)": ch->alignment > 700 ? "{W(saintly)": ch->alignment > 350 ? "{w(good)": ch->alignment > 100 ? "{G(good)": ch->alignment > -100 ? "{G(neutral)": ch->alignment > -350 ? "{G(mean)": ch->alignment > -700 ? "{R(evil)": ch->alignment > -900 ? "{R(demonic)": "{R(wicked)",
  get_ethos(ch));
  send_to_char(buf, ch);

  sprintf(buf,"{y||||                                    :                                  ||||{x\n\r");
  send_to_char(buf, ch);

  sprintf(buf,"{y||||   {cStr: {w%3d{W(%3d)    {cCon: {w%3d{W(%3d)   {y:   {cHit Points: {W%-6d{y/{w%6d      {y||||{x\n\r",
  ch->perm_stat[STAT_STR],
  get_curr_stat(ch,STAT_STR),
  ch->perm_stat[STAT_CON],
  get_curr_stat(ch,STAT_CON),
  ch->hit,
  ch->max_hit);
  send_to_char(buf, ch);

  sprintf(buf,"{y||||   {cInt: {w%3d{W(%3d)    {cWis: {w%3d{W(%3d)   {y:   {cMana:       {W%-6d{y/{w%6d      {y||||{x\n\r",
  ch->perm_stat[STAT_INT],
  get_curr_stat(ch,STAT_INT),
  ch->perm_stat[STAT_WIS],
  get_curr_stat(ch,STAT_WIS),
  ch->mana,
  ch->max_mana);
  send_to_char(buf, ch);

  sprintf(buf,"{y||||   {cDex: {w%3d{W(%3d)    {cChr: {w%3d{W(%3d)   {y:   {cMovement:   {W%-6d{y/{w%6d      {y||||{x\n\r",
  ch->perm_stat[STAT_DEX],
  get_curr_stat(ch,STAT_DEX),
  ch->perm_stat[STAT_CHR],
  get_curr_stat(ch,STAT_CHR),
  ch->move,
  ch->max_move);
  send_to_char(buf, ch);

  sprintf(buf,"{y||||   {cACpierce: {M%1s{W%3d   {cACbash:  {M%1s{W%3d   {y:   {cSteel:    {W%7ld              {y||||{x\n\r",
  GET_AC(ch,AC_PIERCE) < 0 ? "-" : GET_AC(ch,AC_PIERCE) == 0 ? " " : "+",
  abs(GET_AC(ch,AC_PIERCE)),
  GET_AC(ch,AC_BASH) < 0 ? "-" : GET_AC(ch,AC_BASH) == 0 ? " " : "+",
  abs(GET_AC(ch,AC_BASH)),
  ch->steel);
  send_to_char(buf, ch);

  sprintf(buf,"{y||||   {cACslash:  {M%1s{W%3d   {cACmagic: {M%1s{W%3d   {y:   {cGold:     {W%7ld              {y||||{x\n\r",
  (GET_AC(ch,AC_SLASH)<0) ? "-" : (GET_AC(ch,AC_SLASH))==0 ? " " : "+",
  abs(GET_AC(ch,AC_SLASH)),
  (GET_AC(ch,AC_EXOTIC)<0) ? "-" : (GET_AC(ch,AC_EXOTIC))==0 ? " " : "+",
  abs(GET_AC(ch,AC_EXOTIC)),
  ch->gold);
  send_to_char(buf, ch);

  sprintf(buf,"{y||||   {cHitroll:  {M%1s{W%3d   {cDamroll: {M%1s{W%3d   {y:   {cIn Bank:  {W%7ld steel coins  {y||||{x\n\r",
  GET_HITROLL(ch) < 0 ? "-" : GET_HITROLL(ch) == 0 ? " " : "+",
  abs(GET_HITROLL(ch)),
  GET_DAMROLL(ch) < 0 ? "-" : GET_DAMROLL(ch) == 0 ? " " : "+",
  abs(GET_DAMROLL(ch)),
  ch->bank);
  send_to_char(buf, ch);

  sprintf(buf,"{y||||                                    :                                  ||||{x\n\r");
  send_to_char(buf, ch);

  sprintf(buf,"{y||||   {cHunger: {W%-16s         {y:   {cThirst:       {W%-16s {y||||{x\n\r",
  report_condition2( ch, COND_FULL ),
  report_condition2( ch, COND_THIRST ));
  send_to_char(buf, ch);

  sprintf(buf,"{y||||   {cItems: {W%4d {c/ {W%4d               {y:   {cEncumbrance: {W%5ld {c/ {W%7d  {y||||{x\n\r",
  ch->carry_number,
  can_carry_n(ch),
  get_carry_weight(ch)/10,
  can_carry_w(ch)/10 );
  send_to_char(buf, ch);

  if (IS_IMMORTAL(ch))
  {
  sprintf(buf,"{y||||   {MHoly Light: {W%3s  {MInvisible: {W%3d  {y:   {MIncognito: {W%3d   {MTrust: {W%3d    {y||||{x\n\r",
  IS_SET(ch->act,PLR_HOLYLIGHT) ? "ON" : "OFF",
  ch->invis_level,
  ch->incog_level,
  ch->trust);
  send_to_char(buf, ch);
  }

  sprintf(buf,"{y||||   {MYield:  {W%5d                    {y:                                  ||||{x\n\r",
  ch->yield);
  send_to_char(buf, ch);

  sprintf(buf,"{y||||                                    )                                  ||||{x\n\r");
  send_to_char(buf, ch);

  sprintf(buf,"{y||||   {cVulnerable to: {W%-35s                  {y||||{x\n\r",
  imm_bit_name(ch->vuln_flags));
  send_to_char(buf, ch);

  sprintf(buf,"{y||||   {c    Immune to: {W%-35s                  {y||||{x\n\r",
  imm_bit_name(ch->imm_flags));
  send_to_char(buf, ch);

  sprintf(buf,"{y||||   {c Resistant to: {W%-35s                  {y||||{x\n\r",
  imm_bit_name(ch->res_flags));
  send_to_char(buf, ch);

  sprintf(buf,"{y||||                                                                       ||||{x\n\r");
  send_to_char(buf, ch);

  sprintf(buf,"{y||||   {cE-Mail: {W%-35s                         {y||||{x\n\r",
  ch->pcdata->email);
  send_to_char(buf, ch);

  sprintf(buf,"{y||||___________________________________   _________________________________||||{x\n\r");
  send_to_char(buf, ch);

  sprintf(buf,"{y||/====================================\\:/==================================\\||{x\n\r");
  send_to_char(buf, ch);

  sprintf(buf,"{y`-------------------------------------~___~----------------------------------''{x\n\r");
  send_to_char(buf, ch);

  sprintf(buf,"\n\r");
  send_to_char(buf, ch);

  if ( ch->cpose == NULL )
  {
  sprintf( buf, "   {DYou have no cpose set.{x\n\r");
  send_to_char( buf,ch );
  }
  else
  {
  sprintf( buf, "   {DCurrent cpose: {W%s {C%s{x\n\r",
  ch->name,
  ch->cpose);
  send_to_char( buf,ch );
  }

  sprintf( buf, "   {DYou are {C%d {Dyears old, which is {C%s {Dfor your race.{x\n\r",
  get_age(ch),
  get_age_word(ch));
  send_to_char(buf, ch);

  if ( ch->org_id != ORG_NONE )
  {
  sprintf( buf, "   {DYou are a member of the {C%s Clan{D%s{x\n\r",
  org_table[ ORG(ch->org_id) ].name,
  ch->org_id & ORG_LEADER ? ", and you are it's Leader." : "." );
  send_to_char(buf, ch);
  }

  if ( ch->clan_id != CLAN_NONE )
  {
  sprintf( buf, "   {DYou are a member of the {C%s Clan{D%s{x\n\r",
  org_table[ CLAN(ch->clan_id) ].name,
  ch->clan_id & CLAN_LEADER ? ", and you are it's Leader." : "." );
  send_to_char(buf, ch);
  }

  if (ch->devotee_id && !ch->religion_id)
    sprintf( buf, "   {D%s (devotee){x",
    religion_table[ch->devotee_id].longname);
  else
    sprintf( buf, "   {DReligion: %s %s{x",
    religion_table[RELIGION(ch->religion_id)].longname,
    ch->religion_id & RELIGION_LEADER ? "(leader)" :
    ( ch->pcdata->highpriest == 1 ? "(high)" : "" ) );
  send_to_char( buf, ch );

  sprintf(buf,"\n\r");
  send_to_char(buf, ch);

	if (IS_SET(ch->comm,COMM_SHOW_AFFECTS))
	do_affects(ch,"");
}

void do_affects(CHAR_DATA *ch, char *argument )
{
    AFFECT_DATA *paf, *paf_last = NULL;
    char buf[MAX_STRING_LENGTH];
    
    if ( ch->affected != NULL )
    {
	send_to_char( "You are affected by the following spells:\n\r", ch );
	for ( paf = ch->affected; paf != NULL; paf = paf->next )
	{
	    if (paf_last != NULL && paf->type == paf_last->type)
		if (ch->level >= 15)
		    sprintf( buf, "                      ");
		else
		    continue;
	    else
	    	sprintf( buf, "Spell:{G %-15s{x",
skill_table[paf->type].name );

	    send_to_char( buf, ch );

	    if ( ch->level >= 15 )
	    {
		sprintf( buf,
		    ": modifies{G %s{g by{G %d{x ",
		    affect_loc_name( paf->location ),
		    paf->modifier);
		send_to_char( buf, ch );
		if ( paf->duration == -1 )
		    sprintf( buf, "{Rpermanently{x" );
		else
		    sprintf( buf, "for{G %d{g hours{x - aff lvl %d", paf->duration, paf->level );
		send_to_char( buf, ch );
	    }

	    send_to_char( "\n\r", ch );
	    paf_last = paf;
	}
    }
    else 
	send_to_char("You are not affected by any spells affects or any other kind of affects.\n\r",ch);

    if (IS_AFFECTED(ch, AFF_HIDE))
    {
	send_to_char("\n\r",ch);
	send_to_char("You are currently hiding in the shadows.\n\r",ch);
    }

    return;
}



char *	const	day_name	[] =
{
    "Gileadai", "Luindai", "Nuindai", "Soldai", "Manthus",
    "Shinarai", "Boreadai"
};

char *	const	day_name2	[] =
{
    "Gilean", "Lunitari", "Nuitari", "Solinari", "Majere",
    "Shinare", "Zivilyn"
};

char *	const	day_name3	[] =
{
    "Friend Day", "Hunt Day", "Share Day", "Gather Day", "Barter Day",
    "Worship Day", "Council Day"
};

char *	const	day_name4	[] =
{
    "Gateway", "Bright Eye", "Night's Eye", "Dead Eye", "Dream Dance",
    "Winged Trade", "World Tree"
};

char *	const	day_name5	[] =
{
    "Brenzik", "Mithrik", "Adamachtis", "Aurachil", "Cuprig",
    "Ferramis", "Agorin"
};

char *	const	day_name6	[] =
{
    "Praise Day", "Light Day", "Fire Day", "Winds Day", "Waters Day",
    "Ground Day", "Shadow Day"
};

char *	const	month_name	[] =
{
    "Aelmont", "Rannmont", "Mishamont", "Chislmont", "Bran",
    "Corij", "Argon", "Sirrimont", "Reorxmont", 
    "Hiddumont", "H'rarmont",  "Pheonix"
};

char *	const	month_name2	[] =
{
    "Winter Night", "Winter Deep", "Spring Dawning", "Spring Rain",
    "Spring Blossom", "Summer Home", "Summer Run", "Summer End", "Autumn Harvest",
    "Autumn Twilight", "Autumn Dark", "Winter Come"
};

char *	const	month_name3	[] =
{
    "Dark-Crypt", "Dark-Deep", "Damp-Mood", "Damp-Chisel",
    "Dry-Anvil", "Dry-Axe", "Dry-Heat", "Dry-Forge", "Cold-Hammer",
    "Cold-Steel", "Cold-Rust", "Cold-Lode"
};

char *	const	month_name4	[] =
{
    "Snowfun", "Darktime", "Windsong", "Raindrum",
    "Flowerfield", "Homefriends", "Wandertime", "Summerlaze", "Harvestfete",
    "Leafplay", "Bleakcold", "Blessings"
};

char *	const	month_name5	[] =
{
    "Ice Glaze", "Snow Deep", "Mountain Thaw", "Earth Wakes",
    "Flower Blooms", "Home Hearth", "Raging Fire", "Dying Ember", "Harvest Home",
    "Leaf Gild", "Dark Cold", "Frost Eve"
};

char *	const	month_name6	[] =
{
    "Chemosh", "Zeboim", "Mishakal", "Chislev",
    "Branchala", "Kiri-Jolith", "Sargonnas", "Sirrion", "Reorx",
    "Hiddukel", "Morgion", "Habbakuk"
};

char *	const	hour_name	[] =
{
    "Darkwatch", "After Darkwatch Hour", "Deepwatch", "After Deepwatch Hour", 
    "Early Watch", "Waking Hour", "Morning Watch", "First Watch", "Second Watch", 
    "Third Watch", "Fourth Watch", "Fifth Watch", "High Watch", "Seventh Watch", 
    "Eighth Watch", "Ninth Watch", "Tenth Watch", "Eleventh Watch", 
    "Twelfth Watch", "Evening Watch", "Last Watch", "Afterwatch Hour", "Late Watch",
    "Restful Hour"
};

void do_time( CHAR_DATA *ch, char *argument )
{
    extern char str_boot_time[];
    char buf[MAX_STRING_LENGTH];
    char            s[100];
    char *suf;
    int day;

    strftime( s, 100, "%I:%M%p", localtime( &ch->logon ) );

    day     = time_info.day + 1;

         if ( day > 4 && day <  20 ) suf = "th";
    else if ( day % 10 ==  1       ) suf = "st";
    else if ( day % 10 ==  2       ) suf = "nd";
    else if ( day % 10 ==  3       ) suf = "rd";
    else                             suf = "th";

    if ( IS_IMMORTAL(ch))
    {
    sprintf( buf,
	"It is %s (%d %s), Day of %s,\n\r the %d%s of the Month of %s.\n\r",
	hour_name[time_info.hour],
	(time_info.hour % 12 == 0) ? 12 : time_info.hour %12,
	time_info.hour >= 12 ? "pm" : "am",
	day_name2[day % 7],
	day, suf,
	month_name6[time_info.month]);
    send_to_char(buf,ch);

    sprintf(buf,"ROM started up at %s\n\rCurrent System Time: %s\n\rYour Login Time: %s\n\r",
	str_boot_time, (char *) ctime (&current_time), s);
    }
else if ( ch->race == race_lookup("ogre"))
    {
    sprintf( buf,
	"It is %s (%d %s), %s,\n\r the %d%s of the Month of %s.\n\r",
	hour_name[time_info.hour],
	(time_info.hour % 12 == 0) ? 12 : time_info.hour %12,
	time_info.hour >= 12 ? "pm" : "am",
	day_name3[day % 7],
	day, suf,
	month_name5[time_info.month]);
    send_to_char(buf,ch);
    sprintf(buf,"ROM started up at %s\n\rCurrent System Time: %s\n\rYour Login Time: %s\n\r",
	str_boot_time,
	(char *) ctime( &current_time ), s);
    }
else if ( ch->race == race_lookup("kender"))
    {
    sprintf( buf,
	"It is %s (%d %s), %s,\n\r the %d%s of the Month of %s.\n\r",
	hour_name[time_info.hour],
	(time_info.hour % 12 == 0) ? 12 : time_info.hour %12,
	time_info.hour >= 12 ? "pm" : "am",
	day_name6[day % 7],
	day, suf,
	month_name4[time_info.month]);
    send_to_char(buf,ch);
    sprintf(buf,"ROM started up at %s\n\rCurrent System Time: %s\n\rYour Login Time: %s\n\r",
	str_boot_time,
	(char *) ctime( &current_time ), s);
    }
else if ( ch->race == race_lookup("neidar")
    ||   ch->race == race_lookup("hylar")
    ||   ch->race == race_lookup("theiwar")
    ||   ch->race == race_lookup("aghar")
    ||   ch->race == race_lookup("daergar")
    ||   ch->race == race_lookup("daewar")
    ||   ch->race == race_lookup("klar"))
    {
    sprintf( buf,
	"It is %s (%d %s), Day of %s,\n\r the %d%s of the Month of %s.\n\r",
	hour_name[time_info.hour],
	(time_info.hour % 12 == 0) ? 12 : time_info.hour %12,
	time_info.hour >= 12 ? "pm" : "am",
	day_name5[day % 7],
	day, suf,
	month_name3[time_info.month]);
    send_to_char(buf,ch);
    sprintf(buf,"ROM started up at %s\n\rCurrent System Time: %s\n\rYour Login Time: %s\n\r",
	str_boot_time,
	(char *) ctime( &current_time ), s);
    }
else if ( ch->race == race_lookup("silvanesti")
    ||   ch->race == race_lookup("dargonesti")
    ||   ch->race == race_lookup("qualinesti")
    ||   ch->race == race_lookup("dimernesti")
    ||   ch->race == race_lookup("kagonesti"))
    {
    sprintf( buf,
	"It is %s (%d %s), Day of %s,\n\r the %d%s of the Month of %s.\n\r",
	hour_name[time_info.hour],
	(time_info.hour % 12 == 0) ? 12 : time_info.hour %12,
	time_info.hour >= 12 ? "pm" : "am",
	day_name4[day % 7],
	day, suf,
	month_name2[time_info.month]);
    send_to_char(buf,ch);
    sprintf(buf,"ROM started up at %s\n\rCurrent System Time: %s\n\rYour Login Time: %s\n\r",
	str_boot_time,
	(char *) ctime( &current_time ), s);
    }
else
    {
    sprintf( buf,
	"It is %s (%d %s), Day of %s,\n\r %d%s the Month of %s.\n\r",
	hour_name[time_info.hour],
	(time_info.hour % 12 == 0) ? 12 : time_info.hour %12,
	time_info.hour >= 12 ? "pm" : "am",
	day_name[day % 7],
	day, suf,
	month_name[time_info.month]);
    send_to_char(buf,ch);
    sprintf(buf,"ROM started up at %s\n\rCurrent System Time: %s\n\rYour Login Time: %s\n\r",
	str_boot_time,
	(char *) ctime( &current_time ), s);
    }

    send_to_char( buf, ch );
    return;
}

void do_weather( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    static char * const sky_look[4] =
    {
	"cloudless",
	"cloudy",
	"rainy",
	"lit by flashes of lightning"
    };

    if ( !IS_OUTSIDE(ch) )
    {
	send_to_char( "You can't see the weather indoors.\n\r", ch );
	return;
    }

    sprintf( buf, "The sky is %s and %s.\n\r",
	sky_look[weather_info.sky],
	weather_info.change >= 0
	? "a warm southerly breeze blows"
	: "a cold northern gust blows"
	);
    send_to_char( buf, ch );
    return;
}

void do_help( CHAR_DATA *ch, char *argument )
{
    HELP_DATA *pHelp;
    char output[4*MSL];
    char argall[MAX_INPUT_LENGTH],argone[MAX_INPUT_LENGTH];
    int level;

    output[0] = '\0';

    if ( argument[0] == '\0' )
	argument = "summary";

    /* this parts handles help a b so that it returns help 'a b' */
    argall[0] = '\0';
    while (argument[0] != '\0' )
    {
	argument = one_argument(argument,argone);
	if (argall[0] != '\0')
	    strcat(argall," ");
	strcat(argall,argone);
    }

    for ( pHelp = help_first; pHelp != NULL; pHelp = pHelp->next )
    {

      level = (pHelp->level < 0) ? -1 * pHelp->level - 1 : pHelp->level;

      if ( level > get_trust( ch ))
	    continue;
	    
	if ( is_name( argall, pHelp->keyword ) )
	{
	    if ( pHelp->level >= 0 && str_cmp( argall, "imotd" ) )
	    {
		strcat(output, "{W=------=------=------=------=------=------=------=------=------=------=------={x\n\r" );
		strcat(output, pHelp->keyword );
		strcat(output, "\n\r" );
		strcat(output, "{W=------=------=------=------=------=------=------=------=------=------=------={x\n\r" );
		strcat(output, "\n\r" );
	    }

	    /*
	     * Strip leading '.' to allow initial blanks.
	     */
	    if ( pHelp->text[0] == '.' )
		strcat(output, pHelp->text+1 );
	    else
		strcat(output, pHelp->text );
	    page_to_char(output,ch);
	    return;
	}
    }

    send_to_char( "\n\rNo help on that word.\n\r", ch );
    return;
}


/* whois command */
void do_whois (CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    BUFFER *output;
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    bool found = FALSE;
    char org_string [40];
    char clan_string [40];
    char sur_string[40];

    one_argument(argument,arg);
  
    if (arg[0] == '\0')
    {
	send_to_char("You must provide a name.\n\r",ch);
	return;
    }

    output = new_buf();

    for (d = descriptor_list; d != NULL; d = d->next)
    {
	CHAR_DATA *wch;
      char clan_id[MSL];

 	if (d->connected != CON_PLAYING || !can_see(ch,d->character))
	    continue;
	
	wch = ( d->original != NULL ) ? d->original : d->character;

 	if (!can_see(ch,wch))
	    continue;

	    /* see if clan info should be displayed */
    	      clan_string[0] = 0;
		if( wch->clan_id != CLAN_NONE && !IS_NPC(ch) )
            {
	    /* only show if we're IMM, or same clan */
	        if( IS_IMMORTAL(ch)
	        || wch->level > LEVEL_HERO
	        || SAME_CLAN(ch, wch) )
	        {
		    sprintf( clan_string, "[%s] ",
		    org_table[CLAN( wch->clan_id )].upper_name );
	        }
		}

	    /* see if org info should be displayed */
    	      org_string[0] = 0;
		if( wch->org_id != ORG_NONE && !IS_NPC(ch) )
            {
	    /* only show if we're IMM, they're immort, or same org */
	       if( IS_IMMORTAL(ch)
	       || (wch->level > LEVEL_HERO)
	       || (SAME_ORG(ch, wch)))
	        {
		    sprintf( org_string, "-=%s=- ",
		    org_table[ORG( wch->org_id )].upper_name );
	        } 
		}

	  /* see if surname should be displayed */
	    sur_string[0] = 0;
	    if( (wch->surname != NULL)
          && (wch->surname != str_dup( "" )))
          {
 		sprintf( sur_string, " %s",wch->surname );
	    }

	if (!str_prefix(arg,wch->name))
	{
	    found = TRUE;

        sprintf(clan_id,"%s",( wch->clan_id != CLAN_NONE ) ? org_table[CLAN( ch->clan_id )].name : "");
    
	    /* a little formatting */
        sprintf( buf, "  %s%s%s%s%s%s%s%s%s%s%s%s %s %s%s%s\n\r",
	    wch->incog_level >= LEVEL_HERO ? "{c(I){x" : "",
	    wch->invis_level >= LEVEL_HERO ? "{W(W){x" : "",
	    IS_SET(wch->comm, COMM_AFK) ? "{M[AFK]{x" : "",
	    IS_SET(wch->comm, COMM_WRITING) ? "{M[Writing]{x" : "",
	    IS_SET(wch->comm, COMM_EDITOR) ? "{M[Editing]{x" : "",
          IS_SET(wch->act, PLR_KILLER) ? "{R(KILLER){x" : "",
          IS_SET(wch->act, PLR_THIEF)  ? "{R(THIEF){x"  : "",
          IS_SET(wch->act, PLR_ASSAULT)  ? "{R(ASSAULT){x"  : "",
          IS_SET(wch->act, PLR_TREASON)  ? "{R(TREASON){x"  : "",
          IS_SET(wch->act, PLR_OUTLAW)  ? "{R(OUTLAW){x"  : "",
	    org_string,
	    clan_string,
 	    wch->pcdata->pretit, 
	    wch->name,
	    sur_string,
	    IS_NPC(wch) ? "" :
	    IS_SET(wch->act, PLR_AUTOTITLE) ? wch->pcdata->title :
	    IS_IMMORTAL(ch) ? wch->pcdata->title : "");
	    add_buf(output,buf);
	}
    }

    if (!found)
    {
	send_to_char("No one of that name is playing.\n\r",ch);
	return;
    }

    page_to_char(buf_string(output),ch);
    free_buf(output);
}


/*
 * New 'who' command originally by Alander of Rivers of Mud.
 */
void do_who( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    BUFFER *output;
    DESCRIPTOR_DATA *d;
    int nMatch, wlevel;
    bool rgfClass[MAX_CLASS];
    bool rgfRace[MAX_PC_RACE];
    bool fClassRestrict = FALSE;
    bool fRaceRestrict = FALSE;
    bool fImmortalOnly = FALSE;
    bool orgRestrict = FALSE;
    bool clanRestrict = FALSE;
    char clan_string[40];
    char org_string[40];
    char sur_string[40];

    /*
     * show matching chars.
     */
    nMatch = 0;
    buf[0] = '\0';
    output = new_buf();

    {
	sprintf (buf, "{Y*{c========{b----------{c========{Y*{M The Legends of Krynn  {Y*{c========{b----------{c========{Y*{x\n\r");
    send_to_char (buf, ch);
    }

    sprintf( buf, "\n\r");
    add_buf(output,buf);
    sprintf( buf, "{y                   *---------> I M M O R T A L S <--------*{x\n\r");
    add_buf(output,buf);
    sprintf( buf, "{W|{c============================================================================={W|{x\n\r");;
    add_buf(output,buf);
    for( wlevel=MAX_LEVEL; wlevel>LEVEL_HERO; wlevel-- )
    {
      for ( d = descriptor_list; d != NULL; d = d->next )
      {
        CHAR_DATA *wch;
 
        /*
         * Check for match against restrictions.
         * Don't use trust as that exposes trusted mortals.
         */
        if ( d->connected != CON_PLAYING || !can_see( ch, d->character ) )
            continue;
 
        wch   = ( d->original != NULL ) ? d->original : d->character;

	  if (!can_see(ch,wch)  || wch->level < LEVEL_AVATAR)
	    continue;

	  if( wch->level != wlevel )
	    continue;

        if (( fImmortalOnly  && wch->level < LEVEL_AVATAR )
        || ( fClassRestrict && !rgfClass[wch->class] )
        || ( fRaceRestrict && !rgfRace[wch->race])
 	  || ( clanRestrict && !SAME_CLAN(ch, wch))
        || ( orgRestrict && !SAME_ORG(ch, wch)))
            continue;
         else
            nMatch++;

        /*
	   * Format it up.
	   */

	  /* see if clan info should be displayed */
	    clan_string[0] = 0;
	    if( wch->clan_id != CLAN_NONE && !IS_NPC(ch) )
          {
	  /* only show if we're immortal, or same clan */
	      if( IS_IMMORTAL(ch)
	      || wch->level >= LEVEL_HERO
	      || SAME_CLAN(ch, wch))
	      {
 		    sprintf( clan_string, "{c[%s]{x ",
		    org_table[CLAN( wch->clan_id )].upper_name );
	      }
	    }
	  /* see if org info should be displayed */
	    org_string[0] = 0;
	    if( wch->org_id != ORG_NONE && !IS_NPC(ch) )
          {
	  /* only show if we're immortal, or same org */
	      if( IS_IMMORTAL(ch)
	      || wch->level > LEVEL_HERO
	      ||( SAME_ORG(ch, wch)))
	      {
 		sprintf( org_string, "{c[ %s ]{x ",
		org_table[ORG(wch->org_id )].upper_name );
	      }
	    }

        sprintf( buf, "  %-17s%s%s%s%s%s%s%s%s%s%s%s %s %s%s\n\r",
	    org_string,
	    clan_string,
	    wch->incog_level >= LEVEL_HERO ? "{c(I){x" : "",
	    wch->invis_level >= LEVEL_HERO ? "{W(W){x" : "",
	    IS_SET(wch->comm, COMM_AFK) ? "{M[AFK]{x" : "",
	    IS_SET(wch->comm, COMM_WRITING) ? "{M[Writing]{x" : "",
	    IS_SET(wch->comm, COMM_EDITOR) ? "{M[Editing]{x" : "",
          IS_SET(wch->act, PLR_KILLER) ? "{R(KILLER){x" : "",
          IS_SET(wch->act, PLR_THIEF)  ? "{R(THIEF){x"  : "",
          IS_SET(wch->act, PLR_ASSAULT)  ? "{R(ASSAULT){x"  : "",
          IS_SET(wch->act, PLR_TREASON)  ? "{R(TREASON){x"  : "",
          IS_SET(wch->act, PLR_OUTLAW)  ? "{R(OUTLAW){x"  : "",
	    wch->pcdata->pretit, 
	    wch->name,
	    IS_NPC(wch) ? "" :
	    wch->pcdata->title);
        add_buf(output,buf);
      }
    }
    sprintf( buf, "\n\r");;
    add_buf(output,buf);
    sprintf( buf, "{y                     *---------> M O R T A L S <--------*{x\n\r");
    add_buf(output,buf);
    sprintf( buf, "{W|{c============================================================================={W|{x\n\r");;
    add_buf(output,buf);
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        CHAR_DATA *wch;
 
        /*
         * Check for match against restrictions.
         * Don't use trust as that exposes trusted mortals.
         */
        if ( d->connected != CON_PLAYING || !can_see( ch, d->character ) )
            continue;
 
        wch   = ( d->original != NULL ) ? d->original : d->character;

	  if (!can_see(ch,wch))
	    continue;

	  if( wch->level >= 102 
        ||  wch->level <= 0 )
	    continue;

        if (( fImmortalOnly  && wch->level < LEVEL_AVATAR )
        || ( fClassRestrict && !rgfClass[wch->class] )
        || ( fRaceRestrict && !rgfRace[wch->race])
        || ( clanRestrict && !SAME_CLAN(ch, wch))
        || ( orgRestrict && !SAME_ORG(ch, wch)))
            continue;
         else
            nMatch++;

        /*
	   * Format it up.
	   */

	  /* see if clan info should be displayed */
	    clan_string[0] = 0;
	    if( wch->clan_id != CLAN_NONE && !IS_NPC(ch) )
          {
	  /* only show if we're immortal, or same clan */
	      if( IS_IMMORTAL(ch)
	      || wch->level >= LEVEL_HERO
	      || SAME_CLAN(ch, wch))
	      {
 		    sprintf( clan_string, "{c[%s]{x ",
		    org_table[CLAN( wch->clan_id )].upper_name );
	      }
	    }
	  /* see if org info should be displayed */
	    org_string[0] = 0;
	    if( wch->org_id != ORG_NONE && !IS_NPC(ch) )
          {
	  /* only show if we're immortal, or same org */
	      if( IS_IMMORTAL(ch)
	      || wch->level > LEVEL_HERO
	      ||( SAME_ORG(ch, wch)))
	      {
 		sprintf( org_string, "{c[ %s ]{x ",
		org_table[ORG(wch->org_id )].upper_name );
	      }
	    }

	  /* see if surname should be displayed */
	    sur_string[0] = 0;
	    if( (wch->surname != NULL)
          && (wch->surname != str_dup( "" )))
          {
 		sprintf( sur_string, " %s",wch->surname );
	    }

        sprintf( buf, "  %-17s%s%s%s%s%s%s%s%s%s%s%s %s %s%s%s\n\r",
	    org_string,
	    clan_string,
	    wch->incog_level >= LEVEL_HERO ? "{c(I){x" : "",
	    wch->invis_level >= LEVEL_HERO ? "{W(W){x" : "",
	    IS_SET(wch->comm, COMM_AFK) ? "{M[AFK]{x" : "",
	    IS_SET(wch->comm, COMM_WRITING) ? "{M[Writing]{x" : "",
	    IS_SET(wch->comm, COMM_EDITOR) ? "{M[Editing]{x" : "",
          IS_SET(wch->act, PLR_KILLER) ? "{R(KILLER){x" : "",
          IS_SET(wch->act, PLR_THIEF)  ? "{R(THIEF){x"  : "",
          IS_SET(wch->act, PLR_ASSAULT)  ? "{R(ASSAULT){x"  : "",
          IS_SET(wch->act, PLR_TREASON)  ? "{R(TREASON){x"  : "",
          IS_SET(wch->act, PLR_OUTLAW)  ? "{R(OUTLAW){x"  : "",
	    wch->pcdata->pretit, 
	    wch->name,
	    sur_string,
	    IS_NPC(wch) ? "" :
	    IS_SET(wch->act, PLR_AUTOTITLE) ? wch->pcdata->title :
	    IS_IMMORTAL(ch) ? wch->pcdata->title : "");
        add_buf(output,buf);
    }
    sprintf( buf, "\n\r");;
    add_buf(output,buf);
    sprintf( buf2, "\n\rPlayers found: %d\n\r", nMatch );
    add_buf(output,buf2);
    sprintf( buf2, "\n\r** Remember, some players may be invisible to you!\n\r");
    add_buf(output,buf2);
    page_to_char( buf_string(output), ch );
    free_buf(output);
    return;
}

#define ptc printf_to_char

void do_count ( CHAR_DATA *ch, char *argument )
{
    int count, max;
    DESCRIPTOR_DATA *d;
	FILE *fp;

    count = 0;

    if ( IS_NPC(ch) || ch->desc == NULL )
    	return;

    for ( d = descriptor_list; d != NULL; d = d->next )
        if ( d->connected == CON_PLAYING && can_see( ch, d->character ) )
	    count++;

    max_on = UMAX(count,max_on);

	if ( ( fp = fopen( MAX_WHO_FILE,"r" ) ) == NULL )
	{
	    log_string("Error reading from maxwho.txt");
	    return;
	}
	max = fread_number( fp );
	fclose(fp);

	if ( max_on > max )
	{
		if ( ( fp = fopen( MAX_WHO_FILE,"w" ) ) == NULL )
		{
		    log_string("Error writing to maxwho.txt");
		    return;
		}
		fprintf( fp, "%d\n", max_on );
		fclose(fp);
	}

	ptc(ch,"The largest number of active players today was %d.\n\r", max_on );
	ptc(ch,"The largest number of active players ever was %d.\n\r", max );
	ptc(ch,"You can see %d character(s).\n\rSome characters may be invisible to you.\n\r\n\r", count );
}

void do_inventory( CHAR_DATA *ch, char *argument )
{
    send_to_char( "You are carrying:\n\r", ch );
    show_list_to_char( ch->carrying, ch, TRUE, TRUE );
    return;
}


void do_equipment( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    int iWear;

    send_to_char( "\n\r  Wear Slots:        You are using:\n\r", ch );
    send_to_char( "=====================================\n\r", ch );
    for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
    {
      obj = get_eq_char( ch, iWear );

	send_to_char( where_name[iWear], ch );

      if (obj != NULL)
      {
	  if ( can_see_obj( ch, obj ) )
	  {
	    send_to_char( format_obj_to_char( obj, ch, TRUE ), ch );
	    send_to_char( "\n\r", ch );
	  }
	  else
	  {
	    send_to_char( "   something.\n\r", ch );
	  }
      }
      else
	send_to_char( "       -\n\r", ch );
    }
    return;
}


void do_compare( CHAR_DATA *ch, char *argument )
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  OBJ_DATA *obj1;
  OBJ_DATA *obj2;
  int value1;
  int value2;
  char *msg;

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );
  if ( arg1[0] == '\0' )
  {
    send_to_char( "Compare what to what?\n\r", ch );
    return;
  }

  if ( ( obj1 = get_obj_carry( ch, arg1, ch ) ) == NULL )
  {
    send_to_char( "You do not have that item.\n\r", ch );
    return;
  }

  if (arg2[0] == '\0')
  {
    /* Going to comment this out. People are retards and don't understand that
     * if they do not use a second argument, it picks a random object in their
     * inventory and compares it automatically, keeping only in mind the item
     * type. They are too lazy to figure out, if they want to compare 2 items
     * specifically, they need to use the second argument. So, with it this way
     * they will have no choice BUT to enter in a second argument from now on.
     */
    //for (obj2 = ch->carrying; obj2 != NULL; obj2 = obj2->next_content)
    //{
    //    if (obj2->wear_loc != WEAR_NONE
    //    &&  can_see_obj(ch,obj2)
    //    &&  obj1->item_type == obj2->item_type
    //    &&  (obj1->wear_flags & obj2->wear_flags & ~ITEM_TAKE) != 0 )
    //	break;
    //}

    //if (obj2 == NULL)
    //{
    //    send_to_char("You aren't wearing anything comparable.\n\r",ch);
    //    return;
    //}

    send_to_char( "You must provide a second argument as to what you want to compare your first\n\r", ch );
    send_to_char( "argument to!\n\r", ch );
    send_to_char( "\n\r", ch );
    send_to_char( "Syntax: Compare <obj1> <obj2>\n\r", ch );
    send_to_char( "\n\r", ch );
    return;

  } 

  else if ( (obj2 = get_obj_carry(ch,arg2,ch) ) == NULL )
  {
    send_to_char("You do not have that item.\n\r",ch);
    return;
  }

  msg		= NULL;
  value1	= 0;
  value2	= 0;

  if ( obj1 == obj2 )
  {
    msg = "You compare $p to itself.  It looks about the same.";
  }
  else
  if ( obj1->item_type != obj2->item_type )
  {
    msg = "You can't compare $p and $P.";
  }
  else
  {
    switch ( obj1->item_type )
    {
	default:
	    msg = "You can't compare $p and $P.";
	    break;

	case ITEM_ARMOR:
	    value1 = obj1->value[0] + obj1->value[1] + obj1->value[2];
	    value2 = obj2->value[0] + obj2->value[1] + obj2->value[2];
	    break;

	case ITEM_WEAPON:
	    if (obj1->pIndexData->new_format)
		value1 = (1 + obj1->value[2]) * obj1->value[1];
	    else
	    	value1 = obj1->value[1] + obj1->value[2];

	    if (obj2->pIndexData->new_format)
		value2 = (1 + obj2->value[2]) * obj2->value[1];
	    else
	    	value2 = obj2->value[1] + obj2->value[2];
	    break;
    }
  }

  if ( msg == NULL )
  {
    if ( value1 == value2 )
      msg = "$p and $P look about the same.";
    else
    if ( value1  > value2 )
      msg = "$p looks better than $P.";
    else
      msg = "$p looks worse than $P.";
  }

  act( msg, ch, obj1, obj2, TO_CHAR );
  return;
}


void do_credits( CHAR_DATA *ch, char *argument )
{
    do_function(ch, &do_help, "diku" );
    return;
}


void do_where( CHAR_DATA *ch, char *argument )
{

    send_to_char( "Go find them yourself lazy!\n\r", ch );
    return;
}




void do_consider( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    char *msg;
    int diff;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Consider killing whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, NULL, arg ) ) == NULL )
    {
	send_to_char( "They're not here.\n\r", ch );
	return;
    }

    if (is_safe(ch,victim))
    {
	send_to_char("Don't even think about it.\n\r",ch);
	return;
    }

    if ( victim != NULL )
    {
	send_to_char("\n\r",ch);
	act( "You consider fighting with $N....", ch, NULL, victim, TO_CHAR );
	act( "Upon your initial size up of $M, you determine the following:", ch, NULL, victim, TO_CHAR );
	send_to_char("\n\r",ch);
    }

    if (get_curr_stat(victim,STAT_STR) > get_curr_stat(ch,STAT_STR))
    {
	act( "$N is most definitely stronger than you.", ch, NULL, victim, TO_CHAR );
    }
    else
    if (get_curr_stat(victim,STAT_STR) < get_curr_stat(ch,STAT_STR))
    {
	act( "You are much stronger than $N appears to be.", ch, NULL, victim, TO_CHAR );
    }
    else
    if (get_curr_stat(victim,STAT_STR) == get_curr_stat(ch,STAT_STR))
    {
	act( "$N's appears to have strength that matches even your own.", ch, NULL, victim, TO_CHAR );
    }


    if (get_curr_stat(victim,STAT_INT) > get_curr_stat(ch,STAT_INT))
    {
	act( "$N is more intelligent than you.", ch, NULL, victim, TO_CHAR );
    }
    else
    if (get_curr_stat(victim,STAT_INT) < get_curr_stat(ch,STAT_INT))
    {
	act( "Your intelligence is definitely superior.", ch, NULL, victim, TO_CHAR );
    }
    else
    if (get_curr_stat(victim,STAT_INT) == get_curr_stat(ch,STAT_INT))
    {
	act( "Your intelligence seems to match that of $S own.", ch, NULL, victim, TO_CHAR );
    }


    if (get_curr_stat(victim,STAT_WIS) > get_curr_stat(ch,STAT_WIS))
    {
	act( "$N's wisdom greatly exceeds that of your own.", ch, NULL, victim, TO_CHAR );
    }
    else
    if (get_curr_stat(victim,STAT_WIS) < get_curr_stat(ch,STAT_WIS))
    {
	act( "You are much wiser than $M.", ch, NULL, victim, TO_CHAR );
    }
    else
    if (get_curr_stat(victim,STAT_WIS) == get_curr_stat(ch,STAT_WIS))
    {
	act( "$N's wisdom appears to be equivalent to that of your own.", ch, NULL, victim, TO_CHAR );
    }


    if (get_curr_stat(victim,STAT_DEX) > get_curr_stat(ch,STAT_DEX))
    {
	act( "$E seems to be moving much quicker than you.", ch, NULL, victim, TO_CHAR );
    }
    else
    if (get_curr_stat(victim,STAT_DEX) < get_curr_stat(ch,STAT_DEX))
    {
	act( "You seem to be moving much quicker than $M.", ch, NULL, victim, TO_CHAR );
    }
    else
    if (get_curr_stat(victim,STAT_DEX) == get_curr_stat(ch,STAT_DEX))
    {
	act( "Your dexterous maneuverability appears to match that of $S own.", ch, NULL, victim, TO_CHAR );
    }

    send_to_char("\n\r",ch);
    send_to_char("Upon your final assessment, you come to the conclusion that\n\r",ch);

    diff = victim->level - ch->level;

         if ( diff <= -10 ) msg = "$E could be snapped like a twig.";
    else if ( diff <=  -8 ) msg = "$E looks like an easy kill.";
    else if ( diff <=  -5 ) msg = "$E could barely offer any resistance.";
    else if ( diff <=  -2 ) msg = "$E would give you little trouble.";
    else if ( diff <=   1 ) msg = "$E is a perfect match for you.";
    else if ( diff <=   4 ) msg = "$E would put up a good fight.";
    else if ( diff <=   7 ) msg = "$E would be difficult to defeat.";
    else if ( diff <=   9 ) msg = "You had better get some help for this one.";
    else                    msg = "You don't stand a chance against $M.";

    act( msg, ch, NULL, victim, TO_CHAR );
    return;
}



void set_title( CHAR_DATA *ch, char *title )
{
    char buf[MAX_STRING_LENGTH];

    if ( IS_NPC(ch) )
    {
	bug( "Set_title: NPC.", 0 );
	return;
    }

    if ( title[0] != '.' 
    && title[0] != ',' 
    && title[0] != '!'
    && title[0] != '?' )
    {
	buf[0] = ' ';
	strcpy( buf+1, title );
    }
    else
    {
	strcpy( buf, title );
    }

    free_string( ch->pcdata->title );
    ch->pcdata->title = str_dup( buf );
    return;
}

void do_title( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) )
	return;

    if ( IS_SET(ch->act, PLR_NOTITLE ) )
    {
    send_to_char( "Your title privledges have been revoked.\n\r", ch);
    return;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char( "Change your title to what?\n\r", ch );
	return;
    }

    if ( strlen(argument) > 45 )
	argument[45] = '\0';

    smash_tilde( argument );
    set_title( ch, argument );
    send_to_char( "Ok.\n\r", ch );
}

void do_report( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_INPUT_LENGTH];

    sprintf( buf,
	"You say 'I have %d/%d hp %d/%d mana %d/%d mv %d xp.'\n\r",
	ch->hit,  ch->max_hit,
	ch->mana, ch->max_mana,
	ch->move, ch->max_move,
	ch->exp   );

    send_to_char( buf, ch );

    sprintf( buf, "$n says 'I have %d/%d hp %d/%d mana %d/%d mv %d xp.'",
	ch->hit,  ch->max_hit,
	ch->mana, ch->max_mana,
	ch->move, ch->max_move,
	ch->exp   );

    act( buf, ch, NULL, NULL, TO_ROOM );

    return;
}



void do_practice( CHAR_DATA *ch, char *argument )
{
    char buf[4*MSL];
    BUFFER *buffer;
    int sn;

    if ( IS_NPC(ch) )
	return;

    if ( argument[0] == '\0' )
    {
	int col = 0;
	buffer = new_buf();

	send_to_char( "\n\r", ch );

	for ( sn = 0; sn < MAX_SKILL; sn++ )
	{
	  if ( skill_table[sn].name == NULL )
	    break;
	  if ( ch->level < skill_table[sn].skill_level[ch->class] 
	  || ch->pcdata->learned[sn] < 1 /* skill is not known */)
	    continue;

	    sprintf( buf, " %-18s%s %4d%s%% ",
		skill_table[sn].name,
            ch->pcdata->learned[sn] < 50 ? C_B_RED :
		ch->pcdata->learned[sn] < 75 ? C_YELLOW :
		ch->pcdata->learned[sn] < 99 ? C_GREEN : C_WHITE,
            ch->pcdata->learned[sn],
            CLEAR );
	    add_buf( buffer, buf );
	    if ( ++col % 3 == 0 )
		add_buf( buffer, "\n\r" );
      }

	if ( col % 3 != 0 )
	    add_buf( buffer, "\n\r" );

	sprintf( buf, "\n\rYou have %d practice sessions left.\n\r",ch->practice );
      add_buf( buffer, buf );
	page_to_char( buf_string(buffer), ch );
    }
    else
    {
	CHAR_DATA *mob;
	int adept;

	if ( !IS_AWAKE(ch) )
	{
	    send_to_char( "In your dreams, or what?\n\r", ch );
	    return;
	}

	for ( mob = ch->in_room->people; mob != NULL; mob = mob->next_in_room )
	{
	    if ( IS_NPC(mob) && IS_SET(mob->act, ACT_PRACTICE) )
		break;
	}

	if ( mob == NULL )
	{
	    send_to_char( "You can't do that here.\n\r", ch );
	    return;
	}

      if ((ch->in_room == get_room_index(ROOM_VNUM_ALTAR))
      &&  (ch->level >= 11))
	{
	    send_to_char( "You cannot practice here after level 10. You must now\n\r", ch );
	    send_to_char( "find your Guild if you have not done so already.\n\r", ch );
	    return;
	}

	if ( ch->practice <= 0 )
	{
	    send_to_char( "You have no practice sessions left.\n\r", ch );
	    return;
	}

	if ( ( sn = find_spell( ch,argument ) ) < 0
	|| ( !IS_NPC(ch)
	&&   (ch->level < skill_table[sn].skill_level[ch->class] 
 	||    ch->pcdata->learned[sn] < 1 /* skill is not known */
	||    skill_table[sn].rating[ch->class] == 0)))
	{
	    send_to_char( "You can't practice that.\n\r", ch );
	    return;
	}

	adept = IS_NPC(ch) ? 100 : class_table[ch->class].skill_adept;

	if ( ch->pcdata->learned[sn] >= adept )
	{
	    sprintf( buf, "You are already learned at %s.\n\r",
		skill_table[sn].name );
	    send_to_char( buf, ch );
	}
	else
	{
	    ch->practice--;
	    ch->pcdata->learned[sn] += 
		int_app[get_curr_stat(ch,STAT_INT)].learn / 
	        skill_table[sn].rating[ch->class];
	    if ( ch->pcdata->learned[sn] < adept )
	    {
		act( "You practice $T.",
		    ch, NULL, skill_table[sn].name, TO_CHAR );
		act( "$n practices $T.",
		    ch, NULL, skill_table[sn].name, TO_ROOM );
	    }
	    else
	    {
		ch->pcdata->learned[sn] = adept;
		act( "You are now learned at $T.",
		    ch, NULL, skill_table[sn].name, TO_CHAR );
		act( "$n is now learned at $T.",
		    ch, NULL, skill_table[sn].name, TO_ROOM );
	    }
	}
    }
    return;
}



/*
 * 'Wimpy' originally by Dionysos.
 */
void do_wimpy( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int wimpy;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
	wimpy = ch->max_hit / 5;
    else
	wimpy = atoi( arg );

    if ( wimpy < 0 )
    {
	send_to_char( "Your courage exceeds your wisdom.\n\r", ch );
	return;
    }

    if ( wimpy > ch->max_hit/2 )
    {
	send_to_char( "Such cowardice ill becomes you.\n\r", ch );
	return;
    }

    ch->wimpy	= wimpy;
    sprintf( buf, "Wimpy set to %d hit points.\n\r", wimpy );
    send_to_char( buf, ch );
    return;
}



void do_password( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char *pArg;
    char *pwdnew;
    char *p;
    char cEnd;

    if ( IS_NPC(ch) )
	return;

    /*
     * Can't use one_argument here because it smashes case.
     * So we just steal all its code.  Bleagh.
     */
    pArg = arg1;
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
	*pArg++ = *argument++;
    }
    *pArg = '\0';

    pArg = arg2;
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
	*pArg++ = *argument++;
    }
    *pArg = '\0';

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Syntax: password <old> <new>.\n\r", ch );
	return;
    }

    if ( strcmp( crypt( arg1, ch->pcdata->pwd ), ch->pcdata->pwd ) )
    {
	WAIT_STATE( ch, 40 );
	send_to_char( "Wrong password.  Wait 10 seconds.\n\r", ch );
	return;
    }

    if ( strlen(arg2) < 5 )
    {
	send_to_char(
	    "New password must be at least five characters long.\n\r", ch );
	return;
    }

    /*
     * No tilde allowed because of player file format.
     */
    pwdnew = crypt( arg2, ch->name );
    for ( p = pwdnew; *p != '\0'; p++ )
    {
	if ( *p == '~' )
	{
	    send_to_char(
		"New password not acceptable, try again.\n\r", ch );
	    return;
	}
    }

    free_string( ch->pcdata->pwd );
    ch->pcdata->pwd = str_dup( pwdnew );
    save_char_obj( ch );
    send_to_char( "Ok.\n\r", ch );
    return;
}

void do_email( CHAR_DATA *ch, char *argument )
{
int pos;
int approved=FALSE;
char buf[MAX_STRING_LENGTH];

 if ( IS_NPC(ch) )
 return;

 if ( argument[0] == '\0' )
  {
  send_to_char( "Set your e-mail address to what?\n\r", ch );
  return;
  }

 if( !strcmp( argument, "hiddukel" ) )
  {
  send_to_char( "Hiddukel's E-mail address is: cwalters2@chartermi.net \n\r", ch );
  return;
  }

 if( !strcmp( argument, "reorx" ) )
  {
  send_to_char( "Reorx's E-mail address is: jdogg0331@aol.com \n\r", ch );
  return;
  }

 if (!strchr(argument, '@'))
  {
  send_to_char( "I suspect your email address to be invalid.\n\r", ch );
  return;
  }
 if (strchr(argument, '~'))
  {
  send_to_char( "No tilde allowed for political reasons. Get a real email account.\n\r", ch );
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
            send_to_char( "Your domain name was deemed invalid by the parser. Try and be a little more creative.\n\r", ch );
		return;
		}
ch->pcdata->email = str_dup( argument );
send_to_char( "E-mail set.\n\r", ch );
return;
}

void do_glance( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_INPUT_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if (IS_NPC(ch))
	return;
    
    if ( arg[0] == '\0' )
    {
	send_to_char( "Glance at whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, NULL, arg ) ) == NULL )
    {
	send_to_char( "They're not here.\n\r", ch );
	return;
    }

    if (IS_IMMORTAL(victim)) 
    {
	send_to_char( "You are unable to determine their state of health.\n\r", ch );
	return;
    }

    if (!IS_NPC(ch) && !IS_IMMORTAL(ch))
    {
      if ( victim->description[0] != '\0' )
      {
	  send_to_char( victim->description, ch );
	  send_to_char( "\n\r", ch );
      }
      else
      {
	  act( "You see nothing special about $M.", ch, NULL, victim, TO_CHAR );
      }

      if(victim->hit >= (victim->max_hit - (victim->max_hit *10/100)))
      {
  	  sprintf(buf,"{YGlancing at %s, they seem to be in good health. [{r100{Y]{x\n\r",
          IS_NPC(victim) ? victim->short_descr : victim->name);
          send_to_char(buf,ch);
	  return;
      }
      else
      if((victim->hit < (victim->max_hit - (victim->max_hit *10/100)))
         && (victim->hit >= (victim->max_hit - (victim->max_hit *20/100))))
      {
	  sprintf(buf,"{YGlancing at %s, they look to be in moderate health. [{r90{Y]{x\n\r", IS_NPC(victim) ? victim->short_descr : victim->name);
          send_to_char(buf,ch);
	  return;
      }
      else
      if((victim->hit < (victim->max_hit - (victim->max_hit *20/100)))
         && (victim->hit >= (victim->max_hit - (victim->max_hit *30/100))))
      {
	  sprintf(buf,"{YGlancing at %s, they look to be slightly injured. [{r80{Y]{x\n\r", IS_NPC(victim) ? victim->short_descr : victim->name);
          send_to_char(buf,ch);
	  return;
      }
      else
      if((victim->hit < (victim->max_hit - (victim->max_hit *30/100)))
         && (victim->hit >= (victim->max_hit - (victim->max_hit *40/100))))
      {
	  sprintf(buf,"{YGlancing at %s, they look to be slightly bleeding. [{r70{Y]{x\n\r", IS_NPC(victim) ? victim->short_descr : victim->name);
          send_to_char(buf,ch);
	  return;
      }
      else
      if((victim->hit < (victim->max_hit - (victim->max_hit *40/100)))
         && (victim->hit >= (victim->max_hit - (victim->max_hit *50/100))))
      {
	  sprintf(buf,"{YGlancing at %s, they look to be injured and bleeding. [{r60{Y]{x\n\r", IS_NPC(victim) ? victim->short_descr : victim->name);
          send_to_char(buf,ch);
	  return;
      }
      else
      if((victim->hit < (victim->max_hit - (victim->max_hit *50/100)))
         && (victim->hit >= (victim->max_hit - (victim->max_hit *60/100))))
      {
	  sprintf(buf,"{YGlancing at %s, they look to be seriously injured. [{r50{Y]{x\n\r", IS_NPC(victim) ? victim->short_descr : victim->name);
          send_to_char(buf,ch);
	  return;
      }
      else
      if((victim->hit < (victim->max_hit - (victim->max_hit *60/100)))
         && (victim->hit >= (victim->max_hit - (victim->max_hit *70/100))))
      {
	  sprintf(buf,"{YGlancing at %s, they look to be seriously bleeding. [{r40{Y]{x\n\r", IS_NPC(victim) ? victim->short_descr : victim->name);
          send_to_char(buf,ch);
	  return;
      }
      else
      if((victim->hit < (victim->max_hit - (victim->max_hit *70/100)))
         && (victim->hit >= (victim->max_hit - (victim->max_hit *80/100))))
      {
	  sprintf(buf,"{YGlancing at %s, they look to be critically injured. [{r30{Y]{x\n\r", IS_NPC(victim) ? victim->short_descr : victim->name);
          send_to_char(buf,ch);
	  return;
      }
      else
      if((victim->hit < (victim->max_hit - (victim->max_hit *80/100)))
         && (victim->hit >= (victim->max_hit - (victim->max_hit *90/100))))
      {
	  sprintf(buf,"{YGlancing at %s, they look to be critically bleeding. [{r20{Y]{x\n\r", IS_NPC(victim) ? victim->short_descr : victim->name);
          send_to_char(buf,ch);
	  return;
      }
      else
      if((victim->hit < (victim->max_hit - (victim->max_hit *90/100)))
         && (victim->hit >= (victim->max_hit - (victim->max_hit *95/100))))
      {
	  sprintf(buf,"{YGlancing at %s, they look to be almost near death. [{r10{Y]{x\n\r", IS_NPC(victim) ? victim->short_descr : victim->name);
          send_to_char(buf,ch);
	  return;
      }
      else
      if((victim->hit < (victim->max_hit - (victim->max_hit *95/100)))
         && (victim->hit >= 1))
      {
	  sprintf(buf,"{YGlancing at %s, they look to be on the brink of death. [{r5{Y]{x\n\r", IS_NPC(victim) ? victim->short_descr : victim->name);
          send_to_char(buf,ch);
	  return;
      }
      else
      {
	  sprintf(buf,"{YGlancing at %s, you can't seem to determine their health.{x\n\r", IS_NPC(victim) ? victim->short_descr : victim->name);
          send_to_char(buf,ch);
	  return;
      }
    }
    else
    if (!IS_NPC(ch) && IS_IMMORTAL(ch))
    {
	 sprintf(buf, "{r[{YVnum %d{r]{Y$N {rhas %d/%dhp  %d/%dmn  %d/%dmv.{x\n\r{rLevel: %d{x\n\r{rAlign: %d{x\n\r{rDamroll: %d{x\n\r{rHitroll: %d{x\n\r",
	         IS_NPC(victim) ? victim->pIndexData->vnum : 0,
		 victim->hit, victim->max_hit,
		 victim->mana, victim->max_mana,
		 victim->move, victim->max_move,
		 victim->level,
		 victim->alignment,
	         GET_HITROLL(victim), GET_DAMROLL(victim));
         act( buf, ch, NULL, victim, TO_CHAR );
        
         sprintf(buf, "{rAct: %s{x\n\r",act_bit_name(victim->act));
         send_to_char(buf,ch);
    
         if (victim->comm)
         {
    	     sprintf(buf,"{rComm: %s{x\n\r",comm_bit_name(victim->comm));
    	     send_to_char(buf,ch);
         }

         if (IS_NPC(victim) && victim->off_flags)
         {
    	     sprintf(buf, "{rOffense: %s{x\n\r",off_bit_name(victim->off_flags));
	     send_to_char(buf,ch);
         }

         if (victim->imm_flags)
         {
	     sprintf(buf, "{rImmune: %s{x\n\r",imm_bit_name(victim->imm_flags));
	     send_to_char(buf,ch);
         }
 
         if (victim->res_flags)
         {
	     sprintf(buf, "{rResist: %s{x\n\r", imm_bit_name(victim->res_flags));
	     send_to_char(buf,ch);
         }

         if (victim->vuln_flags)
         {
	     sprintf(buf, "{rVulnerable: %s{x\n\r", imm_bit_name(victim->vuln_flags));
	     send_to_char(buf,ch);
         }
	    
         if (victim->affected_by)
         {
	     sprintf(buf, "{rAffected by %s{x\n\r", 
	         affect_bit_name(victim->affected_by));
	     send_to_char(buf,ch);
         }

         if (victim->affected2_by)
         {
	     sprintf(buf, "{rAffected2 by %s{x\n\r", 
	         affect2_bit_name(victim->affected2_by));
	     send_to_char(buf,ch);
         }

         if (victim->affected3_by)
         {
	     sprintf(buf, "{rAffected3 by %s{x\n\r", 
	         affect3_bit_name(victim->affected3_by));
	     send_to_char(buf,ch);
         }

	 return;
    }
    else
    {
	 sprintf(buf,"{YGlancing at %s, you can't seem to determine their health.{x\n\r", IS_NPC(victim) ? victim->short_descr : victim->name);
         send_to_char(buf,ch);
	 return;
    }
}

void do_version( CHAR_DATA *ch, char *argument) 
{
  send_to_char("        {y*************************************************{x\n\r",ch);
  send_to_char("        {y*           {RThis mud runs LoK v1.1              {y*{x\n\r",ch);
  send_to_char("        {y*                                               *{x\n\r",ch);
  send_to_char("        {y* {G             A derivitive of:                 {y*{x\n\r",ch);
  send_to_char("        {y* {G                  Rom2.4                      {y*{x\n\r",ch);
  send_to_char("        {y* {G                    &                         {y*{x\n\r",ch);
  send_to_char("        {y* {G                  Merc2.1                     {y*{x\n\r",ch);
  send_to_char("        {y*************************************************{x\n\r",ch);
  return;
}

void do_instruct( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    AFFECT_DATA af;
    int sn;
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( IS_NPC(ch) )
	return;

    if ( !IS_NPC( ch ) 
    && ch->level < skill_table[gsn_instruct].skill_level[ch->class] )
    {
      send_to_char( "Your don't know the first thing about instructing others.\n\r", ch );
      return;
    }

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
      send_to_char( "\n\r", ch );
      send_to_char( "Syntax: instruct <character> <skill>\n\r", ch );
      send_to_char( "Note: You MUST be at 100% with the skill you wish to instruct!\n\r", ch  );
      send_to_char( "\n\r", ch );
      return;
    }

    if ( ( victim = get_char_room( ch, NULL, arg1 ) ) == NULL )
    {
      send_to_char( "They aren't here.\n\r", ch );
      return;
    }

    if ( IS_AFFECTED3( ch, AFF_INSTRUCTION ))
    {
      send_to_char( "Your still too mentally fatigued from your last session.\n\r", ch );
      return;
    }

    if (!can_see(ch, victim))
    {
      send_to_char( "They aren't here.\n\r", ch );
      return;
    }

    if ( victim == ch )
    {
      send_to_char( "You can't instruct yourself.\n\r", ch );
      return;
    }

    if ( IS_NPC(victim) )
    {
      send_to_char( "You cannot instruct a MOB.\n\r", ch );
      return;
    }

    if (ch->class != victim->class)
    {
      send_to_char( "You cannot instruct someone of a different profession other than yours.\n\r", ch );
      return;
    }

    if (victim->position == POS_DEAD)
    {
      act("$E is dead and has no body.\n\r",ch, NULL, victim, TO_CHAR);
      return;
    }

    if (IS_IMMORTAL(victim))
    {
      send_to_char( "They already know everything.\n\r", ch);
      return;
    }

    if ( !IS_AWAKE(ch) )
    {
      send_to_char( "In your dreams, or what?\n\r", ch );
      return;
    }

    if ( !IS_AWAKE(victim) )
    {
      send_to_char( "They need to be awake to instruct them in anything!\n\r", ch );
      return;
    }

    if ( ( sn = find_spell( ch,arg2 ) ) < 0
    ||    ch->level < skill_table[sn].skill_level[ch->class] 
    ||    ch->pcdata->learned[sn] < 100
    ||    skill_table[sn].rating[ch->class] == 0 )
    {
      send_to_char( "You can't instruct them in that skill.\n\r", ch );
      return;
    }

    if ( ( sn = find_spell( victim,arg2 ) ) < 0
    ||   (victim->level < skill_table[sn].skill_level[ch->class] )
    ||    victim->pcdata->learned[sn] < 1
    ||    skill_table[sn].rating[victim->class] == 0 )
    {
      send_to_char( "They do not know that skill.\n\r", ch );
      return;
    }

    if ( victim->pcdata->learned[sn] >= 75 ) 
    {
      sprintf( buf, "They are already learned at %s.\n\r",skill_table[sn].name );
      send_to_char( buf, ch );
      return;
    }
    else
    {
      victim->pcdata->learned[sn] += 2;
      sprintf( buf, "You instruct %s in the ways of %s.\n\r",
	    victim->name,
	    skill_table[sn].name );
      send_to_char( buf, ch );
      sprintf( buf, "%s instructs you in the ways of %s.\n\r",
	    ch->name,
	    skill_table[sn].name );
      send_to_char( buf, victim );

      if (!IS_IMMORTAL(ch)) /* Small EXP for Instructor and AFFECT */
      {

        af.where			= TO_AFFECTS3;
        af.type			= gsn_instruct;
        af.level			= ch->level;
        af.duration		= 24;
        af.modifier		= 0;
        af.location		= 0;
        af.bitvector		= AFF_INSTRUCTION;
        affect_to_char( ch, &af );

        gain_exp( ch, number_range( 200, 400 ));
	  send_to_char( "You gained a small amount of experience from your period of instruction.\n\r", ch );
	}

    }

    return;
}

void do_occupancy( CHAR_DATA *ch, char *argument )
{
  if(ch->in_room->area->control_flags == 1)
  {
    send_to_char("This area is under the occupancy and control of the Solamnic Knights!\n\r",ch );
  }
  else
  if(ch->in_room->area->control_flags == 2)
  {
    send_to_char("This area is under the occupancy and control of the Knights of Takhisis!\n\r",ch );
  }
  else
  {
    send_to_char("This area is free of control from Military presence!\n\r",ch );
  }
  return;
}

void do_wilderness( CHAR_DATA *ch, char *argument )
{
  ROOM_INDEX_DATA  *in_room; 

  in_room = ch->in_room;

  if ( ch->desc == NULL || IS_NPC( ch ) )
    return;

  if ( ch->position < POS_SLEEPING )
  {
    send_to_char( "You can't see anything but stars!\n\r", ch );
    return;
  }

  if ( ch->position == POS_SLEEPING )
  {
    send_to_char( "You can't see anything, you're sleeping!\n\r", ch );
    return;
  }

  if (!IS_NPC(ch)
  && !IS_SET(ch->act,PLR_HOLYLIGHT)
  && IS_AFFECTED(ch, AFF_BLIND))
  { 
    send_to_char( "You can't see a thing!\n\r", ch ); 
    return; 
  }

  if ( number_percent( ) < ch->pcdata->learned[gsn_wilderness] )
  {
    act( "$n closes $s eyes, takes a few deep breaths, then re-opens $s eyes\n\r"
         "and begins to analyze $s surroundings.", ch, NULL, NULL, TO_NOTVICT);
    act( "You close your eyes and take a few deep breaths.  You then open your\n\r"
         "eyes and begin to analyze your surroundings.", ch, NULL, NULL, TO_CHAR);

    switch(in_room->sector_type)
    {
      case SECT_FIELD:
             send_to_char( "\n\rThrough your skilled wilderness lore, your analysis reveals:\n\r", ch );
		 send_to_char( "\n\rThis area is a wide open field, with only a few scattered\n\r", ch );
		 send_to_char( "trees here and there to obtain shelter.  You don't think you'll\n\r", ch );
		 send_to_char( "have much luck finding any herbs in this area.\n\r", ch );
		 send_to_char( "\n\r", ch ); 
             break;
      case SECT_DESERT:
             send_to_char( "\n\rThrough your skilled wilderness lore, your analysis reveals:\n\r", ch );
		 send_to_char( "\n\rThere is nothing but sand; sand all around you, devoid of any\n\r", ch );
		 send_to_char( "flora save for a few cactuses here and there.  There isn't a lot\n\r", ch );
		 send_to_char( "of sustainable life in this area either.  However, you remember\n\r", ch );
		 send_to_char( "hearing about a certain herb called woodsorrel being buried in sand.\n\r", ch );
		 send_to_char( "Perhaps a little herb finding will prove to be successful.\n\r", ch );
		 send_to_char( "\n\r", ch ); 
             break;
      case SECT_POLAR:
             send_to_char( "\n\rThrough your skilled wilderness lore, your analysis reveals:\n\r", ch );
		 send_to_char( "\n\rThere is nothing but snow and ice.  Definitely no plant life\n\r", ch );
		 send_to_char( "in these parts.  Hopefully you won't run into a snowstorm, or come\n\r", ch );
		 send_to_char( "across any snow beasts.  Searching further will more than likely turn\n\r", ch );
		 send_to_char( "up zero results for any sort of herb.\n\r", ch );
		 send_to_char( "\n\r", ch ); 
             break;
      case SECT_FOREST:
             send_to_char( "\n\rThrough your skilled wilderness lore, your analysis reveals:\n\r", ch );
		 send_to_char( "\n\rAhh... the forest.  There's nothing like being in an area\n\r", ch );
		 send_to_char( "you could call home.  Lavish plant life, plenty of wild animals\n\r", ch );
		 send_to_char( "and the smell of the air brings a smile to your face.  Your definitely\n\r", ch );
		 send_to_char( "in the right place to find many different kinds of herbs; herbs like:\n\r", ch );
		 send_to_char( "garlic bulbs, mint leaves, suaeysit mushrooms, acorns, yavethalion fruit,\n\r", ch );
		 send_to_char( "and winclamit berries.\n\r", ch );
		 send_to_char( "\n\r", ch ); 
             break;
      case SECT_HILLS:
             send_to_char( "\n\rThrough your skilled wilderness lore, your analysis reveals:\n\r", ch );
		 send_to_char( "\n\rAll around you are rolling hills.  The hills go up and down\n\r", ch );
		 send_to_char( "as far as the eye can see, similar to the waves of an ocean.  These\n\r", ch );
		 send_to_char( "hills are typically good places to find herbs such as: fungaboru fungi\n\r", ch );
		 send_to_char( "and goldbloom flowers.\n\r", ch );
		 send_to_char( "\n\r", ch ); 
             break;
      case SECT_MOUNTAIN:
             send_to_char( "\n\rThrough your skilled wilderness lore, your analysis reveals:\n\r", ch );
		 send_to_char( "\n\rThis mountainous area is extremely difficult to traverse, with steep inclines\n\r", ch );
		 send_to_char( "and rock formations that sometimes can make these mountains impassable.\n\r", ch );
		 send_to_char( "Despite the terrain, you know it's a good place to find herbs like: iceroot,\n\r", ch );
		 send_to_char( "and sprouts of stonewort. \n\r", ch );
		 send_to_char( "\n\r", ch ); 
             break;
      case SECT_SWAMP:
             send_to_char( "\n\rThrough your skilled wilderness lore, your analysis reveals:\n\r", ch );
		 send_to_char( "\n\rThe area is smells of swampy gas.  The humidity around here is\n\r", ch );
		 send_to_char( "extremely high; leading you to believe that this area would be a good\n\r", ch );
		 send_to_char( "place to find herbs like: tateesha nuts.\n\r", ch );
		 send_to_char( "\n\r", ch ); 
             break;
      case SECT_GRAVEYARD:
             send_to_char( "\n\rThrough your skilled wilderness lore, your analysis reveals:\n\r", ch );
		 send_to_char( "\n\rThe area is smells of death and nature.  It's an odd combination,\n\r", ch );
		 send_to_char( "one that most, save for the skilled Druid, could not identify.  Graveyards\n\r", ch );
		 send_to_char( "are always good places to find herbs like:  ghostroot and zulsendra mushrooms.\n\r", ch );
		 send_to_char( "\n\r", ch ); 
             break;
      case SECT_WATER_NOSWIM:
      case SECT_WATER_SWIM:
      case SECT_UNDERWATER:
             send_to_char( "\n\rThrough your skilled wilderness lore, your analysis reveals:\n\r", ch );
		 send_to_char( "\n\rWater is about the only thing you can see.  And despite that it\n\r", ch );
		 send_to_char( "isn't a Druid's first choice to call home, even water is another one\n\r", ch );
		 send_to_char( "nature's creations, hence it offers herbs as well, like: seaweed.\n\r", ch );
		 send_to_char( "\n\r", ch ); 
             break;
      case SECT_CITY:
             send_to_char( "\n\rYour ability to gain a good understanding of your surroundings is clouded\n\r", ch );
		 send_to_char( "by the area's lack of wilderness and other nature like environment.\n\r", ch );
		 send_to_char( "The only knowledge you gain from your lore is the firm notion that\n\r", ch );
		 send_to_char( "you really dislike it.  Too many people, too many buildings, paved roads,\n\r", ch );
		 send_to_char( "and a seemingly disregard for all that nature has to offer.\n\r", ch );
		 send_to_char( "\n\r", ch );  
             break;
      default:
             send_to_char( "\n\rYour wilderness lore skill reveals nothing about this area.  It's almost\n\r", ch );
		 send_to_char( "foreign, making it impossible to discern anything from it.\n\r", ch );
		 send_to_char( "\n\r", ch ); 
             break;
        break;
    }

    WAIT_STATE(ch,skill_table[gsn_wilderness].beats);  
    check_improve(ch,gsn_wilderness,TRUE,3);
  }
  else 
  {
    send_to_char( "You fail.\n\r", ch );
    WAIT_STATE(ch,skill_table[gsn_wilderness].beats);
    check_improve(ch,gsn_wilderness,FALSE,3);
  }

  return;
}

