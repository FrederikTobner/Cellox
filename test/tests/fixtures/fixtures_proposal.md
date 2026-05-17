# Test Fixture Proposal

This proposal is focused on reducing duplication in the existing tests and keeping individual files closer to ~200 LOC.

## Current pain points

- `test/unit/chunk_disassembler_unit.cc` is very large (~642 lines).
- It contains repeated fixtures for the same `chunk_t` setup/teardown across multiple derived classes.
- `test/unit/optimization_pass_unit.cc` repeats `virtual_machine_init()`, `virtual_machine_free()`, `chunk_init()`, and `chunk_free()` patterns.
- `test/unit/core_unit.cc` and `test/unit/property_fuzz_unit.cc` also create and destroy chunks manually.
- Several tests capture stdout manually in the same way, which is also a reusable pattern.
- E2E and integration tests currently use plain `TEST` cases in many files, but they often share common program invocation, file path, or environment setup patterns.

## Recommended fixture structure

Create a `test/fixtures/` directory and move shared fixture classes there.

### 1. `ChunkTest`

**File:** `test/fixtures/chunk_fixture.h`

Responsibility:
- Own a `chunk_t` instance
- Initialize and free it in `SetUp()` / `TearDown()`
- Provide helpers for emitting instructions

Proposed API:
- `chunk_t chunk;`
- `void SetUp() override;`
- `void TearDown() override;`
- `void write1(uint8_t op, uint32_t line);`
- `void write2(uint8_t op, uint8_t operand, uint32_t line);`
- `void write3(uint8_t op, uint8_t b1, uint8_t b2, uint32_t line);`
- Optional helpers: `EmitConstant`, `EmitSimple`.

### 2. `VirtualMachineTest`

**File:** `test/fixtures/vm_fixture.h`

Responsibility:
- Initialize the VM before each test
- Free the VM after each test
- Make VM-backed tests easier to express

Proposed API:
- `VirtualMachineTest : public ::testing::Test`
- `void SetUp() override { virtual_machine_init(); }`
- `void TearDown() override { virtual_machine_free(); }`

### 3. `ChunkVMTest`

**File:** `test/fixtures/chunk_vm_fixture.h`

Responsibility:
- Compose `ChunkTest` and `VirtualMachineTest`
- Reuse both chunk and VM lifecycle for tests that need both

Proposed API:
- `class ChunkVMTest : public VirtualMachineTest, public ChunkTest {}`

This can be used by optimization-pass tests and any other tests that need a chunk + VM.

### 4. `ChunkDisassemblerTest`

**File:** `test/fixtures/chunk_disassembler_fixture.h`

Responsibility:
- Inherit from `ChunkTest`
- Add disassembler-specific helpers and stdout capture helpers

Proposed API:
- `std::string captureDisassembleAt(int offset);`
- `int32_t disassembleInstruction(int offset);`
- `std::string disassembleChunk(const char *name, int offset);`

This lets all `ChunkDisassembler*` fixtures inherit from a single base and share the same helper functions.

### 5. `StdlibLayoutTest`

**File:** `test/fixtures/stdlib_fixture.h`

Responsibility:
- Provide file path helpers for stdlib tests
- Provide a reusable `read_text_file()` helper

Proposed API:
- `std::string readTextFile(const std::string &path);`
- `std::string stdlibFilePath(const char *relativeName);`

## Proposed test fixture classes

### `ChunkFixture`

```cpp
class ChunkFixture : public ::testing::Test {
protected:
    chunk_t chunk;

    void SetUp() override {
        chunk_init(&chunk);
    }

    void TearDown() override {
        chunk_free(&chunk);
    }

    void write1(uint8_t op, uint32_t line) {
        chunk_write(&chunk, op, line);
    }

    void write2(uint8_t op, uint8_t operand, uint32_t line) {
        chunk_write(&chunk, op, line);
        chunk_write(&chunk, operand, line);
    }

    void write3(uint8_t op, uint8_t b1, uint8_t b2, uint32_t line) {
        chunk_write(&chunk, op, line);
        chunk_write(&chunk, b1, line);
        chunk_write(&chunk, b2, line);
    }
};
```

### `VirtualMachineFixture`

```cpp
class VirtualMachineFixture : public ::testing::Test {
protected:
    void SetUp() override {
        virtual_machine_init();
    }

    void TearDown() override {
        virtual_machine_free();
    }
};
```

