#include "sat_core.h"
#include "sat_stack.h"
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

void test_basic_core_1()
{
    sat_core core;
    var b0 = core.new_var();
    var b1 = core.new_var();
    var b2 = core.new_var();

    bool nc = core.new_clause({core.new_eq(lit(b0), !lit(b1))});
    assert(nc);

    assert(core.value(b0) == utils::Undefined);
    assert(core.value(b1) == utils::Undefined);
    assert(core.value(b2) == utils::Undefined);

    nc = core.new_clause({lit(b1), lit(b2)});
    assert(nc);

    bool prop = core.propagate();
    assert(prop);

    bool assm = core.assume(lit(b0));
    assert(assm);
    assert(core.value(b0) == utils::True);
    assert(core.value(b1) == utils::False);
    assert(core.value(b2) == utils::True);
}

void test_basic_core_2()
{
    sat_core core;

    var b0 = core.new_var();
    var b1 = core.new_var();
    var b2 = core.new_var();

    bool nc = core.new_clause({lit(b0, false), !lit(b1), lit(b2)});
    assert(nc);
    nc = core.new_clause({lit(b0), lit(b2)});
    assert(nc);
    nc = core.new_clause({lit(b0, false), lit(b1, false), lit(b2, false)});
    assert(nc);

    bool prop = core.propagate();
    assert(prop);

    bool assm = core.assume(lit(b0));
    assert(assm);
    assm = core.check({lit(b1, false), lit(b2, false)});
    assert(assm);
}

void test_no_good()
{
    sat_core core;

    var b0 = core.new_var();
    var b1 = core.new_var();
    var b2 = core.new_var();
    var b3 = core.new_var();
    var b4 = core.new_var();
    var b5 = core.new_var();
    var b6 = core.new_var();
    var b7 = core.new_var();
    var b8 = core.new_var();

    bool nc = core.new_clause({lit(b0), lit(b1)});
    assert(nc);
    nc = core.new_clause({lit(b0), lit(b2), lit(b6)});
    assert(nc);
    nc = core.new_clause({lit(b1, false), lit(b2, false), lit(b3)});
    assert(nc);
    nc = core.new_clause({lit(b3, false), lit(b4), lit(b7)});
    assert(nc);
    nc = core.new_clause({lit(b3, false), lit(b5), lit(b8)});
    assert(nc);
    nc = core.new_clause({lit(b4, false), lit(b5, false)});
    assert(nc);

    bool prop = core.propagate();
    assert(prop);

    bool assm = core.assume(lit(b6, false));
    assert(assm);
    assm = core.assume(lit(b7, false));
    assert(assm);
    assm = core.assume(lit(b8, false));
    assert(assm);
    assm = core.assume(lit(b0, false));
    assert(assm);
}

void test_assumptions()
{
    sat_core core;

    var b0 = core.new_var();
    var b1 = core.new_var();
    var b2 = core.new_var();
    var b3 = core.new_var();
    var b4 = core.new_var();
    var b5 = core.new_var();
    var b6 = core.new_var();
    var b7 = core.new_var();
    var b8 = core.new_var();

    bool nc = core.new_clause({lit(b0), lit(b1)});
    assert(nc);
    nc = core.new_clause({lit(b0), lit(b2), lit(b6)});
    assert(nc);
    nc = core.new_clause({lit(b1, false), lit(b2, false), lit(b3)});
    assert(nc);
    nc = core.new_clause({lit(b3, false), lit(b4), lit(b7)});
    assert(nc);
    nc = core.new_clause({lit(b3, false), lit(b5), lit(b8)});
    assert(nc);
    nc = core.new_clause({lit(b4, false), lit(b5, false)});
    assert(nc);

    bool prop = core.propagate();
    assert(prop);

    bool assm = core.check({lit(b6, false), lit(b7, false), lit(b8, false), lit(b0, false)});
    assert(!assm);
}

