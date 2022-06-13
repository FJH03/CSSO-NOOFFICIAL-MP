//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//
//=============================================================================//
//
// implementation of CHudHealthArmor class
//
#include "cbase.h"
#include "iclientmode.h"

#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui_controls/AnimationController.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/VectorImagePanel.h>
#include "vgui_borderprogress.h"

using namespace vgui;

#include "hudelement.h"
#include "c_cs_player.h"

#include "convar.h"

extern ConVar cl_hud_healthammo_style;
extern ConVar cl_hud_background_alpha;
extern ConVar cl_hud_color;
extern ConVar cl_draw_only_deathnotices;

namespace vgui
{
	
//-----------------------------------------------------------------------------
// Purpose: Overriding Paint method to allow for correct border rendering
//-----------------------------------------------------------------------------
ContinuousProgressBarWithBorder::ContinuousProgressBarWithBorder( Panel *parent, const char *panelName ): ContinuousProgressBar( parent, panelName )
{
}

void ContinuousProgressBarWithBorder::Paint()
{
	// allow for border
	int x = 1, y = 1;
	int wide, tall;
	GetSize(wide, tall);
	wide -= 2;
	tall -= 2;

	surface()->DrawSetColor( GetFgColor() );

	bool bUsePrev = _prevProgress >= 0.f;
	bool bGain = _progress > _prevProgress;

	switch( m_iProgressDirection )
	{
	case PROGRESS_EAST:
		if ( bUsePrev )
		{
			if ( bGain )
			{
				surface()->DrawFilledRect( x, y, x + (int)( wide * _prevProgress ), y + tall );

				// Delta
				surface()->DrawSetColor( m_colorGain );
				surface()->DrawFilledRect( x + (int)( wide * _prevProgress ), y, x + (int)( wide * _progress ), y + tall );
				break;
			}
			else
			{
				// Delta
				surface()->DrawSetColor( m_colorLoss );
				surface()->DrawFilledRect( x + (int)( wide * _progress ), y, x + (int)( wide * _prevProgress ), y + tall );
			}
		}
		surface()->DrawSetColor( GetFgColor() );
		surface()->DrawFilledRect( x, y, x + (int)( wide * _progress ), y + tall );
		break;

	case PROGRESS_WEST:
		if ( bUsePrev )
		{
			if ( bGain )
			{
				surface()->DrawFilledRect( x + (int)( wide * ( 1.0f - _prevProgress ) ), y, x + wide, y + tall );

				// Delta
				surface()->DrawSetColor( m_colorGain );
				surface()->DrawFilledRect( x + (int)( wide * ( 1.0f - _progress ) ), y, x + (int)( wide * ( 1.0f - _prevProgress ) ), y + tall );
				break;
			}
			else
			{
				// Delta
				surface()->DrawSetColor( m_colorLoss );
				surface()->DrawFilledRect( x + (int)( wide * ( 1.0f - _prevProgress ) ), y, x + (int)( wide * ( 1.0f - _progress ) ), y + tall );
			}
		}
		surface()->DrawSetColor( GetFgColor() );
		surface()->DrawFilledRect( x + (int)( wide * ( 1.0f - _progress ) ), y, x + wide, y + tall );
		break;

	case PROGRESS_NORTH:
		if ( bUsePrev )
		{
			if ( bGain )
			{
				surface()->DrawFilledRect( x, y + (int)( tall * ( 1.0f - _prevProgress ) ), x + wide, y + tall );

				// Delta
				surface()->DrawSetColor( m_colorGain );
				surface()->DrawFilledRect( x, y + (int)( tall * ( 1.0f - _progress ) ), x + wide, y + (int)( tall * ( 1.0f - _prevProgress ) ) );
				break;
			}
			else
			{
				// Delta
				surface()->DrawSetColor( m_colorLoss );
				surface()->DrawFilledRect( x, y + (int)( tall * ( 1.0f - _prevProgress ) ), x + wide, y + (int)( tall * ( 1.0f - _progress ) ) );
			}
		}
		surface()->DrawSetColor( GetFgColor() );
		surface()->DrawFilledRect( x, y + (int)( tall * ( 1.0f - _progress ) ), x + wide, y + tall );
		break;

	case PROGRESS_SOUTH:
		if ( bUsePrev )
		{
			if ( bGain )
			{
				surface()->DrawFilledRect( x, y, x + wide, y + (int)( tall * ( 1.0f - _progress ) ) );

				// Delta
				surface()->DrawSetColor( m_colorGain );
				surface()->DrawFilledRect( x, y + (int)( tall * ( 1.0f - _progress ) ), x + wide, y + (int)( tall * ( 1.0f - _prevProgress ) ) );
				break;
			}
			else
			{
				// Delta
				surface()->DrawSetColor( m_colorLoss );
				surface()->DrawFilledRect( x, y + (int)( tall * ( 1.0f - _prevProgress ) ), x + wide, y + (int)( tall * ( 1.0f - _progress  ) ) );
			}
		}
		surface()->DrawSetColor( GetFgColor() );
		surface()->DrawFilledRect( x, y, x + wide, y + (int)( tall * _progress ) );
		break;
	}
}

}

