/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,	   *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *									   *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael	   *
 *  Chastain, Michael Quan, and Mitchell Tse.				   *
 *									   *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc	   *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.						   *
 *									   *
 *  Much time and thought has gone into this software and you are	   *
 *  benefitting.  We hope that you share your changes too.  What goes	   *
 *  around, comes around.						   *
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

/* this is a listing of all the commands and command related data */

/* wrapper function for safe command execution */
void do_function args((CHAR_DATA *ch, DO_FUN *do_fun, char *argument));

/* for command types */
#define ML 	MAX_LEVEL	/* implementor */
#define L1	MAX_LEVEL - 1  	/* creator */
#define L2	MAX_LEVEL - 2	/* supreme being */
#define L3	MAX_LEVEL - 3	/* deity */
#define L4 	MAX_LEVEL - 4	/* god */
#define L5	MAX_LEVEL - 5	/* immortal */
#define L6	MAX_LEVEL - 6	/* demigod */
#define L7	MAX_LEVEL - 7	/* angel */
#define L8	MAX_LEVEL - 8	/* avatar */
#define IM	LEVEL_IMMORTAL 	/* avatar */
#define CL 	LEVEL_CLAN   	/* min level for clans */
#define HE	LEVEL_HERO	/* hero */

#define COM_INGORE	1


/*
 * Structure for a command in the command lookup table.
 */
struct	cmd_type
{
    char * const	name;
    DO_FUN *	do_fun;
    sh_int		position;
    sh_int		level;
    sh_int		log;
    sh_int        show;
    sh_int		cat;
};

/* the command table itself */
extern	const	struct	cmd_type	cmd_table	[];

/*
 * Command functions.
 * Defined in act_*.c (mostly).
 */
