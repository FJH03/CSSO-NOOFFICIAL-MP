//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "cstrikespectatorgui.h"
#include "hud.h"
#include "cs_shareddefs.h"

#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include "vgui/IVGui.h"
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/VectorImagePanel.h>
#include <filesystem.h>
#include "cs_gamerules.h"
#include "c_team.h"
#include "c_cs_playerresource.h"
#include "c_plantedc4.h"
#include "c_cs_hostage.h"
#include "vtf/vtf.h"
#include "clientmode.h"
#include <vgui_controls/AnimationController.h>
#include "voice_status.h"
#include "view.h"
#include "VGuiMatSurface/IMatSystemSurface.h"
#include "c_cs_team.h"
#include "coordsize.h"
#include "hud_macros.h"

using namespace vgui;
DECLARE_HUDELEMENT( CCSMapOverview )
DECLARE_HUD_MESSAGE( CCSMapOverview, UpdateRadar );

CUtlVector<CPlayerRadarFlash> g_RadarFlashes;

void Radar_FlashPlayer( int iPlayer )
{
	if ( g_RadarFlashes.Count() <= iPlayer )
	{
		g_RadarFlashes.AddMultipleToTail( iPlayer - g_RadarFlashes.Count() + 1 );
	}

	CPlayerRadarFlash *pFlash = &g_RadarFlashes[iPlayer];
	pFlash->m_flNextRadarFlashTime = gpGlobals->curtime;
	pFlash->m_iNumRadarFlashes = 16;
	pFlash->m_bRadarFlash = false;

	g_pMapOverview->FlashEntity(iPlayer);
}

extern ConVar sv_disable_radar;
extern ConVar mp_maxrounds;
extern ConVar mp_overtime_maxrounds;
extern ConVar overview_health;
extern ConVar overview_names;
extern ConVar overview_tracks;
extern ConVar overview_locked;
extern ConVar overview_alpha;
extern ConVar cl_draw_only_deathnotices;
ConVar cl_radar_square( "cl_radar_square", "2", FCVAR_ARCHIVE, "0 - round radar, 1 - square radar, 2 - square when scoreboard is visible", true, 0, true, 2 );
ConVar cl_radaralpha( "cl_radaralpha", "200", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, NULL, true, 0, true, 255 );
ConVar cl_radar_rotate( "cl_radar_rotate", "1", FCVAR_ARCHIVE, "1" );
ConVar cl_radar_scale( "cl_radar_scale", "0.7", FCVAR_ARCHIVE, "Sets the radar scale. Valid values are 0.25 to 1.0.", true, 0.25f, true, 1.0f );

#define HOSTAGE_RESCUE_DURATION (2.5f)
#define BOMB_FADE_DURATION (2.5f)
#define DEATH_ICON_FADE (7.5f)
#define DEATH_ICON_DURATION (10.0f)
#define LAST_SEEN_ICON_DURATION (4.0f)
#define LAST_SEEN_ICON_FADE (2.0f)

// To make your own green radar file from the map overview file, turn this on, and include vtf.lib
#define no_GENERATE_RADAR_FILE

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CCSSpectatorGUI::CCSSpectatorGUI(IViewPort *pViewPort) : CSpectatorGUI(pViewPort)
{
	m_pCTLabel = NULL;
	m_pCTScore = NULL;
	m_pTerLabel = NULL;
	m_pTerScore = NULL;
	m_pTimerLabel = NULL;
	m_pRoundCountLabel = NULL;
	m_pPlayerPanelName = NULL;
	m_pPlayerPanelTeam = NULL;
	m_pPlayerPanelBkg = NULL;
	m_pPlayerPanelAvatar = NULL;
	m_pPlayerPanelAvatarBkg = NULL;
	m_pPlayerPanelBorderUpper = NULL;
	m_pBombIcon = NULL;

	m_nLastTime = -1;
	m_nLastSpecMode = -1;
	m_nLastSpecTarget = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CCSSpectatorGUI::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings( pScheme );

	// Grab some control pointers
	m_pCTLabel = dynamic_cast<Label*>(FindChildByName( "CTScoreLabel" ));
	m_pCTScore = dynamic_cast<Label*>(FindChildByName( "CTScoreValue" ));
	m_pTerLabel = dynamic_cast<Label*>(FindChildByName( "TerScoreLabel" ));
	m_pTerScore = dynamic_cast<Label*>(FindChildByName( "TerScoreValue" ));
	m_pTimerLabel = dynamic_cast<Label*>(FindChildByName( "timerlabel" ));
	m_pRoundCountLabel = dynamic_cast<Label*>(FindChildByName( "RoundCountLabel" ));
	m_pPlayerPanelName = dynamic_cast<Label*>(FindChildByName( "PlayerPanelName" ));
	m_pPlayerPanelTeam = dynamic_cast<ImagePanel*>(FindChildByName( "PlayerPanelTeam" ));
	m_pPlayerPanelBkg = dynamic_cast<ImagePanel*>(FindChildByName( "PlayerPanelBkg" ));
	m_pPlayerPanelAvatar = dynamic_cast<CAvatarImagePanel*>(FindChildByName( "PlayerPanelAvatar" ));
	m_pPlayerPanelAvatarBkg = dynamic_cast<Panel*>(FindChildByName( "PlayerPanelAvatarBkg" ));
	m_pPlayerPanelBorderUpper = dynamic_cast<ImagePanel*>(FindChildByName( "PlayerPanelBorderUpper" ));
	m_pBombIcon = dynamic_cast<VectorImagePanel*>(FindChildByName( "BombIcon" ));

	m_pPlayerPanelAvatar->SetDefaultAvatar( scheme()->GetImage( CSTRIKE_DEFAULT_AVATAR, true ) );
	m_pPlayerPanelAvatar->SetShouldScaleImage( true );
	m_pPlayerPanelAvatar->SetShouldDrawFriendIcon( false );
}

//-----------------------------------------------------------------------------
// Purpose: Updates information about teams
//-----------------------------------------------------------------------------
void CCSSpectatorGUI::UpdateTeamInfo()
{
	// these could be NULL if players modified the UI
	if ( !ControlsPresent() )
		return;

    // update the team sections in the scoreboard
    for ( int teamIndex = TEAM_TERRORIST; teamIndex <= TEAM_CT; teamIndex++ )
    {
		wchar_t teamName[512];
        C_CSTeam *team = GetGlobalCSTeam( teamIndex );
        if ( team )
        {
            // choose dialog variables to set depending on team
            const char *pDialogVarTeamName = NULL;
			const char *pDialogVarTeamScore = NULL;
            switch ( teamIndex )
            {
            case TEAM_TERRORIST:
				V_wcsncpy( teamName, g_pVGuiLocalize->Find( "#Cstrike_Team_T_Upper" ), sizeof( teamName ) );
                pDialogVarTeamName = "t_teamname";
				pDialogVarTeamScore = "t_totalteamscore";
                break;
			case TEAM_CT:
				V_wcsncpy( teamName, g_pVGuiLocalize->Find( "#Cstrike_Team_CT_Upper" ), sizeof( teamName ) );
                pDialogVarTeamName = "ct_teamname";
				pDialogVarTeamScore = "ct_totalteamscore";
                break;
            default:
                Assert( false );
                break;
            }

			if ( !StringIsEmpty( team->Get_ClanName() ) )
			{
				g_pVGuiLocalize->ConstructString( teamName, sizeof( teamName ), team->Get_ClanName(), nullptr );
			}

            SetDialogVariable( pDialogVarTeamName, teamName );

			// Team score
            wchar_t wNumScore[16];
            V_snwprintf( wNumScore, ARRAYSIZE( wNumScore ), L"%i", team->Get_Score() );
			SetDialogVariable( pDialogVarTeamScore, wNumScore );
        }
    }
}

//-----------------------------------------------------------------------------
// Purpose: Updates round counter
//-----------------------------------------------------------------------------
void CCSSpectatorGUI::UpdateRoundCounter()
{
	// these could be NULL if players modified the UI
	if ( !ControlsPresent() )
		return;

	if ( !CSGameRules() )
		return;

	wchar_t wszUnicode[64];
	if ( CSGameRules()->GetOvertimePlaying() )
	{
		wchar_t wszRoundCount[16];
		int iRoundNumber = 1 + CSGameRules()->GetTotalRoundsPlayed() - ((CSGameRules()->GetOvertimePlaying() - 1) * mp_overtime_maxrounds.GetInt()) - mp_maxrounds.GetInt();
		V_snwprintf( wszRoundCount, ARRAYSIZE( wszRoundCount ), L"%d/%d", iRoundNumber, mp_overtime_maxrounds.GetInt() );

		wchar_t wszOvertime[4];
		V_snwprintf( wszOvertime, ARRAYSIZE( wszOvertime ), L"%d", CSGameRules()->GetOvertimePlaying() );
		g_pVGuiLocalize->ConstructString( wszUnicode, sizeof( wszUnicode ), g_pVGuiLocalize->Find( "#Cstrike_Spec_RoundsLeftOvertime" ), 2, wszOvertime, wszRoundCount );
	}
	else
	{
		wchar_t wszRoundCount[16];
		V_snwprintf( wszRoundCount, ARRAYSIZE( wszRoundCount ), L"%d/%d", CSGameRules()->GetTotalRoundsPlayed() + 1, mp_maxrounds.GetInt() );
		g_pVGuiLocalize->ConstructString( wszUnicode, sizeof( wszUnicode ), g_pVGuiLocalize->Find( "#Cstrike_Spec_RoundsLeft" ), 1, wszRoundCount );
	}

	m_pRoundCountLabel->SetText( wszUnicode );
}

bool CCSSpectatorGUI::NeedsUpdate( void )
{
	C_CSPlayer *player = C_CSPlayer::GetLocalCSPlayer();
	if ( !player )
		return false;

	if ( m_nLastTime != (int)CSGameRules()->GetRoundRemainingTime() )
		return true;

	if ( m_nLastSpecMode != player->GetObserverMode() )
		return true;

	if ( m_nLastSpecTarget != player->GetObserverTarget() )
		return true;

	return BaseClass::NeedsUpdate();
}

//-----------------------------------------------------------------------------
// Purpose: Updates the timer label if one exists
//-----------------------------------------------------------------------------
void CCSSpectatorGUI::UpdateTimer()
{
	// these could be NULL if players modified the UI
	if ( !ControlsPresent() )
		return;

	bool bBombPlanted = (g_PlantedC4s.Count() > 0);
	if ( bBombPlanted )
	{
		C_PlantedC4 *pC4 = g_PlantedC4s[0];

		if ( pC4->m_bBombDefused )
		{
			m_pBombIcon->SetAlpha( 255 );
			m_pBombIcon->SetFgColor( m_clrC4Defused );
			m_pBombIcon->SetVisible( true );
		}
		else
		{
			int alpha = 255;
			if ( gpGlobals->curtime + 0.1f >= pC4->m_flNextGlow )
				alpha = 128;

			m_pBombIcon->SetAlpha( alpha );
			m_pBombIcon->SetFgColor( m_clrC4Planted );
			m_pBombIcon->SetVisible( !pC4->m_bExplodeWarning );
		}
		m_pTimerLabel->SetText( L" " );
	}
	else
	{
		m_pBombIcon->SetVisible( false );

		m_nLastTime = (int) (CSGameRules()->GetRoundRemainingTime());

		if ( m_nLastTime < 0 )
			m_nLastTime = 0;

		wchar_t szText[16];
		int iMinutes = m_nLastTime / 60;
		int iSeconds = m_nLastTime % 60;

		V_snwprintf( szText, ARRAYSIZE( szText ), L"%d : %.2d", iMinutes, iSeconds );
		m_pTimerLabel->SetText( szText );
	}
}

