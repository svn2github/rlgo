//----------------------------------------------------------------------------
/** @file RlTracker.cpp
*/
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "RlTracker.h"

#include "RlBinaryFeatures.h"
#include "RlMoveFilter.h"

using namespace std;
using namespace SgPointUtil;

//----------------------------------------------------------------------------

RlTracker::RlTracker(GoBoard& board)
:   m_board(board),
    m_mark(false),
    m_tick(0)
{
}

void RlTracker::Initialise()
{
    Tick();
    int activesize = GetActiveSize();
    m_active.Resize(activesize);
    m_changeList.Clear();
}

void RlTracker::DoReset()
{
    ClearChanges();
    Reset();
    PropagateChanges();
    AddChanges(false);
}

void RlTracker::DoExecute(SgMove move, SgBlackWhite colour, bool store)
{
    ClearChanges();
    Execute(move, colour, true, store);
    PropagateChanges();
    AddChanges(store);
}

void RlTracker::DoEvaluate(SgMove move, SgBlackWhite colour)
{
    ClearChanges();
    Execute(move, colour, false, false);
    PropagateChanges();
}

void RlTracker::DoUndo()
{
    Undo();
    SubChanges();
}

void RlTracker::Reset()
{
    Tick();
    m_active.Clear();
    m_changeStack.Clear();
}

void RlTracker::Execute(SgMove move, SgBlackWhite colour, 
    bool execute, bool store)
{
    SG_UNUSED(move);
    SG_UNUSED(colour);
    SG_UNUSED(execute);
    SG_UNUSED(store);
    Tick();
}

void RlTracker::Undo()
{
    Tick();
}

void RlTracker::PropagateChanges()
{
    Tick();
}

void RlTracker::UpdateDirty(SgMove move, SgBlackWhite colour, 
    RlDirtySet& dirty)
{
    SG_UNUSED(move);
    SG_UNUSED(colour);
    SG_UNUSED(dirty);
    Tick();
}

void RlTracker::ClearChanges()
{
    Tick();
    m_changeList.Clear();
}

void RlTracker::AddChanges(bool store)
{
    // Call this after changes have been accumulated
    Tick();
    for (RlChangeList::Iterator i_changes(m_changeList); 
        i_changes; ++i_changes)
    {
        m_active.Change(*i_changes);
    }
    if (store)
        m_changeStack.Store(m_changeList);
}

void RlTracker::SubChanges()
{
    Tick();
    m_changeStack.Restore(m_changeList);
    for (RlChangeList::ReverseIterator i_changes(m_changeList); 
            i_changes; ++i_changes)
    {
        m_active.Change(RlChange(
            i_changes->m_slot, 
            i_changes->m_featureIndex,
            -i_changes->m_occurrences));
    }
}

RlTracker::ChangeStack::ChangeStack()
:   m_capacity(0),
    m_numChangeLists(0)
{
}

void RlTracker::ChangeStack::Clear()
{
    m_numChangeLists = 0;
}

void RlTracker::ChangeStack::Store(const RlChangeList& changelist)
{
    SG_ASSERT(m_numChangeLists <= m_capacity);
    if (m_numChangeLists == m_capacity)
    {
        m_stack.push_back(changelist);
        m_capacity++;
    }
    else
    {
        m_stack[m_numChangeLists].CopyList(changelist);
    }
    m_numChangeLists++;        
}

void RlTracker::ChangeStack::Restore(RlChangeList& changelist)
{
    m_numChangeLists--;
    changelist.CopyList(m_stack[m_numChangeLists]);
}

void RlTracker::MakeChangesToClearActive()
{
    // Make changes to clear all currently active entries
    for (RlActiveSet::Iterator i_active(Active()); i_active; ++i_active)
        NewChange(i_active.Slot(), 
            i_active->m_featureIndex, 
            -i_active->m_occurrences);
}

//----------------------------------------------------------------------------

