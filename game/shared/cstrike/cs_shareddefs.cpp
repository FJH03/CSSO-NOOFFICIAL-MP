//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "cs_shareddefs.h"

const float CS_PLAYER_SPEED_RUN			= 260.0f;
//const float CS_PLAYER_SPEED_WALK		= 100.0f;
const float CS_PLAYER_SPEED_STOPPED		=   1.0f;
const float CS_PLAYER_SPEED_HAS_HOSTAGE	= 200.0f;
const float CS_PLAYER_SPEED_OBSERVER	= 900.0f;

const float CS_PLAYER_SPEED_DUCK_MODIFIER	= 0.34f;
const float CS_PLAYER_SPEED_WALK_MODIFIER	= 0.52f;
const float CS_PLAYER_SPEED_CLIMB_MODIFIER	= 0.34f;

const float CS_PLAYER_DUCK_SPEED_IDEAL = 8.0f;


static PlayerGloves s_playerGloves[MAX_GLOVES+1] =
{
	{ NULL, NULL },

	{ "models/weapons/v_models/arms/glove_bloodhound/v_glove_bloodhound.mdl",				"models/weapons/w_models/arms/w_glove_bloodhound.mdl"				},
	{ "models/weapons/v_models/arms/glove_bloodhound/v_glove_bloodhound_perfectworld.mdl",	"models/weapons/w_models/arms/w_glove_bloodhound_perfectworld.mdl"	},
	{ "models/weapons/v_models/arms/glove_bloodhound/v_glove_bloodhound_brokenfang.mdl",	"models/weapons/w_models/arms/w_glove_bloodhound_brokenfang.mdl"	},
	{ "models/weapons/v_models/arms/glove_bloodhound/v_glove_bloodhound_hydra.mdl",			"models/weapons/w_models/arms/w_glove_bloodhound_hydra.mdl"			},
	{ "models/weapons/v_models/arms/glove_fingerless/v_glove_fingerless.mdl",				"models/weapons/w_models/arms/w_glove_fingerless.mdl"				},
	{ "models/weapons/v_models/arms/glove_fullfinger/v_glove_fullfinger.mdl",				"models/weapons/w_models/arms/w_glove_fullfinger.mdl"				},
	{ "models/weapons/v_models/arms/glove_handwrap_leathery/v_glove_handwrap_leathery.mdl",	"models/weapons/w_models/arms/w_glove_handwrap_leathery.mdl"		},
	{ "models/weapons/v_models/arms/glove_hardknuckle/v_glove_hardknuckle.mdl",				"models/weapons/w_models/arms/w_glove_hardknuckle.mdl"				},
	{ "models/weapons/v_models/arms/glove_hardknuckle/v_glove_hardknuckle_black.mdl",		"models/weapons/w_models/arms/w_glove_hardknuckle_black.mdl"		},
	{ "models/weapons/v_models/arms/glove_hardknuckle/v_glove_hardknuckle_blue.mdl",		"models/weapons/w_models/arms/w_glove_hardknuckle_blue.mdl"			},
	{ "models/weapons/v_models/arms/glove_motorcycle/v_glove_motorcycle.mdl",				"models/weapons/w_models/arms/w_glove_motorcycle.mdl"				},
	{ "models/weapons/v_models/arms/glove_slick/v_glove_slick.mdl",							"models/weapons/w_models/arms/w_glove_slick.mdl"					},
	{ "models/weapons/v_models/arms/glove_specialist/v_glove_specialist.mdl",				"models/weapons/w_models/arms/w_glove_specialist.mdl"				},
	{ "models/weapons/v_models/arms/glove_sporty/v_glove_sporty.mdl",						"models/weapons/w_models/arms/w_glove_sporty.mdl"					},
	{ "models/weapons/v_models/arms/glove_sas_old/v_glove_sas_old.mdl",						"models/weapons/w_models/arms/w_glove_sas_old.mdl"					},
	{ "models/weapons/v_models/arms/glove_fbi_old/v_glove_fbi_old.mdl",						"models/weapons/w_models/arms/w_glove_fbi_old.mdl"					},
	{ "models/weapons/v_models/arms/glove_phoenix_old/v_glove_phoenix_old.mdl",				"models/weapons/w_models/arms/w_glove_phoenix_old.mdl"				},
	{ "models/weapons/v_models/arms/glove_leet_old/v_glove_leet_old.mdl",					"models/weapons/w_models/arms/w_glove_leet_old.mdl"					},
	{ "models/weapons/v_models/arms/bare/v_bare_hands.mdl",									"models/weapons/w_models/arms/w_glove_bare_hands.mdl"				},
};

const PlayerGloves* GetGlovesInfo( int i )
{
	Assert( i >= 0 && i < ARRAYSIZE( s_playerGloves ) );
	if ( i < 0 || i >= ARRAYSIZE( s_playerGloves ) )
		return &s_playerGloves[1]; // 0 is all null so return the first valid index
	return &s_playerGloves[i];
}

const PlayerViewmodelArmConfig *GetPlayerViewmodelArmConfigForPlayerModel( const char* szPlayerModel )
{
	if ( szPlayerModel != NULL )
	{
		for ( int i=0; i<ARRAYSIZE(s_playerViewmodelArmConfigs); i++ )
		{
			if ( V_stristr( szPlayerModel, s_playerViewmodelArmConfigs[i].szPlayerModelSearchSubStr ) )
				return &s_playerViewmodelArmConfigs[i];
		}
	}

	AssertMsg1( false, "Could not determine viewmodel config for character model: %s", szPlayerModel );
	return &s_playerViewmodelArmConfigs[0];
}

