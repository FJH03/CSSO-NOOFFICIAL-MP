//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: HUD overhead name display for teammates
//
//=============================================================================//

#include "cbase.h"
#include "cs_hud_headname.h"
#include "iclientmode.h"
#include "clientmode_shared.h"
#include "c_baseplayer.h"
#include "c_basecombatweapon.h"
#include "c_cs_player.h"
#include "cs_gamerules.h"
#include "hud_macros.h"
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include "vgui_controls/Label.h"
#include "vgui_controls/VectorImagePanel.h"
#include "filesystem.h"
#include "utlmap.h"
#include "utlstring.h"
#include "tier0/memdbgon.h"

extern int ScreenTransform(const Vector &point, Vector &screen);

//-----------------------------------------------------------------------------
// ConVars
//-----------------------------------------------------------------------------
static ConVar cl_headname(
    "cl_headname", "1", FCVAR_ARCHIVE,
    "Show overhead name panels for teammates (0=off, 1=on)",
    true, 0, true, 1);


//-----------------------------------------------------------------------------
// Layout constants — all fixed, no dynamic scaling
//-----------------------------------------------------------------------------
static const int   HN_W             = 110;    // panel width
static const int   HN_ROW_H         = 11;     // name row height
static const int   HN_ICON_H        = 15;     // weapon icon display height (width is per-SVG aspect ratio)
static const int   HN_ICON_GAP      = 2;      // gap between weapon icons
static const int   HN_HP_H          = 10;     // HP row height
static const int   HN_ARROW_H       = 8;      // arrow label height
static const int   HN_PAD           = 2;      // inner padding
static const int   HN_ROW_GAP       = 1;      // gap between rows

// Distance — opacity
static const float HN_DIST_CLOSE    = 200.0f;
static const float HN_DIST_FAR      = 800.0f;

// Opacity values
static const int   HN_ALPHA_CLOSE   = 255;
static const int   HN_ALPHA_FAR     = 155;

// Distance — visibility limit (fix #2: hide panel beyond this distance)
static const float HN_DIST_MAX_SHOW = 1500.0f;

// Viewport edge margin — how many pixels outside the screen edge
// before we hide the panel (prevents flicker at the boundary)
static const int   HN_EDGE_MARGIN   = 20;

//-----------------------------------------------------------------------------
// SVG existence cache — avoid repeated filesystem hits each frame.
// Maps the full SVG path string to a bool (file exists or not).
// This is a process-lifetime cache; SVG assets do not change at runtime.
//-----------------------------------------------------------------------------
static bool SVGFileExists(const char *pPath)
{
    static CUtlMap<CUtlString, bool> s_svgCache(DefLessFunc(CUtlString));

    CUtlString key(pPath);
    unsigned short idx = s_svgCache.Find(key);
    if (idx != s_svgCache.InvalidIndex())
        return s_svgCache[idx];

    bool bExists = g_pFullFileSystem->FileExists(pPath, "GAME");
    s_svgCache.Insert(key, bExists);
    return bExists;
}

//-----------------------------------------------------------------------------
// Get SVG path for weapon — returns NULL if file not found.
// FileExists result is cached so the filesystem is only hit once per weapon.
//-----------------------------------------------------------------------------
static const char* GetWeaponSVGPath(const char *pWeaponClass)
{
    static char path[128];
    const char *pShort = Q_strstr(pWeaponClass, "weapon_") ? pWeaponClass + 7 : pWeaponClass;
    Q_snprintf(path, sizeof(path), "materials/vgui/weapons/svg/%s.svg", pShort);

    return SVGFileExists(path) ? path : NULL;
}

//-----------------------------------------------------------------------------
// Is this weapon a knife?
//-----------------------------------------------------------------------------
static bool IsKnifeWeapon(const char *pWeaponClass)
{
    return Q_stristr(pWeaponClass, "knife") != NULL;
}

