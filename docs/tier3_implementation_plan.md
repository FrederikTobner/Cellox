# Tier 3 Binder Implementation Plan

## Purpose

This document describes an incremental implementation plan for Tier 3 binder features, with pseudocode for each phase.

It is aligned with:
- `docs/tier3_preparation_design.md`
- Existing Tier 1/2 binder behavior already implemented for error expressions.

## Delivery Strategy

- Implement in small phases with independent validation.
- Keep parser/lowering changes localized.
- Reuse existing binder scope model from Tier 1/2.
- Treat each phase as releasable behind an experimental feature flag first.
- Current scope lock: conditional binders plus while-loop binders in V1.
- Iterator traits and generics are explicitly deferred.

## Global Workstreams

1. Grammar updates
2. Parser updates
3. Lowering/bytecode strategy
4. Diagnostics
5. Tests
6. Docs

---

## Phase A: Binder-Aware Conditional Flow

## Goal

Add binder-capable conditional forms while preserving expression-first semantics.

## Candidate Syntax

- Statement form:
  - `if expr |x| stmt else stmt`
- Optional expression form (if approved):
  - `if expr |x| expr else expr`

## High-Level Steps

1. Extend grammar for binder-enabled `if`.
2. Parse binder header and create scoped bound local.
3. Lower branch flow so `expr` is evaluated once.
4. Ensure binder scope exists only inside selected branch.
5. Add diagnostics for malformed binder forms.

## Pseudocode

```text
function parseIfStatement():
    consume('if')
    condExpr = parseExpression()

    if nextToken == '|':
        binderName = parsePipeBinderName()
        emitEvaluateCondition(condExpr)

        jumpElse = emitJumpIfFalse()

        // truthy branch with bound value
        beginScope()
        bindLocal(binderName, valueFrom(condExpr))
        parseStatementOrBlock()
        endScope()

        jumpEnd = emitJump()

        patch(jumpElse)
        parseOptionalElseBranchWithoutBinderOrWithOwnBinder()
        patch(jumpEnd)
    else:
        lowerExistingIfPath(condExpr)
```

## Validation Gate

- Branch binder is unavailable outside its branch.
- Condition is evaluated once.
- Existing `if` syntax is unaffected.

---

## Phase B: Binder-Aware Loop Flow (V1, No Traits)

## Goal

Add immediate loop-binder value through while-form binders and iter.next()-style conditions.

Status:
- Active in current Tier 3 delivery.
- No trait/generic enforcement in V1.
- Arrays are easy to support with a helper iterator.
- Other types can participate via explicit adapter iterators.

## Candidate Syntax

- Active V1 form:
  - `while expr |x| stmt`
- Active usage pattern:
  - `while iter.next() |x| stmt`
- Deferred form:
  - `for (iterExpr) |x| stmt`

## High-Level Steps

1. Extend while parsing to accept optional binder header after condition.
2. Lower condition once per iteration check, then branch by truthiness.
3. Bind loop local only on truthy path.
4. Preserve break/continue scope and cleanup behavior.
5. Document V1 iterator model and adapter expectations.

## Pseudocode

```text
function parseWhileStatementWithBinder():
    loopStart = markLoopStart()
    consume('while')
    consume('(')
    condExpr = parseExpression()
    consume(')')

    if nextToken == '|':
        binderName = parsePipeBinderName()

        emitEvaluateCondition(condExpr)
        exitJump = emitJumpIfFalse()
        emitPopConditionFlag()

        beginScope()
        bindLocal(binderName, valueFrom(condExpr))
        parseStatementOrBlock()
        endScope()

        emitLoopBack(loopStart)
        patch(exitJump)
        emitPopConditionFlag()
    else:
        lowerExistingWhilePath(condExpr)
```

## Validation Gate

- Binder rebinding occurs once per successful iteration.
- Binder local is unavailable outside loop body scope.
- continue jumps to condition check and does not leak locals.
- break unwinds loop locals correctly.
- Existing while syntax remains unaffected.