CCSAgentInfo g_AgentInfosCT[MAX_AGENTS_CT + 1] =
{
	// default
	{ "models/player/custom_player/legacy/ctm_sas.mdl",					NULL,					NULL,					NULL	},
	// Shattered Web
	{ "models/player/custom_player/legacy/ctm_fbi_variantf.mdl",		NULL,					CS_CLASS_FBI,			false	},
	{ "models/player/custom_player/legacy/ctm_fbi_variantf_legacy.mdl",	NULL,					CS_CLASS_FBI,			false	},
	{ "models/player/custom_player/legacy/ctm_fbi_variantg.mdl",		NULL,					CS_CLASS_FBI,			false	},
	{ "models/player/custom_player/legacy/ctm_fbi_varianth.mdl",		NULL,					CS_CLASS_FBI,			false	},
	{ "models/player/custom_player/legacy/ctm_fbi_variantb.mdl",		"fbihrt_epic",			CS_CLASS_FBI,			true	},
	{ "models/player/custom_player/legacy/ctm_sas_variantf.mdl",		NULL,					CS_CLASS_SAS,			false	},
	{ "models/player/custom_player/legacy/ctm_st6_variantk.mdl",		"ctm_gsg9",				CS_CLASS_SEAL_TEAM_6,	false	},
	{ "models/player/custom_player/legacy/ctm_st6_variantk_legacy.mdl",	"ctm_gsg9",				CS_CLASS_SEAL_TEAM_6,	false	},
	{ "models/player/custom_player/legacy/ctm_st6_variante.mdl",		NULL,					CS_CLASS_SEAL_TEAM_6,	false	},
	{ "models/player/custom_player/legacy/ctm_st6_variante_legacy.mdl",	NULL,					CS_CLASS_SEAL_TEAM_6,	false	},
	{ "models/player/custom_player/legacy/ctm_st6_variantg.mdl",		NULL,					CS_CLASS_SEAL_TEAM_6,	false	},
	{ "models/player/custom_player/legacy/ctm_st6_variantm.mdl",		NULL,					CS_CLASS_SEAL_TEAM_6,	false	},
	{ "models/player/custom_player/legacy/ctm_st6_variantm_legacy.mdl",	NULL,					CS_CLASS_SEAL_TEAM_6,	false	},
	{ "models/player/custom_player/legacy/ctm_st6_varianti.mdl",		"seal_epic",			CS_CLASS_SEAL_TEAM_6,	false	},
	{ "models/player/custom_player/legacy/ctm_st6_varianti_legacy.mdl",	"seal_epic",			CS_CLASS_SEAL_TEAM_6,	false	},
	// Broken Fang
	{ "models/player/custom_player/legacy/ctm_swat_variantj.mdl",		NULL,					CS_CLASS_SWAT,			false	},
	{ "models/player/custom_player/legacy/ctm_swat_varianth.mdl",		NULL,					CS_CLASS_SWAT,			false	},
	{ "models/player/custom_player/legacy/ctm_st6_variantj.mdl",		NULL,					CS_CLASS_SEAL_TEAM_6,	false	},
	{ "models/player/custom_player/legacy/ctm_swat_variantg.mdl",		NULL,					CS_CLASS_SWAT,			false	},
	{ "models/player/custom_player/legacy/ctm_swat_varianti.mdl",		NULL,					CS_CLASS_SWAT,			false	},
	{ "models/player/custom_player/legacy/ctm_swat_variantf.mdl",		"swat_fem",				CS_CLASS_SWAT,			true	},
	{ "models/player/custom_player/legacy/ctm_st6_variantl.mdl",		NULL,					CS_CLASS_SEAL_TEAM_6,	false	},
	{ "models/player/custom_player/legacy/ctm_swat_variante.mdl",		"swat_epic",			CS_CLASS_SWAT,			true	},
	// Riptide
	{ "models/player/custom_player/legacy/ctm_diver_varianta.mdl",		"seal_fem",				CS_CLASS_SEAL_TEAM_6,	true	},
	{ "models/player/custom_player/legacy/ctm_diver_variantb.mdl",		"seal_diver_01",		CS_CLASS_SEAL_TEAM_6,	false	},
	{ "models/player/custom_player/legacy/ctm_diver_variantc.mdl",		"seal_diver_02",		CS_CLASS_SEAL_TEAM_6,	false	},
	{ "models/player/custom_player/legacy/ctm_gendarmerie_varianta.mdl","gendarmerie_male",		CS_CLASS_SEAL_TEAM_6,	false	},
	{ "models/player/custom_player/legacy/ctm_gendarmerie_variantb.mdl","gendarmerie_male",		CS_CLASS_SEAL_TEAM_6,	false	},
	{ "models/player/custom_player/legacy/ctm_gendarmerie_variantc.mdl","gendarmerie_fem_epic",	CS_CLASS_SEAL_TEAM_6,	true	},
	{ "models/player/custom_player/legacy/ctm_gendarmerie_variantd.mdl","gendarmerie_male",		CS_CLASS_SEAL_TEAM_6,	false	},
	{ "models/player/custom_player/legacy/ctm_gendarmerie_variante.mdl","gendarmerie_male",		CS_CLASS_SEAL_TEAM_6,	false	},
	{ "models/player/custom_player/legacy/ctm_sas_variantg.mdl",		NULL,					CS_CLASS_SAS,			false	},
	{ "models/player/custom_player/legacy/ctm_st6_variantn.mdl",		NULL,					CS_CLASS_SEAL_TEAM_6,	false	},
	{ "models/player/custom_player/legacy/ctm_swat_variantk.mdl",		"swat_fem",				CS_CLASS_SWAT,			true	},
	// what?
	{ "models/player/custom_player/legacy/ctm_sas_old.mdl",				"ctm_sas",				CS_CLASS_SAS,			false	},
	{ "models/player/custom_player/legacy/ctm_fbi_old.mdl",				"ctm_fbi",				CS_CLASS_FBI,			false	},
	// kill me (let's pretend that they're all ST6 guys just because its the first class for CTs)
	{ "models/player/custom_player/legacy/ctm_jumpsuit_varianta.mdl",	"tm_leet",				CS_CLASS_SEAL_TEAM_6,	false	},
 	{ "models/player/custom_player/legacy/ctm_jumpsuit_variantb.mdl",	"tm_leet",				CS_CLASS_SEAL_TEAM_6,	false	},
 	{ "models/player/custom_player/legacy/ctm_jumpsuit_variantc.mdl",	"tm_leet",				CS_CLASS_SEAL_TEAM_6,	false	}
};

