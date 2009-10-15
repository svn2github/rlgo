//----------------------------------------------------------------------------
/** @file RlTracker.h
    Base class for incrementally tracking evaluation and active set
*/
//----------------------------------------------------------------------------

#ifndef RLTRACKER_H
#define RLTRACKER_H

#include "RlActiveSet.h"
#include "RlUtils.h"

class RlDirtySet;
class RlMoveFilter;

//----------------------------------------------------------------------------
/** Base class for incrementally tracking active set of binary features */
class RlTracker
{
public:

    RlTracker(GoBoard& board);

    virtual ~RlTracker() { }

    /** Initialise tracker after (multi-stage) construction is complete */
    virtual void Initialise();

    /** Clear changes, reset, and apply changes */
    void DoReset();
    
    /** Clear changes, execute, and apply changes 
        Specify whether to store changes for subsequent undo */
    void DoExecute(SgMove move, SgBlackWhite colour, bool store);
    
    /** Clear changes, and execute without applying changes */
    void DoEvaluate(SgMove move, SgBlackWhite colour);
    
    /** Undo and apply changes */
    void DoUndo();

    /** Update dirty points for specified move */
    virtual void UpdateDirty(SgMove move, SgBlackWhite colour,
        RlDirtySet& dirty);

    /** Size of active set */
    virtual int GetActiveSize() const = 0;
    
    /** Debug output */
    virtual void Display(std::ostream& ostr) { SG_UNUSED(ostr); }

    /** Current list of changes (after GenMove, Execute or Undo) */
    const RlChangeList& ChangeList() const { return m_changeList; }

    /** Currently active features */
    const RlActiveSet& Active() const { return m_active; }

    /** Remember current position for fast resets */
    virtual void SetMark() { m_mark = true; }
    virtual void ClearMark() { m_mark = false; }
    bool MarkSet() const { return m_mark; }

    //-------------------------------------------------------------------------
    /* The previous functions provide the main API, and should be preferred. 
       Certain compound trackers may need to control the order of calling,
       for which the following functions are provided for finer control. */

    /** Reset evaluation and features to current board position */
    virtual void Reset() = 0;
    
    /** Update features from this move
        Called after board is updated */
    virtual void Execute(SgMove move, SgBlackWhite colour, 
        bool execute, bool store) = 0;
    
    /** Update features from undo
        Called after board is updated */
    virtual void Undo() = 0;

    /** Clear all changes including child trackers */
    virtual void ClearChanges();

    /** Propagate changes from child trackers */
    virtual void PropagateChanges();

    /** Add current change list into active set, and store for undo */
    virtual void AddChanges(bool store);
    
    /** Restore change list from undo stack, and subtract from active set */
    virtual void SubChanges();
    
    /** Colour graph to make sure that tracker doesn't get duplicate calls */
    void Tick() { m_tick++; }
    int Tock() const { return m_tick; }

protected:

    void StoreChanges();
    void RestoreChanges();
    void NewChange(int slot, int featureindex, RlOccur occurrences);
    void MakeChangesToClearActive();

    void MarkAtarisDirty(SgMove move, SgBlackWhite colour);
    void MarkAllDirty();

protected:

    GoBoard& m_board; //@todo: constify

private:

    /** Currently active features for this tracker */
    RlActiveSet m_active;

    /** Mark is set if data has been stored for fast reset */
    bool m_mark;

    /** For graph colouring */
    int m_tick;

    /** Current set of changes for this tracker 
        Use to update active features by calling AddChanges */
    RlChangeList m_changeList;
    
    /** Stack of previous changes if undo is supported */
    struct ChangeStack
    {
    
        std::vector<RlChangeList> m_stack;
        int m_capacity;
        int m_numChangeLists;
        
        ChangeStack();
        void Clear();
        void Store(const RlChangeList& changelist);
        void Restore(RlChangeList& changelist);
    } m_changeStack;
};

inline void RlTracker::NewChange(int slot, int featureindex, 
    RlOccur occurrences)
{
    m_changeList.Change(RlChange(slot, featureindex, occurrences));
}

//----------------------------------------------------------------------------

#endif // RLTRACKER_H
