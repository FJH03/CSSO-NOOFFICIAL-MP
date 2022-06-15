//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "iclientmode.h"
#include "cs_hud_weaponselection.h"
#include "c_cs_player.h"
#include "cs_gamerules.h"
#include "weapon_basecsgrenade.h"
#include "cs_loadout.h"
#include <vgui/ILocalize.h>
#include <vgui_controls/AnimationController.h>

ConVar cl_showloadout( "cl_showloadout", "1", FCVAR_ARCHIVE, "Toggles display of current loadout." );
extern ConVar cl_hud_color;
extern ConVar cl_draw_only_deathnotices;

DECLARE_HUDELEMENT( CCSHudWeaponSelection );

CCSHudWeaponSelection::CCSHudWeaponSelection( const char* pElementName ) : CHudElement( pElementName ), BaseClass( NULL, "CSHudWeaponSelection" )
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	SetHiddenBits( HIDEHUD_NOT_OBSERVING_PLAYERS );

	m_flUpdateInventoryAt = -1;
	m_bUpdateInventoryReset = false;
	m_nPrevWepAlignSlot = -1;
	V_memset( m_weaponPanels, 0, sizeof( m_weaponPanels ) );
	m_clrNotSelected = Color( 255, 255, 255, 255 );

	m_pDefuserIcon = new VectorImagePanel( pParent, "DefuserIcon" ); // has to be the parent of viewport so it isn't affected by alpha
	m_pDefuserIcon->ClearSchemeUpdateFlag(); // again, I will not even risk that cuz there is no color overrider
	m_bHasDefuser = false;

	for ( int i = 0; i < MAX_WEP_SELECT_PANELS; i++ )
	{
		m_pSlotLabels[i] = NULL;
	}

	ListenForGameEvent( "round_prestart" );
	ListenForGameEvent( "round_start" );
	ListenForGameEvent( "bot_takeover" );
	ListenForGameEvent( "spec_mode_updated" );
	ListenForGameEvent( "spec_target_updated" );
	ListenForGameEvent( "hltv_changed_mode" );
	ListenForGameEvent( "item_equip" );
}

void CCSHudWeaponSelection::OnScreenSizeChanged( int iOldWide, int iOldTall )
{
	// force recalculation of some stuff
	m_iHUDColor = -1;
}

void CCSHudWeaponSelection::ApplySettings( KeyValues *inResourceData )
{
	BaseClass::ApplySettings( inResourceData );

	m_pDefuserIcon->SetRenderSize( defuser_icon_wide, defuser_icon_tall );
	m_pDefuserIcon->SetTexture( inResourceData->GetString( "defuser_icon", NULL ) );
	m_pDefuserIcon->SetPos( GetWide() - icons_base_xpos - m_pDefuserIcon->GetWide() + defuser_icon_xpos,
							defuser_icon_ypos );
	m_pDefuserIcon->SetVisible( false );
}

void CCSHudWeaponSelection::OnThink()
{
	if ( m_iHUDColor != cl_hud_color.GetInt() )
	{
		m_iHUDColor = cl_hud_color.GetInt();
		Color clr = gHUD.GetHUDColor( cl_hud_color.GetInt() );
		m_clrNotSelected = Color( clr.r()*0.5f, clr.g()*0.5f, clr.b()*0.5f, clr.a() );

		UpdateIconColors();
	}

	C_CSPlayer *pPlayer = GetHudPlayer();
	if ( !pPlayer )
		return;

	if ( m_bHasDefuser != pPlayer->HasDefuser() )
	{
		m_bHasDefuser = pPlayer->HasDefuser();
		m_pDefuserIcon->SetFgColor( m_clrDefuser ); // it gets updated after initializing so just change it here
		m_pDefuserIcon->SetVisible( m_bHasDefuser );
		ShowAndUpdateSelection( WEPSELECT_SWITCH ); // update panel positions
	}
}

void CCSHudWeaponSelection::ProcessInput( void )
{
	if ( m_flUpdateInventoryAt >= gpGlobals->curtime )
	{
		if ( m_bUpdateInventoryReset )
		{
			RemoveAllItems();
			m_bUpdateInventoryReset = false;
		}

		ShowAndUpdateSelection( WEPSELECT_SWITCH );
		m_flUpdateInventoryAt = -1;
	}
}

