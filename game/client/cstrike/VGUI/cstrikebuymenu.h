//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef CSTRIKEBUYMENU_H
#define CSTRIKEBUYMENU_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Frame.h>
#include <vgui_controls/Button.h>
#include <game/client/iviewport.h>
#include "cs_weapon_parse.h"

using namespace vgui;

class C_CSPlayer;
class CAvatarImagePanel;

enum BuyMenuSpecialMessageType_t
{
	InvalidMessage = -1,
	PerWeaponMessage,
	GlobalMessage,
};

enum BuyMenuLoadoutIconType_t
{
	AssaultSuit = 0,
	Kevlar,
	Defuser,
	Taser,
	SmokeGrenade,
	HEGrenade,
	Flashbang,
	DecoyGrenade,
	IncGrenade,
	Molotov,
	SecondaryWeapon,
	BombWeapon,
	PrimaryWeapon,

	TotalIconTypes
};


class BuyMenuItemIcon
{
public:
	BuyMenuItemIcon()
	{
		m_nTextureID = -1;
		m_iTextureSize[0] = m_iTextureSize[1] = 0;
		m_flTextureCoords[0] = m_flTextureCoords[1] =
		m_flTextureCoords[2] = m_flTextureCoords[3] = 0.0f;
		m_nItemID = WEAPON_NONE;
	}

	void SetTexture( const char* pszTexturePath, int iWide, int iTall, CSWeaponID nItemID );
	void DestroyTexture();

	int m_nTextureID;
	int m_iTextureSize[2];
	float m_flTextureCoords[4]; // s0, t0, s1, t1
	CSWeaponID m_nItemID;
};

class CCSBuyMenuCategoryButton: public Button
{
	DECLARE_CLASS_SIMPLE( CCSBuyMenuCategoryButton, Button );
public:
	CCSBuyMenuCategoryButton( Panel* parent, const char* panelName );

	virtual void Paint( void );
	
private:
	CPanelAnimationVar( HFont, m_hHotkeyFont, "hotkey_font", "BuyMenuHotkeyFont" );
	CPanelAnimationVar( Color, m_clrHotkey, "hotkey_color", "128 128 128 255" );
	CPanelAnimationVarAliasType( int, hotkey_xpos, "hotkey_xpos", "8", "proportional_xpos" );
	CPanelAnimationVarAliasType( int, hotkey_ypos, "hotkey_ypos", "8", "proportional_ypos" );
};

class CCSBuyMenuItemButton: public CCSBuyMenuCategoryButton
{
	DECLARE_CLASS_SIMPLE( CCSBuyMenuItemButton, CCSBuyMenuCategoryButton );
public:
	CCSBuyMenuItemButton( Panel* parent, const char* panelName );
	~CCSBuyMenuItemButton();

	virtual void Paint( void );
	virtual void OnThink( void );
	virtual void OnCursorEntered();
	virtual void OnCursorExited();

	void SetName( const char* pszName );
	void SetDescription( const char* pszDescription );
	void SetPrice( int iPrice );
	void SetWeaponID( CSWeaponID nWeaponID );
	void SetDisabled( bool bDisabled ) { m_bDisabled = bDisabled; }
	void SetDropBuy( bool bDropBuy ) { m_bDropBuy = bDropBuy; }
	void SetIcon( const char* pszTexturePath ) { m_ItemIcon.SetTexture( pszTexturePath, icon_wide, icon_tall, m_nItemID ); }

private:
	CPanelAnimationVar( Color, m_clrPriceAvailableFg, "fgcolor_price_available", "144 238 144 255" );
	CPanelAnimationVar( Color, m_clrPriceUnavailableFg, "fgcolor_price_unavailable", "255 0 0 255" );
	CPanelAnimationVar( Color, m_clrAvailableFg, "fgcolor_available", "255 255 255 255" );
	CPanelAnimationVar( Color, m_clrAvailableBg, "bgcolor_available", "36 36 36 255" );
	CPanelAnimationVar( Color, m_clrUnavailableFg, "fgcolor_unavailable", "169 169 169 255" );
	CPanelAnimationVar( Color, m_clrUnavailableBg, "bgcolor_unavailable", "24 24 24 255" );
	CPanelAnimationVarAliasType( int, icon_xpos, "icon_xpos", "8", "proportional_xpos" );
	CPanelAnimationVarAliasType( int, icon_wide, "icon_wide", "0", "proportional_width" );
	CPanelAnimationVarAliasType( int, icon_tall, "icon_tall", "24", "proportional_height" );

