using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Hearthplay
{
    static class Extensions
    {
        public static void Swap<T>( this List<T> L, int a, int b )
        {
            T tmp = L[a];
            L[a] = L[b];
            L[b] = tmp;
        }

        public static List<T> Shuffle<T>( this List<T> L, Random R )
        {
            for( int i=0; i < L.Count; ++i )
            {
                L.Swap( i, R.Next( i, L.Count ) );
            }
            return L;
        }
    }
}