void CCSHudWeaponSelection::LevelShutdown( void )
{
	for ( int i=0; i < MAX_WEP_SELECT_PANELS; i++ )
	{
		for ( int j=0; j < MAX_WEP_SELECT_POSITIONS; j++ )
		{
			//Remove all items before we exit
			RemoveWeapon(i,j);
		}
	}

	V_memset( m_weaponPanels, 0, sizeof( m_weaponPanels ) );
}

void CCSHudWeaponSelection::AddWeapon( C_BaseCombatWeapon *pWeapon, bool bSelected )
{
	if ( !pWeapon || !C_CSPlayer::GetLocalCSPlayer() )
		return;

	int nWepSlot = pWeapon->GetSlot();
	int nWepPos = pWeapon->GetPosition();

	CWeaponCSBase *pCSWeapon = (CWeaponCSBase*) pWeapon;
	C_BasePlayer *pPlayer = GetHudPlayer();
	if ( !pCSWeapon || !pPlayer || !pCSWeapon->GetPlayerOwner() || pCSWeapon->GetPlayerOwner() != pPlayer )
		return;

	//Put the new weapon in the list
	if ( !m_weaponPanels[nWepSlot][nWepPos].bInitialized )
	{
		m_weaponPanels[nWepSlot][nWepPos] = CreateNewPanel( nWepSlot, nWepPos, pWeapon, bSelected );
	}
	else
	{
		m_weaponPanels[nWepSlot][nWepPos].hWeapon = pWeapon;
		m_weaponPanels[nWepSlot][nWepPos].bInitialized = true;
		m_weaponPanels[nWepSlot][nWepPos].bSelected = bSelected;
	}
	m_weaponPanels[nWepSlot][nWepPos].pSVGPanel->SetRenderSize( weapon_icon_wide, weapon_icon_tall );
	m_weaponPanels[nWepSlot][nWepPos].pSVGPanel->SetTexture( UTIL_VarArgs( "materials/vgui/weapons/svg/%s.svg", pCSWeapon->GetClassname() + 7 ) );

	if ( pCSWeapon->HasStatTrak() )
	{
		wchar_t wszLocalized[256];
		g_pVGuiLocalize->ConstructString( wszLocalized, sizeof( wszLocalized ), g_pVGuiLocalize->Find( "#Cstrike_WPNHUD_StatTrak" ), 1, g_pVGuiLocalize->Find( pCSWeapon->GetPrintName() ) );
		m_weaponPanels[nWepSlot][nWepPos].pNameLabel->SetText( wszLocalized );
	}
	else
	{
		m_weaponPanels[nWepSlot][nWepPos].pNameLabel->SetText( pCSWeapon->GetPrintName() );
	}

	m_weaponPanels[nWepSlot][nWepPos].pNameLabel->SizeToContents();
	UpdateCountLabels();
	UpdateSlotLabels();

	// force a weapon switch to catch where we got a user message but not the network update, yet
	m_flUpdateInventoryAt = gpGlobals->curtime + 0.1;
}

void CCSHudWeaponSelection::RemoveWeapon( int nSlot, int nPos )
{
	if ( nSlot< 0 || nSlot >= MAX_WEP_SELECT_PANELS )
		return;

	if ( m_weaponPanels[nSlot][nPos].bInitialized )
	{
		if ( m_weaponPanels[nSlot][nPos].pSVGPanel )
		{
			m_weaponPanels[nSlot][nPos].pSVGPanel->DeletePanel();
			m_weaponPanels[nSlot][nPos].pSVGPanel = NULL;
		}
		if ( m_weaponPanels[nSlot][nPos].pCountLabel )
		{
			m_weaponPanels[nSlot][nPos].pCountLabel->DeletePanel();
			m_weaponPanels[nSlot][nPos].pCountLabel = NULL;
		}
		if ( m_weaponPanels[nSlot][nPos].pNameLabel )
		{

			m_weaponPanels[nSlot][nPos].pNameLabel->DeletePanel();
			m_weaponPanels[nSlot][nPos].pNameLabel = NULL;
		}
		m_weaponPanels[nSlot][nPos].bShowCountNumber = false;
		m_weaponPanels[nSlot][nPos].hWeapon = NULL;
		m_weaponPanels[nSlot][nPos].bInitialized = false;
		m_weaponPanels[nSlot][nPos].bSelected = false;
	}

	// force a weapon switch to catch where we got a user message but not the network update, yet
	m_flUpdateInventoryAt = gpGlobals->curtime + 0.1;
}

