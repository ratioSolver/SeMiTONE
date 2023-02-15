#include "idl_theory.h"
#include "rdl_theory.h"
#include <cassert>

using namespace semitone;

void test_config()
{
    auto core = sat_ptr(new sat_core());
    idl_theory idl(core, 5);
    var origin = idl.new_var();
    var horizon = idl.new_var();
    bool nc = core->new_clause({idl.new_distance(horizon, origin, 0)});
    assert(nc);

    bool prop = core->propagate();
    assert(prop);

    var tp0 = idl.new_var();
    nc = core->new_clause({idl.new_distance(tp0, origin, 0)});
    assert(nc);
    nc = core->new_clause({idl.new_distance(horizon, tp0, 0)});
    assert(nc);
    prop = core->propagate();
    assert(prop);

    var tp1 = idl.new_var();
    nc = core->new_clause({idl.new_distance(tp1, origin, 0)});
    assert(nc);
    nc = core->new_clause({idl.new_distance(horizon, tp1, 0)});
    assert(nc);
    prop = core->propagate();
    assert(prop);

    var tp2 = idl.new_var();
    nc = core->new_clause({idl.new_distance(tp2, origin, 0)});
    assert(nc);
    nc = core->new_clause({idl.new_distance(horizon, tp2, 0)});
    assert(nc);
    prop = core->propagate();
    assert(prop);

    nc = core->new_clause({idl.new_distance(tp0, tp1, 10)});
    assert(nc);
    nc = core->new_clause({idl.new_distance(tp1, tp0, 0)});
    assert(nc);

    nc = core->new_clause({idl.new_distance(tp1, tp2, 10)});
    assert(nc);
    nc = core->new_clause({idl.new_distance(tp2, tp1, 0)});
    assert(nc);

    nc = core->new_clause({idl.new_distance(origin, tp0, 10)});
    assert(nc);
    nc = core->new_clause({idl.new_distance(tp0, origin, 0)});
    assert(nc);
    prop = core->propagate();
    assert(prop);

    lit tp2_after_40 = idl.new_distance(tp2, origin, -40);
    assert(core->value(tp2_after_40) == FALSE_var);

    lit tp2_after_20 = idl.new_distance(tp2, origin, -20);
    lit tp2_before_20 = idl.new_distance(origin, tp2, 20);

    lit tp2_after_30 = idl.new_distance(tp2, origin, -30);
    lit tp2_before_30 = idl.new_distance(origin, tp2, 30);

    nc = core->new_clause({core->new_disj({core->new_conj({tp2_after_20, tp2_before_20}), core->new_conj({tp2_after_30, tp2_before_30})})});
    assert(nc);

    nc = core->new_clause({idl.new_distance(origin, tp2, 30)});
    assert(nc);
    nc = core->new_clause({idl.new_distance(tp2, origin, -25)});
    assert(nc);

    prop = core->propagate();
    assert(prop);

    std::pair<utils::I, utils::I> dist_origin_tp2 = idl.distance(origin, tp2);
    assert(dist_origin_tp2.first == 30 && dist_origin_tp2.second == 30);
}

