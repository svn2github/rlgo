//----------------------------------------------------------------------------
/** @file RlMoveFilter.cpp
*/
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "GoEyeUtil.h"
#include "RlMoveFilter.h"
#include "RlUtils.h"
#include "SgPointSetUtil.h"
#include "SgRandom.h"

using namespace std;
using namespace SgPointUtil;
using namespace RlMoveUtil;

//----------------------------------------------------------------------------

IMPLEMENT_OBJECT(RlMoveFilter);

RlMoveFilter::RlMoveFilter(GoBoard& board, bool pass)
:   RlAutoObject(board),
    m_considerPass(pass)
{
}

void RlMoveFilter::LoadSettings(istream& settings)
{
    settings >> RlSetting<bool>("ConsiderPass", m_considerPass);
}

void RlMoveFilter::Reset()
{
    m_vacant.clear();
    if (m_considerPass)
    {
        m_vacant.push_front(SG_PASS);
        m_iVacant[SG_PASS] = m_vacant.begin();
    }
    else
    {
        m_iVacant[SG_PASS] = m_vacant.end();
    }

    for (GoBoard::Iterator i_board(m_board); i_board; ++i_board)
    {
        SgPoint point = *i_board;
        if (m_board.IsEmpty(point))
        {
            m_vacant.push_front(point);
            m_iVacant[point] = m_vacant.begin();
        }
        else
        {
            m_iVacant[point] = m_vacant.end();
        }
    }
    
    m_changes.clear();
    m_step = 0;
}

void RlMoveFilter::Execute(SgMove move, SgBlackWhite colour)
{
    SG_UNUSED(colour);
    if (move != SG_PASS)
    {
        // Track vacant points
        ClearVacant(move);
        Store(move, false);
        
        if (m_board.CapturingMove())
        {
            for (GoPointList::Iterator i_captures(m_board.CapturedStones()); 
                i_captures; ++i_captures)
            {
                SgPoint capture = *i_captures;
                AddVacant(capture);
                Store(capture, true);
            }
        }
    }
    
    m_step++;
}

void RlMoveFilter::Undo()
{
    m_step--;
    while (!m_changes.empty())
    {    
        Change& change = m_changes.back();
        if (change.m_step != m_step)
            break;
        
        if (change.m_addVacant)
            ClearVacant(change.m_point);
        else
            AddVacant(change.m_point);

        m_changes.pop_back();
    }
}

inline void RlMoveFilter::AddVacant(SgPoint point)
{
    SG_ASSERT(point != SG_NULLMOVE && point != SG_PASS);
    SG_ASSERT(! m_board.Occupied(point));
    SG_ASSERT(m_iVacant[point] == m_vacant.end());

    m_vacant.push_front(point);
    m_iVacant[point] = m_vacant.begin();
}

inline void RlMoveFilter::ClearVacant(SgPoint point)
{
    SG_ASSERT(point != SG_NULLMOVE && point != SG_PASS);
    SG_ASSERT(m_board.Occupied(point));
    SG_ASSERT(m_iVacant[point] != m_vacant.end());

    m_vacant.erase(m_iVacant[point]);
    m_iVacant[point] = m_vacant.end();
}

inline void RlMoveFilter::Store(SgPoint point, bool addvacant)
{
    m_changes.push_back(Change(point, addvacant, m_step));
}

bool RlMoveFilter::ConsiderMove(SgMove move, SgBlackWhite colour,
    bool checklegal) const
{
    return !checklegal || m_board.IsLegal(move, colour);
}

SgMove RlMoveFilter::GetRandomMove(SgBlackWhite colour) const
{
    if (m_vacant.empty())
        return SG_PASS;

    // First try a random vacant point, which will usually be allowed
    int numvacant = m_vacant.size();
    int index = SgRandom::Global().Int(numvacant);
    list<SgMove>::const_iterator i_vacant = m_vacant.begin();
    advance(i_vacant, index);
    if (ConsiderMove(*i_vacant, colour))
        return *i_vacant;
    
    // Otherwise enumerate all allowed moves, and choose randomly
    vector<SgMove> moves;
    GetMoveVector(colour, moves);
        
    if (moves.empty())
        return SG_PASS;
    else
        return moves[SgRandom::Global().Int(moves.size())];
}

void RlMoveFilter::GetMoveVector(SgBlackWhite colour, 
    vector<SgMove>& moves) const
{
    moves.clear();
    for (Iterator i_allowed(*this, colour); i_allowed; ++i_allowed)
        moves.push_back(*i_allowed);
}

void RlMoveFilter::GetVacantVector(SgVector<SgMove>& moves) const
{
    moves.Clear();
    for (list<SgMove>::const_iterator i_vacant = m_vacant.begin();
        i_vacant != m_vacant.end(); ++i_vacant)
    {
        SgMove move = *i_vacant;
        moves.PushBack(move);
    }
}

void RlMoveFilter::GetVacantVector(std::vector<SgMove>& moves) const
{
    moves.clear();
    for (list<SgMove>::const_iterator i_vacant = m_vacant.begin();
        i_vacant != m_vacant.end(); ++i_vacant)
    {
        SgMove move = *i_vacant;
        moves.push_back(move);
    }
}

bool RlMoveFilter::Empty(SgBlackWhite colour) const
{
    for (list<SgMove>::const_iterator i_vacant = m_vacant.begin(); 
        i_vacant != m_vacant.end(); ++i_vacant)
    {
        if (ConsiderMove(*i_vacant, colour))
            return false;
    }
    
    return true;
}

