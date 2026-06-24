//========= CS-Source-Advanced ============//
//
// cl_identification.cpp — hardware-based persistent unique ID
//
// Ported and adapted from xash3d-fwgs identification.c
// Copyright (C) 2017 mittorn (xash3d-fwgs)
//
//
//=========================================//

#include "client_pch.h"
#include "cl_identification.h"
#include "common.h"
#include "tier1/checksum_crc.h"
#include "tier1/checksum_md5.h"
#include "tier0/platform.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#if defined( _WIN32 )
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#elif defined( POSIX )
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#endif

//=========================================
// Bloom filter — 64-bit, single hash func
//=========================================

typedef uint64_t bloomfilter_t;

#define BF64_MASK       ((1U << 6) - 1)
#define MAXBITS_GEN     30
#define MAXBITS_CHECK   (MAXBITS_GEN + 6)

#define SYSTEM_XOR_MASK  UINT64_C( 0x10331c2dce4c91db )
#define GAME_XOR_MASK    UINT64_C( 0x7ffc48fbac1711f1 )

static bloomfilter_t s_id = 0;
static char s_idString[32] = { 0 };

//=========================================
// Bloom filter helpers
//=========================================

static bloomfilter_t BF_Process( const char *buf, int size )
{
    if ( size <= 0 || size > 512 )
        return 0;

    CRC32_t crc;
    CRC32_Init( &crc );
    CRC32_ProcessBuffer( &crc, buf, size );
    CRC32_Final( &crc );

    bloomfilter_t value = 0;
    uint32_t c = (uint32_t)crc;
    while ( c )
    {
        value |= ( (uint64_t)1 ) << ( c & BF64_MASK );
        c >>= 6;
    }
    return value;
}

static bloomfilter_t BF_ProcessStr( const char *str )
{
    return BF_Process( str, (int)strlen( str ) );
}

static int BF_Weight( bloomfilter_t value )
{
    int weight = 0;
    while ( value )
    {
        if ( value & 1 ) weight++;
        value >>= 1;
    }
    return weight;
}

//=========================================
// Hex validator — rejects trivial serials
//=========================================

static bool ID_VerifyHEX( const char *hex )
{
    unsigned int chars = 0;
    char prev = 0;
    bool monotonic = true;

    while ( *hex )
    {
        char ch = (char)tolower( (unsigned char)*hex );
        hex++;

        if ( ( ch >= 'a' && ch <= 'f' ) || ( ch >= '0' && ch <= '9' ) )
        {
            if ( prev && ( ch - prev < -1 || ch - prev > 1 ) )
                monotonic = false;

            if ( ch >= 'a' )
                chars |= 1u << ( ch - 'a' + 10 );
            else
                chars |= 1u << ( ch - '0' );

            prev = ch;
        }
    }

    if ( monotonic ) return false;

    int weight = 0;
    while ( chars )
    {
        if ( chars & 1 ) weight++;
        chars >>= 1;
        if ( weight > 2 ) return true;
    }
    return false;
}

//=========================================
// Shared file helper (POSIX)
//=========================================

#if defined( POSIX )

static bool ID_ProcessFile( bloomfilter_t *value, const char *path )
{
    int fd = open( path, O_RDONLY );
    if ( fd < 0 ) return false;

    char buffer[256];
    int ret = (int)read( fd, buffer, sizeof( buffer ) - 1 );
    close( fd );

    if ( ret <= 0 ) return false;
    buffer[ret] = 0;

    if ( !ID_VerifyHEX( buffer ) ) return false;

    *value |= BF_Process( buffer, ret );
    return true;
}

static bool ID_ValidateNetDevice( const char *dev )
{
    if ( strncmp( dev, "ccmni", 5 ) == 0 ||
         strncmp( dev, "ifb",   3 ) == 0 )
        return false;

    char path[256];
    snprintf( path, sizeof( path ), "/sys/class/net/%s/addr_assign_type", dev );

    int fd = open( path, O_RDONLY );
    if ( fd >= 0 )
    {
        char buf[8] = { 0 };
        read( fd, buf, sizeof( buf ) - 1 );
        close( fd );
        if ( atoi( buf ) != 0 )
            return false;
    }
    return true;
}