	const char* m_pszItemName;
	const char* m_pszItemDescription;
	int m_iPrice;
	BuyMenuItemIcon m_ItemIcon;
	wchar_t m_wszPrice[8];
	CSWeaponID m_nItemID;
	CCSWeaponInfo* m_pWeaponInfo;
	Color m_clrPrice;
	Color m_clrIcon;
	bool m_bDisabled;
	bool m_bDropBuy;
};

#define MATERIAL_MAX_LIGHT_COUNT 4
class CCSBuyMenuPlayerImage: public EditablePanel
{
	DECLARE_CLASS_SIMPLE( CCSBuyMenuPlayerImage, EditablePanel );
public:
	CCSBuyMenuPlayerImage( Panel* parent, const char* panelName );
	~CCSBuyMenuPlayerImage();

	virtual void Paint();
	virtual void ApplySettings( KeyValues* inResourceData );
	virtual void OnMousePressed( vgui::MouseCode code );
	virtual void OnMouseReleased( vgui::MouseCode code );
	virtual void OnCursorMoved( int x, int y );
	virtual void OnCursorExited();

	void ParseLightInfo( KeyValues* inResourceData );
	void SetPlayerModel( const char* pszModel );
	void SetWeaponModel( const char* pszModel );
	void SetGlovesModel( const char* pszModel );
	void SetSequence( const char* pszSequence );
	void ResetRotation();

	bool DoesModelSupportGloves( const char* pszGlovesViewModelName, const char* pszDefaultViewModelName );

private:
	int m_nFOV;
	CTextureReference m_DefaultCubemap;
	Vector m_vecCameraPos;
	QAngle m_angCameraAng;
	Vector m_vecAmbientLight;
	LightDesc_t m_pLightDesc[MATERIAL_MAX_LIGHT_COUNT];

	int m_nNumLightDescs;
	int m_nLastMouseX;
	int m_nLastMouseY;
	float m_flRotationAngleLeft;
	float m_flRotationTimeLeft;
	bool m_bMousePressed;

	CHandle<C_BaseAnimating> m_hPlayerModel;
	CHandle<C_BaseAnimating> m_hWeaponModel;
	CHandle<C_BaseAnimating> m_hGlovesModel;
};

class CCSBuyMenuLoadoutPanel: public EditablePanel
{
	DECLARE_CLASS_SIMPLE( CCSBuyMenuLoadoutPanel, EditablePanel );
public:
	CCSBuyMenuLoadoutPanel( Panel* parent, const char* panelName );
	~CCSBuyMenuLoadoutPanel();

	virtual void ApplySchemeSettings( IScheme* pScheme );
	virtual void Paint( void );

	void SetPlayer( C_CSPlayer* pPlayer );

private:
	C_CSPlayer* m_pPlayer;
	CAvatarImagePanel* m_pPlayerAvatarImage;
	VectorImagePanel* m_pPlayerSkullImage;
	BuyMenuItemIcon m_ItemIcons[TotalIconTypes];