//-----------------------------------------------------------------------------
// CPlayerNamePanel
//-----------------------------------------------------------------------------
CPlayerNamePanel::CPlayerNamePanel(vgui::Panel *pParent)
    : vgui::EditablePanel(pParent, "PlayerNamePanel")
{
    SetVisible(false);
    SetPaintBackgroundEnabled(false);
    SetPaintBorderEnabled(false);

    vgui::IScheme *pScheme = vgui::scheme()->GetIScheme(
        vgui::scheme()->GetScheme("ClientScheme"));
    m_hFont = pScheme ? pScheme->GetFont("HeadName", true) : vgui::INVALID_FONT;
    m_hFontSmall = pScheme ? pScheme->GetFont("DefaultVerySmall", true) : vgui::INVALID_FONT;
    if (m_hFont == vgui::INVALID_FONT)
        Warning("CPlayerNamePanel: Could not load 'HeadName' font\n");

    // Name label — centered
    m_pNameLabel = new vgui::Label(this, "Name", "");
    m_pNameLabel->SetFont(m_hFont);
    m_pNameLabel->SetPaintBackgroundEnabled(false);
    m_pNameLabel->SetContentAlignment(vgui::Label::a_center);

    // HP label — centered, smaller font
    m_pHPLabel = new vgui::Label(this, "HP", "");
    m_pHPLabel->SetFont(m_hFontSmall);
    m_pHPLabel->SetPaintBackgroundEnabled(false);
    m_pHPLabel->SetContentAlignment(vgui::Label::a_center);

    // Arrow label — centered, points down toward player
    m_pArrowLabel = new vgui::Label(this, "Arrow", "");
    m_pArrowLabel->SetFont(m_hFont);
    m_pArrowLabel->SetPaintBackgroundEnabled(false);
    m_pArrowLabel->SetContentAlignment(vgui::Label::a_north);
    {
        wchar_t arrow[] = { 0x25BC, 0 }; // ▼
        m_pArrowLabel->SetText(arrow);
    }

    // 6 weapon icon slots: equipment, pistol, primary, grenade x3
    for (int i = 0; i < 6; i++)
    {
        char name[32];
        Q_snprintf(name, sizeof(name), "WeaponIcon%d", i);
        m_pWeaponIcons[i] = new vgui::VectorImagePanel(this, name);
        m_pWeaponIcons[i]->SetVisible(false);
        m_pWeaponIcons[i]->SetFgColor(Color(255, 255, 255, 255));
    }

    m_hLastPrimary  = NULL;
    m_hLastPistol   = NULL;
    m_hLastC4       = NULL;
    m_bHasDefuser   = false;
    m_bHasC4        = false;
    for (int i = 0; i < 3; i++)
        m_hLastGrenades[i] = NULL;

    m_nCachedHP      = -1;
    m_nLastAlpha     = -1;
    m_szCachedName[0] = '\0';
    m_nLastIconCount = -1;
    m_nLastPanelW    = -1;

    // -2 = bone not yet looked up for this player
    m_nHeadBone      = -2;

    SetSize(HN_W, 100);
}

void CPlayerNamePanel::Reset()
{
    SetVisible(false);
    m_hLastPrimary  = NULL;
    m_hLastPistol   = NULL;
    m_hLastC4       = NULL;
    m_bHasDefuser   = false;
    m_bHasC4        = false;
    for (int i = 0; i < 3; i++)
        m_hLastGrenades[i] = NULL;

    m_nCachedHP      = -1;
    m_nLastAlpha     = -1;
    m_szCachedName[0] = '\0';
    m_nLastIconCount = -1;
    m_nLastPanelW    = -1;
    m_nHeadBone      = -2;
    m_hBoneCachePlayer = NULL;

    for (int i = 0; i < 6; i++)
    {
        if (m_pWeaponIcons[i])
            m_pWeaponIcons[i]->SetVisible(false);
    }
}