const CCSAgentInfo* GetCSAgentInfoCT( int i )
{
	Assert( i >= 0 && i < ARRAYSIZE( g_AgentInfosCT ) );
	if ( i < 0 || i >= ARRAYSIZE( g_AgentInfosCT ) )
		return &g_AgentInfosCT[0];
	return &g_AgentInfosCT[i];
}

CCSAgentInfo g_AgentInfosT[MAX_AGENTS_T + 1] =
{
	// default
	{ "models/player/custom_player/legacy/tm_phoenix.mdl",					NULL,				NULL,							NULL	},
	// Shattered Web
	{ "models/player/custom_player/legacy/tm_leet_variantg.mdl",			NULL,				CS_CLASS_L337_KREW,				false	},
	{ "models/player/custom_player/legacy/tm_leet_variantg_legacy.mdl",		NULL,				CS_CLASS_L337_KREW,				false	},
	{ "models/player/custom_player/legacy/tm_leet_varianth.mdl",			NULL,				CS_CLASS_L337_KREW,				false	},
	{ "models/player/custom_player/legacy/tm_leet_varianti.mdl",			NULL,				CS_CLASS_L337_KREW,				false	},
	{ "models/player/custom_player/legacy/tm_leet_varianti_legacy.mdl",		NULL,				CS_CLASS_L337_KREW,				false	},
	{ "models/player/custom_player/legacy/tm_leet_variantf.mdl",			"leet_epic",		CS_CLASS_L337_KREW,				false	},
	{ "models/player/custom_player/legacy/tm_phoenix_varianth.mdl",			NULL,				CS_CLASS_PHOENIX_CONNNECTION,	false	},
	{ "models/player/custom_player/legacy/tm_phoenix_variantf.mdl",			NULL,				CS_CLASS_PHOENIX_CONNNECTION,	false	},
	{ "models/player/custom_player/legacy/tm_phoenix_variantf_legacy.mdl",	NULL,				CS_CLASS_PHOENIX_CONNNECTION,	false	},
	{ "models/player/custom_player/legacy/tm_phoenix_variantg.mdl",			NULL,				CS_CLASS_PHOENIX_CONNNECTION,	false	},
	{ "models/player/custom_player/legacy/tm_balkan_variantf.mdl",			NULL,				CS_CLASS_BALKAN,				false	},
	{ "models/player/custom_player/legacy/tm_balkan_varianti.mdl",			NULL,				CS_CLASS_BALKAN,				false	},
	{ "models/player/custom_player/legacy/tm_balkan_variantg.mdl",			NULL,				CS_CLASS_BALKAN,				false	},
	{ "models/player/custom_player/legacy/tm_balkan_variantj.mdl",			NULL,				CS_CLASS_BALKAN,				false	},
	{ "models/player/custom_player/legacy/tm_balkan_varianth.mdl",			"balkan_epic",		CS_CLASS_BALKAN,				false	},
	// Broken Fang
	{ "models/player/custom_player/legacy/tm_balkan_variantl.mdl",			NULL,				CS_CLASS_BALKAN,				false	},
	{ "models/player/custom_player/legacy/tm_phoenix_varianti.mdl",			NULL,				CS_CLASS_PHOENIX_CONNNECTION,	false	},
	{ "models/player/custom_player/legacy/tm_professional_varj.mdl",		"professional_fem",	CS_CLASS_PROFESSIONAL,			true	},
	{ "models/player/custom_player/legacy/tm_professional_varh.mdl",		NULL,				CS_CLASS_PROFESSIONAL,			false	},
	{ "models/player/custom_player/legacy/tm_balkan_variantk.mdl",			NULL,				CS_CLASS_BALKAN,				false	},
	{ "models/player/custom_player/legacy/tm_professional_varg.mdl",		"professional_fem",	CS_CLASS_PROFESSIONAL,			true	},
	{ "models/player/custom_player/legacy/tm_professional_vari.mdl",		NULL,				CS_CLASS_PROFESSIONAL,			false	},
	{ "models/player/custom_player/legacy/tm_professional_varf.mdl",		"professional_epic",CS_CLASS_PROFESSIONAL,			false	},
	{ "models/player/custom_player/legacy/tm_professional_varf1.mdl",		"professional_epic",CS_CLASS_PROFESSIONAL,			false	},
	{ "models/player/custom_player/legacy/tm_professional_varf2.mdl",		"professional_epic",CS_CLASS_PROFESSIONAL,			false	},
	{ "models/player/custom_player/legacy/tm_professional_varf3.mdl",		"professional_epic",CS_CLASS_PROFESSIONAL,			false	},
	{ "models/player/custom_player/legacy/tm_professional_varf4.mdl",		"professional_epic",CS_CLASS_PROFESSIONAL,			false	},
	// Riptide
	{ "models/player/custom_player/legacy/tm_jungle_raider_varianta.mdl",	"jungle_male",		CS_CLASS_PHOENIX_CONNNECTION,	false	},
	{ "models/player/custom_player/legacy/tm_jungle_raider_variantb.mdl",	"jungle_male_epic",	CS_CLASS_PHOENIX_CONNNECTION,	false	},
	{ "models/player/custom_player/legacy/tm_jungle_raider_variantb2.mdl",	"jungle_male_epic",	CS_CLASS_PHOENIX_CONNNECTION,	false	},
	{ "models/player/custom_player/legacy/tm_jungle_raider_variantc.mdl",	"jungle_male",		CS_CLASS_PHOENIX_CONNNECTION,	false	},
	{ "models/player/custom_player/legacy/tm_jungle_raider_variantd.mdl",	"jungle_male",		CS_CLASS_PHOENIX_CONNNECTION,	false	},
	{ "models/player/custom_player/legacy/tm_jungle_raider_variante.mdl",	"jungle_fem_epic",	CS_CLASS_PHOENIX_CONNNECTION,	true	},
	{ "models/player/custom_player/legacy/tm_jungle_raider_variantf.mdl",	"jungle_fem",		CS_CLASS_PHOENIX_CONNNECTION,	true	},
	{ "models/player/custom_player/legacy/tm_jungle_raider_variantf2.mdl",	"jungle_fem",		CS_CLASS_PHOENIX_CONNNECTION,	true	},
	{ "models/player/custom_player/legacy/tm_leet_variantj.mdl",			NULL,				CS_CLASS_L337_KREW,				false	},
	{ "models/player/custom_player/legacy/tm_professional_varf5.mdl",		"professional_epic",CS_CLASS_PROFESSIONAL,			false	},
	// what?
	{ "models/player/custom_player/legacy/tm_phoenix_old.mdl",				"tm_phoenix",		CS_CLASS_PHOENIX_CONNNECTION,	false	},
	{ "models/player/custom_player/legacy/tm_leet_old.mdl",					"tm_leet",			CS_CLASS_L337_KREW,				false	},
	// kill me
	{ "models/player/custom_player/legacy/tm_jumpsuit_varianta.mdl",		"tm_leet",			CS_CLASS_L337_KREW,				false	},
 	{ "models/player/custom_player/legacy/tm_jumpsuit_variantb.mdl",		"tm_leet",			CS_CLASS_L337_KREW,				false	},
 	{ "models/player/custom_player/legacy/tm_jumpsuit_variantc.mdl",		"tm_leet",			CS_CLASS_L337_KREW,				false	}
};

