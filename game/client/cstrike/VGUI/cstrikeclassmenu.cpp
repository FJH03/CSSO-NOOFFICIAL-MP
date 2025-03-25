//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "cstrikeclassmenu.h"
#include "view_shared.h"
#include "view.h"
#include "model_types.h"
#include "cs_gamerules.h"
#include "cs_loadout.h"

CCSClassMenu::CCSClassMenu( IViewPort* pViewPort ): Frame( NULL, GetName() )
{
	m_pViewPort = pViewPort;

	// initialize dialog
	SetTitle( "", true );

	// load the new scheme early!!
	SetScheme( "ClientScheme" );
	SetMoveable( false );
	SetSizeable( false );

	SetProportional( true );

	// initialize elements
	m_pCancelButton = new Button( this, "CancelButton", "#Cstrike_Cancel" );
	m_pAutoAssignButton = new Button( this, "AutoAssignButton", "#Cstrike_Team_AutoAssign" );
}

void CCSClassMenu::Update()
{
	C_CSPlayer* pPlayer = C_CSPlayer::GetLocalCSPlayer();
	if ( !pPlayer )
		return;

	if ( m_pCancelButton )
		m_pCancelButton->SetVisible( pPlayer->IsAlive() );
}

void CCSClassMenu::ShowPanel( bool bShow )
{
	if ( bShow )
	{
		// hide the system buttons
		SetTitleBarVisible( false );

		Activate();
		SetMouseInputEnabled( true );

		m_pAutoAssignButton->SetHotkey( '8' );

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

void CCSClassMenu::OnClose()
{
	engine->ClientCmd_Unrestricted( "gameui_allowescapetoshow\n" );
	BaseClass::OnClose();
}

void CCSClassMenu::OnCommand( const char* command )
{
	Close();
	gViewPortInterface->ShowBackGround( false );
	engine->ClientCmd( command );
}

void CCSClassMenu::OnKeyCodeTyped( KeyCode code )
{
	C_CSPlayer* pPlayer = C_CSPlayer::GetLocalCSPlayer();
	if ( !pPlayer )
	{
		BaseClass::OnKeyCodeTyped( code );
		return;
	}

	// ESC cancels
	if ( code == KEY_ESCAPE && pPlayer->IsAlive() )
	{
		ShowPanel( false );
	}
	else
	{
		BaseClass::OnKeyCodeTyped( code );
	}
}


CCSClassMenu_TER::CCSClassMenu_TER( IViewPort* pViewPort ): CCSClassMenu( pViewPort )
{
	for ( int i = 0; i < ARRAYSIZE( m_pAgentModels ); i++ )
	{
		char szPanelName[32];
		V_snprintf( szPanelName, sizeof( szPanelName ), "AgentModel%d", i );
		m_pAgentModels[i] = new CCSTeamMenuAgentImage( this, szPanelName, TEAM_TERRORIST );
		m_pAgentModels[i]->SetPaintBackgroundEnabled( false );
	}

	LoadControlSettings( "Resource/UI/ClassMenu_TER.res" );
}

void CCSClassMenu_TER::ShowPanel( bool bShow )
{
	BaseClass::ShowPanel( bShow );

	if ( bShow )
		ResetAgentModels();
}

void CCSClassMenu_TER::ResetAgentModels()
{
	C_CSPlayer* pPlayer = C_CSPlayer::GetLocalCSPlayer();
	if ( !pPlayer )
		return;

	for ( int i = 0; i < ARRAYSIZE( m_pAgentModels ); i++ )
	{
		char szHotkey[4];
		Q_snprintf( szHotkey, sizeof( szHotkey ), "%d", i + 1 );
		m_pAgentModels[i]->SetHotkey( szHotkey[0] );

		const char* pszPlayerModel = NULL;
		switch ( i+FIRST_T_CLASS )
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
		m_pAgentModels[i]->SetPlayerModel( pszPlayerModel );

		CSWeaponID nWeaponID = CSLoadout()->GetLoadoutWeaponID( pPlayer, TEAM_TERRORIST, WEAPON_GLOCK );
		WEAPON_FILE_INFO_HANDLE	hWpnInfo = LookupWeaponInfoSlot( WeaponIdAsString( nWeaponID ) );
		if ( hWpnInfo != GetInvalidWeaponInfoHandle() )
		{
			CCSWeaponInfo* pWeaponInfo = dynamic_cast<CCSWeaponInfo*>(GetFileWeaponInfoFromHandle( hWpnInfo ));
			if ( pWeaponInfo )
			{
				m_pAgentModels[i]->SetWeaponModel( pWeaponInfo->szWorldModel );
				m_pAgentModels[i]->SetSequence( pWeaponInfo->m_szClassMenuAnimT, 0.0f );
			}
		}

		if ( CSLoadout()->HasGlovesSet( pPlayer, TEAM_TERRORIST ) )
		{
			const char* pszGlovesViewModel = GetGlovesInfo( CSLoadout()->GetGlovesForPlayer( pPlayer, TEAM_TERRORIST ) )->szViewModel;
			const char* pszGlovesWorldModel = GetGlovesInfo( CSLoadout()->GetGlovesForPlayer( pPlayer, TEAM_TERRORIST ) )->szWorldModel;
			const char* pszDefaultGlovesModel = GetPlayerViewmodelArmConfigForPlayerModel( pszPlayerModel )->szAssociatedGloveModel;
			if ( pszGlovesViewModel && pszDefaultGlovesModel && m_pAgentModels[i]->DoesModelSupportGloves( pszGlovesViewModel, pszDefaultGlovesModel ) )
			{
				m_pAgentModels[i]->SetGlovesModel( pszGlovesWorldModel );
			}
			else
			{
				m_pAgentModels[i]->SetGlovesModel( NULL );
			}
		}
		else
		{
			m_pAgentModels[i]->SetGlovesModel( NULL );
		}
	}
}


CCSClassMenu_CT::CCSClassMenu_CT( IViewPort* pViewPort ): CCSClassMenu( pViewPort )
{
	for ( int i = 0; i < ARRAYSIZE( m_pAgentModels ); i++ )
	{
		char szPanelName[32];
		V_snprintf( szPanelName, sizeof( szPanelName ), "AgentModel%d", i );
		m_pAgentModels[i] = new CCSTeamMenuAgentImage( this, szPanelName, TEAM_CT );
		m_pAgentModels[i]->SetPaintBackgroundEnabled( false );
	}

	LoadControlSettings( "Resource/UI/ClassMenu_CT.res" );
}

void CCSClassMenu_CT::ShowPanel( bool bShow )
{
	BaseClass::ShowPanel( bShow );

	if ( bShow )
		ResetAgentModels();
}

void CCSClassMenu_CT::ResetAgentModels()
{
	C_CSPlayer* pPlayer = C_CSPlayer::GetLocalCSPlayer();
	if ( !pPlayer )
		return;

	for ( int i = 0; i < ARRAYSIZE( m_pAgentModels ); i++ )
	{
		char szHotkey[4];
		Q_snprintf( szHotkey, sizeof( szHotkey ), "%d", i + 1 );
		m_pAgentModels[i]->SetHotkey( szHotkey[0] );

		const char* pszPlayerModel = NULL;
		switch ( i+FIRST_CT_CLASS )
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
		m_pAgentModels[i]->SetPlayerModel( pszPlayerModel );

		CSWeaponID nWeaponID = CSLoadout()->GetLoadoutWeaponID( pPlayer, TEAM_CT, WEAPON_HKP2000 );
		WEAPON_FILE_INFO_HANDLE	hWpnInfo = LookupWeaponInfoSlot( WeaponIdAsString( nWeaponID ) );
		if ( hWpnInfo != GetInvalidWeaponInfoHandle() )
		{
			CCSWeaponInfo* pWeaponInfo = dynamic_cast<CCSWeaponInfo*>(GetFileWeaponInfoFromHandle( hWpnInfo ));
			if ( pWeaponInfo )
			{
				m_pAgentModels[i]->SetWeaponModel( pWeaponInfo->szWorldModel );
				m_pAgentModels[i]->SetSequence( pWeaponInfo->m_szClassMenuAnim, 0.0f );
			}
		}

		if ( CSLoadout()->HasGlovesSet( pPlayer, TEAM_CT ) )
		{
			const char* pszGlovesViewModel = GetGlovesInfo( CSLoadout()->GetGlovesForPlayer( pPlayer, TEAM_CT ) )->szViewModel;
			const char* pszGlovesWorldModel = GetGlovesInfo( CSLoadout()->GetGlovesForPlayer( pPlayer, TEAM_CT ) )->szWorldModel;
			const char* pszDefaultGlovesModel = GetPlayerViewmodelArmConfigForPlayerModel( pszPlayerModel )->szAssociatedGloveModel;
			if ( pszGlovesViewModel && pszDefaultGlovesModel && m_pAgentModels[i]->DoesModelSupportGloves( pszGlovesViewModel, pszDefaultGlovesModel ) )
			{
				m_pAgentModels[i]->SetGlovesModel( pszGlovesWorldModel );
			}
			else
			{
				m_pAgentModels[i]->SetGlovesModel( NULL );
			}
		}
		else
		{
			m_pAgentModels[i]->SetGlovesModel( NULL );
		}
	}
}
