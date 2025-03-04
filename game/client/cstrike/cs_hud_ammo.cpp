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
#include <vgui_controls/AnimationController.h>
#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/VectorImagePanel.h>

#include "c_cs_player.h"
#include "cs_gamerules.h"


using namespace vgui;

extern ConVar cl_hud_healthammo_style;
extern ConVar cl_hud_background_alpha;

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
	
private:
	float	m_flBackgroundAlpha;

	CHandle<C_BaseCombatWeapon>	m_pActiveWeapon;

	Label				*m_pPrimaryAmmoLabel;
	Label				*m_pPrimaryReserveAmmoLabel;
	VectorImagePanel	*m_pBulletIcon;

	// PiMoN: this one is tricky, I only need one single instance of VectorImagePanel and just need to use
	// SetTexture() every time i need to change the weapon icon, but in reality whenever I use SetTexture()
	// in-game the engine produces an emo-pattern instead of a transparent background, so i have to use
	// this dumb workaround :( at least there isn't much weapons with ITEM_FLAG_EXHAUSTIBLE
	//VectorImagePanel	*m_pExhaustibleWeaponIcon;
	VectorImagePanel	*m_pDecoyIcon;
	VectorImagePanel	*m_pFlashbangIcon;
	VectorImagePanel	*m_pHealthshotIcon;
	VectorImagePanel	*m_pHEGrenadeIcon;
	VectorImagePanel	*m_pIncGrenadeIcon;
	VectorImagePanel	*m_pMolotovIcon;
	VectorImagePanel	*m_pSmokeGrenadeIcon;

	CPanelAnimationVarAliasType( int, simple_wide, "simple_wide", "0", "proportional_width" );
	CPanelAnimationVarAliasType( int, simple_tall, "simple_tall", "0", "proportional_height" );

	bool	m_bUsesClips;
	bool	m_bIsExhaustible;
	int		m_iAmmoCount;

	int		m_iStyle;
	
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

	SetHiddenBits( HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD | HIDEHUD_WEAPONSELECTION );

	m_iSimpleXPos = 0;
	m_iSimpleYPos = 0;
	m_iOriginalXPos = 0;
	m_iOriginalYPos = 0;
	m_iOriginalWide = 0;
	m_iOriginalTall = 0;

	m_pActiveWeapon = NULL;

	m_pPrimaryAmmoLabel = new Label( this, "PrimaryAmmoLabel", "10" );
	m_pPrimaryReserveAmmoLabel = new Label( this, "PrimaryReserveAmmoLabel", "/ 20" );
	m_pBulletIcon = new VectorImagePanel( this, "BulletIcon" );
	//m_pExhaustibleWeaponIcon = new VectorImagePanel( this, "ExhaustibleWeaponIcon" );
	m_pDecoyIcon = new VectorImagePanel( this, "DecoyIcon" );
	m_pFlashbangIcon = new VectorImagePanel( this, "FlashbangIcon" );
	m_pHealthshotIcon = new VectorImagePanel( this, "HealthshotIcon" );
	m_pHEGrenadeIcon = new VectorImagePanel( this, "HEGrenadeIcon" );
	m_pIncGrenadeIcon = new VectorImagePanel( this, "IncGrenadeIcon" );
	m_pMolotovIcon = new VectorImagePanel( this, "MolotovIcon" );
	m_pSmokeGrenadeIcon = new VectorImagePanel( this, "SmokeGrenadeIcon" );

	LoadControlSettings( "resource/hud/ammo.res" );
}

void CHudAmmo::Init( void )
{
	m_flBackgroundAlpha = 0.0f;
	m_iStyle			= -1;

	m_bUsesClips		= false;
	m_bIsExhaustible	= false;
	m_iAmmoCount		= 0;
}

void CHudAmmo::ApplySettings( KeyValues *inResourceData )
{
	BaseClass::ApplySettings( inResourceData );

	GetBounds( m_iOriginalXPos, m_iOriginalYPos, m_iOriginalWide, m_iOriginalTall );

	int alignScreenWide, alignScreenTall;
	surface()->GetScreenSize( alignScreenWide, alignScreenTall );
	// these values have to be computed outside of PanelAnimationVars since those are recomputed before everything else (why??)
	ComputePos( this, inResourceData->GetString( "simple_xpos", NULL ), m_iSimpleXPos, simple_wide, alignScreenWide, true, OP_SET );
	ComputePos( this, inResourceData->GetString( "simple_ypos", NULL ), m_iSimpleYPos, simple_tall, alignScreenTall, false, OP_SET );
}

