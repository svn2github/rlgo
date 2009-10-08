//----------------------------------------------------------------------------
/** @file RlShapeUtil.cpp
*/
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "RlShapeUtil.h"

#include "GoBoard.h"

using namespace std;
using namespace SgPointUtil;

//----------------------------------------------------------------------------

namespace RlShapeUtil
{

int GetBWColour(char ch)
{
    char uch = toupper(ch);
    int c = (uch == 'B' ? eBlack
        : uch == 'W' ? eWhite
        : uch == 'E' ? eEmpty
        : eError);
    SG_ASSERT(c != eError);
    return c;
}

int GetChColour(char c)
{
    int a = (c == 'X' ? eBlack 
       : c == 'O' ? eWhite 
       : c == '.' ? eEmpty
       : eError);

    SG_ASSERT(a != eError);
    return a;
}

char ToChColour(int c)
{
    SG_ASSERT(c >= 0 && c <= 2);

    switch (c)
    {
    case eBlack:
        return 'X';
    case eWhite:
        return 'O';
    case eEmpty:
        return '.';
    default:
        return '!';
    }
}

int GetSymmetryType(const std::string& symmetry)
{
    if (symmetry == "None")
        return eNone;
    if (symmetry == "LD")
        return eLD;
    if (symmetry == "LI")
        return eLI;
    if (symmetry == "CI")
        return eCI;
    if (symmetry == "NLD")
        return eNLD;
    if (symmetry == "NLI")
        return eNLI;

    SG_ASSERT(false);
    return 0;
}

int GetShapeSpec(const std::string& shapespec)
{
    if (shapespec == "ALL")
        return eAll;
    if (shapespec == "RECT")
        return eRect;
    if (shapespec == "SQUARE")
        return eSquare;
    if (shapespec == "MAX")
        return eMax;
        
    SG_ASSERT(false);
    return 0;
}

SgHashCode ReadBoardHash(istream& desc, int size)
{
    // e.g. "B-XOO-O..-.O." is the 3x3 board with Black to play:
    //   XOO
    //   O..
    //   .O.
    // read from left to right, top to bottom,
    // with each row prefixed by a hyphen.

    char c;
    desc >> c;

    GoBoard bd(size);
    bd.SetToPlay(GetBWColour(c));

    abort(); // TODO: Update. GoBoard::AddStone does not exists anymore
#if 0
    for (int j = size; j >= 1; --j)
    {
        desc >> c;
        ASSERT(c == '-');

        for (int i = 1; i <= size; ++i)
        {
            desc >> c;
            if (c == 'X')
                bd.AddStone(Pt(i, j), SG_BLACK);
            if (c == 'O')
                bd.AddStone(Pt(i, j), SG_WHITE);            
        }
    }
#endif

    return bd.GetHashCodeInclToPlay();
}

int MakeNumber(char x1, char x2)
{
    SG_ASSERT(x1 >= '0' && x1 <= '9');
    SG_ASSERT(x2 >= '0' && x2 <= '9');
    return (x1 - '0') * 10 + (x2 - '0');
}

int GetSize(int x, int y)
{
    return x * y;
}

} // namespace RlShapeUtil
