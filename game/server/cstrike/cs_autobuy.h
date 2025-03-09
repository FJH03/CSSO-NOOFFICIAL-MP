//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Headers and defines for Autobuy and Rebuy 
//
//=============================================================================//

/**
 * Weapon classes as used by the AutoBuy
 * Has to be different that the previous ones because these are bitmasked values as a weapon can be from
 * more than one class.  This also includes all the classes of equipment that a player can buy.
 */
 #include "cs_weapon_parse.h"
 enum AutoBuyClassType
 {
	 AUTOBUYCLASS_PRIMARY = 1,
	 AUTOBUYCLASS_SECONDARY = 2,
	 AUTOBUYCLASS_AMMO = 4,
	 AUTOBUYCLASS_ARMOR = 8,
	 AUTOBUYCLASS_DEFUSER = 16,
	 AUTOBUYCLASS_PISTOL = 32,
	 AUTOBUYCLASS_SMG = 64,
	 AUTOBUYCLASS_RIFLE = 128,
	 AUTOBUYCLASS_SNIPERRIFLE = 256,
	 AUTOBUYCLASS_SHOTGUN = 512,
	 AUTOBUYCLASS_MACHINEGUN = 1024,
	 AUTOBUYCLASS_GRENADE = 2048,
	 AUTOBUYCLASS_NIGHTVISION = 4096,
 };
 
 struct AutoBuyInfoStruct
 {
	 AutoBuyClassType m_class;
	 const char *m_command;
	 const char *m_classname;
 };
 
 class RebuyStruct
 {
 
 private:
	 CSWeaponID	m_nPrimaryId;
	 CSWeaponID	m_nSecondaryId;
	 CSWeaponID	m_tertiaryId;				// used just for taser right now
	 CSWeaponID	m_grenades[8];
	 int			m_armor;					// 0, 1, or 2 (0 = none, 1 = kevlar, 2 = kevlar + helmet)
	 bool		m_defuser;					// do we want a defuser
	 bool		m_nightVision;				// do we want night vision
	 bool		m_isNotEmpty;					
 
 public:
	 RebuyStruct()
	 {
		 Clear();
	 }
 
	 void Clear()
	 {
		 memset(this, 0, sizeof(RebuyStruct));
	 }
 
	 bool isEmpty( void )
	 {
		 return !m_isNotEmpty;
	 }
 
	 void SetPrimary( CSWeaponID nPrimaryPos )
	 {
		 m_nPrimaryId = nPrimaryPos;
		 m_isNotEmpty = true;
	 }
 
	 CSWeaponID GetPrimary( void )
	 {
		 return m_nPrimaryId;
	 }
 
	 void SetSecondary( CSWeaponID nSecondaryPos )
	 {
		 m_nSecondaryId = nSecondaryPos;
		 m_isNotEmpty = true;
	 }
 
	 CSWeaponID GetSecondary( void )
	 {
		 return m_nSecondaryId;
	 }
 
	 void SetTertiary( CSWeaponID tertiary )
	 {
		 m_tertiaryId = tertiary;
		 m_isNotEmpty = true;
	 }
 
	 CSWeaponID GetTertiary( void )
	 {
		 return m_tertiaryId;
	 }
 
	 void SetGrenade( int index, CSWeaponID grenade )
	 {
		 m_grenades[ index ] = grenade;
		 m_isNotEmpty = true;
	 }
 
	 CSWeaponID GetGrenade( int index )
	 {
		 return m_grenades[index];
	 }
 
	 void SetArmor( int armor )
	 {
		 m_armor = armor;
		 m_isNotEmpty = true;
	 }
 
	 int GetArmor( void )
	 {
		 return m_armor;
	 }
 
	 void SetDefuser( bool defuser )
	 {
		 m_defuser = defuser;
		 m_isNotEmpty = true;
	 }
 
	 bool GetDefuser( void )
	 {
		 return m_defuser;
	 }
 
	 void SetNightVision( bool nv )
	 {
		 m_nightVision = nv;
		 m_isNotEmpty = true;
	 }
 
	 bool GetNightVision( void )
	 {
		 return m_nightVision;
	 }
 
	 int numGrenades( void )
	 {
		 return ARRAYSIZE( m_grenades );
	 }
 
 };
 
 extern AutoBuyInfoStruct g_autoBuyInfo[];