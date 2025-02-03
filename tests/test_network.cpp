#include "network.hpp"
#include <cassert>

void test_solver0()
{
    semitone::context ctx;
    auto a = ctx.mk_bool_var("a");
    auto b = ctx.mk_bool_var("b");
    auto c = ctx.mk_bool_var("c");
    auto d = ctx.mk_bool_var("d");
    auto and_xpr = ctx.mk_and({!a, b});
    auto or_xpr = ctx.mk_or({a, c});

    semitone::network slv(ctx);
    slv.add(and_xpr);
    slv.add(or_xpr);

    bool sat = slv.propagate();
    assert(sat);

    assert(slv.eval(a) == utils::False);
    assert(slv.eval(b) == utils::True);
    assert(slv.eval(c) == utils::True);
    assert(slv.eval(d) == utils::Undefined);
}

int main()
{
    test_solver0();
    return 0;
}
