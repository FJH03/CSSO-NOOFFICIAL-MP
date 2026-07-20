"resource/hud/ammo.res"
{
	"PrimaryAmmoLabel"
	{
		"fieldName"			"PrimaryAmmoLabel"
		"xpos"				"70"
		"ypos"				"7"
		"wide"				"128"
		"tall"				"36"
		"base_resolution_wide" "1920"
		"base_resolution_tall" "1080"
		"textAlignment"		"east"
		"font"				"AmmoClipFont"
		"fgcolor_override"	"White"
	}

	"PrimaryReserveAmmoLabel"
	{
		"fieldName"			"PrimaryReserveAmmoLabel"
		"xpos"				"178"
		"ypos"				"21"
		"wide"				"64"
		"tall"				"24"
		"base_resolution_wide" "1920"
		"base_resolution_tall" "1080"
		"textAlignment"		"east"
		"font"				"AmmoMaxClipFont"
		"fgcolor_override"	"White"
	}

	"StatTrakCounter"
	{
		"fieldName"			"StatTrakCounter"
		"xpos"				"0"
		"ypos"				"37"
		"wide"				"120"
		"tall"				"16"
		"base_resolution_wide" "1920"
		"base_resolution_tall" "1080"
		"textAlignment"		"east"
		"font"				"StatTrakCounterFont"
		"fgcolor_override"	"White"
	}

	"KillCounter"
	{
		"fieldName"			"KillCounter"
		"xpos"				"100"
		"ypos"				"8"
		"wide"				"64"
		"tall"				"24"
		"base_resolution_wide" "1920"
		"base_resolution_tall" "1080"
		"textAlignment"		"west"
		"font"				"AmmoMaxClipFont"
		"fgcolor_override"	"White"
	}
	
	"KillCounterImage"
	{
		"fieldName"			"KillCounterImage"
		"xpos"				"74"
		"ypos"				"6"
		"wide"				"24"
		"tall"				"24"
		"base_resolution_wide" "1920"
		"base_resolution_tall" "1080"
		"image"				"hud/kill_icon"
		"scaleImage"			"1"
	}

	"KillCounterImage1"
	{
		"fieldName"			"KillCounterImage"
		"xpos"				"80"
		"ypos"				"6"
		"wide"				"24"
		"tall"				"24"
		"base_resolution_wide" "2256"
		"base_resolution_tall" "1080"
		"image"				"hud/kill_icon"
		"scaleImage"			"1"
	}
	"KillCounterImage2"
	{
		"fieldName"			"KillCounterImage"
		"xpos"				"60"
		"ypos"				"6"
		"wide"				"24"
		"tall"				"24"
		"base_resolution_wide" "2256"
		"base_resolution_tall" "1080"
		"image"				"hud/kill_icon"
		"scaleImage"			"1"
	}
	"KillCounterImage3"
	{
		"fieldName"			"KillCounterImage"
		"xpos"				"40"
		"ypos"				"6"
		"wide"				"24"
		"tall"				"24"
		"base_resolution_wide" "2256"
		"base_resolution_tall" "1080"
		"image"				"hud/kill_icon"
		"scaleImage"			"1"
	}
	"KillCounterImage4"
	{
		"fieldName"			"KillCounterImage"
		"xpos"				"20"
		"ypos"				"6"
		"wide"				"24"
		"tall"				"24"
		"base_resolution_wide" "2256"
		"base_resolution_tall" "1080"
		"image"				"hud/kill_icon"
		"scaleImage"			"1"
	}
	"KillCounterImage5"
	{
		"fieldName"			"KillCounterImage"
		"xpos"				"0"
		"ypos"				"6"
		"wide"				"24"
		"tall"				"24"
		"base_resolution_wide" "2256"
		"base_resolution_tall" "1080"
		"image"				"hud/kill_icon"
		"scaleImage"			"1"
	}

	"BulletIcon"
	{
		"fieldName"			"BulletIcon"
		"xpos"				"292"
		"ypos"				"24"
		"wide"				"16"
		"tall"				"16"
		"base_resolution_wide" "1920"
		"base_resolution_tall" "1080"
		"fgcolor_override"	"White"
		
		"repeat_xpos"			"-10"
		"image"				"materials/vgui/hud/svg/bullet.svg"
	}

	"ExhaustibleWeaponIcon"
	{
		"fieldName"			"ExhaustibleWeaponIcon"
		"xpos"				"285"
		"ypos"				"9"
		"wide"				"0" // this has to be calculated procedurally
		"tall"				"32"
		"base_resolution_wide" "1920"
		"base_resolution_tall" "1080"
		"fgcolor_override"	"224 224 224 255"
		
		"repeat_xpos"			"-32"
	}

	"BurstIcon"
	{
		"fieldName"			"BurstIcon"
		"xpos"				"200"
		"ypos"				"8"
		"wide"				"18"
		"tall"				"0" // this has to be calculated procedurally
		"base_resolution_wide" "1920"
		"base_resolution_tall" "1080"
		"fgcolor_override"	"White"

		"repeat_xpos"			"20"
		"repeats_count"		"3"
		"image"				"materials/vgui/hud/svg/bullet_burst_outline.svg"
	}
}