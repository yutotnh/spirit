#ifndef SPIRIT_H

#include "include/A3921.h"
#include "include/CANMessage.h"
#include "include/Error.h"
#include "include/FakeUdpConverter.h"
#include "include/Id.h"
#include "include/InterfaceDigitalOut.h"
#include "include/InterfacePwmOut.h"
#include "include/MdLed.h"
#include "include/Motor.h"
#include "include/MotorDataConverter.h"
#include "include/PwmDataConverter.h"
#include "include/SpeedDataConverter.h"
#include "include/adjust_duty_cycle.h"
#include "include/bfloat16.h"
#include "include/bit.h"
#include "include/mutex.h"

#ifdef MBED_H
#include "platform/mbed/include/DigitalOut.h"
#include "platform/mbed/include/PwmOut.h"
#endif  // MBED_H

#endif  //  SPIRIT_H
