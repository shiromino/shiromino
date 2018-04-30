#include "SPM_Randomizer.hpp"

#include "SPM_Spec.hpp"

SPM_minoID arsToQrsID(SPM_minoID t)
{
    switch(t)
    {
        case 0:
            break;
        case 1:
            t = 6;
            break;
        case 2:
            t = 5;
            break;
        case 3:
            t = 2;
            break;
        case 4:
            t = 3;
            break;
        case 5:
            t = 4;
            break;
        case 6:
            t = 1;
            break;
        default:
            break;
    }

    return t;
}

void history_push(SPM_minoID *history, unsigned int histLen, SPM_minoID t)
{
    unsigned int i = 0;

    for(unsigned int i = 0; i < histLen - 1; i++)
    {
        history[i] = history[i + 1];
    }

    history[histLen - 1] = t;
}

SPM_minoID history_pop(SPM_minoID *history)
{
    SPM_minoID t = history[0];
    history[0] = MINO_ID_INVALID;

    return t;
}

bool in_history(SPM_minoID *history, unsigned int histLen, SPM_minoID t)
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
        history[i] = MINO_ID_INVALID;
    }

    for(int i = 0; i < 35; i++)
    {
        bag[i] = PIECE_ID_INVALID;
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

    history[0] = ARS_S;
    history[1] = ARS_S;
    history[2] = ARS_Z;
    history[3] = g123_getInitPiece(seed);

    for(int i = 0; i < 3; i++)
    {
        // move init piece to history[0] (first preview/next piece) and fill in 3 pieces ahead
        // the S,S,Z get pulled into the void
        pull();
    }
}

SPM_minoID G3_Randomizer::pull()
{
    SPM_minoID t = getNext();
    SPM_minoID result = history_pop(history);

    history_push(history, 4, t);

    return arsToQrsID(result);
}

SPM_minoID G3_Randomizer::getNext()
{
    SPM_minoID piece = MINO_ID_INVALID;
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
