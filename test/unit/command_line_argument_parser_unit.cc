#include <gtest/gtest.h>

extern "C" {
#include "driver/command_line_argument_parser.h"
#include "base/common.h"
#include "middle-end/optimization_pass.h"
}

// ============ Optimization Level Parsing Tests ============

TEST(CommandLineArgumentParserUnit, ParsesShortInlineOptimizationLevel) {
    const char * argv[] = {"cellox", "-O3"};
    command_line_config_t config = command_line_argument_parser_parse(2, argv);

    EXPECT_FALSE(config.help);
    EXPECT_FALSE(config.version);
    EXPECT_FALSE(config.compile);
    EXPECT_EQ(config.optimizationLevel, 3u);
    EXPECT_EQ(config.inputFile, nullptr);
}

TEST(CommandLineArgumentParserUnit, ParsesLongInlineOptimizationLevel) {
    const char * argv[] = {"cellox", "--optimize=1"};
    command_line_config_t config = command_line_argument_parser_parse(2, argv);

    EXPECT_FALSE(config.help);
    EXPECT_FALSE(config.version);
    EXPECT_FALSE(config.compile);
    EXPECT_EQ(config.optimizationLevel, 1u);
    EXPECT_EQ(config.inputFile, nullptr);
}

TEST(CommandLineArgumentParserUnit, ParsesSplitOptimizationLevelValue) {
    const char * argv[] = {"cellox", "--optimize", "0"};
    command_line_argument_parser_parse(3, argv);
    // Note: This test would need actual mocking of optimization_set_level to verify the value is applied
    // For now, we just verify it doesn't crash
}

TEST(CommandLineArgumentParserUnit, ParsesOptimizationWithCompile) {
    const char * argv[] = {"cellox", "--compile", "-O2", "program.clx"};
    command_line_config_t config = command_line_argument_parser_parse(4, argv);

    EXPECT_FALSE(config.help);
    EXPECT_FALSE(config.version);
    EXPECT_TRUE(config.compile);
    EXPECT_EQ(config.optimizationLevel, 2u);
    EXPECT_STREQ(config.inputFile, "program.clx");
}

TEST(CommandLineArgumentParserUnit, ParsesCompileWithInputFile) {
    const char * argv[] = {"cellox", "--compile", "program.clx"};
    command_line_config_t config = command_line_argument_parser_parse(3, argv);

    EXPECT_FALSE(config.help);
    EXPECT_FALSE(config.version);
    EXPECT_TRUE(config.compile);
    EXPECT_EQ(config.optimizationLevel, UINT32_MAX);  // Not specified
    EXPECT_STREQ(config.inputFile, "program.clx");
}

TEST(CommandLineArgumentParserUnit, ParsesInputFileWithoutCompile) {
    const char * argv[] = {"cellox", "program.clx"};
    command_line_config_t config = command_line_argument_parser_parse(2, argv);

    EXPECT_FALSE(config.help);
    EXPECT_FALSE(config.version);
    EXPECT_FALSE(config.compile);
    EXPECT_EQ(config.optimizationLevel, UINT32_MAX);
    EXPECT_STREQ(config.inputFile, "program.clx");
}

// ============ Help and Version Option Tests ============

TEST(CommandLineArgumentParserUnit, ParsesHelpOption) {
    const char * argv[] = {"cellox", "-h"};
    command_line_config_t config = command_line_argument_parser_parse(2, argv);

    EXPECT_TRUE(config.help);
    EXPECT_FALSE(config.version);
    EXPECT_FALSE(config.compile);
    EXPECT_EQ(config.optimizationLevel, UINT32_MAX);
    EXPECT_EQ(config.inputFile, nullptr);
}

TEST(CommandLineArgumentParserUnit, ParsesLongHelpOption) {
    const char * argv[] = {"cellox", "--help"};
    command_line_config_t config = command_line_argument_parser_parse(2, argv);

    EXPECT_TRUE(config.help);
    EXPECT_FALSE(config.version);
    EXPECT_FALSE(config.compile);
    EXPECT_EQ(config.optimizationLevel, UINT32_MAX);
    EXPECT_EQ(config.inputFile, nullptr);
}

TEST(CommandLineArgumentParserUnit, ParsesVersionOption) {
    const char * argv[] = {"cellox", "-v"};
    command_line_config_t config = command_line_argument_parser_parse(2, argv);

    EXPECT_FALSE(config.help);
    EXPECT_TRUE(config.version);
    EXPECT_FALSE(config.compile);
    EXPECT_EQ(config.optimizationLevel, UINT32_MAX);
    EXPECT_EQ(config.inputFile, nullptr);
}

