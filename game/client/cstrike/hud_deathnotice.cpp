//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Draws CSPort's death notices
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "c_playerresource.h"
#include "iclientmode.h"
#include <vgui_controls/Controls.h>
#include <vgui_controls/Panel.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include <KeyValues.h>
#include "c_baseplayer.h"
#include "c_team.h"
#include "vgui_controls/SVGImage.h"
#include "VGuiMatSurface/IMatSystemSurface.h"

#include "cs_shareddefs.h"
#include "clientmode_csnormal.h"
#include "c_cs_player.h"
#include "c_cs_playerresource.h"


// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static ConVar hud_deathnotice_time( "hud_deathnotice_time", "6", 0 );
static ConVar hud_deathnotice_fade_time( "hud_deathnotice_fade_time", "0.5", 0 );
static ConVar hud_deathnotice_scroll_time( "hud_deathnotice_scroll_time", "0.1", 0 );
ConVar cl_show_clan_in_death_notice( "cl_show_clan_in_death_notice", "1", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "Is set, the clan name will show next to player names in the death notices." );
extern ConVar mp_display_kill_assists;

// Player entries in a death notice
struct DeathNoticePlayer
{
	wchar_t		wszName[MAX_DECORATED_PLAYER_NAME_LENGTH];
	int			iEntIndex;
	Color		color;
};

// Contents of each entry in our list of death notices
struct DeathNoticeItem 
{
	DeathNoticeItem()
	{
		iconDeath = NULL;
	}

	DeathNoticePlayer	Killer;
	DeathNoticePlayer   Victim;
	DeathNoticePlayer   Assister;
	SVGImage	*iconDeath;
	bool		bSuicide;
	float		flTimeRemaining;
	bool		bHeadshot;
	bool		bNoScope;
	bool		bBlind;
	bool		bPenetrated;
	bool		bThruSmoke;
	bool		bDomination;
	bool		bRevenge;
	bool		bAssisted;
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CHudDeathNotice : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CHudDeathNotice, vgui::Panel );
public:
	CHudDeathNotice( const char *pElementName );
	~CHudDeathNotice();

	void Init( void );
	void VidInit( void );
	virtual bool ShouldDraw( void );
	virtual void Paint( void );
	virtual void ApplySchemeSettings( vgui::IScheme *scheme );

	void RetireExpiredDeathNotices( void );
	
	void FireGameEvent( IGameEvent *event );

private:

	CPanelAnimationVarAliasType( int, m_iLineHeight, "LineHeight", "15", "proportional_height" );
	CPanelAnimationVarAliasType( int, m_iHeightMargin, "HeightMargin", "0", "proportional_height" );
	CPanelAnimationVarAliasType( int, m_iBackgroundWidthMargin, "BackgroundWidthMargin", "0", "proportional_width" );
	CPanelAnimationVarAliasType( int, m_iRightMargin, "RightMargin", "0", "proportional_width" );
	CPanelAnimationVarAliasType( int, m_iTopMargin, "TopMargin", "0", "proportional_height" );
	CPanelAnimationVarAliasType( int, m_iBorderSize, "BorderSize", "0", "proportional_height" );
	CPanelAnimationVarAliasType( int, m_iIconMargin, "IconMargin", "0", "proportional_width" );
	CPanelAnimationVarAliasType( int, m_iIconWide, "IconWide", "0", "proportional_width" );
	CPanelAnimationVarAliasType( int, m_iIconTall, "IconTall", "0", "proportional_height" );

	CPanelAnimationVar( vgui::HFont, m_hTextFont, "TextFont", "HudNumbersTimer" );

	CPanelAnimationVar( Color, m_clrCTText, "CTTextColor", "CTTextColor" );
	CPanelAnimationVar( Color, m_clrTerroristText, "TerroristTextColor", "TerroristTextColor" );
	CPanelAnimationVar( Color, m_clrIcons, "IconColor", "White" );
	CPanelAnimationVar( Color, m_clrBg, "BackgroundColor", "Black" );
	CPanelAnimationVar( Color, m_clrVictimBg, "VictimBackgroundColor", "Black" );
	CPanelAnimationVar( Color, m_clrBorder, "BorderColor", "White" );
	CPanelAnimationVar( int, m_iMaxDeathNotices, "MaxDeathNotices", "4" );

	// Texture for skull symbol
	SVGImage		*m_iconD_skull; 
	SVGImage		*m_iconD_headshot;
	SVGImage		*m_iconD_dominated;
	SVGImage		*m_iconD_revenge;
	SVGImage		*m_iconD_noscope; 
	SVGImage		*m_iconD_blind;
	SVGImage		*m_iconD_penetrated;
	SVGImage		*m_iconD_thrusmoke;
	SVGImage		*m_iconD_inferno;

	Color			m_teamColors[TEAM_MAXCOUNT];

	CUtlVector<DeathNoticeItem> m_DeathNotices;
	CUtlStringMap<SVGImage*> m_IconCache;

	void DrawScaledIcon( SVGImage *pIcon, int iconW, int iconH, int yIcon, int &xPos );
};

