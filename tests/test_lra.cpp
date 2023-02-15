#include "sat_core.h"
#include "lra_theory.h"
#include "sat_stack.h"
#include <cassert>

using namespace semitone;

void test_lin()
{
    lin l0;
    l0 += lin(0, utils::rational::ONE);
    l0 += lin(1, utils::rational(2));

    lin l1;
    l1 += lin(0, utils::rational::ONE);
    l1 += lin(1, utils::rational(2));

    lin l2 = l0 + l1;
    assert(l2.vars.at(0) == utils::rational(2));
    assert(l2.vars.at(1) == utils::rational(4));
}

void test_lra_theory()
{
    auto core = sat_ptr(new sat_core());
    lra_theory lra(core);

    var x = lra.new_var();
    var y = lra.new_var();
    var s1 = lra.new_var(lin(x, -utils::rational::ONE) + lin(y, utils::rational::ONE));
    var s2 = lra.new_var(lin(x, utils::rational::ONE) + lin(y, utils::rational::ONE));

    // x <= -4
    bool nc = core->new_clause({lra.new_leq(lin(x, utils::rational::ONE), lin(utils::rational(-4)))});
    assert(nc);
    // x >= -8
    nc = core->new_clause({lra.new_geq(lin(x, utils::rational::ONE), lin(-utils::rational(8)))});
    assert(nc);
    // s1 <= 1
    nc = core->new_clause({lra.new_leq(lin(s1, utils::rational::ONE), lin(utils::rational::ONE))});
    assert(nc);

    bool prop = core->propagate();
    assert(prop);

    // s2 >= -3
    bool assm = core->assume(lra.new_geq(lin(s2, utils::rational::ONE), lin(-utils::rational(3))));
    assert(!assm);
}

void test_inequalities_0()
{
    auto core = sat_ptr(new sat_core());
    lra_theory lra(core);

    var x = lra.new_var();
    var y = lra.new_var();

    // x >= y
    bool nc = core->new_clause({lra.new_geq(lin(x, utils::rational::ONE), lin(y, utils::rational::ONE))});
    assert(nc);

    utils::inf_rational x_val = lra.value(x);
    assert(x_val == utils::rational::ZERO);

    utils::inf_rational y_val = lra.value(y);
    assert(y_val == utils::rational::ZERO);

    // y >= 1
    nc = core->new_clause({lra.new_geq(lin(y, utils::rational::ONE), lin(utils::rational::ONE))});
    assert(nc);

    bool prop = core->propagate();
    assert(prop);

    x_val = lra.value(x);
    assert(x_val == utils::rational::ONE);

    y_val = lra.value(y);
    assert(y_val == utils::rational::ONE);
}

void test_inequalities_1()
{
    auto core = sat_ptr(new sat_core());
    lra_theory lra(core);

    var x = lra.new_var();
    var y = lra.new_var();

    // x >= y
    bool nc = core->new_clause({lra.new_geq(lin(x, utils::rational::ONE), lin(y, utils::rational::ONE))});
    assert(nc);

    bool prop = core->propagate();
    assert(prop);

    utils::inf_rational x_val = lra.value(x);
    assert(x_val == utils::rational::ZERO);

    utils::inf_rational y_val = lra.value(y);
    assert(y_val == utils::rational::ZERO);

    // y >= 1
    nc = core->new_clause({lra.new_geq(lin(y, utils::rational::ONE), lin(utils::rational::ONE))});
    assert(nc);

    prop = core->propagate();
    assert(prop);

    x_val = lra.value(x);
    assert(x_val == utils::rational::ONE);

    y_val = lra.value(y);
    assert(y_val == utils::rational::ONE);
}

void test_strict_inequalities_0()
{
    auto core = sat_ptr(new sat_core());
    lra_theory lra(core);

    var x = lra.new_var();
    var y = lra.new_var();

    // x > y
    bool nc = core->new_clause({lra.new_gt(lin(x, utils::rational::ONE), lin(y, utils::rational::ONE))});
    assert(nc);

    bool prop = core->propagate();
    assert(prop);

    utils::inf_rational x_val = lra.value(x);
    assert(x_val == utils::inf_rational(utils::rational::ZERO, utils::rational::ONE));

    utils::inf_rational y_val = lra.value(y);
    assert(y_val == utils::rational::ZERO);

    // y >= 1
    nc = core->new_clause({lra.new_geq(lin(y, utils::rational::ONE), lin(utils::rational::ONE))});
    assert(nc);

    prop = core->propagate();
    assert(prop);

    x_val = lra.value(x);
    assert(x_val == utils::inf_rational(utils::rational::ONE, utils::rational::ONE));

    y_val = lra.value(y);
    assert(y_val == utils::rational::ONE);
}

