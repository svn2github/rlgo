//----------------------------------------------------------------------------
/** @file RlLocalShapeFeatures.h
    Features based on local, rectangular patterns of stones
*/
//----------------------------------------------------------------------------

#ifndef RLLOCALSHAPEFEATURES_H
#define RLLOCALSHAPEFEATURES_H

#include "RlBinaryFeatures.h"

//----------------------------------------------------------------------------
/** Features based on local, rectangular patterns of stones */
class RlLocalShapeFeatures : public RlBinaryFeatures
{
public:

    DECLARE_OBJECT(RlLocalShapeFeatures);

    RlLocalShapeFeatures(GoBoard& board, int xsize = 1, int ysize = 1);

    /** Load in the settings for this feature set */
    virtual void LoadSettings(std::istream& settings);

    /** Initialise */
    virtual void Initialise();

    /** Create corresponding object for incremental tracking */
    virtual RlTracker* CreateTracker(
        std::map<RlBinaryFeatures*, RlTracker*>& trackermap);

    /** Total number of features */
    virtual int GetNumFeatures() const { return m_numFeatures; }

    /** Get a feature by description (see implementation for spec) */
    virtual int ReadFeature(std::istream& desc) const;

    /** Describe the shape of a feature concisely */
    virtual void DescribeFeature(int featureindex, std::ostream& str) const;

    /** Describe the shape of a feature in LaTex using default display mode 
        (display whole board) */
    virtual void DescribeTex(int featureindex, std::ostream& str,
        bool invert) const;

    /** Display the shape of the feature in GoGui */
    virtual void DisplayFeature(int featureindex, std::ostream& cmd) const;

    /** Short description of the feature set (single word, no whitespace) */
    virtual void DescribeSet(std::ostream& name) const;

    /** Anchor position for this feature */
    virtual int GetPosition(int featureindex) const;

    /** Get a page of features */
    virtual void GetPage(int pagenum, std::vector<int>& indices, 
        std::ostream& pagename) const;

    /** Get number of feature pages */
    virtual int GetNumPages() const;
    
    /** Check whether a feature touches the specified point */
    virtual bool Touches(int featureindex, SgPoint point) const;

    /** Collect all points touching this feature */
    virtual void CollectPoints(int featureindex, 
        std::vector<SgPoint>& points) const;

    /** Rotate/reflect feature to new position */
    int Transform(int featureindex, 
        bool flipx, bool flipy, bool transpose) const;
    
    /** Invert all colours in the shape */
    int Invert(int featureindex) const;
    
    /** Tranpose local shape */
    int Transpose(int featureindex) const;

    /** Translate feature to new position */
    int Translate(int featureindex, int x, int y) const;
    
    /** Make a local move in the shape */
    int LocalMove(int featureindex, int x, int y, int c) const;
        
    int GetXSize() const { return m_xSize; }
    int GetYSize() const { return m_ySize; }
    int GetXNum() const { return m_xNum; }
    int GetYNum() const { return m_yNum; }
    int GetNumShapes() const { return m_numShapes; }
    
    bool IsEmpty(int featureindex) const;
    SgRect GetBounds(int featureindex) const;
    
    bool Square() const
    {
        return m_xSize == m_ySize;
    }
    
    // Display mode used by text and LaTex shape descriptions
    void SetDisplayMode(int displaymode) { m_displayMode = displaymode; }
    
public: // was protected but useful for RlLocalShapeFusion

    /** Get index for a given anchor position. */
    int GetAnchorIndex(int x, int y) const { 
        SG_ASSERT(x>=0 && x<m_xNum && y>=0 && y<m_yNum);
        return y * m_xNum + x; 
    }

    /** Decompose index into position and shape components */
    void DecodeIndex(
        int featureindex,
        int& shapeindex, int& anchorindex,
        int& x, int& y) const
    {
        shapeindex = featureindex % m_numShapes;
        anchorindex = featureindex / m_numShapes;
        x = anchorindex % m_xNum;
        y = anchorindex / m_xNum;
    }

    /** Compose index from position and shape components */
    int EncodeIndex(int shapeindex, int anchorindex) const
    {
        SG_ASSERT(shapeindex>=0 && shapeindex<m_numShapes);
        SG_ASSERT(anchorindex>=0 && anchorindex<m_xNum*m_yNum);
        return anchorindex * m_numShapes + shapeindex;
    }
    
    /** Generate a list of all the shape features that touch point p */
    void GetFeaturesAt(SgPoint p, std::vector<int>& indices);
    
protected:
    
    /** Calculate shape index from text description */
    int GetShapeIndex(std::istream& desc) const;

    /** Describe a shape feature in its board context (single line form) */
    void DescribeFeatureShort(
        int shapeindex, 
        int x, int y, 
        std::ostream& str) const;

    /** Describe a shape feature in its board context (multi-line form) */
    void DescribeFeatureLong(
        int shapeindex, 
        int x, int y, 
        std::ostream& str) const;

    /** Describe a shape feature in Tex format */
    void DescribeFeatureTex(
        int shapeindex, 
        int x, int y, 
        std::ostream& tex) const;
    
    /** Calculate partial rectangle to display */
    SgRect DisplayRect(int x, int y, 
        int& offsetx, int& offsety) const;

    /** Check whether a point lies within the region of a local shape */
    bool InLocalShape(int x, int y, int i, int j) const
    {
        return i >= x && i < x + m_xSize && j >= y && j < y + m_ySize;
    }
    
protected:

    /** Shape dimensions */
    int m_xSize, m_ySize;
    int m_xNum, m_yNum;
    int m_numShapes, m_numFeatures;
    int m_displayMode;
};

//----------------------------------------------------------------------------

#endif // RLLOCALSHAPEFEATURES_H
