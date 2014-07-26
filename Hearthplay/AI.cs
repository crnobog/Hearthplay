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

        public Move ChooseMove( GameState State )
        {
            var Moves = State.GetPossibleMoves( );
            Move M = Moves[RNG.Next( 0, Moves.Count )];
            return M;
        }
    }
}
