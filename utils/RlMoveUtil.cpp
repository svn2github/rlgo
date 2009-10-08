//----------------------------------------------------------------------------
/** @file RlMoveUtil.cpp
    See RlMoveUtil.h
*/
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "RlMoveUtil.h"

#include "GoBoard.h"
#include "SgBWArray.h"
#include "SgWrite.h"
#include <math.h>

using namespace std;

//----------------------------------------------------------------------------

void RlStoneList::Describe(ostream& ostr) const
{
    for (vector<RlStone>::const_iterator i_stones = m_stones.begin();
        i_stones != m_stones.end(); ++i_stones)
    {
        const RlStone& stone = *i_stones;
        if (stone.second == SG_BLACK)
            ostr << 'B';
        if (stone.second == SG_WHITE)
            ostr << 'W';
        if (stone.second == SG_EMPTY)
            ostr << 'E';
        ostr << "-";
        ostr << SgWritePoint(stone.first);
        ostr << "-";
    }
}

void RlStoneList::Display(ostream& cmd) const
{
    for (vector<RlStone>::const_iterator i_stones = m_stones.begin();
        i_stones != m_stones.end(); ++i_stones)
    {
        const RlStone& stone = *i_stones;
        if (stone.second == SG_BLACK)
            cmd << "BLACK " << SgWritePoint(stone.first) << "\n";
        if (stone.second == SG_WHITE)
            cmd << "WHITE " << SgWritePoint(stone.first) << "\n";
        cmd << "MARK " << SgWritePoint(stone.first) << "\n";
    }
}

//----------------------------------------------------------------------------

namespace RlMoveUtil
{

RlStone GetHistory(const GoBoard& board, int movenum)
{
    RlStone stone;
    GoPlayerMove move = board.Move(movenum);
    stone.first = move.Point();
    stone.second = move.Color();
    return stone;
}

bool QuickResign(const GoBoard& board, SgBlackWhite toplay)
{
    SgBWArray<int> numstones;
    numstones[SG_BLACK] = board.All(SG_BLACK).Size();
    numstones[SG_WHITE] = board.All(SG_WHITE).Size();    
    int numempty = board.AllEmpty().Size();
    
    // @todo: check for large ataris
    bool largeatari = false;

    // Resign if 1/3 of board is occupied, we have 1/3 or less stones,
    // and the opponent is not in atari
    return ((numstones[SG_BLACK] + numstones[SG_WHITE]) * 2 > numempty
        && numstones[toplay] * 2 < numstones[SgOppBW(toplay)]
        && !largeatari);
}

int SelectRandom(const vector<RlFloat>& probs)
{
    // Select a random index from discrete probability distribution
    int size = probs.size();
    RlFloat r = SgRandomFloat(0, 1);
    for (int i = 0; i < size; ++i)
    {
        r -= probs[i];
        if (r < 0)
            return i;
    }
    
    SG_ASSERT(fabs(r) < 0.001f);
    return size - 1;
}

RlFloat GetEntropy(const vector<RlFloat>& probs)
{
    int size = probs.size();
    float entropy = 0.0;
    for (int i = 0; i < size; ++i)
        if (probs[i] > 0.0)
            entropy -= probs[i] * log(probs[i]);
    return entropy;
}

} // namespace RlMoveUtil

//----------------------------------------------------------------------------
