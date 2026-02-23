//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================

#include "cbase.h"
#include <vgui_controls/Controls.h>
#include <vgui_controls/Panel.h>
#include <vgui/ISurface.h>
#include "vgui_avatarimage.h"
#if defined( _X360 )
#include "xbox/xbox_win32stubs.h"
#endif
#include "steam/steam_api.h"
#include "vtf/vtf.h"
#include "filesystem.h"
#include "c_cs_playerresource.h"
#include "checksum_crc.h"
#include "qlimits.h"

// cl_avatar is defined in engine/client.cpp with FCVAR_USERINFO for proper custom file upload
extern ConVar cl_avatar;

// Helper class to build CRC-based filename (same as engine uses for sprays)
// Matches the format in engine/logofile_shared.h
class CAvatarCustomFilename
{
public:
        CAvatarCustomFilename( CRC32_t value ) 
        {
                char hex[16];
                Q_binarytohex( (byte *)&value, sizeof( value ), hex, sizeof( hex ) );
                Q_snprintf( m_Filename, sizeof( m_Filename ), "user_custom/%c%c/%s.dat", hex[0], hex[1], hex );
        }
        char m_Filename[MAX_OSPATH];
};

// Cache for VTF avatar texture ID (keyed by CRC)
static int s_iVTFAvatarTextureID = -1;
static CRC32_t s_nVTFAvatarCRC = 0;

DECLARE_BUILD_FACTORY( CAvatarImagePanel );

//-----------------------------------------------------------------------------
// Purpose: Load avatar image data from a VTF file
// Returns: true if successful, caller must free *ppRGBA with delete[]
//-----------------------------------------------------------------------------
bool AvatarImage_LoadVTFAvatarImage( const char *szFilePath, byte **ppRGBA, int *pWidth, int *pHeight )
{
    if ( !szFilePath || !szFilePath[0] || !ppRGBA || !pWidth || !pHeight )
        return false;

    *ppRGBA = NULL;
    *pWidth = 0;
    *pHeight = 0;

    // Build full path - only add .vtf extension if not already present
    char szFullPath[MAX_PATH];
    int nLen = Q_strlen( szFilePath );
    if ( nLen > 4 && Q_stricmp( szFilePath + nLen - 4, ".vtf" ) == 0 )
    {
        Q_strncpy( szFullPath, szFilePath, sizeof(szFullPath) );
    }
    else
    {
        Q_snprintf( szFullPath, sizeof(szFullPath), "%s.vtf", szFilePath );
    }

    // Read the VTF file in binary mode (critical for VTF files)
    CUtlBuffer buf( 0, 0, 0 ); // Binary mode - no text translation
    if ( !g_pFullFileSystem->ReadFile( szFullPath, "GAME", buf ) )
    {
        Warning( "AvatarImage_LoadVTFAvatarImage: Failed to read file '%s'\n", szFullPath );
        return false;
    }

    // Create and load VTF texture
    IVTFTexture *pVTFTexture = CreateVTFTexture();
    if ( !pVTFTexture )
    {
        Warning( "AvatarImage_LoadVTFAvatarImage: Failed to create VTF texture\n" );
        return false;
    }

    if ( !pVTFTexture->Unserialize( buf ) )
    {
        Warning( "AvatarImage_LoadVTFAvatarImage: Failed to unserialize VTF '%s'\n", szFullPath );
        DestroyVTFTexture( pVTFTexture );
        return false;
    }

    // Get dimensions
    int nWidth = pVTFTexture->Width();
    int nHeight = pVTFTexture->Height();

    if ( nWidth <= 0 || nHeight <= 0 )
    {
        Warning( "AvatarImage_LoadVTFAvatarImage: Invalid dimensions in '%s'\n", szFullPath );
        DestroyVTFTexture( pVTFTexture );
        return false;
    }

    // Convert to RGBA8888 format
    pVTFTexture->ConvertImageFormat( IMAGE_FORMAT_RGBA8888, false );

    // Allocate output buffer
    int nBufferSize = nWidth * nHeight * 4;
    byte *pRGBA = new byte[nBufferSize];
    if ( !pRGBA )
    {
        DestroyVTFTexture( pVTFTexture );
        return false;
    }

    // Copy image data
    byte *pSrcData = pVTFTexture->ImageData( 0, 0, 0 );
    if ( pSrcData )
    {
        Q_memcpy( pRGBA, pSrcData, nBufferSize );
    }
    else
    {
        delete[] pRGBA;
        DestroyVTFTexture( pVTFTexture );
        return false;
    }

    DestroyVTFTexture( pVTFTexture );

    *ppRGBA = pRGBA;
    *pWidth = nWidth;
    *pHeight = nHeight;

    return true;
}


