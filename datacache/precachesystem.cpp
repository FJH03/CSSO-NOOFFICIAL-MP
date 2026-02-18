//===== Copyright � 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose:
//
//===========================================================================//

#include "tier1/utlsymbol.h"
#include "tier1/UtlStringMap.h"
#include "tier2/tier2.h"
#include "datacache/iprecachesystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Implementation class
//-----------------------------------------------------------------------------
class CPrecacheSystem : public CTier2AppSystem< IPrecacheSystem >
{
	typedef CTier2AppSystem< IPrecacheSystem > BaseClass;

	// Inherited from IAppSystem
public:

	// Inherited from IResourceAccessControl
public:
	void Register( IResourcePrecacher *pResourcePrecacherFirst, PrecacheSystem_t nSystem );

	// Precaches/uncaches all resources used by a particular system
	void Cache( IPrecacheHandler *pPrecacheHandler, PrecacheSystem_t nSystem, const char *pName, bool bPrecache );

	void UncacheAll( IPrecacheHandler *pPrecacheHandler );

private:
	IResourcePrecacher *m_pFirstPrecacher[PRECACHE_SYSTEM_COUNT];
};

//-----------------------------------------------------------------------------
// String names corresponding to resource types
//-----------------------------------------------------------------------------
static const char *s_pResourceSystemName[] =
{
	"global client resource",
	"global server resource",
	"vgui panel",		
	"dispatch effect",	
	"shared system",
};

//-----------------------------------------------------------------------------
// Singleton
//-----------------------------------------------------------------------------
CPrecacheSystem g_PrecacheSystem;

EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CPrecacheSystem, IPrecacheSystem, 
	PRECACHE_SYSTEM_INTERFACE_VERSION, g_PrecacheSystem );

//-----------------------------------------------------------------------------
// Precaches/uncaches all resources used by a particular system
//-----------------------------------------------------------------------------
void CPrecacheSystem::Cache( IPrecacheHandler *pPrecacheHandler, PrecacheSystem_t nSystem, const char *pName, bool bPrecache )
{	
	COMPILE_TIME_ASSERT( ARRAYSIZE( s_pResourceSystemName ) == PRECACHE_SYSTEM_COUNT );

	IResourcePrecacher *pPrecacher = m_pFirstPrecacher[nSystem];
	for( ; pPrecacher; pPrecacher = pPrecacher->GetNext() )
	{
		if ( pName && Q_stricmp( pName, pPrecacher->GetName() ) )
			continue;

		pPrecacher->Cache( pPrecacheHandler, bPrecache, false );
	}
}

//-----------------------------------------------------------------------------
// Uncaches everything
//-----------------------------------------------------------------------------
void CPrecacheSystem::UncacheAll( IPrecacheHandler *pPrecacheHandler )
{
	int nSystem;
	for ( nSystem = 0; nSystem < PRECACHE_SYSTEM_COUNT; nSystem ++ )
	{
		IResourcePrecacher *pPrecacher = m_pFirstPrecacher[nSystem];
		for( ; pPrecacher; pPrecacher = pPrecacher->GetNext() )
		{
			pPrecacher->Cache( pPrecacheHandler, false, false );
		}
	}
}

//-----------------------------------------------------------------------------
// Called to register a list of resource precachers for a given system
//-----------------------------------------------------------------------------
void CPrecacheSystem::Register( IResourcePrecacher *pResourcePrecacherFirst, PrecacheSystem_t nSystem )
{
	// do we already have any precachers registered for this system?
	IResourcePrecacher *pCur = m_pFirstPrecacher[nSystem];

	if ( pCur )
	{
		while ( pCur->GetNext() != NULL )
		{
			pCur = pCur->GetNext();
		}
		// add the head of the new list to the tail of the existing list
		pCur->SetNext( pResourcePrecacherFirst );
	}
	else
	{
		m_pFirstPrecacher[nSystem] = pResourcePrecacherFirst;
	}	
}
