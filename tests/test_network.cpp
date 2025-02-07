#include "network.hpp"
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

int main()
{
    test_network0();
    test_no_good();
    return 0;
}
