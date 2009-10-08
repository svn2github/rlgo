//----------------------------------------------------------------------------
/** @file RlLocalShapeFeatures.cpp
    See RlLocalShapeFeatures.h
*/
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "RlLocalShapeFeatures.h"

#include "RlLocalShape.h"
#include "RlLocalShapeTracker.h"
#include "RlShapeUtil.h"
//!!!#include "RlTex.h"

using namespace std;
using namespace RlShapeUtil;
using namespace SgHashUtil;
using namespace SgPointUtil;

//----------------------------------------------------------------------------
/** Note that all (x, y) coordinates are indexed from 0 to size - 1 */

IMPLEMENT_OBJECT(RlLocalShapeFeatures);

RlLocalShapeFeatures::RlLocalShapeFeatures(
    GoBoard& board, 
    int xsize, int ysize)
:   RlBinaryFeatures(board),
    m_xSize(xsize),
    m_ySize(ysize),
    m_displayMode(eNone)
{
}

void RlLocalShapeFeatures::LoadSettings(istream& settings)
{
    RlBinaryFeatures::LoadSettings(settings);

    int version;
    settings >> RlVersion(version, 1);
    settings >> RlSetting<int>("XSize", m_xSize);
    settings >> RlSetting<int>("YSize", m_ySize);
}

void RlLocalShapeFeatures::Initialise()
{
    m_xNum = m_board.Size() - m_xSize + 1;
    m_yNum = m_board.Size() - m_ySize + 1;

    m_numShapes = 1;
    for (int i = 0; i < m_xSize * m_ySize; ++i)
        m_numShapes *= 3;
        
    m_numFeatures = m_xNum * m_yNum * m_numShapes;
    RlBinaryFeatures::Initialise();
}

RlTracker* RlLocalShapeFeatures::CreateTracker(
    map<RlBinaryFeatures*, RlTracker*>& trackermap)
{
    SG_UNUSED(trackermap);
    SG_ASSERT(IsInitialised());
    return new RlLocalShapeTracker(m_board, this);
}

int RlLocalShapeFeatures::ReadFeature(istream& desc) const
{
    // Descriptions must be of the form:
    //   "03-02-shape"
    // where coordinates specify anchor point: XX YY
    // and "shape" depends on particular shape class
    
    // Get anchor coords (indexed from 0)
    char x1, x2, y1, y2, c;
    desc >> x1 >> x2 >> c >> y1 >> y2;

    int x = MakeNumber(x1, x2) - 1;
    int y = MakeNumber(y1, y2) - 1;
    SG_ASSERT(c == '-');
    
    int anchorindex = GetAnchorIndex(x, y);
    int shapeindex = GetShapeIndex(desc);
    SG_ASSERT(shapeindex < m_numShapes);
    int featureindex = EncodeIndex(shapeindex, anchorindex);
        
    return featureindex;
}
 
SgRect RlLocalShapeFeatures::DisplayRect(int x, int y, 
    int& offsetx, int& offsety) const
{
    // Calculate partial rectangle to display (index from 0)
    if (m_displayMode == eLI)
    {
        offsetx = offsety = 2; // offset display away from border
        return SgRect(2, 2 + m_xSize + 1, 2, 2 + m_ySize + 1);
    }
    else if (m_displayMode == eLD)
    {
        offsetx = offsety = 0;
        return SgRect(0, x + m_xSize + 1, 0, y + m_ySize + 1);
    }
    else
    {
        offsetx = offsety = 0;
        return SgRect(); // full board display
    }
}

void RlLocalShapeFeatures::DescribeFeature(
    int featureindex, ostream& str) const
{
    int shapeindex, anchorindex, x, y;
    DecodeIndex(featureindex, shapeindex, anchorindex, x, y);
    return DescribeFeatureShort(shapeindex, x, y, str);
}

