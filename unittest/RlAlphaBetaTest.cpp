//----------------------------------------------------------------------------
/** @file RlAlphaBeta.cpp
    Unit tests for RlAlphaBeta
*/
//----------------------------------------------------------------------------

#include "SgSystem.h"

#include <boost/test/auto_unit_test.hpp>
#include "RlAlphaBeta.h"

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

double tol = 0.01;

BOOST_AUTO_TEST_CASE(RlKillerTest)
{
    RlKiller killer;
    killer.Init(3);
    BOOST_CHECK_EQUAL(killer.GetKiller(0), SG_NULLMOVE);
    BOOST_CHECK_EQUAL(killer.GetKiller(1), SG_NULLMOVE);
    BOOST_CHECK_EQUAL(killer.GetKiller(2), SG_NULLMOVE);
    killer.MarkKiller(Pt(1, 1));
    BOOST_CHECK_EQUAL(killer.GetKiller(0), Pt(1, 1));
    BOOST_CHECK_EQUAL(killer.GetKiller(1), SG_NULLMOVE);
    BOOST_CHECK_EQUAL(killer.GetKiller(2), SG_NULLMOVE);
    killer.MarkKiller(Pt(2, 2));
    BOOST_CHECK_EQUAL(killer.GetKiller(0), Pt(2, 2));
    BOOST_CHECK_EQUAL(killer.GetKiller(1), Pt(1, 1));
    BOOST_CHECK_EQUAL(killer.GetKiller(2), SG_NULLMOVE);
    killer.MarkKiller(Pt(3, 3));
    BOOST_CHECK_EQUAL(killer.GetKiller(0), Pt(3, 3));
    BOOST_CHECK_EQUAL(killer.GetKiller(1), Pt(2, 2));
    BOOST_CHECK_EQUAL(killer.GetKiller(2), Pt(1, 1));
    killer.MarkKiller(Pt(4, 4));
    BOOST_CHECK_EQUAL(killer.GetKiller(0), Pt(4, 4));
    BOOST_CHECK_EQUAL(killer.GetKiller(1), Pt(3, 3));
    BOOST_CHECK_EQUAL(killer.GetKiller(2), Pt(2, 2));
    killer.MarkKiller(Pt(3, 3));
    BOOST_CHECK_EQUAL(killer.GetKiller(0), Pt(3, 3));
    BOOST_CHECK_EQUAL(killer.GetKiller(1), Pt(4, 4));
    BOOST_CHECK_EQUAL(killer.GetKiller(2), Pt(2, 2));
    killer.MarkKiller(Pt(2, 2));
    BOOST_CHECK_EQUAL(killer.GetKiller(0), Pt(2, 2));
    BOOST_CHECK_EQUAL(killer.GetKiller(1), Pt(3, 3));
    BOOST_CHECK_EQUAL(killer.GetKiller(2), Pt(4, 4));
    killer.MarkKiller(Pt(1, 1));
    BOOST_CHECK_EQUAL(killer.GetKiller(0), Pt(1, 1));
    BOOST_CHECK_EQUAL(killer.GetKiller(1), Pt(2, 2));
    BOOST_CHECK_EQUAL(killer.GetKiller(2), Pt(3, 3));
}

void MakeWeights(GoBoard& bd, RlWeightSet& unsharedweights)
{
    RlLocalShapeSet sharedshapes(bd, 1, 1, eSquare, (1 << eLI));
    RlLocalShapeSet unsharedshapes(bd, 1, 1, eSquare, (1 << eNone));
    RlLocalShapeUnshare unshare(bd, &sharedshapes, &unsharedshapes);
    RlWeightSet sharedweights(bd, &sharedshapes);

    unshare.EnsureInitialised();
    sharedweights.EnsureInitialised();
    unsharedweights.EnsureInitialised();
    BOOST_CHECK_EQUAL(
        unsharedweights.GetNumFeatures(), 
        unsharedshapes.GetNumFeatures());
    BOOST_CHECK_EQUAL(sharedweights.GetNumFeatures(), 1);
    sharedweights.Get(0).Weight() = +1.0;
    unshare.Convert(&sharedweights, &unsharedweights);
}

RlFloat SearchValue(RlAlphaBeta& alphabeta, int depth, bool q, bool ladder)
{
    alphabeta.Clear();
    alphabeta.SetMaxDepth(depth);
    alphabeta.SetMaxTime(+RlInfinity);
    alphabeta.SetMaxPredictedTime(+RlInfinity);
    alphabeta.SetQuiescence(q);
    alphabeta.SetEnsureParity(true);
    alphabeta.SetReadLadders(ladder);
    alphabeta.SetMaxLadderDepth(5);
    alphabeta.SetEstimateTenukiValue(false);
    alphabeta.SetTenukiValue(1.0);

    vector<SgMove> pv;
    return alphabeta.Search(pv);
}

