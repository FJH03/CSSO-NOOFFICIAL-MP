"Resource/HudLayout.res"
{
	CSHudWeaponSelection
	{
		"fieldName" "CSHudWeaponSelection"
		"visible"	"1"
		"enabled"	"1"
		"xpos"		"0"
		"ypos"		"0"
		"wide"		"w1920" // w means calculate proportions by width instead of height (otherwise the width is incorrect on 4:3)
		"tall"		"1080"
		"base_resolution_wide"	"1920"
		"base_resolution_tall"	"1080"
		
		"icons_base_xpos"		"w34" // counts from the right side
		"icons_base_ypos"		"953"

		"name_label_xpos"		"w4" // relative to icons_base_xpos
		"name_label_ypos"		"48"

		"count_label_xpos"	"w32" // relative to the icon
		"count_label_ypos"	"27" // relative to the icon

		"slot_label_xpos"		"w5" // counts from the right side
		"slot_label_ypos"		"2" // relative to the icon

		"weapon_icon_tall"			"48"
		"weapon_icon_slot_margin"		"80"
		"weapon_icon_pos_margin"		"w30"
		"weapon_icon_defuser_margin"	"39"

		"defuser_icon_xpos"	"w3" // relative to icons_base_xpos
		"defuser_icon_ypos"	"980"
		"defuser_icon_tall"	"45"
		"defuser_icon"		"materials/vgui/hud/svg/defuser.svg"
		"defuser_fgcolor"		"71 93 117 255"
	}
	"HudAccount"
	{
		"fieldName" "HudAccount"
		"xpos"	"12"
		"ypos"	"350"
		"wide"	"164"
		"tall"  "48"
		"base_resolution_wide"	"1920"
		"base_resolution_tall"	"1080"
		"visible" "1"
		"enabled" "1"

		"PaintBackgroundType"	"1"
		"bgcolor_override"	"White"
		"Texture1"			"vgui/hud/money_background"
		
		"BuyZoneIconFg"		"White"
		"margin_right"		"48"
	}
	"HudAmmo"
	{
		"fieldName" "HudAmmo"
		"xpos"	"r342"
		"ypos"	"r54"
		"wide"	"342"
		"tall"  "54"
		"base_resolution_wide"	"1920"
		"base_resolution_tall"	"1080"
		"visible" "1"
		"enabled" "1"
		"proportional"	"0"

		"PaintBackgroundType"	"1"
		"bgcolor_override"	"White"
		"Texture1"			"vgui/hud/ammo_background"

		"simple_xpos"	"r282"
		"simple_ypos"	"r54"
 		"simple_wide"	"282"
		"simple_tall"	"54"
	}
	"HudHealthArmor"
	{
		"fieldName"		"HudHealthArmor"
		"xpos"			"8"
		"ypos"			"r53"
		"zpos"			"0"
 		"wide"			"424"
		"tall"			"53"
		"base_resolution_wide"	"1920"
		"base_resolution_tall"	"1080"
		"visible"		"1"
		"enabled"		"1"
		"proportional"	"0"

		"PaintBackgroundType"	"1"
		"bgcolor_override"	"White"
		"Texture1"			"vgui/hud/healtharmor_background"

 		"simple_wide"	"248"
		"simple_tall"	"53"

		"health_icon_xpos"	"19"
		"health_icon_ypos"	"16"
		"armor_icon_xpos"		"227"
		"armor_icon_ypos"		"16"

		"simple_health_icon_xpos"	"19"
		"simple_health_icon_ypos"	"16"
		"simple_armor_icon_xpos"	"144"
		"simple_armor_icon_ypos"	"16"
	}
	"HudScenarioHostageIcon"
	{
		"fieldName" "HudScenarioHostageIcon"
		"xpos"	"0"
		"ypos"	"430"
		"wide"	"256"
		"tall"  "40"
		"base_resolution_wide"	"1920"
		"base_resolution_tall"	"1080"
		"visible" "1"
		"enabled" "1"

		"hostage_alive_fgcolor"		"255 255 0 255"
		"hostage_rescued_fgcolor"		"0 128 0 255"
		"rescuezone_icon_margin_right"	"12"
	}
	"HudTeamCounter"
	{
		"fieldName" "HudTeamCounter"
		"xpos"	"c-96"
		"ypos"	"2"
		"wide"	"192"
		"tall"  "55"
		"base_resolution_wide"	"1920"
		"base_resolution_tall"	"1080"
		"visible" "1"
		"enabled" "1"
		
		"RoundTimerColor" "White"
		"RoundTimerLowColor" "255 0 0 255"
		"C4PlantedColor" "184 0 0 255"
		"C4DefusedColor" "69 170 46 255"
	}
	"HudUniqueAlerts"
	{
		"fieldName" "HudUniqueAlerts"
		"xpos"	"0"
		"ypos"	"0"
		"wide"	"640"
		"tall"	"480"
		"visible" "1"
		"enabled" "1"
		
		"MatchStartAlert" "225 0 0 255"
	}
	"HudVoiceDisabledPanel"
	{
		"fieldName" "HudVoiceDisabledPanel"
		"xpos"	"18"
		"ypos"	"1002"
		"wide"	"110"
		"tall"  "22"
		"base_resolution_wide"	"1920"
		"base_resolution_tall"	"1080"
		"visible" "1"
		"enabled" "1"

		"PaintBackgroundType"	"1"
		"bgcolor_override"	"White"
		"Texture1"			"vgui/hud/voice_disabled_background"
		
		"margin_right"		"8"
	}
	"WinPanel_Round"
	{
		"fieldName"			"WinPanel_Round"
		"xpos"				"w450"
		"ypos"				"128"
		"wide"				"w1020"
		"tall"				"275"
		"base_resolution_wide"	"1920"
		"base_resolution_tall"	"1080"
		"visible"			"1"
		"enabled"			"1"

		"mvp_avatar_margin"	"12"
	}

	HudCommentary
	{
		"fieldName" "HudCommentary"
		"xpos"	"c-190"
		"ypos"	"350"
		"wide"	"380"
		"tall"  "40"
		"visible" "1"
		"enabled" "1"
		
		"PaintBackgroundType"	"2"
		
		"bar_xpos"		"50"
		"bar_ypos"		"20"
		"bar_height"	"8"
		"bar_width"		"320"
		"speaker_xpos"	"50"
		"speaker_ypos"	"8"
		"count_xpos_from_right"	"10"	// Counts from the right side
		"count_ypos"	"8"
		
		"icon_texture"	"vgui/hud/icon_commentary"
		"icon_xpos"		"0"
		"icon_ypos"		"0"		
		"icon_width"	"40"
		"icon_height"	"40"
	}
	
	HudHDRDemo
	{
		"fieldName" "HudHDRDemo"
		"xpos"	"0"
		"ypos"	"0"
		"wide"	"640"
		"tall"  "480"
		"visible" "0"
		"enabled" "1"
		
		"Alpha"	"255"
		"PaintBackgroundType"	"2"
		
		"BorderColor"	"0 0 0 255"
		"BorderLeft"	"16"
		"BorderRight"	"16"
		"BorderTop"		"16"
		"BorderBottom"	"64"
		"BorderCenter"	"0"
		
		"TextColor"		"255 255 255 255"
		"LeftTitleY"	"422"
		"RightTitleY"	"422"
	}

	TargetID
	{
		"fieldName" "TargetID"
		"visible" "1"
		"enabled" "1"
		"wide"	 "640"
		"tall"	 "480"
	}

	HudSuit
	{
		"fieldName"		"HudSuit"
		"xpos"	"140"
		"ypos"	"432"
		"wide"	"108"
		"tall"  "36"
		"visible" "1"
		"enabled" "1"

		"PaintBackgroundType"	"2"

		
		"text_xpos" "8"
		"text_ypos" "20"
		"digit_xpos" "50"
		"digit_ypos" "2"
	}

	HudProgressBar
	{
		"fieldName"			"HudProgressBar"
		"xpos"				"c-300"
		"ypos"				"616"
		"wide"				"600"
		"tall"				"116"
		"base_resolution_wide"	"1920"
		"base_resolution_tall"	"1080"
		"visible"			"1"
		"enabled"			"1"

		"PaintBackgroundType"	"1"
		"bgcolor_override"	"White"
		"Texture1"			"vgui/hud/progressbar_background"
	}

	HudShoppingCart
	{
		"fieldName" "HudShoppingCart"
		"xpos"	"116"
		"ypos"	"126"
		"wide"	"40"
		"tall"  "40"
		"visible" "1"
		"enabled" "1"

		"legacy_xpos"	"64"
		"legacy_ypos"	"240"

		"PaintBackgroundType"	"2"
		"IconColor"			"HudIcon_Green"

	}

	HudC4
	{
		"fieldName" "HudC4"
		"xpos"	"28"
		"ypos"	"433"
		"wide"	"32"
		"tall"  "0" // calculated procedurally
		"base_resolution_wide"	"1920"
		"base_resolution_tall"	"1080"
		"visible" "1"
		"enabled" "1"

		"IconColor"		"255 177 54 255"
		"BombZoneColor"	"255 177 54 128"

		"mirror_x"		"1"
		"image"			"materials/vgui/weapons/svg/c4.svg"
	}

	HudFlashlight
	{
		"fieldName" "HudFlashlight"
		"visible" "1"
		"enabled" "1"
		"xpos"	"16"
		"ypos"	"370"
		"wide"	"102"
		"tall"	"20"
		
		"text_xpos" "8"
		"text_ypos" "6"
		"TextColor"	"255 170 0 220"

		"PaintBackgroundType"	"2"
	}
	
	HudDamageIndicator
	{
		"fieldName" "HudDamageIndicator"
		"visible" "1"
		"enabled" "1"
		"DmgColorLeft" "255 0 0 0"
		"DmgColorRight" "255 0 0 0"
		
		"dmg_xpos" "30"
		"dmg_ypos" "100"
		"dmg_wide" "36"
		"dmg_tall1" "240"
		"dmg_tall2" "200"
	}

	HudZoom
	{
		"fieldName" "HudZoom"
		"visible" "1"
		"enabled" "1"
		"Circle1Radius" "66"
		"Circle2Radius"	"74"
		"DashGap"	"16"
		"DashHeight" "4"
		"BorderThickness" "88"
	}

	HudWeaponSelection
	{
		"fieldName" "HudWeaponSelection"
		"xpos"	"r640"
		"wide"	"640"
		"ypos" 	"16"
		"visible" "1"
		"enabled" "1"
		"SmallBoxSize" "60"
		"LargeBoxWide" "108"
		"LargeBoxTall" "80"
		"BoxGap" "8"
		"SelectionNumberXPos" "4"
		"SelectionNumberYPos" "4"
		"SelectionGrowTime"	"0.4"
		"IconXPos" "8"
		"IconYPos" "0"
		"TextYPos" "68"	
		"TextColor" "SelectionTextFg"
		"MaxSlots"	"5"
		"PlaySelectSounds"	"0"
	}

	HudCrosshair
	{
		"fieldName" "HudCrosshair"
		"visible" "1"
		"enabled" "1"
		"wide"	 "640"
		"tall"	 "480"
	}

	HudDeathNotice
	{
		"fieldName" "HudDeathNotice"
		"visible" "1"
		"enabled" "1"
		"xpos"	 "0"
		"ypos"	 "0"
		"wide"	 "w1920"
		"tall"	 "1080"
		"base_resolution_wide"	"1920"
		"base_resolution_tall"	"1080"

		"Texture1"				"vgui/hud/deathnotice_corner1"
		"Texture2"				"vgui/hud/deathnotice_corner2"
		"Texture3"				"vgui/hud/deathnotice_corner3"
		"Texture4"				"vgui/hud/deathnotice_corner4"
		"OutlineTexture1"			"vgui/hud/deathnotice_outline_corner1"
		"OutlineTexture2"			"vgui/hud/deathnotice_outline_corner2"
		"OutlineTexture3"			"vgui/hud/deathnotice_outline_corner3"
		"OutlineTexture4"			"vgui/hud/deathnotice_outline_corner4"
		"scale_corner_textures"	"1"

		"MaxDeathNotices"			"15"
		"LineHeight"				"31"
		"HeightMargin"			"3"
		"BackgroundWidthMargin"	"w10"
		"RightMargin"				"w10"
		"TopMargin"				"75"
		"BorderSize"				"2"
		"IconMargin"				"w4"
		"IconTall"				"24"

		"BackgroundColor"			"0 0 0 166"
		"VictimBackgroundColor"	"168 19 19 178"
		"TextFont"				"DeathnoticeFont"
		"CTTextColor"				"TeamCTDark"
		"TerroristTextColor"		"TeamTDark"
		"BorderColor"				"225 0 0 255"
	}

	HudVehicle
	{
		"fieldName" "HudVehicle"
		"visible" "1"
		"enabled" "1"
		"wide"	 "640"
		"tall"	 "480"
	}


	CVProfPanel
	{
		"fieldName" "CVProfPanel"
		"visible" "1"
		"enabled" "1"
		"wide"	 "640"
		"tall"	 "480"
	}

	ScorePanel
	{
		"fieldName" "ScorePanel"
		"visible" "1"
		"enabled" "1"
		"wide"	 "640"
		"tall"	 "480"
	}

	HudTrain
	{
		"fieldName" "HudTrain"
		"visible" "1"
		"enabled" "1"
		"wide"	 "640"
		"tall"	 "480"
	}

	HudMOTD
	{
		"fieldName" "HudMOTD"
		"visible" "1"
		"enabled" "1"
		"wide"	 "640"
		"tall"	 "480"
	}

	HudMessage
	{
		"fieldName" "HudMessage"
		"visible" "1"
		"enabled" "1"
		"wide"	 "f0"
		"tall"	 "480"
	}

	HudMenu
	{
		"fieldName" "HudMenu"
		"visible" "1"
		"enabled" "1"
		"wide"	 "640"
		"tall"	 "480"
		"zpos" "1"

		"TextFont"			"Default"
		"ItemFont"			"Default"
		"ItemFontPulsing"		"Default"

	}

	HudCloseCaption
	{
		"fieldName" "HudCloseCaption"
		"visible"	"1"
		"enabled"	"1"
		"xpos"		"c-250"
		"ypos"		"276"	[$WIN32]
		"ypos"		"236"	[$X360]
		"wide"		"500"
		"tall"		"136"	[$WIN32]
		"tall"		"176"	[$X360]

		"BgAlpha"	"128"

		"GrowTime"		"0.25"
		"ItemHiddenTime"	"0.2"  // Nearly same as grow time so that the item doesn't start to show until growth is finished
		"ItemFadeInTime"	"0.15"	// Once ItemHiddenTime is finished, takes this much longer to fade in
		"ItemFadeOutTime"	"0.3"
		"topoffset"		"0"		[$WIN32]
		"topoffset"		"0"	[$X360]
	}

	HudGeiger
	{
		"fieldName" "HudGeiger"
		"visible" "1"
		"enabled" "1"
		"wide"	 "640"
		"tall"	 "480"
	}

	HUDQuickInfo
	{
		"fieldName" "HUDQuickInfo"
		"visible" "1"
		"enabled" "1"
		"wide"	 "640"
		"tall"	 "480"
	}

	HudWeapon
	{
		"fieldName" "HudWeapon"
		"visible" "1"
		"enabled" "1"
		"wide"	 "640"
		"tall"	 "480"
	}
	HudAnimationInfo
	{
		"fieldName" "HudAnimationInfo"
		"visible" "1"
		"enabled" "1"
		"wide"	 "640"
		"tall"	 "480"
	}
	CBudgetPanel
	{
		"fieldName" "CBudgetPanel"
		"visible" "1"
		"enabled" "1"
		"wide"	 "640"
		"tall"	 "480"
	}
	CTextureBudgetPanel
	{
		"fieldName" "CTextureBudgetPanel"
		"visible" "1"
		"enabled" "1"
		"wide"	 "640"
		"tall"	 "480"
	}

	HudPredictionDump
	{
		"fieldName" "HudPredictionDump"
		"visible" "1"
		"enabled" "1"
		"wide"	 "640"
		"tall"	 "480"
	}

	overview
	{
		"fieldName" "overview"
		"visible"	"1"
		"enabled"	"1"
		"xpos"		"24"
		"ypos"		"80"
		"wide"		"256"
		"tall"		"256"
		"base_resolution_wide"	"1920"
		"base_resolution_tall"	"1080"
		"transparent_border_size"	"2"

		"bgcolor_override"		"0 0 0 255"
	}

	HudLocation
	{
		"fieldName" "HudLocation"
		"visible"	"1"
		"enabled"	"1"
		"xpos"		"25"
		"ypos"		"30"
		"wide"		"512"
		"tall"		"24"
		"base_resolution_wide"	"1920"
		"base_resolution_tall"	"1080"
		"textAlignment"	"west"
		"font"		"LocationFont"
		"fgcolor_override"	"180 180 180 230"
	}

	HudScope
	{
		"fieldName" "HudZoom"
		"visible" "1"
		"enabled" "1"
		"wide"	 "640"
		"tall"	 "480"
	}

	HudVoiceStatus
	{
		"fieldName" "HudVoiceStatus"
		"visible" "1"
		"enabled" "1"
		"xpos" "24"
		"ypos" "r498"
		"wide" "1280"
		"tall" "400"
		"base_resolution_wide"	"1920"
		"base_resolution_tall"	"1080"

		"item_tall"	"20"
		"item_wide"	"768"
		"item_spacing" "1"
		"item_font"	"VoiceStatusFont"
		
		"show_avatar"		"1"
		"show_friend"		"0"
		"show_voice_icon"	"1"
		"show_dead_icon"	"1"

		"dead_xpos"	"28"
		"dead_ypos"	"0"
		"dead_wide"	"20"
		"dead_tall"	"20"

		"avatar_xpos"	"28"
		"avatar_ypos"	"0"
		"avatar_wide"	"20"
		"avatar_tall"	"20"

		"icon_xpos"	"6"
		"icon_ypos"	"0"
		"icon_wide"	"20"
		"icon_tall"	"20"
		
		"text_xpos"	"56"
		
		"fade_in_time" "0.2"
		"fade_out_time" "0.2"
	}

	HudFlashbang
	{
	}

	HudHintKeyDisplay
	{
		"fieldName"	"HudHintKeyDisplay"
		"visible"	"0"
		"enabled" 	"1"
		"xpos"		"r120"
		"ypos"		"r340"
		"wide"		"100"
		"tall"		"200"
		"text_xpos"	"8"
		"text_ypos"	"8"
		"text_xgap"	"8"
		"text_ygap"	"8"
		"TextColor"	"255 170 0 220"

		"PaintBackgroundType"	"2"
	}

	HudTerritory
	{
		"fieldName" "HudTerritory"
		"visible" "1"
		"enabled" "1"
		"xpos"	"240"
	    	"ypos"	"432"
	    	"wide" "240"
	    	"tall" "48"
	}

	TerritorySCore
	{
	    "fieldName" "TerritoryScore"
	    "visible" "0"
	    "enabled" "0"
	    "xpos"	"240"
	    "ypos"	"450"
	    "wide" "200"
	    "tall" "200"
	    "text_xpos" "8"
	    "text_ypos" "4"
	}
	"HudChat"
	{
		"ControlName"		"EditablePanel"
		"fieldName" 		"HudChat"
		"visible" 		"1"
		"enabled" 		"1"
		"xpos"			"10"	[$WIN32]
		"xpos"			"42"	[$X360]
		"ypos"			"275"
		"wide"	 		"320"
		"tall"	 		"120"
		"PaintBackgroundType"	"2"
	}
	
	WinPanel_Match
	{
		"fieldName"				"WinPanel_Match"	
		"visible"				"1"
		"enabled"				"1"
		"xpos"					"c-150"
		"ypos"					"c-175"
		"wide"					"300"
		"tall"					"350"
		"PaintBackgroundType"	"2"
	}
	
	FreezePanel
	{
		"fieldName"			"FreezePanel"
		"visible"			"1"
		"enabled"			"1"
		"xpos"				"c-255"
		"ypos"				"540"
		"wide"				"510"
		"tall"				"154"
		"base_resolution_wide"	"1920"
		"base_resolution_tall"	"1080"

		"screenshot_label_ypos" "86"
		"screenshot_label_ypos_damage_taken" "109"
		"screenshot_label_ypos_damage_given" "132"
	}
	
	AchievementAnnouncePanel
	{
		"fieldName"				"AchievementAnnouncePanel"
		"visible"				"1"
		"enabled"				"1"
		"xpos"					"0"
		"ypos"					"0"
		"wide"					"f0"
		"tall"					"480"
	}
	
	StatPanel
	{
		"fieldName"				"StatPanel"
		"visible"				"0"
		"enabled"				"1"
	}
	
	AchievementNotificationPanel
	{
		"fieldName"				"AchievementNotificationPanel"
		"visible"				"0"
		"enabled"				"1"
	}
	
	HUDAutoAim
	{
		"fieldName"				"HUDAutoAim"
		"visible"				"0"
		"enabled"				"1"
	}

	"HudAchievementTracker"
	{
		"ControlName"	"EditablePanel"
		"fieldName"		"HudAchievementTracker"
		"xpos"			"10"
		"NormalY"		"120"
		"EngineerY"		"170"
		"zpos"			"20"
		"wide"			"250"
		"tall"			"280"
		"visible"		"1"
		"enabled"		"1"	
	}
	
	"HudHeadName"
	{
		"ControlName"   "EditablePanel"
		"fieldName"     "HudHeadName"
		"xpos"          "0"
		"ypos"          "0"
		"zpos"          "0"
		"wide"          "f0"
		"tall"          "f0"
		"visible"       "1"
		"enabled"       "1"
		"item_font"     "VoiceStatusFont"
	}
}