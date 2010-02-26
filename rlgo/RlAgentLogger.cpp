//----------------------------------------------------------------------------
/** @file RlAgentLogger.cpp
    See RlAgentLogger.h
*/
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "RlAgentLogger.h"

#include "RlBinaryFeatures.h"
#include "RlWeightSet.h"
#include "RlEvaluator.h"
#include "RlLearningRule.h"
#include "RlMoveFilter.h"
#include "RlPolicy.h"
#include "RlSetup.h"
#include "RlSimulator.h"
#include "RlTrace.h"
#include "GoGame.h"
#include "SgGameWriter.h"
#include "SgPointSetUtil.h"
#include "SgWrite.h"

#include <boost/timer.hpp>

using namespace boost;
using namespace std;
using namespace RlMathUtil;
using namespace RlMoveUtil;
using namespace RlPathUtil; 
using namespace SgPointUtil;
using namespace SgPropUtil;

//----------------------------------------------------------------------------

IMPLEMENT_OBJECT(RlAgentLogger);

RlAgentLogger::RlAgentLogger(GoBoard& board)
:   RlLogger(board),
    m_agent(0),
    m_saveRecord(true),
    m_saveWeights(false),
    m_topTex(0),
    m_liveGraphics(false),
    m_pause(0),
    m_numPV(10),
    m_numBest(4)
{
}

void RlAgentLogger::LoadSettings(istream& settings)
{
    int version;
    settings >> RlVersion(version, 4, 3);
    settings >> RlSetting<RlAgent*>("Agent", m_agent);
    RlLogger::LoadSettings(settings);
    settings >> RlSetting<bool>("SaveRecord", m_saveRecord);
    settings >> RlSetting<bool>("SaveWeights", m_saveWeights);
    settings >> RlSetting<int>("TopTex", m_topTex);
    settings >> RlSetting<bool>("LiveGraphics", m_liveGraphics);
    settings >> RlSetting<RlFloat>("Pause", m_pause);
    settings >> RlSetting<RlPolicy*>("Policy", m_policy);
    settings >> RlSetting<int>("NumPV", m_numPV);
    settings >> RlSetting<int>("NumBest", m_numBest);
}

void RlAgentLogger::Initialise()
{
    m_agent->EnsureInitialised();
    TraceFeatures(m_agent->m_featureSet);
    InitLogs();
    AddItems();
}    

void RlAgentLogger::InitLogs()
{
    m_featureTrace.reset(
        new RlTrace(this, "Weight"));
    m_timeTrace.reset(
        new RlTrace(this, "Time"));

    m_gameLog.reset(new RlLog(this, "Game"));
    m_stepLog.reset(new RlLog(this, "Step"));
    m_evalLog.reset(new RlLog(this, "Eval"));
}

void RlAgentLogger::AddItems()
{
    m_featureTrace->AddLog("Weight");
#ifdef RL_ELIGIBILITY
    m_featureTrace->AddLog("Eligibility");
#endif
#ifdef RL_COUNT
    m_featureTrace->AddLog("Count");
#endif
#ifdef RL_STEP
    m_featureTrace->AddLog("Step");
#endif
#ifdef RL_TRACE
    m_featureTrace->AddLog("Trace");
#endif

    m_gameLog->AddItem("Game");
    m_gameLog->AddItem("Length");
    m_gameLog->AddItem("Return");

    m_stepLog->AddItem("Game");
    m_stepLog->AddItem("TimeStep");
    m_stepLog->AddItem("Colour");
    m_stepLog->AddItem("Move");
    m_stepLog->AddItem("Evaluated");
    m_stepLog->AddItem("Terminal");
    m_stepLog->AddItem("TotalActive");
    m_stepLog->AddItem("Reward");
    m_stepLog->AddItem("Value");
    m_stepLog->AddItem("PWin");
    m_stepLog->AddItem("BestMove");
    m_stepLog->AddItem("BestValue");
    m_stepLog->AddItem("OnPolicy");
    
    m_evalLog->AddItem("PASS", SG_PASS, string("X"));
    for (GoBoard::Iterator i_board(m_board); i_board; ++i_board)
    {
        SgMove move = *i_board;
        ostringstream oss;
        oss << SgWritePoint(move);
        m_evalLog->AddItem(oss.str(), move, string("X"));
    }
    
    m_timeTrace->AddLog("Value");

    for (int i = 0; i < GetNumTraceFeatures(); ++i)
        m_featureTrace->AddItemToAll(
            GetTraceFeatureName(i), 
            GetTraceFeatureIndex(i));
}

