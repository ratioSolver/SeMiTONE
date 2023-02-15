# SeMiTONE

![Build Status](https://github.com/ratioSolver/SeMiTONE/actions/workflows/cmake.yml/badge.svg)

Satisfiability Modulo Theories (SMT) concerns the satisfiability of formulas with respect to some background theory.
SeMiTONE is a Satisfiability Modulo TheOries NEtwork, allowing the creation of variables and constraints in different underlying theories.

SeMiTONE maintains backtrackable data structures, allows the creation of variables and constraints, performs constraint propagation and, whenever conflicts arise, performs conflict analysis, learns a no-good and backjumps to the highest level. It is worth noting that SeMiTONE is not an SMT solver. SeMiTONE is, on the contrary, a network on top of which SMT solvers can be built. In this regard, SeMiTONE deliberately neglects all the aspects related to 'search' as, for example, search algorithms and resolution heuristics, demanding to external modules solving SMT problems.

## Usage

At the core of SeMiTONE there is the `sat_core` module which allows the creation of propositional variables and constraints. Propositional variables are identified through integers. The clause creation procedure introduces a new clause, represented by an array of (direct or negated) literals, into the network, returning `false` if some trivial inconsistency is recognized. It is worth noting that in case the clause creation procedure returns `true` there is no guarantee that the network is still consistent since identifying inconsistencies might occur only after a search process.

```cpp
sat_core sat;

// we create two propositional variables
var b0 = sat.new_var();
var b1 = sat.new_var();

// we create a propositional constraint (i.e. the (¬b0 ∨ b1) clause)
bool nc = sat.new_clause({lit(b0, false), b1});

// the current value of `b0` (i.e. Undefined)
lbool b0_val = sat.value(b0);
```

Once propositional variables and constraints are created, it is possible to assume values for the variables and verify the consequences through propagation. Assuming a value for a propositional variable stores the context of the network allowing subsequent backtracking (i.e. restoring the context prior of the assignment). While the variable assumption procedure returns `false` if some trivial inconsistency is introduced by the assumption, the propagation procedure might recognize an inconsistency, generate a no-good and backtrack at the highest possible level, returning `false` only in case the network becomes definitely inconsistent, independently from possible subsequent assignments.

```cpp
// we store the context and assume b0
bool assm = core.assume(lit(b0));

// the current value of `b0` is now True as a consequence of the assignment
b0_val = sat.value(b0);
// the current value of `b1` is now True as a consequence of the propagation
lbool b1_val = sat.value(b1);
```

Finally, it is possible to restore the context prior of the assignment through the `pop()` procedure.

```cpp
sat.pop();

// the current values of the `b0` and `b1` variables is now back to Undefined
b0_val = sat.value(b0);
b1_val = sat.value(b1);
```

## Theories

SeMiTONE allows the creation of variables and constraints in different underlying theories. Although new theories can be easily integrated, SeMiTONE currently manages a a linear real arithmetic theory and an object variable theory and an integer and real difference logic theory.

### Linear Real Arithmetic Theory

The Linear Real Arithmetic ([LRA](https://smtlib.cs.uiowa.edu/logics-all.shtml#LRA)) theory allows the creation of real variables and constraints in the form of linear inequalities and equalities. The theory is implemented in the `lra_theory` module.

```cpp
auto core = sat_ptr(new sat_core());
lra_theory lra(core);

// we create two real variables
var x = lra.new_var();
var y = lra.new_var();

// we create a real constraint (i.e. the (x + y >= 1) constraint)
var x_plus_y_leq_1 = lra.new_geq(lin(x, rational::ONE) + lin(y, rational::ONE), lin(rational::ONE));

// we assume the constraint
bool assm = core->assume(x_plus_y_leq_1);

// the current value of `x`
inf_rational x_val = lra.value(x);
// the current value of `y`
inf_rational y_val = lra.value(y);
```

### Object Variable Theory

The Object Variable theory allows the creation of object variables and constraints in the form of object equalities and disequalities. The theory is implemented in the `ov_theory` module. In order to create object variables and constraints, it is necessary to provide a `var_value` implementation in order to identify the allowed values for the object variables.

```cpp
class test_val : public var_value
{
};
```

Once the `var_value` implementation is provided, it is possible to create object variables and constraints.

```cpp
auto core = sat_ptr(new sat_core());
ov_theory ov(core);

// we create some possible allowed values for the object variables
test_val a;
test_val b;
test_val c;

// we create two object variables
var x = ov.new_var({&a, &b});
var y = ov.new_var({&a, &b, &c});

// we create an object constraint (i.e. the (x = y) constraint)
var x_eq_y = ov.new_eq(x, y);

// we assume the constraint
bool assm = core->assume(x_eq_y);

// the current allowed values for `x`
std::unordered_set<var_value *> x_val = ov.value(x);
// the current allowed values for `y`
std::unordered_set<var_value *> y_val = ov.value(y);
```

### Integer and Real Difference Logic Theory

The Integer and Real Difference Logic ([IDL](https://smtlib.cs.uiowa.edu/logics-all.shtml#QF_IDL) and [RDL](https://smtlib.cs.uiowa.edu/logics-all.shtml#QF_RDL) theories. The theories allow the creation of integer and real variables and constraints in the form of difference logic inequalities and equalities. The theories are implemented in the `idl_theory` and `rdl_theory` modules.

```cpp
auto core = sat_ptr(new sat_core());
idl_theory idl(core);

// we create two integer variables
var x = idl.new_var();
var y = idl.new_var();

// we create an integer constraint (i.e. the (x - y >= 1) constraint)
var x_minus_y_leq_1 = idl.new_geq(lin(x, rational::ONE) - lin(y, rational::ONE), lin(rational::ONE));

// we assume the constraint
bool assm = core->assume(x_minus_y_leq_1);

// the current value of `x`
inf_rational x_val = idl.value(x);
// the current value of `y`
inf_rational y_val = idl.value(y);
```

The `idl_theory` and `rdl_theory` modules are very similar. The only difference is that the `idl_theory` module allows the creation of integer variables and constraints while the `rdl_theory` module allows the creation of real variables and constraints.

The Difference Logic theories allow to manage Disjunctive Temporal Networks (DTNs). The following example shows how to create a DTN with a disjunctive constraint.

```cpp
auto core = sat_ptr(new sat_core());
idl_theory idl(core);

// we create the origin and the horizon of the DTN
var origin = idl.new_var();
var horizon = idl.new_var();

// we create a new clause (i.e. the (horizon - origin >= 0) constraint)
bool nc = core->new_clause({idl.new_distance(horizon, origin, 0)});

// we create two integer variables
var x = idl.new_var();
var y = idl.new_var();

// we create a new clause (i.e. the (x - origin >= 0) constraint)
nc = core->new_clause({idl.new_distance(x, origin, 0)});
// we create a new clause (i.e. the (horizon - x >= 0) constraint)
nc = core->new_clause({idl.new_distance(horizon, x, 0)});

// we create a new clause (i.e. the (y - origin >= 0) constraint)
nc = core->new_clause({idl.new_distance(y, origin, 0)});
// we create a new clause (i.e. the (horizon - y >= 0) constraint)
nc = core->new_clause({idl.new_distance(horizon, y, 0)});

// we create two disjunctive temporal constraints
// (i.e. the (x - y >= 1) constraint)
var x_minus_y_geq_1 = idl.new_distance(x, y, 1);
// (i.e. the (x - y <= 2) constraint)
var x_minus_y_leq_2 = idl.new_distance(y, x, 2);

// we create a new clause (i.e. the (x_minus_y_geq_1 or x_minus_y_leq_2) constraint)
nc = core->new_clause({x_minus_y_geq_1, x_minus_y_leq_2});
```
