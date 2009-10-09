//----------------------------------------------------------------------------
/** @file RlAgentLog.cpp
    See RlAgentLog.h
*/
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "RlAgentLog.h"

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

IMPLEMENT_OBJECT(RlAgentLog);

RlAgentLog::RlAgentLog(GoBoard& board)
:   RlAutoObject(board),
    m_active(false),
    m_agent(0),
    m_debugLevel(RlSetup::VOCAL),
    m_logMode(LOG_LINEAR),
    m_logNext(0),
    m_interval(1),
    m_intervalMul(2),
    m_numGames(-1),
    m_logStartOnly(false),
    m_saveRecord(true),
    m_saveWeights(false),
    m_topTex(0),
    m_liveGraphics(false),
    m_traceFile(""),
    m_pause(0),
    m_numPV(10),
    m_numBest(4)
{
}

void RlAgentLog::LoadSettings(istream& settings)
{
    int version;
    settings >> RlVersion(version, 4, 3);
    settings >> RlSetting<RlAgent*>("Agent", m_agent);
    settings >> RlSetting<int>("DebugLevel", m_debugLevel);
    settings >> RlSetting<int>("LogMode", m_logMode);
    settings >> RlSetting<RlFloat>("Interval", m_interval);
    settings >> RlSetting<RlFloat>("IntervalMul", m_intervalMul);
    settings >> RlSetting<bool>("LogStartOnly", m_logStartOnly);
    settings >> RlSetting<bool>("SaveRecord", m_saveRecord);
    settings >> RlSetting<bool>("SaveWeights", m_saveWeights);
    settings >> RlSetting<int>("TopTex", m_topTex);
    settings >> RlSetting<bool>("LiveGraphics", m_liveGraphics);
    settings >> RlSetting<string>("TraceFeatures", m_traceFile);
    settings >> RlSetting<RlFloat>("Pause", m_pause);
    settings >> RlSetting<RlPolicy*>("Policy", m_policy);
    settings >> RlSetting<int>("NumPV", m_numPV);
    settings >> RlSetting<int>("NumBest", m_numBest);
}

void RlAgentLog::Initialise()
{
    m_agent->EnsureInitialised();
    InitLogs();
    AddItems();
    TraceFeatures();
}    

void RlAgentLog::InitLogs()
{
    m_featureTrace.reset(
        new RlTrace(this, "Weight"));
    m_timeTrace.reset(
        new RlTrace(this, "Time"));

    m_gameLog.reset(new RlLog(this, "Game"));
    m_stepLog.reset(new RlLog(this, "Step"));
    m_evalLog.reset(new RlLog(this, "Eval"));
}

void RlAgentLog::TraceFeatures()
{
    // Read features to trace from input file
    RlBinaryFeatures* featureset = m_agent->m_featureSet;
    bfs::ifstream tracefeatures(GetInputPath() / m_traceFile);
    tracefeatures >> ws;
    while (!tracefeatures.eof())
    {
        int featureindex = featureset->ReadFeature(tracefeatures);
        if (featureindex >= 0)
        {
            ostringstream oss;
            featureset->DescribeFeature(featureindex, oss);
            string featurename = oss.str();
            m_traceIndices[featureindex] = m_traceFeatures.size();
            m_traceFeatures.push_back(
                pair<string, int>(featurename, featureindex));
            m_featureTrace->AddItemToAll(featurename, featureindex);
        }
        tracefeatures >> ws;
    }
}

void RlAgentLog::AddItems()
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
}

void RlAgentLog::LogWeights()
{
    if (!LogIsActive()) return;
    for (int i = 0; i < ssize(m_traceFeatures); ++i)
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

void RlAgentLog::LogGame()
{
    if (!LogIsActive()) return;
    int length = m_agent->m_history->GetLength();
    RlFloat freturn = m_agent->GetHistory()->GetReturn();
    m_gameLog->Log("Game", m_numGames);
    m_gameLog->Log("Length", length);
    m_gameLog->Log("Return", freturn);
    m_gameLog->Step();
    
    (*m_timeTrace)["Value"]->Log(length, freturn);
    m_timeTrace->StepAll();
}

void RlAgentLog::LogEval()
{
    // @todo: best, worst, range and numevals tracked here
    // @todo: highlight best eval in live graphics
    // @todo: afterstate evals for all indices (value, uncertainty, etc.)
    
    if (!StepLogIsActive()) return;

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

void RlAgentLog::LogEvalMove(SgMove move)
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

void RlAgentLog::PrintValue()
{
    if (!StepLogIsActive()) return;

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

void RlAgentLog::LogStep()
{
    if (!StepLogIsActive()) return;

    RlState& state = m_agent->GetState();
    Debug(RlSetup::VOCAL) << "\n" << m_agent->m_timestep << ": "
        << SgBW(state.Colour()) << SgWritePoint(state.Move())
        << (state.OnPolicy() ? " (OnPolicy)\n" : " (OffPolicy)\n");

    RlFloat value = state.Evaluated() ? state.Eval() : 0;
    RlFloat pwin = Logistic(value);

    m_stepLog->Log("Game", GameIndex());
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

void RlAgentLog::TopTex()
{
    static const int rows = 10, cols = 2;

    if (LogIsActive() && m_topTex)
    {
        bfs::path texpath = RlLog::GenLogName(
            this, "Tex", ".tex", m_numGames);
        bfs::ofstream texfile(texpath);
        m_agent->GetFeatureSet()->TopTex(
            texfile, m_agent->GetWeightSet(), rows, cols);
    }
}

void RlAgentLog::SaveRecord()
{
    if (LogIsActive() && m_saveRecord)
    {
        bfs::path recordpath = RlLog::GenLogName(
            this, "Record", ".sgf", m_numGames);
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

void RlAgentLog::SaveWeights()
{
    if (LogIsActive() && m_saveWeights)
    {
        bfs::path weightpath = RlLog::GenLogName(
            this, 
            "Weights", 
            string(".w"), 
            m_numGames);
        m_agent->Save(weightpath);
    }
}

void RlAgentLog::PrintBoard()
{
    if (!StepLogIsActive())
        return;

    Debug(RlSetup::VOCAL) << m_board;
}

void RlAgentLog::PrintPV()
{
    if (!StepLogIsActive())
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

void RlAgentLog::PrintBest()
{
    if (!StepLogIsActive())
        return;

    Debug(RlSetup::VOCAL) << "Best moves: ";
    RlMoveSorter sorter;
    sorter.Sort(m_policy->Evaluator(), m_board.ToPlay());

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

void RlAgentLog::StartGame()
{
    m_numGames++;
    m_active = m_logMode != LOG_NEVER && m_numGames == m_logNext;
    if (!m_active)
        return;

    m_logNext = m_numGames + m_interval;
    switch (m_logMode)
    {
    case LOG_LINEAR: // e.g. 0, 100, 200, 300, ...
        break;
    case LOG_EXP: // e.g. 0, 100, 200, 400, 800, ...
        if (m_numGames > 0)
            m_interval *= m_intervalMul;
        break;
    case LOG_EXPINTERVAL: // e.g. 0, 100, 300, 700, 1500, ...
        m_interval *= m_intervalMul;
        break;
    case LOG_NICE: // e.g. 0, 1..9, 10..90, 100..900, ...
        if (m_numGames > 0 && 
            m_numGames + m_interval >= m_interval * m_intervalMul)
            m_interval *= m_intervalMul;
        break;
    }
}

void RlAgentLog::EndGame()
{        
}


//----------------------------------------------------------------------------
