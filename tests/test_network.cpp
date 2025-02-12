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

    net.new_clause({utils::lit(b0, false), !utils::lit(b1), utils::lit(b2)});
    net.propagate();
    assert(net.value(b0) == utils::Undefined);
    assert(net.value(b1) == utils::Undefined);
    assert(net.value(b2) == utils::Undefined);

    net.assume(utils::lit(b0));
    assert(net.value(b0) == utils::True);
    assert(net.value(b1) == utils::Undefined);
    assert(net.value(b2) == utils::Undefined);

    net.assume(utils::lit(b1));
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

    net.new_clause({utils::lit(b0), utils::lit(b1)});
    net.new_clause({utils::lit(b0), utils::lit(b2), utils::lit(b6)});
    net.new_clause({utils::lit(b1, false), utils::lit(b2, false), utils::lit(b3)});
    net.new_clause({utils::lit(b3, false), utils::lit(b4), utils::lit(b7)});
    net.new_clause({utils::lit(b3, false), utils::lit(b5), utils::lit(b8)});
    net.new_clause({utils::lit(b4, false), utils::lit(b5, false)});

    net.propagate();

    net.assume(utils::lit(b6, false));
    net.assume(utils::lit(b7, false));
    net.assume(utils::lit(b8, false));
    net.assume(utils::lit(b0, false));
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
    net.new_le(utils::lin(x, utils::rational::one), utils::lin(utils::rational(-4)), utils::lit(x_le_m4));
    // x >= -8
    auto x_ge_m8 = net.new_var();
    net.new_le(utils::lin(utils::rational(-8)), utils::lin(x, utils::rational::one), utils::lit(x_ge_m8));
    // s1 <= 1
    auto s1_le_1 = net.new_var();
    net.new_le(utils::lin(s1, utils::rational::one), utils::lin(utils::rational::one), utils::lit(s1_le_1));
    // s2 >= -3
    auto s2_geq_m3 = net.new_var();
    net.new_le(utils::lin(utils::rational(-3)), utils::lin(s2, utils::rational::one), utils::lit(s2_geq_m3));

    net.propagate();

    net.assume(utils::lit(x_le_m4));
    net.assume(utils::lit(x_ge_m8));
    net.assume(utils::lit(s1_le_1));
    net.assume(utils::lit(s2_geq_m3));
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
    net.new_distance(0, tp0, utils::rational(3), utils::rational(7), utils::lit(origin_3_7_tp0));
    // tp0 -[2, 5]-> tp1
    auto tp0_2_5_tp1 = net.new_var();
    net.new_distance(tp0, tp1, utils::rational(2), utils::rational(5), utils::lit(tp0_2_5_tp1));
    // tp1 -[0, 10]-> tp2
    auto tp1_0_10_tp2 = net.new_var();
    net.new_distance(tp1, tp2, utils::rational(0), utils::rational(10), utils::lit(tp1_0_10_tp2));
    // origin -[2, 4]-> tp2
    auto origin_2_4_tp2 = net.new_var();
    net.new_distance(0, tp2, utils::rational(2), utils::rational(4), utils::lit(origin_2_4_tp2));

    net.propagate();

    net.assume(utils::lit(origin_3_7_tp0));
    net.assume(utils::lit(tp0_2_5_tp1));
    net.assume(utils::lit(tp1_0_10_tp2));

    // we apply Floyd-Warshall algorithm to check the results..
    utils::floyd_warshall<double, 4> fw;
    fw.add_edge(0, 1, 7.0);
    fw.add_edge(1, 0, -3.0);
    fw.add_edge(1, 2, 5.0);
    fw.add_edge(2, 1, -2.0);
    fw.add_edge(2, 3, 10.0);
    fw.add_edge(3, 2, 0.0);

    fw.compute_all_pairs_shortest_paths();

    assert(net.value(origin_3_7_tp0) == utils::True);
    assert(net.value(tp0_2_5_tp1) == utils::True);
    assert(net.value(tp1_0_10_tp2) == utils::True);
    assert(net.value(origin_2_4_tp2) == utils::False);

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