void CCSSpectatorGUI::Update()
{
	BaseClass::Update();
	
	C_BasePlayer *pLocalPlayer = C_BasePlayer::GetLocalPlayer();
	if ( !pLocalPlayer )
		return;

	C_CS_PlayerResource *cs_PR = dynamic_cast<C_CS_PlayerResource *>(g_PR);
	if ( !cs_PR )
		return;

	m_nLastSpecMode = pLocalPlayer->GetObserverMode();

	if ( m_nLastSpecTarget != pLocalPlayer->GetObserverTarget() )
	{
		m_nLastSpecTarget = pLocalPlayer->GetObserverTarget();

		if ( m_nLastSpecTarget && m_nLastSpecTarget->IsPlayer() && m_nLastSpecMode > OBS_MODE_FREEZECAM )
		{
			m_pPlayerPanelAvatar->ClearAvatar();
			C_CSPlayer *pSpecTarget = ToCSPlayer( m_nLastSpecTarget );
			if ( pSpecTarget ) // shouldn't even be possible but just to be sure
			{
				m_pPlayerPanelAvatar->SetDefaultAvatar( GetDefaultAvatarImage( pSpecTarget ) );
				m_pPlayerPanelAvatar->SetPlayer( pSpecTarget, k_EAvatarSize64x64 );
				m_pPlayerPanelAvatar->SetVisible( true );
				m_pPlayerPanelAvatarBkg->SetVisible( true );

				wchar_t wszSpecTargetName[MAX_DECORATED_PLAYER_NAME_LENGTH];
				wszSpecTargetName[0] = '\0';
				cs_PR->GetDecoratedPlayerName( pSpecTarget->entindex(), wszSpecTargetName, sizeof( wszSpecTargetName ), k_EDecoratedPlayerNameFlag_AddBotToNameIfControllingBot );
				m_pPlayerPanelName->SetVisible( true );
				m_pPlayerPanelName->SetText( wszSpecTargetName );

				m_pPlayerPanelBkg->SetVisible( true );
				m_pPlayerPanelTeam->SetVisible( true );
				m_pPlayerPanelBorderUpper->SetVisible( true );
				m_pPlayerPanelTeam->SetDrawColor( m_nLastSpecTarget->GetTeamNumber() == TEAM_CT ? m_pCTStripColor : m_pTStripColor );
			}
		}
		else
		{
			m_pPlayerPanelName->SetVisible( false );
			m_pPlayerPanelBkg->SetVisible( false );
			m_pPlayerPanelTeam->SetVisible( false );
			m_pPlayerPanelAvatar->SetVisible( false );
			m_pPlayerPanelAvatarBkg->SetVisible( false );
			m_pPlayerPanelBorderUpper->SetVisible( false );
		}
	}

	UpdateTimer();
	UpdateTeamInfo();
	UpdateRoundCounter();
}

//-----------------------------------------------------------------------------
// Purpose: Verify controls are present to resize
//-----------------------------------------------------------------------------
bool CCSSpectatorGUI::ControlsPresent( void ) const
{
	return ( m_pCTLabel != NULL &&
			 m_pCTScore != NULL &&
			 m_pTerLabel != NULL &&
			 m_pTerScore != NULL &&
			 m_pTimerLabel != NULL &&
			 m_pRoundCountLabel != NULL &&
			 m_pPlayerPanelName != NULL &&
			 m_pPlayerPanelBkg != NULL &&
			 m_pPlayerPanelTeam != NULL &&
			 m_pPlayerPanelAvatar != NULL && 
			 m_pPlayerPanelAvatarBkg != NULL &&
			 m_pBombIcon != NULL );
}



//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static int AdjustValue( int curValue, int targetValue, int amount )
{
	if ( curValue > targetValue )
	{
		curValue -= amount;

		if ( curValue < targetValue )
			curValue = targetValue;
	}
	else if ( curValue < targetValue )
	{
		curValue += amount;

		if ( curValue > targetValue )
			curValue = targetValue;
	}

	return curValue;
}

void CCSMapOverview::InitTeamColorsAndIcons()
{
	BaseClass::InitTeamColorsAndIcons();

	Q_memset( m_TeamIconsSelf, 0, sizeof(m_TeamIconsSelf) );
	Q_memset( m_TeamIconsDead, 0, sizeof(m_TeamIconsDead) );
	Q_memset( m_TeamIconsOffscreen, 0, sizeof(m_TeamIconsOffscreen) );
	Q_memset( m_TeamIconsGhost, 0, sizeof( m_TeamIconsGhost ) );
	Q_memset( m_TeamIconsBomb, 0, sizeof( m_TeamIconsBomb ) );

	m_bombRingPlanted = -1;
	m_bombRingDropped = -1;
	m_radioFlash = -1;
	m_radioFlashOffscreen = -1;
	m_radarTint = -1;
	m_hostageFollowing = -1;
	m_hostageFollowingOffscreen = -1;
	m_playerFacing = -1;
	m_cameraIconFirst = -1;
	m_cameraIconThird = -1;
	m_cameraIconFree = -1;
	m_hostageRescueIcon = -1;
	m_bombSiteIconA = -1;
	m_bombSiteIconB = -1;


	//setup team red
	m_TeamColors[MAP_ICON_T] = COLOR_RED;
	m_TeamIcons[MAP_ICON_T] = AddIconTexture( "sprites/player_red_small" );
	m_TeamIconsSelf[MAP_ICON_T] = AddIconTexture( "sprites/player_red_self" );
	m_TeamIconsDead[MAP_ICON_T] = AddIconTexture( "sprites/player_red_dead" );
	m_TeamIconsOffscreen[MAP_ICON_T] = AddIconTexture( "sprites/player_red_offscreen" );
	m_TeamIconsGhost[MAP_ICON_T] = AddIconTexture( "sprites/player_red_ghost" );
	m_TeamIconsBomb[MAP_ICON_T] = AddIconTexture( "sprites/player_red_bomb" );

	// setup team blue
	m_TeamColors[MAP_ICON_CT] = COLOR_BLUE;
	m_TeamIcons[MAP_ICON_CT] = AddIconTexture( "sprites/player_blue_small" );
	m_TeamIconsSelf[MAP_ICON_CT] = AddIconTexture( "sprites/player_blue_self" );
	m_TeamIconsDead[MAP_ICON_CT] = AddIconTexture( "sprites/player_blue_dead" );
	m_TeamIconsOffscreen[MAP_ICON_CT] = AddIconTexture( "sprites/player_blue_offscreen" );
	m_TeamIconsGhost[MAP_ICON_CT] = AddIconTexture( "sprites/player_blue_ghost" );
	m_TeamIconsBomb[MAP_ICON_CT] = AddIconTexture( "sprites/player_blue_bomb" );

	// setup team other
	m_TeamColors[MAP_ICON_HOSTAGE] = COLOR_GREY;
	m_TeamIcons[MAP_ICON_HOSTAGE] = AddIconTexture( "sprites/player_hostage_small" );
	m_TeamIconsSelf[MAP_ICON_HOSTAGE] = -1;
	m_TeamIconsDead[MAP_ICON_HOSTAGE] = AddIconTexture( "sprites/player_hostage_dead" );
	m_TeamIconsOffscreen[MAP_ICON_HOSTAGE] = AddIconTexture( "sprites/player_hostage_offscreen" );
	m_TeamIconsGhost[MAP_ICON_HOSTAGE] = AddIconTexture( "sprites/player_hostage_ghost" );
	m_TeamIconsBomb[MAP_ICON_HOSTAGE] = -1;

	m_bombRingPlanted = AddIconTexture( "sprites/bomb_planted_ring" );
	m_bombRingDropped = AddIconTexture( "sprites/bomb_dropped_ring" );

	m_hostageFollowing = AddIconTexture( "sprites/hostage_following" );
	m_hostageFollowingOffscreen = AddIconTexture( "sprites/hostage_following_offscreen" );
	m_playerFacing = AddIconTexture( "sprites/player_tick" );
	m_cameraIconFirst = AddIconTexture( "sprites/spectator_eye" );
	m_cameraIconThird = AddIconTexture( "sprites/spectator_3rdcam" );
	m_cameraIconFree = AddIconTexture( "sprites/spectator_freecam" );
	m_hostageRescueIcon = AddIconTexture( "sprites/objective_rescue" );;
	m_bombSiteIconA = AddIconTexture( "sprites/objective_site_a" );;
	m_bombSiteIconB = AddIconTexture( "sprites/objective_site_b" );;

	m_radioFlash = AddIconTexture("sprites/player_radio_ring");
	m_radioFlashOffscreen = AddIconTexture("sprites/player_radio_ring_offscreen");

	m_radarTint = AddIconTexture("sprites/radar_trans");

	m_enemyIcon = AddIconTexture( "sprites/player_enemy_small" );
	m_enemyIconDead = AddIconTexture( "sprites/player_enemy_dead" );
	m_enemyIconOffscreen = AddIconTexture( "sprites/player_enemy_offscreen" );
	m_enemyIconGhost = AddIconTexture( "sprites/player_enemy_ghost" );
	m_enemyIconBomb = AddIconTexture( "sprites/player_enemy_bomb" );

}

//-----------------------------------------------------------------------------
void CCSMapOverview::ApplySchemeSettings(vgui::IScheme *scheme)
{
	BaseClass::ApplySchemeSettings( scheme );

	m_hIconFont = scheme->GetFont( "DefaultSmall", true );
}

//-----------------------------------------------------------------------------
void CCSMapOverview::ApplySettings(KeyValues *inResourceData)
{
	BaseClass::ApplySettings( inResourceData );

	g_pMatSystemSurface->OverrideProportionalBase( m_iBaseResolutionOverride[0], m_iBaseResolutionOverride[1] );
	m_nBorderSize = scheme()->GetProportionalScaledValue( inResourceData->GetInt( "transparent_border_size" ) );
	g_pMatSystemSurface->RestoreProportionalBase();
}

//-----------------------------------------------------------------------------
void CCSMapOverview::Update( void )
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();

	if ( !pPlayer )
		return;

	bool inRadarMode = (GetMode() == MAP_MODE_RADAR);
	// if alive, we can only be in radar mode
	if( !inRadarMode && pPlayer->IsAlive())
	{
		SetMode( MAP_MODE_RADAR );
		inRadarMode = true;
	}

	SetFollowEntity(pPlayer->entindex());
	UpdatePlayers();

	BaseClass::Update();

	if ( m_vecRadarVerticalSections.Count() )
	{
		float flPlayerZ = 0;
		if ( pPlayer && pPlayer->GetObserverMode() == OBS_MODE_NONE )
		{
			flPlayerZ = pPlayer->GetLocalOrigin().z;
		}
		else 
		{
			flPlayerZ = MainViewOrigin().z;
		}

		FOR_EACH_VEC( m_vecRadarVerticalSections, i )
		{
			HudRadarLevelVerticalSection_t *pRadarSection = &m_vecRadarVerticalSections[i];

			if ( flPlayerZ >= pRadarSection->m_flSectionAltitudeFloor && 
				 flPlayerZ < pRadarSection->m_flSectionAltitudeCeiling && 
				m_nCurrentRadarVerticalSection != pRadarSection->m_nSectionIndex )
			{
				m_nCurrentRadarVerticalSection = pRadarSection->m_nSectionIndex;
				break;
			}
		}
	}
}

//-----------------------------------------------------------------------------
CCSMapOverview::CSMapPlayer_t* CCSMapOverview::GetCSInfoForPlayerIndex( int index )
{
	if ( index < 0 || index >= MAX_PLAYERS )
		return NULL;

	return &m_PlayersCSInfo[ index ];
}

//-----------------------------------------------------------------------------
CCSMapOverview::CSMapPlayer_t* CCSMapOverview::GetCSInfoForPlayer(MapPlayer_t *player)
{
	if( player == NULL )
		return NULL;

	for( int i = 0; i < MAX_PLAYERS; i++ )
	{
		if( &m_Players[i] == player )
			return &m_PlayersCSInfo[i];
	}

	return NULL;
}

//-----------------------------------------------------------------------------
CCSMapOverview::CSMapPlayer_t* CCSMapOverview::GetCSInfoForHostage(MapPlayer_t *hostage)
{
	if( hostage == NULL )
		return NULL;

	for( int i = 0; i < MAX_HOSTAGES; i++ )
	{
		if( &m_Hostages[i] == hostage )
			return &m_HostagesCSInfo[i];
	}

	return NULL;
}

//-----------------------------------------------------------------------------
#define TIME_SPOTS_STAY_SEEN (1.0f)
// rules that define if you can see a player on the overview or not
bool CCSMapOverview::CanPlayerBeSeen( MapPlayer_t *player )
{
	C_CSPlayer *localPlayer = C_CSPlayer::GetLocalCSPlayer();

	if (!localPlayer || !player )
		return false;

	CSMapPlayer_t *csPlayer = GetCSInfoForPlayer(player);
		
	if ( !csPlayer )
		return false;

	if( GetMode() == MAP_MODE_RADAR )
	{
		// This level will be for all the RadarMode thinking.  Base class will be the old way for the other modes.
		float now = gpGlobals->curtime;

		if( player->position == Vector(0,0,0) )
			return false; // Invalid guy.

		// draw special icons if within time
		if ( csPlayer->overrideExpirationTime != -1 && csPlayer->overrideExpirationTime > gpGlobals->curtime )
			return true;

		if ( localPlayer->GetUserID() == player->userid && !localPlayer->IsObserver() )
			return true; // always yes for local player, unless we are spectating

		// otherwise, not dead people
		if( player->health <= 0 )
			return false;

		if ( !IsOtherEnemy( localPlayer->entindex(), player->index+1 ) )
			return true; // always yes for teammates.

		// and a living enemy needs to have been seen recently, and have been for a while
		if( csPlayer->timeLastSeen != -1  
			&& ( now - csPlayer->timeLastSeen < TIME_SPOTS_STAY_SEEN )
			)
			return true;

		return false;
	}
	else if( player->health <= 0 )
	{
		// Have to be under the overriden icon time to draw when dead.
		if ( csPlayer->overrideExpirationTime == -1  ||  csPlayer->overrideExpirationTime <= gpGlobals->curtime )
			return false;
	}
	
	return BaseClass::CanPlayerBeSeen(player);
}

