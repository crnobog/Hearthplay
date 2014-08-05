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

enum class SpellEffect
{
	None,
	AddMana, // Coin and Innervate
};

enum class CardFlags
{
	None = 0x0,
	NotFullyImplemented = 0x1,
};

inline CardFlags operator|(CardFlags l, CardFlags r)
{
	return (CardFlags)((int)l | (int)r);
}

inline CardFlags operator&(CardFlags l, CardFlags r)
{
	return (CardFlags)((int)l & (int)r);
}


enum class MinionCardFlags
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

inline MinionCardFlags operator|(MinionCardFlags l, MinionCardFlags r)
{
	return (MinionCardFlags)((int)l | (int)r);
}

inline MinionCardFlags operator&(MinionCardFlags l, MinionCardFlags r)
{
	return (MinionCardFlags)((int)l & (int)r);
}


struct CardData
{
	CardType Type;
	uint8_t ManaCost;
	const char* Name;

	uint8_t Attack;
	int8_t Health;
	MinionCardFlags MinionFlags;

	SpellEffect Effect;
	uint8_t EffectParam;

	CardFlags Flags;

	// Vanilla minion constructor
	CardData(uint8_t mana_cost, const char* name, uint8_t attack, uint8_t health, CardFlags card_flags = CardFlags::NotFullyImplemented );
	// Minion with abilities constructor
	CardData(uint8_t mana_cost, const char* name, uint8_t attack, uint8_t health, MinionCardFlags minion_flags, CardFlags card_flags = CardFlags::NotFullyImplemented );

	// Spell constructor
	CardData(CardType type, uint8_t mana_cost, const char* name, SpellEffect effect, uint8_t effect_param, CardFlags card_flag = CardFlags::NotFullyImplemented );
};

const CardData* GetCardData(Card c);

extern std::vector<Card> DeckPossibleCards;
void FilterDeckPossibleCards( );