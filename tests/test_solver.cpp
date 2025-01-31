#include "solver.hpp"
#include <cassert>

void test_solver0()
{
    semitone::context ctx;
    auto x = ctx.mk_bool_var("x");
    auto y = ctx.mk_bool_var("y");
    auto z = ctx.mk_bool_var("z");
    auto and_xpr = ctx.mk_and({!x, y});
    auto or_xpr = ctx.mk_or({x, y, z});

    semitone::solver slv(ctx);
    slv.add(and_xpr);
    slv.add(or_xpr);

    slv.eval(and_xpr);
    assert(x->val() == utils::False);
    assert(y->val() == utils::True);
    assert(z->val() == utils::Undefined);
}

int main()
{
    test_solver0();
    return 0;
}