void CCSHudWeaponSelection::RemoveAllItems( void )
{
	//Remove all items before we exit
	for ( int i = 0; i < MAX_WEP_SELECT_PANELS; i++ )
	{
		for ( int j = 0; j < MAX_WEP_SELECT_POSITIONS; j++ )
		{
			//Remove all items before we exit
			RemoveWeapon( i, j );
		}
	}

	V_memset( m_weaponPanels, 0, sizeof( m_weaponPanels ) );
}

WeaponSelectPanel CCSHudWeaponSelection::CreateNewPanel( int nSlot, int nPos, C_BaseCombatWeapon *pWeapon, bool bSelected )
{
	if ( pWeapon == NULL || C_CSPlayer::GetLocalCSPlayer() == NULL )
	{
		bSelected = false;
	}

	//Put the new panel in the list
	WeaponSelectPanel newPanel;

	const char *szSVGPanelName = UTIL_VarArgs( "WeaponSelectPanel_%d_%d", nSlot, nPos );
	VectorImagePanel *pSVGPanel = dynamic_cast<VectorImagePanel*>(FindChildByName( szSVGPanelName )); // the panel might already exist
	if ( !pSVGPanel )
		pSVGPanel = new VectorImagePanel( this, szSVGPanelName ); // create one if it doesn't exist
	if ( pSVGPanel )
	{
		pSVGPanel->ClearSchemeUpdateFlag(); // DO NOT change the fucking FgColor!!
		pSVGPanel->SetMirrorX( true );
	}
	newPanel.pSVGPanel = pSVGPanel;

	const char *szNameLabelName = UTIL_VarArgs( "WeaponSelectPanel_NameLabel_%d_%d", nSlot, nPos );
	Label *pNameLabel = dynamic_cast<Label*>(FindChildByName( szNameLabelName )); // the panel might already exist
	if ( !pNameLabel )
		pNameLabel = new Label( this, szNameLabelName, "" ); // create one if it doesn't exist
	if ( pNameLabel )
	{
		pNameLabel->ClearSchemeUpdateFlag(); // I will not even risk with it after last time
		pNameLabel->SetFont( m_hNameLabelFont );
		pNameLabel->SetFgColor( m_clrSelected );
		pNameLabel->SetContentAlignment( Label::a_east );
		pNameLabel->SetVisible( bSelected );
	}
	newPanel.pNameLabel = pNameLabel;

	CWeaponCSBase *pCSWeapon = (CWeaponCSBase*) pWeapon;
	if ( nSlot == 3 || (pCSWeapon && nSlot == 4 && pCSWeapon->IsKindOf( WEAPONTYPE_STACKABLEITEM )) )
	{
		const char *szCountLabelName = UTIL_VarArgs( "WeaponSelectPanel_CountLabel_%d_%d", nSlot, nPos );
		Label *pCountLabel = dynamic_cast<Label*>(FindChildByName( szCountLabelName )); // the panel might already exist
		if ( !pCountLabel )
			pCountLabel = new Label( this, szCountLabelName, "" ); // create one if it doesn't exist
		if ( pCountLabel )
		{
			pCountLabel->ClearSchemeUpdateFlag(); // I will not even risk with it after last time
			pCountLabel->SetFont( m_hCountLabelFont );
		}

		newPanel.pCountLabel = pCountLabel;
		newPanel.bShowCountNumber = true;
	}

	newPanel.hWeapon = pWeapon;
	newPanel.bInitialized = true;
	newPanel.bSelected = bSelected;

	return newPanel;
}

