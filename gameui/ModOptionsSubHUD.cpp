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
#include "EngineInterface.h"
#include "tier1/convar.h"

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
	m_pSimplePlayerModelLighting = new CLabeledCommandComboBox( this, "SimplePlayerModelLightingComboBox" );
	m_pHUDColor = new CLabeledCommandComboBox( this, "HUDColorComboBox" );
	m_pHUDBackgroundAlpha = new CCvarSlider( this, "HUDBackgroundAlphaSlider", "", 0.0f, 1.0f, "cl_hud_background_alpha" );

	m_pPlayerCountPos->AddItem( "#GameUI_HUD_PlayerCount_Top", "hud_playercount_pos 0" );
	m_pPlayerCountPos->AddItem( "#GameUI_HUD_PlayerCount_Bottom", "hud_playercount_pos 1" );

	m_pHealthAmmoStyle->AddItem( "#GameUI_HUD_HealthAmmoStyle_0", "cl_hud_healthammo_style 0" );
	m_pHealthAmmoStyle->AddItem( "#GameUI_HUD_HealthAmmoStyle_1", "cl_hud_healthammo_style 1" );

	m_pSimplePlayerModelLighting->AddItem( "#GameUI_HUD_SimplePlayerModelLighting_0", "cl_simple_player_lighting 0" );
	m_pSimplePlayerModelLighting->AddItem( "#GameUI_HUD_SimplePlayerModelLighting_1", "cl_simple_player_lighting 1" );

	char localization[64];
	char command[64];
	for ( int i = 0; i < MAX_HUD_COLORS; i++ )
	{
		Q_snprintf( localization, sizeof( localization ), "#GameUI_HUD_Color_%d", i );
		Q_snprintf( command, sizeof( command ), "cl_hud_color %d", i );
		m_pHUDColor->AddItem( localization, command );
	}

	m_pPlayerCountPos->AddActionSignalTarget( this );
	m_pHealthAmmoStyle->AddActionSignalTarget( this );
	m_pSimplePlayerModelLighting->AddActionSignalTarget( this );
	m_pHUDColor->AddActionSignalTarget( this );
	m_pHUDBackgroundAlpha->AddActionSignalTarget( this );

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

	ConVarRef cl_simple_player_lighting( "cl_simple_player_lighting" );
	if ( cl_simple_player_lighting.IsValid() )
		m_pSimplePlayerModelLighting->SetInitialItem( cl_simple_player_lighting.GetInt() );

	ConVarRef cl_hud_color( "cl_hud_color" );
	if ( cl_hud_color.IsValid() )
		m_pHUDColor->SetInitialItem( cl_hud_color.GetInt() );

	m_pHUDBackgroundAlpha->Reset();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CModOptionsSubHUD::OnApplyChanges()
{
	m_pPlayerCountPos->ApplyChanges();
	m_pHealthAmmoStyle->ApplyChanges();
	m_pSimplePlayerModelLighting->ApplyChanges();
	m_pHUDColor->ApplyChanges();
	m_pHUDBackgroundAlpha->ApplyChanges();
}