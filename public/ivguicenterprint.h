//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================//
#if !defined( IVGUICENTERPRINT_H )
#define IVGUICENTERPRINT_H
#ifdef _WIN32
#pragma once
#endif

#include "interface.h"

//-----------------------------------------------------------------------------
// Purpose: Engine Center Print Interface
//-----------------------------------------------------------------------------
abstract_class ICenterPrint
{
public:
	virtual void		Print( char *text ) = 0;
	virtual void		Print( wchar_t *text ) = 0;
	virtual void		Clear( void ) = 0;
};

extern ICenterPrint *centerprint;

#define VCENTERPRINT_INTERFACE_VERSION "VCENTERPRINT002"

#endif // IVGUICENTERPRINT_H