//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Gyroscope input support for Android devices using SDL2
//
//=============================================================================//

#include "cbase.h"
#include "hud.h"
#include "gyroscope.h"

#ifdef __ANDROID__

// FjH_03: Use SDL2 sensor API instead of direct NDK calls
#include <SDL.h>
#endif

#include <math.h>

//-----------------------------------------------------------------------------
// CVARs
//-----------------------------------------------------------------------------
ConVar gyroscope( "gyroscope", "0", FCVAR_ARCHIVE, "Enable gyroscope input on Android" );
ConVar gyroscope_sensitivity( "gyroscope_sensitivity", "1.0", FCVAR_ARCHIVE, "Gyroscope sensitivity multiplier" );
ConVar gyroscope_reverse_x( "gyroscope_reverse_x", "0", FCVAR_ARCHIVE, "Reverse gyroscope yaw (left/right)" );
ConVar gyroscope_reverse_y( "gyroscope_reverse_y", "0", FCVAR_ARCHIVE, "Reverse gyroscope pitch (top/bottom)" );

#ifdef __ANDROID__

//-----------------------------------------------------------------------------
// SDL2 Gyroscope Implementation
//-----------------------------------------------------------------------------

static SDL_Sensor *g_pSDLSensor = NULL;
static float g_flGyroYaw = 0.0f;
static float g_flGyroPitch = 0.0f;
static bool g_bGyroInitialized = false;
static bool g_bGyroSensorEnabled = false;
static float g_flSmoothYaw = 0.0f;
static float g_flSmoothPitch = 0.0f;

// Constants
static const float GYRO_RAD2DEG = 57.2957795f;
static const float GYRO_MIN_DEADZONE = 0.0001f;

//-----------------------------------------------------------------------------
// Enable gyroscope sensor
//-----------------------------------------------------------------------------
static void Gyro_EnableSensor( void )
{
	if ( !g_bGyroInitialized || !g_pSDLSensor )
		return;
	
	if ( !g_bGyroSensorEnabled )
	{
		g_bGyroSensorEnabled = true;
		g_flGyroYaw = 0.0f;
		g_flGyroPitch = 0.0f;
		g_flSmoothYaw = 0.0f;
		g_flSmoothPitch = 0.0f;
	}
}

//-----------------------------------------------------------------------------
// Disable gyroscope sensor
//-----------------------------------------------------------------------------
static void Gyro_DisableSensor( void )
{
	if ( !g_bGyroInitialized || !g_pSDLSensor )
		return;
	
	if ( g_bGyroSensorEnabled )
	{
		SDL_SensorClose( g_pSDLSensor );
		g_bGyroSensorEnabled = false;
		g_flGyroYaw = 0.0f;
		g_flGyroPitch = 0.0f;
		g_flSmoothYaw = 0.0f;
		g_flSmoothPitch = 0.0f;
	}
}

//-----------------------------------------------------------------------------
// Update gyroscope data from SDL
//-----------------------------------------------------------------------------
static void Gyro_ReadSensor( void )
{
	if ( !g_bGyroSensorEnabled || !g_pSDLSensor )
		return;

	float data[4];
	if ( SDL_SensorGetData( g_pSDLSensor, data, 4 ) < 0 )
		return;

	// SDL gyro data is in rad/s
	float gyroX = data[0];
	float gyroY = data[1];

	// Apply axis mapping
	float rawYaw   = -gyroX;
	float rawPitch =  gyroY;

	// Apply reversal CVARs
	if ( gyroscope_reverse_x.GetBool() )
		rawYaw = -rawYaw;
	
	if ( gyroscope_reverse_y.GetBool() )
		rawPitch = -rawPitch;

	// Apply deadzone
	if ( fabsf( rawYaw ) < GYRO_MIN_DEADZONE )
		rawYaw = 0.0f;
	if ( fabsf( rawPitch ) < GYRO_MIN_DEADZONE )
		rawPitch = 0.0f;

	// Get sensitivity
	float sens = gyroscope_sensitivity.GetFloat();
	if ( sens <= 0.0f )
		sens = 1.0f;

	// Apply exponential sensitivity curve
	float expo = sens * ( 1.0f + sens * 0.25f );

	// Assume ~16ms frame time for smoothing
	float dT = 0.016f;
	float scaledYaw   = rawYaw   * dT * expo * GYRO_RAD2DEG;
	float scaledPitch = rawPitch * dT * expo * GYRO_RAD2DEG;

	// Bypass smoothing for responsive real gyroscope
        g_flSmoothYaw   = scaledYaw;
        g_flSmoothPitch = scaledPitch;

	// Accumulate deltas
	g_flGyroYaw   += g_flSmoothYaw;
	g_flGyroPitch += g_flSmoothPitch;
}