### `ChunkVMFixture`

```cpp
class ChunkVMFixture : public VirtualMachineFixture, public ChunkFixture {};
```

### `ChunkDisassemblerFixture`

```cpp
class ChunkDisassemblerFixture : public ChunkFixture {
protected:
    std::string captureDisassemble(int offset) {
        testing::internal::CaptureStdout();
        chunk_disassembler_disassemble_instruction(&chunk, offset);
        return testing::internal::GetCapturedStdout();
    }

    int32_t disassembleInstruction(int offset) {
        testing::internal::CaptureStdout();
        int32_t next = chunk_disassembler_disassemble_instruction(&chunk, offset);
        testing::internal::GetCapturedStdout();
        return next;
    }
};
```

## Recommended splits for large files

### Chunk disassembler
Split `chunk_disassembler_unit.cc` into smaller files, all sharing `ChunkDisassemblerFixture`:
- `test/unit/chunk_disassembler_simple_unit.cc`
- `test/unit/chunk_disassembler_byte_unit.cc`
- `test/unit/chunk_disassembler_constant_unit.cc`
- `test/unit/chunk_disassembler_jump_unit.cc`
- `test/unit/chunk_disassembler_invoke_unit.cc`
- `test/unit/chunk_disassembler_chunk_unit.cc`

This keeps each file closer to 150-200 lines and improves readability.

### Optimization pass tests
Split `optimization_pass_unit.cc` into focused files:
- `test/unit/optimization_pass_constant_folding_unit.cc`
- `test/unit/optimization_pass_dead_code_unit.cc`
- `test/unit/optimization_pass_branch_predication_unit.cc`

Use `ChunkVMFixture` for shared VM + chunk lifecycle.

## Additional fixture opportunities

### Stdout capture helper
Many tests use `testing::internal::CaptureStdout()` manually. A shared helper reduces noise:

```cpp
static std::string captureStdout(std::function<void()> fn) {
    testing::internal::CaptureStdout();
    fn();
    return testing::internal::GetCapturedStdout();
}
```

### Command line parser tests
If more CLI tests are added, a small fixture can hold common argv builders or config expectations.

### File-based / environment tests
Integration tests that load `.clx` files or stdlib files can share a fixture that sets up `TEST_PROGRAM_BASE_PATH`, path helpers, and environment cleanup.

## E2E / Integration fixture opportunities

Most end-to-end and integration tests are currently written as plain `TEST(...)` cases, but they still share common concerns:
- launching the compiler/runner or module loader
- setting up temporary files or stdlib paths
- capturing stdout/stderr
- resetting environment variables or working directories

A shared fixture for these layers can provide:
- `TestProgramFixture` for CLI/runner execution
- `StdlibIntegrationFixture` for path and module loader setup
- `CaptureOutputFixture` for consistent stdout/stderr capture and restoration

That keeps the unit/test fixture model consistent across all test layers, while preserving the simpler `TEST(...)` style for cases that do not need shared state.

## Why this helps

- Centralizes chunk setup/teardown in one place instead of repeating it in six fixture classes.
- Allows `ChunkDisassembler*` tests to inherit behavior instead of duplicating identical `SetUp()`/`TearDown()` code.
- Makes `optimization_pass_unit.cc` smaller by moving VM lifecycle into a base fixture.
- Enables test file splits without repeating boilerplate.
- Keeps future fixtures consistent as new test suites are added.

## Suggested directory layout

```
test/
  fixtures/
    chunk_fixture.h
    chunk_disassembler_fixture.h
    vm_fixture.h
    stdlib_fixture.h
  unit/
    chunk_disassembler_simple_unit.cc
    chunk_disassembler_byte_unit.cc
    chunk_disassembler_constant_unit.cc
    chunk_disassembler_jump_unit.cc
    chunk_disassembler_invoke_unit.cc
    chunk_disassembler_chunk_unit.cc
    optimization_pass_constant_folding_unit.cc
    optimization_pass_dead_code_unit.cc
    optimization_pass_branch_predication_unit.cc
    command_line_argument_parser_unit.cc
    core_unit.cc
    error_runtime_unit.cc
    property_fuzz_unit.cc
    stdlib_layout_unit.cc
```

## Next step
If you want, I can also prepare the first fixture header and move the `ChunkDisassembler` setup into it so the refactor becomes concrete.