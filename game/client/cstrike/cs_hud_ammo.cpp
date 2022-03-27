//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "iclientmode.h"

#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include <vgui_controls/AnimationController.h>
#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/VectorImagePanel.h>
#include <vgui_controls/ImagePanel.h>

#include "c_cs_player.h"
#include "cs_client_gamestats.h"
#include "cs_gamerules.h"

using namespace vgui;

extern ConVar cl_hud_healthammo_style;
extern ConVar cl_hud_background_alpha;
extern ConVar cl_hud_color;
extern ConVar loadout_stattrak;
extern ConVar cl_draw_only_deathnotices;

//-----------------------------------------------------------------------------
// Purpose: Displays current ammunition level
//-----------------------------------------------------------------------------
class CHudAmmo : public CHudElement, public EditablePanel
{
	DECLARE_CLASS_SIMPLE( CHudAmmo, EditablePanel );

public:
	CHudAmmo( const char *pElementName );
	virtual void Init( void );
	virtual void ApplySettings( KeyValues *inResourceData );
	virtual void Reset( void );
	virtual void OnThink();
	virtual void OnScreenSizeChanged( int iOldWide, int iOldTall );
	virtual bool ShouldDraw();
	
private:
	CHandle<C_WeaponCSBase>	m_pActiveWeapon;

	Label				*m_pPrimaryAmmoLabel;
	Label				*m_pPrimaryReserveAmmoLabel;
	Label				*m_pStatTrakCounter;
	Label				*m_pKillCounter;
	ImagePanel			*m_pKillCounterImage;
	VectorImagePanel	*m_pBulletIcon;
	VectorImagePanel	*m_pExhaustibleWeaponIcon;
	VectorImagePanel	*m_pBurstIcon;

	CPanelAnimationVarAliasType( int, simple_wide, "simple_wide", "0", "proportional_width" );
	CPanelAnimationVarAliasType( int, simple_tall, "simple_tall", "0", "proportional_height" );

	bool	m_bUsesClips;
	bool	m_bIsExhaustible;
	int		m_iAmmoCount;
	bool	m_bBurstMode;
	
	int		m_iSimpleXPos;
	int		m_iSimpleYPos;
	int		m_iOriginalXPos;
	int		m_iOriginalYPos;
	int		m_iOriginalWide;
	int		m_iOriginalTall;
};

DECLARE_HUDELEMENT( CHudAmmo );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudAmmo::CHudAmmo( const char *pElementName ): CHudElement( pElementName ), EditablePanel( NULL, "HudAmmo" )
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	SetHiddenBits( HIDEHUD_WEAPONSELECTION | HIDEHUD_NOT_OBSERVING_PLAYERS );

	m_iSimpleXPos = 0;
	m_iSimpleYPos = 0;
	m_iOriginalXPos = 0;
	m_iOriginalYPos = 0;
	m_iOriginalWide = 0;
	m_iOriginalTall = 0;

	m_pActiveWeapon = NULL;

	m_pPrimaryAmmoLabel = new Label( this, "PrimaryAmmoLabel", L"10" );
	m_pPrimaryReserveAmmoLabel = new Label( this, "PrimaryReserveAmmoLabel", L"/ 20" );
	m_pStatTrakCounter = new Label( this, "StatTrakCounter", L"0" );
	m_pKillCounter = new Label( this, "KillCounter", L"x0" );
	m_pKillCounterImage = new ImagePanel( this, "KillCounterImage" );
	m_pBulletIcon = new VectorImagePanel( this, "BulletIcon" );
	m_pExhaustibleWeaponIcon = new VectorImagePanel( this, "ExhaustibleWeaponIcon" );
	m_pBurstIcon = new VectorImagePanel( this, "BurstIcon" );

	LoadControlSettings( "resource/hud/ammo.res" );
}

void CHudAmmo::OnScreenSizeChanged( int iOldWide, int iOldTall )
{
 	// reload the .res file so items are rescaled
 	LoadControlSettings( "resource/hud/ammo.res" );
 
 	// force recalculation of some stuff
 	m_iHUDColor = -1;
 	m_flBackgroundAlpha = 0.0f;
 	m_iStyle = -1;
 	m_bUsesClips = false;
 	m_bIsExhaustible = false;
 	m_iAmmoCount = 0;
 	m_bBurstMode = false;
}

