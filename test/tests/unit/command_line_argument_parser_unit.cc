#include <gtest/gtest.h>

extern "C" {
#include "base/common.h"
#include "driver/command_line_argument_parser.h"
#include "middle-end/optimization_pass.h"
}

// ============ Optimization Level Parsing Tests ============

TEST(CommandLineArgumentParserUnit, ParsesShortInlineOptimizationLevel) {
    // Arrange
    const char * argv[] = {"cellox", "-O3"};
    // Act
    command_line_config_t config = command_line_argument_parser_parse(2, argv);

    // Assert
    EXPECT_FALSE(config.help);
    EXPECT_FALSE(config.version);
    EXPECT_FALSE(config.compile);
    EXPECT_EQ(config.optimizationLevel, 3u);
    EXPECT_EQ(config.inputFile, nullptr);
}

TEST(CommandLineArgumentParserUnit, ParsesLongInlineOptimizationLevel) {
    // Arrange
    const char * argv[] = {"cellox", "--optimize=1"};
    // Act
    command_line_config_t config = command_line_argument_parser_parse(2, argv);

    // Assert
    EXPECT_FALSE(config.help);
    EXPECT_FALSE(config.version);
    EXPECT_FALSE(config.compile);
    EXPECT_EQ(config.optimizationLevel, 1u);
    EXPECT_EQ(config.inputFile, nullptr);
}

TEST(CommandLineArgumentParserUnit, ParsesSplitOptimizationLevelValue) {
    // Arrange
    const char * argv[] = {"cellox", "--optimize", "0"};
    // Act
    command_line_argument_parser_parse(3, argv);
    // Note: This test would need actual mocking of optimization_set_level to verify the value is applied
    // For now, we just verify it doesn't crash
}

TEST(CommandLineArgumentParserUnit, ParsesOptimizationWithCompile) {
    // Arrange.
    const char * argv[] = {"cellox", "--compile", "-O2", "program.clx"};
    // Act.
    command_line_config_t config = command_line_argument_parser_parse(4, argv);

    // Assert.
    EXPECT_FALSE(config.help);
    EXPECT_FALSE(config.version);
    EXPECT_TRUE(config.compile);
    EXPECT_EQ(config.optimizationLevel, 2u);
    EXPECT_STREQ(config.inputFile, "program.clx");
}

TEST(CommandLineArgumentParserUnit, ParsesCompileWithInputFile) {
    // Arrange
    const char * argv[] = {"cellox", "--compile", "program.clx"};
    // Act
    command_line_config_t config = command_line_argument_parser_parse(3, argv);

    // Assert
    EXPECT_FALSE(config.help);
    EXPECT_FALSE(config.version);
    EXPECT_TRUE(config.compile);
    EXPECT_EQ(config.optimizationLevel, UINT32_MAX); // Not specified
    EXPECT_STREQ(config.inputFile, "program.clx");
}

TEST(CommandLineArgumentParserUnit, ParsesInputFileWithoutCompile) {
    // Arrange
    const char * argv[] = {"cellox", "program.clx"};
    // Act
    command_line_config_t config = command_line_argument_parser_parse(2, argv);

    // Assert
    EXPECT_FALSE(config.help);
    EXPECT_FALSE(config.version);
    EXPECT_FALSE(config.compile);
    EXPECT_EQ(config.optimizationLevel, UINT32_MAX);
    EXPECT_STREQ(config.inputFile, "program.clx");
}

// ============ Help and Version Option Tests ============

TEST(CommandLineArgumentParserUnit, ParsesHelpOption) {
    // Arrange
    const char * argv[] = {"cellox", "-h"};
    // Act
    command_line_config_t config = command_line_argument_parser_parse(2, argv);

    // Assert
    EXPECT_TRUE(config.help);
    EXPECT_FALSE(config.version);
    EXPECT_FALSE(config.compile);
    EXPECT_EQ(config.optimizationLevel, UINT32_MAX);
    EXPECT_EQ(config.inputFile, nullptr);
}

TEST(CommandLineArgumentParserUnit, ParsesLongHelpOption) {
    // Arrange.
    const char * argv[] = {"cellox", "--help"};
    // Act.
    command_line_config_t config = command_line_argument_parser_parse(2, argv);

    // Assert.
    EXPECT_TRUE(config.help);
    EXPECT_FALSE(config.version);
    EXPECT_FALSE(config.compile);
    EXPECT_EQ(config.optimizationLevel, UINT32_MAX);
    EXPECT_EQ(config.inputFile, nullptr);
}

TEST(CommandLineArgumentParserUnit, ParsesVersionOption) {
    // Arrange
    const char * argv[] = {"cellox", "-v"};
    // Act
    command_line_config_t config = command_line_argument_parser_parse(2, argv);

    // Assert
    EXPECT_FALSE(config.help);
    EXPECT_TRUE(config.version);
    EXPECT_FALSE(config.compile);
    EXPECT_EQ(config.optimizationLevel, UINT32_MAX);
    EXPECT_EQ(config.inputFile, nullptr);
}

