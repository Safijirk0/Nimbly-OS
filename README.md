# Nimbly-OS 🚀

A lightweight, standalone hobby operating system built from scratch, utilizing multi-mode development and low-level system programming. Developed under **Ferdo Studios**, Nimbly-OS features a fully operational text-mode console alongside custom mini-games like *CoreGuess*. The system is currently running in retro **VGA Mode 13h (320x200, 256 colors)** with active development scaling up to VESA high-resolution modes (800x600).

---

## 🌟 Features
- **Monolithic Real-Mode/Protected-Mode Kernel**: Engineered for x86 architectures with custom hardware initialization.
- **Interactive Command Line**: A built-in custom command interpreter parsing hardware-level keyboard interrupts scan-codes.
- **Custom Shell Apps**: Includes built-in games like **CoreGuess** ("Ferdo Studios approves your luck!").
- **Hybrid Video Architecture**: Supports native VGA Text-Mode (`0xB8000`) and low-level Mode 13h (`0xA0000`) graphic rendering pipelines.

---

## 🛠️ Prerequisites & Toolchain

To compile and build the ISO, your environment must have the following tools installed:

### On Linux (Ubuntu/Debian) or WSL:
```bash
sudo apt update
sudo apt install build-essential gcc-multilib genisoimage qemu-system-x86 git
