#include "sat_core.h"
#include "ov_theory.h"
#include <cassert>

using namespace semitone;

class test_val : public utils::enum_val
{
};

void test_ov_0()
{
    auto core = sat_ptr(new sat_core());
    ov_theory ov(core);

    test_val a;
    test_val b;
    test_val c;

    var v0 = ov.new_var({&a, &b, &c});
    var v1 = ov.new_var({&a, &b});

    // v0 == v1
    bool nc = core->new_clause({ov.new_eq(v0, v1)});
    assert(nc);

    bool prop = core->propagate();
    assert(prop);

    assert(core->value(ov.allows(v0, a)) == utils::Undefined);
    assert(core->value(ov.allows(v0, b)) == utils::Undefined);
    assert(core->value(ov.allows(v0, c)) == utils::False);

    bool assm = core->assume(ov.allows(v0, a));
    assert(assm);
    assert(core->value(ov.allows(v0, a)) == utils::True);
    assert(core->value(ov.allows(v0, b)) == utils::False);
    assert(core->value(ov.allows(v0, c)) == utils::False);
    assert(core->value(ov.allows(v1, a)) == utils::True);
    assert(core->value(ov.allows(v1, b)) == utils::False);
}

void test_ov_1()
{
    auto core = sat_ptr(new sat_core());
    ov_theory ov(core);

    test_val a;
    test_val b;
    test_val c;

    var v0 = ov.new_var({&a, &b, &c});
    var v1 = ov.new_var({&a, &b});

    // v0 == v1
    bool nc = core->new_clause({ov.new_eq(v0, v1)});
    assert(nc);

    bool prop = core->propagate();
    assert(prop);

    assert(core->value(ov.allows(v0, a)) == utils::Undefined);
    assert(core->value(ov.allows(v0, b)) == utils::Undefined);
    assert(core->value(ov.allows(v0, c)) == utils::False);

    bool assm = core->assume(!ov.allows(v0, a));
    assert(assm);
    assert(core->value(ov.allows(v0, a)) == utils::False);
    assert(core->value(ov.allows(v0, b)) == utils::True);
    assert(core->value(ov.allows(v0, c)) == utils::False);
    assert(core->value(ov.allows(v1, a)) == utils::False);
    assert(core->value(ov.allows(v1, b)) == utils::True);
}

void test_ov_2()
{
    auto core = sat_ptr(new sat_core());
    ov_theory ov(core);

    test_val a;
    test_val b;
    test_val c;

    var v0 = ov.new_var({&a, &b, &c});
    var v1 = ov.new_var({&a, &b});

    // v0 == v1
    lit eq0 = ov.new_eq(v0, v1);

    bool prop = core->propagate();
    assert(prop);

    // enforcing the [v0 != v1] constraint has no effect on the allowed values of the variables..
    bool assm = core->assume(!eq0);
    assert(assm);
    assert(core->value(eq0) == utils::False);
    assert(core->value(ov.allows(v0, a)) == utils::Undefined);
    assert(core->value(ov.allows(v0, b)) == utils::Undefined);
    assert(core->value(ov.allows(v0, c)) == utils::Undefined);
    assert(core->value(ov.allows(v1, a)) == utils::Undefined);
    assert(core->value(ov.allows(v1, b)) == utils::Undefined);

    // a new [v0 == v1] constraint is controlled by the same literal..
    lit eq1 = ov.new_eq(v0, v1);
    assert(eq0 == eq1);

    // removing the `a` value from the `v1` variable, however, results in the removal of the `b` value from the `v0` variable..
    assm = core->assume(!ov.allows(v1, a));
    assert(assm);
    assert(core->value(ov.allows(v0, a)) == utils::Undefined);
    assert(core->value(ov.allows(v0, b)) == utils::False);
    assert(core->value(ov.allows(v0, c)) == utils::Undefined);
    assert(core->value(ov.allows(v1, a)) == utils::False);
    assert(core->value(ov.allows(v1, b)) == utils::True);

    // a new [v0 == v1] constraint is still controlled by the same literal..
    lit eq2 = ov.new_eq(v0, v1);
    assert(eq0 == eq2);
}

int main(int, char **)
{
    test_ov_0();
    test_ov_1();
    test_ov_2();
}