void test_real_distance_logic()
{
    auto core = sat_ptr(new sat_core());
    rdl_theory rdl(core, 5);
    var origin = rdl.new_var();
    var horizon = rdl.new_var();
    bool nc = core->new_clause({rdl.new_distance(horizon, origin, utils::inf_rational(utils::rational::ZERO))});
    assert(nc);

    bool prop = core->propagate();
    assert(prop);

    var tp0 = rdl.new_var();
    nc = core->new_clause({rdl.new_distance(tp0, origin, utils::inf_rational(utils::rational::ZERO))});
    assert(nc);
    nc = core->new_clause({rdl.new_distance(horizon, tp0, utils::inf_rational(utils::rational::ZERO))});
    assert(nc);

    var tp1 = rdl.new_var();
    nc = core->new_clause({rdl.new_distance(tp1, origin, utils::inf_rational(utils::rational::ZERO))});
    assert(nc);
    nc = core->new_clause({rdl.new_distance(horizon, tp1, utils::inf_rational(utils::rational::ZERO))});
    assert(nc);
    prop = core->propagate();
    assert(prop);

    var tp2 = rdl.new_var();
    nc = core->new_clause({rdl.new_distance(tp2, origin, utils::inf_rational(utils::rational::ZERO))});
    assert(nc);
    nc = core->new_clause({rdl.new_distance(horizon, tp2, utils::inf_rational(utils::rational::ZERO))});
    assert(nc);
    prop = core->propagate();
    assert(prop);

    nc = core->new_clause({rdl.new_distance(tp0, tp1, utils::inf_rational(10))});
    assert(nc);
    nc = core->new_clause({rdl.new_distance(tp1, tp0, utils::inf_rational(utils::rational::ZERO))});
    assert(nc);

    nc = core->new_clause({rdl.new_distance(tp1, tp2, utils::inf_rational(10))});
    assert(nc);
    nc = core->new_clause({rdl.new_distance(tp2, tp1, utils::inf_rational(utils::rational::ZERO))});
    assert(nc);

    nc = core->new_clause({rdl.new_distance(origin, tp0, utils::inf_rational(10))});
    assert(nc);
    nc = core->new_clause({rdl.new_distance(tp0, origin, utils::inf_rational(utils::rational::ZERO))});
    assert(nc);
    prop = core->propagate();
    assert(prop);

    lit tp2_after_40 = rdl.new_distance(tp2, origin, -utils::inf_rational(40));
    assert(tp2_after_40 == FALSE_lit);

    lit tp2_after_20 = rdl.new_distance(tp2, origin, -utils::inf_rational(20));
    lit tp2_before_20 = rdl.new_distance(origin, tp2, utils::inf_rational(20));

    lit tp2_after_30 = rdl.new_distance(tp2, origin, -utils::inf_rational(30));
    lit tp2_before_30 = rdl.new_distance(origin, tp2, utils::inf_rational(30));

    nc = core->new_clause({core->new_disj({core->new_conj({tp2_after_20, tp2_before_20}), core->new_conj({tp2_after_30, tp2_before_30})})});
    assert(nc);

    nc = core->new_clause({rdl.new_distance(origin, tp2, utils::inf_rational(30))});
    assert(nc);
    nc = core->new_clause({rdl.new_distance(tp2, origin, -utils::inf_rational(25))});
    assert(nc);

    prop = core->propagate();
    assert(prop);

    std::pair<utils::inf_rational, utils::inf_rational> dist_origin_tp2 = rdl.distance(origin, tp2);
    assert(dist_origin_tp2.first == 30 && dist_origin_tp2.second == 30);
}

void test_constraints_0()
{
    auto core = sat_ptr(new sat_core());
    idl_theory idl(core, 5);
    var origin = idl.new_var();
    // origin >= 0..
    bool nc = core->new_clause({idl.new_distance(origin, 0, 0)});
    assert(nc);
    bool prop = core->propagate();
    assert(prop);
    std::pair<utils::I, utils::I> bound_origin = idl.bounds(origin);
    assert(bound_origin.first == 0 && bound_origin.second == idl_theory::inf());

    var horizon = idl.new_var();
    // horizon >= origin..
    nc = core->new_clause({idl.new_distance(horizon, origin, 0)});
    assert(nc);
    prop = core->propagate();
    assert(prop);
    std::pair<utils::I, utils::I> bound_horizon = idl.bounds(horizon);
    assert(bound_horizon.first == 0 && bound_horizon.second == idl_theory::inf());

    // horizon < 20..
    lit horizon_lt_20 = idl.new_lt(lin(horizon, utils::rational::ONE), lin(utils::rational(20)));
    assert(bound_horizon == idl.bounds(horizon));

    nc = core->new_clause({horizon_lt_20});
    assert(nc);
    prop = core->propagate();
    assert(prop);
    bound_horizon = idl.bounds(horizon);
    assert(bound_horizon.first == 0 && bound_horizon.second == 19);

    // horizon <= 15..
    lit horizon_leq_15 = idl.new_leq(lin(horizon, utils::rational::ONE), lin(utils::rational(15)));
    assert(bound_horizon == idl.bounds(horizon));

    nc = core->new_clause({horizon_leq_15});
    assert(nc);
    prop = core->propagate();
    assert(prop);
    bound_horizon = idl.bounds(horizon);
    assert(bound_horizon.first == 0 && bound_horizon.second == 15);
    bound_origin = idl.bounds(origin);
    assert(bound_origin.first == 0 && bound_origin.second == 15);

    // origin > 5..
    lit origin_gt_5 = idl.new_gt(lin(origin, utils::rational::ONE), lin(utils::rational(5)));
    assert(bound_origin == idl.bounds(origin));

    nc = core->new_clause({origin_gt_5});
    assert(nc);
    prop = core->propagate();
    assert(prop);
    bound_origin = idl.bounds(origin);
    assert(bound_origin.first == 6 && bound_origin.second == 15);

    // origin >= 10..
    lit origin_geq_10 = idl.new_geq(lin(origin, utils::rational::ONE), lin(utils::rational(10)));
    assert(bound_origin == idl.bounds(origin));

    nc = core->new_clause({origin_geq_10});
    assert(nc);
    prop = core->propagate();
    assert(prop);
    bound_origin = idl.bounds(origin);
    assert(bound_origin.first == 10 && bound_origin.second == 15);
}