bool CCSMapOverview::CanHostageBeSeen( MapPlayer_t *hostage )
{
	C_BasePlayer *localPlayer = C_BasePlayer::GetLocalPlayer();

	if ( !localPlayer || !hostage )
		return false;

	C_CS_PlayerResource *pCSPR = (C_CS_PlayerResource*)GameResources();
	if ( !pCSPR )
		return false;

	CSMapPlayer_t *csHostage = GetCSInfoForHostage(hostage);

	if ( !csHostage )
		return false;

	if ( hostage->index < 1 )
		return false;

	if( GetMode() == MAP_MODE_RADAR )
	{
		// This level will be for all the RadarMode thinking.  Base class will be the old way for the other modes.
		float now = gpGlobals->curtime;

		if( hostage->position == Vector(0,0,0) )
			return false; // Invalid guy.

		// draw special icons if within time
		if ( csHostage->overrideExpirationTime != -1  &&  csHostage->overrideExpirationTime > gpGlobals->curtime )
			return true;

		// otherwise, not dead people
		if( hostage->health <= 0 )
			return false;

		if ( localPlayer->GetTeamNumber() == TEAM_TERRORIST )
		{
			if ( csHostage->hasHostageBeenCarried )
			{
				// a carried hostage needs to have been seen recently, and have been for a while
				if ( csHostage->timeLastSeen != -1
					 && (now - csHostage->timeLastSeen < TIME_SPOTS_STAY_SEEN)
					 )
					return true;
				else
					return false;
			}
		}

		return true;
	}
	else if( hostage->health <= 0 )
	{
		// Have to be under the overriden icon time to draw when dead.
		if ( csHostage->overrideExpirationTime == -1  ||  csHostage->overrideExpirationTime <= gpGlobals->curtime )
			return false;
	}

	return BaseClass::CanPlayerBeSeen(hostage);
}

CCSMapOverview::CCSMapOverview( const char *pElementName ) : BaseClass( pElementName )
{
	m_nRadarMapTextureID = -1;
	m_nCircleBackgroundTextureID = -1;
	m_nCircleOverlayTextureID = -1;
	m_nSquareOverlayTextureID = -1;

	g_pMapOverview = this;  // for cvars access etc

	m_nCurrentRadarVerticalSection = -1;
	m_vecRadarVerticalSections.RemoveAll();

	m_bRoundRadar = true;
}

void CCSMapOverview::Init( void )
{
	BaseClass::Init();

	if ( m_nCircleBackgroundTextureID == -1 )
	{
		m_nCircleBackgroundTextureID = surface()->CreateNewTextureID();
		surface()->DrawSetTextureFile( m_nCircleBackgroundTextureID, "vgui/hud/circle_radar_background", true, false );
	}
	if ( m_nCircleOverlayTextureID == -1 )
	{
		m_nCircleOverlayTextureID = surface()->CreateNewTextureID();
		surface()->DrawSetTextureFile( m_nCircleOverlayTextureID, "vgui/hud/circle_radar_overlay", true, false );
	}
	if ( m_nSquareOverlayTextureID == -1 )
	{
		m_nSquareOverlayTextureID = surface()->CreateNewTextureID();
		surface()->DrawSetTextureFile( m_nSquareOverlayTextureID, "vgui/hud/square_radar_overlay", true, false );
	}

	// register for events as client listener
	ListenForGameEvent( "hostage_killed" );
	ListenForGameEvent( "hostage_rescued" );
	ListenForGameEvent( "hostage_follows" );
	ListenForGameEvent( "bomb_defused" );
	ListenForGameEvent( "bomb_exploded" );
	ListenForGameEvent( "bot_takeover" );

	HOOK_HUD_MESSAGE( CCSMapOverview, UpdateRadar );
}

CCSMapOverview::~CCSMapOverview()
{
	g_pMapOverview = NULL;

	//TODO release Textures ? clear lists
}

void CCSMapOverview::UpdateFollowEntity()
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if ( !pPlayer )
		return;

	if ( m_bRoundRadar || (cl_radar_square.GetInt() == 1 && pPlayer->IsAlive()) ) // if the radar is round or square but not in scoreboard mode
	{
		if ( m_nFollowEntity != 0 )
		{
			C_BaseEntity *ent = ClientEntityList().GetEnt( m_nFollowEntity );

			if ( ent )
			{
				Vector position = MainViewOrigin();	// Use MainViewOrigin so SourceTV works in 3rd person
				QAngle angle = ent->EyeAngles();

				if ( m_nFollowEntity <= MAX_PLAYERS )
				{
					SetPlayerPositions( m_nFollowEntity - 1, position, angle );
				}

				SetCenter( WorldToMap( position ) );
				SetAngle( angle[YAW] );
			}
		}
	}
	else
	{
		if ( m_nMapTextureID > 0 || m_nRadarMapTextureID > 0 )
			SetCenter( Vector2D( OVERVIEW_MAP_SIZE / 2, OVERVIEW_MAP_SIZE / 2 ) );
		else
			SetCenter( vec3_origin.AsVector2D() ); // if there's no radar texture, set center at 0 0 so that the radar shows up correctly
		SetAngle( 0 );
	}
}

void CCSMapOverview::UpdatePlayers()
{
	if( !m_goalIconsLoaded )
		UpdateGoalIcons();

	UpdateHostages();// Update before players so players can spot them

	UpdateBomb();// Before players so player can properly spot where it is in this update

	UpdateFlashes();

	C_CS_PlayerResource *pCSPR = (C_CS_PlayerResource*)GameResources();
	if ( !pCSPR )
		return;

	float now = gpGlobals->curtime;

	C_CSPlayer *localPlayer = C_CSPlayer::GetLocalCSPlayer();
	if( localPlayer == NULL )
		return;

	MapPlayer_t *localMapPlayer = GetPlayerByUserID(localPlayer->GetUserID());
	if( localMapPlayer == NULL )
		return;

	for ( int i = 1; i<= gpGlobals->maxClients; i++)
	{
		MapPlayer_t *player = &m_Players[i-1];
		CSMapPlayer_t *playerCS = GetCSInfoForPlayerIndex(i-1);

		if ( !playerCS )
			continue;

		// update from global player resources
		if ( pCSPR->IsConnected(i) )
		{
			player->health = pCSPR->GetHealth( i );

			if ( !pCSPR->IsAlive( i ) )
			{
				// Safety actually happens after a TKPunish.
				player->health = 0;
				playerCS->isDead = true;
			}

			if ( player->team != pCSPR->GetTeam( i ) )
			{
				player->team = pCSPR->GetTeam( i );

				if( player == localMapPlayer )
					player->icon = m_TeamIconsSelf[ GetIconNumberFromTeamNumber(player->team) ];
				else
					player->icon = m_TeamIcons[ GetIconNumberFromTeamNumber(player->team) ];

				player->color = m_TeamColors[ GetIconNumberFromTeamNumber(player->team) ];
			}
		}

		Vector position = player->position;
		QAngle angles = player->angle;
		C_BasePlayer *pPlayer = UTIL_PlayerByIndex( i );
		if ( pPlayer && !pPlayer->IsDormant() )
		{
			// update position of active players in our PVS
			position = pPlayer->EyePosition();
			angles = pPlayer->EyeAngles();

			SetPlayerPositions( i-1, position, angles );
		}
	}

	if ( GetMode() == MAP_MODE_RADAR )
	{
		// Check for teammates spotting the bomb
		if ( m_bomb.state != CSMapBomb_t::BOMB_INVALID && pCSPR->IsBombSpotted() )
		{
			SetBombSeen( true );
		}

		// Check for teammates spotting enemy players
		for ( int i = 1; i<= gpGlobals->maxClients; ++i )
		{
			MapPlayer_t *baseEnemy = &m_Players[i - 1];
			if ( !pCSPR->IsConnected(i) )
				continue;

			if ( !pCSPR->IsAlive(i) )
				continue;

			if ( !IsOtherEnemy( localPlayer->entindex(), baseEnemy->index+1 ) )
				continue;

			if ( pCSPR->IsPlayerSpotted(i) )
			{
				SetPlayerSeen( i-1 );

				if( baseEnemy->health > 0 )
				{
					// They were just seen, so if they are alive get rid of their "last known" icon
					CSMapPlayer_t *csEnemy = GetCSInfoForPlayerIndex(i-1);

					if ( csEnemy )
					{
						csEnemy->overrideIcon = -1;
						csEnemy->overrideIconOffscreen = -1;
						csEnemy->overridePosition = Vector(0, 0, 0);
						csEnemy->overrideAngle = QAngle(0, 0, 0);
						csEnemy->overrideFadeTime = -1;
						csEnemy->overrideExpirationTime = -1;
					}
				}
			}
		}

		for( int i = 1; i<= gpGlobals->maxClients; i++ )
		{
			MapPlayer_t *player = &m_Players[i-1];
			CSMapPlayer_t *playerCS = GetCSInfoForPlayerIndex(i-1);
			C_BasePlayer *pPlayer = UTIL_PlayerByIndex( i );

			if ( !pPlayer || !playerCS )
				continue;

			float timeSinceLastSeen = now - playerCS->timeLastSeen;
			bool bEnemy = IsOtherEnemy( localPlayer->entindex(), player->index+1 );
			if( timeSinceLastSeen < 0.25f )
				continue;
			if( player->health <= 0 )
				continue;// We don't need to spot dead guys, since they always show
			if ( !bEnemy )
				continue;// We don't need to spot our own guys

			// Now that everyone has had a say on people they can see for us, go through and handle baddies that can no longer be seen.
			if( playerCS->timeLastSeen != now  &&  player->health > 0 )
			{
				// We are not seen now, but if we were seen recently (and for long enough),
				// put up a "last known" icon and clear timelastseen
				// if they are alive.  Death icon is more important, which is why the health check above.
				if( timeSinceLastSeen < TIME_SPOTS_STAY_SEEN && ( playerCS->timeLastSeen != -1 ) )
				{
					if ( bEnemy )
					{
						playerCS->overrideIcon = m_enemyIconGhost;
						playerCS->overrideIconOffscreen = m_enemyIconOffscreen;
					}
					else
					{
						playerCS->overrideIcon = m_TeamIconsGhost[ GetIconNumberFromTeamNumber(player->team) ];
						playerCS->overrideIconOffscreen = m_TeamIconsOffscreen[ GetIconNumberFromTeamNumber(player->team) ];
					}
					playerCS->overridePosition = player->position;
					playerCS->overrideFadeTime = -1;
					playerCS->overrideExpirationTime = now + LAST_SEEN_ICON_DURATION;
					playerCS->overrideFadeTime = now + LAST_SEEN_ICON_FADE;
					playerCS->overrideAngle = player->angle;
					playerCS->timeLastSeen = -1;
					playerCS->timeFirstSeen = -1;
				}
			}
		}
	}
}

