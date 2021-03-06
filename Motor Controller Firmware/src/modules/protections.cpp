/**
 * Motor Controller Firmware
 * Copyright (c) 2020 Applied Engineering
 * 
 * Based on software from various contributors.
 * Copyright (c) 2020 Andrew Berkun / Alex Liu / William Zhou
 * 
 */

/**
 * protections.cpp
 * 
 * Checks limits to protect the controller.
 * 
 */

#include "protections.h"

bool mddActive, ocpActive, ovpActive;

void checkProtections() {
  #if defined(MINIMUM_DUTY_DETECTION)
    // Keep load current above 0 amps.
    if (iSenseVADC < zeroISenseVADC) {
      duty++;
      mddActive = true;
    } else {
      mddActive = false;
    }
  #endif

  #if defined(OVERCURRENT_PROTECTION)
    // Keep load current under 40 amps.
    if (iSenseVADC > 454) {
      duty -= 2;
      ocpActive = true;
    } else {
      ocpActive = false;
    }
  #endif

  #if defined(OVERVOLTAGE_PREVENTION)
    // Keep input voltage below 37 volts (37 * 1024/44.8 = 845).
    // Protects against regen on solar.
    if (vSenseADC > 845) {
      duty += 2;
      ovpActive = true;
    } else {
      ovpActive = false;
    }
  #endif
}