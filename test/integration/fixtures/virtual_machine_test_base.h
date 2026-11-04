#pragma once

#include <gtest/gtest.h>

extern "C" {
#include "backend/virtual_machine.h"
}

/// Base for the fixtures of tests that need a running virtual machine.
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