const CCSAgentInfo* GetCSAgentInfoT( int i )
{
	Assert( i >= 0 && i < ARRAYSIZE( g_AgentInfosT ) );
	if ( i < 0 || i >= ARRAYSIZE( g_AgentInfosT ) )
		return &g_AgentInfosT[0];
	return &g_AgentInfosT[i];
}

CSMainMenuWeapons g_MainMenuWeaponsT[MAX_MAINMENU_WEAPONS_T] =
{
	{	"models/weapons/w_knife_default_t.mdl",			"t_loadout_knife_idle1",				"#Cstrike_WPNHUD_Knife"						},
	{	"models/weapons/w_knife_css.mdl",				"t_loadout_knife_idle1",				"#Cstrike_WPNHUD_Knife_CSS"					},
	{	"models/weapons/w_knife_karam.mdl",				"t_loadout_knife_idle1",				"#Cstrike_WPNHUD_Knife_Karam"				},
	{	"models/weapons/w_knife_flip.mdl",				"t_loadout_knife_idle1",				"#Cstrike_WPNHUD_Knife_Flip"				},
	{	"models/weapons/w_knife_bayonet.mdl",			"t_loadout_knife_idle1",				"#Cstrike_WPNHUD_Knife_Bayonet"				},
	{	"models/weapons/w_knife_m9_bay.mdl",			"t_loadout_knife_idle1",				"#Cstrike_WPNHUD_Knife_M9"					},
	{	"models/weapons/w_knife_butterfly.mdl",			"t_loadout_knife_idle1",				"#Cstrike_WPNHUD_Knife_Butterfly"			},
	{	"models/weapons/w_knife_gut.mdl",				"t_loadout_knife_idle1",				"#Cstrike_WPNHUD_Knife_Gut"					},
	{	"models/weapons/w_knife_tactical.mdl",			"t_loadout_knife_idle1",				"#Cstrike_WPNHUD_Knife_Tactical"			},
	{	"models/weapons/w_knife_falchion_advanced.mdl",	"t_loadout_knife_idle1",				"#Cstrike_WPNHUD_Knife_Falchion_Advanced"	},
	{	"models/weapons/w_knife_survival_bowie.mdl",	"t_loadout_knife_idle1",				"#Cstrike_WPNHUD_Knife_Survival_Bowie"		},
	{	"models/weapons/w_knife_canis.mdl",				"t_loadout_knife_idle1",				"#Cstrike_WPNHUD_Knife_Canis"				},
	{	"models/weapons/w_knife_cord.mdl",				"t_loadout_knife_idle1",				"#Cstrike_WPNHUD_Knife_Cord"				},
	{	"models/weapons/w_knife_gypsy_jackknife.mdl",	"t_loadout_knife_idle1",				"#Cstrike_WPNHUD_Knife_Gypsy_Jackknife"		},
	{	"models/weapons/w_knife_outdoor.mdl",			"t_loadout_knife_idle1",				"#Cstrike_WPNHUD_Knife_Outdoor"				},
	{	"models/weapons/w_knife_skeleton.mdl",			"t_loadout_knife_idle1",				"#Cstrike_WPNHUD_Knife_Skeleton"			},
	{	"models/weapons/w_knife_stiletto.mdl",			"t_loadout_knife_idle1",				"#Cstrike_WPNHUD_Knife_Stiletto"			},
	{	"models/weapons/w_knife_ursus.mdl",				"t_loadout_knife_idle1",				"#Cstrike_WPNHUD_Knife_Ursus"				},
	{	"models/weapons/w_knife_widowmaker.mdl",		"t_loadout_knife_idle1",				"#Cstrike_WPNHUD_Knife_Widowmaker"			},
	{	"models/weapons/w_knife_push.mdl",				"t_loadout_push_idle",					"#Cstrike_WPNHUD_Knife_Push"				},
	{	"models/weapons/w_c4.mdl",						"t_loadout_c4_idle",					"#Cstrike_WPNHUD_C4"						},
	{	"models/weapons/w_pist_glock18.mdl",			"t_loadout_pistol_idle",				"#Cstrike_WPNHUD_Glock18"					},
	{	"models/weapons/w_pist_elite.mdl",				"t_loadout_dual_idle",					"#Cstrike_WPNHUD_Elites"					},
	{	"models/weapons/w_pist_p250.mdl",				"t_loadout_pistol_idle",				"#Cstrike_WPNHUD_P250"						},
	{	"models/weapons/w_pist_tec9.mdl",				"t_loadout_pistol_idle",				"#Cstrike_WPNHUD_Tec9"						},
	{	"models/weapons/w_pist_cz_75.mdl",				"t_loadout_pistol_idle",				"#Cstrike_WPNHUD_CZ75"						},
	{	"models/weapons/w_pist_deagle.mdl",				"t_loadout_pistol_idle",				"#Cstrike_WPNHUD_DesertEagle"				},
	{	"models/weapons/w_pist_revolver.mdl",			"t_loadout_pistol_idle",				"#Cstrike_WPNHUD_Revolver"					},
	{	"models/weapons/w_smg_mac10.mdl",				"t_loadout_shotgun_xm_idle",			"#Cstrike_WPNHUD_MAC10"						},
	{	"models/weapons/w_smg_mp7.mdl",					"t_loadout_mp7_idle",					"#Cstrike_WPNHUD_MP7"						},
	{	"models/weapons/w_smg_mp5sd.mdl",				"t_loadout_shotgun_xm_idle",			"#Cstrike_WPNHUD_MP5SD"						},
	{	"models/weapons/w_smg_ump45.mdl",				"t_loadout_ump45_idle",					"#Cstrike_WPNHUD_UMP45"						},
	{	"models/weapons/w_smg_p90.mdl",					"t_loadout_p90_idle",					"#Cstrike_WPNHUD_P90"						},
	{	"models/weapons/w_smg_bizon.mdl",				"t_loadout_bizon_idle",					"#Cstrike_WPNHUD_Bizon"						},
	{	"models/weapons/w_rif_galilar.mdl",				"t_loadout_rifle02_idle_galil",			"#Cstrike_WPNHUD_GalilAR"					},
	{	"models/weapons/w_rif_ak47.mdl",				"t_loadout_rifle02_idle",				"#Cstrike_WPNHUD_AK47"						},
	{	"models/weapons/w_snip_ssg08.mdl",				"t_loadout_rifle02_idle_sniper",		"#Cstrike_WPNHUD_SSG08"						},
	{	"models/weapons/w_rif_sg556.mdl",				"t_loadout_rifle02_idle",				"#Cstrike_WPNHUD_SG556"						},
	{	"models/weapons/w_snip_awp.mdl",				"t_loadout_rifle02_idle_awp",			"#Cstrike_WPNHUD_AWP"						},
	{	"models/weapons/w_snip_g3sg1.mdl",				"t_loadout_rifle02_idle_g3sg",			"#Cstrike_WPNHUD_G3SG1"						},
	{	"models/weapons/w_shot_nova.mdl",				"t_loadout_shotgun_idle",				"#Cstrike_WPNHUD_Nova"						},
	{	"models/weapons/w_shot_xm1014.mdl",				"t_loadout_shotgun_xm_idle",			"#Cstrike_WPNHUD_xm1014"					},
	{	"models/weapons/w_shot_sawedoff.mdl",			"t_loadout_shotgun_idle",				"#Cstrike_WPNHUD_SawedOff"					},
	{	"models/weapons/w_mach_m249.mdl",				"t_loadout_heavy_m249_idle",			"#Cstrike_WPNHUD_M249"						},
	{	"models/weapons/w_mach_negev.mdl",				"t_loadout_heavy_idle",					"#Cstrike_WPNHUD_Negev"						},
};