TEST(CommandLineArgumentParserUnit, ParsesLongVersionOption) {
    // Arrange
    const char * argv[] = {"cellox", "--version"};
    // Act
    command_line_config_t config = command_line_argument_parser_parse(2, argv);

    // Assert
    EXPECT_FALSE(config.help);
    EXPECT_TRUE(config.version);
    EXPECT_FALSE(config.compile);
    EXPECT_EQ(config.optimizationLevel, UINT32_MAX);
    EXPECT_EQ(config.inputFile, nullptr);
}

// ============ Error Cases - Constraint Violations ============

TEST(CommandLineArgumentParserUnit, InvalidInlineOptimizationLevelExitsWithUsageError) {
    // Arrange
    const char * argv[] = {"cellox", "-O9"};
    // Act / Assert
    EXPECT_EXIT(command_line_argument_parser_parse(2, argv),
                ::testing::ExitedWithCode(EXIT_CODE_COMMAND_LINE_USAGE_ERROR), "Invalid optimization level");
}

TEST(CommandLineArgumentParserUnit, MissingOptimizationLevelValueExitsWithUsageError) {
    // Arrange
    const char * argv[] = {"cellox", "-O"};
    // Act / Assert
    EXPECT_EXIT(command_line_argument_parser_parse(2, argv),
                ::testing::ExitedWithCode(EXIT_CODE_COMMAND_LINE_USAGE_ERROR), "Missing optimization level");
}

TEST(CommandLineArgumentParserUnit, UnknownOptionExitsWithUsageError) {
    // Arrange
    const char * argv[] = {"cellox", "--unknown-option"};
    // Act / Assert
    EXPECT_EXIT(command_line_argument_parser_parse(2, argv),
                ::testing::ExitedWithCode(EXIT_CODE_COMMAND_LINE_USAGE_ERROR), "Unknown option");
}

TEST(CommandLineArgumentParserUnit, HelpAndVersionCombinedExitsWithUsageError) {
    // Arrange
    const char * argv[] = {"cellox", "-h", "-v"};
    // Act / Assert
    EXPECT_EXIT(command_line_argument_parser_parse(3, argv),
                ::testing::ExitedWithCode(EXIT_CODE_COMMAND_LINE_USAGE_ERROR),
                "Help and version options cannot be combined");
}

TEST(CommandLineArgumentParserUnit, HelpWithCompileExitsWithUsageError) {
    // Arrange
    const char * argv[] = {"cellox", "--help", "--compile"};
    // Act / Assert
    EXPECT_EXIT(command_line_argument_parser_parse(3, argv),
                ::testing::ExitedWithCode(EXIT_CODE_COMMAND_LINE_USAGE_ERROR), "cannot be combined");
}

TEST(CommandLineArgumentParserUnit, HelpWithOptimizationLevelExitsWithUsageError) {
    // Arrange
    const char * argv[] = {"cellox", "-h", "-O3"};
    // Act / Assert
    EXPECT_EXIT(command_line_argument_parser_parse(3, argv),
                ::testing::ExitedWithCode(EXIT_CODE_COMMAND_LINE_USAGE_ERROR), "cannot be combined");
}

TEST(CommandLineArgumentParserUnit, HelpWithInputFileExitsWithUsageError) {
    // Arrange
    const char * argv[] = {"cellox", "--help", "program.clx"};
    // Act / Assert
    EXPECT_EXIT(command_line_argument_parser_parse(3, argv),
                ::testing::ExitedWithCode(EXIT_CODE_COMMAND_LINE_USAGE_ERROR), "cannot be combined");
}

TEST(CommandLineArgumentParserUnit, MultipleInputFilesExitsWithUsageError) {
    // Arrange
    const char * argv[] = {"cellox", "file1.clx", "file2.clx"};
    // Act / Assert
    EXPECT_EXIT(command_line_argument_parser_parse(3, argv),
                ::testing::ExitedWithCode(EXIT_CODE_COMMAND_LINE_USAGE_ERROR), "Multiple input files");
}

// ============ Interactive REPL Mode ============

TEST(CommandLineArgumentParserUnit, NoArgumentsEntersREPLMode) {
    // Arrange
    const char * argv[] = {"cellox"};
    // Act
    command_line_config_t config = command_line_argument_parser_parse(1, argv);

    // Assert
    EXPECT_FALSE(config.help);
    EXPECT_FALSE(config.version);
    EXPECT_FALSE(config.compile);
    EXPECT_EQ(config.optimizationLevel, UINT32_MAX);
    EXPECT_EQ(config.inputFile, nullptr);
}
