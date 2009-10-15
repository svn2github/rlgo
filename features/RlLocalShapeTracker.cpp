//----------------------------------------------------------------------------
/** @file RlLocalShapeTracker.cpp
    See RlLocalShapeTracker.h
*/
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "RlLocalShapeTracker.h"

#include "RlDirtySet.h"
#include "RlLocalShape.h"
#include "RlLocalShapeFeatures.h"
#include "RlSetup.h"
#include "SgDebug.h"
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>

using namespace std;
using namespace RlPathUtil;
using namespace RlShapeUtil;
using namespace SgPointUtil;

//----------------------------------------------------------------------------

RlLocalShapeTracker::RlLocalShapeTracker(GoBoard& board, 
    RlLocalShapeFeatures* shapes, bool successorFile)
:   RlTracker(board),
    m_shapes(shapes),
    m_successorFile(successorFile)
{
    m_shapes->EnsureInitialised();

    m_numLocal = m_shapes->GetXSize() * m_shapes->GetYSize() * 3;
    m_numEntries = m_shapes->GetNumFeatures() * m_numLocal;
    m_successor = new int[m_numEntries];
    m_ignore = new bool[m_shapes->GetNumFeatures()];

    MakeLocalMoves();
    if (!LoadSuccessors())
    {
        MakeSuccessors();
        SaveSuccessors();
    }
}

RlLocalShapeTracker::~RlLocalShapeTracker()
{
    delete [] m_successor;
    delete [] m_ignore;
}

void RlLocalShapeTracker::Reset()
{
    RlTracker::Reset();

    if (MarkSet()) // Previous position marked for fast resets
    {
        for (int y = 0; y < m_shapes->GetYNum(); ++y)
        {
            for (int x = 0; x < m_shapes->GetXNum(); ++x)
            {
                SgPoint pt = Pt(x + 1, y + 1);
                m_index[pt] = m_markIndex[pt];
                if (!m_ignore[m_index[pt]])
                    NewChange(GetOffset(pt), m_index[pt], +1);
            }
        }
    }
    else
    {
        // Initialise to empty shapes everywhere
        for (int y = 0; y < m_shapes->GetYNum(); ++y)
        {
            for (int x = 0; x < m_shapes->GetXNum(); ++x)
            {
                SgPoint pt = Pt(x + 1, y + 1);
                m_index[pt] = m_shapes->EncodeIndex(
                    0, m_shapes->GetAnchorIndex(x, y));
            }
        }
        
        // Update once for each stone on the board
        for (GoBlockIterator i_block(m_board); i_block; ++i_block)
        {
            for (GoBoard::StoneIterator i_stone(m_board, *i_block); 
                i_stone; ++i_stone)
            {
                SgPoint stone = *i_stone;
                SgBlackWhite colour = m_board.GetColor(stone);

                int c = ColourIndex(colour);
                vector<LocalMove>& localmoves = m_localMoves[c][stone];
                for (vector<LocalMove>::iterator i_local = localmoves.begin(); 
                    i_local != localmoves.end(); ++i_local)
                {
                    m_index[i_local->m_anchor] = GetSuccessor(
                        m_index[i_local->m_anchor], i_local->m_localMove);
                }
            }
        }

        // Add one change for each anchor
        for (int y = 0; y < m_shapes->GetYNum(); ++y)
        {
            for (int x = 0; x < m_shapes->GetXNum(); ++x)
            {
                SgPoint pt = Pt(x + 1, y + 1);
                if (!m_ignore[m_index[pt]])
                    NewChange(GetOffset(pt), m_index[pt], +1);
            }
        }
    }
    
    m_changes.clear();
    m_step = 0;

    if (RlSetup::Get()->GetVerification())
        Verify();
}

void RlLocalShapeTracker::Execute(SgMove move, SgBlackWhite colour, 
    bool execute, bool store)
{
    RlTracker::Execute(move, colour, execute, store); 

    if (move != SG_PASS)
    {
        UpdateStone(move, colour, execute, store);
        if (m_board.CapturingMove())
        {
            for (GoPointList::Iterator i_captures(m_board.CapturedStones()); 
                i_captures; ++i_captures)
            {
                UpdateStone(*i_captures, SG_EMPTY, execute, store);
            }
        }
    }
    
    if (execute && RlSetup::Get()->GetVerification())
        Verify();
    if (execute)
        m_step++;
}

