//----------------------------------------------------------------------------
/** @file RlPriorityPolicy.h
    Use priority policy whenever possible, otherwise use normal policy
*/
//----------------------------------------------------------------------------

#ifndef RLPRIORITYPOLICY_H
#define RLPRIORITYPOLICY_H

#include "RlPolicy.h"

//----------------------------------------------------------------------------
/** Use priority policy if available, otherwise use normal policy */
class RlPriorityPolicy : public RlPolicy
{
public:

    DECLARE_OBJECT(RlPriorityPolicy);
    
    RlPriorityPolicy(GoBoard& board, RlPolicy* priority = 0,
        RlPolicy* normal = 0);

    virtual void LoadSettings(std::istream& settings);
    
    /** Select a move in the current position for the specified colour */
    virtual SgMove SelectMove(RlState& state);

private:

    RlPolicy* m_priorityPolicy;
    RlPolicy* m_normalPolicy;
};

//----------------------------------------------------------------------------
/** Select a move that responds to the biggest atari, or return null move */
class RlAtariPolicy : public RlPolicy
{
public:

    DECLARE_OBJECT(RlAtariPolicy);

    RlAtariPolicy(GoBoard& board);
    
    /** Load settings from specified file */
    virtual void LoadSettings(std::istream& settings);

    /** Select a move in the current position for the specified colour */
    virtual SgMove SelectMove(RlState& state);
};

//----------------------------------------------------------------------------

#endif // RLPRIORITYPOLICY_H
