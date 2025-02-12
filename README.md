# SeMiTONE

![Build Status](https://github.com/ratioSolver/SeMiTONE/actions/workflows/cmake.yml/badge.svg)
[![codecov](https://codecov.io/gh/ratioSolver/SeMiTONE/branch/master/graph/badge.svg)](https://codecov.io/gh/ratioSolver/SeMiTONE)

Satisfiability Modulo Theories (SMT) concerns the satisfiability of formulas with respect to some background theory.
SeMiTONE is a Satisfiability Modulo TheOries NEtwork, allowing the creation of variables and constraints in different underlying theories.

SeMiTONE maintains backtrackable data structures, allows the creation of variables and constraints, performs constraint propagation and, whenever conflicts arise, performs conflict analysis, learns a no-good and backjumps to the highest level. It is worth noting that SeMiTONE is not an SMT solver. SeMiTONE is, on the contrary, a network on top of which SMT solvers can be built. In this regard, SeMiTONE deliberately neglects all the aspects related to 'search' as, for example, search algorithms and resolution heuristics, demanding to external modules solving SMT problems.

## Usage

At the core of SeMiTONE there is the `network` module which allows the creation of propositional variables and constraints. Variables are identified through integers. The clause creation procedure introduces a new clause, represented by an array of (direct or negated) literals, into the network, throwing `semitone::unsolvable_exception` in case the clause is trivially unsatisfiable. It is worth noting that in case the clause creation procedure succeeds, there is no guarantee that the network is still consistent, since identifying inconsistencies might occur only after a search process.

```cpp
semitone::network net;

// we create two propositional variables
auto b0 = net.new_var();
auto b1 = net.new_var();

// we create a propositional constraint (i.e. the (¬b0 ∨ b1) clause)
net.new_clause({lit(b0, false), b1});

// the current value of `b0` (i.e. utils::Undefined)
utils::lbool b0_val = net.value(b0);
```

Once propositional variables and constraints are created, it is possible to assume values for the variables and verify the consequences through propagation. Assuming a value for a propositional variable stores the context of the network allowing subsequent backtracking (i.e. restoring the context prior of the assignment). In case the assignment or the propagation procedure introduces an inconsistency, the network generates a no-good and backtracks at the highest possible level, throwing `semitone::unsolvable_exception` in case the network becomes definitely inconsistent.

```cpp
// we store the context and assume b0
net.assume(lit(b0));

// the current value of `b0` is now True as a consequence of the assignment
b0_val = net.value(b0);
// the current value of `b1` is now True as a consequence of the propagation
utils::lbool b1_val = net.value(b1);
```

Finally, it is possible to restore the context prior of the assignment through the `pop()` procedure.

```cpp
net.pop();

// the current values of the `b0` and `b1` variables is now back to Undefined
b0_val = net.value(b0);
b1_val = net.value(b1);
```

## Theories

SeMiTONE allows the creation of variables and constraints in different underlying theories. Although new theories can be easily integrated, SeMiTONE currently manages a linear arithmetic theory a difference logic theory.

### Linear Arithmetic Theory

The Linear Arithmetic ([LIA](https://smtlib.cs.uiowa.edu/logics-all.shtml#LIA) and [LRA](https://smtlib.cs.uiowa.edu/logics-all.shtml#LRA)) theory allows the creation of numeric variables and constraints in the form of linear inequalities and equalities (e.g. `2x + 3y + 4z >= 5`).

```cpp
// we create two real variables
auto x = net.new_real();
auto y = net.new_real();

// we create a new propositional variable
auto b = net.new_var();
// we create a real constraint (i.e. the (x + y >= 1) constraint)
net.new_ge(utils::lin(x, utils::rational::one) + utils::lin(y, utils::rational::one), utils::lin(utils::rational::one), utils::lit(b));

// we assume the constraint
net.assume(x_plus_y_leq_1);

// the current value of `x`
auto x_val = net.arith_value(x);
// the current value of `y`
auto y_val = net.arith_value(y);
```

### Difference Logic Theory

The Difference Logic ([IDL](https://smtlib.cs.uiowa.edu/logics-all.shtml#QF_IDL) and [RDL](https://smtlib.cs.uiowa.edu/logics-all.shtml#QF_RDL)) theory allows the creation of variables and constraints in the form of difference logic inequalities (e.g. `x - y >= 1`).

```cpp
// we create two variables
auto x = net.new_tp();
auto y = net.new_tp();

// we create a new propositional variable
auto b = net.new_var();
// we create an integer constraint (i.e. the (x - y >= 1) constraint)
net.new_distance(x, y, utils::rational::one, utils::lit(b));

// we assume the constraint
net.assume(x_minus_y_leq_1);

// the current bounds of `x`
auto x_val = net.tp_bounds(x);
// the current bounds of `y`
auto y_val = net.tp_bounds(y);
```

The Difference Logic theory allows to manage Disjunctive Temporal Networks (DTNs). The following example shows how to create a DTN with a disjunctive constraint.

```cpp
// we create the horizon of the DTN
auto horizon = net.new_tp();

// we create two variables
auto x = net.new_tp();
auto y = net.new_tp();

// we constrain the variables to be greater or equal to the origin and less or equal to the horizon
net.new_distance(x, 0, utils::rational::zero);
net.new_distance(horizon, x, utils::rational::zero);
net.new_distance(y, 0, utils::rational::zero);
net.new_distance(horizon, y, utils::rational::zero);

// we create a disjunctive temporal constraints (i.e. the (x - y >= 1 or y - x >= 1) constraint)
auto b0 = net.new_var();
auto b1 = net.new_var();

net.new_distance(x, y, utils::rational::one, utils::lit(b0));
net.new_distance(y, x, utils::rational::one, utils::lit(b1));

// we create a new clause (i.e. the (b0 or b1) clause)
net.new_clause({lit(b0), lit(b1)});
```
