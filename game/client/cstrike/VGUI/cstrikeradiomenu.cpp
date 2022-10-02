//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "cstrikeradiomenu.h"
#include "filesystem.h"
#include "vgui_controls/ScrollBar.h"

using namespace vgui;

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

//-----------------------------------------------------------------------------
// Purpose: Basic help dialog
//-----------------------------------------------------------------------------
CCSRadioMenu::CCSRadioMenu( IViewPort* pViewPort ): Frame( NULL, GetName() )
{
	m_pViewPort = pViewPort;

	// initialize dialog
	SetTitle( "", true );

	// load the new scheme early!!
	SetScheme( "ClientScheme" );
	SetMoveable( false );
	SetSizeable( false );
	SetProportional( false );

	MakePopup();
	SetKeyBoardInputEnabled( false );

	// initialize elements
	m_pRadioList = new SectionedListPanel( this, "RadioList" );

	LoadControlSettings( "Resource/UI/RadioMenu.res" );

	LoadRadioCommands();
}

void CCSRadioMenu::ShowPanel( bool bShow )
{
	if ( bShow )
	{
		// hide the system buttons
		SetTitleBarVisible( false );

		Activate();
		MoveToCenterOfScreen();
	}
	else
	{
		Close();

		PlayRadioCommand();
	}

	m_pViewPort->ShowBackGround( bShow );
}

void CCSRadioMenu::LoadRadioCommands()
{
	KeyValues* pKV = new KeyValues( "RadioMenuConfig" );
	if ( pKV->LoadFromFile( g_pFullFileSystem, "scripts/radiomenuconfig.txt", "GAME" ) )
	{
		int iSection = 0;
		for ( KeyValues* pKVSection = pKV->GetFirstSubKey(); pKVSection; pKVSection = pKVSection->GetNextKey() )
		{
			m_pRadioList->AddSection( iSection, "" );
			m_pRadioList->AddColumnToSection( iSection, "text", pKVSection->GetName(), 0, m_pRadioList->GetWide() );

			for ( KeyValues* pKVCommand = pKVSection->GetFirstValue(); pKVCommand; pKVCommand = pKVCommand->GetNextValue() )
			{
				KeyValues* pKVData = new KeyValues( "data" );
				if ( pKVData )
				{
					pKVData->SetString( "text", pKVCommand->GetString() );
					pKVData->SetString( "command", pKVCommand->GetName() );
					m_pRadioList->AddItem( iSection, pKVData );
				}
			}

			iSection++;
		}
	}

	pKV->deleteThis();
}

void CCSRadioMenu::PlayRadioCommand()
{
	KeyValues* pKVData = m_pRadioList->GetItemData( m_pRadioList->GetSelectedItem() );
	if ( pKVData )
	{
		char szCommand[128];
		V_snprintf( szCommand, sizeof( szCommand ), "playerradio %s %s", pKVData->GetString( "command" ), pKVData->GetString( "text" ) );
		engine->ClientCmd( szCommand );

		m_pRadioList->ClearSelection();
		m_pRadioList->GetScrollBar()->SetValue( 0 );
	}
}

void CCSRadioMenu::OnItemDoubleLeftClick( KeyValues* pKV )
{
	PlayRadioCommand();
	engine->ClientCmd( "-radiomenu" ); // hide the menu
}
