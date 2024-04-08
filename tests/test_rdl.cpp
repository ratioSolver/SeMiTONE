#include "sat_core.hpp"
#include "rdl_theory.hpp"

#include <cassert>

void test_ov()
{
    auto sat = std::make_shared<semitone::sat_core>();
    semitone::rdl_theory rdl(sat);

    auto origin = rdl.new_var();
    auto horizon = rdl.new_var();
}

int main(int argc, char const *argv[])
{
    test_ov();

    return 0;
}
