//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: CS's custom C_PlayerResource
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "c_cs_playerresource.h"
#include <shareddefs.h>
#include <cs_shareddefs.h>
#include "hud.h"
#include <vgui_controls/Controls.h>
#include "vgui/ILocalize.h"
#include <vgui/IScheme.h>
#include "gamestringpool.h"
#include "c_cs_player.h"
#include "tier3/tier3.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_CLIENTCLASS_DT(C_CS_PlayerResource, DT_CSPlayerResource, CCSPlayerResource)
	RecvPropInt( RECVINFO( m_iPlayerC4 ) ),
	RecvPropVector( RECVINFO(m_vecC4) ),
	RecvPropArray3( RECVINFO_ARRAY(m_bHostageAlive), RecvPropInt( RECVINFO(m_bHostageAlive[0]))),
	RecvPropArray3( RECVINFO_ARRAY(m_isHostageFollowingSomeone), RecvPropInt( RECVINFO(m_isHostageFollowingSomeone[0]))),
	RecvPropArray3( RECVINFO_ARRAY(m_iHostageEntityIDs), RecvPropInt( RECVINFO(m_iHostageEntityIDs[0]))),
	RecvPropArray3( RECVINFO_ARRAY(m_iHostageX), RecvPropInt( RECVINFO(m_iHostageX[0]))),
	RecvPropArray3( RECVINFO_ARRAY(m_iHostageY), RecvPropInt( RECVINFO(m_iHostageY[0]))),
	RecvPropArray3( RECVINFO_ARRAY(m_iHostageZ), RecvPropInt( RECVINFO(m_iHostageZ[0]))),
	RecvPropVector( RECVINFO(m_bombsiteCenterA) ),
	RecvPropVector( RECVINFO(m_bombsiteCenterB) ),
	RecvPropArray3( RECVINFO_ARRAY(m_hostageRescueX), RecvPropInt( RECVINFO(m_hostageRescueX[0]))),
	RecvPropArray3( RECVINFO_ARRAY(m_hostageRescueY), RecvPropInt( RECVINFO(m_hostageRescueY[0]))),
	RecvPropArray3( RECVINFO_ARRAY(m_hostageRescueZ), RecvPropInt( RECVINFO(m_hostageRescueZ[0]))),
	RecvPropInt( RECVINFO( m_bBombSpotted ) ),
	RecvPropArray3( RECVINFO_ARRAY(m_bPlayerSpotted), RecvPropInt( RECVINFO(m_bPlayerSpotted[0]))),
	RecvPropArray3( RECVINFO_ARRAY(m_bHostageSpotted), RecvPropInt( RECVINFO(m_bHostageSpotted[0]))),
	RecvPropArray3( RECVINFO_ARRAY(m_iMVPs), RecvPropInt( RECVINFO(m_iMVPs[0]))),
	RecvPropArray3( RECVINFO_ARRAY(m_bHasDefuser), RecvPropInt( RECVINFO(m_bHasDefuser[0]))),
	RecvPropArray3( RECVINFO_ARRAY(m_iAccount), RecvPropInt( RECVINFO(m_iAccount[0]))),
	RecvPropArray3( RECVINFO_ARRAY(m_iGunGameProgressiveWeaponIndex), RecvPropInt( RECVINFO(m_iGunGameProgressiveWeaponIndex[0]))),
	RecvPropArray3( RECVINFO_ARRAY(m_iContributionScore), RecvPropInt( RECVINFO(m_iContributionScore[0]))),
	RecvPropArray3( RECVINFO_ARRAY(m_nMusicID), RecvPropInt( RECVINFO(m_nMusicID[0]))),
	RecvPropArray3( RECVINFO_ARRAY(m_bControllingBot), RecvPropInt( RECVINFO(m_bControllingBot[0]))),
	RecvPropArray3( RECVINFO_ARRAY(m_iControlledPlayer), RecvPropInt( RECVINFO(m_iControlledPlayer[0]))),
	RecvPropArray3( RECVINFO_ARRAY(m_iControlledByPlayer), RecvPropInt( RECVINFO(m_iControlledByPlayer[0]))),
	RecvPropArray3( RECVINFO_ARRAY(m_szClan), RecvPropString( RECVINFO(m_szClan[0]))),
END_RECV_TABLE()
 
