//----------------------------------------------------------------------------
/** @file RlLocalShape.h
    Local shape manipulation
*/
//----------------------------------------------------------------------------

#ifndef RLLOCALSHAPE_H
#define RLLOCALSHAPE_H

#include "GoBoard.h"
#include "RlHashUtil.h"
#include "SgRect.h"

//----------------------------------------------------------------------------
/** Class for computing shape index for local configurations of stones
    This class is not designed for efficiency, and should not usually
    be directly used online */
class RlLocalShape
{
public:

    RlLocalShape(int xsize, int ysize);
    
    void SetShapeIndex(int shapeindex);
    int GetShapeIndex();

    void SetSubShape(RlLocalShape& subshape, int xoff, int yoff);
    void GetSubShape(RlLocalShape& subshape, int xoff, int yoff);

    void FlipX();
    void FlipY();
    void Transpose();
    void Invert(int maxlibs = 0);

    void operator=(RlLocalShape& shape);

    void SetFromBoard(GoBoard& board, int x, int y, int maxlibs = 0);
    RlHash GetHashCode();

    void DescribePos(std::ostream& str, int x, int y);
    void DescribeShape(std::ostream& str, int maxlibs = 0);

    int& GetPoint(int x, int y)
    {
        SG_ASSERT(x >= 0 && x < m_xSize && y >= 0 && y < m_ySize);
        return m_shape[y * m_xSize + x];
    }

    SgRect GetBounds();
    
private:

    int m_shape[SG_MAX_SIZE * SG_MAX_SIZE];
    int m_xSize, m_ySize;
};

//----------------------------------------------------------------------------

#endif // RLLOCALSHAPE_H