//-----------------------------------------------------------------------------
// Update — called every frame for one teammate
//-----------------------------------------------------------------------------
void CPlayerNamePanel::Update(C_CSPlayer *pPlayer, int screenX, int screenY)
{
    if (!pPlayer)
    {
        SetVisible(false);
        return;
    }

    //--------------------------------------------------------------------------
    // Distance from local player — used for opacity and visibility limit
    //--------------------------------------------------------------------------
    C_CSPlayer *pLocal = C_CSPlayer::GetLocalCSPlayer();
    float flDistance = 0.0f;

    if (pLocal)
    {
        flDistance = (pPlayer->EyePosition() - pLocal->EyePosition()).Length();
    }

    // Fix #2: hide beyond maximum display distance (no panel scaling at all)
    if (flDistance > HN_DIST_MAX_SHOW)
    {
        SetVisible(false);
        return;
    }

    //--------------------------------------------------------------------------
    // Opacity: fade as distance increases
    //--------------------------------------------------------------------------
    int iAlpha = HN_ALPHA_CLOSE;
    if (flDistance > HN_DIST_CLOSE)
    {
        float t = clamp((flDistance - HN_DIST_CLOSE) / (HN_DIST_FAR - HN_DIST_CLOSE), 0.0f, 1.0f);
        iAlpha = (int)Lerp(t, (float)HN_ALPHA_CLOSE, (float)HN_ALPHA_FAR);
    }
    iAlpha = clamp(iAlpha, HN_ALPHA_FAR, HN_ALPHA_CLOSE);

    bool bAlphaChanged = (iAlpha != m_nLastAlpha);

    //--------------------------------------------------------------------------
    // Collect weapons — skip knives; pistol collected but only displayed when
    // the player has no primary weapon (slot 0).
    // C4 is collected from the weapon scan directly (weapon_c4 in slot 4)
    // Defuser is a player flag, not a weapon entity — checked via HasDefuser()
    // Display order: equipment (defuser → C4) → grenades → primary (or pistol)
    //--------------------------------------------------------------------------
    C_BaseCombatWeapon *pPrimary    = NULL;
    C_BaseCombatWeapon *pPistol     = NULL;
    C_BaseCombatWeapon *pC4         = NULL;
    C_BaseCombatWeapon *pGrenade[3] = { NULL, NULL, NULL };
    int grenadeCount = 0;
    bool bHasDefuser = pPlayer->HasDefuser();

    for (int i = 0; i < MAX_WEAPONS; i++)
    {
        C_BaseCombatWeapon *pWep = pPlayer->GetWeapon(i);
        if (!pWep) continue;

        const char *pClass = pWep->GetClassname();

        // Skip knives
        if (IsKnifeWeapon(pClass))
            continue;

        // C4 — store separately, do not treat as grenade
        if (Q_stristr(pClass, "weapon_c4"))
        {
            pC4 = pWep;
            continue;
        }

        int slot = pWep->GetSlot();

        if (slot == 0 && !pPrimary)
            pPrimary = pWep;
        else if (slot == 1 && !pPistol)
            pPistol = pWep;  // collected but only shown if no primary
        else if ((slot == 3 || slot == 4) && grenadeCount < 3)
            pGrenade[grenadeCount++] = pWep;
    }

    // Show pistol only when the player has no primary weapon
    if (pPrimary)
        pPistol = NULL;

    //--------------------------------------------------------------------------
    // Weapon state change detection
    // Only rebuild icons (SetTexture) when weapons actually changed.
    // SetTexture() rasterizes SVG files and is very expensive per-frame.
    //--------------------------------------------------------------------------
    bool bWeaponsChanged =
        (pPrimary     != m_hLastPrimary.Get())     ||
        (pPistol      != m_hLastPistol.Get())      ||
        (pC4          != m_hLastC4.Get())          ||
        (bHasDefuser  != m_bHasDefuser)            ||
        (pGrenade[0]  != m_hLastGrenades[0].Get()) ||
        (pGrenade[1]  != m_hLastGrenades[1].Get()) ||
        (pGrenade[2]  != m_hLastGrenades[2].Get());

    int iconCount  = m_nLastIconCount;
    int panelW     = m_nLastPanelW;

    // Fallback: if layout hasn't been built yet, force a rebuild
    if (m_nLastIconCount < 0 || m_nLastPanelW < 0)
        bWeaponsChanged = true;

    if (bWeaponsChanged)
    {
        // Update cached weapon handles
        m_hLastPrimary       = pPrimary;
        m_hLastPistol        = pPistol;
        m_hLastC4            = pC4;
        m_bHasDefuser        = bHasDefuser;
        m_bHasC4             = (pC4 != NULL);
        m_hLastGrenades[0]   = pGrenade[0];
        m_hLastGrenades[1]   = pGrenade[1];
        m_hLastGrenades[2]   = pGrenade[2];

        //----------------------------------------------------------------------
        // Fill icon slots in order: defuser → C4 → grenades → primary
        //
        // How scaling works in VectorImagePanel:
        //   SetTexture() renders the SVG at its natural dimensions and internally
        //   calls SetSize(natural_w, natural_h).  Paint() then stretches that
        //   texture to whatever SetSize says the panel is.
        //
        //   Fix: after SetTexture(), read the natural size and call
        //   SetSize(scaled_w, HN_ICON_H) maintaining the aspect ratio.
        //   Every icon is exactly HN_ICON_H pixels tall, with its correct width.
        //----------------------------------------------------------------------
        iconCount = 0;
        int iconWidths[6] = {};

        auto SetIcon = [&](const char *pPath)
        {
            if (!pPath || iconCount >= 6) return;

            vgui::VectorImagePanel *pIcon = m_pWeaponIcons[iconCount];
            pIcon->SetTexture(pPath);   // renders at natural size, sets panel to natural size

            // Read natural dimensions set by SetTexture.
            // Guard against 0-size: if the SVG hasn't rasterized yet on the
            // first frame, natW/natH can be 0, which makes dispW=0 and corrupts
            // the entire icon layout until weapons change again (next round).
            // Fall back to a safe 2:1 width so the panel always has valid geometry.
            int natW, natH;
            pIcon->GetSize(natW, natH);

            int dispW;
            if (natH > 0 && natW > 0)
                dispW = (natW * HN_ICON_H) / natH;
            else
                dispW = HN_ICON_H * 2;  // safe fallback, 2:1 aspect ratio

            pIcon->SetSize(dispW, HN_ICON_H);

            iconWidths[iconCount] = dispW;
            pIcon->SetFgColor(Color(255, 255, 255, iAlpha));
            pIcon->SetVisible(true);
            iconCount++;
        };

        // Equipment first: defuser then C4
        if (bHasDefuser)
            SetIcon("materials/vgui/hud/svg/defuser.svg");
        if (pC4)
            SetIcon("materials/vgui/hud/svg/bomb_c4.svg");

        // Grenades second
        for (int i = 0; i < grenadeCount; i++)
        {
            if (pGrenade[i])
                SetIcon(GetWeaponSVGPath(pGrenade[i]->GetClassname()));
        }

        // Primary weapon last; if the player has no primary, show pistol instead
        if (pPrimary)
            SetIcon(GetWeaponSVGPath(pPrimary->GetClassname()));
        else if (pPistol)
            SetIcon(GetWeaponSVGPath(pPistol->GetClassname()));

        // Hide unused slots
        for (int i = iconCount; i < 6; i++)
            m_pWeaponIcons[i]->SetVisible(false);

        //----------------------------------------------------------------------
        // Layout — icons have variable widths (each at its natural aspect ratio)
        //----------------------------------------------------------------------
        int totalIconWidth = 0;
        for (int i = 0; i < iconCount; i++)
            totalIconWidth += iconWidths[i] + (i > 0 ? HN_ICON_GAP : 0);

        // Panel is at least HN_W wide; expands to fit icons if wider
        panelW = max(HN_W, totalIconWidth + HN_PAD * 2);

        int iconStartX = (panelW - totalIconWidth) / 2;
        int curX = iconStartX;
        for (int i = 0; i < iconCount; i++)
        {
            m_pWeaponIcons[i]->SetPos(curX, 0);
            curX += iconWidths[i] + HN_ICON_GAP;
        }

        int iconRowH = (iconCount > 0) ? HN_ICON_H + HN_ROW_GAP : 0;
        int nameY    = iconRowH;
        int hpY      = nameY + HN_ROW_H + HN_ROW_GAP + 2;
        int arrowY   = hpY + HN_HP_H + HN_ROW_GAP;
        int totalH   = arrowY + HN_ARROW_H;

        m_pNameLabel->SetBounds(HN_PAD, nameY, panelW - HN_PAD * 2, HN_ROW_H);
        m_pHPLabel->SetBounds(HN_PAD, hpY, panelW - HN_PAD * 2, HN_HP_H);
        m_pArrowLabel->SetBounds((panelW - 10) / 2, arrowY, 10, HN_ARROW_H);

        SetSize(panelW, totalH);

        // Save layout for future frames
        m_nLastIconCount = iconCount;
        m_nLastPanelW    = panelW;
    }
    else if (bAlphaChanged)
    {
        // Weapons unchanged but alpha changed — just update icon colors
        for (int i = 0; i < iconCount; i++)
            m_pWeaponIcons[i]->SetFgColor(Color(255, 255, 255, iAlpha));
    }

    //--------------------------------------------------------------------------
    // Name label — only update text/color when name or alpha changed
    //--------------------------------------------------------------------------
    const char *pName = pPlayer->GetPlayerName();
    bool bNameChanged = (Q_strcmp(pName, m_szCachedName) != 0);

    if (bNameChanged)
    {
        m_pNameLabel->SetText(pName);
        Q_strncpy(m_szCachedName, pName, sizeof(m_szCachedName));
    }

    if (bNameChanged || bAlphaChanged)
    {
        bool bIsCT = (pPlayer->GetTeamNumber() == TEAM_CT);
        Color nameColor = bIsCT
            ? Color(100, 180, 255, iAlpha)   // CT Blue
            : Color(100, 255, 100, iAlpha);  // T Green
        m_pNameLabel->SetFgColor(nameColor);
    }

    //--------------------------------------------------------------------------
    // HP label — only update when HP or alpha changed
    //--------------------------------------------------------------------------
    int hp = clamp(pPlayer->GetHealth(), 0, 100);
    bool bHPChanged = (hp != m_nCachedHP);

    if (bHPChanged)
    {
        char hpText[16];
        Q_snprintf(hpText, sizeof(hpText), "%d%%", hp);
        m_pHPLabel->SetText(hpText);
        m_nCachedHP = hp;
    }

    if (bHPChanged || bAlphaChanged)
    {
        Color hpColor = (hp <= 30)
            ? Color(255, 80, 80, iAlpha)
            : Color(255, 255, 255, iAlpha);
        m_pHPLabel->SetFgColor(hpColor);
    }

    //--------------------------------------------------------------------------
    // Arrow color — only update when alpha changed
    //--------------------------------------------------------------------------
    if (bAlphaChanged)
        m_pArrowLabel->SetFgColor(Color(255, 255, 255, iAlpha));

    m_nLastAlpha = iAlpha;

    //--------------------------------------------------------------------------
    // Position — centered above head, updated every frame
    //--------------------------------------------------------------------------
    int posX = screenX - panelW / 2;
    int posY = screenY - GetTall() - 4;

    SetPos(posX, posY);
    SetVisible(true);
}