void test_strict_inequalities_1()
{
    auto core = sat_ptr(new sat_core());
    lra_theory lra(core);

    var x = lra.new_var();
    var y = lra.new_var();

    // ![x >= y] --> x < y
    bool nc = core->new_clause({!lra.new_geq(lin(x, utils::rational::ONE), lin(y, utils::rational::ONE))});
    assert(nc);

    bool prop = core->propagate();
    assert(prop);

    utils::inf_rational x_val = lra.value(x);
    assert(x_val == utils::inf_rational(utils::rational::ZERO, -utils::rational::ONE));

    utils::inf_rational y_val = lra.value(y);
    assert(y_val == utils::rational::ZERO);

    // x >= 1
    nc = core->new_clause({lra.new_geq(lin(x, utils::rational::ONE), lin(utils::rational::ONE))});
    assert(nc);

    prop = core->propagate();
    assert(prop);

    x_val = lra.value(x);
    assert(x_val == utils::rational::ONE);

    y_val = lra.value(y);
    assert(y_val == utils::inf_rational(utils::rational::ONE, utils::rational::ONE));
}

void test_nonroot_constraints()
{
    auto core = sat_ptr(new sat_core());
    lra_theory lra(core);

    var x = lra.new_var();
    var y = lra.new_var();

    lit x_leq_y = lra.new_leq(lin(x, utils::rational::ONE), lin(y, utils::rational::ONE));
    lit y_leq_x = lra.new_leq(lin(y, utils::rational::ONE), lin(x, utils::rational::ONE));

    bool nc = core->new_clause({lra.new_leq(lin(y, utils::rational::ONE), lin(utils::rational::ONE))});
    assert(nc);

    bool prop = core->propagate();
    assert(prop);

    nc = core->new_clause({x_leq_y, y_leq_x});
    assert(nc);

    bool assm = core->assume({x_leq_y});
    assert(assm);

    assert(core->value(x_leq_y) == True);

    prop = lra.set_lb(x, utils::inf_rational(utils::rational::ONE), TRUE_lit);
    assert(prop);

    prop = lra.set_lb(x, utils::inf_rational(utils::rational(2)), TRUE_lit);
    assert(!prop);
}

void test_sat_stack_0()
{
    sat_stack stack;
    lra_theory lra(stack.top());

    var x = lra.new_var();
    var y = lra.new_var();

    // x >= y
    bool nc = stack.top()->new_clause({lra.new_geq(lin(x, utils::rational::ONE), lin(y, utils::rational::ONE))});
    assert(nc);

    bool prop = stack.top()->propagate();
    assert(prop);

    utils::inf_rational x_val = lra.value(x);
    assert(x_val == utils::rational::ZERO);

    utils::inf_rational y_val = lra.value(y);
    assert(y_val == utils::rational::ZERO);

    // linear constraints have to be created before pushing..
    // y >= 1
    auto y_geq_1 = lra.new_geq(lin(y, utils::rational::ONE), lin(utils::rational::ONE));
    // x <= 0
    auto x_leq_0 = lra.new_leq(lin(x, utils::rational::ONE), lin(utils::rational::ZERO));

    // we push the sat stack..
    stack.push();

    nc = stack.top()->new_clause({y_geq_1});
    assert(nc);

    prop = stack.top()->propagate();
    assert(prop);

    x_val = lra.value(x);
    assert(x_val == utils::rational::ONE);

    y_val = lra.value(y);
    assert(y_val == utils::rational::ONE);

    // we pop the sat stack..
    stack.pop();

    nc = stack.top()->new_clause({x_leq_0});
    assert(nc);

    prop = stack.top()->propagate();
    assert(prop);

    x_val = lra.value(x);
    assert(x_val == utils::rational::ZERO);

    y_val = lra.value(y);
    assert(y_val == utils::rational::ZERO);
}

int main(int, char **)
{
    test_lin();

    test_lra_theory();
    test_inequalities_0();
    test_inequalities_1();

    test_strict_inequalities_0();
    test_strict_inequalities_1();

    test_nonroot_constraints();

    test_sat_stack_0();
}