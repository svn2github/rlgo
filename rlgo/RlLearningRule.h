//----------------------------------------------------------------------------
/** @file RlLearningRule.h
    Logistic Linear Learning Rules for updating Weights.
*/
//----------------------------------------------------------------------------

#ifndef RLLEARNINGRULE_H
#define RLLEARNINGRULE_H

#include "RlActiveSet.h"
#include "RlAgentLog.h"
#include "RlWeight.h"
#include "RlWeightSet.h"
#include "RlFactory.h"
#include "RlTrace.h"
#include "RlWeight.h"

#include <list>

class RlAgentLog;
class RlWeightSet;
class RlHistory;
class RlState;

//----------------------------------------------------------------------------
/** Abstract class representing a single rule for updating weights */
class RlLearningRule : public RlAutoObject
{
public:

    RlLearningRule(GoBoard& board, RlWeightSet* wset = 0, RlAgentLog* log = 0);

    //@todo: The ability to AddSettings without using a settings file

    /** Load settings from specified file */
    virtual void LoadSettings(std::istream& settings);

    /** Initialise learning rule */
    virtual void Initialise();

    /** Start a new episode */
    virtual void Start(RlHistory* history, int episode);

    /** End an episode */
    virtual void End();

    /** Do a step of learning (SetData, Learn) */
    void DoLearn(RlHistory* history, int episode, int t1, int t2);

    /** Whether forwards and backwards execution can be used with this rule */
    virtual bool IsForwards() const { return true; }
    virtual bool IsBackwards() const { return true; }

    /** Set data for this timestep.
        SetData must be called before Learn for each step */
    virtual void SetData(RlHistory* history, int from, int to, int episode);
    virtual void SetData(RlState& oldstate, RlState& newstate);

    /** Update all weights. SetData must be called before Learn each step */
    virtual void Learn();

    /** Accessors */
    RlFloat GetDelta() const { return m_delta; }    
    RlFloat GetReward() const { return m_reward; }

    static const int MAX_TD = 2;

protected:

    /** Calculate learning error */
    virtual void CalcDelta() = 0;
    
    /** Calculate learning gradient */
    void CalcLogisticGradient();
    
    /** Calculate step-size */
    void CalcStepSize();
    
    void CalcValues();
    void SetOnPolicy(RlHistory* history, int from, int to, int episode);
    
    /** Whether learning on-policy */
    bool CheckOnPolicy() const;

    /** Apply logistic function if m_logistic, otherwise use linear value */
    RlFloat ApplyLogistic(RlFloat) const;
    
    /** Debug logs */
    virtual void InitLogs();
    virtual void LogLearn();
    virtual void LogGame();
    void LogUpdate(int id, RlFloat step, RlFloat delta, RlOccur occur, 
        RlFloat update, RlFloat w);
    int TraceID(RlWeight& weight) const;
    bool DoLog(RlWeight& weight) const;

    /** Basic weight update */
    void UpdateWeight(RlWeight& weight, RlOccur occurrences);

    RlFloat GetStep(RlWeight& weight, int index);
    void IncCount(RlWeight& weight, int index);
    void CountFeatures();

protected:

    RlWeightSet* m_weightSet;
    RlAgentLog* m_log;

    /** Step size modes */
    enum
    {
        RL_CONSTANT,   // Step-size is set to constant value of alpha
        RL_NORM,       // Step-size is normalised to give total update of alpha
        RL_BINARY,     // As above, but assumes binary features for efficiency
        RL_RECIPROCAL  // Step-size set to alpha/N where N is number of games
    };

    /** Basic step-size */
    RlFloat m_alpha;

    /** Step-size mode */
    int m_stepSizeMode;
    
    /** Whether to learn from off-policy data */
    bool m_useOffPolicy;

    /** Whether to use logistic in value function approximation */
    bool m_logistic;

    /** Whether to minimise MSE or cross-entropy */
    //@Comment This could be generalized to other loss functions
    bool m_mse;

    /** Cap the value to avoid region with flat gradient */
    RlFloat m_minGrad;
    
    /** Current learning data */
    RlState* m_oldState; //@todo: should this be const?
    RlState* m_newState; //@todo: should this be const?
    RlFloat m_oldValue;
    RlFloat m_newValue;
    RlFloat m_reward;
    RlFloat m_return;
    bool m_terminal;
    RlFloat m_stepSize;
    RlFloat m_delta;
    RlFloat m_logisticGradient;
    RlFloat m_fraction;
    RlFloat m_target;
    
    /** Colour to play in target state (i.e. old time-step) */
    SgBlackWhite m_colour; 
    
    /** Timestep of target state (i.e. old time-step) */
    int m_timeStep;

    // @todo: centralize onPolicy control/calculation outside of this class
    bool m_onPolicy;
    
    //SetData(...) must be called before Learn(). This variable
    //is used by a flag in Debug mode to ensure SetData is called
    //before Learn.
    bool m_isDataSet;
    
    /** Debugging statistics */
    int m_learnCount;
    int m_numSteps;
    int m_numGames;
    
    RlStat m_statDelta;
    RlStat m_statDelta2;
    RlStat m_statDeltaT[RL_MAX_TIME];
    RlStat m_statDelta2T[RL_MAX_TIME];
    RlStat m_statMCError;
    RlStat m_statMCError2;
    RlStat m_statCrossEntropy;
    
    std::auto_ptr<RlLog> m_learnLog; // Learning data each timestep
    std::auto_ptr<RlLog> m_gameLog; // Learning data each game
    std::auto_ptr<RlTrace> m_updateTrace; // Update log for traced features
};

inline bool RlLearningRule::CheckOnPolicy() const
{
    return (m_useOffPolicy || m_onPolicy);
}

inline int RlLearningRule::TraceID(RlWeight& weight) const
{
    return m_weightSet->GetFeatureIndex(&weight);
}

inline bool RlLearningRule::DoLog(RlWeight& weight) const
{
    return m_log 
        && m_log->LogIsActive()
        && m_updateTrace->ExistsLog(m_weightSet->GetFeatureIndex(&weight));
}

//----------------------------------------------------------------------------

#endif // RLLEARNINGRULE_H
