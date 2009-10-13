//----------------------------------------------------------------------------
/** @file RlSearch.cpp
*/
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "RlSearch.h"

#include "GoBoard.h"
#include "GoBoardUtil.h"
#include "GoGame.h"
#include "GoTimeControl.h"
#include "RlAgent.h"
#include "RlEvaluator.h"
#include "RlMoveFilter.h"
#include "RlProcessUtil.h"
#include "RlSetup.h"
#include "RlTrace.h"
#include "RlUtils.h"
#include "SgPoint.h"
#include "SgTimeRecord.h"

#include <boost/lexical_cast.hpp>
#include <boost/filesystem/fstream.hpp>
#include <iostream>
#include <algorithm>
#include <functional>
#include <math.h>

using namespace boost;
using namespace std;
using namespace GoBoardUtil;
using namespace RlMathUtil;
using namespace RlMoveUtil;
using namespace RlPathUtil;

const int MAX_EVAL = 30000; // SgSearch uses signed 16-bit values in hashtable

//----------------------------------------------------------------------------

RlSearch::RlSearch(GoBoard& board, RlEvaluator* evaluator, RlAgent* agent)
:   GoSearch(board, 0),
    RlPolicy(board, evaluator),
    m_minDepth(0),
    m_maxDepth(3),
    m_iterative(true),
    m_trace(false),
    m_log(true),
    m_useProbCut(false),
    m_searchValue(0),
    m_variation(0),
    m_hashTable(0),
    m_agent(agent)
{
    SetKillers(true);
    SetOpponentBest(true);
    SetNullMove(true);
    SetAbortFrequency(1000);
}

RlSearch::~RlSearch()
{
    if (m_hashTable)
        delete m_hashTable;
}

void RlSearch::LoadSettings(istream& settings)
{
    RlPolicy::LoadSettings(settings);
    int version, abortfreq, nullmovedepth;
    bool killers, opponentbest;
    settings >> RlVersion(version, 5, 5);
    settings >> RlSetting<RlAgent*>("SearchAgent", m_agent);
    settings >> RlSetting<int>("HashSize", m_hashSize);
    settings >> RlSetting<bool>("Iterative", m_iterative);
    settings >> RlSetting<bool>("Killers", killers);
    settings >> RlSetting<bool>("OpponentBest", opponentbest);
    settings >> RlSetting<int>("NullMoveDepth", nullmovedepth);
    settings >> RlSetting<int>("MinDepth", m_minDepth);
    settings >> RlSetting<int>("MaxDepth", m_maxDepth);
    settings >> RlSetting<bool>("Trace", m_trace);
    settings >> RlSetting<bool>("Log", m_log);
    settings >> RlSetting<bool>("ProbCut", m_useProbCut);
    settings >> RlSetting<int>("AbortFrequency", abortfreq);
    if (m_useProbCut)
        settings >> RlSetting<string>("ProbCutFile", m_probCutFile);
    
    SetKillers(killers);
    SetOpponentBest(opponentbest);
    SetNullMove(nullmovedepth > 0);
    if (nullmovedepth)
        SetNullMoveDepth(nullmovedepth);
    SetAbortFrequency(abortfreq);
    
}

void RlSearch::Initialise()
{
    if (m_log)
        InitLog();

    m_hashTable = new SgSearchHashTable(m_hashSize);
    SetHashTable(m_hashTable);

    InitProbCut();
}

SgMove RlSearch::SelectMove(RlState& state)
{
    // Search does not currently support difference computations
    m_evaluator->EnsureSimple();

    // If there are no legal moves then return Pass immediately
    if (m_evaluator->GetMoveFilter()->Empty(state.Colour()))
        return SG_PASS;

    // Start a trace
    if (m_trace)
        InitTracing("RLGO search");

    RlDebug(RlSetup::VOCAL) << "\nSEARCHING...\n";

    SgVector<SgMove> pv;
    RunSearch(&pv, TraceNode());

    SgMove bestmove;
    if (pv.IsEmpty())
        bestmove = SG_PASS;
    else
        bestmove = pv.Top();

    if (m_trace)
        AppendTrace(RlSetup::Get()->GetGame()->CurrentNode());
        
    return bestmove;
}

