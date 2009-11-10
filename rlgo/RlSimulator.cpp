//----------------------------------------------------------------------------
/** @file RlSimulator.cpp
*/
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "RlSimulator.h"

#include "SgProp.h"
#include "RlAgent.h"
#include "RlAgentLog.h"
#include "RlEvaluator.h"
#include "RlPolicy.h"
#include "RlSetup.h"
#include "RlTimeControl.h"
#include "RlFuegoPlayout.h"

#include <boost/lexical_cast.hpp>

using namespace boost;
using namespace std;
using namespace GoBoardUtil;
using namespace RlMoveUtil;
using namespace SgPropUtil;

//----------------------------------------------------------------------------

IMPLEMENT_OBJECT(RlSimulator);

RlSimulator::RlSimulator(GoBoard& board, RlAgent* agent, int maxgames)
:   RlAutoObject(board),
    m_agent(agent),
    m_controlMode(eMaxGames),
    m_timeControl(0),
    m_maxGames(maxgames),
    m_truncate(-1),
    m_defaultPolicy(0),
    m_fuegoPlayout(0),
    m_maxSimMoves(RL_MAX_TIME - 2),
    m_log(false),
    m_record(false),
    m_pondering(false),
    m_ready(false),
    m_gameRecorder(board)
{
}

void RlSimulator::LoadSettings(istream& settings)
{
    int version;
    settings >> RlVersion(version, 13, 13);
    settings >> RlSetting<RlAgent*>("Agent", m_agent);
    settings >> RlSetting<int>("ControlMode", m_controlMode);
    settings >> RlSetting<RlTimeControl*>("TimeControl", m_timeControl);
    settings >> RlSetting<int>("MaxGames", m_maxGames);
    settings >> RlSetting<int>("Truncate", m_truncate);
    settings >> RlSetting<bool>("Resign", m_resign);
    settings >> RlSetting<RlPolicy*>("DefaultPolicy", m_defaultPolicy);
    settings >> RlSetting<RlFuegoPlayout*>("FuegoPlayout", m_fuegoPlayout);
    settings >> RlSetting<int>("MaxSimMoves", m_maxSimMoves);
    settings >> RlSetting<bool>("FastReset", m_fastReset);
    settings >> RlSetting<bool>("Log", m_log);
    settings >> RlSetting<bool>("Record", m_record);
    settings >> RlSetting<bool>("Pondering", m_pondering);
}

void RlSimulator::Initialise()
{
    if (m_log)
        InitLog();
        
    if (m_maxSimMoves > RL_MAX_TIME - 2)
        m_maxSimMoves = RL_MAX_TIME - 2;
}

void RlSimulator::Simulate()
{
    RlDebug(RlSetup::QUIET) << "\nSIMULATING...\n";
    Simulate(m_controlMode);
}

void RlSimulator::Simulate(int controlmode)
{
    // Remember current position for fast resetting
    m_agent->GetEvaluator()->Reset();
    if (m_fastReset)
        m_agent->GetEvaluator()->SetMark();

    // Temporarily switch ko rule to SIMPLEKO to avoid slow full board
    // repetition test in GoBoard::Play()
    GoRestoreKoRule restoreKoRule(m_board);
    m_board.Rules().SetKoRule(GoRules::SIMPLEKO);

    ClearStats();
    m_elapsedTime.Start();
    if (m_record)
        m_gameRecorder.RecordStart(this);

    switch (controlmode)
    {
        case eMaxGames:
            SimulateMaxGames();
            break;
        case eMaxTime:
            SimulateMaxTime();
            break;
        case ePonder:
            SimulatePonder();
            break;
    }

    if (m_record)
        m_gameRecorder.RecordEnd();
    DisplayStats();

    m_agent->GetEvaluator()->Reset();
    if (m_fastReset)
        m_agent->GetEvaluator()->ClearMark();
}

void RlSimulator::SimulateMaxGames()
{
    for (m_numGames = 0; m_numGames < m_maxGames; ++m_numGames)
        SelfPlayGame();
}

