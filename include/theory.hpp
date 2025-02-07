#pragma once

#include "lit.hpp"
#include <vector>

namespace semitone
{
  class network;

  class theory
  {
    friend class network;

  public:
    theory(network &net) noexcept;
    virtual ~theory() noexcept = default;

  protected:
    /**
     * @brief Binds a variable to the theory.
     *
     * The theory will be notified when the variable changes.
     *
     * @param v The variable to bind.
     */
    void bind(const utils::var &v) noexcept;

  private:
    /**
     * @brief Asks the theory to perform propagation after the given literal has been assigned.
     * Returns true if the propagation succeeds or false if an inconsistency is found.
     * In case of inconsistency, the `confl` vector must be filled with the conflicting constraint.
     *
     * @param p the literal that has been assigned.
     * @return true if propagation succeeds or false if an inconsistency is found.
     */
    [[nodiscard]] virtual bool propagate(const utils::lit &p) noexcept = 0;

    /**
     * @brief Checks whether the theory is consistent with the given propositional assignments.
     * Returns true if the theory is consistent or false if an inconsistency is found.
     * In case of inconsistency, the `confl` vector must be filled with the conflicting constraint.
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

  private:
    /**
     * @brief Analyzes the current conflict and backjumps to the proper decision level.
     */
    void analyze_and_backjump() noexcept;

  protected:
    network &net;
    std::vector<utils::lit> cnfl;
  };
} // namespace semitone