	CPanelAnimationVarAliasType( int, avatar_xpos, "avatar_xpos", "w36", "proportional_xpos" );
	CPanelAnimationVarAliasType( int, avatar_ypos, "avatar_ypos", "2", "proportional_ypos" );
	CPanelAnimationVarAliasType( int, avatar_wide, "avatar_wide", "32", "proportional_width" );
	CPanelAnimationVarAliasType( int, avatar_tall, "avatar_tall", "32", "proportional_height" );
	CPanelAnimationVarAliasType( int, skull_xpos, "skull_xpos", "w32", "proportional_xpos" );
	CPanelAnimationVarAliasType( int, skull_ypos, "skull_ypos", "6", "proportional_ypos" );
	CPanelAnimationVarAliasType( int, skull_wide, "skull_wide", "24", "proportional_width" );
	CPanelAnimationVarAliasType( int, skull_tall, "skull_tall", "24", "proportional_height" );
	CPanelAnimationVarAliasType( int, money_xpos, "money_xpos", "w8", "proportional_xpos" );
	CPanelAnimationVarAliasType( int, money_ypos, "money_ypos", "12", "proportional_ypos" );
	CPanelAnimationVarAliasType( int, icons_wide, "icons_wide", "24", "proportional_width" );
	CPanelAnimationVarAliasType( int, icons_tall, "icons_tall", "24", "proportional_height" );
	CPanelAnimationVarAliasType( int, armor_icons_size, "armor_icons_size", "20", "proportional_height" );
	CPanelAnimationVarAliasType( int, icons_xpos, "icons_xpos", "152", "proportional_xpos" );
	CPanelAnimationVarAliasType( int, icons_margin, "icons_margin", "w8", "proportional_width" );
	CPanelAnimationVar( Color, m_clrMoney, "money_color", "204 204 204 255" );
	CPanelAnimationVar( HFont, m_hMoneyFont, "money_font", "BuyMenuLoadoutMoneyFont" );
};

class CCSBuyMenu: public Frame, public IViewPortPanel
{
	DECLARE_CLASS_SIMPLE( CCSBuyMenu, Frame );
public:
	CCSBuyMenu( IViewPort* pViewPort );
	~CCSBuyMenu();

	// IViewPortPanel overrides
	virtual const char* GetName( void ) { return PANEL_BUY; }
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
	virtual void PaintBackground();
	virtual void OnCommand( const char* command );
	virtual vgui::Panel* CreateControlByName( const char* controlName );
	virtual void OnClose();
	virtual void OnKeyCodeTyped( KeyCode code );
	virtual void OnKeyCodePressed( KeyCode code );
	virtual void OnKeyCodeReleased( KeyCode code );

	void ShowCategory( KeyValues* kvCategory );
	void HideCategory();
	void SetPlayerImageWeapon( const char* pszWeaponModel, const char* pszWeaponSequence );
	void SetItemNameAndDescription( const char* pszName, const char* pszDescription );
	void ResetWeapon();
	void ShowSpecialMessage( const char* pszText, BuyMenuSpecialMessageType_t nMessageType );
	void ShowSpecialMessage( const wchar_t* pwszText, BuyMenuSpecialMessageType_t nMessageType );
	void HideSpecialMessage( BuyMenuSpecialMessageType_t nMessageType );

private:
	Label* m_pMoneyLabel;
	Label* m_pSpecialMessageLabel;
	Label* m_pBuyTimeLeftLabel;
	Label* m_pItemNameLabel;
	Label* m_pItemDescriptionLabel;
	Panel* m_pBuyItemsBackground;
	CCSBuyMenuPlayerImage* m_pPlayerModel;

	KeyValues* m_kvBuyMenuConfig;
	IViewPort* m_pViewPort;

	ButtonCode_t m_iBuyMenuKey;
	ButtonCode_t m_iReBuyKey;
	ButtonCode_t m_iAutoBuyKey;
	bool m_bShowingCategory;
	int m_iAccount;
	bool m_bDropBuy;
	BuyMenuSpecialMessageType_t m_nMessageType;
	int m_iBuyTimeLeft;
};

#endif // CSTRIKEBUYMENU_H