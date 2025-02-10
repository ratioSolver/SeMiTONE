#include "network.hpp"
#include "logging.hpp"
#include "floyd_warshall.hpp"
#include <cassert>

void test_network0()
{
    semitone::network net;
    utils::var b0 = net.new_var();
    utils::var b1 = net.new_var();
    utils::var b2 = net.new_var();

    net.add_clause({utils::lit(b0, false), !utils::lit(b1), utils::lit(b2)});
    bool ch = net.propagate();
    assert(ch);
    assert(net.value(b0) == utils::Undefined);
    assert(net.value(b1) == utils::Undefined);
    assert(net.value(b2) == utils::Undefined);

    bool assm = net.assume(utils::lit(b0));
    assert(assm);
    assert(net.value(b0) == utils::True);
    assert(net.value(b1) == utils::Undefined);
    assert(net.value(b2) == utils::Undefined);

    assm = net.assume(utils::lit(b1));
    assert(assm);
    assert(net.value(b0) == utils::True);
    assert(net.value(b1) == utils::True);
    assert(net.value(b2) == utils::True);
}

void test_no_good()
{
    semitone::network net;

    utils::var b0 = net.new_var();
    utils::var b1 = net.new_var();
    utils::var b2 = net.new_var();
    utils::var b3 = net.new_var();
    utils::var b4 = net.new_var();
    utils::var b5 = net.new_var();
    utils::var b6 = net.new_var();
    utils::var b7 = net.new_var();
    utils::var b8 = net.new_var();

    net.add_clause({utils::lit(b0), utils::lit(b1)});
    net.add_clause({utils::lit(b0), utils::lit(b2), utils::lit(b6)});
    net.add_clause({utils::lit(b1, false), utils::lit(b2, false), utils::lit(b3)});
    net.add_clause({utils::lit(b3, false), utils::lit(b4), utils::lit(b7)});
    net.add_clause({utils::lit(b3, false), utils::lit(b5), utils::lit(b8)});
    net.add_clause({utils::lit(b4, false), utils::lit(b5, false)});

    bool prop = net.propagate();
    assert(prop);

    bool assm = net.assume(utils::lit(b6, false));
    assert(assm);
    assm = net.assume(utils::lit(b7, false));
    assert(assm);
    assm = net.assume(utils::lit(b8, false));
    assert(assm);
    assm = net.assume(utils::lit(b0, false));
    assert(assm);
}

void test_net()
{
    semitone::network net;

    auto x = net.new_real();
    auto y = net.new_real();
    auto s1 = net.new_real(utils::lin(x, -utils::rational::one) + utils::lin(y, utils::rational::one));
    auto s2 = net.new_real(utils::lin(x, utils::rational::one) + utils::lin(y, utils::rational::one));

    // x <= -4
    auto x_le_m4 = net.new_var();
    net.new_le(utils::lit(x_le_m4), utils::lin(x, utils::rational::one), utils::lin(utils::rational(-4)));
    // x >= -8
    auto x_ge_m8 = net.new_var();
    net.new_le(utils::lit(x_ge_m8), utils::lin(utils::rational(-8)), utils::lin(x, utils::rational::one));
    // s1 <= 1
    auto s1_le_1 = net.new_var();
    net.new_le(utils::lit(s1_le_1), utils::lin(s1, utils::rational::one), utils::lin(utils::rational::one));
    // s2 >= -3
    auto s2_geq_m3 = net.new_var();
    net.new_le(utils::lit(s2_geq_m3), utils::lin(utils::rational(-3)), utils::lin(s2, utils::rational::one));

    bool prop = net.propagate();
    assert(prop);

    auto a = net.assume(utils::lit(x_le_m4));
    assert(a);
    a = net.assume(utils::lit(x_ge_m8));
    assert(a);
    a = net.assume(utils::lit(s1_le_1));
    assert(a);
    a = net.assume(utils::lit(s2_geq_m3));
    assert(a);
    assert(net.value(s2_geq_m3) == utils::False);
}

void test_dl()
{
    semitone::network net;

    auto tp0 = net.new_tp();
    auto tp1 = net.new_tp();
    auto tp2 = net.new_tp();

    // origin -[3, 7]-> tp0
    auto origin_3_7_tp0 = net.new_var();
    net.new_distance(utils::lit(origin_3_7_tp0), 0, tp0, utils::rational(3), utils::rational(7));
    // tp0 -[2, 5]-> tp1
    auto tp0_2_5_tp1 = net.new_var();
    net.new_distance(utils::lit(tp0_2_5_tp1), tp0, tp1, utils::rational(2), utils::rational(5));
    // tp1 -[0, 10]-> tp2
    auto tp1_0_10_tp2 = net.new_var();
    net.new_distance(utils::lit(tp1_0_10_tp2), tp1, tp2, utils::rational(0), utils::rational(10));

    bool prop = net.propagate();
    assert(prop);

    auto a = net.assume(utils::lit(origin_3_7_tp0));
    assert(a);
    a = net.assume(utils::lit(tp0_2_5_tp1));
    assert(a);
    a = net.assume(utils::lit(tp1_0_10_tp2));
    assert(a);

    utils::floyd_warshall<double, 4> fw;
    fw.add_edge(0, 1, 7.0);
    fw.add_edge(1, 0, -3.0);
    fw.add_edge(1, 2, 5.0);
    fw.add_edge(2, 1, -2.0);
    fw.add_edge(2, 3, 10.0);
    fw.add_edge(3, 2, 0.0);

    fw.compute_all_pairs_shortest_paths();
    LOG_TRACE(fw);

    assert(net.value(origin_3_7_tp0) == utils::True);
    assert(net.value(tp0_2_5_tp1) == utils::True);
    assert(net.value(tp1_0_10_tp2) == utils::True);

    assert(net.tp_lb(tp0) == -fw.get_distance(1, 0));
    assert(net.tp_ub(tp0) == fw.get_distance(0, 1));
    assert(net.tp_lb(tp1) == -fw.get_distance(2, 0));
    assert(net.tp_ub(tp1) == fw.get_distance(0, 2));
    assert(net.tp_lb(tp2) == -fw.get_distance(3, 0));
    assert(net.tp_ub(tp2) == fw.get_distance(0, 3));
}

int main()
{
    test_network0();
    test_no_good();

    test_net();

    test_dl();

    return 0;
}
