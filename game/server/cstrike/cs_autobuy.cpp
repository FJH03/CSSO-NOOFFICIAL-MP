//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Data for Autobuy and Rebuy 
//
//=============================================================================//

#include "cbase.h"
#include "cs_autobuy.h"

// Weapon class information for each weapon including the class name and the buy command alias.
AutoBuyInfoStruct g_autoBuyInfo[] = 
{
	{ (AutoBuyClassType)(AUTOBUYCLASS_PRIMARY | AUTOBUYCLASS_RIFLE), "ak47", "weapon_ak47" },  // ak47
	{ (AutoBuyClassType)(AUTOBUYCLASS_PRIMARY | AUTOBUYCLASS_RIFLE), "galilar", "weapon_galilar" }, // galilar
	{ (AutoBuyClassType)(AUTOBUYCLASS_PRIMARY | AUTOBUYCLASS_SNIPERRIFLE), "awp", "weapon_awp" }, // awp
	{ (AutoBuyClassType)(AUTOBUYCLASS_PRIMARY | AUTOBUYCLASS_SNIPERRIFLE), "g3sg1", "weapon_g3sg1" }, // g3sg1
	{ (AutoBuyClassType)(AUTOBUYCLASS_PRIMARY | AUTOBUYCLASS_RIFLE), "m4a4", "weapon_m4a4" }, // m4a4
	{ (AutoBuyClassType)(AUTOBUYCLASS_PRIMARY | AUTOBUYCLASS_RIFLE), "m4a1_silencer", "weapon_m4a1_silencer" }, // m4a1_silencer
	{ (AutoBuyClassType)(AUTOBUYCLASS_PRIMARY | AUTOBUYCLASS_RIFLE), "famas", "weapon_famas" }, // famas
	{ (AutoBuyClassType)(AUTOBUYCLASS_PRIMARY | AUTOBUYCLASS_RIFLE), "aug", "weapon_aug" }, // aug
	{ (AutoBuyClassType)(AUTOBUYCLASS_PRIMARY | AUTOBUYCLASS_RIFLE), "sg556", "weapon_sg556" }, // sg556
	{ (AutoBuyClassType)(AUTOBUYCLASS_PRIMARY | AUTOBUYCLASS_SNIPERRIFLE), "scar20", "weapon_scar20" }, // scar20
	{ (AutoBuyClassType)(AUTOBUYCLASS_PRIMARY | AUTOBUYCLASS_SNIPERRIFLE), "ssg08", "weapon_ssg08" }, // ssg08
	{ (AutoBuyClassType)(AUTOBUYCLASS_SECONDARY | AUTOBUYCLASS_PISTOL), "glock", "weapon_glock" }, // glock
	{ (AutoBuyClassType)(AUTOBUYCLASS_SECONDARY | AUTOBUYCLASS_PISTOL), "usp_silencer", "weapon_usp_silencer" }, // usp
	{ (AutoBuyClassType)(AUTOBUYCLASS_SECONDARY | AUTOBUYCLASS_PISTOL), "deagle", "weapon_deagle" }, // deagle
	{ (AutoBuyClassType)(AUTOBUYCLASS_SECONDARY | AUTOBUYCLASS_PISTOL), "revolver", "weapon_revolver" }, // revolver
	{ (AutoBuyClassType)(AUTOBUYCLASS_SECONDARY | AUTOBUYCLASS_PISTOL), "elite", "weapon_elite" },	// elites
	{ (AutoBuyClassType)(AUTOBUYCLASS_SECONDARY | AUTOBUYCLASS_PISTOL), "fn57", "weapon_fiveseven" },	// fn57
	{ (AutoBuyClassType)(AUTOBUYCLASS_SECONDARY | AUTOBUYCLASS_PISTOL), "hkp2000", "weapon_hkp2000" }, // hkp2000
	{ (AutoBuyClassType)(AUTOBUYCLASS_SECONDARY | AUTOBUYCLASS_PISTOL), "p250", "weapon_p250" }, // p250
	{ (AutoBuyClassType)(AUTOBUYCLASS_SECONDARY | AUTOBUYCLASS_PISTOL), "cz75a", "weapon_cz75a" },	// cz75a
	{ (AutoBuyClassType)(AUTOBUYCLASS_SECONDARY | AUTOBUYCLASS_PISTOL), "tec9", "weapon_tec9" },	// tec9
	{ (AutoBuyClassType)(AUTOBUYCLASS_SECONDARY | AUTOBUYCLASS_PISTOL),	"taser", "weapon_taser" },	// taser
	{ (AutoBuyClassType)(AUTOBUYCLASS_PRIMARY | AUTOBUYCLASS_SHOTGUN), "nova", "weapon_nova" }, // nova
	{ (AutoBuyClassType)(AUTOBUYCLASS_PRIMARY | AUTOBUYCLASS_SHOTGUN), "xm1014", "weapon_xm1014" },	// xm1014
	{ (AutoBuyClassType)(AUTOBUYCLASS_PRIMARY | AUTOBUYCLASS_SHOTGUN), "mag7", "weapon_mag7" }, // mag7
	{ (AutoBuyClassType)(AUTOBUYCLASS_PRIMARY | AUTOBUYCLASS_SHOTGUN), "sawedoff", "weapon_sawedoff" }, // sawedoff
	{ (AutoBuyClassType)(AUTOBUYCLASS_PRIMARY | AUTOBUYCLASS_SMG), "mac10", "weapon_mac10" },	// mac10
	{ (AutoBuyClassType)(AUTOBUYCLASS_PRIMARY | AUTOBUYCLASS_SMG), "ump45", "weapon_ump45" },	// ump45
	{ (AutoBuyClassType)(AUTOBUYCLASS_PRIMARY | AUTOBUYCLASS_SMG), "p90", "weapon_p90" },	// p90
	{ (AutoBuyClassType)(AUTOBUYCLASS_PRIMARY | AUTOBUYCLASS_SMG), "bizon", "weapon_bizon" },	// bizon
	{ (AutoBuyClassType)(AUTOBUYCLASS_PRIMARY | AUTOBUYCLASS_SMG), "mp7", "weapon_mp7" },	// mp7
	{ (AutoBuyClassType)(AUTOBUYCLASS_PRIMARY | AUTOBUYCLASS_SMG), "mp5sd", "weapon_mp5sd" },	// mp5sd
	{ (AutoBuyClassType)(AUTOBUYCLASS_PRIMARY | AUTOBUYCLASS_SMG), "mp9", "weapon_mp9" },	// mp9
	{ (AutoBuyClassType)(AUTOBUYCLASS_PRIMARY | AUTOBUYCLASS_MACHINEGUN), "m249", "weapon_m249" },	// m249
	{ (AutoBuyClassType)(AUTOBUYCLASS_PRIMARY | AUTOBUYCLASS_MACHINEGUN), "negev", "weapon_negev" },	// negev
	{ (AutoBuyClassType)(AUTOBUYCLASS_PRIMARY | AUTOBUYCLASS_AMMO), "primammo", "primammo" },	// primammo
	{ (AutoBuyClassType)(AUTOBUYCLASS_SECONDARY | AUTOBUYCLASS_AMMO), "secammo", "secammo" }, // secmmo
	{ (AutoBuyClassType)(AUTOBUYCLASS_ARMOR), "kevlar", "item_kevlar" }, // kevlar
 	{ (AutoBuyClassType)(AUTOBUYCLASS_ARMOR), "assaultsuit", "item_assaultsuit" }, // assaultsuit
	{ (AutoBuyClassType)(AUTOBUYCLASS_GRENADE), "flashbang", "weapon_flashbang" }, // flash
	{ (AutoBuyClassType)(AUTOBUYCLASS_GRENADE), "hegrenade", "weapon_hegrenade" }, // hegren
	{ (AutoBuyClassType)(AUTOBUYCLASS_GRENADE), "smokegrenade", "weapon_smokegrenade" }, // sgren
	{ (AutoBuyClassType)(AUTOBUYCLASS_GRENADE), "molotov", "weapon_molotov" }, // molotov
	{ (AutoBuyClassType)(AUTOBUYCLASS_GRENADE), "incgrenade", "weapon_incgrenade" }, // incgrenade
	{ (AutoBuyClassType)(AUTOBUYCLASS_GRENADE), "decoy", "weapon_decoy" }, // decoy
	{ (AutoBuyClassType)(AUTOBUYCLASS_NIGHTVISION), "nvgs", "nvgs" }, // nvgs
	{ (AutoBuyClassType)(AUTOBUYCLASS_DEFUSER), "defuser", "defuser" }, // defuser

	{ (AutoBuyClassType)0, "", "" } // last one, must be at end.
};