# Cellox Modules and Imports Design Specification

## Document Status

- Status: Draft for review
- Target milestone: First public module support release
- Audience: Language users, maintainers, contributors, tooling authors

## Purpose

This document defines the product and language design for module and import support in Cellox. It is intentionally focused on externally visible behavior, user experience, compatibility rules, and acceptance criteria.

Companion document for tradeoff analysis: [Modules and Imports Decision Matrix](module_imports_decision_matrix.md).
Companion document for execution details: [Modules and Imports Implementation Plan](module_imports_implementation_plan.md).

This document does not define runtime internals, parser internals, bytecode details, memory layout, or any implementation strategy.

## Background and Problem Statement

Cellox currently emphasizes single-file workflows. As programs grow, users need a way to split logic into reusable units, share common functionality across files, and avoid copy and paste reuse.

Without module boundaries and imports, maintainability and testability degrade quickly. The language also lacks a clear model for how names from one file become available in another file.

## Design Goals

- Enable multi-file programs with clear boundaries.
- Keep first release simple and predictable.
- Ensure deterministic module loading behavior.
- Prevent repeated side effects from repeated imports.
- Provide clear errors for missing modules, invalid imports, and cycles.
- Preserve backward compatibility for existing single-file programs.
- Support future extension to package-level workflows without breaking early users.

## Non-Goals for First Release

- No external package registry.
- No semantic version dependency resolution.
- No conditional imports.
- No dynamic string-computed imports.
- No network-based module loading.
- No plugin architecture requirements.

## Key Terms

- Module: A source file treated as a reusable unit.
- Importer: The module that requests another module.
- Imported module: The requested module.
- Export: A name intentionally made available to other modules.
- Module namespace: The set of exported names visible to importers.
- Module graph: Directed relationships between importers and imported modules.

## User Stories

- As a user, I can split utility logic into separate files and consume it from application files.
- As a user, importing the same module from multiple places does not rerun module initialization side effects multiple times.
- As a user, I can understand where a name comes from when reading source.
- As a user, I receive actionable errors when an import cannot be resolved.
- As a user, cyclic imports fail with an explicit cycle-oriented diagnostic.

## Language Surface and Semantics

### Module Identity

- A module identity is based on canonical file path.
- Different path spellings that refer to the same canonical location must be treated as the same module.

### Import Resolution Scope

- First release resolves modules via relative file locations from the importing file.
- Relative resolution must be deterministic and platform-consistent.
- If a path cannot be resolved, compilation or startup must fail with a clear message.

### Export Model

- First release adopts explicit export declarations to avoid accidental API leakage.
- Non-exported names remain private to the declaring module.
- Export visibility is stable and independent of import order.

### Import Visibility Model

- First release uses explicit imported names and namespace imports as the primary mechanisms.
- Imported names are read-only bindings from the importer perspective.
- Imported names participate in normal lexical lookup after local scopes.

### Initialization Behavior

- Each module executes top-level initialization once per process execution.
- Repeated imports of the same module reuse the prior evaluated module state.
- Top-level side effects happen at most once for a module identity.

### Cycle Handling

- Cycles are detected in the module graph during loading.
- First release rejects cycles with a dedicated error class and clear path chain.
- Error text should help users break dependency loops.

### Error Semantics

The language must define clear user-facing errors for:

- Module file not found.
- Module file not readable.
- Import references unknown exported name.
- Duplicate export declaration in the same module.
- Duplicate imported name collision in the same scope.
- Cyclic dependency detected.
- Invalid module declaration structure.

Error messages should include file and line context whenever available.

## Name Resolution Rules

- Local declarations have highest precedence inside their scope.
- Imported bindings and module namespace bindings follow lexical rules.
- Private names in imported modules are never visible.
- Name collisions must trigger deterministic compile-time diagnostics.

## Backward Compatibility

- Existing single-file programs continue to run unchanged.
- Programs without modules should observe no semantic behavior change.
- Existing language keywords and built-in functions remain compatible unless explicitly reserved by accepted language change process.

## Security and Safety Considerations

- Relative path traversal behavior must be clearly defined and constrained to expected file-system semantics.
- Error output should avoid leaking unnecessary host environment details.
- Repeated import execution must be prevented to reduce accidental repeated side effects.

## Developer Experience Considerations

- Diagnostics should identify both importer and imported file locations.
- Failure modes should clearly distinguish syntax errors from module graph errors.
- Documentation should provide migration guidance from single-file to multi-file projects.

## Tooling and Ecosystem Considerations

- Formatting and linting tools need a stable understanding of import and export declarations.
- Future IDE features should be able to map symbol origins across module boundaries.
- The specification should preserve room for future package roots and aliasing.

## Testing and Validation Requirements

The feature is considered acceptable only if the following behavior classes are covered by automated tests:

- Successful imports across multiple files.
- Single-execution behavior under repeated imports.
- Explicit export visibility rules.
- Invalid import diagnostics.
- Cycle diagnostics.
- Name collision diagnostics.
- Mixed module and non-module usage.
- Cross-platform path behavior under supported operating systems.

## Rollout Plan

### Phase 1: Minimal Module Support

- Relative imports.
- Explicit exports.
- Explicit imported names and namespace imports.
- Single-execution semantics.
- Cycle rejection.

### Phase 2: Ergonomics and Diagnostics

- Improved diagnostic quality and remediation hints.
- Documentation examples and migration notes.
- Broader test matrix for path edge cases.

### Phase 3: Future Extension (Not committed)

- Package roots.
- Import aliasing enhancements.
- Potential package manager integration.

## Open Questions

- Should export defaults be supported in the first release or deferred?
- Should namespace imports be mandatory for ambiguous name scenarios?
- What exact path normalization behavior should be guaranteed across all supported platforms?
- What reserved words policy should be used if new module-related keywords are introduced?

## Acceptance Criteria

- Feature behavior matches the semantics in this document.
- Error classes and messages are consistent and actionable.
- Existing non-module programs remain compatible.
- Test suites covering module behaviors pass across supported operating systems.
- Documentation is published and linked from project planning and user-facing docs.
