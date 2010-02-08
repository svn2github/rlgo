//-----------------------------------------------------------------------------
/* @file RlAlphaBeta.cpp 
*/
//-----------------------------------------------------------------------------

#include "SgSystem.h"
#include "RlAlphaBeta.h"
#include "RlEvaluator.h"
#include "RlMoveFilter.h"
#include "RlSetup.h"

using namespace GoBoardUtil;
using namespace RlMoveUtil;
using namespace std;

//-----------------------------------------------------------------------------

IMPLEMENT_OBJECT(RlAlphaBeta);

RlAlphaBeta::RlAlphaBeta(GoBoard& board, RlEvaluator* evaluator)
:   RlAutoObject(board),
    m_evaluator(evaluator),
    m_maxDepth(32),
    m_maxTime(10),
    m_maxPredictedTime(RlInfinity),
    m_hashSize(0x100000),
    m_sortDepth(2),
    m_historyHeuristic(true),
    m_killerHeuristic(true),
    m_numKillers(2),
    m_opponentKillers(2),
    m_cutMargin(10),
    m_maxReductions(1),
    m_maxExtensions(0),
    m_ensureParity(true),
    m_pvs(true),
    m_branchPower(0.25)
{
}

RlAlphaBeta::~RlAlphaBeta()
{
    delete [] m_hashTable;
}

void RlAlphaBeta::LoadSettings(istream& settings)
{
    settings >> RlSetting<RlEvaluator*>("Evaluator", m_evaluator);
    settings >> RlSetting<int>("MaxDepth", m_maxDepth);
    settings >> RlSetting<double>("MaxTime", m_maxTime);
    settings >> RlSetting<double>("MaxPredictedTime", m_maxPredictedTime);
    settings >> RlSetting<int>("HashSize", m_hashSize);
    settings >> RlSetting<int>("SortDepth", m_sortDepth);    
    settings >> RlSetting<bool>("HistoryHeuristic", m_historyHeuristic);
    settings >> RlSetting<bool>("KillerHeuristic", m_killerHeuristic);
    settings >> RlSetting<int>("NumKillers", m_numKillers);
    settings >> RlSetting<int>("OpponentKillers", m_opponentKillers);
    settings >> RlSetting<int>("CutMargin", m_cutMargin);
    settings >> RlSetting<int>("MaxReductions", m_maxReductions);
    settings >> RlSetting<int>("MaxExtensions", m_maxExtensions);
    settings >> RlSetting<bool>("EnsureParity", m_ensureParity);
    settings >> RlSetting<bool>("PVS", m_pvs);
    settings >> RlSetting<RlFloat>("BranchPower", m_branchPower);
}

void RlAlphaBeta::Initialise()
{
    m_evaluator->EnsureInitialised();
    m_hashTable = new HashEntry[m_hashSize];
    SG_ASSERT(m_maxExtensions < 256);
    Clear();
}

int RlAlphaBeta::Search(vector<SgMove>& pv)
{
    m_elapsedTime = 0;

    for (m_iterationDepth = 1; ; ++m_iterationDepth)
    {
        m_timer.Start();
        ClearStatistics();
        int eval = AlphaBeta(m_iterationDepth, 
            -RL_SEARCH_MAX, +RL_SEARCH_MAX, 0, 0, 0);
        PrincipalVariation(pv);
        OutputStatistics(eval, pv, RlDebug(RlSetup::VOCAL));
        if (CheckAbort())
            return m_board.ToPlay() == SG_BLACK ? eval : -eval;
    }
}