void RlLocalShapeFeatures::DescribeTex(
    int featureindex, ostream& tex, bool invert) const
{
    int shapeindex, anchorindex, x, y;
    if (invert)
        featureindex = Invert(featureindex);
    DecodeIndex(featureindex, shapeindex, anchorindex, x, y);
    DescribeFeatureTex(shapeindex, x, y, tex);
}

void RlLocalShapeFeatures::DescribeFeatureShort(
    int shapeindex, int x, int y, ostream& str) const
{
    RlLocalShape localshape(m_xSize, m_ySize);
    localshape.SetShapeIndex(shapeindex);
    localshape.DescribePos(str, x + 1, y + 1);
    localshape.DescribeShape(str);
}

void RlLocalShapeFeatures::DescribeFeatureLong(
    int shapeindex, int x, int y, ostream& str) const
{
    // Describe the shape in its context on the Go board (as a board region)
    int mul = 1;
    int offsetx, offsety;
    SgRect displayrect = DisplayRect(x, y, offsetx, offsety);

    for (int j = displayrect.Top(); j <= displayrect.Bottom(); ++j)
    {
        for (int i = displayrect.Left(); i <= displayrect.Right(); ++i)
        {
            if (InLocalShape(x, y, i - offsetx, j - offsety))
            {
                int shapevalue = (shapeindex / mul) % 3;
                mul *= 3;
                str << ToChColour(shapevalue);
            }
            else if (m_displayMode == eLI)
            {
                str << ' '; // don't display boundaries
            }
            else
            {
                bool xboundary = (i == 0 || i == m_board.Size() - 1);
                bool yboundary = (j == 0 || j == m_board.Size() - 1);
                if (xboundary && yboundary)
                    str << '+';
                else if (xboundary)
                    str << '|';
                else if (yboundary)
                    str << '-';
                else 
                    str << ' ';
            }
        }
        str << "\n";
    }
}

void RlLocalShapeFeatures::DescribeFeatureTex(
    int shapeindex, int x, int y, ostream& tex) const
{        
    //!!!!!!!!!!!!!
    // commented out until I figure out how to deal with
    // calling sumfeatures from rlgo module
    /*
    int offsetx, offsety;
    SgRect displayrect = DisplayRect(x, y, offsetx, offsety);

    RlTexBoard texboard(m_board.Size(), tex, offsetx, offsety, displayrect);
    texboard.WriteBegin();

    // Output markers for shape
    int mul = 1;
    for (int j = 0; j < m_ySize; ++j)
    {
        for (int i = 0; i < m_xSize; ++i)
        {
            int shapevalue = (shapeindex / mul) % 3;
            mul *= 3;

            switch (shapevalue)
            {
            case eBlack:
                texboard.WriteBlack(x + i, y + j);
                break;
            case eWhite:
                texboard.WriteWhite(x + i, y + j);
                break;
            case eEmpty:
                texboard.WriteEmpty(x + i, y + j);
                break;
            default:
                texboard.WriteEmptyCross(x + i, y + j);
                break;
            }
        }
    }

    texboard.WriteEnd();
    */
}

void RlLocalShapeFeatures::DisplayFeature(
    int featureindex, ostream& cmd) const
{
    int shapeindex, anchorindex, x, y;
    DecodeIndex(featureindex, shapeindex, anchorindex, x, y);
    
    int mul = 1;
    for (int j = 0; j < m_ySize; ++j)
    {
        for (int i = 0; i < m_xSize; ++i)
        {
            SgPoint pt = Pt(x + i + 1, y + j + 1);
            int shapevalue = (shapeindex / mul) % 3;
            mul *= 3;

            cmd << "SQUARE " << SgWritePoint(pt) << "\n";
            switch (shapevalue)
            {
            case eBlack:
                cmd << "BLACK " << SgWritePoint(pt) << "\n";
                break;
            case eWhite:
                cmd << "WHITE " << SgWritePoint(pt) << "\n";
                break;
            case eEmpty:
                break;
            default:
                cmd << "MARK " << SgWritePoint(pt) << "\n";
                break;
            }
        }
    }
}

