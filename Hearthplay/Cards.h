#pragma once

#include <cstdint>
#include <vector>

enum class Card
{
	Coin,

	// 0-mana neutral cards
	Wisp,

	// 1-mana neutral cards
	AbusiveSergeant,
	AngryChicken,
	ArgentSquire,
	BloodsailCorsair,
	ElvenArcher,
	GoldshireFootman,
	GrimscaleOracle,
	HungryCrab,
	LeperGnome,
	Lightwarden,
	MurlocRaider,
	MurlocTidecaller,
	Secretkeeper,
	Shieldbearer,
	SouthseaDeckhand,
	StonetuskBoar,
	Undertaker,
	VoodooDoctor,
	WorgenInfiltrator,
	YoungDragonhawk,
	YoungPriestess,
	ZombieChow,

	// 2-mana neutral cards
	AcidicSwampOoze,
	AmaniBerserker,
	AncientWatcher,
	BloodfenRaptor,
	BloodmageThalnos,
	BloodsailRaider,
	BluegillWarrior,
	CaptainsParrot,
	CrazedAlchemist,
	DireWolfAlpha,
	Doomsayer,
	EchoingOoze,
	FaerieDragon,
	FrostwolfGrunt,
	HauntedCreeper,
	IronbeakOwl,
	KnifeJuggler,
	KoboldGeomancer,
	LootHoarder,
	LorewalkerCho,
	MadBomber,
	MadScientist,
	ManaAddict,
	ManaWraith,
	MasterSwordsmith,
	MillhouseManastorm,
	MurlocTidehunter,
	NatPagle,
	NerubarWeblord,
	NerubianEgg,
	NoviceEngineer,
	PintSizedSummoner,
	RiverCrocolisk,
	SunfuryProtector,
	UnstableGhoul,
	WildPyromancer,
	YouthfulBrewmaster,

	// 3-mana neutral cards
	AcolyteOfPain,
	AlarmoBot,
	ArcaneGolem,
	BigGameHunter,
	BloodKnight,
	ColdlightOracle,
	ColdlightSeer,
	DalaranMage,
	DancingSwords,
	Deathlord,
	Demolisher,
	EarthenRingFarseer,
	EmperorCobra,
	FlesheatingGhoul,
	HarvestGolem,
	ImpMaster,
	InjuredBlademaster,
	IronforgeRifleman,
	IronfurGrizzly,
	JunglePanther,
	KingMukla,
	MagmaRager,
	MindControlTech,
	MurlocWarleader,
	QuestingAdventurer,
	RagingWorgen,
	RaidLeader,
	RazorfenHunter,
	ScarletCrusader,
	ShadeOfNaxxramas,
	ShatteredSunCleric,
	SilverbackPatriarch,
	SouthseaCaptain,
	StoneskinGargoyle,
	TaurenWarrior,
	ThrallmarFarseer,
	TinkmasterOverspark,
	Wolfrider,

	// 4-mana neutral cards
	AncientBrewmaster,
	AncientMage,
	BaronRiverdare,
	ChillwindYeti,
	CultMaster,
	DarkIronDwarf,
	DefenderOfArgus,
	DragonlingMechanic,
	DreadCorsair,
	GnomishInventor,
	LeeroyJenkins,
	MogushanWarden,
	OasisSnapjaw,
	OgreMagi,
	OldMurkEye,
	SenjinShieldMasta,
	SilvermoonGuardian,
	Spellbreaker,
	StormwindKnight,
	TwilightDrake,
	VioletTeacher,
	WailingSoul,

	// 5-mana neutral cards
	Abomination,
	AzureDrake,
	BootyBayBodyguard,
	CaptainGreenskin,
	DarkscaleHealer,
	EliteTaurenChieftan,
	FacelessManipulator,
	FenCreeper,
	Feugen,
	FrostwolfWarlord,
	GadgetzanAuctioneer,
	GurubashiBerserker,
	HarrisonJones,
	Loatheb,
	Nightblade,
	SilverHandKnight,
	SludgeBelcher,
	SpectralKnight,
	SpitefulSmith,
	Stalaag,
	StampedingKodo,
	StormpikeCommando,
	StranglethornTiger,
	VentureCoMercenary,
	
	// 6-mana neutral cards
	Archmage,
	ArgentCommander,
	BoulderfistOgre,
	CairneBloodhoof,
	FrostElemental,
	GelbinMekkatorque,
	Hogger,
	IllidanStormrage,
	LordOfTheArena,
	Maexxna,
	PriestessOfElune,
	RecklessRocketeer,
	Sunwalker,
	SylvanasWindrunner,
	TheBeast,
	TheBlackKnight,
	WindfuryHarpy,

	// 7-mana neutral cards
	BaronGeddon,
	CoreHound,
	RavenholdtAssassin,
	StormwindChampion,
	WarGolem,

	// 8-mana neutral cards
	Gruul,
	KelThuzad,
	RagnarosTheFirelord,
	
	// 9-mana neutral cards
	Alexstrasza,
	Malygos,
	Nozdormu,
	Onyxia,
	Ysera,

	// 10-mana neutral cards
	Deathwing,
	SeaGiant,

	// 12-mana neutral cards
	MountainGiant,
	
	// 20-mana neutral cards
	MoltenGiant,

	MAX
};

enum class CardType : uint8_t
{
	Minion,
	Spell,
};

enum class MinionRace : uint8_t
{
	None,
	Beast,
	Murloc,
	Demon,
	Dragon,
	Pirate,
	Totem,
};

enum class TargetType : uint8_t
{
	None,
	AnyCharacter,
	AnyMinion,
	Opponent,
	Self,