int RlAlphaBeta::AlphaBeta(int depth, int alpha, int beta, 
    int numReductions, int numExtensions, int child)
{
    int eval;
    bool lower = false;
    SgMove bestMove = SG_NULLMOVE;
    bool parity = (numExtensions % 2) == 0;
    m_stats[depth][STAT_NODES]++;

    // Make sure that all lines are evaluated to same parity 
    // if using depth extensions (all odd or all even depths)
    if (m_ensureParity && depth == 0 && !parity)
    {
        m_stats[depth][STAT_PARITY]++;
        return AlphaBeta(1, alpha, beta, 0, 0, child);
    }

    // Evaluate leaf node
    if (depth == 0 || TwoPasses(m_board))
        return Evaluate(depth);

    // Check hash table for existing value
    if (ProbeHash(depth, alpha, beta, eval, bestMove))
        return eval;

    // Soft margin depth reductions
    if (depth >= 2 && numReductions < m_maxReductions && beta < +RL_SEARCH_MAX)
    {
        m_stats[depth][STAT_REDUCTIONS]++;
        eval = AlphaBeta(depth - 2, beta + m_cutMargin - 1, beta + m_cutMargin, 
            numReductions + 1, numExtensions, child); // reduce recursively
        if (eval >= beta + m_cutMargin)
            return BetaCut(depth, beta, ProbeBestMove(), STAT_REDCUTS);
    }

    // Principal variation search
    if (m_pvs && child > 0 && beta < RL_SEARCH_MAX && beta - alpha > 1)
    {
        m_stats[depth][STAT_PVS]++;
        eval = AlphaBeta(depth, beta - 1, beta, 
            numReductions, numExtensions, child);
        if (eval >= beta)
            return BetaCut(depth, beta, ProbeBestMove(), STAT_PVSCUTS);
    }
    
    if (bestMove == SG_NULLMOVE)
        bestMove = ProbeBestMove();
        
    // Move generation
    vector<SgMove> moves, extensions;
    GenerateMoves(moves, depth, bestMove);
    if (numExtensions < m_maxExtensions)
        GenerateExtensions(extensions);

    // Main loop
    m_stats[depth][STAT_FULLWIDTH]++;
    child = 0;
    for (vector<SgMove>::reverse_iterator i_moves = moves.rbegin(); 
        i_moves != moves.rend(); ++i_moves)
    {
        SgMove move = *i_moves;
        if (ConsiderMove(move))
        {
            bool isExtension = Contains(extensions, move);
            if (isExtension)
                m_stats[depth][STAT_EXTENSIONS]++;

            Play(move);
            eval = -AlphaBeta(depth - 1 + isExtension, 
                -beta, -alpha, 
                0, numExtensions + isExtension, child);
            Undo();
            m_stats[depth][STAT_CHILDREN]++;
            
            if (eval >= beta)
                return BetaCut(depth, beta, move, STAT_BETACUTS);

            if (eval > alpha)
            {
                alpha = eval;
                bestMove = move;
                lower = true;
            }
            child++;
        }
    }

    StoreHash(depth, bestMove, alpha, lower, true);
    MarkKiller(depth, bestMove);
    m_stats[depth][STAT_NOCUTS]++;
    return alpha;
}

int RlAlphaBeta::BetaCut(int depth, int beta, SgMove bestMove, int stat)
{
    m_stats[depth][stat]++;
    StoreHash(depth, bestMove, beta, true, false);
    MarkKiller(depth, bestMove);
    return beta;
}

void RlAlphaBeta::SortMoves(vector<SgMove>& moves)
{
    RlMoveSorter sorter;
    for (GoBoard::Iterator i_board(m_board); i_board; ++i_board)
    {
        SgPoint move = *i_board;
        int h = ConsiderMove(move) ? m_history[move] : -1;
        sorter.AddMove(move, h);
    }
    sorter.SortMoves(m_board.ToPlay()); // best moves will be last

    moves.clear();
    moves.push_back(SG_PASS);
    for (int i = 0; i < sorter.GetNumMoves(); ++i)
        moves.push_back(sorter.GetMove(i));
}

