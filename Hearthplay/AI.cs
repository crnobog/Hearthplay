using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Hearthplay
{
    interface AI
    {
        Move ChooseMove( GameState State );
    }

    class RandomAI : AI
    {
        Random RNG = new Random( );
        Move[] MoveBuffer;

        public RandomAI( Move[] InMoveBuffer )
        {
            MoveBuffer = InMoveBuffer;
        }

        public Move ChooseMove( GameState State )
        {
            int Moves = State.GetPossibleMoves( MoveBuffer );
            Move M = MoveBuffer[RNG.Next( 0, Moves )];
            return M;
        }
    }

    class CheatingMCTS : AI
    {
        class Node
        {
            public Node Parent;
            public Move Move; // The move used to reach this node from its parent

            public List<Move> UntriedMoves;
            public List<Node> Children = new List<Node>();

            public int Visits;
            public int Wins;

            public Node( Node InParent, Move[] MoveBuffer, int NumMoves )
            {
                Parent = InParent;
                UntriedMoves = new List<Move>( MoveBuffer.Take( NumMoves ) ); //ugh
            }

            public Node SelectChild( )
            {
                Node BestChild = Children[0];
                float BestScore = 0;

                for( int i=1; i < Children.Count; ++i )
                {
                    Node C = Children[i];
                    float Score = C.Wins / (float)C.Visits + (float)Math.Sqrt( Math.Log( Visits ) / C.Visits ); // TODO tiebreak?
                    if( Score > BestScore )
                    {
                        BestChild = C;
                        BestScore = Score;
                    }
                }

                return BestChild;
            }
        };

        Random RNG = new Random( );
        Move[] MoveBuffer;
        int Iterations;

        public CheatingMCTS( Move[] InMoveBuffer, int InIterations )
        {
            MoveBuffer = InMoveBuffer;
            Iterations = InIterations;
        }

        public Move ChooseMove( GameState AuthoritativeState )
        {
            VictoryState VictoryCondition = AuthoritativeState.PlayerToAct == 0 ? VictoryState.PlayerOneWins : VictoryState.PlayerTwoWins;

            int Moves = AuthoritativeState.GetPossibleMoves( MoveBuffer );
            Node Root = new Node( null, MoveBuffer, Moves );
            for( int i=0; i < Iterations; ++i )
            {
                GameState SimState = AuthoritativeState.Clone( );
                Node N = Root;
                while( N.UntriedMoves.Count > 0 && N.Children.Count != 0 )
                {
                    // Select a child
                    N = N.SelectChild( );
                    // Mirror the move
                    SimState.ProcessMove( N.Move );
                }

                // Pick an untried move and add a child
                Move M = N.UntriedMoves[RNG.Next( 0, N.UntriedMoves.Count )];
                SimState.ProcessMove( M );
                Moves = SimState.GetPossibleMoves( MoveBuffer );
                Node Child = new Node( N, MoveBuffer, Moves );
                Child.Move = M;

                N.Children.Add( Child );

                SimState.PlayOutRandomly( RNG );

                bool Won = SimState.VictoryState == VictoryCondition;
                N = Child;
                while( N != null )
                {
                    N.Visits += 1;
                    if( Won ) N.Wins += 1;

                    N = N.Parent;
                }
            }

            int MaxVisits = 0;
            Node BestChild = null;
            foreach( Node C in Root.Children )
            {
                if( C.Visits > MaxVisits )
                {
                    MaxVisits = C.Visits;
                    BestChild = C;
                }
            }
            return BestChild.Move;
        }
    }
}
