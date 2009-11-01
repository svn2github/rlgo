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
    m_mark(false)
{
}

void RlTracker::Initialise()
{
}

void RlTracker::Reset()
{
    ClearChanges();
}

void RlTracker::Execute(SgMove move, SgBlackWhite colour, 
    bool execute, bool store)
{
    SG_UNUSED(move);
    SG_UNUSED(colour);
    SG_UNUSED(execute);
    SG_UNUSED(store);
    ClearChanges();
}

void RlTracker::Undo()
{
    ClearChanges();
}

void RlTracker::UpdateDirty(SgMove move, SgBlackWhite colour, 
    RlDirtySet& dirty)
{
    SG_UNUSED(move);
    SG_UNUSED(colour);
    SG_UNUSED(dirty);
}

//----------------------------------------------------------------------------

