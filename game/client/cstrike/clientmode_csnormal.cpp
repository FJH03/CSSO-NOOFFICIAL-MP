//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//===========================================================================//
#include "cbase.h"
#include "hud.h"
#include "clientmode_csnormal.h"
#include "cdll_client_int.h"
#include <engine/ivdebugoverlay.h>
#include "iinput.h"
#include "vgui/ISurface.h"
#include "vgui/IPanel.h"
#include <vgui_controls/AnimationController.h>
#include "ivmodemanager.h"
#include "buymenu.h"
#include "filesystem.h"
#include "vgui/IVGui.h"
#include "hud_basechat.h"
#include "view_shared.h"
#include "view.h"
#include "ivrenderview.h"
#include "cstrikeclassmenu.h"
#include "cstrikebuymenu.h"
#include "c_te_legacytempents.h"
#include "tempent.h"
#include "model_types.h"
#include "iefx.h"
#include "dlight.h"
#include <imapoverview.h>
#include "c_playerresource.h"
#include "c_soundscape.h"
#include <KeyValues.h>
#include "text_message.h"
#include "panelmetaclassmgr.h"
#include "vguicenterprint.h"
#include "physpropclientside.h"
#include "c_weapon__stubs.h"
#include <engine/IEngineSound.h>
#include "c_cs_hostage.h"
#include "bitbuf.h"
#include "usermessages.h"
#include "prediction.h"
#include "datacache/imdlcache.h"
#include "cs_shareddefs.h"
#include "cs_loadout.h"
#include "materialsystem/itexture.h"
#include "viewpostprocess.h"
#include "cstrikeclientscoreboard.h"
#include "cam_thirdperson.h"

// [tj] Needed to retrieve achievement text
// [menglish] Need access to message macros 
#include "achievementmgr.h"
#include "hud_macros.h"
#include "c_plantedc4.h"
#include "tier1/fmtstr.h"
#include "cs_client_gamestats.h"
#include "c_cs_playerresource.h"


void __MsgFunc_MatchEndConditions( bf_read &msg );

class CHudChat;

ConVar cl_draw_only_deathnotices( "cl_draw_only_deathnotices", "0", FCVAR_CHEAT, "For drawing only the crosshair and death notices (used for moviemaking)" );

ConVar default_fov( "default_fov", "90", FCVAR_CHEAT );

IClientMode *g_pClientMode = NULL;

STUB_WEAPON_CLASS( cycler_weapon,	WeaponCycler,	C_BaseCombatWeapon );
STUB_WEAPON_CLASS( weapon_cubemap,	WeaponCubemap,	C_BaseCombatWeapon );

//-----------------------------------------------------------------------------
// HACK: the detail sway convars are archive, and default to 0.  Existing CS:S players thus have no detail
// prop sway.  We'll force them to DoD's default values for now.  What we really need in the long run is
// a system to apply changes to archived convars' defaults to existing players.
extern ConVar cl_detail_max_sway;
extern ConVar cl_detail_avoid_radius;
extern ConVar cl_detail_avoid_force;
extern ConVar cl_detail_avoid_recover_speed;
extern ConVar v_viewmodel_fov;
extern ConVar view_recoil_tracking;
extern ConVar cam_recoil;

ConVar mat_blur_strength( "mat_blur_strength", "1.0", FCVAR_ARCHIVE );
ConVar mat_blur_desaturate( "mat_blur_desaturate", "0.5", FCVAR_ARCHIVE );

//-----------------------------------------------------------------------------
ConVar cl_autobuy(
	"cl_autobuy",
	"",
	FCVAR_USERINFO,
	"The order in which autobuy will attempt to purchase items" );

//-----------------------------------------------------------------------------
ConVar cl_rebuy(
	"cl_rebuy",
	"",
	FCVAR_USERINFO,
	"The order in which rebuy will attempt to repurchase items" );

//-----------------------------------------------------------------------------
void SetBuyData( const ConVar &buyVar, const char *filename )
{
	// if we already have autobuy data, don't bother re-parsing the text file
	if ( *buyVar.GetString() )
		return;

	// First, look for a mapcycle file in the cfg directory, which is preferred
	char szRecommendedName[ 256 ];
	char szResolvedName[ 256 ];
	V_sprintf_safe( szRecommendedName, "cfg/%s", filename );
	V_strcpy_safe( szResolvedName, szRecommendedName );
	if ( filesystem->FileExists( szResolvedName, "GAME" ) )
	{
		Msg( "Loading '%s'.\n", szResolvedName );
	}
	else
	{
		// Check the root
		V_strcpy_safe( szResolvedName, filename );
		if ( filesystem->FileExists( szResolvedName, "GAME" ) )
		{
			Msg( "Loading '%s'  ('%s' was not found.)\n", szResolvedName, szRecommendedName );
		}
		else
		{

			// Try cfg/xxx_default.txt
			V_strcpy_safe( szResolvedName, szRecommendedName );
			char *dotTxt = V_stristr( szResolvedName, ".txt" );
			Assert( dotTxt );
			if ( dotTxt )
			{
				V_strcpy( dotTxt, "_default.txt" );
			}
			if ( !filesystem->FileExists( szResolvedName, "GAME" ) )
			{
				Warning( "Not loading buy data.  Neither '%s' nor %s were found.\n", szResolvedName, szRecommendedName );
				return;
			}
			Msg( "Loading '%s'\n", szResolvedName );
		}
	}

	CUtlBuffer buf;
	if ( !filesystem->ReadFile( szResolvedName, "GAME", buf ) )
	{
		// WAT
		Warning( "Failed to load '%s'.\n", szResolvedName );
		return;
	}
	buf.PutChar('\0');

	char token[256];
	char buystring[256];
	V_sprintf_safe( buystring, "setinfo %s \"", buyVar.GetName() );

	const char *pfile = engine->ParseFile( (const char *)buf.Base(), token, sizeof(token) );

	bool first = true;

	while (pfile != NULL)
	{
		if (first)
		{
			first = false;
		}
		else
		{
			Q_strncat(buystring, " ", sizeof(buystring), COPY_ALL_CHARACTERS);
		}

		Q_strncat(buystring, token, sizeof(buystring), COPY_ALL_CHARACTERS);

		pfile = engine->ParseFile( pfile, token, sizeof(token) );
	}

	Q_strncat(buystring, "\"", sizeof(buystring), COPY_ALL_CHARACTERS);

	engine->ClientCmd(buystring);
}