//=============================================================================
// HPE_END
//=============================================================================

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_CS_PlayerResource::C_CS_PlayerResource()
{
	vgui::IScheme *pClientScheme = vgui::scheme()->GetIScheme( vgui::scheme()->GetScheme( "ClientScheme" ) );

	m_Colors[TEAM_TERRORIST] = pClientScheme->GetColor( "TeamT", COLOR_BLUE );
	m_Colors[TEAM_CT] = pClientScheme->GetColor( "TeamCT", COLOR_RED );
	memset( m_iMVPs, 0, sizeof( m_iMVPs ) );
	memset( m_bHasDefuser, 0, sizeof( m_bHasDefuser ) );
	memset( m_iAccount, 0, sizeof( m_iAccount ) );
	memset( m_iGunGameProgressiveWeaponIndex, 0, sizeof( m_iGunGameProgressiveWeaponIndex ) );
	memset( m_iContributionScore, 0, sizeof( m_iContributionScore ) );
	memset( m_nMusicID, 0, sizeof( m_nMusicID ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_CS_PlayerResource::~C_CS_PlayerResource()
{
}

bool C_CS_PlayerResource::HasC4(int iIndex )
{
	return m_iPlayerC4 == iIndex;
}

bool C_CS_PlayerResource::IsHostageAlive(int iIndex)
{
	if ( iIndex < 0 || iIndex >= MAX_HOSTAGES )
		return false;

	return m_bHostageAlive[iIndex];
}

bool C_CS_PlayerResource::IsHostageFollowingSomeone(int iIndex)
{
	if ( iIndex < 0 || iIndex >= MAX_HOSTAGES )
		return false;

	return m_isHostageFollowingSomeone[iIndex];
}

int C_CS_PlayerResource::GetHostageEntityID(int iIndex)
{
	if ( iIndex < 0 || iIndex >= MAX_HOSTAGES )
		return -1;

	return m_iHostageEntityIDs[iIndex];
}

const Vector C_CS_PlayerResource::GetHostagePosition( int iIndex )
{
	if ( iIndex < 0 || iIndex >= MAX_HOSTAGES )
		return vec3_origin;

	Vector ret;

	ret.x = m_iHostageX[iIndex];
	ret.y = m_iHostageY[iIndex];
	ret.z = m_iHostageZ[iIndex];

	return ret;
}

const Vector C_CS_PlayerResource::GetC4Postion()
{
	if ( m_iPlayerC4 > 0 )
	{
		// C4 is carried by player
		C_BasePlayer *pPlayer = UTIL_PlayerByIndex( m_iPlayerC4 );

		if ( pPlayer )
		{
			return pPlayer->GetAbsOrigin();
		}
	}

	// C4 is lying on ground
	return m_vecC4;
}

const Vector C_CS_PlayerResource::GetBombsiteAPosition()
{
	return m_bombsiteCenterA;
}

const Vector C_CS_PlayerResource::GetBombsiteBPosition()
{
	return m_bombsiteCenterB;
}

const Vector C_CS_PlayerResource::GetHostageRescuePosition( int iIndex )
{
	if ( iIndex < 0 || iIndex >= MAX_HOSTAGE_RESCUES )
		return vec3_origin;

	Vector ret;

	ret.x = m_hostageRescueX[iIndex];
	ret.y = m_hostageRescueY[iIndex];
	ret.z = m_hostageRescueZ[iIndex];

	return ret;
}

int C_CS_PlayerResource::GetPlayerClass( int iIndex )
{
	if ( !IsConnected( iIndex ) )
	{
		return CS_CLASS_NONE;
	}

	return m_iPlayerClasses[ iIndex ];
}

//--------------------------------------------------------------------------------------------------------
bool C_CS_PlayerResource::IsBombSpotted( void ) const
{
	return m_bBombSpotted;
}

//--------------------------------------------------------------------------------------------------------
bool C_CS_PlayerResource::IsPlayerSpotted( int iIndex )
{
	if ( !IsConnected( iIndex ) )
		return false;

	return m_bPlayerSpotted[iIndex];
}

//--------------------------------------------------------------------------------------------------------
bool C_CS_PlayerResource::IsHostageSpotted( int iIndex )
{
	return m_bHostageSpotted[iIndex];
}

//-----------------------------------------------------------------------------
const char *C_CS_PlayerResource::GetClanTag( int iIndex )
{
	if ( iIndex < 1 || iIndex > MAX_PLAYERS )
	{
		Assert( false );
		return "";
	}

	if ( !IsConnected( iIndex ) )
		return "";

	return m_szClan[iIndex];
}

bool C_CS_PlayerResource::IsControllingBot( int index )
{
	return m_bControllingBot[index];
}

int C_CS_PlayerResource::GetControlledPlayer( int index )
{
	return m_iControlledPlayer[index];
}

int C_CS_PlayerResource::GetControlledByPlayer( int index )
{
	return m_iControlledByPlayer[index];
}

ConVar cl_add_bot_prefix( "cl_add_bot_prefix", "1", FCVAR_ARCHIVE, "Whether to add a BOT prefix to bot names or not.", true, 0, true, 1 );
const wchar_t* C_CS_PlayerResource::GetDecoratedPlayerName( int index, wchar_t* buffer, int buffsize, EDecoratedPlayerNameFlag_t flags )
{
	if ( IsConnected( index ) )
	{
		bool addBotToNameIfControllingBot = !!(flags & k_EDecoratedPlayerNameFlag_AddBotToNameIfControllingBot);
		bool useNameOfControllingPlayer = !(flags & k_EDecoratedPlayerNameFlag_DontUseNameOfControllingPlayer);
		bool bShowClanName = !(flags & k_EDecoratedPlayerNameFlag_DontShowClanName);

		int nameIndex = index;
		int controlledBy = GetControlledByPlayer( index );
		int nBotControlStringType = 0; // normal name

		if( controlledBy && useNameOfControllingPlayer )
		{
			nBotControlStringType = 1;// BOT ( name )
			nameIndex = controlledBy;
		}
		else if ( IsFakePlayer( index ) )
		{
			nBotControlStringType = 2; // BOT name
		}
		else if ( IsControllingBot( index ) && addBotToNameIfControllingBot )
		{
			nBotControlStringType = 1; // BOT ( name )
		}

		wchar_t wide_name[MAX_PLAYER_NAME_LENGTH];
		wide_name[0] = L'\0';
		char nameBuf[MAX_PLAYER_NAME_LENGTH] = {0};

		if ( !cl_add_bot_prefix.GetBool() )
			nBotControlStringType = 0;

		if ( nBotControlStringType == 2 )
		{
			V_snprintf( nameBuf, ARRAYSIZE( nameBuf ) - 1, "%s", GetPlayerName( nameIndex ) );
		}
		else
		{
			//wchar_t wszClanTag[ MAX_PLAYER_NAME_LENGTH ];
			char szClan[MAX_PLAYER_NAME_LENGTH];
			if ( bShowClanName && Q_strlen( GetClanTag( index ) ) > 1 )
			{
				const char* optionalSpace = "";
				if ( GetClanTag( index )[0] == '#' )
				{
					optionalSpace = " ";
				}
				Q_snprintf( szClan, sizeof( szClan ), "%s%s ", optionalSpace, GetClanTag( index ) );
			}
			else
			{
				szClan[0] = 0;
			}
			//g_pVGuiLocalize->ConvertANSIToUnicode( szClan, wszClanTag, sizeof( wszClanTag ) );

			V_snprintf( nameBuf, ARRAYSIZE( nameBuf ) - 1, "%s%s", szClan, GetPlayerName( nameIndex ) );
		}

		V_UTF8ToUnicode( nameBuf /*GetPlayerName( nameIndex )*/, wide_name, sizeof( wide_name ) );

		if ( !nBotControlStringType ) // normal name
		{
			V_wcsncpy( buffer, wide_name, buffsize );
		}
		else
		{
			const char* translationID = ( nBotControlStringType == 1 ) ? "#Cstrike_bot_controlled_by" : "#Cstrike_bot_decorated_name";
			g_pVGuiLocalize->ConstructString( buffer, buffsize, g_pVGuiLocalize->Find( translationID ), 1, wide_name );
		}
	}
	else
	{
		*buffer = L'\0';
	}

	return buffer;
}

C_CS_PlayerResource * GetCSResources( void )
{
	return (C_CS_PlayerResource*)g_PR;
}

//-----------------------------------------------------------------------------
int C_CS_PlayerResource::GetNumMVPs( int iIndex )
{
	if ( !IsConnected( iIndex ) )
		return false;

	return m_iMVPs[iIndex];
} 

//-----------------------------------------------------------------------------
bool C_CS_PlayerResource::HasDefuser( int iIndex )
{
	if ( !IsConnected( iIndex ) )
		return false;

	return m_bHasDefuser[iIndex];
}

//-----------------------------------------------------------------------------
int C_CS_PlayerResource::GetAccount( int iIndex )
{
	if ( !IsConnected( iIndex ) )
		return false;

	return m_iAccount[iIndex];
}

//-----------------------------------------------------------------------------
int C_CS_PlayerResource::GetPlayerGunGameWeaponIndex( int iIndex )
{
	if ( !IsConnected( iIndex ) )
		return false;

	return m_iGunGameProgressiveWeaponIndex[iIndex];
}

//--------------------------------------------------------------------------------------------------------
int C_CS_PlayerResource::GetContributionScore( int iIndex )
{
	if ( !IsConnected( iIndex ) )
		return 0;

	return m_iContributionScore[iIndex];
}

//--------------------------------------------------------------------------------------------------------
int C_CS_PlayerResource::GetMusicID( int iIndex )
{
	if ( !IsConnected( iIndex ) )
		return 0;

	return m_nMusicID[ iIndex ];
}

