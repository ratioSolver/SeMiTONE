#include "sat_core.h"
#include <cassert>

using namespace semitone;

void test_literals()
{
    lit l0(2, false);
    lit l1 = !lit(3);
    lit l2(4);

    assert(variable(l0) == 2);
    assert(sign(l0) == false);
    assert(variable(l1) == 3);
    assert(sign(l1) == false);
    assert(variable(l2) == 4);
    assert(sign(l2) == true);
}

void test_basic_core_0()
{
    sat_core core;
    var b0 = core.new_var();
    var b1 = core.new_var();
    var b2 = core.new_var();
}

int main(int, char **)
{
    test_literals();

    test_basic_core_0();
}