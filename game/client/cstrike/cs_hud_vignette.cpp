//========= Copyright Valve Corporation, All rights reserved. ============//

#include "cbase.h"
#include "hudelement.h"
#include "iclientmode.h"
#include <vgui/ISurface.h>
#include <vgui_controls/Panel.h>

// ConVars for easy in-game tuning
static ConVar csgo_vignette(
    "csgo_vignette", "1", FCVAR_ARCHIVE,
    "Enable CS:GO-style vignette overlay (dark gradient at screen edges)" );

static ConVar csgo_vignette_top_alpha(
    "csgo_vignette_top_alpha", "140", FCVAR_ARCHIVE,
    "Top vignette gradient peak alpha (0-255). Default 140." );

static ConVar csgo_vignette_top_size(
    "csgo_vignette_top_size", "0.20", FCVAR_ARCHIVE,
    "Top vignette gradient height as a fraction of screen height (0.0-1.0). Default 0.20." );

static ConVar csgo_vignette_side_alpha(
    "csgo_vignette_side_alpha", "80", FCVAR_ARCHIVE,
    "Side vignette gradient peak alpha (0-255). Default 80." );

static ConVar csgo_vignette_side_size(
    "csgo_vignette_side_size", "0.12", FCVAR_ARCHIVE,
    "Side vignette gradient width as a fraction of screen width (0.0-1.0). Default 0.12." );

static ConVar csgo_vignette_bottom_alpha(
    "csgo_vignette_bottom_alpha", "60", FCVAR_ARCHIVE,
    "Bottom vignette gradient peak alpha (0-255). Default 60." );

static ConVar csgo_vignette_bottom_size(
    "csgo_vignette_bottom_size", "0.10", FCVAR_ARCHIVE,
    "Bottom vignette gradient height as a fraction of screen height (0.0-1.0). Default 0.10." );

//-----------------------------------------------------------------------------
class CHudVignette : public CHudElement, public vgui::Panel
{
    DECLARE_CLASS_SIMPLE( CHudVignette, vgui::Panel );

public:
    CHudVignette( const char *pElementName )
        : CHudElement( pElementName ), vgui::Panel( NULL, "HudVignette" )
    {
        vgui::Panel *pParent = g_pClientMode->GetViewport();
        SetParent( pParent );

        // No background, we draw our own gradient quads
        SetPaintBackgroundEnabled( false );
        SetPaintBorderEnabled( false );
        SetProportional( false );
        SetKeyBoardInputEnabled( false );
        SetMouseInputEnabled( false );

        // Cover the full screen
        SetPos( 0, 0 );
        SetSize( ScreenWidth(), ScreenHeight() );

        // Draw ABOVE the 3D world but BEHIND the HUD
        SetZPos( -10 );
    }

    virtual bool ShouldDraw()
    {
        return csgo_vignette.GetBool();
    }

    virtual void ApplySchemeSettings( vgui::IScheme *pScheme )
    {
        BaseClass::ApplySchemeSettings( pScheme );
        SetPos( 0, 0 );
        SetSize( ScreenWidth(), ScreenHeight() );
    }

    virtual void OnScreenSizeChanged( int nOldWide, int nOldTall )
    {
        BaseClass::OnScreenSizeChanged( nOldWide, nOldTall );
        SetPos( 0, 0 );
        SetSize( ScreenWidth(), ScreenHeight() );
    }

    virtual void Paint()
    {
        if ( !csgo_vignette.GetBool() )
            return;

        int wide, tall;
        GetSize( wide, tall );

        // All gradients use black
        vgui::surface()->DrawSetColor( 0, 0, 0, 255 );

        // --- Top gradient ---
        // Fades from peak alpha at the very top → 0 at the bottom of the gradient zone
        int topAlpha = clamp( csgo_vignette_top_alpha.GetInt(), 0, 255 );
        int topH     = (int)( tall * clamp( csgo_vignette_top_size.GetFloat(), 0.0f, 1.0f ) );
        if ( topH > 0 && topAlpha > 0 )
        {
            // bHorizontal = false → vertical gradient (top→bottom)
            vgui::surface()->DrawFilledRectFade( 0, 0, wide, topH, topAlpha, 0, false );
        }

        // --- Bottom gradient ---
        // Fades from 0 at the top of the zone → peak alpha at the very bottom
        int botAlpha = clamp( csgo_vignette_bottom_alpha.GetInt(), 0, 255 );
        int botH     = (int)( tall * clamp( csgo_vignette_bottom_size.GetFloat(), 0.0f, 1.0f ) );
        if ( botH > 0 && botAlpha > 0 )
        {
            vgui::surface()->DrawFilledRectFade( 0, tall - botH, wide, tall, 0, botAlpha, false );
        }

        // --- Left gradient ---
        // Fades from peak alpha at the left edge → 0 at the right end of the zone
        int sideAlpha = clamp( csgo_vignette_side_alpha.GetInt(), 0, 255 );
        int sideW     = (int)( wide * clamp( csgo_vignette_side_size.GetFloat(), 0.0f, 1.0f ) );
        if ( sideW > 0 && sideAlpha > 0 )
        {
            // bHorizontal = true → horizontal gradient (left→right)
            vgui::surface()->DrawFilledRectFade( 0, 0, sideW, tall, sideAlpha, 0, true );

            // --- Right gradient ---
            // Fades from 0 at the left end of the zone → peak alpha at the right edge
            vgui::surface()->DrawFilledRectFade( wide - sideW, 0, wide, tall, 0, sideAlpha, true );
        }
    }
};

DECLARE_HUDELEMENT( CHudVignette );
