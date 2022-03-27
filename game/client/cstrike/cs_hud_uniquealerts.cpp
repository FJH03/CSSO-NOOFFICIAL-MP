//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//
//=============================================================================//
//
// implementation of CHudUniqueAlerts class
//
#include "cbase.h"
#include "iclientmode.h"

#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui/ILocalize.h>
#include <vgui_controls/AnimationController.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/EditablePanel.h>
#include "cs_gamerules.h"
#include "c_team.h"
#include "engine/IEngineSound.h"

using namespace vgui;

#include "hudelement.h"
#include "c_cs_player.h"

extern ConVar mp_team_timeout_max;
extern ConVar cl_draw_only_deathnotices;

//-----------------------------------------------------------------------------
// Purpose: Money panel
//-----------------------------------------------------------------------------
class CHudUniqueAlerts: public CHudElement, public EditablePanel
{
	DECLARE_CLASS_SIMPLE( CHudUniqueAlerts, EditablePanel );

public:
	CHudUniqueAlerts( const char *pElementName );
	virtual void LevelInit( void );
	virtual void OnThink();
	virtual void FireGameEvent( IGameEvent *event );
	virtual void OnScreenSizeChanged( int iOldWide, int iOldTall );
	virtual bool ShouldDraw();

	// Display an alert in the 'alert text' area.  If 'oneShot' is true, will automatically hide.
	// One-shot messages always flash on set, otherwise will only flash if the panel is coming from
	// hidden to showing.
	void ShowAlertText( const wchar_t* szMessage, bool oneShot = false, Color clrText = COLOR_WHITE );
	void HideAlertText();
	void ProcessAlertBar();
	void ShowWarmupAlertPanel( void );

private:
	float m_flNextAlertTick;
	bool m_bLastAlertIsOneShot;

	Label *m_pAlertLabel;

	CPanelAnimationVar( Color, m_clrMatchStart, "MatchStartAlert", "White" );
};

DECLARE_HUDELEMENT( CHudUniqueAlerts );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudUniqueAlerts::CHudUniqueAlerts( const char *pElementName ): CHudElement( pElementName ), EditablePanel( NULL, "HudUniqueAlerts" )
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	m_flNextAlertTick = 0.0f;
	m_bLastAlertIsOneShot = false;

	m_pAlertLabel = new Label( this, "AlertLabel", L" " );

	LoadControlSettings( "resource/hud/uniquealerts.res" );

	ListenForGameEvent( "round_start" );
	ListenForGameEvent( "player_spawn" );
	ListenForGameEvent( "round_announce_final" );
	ListenForGameEvent( "round_announce_match_point" );
	ListenForGameEvent( "round_announce_last_round_half" );
	ListenForGameEvent( "round_announce_match_start" );
	ListenForGameEvent( "round_announce_warmup" );
}

void CHudUniqueAlerts::OnScreenSizeChanged( int iOldWide, int iOldTall )
{
 	// reload the .res file so items are rescaled
 	LoadControlSettings( "resource/hud/uniquealerts.res" );
 
 	// force recalculation of some stuff
 	m_flNextAlertTick = -1;
 	m_bLastAlertIsOneShot = false;
}

void CHudUniqueAlerts::LevelInit( void )
{
	m_flNextAlertTick = -1;
}

