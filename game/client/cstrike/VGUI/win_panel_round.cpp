//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Create and display a win panel at the end of a round displaying interesting stats and info about the round.
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "win_panel_round.h"
#include "vgui_controls/AnimationController.h"
#include "iclientmode.h"
#include "c_cs_playerresource.h"
#include <vgui_controls/Label.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include "fmtstr.h"
#include "cs_gamestats_shared.h"
#include "c_team.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

DECLARE_HUDELEMENT_DEPTH( WinPanel_Round, 1 );	// 1 is foreground
extern const wchar_t *LocalizeFindSafe( const char *pTokenName );


//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
WinPanel_Round::WinPanel_Round( const char *pElementName ): CHudElement( pElementName ), EditablePanel( NULL, pElementName )
{
	SetParent( g_pClientMode->GetViewport() );

	// listen for events
	ListenForGameEvent( "round_end" );
	ListenForGameEvent( "round_start" );
	ListenForGameEvent( "cs_win_panel_round" );
	ListenForGameEvent( "cs_win_panel_match" );
	ListenForGameEvent( "round_mvp" );

	m_pMVPAvatarBkg = new ImagePanel( this, "MVP_AvatarBkg" );
	m_pMVPAvatar = new CAvatarImagePanel( this, "MVP_Avatar" );
	m_pMVPAvatar->SetDefaultAvatar( scheme()->GetImage( CSTRIKE_DEFAULT_AVATAR, true ) );
	m_pMVPAvatar->SetShouldDrawFriendIcon( false );
	m_pWinLabel = new Label( this, "WinLabel", L" " );
	m_pFunFactLabel = new Label( this, "FunFactLabel", L" " );
	m_pMVPText = new Label( this, "MVP_Text", L"MVP_TEXT" );
	m_pMainBackground = new ImagePanel( this, "MainBackground" );
	m_pTeamIcon = new ImagePanel( this, "TeamLogo" );

	LoadControlSettings( "Resource/UI/Win_Round.res" );
}