void MsgFunc_KillCam(bf_read &msg) 
{
	C_CSPlayer *pPlayer = ToCSPlayer( C_BasePlayer::GetLocalPlayer() );

	if ( !pPlayer )
		return;

	int newMode = msg.ReadByte();

	if ( newMode != g_nKillCamMode )
	{
#if !defined( NO_ENTITY_PREDICTION )
		if ( g_nKillCamMode == OBS_MODE_NONE )
		{
			// kill cam is switch on, turn off prediction
			g_bForceCLPredictOff = true;
		}
		else if ( newMode == OBS_MODE_NONE )
		{
			// kill cam is switched off, restore old prediction setting is we switch back to normal mode
			g_bForceCLPredictOff = false;
		}
#endif
		g_nKillCamMode = newMode;
	}

	g_nKillCamTarget1	= msg.ReadByte();
	g_nKillCamTarget2	= msg.ReadByte();
}

// --------------------------------------------------------------------------------- //
// CCSModeManager.
// --------------------------------------------------------------------------------- //

class CCSModeManager : public IVModeManager
{
public:
	virtual void	Init();
	virtual void	SwitchMode( bool commander, bool force ) {}
	virtual void	LevelInit( const char *newmap );
	virtual void	LevelShutdown( void );
	virtual void	ActivateMouse( bool isactive ) {}
};

static CCSModeManager g_ModeManager;
IVModeManager *modemanager = ( IVModeManager * )&g_ModeManager;

// --------------------------------------------------------------------------------- //
// CCSModeManager implementation.
// --------------------------------------------------------------------------------- //

#define SCREEN_FILE		"scripts/vgui_screens.txt"

void CCSModeManager::Init()
{
	g_pClientMode = GetClientModeNormal();
	
	PanelMetaClassMgr()->LoadMetaClassDefinitionFile( SCREEN_FILE );
}

void CCSModeManager::LevelInit( const char *newmap )
{
	g_pClientMode->LevelInit( newmap );

	SetBuyData( cl_autobuy, "autobuy.txt" );
	SetBuyData( cl_rebuy, "rebuy.txt" );

#if !defined( NO_ENTITY_PREDICTION )
	if ( g_nKillCamMode > OBS_MODE_NONE )
	{
		g_bForceCLPredictOff = false;
	}
#endif

	g_nKillCamMode		= OBS_MODE_NONE;
	g_nKillCamTarget1	= 0;
	g_nKillCamTarget2	= 0;

	// HACK: the detail sway convars are archive, and default to 0.  Existing CS:S players thus have no detail
	// prop sway.  We'll force them to DoD's default values for now.
	if ( !cl_detail_max_sway.GetFloat() &&
		!cl_detail_avoid_radius.GetFloat() &&
		!cl_detail_avoid_force.GetFloat() &&
		!cl_detail_avoid_recover_speed.GetFloat() )
	{
		cl_detail_max_sway.SetValue( "5" );
		cl_detail_avoid_radius.SetValue( "64" );
		cl_detail_avoid_force.SetValue( "0.4" );
		cl_detail_avoid_recover_speed.SetValue( "0.25" );
	}
}

