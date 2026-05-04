# Tier 3 Preparation Design

## Purpose

This document defines the high-level design and rollout strategy to prepare Tier 3 language features with low risk while preserving an expression-first direction.

It is intentionally non-technical and excludes implementation details.

## Current Context

- Tier 1 is in place: expression-oriented error handling with bound catch variables.
- Tier 2 is in place: richer error expression forms, including optional success binders and handler blocks.
- The language is still in beta, so backward compatibility pressure is lower, but stable semantics remain important.
- Iterator traits and generics are higher risk and are deferred for later iterations.

## Design Goals

- Extend binder-based syntax to more constructs beyond error handling.
- Keep language design expression-first to maximize composability.
- Introduce new forms in small, verifiable steps.
- Maintain clear and predictable scoping rules.
- Minimize behavioral surprises as syntax expands.

## Guiding Principles

1. Expression-first over statement-first.
2. One binder model reused consistently across features.
3. Add syntax only after semantics are defined.
4. Prefer incremental rollout over large feature drops.
5. Preserve readability and avoid ambiguous grammar.

## Scope for Tier 3 Preparation

### In Scope

- Defining semantics for binder usage in non-error control flow.
- Deciding feature rollout order for lowest risk and highest user value.
- Establishing acceptance criteria and validation strategy.
- Recording open design decisions and unresolved questions.

### Out of Scope

- Parser/runtime implementation plans.
- Bytecode/VM changes.
- Internal architecture details.

## Proposed Feature Direction

### Phase A: Binder-Aware Conditional Flow

- Add binder-capable conditional forms so values can be bound directly in branch context.
- Focus on clarity and local scope predictability.

### Phase B: Binder-Aware Loop Flow (V1)

- Add binder-capable loop flow for while-style iteration.
- Support iterator-style usage through condition expressions such as iter.next().
- Keep V1 trait-free: no global iterator protocol enforcement yet.
- Accept that non-array types need custom iterator adapters in V1.

### Phase C: Typed/Pattern Binder Surface

- Introduce typed or patterned binder syntax at the language level.
- Defer deeper matching complexity until semantics are fully stabilized.

## Recommended Rollout Order

1. Binder-enabled conditional form.
2. Binder-enabled while-loop form.
3. Iterator helper path for arrays and adapter-based support for other types.
4. Typed/pattern binder syntax declaration.
5. Full typed/pattern matching behavior.

This order prioritizes immediate user value while keeping semantic complexity manageable.

## Semantic Decisions to Lock Before Rollout

- Binder lifetime and scope boundaries.
- Evaluation timing and single-evaluation guarantees.
- Truthiness/match behavior for binder-enabled conditionals.
- Per-iteration rebinding and scope cleanup behavior for loop binders.
- V1 iterator contract without traits (how iter.next() signals completion).
- Rules for array iterator helpers and adapter-based custom iterators.
- Interaction with existing error expression forms.
- Rules for nested and chained binders.

## Risk Management Approach

- Keep each phase independently testable and reviewable.
- Validate each new syntax form against an equivalent baseline behavior.
- Maintain explicit diagnostic rules for ambiguous or invalid binder usage.
- Avoid introducing multiple new constructs in the same release step.
- Keep iterator traits/generics out of V1 and revisit only after loop-binder semantics settle.

## Success Criteria

Tier 3 preparation is considered complete when:

- Semantics are documented and internally consistent.
- Rollout order is agreed and tracked.
- Validation criteria are defined for each planned phase.
- Open questions are reduced to a small, explicit list.

## Open Questions

- Should conditional binders require explicit matching semantics or use existing truthiness?
- Should loop binders initially support only while-form, with for(iter) binder syntax deferred?
- Which custom-iterator adapter pattern should be recommended before traits exist?
- Should typed binders ship as syntax-first or with full matching semantics immediately?
- What level of syntax overlap with future pattern matching is acceptable?

## Tracking Checklist

- [ ] Finalize binder semantics for conditional forms.
- [ ] Finalize binder semantics for loop forms.
- [ ] Finalize V1 iterator model without traits.
- [ ] Finalize typed/pattern binder language surface.
- [ ] Approve phased rollout order.
- [ ] Approve validation and acceptance criteria.
- [ ] Resolve open questions.
- [ ] Record deferred roadmap item for iterator traits/generics.
