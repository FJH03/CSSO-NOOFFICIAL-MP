//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "cstrikebuymenu.h"
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include <vgui/IInput.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/VectorImagePanel.h>
#include "filesystem.h"
#include "IGameUIFuncs.h" // for key bindings
#include "VGuiMatSurface/IMatSystemSurface.h"
#include "view_shared.h"
#include "view.h"
#include "model_types.h"
#include "vgui_avatarimage.h"
#include "cs_hud_weaponselection.h"
#include "viewpostprocess.h"

#include "c_cs_player.h"
#include "cs_loadout.h"
#include "c_breakableprop.h"
#include "ammodef.h"

#include "lunasvg/lunasvg.h"
using namespace lunasvg;

#ifdef ANDROID
#define TOUCH_DISABLE_ON_BUYMENU "1"
#else
#define TOUCH_DISABLE_ON_BUYMENU "0"
#endif

ConVar closeonbuy( "closeonbuy", "0", FCVAR_ARCHIVE, "Set non-zero to close the buy menu after buying something", true, 0, true, 1 );
ConVar touch_disable_on_buymenu ( "touch_disable_on_buymenu", "0", FCVAR_ARCHIVE, "Set non-zero to disable touch for entering to buymenu  ", true, 0, true, 1 );

void BuyMenuItemIcon::SetTexture( const char* pszTexturePath, int iWide, int iTall, CSWeaponID nItemID )
{
	// don't even bother doing anything without a file
	if ( !pszTexturePath )
		return;

	DestroyTexture();

	FileHandle_t f = g_pFullFileSystem->Open( pszTexturePath, "rt" );
	if ( !f )
	{
		DevWarning( "BuyMenuItemIcon: failed to open .svg file \"%s\".\n", pszTexturePath );
		return;
	}

	// read the whole thing into memory
	int size = g_pFullFileSystem->Size( f );
	// read into temporary memory block
	int nBufSize = size + 1;
	if ( IsXbox() )
	{
		nBufSize = AlignValue( nBufSize, 512 );
	}
	char* pMem = (char*) malloc( nBufSize );
	int bytesRead = g_pFullFileSystem->ReadEx( pMem, nBufSize, size, f );
	Assert( bytesRead <= size );
	pMem[bytesRead] = 0;
	g_pFullFileSystem->Close( f );
	std::unique_ptr<Document> document = Document::loadFromData( pMem ); // load the svg
	free( pMem );

	if ( !document )
	{
		DevWarning( "BuyMenuItemIcon: failed to load .svg file \"%s\".\n", pszTexturePath );
		return;
	}

	Bitmap bitmap = document->renderToBitmap( iWide, iTall ); // render the svg

	if ( !bitmap.valid() )
	{
		DevWarning( "BuyMenuItemIcon: failed to render .svg file \"%s\".\n", pszTexturePath );
		return;
	}

	if ( m_nTextureID == -1 )
	{
		m_nTextureID = vgui::surface()->CreateNewTextureID( true );
	}

	int wide = bitmap.width();
	int tall = bitmap.height();
	m_iTextureSize[0] = wide;
	m_iTextureSize[1] = tall;
	vgui::surface()->DrawSetTextureRGBA( m_nTextureID, bitmap.data(), wide, tall, 1, true );

	int textureWide, textureTall;
	vgui::surface()->DrawGetTextureSize( m_nTextureID, textureWide, textureTall );

	m_flTextureCoords[0] = (float) wide / (float) textureWide;
	m_flTextureCoords[1] = 0.0f;
	m_flTextureCoords[2] = 0.0f;
	m_flTextureCoords[3] = (float) tall / (float) textureTall;

	m_nItemID = nItemID;
}

void BuyMenuItemIcon::DestroyTexture()
{
	if ( m_nTextureID != -1 )
	{
		vgui::surface()->DestroyTextureID( m_nTextureID );
		m_nTextureID = -1;
		m_iTextureSize[0] = m_iTextureSize[1] = 0;
		m_flTextureCoords[0] = m_flTextureCoords[1] =
		m_flTextureCoords[2] = m_flTextureCoords[3] = 0.0f;
		m_nItemID = WEAPON_NONE;
	}
}


CCSBuyMenuCategoryButton::CCSBuyMenuCategoryButton( Panel* parent, const char* panelName ):
	Button( parent, panelName, "" )
{
}

void CCSBuyMenuCategoryButton::Paint()
{
	BaseClass::Paint();

	if ( GetHotKey() != '\0' )
	{
		// draw the hotkey on top
		surface()->DrawSetTextFont( m_hHotkeyFont );
		surface()->DrawSetTextPos( hotkey_xpos, hotkey_ypos );
		surface()->DrawSetTextColor( m_clrHotkey );
		surface()->DrawUnicodeChar( GetHotKey() );
	}
}


CCSBuyMenuItemButton::CCSBuyMenuItemButton( Panel* parent, const char* panelName ):
	CCSBuyMenuCategoryButton( parent, panelName )
{
	m_pszItemName = NULL;
	m_pszItemDescription = NULL;
	m_iPrice = -1;
	m_wszPrice[0] = '\0';
	m_nItemID = WEAPON_NONE;
	m_pWeaponInfo = NULL;
	m_clrPrice = COLOR_WHITE;
	m_clrIcon = COLOR_WHITE;
	m_bDisabled = false;
	m_bDropBuy = false;
}

CCSBuyMenuItemButton::~CCSBuyMenuItemButton()
{
	m_ItemIcon.DestroyTexture();
}

void CCSBuyMenuItemButton::Paint()
{
	BaseClass::Paint();

	if ( m_wszPrice[0] != L'\0' )
	{
		HFont hMoneyFont = GetFont();
		int xpos, ypos, wide, tall;
		GetSize( wide, tall );

		xpos = (wide / 2) - (UTIL_ComputeStringWidth( hMoneyFont, m_wszPrice ) / 2);
		ypos = (tall / 2) - (surface()->GetFontTall( hMoneyFont ) / 2);

		surface()->DrawSetTextFont( GetFont() );
		surface()->DrawSetTextPos( xpos, ypos );
		surface()->DrawSetTextColor( m_clrPrice );
		surface()->DrawPrintText( m_wszPrice, wcslen( m_wszPrice ) );
	}

	if ( m_ItemIcon.m_nTextureID != -1 )
	{
		vgui::surface()->DrawSetTexture( m_ItemIcon.m_nTextureID );
		vgui::surface()->DrawSetColor( m_clrIcon );

		g_pMatSystemSurface->DisableClipping( true );

		int x0 = GetWide() - m_ItemIcon.m_iTextureSize[0] - icon_xpos;
		int x1 = x0 + m_ItemIcon.m_iTextureSize[0];
		int y0 = (GetTall() / 2) - (m_ItemIcon.m_iTextureSize[1] / 2);
		int y1 = y0 + m_ItemIcon.m_iTextureSize[1];
		vgui::surface()->DrawTexturedSubRect( x0, y0, x1, y1, m_ItemIcon.m_flTextureCoords[0], m_ItemIcon.m_flTextureCoords[1],
															  m_ItemIcon.m_flTextureCoords[2], m_ItemIcon.m_flTextureCoords[3] );

		g_pMatSystemSurface->DisableClipping( false );
	}
}

