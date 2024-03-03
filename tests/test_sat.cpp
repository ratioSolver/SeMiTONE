#include "sat_core.hpp"
#include <cassert>

using namespace semitone;

void test_basic_core_0()
{
    sat_core core;
    VARIABLE_TYPE b0 = core.new_var();
    VARIABLE_TYPE b1 = core.new_var();
    VARIABLE_TYPE b2 = core.new_var();

    bool nc = core.new_clause({utils::lit(b0, false), !utils::lit(b1), utils::lit(b2)});
    assert(nc);
    bool ch = core.propagate();
    assert(ch);
    assert(core.value(b0) == utils::Undefined);
    assert(core.value(b1) == utils::Undefined);
    assert(core.value(b2) == utils::Undefined);

    bool assm = core.assume(utils::lit(b0));
    assert(assm);
    assert(core.value(b0) == utils::True);
    assert(core.value(b1) == utils::Undefined);
    assert(core.value(b2) == utils::Undefined);

    assm = core.assume(utils::lit(b1));
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

    auto eq = core.new_eq(utils::lit(b0), utils::lit(b1));
    assert(core.value(b0) == utils::Undefined);
    assert(core.value(b1) == utils::Undefined);
    assert(core.value(eq) == utils::Undefined);

    bool assm = core.assume(eq);
    assert(assm);
    assert(core.value(b0) == utils::Undefined);
    assert(core.value(b1) == utils::Undefined);
    assert(core.value(eq) == utils::True);

    assm = core.assume(utils::lit(b0));
    assert(assm);
    assert(core.value(b0) == utils::True);
    assert(core.value(b1) == utils::True);
    assert(core.value(eq) == utils::True);
}

void test_conj()
{
    sat_core core;
    VARIABLE_TYPE b0 = core.new_var();
    VARIABLE_TYPE b1 = core.new_var();
    VARIABLE_TYPE b2 = core.new_var();

    auto conj = core.new_conj({utils::lit(b0), utils::lit(b1), utils::lit(b2, false)});
    assert(core.value(b0) == utils::Undefined);
    assert(core.value(b1) == utils::Undefined);
    assert(core.value(b2) == utils::Undefined);
    assert(core.value(conj) == utils::Undefined);

    bool assm = core.assume(conj);
    assert(assm);
    assert(core.value(b0) == utils::True);
    assert(core.value(b1) == utils::True);
    assert(core.value(b2) == utils::False);
    assert(core.value(conj) == utils::True);

    core.pop();

    assm = core.assume(!conj);
    assert(assm);
    assert(core.value(b0) == utils::Undefined);
    assert(core.value(b1) == utils::Undefined);
    assert(core.value(b2) == utils::Undefined);
    assert(core.value(conj) == utils::False);

    assm = core.assume(utils::lit(b0));
    assert(assm);
    assert(core.value(b0) == utils::True);
    assert(core.value(b1) == utils::Undefined);
    assert(core.value(b2) == utils::Undefined);
    assert(core.value(conj) == utils::False);

    assm = core.assume(utils::lit(b1));
    assert(assm);
    assert(core.value(b0) == utils::True);
    assert(core.value(b1) == utils::True);
    assert(core.value(b2) == utils::True);
    assert(core.value(conj) == utils::False);
}

void test_disj()
{
    sat_core core;
    VARIABLE_TYPE b0 = core.new_var();
    VARIABLE_TYPE b1 = core.new_var();
    VARIABLE_TYPE b2 = core.new_var();

    auto disj = core.new_disj({utils::lit(b0), utils::lit(b1), utils::lit(b2, false)});
    assert(core.value(b0) == utils::Undefined);
    assert(core.value(b1) == utils::Undefined);
    assert(core.value(b2) == utils::Undefined);
    assert(core.value(disj) == utils::Undefined);

    bool assm = core.assume(!disj);
    assert(assm);
    assert(core.value(b0) == utils::False);
    assert(core.value(b1) == utils::False);
    assert(core.value(b2) == utils::True);
    assert(core.value(disj) == utils::False);

    core.pop();

    assm = core.assume(disj);
    assert(assm);
    assert(core.value(b0) == utils::Undefined);
    assert(core.value(b1) == utils::Undefined);
    assert(core.value(b2) == utils::Undefined);
    assert(core.value(disj) == utils::True);

    assm = core.assume(utils::lit(b0, false));
    assert(assm);
    assert(core.value(b0) == utils::False);
    assert(core.value(b1) == utils::Undefined);
    assert(core.value(b2) == utils::Undefined);
    assert(core.value(disj) == utils::True);

    assm = core.assume(utils::lit(b1, false));
    assert(assm);
    assert(core.value(b0) == utils::False);
    assert(core.value(b1) == utils::False);
    assert(core.value(b2) == utils::False);
    assert(core.value(disj) == utils::True);
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
    test_basic_core_0();

    test_eq();
    test_conj();
    test_disj();

    test_core_copy();
}