//----------------------------------------------------------------------------
/** @file RlTex.h
    Utility functions for writing out boards in .tex format
*/
//----------------------------------------------------------------------------

#ifndef RLTEX_H
#define RLTEX_H

#include "SgRect.h"
#include "RlMiscUtil.h"

#include <fstream>
#include <vector>

class RlBinaryFeatures;
class RlWeight;
class RlWeightSet;

//----------------------------------------------------------------------------
/** Class for writing out tables of features into a .tex file
*/
class RlTexTable
{
public:

    /** Constructor writes out tex header */
    RlTexTable(std::ostream& tex, int cols);

    /** Destructor writes out tex footer */
    ~RlTexTable();

    /** Largest absolute weights (xnum*ynum) in all sets 
        Sections will write sectioned table with set names 
        Adds offset to feature indices when getting weights from wset
    */
    void TopFeatures(const RlWeightSet* wset, const RlBinaryFeatures* fset, 
        int rows, bool sections, int offset = 0);
        
    void NextRow();
    void NextColumn();
    void StartTable(bool sections);
    void EndTable();
    void WriteWeight(RlFloat weight);
    void Line();
    void Section(const std::string& name);

private:

    struct WPair
    {
        WPair(RlFloat weight, int index)
        :   m_weight(weight), m_index(index) { }
        
        RlFloat m_weight;
        int m_index;
    };
    
    struct RlWSort
    {
        bool operator()(const WPair& lhs, const WPair& rhs) const;
    };

    std::ostream& m_tex;
    int m_cols;
    int m_precision;
    bool m_invert; // Invert features with -ve weight
};

//----------------------------------------------------------------------------
/** A class for writing out board patches */
class RlTexBoard
{
public:

    RlTexBoard(int boardsize, std::ostream& tex, 
        int offsetx = 0, int offsety = 0, const SgRect& partial = SgRect());

    void GetPatchCentre(int& centrex, int& centrey) const;

    void WriteComment(const std::string& comment);
    void WriteBegin();
    void WriteEnd();
    void WriteCoords(int x, int y);
    void WriteBlack(int x, int y);
    void WriteWhite(int x, int y);
    void WriteEmpty(int x, int y);
    void WriteBlackCount(int x, int y, int count);
    void WriteWhiteCount(int x, int y, int count);
    void WriteEmptyCount(int x, int y, int count);
    void WriteEmptyCross(int x, int y);

private:

    std::ostream& m_tex;
    
    bool m_begun;
    int m_boardSize;
    int m_offsetX, m_offsetY;
    SgRect m_region;
    bool m_hatched;
};

//----------------------------------------------------------------------------

#endif // RLTEX_H

