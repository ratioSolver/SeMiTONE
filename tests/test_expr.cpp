#include "context.hpp"
#include <cassert>

void test_bools()
{
    semitone::context ctx;
    auto and_xpr = ctx.mk_and({ctx.mk_bool_const(utils::True), ctx.mk_bool_const(utils::False)});
    auto or_xpr = ctx.mk_or({ctx.mk_bool_const(utils::True), ctx.mk_bool_const(utils::False)});
    auto not_xpr = ctx.mk_not(ctx.mk_bool_const(utils::True));

    assert(and_xpr->val() == utils::False);
    assert(or_xpr->val() == utils::True);
    assert(not_xpr->val() == utils::False);
}

void test_ints()
{
    semitone::context ctx;
    std::vector<semitone::int_expr> sum_args = {ctx.mk_int_const(utils::integer::one), ctx.mk_int_const(utils::integer::one)};
    auto sum_xpr = ctx.mk_sum(std::move(sum_args));
    std::vector<semitone::int_expr> sub_args = {ctx.mk_int_const(utils::integer::one), ctx.mk_int_const(utils::integer::one)};
    auto sub_xpr = ctx.mk_sub(std::move(sub_args));
    std::vector<semitone::int_expr> mul_args = {ctx.mk_int_const(utils::integer::one), ctx.mk_int_const(utils::integer::one)};
    auto mul_xpr = ctx.mk_mul(std::move(mul_args));
    std::vector<semitone::int_expr> div_args = {ctx.mk_int_const(utils::integer::one), ctx.mk_int_const(utils::integer::one)};
    auto div_xpr = ctx.mk_div(std::move(div_args));

    assert(sum_xpr->val() == 2);
    assert(sub_xpr->val() == utils::integer::zero);
    assert(mul_xpr->val() == utils::integer::one);
    assert(div_xpr->val() == utils::integer::one);

    auto lt_xpr = ctx.mk_lt(ctx.mk_int_const(utils::integer::one), ctx.mk_int_const(utils::integer::one));
    auto le_xpr = ctx.mk_le(ctx.mk_int_const(utils::integer::one), ctx.mk_int_const(utils::integer::one));
    auto eq_xpr = ctx.mk_eq(ctx.mk_int_const(utils::integer::one), ctx.mk_int_const(utils::integer::one));
    auto ge_xpr = ctx.mk_ge(ctx.mk_int_const(utils::integer::one), ctx.mk_int_const(utils::integer::one));
    auto gt_xpr = ctx.mk_gt(ctx.mk_int_const(utils::integer::one), ctx.mk_int_const(utils::integer::one));

    assert(lt_xpr->val() == utils::False);
    assert(le_xpr->val() == utils::True);
    assert(eq_xpr->val() == utils::True);
    assert(ge_xpr->val() == utils::True);
    assert(gt_xpr->val() == utils::False);
}

void test_reals()
{
    semitone::context ctx;
    std::vector<semitone::real_expr> sum_args = {ctx.mk_real_const(utils::rational::one), ctx.mk_real_const(utils::rational::one)};
    auto sum_xpr = ctx.mk_sum(std::move(sum_args));
    std::vector<semitone::real_expr> sub_args = {ctx.mk_real_const(utils::rational::one), ctx.mk_real_const(utils::rational::one)};
    auto sub_xpr = ctx.mk_sub(std::move(sub_args));
    std::vector<semitone::real_expr> mul_args = {ctx.mk_real_const(utils::rational::one), ctx.mk_real_const(utils::rational::one)};
    auto mul_xpr = ctx.mk_mul(std::move(mul_args));
    std::vector<semitone::real_expr> div_args = {ctx.mk_real_const(utils::rational::one), ctx.mk_real_const(utils::rational::one)};
    auto div_xpr = ctx.mk_div(std::move(div_args));

    assert(sum_xpr->val() == 2);
    assert(sub_xpr->val() == utils::rational::zero);
    assert(mul_xpr->val() == utils::rational::one);
    assert(div_xpr->val() == utils::rational::one);

    auto lt_xpr = ctx.mk_lt(ctx.mk_real_const(utils::rational::one), ctx.mk_real_const(utils::rational::one));
    auto le_xpr = ctx.mk_le(ctx.mk_real_const(utils::rational::one), ctx.mk_real_const(utils::rational::one));
    auto eq_xpr = ctx.mk_eq(ctx.mk_real_const(utils::rational::one), ctx.mk_real_const(utils::rational::one));
    auto ge_xpr = ctx.mk_ge(ctx.mk_real_const(utils::rational::one), ctx.mk_real_const(utils::rational::one));
    auto gt_xpr = ctx.mk_gt(ctx.mk_real_const(utils::rational::one), ctx.mk_real_const(utils::rational::one));

    assert(lt_xpr->val() == utils::False);
    assert(le_xpr->val() == utils::True);
    assert(eq_xpr->val() == utils::True);
    assert(ge_xpr->val() == utils::True);
    assert(gt_xpr->val() == utils::False);
}

int main()
{
    test_bools();
    test_ints();
    test_reals();

    return 0;
}
