//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "cstrikebuysubmenu.h"
#include "cstrikebuymenu.h"
#include "cs_shareddefs.h"
#include "backgroundpanel.h"
#include "cstrike/bot/shared_util.h"
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include "cs_gamerules.h"
#include "vgui_controls/RichText.h"
#include "cs_weapon_parse.h"
#include "c_cs_player.h"
#include "cs_ammodef.h"

ConVar closeonbuy( "closeonbuy", "1", FCVAR_ARCHIVE, "Set non-zero to close the buy menu after buying something", true, 0, true, 1 );

using namespace vgui;

// ----------------------------------------------------------------------------- //
// Buy menu player image panels. These maintain a list of the player image panels so 
// it can render 3D images into them.
// ----------------------------------------------------------------------------- //

CUtlVector<CCSBuyMenuPlayerImagePanel*> g_BuyMenuPlayerImagePanels;


CCSBuyMenuPlayerImagePanel::CCSBuyMenuPlayerImagePanel( vgui::Panel *pParent, const char *pName )
	: BaseClass( pParent, pName )
{
	g_BuyMenuPlayerImagePanels.AddToTail( this );
}

CCSBuyMenuPlayerImagePanel::~CCSBuyMenuPlayerImagePanel()
{
	g_BuyMenuPlayerImagePanels.FindAndRemove( this );
}

void CCSBuyMenuPlayerImagePanel::ApplySettings( KeyValues *inResourceData )
{
	m_flViewXPos = inResourceData->GetFloat( "view_xpos", 0.0f );
	m_flViewYPos = inResourceData->GetFloat( "view_ypos", 0.0f );
	m_flViewZPos = inResourceData->GetFloat( "view_zpos", 0.0f );
	m_flViewFOV = inResourceData->GetFloat( "view_fov", 0.0f );

	BaseClass::ApplySettings( inResourceData );
}

// ----------------------------------------------------------------------------- //
// Buy menu image panels. These maintain a list of the buy menu image panels so 
// it can render 3D images into them.
// ----------------------------------------------------------------------------- //

CUtlVector<CCSBuyMenuImagePanel*> g_BuyMenuImagePanels;


CCSBuyMenuImagePanel::CCSBuyMenuImagePanel( vgui::Panel *pParent, const char *pName )
	: BaseClass( pParent, pName )
{
	g_BuyMenuImagePanels.AddToTail( this );
	m_szWeaponName[0] = NULL;
}

CCSBuyMenuImagePanel::~CCSBuyMenuImagePanel()
{
	g_BuyMenuImagePanels.FindAndRemove( this );
}

void CCSBuyMenuImagePanel::ApplySettings( KeyValues *inResourceData )
{
	const char *pString = inResourceData->GetString( "weaponName" );
	if ( pString )
	{
		Q_strncpy( m_szWeaponName, pString, sizeof( m_szWeaponName ) );
	}

	m_flViewXPos = inResourceData->GetFloat( "view_xpos", 0.0f );
	m_flViewYPos = inResourceData->GetFloat( "view_ypos", 0.0f );
	m_flViewZPos = inResourceData->GetFloat( "view_zpos", 0.0f );
	m_flViewFOV = inResourceData->GetFloat( "view_fov", 0.0f );
	
	BaseClass::ApplySettings( inResourceData );
}


