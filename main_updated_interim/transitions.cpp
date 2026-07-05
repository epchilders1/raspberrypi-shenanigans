#include "transitions.h"

// ─────────────────────────────────────────────────────────────────────────────
//  1.  PLANE WIPE (Geometric)
//      A massive top-down commercial jet flies LEFT→RIGHT.
//      Leaves a black wake. Once cleared, draws the new screen.
// ─────────────────────────────────────────────────────────────────────────────
void transitionPlane(MatrixPanel_I2S_DMA* d, void (*drawNew)()) {
  const int PANEL_W  = 64;
  const int PANEL_H  = 32;
  const int STEP     = 1;   // Changed from 2 to 1 for smoother, slower movement
  const int FRAME_MS = 20;  // Increased delay to slow the plane down

  uint16_t bodyColor = d->color565(230, 230, 230);
  uint16_t wingColor = d->color565(120, 150, 200);
  uint16_t tailColor = d->color565(200, 40, 40);
  uint16_t engColor  = d->color565(50, 50, 50);

  for (int planeX = -20; planeX <= PANEL_W + 20; planeX += STEP) {

    // Erase the wake behind the plane to black
    if (planeX - 16 > 0) {
      d->drawLine(planeX - 16, 0, planeX - 16, PANEL_H, 0); // Erase column by column
    }

    // Draw the Plane
    // Wings (Swept back, 32px tip-to-tip)
    d->fillTriangle(planeX - 2, 16, planeX + 8, 16, planeX - 12, 0, wingColor);
    d->fillTriangle(planeX - 2, 16, planeX + 8, 16, planeX - 12, 31, wingColor);

    // Engines
    d->fillRect(planeX - 2, 8, 5, 2, engColor);
    d->fillRect(planeX - 2, 22, 5, 2, engColor);

    // Tail Fins
    d->fillTriangle(planeX - 16, 16, planeX - 10, 16, planeX - 18, 6, tailColor);
    d->fillTriangle(planeX - 16, 16, planeX - 10, 16, planeX - 18, 25, tailColor);

    // Fuselage
    d->fillRoundRect(planeX - 18, 14, 28, 5, 2, bodyColor);

    // Cockpit Window
    d->drawPixel(planeX + 7, 15, 0);
    d->drawPixel(planeX + 7, 17, 0);
    d->drawPixel(planeX + 8, 16, 0);

    // Engine glow
    d->drawPixel(planeX - 19, 16, d->color565(255, 100, 0));

    delay(FRAME_MS);
  }

  // Clear final artifacts and render new screen
  d->fillScreen(0);
  drawNew();
}

// // ─────────────────────────────────────────────────────────────────────────────
// //  2.  MOONWALK (Large Pixel Art)
// //      A 11x26 stick figure with a hat glides RIGHT→LEFT.
// // ─────────────────────────────────────────────────────────────────────────────

// // Frame A: Legs spread
// static const uint8_t MWK_A[26][11] PROGMEM = {
//   {0,0,0,0,0,0,0,0,0,0,0}, {0,0,0,1,1,1,1,1,0,0,0}, {0,0,1,1,1,1,1,1,1,0,0},
//   {0,0,0,0,1,1,1,0,0,0,0}, {0,0,0,1,1,1,1,0,0,0,0}, {0,0,0,1,1,1,1,0,0,0,0},
//   {0,0,0,0,0,1,0,0,0,0,0}, {0,0,0,1,1,1,1,0,0,0,0}, {0,0,1,1,1,1,1,1,0,0,0},
//   {0,1,1,1,1,1,1,1,1,1,0}, {1,1,0,1,1,1,1,0,0,1,1}, {1,0,0,0,1,1,0,0,0,0,1},
//   {0,0,0,0,1,1,0,0,0,0,0}, {0,0,0,0,1,1,0,0,0,0,0}, {0,0,0,0,1,1,0,0,0,0,0},
//   {0,0,0,1,1,1,1,0,0,0,0}, {0,0,1,1,0,0,1,1,0,0,0}, {0,1,1,0,0,0,0,1,1,0,0},
//   {1,1,0,0,0,0,0,0,1,1,0}, {1,0,0,0,0,0,0,0,0,1,0}, {1,0,0,0,0,0,0,0,0,1,0},
//   {1,0,0,0,0,0,0,0,0,1,0}, {1,0,0,0,0,0,0,0,0,1,0}, {1,0,0,0,0,0,0,0,0,1,0},
//   {1,1,1,0,0,0,0,0,1,1,1}, {1,1,1,0,0,0,0,0,1,1,1}
// };

