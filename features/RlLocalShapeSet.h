//----------------------------------------------------------------------------
/** @file RlLocalShapeSet.h
    Set of local shape features and corresponding feature share objects
*/
//----------------------------------------------------------------------------

#ifndef RLLOCALSHAPESET_H
#define RLLOCALSHAPESET_H

#include "RlSumFeatures.h"

class RlLocalShapeShare;

//----------------------------------------------------------------------------
/** Class holding a set of local shape features and corresponding
    feature share objects 
*/
class RlLocalShapeSet : public RlSumFeatures
{
public:

    DECLARE_OBJECT(RlLocalShapeSet);
    
    RlLocalShapeSet(GoBoard& board);
        
    virtual void LoadSettings(std::istream& settings);

    /** Add shapes of specified type */
    void AddAnyShapes(const std::string& shapespec, int sharetypes);

    /** Add all shapes up to specified size */
    void AddAllShapes(int sharetypes);

    /** Add square shapes up to specified size */
    void AddSquareShapes(int sharetypes);

    /** Add rectangular shapes with |xsize-ysize| <= maxoffset */
    void AddRectShapes(int maxoffset, int sharetypes);
    
    /** Add shapes before initialisation */
    void AddShapes(int xsize, int ysize,
        int sharetypes);

    void SetShapeSize(int minsize, int maxsize)
    {
        m_minSize = minsize;
        m_maxSize = maxsize;
    }

protected:

    int GetShareType(const std::vector<std::string>& types);
    void AddShares(RlLocalShapeShare* shares);
        
private:

    /** Whether to use square, almost-square, or rectangular shapes */
    int m_shapeSpec;
    
    /** The minimum and maximum value of x or y to consider for shapes */
    int m_minSize, m_maxSize;

    /** Whether symmetry should be used for hashed features */
    bool m_symmetry;
    
    /** Whether to ignore empty shapes or existing subshapes */
    bool m_ignoreEmpty, m_ignoreSelfInverse;
};


//----------------------------------------------------------------------------

#endif // RLLOCALSHAPESET_H

