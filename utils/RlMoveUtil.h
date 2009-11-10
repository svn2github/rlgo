//----------------------------------------------------------------------------
/** @file RlMoveUtil.h
    Utility functions for moves in RLGO
*/
//----------------------------------------------------------------------------

#ifndef RLMOVEUTIL_H
#define RLMOVEUTIL_H

#include "SgBlackWhite.h"
#include "SgMove.h"
#include "SgPoint.h"
#include "GoBoardUtil.h"
#include "RlMiscUtil.h" 
#include <vector>

class GoBoard;
class RlEvaluator;
class RlMoveFilter;

//----------------------------------------------------------------------------

typedef std::pair<SgPoint, SgBlackWhite> RlStone;

class RlStoneList
{
public:

    void Clear();    
    void AddStone(const RlStone& stone);
    RlStone PopStone();
    int Size() const;
    const RlStone& GetStone(int index) const;

    /** Describe stone list in text form */
    void Describe(std::ostream& ostr) const;

    /** Describe stone list in GoGui form */
    void Display(std::ostream& cmd) const;

private:

    std::vector<RlStone> m_stones;
};

inline void RlStoneList::Clear()
{
    m_stones.clear();
}

inline void RlStoneList::AddStone(const RlStone& stone)
{
    m_stones.push_back(stone);
}

inline RlStone RlStoneList::PopStone()
{
    RlStone stone = m_stones.back();
    m_stones.pop_back();
    return stone;
}

inline int RlStoneList::Size() const 
{ 
    return m_stones.size(); 
}

inline const RlStone& RlStoneList::GetStone(int index) const 
{ 
    return m_stones[index]; 
}

//----------------------------------------------------------------------------
/** For some reason SgMarker doesn't have an exclude function...
    so define one here. */
class RlMarker
{
public:

    RlMarker()
    {
        Init();
    }

    void Include(SgPoint p)
    {
        m_mark[p] = m_thisMark;
    }
    
    void Exclude(SgPoint p)
    {
        m_mark[p] = m_thisMark - 1;
    }
    
    bool Contains(SgPoint p) const
    {
        return m_mark[p] == m_thisMark;
    }
    
    void Clear()
    {
        if (++m_thisMark == 0)
            Init();
    }

private:

    void Init()
    {
        m_thisMark = 1;
        m_mark.Fill(0);    
    }

    /** Current marker number */
    int m_thisMark;

    /** Marked points */
    SgArray<int,SG_MAXPOINT> m_mark;
};

//----------------------------------------------------------------------------

namespace RlMoveUtil
{

/** Get a stone from the board history */
RlStone GetHistory(const GoBoard& board, int movenum);

/** Mercy rule for quick resignation */
bool QuickResign(const GoBoard& board, SgBlackWhite toplay);

/** Select a random index from discrete probability distribution */
int SelectRandom(const std::vector<RlFloat>& probs);

/** Calculate the entropy of a discrete probability distribution */
RlFloat GetEntropy(const std::vector<RlFloat>& probs);

inline int Manhattan(SgPoint p1, SgPoint p2)
{
    return abs(SgPointUtil::Row(p1) - SgPointUtil::Row(p2)) 
        + abs(SgPointUtil::Col(p1) - SgPointUtil::Col(p2));
}

std::string WriteMoveSequence(const std::vector<SgMove>& sequence);

//----------------------------------------------------------------------------

} // namespace RlMoveUtil

//----------------------------------------------------------------------------

#endif // RLMOVEUTIL_H