void CCSBuyMenuItemButton::OnThink()
{
	C_CSPlayer* pPlayer = C_CSPlayer::GetLocalCSPlayer();
	if ( !pPlayer )
		return;

	m_clrPrice = m_clrPriceAvailableFg;
	m_clrIcon = m_clrAvailableFg;
	SetDefaultColor( m_clrAvailableFg, m_clrAvailableBg );

	AcquireMethod::Type nAcquireMethod = AcquireMethod::Buy;
	if ( m_bDropBuy )
		nAcquireMethod = AcquireMethod::BuyDrop;
	if ( pPlayer->CanAcquire( m_nItemID, nAcquireMethod ) != AcquireResult::Allowed || m_bDisabled )
	{
		m_clrPrice = m_clrUnavailableFg;
		m_clrIcon = m_clrUnavailableFg;
		SetDefaultColor( m_clrUnavailableFg, m_clrUnavailableBg );
	}
	else if ( pPlayer->GetAccount() < m_iPrice )
	{
		m_clrPrice = m_clrPriceUnavailableFg;
		m_clrIcon = m_clrUnavailableFg;
		SetDefaultColor( m_clrUnavailableFg, m_clrAvailableBg );
	}
}

void CCSBuyMenuItemButton::OnCursorEntered()
{
	BaseClass::OnCursorEntered();

	C_CSPlayer* pPlayer = C_CSPlayer::GetLocalCSPlayer();
	if ( !pPlayer )
		return;

	const char* pszItemModel = NULL;
	const char* pszItemSequence = NULL;
	const char* pszItemName = m_pszItemName;
	const char* pszItemDescription = m_pszItemDescription;
	if ( m_pWeaponInfo )
	{
		pszItemDescription = m_pWeaponInfo->m_szDescription;
		pszItemModel = m_pWeaponInfo->szWorldModel;
		if ( pPlayer->GetTeamNumber() == TEAM_TERRORIST )
			pszItemSequence = m_pWeaponInfo->m_szBuyMenuAnimT;
		else
			pszItemSequence = m_pWeaponInfo->m_szBuyMenuAnim;
	}
	else
	{
		switch ( m_nItemID )
		{
			case ITEM_DEFUSER:
				pszItemModel = "models/weapons/w_defuser.mdl";
				pszItemSequence = "t_buymenu_defuser";
				break;
			case ITEM_KEVLAR:
				pszItemModel = "models/weapons/w_eq_armor.mdl";
				pszItemSequence = "t_buymenu_armor_helmet";
				break;
			case ITEM_ASSAULTSUIT:
				pszItemModel = "models/weapons/w_eq_armor_helmet.mdl";
				pszItemSequence = "t_buymenu_armor_helmet";
				break;
			case ITEM_NVGS:
				pszItemModel = "models/weapons/w_eq_nvgs.mdl";
				pszItemSequence = "t_buymenu_nvgs";
				break;
			default:
				DevWarning( "Invalid buy menu weapon!\n" );
				return;
		}
	}

	CCSBuyMenu* pParent = dynamic_cast<CCSBuyMenu*>(GetParent());
	if ( pParent )
	{
		pParent->SetPlayerImageWeapon( pszItemModel, pszItemSequence );
		pParent->SetItemNameAndDescription( pszItemName, pszItemDescription );

		AcquireMethod::Type nAcquireMethod = AcquireMethod::Buy;
		if ( m_bDropBuy )
			nAcquireMethod = AcquireMethod::BuyDrop;
		switch ( pPlayer->CanAcquire( m_nItemID, nAcquireMethod ) )
		{
			case AcquireResult::AlreadyOwned:
			{
				pParent->ShowSpecialMessage( "#CStrike_BuyMenu_AlreadyCarrying", PerWeaponMessage );
				break;
			}
			case AcquireResult::AlreadyPurchased:
			{
				pParent->ShowSpecialMessage( "#CStrike_BuyMenu_AlreadyPurchased", PerWeaponMessage );
				break;
			}
			case AcquireResult::ReachedGrenadeTypeLimit:
			{
				wchar_t wszCarryLimit[8];
				V_snwprintf( wszCarryLimit, sizeof( wszCarryLimit ), L"%d", GetAmmoDef()->MaxCarry( m_pWeaponInfo->iAmmoType, pPlayer ) );

				wchar_t wszMessage[256];
				g_pVGuiLocalize->ConstructString( wszMessage, sizeof( wszMessage ), g_pVGuiLocalize->Find( "#CStrike_BuyMenu_MaxItemsOfType" ), 1, wszCarryLimit );

				pParent->ShowSpecialMessage( wszMessage, PerWeaponMessage );
				break;
			}
			case AcquireResult::ReachedGrenadeTotalLimit:
			{
				wchar_t wszCarryLimit[8];
				V_snwprintf( wszCarryLimit, sizeof( wszCarryLimit ), L"%d", ammo_grenade_limit_total.GetInt() );

				wchar_t wszMessage[256];
				g_pVGuiLocalize->ConstructString( wszMessage, sizeof( wszMessage ), g_pVGuiLocalize->Find( "#CStrike_BuyMenu_CanOnlyCarryXGrenades" ), 1, wszCarryLimit );

				pParent->ShowSpecialMessage( wszMessage, PerWeaponMessage );
				break;
			}
			case AcquireResult::NotAllowedByTeam:
			{
				pParent->ShowSpecialMessage( "#CStrike_BuyMenu_NotAllowedByTeam", PerWeaponMessage );
				break;
			}
			case AcquireResult::NotAllowedByMap:
			{
				pParent->ShowSpecialMessage( "#CStrike_BuyMenu_NotAllowedByMap", PerWeaponMessage );
				break;
			}
			case AcquireResult::NotAllowedByMode:
			{
				pParent->ShowSpecialMessage( "#CStrike_BuyMenu_NotAllowedByMode", PerWeaponMessage );
				break;
			}
			case AcquireResult::NotAllowedForPurchase:
			{
				pParent->ShowSpecialMessage( "#CStrike_BuyMenu_NotAllowedForPurchase", PerWeaponMessage );
				break;
			}
			default:
				break;
		}
	}
}

void CCSBuyMenuItemButton::OnCursorExited()
{
	BaseClass::OnCursorExited();

	CCSBuyMenu* pParent = dynamic_cast<CCSBuyMenu*>(GetParent());
	if ( pParent )
		pParent->HideSpecialMessage( PerWeaponMessage );
}

void CCSBuyMenuItemButton::SetName( const char* pszName )
{
	m_pszItemName = pszName;
	SetText( m_pszItemName );
}

void CCSBuyMenuItemButton::SetDescription( const char* pszDescription )
{
	m_pszItemDescription = pszDescription;
}

void CCSBuyMenuItemButton::SetPrice( int iPrice )
{
	m_iPrice = iPrice;
	V_snwprintf( m_wszPrice, sizeof( m_wszPrice ), L"$%d", iPrice );
}

void CCSBuyMenuItemButton::SetWeaponID( CSWeaponID nWeaponID )
{
	C_CSPlayer* pPlayer = C_CSPlayer::GetLocalCSPlayer();
	if ( !pPlayer )
		return;

	m_nItemID = CSLoadout()->GetLoadoutWeaponID( pPlayer, pPlayer->GetTeamNumber(), nWeaponID );
	m_pWeaponInfo = NULL;
	m_bDisabled = false; // reset it once we have a new weapon

	WEAPON_FILE_INFO_HANDLE	hWpnInfo = LookupWeaponInfoSlot( WeaponIdAsString( m_nItemID ) );
	if ( hWpnInfo != GetInvalidWeaponInfoHandle() )
	{
		m_pWeaponInfo = dynamic_cast<CCSWeaponInfo*>(GetFileWeaponInfoFromHandle( hWpnInfo ));
		if ( m_nItemID != nWeaponID && m_pWeaponInfo )
		{
			wchar_t wszOldHotkey = GetHotKey();
			// the player has a different loadout weapon, change some visuals to match it
			char szItemIcon[128];
			Q_snprintf( szItemIcon, sizeof( szItemIcon ), "materials/vgui/weapons/svg/%s.svg", WeaponIDToAlias( m_nItemID ) );
			m_ItemIcon.SetTexture( szItemIcon, icon_wide, icon_tall, m_nItemID );
			SetPrice( m_pWeaponInfo->GetWeaponPrice() );
			SetText( m_pWeaponInfo->szPrintName );
			SetHotkey( wszOldHotkey ); // restore the old hotkey because it's reset after SetText()
		}
	}
}


