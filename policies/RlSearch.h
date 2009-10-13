//----------------------------------------------------------------------------
/** @file RlSearch.h
    Full width alpha-beta search using agent evaluation
*/
//----------------------------------------------------------------------------

#ifndef RLSEARCH_H
#define RLSEARCH_H

#include "GoSearch.h"
#include "GoTimeControl.h"
#include "SgHashTable.h"
#include "SgVector.h"
#include "RlPolicy.h"

class RlAgent;
class RlLog;
class RlSharedMemory;
class RlTrace;

// @todo: remove agent from search class

//----------------------------------------------------------------------------
/** Search control that estimates remaining search time at each iteration */
class RlIterSearchControl : public SgSearchControl
{
public:
    
    RlIterSearchControl(GoBoard& board, int mindepth, 
        double fulltime, double aborttime, double branchpower);
    
    virtual bool Abort(double elapsedTime, int numNodes);
    virtual bool StartNextIteration(int depth, double elapsedTime,
        int numNodes);
          
    void SetMinDepth(int mindepth);
    void SetMaxTime(double fulltime, double aborttime);
    void SetBranchPower(double branchpower);
    
private:

    GoBoard& m_board;
    int m_minDepth;
    double m_fullTime, m_abortTime;
    double m_branchPower;
};

//----------------------------------------------------------------------------
/** Search class using RL evaluator */
class RlSearch : public GoSearch, public RlPolicy
{
public:

    RlSearch(GoBoard& board, RlEvaluator* evaluator = 0, RlAgent* agent = 0);
    ~RlSearch();

    /** GoSearch virtuals */
    virtual bool Execute(SgMove move, int* delta, int depth);
    virtual void TakeBack();
    virtual int Evaluate(SgVector<SgMove>* sequence, bool* isExact, int depth);
    virtual bool TraceIsOn() const;
    virtual void StartOfDepth(int depthLimit);

    /** Policy virtuals */
    virtual SgMove SelectMove(RlState& state);
    virtual void LoadSettings(std::istream& settings);
    virtual void Initialise();
    virtual bool SearchValue(RlFloat& value) const;

    /** Accessors */
    virtual int GetDepth() const { return m_maxDepth; }
    RlAgent* GetAgent() { return m_agent; }
    const SgMove* GetVariation() const { return m_variation; }

protected:

    int SearchToDepth(int depth, SgVector<SgMove>* pv, SgNode* tracenode);
    virtual int RunSearch(SgVector<SgMove>* pv, SgNode* tracenode) = 0;

    int ScaleEval(RlFloat value);
    RlFloat UnscaleEval(int value);
    void ClearVariation();

    // Logging
    virtual void InitLog();
    virtual void StepLog(int depth, int value, SgVector<SgMove>* pv);
    virtual void StepPlyLog(int depth);
    void FinalPlyLog();
    std::string WriteSequence(SgVector<SgMove>& sequence);
    void InitProbCut();

    int m_minDepth, m_maxDepth;
    bool m_iterative;
    bool m_trace;
    bool m_log;
    bool m_useProbCut;
    RlFloat m_searchValue;
    SgMove* m_variation;
    
private:

    int m_hashSize;
    SgSearchHashTable* m_hashTable;

    RlAgent* m_agent;

    std::string m_probCutFile;
    std::auto_ptr<RlLog> m_searchLog;
    std::auto_ptr<RlTrace> m_searchTrace;

    SgProbCut probcut;
};

//----------------------------------------------------------------------------
/** Main search class */
class RlMainSearch : public RlSearch
{
public:

    DECLARE_OBJECT(RlMainSearch);

    RlMainSearch(GoBoard& board, RlEvaluator* evaluator = 0, 
        RlAgent* agent = 0);

    ~RlMainSearch();

    /** AutoObject virtuals */
    virtual void LoadSettings(std::istream& settings);

    /** Initialise after loading */
    virtual void Initialise();

    /** Generate all legal moves in main search */
    virtual void Generate(SgVector<SgMove>* moves, int depth);

    virtual int GetDepth() const;

protected:

    void GenerateN(SgVector<SgMove>& moves, int N);
    void GenerateAll(SgVector<SgMove>& moves);
    virtual int RunSearch(SgVector<SgMove>* pv, SgNode* tracenode);

private:

    enum
    {
        eNone,
        eMaxTime,
        eControlTime,
        eControlIter
    };
    
    int m_controlMode;
    int m_oddEven;
    int m_maxBreadth;
    RlFloat m_maxTime;
    RlFloat m_fraction;
    RlFloat m_branchPower;
    RlFloat m_safetyTime;
        
    std::string m_sharedMemoryId;
    RlSharedMemory* m_sharedMemory;

    SgTimeSearchControl m_timeControl;
    RlIterSearchControl m_iterControl;
    GoTimeControl m_timeManager;
    const SgTimeRecord* m_timeRecord;
};

//----------------------------------------------------------------------------

#endif // RLSEARCH_H