int RlSearch::SearchToDepth(int depth, SgVector<SgMove>* pv, SgNode* tracenode)
{
    int value;

    if (m_iterative)
        value = IteratedSearch(0, depth, pv, true, tracenode);
    else
        value = DepthFirstSearch(depth, pv, true, tracenode);

    if (m_log)
    {
        if (m_iterative)
            FinalPlyLog();
        StepLog(depth, value, pv);
    }

    ClearVariation(); // In case search was aborted
    m_searchValue = UnscaleEval(value);
    return value;
}

bool RlSearch::Execute(SgMove move, int* delta, int depth)
{
    SG_UNUSED(delta);
    SG_UNUSED(depth);
    GoBoard& bd = GoSearch::Board();
    SgBlackWhite toplay = bd.ToPlay();

    // During search the move filter doesn't check legality
    // (since that is done by GoSearch::Execute)
    if (!m_evaluator->GetMoveFilter()->ConsiderMove(move, toplay, false))
        return false;

    // Play move and check for legality
    if (! GoSearch::Execute(move, delta, depth))
        return false;
    m_evaluator->Execute(move, toplay, false);

    if (m_variation)
        m_variation[CurrentDepth()] = move;
    return true;
}

void RlSearch::TakeBack()
{
    GoSearch::TakeBack();
    m_evaluator->Undo(false);
    if (m_variation)
        m_variation[CurrentDepth() - 1] = SG_NULLMOVE;
    // NB current depth only decremented by SgSearch after this call completes
}

int RlSearch::Evaluate(SgVector<SgMove>* sequence, bool* isExact, int depth)
{
    SG_UNUSED(sequence);
    SG_UNUSED(depth);
    *isExact = false;

    int eval;
    RlFloat value = m_evaluator->Eval();
    eval = ScaleEval(value);

    if (m_trace)
        TraceValue(eval);
    
    return eval;
}

int RlSearch::ScaleEval(RlFloat value)
{
    static const RlFloat scaleconstant = MAX_EVAL / RL_MAX_EVAL;
    static const RlFloat tol = 0.01f;

    // Convert [-RL_MAX_EVAL, +RL_MAX_EVAL] to [-MAX_EVAL, MAX_EVAL]
    RlFloat min = -RL_MAX_EVAL + tol;
    RlFloat max = +RL_MAX_EVAL - tol;

    // Ensure there is no wrap-around
    if (value < min) value = min;
    if (value > max) value = max;
    RlFloat floatscale = value * scaleconstant;

    // Discretise
    int eval = static_cast<int>(floatscale);
    
    // SgSearch uses +ve to mean good for ToPlay, -ve bad for ToPlay
    if (Board().ToPlay() == SG_WHITE)
        eval = -eval;
        
    return eval;
}

RlFloat RlSearch::UnscaleEval(int value)
{
    static const RlFloat scaleconstant = RL_MAX_EVAL / MAX_EVAL;

    // SgSearch uses +ve to mean good for ToPlay, -ve bad for ToPlay
    if (Board().ToPlay() == SG_WHITE)
        value = -value;
        
    // Convert [-MAX_EVAL, MAX_EVAL] to [-RL_MAX_EVAL, +RL_MAX_EVAL]
    return static_cast<RlFloat>(value) * scaleconstant;
}

bool RlSearch::TraceIsOn() const
{
    return m_trace;
}  

void RlSearch::StepLog(int depth, int value, SgVector<SgMove>* pv)
{        
    SgSearchStatistics stats;
    GetStatistics(&stats);
    RlFloat eval = UnscaleEval(value);
    RlFloat pwin = Logistic(eval);

    m_searchLog->Log("PV", any(WriteSequence(*pv)));
    m_searchLog->Log("Value", value);
    m_searchLog->Log("Eval", eval);
    m_searchLog->Log("PWin", pwin);
    m_searchLog->Log("DepthAsked", depth);
    m_searchLog->Log("DepthReached", stats.DepthReached());
    m_searchLog->Log("Aborted", Aborted());
    m_searchLog->Log("TimeUsed", stats.TimeUsed());
    m_searchLog->Log("NodesPerSec", stats.NumNodes() / stats.TimeUsed());
    m_searchLog->Log("NumNodes", stats.NumNodes());
    m_searchLog->Log("NumEvals", stats.NumEvals());
    m_searchLog->Log("NumMoves", stats.NumMoves());
    m_searchLog->Step();
}