CCSBuyMenuPlayerImage::CCSBuyMenuPlayerImage( Panel* parent, const char* panelName ): EditablePanel( parent, panelName )
{
	m_nFOV = 54;
	m_hPlayerModel = NULL;
	m_hWeaponModel = NULL;
	m_hGlovesModel = NULL;
	m_vecCameraPos.Init();
	m_angCameraAng.Init();
	m_nNumLightDescs = 0;
	m_vecAmbientLight.Init( 0.4f, 0.4f, 0.4f );
	m_DefaultCubemap.Init( materials->FindTexture( "editor/cube_vertigo", NULL, true ) );
	m_bMousePressed = false;
	m_flRotationAngleLeft = 0.0f;
	m_flRotationTimeLeft = 0.0f;
}

CCSBuyMenuPlayerImage::~CCSBuyMenuPlayerImage()
{
	if ( m_hPlayerModel.Get() )
	{
		m_hPlayerModel->Remove();
		m_hPlayerModel = NULL;
	}
	if ( m_hWeaponModel.Get() )
	{
		m_hWeaponModel->Remove();
		m_hWeaponModel = NULL;
	}
	if ( m_hGlovesModel.Get() )
	{
		m_hGlovesModel->Remove();
		m_hGlovesModel = NULL;
	}

	m_DefaultCubemap.Shutdown();
}

void CCSBuyMenuPlayerImage::ApplySettings( KeyValues* inResourceData )
{
	BaseClass::ApplySettings( inResourceData );

	const char* pCameraOrigin = inResourceData->GetString( "camera_origin" );
	if ( pCameraOrigin[0] != 0 )
	{
		sscanf( pCameraOrigin, "%f %f %f", &m_vecCameraPos.x, &m_vecCameraPos.y, &m_vecCameraPos.z );
	}
	const char* pCameraAngles = inResourceData->GetString( "camera_angles" );
	if ( pCameraAngles[0] != 0 )
	{
		sscanf( pCameraAngles, "%f %f %f", &m_angCameraAng.x, &m_angCameraAng.y, &m_angCameraAng.z );
	}

	m_nFOV = inResourceData->GetInt( "fov", 54 );

	KeyValues* pData = inResourceData->FindKey( "lights" );
	if ( pData )
	{
		ParseLightInfo( pData );
	}
}

void CCSBuyMenuPlayerImage::OnMousePressed( vgui::MouseCode code )
{
	RequestFocus();

	// Save where they clicked
	input()->GetCursorPosition( m_nLastMouseX, m_nLastMouseY );

	m_bMousePressed = true;
}

void CCSBuyMenuPlayerImage::OnMouseReleased( vgui::MouseCode code )
{
	m_bMousePressed = false;
}

#define ROTATION_TIME 1.0f // seconds
void CCSBuyMenuPlayerImage::OnCursorMoved( int x, int y )
{
	if ( m_bMousePressed )
	{
		int xpos, ypos;
		input()->GetCursorPos( xpos, ypos );

		// Only want the x delta.
		float flDelta = xpos - m_nLastMouseX;

		// Apply the delta and rotate the player.
		if ( !m_hPlayerModel.Get() )
			return;

		m_flRotationAngleLeft += flDelta;
		if ( m_flRotationAngleLeft != 0 )
			m_flRotationTimeLeft = ROTATION_TIME;

		m_nLastMouseX = xpos;
		m_nLastMouseY = ypos;
	}
}

void CCSBuyMenuPlayerImage::OnCursorExited()
{
	m_bMousePressed = false;
}

void CCSBuyMenuPlayerImage::ParseLightInfo( KeyValues* inResourceData )
{
	const char* pAmbientColor = inResourceData->GetString( "ambient_light" );
	if ( pAmbientColor[0] != 0 )
	{
		sscanf( pAmbientColor, "%f %f %f", &(m_vecAmbientLight.x), &(m_vecAmbientLight.y), &(m_vecAmbientLight.z) );
	}

	KeyValues* pLightKeys = inResourceData->GetFirstTrueSubKey();
	while ( pLightKeys )
	{
		if ( m_nNumLightDescs >= MATERIAL_MAX_LIGHT_COUNT )
		{
			DevMsg( "Too many lights defined in %s. Only using first %d. \n", GetName(), MATERIAL_MAX_LIGHT_COUNT );
			break;
		}

		const char* pLightType = pLightKeys->GetName();
		if ( pLightType[0] != 0 )
		{
			LightType_t lightType = MATERIAL_LIGHT_DISABLE;

			if ( V_strnicmp( pLightType, "point_light", 11 ) == 0 )
			{
				lightType = MATERIAL_LIGHT_POINT;
			}
			else if ( V_strnicmp( pLightType, "directional_light", 17 ) == 0 )
			{
				lightType = MATERIAL_LIGHT_DIRECTIONAL;
			}
			else if ( V_strnicmp( pLightType, "spot_light", 10 ) == 0 )
			{
				lightType = MATERIAL_LIGHT_SPOT;
			}
			else
			{
				DevMsg( "Error Parsing lights in %s! Unknown light type %s. \n", GetName(), pLightType );
			}

			if ( lightType != MATERIAL_LIGHT_DISABLE )
			{
				Vector lightPosOrDir( 0, 0, 0 );
				Vector lightColor( 0, 0, 0 );
				const char* pLightPosOrDir = pLightKeys->GetString( (lightType == MATERIAL_LIGHT_DIRECTIONAL) ? "direction" : "position" );
				if ( pLightPosOrDir[0] != 0 )
				{
					sscanf( pLightPosOrDir, "%f %f %f", &(lightPosOrDir.x), &(lightPosOrDir.y), &(lightPosOrDir.z) );
				}
				const char* pLightColor = pLightKeys->GetString( "color" );
				if ( pLightColor[0] != 0 )
				{
					sscanf( pLightColor, "%f %f %f", &(lightColor.x), &(lightColor.y), &(lightColor.z) );
				}

				Vector lightLookAt( 0, 0, 0 );
				float lightInnerCone = 1.0f;
				float lightOuterCone = 10.0f;
				if ( lightType == MATERIAL_LIGHT_SPOT )
				{
					const char* pLightLookAt = pLightKeys->GetString( "lookat" );
					if ( pLightLookAt[0] != 0 )
					{
						sscanf( pLightLookAt, "%f %f %f", &(lightLookAt.x), &(lightLookAt.y), &(lightLookAt.z) );
					}
					lightInnerCone = pLightKeys->GetFloat( "inner_cone", 1.0f );
					lightOuterCone = pLightKeys->GetFloat( "outer_cone", 8.0f );
				}

				switch ( lightType )
				{
					case MATERIAL_LIGHT_DIRECTIONAL:
						m_pLightDesc[m_nNumLightDescs].InitDirectional( lightPosOrDir, lightColor );
						break;
					case MATERIAL_LIGHT_POINT:
						m_pLightDesc[m_nNumLightDescs].InitPoint( lightPosOrDir, lightColor );
						break;
					case MATERIAL_LIGHT_SPOT:
						m_pLightDesc[m_nNumLightDescs].InitSpot( lightPosOrDir, lightColor, lightLookAt, lightInnerCone, lightOuterCone );
						break;
				}
				m_nNumLightDescs++;
			}
		}

		pLightKeys = pLightKeys->GetNextTrueSubKey();
	}
}

