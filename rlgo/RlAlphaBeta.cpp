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

//@todo: move into utils
inline SgPoint Seed(const GoBoard& bd, SgPoint pt)
{
    GoBoard::StoneIterator i_stone(bd, bd.Anchor(pt));
    return *i_stone;
}

vector<SgMove> var;

//-----------------------------------------------------------------------------

IMPLEMENT_OBJECT(RlAlphaBeta);

RlAlphaBeta::RlAlphaBeta(GoBoard& board, RlEvaluator* evaluator)
:   RlAutoObject(board),
    m_evaluator(evaluator),
    m_maxDepth(32),
    m_maxTime(10),
    m_maxPredictedTime(RlInfinity),
    m_hashSize(0x100000),
    m_quiescence(true),
    m_ensureParity(true),
    m_readLadders(true),
    m_maxQDepth(12),
    m_maxLadderDepth(9),
    m_nullMovePruning(false),
    m_nullMoveDepth(2),
    m_killerHeuristic(true),
    m_numKillers(2),
    m_opponentKillers(2),
    m_grain(0.01),
    m_estimateTenukiValue(true),
    m_tenukiValue(1.0),
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
    settings >> RlSetting<bool>("Quiescence", m_quiescence);
    settings >> RlSetting<bool>("EnsureParity", m_ensureParity);
    settings >> RlSetting<bool>("ReadLadders", m_readLadders);
    settings >> RlSetting<int>("MaxQDepth", m_maxQDepth);
    settings >> RlSetting<int>("MaxLadderDepth", m_maxLadderDepth);
    settings >> RlSetting<bool>("NullMovePruning", m_nullMovePruning);
    settings >> RlSetting<int>("NullMoveDepth", m_nullMoveDepth);
    settings >> RlSetting<bool>("KillerHeuristic", m_killerHeuristic);
    settings >> RlSetting<int>("NumKillers", m_numKillers);
    settings >> RlSetting<int>("OpponentKillers", m_opponentKillers);
    settings >> RlSetting<RlFloat>("Grain", m_grain);
    settings >> RlSetting<bool>("EstimateTenukiValue", m_estimateTenukiValue);
    settings >> RlSetting<RlFloat>("TenukiValue", m_tenukiValue);
    settings >> RlSetting<RlFloat>("BranchPower", m_branchPower);
}

void RlAlphaBeta::Initialise()
{
    m_evaluator->EnsureInitialised();
    m_hashTable = new HashEntry[m_hashSize];
    Clear();
}

