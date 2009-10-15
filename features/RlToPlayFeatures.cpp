//----------------------------------------------------------------------------
/** @file RlToPlayFeatures.cpp
    See RlToPlayFeatures.h
*/
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "RlToPlayFeatures.h"

#include "RlShapeUtil.h"

using namespace std;
using namespace RlShapeUtil;

//----------------------------------------------------------------------------

IMPLEMENT_OBJECT(RlToPlayFeatures);

RlToPlayFeatures::RlToPlayFeatures(GoBoard& board)
:   RlBinaryFeatures(board)
{
}

RlTracker* RlToPlayFeatures::CreateTracker(
    map<RlBinaryFeatures*, RlTracker*>& trackermap)
{
    SG_UNUSED(trackermap);
    SG_ASSERT(IsInitialised());
    return new RlToPlayTracker(m_board);
}

int RlToPlayFeatures::GetNumFeatures() const
{
    return 2;
}

int RlToPlayFeatures::ReadFeature(std::istream& desc) const
{
    string s;
    desc >> s;
    if (s == "B")
        return 0;
    else if (s == "W")
        return 1;
    else
        throw SgException("Misnamed ToPlay feature");
    return 0;
}

void RlToPlayFeatures::DescribeFeature(int featureindex, std::ostream& str) const
{
    if (featureindex == 0)
        str << "B";
    else if (featureindex == 1)
        str << "W";
    else
        throw SgException("Only two ToPlay features");
}

void RlToPlayFeatures::DescribeSet(std::ostream& str) const
{
    // Single word description (no whitespace)
    str << "ToPlay";
}

//----------------------------------------------------------------------------

RlToPlayTracker::RlToPlayTracker(GoBoard& board)
:   RlTracker(board)
{
}

void RlToPlayTracker::Reset()
{
    RlTracker::Reset();
    if (m_board.ToPlay() == SG_BLACK)
        NewChange(0, 0, 1);
    else
        NewChange(0, 1, 1);
}

void RlToPlayTracker::Execute(SgMove move, SgBlackWhite colour, 
    bool execute, bool store)
{
    RlTracker::Execute(move, colour, execute, store);
    SgBlackWhite newcolour = m_board.ToPlay();
    if (newcolour == SG_BLACK)
    {
        NewChange(0, 0, +1);
        NewChange(0, 1, -1);
    }
    else
    {
        NewChange(0, 0, -1);
        NewChange(0, 1, +1);
    }
}

void RlToPlayTracker::Undo()
{
    RlTracker::Undo();
    SgBlackWhite newcolour = m_board.ToPlay();
    if (newcolour == SG_BLACK)
    {
        NewChange(0, 0, +1);
        NewChange(0, 1, -1);
    }
    else
    {
        NewChange(0, 0, -1);
        NewChange(0, 1, +1);
    }
}

int RlToPlayTracker::GetActiveSize() const
{
    return 1;
}

//----------------------------------------------------------------------------