void CCSBuyMenuPlayerImage::SetPlayerModel( const char* pszModel )
{
	if ( !pszModel )
	{
		if ( m_hPlayerModel.Get() )
		{
			m_hPlayerModel->Remove();
			m_hPlayerModel = NULL;
		}
		return;
	}

	if ( m_hPlayerModel.Get() )
	{
		m_hPlayerModel->SetModel( pszModel );
	}
	else
	{
		C_BaseAnimating* pEnt = new C_BaseAnimating;
		if ( !pEnt )
			return;
		if ( pEnt->InitializeAsClientEntity( pszModel, RENDER_GROUP_OPAQUE_ENTITY ) == false )
		{
			// we failed to initialize this entity so just return gracefully
			pEnt->Remove();
			return;
		}
		// setup the handle
		m_hPlayerModel = pEnt;
		m_hPlayerModel->DontRecordInTools();
		m_hPlayerModel->AddEffects( EF_NODRAW );
	}
}

void CCSBuyMenuPlayerImage::SetWeaponModel( const char* pszModel )
{
	if ( !pszModel || !m_hPlayerModel.Get() )
	{
		if ( m_hWeaponModel.Get() )
		{
			m_hWeaponModel->Remove();
			m_hWeaponModel = NULL;
		}
		return;
	}

	if ( m_hWeaponModel.Get() )
	{
		m_hWeaponModel->SetModel( pszModel );
	}
	else
	{
		C_BaseAnimating* pEnt = new C_BaseAnimating;
		if ( !pEnt )
			return;
		if ( pEnt->InitializeAsClientEntity( pszModel, RENDER_GROUP_OPAQUE_ENTITY ) == false )
		{
			// we failed to initialize this entity so just return gracefully
			pEnt->Remove();
			return;
		}
		// setup the handle
		m_hWeaponModel = pEnt;
		m_hWeaponModel->DontRecordInTools();
		m_hWeaponModel->AddEffects( EF_NODRAW );
		m_hWeaponModel->FollowEntity( m_hPlayerModel.Get() );
	}
}

void CCSBuyMenuPlayerImage::SetGlovesModel( const char* pszModel )
{
	if ( !pszModel || !m_hPlayerModel.Get() )
	{
		if ( m_hGlovesModel.Get() )
		{
			m_hGlovesModel->Remove();
			m_hGlovesModel = NULL;
		}

		if ( m_hPlayerModel.Get() )
		{
			m_hPlayerModel->SetBodygroup( m_hPlayerModel->FindBodygroupByName( "gloves" ), 0 );
		}

		return;
	}

	if ( m_hGlovesModel.Get() )
	{
		m_hGlovesModel->SetModel( pszModel );
	}
	else
	{
		C_BaseAnimating* pEnt = new C_BaseAnimating;
		if ( !pEnt )
			return;
		if ( pEnt->InitializeAsClientEntity( pszModel, RENDER_GROUP_OPAQUE_ENTITY ) == false )
		{
			// we failed to initialize this entity so just return gracefully
			pEnt->Remove();
			return;
		}
		// setup the handle
		m_hGlovesModel = pEnt;
		m_hGlovesModel->DontRecordInTools();
		m_hGlovesModel->AddEffects( EF_NODRAW );
		m_hGlovesModel->FollowEntity( m_hPlayerModel.Get() );

		m_hPlayerModel->SetBodygroup( m_hPlayerModel->FindBodygroupByName( "gloves" ), 1 );
	}
}

void CCSBuyMenuPlayerImage::SetSequence( const char* pszSequence )
{
	if ( m_hPlayerModel.Get() )
	{
		int sequence = m_hPlayerModel->LookupSequence( pszSequence );
		if ( sequence != ACT_INVALID )
		{
			m_hPlayerModel->ResetSequence( sequence );
			m_hPlayerModel->SetCycle( 0 );
		}
	}
}

void CCSBuyMenuPlayerImage::ResetRotation()
{
	m_bMousePressed = false;
	m_flRotationAngleLeft = 0.0f;
	m_flRotationTimeLeft = 0.0f;

	if ( !m_hPlayerModel.Get() )
		return;
	m_hPlayerModel->SetAbsAngles( vec3_angle );
}

bool CCSBuyMenuPlayerImage::DoesModelSupportGloves(const char* pszGlovesViewModelName, const char* pszDefaultViewModelName )
{
	if ( m_hPlayerModel.Get() )
		return m_hPlayerModel->DoesModelSupportGloves( pszGlovesViewModelName , pszDefaultViewModelName );

	return false;
}

void CCSBuyMenuPlayerImage::Paint()
{
	C_BasePlayer* pLocalPlayer = C_BasePlayer::GetLocalPlayer();

	if ( !pLocalPlayer )
		return;

	MDLCACHE_CRITICAL_SECTION();

	if ( !m_hPlayerModel.Get() )
		return;

	int x, y, w, h;
	GetBounds( x, y, w, h );
	ParentLocalToScreen( x, y );

	if ( m_flRotationTimeLeft > 0.0f )
	{
		QAngle angPlayerModel = m_hPlayerModel->GetAbsAngles();

		float flPercentage = m_flRotationTimeLeft / ROTATION_TIME;
		float flDelta = m_flRotationAngleLeft * flPercentage * gpGlobals->frametime;
		angPlayerModel.y += flDelta;
		m_flRotationAngleLeft -= flDelta;

		if ( angPlayerModel.y > 360.0f )
			angPlayerModel.y -= 360.0f;
		else if ( angPlayerModel.y < -360.0f )
			angPlayerModel.y += 360.0f;
		m_hPlayerModel->SetAbsAngles( angPlayerModel );

		m_flRotationTimeLeft -= gpGlobals->frametime;
	}
	else
	{
		m_flRotationAngleLeft = 0.0f;
	}

	// do we have a valid sequence?
	if ( m_hPlayerModel->GetSequence() != -1 )
	{
		m_hPlayerModel->FrameAdvance( gpGlobals->frametime );
	}

	// Now draw it.
	CViewSetup view;
	view.x = x; // we actually want to offset by the 
	view.y = y; // viewport origin here because Push3DView expects global coords below
	view.width = w;
	view.height = h;

	view.m_bOrtho = false;

	// scale the FOV for aspect ratios other than 4/3
	float flWidthRatio = ((float) w / (float) h) / (4.0f / 3.0f);
	view.fov = ScaleFOVByWidthRatio( m_nFOV, flWidthRatio );

	view.origin = m_vecCameraPos;
	view.angles = m_angCameraAng;
	view.zNear = VIEW_NEARZ;
	view.zFar = 1000;

	CMatRenderContextPtr pRenderContext( materials );

	pRenderContext->BindLocalCubemap( m_DefaultCubemap );

	pRenderContext->SetLightingOrigin( vec3_origin );
	pRenderContext->SetAmbientLight( m_vecAmbientLight.x, m_vecAmbientLight.y, m_vecAmbientLight.z );

	g_pStudioRender->SetLocalLights( m_nNumLightDescs, m_pLightDesc );

	Frustum dummyFrustum;
	render->Push3DView( view, 0, NULL, dummyFrustum );

	modelrender->SuppressEngineLighting( true );
	float color[3] = { 1.0f, 1.0f, 1.0f };
	render->SetColorModulation( color );
	render->SetBlend( 1.0f );
	m_hPlayerModel->DrawModel( STUDIO_RENDER );
	if ( m_hWeaponModel.Get() )
		m_hWeaponModel->DrawModel( STUDIO_RENDER );
	if ( m_hGlovesModel.Get() )
		m_hGlovesModel->DrawModel( STUDIO_RENDER );

	modelrender->SuppressEngineLighting( false );

	render->PopView( dummyFrustum );

	pRenderContext->BindLocalCubemap( NULL );
	pRenderContext.SafeRelease();
}


CCSBuyMenuLoadoutPanel::CCSBuyMenuLoadoutPanel( Panel* parent, const char* panelName ): EditablePanel( parent, panelName )
{
	m_pPlayer = NULL;
	m_pPlayerAvatarImage = new CAvatarImagePanel( this, "PlayerAvatarImage" );
	m_pPlayerSkullImage = new VectorImagePanel( this, "PlayerSkullImage" );
	m_pPlayerSkullImage->SetTexture( "materials/vgui/hud/svg/elimination.svg" );
}

