#pragma once

#include <fstream>
#include <gtest/gtest.h>
#include <sstream>
#include <string>

class StdlibLayoutFixture : public ::testing::Test {
  protected:
    std::string read_text_file(std::string const & path) {
        std::ifstream input(path);
        if (!input.is_open()) {
            return "";
        }

        std::ostringstream buffer;
        buffer << input.rdbuf();
        return buffer.str();
    }

    std::string stdlib_file_path(char const * relativeName) {
        std::string path = TEST_PROGRAM_BASE_PATH;
        path += "../../stdlib/";
        path += relativeName;
        return path;
    }
};
