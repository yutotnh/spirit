#include <gtest/gtest.h>

#include "SpeedController.h"
#include "tests/stubs.h"

namespace {

using namespace spirit;

TEST(SpeedController, SpeedControllerTest)
{
    StubInterruptIn a_phase;
    StubInterruptIn b_phase;
    SpeedController speed_controller(a_phase, b_phase);

    speed_controller.pid_gain(1.00f, 1.00f, 1.00f);
    speed_controller.limit(5.00f, 0.00f);

    a_phase.interrupt_rise();
    b_phase.interrupt_rise();
    a_phase.interrupt_fall();
    b_phase.interrupt_fall();

    EXPECT_FLOAT_EQ(speed_controller.angle(), 1.80f);
    EXPECT_FLOAT_EQ(speed_controller.rps(1.00f), 1.80f);
    EXPECT_FLOAT_EQ(speed_controller.calculation(1.80f, 1.00f), 4.50f);

    speed_controller.limit(1.00f, 0.00f);

    EXPECT_FLOAT_EQ(speed_controller.calculation(1.80f, 1.00f), 1.00f);

    speed_controller.reset();

    b_phase.interrupt_fall();
    a_phase.interrupt_fall();
    b_phase.interrupt_rise();
    a_phase.interrupt_rise();

    EXPECT_FLOAT_EQ(speed_controller.angle(), -1.80f);
    EXPECT_FLOAT_EQ(speed_controller.rps(1.00f), -1.80f);
    EXPECT_FLOAT_EQ(speed_controller.calculation(1.80f, 1.00f), 0.00f);
}

}  // namespace