CCSBuyMenuLoadoutPanel::~CCSBuyMenuLoadoutPanel()
{
	m_pPlayer = NULL;

	for ( int i = 0; i < TotalIconTypes; i++ )
	{
		m_ItemIcons[i].DestroyTexture();
	}
}

void CCSBuyMenuLoadoutPanel::ApplySchemeSettings( IScheme* pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	m_pPlayerAvatarImage->SetBounds( GetWide() - avatar_xpos, avatar_ypos, avatar_wide, avatar_tall );
	m_pPlayerAvatarImage->SetShouldDrawFriendIcon( false );
	m_pPlayerAvatarImage->SetShouldScaleImage( true );
	m_pPlayerSkullImage->SetBounds( GetWide() - skull_xpos, skull_ypos, skull_wide, skull_tall );
	m_pPlayerSkullImage->SetZPos( 2 );
	m_pPlayerSkullImage->ClearSchemeUpdateFlag();
	m_pPlayerSkullImage->SetFgColor( COLOR_WHITE );

	m_ItemIcons[AssaultSuit].SetTexture( "materials/vgui/weapons/svg/armor.svg", 0, armor_icons_size, ITEM_ASSAULTSUIT );
	m_ItemIcons[Kevlar].SetTexture( "materials/vgui/hud/svg/shield.svg", 0, armor_icons_size, ITEM_KEVLAR );
	m_ItemIcons[Defuser].SetTexture( "materials/vgui/weapons/svg/defuser.svg", 0, icons_tall, ITEM_DEFUSER );
	m_ItemIcons[Taser].SetTexture( "materials/vgui/weapons/svg/taser.svg", 0, icons_tall, WEAPON_TASER );
	m_ItemIcons[SmokeGrenade].SetTexture( "materials/vgui/weapons/svg/smokegrenade.svg", 0, icons_tall, WEAPON_SMOKEGRENADE );
	m_ItemIcons[HEGrenade].SetTexture( "materials/vgui/weapons/svg/hegrenade.svg", 0, icons_tall, WEAPON_HEGRENADE );
	m_ItemIcons[Flashbang].SetTexture( "materials/vgui/weapons/svg/flashbang.svg", 0, icons_tall, WEAPON_FLASHBANG );
	m_ItemIcons[DecoyGrenade].SetTexture( "materials/vgui/weapons/svg/decoy.svg", 0, icons_tall, WEAPON_DECOY );
	m_ItemIcons[IncGrenade].SetTexture( "materials/vgui/weapons/svg/incgrenade.svg", 0, icons_tall, WEAPON_INCGRENADE );
	m_ItemIcons[Molotov].SetTexture( "materials/vgui/weapons/svg/molotov.svg", 0, icons_tall, WEAPON_MOLOTOV );
	m_ItemIcons[BombWeapon].SetTexture( "materials/vgui/weapons/svg/c4.svg", 0, icons_tall, WEAPON_C4 );
}

void CCSBuyMenuLoadoutPanel::Paint()
{
	BaseClass::Paint();

	if ( !m_pPlayer )
		return;

	wchar_t wszMoney[8];
	V_snwprintf( wszMoney, ARRAYSIZE( wszMoney ), L"$%d", m_pPlayer->GetAccount() );

	int iWide = UTIL_ComputeStringWidth( m_hMoneyFont, wszMoney );
	surface()->DrawSetTextFont( m_hMoneyFont );
	surface()->DrawSetTextPos( m_pPlayerAvatarImage->GetXPos() - money_xpos - iWide, money_ypos );
	surface()->DrawSetTextColor( m_clrMoney );
	surface()->DrawPrintText( wszMoney, wcslen( wszMoney ) );

	int iXPos = GetWide() - icons_xpos;
	for ( int i = 0; i < TotalIconTypes; i++ )
	{
		bool bRender = false;

		m_pPlayerSkullImage->SetVisible( !m_pPlayer->IsAlive() );
		if ( m_pPlayer->IsAlive() )
		{
			switch ( i )
			{
				case AssaultSuit:
					bRender = (m_pPlayer->HasHelmet() && m_pPlayer->ArmorValue() > 0);
					break;
				case Kevlar:
					bRender = (!m_pPlayer->HasHelmet() && m_pPlayer->ArmorValue() > 0);
					break;
				case Defuser:
					bRender = m_pPlayer->HasDefuser();
					break;
				case Taser:
					bRender = (m_pPlayer->Weapon_OwnsThisType( "weapon_taser" ) != NULL);
					break;
				case SmokeGrenade:
					bRender = (m_pPlayer->Weapon_OwnsThisType( "weapon_smokegrenade" ) != NULL);
					break;
				case HEGrenade:
					bRender = (m_pPlayer->Weapon_OwnsThisType( "weapon_hegrenade" ) != NULL);
					break;
				case Flashbang:
					bRender = (m_pPlayer->Weapon_OwnsThisType( "weapon_flashbang" ) != NULL);
					break;
				case DecoyGrenade:
					bRender = (m_pPlayer->Weapon_OwnsThisType( "weapon_decoy" ) != NULL);
					break;
				case IncGrenade:
					bRender = (m_pPlayer->Weapon_OwnsThisType( "weapon_incgrenade" ) != NULL);
					break;
				case Molotov:
					bRender = (m_pPlayer->Weapon_OwnsThisType( "weapon_molotov" ) != NULL);
					break;
				case SecondaryWeapon:
					if ( CWeaponCSBase* pWeapon = dynamic_cast<CWeaponCSBase*>(m_pPlayer->Weapon_GetSlot( WEAPON_SLOT_PISTOL )) )
					{
						bRender = true;
						if ( m_ItemIcons[i].m_nItemID != pWeapon->GetCSWeaponID() )
						{
							char szItemIcon[128];
							Q_snprintf( szItemIcon, sizeof( szItemIcon ), "materials/vgui/weapons/svg/%s.svg", pWeapon->GetClassname() + 7 );
							m_ItemIcons[i].SetTexture( szItemIcon, icons_wide, 0, pWeapon->GetCSWeaponID() );
						}
					}
					break;
				case BombWeapon:
					bRender = m_pPlayer->HasC4();
					break;
				case PrimaryWeapon:
					if ( CWeaponCSBase* pWeapon = dynamic_cast<CWeaponCSBase*>(m_pPlayer->Weapon_GetSlot( WEAPON_SLOT_RIFLE )) )
					{
						bRender = true;
						if ( m_ItemIcons[i].m_nItemID != pWeapon->GetCSWeaponID() )
						{
							char szItemIcon[128];
							Q_snprintf( szItemIcon, sizeof( szItemIcon ), "materials/vgui/weapons/svg/%s.svg", pWeapon->GetClassname() + 7 );
							m_ItemIcons[i].SetTexture( szItemIcon, 0, icons_tall, pWeapon->GetCSWeaponID() );
						}
					}
					break;
			}
		}

		if ( bRender && m_ItemIcons[i].m_nTextureID != -1 )
		{
			vgui::surface()->DrawSetTexture( m_ItemIcons[i].m_nTextureID );
			vgui::surface()->DrawSetColor( COLOR_WHITE );

			g_pMatSystemSurface->DisableClipping( true );

			int x0 = iXPos - m_ItemIcons[i].m_iTextureSize[0];
			int x1 = x0 + m_ItemIcons[i].m_iTextureSize[0];
			int y0 = (GetTall() / 2) - (m_ItemIcons[i].m_iTextureSize[1] / 2);
			int y1 = y0 + m_ItemIcons[i].m_iTextureSize[1];
			vgui::surface()->DrawTexturedSubRect( x0, y0, x1, y1, m_ItemIcons[i].m_flTextureCoords[0], m_ItemIcons[i].m_flTextureCoords[1],
																  m_ItemIcons[i].m_flTextureCoords[2], m_ItemIcons[i].m_flTextureCoords[3] );

			g_pMatSystemSurface->DisableClipping( false );

			iXPos -= m_ItemIcons[i].m_iTextureSize[0];
			iXPos -= icons_margin;
		}
	}
}