//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CCSBuyMenu_CT::CCSBuyMenu_CT(IViewPort *pViewPort) : CCSBaseBuyMenu( pViewPort, "BuySubMenu_CT" )
{
	m_pMainMenu->LoadControlSettings( "Resource/UI/BuyMenu_CT.res" );
	m_pMainMenu->SetVisible( false );

	m_iTeam = TEAM_CT;

	CreateBackground( this );
	m_backgroundLayoutFinished = false;
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CCSBuyMenu_TER::CCSBuyMenu_TER(IViewPort *pViewPort) : CCSBaseBuyMenu( pViewPort, "BuySubMenu_TER" )
{
	m_pMainMenu->LoadControlSettings( "Resource/UI/BuyMenu_TER.res" );
	m_pMainMenu->SetVisible( false );

	m_iTeam = TEAM_TERRORIST;

	CreateBackground( this );
	m_backgroundLayoutFinished = false;
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CCSBaseBuyMenu::CCSBaseBuyMenu(IViewPort *pViewPort, const char *subPanelName) : CBuyMenu( pViewPort )
{
	SetTitle( "#Cstrike_Buy_Menu", true);

	SetProportional( true );

	m_pMainMenu = new CCSBuySubMenu( this, subPanelName );
	m_pMainMenu->SetSize( 10, 10 ); // Quiet "parent not sized yet" spew
 	m_lastMoney = -1;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CCSBaseBuyMenu::SetVisible(bool state)
{
	BaseClass::SetVisible(state);

	if ( state )
	{
		Panel *defaultButton = FindChildByName( "CancelButton" );
		if ( defaultButton )
		{
			defaultButton->RequestFocus();
		}
		SetMouseInputEnabled( true );
		m_pMainMenu->SetMouseInputEnabled( true );
	}
}

//-----------------------------------------------------------------------------
static void GetPanelBounds( Panel *pPanel, wrect_t& bounds )
{
	if ( !pPanel )
	{
		bounds.bottom = bounds.left = bounds.right = bounds.top = 0;
	}
	else
	{
		pPanel->GetBounds( bounds.left, bounds.top, bounds.right, bounds.bottom );
		bounds.right += bounds.left;
		bounds.bottom += bounds.top;
	}
}

//-----------------------------------------------------------------------------
Panel * CCSBaseBuyMenu::CreateControlByName( const char *controlName )
{
	if ( Q_stricmp( controlName, "CCSBuyMenuPlayerImagePanel" ) == 0 )
	{
		return new CCSBuyMenuPlayerImagePanel( NULL, controlName );
	}

	return BaseClass::CreateControlByName( controlName );
}

//-----------------------------------------------------------------------------
// Purpose: The CS background is painted by image panels, so we should do nothing
//-----------------------------------------------------------------------------
void CCSBaseBuyMenu::PaintBackground()
{
}

//-----------------------------------------------------------------------------
// Purpose: Scale / center the window
//-----------------------------------------------------------------------------
void CCSBaseBuyMenu::PerformLayout()
{
	BaseClass::PerformLayout();

	// stretch the window to fullscreen
	if ( !m_backgroundLayoutFinished )
		LayoutBackgroundPanel( this );
	m_backgroundLayoutFinished = true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CCSBaseBuyMenu::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );
	ApplyBackgroundSchemeSettings( this, pScheme );

	m_hUnderlineFont = pScheme->GetFont( "CSUnderline", IsProportional() );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
static bool IsWeaponInvalid( CSWeaponID weaponID )
{
	if ( weaponID == WEAPON_NONE )
		return false;

	C_CSPlayer *pPlayer = C_CSPlayer::GetLocalCSPlayer();
	if ( !pPlayer )
		return true;

	CCSWeaponInfo *info = GetWeaponInfo( weaponID );
	if ( !info )
		return true;

	/// @TODO: assasination maps have a specific set of weapons that can be used in them.
	if ( info->m_iTeam != TEAM_UNASSIGNED && pPlayer->GetTeamNumber() != info->m_iTeam )
		return true;

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CCSBuySubMenu::OnThink()
{
	UpdateVestHelmPrice();
	BaseClass::OnThink();
}

//-----------------------------------------------------------------------------
// Purpose: When buying vest+helmet, if you already have a vest with no damage
// then the price is reduced to just the helmet.  Because this can change during
// the game, we need to update the enable/disable state of the menu item dynamically.
//-----------------------------------------------------------------------------
void CCSBuySubMenu::UpdateVestHelmPrice()
{
	C_CSPlayer *pPlayer = C_CSPlayer::GetLocalCSPlayer();
	if ( pPlayer == NULL )
		return;

	BuyMouseOverPanelButton *pButton = dynamic_cast< BuyMouseOverPanelButton * > ( FindChildByName( "kevlar_helmet", false ) );
	if ( pButton )
	{
		// Set its price to the current value from the player.
		pButton->SetCurrentPrice( pPlayer->GetCurrentAssaultSuitPrice() );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CCSBuySubMenu::OnCommand( const char *command )
{
	if ( FStrEq( command, "buy_unavailable" ) )
	{
		C_CSPlayer *pPlayer = C_CSPlayer::GetLocalCSPlayer();
		if ( pPlayer )
		{
			pPlayer->EmitSound( "BuyPreset.CantBuy" );
		}
		BaseClass::OnCommand( "vguicancel" );
		return;
	}

	BaseClass::OnCommand( command );
}

Panel * CCSBuySubMenu::CreateControlByName(const char *controlName)
{
	if ( Q_stricmp( controlName, "CSBuyMenuImagePanel" ) == 0 )
	{
		return new CCSBuyMenuImagePanel( NULL, controlName );
	}

	return BaseClass::CreateControlByName( controlName );
}

void CCSBuySubMenu::OnSizeChanged(int newWide, int newTall)
{
	m_backgroundLayoutFinished = false;
	BaseClass::OnSizeChanged( newWide, newTall );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CCSBuySubMenu::PerformLayout()
{
	BaseClass::PerformLayout();

	// Buy submenus need to be shoved over for widescreen
	int screenW, screenH;
	GetHudSize( screenW, screenH );

	int fullW, fullH;
	fullW = scheme()->GetProportionalScaledValueEx( GetScheme(), 640 );
	fullH = scheme()->GetProportionalScaledValueEx( GetScheme(), 480 );

	fullW = GetAlternateProportionalValueFromScaled( GetScheme(), fullW );
	fullH = GetAlternateProportionalValueFromScaled( GetScheme(), fullH );

	int offsetX = (screenW - fullW)/2;
	int offsetY = (screenH - fullH)/2;

	if ( !m_backgroundLayoutFinished )
		ResizeWindowControls( this, GetWide(), GetTall(), offsetX, offsetY );
	m_backgroundLayoutFinished = true;
}


