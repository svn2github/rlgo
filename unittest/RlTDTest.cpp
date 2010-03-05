//----------------------------------------------------------------------------
/** @file RlLearningRuleTest.cpp
    Unit tests for RlLearningRule
*/
//----------------------------------------------------------------------------

#include "SgSystem.h"

#include <boost/test/floating_point_comparison.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/auto_unit_test.hpp>
#include "RlTDRules.h"

#include "RlActiveSet.h"
#include "RlEvaluator.h"
#include "RlManualFeatures.h"
#include "RlMoveFilter.h"
#include "RlState.h"
#include "RlWeightSet.h"
#include "RlTestUtil.h"

#include <vector>

using namespace std;

//----------------------------------------------------------------------------

namespace {

// percentage tolerance for floating point comparison
const float tol = 0.001f; 

void RlAgentTestTD1(RlManualFeatureSet& f, RlWeightSet& w, 
    RlEvaluator& ev, RlTD0& td, RlState& s1, RlState& s2)
{
    w.ZeroWeights();
    f.Clear();
    f.Set(0, 1);
    f.Set(1, 1);
    ev.Reset();
    s1.SetActive(ev.Active());
    s1.SetEval(ev.Eval());
    s2.SetEval(1.0);
    td.SetData(s1, s2);
    td.Learn();
    
    BOOST_CHECK(w.Get(0).Weight() > 0);
    BOOST_CHECK(w.Get(1).Weight() > 0);
    BOOST_CHECK(w.Get(2).Weight() == 0);
    BOOST_CHECK(w.Get(3).Weight() == 0);
}

void RlAgentTestTD2(RlManualFeatureSet& f, RlWeightSet& w, 
    RlEvaluator& ev, RlTD0& td, RlState& s1, RlState& s2)
{
    w.ZeroWeights();
    f.Clear();
    f.Set(0, 1);
    f.Set(1, 1);
    ev.Reset();
    s1.SetActive(ev.Active());
    s1.SetEval(ev.Eval());
    s2.SetEval(-1.0);
    td.SetData(s1, s2);
    td.Learn();

    BOOST_CHECK(w.Get(0).Weight() < 0);
    BOOST_CHECK(w.Get(1).Weight() < 0);
    BOOST_CHECK(w.Get(2).Weight() == 0);
    BOOST_CHECK(w.Get(3).Weight() == 0);
}

void RlAgentTestTD3(RlManualFeatureSet& f, RlWeightSet& w, 
    RlEvaluator& ev, RlTD0& td, RlState& s1, RlState& s2)
{
    w.ZeroWeights();
    f.Clear();
    f.Set(0, 3);
    f.Set(1, 2);
    ev.Reset();
    s1.SetActive(ev.Active());
    s1.SetEval(ev.Eval());
    s2.SetEval(1.0);
    td.SetData(s1, s2);
    td.Learn();
    
    BOOST_CHECK(w.Get(0).Weight() > 0);
    BOOST_CHECK(w.Get(1).Weight() > 0);
    BOOST_CHECK(w.Get(2).Weight() == 0);
    BOOST_CHECK(w.Get(3).Weight() == 0);
    BOOST_CHECK(
        w.Get(0).Weight() > 
        w.Get(1).Weight());
}

void RlAgentTestTD4(RlManualFeatureSet& f, RlWeightSet& w, 
    RlEvaluator& ev, RlTD0& td, RlState& s1, RlState& s2)
{
    w.ZeroWeights();
    w.Get(2).Weight() += 1.0;
    f.Clear();
    f.Set(0, 1);
    f.Set(1, 1);
    ev.Reset();
    s1.SetActive(ev.Active());
    s1.SetEval(ev.Eval());
    s2.SetEval(1.0);
    td.SetData(s1, s2);
    td.Learn();

    BOOST_CHECK(w.Get(0).Weight() > 0);
    BOOST_CHECK(w.Get(1).Weight() > 0);
    BOOST_CHECK(w.Get(2).Weight() == 1.0);
    BOOST_CHECK(w.Get(3).Weight() == 0);
}

void RlAgentTestTD5(RlManualFeatureSet& f, RlWeightSet& w, 
    RlEvaluator& ev, RlTD0& td, RlState& s1, RlState& s2)
{
    w.ZeroWeights();
    w.Get(2).Weight() += -1.0;
    f.Clear();
    f.Set(0, 1);
    f.Set(1, 1);
    ev.Reset();
    s1.SetActive(ev.Active());
    s1.SetEval(ev.Eval());
    s2.SetEval(-1.0);
    td.SetData(s1, s2);
    td.Learn();

    BOOST_CHECK(w.Get(0).Weight() < 0);
    BOOST_CHECK(w.Get(1).Weight() < 0);
    BOOST_CHECK(w.Get(2).Weight() == -1.0);
    BOOST_CHECK(w.Get(3).Weight() == 0);
}

void RlAgentTestTD6(RlManualFeatureSet& f, RlWeightSet& w, 
    RlEvaluator& ev, RlTD0& td, RlState& s1, RlState& s2)
{
    w.ZeroWeights();
    w.Get(2).Weight() += 1.0;
    f.Clear();
    f.Set(0, 3);
    f.Set(1, 2);
    ev.Reset();
    s1.SetActive(ev.Active());
    s1.SetEval(ev.Eval());
    s2.SetEval(1.0);
    td.SetData(s1, s2);
    td.Learn();

    BOOST_CHECK(w.Get(0).Weight() > 0);
    BOOST_CHECK(w.Get(1).Weight() > 0);
    BOOST_CHECK(w.Get(2).Weight() == 1.0);
    BOOST_CHECK(w.Get(3).Weight() == 0);
    BOOST_CHECK(
        w.Get(0).Weight() > 
        w.Get(1).Weight());
}

BOOST_AUTO_TEST_CASE(RlAgentTestTD)
{
    GoBoard bd(9);
    RlManualFeatureSet f(bd, 4);
    RlManualTracker tracker(bd, &f);
    RlWeightSet w(bd, &f);
    RlMoveFilter mf(bd);
    RlEvaluator ev(bd, &f, &w, &mf);
    RlTD0 td(bd, &w);
    f.EnsureInitialised();
    w.EnsureInitialised();
    ev.EnsureInitialised();
    td.EnsureInitialised();

    RlState s1(1, SG_BLACK);
    RlState s2(2, SG_WHITE);

    RlAgentTestTD1(f, w, ev, td, s1, s2);
    RlAgentTestTD2(f, w, ev, td, s1, s2);
    RlAgentTestTD3(f, w, ev, td, s1, s2);
    RlAgentTestTD4(f, w, ev, td, s1, s2);
}

} // namespace

//----------------------------------------------------------------------------