void RlLocalShapeFeatures::DescribeSet(ostream& name) const
{
    // Single word description (no whitespace)
    name << "LocalShape-" << m_xSize << "x" << m_ySize;
}

int RlLocalShapeFeatures::GetShapeIndex(istream& desc) const
{
    // e.g. "-XOO-O.." is the 3x2 shape:
    //   XOO
    //   O..
    // shape contains m_xSize * m_ySize points, 
    // read from left to right, top to bottom,
    // with each row prefixed by a hyphen.

    char c;
    int shapeindex = 0;
    int mul = 1;
    for (int j = 0; j < m_ySize; ++j)
    {
        desc >> c;
        SG_ASSERT(c == '-');

        for (int i = 0; i < m_xSize; ++i)
        {
            desc >> c;
            
            int a = GetChColour(c);
            SG_ASSERT(a >= 0);

            shapeindex += mul * a;
            mul *= 3;
        }
    }

    return shapeindex;
}

int RlLocalShapeFeatures::Translate(
    int featureindex, int newx, int newy) const
{
    int shapeindex, oldanchorindex, newanchorindex, oldx, oldy;
    DecodeIndex(featureindex, shapeindex, oldanchorindex, oldx, oldy);
    newanchorindex = GetAnchorIndex(newx, newy);
    return EncodeIndex(shapeindex, newanchorindex);
}

int RlLocalShapeFeatures::Transform(int featureindex,
    bool flipx, bool flipy, bool transpose) const
{
    int oldshapeindex, newshapeindex;
    int oldanchorindex, newanchorindex;
    int x, y;

    DecodeIndex(featureindex, oldshapeindex, oldanchorindex, x, y);
    
    // Get shape index for transformed shape
    RlLocalShape shape(m_xSize, m_ySize);
    shape.SetShapeIndex(oldshapeindex);
    if (flipx)
        shape.FlipX();
    if (flipy)
        shape.FlipY();
    if (transpose)
        shape.Transpose();
    newshapeindex = shape.GetShapeIndex();

    // Get anchor index for transformed anchor position
    if (flipx)
        x = m_board.Size() - x - (transpose ? m_ySize : m_xSize);
    if (flipy)
        y = m_board.Size() - y - (transpose ? m_xSize : m_ySize);

    if (transpose)
        newanchorindex = GetAnchorIndex(y, x);
    else
        newanchorindex = GetAnchorIndex(x, y);
    
    return EncodeIndex(newshapeindex, newanchorindex);
}

int RlLocalShapeFeatures::Invert(int featureindex) const
{
    int oldshapeindex, newshapeindex, anchorindex, x, y;
    DecodeIndex(featureindex, oldshapeindex, anchorindex, x, y);

    // Get shape index for colour inverted shape
    RlLocalShape shape(m_xSize, m_ySize);
    shape.SetShapeIndex(oldshapeindex);
    shape.Invert();
    newshapeindex = shape.GetShapeIndex();
    
    return EncodeIndex(newshapeindex, anchorindex);
}

int RlLocalShapeFeatures::Transpose(int featureindex) const
{
    int oldshapeindex, newshapeindex, anchorindex, x, y;
    DecodeIndex(featureindex, oldshapeindex, anchorindex, x, y);

    // Get shape index for transposed shape
    RlLocalShape shape(m_xSize, m_ySize);
    shape.SetShapeIndex(oldshapeindex);
    shape.Transpose();
    newshapeindex = shape.GetShapeIndex();
    
    return EncodeIndex(newshapeindex, anchorindex);
}