void RlAlphaBeta::GenerateMoves(vector<SgMove>& moves, 
    int depth, SgMove bestMove)
{
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

RlFloat RlAlphaBeta::Search(vector<SgMove>& pv)
{
    StaticMoveOrder();
    if (m_estimateTenukiValue)
        m_tenukiValue = EstimateTenukiValue();

    m_stats.m_elapsedTime = 0;
    for (m_iterationDepth = 1; ; ++m_iterationDepth)
    {
        m_stats.Clear();
        RlFloat eval = AlphaBeta(m_iterationDepth, -RlInfinity, +RlInfinity);
        PrincipalVariation(pv);
        m_stats.Output(m_iterationDepth, eval, m_board.ToPlay(), pv,
            RlDebug(RlSetup::VOCAL));
        if (CheckAbort())
            return m_board.ToPlay() == SG_BLACK ? eval : -eval;
    }
}

RlFloat RlAlphaBeta::AlphaBeta(int depth, RlFloat alpha, RlFloat beta,
    bool lastNull)
{
    RlFloat eval;
    SgMove bestMove = SG_NULLMOVE;
    int hashFlags = HashEntry::RL_ALPHA;
    m_stats.m_nodeCount++;

    // Check hash table for existing value
    if (ProbeHash(depth, alpha, beta, eval, bestMove))
    {
        m_stats.m_hashCount++;
        return eval;
    }

    // Evaluate leaf node
    if (depth == 0)
    {
        m_stats.m_leafCount++;
        if (m_quiescence)
        {
            eval = QuiesceRoot(alpha, beta);
            StoreHash(depth, SG_NULLMOVE, eval, HashEntry::RL_EXACT);
            return eval;
        }
        else
        {
            return Evaluate();
        }
    }
    
    // Scout
    if (!lastNull && beta < RlInfinity)
    {
        eval = AlphaBeta(depth, beta - m_grain, beta, true);
        if (eval >= beta)
            return beta;
    }

    // Null move pruning
    if (NullMovePrune(depth, beta, lastNull))
    {
        m_stats.m_nullCount++;
        return beta;
    }

    // Move generation
    m_stats.m_interiorCount++;
    m_stats.m_interiorDepth += (m_iterationDepth - depth);
    vector<SgMove> moves;
    GenerateMoves(moves, depth, bestMove);
    if (moves.empty()) //@todo: consider pass move
        return EvaluateWithParity(depth);
    
    // Main loop
    for (vector<SgMove>::reverse_iterator i_moves = moves.rbegin(); 
        i_moves != moves.rend(); ++i_moves)
    {
        SgMove move = *i_moves;
        if (ConsiderMove(move))
        {
            Play(move);
            RlFloat eval = -AlphaBeta(depth - 1, -beta, -alpha);
            Undo();
            m_stats.m_interiorWidth++;
            
            if (eval >= beta)
            {
                StoreHash(depth, move, beta, HashEntry::RL_BETA);
                MarkKiller(depth, move);
                return beta;
            }

            if (eval > alpha)
            {
                alpha = eval;
                bestMove = move;
                hashFlags = HashEntry::RL_EXACT;
            }
        }
    }

    StoreHash(depth, bestMove, alpha, hashFlags);
    MarkKiller(depth, bestMove);
    return alpha;
}

inline void RlAlphaBeta::GenerateCaptureMoves(vector<SgMove>& moves)
{
    // Defend own blocks
    for (GoBlockIterator i_block(m_board); i_block; ++i_block)
    {
        SgPoint pt = *i_block;
        if (m_board.InAtari(pt)
            && m_board.GetColor(pt) == m_board.ToPlay())
        {
            SgPoint lib = m_board.TheLiberty(pt);
            moves.push_back(lib);
        }
    }

    // Capture opponent blocks
    for (GoBlockIterator i_block(m_board); i_block; ++i_block)
    {
        SgPoint pt = *i_block;
        if (m_board.InAtari(pt)
            && m_board.GetColor(pt) == m_board.Opponent())
        {
            SgPoint lib = m_board.TheLiberty(pt);
            moves.push_back(lib);
        }
    }
}

inline void RlAlphaBeta::GenerateLadderMoves(vector<SgMove>& moves)
{
    for (GoBlockIterator i_block(m_board); i_block; ++i_block)
    {
        SgPoint pt = *i_block;
        if (m_board.NumLiberties(pt) == 2 &&
            m_ladderMarker.Contains(Seed(m_board, pt)))
        {
            for (GoBoard::LibertyIterator i_lib(m_board, pt); i_lib; ++i_lib)
            {
                SgPoint lib = *i_lib;
                moves.push_back(lib);
            }
        }
    }
}

RlFloat RlAlphaBeta::QuiesceRoot(RlFloat alpha, RlFloat beta)
{
    m_koMarker.Clear();
    m_numTenuki[SG_BLACK] = 0;
    m_numTenuki[SG_WHITE] = 0;
    if (m_readLadders)
    {
        m_ladderMarker.Clear();
        for (GoBlockIterator i_block(m_board); i_block; ++i_block)
        {
            SgPoint pt = *i_block;
            if (m_board.NumLiberties(pt) <= 2)
                m_ladderMarker.Include(Seed(m_board, pt));
        }
    }
    return Quiesce(0, alpha, beta);
}

RlFloat RlAlphaBeta::Quiesce(int depth, RlFloat alpha, RlFloat beta)
{
    m_stats.m_qCount++;
    m_stats.m_qDepth += depth;

    // Use leaf evaluation at max depth or after two successive tenukis
    if (depth <= -m_maxQDepth || TwoPasses(m_board))
        return EvaluateWithParity(depth);

    // Generate capture/defend 
    vector<SgMove> moves;
    GenerateCaptureMoves(moves);
    for (vector<SgMove>::reverse_iterator i_moves = moves.rbegin();
        i_moves != moves.rend(); ++i_moves)
    {
        SgMove move = *i_moves;
        
        if (m_board.IsLegal(move)
            && !m_koMarker.Contains(move))
        {
            Play(move);
            m_koMarker.Include(move);
            RlFloat eval = -Quiesce(depth - 1, -beta, -alpha);
            m_koMarker.Exclude(move);
            Undo();
            m_stats.m_qWidth++;

            if (eval >= beta)
                return beta;
            if (eval > alpha)
                alpha = eval;
        }
    }

    // Generate tenuki
    SgBlackWhite toplay = m_board.ToPlay();
    m_numTenuki[toplay]++;
    Play(SG_PASS);
    RlFloat eval = -Quiesce(depth - 1, 
        -beta + m_tenukiValue, -alpha + m_tenukiValue) 
        + m_tenukiValue;
    Undo();
    m_numTenuki[toplay]--;

    if (eval >= beta)
        return beta;
    if (eval > alpha)
        alpha = eval;            

    // Generate ladder moves
    if (m_readLadders && 
        depth >= -m_maxLadderDepth && 
        m_numTenuki[toplay] == 0)
    {
        moves.clear();
        GenerateLadderMoves(moves);
        for (vector<SgMove>::reverse_iterator i_moves = moves.rbegin();
            i_moves != moves.rend(); ++i_moves)
        {
            SgMove move = *i_moves;
            
            if (m_board.IsLegal(move)
                && !m_koMarker.Contains(move))
            {
                Play(move);
                m_koMarker.Include(move);
                RlFloat eval = -Quiesce(depth - 1, -beta, -alpha);
                m_koMarker.Exclude(move);
                Undo();
                m_stats.m_qWidth++;

                if (eval >= beta)
                    return beta;
                if (eval > alpha)
                    alpha = eval;            
            }        
        }
    }
    
    return alpha;
}

bool RlAlphaBeta::NullMovePrune(int depth, RlFloat beta, bool lastNull)
{
    if (!m_nullMovePruning || lastNull || depth < m_nullMoveDepth)
        return false;
    if (beta >= RlInfinity)
        return false;
    
    Play(SG_PASS);
    RlFloat eval = -AlphaBeta(depth - m_nullMoveDepth, -beta, -beta + m_grain,
        true); // remember null move
    Undo();
    return eval >= beta;
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

inline bool RlAlphaBeta::ProbeHash(int depth, RlFloat alpha, RlFloat beta, 
    RlFloat& eval, SgMove& bestMove)
{
    HashEntry& entry = LookupHash();
    if (entry.m_hash == m_board.GetHashCodeInclToPlay())
    {
        if (entry.m_depth >= depth)
        {
            SG_ASSERT(entry.m_flags != HashEntry::RL_INVALID);
            if (entry.m_flags == HashEntry::RL_EXACT)
            {
                eval = entry.m_eval;
                return true;
            }
            if (entry.m_flags == HashEntry::RL_ALPHA && entry.m_eval <= alpha)
            {
                eval = alpha;
                return true;
            }
            if (entry.m_flags == HashEntry::RL_BETA && entry.m_eval >= beta)
            {
                eval = beta;
                return true;
            }
        }
        bestMove = entry.m_bestMove;
    }
    return false;
}    

inline void RlAlphaBeta::StoreHash(
    int depth, SgMove move, RlFloat eval, int hashFlags)
{
    HashEntry& entry = LookupHash();

    if (depth >= entry.m_depth)
    {
        entry.m_hash = m_board.GetHashCodeInclToPlay();
        entry.m_depth = depth;
        entry.m_bestMove = move;
        entry.m_eval = eval;
        entry.m_flags = hashFlags;
    }
}

void RlAlphaBeta::Clear()
{
    if (m_killerHeuristic)
        for (int depth = 0; depth < RL_MAX_DEPTH; depth++)
            m_killer[depth].Init(m_numKillers);

    for (int i = 0; i < m_hashSize; ++i)
    {
        HashEntry& entry = m_hashTable[i];
        entry.m_hash = RlHash(0);
        entry.m_depth = -RL_MAX_DEPTH; // below any conceivable q-search
        entry.m_bestMove = SG_NULLMOVE;
        entry.m_eval = 0;
        entry.m_flags = HashEntry::RL_INVALID;
    }    
};

inline RlFloat RlAlphaBeta::Evaluate()
{
    m_stats.m_evalCount++;
    RlFloat eval = m_evaluator->Eval();
    //for (int i = 0; i < ssize(m_variation); ++i)
    //    cout << SgWritePoint(m_variation[i]) << " ";
    //cout << eval << endl;
    if (m_board.ToPlay() == SG_WHITE)
        eval = -eval;
    return eval;
}

inline RlFloat RlAlphaBeta::EvaluateWithParity(int depth)
{
    // Take account of the value of having the current move
    RlFloat eval = Evaluate();
    if (m_ensureParity && (depth + RL_MAX_DEPTH) % 2 == 1)
        eval += m_tenukiValue;

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
    if (m_killerHeuristic && move != SG_NULLMOVE)
        m_killer[depth].MarkKiller(move);
}

bool RlAlphaBeta::CheckAbort()
{
    if (m_iterationDepth >= m_maxDepth)
        return true;
        
    double searchTime = m_stats.m_timer.GetTime();
    m_stats.m_elapsedTime += searchTime;
    if (m_stats.m_elapsedTime > m_maxTime)
    {
        RlDebug(RlSetup::VOCAL) << "Elapsed time exceeded maximum: " 
            << m_stats.m_elapsedTime << " > " << m_maxTime << "\n";
        return true;
    }
 
    double estimatedTime = searchTime * pow(
        m_board.TotalNumEmpty(), m_branchPower);
    if (m_stats.m_elapsedTime + estimatedTime > m_maxPredictedTime)
    {
        RlDebug(RlSetup::VOCAL) << "Estimated time exceeded maximum: " 
            << m_stats.m_elapsedTime << " + " << estimatedTime 
            << " > " << m_maxPredictedTime << "\n";
        return true;
    }
        
    return false;
}

void RlAlphaBeta::PrincipalVariation(vector<SgMove>& pv) const
{
    // Walk the transposition table to retrieve the principal variation
    pv.clear();
    HashEntry entry = LookupHash();
    while (entry.m_hash == m_board.GetHashCodeInclToPlay()
        && entry.m_bestMove != SG_NULLMOVE
        && entry.m_flags == HashEntry::RL_EXACT)
    {
        pv.push_back(entry.m_bestMove);
        m_evaluator->PlayExecute(entry.m_bestMove, m_board.ToPlay(), false);
        entry = LookupHash();
    }

    for (int i = 0; i < ssize(pv); ++i)
        m_evaluator->TakeBackUndo(false);
}

void RlAlphaBeta::StaticMoveOrder()
{
    RlMoveSorter sorter;
    for (GoBoard::Iterator i_board(m_board); i_board; ++i_board)
    {
        SgPoint move = *i_board;
        RlFloat eval = -RlInfinity;
        if (ConsiderMove(move))
        {
            Play(move);
            eval = -Evaluate();
            Undo();
        }
        sorter.AddMove(move, eval);
    }
    sorter.SortMoves(m_board.ToPlay()); // best moves will be last

    m_sortedMoves.clear();
    for (int i = 0; i < sorter.GetNumMoves(); ++i)
        m_sortedMoves.push_back(sorter.GetMove(i));
}

RlFloat RlAlphaBeta::EstimateTenukiValue()
{
    RlFloat eval1 = Evaluate();
    RlFloat eval2 = -RlInfinity;

    vector<SgMove> qmoves;
    GenerateCaptureMoves(qmoves);
    if (m_readLadders)
        GenerateLadderMoves(qmoves);

    // Estimate tenuki value from best quiet move at root
    for (GoBoard::Iterator i_board(m_board); i_board; ++i_board)
    {
        SgMove move = *i_board;
        if (ConsiderMove(move) && 
            find(qmoves.begin(), qmoves.end(), move) == qmoves.end())
        {
            Play(move);
            RlFloat eval = -Evaluate();
            if (eval > eval2)
                eval2 = eval;
            Undo();
        }
    }
    
    RlFloat tenukiValue = eval2 - eval1;
    RlDebug(RlSetup::VOCAL) << "Tenuki value: " << tenukiValue << endl;
    return tenukiValue;
}

//-----------------------------------------------------------------------------

void RlSearchStatistics::Clear()
{
    m_evalCount = 0;
    m_nodeCount = 0;
    m_interiorCount = 0;
    m_leafCount = 0;
    m_qCount = 0;
    m_nullCount = 0;
    m_interiorDepth = 0;
    m_interiorWidth = 0;
    m_qDepth = 0;
    m_qWidth = 0;
    m_hashCount = 0;
    m_timer.Start();
}

void RlSearchStatistics::Output(int depth, RlFloat eval, SgBlackWhite toplay,
    const vector<SgMove>& pv, ostream& ostr)
{    
    RlFloat bval = toplay == SG_BLACK ? eval : -eval;
    RlFloat pwin = Logistic(eval);
    
    ostr << "\nDepth: " << depth << "\n";
    ostr << "Nodes: " << m_nodeCount << "\n";
    ostr << "\tHash hits: " << m_hashCount << "\n";
    ostr << "\tLeaf nodes: " << m_leafCount << "\n";
    ostr << "\tNull move pruned nodes: " << m_nullCount << "\n";
    ostr << "\tInterior nodes: " << m_interiorCount << "\n";
    if (m_interiorCount > 0)
    {
        ostr << "\t\tInterior depth: " << (RlFloat) m_interiorDepth / m_interiorCount << "\n";
        ostr << "\t\tInterior width: " << (RlFloat) m_interiorWidth / m_interiorCount << "\n";
    }
    ostr << "Quiescence nodes: " << m_qCount << "\n";
    if (m_qCount > 0)
    {
        ostr << "\tQuiescence depth: " << -(RlFloat) m_qDepth / m_qCount << "\n";
        ostr << "\tQuiescence width: " << (RlFloat) m_qWidth / m_qCount << "\n";
    }
    ostr << "Evaluations: " << m_evalCount << "\n";
    ostr << "Time used: " << m_timer.GetTime() << "\n";
    ostr << "Principal variation: " << WriteMoveSequence(pv) << "\n";
    ostr << "Evaluation: " << bval;
    ostr << " (" << pwin * 100 << "% winning for Black)\n";
}

//-----------------------------------------------------------------------------
/*

RlFloat RlAlphaBeta::MTDSearch(RlFloat f)
{
    RlFloat eval = f;
    RlFloat upperbound = +RlInfinity;
    RlFloat lowerbound = -RlInfinity;
    while (upperbound >= lowerbound + m_grain)
    {
        RlFloat beta = eval == lowerbound ? eval + m_grain : eval;
        eval = AlphaBeta(m_iterationDepth, beta - m_grain, beta);
        if (eval < beta)
            upperbound = eval;
        else
            lowerbound = eval;
    }
    return f;
}

*/