//----------------------------------------------------------------------------
/** @file RlMCPolicy.h
    Select moves by Monte-Carlo simulation
*/
//----------------------------------------------------------------------------

#ifndef RLMCPOLICY_H
#define RLMCPOLICY_H

#include "RlPolicy.h"

class RlSimulator;

//----------------------------------------------------------------------------
/** Select moves by Monte-Carlo simulation */
class RlMCPolicy : public RlPolicy
{
public:

    DECLARE_OBJECT(RlMCPolicy);
    
    RlMCPolicy(GoBoard& board, RlEvaluator* evaluator = 0,
        RlSimulator* simulator = 0);

    virtual void LoadSettings(std::istream& settings);
    
    /** Select a move in the current position for the specified colour */
    virtual SgMove SelectMove(RlState& state);

private:

    RlSimulator* m_simulator;
};

//----------------------------------------------------------------------------

#endif // RLMCPOLICY_H
