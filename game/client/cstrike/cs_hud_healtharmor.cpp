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
#include "hud.h"
#include "hud_macros.h"
#include "view.h"

#include "iclientmode.h"

#define PAIN_NAME "sprites/%d_pain.vmt"

#include <KeyValues.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui_controls/AnimationController.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/ProgressBar.h>

using namespace vgui;

#include "hudelement.h"
#include "hud_numericdisplay.h"
#include "cs_gamerules.h"

#include "convar.h"

ConVar hud_healtharmor_style( "hud_healtharmor_style", "0", FCVAR_ARCHIVE, "0 = default, 1 = simple", true, 0, true, HUD_STYLE_MAX );

extern ConVar cl_hud_background_alpha;


//-----------------------------------------------------------------------------
// Purpose: Overriding Paint method to allow for correct border rendering
//-----------------------------------------------------------------------------
class CHudHealthArmorProgress: public ContinuousProgressBar
{
	DECLARE_CLASS_SIMPLE( CHudHealthArmorProgress, ContinuousProgressBar );

public:
	CHudHealthArmorProgress( Panel *parent, const char *panelName );
	virtual void Paint();
};

CHudHealthArmorProgress::CHudHealthArmorProgress( Panel *parent, const char *panelName ): ContinuousProgressBar( parent, panelName )
{
}

void CHudHealthArmorProgress::Paint()
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


//-----------------------------------------------------------------------------
// Purpose: Health panel
//-----------------------------------------------------------------------------
class CHudHealthArmor : public CHudElement, public EditablePanel
{
	DECLARE_CLASS_SIMPLE( CHudHealthArmor, EditablePanel );

public:
	CHudHealthArmor( const char *pElementName );
	virtual void Init( void );
	virtual void VidInit( void );
	virtual void Reset( void );
	virtual void OnThink();

	virtual void Paint( void );
	virtual void ApplySchemeSettings( IScheme *scheme );
	virtual void ApplySettings( KeyValues *inResourceData );

private:
	int		m_iHealth;
	int		m_iArmor;

	float	m_flBackgroundAlpha;

	CHudTexture	*m_pHealthIcon;
	CHudTexture	*m_pArmorIcon;
	CHudTexture	*m_pArmorHelmetIcon;
	// TODO: if adding heavy armor, it has a separate, bigger armor icon

	Label		*m_pHealthLabel;
	Label		*m_pArmorLabel;
	Label		*m_pSimpleArmorLabel;

	CHudHealthArmorProgress	*m_pHealthProgress;
	CHudHealthArmorProgress	*m_pArmorProgress;

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

	CPanelAnimationVar( Color, m_clrHealthIconFg, "HealthIconFgColor", "FgColor" );
	CPanelAnimationVar( Color, m_clrArmorIconFg, "ArmorIconFgColor", "FgColor" );

	int m_iStyle;
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

	SetHiddenBits( HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD );

	m_iOriginalWide = 0;
	m_iOriginalTall = 0;

	m_pHealthLabel = new Label( this, "HealthLabel", "" );
	m_pArmorLabel = new Label( this, "ArmorLabel", "" );
	m_pSimpleArmorLabel = new Label( this, "SimpleArmorLabel", "" );

	m_pHealthProgress = new CHudHealthArmorProgress( this, "HealthProgress" );
	m_pArmorProgress = new CHudHealthArmorProgress( this, "ArmorProgress" );

