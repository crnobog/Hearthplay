using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Hearthplay
{
    interface Character
    {
        int Attack { get; }
        int Health { get; set; }
        bool AttackedThisTurn { get; set; }
    }

    struct Hero
    {
        public int Attack { get { return 0; } }
        public int Health { get; set; }
        public int MaxHealth;
        public bool AttackedThisTurn { get; set; }
        
        public Hero Clone( )
        {
            return (Hero)MemberwiseClone( );
        }
    }

    struct Minion
    {
        public int Attack { get; set; }
        public int Health { get; set; }
        public int MaxHealth;
        public bool SummonedThisTurn;
        public bool AttackedThisTurn { get; set; }

        public CardData Card;

        public Minion Clone(  )
        {
            return (Minion)MemberwiseClone( );
        }

        public void BeginTurn( )
        {
            SummonedThisTurn = false;
            AttackedThisTurn = false;
        }
    }

    // Moves players can make
    enum MoveType
    {
        EndTurn,
        AttackMinion,
        AttackHero,
        PlayCard,
    }

    enum PlayerID
    {
        Environment,
        One,
        Two
    }

    struct Move
    {
        public MoveType Type;
        public int SourceIndex;
        public int TargetIndex;
    }

    class Player
    {
        public const int MaxMinions = 7;
        public const int MaxCardsInHand = 10;

        public List<Minion> Minions = new List<Minion>( MaxMinions );
        public Hero Hero = new Hero();
        public List<CardData> Hand = new List<CardData>( MaxCardsInHand );
        public List<CardData> Deck;
        public int Mana;
        public int MaxMana;

        public Player( List<CardData> InDeck )
        {
            Deck = InDeck;
            Hero.Health = 30;
            Hero.MaxHealth = 30;
        }

        public Player( Player ToClone )
        {
            Minions = ToClone.Minions.Select( m => m.Clone() ).ToList();
            Hero = ToClone.Hero.Clone();
            Hand = new List<CardData>( ToClone.Hand );
            Deck = new List<CardData>( ToClone.Deck );
            Mana = ToClone.Mana;
            MaxMana = ToClone.MaxMana;
        }

        public void DrawOne( )
        {
            if( Deck.Count > 0 )
            {
                Hand.Add( Deck[Deck.Count-1]);
                Deck.RemoveAt( Deck.Count - 1 );
            }
            else
            {
                // TODO fatigue
            }
        }
    }

    enum VictoryState
    {
        Undetermined,
        PlayerOneWins,
        PlayerTwoWins,
        Draw
    }

    class GameState
    {
        public int PlayerToAct = 0;
        Player[] Players = new Player[2];

        public VictoryState VictoryState = VictoryState.Undetermined;

        public static int MaxPossibleMoves()
        {
            int Num = 1; // End turn
            Num += (Player.MaxMinions + 1) * (Player.MaxMinions + 1); // All character attacking all enemy characters
            Num += (Player.MaxMinions + 1) * (Player.MaxCardsInHand); // Play a card in hand targeting each possible character

            return Num;
        }

        public GameState( List<CardData> DeckOne, List<CardData> DeckTwo )
        {
            Players[0]= new Player( DeckOne );
            for( int i = 0; i < 3; ++i )
            {
                Players[0].DrawOne( );
            }

            Players[1] = new Player( DeckTwo );
            for( int i = 0; i < 4; ++i )
            {
                Players[1].DrawOne( );
            }
            Players[1].Hand.Add( Cards.Coin );

            Players[0].DrawOne( );
            Players[0].Mana = 1;
            Players[0].MaxMana = 1;
        }

        public GameState( GameState ToClone )
        {
            PlayerToAct = ToClone.PlayerToAct;
            Players[0] = new Player( ToClone.Players[0] );
            Players[1] = new Player( ToClone.Players[1] );
        }

        void Victory( int Player )
        {
            if( Player == 0 )
            {
                VictoryState = VictoryState.PlayerOneWins;
            }
            else
            {
                VictoryState = VictoryState.PlayerTwoWins;
            }
        }

        public int GetPossibleMoves( Move[] Moves )
        {
            Player ToAct = Players[PlayerToAct];
            Player Opponent = Players[Math.Abs( PlayerToAct - 1 )];

            int NumMoves = 0;

            // Attack each target with each minion
            for( int i=0; i < ToAct.Minions.Count; ++i )
            {
                if( ToAct.Minions[i].AttackedThisTurn || ToAct.Minions[i].SummonedThisTurn )
                    continue;

                for( int j=0; j < Opponent.Minions.Count; ++j )
                {
                    // Attack minion
                    Moves[NumMoves++] = new Move { Type = MoveType.AttackMinion, SourceIndex = i, TargetIndex = j };
                }

                // Attack opponent
                Moves[NumMoves++] =  new Move { Type = MoveType.AttackHero, SourceIndex = i };
            }

            // Play each card
            for( int i = 0; i < ToAct.Hand.Count; ++i )
            {
                if( ToAct.Hand[i].ManaCost <= ToAct.Mana )
                {
                    Moves[NumMoves++] = new Move { Type = MoveType.PlayCard, SourceIndex = i };
                }
            }

            // End turn
            Moves[NumMoves++] = new Move { Type = MoveType.EndTurn };

            return NumMoves;
        }

        public void ProcessMove( Move M )
        {
            switch( M.Type )
            {
                case MoveType.EndTurn:
                    EndTurn( );
                    break;
                case MoveType.AttackMinion:
                    AttackMinion( M.SourceIndex, M.TargetIndex );
                    break;
                case MoveType.AttackHero:
                    AttackHero( M.SourceIndex);
                    break;
                case MoveType.PlayCard:
                    PlayCard( M.SourceIndex );
                    break;
            }
        }

        void EndTurn( )
        {
            PlayerToAct = Math.Abs( PlayerToAct - 1 );
            Players[PlayerToAct].DrawOne( );
            Players[PlayerToAct].Hero.AttackedThisTurn = false;
            Players[PlayerToAct].MaxMana = Math.Min( 10, Players[PlayerToAct].MaxMana + 1 );
            Players[PlayerToAct].Mana = Players[PlayerToAct].MaxMana;
            for( int i=0; i < Players[PlayerToAct].Minions.Count; ++i )
            {
                Minion m = Players[PlayerToAct].Minions[i];
                m.BeginTurn( );
                Players[PlayerToAct].Minions[i] = m;
            }

            // HACK before fatigue goes in
            if( Players[0].Deck.Count == 0 && Players[1].Deck.Count == 0 )
            {
                VictoryState = Hearthplay.VictoryState.Draw;
            }
        }

        void AttackMinion( int SourceIndex, int TargetIndex )
        {
            Player Active = Players[PlayerToAct];
            Player Opponent = Players[Math.Abs(PlayerToAct-1)];

            Minion Attacker = Active.Minions[SourceIndex];
            Minion Victim = Opponent.Minions[TargetIndex];

            Attacker.Health -= Victim.Attack;
            Victim.Health -= Attacker.Attack;

            Attacker.AttackedThisTurn = true;

            // Handle minion death
            // TODO: Refactor when handling simultaneous minion death
            if( Active.Minions[SourceIndex].Health <= 0 )
            {
                Active.Minions.RemoveAt( SourceIndex );
            }
            else
            {
                Active.Minions[SourceIndex] = Attacker;
            }

            if( Opponent.Minions[TargetIndex].Health <= 0 )
            {
                Opponent.Minions.RemoveAt( TargetIndex );
            }
            else
            {
                Opponent.Minions[TargetIndex] = Victim;
            }
        }

        void AttackHero( int SourceIndex )
        {
            Player Active = Players[PlayerToAct];
            Player Opponent = Players[Math.Abs( PlayerToAct - 1 )];

            Minion Attacker = Active.Minions[SourceIndex];
            Attacker.AttackedThisTurn = true;

            Opponent.Hero.Health -= Attacker.Attack;

            Active.Minions[SourceIndex] = Attacker;

            if( Opponent.Hero.Health <= 0 )
            {
                Victory( PlayerToAct );
            }
        }

        void PlayCard( int SourceIndex )
        {
            Player ToAct = Players[PlayerToAct];
            CardData ToPlay = ToAct.Hand[SourceIndex];
            ToAct.Hand.RemoveAt( SourceIndex );

            ToAct.Mana -= ToPlay.ManaCost;

            if( ToPlay.Type == CardType.Minion )
            {
                ToAct.Minions.Add( new Minion
                {
                    Card = ToPlay,
                    Attack = ToPlay.Attack,
                    Health = ToPlay.Health,
                    MaxHealth = ToPlay.Health,
                    SummonedThisTurn = true,
                    AttackedThisTurn = false
                } );
            }
            else
            {
                switch( ToPlay.Effect )
                {
                    case SpellEffects.AddMana:
                        ToAct.Mana += ToPlay.EffectParam;
                        break;
                }
            }
        }

        public string DescribeMove( Move M )
        {
            Player ToAct = Players[PlayerToAct];
            Player Opponent = Players[Math.Abs(PlayerToAct-1)];

            string Prefix = String.Format( "Player {0} ({1}) ({2}/{3})", PlayerToAct, ToAct.Hero.Health, ToAct.Mana, ToAct.MaxMana );

            switch( M.Type )
            {
                case MoveType.EndTurn:
                    return String.Format( "{0}: End turn", Prefix );
                case MoveType.AttackMinion:
                    return String.Format( "{2}: Attack {0} with {1}", 
                        Opponent.Minions[M.TargetIndex].Card.Name,
                        ToAct.Minions[M.SourceIndex].Card.Name ,
                        Prefix
                        );
                case MoveType.AttackHero:
                    return String.Format( "{0}: Attack opponent with {1}", Prefix, ToAct.Minions[M.SourceIndex].Card.Name );
                case MoveType.PlayCard:
                    return String.Format( "{1}: Play {0}", Players[PlayerToAct].Hand[M.SourceIndex].Name,
                        Prefix );
                default: return "???";
            }
        }
    }
}