void test_exct_one_0()
{
    sat_core core;

    var b0 = core.new_var();
    var b1 = core.new_var();
    var b2 = core.new_var();
    var b3 = core.new_var();

    lit xct_one = core.new_exct_one({lit(b0), lit(b1), lit(b2), lit(b3)});

    bool prop = core.propagate();
    assert(prop);

    bool assm = core.assume(xct_one);
    assert(assm);
    assm = core.assume(lit(b0, false));
    assert(assm);
    assm = core.assume(lit(b1));
    assert(assm);
    assm = core.check({lit(b2, false), lit(b3, false)});
    assert(assm);
}

void test_exct_one_1()
{
    sat_core core;

    var b0 = core.new_var();
    var b1 = core.new_var();
    var b2 = core.new_var();
    var b3 = core.new_var();

    lit xct_one = core.new_exct_one({lit(b0), lit(b1), lit(b2), lit(b3)});

    bool prop = core.propagate();
    assert(prop);

    bool assm = core.assume(lit(b0, false));
    assert(assm);
    assm = core.assume(lit(b1, false));
    assert(assm);
    assm = core.assume(lit(b2, false));
    assert(assm);
    assm = core.assume(lit(b3, false));
    assert(assm);

    assert(core.value(xct_one) == utils::False);
}

void test_exct_one_2()
{
    sat_core core;

    var b0 = core.new_var();
    var b1 = core.new_var();
    var b2 = core.new_var();
    var b3 = core.new_var();

    lit xct_one = core.new_exct_one({lit(b0), lit(b1), lit(b2), lit(b3)});

    bool prop = core.propagate();
    assert(prop);

    bool assm = core.assume(xct_one);
    assert(assm);
    assm = core.assume(lit(b0, false));
    assert(assm);
    assm = core.assume(lit(b1, false));
    assert(assm);
    assm = core.assume(lit(b2, false));
    assert(assm);

    assert(core.value(b3) == utils::True);
}

void test_sat_stack_0()
{
    LOG("test_sat_stack_0");
    sat_stack stack;
    var b0 = stack.top()->new_var();
    var b1 = stack.top()->new_var();
    var b2 = stack.top()->new_var();

    bool nc = stack.top()->new_clause({lit(b0, false), !lit(b1), lit(b2)});
    assert(nc);
    assert(stack.top()->value(b0) == utils::Undefined);
    assert(stack.top()->value(b1) == utils::Undefined);
    assert(stack.top()->value(b2) == utils::Undefined);

    bool assm = stack.top()->assume(lit(b0));
    assert(assm);
    assert(stack.top()->value(b0) == utils::True);
    assert(stack.top()->value(b1) == utils::Undefined);
    assert(stack.top()->value(b2) == utils::Undefined);

    // we push the sat stack..
    stack.push();
    LOG("sat: " << to_json(*stack.top()));

    assm = stack.top()->assume(lit(b1));
    assert(assm);
    assert(stack.top()->value(b0) == utils::True);
    assert(stack.top()->value(b1) == utils::True);
    assert(stack.top()->value(b2) == utils::True);

    // we pop the sat stack..
    stack.pop();

    assm = stack.top()->assume(!lit(b2));
    assert(assm);
    assert(stack.top()->value(b0) == utils::True);
    assert(stack.top()->value(b1) == utils::False);
    assert(stack.top()->value(b2) == utils::False);
}

void test_to_json()
{
    sat_core core;

    var b0 = core.new_var();
    var b1 = core.new_var();
    var b2 = core.new_var();

    bool nc = core.new_clause({lit(b0, false), !lit(b1), lit(b2)});
    assert(nc);
    nc = core.new_clause({lit(b0), lit(b2)});
    assert(nc);

    auto j_sat = to_json(core);

    bool assm = core.new_clause({lit(b0)});
    assert(assm);

    assm = core.new_clause({!lit(b2)});
    assert(assm);

    j_sat = to_json(core);

    bool simplify = core.simplify_db();
    assert(simplify);
}

int main(int, char **)
{
    test_literals();

    test_basic_core_0();
    test_basic_core_1();
    test_basic_core_2();

    test_no_good();

    test_assumptions();

    test_exct_one_0();
    test_exct_one_1();
    test_exct_one_2();

    test_sat_stack_0();

    test_to_json();
}