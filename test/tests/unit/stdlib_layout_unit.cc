#include <gtest/gtest.h>

#include <string>

#include "../fixtures/stdlib_fixture.h"

TEST_F(StdlibLayoutFixture, CollectionsModuleExistsAndExportsFactories) {
    // Act
    std::string text = read_text_file(stdlib_file_path("collections.clx"));
    // Assert
    ASSERT_FALSE(text.empty());

    EXPECT_NE(std::string::npos, text.find("class Stack"));
    EXPECT_NE(std::string::npos, text.find("class Queue"));
    EXPECT_NE(std::string::npos, text.find("class LinkedList"));
    EXPECT_NE(std::string::npos, text.find("class SetLike"));

    EXPECT_NE(std::string::npos, text.find("export fun stack()"));
    EXPECT_NE(std::string::npos, text.find("export fun queue()"));
    EXPECT_NE(std::string::npos, text.find("export fun linked_list()"));
    EXPECT_NE(std::string::npos, text.find("export fun set_like()"));
}

TEST_F(StdlibLayoutFixture, ViewModuleExistsAndExportsArrayViewFactory) {
    // Act
    std::string text = read_text_file(stdlib_file_path("view.clx"));
    // Assert
    ASSERT_FALSE(text.empty());

    EXPECT_NE(std::string::npos, text.find("class ArrayView"));
    EXPECT_NE(std::string::npos, text.find("map(fn)"));
    EXPECT_NE(std::string::npos, text.find("filter(predicate)"));
    EXPECT_NE(std::string::npos, text.find("reduce(fn, initial)"));
    EXPECT_NE(std::string::npos, text.find("take(count)"));
    EXPECT_NE(std::string::npos, text.find("drop(count)"));

    EXPECT_NE(std::string::npos, text.find("export fun array_view(data)"));
}

TEST_F(StdlibLayoutFixture, ExistingCoreModulesStillPresent) {
    // Assert
    EXPECT_FALSE(read_text_file(stdlib_file_path("math.clx")).empty());
    EXPECT_FALSE(read_text_file(stdlib_file_path("string.clx")).empty());
    EXPECT_FALSE(read_text_file(stdlib_file_path("array.clx")).empty());
    EXPECT_FALSE(read_text_file(stdlib_file_path("io.clx")).empty());
    EXPECT_FALSE(read_text_file(stdlib_file_path("os.clx")).empty());
}