// // Frame B: Legs crossed/standing
// static const uint8_t MWK_B[26][11] PROGMEM = {
//   {0,0,0,0,0,0,0,0,0,0,0}, {0,0,0,1,1,1,1,1,0,0,0}, {0,0,1,1,1,1,1,1,1,0,0},
//   {0,0,0,0,1,1,1,0,0,0,0}, {0,0,0,1,1,1,1,0,0,0,0}, {0,0,0,1,1,1,1,0,0,0,0},
//   {0,0,0,0,0,1,0,0,0,0,0}, {0,0,0,1,1,1,1,0,0,0,0}, {0,0,1,1,1,1,1,1,0,0,0},
//   {0,1,1,1,1,1,1,1,1,1,0}, {1,1,0,1,1,1,1,0,0,1,1}, {1,0,0,0,1,1,0,0,0,0,1},
//   {0,0,0,0,1,1,0,0,0,0,0}, {0,0,0,0,1,1,0,0,0,0,0}, {0,0,0,0,1,1,0,0,0,0,0},
//   {0,0,0,1,1,1,1,0,0,0,0}, {0,0,0,1,1,1,1,0,0,0,0}, {0,0,0,1,1,1,1,0,0,0,0},
//   {0,0,0,0,1,1,0,0,0,0,0}, {0,0,0,0,1,1,0,0,0,0,0}, {0,0,0,0,1,1,0,0,0,0,0},
//   {0,0,0,0,1,1,0,0,0,0,0}, {0,0,0,0,1,1,0,0,0,0,0}, {0,0,0,0,1,1,0,0,0,0,0},
//   {0,0,0,1,1,1,1,0,0,0,0}, {0,0,0,1,1,1,1,0,0,0,0}
// };

// void transitionMoonwalk(MatrixPanel_I2S_DMA* d, void (*drawNew)()) {
//   const int MWK_W = 11;
//   const int MWK_H = 26;
//   const int PANEL_W = 64;
//   const int PANEL_H = 32;
//   const int FIG_Y = (PANEL_H - MWK_H) / 2; // Centered
//   const int FRAME_MS = 25;

//   uint16_t skinColor = d->color565(255, 200, 120);
//   uint16_t suitColor = d->color565(200, 200, 220); // Silver suit
//   uint16_t hatColor  = d->color565(20, 20, 20);

//   // Smooth 1-pixel steps
//   for (int figX = PANEL_W; figX >= -MWK_W; figX -= 1) {

//     // Clear his local bounding box to prevent the sprite ghosting itself
//     if (figX >= 0 && figX < PANEL_W) {
//       d->fillRect(figX, FIG_Y, MWK_W + 1, MWK_H, 0);
//     }

//     // Completely wipe the old screen to black behind him (to his right)
//     if (figX + MWK_W < PANEL_W) {
//       d->drawLine(figX + MWK_W + 1, 0, figX + MWK_W + 1, PANEL_H, 0);
//     }

//     // Toggle pose every 6 pixels
//     bool frameA = (( (PANEL_W - figX) / 6) % 2 == 0);

//     for (int row = 0; row < MWK_H; row++) {
//       int sy = FIG_Y + row;
//       for (int col = 0; col < MWK_W; col++) {
//         int sx = figX + col;
//         if (sx < 0 || sx >= PANEL_W) continue;

//         uint8_t px = frameA ? pgm_read_byte(&MWK_A[row][col]) : pgm_read_byte(&MWK_B[row][col]);
//         if (!px) continue;

//         uint16_t c;
//         if (row < 3) c = hatColor;
//         else if (row < 6) c = skinColor;
//         else if (row > 23) c = hatColor; // Shoes
//         else c = suitColor;

//         d->drawPixel(sx, sy, c);
//       }
//     }
//     delay(FRAME_MS);
//   }

//   // Call the new screen function once at the very end to prevent flickering
//   d->fillScreen(0);
//   drawNew();
// }


// ─────────────────────────────────────────────────────────────────────────────
//  3.  STORM FLASH (Classic/Simple)
//      A recognizable classic cloud shape slides in, drops rain, then flashes.
// ─────────────────────────────────────────────────────────────────────────────
void transitionStorm(MatrixPanel_I2S_DMA* d, void (*drawNew)()) {
  uint16_t cloudColor = d->color565(90, 90, 100);
  uint16_t rainColor  = d->color565(70, 130, 255);
  uint16_t flashColor = d->color565(255, 255, 255);

  // 1. Clouds slide in from the right (FULL coverage)
  for (int x = 70; x >= 21; x -= 2) {   // start slightly offscreen for smooth entry
    d->fillRect(0, 0, 64, 12, 0);       // clear full cloud band each frame

    // Cloud 1 (Right)
    d->fillCircle(x, 4, 5, cloudColor);
    d->fillCircle(x - 6, 6, 4, cloudColor);
    d->fillCircle(x + 6, 6, 4, cloudColor);
    d->fillRect(x - 6, 6, 13, 5, cloudColor);

    // Cloud 2 (Left)
    d->fillCircle(x - 16, 2, 4, cloudColor);
    d->fillCircle(x - 21, 4, 3, cloudColor);
    d->fillCircle(x - 11, 4, 3, cloudColor);
    d->fillRect(x - 21, 4, 11, 4, cloudColor);

    delay(20);
  }

  // 2. Rain falls from the clouds (FULL WIDTH + better density)
  for (int frame = 0; frame < 20; frame++) {
    // Increase density + full width
    for (int r = 0; r < 12; r++) {
      int rx = random(0, 64);     // FIX: use full width
      int ry = random(10, 30);    // slightly higher start for overlap
      d->drawLine(rx, ry, rx - 1, ry + 3, rainColor);
    }

    delay(50);

    // Fade instead of hard wipe (looks more like rain)
    d->fillRect(0, 10, 64, 22, 0);
  }

  // 3. Lightning Flash Strobe
  d->fillScreen(flashColor);
  delay(40);
  d->fillScreen(0);
  delay(50);
  d->fillScreen(flashColor);
  delay(20);

  // Reveal new screen
  drawNew();
}