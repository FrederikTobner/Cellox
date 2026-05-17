#pragma once

#include <gtest/gtest.h>

extern "C" {
#include "backend/virtual_machine.h"
}

class VirtualMachineFixture : public ::testing::Test {
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