CUtlMap< AvatarImagePair_t, int> CAvatarImage::s_AvatarImageCache; // cache of steam id's to textureids to use for images
bool CAvatarImage::m_sbInitializedAvatarCache = false;

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CAvatarImage::CAvatarImage( void )
: m_sPersonaStateChangedCallback( this, &CAvatarImage::OnPersonaStateChanged )
{
        ClearAvatarSteamID();
        m_pFriendIcon = NULL;
        m_nX = 0;
        m_nY = 0;
        m_wide = m_tall = 0;
        m_avatarWide = m_avatarTall = 0;
        m_Color = Color( 255, 255, 255, 255 );
        m_bLoadPending = false;
        m_fNextLoadTime = 0.0f;
        m_AvatarSize = k_EAvatarSize32x32;
        m_bIsVTFAvatar = false;
        
        //=============================================================================
        // HPE_BEGIN:
        //=============================================================================
        // [tj] Default to drawing the friend icon for avatars
        m_bDrawFriend = true;

        // [menglish] Default icon for avatar icons if there is no avatar icon for the player
        m_iTextureID = -1;

        // set up friend icon
        m_pFriendIcon = gHUD.GetIcon( "ico_friend_indicator_avatar" );

        m_pDefaultImage = NULL;

        SetAvatarSize(DEFAULT_AVATAR_SIZE, DEFAULT_AVATAR_SIZE);

        //=============================================================================
        // HPE_END
        //=============================================================================

        if ( !m_sbInitializedAvatarCache) 
        {
                m_sbInitializedAvatarCache = true;
                SetDefLessFunc( s_AvatarImageCache );
        }
}

//-----------------------------------------------------------------------------
// Purpose: reset the image to a default state (will render with the default image)
//-----------------------------------------------------------------------------
void CAvatarImage::ClearAvatarSteamID( void )
{
        m_bValid = false;
        m_bFriend = false;
        m_bLoadPending = false;
        m_bIsVTFAvatar = false;
        m_SteamID.Set( 0, k_EUniverseInvalid, k_EAccountTypeInvalid );
        m_sPersonaStateChangedCallback.Unregister();
}


//-----------------------------------------------------------------------------
// Purpose: Set the CSteamID for this image; this will cause a deferred load
//-----------------------------------------------------------------------------
bool CAvatarImage::SetAvatarSteamID( CSteamID steamIDUser, EAvatarSize avatarSize /*= k_EAvatarSize32x32 */ )
{
        ClearAvatarSteamID();

        m_SteamID = steamIDUser;
        m_AvatarSize = avatarSize;
        m_bLoadPending = true;

        m_sPersonaStateChangedCallback.Register( this, &CAvatarImage::OnPersonaStateChanged );

        LoadAvatarImage();
        UpdateFriendStatus();

        return m_bValid;
}

//-----------------------------------------------------------------------------
// Purpose: Called when somebody changes their avatar image
//-----------------------------------------------------------------------------
void CAvatarImage::OnPersonaStateChanged( PersonaStateChange_t *info )
{
        if ( ( info->m_ulSteamID == m_SteamID.ConvertToUint64() ) && ( info->m_nChangeFlags & k_EPersonaChangeAvatar ) )
        {
                // Mark us as invalid.
                m_bValid = false;
                m_bLoadPending = true;

                // Poll
                LoadAvatarImage();
        }
}