void test_constraints_1()
{
    auto core = sat_ptr(new sat_core());
    idl_theory idl(core, 5);
    var origin = idl.new_var();
    // origin >= 0..
    bool nc = core->new_clause({idl.new_distance(origin, 0, 0)});
    assert(nc);
    bool prop = core->propagate();
    assert(prop);
    std::pair<utils::I, utils::I> bound_origin = idl.bounds(origin);
    assert(bound_origin.first == 0 && bound_origin.second == idl_theory::inf());

    var horizon = idl.new_var();
    // horizon >= origin..
    nc = core->new_clause({idl.new_distance(horizon, origin, 0)});
    assert(nc);
    prop = core->propagate();
    assert(prop);
    std::pair<utils::I, utils::I> bound_horizon = idl.bounds(horizon);
    assert(bound_horizon.first == 0 && bound_horizon.second == idl_theory::inf());

    // -horizon >= -20..
    lit horizon_geq_20 = idl.new_geq(lin(horizon, -utils::rational::ONE), lin(utils::rational(-20)));
    assert(bound_horizon == idl.bounds(horizon));

    nc = core->new_clause({horizon_geq_20});
    assert(nc);
    prop = core->propagate();
    assert(prop);
    bound_horizon = idl.bounds(horizon);
    assert(bound_horizon.first == 0 && bound_horizon.second == 20);

    // -horizon > -15..
    lit horizon_gt_15 = idl.new_gt(lin(horizon, -utils::rational::ONE), lin(utils::rational(-15)));
    assert(bound_horizon == idl.bounds(horizon));

    nc = core->new_clause({horizon_gt_15});
    assert(nc);
    prop = core->propagate();
    assert(prop);
    bound_horizon = idl.bounds(horizon);
    assert(bound_horizon.first == 0 && bound_horizon.second == 14);
    bound_origin = idl.bounds(origin);
    assert(bound_origin.first == 0 && bound_origin.second == 14);

    // -origin <= -5..
    lit origin_leq_5 = idl.new_leq(lin(origin, -utils::rational::ONE), lin(utils::rational(-5)));
    assert(bound_origin == idl.bounds(origin));

    nc = core->new_clause({origin_leq_5});
    assert(nc);
    prop = core->propagate();
    assert(prop);
    bound_origin = idl.bounds(origin);
    assert(bound_origin.first == 5 && bound_origin.second == 14);

    // -origin < -10..
    lit origin_lt_10 = idl.new_lt(lin(origin, -utils::rational::ONE), lin(utils::rational(-10)));
    assert(bound_origin == idl.bounds(origin));

    nc = core->new_clause({origin_lt_10});
    assert(nc);
    prop = core->propagate();
    assert(prop);
    bound_origin = idl.bounds(origin);
    assert(bound_origin.first == 11 && bound_origin.second == 14);
}