void RlSearch::InitLog()
{
    m_searchLog.reset(new RlLog(this));
    m_searchLog->AddItem("PV");
    m_searchLog->AddItem("Value");
    m_searchLog->AddItem("Eval");
    m_searchLog->AddItem("PWin");
    m_searchLog->AddItem("DepthAsked");
    m_searchLog->AddItem("DepthReached");
    m_searchLog->AddItem("Aborted");
    m_searchLog->AddItem("TimeUsed");
    m_searchLog->AddItem("NodesPerSec");
    m_searchLog->AddItem("NumNodes");
    m_searchLog->AddItem("NumEvals");
    m_searchLog->AddItem("NumMoves");
    
    m_searchTrace.reset(new RlTrace(this));
    m_searchTrace->AddLog("PV");
    m_searchTrace->AddLog("Value");
    m_searchTrace->AddLog("Eval");
    m_searchTrace->AddLog("PWin");
    m_searchTrace->AddLog("Nodes");
    m_searchTrace->AddLog("TimeUsed");
    for (int i = 0; i <= m_maxDepth; ++i)
        m_searchTrace->AddItemToAll(lexical_cast<string>(i), i);
}

void RlSearch::InitProbCut()
{
    if (!m_useProbCut) return;
    if (m_probCutFile == "") return;
    bfs::ifstream cutfile(GetInputPath() / m_probCutFile);
    if (!cutfile)
    {
        ostringstream ss;
        ss << "Couldn't open " << m_probCutFile;
        throw SgException(ss.str());
    }
    
    RlFloat thresh;
    cutfile >> thresh;
    probcut.SetThreshold(thresh);
    
    while (!cutfile.eof())
    {
        SgProbCut::Cutoff c;
        cutfile >> c.shallow;
        cutfile >> c.deep;
        cutfile >> c.a;
        cutfile >> c.b;
        cutfile >> c.sigma;
        
        if (cutfile.eof()) break;
        probcut.AddCutoff(c);
    }
    
    probcut.SetEnabled(true);
    
    SetProbCut(&probcut);
}

void RlSearch::StartOfDepth(int depth)
{
    SgSearch::StartOfDepth(depth);
    
    // m_log some data at each new ply of search 
    if (depth == 0)
        return;
        
    if (m_log)
        StepPlyLog(depth - 1);
}

void RlSearch::StepPlyLog(int depth)
{
    SgSearchStatistics stats;
    GetStatistics(&stats);
    SgVector<SgMove> pv;
    //@todo: prevvalue deprecated in Fuego
    // int value = PrevValue(&pv);
    // RlFloat eval = UnscaleEval(value);
    // RlFloat pwin = Logistic(eval);
    
    RlDebug(RlSetup::VOCAL) << "\nDepth reached: " 
        << stats.DepthReached() << "\n"
        << "Nodes visited: " << stats.NumNodes() << "\n"
        << "Time used: " << stats.TimeUsed() << "\n"
        << "Principal variation: " << WriteSequence(pv) << "\n";
        // << "Evaluation: " << eval << " (" << pwin * 100 << "%)\n";
    
    (*m_searchTrace)["PV"]->Log(depth, any(WriteSequence(pv)));
    // (*m_searchTrace)["Value"]->Log(depth, value);
    // (*m_searchTrace)["Eval"]->Log(depth, eval);
    // (*m_searchTrace)["PWin"]->Log(depth, pwin);
    (*m_searchTrace)["Nodes"]->Log(depth, stats.NumNodes());
    (*m_searchTrace)["TimeUsed"]->Log(depth, stats.TimeUsed());
}

void RlSearch::FinalPlyLog()
{
    SgSearchStatistics stats;
    GetStatistics(&stats);
    int depth = stats.DepthReached();

    StepPlyLog(depth);

    m_searchTrace->StepAll();
}

