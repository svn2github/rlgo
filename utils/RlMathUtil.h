//----------------------------------------------------------------------------
/** @file RlMathUtil.h
    Simple math functions
*/
//----------------------------------------------------------------------------

#ifndef RLMATHUTIL_H
#define RLMATHUTIL_H

//----------------------------------------------------------------------------

#include "RlMiscUtil.h"
#include <math.h>

namespace RlMathUtil
{

const RlFloat RL_MAX_EVAL = 16.0;

inline RlFloat Bound(RlFloat value, RlFloat min, RlFloat max)
{
    if (value <= min) 
        return min;
    if (value >= max) 
        return max;
    return value;
}

inline RlFloat Truncate(RlFloat value)
{
    static const RlFloat toleval = RL_MAX_EVAL - 0.00001;
    return Bound(value, -toleval, +toleval);
}

inline RlFloat Logistic(RlFloat value)
{
    return 1.0 / (1.0 + exp(-value));
}

inline RlFloat TanH(RlFloat value)
{
    return tanh(value);
}

}

//----------------------------------------------------------------------------

#endif // RLMATHUTIL_H