void RlAlphaBeta::GenerateMoves(vector<SgMove>& moves, 
    int depth, SgMove bestMove)
{
    if (depth >= m_sortDepth)
        SortMoves(m_sortedMoves);

    // Include all points, ordered by root evaluation
    // Legality will only be evaluated when moves are about to be played
    moves = m_sortedMoves;

    // Promote capturing and capture defending moves
    for (GoBlockIterator i_block(m_board); i_block; ++i_block)
    {
        SgPoint pt = *i_block;
        if (m_board.InAtari(pt))
            Promote(moves, m_board.TheLiberty(pt));
    }

    // Promote killer moves
    if (m_killerHeuristic)
        PromoteKillers(moves, depth);

    // Promote best move from previous iteration
    Promote(moves, bestMove);
}

void RlAlphaBeta::GenerateExtensions(vector<SgMove>& extensions)
{
    // Extend ladders from last move only
    SgMove lastmove = m_board.GetLastMove();
    if (lastmove == SG_NULLMOVE || lastmove == SG_PASS)
        return;

    // Attack
    SgPoint anchor = m_board.Anchor(lastmove);
    if (m_board.NumLiberties(anchor) <= 2)
        for (GoBoard::LibertyIterator i_lib(m_board, anchor); i_lib; ++i_lib)
            extensions.push_back(*i_lib);
    
    // Defend (and also capture when opponent plays himself into atari)
    for (SgNb4Iterator i_nb(lastmove); i_nb; ++i_nb)
        if (m_board.Occupied(*i_nb) && m_board.InAtari(*i_nb))
            extensions.push_back(m_board.TheLiberty(*i_nb));
}

void RlAlphaBeta::PromoteKillers(vector<SgMove>& moves, int depth)
{
    for (int n = m_opponentKillers - 1; n >= 0; --n)
        Promote(moves, m_killer[depth + 1].GetKiller(n));
    for (int n = m_numKillers - 1; n >= 0; --n)
        Promote(moves, m_killer[depth].GetKiller(n));
}

inline void RlAlphaBeta::Promote(vector<SgMove>& moves, SgMove move)
{
    if (move == SG_NULLMOVE)
        return;
        
    vector<SgMove>::iterator pos = find(moves.begin(), moves.end(), move);
    if (pos != moves.end())
        moves.erase(pos);
    moves.push_back(move);
}

inline RlAlphaBeta::HashEntry& RlAlphaBeta::LookupHash()
{
    RlHash hashcode = m_board.GetHashCodeInclToPlay();
    int index = hashcode.Hash(m_hashSize);
    return m_hashTable[index];
}

inline const RlAlphaBeta::HashEntry& RlAlphaBeta::LookupHash() const
{
    RlHash hashcode = m_board.GetHashCodeInclToPlay();
    int index = hashcode.Hash(m_hashSize);
    return m_hashTable[index];
}

inline SgMove RlAlphaBeta::ProbeBestMove()
{
    HashEntry& entry = LookupHash();
    if (entry.m_hash == m_board.GetHashCodeInclToPlay())
        return entry.m_bestMove;
    else
        return SG_NULLMOVE;
}

inline bool RlAlphaBeta::ProbeHash(int depth, int& alpha, int& beta, 
    int& eval, SgMove& bestMove)
{
    HashEntry& entry = LookupHash();
    if (entry.m_hash == m_board.GetHashCodeInclToPlay())
    {
        m_stats[depth][STAT_HASHHITS]++;
        bestMove = entry.m_bestMove;
        if (entry.m_depth >= depth)
        {
            if (entry.m_upperBound <= alpha)
            {
                eval = alpha;
                m_stats[depth][STAT_HASHCUTS]++;
                return true;
            }
            if (entry.m_lowerBound >= beta)
            {
                eval = beta;
                m_stats[depth][STAT_HASHCUTS]++;
                return true;
            }
            if (entry.m_lowerBound > alpha)
                alpha = entry.m_lowerBound;
            if (entry.m_upperBound < beta)
                beta = entry.m_upperBound;
        }
    }
    else 
    {
        if (entry.m_hash == RlHash(0))
            m_stats[depth][STAT_HASHMISSES]++;
        else
            m_stats[depth][STAT_COLLISIONS]++;
    }
    return false;
}    