void CHudAmmo::Init( void )
{
	m_bUsesClips		= false;
	m_bIsExhaustible	= false;
	m_iAmmoCount		= 0;
	m_bBurstMode		= false;
}

void CHudAmmo::ApplySettings( KeyValues *inResourceData )
{
	BaseClass::ApplySettings( inResourceData );

	GetBounds( m_iOriginalXPos, m_iOriginalYPos, m_iOriginalWide, m_iOriginalTall );

	int alignScreenWide, alignScreenTall;
	surface()->GetScreenSize( alignScreenWide, alignScreenTall );
	// these values have to be computed outside of PanelAnimationVars since those are recomputed before everything else (why??)
	ComputePos( this, inResourceData->GetString( "simple_xpos", NULL ), m_iSimpleXPos, simple_wide, alignScreenWide, m_iBaseResolutionOverride[0], m_iBaseResolutionOverride[1], true, OP_SET );
	ComputePos( this, inResourceData->GetString( "simple_ypos", NULL ), m_iSimpleYPos, simple_tall, alignScreenTall, m_iBaseResolutionOverride[0], m_iBaseResolutionOverride[1], false, OP_SET );
}

void CHudAmmo::Reset()
{
	g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( gHUD.GetSequenceNameForHUDColor( "AmmoCounterReset", m_iHUDColor ) );
}

