#include "network.hpp"
#include <cassert>

void test_network0()
{
    semitone::network net;
    auto x = net.new_var();
    assert(net.value(x) == utils::Undefined);
}

int main()
{
    test_network0();
    return 0;
}
