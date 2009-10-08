//----------------------------------------------------------------------------
/** @file RlLocalShape.cpp
*/
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "RlLocalShape.h"

#include "RlWeightSet.h"
#include "RlShapeUtil.h"
#include <sstream>

using namespace std;
using namespace RlShapeUtil;
using namespace SgHashUtil;
using namespace SgPointUtil;

//----------------------------------------------------------------------------

RlLocalShape::RlLocalShape(int xsize, int ysize)
:   m_xSize(xsize), m_ySize(ysize) 
{
    // To help debugging
    for (int i = 0; i < SG_MAX_SIZE * SG_MAX_SIZE; ++i)
        m_shape[i] = -1;
}

void RlLocalShape::SetShapeIndex(int shapeindex)
{
    int mul = 1;
    for (int j = 0; j < m_ySize; ++j)
    {
        for (int i = 0; i < m_xSize; ++i)
        {
            GetPoint(i, j) = (shapeindex / mul) % 3;
            mul *= 3;
        }
    }
}

int RlLocalShape::GetShapeIndex()
{
    int shapeindex = 0;
    int mul = 1;
    for (int j = 0; j < m_ySize; ++j)
    {
        for (int i = 0; i < m_xSize; ++i)
        {
            shapeindex += mul * GetPoint(i, j);
            mul *= 3;
        }
    }
    
    return shapeindex;
}

void RlLocalShape::SetSubShape(RlLocalShape& subshape, int xoff, int yoff)
{
    for (int j = 0; j < subshape.m_ySize; ++j)
    {
        for (int i = 0; i < subshape.m_xSize; ++i)
        {
            GetPoint(i + xoff, j + yoff) = subshape.GetPoint(i, j);
        }
    }
}

void RlLocalShape::GetSubShape(RlLocalShape& subshape, int xoff, int yoff)
{
    for (int j = 0; j < subshape.m_ySize; ++j)
    {
        for (int i = 0; i < subshape.m_xSize; ++i)
        {
            subshape.GetPoint(i, j) = GetPoint(i + xoff, j + yoff);
        }
    }
}

void RlLocalShape::FlipX()
{
    RlLocalShape copy = *this;
    for (int j = 0; j < m_ySize; ++j)
        for (int i = 0; i < m_xSize; ++i)
            GetPoint(i, j) = copy.GetPoint(m_xSize - i - 1, j);
}

void RlLocalShape::FlipY()
{
    RlLocalShape copy = *this;
    for (int j = 0; j < m_ySize; ++j)
        for (int i = 0; i < m_xSize; ++i)
            GetPoint(i, j) = copy.GetPoint(i, m_ySize - j - 1);
}

void RlLocalShape::Transpose()
{
    RlLocalShape copy = *this;
    for (int i = 0; i < m_xSize; ++i)
        for (int j = 0; j < m_ySize; ++j)
            GetPoint(i, j) = copy.GetPoint(j, i);
    swap(m_xSize, m_ySize);
}

void RlLocalShape::Invert(int maxlibs)
{
    for (int i = 0; i < m_xSize; ++i)
    {
        for (int j = 0; j < m_ySize; ++j)
        {
            if (maxlibs > 0)
            {
                int colour = InvertColourIndex(GetPoint(i, j) % 3);
                int libs = GetPoint(i, j) / 3;
                GetPoint(i, j) = libs * 3 + colour;
            }
            else
            {
                GetPoint(i, j) = InvertColourIndex(GetPoint(i, j));
            }
        }
    }
}

void RlLocalShape::operator=(RlLocalShape& shape)
{
    m_xSize = shape.m_xSize;
    m_ySize = shape.m_ySize;

    for (int j = 0; j < m_ySize; ++j)
        for (int i = 0; i < m_xSize; ++i)
            GetPoint(i, j) = shape.GetPoint(i, j);
}

void RlLocalShape::SetFromBoard(GoBoard& board, int x, int y, int maxlibs)
{
    // (x, y) indexed from 1
    for (int j = 0; j < m_ySize; ++j)
    {
        for (int i = 0; i < m_xSize; ++i)
        {
            SgPoint p = Pt(x + i, y + j);
            int c = board.GetColor(p);
            SG_ASSERT_EBW(c);
            int libs = 0;
            if (maxlibs > 0 && board.Occupied(p))
                libs = min(board.NumLiberties(p), maxlibs);
            GetPoint(i, j) = libs * 3 + ColourIndex(c);
        }
    }
}

RlHash RlLocalShape::GetHashCode()
{
    // (x, y) indexed from 1

    // @todo: This is hard-coded to work identically to FastShapes hash.
    // If FastShapes hash computation changes, this will become incorrect.
    RlHash fullhash;
    for (int y = m_ySize - 1; y >= 0; --y)
    {
        RlHash rowhash;
        for (int x = m_xSize - 1; x >= 0; --x)
        {
            int pval = GetPoint(x, y);
            RlHash pointhash = 
                pval ? GetZobrist<RL_HASHSIZE>(SG_MAXPOINT + pval) : RlHash();
            if (x < m_xSize - 1)
                rowhash.RollLeft(1);
            rowhash.Xor(pointhash);
        }

        if (y < m_ySize - 1)
            fullhash.RollLeft(m_xSize);
        fullhash.Xor(rowhash);
    }
    
    return fullhash;
}

void RlLocalShape::DescribePos(ostream& str, int x, int y)
{
    // (x, y) indexed from 1
    str.fill('0');
    str.width(2);
    str << x << "-";
    str.fill('0');
    str.width(2);
    str << y;
}

void RlLocalShape::DescribeShape(ostream& str, int maxlibs)
{
    for (int j = 0; j < m_ySize; ++j)
    {
        str << "-";
        for (int i = 0; i < m_xSize; ++i)
        {
            if (maxlibs > 0)
            {
                int colour = GetPoint(i, j) % 3;
                int libs = GetPoint(i, j) / 3;
                str << ToChColour(colour);
                SG_ASSERT(ToChColour(colour) != '!');
                if (colour == 0)
                {
                    SG_ASSERT(libs == 0);
                    str << '_';
                }
                else
                {
                    str << libs;
                }
            }
            else
            {
                str << ToChColour(GetPoint(i, j));
            }
        }
    }
}

SgRect RlLocalShape::GetBounds()
{
    // Bounds are indexed from 1 for consistency with SgRect
    SgRect bounds;
    for (int j = 0; j < m_ySize; ++j)
    {
        for (int i = 0; i < m_xSize; ++i)
        {
            int colour = GetPoint(i, j) % 3;
            if (colour != eEmpty)
                bounds.Include(Pt(i + 1, j + 1));
        }
    }
    return bounds;
}

//----------------------------------------------------------------------------
