//----------------------------------------------------------------------------
/** @file RlHashUtil.h
    Simple hash wrapper
*/
//----------------------------------------------------------------------------

#ifndef RLHASHUTIL_H
#define RLHASHUTIL_H

//----------------------------------------------------------------------------

#include "SgHash.h"

#define RL_HASHSIZE 64

typedef SgHash<RL_HASHSIZE> RlHash;

class RlIntHash
{
public:
    unsigned int operator()(int value) const 
    { 
        return SgHash<RL_HASHSIZE>(value).Code1(); 
    }
};

//----------------------------------------------------------------------------

#endif // RLHASHUTIL_H