---

## Phase C: Typed/Pattern Binder Surface (Syntax-First)

## Goal

Introduce typed/pattern binder syntax without full matching complexity in first step.

## Candidate Syntax

- `|Type.Variant as e|`
- `|Type as x|`

## High-Level Steps

1. Extend binder grammar to parse typed/pattern forms.
2. Store parsed pattern metadata in AST/compiler state.
3. Initially gate as syntax-only (or minimal semantic checks).
4. Add explicit diagnostics for unsupported pattern semantics.

## Pseudocode

```text
function parsePipeBinder():
    consume('|')

    if looksLikeTypedPattern():
        pattern = parsePatternSpec()   // Type, Variant, alias
        consume('|')
        return Binder(pattern=pattern)
    else:
        name = consumeIdentifier()
        consume('|')
        return Binder(name=name)
```

```text
function lowerBinder(binder, sourceValue):
    if binder.isSimpleName:
        bindLocal(binder.name, sourceValue)
        return

    if featureFlagTypedBindersDisabled:
        error('Typed binders not enabled')
        return

    if typedBinderSemanticsNotImplemented:
        error('Typed binder parsed but semantic matching not yet implemented')
        return

    performTypedMatchAndBind(binder.pattern, sourceValue)
```

## Validation Gate

- Parser accepts intended typed syntax.
- Clear diagnostics for unsupported semantics.
- No ambiguity with simple binders.

---

## Phase D: Full Typed/Pattern Matching Semantics

## Goal

Enable actual runtime/compiler behavior for typed and variant binder matching.

## High-Level Steps

1. Define matching truth table (success/failure cases).
2. Lower matching checks before binder assignment.
3. Ensure failure path behavior is consistent across `if`/`while`/error forms.
4. Add complete positive and negative tests.

## Pseudocode

```text
function performTypedMatchAndBind(pattern, value):
    if matches(pattern, value):
        bindLocal(pattern.alias, extract(pattern, value))
        return MATCH_OK
    else:
        return MATCH_FAIL
```

```text
function lowerIfWithTypedBinder(condExpr, binderPattern):
    val = evalOnce(condExpr)
    if performTypedMatchAndBind(binderPattern, val) == MATCH_OK:
        executeThenBranch()
    else:
        executeElseBranch()
```

## Validation Gate

- Match success/failure paths are deterministic.
- Alias binding is correct.
- Non-matching values do not leak stale locals.

---

## Testing Plan by Phase

## Phase A Tests

- `if expr |x|` truthy branch binding
- else path behavior
- nested binder scopes
- regression tests for existing `if`

## Phase B Tests

- `while expr |x|` with truthy values binds x correctly.
- `while iter.next() |x|` over array iterator helper produces expected sequence.
- continue/break semantics with binder locals.
- regression tests for existing `while` without binder.
- adapter iterator smoke test for a non-array type.

## Phase C Tests

- parse acceptance for typed binder forms
- unambiguous parse against simple binders
- expected diagnostics when semantics disabled

## Phase D Tests

- typed match success
- typed match failure
- variant-specific matching
- mixed nested binders and error binders

---

## Rollback and Safety Checkpoints

1. Keep each phase behind feature toggle until tests pass.
2. Merge grammar+parser changes before semantic activation when possible.
3. Ship only one new binder form at a time.
4. Keep a rollback path by isolating phase commits.

---

## Milestone Checklist

- [ ] Phase A grammar approved
- [ ] Phase A parser/lowering complete
- [ ] Phase A tests green
- [ ] Phase B grammar approved
- [ ] Phase B parser/lowering complete
- [ ] Phase B tests green
- [ ] Deferred roadmap item created for iterator traits and generics
- [ ] Phase C syntax parse complete
- [ ] Phase C diagnostics complete
- [ ] Phase C tests green
- [ ] Phase D matching semantics complete
- [ ] Phase D tests green
- [ ] Final docs refresh for all binder forms