//-----------------------------------------------------------------------------
// Purpose: called every frame to get ammo info from the weapon
//-----------------------------------------------------------------------------
void CHudAmmo::OnThink()
{
	bool bWeaponChanged = false;
	if ( m_iStyle != cl_hud_healthammo_style.GetInt() )
	{
		m_iStyle = cl_hud_healthammo_style.GetInt();
		bWeaponChanged = true; // force weapon icon to recompute visibility

		switch ( m_iStyle )
		{
			case 0: // default
				SetBounds( m_iOriginalXPos, m_iOriginalYPos, m_iOriginalWide, m_iOriginalTall );
				break;

			case 1: // simple
				SetBounds( m_iSimpleXPos, m_iSimpleYPos, simple_wide, simple_tall );
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

		m_pPrimaryAmmoLabel->SetFgColor( clr );
		m_pPrimaryReserveAmmoLabel->SetFgColor( clr );
		m_pBulletIcon->SetFgColor( clr );
	}

	C_CSPlayer *pPlayer = GetHudPlayer();
	if ( !pPlayer )
	{
		SetPaintEnabled( false );
		SetPaintBackgroundEnabled( false );
		return;
	}

	C_WeaponCSBase *pWeapon = pPlayer->GetActiveCSWeapon();
	if ( !pWeapon )
	{
		m_pActiveWeapon = NULL;
		SetPaintEnabled( false );
		SetPaintBackgroundEnabled( false );
		return;
	}

	SetPaintEnabled( true );

	if ( pWeapon != m_pActiveWeapon )
	{
		m_pActiveWeapon = pWeapon;
		bWeaponChanged = true;
	}

	if ( bWeaponChanged )
	{
		m_bUsesClips = !(m_pActiveWeapon->GetWpnData().iFlags & ITEM_FLAG_EXHAUSTIBLE) && m_pActiveWeapon->UsesClipsForAmmo1();
		m_bIsExhaustible = (m_pActiveWeapon->GetWpnData().iFlags & ITEM_FLAG_EXHAUSTIBLE) && !m_pActiveWeapon->UsesClipsForAmmo1();

		SetPaintBackgroundEnabled( m_bUsesClips );

		m_pPrimaryAmmoLabel->SetVisible( m_bUsesClips );
		m_pPrimaryReserveAmmoLabel->SetVisible( m_bUsesClips );

		m_pBulletIcon->SetVisible( m_bUsesClips && (m_iStyle == 0) );

		m_pExhaustibleWeaponIcon->SetVisible( m_bIsExhaustible && (m_iStyle == 0) );
		m_pBurstIcon->SetVisible( m_pActiveWeapon->WeaponHasBurst() );
	}

	if ( m_bUsesClips )
	{
		if ( m_iAmmoCount < m_pActiveWeapon->Clip1() )
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( gHUD.GetSequenceNameForHUDColor( "AmmoCounterReset", m_iHUDColor ) );

		m_iAmmoCount = m_pActiveWeapon->Clip1();

		if ( ((float) m_iAmmoCount) / ((float) pWeapon->GetMaxClip1()) <= 0.2f ) // 20% or fewer bullets remaining
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "AmmoCounterLow" );

		wchar_t unicode[8];
		V_snwprintf( unicode, ARRAYSIZE( unicode ), L"%d", m_iAmmoCount );
		m_pPrimaryAmmoLabel->SetText( unicode );

		V_snwprintf( unicode, ARRAYSIZE( unicode ), L"/ %d", m_pActiveWeapon->GetReserveAmmoCount( AMMO_POSITION_PRIMARY ) );
		m_pPrimaryReserveAmmoLabel->SetText( unicode );

		m_pBulletIcon->SetRepeatsCount( Clamp( m_iAmmoCount, 0, 5 ) );

		if ( m_bBurstMode != m_pActiveWeapon->IsInBurstMode() )
		{
			m_bBurstMode = m_pActiveWeapon->IsInBurstMode();
			m_pBurstIcon->SetTexture( m_bBurstMode ? "materials/vgui/hud/svg/bullet_burst.svg" : "materials/vgui/hud/svg/bullet_burst_outline.svg" );
		}
	}

	// don't do it every frame, only do it when needed
	if ( m_bIsExhaustible && (m_iStyle == 0) )
	{
		m_pExhaustibleWeaponIcon->SetRepeatsCount( pPlayer->GetAmmoCount( pWeapon->GetPrimaryAmmoType() ) );
		if ( bWeaponChanged )
			m_pExhaustibleWeaponIcon->SetTexture( UTIL_VarArgs( "materials/vgui/weapons/svg/%s.svg", pWeapon->GetClassname() + 7 ) );
	}

	wchar_t wszString[8];
	if ( pWeapon->HasStatTrak() )
	{
		int iStatTrakCounter = 0;
		int entindex = pPlayer->entindex();
		if ( pPlayer->IsControllingBot() )
			entindex = pPlayer->GetControlledBotIndex();

		if ( pWeapon->GetOriginalOwnerIndex() == entindex )
			iStatTrakCounter = g_CSClientGameStats.GetStatById( GetWeaponTableEntryFromWeaponId( pWeapon->GetCSWeaponID() ).killStatId ).iStatValue;
		else
			iStatTrakCounter = 0;

		if ( iStatTrakCounter > 0 )
		{
			V_snwprintf( wszString, sizeof( wszString ), L"%d", iStatTrakCounter );

			wchar_t wszLocalized[32];
			g_pVGuiLocalize->ConstructString( wszLocalized, sizeof( wszLocalized ), g_pVGuiLocalize->Find( "#Cstrike_WPNHUD_StatTrak" ), 1, wszString );

			m_pStatTrakCounter->SetText( wszLocalized );
			m_pStatTrakCounter->SetVisible( true );
		}
		else
		{
			m_pStatTrakCounter->SetVisible( false );
		}
	}
	else
	{
		m_pStatTrakCounter->SetVisible( false );
	}

	int iNumKills = pPlayer->GetNumKillsThisSpawn();
	if ( iNumKills > 0 )
	{
		V_snwprintf( wszString, sizeof( wszString ), L"x%d", iNumKills );
		m_pKillCounter->SetText( wszString );
		m_pKillCounter->SetVisible( true );
		m_pKillCounterImage->SetVisible( true );
	}
	else
	{
		m_pKillCounter->SetVisible( false );
		m_pKillCounterImage->SetVisible( false );
	}
}

bool CHudAmmo::ShouldDraw()
{
	if ( cl_draw_only_deathnotices.GetBool() )
		return false;

	return CHudElement::ShouldDraw();
}
