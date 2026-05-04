# Pipe Binding Extension Options

This document captures all currently identified language changes to extend `|err|`-style bindings beyond current usage.

## Current State

Cellox currently supports `|name|` binding in statement form for `iferror`:

```cellox
iferror expr |err| {
    // error branch
} else |val| {
    // success branch
}
```

Expression-level `catch` currently supports fallback values but does not bind the error value.

## Problem Summary

The binding form is powerful but underused:

- Expression recovery is concise, but cannot inspect `err`.
- Detailed error handling requires switching to statement form.
- This creates friction and duplicated patterns in otherwise expression-oriented code.

## Design Goals

- Keep existing semantics stable.
- Add expressive error binding where users already write `catch`.
- Prefer parser-level changes that reuse existing runtime opcodes.
- Roll out incrementally with minimal breakage risk.

## Non-Error Use Cases for Pipe Binders

The same `|name|` binding idea can improve more than error handling. The core value is: evaluate once, bind a short-lived name, and keep logic local.

### 1. Conditional value binding in `if`

```cellox
if parse_int(text) |n| {
    printf("{}\n", n + 1);
} else {
    printf("invalid\n");
}
```

Why useful:

- Avoids temporary variables outside the branch.
- Avoids double-evaluating expensive expressions.
- Keeps bound value scoped to the branch.

### 2. Loop binding in `while`

```cellox
while next_token(scanner) |tok| {
    printf("{}\n", tok);
}
```

Why useful:

- Natural for iterator/stream APIs.
- Eliminates repeated `var t = ...; if (...) break;` patterns.

### 3. Loop element destructuring in `for`

```cellox
for entry in map |key, value| {
    printf("{}:{}\n", key, value);
}
```

Why useful:

- More ergonomic than index + manual extraction.
- Fits collection-heavy code and stdlib container APIs.

### 4. Pattern binders in `match`/`switch`-style branching

```cellox
match node {
    Binary |lhs, op, rhs| => eval(lhs) + eval(rhs),
    Number |n| => n,
}
```

Why useful:

- Uniform binder model across branch constructs.
- Sets foundation for future AST/data-pattern work.

### 5. Resource/handle binders in `with`/`using` statements

```cellox
with open_file(path) |f| {
    println(read_all(f));
}
```

Why useful:

- Clear ownership scope.
- Good fit for file/socket/resource APIs.

### 6. Import namespace aliasing binders

```cellox
import "./math.clx" |math|;
println(math.add(1, 2));
```

Why useful:

- Prevents global symbol pollution.
- Useful for large modules or naming collisions.

### 7. Filter/guard binders in comprehensions or pipelines

```cellox
var ys = xs
    |> map(|x| transform(x))
    |> filter(|y| y > 0);
```

or statement form:

```cellox
if value |v| when v > 0 {
    println(v);
}
```

Why useful:

- Cleaner local naming in data-flow code.
- Reduces temporary-variable noise.

### 8. Regex/parse captures with named binder scope

```cellox
if match(text, "(\\d+)-(\\d+)") |whole, a, b| {
    println(a);
}
```

Why useful:

- Makes capture-group usage explicit and scoped.
- Strong readability boost in parser-like code.

## Prioritization Outside Error Handling

If adopted incrementally, this order usually yields best value-to-complexity ratio:

1. `if expr |x| { ... } else { ... }`
2. `while expr |x| { ... }`
3. import alias binders (`import ... |ns|`)
4. multi-bind destructuring (`|a, b|`) in loops and match

This keeps parser complexity controlled while extending a single mental model across statements and expressions.

## Option 1: Catch Binder (Expression-Level)

### Syntax

```cellox
result = read_file(path) catch |err| fallback(err);
```

### Grammar Addition

```ebnf
error_catch -> equality ( "catch" catch_handler )* ;

catch_handler -> equality
               | "|" IDENTIFIER "|" handler_expr ( "else" "|" IDENTIFIER "|" handler_expr )? ;

handler_expr -> expression | handler_block ;
handler_block -> "{" expression ( ";" expression )* ";"? "}" ;
```

### Semantics

- If lhs is success: unwrap and return success value.
- If lhs is error: bind payload to `err` and evaluate fallback expression.

### Pros

- Biggest ergonomics improvement for smallest language change.
- Keeps value-producing style.
- Can be implemented largely in parser/compiler lowering.