using namespace vgui;

DECLARE_HUDELEMENT( CHudDeathNotice );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CHudDeathNotice::CHudDeathNotice( const char *pElementName ) :
	CHudElement( pElementName ), BaseClass( NULL, "HudDeathNotice" )
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	m_iconD_skull = new SVGImage;
	m_iconD_headshot = new SVGImage;
	m_iconD_dominated = new SVGImage;
	m_iconD_revenge = new SVGImage;
	m_iconD_noscope = new SVGImage;
	m_iconD_blind = new SVGImage;
	m_iconD_penetrated = new SVGImage;
	m_iconD_thrusmoke = new SVGImage;
	m_iconD_inferno = new SVGImage;

	SetHiddenBits( HIDEHUD_MISCSTATUS );
}

CHudDeathNotice::~CHudDeathNotice()
{
	delete m_iconD_skull;
	m_iconD_skull = NULL;
	delete m_iconD_headshot;
	m_iconD_headshot = NULL;
	delete m_iconD_dominated;
	m_iconD_dominated = NULL;
	delete m_iconD_revenge;
	m_iconD_revenge = NULL;
	delete m_iconD_noscope;
	m_iconD_noscope = NULL;
	delete m_iconD_blind;
	m_iconD_blind = NULL;
	delete m_iconD_penetrated;
	m_iconD_penetrated = NULL;
	delete m_iconD_thrusmoke;
	m_iconD_thrusmoke = NULL;
	delete m_iconD_inferno;
	m_iconD_inferno = NULL;

	m_IconCache.PurgeAndDeleteElements();
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudDeathNotice::ApplySchemeSettings( IScheme *scheme )
{
	BaseClass::ApplySchemeSettings( scheme );
	SetPaintBackgroundEnabled( false );

	// make team color lookups easier
	memset(m_teamColors, 0, sizeof(m_teamColors));
	m_teamColors[TEAM_CT] = m_clrCTText;
	m_teamColors[TEAM_TERRORIST] = m_clrTerroristText;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudDeathNotice::Init( void )
{
	ListenForGameEvent( "player_death" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudDeathNotice::VidInit( void )
{
	m_iconD_skull->SetSize( m_iIconWide, m_iIconTall );
	m_iconD_skull->SetTexture( "materials/vgui/hud/svg/icon_suicide.svg" );
	m_iconD_headshot->SetSize( m_iIconWide, m_iIconTall );
	m_iconD_headshot->SetTexture( "materials/vgui/hud/svg/icon_headshot.svg" );
	m_iconD_dominated->SetSize( m_iIconWide, m_iIconTall );
	m_iconD_dominated->SetTexture( "materials/vgui/hud/svg/domination.svg" );
	m_iconD_revenge->SetSize( m_iIconWide, m_iIconTall );
	m_iconD_revenge->SetTexture( "materials/vgui/hud/svg/revenge.svg" );
	m_iconD_noscope->SetSize( m_iIconWide, m_iIconTall );
	m_iconD_noscope->SetTexture( "materials/vgui/hud/svg/noscope.svg" );
	m_iconD_blind->SetSize( m_iIconWide, m_iIconTall );
	m_iconD_blind->SetTexture( "materials/vgui/hud/svg/blind_kill.svg" );
	m_iconD_penetrated->SetSize( m_iIconWide, m_iIconTall );
	m_iconD_penetrated->SetTexture( "materials/vgui/hud/svg/penetrate.svg" );
	m_iconD_thrusmoke->SetSize( m_iIconWide, m_iIconTall );
	m_iconD_thrusmoke->SetTexture( "materials/vgui/hud/svg/smoke_kill.svg" );
	m_iconD_inferno->SetSize( m_iIconWide, m_iIconTall );
	m_iconD_inferno->SetTexture( "materials/vgui/hud/svg/inferno.svg" );
	m_DeathNotices.Purge();
}

//-----------------------------------------------------------------------------
// Purpose: Draw if we've got at least one death notice in the queue
//-----------------------------------------------------------------------------
bool CHudDeathNotice::ShouldDraw( void )
{
	C_CSPlayer *pPlayer = C_CSPlayer::GetLocalCSPlayer();

	if ( !pPlayer )
		return false;

	// don't show death notices when flashed
	if ( pPlayer->IsAlive() && pPlayer->m_flFlashBangTime >= gpGlobals->curtime )
	{
		float flAlpha = pPlayer->m_flFlashMaxAlpha * (pPlayer->m_flFlashBangTime - gpGlobals->curtime) / pPlayer->m_flFlashDuration;
		if ( flAlpha > 128.0f ) // 0..255
		{
			return false;
		}
	}

	return ( CHudElement::ShouldDraw() && ( m_DeathNotices.Count() ) );
}

//-----------------------------------------------------------------------------
// Purpose: Draw an icon scaled down to prevent overlapping text
//-----------------------------------------------------------------------------
void CHudDeathNotice::DrawScaledIcon( SVGImage *pIcon, int iconW, int iconH, int yIcon, int &xPos )
{
	const float flScale = 0.85f;
	int drawW = (int)(iconW * flScale);
	int drawH = (int)(iconH * flScale);
	int drawX = xPos + (iconW - drawW) / 2;
	int drawY = yIcon + (m_iLineHeight - drawH) / 2;

	int texW, texH;
	surface()->DrawGetTextureSize( pIcon->GetID(), texW, texH );
	float texR = texW > 0 ? (float)iconW / (float)texW : 1.0f;
	float texB = texH > 0 ? (float)iconH / (float)texH : 1.0f;

	surface()->DrawSetTexture( pIcon->GetID() );
	surface()->DrawSetColor( m_clrIcons );
	g_pMatSystemSurface->DisableClipping( true );
	surface()->DrawTexturedSubRect( drawX, drawY, drawX + drawW, drawY + drawH,
		0.0f, 0.0f, texR, texB );
	g_pMatSystemSurface->DisableClipping( false );

	xPos += iconW + m_iIconMargin;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudDeathNotice::Paint()
{
	if ( !m_iconD_headshot || !m_iconD_skull || !m_iconD_dominated ||
		 !m_iconD_revenge || !m_iconD_noscope || !m_iconD_blind ||
		 !m_iconD_penetrated || !m_iconD_thrusmoke || !m_iconD_inferno )
		return;

	int yStart = m_iTopMargin;

	surface()->DrawSetTextFont( m_hTextFont );
	surface()->DrawSetTextColor( m_clrCTText );

	int iconDominationWide, iconDominationTall;
	m_iconD_dominated->GetContentSize( iconDominationWide, iconDominationTall );

	int iconRevengeWide, iconRevengeTall;
	m_iconD_revenge->GetContentSize( iconRevengeWide, iconRevengeTall );

	int iconHeadshotWide, iconHeadshotTall;
	m_iconD_headshot->GetContentSize( iconHeadshotWide, iconHeadshotTall );

	int iconNoScopeWide, iconNoScopeTall;
	m_iconD_noscope->GetContentSize( iconNoScopeWide, iconNoScopeTall );

	int iconBlindWide, iconBlindTall;
	m_iconD_blind->GetContentSize( iconBlindWide, iconBlindTall );

	int iconPenetrateWide, iconPenetrateTall;
	m_iconD_penetrated->GetContentSize( iconPenetrateWide, iconPenetrateTall );

	int iconThruSmokeWide, iconThruSmokeTall;
	m_iconD_thrusmoke->GetContentSize( iconThruSmokeWide, iconThruSmokeTall );

	int iCount = m_DeathNotices.Count();
	for ( int i = 0; i < iCount; i++ )
	{
		SVGImage *icon = m_DeathNotices[i].iconDeath;
		if ( !icon )
			continue;

		float flFadeTime = hud_deathnotice_fade_time.GetFloat();
		float flAlpha = 1.0f;
		float flOldAlpha = surface()->DrawGetAlphaMultiplier();
		if ( m_DeathNotices[i].flTimeRemaining < flFadeTime )
			flAlpha = flOldAlpha * clamp( m_DeathNotices[i].flTimeRemaining, 0.0f, flFadeTime ) / flFadeTime;

		surface()->DrawSetAlphaMultiplier( flAlpha );
		
		int iLocalPlayerIndex = GetLocalPlayerIndex();
		bool bKillerIsLocalPlayer = (m_DeathNotices[i].Killer.iEntIndex == iLocalPlayerIndex);
		bool bVictimIsLocalPlayer = (m_DeathNotices[i].Victim.iEntIndex == iLocalPlayerIndex);
		bool bAssisterIsLocalPlayer = (m_DeathNotices[i].Assister.iEntIndex == iLocalPlayerIndex);

		const wchar_t *victim = m_DeathNotices[i].Victim.wszName;
		const wchar_t *killer = m_DeathNotices[i].Killer.wszName;
		const wchar_t *assister = m_DeathNotices[i].Assister.wszName;

		static wchar_t assistplussign[4] = L" + ";

		// Get the local position for this notice
		int victimNameLen = UTIL_ComputeStringWidth( m_hTextFont, victim );
		int yText = yStart + ((m_iHeightMargin + m_iLineHeight) * i);
		int yIcon = yText;

		float flScrollTime = hud_deathnotice_scroll_time.GetFloat();
		float flScroll = 0.0f;
		if ( m_DeathNotices[i].flTimeRemaining < flScrollTime )
		{
			flScroll = 1.0f - clamp( m_DeathNotices[i].flTimeRemaining, 0.0f, flScrollTime ) / flScrollTime;
			yStart -= ((m_iHeightMargin + m_iLineHeight) * flScroll);
		}

		int iconWide, iconTall;
		icon->GetContentSize( iconWide, iconTall );

		int x = GetWide() - m_iRightMargin - m_iBackgroundWidthMargin;
		x -= victimNameLen;
		x -= iconWide;
		x -= m_iIconMargin + m_iIconMargin;

		if ( m_DeathNotices[i].bBlind )
			x -= iconBlindWide + m_iIconMargin;

		if ( m_DeathNotices[i].bNoScope )
			x -= iconNoScopeWide + m_iIconMargin;

		if ( m_DeathNotices[i].bPenetrated )
			x -= iconPenetrateWide + m_iIconMargin;

		if ( m_DeathNotices[i].bThruSmoke )
			x -= iconThruSmokeWide + m_iIconMargin;

		if ( m_DeathNotices[i].bHeadshot )
			x -= iconHeadshotWide + m_iIconMargin;

		if ( m_DeathNotices[i].bAssisted )
		{
			x -= UTIL_ComputeStringWidth( m_hTextFont, assistplussign );
			x -= UTIL_ComputeStringWidth( m_hTextFont, assister );
		}
			
		//if ( !m_DeathNotices[i].bSuicide )
		{
			x -= UTIL_ComputeStringWidth( m_hTextFont, killer );
		}
			
		if (m_DeathNotices[i].bDomination)
		{
			x -= iconDominationWide + m_iIconMargin;
		}
		if (m_DeathNotices[i].bRevenge)
		{
			x -= iconRevengeWide + m_iIconMargin;
		}

		int bkgX = x - m_iBackgroundWidthMargin;
		int bkgY = yText;
		int bkgWide = GetWide() - m_iRightMargin - bkgX;
		int bkgTall = m_iLineHeight;

		// Draw background first
		DrawBox( bkgX, bkgY,
				 bkgWide, bkgTall,
				 bVictimIsLocalPlayer ? m_clrVictimBg : m_clrBg, 1.0f );
		if ( ((bKillerIsLocalPlayer && !m_DeathNotices[i].bSuicide) || bAssisterIsLocalPlayer) )
			DrawOutlinedBox( bkgX, bkgY, bkgWide, bkgTall, m_clrBorder, 1.0f, MAX(1, m_iBorderSize) ); // HACK: border has 0 size on very low res :(

		int fontTall = surface()->GetFontTall( m_hTextFont );

		if (m_DeathNotices[i].bDomination)
		{
			DrawScaledIcon( m_iconD_dominated, iconDominationWide, iconDominationTall, yIcon, x );
		}
		if (m_DeathNotices[i].bRevenge)
		{
			DrawScaledIcon( m_iconD_revenge, iconRevengeWide, iconRevengeTall, yIcon, x );
		}
		
		// Only draw killers name if it wasn't a suicide
		//if ( !m_DeathNotices[i].bSuicide )
		{
			if ( m_DeathNotices[i].bBlind )
			{
				DrawScaledIcon( m_iconD_blind, iconBlindWide, iconBlindTall, yIcon, x );
			}

			// Draw killer's name
			surface()->DrawSetTextColor( m_DeathNotices[i].Killer.color );
			surface()->DrawSetTextPos( x, yText + ((m_iLineHeight - fontTall) / 2) );
			surface()->DrawSetTextFont( m_hTextFont );
			surface()->DrawUnicodeString( killer );
			surface()->DrawGetTextPos( x, yText );
		}
		
		if ( m_DeathNotices[i].bAssisted )
		{
			// Draw the plus sign in between killer and assister name
			surface()->DrawSetTextColor( m_clrIcons );
			//surface()->DrawSetTextColor( iconColor );
			surface()->DrawSetTextPos( x, yText ); // don't add anything to Y since GetTextPos() above takes care of that
			surface()->DrawSetTextFont( m_hTextFont );
			surface()->DrawUnicodeString( assistplussign );
			surface()->DrawGetTextPos( x, yText );

			// Draw assister's name
			surface()->DrawSetTextColor( m_DeathNotices[i].Assister.color );
			surface()->DrawSetTextPos( x, yText );
			surface()->DrawSetTextFont( m_hTextFont );
			surface()->DrawUnicodeString( assister );
			surface()->DrawGetTextPos( x, yText );
		}

		// Draw death weapon (scaled)
		x += m_iIconMargin;
		DrawScaledIcon( icon, iconWide, iconTall, yIcon, x );

		if( m_DeathNotices[i].bNoScope )
		{
			DrawScaledIcon( m_iconD_noscope, iconNoScopeWide, iconNoScopeTall, yIcon, x );
		}

		if( m_DeathNotices[i].bThruSmoke )
		{
			DrawScaledIcon( m_iconD_thrusmoke, iconThruSmokeWide, iconThruSmokeTall, yIcon, x );
		}

		if( m_DeathNotices[i].bPenetrated )
		{
			DrawScaledIcon( m_iconD_penetrated, iconPenetrateWide, iconPenetrateTall, yIcon, x );
		}

		if( m_DeathNotices[i].bHeadshot )
		{
			DrawScaledIcon( m_iconD_headshot, iconHeadshotWide, iconHeadshotTall, yIcon, x );
		}

		// Draw victims name
		surface()->DrawSetTextColor( m_DeathNotices[i].Victim.color );
		surface()->DrawSetTextPos( x, yText );
		surface()->DrawSetTextFont( m_hTextFont );	//reset the font, draw icon can change it
		surface()->DrawUnicodeString( victim );

		surface()->DrawSetAlphaMultiplier( flOldAlpha );
		m_DeathNotices[i].flTimeRemaining -= gpGlobals->frametime;
		if ( m_DeathNotices[i].flTimeRemaining < 0.0f )
			m_DeathNotices[i].flTimeRemaining = 0.0f;
	}

	// Now retire any death notices that have expired
	RetireExpiredDeathNotices();
}

//-----------------------------------------------------------------------------
// Purpose: This message handler may be better off elsewhere
//-----------------------------------------------------------------------------
void CHudDeathNotice::RetireExpiredDeathNotices( void )
{
	// Loop backwards because we might remove one
	int iSize = m_DeathNotices.Size();
	for ( int i = iSize-1; i >= 0; i-- )
	{
		if ( m_DeathNotices[i].flTimeRemaining <= 0.0f )
		{
			m_DeathNotices.Remove(i);
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Server's told us that someone's died
//-----------------------------------------------------------------------------
void CHudDeathNotice::FireGameEvent( IGameEvent *event )
{
	if (!g_PR)
		return;

	C_CS_PlayerResource *cs_PR = dynamic_cast<C_CS_PlayerResource *>( g_PR );
	if ( !cs_PR )
		return;

	if ( hud_deathnotice_time.GetFloat() == 0 )
		return;

	// the event should be "player_death"
	
	int iKiller = engine->GetPlayerForUserID( event->GetInt("attacker") );
	int iAssister = mp_display_kill_assists.GetBool() ? engine->GetPlayerForUserID( event->GetInt("assister") ) : 0;
	int iVictim = engine->GetPlayerForUserID( event->GetInt("userid") );
	const char *killedwith = event->GetString( "weapon" );
	bool headshot = event->GetInt( "headshot" ) > 0;
	bool noscope = event->GetInt( "noscope" ) > 0;
	bool blind = event->GetInt( "blind" ) > 0;
	bool penetrated = event->GetInt( "penetrated" ) > 0;

	C_CSPlayer* pKiller = ToCSPlayer( ClientEntityList().GetBaseEntity( iKiller ) );
	C_CSPlayer* pVictim = ToCSPlayer( ClientEntityList().GetBaseEntity( iVictim ) );

	if ( !iKiller )
	{
		// PiMoN: assuming that we're suicided, so the killer equals a victim
		// probably won't cause any problems like that but if it will, then
		// its and easy-fix by just adding a check for suicide ( !iKiller || iKiller == iVictim );
		iKiller = iVictim;
	}

	bool thrusmoke = false;
	if ( pKiller && pVictim )
		thrusmoke = LineGoesThroughSmoke( pKiller->GetAbsOrigin(), pVictim->GetAbsOrigin(), 1.0f );

	// no thrusmoke icon for grenades
	if ( thrusmoke )
	{
		char pWeaponName[64];
		V_sprintf_safe( pWeaponName, "weapon_%s", killedwith );
		WEAPON_FILE_INFO_HANDLE	hWpnInfo = LookupWeaponInfoSlot( pWeaponName );
		if ( hWpnInfo != GetInvalidWeaponInfoHandle() )
		{
			CCSWeaponInfo *pWeaponInfo = dynamic_cast<CCSWeaponInfo*>(GetFileWeaponInfoFromHandle( hWpnInfo ));
			if ( pWeaponInfo )
				thrusmoke = (pWeaponInfo->m_WeaponType != WEAPONTYPE_GRENADE);
		}
		// no thrusmoke icon for inferno as well
		else if ( !V_strcmp( killedwith, "inferno" ) )
			thrusmoke = false;
	}

	char fullkilledwith[64];
	if ( killedwith && *killedwith )
	{
		Q_snprintf( fullkilledwith, sizeof(fullkilledwith), "materials/vgui/weapons/svg/%s.svg", killedwith );
	}
	else
	{
		fullkilledwith[0] = 0;
	}

	// Do we have too many death messages in the queue?
	if ( m_DeathNotices.Count() > 0 &&
		m_DeathNotices.Count() >= m_iMaxDeathNotices )
	{
		// Remove the oldest one in the queue, which will always be the first
		m_DeathNotices.Remove(0);
	}

	// Get the names of the players
	wchar_t wszKillerName[MAX_DECORATED_PLAYER_NAME_LENGTH];
	wchar_t wszVictimName[MAX_DECORATED_PLAYER_NAME_LENGTH];
	wchar_t wszAssisterName[MAX_DECORATED_PLAYER_NAME_LENGTH];

	wszKillerName[0] = L'\0';
	wszVictimName[0] = L'\0';
	wszAssisterName[0] = L'\0';

	EDecoratedPlayerNameFlag_t kDontShowClanName = k_EDecoratedPlayerNameFlag_DontShowClanName;
	if ( cl_show_clan_in_death_notice.GetInt() > 0 )
		kDontShowClanName = k_EDecoratedPlayerNameFlag_Simple;

	if ( iKiller > 0 )
	{
		cs_PR->GetDecoratedPlayerName( iKiller, wszKillerName, sizeof( wszKillerName ), EDecoratedPlayerNameFlag_t( k_EDecoratedPlayerNameFlag_AddBotToNameIfControllingBot | kDontShowClanName ) );
	}

	if ( iVictim > 0 )
	{
		cs_PR->GetDecoratedPlayerName( iVictim, wszVictimName, sizeof( wszVictimName ), EDecoratedPlayerNameFlag_t( k_EDecoratedPlayerNameFlag_AddBotToNameIfControllingBot | kDontShowClanName ) );
	}

	if ( iAssister > 0 )
	{
		// if our attacker is the same as our assiter, it means a bot attacked the victim and a player took over that bot
		if ( iAssister == iKiller || cs_PR->GetControlledByPlayer(iAssister) == iKiller ) // TODO: GetControlledByPlayer check is a hack!
			iAssister = 0;
		else
		{
			cs_PR->GetDecoratedPlayerName( iAssister, wszAssisterName, sizeof( wszAssisterName ), EDecoratedPlayerNameFlag_t( k_EDecoratedPlayerNameFlag_AddBotToNameIfControllingBot | kDontShowClanName ) );
		}
	}

	// Make a new death notice
	DeathNoticeItem deathMsg;
	deathMsg.Killer.iEntIndex = iKiller;
	deathMsg.Victim.iEntIndex = iVictim;
	deathMsg.Assister.iEntIndex = iAssister;
	deathMsg.Killer.color = iKiller > 0 ? m_teamColors[g_PR->GetTeam(iKiller)] : COLOR_WHITE;
	deathMsg.Victim.color = iVictim > 0 ? m_teamColors[g_PR->GetTeam(iVictim)] : COLOR_WHITE;
	deathMsg.Assister.color = iAssister > 0 ? m_teamColors[g_PR->GetTeam(iAssister)] : COLOR_WHITE;
	Q_wcsncpy( deathMsg.Killer.wszName, wszKillerName, MAX_DECORATED_PLAYER_NAME_LENGTH );
	Q_wcsncpy( deathMsg.Victim.wszName, wszVictimName, MAX_DECORATED_PLAYER_NAME_LENGTH );
	Q_wcsncpy( deathMsg.Assister.wszName, wszAssisterName, MAX_DECORATED_PLAYER_NAME_LENGTH );
	deathMsg.flTimeRemaining = hud_deathnotice_time.GetFloat();
	deathMsg.bSuicide = ( !iKiller || iKiller == iVictim );
	deathMsg.bHeadshot = headshot;
	deathMsg.bNoScope = noscope;
	deathMsg.bBlind = blind;
	deathMsg.bPenetrated = penetrated;
	deathMsg.bThruSmoke = thrusmoke;
	deathMsg.bDomination = event->GetInt( "dominated" ) > 0 || (pKiller != NULL && pKiller->IsPlayerDominated( iVictim ));
	deathMsg.bRevenge = event->GetInt( "revenge" ) > 0;
	deathMsg.bAssisted = iAssister > 0;

	SVGImage *pngIcon = NULL;
	UtlSymId_t sym = m_IconCache.Find( killedwith );
	if ( sym == m_IconCache.InvalidIndex() )
	{
		pngIcon = new SVGImage;
		pngIcon->SetSize( m_iIconWide, m_iIconTall );
		if ( !pngIcon->SetTexture( fullkilledwith ) )
		{
			// Can't find it, so use the default skull & crossbones icon
			*pngIcon = *m_iconD_skull;
		}
		sym = m_IconCache.Insert( killedwith, pngIcon );
	}
	
	deathMsg.iconDeath = m_IconCache[sym];

	// Add it to our list of death notices
	m_DeathNotices.AddToTail( deathMsg );

	char sDeathMsg[512];
	char szVictimName[MAX_DECORATED_PLAYER_NAME_LENGTH];
	char szKillerName[MAX_DECORATED_PLAYER_NAME_LENGTH];
	g_pVGuiLocalize->ConvertUnicodeToANSI( wszVictimName, szVictimName, sizeof( szVictimName ) );
	g_pVGuiLocalize->ConvertUnicodeToANSI( wszKillerName, szKillerName, sizeof( szKillerName ) );

	// Record the death notice in the console
	if ( deathMsg.bSuicide )
	{
		if ( !strcmp( fullkilledwith, "d_planted_c4" ) )
		{
			Q_snprintf( sDeathMsg, sizeof( sDeathMsg ), "%s was a bit too close to the c4.\n", szVictimName );
		}
		else if ( !strcmp( fullkilledwith, "d_worldspawn" ) )
		{
			Q_snprintf( sDeathMsg, sizeof( sDeathMsg ), "%s died.\n", szVictimName );
		}
		else	//d_world
		{
			Q_snprintf( sDeathMsg, sizeof( sDeathMsg ), "%s suicided.\n", szVictimName );
		}
	}
	else
	{
		Q_snprintf( sDeathMsg, sizeof( sDeathMsg ), "%s killed %s", szKillerName, szVictimName );

		if ( fullkilledwith && *fullkilledwith && (*fullkilledwith > 13 ) )
		{
			Q_strncat( sDeathMsg, VarArgs( " with %s.\n", fullkilledwith + 2 ), sizeof( sDeathMsg ), COPY_ALL_CHARACTERS );
		}
	}

	Msg( "%s", sDeathMsg );
}