void CCSMapOverview::UpdateHostages()
{
	C_CS_PlayerResource *pCSPR = (C_CS_PlayerResource*)GameResources();
	if ( !pCSPR )
		return;

	float now = gpGlobals->curtime;

	C_CSPlayer* localPlayer = C_CSPlayer::GetLocalCSPlayer();
	if ( localPlayer == NULL )
		return;

	for( int i=0; i < MAX_HOSTAGES; i++ )
	{
		if ( pCSPR->GetHostageEntityID( i ) > 0 )
		{
			if ( pCSPR->IsHostageAlive( i ) )
			{
				MapPlayer_t *hostage = GetHostageByEntityID( pCSPR->GetHostageEntityID(i) );
				if( hostage == NULL )
					hostage = &m_Hostages[i];// Don't have entry yet, so need one.  This'll only happen once, at start of map

				CSMapPlayer_t *hostageCS = GetCSInfoForHostage(hostage);

				if ( !hostageCS )
					return;

				bool bSpotted = pCSPR->IsHostageSpotted( i );
				if ( bSpotted )
					SetHostageSeen( hostageCS );

				if( !hostageCS->isDead )
				{
					if ( bSpotted )
					{
						hostageCS->overrideIcon = -1;
						hostageCS->overrideIconOffscreen = -1;
						hostageCS->overridePosition = Vector( 0, 0, 0 );
						hostageCS->overrideAngle = QAngle( 0, 0, 0 );
						hostageCS->overrideFadeTime = -1;
						hostageCS->overrideExpirationTime = -1;
					}
					hostage->index = pCSPR->GetHostageEntityID(i);
					hostage->position = pCSPR->GetHostagePosition( i );
					hostage->health = 100; // Hostages don't have health available from pCSPR.
					hostage->angle = QAngle(0, 0, 0); // No facing, like no health
					hostage->team = TEAM_CT; // CT in terms of who sees them
					hostage->icon = m_TeamIcons[ MAP_ICON_HOSTAGE ]; // But hostage for icon.
					hostage->color = m_TeamColors[ MAP_ICON_HOSTAGE ];
					hostageCS->isHostage = true;

	//				engine->Con_NPrintf( i + 15, "ID:%d Pos:(%.0f,%.0f,%.0f)", hostage->index, hostage->position.x, hostage->position.y, hostage->position.z );
				}
				else
				{
	//				engine->Con_NPrintf( i + 15, "Mostly Dead" );
				}
			}
			else
			{
	//			engine->Con_NPrintf( i + 15, "Dead" );
			}
		}
	}

	for ( int i = 0; i < MAX_HOSTAGES; i++ )
	{
		if ( pCSPR->GetHostageEntityID( i ) > 0 )
		{
			MapPlayer_t* hostage = GetHostageByEntityID( pCSPR->GetHostageEntityID( i ) );
			CSMapPlayer_t* hostageCS = GetCSInfoForHostage( hostage );

			if ( !hostage || !hostageCS )
				continue;

			float timeSinceLastSeen = now - hostageCS->timeLastSeen;
			if ( timeSinceLastSeen < 0.25f )
				continue;
			if ( hostageCS->isDead )
				continue;// We don't need to spot dead guys, since they always show
			if ( !hostageCS->hasHostageBeenCarried )
				continue;// We don't need to spot those that are always showed
			if ( pCSPR->IsHostageFollowingSomeone( i ) )
				continue;// Don't draw a ghost icon for hostages that are carried
			if ( localPlayer->GetTeamNumber() != TEAM_TERRORIST )
				continue;// We don't need to spot our own guys

			// Now that everyone has had a say on people they can see for us, go through and handle baddies that can no longer be seen.
			if ( hostageCS->timeLastSeen != now && hostage->health > 0 )
			{
				// We are not seen now, but if we were seen recently (and for long enough),
				// put up a "last known" icon and clear timelastseen
				// if they are alive.  Death icon is more important, which is why the health check above.
				if ( timeSinceLastSeen < TIME_SPOTS_STAY_SEEN && (hostageCS->timeLastSeen != -1) )
				{
					hostageCS->overrideIcon = m_TeamIconsGhost[MAP_ICON_HOSTAGE];
					hostageCS->overrideIconOffscreen = m_TeamIconsOffscreen[MAP_ICON_HOSTAGE];
					hostageCS->overridePosition = hostage->position;
					hostageCS->overrideFadeTime = -1;
					hostageCS->overrideExpirationTime = now + LAST_SEEN_ICON_DURATION;
					hostageCS->overrideFadeTime = now + LAST_SEEN_ICON_FADE;
					hostageCS->overrideAngle = hostage->angle;
					hostageCS->timeLastSeen = -1;
					hostageCS->timeFirstSeen = -1;
				}
			}
		}
	}
}

void CCSMapOverview::UpdateBomb()
{
	if( m_bomb.state == CSMapBomb_s::BOMB_GONE )
		return;// no more updates until map restart

	float now = gpGlobals->curtime;

	// First, decide if it has been too long since the bomb has been seen to clear visibility timers.
	if( now - m_bomb.timeLastSeen >= TIME_SPOTS_STAY_SEEN  &&  m_bomb.timeFirstSeen != -1 )
	{
		SetBombSeen( false );
	}

	C_CS_PlayerResource *pCSPR = (C_CS_PlayerResource*)GameResources();
	if ( !pCSPR )
		return;

	m_bomb.carrierIndex = -1;
	float biggestRadius = 0, smallestRadius = 0;
	if ( g_PlantedC4s.Count() > 0 )
	{
		// bomb is planted
		C_PlantedC4 *pC4 = g_PlantedC4s[0];

		if( pC4->IsBombActive() )
		{
			m_bomb.position = pC4->GetAbsOrigin();
			m_bomb.state = CSMapBomb_t::BOMB_PLANTED;
			m_bomb.ringTravelTime = 2.0f;
			smallestRadius = m_flIconSize;
			biggestRadius = m_flIconSize * 4.0f;
		}
		else
		{
			// Defused or exploded
			m_bomb.state = CSMapBomb_t::BOMB_GONE;
		}
	}
	else if ( pCSPR->HasC4( 0 ) )
	{
		// bomb dropped 
		Vector pos = pCSPR->GetC4Postion();

		if ( pos.x != 0 || pos.y != 0 || pos.z != 0 )
		{
			m_bomb.position = pos;
			m_bomb.state = CSMapBomb_t::BOMB_DROPPED;
			m_bomb.ringTravelTime = 1.0f;
			smallestRadius = m_flIconSize;
			biggestRadius = m_flIconSize * 2.0f;
		}
		else
		{
			m_bomb.state = CSMapBomb_t::BOMB_INVALID;
			//Not a bomb map.  Man, what a weird system instead of IsBombMap.  If nobody has it, and it isn't on the ground, then it isn't a bomb map.
		}
	}
	else
	{
		for( int i = 1; i<= gpGlobals->maxClients; i++ )
		{
			if( pCSPR->HasC4(i) )
			{
				C_BasePlayer *pPlayer = UTIL_PlayerByIndex( i );
				if( pPlayer == NULL  ||  pPlayer->IsDormant() )
				{
					// Dormant or no player means we are relying on RadarUpdate messages so we can trust the MapOverview position.
					MapPlayer_t *player = &m_Players[i-1];
					m_bomb.position = player->position;
				}
				else
				{
					// Update players is about to put this Real Data in the player sturct, and we don't want the bomb pos lagged one update behind
					m_bomb.position = pPlayer->GetAbsOrigin();
				}

				m_bomb.state = CSMapBomb_t::BOMB_CARRIED;
				m_bomb.ringTravelTime = 0;
				m_bomb.carrierIndex = i-1;
				break;
			}
		}
	}

	int alpha = GetMasterAlpha();

	if( m_bomb.currentRingRadius == m_bomb.maxRingRadius  ||  m_bomb.ringTravelTime == 0 )
	{
		m_bomb.currentRingRadius = smallestRadius;
		m_bomb.maxRingRadius = biggestRadius;
		m_bomb.currentRingAlpha = alpha;
	}
	else
	{
		m_bomb.currentRingRadius += (m_bomb.maxRingRadius - m_flIconSize) * gpGlobals->frametime / m_bomb.ringTravelTime;
		m_bomb.currentRingRadius = MIN( m_bomb.currentRingRadius, m_bomb.maxRingRadius );
		m_bomb.currentRingAlpha = (alpha - 55) * ((m_bomb.maxRingRadius - m_bomb.currentRingRadius) / (m_bomb.maxRingRadius - m_flIconSize)) + 55;
	}
}

bool CCSMapOverview::ShouldDraw( void )
{
	if ( cl_draw_only_deathnotices.GetBool() )
		return false;

	int alpha = GetMasterAlpha();
	if( alpha == 0 )
		return false;// we have been set to fully transparent

	// [smessick] Turn off map display when in freezecam.
	if ( IsInFreezeCam() )
	{
		return false;
	}
	
	if ( sv_disable_radar.GetInt() == 1 )
	{
		return false;
	}
	else if ( sv_disable_radar.GetInt() == 2 )
	{
		if ( CSGameRules() && CSGameRules()->IsWarmupPeriod() )
			return false;
	}

	return BaseClass::ShouldDraw();
}

CCSMapOverview::MapPlayer_t* CCSMapOverview::GetHostageByEntityID( int entityID )
{
	for (int i=0; i<MAX_HOSTAGES; i++)
	{
		if ( m_Hostages[i].index == entityID )
			return &m_Hostages[i];
	}

	return NULL;
}

CCSMapOverview::MapPlayer_t* CCSMapOverview::GetPlayerByEntityID( int entityID )
{
	C_BasePlayer *realPlayer = UTIL_PlayerByIndex(entityID);

	if( realPlayer == NULL )
		return NULL;

	for (int i=0; i<MAX_PLAYERS; i++)
	{
		MapPlayer_t *player = &m_Players[i];

		if ( player->userid == realPlayer->GetUserID() )
			return player;
	}

	return NULL;
}

#define BORDER_WIDTH 4
bool CCSMapOverview::AdjustPointToPanel(Vector2D *pos)
{
	if( pos == NULL )
		return false;

	int mapInset = GetBorderSize();// This gives us the amount inside the panel that the background is drawing.
	if( mapInset != 0 )
		mapInset += BORDER_WIDTH; // And this gives us the border inside the map edge to give us room for offscreen icons.

	int w,t;

	//MapTpPanel has already offset the x and y.  That's why we use 0 for left and top.
	GetSize( w,t );

	bool madeChange = false;

	if ( GetMode() == MAP_MODE_RADAR && m_bRoundRadar )
	{
		float radius = (w - mapInset + BORDER_WIDTH*2) / 2;
		Vector2D newMapPosition = *pos;
		Vector2D center( w / 2, t / 2 ); // center of the radar
		newMapPosition -= center;

		float dist = newMapPosition.LengthSqr();
		if ( dist >= radius*radius )
		{
			newMapPosition *= sqrt( radius*radius / dist );
			pos->Init( newMapPosition.x + center.x, newMapPosition.y + center.y );

			madeChange = true;
		}
	}
	else
	{
		if ( pos->x < mapInset )
		{
			pos->x = mapInset;
			madeChange = true;
		}
		if ( pos->x > w - mapInset )
		{
			pos->x = w - mapInset;
			madeChange = true;
		}
		if ( pos->y < mapInset )
		{
			pos->y = mapInset;
			madeChange = true;
		}
		if ( pos->y > t - mapInset )
		{
			pos->y = t - mapInset;
			madeChange = true;
		}
	}

	return madeChange;
}

#define CIRCLE_SEGMENTS 60 // ideally 360, but with such small vertex positions it isn't noticeable

void CCSMapOverview::PaintBackground()
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if ( !pPlayer )
		return;

	int mapInset = GetBorderSize();
	int pwidth, pheight;
	GetSize( pwidth, pheight );
	if ( GetMode() == MAP_MODE_RADAR )
	{
		if ( m_bRoundRadar )
		{
			// draw a transparent outline first
			surface()->DrawSetColor( 255, 255, 255, cl_radaralpha.GetInt() * 0.25f );
			surface()->DrawSetTexture( m_nCircleBackgroundTextureID );
			surface()->DrawTexturedRect( 0, 0, pwidth, pheight );

			// now draw the actual background
			Vertex_t points[CIRCLE_SEGMENTS];
			float invDelta = 2.0f * M_PI / CIRCLE_SEGMENTS;
			for ( int i = 0; i < CIRCLE_SEGMENTS; ++i )
			{
				float flRadians = i * invDelta;
				float ca = cos( flRadians );
				float sa = sin( flRadians );

				// Rotate it around the circle
				float x = pwidth / 2 + ((pwidth - mapInset) / 2 * ca);
				float y = pheight / 2 + ((pheight - mapInset) / 2 * sa);
				Vector2D position( x, y );

				points[i].m_Position = position;
			}

			g_pMatSystemSurface->DrawSetColor( GetBgColor() );
			g_pMatSystemSurface->DrawFilledPolygon( CIRCLE_SEGMENTS, points );
		}
		else
		{
			if ( cl_radar_square.GetInt() == 1 && pPlayer->IsAlive() ) // always square
			{
				// draw a transparent outline first
				Color clr = GetBgColor();
				surface()->DrawSetColor( clr.r(), clr.g(), clr.b(), cl_radaralpha.GetInt() * 0.25f );
				surface()->DrawFilledRect( 0, 0, pwidth, pheight );

				// now draw the actual background
				surface()->DrawSetColor( GetBgColor() );
				surface()->DrawFilledRect( mapInset, mapInset, pwidth - mapInset, pheight - mapInset );
			}
			else
			{
				surface()->DrawSetColor( GetBgColor() );
				surface()->DrawFilledRect( 0, 0, pwidth, pheight );
			}
		}
	}
}

