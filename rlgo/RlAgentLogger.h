//----------------------------------------------------------------------------
/** @file RlAgentLogger.h
    Logging functionality associated with agents
*/
//----------------------------------------------------------------------------

#ifndef RLAGENTLOGGER_H
#define RLAGENTLOGGER_H

#include "RlAgent.h"
#include "RlLogger.h"

//----------------------------------------------------------------------------
/** Class for logging agent statistics */
class RlAgentLogger : public RlLogger
{
public:

    DECLARE_OBJECT(RlAgentLogger);

    RlAgentLogger(GoBoard& board);

    virtual void Initialise();
    virtual void LoadSettings(std::istream& settings);

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
    
protected:

    void LogEvalMove(SgMove move);
    virtual void InitLogs();
    void AddItems();

private:

    /** Agent that is being tracked */
    RlAgent* m_agent;

    /** Whether to save out the game record of each active game */
    bool m_saveRecord;

    /** Whether to save out the full weights file after each active game */
    bool m_saveWeights;

    /** Whether to save out the top weights after each active game */
    int m_topTex;

    /** Which evalindex to use live graphics */
    bool m_liveGraphics;

    /** How much to pause on each execute */
    RlFloat m_pause;
    
    /** Policy to use for evaluating best moves and PV (typically greedy) */
    RlPolicy* m_policy;
    
    /** Number of moves to display in PV */
    int m_numPV;
    
    /** Number of best moves to display */
    int m_numBest;

    // Debugging data
    std::auto_ptr<RlTrace> m_featureTrace;
    std::auto_ptr<RlTrace> m_timeTrace;
    std::auto_ptr<RlLog> m_stepLog;
    std::auto_ptr<RlLog> m_gameLog;
    std::auto_ptr<RlLog> m_updateLog;
    std::auto_ptr<RlLog> m_evalLog;
};

//----------------------------------------------------------------------------

#endif // RLAGENTLOGGER_H