	LoadControlSettings( "resource/hud/healtharmor.res" );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudHealthArmor::Init()
{
	m_iHealth			= -1;
	m_iArmor			= -1;
	m_iStyle			= -1;
	m_flBackgroundAlpha = 0.0f;
}

void CHudHealthArmor::ApplySchemeSettings( IScheme *scheme )
{
	BaseClass::ApplySchemeSettings( scheme );

	if( !m_pHealthIcon )
	{
		m_pHealthIcon = gHUD.GetIcon( "health_icon" );
	}
	if ( !m_pArmorIcon )
	{
		m_pArmorIcon = gHUD.GetIcon( "shield_bright" );
	}
	if ( !m_pArmorHelmetIcon )
	{
		m_pArmorHelmetIcon = gHUD.GetIcon( "shield_kevlar_bright" );
	}
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
	g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("HealthRestored");
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudHealthArmor::VidInit()
{
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudHealthArmor::OnThink()
{
	if ( m_iStyle != hud_healtharmor_style.GetInt() )
	{
		m_iStyle = hud_healtharmor_style.GetInt();

		switch ( m_iStyle )
		{
			case HUD_STYLE_DEFAULT:
				SetSize( m_iOriginalWide, m_iOriginalTall );

				m_pHealthProgress->SetVisible( true );
				m_pArmorProgress->SetVisible( true );

				m_pArmorLabel->SetVisible( true );
				m_pSimpleArmorLabel->SetVisible( false );
				break;

			case HUD_STYLE_SIMPLE:
				SetSize( simple_wide, simple_tall );

				m_pHealthProgress->SetVisible( false );
				m_pArmorProgress->SetVisible( false );

				m_pArmorLabel->SetVisible( false );
				m_pSimpleArmorLabel->SetVisible( true );
				break;
		}
	}

	if ( m_flBackgroundAlpha != cl_hud_background_alpha.GetFloat() )
	{
		Color oldColor = GetBgColor();
		Color newColor( oldColor.r(), oldColor.g(), oldColor.b(), cl_hud_background_alpha.GetFloat() * 255 );
		SetBgColor( newColor );
	}

	int realHealth = 0;
	int realArmor = 0;
	C_CSPlayer *local = C_CSPlayer::GetLocalCSPlayer();
	if ( local )
	{
		// Never below zero
		realHealth = MAX( local->GetHealth(), 0 );
		realArmor = MAX( local->ArmorValue(), 0 );
	}

	// Only update the fade if we've changed health
	if ( realHealth != m_iHealth )
	{
		if ( realHealth > m_iHealth )
		{
			// round restarted, we have 100 again
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "HealthRestored" );
		}
		else if ( realHealth <= 25 )
		{
			// we are badly injured
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "HealthLow" );
		}
		else if ( realHealth < m_iHealth )
		{
			// took a hit
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "HealthTookDamage" );
		}

		m_iHealth = realHealth;

		m_pHealthLabel->SetText( UTIL_VarArgs( "%d", m_iHealth ) );
		m_pHealthProgress->SetProgress( clamp( m_iHealth / 100.0f, 0.0f, 1.0f ) );
	}

	if ( realArmor != m_iArmor )
	{
		m_iArmor = realArmor;
		const char *szArmorText = UTIL_VarArgs( "%d", m_iArmor );
		m_pArmorLabel->SetText( szArmorText );
		m_pSimpleArmorLabel->SetText( szArmorText );
		m_pArmorProgress->SetProgress( clamp( m_iArmor / 100.0f, 0.0f, 1.0f ) );
	}
}

void CHudHealthArmor::Paint( void )
{
	C_CSPlayer *pPlayer = C_CSPlayer::GetLocalCSPlayer();
	if ( !pPlayer )
		return;

	if ( m_pHealthIcon )
	{
		switch ( m_iStyle )
		{
			case HUD_STYLE_DEFAULT:
				m_pHealthIcon->DrawSelf( health_icon_xpos, health_icon_ypos, m_clrHealthIconFg );
				break;

			case HUD_STYLE_SIMPLE:
				m_pHealthIcon->DrawSelf( simple_health_icon_xpos, simple_health_icon_ypos, m_clrHealthIconFg );
				break;
		}
	}
	if ( pPlayer->HasHelmet() && m_iArmor > 0 )
	{
		if ( m_pArmorHelmetIcon )
		{
			switch ( m_iStyle )
			{
				case HUD_STYLE_DEFAULT:
					m_pArmorHelmetIcon->DrawSelf( armor_icon_xpos, armor_icon_ypos, m_clrArmorIconFg );
					break;

				case HUD_STYLE_SIMPLE:
					m_pArmorHelmetIcon->DrawSelf( simple_armor_icon_xpos, simple_armor_icon_ypos, m_clrArmorIconFg );
					break;
			}
		}
	}
	else
	{
		if ( m_pArmorIcon )
		{
			switch ( m_iStyle )
			{
				case HUD_STYLE_DEFAULT:
					m_pArmorIcon->DrawSelf( armor_icon_xpos, armor_icon_ypos, m_clrArmorIconFg );
					break;

				case HUD_STYLE_SIMPLE:
					m_pArmorIcon->DrawSelf( simple_armor_icon_xpos, simple_armor_icon_ypos, m_clrArmorIconFg );
					break;
			}
		}
	}

	//draw the health icon
	BaseClass::Paint();
}