void CCSMapOverview::DrawMapTexture()
{
	int alpha = GetMasterAlpha();

	if ( alpha == 0 )
		return;

	int textureIDToUse = m_nMapTextureID;
	if( m_nRadarMapTextureID != -1 && GetMode() == MAP_MODE_RADAR )
	{
		textureIDToUse = m_nRadarMapTextureID;
	}

	if ( m_vecRadarVerticalSections.Count() )
	{
		if ( m_vecRadarVerticalSections[m_nCurrentRadarVerticalSection].m_iTextureID != -1 )
			textureIDToUse = m_vecRadarVerticalSections[m_nCurrentRadarVerticalSection].m_iTextureID;
	}

	int mapInset = GetBorderSize();
	int pwidth, pheight; 
	GetSize(pwidth, pheight);

	if ( GetMode() == MAP_MODE_RADAR && m_bRoundRadar )
	{
		if ( textureIDToUse > 0 )
		{
			Vertex_t points[CIRCLE_SEGMENTS];
			float invDelta = 2.0f * M_PI / CIRCLE_SEGMENTS;
			for ( int i = 0; i < CIRCLE_SEGMENTS; ++i )
			{
				float flRadians = i * invDelta;
				float ca = cos( flRadians );
				float sa = sin( flRadians );

				// Rotate it around the circle
				float x = pwidth / 2 + ((pwidth - mapInset) / 2 * ca);
				float y = pheight / 2 + ((pheight - mapInset) / 2 * sa);
				Vector2D position( x, y );
				Vector2D texCoord( PanelToMap( position ) );

				points[i].m_Position = position;
				points[i].m_TexCoord = texCoord / OVERVIEW_MAP_SIZE;
			}

			surface()->DrawSetColor( 255, 255, 255, alpha );
			surface()->DrawSetTexture( textureIDToUse );
			surface()->DrawTexturedPolygon( CIRCLE_SEGMENTS, points );

			// last, draw an overlay texture
			surface()->DrawSetTexture( m_nCircleOverlayTextureID );
			surface()->DrawSetColor( 255, 255, 255, 255 );
			surface()->DrawTexturedRect( 0, 0, pwidth, pheight );
		}
	}
	else
	{
		if ( textureIDToUse > 0 )
		{
			// We are drawing to the whole panel with a little border
			Vector2D panelTL = Vector2D( mapInset, mapInset );
			Vector2D panelTR = Vector2D( pwidth - mapInset, mapInset );
			Vector2D panelBR = Vector2D( pwidth - mapInset, pheight - mapInset );
			Vector2D panelBL = Vector2D( mapInset, pheight - mapInset );

			// So where are those four points on the great big map?
			Vector2D textureTL = PanelToMap( panelTL );// The top left corner of the display is where on the master map?
			textureTL /= OVERVIEW_MAP_SIZE;// Texture Vec2D is 0 to 1
			Vector2D textureTR = PanelToMap( panelTR );
			textureTR /= OVERVIEW_MAP_SIZE;
			Vector2D textureBR = PanelToMap( panelBR );
			textureBR /= OVERVIEW_MAP_SIZE;
			Vector2D textureBL = PanelToMap( panelBL );
			textureBL /= OVERVIEW_MAP_SIZE;

			Vertex_t points[4] =
			{
				// To draw a textured polygon, the first column is where you want to draw (to), and the second is what you want to draw (from).
				// We want to draw to the panel (pulled in for a border), and we want to draw the part of the map texture that should be seen.
				// First column is in panel coords, second column is in 0-1 texture coords
				Vertex_t( panelTL, textureTL ),
				Vertex_t( panelTR, textureTR ),
				Vertex_t( panelBR, textureBR ),
				Vertex_t( panelBL, textureBL )
			};

			surface()->DrawSetColor( 255, 255, 255, alpha );
			surface()->DrawSetTexture( textureIDToUse );
			surface()->DrawTexturedPolygon( 4, points );

			// last, draw an overlay texture
			surface()->DrawSetTexture( m_nSquareOverlayTextureID );
			surface()->DrawSetColor( 255, 255, 255, 255 );
			surface()->DrawTexturedRect( 0, 0, pwidth, pheight );
		}
	}
}

void CCSMapOverview::DrawBomb()
{
    if( m_bomb.state == CSMapBomb_t::BOMB_INVALID ||
		m_bomb.state == CSMapBomb_t::BOMB_CARRIED )
		return;

	C_CSPlayer *localPlayer = C_CSPlayer::GetLocalCSPlayer();
	if( localPlayer == NULL )
		return;
	MapPlayer_t *localMapPlayer = GetPlayerByUserID(localPlayer->GetUserID());
	if( localMapPlayer == NULL )
		return;
	float now = gpGlobals->curtime;

	if( localMapPlayer->team == TEAM_CT )
	{
		// CT's only get to see it if...

		if( localMapPlayer->health <= 0 )
		{
			if ( mp_forcecamera.GetInt() != OBS_ALLOW_ALL )
				return;// They're dead and spectating isn't restricted 
		}
		else if( (m_bomb.timeLastSeen == -1)  
			||  ( now - m_bomb.timeLastSeen >= TIME_SPOTS_STAY_SEEN )
			)
		{
			return;// It's in view
		}
	}
	// else if you aren't CT you can always see it

	bool bDrawRing = ((m_bomb.state != CSMapBomb_t::BOMB_GONE) && (m_bomb.state != CSMapBomb_t::BOMB_CARRIED));

	int bombRing;
	int bombRingOffscreen;
	if ( m_bomb.state == CSMapBomb_t::BOMB_DROPPED )
	{
		bombRing = m_bombRingDropped;
		bombRingOffscreen = m_bombRingDropped;
	}
	else
	{
		bombRing = m_bombRingPlanted;
		bombRingOffscreen = m_bombRingPlanted;
	}

	int bombIcon;
	if ( localPlayer->IsOtherEnemyTeam( TEAM_TERRORIST ) )
		bombIcon = m_enemyIconBomb;
	else
		bombIcon = m_TeamIconsBomb[MAP_ICON_T];

	int alpha = 255;

	if( m_bomb.timeGone != -1  &&  m_bomb.timeFade <= gpGlobals->curtime )
		alpha *= 1 - ( (float)(gpGlobals->curtime - m_bomb.timeFade) / (float)(m_bomb.timeGone - m_bomb.timeFade) );
	
	if( bDrawRing )
		DrawIconCS(bombRing, bombRingOffscreen, m_bomb.position, m_bomb.currentRingRadius, 0, m_bomb.currentRingAlpha);
	DrawIconCS(bombIcon, bombIcon, m_bomb.position, m_flIconSize, 0, alpha);
}

#define ICON_SCALE_FACTOR 0.4f
void CCSMapOverview::DrawIconCS( int textureID, int offscreenTextureID, Vector pos, float scale, float angle, int alpha, bool allowRotation, const char *text, Color *textColor, float status, Color *statusColor )
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if ( !pPlayer )
		return;

	if( GetMode() == MAP_MODE_RADAR  &&  cl_radaralpha.GetInt() == 0 )
		return;

	if( alpha <= 0 )
		return;

	// PiMoN: sometimes local player icon is incorrect
	// and I am not sure how to solve it...
	if ( textureID < 1 )
		return;

	// magic trick to make the icons appear the same on different map scale
	scale *= m_fMapScale;
	scale *= ICON_SCALE_FACTOR;

	Vector2D pospanel = WorldToMap( pos );
	pospanel = MapToPanel( pospanel );

	int idToUse = textureID;
	float angleToUse = angle;

	Vector2D oldPos = pospanel;
	Vector2D adjustment(0,0);
	if( AdjustPointToPanel( &pospanel ) && (m_bRoundRadar || (cl_radar_square.GetInt() == 1 && pPlayer->IsAlive())) )
	{
		if ( offscreenTextureID == -1 )
			return; //Doesn't want to draw if off screen.

		// Move it in to on panel, and change the icon.
		idToUse = offscreenTextureID;
		// And point towards the original spot
		adjustment = Vector2D(pospanel.x - oldPos.x, pospanel.y - oldPos.y);
		QAngle adjustmentAngles;
		Vector adjustment3D(adjustment.x, -adjustment.y, 0); // Y gets flipped in WorldToMap
		VectorAngles(adjustment3D, adjustmentAngles) ;
		if( allowRotation )
		{
			// Some icons don't want to rotate even when off radar
			angleToUse = adjustmentAngles[YAW];

			// And the angle needs to be in world space, not panel space.
			if( m_bFollowAngle )
			{
				angleToUse += m_fViewAngle;
			}
			else 
			{
				angleToUse += 90.0f;
			}
		}

		// Don't draw names for icons that are offscreen (bunches up and looks bad)
		text = NULL;
	}

	int d = GetPixelOffset( scale );

	Vector offset;

	offset.x = -scale;	offset.y = scale;
	VectorYawRotate( offset, angleToUse, offset );
	Vector2D pos1 = WorldToMap( pos + offset );
	Vector2D pos1Panel = MapToPanel(pos1);
	pos1Panel.x += adjustment.x;
	pos1Panel.y += adjustment.y;

	offset.x = scale;	offset.y = scale;
	VectorYawRotate( offset, angleToUse, offset );
	Vector2D pos2 = WorldToMap( pos + offset );
	Vector2D pos2Panel = MapToPanel(pos2);
	pos2Panel.x += adjustment.x;
	pos2Panel.y += adjustment.y;

	offset.x = scale;	offset.y = -scale;
	VectorYawRotate( offset, angleToUse, offset );
	Vector2D pos3 = WorldToMap( pos + offset );
	Vector2D pos3Panel = MapToPanel(pos3);
	pos3Panel.x += adjustment.x;
	pos3Panel.y += adjustment.y;

	offset.x = -scale;	offset.y = -scale;
	VectorYawRotate( offset, angleToUse, offset );
	Vector2D pos4 = WorldToMap( pos + offset );
	Vector2D pos4Panel = MapToPanel(pos4);
	pos4Panel.x += adjustment.x;
	pos4Panel.y += adjustment.y;

	Vertex_t points[4] =
	{
		Vertex_t( pos1Panel, Vector2D(0,0) ),
			Vertex_t( pos2Panel, Vector2D(1,0) ),
			Vertex_t( pos3Panel, Vector2D(1,1) ),
			Vertex_t( pos4Panel, Vector2D(0,1) )
	};

	surface()->DrawSetColor( 255, 255, 255, alpha );
	surface()->DrawSetTexture( idToUse );
	surface()->DrawTexturedPolygon( 4, points );

	pospanel.y += d + 4;

	if ( status >=0.0f  && status <= 1.0f && statusColor )
	{
		// health bar is 50x3 pixels
		surface()->DrawSetColor( 0,0,0,255 );
		surface()->DrawFilledRect( pospanel.x-d, pospanel.y-1, pospanel.x+d, pospanel.y+1 );

		int length = (float)(d*2)*status;
		surface()->DrawSetColor( statusColor->r(), statusColor->g(), statusColor->b(), 255 );
		surface()->DrawFilledRect( pospanel.x-d, pospanel.y-1, pospanel.x-d+length, pospanel.y+1 );

		pospanel.y += 3;
	}

	if ( text && textColor )
	{
		wchar_t iconText[ MAX_PLAYER_NAME_LENGTH*2 ];

		g_pVGuiLocalize->ConvertANSIToUnicode( text, iconText, sizeof( iconText ) );

		int wide, tall;
		surface()->GetTextSize( m_hIconFont, iconText, wide, tall );

		int x = pospanel.x-(wide/2);
		int y = pospanel.y;

		// draw black shadow text
		surface()->DrawSetTextColor( 0, 0, 0, 255 );
		surface()->DrawSetTextPos( x+1, y );
		surface()->DrawPrintText( iconText, wcslen(iconText) );

		// draw name in color 
		surface()->DrawSetTextColor( textColor->r(), textColor->g(), textColor->b(), 255 );
		surface()->DrawSetTextPos( x, y );
		surface()->DrawPrintText( iconText, wcslen(iconText) );
	}
}

