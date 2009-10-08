//----------------------------------------------------------------------------
/** @file RlSimulator.h
    Simulate games from the current position
*/
//----------------------------------------------------------------------------

#ifndef RLSIMULATOR_H
#define RLSIMULATOR_H

#include "GoTimeControl.h"
#include "RlFactory.h"
#include "RlTrace.h"
#include "RlWeight.h"
#include "SgArray.h"
#include "SgBlackWhite.h"
#include "SgMove.h"
#include "SgPoint.h"
#include "SgTimer.h"
#include <boost/filesystem/fstream.hpp>

class RlAgent;
class RlPolicy;
class RlFuegoPlayout;

//----------------------------------------------------------------------------

class RlSimulator : public RlAutoObject
{
public:

    DECLARE_OBJECT(RlSimulator);
    
    RlSimulator(GoBoard& board, RlAgent* agent = 0, int maxgames = 1000);
    
    virtual void LoadSettings(std::istream& settings);
    virtual void Initialise();

    /** Simulate many games according to control settings */
    void Simulate();

    /** Pondering */
    void Ponder();

    /** Set flag when simulator is ready to ponder */
    void SetReady(bool ready) { m_ready = ready; }

    //-------------------------------------------------------------------------
    // Statistics and accessors
    
    /** Number of games simulated */
    int GetNumGames() const { return m_numGames; }

    /** Get most frequently chosen initial move during simulation */
    SgMove GetFreqMove() const;

    /** Get frequencies with which all initial moves were chosen */
    void GetAllFreqs(std::vector<RlFloat>& freqs) const;

    /** Get average score of all simulations */
    RlFloat AverageScore() const { return m_averageScore; }

    /** Get total number of steps in all simulations */
    int GetTotalSteps() const { return m_totalSteps; }

    RlAgent* GetAgent() { return m_agent; }

    void SetMaxGames(int maxgames) { m_maxGames = maxgames; }

protected:

    void InitLog();
    void StepLog(int nummoves, RlFloat score);

    void SimulateMaxGames();
    void SimulateMaxTime();
    void SimulateControlTime();
    void SimulatePonder();
    void Simulate(int controlmode);

    void ClearStats();
    void DisplayStats();
    void StartClock();
    bool Unsafe() const;

    void SelfPlayGame();
    SgMove SelectAndPlay(SgBlackWhite toplay, int movenum);
    bool GameOver();
    bool Truncate(int nummoves);
    void PlayOut(int& movenum, bool& resign);

    void RecordStart();
    void RecordEnd();
    void RecordVarStart();
    void RecordVarEnd();
    void RecordMove(SgMove move, SgBlackWhite colour);
    
private:

    /** Agent used to simulate moves */
    RlAgent* m_agent;
        
    enum
    {
        eMaxGames,
        eMaxTime,
        eControlTime,
        ePonder
    };

    /** Control mode to use for simulation */
    int m_controlMode;

    /** Min/max number of games to simulate each move (0 for no maximum) */
    int m_minGames;
    int m_maxGames;
    
    /** Min/max time to spend on simulations each move */
    double m_minTime;
    double m_maxTime;

    /** Proportion of time to allocate to simulation */
    RlFloat m_fraction;

    /** Don't simulate during safety time (0 for no safety time) */
    double m_safetyTime;

    /** Time elapsed in simulations this move */
    SgTimer m_elapsedTime;
    double m_lastTime;
    double m_searchTime;
    double m_timeLeft;
    int m_numGames;
    
    /** Simple time controller to determine how many simulations to perform */
    GoTimeControl m_timeControl;
        
    /** When to truncate simulated games (-1 = never) */
    int m_truncate;

    /** Whether to test for resignation in simulated games */
    bool m_resign;
    
    /** Play out games after truncation using this policy */
    RlPolicy* m_defaultPolicy;
    
    /** Fuego playout policy used during simulation (if used at all) */
    RlFuegoPlayout* m_fuegoPlayout;
    
    /** Maximum number of moves in simulation */
    int m_maxSimMoves;
    
    /** Do minimal simulation if opponent passed */
    bool m_minSimAfterPass;

    /** Remember start position for fast resets */
    bool m_fastReset;
    
    /** Log simulations */
    bool m_log;
    
    /** Record games */
    bool m_record;
    
    /** Whether pondering is enabled */
    bool m_pondering;

    /** Flag set when pondering is possible */
    bool m_ready;
        
    std::auto_ptr<RlLog> m_simLog;
    std::auto_ptr<RlLog> m_evalLog;
    bfs::ofstream m_gameRecord;
    RlFloat m_averageScore;
    int m_totalSteps;

    SgArray<int, SG_PASS + 1> m_freqs;
};

//----------------------------------------------------------------------------

#endif // RLSIMULATOR_H

