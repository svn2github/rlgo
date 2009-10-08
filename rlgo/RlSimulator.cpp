//----------------------------------------------------------------------------
/** @file RlSimulator.cpp
*/
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "RlSimulator.h"

#include "GoBoardUtil.h"
#include "GoGame.h"
#include "SgProp.h"
#include "SgRandom.h"
#include "SgTimeRecord.h"
#include "RlAgent.h"
#include "RlAgentLog.h"
#include "RlBinaryFeatures.h"
#include "RlEvaluator.h"
#include "RlLearningRule.h"
#include "RlPolicy.h"
#include "RlSetup.h"
#include "RlTrainer.h"
#include "RlFuegoPlayout.h"
#include "RlUtils.h"

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
    m_minGames(maxgames / 10),
    m_maxGames(maxgames),
    m_minTime(0.1f),
    m_maxTime(1.0),
    m_fraction(1),
    m_safetyTime(0),
    m_timeControl(board),
    m_truncate(-1),
    m_defaultPolicy(0),
    m_fuegoPlayout(0),
    m_maxSimMoves(RL_MAX_TIME - 2),
    m_minSimAfterPass(false),
    m_log(false),
    m_record(false),
    m_pondering(false),
    m_ready(false)
{
}

void RlSimulator::LoadSettings(istream& settings)
{
    int version;
    settings >> RlVersion(version, 12, 12);
    settings >> RlSetting<RlAgent*>("Agent", m_agent);
    settings >> RlSetting<int>("ControlMode", m_controlMode);
    switch (m_controlMode)
    {
        case eMaxGames:
        {
            settings >> RlSetting<int>("MinGames", m_minGames);
            settings >> RlSetting<int>("MaxGames", m_maxGames);
            break;
        }
        case eMaxTime:
        {
            settings >> RlSetting<double>("MinTime", m_minTime);
            settings >> RlSetting<double>("MaxTime", m_maxTime);
            break;
        }
        case eControlTime:
        {
            RlFloat remain, finalspace, fastfactor;
            int fastopen;
            settings >> RlSetting<int>("FastOpen", fastopen);
            settings >> RlSetting<RlFloat>("FastFactor", fastfactor);
            settings >> RlSetting<RlFloat>("RemainingConstant", remain);
            settings >> RlSetting<double>("MinTime", m_minTime);
            settings >> RlSetting<RlFloat>("FinalSpace", finalspace);
            settings >> RlSetting<RlFloat>("Fraction", m_fraction);
            m_timeControl.SetFastOpenMoves(fastopen);
            m_timeControl.SetFastOpenFactor(fastfactor);
            m_timeControl.SetRemainingConstant(remain);
            m_timeControl.SetMinTime(m_minTime);
            m_timeControl.SetFinalSpace(finalspace);
            break;
        }
    }
    
    settings >> RlSetting<double>("SafetyTime", m_safetyTime);
    settings >> RlSetting<int>("Truncate", m_truncate);
    settings >> RlSetting<bool>("Resign", m_resign);
    settings >> RlSetting<RlPolicy*>("DefaultPolicy", m_defaultPolicy);
    settings >> RlSetting<RlFuegoPlayout*>("FuegoPlayout", m_fuegoPlayout);
    settings >> RlSetting<int>("MaxSimMoves", m_maxSimMoves);
    settings >> RlSetting<bool>("MinSimAfterPass", m_minSimAfterPass);
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
    StartClock();
    RecordStart();

    switch (controlmode)
    {
        case eMaxGames:
            SimulateMaxGames();
            break;
        case eMaxTime:
            SimulateMaxTime();
            break;
        case eControlTime:
            SimulateControlTime();
            break;
        case ePonder:
            SimulatePonder();
            break;
    }

    RecordEnd();
    DisplayStats();

    m_agent->GetEvaluator()->Reset();
    if (m_fastReset)
        m_agent->GetEvaluator()->ClearMark();
}

void RlSimulator::SimulateMaxGames()
{
    int maxgames;
    if (m_minSimAfterPass && m_board.GetLastMove() == SG_PASS)
        maxgames = m_minGames;
    else
        maxgames = m_maxGames;

    for (m_numGames = 0; m_numGames < maxgames; ++m_numGames)
    {
        if (Unsafe())
            break;
        SelfPlayGame();
    }
}