void CCSMapOverview::DrawMapPlayers()
{
	DrawGoalIcons();

	C_CS_PlayerResource *pCSPR = (C_CS_PlayerResource*)GameResources();
	surface()->DrawSetTextFont( m_hIconFont );

	Color colorGreen( 0, 255, 0, 255 );	// health bar color
	C_CSPlayer *localPlayer = C_CSPlayer::GetLocalCSPlayer();

	for (int i=0; i < MAX_PLAYERS; i++)
	{
		int alpha = 255;
		MapPlayer_t *player = &m_Players[i];
		CSMapPlayer_t *playerCS = GetCSInfoForPlayerIndex(i);

		if ( !playerCS )
			continue;

		if ( !CanPlayerBeSeen( player ) )
			continue;

		float status = -1;
		const char *name = NULL;

		bool bIsTeammate = (!IsOtherEnemy( localPlayer->entindex(), player->index+1 ));
		if ( m_bShowNames && CanPlayerNameBeSeen( player ) )
			name = player->name;

		if ( m_bShowHealth && CanPlayerHealthBeSeen( player ) )
			status = player->health/100.0f;

		// Now draw them
		if( playerCS->overrideExpirationTime > gpGlobals->curtime )// If dead, an X, if alive, an alpha'd normal icon
		{
			int alphaToUse = alpha;
			if( playerCS->overrideFadeTime != -1 && playerCS->overrideFadeTime <= gpGlobals->curtime )
			{
				// Fade linearly from fade start to disappear
				alphaToUse *= 1 - (float)(gpGlobals->curtime - playerCS->overrideFadeTime) / (float)(playerCS->overrideExpirationTime - playerCS->overrideFadeTime);
			}

			DrawIconCS( playerCS->overrideIcon, playerCS->overrideIconOffscreen, playerCS->overridePosition, m_flIconSize * 1.1f, GetViewAngle(), alphaToUse, true, name, &player->color, -1, &colorGreen );
			if( player->health > 0 && bIsTeammate )
				DrawIconCS( m_playerFacing, -1, playerCS->overridePosition, m_flIconSize * 1.1f, playerCS->overrideAngle[YAW], alphaToUse, true, name, &player->color, status, &colorGreen );
		}
		else
		{
			float sizeForRing = m_flIconSize * 1.4f;
			float sizeForPlayer = m_flIconSize * 1.1f; // The 1.1 is because the player dots are shrunken a little, so their facing pip can have some space to live

			bool showTalkRing = localPlayer && (localPlayer->GetTeamNumber() == player->team || localPlayer->GetTeamNumber() == TEAM_SPECTATOR);

			if( showTalkRing && playerCS->currentFlashAlpha > 0 )// Flash type
			{
				// Make them flash a halo
				DrawIconCS(m_radioFlash, m_radioFlashOffscreen, player->position, sizeForRing, player->angle[YAW], playerCS->currentFlashAlpha);
			}
			else if( showTalkRing && pCSPR->IsAlive( i + 1 ) && GetClientVoiceMgr()->IsPlayerSpeaking( i + 1) ) // Or solid on type
			{
				// Make them show a halo
				DrawIconCS(m_radioFlash, m_radioFlashOffscreen, player->position, sizeForRing, player->angle[YAW], 255);
			}
			
			bool doingLocalPlayer = localPlayer->GetUserID() == player->userid;
			bool doingBomb = (m_bomb.state == CSMapBomb_t::BOMB_CARRIED && m_bomb.carrierIndex == player->index);
			float angleForPlayer = GetViewAngle();

			if( doingLocalPlayer )
			{
				sizeForPlayer *= 16.0f; // The self icon is really big since it has a camera view cone attached.
				angleForPlayer = player->angle[YAW];// And, the self icon now rotates, natch.
			}

			int icon = player->icon;
			if ( doingBomb && !doingLocalPlayer )
			{
				if ( IsOtherEnemy( localPlayer->entindex(), player->index + 1 ) )
					icon = m_enemyIconBomb;
				else
					icon = m_TeamIconsBomb[GetIconNumberFromTeamNumber( player->team )];
			}
			else
			{
				if ( IsOtherEnemy( localPlayer->entindex(), player->index + 1 ) )
					icon = m_enemyIcon;
			}

			int offscreenIcon = m_TeamIconsOffscreen[GetIconNumberFromTeamNumber(player->team)];
			if ( IsOtherEnemy( localPlayer->entindex(), player->index + 1 ) )
				offscreenIcon = m_enemyIconOffscreen;

			DrawIconCS( icon, offscreenIcon, player->position, sizeForPlayer, angleForPlayer, alpha, true, name, &player->color, status, &colorGreen );
			if( !doingLocalPlayer && player->health > 0 && !doingBomb && bIsTeammate )
			{
				// Draw the facing for teammates only.
				DrawIconCS( m_playerFacing, -1, player->position, sizeForPlayer, player->angle[YAW], alpha, true, name, &player->color, status, &colorGreen );
			}
		}
	}

	// After players so it can draw on top
	DrawHostages();
	DrawBomb();
}

void CCSMapOverview::DrawHostages()
{
	C_CS_PlayerResource *pCSPR = (C_CS_PlayerResource*)GameResources();
	if ( !pCSPR )
		return;

	surface()->DrawSetTextFont( m_hIconFont );

	Color colorGreen( 0, 255, 0, 255 );	// health bar color
	CBasePlayer *localPlayer = C_BasePlayer::GetLocalPlayer();

	for (int i=0; i < MAX_HOSTAGES; i++)
	{
		int alpha = 255;
		MapPlayer_t *hostage = GetHostageByEntityID( pCSPR->GetHostageEntityID(i) );
		if( hostage == NULL )
			continue;

		CSMapPlayer_t *hostageCS = GetCSInfoForHostage(hostage);

		if ( !hostageCS )
			continue;

		if ( !CanHostageBeSeen( hostage ) )
		{
//			engine->Con_NPrintf( i + 30, "Can't be seen." );
			continue;
		}

		float status = -1;
		const char *name = NULL;

		if( hostageCS->overrideExpirationTime > gpGlobals->curtime )// If dead, an X, if alive, an alpha'd normal icon
		{
//			engine->Con_NPrintf( i + 30, "ID:%d Override Pos:(%.0f,%.0f,%.0f)", hostage->index, hostageCS->overridePosition.x, hostageCS->overridePosition.y, hostageCS->overridePosition.z );
			int alphaToUse = alpha;
			if( hostageCS->overrideFadeTime != -1 && hostageCS->overrideFadeTime <= gpGlobals->curtime )
			{
				// Fade linearly from fade start to disappear
				alphaToUse *= 1 - (float)(gpGlobals->curtime - hostageCS->overrideFadeTime) / (float)(hostageCS->overrideExpirationTime - hostageCS->overrideFadeTime);
			}

			DrawIconCS( hostageCS->overrideIcon, hostageCS->overrideIconOffscreen, hostageCS->overridePosition, m_flIconSize, hostageCS->overrideAngle[YAW], alphaToUse, true, name, &hostage->color, status, &colorGreen );
		}
		else
		{
			if( localPlayer && localPlayer->GetTeamNumber() == hostage->team && hostageCS->currentFlashAlpha > 0 )
			{
				// Make them flash a halo
				DrawIconCS(m_radioFlash, m_radioFlashOffscreen, hostage->position, m_flIconSize * 1.4f, hostage->angle[YAW], hostageCS->currentFlashAlpha);
			}

//			engine->Con_NPrintf( i + 30, "ID:%d Pos:(%.0f,%.0f,%.0f)", hostage->index, hostage->position.x, hostage->position.y, hostage->position.z );

			if( pCSPR->IsHostageFollowingSomeone( i ) )
			{
				// If they are following a CT, then give them a little extra symbol to show it.
				DrawIconCS( m_hostageFollowing, m_hostageFollowingOffscreen, hostage->position, m_flIconSize, hostage->angle[YAW], alpha );
			}
			else
			{
				DrawIconCS( hostage->icon, m_TeamIconsOffscreen[MAP_ICON_HOSTAGE], hostage->position, m_flIconSize, GetViewAngle(), alpha, true, name, &hostage->color, status, &colorGreen );
			}
		}
	}
}
void CCSMapOverview::SetMap(const char * levelname)
{
	BaseClass::SetMap(levelname);

	int wide, tall;
	surface()->DrawGetTextureSize( m_nMapTextureID, wide, tall );
	if( wide == 0 && tall == 0 )
	{
		m_nMapTextureID = -1;
		m_nRadarMapTextureID = -1;
		return;// No map image, so no radar image
	}

	char radarFileName[MAX_PATH];
	Q_snprintf(radarFileName, MAX_PATH, "%s_radar", m_MapKeyValues->GetString("material"));
	m_nRadarMapTextureID = surface()->CreateNewTextureID();
	surface()->DrawSetTextureFile(m_nRadarMapTextureID, radarFileName, true, false);
	int radarWide = -1;
	int radarTall = -1;
	surface()->DrawGetTextureSize(m_nRadarMapTextureID, radarWide, radarTall);
	bool radarTextureFound = false;
	if( radarWide == wide  &&  radarTall == tall )
	{
		// Unbelievable that these is no failure return from SetTextureFile, and not
		// even a ValidTexture check on the ID.  So I can check if it is different from
		// the original.  It'll be a 32x32 default if not present.
		radarTextureFound = true;
	}

	if( !radarTextureFound )
	{
		if( !CreateRadarImage(m_MapKeyValues->GetString("material"), radarFileName) )
			m_nRadarMapTextureID = -1;
	}

	// make sure its cleared once we load a new map
	if ( m_vecRadarVerticalSections.Count() )
		m_vecRadarVerticalSections.Purge();

	KeyValues* pSections = m_MapKeyValues->FindKey( "verticalsections" );
	if ( pSections )
	{
		int nIndex = 0;
		char szSectionName[MAX_MAP_NAME];
		for ( KeyValues *kSection = pSections->GetFirstSubKey(); kSection != NULL; kSection = kSection->GetNextKey() )
		{
			float flAltMin = kSection->GetFloat( "AltitudeMin", 0.0f );
			float flAltMax = kSection->GetFloat( "AltitudeMax", 0.0f );

			if ( flAltMin < flAltMax )
			{
				HudRadarLevelVerticalSection_t *pNewSection = &m_vecRadarVerticalSections[ m_vecRadarVerticalSections.AddToTail() ];
				
				if ( !V_strcmp( kSection->GetName(), "default" ) || !V_strcmp( kSection->GetName(), "Default" ) ) // don't say anything.
				{
					V_sprintf_safe( szSectionName, "overviews/%s", levelname );
				}
				else
				{
					V_sprintf_safe( szSectionName, "overviews/%s_%s", levelname, kSection->GetName() );
				}

				pNewSection->m_nSectionIndex = nIndex;
				pNewSection->m_flSectionAltitudeFloor = flAltMin;
				pNewSection->m_flSectionAltitudeCeiling = flAltMax;
				pNewSection->m_iTextureID = surface()->CreateNewTextureID();
				surface()->DrawSetTextureFile( pNewSection->m_iTextureID, szSectionName, true, false );

				int sectionWide, sectionTall;
				surface()->DrawGetTextureSize( pNewSection->m_iTextureID, sectionWide, sectionTall );
				if ( sectionWide != wide || sectionTall != tall )
				{
					m_vecRadarVerticalSections.Purge();
					break;
				}

				nIndex++;
			}
			else
			{
				DevWarning( "Radar vertical section is invalid!\n" );
			}
		}

		if ( m_vecRadarVerticalSections.Count() )
		{
			DevMsg( "Loaded radar vertical section keyvalues.\n" );
		}
	}
	else
	{
		// map doesn't have sections, clear any sections from previous map
		// TODO: Perhaps initialize a single giant vertical section that covers the whole map,
		//       to reduce variation in handling radar in downstream code?
		m_vecRadarVerticalSections.Purge();
	}

	ClearGoalIcons();
}

bool CCSMapOverview::CreateRadarImage(const char *mapName, const char * radarFileName)
{
#ifdef GENERATE_RADAR_FILE
	char fullFileName[MAX_PATH];
	Q_snprintf(fullFileName, MAX_PATH, "materials/%s.vtf", mapName);
	char fullRadarFileName[MAX_PATH];
	Q_snprintf(fullRadarFileName, MAX_PATH, "materials/%s.vtf", radarFileName);

	// Not found, so try to make one
	FileHandle_t fp;
	fp = ::filesystem->Open( fullFileName, "rb" );
	if( !fp )
	{
		return false;
	}
	::filesystem->Seek( fp, 0, FILESYSTEM_SEEK_TAIL );
	int srcVTFLength = ::filesystem->Tell( fp );
	::filesystem->Seek( fp, 0, FILESYSTEM_SEEK_HEAD );

	CUtlBuffer buf;
	buf.EnsureCapacity( srcVTFLength );
	int overviewMapBytesRead = ::filesystem->Read( buf.Base(), srcVTFLength, fp );
	::filesystem->Close( fp );

	buf.SeekGet( CUtlBuffer::SEEK_HEAD, 0 );// Need to set these explicitly since ->Read goes straight to memory and skips them.
	buf.SeekPut( CUtlBuffer::SEEK_HEAD, overviewMapBytesRead );

	IVTFTexture *radarTexture = CreateVTFTexture();
	if (radarTexture->Unserialize(buf))
	{
		ImageFormat oldImageFormat = radarTexture->Format();
		radarTexture->ConvertImageFormat(IMAGE_FORMAT_RGBA8888, false);
		unsigned char *imageData = radarTexture->ImageData(0,0,0);
		int size = radarTexture->ComputeTotalSize(); // in bytes!
		unsigned char *pEnd = imageData + size;

		for( ; imageData < pEnd; imageData += 4 )
		{
			imageData[0] = 0; // R
			imageData[2] = 0; // B
		}

		radarTexture->ConvertImageFormat(oldImageFormat, false);

		buf.Clear();
		radarTexture->Serialize(buf);

		fp = ::filesystem->Open(fullRadarFileName, "wb");
		::filesystem->Write(buf.Base(), buf.TellPut(), fp);
		::filesystem->Close(fp);
		DestroyVTFTexture(radarTexture);
		buf.Purge();

		// And need a vmt file to go with it.
		char vmtFilename[MAX_PATH];
		Q_snprintf(vmtFilename, MAX_PATH, "%s", fullRadarFileName);
		char *extension = &vmtFilename[Q_strlen(vmtFilename) - 3];
		*extension++ = 'v';
		*extension++ = 'm';
		*extension++ = 't';
		*extension++ = '\0';
		fp = ::filesystem->Open(vmtFilename, "wt");
		::filesystem->Write("\"UnlitGeneric\"\n", 15, fp);
		::filesystem->Write("{\n", 2, fp);
		::filesystem->Write("\t\"$translucent\" \"1\"\n", 20, fp);
		::filesystem->Write("\t\"$basetexture\" \"", 17, fp);
		::filesystem->Write(radarFileName, Q_strlen(radarFileName), fp);
		::filesystem->Write("\"\n", 2, fp);
		::filesystem->Write("\t\"$vertexalpha\" \"1\"\n", 20, fp);
		::filesystem->Write("\t\"$vertexcolor\" \"1\"\n", 20, fp);
		::filesystem->Write("\t\"$no_fullbright\" \"1\"\n", 22, fp);
		::filesystem->Write("\t\"$ignorez\" \"1\"\n", 16, fp);
		::filesystem->Write("}\n", 2, fp);
		::filesystem->Close(fp);

		m_nRadarMapTextureID = surface()->CreateNewTextureID();
		surface()->DrawSetTextureFile( m_nRadarMapTextureID, radarFileName, true, true);
		return true;
	}
#endif
	return false;
}

