# Cellox Short-Term Priorities (High Value, Low Complexity)

This roadmap is optimized for limited implementation time. Focus is on improvements that deliver clear user-visible value and reduce maintenance risk without major architecture changes.

## Prioritization Method

- Value: Runtime benefit, developer productivity, language usability, and stability.
- Complexity: Estimated implementation effort in this codebase.
- Priority rule: Highest value with lowest complexity first.

## Top Recommendations

| Priority | Item | Why it is valuable | Complexity | Estimated Effort |
|---|---|---|---|---|
| 1 | Bytecode constant/index robustness and regression hardening | Prevents crashes/corruption in serialized programs and protects future features | Low | 0.5-1 day |
| 2 | Add import modules (minimal static file-based imports) | Biggest usability gain for real projects; enables code organization and reuse | Medium | 2-4 days |
| 3 | Add small peephole optimization pass in compiler backend | Quick runtime improvements on common patterns without VM redesign | Low-Medium | 1-2 days |
| 4 | Add two practical language features: `break` and `continue` | High user value for loops; easy to understand and widely expected | Medium | 1-2 days |
| 5 | Better diagnostics: filename + line + snippet in compile/runtime errors | Big productivity gain when debugging scripts and tests | Low | 0.5-1 day |

## Detailed Suggestions

## 1) Bytecode Serialization Hardening (Do First)

- Goal: Keep serialization safe and deterministic as language/features evolve.
- Scope:
- Add round-trip tests for edge values (negative doubles, large doubles, NaN/inf handling policy, nested closures with upvalues).
- Add file-format version guard and explicit compatibility error path.
- Add malformed/truncated bytecode file tests.
- Why now: Very high risk area; cheap to harden and directly impacts reliability.

## 2) Minimal Module Support (High Value)

- Recommended MVP:
- Syntax: `import "path/to/module.clx";`
- Behavior: load once (module cache), execute top-level once, expose module globals into importer namespace via prefix object or direct import policy.
- Keep it simple:
- No cyclic dependency resolution in v1 beyond basic in-progress detection error.
- No package manager or search paths initially; relative paths only.
- Why this version: Delivers real project structuring with controlled complexity.

## 3) Small Peephole Optimizations

- Add/extend low-risk optimizations where bytecode patterns are obvious:
- Constant fold chained numeric operations where safe.
- Remove redundant push/pop sequences.
- Fold unconditional jumps to next instruction.
- Keep optimizer deterministic and test-driven using bytecode golden tests.
- Why: Good performance ROI without touching parser semantics.

## 4) `break` and `continue`

- Scope:
- Support in `for`, `while`, and `do-while`.
- Compile-time error if used outside loops.
- Add nested-loop tests and interaction tests with blocks.
- Why: Users immediately benefit; implementation is mostly parser + jump patching.

## 5) Error UX Improvements

- Add:
- File path in messages.
- Line text snippet and caret indicator when available.
- Distinguish parse vs runtime formatting consistently.
- Why: Saves significant developer time and reduces friction when adopting modules.

## What to Defer (Higher Complexity)

- Full package/module system with search paths and semantic versioning.
- Advanced optimizer passes requiring control-flow graph or SSA.
- JIT or major VM architecture changes.
- Type system additions.

## Suggested 1-Week Execution Plan

- Day 1:
- Bytecode hardening tests and format compatibility checks.
- Day 2-4:
- Minimal module import MVP + cycle detection error + tests.
- Day 5:
- `break` and `continue` + tests.
- Day 6:
- Peephole optimizer extensions + benchmark spot check.
- Day 7:
- Error message improvements + docs update.

## Acceptance Criteria

- All labeled test suites pass (`unit`, `integration`, `e2e`, `fuzz`).
- New feature tests include positive, negative, and edge cases.
- No sanitizer regressions.
- New syntax/features documented in README and changelog.

## If You Only Do Two Things

1. Implement minimal modules.
2. Add `break` and `continue`.

This combination gives the highest end-user impact for the least conceptual overhead.