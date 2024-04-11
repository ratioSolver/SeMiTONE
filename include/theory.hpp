#pragma once

#include <memory>
#include <vector>
#include "lit.hpp"

namespace semitone
{
  class sat_core;

  class theory : public std::enable_shared_from_this<theory>
  {
    friend class sat_core;

  public:
    virtual ~theory() = default;

    [[nodiscard]] sat_core &get_sat() const noexcept { return *sat; }

    /**
     * @brief Asks the theory to perform propagation after the given literal has been assigned. Returns true if the propagation succeeds or false if an inconsistency is found. In case of inconsistency, the confl vector must be filled with the conflicting constraint.
     *
     * @param p the literal that has been assigned.
     * @return true if propagation succeeds or false if an inconsistency is found.
     */
    [[nodiscard]] virtual bool propagate(const utils::lit &p) noexcept = 0;

    /**
     * @brief Checks whether the theory is consistent with the given propositional assignments. Returns true if the theory is consistent or false if an inconsistency is found. In case of inconsistency, the confl vector must be filled with the conflicting constraint.
     *
     * @return true if the theory is consistent or false if an inconsistency is found.
     */
    [[nodiscard]] virtual bool check() noexcept = 0;

    /**
     * @brief Notifies the theory that some information for subsequent backtracking might need to be stored.
     */
    virtual void push() noexcept = 0;

    /**
     * @brief Notifies the theory that a backtracking step is required.
     */
    virtual void pop() noexcept = 0;

  protected:
    /**
     * @brief Binds the theory to a propositional variable.
     */
    void bind(VARIABLE_TYPE v) noexcept;
    /**
     * @brief Records a new clause which might be inferred by the theory.
     */
    void record(std::vector<utils::lit> &&clause) noexcept;

  private:
    /**
     * @brief Analyzes the current conflict and backjumps to the proper decision level.
     */
    void analyze_and_backjump() noexcept;

  private:
    sat_core *sat;

  protected:
    std::vector<utils::lit> cnfl; // conflict clause to be analyzed by the SAT solver after a conflict is detected by propagate or check
  };
} // namespace semitone