//-----------------------------------------------------------------------------
// Purpose: Health panel
//-----------------------------------------------------------------------------
class CHudHealthArmor : public CHudElement, public EditablePanel
{
	DECLARE_CLASS_SIMPLE( CHudHealthArmor, EditablePanel );

public:
	CHudHealthArmor( const char *pElementName );
	virtual void Init( void );
	virtual void ApplySettings( KeyValues *inResourceData );
	virtual void Reset( void );
	virtual void OnThink();
	virtual void OnScreenSizeChanged( int iOldWide, int iOldTall );
	virtual bool ShouldDraw();

private:
	int		m_iHealth;
	int		m_iArmor;

	VectorImagePanel	*m_pHealthIcon;
	VectorImagePanel	*m_pArmorIcon;
	VectorImagePanel	*m_pHelmetIcon;
	// TODO: if adding heavy armor, it has a separate, bigger armor icon

	Label		*m_pHealthLabel;
	Label		*m_pArmorLabel;
	Label		*m_pSimpleArmorLabel;

	ContinuousProgressBarWithBorder	*m_pHealthProgress;
	ContinuousProgressBarWithBorder	*m_pArmorProgress;

	CPanelAnimationVarAliasType( int, simple_wide, "simple_wide", "0", "proportional_width" );
	CPanelAnimationVarAliasType( int, simple_tall, "simple_tall", "0", "proportional_height" );

	CPanelAnimationVarAliasType( int, health_icon_xpos, "health_icon_xpos", "0", "proportional_xpos" );
	CPanelAnimationVarAliasType( int, health_icon_ypos, "health_icon_ypos", "0", "proportional_ypos" );
	CPanelAnimationVarAliasType( int, armor_icon_xpos, "armor_icon_xpos", "0", "proportional_xpos" );
	CPanelAnimationVarAliasType( int, armor_icon_ypos, "armor_icon_ypos", "0", "proportional_ypos" );

	CPanelAnimationVarAliasType( int, simple_health_icon_xpos, "simple_health_icon_xpos", "0", "proportional_xpos" );
	CPanelAnimationVarAliasType( int, simple_health_icon_ypos, "simple_health_icon_ypos", "0", "proportional_ypos" );
	CPanelAnimationVarAliasType( int, simple_armor_icon_xpos, "simple_armor_icon_xpos", "0", "proportional_xpos" );
	CPanelAnimationVarAliasType( int, simple_armor_icon_ypos, "simple_armor_icon_ypos", "0", "proportional_ypos" );

	int m_iOriginalWide;
	int m_iOriginalTall;
};

DECLARE_HUDELEMENT( CHudHealthArmor );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudHealthArmor::CHudHealthArmor( const char *pElementName ) : CHudElement( pElementName ), EditablePanel(NULL, "HudHealthArmor")
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	SetHiddenBits( HIDEHUD_NOT_OBSERVING_PLAYERS );

	m_iOriginalWide = 0;
	m_iOriginalTall = 0;

	m_pHealthIcon = new VectorImagePanel( this, "HealthIcon" );
	m_pArmorIcon = new VectorImagePanel( this, "ArmorIcon" );
	m_pHelmetIcon = new VectorImagePanel( this, "HelmetIcon" );

	m_pHealthLabel = new Label( this, "HealthLabel", "" );
	m_pArmorLabel = new Label( this, "ArmorLabel", "" );
	m_pSimpleArmorLabel = new Label( this, "SimpleArmorLabel", "" );

	m_pHealthProgress = new ContinuousProgressBarWithBorder( this, "HealthProgress" );
	m_pArmorProgress = new ContinuousProgressBarWithBorder( this, "ArmorProgress" );

	LoadControlSettings( "resource/hud/healtharmor.res" );
}

