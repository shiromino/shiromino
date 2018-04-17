#ifndef _bstr_to_std_hpp
#define _bstr_to_std_hpp

#include <string>
#include "bstrlib.h"

inline std::string bstr_to_std(bstring b)
{
    if(!b || !b->data)
    {
        return {};
    }

    std::string s {(char *)b->data};
    bdestroy(b);
    return s;
}

#endif