static int ID_ProcessNetDevices( bloomfilter_t *value )
{
    DIR *dir = opendir( "/sys/class/net" );
    if ( !dir ) return 0;

    int count = 0;
    struct dirent *entry;
    while ( ( entry = readdir( dir ) ) && BF_Weight( *value ) < MAXBITS_GEN )
    {
        if ( !strcmp( entry->d_name, "." ) || !strcmp( entry->d_name, ".." ) )
            continue;
        if ( !ID_ValidateNetDevice( entry->d_name ) )
            continue;

        char path[256];
        snprintf( path, sizeof( path ), "/sys/class/net/%s/address", entry->d_name );
        if ( ID_ProcessFile( value, path ) ) count++;
    }
    closedir( dir );
    return count;
}

static int ID_CheckNetDevices( bloomfilter_t stored )
{
    DIR *dir = opendir( "/sys/class/net" );
    if ( !dir ) return 0;

    int count = 0;
    struct dirent *entry;
    while ( ( entry = readdir( dir ) ) )
    {
        if ( !strcmp( entry->d_name, "." ) || !strcmp( entry->d_name, ".." ) )
            continue;
        if ( !ID_ValidateNetDevice( entry->d_name ) )
            continue;

        char path[256];
        snprintf( path, sizeof( path ), "/sys/class/net/%s/address", entry->d_name );
        bloomfilter_t f = 0;
        if ( ID_ProcessFile( &f, path ) )
            count += ( ( stored & f ) == f ) ? 1 : 0;
    }
    closedir( dir );
    return count;
}

static int ID_ProcessBlockDevices( bloomfilter_t *value )
{
    DIR *dir = opendir( "/sys/block" );
    if ( !dir ) return 0;

    int count = 0;
    struct dirent *entry;
    while ( ( entry = readdir( dir ) ) && BF_Weight( *value ) < MAXBITS_GEN )
    {
        if ( !strcmp( entry->d_name, "." ) || !strcmp( entry->d_name, ".." ) )
            continue;
        char path[256];
        snprintf( path, sizeof( path ), "/sys/block/%s/device/cid", entry->d_name );
        if ( ID_ProcessFile( value, path ) ) count++;
    }
    closedir( dir );
    return count;
}

static int ID_CheckBlockDevices( bloomfilter_t stored )
{
    DIR *dir = opendir( "/sys/block" );
    if ( !dir ) return 0;

    int count = 0;
    struct dirent *entry;
    while ( ( entry = readdir( dir ) ) )
    {
        if ( !strcmp( entry->d_name, "." ) || !strcmp( entry->d_name, ".." ) )
            continue;
        char path[256];
        snprintf( path, sizeof( path ), "/sys/block/%s/device/cid", entry->d_name );
        bloomfilter_t f = 0;
        if ( ID_ProcessFile( &f, path ) )
            count += ( ( stored & f ) == f ) ? 1 : 0;
    }
    closedir( dir );
    return count;
}

static bool ID_ProcessCPUInfo( bloomfilter_t *value )
{
    int fd = open( "/proc/cpuinfo", O_RDONLY );
    if ( fd < 0 ) return false;

    char buffer[1024];
    int ret = (int)read( fd, buffer, sizeof( buffer ) - 1 );
    close( fd );

    if ( ret <= 0 ) return false;
    buffer[ret] = 0;

    char *pbuf = strcasestr( buffer, "Serial" );
    if ( !pbuf ) return false;
    pbuf += 6;

    char *pbuf2 = strchr( pbuf, '\n' );
    if ( pbuf2 ) *pbuf2 = 0;
    else pbuf2 = pbuf + strlen( pbuf );

    if ( !ID_VerifyHEX( pbuf ) ) return false;

    *value |= BF_Process( pbuf, (int)( pbuf2 - pbuf ) );
    return true;
}

#endif // POSIX

//=========================================
// Platform: Windows — WMIC queries
//=========================================

#if defined( _WIN32 )

#define WMIC_BUFSIZE 4096

