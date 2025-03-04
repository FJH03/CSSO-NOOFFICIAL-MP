//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: a small piece of HUD that shows alive counter and win counter for each team
//
// $NoKeywords: $
//
//=============================================================================//

#include "cbase.h"
#include "iclientmode.h"
#include "hudelement.h"
#include "c_cs_player.h"
#include "c_cs_team.h"
#include "c_cs_playerresource.h"
#include "cs_gamerules.h"
#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/ImagePanel.h>

using namespace vgui;

ConVar hud_playercount_pos( "hud_playercount_pos", "0", FCVAR_ARCHIVE, "0 = default (top), 1 = bottom" );


class CHudTeamCounter: public CHudElement, public EditablePanel
{
	DECLARE_CLASS_SIMPLE( CHudTeamCounter, EditablePanel );

public:
	CHudTeamCounter( const char *pElementName );
	virtual void ApplySettings( KeyValues *inResourceData );
	virtual bool ShouldDraw();
	virtual void Think();

private:
	Label	*m_pCTWinCounterLabel;
	Label	*m_pCTAliveCounterLabel;
	Label	*m_pCTAliveTextLabel;
	Label	*m_pTWinCounterLabel;
	Label	*m_pTAliveCounterLabel;
	Label	*m_pTAliveTextLabel;
	Label	*m_pRoundTimerLabel;
	ImagePanel	*m_pCTSkullImage;
	ImagePanel	*m_pTSkullImage;

	int m_iOriginalXPos;
	int m_iOriginalYPos;

	bool m_bIsAtTheBottom;
};

DECLARE_HUDELEMENT( CHudTeamCounter );

CHudTeamCounter::CHudTeamCounter( const char *pElementName ): CHudElement( pElementName ), EditablePanel( NULL, "HudTeamCounter" )
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	SetHiddenBits( HIDEHUD_PLAYERDEAD );

	m_bIsAtTheBottom = false;

	m_pCTWinCounterLabel = new Label( this, "CTWinCounterLabel", "0" );
	m_pCTAliveCounterLabel = new Label( this, "CTAliveCounterLabel", "0" );
	m_pCTAliveTextLabel = new Label( this, "CTAliveTextLabel", "#Cstrike_PlayerCount_Alive" );
	m_pTWinCounterLabel = new Label( this, "TWinCounterLabel", "0" );
	m_pTAliveCounterLabel = new Label( this, "TAliveCounterLabel", "0" );
	m_pTAliveTextLabel = new Label( this, "TAliveTextLabel", "#Cstrike_PlayerCount_Alive" );
	m_pRoundTimerLabel = new Label( this, "RoundTimerLabel", "0:00" );
	m_pCTSkullImage = new ImagePanel( this, "CTSkullImage" );
	m_pTSkullImage = new ImagePanel( this, "TSkullImage" );

	LoadControlSettings( "resource/hud/teamcounter.res" );
}

void CHudTeamCounter::ApplySettings( KeyValues *inResourceData )
{
	BaseClass::ApplySettings( inResourceData );

	GetPos( m_iOriginalXPos, m_iOriginalYPos );
}

bool CHudTeamCounter::ShouldDraw()
{
	C_CSPlayer *pPlayer = C_CSPlayer::GetLocalCSPlayer();
	if ( !pPlayer )
		return false;

	if ( pPlayer->IsObserver() )
		return false;

	return true;
}

void CHudTeamCounter::Think()
{
	if ( m_bIsAtTheBottom != hud_playercount_pos.GetBool() )
	{
		m_bIsAtTheBottom = hud_playercount_pos.GetBool();

		if ( m_bIsAtTheBottom )
		{
			int ypos = ScreenHeight() - m_iOriginalYPos - GetTall(); // inverse its Y pos
			SetPos( m_iOriginalXPos, ypos );
		}
		else
		{
			SetPos( m_iOriginalXPos, m_iOriginalYPos );
		}
	}

	C_CSTeam *teamCT = GetGlobalCSTeam( TEAM_CT );
	C_CSTeam *teamT = GetGlobalCSTeam( TEAM_TERRORIST );
	if ( teamCT )
		m_pCTWinCounterLabel->SetText( UTIL_VarArgs( "%d", teamCT->Get_Score() ) );
	if ( teamT )
		m_pTWinCounterLabel->SetText( UTIL_VarArgs( "%d", teamT->Get_Score() ) );

	if ( g_PR )
	{
		// Count the players on the team.
		int iCTCounter = 0;
		int iTCounter = 0;
		for ( int playerIndex = 1; playerIndex <= MAX_PLAYERS; playerIndex++ )
		{
			if ( g_PR->IsConnected( playerIndex ) && g_PR->IsAlive( playerIndex ) )
			{
				if ( g_PR->GetTeam( playerIndex ) == TEAM_CT )
					iCTCounter++;

				if ( g_PR->GetTeam( playerIndex ) == TEAM_TERRORIST )
					iTCounter++;
			}
		}

		m_pCTAliveCounterLabel->SetText( UTIL_VarArgs( "%d", iCTCounter ) );
		m_pTAliveCounterLabel->SetText( UTIL_VarArgs( "%d", iTCounter ) );

		m_pCTAliveCounterLabel->SetVisible( iCTCounter > 0 );
		m_pCTAliveTextLabel->SetVisible( iCTCounter > 0 );
		m_pTAliveCounterLabel->SetVisible( iTCounter > 0 );
		m_pTAliveTextLabel->SetVisible( iTCounter > 0 );
		m_pCTSkullImage->SetVisible( iCTCounter < 1 );
		m_pTSkullImage->SetVisible( iTCounter < 1 );
	}

	C_CSGameRules *pRules = CSGameRules();
	if ( !pRules )
		return;

	int iTimer = (int) ceil( pRules->GetRoundRemainingTime() );

	if ( pRules->IsWarmupPeriod() && !pRules->IsWarmupPeriodPaused() )
	{
		iTimer = (int) ceil( pRules->GetWarmupRemainingTime() );
	}
	if ( pRules->IsFreezePeriod() )
	{
		// in freeze period countdown to round start time
		iTimer = (int) ceil( pRules->GetRoundStartTime() - gpGlobals->curtime );
	}

	if ( iTimer < 0 )
		iTimer = 0;

	int iMinutes = iTimer / 60;
	int iSeconds = iTimer % 60;
	
	wchar_t unicode[8];
	V_snwprintf( unicode, ARRAYSIZE( unicode ), L"%d : %.2d", iMinutes, iSeconds );
	m_pRoundTimerLabel->SetText( unicode );
}