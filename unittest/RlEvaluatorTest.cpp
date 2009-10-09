//----------------------------------------------------------------------------
/** @file RlEvaluatorTest.cpp
    Unit tests for RlEvaluator
*/
//----------------------------------------------------------------------------

#include "SgSystem.h"

#include <boost/test/floating_point_comparison.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/auto_unit_test.hpp>
#include "RlEvaluator.h"

#include "RlActiveSet.h"
#include "RlManualFeatures.h"
#include "RlMoveFilter.h"
#include "RlWeightSet.h"

//----------------------------------------------------------------------------

namespace {

// percentage tolerance for floating point comparison
const float tol = 0.001f; 

void TestEvaluator1(RlEvaluator& ev, RlManualFeatureSet& f,
    RlWeightSet* w)
{
    w->ZeroWeights();
    f.Clear();
    f.Set(0, 1);
    f.Set(1, 1);
    w->Get(0).Weight() = 0.2f;
    w->Get(1).Weight() = 0.3f;
    
    ev.Reset();
    float eval = ev.Eval();
    BOOST_CHECK_EQUAL(ev.Active().GetTotalActive(), 2);
    BOOST_CHECK_EQUAL(ev.Active().GetOccurrences(0), 1);
    BOOST_CHECK_EQUAL(ev.Active().GetOccurrences(1), 1);
    BOOST_CHECK_CLOSE(eval, 0.5f, tol);

    ev.EvaluateMove(SG_PASS, SG_BLACK);
    eval = ev.Eval();
    BOOST_CHECK_CLOSE(eval, 0.5f, tol);
}

void TestEvaluator2(RlEvaluator& ev, RlManualFeatureSet& f,
    RlWeightSet* w)
{
    w->ZeroWeights();
    f.Clear();
    f.Set(0, 3);
    f.Set(1, 2);
    w->Get(0).Weight() = 0.2f;
    w->Get(1).Weight() = 0.3f;
    
    ev.Reset();
    float eval = ev.Eval();
    BOOST_CHECK_EQUAL(ev.Active().GetTotalActive(), 5);
    BOOST_CHECK_EQUAL(ev.Active().GetOccurrences(0), 3);
    BOOST_CHECK_EQUAL(ev.Active().GetOccurrences(1), 2);
    BOOST_CHECK_CLOSE(eval, 1.2f, tol);

    ev.EvaluateMove(SG_PASS, SG_BLACK);
    eval = ev.Eval();
    BOOST_CHECK_CLOSE(eval, 1.2f, tol);
}

BOOST_AUTO_TEST_CASE(RlEvaluatorTest)
{
    GoBoard bd(9);
    RlManualFeatureSet f(bd, 4);
    RlWeightSet w(bd, &f);
    RlMoveFilter mf(bd);
    RlEvaluator ev(bd, &f, &w, &mf);
    f.EnsureInitialised();
    w.EnsureInitialised();
    ev.EnsureInitialised();

    TestEvaluator1(ev, f, &w);
    TestEvaluator2(ev, f, &w);
}

} // namespace

//----------------------------------------------------------------------------

