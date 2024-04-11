#include "sat_core.hpp"
#include "idl_theory.hpp"
#include "rdl_theory.hpp"
#include "logging.hpp"
#include <cassert>

VARIABLE_TYPE new_var(semitone::idl_theory &dl, VARIABLE_TYPE horizon)
{
    auto var = dl.new_var();
    bool nc = dl.get_sat().new_clause({dl.new_distance(var, 0, 0)}) && dl.get_sat().new_clause({dl.new_distance(horizon, var, 0)});
    assert(nc);
    bool prop = dl.get_sat().propagate();
    assert(prop);
    return var;
}

VARIABLE_TYPE new_var(semitone::rdl_theory &dl, VARIABLE_TYPE horizon)
{
    auto var = dl.new_var();
    bool nc = dl.get_sat().new_clause({dl.new_distance(var, 0, utils::inf_rational::zero)}) && dl.get_sat().new_clause({dl.new_distance(horizon, var, utils::inf_rational::zero)});
    assert(nc);
    bool prop = dl.get_sat().propagate();
    assert(prop);
    return var;
}

void test_idl()
{
    semitone::sat_core sat;
    auto &dl = sat.new_theory<semitone::idl_theory>();

    auto horizon = dl.new_var();

    bool nc = sat.new_clause({dl.new_distance(horizon, 0, 0)});
    assert(nc);
    bool prop = sat.propagate();
    assert(prop);

    VARIABLE_TYPE tp0 = new_var(dl, horizon);
    VARIABLE_TYPE tp1 = new_var(dl, horizon);
    VARIABLE_TYPE tp2 = new_var(dl, horizon);

    nc = sat.new_clause({dl.new_distance(tp0, tp1, 0, 10)});
    assert(nc);
    nc = sat.new_clause({dl.new_distance(tp1, tp2, 0, 10)});
    assert(nc);
    nc = sat.new_clause({dl.new_distance(0, tp0, 0, 10)});
    assert(nc);
    prop = sat.propagate();
    assert(prop);

    auto bounds_tp0 = dl.bounds(tp0);
    LOG_DEBUG("Bounds of tp0: [" << bounds_tp0.first << ", " << bounds_tp0.second << "]");
    auto bounds_tp1 = dl.bounds(tp1);
    LOG_DEBUG("Bounds of tp1: [" << bounds_tp1.first << ", " << bounds_tp1.second << "]");
    auto bounds_tp2 = dl.bounds(tp2);
    LOG_DEBUG("Bounds of tp2: [" << bounds_tp2.first << ", " << bounds_tp2.second << "]");

    auto or_tp0 = dl.distance(0, tp0);
    LOG_DEBUG("Distance from 0 to tp0: [" << or_tp0.first << ", " << or_tp0.second << "]");
    auto tp0_tp1 = dl.distance(tp0, tp1);
    LOG_DEBUG("Distance from tp0 to tp1: [" << tp0_tp1.first << ", " << tp0_tp1.second << "]");
    auto tp1_tp2 = dl.distance(tp1, tp2);
    LOG_DEBUG("Distance from tp1 to tp2: [" << tp1_tp2.first << ", " << tp1_tp2.second << "]");
    auto or_tp1 = dl.distance(0, tp1);
    LOG_DEBUG("Distance from 0 to tp1: [" << or_tp1.first << ", " << or_tp1.second << "]");
    auto or_tp2 = dl.distance(0, tp2);
    LOG_DEBUG("Distance from 0 to tp2: [" << or_tp2.first << ", " << or_tp2.second << "]");
    auto tp0_tp2 = dl.distance(tp0, tp2);
    LOG_DEBUG("Distance from tp0 to tp2: [" << tp0_tp2.first << ", " << tp0_tp2.second << "]");
}

void test_rdl()
{
    semitone::sat_core sat;
    auto &dl = sat.new_theory<semitone::rdl_theory>();

    auto horizon = dl.new_var();

    bool nc = sat.new_clause({dl.new_distance(horizon, 0, utils::inf_rational::zero)});
    assert(nc);
    bool prop = sat.propagate();
    assert(prop);

    VARIABLE_TYPE tp0 = new_var(dl, horizon);
    VARIABLE_TYPE tp1 = new_var(dl, horizon);
    VARIABLE_TYPE tp2 = new_var(dl, horizon);

    nc = sat.new_clause({dl.new_distance(tp0, tp1, utils::inf_rational::zero, utils::inf_rational(10))});
    assert(nc);
    nc = sat.new_clause({dl.new_distance(tp1, tp2, utils::inf_rational::zero, utils::inf_rational(10))});
    assert(nc);
    nc = sat.new_clause({dl.new_distance(0, tp0, utils::inf_rational::zero, utils::inf_rational(10))});
    assert(nc);
    prop = sat.propagate();
    assert(prop);

    auto bounds_tp0 = dl.bounds(tp0);
    LOG_DEBUG("Bounds of tp0: [" << to_string(bounds_tp0.first) << ", " << to_string(bounds_tp0.second) << "]");
    auto bounds_tp1 = dl.bounds(tp1);
    LOG_DEBUG("Bounds of tp1: [" << to_string(bounds_tp1.first) << ", " << to_string(bounds_tp1.second) << "]");
    auto bounds_tp2 = dl.bounds(tp2);
    LOG_DEBUG("Bounds of tp2: [" << to_string(bounds_tp2.first) << ", " << to_string(bounds_tp2.second) << "]");

    auto or_tp0 = dl.distance(0, tp0);
    LOG_DEBUG("Distance from 0 to tp0: [" << to_string(or_tp0.first) << ", " << to_string(or_tp0.second) << "]");
    auto tp0_tp1 = dl.distance(tp0, tp1);
    LOG_DEBUG("Distance from tp0 to tp1: [" << to_string(tp0_tp1.first) << ", " << to_string(tp0_tp1.second) << "]");
    auto tp1_tp2 = dl.distance(tp1, tp2);
    LOG_DEBUG("Distance from tp1 to tp2: [" << to_string(tp1_tp2.first) << ", " << to_string(tp1_tp2.second) << "]");
    auto or_tp1 = dl.distance(0, tp1);
    LOG_DEBUG("Distance from 0 to tp1: [" << to_string(or_tp1.first) << ", " << to_string(or_tp1.second) << "]");
    auto or_tp2 = dl.distance(0, tp2);
    LOG_DEBUG("Distance from 0 to tp2: [" << to_string(or_tp2.first) << ", " << to_string(or_tp2.second) << "]");
    auto tp0_tp2 = dl.distance(tp0, tp2);
    LOG_DEBUG("Distance from tp0 to tp2: [" << to_string(tp0_tp2.first) << ", " << to_string(tp0_tp2.second) << "]");
}

int main(int argc, char const *argv[])
{
    test_idl();
    test_rdl();

    return 0;
}