void test_constraints_2()
{
    auto core = sat_ptr(new sat_core());
    rdl_theory rdl(core, 5);
    var origin = rdl.new_var();
    // origin >= 0..
    bool nc = core->new_clause({rdl.new_distance(origin, 0, utils::inf_rational(utils::rational::ZERO))});
    assert(nc);
    bool prop = core->propagate();
    assert(prop);
    std::pair<utils::inf_rational, utils::inf_rational> bound_origin = rdl.bounds(origin);
    assert(bound_origin.first == 0 && bound_origin.second == utils::rational::POSITIVE_INFINITY);

    var horizon = rdl.new_var();
    // horizon >= origin..
    nc = core->new_clause({rdl.new_distance(horizon, origin, utils::inf_rational(utils::rational::ZERO))});
    assert(nc);
    prop = core->propagate();
    assert(prop);
    std::pair<utils::inf_rational, utils::inf_rational> bound_horizon = rdl.bounds(horizon);
    assert(bound_horizon.first == 0 && bound_horizon.second == utils::rational::POSITIVE_INFINITY);

    // horizon < 20..
    lit horizon_lt_20 = rdl.new_lt(lin(horizon, utils::rational::ONE), lin(utils::rational(20)));
    assert(bound_horizon == rdl.bounds(horizon));

    nc = core->new_clause({horizon_lt_20});
    assert(nc);
    prop = core->propagate();
    assert(prop);
    bound_horizon = rdl.bounds(horizon);
    assert(bound_horizon.first == 0 && bound_horizon.second == utils::inf_rational(utils::rational(20), -1));

    // horizon <= 15..
    lit horizon_leq_15 = rdl.new_leq(lin(horizon, utils::rational::ONE), lin(utils::rational(15)));
    assert(bound_horizon == rdl.bounds(horizon));

    nc = core->new_clause({horizon_leq_15});
    assert(nc);
    prop = core->propagate();
    assert(prop);
    bound_horizon = rdl.bounds(horizon);
    assert(bound_horizon.first == 0 && bound_horizon.second == 15);
    bound_origin = rdl.bounds(origin);
    assert(bound_origin.first == 0 && bound_origin.second == 15);

    // origin > 5..
    lit origin_gt_5 = rdl.new_gt(lin(origin, utils::rational::ONE), lin(utils::rational(5)));
    assert(bound_origin == rdl.bounds(origin));

    nc = core->new_clause({origin_gt_5});
    assert(nc);
    prop = core->propagate();
    assert(prop);
    bound_origin = rdl.bounds(origin);
    assert(bound_origin.first == utils::inf_rational(utils::rational(5), 1) && bound_origin.second == 15);

    // origin >= 10..
    lit origin_geq_10 = rdl.new_geq(lin(origin, utils::rational::ONE), lin(utils::rational(10)));
    assert(bound_origin == rdl.bounds(origin));

    nc = core->new_clause({origin_geq_10});
    assert(nc);
    prop = core->propagate();
    assert(prop);
    bound_origin = rdl.bounds(origin);
    assert(bound_origin.first == 10 && bound_origin.second == 15);
}

void test_constraints_3()
{
    auto core = sat_ptr(new sat_core());
    rdl_theory rdl(core, 5);
    var origin = rdl.new_var();
    // origin >= 0..
    bool nc = core->new_clause({rdl.new_distance(origin, 0, utils::inf_rational(utils::rational::ZERO))});
    assert(nc);
    bool prop = core->propagate();
    assert(prop);
    std::pair<utils::inf_rational, utils::inf_rational> bound_origin = rdl.bounds(origin);
    assert(bound_origin.first == 0 && bound_origin.second == utils::rational::POSITIVE_INFINITY);

    var horizon = rdl.new_var();
    // horizon >= origin..
    nc = core->new_clause({rdl.new_distance(horizon, origin, utils::inf_rational(utils::rational::ZERO))});
    assert(nc);
    prop = core->propagate();
    assert(prop);
    std::pair<utils::inf_rational, utils::inf_rational> bound_horizon = rdl.bounds(horizon);
    assert(bound_horizon.first == 0 && bound_horizon.second == utils::rational::POSITIVE_INFINITY);

    // -horizon >= -20..
    lit horizon_geq_20 = rdl.new_geq(lin(horizon, -utils::rational::ONE), lin(utils::rational(-20)));
    assert(bound_horizon == rdl.bounds(horizon));

    nc = core->new_clause({horizon_geq_20});
    assert(nc);
    prop = core->propagate();
    assert(prop);
    bound_horizon = rdl.bounds(horizon);
    assert(bound_horizon.first == 0 && bound_horizon.second == 20);

    // -horizon > -15..
    lit horizon_gt_15 = rdl.new_gt(lin(horizon, -utils::rational::ONE), lin(utils::rational(-15)));
    assert(bound_horizon == rdl.bounds(horizon));

    nc = core->new_clause({horizon_gt_15});
    assert(nc);
    prop = core->propagate();
    assert(prop);
    bound_horizon = rdl.bounds(horizon);
    assert(bound_horizon.first == 0 && bound_horizon.second == utils::inf_rational(utils::rational(15), -1));
    bound_origin = rdl.bounds(origin);
    assert(bound_origin.first == 0 && bound_origin.second == utils::inf_rational(utils::rational(15), -1));

    // -origin <= -5..
    lit origin_leq_5 = rdl.new_leq(lin(origin, -utils::rational::ONE), lin(utils::rational(-5)));
    assert(bound_origin == rdl.bounds(origin));

    nc = core->new_clause({origin_leq_5});
    assert(nc);
    prop = core->propagate();
    assert(prop);
    bound_origin = rdl.bounds(origin);
    assert(bound_origin.first == 5 && bound_origin.second == utils::inf_rational(utils::rational(15), -1));

    // -origin < -10..
    lit origin_lt_10 = rdl.new_lt(lin(origin, -utils::rational::ONE), lin(utils::rational(-10)));
    assert(bound_origin == rdl.bounds(origin));

    nc = core->new_clause({origin_lt_10});
    assert(nc);
    prop = core->propagate();
    assert(prop);
    bound_origin = rdl.bounds(origin);
    assert(bound_origin.first == utils::inf_rational(utils::rational(10), 1) && bound_origin.second == utils::inf_rational(utils::rational(15), -1));
}

