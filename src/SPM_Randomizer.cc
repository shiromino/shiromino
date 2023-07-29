#include "SPM_Randomizer.h"
#include "SPM_Structures.h"
#include "MinoCanonicalID.h"
#include "PieceIDMacros.h"

SPM_minoID arsIDConversion(SPM_minoID t)
{
    switch(t)
    {
        case A_ARS_I:
            return Shiro::Mino::I4;
        case A_ARS_Z:
            return Shiro::Mino::Z4;
        case A_ARS_S:
            return Shiro::Mino::S4;
        case A_ARS_J:
            return Shiro::Mino::J4;
        case A_ARS_L:
            return Shiro::Mino::L4;
        case A_ARS_O:
            return Shiro::Mino::O4;
        case A_ARS_T:
            return Shiro::Mino::T4;

        default:
            break;
    }

    return Shiro::Mino::Err;
}

void history_push(SPM_minoID *history, std::size_t histLen, SPM_minoID t)
{
    for(unsigned int i = 0; i < histLen - 1; i++)
    {
        history[i] = history[i + 1];
    }

    history[histLen - 1] = t;
}

SPM_minoID history_pop(SPM_minoID *history)
{
    SPM_minoID t = history[0];
    history[0] = Shiro::Mino::Err;

    return t;
}

bool in_history(SPM_minoID *history, std::size_t histLen, SPM_minoID t)
{
    for(unsigned i = 0; i < histLen; i++)
    {
        if(history[i] == t)
        {
            return true;
        }
    }

    return false;
}

SPM_randomSeed g123_readRand(SPM_randomSeed& seed)
{
    seed = ((seed - 12345) * 0xeeb9eb65);
    return (seed >> 10) & 0x7fff;
}

SPM_minoID g123_getInitPiece(SPM_randomSeed& seed)
{
    SPM_minoID t = 1;

    while(t == 1 || t == 2 || t == 5)
    {
        t = g123_readRand(seed) % 7;
    }

    return t;
}

G3_Randomizer::G3_Randomizer()
    : SPM_Randomizer(7)
{
    seed = 0;

    for(int i = 0; i < 4; i++)
    {
        history[i] = Shiro::Mino::Err;
    }

    for(int i = 0; i < 35; i++)
    {
        bag[i] = Shiro::Mino::Err;
    }

    for(int i = 0; i < 7; i++)
    {
        histogram[i] = 0;
    }
}

void G3_Randomizer::init(SPM_randomSeed seed)
{
    this->seed = seed;

    for(int i = 0; i < 35; i++)
    {
        bag[i] = i / 5;
    }

    history[0] = A_ARS_S;
    history[1] = A_ARS_S;
    history[2] = A_ARS_Z;
    history[3] = g123_getInitPiece(seed);

    // move init piece to history[0] (first preview/next piece) and fill in 3 pieces ahead
    // the S,S,Z get pulled into the void
    pull(); pull(); pull();
}

SPM_minoID G3_Randomizer::pull()
{
    SPM_minoID t = getNext();
    SPM_minoID result = history_pop(history);

    history_push(history, 4, t);

    return arsIDConversion(result);
}

SPM_minoID G3_Randomizer::getNext()
{
    SPM_minoID piece = Shiro::Mino::Err;
    unsigned int bagpos = 0;
    int i = 0;

    for(i = 0; i < 6; i++)
    {
        bagpos = g123_readRand(seed) % 35;
        piece = bag[bagpos];

        // piece is not in the history, this is fine
        if(!in_history(history, 4, piece))
        {
            break;
        }

        // piece is already in the history, churn the bag a little and reroll
        bag[bagpos] = mostDroughtedPiece();

        // We might be about to fall out of the loop, pick something at random
        bagpos = g123_readRand(seed) % 35;
        piece = bag[bagpos];
    }

    // make the pieces we didn't pick more likely in future, and this piece less
    for(i = 0; i < 7; i++)
    {
        if(i == (int)(piece))
        {
            histogram[i] = 0;
        }
        else
        {
            histogram[i]++;
        }
    }

    // piece has finally be chosen, histogram is up to date, put the rarest piece back into the bag and return.
    bag[bagpos] = mostDroughtedPiece();

    return piece;
}
