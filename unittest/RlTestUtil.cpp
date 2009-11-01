//----------------------------------------------------------------------------
/** @file RlTestUtil.cpp
    See RlTestUtil.h
*/
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "RlTestUtil.h"

#include "RlBinaryFeatures.h"
#include "RlTracker.h"

using namespace std;

//----------------------------------------------------------------------------

bool tryall = true;
bool printdebug = false;

void TryAll(GoBoard& bd, RlTracker& tracker)
{
    if (!tryall)
        return;

    SgBlackWhite colour = bd.ToPlay();
    for (GoBoard::Iterator i_board(bd); i_board; ++i_board)
    {
        SgPoint p = *i_board;
        if (bd.IsLegal(p, colour))
        {
            bd.Play(p, colour);
            tracker.Execute(p, colour, false, false);
            bd.Undo();
        }
    }
}

void Reset(RlTracker& tracker, RlActiveSet& active, RlBinaryFeatures& features)
{
    tracker.Reset();
    for (RlChangeList::Iterator i_changes(tracker.ChangeList()); 
        i_changes; ++i_changes)
        active.Change(*i_changes);
    DisplayActive(active, features);
}

void Play(SgPoint point, SgBlackWhite colour, 
    GoBoard& bd, RlTracker& tracker, RlActiveSet& active, 
    RlBinaryFeatures& features)
{
    if (printdebug)
        cout << "Play " << SgBW(colour) << SgWritePoint(point) << endl;

    // Play/undo all moves at each step
    TryAll(bd, tracker);
    bd.Play(point, colour);
    tracker.Execute(point, colour, true, true);
    for (RlChangeList::Iterator i_changes(tracker.ChangeList()); 
        i_changes; ++i_changes)
        active.Change(*i_changes);
    DisplayActive(active, features);
}

void Undo(GoBoard& bd, RlTracker& tracker, RlActiveSet& active, RlBinaryFeatures& features)
{
    if (printdebug)
        cout << "Undo" << endl;

    // Play/undo all moves at each step
    TryAll(bd, tracker);
    bd.Undo();
    tracker.Undo();
    for (RlChangeList::Iterator i_changes(tracker.ChangeList()); 
        i_changes; ++i_changes)
        active.Change(*i_changes);
    DisplayActive(active, features);
}

SgRect MakeRect(int x, int y)
{
    return SgRect(x, x, y, y);
}

int CountOccurrences(const RlActiveSet& active, int featureindex)
{
    int total = 0;
    for (RlActiveSet::Iterator i_active(active); 
        i_active; ++i_active)
    {
        if (i_active->m_featureIndex == featureindex)
            total += i_active->m_occurrences;
    }
    return total;
}

void DisplayActive(const RlActiveSet& active, RlBinaryFeatures& features)
{
    if (!printdebug)
        return;

    cout << features.GetBoard();
    cout << "Active: " << active.GetTotalActive() << endl;
    for (RlActiveSet::Iterator i_active(active); 
        i_active; ++i_active)
    {
        int index = i_active->m_featureIndex;
        features.DescribeFeature(index, cout);
        cout << " : " << i_active->m_occurrences << " #" << index << endl;
    }
}

//----------------------------------------------------------------------------