//-----------------------------------------------------------------------------
// Purpose: load the avatar image if we have a load pending
//-----------------------------------------------------------------------------
void CAvatarImage::LoadAvatarImage()
{
#ifdef CSS_PERF_TEST
        return;
#endif
        // attempt to retrieve the avatar image from Steam
        if ( m_bLoadPending && steamapicontext->SteamFriends() && steamapicontext->SteamUtils() && gpGlobals->curtime >= m_fNextLoadTime )
        {
                if ( !steamapicontext->SteamFriends()->RequestUserInformation( m_SteamID, false ) )
                {
                        int iAvatar = 0;
                        switch( m_AvatarSize )
                        {
                                case k_EAvatarSize32x32: 
                                        iAvatar = steamapicontext->SteamFriends()->GetSmallFriendAvatar( m_SteamID );
                                        break;
                                case k_EAvatarSize64x64: 
                                        iAvatar = steamapicontext->SteamFriends()->GetMediumFriendAvatar( m_SteamID );
                                        break;
                                case k_EAvatarSize184x184: 
                                        iAvatar = steamapicontext->SteamFriends()->GetLargeFriendAvatar( m_SteamID );
                                        break;
                        }

                        //Msg( "Got avatar %d for SteamID %llud (%s)\n", iAvatar, m_SteamID.ConvertToUint64(), steamapicontext->SteamFriends()->GetFriendPersonaName( m_SteamID ) );

                        if ( iAvatar > 0 ) // if its zero, user doesn't have an avatar.  If -1, Steam is telling us that it's fetching it
                        {
                                uint32 wide = 0, tall = 0;
                                if ( steamapicontext->SteamUtils()->GetImageSize( iAvatar, &wide, &tall ) && wide > 0 && tall > 0 )
                                {
                                        int destBufferSize = wide * tall * 4;
                                        byte *rgbDest = (byte*)stackalloc( destBufferSize );
                                        if ( steamapicontext->SteamUtils()->GetImageRGBA( iAvatar, rgbDest, destBufferSize ) )
                                                InitFromRGBA( iAvatar, rgbDest, wide, tall );
                                        
                                        stackfree( rgbDest );
                                }
                        }
                }

                if ( m_bValid )
                {
                        // if we have a valid image, don't attempt to load it again
                        m_bLoadPending = false;
                }
                else
                {
                        // otherwise schedule another attempt to retrieve the image
                        m_fNextLoadTime = gpGlobals->curtime + 1.0f;
                }
        }
}


//-----------------------------------------------------------------------------
// Purpose: Query Steam to set the m_bFriend status flag
//-----------------------------------------------------------------------------
void CAvatarImage::UpdateFriendStatus( void )
{
        if ( !m_SteamID.IsValid() )
                return;

        if ( steamapicontext->SteamFriends() && steamapicontext->SteamUtils() )
                m_bFriend = steamapicontext->SteamFriends()->HasFriend( m_SteamID, k_EFriendFlagImmediate );
}

//-----------------------------------------------------------------------------
// Purpose: Initialize the surface with the supplied raw RGBA image data
//-----------------------------------------------------------------------------
void CAvatarImage::InitFromRGBA( int iAvatar, const byte *rgba, int width, int height )
{
        int iTexIndex = s_AvatarImageCache.Find( AvatarImagePair_t( m_SteamID, iAvatar ) );
        if ( iTexIndex == s_AvatarImageCache.InvalidIndex() )
        {
                m_iTextureID = vgui::surface()->CreateNewTextureID( true );
                vgui::surface()->DrawSetTextureRGBA( m_iTextureID, rgba, width, height, false, false );
                iTexIndex = s_AvatarImageCache.Insert( AvatarImagePair_t( m_SteamID, iAvatar ) );
                s_AvatarImageCache[ iTexIndex ] = m_iTextureID;
        }
        else
                m_iTextureID = s_AvatarImageCache[ iTexIndex ];
        
        m_bValid = true;
}

//-----------------------------------------------------------------------------
// Purpose: Initialize from VTF RGBA data (for VTF avatar loading)
//-----------------------------------------------------------------------------
void CAvatarImage::InitFromRGBA_VTF( const byte *rgba, int width, int height, CRC32_t crc )
{
        // Check if we can reuse cached VTF avatar texture
        if ( s_iVTFAvatarTextureID != -1 && s_nVTFAvatarCRC == crc && crc != 0 )
        {
                // Reuse cached texture
                m_iTextureID = s_iVTFAvatarTextureID;
        }
        else
        {
                // Create new texture for VTF avatar
                m_iTextureID = vgui::surface()->CreateNewTextureID( true );
                vgui::surface()->DrawSetTextureRGBA( m_iTextureID, rgba, width, height, false, false );
                
                // Cache it
                s_iVTFAvatarTextureID = m_iTextureID;
                s_nVTFAvatarCRC = crc;
        }
        
        m_bValid = true;
        m_bIsVTFAvatar = true;
}

