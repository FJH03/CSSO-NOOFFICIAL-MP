//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Gyroscope input support for Android devices
//
//=============================================================================//

#include "cbase.h"
#include "hud.h"
#include "gyroscope.h"

#ifdef __ANDROID__
#include <android/sensor.h>
#include <android/looper.h>
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
// Android Gyroscope Implementation
//-----------------------------------------------------------------------------

static ASensorManager *g_pSensorManager = NULL;
static ASensorEventQueue *g_pSensorEventQueue = NULL;
static const ASensor *g_pGyroSensor = NULL;
static ALooper *g_pLooper = NULL;

static float g_flGyroYaw = 0.0f;
static float g_flGyroPitch = 0.0f;
static int64_t g_iLastTimestamp = 0;
static bool g_bGyroInitialized = false;
static bool g_bGyroSensorEnabled = false;
static bool g_bFirstEvent = true;
static float g_flSmoothYaw = 0.0f;
static float g_flSmoothPitch = 0.0f;

// Constants
static const float GYRO_RAD2DEG = 57.2957795f;
static const float GYRO_MIN_DEADZONE = 0.010f;
static const float NS2S = 1.0f / 1000000000.0f;

//-----------------------------------------------------------------------------
// Enable gyroscope sensor
//-----------------------------------------------------------------------------
static void Gyro_EnableSensor( void )
{
	if ( !g_bGyroInitialized || !g_pSensorEventQueue || !g_pGyroSensor )
		return;
	
	if ( !g_bGyroSensorEnabled )
	{
		ASensorEventQueue_enableSensor( g_pSensorEventQueue, g_pGyroSensor );
		ASensorEventQueue_setEventRate( g_pSensorEventQueue, g_pGyroSensor, 16666 ); // 60Hz
		g_bGyroSensorEnabled = true;
		g_bFirstEvent = true;
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
	if ( !g_bGyroInitialized || !g_pSensorEventQueue || !g_pGyroSensor )
		return;
	
	if ( g_bGyroSensorEnabled )
	{
		ASensorEventQueue_disableSensor( g_pSensorEventQueue, g_pGyroSensor );
		g_bGyroSensorEnabled = false;
		g_flGyroYaw = 0.0f;
		g_flGyroPitch = 0.0f;
		g_flSmoothYaw = 0.0f;
		g_flSmoothPitch = 0.0f;
	}
}

//-----------------------------------------------------------------------------
// Sensor event callback
//-----------------------------------------------------------------------------
static int Gyro_SensorCallback( int fd, int events, void *data )
{
	ASensorEvent event;

	while ( ASensorEventQueue_getEvents( g_pSensorEventQueue, &event, 1 ) > 0 )
	{
		if ( event.type != ASENSOR_TYPE_GYROSCOPE )
			continue;

		// Skip first event (initialization)
		if ( g_bFirstEvent )
		{
			g_iLastTimestamp = event.timestamp;
			g_bFirstEvent = false;
			continue;
		}

		// Calculate delta time
		float dT = ( float )( event.timestamp - g_iLastTimestamp ) * NS2S;
		g_iLastTimestamp = event.timestamp;

		if ( dT <= 0.0f || dT > 0.5f )
			continue;

		// Get raw gyro data (angular velocity in rad/s)
		float gyroX = event.data[0];
		float gyroY = event.data[1];

		// Apply axis mapping and reversal
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

		// Get sensitivity from CVAR
		float sens = gyroscope_sensitivity.GetFloat();
		if ( sens <= 0.0f )
			sens = 1.0f;

		// Apply exponential sensitivity curve
		float expo = sens * ( 1.0f + sens * 0.25f );

		// Scale by delta time and convert to degrees
		float scaledYaw   = rawYaw   * dT * expo * GYRO_RAD2DEG;
		float scaledPitch = rawPitch * dT * expo * GYRO_RAD2DEG;

		// Adaptive smoothing based on movement magnitude
		float mag = fmaxf( fabsf( scaledYaw ), fabsf( scaledPitch ) );
		float baseAlpha = dT * 140.0f;

		if ( baseAlpha < 0.12f )
			baseAlpha = 0.12f;
		if ( baseAlpha > 0.30f )
			baseAlpha = 0.30f;

		float alpha = baseAlpha + mag * 0.40f;
		if ( alpha > 0.85f )
			alpha = 0.85f;

		// Apply smoothing
		g_flSmoothYaw   = g_flSmoothYaw   * ( 1.0f - alpha ) + scaledYaw   * alpha;
		g_flSmoothPitch = g_flSmoothPitch * ( 1.0f - alpha ) + scaledPitch * alpha;

		// Accumulate deltas
		g_flGyroYaw   += g_flSmoothYaw;
		g_flGyroPitch += g_flSmoothPitch;
	}

	return 1;
}

#endif // __ANDROID__

//-----------------------------------------------------------------------------
// Initialize gyroscope
//-----------------------------------------------------------------------------
void Gyro_Init( void )
{
#ifdef __ANDROID__
	// Get sensor manager
	g_pSensorManager = ASensorManager_getInstance();
	if ( !g_pSensorManager )
	{
		Msg( "Gyroscope: Failed to get sensor manager\n" );
		return;
	}
	
	// Get gyroscope sensor
	g_pGyroSensor = ASensorManager_getDefaultSensor( g_pSensorManager, ASENSOR_TYPE_GYROSCOPE );
	if ( !g_pGyroSensor )
	{
		Msg( "Gyroscope: Gyroscope sensor not available\n" );
		return;
	}
	
	// Get or create looper
	g_pLooper = ALooper_forThread();
	if ( !g_pLooper )
	{
		g_pLooper = ALooper_prepare( ALOOPER_PREPARE_ALLOW_NON_CALLBACKS );
	}
	
	if ( !g_pLooper )
	{
		Msg( "Gyroscope: Failed to get looper\n" );
		return;
	}
	
	// Create event queue
	g_pSensorEventQueue = ASensorManager_createEventQueue(
		g_pSensorManager,
		g_pLooper,
		ALOOPER_POLL_CALLBACK,
		Gyro_SensorCallback,
		NULL
	);
	
	if ( !g_pSensorEventQueue )
	{
		Msg( "Gyroscope: Failed to create event queue\n" );
		return;
	}
	
	g_bGyroInitialized = true;
	g_bGyroSensorEnabled = false;
	g_bFirstEvent = true;
	g_flGyroYaw = 0.0f;
	g_flGyroPitch = 0.0f;
	g_flSmoothYaw = 0.0f;
	g_flSmoothPitch = 0.0f;
	
	Msg( "Gyroscope: Initialized successfully\n" );
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
	
	if ( g_pSensorManager && g_pSensorEventQueue )
	{
		ASensorManager_destroyEventQueue( g_pSensorManager, g_pSensorEventQueue );
	}
	
	g_pSensorEventQueue = NULL;
	g_pGyroSensor = NULL;
	g_pSensorManager = NULL;
	g_pLooper = NULL;
	g_bGyroInitialized = false;
	
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

	// Process pending sensor events
	if ( g_pLooper )
	{
		ALooper_pollOnce( 0, NULL, NULL, NULL );
	}

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
	g_bFirstEvent = true;
	g_iLastTimestamp = 0;
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