void CHudUniqueAlerts::FireGameEvent( IGameEvent *event )
{
	const char *type = event->GetName();
	C_CSPlayer *pLocalPlayer = C_CSPlayer::GetLocalCSPlayer();
	if ( !pLocalPlayer )
		return;

	int EventUserID = event->GetInt( "userid", -1 );
	int LocalPlayerID = ( pLocalPlayer != NULL ) ? pLocalPlayer->GetUserID() : -2;

	if ( Q_strcmp( "round_announce_match_start", type ) == 0 )
	{
		ShowAlertText( g_pVGuiLocalize->Find( "#Cstrike_Alert_Match_Start" ), true );
		
		C_RecipientFilter filter;
		filter.AddRecipient( pLocalPlayer );
		C_BaseEntity::EmitSound( filter, SOUND_FROM_WORLD, "Music.Match_Start_Stinger" );
	}
	else if ( Q_strcmp( "round_start", type ) == 0 )
	{
		if ( pLocalPlayer->IsHLTV() )
			HideAlertText();

		m_flNextAlertTick = -1;
	}
	else if (	Q_strcmp( "round_announce_final", type ) == 0 || 
				Q_strcmp( "round_announce_last_round_half", type ) == 0 || 
				Q_strcmp( "round_announce_match_point", type ) == 0 ||
				Q_strcmp( "player_spawn", type ) == 0 )
	{
		if ( Q_strcmp( "round_announce_final", type ) == 0 )
		{
			ShowAlertText( g_pVGuiLocalize->Find( "#Cstrike_Alert_Final_Round" ), true );

			C_RecipientFilter filter;
			filter.AddRecipient( pLocalPlayer );
			C_BaseEntity::EmitSound( filter, SOUND_FROM_WORLD, "Music.Final_Round_Stinger" );
		}
		else if ( Q_strcmp( "round_announce_match_point", type ) == 0 )
		{
			ShowAlertText( g_pVGuiLocalize->Find( "#Cstrike_Alert_Match_Point" ), true );

			C_RecipientFilter filter;
			filter.AddRecipient( pLocalPlayer );
			C_BaseEntity::EmitSound( filter, SOUND_FROM_WORLD, "Music.Match_Point_Stinger" );
		}
		else if ( Q_strcmp( "round_announce_last_round_half", type ) == 0 )
		{
			ShowAlertText( g_pVGuiLocalize->Find( "#Cstrike_Alert_Last_Round_Half" ), true );
		}
		else if ( Q_strcmp( "player_spawn", type ) == 0 && EventUserID == LocalPlayerID )
		{
			HideAlertText();
		}
	}
	else if ( Q_strcmp( "round_announce_warmup", type ) == 0 )
	{
		HideAlertText();
	}
}

void CHudUniqueAlerts::ShowAlertText( const wchar_t *szMsg, bool oneShot, Color clrText )
{
	m_bLastAlertIsOneShot = oneShot;
	m_pAlertLabel->SetFgColor( clrText );
	m_pAlertLabel->SetText( szMsg );
	g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( oneShot ? "AlertShowOneShot" : "AlertShow", false );
}

void CHudUniqueAlerts::HideAlertText()
{
	if ( !m_bLastAlertIsOneShot )
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "AlertHide" );
}

void CHudUniqueAlerts::ProcessAlertBar()
{
	if ( !CSGameRules() )
		return;

	// get the round restart time first
	float flEndTime = CSGameRules()->GetRoundRestartTime() - 0.5f;
	bool bIsRestarting = CSGameRules()->IsGameRestarting();

	if ( bIsRestarting )
	{
		int nLeft = ( int )flEndTime - ( int )gpGlobals->curtime;
		if ( nLeft >= 0 )
		{
			wchar_t szNotice[128] = L"";
			wchar_t wzSecs[16];
			V_swprintf_safe( wzSecs, L"%d", nLeft );

			if ( CSGameRules()->IsWarmupPeriod() )
			{
				if ( nLeft == 0 )
					g_pVGuiLocalize->ConstructString( szNotice, sizeof( szNotice ), g_pVGuiLocalize->Find( "#Cstrike_Alert_Match_Starting" ), 1, wzSecs );
				else
					g_pVGuiLocalize->ConstructString( szNotice, sizeof( szNotice ), g_pVGuiLocalize->Find( "#Cstrike_Alert_Match_Starting_In" ), 1, wzSecs );
			}
			else
			{
				if ( nLeft == 0 )
					g_pVGuiLocalize->ConstructString( szNotice, sizeof( szNotice ), g_pVGuiLocalize->Find( "#Cstrike_Alert_Match_Restarting" ), 1, wzSecs );
				else
					g_pVGuiLocalize->ConstructString( szNotice, sizeof( szNotice ), g_pVGuiLocalize->Find( "#Cstrike_Alert_Match_Restarting_In" ), 1, wzSecs );
			}

			ShowAlertText( szNotice, false, m_clrMatchStart );
		}
	}
	else if ( CSGameRules()->IsWarmupPeriod() )
	{
		ShowWarmupAlertPanel();
	}
	else if ( CSGameRules()->IsFreezePeriod() )// we're paused
	{
		if ( CSGameRules()->IsMatchWaitingForResume() )
		{
			ShowAlertText( g_pVGuiLocalize->Find( "#Cstrike_Alert_Freeze_Pause" ) );
		}
		else
		{
			HideAlertText();
		}
	}
	else
	{
		HideAlertText(); // Is this a good thing to do here?  seems like we might be overriding someone else....
	}
}

