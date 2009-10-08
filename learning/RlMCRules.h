//----------------------------------------------------------------------------
/** @file RlMCRules.h
    Monte-Carlo learning rules
*/
//----------------------------------------------------------------------------

#ifndef RLMCRULES_H
#define RLMCRULES_H

#include "RlLearningRule.h"

//----------------------------------------------------------------------------
/** Update rule based on Monte-Carlo */
class RlMonteCarlo : public RlLearningRule
{
public:

    DECLARE_OBJECT(RlMonteCarlo);

    RlMonteCarlo(GoBoard& board, RlWeightSet* wset = 0, RlAgentLog* log = 0);

protected:

    /** Error between actual return and estimated value */
    virtual void CalcDelta();
};


//----------------------------------------------------------------------------

#endif // RLMCRULES_H
