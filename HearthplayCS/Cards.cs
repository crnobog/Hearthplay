using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Hearthplay
{
    enum CardType
    {
        Minion,
        Spell,
    }

    enum SpellEffects
    {
        AddMana,
    }

    class CardData
    {
        // For all cards
        public CardType Type;
        public string Name;
        public int ManaCost;

        // For minions
        public int Attack;
        public int Health;

        // For spells, maybe battlecries later
        public SpellEffects Effect;
        public int EffectParam;
    }

    enum Card
    {
        Unknown,
        Coin,
        MurlocRaider,
        BloodfenRaptor,
        RiverCrocolisk,
        MagmaRager,
        ChillwindYeti,
        OasisSnapjaw,
        BoulderfistOgre,
        CoreHound,
        WarGolem
    }

    static class Cards
    {
       public static readonly List<CardData> AllCards;

        static Cards( )
        {
            AllCards = new List<CardData>( );
            foreach( string Name in Enum.GetNames(typeof(Card)) )
            {
                AllCards.Add( (CardData) typeof( Cards ).GetField( Name ).GetValue( null ) );
            }
        }

        public static readonly CardData Unknown = new CardData
        {
        };

        public static readonly CardData Coin = new CardData
        {
            Type = CardType.Spell,
            Name = "Coin",
            ManaCost = 0,
            Effect = SpellEffects.AddMana,
            EffectParam = 1
        };

        public static readonly CardData MurlocRaider = new CardData
        {
            Type = CardType.Minion,
            Name = "Murloc Raider",
            ManaCost = 1,
            Attack = 2,
            Health = 1
        };
        public static readonly CardData BloodfenRaptor = new CardData
        {
            Type = CardType.Minion,
            Name = "Bloodfen Raptor",
            ManaCost = 2,
            Attack = 3,
            Health = 2
        };
        public static readonly CardData RiverCrocolisk = new CardData
        {
            Type = CardType.Minion,
            Name = "River Crocolisk",
            ManaCost = 2,
            Attack = 2,
            Health = 3
        };
        public static readonly CardData MagmaRager = new CardData
        {
            Type = CardType.Minion,
            Name = "Magma Rager",
            ManaCost = 3,
            Attack = 5,
            Health = 1
        };
        public static readonly CardData ChillwindYeti = new CardData { 
            Type = CardType.Minion, 
            Name = "Chillwind Yeti", 
            ManaCost = 4, 
            Attack = 4,
            Health = 5
        };
        public static readonly CardData OasisSnapjaw = new CardData
        {
            Type = CardType.Minion,
            Name = "Oasis Snapjaw",
            ManaCost = 4,
            Attack = 2,
            Health = 7
        };
        public static readonly CardData BoulderfistOgre = new CardData
        {
            Type = CardType.Minion,
            Name = "Boulderfist Ogre",
            ManaCost = 6,
            Attack = 6,
            Health = 7
        };
        public static readonly CardData CoreHound = new CardData
        {
            Type = CardType.Minion,
            Name = "Core Hound",
            ManaCost = 7,
            Attack = 9,
            Health = 5
        };
        public static readonly CardData WarGolem = new CardData
        {
            Type = CardType.Minion,
            Name = "War Golem",
            ManaCost = 7,
            Attack = 7,
            Health = 7
        };
    }
}