void CCSHudWeaponSelection::ShowAndUpdateSelection( int nType, C_BaseCombatWeapon *pWeapon )
{
	if ( !pWeapon || pWeapon->IsMarkedForDeletion() )
	{
		pWeapon = NULL;
		nType = WEPSELECT_SWITCH;
	}

	C_BasePlayer *pPlayer = GetHudPlayer();
	if ( !pPlayer )
		return;

	CHudWeaponSelection *pHudSelection = (CHudWeaponSelection *)GET_HUDELEMENT( CHudWeaponSelection );
	if ( !pHudSelection )
		return;

	m_hSelectedWeapon = (pWeapon && nType == WEPSELECT_SWITCH) ? pWeapon : pPlayer->GetActiveWeapon();

	C_WeaponCSBase *pNextWeapon= NULL;

	switch ( nType )
	{
	default:
	case WEPSELECT_SWITCH :
		{
			bool bFoundSelectedItem = false;
			// loop through all of our slots and update differences
			for ( int i=0; i < MAX_WEP_SELECT_PANELS; i++ )
			{
				for ( int j=0; j < MAX_WEP_SELECT_POSITIONS; j++ )
				{
					C_WeaponCSBase *pPanelWeapon = dynamic_cast<C_WeaponCSBase*>(m_weaponPanels[i][j].hWeapon.Get());
					pNextWeapon = dynamic_cast<C_WeaponCSBase*>(pHudSelection->GetWeaponInSlotForTarget( pPlayer, i, j ));
					CBaseCSGrenade *pGrenade = NULL;

					// we have a weapon in our list, but we don't have it in our inventory, remove it
					if ( pPanelWeapon && !pNextWeapon )
					{
						RemoveWeapon( i, j );
					}
					else if ( pNextWeapon && !pPanelWeapon )
					{
						if ( pNextWeapon->GetCSWpnData().m_WeaponType == WEAPONTYPE_GRENADE )
						{
							pGrenade = static_cast<CBaseCSGrenade*>( pNextWeapon );
						}
						// this is an edge case where this happens when the very first round starts
						// but we can't add grenades back after they've been thrown because they are set as thrown before they've left our inventory.....
						// if it's not a grenade, OR if its a grenade and hasn't been thrown, add it back
						// we are awarded bonus grenades late during gun gun arsenal mode, so we have to catch them here
						if ( !pGrenade || (pGrenade && !pGrenade->IsPinPulled() && !pGrenade->IsBeingThrown() && !pGrenade->GetIsThrown()) )
						{
							AddWeapon( pNextWeapon, (GetSelectedWeapon() == pNextWeapon) );
						}
					}
					
					if ( pPanelWeapon && pNextWeapon && pPanelWeapon == pNextWeapon )
					{
						bool bJustRemovedGrenade = false;
						if ( pPanelWeapon->GetCSWpnData().m_WeaponType == WEAPONTYPE_GRENADE )
						{
							pGrenade = static_cast<CBaseCSGrenade*>( pPanelWeapon );
 							bool bShouldRemove = true;
 							int ammo = pPanelWeapon->UsesPrimaryAmmo() ? pPanelWeapon->Clip1() : 0;
 							if ( ammo < 0 )
 								ammo = pPanelWeapon->GetReserveAmmoCount( AMMO_POSITION_PRIMARY );
 
 							if ( ammo > 0 )
 								bShouldRemove = false;
 
 							if ( bShouldRemove )
 							{
 								RemoveWeapon( i, j );
 								bJustRemovedGrenade = true;
 							}

//							pGrenade = static_cast<CBaseCSGrenade*>( pPanelWeapon );
//							if ( pGrenade && pGrenade->GetIsThrown() && pGrenade->IsPinPulled() )
//							{
//								RemoveWeapon( i, j );
//								bJustRemovedGrenade = true;
//							}

						}

						// if we just removed the panel because we just threw the grenade, don't count it as selected because it's about to be removed
						if ( GetSelectedWeapon() == pPanelWeapon && !bJustRemovedGrenade )
							bFoundSelectedItem = true;

						m_weaponPanels[i][j].bSelected = (GetSelectedWeapon() == pPanelWeapon);
					}
				}
			}

			// if we didn't find the item selected, try again in a short while
			if ( bFoundSelectedItem == false )
			{
				m_flUpdateInventoryAt = gpGlobals->curtime + 0.1;
			}

			break;
		}

	case WEPSELECT_PICKUP :
		{
			bool bSelected = (pWeapon == GetSelectedWeapon());
			if ( pWeapon )
			{
				AddWeapon( pWeapon, bSelected );
			}
			break;
		}

	case WEPSELECT_DROP :
		{
			if ( pWeapon )
			{
				int nWepSlot = pWeapon->GetSlot();
				int nWepPos = pWeapon->GetPosition();
				C_WeaponCSBase *pPanelWeapon = dynamic_cast<C_WeaponCSBase*>(m_weaponPanels[nWepSlot][nWepPos].hWeapon.Get());
				// we're dropping a weapon so just find it, remove it and be done
				if ( pPanelWeapon && pWeapon && pPanelWeapon == pWeapon )
				{
					bool bShouldRemove = true;
					if ( pPanelWeapon && pPanelWeapon->GetCSWpnData().m_WeaponType == WEAPONTYPE_GRENADE )
					{
						int ammo = pPanelWeapon->UsesPrimaryAmmo() ? pPanelWeapon->Clip1() : 0;
						if ( ammo < 0 )
							ammo = pPanelWeapon->GetReserveAmmoCount( AMMO_POSITION_PRIMARY );

						if ( ammo > 0 )
							bShouldRemove = false;
					}

					if ( bShouldRemove )
						RemoveWeapon( nWepSlot, nWepPos );
				}
			}

			// force a weapon switch to catch where we got a user message but not the network update, yet
			m_flUpdateInventoryAt = gpGlobals->curtime + 0.1;

			break;
		}
	}

	UpdateIconColors();
	UpdateCountLabels();
	UpdateSlotLabels();
	UpdatePanelPositions();

	if ( !cl_showloadout.GetBool() )
 	{
 		g_pClientMode->GetViewportAnimationController()->CancelAnimationsForPanel( this );
 		g_pClientMode->GetViewportAnimationController()->RunAnimationCommand( this, "alpha", 0, WEAPON_SELECTION_FADE_DELAY, WEAPON_SELECTION_FADE_TIME_SEC, vgui::AnimationController::INTERPOLATOR_LINEAR );
 	}
}

