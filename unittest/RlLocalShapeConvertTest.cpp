//----------------------------------------------------------------------------
/** @file RlLocalShapeConvertTest.cpp
    Unit tests for RlLocalShapeConvert
*/
//----------------------------------------------------------------------------

#include "SgSystem.h"

#include <boost/test/auto_unit_test.hpp>
#include "RlLocalShapeTracker.h"

#include <boost/test/floating_point_comparison.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/auto_unit_test.hpp>
#include "RlEvaluator.h"
#include "RlLocalShape.h"
#include "RlLocalShapeConvert.h"
#include "RlLocalShapeFeatures.h"
#include "RlLocalShapeSet.h"
#include "RlMoveFilter.h"
#include "RlUtils.h"
#include "RlWeightSet.h"
#include "RlTestUtil.h"

using namespace std;
using namespace SgPointUtil;
using namespace RlShapeUtil;
using namespace boost::test_tools;

//----------------------------------------------------------------------------

namespace {

// percentage tolerance for floating point comparison
const float tol = 0.001f; 

// All of the following tests are based on the shapes in this position:

// . . . . .
// . . O . .
// . O X O .
// . X . X X
// . . . . .

BOOST_AUTO_TEST_CASE(RlLocalShapeFusionTest)
{
    GoBoard bd(5);
    RlLocalShapeSet allshapes(bd, 1, 2);
    RlLocalShapeFeatures fusedshapes(bd, 2, 2);
    RlLocalShapeFusion fusion(bd, &allshapes, &fusedshapes);
    RlWeightSet allweights(bd, &allshapes);
    RlWeightSet fusedweights(bd, &fusedshapes);
    RlMoveFilter allmf(bd), fusedmf(bd);
    RlEvaluator allevaluator(bd, &allshapes, &allweights, &allmf);
    RlEvaluator fusedevaluator(bd, &fusedshapes, &fusedweights, &fusedmf);

    fusion.EnsureInitialised();
    allevaluator.EnsureInitialised();
    fusedevaluator.EnsureInitialised();
    
    // Randomise weights... except:
    // set all empty weights to zero to avoid issues with ignore empty
    allweights.RandomiseWeights(-1.0, +1.0);
    int shapeindex = 0, anchorindex, featureindex;
    for (int set = 0; set < 2; set++)
    {
        for (int x = 0; x < allshapes.GetShapes(set)->GetXNum(); x++)
        {
            for (int y = 0; y < allshapes.GetShapes(set)->GetYNum(); y++)
            {
                anchorindex = allshapes.GetShapes(set)->GetAnchorIndex(x, y);
                featureindex = allshapes.GetShapes(set)->EncodeIndex(shapeindex, anchorindex);
                allweights.Get(featureindex).Weight() = 0;
            }
        }
    }
        
    fusedweights.ZeroWeights();
    fusion.Convert(&allweights, &fusedweights);
    
    bd.Init(5);
    bd.Play(Pt(3, 1), SG_BLACK);
    bd.Play(Pt(2, 3), SG_WHITE);
    bd.Play(Pt(2, 2), SG_BLACK);
    bd.Play(Pt(3, 4), SG_WHITE);
    bd.Play(Pt(4, 2), SG_BLACK);
    bd.Play(Pt(4, 3), SG_WHITE);
    bd.Play(Pt(5, 2), SG_BLACK);
    
    allevaluator.Reset();
    fusedevaluator.Reset();

    BOOST_CHECK_CLOSE(allevaluator.Eval(), fusedevaluator.Eval(), tol);
}

BOOST_AUTO_TEST_CASE(RlLocalShapeUnshareTest)
{
    GoBoard bd(5);
    RlLocalShapeSet sharedshapes(bd, 1, 2, eSquare, (1 << eLI) | (1 << eLD));
    RlLocalShapeSet unsharedshapes(bd, 1, 2, eSquare, (1 << eNone));
    RlLocalShapeUnshare unshare(bd, &sharedshapes, &unsharedshapes);
    RlWeightSet sharedweights(bd, &sharedshapes);
    RlWeightSet unsharedweights(bd, &unsharedshapes);
    RlMoveFilter sharedmf(bd), unsharedmf(bd);
    RlEvaluator sharedevaluator(bd, &sharedshapes, &sharedweights, &sharedmf);
    RlEvaluator unsharedevaluator(bd, &unsharedshapes, &unsharedweights, &unsharedmf);

    unshare.EnsureInitialised();
    sharedevaluator.EnsureInitialised();
    unsharedevaluator.EnsureInitialised();
    
    // Randomise weights... except:
    // set shared empty weights to zero to avoid issues with ignore empty
    sharedweights.RandomiseWeights(-1.0, +1.0);
    int shapeindex = 0, anchorindex, featureindex;
    for (int set = 0; set < 2; set++)
    {
        for (int x = 0; x < sharedshapes.GetShapes(set)->GetXNum(); x++)
        {
            for (int y = 0; y < sharedshapes.GetShapes(set)->GetYNum(); y++)
            {
                anchorindex = sharedshapes.GetShapes(set)->GetAnchorIndex(x, y);
                featureindex = sharedshapes.GetShapes(set)->EncodeIndex(shapeindex, anchorindex);
                sharedweights.Get(featureindex).Weight() = 0;
            }
        }
    }

    unsharedweights.ZeroWeights();
    unshare.Convert(&sharedweights, &unsharedweights);
    
    bd.Init(5);
    bd.Play(Pt(3, 1), SG_BLACK);
    bd.Play(Pt(2, 3), SG_WHITE);
    bd.Play(Pt(2, 2), SG_BLACK);
    bd.Play(Pt(3, 4), SG_WHITE);
    bd.Play(Pt(4, 2), SG_BLACK);
    bd.Play(Pt(4, 3), SG_WHITE);
    bd.Play(Pt(5, 2), SG_BLACK);
    
    sharedevaluator.Reset();
    unsharedevaluator.Reset();

    BOOST_CHECK_CLOSE(sharedevaluator.Eval(), unsharedevaluator.Eval(), tol);
}

} // namespace