//-----------------------------------------------------------------------------
// CHudPlayerName
//-----------------------------------------------------------------------------
DECLARE_HUDELEMENT(CHudPlayerName);

CHudPlayerName::CHudPlayerName(const char *pElementName)
    : CHudElement(pElementName), vgui::EditablePanel(NULL, "HudPlayerName")
{
    vgui::Panel *pParent = g_pClientMode->GetViewport();
    SetParent(pParent);
    SetHiddenBits(HIDEHUD_PLAYERDEAD);
    SetPaintBackgroundEnabled(false);
    SetPaintBorderEnabled(false);

    memset(m_pPanels, 0, sizeof(m_pPanels));
}

void CHudPlayerName::Init()
{
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        if (!m_pPanels[i])
        {
            m_pPanels[i] = new CPlayerNamePanel(this);
            m_pPanels[i]->SetVisible(false);
        }
    }
}

void CHudPlayerName::Reset()
{
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        if (m_pPanels[i])
            m_pPanels[i]->Reset();
    }
}

void CHudPlayerName::OnThink()
{
    // Keep this panel full-screen so children can position anywhere
    int sw, sh;
    vgui::surface()->GetScreenSize(sw, sh);
    SetBounds(0, 0, sw, sh);

    if (!cl_headname.GetBool())
    {
        for (int i = 0; i < MAX_PLAYERS; i++)
            if (m_pPanels[i]) m_pPanels[i]->SetVisible(false);
        return;
    }

    // Disable headname in free-for-all mode — teammates are enemies, nothing to show
    if (mp_teammates_are_enemies.GetBool())
    {
        for (int i = 0; i < MAX_PLAYERS; i++)
            if (m_pPanels[i]) m_pPanels[i]->SetVisible(false);
        return;
    }

    C_CSPlayer *pLocal = C_CSPlayer::GetLocalCSPlayer();
    if (!pLocal)
    {
        for (int i = 0; i < MAX_PLAYERS; i++)
            if (m_pPanels[i]) m_pPanels[i]->SetVisible(false);
        return;
    }

    int localIdx = pLocal->entindex();

    // When spectating a teammate first-person, their head bone projects to the
    // very top of the screen (behind the camera). Detect the spectated player
    // and skip drawing their headname label entirely.
    int spectatedIdx = -1;
    if (!pLocal->IsAlive())
    {
        C_BaseEntity *pTarget = pLocal->GetObserverTarget();
        if (pTarget)
            spectatedIdx = pTarget->entindex();
    }

    for (int i = 1; i <= MAX_PLAYERS; i++)
    {
        CPlayerNamePanel *pPanel = m_pPanels[i - 1];
        if (!pPanel)
            continue;

        // Hide our own panel and the panel of whoever we are spectating
        if (i == localIdx || i == spectatedIdx)
        {
            pPanel->SetVisible(false);
            continue;
        }

        C_CSPlayer *pPlayer = dynamic_cast<C_CSPlayer *>(UTIL_PlayerByIndex(i));

        // Only alive teammates
        if (!pPlayer || !pPlayer->IsAlive() ||
            pPlayer->GetTeamNumber() != pLocal->GetTeamNumber())
        {
            pPanel->SetVisible(false);
            continue;
        }

        int sx, sy;
        // Fix #1: GetHeadScreenPos returns false if off-screen — panel hidden
        if (!GetHeadScreenPos(pPlayer, sx, sy, sw, sh))
        {
            pPanel->SetVisible(false);
            continue;
        }

        pPanel->Update(pPlayer, sx, sy);
    }
}