	MAX,
};

enum class SpellEffect : uint8_t
{
	None,
	AddMana,			// Coin and Innervate
	DamageCharacter,	// Elven Archer, Stormpike Commando, Holy Smite, Arcane Shot
	HealCharacter,		// Voodoo Doctor, Earthen Ring Farseer
	AddMinionAura,		// Abusive Sergeant, Dark Iron Dwarf
};

enum class MinionAuraEffect : uint8_t
{
	None,
	BonusAttack,
};

enum class AuraDuration : uint8_t
{
	None,
	EndOfTurn,
};

struct MinionAura
{
	MinionAuraEffect	m_effect;
	uint8_t				m_param;
	AuraDuration		m_duration;

	MinionAura( )
		: m_effect( MinionAuraEffect::None )
		, m_param(0)
		, m_duration( AuraDuration::None )
	{
	}

	MinionAura(MinionAuraEffect effect, uint8_t param, AuraDuration duration)
		: m_effect(effect)
		, m_param(param)
		, m_duration(duration)
	{
	}
};

#define IMPLEMENT_FLAGS( EnumType, UnderlyingType ) \
	inline EnumType operator|( EnumType l, EnumType r ) \
		{ return (EnumType)((UnderlyingType)l | (UnderlyingType)r ); } \
	inline EnumType operator&( EnumType l, EnumType r ) \
		{ return (EnumType)((UnderlyingType)l & (UnderlyingType)r); } \
	inline EnumType& operator|=(EnumType& l, EnumType r) \
		{ l = l | r; return l; } \
	inline EnumType& operator&=(EnumType& l, EnumType r) \
		{ l = l & r; return l; } \
	inline bool HasFlag( EnumType l, EnumType r ) \
		{ return (l&r) != EnumType::None; } \
	inline EnumType operator~(EnumType r) \
		{ return (EnumType)(~(UnderlyingType)r); } 

enum class CardFlags : uint8_t
{
	None = 0x0,
	CanBeInDecks = 0x1,
};

IMPLEMENT_FLAGS(CardFlags, uint8_t);

enum class MinionAbilityFlags : uint8_t
{
	None			= 0x0,
	Taunt			= 0x1,
	DivineShield	= 0x2,
	Charge			= 0x4,
	Windfury		= 0x8,
	CannotAttack	= 0x10,
	Stealth			= 0x20,
	CannotBeTargeted = 0x40,
};

IMPLEMENT_FLAGS(MinionAbilityFlags, uint8_t);

struct SpellData
{
	SpellEffect m_effect;
	uint8_t		m_param;
	MinionAura  m_aura;
	TargetType  m_target_type;

	SpellData( )
		: m_effect(SpellEffect::None)
		, m_param(0)
		, m_aura()
		, m_target_type(TargetType::None)
	{
	}

	SpellData( SpellEffect effect, uint8_t param, TargetType target_type)
		: m_effect(effect)
		, m_param(param)
		, m_aura( )
		, m_target_type(target_type)
	{
	}

	SpellData(SpellEffect effect, uint8_t param, TargetType target_type, MinionAura aura)
		: m_effect(effect)
		, m_param(param)
		, m_aura(aura)
		, m_target_type(target_type)
	{
	}
};

struct Deathrattle : SpellData
{
	Deathrattle( )
	{
	}

	Deathrattle(SpellEffect effect, uint8_t param, TargetType target_type)
		: SpellData(effect, param, target_type)
	{
	}
};

struct Battlecry : SpellData
{
	Battlecry( )
	{
	}

	Battlecry( SpellEffect effect, uint8_t param, TargetType target_type )
		: SpellData(effect, param, target_type)
	{
	}

	Battlecry(MinionAura aura, TargetType target_type)
		: SpellData(SpellEffect::AddMinionAura, 0, target_type, aura)
	{

	}
};

struct CardData
{
	CardType			m_type;
	uint8_t				m_mana_cost;
	const char*			m_name;

	uint8_t				m_attack;
	int8_t				m_health;
	MinionAbilityFlags	m_minion_abilities;
	MinionRace			m_minion_race;

	SpellData			m_spell_data;

	Deathrattle			m_minion_deathrattle;
	Battlecry			m_minion_battlecry;

	CardFlags			m_flags;

	// Vanilla minion constructor
	CardData(uint8_t mana_cost, const char* name, uint8_t attack, uint8_t health, CardFlags card_flags = CardFlags::None, MinionRace race = MinionRace::None );
	// Minion with abilities constructor
	CardData(uint8_t mana_cost, const char* name, uint8_t attack, uint8_t health, MinionAbilityFlags minion_flags, CardFlags card_flags = CardFlags::None, MinionRace race = MinionRace::None);
	// Minion with deathrattle constructor
	CardData(uint8_t mana_cost, const char* name, uint8_t attack, uint8_t health, Deathrattle deathrattle, CardFlags card_flags = CardFlags::None, MinionRace race = MinionRace::None);
	// Minion with battlecry constructor
	CardData(uint8_t mana_cost, const char* name, uint8_t attack, uint8_t health, Battlecry battlecry, CardFlags card_flags = CardFlags::None, MinionRace race = MinionRace::None);

	// Spell constructor
	CardData(CardType type, uint8_t mana_cost, const char* name, SpellEffect effect, uint8_t effect_param, TargetType target_type, CardFlags card_flag = CardFlags::None );

	bool HasBattlecry( ) const;

};

const CardData* GetCardData(Card c);

extern std::vector<Card> DeckPossibleCards;
void FilterDeckPossibleCards( );