void CCSHudWeaponSelection::UpdatePanelPositions( void )
{
	SetAlpha( 255 );

	int nYPos = 0;
	int nXPos = 0;
	bool bFirstTime = true;

	int nStartWepSlot = -1;
	int nStartWepSlotPos = -1;
	bool bReverse = true;
	if ( bReverse )
	{
		nStartWepSlot = MAX_WEAPON_SLOTS;
		nStartWepSlotPos = MAX_WEAPON_POSITIONS;
	}

	C_CSPlayer *pLocalPlayer = C_CSPlayer::GetLocalCSPlayer();
	if ( !pLocalPlayer )
		return;

	CHudWeaponSelection *pHudSelection = (CHudWeaponSelection *) GET_HUDELEMENT( CHudWeaponSelection );
	if ( !pHudSelection )
		return;

	C_CSPlayer *pPlayer = GetHudPlayer();
	if ( !pPlayer )
		return;

	C_WeaponCSBase *pNextWeapon = NULL;
	for ( int i = MAX_WEP_SELECT_PANELS - 1; i >= 0; i-- )
	{
		for ( int j = 0; j < MAX_WEP_SELECT_POSITIONS; j++ )
		{
			if ( !m_weaponPanels[i][j].bInitialized )
				continue;

			C_WeaponCSBase *pPanelWeapon = static_cast<C_WeaponCSBase*>(m_weaponPanels[i][j].hWeapon.Get());

			pNextWeapon = static_cast<C_WeaponCSBase*>(pHudSelection->GetWeaponInSlotForTarget( pPlayer, i, j ));
			// little hack checking knives because we remove the first knife given on initial spawn and the panel doesn't pick up the change until next update, so the knife isn't shown on initial spawn int eh first map
			if ( pPanelWeapon && pNextWeapon && (pPanelWeapon == pNextWeapon || (CSLoadout()->IsKnife( pPanelWeapon->GetCSWeaponID() ) && CSLoadout()->IsKnife( pNextWeapon->GetCSWeaponID() ))) )
			{
				int nIconWide = m_weaponPanels[i][j].pSVGPanel->GetWide();
				int nSlot = pPanelWeapon->GetSlot();
				bool bShowCountNumber = m_weaponPanels[i][j].bShowCountNumber;
				bool bSelected = m_weaponPanels[i][j].bSelected;
				int nNameLabelWide, nNameLabelTall;
				m_weaponPanels[i][j].pNameLabel->GetSize( nNameLabelWide, nNameLabelTall );

				if ( bFirstTime )
				{
					nXPos = GetWide() - icons_base_xpos - nIconWide;

					if ( pPlayer && pPlayer->HasDefuser() )
					{
						nYPos = icons_base_ypos - weapon_icon_defuser_margin;
					}
					else
					{
						nYPos = icons_base_ypos;
					}

					bFirstTime = false;
				}
				else
				{
					bool bShiftLeft = false;
					if ( m_nPrevWepAlignSlot == nSlot )
					{
						bShiftLeft = true;
					}
					else
					{
						nXPos = GetWide() - icons_base_xpos - nIconWide;
					}

					if ( bShiftLeft )
					{
						// grenades are 3
						if ( nSlot == 3 )
							nXPos -= nIconWide+(weapon_icon_pos_margin*0.75);
						else
							nXPos -= nIconWide+(weapon_icon_pos_margin*1.25);
					}
					else
					{
						nYPos -= weapon_icon_slot_margin;
					}
				}

				m_weaponPanels[i][j].pSVGPanel->SetPos( nXPos, nYPos );
				m_weaponPanels[i][j].pNameLabel->SetPos( nXPos + nIconWide - nNameLabelWide + name_label_xpos, nYPos + name_label_ypos );
				m_weaponPanels[i][j].pNameLabel->SetVisible( bSelected );
				if ( bShowCountNumber )
					m_weaponPanels[i][j].pCountLabel->SetPos( nXPos + count_label_xpos, nYPos + count_label_ypos );

				m_nPrevWepAlignSlot = nSlot;
			}
		}
	}
}

