#include <gtest/gtest.h>

#include "spirit.h"

int other_translation_unit_function();

/// spirit.h を複数の翻訳単位から include してもリンクできることの確認
TEST(MultipleTranslationUnits, LinkTest)
{
    spirit::Motor::State next_state      = spirit::Motor::State::Coast;
    float                next_duty_cycle = 0.0F;
    spirit::adjust_duty_cycle(spirit::Motor::State::CW, 1.0F, 0.5F, 0.5F, spirit::Motor::State::CW, 0.0F, next_state,
                              next_duty_cycle);
    EXPECT_FLOAT_EQ(next_duty_cycle, 0.5F);
    EXPECT_EQ(other_translation_unit_function(), 1);
}
