#include "OptionsSubGyro.h"
#include <stdio.h>

#include <vgui_controls/Button.h>
#include <vgui_controls/Label.h>
#include "tier1/KeyValues.h"

#include "CvarToggleCheckButton.h"
#include "cvarslider.h"
#include "tier1/convar.h"

#include <tier0/memdbgon.h>

using namespace vgui;

ClientModGyroMenu::ClientModGyroMenu( vgui::Panel *parent )
        : vgui::PropertyPage( parent, "ClientModGyroMenu" )
{
        m_pGyroEnable = new CCvarToggleCheckButton(
                this, "GyroEnableCheckBox",
                "#GameUI_Gyro_Enable", "gyro_enable" );

        m_pGyroReversePitch = new CCvarToggleCheckButton(
                this, "GyroReversePitchCheckBox",
                "#GameUI_Gyro_ReversePitch", "gyro_reverse_pitch" );

        m_pGyroReverseYaw = new CCvarToggleCheckButton(
                this, "GyroReverseYawCheckBox",
                "#GameUI_Gyro_ReverseYaw", "gyro_reverse_yaw" );

        m_pGyroPitchSensitivity = new CCvarSlider(
                this, "GyroPitchSensitivitySlider",
                "#GameUI_Gyro_PitchSensitivity",
                0.0f, 2.0f, "gyro_pitch_sensitivity" );

        m_pGyroYawSensitivity = new CCvarSlider(
                this, "GyroYawSensitivitySlider",
                "#GameUI_Gyro_YawSensitivity",
                0.0f, 2.0f, "gyro_yaw_sensitivity" );

        m_pGyroScopedSensitivity = new CCvarSlider(
                this, "GyroScopedSensitivitySlider",
                "#GameUI_Gyro_ScopedSensitivity",
                0.0f, 2.0f, "gyro_scoped_sensitivity" );

        m_pGyroDeadzone = new CCvarSlider(
                this, "GyroDeadzoneSlider",
                "#GameUI_Gyro_Deadzone",
                0.0f, 3.0f, "gyro_deadzone" );

        m_pGyroEnable->AddActionSignalTarget( this );
        m_pGyroReversePitch->AddActionSignalTarget( this );
        m_pGyroReverseYaw->AddActionSignalTarget( this );
        m_pGyroPitchSensitivity->AddActionSignalTarget( this );
        m_pGyroYawSensitivity->AddActionSignalTarget( this );
        m_pGyroScopedSensitivity->AddActionSignalTarget( this );
        m_pGyroDeadzone->AddActionSignalTarget( this );

        LoadControlSettings( "Resource/OptionsSubGyro.res" );
}

ClientModGyroMenu::~ClientModGyroMenu()
{
}

void ClientModGyroMenu::OnControlModified()
{
        PostMessage( GetParent(), new KeyValues( "ApplyButtonEnable" ) );
        InvalidateLayout();
}

void ClientModGyroMenu::OnTextChanged( vgui::Panel *panel )
{
        OnControlModified();
}

void ClientModGyroMenu::OnSliderMoved( KeyValues *data )
{
        OnControlModified();
}

void ClientModGyroMenu::OnCheckButtonChecked()
{
        OnControlModified();
}

void ClientModGyroMenu::OnResetData()
{
        m_pGyroEnable->Reset();
        m_pGyroReversePitch->Reset();
        m_pGyroReverseYaw->Reset();
        m_pGyroPitchSensitivity->Reset();
        m_pGyroYawSensitivity->Reset();
        m_pGyroScopedSensitivity->Reset();
        m_pGyroDeadzone->Reset();
}

void ClientModGyroMenu::OnApplyChanges()
{
        m_pGyroEnable->ApplyChanges();
        m_pGyroReversePitch->ApplyChanges();
        m_pGyroReverseYaw->ApplyChanges();
        m_pGyroPitchSensitivity->ApplyChanges();
        m_pGyroYawSensitivity->ApplyChanges();
        m_pGyroScopedSensitivity->ApplyChanges();
        m_pGyroDeadzone->ApplyChanges();
}
