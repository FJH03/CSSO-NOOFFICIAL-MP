//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "iclientmode.h"
#include "hudelement.h"
#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/AnimationController.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include "c_cs_player.h"
#include "clientmode_csnormal.h"
#include "vgui_borderprogress.h"

extern ConVar cl_draw_only_deathnotices;

class CHudProgressBar : public CHudElement, public vgui::EditablePanel
{
public:
	DECLARE_CLASS_SIMPLE( CHudProgressBar, vgui::EditablePanel );

	CHudProgressBar( const char *name );

	// vgui overrides
	virtual bool ShouldDraw();
	virtual void OnThink();

private:
	vgui::Label*							m_pActionText;
	vgui::Label*							m_pTimerText;
	vgui::ContinuousProgressBarWithBorder*	m_pActionProgress;
	CPanelAnimationVar( Color, m_clrStart, "ColorStart", "255 0 0 255" );
	CPanelAnimationVar( Color, m_clrMiddle, "ColorMiddle", "255 255 0 255" );
	CPanelAnimationVar( Color, m_clrEnd, "ColorEnd", "0 128 0 255" );
};


DECLARE_HUDELEMENT( CHudProgressBar );


CHudProgressBar::CHudProgressBar( const char *name ) :
	vgui::EditablePanel( NULL, "HudProgressBar" ), CHudElement( name )
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	m_pActionText = new vgui::Label( this, "ActionText", L" " );
	m_pTimerText = new vgui::Label( this, "TimerText", L" " );
	m_pActionProgress = new vgui::ContinuousProgressBarWithBorder( this, "ActionProgress" );
	
	SetHiddenBits( HIDEHUD_PLAYERDEAD | HIDEHUD_WEAPONSELECTION );

	LoadControlSettings( "resource/hud/progressbar.res" );
}

bool CHudProgressBar::ShouldDraw()
{
	if ( cl_draw_only_deathnotices.GetBool() )
		return false;

	C_CSPlayer *pPlayer = C_CSPlayer::GetLocalCSPlayer();

	if( pPlayer && pPlayer->GetObserverMode() == OBS_MODE_IN_EYE )
	{
		C_BaseEntity *pTarget = pPlayer->GetObserverTarget();

		if( pTarget && pTarget->IsPlayer() )
		{
			pPlayer = ToCSPlayer( pTarget );

			if( !pPlayer->IsAlive() )
				return false;
		}
		else
			return false;
	}

	if( !pPlayer || pPlayer->m_iProgressBarDuration == 0 || pPlayer->m_lifeState == LIFE_DEAD )
	{
		return false;
	}

	return CHudElement::ShouldDraw();
}

void CHudProgressBar::OnThink()
{
	bool bSpectating = false;
	C_CSPlayer* pPlayer = C_CSPlayer::GetLocalCSPlayer();
	if ( pPlayer && pPlayer->GetObserverMode() == OBS_MODE_IN_EYE )
	{
		C_BaseEntity* pTarget = pPlayer->GetObserverTarget();

		if ( pTarget && pTarget->IsPlayer() )
		{
			pPlayer = ToCSPlayer( pTarget );
			bSpectating = true;
		}
	}

	if ( !pPlayer || pPlayer->m_iProgressBarDuration == 0 || !pPlayer->IsAlive() )
	{
		return;
	}

	if ( bSpectating )
	{
		wchar_t wszLocalized[128];
		wchar_t wszPlayerName[MAX_PLAYER_NAME_LENGTH];
		g_pVGuiLocalize->ConvertANSIToUnicode( pPlayer->GetPlayerName(), wszPlayerName, sizeof( wszPlayerName ) );

		if ( pPlayer->m_bIsGrabbingHostage )
		{
			g_pVGuiLocalize->ConstructString( wszLocalized, sizeof( wszLocalized ), g_pVGuiLocalize->Find( "#Cstrike_Progress_Spec_HostageText" ), 1, wszPlayerName );
		}
		else
		{
			if ( pPlayer->HasDefuser() )
				g_pVGuiLocalize->ConstructString( wszLocalized, sizeof( wszLocalized ), g_pVGuiLocalize->Find( "#Cstrike_Progress_Spec_DefuseText" ), 1, wszPlayerName );
			else
				g_pVGuiLocalize->ConstructString( wszLocalized, sizeof( wszLocalized ), g_pVGuiLocalize->Find( "#Cstrike_Progress_Spec_DefuseText_NoKit" ), 1, wszPlayerName );
		}

		m_pActionText->SetText( wszLocalized );
	}
	else
	{
		if ( pPlayer->m_bIsGrabbingHostage )
		{
			m_pActionText->SetText( "#Cstrike_Progress_HostageText" );
		}
		else
		{
			if ( pPlayer->HasDefuser() )
				m_pActionText->SetText( "#Cstrike_Progress_DefuseText" );
			else
				m_pActionText->SetText( "#Cstrike_Progress_DefuseText_NoKit" );
		}
	}

	float flTimeLeft = (pPlayer->m_flProgressBarStartTime + (float)pPlayer->m_iProgressBarDuration) - pPlayer->m_flSimulationTime;
	flTimeLeft = MAX( flTimeLeft, 0.0f );
	wchar_t wszTimer[16];
	V_snwprintf( wszTimer, sizeof( wszTimer ), L"00:%06.3f", flTimeLeft );
	m_pTimerText->SetText( wszTimer );

	float flPercentage = flTimeLeft / (float)pPlayer->m_iProgressBarDuration;
	m_pActionProgress->SetProgress( flPercentage );

#if 0
	if ( !bVisible )
	{
		float flHalfDuration = (float)pPlayer->m_iProgressBarDuration * 0.5f;
		m_pActionProgress->SetFgColor( m_clrStart );
		g_pClientMode->GetViewportAnimationController()->RunAnimationCommand( m_pActionProgress, "FgColor", m_clrMiddle, 0.0f, flHalfDuration, AnimationController::INTERPOLATOR_LINEAR, 0.0f, true );
		g_pClientMode->GetViewportAnimationController()->RunAnimationCommand( m_pActionProgress, "FgColor", m_clrEnd, flHalfDuration, flHalfDuration, AnimationController::INTERPOLATOR_LINEAR, 0.0f, false );
	}
#else
	float flR;
	float flG;
	float flB;
	if ( flPercentage > 0.5f )
	{
		flR = ((m_clrMiddle.r() - m_clrStart.r()) * (1.0f - ((flPercentage - 0.5f) * 2.0f ))) + m_clrStart.r();
		flG = ((m_clrMiddle.g() - m_clrStart.g()) * (1.0f - ((flPercentage - 0.5f) * 2.0f ))) + m_clrStart.g();
		flB = ((m_clrMiddle.b() - m_clrStart.b()) * (1.0f - ((flPercentage - 0.5f) * 2.0f ))) + m_clrStart.b();
	}
	else
	{
		flR = ((m_clrEnd.r() - m_clrMiddle.r()) * (1.0f - (flPercentage / 0.5f))) + m_clrMiddle.r();
		flG = ((m_clrEnd.g() - m_clrMiddle.g()) * (1.0f - (flPercentage / 0.5f))) + m_clrMiddle.g();
		flB = ((m_clrEnd.b() - m_clrMiddle.b()) * (1.0f - (flPercentage / 0.5f))) + m_clrMiddle.b(); 
	}

	m_pActionProgress->SetFgColor( Color( flR, flG, flB, 255 ) );
#endif
}