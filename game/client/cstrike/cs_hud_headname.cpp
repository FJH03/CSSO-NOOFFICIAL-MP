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
#include "tier0/memdbgon.h"

extern int ScreenTransform(const Vector &point, Vector &screen);

//-----------------------------------------------------------------------------
// ConVars
//-----------------------------------------------------------------------------
static ConVar cl_teamid_overhead(
    "cl_teamid_overhead", "2", FCVAR_ARCHIVE,
    "Overhead teammate HUD: 0=off, 1=name+money(freeze only), 2=full always",
    true, 0, true, 2);

static ConVar cl_hud_headname_lerp(
    "cl_hud_headname_lerp", "0.25", FCVAR_ARCHIVE,
    "Smoothing for overhead name panel movement (0=instant, 1=very smooth)",
    true, 0.0f, true, 1.0f);

//-----------------------------------------------------------------------------
// Layout constants
//-----------------------------------------------------------------------------
static const int HN_W       = 110;   // panel width
static const int HN_ROW_H   = 11;    // name/hp row height
static const int HN_ICON_H  = 9;     // weapon icon row height
static const int HN_ICON_W  = 54;    // weapon icon width
static const int HN_ARROW_H = 8;     // arrow label height
static const int HN_PAD     = 2;     // inner padding

// Total content height (without arrow — arrow is outside the bg box)
static const int HN_CONTENT_H = HN_PAD + HN_ROW_H + 1 + HN_ICON_H + HN_PAD;

//-----------------------------------------------------------------------------
// CPlayerNamePanel
//-----------------------------------------------------------------------------
CPlayerNamePanel::CPlayerNamePanel(vgui::Panel *pParent)
    : vgui::EditablePanel(pParent, "PlayerNamePanel")
{
    SetVisible(false);
    SetPaintBackgroundEnabled(true);
    SetPaintBorderEnabled(false);

    vgui::IScheme *pScheme = vgui::scheme()->GetIScheme(
        vgui::scheme()->GetScheme("ClientScheme"));
    m_hFont = pScheme ? pScheme->GetFont("HeadName", true) : vgui::INVALID_FONT;
    if (m_hFont == vgui::INVALID_FONT)
        Warning("CPlayerNamePanel: Could not load 'HeadName' font\n");

    // Name label — left side of first row
    m_pNameLabel = new vgui::Label(this, "Name", "");
    m_pNameLabel->SetFont(m_hFont);
    m_pNameLabel->SetPaintBackgroundEnabled(false);
    m_pNameLabel->SetContentAlignment(vgui::Label::a_west);

    // HP / money label — right side of first row
    m_pHPLabel = new vgui::Label(this, "HP", "");
    m_pHPLabel->SetFont(m_hFont);
    m_pHPLabel->SetPaintBackgroundEnabled(false);
    m_pHPLabel->SetContentAlignment(vgui::Label::a_east);

    // Weapon icon — second row, centered
    m_pWeaponIcon = new vgui::VectorImagePanel(this, "Weapon");
    m_pWeaponIcon->SetVisible(false);
    m_pWeaponIcon->SetFgColor(Color(255, 255, 255, 200));

    // Down-arrow label — sits just below the background box
    m_pArrowLabel = new vgui::Label(this, "Arrow", "");
    m_pArrowLabel->SetFont(m_hFont);
    m_pArrowLabel->SetFgColor(Color(255, 255, 255, 200));
    m_pArrowLabel->SetPaintBackgroundEnabled(false);
    m_pArrowLabel->SetContentAlignment(vgui::Label::a_north);
    {
        wchar_t arrow[] = { 0x25BC, 0 }; // ▼
        m_pArrowLabel->SetText(arrow);
    }

    m_hLastWeapon = NULL;
    m_flCurrentX  = 0.0f;
    m_flCurrentY  = 0.0f;
    m_targetX     = 0.0f;
    m_targetY     = 0.0f;

    SetSize(HN_W, HN_CONTENT_H + HN_ARROW_H);
    SetBgColor(Color(0, 0, 0, 0));
}

void CPlayerNamePanel::Reset()
{
    SetVisible(false);
    m_hLastWeapon = NULL;
    m_flCurrentX  = 0.0f;
    m_flCurrentY  = 0.0f;
    m_targetX     = 0.0f;
    m_targetY     = 0.0f;
    if (m_pWeaponIcon)
        m_pWeaponIcon->SetVisible(false);
}

