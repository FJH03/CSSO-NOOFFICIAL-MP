//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================

#ifndef CS_HUD_FREEZEPANEL_H
#define CS_HUD_FREEZEPANEL_H
#ifdef _WIN32
#pragma once
#endif

#include <KeyValues.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui_controls/AnimationController.h>
#include <vgui_controls/EditablePanel.h>
#include "vgui/ILocalize.h"
#include "vgui_avatarimage.h"
#include "hud.h"
#include "hudelement.h"
#include "vgui_borderprogress.h"

#include "cs_shareddefs.h"

using namespace vgui;

class ContinuousProgressBarWithBorder;

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CCSFreezePanel : public EditablePanel, public CHudElement
{
private:
	DECLARE_CLASS_SIMPLE( CCSFreezePanel, EditablePanel );

public:
	CCSFreezePanel( const char *pElementName );

	virtual void Reset();
	virtual void Init();

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void FireGameEvent( IGameEvent * event );
	virtual bool ShouldDraw();
	virtual void OnScreenSizeChanged(int nOldWide, int nOldTall);

	virtual void SetActive( bool bActive );

	void Show();
	void Hide();

	int	HudElementKeyInput( int down, ButtonCode_t keynum, const char *pszCurrentBinding );

protected:

private:
	CAvatarImagePanel*						m_pAvatar;
	vgui::ContinuousProgressBarWithBorder*	m_pKillerHealth;
	ImagePanel*								m_pDominationIcon;
	ImagePanel*								m_pDamageTakenBackground;
	ImagePanel*								m_pDamageGivenBackground;
	Label*									m_pDamageTakenLabel;
	Label*									m_pDamageGivenLabel;
	Label*									m_pScreenshotLabel;

	bool					m_bShouldBeVisible;

	CPanelAnimationVarAliasType( int, screenshot_label_ypos, "screenshot_label_ypos", "0", "proportional_ypos" );
	CPanelAnimationVarAliasType( int, screenshot_label_ypos_damage_taken, "screenshot_label_ypos_damage_taken", "0", "proportional_ypos" );
	CPanelAnimationVarAliasType( int, screenshot_label_ypos_damage_given, "screenshot_label_ypos_damage_given", "0", "proportional_ypos" );
};

#endif //CS_HUD_FREEZEPANEL_H