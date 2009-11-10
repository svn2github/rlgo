//-----------------------------------------------------------------------------
/* @file RlAlphaBeta.h 
    Alpha beta search engine */
//-----------------------------------------------------------------------------

#ifndef RL_ALPHABETA_H
#define RL_ALPHABETA_H

#include "RlUtils.h"
#include "SgTimer.h"

const int RL_MAX_KILLERS = 16;
const int RL_MAX_DEPTH = SG_MAX_MOVES * 2;

using namespace RlMathUtil;

//-----------------------------------------------------------------------------

class RlKiller
{
public:

    RlKiller();
    void Init(int numkillers);
    void MarkKiller(SgMove move);
    SgMove& GetKiller(int n);
    SgMove GetKiller(int n) const;
    
private:

    int m_numKillers;
    SgMove m_killerMoves[RL_MAX_KILLERS];
};

//-----------------------------------------------------------------------------

class RlSearchStatistics
{
public:

    void Clear();
    void Output(int depth, RlFloat eval, SgBlackWhite toplay, 
        const std::vector<SgMove>& pv, std::ostream& ostr);

    int m_evalCount;
    int m_nodeCount;
    int m_interiorCount;
    int m_leafCount;
    int m_qCount;
    int m_nullCount;
    int m_hashCount;
    int m_interiorDepth;
    int m_interiorWidth;
    int m_qDepth;
    int m_qWidth;
    
    /** Timer for current search depth */
    SgTimer m_timer;

    /** Time since search began */
    double m_elapsedTime;    
};

//-----------------------------------------------------------------------------

class RlAlphaBeta : public RlAutoObject
{
public:

    DECLARE_OBJECT(RlAlphaBeta);

    RlAlphaBeta(GoBoard& board, RlEvaluator* evaluator = 0);
    
    ~RlAlphaBeta();

    virtual void LoadSettings(std::istream& settings);
    virtual void Initialise();

    /** Iterative deepening alpha-beta search.
        Iterations continue until abort criterion is matched (depth or time) 
        Returns root value and principal variation. */
    RlFloat Search(std::vector<SgMove>& pv);

    /** Clear hash table and other search data */
    void Clear();
    
    void SetMaxDepth(int value) { m_maxDepth = value; }
    void SetMaxTime(double value) { m_maxTime = value; }
    void SetMaxPredictedTime(double value) { m_maxPredictedTime = value; }    
    void SetQuiescence(bool value) { m_quiescence = value; }
    void SetReadLadders(bool value) { m_readLadders = value; }
    void SetMaxQDepth(int value) { m_maxQDepth = value; }
    void SetMaxLadderDepth(int value) { m_maxLadderDepth = value; }
    void SetEnsureParity(bool value) { m_ensureParity = value; }
    void SetNullMovePruning(bool value) { m_nullMovePruning = value; }
    void SetNullMoveDepth(int value) { m_nullMoveDepth = value; }
    void SetEstimateTenukiValue(bool value) { m_estimateTenukiValue = value; }
    void SetTenukiValue(RlFloat value) { m_tenukiValue = value; }
    
private:

    struct HashEntry
    {
        enum
        {
            RL_EXACT = 0,
            RL_ALPHA = 1,
            RL_BETA = 2,
            RL_INVALID = 3
        };
        
        RlHash m_hash;
        short m_depth;
        short m_flags;
        RlFloat m_eval;
        SgMove m_bestMove;
        // 24 bytes per hash entry
    };
    
    void PrincipalVariation(std::vector<SgMove>& pv) const;
    void StaticMoveOrder();
    RlFloat EstimateTenukiValue();
    void GenerateMoves(std::vector<SgMove>& moves, int depth, SgMove bestMove);
    RlFloat AlphaBetaSearch();
    RlFloat AlphaBeta(int depth, RlFloat alpha, RlFloat beta, 
        bool lastNull = false);
    RlFloat Evaluate();
    RlFloat EvaluateWithParity(int depth);
    void GenerateCaptureMoves(std::vector<SgMove>& moves);
    void GenerateLadderMoves(std::vector<SgMove>& moves);
    RlFloat QuiesceRoot(RlFloat alpha, RlFloat beta);
    RlFloat Quiesce(int depth, RlFloat alpha, RlFloat beta);

