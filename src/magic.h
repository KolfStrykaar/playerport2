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


/*
 * Spell functions.
 * Defined in magic.c.
 */
DECLARE_SPELL_FUN(	spell_null				);
DECLARE_SPELL_FUN(	spell_acid_blast			);
DECLARE_SPELL_FUN(	spell_acid_breath			);
DECLARE_SPELL_FUN(	spell_aid				);
DECLARE_SPELL_FUN(	spell_alarm				);
DECLARE_SPELL_FUN(	spell_alter_reality		);
DECLARE_SPELL_FUN(	spell_animate_corpse		);
DECLARE_SPELL_FUN(	spell_armor				);
DECLARE_SPELL_FUN(	spell_attunement			);
DECLARE_SPELL_FUN(      spell_aura_cleanse      	);
DECLARE_SPELL_FUN(      spell_badberry          	);
DECLARE_SPELL_FUN(	spell_bad_luck	        	);
DECLARE_SPELL_FUN(	spell_banish			);
DECLARE_SPELL_FUN(	spell_bark_skin			);
DECLARE_SPELL_FUN(	spell_battlestaff			);
DECLARE_SPELL_FUN(	spell_battlefate			);
DECLARE_SPELL_FUN(	spell_battletide			);
DECLARE_SPELL_FUN(	spell_binding			);
DECLARE_SPELL_FUN(	spell_black_cairn			);
DECLARE_SPELL_FUN(      spell_blade_barrier         	);
DECLARE_SPELL_FUN(	spell_bless				);
DECLARE_SPELL_FUN(	spell_blessed_warmth		);
DECLARE_SPELL_FUN(	spell_blessed_watchfulness	);
DECLARE_SPELL_FUN(	spell_blindness			);
DECLARE_SPELL_FUN(	spell_blood_boil			);
DECLARE_SPELL_FUN(	spell_blood_bond			);
DECLARE_SPELL_FUN(	spell_break_enchantment		);
DECLARE_SPELL_FUN(      spell_breath_of_nature  	);
DECLARE_SPELL_FUN(	spell_breath_of_the_dead	);
DECLARE_SPELL_FUN(	spell_burning_hands		);
DECLARE_SPELL_FUN(	spell_call_lightning		);
DECLARE_SPELL_FUN(	spell_call_of_the_wild		);
DECLARE_SPELL_FUN(      spell_calm				);
DECLARE_SPELL_FUN(      spell_cancellation		);
DECLARE_SPELL_FUN(	spell_cantrip			);
DECLARE_SPELL_FUN(	spell_cause_critical		);
DECLARE_SPELL_FUN(	spell_cause_discord		);
DECLARE_SPELL_FUN(	spell_cause_light			);
DECLARE_SPELL_FUN(	spell_cause_serious		);
DECLARE_SPELL_FUN(	spell_cavorting_bones		);
DECLARE_SPELL_FUN(	spell_change_sex			);
DECLARE_SPELL_FUN(      spell_chain_lightning  		);
DECLARE_SPELL_FUN(	spell_chant_of_battle		);
DECLARE_SPELL_FUN(	spell_charm_person		);
DECLARE_SPELL_FUN(	spell_chaos_flux			);
DECLARE_SPELL_FUN(	spell_chill_touch			);
DECLARE_SPELL_FUN(	spell_chords_of_dissonance	);
DECLARE_SPELL_FUN(	spell_clear_thoughts		);
DECLARE_SPELL_FUN(	spell_cloak_starlight		);
DECLARE_SPELL_FUN(	spell_closing_weave		);
DECLARE_SPELL_FUN(      spell_coldfire          	);
DECLARE_SPELL_FUN(	spell_coldlight			);
DECLARE_SPELL_FUN(	spell_colour_spray		);
DECLARE_SPELL_FUN(	spell_conjure_food		);
DECLARE_SPELL_FUN(	spell_continual_light		);
DECLARE_SPELL_FUN(	spell_control_weather		);
DECLARE_SPELL_FUN(	spell_convoke_swarm 		);
DECLARE_SPELL_FUN(	spell_create_anvil		);
DECLARE_SPELL_FUN(	spell_create_barrel		);
DECLARE_SPELL_FUN(	spell_create_flame		);
DECLARE_SPELL_FUN(	spell_create_food			);
DECLARE_SPELL_FUN(	spell_create_furnace		);
DECLARE_SPELL_FUN(	spell_create_grinder		);
DECLARE_SPELL_FUN(	spell_create_rose			);
DECLARE_SPELL_FUN(	spell_create_spring		);
DECLARE_SPELL_FUN(	spell_create_water		);
DECLARE_SPELL_FUN(	spell_cure_blindness		);
DECLARE_SPELL_FUN(	spell_cure_critical		);
DECLARE_SPELL_FUN(	spell_cure_deafness		);
DECLARE_SPELL_FUN(      spell_cure_disease		);
DECLARE_SPELL_FUN(	spell_cure_light			);
DECLARE_SPELL_FUN(	spell_cure_poison			);
DECLARE_SPELL_FUN(	spell_cure_serious		);
DECLARE_SPELL_FUN(	spell_curse				);
DECLARE_SPELL_FUN(	spell_darkness			);
DECLARE_SPELL_FUN(	spell_dark_taint			);
DECLARE_SPELL_FUN(	spell_dazzle			);
DECLARE_SPELL_FUN(	spell_deadeye			);
DECLARE_SPELL_FUN(	spell_deafness			);
DECLARE_SPELL_FUN(      spell_demonfire			);
DECLARE_SPELL_FUN(	spell_detect_evil			);
DECLARE_SPELL_FUN(	spell_detect_good			);
DECLARE_SPELL_FUN(	spell_detect_hidden		);
DECLARE_SPELL_FUN(	spell_detect_presence		);
DECLARE_SPELL_FUN(	spell_dimensional_door		);
DECLARE_SPELL_FUN(	spell_disintegrate		);
DECLARE_SPELL_FUN(	spell_divine_favor		);
DECLARE_SPELL_FUN(      spell_doppelganger      	);
DECLARE_SPELL_FUN(	spell_dragon_skin    		);
DECLARE_SPELL_FUN(	spell_drain_rune			);
DECLARE_SPELL_FUN(	spell_detect_invis		);
DECLARE_SPELL_FUN(	spell_detect_magic		);
DECLARE_SPELL_FUN(	spell_detect_poison		);
DECLARE_SPELL_FUN(	spell_dispel_evil			);
DECLARE_SPELL_FUN(      spell_dispel_good       	);
DECLARE_SPELL_FUN(	spell_dispel_magic		);
DECLARE_SPELL_FUN(      spell_druidstaff        	);
DECLARE_SPELL_FUN(	spell_eagle_eye			);
DECLARE_SPELL_FUN(	spell_earthmeld			);
DECLARE_SPELL_FUN(	spell_earthquake			);
DECLARE_SPELL_FUN(	spell_empowerment			);
DECLARE_SPELL_FUN(	spell_enchant_armor		);
DECLARE_SPELL_FUN(	spell_enchant_weapon		);
DECLARE_SPELL_FUN(	spell_endure_disease		);
DECLARE_SPELL_FUN(	spell_energy_drain		);
DECLARE_SPELL_FUN(	spell_enervation			);
DECLARE_SPELL_FUN(	spell_enlarge			);
DECLARE_SPELL_FUN(	spell_enlightenment		);
DECLARE_SPELL_FUN(	spell_entangle			);
DECLARE_SPELL_FUN(	spell_ethereal_warrior  	);
DECLARE_SPELL_FUN(	spell_evil_cloak			);
DECLARE_SPELL_FUN(	spell_farsight			);
DECLARE_SPELL_FUN(	spell_fear				);
DECLARE_SPELL_FUN(	spell_feeblemind			);
DECLARE_SPELL_FUN(	spell_fireball			);
DECLARE_SPELL_FUN(	spell_fire_breath			);
DECLARE_SPELL_FUN(	spell_fireproof			);
DECLARE_SPELL_FUN(	spell_fire_seed			);
DECLARE_SPELL_FUN(	spell_firestorm			);
DECLARE_SPELL_FUN(	spell_firetongue			);
DECLARE_SPELL_FUN(	spell_flame_rune			);
DECLARE_SPELL_FUN(	spell_flame_shield  		);
DECLARE_SPELL_FUN(	spell_flamestrike			);
DECLARE_SPELL_FUN(	spell_flash				);
DECLARE_SPELL_FUN(	spell_flesh_to_stone		);
DECLARE_SPELL_FUN(	spell_frost_breath		);
DECLARE_SPELL_FUN(	spell_frost_rune			);
DECLARE_SPELL_FUN(      spell_flyingswords      	);
DECLARE_SPELL_FUN(      spell_forest_vision	 	);
DECLARE_SPELL_FUN(      spell_forget            	);
DECLARE_SPELL_FUN(	spell_forgedeath			);
DECLARE_SPELL_FUN(      spell_frenzy			);
DECLARE_SPELL_FUN(	spell_fumble			);
DECLARE_SPELL_FUN(	spell_garble			);
DECLARE_SPELL_FUN(	spell_gas_breath			);
DECLARE_SPELL_FUN(	spell_gaseous_form		);
DECLARE_SPELL_FUN(	spell_gate				);
DECLARE_SPELL_FUN(	spell_general_purpose		);
DECLARE_SPELL_FUN(	spell_giant_strength		);
DECLARE_SPELL_FUN(	spell_gift				);
DECLARE_SPELL_FUN(	spell_glamour			);
DECLARE_SPELL_FUN(	spell_glitterdust			);
DECLARE_SPELL_FUN(	spell_god_fire			);
DECLARE_SPELL_FUN(	spell_goodberry			);
DECLARE_SPELL_FUN(	spell_grandeur			);
DECLARE_SPELL_FUN(	spell_grasping_roots		);
DECLARE_SPELL_FUN(	spell_gravity_flux		);
DECLARE_SPELL_FUN(	spell_great_wasting		);
DECLARE_SPELL_FUN(	spell_gust_of_wind		);
DECLARE_SPELL_FUN(	spell_harm				);
DECLARE_SPELL_FUN(      spell_haste				);
DECLARE_SPELL_FUN(	spell_heal				);
DECLARE_SPELL_FUN(	spell_heat_metal			);
DECLARE_SPELL_FUN(	spell_high_explosive		);
DECLARE_SPELL_FUN(      spell_holy_word			);
DECLARE_SPELL_FUN(	spell_horrid_wilting		);
DECLARE_SPELL_FUN(	spell_iceblast			);
DECLARE_SPELL_FUN(	spell_ice_shield			);
DECLARE_SPELL_FUN(	spell_identify			);
DECLARE_SPELL_FUN(	spell_illusion			);
DECLARE_SPELL_FUN(	spell_immolation			);
DECLARE_SPELL_FUN(	spell_incendiary_cloud		);
DECLARE_SPELL_FUN(	spell_infravision			);
DECLARE_SPELL_FUN(	spell_intensify_death		);
DECLARE_SPELL_FUN(	spell_invigorate			);
DECLARE_SPELL_FUN(	spell_invis				);
DECLARE_SPELL_FUN(      spell_knock             	);
DECLARE_SPELL_FUN(	spell_know_alignment		);
DECLARE_SPELL_FUN(	spell_levitate			);
DECLARE_SPELL_FUN(	spell_life_transfer		);
DECLARE_SPELL_FUN(	spell_lightning_breath		);
DECLARE_SPELL_FUN(	spell_locate_object		);
DECLARE_SPELL_FUN(	spell_lock				);
DECLARE_SPELL_FUN(	spell_looking_glass		);
DECLARE_SPELL_FUN(	spell_luck				);
DECLARE_SPELL_FUN(	spell_magic_missile		);
DECLARE_SPELL_FUN(	spell_mark_of_origin		);
DECLARE_SPELL_FUN(      spell_mass_healing		);
DECLARE_SPELL_FUN(	spell_mass_invis			);
DECLARE_SPELL_FUN(	spell_mind_shatter		);
DECLARE_SPELL_FUN(	spell_mirror			);
DECLARE_SPELL_FUN(	spell_mooncloak			);
DECLARE_SPELL_FUN(      spell_natures_embrace    	);
DECLARE_SPELL_FUN(      spell_natures_warmth    	);
DECLARE_SPELL_FUN(	spell_nexus				);
DECLARE_SPELL_FUN(	spell_nightmare			);
DECLARE_SPELL_FUN(	spell_orb_of_containment	);
DECLARE_SPELL_FUN(	spell_paralysis			);
DECLARE_SPELL_FUN(	spell_pass_door			);
DECLARE_SPELL_FUN(	spell_pass_without_trace	);
DECLARE_SPELL_FUN(	spell_phantom_armor		);
DECLARE_SPELL_FUN(	spell_pine_needles		);
DECLARE_SPELL_FUN(      spell_plague			);
DECLARE_SPELL_FUN(	spell_plant_growth		);
DECLARE_SPELL_FUN(	spell_poison			);
DECLARE_SPELL_FUN(	spell_preserve_dead		);
DECLARE_SPELL_FUN(	spell_preserve_part		);
DECLARE_SPELL_FUN(	spell_protection_evil		);
DECLARE_SPELL_FUN(	spell_protection_good		);
DECLARE_SPELL_FUN(	spell_purify			);
DECLARE_SPELL_FUN(	spell_quench			);
DECLARE_SPELL_FUN(	spell_quicksand			);
DECLARE_SPELL_FUN(	spell_rabbit_summon		);
DECLARE_SPELL_FUN(	spell_rainbow_pattern  		);
DECLARE_SPELL_FUN(	spell_ray_of_fatigue		);
DECLARE_SPELL_FUN(	spell_ray_of_truth		);
DECLARE_SPELL_FUN(	spell_recharge			);
DECLARE_SPELL_FUN(	spell_refresh			);
DECLARE_SPELL_FUN(	spell_regeneration		);
DECLARE_SPELL_FUN(	spell_remove_curse		);
DECLARE_SPELL_FUN(	spell_remove_paralysis		);
DECLARE_SPELL_FUN(	spell_renew_bones			);
DECLARE_SPELL_FUN(	spell_repulsion			);
DECLARE_SPELL_FUN(	spell_resist_cold			);
DECLARE_SPELL_FUN(	spell_salvation			);
DECLARE_SPELL_FUN(	spell_sanctuary			);
DECLARE_SPELL_FUN(	spell_sate				);
DECLARE_SPELL_FUN(	spell_scrye				);
DECLARE_SPELL_FUN(	spell_seal_door			);
DECLARE_SPELL_FUN(	spell_sequester			);
DECLARE_SPELL_FUN(	spell_shackles			);
DECLARE_SPELL_FUN(	spell_shadow_walk			);
DECLARE_SPELL_FUN(	spell_shallow_breath		);
DECLARE_SPELL_FUN(	spell_shield			);
DECLARE_SPELL_FUN(	spell_shifting_shadows		);
DECLARE_SPELL_FUN(	spell_shocking_grasp		);
DECLARE_SPELL_FUN(	spell_shocking_rune		);
DECLARE_SPELL_FUN(	spell_shrink			);
DECLARE_SPELL_FUN(	spell_silence			);
DECLARE_SPELL_FUN(	spell_sleep				);
DECLARE_SPELL_FUN(	spell_sleepless_curse		);
DECLARE_SPELL_FUN(	spell_slow				);
DECLARE_SPELL_FUN(	spell_sparkshot			);
DECLARE_SPELL_FUN(	spell_spiritual_hammer		);
DECLARE_SPELL_FUN(	spell_spook				);
DECLARE_SPELL_FUN(	spell_sticks_to_snakes		);
DECLARE_SPELL_FUN(      spell_stonespirit       	);
DECLARE_SPELL_FUN(	spell_storm_of_vengeance	);
DECLARE_SPELL_FUN(	spell_summon			);
DECLARE_SPELL_FUN(      spell_summon_insects    	);
DECLARE_SPELL_FUN(	spell_sunbeam			);
DECLARE_SPELL_FUN(	spell_sunray			);
DECLARE_SPELL_FUN(	spell_sympathy			);
DECLARE_SPELL_FUN(	spell_teleport			);
DECLARE_SPELL_FUN(	spell_thors_hammer		);
DECLARE_SPELL_FUN(	spell_thunder_clap		);
DECLARE_SPELL_FUN(	spell_torment			);
DECLARE_SPELL_FUN(	spell_truefire			);
DECLARE_SPELL_FUN(	spell_undead_disruption		);
DECLARE_SPELL_FUN(	spell_undetectable_align	);
DECLARE_SPELL_FUN(	spell_unholy_armor		);
DECLARE_SPELL_FUN(	spell_unseal_door			);
DECLARE_SPELL_FUN(	spell_unseen_servant  		);
DECLARE_SPELL_FUN(	spell_vampiric_touch		);
DECLARE_SPELL_FUN(	spell_ventriloquate		);
DECLARE_SPELL_FUN(	spell_vermin_plague		);
DECLARE_SPELL_FUN(	spell_vortex			);
DECLARE_SPELL_FUN(	spell_ward				);
DECLARE_SPELL_FUN(	spell_waterlungs			);
DECLARE_SPELL_FUN(	spell_weaken			);
DECLARE_SPELL_FUN(      spell_windspirit        	);
DECLARE_SPELL_FUN(	spell_wind_wall			);
DECLARE_SPELL_FUN(	spell_word_of_death		);
DECLARE_SPELL_FUN(	spell_word_of_recall		);
DECLARE_SPELL_FUN(	spell_wraithform			);
DECLARE_SPELL_FUN(	spell_wrath				);
DECLARE_SPELL_FUN(	spell_wrath_of_nature		);