void CHudUniqueAlerts::ShowWarmupAlertPanel( void )
{
	C_CSPlayer *pPlayer = C_CSPlayer::GetLocalCSPlayer();
	if ( !pPlayer || !CSGameRules() )
		return;

	if ( !m_bActive )
	{
		return;
	}

	float flEndTime = CSGameRules()->GetWarmupPeriodEndTime() - 0.5f;
	bool bIsRestarting = CSGameRules()->IsGameRestarting();

	int nTimeLeftInSec = (int)flEndTime - (int)gpGlobals->curtime;
	if ( nTimeLeftInSec > 0 )
	{
		int nMinLeft = nTimeLeftInSec / 60;
		int nSecLeft = nTimeLeftInSec - ( nMinLeft * 60 ); 

		wchar_t szNotice[64] = L"";
		wchar_t wzTime[8] = L"";
			
		if ( !CSGameRules()->IsWarmupPeriodPaused() )
		{
			V_swprintf_safe( wzTime, L"%d:%02d", nMinLeft, nSecLeft );
		}

		if ( nTimeLeftInSec <= 5 )
		{
			g_pVGuiLocalize->ConstructString( szNotice, sizeof( szNotice ), g_pVGuiLocalize->Find( "#Cstrike_Alert_Warmup_Period_Ending" ), 1, wzTime );
			ShowAlertText( szNotice );

			if ( !bIsRestarting )
				pPlayer->EmitSound("Alert.WarmupTimeoutBeep");
		}
		else
		{
			// client-side UTIL_HumansInGame
			int nTotalPlayers = 0;
			for ( int i = 1; i <= gpGlobals->maxClients; i++ )
			{
				CCSPlayer* pPlayer = (CCSPlayer*) UTIL_PlayerByIndex( i );
				if ( pPlayer && pPlayer->GetTeamNumber() != TEAM_SPECTATOR && !pPlayer->IsBot() )
					nTotalPlayers++;
			}
			int nNumHumansNeeded = CSGameRules() ? CSGameRules()->GetMinPlayers() : 0;

			if ( nTotalPlayers < nNumHumansNeeded )
				g_pVGuiLocalize->ConstructString( szNotice, sizeof( szNotice ), g_pVGuiLocalize->Find( "#Cstrike_Alert_Waiting_For_Players" ), 1, wzTime );
			else
				g_pVGuiLocalize->ConstructString( szNotice, sizeof( szNotice ), g_pVGuiLocalize->Find( "#Cstrike_Alert_Warmup_Period" ), 1, wzTime );

			ShowAlertText( szNotice );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudUniqueAlerts::OnThink()
{
	if ( m_flNextAlertTick <= gpGlobals->curtime )
	{
		m_flNextAlertTick = gpGlobals->curtime + 1;
		ProcessAlertBar();
	}
}

bool CHudUniqueAlerts::ShouldDraw()
{
	if ( cl_draw_only_deathnotices.GetBool() )
		return false;

	return CHudElement::ShouldDraw();
}