void CHudAmmo::Reset()
{
	g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "AmmoCounterReset" );
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
		Color oldColor = GetBgColor();
		Color newColor( oldColor.r(), oldColor.g(), oldColor.b(), cl_hud_background_alpha.GetFloat() * 255 );
		SetBgColor( newColor );
	}

	C_BaseCombatWeapon *pWeapon = GetActiveWeapon();
	if ( pWeapon != m_pActiveWeapon )
	{
		m_pActiveWeapon = pWeapon;
		bWeaponChanged = true;
	}

	if ( bWeaponChanged )
	{
		m_bUsesClips = m_pActiveWeapon && !(m_pActiveWeapon->GetWpnData().iFlags & ITEM_FLAG_EXHAUSTIBLE) && m_pActiveWeapon->UsesClipsForAmmo1();
		m_bIsExhaustible = m_pActiveWeapon && (m_pActiveWeapon->GetWpnData().iFlags & ITEM_FLAG_EXHAUSTIBLE) && !m_pActiveWeapon->UsesClipsForAmmo1();

		SetPaintBackgroundEnabled( m_bUsesClips );

		m_pPrimaryAmmoLabel->SetVisible( m_bUsesClips );
		m_pPrimaryReserveAmmoLabel->SetVisible( m_bUsesClips );

		m_pBulletIcon->SetVisible( m_bUsesClips && (m_iStyle == 0) );

		//m_pExhaustibleWeaponIcon->SetVisible( m_bIsExhaustible && (m_iStyle == 0) );
		m_pDecoyIcon->SetVisible( m_bIsExhaustible && m_pActiveWeapon->GetWeaponID() == WEAPON_DECOY && (m_iStyle == 0) );
		m_pFlashbangIcon->SetVisible( m_bIsExhaustible && m_pActiveWeapon->GetWeaponID() == WEAPON_FLASHBANG && (m_iStyle == 0) );
		m_pHealthshotIcon->SetVisible( m_bIsExhaustible && m_pActiveWeapon->GetWeaponID() == WEAPON_HEALTHSHOT && (m_iStyle == 0) );
		m_pHEGrenadeIcon->SetVisible( m_bIsExhaustible && m_pActiveWeapon->GetWeaponID() == WEAPON_HEGRENADE && (m_iStyle == 0) );
		m_pIncGrenadeIcon->SetVisible( m_bIsExhaustible && m_pActiveWeapon->GetWeaponID() == WEAPON_INCGRENADE && (m_iStyle == 0) );
		m_pMolotovIcon->SetVisible( m_bIsExhaustible &&  m_pActiveWeapon->GetWeaponID() == WEAPON_MOLOTOV && (m_iStyle == 0) );
		m_pSmokeGrenadeIcon->SetVisible( m_bIsExhaustible && m_pActiveWeapon->GetWeaponID() == WEAPON_SMOKEGRENADE && (m_iStyle == 0) );
	}

	if ( m_bUsesClips )
	{
		if ( m_iAmmoCount < m_pActiveWeapon->Clip1() )
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "AmmoCounterReset" );

		m_iAmmoCount = m_pActiveWeapon->Clip1();

		if ( ((float) m_iAmmoCount) / ((float) pWeapon->GetMaxClip1()) <= 0.2f ) // 20% or fewer bullets remaining
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "AmmoCounterLow" );

		wchar_t unicode[8];
		V_snwprintf( unicode, ARRAYSIZE( unicode ), L"%d", m_iAmmoCount );
		m_pPrimaryAmmoLabel->SetText( unicode );

		V_snwprintf( unicode, ARRAYSIZE( unicode ), L"/ %d", m_pActiveWeapon->GetReserveAmmoCount( AMMO_POSITION_PRIMARY ) );
		m_pPrimaryReserveAmmoLabel->SetText( unicode );

		m_pBulletIcon->SetRepeatsCount( Clamp( m_iAmmoCount, 0, 5 ) );
	}
	else
	{
		m_pBulletIcon->SetVisible( false );
	}

	C_CSPlayer *pPlayer = C_CSPlayer::GetLocalCSPlayer();
	if ( !pPlayer )
		return;

	// don't do it every frame, only do it when needed
	if ( m_bIsExhaustible && (m_iStyle == 0) )
	{
		//m_pExhaustibleWeaponIcon->SetRepeatsCount( pPlayer->GetAmmoCount( pWeapon->GetPrimaryAmmoType() ) );
		m_pDecoyIcon->SetRepeatsCount( pPlayer->GetAmmoCount( AMMO_TYPE_DECOY ) );
		m_pFlashbangIcon->SetRepeatsCount( pPlayer->GetAmmoCount( AMMO_TYPE_FLASHBANG ) );
		m_pHealthshotIcon->SetRepeatsCount( pPlayer->GetAmmoCount( AMMO_TYPE_HEALTHSHOT ) );
		m_pHEGrenadeIcon->SetRepeatsCount( pPlayer->GetAmmoCount( AMMO_TYPE_HEGRENADE ) );
		m_pIncGrenadeIcon->SetRepeatsCount( pPlayer->GetAmmoCount( AMMO_TYPE_MOLOTOV ) );
		m_pMolotovIcon->SetRepeatsCount( pPlayer->GetAmmoCount( AMMO_TYPE_MOLOTOV ) );
		m_pSmokeGrenadeIcon->SetRepeatsCount( pPlayer->GetAmmoCount( AMMO_TYPE_SMOKEGRENADE ) );
		/*if ( bWeaponChanged )
			m_pExhaustibleWeaponIcon->SetTexture( UTIL_VarArgs( "materials/vgui/weapons/svg/%s.svg", pWeapon->GetClassname() + 7 ) );*/
	}
}