void CCSHudWeaponSelection::UpdateIconColors()
{
	for ( int nSlot = 0; nSlot < MAX_WEP_SELECT_PANELS; nSlot++ )
	{
		for ( int nPos = 0; nPos < MAX_WEP_SELECT_POSITIONS; nPos++ )
		{
			bool bSelected = m_weaponPanels[nSlot][nPos].bSelected;
			if ( m_weaponPanels[nSlot][nPos].bInitialized )
			{
				m_weaponPanels[nSlot][nPos].pSVGPanel->SetFgColor( bSelected ? m_clrSelected : m_clrNotSelected );
				m_weaponPanels[nSlot][nPos].pNameLabel->SetFgColor( m_clrSelected );
				if ( m_weaponPanels[nSlot][nPos].bShowCountNumber )
				{
					Color clrCountNotSelected( m_clrSelected.r()*0.5f, m_clrSelected.g()*0.5f, m_clrSelected.b()*0.5f, m_clrSelected.a() );
					m_weaponPanels[nSlot][nPos].pCountLabel->SetFgColor( bSelected ? m_clrSelected : clrCountNotSelected );
				}
			}
		}
	}
}

void CCSHudWeaponSelection::UpdateCountLabels()
{
	for ( int nSlot = 0; nSlot < MAX_WEP_SELECT_PANELS; nSlot++ )
	{
		for ( int nPos = 0; nPos < MAX_WEP_SELECT_POSITIONS; nPos++ )
		{
			if ( !m_weaponPanels[nSlot][nPos].bShowCountNumber )
			{
				if ( m_weaponPanels[nSlot][nPos].pCountLabel )
				{
					m_weaponPanels[nSlot][nPos].pCountLabel->DeletePanel();
					m_weaponPanels[nSlot][nPos].pCountLabel = NULL;
				}
				continue;
			}

			C_BaseCombatWeapon *pWeapon = dynamic_cast<C_BaseCombatWeapon*>(m_weaponPanels[nSlot][nPos].hWeapon.Get());
			if ( !pWeapon )
			{
				if ( m_weaponPanels[nSlot][nPos].pCountLabel )
				{
					m_weaponPanels[nSlot][nPos].pCountLabel->DeletePanel();
					m_weaponPanels[nSlot][nPos].pCountLabel = NULL;
				}
				m_weaponPanels[nSlot][nPos].bShowCountNumber = false;
				continue;
			}

			int ammo = pWeapon->UsesPrimaryAmmo() ? pWeapon->Clip1() : 0;
			if ( ammo < 0 )
				ammo = pWeapon->GetReserveAmmoCount( AMMO_POSITION_PRIMARY );

			m_weaponPanels[nSlot][nPos].pCountLabel->SetVisible( ammo > 1 );
			wchar_t buf[32];
			V_snwprintf( buf, ARRAYSIZE( buf ), L"x%d", ammo );
			m_weaponPanels[nSlot][nPos].pCountLabel->SetText( buf );
		}
	}
}
void CCSHudWeaponSelection::UpdateSlotLabels()
{
	C_CSPlayer* pPlayer = GetHudPlayer();
	if ( !pPlayer )
		return;

	int iXPos = 0, iYPos = 0;
	bool bFirstTime = true;
	for ( int i = MAX_WEP_SELECT_PANELS - 1; i >= 0; i-- )
	{
		if ( pPlayer->Weapon_GetSlot( i ) )
		{
			if ( !m_pSlotLabels[i] )
			{
				char szPanelName[128];
				Q_snprintf( szPanelName, sizeof( szPanelName ), "WeaponSelectPanel_SlotLabel_%d", i );

				wchar_t wszSlot[4];
				V_snwprintf( wszSlot, sizeof( wszSlot ), L"%d", i+1 );

				m_pSlotLabels[i] = new Label( this, szPanelName, wszSlot );
				m_pSlotLabels[i]->ClearSchemeUpdateFlag(); // I will not even risk with it after last time
				m_pSlotLabels[i]->SetFont( m_hSlotLabelFont );
				m_pSlotLabels[i]->SetFgColor( m_clrSelected );
				m_pSlotLabels[i]->SetContentAlignment( Label::a_east );
			}

			if ( bFirstTime )
			{
				iXPos = GetWide() - icons_base_xpos + slot_label_xpos;
				iYPos = icons_base_ypos + slot_label_ypos;
				if ( pPlayer->HasDefuser() )
					iYPos -= weapon_icon_defuser_margin;

				bFirstTime = false;
			}
			else
			{
				iYPos -= weapon_icon_slot_margin;
			}
			m_pSlotLabels[i]->SetPos( iXPos, iYPos );
		}
		else
		{
			if ( m_pSlotLabels[i] )
			{
				m_pSlotLabels[i]->DeletePanel();
				m_pSlotLabels[i] = NULL;
			}
		}
	}
}
void CCSHudWeaponSelection::FireGameEvent( IGameEvent *event )
{
	const char *type = event->GetName();
	C_CSPlayer *pLocalPlayer = C_CSPlayer::GetLocalCSPlayer();
	if ( !pLocalPlayer )
		return;

	int nPlayerUserID = pLocalPlayer->GetUserID();
	int nEventUserID = event->GetInt( "userid" );

	if ( StringHasPrefix( type, "round_start" ) )
	{
		ShowAndUpdateSelection( WEPSELECT_SWITCH );
		m_flUpdateInventoryAt = gpGlobals->curtime; // update it the next frame
		m_bUpdateInventoryReset = true;
	}

	if ( ((Q_strcmp( "bot_takeover", type ) == 0 || Q_strcmp( "spec_target_updated", type ) == 0 || Q_strcmp( "item_equip", type ) == 0) && nPlayerUserID == nEventUserID) )
	{
		ShowAndUpdateSelection( WEPSELECT_SWITCH );
		m_flUpdateInventoryAt = gpGlobals->curtime + 0.1;
		m_bUpdateInventoryReset = true;
	}
	else if ( (Q_strcmp( "spec_mode_updated", type ) == 0 && nPlayerUserID == nEventUserID) || Q_strcmp( "hltv_changed_mode", type ) == 0 )
	{
		ShowAndUpdateSelection( WEPSELECT_SWITCH );
		m_flUpdateInventoryAt = gpGlobals->curtime + 0.1;
	}
	else if ( Q_strcmp( "item_equip", type ) == 0 )
 	{
 		C_CSPlayer *pPlayer = GetHudPlayer();
 		if ( pPlayer && pPlayer->GetUserID() == nEventUserID )
 		{
 			ShowAndUpdateSelection( WEPSELECT_SWITCH );
 			m_flUpdateInventoryAt = gpGlobals->curtime + 0.1;
 			m_bUpdateInventoryReset = true;
 		}
 	}
}

bool CCSHudWeaponSelection::ShouldDraw()
{
	if ( cl_draw_only_deathnotices.GetBool() )
		return false;

	return CHudElement::ShouldDraw();
}