void RlSimulator::SimulateMaxTime()
{
    double searchTime = m_timeControl->TimeForCurrentMove(
        RlSetup::Get()->GetTimeRecord());
    for (m_numGames = 0; m_elapsedTime.GetTime() < searchTime; ++m_numGames)
        SelfPlayGame();
}

void RlSimulator::SimulatePonder()
{ 
    for (m_numGames = 0; !SgUserAbort(); ++m_numGames)
        SelfPlayGame();
}

void RlSimulator::ClearStats()
{
    m_totalSteps = 0;
    m_averageScore = 0;
    for (int i = 0; i <= SG_PASS; ++i)
        m_freqs[i] = 0;
}

void RlSimulator::DisplayStats()
{
    double seconds = m_elapsedTime.GetTime();
    RlDebug(RlSetup::VOCAL) << "Simulated " 
        << m_numGames << " games in " << seconds << " seconds: " 
        << m_numGames / seconds << " games/second ("
        << m_totalSteps / seconds << " moves/second)\n";

    if (m_numGames > 0)
    {
        RlDebug(RlSetup::VOCAL) << "Average length: " 
            << m_totalSteps / m_numGames << "\n";
        RlDebug(RlSetup::VOCAL) << "Average score: " 
            << m_averageScore << "\n";
        RlDebug(RlSetup::VOCAL) << "Most frequent first move: " 
            << SgWritePoint(GetFreqMove()) << "\n";
    }
}

void RlSimulator::Ponder()
{
    if (m_pondering && m_ready)
    {
        RlDebug(RlSetup::QUIET) << "\nPONDERING... " 
            << SgBW(m_board.ToPlay()) << " to play \n";
        Simulate(ePonder);
    }
}

SgMove RlSimulator::SelectAndPlay(SgBlackWhite toplay, int movenum)
{
    if (m_resign && QuickResign(m_board, toplay))
        return SG_RESIGN;
    SgMove move = m_agent->SelectMove();
    if (m_resign && move == SG_RESIGN)
        return SG_RESIGN;
    SG_ASSERT(m_board.IsLegal(move));
    m_agent->Execute(move, toplay);
    if (m_fuegoPlayout)
        m_fuegoPlayout->OnPlay();
    if (m_record)
        m_gameRecorder.RecordMove(move, toplay);
    if (movenum == 0)
        m_freqs[move]++;
    return move;
}

inline bool RlSimulator::GameOver()
{
    return TwoPasses(m_board)
        || m_board.MoveNumber() >= m_maxSimMoves;
}

inline bool RlSimulator::Truncate(int nummoves)
{
    // Truncate 1. after fixed number of moves in simulation 
    // 2. if there is no representation in the current state
    //    (to avoid backups passing through unknown states)
    return m_truncate >= 0 && nummoves >= m_truncate;
}

void RlSimulator::PlayOut(int& nummoves, bool& resign)
{
    while (!TwoPasses(m_board) && m_board.MoveNumber() < m_maxSimMoves
        && !(m_resign && QuickResign(m_board, m_board.ToPlay())))
    {
        nummoves++;
        SgBlackWhite toplay = m_board.ToPlay();

        // Use temporary state during playouts, don't record in history
        RlState state(nummoves, toplay);
        SgMove move;
        if (m_fuegoPlayout)
        {
            move = m_fuegoPlayout->GenerateMove();
            m_board.Play(move, toplay);
            m_fuegoPlayout->OnPlay();
        }
        else
        {
            move = m_defaultPolicy->SelectMove(state);
            //@todo: this is currently broken: it does record in history.
            m_agent->Execute(move, toplay, true);            
        }
        if (m_record)
            m_gameRecorder.RecordMove(move, toplay);
    }
    resign = QuickResign(m_board, m_board.ToPlay());
}

