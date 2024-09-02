//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Laser Rifle & Shield combo
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "in_buttons.h"
#include "takedamageinfo.h"
#include "weapon_csbase.h"
#include "ammodef.h"
#include "cs_gamerules.h"
#include "basegrenade_shared.h"
#include "npcevent.h"
#include "eventlist.h"

#define ALLOW_WEAPON_SPREAD_DISPLAY	0

#if defined( CLIENT_DLL )

	#include "vgui/ISurface.h"
	#include "vgui_controls/Controls.h"
	#include "c_cs_player.h"
	#include "hud_crosshair.h"
	#include "c_te_effect_dispatch.h"
	#include "c_te_legacytempents.h"
	#include "predicted_viewmodel.h"

	extern IVModelInfoClient* modelinfo;

#else

	#include "cs_player.h"
	#include "te_effect_dispatch.h"
	#include "KeyValues.h"
	#include "cs_ammodef.h"

	extern IVModelInfo* modelinfo;

#endif

// CS-PRO TEST CHANGE: instant movement inaccuracy, curve exponent x^0.25
#define MOVEMENT_ACCURACY_DECAYED	0
#define MOVEMENT_CURVE01_EXPONENT   0.25

extern WeaponRecoilData g_WeaponRecoilData;
extern ConVar sv_jump_impulse;

ConVar weapon_accuracy_model( "weapon_accuracy_model", "2", FCVAR_REPLICATED | FCVAR_DEVELOPMENTONLY | FCVAR_ARCHIVE );

ConVar weapon_recoil_decay2_exp( "weapon_recoil_decay2_exp", "8", FCVAR_CHEAT | FCVAR_REPLICATED, "Decay factor exponent for weapon recoil" );
ConVar weapon_recoil_decay2_lin( "weapon_recoil_decay2_lin", "18", FCVAR_CHEAT | FCVAR_REPLICATED, "Decay factor (linear term) for weapon recoil" );
ConVar weapon_recoil_vel_decay( "weapon_recoil_vel_decay", "4.5", FCVAR_CHEAT | FCVAR_REPLICATED, "Decay factor for weapon recoil velocity" );

ConVar weapon_accuracy_nospread( "weapon_accuracy_nospread", "0", FCVAR_CHEAT | FCVAR_REPLICATED, "Disable weapon inaccuracy spread" );
ConVar weapon_recoil_scale( "weapon_recoil_scale", "2.0", FCVAR_CHEAT | FCVAR_REPLICATED, "Overall scale factor for recoil. Used to reduce recoil on specific platforms" );
ConVar weapon_air_spread_scale( "weapon_air_spread_scale", "1.0", FCVAR_CHEAT | FCVAR_REPLICATED, "Scale factor for jumping inaccuracy, set to 0 to make jumping accuracy equal to standing", true, 0.0f, false, 1.0f );

ConVar weapon_recoil_decay_coefficient( "weapon_recoil_decay_coefficient", "2.0", FCVAR_CHEAT | FCVAR_REPLICATED, "" );

// ----------------------------------------------------------------------------- //
// Global functions.
// ----------------------------------------------------------------------------- //



struct WeaponAliasTranslationInfoStruct
{
	const char* alias;
	const char* translatedAlias;
};

static const WeaponAliasTranslationInfoStruct s_WeaponAliasTranslationInfo[] =
{
	{ "cv47", "ak47" },
	{ "defender", "galil" },
	{ "krieg552", "sg552" },
	{ "magnum", "awp" },
	{ "d3au1", "g3sg1" },
	{ "clarion", "famas" },
	{ "bullpup", "aug" },
	{ "krieg550", "sg550" },
	{ "9x19mm", "glock" },
	{ "km45", "usp" },
	{ "228compact", "p228" },
	{ "nighthawk", "deagle" },
	{ "elites", "elite" },
	{ "fn57", "fiveseven" },
	{ "12gauge", "m3" },
	{ "autoshotgun", "xm1014" },
	{ "mp", "tmp" },
	{ "smg", "mp5navy" },
	{ "mp5", "mp5navy" },
	{ "c90", "p90" },
	{ "vest", "kevlar" },
	{ "vesthelm", "assaultsuit" },
	{ "smokegrenade", "sgren" },
	{ "smokegrenade", "sgren" },
	{ "nvgs", "nightvision" },

	{ "", "" } // this needs to be last
};


struct WeaponAliasInfo
{
	CSWeaponID id;
	const char* alias;
};

WeaponAliasInfo s_weaponAliasInfo[] =
{
	{ WEAPON_P250,				"p250" },
	{ WEAPON_GLOCK,				"glock" },
	{ WEAPON_SSG08,				"ssg08" },
	{ WEAPON_XM1014,			"xm1014" },
	{ WEAPON_MAC10,				"mac10" },
	{ WEAPON_AUG,				"aug" },
	{ WEAPON_ELITE,				"elite" },
	{ WEAPON_FIVESEVEN,			"fiveseven" },
	{ WEAPON_UMP45,				"ump45" },
	{ WEAPON_SCAR20,				"scar20" },
	{ WEAPON_GALILAR,			"galilar" },
	{ WEAPON_FAMAS,				"famas" },
	{ WEAPON_USP,				"usp_silencer" },
	{ WEAPON_AWP,				"awp" },
	{ WEAPON_MP5SD,				"mp5sd" },
	{ WEAPON_M249,				"m249" },
	{ WEAPON_NOVA,				"nova" },
	{ WEAPON_M4A1,				"m4a1_silencer" },
	{ WEAPON_MP9,				"mp9" },
	{ WEAPON_G3SG1,				"g3sg1" },
	{ WEAPON_DEAGLE,			"deagle" },
	{ WEAPON_SG556,				"sg556" },
	{ WEAPON_AK47,				"ak47" },
	{ WEAPON_P90,				"p90" },

	{ WEAPON_HKP2000,			"hkp2000" },
	{ WEAPON_TEC9,				"tec9" },
	{ WEAPON_M4A4,				"m4a4" },
	{ WEAPON_REVOLVER,			"revolver" },
	{ WEAPON_CZ75,				"cz75" },
	{ WEAPON_MAG7,				"mag7" },
	{ WEAPON_SAWEDOFF,			"sawedoff" },
	{ WEAPON_NEGEV,				"negev" },
	{ WEAPON_MP7,				"mp7" },
	{ WEAPON_BIZON,				"bizon" },
	{ WEAPON_TASER,				"taser" },

	//knife massive
	{ WEAPON_KNIFE,				"weapon_knife" },
	{ WEAPON_KNIFE_T,			"weapon_knife_t" },
	{ WEAPON_KNIFE_CSS,			"weapon_knife_css" },
	{ WEAPON_KNIFE_KARAMBIT,	"weapon_knife_karambit" },
	{ WEAPON_KNIFE_FLIP,		"weapon_knife_flip" },
	{ WEAPON_KNIFE_BAYONET,		"weapon_knife_bayonet" },
	{ WEAPON_KNIFE_M9_BAYONET,	"weapon_knife_m9_bayonet" },
	{ WEAPON_KNIFE_BUTTERFLY,	"weapon_knife_butterfly" },
	{ WEAPON_KNIFE_GUT,			"weapon_knife_gut" },
	{ WEAPON_KNIFE_TACTICAL,	"weapon_knife_tactical" },
	{ WEAPON_KNIFE_FALCHION,	"weapon_knife_falchion" },
	{ WEAPON_KNIFE_SURVIVAL_BOWIE,"weapon_knife_survival_bowie" },
	{ WEAPON_KNIFE_CANIS,		"weapon_knife_canis" },
	{ WEAPON_KNIFE_CORD,		"weapon_knife_cord" },
	{ WEAPON_KNIFE_GYPSY,		"weapon_knife_gypsy_jackknife" },
	{ WEAPON_KNIFE_OUTDOOR,		"weapon_knife_outdoor" },
	{ WEAPON_KNIFE_SKELETON,	"weapon_knife_skeleton" },
	{ WEAPON_KNIFE_STILETTO,	"weapon_knife_stiletto" },
	{ WEAPON_KNIFE_URSUS,		"weapon_knife_ursus" },
	{ WEAPON_KNIFE_WIDOWMAKER,	"weapon_knife_widowmaker" },
	
	{ WEAPON_C4,				"c4" },
	{ WEAPON_FLASHBANG,			"flashbang" },
	{ WEAPON_SMOKEGRENADE,		"smokegrenade" },
	{ WEAPON_SMOKEGRENADE,		"sgren" },
	{ WEAPON_HEGRENADE,			"hegrenade" },
	{ WEAPON_HEGRENADE,			"hegren" },
	{ WEAPON_MOLOTOV,			"molotov" },
	{ WEAPON_INCGRENADE,		"incgrenade" },
	{ WEAPON_DECOY,				"decoy" },

	// not sure any of these are needed
	{ WEAPON_SHIELDGUN,			"shield" },
	{ WEAPON_SHIELDGUN,			"shieldgun" },
	{ WEAPON_KEVLAR,			"kevlar" },
	{ WEAPON_ASSAULTSUIT,		"assaultsuit" },
	{ WEAPON_NVG,				"nightvision" },
	{ WEAPON_NVG,				"nvg" },

	{ WEAPON_NONE,				"none" },
};


bool IsAmmoType( int iAmmoType, const char *pAmmoName )
{
	return GetAmmoDef()->Index( pAmmoName ) == iAmmoType;
}

//--------------------------------------------------------------------------------------------------------
//
// Given an alias, return the translated alias.
//
const char * GetTranslatedWeaponAlias( const char *szAlias )
{
	for ( int i = 0; i < ARRAYSIZE(s_WeaponAliasTranslationInfo); ++i )
	{
		if ( Q_stricmp(s_WeaponAliasTranslationInfo[i].alias, szAlias) == 0 )
		{
			return s_WeaponAliasTranslationInfo[i].translatedAlias;
		}
	}

	return szAlias;
}

//--------------------------------------------------------------------------------------------------------
//
// Given a translated alias, return the alias.
//
const char * GetWeaponAliasFromTranslated(const char *translatedAlias)
{
	int i = 0;
	const WeaponAliasTranslationInfoStruct *info = &(s_WeaponAliasTranslationInfo[i]);

	while (info->alias[0] != 0)
	{
		if (Q_stricmp(translatedAlias, info->translatedAlias) == 0)
		{
			return info->alias;
		}
		info = &(s_WeaponAliasTranslationInfo[++i]);
	}

	return translatedAlias;
}

//--------------------------------------------------------------------------------------------------------
//
// Given an alias, return the associated weapon ID
//
CSWeaponID AliasToWeaponID( const char *szAlias )
{
	if ( szAlias )
	{
		for ( int i=0; i < ARRAYSIZE(s_weaponAliasInfo); ++i)
		{
			if ( Q_stricmp( s_weaponAliasInfo[i].alias, szAlias) == 0 )
				return s_weaponAliasInfo[i].id;
		}
	}

	return WEAPON_NONE;
}

//--------------------------------------------------------------------------------------------------------
//
// Given a weapon ID, return its alias
//
const char *WeaponIDToAlias( int id )
{
	for ( int i=0; i < ARRAYSIZE(s_weaponAliasInfo); ++i)
	{
		if ( s_weaponAliasInfo[i].id == id )
			return s_weaponAliasInfo[i].alias;
	}

	return NULL;
}

//--------------------------------------------------------------------------------------------------------
//
// Return true if given weapon ID is a primary weapon
//
bool IsPrimaryWeapon( CSWeaponID id )
{
	const CCSWeaponInfo* pWeaponInfo = GetWeaponInfo( id );
	if ( pWeaponInfo )
	{
		return pWeaponInfo->iSlot == WEAPON_SLOT_RIFLE;
	}

	return false;
}

//--------------------------------------------------------------------------------------------------------
//
// Return true if given weapon ID is a secondary weapon
//
bool IsSecondaryWeapon( CSWeaponID id )
{
	const CCSWeaponInfo* pWeaponInfo = GetWeaponInfo( id );
	if ( pWeaponInfo )
		return pWeaponInfo->iSlot == WEAPON_SLOT_PISTOL;

	return false;
}

#ifdef CLIENT_DLL
int GetShellForAmmoType( const char *ammoname )
{
	if ( !Q_strcmp( BULLET_PLAYER_762MM, ammoname ) )
		return CS_SHELL_762NATO;

	if ( !Q_strcmp( BULLET_PLAYER_556MM, ammoname ) )
		return CS_SHELL_556;

	if ( !Q_strcmp( BULLET_PLAYER_338MAG, ammoname ) )
		return CS_SHELL_338MAG;

	if ( !Q_strcmp( BULLET_PLAYER_BUCKSHOT, ammoname ) )
		return CS_SHELL_12GAUGE;

	if ( !Q_strcmp( BULLET_PLAYER_57MM, ammoname ) )
		return CS_SHELL_57;

	// default 9 mm
	return CS_SHELL_9MM;
}
#endif


// ----------------------------------------------------------------------------- //
// CWeaponCSBase tables.
// ----------------------------------------------------------------------------- //

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponCSBase, DT_WeaponCSBase )

BEGIN_NETWORK_TABLE( CWeaponCSBase, DT_WeaponCSBase )
#if !defined( CLIENT_DLL )
SendPropInt( SENDINFO( m_weaponMode ), 1, SPROP_UNSIGNED ),
SendPropFloat(SENDINFO(m_fAccuracyPenalty) ),
// world weapon models have no aminations
SendPropExclude( "DT_AnimTimeMustBeFirst", "m_flAnimTime" ),
SendPropExclude( "DT_BaseAnimating", "m_nSequence" ),
SendPropTime( SENDINFO( m_flPostponeFireReadyTime ) ),
SendPropBool( SENDINFO( m_bReloadVisuallyComplete ) ),
SendPropTime( SENDINFO( m_flDoneSwitchingSilencer ) ),
SendPropFloat( SENDINFO( m_fLastShotTime ) ),
SendPropFloat( SENDINFO( m_flRecoilIndex ) ),
//	SendPropExclude( "DT_LocalActiveWeaponData", "m_flTimeWeaponIdle" ),
#if IRONSIGHT
SendPropInt( SENDINFO( m_iIronSightMode ), 2, SPROP_UNSIGNED ),
#endif //IRONSIGHT
#else
RecvPropInt( RECVINFO( m_weaponMode ) ),
RecvPropFloat( RECVINFO(m_fAccuracyPenalty)),
RecvPropTime( RECVINFO( m_flPostponeFireReadyTime ) ),
RecvPropBool( RECVINFO( m_bReloadVisuallyComplete ) ),
RecvPropTime( RECVINFO( m_flDoneSwitchingSilencer ) ),
RecvPropFloat( RECVINFO( m_fLastShotTime ) ),
RecvPropFloat( RECVINFO( m_flRecoilIndex ) ),
#if IRONSIGHT
RecvPropInt( RECVINFO( m_iIronSightMode ) ),
#endif //IRONSIGHT
#endif
END_NETWORK_TABLE()

