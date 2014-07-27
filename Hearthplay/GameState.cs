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

    class Hero : Character
    {
        public int Attack { get { return 0; } }
        public int Health { get; set; }
        public int MaxHealth;
        public int Mana;
        public int MaxMana;
        public bool AttackedThisTurn { get; set; }
        
        public Hero Clone( )
        {
            return (Hero)MemberwiseClone( );
        }

        public void BeginTurn( )
        {
            MaxMana = Math.Min( MaxMana + 1, 10 );
            Mana = MaxMana;
            AttackedThisTurn = false;
        }
    }

    class Minion : Character
    {
        public int Attack { get; set; }
        public int Health { get; set; }
        public int MaxHealth;
        public bool SummonedThisTurn;
        public bool AttackedThisTurn { get; set; }

        public Card Card;

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
        Attack,
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
        public List<Card> Hand = new List<Card>( MaxCardsInHand );
        public List<Card> Deck;

        public Player( List<Card> InDeck )
        {
            Deck = InDeck;
            Hero.Health = 30;
            Hero.MaxHealth = 30;
        }

        public Player( Player ToClone )
        {
            Minions = ToClone.Minions.Select( m => m.Clone() ).ToList();
            Hero = ToClone.Hero.Clone();
            Hand = new List<Card>( ToClone.Hand );
            Deck = new List<Card>( ToClone.Deck );
        }

        public void DrawOne( )
        {
            if( Deck.Count > 0 )
            {
                Hand.Add( Deck.Last( ) );
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

        public GameState( List<Card> DeckOne, List<Card> DeckTwo )
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

            Players[PlayerToAct].DrawOne( );
            Players[PlayerToAct].Hero.BeginTurn( );
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
                    Moves[NumMoves++] = new Move { Type = MoveType.Attack, SourceIndex = i + 1, TargetIndex = j + 1 };
                }

                // Attack opponent
                Moves[NumMoves++] =  new Move { Type = MoveType.Attack, SourceIndex = i + 1, TargetIndex = 0 };
            }

            // Play each card
            for( int i = 0; i < ToAct.Hand.Count; ++i )
            {
                if( ToAct.Hand[i].ManaCost <= ToAct.Hero.Mana )
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
                    PlayerToAct = Math.Abs(PlayerToAct-1);
                    Players[PlayerToAct].DrawOne( );
                    Players[PlayerToAct].Hero.BeginTurn( );
                    foreach( Minion m in Players[PlayerToAct].Minions )
                    {
                        m.BeginTurn( );
                    }

                    // HACK before fatigue goes in
                    if( Players[0].Deck.Count == 0 && Players[1].Deck.Count == 0 )
                    {
                        VictoryState = Hearthplay.VictoryState.Draw;
                    }
                    break;
                case MoveType.Attack:
                    Attack( M.SourceIndex, M.TargetIndex );
                    break;
                case MoveType.PlayCard:
                    PlayCard( M.SourceIndex );
                    break;
            }
        }

        void Attack( int SourceIndex, int TargetIndex )
        {
            Player ToAct = Players[PlayerToAct];
            Player Opponent = Players[Math.Abs(PlayerToAct-1)];

            Character Source = SourceIndex == 0 ? (Character)ToAct.Hero : ToAct.Minions[SourceIndex - 1];
            Character Target = TargetIndex == 0 ? (Character)Opponent.Hero : Opponent.Minions[ TargetIndex - 1 ];

            Source.Health -= Target.Attack;
            Target.Health -= Source.Attack;

            Source.AttackedThisTurn = true;

            // Handle player death
            if( SourceIndex == 0 || TargetIndex == 0 )
            {
                if( Source.Health < 0 && Target.Health < 0 )
                {
                    VictoryState = VictoryState.Draw;
                    return;
                }
                else if( Source.Health < 0 )
                {
                    Victory( Math.Abs( PlayerToAct - 1 ) );
                }
                else if( Target.Health < 0 )
                {
                    Victory( PlayerToAct );
                }
            }

            // Handle minion death
            // TODO: Refactor when handling simultaneous minion death
            if( Source.Health < 0 )
            {
                if( SourceIndex != 0 )
                {
                    ToAct.Minions.RemoveAt( SourceIndex - 1 );
                }
            }

            if( Target.Health < 0 )
            {
                if( TargetIndex != 0 )
                {
                    Opponent.Minions.RemoveAt( TargetIndex - 1 );
                }
            }
        }

        void PlayCard( int SourceIndex )
        {
            Player ToAct = Players[PlayerToAct];
            Card ToPlay = ToAct.Hand[SourceIndex];
            ToAct.Hand.RemoveAt( SourceIndex );

            ToAct.Hero.Mana -= ToPlay.ManaCost;

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

        public string DescribeMove( Move M )
        {
            Player ToAct = Players[PlayerToAct];
            Player Opponent = Players[Math.Abs(PlayerToAct-1)];

            switch( M.Type )
            {
                case MoveType.EndTurn:
                    return String.Format( "Player {0}: End turn", PlayerToAct );
                case MoveType.Attack:
                    return String.Format( "Player {2}: Attack {0} with {1}", 
                        M.TargetIndex == 0 ? "Opponent" : Opponent.Minions.ElementAt(M.TargetIndex-1).Card.Name,
                        M.SourceIndex == 0 ? "Hero" : ToAct.Minions.ElementAt(M.SourceIndex-1).Card.Name ,
                        PlayerToAct
                        );
                case MoveType.PlayCard:
                    return String.Format( "Player {1}: Play {0}", Players[PlayerToAct].Hand[M.SourceIndex].Name,
                        PlayerToAct );
                default: return "???";
            }
        }
    }
}
