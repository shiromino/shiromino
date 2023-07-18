#pragma once
#include "SPM_Structures.h"
#include <cstdint>

// IJLXSZNGUTFaFbPQWYaYbV I4T4J4L4OS4Z4 - felicity's arbitrary pentomino+tetromino ordering
#define QRS_I 0
#define QRS_J 1
#define QRS_L 2
#define QRS_X 3
#define QRS_S 4
#define QRS_Z 5
#define QRS_N 6
#define QRS_G 7
#define QRS_U 8
#define QRS_T 9
#define QRS_Fa 10
#define QRS_Fb 11
#define QRS_P 12
#define QRS_Q 13
#define QRS_W 14
#define QRS_Ya 15
#define QRS_Yb 16
#define QRS_V 17
#define QRS_I4 18
#define QRS_T4 19
#define QRS_J4 20
#define QRS_L4 21
#define QRS_O 22
#define QRS_S4 23
#define QRS_Z4 24

// ITJLOSZ - felicity's arbitrary tetromino ordering
#define QRS_ARS_I 0
#define QRS_ARS_T 1
#define QRS_ARS_J 2
#define QRS_ARS_L 3
#define QRS_ARS_O 4
#define QRS_ARS_S 5
#define QRS_ARS_Z 6

// IZSJLOT - Arika's arbitrary tetromino ordering
#define ARS_I 0
#define ARS_Z 1
#define ARS_S 2
#define ARS_J 3
#define ARS_L 4
#define ARS_O 5
#define ARS_T 6

typedef uint64_t SPM_randomSeed;
SPM_minoID arsToQrsID(SPM_minoID t);
void history_push(SPM_minoID *history, unsigned int histLen, SPM_minoID t);
SPM_minoID history_pop(SPM_minoID *history);
bool in_history(SPM_minoID *history, unsigned int histLen, SPM_minoID t);

SPM_randomSeed g123_readRand(SPM_randomSeed& seed);
SPM_minoID g123_getInitPiece(SPM_randomSeed& seed);

class SPM_Randomizer
{
public:
    SPM_Randomizer(unsigned int numMinoes) : numMinoes(numMinoes) {}
    virtual ~SPM_Randomizer() {}

    virtual void init(SPM_randomSeed) {}

    virtual SPM_minoID pull() = 0;
    virtual SPM_minoID lookahead(unsigned int) = 0;

protected:
    unsigned int numMinoes;
};

class SPM_NonRandomizer : public SPM_Randomizer
{
public:
    SPM_NonRandomizer(unsigned int num) : SPM_Randomizer(num) { index = 0; }

    SPM_minoID pull()
    {
        SPM_minoID val = index;
        index = (index + 1) % numMinoes;
        return val;
    }

    SPM_minoID lookahead(unsigned int distance) { return (index + distance) % numMinoes; }

protected:
    SPM_minoID index;
};

class G1_Randomizer : public SPM_Randomizer
{
public:
    G1_Randomizer() : SPM_Randomizer(7)
    {
        seed = 0;
        for(int i = 0; i < 4; i++)
        {
            history[i] = MINO_ID_INVALID;
        }

        rolls = 4;
    }

    virtual void init(SPM_randomSeed seed) override
    {
        this->seed = seed & 0xFFFFFFFF;

        history[0] = ARS_Z;
        history[1] = ARS_Z;
        history[2] = ARS_Z;
        history[3] = g123_getInitPiece(seed);

        for(int i = 0; i < 3; i++)
        {
            pull();
        }
    }

    virtual SPM_minoID pull() override
    {
        SPM_minoID t = getNext();
        SPM_minoID result = history_pop(history);

        history_push(history, 4, t);

        return arsToQrsID(result);
    }

    virtual SPM_minoID lookahead(unsigned int distance) override
    {
        if(distance > 4 || distance < 1)
        {
            return MINO_ID_INVALID;
        }

        return arsToQrsID(history[distance - 1]);
    }

    SPM_minoID getNext()
    {
        SPM_minoID t = MINO_ID_INVALID;

        for(int i = 0; i < rolls; i++)
        {
            t = g123_readRand(seed) % 7;
            bool inHistory = false;

            for(int j = 0; j < 4; j++)
            {
                if(history[j] == t)
                {
                    inHistory = true;
                }
            }

            if(!inHistory)
            {
                break;
            }

            t = g123_readRand(seed) % 7;
        }

        return t;
    }

protected:
    SPM_randomSeed seed;
    SPM_minoID history[4];
    int rolls;
};

class G2_Randomizer : public G1_Randomizer
{
public:
    G2_Randomizer() : G1_Randomizer()
    {
        rolls = 6;
    }

    virtual void init(SPM_randomSeed seed) override
    {
        this->seed = seed & 0xFFFFFFFF;

        history[0] = ARS_S;
        history[1] = ARS_S;
        history[2] = ARS_Z;
        history[3] = g123_getInitPiece(seed);

        for(int i = 0; i < 3; i++)
        {
            pull();
        }
    }
};

class G3_Randomizer : public SPM_Randomizer
{
public:
    G3_Randomizer();

    void init(SPM_randomSeed seed) override;

    SPM_minoID pull() override;

    SPM_minoID getNext();

    virtual SPM_minoID lookahead(unsigned int distance) override
    {
        if(distance > 4 || distance < 1)
        {
            return MINO_ID_INVALID;
        }

        return arsToQrsID(history[distance - 1]);
    }

    SPM_minoID mostDroughtedPiece()
    {
        size_t i = 0;
        int n = 0;
        size_t maxi = 0;

        for(i = 0; i < 7; i++)
        {
            if(n < histogram[i])
            {
                n = histogram[i];
                maxi = i;
            }
        }

        return (SPM_minoID)(maxi);
    }

protected:
    SPM_randomSeed seed;

    SPM_minoID history[4];
    SPM_minoID bag[35];
    int histogram[7];
};