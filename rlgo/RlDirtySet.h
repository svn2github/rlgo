//----------------------------------------------------------------------------
/** @file RlDirtySet.h
    Class to track moves for which previous computation is no longer valid
*/
//----------------------------------------------------------------------------

#ifndef RLDIRTYSET_H
#define RLDIRTYSET_H

#include "RlShapeUtil.h"
#include "SgBWSet.h"

class GoBoard;

//----------------------------------------------------------------------------

class RlDirtySet
{
public:

    void ClearAll(GoBoard& bd);
    void MarkAll(GoBoard& bd);
    void Clear(SgMove move, SgBlackWhite colour);
    void Mark(SgMove move, SgBlackWhite colour);
    bool IsDirty(SgMove move, SgBlackWhite colour) const;
    void MarkAtaris(GoBoard& bd, SgMove move, SgBlackWhite colour);
    void Print(GoBoard& bd, std::ostream& str) const;

    /** Count how frequently moves are pruned by difference evaluation */
    void IncPruned(bool pruned) { m_pruned[pruned]++; }

private:

    bool m_dirty[2][SG_PASS + 1];
    int m_pruned[2];
};

inline void RlDirtySet::Mark(SgMove move, SgBlackWhite colour)
{
    m_dirty[RlShapeUtil::BWIndex(colour)][move] = true;
}

inline void RlDirtySet::Clear(SgMove move, SgBlackWhite colour)
{
    m_dirty[RlShapeUtil::BWIndex(colour)][move] = false;
}

inline bool RlDirtySet::IsDirty(SgMove move, SgBlackWhite colour) const
{
    return m_dirty[RlShapeUtil::BWIndex(colour)][move];
}

//----------------------------------------------------------------------------

#endif // RLDIRTYSET_H
