#ifndef _spm_randomizer_hpp
#define _spm_randomizer_hpp

#include <cstdint>

#include "SPM_Spec.hpp"

typedef uint64_t SPM_randomSeed;

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
        index = (index + 1) % 7;
        return val;
    }

    SPM_minoID lookahead(unsigned int distance) { return (index + distance) % 7; }

protected:
    SPM_minoID index;
};

#endif