int RlMoveFilter::NumAllowed(SgBlackWhite colour) const
{
    int numallowed = 0;
    for (list<SgMove>::const_iterator i_vacant = m_vacant.begin(); 
        i_vacant != m_vacant.end(); ++i_vacant)
    {
        if (ConsiderMove(*i_vacant, colour))
            numallowed++;
    }
    return numallowed;
}

//----------------------------------------------------------------------------

IMPLEMENT_OBJECT(RlSingleEyeFilter);

RlSingleEyeFilter::RlSingleEyeFilter(GoBoard& board, bool pass)
:   RlMoveFilter(board, pass)
{
}

bool RlSingleEyeFilter::ConsiderMove(SgMove move, SgBlackWhite colour,
    bool checklegal) const
{
    return RlMoveFilter::ConsiderMove(move, colour, checklegal)
        && !GoEyeUtil::IsSinglePointEye(m_board, move, colour);
}

//----------------------------------------------------------------------------

IMPLEMENT_OBJECT(RlSimpleEyeFilter);

RlSimpleEyeFilter::RlSimpleEyeFilter(GoBoard& board, bool pass)
:   RlMoveFilter(board, pass)
{
}

bool RlSimpleEyeFilter::CheckEye(GoBoard& board, 
    SgMove move, SgBlackWhite colour)
{
    if (move == SG_PASS)
        return false;

    int numadj, numfalse;
    if (board.Line(move) == 1)
    {
        if (board.Pos(move) == 1)
        {
            numadj = 2;
            numfalse = 1;
        }
        else
        {
            numadj = 3;
            numfalse = 1;        
        }
    }
    else
    {
        numadj = 4;
        numfalse = 2;
    }
    
    // Must be adjacent on all sides to friendly stones
    if (board.NumNeighbors(move, colour) < numadj)
        return false;

    // Must not be a false eye
    if (board.NumDiagonals(move, SgOppBW(colour)) >= numfalse)
        return false;

    return true;
}

bool RlSimpleEyeFilter::ConsiderMove(SgMove move, SgBlackWhite colour,
    bool checklegal) const
{
    return RlMoveFilter::ConsiderMove(move, colour, checklegal)
        && !CheckEye(m_board, move, colour);
}

//----------------------------------------------------------------------------

IMPLEMENT_OBJECT(RlProximityFilter);

RlProximityFilter::RlProximityFilter(GoBoard& board, 
    bool pass, int maxdistance)
:   RlMoveFilter(board, pass),
    m_maxDistance(maxdistance)
{
}

void RlProximityFilter::LoadSettings(istream& settings)
{
    settings >> RlSetting<int>("MaxDistance", m_maxDistance);
}

bool RlProximityFilter::ConsiderMove(SgMove move, SgBlackWhite colour,
    bool checklegal) const
{
    return Manhattan(m_board.GetLastMove(), move) <= m_maxDistance
        && RlMoveFilter::ConsiderMove(move, colour, checklegal);
}

//----------------------------------------------------------------------------

IMPLEMENT_OBJECT(RlPointSetFilter);

RlPointSetFilter::RlPointSetFilter(GoBoard& board, bool pass)
:   RlMoveFilter(board, pass)
{
}

void RlPointSetFilter::LoadSettings(istream& settings)
{
    settings >> RlToken("Points") 
             >> RlToken("=") 
             >> SgReadPointSet(m_points);
}

bool RlPointSetFilter::ConsiderMove(SgMove move, SgBlackWhite colour,
    bool checklegal) const
{
    return m_points.Contains(move)
        && RlMoveFilter::ConsiderMove(move, colour, checklegal);
}

//----------------------------------------------------------------------------

IMPLEMENT_OBJECT(RlUnionFilter);

RlUnionFilter::RlUnionFilter(GoBoard& board, bool pass)
:   RlMoveFilter(board, pass)
{
}

void RlUnionFilter::LoadSettings(istream& settings)
{
    settings >> RlSetting<vector<RlMoveFilter*> >("Filters", m_filters);
}

bool RlUnionFilter::ConsiderMove(SgMove move, SgBlackWhite colour,
    bool checklegal) const
{
    for (vector<RlMoveFilter*>::const_iterator i_filters = m_filters.begin();
        i_filters != m_filters.end(); ++i_filters)
    {
        const RlMoveFilter* filter = *i_filters;
        if (filter->ConsiderMove(move, colour, checklegal))
            return true;
    }
    
    return false;
}

//----------------------------------------------------------------------------

IMPLEMENT_OBJECT(RlIntersectionFilter);

RlIntersectionFilter::RlIntersectionFilter(GoBoard& board, bool pass)
:   RlMoveFilter(board, pass)
{
}

void RlIntersectionFilter::LoadSettings(istream& settings)
{
    settings >> RlSetting<vector<RlMoveFilter*> >("Filters", m_filters);
}

bool RlIntersectionFilter::ConsiderMove(SgMove move, SgBlackWhite colour,
    bool checklegal) const
{
    for (vector<RlMoveFilter*>::const_iterator i_filters = m_filters.begin();
        i_filters != m_filters.end(); ++i_filters)
    {
        const RlMoveFilter* filter = *i_filters;
        if (!filter->ConsiderMove(move, colour, checklegal))
            return false;
    }
    
    return true;
}

//----------------------------------------------------------------------------