static bool ID_RunWMIC( char *buffer, const wchar_t *cmdline )
{
    HANDLE hInRd = NULL, hInWr = NULL, hOutRd = NULL, hOutWr = NULL;
    SECURITY_ATTRIBUTES sa = { sizeof( SECURITY_ATTRIBUTES ), NULL, TRUE };
    DWORD dwRead = 0;
    bool bSuccess = false;

    if ( !CreatePipe( &hInRd,  &hInWr,  &sa, 0 ) ) return false;
    if ( !CreatePipe( &hOutRd, &hOutWr, &sa, 0 ) ) goto cleanup;
    SetHandleInformation( hInWr, HANDLE_FLAG_INHERIT, 0 );

    int wlen = (int)( wcslen( cmdline ) + 1 );
    wchar_t *wcopy = (wchar_t *)malloc( wlen * sizeof( wchar_t ) );
    if ( !wcopy ) goto cleanup;
    memcpy( wcopy, cmdline, wlen * sizeof( wchar_t ) );

    STARTUPINFOW si;
    memset( &si, 0, sizeof( si ) );
    si.cb          = sizeof( si );
    si.dwFlags     = STARTF_USESTDHANDLES;
    si.hStdInput   = hInRd;
    si.hStdOutput  = hOutWr;
    si.hStdError   = hOutWr;

    PROCESS_INFORMATION pi;
    memset( &pi, 0, sizeof( pi ) );

    if ( CreateProcessW( NULL, wcopy, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi ) )
    {
        WaitForSingleObject( pi.hProcess, 800 );
        bSuccess = ReadFile( hOutRd, buffer, WMIC_BUFSIZE - 1, &dwRead, NULL ) != FALSE;
        buffer[dwRead] = 0;
        TerminateProcess( pi.hProcess, 0 );
        CloseHandle( pi.hProcess );
        CloseHandle( pi.hThread );
    }
    free( wcopy );

cleanup:
    if ( hInRd  ) CloseHandle( hInRd  );
    if ( hInWr  ) CloseHandle( hInWr  );
    if ( hOutRd ) CloseHandle( hOutRd );
    if ( hOutWr ) CloseHandle( hOutWr );
    return bSuccess;
}

static int ID_ProcessWMIC( bloomfilter_t *value, const wchar_t *cmdline )
{
    char buffer[WMIC_BUFSIZE];
    if ( !ID_RunWMIC( buffer, cmdline ) ) return 0;

    int count = 0;
    char *p = strchr( buffer, '\n' );
    if ( !p ) return 0;
    p++;

    char token[WMIC_BUFSIZE];
    while ( *p )
    {
        while ( *p == ' ' || *p == '\t' || *p == '\r' || *p == '\n' ) p++;
        if ( !*p ) break;

        int i = 0;
        while ( *p && *p != ' ' && *p != '\t' && *p != '\r' && *p != '\n' )
            token[i++] = *p++;
        token[i] = 0;
        if ( !i || !ID_VerifyHEX( token ) ) continue;

        *value |= BF_ProcessStr( token );
        count++;
    }
    return count;
}

static int ID_CheckWMIC( bloomfilter_t stored, const wchar_t *cmdline )
{
    char buffer[WMIC_BUFSIZE];
    if ( !ID_RunWMIC( buffer, cmdline ) ) return 0;

    int count = 0;
    char *p = strchr( buffer, '\n' );
    if ( !p ) return 0;
    p++;

    char token[WMIC_BUFSIZE];
    while ( *p )
    {
        while ( *p == ' ' || *p == '\t' || *p == '\r' || *p == '\n' ) p++;
        if ( !*p ) break;

        int i = 0;
        while ( *p && *p != ' ' && *p != '\t' && *p != '\r' && *p != '\n' )
            token[i++] = *p++;
        token[i] = 0;
        if ( !i || !ID_VerifyHEX( token ) ) continue;

        bloomfilter_t f = BF_ProcessStr( token );
        count += ( ( stored & f ) == f ) ? 1 : 0;
    }
    return count;
}

