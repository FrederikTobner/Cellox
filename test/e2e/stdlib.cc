#include <gtest/gtest.h>

#include "test_cellox.hh"

// ── math.clx ─────────────────────────────────────────────────────────────────

TEST(StdlibMath, Abs) {
    test_cellox_program("stdlib/math_abs.clx", "7\n3\n0\n");
}

TEST(StdlibMath, MinMax) {
    test_cellox_program("stdlib/math_min_max.clx", "3\n-1\n8\n2\n");
}

TEST(StdlibMath, Clamp) {
    test_cellox_program("stdlib/math_clamp.clx", "5\n0\n10\n");
}

TEST(StdlibMath, FloorCeil) {
    test_cellox_program("stdlib/math_floor_ceil.clx", "3\n-2\n4\n-1\n");
}

TEST(StdlibMath, Pow) {
    test_cellox_program("stdlib/math_pow.clx", "1\n8\n81\n");
}

// ── string.clx ───────────────────────────────────────────────────────────────

TEST(StdlibString, StartsWith) {
    test_cellox_program("stdlib/string_starts_with.clx", "true\nfalse\nfalse\n");
}

TEST(StdlibString, EndsWith) {
    test_cellox_program("stdlib/string_ends_with.clx", "true\nfalse\nfalse\n");
}

TEST(StdlibString, Repeat) {
    test_cellox_program("stdlib/string_repeat.clx", "ababab\nx\n\n");
}

TEST(StdlibString, Pad) {
    test_cellox_program("stdlib/string_pad.clx", "00042\nhi...\n");
}

TEST(StdlibString, Contains) {
    test_cellox_program("stdlib/string_contains.clx", "true\nfalse\ntrue\n");
}

TEST(StdlibString, Reverse) {
    test_cellox_program("stdlib/string_reverse.clx", "olleh\nba\na\n");
}

// ── array.clx ────────────────────────────────────────────────────────────────

TEST(StdlibArray, Contains) {
    test_cellox_program("stdlib/array_contains.clx", "true\nfalse\n");
}

TEST(StdlibArray, IndexOf) {
    test_cellox_program("stdlib/array_index_of.clx", "1\n-1\n");
}

TEST(StdlibArray, SumAvg) {
    test_cellox_program("stdlib/array_sum_avg.clx", "15\n3\n");
}

TEST(StdlibArray, MinMax) {
    test_cellox_program("stdlib/array_min_max.clx", "1\n9\n");
}

TEST(StdlibArray, Count) {
    test_cellox_program("stdlib/array_count.clx", "3\n0\n");
}

// ── io.clx ───────────────────────────────────────────────────────────────────

TEST(StdlibIo, Println) {
    test_cellox_program("stdlib/io_println.clx", "hello\nworld\n");
}

TEST(StdlibIo, Separator) {
    test_cellox_program("stdlib/io_separator.clx", "-----\n---\n");
}

// ── os.clx ───────────────────────────────────────────────────────────────────

TEST(StdlibOs, Name) {
#ifdef __linux__
    test_cellox_program("stdlib/os_name.clx", "linux\n");
#elif defined(__APPLE__)
    test_cellox_program("stdlib/os_name.clx", "macos\n");
#elif defined(_WIN32)
    test_cellox_program("stdlib/os_name.clx", "windows\n");
#endif
}

TEST(StdlibOs, Benchmark) {
    test_cellox_program("stdlib/os_benchmark.clx", "true\n");
}
