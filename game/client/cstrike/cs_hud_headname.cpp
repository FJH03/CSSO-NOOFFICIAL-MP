#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "iclientmode.h"
#include "c_cs_player.h"
#include "c_cs_team.h"
#include "c_cs_playerresource.h"
#include "vgui/ILocalize.h"
#include "vgui/ISurface.h"
#include "vgui_controls/EditablePanel.h"
#include "voice_status.h"
#include "filesystem.h"
#include "lunasvg/lunasvg.h"
#include "convar.h"

using namespace vgui;
using namespace lunasvg;

#define MAX_HEADNAME_ICONS 6

static const int ICON_H   = 14;
static const int ICON_GAP = 2;
static const int ROW_GAP  = 2;

struct HeadIcon_t
{
	int   texId;
	int   dispWidth;
	float uvs[4];
	char  path[128];

	HeadIcon_t() : texId(-1), dispWidth(ICON_H)
	{
		uvs[0] = uvs[1] = 0.0f;
		uvs[2] = uvs[3] = 1.0f;
		path[0] = '\0';
	}
};

class CHudHeadName : public CHudElement, public EditablePanel
{
	DECLARE_CLASS_SIMPLE( CHudHeadName, EditablePanel );

public:
	CHudHeadName( const char *name );
	~CHudHeadName();

	virtual bool ShouldDraw();
	virtual void Paint();

private:
	void DrawPlayerNames();
	int  BuildIconList( C_CSPlayer *pPlayer, char out[MAX_HEADNAME_ICONS][128] );
	void LoadIcon( HeadIcon_t &icon, const char *path );
	void DrawIcons( int slot, C_CSPlayer *pPlayer, int cx, int bottomY, int alpha );

	CPanelAnimationVar( HFont, m_hFont, "item_font", "HeadName" );

	HeadIcon_t m_Icons[MAX_PLAYERS][MAX_HEADNAME_ICONS];
};

DECLARE_HUDELEMENT( CHudHeadName );

CHudHeadName::CHudHeadName( const char *name )
	: CHudElement( name ), EditablePanel( NULL, "HudHeadName" )
{
	SetParent( g_pClientMode->GetViewport() );
}

CHudHeadName::~CHudHeadName()
{
	for ( int s = 0; s < MAX_PLAYERS; s++ )
		for ( int i = 0; i < MAX_HEADNAME_ICONS; i++ )
			if ( m_Icons[s][i].texId != -1 )
				vgui::surface()->DestroyTextureID( m_Icons[s][i].texId );
}

ConVar cl_headname( "cl_headname", "1", FCVAR_ARCHIVE );

bool CHudHeadName::ShouldDraw()
{
	ConVarRef mp_teammates_are_enemies( "mp_teammates_are_enemies" );
	return !mp_teammates_are_enemies.GetBool() && cl_headname.GetBool();
}

static bool WorldToScreen( const Vector &world, Vector2D &screen )
{
	const VMatrix &m = engine->WorldToScreenMatrix();

	float w = m[3][0]*world.x + m[3][1]*world.y + m[3][2]*world.z + m[3][3];
	if ( w < 0.001f )
		return false;

	float iw = 1.0f / w;
	float nx  = ( m[0][0]*world.x + m[0][1]*world.y + m[0][2]*world.z + m[0][3] ) * iw;
	float ny  = ( m[1][0]*world.x + m[1][1]*world.y + m[1][2]*world.z + m[1][3] ) * iw;

	int sw, sh;
	vgui::surface()->GetScreenSize( sw, sh );
	screen.x = sw * 0.5f + nx * sw * 0.5f;
	screen.y = sh * 0.5f - ny * sh * 0.5f;
	return true;
}

void CHudHeadName::LoadIcon( HeadIcon_t &icon, const char *path )
{
	if ( icon.texId != -1 )
	{
		vgui::surface()->DestroyTextureID( icon.texId );
		icon.texId = -1;
	}

	FileHandle_t f = g_pFullFileSystem->Open( path, "rt" );
	if ( !f )
		return;

	int size = g_pFullFileSystem->Size( f );
	char *buf = (char *)malloc( size + 1 );
	int read = g_pFullFileSystem->ReadEx( buf, size + 1, size, f );
	buf[read] = '\0';
	g_pFullFileSystem->Close( f );

	auto doc = Document::loadFromData( buf );
	free( buf );
	if ( !doc )
		return;

	Bitmap bmp = doc->renderToBitmap( 0, 0 );
	if ( !bmp.valid() )
		return;

	icon.texId = vgui::surface()->CreateNewTextureID( true );

	int bw = bmp.width(), bh = bmp.height();
	vgui::surface()->DrawSetTextureRGBA( icon.texId, bmp.data(), bw, bh, 1, true );

	int tw, th;
	vgui::surface()->DrawGetTextureSize( icon.texId, tw, th );
	icon.uvs[0] = 0.0f;
	icon.uvs[1] = 0.0f;
	icon.uvs[2] = tw > 0 ? (float)bw / tw : 1.0f;
	icon.uvs[3] = th > 0 ? (float)bh / th : 1.0f;
	icon.dispWidth = bh > 0 ? ( bw * ICON_H ) / bh : ICON_H;

	Q_strncpy( icon.path, path, sizeof( icon.path ) );
}

