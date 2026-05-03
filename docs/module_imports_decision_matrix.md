# Cellox Modules and Imports Decision Matrix

<!-- markdownlint-disable MD024 -->

## Document Status

- Status: Draft for review
- Related specification: module_imports_design_spec.md
- Purpose: Compare major product choices and capture clear decision criteria before implementation begins.

## How to Use This Document

- Review each decision area independently.
- Use the decision criteria table as the baseline for acceptance.
- Select one option per area.
- Record final choices in the Decision Log section.

## Decision Criteria

| Criterion | Meaning |
| --- | --- |
| Learnability | How easy the model is for new users to understand. |
| Predictability | Whether behavior is intuitive and stable across projects. |
| Safety | Likelihood of preventing accidental errors and hidden coupling. |
| Compatibility | Risk of breaking current and future language evolution. |
| Tooling friendliness | Ease of supporting linting, docs, and IDE features. |
| Future extensibility | Ability to expand into package-level workflows later. |

Scoring scale used below:

- 1 = weak fit
- 2 = limited fit
- 3 = acceptable fit
- 4 = strong fit
- 5 = best fit

## Decision Area A: Export Policy

### Options

- A1: Explicit exports only
- A2: Export-all by default
- A3: Hybrid (explicit exports recommended, export-all allowed via opt-in file mode)

### Tradeoff Matrix

| Option | Learnability | Predictability | Safety | Compatibility | Tooling friendliness | Future extensibility | Summary |
| --- | --- | --- | --- | --- | --- | --- | --- |
| A1 | 4 | 5 | 5 | 5 | 5 | 5 | Most controlled and least surprising API surface. |
| A2 | 5 | 2 | 2 | 2 | 2 | 2 | Easy to start, but high accidental API leakage risk. |
| A3 | 3 | 3 | 3 | 3 | 3 | 4 | Flexible, but policy complexity appears early. |

### Recommendation

- Recommended default for first release: A1 (explicit exports only).

### Rationale

- Minimizes accidental public surface.
- Produces clearer diagnostics and documentation boundaries.
- Supports future packaging and versioning behavior without semantic breakage.

## Decision Area B: Import Style and Name Exposure

### Options

- B1: Named imports plus namespace imports
- B2: Namespace imports only
- B3: Named imports only

### Tradeoff Matrix

| Option | Learnability | Predictability | Safety | Compatibility | Tooling friendliness | Future extensibility | Summary |
| --- | --- | --- | --- | --- | --- | --- | --- |
| B1 | 4 | 5 | 5 | 5 | 5 | 5 | Best balance of explicitness and ergonomics. |
| B2 | 3 | 5 | 5 | 4 | 4 | 4 | Very safe, but verbose for simple cases. |
| B3 | 4 | 4 | 4 | 4 | 4 | 4 | Cleaner syntax, but less ergonomic for wide APIs. |

### Recommendation

- Recommended default for first release: B1 (named + namespace imports).

### Rationale

- Offers strong readability while preserving scalable usage patterns.
- Helps avoid global namespace pollution.
- Keeps migration path open for later aliasing policies.

## Decision Area C: Cycle Policy

### Options

- C1: Reject all cycles in first release
- C2: Allow cycles with restricted semantics
- C3: Allow all cycles with runtime partial-state exposure

### Tradeoff Matrix

| Option | Learnability | Predictability | Safety | Compatibility | Tooling friendliness | Future extensibility | Summary |
| --- | --- | --- | --- | --- | --- | --- | --- |
| C1 | 5 | 5 | 5 | 5 | 5 | 4 | Clear behavior and strong diagnostics; easiest to reason about. |
| C2 | 2 | 2 | 3 | 3 | 2 | 4 | Hard to explain; users face nuanced edge cases. |
| C3 | 2 | 1 | 1 | 2 | 1 | 3 | Maximum ambiguity and highest bug risk for users. |

### Recommendation

- Recommended default for first release: C1 (reject cycles with explicit graph diagnostics).

### Rationale

- Reduces semantic complexity during initial adoption.
- Keeps user mental model consistent.
- Allows future relaxation with deliberate compatibility policy if needed.

## Decision Area D: Module Resolution Scope

### Options

- D1: Relative paths only
- D2: Relative paths plus configurable roots
- D3: Relative paths plus global search paths

### Tradeoff Matrix

| Option | Learnability | Predictability | Safety | Compatibility | Tooling friendliness | Future extensibility | Summary |
| --- | --- | --- | --- | --- | --- | --- | --- |
| D1 | 5 | 5 | 5 | 5 | 5 | 4 | Most deterministic baseline for first release. |
| D2 | 3 | 3 | 4 | 4 | 3 | 5 | Useful for larger projects, but introduces policy complexity. |
| D3 | 2 | 2 | 2 | 3 | 2 | 4 | Environment-dependent behavior is harder to debug. |

### Recommendation

- Recommended default for first release: D1 (relative paths only).

### Rationale

- Avoids host-environment ambiguity.
- Easiest behavior to document and test consistently across operating systems.
- Maintains straightforward migration path toward D2 in later phases.

## Decision Area E: Default Export Support in First Release

### Options

- E1: No default export support in first release
- E2: Support default export in first release

### Tradeoff Matrix

| Option | Learnability | Predictability | Safety | Compatibility | Tooling friendliness | Future extensibility | Summary |
| --- | --- | --- | --- | --- | --- | --- | --- |
| E1 | 4 | 5 | 5 | 5 | 5 | 5 | Keeps model minimal and avoids dual export semantics early. |
| E2 | 3 | 3 | 3 | 3 | 3 | 4 | Familiar to some users, but adds immediate complexity. |

### Recommendation

- Recommended default for first release: E1 (defer default export support).

### Rationale

- Keeps public API shape explicit and uniform.
- Reduces complexity in docs and diagnostics during initial rollout.

## Cross-Cutting Risk Register

| Risk | Impact | Probability | Mitigation (policy-level) |
| --- | --- | --- | --- |
| Ambiguous name visibility | High | Medium | Prefer explicit exports and explicit imports. |
| Import-order confusion | Medium | Medium | Enforce single-execution semantics and deterministic resolution. |
| Platform path inconsistency | High | Medium | Define canonical identity and strict relative resolution. |
| User confusion around cycles | High | Medium | Reject cycles with explicit path chain diagnostics. |
| Future package migration friction | Medium | Low | Keep first release constraints simple and forward-compatible. |

## Proposed First-Release Policy Bundle

For a coherent first release, this bundle is recommended:

- A1: Explicit exports only
- B1: Named imports plus namespace imports
- C1: Reject cycles
- D1: Relative paths only
- E1: No default export support in first release

This bundle maximizes predictability and safety while preserving a clear path to future extensions.

## Decision Log (To Be Filled During Review)

| Area | Selected Option | Decision Date | Decision Owner | Notes |
| --- | --- | --- | --- | --- |
| A: Export policy | A1 | 2026-05-03 | Maintainers | Accepted in planning session |
| B: Import style | B1 | 2026-05-03 | Maintainers | Accepted in planning session |
| C: Cycle policy | C1 | 2026-05-03 | Maintainers | Accepted in planning session |
| D: Resolution scope | D1 | 2026-05-03 | Maintainers | Accepted in planning session |
| E: Default export | E1 | 2026-05-03 | Maintainers | Accepted in planning session |

## Review Exit Criteria

The review is complete when:

- One option is chosen for each decision area.
- Any deviation from the proposed bundle is documented with rationale.
- Open questions in module_imports_design_spec.md are updated to reflect final decisions.
- The language documentation plan is updated to match selected policies.
