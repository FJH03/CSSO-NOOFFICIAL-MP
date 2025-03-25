//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "cstriketeammenu.h"
#include "view_shared.h"
#include "view.h"
#include "model_types.h"
#include "cs_gamerules.h"
#include "cs_loadout.h"
#include "c_team.h"
#include "viewpostprocess.h"
#include <vgui/ILocalize.h>
#include "c_playerresource.h"
#include "engine/IEngineSound.h"

CCSTeamMenuAgentImage::CCSTeamMenuAgentImage( Panel* parent, const char* panelName, int nTeamNumber ): Button( parent, panelName, L"" )
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
	m_nTeamNumber = nTeamNumber;
}

CCSTeamMenuAgentImage::~CCSTeamMenuAgentImage()
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

void CCSTeamMenuAgentImage::ApplySettings( KeyValues* inResourceData )
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

void CCSTeamMenuAgentImage::OnCursorEntered()
{
	switch ( m_nTeamNumber )
	{
		case TEAM_TERRORIST:
			SetSequence( "t_teamselect_confirm01", 0.2f );
			break;
		case TEAM_CT:
			SetSequence( "ct_teamselect_confirm01", 0.2f );
			break;
	}
}

void CCSTeamMenuAgentImage::OnCursorExited()
{
	C_CSPlayer* pPlayer = C_CSPlayer::GetLocalCSPlayer();
	if ( !pPlayer )
		return;

	CSWeaponID nWeaponID = CSLoadout()->GetLoadoutWeaponID( pPlayer, m_nTeamNumber, (m_nTeamNumber == TEAM_CT) ? WEAPON_HKP2000 : WEAPON_GLOCK );
	WEAPON_FILE_INFO_HANDLE	hWpnInfo = LookupWeaponInfoSlot( WeaponIdAsString( nWeaponID ) );
	if ( hWpnInfo != GetInvalidWeaponInfoHandle() )
	{
		CCSWeaponInfo* pWeaponInfo = dynamic_cast<CCSWeaponInfo*>(GetFileWeaponInfoFromHandle( hWpnInfo ));
		if ( pWeaponInfo )
		{
			SetSequence( (m_nTeamNumber == TEAM_CT) ? pWeaponInfo->m_szClassMenuAnim : pWeaponInfo->m_szClassMenuAnimT, 2.0f );
		}
	}
}

void CCSTeamMenuAgentImage::ParseLightInfo( KeyValues* inResourceData )
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

void CCSTeamMenuAgentImage::SetPlayerModel( const char* pszModel )
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

void CCSTeamMenuAgentImage::SetWeaponModel( const char* pszModel )
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

void CCSTeamMenuAgentImage::SetGlovesModel( const char* pszModel )
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

void CCSTeamMenuAgentImage::SetSequence( const char* pszSequence, float flSequenceFade )
{
	if ( m_hPlayerModel.Get() )
	{
		int sequence = m_hPlayerModel->LookupSequence( pszSequence );
		if ( sequence != ACT_INVALID )
		{
			m_hPlayerModel->SetSequenceTransitionFadeOverride( flSequenceFade );
			m_hPlayerModel->ResetSequence( sequence );
			m_hPlayerModel->SetCycle( 0 );
		}
	}
}

bool CCSTeamMenuAgentImage::DoesModelSupportGloves( const char* pszGlovesViewModelName, const char* pszDefaultViewModelName )
{
	if ( m_hPlayerModel.Get() )
		return m_hPlayerModel->DoesModelSupportGloves( pszGlovesViewModelName, pszDefaultViewModelName );

	return false;
}

