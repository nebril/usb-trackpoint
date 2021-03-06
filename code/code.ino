// Allows control of the mouse cursor on the connected computer, via a
// TrackPoint.
//
// Depends on arduino-trackpoint version 0.1.0:
//
// <https://github.com/feklee/arduino-trackpoint>

// Copyright (C) 2013 Felix E. Klee <felix.klee@inka.de>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

#include "Trackpoint.h"

const byte potiSliderAnalogPin = 0;

const byte decadeCounterClrPin = 4; //4017 CLR pin (reset)
const byte decadeCounterClkPin = 5; //4017 CLK pin
byte decadeCounterPos;

Trackpoint trackpoint(3, // CLK
                      9, // DATA
                      12); // RESET

void clrDecadeCounter() {
  digitalWrite(decadeCounterClrPin, HIGH);
  digitalWrite(decadeCounterClrPin, LOW);
  decadeCounterPos = 0;
}

void incrementDecadeCounter() {
  digitalWrite(decadeCounterClkPin, HIGH);
  digitalWrite(decadeCounterClkPin, LOW);
  decadeCounterPos++;
}

void cycleRgbLed() {
  if (decadeCounterPos >= 3) {
    clrDecadeCounter();
  } else {
    incrementDecadeCounter();
  }
}

void setupTrackpoint() {
  Mouse.begin();

  trackpoint.reset();
  trackpoint.setRemoteMode();

  cycleRgbLed(); // -> green
}

void setup() {
  pinMode(decadeCounterClkPin, OUTPUT);
  pinMode(decadeCounterClrPin, OUTPUT);
  clrDecadeCounter();

  setupTrackpoint();
}

// between 0 and 1
float potiPos() {
  return analogRead(potiSliderAnalogPin) / 1023.;
}

void sendButtonState(byte state) {
  static const byte hidStates[] = {MOUSE_LEFT, MOUSE_RIGHT};

  for (byte i = 0; i < sizeof(hidStates); i++) {
    byte hidState = hidStates[i];
    if (state & (1 << i)) {
      Mouse.press(hidState);
    } else if (Mouse.isPressed(hidState)) {
      Mouse.release(hidState);
    }
  }
}

// Reads TrackPoint data and sends data to computer.
void loop() {
  static float oldPotiPos = -1;
  float newPotiPos = potiPos();

  cycleRgbLed(); // to see if program is still looping, or if it hangs

  Trackpoint::DataReport d = trackpoint.readData();

  if (d.state & (1 << 2)) { // middle button down => scroll
    Mouse.move(0, 0, d.y);
  } else {
    Mouse.move(d.x, -d.y, 0);
    sendButtonState(d.state);
  }

  if (abs(newPotiPos - oldPotiPos) > 0.05) { // ignores small fluctuations
    trackpoint.setSensitivityFactor(0xff * potiPos());
    oldPotiPos = newPotiPos;
  }
}
