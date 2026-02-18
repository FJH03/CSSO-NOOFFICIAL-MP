//===== Copyright � 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose: 
//
// $NoKeywords: $
//===========================================================================//
#include "cbase.h"
#include "precache_register.h"
#include "tier0/platform.h"
#include "tier1/KeyValues.h"
#include "tier2/tier2.h"
#include "filesystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static const char *s_pPrecacheResourceTypeName[] =
{
	"vgui_resource",
	"material",		
	"model",			
	"scriptsound",		// NOTE: This is named this way for backward compat in reading from kv files	
	"particle_system",	
	"entity",
	"decal",
	"pmaterial",
	"dependency_file",
	"game_material_decals",
	"shared",
};

//-----------------------------------------------------------------------------
// Provides callback to do actual precaching of resources
//-----------------------------------------------------------------------------
class CPrecacheHandler : public IPrecacheHandler
{
public:
	void CacheResource( PrecacheResourceType_t nType, const char *pName, bool bPrecache, int *pIndex = NULL );

private:
	void CacheResourceFile( const char *pFilename, bool bPrecache );
	void PrecacheGameMaterialDecals( bool bPrecache );
};

//-----------------------------------------------------------------------------
// Singletons
//-----------------------------------------------------------------------------
static CPrecacheRegister	s_PrecacheRegister;
CPrecacheRegister *g_pPrecacheRegister = &s_PrecacheRegister;

static CPrecacheHandler		s_PrecacheHandler;
IPrecacheHandler *g_pPrecacheHandler = &s_PrecacheHandler;

bool CPrecacheRegister::Init()
{ 
	return true; 
}

//-----------------------------------------------------------------------------
// Level startup, shutdown
//-----------------------------------------------------------------------------
void CPrecacheRegister::LevelInitPreEntity()
{
	COM_TimestampedLog( "LevelInitPreEntity - PreCache - Start" );
	g_pPrecacheSystem->Cache( g_pPrecacheHandler, GLOBAL, NULL, true );
	COM_TimestampedLog( "LevelInitPreEntity - PreCache - Finish" );

#ifdef CLIENT_DLL
	//FIXME: Double check this
	//Finally, force the cache of these materials
	COM_TimestampedLog( "LevelInitPreEntity - CacheUsedMaterials - Start" );
	materials->CacheUsedMaterials();
	COM_TimestampedLog( "LevelInitPreEntity - CacheUsedMaterials - Finish" );
#endif
}

void CPrecacheRegister::LevelShutdownPostEntity()
{
	// FIXME: How to uncache all resources cached during the course of the level?
	g_pPrecacheSystem->UncacheAll( g_pPrecacheHandler );
}

//-----------------------------------------------------------------------------
// Purpose: Precache game-specific models & sounds
//-----------------------------------------------------------------------------
void CPrecacheHandler::CacheResourceFile( const char *pFilename, bool bPrecache )
{
	COMPILE_TIME_ASSERT( ARRAYSIZE(s_pPrecacheResourceTypeName) == PRECACHE_RESOURCE_TYPE_COUNT );

	KeyValues *pValues = new KeyValues( "ResourceFile" );

	if ( !pValues->LoadFromFile( g_pFullFileSystem, pFilename, "GAME" ) )
	{
		Warning( "Can't open %s for client precache info.", pFilename );
		pValues->deleteThis();
		return;
	}

	for ( KeyValues *pData = pValues->GetFirstSubKey(); pData != NULL; pData = pData->GetNextKey() )
	{
		const char *pszType = pData->GetName();
		const char *pszFile = pData->GetString();

		if ( Q_strlen( pszType ) == 0 || Q_strlen( pszFile ) == 0 )
			continue;

		bool bFoundMatch = false;
		for ( int i = 0; i < PRECACHE_RESOURCE_TYPE_COUNT; ++i )
		{
			if ( !Q_stricmp( pData->GetName(), s_pPrecacheResourceTypeName[i] ) )
			{
				CacheResource( (PrecacheResourceType_t)i, pszFile, bPrecache );
				bFoundMatch = true;
				break;
			}
		}

		if ( !bFoundMatch )
		{
			Warning( "Error in precache file \"%s\":\n", pFilename );
			Warning( "\tUnknown resource type specified \"%s\", value \"%s\"\n", pszType, pszFile );
		}
	}

	pValues->deleteThis();
}


//-----------------------------------------------------------------------------
// Precaches game material decals
//-----------------------------------------------------------------------------
void CPrecacheHandler::PrecacheGameMaterialDecals( bool bPrecache )
{
}


//-----------------------------------------------------------------------------
// Caches/uncaches resources
//-----------------------------------------------------------------------------
void CPrecacheHandler::CacheResource( PrecacheResourceType_t nType, const char *pName, 
	bool bPrecache, int *pIndex )
{
	if ( bPrecache )
	{
		if ( pIndex )
		{
			*pIndex = 0;
		}

		switch( nType )
		{
		case VGUI_RESOURCE:
			break;

		case MATERIAL:
			PrecacheMaterial( pName ); 
			if ( pIndex )
			{
				*pIndex = GetMaterialIndex( pName );
			}
			break;

		case PARTICLE_MATERIAL:
			{
#ifdef CLIENT_DLL
				void* nIndex = ParticleMgr()->GetPMaterial( pName );
#else
				void* nIndex = 0;
#endif
				if ( pIndex )
				{
					void** pIndexMaterial = (void**)pIndex;

					*pIndexMaterial = nIndex;
				}
			}
			break;

		case GAME_MATERIAL_DECALS:
			PrecacheGameMaterialDecals( bPrecache );
			break;

		case DECAL:
			{
				int nIndex = UTIL_PrecacheDecal( pName, true );
				if ( pIndex )
				{
					*pIndex = nIndex;
				}
			}
			break;

		case MODEL:
			{
				int nIndex = CBaseEntity::PrecacheModel( pName );
				if ( pIndex )
				{
					*pIndex = nIndex;
				}
			}
			break;

		case GAMESOUND:
			{
				int nIndex = CBaseEntity::PrecacheScriptSound( pName );
				if ( pIndex )
				{
					*pIndex = nIndex;
				}
			}
			break;

		case PARTICLE_SYSTEM:
			PrecacheParticleSystem( pName );
			if ( pIndex )
			{
				*pIndex = GetParticleSystemIndex( pName );
			}
			break;

		case ENTITY:
			UTIL_PrecacheOther( pName );
			break;

		case SHARED:
			g_pPrecacheSystem->Cache( this, SHARED_SYSTEM, pName, bPrecache );
			break;

		case KV_DEP_FILE:
			CacheResourceFile( pName, bPrecache );
			break;
		}
		return;
	}

	// Blat out value
	if ( pIndex )
	{
		*pIndex = 0;
	}

	switch( nType )
	{
	case VGUI_RESOURCE:
		break;
	case MATERIAL:
		break;
	case MODEL:
		break;
	case GAMESOUND:
		break;
	case PARTICLE_SYSTEM:
		break;
	case ENTITY:
		break;
	case DECAL:
		break;
	case KV_DEP_FILE:
		break;
	}
}