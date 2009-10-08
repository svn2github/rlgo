//----------------------------------------------------------------------------
/** @file RlPointUtil.h
    Simple point utility functions
*/
//----------------------------------------------------------------------------

#ifndef RLPOINTUTIL_H
#define RLPOINTUTIL_H

#include "SgPoint.h"

//----------------------------------------------------------------------------
/** Class to convert SgPoint to an index in [0, size*size) */
class RlPointIndex
{
public:

    RlPointIndex(int size);

    int GetIndex(SgPoint point) const { return m_index[point]; }
    SgPoint GetPoint(int index) const { return m_point[index]; }
    int NumPoints() const { return m_numPoints; }
    int NumPointsPass() const { return m_numPoints + 1; }
    int Pass() const { return m_numPoints; }
    int Error() const { return m_numPoints + 1; }
    
    static const RlPointIndex* Get() { return s_pointIndex; }
    
private:

    static const RlPointIndex* s_pointIndex;
    int m_numPoints;
    int m_index[SG_PASS + 1]; // Includes pass move
    SgPoint m_point[SG_PASS + 1];
};

inline const RlPointIndex* RlPoint()
{
    return RlPointIndex::Get();
}

//----------------------------------------------------------------------------

#endif // RLPOINTUTIL_H
