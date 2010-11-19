//----------------------------------------------------------------------------
/** @file RlSimulator.h
    Simulate games from the current position
*/
//----------------------------------------------------------------------------

#ifndef RLSIMULATOR_H
#define RLSIMULATOR_H

#include "GoTimeControl.h"
#include "RlFactory.h"
#include "RlTimeControl.h"
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
/** Simple class for recording straight simulations to .sgf file */
class RlGameRecorder
{
public:

    RlGameRecorder(const GoBoard& board);

    void RecordStart(RlAutoObject* caller);
    void RecordEnd();
    void RecordVarStart();
    void RecordVarEnd();
    void RecordMove(SgMove move, SgBlackWhite colour);
    
private:

    const GoBoard& m_board;
    int m_count;
    bfs::ofstream m_gameRecord;
};

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
    void SimulatePonder();
    void Simulate(int controlmode);

    void ClearStats();
    void DisplayStats();

    void SelfPlayGame();
    SgMove SelectAndPlay(SgBlackWhite toplay, int movenum);
    bool GameOver();
    bool Truncate(int nummoves);
    void PlayOut(int& movenum, bool& resign);

private:

    /** Agent used to simulate moves */
    RlAgent* m_agent;
        
    enum
    {
        eMaxGames,
        eMaxTime,
        ePonder,
        eNoSimulation
    };

    /** Control mode to use for simulation */
    int m_controlMode;

    /** Time controller to determine how many simulations to perform */
    RlTimeControl* m_timeControl;

    /** Maximum number of games to simulate each move */
    int m_maxGames;

    /** Number of games simulated */
    int m_numGames;
        
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
    RlFloat m_averageScore;
    int m_totalSteps;
    SgTimer m_elapsedTime;

    SgArray<int, SG_PASS + 1> m_freqs;
    
    RlGameRecorder m_gameRecorder;
};

//----------------------------------------------------------------------------

#endif // RLSIMULATOR_H