void RlAgentLogger::LogWeights()
{
    if (!GameLogIsActive()) return;
    for (int i = 0; i < GetNumTraceFeatures(); ++i)
    {
        int featureindex = GetTraceFeatureIndex(i);
        RlWeightSet* wset = m_agent->GetWeightSet();
        (*m_featureTrace)["Weight"]->Log(featureindex, 
            wset->Get(featureindex).Weight());
#ifdef RL_ELIGIBILITY
        (*m_featureTrace)["Eligibility"]->Log(featureindex, 
            wset->Get(featureindex).Eligibility());
#endif
#ifdef RL_COUNT
        (*m_featureTrace)["Count"]->Log(featureindex, 
            wset->Get(featureindex).Count());
#endif
#ifdef RL_STEP
        (*m_featureTrace)["Step"]->Log(featureindex, 
            wset->Get(featureindex).Step());
#endif
#ifdef RL_TRACE
        (*m_featureTrace)["Trace"]->Log(featureindex, 
            wset->Get(featureindex).Trace());
#endif
    }
    m_featureTrace->StepAll();
}

void RlAgentLogger::LogGame()
{
    if (!GameLogIsActive()) return;
    int length = m_agent->m_history->GetLength();
    RlFloat freturn = m_agent->GetHistory()->GetReturn();
    m_gameLog->Log("Game", m_agent->m_numGames);
    m_gameLog->Log("Length", length);
    m_gameLog->Log("Return", freturn);
    m_gameLog->Step();
    
    (*m_timeTrace)["Value"]->Log(length, freturn);
    m_timeTrace->StepAll();
}

void RlAgentLogger::LogEval()
{
    // @todo: best, worst, range and numevals tracked here
    // @todo: highlight best eval in live graphics
    // @todo: afterstate evals for all indices (value, uncertainty, etc.)
    
    if (!MoveLogIsActive()) return;

    if (m_liveGraphics)
    {
        cerr << "gogui-gfx: CLEAR\n";
        cerr << "gogui-gfx: INFLUENCE ";
    }

    const RlMoveFilter* filter = m_agent->GetEvaluator()->GetMoveFilter();
    for (RlMoveFilter::Iterator i_filter(*filter, m_board.ToPlay());
        i_filter; ++i_filter)
    {
        SgMove move = *i_filter;
        LogEvalMove(move);
    }

    m_evalLog->Step();

    if (m_liveGraphics)
        cerr << endl;
}

void RlAgentLogger::LogEvalMove(SgMove move)
{
    RlEvaluator* evaluator = m_agent->GetEvaluator();
    SgBlackWhite toplay = m_board.ToPlay();    
    RlFloat eval = evaluator->EvaluateMove(move, toplay);
    m_evalLog->Log(move, eval);

    /*
    RlWeightSet* wset = m_agent->GetWeightSet();
    const RlChangeList& changelist = evaluator->ChangeList();
    Debug(RlSetup::VERBOSE) << "Evaluating " << SgWritePoint(move) 
        << " = " << eval << ":\n";
    for (RlChangeList::Iterator i_changes(changelist); i_changes; ++i_changes)
    {
        RlWeight& weight = wset->Get(i_changes->m_featureIndex);
        m_agent->GetFeatureSet()->DescribeFeature(
            i_changes->m_featureIndex, Debug(RlSetup::VERBOSE));
        Debug(RlSetup::VERBOSE) 
            << " Slot: " << i_changes->m_slot << ", " 
            << "Index: " << i_changes->m_featureIndex << ", "
            << "Occurrences: " << i_changes->m_occurrences << ", "
            << "Weight: " << weight.Weight() << "\n";
    }
    */

    if (m_liveGraphics)
    {
        RlFloat pwin = TanH(eval);
        cerr << SgWritePoint(move) << " " << pwin << " ";
    }
}

void RlAgentLogger::PrintValue()
{
    if (!MoveLogIsActive()) return;

    RlState& state = m_agent->GetState();
    RlFloat value = state.Eval();
    RlFloat pwin = Logistic(value);

    Debug(RlSetup::VOCAL) << "Value: " << value << " (";
    Debug(RlSetup::VOCAL).precision(4);
    Debug(RlSetup::VOCAL) << pwin * 100 << "% winning for Black)\n";

    Debug(RlSetup::VERBOSE) << "Active features:\n";
    RlWeightSet* wset = m_agent->GetWeightSet();
    for (RlActiveSet::Iterator i_active(state.Active()); 
        i_active; ++i_active)
    {
        RlWeight& weight = wset->Get(i_active->m_featureIndex);
        m_agent->GetFeatureSet()->DescribeFeature(
            i_active->m_featureIndex, 
            Debug(RlSetup::VERBOSE));
        Debug(RlSetup::VERBOSE) 
            << " Slot: " << i_active.Slot() << ", " 
            << "Index: " << i_active->m_featureIndex << ", "
            << "Occurrences: " << i_active->m_occurrences << ", "
            << "Weight: " << weight.Weight() << "\n";
    }
}

