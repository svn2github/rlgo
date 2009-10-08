//----------------------------------------------------------------------------
/** @file RlShapeUtil.h
    Simple shape utility functions
*/
//----------------------------------------------------------------------------

#ifndef RLSHAPEUTIL_H
#define RLSHAPEUTIL_H

#include "SgBoardColor.h"
#include "SgHash.h"
#include "SgPoint.h"

//----------------------------------------------------------------------------

namespace RlShapeUtil
{

/** Symmetry type */
enum
{ 
    eNone, 
    eLD,  // location dependent
    eLI,  // location independent
    eCI,  // colour independent
    eNLD, // location dependent (no colour inversion)
    eNLI, // location independent (no colour inversion)
    eNumSymmetries
};

/** Shape spec */
enum
{
    eAll,
    eRect,
    eSquare,
    eMax
};

/** Colour indices */
enum 
{ 
    eEmpty, 
    eBlack, 
    eWhite, 
    eError = -1
};

/** Convert SgPoint to a point index between 0 and size*size */
int PointIndex(SgPoint point);

/** Get number of points (size*size) */
int NumPoints();

/** Convert point index to an SgPoint */
int GetSgPoint(int p);

/** Convert colour to a colour index */
int ColourIndex(SgEmptyBlackWhite colour);

/** Convert colour to 0 for black, 1 for white */
int BWIndex(SgBlackWhite colour);

/** Convert a character in {'B','W','E'} to a colour index */
int GetBWColour(char c);

/** Convert a character in {'X','O','.'} to a colour index */
int GetChColour(char c);

/** Get character representing a colour index */
char ToChColour(int sh);

/** Invert black/white in colour index */
int InvertColourIndex(int c);

/** Get symmetry type from string */
int GetSymmetryType(const std::string& symmetry);

/** Get shape spec from string */
int GetShapeSpec(const std::string& shapespec);

/** Get a scalar indication of size */
int GetSize(int xsize, int ysize);

/** Convert two characters to a number */
int MakeNumber(char x1, char x2);

/** Read a board position and return its hash code */
SgHashCode ReadBoardHash(std::istream& desc, int size);

//----------------------------------------------------------------------------

inline int InvertColourIndex(int c)
{
    static int invert[3] =
    {
        eEmpty,
        eWhite,
        eBlack
    };
    
    SG_ASSERT(c >= 0 && c <= 2);
    return invert[c];
}

inline int ColourIndex(SgEmptyBlackWhite colour)
{
    // For historical reasons, RLGO uses Empty=0, Black=1, White=2
    // whereas SmartBoard uses Black=0, White=1, Empty=2
    static int indices[3] = 
    { 
        eBlack,
        eWhite,
        eEmpty
    };

    return indices[colour];
}

inline int BWIndex(SgBlackWhite colour)
{
    SG_DEBUG_ONLY(colour);
    SG_ASSERT(colour == 0 || colour == 1);
    return colour; 
}

//----------------------------------------------------------------------------

} // namespace RlShapeUtil

#endif // RLSHAPEUTIL_H

