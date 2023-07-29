#pragma once
#include "SPM_Structures.h"
#include "MinoCanonicalID.h"
#include <cstdint>

typedef uint64_t SPM_randomSeed;
SPM_minoID arsIDConversion(SPM_minoID t);

void history_push(SPM_minoID *history, std::size_t histLen, SPM_minoID t);
SPM_minoID history_pop(SPM_minoID *history);
bool in_history(SPM_minoID *history, std::size_t histLen, SPM_minoID t);

SPM_randomSeed g123_readRand(SPM_randomSeed& seed);
SPM_minoID g123_getInitPiece(SPM_randomSeed& seed);

class SPM_Randomizer
{
public:
    SPM_Randomizer(std::size_t numMinoes) : numMinoes(numMinoes) {}
    virtual ~SPM_Randomizer() {}

    virtual void init(SPM_randomSeed) {}

    virtual SPM_minoID pull() = 0;
    virtual SPM_minoID lookahead(std::size_t) = 0;

protected:
    virtual SPM_minoID toCanonicalID(SPM_minoID val) { return val; }

    std::size_t numMinoes;
};

class SPM_NonRandomizer : public SPM_Randomizer
{
public:
    SPM_NonRandomizer(std::size_t num) : SPM_Randomizer(num) { index = 0; }

    SPM_minoID pull() override
    {
        SPM_minoID val = index;
        index = (index + 1) % numMinoes;
        return toCanonicalID(val);
    }

    SPM_minoID lookahead(std::size_t distance) override
    {
        return (index + distance) % numMinoes;
    }

protected:
    SPM_minoID toCanonicalID(SPM_minoID val) override
    {
        return val + Shiro::Mino::I4;
    }

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
            history[i] = Shiro::Mino::Err;
        }

        rolls = 4;
    }

    void init(SPM_randomSeed seed) override
    {
        this->seed = seed & 0xFFFFFFFF;

        history[0] = Shiro::Mino::Z4;
        history[1] = Shiro::Mino::Z4;
        history[2] = Shiro::Mino::Z4;
        history[3] = toCanonicalID(g123_getInitPiece(seed));

        pull(); pull(); pull();
    }

    SPM_minoID pull() override
    {
        SPM_minoID t = getNext();
        SPM_minoID result = history_pop(history);

        history_push(history, 4, t);

        return arsIDConversion(result);
    }

    SPM_minoID lookahead(std::size_t distance) override
    {
        if(distance > 4 || distance < 1)
        {
            return Shiro::Mino::Err;
        }

        return arsIDConversion(history[distance - 1]);
    }

    SPM_minoID getNext()
    {
        SPM_minoID t = Shiro::Mino::Err;

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
    SPM_minoID toCanonicalID(SPM_minoID val) override
    {
        return arsIDConversion(val);
    }

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

    void init(SPM_randomSeed seed) override
    {
        this->seed = seed & 0xFFFFFFFF;

        history[0] = Shiro::Mino::S4;
        history[1] = Shiro::Mino::S4;
        history[2] = Shiro::Mino::Z4;
        history[3] = toCanonicalID(g123_getInitPiece(seed));

        pull(); pull(); pull();
    }
};

class G3_Randomizer : public SPM_Randomizer
{
public:
    G3_Randomizer();

    void init(SPM_randomSeed seed) override;

    SPM_minoID pull() override;

    SPM_minoID getNext();

    SPM_minoID lookahead(std::size_t distance) override
    {
        if(distance > 4 || distance < 1)
        {
            return Shiro::Mino::Err;
        }

        return arsIDConversion(history[distance - 1]);
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
    SPM_minoID toCanonicalID(SPM_minoID val) override
    {
        return arsIDConversion(val);
    }

    SPM_randomSeed seed;

    SPM_minoID history[4];
    SPM_minoID bag[35];
    int histogram[7];
};