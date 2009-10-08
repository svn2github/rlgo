//----------------------------------------------------------------------------
/** @file RlEpsilonPolicy.h
    Select one policy with probability epsilon, and a second with probability 1 - epsilon
*/
//----------------------------------------------------------------------------

#ifndef RLEPSILONPOLICY_H
#define RLEPSILONPOLICY_H

#include "RlPolicy.h"

//----------------------------------------------------------------------------
/** Select a random move with probability epsilon,
    otherwise follow the specified policy */
class RlEpsilonPolicy : public RlPolicy
{
public:

    DECLARE_OBJECT(RlEpsilonPolicy);

    RlEpsilonPolicy(GoBoard& board, 
        RlPolicy* ppolicy = 0, RlPolicy* npolicy = 0);

    /** Load settings from specified file */
    virtual void LoadSettings(std::istream& settings);

    /** Select a move in the current position for the specified colour */
    virtual SgMove SelectMove(RlState& state);
    
    void SetCurrentMove(RlState& state);
    
protected:

    /** Policy to use when exploring with probability epsilon */
    RlPolicy* m_pPolicy;
    
    /** Policy to use when not exploring with probability 1-epsilon */
    RlPolicy* m_nPolicy;
    
    /** Probability of exploration */
    RlFloat m_epsilon;
};

//----------------------------------------------------------------------------
/** Epsilon policy with decaying parameters */
class RlEpsilonDecayPolicy : public RlEpsilonPolicy
{
public:

    DECLARE_OBJECT(RlEpsilonDecayPolicy);

    RlEpsilonDecayPolicy(GoBoard& board, 
        RlPolicy* ppolicy = 0, RlPolicy* npolicy = 0);

    /** Load settings from specified file */
    virtual void LoadSettings(std::istream& settings);

    /** Initialise epsilon */
    virtual void Initialise();

    /** Select a move in the current position for the specified colour */
    virtual SgMove SelectMove(RlState& state);
        
    /** Reset epsilon at start of new overall game */
    virtual void Reset();
    
    /** Decay start epsilon each simulation */
    virtual void NewGame();

private:

    /** Initial epsilon */
    RlFloat m_initEpsilon;
    
    /** Decay epsilon this amount each episode */
    RlFloat m_episodeDecay;
};

//----------------------------------------------------------------------------

#endif // RLEPSILONPOLICY_H
