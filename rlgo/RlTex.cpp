//----------------------------------------------------------------------------
/** @file RlTex.cpp
    @see RlTex.h
*/
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "RlTex.h"

#include "RlBinaryFeatures.h"
#include "RlWeightSet.h"
#include <math.h>

using namespace std;

//----------------------------------------------------------------------------

RlTexTable::RlTexTable(ostream& tex, int cols)
:   m_tex(tex),
    m_cols(cols),
    m_precision(3),
    m_invert(false)
{
    m_tex << "\\documentclass{article}\n";
    m_tex << "\\usepackage{psgo}\n";
    m_tex << "\\setgounit{0.15cm}\n";
    m_tex << "\\begin{document}\n\n";
}

RlTexTable::~RlTexTable()
{
    m_tex << "\\end{document}\n";
}

void RlTexTable::StartTable(bool sections)
{
    m_tex << "\\begin{tabular}{";
    if (sections)
        m_tex << "l|";
    for (int i = 0; i < m_cols; ++i)
        m_tex << 'c';
    m_tex << "}\n";
}

void RlTexTable::EndTable()
{
    m_tex << "\\end{tabular}\n";
}

void RlTexTable::NextColumn()
{
    m_tex << "&\n";
}

void RlTexTable::NextRow()
{
    m_tex << "\\\\\n";
}

void RlTexTable::WriteWeight(RlFloat weight)
{
    m_tex << "{\\small " << weight << "}\n";
}

void RlTexTable::Line()
{
    m_tex << "\n\\hline\n";
}

void RlTexTable::Section(const string& name)
{
    m_tex << "\n" << name << "\n&\n";
}

void RlTexTable::TopFeatures(
    const RlWeightSet* wset, 
    const RlBinaryFeatures* fset,
    int rows, bool sections, int offset)
{
    // Get all weights
    vector<WPair> weights;
    for (int i = 0; i < fset->GetNumFeatures(); ++i)
        weights.push_back(
            WPair(
                wset->Get(i + offset).Weight(), 
                i));

    // Sort weights by absolute weight
    sort(weights.begin(), weights.end(), RlWSort());

    // Output top features to tex
    for (int y = 0; y < rows; ++y)
    {            
        if (y * m_cols >= ssize(weights))
            break;

        // Output row of shapes
        if (sections)
            Section(y == 0 ? fset->SetName() : "");            
        for (int x = 0; x < m_cols; ++x)
        {
            int index = y * m_cols + x;

            if (index < ssize(weights))
            {
                int featureindex = weights[index].m_index;
                bool invert = m_invert && weights[index].m_weight < 0;
                fset->DescribeTex(featureindex, m_tex, invert);
            }

            if (x < m_cols - 1)
                NextColumn();
        }
        
        NextRow();

        // Output row of weights
        if (sections)
            Section("");
            
        for (int x = 0; x < m_cols; ++x)
        {
            int index = y * m_cols + x;

            if (index < ssize(weights))
            {
                m_tex.precision(m_precision);
                if (m_invert && weights[index].m_weight < 0)
                    WriteWeight(-weights[index].m_weight);
                else
                    WriteWeight(weights[index].m_weight);
            }

            if (x < m_cols - 1)
                NextColumn();
        }
        
        NextRow();
    }    
}

bool RlTexTable::RlWSort::operator()
    (const WPair& lhs, const WPair& rhs) const
{
    return fabs(lhs.m_weight) > fabs(rhs.m_weight);
}

//----------------------------------------------------------------------------

RlTexBoard::RlTexBoard(int boardsize, ostream& tex, 
    int offsetx, int offsety, const SgRect& partial)
:   m_tex(tex),
    m_begun(false),
    m_boardSize(boardsize),
    m_offsetX(offsetx),
    m_offsetY(offsety),
    m_region(partial),
    m_hatched(false)
{
}

void RlTexBoard::WriteComment(const string& comment)
{
    m_tex << comment << "\n";
}

void RlTexBoard::WriteBegin()
{
    if (m_region.IsEmpty())
    {
        m_tex << "\\begin{psgoboard*}[" << m_boardSize << "]\n";
    }
    else
    {
        m_tex << "\\begin{psgopartialboard*}[" << m_boardSize << "]{(" 
            << m_region.Left() << "," << m_region.Top() << ")("
            << m_region.Right() << "," << m_region.Bottom() << ")}\n";
    }
    
    m_begun = true;
}

void RlTexBoard::WriteEnd()
{
    if (m_region.IsEmpty())
    {
        m_tex << "\\end{psgoboard*}" << "\n";
    }
    else
    {
        m_tex << "\\end{psgopartialboard*}" << "\n";
    }
    
    m_begun = false;
}

void RlTexBoard::WriteCoords(int x, int y)
{
    SG_ASSERT(m_begun);

    char xcoord = 'a' + m_offsetX + x;
    if (xcoord >= 'i')
        xcoord++;
    int ycoord = m_offsetY + y + 1;

    m_tex << "{" << xcoord << "}{" << ycoord << "}";
}

void RlTexBoard::WriteBlack(int x, int y)
{
    m_tex << "\\stone{black}";
    WriteCoords(x, y);
    m_tex << "\n";
}

void RlTexBoard::WriteWhite(int x, int y)
{
    m_tex << "\\stone{white}";
    WriteCoords(x, y);
    m_tex << "\n";
}

void RlTexBoard::WriteEmpty(int x, int y)
{
    if (m_hatched)
    {
        m_tex << "\\markpos{\\markdd}";
        WriteCoords(x, y);
        m_tex << "\n";
    }
}

void RlTexBoard::WriteBlackCount(int x, int y, int count)
{
    m_tex << "\\stone[" << count << "]{black}";
    WriteCoords(x, y);
    m_tex << "\n";
}

void RlTexBoard::WriteWhiteCount(int x, int y, int count)
{
    m_tex << "\\stone[" << count << "]{white}";
    WriteCoords(x, y);
    m_tex << "\n";
}

void RlTexBoard::WriteEmptyCount(int x, int y, int count)
{
    m_tex << "\\markpos{" << count << "}";
    WriteCoords(x, y);
    m_tex << "\n";
}

void RlTexBoard::WriteEmptyCross(int x, int y)
{
    m_tex << "\\markpos{\\markma}";
    WriteCoords(x, y);
    m_tex << "\n";
}

//----------------------------------------------------------------------------