void CCSTeamMenuAgentImage::Paint()
{
	C_BasePlayer* pLocalPlayer = C_BasePlayer::GetLocalPlayer();

	if ( !pLocalPlayer )
		return;

	MDLCACHE_CRITICAL_SECTION();

	if ( !m_hPlayerModel.Get() )
		return;

	// do we have a valid sequence?
	if ( m_hPlayerModel->GetSequence() != -1 )
	{
		m_hPlayerModel->FrameAdvance( gpGlobals->frametime );
	}

	// Now draw it.
	CViewSetup view;
	view.x = viewport_xpos; // we actually want to offset by the 
	view.y = viewport_ypos; // viewport origin here because Push3DView expects global coords below
	view.width = viewport_wide;		// added separate viewport width and height parameters so that
	view.height = viewport_tall;	// FOV gets scaled correctly down below

	view.m_bOrtho = false;

	view.fov = m_nFOV;

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


CCSTeamMenu::CCSTeamMenu( IViewPort* pViewPort ): Frame( NULL, PANEL_TEAM )
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

	// initialize elements
	m_pAgentModelT = new CCSTeamMenuAgentImage( this, "AgentModelT", TEAM_TERRORIST );
	m_pAgentModelT->SetPaintBackgroundEnabled( false );
	m_pAgentModelCT = new CCSTeamMenuAgentImage( this, "AgentModelCT", TEAM_CT );
	m_pAgentModelCT->SetPaintBackgroundEnabled( false );
	m_pCancelButton = new Button( this, "CancelButton", "#Cstrike_Cancel" );
	m_pSpectateButton = new Button( this, "SpectateButton", "#Cstrike_Menu_Spectate" );
	m_pAutoAssignButton = new Button( this, "AutoAssignButton", "#Cstrike_Team_AutoAssign" );
	m_pTPlayerCount = new Label( this, "TPlayerCount", L"" );
	m_pCTPlayerCount = new Label( this, "CTPlayerCount", L"" );

	LoadControlSettings( "Resource/UI/TeamMenu.res" );
}

void CCSTeamMenu::ShowPanel( bool bShow )
{
	C_CSPlayer* pPlayer = C_CSPlayer::GetLocalCSPlayer();
	if ( !pPlayer )
		return;

	if ( bShow )
	{
		if ( !IsVisible() )
		{
			CLocalPlayerFilter filter;
			PlayMusicSelection( filter, CSMUSIC_SELECTION );
		}

		// hide the system buttons
		SetTitleBarVisible( false );

		Activate();
		SetMouseInputEnabled( true );

		ResetAgentModels();
		m_pAgentModelT->SetHotkey( '1' );
		m_pAgentModelCT->SetHotkey( '2' );
		m_pAutoAssignButton->SetHotkey( '5' );
		m_pSpectateButton->SetHotkey( '6' );

		UpdatePlayerCount();

		bool bAllowSpectate = false;
		const ConVar* allowSpectators = cvar->FindVar( "mp_allowspectators" );
		if ( allowSpectators &&
			 allowSpectators->GetBool() &&
			 (  GetGlobalTeam( TEAM_SPECTATOR )->GetNumPlayers() < CSGameRules()->GetMaxSpectatorSlots() ) )
		{
			bAllowSpectate = true;
		}

		m_pSpectateButton->SetVisible( bAllowSpectate );
		m_pCancelButton->SetVisible( pPlayer->GetTeamNumber() != TEAM_UNASSIGNED );

		engine->ClientCmd_Unrestricted( "gameui_preventescapetoshow\n" );
	}
	else
	{
		engine->ClientCmd_Unrestricted( "gameui_allowescapetoshow\n" );

		SetVisible( false );
		SetMouseInputEnabled( false );
	}

	m_pViewPort->ShowBackGround( bShow );
}

extern ConVar mat_blur_strength;
extern ConVar mat_blur_desaturate;
void CCSTeamMenu::PaintBackground()
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

void CCSTeamMenu::OnClose()
{
	engine->ClientCmd_Unrestricted( "gameui_allowescapetoshow\n" );
	BaseClass::OnClose();
}

void CCSTeamMenu::OnCommand( const char* command )
{
	Close();
	gViewPortInterface->ShowBackGround( false );
	engine->ClientCmd( command );
}

void CCSTeamMenu::OnKeyCodeTyped( KeyCode code )
{
	C_CSPlayer* pPlayer = C_CSPlayer::GetLocalCSPlayer();
	if ( !pPlayer )
	{
		BaseClass::OnKeyCodeTyped( code );
		return;
	}

	// ESC cancels
	if ( code == KEY_ESCAPE && pPlayer->GetTeamNumber() != TEAM_UNASSIGNED )
	{
		ShowPanel( false );
	}
	else
	{
		BaseClass::OnKeyCodeTyped( code );
	}
}