void RlSimulator::SimulateMaxTime()
{
    if (m_minSimAfterPass && m_board.GetLastMove() == SG_PASS)
        m_searchTime = m_minTime;
    else
        m_searchTime = m_maxTime;

    for (m_numGames = 0; m_elapsedTime.GetTime() < m_searchTime; ++m_numGames)
    {
        if (Unsafe())
            break;
        SelfPlayGame();
    }
}

void RlSimulator::SimulateControlTime()
{
    SgTimeRecord timerecord = RlSetup::Get()->GetGame()->Time();

    if (m_minSimAfterPass && m_board.GetLastMove() == SG_PASS)
        m_searchTime = m_minTime;
    else
        m_searchTime = m_timeControl.TimeForCurrentMove(timerecord)
            * m_fraction;

    for (m_numGames = 0; m_elapsedTime.GetTime() < m_searchTime; ++m_numGames)
    {
        if (Unsafe())
            break;
        SelfPlayGame();
    }
}

void RlSimulator::SimulatePonder()
{ 
    for (m_numGames = 0; !SgUserAbort(); ++m_numGames)
        SelfPlayGame();
}
    

void RlSimulator::StartClock()
{
    m_lastTime = 0;
    m_elapsedTime.Start();
    SgTimeRecord timerecord = RlSetup::Get()->GetGame()->Time();
    m_timeLeft = timerecord.TimeLeft(m_board.ToPlay());
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
    RlDebug(RlSetup::VOCAL) << "Average length: " 
        << m_totalSteps / m_numGames << "\n";
    RlDebug(RlSetup::VOCAL) << "Average score: " 
        << m_averageScore << "\n";
    RlDebug(RlSetup::VOCAL) << "Most frequent first move: " 
        << SgWritePoint(GetFreqMove()) << "\n";
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
    RecordMove(move, toplay);
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
    if (m_truncate >= 0 && nummoves >= m_truncate)
        return true;
    //@TODO: make this work properly
    // currently causes some games to terminate early
    //if (m_board.MoveNumber() > 0 && 
    //    m_agent->GetState().Active().GetTotalActive() == 0)
    //    return true;
    return false;
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
        RecordMove(move, toplay);
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
    RecordVarStart();
    
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
    RecordVarEnd();
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
    double elapsed = m_elapsedTime.GetTime();
    double gametime = elapsed - m_lastTime;
    m_lastTime = elapsed;

    m_simLog->Log("NumMoves", nummoves);
    m_simLog->Log("Score", score);
    m_simLog->Log("TimeUsed", gametime);
    m_simLog->Step();
}

void RlSimulator::InitLog()
{
    m_simLog.reset(new RlLog(this));    
    m_simLog->AddItem("NumMoves");
    m_simLog->AddItem("Score");
    m_simLog->AddItem("TimeUsed");
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

bool RlSimulator::Unsafe() const
{
    if (m_safetyTime != 0 && m_timeLeft < m_safetyTime)
    {
        RlDebug(RlSetup::QUIET) 
            << "Inside safety time, cutting off simulation\n";
        return true;
    }
    return false;
}

void RlSimulator::GetAllFreqs(vector<RlFloat>& freqs) const
{
    for (GoBoard::Iterator i_board(m_board); i_board; ++i_board)
        freqs.push_back(static_cast<RlFloat>(m_freqs[*i_board]) 
            / static_cast<RlFloat>(m_numGames));
}

void RlSimulator::RecordStart()
{
    if (m_record)
    {
        static int count = 0;
        bfs::path filename = RlLog::GenLogName(this, 
            "Games" + lexical_cast<string>(count++), ".sgf");
        m_gameRecord.close();
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
}

void RlSimulator::RecordEnd()
{
    if (m_record)
        m_gameRecord << ")\n";
}

void RlSimulator::RecordVarStart()
{
    if (m_record)
        m_gameRecord << "(";
}

void RlSimulator::RecordVarEnd()
{
    if (m_record)
        m_gameRecord << ")\n";
}

void RlSimulator::RecordMove(SgMove move, SgBlackWhite toplay)
{
    if (m_record)
        m_gameRecord << ";" << SgBW(toplay) << "[" 
            << PointToSgfString(move, m_board.Size(), SG_PROPPOINTFMT_GO, 4)
            << "]";
}

//----------------------------------------------------------------------------

