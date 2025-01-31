#include "solver.hpp"

void test_solver0()
{
    semitone::context ctx;
    auto and_xpr = ctx.mk_and({ctx.mk_not(ctx.mk_bool_var("x")), ctx.mk_bool_var("y"), ctx.mk_bool_var("z")});
    auto or_xpr = ctx.mk_or({ctx.mk_bool_var("x"), ctx.mk_bool_var("y"), ctx.mk_bool_var("z")});

    semitone::solver solver(ctx);
    solver.add(and_xpr);
    solver.add(or_xpr);
}

int main()
{
    test_solver0();
    return 0;
}
