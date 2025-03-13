#include <vgui_controls/ProgressBar.h>

namespace vgui
{

//-----------------------------------------------------------------------------
// Purpose: Overriding Paint method to allow for correct border rendering
//-----------------------------------------------------------------------------
class ContinuousProgressBarWithBorder: public ContinuousProgressBar
{
	DECLARE_CLASS_SIMPLE( ContinuousProgressBarWithBorder, ContinuousProgressBar );

public:
	ContinuousProgressBarWithBorder( Panel *parent, const char *panelName );
	virtual void Paint();
};

}