SgMove SearchMove(RlAlphaBeta& alphabeta, int depth, bool q, bool ladder)
{
    alphabeta.Clear();
    alphabeta.SetMaxDepth(depth);
    alphabeta.SetMaxTime(+RlInfinity);
    alphabeta.SetMaxPredictedTime(+RlInfinity);
    alphabeta.SetQuiescence(q);
    alphabeta.SetEnsureParity(true);
    alphabeta.SetReadLadders(ladder);
    alphabeta.SetMaxLadderDepth(5);
    alphabeta.SetEstimateTenukiValue(false);
    alphabeta.SetTenukiValue(1.0);

    vector<SgMove> pv;
    alphabeta.Search(pv);
    return pv.front();
}

void TestLadders(RlEvaluator& evaluator, RlAlphaBeta& alphabeta)
{    
    // . . . . .
    // . . . . .
    // . . X . .
    // . X O . .
    // . . . . .
    evaluator.Reset();
    evaluator.PlayExecute(Pt(3, 3), SG_BLACK, false);
    evaluator.PlayExecute(Pt(3, 2), SG_WHITE, false);
    evaluator.PlayExecute(Pt(2, 2), SG_BLACK, false);
    evaluator.PlayExecute(SG_PASS, SG_WHITE, false);

    BOOST_CHECK_CLOSE(SearchValue(alphabeta, 1, false, false), 2.0, tol);
    BOOST_CHECK_CLOSE(SearchValue(alphabeta, 1, true, false), 2.0, tol);
    BOOST_CHECK_CLOSE(SearchValue(alphabeta, 1, true, true), 3.0, tol);
    BOOST_CHECK_EQUAL(SearchMove(alphabeta, 1, true, true), Pt(4, 2));
    BOOST_CHECK_CLOSE(SearchValue(alphabeta, 2, false, false), 1.0, tol);
    BOOST_CHECK_CLOSE(SearchValue(alphabeta, 2, true, false), 1.0, tol);
    BOOST_CHECK_CLOSE(SearchValue(alphabeta, 2, true, true), 2.0, tol);
    BOOST_CHECK_EQUAL(SearchMove(alphabeta, 2, true, true), Pt(4, 2));
    BOOST_CHECK_CLOSE(SearchValue(alphabeta, 3, false, false), 2.0, tol);
    BOOST_CHECK_CLOSE(SearchValue(alphabeta, 3, true, false), 3.0, tol);
    BOOST_CHECK_EQUAL(SearchMove(alphabeta, 3, true, true), Pt(4, 2));
    BOOST_CHECK_CLOSE(SearchValue(alphabeta, 3, true, true), 3.0, tol);
    BOOST_CHECK_CLOSE(SearchValue(alphabeta, 4, false, false), 1.0, tol);
    BOOST_CHECK_CLOSE(SearchValue(alphabeta, 4, true, false), 2.0, tol);
    BOOST_CHECK_EQUAL(SearchMove(alphabeta, 4, true, true), Pt(4, 2));
    BOOST_CHECK_CLOSE(SearchValue(alphabeta, 4, true, true), 2.0, tol);
    BOOST_CHECK_CLOSE(SearchValue(alphabeta, 5, false, false), 3.0, tol);
    BOOST_CHECK_CLOSE(SearchValue(alphabeta, 5, true, false), 3.0, tol);
    BOOST_CHECK_EQUAL(SearchMove(alphabeta, 5, true, true), Pt(4, 2));
    BOOST_CHECK_CLOSE(SearchValue(alphabeta, 5, true, true), 3.0, tol);
    BOOST_CHECK_CLOSE(SearchValue(alphabeta, 6, false, false), 2.0, tol);
    BOOST_CHECK_CLOSE(SearchValue(alphabeta, 7, false, false), 3.0, tol);

    evaluator.TakeBackUndo(false);
    evaluator.TakeBackUndo(false);
    evaluator.TakeBackUndo(false);
    evaluator.TakeBackUndo(false);
}

BOOST_AUTO_TEST_CASE(RlAlphaBetaTestLadders)
{
    GoBoard bd(5);
    RlLocalShapeFeatures shapes(bd, 1, 1);
    RlWeightSet weights(bd, &shapes);
    RlMoveFilter movefilter(bd);
    RlEvaluator evaluator(bd, &shapes, &weights, &movefilter);
    RlAlphaBeta alphabeta(evaluator.GetBoard(), &evaluator);

    // Material value of 1.0 for each stone
    alphabeta.EnsureInitialised();
    MakeWeights(bd, weights);

    alphabeta.SetNullMovePruning(false);
    TestLadders(evaluator, alphabeta);
    //alphabeta.SetNullMovePruning(true);
    //alphabeta.SetNullMoveDepth(2);
    //TestLadders(evaluator, alphabeta);
}

} // namespace

//----------------------------------------------------------------------------