DECLARE_DO_FUN( do_addapply		);
DECLARE_DO_FUN(	do_admit		);
DECLARE_DO_FUN(	do_advance		);
DECLARE_DO_FUN( do_aedit		);
DECLARE_DO_FUN( do_affects		);
DECLARE_DO_FUN( do_afk			);
DECLARE_DO_FUN( do_alia			);
DECLARE_DO_FUN( do_alias		);
DECLARE_DO_FUN( do_alist		);
DECLARE_DO_FUN(	do_allow		);
DECLARE_DO_FUN( do_ambush		);
DECLARE_DO_FUN(	do_apprehend		);
DECLARE_DO_FUN(	do_apurge		);
DECLARE_DO_FUN(	do_areas		);
DECLARE_DO_FUN( do_asave		);
DECLARE_DO_FUN( do_asperson		);
DECLARE_DO_FUN(	do_assassinate		);
DECLARE_DO_FUN(	do_at			);
DECLARE_DO_FUN( do_autoassist		);
DECLARE_DO_FUN( do_autodig		);
DECLARE_DO_FUN( do_autoexit		);
DECLARE_DO_FUN( do_autogold		);
DECLARE_DO_FUN( do_autolist		);
DECLARE_DO_FUN( do_autoloot		);
DECLARE_DO_FUN( do_automercy 		);
DECLARE_DO_FUN( do_autosac		);
DECLARE_DO_FUN( do_autosplit		);
DECLARE_DO_FUN( do_autotitle 		);
DECLARE_DO_FUN( do_autoyield 		);
DECLARE_DO_FUN(	do_backstab		);
DECLARE_DO_FUN(	do_backup		);
DECLARE_DO_FUN( do_backcheck 		);
DECLARE_DO_FUN( do_backward 		);
DECLARE_DO_FUN(	do_background		);
DECLARE_DO_FUN( do_balance		);
DECLARE_DO_FUN(	do_bamfin		);
DECLARE_DO_FUN(	do_bamfout		);
DECLARE_DO_FUN(	do_ban			);
DECLARE_DO_FUN( do_bash			);
DECLARE_DO_FUN( do_battlecry    	);
DECLARE_DO_FUN( do_bear_call 		);
DECLARE_DO_FUN(	do_beastspite		);
DECLARE_DO_FUN( do_beep			);
DECLARE_DO_FUN( do_berserk		);
DECLARE_DO_FUN(	do_bestow_curse		);
DECLARE_DO_FUN( do_bind			);
DECLARE_DO_FUN( do_blackjack 		);
DECLARE_DO_FUN( do_bounty		);
DECLARE_DO_FUN(	do_brainjar		);
DECLARE_DO_FUN(	do_brandish		);
DECLARE_DO_FUN(	do_bravado		);
DECLARE_DO_FUN( do_break_weapon		);
DECLARE_DO_FUN( do_breakneck		);
DECLARE_DO_FUN(	do_brew			);
DECLARE_DO_FUN( do_brief		);
DECLARE_DO_FUN(	do_bug			);
DECLARE_DO_FUN( do_butcher 		);
DECLARE_DO_FUN(	do_buy			);
DECLARE_DO_FUN(	do_caltrops		);
DECLARE_DO_FUN( do_camouflage   	);
DECLARE_DO_FUN( do_campfire		);
DECLARE_DO_FUN(	do_cast			);
DECLARE_DO_FUN(	do_chain_repair		);
DECLARE_DO_FUN( do_changes		);
DECLARE_DO_FUN( do_channels		);
DECLARE_DO_FUN(	do_charge		);
DECLARE_DO_FUN( do_check		);
DECLARE_DO_FUN(	do_circle 		);
DECLARE_DO_FUN( do_clantalk		);
DECLARE_DO_FUN( do_clean		);
DECLARE_DO_FUN(	do_cleave		);
DECLARE_DO_FUN(	do_clist		);
DECLARE_DO_FUN( do_clone		);
DECLARE_DO_FUN(	do_close		);
DECLARE_DO_FUN(	do_colour		);
DECLARE_DO_FUN(	do_commands		);
DECLARE_DO_FUN( do_combine		);
DECLARE_DO_FUN( do_compact		);
DECLARE_DO_FUN(	do_compare		);
DECLARE_DO_FUN(	do_consider		);
DECLARE_DO_FUN(	do_copyover		);
DECLARE_DO_FUN( do_count		);
DECLARE_DO_FUN(	do_cpose		);
DECLARE_DO_FUN(	do_craft		);
DECLARE_DO_FUN(	do_credits		);
DECLARE_DO_FUN(	do_crush		);
DECLARE_DO_FUN(	do_cutthroat		);
DECLARE_DO_FUN( do_dancing_strike	);
DECLARE_DO_FUN( do_deaf			);
DECLARE_DO_FUN(	do_deathblade		);
DECLARE_DO_FUN( do_deathgrip    	);
DECLARE_DO_FUN( do_delet		);
DECLARE_DO_FUN( do_delete		);
DECLARE_DO_FUN( do_demand       	);
DECLARE_DO_FUN(	do_deny			);
DECLARE_DO_FUN( do_deposit		);
DECLARE_DO_FUN(	do_description		);
DECLARE_DO_FUN(	do_desocket		);
DECLARE_DO_FUN( do_devote 		);
DECLARE_DO_FUN( do_dirt			);
DECLARE_DO_FUN( do_disable		);
DECLARE_DO_FUN(	do_disarm		);
DECLARE_DO_FUN(	do_disconnect		);
DECLARE_DO_FUN(	do_disguise		);
DECLARE_DO_FUN(	do_dismantle		);
DECLARE_DO_FUN(	do_dismiss		);
DECLARE_DO_FUN( do_dismount 		);
DECLARE_DO_FUN( do_donate 		);
DECLARE_DO_FUN(	do_down			);
DECLARE_DO_FUN( do_drag         	);
DECLARE_DO_FUN(	do_drain		);
DECLARE_DO_FUN(	do_dreamevent		);
DECLARE_DO_FUN(	do_drink		);
DECLARE_DO_FUN(	do_drop			);
DECLARE_DO_FUN( do_dual_wield 		);
DECLARE_DO_FUN( do_dump			);
DECLARE_DO_FUN( do_earpunch		);
DECLARE_DO_FUN(	do_east			);
DECLARE_DO_FUN(	do_eat			);
DECLARE_DO_FUN(	do_echo			);
DECLARE_DO_FUN(	do_email		);
DECLARE_DO_FUN(	do_emote		);
DECLARE_DO_FUN( do_empower		);
DECLARE_DO_FUN(	do_endure		);
DECLARE_DO_FUN( do_enter		);
DECLARE_DO_FUN(	do_entwine		);
DECLARE_DO_FUN( do_envenom		);
DECLARE_DO_FUN(	do_equipment		);
DECLARE_DO_FUN(	do_eversharp		);
DECLARE_DO_FUN(	do_evict		);
DECLARE_DO_FUN(	do_examine		);
DECLARE_DO_FUN(	do_execute		);
DECLARE_DO_FUN(	do_exits		);
DECLARE_DO_FUN( do_exlist   		);
DECLARE_DO_FUN( do_extrabit 		);
DECLARE_DO_FUN(	do_eyerub		);
DECLARE_DO_FUN(	do_fadein		);
DECLARE_DO_FUN(	do_fadeout		);
DECLARE_DO_FUN(	do_fill			);
DECLARE_DO_FUN( do_find 		);
DECLARE_DO_FUN(	do_fingerbreak		);
DECLARE_DO_FUN( do_fire			);
DECLARE_DO_FUN( do_flag			);
DECLARE_DO_FUN(	do_flameblade		);
DECLARE_DO_FUN(	do_flank_attack		);
DECLARE_DO_FUN(	do_flee			);
DECLARE_DO_FUN(	do_fletching		);
DECLARE_DO_FUN(	do_follow		);
DECLARE_DO_FUN( do_for      		);
DECLARE_DO_FUN(	do_force		);
DECLARE_DO_FUN(	do_forge		);
DECLARE_DO_FUN(	do_freeze		);
DECLARE_DO_FUN( do_fremove 		);
DECLARE_DO_FUN( do_fslay 		);
DECLARE_DO_FUN(	do_fvlist		);
DECLARE_DO_FUN( do_gain			);
DECLARE_DO_FUN(	do_get			);
DECLARE_DO_FUN(	do_give			);
DECLARE_DO_FUN(	do_glance		);
DECLARE_DO_FUN( do_glide 		);
DECLARE_DO_FUN(	do_goto			);
DECLARE_DO_FUN( do_grab			);
DECLARE_DO_FUN(	do_group		);
DECLARE_DO_FUN(	do_gtell		);
DECLARE_DO_FUN( do_guard		);
DECLARE_DO_FUN( do_head_butt		);
DECLARE_DO_FUN( do_heal			);
DECLARE_DO_FUN(	do_hedit		);
DECLARE_DO_FUN(	do_heel			);
DECLARE_DO_FUN(	do_help			);
DECLARE_DO_FUN(	do_hero			);
DECLARE_DO_FUN(	do_hide			);
DECLARE_DO_FUN(	do_holylight		);
DECLARE_DO_FUN(	do_hunt			);
DECLARE_DO_FUN(	do_idea			);
DECLARE_DO_FUN( do_ignore 		);
DECLARE_DO_FUN(	do_igrab		);
DECLARE_DO_FUN( do_illegalname		);
DECLARE_DO_FUN(	do_immtalk		);
DECLARE_DO_FUN(	do_immortalfy		);
DECLARE_DO_FUN( do_imotd		);
DECLARE_DO_FUN(	do_impale		);
DECLARE_DO_FUN( do_incognito		);
DECLARE_DO_FUN(	do_inspire		);
DECLARE_DO_FUN(	do_instruct		);
DECLARE_DO_FUN(	do_inventory		);
DECLARE_DO_FUN(	do_invis		);
DECLARE_DO_FUN(	do_jab			);
DECLARE_DO_FUN(	do_joust		);
DECLARE_DO_FUN(	do_key_origin		);
DECLARE_DO_FUN(	do_kick			);
DECLARE_DO_FUN(	do_kill			);
DECLARE_DO_FUN( do_laston       	);
DECLARE_DO_FUN( do_laston_immortals	);
DECLARE_DO_FUN( do_land 		);
DECLARE_DO_FUN(	do_leather_repair	);
DECLARE_DO_FUN(	do_list			);
DECLARE_DO_FUN( do_load			);
DECLARE_DO_FUN(	do_lock			);
DECLARE_DO_FUN(	do_log			);
DECLARE_DO_FUN(	do_look			);
DECLARE_DO_FUN( do_lore 		);
DECLARE_DO_FUN( do_losereply		);
DECLARE_DO_FUN(	do_lull			);
DECLARE_DO_FUN( do_lunge 		);
DECLARE_DO_FUN( do_mark 		); /* Used to induct into religions */
DECLARE_DO_FUN(	do_maskip		);
DECLARE_DO_FUN(	do_masterwork		);
DECLARE_DO_FUN( do_medit		);
DECLARE_DO_FUN(	do_memory		);
DECLARE_DO_FUN(	do_mend			);
DECLARE_DO_FUN(	do_message		);
DECLARE_DO_FUN(	do_mfind		);
DECLARE_DO_FUN(	do_mload		);
DECLARE_DO_FUN(	do_mset			);
DECLARE_DO_FUN(	do_mstat		);
DECLARE_DO_FUN(	do_mwhere		);
DECLARE_DO_FUN( do_mob			);
DECLARE_DO_FUN( do_motd			);
DECLARE_DO_FUN( do_mount 		);
DECLARE_DO_FUN(	do_mountjack		);
DECLARE_DO_FUN( do_mpstat		);
DECLARE_DO_FUN( do_mpdump		);
DECLARE_DO_FUN( do_mpedit		);
DECLARE_DO_FUN(	do_murde		);
DECLARE_DO_FUN(	do_murder		);
DECLARE_DO_FUN( do_nerve_pinch 		);
DECLARE_DO_FUN( do_newlock		);
DECLARE_DO_FUN( do_news			);
DECLARE_DO_FUN( do_nochannels		);
DECLARE_DO_FUN(	do_noemote		);
DECLARE_DO_FUN( do_noexp		);
DECLARE_DO_FUN( do_nofollow		);
DECLARE_DO_FUN(	do_noidle		);
DECLARE_DO_FUN( do_noloot		);
DECLARE_DO_FUN( do_nonote		);
DECLARE_DO_FUN(	do_nopk			);
DECLARE_DO_FUN(	do_north		);
DECLARE_DO_FUN(	do_noshout		);
DECLARE_DO_FUN(	do_note			);
DECLARE_DO_FUN(	do_notell		);
DECLARE_DO_FUN( do_notitle		);
DECLARE_DO_FUN(	do_occupancy		);
DECLARE_DO_FUN( do_oedit		);
DECLARE_DO_FUN(	do_ofind		);
DECLARE_DO_FUN( do_olc			);
DECLARE_DO_FUN(	do_oload		);
DECLARE_DO_FUN( do_opdump 		);
DECLARE_DO_FUN( do_opedit 		);
DECLARE_DO_FUN( do_opstat 		);
DECLARE_DO_FUN(	do_open			);
DECLARE_DO_FUN(	do_order		);
DECLARE_DO_FUN(	do_oset			);
DECLARE_DO_FUN(	do_ostat		);
DECLARE_DO_FUN( do_outfit		);
DECLARE_DO_FUN( do_owhere		);
DECLARE_DO_FUN(	do_pacify		);
DECLARE_DO_FUN(	do_pardon		);
DECLARE_DO_FUN(	do_password		);
DECLARE_DO_FUN(	do_pfile_backup		);
DECLARE_DO_FUN(	do_peace		);
DECLARE_DO_FUN( do_pecho		);
DECLARE_DO_FUN( do_penalize		);
DECLARE_DO_FUN( do_penalty		);
DECLARE_DO_FUN( do_permban		);
DECLARE_DO_FUN(	do_pick			);
DECLARE_DO_FUN(	do_pickpocket		);
DECLARE_DO_FUN(	do_pillify		);
DECLARE_DO_FUN(	do_plate_repair		);
DECLARE_DO_FUN( do_play			);
DECLARE_DO_FUN( do_plist		);
DECLARE_DO_FUN( do_pload    		);
DECLARE_DO_FUN( do_pmote		);
DECLARE_DO_FUN(	do_polish		);
DECLARE_DO_FUN(	do_pose			);
DECLARE_DO_FUN( do_pour			);
DECLARE_DO_FUN(	do_pprestar		);
DECLARE_DO_FUN(	do_pprestart		);
DECLARE_DO_FUN(	do_practice		);
DECLARE_DO_FUN( do_pray        		);
DECLARE_DO_FUN(	do_prayfile		);
DECLARE_DO_FUN(	do_preach		);
DECLARE_DO_FUN( do_prefi		);
DECLARE_DO_FUN( do_prefix		);
DECLARE_DO_FUN( do_pretitle 		);
DECLARE_DO_FUN( do_prompt		);
DECLARE_DO_FUN( do_protect		);
DECLARE_DO_FUN( do_pry			);
DECLARE_DO_FUN( do_punload  		);
DECLARE_DO_FUN(	do_purge		);
DECLARE_DO_FUN( do_purifying_fire	);
DECLARE_DO_FUN( do_push         	);
DECLARE_DO_FUN(	do_put			);
DECLARE_DO_FUN(	do_quaff		);
DECLARE_DO_FUN( do_quest        	);
DECLARE_DO_FUN(	do_questreset		);
DECLARE_DO_FUN(	do_qui			);
DECLARE_DO_FUN( do_quiet		);
DECLARE_DO_FUN(	do_quit			);
DECLARE_DO_FUN(	do_rally		);
DECLARE_DO_FUN( do_ravage_bite 		);
DECLARE_DO_FUN( do_read			);
DECLARE_DO_FUN(	do_reboo		);
DECLARE_DO_FUN(	do_reboot		);
DECLARE_DO_FUN(	do_recall		);
DECLARE_DO_FUN(	do_recho		);
DECLARE_DO_FUN(	do_recite		);
DECLARE_DO_FUN( do_redit		);
DECLARE_DO_FUN(	do_relegate		); 
DECLARE_DO_FUN(	do_remove		);
DECLARE_DO_FUN( do_rename       	);
DECLARE_DO_FUN(	do_rent			);
DECLARE_DO_FUN(	do_repair_shield	);
DECLARE_DO_FUN( do_replay		);
DECLARE_DO_FUN(	do_reply		);
DECLARE_DO_FUN(	do_report		);
DECLARE_DO_FUN(	do_rescue		);
DECLARE_DO_FUN( do_resets		);
DECLARE_DO_FUN(	do_rest			);
DECLARE_DO_FUN(	do_restore		);
DECLARE_DO_FUN(	do_retool		);
DECLARE_DO_FUN( do_retribution  	);
DECLARE_DO_FUN(	do_return		);
DECLARE_DO_FUN(	do_reward		);
DECLARE_DO_FUN( do_roomcheck 		);
DECLARE_DO_FUN( do_rpdump 		);
DECLARE_DO_FUN( do_rpedit 		);
DECLARE_DO_FUN( do_rpstat 		);
DECLARE_DO_FUN(	do_rset			);
DECLARE_DO_FUN(	do_rstat		);
DECLARE_DO_FUN( do_rstrip 		); /* Used to remove from religions */
DECLARE_DO_FUN( do_rules		);
DECLARE_DO_FUN(	do_runeblade		);
DECLARE_DO_FUN(	do_rupture_organ	);
DECLARE_DO_FUN(	do_rwhere		);
DECLARE_DO_FUN(	do_sacrifice		);
DECLARE_DO_FUN(	do_salign		);
DECLARE_DO_FUN(	do_save			);
DECLARE_DO_FUN( save_guilds     	);
DECLARE_DO_FUN(	do_say			);
DECLARE_DO_FUN(	do_scan			);
DECLARE_DO_FUN(	do_scatter		);
DECLARE_DO_FUN(	do_scribe		);
DECLARE_DO_FUN(	do_score		);
DECLARE_DO_FUN(	do_scout		);
DECLARE_DO_FUN( do_scroll		);
DECLARE_DO_FUN( do_search_water 	);
DECLARE_DO_FUN(	do_sedit		);
DECLARE_DO_FUN(	do_sell			);
DECLARE_DO_FUN( do_sendhome		);
DECLARE_DO_FUN(	do_sendmail		);
DECLARE_DO_FUN(	do_serenade		);
DECLARE_DO_FUN( do_set			);
DECLARE_DO_FUN(	do_sgive		);
DECLARE_DO_FUN( do_sharpen		);
DECLARE_DO_FUN(	do_shield_rush		);
DECLARE_DO_FUN(	do_shield_shatter	);
DECLARE_DO_FUN( do_show			);
DECLARE_DO_FUN(	do_shutdow		);
DECLARE_DO_FUN(	do_shutdown		);
DECLARE_DO_FUN( do_sing 		);
DECLARE_DO_FUN( do_sit			);
DECLARE_DO_FUN( do_skills		);
DECLARE_DO_FUN(	do_sla			);
DECLARE_DO_FUN(	do_slay			);
DECLARE_DO_FUN(	do_sleep		);
DECLARE_DO_FUN(	do_sload		);
DECLARE_DO_FUN(	do_slookup		);
DECLARE_DO_FUN(	do_slowage		);
DECLARE_DO_FUN( do_smite        	);
DECLARE_DO_FUN(	do_smload		);
DECLARE_DO_FUN( do_smote		);
DECLARE_DO_FUN(	do_smother		);
DECLARE_DO_FUN(	do_sneak		);
DECLARE_DO_FUN(	do_snoop		);
DECLARE_DO_FUN( do_socials		);
DECLARE_DO_FUN(	do_soload		);
DECLARE_DO_FUN( do_song 		);
DECLARE_DO_FUN( do_songs 		);
DECLARE_DO_FUN(	do_south		);
DECLARE_DO_FUN( do_sockets		);
DECLARE_DO_FUN( do_speak		);
DECLARE_DO_FUN( do_spells		);
DECLARE_DO_FUN(	do_split		);
DECLARE_DO_FUN(	do_squire_call		);
DECLARE_DO_FUN(	do_sset			);
DECLARE_DO_FUN(	do_stainless		);
DECLARE_DO_FUN(	do_stalk		);
DECLARE_DO_FUN(	do_stand		);
DECLARE_DO_FUN( do_stat			);
DECLARE_DO_FUN(	do_steal		);
DECLARE_DO_FUN( do_story		);
DECLARE_DO_FUN( do_strangle		);
DECLARE_DO_FUN( do_string		);
DECLARE_DO_FUN(	do_strip		);
DECLARE_DO_FUN(	do_surge		);
DECLARE_DO_FUN(	do_surrender		);
DECLARE_DO_FUN(	do_sweep		);
DECLARE_DO_FUN(	do_switch		);
DECLARE_DO_FUN(	do_tail			);
DECLARE_DO_FUN(	do_tame			);
DECLARE_DO_FUN(	do_taunt		);
DECLARE_DO_FUN(	do_tell			);
DECLARE_DO_FUN(	do_tether		);
DECLARE_DO_FUN( do_tick	 		);
DECLARE_DO_FUN(	do_time			);
DECLARE_DO_FUN(	do_title		);
DECLARE_DO_FUN(	do_tonguecut		);
DECLARE_DO_FUN(	do_track		);
DECLARE_DO_FUN(	do_train		);
DECLARE_DO_FUN( do_traject 		);
DECLARE_DO_FUN(	do_trammel		);
DECLARE_DO_FUN( do_transin      	);
DECLARE_DO_FUN( do_transout     	);
DECLARE_DO_FUN(	do_transfer		);
DECLARE_DO_FUN(	do_treeform		);
DECLARE_DO_FUN( do_trip			);
DECLARE_DO_FUN( do_trophy 		);
DECLARE_DO_FUN(	do_trust		);
DECLARE_DO_FUN(	do_typo			);
DECLARE_DO_FUN( do_unalias		);
DECLARE_DO_FUN(	do_unholyrite		);
DECLARE_DO_FUN( do_unignore 		);
DECLARE_DO_FUN(	do_unlock		);
DECLARE_DO_FUN(	do_unmask		);
DECLARE_DO_FUN( do_unread		);
DECLARE_DO_FUN(	do_untether		);
DECLARE_DO_FUN(	do_up			);
DECLARE_DO_FUN( do_users		);
DECLARE_DO_FUN(	do_value		);
DECLARE_DO_FUN(	do_version		);
DECLARE_DO_FUN(	do_visible		);
DECLARE_DO_FUN( do_violate		);
DECLARE_DO_FUN( do_vnum			);
DECLARE_DO_FUN(	do_vsearch		);
DECLARE_DO_FUN(	do_wake			);
DECLARE_DO_FUN(	do_weapon_repair	);
DECLARE_DO_FUN(	do_wear			);
DECLARE_DO_FUN(	do_weather		);
DECLARE_DO_FUN( do_wedge		);
DECLARE_DO_FUN(	do_west			);
DECLARE_DO_FUN(	do_where		);
DECLARE_DO_FUN( do_whirlwind		);
DECLARE_DO_FUN(	do_whisper		);
DECLARE_DO_FUN(	do_who			);
DECLARE_DO_FUN( do_whois		);
DECLARE_DO_FUN(	do_wilderness		);
DECLARE_DO_FUN(	do_wimpy		);
DECLARE_DO_FUN(	do_windtalk		);
DECLARE_DO_FUN( do_withdraw		);
DECLARE_DO_FUN(	do_wizhelp		);
DECLARE_DO_FUN(	do_wizlock		);
DECLARE_DO_FUN( do_wizlist		);
DECLARE_DO_FUN( do_wiznet		);
DECLARE_DO_FUN( do_worth		);
DECLARE_DO_FUN(	do_yell			);
DECLARE_DO_FUN(	do_yield		);
DECLARE_DO_FUN(	do_zap			);
DECLARE_DO_FUN( do_zecho		);

