//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "cstrikerotatingplayerpanel.h"
#include "inputsystem/iinputsystem.h"
#include <vgui/IInput.h>

using namespace vgui;

CCSRotatingPlayerImagePanel::CCSRotatingPlayerImagePanel( vgui::Panel *pParent, const char *pName )
	: BaseClass( pParent, pName )
{
	m_flModelYaw = 0.0f;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CCSRotatingPlayerImagePanel::OnMousePressed( vgui::MouseCode code )
{
	if ( code != MOUSE_LEFT )
		return;

	RequestFocus();

	EnableMouseCapture( true, code );

	// Save where they clicked
	input()->GetCursorPosition( m_nClickStartX, m_nClickStartY );

	// Warp the mouse to the center of the screen
	int width, height;
	GetSize( width, height );
	int x = width / 2;
	int y = height / 2;

	int xpos = x;
	int ypos = y;
	LocalToScreen( xpos, ypos );
	input()->SetCursorPos( xpos, ypos );

	m_nManipStartX = xpos;
	m_nManipStartY = ypos;

	m_bMousePressed = true;
}

void CCSRotatingPlayerImagePanel::OnMouseReleased( vgui::MouseCode code )
{
	if ( code != MOUSE_LEFT )
		return;

	EnableMouseCapture( false );
	m_bMousePressed = false;

	// Restore the cursor to where the clicked
	input()->SetCursorPos( m_nClickStartX, m_nClickStartY );
}

void CCSRotatingPlayerImagePanel::OnCursorMoved( int x, int y )
{
	if ( m_bMousePressed )
	{
		WarpMouse( x, y );
		int xpos, ypos;
		input()->GetCursorPos( xpos, ypos );

		// Only want the x delta.
		float flDelta = xpos - m_nManipStartX;

		// Apply the delta and rotate the player.
		RotateYaw( flDelta );
	}
}

void CCSRotatingPlayerImagePanel::RotateYaw( float flDelta )
{
	m_flModelYaw += flDelta;
	if ( m_flModelYaw > 360.0f )
	{
		m_flModelYaw = m_flModelYaw - 360.0f;
	}
	else if ( m_flModelYaw < -360.0f )
	{
		m_flModelYaw = m_flModelYaw + 360.0f;
	}
}

void CCSRotatingPlayerImagePanel::EnableMouseCapture( bool enable, vgui::MouseCode mouseCode /* = -1 */ )
{
	if ( enable )
	{
		m_nCaptureMouseCode = mouseCode;
		SetCursor( vgui::dc_none );
		input()->SetMouseCaptureEx( GetVPanel(), m_nCaptureMouseCode );
	}
	else
	{
		m_nCaptureMouseCode = vgui::MouseCode( -1 );
		input()->SetMouseCapture( (VPANEL)0 );
		SetCursor( vgui::dc_arrow );
	}
}

bool CCSRotatingPlayerImagePanel::WarpMouse( int &x, int &y )
{
	// Re-force capture if it was lost...
	if ( input()->GetMouseCapture() != GetVPanel() )
	{
		input()->GetCursorPos( m_nManipStartX, m_nManipStartY );
		EnableMouseCapture( true, m_nCaptureMouseCode );
	}

	int width, height;
	GetSize( width, height );

	int centerx = width / 2;
	int centery = height / 2;

	// skip this event
	if ( x == centerx && y == centery )
		return false; 

	int xpos = centerx;
	int ypos = centery;
	LocalToScreen( xpos, ypos );

#if defined( DX_TO_GL_ABSTRACTION )
	//
	// Really reset the cursor to the center for the PotteryWheel Control
	//
	// In TF2's edit loadout dialog there is a character model that you can rotate
	// around using the mouse.  This control resets the cursor to the center of the window
	// after each mouse move.  Except the input()->SetCursorPos results (after a lot of redirection) to
	// vgui/matsurface/Cursor.cpp function CursorSetPos but it has a (needed) test to not move the 
	// cursor if it's currently hidden. Rather than change all the levels between here and there
	// to support a flag, we are just jumping to the chase and directly calling the inputsystem
	// SetCursorPosition on OpenGL platforms
	//
	g_pInputSystem->SetCursorPosition( xpos, ypos );
#else
	input()->SetCursorPos( xpos, ypos );
#endif

	int dx = x - centerx;
	int dy = y - centery;

	x += m_xoffset;
	y += m_yoffset;

	m_xoffset += dx;
	m_yoffset += dy;

	return true;
}