void test_constraints_4()
{
    auto core = sat_ptr(new sat_core());
    rdl_theory rdl(core, 5);
    var origin = rdl.new_var();
    // origin >= 0..
    bool nc = core->new_clause({rdl.new_geq(lin(origin, utils::rational::ONE), lin(utils::rational::ZERO))});
    assert(nc);
    bool prop = core->propagate();
    assert(prop);
    std::pair<utils::inf_rational, utils::inf_rational> bound_origin = rdl.bounds(origin);
    assert(bound_origin.first == 0 && bound_origin.second == utils::rational::POSITIVE_INFINITY);

    var horizon = rdl.new_var();
    // horizon >= origin..
    nc = core->new_clause({rdl.new_geq(lin(horizon, utils::rational::ONE), lin(origin, utils::rational::ONE))});
    assert(nc);
    prop = core->propagate();
    assert(prop);
    std::pair<utils::inf_rational, utils::inf_rational> bound_horizon = rdl.bounds(horizon);
    assert(bound_horizon.first == 0 && bound_horizon.second == utils::rational::POSITIVE_INFINITY);

    lit hor_eq_10 = rdl.new_eq(lin(horizon, utils::rational::ONE), lin(utils::rational(10)));
    assert(bound_horizon == rdl.bounds(horizon));
    assert(bound_origin == rdl.bounds(origin));

    nc = core->new_clause({hor_eq_10});
    assert(nc);
    prop = core->propagate();
    assert(prop);
    bound_origin = rdl.bounds(origin);
    assert(bound_origin.first == 0 && bound_origin.second == 10);
    bound_horizon = rdl.bounds(horizon);
    assert(bound_horizon.first == 10 && bound_horizon.second == 10);
}

void test_constraints_5()
{
    auto core = sat_ptr(new sat_core());
    rdl_theory rdl(core, 5);
    var origin = rdl.new_var();
    // origin >= 0..
    bool nc = core->new_clause({rdl.new_geq(lin(origin, utils::rational::ONE), lin(utils::rational::ZERO))});
    assert(nc);
    bool prop = core->propagate();
    assert(prop);
    std::pair<utils::inf_rational, utils::inf_rational> bound_origin = rdl.bounds(origin);
    assert(bound_origin.first == 0 && bound_origin.second == utils::rational::POSITIVE_INFINITY);

    var horizon = rdl.new_var();
    // horizon >= origin..
    nc = core->new_clause({rdl.new_geq(lin(horizon, utils::rational::ONE), lin(origin, utils::rational::ONE))});
    assert(nc);
    prop = core->propagate();
    assert(prop);
    std::pair<utils::inf_rational, utils::inf_rational> bound_horizon = rdl.bounds(horizon);
    assert(bound_horizon.first == 0 && bound_horizon.second == utils::rational::POSITIVE_INFINITY);

    var v1 = rdl.new_var(), v2 = rdl.new_var(), v3 = rdl.new_var(), v4 = rdl.new_var();
    nc = core->new_clause({rdl.new_distance(v1, origin, utils::inf_rational(utils::rational::ZERO))});
    assert(nc);
    nc = core->new_clause({rdl.new_distance(horizon, v1, utils::inf_rational(utils::rational::ZERO))});
    assert(nc);
    nc = core->new_clause({rdl.new_distance(v2, origin, utils::inf_rational(utils::rational::ZERO))});
    assert(nc);
    nc = core->new_clause({rdl.new_distance(horizon, v2, utils::inf_rational(utils::rational::ZERO))});
    assert(nc);
    nc = core->new_clause({rdl.new_distance(v3, origin, utils::inf_rational(utils::rational::ZERO))});
    assert(nc);
    nc = core->new_clause({rdl.new_distance(horizon, v3, utils::inf_rational(utils::rational::ZERO))});
    assert(nc);
    nc = core->new_clause({rdl.new_distance(v4, origin, utils::inf_rational(utils::rational::ZERO))});
    assert(nc);
    nc = core->new_clause({rdl.new_distance(horizon, v4, utils::inf_rational(utils::rational::ZERO))});
    assert(nc);

    nc = core->new_clause({rdl.new_distance(v1, v2, utils::inf_rational(20), utils::inf_rational(20))});
    assert(nc);
    nc = core->new_clause({rdl.new_distance(v3, v4, utils::inf_rational(20), utils::inf_rational(20))});
    assert(nc);

    lit v2_v3 = rdl.new_leq(lin(v2, utils::rational::ONE), lin(v3, utils::rational::ONE));
    assert(core->value(v2_v3) == Undefined);
    lit v4_v1 = rdl.new_leq(lin(v4, utils::rational::ONE), lin(v1, utils::rational::ONE));
    assert(core->value(v4_v1) == Undefined);

    nc = core->new_clause({rdl.new_distance(v1, v3, utils::inf_rational(utils::rational::ZERO), utils::inf_rational(utils::rational::ZERO))});
    assert(nc);
    prop = core->propagate();
    assert(prop);
    assert(core->value(v2_v3) == False);
    assert(core->value(v4_v1) == False);
}

