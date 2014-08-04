#pragma once

#include <cstdint>
#include <vector>

enum class Card
{
	Unknown,
	Coin,

	// 1-mana neutral cards
	Wisp,
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
	SilverhandKnight,
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
	AddMana, // Coin and Innervate
};

struct CardData
{
	CardType Type;
	uint8_t ManaCost;
	const char* Name;

	uint8_t Attack;
	uint8_t Health;

	SpellEffect Effect;
	uint8_t EffectParam;
};

extern const CardData AllCards[];

extern const std::vector<Card> DeckPossibleCards; // Cards that have been implemented

const CardData* GetCardData(Card c);