static int ID_GetRegistryData( HKEY hRoot, const char *subKey, const char *name,
                                char *data, DWORD size )
{
    HKEY hKey;
    if ( RegOpenKeyExA( hRoot, subKey, 0, KEY_QUERY_VALUE, &hKey ) != ERROR_SUCCESS )
        return 0;
    DWORD ret = RegQueryValueExA( hKey, name, NULL, NULL, (LPBYTE)data, &size );
    RegCloseKey( hKey );
    return ( ret == ERROR_SUCCESS ) ? 1 : 0;
}

static void ID_SetRegistryData( HKEY hRoot, const char *subKey, const char *name,
                                 const char *data )
{
    HKEY hKey;
    if ( RegCreateKeyA( hRoot, subKey, &hKey ) != ERROR_SUCCESS ) return;
    RegSetValueExA( hKey, name, 0, REG_SZ, (const BYTE *)data, (DWORD)strlen( data ) + 1 );
    RegCloseKey( hKey );
}

#endif // _WIN32

//=========================================
// Generate raw ID from hardware
//=========================================

static bloomfilter_t ID_GenerateRawId( void )
{
    bloomfilter_t value = 0;

#if defined( _WIN32 )
    ID_ProcessWMIC( &value, L"wmic path win32_physicalmedia get SerialNumber " );
    ID_ProcessWMIC( &value, L"wmic bios get serialnumber " );
#endif

#if defined( POSIX )
#if defined( ANDROID )

    {
        const char *envvars[] = { "ANDROID_ID", "DEVICE_FINGERPRINT", "DEVICE_MODEL", "DEVICE_MANUFACTURER", NULL };
        for ( int i = 0; envvars[i] && BF_Weight( value ) < MAXBITS_GEN; i++ )
        {
            const char *v = getenv( envvars[i] );
            if ( v && v[0] )
                value |= BF_Process( v, (int)strlen( v ) );
        }
    }
#else
    // /etc/machine-id
    {
        int fd = open( "/etc/machine-id", O_RDONLY );
        if ( fd >= 0 )
        {
            char buf[64] = { 0 };
            int r = (int)read( fd, buf, sizeof( buf ) - 1 );
            close( fd );
            if ( r > 0 )
            {
                buf[r] = 0;
                if ( ID_VerifyHEX( buf ) )
                    value |= BF_Process( buf, r );
            }
        }
    }

    ID_ProcessCPUInfo( &value );
    ID_ProcessBlockDevices( &value );
    ID_ProcessNetDevices( &value );
#endif // ANDROID
#endif // POSIX

    return value;
}

//=========================================
// Verify stored ID matches current hardware
//=========================================

static int ID_CheckRawId( bloomfilter_t stored )
{
    int count = 0;

#if defined( _WIN32 )
    count += ID_CheckWMIC( stored, L"wmic path win32_physicalmedia get SerialNumber" );
    count += ID_CheckWMIC( stored, L"wmic bios get serialnumber" );
#endif

#if defined( POSIX )
#if defined( ANDROID )
    // Mirror ID_GenerateRawId
    {
        const char *envvars[] = { "ANDROID_ID", "DEVICE_FINGERPRINT", "DEVICE_MODEL", "DEVICE_MANUFACTURER", NULL };
        for ( int i = 0; envvars[i]; i++ )
        {
            const char *v = getenv( envvars[i] );
            if ( v && v[0] )
            {
                bloomfilter_t f = BF_Process( v, (int)strlen( v ) );
                count += ( ( stored & f ) == f ) ? 1 : 0;
            }
        }
    }
#else
    {
        int fd = open( "/etc/machine-id", O_RDONLY );
        if ( fd >= 0 )
        {
            char buf[64] = { 0 };
            int r = (int)read( fd, buf, sizeof( buf ) - 1 );
            close( fd );
            if ( r > 0 )
            {
                buf[r] = 0;
                if ( ID_VerifyHEX( buf ) )
                {
                    bloomfilter_t f = BF_Process( buf, r );
                    count += ( ( stored & f ) == f ) ? 1 : 0;
                }
            }
        }
    }

    {
        bloomfilter_t f = 0;
        if ( ID_ProcessCPUInfo( &f ) )
            count += ( ( stored & f ) == f ) ? 1 : 0;
    }

    count += ID_CheckBlockDevices( stored );
    count += ID_CheckNetDevices( stored );
#endif // ANDROID
#endif // POSIX

    return count;
}