void test_semantic_branching()
{
    auto core = sat_ptr(new sat_core());
    rdl_theory rdl(core, 5);
    var origin = rdl.new_var();
    // origin >= 0..
    bool nc = core->new_clause({rdl.new_geq(lin(origin, utils::rational::ONE), lin(utils::rational::ZERO))});
    assert(nc);
    bool prop = core->propagate();
    assert(prop);
    std::pair<utils::inf_rational, utils::inf_rational> bound_origin = rdl.bounds(origin);
    assert(bound_origin.first == 0 && bound_origin.second == utils::rational::POSITIVE_INFINITY);

    var horizon = rdl.new_var();
    // horizon >= origin..
    nc = core->new_clause({rdl.new_geq(lin(horizon, utils::rational::ONE), lin(origin, utils::rational::ONE))});
    assert(nc);
    prop = core->propagate();
    assert(prop);
    std::pair<utils::inf_rational, utils::inf_rational> bound_horizon = rdl.bounds(horizon);
    assert(bound_horizon.first == 0 && bound_horizon.second == utils::rational::POSITIVE_INFINITY);

    lit hor_leq_10 = rdl.new_leq(lin(horizon, utils::rational::ONE), lin(utils::rational(10)));
    assert(bound_horizon == rdl.bounds(horizon));
    assert(bound_origin == rdl.bounds(origin));

    nc = core->new_clause({!hor_leq_10});
    assert(nc);
    prop = core->propagate();
    assert(prop);
    bound_origin = rdl.bounds(origin);
    assert(bound_origin.first == 0 && bound_origin.second == utils::rational::POSITIVE_INFINITY);
    bound_horizon = rdl.bounds(horizon);
    assert(bound_horizon.first == utils::inf_rational(utils::rational(10), 1) && bound_horizon.second == utils::rational::POSITIVE_INFINITY);

    lit hor_geq_20 = rdl.new_geq(lin(horizon, utils::rational::ONE), lin(utils::rational(20)));
    assert(bound_horizon == rdl.bounds(horizon));
    assert(bound_origin == rdl.bounds(origin));

    nc = core->new_clause({!hor_geq_20});
    assert(nc);
    prop = core->propagate();
    assert(prop);
    bound_origin = rdl.bounds(origin);
    assert(bound_origin.first == 0 && bound_origin.second == utils::inf_rational(utils::rational(20), -1));
    bound_horizon = rdl.bounds(horizon);
    assert(bound_horizon.first == utils::inf_rational(utils::rational(10), 1) && bound_horizon.second == utils::inf_rational(utils::rational(20), -1));
}

int main(int, char **)
{
    test_config();
    test_real_distance_logic();

    test_constraints_0();
    test_constraints_1();
    test_constraints_2();
    test_constraints_3();

    test_constraints_4();
    test_constraints_5();

    test_semantic_branching();
}