//----------------------------------------------------------------------------
/** @file RlActiveSetTest.cpp
    Unit tests for RlActiveSet.h
*/
//----------------------------------------------------------------------------

#include "SgSystem.h"

#include "RlActiveSet.h"

#include <math.h>
#include <boost/test/unit_test.hpp>
#include <boost/test/auto_unit_test.hpp>
#include <iostream>

using namespace std;

//----------------------------------------------------------------------------

namespace {

BOOST_AUTO_TEST_CASE(RlActiveSetTest)
{
    int f1 = 12345;
    int f2 = 23456;
    int f3 = 34567;

    RlActiveSet active(2);
    BOOST_CHECK_EQUAL(active.GetTotalActive(), 0);
    active.Change(RlChange(0, f1, +10));
    BOOST_CHECK_EQUAL(active.GetTotalActive(), 10);
    BOOST_CHECK_EQUAL(active.GetFeatureIndex(0), f1);
    BOOST_CHECK_EQUAL(active.GetOccurrences(0), 10);
    active.Change(RlChange(1, f2, +5));
    BOOST_CHECK_EQUAL(active.GetTotalActive(), 15);
    BOOST_CHECK_EQUAL(active.GetFeatureIndex(1), f2);
    BOOST_CHECK_EQUAL(active.GetOccurrences(1), 5);
    active.Change(RlChange(0, f1, -10));
    BOOST_CHECK_EQUAL(active.GetTotalActive(), 5);
    BOOST_CHECK_EQUAL(active.GetFeatureIndex(0), -1);
    BOOST_CHECK_EQUAL(active.GetOccurrences(0), 0);
    active.Change(RlChange(0, f3, +5));
    BOOST_CHECK_EQUAL(active.GetTotalActive(), 10);
    BOOST_CHECK_EQUAL(active.GetFeatureIndex(0), f3);
    BOOST_CHECK_EQUAL(active.GetOccurrences(0), 5);
    active.Change(RlChange(1, f2, +5));
    BOOST_CHECK_EQUAL(active.GetTotalActive(), 15);
    BOOST_CHECK_EQUAL(active.GetFeatureIndex(1), f2);
    BOOST_CHECK_EQUAL(active.GetOccurrences(1), 10);
}

BOOST_AUTO_TEST_CASE(RlActiveSetIteratorTest)
{
    int f1 = 12345;
    int f2 = 23456;
    int f3 = 34567;

    RlActiveSet active(5);
    active.Change(RlChange(0, f1, +10));
    active.Change(RlChange(1, f1, +10));
    active.Change(RlChange(2, f2, +10));
    active.Change(RlChange(3, f2, +10));
    active.Change(RlChange(4, f3, +10));
    active.Change(RlChange(1, f1, -10));
    active.Change(RlChange(3, f2, -10));

    for (RlActiveSet::Iterator i_active(active); i_active; ++i_active)
    {
        BOOST_CHECK_EQUAL(i_active.Slot() % 2, 0);
        BOOST_CHECK_EQUAL(i_active->m_occurrences, 10);
    }
}

} // namespace

//----------------------------------------------------------------------------