//=========================================
// Validate: weight check + hardware match
//=========================================

static bool ID_Check( bloomfilter_t id )
{
    if ( !id ) return false;

    int weight = BF_Weight( id );
    if ( weight > MAXBITS_CHECK ) return false;

    int mincount = weight >> 2;
    if ( mincount < 1 ) mincount = 1;

    int hwcount = ID_CheckRawId( id );

    // Hardware is readable and matches
    if ( hwcount >= mincount ) return true;

    // Hardware didn't fully match.
    bloomfilter_t hw = ID_GenerateRawId();
    if ( hw == 0 ) return true;  // no hardware

    return false;
}

//=========================================
// File I/O helpers
//=========================================

static void ID_WriteFile( const char *path, bloomfilter_t id, uint64_t mask )
{
    FILE *f = fopen( path, "w" );
    if ( !f ) return;
    fprintf( f, "%016llX", (unsigned long long)( id ^ mask ) );
    fclose( f );
}

static bloomfilter_t ID_ReadFile( const char *path, uint64_t mask )
{
    FILE *f = fopen( path, "r" );
    if ( !f ) return 0;

    unsigned long long raw = 0;
    int ok = fscanf( f, "%016llX", &raw );
    fclose( f );

    if ( ok != 1 ) return 0;
    return (bloomfilter_t)raw ^ mask;
}

//=========================================
// Build path to <com_gamedir>/.csso_id
//=========================================

static void ID_GetGameFilePath( char *out, int outSize )
{
    if ( com_gamedir[0] != '\0' )
        Q_snprintf( out, outSize, "%s/.csso_id", com_gamedir );
    else
        Q_strncpy( out, ".csso_id", outSize );
}

//=========================================
// ID_Init — called once from Host_Init
//=========================================

