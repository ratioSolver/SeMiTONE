#pragma once

#include <memory>
#include "lit.hpp"

namespace semitone
{
  class sat_core;

  class theory
  {
  public:
    theory(std::shared_ptr<sat_core> sat) : sat(sat) {}
    virtual ~theory() = default;

    sat_core &get_sat() const { return *sat; }

    /**
     * @brief Asks the theory to perform propagation after the given literal has been assigned. Returns true if the propagation succeeds or false if an inconsistency is found. In case of inconsistency, the confl vector must be filled with the conflicting constraint.
     *
     * @param p the literal that has been assigned.
     * @return true if propagation succeeds or false if an inconsistency is found.
     */
    virtual bool propagate(const lit &p) = 0;

    /**
     * @brief Checks whether the theory is consistent with the given propositional assignments. Returns true if the theory is consistent or false if an inconsistency is found. In case of inconsistency, the confl vector must be filled with the conflicting constraint.
     *
     * @return true if the theory is consistent or false if an inconsistency is found.
     */
    virtual bool check() = 0;

    /**
     * @brief Notifies the theory that some information for subsequent backtracking might need to be stored.
     */
    virtual void push() = 0;

    /**
     * @brief Notifies the theory that a backtracking step is required.
     */
    virtual void pop() = 0;

  protected:
    std::shared_ptr<sat_core> sat;
  };
} // namespace semitone