void CCSTeamMenu::ResetAgentModels()
{
	C_CSPlayer* pPlayer = C_CSPlayer::GetLocalCSPlayer();
	if ( !pPlayer )
		return;

	// Ts
	{
		const char* pszPlayerModel = NULL;
		int iMapFaction = CSGameRules()->GetMapFactionsForThisPlayer( pPlayer, TEAM_TERRORIST );
		bool bUseAgent = CSLoadout()->HasAgentSet( pPlayer, TEAM_TERRORIST );
		if ( bUseAgent )
		{
			pszPlayerModel = GetCSAgentInfoT( CSLoadout()->GetAgentForPlayer( pPlayer, TEAM_TERRORIST ) )->m_szModel;
		}
		else if ( iMapFaction > -1 )
		{
			switch ( iMapFaction )
			{
				case CS_CLASS_PHOENIX_CONNNECTION:
				{
					pszPlayerModel = TPhoenixPlayerModelStrings[0];
					break;
				}
				case CS_CLASS_L337_KREW:
				{
					pszPlayerModel = TLeetPlayerModelStrings[0];
					break;
				}
				case CS_CLASS_SEPARATIST:
				{
					pszPlayerModel = TSeparatistPlayerModelStrings[0];
					break;
				}
				case CS_CLASS_BALKAN:
				{
					pszPlayerModel = TBalkanPlayerModelStrings[0];
					break;
				}
				case CS_CLASS_PROFESSIONAL:
				{
					pszPlayerModel = TProfessionalPlayerModelStrings[0];
					break;
				}
				case CS_CLASS_ANARCHIST:
				{
					pszPlayerModel = TAnarchistPlayerModelStrings[0];
					break;
				}
				case CS_CLASS_PIRATE:
				{
					pszPlayerModel = TPiratePlayerModelStrings[0];
					break;
				}
			}
		}
		m_pAgentModelT->SetPlayerModel( pszPlayerModel );

		CSWeaponID nWeaponID = CSLoadout()->GetLoadoutWeaponID( pPlayer, TEAM_TERRORIST, WEAPON_GLOCK );
		WEAPON_FILE_INFO_HANDLE	hWpnInfo = LookupWeaponInfoSlot( WeaponIdAsString( nWeaponID ) );
		if ( hWpnInfo != GetInvalidWeaponInfoHandle() )
		{
			CCSWeaponInfo* pWeaponInfo = dynamic_cast<CCSWeaponInfo*>(GetFileWeaponInfoFromHandle( hWpnInfo ));
			if ( pWeaponInfo )
			{
				m_pAgentModelT->SetWeaponModel( pWeaponInfo->szWorldModel );
				m_pAgentModelT->SetSequence( pWeaponInfo->m_szClassMenuAnimT, 0.0f );
			}
		}

		if ( CSLoadout()->HasGlovesSet( pPlayer, TEAM_TERRORIST ) )
		{
			const char* pszGlovesViewModel = GetGlovesInfo( CSLoadout()->GetGlovesForPlayer( pPlayer, TEAM_TERRORIST ) )->szViewModel;
			const char* pszGlovesWorldModel = GetGlovesInfo( CSLoadout()->GetGlovesForPlayer( pPlayer, TEAM_TERRORIST ) )->szWorldModel;
			const char* pszDefaultGlovesModel = GetPlayerViewmodelArmConfigForPlayerModel( pszPlayerModel )->szAssociatedGloveModel;
			if ( pszGlovesViewModel && pszDefaultGlovesModel && m_pAgentModelT->DoesModelSupportGloves( pszGlovesViewModel, pszDefaultGlovesModel ) )
			{
				m_pAgentModelT->SetGlovesModel( pszGlovesWorldModel );
			}
			else
			{
				m_pAgentModelT->SetGlovesModel( NULL );
			}
		}
		else
		{
			m_pAgentModelT->SetGlovesModel( NULL );
		}
	}

	// CTs
	{
		const char* pszPlayerModel = NULL;
		int iMapFaction = CSGameRules()->GetMapFactionsForThisPlayer( pPlayer, TEAM_CT );
		bool bUseAgent = CSLoadout()->HasAgentSet( pPlayer, TEAM_CT );
		if ( bUseAgent )
		{
			pszPlayerModel = GetCSAgentInfoCT( CSLoadout()->GetAgentForPlayer( pPlayer, TEAM_CT ) )->m_szModel;
		}
		else if ( iMapFaction > -1 )
		{
			switch ( iMapFaction )
			{
				case CS_CLASS_SEAL_TEAM_6:
				{
					pszPlayerModel = CTST6PlayerModelStrings[0];
					break;
				}
				case CS_CLASS_GSG_9:
				{
					pszPlayerModel = CTGSG9PlayerModelStrings[0];
					break;
				}
				case CS_CLASS_SAS:
				{
					pszPlayerModel = CTSASPlayerModelStrings[0];
					break;
				}
				case CS_CLASS_GIGN:
				{
					pszPlayerModel = CTGIGNPlayerModelStrings[0];
					break;
				}
				case CS_CLASS_FBI:
				{
					pszPlayerModel = CTFBIPlayerModelStrings[0];
					break;
				}
				case CS_CLASS_IDF:
				{
					pszPlayerModel = CTIDFPlayerModelStrings[0];
					break;
				}
				case CS_CLASS_SWAT:
				{
					pszPlayerModel = CTSWATPlayerModelStrings[0];
					break;
				}
			}
		}
		m_pAgentModelCT->SetPlayerModel( pszPlayerModel );

		CSWeaponID nWeaponID = CSLoadout()->GetLoadoutWeaponID( pPlayer, TEAM_CT, WEAPON_HKP2000 );
		WEAPON_FILE_INFO_HANDLE	hWpnInfo = LookupWeaponInfoSlot( WeaponIdAsString( nWeaponID ) );
		if ( hWpnInfo != GetInvalidWeaponInfoHandle() )
		{
			CCSWeaponInfo* pWeaponInfo = dynamic_cast<CCSWeaponInfo*>(GetFileWeaponInfoFromHandle( hWpnInfo ));
			if ( pWeaponInfo )
			{
				m_pAgentModelCT->SetWeaponModel( pWeaponInfo->szWorldModel );
				m_pAgentModelCT->SetSequence( pWeaponInfo->m_szClassMenuAnim, 0.0f );
			}
		}

		if ( CSLoadout()->HasGlovesSet( pPlayer, TEAM_CT ) )
		{
			const char* pszGlovesViewModel = GetGlovesInfo( CSLoadout()->GetGlovesForPlayer( pPlayer, TEAM_CT ) )->szViewModel;
			const char* pszGlovesWorldModel = GetGlovesInfo( CSLoadout()->GetGlovesForPlayer( pPlayer, TEAM_CT ) )->szWorldModel;
			const char* pszDefaultGlovesModel = GetPlayerViewmodelArmConfigForPlayerModel( pszPlayerModel )->szAssociatedGloveModel;
			if ( pszGlovesViewModel && pszDefaultGlovesModel && m_pAgentModelCT->DoesModelSupportGloves( pszGlovesViewModel, pszDefaultGlovesModel ) )
			{
				m_pAgentModelCT->SetGlovesModel( pszGlovesWorldModel );
			}
			else
			{
				m_pAgentModelCT->SetGlovesModel( NULL );
			}
		}
		else
		{
			m_pAgentModelCT->SetGlovesModel( NULL );
		}
	}
}