const CSMainMenuWeapons* GetCSMainMenuWeaponT( int i )
{
	Assert( i >= 0 && i < ARRAYSIZE( g_MainMenuWeaponsT ) );
	return &g_MainMenuWeaponsT[i];
}

CSMainMenuWeapons g_MainMenuWeaponsCT[MAX_MAINMENU_WEAPONS_CT] =
{
	{	"models/weapons/w_knife_default_ct.mdl",		"ct_loadout_knife_idle1",				"#Cstrike_WPNHUD_Knife"						},
	{	"models/weapons/w_knife_css.mdl",				"ct_loadout_knife_idle1",				"#Cstrike_WPNHUD_Knife_CSS"					},
	{	"models/weapons/w_knife_karam.mdl",				"ct_loadout_knife_idle1",				"#Cstrike_WPNHUD_Knife_Karam"				},
	{	"models/weapons/w_knife_flip.mdl",				"ct_loadout_knife_idle1",				"#Cstrike_WPNHUD_Knife_Flip"				},
	{	"models/weapons/w_knife_bayonet.mdl",			"ct_loadout_knife_idle1",				"#Cstrike_WPNHUD_Knife_Bayonet"				},
	{	"models/weapons/w_knife_m9_bay.mdl",			"ct_loadout_knife_idle1",				"#Cstrike_WPNHUD_Knife_M9"					},
	{	"models/weapons/w_knife_butterfly.mdl",			"ct_loadout_knife_idle1",				"#Cstrike_WPNHUD_Knife_Butterfly"			},
	{	"models/weapons/w_knife_gut.mdl",				"ct_loadout_knife_idle1",				"#Cstrike_WPNHUD_Knife_Gut"					},
	{	"models/weapons/w_knife_tactical.mdl",			"ct_loadout_knife_idle1",				"#Cstrike_WPNHUD_Knife_Tactical"			},
	{	"models/weapons/w_knife_falchion_advanced.mdl",	"ct_loadout_knife_idle1",				"#Cstrike_WPNHUD_Knife_Falchion_Advanced"	},
	{	"models/weapons/w_knife_survival_bowie.mdl",	"ct_loadout_knife_idle1",				"#Cstrike_WPNHUD_Knife_Survival_Bowie"		},
	{	"models/weapons/w_knife_canis.mdl",				"ct_loadout_knife_idle1",				"#Cstrike_WPNHUD_Knife_Canis"				},
	{	"models/weapons/w_knife_cord.mdl",				"ct_loadout_knife_idle1",				"#Cstrike_WPNHUD_Knife_Cord"				},
	{	"models/weapons/w_knife_gypsy_jackknife.mdl",	"ct_loadout_knife_idle1",				"#Cstrike_WPNHUD_Knife_Gypsy_Jackknife"		},
	{	"models/weapons/w_knife_outdoor.mdl",			"ct_loadout_knife_idle1",				"#Cstrike_WPNHUD_Knife_Outdoor"				},
	{	"models/weapons/w_knife_skeleton.mdl",			"ct_loadout_knife_idle1",				"#Cstrike_WPNHUD_Knife_Skeleton"			},
	{	"models/weapons/w_knife_stiletto.mdl",			"ct_loadout_knife_idle1",				"#Cstrike_WPNHUD_Knife_Stiletto"			},
	{	"models/weapons/w_knife_ursus.mdl",				"ct_loadout_knife_idle1",				"#Cstrike_WPNHUD_Knife_Ursus"				},
	{	"models/weapons/w_knife_widowmaker.mdl",		"ct_loadout_knife_idle1",				"#Cstrike_WPNHUD_Knife_Widowmaker"			},
	{	"models/weapons/w_knife_push.mdl",				"ct_loadout_push_idle",					"#Cstrike_WPNHUD_Knife_Push"				},
	{	"models/weapons/w_pist_hkp2000.mdl",			"ct_loadout_pistol01_idle",				"#Cstrike_WPNHUD_HKP2000"					},
	{	"models/weapons/w_pist_usp.mdl",				"ct_loadout_pistol01_idle",				"#Cstrike_WPNHUD_USP45"						},
	{	"models/weapons/w_pist_elite.mdl",				"ct_loadout_dual_idle",					"#Cstrike_WPNHUD_Elites"					},
	{	"models/weapons/w_pist_p250.mdl",				"ct_loadout_pistol01_idle",				"#Cstrike_WPNHUD_P250"						},
	{	"models/weapons/w_pist_fiveseven.mdl",			"ct_loadout_pistol01_idle",				"#Cstrike_WPNHUD_FiveSeven"					},
	{	"models/weapons/w_pist_cz_75.mdl",				"ct_loadout_pistol01_idle",				"#Cstrike_WPNHUD_CZ75"						},
	{	"models/weapons/w_pist_deagle.mdl",				"ct_loadout_pistol01_idle",				"#Cstrike_WPNHUD_DesertEagle"				},
	{	"models/weapons/w_pist_revolver.mdl",			"ct_loadout_pistol01_idle",				"#Cstrike_WPNHUD_Revolver"					},
	{	"models/weapons/w_smg_mp9.mdl",					"ct_loadout_p90_idle",					"#Cstrike_WPNHUD_MP9"						},
	{	"models/weapons/w_smg_mp7.mdl",					"ct_loadout_mp7_idle",					"#Cstrike_WPNHUD_MP7"						},
	{	"models/weapons/w_smg_mp5sd.mdl",				"ct_loadout_p90_idle",					"#Cstrike_WPNHUD_MP5SD"						},
	{	"models/weapons/w_smg_ump45.mdl",				"ct_loadout_p90_idle",					"#Cstrike_WPNHUD_UMP45"						},
	{	"models/weapons/w_smg_p90.mdl",					"ct_loadout_p90_idle",					"#Cstrike_WPNHUD_P90"						},
	{	"models/weapons/w_smg_bizon.mdl",				"ct_loadout_p90_idle",					"#Cstrike_WPNHUD_Bizon"						},
	{	"models/weapons/w_rif_famas.mdl",				"ct_loadout_famas_idle",				"#Cstrike_WPNHUD_Famas"						},
	{	"models/weapons/w_rif_m4a4.mdl",				"ct_loadout_rifle_idle_handrepo_m4",	"#Cstrike_WPNHUD_M4A4"						},
	{	"models/weapons/w_rif_m4a1_silencer.mdl",		"ct_loadout_rifle_idle_handrepo_m4",	"#Cstrike_WPNHUD_M4A1"						},
	{	"models/weapons/w_snip_ssg08.mdl",				"ct_loadout_rifle_ssg08_idle",			"#Cstrike_WPNHUD_SSG08"						},
	{	"models/weapons/w_rif_aug.mdl",					"ct_loadout_rifle_idle_handrepo_aug",	"#Cstrike_WPNHUD_Aug"						},
	{	"models/weapons/w_snip_awp.mdl",				"ct_loadout_rifle_awp_idle",			"#Cstrike_WPNHUD_AWP"						},
	{	"models/weapons/w_snip_scar20.mdl",				"ct_loadout_rifle_scar_idle",			"#Cstrike_WPNHUD_SCAR20"					},
	{	"models/weapons/w_shot_nova.mdl",				"ct_loadout_nova_idle",					"#Cstrike_WPNHUD_Nova"						},
	{	"models/weapons/w_shot_xm1014.mdl",				"ct_loadout_xm1014_idle",				"#Cstrike_WPNHUD_xm1014"					},
	{	"models/weapons/w_shot_mag7.mdl",				"ct_loadout_mag7_idle",					"#Cstrike_WPNHUD_MAG7"						},
	{	"models/weapons/w_mach_m249.mdl",				"ct_loadout_heavystanding_idle",		"#Cstrike_WPNHUD_M249"						},
	{	"models/weapons/w_mach_negev.mdl",				"ct_loadout_heavystanding_idle",		"#Cstrike_WPNHUD_Negev"						},
};

