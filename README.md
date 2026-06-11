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

🚀 Building and Running Nimbly-OS
🐧 Option 1: Running Natively on Linux / Windows Subsystem for Linux (WSL)
If you are inside your project repository folder (~/nimbly-os), compile the kernel and launch it instantly via QEMU using the following setup:
Clean and Compile the Code:
Bash
make clean && make
This removes old artifacts and builds the mykernel.bin binary or output files.
Run via QEMU Emulator:
Bash
qemu-system-i386 -kernel mykernel.bin
(Alternatively, if your Makefile generates a bootable ISO image):
Bash
qemu-system-i386 -cdrom nimblyos.iso
🪟 Option 2: Running Natively on Windows
If you compiled your project inside WSL but want to run or test the final image directly within native Windows, use this workflow:
Method A: Using QEMU for Windows (Recommended)
Download and install QEMU for Windows from the official site.
Add the QEMU path (e.g., C:\Program Files\qemu) to your Windows System Environment Variables.
Open a standard Windows Command Prompt (cmd) or PowerShell, navigate to your build artifacts, and boot the OS:
PowerShell
qemu-system-i386.exe -kernel .\mykernel.bin
or using the ISO file:
PowerShell
qemu-system-i386.exe -cdrom .\nimblyos.iso
Method B: Booting via VirtualBox / VMware
Copy the generated nimblyos.iso from your build directory onto your Windows filesystem.
Open VirtualBox or VMware Player.
Create a new VM:
Type: Other
Version: Other/Unknown (32-bit)
Memory: 32 MB or 64 MB is more than enough.
Mount nimblyos.iso into the virtual optical IDE drive.
Click Start to watch your kernel boot!
📂 Repository Structure
kernel.c — Main kernel execution logic, command interpreter, string parsers, and VGA hardware address mapping.
kernel.s / boot.s — Assembly bootstrap routine initializing GDT/stack and calling the C entry point.
makefile — Standard compilation automation scripts targeting -m32 cross-compilation architectures.
🎮 Built-in Commands
Type these commands inside the shell:
help — Prints the diagnostic help menu.
clear — Wipes the VGA display framebuffers clean.
guess — Launches the CoreGuess numerical guessing mini-game.
version — Displays current environment build metrics.
Proudly maintained by Ferdo Studios.