void CCSTeamMenu::UpdatePlayerCount()
{
	if ( !g_PR )
		return;

	int iTHumans = 0;
	int iTBots = 0;
	int iCTHumans = 0;
	int iCTBots = 0;
	for ( int i = 0; i < MAX_PLAYERS; i++ )
	{
		if ( g_PR->IsConnected( i ) )
		{
			int iTeamNumber = g_PR->GetTeam( i );
			if ( iTeamNumber == TEAM_TERRORIST )
			{
				if ( g_PR->IsFakePlayer( i ) )
					iTBots++;
				else
					iTHumans++;
			}
			else if ( iTeamNumber == TEAM_CT )
			{
				if ( g_PR->IsFakePlayer( i ) )
					iCTBots++;
				else
					iCTHumans++;
			}
		}
	}

	wchar_t wszHumanCount[8];
	wchar_t wszBotCount[8];
	wchar_t wszString[64];
	const wchar_t* wszLocalized = g_pVGuiLocalize->Find( "#CStrike_Team_Humans_Bots" );

	V_snwprintf( wszHumanCount, sizeof( wszHumanCount ), L"%d", iTHumans );
	V_snwprintf( wszBotCount, sizeof( wszBotCount ), L"%d", iTBots );
	g_pVGuiLocalize->ConstructString( wszString, sizeof( wszString ), wszLocalized, 2, wszHumanCount, wszBotCount );
	m_pTPlayerCount->SetText( wszString );

	V_snwprintf( wszHumanCount, sizeof( wszHumanCount ), L"%d", iCTHumans );
	V_snwprintf( wszBotCount, sizeof( wszBotCount ), L"%d", iCTBots );
	g_pVGuiLocalize->ConstructString( wszString, sizeof( wszString ), wszLocalized, 2, wszHumanCount, wszBotCount );
	m_pCTPlayerCount->SetText( wszString );
}