inline void RlAlphaBeta::StoreHash(int depth, SgMove move, int eval,
    bool lower, bool upper)
{
    HashEntry& entry = LookupHash();

    // Replace entry
    if (depth > entry.m_depth)
    {
        entry.m_hash = m_board.GetHashCodeInclToPlay();
        entry.m_depth = depth;
        entry.m_bestMove = move;
        entry.m_lowerBound = lower ? eval : -RL_SEARCH_MAX;
        entry.m_upperBound = upper ? eval : +RL_SEARCH_MAX;
    }
    
    // Update bounds
    else if (depth == entry.m_depth && 
        entry.m_hash == m_board.GetHashCodeInclToPlay())
    {    
        if (lower)
        {
            SG_ASSERT(eval >= entry.m_lowerBound);
            entry.m_lowerBound = eval;
            entry.m_bestMove = move;
        }
        if (upper)
        {
            SG_ASSERT(eval <= entry.m_upperBound);
            entry.m_upperBound = eval;
            entry.m_bestMove = move;
        }
    }
}

void RlAlphaBeta::Clear()
{
    if (m_killerHeuristic)
        for (int depth = 0; depth < RL_MAX_DEPTH; depth++)
            m_killer[depth].Init(m_numKillers);

    if (m_historyHeuristic)
        for (int i = 0; i < RL_MAX_MOVES; ++i)
            m_history[i] = 0;

    RlDebug(RlSetup::VOCAL) << "Clearing hash table...";
    for (int i = 0; i < m_hashSize; ++i)
    {
        HashEntry& entry = m_hashTable[i];
        entry.m_hash = RlHash(0);
        entry.m_depth = -RL_MAX_DEPTH;
        entry.m_bestMove = SG_NULLMOVE;
        entry.m_lowerBound = -RL_SEARCH_MAX;
        entry.m_upperBound = +RL_SEARCH_MAX;
    }    
    RlDebug(RlSetup::VOCAL) << " done\n";
};

inline int RlAlphaBeta::Evaluate(int depth)
{
    RlFloat feval = Truncate(m_evaluator->Eval());
    int eval = (int) (feval * 100);
    if (m_board.ToPlay() == SG_WHITE)
        eval = -eval;

    m_stats[depth][STAT_EVALUATIONS]++;
    return eval;
}

inline void RlAlphaBeta::Play(SgMove move)
{
    m_evaluator->PlayExecute(move, m_board.ToPlay(), false);
    m_variation.push_back(move);
}

inline void RlAlphaBeta::Undo()
{
    m_evaluator->TakeBackUndo(false);
    m_variation.pop_back();
}

inline bool RlAlphaBeta::ConsiderMove(SgMove move)
{
    // Check for illegal or eye filling moves
    return m_evaluator->GetMoveFilter()->ConsiderMove(
        move, m_board.ToPlay(), true);
}

inline void RlAlphaBeta::MarkKiller(int depth, SgMove move)
{
    if (m_historyHeuristic && move != SG_NULLMOVE)
        m_history[move] += (1 << depth);
    if (m_killerHeuristic && move != SG_NULLMOVE)
        m_killer[depth].MarkKiller(move);
}

bool RlAlphaBeta::CheckAbort()
{
    if (m_iterationDepth >= m_maxDepth)
        return true;
        
    double searchTime = m_timer.GetTime();
    m_elapsedTime += searchTime;
    if (m_elapsedTime > m_maxTime)
    {
        RlDebug(RlSetup::VOCAL) << "Elapsed time exceeded maximum: " 
            << m_elapsedTime << " > " << m_maxTime << "\n";
        return true;
    }
 
    double estimatedTime = searchTime * pow(
        m_board.TotalNumEmpty(), m_branchPower);
    if (m_elapsedTime + estimatedTime > m_maxPredictedTime)
    {
        RlDebug(RlSetup::VOCAL) << "Estimated time exceeded maximum: " 
            << m_elapsedTime << " + " << estimatedTime 
            << " > " << m_maxPredictedTime << "\n";
        return true;
    }
        
    return false;
}

