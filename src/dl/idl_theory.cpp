#include "idl_theory.hpp"
#include "integer.hpp"

namespace semitone
{
    idl_theory::idl_theory(std::shared_ptr<sat_core> sat, const size_t &size) : theory(sat), dists(size, std::vector<INTEGER_TYPE>(size, utils::inf())), preds(size, std::vector<VARIABLE_TYPE>(size, std::numeric_limits<INTEGER_TYPE>::max()))
    {
        for (size_t i = 0; i < size; ++i)
        {
            dists[i][i] = 0;
            preds[i][i] = i;
        }
    }

    VARIABLE_TYPE idl_theory::new_var() noexcept
    {
        auto var = n_vars++;
        if (var >= dists.size())
            resize((dists.size() * 3) / 2 + 1);
        return var;
    }

    void idl_theory::resize(const size_t &size) noexcept
    {
        const size_t c_size = dists.size();
        for (size_t i = 0; i < c_size; ++i)
        {
            dists[i].resize(size, utils::inf());
            preds[i].resize(size, std::numeric_limits<INTEGER_TYPE>::max());
        }
        dists.resize(size, std::vector<INTEGER_TYPE>(size, utils::inf()));
        preds.resize(size, std::vector<VARIABLE_TYPE>(size, std::numeric_limits<INTEGER_TYPE>::max()));
        for (size_t i = c_size; i < size; ++i)
        {
            dists[i][i] = 0;
            preds[i][i] = i;
        }
    }
} // namespace semitone