//-----------------------------------------------------------------------------
// Purpose: Load avatar from a VTF file using CRC-based path (like sprays)
// The avatar VTF is uploaded via sv_allowupload and stored in user_custom/ folder
// Returns: true if successful
//-----------------------------------------------------------------------------
bool CAvatarImage::SetAvatarFromCRC( CRC32_t crc )
{
        if ( crc == 0 )
        {
                // No avatar CRC set - will use default team avatar
                return false;
        }
        
        // Check if already cached with same CRC
        if ( s_iVTFAvatarTextureID != -1 && s_nVTFAvatarCRC == crc )
        {
                ClearAvatarSteamID();
                m_iTextureID = s_iVTFAvatarTextureID;
                m_bValid = true;
                m_bIsVTFAvatar = true;
                return true;
        }
        
        // Build path from CRC using same format as engine spray system
        CAvatarCustomFilename customFile( crc );
        
        // Read the custom file (it's a VTF stored as .dat)
        CUtlBuffer buf( 0, 0, 0 );
        if ( !g_pFullFileSystem->ReadFile( customFile.m_Filename, "GAME", buf ) )
        {
                // Try download folder as well (for files downloaded from server)
                if ( !g_pFullFileSystem->ReadFile( customFile.m_Filename, "download", buf ) )
                {
                        // File not found - may not have been downloaded yet
                        DevMsg( "Avatar: CRC %08X file not found at %s\n", crc, customFile.m_Filename );
                        return false;
                }
        }
        
        // Create and load VTF texture from buffer
        IVTFTexture *pVTFTexture = CreateVTFTexture();
        if ( !pVTFTexture )
                return false;
                
        if ( !pVTFTexture->Unserialize( buf ) )
        {
                Warning( "Avatar: Failed to unserialize VTF from %s\n", customFile.m_Filename );
                DestroyVTFTexture( pVTFTexture );
                return false;
        }
        
        int nWidth = pVTFTexture->Width();
        int nHeight = pVTFTexture->Height();
        
        if ( nWidth <= 0 || nHeight <= 0 )
        {
                DestroyVTFTexture( pVTFTexture );
                return false;
        }
        
        pVTFTexture->ConvertImageFormat( IMAGE_FORMAT_RGBA8888, false );
        
        int nBufferSize = nWidth * nHeight * 4;
        byte *pRGBA = new byte[nBufferSize];
        
        byte *pSrcData = pVTFTexture->ImageData( 0, 0, 0 );
        if ( pSrcData )
        {
                Q_memcpy( pRGBA, pSrcData, nBufferSize );
        }
        else
        {
                delete[] pRGBA;
                DestroyVTFTexture( pVTFTexture );
                return false;
        }
        
        DestroyVTFTexture( pVTFTexture );
        
        ClearAvatarSteamID();
        InitFromRGBA_VTF( pRGBA, nWidth, nHeight, crc );
        
        delete[] pRGBA;
        
        DevMsg( "Avatar: Loaded from CRC %08X successfully\n", crc );
        return m_bValid;
}

//-----------------------------------------------------------------------------
// Purpose: Load avatar for any player using their networked avatar CRC from server
// This enables server-side avatar sharing (works like sprays with sv_allowupload)
// Returns: true if successfully loaded VTF avatar for the player
//-----------------------------------------------------------------------------
bool CAvatarImage::SetAvatarFromNetworkedCRC( int iPlayerIndex )
{
        // Get the player resource to access networked avatar CRCs
        C_CS_PlayerResource *pResource = dynamic_cast<C_CS_PlayerResource*>( g_PR );
        if ( !pResource )
                return false;
        
        // Get the networked avatar CRC for this player
        CRC32_t avatarCRC = pResource->GetAvatarCRC( iPlayerIndex );
        if ( avatarCRC == 0 )
        {
                // No custom avatar set for this player
                return false;
        }
        
        // Load the VTF avatar from the CRC-based path
        return SetAvatarFromCRC( avatarCRC );
}

//-----------------------------------------------------------------------------
// Purpose: Load avatar directly from a VTF file path
// This is used for the local player when not connected to a server
// Returns: true if successfully loaded the VTF avatar
//-----------------------------------------------------------------------------
bool CAvatarImage::SetAvatarFromVTFFile( const char *szFilePath )
{
        if ( !szFilePath || !szFilePath[0] )
                return false;

        byte *pRGBA = NULL;
        int nWidth = 0, nHeight = 0;

        if ( !AvatarImage_LoadVTFAvatarImage( szFilePath, &pRGBA, &nWidth, &nHeight ) )
        {
                return false;
        }

        ClearAvatarSteamID();
        InitFromRGBA_VTF( pRGBA, nWidth, nHeight, 0 );

        delete[] pRGBA;

        DevMsg( "Avatar: Loaded from file %s successfully\n", szFilePath );
        return m_bValid;
}

