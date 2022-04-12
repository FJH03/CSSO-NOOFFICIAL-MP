//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#ifndef CSCLASSMENU_H
#define CSCLASSMENU_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Frame.h>
#include <vgui_controls/Button.h>
#include <game/client/iviewport.h>
#include "cstriketeammenu.h"
#include "cs_shareddefs.h"

using namespace vgui;

class CCSClassMenu: public Frame, public IViewPortPanel
{
	DECLARE_CLASS_SIMPLE( CCSClassMenu, Frame );
public:
	CCSClassMenu( IViewPort* pViewPort );

	// IViewPortPanel overrides
	virtual const char* GetName( void ) { return PANEL_CLASS; }
	virtual void SetData( KeyValues* data ) {}
	virtual void Reset() {}
	virtual void Update();
	virtual bool NeedsUpdate( void ) { return true; }
	virtual bool HasInputElements( void ) { return true; }
	vgui::VPANEL GetVPanel( void ) { return BaseClass::GetVPanel(); }
	virtual bool IsVisible() { return BaseClass::IsVisible(); }
	virtual void SetParent( vgui::VPANEL parent ) { BaseClass::SetParent( parent ); }
	virtual void ShowPanel( bool bShow );

	// vgui overrides
	virtual void OnClose();
	virtual void OnCommand( const char* command );
	virtual void OnKeyCodeTyped( KeyCode code );

private:
	Button* m_pCancelButton;
	Button* m_pAutoAssignButton;

	IViewPort* m_pViewPort;
};

class CCSClassMenu_TER: public CCSClassMenu
{
	DECLARE_CLASS_SIMPLE( CCSClassMenu_TER, CCSClassMenu );
public:
	CCSClassMenu_TER( IViewPort* pViewPort );

	virtual const char* GetName( void ) { return PANEL_CLASS_TER; }
	virtual void ShowPanel( bool bShow );

	void ResetAgentModels();

private:
	CCSTeamMenuAgentImage* m_pAgentModels[LAST_T_CLASS];
};

class CCSClassMenu_CT: public CCSClassMenu
{
	DECLARE_CLASS_SIMPLE( CCSClassMenu_CT, CCSClassMenu );
public:
	CCSClassMenu_CT( IViewPort* pViewPort );

	virtual const char* GetName( void ) { return PANEL_CLASS_CT; }
	virtual void ShowPanel( bool bShow );

	void ResetAgentModels();

private:
	CCSTeamMenuAgentImage* m_pAgentModels[LAST_T_CLASS];
};

#endif // CSCLASSMENU_H
