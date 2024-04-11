#include "sat_core.hpp"
#include "lra_theory.hpp"
#include <cassert>

void test_lra()
{
    semitone::sat_core sat;
    auto &lra = sat.new_theory<semitone::lra_theory>();

    auto x = lra.new_var();
    auto y = lra.new_var();
    auto s1 = lra.new_var(utils::lin(x, -utils::rational::one) + utils::lin(y, utils::rational::one));
    auto s2 = lra.new_var(utils::lin(x, utils::rational::one) + utils::lin(y, utils::rational::one));

    // x <= -4
    bool nc = sat.new_clause({lra.new_leq(utils::lin(x, utils::rational::one), utils::lin(utils::rational(-4)))});
    assert(nc);
    // x >= -8
    nc = sat.new_clause({lra.new_geq(utils::lin(x, utils::rational::one), utils::lin(-utils::rational(8)))});
    assert(nc);
    // s1 <= 1
    nc = sat.new_clause({lra.new_leq(utils::lin(s1, utils::rational::one), utils::lin(utils::rational::one))});
    assert(nc);

    bool prop = sat.propagate();
    assert(prop);

    // s2 >= -3
    auto s2_geq = lra.new_geq(utils::lin(s2, utils::rational::one), utils::lin(-utils::rational(3)));
    assert(sat.value(s2_geq) == utils::False);
}

void test_inequalities_0()
{
    semitone::sat_core sat;
    auto &lra = sat.new_theory<semitone::lra_theory>();

    auto x = lra.new_var();
    auto y = lra.new_var();

    // x >= y
    bool nc = sat.new_clause({lra.new_geq(utils::lin(x, utils::rational::one), utils::lin(y, utils::rational::one))});
    assert(nc);

    utils::inf_rational x_val = lra.value(x);
    assert(x_val == utils::rational::zero);

    utils::inf_rational y_val = lra.value(y);
    assert(y_val == utils::rational::zero);

    // y >= 1
    nc = sat.new_clause({lra.new_geq(utils::lin(y, utils::rational::one), utils::lin(utils::rational::one))});
    assert(nc);

    bool prop = sat.propagate();
    assert(prop);

    x_val = lra.value(x);
    assert(x_val == utils::rational::one);

    y_val = lra.value(y);
    assert(y_val == utils::rational::one);
}

void test_inequalities_1()
{
    semitone::sat_core sat;
    auto &lra = sat.new_theory<semitone::lra_theory>();

    auto x = lra.new_var();
    auto y = lra.new_var();

    // x >= y
    bool nc = sat.new_clause({lra.new_geq(utils::lin(x, utils::rational::one), utils::lin(y, utils::rational::one))});
    assert(nc);

    bool prop = sat.propagate();
    assert(prop);

    utils::inf_rational x_val = lra.value(x);
    assert(x_val == utils::rational::zero);

    utils::inf_rational y_val = lra.value(y);
    assert(y_val == utils::rational::zero);

    // y >= 1
    nc = sat.new_clause({lra.new_geq(utils::lin(y, utils::rational::one), utils::lin(utils::rational::one))});
    assert(nc);

    prop = sat.propagate();
    assert(prop);

    x_val = lra.value(x);
    assert(x_val == utils::rational::one);

    y_val = lra.value(y);
    assert(y_val == utils::rational::one);
}

void test_strict_inequalities_0()
{
    semitone::sat_core sat;
    auto &lra = sat.new_theory<semitone::lra_theory>();

    auto x = lra.new_var();
    auto y = lra.new_var();

    // x > y
    bool nc = sat.new_clause({lra.new_gt(utils::lin(x, utils::rational::one), utils::lin(y, utils::rational::one))});
    assert(nc);

    bool prop = sat.propagate();
    assert(prop);

    utils::inf_rational x_val = lra.value(x);
    assert(x_val == utils::inf_rational(utils::rational::zero, utils::rational::one));

    utils::inf_rational y_val = lra.value(y);
    assert(y_val == utils::rational::zero);

    // y >= 1
    nc = sat.new_clause({lra.new_geq(utils::lin(y, utils::rational::one), utils::lin(utils::rational::one))});
    assert(nc);

    prop = sat.propagate();
    assert(prop);

    x_val = lra.value(x);
    assert(x_val == utils::inf_rational(utils::rational::one, utils::rational::one));

    y_val = lra.value(y);
    assert(y_val == utils::rational::one);
}

void test_strict_inequalities_1()
{
    semitone::sat_core sat;
    auto &lra = sat.new_theory<semitone::lra_theory>();

    auto x = lra.new_var();
    auto y = lra.new_var();

    // ![x >= y] --> x < y
    bool nc = sat.new_clause({!lra.new_geq(utils::lin(x, utils::rational::one), utils::lin(y, utils::rational::one))});
    assert(nc);

    bool prop = sat.propagate();
    assert(prop);

    utils::inf_rational x_val = lra.value(x);
    assert(x_val == utils::inf_rational(utils::rational::zero, -utils::rational::one));

    utils::inf_rational y_val = lra.value(y);
    assert(y_val == utils::rational::zero);

    // x >= 1
    nc = sat.new_clause({lra.new_geq(utils::lin(x, utils::rational::one), utils::lin(utils::rational::one))});
    assert(nc);

    prop = sat.propagate();
    assert(prop);

    x_val = lra.value(x);
    assert(x_val == utils::rational::one);

    y_val = lra.value(y);
    assert(y_val == utils::inf_rational(utils::rational::one, utils::rational::one));
}

void test_nonroot_constraints()
{
    semitone::sat_core sat;
    auto &lra = sat.new_theory<semitone::lra_theory>();

    auto x = lra.new_var();
    auto y = lra.new_var();

    auto x_leq_y = lra.new_leq(utils::lin(x, utils::rational::one), utils::lin(y, utils::rational::one));
    auto y_leq_x = lra.new_leq(utils::lin(y, utils::rational::one), utils::lin(x, utils::rational::one));

    bool nc = sat.new_clause({lra.new_leq(utils::lin(y, utils::rational::one), utils::lin(utils::rational::one))});
    assert(nc);

    bool prop = sat.propagate();
    assert(prop);

    nc = sat.new_clause({x_leq_y, y_leq_x});
    assert(nc);

    bool assm = sat.assume({x_leq_y});
    assert(assm);

    assert(sat.value(x_leq_y) == utils::True);

    prop = lra.set_lb(x, utils::inf_rational(utils::rational::one), utils::TRUE_lit);
    assert(prop);

    prop = lra.set_lb(x, utils::inf_rational(utils::rational(2)), utils::TRUE_lit);
    assert(!prop);
}

int main(int argc, char const *argv[])
{
    test_lra();

    test_inequalities_0();
    test_inequalities_1();

    test_strict_inequalities_0();
    test_strict_inequalities_1();

    test_nonroot_constraints();

    return 0;
}
