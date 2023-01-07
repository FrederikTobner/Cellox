#include <gtest/gtest.h>

#include "test_cellox.hh"

TEST(Comments, MultiLine)
{
    test_cellox_program("comments/multi_line.clx", "ok\n");
}

TEST(Comments, Nested)
{
    test_cellox_program("comments/nested.clx", "ok\n");
}

TEST(Comments, SingleLine)
{
    test_cellox_program("comments/single_line.clx", "ok\n");
}

TEST(Comments, UniCodeCharacters)
{
    test_cellox_program("comments/unicode_in_comment.clx", "ok\n");
}

TEST(Comments, UnterminatedComment)
{
    test_failing_cellox_program("comments/unterminated_comment.clx", "[line 2] Error at 'Unterminated comment': Unterminated comment\n");
}