#include "inputsystem.h"
#include "tier0/icommandline.h"

#if defined( USE_SDL )

#undef M_PI
#include "SDL.h"
#include "SDL_sensor.h"

// NOTE: This has to be the last file included!
#include "tier0/memdbgon.h"

// Internal cast helper — only used within this file.
#define GYRO_SENSOR()  ( reinterpret_cast<SDL_Sensor *>( m_pGyroSensor ) )

static int GyroSDLWatcher( void *userInfo, SDL_Event *event )
{
        if ( event->type != SDL_SENSORUPDATE )
                return 1;   // not a sensor event

        CInputSystem *pInputSystem = static_cast<CInputSystem *>( userInfo );
        if ( !pInputSystem || !pInputSystem->IsGyroAvailable() )
                return 1;

        const float RAD_TO_DEG = 180.0f / M_PI;
        
        pInputSystem->SetGyroAngularVelocity(
                event->sensor.data[1] * RAD_TO_DEG,    // pitch <- Y axis
                event->sensor.data[0] * RAD_TO_DEG );  // yaw   <- X axis

        return 1;
}

void CInputSystem::InitializeGyro( void )
{
        m_pGyroSensor       = NULL;
        m_bGyroInitialized  = false;
        m_flGyroAngVelPitch = 0.0f;
        m_flGyroAngVelYaw   = 0.0f;

        if ( CommandLine()->FindParm( "-nogyro" ) )
                return;

        if ( SDL_InitSubSystem( SDL_INIT_SENSOR ) != 0 )
        {
                DevMsg( "Gyro: SDL_InitSubSystem(SDL_INIT_SENSOR) failed: %s\n", SDL_GetError() );
                return;
        }

        const int numSensors = SDL_NumSensors();
        for ( int i = 0; i < numSensors; ++i )
        {
                SDL_Sensor *pCandidate = SDL_SensorOpen( i );
                if ( !pCandidate )
                        continue;

                if ( SDL_SensorGetType( pCandidate ) == SDL_SENSOR_GYRO )
                {
                        m_pGyroSensor      = pCandidate;
                        m_bGyroInitialized = true;
                        SDL_AddEventWatch( GyroSDLWatcher, this );
                        DevMsg( "Gyro: opened sensor '%s'\n", SDL_SensorGetName( pCandidate ) );
                        break;
                }

                // Not a gyroscope; close and try the next one.
                SDL_SensorClose( pCandidate );
        }

        if ( !m_bGyroInitialized )
        {
                DevMsg( "Gyro: no gyroscope sensor found on this device.\n" );
                SDL_QuitSubSystem( SDL_INIT_SENSOR );
        }
}

void CInputSystem::ShutdownGyro( void )
{
        if ( !m_bGyroInitialized )
                return;

        SDL_DelEventWatch( GyroSDLWatcher, this );

        if ( m_pGyroSensor )
        {
                SDL_SensorClose( GYRO_SENSOR() );
                m_pGyroSensor = NULL;
        }

        SDL_QuitSubSystem( SDL_INIT_SENSOR );
        m_bGyroInitialized = false;
}

bool CInputSystem::GetGyroAccumulators( float &pitch, float &yaw )
{
        if ( !m_bGyroInitialized )
                return false;

        pitch = m_flGyroAngVelPitch;
        yaw   = m_flGyroAngVelYaw;

        // Clear after read so the same snapshot is not applied next frame.
        m_flGyroAngVelPitch = 0.0f;
        m_flGyroAngVelYaw   = 0.0f;

        return true;
}

void CInputSystem::SetGyroAngularVelocity( float pitch_dps, float yaw_dps )
{
        m_flGyroAngVelPitch = pitch_dps;
        m_flGyroAngVelYaw   = yaw_dps;
}

#else // !USE_SDL

void CInputSystem::InitializeGyro( void )
{
        m_pGyroSensor       = NULL;
        m_bGyroInitialized  = false;
        m_flGyroAngVelPitch = 0.0f;
        m_flGyroAngVelYaw   = 0.0f;
}

void CInputSystem::ShutdownGyro( void ) {}

bool CInputSystem::GetGyroAccumulators( float &pitch, float &yaw )
{
        pitch = yaw = 0.0f;
        return false;
}

void CInputSystem::SetGyroAngularVelocity( float, float ) {}

#endif // USE_SDL
