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
#include <vgui_controls/AnimationController.h>
#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/VectorImagePanel.h>
#include "c_plantedc4.h"

using namespace vgui;

ConVar hud_playercount_pos( "hud_playercount_pos", "0", FCVAR_ARCHIVE, "0 = default (top), 1 = bottom" );
extern ConVar cl_draw_only_deathnotices;


class CHudTeamCounter: public CHudElement, public EditablePanel
{
	DECLARE_CLASS_SIMPLE( CHudTeamCounter, EditablePanel );

public:
	CHudTeamCounter( const char *pElementName );
	virtual void Init( void );
	virtual void ApplySettings( KeyValues *inResourceData );
	virtual void Reset( void );
	virtual bool ShouldDraw();
	virtual void OnThink();
	virtual void OnScreenSizeChanged( int iOldWide, int iOldTall );

private:
	Label				*m_pCTWinCounterLabel;
	Label				*m_pCTAliveCounterLabel;
	Label				*m_pCTAliveTextLabel;
	Label				*m_pTWinCounterLabel;
	Label				*m_pTAliveCounterLabel;
	Label				*m_pTAliveTextLabel;
	Label				*m_pRoundTimerLabel;
	VectorImagePanel	*m_pBombIcon;
	ImagePanel			*m_pCTSkullImage;
	ImagePanel			*m_pTSkullImage;

	int m_iRoundTime;

	int m_iOriginalXPos;
	int m_iOriginalYPos;
	bool m_bIsAtTheBottom;

	CPanelAnimationVar( Color, m_clrC4Planted, "C4PlantedColor", "White" );
	CPanelAnimationVar( Color, m_clrC4Defused, "C4DefusedColor", "White" );
};

DECLARE_HUDELEMENT( CHudTeamCounter );

CHudTeamCounter::CHudTeamCounter( const char *pElementName ): CHudElement( pElementName ), EditablePanel( NULL, "HudTeamCounter" )
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	SetHiddenBits( HIDEHUD_PLAYERDEAD );

	m_pCTWinCounterLabel = new Label( this, "CTWinCounterLabel", "0" );
	m_pCTAliveCounterLabel = new Label( this, "CTAliveCounterLabel", "0" );
	m_pCTAliveTextLabel = new Label( this, "CTAliveTextLabel", "#Cstrike_PlayerCount_Alive" );
	m_pTWinCounterLabel = new Label( this, "TWinCounterLabel", "0" );
	m_pTAliveCounterLabel = new Label( this, "TAliveCounterLabel", "0" );
	m_pTAliveTextLabel = new Label( this, "TAliveTextLabel", "#Cstrike_PlayerCount_Alive" );
	m_pRoundTimerLabel = new Label( this, "RoundTimerLabel", "0:00" );
	m_pBombIcon = new VectorImagePanel( this, "BombIcon" );
	m_pCTSkullImage = new ImagePanel( this, "CTSkullImage" );
	m_pTSkullImage = new ImagePanel( this, "TSkullImage" );

	LoadControlSettings( "resource/hud/teamcounter.res" );
}

void CHudTeamCounter::OnScreenSizeChanged( int iOldWide, int iOldTall )
{
	// reload the .res file so items are rescaled
	LoadControlSettings( "resource/hud/teamcounter.res" );

	// force recalculation of some stuff
	m_bIsAtTheBottom = false;
}

void CHudTeamCounter::Init( void )
{
	m_iRoundTime = 0;

	m_bIsAtTheBottom = false;
}

void CHudTeamCounter::ApplySettings( KeyValues *inResourceData )
{
	BaseClass::ApplySettings( inResourceData );

	GetPos( m_iOriginalXPos, m_iOriginalYPos );
}

void CHudTeamCounter::Reset()
{
	g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "RoundTimerReset" );
}

bool CHudTeamCounter::ShouldDraw()
{
	if ( cl_draw_only_deathnotices.GetBool() )
		return false;

	C_CSPlayer *pPlayer = C_CSPlayer::GetLocalCSPlayer();
	if ( !pPlayer )
		return false;

	if ( pPlayer->IsObserver() )
		return false;

	return CHudElement::ShouldDraw();
}

void CHudTeamCounter::OnThink()
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

	wchar_t unicode[16];
	if ( teamCT )
	{
		V_snwprintf( unicode, ARRAYSIZE( unicode ), L"%d", teamCT->Get_Score() );
		m_pCTWinCounterLabel->SetText( unicode );
	}
	if ( teamT )
	{
		V_snwprintf( unicode, ARRAYSIZE( unicode ), L"%d", teamT->Get_Score() );
		m_pCTWinCounterLabel->SetText( unicode );
	}

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

		V_snwprintf( unicode, ARRAYSIZE( unicode ), L"%d", iCTCounter );
		m_pCTAliveCounterLabel->SetText( unicode );

		V_snwprintf( unicode, ARRAYSIZE( unicode ), L"%d", iTCounter );
		m_pTAliveCounterLabel->SetText( unicode );

		C_CSTeam *team = GetGlobalCSTeam( TEAM_CT );
		if ( team )
		{
			V_snwprintf( unicode, ARRAYSIZE( unicode ), L"%d", team->Get_Score() );
			m_pCTWinCounterLabel->SetText( unicode );
		}
		team = GetGlobalCSTeam( TEAM_TERRORIST );
		if ( team )
		{
			V_snwprintf( unicode, ARRAYSIZE( unicode ), L"%d", team->Get_Score() );
			m_pTWinCounterLabel->SetText( unicode );
		}

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
	}
	else
		m_pBombIcon->SetVisible( false );

	if ( bBombPlanted || pRules->IsTimeOutActive() || pRules->IsWarmupPeriod() )
		m_pRoundTimerLabel->SetText( L" " );
	else
	{
		if ( m_iRoundTime < (int) ceil( pRules->GetRoundRemainingTime() ) )
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "RoundTimerReset" );

		m_iRoundTime = (int) ceil( pRules->GetRoundRemainingTime() );

		if ( pRules->IsFreezePeriod() )
		{
			// in freeze period countdown to round start time
			m_iRoundTime = (int) ceil( pRules->GetRoundStartTime() - gpGlobals->curtime );
		}

		if ( m_iRoundTime < 0 )
			m_iRoundTime = 0;

		if ( m_iRoundTime <= 10 )
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "RoundTimerLow" );

		int iMinutes = m_iRoundTime / 60;
		int iSeconds = m_iRoundTime % 60;

		V_snwprintf( unicode, ARRAYSIZE( unicode ), L"%d : %.2d", iMinutes, iSeconds );
		m_pRoundTimerLabel->SetText( unicode );
	}
}