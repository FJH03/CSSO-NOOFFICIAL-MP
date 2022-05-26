//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//


#if defined( WIN32 ) && !defined( _X360 )
#include <windows.h> // SRC only!!
#endif

#include "ModOptionsSubHUD.h"
#include <stdio.h>

#include <vgui/ISystem.h>
#include <vgui/ISurface.h>

#include "LabeledCommandComboBox.h"
#include "cvarslider.h"
#include "CvarToggleCheckButton.h"
#include "EngineInterface.h"
#include "tier1/convar.h"
#include "BasePanel.h"
#include "GameUI_Interface.h"

#include "hud.h" // for MAX_HUD_COLORS

#if defined( _X360 )
#include "xbox/xbox_win32stubs.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Basic help dialog
//-----------------------------------------------------------------------------
CModOptionsSubHUD::CModOptionsSubHUD( vgui::Panel *parent ): vgui::PropertyPage( parent, "ModOptionsSubHUD" )
{
	Button *cancel = new Button( this, "Cancel", "#GameUI_Cancel" );
	cancel->SetCommand( "Close" );

	Button *ok = new Button( this, "OK", "#GameUI_OK" );
	ok->SetCommand( "Ok" );

	Button *apply = new Button( this, "Apply", "#GameUI_Apply" );
	apply->SetCommand( "Apply" );

	//=========
	m_pPlayerCountPos = new CLabeledCommandComboBox( this, "PlayerCountPositionComboBox" );
	m_pHealthAmmoStyle = new CLabeledCommandComboBox( this, "HealthAmmoStyleComboBox" );
	m_pHUDColor = new CLabeledCommandComboBox( this, "HUDColorComboBox" );
	m_pHUDBackgroundAlpha = new CCvarSlider( this, "HUDBackgroundAlphaSlider", "", 0.0f, 1.0f, "cl_hud_background_alpha" );
	m_pRadarScale = new CCvarSlider( this, "RadarScaleSlider", "", 0.25f, 1.0f, "cl_radar_scale" );
	m_pAlwaysShowInventory = new CCvarToggleCheckButton( this, "AlwaysShowInventoryCheckbox", "#GameUI_HUD_AlwaysShowInventory", "cl_showloadout" );
	m_pRadarRotate = new CCvarToggleCheckButton( this, "RadarRotateCheckbox", "#GameUI_HUD_RotateRadar", "cl_radar_rotate" );
	m_pRadarSquare = new CLabeledCommandComboBox( this, "RadarSquareComboBox" );
	m_pMenuBackground = new CLabeledCommandComboBox( this, "MenuBackgroundComboBox" );
	m_pMenuAgent = new CLabeledCommandComboBox( this, "MenuAgentComboBox" );

	m_pPlayerCountPos->AddItem( "#GameUI_HUD_PlayerCount_Top", "hud_playercount_pos 0" );
	m_pPlayerCountPos->AddItem( "#GameUI_HUD_PlayerCount_Bottom", "hud_playercount_pos 1" );

	m_pHealthAmmoStyle->AddItem( "#GameUI_HUD_HealthAmmoStyle_0", "cl_hud_healthammo_style 0" );
	m_pHealthAmmoStyle->AddItem( "#GameUI_HUD_HealthAmmoStyle_1", "cl_hud_healthammo_style 1" );

	m_pRadarSquare->AddItem( "#GameUI_HUD_RadarSquare_0", "cl_radar_square 0" );
	m_pRadarSquare->AddItem( "#GameUI_HUD_RadarSquare_1", "cl_radar_square 1" );
	m_pRadarSquare->AddItem( "#GameUI_HUD_RadarSquare_2", "cl_radar_square 2" );

	m_pMenuAgent->AddItem( "#GameUI_Loadout_Agent_None", "loadout_mainmenu_agent 0" );
	m_pMenuAgent->AddItem( "#GameUI_Loadout_Agent_T", "loadout_mainmenu_agent 1" );
	m_pMenuAgent->AddItem( "#GameUI_Loadout_Agent_CT", "loadout_mainmenu_agent 2" );

	char localization[64];
	char command[64];
	for ( int i = 0; i < ARRAYSIZE(s_MenuBackgrounds); i++ )
	{
		Q_snprintf( command, sizeof( command ), "cl_menu_background %d", i );
		m_pMenuBackground->AddItem( s_MenuBackgrounds[i].szName, command );
	}
	for ( int i = 0; i < MAX_HUD_COLORS; i++ )
	{
		Q_snprintf( localization, sizeof( localization ), "#GameUI_HUD_Color_%d", i );
		Q_snprintf( command, sizeof( command ), "cl_hud_color %d", i );
		m_pHUDColor->AddItem( localization, command );
	}

	m_pPlayerCountPos->AddActionSignalTarget( this );
	m_pHealthAmmoStyle->AddActionSignalTarget( this );
	m_pHUDColor->AddActionSignalTarget( this );
	m_pHUDBackgroundAlpha->AddActionSignalTarget( this );
	m_pRadarScale->AddActionSignalTarget( this );
	m_pAlwaysShowInventory->AddActionSignalTarget( this );
	m_pRadarRotate->AddActionSignalTarget( this );
	m_pRadarSquare->AddActionSignalTarget( this );
	m_pMenuBackground->AddActionSignalTarget( this );
	m_pMenuAgent->AddActionSignalTarget( this );

	LoadControlSettings( "Resource/ModOptionsSubHUD.res" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CModOptionsSubHUD::~CModOptionsSubHUD()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CModOptionsSubHUD::OnControlModified()
{
	PostMessage( GetParent(), new KeyValues( "ApplyButtonEnable" ) );
	InvalidateLayout();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CModOptionsSubHUD::OnResetData()
{
	ConVarRef hud_playercount_pos( "hud_playercount_pos" );
	if ( hud_playercount_pos.IsValid() )
		m_pPlayerCountPos->SetInitialItem( hud_playercount_pos.GetInt() );

	ConVarRef cl_hud_healthammo_style( "cl_hud_healthammo_style" );
	if ( cl_hud_healthammo_style.IsValid() )
		m_pHealthAmmoStyle->SetInitialItem( cl_hud_healthammo_style.GetInt() );

	ConVarRef cl_hud_color( "cl_hud_color" );
	if ( cl_hud_color.IsValid() )
		m_pHUDColor->SetInitialItem( cl_hud_color.GetInt() );

	ConVarRef cl_radar_square( "cl_radar_square" );
	if ( cl_radar_square.IsValid() )
		m_pRadarSquare->SetInitialItem( cl_radar_square.GetInt() );

	ConVarRef cl_menu_background( "cl_menu_background" );
	if ( cl_menu_background.IsValid() )
		m_pMenuBackground->SetInitialItem( cl_menu_background.GetInt() );

	ConVarRef loadout_mainmenu_agent( "loadout_mainmenu_agent" );
	if ( loadout_mainmenu_agent.IsValid() )
		m_pMenuAgent->SetInitialItem( loadout_mainmenu_agent.GetInt() );

	m_pHUDBackgroundAlpha->Reset();
	m_pRadarScale->Reset();
	m_pAlwaysShowInventory->Reset();
	m_pRadarRotate->Reset();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CModOptionsSubHUD::OnApplyChanges()
{
	m_pPlayerCountPos->ApplyChanges();
	m_pHealthAmmoStyle->ApplyChanges();
	m_pHUDColor->ApplyChanges();
	m_pHUDBackgroundAlpha->ApplyChanges();
	m_pRadarScale->ApplyChanges();
	m_pAlwaysShowInventory->ApplyChanges();
	m_pRadarRotate->ApplyChanges();
	m_pRadarSquare->ApplyChanges();

	ConVarRef cl_menu_background( "cl_menu_background" );
	if ( cl_menu_background.GetInt() != m_pMenuBackground->GetActiveItem() )
	{
		m_pMenuBackground->ApplyChanges();
		BasePanel()->RestartBackgroundVideo();
	}

	m_pMenuAgent->ApplyChanges();

	// update agent on main menu
	GameUI().UpdateAgentModel();
}