#pragma once

#include "theory.hpp"
#include "inf_rational.hpp"
#include "memory.hpp"
#include <map>
#include <optional>
#include <unordered_map>

namespace semitone
{
  class distance_constraint;

  class dl_theory : public theory
  {
  public:
    dl_theory(network &net, const size_t &size = 16) noexcept;

    [[nodiscard]] utils::var new_var() noexcept;

  private:
    [[nodiscard]] bool propagate(const utils::lit &p) noexcept override;
    [[nodiscard]] bool check() noexcept override;
    void push() noexcept override;
    void pop() noexcept override;

  private:
    /**
     * @brief Resize the distance and predecessor matrices.
     *
     * @param size the new size of the matrices.
     */
    void resize(const size_t &size) noexcept;

    struct layer
    {
      std::map<std::pair<utils::var, utils::var>, utils::inf_rational> old_dists;                                      // the updated distances..
      std::map<std::pair<utils::var, utils::var>, utils::var> old_preds;                                               // the updated predecessors..
      std::map<std::pair<utils::var, utils::var>, std::optional<utils::ref_wrapper<distance_constraint>>> old_constrs; // the updated constraints..
    };

  private:
    size_t n_vars = 1;                                                                                              // the number of variables..
    std::vector<std::vector<utils::inf_rational>> dists;                                                            // the distance matrix..
    std::vector<std::vector<utils::var>> preds;                                                                     // the predecessor matrix..
    std::unordered_map<utils::var, std::vector<utils::u_ptr<distance_constraint>>> var_dists;                       // the constraints controlled by a propositional variable (for propagation purposes)..
    std::map<std::pair<utils::var, utils::var>, std::vector<utils::ref_wrapper<distance_constraint>>> dist_constrs; // the constraints between two temporal points (for propagation purposes)..
    std::map<std::pair<utils::var, utils::var>, utils::ref_wrapper<distance_constraint>> dist_constr;               // the currently enforced constraints..
    std::vector<layer> layers;                                                                                      // we store the updates..
  };

  class distance_constraint
  {
  public:
    distance_constraint(const utils::lit &b, utils::var from, utils::var to, utils::inf_rational &&dist) noexcept : b(b), from(from), to(to), dist(dist) {}

    [[nodiscard]] const utils::lit &get_lit() const noexcept { return b; }
    [[nodiscard]] utils::var get_from() const noexcept { return from; }
    [[nodiscard]] utils::var get_to() const noexcept { return to; }
    [[nodiscard]] const utils::inf_rational &get_dist() const noexcept { return dist; }

  private:
    utils::lit b;
    utils::var from;
    utils::var to;
    utils::inf_rational dist;
  };
} // namespace semitone