int RlLocalShapeFeatures::LocalMove(
    int featureindex, int mx, int my, int sh) const
{
    int oldshapeindex, newshapeindex, anchorindex, x, y;
    DecodeIndex(featureindex, oldshapeindex, anchorindex, x, y);

    // Get shape index after local move
    RlLocalShape shape(m_xSize, m_ySize);
    shape.SetShapeIndex(oldshapeindex);
    if ((shape.GetPoint(mx, my) == eEmpty && sh == eEmpty)
        || (shape.GetPoint(mx, my) != eEmpty && sh != eEmpty))
        return -1;
        
    shape.GetPoint(mx, my) = sh;
    newshapeindex = shape.GetShapeIndex();
    return EncodeIndex(newshapeindex, anchorindex);
}

bool RlLocalShapeFeatures::IsEmpty(int featureindex) const
{
    int shapeindex, anchorindex, x, y;
    DecodeIndex(featureindex, shapeindex, anchorindex, x, y);
    return shapeindex == 0;
}

SgRect RlLocalShapeFeatures::GetBounds(int featureindex) const
{
    int shapeindex, anchorindex, x, y;
    DecodeIndex(featureindex, shapeindex, anchorindex, x, y);

    RlLocalShape shape(m_xSize, m_ySize);
    shape.SetShapeIndex(shapeindex);
    return shape.GetBounds();
}

SgPoint RlLocalShapeFeatures::GetPosition(int featureindex) const
{
    int shapeindex, anchorindex, x, y;
    DecodeIndex(featureindex, shapeindex, anchorindex, x, y);
    return Pt(x + 1, y + 1);
}

bool RlLocalShapeFeatures::Touches(int featureindex, SgPoint point) const
{
    // @todo use GetBounds
    int px = Col(point), py = Row(point);
    int shapeindex, anchorindex, x, y;
    DecodeIndex(featureindex, shapeindex, anchorindex, x, y);
    return (px >= x && py >= y && px < x + m_xSize && py < y + m_ySize);
}

void RlLocalShapeFeatures::CollectPoints(int featureindex,
    vector<SgPoint>& points) const
{
    int shapeindex, anchorindex, x, y;
    DecodeIndex(featureindex, shapeindex, anchorindex, x, y);
    for (int j = 0; j < m_ySize; ++j)
        for (int i = 0; i < m_xSize; ++i)
            points.push_back(Pt(x + i + 1, y + j + 1));
}

void RlLocalShapeFeatures::GetPage(int pagenum, vector<int>& indices, 
    ostream& pagename) const
{
    SG_ASSERT(pagenum >= 0 && pagenum < m_numShapes);
    indices.clear();
    for (int y = 0; y < m_board.Size(); ++y)
    {
        for (int x = 0; x < m_board.Size(); ++x)
        {
            if (x < m_xNum && y < m_yNum)
            {
                int anchorindex = GetAnchorIndex(x, y);
                int featureindex = EncodeIndex(pagenum, anchorindex);
                indices.push_back(featureindex);
            }
            else
            {
                indices.push_back(-1);
            }
        }
    }
    
    RlLocalShape localshape(m_xSize, m_ySize);
    localshape.SetShapeIndex(pagenum);
    localshape.DescribeShape(pagename);
}

int RlLocalShapeFeatures::GetNumPages() const
{
    return m_numShapes;
}

//ANNA
void RlLocalShapeFeatures::GetFeaturesAt(SgPoint p, std::vector<int>& indices)
{
    int curx, cury, minx, miny;
    indices.clear();
    // set up the minimal possible anchor point
    miny=Row(p)-1;
    minx=Col(p)-1;
    // for every possible anchor point of a shape that will touch this point
    // checking that we don't pass illegal anchor points
    for (curx = minx; curx < minx + GetXSize() && curx < GetXNum(); ++curx)
    {
        for (cury = miny; 
            cury < miny + GetYSize() && cury < GetYNum(); ++cury )
        {
          int anchorindex = GetAnchorIndex(curx, cury);
            // loop over all possible features at this point
            for (int featureindex=0; featureindex < m_numShapes; 
                ++featureindex) 
            {
                indices.push_back(EncodeIndex(featureindex, anchorindex));
            }
        }
    }
}
                


//----------------------------------------------------------------------------