void WinPanel_Round::OnScreenSizeChanged( int iOldWide, int iOldTall )
{
	// reload the .res file so items are rescaled
	LoadControlSettings( "Resource/UI/Win_Round.res" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void WinPanel_Round::Reset()
{
	Hide();
}

// [Forrest] Allow win panel to be turned off on client
ConVar cl_nowinpanel(
	"cl_nowinpanel",
	"0",
	FCVAR_ARCHIVE,
	"Turn on/off win panel on client"
	);

void WinPanel_Round::FireGameEvent( IGameEvent* event )
{
	const char *pEventName = event->GetName();

	if ( Q_strcmp( "round_end", pEventName ) == 0 )
	{
	}		
	else if ( Q_strcmp( "round_start", pEventName ) == 0 )
	{
		// Reset MVP info when round starts
		SetMVP( NULL, CSMVP_UNDEFINED );

		Hide();
	}
	else if( Q_strcmp( "cs_win_panel_match", pEventName ) == 0 )
	{
		Hide();
	}
	else if( Q_strcmp( "round_mvp", pEventName ) == 0 )
	{		
		C_BasePlayer *basePlayer = UTIL_PlayerByUserId( event->GetInt( "userid" ) );
		CSMvpReason_t mvpReason = (CSMvpReason_t)event->GetInt( "reason" );

		if( basePlayer )
		{
			SetMVP( ToCSPlayer( basePlayer ), mvpReason );
		}
	}
	else if ( Q_strcmp( "cs_win_panel_round", pEventName ) == 0 )
	{
		/*
		"final_event"		"byte"		// 0 - no event, 1 - bomb exploded, 2 - flag capped, 3 - timer expired

		"funfact_type"		"byte"		//WINPANEL_FUNFACT in cs_shareddef.h
		"funfact_player"	"byte"
		"funfact_data1"		"long"
		"funfact_data2"		"long"
		"funfact_data3"		"long"
		*/

		if ( !g_PR )
			return;

		// [Forrest] Check if win panel is disabled.
		static ConVarRef sv_nowinpanel( "sv_nowinpanel" );
		if ( sv_nowinpanel.GetBool() || cl_nowinpanel.GetBool() )
			return;

		// Final Fun Fact
		m_pFunFactLabel->SetText( L" " );
		int iFunFactPlayer = event->GetInt("funfact_player");
		const char* funfactToken = event->GetString("funfact_token", "");

		if (strlen(funfactToken) != 0)
		{
			wchar_t funFactText[256];
			wchar_t playerText[MAX_DECORATED_PLAYER_NAME_LENGTH];
			wchar_t dataText1[8], dataText2[8], dataText3[8];
			int param1 = event->GetInt("funfact_data1");
			int param2 = event->GetInt("funfact_data2");
			int param3 = event->GetInt("funfact_data3");
			if ( iFunFactPlayer >= 1 && iFunFactPlayer <= MAX_PLAYERS )
			{
				playerText[0] = L'\0';

				C_CS_PlayerResource *cs_PR = dynamic_cast<C_CS_PlayerResource *>(g_PR);
				if ( !cs_PR )
					return;

				cs_PR->GetDecoratedPlayerName( iFunFactPlayer, playerText, sizeof( playerText ), k_EDecoratedPlayerNameFlag_Simple );

				if ( playerText[0] == L'\0' )
				{
					V_snwprintf( playerText, ARRAYSIZE( playerText ), FMT_WS, g_pVGuiLocalize->Find( "#winpanel_former_player" ) );
				}
			}
			else
			{
				_snwprintf( playerText, ARRAYSIZE( playerText ), L"" );
			}
			_snwprintf( dataText1, ARRAYSIZE( dataText1 ), L"%i", param1 );
			_snwprintf( dataText2, ARRAYSIZE( dataText2 ), L"%i", param2 );
			_snwprintf( dataText3, ARRAYSIZE( dataText3 ), L"%i", param3 );
			g_pVGuiLocalize->ConstructString( funFactText, sizeof(funFactText), (wchar_t *)LocalizeFindSafe(funfactToken), 4,
				playerText, dataText1, dataText2, dataText3 );
			m_pFunFactLabel->SetText( funFactText );
			m_pFunFactLabel->SetVisible( true );
		}
		else
		{
			m_pFunFactLabel->SetVisible( false );
		}

		int iEndEvent = event->GetInt( "final_event" );

		wchar_t wszName[512];
		int iTeamID = TEAM_UNASSIGNED;
		switch(iEndEvent)
		{
		case Target_Bombed:
		case VIP_Assassinated:
		case Terrorists_Escaped:
		case Terrorists_Win:
		case Hostages_Not_Rescued:
		case VIP_Not_Escaped:
			iTeamID = TEAM_TERRORIST;
			g_pVGuiLocalize->ConstructString( wszName, sizeof( wszName ), g_pVGuiLocalize->Find( "#winpanel_t_win" ), nullptr );
			m_pWinLabel->SetFgColor( m_clrT );
			m_pMainBackground->SetImage( "hud/winpanel_t_background" );
			m_pTeamIcon->SetImage( "hud/t_patch" );
			m_pTeamIcon->SetVisible( true );
			break;

		case VIP_Escaped:
		case CTs_PreventEscape:
		case Escaping_Terrorists_Neutralized:
		case Bomb_Defused:
		case CTs_Win:
		case All_Hostages_Rescued:
		case Target_Saved:
		case Terrorists_Not_Escaped:
			iTeamID = TEAM_CT;
			g_pVGuiLocalize->ConstructString( wszName, sizeof( wszName ), g_pVGuiLocalize->Find( "#winpanel_ct_win" ), nullptr );
			m_pWinLabel->SetFgColor( m_clrCT );
			m_pMainBackground->SetImage( "hud/winpanel_ct_background" );
			m_pTeamIcon->SetImage( "hud/ct_patch" );
			m_pTeamIcon->SetVisible( true );
			break;

		case Round_Draw:
			g_pVGuiLocalize->ConstructString( wszName, sizeof( wszName ), g_pVGuiLocalize->Find( "#winpanel_draw" ), nullptr );
			m_pMainBackground->SetImage( "hud/winpanel_draw_background" );
			m_pWinLabel->SetFgColor( COLOR_WHITE );
			m_pTeamIcon->SetVisible( false );
			break;
		}

		C_Team *pTeam = GetGlobalTeam( iTeamID );
		if ( pTeam && !StringIsEmpty( pTeam->Get_ClanName() ) )
		{
			wchar_t wszTemp[MAX_TEAM_NAME_LENGTH];
			g_pVGuiLocalize->ConvertANSIToUnicode( pTeam->Get_ClanName(), wszTemp, sizeof( wszTemp ) );
			g_pVGuiLocalize->ConstructString( wszName, sizeof( wszName ), g_pVGuiLocalize->Find( "#winpanel_team_win_team" ), 1, wszTemp );
		}

		m_pWinLabel->SetText( wszName );

		Show();
	}
}

void WinPanel_Round::SetMVP( C_CSPlayer* pPlayer, CSMvpReason_t reason )
{
	if ( !g_PR )
		return;
	
	if ( m_pMVPAvatar )
	{
		m_pMVPAvatar->ClearAvatar();
	}

	// [Forrest] Allow MVP to be turned off for a server
	bool isThereAnMVP = ( pPlayer != NULL );
	if ( isThereAnMVP )
	{
		//First set the text to the name of the player
		wchar_t wszPlayerName[MAX_DECORATED_PLAYER_NAME_LENGTH];
		((C_CS_PlayerResource*) g_PR)->GetDecoratedPlayerName( pPlayer->entindex(), wszPlayerName, sizeof( wszPlayerName ), EDecoratedPlayerNameFlag_t( k_EDecoratedPlayerNameFlag_Simple | k_EDecoratedPlayerNameFlag_DontUseNameOfControllingPlayer ) );
		
		const char* mvpReasonToken = NULL;
		switch ( reason )
		{
		case CSMVP_ELIMINATION:
			mvpReasonToken = "winpanel_mvp_award_kills";
			break;
		case CSMVP_BOMBPLANT:
			mvpReasonToken = "winpanel_mvp_award_bombplant";
			break;
		case CSMVP_BOMBDEFUSE:
			mvpReasonToken = "winpanel_mvp_award_bombdefuse";
			break;
		case CSMVP_HOSTAGERESCUE:
			mvpReasonToken = "winpanel_mvp_award_rescue";
			break;
		case CSMVP_GUNGAMEWINNER:
			mvpReasonToken = "winpanel_mvp_award_gungame";
			break;
		default:
			mvpReasonToken = "winpanel_mvp_award";
			break;
		}

		wchar_t wszBuf[256];
		wchar_t *pReason = g_pVGuiLocalize->Find( mvpReasonToken );
		if ( !pReason )
		{
			pReason = L"%s1";
		}

		g_pVGuiLocalize->ConstructString( wszBuf, sizeof( wszBuf ), pReason, 1, wszPlayerName );
		m_pMVPText->SetText( wszBuf );
		m_pMVPText->InternalPerformLayout(); // layout it right now so that is resizes this frame
		m_pMVPText->SetVisible( true );

		player_info_t pi;
		if ( engine->GetPlayerInfo(pPlayer->entindex(), &pi) )
		{
			if ( m_pMVPAvatar )
			{
				int x1, y1, x2, y2;
				m_pMVPText->ComputeAlignment( x1, y1, x2, y2 );
				m_pMVPAvatarBkg->SetPos( x1 - m_pMVPAvatarBkg->GetWide() - mvp_avatar_margin, m_pMVPAvatarBkg->GetYPos() );
				m_pMVPAvatar->SetDefaultAvatar( GetDefaultAvatarImage( pPlayer ) );
				m_pMVPAvatar->SetPlayer( pPlayer, k_EAvatarSize64x64 );
			}
		}
	}
	else
	{
		m_pMVPText->SetVisible( false );
	}

	// [Forrest] Allow MVP to be turned off for a server
	// The avatar image and its accompanying elements should be hidden if there is no MVP for the round.
	if ( m_pMVPAvatar )
	{
		m_pMVPAvatar->SetVisible( isThereAnMVP );
		m_pMVPAvatarBkg->SetVisible( isThereAnMVP );
	}
}

void WinPanel_Round::Show( void )
{
	int iRenderGroup = gHUD.LookupRenderGroupIndexByName( "hide_for_round_panel" );
	if ( iRenderGroup >= 0)
	{
		gHUD.LockRenderGroup( iRenderGroup );
	}
	g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "WinPanelShow" );
}

void WinPanel_Round::Hide( void )
{
	int iRenderGroup = gHUD.LookupRenderGroupIndexByName( "hide_for_round_panel" );
	if ( iRenderGroup >= 0 )
	{
		gHUD.UnlockRenderGroup( iRenderGroup );
	}
	g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "WinPanelHide" );
}

void WinPanel_Round::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	m_clrCT = pScheme->GetColor( "TeamCT", COLOR_WHITE );
	m_clrT = pScheme->GetColor( "TeamT", COLOR_WHITE );
}