void CCSModeManager::LevelShutdown( void )
{
	g_pClientMode->LevelShutdown();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
ClientModeCSNormal::ClientModeCSNormal()
{
	m_CCDeathHandle = INVALID_CLIENT_CCHANDLE;
	m_CCDeathPercent = 0.0f;
	m_CCFreezePeriodHandle_CT = INVALID_CLIENT_CCHANDLE;
	m_CCFreezePeriodPercent_CT = 0.0f;
	m_CCFreezePeriodHandle_T = INVALID_CLIENT_CCHANDLE;
	m_CCFreezePeriodPercent_T = 0.0f;

	HOOK_MESSAGE( MatchEndConditions );
}

void ClientModeCSNormal::Init()
{
	BaseClass::Init();

	ListenForGameEvent( "round_end" );
	ListenForGameEvent( "round_start" );
	ListenForGameEvent( "round_time_warning" );
	ListenForGameEvent( "cs_round_start_beep" );
	ListenForGameEvent( "cs_round_final_beep" );
	ListenForGameEvent( "player_team" );
	ListenForGameEvent( "player_death" );
	ListenForGameEvent( "bomb_planted" );
	ListenForGameEvent( "bomb_exploded" );
	ListenForGameEvent( "bomb_defused" );
	ListenForGameEvent( "hostage_follows" );
	ListenForGameEvent( "hostage_killed" );
	ListenForGameEvent( "hostage_hurt" );
	ListenForGameEvent( "round_freeze_end" );
	ListenForGameEvent( "round_mvp" );
	ListenForGameEvent( "bot_takeover" );
	ListenForGameEvent( "server_spawn" );
	ListenForGameEvent( "smokegrenade_detonate" );
	ListenForGameEvent( "smokegrenade_expired" );

	usermessages->HookMessage( "KillCam", MsgFunc_KillCam );

	if ( m_CCDeathHandle == INVALID_CLIENT_CCHANDLE )
	{
		const char *szRawFile = "materials/correction/cc_death.raw";
		m_CCDeathPercent = 0.0f;
		m_CCDeathHandle = g_pColorCorrectionMgr->AddColorCorrection( szRawFile );
	}

	if ( m_CCFreezePeriodHandle_CT == INVALID_CLIENT_CCHANDLE )
	{
		const char *szRawFile = "materials/correction/cc_freeze_ct.raw";
		m_CCFreezePeriodPercent_CT = 0.0f;
		m_CCFreezePeriodHandle_CT = g_pColorCorrectionMgr->AddColorCorrection( szRawFile );
	}

	if ( m_CCFreezePeriodHandle_T == INVALID_CLIENT_CCHANDLE )
	{
		const char *szRawFile = "materials/correction/cc_freeze_t.raw";
		m_CCFreezePeriodPercent_T = 0.0f;
		m_CCFreezePeriodHandle_T = g_pColorCorrectionMgr->AddColorCorrection( szRawFile );
	}

	m_fDelayedCTWinTime = -1.0f;
	m_nRoundMVP = 0;
}

void ClientModeCSNormal::InitViewport()
{
	BaseClass::InitViewport();

	m_pViewport = new CounterStrikeViewport();
	m_pViewport->Start( gameuifuncs, gameeventmanager );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ClientModeCSNormal::LevelShutdown( void )
{
	BaseClass::LevelShutdown();

	// Remove any lingering debug overlays, since it's possible they won't get cleaned up automatically later.
	// This is in response to anecdotal reports that players can 'mark' the world with showimpacts or grenade trajectories,
	// then use them to their advantage on subsequent games played immediately on the same map.
	debugoverlay->ClearAllOverlays();
}


void ClientModeCSNormal::Update()
{
	BaseClass::Update();

	// Override the hud's visibility if this is a logo (like E3 demo) map.
	if ( CSGameRules() && CSGameRules()->IsLogoMap() )
		m_pViewport->SetVisible( false );

	if ( (m_fDelayedCTWinTime > 0.0f) && (gpGlobals->curtime >= m_fDelayedCTWinTime) )
	{
		CLocalPlayerFilter filter;
		C_BaseEntity::EmitSound( filter, SOUND_FROM_LOCAL_PLAYER, "Event.CTWin");

		m_fDelayedCTWinTime = -1.0f;
	}

	// halftime music needs a delay thusly
	static bool bStartedHalfTimeMusic = false;
	static float flHalfTimeStart = 0.0;

	if ( CSGameRules() && (CSGameRules()->GetGamePhase() == GAMEPHASE_HALFTIME || CSGameRules()->GetGamePhase() == GAMEPHASE_MATCH_ENDED) )
	{
		if( !bStartedHalfTimeMusic && gpGlobals->curtime - flHalfTimeStart > 6.5 )
		{
			bStartedHalfTimeMusic = true;
			CSingleUserRecipientFilter filter(C_BasePlayer::GetLocalPlayer());
			if( CSGameRules()->GetGamePhase() == GAMEPHASE_HALFTIME )
			{
				PlayMusicSelection(filter, CSMUSIC_HALFTIME);
			}
			else if( m_nRoundMVP != 0 )
			{
				PlayMusicSelection(filter, CSMUSIC_HALFTIME, m_nRoundMVP );
			}
		}
	}
	else
	{
		flHalfTimeStart = gpGlobals->curtime;
		bStartedHalfTimeMusic = false;
	}
}

//--------------------------------------------------------------------------------------------------------
void ClientModeCSNormal::UpdateColorCorrectionWeights( void )
{
	C_CSPlayer* pPlayer = C_CSPlayer::GetLocalCSPlayer();

	if ( !pPlayer )
	{
		m_CCDeathPercent = 0.0f;
		m_CCFreezePeriodPercent_CT = 0.0f;
		m_CCFreezePeriodPercent_T = 0.0f;
		return;
	}

	bool isDying = false;
	if ( !pPlayer->IsAlive() && (pPlayer->GetObserverMode() == OBS_MODE_DEATHCAM) )
	{
		isDying = true;
	}

	m_CCDeathPercent = clamp( m_CCDeathPercent + ((isDying) ? 0.1f : -0.1f), 0.0f, 1.0f );

	// apply CT and T CCs from spectated players as well
	// no need for that with death CC
	pPlayer = GetHudPlayer();
	if ( !pPlayer )
	{
		m_CCDeathPercent = 0.0f;
		m_CCFreezePeriodPercent_CT = 0.0f;
		m_CCFreezePeriodPercent_T = 0.0f;
		return;
	}
	
	float flTimer = 0;

	bool bFreezePeriod = CSGameRules()->IsFreezePeriod();
	bool bImmune = pPlayer->m_bImmunity;
	if ( bFreezePeriod || bImmune )
	{
		float flFadeBegin = 2.0f;

		// countdown to the start of the round while we're in freeze period
		if ( bImmune )
		{
			// if freeze time is also active and freeze time is longer than immune time, use that time instead
			if ( bFreezePeriod && (CSGameRules()->GetRoundStartTime() - gpGlobals->curtime) > (pPlayer->m_fImmuneToDamageTime - gpGlobals->curtime))
				flTimer = CSGameRules()->GetRoundStartTime() - gpGlobals->curtime;
			else
			{
				flTimer = pPlayer->m_fImmuneToDamageTime - gpGlobals->curtime;
				flFadeBegin = 0.5;
			}
		}
		else if ( bFreezePeriod )
		{
			flTimer = CSGameRules()->GetRoundStartTime() - gpGlobals->curtime;
		}

		if ( flTimer > flFadeBegin )
		{
			if ( pPlayer->GetTeamNumber() == TEAM_CT )
			{
				m_CCFreezePeriodPercent_CT = 1.0f;
				m_CCFreezePeriodPercent_T = 0.0f;
			}
			else if ( pPlayer->GetTeamNumber() == TEAM_TERRORIST )
			{
				m_CCFreezePeriodPercent_CT = 0.0f;
				m_CCFreezePeriodPercent_T = 1.0f;
			}
			else
			{
				m_CCFreezePeriodPercent_CT = 0.0f;
				m_CCFreezePeriodPercent_T = 0.0f;
			}
		}
		else
		{
			if ( pPlayer->GetTeamNumber() == TEAM_CT )
			{
				m_CCFreezePeriodPercent_CT = clamp( flTimer / flFadeBegin, 0.05f, 1.0f );
				m_CCFreezePeriodPercent_T = 0;
			}
			else if ( pPlayer->GetTeamNumber() == TEAM_TERRORIST )
			{
				m_CCFreezePeriodPercent_T = clamp( flTimer / flFadeBegin, 0.05f, 1.0f );
				m_CCFreezePeriodPercent_CT = 0;
			}
			else
			{
				m_CCFreezePeriodPercent_CT = 0.0f;
				m_CCFreezePeriodPercent_T = 0.0f;
			}
		}
	}
	else
	{
		m_CCFreezePeriodPercent_T = 0;
		m_CCFreezePeriodPercent_CT = 0;
	}

}

void ClientModeCSNormal::OnColorCorrectionWeightsReset( void )
{
	UpdateColorCorrectionWeights();
	g_pColorCorrectionMgr->SetColorCorrectionWeight( m_CCDeathHandle, m_CCDeathPercent );
	g_pColorCorrectionMgr->SetColorCorrectionWeight( m_CCFreezePeriodHandle_CT, m_CCFreezePeriodPercent_CT );
	g_pColorCorrectionMgr->SetColorCorrectionWeight( m_CCFreezePeriodHandle_T, m_CCFreezePeriodPercent_T );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pSetup - 
//-----------------------------------------------------------------------------
void ClientModeCSNormal::OverrideView( CViewSetup* pSetup )
{
	QAngle camAngles;

	// Let the player override the view.
	C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
	if ( !pPlayer )
		return;

	pPlayer->OverrideView( pSetup );

	if ( ::input->CAM_IsThirdPerson() )
	{
		const Vector& cam_ofs = g_ThirdPersonManager.GetCameraOffsetAngles();
		Vector cam_ofs_distance = g_ThirdPersonManager.GetDesiredCameraOffset();

		cam_ofs_distance *= g_ThirdPersonManager.GetDistanceFraction();

		camAngles[PITCH] = cam_ofs[PITCH];
		camAngles[YAW] = cam_ofs[YAW];
		camAngles[ROLL] = 0;

		Vector camForward, camRight, camUp;

		if ( g_ThirdPersonManager.IsOverridingThirdPerson() == false )
		{
			engine->GetViewAngles( camAngles );
		}

		// get the forward vector
		AngleVectors( camAngles, &camForward, &camRight, &camUp );

		VectorMA( pSetup->origin, -cam_ofs_distance[0], camForward, pSetup->origin );
		VectorMA( pSetup->origin, cam_ofs_distance[1], camRight, pSetup->origin );
		VectorMA( pSetup->origin, cam_ofs_distance[2], camUp, pSetup->origin );

		CBasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();

		// PiMoN: apply view punch separately for third person
		if ( pPlayer && cam_recoil.GetBool() )
		{
			// Apply punch angles
			VectorAdd( camAngles, pPlayer->m_Local.m_viewPunchAngle, camAngles );

			// TODO[pmf]: apply a scaling factor to this
			VectorAdd( camAngles, pPlayer->GetAimPunchAngle() * view_recoil_tracking.GetFloat(), camAngles );
		}

		// Override angles from third person camera
		VectorCopy( camAngles, pSetup->angles );
	}
	else if ( ::input->CAM_IsOrthographic() )
	{
		pSetup->m_bOrtho = true;
		float w, h;
		::input->CAM_OrthographicSize( w, h );
		w *= 0.5f;
		h *= 0.5f;
		pSetup->m_OrthoLeft = -w;
		pSetup->m_OrthoTop = -h;
		pSetup->m_OrthoRight = w;
		pSetup->m_OrthoBottom = h;
	}
}

/*
void ClientModeCSNormal::UpdateSpectatorMode( void )
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();

	if ( !pPlayer )
		return;

	IMapOverview * overviewmap = m_pViewport->GetMapOverviewInterface();

	if ( !overviewmap )
		return;

	overviewmap->SetTime( gpGlobals->curtime );

	int obs_mode = pPlayer->GetObserverMode();

	if ( obs_mode < OBS_MODE_IN_EYE )
		return;

	Vector worldpos = pPlayer->GetLocalOrigin();
	QAngle angles; engine->GetViewAngles( angles );

	C_BaseEntity *target = pPlayer->GetObserverTarget();

	if ( target && (obs_mode == OBS_MODE_IN_EYE || obs_mode == OBS_MODE_CHASE) )
	{
		worldpos = target->GetAbsOrigin();

		if ( obs_mode == OBS_MODE_IN_EYE )
		{
			angles = target->GetAbsAngles();
		}
	}

	Vector2D mappos = overviewmap->WorldToMap( worldpos );

	overviewmap->SetCenter( mappos );
	overviewmap->SetAngle( angles.y );	
	
	for ( int i = 1; i<= MAX_PLAYERS; i++)
	{
		C_BaseEntity *ent = ClientEntityList().GetEnt( i );

		if ( !ent || !ent->IsPlayer() )
			continue;

		C_BasePlayer *p = ToBasePlayer( ent );

		// update position of active players in our PVS
		Vector position = p->GetAbsOrigin();
		QAngle angle = p->GetAbsAngles();

		if ( p->IsDormant() )
		{
			// if player is not in PVS, use PlayerResources data
			position = g_PR->GetPosition( i );
			angles[1] = g_PR->GetViewAngle( i );
		}
		
		overviewmap->SetPlayerPositions( i-1, position, angles );
	}
} */

//-----------------------------------------------------------------------------
// Purpose: We've received a keypress from the engine. Return 1 if the engine is allowed to handle it.
//-----------------------------------------------------------------------------
int	ClientModeCSNormal::KeyInput( int down, ButtonCode_t keynum, const char *pszCurrentBinding )
{
	// don't process input in LogoMaps
	if( CSGameRules() && CSGameRules()->IsLogoMap() )
		return 1;

#if SCOREBOARD_MOUSE_INPUT
	if ( keynum == MOUSE_RIGHT )
	{
		EditablePanel* pScoreBoard = dynamic_cast<EditablePanel*>(gViewPortInterface->FindPanelByName( PANEL_SCOREBOARD ));
		if ( pScoreBoard && pScoreBoard->IsVisible() )
		{
			pScoreBoard->SetMouseInputEnabled( true );
			pScoreBoard->RequestFocus();
			return 0;
		}
	}
#endif
	
	return BaseClass::KeyInput( down, keynum, pszCurrentBinding );
}



IClientMode *GetClientModeNormal()
{
	static ClientModeCSNormal g_ClientModeNormal;
	return &g_ClientModeNormal;
}


ClientModeCSNormal* GetClientModeCSNormal()
{
	Assert( dynamic_cast< ClientModeCSNormal* >( GetClientModeNormal() ) );

	return static_cast< ClientModeCSNormal* >( GetClientModeNormal() );
}

#ifdef DEBUG
	ConVar ironsight_scoped_viewmodel_fov( "ironsight_scoped_viewmodel_fov", "54", FCVAR_CHEAT, "The fov of the viewmodel when ironsighted" );
#else
	#define IRONSIGHT_SCOPED_FOV 54.0f
#endif

float ClientModeCSNormal::GetViewModelFOV( void )
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if ( pPlayer )
	{
		CWeaponCSBase *pIronSightWeapon = (CWeaponCSBase*)pPlayer->GetActiveWeapon();
		if ( pIronSightWeapon )
		{
			CIronSightController* pIronSightController = pIronSightWeapon->GetIronSightController();
			if ( pIronSightController && pIronSightController->IsInIronSight() )
			{
				return FLerp( v_viewmodel_fov.GetFloat(),	
					#ifdef DEBUG
						ironsight_scoped_viewmodel_fov.GetFloat(),
					#else
						IRONSIGHT_SCOPED_FOV,
					#endif
				pIronSightController->GetIronSightAmount() );
			}
		}
	}
	return v_viewmodel_fov.GetFloat();
}

void ClientModeCSNormal::FireGameEvent( IGameEvent *event )
{
	C_BasePlayer *pLocalPlayer = C_BasePlayer::GetLocalPlayer();
	CLocalPlayerFilter filter;

	if ( !pLocalPlayer )
		return;

	const char *eventname = event->GetName();

	if ( !eventname || !eventname[0] )
		return;

	if ( Q_strcmp( "round_start", eventname ) == 0 )
	{
		m_nRoundMVP = 0;
		// recreate all client side physics props
		C_PhysPropClientside::RecreateAll();

		// remove hostage ragdolls
		for ( int i=0; i<g_HostageRagdolls.Count(); ++i )
		{
			// double-check that the EHANDLE is still valid
			if ( g_HostageRagdolls[i] )
			{
				g_HostageRagdolls[i]->Release();
			}
		}
		g_HostageRagdolls.RemoveAll();

		// Just tell engine to clear decals
		engine->ClientCmd( "r_cleardecals\n" );

		//stop any looping sounds
		//enginesound->StopAllSounds( true );

		CBaseEntity::EmitSound( filter, pLocalPlayer->entindex(), "Music.StopAllExceptMusic" );

		Soundscape_OnStopAllSounds();	// Tell the soundscape system.

		// Remove any left over particle effects from the last round.
		ParticleMgr()->SetRemoveAllParticleEffects();
	}
	else if ( Q_strcmp( "round_end", eventname ) == 0 )
	{
		if ( !pLocalPlayer )
			return;

		int winningTeam = event->GetInt("winner");
		int reason = event->GetInt("reason");

		if ( reason != Game_Commencing )
		{
			// if spectating play music for team being spectated at that moment
			C_BasePlayer* pTeamPlayer = pLocalPlayer;
			if ( pLocalPlayer->GetTeamNumber() == TEAM_SPECTATOR || pLocalPlayer->IsHLTV() )
			{
				pTeamPlayer = GetHudPlayer();
			}
			if ( winningTeam == pTeamPlayer->GetTeamNumber() )
			{
				PlayMusicSelection( filter, CSMUSIC_WONROUND );
			}
			else
			{
				PlayMusicSelection( filter, CSMUSIC_LOSTROUND );
			}
		}

		// play endround announcer sound
		if ( winningTeam == TEAM_CT )
		{
			if ( reason == Bomb_Defused )
			{
				C_BaseEntity::EmitSound( filter, SOUND_FROM_LOCAL_PLAYER, "Event.BombDefused" );
				m_fDelayedCTWinTime = gpGlobals->curtime + C_BaseEntity::GetSoundDuration( "Event.BombDefused", NULL ) + 0.3;
			}
			else
			{
				C_BaseEntity::EmitSound( filter, SOUND_FROM_LOCAL_PLAYER, "Event.CTWin");
			}
		}
		else if ( winningTeam == TEAM_TERRORIST )
		{
			C_BaseEntity::EmitSound( filter, SOUND_FROM_LOCAL_PLAYER, "Event.TERWin");
		}
		else
		{
			C_BaseEntity::EmitSound( filter, SOUND_FROM_LOCAL_PLAYER, "Event.RoundDraw");
		}
		
		// [pfreese] Only show centerprint message for game commencing; the rest of 
		// these messages are handled by the end-of-round panel.
		// [Forrest] Show all centerprint messages if the end-of-round panel is disabled.
		static ConVarRef sv_nowinpanel( "sv_nowinpanel" );
		static ConVarRef cl_nowinpanel( "cl_nowinpanel" );
		bool isFinishedGunGameRound = CSGameRules()->IsPlayingGunGameProgressive() && (reason == CTs_Win || reason == Terrorists_Win);
		if ( isFinishedGunGameRound || reason == Game_Commencing || sv_nowinpanel.GetBool() || cl_nowinpanel.GetBool() )
		{
			internalCenterPrint->Print( hudtextmessage->LookupString( event->GetString("message") ) );

			// we are starting a new round; clear the current match stats
			g_CSClientGameStats.ResetMatchStats();
		}
	}
	else if ( Q_strcmp( "player_team", eventname ) == 0 )
	{
		CBaseHudChat *pHudChat = (CBaseHudChat *)GET_HUDELEMENT( CHudChat );
		C_BasePlayer *pPlayer = USERID2PLAYER( event->GetInt("userid") );
		
		if ( !pPlayer || !pHudChat )
			return;

		bool bDisconnected = event->GetBool("disconnect");

		if ( bDisconnected )
			return;

		int iTeam = event->GetInt("team");

		if ( pPlayer->IsLocalPlayer() )
		{
			// that's me
			pPlayer->TeamChange( iTeam );
		}
		
		bool bSilent = event->GetBool( "silent" );
		if ( !bSilent )
		{
			wchar_t wszLocalized[100];
			wchar_t wszPlayerName[MAX_PLAYER_NAME_LENGTH];
			char szLocalized[100];
			bool bIsBot = event->GetBool("isbot"); // squelch 'bot has joined the game' messages

			if ( iTeam == TEAM_SPECTATOR && !bIsBot )
			{
				g_pVGuiLocalize->ConvertANSIToUnicode( pPlayer->GetPlayerName(), wszPlayerName, sizeof(wszPlayerName) );
				g_pVGuiLocalize->ConstructString( wszLocalized, sizeof( wszLocalized ), g_pVGuiLocalize->Find( "#Cstrike_game_join_spectators" ), 1, wszPlayerName );

				g_pVGuiLocalize->ConvertUnicodeToANSI( wszLocalized, szLocalized, sizeof(szLocalized) );
				pHudChat->Printf( CHAT_FILTER_NONE, "%s", szLocalized );
			}
			else if ( iTeam == TEAM_TERRORIST && !bIsBot )
			{
				g_pVGuiLocalize->ConvertANSIToUnicode( pPlayer->GetPlayerName(), wszPlayerName, sizeof(wszPlayerName) );
				g_pVGuiLocalize->ConstructString( wszLocalized, sizeof( wszLocalized ), g_pVGuiLocalize->Find( "#Cstrike_game_join_terrorist" ), 1, wszPlayerName );

				g_pVGuiLocalize->ConvertUnicodeToANSI( wszLocalized, szLocalized, sizeof(szLocalized) );
				pHudChat->Printf( CHAT_FILTER_NONE, "%s", szLocalized );
			}
			else if ( iTeam == TEAM_CT && !bIsBot )
			{
				g_pVGuiLocalize->ConvertANSIToUnicode( pPlayer->GetPlayerName(), wszPlayerName, sizeof(wszPlayerName) );
				g_pVGuiLocalize->ConstructString( wszLocalized, sizeof( wszLocalized ), g_pVGuiLocalize->Find( "#Cstrike_game_join_ct" ), 1, wszPlayerName );

				g_pVGuiLocalize->ConvertUnicodeToANSI( wszLocalized, szLocalized, sizeof(szLocalized) );
				pHudChat->Printf( CHAT_FILTER_NONE, "%s", szLocalized );
			}
		}
	}
	else if ( Q_strcmp( "bomb_planted", eventname ) == 0 )
	{
		//C_BasePlayer *pPlayer = USERID2PLAYER( event->GetInt("userid") );

		wchar_t wszLocalized[100];
		wchar_t seconds[4];

		V_swprintf_safe( seconds, L"%d", mp_c4timer.GetInt() );

		g_pVGuiLocalize->ConstructString( wszLocalized, sizeof( wszLocalized ), g_pVGuiLocalize->Find( "#Cstrike_TitlesTXT_Bomb_Planted" ), 1, seconds );

		// show centerprint message
		internalCenterPrint->Print( wszLocalized );

		PlayMusicSelection( filter, CSMUSIC_BOMB );

		// play sound
		 C_BaseEntity::EmitSound( filter, SOUND_FROM_LOCAL_PLAYER, "Event.BombPlanted")  ;
	}
	else if ( Q_strcmp( "bomb_defused", eventname ) == 0 )
	{
		// C_BasePlayer *pPlayer = USERID2PLAYER( event->GetInt("userid") );
	}

	// [menglish] Tell the client side bomb that the bomb has exploding here creating the explosion particle effect	 
	else if ( Q_strcmp( "bomb_exploded", eventname ) == 0 )
	{
		if ( g_PlantedC4s.Count() > 0 )
		{
			// bomb is planted
			C_PlantedC4 *pC4 = g_PlantedC4s[0];
			pC4->Explode();
		}
	}
	else if ( Q_strcmp( "hostage_follows", eventname ) == 0 )
	{
		internalCenterPrint->Print( "#Cstrike_TitlesTXT_Hostage_Being_Taken" );

		bool roundWasAlreadyWon = ( CSGameRules()->m_iRoundWinStatus != WINNER_NONE );
		if ( !roundWasAlreadyWon )
		{
			PlayMusicSelection( filter, CSMUSIC_HOSTAGE );
		}
	}
	else if ( Q_strcmp( "hostage_killed", eventname ) == 0 )
	{
		if ( !pLocalPlayer )
			return;

		// play sound for spectators and CTs
		if ( pLocalPlayer->IsObserver() || (pLocalPlayer->GetTeamNumber() == TEAM_CT) )
		{
			C_BaseEntity::EmitSound( filter, SOUND_FROM_LOCAL_PLAYER, "Event.HostageKilled")  ;
		}

		// Show warning to killer
		if ( pLocalPlayer->GetUserID() == event->GetInt("userid") )
		{
			internalCenterPrint->Print( "#Cstrike_TitlesTXT_Killed_Hostage" );
		}
	}
	else if ( Q_strcmp( "hostage_hurt", eventname ) == 0 )
	{
		if ( !pLocalPlayer )
			return;

		// Let the loacl player know he harmed a hostage
		if ( pLocalPlayer->GetUserID() == event->GetInt("userid") )
		{
			internalCenterPrint->Print( "#Cstrike_TitlesTXT_Injured_Hostage" );
		}
	}
	else if ( Q_strcmp( "player_death", eventname ) == 0 )
	{
		C_BasePlayer *pPlayer = USERID2PLAYER( event->GetInt("userid") );

		C_CSPlayer* csPlayer = ToCSPlayer(pPlayer);
		if (csPlayer)
		{
			csPlayer->ClearSoundEvents();
		}

		if ( pPlayer == C_BasePlayer::GetLocalPlayer() )
		{
			// we just died, hide any buy panels
			gViewPortInterface->ShowPanel( PANEL_BUY, false );
		}
	}
	else if ( Q_strcmp( "player_changename", eventname ) == 0 )
	{
		return; // server sends a colorized text string for this
	}

    // [tj] We handle this here instead of in the base class 
    //      The reason is that we don't use string tables to localize.
    //      Instead, we use the steam localization mechanism.

    // [dwenger] Remove dependency on stats system for name of achievement.
    else if ( Q_strcmp( "achievement_earned", eventname ) == 0 )
    {
        CBaseHudChat *hudChat = (CBaseHudChat *)GET_HUDELEMENT( CHudChat );
        int iPlayerIndex = event->GetInt( "player" );
        C_BasePlayer *pPlayer = UTIL_PlayerByIndex( iPlayerIndex );
        int iAchievement = event->GetInt( "achievement" );

        if ( !hudChat || !pPlayer )
            return;

        CAchievementMgr *pAchievementMgr = dynamic_cast<CAchievementMgr *>( engine->GetAchievementMgr() );
        if ( !pAchievementMgr )
            return;

        IAchievement *pAchievement = pAchievementMgr->GetAchievementByID( iAchievement );
        if ( pAchievement )
        {
            if ( !pPlayer->IsDormant() && pPlayer->ShouldAnnounceAchievement() )
            {
                pPlayer->SetNextAchievementAnnounceTime( gpGlobals->curtime + ACHIEVEMENT_ANNOUNCEMENT_MIN_TIME );

                //Do something for the player - Actually we should probably do this client-side when the achievement is first earned.
                if (pPlayer->IsLocalPlayer()) 
                {
                }
                pPlayer->OnAchievementAchieved( iAchievement );
            }

			if ( C_CS_PlayerResource *cs_PR = dynamic_cast<C_CS_PlayerResource *>(g_PR) )
			{
				wchar_t wszPlayerName[MAX_DECORATED_PLAYER_NAME_LENGTH];
				cs_PR->GetDecoratedPlayerName( iPlayerIndex, wszPlayerName, sizeof( wszPlayerName ), k_EDecoratedPlayerNameFlag_Simple );

                wchar_t achievementName[1024];
                const wchar_t* constAchievementName = &achievementName[0];

                constAchievementName = ACHIEVEMENT_LOCALIZED_NAME( pAchievement );

                if (constAchievementName)
                {
                    wchar_t wszLocalizedString[128];
                    g_pVGuiLocalize->ConstructString( wszLocalizedString, sizeof( wszLocalizedString ), g_pVGuiLocalize->Find( "#Achievement_Earned" ), 2, wszPlayerName, constAchievementName/*wszAchievementString*/ );

                    char szLocalized[128];
                    g_pVGuiLocalize->ConvertUnicodeToANSI( wszLocalizedString, szLocalized, sizeof( szLocalized ) );

                    hudChat->ChatPrintf( iPlayerIndex, CHAT_FILTER_ACHIEVEMENT, "%s", szLocalized );
                }
            }
        }
    }
	else if ( V_strcmp( "round_freeze_end", eventname ) == 0 )
	{
		if ( !pLocalPlayer )
			return;

		int nObsMode = pLocalPlayer->GetObserverMode();
		if ( nObsMode == OBS_MODE_FIXED || nObsMode == OBS_MODE_ROAMING )
		{
			C_CSPlayer* pCSLocalPlayer = ToCSPlayer( pLocalPlayer );
			if ( pCSLocalPlayer->GetCurrentMusic() == CSMUSIC_START )
			{
				CLocalPlayerFilter filter;
				PlayMusicSelection( filter, CSMUSIC_ACTION );
				pCSLocalPlayer->SetCurrentMusic( CSMUSIC_ACTION );
			}
		}
	}
	else if ( V_strcmp( "round_time_warning", eventname ) == 0 )
	{
		if(	!CSGameRules()->m_bBombPlanted )
		{
			PlayMusicSelection( filter, CSMUSIC_ROUNDTEN );
		}
	}
	else if ( V_strcmp( "cs_round_start_beep", eventname ) == 0 )
	{
		bool bTeamPanelActive = ( gViewPortInterface->GetActivePanel() &&  ( V_strcmp( gViewPortInterface->GetActivePanel()->GetName(), PANEL_TEAM ) == 0 ) );
	
		if( !bTeamPanelActive )
		{
			CLocalPlayerFilter filter;
			CBaseEntity::EmitSound( filter, 0, "UI.CounterBeep" );
		}
	}
	else if ( V_strcmp( "cs_round_final_beep", eventname ) == 0 )
	{
		if ( !pLocalPlayer )
			return;

		bool bTeamPanelActive = ( gViewPortInterface->GetActivePanel() && ( V_strcmp( gViewPortInterface->GetActivePanel()->GetName(), PANEL_TEAM ) == 0 ) );

		if( !bTeamPanelActive )
		{
			CBaseEntity::EmitSound( filter, 0, "UI.CounterDoneBeep" );
		}

		int nObsMode = pLocalPlayer->GetObserverMode();
		if( nObsMode == OBS_MODE_FIXED || nObsMode == OBS_MODE_ROAMING )
		{
			C_CSPlayer *pCSLocalPlayer = ToCSPlayer(pLocalPlayer);
			if(pCSLocalPlayer->GetCurrentMusic() == CSMUSIC_START )
			{
				CLocalPlayerFilter filter;
				PlayMusicSelection(filter, CSMUSIC_ACTION);
				pCSLocalPlayer->SetCurrentMusic(CSMUSIC_ACTION);
			}
		}
	}
	else if ( V_strcmp( "round_mvp", eventname ) == 0 )
	{
		C_BasePlayer *pPlayer = USERID2PLAYER( event->GetInt("userid") );
		if ( pPlayer )
		{
			int nPlayerIndex = pPlayer->entindex();

			if ( C_CS_PlayerResource *cs_PR = dynamic_cast< C_CS_PlayerResource * >( g_PR ) )
			{
				int nMusicID = cs_PR->GetMusicID( nPlayerIndex );
				if ( nMusicID > 1 )
				{
					m_nRoundMVP = nPlayerIndex;
					PlayMusicSelection( filter, CSMUSIC_MVP, nPlayerIndex );
				}
			}
		}
	}
	else if ( V_strcmp( "bot_takeover", eventname ) == 0 )
	{
		C_BasePlayer* pBot = UTIL_PlayerByUserId( event->GetInt( "botid" ) );
		if ( pBot && pLocalPlayer && pLocalPlayer->GetUserID() == event->GetInt( "userid" ) )
		{
			wchar_t wszLocalized[100];
			wchar_t wszPlayerName[MAX_PLAYER_NAME_LENGTH];
			g_pVGuiLocalize->ConvertANSIToUnicode( pBot->GetPlayerName(), wszPlayerName, sizeof( wszPlayerName ) );
			g_pVGuiLocalize->ConstructString( wszLocalized, sizeof( wszLocalized ), g_pVGuiLocalize->Find( "#Cstrike_TitlesTXT_Hint_Bot_Takeover" ), 1, wszPlayerName );

			internalCenterPrint->HintPrint( wszLocalized );
		}
	}
	else if ( V_strcmp( "smokegrenade_detonate", eventname ) == 0 )
	{
		int index = event->GetInt( "entityid" );
		C_BaseEntity* pEnt = ClientEntityList().GetBaseEntity( index );
		if ( pEnt )
			AddSmokeGrenadeHandle( pEnt );
	}
	else if ( V_strcmp( "smokegrenade_expired", eventname ) == 0 )
	{
		int index = event->GetInt( "entityid" );
		C_BaseEntity* pEnt = ClientEntityList().GetBaseEntity( index );
		if ( pEnt )
			RemoveSmokeGrenadeHandle( pEnt );
	}
	else
	{
		BaseClass::FireGameEvent( event );
	}
}

bool ClientModeCSNormal::ShouldDrawViewModel( void )
{
	C_CSPlayer *pPlayer = GetHudPlayer();
	
	if( pPlayer && pPlayer->GetFOV() != CSGameRules()->DefaultFOV() && pPlayer->m_bIsScoped )
	{
		CWeaponCSBase *pWpn = pPlayer->GetActiveCSWeapon();

		if( pWpn && pWpn->GetCSWpnData().m_bHideViewmodelWhenZoomed )
		{
			return false;
		}
	}

	return BaseClass::ShouldDrawViewModel();
}


bool ClientModeCSNormal::CanRecordDemo( char *errorMsg, int length ) const
{
	C_CSPlayer *player = C_CSPlayer::GetLocalCSPlayer();
	if ( !player )
	{
		return true;
	}

	if ( !player->IsAlive() )
	{
		return true;
	}

	// don't start recording while flashed, as it would remove the flash
	if ( player->m_flFlashBangTime > gpGlobals->curtime )
	{
		Q_strncpy( errorMsg, "Cannot record demos while blind.", length );
		return false;
	}

	// don't start recording while smoke grenades are spewing smoke, as the existing smoke would be destroyed
	C_BaseEntityIterator it;
	C_BaseEntity *ent;
	while ( (ent = it.Next()) != NULL )
	{
		if ( Q_strcmp( ent->GetClassname(), "class C_ParticleSmokeGrenade" ) == 0 )
		{
			Q_strncpy( errorMsg, "Cannot record demos while a smoke grenade is active.", length );
			return false;
		}
	}

	return true;
}

void ClientModeCSNormal::DoPostScreenSpaceEffects( const CViewSetup *pSetup ) 
{
}

//=============================================================================
// HPE_BEGIN:
// [menglish] Save server information shown to the client in a persistent place
//=============================================================================
 
void ClientModeCSNormal::SetServerName(wchar_t* name)
{
	V_wcsncpy( m_pServerName, name, sizeof( m_pServerName ) );
}

void ClientModeCSNormal::SetMapName(wchar_t* name)
{
	V_wcsncpy( m_pMapName, name, sizeof( m_pMapName ) );
}

//=============================================================================
// HPE_END
//=============================================================================

// Receive the PlayerIgnited user message and send out a clientside event for achievements to hook.
void __MsgFunc_MatchEndConditions( bf_read &msg )
{
	int iFragLimit = (int) msg.ReadLong();
	int iMaxRounds = (int) msg.ReadLong();
	int iTimeLimit = (int) msg.ReadLong();

	IGameEvent *event = gameeventmanager->CreateEvent( "match_end_conditions" );
	if ( event )
	{
		event->SetInt( "frags", iFragLimit );
		event->SetInt( "max_rounds", iMaxRounds );
		event->SetInt( "time", iTimeLimit );
		gameeventmanager->FireEventClientSide( event );
	}
}
