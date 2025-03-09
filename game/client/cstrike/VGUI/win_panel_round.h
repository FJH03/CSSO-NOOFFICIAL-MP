//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Create and display a win panel at the end of a round displaying interesting stats and info about the round.
//
// $NoKeywords: $
//=============================================================================//

#ifndef CSWINPANEL_ROUND_H
#define CSWINPANEL_ROUND_H
#ifdef _WIN32
#pragma once
#endif

#include "VGUI/bordered_panel.h"
#include <game/client/iviewport.h>
#include <vgui/IScheme.h>
#include "hud.h"
#include "hudelement.h"
#include "c_cs_player.h"
#include "vgui_avatarimage.h"

#include "cs_shareddefs.h"

using namespace vgui;

class WinPanel_Round: public EditablePanel, public CHudElement
{
private:
	DECLARE_CLASS_SIMPLE( WinPanel_Round, EditablePanel );

public:
	WinPanel_Round(const char *pElementName);

	virtual void Reset();
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void FireGameEvent( IGameEvent * event );
	virtual void OnScreenSizeChanged( int iOldWide, int iOldTall );

	void Show();
	void Hide();

protected:
	void SetMVP( C_CSPlayer* pPlayer, CSMvpReason_t reason );

private:
	Label* m_pWinLabel;
	Label* m_pFunFactLabel;
	Label* m_pMVPText;
	ImagePanel* m_pMainBackground;
	ImagePanel* m_pTeamIcon;
	CAvatarImagePanel* m_pMVPAvatar;

	Color m_clrCT;
	Color m_clrT;

	CPanelAnimationVarAliasType( int, mvp_avatar_margin, "mvp_avatar_margin", "0", "proportional_width" );
};

#endif //CSWINPANEL_ROUND_H