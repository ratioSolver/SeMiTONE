#include "sat_core.hpp"
#include "ov_theory.hpp"

#include <cassert>

class test_val : public utils::enum_val
{
};

void test_ov()
{
    auto sat = std::make_shared<semitone::sat_core>();
    auto &ov = sat->new_theory<semitone::ov_theory>();

    test_val a;
    test_val b;
    test_val c;

    std::vector<std::reference_wrapper<utils::enum_val>> v0_domain = {a, b, c};
    auto v0 = ov.new_var(std::move(v0_domain), true);

    assert(ov.domain(v0).size() == 3);
    assert(sat->value(ov.allows(v0, a)) == utils::Undefined);
    assert(sat->value(ov.allows(v0, b)) == utils::Undefined);
    assert(sat->value(ov.allows(v0, c)) == utils::Undefined);

    std::vector<std::reference_wrapper<utils::enum_val>> v1_domain = {a, b};
    auto v1 = ov.new_var(std::move(v1_domain), true);

    assert(ov.domain(v1).size() == 2);
    assert(sat->value(ov.allows(v1, a)) == utils::Undefined);
    assert(sat->value(ov.allows(v1, b)) == utils::Undefined);

    auto eq = ov.new_eq(v0, v1);
    assert(sat->value(eq) == utils::Undefined);

    bool assm = sat->assume(eq);
    assert(assm);
    assert(sat->value(eq) == utils::True);
    assert(sat->value(ov.allows(v0, a)) == utils::Undefined);
    assert(sat->value(ov.allows(v0, b)) == utils::Undefined);
    assert(sat->value(ov.allows(v0, c)) == utils::False);
    assert(ov.domain(v0).size() == 2);
    assert(sat->value(ov.allows(v1, a)) == utils::Undefined);
    assert(sat->value(ov.allows(v1, b)) == utils::Undefined);
    assert(ov.domain(v1).size() == 2);

    assm = sat->assume(ov.allows(v0, a));
    assert(assm);
    assert(sat->value(eq) == utils::True);
    assert(sat->value(ov.allows(v0, a)) == utils::True);
    assert(sat->value(ov.allows(v0, b)) == utils::False);
    assert(sat->value(ov.allows(v0, c)) == utils::False);
    assert(sat->value(ov.allows(v1, a)) == utils::True);
    assert(sat->value(ov.allows(v1, b)) == utils::False);

    assert(ov.domain(v0).size() == 1);
    assert(&ov.domain(v0).front().get() == &a);
    assert(ov.domain(v1).size() == 1);
    assert(&ov.domain(v1).front().get() == &a);
}

int main(int argc, char const *argv[])
{
    test_ov();

    return 0;
}