void RlAgentLogger::LogStep()
{
    if (!MoveLogIsActive()) return;

    RlState& state = m_agent->GetState();
    Debug(RlSetup::VOCAL) << "\n" << m_agent->m_timestep << ": "
        << SgBW(state.Colour()) << SgWritePoint(state.Move())
        << (state.OnPolicy() ? " (OnPolicy)\n" : " (OffPolicy)\n");

    RlFloat value = state.Evaluated() ? state.Eval() : 0;
    RlFloat pwin = Logistic(value);

    m_stepLog->Log("Game", m_agent->m_numGames);
    m_stepLog->Log("TimeStep", state.TimeStep());
    m_stepLog->Log("Colour", SgBW(state.Colour()));
    m_stepLog->Log("Move", state.Move());
    m_stepLog->Log("Evaluated", state.Evaluated());
    m_stepLog->Log("Terminal", state.Terminal());
    m_stepLog->Log("TotalActive", state.ActiveSet() ? 
        state.Active().GetTotalActive() : 0);
    m_stepLog->Log("Reward", state.Reward());
    m_stepLog->Log("Value", value);
    m_stepLog->Log("PWin", pwin);
    if (state.PolicyType() == RlState::POL_BEST)
    {
        m_stepLog->Log("BestMove", SgWritePoint(state.BestMove()));
        m_stepLog->Log("BestValue", state.BestValue());
    }
    m_stepLog->Log("OnPolicy", state.OnPolicy());
    m_stepLog->Step();

    (*m_timeTrace)["Value"]->Log(
        m_agent->m_timestep, value);

    if (m_liveGraphics && state.Move() != SG_PASS)
        cerr << "gogui-gfx: VAR " << SgBW(state.Colour()) 
            << " " << PointToString(state.Move()) << endl;

    timer t;
    while (t.elapsed() < m_pause)
        ;
}

void RlAgentLogger::TopTex()
{
    static const int rows = 2, cols = 10;

    if (GameLogIsActive() && m_topTex)
    {
        bfs::path texpath = RlLog::GenLogName(
            this, "Tex", ".tex", m_agent->m_numGames);
        bfs::ofstream texfile(texpath);
        m_agent->GetFeatureSet()->TopTex(
            texfile, m_agent->GetWeightSet(), rows, cols);
    }
}

void RlAgentLogger::SaveRecord()
{
    if (GameLogIsActive() && m_saveRecord)
    {
        bfs::path recordpath = RlLog::GenLogName(
            this, "Record", ".sgf", m_agent->m_numGames);
        bfs::ofstream gamerecord(recordpath);
        if (!gamerecord)
        {
            cout << "Record path: " << recordpath << endl;
            throw SgException("Failed to save game record");
        }

        gamerecord << "(;FF[4]KM[" << m_board.Rules().Komi() << "]"
            << "SZ[" << m_board.Size() << "]";
        for (int i = 0; i < m_board.MoveNumber(); ++i)
        {
            RlStone stone = GetHistory(m_board, i);
            gamerecord << ";" << SgBW(stone.second) 
                << "[" 
                << PointToSgfString(
                    stone.first, m_board.Size(), SG_PROPPOINTFMT_GO, 4)
                << "]";
        }
        gamerecord << ")\n";
    }
}

void RlAgentLogger::SaveWeights()
{
    if (GameLogIsActive() && m_saveWeights)
    {
        bfs::path weightpath = RlLog::GenLogName(
            this, 
            "Weights", 
            string(".w"), 
            m_agent->m_numGames);
        m_agent->Save(weightpath);
    }
}

void RlAgentLogger::PrintBoard()
{
    if (!MoveLogIsActive())
        return;

    Debug(RlSetup::VOCAL) << m_board;
}

void RlAgentLogger::PrintPV()
{
    if (!MoveLogIsActive())
        return;

    Debug(RlSetup::VOCAL) << "PV: ";
    for (int i = 0; i < m_numPV; ++i)
    {
        RlState state(m_agent->GetTimeStep() + i, m_board.ToPlay());
        SgMove move = m_policy->SelectMove(state);
        m_agent->m_evaluator->PlayExecute(move, m_board.ToPlay(), false);
        Debug(RlSetup::VOCAL) << SgWritePoint(move);
        if (i < m_numPV - 1)
            Debug(RlSetup::VOCAL) << " ";
    }
    for (int i = 0; i < m_numPV; ++i)
        m_agent->m_evaluator->TakeBackUndo(false);
    Debug(RlSetup::VOCAL) << "\n";
}

void RlAgentLogger::PrintBest()
{
    if (!MoveLogIsActive())
        return;

    Debug(RlSetup::VOCAL) << "Best moves: ";
    RlMoveSorter sorter;
    sorter.Evaluate(m_policy->Evaluator(), m_board.ToPlay());
    sorter.SortMoves(m_board.ToPlay());

    Debug(RlSetup::VOCAL).precision(4);
    for (int i = 0; i < m_numBest && i < sorter.GetNumMoves(); ++i)
    {
        Debug(RlSetup::VOCAL) << SgWritePoint(sorter.GetMove(i)) << "=" 
            << Logistic(sorter.GetEval(i)) * 100 << "%";
        if (i < m_numBest - 1)
            Debug(RlSetup::VOCAL) << "; ";
    }
    Debug(RlSetup::VOCAL) << "\n";
}

//----------------------------------------------------------------------------