void RlLocalShapeTracker::Undo()
{
    RlTracker::Undo();

    m_step--;
    while (!m_changes.empty())
    {    
        Change& change = m_changes.back();
        if (change.m_step != m_step)
            break;
        
        m_index[change.m_anchor] = change.m_index;
        m_changes.pop_back();
    }

    if (RlSetup::Get()->GetVerification())
        Verify();
}

void RlLocalShapeTracker::UpdateStone(SgPoint stone, SgEmptyBlackWhite colour,
    bool execute, bool store)
{
    int c = ColourIndex(colour);
    vector<LocalMove>& localmoves = m_localMoves[c][stone];
    for (vector<LocalMove>::iterator i_local = localmoves.begin(); 
        i_local != localmoves.end(); ++i_local)
    {
        SgPoint anchor = i_local->m_anchor;
        if (execute)
        {
            int slot = GetOffset(anchor);
            if (store)
                Store(anchor);
            if (!m_ignore[m_index[anchor]])
                NewChange(slot, m_index[anchor], -1);
                
            m_index[anchor] = GetSuccessor(
                m_index[anchor], i_local->m_localMove);
            assert(m_index[anchor] != -1);
            if (!m_ignore[m_index[anchor]])
                NewChange(slot, m_index[anchor], +1);
        }
        else
        {
            int slot = GetOffset(anchor);
            if (!m_ignore[m_index[anchor]])
                NewChange(slot, m_index[anchor], -1);
            int successor = GetSuccessor(
                m_index[anchor], i_local->m_localMove);
            if (!m_ignore[successor])
                NewChange(slot, successor, +1);
        }
    }
}

void RlLocalShapeTracker::SetMark()
{
    RlTracker::SetMark();
    for (int y = 0; y < m_shapes->GetYNum(); ++y)
    {
        for (int x = 0; x < m_shapes->GetXNum(); ++x)
        {
            SgPoint pt = Pt(x + 1, y + 1);
            m_markIndex[pt] = m_index[pt];
        }
    }
}

int RlLocalShapeTracker::GetActiveSize() const
{
    return m_shapes->GetXNum() * m_shapes->GetYNum();
}

void RlLocalShapeTracker::UpdateDirty(SgMove move, SgBlackWhite colour,
    RlDirtySet& dirty)
{
    if (move == SG_PASS)
        return;

    dirty.MarkAtaris(m_board, move, colour);
    UpdateDirty(move, dirty);
    if (m_board.CapturingMove())
    {
        for (GoPointList::Iterator i_captures(m_board.CapturedStones()); 
            i_captures; ++i_captures)
        {
            UpdateDirty(*i_captures, dirty);
        }
    }
}

void RlLocalShapeTracker::UpdateDirty(SgPoint stone, RlDirtySet& dirty)
{
    int xoff = m_shapes->GetXSize() - 1;
    int yoff = m_shapes->GetYSize() - 1;
    SgRect rect(
        max(1, Col(stone) - xoff),
        min(m_board.Size(), Col(stone) + xoff),
        max(1, Row(stone) - yoff),
        min(m_board.Size(), Row(stone) + yoff));

    for (SgRectIterator i_rect(rect); i_rect; ++i_rect)
    {
        dirty.Mark(*i_rect, SG_BLACK);
        dirty.Mark(*i_rect, SG_WHITE);
    }
}

void RlLocalShapeTracker::MakeLocalMoves()
{
    for (int c = 0; c < 3; ++c)
    {
        for (GoBoard::Iterator i_board(m_board); i_board; ++i_board)
        {
            SgPoint point = *i_board;
            m_localMoves[c][point].clear();
            
            // Anchors of local shape affected by this stone
            int col = Col(point);
            int row = Row(point);
            int xmin = max(1, col - (m_shapes->GetXSize() - 1));
            int ymin = max(1, row - (m_shapes->GetYSize() - 1));
            int xmax = min(col, m_shapes->GetXNum());
            int ymax = min(row, m_shapes->GetYNum());
            
            for (int y = ymin; y <= ymax; ++y)
            {
                for (int x = xmin; x <= xmax; ++x)
                {
                    LocalMove localmove;
                    localmove.m_anchor = Pt(x, y);
                    localmove.m_localMove = GetLocalMove(col - x, row - y, c);
                    m_localMoves[c][point].push_back(localmove);
                }
            }            
        }
    }
}