bool CHudPlayerName::GetHeadScreenPos(C_CSPlayer *pPlayer, int &sx, int &sy, int sw, int sh)
{
    Vector worldPos;
    QAngle dummy;

    // Cache the bone index per player to avoid string lookup every frame.
    // We compare the player pointer to detect player slot reuse.
    CPlayerNamePanel *pPanel = m_pPanels[pPlayer->entindex() - 1];
    if (pPanel)
    {
        if (pPanel->m_hBoneCachePlayer.Get() != pPlayer)
        {
            // Player changed for this slot — invalidate bone cache
            pPanel->m_nHeadBone = -2;
            pPanel->m_hBoneCachePlayer = pPlayer;
        }

        if (pPanel->m_nHeadBone == -2)
        {
            // First lookup for this player
            pPanel->m_nHeadBone = pPlayer->LookupBone("head_0");
        }

        int bone = pPanel->m_nHeadBone;
        if (bone != -1)
        {
            pPlayer->GetBonePosition(bone, worldPos, dummy);
            worldPos.z += 8.0f;
        }
        else
        {
            worldPos = pPlayer->EyePosition();
            worldPos.z += 8.0f;
        }
    }
    else
    {
        // Fallback (no panel): do the lookup without caching
        int bone = pPlayer->LookupBone("head_0");
        if (bone != -1)
        {
            pPlayer->GetBonePosition(bone, worldPos, dummy);
            worldPos.z += 8.0f;
        }
        else
        {
            worldPos = pPlayer->EyePosition();
            worldPos.z += 8.0f;
        }
    }

    Vector screen;
    if (ScreenTransform(worldPos, screen) != 0)
        return false;

    sx = (int)(0.5f * (1.0f + screen.x) * sw);
    sy = (int)(0.5f * (1.0f - screen.y) * sh);

    // Hide the panel if the projected point is outside the viewport.
    // A small margin (HN_EDGE_MARGIN) prevents flicker when the head
    // sits right on the screen boundary and crosses it with micro-movement.
    if (sx < -HN_EDGE_MARGIN || sx > sw + HN_EDGE_MARGIN ||
        sy < -HN_EDGE_MARGIN || sy > sh + HN_EDGE_MARGIN)
        return false;

    return true;
}