void ID_Init( void )
{
    bloomfilter_t id = 0;

    char gameFilePath[MAX_OSPATH];
    ID_GetGameFilePath( gameFilePath, sizeof( gameFilePath ) );

    //------------------------------------
    // 1. Try platform-specific system storage
    //------------------------------------

#if defined( _WIN32 )
    {
        char buf[64] = { 0 };
        if ( ID_GetRegistryData( HKEY_CURRENT_USER,
                                  "Software\\Valve\\Steam\\csso_id",
                                  "csso_id", buf, sizeof( buf ) ) )
        {
            unsigned long long raw = 0;
            if ( sscanf( buf, "%016llX", &raw ) == 1 )
                id = (bloomfilter_t)raw ^ SYSTEM_XOR_MASK;
        }
    }
#elif defined( ANDROID )
    // Android primary storage: APP_DATA_PATH/.csso_id
    {
        const char *appdata = getenv( "APP_DATA_PATH" );
        if ( appdata && appdata[0] )
        {
            char path[512];
            snprintf( path, sizeof( path ), "%s/.csso_id", appdata );
            id = ID_ReadFile( path, SYSTEM_XOR_MASK );
        }
    }
#elif defined( POSIX )
    // Desktop Linux: ~/.config/.csso_id  (XDG)
    {
        const char *home = getenv( "HOME" );
        if ( home && home[0] != '\0' )
        {
            char path[512];

            snprintf( path, sizeof( path ), "%s/.config/.csso_id", home );
            id = ID_ReadFile( path, SYSTEM_XOR_MASK );

            if ( !id )
            {
                snprintf( path, sizeof( path ), "%s/.local/.csso_id", home );
                id = ID_ReadFile( path, SYSTEM_XOR_MASK );
            }

            if ( !id )
            {
                snprintf( path, sizeof( path ), "%s/.csso_id", home );
                id = ID_ReadFile( path, SYSTEM_XOR_MASK );
            }
        }
    }
#endif

    if ( id && !ID_Check( id ) )
        id = 0;

    //------------------------------------
    // 2. Game directory fallback
    //    <com_gamedir>/.csso_id  (all platforms)
    //------------------------------------

    if ( !id )
    {
        id = ID_ReadFile( gameFilePath, GAME_XOR_MASK );
        if ( id && !ID_Check( id ) )
            id = 0;
    }

    //------------------------------------
    // 3. Generate from hardware
    //------------------------------------

    if ( !id )
        id = ID_GenerateRawId();

    //------------------------------------
    // 4. Random fallback — used on devices
    //------------------------------------

    if ( !id )
    {
        char seed[64];
        uint32_t t  = (uint32_t)time( NULL );
        uint32_t r1 = (uint32_t)(uintptr_t)&id ^ t;
        uint32_t r2 = r1 * 1664525u + 1013904223u;   // LCG step 1
        uint32_t r3 = r2 * 1664525u + 1013904223u;   // LCG step 2

        for ( int i = 0; i < 16 && BF_Weight( id ) < MAXBITS_GEN; i++ )
        {
            Q_snprintf( seed, sizeof( seed ),
                        "rnd%d_%08X_%08X_%08X", i, r1 ^ (uint32_t)i, r2 + i, r3 ^ (r1 << i) );
            id |= BF_ProcessStr( seed );
        }
        if ( !id ) id = BF_ProcessStr( "css_fallback_id" ); // absolute last resort
    }

    s_id = id;

    //------------------------------------
    // 5. Persist to all storage locations
    //------------------------------------

#if defined( _WIN32 )
    {
        char buf[64];
        snprintf( buf, sizeof( buf ), "%016llX", (unsigned long long)( id ^ SYSTEM_XOR_MASK ) );
        ID_SetRegistryData( HKEY_CURRENT_USER,
                             "Software\\Valve\\Steam\\csso_id",
                             "csso_id", buf );
    }
#elif defined( ANDROID )
    // Save to APP_DATA_PATH/.csso_id — survives game-directory wipes.
    {
        const char *appdata = getenv( "APP_DATA_PATH" );
        if ( appdata && appdata[0] )
        {
            char path[512];
            snprintf( path, sizeof( path ), "%s/.csso_id", appdata );
            ID_WriteFile( path, id, SYSTEM_XOR_MASK );
        }
    }
#elif defined( POSIX )
    {
        const char *home = getenv( "HOME" );
        if ( home && home[0] != '\0' )
        {
            char dir[512];
            char path[512];
            bool saved = false;

            // Ensure ~/.config exists — fopen(..., "w") fails silently
            snprintf( dir, sizeof( dir ), "%s/.config", home );
            mkdir( dir, 0700 );

            snprintf( path, sizeof( path ), "%s/.config/.csso_id", home );
            {
                FILE *f = fopen( path, "w" );
                if ( f )
                {
                    fprintf( f, "%016llX", (unsigned long long)( id ^ SYSTEM_XOR_MASK ) );
                    fclose( f );
                    saved = true;
                }
            }
            if ( !saved )
            {
                snprintf( dir, sizeof( dir ), "%s/.local", home );
                mkdir( dir, 0700 );

                snprintf( path, sizeof( path ), "%s/.local/.csso_id", home );
                FILE *f = fopen( path, "w" );
                if ( f )
                {
                    fprintf( f, "%016llX", (unsigned long long)( id ^ SYSTEM_XOR_MASK ) );
                    fclose( f );
                    saved = true;
                }
            }
            if ( !saved )
            {
                snprintf( path, sizeof( path ), "%s/.csso_id", home );
                FILE *f = fopen( path, "w" );
                if ( f )
                {
                    fprintf( f, "%016llX", (unsigned long long)( id ^ SYSTEM_XOR_MASK ) );
                    fclose( f );
                }
            }
        }
    }
#endif

    ID_WriteFile( gameFilePath, id, GAME_XOR_MASK );

    //------------------------------------
    // 6. Build output string
    //------------------------------------

    Q_snprintf( s_idString, sizeof( s_idString ),
                "CSSO_ID_%016llX", (unsigned long long)id );
}

//=========================================
// ID_GetUniqueIDString
//=========================================

const char *ID_GetUniqueIDString( void )
{
    if ( s_idString[0] == '\0' )
        return "STEAM_ID_PENDING";

    return s_idString;
}
