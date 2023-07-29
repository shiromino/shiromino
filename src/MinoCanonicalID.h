#pragma once
#include "SPM_Structures.h"

namespace Shiro {
    namespace Mino {
        // The Shiromino Project's axiomatic list of names and encoded IDs for
        // polyominoes up to degree 6

        /*
         * I'm creating this to solve a design issue that arose as a result of
         * writing the SPM code to be hyper-generic. These IDs should be used
         * wherever appropriate in the code base for Shiromino.
         *
         * Names for all polyominoes of degree 5 and higher were arbitrarily
         * chosen by myself, within the set of all possible names based on
         * letters of the Latin alphabet.
         *
         * These numbers are 1-indexed because 0 is reserved for the empty cell
         * encoding for playfield objects. All entries therefore also must be
         * explicitly assigned a positive integral value.
         *
         * -Felicity Violette
        */

        enum CanonicalID : SPM_minoID
        {
            // monomino
            O1 = 1,

            // domino
            I2 = 2,

            // triminoes
            I3 = 3,
            V3 = 4,

            // tetrominoes
            I4 = 5,
            T4 = 6,
            J4 = 7,
            L4 = 8,
            O4 = 9,
            S4 = 10,
            Z4 = 11,

            // pentominoes
            I5 = 12,
            J5 = 13,
            L5 = 14,
            X5 = 15,
            S5 = 16,
            Z5 = 17,
            N5 = 18,
            G5 = 19,
            U5 = 20,
            T5 = 21,
            Fa5 = 22,
            Fb5 = 23,
            P5 = 24,
            Q5 = 25,
            W5 = 26,
            Ya5 = 27,
            Yb5 = 28,
            V5 = 29,

            // hexominoes
            // mirror-symmetric (non-chiral)
            I6 = 30,
            T6 = 31,
            O6 = 32,
            D6 = 33,
            U6 = 34,
            X6 = 35,
            Y6 = 36,
            OX6 = 37,
            TI6 = 38,
            WV6 = 39,

            // mirror-asymmetric (chiral) - listed in mirror pairs
            J6 = 40,
            L6 = 41,
            S6 = 42,
            Z6 = 43,
            Fa6 = 44,
            Fb6 = 45,
            Ha6 = 46,
            Hb6 = 47,
            Wa6 = 48,
            Wb6 = 49,
            
            JF6 = 50,
            LF6 = 51,
            JI6 = 52,
            LI6 = 53,
            JO6 = 54,
            LO6 = 55,
            NI6 = 56,
            GI6 = 57,
            PI6 = 58,
            QI6 = 59,

            PJ6 = 60,
            QL6 = 61,
            TJ6 = 62,
            TL6 = 63,
            US6 = 64,
            UZ6 = 65,
            VJ6 = 66,
            VL6 = 67,
            VS6 = 68,
            VZ6 = 69,

            WJ6 = 70,
            WL6 = 71,
            WS6 = 72,
            WZ6 = 73,
            YJ6 = 74,
            YL6 = 75,
            SI6 = 76,
            ZI6 = 77,
            SO6 = 78,
            ZO6 = 79,

            TVa6 = 80,
            TVb6 = 81,
            VFa6 = 82,
            VFb6 = 83,
            WIa6 = 84,
            WIb6 = 85,
            XIa6 = 86,
            XIb6 = 87,
            YIa6 = 88,
            YIb6 = 89,

            // invalid mino (error ID)
            Err = 0x7FFFFFFF
        };
    }
}