### Cons

- Introduces a new scoped binder in expression context.
- Requires clear precedence + associativity documentation.

### Implementation Notes

- Reuse `OP_RESULT_IS_ERROR`, `OP_RESULT_UNWRAP`, `OP_RESULT_UNWRAP_ERROR`.
- Compiler can open a temporary scope for the bound variable in the error path.

## Option 2: Catch Binder With Block Handler

### Syntax

```cellox
result = read_file(path) catch |err| {
    log(err);
    "default";
};
```

### Grammar Addition

```ebnf
catch_handler -> equality
               | "|" IDENTIFIER "|" expression
               | "|" IDENTIFIER "|" blockExpr ;

blockExpr -> "{" declaration* expression ";" "}" ;
```

### Semantics

- Same as Option 1, but fallback can be multi-step.

### Pros

- Strong expressiveness.
- Eliminates need to switch to `iferror` for richer fallback.

### Cons

- Requires block-expression support (new concept).
- Larger parser and language-surface change than Option 1.

## Option 3: Expression-Level Iferror Form

### Syntax

```cellox
value = iferror open(path) |err| err_to_default(err) else |val| transform(val);
```

### Grammar Addition

```ebnf
primary -> ...
         | iferrorExpr ;

iferrorExpr -> "iferror" expression
               "|" IDENTIFIER "|" handler_expr
               "else"
               "|" IDENTIFIER "|" handler_expr ;
```

### Semantics

- Value-producing twin of statement `iferror`.

### Pros

- Conceptual symmetry with existing statement form.
- Gives both branches first-class values.

### Cons

- Adds another expression construct with branch semantics.
- Can overlap mentally with `catch` and increase feature surface.

## Option 4: Guard Statement With Pipe Binders

### Syntax

```cellox
guard open(path) |err| {
    return err;
} else |val| {
    use(val);
}
```

### Grammar Addition

```ebnf
statement -> ... | guardStmt ;

guardStmt -> "guard" expression
             "|" IDENTIFIER "|" block
             "else"
             "|" IDENTIFIER "|" block ;
```

### Semantics

- Statement-level clarity for early-exit workflows.

### Pros

- Highly readable in imperative flows.
- Easy migration target from repetitive `iferror` patterns.

### Cons

- New keyword + near-duplicate of `iferror` statement.
- Weaker ROI than improving `catch` first.

## Option 5: Typed or Patterned Pipe Binders

### Syntax Candidates

```cellox
iferror expr |IoError.OpenFailed as err| { ... } else |val| { ... }
expr catch |IoError.OpenFailed as err| fallback
```

### Grammar Direction

```ebnf
pipeBinding -> "|" patternBinding "|" ;
patternBinding -> IDENTIFIER
                | qualifiedErrorName "as" IDENTIFIER ;
```

### Pros

- Enables variant-specific handling inline.
- Strong path toward future pattern matching.

### Cons

- Significant complexity jump.
- Requires robust type/variant matching rules.

## Option 6: Multiple Pipe Binders in Catch Chain

### Syntax

```cellox
value = connect(a)
    catch |timeoutErr| retry(timeoutErr)
    catch |refusedErr| fallback(refusedErr)
    catch "default";
```

### Semantics

- Each chained `catch` may independently bind error payload when active.

### Pros

- Natural extension of existing catch chaining.
- No new keyword.

### Cons

- Requires careful lowering rules in chained control flow.

## Option 7: Optional Else Binder in Statement Iferror

### Syntax

```cellox
iferror expr |err| {
    handle(err);
} else {
    // no bound success value needed
}
```

or

```cellox
iferror expr |err| {
    handle(err);
} else |val| {
    use(val);
}
```

### Pros

- Reduces ceremony where success value is unused.

### Cons

- Minor gain; does not address expression-side binder need.

## Option 8: Try/Catch Pipe Statement

### Syntax

```cellox
try {
    var x = must might_fail();
} catch |err| {
    log(err);
}
```

### Pros

- Familiar syntax for many users.

### Cons

- Pushes design toward exception-like shape.
- Collides conceptually with current explicit-result direction.
- Not recommended for near-term roadmap.

## Compatibility and Risk Assessment

