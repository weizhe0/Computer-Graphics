/*
    TCG6223 Computer Graphics Group Project
    Blind Boxes Collection - Minecraft Battle

    This header declares the virtual world used by CGLabmain.cpp.
*/

#ifndef YP_CGLABMINECRAFT_HPP
#define YP_CGLABMINECRAFT_HPP

namespace CGLabMinecraft
{
class MyVirtualWorld
{
public:
    MyVirtualWorld();
    ~MyVirtualWorld();

    void init();
    void draw();
    void tickTime();

    bool handleKeyboard(unsigned char key);
    bool handleSpecial(int key);
    void setPlayerCharacters(int playerOneCharacter, int playerTwoCharacter);

    void rotateView(float xinc, float yinc, float zinc);
    void resetView();

private:
    struct WorldData;
    WorldData* mData;

    MyVirtualWorld(const MyVirtualWorld&);
    MyVirtualWorld& operator=(const MyVirtualWorld&);
};
}

#endif
