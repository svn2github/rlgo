//----------------------------------------------------------------------------
/** @file RlLocalShapeSet.h
    Set of local shape features and corresponding feature share objects
*/
//----------------------------------------------------------------------------

#ifndef RLLOCALSHAPESET_H
#define RLLOCALSHAPESET_H

#include "RlSumFeatures.h"

class RlLocalShapeFeatures;
class RlLocalShapeShare;

//----------------------------------------------------------------------------
/** Class holding a set of local shape features and corresponding
    feature share objects 
*/
class RlLocalShapeSet : public RlSumFeatures
{
public:

    DECLARE_OBJECT(RlLocalShapeSet);
    
    RlLocalShapeSet(GoBoard& board, 
        int minsize = 1, 
        int maxsize = 3,
        int shapespec = RlShapeUtil::eSquare, 
        int sharetypes = (1 << RlShapeUtil::eNone));
    ~RlLocalShapeSet();
        
    virtual void LoadSettings(std::istream& settings);
    virtual void Initialise();

    int GetMinSize() const { return m_minSize; }
    int GetMaxSize() const { return m_maxSize; }
    int GetShapeSpec() const { return m_shapeSpec; }
    int GetShareTypes() const { return m_shareTypes; }

    int GetNumShapeSets() const { return m_shapeSets.size(); }
    int GetNumShares(int set) const { return m_shapeSets[set].m_shares.size(); }
    RlLocalShapeFeatures* GetShapes(int set) { return m_shapeSets[set].m_shapes; }
    RlLocalShapeShare* GetShare(int set, int sh) { return m_shapeSets[set].m_shares[sh]; }

protected:

    /** Add shapes of specified type */
    void AddAnyShapes(int shapespec, int sharetypes);

    /** Add all shapes up to specified size */
    void AddAllShapes(int sharetypes);

    /** Add square shapes up to specified size */
    void AddSquareShapes(int sharetypes);

    /** Add rectangular shapes with |xsize-ysize| <= maxoffset */
    void AddRectShapes(int maxoffset, int sharetypes);
    
    /** Add shapes before initialisation */
    void AddShapes(int xsize, int ysize,
        int sharetypes);

private:

    /** Whether to use square, almost-square, or rectangular shapes */
    int m_shapeSpec;
    
    /** Flags for weight sharing types (LI, LD, etc.) */
    int m_shareTypes;

    /** The minimum and maximum value of x or y to consider for shapes */
    int m_minSize, m_maxSize;

    /** Whether to ignore empty shapes or existing subshapes */
    bool m_ignoreEmpty, m_ignoreSelfInverse;

    struct ShapeSet
    {
        ShapeSet(RlLocalShapeFeatures* shapes)
        :   m_shapes(shapes) { }
    
        RlLocalShapeFeatures* m_shapes;
        std::vector<RlLocalShapeShare*> m_shares;
    };
    std::vector<ShapeSet> m_shapeSets;

    int ReadShareTypes(const std::vector<std::string>& types);
    void AddShares(RlLocalShapeShare* shares, ShapeSet& shapeset);
};


//----------------------------------------------------------------------------

#endif // RLLOCALSHAPESET_H

