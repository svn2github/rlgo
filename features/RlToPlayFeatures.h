//----------------------------------------------------------------------------
/** @file RlToPlayFeatures.h
    Features for the stage of the game
*/
//----------------------------------------------------------------------------

#ifndef RLTOPLAYFEATURES_H
#define RLTOPLAYFEATURES_H

#include "RlBinaryFeatures.h"
#include "RlTracker.h"

//----------------------------------------------------------------------------
/** Two mutually exclusive binary features specifying colour to play
        Feature 0: black to play
        Feature 1: white to play */
class RlToPlayFeatures : public RlBinaryFeatures
{
public:

    DECLARE_OBJECT(RlToPlayFeatures);

    RlToPlayFeatures(GoBoard& board);
    
    /** Create corresponding object for incremental tracking */
    virtual RlTracker* CreateTracker(
        std::map<RlBinaryFeatures*, RlTracker*>& trackermap);

    /** Get the total number of features currently in this set */
    virtual int GetNumFeatures() const;

    /** Read a feature from stream (see implementation for spec) */
    virtual int ReadFeature(std::istream& desc) const;

    /** Describe a feature in text form */
    virtual void DescribeFeature(int featureindex, std::ostream& str) const;

    /** Single word description of feature set */
    virtual void DescribeSet(std::ostream& str) const;
};

class RlToPlayTracker : public RlTracker
{
public:
    
    RlToPlayTracker(GoBoard& board);

    /** Reset to current board position */
    virtual void Reset();
    
    /** Incremental execute */
    virtual void Execute(SgMove move, SgBlackWhite colour, 
        bool execute, bool store);

    /** Incremental undo */
    virtual void Undo();

    /** Size of active set */
    virtual int GetActiveSize() const;    
};

//----------------------------------------------------------------------------

#endif // RLTOPLAYFEATURES_H
