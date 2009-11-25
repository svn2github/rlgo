//-----------------------------------------------------------------------------
/* @file RlAlphaBeta.h 
    Alpha beta search engine */
//-----------------------------------------------------------------------------

#ifndef RL_ALPHABETA_H
#define RL_ALPHABETA_H

#include "RlUtils.h"
#include "SgTimer.h"

const int RL_SEARCH_MAX = 1600;
const int RL_MAX_KILLERS = 16;
const int RL_MAX_DEPTH = SG_MAX_MOVES * 2;

using namespace RlMathUtil;

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
    int Search(std::vector<SgMove>& pv);

    /** Clear hash table and other search data */
    void Clear();
    
    void SetMaxDepth(int value) { m_maxDepth = value; }
    void SetMaxTime(double value) { m_maxTime = value; }
    void SetMaxPredictedTime(double value) { m_maxPredictedTime = value; }    
    void SetMaxReductions(int value) { m_maxReductions = value; }
    void SetMaxExtensions(int value) { m_maxExtensions = value; }

    // Make public for unit testing only
    class Killer
    {
    public:

        Killer();
        void Init(int numkillers);
        void MarkKiller(SgMove move);
        SgMove& GetKiller(int n);
        SgMove GetKiller(int n) const;
        
    private:

        int m_numKillers;
        SgMove m_killerMoves[RL_MAX_KILLERS];
    };

private:

    struct HashEntry
    {
        RlHash m_hash;
        int m_depth;
        int m_lowerBound;
        int m_upperBound;
        SgMove m_bestMove;
    };

    enum
    {
        STAT_NODES,
        STAT_HASHHITS,
        STAT_HASHMISSES,
        STAT_HASHCUTS,
        STAT_COLLISIONS,
        STAT_EVALUATIONS,
        STAT_REDUCTIONS,
        STAT_REDCUTS,
        STAT_PVS,
        STAT_PVSCUTS,
        STAT_LATE,
        STAT_LATECUTS,
        STAT_EXTENSIONS,
        STAT_PARITY,
        STAT_BETACUTS,
        STAT_NOCUTS,
        STAT_FULLWIDTH,
        STAT_CHILDREN,
        NUM_STATS
    };
    
    int AlphaBeta(int depth, int alpha, int beta, 
        int numReductions, int numExtensions, int child);
    int BetaCut(int depth, int beta, SgMove bestMove, int stat);
    void PrincipalVariation(std::vector<SgMove>& pv);
    void SortMoves(std::vector<SgMove>& moves);
    void GenerateMoves(std::vector<SgMove>& moves, int depth, SgMove bestMove);
    void GenerateExtensions(std::vector<SgMove>& extensions);
    void PromoteKillers(std::vector<SgMove>& moves, int depth);
    void Promote(std::vector<SgMove>& moves, SgMove move);
    HashEntry& LookupHash();
    const HashEntry& LookupHash() const;
    SgMove ProbeBestMove();
    bool ProbeHash(int depth, int& alpha, int& beta, 
        int& eval, SgMove& bestMove);
    void StoreHash(int depth, SgMove move, int eval, 
        bool lower, bool upper);
    int Evaluate(int depth);
    void Play(SgMove move);
    void Undo();
    bool ConsiderMove(SgMove move);
    void MarkKiller(int depth, SgMove move);
    bool CheckAbort();
    void OutputStatistics(int eval, const std::vector<SgMove>& pv, 
        std::ostream& ostr);
    void OutputStatistic(const std::string& name, 
        int stat, std::ostream& ostr);
    void ClearStatistics();

    RlEvaluator* m_evaluator;

    /** Abort search if this depth is exceeded */
    int m_maxDepth;
    
    /** Abort search if las this time is exceeded after last iteration */
    double m_maxTime;

    /** Abort search if next iteration is expected to exceed this time */
    double m_maxPredictedTime;
    
    /** Number of entries in the hash table */
    int m_hashSize;

    /** Perform a full move sort at this depth or more */
    int m_sortDepth;
    
    /** Whether to use history heuristic */
    bool m_historyHeuristic;

    /** Whether to use killer heuristic */
    bool m_killerHeuristic;
    
    /** Number of killer moves to store and use */
    int m_numKillers;
    
    /** Number of opponent killer moves to use (<= m_numKillers) */
    int m_opponentKillers;
    
    /** Margin to use for cutting nodes */
    int m_cutMargin;
    
    /** Maximum number of recursive depth reductions in the same node */
    int m_maxReductions;

    /** Maximum number of depth extensions in any variation */
    int m_maxExtensions;
    
    /** Whether to ensure odd/even parity when using depth extensions */
    bool m_ensureParity;

    /** Whether to use null window PVS search */
    bool m_pvs;

    /** Use late move reductions on children with this branch or more
        Large number means never use */
    int m_lateMove;
    
    /** Power to use when estimating time for next iteration */
    RlFloat m_branchPower;
    
    /** The hash table */
    HashEntry* m_hashTable;
    
    /** Current maximum depth during iterative deepening */
    int m_iterationDepth;
    
    /** Killer moves */
    Killer m_killer[RL_MAX_DEPTH];
    
    /** History score */
    int m_history[RL_MAX_MOVES];

    /** All moves sorted by history score */
    std::vector<SgMove> m_sortedMoves;
    
    /** Current variation */
    std::vector<SgMove> m_variation;

    /** Statistics for current iteration of search */
    int m_stats[RL_MAX_DEPTH][NUM_STATS];

    /** Timer for current search depth */
    SgTimer m_timer;

    /** Time since search began */
    double m_elapsedTime;    
};

//-----------------------------------------------------------------------------

inline RlAlphaBeta::Killer::Killer()
{
    Init(RL_MAX_KILLERS);
}

inline void RlAlphaBeta::Killer::Init(int numkillers)
{
    m_numKillers = numkillers;
    for (int n = 0; n < numkillers; n++)
        m_killerMoves[n] = SG_NULLMOVE;
}

inline SgMove& RlAlphaBeta::Killer::GetKiller(int n)
{
    return m_killerMoves[n];
}

inline SgMove RlAlphaBeta::Killer::GetKiller(int n) const
{
    return m_killerMoves[n];
}

inline void RlAlphaBeta::Killer::MarkKiller(SgMove move)
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