void RlAlphaBeta::PrincipalVariation(vector<SgMove>& pv)
{
    // Walk the transposition table to retrieve the principal variation
    HashEntry entry = LookupHash();
    while (entry.m_hash == m_board.GetHashCodeInclToPlay()
        && entry.m_bestMove != SG_NULLMOVE
        && entry.m_lowerBound == entry.m_upperBound
        && !TwoPasses(m_board))
    {
        Play(entry.m_bestMove);
        entry = LookupHash();
    }

    pv = m_variation;
    for (int i = 0; i < ssize(pv); ++i)
        Undo();
}

void RlAlphaBeta::OutputStatistics(int eval, 
    const vector<SgMove>& pv, ostream& ostr)
{    
    int bval = m_board.ToPlay() == SG_BLACK ? eval : -eval;
    int pwin = Logistic((RlFloat) bval / 100);
    
    ostr << "\nDepth: " << m_iterationDepth << endl;
    ostr << "Principal variation: " << WriteMoveSequence(pv) << endl;
    ostr << "Evaluation: " << bval 
        << " (" << pwin * 100 << "% winning for Black)" << endl;
    ostr << "Time used: " << m_timer.GetTime() << endl;

    ostr << setw(16) << "Depth: ";
    for (int d = 0; d <= m_iterationDepth; ++d)
        ostr << setw(12) << d;
    ostr << endl;

    OutputStatistic("Nodes", STAT_NODES, ostr);
    OutputStatistic("Evaluations", STAT_EVALUATIONS, ostr);
    OutputStatistic("Beta cuts", STAT_BETACUTS, ostr);
    OutputStatistic("No cuts", STAT_NOCUTS, ostr);
    OutputStatistic("Hash hits", STAT_HASHHITS, ostr);
    OutputStatistic("Hash misses", STAT_HASHMISSES, ostr);
    OutputStatistic("Collisions", STAT_COLLISIONS, ostr);
    OutputStatistic("Hash cuts", STAT_HASHCUTS, ostr);
    if (m_maxReductions > 0)
    {
        OutputStatistic("Reductions", STAT_REDUCTIONS, ostr);
        OutputStatistic("RedCuts", STAT_REDCUTS, ostr);
    }
    if (m_pvs)
    {
        OutputStatistic("PVS", STAT_PVS, ostr);
        OutputStatistic("PVSCuts", STAT_PVSCUTS, ostr);
    }
    if (m_maxExtensions > 0)
    {
        OutputStatistic("Extensions", STAT_EXTENSIONS, ostr);
        OutputStatistic("Parity", STAT_PARITY, ostr);
    }

    ostr << setw(16) << "Width: ";
    for (int d = 0; d <= m_iterationDepth; ++d)
        ostr << setw(12) << (RlFloat) m_stats[d][STAT_CHILDREN]
            / (RlFloat) m_stats[d][STAT_FULLWIDTH];
    ostr << endl;
}

void RlAlphaBeta::OutputStatistic(const std::string& name, int stat,
    ostream& ostr)
{
    ostr << setw(16) << (name + ": ");
    for (int d = 0; d <= m_iterationDepth; ++d)
        ostr << setw(12) << m_stats[d][stat];
    ostr << endl;
}

void RlAlphaBeta::ClearStatistics()
{
    for (int d = 0; d < m_iterationDepth; ++d)
        for (int stat = 0; stat < NUM_STATS; ++stat)
            m_stats[d][stat] = 0;
}

//-----------------------------------------------------------------------------
