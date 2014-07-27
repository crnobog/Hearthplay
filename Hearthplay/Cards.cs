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

    class Card
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

    static class Cards
    {
        public static readonly Card Coin = new Card
        {
            Type = CardType.Spell,
            Name = "Coin",
            ManaCost = 0,
            Effect = SpellEffects.AddMana,
            EffectParam = 1
        };

        public static readonly Card MurlocRaider = new Card
        {
            Type = CardType.Minion,
            Name = "Murloc Raider",
            ManaCost = 1,
            Attack = 2,
            Health = 1
        };
        public static readonly Card BloodfenRaptor = new Card
        {
            Type = CardType.Minion,
            Name = "Bloodfen Raptor",
            ManaCost = 2,
            Attack = 3,
            Health = 2
        };
        public static readonly Card RiverCrocolisk = new Card
        {
            Type = CardType.Minion,
            Name = "River Crocolisk",
            ManaCost = 2,
            Attack = 2,
            Health = 3
        };
        public static readonly Card MagmaRager = new Card
        {
            Type = CardType.Minion,
            Name = "Magma Rager",
            ManaCost = 3,
            Attack = 5,
            Health = 1
        };
        public static readonly Card ChillwindYeti = new Card { 
            Type = CardType.Minion, 
            Name = "Chillwind Yeti", 
            ManaCost = 4, 
            Attack = 4,
            Health = 5
        };
        public static readonly Card OasisSnapjaw = new Card
        {
            Type = CardType.Minion,
            Name = "Oasis Snapjaw",
            ManaCost = 4,
            Attack = 2,
            Health = 7
        };
        public static readonly Card BoulderfistOgre = new Card
        {
            Type = CardType.Minion,
            Name = "Boulderfist Ogre",
            ManaCost = 6,
            Attack = 6,
            Health = 7
        };
        public static readonly Card CoreHound = new Card
        {
            Type = CardType.Minion,
            Name = "Core Hound",
            ManaCost = 7,
            Attack = 9,
            Health = 5
        };
        public static readonly Card WarGolem = new Card
        {
            Type = CardType.Minion,
            Name = "War Golem",
            ManaCost = 7,
            Attack = 7,
            Health = 7
        };
    }
}
