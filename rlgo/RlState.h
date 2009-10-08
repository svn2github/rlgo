//----------------------------------------------------------------------------
/** @file RlState.h
    Learning state for a single timestep
*/
//----------------------------------------------------------------------------

#ifndef RLSTATE_H
#define RLSTATE_H

#include "RlActiveSet.h"
#include "RlUtils.h"
#include "SgBlackWhite.h"
#include "SgMove.h"

//----------------------------------------------------------------------------
/** Simple class containing state information for an individual time-step */
class RlState
{
public:

    /** Types of policy, for determining on/off-policy status */
    enum
    {
        POL_NONE, // No move selected
        POL_BEST, // Best move selected (sets bestMove and bestEval)
        POL_ON,   // Always considered on-policy
        POL_OFF,  // Always considered off-policy
        POL_TERMINAL // Special value for terminal states
    };

    RlState();
    RlState(int timestep, SgBlackWhite colour);

    /** Initialise this state */
    void Initialise(int timestep, SgBlackWhite colour);

    /** Uninitialise this state */
    void Uninitialise();

    /** Re-initialise this state */
    void Reinitialise();

    /** Resize active sets */
    void Resize(int activesize);

    /** Set this state to be a terminal state with specified reward */
    void SetTerminal(RlFloat score);

    /** Set the evaluation of this state */
    void SetEval(RlFloat value);

    /** Set the policy type */
    void SetPolicyType(int policytype);

    /** Set the move */
    void SetMove(SgMove move);

    /** Set the active features */
    void SetActive(const RlActiveSet& active);

    /** Check whether this state is on-policy */
    bool OnPolicy() const;

    /** Copy best moves and values from source state */
    void CopyBest(const RlState& sourcestate);
    
    //------------------------------------------------------------------------
    /** Accessors */

    bool Initialised() const
    {
        return m_timestep >= 0;
    }

    int TimeStep() const 
    { 
        SG_ASSERT(Initialised());
        return m_timestep; 
    }
    
    SgBlackWhite Colour() const 
    { 
        SG_ASSERT(Initialised());
        return m_colour; 
    }
    
    SgMove Move() const 
    { 
        SG_ASSERT(Initialised());
        return m_move; 
    }
    
    bool Evaluated() const 
    { 
        SG_ASSERT(Initialised());
        return m_evaluated; 
    }
    
    bool ActiveSet() const
    {
        SG_ASSERT(Initialised());
        return m_activeSet;
    }
    
    bool Terminal() const 
    { 
        SG_ASSERT(Initialised());
        return m_terminal; 
    }
    
    const RlActiveSet& Active() const
    { 
        SG_ASSERT(ActiveSet());
        return m_active; 
    }

    const RlFloat Reward() const 
    { 
        SG_ASSERT(Initialised());
        return m_reward; 
    }
    
    const RlFloat Eval() const 
    { 
        SG_ASSERT(Evaluated());
        return m_eval; 
    }
    
    SgMove BestMove() const 
    { 
        SG_ASSERT(Initialised());
        SG_ASSERT(m_policyType == POL_BEST);
        return m_bestMove; 
    }
    
    RlFloat BestValue() const 
    { 
        SG_ASSERT(Initialised());
        SG_ASSERT(m_policyType == POL_BEST);
        return m_bestEval; 
    }
    
    int PolicyType() const { return m_policyType; }
    
private:
    
    void ClearBest();
                
    /** Time in this state */
    int m_timestep;

    /** Colour to play in current state */
    SgBlackWhite m_colour;
    
    /** Selected move */
    SgMove m_move;

    /** Type of policy used to select move in this state */
    int m_policyType;
    
    /** Whether state has been evaluated yet */
    bool m_evaluated;
    
    /** Whether active features have been set yet */
    bool m_activeSet;
    
    /** Whether this is a terminal state */
    bool m_terminal;

    /** Active features in current state */
    RlActiveSet m_active;

    /** Reward received */
    RlFloat m_reward;
    
    /** The linear evaluation (unsquashed) of this state */
    RlFloat m_eval;

    /** Best move, if computed */
    SgMove m_bestMove;
    
    /** Value of best move, if computed */
    RlFloat m_bestEval;

friend class RlEvaluator; // For setting best moves and values
};

inline void RlState::Initialise(int timestep, SgBlackWhite colour)
{
    m_timestep = timestep;
    m_colour = colour;
    m_move = SG_NULLMOVE;
    m_policyType = POL_NONE;
    m_evaluated = false;
    m_activeSet = false;
    m_terminal = false;
    // Active set is not cleared, for efficiency
    m_reward = 0;
    m_eval = 0;
    // Best moves and values are not cleared, for efficiency
}

inline void RlState::Uninitialise()
{
    m_timestep = -1;
    m_colour = SG_EMPTY;
}

inline void RlState::Reinitialise()
{
    Initialise(m_timestep, m_colour);
}

inline void RlState::SetMove(SgMove move)
{
#ifndef RL_REUSE
    SG_ASSERT(m_move == SG_NULLMOVE);
#endif // RL_REUSE
    m_move = move;
}

inline void RlState::SetActive(const RlActiveSet& active)
{
    m_active = active;
    m_activeSet = true;
}

inline void RlState::SetPolicyType(int type)
{
#ifndef RL_REUSE
    SG_ASSERT(m_policyType == POL_NONE);
#endif // RL_REUSE
    m_policyType = type;
}

inline void RlState::SetTerminal(RlFloat score)
{
    m_terminal = true;
    m_reward = score;
    m_policyType = POL_TERMINAL;
}

inline void RlState::SetEval(RlFloat value)
{
    // Allow value to be refreshed even if already evaluated
    m_eval = value;
    m_evaluated = true;
}

//----------------------------------------------------------------------------

#endif // RLSTATE_H
