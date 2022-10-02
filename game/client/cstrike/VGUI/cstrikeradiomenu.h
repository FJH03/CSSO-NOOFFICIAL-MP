//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef CSRADIOMENU_H
#define CSRADIOMENU_H
#ifdef _WIN32
#pragma once
#endif

#include "vgui_controls/Frame.h"
#include "vgui_controls/SectionedListPanel.h"
#include "game/client/iviewport.h"

using namespace vgui;

class CCSRadioMenu : public Frame, public IViewPortPanel
{
	DECLARE_CLASS_SIMPLE( CCSRadioMenu, Frame );

public:
	CCSRadioMenu( IViewPort* pViewPort );

	// IViewPortPanel overrides
	virtual const char* GetName( void ) { return PANEL_RADIO_MENU; }
	virtual void SetData( KeyValues* data ) {}
	virtual void Reset() {}
	virtual void Update() {}
	virtual bool NeedsUpdate( void ) { return false; }
	virtual bool HasInputElements( void ) { return true; }
	vgui::VPANEL GetVPanel( void ) { return BaseClass::GetVPanel(); }
	virtual bool IsVisible() { return BaseClass::IsVisible(); }
	virtual void SetParent( vgui::VPANEL parent ) { BaseClass::SetParent( parent ); }
	virtual void ShowPanel( bool bShow );

	void LoadRadioCommands();
	void PlayRadioCommand();
	MESSAGE_FUNC_PARAMS( OnItemDoubleLeftClick, "ItemDoubleLeftClick", pKV );

private:
	SectionedListPanel* m_pRadioList;
	IViewPort* m_pViewPort;
};

#endif // CSRADIOMENU_H
