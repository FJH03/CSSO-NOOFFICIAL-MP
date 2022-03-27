//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef CS_HUD_WEAPONSELECTION_H
#define CS_HUD_WEAPONSELECTION_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/VectorImagePanel.h>
#include <vgui_controls/Label.h>
#include "hudelement.h"
#include "cs_weapon_selection.h"
#include "weapon_csbase.h"

using namespace vgui;

#define MAX_WEP_SELECT_PANELS 6
#define MAX_WEP_SELECT_POSITIONS 6
#define WEAPON_SELECTION_FADE_TIME_SEC 0.1
#define WEAPON_SELECTION_FADE_DELAY 5.0

struct WeaponSelectPanel
{
	WeaponSelectPanel()
	{
		pSVGPanel = NULL;
		pNameLabel = NULL;
		pCountLabel = NULL;
		bShowCountNumber = false;
		hWeapon = NULL;
		bInitialized = false;
		bSelected = false;
	}

	VectorImagePanel *pSVGPanel;
	Label *pNameLabel;
	Label *pCountLabel;
	bool bShowCountNumber;
	EHANDLE hWeapon;
	bool bInitialized;
	bool bSelected;
};


// CHudWeaponSelection is already taken :(
class CCSHudWeaponSelection: public CHudElement, public EditablePanel
{
	DECLARE_CLASS_SIMPLE( CCSHudWeaponSelection, EditablePanel );
public:
	CCSHudWeaponSelection( const char *pElementName );
	virtual void ApplySettings( KeyValues *inResourceData );
	virtual void OnThink();
	virtual void ProcessInput( void );
	virtual void LevelShutdown( void );
	virtual void OnScreenSizeChanged( int iOldWide, int iOldTall );
	virtual bool ShouldDraw();

	void AddWeapon( C_BaseCombatWeapon *pWeapon, bool bSelected );
	void RemoveWeapon( int nSlot, int nPos );
	void RemoveAllItems( void );
	WeaponSelectPanel CreateNewPanel( int nSlot, int nPos, C_BaseCombatWeapon *pWeapon = NULL, bool bSelected = false );
	void ShowAndUpdateSelection( int nType = WEPSELECT_SWITCH, C_BaseCombatWeapon *pWeapon = NULL );
	void UpdatePanelPositions( void );
	void UpdateIconColors();
	void UpdateCountLabels();
	void UpdateSlotLabels();

protected:
	virtual C_WeaponCSBase	*GetSelectedWeapon( void )
	{
		return dynamic_cast<C_WeaponCSBase*>(m_hSelectedWeapon.Get());
	}

	// CGameEventListener methods
	virtual void FireGameEvent( IGameEvent *event );

private:
	WeaponSelectPanel m_weaponPanels[MAX_WEP_SELECT_PANELS][MAX_WEP_SELECT_POSITIONS];
	CHandle< C_BaseCombatWeapon > m_hSelectedWeapon;

	float	m_flUpdateInventoryAt;
	bool	m_bUpdateInventoryReset;

	Color	m_clrNotSelected;

	int m_nPrevWepAlignSlot;

	VectorImagePanel *m_pDefuserIcon;
	bool m_bHasDefuser;
	Label *m_pSlotLabels[MAX_WEP_SELECT_PANELS];
	CPanelAnimationVarAliasType( int, icons_base_xpos, "icons_base_xpos", "0", "proportional_xpos" );
	CPanelAnimationVarAliasType( int, icons_base_ypos, "icons_base_ypos", "0", "proportional_ypos" );
	CPanelAnimationVarAliasType( int, name_label_xpos, "name_label_xpos", "0", "proportional_xpos" );
	CPanelAnimationVarAliasType( int, name_label_ypos, "name_label_ypos", "0", "proportional_ypos" );
	CPanelAnimationVarAliasType( int, slot_label_xpos, "slot_label_xpos", "0", "proportional_xpos" );
	CPanelAnimationVarAliasType( int, slot_label_ypos, "slot_label_ypos", "0", "proportional_ypos" );

	CPanelAnimationVarAliasType( int, count_label_xpos, "count_label_xpos", "0", "proportional_xpos" );
	CPanelAnimationVarAliasType( int, count_label_ypos, "count_label_ypos", "0", "proportional_ypos" );
	CPanelAnimationVarAliasType( int, weapon_icon_wide, "weapon_icon_wide", "0", "proportional_width" );
	CPanelAnimationVarAliasType( int, weapon_icon_tall, "weapon_icon_tall", "0", "proportional_height" );
	CPanelAnimationVarAliasType( int, weapon_icon_slot_margin, "weapon_icon_slot_margin", "0", "proportional_height" );
	CPanelAnimationVarAliasType( int, weapon_icon_pos_margin, "weapon_icon_pos_margin", "0", "proportional_width" );
	CPanelAnimationVarAliasType( int, weapon_icon_defuser_margin, "weapon_icon_defuser_margin", "0", "proportional_height" );
	CPanelAnimationVarAliasType( int, defuser_icon_xpos, "defuser_icon_xpos", "0", "proportional_xpos" );
	CPanelAnimationVarAliasType( int, defuser_icon_ypos, "defuser_icon_ypos", "0", "proportional_ypos" );
	CPanelAnimationVarAliasType( int, defuser_icon_wide, "defuser_icon_wide", "0", "proportional_width" );
	CPanelAnimationVarAliasType( int, defuser_icon_tall, "defuser_icon_tall", "0", "proportional_height" );
	CPanelAnimationVar( Color, m_clrSelected, "selected_weapon_color", "White" );
	CPanelAnimationVar( Color, m_clrDefuser, "defuser_fgcolor", "White" );
	CPanelAnimationVar( HFont, m_hNameLabelFont, "name_label_font", "WeaponSelectionNameLabelFont" );
	CPanelAnimationVar( HFont, m_hCountLabelFont, "count_label_font", "WeaponSelectionCountLabelFont" );
	CPanelAnimationVar( HFont, m_hSlotLabelFont, "count_label_font", "WeaponSelectionSlotLabelFont" );

};

#endif	//CS_HUD_WEAPONSELECTION_H