void RlLocalShapeTracker::MakeSuccessors()
{
    RlDebug(RlSetup::VOCAL) << "Making successors for " 
        << m_shapes->SetName() << "...";
    for (int index = 0; index < m_shapes->GetNumFeatures(); ++index)
        for (int x = 0; x < m_shapes->GetXSize(); ++x)
            for (int y = 0; y < m_shapes->GetYSize(); ++y)
                for (int c = 0; c < 3; ++c)
                    m_successor[index * m_numLocal + GetLocalMove(x, y, c)] 
                        = m_shapes->LocalMove(index, x, y, c);
    for (int index = 0; index < m_shapes->GetNumFeatures(); ++index)
        m_ignore[index] = m_shapes->IsEmpty(index);
    RlDebug(RlSetup::VOCAL) << " done\n";
}

bfs::path RlLocalShapeTracker::GetFileName()
{
    ostringstream oss;
    oss << "Successors-" 
        << m_board.Size() << "x" << m_board.Size() << "-"
        << m_shapes->GetXSize() << "x" << m_shapes->GetYSize()
        << ".dat";
    return GetInputPath() / oss.str();
}

bool RlLocalShapeTracker::LoadSuccessors()
{
    if (!m_successorFile)
        return false;

    bfs::ifstream succ(GetFileName(), ios::binary);
    if (!succ)
        return false;
    RlDebug(RlSetup::VOCAL) << "Loading successors for " 
        << m_shapes->SetName() << "...";
    succ.read((char*) m_successor, m_numEntries * sizeof(int));
    succ.read((char*) m_ignore, m_shapes->GetNumFeatures() * sizeof(bool));
    RlDebug(RlSetup::VOCAL) << " done\n";
    return true;
}

bool RlLocalShapeTracker::SaveSuccessors()
{
    if (!m_successorFile)
        return false;

    bfs::ofstream succ(GetFileName(), ios::binary);
    if (!succ)
        return false;
        
    RlDebug(RlSetup::VOCAL) << "Saving successors for " 
        << m_shapes->SetName() << "...";
    succ.write((char*) m_successor, m_numEntries * sizeof(int));
    succ.write((char*) m_ignore, m_shapes->GetNumFeatures() * sizeof(bool));
    RlDebug(RlSetup::VOCAL) << " done\n";
    return true;
}

void RlLocalShapeTracker::DeleteSuccessorFile()
{
    RlDebug(RlSetup::VOCAL) << "Deleting successor file for " 
        << m_shapes->SetName() << "\n";
    bfs::remove(GetFileName());
}

int RlLocalShapeTracker::GetSuccessor(
    int index, int x, int y, int c) const
{
    return GetSuccessor(index, GetLocalMove(x, y, c));
}


inline int RlLocalShapeTracker::GetSuccessor(int index, int localmove) const
{
    int successor = m_successor[index * m_numLocal + localmove];
    SG_ASSERT(successor >= 0
              && successor < m_shapes->GetNumFeatures());
    return successor;
}

inline int RlLocalShapeTracker::GetLocalMove(
    int x, int y, int c) const
{
    int pos = y * m_shapes->GetXSize() + x;
    return pos * 3 + c;
}

inline int RlLocalShapeTracker::GetOffset(SgPoint anchor) const
{
    return (Row(anchor) - 1) * m_shapes->GetXNum() 
        + (Col(anchor) - 1);
}

inline void RlLocalShapeTracker::Store(SgPoint anchor)
{
    m_changes.push_back(Change(m_step, anchor, m_index[anchor]));
}

void RlLocalShapeTracker::Verify() const
{
    for (int y = 0; y < m_shapes->GetYNum(); ++y)
    {
        for (int x = 0; x < m_shapes->GetXNum(); ++x)
        {
            SgPoint point = Pt(x + 1, y + 1);
            RlLocalShape localshape(
                m_shapes->GetXSize(), m_shapes->GetYSize());
            localshape.SetFromBoard(m_board, x + 1, y + 1);
            int anchorindex = m_shapes->GetAnchorIndex(x, y);
            int shapeindex = localshape.GetShapeIndex();
            int index = m_shapes->EncodeIndex(shapeindex, anchorindex);
            if (index != m_index[point])
                throw SgException("Incremental update error");
        }
    }
}

//----------------------------------------------------------------------------