const CSMainMenuWeapons* GetCSMainMenuWeaponCT( int i )
{
	Assert( i >= 0 && i < ARRAYSIZE( g_MainMenuWeaponsCT ) );
	return &g_MainMenuWeaponsCT[i];
}

const char *pszWinPanelCategoryHeaders[] =
{
	"",
	"#winpanel_topdamage",
	"#winpanel_topheadshots",
	"#winpanel_kills"
};

// todo: rewrite this because it's TOO MASSIVE!
const char* TPhoenixPlayerModelStrings[MAX_FACTION_SKINS] =
{
	"models/player/custom_player/legacy/tm_phoenix.mdl",
	"models/player/custom_player/legacy/tm_phoenix_varianta.mdl",
	"models/player/custom_player/legacy/tm_phoenix_variantb.mdl",
	"models/player/custom_player/legacy/tm_phoenix_variantc.mdl",
	"models/player/custom_player/legacy/tm_phoenix_variantd.mdl",
};
const char* TLeetPlayerModelStrings[MAX_FACTION_SKINS] =
{
	"models/player/custom_player/legacy/tm_leet_variantA.mdl",
	"models/player/custom_player/legacy/tm_leet_variantB.mdl",
	"models/player/custom_player/legacy/tm_leet_variantC.mdl",
	"models/player/custom_player/legacy/tm_leet_variantD.mdl",
	"models/player/custom_player/legacy/tm_leet_variantE.mdl",

};
const char* TSeparatistPlayerModelStrings[MAX_FACTION_SKINS] =
{
	"models/player/custom_player/legacy/tm_separatist.mdl",
	"models/player/custom_player/legacy/tm_separatist_varianta.mdl",
	"models/player/custom_player/legacy/tm_separatist_variantb.mdl",
	"models/player/custom_player/legacy/tm_separatist_variantc.mdl",
	"models/player/custom_player/legacy/tm_separatist_variantd.mdl",
};
const char* TBalkanPlayerModelStrings[MAX_FACTION_SKINS] =
{
	"models/player/custom_player/legacy/tm_balkan_varianta.mdl",
	"models/player/custom_player/legacy/tm_balkan_variantb.mdl",
	"models/player/custom_player/legacy/tm_balkan_variantc.mdl",
	"models/player/custom_player/legacy/tm_balkan_variantd.mdl",
	"models/player/custom_player/legacy/tm_balkan_variante.mdl",
};
const char* TProfessionalPlayerModelStrings[MAX_FACTION_SKINS] =
{
	"models/player/custom_player/legacy/tm_professional.mdl",
	"models/player/custom_player/legacy/tm_professional_var1.mdl",
	"models/player/custom_player/legacy/tm_professional_var2.mdl",
	"models/player/custom_player/legacy/tm_professional_var3.mdl",
	"models/player/custom_player/legacy/tm_professional_var4.mdl",
};
const char* TAnarchistPlayerModelStrings[MAX_FACTION_SKINS] =
{
	"models/player/custom_player/legacy/tm_anarchist.mdl",
	"models/player/custom_player/legacy/tm_anarchist_varianta.mdl",
	"models/player/custom_player/legacy/tm_anarchist_variantb.mdl",
	"models/player/custom_player/legacy/tm_anarchist_variantc.mdl",
	"models/player/custom_player/legacy/tm_anarchist_variantd.mdl",
};
const char* TPiratePlayerModelStrings[MAX_FACTION_SKINS] =
{
	"models/player/custom_player/legacy/tm_pirate.mdl",
	"models/player/custom_player/legacy/tm_pirate_varianta.mdl",
	"models/player/custom_player/legacy/tm_pirate_variantb.mdl",
	"models/player/custom_player/legacy/tm_pirate_variantc.mdl",
	"models/player/custom_player/legacy/tm_pirate_variantd.mdl",
};