void CCSBuyMenuLoadoutPanel::SetPlayer( C_CSPlayer* pPlayer )
{
	if ( !pPlayer )
		return;

	m_pPlayer = pPlayer;
	m_pPlayerAvatarImage->SetPlayer( pPlayer, k_EAvatarSize32x32 );
	m_pPlayerAvatarImage->SetDefaultAvatar( GetDefaultAvatarImage( pPlayer ) );
}


CCSBuyMenu::CCSBuyMenu( IViewPort* pViewPort ): Frame( NULL, PANEL_BUY )
{
	m_pViewPort = pViewPort;

	// initialize dialog
	SetTitle( "", true );

	// load the new scheme early!!
	SetScheme( "ClientScheme" );
	SetMoveable( false );
	SetSizeable( false );

	SetProportional( true );
	SetPaintBackgroundEnabled( true );

	// initialize variables
	m_bShowingCategory = false;
	m_iBuyMenuKey = m_iReBuyKey = m_iAutoBuyKey = BUTTON_CODE_INVALID;
	m_iAccount = -1;
	m_bDropBuy = false;
	m_nMessageType = InvalidMessage;
	m_iBuyTimeLeft = -1;

	// initialize elements
	m_pMoneyLabel = new Label( this, "MoneyLabel", L"" );
	m_pSpecialMessageLabel = new Label( this, "SpecialMessageLabel", L"" );
	m_pBuyTimeLeftLabel = new Label( this, "BuyTimeLeftLabel", L"" );
	m_pItemNameLabel = new Label( this, "ItemNameLabel", L"" );
	m_pItemDescriptionLabel = new Label( this, "ItemDescriptionLabel", L"" );
	m_pBuyItemsBackground = new Panel( this, "BuyItemsBackground" );
	m_pPlayerModel = new CCSBuyMenuPlayerImage( this, "PlayerModel" );
	m_kvBuyMenuConfig = new KeyValues( "BuyMenuConfig" );
	if ( !m_kvBuyMenuConfig->LoadFromFile( g_pFullFileSystem, "scripts/buymenuconfig.txt", "GAME" ) )
		DevWarning( "Failed to load the buy menu config! Buy menu won't work!\n" );

	LoadControlSettings( "Resource/UI/BuyMenu.res" );
}

CCSBuyMenu::~CCSBuyMenu()
{
	m_kvBuyMenuConfig->deleteThis();
}

void CCSBuyMenu::ShowPanel( bool bShow )
{
	C_CSPlayer* pPlayer = C_CSPlayer::GetLocalCSPlayer();
	if ( !pPlayer )
		return;

	bool bTouchEnable = (cvar->FindVar("touch_disable_on_buymenu")->GetInt() != 0);

	if ( bShow )
	{
		// hide the system buttons
		SetTitleBarVisible( false );

		Activate();
		SetMouseInputEnabled( true );

		m_iBuyMenuKey = gameuifuncs->GetButtonCodeForBind( "buymenu" );
		m_iReBuyKey = gameuifuncs->GetButtonCodeForBind( "rebuy" );
		m_iAutoBuyKey = gameuifuncs->GetButtonCodeForBind( "autobuy" );
		m_iAccount = -1;

		const char* pszPlayerModel = modelinfo->GetModelName( pPlayer->GetModel() );
		m_pPlayerModel->SetPlayerModel( pszPlayerModel );
		if ( CSLoadout()->HasGlovesSet( pPlayer, pPlayer->GetTeamNumber() ) )
		{
			const char* pszGlovesViewModel = GetGlovesInfo( CSLoadout()->GetGlovesForPlayer( pPlayer, pPlayer->GetTeamNumber() ) )->szViewModel;
			const char* pszGlovesWorldModel = GetGlovesInfo( CSLoadout()->GetGlovesForPlayer( pPlayer, pPlayer->GetTeamNumber() ) )->szWorldModel;
			if ( pszGlovesViewModel && pPlayer->m_szPlayerDefaultGloves && m_pPlayerModel->DoesModelSupportGloves( pszGlovesViewModel, pPlayer->m_szPlayerDefaultGloves ) )
			{
				m_pPlayerModel->SetGlovesModel( pszGlovesWorldModel );
			}
			else
			{
				m_pPlayerModel->SetGlovesModel( NULL );
			}
		}
		else
		{
			m_pPlayerModel->SetGlovesModel( NULL );
		}
		m_pPlayerModel->ResetRotation(); // reset the mouse state so it wont rotate the player when you get back in
		ResetWeapon();
		m_pItemNameLabel->SetVisible( false );
		m_pItemDescriptionLabel->SetVisible( false );

		int iPanel = 1;
		while ( true )
		{
			char szPanelName[16];
			Q_snprintf( szPanelName, sizeof( szPanelName ), "BuyLoadout%d", iPanel );
			CCSBuyMenuLoadoutPanel* pPanel = dynamic_cast<CCSBuyMenuLoadoutPanel*>(FindChildByName( szPanelName ));
			if ( pPanel )
			{
				pPanel->SetVisible( false );
			}
			else
			{
				// ran out of buttons, stop
				break;
			}
			iPanel++;
		}

		iPanel = 1;
		for ( int i = 0; i <= gpGlobals->maxClients; i++ )
		{
			C_CSPlayer* pLoadoutPlayer = ToCSPlayer( UTIL_PlayerByIndex( i ) );
			if ( !pLoadoutPlayer || pLoadoutPlayer->IsOtherEnemy( pPlayer ) )
				continue;

			char szPanelName[16];
			Q_snprintf( szPanelName, sizeof( szPanelName ), "BuyLoadout%d", iPanel );
			CCSBuyMenuLoadoutPanel* pPanel = dynamic_cast<CCSBuyMenuLoadoutPanel*>(FindChildByName( szPanelName ));
			if ( pPanel )
			{
				iPanel++;
				pPanel->SetVisible( true );
				pPanel->SetPlayer( pLoadoutPlayer );
			}
			else
			{
				// ran out of buttons, stop
				break;
			}
		}

		engine->ClientCmd_Unrestricted( "gameui_preventescapetoshow\n" );

		if (bTouchEnable)
 		{
 			engine->ClientCmd_Unrestricted("touch_enable 0\n");
 		}
	}
	else
	{
		engine->ClientCmd_Unrestricted( "gameui_allowescapetoshow\n" );

		CCSHudWeaponSelection* pHudWS = GET_HUDELEMENT( CCSHudWeaponSelection );
		if ( pHudWS )
		{
			pHudWS->ShowAndUpdateSelection( WEPSELECT_SWITCH );
		}

		SetVisible( false );
		SetMouseInputEnabled( false );

		if (bTouchEnable)
 		{
 			engine->ClientCmd_Unrestricted("touch_enable 1\n");
 		}
	}

	m_pViewPort->ShowBackGround( bShow );
	HideCategory();
}

