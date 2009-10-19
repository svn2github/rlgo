#ifndef RL_CONVERT
#define RL_CONVERT

#include "RlUtils.h"

class RlWeightSet;

class RlConvert : public RlAutoObject
{
public:

    RlConvert(GoBoard& board)
    :   RlAutoObject(board) { }
    
    virtual void Convert(const RlWeightSet* sourceweights,
        RlWeightSet* targetweights) const = 0;
};

#endif // RL_CONVERT