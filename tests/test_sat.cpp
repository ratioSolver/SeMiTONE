#include "sat_core.hpp"
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
    VARIABLE_TYPE b0 = core.new_var();
    VARIABLE_TYPE b1 = core.new_var();
    VARIABLE_TYPE b2 = core.new_var();

    bool nc = core.new_clause({lit(b0, false), !lit(b1), lit(b2)});
    assert(nc);
    bool ch = core.propagate();
    assert(ch);
    assert(core.value(b0) == utils::Undefined);
    assert(core.value(b1) == utils::Undefined);
    assert(core.value(b2) == utils::Undefined);

    bool assm = core.assume(lit(b0));
    assert(assm);
    assert(core.value(b0) == utils::True);
    assert(core.value(b1) == utils::Undefined);
    assert(core.value(b2) == utils::Undefined);

    assm = core.assume(lit(b1));
    assert(assm);
    assert(core.value(b0) == utils::True);
    assert(core.value(b1) == utils::True);
    assert(core.value(b2) == utils::True);
}

void test_eq()
{
    sat_core core;
    VARIABLE_TYPE b0 = core.new_var();
    VARIABLE_TYPE b1 = core.new_var();

    lit eq = core.new_eq(lit(b0), lit(b1));
    assert(core.value(b0) == utils::Undefined);
    assert(core.value(b1) == utils::Undefined);
    assert(core.value(eq) == utils::Undefined);

    bool assm = core.assume(eq);
    assert(assm);
    assert(core.value(b0) == utils::Undefined);
    assert(core.value(b1) == utils::Undefined);
    assert(core.value(eq) == utils::True);

    assm = core.assume(lit(b0));
    assert(assm);
    assert(core.value(b0) == utils::True);
    assert(core.value(b1) == utils::True);
    assert(core.value(eq) == utils::True);
}

void test_core_copy()
{
    sat_core core;
    VARIABLE_TYPE b0 = core.new_var();
    VARIABLE_TYPE b1 = core.new_var();
    VARIABLE_TYPE b2 = core.new_var();

    sat_core core2(core);
    assert(core2.value(b0) == core.value(b0));
    assert(core2.value(b1) == core.value(b1));
    assert(core2.value(b2) == core.value(b2));
}

int main(int, char **)
{
    test_literals();

    test_basic_core_0();

    test_eq();

    test_core_copy();
}