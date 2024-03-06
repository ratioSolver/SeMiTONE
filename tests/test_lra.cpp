#include "sat_core.hpp"
#include "lra_theory.hpp"

#include <cassert>

void test_lra()
{
    auto sat = std::make_shared<semitone::sat_core>();
    semitone::lra_theory lra(sat);

    auto origin = lra.new_var();
    auto horizon = lra.new_var();
}

int main(int argc, char const *argv[])
{
    test_lra();

    return 0;
}