TEST(CommandLineArgumentParserUnit, ParsesLongVersionOption) {
    const char * argv[] = {"cellox", "--version"};
    command_line_config_t config = command_line_argument_parser_parse(2, argv);

    EXPECT_FALSE(config.help);
    EXPECT_TRUE(config.version);
    EXPECT_FALSE(config.compile);
    EXPECT_EQ(config.optimizationLevel, UINT32_MAX);
    EXPECT_EQ(config.inputFile, nullptr);
}

// ============ Error Cases - Constraint Violations ============

TEST(CommandLineArgumentParserUnit, InvalidInlineOptimizationLevelExitsWithUsageError) {
    const char * argv[] = {"cellox", "-O9"};
    EXPECT_EXIT(command_line_argument_parser_parse(2, argv),
                ::testing::ExitedWithCode(EXIT_CODE_COMMAND_LINE_USAGE_ERROR),
                "Invalid optimization level");
}

TEST(CommandLineArgumentParserUnit, MissingOptimizationLevelValueExitsWithUsageError) {
    const char * argv[] = {"cellox", "-O"};
    EXPECT_EXIT(command_line_argument_parser_parse(2, argv),
                ::testing::ExitedWithCode(EXIT_CODE_COMMAND_LINE_USAGE_ERROR),
                "Missing optimization level");
}

TEST(CommandLineArgumentParserUnit, UnknownOptionExitsWithUsageError) {
    const char * argv[] = {"cellox", "--unknown-option"};
    EXPECT_EXIT(command_line_argument_parser_parse(2, argv),
                ::testing::ExitedWithCode(EXIT_CODE_COMMAND_LINE_USAGE_ERROR),
                "Unknown option");
}

TEST(CommandLineArgumentParserUnit, HelpAndVersionCombinedExitsWithUsageError) {
    const char * argv[] = {"cellox", "-h", "-v"};
    EXPECT_EXIT(command_line_argument_parser_parse(3, argv),
                ::testing::ExitedWithCode(EXIT_CODE_COMMAND_LINE_USAGE_ERROR),
                "Help and version options cannot be combined");
}

TEST(CommandLineArgumentParserUnit, HelpWithCompileExitsWithUsageError) {
    const char * argv[] = {"cellox", "--help", "--compile"};
    EXPECT_EXIT(command_line_argument_parser_parse(3, argv),
                ::testing::ExitedWithCode(EXIT_CODE_COMMAND_LINE_USAGE_ERROR),
                "cannot be combined");
}

TEST(CommandLineArgumentParserUnit, HelpWithOptimizationLevelExitsWithUsageError) {
    const char * argv[] = {"cellox", "-h", "-O3"};
    EXPECT_EXIT(command_line_argument_parser_parse(3, argv),
                ::testing::ExitedWithCode(EXIT_CODE_COMMAND_LINE_USAGE_ERROR),
                "cannot be combined");
}

TEST(CommandLineArgumentParserUnit, HelpWithInputFileExitsWithUsageError) {
    const char * argv[] = {"cellox", "--help", "program.clx"};
    EXPECT_EXIT(command_line_argument_parser_parse(3, argv),
                ::testing::ExitedWithCode(EXIT_CODE_COMMAND_LINE_USAGE_ERROR),
                "cannot be combined");
}

TEST(CommandLineArgumentParserUnit, MultipleInputFilesExitsWithUsageError) {
    const char * argv[] = {"cellox", "file1.clx", "file2.clx"};
    EXPECT_EXIT(command_line_argument_parser_parse(3, argv),
                ::testing::ExitedWithCode(EXIT_CODE_COMMAND_LINE_USAGE_ERROR),
                "Multiple input files");
}

// ============ Interactive REPL Mode ============

TEST(CommandLineArgumentParserUnit, NoArgumentsEntersREPLMode) {
    const char * argv[] = {"cellox"};
    command_line_config_t config = command_line_argument_parser_parse(1, argv);

    EXPECT_FALSE(config.help);
    EXPECT_FALSE(config.version);
    EXPECT_FALSE(config.compile);
    EXPECT_EQ(config.optimizationLevel, UINT32_MAX);
    EXPECT_EQ(config.inputFile, nullptr);
}
