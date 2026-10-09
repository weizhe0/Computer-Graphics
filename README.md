# Minecraft Blind Box Battle

A local two-player battle game built with **C++, OpenGL, and GLUT** for the TCG6223 Computer Graphics course. The project explores 3D rendering, character movement, combat, and interactive gameplay in a Minecraft-inspired arena.

## Features

- Local two-player battles on one computer.
- Minecraft-inspired characters: Creeper, Enderman, and Blaze.
- Character skills and combat effects.
- Health and skill cooldown displays.
- Restart a match after the battle ends.

## Tech stack

| Component | Technology |
| --- | --- |
| Programming language | C++ |
| Graphics | OpenGL and GLU |
| Window and input handling | GLUT |
| IDE | Code::Blocks |
| Compiler | GCC / MinGW |
| Setup platform | Windows |

## Requirements

- Code::Blocks with a GCC / MinGW compiler.
- The included C++ source files and Code::Blocks project file.
- OpenGL / GLUT headers, import libraries, and runtime DLLs from the supplied dependency package.

The course lab uses **Code::Blocks 17.12 with MinGW**. If your existing installation builds the project successfully, you can continue using it. For another compiler version, use compatible GLUT libraries.

**The compiler target, import libraries, and runtime DLLs must match: all 32-bit or all 64-bit.**

## Windows setup

### 1. Download the project

On GitHub, select **Code → Download ZIP**, then extract the complete ZIP into a folder on your computer. Keep the supplied dependency files and folders together with the project.

### 2. Install Code::Blocks and MinGW

Download an installer containing **`mingw-setup.exe`** from the [official Code::Blocks downloads page](https://www.codeblocks.org/downloads/binaries/). This installer includes a compiler.

If Code::Blocks and MinGW are already installed, skip this step.

### 3. Check the compiler

In Code::Blocks:

1. Open **Settings → Compiler**.
2. Select **GNU GCC Compiler**.
3. Open **Toolchain executables** and select **Auto-detect**.
4. Confirm that the compiler directory points to your actual MinGW installation.

Example only—your installation path may differ:

```text
C:\Program Files\CodeBlocks\MinGW
```

### 4. Configure the supplied OpenGL / GLUT files

Use the dependency files included with the project. If the supplied package is still a ZIP, extract it first.

| File type | Files |
| --- | --- |
| Headers | `gl.h`, `glu.h`, `glut.h` |
| Import libraries | `libopengl32.a`, `libglu32.a`, `libglut32.a` |
| GLUT runtime | `glut32.dll` |

For the lab's matching MinGW package:

- Put missing headers in **`MinGW\include\GL`**.
- Put compatible import libraries in **`MinGW\lib`**.
- Keep existing working compiler headers and libraries rather than replacing them unnecessarily.

Alternatively, keep dependencies inside the project and configure **Build options → Search directories**: the compiler search path should point to the parent folder containing `GL`, and the linker search path should point to the folder containing the `.a` files. Use your supplied folders' actual names.

**Do not copy or replace DLLs in `C:\Windows\System32`.** Windows normally supplies OpenGL and GLU. Place the matching `glut32.dll` beside the executable instead.

If the project includes local `opengl32.dll` and `glu32.dll` for the lab's software renderer, keep that tested runtime arrangement until you have confirmed the game works without them.

### 5. Open the existing project

Select **File → Open** and open:

```text
Minecraft_Blind_Box_Battle.cbp
```

Use this existing project rather than creating a new one. Its source files should include:

```text
CGLabmain.cpp
CGLabmain.hpp
CGLabMinecraft.cpp
CGLabMinecraft.hpp
```

Windows may hide extensions, so the `.cbp` file can appear as a Code::Blocks project file.

### 6. Configure linker libraries

Right-click the project and select **Build options**. Select the project name on the left to apply shared settings to Debug and Release.

Under **Linker settings → Link libraries**, add these if they are not already present:

```text
glut32
glu32
opengl32
```

Under **Search directories → Linker**, ensure Code::Blocks can find the folder containing the matching import libraries.

### 7. Build and run

Press **F9** to build and run the game.

Find the executable location under **Project → Properties → Build targets → Output filename**. Depending on the project settings, it may be in the project root or a folder such as `bin\Debug`.

Copy the matching **`glut32.dll` into that executable's folder**. Keep any required assets in the locations expected by the project.

## Run the included executable

If a prebuilt `Minecraft_Blind_Box_Battle.exe` is included, you can try opening it directly without compiling. Keep its runtime DLLs beside it and extract the complete project before running it.

To modify the code or rebuild the game, follow the setup steps above.

## Controls

| Action | Player 1 | Player 2 |
| --- | --- | --- |
| Move | W, A, S, D | Arrow keys |
| Skill 1 | G | 8 |
| Skill 2 | H | 9 |
| Restart match | Enter | Enter |

These controls follow the project report; check the input handlers in the source if your game version uses different keys.

## Main project files

| File | Purpose |
| --- | --- |
| `Minecraft_Blind_Box_Battle.cbp` | Code::Blocks project and build settings |
| `CGLabmain.cpp` / `CGLabmain.hpp` | Main application source and declarations |
| `CGLabMinecraft.cpp` / `CGLabMinecraft.hpp` | Game source and declarations |
| `glut32.dll` | GLUT runtime dependency |
| Supplied dependency folders | Headers and import libraries used to build the game |
| `README.md` | Project overview and setup instructions |

## Troubleshooting

| Problem | What to check |
| --- | --- |
| Compiler not found | Check **Settings → Compiler → Toolchain executables** and your MinGW installation. |
| `GL/glut.h: No such file or directory` | Check the headers and compiler search directory. The search directory must contain the `GL` folder. |
| `cannot find -lglut32` | Check that `libglut32.a` exists in the linker search directory. |
| Undefined references to OpenGL / GLUT functions | Check linker libraries, library order, and architecture compatibility. |
| `glut32.dll` missing | Copy the matching DLL beside the executable actually being launched. |
| `0xc000007b` or incompatible file format | Check for mixed 32-bit and 64-bit binaries or incompatible dependency versions. |
| Missing assets or blank output | Check asset paths, working directory, graphics drivers, and the runtime DLL arrangement. |
| Multiple definitions of `main` | Remove any automatically created extra `main.cpp`; use the existing project source files. |

After changing compiler or library settings, select **Build → Clean**, then **Build → Rebuild**.

## Project purpose

This project was developed to practise computer graphics through a playable application, combining 3D rendering, keyboard input, game logic, and visual feedback.

## Credits

Developed for **TCG6223 Computer Graphics** by:

- Tam Yu Siong
- Elysa Lee Xing Wan
- Tai Wei Zhe
- Chong Jia Ming

This is an educational project inspired by Minecraft and is not affiliated with Mojang or Microsoft.

## Setup references

- Course handout: **LAB01.pdf**.
- [Code::Blocks downloads](https://www.codeblocks.org/downloads/binaries/).
- [Microsoft: Dynamic-link library search order](https://learn.microsoft.com/en-us/windows/win32/dlls/dynamic-link-library-search-order).
