// ═════════════════════════════════════════════════════════════════════════════
//  transitions.h  –  Cool wipe animations for the 64×32 HUB75 matrix
// ═════════════════════════════════════════════════════════════════════════════
#pragma once
#include <Arduino.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

// ─────────────────────────────────────────────────────────────────────────────
//  1.  PLANE WIPE (Geometric)
//      A massive top-down commercial jet flies LEFT→RIGHT.
//      Leaves a black wake. Once cleared, draws the new screen.
// ─────────────────────────────────────────────────────────────────────────────
void transitionPlane(MatrixPanel_I2S_DMA* d, void (*drawNew)());

// // ─────────────────────────────────────────────────────────────────────────────
// //  2.  MOONWALK (Large Pixel Art)
// //      A 11x26 stick figure with a hat glides RIGHT→LEFT.
// // ─────────────────────────────────────────────────────────────────────────────
// void transitionMoonwalk(MatrixPanel_I2S_DMA* d, void (*drawNew)());

// ─────────────────────────────────────────────────────────────────────────────
//  3.  STORM FLASH (Classic/Simple)
//      A recognizable classic cloud shape slides in, drops rain, then flashes.
// ─────────────────────────────────────────────────────────────────────────────
void transitionStorm(MatrixPanel_I2S_DMA* d, void (*drawNew)());

// ─────────────────────────────────────────────────────────────────────────────
//  Convenience wrappers
// ─────────────────────────────────────────────────────────────────────────────
extern void displayWeather();
extern void displayPlane();
extern void displayClock();

inline void transitionToWeather(MatrixPanel_I2S_DMA* d) { transitionStorm(d, displayWeather); }
inline void transitionToPlane(MatrixPanel_I2S_DMA* d)   { transitionPlane(d, displayPlane); }
//inline void transitionToMoonwalk(MatrixPanel_I2S_DMA* d, void (*drawNew)()) { transitionMoonwalk(d, drawNew); }