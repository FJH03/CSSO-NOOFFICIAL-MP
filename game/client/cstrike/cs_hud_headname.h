#pragma once

#include "hudelement.h"
#include "vgui_controls/EditablePanel.h"
#include "vgui_controls/Label.h"
#include "vgui_controls/VectorImagePanel.h"
#include "utlvector.h"
#include <vgui/IScheme.h>
#include "c_cs_player.h"

//-----------------------------------------------------------------------------
// CPlayerNamePanel
// One panel per player slot, drawn above the player's head in world-space.
//-----------------------------------------------------------------------------
class CPlayerNamePanel : public vgui::EditablePanel
{
    DECLARE_CLASS_SIMPLE(CPlayerNamePanel, vgui::EditablePanel);
    friend class CHudPlayerName;

public:
    CPlayerNamePanel(vgui::Panel *pParent);

    void Update(C_CSPlayer *pPlayer, int screenX, int screenY);
    void Reset();

private:
    vgui::Label            *m_pNameLabel;
    vgui::Label            *m_pHPLabel;
    vgui::VectorImagePanel *m_pWeaponIcons[6];
    vgui::Label            *m_pArrowLabel;
    vgui::HFont             m_hFont;
    vgui::HFont             m_hFontSmall;

    // --- Weapon state cache ---
    // Compare each frame; only call SetTexture when something actually changed.
    CHandle<C_BaseCombatWeapon> m_hLastPrimary;
    CHandle<C_BaseCombatWeapon> m_hLastPistol;
    CHandle<C_BaseCombatWeapon> m_hLastC4;
    CHandle<C_BaseCombatWeapon> m_hLastGrenades[3];
    bool                        m_bHasDefuser;
    bool                        m_bHasC4;

    // --- Label value cache ---
    // Avoid SetText / SetFgColor calls when value hasn't changed.
    int                         m_nCachedHP;
    int                         m_nLastAlpha;
    char                        m_szCachedName[128];

    // --- Layout cache ---
    // Avoid recomputing SetBounds / SetSize when icon count and width didn't change.
    int                         m_nLastIconCount;
    int                         m_nLastPanelW;

    // --- Bone index cache ---
    // LookupBone("head_0") is a string search; cache the result per player.
    // -2 = not yet looked up, -1 = not found, >= 0 = valid index.
    int                         m_nHeadBone;
    CHandle<C_CSPlayer>         m_hBoneCachePlayer;
};

//-----------------------------------------------------------------------------
// CHudPlayerName
// HUD element that owns all per-player name panels and updates them each frame.
//-----------------------------------------------------------------------------
class CHudPlayerName : public CHudElement, public vgui::EditablePanel
{
    DECLARE_CLASS_SIMPLE(CHudPlayerName, vgui::EditablePanel);

public:
    CHudPlayerName(const char *pElementName);

    virtual void Init();
    virtual void Reset();
    virtual void OnThink();

private:
    bool GetHeadScreenPos(C_CSPlayer *pPlayer, int &sx, int &sy, int sw, int sh);

    CPlayerNamePanel *m_pPanels[MAX_PLAYERS];
};