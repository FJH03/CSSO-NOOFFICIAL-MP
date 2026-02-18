//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "c_basetempentity.h"
#include "iefx.h"
#include "fx.h"
#include "decals.h"
#include "materialsystem/imaterialsystem.h"
#include "filesystem.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/itexture.h"
#include "materialsystem/imaterialvar.h"
#include "tier0/vprof.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static ConVar cl_playerspraydisable( "cl_playerspraydisable", "0", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "Disable player sprays." );

#ifndef _XBOX
PRECACHE_REGISTER_BEGIN( GLOBAL, PrecachePlayerDecal )
PRECACHE( MATERIAL, "decals/playerlogo01" )
PRECACHE_REGISTER_END()
#endif

//-----------------------------------------------------------------------------
// Purpose: Player Decal TE
//-----------------------------------------------------------------------------
class C_TEPlayerDecal : public C_BaseTempEntity
{
public:
	DECLARE_CLASS( C_TEPlayerDecal, C_BaseTempEntity );
	DECLARE_CLIENTCLASS();

					C_TEPlayerDecal( void );
	virtual			~C_TEPlayerDecal( void );

	virtual void	PostDataUpdate( DataUpdateType_t updateType );

	virtual void	Precache( void );

public:
	int				m_nPlayer;
	Vector			m_vecOrigin;
	int				m_nEntity;
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_TEPlayerDecal::C_TEPlayerDecal( void )
{
	m_nPlayer = 0;
	m_vecOrigin.Init();
	m_nEntity = 0;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_TEPlayerDecal::~C_TEPlayerDecal( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_TEPlayerDecal::Precache( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
IMaterial *CreateTempMaterialForPlayerLogo( int iPlayerIndex, player_info_t *info, char *texname, int nchars )
{
	// Doesn't have a logo?
	if ( !info->customFiles[0] )	
		return NULL;

	IMaterial *logo = materials->FindMaterial( VarArgs("decals/playerlogo%2.2d", iPlayerIndex), TEXTURE_GROUP_DECAL );
	if ( IsErrorMaterial( logo ) )
		return NULL;

	char logohex[ 16 ];
	Q_binarytohex( (byte *)&info->customFiles[0], sizeof( info->customFiles[0] ), logohex, sizeof( logohex ) );

	// See if logo has been downloaded.
	Q_snprintf( texname, nchars, "temp/%s", logohex );
	char fulltexname[ 512 ];
	Q_snprintf( fulltexname, sizeof( fulltexname ), "materials/temp/%s.vtf", logohex );

	if ( !filesystem->FileExists( fulltexname ) )
	{
		char custname[ 512 ];
		Q_snprintf( custname, sizeof( custname ), "download/user_custom/%c%c/%s.dat", logohex[0], logohex[1], logohex );
		// it may have been downloaded but not copied under materials folder
		if ( !filesystem->FileExists( custname ) )
			return NULL; // not downloaded yet

		// copy from download folder to materials/temp folder
		// this is done since material system can access only materials/*.vtf files

		if ( !engine->CopyLocalFile( custname, fulltexname) )
			return NULL;
	}

	return logo;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : filter - 
//			delay - 
//			pos - 
//			player - 
//			entity - 
//-----------------------------------------------------------------------------
void TE_PlayerDecal( IRecipientFilter& filter, float delay,
	const Vector* pos, int player, int entity  )
{
	if ( cl_playerspraydisable.GetBool() )
		return;

	// No valid target?
	C_BaseEntity *ent = cl_entitylist->GetEnt( entity );
	if ( !ent )
		return;

	// Find player logo for shooter
	player_info_t info;
	engine->GetPlayerInfo( player, &info );

	// Make sure we've got the material for this player's logo
	char texname[ 512 ];
	IMaterial *logo = CreateTempMaterialForPlayerLogo( player, &info, texname, 512 );
	if ( !logo )
		return;

	ITexture *texture = materials->FindTexture( texname, TEXTURE_GROUP_DECAL );
	if ( IsErrorTexture( texture ) ) 
	{
		return; // not found 
	}

	// Update the texture used by the material if need be.
	bool bFound = false;
	IMaterialVar *pMatVar = logo->FindVar( "$basetexture", &bFound );
	if ( bFound && pMatVar )
	{
		if ( pMatVar->GetTextureValue() != texture )
		{
			pMatVar->SetTextureValue( texture );
			logo->RefreshPreservingMaterialVars();
		}
	}

	color32 rgbaColor = { 255, 255, 255, 255 };
	effects->PlayerDecalShoot( 
		logo, 
		(void *)(intp)player,
		entity, 
		ent->GetModel(), 
		ent->GetAbsOrigin(), 
		ent->GetAbsAngles(), 
		*pos, 
		0, 
		0,
		rgbaColor );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : bool - 
//-----------------------------------------------------------------------------
void C_TEPlayerDecal::PostDataUpdate( DataUpdateType_t updateType )
{
#ifndef _XBOX
	VPROF( "C_TEPlayerDecal::PostDataUpdate" );

	// Decals disabled?
	if ( !r_decals.GetBool() )
		return;

	CLocalPlayerFilter filter;
	TE_PlayerDecal(  filter, 0.0f, &m_vecOrigin, m_nPlayer, m_nEntity );
#endif
}

IMPLEMENT_CLIENTCLASS_EVENT_DT(C_TEPlayerDecal, DT_TEPlayerDecal, CTEPlayerDecal)
	RecvPropVector( RECVINFO(m_vecOrigin)),
	RecvPropInt( RECVINFO(m_nEntity)),
	RecvPropInt( RECVINFO(m_nPlayer)),
END_RECV_TABLE()
