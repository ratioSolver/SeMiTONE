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

int main()
{
    test_network0();
    return 0;
}
