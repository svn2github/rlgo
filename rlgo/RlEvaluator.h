//----------------------------------------------------------------------------
/** @file RlEvaluator.h
    Class for tracking simple, incremental, or differenced evaluations
*/
//----------------------------------------------------------------------------

#ifndef RLEVALUATOR_H
#define RLEVALUATOR_H

#include "RlActiveSet.h"
#include "RlDirtySet.h"
#include "RlTracker.h"
#include "RlUtils.h"

class RlBinaryFeatures;
class RlMoveFilter;
class RlState;
class RlWeightSet;

//----------------------------------------------------------------------------
/** Abstract class for tracking evaluations */
class RlEvaluator : public RlAutoObject
{
public:

    DECLARE_OBJECT(RlEvaluator);

    RlEvaluator(GoBoard& board, 
        RlBinaryFeatures* featureset = 0, 
        RlWeightSet* weightset = 0,
        RlMoveFilter* filter = 0);

    /** Load settings from specified file */
    void LoadSettings(std::istream& settings);

    /** Initialise before using, after trackers are added */
    virtual void Initialise();

    /** Reset evaluation to current position */
    void Reset();

    /** Update evaluation after a single move is made */
    void Execute(SgMove move, SgBlackWhite colour, bool real);

    /** Play move on board and update evaluation */
    void PlayExecute(SgMove move, SgBlackWhite colour, bool real);

    /** Update evaluation after taking back move */
    void Undo(bool real);
    
    /** Take back move on board and update evaluation */
    void TakeBackUndo(bool real);

    /** Evaluate a candidate move. 
        Collect changes in changelist (if specified). */
    RlFloat EvaluateMove(SgMove move, SgBlackWhite colour);

    RlFloat Eval() { return m_eval; }

    /** Evaluate all moves and calculate best move */
    void FindBest(RlState& state); 

    /** Refresh state evaluation to take account of weight changes */
    void RefreshValue(RlState& state);

    /** How many active features are computed by this evaluator */
    int GetActiveSize() const;

    /** Remember current position for fast resetting */
    void SetMark();
    void ClearMark();

    /** Get move filter */
    const RlMoveFilter* GetMoveFilter() const { return m_moveFilter; }
    
    /** Get currently active features */
    const RlActiveSet& Active() const { return m_tracker->Active(); }

    /** Get currently tracked change list */
    const RlChangeList& ChangeList() const { return m_tracker->ChangeList(); }
        
    /** Check whether specified move is dirty */
    bool IsDirty(SgMove move, SgBlackWhite colour) const 
    { 
        return m_dirty.IsDirty(move, colour);
    }
    
    /** Debug output */
    void PrintDirty(std::ostream& ostr);

    /** Ensure that differences are not enabled */
    void EnsureSimple();

protected:

    void AddWeights(const RlChangeList& changes, RlFloat& eval);
    void SubWeights(const RlChangeList& changes, RlFloat& eval);

    RlFloat EvalMoveSimple(SgMove move, SgBlackWhite colour);
    RlFloat EvalMoveDiffs(SgMove move, SgBlackWhite colour);

private:

    /** Top-level feature set. Used to create tracker(s) */
    RlBinaryFeatures* m_featureSet;

    /** Tracker to incrementally update features */
    RlTracker* m_tracker;
    
    /** Weight set corresponding to all features */
    RlWeightSet* m_weightSet;

    /** Move filter to incrementally update allowed moves */
    RlMoveFilter* m_moveFilter;
    
    /** Whether to use differences */
    bool m_differences;
    
    /** Whether to support undo */
    bool m_supportUndo;

    /** Current evaluation */
    RlFloat m_eval;

    /** Differences to evaluation for each move. 2 SG_PASS+1*/
    RlFloat m_diffs[RL_NUM_COLOURS][RL_MAX_MOVES];

    /** Which of the above differences are dirty and need recomputation */
    RlDirtySet m_dirty;
    
    /** Upper bound on the magnitude of evaluations */
    static const RlFloat s_maxEval;
};

//----------------------------------------------------------------------------
/** Class for sorting moves by their evaluation */
class RlMoveSorter
{
public:

    /** Evaluate moves and sort into best first order (according to colour) */
    void Sort(RlEvaluator* evaluator, SgBlackWhite toplay);
    
    /** Get the rank of a move in the sorted list */
    int GetRank(SgMove move) const;

    /** Get the proportional rank, i.e. rank/nummoves */
    RlFloat GetProportionalRank(SgMove move) const; 

    /** Accessors for sorted list of moves */
    int GetNumMoves() const { return m_evals.size(); }
    SgMove GetMove(int i) const { return m_evals[i].m_move; }
    RlFloat GetEval(int i) const { return m_evals[i].m_eval; }

private:

    void SortMoves(SgBlackWhite toplay);

    struct EvalPair
    {
        EvalPair(SgMove move, RlFloat eval)
        :   m_move(move), m_eval(eval)
        {
        }
        
        bool operator<(const EvalPair& rhs) const
        {
            return m_eval < rhs.m_eval;
        }

        bool operator>(const EvalPair& rhs) const
        {
            return m_eval > rhs.m_eval;
        }
        
        SgMove m_move;
        RlFloat m_eval;
    };

    std::vector<EvalPair> m_evals;
};

//----------------------------------------------------------------------------

#endif // RLEVALUATOR_H
