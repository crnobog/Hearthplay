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
            public string[] PlayerNames = new string[2];
            Func<AI>[] CreatePlayer = new Func<AI>[2];
            CardData[][] Decks = new CardData[2][];
            public int Trials;
            public int[] Wins = new int[2];

            public TrialRunner( Func<AI> PlayerOne, Func<AI> PlayerTwo, CardData[] DeckOne, CardData[] DeckTwo )
            {
                CreatePlayer[0] = PlayerOne;
                CreatePlayer[1] = PlayerTwo;

                Decks[0] = DeckOne;
                Decks[1] = DeckTwo;

                PlayerNames[0] = PlayerOne().ToString();
                PlayerNames[1] = PlayerTwo().ToString();
            }

            public void RunTrial( bool SwitchOrder )
            {
                // Set up initial game state
                Random R = new Random( );
                GameState AuthoritativeState = new GameState( new List<CardData>( Decks[0] ).Shuffle( R ), new List<CardData>( Decks[1] ).Shuffle( R ) );

                AI[] Players = new AI[2];
                Players[0] = CreatePlayer[0]( );
                Players[1] = CreatePlayer[1]( );
                GameState[] Views = new GameState[2];

                while( AuthoritativeState.VictoryState == VictoryState.Undetermined )
                {
                    int ToAct = AuthoritativeState.PlayerToAct;
                    ToAct = SwitchOrder ? Math.Abs( ToAct - 1 ) : ToAct;
                    Move M = Players[ToAct].ChooseMove( AuthoritativeState );
                    //Console.WriteLine( AuthoritativeState.DescribeMove( M ) );
                    AuthoritativeState.ProcessMove( M );
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

                //Console.WriteLine( "Result {0}", AuthoritativeState.VictoryState );
                //Console.ReadLine( );
            }
        }

        static void Main(string[] args)
        {
            // Build deck
            Card[] Deck = 
            { 
                Card.MurlocRaider, Card.MurlocRaider, Card.MurlocRaider,
                Card.RiverCrocolisk, Card.RiverCrocolisk, Card.RiverCrocolisk,
                Card.BloodfenRaptor, Card.BloodfenRaptor, Card.BloodfenRaptor,
                Card.BloodfenRaptor, Card.BloodfenRaptor, Card.BloodfenRaptor,
                Card.MagmaRager, Card.MagmaRager, Card.MagmaRager, 
                Card.ChillwindYeti, Card.ChillwindYeti, Card.ChillwindYeti, 
                Card.OasisSnapjaw, Card.OasisSnapjaw, Card.OasisSnapjaw,
                Card.BoulderfistOgre, Card.BoulderfistOgre, Card.BoulderfistOgre, 
                Card.CoreHound, Card.CoreHound, Card.CoreHound,
                Card.WarGolem, Card.WarGolem, Card.WarGolem, 
            };
            CardData[] DeckData = Deck.Select( c => Cards.AllCards[(int)c] ).ToArray();

            Move[][] MoveBuffers = new Move[2][];
            MoveBuffers[0] = new Move[GameState.MaxPossibleMoves()];
            MoveBuffers[1] = new Move[GameState.MaxPossibleMoves()];

            for( int i=0; i < 2; ++i )
            {
                TrialRunner T = new TrialRunner(
                    ( ) => new RandomAI( MoveBuffers[0] ),
                    ( ) => new CheatingMCTS( MoveBuffers[1], 1000 ),
                    //( ) => new RandomAI( MoveBuffers[1] ),
                    DeckData,
                    DeckData );

                var Timer = System.Diagnostics.Stopwatch.StartNew( );
                for( int j = 0; j < 1; ++j )
                {
                    T.RunTrial( i == 1 );
                }
                Timer.Stop( );
                Console.WriteLine( "Player one - {1}: {0}% ", 100 * (T.Wins[0] / (double)T.Trials), T.PlayerNames[0] );
                Console.WriteLine( "Player two - {1}: {0}% ", 100 * (T.Wins[1] / (double)T.Trials), T.PlayerNames[1] );
                Console.WriteLine( "{0} trials in {1} seconds", T.Trials, Timer.Elapsed.TotalSeconds );
            }

            Console.ReadLine( );
        }
    }
}
