//----------------------------------------------------------------------------
/** @file RlAgentLog.h
    Logging functionality associated with agents
*/
//----------------------------------------------------------------------------

#ifndef RLAGENTLOG_H
#define RLAGENTLOG_H

#include "RlAgent.h"

//----------------------------------------------------------------------------
/** Class for logging agent statistics */
class RlAgentLog : public RlAutoObject
{
public:

    DECLARE_OBJECT(RlAgentLog);

    RlAgentLog(GoBoard& board);

    virtual void Initialise();
    virtual void LoadSettings(std::istream& settings);

    bool LogIsActive() const;
    bool StepLogIsActive() const; // once per step data
    int GameIndex() const;

    virtual void StartGame();
    virtual void EndGame();

    std::ostream& Debug(int debuglevel = RlSetup::VERBOSE);
    int DebugLevel() const { return m_debugLevel; }

    void SaveRecord();
    void SaveWeights();
    void TopTex();

    void LogWeights();
    void LogGame();
    void LogStep();
    void LogEval();
    void PrintValue();
    void PrintBoard();
    void PrintPV();
    void PrintBest();
    
    int GetNumTraceFeatures() const; 
    std::string GetTraceFeatureName(int i) const;
    int GetTraceFeatureIndex(int i) const;
    int GetTraceFeatureNum(int featureindex) const;
    
protected:

    void LogEvalMove(SgMove move);
    virtual void InitLogs();
    void TraceFeatures();
    void AddItems();

    /** Whether log is currently active */
    bool m_active;

    /** Agent that is being tracked */
    RlAgent* m_agent;

private:

    enum
    {
        LOG_NEVER,
        LOG_LINEAR,
        LOG_EXP,
        LOG_EXPINTERVAL,
        LOG_NICE,
        NUM_LOG_MODES
    };

    /** How verbose this log should be */
    int m_debugLevel;

    /** Mode with which to log statistics
        (Never, at linear intervals, at exponential intervals) */
    int m_logMode;

    /** Game number at which log will next be active */
    int m_logNext;

    /** How frequently to log statistics */
    RlFloat m_interval;

    /** Multiplier for interval */
    RlFloat m_intervalMul;
        
    /** Current game number */
    int m_numGames;
    
    /** Only log the first position or move, not the rest of the game */
    bool m_logStartOnly;
    
    /** Whether to save out the game record of each active game */
    bool m_saveRecord;

    /** Whether to save out the full weights file after each active game */
    bool m_saveWeights;

    /** Whether to save out the top weights after each active game */
    int m_topTex;

    /** Which evalindex to use live graphics */
    bool m_liveGraphics;

    /** Which features to trace statistics */
    std::string m_traceFile;

    /** How much to pause on each execute */
    RlFloat m_pause;
    
    /** Policy to use for evaluating best moves and PV (typically greedy) */
    RlPolicy* m_policy;
    
    /** Number of moves to display in PV */
    int m_numPV;
    
    /** Number of best moves to display */
    int m_numBest;

    /** Feature indices to be tracked */
    std::vector<std::pair<std::string, int> > m_traceFeatures;
    std::map<int, int> m_traceIndices;

    // Debugging data
    std::auto_ptr<RlTrace> m_featureTrace;
    std::auto_ptr<RlTrace> m_timeTrace;
    std::auto_ptr<RlLog> m_stepLog;
    std::auto_ptr<RlLog> m_gameLog;
    std::auto_ptr<RlLog> m_updateLog;
    std::auto_ptr<RlLog> m_evalLog;
};

inline bool RlAgentLog::LogIsActive() const
{
    return m_active;
}

inline bool RlAgentLog::StepLogIsActive() const
{
    return LogIsActive()
        && (!m_logStartOnly || m_agent->m_timestep == 0);
}

inline std::ostream& RlAgentLog::Debug(int debuglevel)
{
    static std::ofstream s_rlNullStream;
    if (LogIsActive() && m_debugLevel >= debuglevel)
        return RlDebug(debuglevel);
    else
        return s_rlNullStream;
}

// @todo: make trace features a sub-class

inline int RlAgentLog::GetNumTraceFeatures() const 
{ 
    return m_traceFeatures.size(); 
}

inline std::string RlAgentLog::GetTraceFeatureName(int i) const
{ 
    SG_ASSERT(i < ssize(m_traceFeatures)); 
    return m_traceFeatures[i].first;
}

inline int RlAgentLog::GetTraceFeatureNum(int featureindex) const
{ 
    std::map<int, int>::const_iterator i_trace = 
        m_traceIndices.find(featureindex);
    if (i_trace == m_traceIndices.end())
        return -1;
    else
        return i_trace->second;
}

inline int RlAgentLog::GetTraceFeatureIndex(int i) const
{ 
    SG_ASSERT(i < ssize(m_traceFeatures)); 
    return m_traceFeatures[i].second;
}

inline int RlAgentLog::GameIndex() const
{
    return m_numGames;
}

//----------------------------------------------------------------------------

#endif // RLAGENTLOG_H
