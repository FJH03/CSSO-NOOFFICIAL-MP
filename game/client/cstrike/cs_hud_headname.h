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

public:
    CPlayerNamePanel(vgui::Panel *pParent);

    void Update(C_CSPlayer *pPlayer, int screenX, int screenY);
    void Reset();

private:
    vgui::Label            *m_pNameLabel;
    vgui::Label            *m_pHPLabel;
    vgui::VectorImagePanel *m_pWeaponIcon;
    vgui::Label            *m_pArrowLabel;
    vgui::HFont             m_hFont;

    CHandle<C_BaseCombatWeapon> m_hLastWeapon;
    float m_flCurrentX;
    float m_flCurrentY;
    float m_targetX;
    float m_targetY;
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
    bool GetHeadScreenPos(C_CSPlayer *pPlayer, int &sx, int &sy);

    CPlayerNamePanel *m_pPanels[MAX_PLAYERS];
};
