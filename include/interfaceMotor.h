#ifndef SPIRIT_INTERFACE_MOTOR_H
#define SPIRIT_INTERFACE_MOTOR_H

namespace spirit {

enum class State {
    Free = 0,
    Coast = Free,
    CW,   // Clock Wise
    CCW,  // Counter Clock Wise
    Brake,
};

/** A Motor is abstract base class for moving the motor
 */
class interfaceMotor {
public:
    virtual void  duty_cycle(const float value)     = 0;
    virtual float duty_cycle() const                = 0;
    virtual void  state(const State type)           = 0;
    virtual State state() const                     = 0;
    virtual void  pulse_period(const float seconds) = 0;
    virtual float pulse_period() const              = 0;
    virtual void  frequency(const float hz)         = 0;

    virtual interfaceMotor &operator=(const float value)  = 0;
    virtual interfaceMotor &operator+=(const float value) = 0;
    virtual interfaceMotor &operator-=(const float value) = 0;
    virtual                 operator float() const        = 0;
    virtual interfaceMotor &operator=(const State type)   = 0;
    virtual                 operator State() const        = 0;

    static constexpr State default_state        = State::Brake;
    static constexpr float default_pulse_period = 0.000'02F;  // 5.0kHz
    static constexpr float default_frequency    = 1.0F / default_pulse_period;
    static constexpr float min_pulse_period     = 1.0F / 60'000.0F;
    static constexpr float max_pulse_period     = 1.0F / 10.0F;
};

}  // namespace spirit

#endif  // SPIRIT_INTERFACE_MOTOR_H
