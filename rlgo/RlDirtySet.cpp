//----------------------------------------------------------------------------
/** @file RlDirtySet.cpp
    See RlDirtySet.h
*/
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "RlDirtySet.h"
#include "RlUtils.h"

#include "GoBoard.h"

using namespace std;
using namespace SgPointUtil;

//----------------------------------------------------------------------------

void RlDirtySet::MarkAtaris(GoBoard& bd, SgMove move, SgBlackWhite colour)
{
    // Mark ataris as dirty when specified move is executed.
    if (move == SG_PASS)
        return;
        
    for (SgNb4Iterator i_nb(move); i_nb; ++i_nb)
    {
        SgPoint nb = *i_nb;
        
        // Put enemy block in atari
        if (bd.GetColor(nb) == SgOppBW(colour)
            && bd.NumLiberties(nb) == 1)
        {
            Mark(bd.TheLiberty(nb), colour);
        }
    }
}

void RlDirtySet::MarkAll(GoBoard& bd)
{
    for (GoBoard::Iterator i_board(bd); i_board; ++i_board)
    {
        Mark(*i_board, SG_BLACK);
        Mark(*i_board, SG_WHITE);
    }
    Mark(SG_PASS, SG_BLACK);
    Mark(SG_PASS, SG_WHITE);
}

void RlDirtySet::ClearAll(GoBoard& bd)
{
    for (GoBoard::Iterator i_board(bd); i_board; ++i_board)
    {
        Clear(*i_board, SG_BLACK);
        Clear(*i_board, SG_WHITE);
    }
}

void RlDirtySet::Print(GoBoard& bd, ostream& str) const
{
    for (int j = bd.Size(); j >= 1; --j)
    {
        for (int i = 1; i <= bd.Size(); ++i)
        {
            str << m_dirty[0][Pt(i, j)];
            str << m_dirty[1][Pt(i, j)] << " ";
        }
        str << "\n";
    }
}


//----------------------------------------------------------------------------
