# RAM-Runner
[ RAM-RUNNER - Retro Cyberpunk Arcade for DOS ]

<p>
  <i>My second game with 64 bit code and multi-core for DOS, and probably one of the world's first ones too.</i>
</p>
<p align="center">
  <img src="https://img.shields.io/badge/Platform-DOS%2032%2F64--bit-blue.svg" alt="Platform DOS">
  <img src="https://img.shields.io/badge/Architecture-Pentium%2B%20%2F%20XMS%203.0-green.svg" alt="Architecture">
  <img src="https://img.shields.io/badge/Audio-AdLib%20FM%20%2F%20HDA-orange.svg" alt="Audio AdLib HDA">
  <img src="https://img.shields.io/badge/Language-C%20%2F%20ASM-yellow.svg" alt="Language C++ ASM">
</p>

---

Welcome to the official repository of **RAM-Runner**, a fast-paced arcade game (inspired by classic maze chasers, drenched in silicon dust, corrupted code, and DOS nostalgia) available in both **32-bit** and **64-bit Asymmetric Multiprocessing** editions.

Playing as a digital forensics analyst, your mission is to scrub the RAM clean of file fragments, disturbance scripts, and a ruthless gang of rogue hackers aiming to throw your system into a Kernel Panic. Each actor has its own customized AI.

#### 🕹️ Game Mechanics
* **Inspecting Devices:** Forget traditional "levels". You progress by systematically inspecting and cleaning different compromised storage devices.
* **Recovered Bytes:** Your score translates directly into saved data; every file fragment and recovered byte increases your data pool.
* **Remaining Backups:** Your extra lives. If a rogue hacker catches you or a crash occurs, a system backup restores your session.
* **System Power-ups:** Terminal shells, smart cards, PGP keys, and SSD units that turn the tables on your pursuers (freezing actors, granting temporary invincibility, or region-locking corridors).

#### 🌟 Key Features
* **Dual Engine (32-bit & dual core 64-bit):** Built to leverage VBE/LFB graphics modes (1024x768 @ 32 bpp) and harness the enormous computing power of modern CPUs with multiple cores and SSE2 and AVX vector instructions.
* **XMS Memory Management (64-bit build):** Utilizes XMS 3.0 to allocate and lock extended memory blocks (up to 12 MB) via `2Fh` interrupt hooking with robust fallback mechanisms, avoiding compromising the protected Ring-3 32 bit protected mode under HDPMI 32 DOS Extender.
* **Layered Behavioral AI:** Custom thought processes and routines for every rogue actor on the grid (from the *Loud Basement Streamer* to the existential *Coffee Shop Game Dev*).
* **Retro Audio Integration:** Two-channel FM synthesis driven by the AdLib audio card or Sound Blaster, alongside classic PC Speaker support.
* **Compatible with SBEMU and VSBHDA**
* **Requires HDPMI 32 bit DOS Extender**

---

### 🚀 System Requirements & Running

* **CPU:** Intel Pentium or better for the 32 bit, Intel Core 2 Duo or better for the 64 bit.
* **Video:** Graphic Card VBE 2.0+ with LFB (1024 x 768 @ 32 bpp); S3 Virge or better.
* **Audio:** AdLib, Sound Blaster or emulation (es. HDA in QEMU with VSBHDA).
* **RAM:** At least 16 MB of free RAM.

#### 🧪 DOSBOX Staging (32-bit only)
The 32 bit application is fully compatible with DOSBOX Staging.

#### 🧪 QEMU (32-bit and 64-bit)
The 32 bit application can be executed by the following QEMU emulator; I tested with MS-DOS 7.1.
```bash
qemu-system-i386.exe -m 64 -cpu pentium -audiodev dsound,id=snd0 -device ich9-intel-hda -device hda-duplex,audiodev=snd0 -rtc base=localtime -boot c -drive file=C:\QDOS\DOS.qcow2,format=qcow2
```

The 64 bit application can be executed by the following QEMU emulator; I tested with MS-DOS 7.1. The Intel Core 2 Duo processor supports SSE and XMM registers but not AVX and YMM registers; you may try more advanced CPUs or use hardware accelerated virtualization (WHPX).
```bash
qemu-system-x86_64.exe -cpu core2duo -smp 2,cores=2 -audiodev dsound,id=snd0 -device ich9-intel-hda -device hda-duplex,audiodev=snd0 -rtc base=localtime -boot c -drive file=C:\QDOS\DOS.qcow2,format=qcow2
```

