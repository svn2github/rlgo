#ifdef RL_ELIGIBILITY
BOOST_AUTO_TEST_CASE(RlAgentTestTDLambda1)
{
    GoBoard bd(9);
    RlActiveConnections active(4);
    RlConnector connector(bd);
    RlManualFeatureSet f(bd, 4);
    RlManualTracker tracker(bd, &f, &connector);
    RlConnectionSet* w = connector.AddSet(&f);
    RlEvaluator ev(bd);
    ev.AddTracker(&tracker);
    RlSimpleStepSize stepsize(bd, 1.0);
    RlTDLambda td(bd, &ev, &stepsize, 0, 0, 0.5f);
    connector.EnsureInitialised();
    ev.EnsureInitialised();
    td.EnsureInitialised();

    w.ZeroWeights();
    w.Get(3).Weight() += 1.0;

    f.Clear();
    f.Set(0, 1);
    f.Set(1, 1);
    ev.Reset();
    ev.Evaluate();
    ev.GetActive(active);
    float value1 = ev.Eval();
    td.SetData(&active, value1, value1, 0.0, false);
    td.Learn();
    
    active.Clear();
    f.Clear();
    f.Set(1, 1);
    f.Set(2, 1);
    ev.Reset();
    ev.Evaluate();
    ev.GetActive(active);
    float value2 = ev.Eval();
    td.SetData(&active, value2, value2, 1.0, false);
    td.Learn();

    BOOST_CHECK_CLOSE(
        w.Get(0)->Eligibility(), 0.5f, tol);
    BOOST_CHECK_CLOSE(
        w.Get(1)->Eligibility(), 1.5f, tol);
    BOOST_CHECK_CLOSE(
        w.Get(2)->Eligibility(), 1.0, tol);

    BOOST_CHECK(w.Get(0).Weight() > 0);
    BOOST_CHECK(w.Get(1).Weight() > 0);
    BOOST_CHECK(w.Get(2).Weight() > 0);
    BOOST_CHECK(w.Get(3).Weight() == 1.0);

    BOOST_CHECK_CLOSE(
        w.Get(1).Weight(),
        w.Get(0).Weight() * 3.0, tol);
    BOOST_CHECK_CLOSE(
        w.Get(1).Weight(),
        w.Get(2).Weight() * 1.5f, tol);
    BOOST_CHECK_CLOSE(
        w.Get(2).Weight(),
        w.Get(0).Weight() * 2.0, tol);
}

BOOST_AUTO_TEST_CASE(RlAgentTestTDLambda2)
{
    GoBoard bd(9);
    RlActiveConnections active(4);
    RlConnector connector(bd);
    RlManualFeatureSet f(bd, 4);
    RlManualTracker tracker(bd, &f, &connector);
    RlConnectionSet* w = connector.AddSet(&f);
    RlEvaluator ev(bd);
    ev.AddTracker(&tracker);
    RlSimpleStepSize stepsize(bd, 1.0);
    RlTDLambda td(bd, &ev, &stepsize, 0, 0, 0.5f);
    connector.EnsureInitialised();
    ev.EnsureInitialised();
    td.EnsureInitialised();

    w.ZeroWeights();
    w.Get(3).Weight() += -1.0;

    f.Clear();
    f.Set(0, 1);
    f.Set(1, 1);
    ev.Reset();
    ev.Evaluate();
    ev.GetActive(active);
    float value1 = ev.Eval();
    td.SetData(&active, value1, value1, 0.0, false);
    td.Learn();

    active.Clear();
    f.Clear();
    f.Set(1, 1);
    f.Set(2, 1);
    ev.Reset();
    ev.Evaluate();
    ev.GetActive(active);
    float value2 = ev.Eval();
    td.SetData(&active, value2, value2, -1.0, false);
    td.Learn();

    BOOST_CHECK_CLOSE(
        w.Get(0)->Eligibility(), 0.5f, tol);
    BOOST_CHECK_CLOSE(
        w.Get(1)->Eligibility(), 1.5f, tol);
    BOOST_CHECK_CLOSE(
        w.Get(2)->Eligibility(), 1.0, tol);

    BOOST_CHECK(w.Get(0).Weight() < 0);
    BOOST_CHECK(w.Get(1).Weight() < 0);
    BOOST_CHECK(w.Get(2).Weight() < 0);
    BOOST_CHECK(w.Get(3).Weight() == -1.0);

    BOOST_CHECK_CLOSE(
        w.Get(1).Weight(),
        w.Get(0).Weight() * 3.0, tol);
    BOOST_CHECK_CLOSE(
        w.Get(1).Weight(),
        w.Get(2).Weight() * 1.5f, tol);
    BOOST_CHECK_CLOSE(
        w.Get(2).Weight(),
        w.Get(0).Weight() * 2.0, tol);
}

BOOST_AUTO_TEST_CASE(RlAgentTestTDLambda3)
{
    GoBoard bd(9);
    RlActiveConnections active(4);
    RlConnector connector(bd);
    RlManualFeatureSet f(bd, 4);
    RlManualTracker tracker(bd, &f, &connector);
    RlConnectionSet* w = connector.AddSet(&f);
    RlEvaluator ev(bd);
    ev.AddTracker(&tracker);
    RlSimpleStepSize stepsize(bd, 1.0);
    RlTDLambda td(bd, &ev, &stepsize, 0, 0, 0.5f);
    connector.EnsureInitialised();
    ev.EnsureInitialised();
    td.EnsureInitialised();

    w.ZeroWeights();
    w.Get(3).Weight() += 1.0;

    f.Clear();
    f.Set(0, 3);
    f.Set(1, 2);
    ev.Reset();
    ev.Evaluate();
    ev.GetActive(active);
    float value1 = ev.Eval();
    td.SetData(&active, value1, value1, 0.0, false);
    td.Learn();
    
    active.Clear();
    f.Clear();
    f.Set(1, 2);
    f.Set(2, 1);
    ev.Reset();
    ev.Evaluate();
    ev.GetActive(active);
    float value2 = ev.Eval();
    td.SetData(&active, value2, value2, 1.0, false);
    td.Learn();

    BOOST_CHECK_CLOSE(
        w.Get(0)->Eligibility(), 1.5f, tol);
    BOOST_CHECK_CLOSE(
        w.Get(1)->Eligibility(), 3.0, tol);
    BOOST_CHECK_CLOSE(
        w.Get(2)->Eligibility(), 1.0, tol);

    BOOST_CHECK(w.Get(0).Weight() > 0);
    BOOST_CHECK(w.Get(1).Weight() > 0);
    BOOST_CHECK(w.Get(2).Weight() > 0);
    BOOST_CHECK(w.Get(3).Weight() == 1.0);

    BOOST_CHECK_CLOSE(
        w.Get(1).Weight(),
        w.Get(0).Weight() * 2.0, tol);
    BOOST_CHECK_CLOSE(
        w.Get(1).Weight(),
        w.Get(2).Weight() * 3.0, tol);
    BOOST_CHECK_CLOSE(
        w.Get(0).Weight(),
        w.Get(2).Weight() * 1.5f, tol);
}
#endif // RL_ELIGIBILITY