void CHudHealthArmor::OnScreenSizeChanged( int iOldWide, int iOldTall )
{
	// reload the .res file so items are rescaled
	LoadControlSettings( "resource/hud/healtharmor.res" );

	// force recalculation of some stuff
	m_iHUDColor = -1;
	m_flBackgroundAlpha = 0.0f;
	m_iStyle = -1;
	m_iHealth = -1;
	m_iArmor = -1;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudHealthArmor::Init()
{
	m_iHealth			= -1;
	m_iArmor			= -1;
}

void CHudHealthArmor::ApplySettings( KeyValues *inResourceData )
{
	BaseClass::ApplySettings( inResourceData );

	GetSize( m_iOriginalWide, m_iOriginalTall );
}

//-----------------------------------------------------------------------------
// Purpose: reset health to normal color at round restart
//-----------------------------------------------------------------------------
void CHudHealthArmor::Reset()
{
	// force recomputation of health vgui animation
	m_iHealth = -1;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudHealthArmor::OnThink()
{
	if ( m_iStyle != cl_hud_healthammo_style.GetInt() )
	{
		m_iStyle = cl_hud_healthammo_style.GetInt();

		switch ( m_iStyle )
		{
			case 0: // default
				SetSize( m_iOriginalWide, m_iOriginalTall );

				m_pHealthProgress->SetVisible( true );
				m_pArmorProgress->SetVisible( true );

				m_pArmorLabel->SetVisible( true );
				m_pSimpleArmorLabel->SetVisible( false );

				m_pHealthIcon->SetPos( health_icon_xpos, health_icon_ypos );
				m_pArmorIcon->SetPos( armor_icon_xpos, armor_icon_ypos );
				m_pHelmetIcon->SetPos( armor_icon_xpos, armor_icon_ypos );
				break;

			case 1: // simple
				SetSize( simple_wide, simple_tall );

				m_pHealthProgress->SetVisible( false );
				m_pArmorProgress->SetVisible( false );

				m_pArmorLabel->SetVisible( false );
				m_pSimpleArmorLabel->SetVisible( true );

				m_pHealthIcon->SetPos( simple_health_icon_xpos, simple_health_icon_ypos );
				m_pArmorIcon->SetPos( simple_armor_icon_xpos, simple_armor_icon_ypos );
				m_pHelmetIcon->SetPos( simple_armor_icon_xpos, simple_armor_icon_ypos );
				break;
		}
	}

	if ( m_flBackgroundAlpha != cl_hud_background_alpha.GetFloat() )
	{
		Color newColor = GetBgColor();
		newColor[3] = cl_hud_background_alpha.GetFloat() * 255;
		SetBgColor( newColor );
	}

	if ( m_iHUDColor != cl_hud_color.GetInt() )
	{
		m_iHUDColor = cl_hud_color.GetInt();
		Color clr = gHUD.GetHUDColor( m_iHUDColor );

		m_pHealthIcon->SetFgColor( clr );
		m_pArmorIcon->SetFgColor( clr );
		m_pHealthLabel->SetFgColor( clr );
		m_pArmorLabel->SetFgColor( clr );
		m_pSimpleArmorLabel->SetFgColor( clr );
		m_pHealthProgress->SetFgColor( clr );
		m_pArmorProgress->SetFgColor( clr );
	}

	C_CSPlayer *pPlayer = GetHudPlayer();
	if ( !pPlayer )
	{
		SetPaintEnabled( false );
		SetPaintBackgroundEnabled( false );
		return;
	}
	
	SetPaintEnabled( true );
	SetPaintBackgroundEnabled( true );

	int iRealHealth = 0;
	int iRealArmor = 0;
	int iMaxHealth = pPlayer->GetMaxHealth();
	int iMaxArmor = pPlayer->GetMaxArmor();
	// Never below zero
	iRealHealth = MAX( pPlayer->GetHealth(), 0 );
	iRealArmor = MAX( pPlayer->ArmorValue(), 0 );

	wchar_t unicode[8];
	// Only update the fade if we've changed health
	if ( iRealHealth != m_iHealth )
	{
		if ( iRealHealth <= 20 )
		{
			// we are badly injured
			g_pClientMode->GetViewportAnimationController()->CancelAnimationsForPanel( this );
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "HealthLow" );
		}
		else if ( iRealHealth < m_iHealth )
		{
			// took a hit
			g_pClientMode->GetViewportAnimationController()->CancelAnimationsForPanel( this );
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( gHUD.GetSequenceNameForHUDColor( "HealthTookDamage", m_iHUDColor ) );
		}
		else
		{
			// round restarted, we have 100 again
			g_pClientMode->GetViewportAnimationController()->CancelAnimationsForPanel( this );
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( gHUD.GetSequenceNameForHUDColor( "HealthRestored", m_iHUDColor ) );
		}

		m_iHealth = iRealHealth;

		V_snwprintf( unicode, sizeof( unicode ), L"%d", m_iHealth );
		m_pHealthLabel->SetText( unicode );
		m_pHealthProgress->SetProgress( clamp( (float)m_iHealth / (float)iMaxHealth, 0.0f, 1.0f ) );
	}

	if ( iRealArmor != m_iArmor )
	{
		m_iArmor = iRealArmor;

		V_snwprintf( unicode, sizeof( unicode ), L"%d", m_iArmor );
		m_pArmorLabel->SetText( unicode );
		m_pSimpleArmorLabel->SetText( unicode );
		m_pArmorProgress->SetProgress( clamp( (float)m_iArmor / (float)iMaxArmor, 0.0f, 1.0f ) );
	}

	m_pHelmetIcon->SetVisible( pPlayer->HasHelmet() );
}

bool CHudHealthArmor::ShouldDraw()
{
	if ( cl_draw_only_deathnotices.GetBool() )
		return false;

	return CHudElement::ShouldDraw();
}