### 📜 Instructions
Many people believe that a game for the MS-DOS operating system cannot take advantage of multitasking or modern operating modes, such as protected mode and long mode; however, despite the system's significant limitations, this is not entirely accurate. In reality, there are no physical or technical barriers preventing an MS-DOS game from utilizing long mode or activating multiple processor cores: the constraints are purely a matter of software engineering.
After seeing White Ambit’s DOS64SMP project (https://github.com/TheWhiteAmbit/dos64smp), I decided to modify a game of mine from 2005—a Pac-Man clone that includes the option to activate a fifth ghost—so that it would use a similar technique, allowing it to run at incredible speeds at a resolution of 1024 x 768 pixels; however, I can not publish it because I have not the rights to the game.
In my game, I employ state-of-the-art management of DPMI resources and IRQ ISR, sophisticated handling of advanced vector instructions—much like what is used in modern operating systems—and an efficient data organization, all contained within a single archive called RR.DAT, which features a structure similar to Ken Silverman’s well-known GRP files. Another difficulty stems from the inability to use library functions: I wrote all the code myself, including the code for converting numbers into text.
Core 0 is dedicated to game logic, music and sound, and the graphics for splash screens, while Core 1 is dedicated to the rapid rendering of game frames. The splash screens are handled by Core 0 because, at a resolution of 1024 x 768 pixels with 4 bytes per pixel, the images are so large that they require numerous calls to DOS services in order to load the graphical assets.

---

To build 32-bit RR
1. Run the script Build.cmd, that compiles all source files with MASM and Visual C++.
2. Copy all the .OBJ files in a drive mounted in DosBox, QEMU, or a true DOS machine; JWLINKD e HDPMI32 are required.

```bash
JWLINKD.EXE @RR.DEF
```

---

To build 64-bit RR
1. Run the script Build.cmd, that compiles all source files with MASM and Visual C++.

2. Copy all the .OBJ files in a drive mounted in DosBox, QEMU, or a true DOS machine; JWLINKD e HDPMI32 are required.

```bash
JWLINKD.EXE @RR64.DEF
JWASMD.EXE -coff -c RR64.ASM
JWLINKD.EXE @RR.DEF
```

3. To build the START.EXE program, you need Turbo Assembler 5.0; the testing program requires Turbo Pascal 7.0.

```bash
TASM.EXE START.ASM
TLINK.EXE /x START.OBJ
TPC.EXE RR.PAS
```

---


⚙️ Architecture Deep Dive: Asymmetric Multiprocessing (AMP) under DOS

Running 64-bit multi-core code under MS-DOS requires overcoming severe limitations in how legacy memory managers (XMS) and DOS extenders (DPMI) handle physical memory. Here is how RAM-Runner bridges the gap between 32-bit Protected Mode and 64-bit Long Mode:

1. Physical Memory Allocation (START.ASM)

Standard DPMI 0.9 services do not provide a reliable way to retrieve the physical address of large memory allocations, and Virtual DMA Services (VDS) are unsuitable for a 12 MB buffer. To solve this:

• A lightweight, pre-launch executable (START.EXE, written in assembly) allocates a 12 MB XMS block and locks it using XMS functions 8Ch or 0Ch to obtain its true 32-bit physical base address.

• It then installs a custom Multiplex Interrupt (INT 2Fh) handler exposing two services to the main executable: an installation check and a function returning the physical address of the locked XMS block.

2. Shared Memory Mapping & Isolation

Once the main 32-bit engine starts under HDPMI32:

• Core 0 (BSP) queries INT 2Fh and maps the physical XMS block into its 32-bit linear address space via DPMI service 0800h (Physical Address Mapping).

• Core 1 (AP) operates using Identity Mapping in 64-bit Long Mode, allowing both cores to share the exact same physical memory with zero-copy overhead.

• Protecting the 64-bit Payload: During early prototypes, leaving 64-bit code in conventional memory led to random crashes, as HDPMI32 occasionally overwrote those regions (likely using them as bounce buffers for 16-bit DOS/BIOS calls). To ensure complete isolation, Core 0 copies the entire 64-bit kernel—including Page Tables, GDT, IDT, minimal ISRs, and the 64-bit executable code itself—directly inside the locked XMS block.

3. Core 1 Bootstrapping & Fault Handling

• Core 0 initializes a shared WorkArea structure containing all necessary pointers and sets up minimal Interrupt Service Routines (ISRs) for Core 1. If a hardware exception occurs on the 64-bit core, its ISR safely sets flag, checked by the function "bool HasCrashed", allowing Core 0 to detect the fault and exit gracefully instead of freezing the machine.

• The AP bootstrap trampoline wakes Core 1, transitions it from Real Mode through Protected Mode into 64-bit Long Mode, and passes the physical pointer to the WorkArea directly in the RCX register (following the standard x64 calling convention).

4. Lock-Free SPSC Command Queue

Communication between the two cores relies on a Single-Producer, Single-Consumer (SPSC) ring buffer inside the shared XMS area:

• Core 0 (Core32.cpp) pushes rendering commands into the queue and executes an SFENCE instruction before updating the tail pointer. This guarantees all memory writes (including non-temporal SSE2/AVX stores) are globally visible before Core 1 sees the new command.

• Core 1 (Core64.cpp) polls and pops commands without requiring locking instructions or memory barriers, leveraging the strict Total Store Order (TSO) guarantees of x86/x64 processors.