#if defined(CLIENT_DLL)
BEGIN_PREDICTION_DATA( CWeaponCSBase )
	DEFINE_PRED_FIELD( m_flTimeWeaponIdle, FIELD_FLOAT, FTYPEDESC_OVERRIDE | FTYPEDESC_NOERRORCHECK ),
	DEFINE_PRED_FIELD( m_flNextPrimaryAttack, FIELD_FLOAT, FTYPEDESC_OVERRIDE | FTYPEDESC_NOERRORCHECK ),
	DEFINE_PRED_FIELD( m_flNextSecondaryAttack, FIELD_FLOAT, FTYPEDESC_OVERRIDE | FTYPEDESC_NOERRORCHECK ),
	DEFINE_PRED_FIELD( m_bDelayFire, FIELD_BOOLEAN, 0 ),
	DEFINE_PRED_FIELD( m_weaponMode, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD_TOL( m_fAccuracyPenalty, FIELD_FLOAT, FTYPEDESC_INSENDTABLE, 0.00005f ),
	DEFINE_PRED_FIELD( m_fLastShotTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_flPostponeFireReadyTime, FIELD_FLOAT, FTYPEDESC_OVERRIDE | FTYPEDESC_NOERRORCHECK ),
	DEFINE_PRED_FIELD( m_fLastShotTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_flRecoilIndex, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
#if IRONSIGHT
	DEFINE_PRED_FIELD( m_iIronSightMode, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
#endif
END_PREDICTION_DATA()
#endif


LINK_ENTITY_TO_CLASS( weapon_cs_base, CWeaponCSBase );


#ifdef GAME_DLL

	BEGIN_DATADESC( CWeaponCSBase )

		//DEFINE_FUNCTION( DefaultTouch ),
		DEFINE_THINKFUNC( FallThink )

	END_DATADESC()

#endif

#if defined( CLIENT_DLL )
	ConVar cl_crosshairstyle( "cl_crosshairstyle", "2", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "0 = DEFAULT, 1 = DEFAULT STATIC, 2 = ACCURATE SPLIT (accurate recoil/spread feedback with a fixed inner part), 3 = ACCURATE DYNAMIC (accurate recoil/spread feedback), 4 = CLASSIC STATIC, 5 = OLD CS STYLE (fake recoil - inaccurate feedback)" );
	ConVar cl_crosshaircolor( "cl_crosshaircolor", "1", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "Set crosshair color as defined in game_options.consoles.txt" );
	ConVar cl_dynamiccrosshair( "cl_dynamiccrosshair", "1", FCVAR_CLIENTDLL | FCVAR_ARCHIVE ); 
	ConVar cl_scalecrosshair( "cl_scalecrosshair", "1", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "Enable crosshair scaling (deprecated)" );
	ConVar cl_crosshairscale( "cl_crosshairscale", "0", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "Crosshair scaling factor (deprecated)" );
	ConVar cl_crosshairalpha( "cl_crosshairalpha", "200", FCVAR_CLIENTDLL | FCVAR_ARCHIVE );
	ConVar cl_crosshairusealpha( "cl_crosshairusealpha", "1", FCVAR_CLIENTDLL | FCVAR_ARCHIVE );
	ConVar cl_crosshairgap( "cl_crosshairgap", "1", FCVAR_CLIENTDLL | FCVAR_ARCHIVE );
	ConVar cl_crosshairgap_useweaponvalue( "cl_crosshairgap_useweaponvalue", "0", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "If set to 1, the gap will update dynamically based on which weapon is currently equipped" );
	ConVar cl_crosshairsize( "cl_crosshairsize", "5", FCVAR_CLIENTDLL | FCVAR_ARCHIVE );
	ConVar cl_crosshairthickness( "cl_crosshairthickness", "0.5", FCVAR_CLIENTDLL | FCVAR_ARCHIVE );
	ConVar cl_crosshairdot( "cl_crosshairdot", "1", FCVAR_CLIENTDLL | FCVAR_ARCHIVE );
	ConVar cl_crosshaircolor_r( "cl_crosshaircolor_r", "50", FCVAR_CLIENTDLL | FCVAR_ARCHIVE );
	ConVar cl_crosshaircolor_g( "cl_crosshaircolor_g", "250", FCVAR_CLIENTDLL | FCVAR_ARCHIVE );
	ConVar cl_crosshaircolor_b( "cl_crosshaircolor_b", "50", FCVAR_CLIENTDLL | FCVAR_ARCHIVE );
	ConVar weapon_debug_spread_show( "weapon_debug_spread_show", "0", FCVAR_CLIENTDLL | FCVAR_CHEAT, "Enables display of weapon accuracy; 1: show accuracy box, 3: show accuracy with dynamic crosshair" );
	ConVar weapon_debug_spread_gap( "weapon_debug_spread_gap", "0.67", FCVAR_CLIENTDLL | FCVAR_CHEAT );
	ConVar cl_crosshair_drawoutline( "cl_crosshair_drawoutline", "1", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "Draws a black outline around the crosshair for better visibility" );
	ConVar cl_crosshair_outlinethickness( "cl_crosshair_outlinethickness", "1", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "Set how thick you want your crosshair outline to draw (0.1-3)", true, 0.1, true, 3 );
	ConVar cl_crosshair_t( "cl_crosshair_t", "0", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "T style crosshair" );

	ConVar cl_crosshair_dynamic_splitdist("cl_crosshair_dynamic_splitdist", "7", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "If using cl_crosshairstyle 2, this is the distance that the crosshair pips will split into 2. (default is 7)"/*, true, 10, true, 3*/);
	ConVar cl_crosshair_dynamic_splitalpha_innermod("cl_crosshair_dynamic_splitalpha_innermod", "1", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "If using cl_crosshairstyle 2, this is the alpha modification that will be used for the INNER crosshair pips once they've split. [0 - 1]", true, 0, true, 1);
	ConVar cl_crosshair_dynamic_splitalpha_outermod("cl_crosshair_dynamic_splitalpha_outermod", "0.5", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "If using cl_crosshairstyle 2, this is the alpha modification that will be used for the OUTER crosshair pips once they've split. [0.3 - 1]", true, 0.3, true, 1);
	ConVar cl_crosshair_dynamic_maxdist_splitratio("cl_crosshair_dynamic_maxdist_splitratio", "0.35", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "If using cl_crosshairstyle 2, this is the ratio used to determine how long the inner and outer xhair pips will be. [inner = cl_crosshairsize*(1-cl_crosshair_dynamic_maxdist_splitratio), outer = cl_crosshairsize*cl_crosshair_dynamic_maxdist_splitratio]  [0 - 1]", true, 0, true, 1);

#if ALLOW_WEAPON_SPREAD_DISPLAY
	ConVar weapon_debug_spread_show( "weapon_debug_spread_show", "0", FCVAR_CLIENTDLL | FCVAR_DEVELOPMENTONLY, "Enables display of weapon accuracy; 1: show accuracy box, 2: show box with recoil offset" );
	ConVar weapon_debug_spread_gap( "weapon_debug_spread_gap", "0.67", FCVAR_CLIENTDLL | FCVAR_DEVELOPMENTONLY );
#endif

	// [paquin] make sure crosshair scales independent of frame rate
	// unless legacy cvar is set
	ConVar cl_legacy_crosshair_recoil( "cl_legacy_crosshair_recoil", "0", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "Enable legacy framerate dependent crosshair recoil");

	// use old scaling behavior
	ConVar cl_legacy_crosshair_scale( "cl_legacy_crosshair_scale", "0", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "Enable legacy crosshair scaling");

void DrawCrosshairRect( int r, int g, int b, int a, int x0, int y0, int x1, int y1, bool bAdditive )
{
	if ( cl_crosshair_drawoutline.GetBool() )
	{
		float flThick = cl_crosshair_outlinethickness.GetFloat();
		vgui::surface()->DrawSetColor( 0, 0, 0, a );
		vgui::surface()->DrawFilledRect( x0-flThick, y0-flThick, x1+flThick, y1+flThick );
	}

	vgui::surface()->DrawSetColor( r, g, b, a );

	if ( bAdditive )
	{
		vgui::surface()->DrawTexturedRect( x0, y0, x1, y1 );
	}
	else
	{
		// Alpha-blended crosshair
		vgui::surface()->DrawFilledRect( x0, y0, x1, y1 );
	}
}


#endif

// must be included after the above macros
#ifndef CLIENT_DLL
	#include "cs_bot.h"
#endif


// ----------------------------------------------------------------------------- //
// CWeaponCSBase implementation.
// ----------------------------------------------------------------------------- //
CWeaponCSBase::CWeaponCSBase()
{
	SetPredictionEligible( true );
	m_bDelayFire = true;
	m_nextPrevOwnerTouchTime = 0.0;
	m_prevOwner = NULL;
	AddSolidFlags( FSOLID_TRIGGER ); // Nothing collides with these but it gets touches.

#ifdef CLIENT_DLL
	m_iCrosshairTextureID = 0;
#else
	m_iDefaultExtraAmmo = 0;
#endif

	m_fAccuracyPenalty = 0.0f;

	m_fLastShotTime = 0.0f;

	m_flRecoilIndex = 0.0f;

	m_weaponMode = Primary_Mode;

	m_bReloadVisuallyComplete = false;

	m_flDoneSwitchingSilencer = 0.0f;
}

CWeaponCSBase::~CWeaponCSBase()
{

#if IRONSIGHT
	delete m_IronSightController;
	m_IronSightController = NULL;
#endif //IRONSIGHT
}

#ifndef CLIENT_DLL
bool CWeaponCSBase::KeyValue( const char *szKeyName, const char *szValue )
{
	if ( !BaseClass::KeyValue( szKeyName, szValue ) )
	{
		if ( FStrEq( szKeyName, "ammo" ) )
		{
			int bullets = atoi( szValue );
			if ( bullets < 0 )
				return false;

			m_iDefaultExtraAmmo = bullets;

			return true;
		}
	}

	return false;
}
#endif


bool CWeaponCSBase::IsPredicted() const
{
	return true;
}


bool CWeaponCSBase::IsPistol() const
{
	return GetCSWpnData().m_WeaponType == WEAPONTYPE_PISTOL;
}


bool CWeaponCSBase::IsFullAuto() const
{
	return GetCSWpnData().m_bFullAuto;
}


bool CWeaponCSBase::PlayEmptySound()
{
	//MIKETODO: certain weapons should override this to make it empty:
	//	C4
	//	Flashbang
	//	HE Grenade
	//	Smoke grenade

	CPASAttenuationFilter filter( this );
	filter.UsePredictionRules();

	if ( IsPistol() )
	{
		EmitSound( filter, entindex(), "Default.ClipEmpty_Pistol" );
	}
	else
	{
		EmitSound( filter, entindex(), "Default.ClipEmpty_Rifle" );
	}

	return 0;
}

CCSPlayer* CWeaponCSBase::GetPlayerOwner() const
{
	return dynamic_cast< CCSPlayer* >( GetOwner() );
}

//=============================================================================
// HPE_BEGIN:
//=============================================================================

//[dwenger] Accessors for the prior owner list
void CWeaponCSBase::AddToPriorOwnerList(CCSPlayer* pPlayer)
{
    if ( !IsAPriorOwner( pPlayer ) )
    {
        // Add player to prior owner list
        m_PriorOwners.AddToTail( pPlayer );
    }
}

bool CWeaponCSBase::IsAPriorOwner(CCSPlayer* pPlayer)
{
    return (m_PriorOwners.Find( pPlayer ) != -1);
}

//=============================================================================
// HPE_END
//=============================================================================


void CWeaponCSBase::SecondaryAttack( void )
{
#ifndef CLIENT_DLL
	CCSPlayer *pPlayer = GetPlayerOwner();

	if ( !pPlayer )
		return;

	if ( pPlayer->HasShield() == false )
		 BaseClass::SecondaryAttack();
	else
	{
		pPlayer->SetShieldDrawnState( !pPlayer->IsShieldDrawn() );

		if ( pPlayer->IsShieldDrawn() )
			 SendWeaponAnim( ACT_SHIELD_UP );
		else
			 SendWeaponAnim( ACT_SHIELD_DOWN );

		m_flNextSecondaryAttack = gpGlobals->curtime + 0.4;
		m_flNextPrimaryAttack = gpGlobals->curtime + 0.4;
	}
#endif
}

bool CWeaponCSBase::SendWeaponAnim( int iActivity )
{
#ifdef CS_SHIELD_ENABLED
	CCSPlayer *pPlayer = GetPlayerOwner();

	if ( pPlayer && pPlayer->HasShield() )
	{
		CBaseViewModel *vm = pPlayer->GetViewModel( 1 );

		if ( vm == NULL )
			return false;

		vm->SetWeaponModel( SHIELD_VIEW_MODEL, this );

		int	idealSequence = vm->SelectWeightedSequence( (Activity)iActivity );

		if ( idealSequence >= 0 )
		{
			vm->SendViewModelMatchingSequence( idealSequence );
		}
	}

#endif

#ifndef CLIENT_DLL
 // firing or reloading should interrupt weapon inspection
 	if ( iActivity == ACT_VM_PRIMARYATTACK || iActivity == ACT_VM_RELOAD || iActivity == ACT_SECONDARY_VM_RELOAD || iActivity == ACT_VM_ATTACH_SILENCER || iActivity == ACT_VM_DETACH_SILENCER )
 	{
 	 if ( CCSPlayer *pPlayer = GetPlayerOwner() )
 		{
  			pPlayer->StopLookingAtWeapon();
 		}
	}
#endif

	return BaseClass::SendWeaponAnim( iActivity );
}

void CWeaponCSBase::SendViewModelAnim( int nSequence )
{
 CCSPlayer *pPlayer = GetPlayerOwner();
 if ( !pPlayer || pPlayer->IsLookingAtWeapon() )
  return;

 CBaseViewModel *vm = pPlayer->GetViewModel( m_nViewModelIndex );
 if ( vm )
 	{
 	 bool bIsLookingAt = (vm->GetSequence() != ACT_INVALID && V_stristr( vm->GetSequenceName( vm->GetSequence() ), "lookat" ));

 	 if ( vm->GetCycle() < 0.98f && bIsLookingAt && V_stristr( vm->GetSequenceName( nSequence ), "idle" ) )
 		 {
  		 // Don't switch from taunt to idle
  		 return;
 		 }
  }

 BaseClass::SendViewModelAnim( nSequence );
}

// Common code put here to support separate zoom from silencer/burst
void CWeaponCSBase::CallSecondaryAttack()
{
	CCSPlayer *pPlayer = GetPlayerOwner();

	if ( !pPlayer )
		return;

	if ( m_iClip2 != -1 && !GetReserveAmmoCount( AMMO_POSITION_SECONDARY ) )
	{
		m_bFireOnEmpty = TRUE;
	}

	if ( pPlayer->HasShield() )
		CWeaponCSBase::SecondaryAttack();
	else
		SecondaryAttack();
}

void CWeaponCSBase::ItemPostFrame()
{
	CCSPlayer *pPlayer = GetPlayerOwner();

	if ( !pPlayer )
		return;

	UpdateAccuracyPenalty();

	UpdateShieldState();

	if ( ( m_bInReload ) && ( pPlayer->m_flNextAttack <= gpGlobals->curtime ))
	{
		// the AE_WPN_COMPLETE_RELOAD event should handle the stocking the clip, but in case it's missing, we can do it here as well
		int j = MIN( GetMaxClip1() - m_iClip1, GetReserveAmmoCount( AMMO_POSITION_PRIMARY ) );	

		// Add them to the clip
		m_iClip1 += j;
		GiveReserveAmmo( AMMO_POSITION_PRIMARY, -j, true );

		m_bInReload = false;
	}

	if ( (pPlayer->m_nButtons & IN_ATTACK ) && ( m_flNextPrimaryAttack <= gpGlobals->curtime ) )
	{
		ItemPostFrame_ProcessPrimaryAttack( pPlayer );
	}
	else if ( ( pPlayer->m_nButtons & IN_ZOOM ) && ( m_flNextSecondaryAttack <= gpGlobals->curtime ) )
	{
		if ( ItemPostFrame_ProcessZoomAction( pPlayer ) )
			pPlayer->m_nButtons &= ~IN_ZOOM;
	}
	else if ( (pPlayer->m_nButtons & IN_ATTACK2 ) && ( m_flNextSecondaryAttack <= gpGlobals->curtime ))
	{
		if ( ItemPostFrame_ProcessSecondaryAttack( pPlayer ) )
			pPlayer->m_nButtons &= ~IN_ATTACK2;
	}
	else if ( pPlayer->m_nButtons & IN_RELOAD && GetMaxClip1() != WEAPON_NOCLIP && !m_bInReload && m_flNextPrimaryAttack < gpGlobals->curtime ) 
	{
		ItemPostFrame_ProcessReloadAction( pPlayer );
	}
	else if ( !( pPlayer->m_nButtons & ( IN_ATTACK|IN_ATTACK2|IN_ZOOM ) ) )
	{
		ItemPostFrame_ProcessIdleNoAction( pPlayer );
	}
}

void CWeaponCSBase::ItemPostFrame_ProcessPrimaryAttack( CCSPlayer *pPlayer )
{
	if ( (m_iClip1 == 0) || (GetMaxClip1() == -1 && !GetReserveAmmoCount( AMMO_POSITION_PRIMARY )) )
	{
		m_bFireOnEmpty = TRUE;
	}

	if ( CSGameRules()->IsFreezePeriod() )	// Can't shoot during the freeze period
		return;

	if ( pPlayer->m_bIsDefusing )
		return;

	if ( pPlayer->State_Get() != STATE_ACTIVE )
		return;

	if ( pPlayer->IsShieldDrawn() )
		return;

	// don't repeat fire if this is not a full auto weapon or it's clip is empty
	if ( pPlayer->m_iShotsFired > 0 && (!IsFullAuto() || m_iClip1 == 0) )
		return;

#if !defined(CLIENT_DLL)
	// allow the bots to react to the gunfire
	if ( GetCSWpnData().m_WeaponType != WEAPONTYPE_GRENADE )
	{
		IGameEvent * event = gameeventmanager->CreateEvent( (HasAmmo()) ? "weapon_fire" : "weapon_fire_on_empty" );
		if ( event )
		{
			const char *weaponName = STRING( m_iClassname );
			if ( strncmp( weaponName, "weapon_", 7 ) == 0 )
			{
				weaponName += 7;
			}

			event->SetInt( "userid", pPlayer->GetUserID() );
			event->SetString( "weapon", weaponName );
			event->SetBool( "silenced", IsSilenced() );
			gameeventmanager->FireEvent( event );
		}
	}
#endif

	if ( IsRevolver() ) // holding primary, will fire when time is elapsed
	{

		// don't allow a rapid fire shot instantly in the middle of a haul back hold, let the hammer return first
		m_flNextSecondaryAttack = gpGlobals->curtime + 0.25f;

		if ( GetActivity() != ACT_VM_HAULBACK )
		{
			ResetPostponeFireReadyTime();
			BaseClass::SendWeaponAnim( ACT_VM_HAULBACK );
			return;
		}

		m_weaponMode = Primary_Mode;

		if ( !IsPostponFireReadyTimeElapsed() )
			return;

		if ( m_bFireOnEmpty )
		{
			ResetPostponeFireReadyTime();
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + 0.5f;
		}

		// we're going to fire after this point

	}

	PrimaryAttack();
	m_fLastShotTime = gpGlobals->curtime;

	if ( IsRevolver() )
	{
		// we just fired.
		// there's a bit of a cool-off before you can alt-fire at normal alt-fire rate
		m_flNextSecondaryAttack = gpGlobals->curtime + (GetCSWpnData().m_flCycleTimeAlt * 1.7f);
	}

#ifndef CLIENT_DLL
	pPlayer->ClearImmunity();
#endif
}

bool CWeaponCSBase::ItemPostFrame_ProcessZoomAction( CCSPlayer *pPlayer )
{
	if ( IsRevolver() )	// Revolver treats zoom as secondary fire
		return ItemPostFrame_ProcessSecondaryAttack( pPlayer );

	if ( IsKindOf( WEAPONTYPE_SNIPER_RIFLE ) || IsKindOf( WEAPONTYPE_KNIFE ) ) //need test sensitivity on snipers
	{
		CallSecondaryAttack();
	}

	return true;
}

void CWeaponCSBase::CallWeaponIronsight()
{
	CCSPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

#if IRONSIGHT
	if ( pPlayer->GetFOV() == pPlayer->GetDefaultFOV() )
	{
		CIronSightController* pIronSightController = pPlayer->GetActiveCSWeapon()->GetIronSightController();
		if ( pIronSightController )
		{
			pPlayer->GetActiveCSWeapon()->UpdateIronSightController();
			pPlayer->SetFOV( pPlayer, pIronSightController->GetIronSightIdealFOV(), pIronSightController->GetIronSightPullUpDuration() );
			pIronSightController->SetState( IronSight_should_approach_sighted );

			//stop looking at weapon when going into ironsights
#ifndef CLIENT_DLL
			pPlayer->StopLookingAtWeapon();

			//force idle animation
			CBaseViewModel* pViewModel = pPlayer->GetViewModel();
			if ( pViewModel )
			{
				int nSequence = pViewModel->LookupSequence( "idle" );
				if ( nSequence != ACTIVITY_NOT_AVAILABLE )
				{
					pViewModel->ForceCycle( 0 );
					pViewModel->ResetSequence( nSequence );
				}
			}
#endif
			m_weaponMode = Secondary_Mode;
			WeaponSound( SPECIAL3 ); // Zoom in sound
		}
	}
	else
	{
		CIronSightController* pIronSightController = pPlayer->GetActiveCSWeapon()->GetIronSightController();
		if ( pIronSightController )
		{
			pPlayer->GetActiveCSWeapon()->UpdateIronSightController();
			int iFOV = pPlayer->GetDefaultFOV();
			pPlayer->SetFOV( pPlayer, iFOV, pIronSightController->GetIronSightPutDownDuration() );
			pIronSightController->SetState( IronSight_should_approach_unsighted );
			SendWeaponAnim( ACT_VM_FIDGET );
			m_weaponMode = Primary_Mode;
			if ( GetPlayerOwner() )
			{
				WeaponSound( SPECIAL2 ); // Zoom out sound
			}
		}
	}
#else

	if ( pPlayer->GetFOV() == pPlayer->GetDefaultFOV() )
	{
		pPlayer->SetFOV( pPlayer, 55, 0.2f );
		m_weaponMode = Secondary_Mode;
	}
	else if ( pPlayer->GetFOV() == 55 )
	{
		pPlayer->SetFOV( pPlayer, pPlayer->GetDefaultFOV(), 0.15f );
		m_weaponMode = Primary_Mode;
	}
	else 
	{
		pPlayer->SetFOV( pPlayer, pPlayer->GetDefaultFOV() );
		m_weaponMode = Primary_Mode;
	}
#endif

	m_flNextSecondaryAttack = gpGlobals->curtime + 0.3;
}

bool CWeaponCSBase::ItemPostFrame_ProcessSecondaryAttack( CCSPlayer *pPlayer )
{
	if ( IsRevolver() )
	{
		if ( CSGameRules()->IsFreezePeriod() )	// you can't fire the revolver in freezetime
			return false;

		if ( pPlayer->m_bIsDefusing )
			return false;

		if ( pPlayer->State_Get() != STATE_ACTIVE )
			return false;

		if ( ( m_iClip1 == 0 ) || ( GetMaxClip1() == -1 && !GetReserveAmmoCount( AMMO_POSITION_PRIMARY ) ) )
		{
			m_bFireOnEmpty = TRUE;
		}

		m_weaponMode = Secondary_Mode;

		if ( !m_bFireOnEmpty )
		{
			ResetPostponeFireReadyTime();

			if ( GetActivity() == ACT_VM_HAULBACK )
			{
				BaseClass::SendWeaponAnim( ACT_VM_IDLE );
				return false;
			}
		}

		if ( pPlayer->m_iShotsFired > 0 ) // revolver secondary isn't full-auto even though primary is
			return false; // shots fired is zeroed when the buttons release

		if ( m_bFireOnEmpty )
		{
			if ( GetActivity() != ACT_VM_HAULBACK )
			{
				ResetPostponeFireReadyTime();
				BaseClass::SendWeaponAnim( ACT_VM_HAULBACK );
				return false;
			}

			if ( !IsPostponFireReadyTimeElapsed() )
				return false;
		}
	}

#if !defined(CLIENT_DLL)
	// allow the chickens to react to the knife
	if ( GetCSWpnData().m_WeaponType == WEAPONTYPE_KNIFE )
	{
		IGameEvent * event = gameeventmanager->CreateEvent("weapon_fire");
		if ( event )
		{
			const char *weaponName = STRING( m_iClassname );
			if ( strncmp( weaponName, "weapon_", 7 ) == 0 )
			{
				weaponName += 7;
			}

			event->SetInt( "userid", pPlayer->GetUserID() );
			event->SetString( "weapon", weaponName );
			event->SetBool( "silenced", false );
			gameeventmanager->FireEvent( event );
		}
	}
#endif

	if ( GetCSWpnData().m_bIronsightCapable )
	{
		CallWeaponIronsight();
	}
	else
	{
		CallSecondaryAttack();
	}

#ifndef CLIENT_DLL
	pPlayer->ClearImmunity();
#endif

	return true;
}

void CWeaponCSBase::ItemPostFrame_ProcessReloadAction( CCSPlayer *pPlayer )
{
	// reload when reload is pressed, or if no buttons are down and weapon is empty.

	//MIKETODO: add code for shields...
	//if ( !FBitSet( m_iWeaponState, WPNSTATE_SHIELD_DRAWN ) )

	ItemPostFrame_RevolverResetHaulback();

	if ( !pPlayer->IsShieldDrawn() )
	{
		if ( Reload() )
		{
#ifndef CLIENT_DLL
			// allow the bots to react to the reload
			IGameEvent * event = gameeventmanager->CreateEvent( "weapon_reload" );
			if ( event )
			{
				event->SetInt( "userid", pPlayer->GetUserID() );
				gameeventmanager->FireEvent( event );
			}
#endif
		}
	}
}

void CWeaponCSBase::ItemPostFrame_ProcessIdleNoAction( CCSPlayer *pPlayer )
{
	ItemPostFrame_RevolverResetHaulback();

	// no fire buttons down
	m_bFireOnEmpty = FALSE;

	// set the shots fired to 0 after the player releases a button
	pPlayer->m_iShotsFired = 0;

	if ( gpGlobals->curtime > m_flNextPrimaryAttack && m_iClip1 == 0 && IsUseable() && !( GetWeaponFlags() & ITEM_FLAG_NOAUTORELOAD ) && !m_bInReload )
	{
		// Reload if current clip is empty and weapon has waited as long as it has to after firing
		Reload();
		return;
	}

#if IRONSIGHT
	UpdateIronSightController();
	if ( m_iIronSightMode == IronSight_viewmodel_is_deploying && GetActivity() != GetDeployActivity() )
	{
		m_iIronSightMode = IronSight_should_approach_unsighted;
	}
#endif

	WeaponIdle();
}

void CWeaponCSBase::ItemPostFrame_RevolverResetHaulback()
{
	if ( IsRevolver() ) // not holding any weapon buttons
	{
		m_weaponMode = Secondary_Mode;
		ResetPostponeFireReadyTime();
		if ( GetActivity() == ACT_VM_HAULBACK )
		{
			BaseClass::SendWeaponAnim( ACT_VM_IDLE );
		}
	}
}

void CWeaponCSBase::ItemBusyFrame()
{
	UpdateAccuracyPenalty();

	BaseClass::ItemBusyFrame();
}


float CWeaponCSBase::GetInaccuracy() const
{
	CCSPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return 0.0f;

	if ( weapon_accuracy_nospread.GetBool() )
		return 0.0f;

	const CCSWeaponInfo& weaponInfo = GetCSWpnData();

	float fMaxSpeed = GetMaxSpeed();
	if ( fMaxSpeed == 0.0f )
		fMaxSpeed = GetCSWpnData().m_flMaxSpeed;

	return m_fAccuracyPenalty + 
		RemapValClamped(pPlayer->GetAbsVelocity().Length2D(), 
		fMaxSpeed * CS_PLAYER_SPEED_DUCK_MODIFIER, 
		fMaxSpeed * 0.95f,							// max out at 95% of run speed to avoid jitter near max speed
		0.0f, weaponInfo.m_fInaccuracyMove[m_weaponMode]);
}


float CWeaponCSBase::GetSpread() const
{
	if ( weapon_accuracy_model.GetInt() == 1 )
		return 0.0f;

	return GetCSWpnData().m_fSpread[m_weaponMode];
}


float CWeaponCSBase::GetMaxSpeed() const
{
	// The weapon should have set this in its constructor.
	float flRet = GetCSWpnData().m_flMaxSpeed;
	Assert( flRet > 1 );
	return flRet;
}

const CCSWeaponInfo &CWeaponCSBase::GetCSWpnData() const
{
	const FileWeaponInfo_t *pWeaponInfo = &GetWpnData();
	const CCSWeaponInfo *pCSInfo;

	#ifdef _DEBUG
		pCSInfo = dynamic_cast< const CCSWeaponInfo* >( pWeaponInfo );
		Assert( pCSInfo );
	#else
		pCSInfo = static_cast< const CCSWeaponInfo* >( pWeaponInfo );
	#endif

	return *pCSInfo;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
const char *CWeaponCSBase::GetViewModel( int /*viewmodelindex = 0 -- this is ignored in the base class here*/ ) const
{
	CCSPlayer *pOwner = GetPlayerOwner();

	if ( pOwner == NULL )
		 return BaseClass::GetViewModel();

	if ( pOwner->HasShield() && GetCSWpnData().m_bCanUseWithShield )
		 return GetCSWpnData().m_szShieldViewModel;
	else
		 return GetWpnData().szViewModel;

	return BaseClass::GetViewModel();

}

void CWeaponCSBase::Precache( void )
{
	BaseClass::Precache();

#ifdef CS_SHIELD_ENABLED
	if ( GetCSWpnData().m_bCanUseWithShield )
	{
		 PrecacheModel( GetCSWpnData().m_szShieldViewModel );
	}
#endif

	if ( GetCSWpnData().m_szMuzzleFlash1stPerson[0] != 0 )
		PrecacheParticleSystem( GetCSWpnData().m_szMuzzleFlash1stPerson );
	if ( GetCSWpnData().m_szMuzzleFlash1stPersonAlt[0] != 0 )
		PrecacheParticleSystem( GetCSWpnData().m_szMuzzleFlash1stPersonAlt );
	if ( GetCSWpnData().m_szMuzzleFlash3rdPerson[0] != 0 )
		PrecacheParticleSystem( GetCSWpnData().m_szMuzzleFlash3rdPerson );
	if ( GetCSWpnData().m_szMuzzleFlash3rdPersonAlt[0] != 0 )
		PrecacheParticleSystem( GetCSWpnData().m_szMuzzleFlash3rdPersonAlt );

	PrecacheScriptSound( "Default.ClipEmpty_Pistol" );
	PrecacheScriptSound( "Default.ClipEmpty_Rifle" );

	PrecacheScriptSound( "Default.Zoom" );

	// PiMoN: weaponscript parsing happens on Precache() of BaseCombatWeapon
	// so moving it here from construct is actually a good solution, all
	// those players with not working ironsights just had different problems...
	// lost a week to understand that I was fixing what was working as intended >-<
#if IRONSIGHT
	m_iIronSightMode = IronSight_should_approach_unsighted;
	m_IronSightController = NULL;
	UpdateIronSightController();
#endif //IRONSIGHT
}

Activity CWeaponCSBase::GetDeployActivity( void )
{
	return ACT_VM_DRAW;
}

bool CWeaponCSBase::DefaultDeploy( char *szViewModel, char *szWeaponModel, int iActivity, char *szAnimExt )
{
	// Msg( "deploy %s at %f\n", GetClassname(), gpGlobals->curtime );
	CCSPlayer *pOwner = GetPlayerOwner();
	if ( !pOwner )
	{
		return false;
	}

	pOwner->SetAnimationExtension( szAnimExt );

	SetViewModel();
	SendWeaponAnim( GetDeployActivity() );

	pOwner->SetNextAttack( gpGlobals->curtime + SequenceDuration() );
	m_flNextPrimaryAttack	= gpGlobals->curtime;
	m_flNextSecondaryAttack	= gpGlobals->curtime;

	SetWeaponVisible( true );
	pOwner->SetShieldDrawnState( false );

	if ( pOwner->HasShield() == true )
		 SetWeaponModelIndex( SHIELD_WORLD_MODEL);
	else
		 SetWeaponModelIndex( szWeaponModel );

#if IRONSIGHT
	m_iIronSightMode = IronSight_viewmodel_is_deploying;
#endif //IRONSIGHT

	return true;
}

void CWeaponCSBase::UpdateShieldState( void )
{
	//empty by default.
	CCSPlayer *pOwner = GetPlayerOwner();

	if ( pOwner == NULL )
		 return;

	//ADRIANTODO
	//Make the hitbox set switches here!!!
	if ( pOwner->HasShield() == false )
	{

		pOwner->SetShieldDrawnState( false );
		//pOwner->SetHitBoxSet( 0 );
		return;
	}
	else
	{
		//pOwner->SetHitBoxSet( 1 );
	}
}

void CWeaponCSBase::SetWeaponModelIndex( const char *pName )
{
 	 m_iWorldModelIndex = modelinfo->GetModelIndex( pName );
}

bool CWeaponCSBase::CanBeSelected( void )
{
	if ( !VisibleInWeaponSelection() )
		return false;

	return true;
}

bool CWeaponCSBase::CanDeploy( void )
{
	CCSPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return false;

	if ( pPlayer->HasShield() && GetCSWpnData().m_bCanUseWithShield == false )
		 return false;

	return BaseClass::CanDeploy();
}

float CWeaponCSBase::CalculateNextAttackTime( float fCycleTime )
{
	float fCurAttack = m_flNextPrimaryAttack;
	float fDeltaAttack = gpGlobals->curtime - fCurAttack;
	if ( fDeltaAttack < 0 || fDeltaAttack > gpGlobals->interval_per_tick )
	{
		fCurAttack = gpGlobals->curtime;
	}
	m_flNextSecondaryAttack = m_flNextPrimaryAttack = fCurAttack + fCycleTime;

	return fCurAttack;
}

bool CWeaponCSBase::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	CCSPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return false;

	if ( pPlayer )
		pPlayer->SetFOV( pPlayer, 0 ); // reset the default FOV.

	if ( pPlayer )
		pPlayer->SetShieldDrawnState( false );

	return BaseClass::Holster( pSwitchingTo );
}

bool CWeaponCSBase::Deploy()
{
#if IRONSIGHT
	if ( GetIronSightController() )
		GetIronSightController()->SetState( IronSight_viewmodel_is_deploying );
#endif //IRONSIGHT

	CCSPlayer *pPlayer = GetPlayerOwner();

#ifdef CLIENT_DLL
	m_iAlpha =  80;
	if ( pPlayer )
	{
		pPlayer->m_bIsScoped = false;
		pPlayer->m_iLastZoom = 0;
		pPlayer->SetFOV( pPlayer, 0 );
	}
#else

	m_flDecreaseShotsFired = gpGlobals->curtime;


	if ( pPlayer )
	{
		pPlayer->m_iShotsFired = 0;
		pPlayer->m_bResumeZoom = false;
		pPlayer->m_iLastZoom = 0;
		pPlayer->SetFOV( pPlayer, 0 );
	}
#endif

	m_fAccuracyPenalty = 0.0f;

	return BaseClass::Deploy();
}

#ifndef CLIENT_DLL
bool CWeaponCSBase::IsRemoveable()
{
	if ( BaseClass::IsRemoveable() == true )
	{
		if ( m_nextPrevOwnerTouchTime > gpGlobals->curtime )
		{
			return false;
		}
	}

	return BaseClass::IsRemoveable();
}
#endif

void CWeaponCSBase::Drop(const Vector &vecVelocity)
{

#ifdef CLIENT_DLL
	BaseClass::Drop(vecVelocity);
	return;
#else

	// Once somebody drops a gun, it's fair game for removal when/if
	// a game_weapon_manager does a cleanup on surplus weapons in the
	// world.
	SetRemoveable( true );

	StopAnimation();
	StopFollowingEntity( );
	SetMoveType( MOVETYPE_FLYGRAVITY );
	// clear follow stuff, setup for collision
	SetGravity(1.0);
	m_iState = WEAPON_NOT_CARRIED;
	RemoveEffects( EF_NODRAW );
	FallInit();
	SetGroundEntity( NULL );

	m_bInReload = false; // stop reloading

	SetThink( NULL );
	m_nextPrevOwnerTouchTime = gpGlobals->curtime + 0.8f;
	m_prevOwner = GetPlayerOwner();

	SetTouch(&CWeaponCSBase::DefaultTouch);

	IPhysicsObject *pObj = VPhysicsGetObject();
	if ( pObj != NULL )
	{
		AngularImpulse	angImp( 200, 200, 200 );
		pObj->AddVelocity( &vecVelocity, &angImp );
	}
	else
	{
		SetAbsVelocity( vecVelocity );
	}

	SetNextThink( gpGlobals->curtime );

	SetOwnerEntity( NULL );
	SetOwner( NULL );

	m_bReloadVisuallyComplete = false;

#if IRONSIGHT
	if ( GetIronSightController() )
		GetIronSightController()->SetState( IronSight_weapon_is_dropped );
#endif

#endif
}

// whats going on here is that if the player drops this weapon, they shouldn't take it back themselves
// for a little while.  But if they throw it at someone else, the other player should get it immediately.
void CWeaponCSBase::DefaultTouch(CBaseEntity *pOther)
{
	if ((m_prevOwner != NULL) && (pOther == m_prevOwner) && (gpGlobals->curtime < m_nextPrevOwnerTouchTime))
	{
		return;
	}

	BaseClass::DefaultTouch(pOther);
}

#if defined( CLIENT_DLL )

	//-----------------------------------------------------------------------------
	// Purpose: Draw the weapon's crosshair
	//-----------------------------------------------------------------------------
	void CWeaponCSBase::DrawCrosshair()
	{
		if ( !crosshair.GetInt() )
			return;
		CHudCrosshair *pCrosshair = GET_HUDELEMENT( CHudCrosshair );

		// clear old hud crosshair
		if ( !pCrosshair )
			return;
		pCrosshair->SetCrosshair( 0, Color( 255, 255, 255, 255 ) );

		CCSPlayer* pPlayer = (CCSPlayer*) C_BasePlayer::GetLocalPlayer();

		if ( !pPlayer || GetPlayerOwner() == NULL )
			return;

		Assert( (pPlayer == GetPlayerOwner()) || (pPlayer->GetObserverMode() == OBS_MODE_IN_EYE) );
		if ( pPlayer->IsInVGuiInputMode() )
			return;



		int	r, g, b;
		switch ( cl_crosshaircolor.GetInt() )
		{
			case 0:	r = 250;	g = 50;		b = 50;		break;
			case 1:	r = 50;		g = 250;	b = 50;		break;
			case 2:	r = 250;	g = 250;	b = 50;		break;
			case 3:	r = 50;		g = 50;		b = 250;	break;
			case 4:	r = 50;		g = 250;	b = 250;	break;
			case 5:
				r = cl_crosshaircolor_r.GetInt();
				g = cl_crosshaircolor_g.GetInt();
				b = cl_crosshaircolor_b.GetInt();
				break;
			default:	r = 50;		g = 250;	b = 50;		break;
		}

		// if user is using nightvision, make the crosshair red.
		if ( pPlayer->m_bNightVisionOn )
		{
			r = 250;
			g = 50;
			b = 50;
		}

		int alpha = clamp( cl_crosshairalpha.GetInt(), 0, 255 );

		if ( !m_iCrosshairTextureID )
		{
			CHudTexture *pTexture = gHUD.GetIcon( "whiteAdditive" );
			if ( pTexture )
			{
				m_iCrosshairTextureID = pTexture->textureId;
			}
		}

		bool bAdditive = !cl_crosshairusealpha.GetBool() && !pPlayer->m_bNightVisionOn;
		if ( bAdditive )
		{
			vgui::surface()->DrawSetTexture( m_iCrosshairTextureID );
			alpha = 200;
		}

#ifdef IRONSIGHT
		if ( GetIronSightController() && GetIronSightController()->ShouldHideCrossHair() )
		{
			alpha = 0;
		}
#endif

		bool bCrosshairVisible = crosshair.GetBool() && GetCSWpnData().m_WeaponType != WEAPONTYPE_SNIPER_RIFLE;

		if ( pPlayer->HasShield() && pPlayer->IsShieldDrawn() == true )
			return;
		if ( !bCrosshairVisible )
			 return;

		float fHalfFov = DEG2RAD( pPlayer->GetFOV() ) * 0.5f;
		float flInaccuracy = GetInaccuracy();
		float flSpread = GetSpread();

		//	float flCrossDistanceScale = cl_crosshair_dynamic_distancescale.GetFloat();
		float fSpreadDistance = ((flInaccuracy + flSpread) * 320.0f / tanf( fHalfFov ))/* * flCrossDistanceScale*/;
		float flCappedSpreadDistance = fSpreadDistance;
		float flMaxCrossDistance = cl_crosshair_dynamic_splitdist.GetFloat();
		if ( fSpreadDistance > flMaxCrossDistance )
		{
			// 		bHitMax = true;
			// 		//flLineAlpha = alpha * MAX(0, 1 - ((fSpreadDistance - flMaxCrossDistance) / 20));
			flCappedSpreadDistance = flMaxCrossDistance;
		}

		int iSpreadDistance = cl_crosshairstyle.GetInt() < 4 ? RoundFloatToInt( YRES( fSpreadDistance ) ) : 2;
		int iCappedSpreadDistance = cl_crosshairstyle.GetInt() < 4 ? RoundFloatToInt( YRES( flCappedSpreadDistance ) ) : 2;

		int iDeltaDistance = GetCSWpnData().m_iCrosshairDeltaDistance; // Amount by which the crosshair expands when shooting ( per frame )
		float fCrosshairDistanceGoal = cl_crosshairgap_useweaponvalue.GetBool() ? GetCSWpnData().m_iCrosshairMinDistance : 4; // The minimum distance the crosshair can achieve...

		//0 = default
		//1 = default static
		//2 = classic standard
		//3 = classic dynamic
		//4 = classic static
		//if ( cl_dynamiccrosshair.GetBool() )
		if ( cl_crosshairstyle.GetInt() != 4 && (cl_crosshairstyle.GetInt() == 2 || cl_crosshairstyle.GetInt() == 3) )
		{
			if ( !(pPlayer->GetFlags() & FL_ONGROUND) )
				fCrosshairDistanceGoal *= 2.0f;
			else if ( pPlayer->GetFlags() & FL_DUCKING )
				fCrosshairDistanceGoal *= 0.5f;
			else if ( pPlayer->GetAbsVelocity().Length() > 100 )
				fCrosshairDistanceGoal *= 1.5f;
		}

		// [jpaquin] changed to only bump up the crosshair size if the player is still shooting or is spectating someone else
		if ( pPlayer->m_iShotsFired > m_iAmmoLastCheck && (pPlayer->m_nButtons & (IN_ATTACK | IN_ATTACK2)) && m_iClip1 >= 0 )
		{
			if ( cl_crosshairstyle.GetInt() == 5 )
				m_flCrosshairDistance += (GetCSWpnData().m_fRecoilMagnitude[m_weaponMode] / 3.5);
			else if ( cl_crosshairstyle.GetInt() != 4 )
				fCrosshairDistanceGoal += iDeltaDistance;
		}

		m_iAmmoLastCheck = pPlayer->m_iShotsFired;

		if ( m_flCrosshairDistance > fCrosshairDistanceGoal )
		{
			if ( cl_crosshairstyle.GetInt() == 5 )
			{
				//float flPer = ( gpGlobals->frametime / 0.025f );
				//m_flCrosshairDistance -= (0.11 + ( m_flCrosshairDistance * 0.01 )) * gpGlobals->frametime;
				m_flCrosshairDistance -= 42.0 * gpGlobals->frametime;
			}
			else
				m_flCrosshairDistance = Lerp( (gpGlobals->frametime / 0.025f), fCrosshairDistanceGoal, m_flCrosshairDistance );
		}

		// clamp max crosshair expansion
		m_flCrosshairDistance = clamp( m_flCrosshairDistance, fCrosshairDistanceGoal, 25.0f );

		int iCrosshairDistance;
		int iCappedCrosshairDistance = 0;
		int iBarSize;
		int iBarThickness;

		iCrosshairDistance = RoundFloatToInt( (m_flCrosshairDistance * ScreenHeight() / 1200.0f) + cl_crosshairgap.GetFloat() );
		iBarSize = RoundFloatToInt( YRES( cl_crosshairsize.GetFloat() ) );
		iBarThickness = MAX( 1, RoundFloatToInt( YRES( cl_crosshairthickness.GetFloat() ) ) );

		//0 = default
		//1 = default static
		//2 = classic standard
		//3 = classic dynamic
		//4 = classic static
		//if ( weapon_debug_spread_show.GetInt() == 2 )
		if ( weapon_debug_spread_show.GetInt() == 2 || (iSpreadDistance > 0 && (cl_crosshairstyle.GetInt() == 2 || cl_crosshairstyle.GetInt() == 3)) )
		{
			iCrosshairDistance = iSpreadDistance + cl_crosshairgap.GetFloat();

			if ( cl_crosshairstyle.GetInt() == 2 )
				iCappedCrosshairDistance = iCappedSpreadDistance + cl_crosshairgap.GetFloat();
		}
		else if ( cl_crosshairstyle.GetInt() == 4 || (iSpreadDistance == 0 && (cl_crosshairstyle.GetInt() == 2 || cl_crosshairstyle.GetInt() == 3)) )
		{
			iCrosshairDistance = fCrosshairDistanceGoal + cl_crosshairgap.GetFloat();
			iCappedCrosshairDistance = 4 + cl_crosshairgap.GetFloat();
		}

#ifdef SIXENSE
		int iCenterX;
		int iCenterY;

		if ( g_pSixenseInput->IsEnabled() )
		{
			// Never autoaim a predicted weapon (for now)
			Vector	aimVector;
			AngleVectors( CurrentViewAngles() - g_pSixenseInput->GetViewAngleOffset(), &aimVector );

			// calculate where the bullet would go so we can draw the cross appropriately
			Vector vecStart = pPlayer->Weapon_ShootPosition();
			Vector vecEnd = pPlayer->Weapon_ShootPosition() + aimVector * MAX_TRACE_LENGTH;


			trace_t tr;
			UTIL_TraceLine( vecStart, vecEnd, MASK_SHOT, pPlayer, COLLISION_GROUP_NONE, &tr );

			Vector screen;
			screen.Init();
			ScreenTransform( tr.endpos, screen );

			iCenterX = ScreenWidth() / 2;
			iCenterY = ScreenHeight() / 2;

			iCenterX += 0.5 * screen[0] * ScreenWidth() + 0.5;
			iCenterY += 0.5 * screen[1] * ScreenHeight() + 0.5;
			iCenterY = ScreenHeight() - iCenterY;

		}
		else
		{
			iCenterX = ScreenWidth() / 2;
			iCenterY = ScreenHeight() / 2;
		}

#else
		int iCenterX = ScreenWidth() / 2;
		int iCenterY = ScreenHeight() / 2;
#endif

		float flAngleToScreenPixel = 0;



		/*
		// Optionally subtract out viewangle since it doesn't affect shooting.
		if ( cl_flinch_compensate_crosshair.GetBool() )
		{
		QAngle viewPunch = pPlayer->GetViewPunchAngle();

		if ( viewPunch.x != 0 || viewPunch.y != 0 )
		{
		if ( flAngleToScreenPixel == 0 )
		flAngleToScreenPixel = VIEWPUNCH_COMPENSATE_MAGIC_SCALAR * 2 * ( ScreenHeight() / ( 2.0f * tanf(DEG2RAD( pPlayer->GetFOV() ) / 2.0f) ) );

		iCenterY -= flAngleToScreenPixel * sinf( DEG2RAD( viewPunch.x ) );
		iCenterX += flAngleToScreenPixel * sinf( DEG2RAD( viewPunch.y ) );
		}
		}
		*/

		float flAlphaSplitInner = cl_crosshair_dynamic_splitalpha_innermod.GetFloat();
		float flAlphaSplitOuter = cl_crosshair_dynamic_splitalpha_outermod.GetFloat();
		float flSplitRatio = cl_crosshair_dynamic_maxdist_splitratio.GetFloat();
		int iInnerCrossDist = iCrosshairDistance;
		float flLineAlphaInner = alpha;
		float flLineAlphaOuter = alpha;
		int iBarSizeInner = iBarSize;
		int iBarSizeOuter = iBarSize;

		// draw the crosshair that splits off from the main xhair
		if ( cl_crosshairstyle.GetInt() == 2 && fSpreadDistance > flMaxCrossDistance )
		{
			iInnerCrossDist = iCappedCrosshairDistance;
			flLineAlphaInner = alpha * flAlphaSplitInner;
			flLineAlphaOuter = alpha * flAlphaSplitOuter;
			iBarSizeInner = ceil( (float) iBarSize * (1.0f - flSplitRatio) );
			iBarSizeOuter = floor( (float) iBarSize * flSplitRatio );

			// draw horizontal crosshair lines
			int iInnerLeft = (iCenterX - iCrosshairDistance - iBarThickness / 2) - iBarSizeInner;
			int iInnerRight = iInnerLeft + 2 * (iCrosshairDistance + iBarSizeInner) + iBarThickness;
			int iOuterLeft = iInnerLeft - iBarSizeOuter;
			int iOuterRight = iInnerRight + iBarSizeOuter;
			int y0 = iCenterY - iBarThickness / 2;
			int y1 = y0 + iBarThickness;
			DrawCrosshairRect( r, g, b, flLineAlphaOuter, iOuterLeft, y0, iInnerLeft, y1, bAdditive );
			DrawCrosshairRect( r, g, b, flLineAlphaOuter, iInnerRight, y0, iOuterRight, y1, bAdditive );

			// draw vertical crosshair lines
			int iInnerTop = (iCenterY - iCrosshairDistance - iBarThickness / 2) - iBarSizeInner;
			int iInnerBottom = iInnerTop + 2 * (iCrosshairDistance + iBarSizeInner) + iBarThickness;
			int iOuterTop = iInnerTop - iBarSizeOuter;
			int iOuterBottom = iInnerBottom + iBarSizeOuter;
			int x0 = iCenterX - iBarThickness / 2;
			int x1 = x0 + iBarThickness;
			if ( !cl_crosshair_t.GetBool() )
				DrawCrosshairRect( r, g, b, flLineAlphaOuter, x0, iOuterTop, x1, iInnerTop, bAdditive );
			DrawCrosshairRect( r, g, b, flLineAlphaOuter, x0, iInnerBottom, x1, iOuterBottom, bAdditive );
		}

		// draw horizontal crosshair lines
		int iInnerLeft = iCenterX - iInnerCrossDist - iBarThickness / 2;
		int iInnerRight = iInnerLeft + 2 * iInnerCrossDist + iBarThickness;
		int iOuterLeft = iInnerLeft - iBarSizeInner;
		int iOuterRight = iInnerRight + iBarSizeInner;
		int y0 = iCenterY - iBarThickness / 2;
		int y1 = y0 + iBarThickness;
		DrawCrosshairRect( r, g, b, flLineAlphaInner, iOuterLeft, y0, iInnerLeft, y1, bAdditive );
		DrawCrosshairRect( r, g, b, flLineAlphaInner, iInnerRight, y0, iOuterRight, y1, bAdditive );

		// draw vertical crosshair lines
		int iInnerTop = iCenterY - iInnerCrossDist - iBarThickness / 2;
		int iInnerBottom = iInnerTop + 2 * iInnerCrossDist + iBarThickness;
		int iOuterTop = iInnerTop - iBarSizeInner;
		int iOuterBottom = iInnerBottom + iBarSizeInner;
		int x0 = iCenterX - iBarThickness / 2;
		int x1 = x0 + iBarThickness;
		if ( !cl_crosshair_t.GetBool() )
			DrawCrosshairRect( r, g, b, flLineAlphaInner, x0, iOuterTop, x1, iInnerTop, bAdditive );
		DrawCrosshairRect( r, g, b, flLineAlphaInner, x0, iInnerBottom, x1, iOuterBottom, bAdditive );

		// draw dot
		if ( cl_crosshairdot.GetBool() )
		{
			int x0 = iCenterX - iBarThickness / 2;
			int x1 = x0 + iBarThickness;
			int y0 = iCenterY - iBarThickness / 2;
			int y1 = y0 + iBarThickness;
			DrawCrosshairRect( r, g, b, alpha, x0, y0, x1, y1, bAdditive );
		}

		if ( weapon_debug_spread_show.GetInt() == 1 )
		{
			// colors
			r = 250;
			g = 250;
			b = 50;
			//vgui::surface()->DrawSetColor( r, g, b, alpha );

			int iBarThickness = MAX( 1, RoundFloatToInt( YRES( cl_crosshairthickness.GetFloat() ) ) );

			// draw vertical spread lines
			int iInnerLeft = iCenterX - iSpreadDistance;
			int iInnerRight = iCenterX + iSpreadDistance;
			int iOuterLeft = iInnerLeft - iBarThickness;
			int iOuterRight = iInnerRight + iBarThickness;
			int iInnerTop = iCenterY - iSpreadDistance;
			int iInnerBottom = iCenterY + iSpreadDistance;
			int iOuterTop = iInnerTop - iBarThickness;
			int iOuterBottom = iInnerBottom + iBarThickness;

			int iGap = RoundFloatToInt( weapon_debug_spread_gap.GetFloat() * iSpreadDistance );

			// draw horizontal lines
			DrawCrosshairRect( r, g, b, alpha, iOuterLeft, iOuterTop, iCenterX - iGap, iInnerTop, bAdditive );
			DrawCrosshairRect( r, g, b, alpha, iCenterX + iGap, iOuterTop, iOuterRight, iInnerTop, bAdditive );
			DrawCrosshairRect( r, g, b, alpha, iOuterLeft, iInnerBottom, iCenterX - iGap, iOuterBottom, bAdditive );
			DrawCrosshairRect( r, g, b, alpha, iCenterX + iGap, iInnerBottom, iOuterRight, iOuterBottom, bAdditive );

			// draw vertical lines
			DrawCrosshairRect( r, g, b, alpha, iOuterLeft, iOuterTop, iInnerLeft, iCenterY - iGap, bAdditive );
			DrawCrosshairRect( r, g, b, alpha, iOuterLeft, iCenterY + iGap, iInnerLeft, iOuterBottom, bAdditive );
			DrawCrosshairRect( r, g, b, alpha, iInnerRight, iOuterTop, iOuterRight, iCenterY - iGap, bAdditive );
			DrawCrosshairRect( r, g, b, alpha, iInnerRight, iCenterY + iGap, iOuterRight, iOuterBottom, bAdditive );
		}

	}

	void CWeaponCSBase::OnDataChanged( DataUpdateType_t type )
	{
		BaseClass::OnDataChanged( type );

#if IRONSIGHT
		UpdateIronSightController();
#endif //IRONSIGHT

		if ( GetPredictable() && !ShouldPredict() )
			ShutdownPredictable();
	}


	bool CWeaponCSBase::ShouldPredict()
	{
		if ( GetOwner() && GetOwner() == C_BasePlayer::GetLocalPlayer() )
			return true;

		return BaseClass::ShouldPredict();
	}

	void CWeaponCSBase::ProcessMuzzleFlashEvent()
	{
		// This is handled from the player's animstate, so it can match up to the beginning of the fire animation
	}

#ifdef CLIENT_DLL
	int CWeaponCSBase::GetMuzzleAttachmentIndex( C_BaseAnimating* pAnimating, bool isThirdPerson )
	{
		if ( pAnimating )
		{
			if ( IsSilenced() )
				return pAnimating->LookupAttachment( "muzzle_flash2" );

			if ( isThirdPerson )
				return pAnimating->LookupAttachment( "muzzle_flash" );
			else
				return pAnimating->LookupAttachment( "1" );
		}
		return -1;
	}

	const char* CWeaponCSBase::GetMuzzleFlashEffectName( bool bThirdPerson )
	{
		if ( IsSilenced() )
		{
			return bThirdPerson ? GetCSWpnData().m_szMuzzleFlash3rdPersonAlt : GetCSWpnData().m_szMuzzleFlash1stPersonAlt;
		}
		else
		{
			return bThirdPerson ? GetCSWpnData().m_szMuzzleFlash3rdPerson : GetCSWpnData().m_szMuzzleFlash1stPerson;
		}
	}

	int CWeaponCSBase::GetEjectBrassAttachmentIndex( C_BaseAnimating* pAnimating, bool isThirdPerson )
	{
		if ( pAnimating )
		{
			if ( isThirdPerson )
				return pAnimating->LookupAttachment( "shell_eject" );
			else
				return pAnimating->LookupAttachment( "2" );
		}

		return -1;
	}
#endif

	bool CWeaponCSBase::OnFireEvent( C_BaseViewModel *pViewModel, const Vector& origin, const QAngle& angles, int event, const char *options )
	{
		if( event == 5001 )
		{
			C_CSPlayer *pPlayer = ToCSPlayer( GetOwner() );
			if( pPlayer && pPlayer->GetFOV() < pPlayer->GetDefaultFOV() && HideViewModelWhenZoomed() )
				return true;

			if ( pPlayer && pPlayer->GetFOV() != pPlayer->GetDefaultFOV() && pPlayer->m_bIsScoped )
				return true;

			CEffectData data;
			data.m_fFlags = 0;
			data.m_hEntity = pViewModel->GetRefEHandle();
			data.m_nAttachmentIndex = 1;
			data.m_flScale = GetCSWpnData().m_flMuzzleScale;

			switch( GetMuzzleFlashStyle() )
			{
			case CS_MUZZLEFLASH_NONE:
				break;

			case CS_MUZZLEFLASH_X:
				{
					DispatchEffect( "CS_MuzzleFlash_X", data );
				}
				break;

			case CS_MUZZLEFLASH_NORM:
			default:
				{
					DispatchEffect( "CS_MuzzleFlash", data );
				}
				break;
			}

			return true;
		}

		return BaseClass::OnFireEvent( pViewModel, origin, angles, event, options );
	}

	int CWeaponCSBase::GetMuzzleFlashStyle( void )
	{
		return GetCSWpnData().m_iMuzzleFlashStyle;
	}

	int CWeaponCSBase::GetMuzzleAttachment( void )
	{
		return LookupAttachment( "muzzle_flash" );
	}

#else

	//-----------------------------------------------------------------------------
	// Purpose: Get the accuracy derived from weapon and player, and return it
	//-----------------------------------------------------------------------------
	const Vector& CWeaponCSBase::GetBulletSpread()
	{
		static Vector cone = VECTOR_CONE_8DEGREES;
		return cone;
	}

	//-----------------------------------------------------------------------------
	// Purpose: Match the anim speed to the weapon speed while crouching
	//-----------------------------------------------------------------------------
	float CWeaponCSBase::GetDefaultAnimSpeed()
	{
		return 1.0;
	}

	//-----------------------------------------------------------------------------
	// Purpose: Draw the laser rifle effect
	//-----------------------------------------------------------------------------
	void CWeaponCSBase::BulletWasFired( const Vector &vecStart, const Vector &vecEnd )
	{
	}


	bool CWeaponCSBase::ShouldRemoveOnRoundRestart()
	{
		if ( GetPlayerOwner() )
			return false;
		else
			return true;
	}


    //=============================================================================
    // HPE_BEGIN:
    // [dwenger] Handle round restart processing for the weapon.
    //=============================================================================

    void CWeaponCSBase::OnRoundRestart()
    {
        // Clear out the list of prior owners
        m_PriorOwners.RemoveAll();
    }

    //=============================================================================
    // HPE_END
    //=============================================================================

	//=========================================================
	// Materialize - make a CWeaponCSBase visible and tangible
	//=========================================================
	void CWeaponCSBase::Materialize()
	{
		if ( IsEffectActive( EF_NODRAW ) )
		{
			// changing from invisible state to visible.
			RemoveEffects( EF_NODRAW );
			DoMuzzleFlash();
		}

		AddSolidFlags( FSOLID_TRIGGER );

		//SetTouch( &CWeaponCSBase::DefaultTouch );

		SetThink( NULL );

	}

	//=========================================================
	// AttemptToMaterialize - the item is trying to rematerialize,
	// should it do so now or wait longer?
	//=========================================================
	void CWeaponCSBase::AttemptToMaterialize()
	{
		float time = g_pGameRules->FlWeaponTryRespawn( this );

		if ( time == 0 )
		{
			Materialize();
			return;
		}

		SetNextThink( gpGlobals->curtime + time );
	}

	//=========================================================
	// CheckRespawn - a player is taking this weapon, should
	// it respawn?
	//=========================================================
	void CWeaponCSBase::CheckRespawn()
	{
		//GOOSEMAN : Do not respawn weapons!
		return;
	}


	//=========================================================
	// Respawn- this item is already in the world, but it is
	// invisible and intangible. Make it visible and tangible.
	//=========================================================
	CBaseEntity* CWeaponCSBase::Respawn()
	{
		// make a copy of this weapon that is invisible and inaccessible to players (no touch function). The weapon spawn/respawn code
		// will decide when to make the weapon visible and touchable.
		CBaseEntity *pNewWeapon = CBaseEntity::Create( GetClassname(), g_pGameRules->VecWeaponRespawnSpot( this ), GetAbsAngles(), GetOwner() );

		if ( pNewWeapon )
		{
			pNewWeapon->AddEffects( EF_NODRAW );// invisible for now
			pNewWeapon->SetTouch( NULL );// no touch
			pNewWeapon->SetThink( &CWeaponCSBase::AttemptToMaterialize );

			UTIL_DropToFloor( this, MASK_SOLID );

			// not a typo! We want to know when the weapon the player just picked up should respawn! This new entity we created is the replacement,
			// but when it should respawn is based on conditions belonging to the weapon that was taken.
			pNewWeapon->SetNextThink( gpGlobals->curtime + g_pGameRules->FlWeaponRespawnTime( this ) );
		}
		else
		{
			Msg( "Respawn failed to create %s!\n", GetClassname() );
		}

		return pNewWeapon;
	}

	//-----------------------------------------------------------------------------
	// Purpose:
	//-----------------------------------------------------------------------------
	void CWeaponCSBase::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
	{
		CBasePlayer *pPlayer = ToBasePlayer( pActivator );
		
		if ( pPlayer )
		{
			m_OnPlayerUse.FireOutput( pActivator, pCaller );
		}
	}

	void CWeaponCSBase::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
{
	int nEvent = pEvent->event;

	 if ( (pEvent->type & AE_TYPE_NEWEVENTSYSTEM) && (pEvent->type & AE_TYPE_SERVER) )
		{
			if ( nEvent == AE_WPN_COMPLETE_RELOAD )
			{
				m_bReloadVisuallyComplete = true;
				CCSPlayer *pPlayer = GetPlayerOwner();

				if ( pPlayer )
				{
					int j = MIN( GetMaxClip1() - m_iClip1, GetReserveAmmoCount( AMMO_POSITION_PRIMARY ) );

					// Add them to the clip
					m_iClip1 += j;
					GiveReserveAmmo( AMMO_POSITION_PRIMARY, -j, true );
				}
			}

		else if ( nEvent == AE_BEGIN_TAUNT_LOOP )
		{
			CCSPlayer *pPlayer = GetPlayerOwner();

			if ( pPlayer && pPlayer->IsLookingAtWeapon() && pPlayer->IsHoldingLookAtWeapon() )
				{
					CBaseViewModel *pViewModel = pPlayer->GetViewModel();

					float flPrevCycle = pViewModel->GetCycle();
					float flNewCycle = V_atof( pEvent->options );
					pViewModel->SetCycle( flNewCycle );

					float flSequenceDuration = pViewModel->SequenceDuration( pViewModel->GetSequence() );
					pPlayer->ModifyTauntDuration( (flNewCycle - flPrevCycle) * flSequenceDuration );
				}

			return;

		}
		//update the bullet bodygroup on the client
			else if ( nEvent == AE_CL_BODYGROUP_SET_TO_CLIP )
			{
				CBasePlayer *pOwner = ToBasePlayer( GetPlayerOwner() );
				if ( pOwner )
				{
					CBaseViewModel *vm = pOwner->GetViewModel( m_nViewModelIndex );
					if ( vm )
					{
						int iNumBodygroupIndices = vm->GetNumBodyGroups();

						for ( int iGroup=1; iGroup<iNumBodygroupIndices; iGroup++ )
						{
							vm->SetBodygroup( iGroup, (m_iClip1 >= iGroup) ? 0 : 1 );
						}
					}
				}
			}
			else if ( nEvent == AE_CL_BODYGROUP_SET_TO_NEXTCLIP )
			{
				CBasePlayer *pOwner = ToBasePlayer( GetPlayerOwner() );
				if ( pOwner )
				{
					CBaseViewModel *vm = pOwner->GetViewModel( m_nViewModelIndex );
					if ( vm )
					{
						int iNextClip = MIN( GetMaxClip1(), m_iClip1 + GetReserveAmmoCount( AMMO_POSITION_PRIMARY ) );
						int iNumBodygroupIndices = vm->GetNumBodyGroups();
						for ( int iGroup=1; iGroup<iNumBodygroupIndices; iGroup++ )
						{
							vm->SetBodygroup( iGroup, (iNextClip >= iGroup) ? 0 : 1 );
						}
					}
				}
			}
			else if ( nEvent == AE_WPN_NEXTCLIP_TO_POSEPARAM )
			{
				// sets the given pose param to a 0..1 value representing the clip amount after an impending reload
				CBasePlayer *pOwner = ToBasePlayer( GetPlayerOwner() );
				if ( pOwner )
				{
					CBaseViewModel *vm = pOwner->GetViewModel( m_nViewModelIndex );
					if ( vm )
					{
						int iNextClip = MIN( GetMaxClip1(), m_iClip1 + GetReserveAmmoCount( AMMO_POSITION_PRIMARY ) );
						vm->SetPoseParameter( pEvent->options, 1.0f - (((float) iNextClip) / ((float) GetMaxClip1())) );
					}
				}
			}
			else if ( nEvent == AE_WPN_CLIP_TO_POSEPARAM )
			{
				// sets the given pose param to a 0..1 value representing the clip amount
				CBasePlayer *pOwner = ToBasePlayer( GetPlayerOwner() );
				if ( pOwner )
				{
					CBaseViewModel *vm = pOwner->GetViewModel( m_nViewModelIndex );
					if ( vm )
					{
						vm->SetPoseParameter( pEvent->options, 1.0f - (((float) m_iClip1) / ((float) GetMaxClip1())) );
					}
				}
			}
			else if ( nEvent == AE_CL_EJECT_MAG )
			{
				SetBodygroup( FindBodygroupByName( "magazine" ), 1 );
			}
			else if ( nEvent == AE_CL_EJECT_MAG_UNHIDE )
			{
				SetBodygroup( FindBodygroupByName( "magazine" ), 0 );
			}
			else if ( nEvent == AE_WPN_PRIMARYATTACK )
			{
				// delay time
				const char *p = pEvent->options;
				char token[256];
				p = nexttoken( token, p, ' ' );
				if ( token )
				{
					SetPostponeFireReadyTime( gpGlobals->curtime + atof( token ) );

					// play the prepare warning sound to other players
					CCSPlayer *pCSPlayer = GetPlayerOwner();
					if ( pCSPlayer )
					{
						Vector soundPosition = pCSPlayer->GetAbsOrigin() + Vector( 0, 0, 50 );
						CPASAttenuationFilter filter( soundPosition );
						filter.RemoveRecipient( pCSPlayer );

						// remove anyone that is first person spectating this player, since the viewmodel should play this sound for them on their client.
						int i;
						CBasePlayer *pPlayer;
						for ( i = 1; i <= gpGlobals->maxClients; i++ )
						{
							pPlayer = UTIL_PlayerByIndex( i );

							if ( !pPlayer )
								continue;

							if ( pPlayer->GetObserverMode() == OBS_MODE_IN_EYE && pPlayer->GetObserverTarget() == pCSPlayer )
							{
								filter.RemoveRecipient( pPlayer );
							}
						}

						EmitSound( filter, entindex(), "Weapon_Revolver.Prepare" );
					}

				}

				return;
			}
			else if ( nEvent == AE_CL_BODYGROUP_SET_VALUE )
			{
				CCSPlayer *pCSPlayer = GetPlayerOwner();
				if ( pCSPlayer && pCSPlayer->GetActiveCSWeapon() )
				{
					if ( CBaseViewModel *vm = pCSPlayer->GetViewModel( m_nViewModelIndex ) )
					{
						char szBodygroupName[256];
						int value = 0;

						char token[256];

						const char *p = pEvent->options;

						// Bodygroup Name
						p = nexttoken( token, p, ' ' );
						if ( token )
						{
							Q_strncpy( szBodygroupName, token, sizeof( szBodygroupName ) );
						}

						// Get the desired value
						p = nexttoken( token, p, ' ' );
						if ( token )
						{
							value = atoi( token );
						}

						int index = vm->FindBodygroupByName( szBodygroupName );
						if ( index >= 0 )
						{
							vm->SetBodygroup( index, value );
						}
					}
				}
				return;
			}
		}
	BaseClass::Operator_HandleAnimEvent( pEvent, pOperator );
}

	bool CWeaponCSBase::Reload()
	{
		m_bReloadVisuallyComplete = false;

		CCSPlayer *pPlayer = GetPlayerOwner();
		if ( !pPlayer )
			return false;
		
		pPlayer->StopLookingAtWeapon();
		pPlayer->m_iShotsFired = 0;

		bool retval = BaseClass::Reload();

		return retval;
	}

	void CWeaponCSBase::Spawn()
	{
		BaseClass::Spawn();

		// Override the bloat that our base class sets as it's a little bit bigger than we want.
		// If it's too big, you drop a weapon and its box is so big that you're still touching it
		// when it falls and you pick it up again right away.
		CollisionProp()->UseTriggerBounds( true, 30 );

		// Set this here to allow players to shoot dropped weapons
		SetCollisionGroup( COLLISION_GROUP_WEAPON );

		SetExtraAmmoCount( m_iDefaultExtraAmmo );	//Start with no additional ammo

		m_nextPrevOwnerTouchTime = 0.0;
		m_prevOwner = NULL;

        //=============================================================================
        // HPE_BEGIN:
        //=============================================================================

        // [tj] initialize donor of this weapon
        m_donor = NULL;
        m_donated = false;

		m_weaponMode = Primary_Mode;

        //=============================================================================
        // HPE_END
        //=============================================================================

#if IRONSIGHT
		UpdateIronSightController();
#endif //IRONSIGHT	
	}

	bool CWeaponCSBase::DefaultReload( int iClipSize1, int iClipSize2, int iActivity )
	{
		if ( BaseClass::DefaultReload( iClipSize1, iClipSize2, iActivity ) )
		{
			SendReloadEvents();
			return true;
		}
		else
		{
			return false;
		}
	}

	void CWeaponCSBase::SendReloadEvents()
	{
		CCSPlayer *pPlayer = dynamic_cast< CCSPlayer* >( GetOwner() );
		if ( !pPlayer )
			return;

		// Send a message to any clients that have this entity to play the reload.
		CPASFilter filter( pPlayer->GetAbsOrigin() );
		filter.RemoveRecipient( pPlayer );

		UserMessageBegin( filter, "ReloadEffect" );
			WRITE_SHORT( pPlayer->entindex() );
		MessageEnd();

		// Make the player play his reload animation.
		pPlayer->DoAnimationEvent( PLAYERANIMEVENT_RELOAD );
	}

#endif


bool CWeaponCSBase::DefaultPistolReload()
{
	CCSPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return false;

	if ( GetReserveAmmoCount( AMMO_POSITION_PRIMARY ) <= 0 )
		return true;

	if ( !DefaultReload( GetCSWpnData().iDefaultClip1, 0, ACT_VM_RELOAD ) )
		return false;

	pPlayer->m_iShotsFired = 0;

	return true;
}

bool CWeaponCSBase::IsUseable()
{
	CCSPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return false;

	if ( Clip1() <= 0 )
	{
		if ( GetReserveAmmoCount( AMMO_POSITION_PRIMARY ) <= 0 && GetMaxClip1() != -1 )
		{
			// clip is empty (or nonexistant) and the player has no more ammo of this type.
			return false;
		}
	}

	return true;
}


#if defined( CLIENT_DLL )

float	g_lateralBob = 0;
float	g_verticalBob = 0;

static ConVar	cl_bob_version( "cl_bob_version", "0", FCVAR_CHEAT );
static ConVar	cl_bobcycle( "cl_bobcycle", "0.98", FCVAR_CHEAT, "the frequency at which the viewmodel bobs.", true, 0.1, true, 2.0 );
static ConVar	cl_bobup( "cl_bobup", "0.5", FCVAR_CHEAT );

ConVar	cl_use_new_headbob( "cl_use_new_headbob", "1", FCVAR_ARCHIVE, "What viewbob style to use: CS:S (0) or CS:GO (1)." );
static ConVar	cl_bobamt_vert( "cl_bobamt_vert", "0.25", FCVAR_ARCHIVE, "The amount the viewmodel moves up and down when running", true, 0.1, true, 2 );
static ConVar	cl_bobamt_lat( "cl_bobamt_lat", "0.4", FCVAR_ARCHIVE, "The amount the viewmodel moves side to side when running", true, 0.1, true, 2 );
static ConVar	cl_bob_lower_amt( "cl_bob_lower_amt", "21", FCVAR_ARCHIVE, "The amount the viewmodel lowers when running", true, 5, true, 30 );

static ConVar	cl_viewmodel_shift_left_amt( "cl_viewmodel_shift_left_amt", "1.5", FCVAR_ARCHIVE, "The amount the viewmodel shifts to the left when shooting accuracy increases.", true, 0.5, true, 2.0 );
static ConVar	cl_viewmodel_shift_right_amt( "cl_viewmodel_shift_right_amt", "0.75", FCVAR_ARCHIVE, "The amount the viewmodel shifts to the right when shooting accuracy decreases.", true, 0.25, true, 2.0 );

//-----------------------------------------------------------------------------
// Purpose: Helper function to calculate head bob
//-----------------------------------------------------------------------------
float CalcNewViewModelBobbing( CBasePlayer *player, BobState_t *pBobState, int nVMIndex )
{
	if ( cl_use_new_headbob.GetBool() == false )
		return 0;

	Assert( pBobState );
	if ( !pBobState )
		return 0;

	float	cycle;

	//NOTENOTE: For now, let this cycle continue when in the air, because it snaps badly without it

	if ( (!gpGlobals->frametime) || (player == NULL) )
	{
		//NOTENOTE: We don't use this return value in our case (need to restructure the calculation function setup!)
		return 0.0f;// just use old value
	}

	//Find the speed of the player
	float speed = player->GetLocalVelocity().Length2D();

	// do the dip before speed gets clamped
	float flSpeedFactor;

	float flRunAddAmt = 0.0f;

	float flmaxSpeedDelta = MAX( 0, (gpGlobals->curtime - pBobState->m_flLastBobTime) * 640.0f );//320

	//	Msg( "speed raw = %f\n", speed );

	// don't allow too big speed changes
	speed = clamp( speed, pBobState->m_flLastSpeed - flmaxSpeedDelta, pBobState->m_flLastSpeed + flmaxSpeedDelta );
	speed = clamp( speed, -320.0f, 320.0f );

	pBobState->m_flLastSpeed = speed;

	C_BaseViewModel *pViewModel = player ? player->GetViewModel( nVMIndex ) : NULL;
	bool bShouldIgnoreOffsetAndAccuracy = (pViewModel && pViewModel->m_bShouldIgnoreOffsetAndAccuracy);

	// when the player is moving forward, the gun lowers a bit -mtw
	C_CSPlayer *pPlayer = ToCSPlayer( player );
	if ( pPlayer && !pPlayer->IsZoomed() )
	{
		flSpeedFactor = speed * 0.006f;
		clamp( flSpeedFactor, 0.0f, 0.5f );
		float flLowerAmt = cl_bob_lower_amt.GetFloat() * 0.2;

		// HACK until we get 'ACT_VM_IDLE_LOWERED' anims
		//if ( pPlayer->m_Shared.IsSprinting() )
		//flLowerAmt *= 10.0f;

		if ( bShouldIgnoreOffsetAndAccuracy )
			flLowerAmt *= 0.1;

		flRunAddAmt = (flLowerAmt * flSpeedFactor);
	}

	//FIXME: This maximum speed value must come from the server.
	//		 MaxSpeed() is not sufficient for dealing with sprinting - jdw

	float bob_offset = RemapVal( speed, 0.0f, 320.0f, 0.0f, 1.0f );

	pBobState->m_flBobTime += (gpGlobals->curtime - pBobState->m_flLastBobTime) * bob_offset;
	pBobState->m_flLastBobTime = gpGlobals->curtime;

	//Msg( "speed = %f\n", speed );

	float flBobCycle = 0.5f;
	float flAccuracyDiff = 0;
	float flGunAccPos = 0;


	CWeaponCSBase *pWeapon = ((bShouldIgnoreOffsetAndAccuracy || !pPlayer) ? NULL : pPlayer->GetActiveCSWeapon());
	if ( pPlayer && pWeapon )
	{
		float flMaxSpeed = pWeapon->GetMaxSpeed();
		flBobCycle = (((1000.0f - flMaxSpeed) / 3.5f) * 0.001f) * cl_bobcycle.GetFloat();

		float flAccuracy = 0.0f;

		bool bIsElites = (pWeapon->GetWeaponID() == WEAPON_ELITE) ? true : false;

		// don't move the gun around based on accuracy when reloading
		if ( !pWeapon->m_bInReload && !bIsElites )
		{
			// move the gun left or right based on the players current accuracy
			float flCrouchAccuracy = pWeapon->GetCSWpnData().m_fInaccuracyCrouch[pWeapon->m_weaponMode];
			float flBaseAccuracy = pWeapon->GetCSWpnData().m_fInaccuracyStand[pWeapon->m_weaponMode];
			if ( pPlayer->m_Local.m_bDucking || pPlayer->m_Local.m_bDucked )
				flAccuracy = flCrouchAccuracy;
			else
				flAccuracy = pWeapon->GetInaccuracy();
				bool bIsSniper = (pWeapon->GetCSWpnData().m_WeaponType != WEAPONTYPE_SNIPER_RIFLE) ? true : false;


			float flMultiplier = 1;
			if ( flAccuracy < flBaseAccuracy )
			{
				if ( !bIsSniper )
					flMultiplier = 18;
				else
					flMultiplier = 0.15;

				flMultiplier *= cl_viewmodel_shift_left_amt.GetFloat();
			}
			else
			{
				flAccuracy = MIN( flAccuracy, 0.082f );
				flMultiplier *= cl_viewmodel_shift_right_amt.GetFloat();
			}

			// clamp the crouch accuracy
			flAccuracyDiff = MAX( (flAccuracy - flBaseAccuracy) * flMultiplier, -0.1 );
		}

		/*
		if ( pWeapon->m_flGunAccuracyPosition > 0.0f )
		Msg( "m_flGunAccuracyPosition1 = %f, flAccuracy = %f\n", pWeapon->m_flGunAccuracyPosition, flAccuracy );
		pWeapon->m_flGunAccuracyPosition = Lerp( ( gpGlobals->frametime / 0.02f ), (flAccuracyDiff*20), pWeapon->m_flGunAccuracyPosition );
		if ( pWeapon->m_flGunAccuracyPosition > 0.0f )
		Msg( "m_flGunAccuracyPosition1 = %f, flAccuracy = %f\n", pWeapon->m_flGunAccuracyPosition, flAccuracy );
		*/

	}
	else
	{
		flBobCycle = (((1000.0f - 150) / 3.5f) * 0.001f) * cl_bobcycle.GetFloat();
	}

	//Msg( "flBobCycle = %f\n", flBobCycle );
	//Calculate the vertical bob
	cycle = pBobState->m_flBobTime - (int)(pBobState->m_flBobTime / flBobCycle)*flBobCycle;
	cycle /= flBobCycle;

	if ( cycle < cl_bobup.GetFloat() )
	{
		cycle = M_PI * cycle / cl_bobup.GetFloat();
	}
	else
	{
		cycle = M_PI + M_PI*(cycle - cl_bobup.GetFloat()) / (1.0 - cl_bobup.GetFloat());
	}
	float flBobMultiplier = 0.00625f;
	// if we're in the air, slow our bob down a bit
	if ( player->GetGroundEntity() == NULL )
		flBobMultiplier = 0.00125f;

	float flBobVert = bShouldIgnoreOffsetAndAccuracy ? 0.3 : cl_bobamt_vert.GetFloat();
	pBobState->m_flVerticalBob = speed*(flBobMultiplier * flBobVert);
	pBobState->m_flVerticalBob = (pBobState->m_flVerticalBob*0.3 + pBobState->m_flVerticalBob*0.7*sin( cycle ));
	pBobState->m_flRawVerticalBob = pBobState->m_flVerticalBob;

	pBobState->m_flVerticalBob = clamp( (pBobState->m_flVerticalBob - (flRunAddAmt - (flGunAccPos*0.2))), -7.0f, 4.0f );

	//Msg( "pBobState->m_flVerticalBob = %f, cycle = %f, pBobState->m_flBobTime = %f\n", pBobState->m_flVerticalBob, cycle, pBobState->m_flBobTime );

	//Calculate the lateral bob
	cycle = pBobState->m_flBobTime - (int)(pBobState->m_flBobTime / flBobCycle * 2)*flBobCycle * 2;
	cycle /= flBobCycle * 2;

	if ( cycle < cl_bobup.GetFloat() )
	{
		cycle = M_PI * cycle / cl_bobup.GetFloat();
	}
	else
	{
		cycle = M_PI + M_PI*(cycle - cl_bobup.GetFloat()) / (1.0 - cl_bobup.GetFloat());
	}

	float flBobLat = bShouldIgnoreOffsetAndAccuracy ? 0.5 : cl_bobamt_lat.GetFloat();
	if ( pPlayer && pWeapon )
	{
		pBobState->m_flLateralBob = speed*(flBobMultiplier * flBobLat);
		pBobState->m_flLateralBob = pBobState->m_flLateralBob*0.3 + pBobState->m_flLateralBob*0.7*sin( cycle );
		pBobState->m_flRawLateralBob = pBobState->m_flLateralBob;

		pBobState->m_flLateralBob = clamp( (pBobState->m_flLateralBob + flGunAccPos*0.25), -8.0f, 8.0f );
		//Msg( "flAccuracyDiff = %f\n", flAccuracyDiff );
	}

	//NOTENOTE: We don't use this return value in our case (need to restructure the calculation function setup!)
	return 0.0f;
}

//-----------------------------------------------------------------------------
// Purpose: Helper function to add head bob
//-----------------------------------------------------------------------------
void AddNewViewModelBobbing( Vector &origin, QAngle &angles, BobState_t *pBobState )
{
	if ( cl_use_new_headbob.GetBool() == false )
		return;

	Assert( pBobState );
	if ( !pBobState )
		return;

	Vector	forward, right;
	AngleVectors( angles, &forward, &right, NULL );

	// Apply bob, but scaled down to 40%
	VectorMA( origin, pBobState->m_flVerticalBob * 0.4f, forward, origin );

	// Z bob a bit more
	origin[2] += pBobState->m_flVerticalBob * 0.1f;

	// bob the angles
	angles[ROLL] += pBobState->m_flVerticalBob * 0.5f;
	angles[PITCH] -= pBobState->m_flVerticalBob * 0.4f;
	angles[YAW] -= pBobState->m_flLateralBob  * 0.3f;

	VectorMA( origin, pBobState->m_flLateralBob * 0.2f, right, origin );
}
	//-----------------------------------------------------------------------------
	// Purpose:
	// Output : float
	//-----------------------------------------------------------------------------
	float CWeaponCSBase::CalcViewmodelBob( void )
	{
		if ( cl_use_new_headbob.GetBool() == true )
		{
			CBasePlayer *player = ToBasePlayer( GetOwner() );
			//Assert( player );
			BobState_t *pBobState = GetBobState();
			if ( pBobState )
			{
				return ::CalcNewViewModelBobbing( player, pBobState );
			}
			else
				return 0;
		}

		static	float bobtime;
		static	float lastbobtime;
		static  float lastspeed;
		float	cycle;

		CBasePlayer *player = ToBasePlayer( GetOwner() );
		//Assert( player );

		//NOTENOTE: For now, let this cycle continue when in the air, because it snaps badly without it

		if ( ( !gpGlobals->frametime ) ||
			 ( player == NULL ) ||
			 ( cl_bobcycle.GetFloat() <= 0.0f ) ||
			 ( cl_bobup.GetFloat() <= 0.0f ) ||
			 ( cl_bobup.GetFloat() >= 1.0f ) )
		{
			//NOTENOTE: We don't use this return value in our case (need to restructure the calculation function setup!)
			return 0.0f;// just use old value
		}

		//Find the speed of the player
		float speed = player->GetLocalVelocity().Length2D();
		float flmaxSpeedDelta = MAX( 0, (gpGlobals->curtime - lastbobtime) * 320.0f );

		// don't allow too big speed changes
		speed = clamp( speed, lastspeed-flmaxSpeedDelta, lastspeed+flmaxSpeedDelta );
		speed = clamp( speed, -320, 320 );

		lastspeed = speed;

		//FIXME: This maximum speed value must come from the server.
		//		 MaxSpeed() is not sufficient for dealing with sprinting - jdw



		float bob_offset = RemapVal( speed, 0, 320, 0.0f, 1.0f );

		bobtime += ( gpGlobals->curtime - lastbobtime ) * bob_offset;
		lastbobtime = gpGlobals->curtime;

		//Calculate the vertical bob
		cycle = bobtime - (int)(bobtime/cl_bobcycle.GetFloat())*cl_bobcycle.GetFloat();
		cycle /= cl_bobcycle.GetFloat();

		if ( cycle < cl_bobup.GetFloat() )
		{
			cycle = M_PI * cycle / cl_bobup.GetFloat();
		}
		else
		{
			cycle = M_PI + M_PI*(cycle-cl_bobup.GetFloat())/(1.0 - cl_bobup.GetFloat());
		}

		g_verticalBob = speed*0.005f;
		g_verticalBob = g_verticalBob*0.3 + g_verticalBob*0.7*sin(cycle);

		g_verticalBob = clamp( g_verticalBob, -7.0f, 4.0f );

		//Calculate the lateral bob
		cycle = bobtime - (int)(bobtime/cl_bobcycle.GetFloat()*2)*cl_bobcycle.GetFloat()*2;
		cycle /= cl_bobcycle.GetFloat()*2;

		if ( cycle < cl_bobup.GetFloat() )
		{
			cycle = M_PI * cycle / cl_bobup.GetFloat();
		}
		else
		{
			cycle = M_PI + M_PI*(cycle-cl_bobup.GetFloat())/(1.0 - cl_bobup.GetFloat());
		}

		g_lateralBob = speed*0.005f;
		g_lateralBob = g_lateralBob*0.3 + g_lateralBob*0.7*sin(cycle);
		g_lateralBob = clamp( g_lateralBob, -7.0f, 4.0f );

		//NOTENOTE: We don't use this return value in our case (need to restructure the calculation function setup!)
		return 0.0f;

	}

	//-----------------------------------------------------------------------------
	// Purpose:
	// Input  : &origin -
	//			&angles -
	//			viewmodelindex -
	//-----------------------------------------------------------------------------
	void CWeaponCSBase::AddViewmodelBob( CBaseViewModel *viewmodel, Vector &origin, QAngle &angles )
	{
		if ( cl_use_new_headbob.GetBool() == true )
		{
			// call helper functions to do the calculation
			BobState_t *pBobState = GetBobState();
			if ( pBobState )
			{
				CalcViewmodelBob();
				::AddNewViewModelBobbing( origin, angles, pBobState );
			}
			return;
		}

		Vector	forward, right;
		AngleVectors( angles, &forward, &right, NULL );

		CalcViewmodelBob();

		// Apply bob, but scaled down to 40%
		VectorMA( origin, g_verticalBob * 0.4f, forward, origin );

		// Z bob a bit more
		origin[2] += g_verticalBob * 0.1f;

		// bob the angles
		angles[ ROLL ]	+= g_verticalBob * 0.5f;
		angles[ PITCH ]	-= g_verticalBob * 0.4f;

		angles[ YAW ]	-= g_lateralBob  * 0.3f;

	//	VectorMA( origin, g_lateralBob * 0.2f, right, origin );
	}

	//-----------------------------------------------------------------------------
	// Purpose: Returns the head bob state for this weapon, which is stored
	//			in the view model.  Note that this this function can return
	//			NULL if the player is dead or the view model is otherwise not present.
	//-----------------------------------------------------------------------------
	BobState_t *CWeaponCSBase::GetBobState()
	{
		// get the view model for this weapon
		CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
		if ( pOwner == NULL )
			return NULL;
		CBaseViewModel *baseViewModel = pOwner->GetViewModel( m_nViewModelIndex );
		if ( baseViewModel == NULL )
			return NULL;
		CPredictedViewModel *viewModel = dynamic_cast<CPredictedViewModel *>(baseViewModel);
		if ( viewModel == NULL )
			return NULL;

		//Assert( viewModel );

		// get the bob state out of the view model
		return &(viewModel->GetBobState());
	}

#else

	void CWeaponCSBase::AddViewmodelBob( CBaseViewModel *viewmodel, Vector &origin, QAngle &angles )
	{

	}

	float CWeaponCSBase::CalcViewmodelBob( void )
	{
		return 0.0f;
	}

#endif

#ifndef CLIENT_DLL
bool CWeaponCSBase::PhysicsSplash( const Vector &centerPoint, const Vector &normal, float rawSpeed, float scaledSpeed )
{
	if ( rawSpeed > 20 )
	{

		float size = 4.0f;
		if ( !IsPistol() )
			size += 2.0f;

		// adjust splash size based on speed
		size += RemapValClamped( rawSpeed, 0, 400, 0, 3 );

		CEffectData	data;
 		data.m_vOrigin = centerPoint;
		data.m_vNormal = normal;
		data.m_flScale = random->RandomFloat( size, size + 1.0f );

		if ( GetWaterType() & CONTENTS_SLIME )
		{
			data.m_fFlags |= FX_WATER_IN_SLIME;
		}

		DispatchEffect( "gunshotsplash", data );

		return true;
	}

	return false;
}
#endif // !CLIENT_DLL

//-----------------------------------------------------------------------------
// Purpose:
// Input  : *pPicker -
//-----------------------------------------------------------------------------
void CWeaponCSBase::OnPickedUp( CBaseCombatCharacter *pNewOwner )
{
#if !defined( CLIENT_DLL )
	RemoveEffects( EF_ITEM_BLINK );

	if( pNewOwner->IsPlayer() && pNewOwner->IsAlive() )
	{
		m_OnPlayerPickup.FireOutput(pNewOwner, this);

		// Play the pickup sound for 1st-person observers
		CRecipientFilter filter;
		for ( int i=0; i<gpGlobals->maxClients; ++i )
		{
			CBasePlayer *player = UTIL_PlayerByIndex(i);
			if ( player && !player->IsAlive() && player->GetObserverMode() == OBS_MODE_IN_EYE )
			{
				filter.AddRecipient( player );
			}
		}
		if ( filter.GetRecipientCount() )
		{
			CBaseEntity::EmitSound( filter, pNewOwner->entindex(), "Player.PickupWeapon" );
		}

		// Robin: We don't want to delete weapons the player has picked up, so
		// clear the name of the weapon. This prevents wildcards that are meant
		// to find NPCs finding weapons dropped by the NPCs as well.
		SetName( NULL_STRING );
	}

	// Someone picked me up, so make it so that I can't be removed.
	SetRemoveable( false );
#endif

#if IRONSIGHT
	UpdateIronSightController();
#endif //IRONSIGHT
}


void CWeaponCSBase::UpdateAccuracyPenalty()
{
	CCSPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

	const CCSWeaponInfo& weaponInfo = GetCSWpnData();

	float fNewPenalty = 0.0f;

	// on ladder?
	if ( pPlayer->GetMoveType() == MOVETYPE_LADDER )
	{
		fNewPenalty += weaponInfo.m_fInaccuracyStand[m_weaponMode] + weaponInfo.m_fInaccuracyLadder[m_weaponMode];
	}
	// in the air?
// 	else if ( !FBitSet( pPlayer->GetFlags(), FL_ONGROUND ) )
// 	{
// 		fNewPenalty += weaponInfo.m_fInaccuracyStand[m_weaponMode] + weaponInfo.m_fInaccuracyJump[m_weaponMode];
// 	}
	else if ( FBitSet( pPlayer->GetFlags(), FL_DUCKING) )
	{
		fNewPenalty += weaponInfo.m_fInaccuracyCrouch[m_weaponMode];
	}
	else
	{
		fNewPenalty += weaponInfo.m_fInaccuracyStand[m_weaponMode];
	}

	if ( m_bInReload )
	{
 		fNewPenalty += weaponInfo.m_fInaccuracyReload;
	}

	if ( fNewPenalty > m_fAccuracyPenalty )
	{
		m_fAccuracyPenalty = fNewPenalty;
	}
	else
	{
		float fDecayFactor = logf( 10.0f ) / GetRecoveryTime( );

		m_fAccuracyPenalty = Lerp( expf( TICK_INTERVAL * -fDecayFactor ), fNewPenalty, ( float ) m_fAccuracyPenalty );
	}

#define WEAPON_RECOIL_DECAY_THRESHOLD 1.10	
	// Decay the recoil index if a little more than cycle time has elapsed since the last shot. In other words,
	// don't decay if we're firing full-auto.
	if ( gpGlobals->curtime > m_fLastShotTime + ( GetCSWpnData().m_flCycleTime * WEAPON_RECOIL_DECAY_THRESHOLD ) )
	{
		float fDecayFactor = logf( 10.0f ) * weapon_recoil_decay_coefficient.GetFloat( );

		m_flRecoilIndex = Lerp( expf( TICK_INTERVAL * -fDecayFactor ), 0.0f, ( float ) m_flRecoilIndex );
	}
}

float CWeaponCSBase::GetRecoveryTime( void )
{
	CCSPlayer *pPlayer = GetPlayerOwner( );
	if ( !pPlayer )
		return -1.0f;

	const CCSWeaponInfo& weaponInfo = GetCSWpnData();

	if ( pPlayer->GetMoveType( ) == MOVETYPE_LADDER )
	{
		return weaponInfo.m_fRecoveryTimeStand;
	}
	else if ( !FBitSet( pPlayer->GetFlags( ), FL_ONGROUND ) )	// in air
	{
		// enforce a large recovery speed penalty (400%) for players in the air; this helps to provide
		// comparable in-air accuracy to the old weapon model

		return weaponInfo.m_fRecoveryTimeCrouch * 4.0f;
	}
	else if ( FBitSet( pPlayer->GetFlags( ), FL_DUCKING ) )
	{
		float flRecoveryTime = weaponInfo.m_fRecoveryTimeCrouch;
		float flRecoveryTimeFinal = weaponInfo.m_fRecoveryTimeCrouchFinal;

		if ( flRecoveryTimeFinal != -1.0f )	// uninitialized final recovery values are set to -1.0 from the weapon_base prefab in schema
		{
			int nRecoilIndex = m_flRecoilIndex;

			flRecoveryTime = RemapValClamped( nRecoilIndex, weaponInfo.m_iRecoveryTransitionStartBullet, weaponInfo.m_iRecoveryTransitionEndBullet, flRecoveryTime, flRecoveryTimeFinal );
		}
		
		return flRecoveryTime;
	}
	else
	{
		float flRecoveryTime = weaponInfo.m_fRecoveryTimeStand;
		float flRecoveryTimeFinal = weaponInfo.m_fRecoveryTimeStandFinal;

		if ( flRecoveryTimeFinal != -1.0f )	// uninitialized final recovery values are set to -1.0 from the weapon_base prefab in schema
		{
			int nRecoilIndex = m_flRecoilIndex;

			flRecoveryTime = RemapValClamped( nRecoilIndex, weaponInfo.m_iRecoveryTransitionStartBullet, weaponInfo.m_iRecoveryTransitionEndBullet, flRecoveryTime, flRecoveryTimeFinal );
		}
		return flRecoveryTime;
	}
}


const float kJumpVelocity = sqrtf(2.0f * 800.0f * 57.0f);	// see CCSGameMovement::CheckJumpButton()

void CWeaponCSBase::OnJump( float fImpulse )
{
	m_fAccuracyPenalty += GetCSWpnData().m_fInaccuracyJump[m_weaponMode] * fImpulse / kJumpVelocity;
}

void CWeaponCSBase::OnLand( float fVelocity )
{
	float fPenalty = GetCSWpnData().m_fInaccuracyLand[m_weaponMode] * fVelocity / kJumpVelocity;
	m_fAccuracyPenalty += fPenalty;

	// this bit of code is only if we want to punch the player view on all landings

	CCSPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

	QAngle angle = pPlayer->GetRawAimPunchAngle();
	float fVKick = RAD2DEG(asinf(fPenalty)) * 0.2f;
	float fHKick = SharedRandomFloat("LandPunchAngleYaw", -1.0f, +1.0f) * fVKick * 0.1f;

	angle.x += fVKick;	// pitch
	angle.y += fHKick;	// yaw
	pPlayer->SetAimPunchAngle( angle );
}

void CWeaponCSBase::Recoil( CSWeaponMode weaponMode )
{
	// PiMoN: huge thanks to https://github.com/SwagSoftware/Kisak-Strike/!
	//lwss - rebuilt this function from reversing retail bins
	CCSPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

	int index;
	if ( IsFullAuto() ) // PiMoN TODO: what should be here?
		index = m_flRecoilIndex;
	else
		index = GetPredictionRandomSeed();

	float angle;
	float magnitude;
	g_WeaponRecoilData.GetRecoilOffsets( this, weaponMode, index, angle, magnitude );

	pPlayer->KickBack( angle, magnitude );
	//lwss end
}

#if IRONSIGHT
CIronSightController *CWeaponCSBase::GetIronSightController( void )
{
	if ( m_IronSightController && m_IronSightController->IsInitializedAndAvailable() )
	{
		return m_IronSightController;
	}
	return NULL;
}

void CWeaponCSBase::UpdateIronSightController()
{
	if (!m_IronSightController)
		m_IronSightController = new CIronSightController();
	
	if (m_IronSightController)
		m_IronSightController->Init(this);
}
#endif