void CCSBuyMenu::Update()
{
	C_CSPlayer* pPlayer = C_CSPlayer::GetLocalCSPlayer();
	if ( !pPlayer )
		return;

	if ( !pPlayer->IsInBuyZone() )
	{
		// player has left the buy zone
		ShowPanel( false );
		return;
	}

	if ( m_iAccount != pPlayer->GetAccount() )
	{
		m_iAccount = pPlayer->GetAccount();
		wchar_t wszUnicode[8];
		V_snwprintf( wszUnicode, ARRAYSIZE( wszUnicode ), L"$%d", m_iAccount );
		m_pMoneyLabel->SetText( wszUnicode );
	}

	int iBuyTimeLeft = (int) (CSGameRules()->GetBuyTimeLength() - CSGameRules()->GetRoundElapsedTime());
	if ( CSGameRules()->IsPlayingGunGameDeathmatch() )
		iBuyTimeLeft = (int) (pPlayer->m_fImmuneToDamageTime - gpGlobals->curtime);
	if ( iBuyTimeLeft < 0 )
		iBuyTimeLeft = 0;

	if ( m_iBuyTimeLeft != iBuyTimeLeft )
	{
		m_iBuyTimeLeft = iBuyTimeLeft;
		wchar_t wszTimer[32];
		int iHours = m_iBuyTimeLeft / 3600;
		int iMinutes = (m_iBuyTimeLeft - 3600 * iHours) / 60;
		int iSeconds = m_iBuyTimeLeft % 60;
		V_snwprintf( wszTimer, sizeof( wszTimer ), L"%.2d : %.2d : %.2d", iHours, iMinutes, iSeconds );

		wchar_t wszString[256];
		if ( CSGameRules()->IsPlayingGunGameDeathmatch() )
			g_pVGuiLocalize->ConstructString( wszString, sizeof( wszString ), g_pVGuiLocalize->Find( "BuyMenu_ImmunityTimerText" ), 1, wszTimer );
		else
			g_pVGuiLocalize->ConstructString( wszString, sizeof( wszString ), g_pVGuiLocalize->Find( "BuyMenu_TimerText" ), 1, wszTimer );

		m_pBuyTimeLeftLabel->SetText( wszString );
	}
}

extern ConVar mat_blur_strength;
extern ConVar mat_blur_desaturate;
void CCSBuyMenu::PaintBackground()
{
	if ( engine->GetDXSupportLevel() < 90 )
		BaseClass::PaintBackground();
	else
	{
		// do the blur here instead of clientmode because it needs to render over VGUI elements
		int x, y, w, h;
		GetBounds( x, y, w, h );
		DoBlurFade( mat_blur_strength.GetFloat(), mat_blur_desaturate.GetFloat(), x, y, w, h );
	}
}

void CCSBuyMenu::OnClose()
{
	engine->ClientCmd_Unrestricted( "gameui_allowescapetoshow\n" );

	CCSHudWeaponSelection* pHudWS = GET_HUDELEMENT( CCSHudWeaponSelection );
	if ( pHudWS )
	{
		pHudWS->ShowAndUpdateSelection( WEPSELECT_SWITCH );
	}

	bool bTouchEnable = (cvar->FindVar("touch_disable_on_buymenu")->GetInt() != 0);
 
 	if (bTouchEnable)
 	{
 		engine->ClientCmd_Unrestricted("touch_enable 1\n");
	}

	BaseClass::OnClose();
}

void CCSBuyMenu::OnCommand( const char* command )
{
	C_CSPlayer* pPlayer = C_CSPlayer::GetLocalCSPlayer();
	if ( !pPlayer )
		return;

	if ( !Q_stricmp( command, "vguicancel" ) )
	{
		Close();

		gViewPortInterface->ShowBackGround( false );
	}
	else
	{
		KeyValues* kvSubMenu = m_kvBuyMenuConfig->FindKey( command );
		if ( kvSubMenu )
		{
			ShowCategory( kvSubMenu );
		}
		else
		{
			if ( !V_strncmp( command, "buy ", 4 ) )
			{
				if ( closeonbuy.GetBool() )
				{
					ShowPanel( false );
				}

				if ( m_bDropBuy )
				{
					char szCommand[128];
					Q_snprintf( szCommand, sizeof( szCommand ), "%s drop", command );
					engine->ClientCmd( szCommand );
				}
				else
				{
					engine->ClientCmd( command );
					CSWeaponID nWeaponID = AliasToWeaponID( command + 4 );
					if ( nWeaponID < WEAPON_LAST ) // it is a firearm
						HideCategory();
				}
			}
			else
			{
				engine->ClientCmd( command );
			}
		}
	}
}

Panel* CCSBuyMenu::CreateControlByName( const char* controlName )
{
	if ( !Q_stricmp( "CCSBuyMenuCategoryButton", controlName ) )
	{
		return new CCSBuyMenuCategoryButton( this, controlName );
	}
	else if ( !Q_stricmp( "CCSBuyMenuItemButton", controlName ) )
	{
		return new CCSBuyMenuItemButton( this, controlName );
	}
	else if ( !Q_stricmp( "CCSBuyMenuLoadoutPanel", controlName ) )
	{
		return new CCSBuyMenuLoadoutPanel( this, controlName );
	}
	else
	{
		return BaseClass::CreateControlByName( controlName );
	}
}

void CCSBuyMenu::OnKeyCodeTyped( KeyCode code )
{
	// ESC cancels
	if ( code == KEY_ESCAPE )
	{
		if ( m_bShowingCategory )
			HideCategory();
		else
			ShowPanel( false );
	}
	else if ( m_iBuyMenuKey != BUTTON_CODE_INVALID && m_iBuyMenuKey == code )
	{
		HideCategory();
	}
	else if ( m_iReBuyKey != BUTTON_CODE_INVALID && m_iReBuyKey == code )
	{
		OnCommand( "rebuy" );
	}
	else if ( m_iAutoBuyKey != BUTTON_CODE_INVALID && m_iAutoBuyKey == code )
	{
		OnCommand( "autobuy" );
	}
	else
	{
		BaseClass::OnKeyCodeTyped( code );
	}
}

void CCSBuyMenu::OnKeyCodePressed( KeyCode code )
{
	if ( code == KEY_LCONTROL )
	{
		m_bDropBuy = true;
		ShowSpecialMessage( "#CStrike_BuyMenu_BuyForTeammate_Hint", GlobalMessage );

		int i = 1;
		while ( true )
		{
			char szPanelName[16];
			Q_snprintf( szPanelName, sizeof( szPanelName ), "BuyItem%d", i );
			CCSBuyMenuItemButton* pButton = dynamic_cast<CCSBuyMenuItemButton*>(FindChildByName( szPanelName ));
			if ( pButton )
			{
				pButton->SetDropBuy( m_bDropBuy );
			}
			else
			{
				// ran out of buttons, stop
				break;
			}
			i++;
		}
	}
	else
	{
		BaseClass::OnKeyCodePressed( code );
	}
}

void CCSBuyMenu::OnKeyCodeReleased( KeyCode code )
{
	if ( code == KEY_LCONTROL )
	{
		m_bDropBuy = false;
		HideSpecialMessage( GlobalMessage );

		int i = 1;
		while ( true )
		{
			char szPanelName[16];
			Q_snprintf( szPanelName, sizeof( szPanelName ), "BuyItem%d", i );
			CCSBuyMenuItemButton* pButton = dynamic_cast<CCSBuyMenuItemButton*>(FindChildByName( szPanelName ));
			if ( pButton )
			{
				pButton->SetDropBuy( m_bDropBuy );
			}
			else
			{
				// ran out of buttons, stop
				break;
			}
			i++;
		}
	}
	else
	{
		BaseClass::OnKeyCodeReleased( code );
	}
}