void CPlayerNamePanel::Update(C_CSPlayer *pPlayer, int screenX, int screenY)
{
    if (!pPlayer)
    {
        SetVisible(false);
        return;
    }

    // ---- Name ---------------------------------------------------------------
    const char *pName = pPlayer->GetPlayerName();
    m_pNameLabel->SetText(pName);

    bool bIsCT = (pPlayer->GetTeamNumber() == TEAM_CT);
    m_pNameLabel->SetFgColor(bIsCT
        ? Color(160, 200, 255, 255)
        : Color(255, 215, 120, 255));

    // Name occupies left ~60% of the row
    static const int HP_W = 36;
    m_pNameLabel->SetBounds(HN_PAD, HN_PAD, HN_W - HP_W - HN_PAD * 3, HN_ROW_H);

    // ---- HP / Money ---------------------------------------------------------
    bool bFreeze = CSGameRules() && CSGameRules()->IsFreezePeriod();
    char statText[16];
    if (bFreeze)
    {
        Q_snprintf(statText, sizeof(statText), "$%d", pPlayer->GetAccount());
        m_pHPLabel->SetFgColor(Color(100, 255, 100, 255));
    }
    else
    {
        int hp = clamp(pPlayer->GetHealth(), 0, 100);
        Q_snprintf(statText, sizeof(statText), "%d%%", hp);
        m_pHPLabel->SetFgColor((hp <= 30)
            ? Color(255, 80, 80, 255)
            : Color(220, 220, 220, 255));
    }
    m_pHPLabel->SetText(statText);
    m_pHPLabel->SetBounds(HN_W - HP_W - HN_PAD, HN_PAD, HP_W, HN_ROW_H);

    // ---- Weapon icon --------------------------------------------------------
    C_BaseCombatWeapon *pWeapon = pPlayer->GetActiveWeapon();
    if (pWeapon != m_hLastWeapon)
    {
        m_hLastWeapon = pWeapon;
        if (pWeapon)
        {
            const char *pClass    = pWeapon->GetClassname();
            const char *pShort    = Q_strstr(pClass, "weapon_") ? pClass + 7 : pClass;
            char        svgPath[128];
            Q_snprintf(svgPath, sizeof(svgPath),
                "materials/vgui/weapons/svg/%s.svg", pShort);

            if (g_pFullFileSystem->FileExists(svgPath, "GAME"))
            {
                m_pWeaponIcon->SetRenderSize(HN_ICON_W, HN_ICON_H);
                m_pWeaponIcon->SetSize(HN_ICON_W, HN_ICON_H);
                m_pWeaponIcon->SetPos((HN_W - HN_ICON_W) / 2,
                    HN_PAD + HN_ROW_H + 1);
                m_pWeaponIcon->SetTexture(svgPath);
                m_pWeaponIcon->SetVisible(true);
            }
            else
            {
                m_pWeaponIcon->SetVisible(false);
            }
        }
        else
        {
            m_pWeaponIcon->SetVisible(false);
        }
    }

    // If weapon is gone but cache hasn't caught up yet, hide
    if (!pWeapon)
        m_pWeaponIcon->SetVisible(false);

    // ---- Arrow (outside the bg box, points down toward the player) ----------
    m_pArrowLabel->SetBounds((HN_W - 10) / 2, HN_CONTENT_H, 10, HN_ARROW_H);

    // ---- Background box covers only the content rows, not the arrow ---------
    SetBgColor(Color(0, 0, 0, 140));
    SetPaintBackgroundType(0);
    SetSize(HN_W, HN_CONTENT_H + HN_ARROW_H);

    // ---- Smooth-lerp position above the head --------------------------------
    float targetX = (float)(screenX - HN_W / 2);
    float targetY = (float)(screenY - HN_CONTENT_H - HN_ARROW_H - 4);

    if (!IsVisible())
    {
        m_flCurrentX = targetX;
        m_flCurrentY = targetY;
    }
    else
    {
        float f = cl_hud_headname_lerp.GetFloat();
        if ((m_targetX - m_flCurrentX) * (m_targetX - m_flCurrentX) +
            (m_targetY - m_flCurrentY) * (m_targetY - m_flCurrentY) > 60.0f * 60.0f)
        {
            f = MIN(1.0f, f * 2.5f); // speed up on large jumps
        }
        m_flCurrentX = Lerp(f, m_flCurrentX, targetX);
        m_flCurrentY = Lerp(f, m_flCurrentY, targetY);
    }
    m_targetX = targetX;
    m_targetY = targetY;

    int sw, st;
    vgui::surface()->GetScreenSize(sw, st);
    m_flCurrentX = clamp(m_flCurrentX, 0.0f, (float)(sw - HN_W));
    m_flCurrentY = clamp(m_flCurrentY, 0.0f, (float)(st - HN_CONTENT_H));

    SetPos((int)m_flCurrentX, (int)m_flCurrentY);
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
    int sw, st;
    vgui::surface()->GetScreenSize(sw, st);
    SetBounds(0, 0, sw, st);

    // If disabled, hide everything
    if (cl_teamid_overhead.GetInt() == 0)
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

    for (int i = 1; i <= MAX_PLAYERS; i++)
    {
        CPlayerNamePanel *pPanel = m_pPanels[i - 1];
        if (!pPanel)
            continue;

        // Skip self
        if (i == localIdx)
        {
            pPanel->SetVisible(false);
            continue;
        }

        C_CSPlayer *pPlayer = dynamic_cast<C_CSPlayer *>(UTIL_PlayerByIndex(i));

        // Only show alive teammates
        if (!pPlayer || !pPlayer->IsAlive() ||
            pPlayer->GetTeamNumber() != pLocal->GetTeamNumber())
        {
            pPanel->SetVisible(false);
            continue;
        }

        int sx, sy;
        if (!GetHeadScreenPos(pPlayer, sx, sy))
        {
            pPanel->SetVisible(false);
            continue;
        }

        pPanel->Update(pPlayer, sx, sy);
    }
}

bool CHudPlayerName::GetHeadScreenPos(C_CSPlayer *pPlayer, int &sx, int &sy)
{
    Vector worldPos;
    QAngle dummy;

    int bone = pPlayer->LookupBone("head_0");
    if (bone != -1)
    {
        pPlayer->GetBonePosition(bone, worldPos, dummy);
        worldPos.z += 8.0f;
    }
    else
    {
        // Fallback: above eye position
        worldPos = pPlayer->EyePosition();
        worldPos.z += 8.0f;
    }

    Vector screen;
    if (ScreenTransform(worldPos, screen) != 0)
        return false;

    int sw, st;
    vgui::surface()->GetScreenSize(sw, st);
    sx = (int)(0.5f * (1.0f + screen.x) * sw);
    sy = (int)(0.5f * (1.0f - screen.y) * st);
    return true;
}
