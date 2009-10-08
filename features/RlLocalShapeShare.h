//----------------------------------------------------------------------------
/** @file RlLocalShapeShare.h
    Feature sharing for local shapes
*/
//----------------------------------------------------------------------------

#ifndef RLLOCALSHAPESHARE_H
#define RLLOCALSHAPESHARE_H

//----------------------------------------------------------------------------

#include "RlSharedFeatures.h"

class RlLocalShapeFeatures;

//----------------------------------------------------------------------------
/** Feature sharing for local shapes */
class RlLocalShapeShare : public RlSharedFeatures
{
public:

    DECLARE_OBJECT(RlLocalShapeShare);

    RlLocalShapeShare(GoBoard& board, 
        RlLocalShapeFeatures* localshapes = 0, 
        bool invert = true);

    /** Load settings from specified file */
    virtual void LoadSettings(std::istream& settings);

    virtual void Initialise();
    virtual bool IgnoreFeature(int featureindex) const;

    /** Set name */
    virtual void DescribeSet(std::ostream& name) const;

    /** Get a page of features */
    virtual void GetPage(int pagenum, std::vector<int>& indices, 
        std::ostream& pagename) const;

    /** Get number of feature pages */
    virtual int GetNumPages() const;

    RlLocalShapeFeatures* GetShapeFeatures() { return m_localShapes; }

    void IgnoreEmpty(bool ignoreempty) { m_ignoreEmpty = ignoreempty; }

protected:

    RlLocalShapeFeatures* m_localShapes;
    bool m_flipX, m_flipY, m_transpose, m_invert;
    bool m_ignoreEmpty;
};

class RlLDFeatureShare : public RlLocalShapeShare
{
public:

    DECLARE_OBJECT(RlLDFeatureShare);

    RlLDFeatureShare(GoBoard& board, 
        RlLocalShapeFeatures* localshapes = 0,
        bool invert = true)
        : RlLocalShapeShare(board, localshapes, invert) {}

    /** Equivalence class contains rotation reflection and colour inversion */
    virtual void GetEquivalent(int featureindex, 
        std::vector<int>& equivalent,
        std::vector<int>& signs) const;    

    /** Describe the shape of a feature using LD display mode */
    virtual void DescribeTex(int featureindex, std::ostream& tex, 
        bool invert) const;

    /** Set name */
    virtual void DescribeSet(std::ostream& name) const;
    
    virtual int GetSymmetry() const { return RlShapeUtil::eLD; }
};

class RlLIFeatureShare : public RlLocalShapeShare
{
public:

    DECLARE_OBJECT(RlLIFeatureShare);

    RlLIFeatureShare(GoBoard& board, 
        RlLocalShapeFeatures* localshapes = 0,
        bool invert = true)
        : RlLocalShapeShare(board, localshapes, invert) {}

    /** Equivalence class contains rotation, reflection and colour inversion
        and also translation. */
    virtual void GetEquivalent(int featureindex, 
        std::vector<int>& equivalent,
        std::vector<int>& signs) const;

    /** Describe the shape of a feature using LI display mode */
    virtual void DescribeTex(int featureindex, std::ostream& tex, 
        bool invert) const;

    /** Set name */
    virtual void DescribeSet(std::ostream& name) const;

    virtual int GetSymmetry() const { return RlShapeUtil::eLI; }
};

class RlCIFeatureShare : public RlLocalShapeShare
{
public:

    DECLARE_OBJECT(RlCIFeatureShare);

    RlCIFeatureShare(GoBoard& board, RlLocalShapeFeatures* localshapes = 0)
        : RlLocalShapeShare(board, localshapes, true) {}

    /** Equivalence class contains colour inversion only */
    virtual void GetEquivalent(int featureindex, 
        std::vector<int>& equivalent,
        std::vector<int>& signs) const;

    /** Describe the shape of a feature using normal display mode */
    virtual void DescribeTex(int featureindex, std::ostream& tex,
        bool invert) const;

    /** Set name */
    virtual void DescribeSet(std::ostream& name) const;

    virtual int GetSymmetry() const { return RlShapeUtil::eCI; }
};

//----------------------------------------------------------------------------

#endif // RLLOCALSHAPESHARE_H
