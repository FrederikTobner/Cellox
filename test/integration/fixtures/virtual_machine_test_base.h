#pragma once

#include <gtest/gtest.h>

extern "C" {
#include "backend/virtual_machine.h"
}

/// Base for the fixtures of tests that need a running virtual machine. Not meant to be used directly with TEST_F, every
/// test file derives its own fixture that is named after the functionality that is tested.
class VirtualMachineTestBase : public ::testing::Test {
  protected:
    void SetUp() override {
        virtual_machine_init();
    }

    void TearDown() override {
        virtual_machine_free();
    }

    void reset() {
        virtual_machine_free();
        virtual_machine_init();
    }
};