string RlSearch::WriteSequence(SgVector<SgMove>& sequence)
{
    if (sequence.IsEmpty())
        return "NONE";

    ostringstream oss;
    bool first = true;
    for (SgVectorIterator<SgMove> i_sequence(sequence); 
        i_sequence; ++i_sequence)
    {
        if (!first)
            oss << " ";
        oss << SgWritePoint(*i_sequence);
        first = false;
    }
    
    return oss.str();
}

bool RlSearch::SearchValue(RlFloat& value) const 
{ 
    value = m_searchValue; 
    return true;
}

void RlSearch::ClearVariation()
{
    for (int i = 0; i < RL_MAX_TIME; ++i)
        m_variation[i] = SG_NULLMOVE;
}

//----------------------------------------------------------------------------

IMPLEMENT_OBJECT(RlMainSearch);

RlMainSearch::RlMainSearch(GoBoard& board, RlEvaluator* evaluator, 
    RlAgent* agent)
:   RlSearch(board, evaluator, agent),
    m_controlMode(eNone),
    m_maxBreadth(0),
    m_maxTime(0),
    m_fraction(1.0),
    m_branchPower(0.5f),
    m_sharedMemoryId("NULL"),
    m_sharedMemory(0),
    m_timeControl(m_maxTime),
    m_iterControl(Board(), m_minDepth, 
        m_maxTime, m_maxTime * 2, m_branchPower),
    m_timeManager(Board())
{
}

RlMainSearch::~RlMainSearch()
{
    if (m_sharedMemory)
        delete m_sharedMemory;
    else
        delete m_variation;
}

void RlMainSearch::LoadSettings(istream& settings)
{
    RlSearch::LoadSettings(settings);

    settings >> RlSetting<int>("MaxBreadth", m_maxBreadth);
    settings >> RlSetting<int>("ControlMode", m_controlMode);
    switch (m_controlMode)
    {
        case eMaxTime:
        {
            settings >> RlSetting<RlFloat>("MaxTime", m_maxTime);
            break;
        }
        case eControlTime: // intentional drop-through
        case eControlIter:
        {
            RlFloat remain, mintime, finalspace, fastfactor;
            int fastopen;
            settings >> RlSetting<int>("FastOpen", fastopen);
            settings >> RlSetting<RlFloat>("FastFactor", fastfactor);
            settings >> RlSetting<RlFloat>("RemainingConstant", remain);
            settings >> RlSetting<RlFloat>("MinTime", mintime);
            settings >> RlSetting<RlFloat>("FinalSpace", finalspace);
            settings >> RlSetting<RlFloat>("Fraction", m_fraction);
            settings >> RlSetting<RlFloat>("BranchPower", m_branchPower);
            m_timeManager.SetFastOpenMoves(fastopen);
            m_timeManager.SetFastOpenFactor(fastfactor);
            m_timeManager.SetRemainingConstant(remain);
            m_timeManager.SetMinTime(mintime);
            m_timeManager.SetFinalSpace(finalspace);
            break;
        }
    }
    settings >> RlSetting<RlFloat>("SafetyTime", m_safetyTime);
    settings >> RlSetting<string>("Shared", m_sharedMemoryId);
}

void RlMainSearch::Initialise()
{
    RlSearch::Initialise();
    if (m_sharedMemoryId != "NULL")
    {
        int bytes = RL_MAX_TIME * sizeof(SgMove);
        bfs::path pathname = GetInputPath() / m_sharedMemoryId;
        m_sharedMemory = new RlSharedMemory(pathname, 0, bytes);
        m_variation = (SgMove*) m_sharedMemory->GetData();
    }
    else
    {
        m_variation = new SgMove[RL_MAX_TIME];
    }
    ClearVariation();
}

