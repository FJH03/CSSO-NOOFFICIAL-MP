//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//
#if !defined( VGUICENTERPRINT_H )
#define VGUICENTERPRINT_H
#ifdef _WIN32
#pragma once
#endif

#include "ivguicenterprint.h"
#include <vgui/VGUI.h>

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
namespace vgui
{
class Panel;
}

class CNotificationPanel;
class CCenterPrint : public ICenterPrint
{
public:
	virtual void		Print( char *text );
	virtual void		Print( wchar_t *text );
	virtual void		HintPrint( char *text );
	virtual void		HintPrint( wchar_t *text );
	virtual void		Clear( void );

};

extern CCenterPrint *internalCenterPrint;
extern CNotificationPanel vguiNotificationPanel;

#endif // VGUICENTERPRINT_H