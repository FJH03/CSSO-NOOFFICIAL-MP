///////////////////////////////////////////////////////////
// Tracker scheme resource file
//
// sections:
//		Colors			- all the colors used by the scheme
//		BaseSettings	- contains settings for app to use to draw controls
//		Fonts			- list of all the fonts used by app
//		Borders			- description of all the borders
//
///////////////////////////////////////////////////////////
#base "ClientScheme.res"
Scheme
{
	//////////////////////// COLORS ///////////////////////////
	// color details
	// this is a list of all the colors used by the scheme
	Colors
	{
		"TextDefault"			"255 255 255 255"
		"TextTeamT"				"255 223 147 255"
		"TextTeamCT"			"162 198 255 255"
		"TextTeamUnassigned"	"255 255 255 255"
		"TextLocation"			"64 255 64 255"
	}

	///////////////////// BASE SETTINGS ////////////////////////
	//
	// default settings for all panels
	// controls use these to determine their settings
	BaseSettings
	{
		// vgui_controls color specifications
		Border.Bright					"0 0 0 0"	// the lit side of a control
		Border.Dark						"0 0 0 0"	// the dark/unlit side of a control
		Border.Selection				"0 0 0 0"	// the additional border color for displaying the default/selected button

		Button.TextColor				"160 160 160 255"
		Button.ArmedTextColor			"White"
		Button.BgColor					"0 0 0 0"
		Button.ArmedBgColor				"0 0 0 0"
		Button.SelectedBgColor			"0 0 0 0"
		Button.DepressedBgColor			"0 0 0 0"
		Button.DisabledBgColor			"0 0 0 0"

		Panel.FgColor					"WhiteDim"
		Panel.BgColor					"0 0 0 192"

		RichText.BgColor				"0 0 0 153"
		RichText.SelectedBgColor		"50 129 172 64"
	
		TextEntry.SelectedTextColor		"White"
		TextEntry.SelectedBgColor		"50 129 172 64"
	}

	//
	//////////////////////// FONTS /////////////////////////////
	//
	// describes all the fonts
	Fonts
	{
		// fonts are used in order that they are listed
		// fonts listed later in the order will only be used if they fulfill a range not already filled
		// if a font fails to load then the subsequent fonts will replace
		"ChatSendFont"
		{
			"1"
			{
				"name"		"StratumNo2"
				"tall"		"7"
				"weight"	"0"
				"antialias"	"1"
			}
		}
	}
}