- Lowest risk: Option 1, Option 7.
- Medium risk: Option 3, Option 4, Option 6.
- Highest risk: Option 2, Option 5, Option 8.

## Recommended Rollout

1. Implement Option 1 first (`catch |err| expr`).
2. Add Option 6 support naturally as part of catch-chain lowering.
3. Add Option 7 as a small quality-of-life follow-up.
4. Re-evaluate Option 3 only after usage feedback.
5. Defer Options 2/5/8 until a broader expression/pattern roadmap exists.

## Suggested Test Matrix

- Success path with binder syntax (binder not executed).
- Error path with binder syntax (binder receives exact payload).
- Chained catches mixing bound and unbound handlers.
- Scope hygiene: binder not visible outside handler.
- Precedence interactions with `or`, `and`, assignment.
- Nested usage inside function calls and assignments.
- Backward compatibility for existing `catch fallback` and `iferror` tests.

## Grammar Patch Candidate (Minimal, Incremental)

```ebnf
logic_and -> error_catch ( "and" error_catch )* ;

error_catch -> equality ( "catch" catch_handler )* ;

catch_handler -> equality
               | "|" IDENTIFIER "|" equality ;
```

This is the minimal change that unlocks bound errors in expression context while preserving current grammar shape.

## High-ROI Insertion Points in Current Grammar

This section maps directly to the current rules in grammer.md and highlights where changes are both easy and high impact.

### Tier 1: Easy + Huge Benefit

1. Update error_catch in expression layer.

Current:

```ebnf
error_catch -> equality ( "catch" equality )* ;
```

Proposed:

```ebnf
error_catch -> equality ( "catch" catch_handler )* ;
catch_handler -> equality | "|" IDENTIFIER "|" equality ;
```

Why this is best first:

- Lowest parser risk: one production expansion.
- Highest ergonomic win: bound error payload without leaving expression flow.
- Fully aligned with expression-first design.

2. Upgrade iferror from statement-only to expression-capable form.

Current statement form already exists:

```ebnf
iferrorStmt -> "iferror" expression "|" IDENTIFIER "|" block "else" "|" IDENTIFIER "|" block ;
```

Add expression form:

```ebnf
primary -> ... | iferrorExpr ;
iferrorExpr -> "iferror" expression "|" IDENTIFIER "|" expression "else" "|" IDENTIFIER "|" expression ;
```

Why this is high value:

- Reuses known syntax, no new keyword.
- Makes iferror composable in assignments, arguments, and returns.
- Keeps statement form as compatibility sugar.

### Tier 2: Medium Effort + Strong Benefit

3. Add optional success binder in catch.

```ebnf
catch_handler -> equality
               | "|" IDENTIFIER "|" handler_expr ( "else" "|" IDENTIFIER "|" handler_expr )? ;
```

Why:

- Symmetry with iferror branches.
- Useful when both error and success need local naming in expression pipelines.

4. Add block expression support for rich handlers.

```ebnf
primary -> ... | blockExpr ;
blockExpr -> "{" declaration* expression ";" "}" ;
```

Why:

- Unlocks multi-step expression handlers in catch and iferrorExpr.
- Enables Zig-like expression-oriented style across control constructs.

### Tier 3: Defer Until Core Is Stable

5. Non-error binder forms in statements (`if expr |x|`, `while expr |x|`).

These are valuable, but they introduce new statement parsing patterns and should follow once expression-side binder semantics are stable.

6. Typed/pattern binders (`|Type.Variant as e|`).

Great long-term direction, but should wait for pattern semantics and stronger type/variant rules.

## Expression-First (Zig-Like) Grammar Direction

If we optimize for composability, the grammar should treat statements as thin wrappers around expression forms whenever possible.

Recommended direction:

1. Keep expression as the central composition unit.
2. Add binder-enabled catch and iferror as expressions first.
3. Preserve statement forms as desugaring targets for readability and backward compatibility.
4. Avoid introducing keyword-heavy statement-only constructs before expression forms are complete.

## Practical Minimal Set to Implement Next

If you want maximum user benefit with minimal churn, implement exactly these two grammar edits first:

1. `error_catch -> equality ( "catch" catch_handler )*`
2. `primary -> ... | iferrorExpr`

This gives:

- Bound error capture in expression context.
- Full branch handling in expression context.
- Immediate reuse inside assignments, returns, calls, and nested expressions.
