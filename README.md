# C1B3RTR4CKS Badge 2026

A collection of practical and educational examples for the **C1b3rtr4cks 2026 Badge**, written in **C** and built using **PlatformIO**.
<p align="center">
  <img width="34%" alt="badge_1" src="https://github.com/user-attachments/assets/185068e2-7d47-478f-b1b8-4d3a16dc023b" />
  &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
  <img width="35%" alt="badge_2" src="https://github.com/user-attachments/assets/8962c77f-dd72-4df4-93ad-628dc1487567" />
</p>

This repository is designed to help you quickly understand, test, and extend the capabilities of your badge hardware. Each example focuses on a specific feature (GPIO, LEDs, buttons, display, communication, etc.) and follows a clean, minimal structure to make learning and experimentation easy.

---

## 📦 Project Overview

This project contains:

- Multiple self-contained example applications
- Clean and readable C code
- PlatformIO-based build configuration
- Hardware-focused demonstrations
- Simple structure for easy customization

The goal is to provide:

- A starting point for badge firmware development
- Reference implementations for common peripherals
- Clear examples for workshops, hackathons, or personal learning

---

## 🛠 Requirements

Before getting started, make sure you have:

- [PlatformIO](https://platformio.org/) (CLI or VSCode extension)
- A supported PCB badge
- USB cable for flashing
- Basic knowledge of C programming

Optional:

- Serial monitor (PlatformIO built-in or external)
- Soldered peripherals (if required by specific examples)

---

## 🚀Getting Started

### 1. Clone the Repository

```
git clone https://github.com/uamseif/badge-2026.git  
cd badge-2026
```

### 2. Open with PlatformIO

If using VSCode:

- Open the project folder
- PlatformIO will automatically detect the configuration

### 3. Select an Example

Each example is typically organized as:

```
/examples  
/board_tester_noneos-sdk
```

You can:

- Change the `src/` folder contents
- Or modify `platformio.ini` to point to a specific example
- Or use multiple environments inside `platformio.ini`

### 4. Build the Project

```
pio run
```

### 5. Upload to the Badge

```
pio run --target upload
```

### 6. Open Serial Monitor (Optional)

```
pio device monitor
```