    bool NullMovePrune(int depth, RlFloat beta, bool lastNull);
    void PromoteKillers(std::vector<SgMove>& moves, int depth);
    void Promote(std::vector<SgMove>& moves, SgMove move);
    HashEntry& LookupHash();
    const HashEntry& LookupHash() const;
    bool ProbeHash(int depth, RlFloat alpha, RlFloat beta, 
        RlFloat& eval, SgMove& bestMove);
    void StoreHash(int depth, SgMove move, RlFloat eval, int hashFlags);
    void Play(SgMove move);
    void Undo();
    bool ConsiderMove(SgMove move);
    void MarkKiller(int depth, SgMove move);
    bool CheckAbort();
    void ClearStatistics();
    void PrintStatistics(RlFloat eval);

    RlEvaluator* m_evaluator;

    /** Abort search if this depth is exceeded */
    int m_maxDepth;
    
    /** Abort search if las this time is exceeded after last iteration */
    double m_maxTime;

    /** Abort search if next iteration is expected to exceed this time */
    double m_maxPredictedTime;
    
    /** Number of entries in the hash table */
    int m_hashSize;
    
    /** Whether to use quiescence search */
    bool m_quiescence;
    
    /** Whether to ensure that all branches are evaluated to same parity
        (same colour to play at leaf evaluation) */
    bool m_ensureParity;
    
    /** Whether to read ladders during quiescence search */
    bool m_readLadders;

    /** Maximum depth for quiescence search */
    int m_maxQDepth;

    /** Maximum depth for reading ladders in quiescence search */
    int m_maxLadderDepth;
    
    /** Whether to use null move pruning */
    bool m_nullMovePruning;
    
    /** Relative depth of null move search */
    int m_nullMoveDepth;
    
    /** Whether to use killer heuristic */
    bool m_killerHeuristic;
    
    /** Number of killer moves to store and use */
    int m_numKillers;
    
    /** Number of opponent killer moves to use (<= m_killer) */
    int m_opponentKillers;
    
    /** Minimum grain of discrimination during search */
    RlFloat m_grain;

    /** Whether to compute tenuki value (see below) automatically */
    bool m_estimateTenukiValue;

    /** The value of playing tenuki during q-search. */
    RlFloat m_tenukiValue;

    /** Power to use when estimating time for next iteration */
    RlFloat m_branchPower;

    /** The hash table */
    HashEntry* m_hashTable;
    
    /** Used during q-search to avoid recapturing kos */
    RlMarker m_koMarker;
    
    /** Used during q-search to mark ladders to pursue */
    RlMarker m_ladderMarker;
    
    /** How many times each player has played tenuki during q-search */
    SgBWArray<int> m_numTenuki;

    /** Current maximum depth during iterative deepening */
    int m_iterationDepth;
    
    /** Killer moves */
    RlKiller m_killer[RL_MAX_DEPTH];

    /** All moves sorted by static evaluation at root */
    std::vector<SgMove> m_sortedMoves;
    
    /** Current variation */
    std::vector<SgMove> m_variation;

    /** Statistics for current iteration of search */
    RlSearchStatistics m_stats;
};

//-----------------------------------------------------------------------------

inline RlKiller::RlKiller()
{
    Init(RL_MAX_KILLERS);
}

inline void RlKiller::Init(int numkillers)
{
    m_numKillers = numkillers;
    for (int n = 0; n < numkillers; n++)
        m_killerMoves[n] = SG_NULLMOVE;
}

inline SgMove& RlKiller::GetKiller(int n)
{
    return m_killerMoves[n];
}

inline SgMove RlKiller::GetKiller(int n) const
{
    return m_killerMoves[n];
}

inline void RlKiller::MarkKiller(SgMove move)
{
    SG_ASSERT(move != SG_NULLMOVE);

    int n;
    for (n = 0; n < m_numKillers; n++)
        if (m_killerMoves[n] == move)
            break;
            
    if (n < m_numKillers)
    {
        for (int i = n; i >= 1; --i)
            m_killerMoves[i] = m_killerMoves[i - 1];
    }
    else
    {
        for (int i = m_numKillers - 1; i >= 1; --i)
            m_killerMoves[i] = m_killerMoves[i - 1];
    }
    m_killerMoves[0] = move;
}

#endif // RL_ALPHABETA_H

//-----------------------------------------------------------------------------
