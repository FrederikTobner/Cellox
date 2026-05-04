# Tier 3 Conditional and Loop Binder Semantics (V1)

## Purpose

This document defines the language semantics for binder-aware conditional and loop flow in V1.

It is intended to be implementation-ready for parser and lowering work and to clarify behavior before traits/generics are introduced.

## Scope

In scope for V1:

1. Binder-aware if statement form
2. Binder-aware while statement form
3. Iterator-style loop usage through while condition expressions (for example iter.next())

Out of scope for V1:

1. Trait-based iterator conformance
2. Generic iterator abstractions
3. Dedicated for(iterable) binder syntax
4. Typed or pattern binders

## Syntax Surface (V1)

1. Conditional binder:
   - if (expression) |name| statement else statement

2. While binder:
   - while (expression) |name| statement

3. Iterator usage pattern:
   - while (iter.next()) |item| statement

Existing non-binder forms remain valid and unchanged.

## Truthiness and Branching

V1 binder control-flow uses existing language truthiness.

1. Condition expression is evaluated.
2. If the value is truthy, binder branch/body executes.
3. If the value is falsey, else branch runs for if, or loop exits for while.

No special unwrap semantics are introduced here for result values.
Use existing error forms for result-aware branching:

1. iferror for statement-level result split
2. catch and iferror expression forms for expression-level result split

## Conditional Binder Semantics

Form:

if (condExpr) |x| thenStmt else elseStmt

Evaluation and scope rules:

1. condExpr is evaluated exactly once.
2. Truthiness of condExpr determines branch.
3. Binder x is created only in the then branch when condExpr is truthy.
4. Binder x is not visible in else branch or after the if statement.
5. If nested binders reuse the same name, normal local shadowing rules apply.

Equivalent conceptual desugaring:

1. Evaluate condExpr into a temporary value t.
2. If t is falsey, jump to else branch.
3. Create scope, bind x to t, run thenStmt, end scope.
4. Jump over else branch.

## While Binder Semantics

Form:

while (condExpr) |x| bodyStmt

Evaluation and scope rules:

1. condExpr is evaluated exactly once per iteration check.
2. If condExpr is falsey, loop exits.
3. If condExpr is truthy, x is bound to that value for the current iteration only.
4. x is recreated each iteration and is not visible outside the loop body.
5. continue jumps to next condition evaluation.
6. break exits loop and unwinds current loop scope correctly.

Equivalent conceptual desugaring:

1. loop_start:
2. Evaluate condExpr into temporary t.
3. If t is falsey, jump loop_end.
4. Begin scope, bind x to t, execute bodyStmt, end scope.
5. Jump loop_start.
6. loop_end.

## Iterator Usage Semantics in V1

V1 does not require traits or generics for iterator participation.

Recommended pattern:

1. Create iterator object value iter.
2. Call iter.next() in loop condition.
3. next returns either:
   - next item value for continuation (truthy path)
   - falsey sentinel for completion (loop exit)

Example pattern:

1. it = array_iter(values)
2. while (it.next()) |item| { ... }

## Array Support and Non-Array Types

V1 support guidance:

1. Arrays should have a first-party helper iterator path.
2. Non-array types can still participate by providing explicit adapter iterators.
3. Because there is no trait system in V1, conformance is behavioral, not static.

Implication:

1. Yes, custom types may require per-type iterator adapters in V1.
2. This is an acceptable first iteration tradeoff.
3. Future traits/generics should remove that adapter friction.

## Diagnostics and Error Rules

Parser/compile-time diagnostics:

1. Missing opening or closing pipe in binder header
2. Missing binder identifier
3. Invalid binder placement for unsupported forms
4. Illegal binder variable reuse in same local scope

Runtime diagnostics for iterator usage:

1. Calling next on a value without callable next member
2. next returning unsupported shape for the chosen loop contract

Error messages should be explicit about expected V1 contract.

## Interaction With Existing Features

1. Existing if and while forms must remain behaviorally unchanged.
2. Existing iferror, catch, try, and must semantics must remain unchanged.
3. Binder-aware conditionals/loops should not alter operator precedence or expression parsing outside binder headers.

## Implementation Constraints

The following constraints are mandatory:

1. Single-evaluation guarantee for each condition check
2. Correct jump patching for then/else/loop flow
3. No binder variable leakage outside binder scope
4. Correct break/continue local unwinding
5. Compatibility with existing closure/upvalue rules

## Test Matrix (Minimum)

Conditional binder tests:

1. Truthy conditional binds value
2. Falsey conditional skips binder branch
3. Binder not visible outside branch
4. Nested binder shadowing behavior
5. Regression for plain if

While binder tests:

1. Loop binder receives per-iteration values
2. Falsey condition exits loop
3. continue path re-evaluates condition and rebinds correctly
4. break path unwinds locals correctly
5. Regression for plain while

Iterator pattern tests:

1. Array helper iterator with while (it.next()) binder loop
2. Custom adapter iterator for non-array type
3. Runtime diagnostics for missing next
4. Runtime diagnostics for invalid next contract

## Deferred Follow-Up

The following are explicitly deferred after V1 stabilization:

1. Trait-based iterator protocol
2. Generic iteration abstractions
3. for(iterable) binder syntax
4. Typed and pattern binders integrated with loop payload matching
