using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Hearthplay
{
    class Program
    {
        class TrialRunner
        {
            Func<AI>[] CreatePlayer = new Func<AI>[2];
            Card[][] Decks = new Card[2][];
            public int Trials;
            public int[] Wins = new int[2];

            public TrialRunner( Func<AI> PlayerOne, Func<AI> PlayerTwo, Card[] DeckOne, Card[] DeckTwo )
            {
                CreatePlayer[0] = PlayerOne;
                CreatePlayer[1] = PlayerTwo;

                Decks[0] = DeckOne;
                Decks[1] = DeckTwo;
            }

            public void RunTrial( )
            {
                // Set up initial game state
                Random R = new Random( );
                GameState AuthoritativeState = new GameState( new List<Card>( Decks[0] ).Shuffle( R ), new List<Card>( Decks[1] ).Shuffle( R ) );

                AI[] Players = new AI[2];
                Players[0] = CreatePlayer[0]( );
                Players[1] = CreatePlayer[1]( );
                GameState[] Views = new GameState[2];
                Views[0] = new GameState( AuthoritativeState );
                Views[1] = new GameState( AuthoritativeState );

                while( AuthoritativeState.VictoryState == VictoryState.Undetermined )
                {
                    int ToAct = AuthoritativeState.PlayerToAct;
                    Move M = Players[ToAct].ChooseMove( Views[ToAct] );
                    Console.WriteLine( AuthoritativeState.DescribeMove( M ) );
                    AuthoritativeState.ProcessMove( M );
                    Views[0].ProcessMove( M );
                    Views[1].ProcessMove( M );
                }

                switch( AuthoritativeState.VictoryState )
                {
                    case VictoryState.PlayerOneWins:
                        Wins[0]++; break;
                    case VictoryState.PlayerTwoWins:
                        Wins[1]++; break;
                    case VictoryState.Draw:
                        break;
                }

                ++Trials;

                Console.WriteLine( "Result {0}", AuthoritativeState.VictoryState );
                Console.ReadLine( );
            }
        }

        static void Main(string[] args)
        {
            // Build deck
            Card[] Deck = 
            { 
                Cards.MurlocRaider, Cards.MurlocRaider, Cards.MurlocRaider,
                Cards.RiverCrocolisk, Cards.RiverCrocolisk, Cards.RiverCrocolisk,
                Cards.BloodfenRaptor, Cards.BloodfenRaptor, Cards.BloodfenRaptor,
                Cards.BloodfenRaptor, Cards.BloodfenRaptor, Cards.BloodfenRaptor,
                Cards.MagmaRager, Cards.MagmaRager, Cards.MagmaRager, 
                Cards.ChillwindYeti, Cards.ChillwindYeti, Cards.ChillwindYeti, 
                Cards.OasisSnapjaw, Cards.OasisSnapjaw, Cards.OasisSnapjaw,
                Cards.BoulderfistOgre, Cards.BoulderfistOgre, Cards.BoulderfistOgre, 
                Cards.CoreHound, Cards.CoreHound, Cards.CoreHound,
                Cards.WarGolem, Cards.WarGolem, Cards.WarGolem, 
            };

            Move[][] MoveBuffers = new Move[2][];
            MoveBuffers[0] = new Move[GameState.MaxPossibleMoves()];
            MoveBuffers[1] = new Move[GameState.MaxPossibleMoves()];
            TrialRunner T = new TrialRunner( 
                ( ) => new RandomAI( MoveBuffers[0] ), 
                ( ) => new RandomAI( MoveBuffers[1] ),
                Deck,
                Deck );

            var Timer = System.Diagnostics.Stopwatch.StartNew( );
            for( int i=0; i < 100000; ++i )
            {
                T.RunTrial( );
            }
            Timer.Stop( );
            Console.WriteLine( "Player one: {0}% ", 100*(T.Wins[0] / (double)T.Trials) );
            Console.WriteLine( "Player two: {0}% ", 100*(T.Wins[1] / (double)T.Trials) );
            Console.WriteLine( "{0} trials in {1} seconds", T.Trials, Timer.Elapsed.TotalSeconds );

            Console.ReadLine( );
        }
    }
}
