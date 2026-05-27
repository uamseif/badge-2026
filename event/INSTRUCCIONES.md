# Badge Cibertracks — Guía de usuario

## Descripción general

Tu badge es una consola de juegos portátil construida sobre un microcontrolador CH32V003 RISC-V y una matriz de LEDs RGB de 8×16. Incluye varios minijuegos clásicos y permite configurar un nombre personalizado que aparece en la marquesina animada.

---

## Orientación física

Sostén el badge **en horizontal** (apaisado), con la matriz de LEDs mirando hacia ti. La pantalla tiene 16 columnas de ancho y 8 filas de alto.

```
                  [MENU]
  [A]┌─────────────────────────────┐[C]
     │  · · · · · · · · · · · · ·  │
     │  · · · · · · · · · · · · ·  │  ← Matriz de LEDs 8×16
     │  · · · · · · · · · · · · ·  │
  [B]└─────────────────────────────┘[D]
```

---

## Botones

| Botón | Descripción |
|-------|-------------|
| **MENU** | Dependiente del contexto: pulsación corta = acción, pulsación larga (mantener) = salir / volver |
| **A** | Arriba / anterior |
| **B** | Izquierda |
| **C** | Derecha |
| **D** | Abajo / siguiente |

> **Pulsación larga** significa mantener el botón pulsado aproximadamente un segundo hasta que la acción se active.

---

## Menú de juegos

El menú desplaza el nombre del juego seleccionado en el color correspondiente a ese juego.

| Botón | Acción |
|-------|--------|
| **B** | Juego anterior |
| **C** | Juego siguiente |
| **A / D / MENU** | Iniciar el juego seleccionado |

El menú vuelve a la marquesina automáticamente tras **10 segundos** sin actividad.

---

## Juegos

### Tetris
Las piezas caen desde arriba. Completa filas horizontales para puntuar.

| Botón | Acción |
|-------|--------|
| C | Mover a la izquierda |
| D | Mover a la derecha |
| B | Rotar pieza |
| A | Caída suave (más rápido) |
| MENU | Caída instantánea |
| MENU (largo) | Salir al menú |

---

### Pong
Juego de paletas para dos jugadores en el mismo badge.

| Botón | Jugador 1 | Jugador 2 |
|-------|-----------|-----------|
| B | ← izquierda | — |
| A | → derecha | — |
| C | — | ← izquierda |
| D | — | → derecha |
| MENU (largo) | Salir | Salir |

---

### Snake
Guía la serpiente para comer sin chocar con las paredes ni contigo mismo.

| Botón | Acción |
|-------|--------|
| B | Girar a la izquierda |
| C | Girar a la derecha |
| MENU (largo) | Salir al menú |

---

### Flappy Bird
Pulsa cualquier botón para aletear y mantener al pájaro entre las tuberías.

| Botón | Acción |
|-------|--------|
| A / B / C / D | Aletear |
| MENU (largo) | Salir al menú |

---

### Space Invaders
Elimina las oleadas de alienígenas antes de que lleguen al fondo.

| Botón | Acción |
|-------|--------|
| C | Mover a la izquierda |
| D | Mover a la derecha |
| MENU (largo) | Salir al menú |

---

### Frogger
Guía a la rana cruzando los carriles de tráfico hasta llegar al otro lado.

| Botón | Acción |
|-------|--------|
| A | Mover arriba |
| D | Mover abajo |
| B | Mover a la izquierda |
| C | Mover a la derecha |
| MENU (largo) | Salir al menú |

---

### Tres en raya
Juego para dos jugadores en el mismo badge. Los jugadores alternan turnos colocando sus fichas.

- El jugador 1 juega con **X** (rojo) — el cursor parpadea en rojo en su turno.
- El jugador 2 juega con **O** (azul) — el cursor parpadea en azul en su turno.

| Botón | Acción |
|-------|--------|
| A | Mover cursor arriba |
| D | Mover cursor abajo |
| B | Mover cursor a la izquierda |
| C | Mover cursor a la derecha |
| MENU | Colocar ficha |
| MENU (largo) | Salir al menú |

Al terminar la partida, pulsa cualquier botón para volver a jugar.

---

## Editor de nombre

Configura un nombre de hasta 6 caracteres que aparecerá en la marquesina junto al nombre Cibertracks.

| Botón | Acción |
|-------|--------|
| D | Carácter siguiente (A → Z → 0 → 9) |
| A | Carácter anterior |
| C | Mover cursor a la derecha |
| B | Mover cursor a la izquierda |
| MENU (largo) | Guardar y salir |

**Pantalla:** el carácter actual se muestra en blanco en el centro. Los caracteres adyacentes aparecen en azul a cada lado. Seis puntos en la fila inferior indican la posición del cursor.

**Caracteres disponibles:** letras mayúsculas A–Z y dígitos 0–9.

---