void CCSMapOverview::ResetRound()
{
	BaseClass::ResetRound();

	for (int i=0; i<MAX_PLAYERS; i++)
	{
		CSMapPlayer_t *p = &m_PlayersCSInfo[i];

		p->isDead = false;

		p->overrideFadeTime = -1;
		p->overrideExpirationTime = -1;
		p->overrideIcon = -1;
		p->overrideIconOffscreen = -1;
		p->overridePosition = Vector( 0, 0, 0);
		p->overrideAngle = QAngle(0, 0, 0);

		p->timeLastSeen = -1;
		p->timeFirstSeen = -1;
		p->isHostage = false;
		p->hasHostageBeenCarried = false;

		p->flashUntilTime = -1;
		p->nextFlashPeakTime = -1;
		p->currentFlashAlpha = 0;
	}

	for (int i=0; i<MAX_HOSTAGES; i++)
	{
		MapPlayer_t *basep = &m_Hostages[i];
		CSMapPlayer_t *p = &m_HostagesCSInfo[i];

		basep->health = 100;
		Q_memset( basep->trail, 0, sizeof(basep->trail) );
		basep->position = Vector( 0, 0, 0 );
		basep->index = 0;

		p->isDead = false;

		p->overrideFadeTime = -1;
		p->overrideExpirationTime = -1;
		p->overrideIcon = -1;
		p->overrideIconOffscreen = -1;
		p->overridePosition = Vector( 0, 0, 0);
		p->overrideAngle = QAngle(0, 0, 0);

		p->timeLastSeen = -1;
		p->timeFirstSeen = -1;
		p->isHostage = false;
		p->hasHostageBeenCarried = false;

		p->flashUntilTime = -1;
		p->nextFlashPeakTime = -1;
		p->currentFlashAlpha = 0;
	}

	m_bomb.position = Vector(0,0,0);
	m_bomb.state = CSMapBomb_t::BOMB_INVALID;
	m_bomb.timeLastSeen = -1;
	m_bomb.timeFirstSeen = -1;
	m_bomb.timeFade = -1;
	m_bomb.timeGone = -1;

	m_bomb.currentRingRadius = -1;
	m_bomb.currentRingAlpha = -1;
	m_bomb.maxRingRadius = -1;
	m_bomb.ringTravelTime = -1;

	ClearGoalIcons();
}

void CCSMapOverview::DrawCamera()
{
	C_BasePlayer *localPlayer = C_BasePlayer::GetLocalPlayer();

	if (!localPlayer)
		return;

	if( localPlayer->GetObserverMode() == OBS_MODE_ROAMING )
	{
		// Instead of the programmer-art red dot, we'll draw an icon for when our camera is roaming.
		int alpha = 255;
		DrawIconCS(m_cameraIconFree, m_cameraIconFree, localPlayer->GetAbsOrigin(), m_flIconSize * 3.0f, localPlayer->EyeAngles()[YAW], alpha);
	}
	else if( localPlayer->GetObserverMode() == OBS_MODE_IN_EYE )
	{
		if( localPlayer->GetObserverTarget() )
		{
			// Fade it if it is on top of a player dot.  And don't rotate it.
			int alpha = 255 * 0.5f;
			DrawIconCS(m_cameraIconFirst, m_cameraIconFirst, localPlayer->GetObserverTarget()->GetAbsOrigin(), m_flIconSize * 1.5f, GetViewAngle(), alpha);
		}
	}
	else if( localPlayer->GetObserverMode() == OBS_MODE_CHASE )
	{
		if( localPlayer->GetObserverTarget() )
		{
			// Or Draw the third-camera a little bigger. (Needs room to be off the dot being followed)
			int alpha = 255;
			DrawIconCS(m_cameraIconThird, m_cameraIconThird, localPlayer->GetObserverTarget()->GetAbsOrigin(), m_flIconSize * 3.0f, localPlayer->EyeAngles()[YAW], alpha);
		}
	}
}

void CCSMapOverview::FireGameEvent( IGameEvent *event )
{
	const char * type = event->GetName();

	if ( Q_strcmp(type,"hostage_killed") == 0 )
	{
		MapPlayer_t *hostage = GetHostageByEntityID( event->GetInt("hostage") );

//		DevMsg("Hostage id %d just died.\n", event->GetInt("hostage"));

		if ( !hostage )
			return;

		CSMapPlayer_t *hostageCS = GetCSInfoForHostage(hostage);

		if ( !hostageCS )
			return;

		hostage->health = 0;
		hostageCS->isDead = true;
		Q_memset( hostage->trail, 0, sizeof(hostage->trail) ); // clear trails

		hostageCS->overrideIcon = m_TeamIconsDead[MAP_ICON_HOSTAGE];
		hostageCS->overrideIconOffscreen = hostageCS->overrideIcon;
		hostageCS->overridePosition = hostage->position;
		hostageCS->overrideAngle = hostage->angle;
		hostageCS->overrideFadeTime = gpGlobals->curtime + DEATH_ICON_FADE;
		hostageCS->overrideExpirationTime = gpGlobals->curtime + DEATH_ICON_DURATION;
	}
	else if ( Q_strcmp(type,"hostage_rescued") == 0 )
	{
		MapPlayer_t *hostage = GetHostageByEntityID( event->GetInt("hostage") );

//		DevMsg("Hostage id %d just got rescued.\n", event->GetInt("hostage"));

		if ( !hostage )
			return;

		CSMapPlayer_t *hostageCS = GetCSInfoForHostage(hostage);

		if ( !hostageCS )
			return;

		hostage->health = 0;
		hostageCS->isDead = true;
		Q_memset( hostage->trail, 0, sizeof(hostage->trail) ); // clear trails

		hostageCS->overrideIcon = hostage->icon;
		hostageCS->overrideIconOffscreen = -1;
		hostageCS->overridePosition = hostage->position;
		hostageCS->overrideAngle = hostage->angle;
		hostageCS->overrideFadeTime = gpGlobals->curtime;
		hostageCS->overrideExpirationTime = gpGlobals->curtime + HOSTAGE_RESCUE_DURATION;
	}
	else if ( Q_strcmp(type,"hostage_follows") == 0 )
	{
		MapPlayer_t *hostage = GetHostageByEntityID( event->GetInt("hostage") );

//		DevMsg("Hostage id %d just got rescued.\n", event->GetInt("hostage"));

		if ( !hostage )
			return;

		CSMapPlayer_t *hostageCS = GetCSInfoForHostage(hostage);

		if ( !hostageCS )
			return;

		hostageCS->hasHostageBeenCarried = true;
	}
	else if ( Q_strcmp(type,"bomb_defused") == 0 )
	{
		m_bomb.state = CSMapBomb_t::BOMB_GONE;
		m_bomb.timeFade = gpGlobals->curtime;
		m_bomb.timeGone = gpGlobals->curtime + BOMB_FADE_DURATION;
	}
	else if ( Q_strcmp(type,"bomb_exploded") == 0 )
	{
		m_bomb.state = CSMapBomb_t::BOMB_GONE;
		m_bomb.timeFade = gpGlobals->curtime;
		m_bomb.timeGone = gpGlobals->curtime + BOMB_FADE_DURATION;
	}
	else if ( Q_strcmp(type,"player_death") == 0 )
	{
		C_CSPlayer* pLocalPlayer = C_CSPlayer::GetLocalCSPlayer();
		if ( !pLocalPlayer )
			return;

		MapPlayer_t *player = GetPlayerByUserID( event->GetInt("userid") );

		if ( !player )
			return;

		player->health = 0;
		Q_memset( player->trail, 0, sizeof(player->trail) ); // clear trails

		CSMapPlayer_t *playerCS = GetCSInfoForPlayer(player);

		if ( !playerCS )
			return;

		playerCS->isDead = true;
		if ( IsOtherEnemy(pLocalPlayer->entindex(), player->index+1) )
			playerCS->overrideIcon = m_enemyIconDead;
		else
			playerCS->overrideIcon = m_TeamIconsDead[GetIconNumberFromTeamNumber(player->team)];
		playerCS->overrideIconOffscreen = playerCS->overrideIcon;
		playerCS->overridePosition = player->position;
		playerCS->overrideAngle = player->angle;
		playerCS->overrideFadeTime = gpGlobals->curtime + DEATH_ICON_FADE;
		playerCS->overrideExpirationTime = gpGlobals->curtime + DEATH_ICON_DURATION;
	}
	else if ( Q_strcmp(type,"player_spawn") == 0 )
	{
		MapPlayer_t *player = GetPlayerByUserID( event->GetInt("userid") );

		if ( !player )
			return;

		player->health = 0;
		Q_memset( player->trail, 0, sizeof(player->trail) ); // clear trails

		CSMapPlayer_t *playerCS = GetCSInfoForPlayer(player);

		if ( !playerCS )
			return;

		playerCS->isDead = false;

		playerCS->overrideFadeTime = -1;
		playerCS->overrideExpirationTime = -1;
		playerCS->overrideIcon = -1;
		playerCS->overrideIconOffscreen = -1;
		playerCS->overridePosition = Vector( 0, 0, 0 );
		playerCS->overrideAngle = QAngle( 0, 0, 0 );

		playerCS->timeLastSeen = -1;
		playerCS->timeFirstSeen = -1;
		playerCS->isHostage = false;
		playerCS->hasHostageBeenCarried = false;

		playerCS->flashUntilTime = -1;
		playerCS->nextFlashPeakTime = -1;
		playerCS->currentFlashAlpha = 0;
	}
	else if ( Q_strcmp(type,"bot_takeover") == 0 )
	{
		MapPlayer_t *player = GetPlayerByUserID( event->GetInt("botid") );

		if ( !player )
			return;

		player->health = 0;
		Q_memset( player->trail, 0, sizeof(player->trail) ); // clear trails

		CSMapPlayer_t *playerCS = GetCSInfoForPlayer(player);

		if ( !playerCS )
			return;

		playerCS->isDead = false;

		playerCS->overrideFadeTime = -1;
		playerCS->overrideExpirationTime = -1;
		playerCS->overrideIcon = -1;
		playerCS->overrideIconOffscreen = -1;
		playerCS->overridePosition = Vector( 0, 0, 0 );
		playerCS->overrideAngle = QAngle( 0, 0, 0 );

		playerCS->timeLastSeen = -1;
		playerCS->timeFirstSeen = -1;
		playerCS->isHostage = false;
		playerCS->hasHostageBeenCarried = false;

		playerCS->flashUntilTime = -1;
		playerCS->nextFlashPeakTime = -1;
		playerCS->currentFlashAlpha = 0;
	}
	else if ( Q_strcmp(type,"player_team") == 0 )
	{
		MapPlayer_t *player = GetPlayerByUserID( event->GetInt("userid") );

		if ( !player )
			return;

		C_CSPlayer *localPlayer = C_CSPlayer::GetLocalCSPlayer();
		if( localPlayer == NULL )
			return;
		MapPlayer_t *localMapPlayer = GetPlayerByUserID(localPlayer->GetUserID());

		player->team = event->GetInt("team");

		if( player == localMapPlayer )
			player->icon = m_TeamIconsSelf[ GetIconNumberFromTeamNumber(player->team) ];
		else
			player->icon = m_TeamIcons[ GetIconNumberFromTeamNumber(player->team) ];

		player->color = m_TeamColors[ GetIconNumberFromTeamNumber(player->team) ];
	}
	else
	{
		BaseClass::FireGameEvent(event);
	}
}

