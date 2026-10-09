Blind Boxes Collection - Minecraft Battle

Source files to add in Code::Blocks:
1. CGLabmain.cpp
2. CGLabMinecraft.cpp

Required headers in the same project folder:
1. CGLabmain.hpp
2. CGLabMinecraft.hpp

Link libraries:
opengl32
glu32
glut32

Command-line compile example:
g++ CGLabmain.cpp CGLabMinecraft.cpp -o Minecraft_Blind_Box_Battle.exe -lopengl32 -lglu32 -lglut32

Controls:
Menu        - UP/DOWN choose row, LEFT/RIGHT change character, ENTER confirm
Characters  - Creeper, Enderman, Blaze
P1          - W/A/S/D move, F primary skill, G secondary skill
P2          - Arrow keys move, Space or O primary skill, P secondary skill
R           - Reset battle
F1          - Toggle fill / wireframe
F2          - Toggle axis
F3          - Toggle lighting
Home        - Restore camera/world defaults
Esc         - Return to menu during battle; quit from menu

Project features matched to the guideline:
1. Character selection screen for P1 and P2: Creeper, Enderman, or Blaze.
2. Creeper skill set: explosion ring and powder cloud damage.
3. Enderman skill set: teleport slash and void beam.
4. Blaze skill set: fire burst and fireball with flame particles.
5. Battle environment changes color and lighting during skills.
6. Procedural texture mapping is generated at runtime; no downloaded model files are used.
7. Characters are built from code using transformations, custom cube faces, materials, lighting, and animation.