#endif // __ANDROID__

//-----------------------------------------------------------------------------
// Initialize gyroscope
//-----------------------------------------------------------------------------
void Gyro_Init( void )
{
#ifdef __ANDROID__
	// Initialize SDL sensor subsystem
	if ( SDL_InitSubSystem( SDL_INIT_SENSOR ) < 0 )
	{
		Msg( "Gyroscope: Failed to initialize SDL sensor subsystem: %s\n", SDL_GetError() );
		return;
	}

	// Try to open the first gyroscope sensor
	int sensorCount = SDL_NumSensors();
	for ( int i = 0; i < sensorCount; i++ )
	{
		SDL_SensorType type = SDL_SensorGetDeviceType( i );
		if ( type == SDL_SENSOR_GYRO )
		{
			g_pSDLSensor = SDL_SensorOpen( i );
			if ( g_pSDLSensor )
			{
				g_bGyroInitialized = true;
				g_bGyroSensorEnabled = false;
				g_flGyroYaw = 0.0f;
				g_flGyroPitch = 0.0f;
				g_flSmoothYaw = 0.0f;
				g_flSmoothPitch = 0.0f;
				
				Msg( "Gyroscope: Initialized successfully (SDL sensor %d)\n", i );
				return;
			}
		}
	}

	Msg( "Gyroscope: No gyroscope sensor found\n" );
	SDL_QuitSubSystem( SDL_INIT_SENSOR );
#else
	Msg( "Gyroscope: Not available on this platform\n" );
#endif
}

//-----------------------------------------------------------------------------
// Shutdown gyroscope
//-----------------------------------------------------------------------------
void Gyro_Shutdown( void )
{
#ifdef __ANDROID__
	Gyro_DisableSensor();
	
	if ( g_bGyroInitialized )
	{
		SDL_QuitSubSystem( SDL_INIT_SENSOR );
		g_bGyroInitialized = false;
	}
	
	Msg( "Gyroscope: Shutdown\n" );
#endif
}

//-----------------------------------------------------------------------------
// Update gyroscope state and get accumulated deltas
//-----------------------------------------------------------------------------
void Gyro_Update( float *yaw, float *pitch )
{
#ifdef __ANDROID__
	if ( !g_bGyroInitialized )
	{
		if ( yaw )   *yaw = 0.0f;
		if ( pitch ) *pitch = 0.0f;
		return;
	}

	// Check if gyroscope should be enabled
	bool bShouldEnable = gyroscope.GetBool();

	if ( bShouldEnable && !g_bGyroSensorEnabled )
	{
		Gyro_EnableSensor();
	}
	else if ( !bShouldEnable && g_bGyroSensorEnabled )
	{
		Gyro_DisableSensor();
	}

	if ( !g_bGyroSensorEnabled )
	{
		g_flSmoothYaw = g_flSmoothPitch = 0.0f;
		g_flGyroYaw = g_flGyroPitch = 0.0f;
		if ( yaw )   *yaw = 0.0f;
		if ( pitch ) *pitch = 0.0f;
		return;
	}

	// Read sensor data
	Gyro_ReadSensor();

	// Output accumulated deltas
	if ( yaw )   *yaw = g_flGyroYaw;
	if ( pitch ) *pitch = g_flGyroPitch;

	// Reset accumulators (deltas are consumed)
	g_flGyroYaw = 0.0f;
	g_flGyroPitch = 0.0f;
#else
	if ( yaw )   *yaw = 0.0f;
	if ( pitch ) *pitch = 0.0f;
#endif
}

//-----------------------------------------------------------------------------
// Reset gyroscope accumulators
//-----------------------------------------------------------------------------
void Gyro_Reset( void )
{
#ifdef __ANDROID__
	g_flGyroYaw = 0.0f;
	g_flGyroPitch = 0.0f;
	g_flSmoothYaw = 0.0f;
	g_flSmoothPitch = 0.0f;
#endif
}

//-----------------------------------------------------------------------------
// Check if gyroscope is enabled
//-----------------------------------------------------------------------------
int Gyro_IsEnabled( void )
{
#ifdef __ANDROID__
	if ( !g_bGyroInitialized )
		return 0;
	return gyroscope.GetBool() ? 1 : 0;
#else
	return 0;
#endif
}