int CHudHeadName::BuildIconList( C_CSPlayer *pPlayer, char out[MAX_HEADNAME_ICONS][128] )
{
	int n = 0;

	if ( pPlayer->HasDefuser() )
		Q_strncpy( out[n++], "materials/vgui/hud/svg/defuser.svg", 128 );

	if ( pPlayer->HasC4() )
		Q_strncpy( out[n++], "materials/vgui/weapons/svg/c4.svg", 128 );

	C_BaseCombatWeapon *pPrimary   = NULL;
	C_BaseCombatWeapon *pSecondary = NULL;
	C_BaseCombatWeapon *pGrenades[3] = {};
	int nGrenades = 0;

	for ( int i = 0; i < MAX_WEAPONS && n < MAX_HEADNAME_ICONS; i++ )
	{
		C_BaseCombatWeapon *pW = pPlayer->GetWeapon( i );
		if ( !pW )
			continue;

		const char *cls = pW->GetClassname();
		if ( Q_stristr( cls, "knife" ) || Q_stristr( cls, "weapon_c4" ) )
			continue;

		switch ( pW->GetSlot() )
		{
		case WEAPON_SLOT_RIFLE:    if ( !pPrimary )              pPrimary   = pW; break;
		case WEAPON_SLOT_PISTOL:   if ( !pSecondary )            pSecondary = pW; break;
		case WEAPON_SLOT_GRENADES: if ( nGrenades < 3 )          pGrenades[nGrenades++] = pW; break;
		}
	}

	for ( int i = 0; i < nGrenades && n < MAX_HEADNAME_ICONS; i++ )
		Q_snprintf( out[n++], 128, "materials/vgui/weapons/svg/%s.svg", pGrenades[i]->GetClassname() + 7 );

	C_BaseCombatWeapon *pMain = pPrimary ? pPrimary : pSecondary;
	if ( pMain && n < MAX_HEADNAME_ICONS )
		Q_snprintf( out[n++], 128, "materials/vgui/weapons/svg/%s.svg", pMain->GetClassname() + 7 );

	return n;
}

void CHudHeadName::DrawIcons( int slot, C_CSPlayer *pPlayer, int cx, int bottomY, int alpha )
{
	char paths[MAX_HEADNAME_ICONS][128];
	int n = BuildIconList( pPlayer, paths );
	if ( n == 0 )
		return;

	int totalW = 0;
	for ( int i = 0; i < n; i++ )
	{
		HeadIcon_t &icon = m_Icons[slot][i];
		if ( Q_strcmp( icon.path, paths[i] ) != 0 )
			LoadIcon( icon, paths[i] );
		if ( icon.texId != -1 )
			totalW += icon.dispWidth + ( i > 0 ? ICON_GAP : 0 );
	}

	int curX = cx - totalW / 2;
	int iconY = bottomY - ICON_H;

	for ( int i = 0; i < n; i++ )
	{
		HeadIcon_t &icon = m_Icons[slot][i];
		if ( icon.texId == -1 )
			continue;

		vgui::surface()->DrawSetColor( 255, 255, 255, alpha );
		vgui::surface()->DrawSetTexture( icon.texId );
		vgui::surface()->DrawTexturedSubRect(
			curX,                  iconY,
			curX + icon.dispWidth, iconY + ICON_H,
			icon.uvs[0], icon.uvs[1], icon.uvs[2], icon.uvs[3] );

		curX += icon.dispWidth + ICON_GAP;
	}
}

void CHudHeadName::DrawPlayerNames()
{
	CBasePlayer *local = CBasePlayer::GetLocalPlayer();
	if ( !local )
		return;

	int localTeam = local->GetTeamNumber();
	int sw, sh;
	vgui::surface()->GetScreenSize( sw, sh );
	vgui::surface()->DrawSetTextFont( m_hFont );

	wchar_t arrow[] = { 0x25BC, 0 };
	int arrowW, arrowH;
	vgui::surface()->GetTextSize( m_hFont, arrow, arrowW, arrowH );

	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CBasePlayer *player = UTIL_PlayerByIndex( i );
		if ( !player || player == local || !player->IsAlive() )
			continue;
		if ( player->GetTeamNumber() != localTeam )
			continue;

		Vector headPos = player->WorldSpaceCenter();
		headPos.z += GetClientVoiceMgr()->GetHeadLabelOffset() + 10;

		Vector2D sp;
		if ( !WorldToScreen( headPos, sp ) )
			continue;
		if ( sp.x < 0 || sp.x > sw || sp.y < 0 || sp.y > sh )
			continue;

		float dist = ( local->GetAbsOrigin() - player->GetAbsOrigin() ).Length();
		int alpha  = dist < 300.0f ? 255 : 100;
		int cx     = (int)sp.x;
		int textY  = (int)sp.y;

		char raw[128];
		Q_snprintf( raw, sizeof(raw), "%s %d%%", player->GetPlayerName(), player->GetHealth() );
		wchar_t wide[64];
		g_pVGuiLocalize->ConvertANSIToUnicode( raw, wide, sizeof(wide) );

		int textW, textH;
		vgui::surface()->GetTextSize( m_hFont, wide, textW, textH );

		if ( player->GetTeamNumber() == TEAM_CT )
			vgui::surface()->DrawSetTextColor( 181, 211, 235, alpha );
		else
			vgui::surface()->DrawSetTextColor( 209, 189, 128, alpha );

		vgui::surface()->DrawSetTextPos( cx - textW / 2, textY );
		vgui::surface()->DrawPrintText( wide, wcslen(wide) );

		C_CSPlayer *csPlayer = static_cast<C_CSPlayer *>( player );
		DrawIcons( i - 1, csPlayer, cx, textY - ROW_GAP, alpha );

		vgui::surface()->DrawSetTextColor( 255, 255, 255, alpha );
		vgui::surface()->DrawSetTextPos( cx - arrowW / 2, textY + textH + ROW_GAP );
		vgui::surface()->DrawPrintText( arrow, wcslen(arrow) );
	}
}

void CHudHeadName::Paint()
{
	BaseClass::Paint();
	DrawPlayerNames();
}
