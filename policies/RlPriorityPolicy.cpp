//----------------------------------------------------------------------------
/** @file RlPriorityPolicy.cpp
    See RlPriorityPolicy.h
*/
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "RlPriorityPolicy.h"

using namespace std;
using namespace GoBoardUtil;

//----------------------------------------------------------------------------

IMPLEMENT_OBJECT(RlPriorityPolicy);

RlPriorityPolicy::RlPriorityPolicy(GoBoard& board, RlPolicy* priority,
    RlPolicy* normal)
:   RlPolicy(board, 0),
    m_priorityPolicy(priority),
    m_normalPolicy(normal)
{
}

void RlPriorityPolicy::LoadSettings(istream& settings)
{
    settings >> RlSetting<RlPolicy*>("PriorityPolicy", m_priorityPolicy);
    settings >> RlSetting<RlPolicy*>("NormalPolicy", m_normalPolicy);
}

SgMove RlPriorityPolicy::SelectMove(RlState& state)
{
    SgMove move = SG_NULLMOVE;
    if (m_priorityPolicy)
        move = m_priorityPolicy->SelectMove(state);
    if (move == SG_NULLMOVE && m_normalPolicy)
        move = m_normalPolicy->SelectMove(state);
    return move;
}

//----------------------------------------------------------------------------

IMPLEMENT_OBJECT(RlAtariPolicy);

RlAtariPolicy::RlAtariPolicy(GoBoard& board)
:   RlPolicy(board, 0)
{
}

SgMove RlAtariPolicy::SelectMove(RlState& state)
{
    SgBlackWhite toplay = state.Colour();
    SgBlackWhite opp = SgOppBW(toplay);
    int capturestones = 0;
    int defendstones = 0;
    SgMove move = SG_NULLMOVE;

    // Consider only legal capturing or capture-saving moves
    for (GoBlockIterator iBlock(m_board); iBlock; ++iBlock)
    {
        SgPoint anchor = *iBlock;
        
        // Play largest available capture atari
        if (m_board.AtMostNumLibs(anchor, 1)
            && m_board.IsColor(anchor, opp) // capture atari
            && m_board.NumStones(anchor) >= capturestones)
        {
            SgMove candidate = m_board.TheLiberty(anchor);
            if (m_board.IsLegal(candidate))
            {
                move = candidate;                
                capturestones = m_board.NumStones(anchor);

                // Add any extra stones saved by this capture
                SgPointSet touched;
                for (GoBoard::StoneIterator icap(m_board, anchor); icap;
                     ++icap)
                {
                    for (SgNb4Iterator inb(*icap); inb; ++inb)
                    {
                        if (m_board.IsColor(*inb, toplay))
                        {
                            SgPoint anchor2 = m_board.Anchor(*inb);
                            if (m_board.AtMostNumLibs(anchor2, 1)
                                && !touched.Contains(anchor2))
                            {
                                capturestones += m_board.NumStones(anchor2);
                                touched.Include(anchor2);
                            }
                        }
                    }
                }
            }
        }

        // Play largest available defend atari, if no captures are available
        else if (m_board.AtMostNumLibs(anchor, 1)
            && m_board.IsColor(anchor, toplay)
            && capturestones == 0 
            && m_board.NumStones(anchor) >= defendstones)
        {
            SgMove candidate = m_board.TheLiberty(anchor);
            
            if (m_board.IsLegal(candidate) 
                && Approx2Libs(m_board, anchor, candidate, toplay) >= 2)
            {
                move = candidate;
                defendstones = m_board.NumStones(anchor);
            }
        }
    }
    
    if (move != SG_NULLMOVE)
        state.SetPolicyType(m_onPolicy ? RlState::POL_ON : RlState::POL_OFF);

    return move;
}

void RlAtariPolicy::LoadSettings(istream& settings)
{
    SG_UNUSED(settings);
}

//----------------------------------------------------------------------------
