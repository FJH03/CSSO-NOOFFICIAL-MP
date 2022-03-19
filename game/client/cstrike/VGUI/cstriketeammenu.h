//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef CSTEAMMENU_H
#define CSTEAMMENU_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Frame.h>
#include <vgui_controls/Button.h>
#include <game/client/iviewport.h>

using namespace vgui;

#define MATERIAL_MAX_LIGHT_COUNT 4
class CCSTeamMenuAgentImage: public Button
{
	DECLARE_CLASS_SIMPLE( CCSTeamMenuAgentImage, Button );
public:
	CCSTeamMenuAgentImage( Panel* parent, const char* panelName, int nTeamNumber );
	~CCSTeamMenuAgentImage();

	virtual void Paint();
	virtual void ApplySettings( KeyValues* inResourceData );
	virtual void OnCursorEntered();
	virtual void OnCursorExited();

	void ParseLightInfo( KeyValues* inResourceData );
	void SetPlayerModel( const char* pszModel );
	void SetWeaponModel( const char* pszModel );
	void SetGlovesModel( const char* pszModel );
	void SetSequence( const char* pszSequence, float flSequenceFade );

	bool DoesModelSupportGloves( const char* pszGlovesViewModelName, const char* pszDefaultViewModelName );

private:
	int m_nFOV;
	CTextureReference m_DefaultCubemap;
	Vector m_vecCameraPos;
	QAngle m_angCameraAng;
	Vector m_vecAmbientLight;
	LightDesc_t m_pLightDesc[MATERIAL_MAX_LIGHT_COUNT];
	int m_nNumLightDescs;
	int m_nTeamNumber;

	CHandle<C_BaseAnimating> m_hPlayerModel;
	CHandle<C_BaseAnimating> m_hWeaponModel;
	CHandle<C_BaseAnimating> m_hGlovesModel;

	CPanelAnimationVarAliasType( int, viewport_xpos, "viewport_xpos", "0", "proportional_xpos" );
	CPanelAnimationVarAliasType( int, viewport_ypos, "viewport_ypos", "0", "proportional_ypos" );
	CPanelAnimationVarAliasType( int, viewport_wide, "viewport_wide", "0", "proportional_width" );
	CPanelAnimationVarAliasType( int, viewport_tall, "viewport_tall", "0", "proportional_height" );
};

class CCSTeamMenu: public Frame, public IViewPortPanel
{
	DECLARE_CLASS_SIMPLE( CCSTeamMenu, Frame );
public:
	CCSTeamMenu( IViewPort* pViewPort );

	// IViewPortPanel overrides
	virtual const char* GetName( void ) { return PANEL_TEAM; }
	virtual void SetData( KeyValues* data ) {}
	virtual void Reset() {}
	virtual void Update() {}
	virtual bool NeedsUpdate( void ) { return false; }
	virtual bool HasInputElements( void ) { return true; }
	vgui::VPANEL GetVPanel( void ) { return BaseClass::GetVPanel(); }
	virtual bool IsVisible() { return BaseClass::IsVisible(); }
	virtual void SetParent( vgui::VPANEL parent ) { BaseClass::SetParent( parent ); }
	virtual void ShowPanel( bool bShow );

	// vgui overrides
	virtual void PaintBackground();
	virtual void OnClose();
	virtual void OnCommand( const char* command );
	virtual void OnKeyCodeTyped( KeyCode code );

	void ResetAgentModels();
	void UpdatePlayerCount();

private:
	CCSTeamMenuAgentImage* m_pAgentModelT;
	CCSTeamMenuAgentImage* m_pAgentModelCT;
	Button* m_pCancelButton;
	Button* m_pSpectateButton;
	Button* m_pAutoAssignButton;
	Label* m_pTPlayerCount;
	Label* m_pCTPlayerCount;

	IViewPort* m_pViewPort;
};

#endif // CSTEAMMENU_H