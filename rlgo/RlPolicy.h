//----------------------------------------------------------------------------
/** @file RlPolicy.h
    Base class for Go playing policy

    $Id: RlPolicy.h,v 1.49 2008/05/29 23:20:58 silver Exp $
    $Source: /usr/cvsroot/project_explorer/rlgo/RlPolicy.h,v $
*/
//----------------------------------------------------------------------------

#ifndef RLPOLICY_H
#define RLPOLICY_H

#include "RlFactory.h"
#include "RlState.h"
#include "RlTrace.h"

class RlAgentLog;
class RlEvaluator;
class RlHistory;
class RlSimulator;
class RlState;
class RlWeightSet;

//----------------------------------------------------------------------------
/** A policy selects a move given a value function */
class RlPolicy : public RlAutoObject
{
public:
    
    RlPolicy(GoBoard& board, RlEvaluator* evaluator = 0,
        RlAgentLog* log = 0);
    
    /** Load settings from specified file */
    virtual void LoadSettings(std::istream& settings);
    
    /** Initialise policy */
    virtual void Initialise();
    
    /** Select a move in the current position for the specified colour */
    virtual SgMove SelectMove(RlState& state) = 0;

    /** Value of the last selected move */
    virtual bool SearchValue(RlFloat& value) const;

    /** Sample probability distribution over all moves */
    virtual void SampleProbabilities(int timestep, 
        int numsamples, std::vector<RlFloat>& probs);

    /** Get actual probability for specified move.
        Throws an exception by default (not supported by most policies). */
    virtual RlFloat GetProbability(SgMove move) const;

    /** Evaluator accessor */
    RlEvaluator* Evaluator() { return m_evaluator; }

    /** Specify the current move, to add into normalised features
        NB. This is the move played in the state, 
            not the move selected by the policy. */
    virtual void SetCurrentMove(RlState& state) { SG_UNUSED(state); }

protected:

    /** Initialise output log for this policy */
    virtual void InitLogs();

    /** Log any info for this policy */
    virtual void LogPolicy(const RlState& state, SgBlackWhite move);
    
    RlEvaluator* m_evaluator;
    RlAgentLog* m_log;
    
    /** Whether the moves selected by this policy should considered
        on-policy or off-policy during learning */
    bool m_onPolicy;
    
    std::auto_ptr<RlLog> m_policyLog;
};

//----------------------------------------------------------------------------
/** Select the best move in the current position */
class RlGreedy : public RlPolicy
{
public:

    DECLARE_OBJECT(RlGreedy);

    RlGreedy(GoBoard& board, RlEvaluator* evaluator = 0, 
        RlHistory* history = 0);

    virtual void LoadSettings(std::istream& settings);

    /** Select a move in the current position for the specified colour */
    virtual SgMove SelectMove(RlState& state);

protected:

    /** Initialise output log for this policy */
    virtual void InitLogs();

    /** Log any info for this policy */
    virtual void LogPolicy(const RlState& state, SgBlackWhite move);

private:

    /** History from which to retrieve state (best moves) */
    RlHistory* m_history;
};

//----------------------------------------------------------------------------
/** Select a random move */
class RlRandomPolicy : public RlPolicy
{
public:

    DECLARE_OBJECT(RlRandomPolicy);

    RlRandomPolicy(GoBoard& board, RlEvaluator* evaluator = 0);

    /** Select a move in the current position for the specified colour */
    virtual SgMove SelectMove(RlState& state);
};

//----------------------------------------------------------------------------
/** Select most frequently chosen move in simulation */
class RlSimMaxPolicy : public RlPolicy
{
public:

    DECLARE_OBJECT(RlSimMaxPolicy);
    
    RlSimMaxPolicy(GoBoard& board, RlSimulator* simulator = 0);

    /** Load settings from specified file */
    virtual void LoadSettings(std::istream& settings);

    /** Select a move in the current position for the specified colour */
    virtual SgMove SelectMove(RlState& state);
        
private:

    RlSimulator* m_simulator;
};

//----------------------------------------------------------------------------

#endif // RLPOLICY_H
