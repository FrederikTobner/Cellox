#include <gtest/gtest.h>

#include <fstream>
#include <sstream>
#include <string>

static std::string read_text_file(std::string const & path) {
    std::ifstream input(path);
    if (!input.is_open()) {
        return "";
    }
    std::ostringstream buffer;
    buffer << input.rdbuf();
    return buffer.str();
}

static std::string stdlib_file_path(char const * relativeName) {
    std::string path = TEST_PROGRAM_BASE_PATH;
    path += "../stdlib/";
    path += relativeName;
    return path;
}

TEST(StdlibUnit, CollectionsModuleExistsAndExportsFactories) {
    std::string text = read_text_file(stdlib_file_path("collections.clx"));
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

TEST(StdlibUnit, ViewModuleExistsAndExportsArrayViewFactory) {
    std::string text = read_text_file(stdlib_file_path("view.clx"));
    ASSERT_FALSE(text.empty());

    EXPECT_NE(std::string::npos, text.find("class ArrayView"));
    EXPECT_NE(std::string::npos, text.find("map(fn)"));
    EXPECT_NE(std::string::npos, text.find("filter(predicate)"));
    EXPECT_NE(std::string::npos, text.find("reduce(fn, initial)"));
    EXPECT_NE(std::string::npos, text.find("take(count)"));
    EXPECT_NE(std::string::npos, text.find("drop(count)"));

    EXPECT_NE(std::string::npos, text.find("export fun array_view(data)"));
}

TEST(StdlibUnit, ExistingCoreModulesStillPresent) {
    EXPECT_FALSE(read_text_file(stdlib_file_path("math.clx")).empty());
    EXPECT_FALSE(read_text_file(stdlib_file_path("string.clx")).empty());
    EXPECT_FALSE(read_text_file(stdlib_file_path("array.clx")).empty());
    EXPECT_FALSE(read_text_file(stdlib_file_path("io.clx")).empty());
    EXPECT_FALSE(read_text_file(stdlib_file_path("os.clx")).empty());
}
