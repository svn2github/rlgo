//----------------------------------------------------------------------------
/** @file RlLogger.h
    Logging functionality
*/
//----------------------------------------------------------------------------

#ifndef RLLOGGER_H
#define RLLOGGER_H

#include "RlSetup.h"
#include "RlUtils.h"

class RlBinaryFeatures;

//----------------------------------------------------------------------------
/** Class for logging game by game or move by move statistics */
class RlLogger : public RlAutoObject
{
public:

    DECLARE_OBJECT(RlLogger);

    RlLogger(GoBoard& board);

    virtual void Initialise();
    virtual void LoadSettings(std::istream& settings);

    bool GameLogIsActive() const;
    bool MoveLogIsActive() const; // once per step data
    int GameIndex() const { return m_numGames; }

    virtual void StartGame();
    virtual void EndGame();
    virtual void StepMove();

    std::ostream& Debug(int debuglevel = RlSetup::VERBOSE);
    int DebugLevel() const { return m_debugLevel; }

    int GetNumTraceFeatures() const; 
    std::string GetTraceFeatureName(int i) const;
    int GetTraceFeatureIndex(int i) const;
    int GetTraceFeatureNum(int featureindex) const;
        
protected:

    void TraceFeatures(RlBinaryFeatures* featureset);

    /** Whether log is currently active */
    bool m_active;

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
        
    /** Only log the first position or move, not the rest of the game */
    bool m_logStartOnly;
    
    /** Which features to trace statistics */
    std::string m_traceFile;

    /** Feature indices to be tracked */
    std::vector<std::pair<std::string, int> > m_traceFeatures;
    std::map<int, int> m_traceIndices;

    int m_numGames;
    int m_numMoves;
};

inline bool RlLogger::GameLogIsActive() const
{
    return m_active;
}

inline bool RlLogger::MoveLogIsActive() const
{
    return m_active && !m_logStartOnly || m_numMoves == 0;
}

inline std::ostream& RlLogger::Debug(int debuglevel)
{
    static std::ofstream s_rlNullStream;
    if (GameLogIsActive() && m_debugLevel >= debuglevel)
        return RlDebug(debuglevel);
    else
        return s_rlNullStream;
}

inline int RlLogger::GetNumTraceFeatures() const 
{ 
    return m_traceFeatures.size(); 
}

inline std::string RlLogger::GetTraceFeatureName(int i) const
{ 
    SG_ASSERT(i < ssize(m_traceFeatures)); 
    return m_traceFeatures[i].first;
}

inline int RlLogger::GetTraceFeatureNum(int featureindex) const
{ 
    std::map<int, int>::const_iterator i_trace = 
        m_traceIndices.find(featureindex);
    if (i_trace == m_traceIndices.end())
        return -1;
    else
        return i_trace->second;
}

inline int RlLogger::GetTraceFeatureIndex(int i) const
{ 
    SG_ASSERT(i < ssize(m_traceFeatures)); 
    return m_traceFeatures[i].second;
}

//----------------------------------------------------------------------------

#endif // RLLOGGER_H