const char* CTST6PlayerModelStrings[MAX_FACTION_SKINS] =
{
	"models/player/custom_player/legacy/ctm_st6.mdl",
	"models/player/custom_player/legacy/ctm_st6_varianta.mdl",
	"models/player/custom_player/legacy/ctm_st6_variantb.mdl",
	"models/player/custom_player/legacy/ctm_st6_variantc.mdl",
	"models/player/custom_player/legacy/ctm_st6_variantd.mdl",
};
const char* CTGSG9PlayerModelStrings[MAX_FACTION_SKINS] =
{
	"models/player/custom_player/legacy/ctm_gsg9.mdl",
	"models/player/custom_player/legacy/ctm_gsg9_varianta.mdl",
	"models/player/custom_player/legacy/ctm_gsg9_variantb.mdl",
	"models/player/custom_player/legacy/ctm_gsg9_variantc.mdl",
	"models/player/custom_player/legacy/ctm_gsg9_variantd.mdl",
};
const char* CTSASPlayerModelStrings[MAX_FACTION_SKINS] =
{
	"models/player/custom_player/legacy/ctm_sas.mdl",
	"models/player/custom_player/legacy/ctm_sas_varianta.mdl",
	"models/player/custom_player/legacy/ctm_sas_variantb.mdl",
	"models/player/custom_player/legacy/ctm_sas_variantc.mdl",
	"models/player/custom_player/legacy/ctm_sas_variantd.mdl",
};
const char* CTGIGNPlayerModelStrings[MAX_FACTION_SKINS] =
{
	"models/player/custom_player/legacy/ctm_gign.mdl",
	"models/player/custom_player/legacy/ctm_gign_varianta.mdl",
	"models/player/custom_player/legacy/ctm_gign_variantb.mdl",
	"models/player/custom_player/legacy/ctm_gign_variantc.mdl",
	"models/player/custom_player/legacy/ctm_gign_variantd.mdl",
};
const char* CTFBIPlayerModelStrings[MAX_FACTION_SKINS] =
{
	"models/player/custom_player/legacy/ctm_fbi.mdl",
	"models/player/custom_player/legacy/ctm_fbi_varianta.mdl",
	"models/player/custom_player/legacy/ctm_fbi_variantc.mdl",
	"models/player/custom_player/legacy/ctm_fbi_variantd.mdl",
	"models/player/custom_player/legacy/ctm_fbi_variante.mdl",
};
const char* CTIDFPlayerModelStrings[MAX_IDF_SKINS] =
{
	"models/player/custom_player/legacy/ctm_idf.mdl",
	"models/player/custom_player/legacy/ctm_idf_variantb.mdl",
	"models/player/custom_player/legacy/ctm_idf_variantc.mdl",
	"models/player/custom_player/legacy/ctm_idf_variantd.mdl",
	"models/player/custom_player/legacy/ctm_idf_variante.mdl",
	"models/player/custom_player/legacy/ctm_idf_variantf.mdl",
};
const char* CTSWATPlayerModelStrings[MAX_FACTION_SKINS] =
{
	"models/player/custom_player/legacy/ctm_swat.mdl",
	"models/player/custom_player/legacy/ctm_swat_varianta.mdl",
	"models/player/custom_player/legacy/ctm_swat_variantb.mdl",
	"models/player/custom_player/legacy/ctm_swat_variantc.mdl",
	"models/player/custom_player/legacy/ctm_swat_variantd.mdl",
};