void CCSBuyMenu::ShowCategory( KeyValues* kvCategory )
{
	HideCategory();

	C_CSPlayer* pPlayer = C_CSPlayer::GetLocalCSPlayer();
	if ( !pPlayer )
		return;

	if ( !kvCategory )
	{
		DevWarning( "Got NULL buy menu category!\n" );
		return;
	}

	int i = 1;
	for ( KeyValues* pkvItem = kvCategory->GetFirstSubKey(); pkvItem; pkvItem = pkvItem->GetNextKey() )
	{
		const char* pszName = pkvItem->GetString( "name", NULL );
		const char* pszDescription = pkvItem->GetString( "description", NULL );
		const char* pszWeapon = pkvItem->GetName();
		const char* pszIcon = pkvItem->GetString( "icon" );
		int iPrice = pkvItem->GetInt( "price" );
		int iTeamNumber = pkvItem->GetInt( "team" );
		bool bBombHostageOnly = pkvItem->GetBool( "de_cs_only" );
		if ( iTeamNumber != TEAM_UNASSIGNED && pPlayer->GetTeamNumber() != iTeamNumber )
			continue;

		CSWeaponID nWeaponID = AliasToWeaponID( pszWeapon );
		if ( nWeaponID == WEAPON_NONE )
			continue;

		char szPanelName[16];
		Q_snprintf( szPanelName, sizeof( szPanelName ), "BuyItem%d", i );
		CCSBuyMenuItemButton* pButton = dynamic_cast<CCSBuyMenuItemButton*>(FindChildByName( szPanelName ));
		if ( pButton )
		{
			char szHotkey[4];
			Q_snprintf( szHotkey, sizeof( szHotkey ), "%d", i );

			if ( nWeaponID == ITEM_DEFUSER && CSGameRules()->IsHostageRescueMap() )
			{
				pszName = "#CStrike_WPNHUD_Cutters";
			}

			pButton->SetName( pszName );
			pButton->SetDescription( pszDescription );
			pButton->SetHotkey( szHotkey[0] );
			pButton->SetPrice( iPrice );
			pButton->SetIcon( pszIcon );
			pButton->SetWeaponID( nWeaponID );
			if ( bBombHostageOnly && !CSGameRules()->IsBombDefuseMap() && !CSGameRules()->IsHostageRescueMap() )
				pButton->SetDisabled( true );

			char szCommand[128];
			Q_snprintf( szCommand, sizeof( szCommand ), "buy %s", pszWeapon );
			pButton->SetCommand( szCommand );

			pButton->SetVisible( true );
		}
		else
		{
			// ran out of buttons, stop
			break;
		}

		i++;
	}

	// remove hotkeys from categories
	i = 1;
	while ( true )
	{
		char szPanelName[16];
		Q_snprintf( szPanelName, sizeof( szPanelName ), "BuyCategory%d", i );
		Button* pButton = dynamic_cast<Button*>(FindChildByName( szPanelName ));
		if ( pButton )
		{
			pButton->SetHotkey( '\0' );
		}
		else
		{
			// ran out of buttons, stop
			break;
		}
		i++;
	}

	m_pBuyItemsBackground->SetVisible( true );
	m_pSpecialMessageLabel->SetVisible( true );
	m_bShowingCategory = true;
}

void CCSBuyMenu::HideCategory()
{
	// hide the items first
	int i = 1;
	while ( true )
	{
		char szPanelName[16];
		Q_snprintf( szPanelName, sizeof( szPanelName ), "BuyItem%d", i );
		Button* pButton = dynamic_cast<Button*>(FindChildByName( szPanelName ));
		if ( pButton )
		{
			pButton->SetVisible( false );
			pButton->SetHotkey( '\0' );
		}
		else
		{
			// ran out of buttons, stop
			break;
		}
		i++;
	}

	// then add hotkeys back to categories
	i = 1;
	while ( true )
	{
		char szPanelName[16];
		Q_snprintf( szPanelName, sizeof( szPanelName ), "BuyCategory%d", i );
		Button* pButton = dynamic_cast<Button*>(FindChildByName( szPanelName ));
		if ( pButton )
		{
			char szHotkey[4];
			Q_snprintf( szHotkey, sizeof( szHotkey ), "%d", i );

			pButton->SetHotkey( szHotkey[0] );
		}
		else
		{
			// ran out of buttons, stop
			break;
		}
		i++;
	}

	m_pBuyItemsBackground->SetVisible( false );
	m_pSpecialMessageLabel->SetVisible( false );
	m_bShowingCategory = false;
	ResetWeapon();
	m_pItemNameLabel->SetVisible( false );
	m_pItemDescriptionLabel->SetVisible( false );
	HideSpecialMessage( GlobalMessage );
}

void CCSBuyMenu::SetPlayerImageWeapon( const char* pszWeaponModel, const char* pszWeaponSequence )
{
	m_pPlayerModel->SetWeaponModel( pszWeaponModel );
	m_pPlayerModel->SetSequence( pszWeaponSequence );
}

void CCSBuyMenu::SetItemNameAndDescription( const char* pszName, const char* pszDescription )
{
	if ( pszName )
	{
		m_pItemNameLabel->SetVisible( true );
		m_pItemNameLabel->SetText( pszName );
	}
	else
	{
		m_pItemNameLabel->SetVisible( false );
	}

	if ( pszDescription )
	{
		m_pItemDescriptionLabel->SetVisible( true );
		m_pItemDescriptionLabel->SetText( pszDescription );
	}
	else
	{
		m_pItemDescriptionLabel->SetVisible( false );
	}
}

void CCSBuyMenu::ResetWeapon()
{
	C_CSPlayer* pPlayer = C_CSPlayer::GetLocalCSPlayer();
	if ( !pPlayer )
		return;

	const char* pszPlayerSequence = "t_buymenu_nowep";
	const char* pszPlayerWeaponModel = NULL;

	C_WeaponCSBase* pWeapon = dynamic_cast<C_WeaponCSBase*>(pPlayer->Weapon_GetSlot( WEAPON_SLOT_RIFLE ));
	if ( !pWeapon )
	{
		pWeapon = dynamic_cast<C_WeaponCSBase*>(pPlayer->Weapon_GetSlot( WEAPON_SLOT_PISTOL ));
		if ( !pWeapon )
		{
			pWeapon = dynamic_cast<C_WeaponCSBase*>(pPlayer->Weapon_GetSlot( WEAPON_SLOT_KNIFE ));
			if ( !pWeapon )
			{
				pWeapon = pPlayer->GetActiveCSWeapon();
			}
		}
	}
	if ( pWeapon )
	{
		pszPlayerWeaponModel = pWeapon->GetCSWpnData().szWorldModel;
		if ( pPlayer->GetTeamNumber() == TEAM_TERRORIST )
			pszPlayerSequence = pWeapon->GetCSWpnData().m_szBuyMenuAnimT;
		else
			pszPlayerSequence = pWeapon->GetCSWpnData().m_szBuyMenuAnim;
	}

	m_pPlayerModel->SetWeaponModel( pszPlayerWeaponModel );
	m_pPlayerModel->SetSequence( pszPlayerSequence );
}

void CCSBuyMenu::ShowSpecialMessage( const char* pszText, BuyMenuSpecialMessageType_t nMessageType )
{
	if ( !m_pSpecialMessageLabel )
		return;

	if ( nMessageType >= m_nMessageType )
	{
		if ( pszText && *pszText )
		{
			m_pSpecialMessageLabel->SetText( pszText );
			m_nMessageType = nMessageType;
		}
	}
}

void CCSBuyMenu::ShowSpecialMessage( const wchar_t* pwszText, BuyMenuSpecialMessageType_t nMessageType )
{
	if ( !m_pSpecialMessageLabel )
		return;

	if ( nMessageType >= m_nMessageType )
	{
		if ( pwszText && *pwszText )
		{
			m_pSpecialMessageLabel->SetText( pwszText );
			m_nMessageType = nMessageType;
		}
	}
}

void CCSBuyMenu::HideSpecialMessage( BuyMenuSpecialMessageType_t nMessageType )
{
	if ( !m_pSpecialMessageLabel )
		return;

	if ( nMessageType >= m_nMessageType )
	{
		m_pSpecialMessageLabel->SetText( "" );
		m_nMessageType = InvalidMessage;
	}
}
