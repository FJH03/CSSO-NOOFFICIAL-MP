"resource/hud/teamcounter.res"
{	
	"RoundTimerLabel"
	{
		"fieldName"			"RoundTimerLabel"
		"xpos"				"111"
		"ypos"				"0"
		"wide"				"84"
		"tall"				"37"
		"base_resolution_wide" "1920"
		"base_resolution_tall" "1080"
		"textAlignment"		"center"
		"font"				"RoundTimerFont"
		"fgcolor_override"	"White"
		
		"bgcolor_override"	"0 0 0 153"
		"PaintBackgroundType"	"0"
	}
	
	"BombIcon"
	{
		"fieldName"			"BombIcon"
		"xpos"				"135"
		"ypos"				"-2"
		"wide"				"42"
		"tall"				"0" // computed procedurally
		"base_resolution_wide" "1920"
		"base_resolution_tall" "1080"
		
		"image"				"materials/vgui/hud/svg/bomb_c4.svg"
	}
	
	"CTWinCounterLabel"
	{
		"fieldName"			"CTWinCounterLabel"
		"xpos"				"111"
		"ypos"				"40"
		"wide"				"40"
		"tall"				"31"
		"base_resolution_wide" "1920"
		"base_resolution_tall" "1080"
		"textAlignment"		"center"
		"font"				"RoundTimerFont"
		"fgcolor_override"	"TeamCT"
		
		"bgcolor_override"	"0 0 0 153"
		"PaintBackgroundType"	"0"
	}

	"TWinCounterLabel"
	{
		"fieldName"			"TWinCounterLabel"
		"xpos"				"153"
		"ypos"				"40"
		"wide"				"41"
		"tall"				"31"
		"base_resolution_wide" "1920"
		"base_resolution_tall" "1080"
		"textAlignment"		"center"
		"font"				"RoundTimerFont"
		"fgcolor_override"	"TeamT"
		
		"bgcolor_override"	"0 0 0 153"
		"PaintBackgroundType"	"0"
	}
}