// any new knives? add them here
const char *KnivesEntitiesStrings[MAX_KNIVES] =
{
	"weapon_knife_css",
	"weapon_knife_karambit",
	"weapon_knife_flip",
	"weapon_knife_bayonet",
	"weapon_knife_m9_bayonet",
	"weapon_knife_butterfly",
	"weapon_knife_gut",
	"weapon_knife_tactical",
	"weapon_knife_falchion",
	"weapon_knife_survival_bowie",
	"weapon_knife_canis",
	"weapon_knife_cord",
	"weapon_knife_gypsy_jackknife",
	"weapon_knife_outdoor",
	"weapon_knife_skeleton",
	"weapon_knife_stiletto",
	"weapon_knife_ursus",
	"weapon_knife_widowmaker",
	"weapon_knife_push",
};

const char* g_szMusicKits[MAX_MUSIC] =
{
	"valve_csgo_01", // the default one should be on top
	"valve_csgo_02", // the default one should be on top
	"amontobin_01",
	"austinwintory_01",
	"austinwintory_02",
	"austinwintory_03",
	"awolnation_01",
	"bbnos_01",
	"beartooth_01",
	"beartooth_02",
	"blitzkids_01",
	"chipzel_01",
	"damjanmravunac_01",
	"danielsadowski_01",
	"danielsadowski_02",
	"danielsadowski_03",
	"danielsadowski_04",
	"darude_01",
	"dren_01",
	"dren_02",
	"feedme_01",
	"freakydna_01",
	"hades_01",
	"halflife_alyx_01",
	"halo_01",
	"hotlinemiami_01",
	"hundredth_01",
	"ianhultquist_01",
	"jesseharlin_01",
	"kellybailey_01",
	"kitheory_01",
	"laurashigihara_01",
	"lenniemoore_01",
	"mateomessina_01",
	"mattlange_01",
	"mattlevine_01",
	"michaelbross_01",
	"midnightriders_01",
	"mordfustang_01",
	"neckdeep_01",
	"neckdeep_02",
	"newbeatfund_01",
	"noisia_01",
	"proxy_01",
	"roam_01",
	"robertallaire_01",
	"sammarshall_01",
	"sarahschachner_01",
	"sasha_01",
	"scarlxrd_01",
	"scarlxrd_02",
	"seanmurray_01",
	"skog_01",
	"skog_02",
	"skog_03",
	"theverkkars_01",
	"theverkkars_02",
	"timhuling_01",
	"treeadams_benbromfield_01",
	"troelsfolmann_01",
	"twinatlantic_01"
};