//-----------------------------------------------------------------------------
// Purpose: Draw the image and optional friend icon
//-----------------------------------------------------------------------------
void CAvatarImage::Paint( void )
{
        if ( m_bFriend && m_pFriendIcon && m_bDrawFriend)
        {
                m_pFriendIcon->DrawSelf( m_nX, m_nY, m_wide, m_tall, m_Color );
        }

        int posX = m_nX;
        int posY = m_nY;

        if (m_bDrawFriend)
        {
                posX += FRIEND_ICON_AVATAR_INDENT_X * m_avatarWide / DEFAULT_AVATAR_SIZE;
                posY += FRIEND_ICON_AVATAR_INDENT_Y * m_avatarTall / DEFAULT_AVATAR_SIZE;
        }
        
        if ( m_bLoadPending )
        {
                LoadAvatarImage();
        }

        if ( m_bValid )
        {
                vgui::surface()->DrawSetTexture( m_iTextureID );
                vgui::surface()->DrawSetColor( m_Color );
                vgui::surface()->DrawTexturedRect(posX, posY, posX + m_avatarWide, posY + m_avatarTall);
        }
        else if (m_pDefaultImage)
        {
                // draw default
                m_pDefaultImage->SetSize(m_avatarWide, m_avatarTall);
                m_pDefaultImage->SetPos(posX, posY);
                m_pDefaultImage->SetColor(m_Color);
                m_pDefaultImage->Paint();
        }
}

//-----------------------------------------------------------------------------
// Purpose: Set the avatar size; scale the total image and friend icon to fit
//-----------------------------------------------------------------------------
void CAvatarImage::SetAvatarSize(int wide, int tall)
{
        m_avatarWide = wide;
        m_avatarTall = tall;

        if (m_bDrawFriend)
        {
                // scale the size of the friend background frame icon
                m_wide = FRIEND_ICON_SIZE_X * m_avatarWide / DEFAULT_AVATAR_SIZE;
                m_tall = FRIEND_ICON_SIZE_Y * m_avatarTall / DEFAULT_AVATAR_SIZE;
        }
        else
        {
                m_wide = m_avatarWide;
                m_tall = m_avatarTall;
        }
}


//-----------------------------------------------------------------------------
// Purpose: Set the total image size; scale the avatar portion to fit
//-----------------------------------------------------------------------------
void CAvatarImage::SetSize( int wide, int tall )
{
        m_wide = wide;
        m_tall = tall;

        if (m_bDrawFriend)
        {
                // scale the size of the avatar portion based on the total image size
                m_avatarWide = DEFAULT_AVATAR_SIZE * m_wide / FRIEND_ICON_SIZE_X;
                m_avatarTall = DEFAULT_AVATAR_SIZE * m_tall / FRIEND_ICON_SIZE_Y ;
        }
        else
        {
                m_avatarWide = m_wide;
                m_avatarTall = m_tall;
        }
}

bool CAvatarImage::Evict()
{
        return false;
}

int CAvatarImage::GetNumFrames()
{
        return 0;
}

void CAvatarImage::SetFrame( int nFrame )
{
}

