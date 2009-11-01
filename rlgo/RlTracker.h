//----------------------------------------------------------------------------
/** @file RlTracker.h
    Base class for incrementally tracking changes to active set
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

    /** Reset evaluation and features to current board position */
    virtual void Reset() = 0;
    
    /** Update features from this move
        Called after board is updated */
    virtual void Execute(SgMove move, SgBlackWhite colour, 
        bool execute, bool store) = 0;
    
    /** Update features from undo
        Called after board is updated */
    virtual void Undo() = 0;

    /** Update dirty points for specified move */
    virtual void UpdateDirty(SgMove move, SgBlackWhite colour,
        RlDirtySet& dirty);

    /** Size of active set */
    virtual int GetActiveSize() const = 0;
    
    /** Debug output */
    virtual void Display(std::ostream& ostr) { SG_UNUSED(ostr); }

    /** Current list of changes (after Reset, Execute or Undo) */
    const RlChangeList& ChangeList() const { return m_changeList; }

    /** Remember current position for fast resets */
    virtual void SetMark() { m_mark = true; }
    virtual void ClearMark() { m_mark = false; }
    bool MarkSet() const { return m_mark; }

protected:

    void ClearChanges() { m_changeList.Clear(); }
    void NewChange(int slot, int featureindex, RlOccur occurrences);

    void MarkAtarisDirty(SgMove move, SgBlackWhite colour);
    void MarkAllDirty();

protected:

    GoBoard& m_board; //@todo: constify

private:

    /** Mark is set if data has been stored for fast reset */
    bool m_mark;

    /** Current set of changes for this tracker 
        Use to update active features by calling AddChanges */
    RlChangeList m_changeList;    
};

inline void RlTracker::NewChange(int slot, int featureindex, 
    RlOccur occurrences)
{
    m_changeList.Change(RlChange(slot, featureindex, occurrences));
}

//----------------------------------------------------------------------------

#endif // RLTRACKER_H