void RlSimulator::SelfPlayGame()
{
    // Self-play current game to completion from current position
    int nummoves = 0;
    bool gameover = false;
    bool resign = false;
    SgMove move = SG_NULLMOVE;
    SgBlackWhite toplay = m_board.ToPlay();
    if (m_fuegoPlayout)
        m_fuegoPlayout->OnStart();
    if (m_record)
        m_gameRecorder.RecordVarStart();
    
    // Main loop
    m_agent->NewGame();
    while (!gameover && !Truncate(nummoves))
    {
        toplay = m_board.ToPlay();
        move = SelectAndPlay(toplay, nummoves);
        resign = (move == SG_RESIGN);
        gameover = GameOver() || resign;
        if (!resign)
            nummoves++;
    }

    if (!resign && m_defaultPolicy && Truncate(nummoves))
        PlayOut(nummoves, resign);

    // End game (calls trainer to learn from history of this simulation)
    RlFloat score = m_agent->EndGame(resign);

    // Reset board to original position
    if (m_fuegoPlayout)
        m_fuegoPlayout->OnEnd();
    if (m_record)
        m_gameRecorder.RecordVarEnd();
    for (int i = 0; i < nummoves; ++i)
        m_board.Undo();
        
    // Update statistics
    RlFloat step = (1.0 / (m_numGames + 1));
    m_averageScore += (score - m_averageScore) * step;
    m_totalSteps += nummoves;

    if (m_log)
        StepLog(nummoves, score);
}

void RlSimulator::StepLog(int nummoves, RlFloat score)
{
    m_simLog->Log("NumMoves", nummoves);
    m_simLog->Log("Score", score);
    m_simLog->Step();
}

void RlSimulator::InitLog()
{
    m_simLog.reset(new RlLog(this));    
    m_simLog->AddItem("NumMoves");
    m_simLog->AddItem("Score");
}

SgMove RlSimulator::GetFreqMove() const
{
    int maxfreq = 0;
    int maxmove = SG_PASS;
    for (int i = 0; i <= SG_PASS; ++i)
    {
        if (m_freqs[i] > maxfreq)
        {
            maxmove = i;
            maxfreq = m_freqs[i];
        }
    }

    return maxmove;
}

void RlSimulator::GetAllFreqs(vector<RlFloat>& freqs) const
{
    for (GoBoard::Iterator i_board(m_board); i_board; ++i_board)
        freqs.push_back(static_cast<RlFloat>(m_freqs[*i_board]) 
            / static_cast<RlFloat>(m_numGames));
}

//----------------------------------------------------------------------------

RlGameRecorder::RlGameRecorder(const GoBoard& board)
:   m_board(board),
    m_count(0)
{
}

void RlGameRecorder::RecordStart(RlAutoObject* caller)
{
    m_gameRecord.close();
    bfs::path filename = RlLog::GenLogName(caller, 
        "Games" + lexical_cast<string>(m_count++), ".sgf");
    m_gameRecord.open(filename);
    if (!m_gameRecord)
        throw SgException("Failed to save simulation record");

    m_gameRecord << "(;FF[4]KM[" << m_board.Rules().Komi() << "]"
        << "SZ[" << m_board.Size() << "]";
    for (int i = 0; i < m_board.MoveNumber(); ++i)
    {
        RlStone stone = GetHistory(m_board, i);
        RecordMove(stone.first, stone.second);
    }
    m_gameRecord << "\n";
}

void RlGameRecorder::RecordEnd()
{
    m_gameRecord << ")\n";
}

void RlGameRecorder::RecordVarStart()
{
    m_gameRecord << "(";
}

void RlGameRecorder::RecordVarEnd()
{
    m_gameRecord << ")\n";
}

void RlGameRecorder::RecordMove(SgMove move, SgBlackWhite toplay)
{
    m_gameRecord << ";" << SgBW(toplay) << "[" 
        << PointToSgfString(move, m_board.Size(), SG_PROPPOINTFMT_GO, 4)
        << "]";
}

//----------------------------------------------------------------------------
