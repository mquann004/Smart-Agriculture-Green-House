#ifndef DEVICE_MANAGER_H
#define DEVICE_MANAGER_H

#include <Arduino.h>
#include "config.h"

// ===== DEVICE MANAGER =====
// Điều khiển ON/OFF cho Quạt, Bơm, Đèn
class DeviceManager {
public:
    static void initialize();

    static void fanOn();
    static void fanOff();
    static void pumpOn();
    static void pumpOff();
    static void lightOn();
    static void lightOff();

    static bool getFanState();
    static bool getPumpState();
    static bool getLightState();

private:
    static bool _fanState;
    static bool _pumpState;
    static bool _lightState;
};

#endif
