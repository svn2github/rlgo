//----------------------------------------------------------------------------
/** @file RlPointUtil.cpp
    See RlPointUtil.h
*/
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "RlPointUtil.h"

using namespace std;
using namespace SgPointUtil;

//----------------------------------------------------------------------------

const RlPointIndex* RlPointIndex::s_pointIndex = 0;

RlPointIndex::RlPointIndex(int size)
{
    m_numPoints = size * size;

    for (int i = 0; i < SG_MAXPOINT; ++i)
    {
        m_index[i] = Error();
        m_point[i] = SG_NULLPOINT;
    }
    
    for (int x = 0; x < size; ++x)
    {
        for (int y = 0; y < size; ++y)
        {
            m_index[Pt(x + 1, y + 1)] = y * size + x;
            m_point[y * size + x] = Pt(x + 1, y + 1);
        }
    }

    m_index[SG_PASS] = Pass();
    m_point[Pass()] = SG_PASS;

    s_pointIndex = this;
}

//----------------------------------------------------------------------------
