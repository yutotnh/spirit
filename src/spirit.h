#ifndef SPIRIT_H
#define SPIRIT_H

#include "spirit/include/A3921.h"
#include "spirit/include/CANMessage.h"
#include "spirit/include/Error.h"
#include "spirit/include/FakeUdpConverter.h"
#include "spirit/include/Id.h"
#include "spirit/include/InterfaceDigitalOut.h"
#include "spirit/include/InterfacePwmOut.h"
#include "spirit/include/MdLed.h"
#include "spirit/include/Motor.h"
#include "spirit/include/MotorDataConverter.h"
#include "spirit/include/PwmDataConverter.h"
#include "spirit/include/SpeedDataConverter.h"
#include "spirit/include/adjust_duty_cycle.h"
#include "spirit/include/bfloat16.h"
#include "spirit/include/bit.h"
#include "spirit/include/mutex.h"

#ifdef __MBED__
#include "platform/mbed/include/DigitalOut.h"
#include "platform/mbed/include/PwmOut.h"
#endif  // MBED_H

#endif  //  SPIRIT_H
