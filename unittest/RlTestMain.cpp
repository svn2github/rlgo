#define BOOST_TEST_DYN_LINK // Must be defined before including unit_test.hpp
#include <boost/test/unit_test.hpp>
#include <iostream>

#include "SgSystem.h"
#include "GoBoard.h"
#include "GoInit.h"
#include "SgInit.h"
#include "RlSetup.h"

bool init_unit_test()
{
    SgInit();
    GoInit();
    GoBoard bd(9);
    RlSetup* setup = new RlSetup(bd);
    setup->SetDebugLevel(RlSetup::VERBOSE);
    return true;
}

int main(int argc, char* argv[])
{
    return boost::unit_test::unit_test_main(&init_unit_test, argc, argv);
}
