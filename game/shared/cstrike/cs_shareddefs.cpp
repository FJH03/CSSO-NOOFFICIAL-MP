//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "cs_shareddefs.h"

const float CS_PLAYER_SPEED_RUN		 = 260.0f;
const float CS_PLAYER_SPEED_VIP		 = 227.0f;
const float CS_PLAYER_SPEED_WALK	 = 100.0f;
const float CS_PLAYER_SPEED_SHIELD	 = 160.0f;
const float CS_PLAYER_SPEED_STOPPED	 =   1.0f;
const float CS_PLAYER_SPEED_OBSERVER = 900.0f;

const float CS_PLAYER_SPEED_DUCK_MODIFIER	= 0.34f;
const float CS_PLAYER_SPEED_WALK_MODIFIER	= 0.52f;
const float CS_PLAYER_SPEED_CLIMB_MODIFIER	= 0.34f;

const float CS_PLAYER_DUCK_SPEED_IDEAL = 8.0f;


CCSClassInfo g_ClassInfos[] =
{
	{ "None" },

	{ "Phoenix Connection" },
	{ "L337 KREW" },
	{ "Arctic Avengers" },
	{ "Guerilla Warfare" },

	{ "Seal Team 6" },
	{ "GSG-9" },
	{ "SAS" },
	{ "GIGN" }
};

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

const CCSClassInfo* GetCSClassInfo( int i )
{
	Assert( i >= 0 && i < ARRAYSIZE( g_ClassInfos ) );
	return &g_ClassInfos[i];
}

const char *pszWinPanelCategoryHeaders[] =
{
	"",
	"#winpanel_topdamage",
	"#winpanel_topheadshots",
	"#winpanel_kills"
};

// Construct some arrays of player model strings, so we can statically initialize CUtlVectors for general usage
const char *CTPlayerModelStrings[] =
{
	"models/player/ct_urban.mdl",
	"models/player/ct_gsg9.mdl",
	"models/player/ct_sas.mdl",
	"models/player/ct_gign.mdl",
};
const char *TerroristPlayerModelStrings[] =
{
	"models/player/t_phoenix.mdl",
	"models/player/t_leet.mdl",
	"models/player/t_arctic.mdl",
	"models/player/t_guerilla.mdl",
};
const char *KnivesEntitiesStrings[] =
{
	"weapon_knife",
	"weapon_knife_t",
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
};
CUtlVectorInitialized< const char * > CTPlayerModels( CTPlayerModelStrings, ARRAYSIZE( CTPlayerModelStrings ) );
CUtlVectorInitialized< const char * > TerroristPlayerModels( TerroristPlayerModelStrings, ARRAYSIZE( TerroristPlayerModelStrings ) );
CUtlVectorInitialized< const char * > KnivesEntities( KnivesEntitiesStrings, ARRAYSIZE( KnivesEntitiesStrings ) );
