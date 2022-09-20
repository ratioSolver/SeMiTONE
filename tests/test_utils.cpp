#include "combinations.h"
#include "cartesian_product.h"
#include <cassert>

using namespace semitone;

void test_combinations()
{
    auto combs = combinations(std::vector<char>({'a', 'b', 'c', 'd'}), 3);
    assert(combs.size() == 4);
    assert(combs.at(0) == std::vector<char>({'a', 'b', 'c'}));
    assert(combs.at(1) == std::vector<char>({'a', 'b', 'd'}));
    assert(combs.at(2) == std::vector<char>({'a', 'c', 'd'}));
    assert(combs.at(3) == std::vector<char>({'b', 'c', 'd'}));
}

void test_cartesian_product()
{
    auto prod = cartesian_product(std::vector<std::vector<char>>({{'a', 'b'}, {'c', 'd'}}));
    assert(prod.size() == 4);
    assert(prod.at(0) == std::vector<char>({'a', 'c'}));
    assert(prod.at(1) == std::vector<char>({'a', 'd'}));
    assert(prod.at(2) == std::vector<char>({'b', 'c'}));
    assert(prod.at(3) == std::vector<char>({'b', 'd'}));
}

int main(int, char **)
{
    test_combinations();
    test_cartesian_product();
}