vgui::HTexture CAvatarImage::GetID()
{
        return 0;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CAvatarImagePanel::CAvatarImagePanel( vgui::Panel *parent, const char *name ) : BaseClass( parent, name )
{
        m_bScaleImage = false;
        m_pImage = new CAvatarImage();
        m_bSizeDirty = true;
        m_bClickable = false;
}


//-----------------------------------------------------------------------------
// Purpose: Set the avatar by C_BasePlayer pointer
//-----------------------------------------------------------------------------
void CAvatarImagePanel::SetPlayer( C_BasePlayer *pPlayer, EAvatarSize avatarSize )
{
        if ( pPlayer )
        {
                int iIndex = pPlayer->entindex();
                SetPlayer(iIndex, avatarSize);
        }
        else
                m_pImage->ClearAvatarSteamID();

}


//-----------------------------------------------------------------------------
// Purpose: Set the avatar by entity number
// Uses CRC-based system like sprays - avatar VTF is uploaded via sv_allowupload (slot 2)
//-----------------------------------------------------------------------------
void CAvatarImagePanel::SetPlayer( int entindex, EAvatarSize avatarSize )
{
        m_pImage->ClearAvatarSteamID();

        // Try to load avatar from networked CRC (uploaded like sprays via sv_allowupload)
        // This works for all players - the VTF file is uploaded to server and downloaded by other clients
        if ( m_pImage->SetAvatarFromNetworkedCRC( entindex ) )
        {
                return; // Successfully loaded avatar from CRC
        }

        // Fallback: For local player, also check cl_avatar directly (in case not yet networked)
        C_BasePlayer *pLocalPlayer = C_BasePlayer::GetLocalPlayer();
        if ( pLocalPlayer && pLocalPlayer->entindex() == entindex )
        {
                // Check if local player has avatar set via cl_avatar cvar
                player_info_t pi;
                if ( engine->GetPlayerInfo( entindex, &pi ) && pi.customFiles[2] != 0 )
                {
                        // Try loading from local custom file
                        if ( m_pImage->SetAvatarFromCRC( pi.customFiles[2] ) )
                        {
                                return;
                        }
                }
        }

        // No custom avatar - try Steam avatar
        player_info_t pi;
        if ( engine->GetPlayerInfo(entindex, &pi) )
        {
                if ( pi.friendsID != 0  && steamapicontext->SteamUtils() )
                {               
                        CSteamID steamIDForPlayer( pi.friendsID, 1, steamapicontext->SteamUtils()->GetConnectedUniverse(), k_EAccountTypeIndividual );
                        SetPlayer(steamIDForPlayer, avatarSize);
                }
                else
                {
                        m_pImage->ClearAvatarSteamID();
                }
        }
}

//-----------------------------------------------------------------------------
// Purpose: Set the avatar by SteamID
//-----------------------------------------------------------------------------
void CAvatarImagePanel::SetPlayer(CSteamID steamIDForPlayer, EAvatarSize avatarSize )
{
        m_pImage->ClearAvatarSteamID();

        if (steamIDForPlayer.GetAccountID() != 0 )
                m_pImage->SetAvatarSteamID( steamIDForPlayer, avatarSize );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CAvatarImagePanel::PaintBackground( void )
{
        if ( m_bSizeDirty )
                UpdateSize();

        m_pImage->Paint();
}

void CAvatarImagePanel::ClearAvatar()
{
        m_pImage->ClearAvatarSteamID();
}

void CAvatarImagePanel::SetDefaultAvatar( vgui::IImage* pDefaultAvatar )
{
        m_pImage->SetDefaultImage(pDefaultAvatar);
}

bool CAvatarImagePanel::SetAvatarFromVTFFile( const char *szFilePath )
{
        return m_pImage->SetAvatarFromVTFFile( szFilePath );
}

void CAvatarImagePanel::SetAvatarSize( int width, int height )
{
        if ( m_bScaleImage )
        {
                // panel is charge of image size - setting avatar size this way not allowed
                Assert(false);
                return;
        }
        else
        {
                m_pImage->SetAvatarSize( width, height );
                m_bSizeDirty = true;
        }
}

void CAvatarImagePanel::OnSizeChanged( int newWide, int newTall )
{
        BaseClass::OnSizeChanged(newWide, newTall);
        m_bSizeDirty = true;
}

void CAvatarImagePanel::OnMousePressed(vgui::MouseCode code)
{
        if ( !m_bClickable || code != MOUSE_LEFT )
                return;

        PostActionSignal( new KeyValues("AvatarMousePressed") );

        // audible feedback
        const char *soundFilename = "ui/buttonclick.wav";

        vgui::surface()->PlaySound( soundFilename );
}

void CAvatarImagePanel::SetShouldScaleImage( bool bScaleImage )
{
        m_bScaleImage = bScaleImage;
        m_bSizeDirty = true;
}

void CAvatarImagePanel::SetShouldDrawFriendIcon( bool bDrawFriend )
{
        m_pImage->SetDrawFriend(bDrawFriend);
        m_bSizeDirty = true;
}

void CAvatarImagePanel::UpdateSize()
{
        if ( m_bScaleImage )
        {
                // the panel is in charge of the image size
                m_pImage->SetAvatarSize(GetWide(), GetTall());
        }
        else
        {
                // the image is in charge of the panel size
                SetSize(m_pImage->GetAvatarWide(), m_pImage->GetAvatarTall() );
        }

        m_bSizeDirty = false;
}

void CAvatarImagePanel::ApplySettings( KeyValues *inResourceData )
{
        m_bScaleImage = inResourceData->GetInt("scaleImage", 0);

        BaseClass::ApplySettings(inResourceData);
}
