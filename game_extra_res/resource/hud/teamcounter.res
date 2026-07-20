"resource/hud/teamcounter_digital.res"
{	
	"RoundTimerLabel"
	{
		"fieldName"			"RoundTimerLabel"
		"xpos"				"54"
		"ypos"				"0"
		"wide"				"84"
		"tall"				"27"
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
		"xpos"				"79"
		"ypos"				"-2"
		"wide"				"42"
		"tall"				"0" // computed procedurally
		"base_resolution_wide" "1920"
		"base_resolution_tall" "1080"
		
		"image"				"materials/vgui/hud/svg/bomb_c4.svg"
	}
	
	"CTAliveBgImage"
	{
		"ControlName"			"ImagePanel"
		"fieldName"			"CTAliveBgImage"
		"xpos"				"0"
		"ypos"				"0"
		"wide"				"51"
		"tall"				"55"
		"base_resolution_wide" "1920"
		"base_resolution_tall" "1080"
		"image"				"hud/teamcounter_alivebgct"
		"scaleImage"			"1"
		"zpos"				"-1"
	}
	"TAliveBgImage"
	{
		"ControlName"			"ImagePanel"
		"fieldName"			"TAliveBgImage"
		"xpos"				"141"
		"ypos"				"0"
		"wide"				"51"
		"tall"				"55"
		"base_resolution_wide" "1920"
		"base_resolution_tall" "1080"
		"image"				"hud/teamcounter_alivebgt"
		"scaleImage"			"1"
		"zpos"				"-1"
	}
	
	"CTWinCounterLabel"
	{
		"fieldName"			"CTWinCounterLabel"
		"xpos"				"54"
		"ypos"				"28"
		"wide"				"42"
		"tall"				"27"
		"base_resolution_wide" "1920"
		"base_resolution_tall" "1080"
		"textAlignment"		"center"
		"font"				"RoundTimerFont"
		"fgcolor_override"	"TeamCT"
		
		"bgcolor_override"	"0 0 0 153"
		"PaintBackgroundType"	"0"
	}
	"CTAliveCounterLabel"
	{
		"fieldName"			"CTAliveCounterLabel"
		"xpos"				"0"
		"ypos"				"0"
		"wide"				"51"
		"tall"				"51"
		"base_resolution_wide" "1920"
		"base_resolution_tall" "1080"
		"textAlignment"		"center"
		"font"				"AliveCountFont"
		"fgcolor_override"	"White"
	}
	"CTAliveTextLabel"
	{
		"fieldName"			"CTAliveTextLabel"
		"xpos"				"0"
		"ypos"				"37"
		"wide"				"51"
		"tall"				"18"
		"base_resolution_wide" "1920"
		"base_resolution_tall" "1080"
		"textAlignment"		"center"
		"font"				"AliveTextFont"
		"fgcolor_override"	"White"
		"labelText"			"#Cstrike_PlayerCount_Alive"
	}
	"CTSkullImage"
	{
		"fieldName"			"CTSkullImage"
		"xpos"				"9"
		"ypos"				"10"
		"wide"				"32"
		"tall"				"32"
		"base_resolution_wide" "1920"
		"base_resolution_tall" "1080"
		"image"				"hud/teamcounter_aliveskull"
		"scaleImage"			"1"
	}
	
	"TWinCounterLabel"
	{
		"fieldName"			"TWinCounterLabel"
		"xpos"				"97"
		"ypos"				"28"
		"wide"				"41"
		"tall"				"27"
		"base_resolution_wide" "1920"
		"base_resolution_tall" "1080"
		"textAlignment"		"center"
		"font"				"RoundTimerFont"
		"fgcolor_override"	"TeamT"
		
		"bgcolor_override"	"0 0 0 153"
		"PaintBackgroundType"	"0"
	}
	"TAliveCounterLabel"
	{
		"fieldName"			"TAliveCounterLabel"
		"xpos"				"141"
		"ypos"				"0"
		"wide"				"51"
		"tall"				"51"
		"base_resolution_wide" "1920"
		"base_resolution_tall" "1080"
		"textAlignment"		"center"
		"font"				"AliveCountFont"
		"fgcolor_override"	"White"
	}
	"TAliveTextLabel"
	{
		"fieldName"			"TAliveTextLabel"
		"xpos"				"141"
		"ypos"				"37"
		"wide"				"51"
		"tall"				"18"
		"base_resolution_wide" "1920"
		"base_resolution_tall" "1080"
		"textAlignment"		"center"
		"font"				"AliveTextFont"
		"fgcolor_override"	"White"
		"labelText"			"#Cstrike_PlayerCount_Alive"
	}
	"TSkullImage"
	{
		"fieldName"			"TSkullImage"
		"xpos"				"151"
		"ypos"				"10"
		"wide"				"32"
		"tall"				"32"
		"base_resolution_wide" "1920"
		"base_resolution_tall" "1080"
		"image"				"hud/teamcounter_aliveskull"
		"scaleImage"			"1"
	}
}