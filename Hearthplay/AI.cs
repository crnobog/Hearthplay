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
}
