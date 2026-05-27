# Cibertracks Badge — User Guide

## Overview

Your badge is a wearable game console built around a CH32V003 RISC-V microcontroller and an 8×16 RGB LED matrix display. It runs several classic mini-games and lets you set a personal name that appears in the scrolling marquee.

---

## Physical Orientation

Hold the badge **horizontally** (landscape), with the LED matrix facing you. The display is 16 columns wide and 8 rows tall.

```
  ┌─────────────────────────────┐
  │  · · · · · · · · · · · · · │
  │  · · · · · · · · · · · · · │  ← 8×16 LED matrix
  │  · · · · · · · · · · · · · │
  └─────────────────────────────┘
       [MENU]   [A] [B] [C] [D]
```

---

## Buttons

| Button | Description |
|--------|-------------|
| **MENU** | Context-sensitive: short press = action, long press (hold) = exit / back |
| **A** | Up / previous |
| **B** | Left |
| **C** | Right |
| **D** | Down / next |

> **Long press** means holding a button for roughly one second until the action triggers.

---

## Boot Sequence

On power-up the badge shows a **scrolling marquee** with the Cibertracks brand name (and your custom name once you set one). The marquee cycles through rainbow colours.

Press **any button** to enter the game selection menu.

---

## Game Menu

The menu scrolls the name of the currently selected game in the matching game colour.

| Button | Action |
|--------|--------|
| **B** | Previous game |
| **C** | Next game |
| **A / D / MENU** | Launch selected game |

The menu returns to the marquee automatically after **10 seconds** of inactivity.

---

## Games

### Tetris
Pieces fall from the top. Clear complete rows to score.

| Button | Action |
|--------|--------|
| C | Move left |
| D | Move right |
| B | Rotate |
| A | Soft drop (faster fall) |
| MENU | Hard drop (instant place) |
| MENU (long) | Exit to menu |

---

### Pong
Two-player paddle game on a shared badge. Each player controls one paddle.

| Button | Player 1 | Player 2 |
|--------|----------|----------|
| B | ← left | — |
| A | → right | — |
| C | — | ← left |
| D | — | → right |
| MENU (long) | Exit | Exit |

---

### Snake
Guide the snake to eat food without hitting the walls or itself.

| Button | Action |
|--------|--------|
| B | Turn left |
| C | Turn right |
| MENU (long) | Exit to menu |

---

### Flappy Bird
Tap any button to flap and keep the bird airborne between pipes.

| Button | Action |
|--------|--------|
| A / B / C / D | Flap |
| MENU (long) | Exit to menu |

---

### Space Invaders
Shoot down the alien waves before they reach the bottom.

| Button | Action |
|--------|--------|
| C | Move left |
| D | Move right |
| MENU (long) | Exit to menu |

---

### Frogger
Guide the frog across traffic lanes and reach the other side safely.

| Button | Action |
|--------|--------|
| A | Move up |
| D | Move down |
| B | Move left |
| C | Move right |
| MENU (long) | Exit to menu |

---

## Name Editor

Set a 6-character name that appears in the scrolling marquee alongside the Cibertracks brand.

| Button | Action |
|--------|--------|
| D | Next character (A → Z → 0 → 9) |
| A | Previous character |
| C | Move cursor right |
| B | Move cursor left |
| MENU (long) | Save and exit |

**Display:** the current character is shown in white in the centre. Adjacent characters are shown in blue on either side. Six dots at the bottom indicate the cursor position.

**Character set:** uppercase letters A–Z and digits 0–9.

---

## Tips

- Names are saved in memory and persist between games — no need to re-enter.
- The marquee updates with your name immediately after you exit the name editor.
- If the badge freezes or behaves unexpectedly, a quick power cycle resets everything.