int RlMainSearch::RunSearch(SgVector<SgMove>* pv, SgNode* node)
{
    switch (m_controlMode)
    {
        case eMaxTime:
        {
            SetSearchControl(&m_timeControl);
            m_timeControl.SetMaxTime(m_maxTime);
            return SearchToDepth(GetDepth(), pv, node);
        }

        case eControlTime:
        {
            RlFloat searchtime = 
                m_timeManager.TimeForCurrentMove(RlSetup::Get()->GetTimeRecord())
                * m_fraction; // proportion of time to spend on search
            SetSearchControl(&m_timeControl);
            m_timeControl.SetMaxTime(searchtime);
            return SearchToDepth(GetDepth(), pv, node);
        }

        case eControlIter:
        {
            RlFloat searchtime = 
                m_timeManager.TimeForCurrentMove(RlSetup::Get()->GetTimeRecord())
                * m_fraction; // proportion of time to spend on search
            SetSearchControl(&m_iterControl);
            m_iterControl.SetMinDepth(m_minDepth);
            m_iterControl.SetMaxTime(searchtime, searchtime * 2);
            m_iterControl.SetBranchPower(m_branchPower);
            return SearchToDepth(GetDepth(), pv, node);
        }

        case eNone:
        default:
        {
            SetSearchControl(0);
            return SearchToDepth(GetDepth(), pv, node);
        }
    }
}

void RlMainSearch::Generate(SgVector<SgMove>* moves, int depth)
{
    SG_UNUSED(depth);

    // Execute already tests for legality
    // so there is no need to do full legality test here
    if (m_maxBreadth > 0 
        && m_maxBreadth < m_evaluator->GetMoveFilter()->NumVacant())
        GenerateN(*moves, m_maxBreadth);
    else
        GenerateAll(*moves);
}

void RlMainSearch::GenerateN(SgVector<SgMove>& moves, int N)
{
    static RlMoveSorter sorter;
    sorter.Sort(m_evaluator, Board().ToPlay());
    for (int i = 0; i < N && i < sorter.GetNumMoves(); ++i)
        moves.Append(sorter.GetMove(i));
}

void RlMainSearch::GenerateAll(SgVector<SgMove>& moves)
{
    // Includes potentially illegal or disallowed moves
    m_evaluator->GetMoveFilter()->GetVacantVector(moves);
}

int RlMainSearch::GetDepth() const
{
    RlFloat timeleft = RlSetup::Get()->GetTimeRecord().TimeLeft(Board().ToPlay());

    int depth = m_maxDepth;
    if (m_safetyTime != 0 && timeleft < m_safetyTime)
        depth = m_minDepth; 
    SG_ASSERT(depth);
    return depth;
}

//----------------------------------------------------------------------------

RlIterSearchControl::RlIterSearchControl(GoBoard& board, int mindepth, 
    double fulltime, double aborttime, double branchpower)
:   m_board(board), m_minDepth(mindepth), 
    m_fullTime(fulltime), m_abortTime(aborttime),
    m_branchPower(branchpower)
{
}

bool RlIterSearchControl::Abort(double elapsedTime, int numNodes)
{
    SG_UNUSED(numNodes);
    if (elapsedTime > m_abortTime)
    {
        RlDebug(RlSetup::VOCAL) << "Aborting mid-ply\n";
        return true;
    }
    return false;
}

bool RlIterSearchControl::StartNextIteration(int depth, 
    double elapsedTime, int numNodes)
{
    SG_UNUSED(numNodes);

    // Next iteration time will be longer by ~sqrt of branching factor
    RlFloat estimatedTime = elapsedTime *
        pow(m_board.TotalNumEmpty(), m_branchPower);
    bool continuesearch = depth <= m_minDepth 
        || estimatedTime < m_fullTime - elapsedTime;

    if (!continuesearch)
        RlDebug(RlSetup::VOCAL) << "Stopping search at depth " 
            << depth - 1 << ": " 
            << elapsedTime << " elapsed, "
            << estimatedTime << " estimated, "
            << m_fullTime - elapsedTime << " remaining\n";

    return continuesearch;
}

void RlIterSearchControl::SetMinDepth(int mindepth)
{
    m_minDepth = mindepth;
}

void RlIterSearchControl::SetMaxTime(double fulltime, double aborttime)
{
    m_fullTime = fulltime;
    m_abortTime = aborttime;
}

void RlIterSearchControl::SetBranchPower(double branchpower)
{
    m_branchPower = branchpower;
}

//----------------------------------------------------------------------------