void CCSMapOverview::SetMode(int mode)
{
	if ( mode == MAP_MODE_RADAR )
	{
		m_fZoom = cl_radar_scale.GetFloat() * (OVERVIEW_MAP_SIZE / DESIRED_RADAR_RESOLUTION);

		if( CBasePlayer::GetLocalPlayer() )
			SetFollowEntity( CBasePlayer::GetLocalPlayer()->entindex() );

		SetPaintBackgroundEnabled( true );
		ShowPanel( true );
	}
	else 
	{
		SetPaintBackgroundEnabled( false );

		float desiredZoom = 1.0f;

		g_pClientMode->GetViewportAnimationController()->RunAnimationCommand( this, "zoom", desiredZoom, 0.0f, 0.2f, vgui::AnimationController::INTERPOLATOR_LINEAR );
	}

	BaseClass::SetMode(mode);
}

void CCSMapOverview::UpdateSizeAndPosition()
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if ( !pPlayer )
		return;

	if ( GetMode() == MAP_MODE_RADAR )
	{
		// Radar type
		int iObserverMode = pPlayer->GetObserverMode();
		if ( engine->IsHLTV() || pPlayer->GetTeamNumber() == TEAM_SPECTATOR ||
			 pPlayer->IsObserver() && iObserverMode > OBS_MODE_DEATHCAM )
		{
			m_bRoundRadar = false;
			m_fZoom = 1.0f; // fit the entire map in a square
		}
		else
		{
			switch ( cl_radar_square.GetInt() )
			{
				case 0: // always round
					m_bRoundRadar = true;
					m_fZoom = cl_radar_scale.GetFloat() * (OVERVIEW_MAP_SIZE / DESIRED_RADAR_RESOLUTION);
					break;
				case 1: // always square
					m_bRoundRadar = false;
					m_fZoom = cl_radar_scale.GetFloat() * (OVERVIEW_MAP_SIZE / DESIRED_RADAR_RESOLUTION);
					break;
				case 2: // square with scoreboard
					IViewPortPanel* panel = gViewPortInterface->FindPanelByName( PANEL_SCOREBOARD );
					bool bScoreboardIsVisible = panel->IsVisible();
					m_bRoundRadar = !bScoreboardIsVisible;
					m_fZoom = bScoreboardIsVisible ? 1.0f : cl_radar_scale.GetFloat() * (OVERVIEW_MAP_SIZE / DESIRED_RADAR_RESOLUTION); // fit the entire map in a square with scoreboard
					break;
			}
		}
	}
}

void CCSMapOverview::SetPlayerSeen( int index )
{
	CSMapPlayer_t *pCS = GetCSInfoForPlayerIndex(index);

	float now = gpGlobals->curtime;

	if( pCS )
	{
		if( pCS->timeLastSeen == -1 )
			pCS->timeFirstSeen = now;

		pCS->timeLastSeen = now;
	}
}

void CCSMapOverview::SetHostageSeen( CSMapPlayer_t *hostage )
{
	float now = gpGlobals->curtime;

	if( hostage )
	{
		if( hostage->timeLastSeen == -1 )
			hostage->timeFirstSeen = now;

		hostage->timeLastSeen = now;
	}
}

void CCSMapOverview::SetBombSeen( bool seen )
{
	if( seen )
	{
		float now = gpGlobals->curtime;

		if( m_bomb.timeLastSeen == -1 )
			m_bomb.timeFirstSeen = now;

		m_bomb.timeLastSeen = now;
	}
	else
	{
		m_bomb.timeFirstSeen = -1;
		m_bomb.timeLastSeen = -1;
	}
}

void CCSMapOverview::FlashEntity( int entityID )
{
	MapPlayer_t *player = GetPlayerByEntityID(entityID);
	if( player == NULL )
		return;

	CSMapPlayer_t *playerCS = GetCSInfoForPlayer(player);

	if ( !playerCS )
		return;

	playerCS->flashUntilTime = gpGlobals->curtime + 2.0f;
	playerCS->currentFlashAlpha = 255;
	playerCS->nextFlashPeakTime = gpGlobals->curtime + 0.5f;
}

void CCSMapOverview::UpdateFlashes()
{
	float now = gpGlobals->curtime;
	for (int i=0; i<MAX_PLAYERS; i++)
	{
		CSMapPlayer_t *playerCS = GetCSInfoForPlayerIndex(i);
		if( playerCS->flashUntilTime <= now )
		{
			// Flashing over.
			playerCS->currentFlashAlpha = 0;
		}
		else
		{
			if( playerCS->nextFlashPeakTime <= now )
			{
				// Time for a peak
				playerCS->currentFlashAlpha = 255;
				playerCS->nextFlashPeakTime = now + 0.5f;
				playerCS->nextFlashPeakTime = MIN( playerCS->nextFlashPeakTime, playerCS->flashUntilTime );
			}
			else
			{
				// Just fade away
				playerCS->currentFlashAlpha -= ((playerCS->currentFlashAlpha * gpGlobals->frametime) / (playerCS->nextFlashPeakTime - now));
				playerCS->currentFlashAlpha = MAX( 0, playerCS->currentFlashAlpha );
			}
		}
	}
}


//-----------------------------------------------------------------------------
int CCSMapOverview::GetIconNumberFromTeamNumber( int teamNumber )
{
	switch(teamNumber) 
	{
	case TEAM_TERRORIST:
		return MAP_ICON_T;

	case TEAM_CT:
		return MAP_ICON_CT;

	default:
		return MAP_ICON_HOSTAGE;
	}
}

//-----------------------------------------------------------------------------
void CCSMapOverview::ClearGoalIcons()
{
	m_goalIcons.RemoveAll();
	m_goalIconsLoaded = false;
}

//-----------------------------------------------------------------------------
void CCSMapOverview::UpdateGoalIcons()
{
	// The goal entities don't exist on the client, so we have to get them from the CS Resource.
	C_CS_PlayerResource *pCSPR = (C_CS_PlayerResource*)GameResources();
	if ( !pCSPR )
		return;

	Vector bombA = pCSPR->GetBombsiteAPosition();
	if( bombA != vec3_origin )
	{
		CSMapGoal_t bombGoalA;
		bombGoalA.position = bombA;
		bombGoalA.iconToUse = m_bombSiteIconA;
		m_goalIcons.AddToTail( bombGoalA );
		m_goalIconsLoaded = true;
	}

	Vector bombB = pCSPR->GetBombsiteBPosition();
	if( bombB != vec3_origin )
	{
		CSMapGoal_t bombGoalB;
		bombGoalB.position = bombB;
		bombGoalB.iconToUse = m_bombSiteIconB;
		m_goalIcons.AddToTail( bombGoalB );
		m_goalIconsLoaded = true;
	}

	for( int rescueIndex = 0; rescueIndex < MAX_HOSTAGE_RESCUES; rescueIndex++ )
	{
		Vector hostageI = pCSPR->GetHostageRescuePosition( rescueIndex );
		if( hostageI != vec3_origin )
		{
			CSMapGoal_t hostageGoalI;
			hostageGoalI.position = hostageI;
			hostageGoalI.iconToUse = m_hostageRescueIcon;
			m_goalIcons.AddToTail( hostageGoalI );
			m_goalIconsLoaded = true;
		}
	}
}

//-----------------------------------------------------------------------------
void CCSMapOverview::DrawGoalIcons()
{
	for( int iconIndex = 0; iconIndex < m_goalIcons.Count(); iconIndex++ )
	{
		// Goal icons are drawn without turning, but with edge adjustment.
		CSMapGoal_t *currentIcon = &(m_goalIcons[iconIndex]);
		int alpha = 255;
		DrawIconCS(currentIcon->iconToUse, currentIcon->iconToUse, currentIcon->position, m_flIconSize, GetViewAngle(), alpha, false);
	}
}

//-----------------------------------------------------------------------------
bool CCSMapOverview::IsRadarLocked() 
{
	return m_bRoundRadar ? !cl_radar_rotate.GetBool() : false;
}

//-----------------------------------------------------------------------------
int CCSMapOverview::GetMasterAlpha( void )
{
	// The master alpha is the alpha that the map wants to draw at.  The background will be at half that, and the icons
	// will always be full.  (The icons fade themselves for functional reasons like seen-recently.)
	int alpha = 255;
	if( GetMode() == MAP_MODE_RADAR )
		alpha = cl_radaralpha.GetInt();
	else
		alpha = overview_alpha.GetFloat() * 255;
	alpha = clamp( alpha, 0, 255 );

	return alpha;
}

//-----------------------------------------------------------------------------
int CCSMapOverview::GetBorderSize( void )
{
	if ( GetMode() == MAP_MODE_OFF )
		return 0;

	if ( !m_bRoundRadar && cl_radar_square.GetInt() != 1 )
		return 0;

	return m_nBorderSize;
}

//-----------------------------------------------------------------------------
Vector2D CCSMapOverview::PanelToMap( const Vector2D &panelPos )
{
	// This is the reversing of baseclass's MapToPanel
	int pwidth, pheight; 
	GetSize(pwidth, pheight);
	float viewAngle = GetViewAngle();
	float fScale = m_fZoom / OVERVIEW_MAP_SIZE;

	Vector offset;
	offset.x = (panelPos.x - (pwidth * 0.5f)) / pheight;
	offset.y = (panelPos.y - (pheight * 0.5f)) / pheight;

	offset.x /= fScale;
	offset.y /= fScale;

	VectorYawRotate( offset, -viewAngle, offset );

	Vector2D mapPos;
	mapPos.x = offset.x + m_MapCenter.x;
	mapPos.y = offset.y + m_MapCenter.y;

	return mapPos;
}

//-----------------------------------------------------------------------------
float CCSMapOverview::GetViewAngle( void )
{
	float viewAngle = m_fViewAngle;
	if ( m_fZoom != 1.0f || m_bRoundRadar ) // if not showing full map in square (with scoreboard)
		viewAngle -= 90.0f;

	if ( !m_bFollowAngle )
	{
		// We don't use fViewAngle.  We just show straight at all times.
		viewAngle = 0.0f;
	}

	return viewAngle;
}

//-----------------------------------------------------------------------------
void CCSMapOverview::MsgFunc_UpdateRadar( bf_read &msg )
{
	int iPlayerEntity = msg.ReadByte();

	//Draw objects on the radar
	//=============================
	C_CSPlayer *pLocalPlayer = C_CSPlayer::GetLocalCSPlayer();

	if ( !pLocalPlayer )
		return;

	C_CS_PlayerResource *pCSPR = (C_CS_PlayerResource*) GameResources();

	if ( !pCSPR )
		return;

	while ( iPlayerEntity > 0 )
	{
		int x = msg.ReadSBitLong( COORD_INTEGER_BITS - 1 ) * 4;
		int y = msg.ReadSBitLong( COORD_INTEGER_BITS - 1 ) * 4;
		int z = msg.ReadSBitLong( COORD_INTEGER_BITS - 1 ) * 4;
		int a = msg.ReadSBitLong( 9 );

		Vector origin( x, y, z );
		QAngle angles( 0, a, 0 );

		SetPlayerPositions( iPlayerEntity - 1, origin, angles );

		iPlayerEntity = msg.ReadByte(); // read index for next player
	}
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// Location text under radar

DECLARE_HUDELEMENT( CHudLocation );

CHudLocation::CHudLocation( const char *pName ) :	vgui::Label( NULL, "HudLocation", "" ), CHudElement( pName )
{
	SetParent( g_pClientMode->GetViewport() );
}

void CHudLocation::Init()
{
	// Make sure we get ticked...
	vgui::ivgui()->AddTickSignal( GetVPanel() );
}

bool CHudLocation::ShouldDraw()
{
	CCSMapOverview *pCSMapOverview = (CCSMapOverview *)GET_HUDELEMENT( CCSMapOverview );

	if( g_pMapOverview && g_pMapOverview->GetMode() == CMapOverview::MAP_MODE_RADAR && pCSMapOverview && pCSMapOverview->ShouldDraw() == true )
		return true;

	return false;
}

void CHudLocation::OnTick()
{
	const char *pszLocation = "";
	C_CSPlayer *pPlayer = C_CSPlayer::GetLocalCSPlayer();
	if ( pPlayer )
	{
		pszLocation = pPlayer->GetLastKnownPlaceName();
	}
	SetText( g_pVGuiLocalize->Find( pszLocation ) );
}
