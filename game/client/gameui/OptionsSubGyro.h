#ifndef OPTIONSUBGYROMENU_H
#define OPTIONSUBGYROMENU_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/PropertyPage.h>
#include <vgui_controls/Label.h>

class CCvarToggleCheckButton;
class CCvarSlider;

class ClientModGyroMenu : public vgui::PropertyPage
{
        DECLARE_CLASS_SIMPLE( ClientModGyroMenu, vgui::PropertyPage );

public:
        ClientModGyroMenu( vgui::Panel *parent );
        ~ClientModGyroMenu();

protected:
        virtual void OnResetData();
        virtual void OnApplyChanges();
        MESSAGE_FUNC( OnControlModified,       "ControlModified"      );
        MESSAGE_FUNC_PTR( OnTextChanged,       "TextChanged", panel   );
        MESSAGE_FUNC_PARAMS( OnSliderMoved,    "SliderMoved", data    );
        MESSAGE_FUNC( OnCheckButtonChecked,    "CheckButtonChecked"   );

private:
        CCvarToggleCheckButton  *m_pGyroEnable;
        CCvarToggleCheckButton  *m_pGyroReversePitch;
        CCvarToggleCheckButton  *m_pGyroReverseYaw;

        CCvarSlider             *m_pGyroPitchSensitivity;
        CCvarSlider             *m_pGyroYawSensitivity;
        CCvarSlider             *m_pGyroScopedSensitivity;
        CCvarSlider             *m_pGyroDeadzone;
};

#endif // OPTIONSUBGYROMENU_H
