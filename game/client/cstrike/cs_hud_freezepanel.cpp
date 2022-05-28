//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "cs_hud_freezepanel.h"
#include <vgui/IVGui.h>
#include "vgui_controls/AnimationController.h"
#include "iclientmode.h"
#include "c_cs_player.h"
#include "c_cs_playerresource.h"
#include <vgui_controls/Label.h>
#include <vgui_controls/ProgressBar.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include "VGUI/bordered_panel.h"
#include "fmtstr.h"
#include "cs_gamerules.h"
#include "view.h"
#include "ivieweffects.h"
#include "viewrender.h"
#include "usermessages.h"
#include "hud_macros.h"
#include "c_baseanimating.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

DECLARE_HUDELEMENT_DEPTH( CCSFreezePanel, 1 );

ConVar cl_disablefreezecam(
	"cl_disablefreezecam",
	"0",
	FCVAR_ARCHIVE,
	"Turn on/off freezecam on client"
	);

extern Color LerpColors( Color cStart, Color cEnd, float flPercent );
extern ConVar cl_draw_only_deathnotices;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CCSFreezePanel::CCSFreezePanel( const char *pElementName ) :
	EditablePanel( NULL, "FreezePanel" ), 
	CHudElement( pElementName )
{
	SetParent(g_pClientMode->GetViewport());
	m_bShouldBeVisible = false;
	RegisterForRenderGroup( "hide_for_scoreboard" );

	m_pAvatar = new CAvatarImagePanel( this, "AvatarImage" );
	m_pKillerHealth = new vgui::ContinuousProgressBarWithBorder( this, "KillerHealth" );
	m_pDominationIcon = new ImagePanel( this, "DominationIcon" );
	m_pDamageTakenBackground = new ImagePanel( this, "DamageTakenBackground" );
	m_pDamageGivenBackground = new ImagePanel( this, "DamageGivenBackground" );
	m_pDamageTakenLabel = new Label( this, "DamageTakenLabel", L" " );
	m_pDamageGivenLabel = new Label( this, "DamageGivenLabel", L" " );
	m_pScreenshotLabel = new Label( this, "ScreenshotLabel", L"ScreenshotLabel" );

	m_pAvatar->SetDefaultAvatar( scheme()->GetImage( CSTRIKE_DEFAULT_AVATAR, true ) );
	m_pAvatar->SetShouldScaleImage( true );
	m_pAvatar->SetShouldDrawFriendIcon( false );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CCSFreezePanel::Reset()
{
	Hide();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CCSFreezePanel::Init()
{
	CHudElement::Init();

	// listen for events
	ListenForGameEvent( "show_freezepanel" );
	ListenForGameEvent( "hide_freezepanel" );
	ListenForGameEvent( "freezecam_started" );
	ListenForGameEvent( "player_death" );

	Hide();

	LoadControlSettings( "resource/UI/FreezePanel_Basic.res" );
}

//-----------------------------------------------------------------------------
// Purpose: Applies scheme settings
//-----------------------------------------------------------------------------
void CCSFreezePanel::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CCSFreezePanel::FireGameEvent( IGameEvent * event )
{
	const char *pEventName = event->GetName();

	if ( Q_strcmp( "player_death", pEventName ) == 0 )
	{
		// see if the local player died
		int iPlayerIndexVictim = engine->GetPlayerForUserID( event->GetInt( "userid" ) );
		int iPlayerIndexKiller = engine->GetPlayerForUserID( event->GetInt( "attacker" ) );
		const char *szWeapon = event->GetString( "weapon" );
		C_BasePlayer *pLocalPlayer = C_BasePlayer::GetLocalPlayer();
		C_CSPlayer* pKiller = ToCSPlayer( ClientEntityList().GetBaseEntity( iPlayerIndexKiller ) );

		if ( pLocalPlayer && iPlayerIndexVictim == pLocalPlayer->entindex() )
		{
			// the local player is dead, see if this is a new nemesis or a revenge
			if ( event->GetInt( "dominated" ) > 0)
			{
				m_pDominationIcon->SetImage( "hud/freeze_nemesis" );
				m_pDominationIcon->SetVisible( true );

				//SetDialogVariable( "InfoLabel", g_pVGuiLocalize->Find("#FreezePanel_NewNemesis"));
			}
			// was the killer your pre-existing nemesis?
			else if ( pKiller && pKiller->IsPlayerDominated( iPlayerIndexVictim ) )
			{
				m_pDominationIcon->SetImage( "hud/freeze_nemesis" );
				m_pDominationIcon->SetVisible( true );

				//SetDialogVariable( "InfoLabel", g_pVGuiLocalize->Find("#FreezePanel_OldNemesis"));
			}
			else if ( event->GetInt( "revenge" ) > 0 )
			{
				m_pDominationIcon->SetImage( "hud/freeze_revenge" );
				m_pDominationIcon->SetVisible( true );

				//SetDialogVariable( "InfoLabel", g_pVGuiLocalize->Find("#FreezePanel_Revenge"));
			}
			else if ( pKiller == pLocalPlayer || pKiller == NULL )
			{
				m_pDominationIcon->SetVisible( false );

				// is it a grenade suicide?
				// only he and decoy is included because you cant suicide from other grenades (no shit)
				/*if ( StringHasPrefixCaseSensitive(szWeapon, "hegrenade") ||
					 StringHasPrefixCaseSensitive( szWeapon, "decoy" ) )
				{
					SetDialogVariable( "InfoLabel", g_pVGuiLocalize->Find( "#FreezePanel_KilledByOwnGrenade" ) );
				}
				else if ( szWeapon && szWeapon[0] && !V_strcmp( szWeapon, "inferno" ) )
				{
					SetDialogVariable( "InfoLabel", g_pVGuiLocalize->Find( "#FreezePanel_KilledByFire" ) );
				}
				else
				{
					SetDialogVariable( "InfoLabel", g_pVGuiLocalize->Find( "#FreezePanel_KilledSelf" ) );
				}*/
			}
			else if ( szWeapon && szWeapon[0] )
			{
				m_pDominationIcon->SetVisible( false );
				/*if ( pKiller )
				{
					const char *pszLocString = "#FreezePanel_Killer_Weapon";
					wchar_t wszLocalizedString[256];

					CSWeaponID nWeaponID = AliasToWeaponID( szWeapon );
					WEAPON_FILE_INFO_HANDLE	hWpnInfo = LookupWeaponInfoSlot( WeaponIdAsString( nWeaponID ) );
					if ( hWpnInfo == GetInvalidWeaponInfoHandle() )
					{
						if ( StringHasPrefixCaseSensitive( szWeapon, "hegrenade" ) )
						{
							g_pVGuiLocalize->ConstructString( wszLocalizedString, sizeof( wszLocalizedString ), g_pVGuiLocalize->Find( pszLocString ), 1, g_pVGuiLocalize->Find( "#Cstrike_WPNHUD_HE_Grenade" ) );
						}
						else if ( StringHasPrefixCaseSensitive( szWeapon, "flashbang" ) )
						{
							g_pVGuiLocalize->ConstructString( wszLocalizedString, sizeof( wszLocalizedString ), g_pVGuiLocalize->Find( pszLocString ), 1, g_pVGuiLocalize->Find( "#Cstrike_WPNHUD_Flashbang" ) );
						}
						else if ( StringHasPrefixCaseSensitive( szWeapon, "decoy" ) )
						{
							g_pVGuiLocalize->ConstructString( wszLocalizedString, sizeof( wszLocalizedString ), g_pVGuiLocalize->Find( pszLocString ), 1, g_pVGuiLocalize->Find( "#Cstrike_WPNHUD_Decoy" ) );
						}
						else if ( StringHasPrefixCaseSensitive( szWeapon, "smokegrenade" ) )
						{
							g_pVGuiLocalize->ConstructString( wszLocalizedString, sizeof( wszLocalizedString ), g_pVGuiLocalize->Find( pszLocString ), 1, g_pVGuiLocalize->Find( "#Cstrike_WPNHUD_Smoke_Grenade" ) );
						}
						else if ( StringHasPrefixCaseSensitive( szWeapon, "incgrenade" ) )
						{
							g_pVGuiLocalize->ConstructString( wszLocalizedString, sizeof( wszLocalizedString ), g_pVGuiLocalize->Find( pszLocString ), 1, g_pVGuiLocalize->Find( "#Cstrike_WPNHUD_IncGrenade" ) );
						}
						else if ( StringHasPrefixCaseSensitive( szWeapon, "molotov" ) )
						{
							g_pVGuiLocalize->ConstructString( wszLocalizedString, sizeof( wszLocalizedString ), g_pVGuiLocalize->Find( pszLocString ), 1, g_pVGuiLocalize->Find( "#Cstrike_WPNHUD_Molotov" ) );
						}
						else if ( !V_strcmp(szWeapon, "inferno") )
						{
							V_swprintf_safe( wszLocalizedString, L"" FMT_WS, g_pVGuiLocalize->Find( "#FreezePanel_KilledByFire" ) );
							SetDialogVariable( "InfoLabel", wszLocalizedString );
						}
						else
						{
							return;
						}
					}
					else
					{
						CCSWeaponInfo *pWeaponInfo = dynamic_cast<CCSWeaponInfo*>(GetFileWeaponInfoFromHandle( hWpnInfo ));
						if ( !pWeaponInfo )
							return;

						if ( nWeaponID == WEAPON_ELITE )
							pszLocString = "#FreezePanel_Killer_Weapon_Plural";

						g_pVGuiLocalize->ConstructString( wszLocalizedString, sizeof( wszLocalizedString ), g_pVGuiLocalize->Find( pszLocString ), 1, g_pVGuiLocalize->Find( pWeaponInfo->szPrintName ) );
					}
					SetDialogVariable( "InfoLabel", wszLocalizedString );
				}*/
			}

			wchar_t wzBind[16] = L"";
			wchar_t navBarText[256] = L"";
			UTIL_ReplaceKeyBindings( L"%jpeg%", 0, wzBind, sizeof( wzBind ) );
			g_pVGuiLocalize->ConstructString( navBarText, sizeof( navBarText ), g_pVGuiLocalize->Find( "#FreezePanel_ScreenShot" ), 1, wzBind );
			m_pScreenshotLabel->SetText( navBarText );
		}
	}
	else if ( Q_strcmp( "hide_freezepanel", pEventName ) == 0 )
	{
		Hide();
	}
	else if ( Q_strcmp( "freezecam_started", pEventName ) == 0 )
	{
	}
	else if ( Q_strcmp( "show_freezepanel", pEventName ) == 0 )
	{
		C_CS_PlayerResource *cs_PR = dynamic_cast<C_CS_PlayerResource *>( g_PR );
		if ( !cs_PR )
			return;

		Show();

		// Get the entity who killed us
		int iKillerIndex = event->GetInt( "killer" );
		int iVictimIndex = event->GetInt( "victim" ); // always the local player
		if ( iKillerIndex == 0 ) // world killed us, means it was a suicide
			iKillerIndex = iVictimIndex;
		CCSPlayer* pKiller = ToCSPlayer(ClientEntityList().GetBaseEntity(iKillerIndex));
		m_pAvatar->ClearAvatar();

		if ( pKiller )
		{
			int iMaxHealth = pKiller->GetMaxHealth();
			int iKillerHealth = pKiller->GetHealth();
			if ( !pKiller->IsAlive() )
			{
				iKillerHealth = 0;
			}

			static ConVarRef sv_damage_print_enable( "sv_damage_print_enable" );

			m_pKillerHealth->SetProgress( clamp( (float) iKillerHealth / (float) iMaxHealth, 0.0f, 1.0f ) );
			// do not show health bar in competitive and if damage print is disabled
			m_pKillerHealth->SetVisible( sv_damage_print_enable.GetBool() && !CSGameRules()->IsPlayingAnyCompetitiveStrictRuleset() );

			wchar_t wszkillerName[MAX_DECORATED_PLAYER_NAME_LENGTH];
			wszkillerName[0] = '\0';
			cs_PR->GetDecoratedPlayerName( iKillerIndex, wszkillerName, sizeof( wszkillerName ), k_EDecoratedPlayerNameFlag_AddBotToNameIfControllingBot );

			int nHitsTaken = 0;
			int nDamTaken = 0;
			int nHitsGiven = 0;
			int nDamGiven = 0;

			if ( sv_damage_print_enable.GetBool() )
			{
				nHitsTaken = event->GetInt( "hits_taken" );
				nDamTaken = event->GetInt( "damage_taken" );
				nHitsGiven = event->GetInt( "hits_given" );
				nDamGiven = event->GetInt( "damage_given" );
			}

			wchar_t wszLocalizedDamageString[256];
			wchar_t wszDamage[8];
			wchar_t wszHits[8];
			if ( nDamTaken )
			{
				V_snwprintf( wszDamage, sizeof( wszDamage ), L"%d", nDamTaken );
				V_snwprintf( wszHits, sizeof( wszHits ), L"%d", nHitsTaken );
				g_pVGuiLocalize->ConstructString( wszLocalizedDamageString, sizeof( wszLocalizedDamageString ),
												  g_pVGuiLocalize->Find( nHitsTaken > 1 ? "#FreezePanel_DamageTaken_Multi" : "#FreezePanel_DamageTaken" ), 3,
												  wszDamage, wszHits, wszkillerName );

				m_pDamageTakenLabel->SetText( wszLocalizedDamageString );
				m_pScreenshotLabel->SetPos( m_pScreenshotLabel->GetXPos(), screenshot_label_ypos_damage_taken );

				m_pDamageTakenLabel->SetVisible( true );
				m_pDamageTakenBackground->SetVisible( true );
			}
			else
			{
				m_pDamageTakenLabel->SetVisible( false );
				m_pDamageTakenBackground->SetVisible( false );
			}

			if ( nDamGiven )
			{
				V_snwprintf( wszDamage, sizeof( wszDamage ), L"%d", nDamGiven );
				V_snwprintf( wszHits, sizeof( wszHits ), L"%d", nHitsGiven );
				g_pVGuiLocalize->ConstructString( wszLocalizedDamageString, sizeof( wszLocalizedDamageString ),
												  g_pVGuiLocalize->Find( nHitsGiven > 1 ? "#FreezePanel_DamageGiven_Multi" : "#FreezePanel_DamageGiven" ), 3,
												  wszDamage, wszHits, wszkillerName );

				m_pDamageGivenLabel->SetText( wszLocalizedDamageString );
				m_pScreenshotLabel->SetPos( m_pScreenshotLabel->GetXPos(), screenshot_label_ypos_damage_given );

				m_pDamageGivenLabel->SetVisible( true );
				m_pDamageGivenBackground->SetVisible( true );
			}
			else
			{
				m_pDamageGivenLabel->SetVisible( false );
				m_pDamageGivenBackground->SetVisible( false );
			}

			if ( !nDamGiven && !nDamTaken )
			{
				m_pScreenshotLabel->SetPos( m_pScreenshotLabel->GetXPos(), screenshot_label_ypos );
			}

			SetDialogVariable( "killername", wszkillerName );

			int iKillerIndex = pKiller->entindex();
			player_info_t pi;

			m_pAvatar->SetDefaultAvatar( GetDefaultAvatarImage( pKiller ) );

			if ( engine->GetPlayerInfo(iKillerIndex, &pi) )
			{
				m_pAvatar->SetPlayer( (C_BasePlayer*)pKiller, k_EAvatarSize64x64);
				m_pAvatar->SetVisible(true);
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CCSFreezePanel::ShouldDraw( void )
{
	if ( cl_draw_only_deathnotices.GetBool() )
		return false;

	// [Forrest] Added sv_disablefreezecam check
	static ConVarRef sv_disablefreezecam( "sv_disablefreezecam" );
	return ( m_bShouldBeVisible && !cl_disablefreezecam.GetBool() && !sv_disablefreezecam.GetBool() && CHudElement::ShouldDraw() );
}

void CCSFreezePanel::OnScreenSizeChanged( int nOldWide, int nOldTall )
{
	BaseClass::OnScreenSizeChanged(nOldWide, nOldTall);

	LoadControlSettings( "resource/UI/FreezePanel_Basic.res" );
}

void CCSFreezePanel::SetActive( bool bActive )
{
	CHudElement::SetActive( bActive );

	if ( bActive )
	{
		// Setup replay key binding in UI
		const char *pKey = engine->Key_LookupBinding( "save_replay" );
		if ( pKey == NULL || FStrEq( pKey, "(null)" ) )
		{
			pKey = "<NOT BOUND>";
		}

		char szKey[16];
		Q_snprintf( szKey, sizeof(szKey), "%s", pKey );
		wchar_t wKey[16];
		wchar_t wLabel[256];

		g_pVGuiLocalize->ConvertANSIToUnicode( szKey, wKey, sizeof( wKey ) );
		g_pVGuiLocalize->ConstructString( wLabel, sizeof( wLabel ), g_pVGuiLocalize->Find("#FreezePanel_SaveReplay" ), 1, wKey );

		SetDialogVariable( "savereplay", wLabel );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CCSFreezePanel::Show()
{
	m_bShouldBeVisible = true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CCSFreezePanel::Hide()
{
	m_bShouldBeVisible = false;
}
