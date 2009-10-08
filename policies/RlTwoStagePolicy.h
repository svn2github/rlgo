//----------------------------------------------------------------------------
/** @file RlTwoStagePolicy.h
    Policy that switches between first and second stage
*/
//----------------------------------------------------------------------------

#ifndef RLTWOSTAGEPOLICY_H
#define RLTWOSTAGEPOLICY_H

#include "RlPolicy.h"

class RlSimulator;

//----------------------------------------------------------------------------
/** Two stage policy */
class RlTwoStagePolicy : public RlPolicy
{
public:

    DECLARE_OBJECT(RlTwoStagePolicy);
    
    RlTwoStagePolicy(GoBoard& board, 
        RlPolicy* policy1 = 0, RlPolicy* policy2 = 0);

    /** Load settings from specified file */
    virtual void LoadSettings(std::istream& settings);

    /** Select a move in the current position for the specified colour */
    virtual SgMove SelectMove(RlState& state);
        
    /** When to switch policies. 
        Fractional values are dealt with probabilistically for last step */
    virtual RlFloat SwitchTime(int episodes) const;

    virtual void Reset();
    virtual void NewGame();
    virtual void SetCurrentMove(RlState& state);

protected:

    RlPolicy* m_policy1;
    RlPolicy* m_policy2;
    RlFloat m_switchTime;
    int m_episodes;
};

//----------------------------------------------------------------------------
/** Two stage policy with logarithmically increasing switch time */
class RlLogTwoStagePolicy : public RlTwoStagePolicy
{
public:

    DECLARE_OBJECT(RlLogTwoStagePolicy);
    
    RlLogTwoStagePolicy(GoBoard& board, 
        RlPolicy* policy1 = 0, RlPolicy* policy2 = 0);

    /** Load settings from specified file */
    virtual void LoadSettings(std::istream& settings);

    /** When to switch policies. 
        Fractional values are dealt with probabilistically for last step */
    virtual RlFloat SwitchTime(int episodes) const;
    
private:

    RlFloat m_base;
};


//----------------------------------------------------------------------------
/** Two stage policy with linearly increasing switch time */
class RlLinearTwoStagePolicy : public RlTwoStagePolicy
{
public:

    DECLARE_OBJECT(RlLinearTwoStagePolicy);
    
    RlLinearTwoStagePolicy(GoBoard& board, 
        RlPolicy* policy1 = 0, RlPolicy* policy2 = 0);

    /** Load settings from specified file */
    virtual void LoadSettings(std::istream& settings);

    /** When to switch policies. 
        Fractional values are dealt with probabilistically for last step */
    virtual RlFloat SwitchTime(int episodes) const;
    
private:

    RlSimulator* m_simulator;
};

//----------------------------------------------------------------------------

#endif // RLTWOSTAGEPOLICY_H
