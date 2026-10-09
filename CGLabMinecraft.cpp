/*
    Blind Boxes Collection - Minecraft Battle
    ------------------------------------------------------------
    University OpenGL / GLUT project.

    No external texture image files are required. The Creeper,
    Enderman, Blaze, floor, and blind box textures are generated at runtime
    with glGenTextures, glBindTexture, and glTexImage2D.

    Code::Blocks / MinGW setup:
      1. Add CGLabmain.cpp and this file as your .cpp sources.
      2. Link these libraries: opengl32, glu32, glut32/freeglut.
      3. Place glut32.dll/freeglut.dll beside the .exe if Code::Blocks
         does not already find it.

    Controls:
      Player 1 - W/A/S/D move, G primary skill, H secondary skill
      Player 2 - Arrow keys move, 8 primary skill, 9 secondary skill
      R reset battle, Esc return to menu during battle

    Technical notes:
      - Characters are assembled hierarchically with glPushMatrix,
        glPopMatrix, glTranslatef, glRotatef, and glScalef.
      - Cubes/prisms are drawn manually from vertex, face, normal,
        and texture-coordinate arrays. No object loaders are used.
      - Fixed-function OpenGL lighting, shading, and procedural
        texturing are used throughout.
*/

#include <GL/glut.h>

#include "CGLabMinecraft.hpp"

#include <windows.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <vector>

const float PI = 3.1415926535f;
const float DEG_PER_RAD = 180.0f / PI;
const float ARENA_HALF_SIZE = 12.0f;
const int WINDOW_START_WIDTH = 1100;
const int WINDOW_START_HEIGHT = 720;

enum CharacterType
{
    CHARACTER_CREEPER = 0,
    CHARACTER_ENDERMAN = 1,
    CHARACTER_BLAZE = 2,
    CHARACTER_COUNT = 3
};

CharacterType normalizeCharacterType(int value)
{
    if (value < 0 || value >= CHARACTER_COUNT)
    {
        return CHARACTER_CREEPER;
    }
    return static_cast<CharacterType>(value);
}

const char* characterTypeName(CharacterType type)
{
    switch (type)
    {
        case CHARACTER_CREEPER:
            return "Creeper";
        case CHARACTER_ENDERMAN:
            return "Enderman";
        case CHARACTER_BLAZE:
            return "Blaze";
        default:
            return "Unknown";
    }
}

struct Vec3
{
    float x;
    float y;
    float z;

    Vec3() : x(0.0f), y(0.0f), z(0.0f) {}
    Vec3(float px, float py, float pz) : x(px), y(py), z(pz) {}
};

Vec3 operator+(const Vec3& a, const Vec3& b)
{
    return Vec3(a.x + b.x, a.y + b.y, a.z + b.z);
}

Vec3 operator-(const Vec3& a, const Vec3& b)
{
    return Vec3(a.x - b.x, a.y - b.y, a.z - b.z);
}

Vec3 operator*(const Vec3& a, float scalar)
{
    return Vec3(a.x * scalar, a.y * scalar, a.z * scalar);
}

float clampFloat(float value, float low, float high)
{
    return std::max(low, std::min(value, high));
}

float length2D(const Vec3& v)
{
    return std::sqrt(v.x * v.x + v.z * v.z);
}

Vec3 normalize2D(const Vec3& v)
{
    const float len = length2D(v);
    if (len < 0.0001f)
    {
        return Vec3();
    }
    return Vec3(v.x / len, 0.0f, v.z / len);
}

float randomRange(float low, float high)
{
    const float t = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
    return low + (high - low) * t;
}

struct Material
{
    float ambient[4];
    float diffuse[4];
    float specular[4];
    float emission[4];
    float shininess;

    Material()
    {
        ambient[0] = ambient[1] = ambient[2] = 0.2f;
        ambient[3] = 1.0f;
        diffuse[0] = diffuse[1] = diffuse[2] = 0.8f;
        diffuse[3] = 1.0f;
        specular[0] = specular[1] = specular[2] = 0.1f;
        specular[3] = 1.0f;
        emission[0] = emission[1] = emission[2] = 0.0f;
        emission[3] = 1.0f;
        shininess = 16.0f;
    }
};

Material makeMaterial(float r, float g, float b, float shininess)
{
    Material m;
    m.ambient[0] = r * 0.35f;
    m.ambient[1] = g * 0.35f;
    m.ambient[2] = b * 0.35f;
    m.diffuse[0] = r;
    m.diffuse[1] = g;
    m.diffuse[2] = b;
    m.specular[0] = m.specular[1] = m.specular[2] = 0.18f;
    m.shininess = shininess;
    return m;
}

Material makeEmissiveMaterial(float r, float g, float b)
{
    Material m = makeMaterial(r, g, b, 8.0f);
    m.emission[0] = r;
    m.emission[1] = g;
    m.emission[2] = b;
    return m;
}

void applyMaterial(const Material& m)
{
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, m.ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, m.diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, m.specular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, m.emission);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, m.shininess);
    glColor4fv(m.diffuse);
}

GLuint createTexture(const std::vector<unsigned char>& data, int width, int height)
{
    GLuint textureId = 0;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, &data[0]);
    glBindTexture(GL_TEXTURE_2D, 0);
    return textureId;
}

GLuint buildCreeperTexture()
{
    const int size = 16;
    std::vector<unsigned char> pixels(size * size * 3);
    for (int y = 0; y < size; ++y)
    {
        for (int x = 0; x < size; ++x)
        {
            const int block = ((x / 2) + (y / 2)) % 4;
            unsigned char r = 55;
            unsigned char g = static_cast<unsigned char>(135 + block * 24);
            unsigned char b = static_cast<unsigned char>(48 + block * 8);
            if ((x + y) % 7 == 0)
            {
                r = 30;
                g = 95;
                b = 32;
            }
            const int index = (y * size + x) * 3;
            pixels[index + 0] = r;
            pixels[index + 1] = g;
            pixels[index + 2] = b;
        }
    }
    return createTexture(pixels, size, size);
}

GLuint buildEndermanTexture()
{
    const int size = 16;
    std::vector<unsigned char> pixels(size * size * 3);
    for (int y = 0; y < size; ++y)
    {
        for (int x = 0; x < size; ++x)
        {
            const int noise = ((x * 13 + y * 17) % 5);
            const unsigned char value = static_cast<unsigned char>(8 + noise * 7);
            const int index = (y * size + x) * 3;
            pixels[index + 0] = value;
            pixels[index + 1] = value;
            pixels[index + 2] = static_cast<unsigned char>(value + 6);
        }
    }
    return createTexture(pixels, size, size);
}

GLuint buildBlazeTexture()
{
    const int size = 16;
    std::vector<unsigned char> pixels(size * size * 3);
    for (int y = 0; y < size; ++y)
    {
        for (int x = 0; x < size; ++x)
        {
            const int noise = (x * 17 + y * 29 + (x * y) % 31) % 28;
            const int band = y / 3;
            const bool brightPlate = (y < 4 && x > 1 && x < size - 2);
            const bool palePlate = ((x + y) % 5 == 0 && y < 11);
            const bool goldenStripe = (y == 4 || y == 8 || y == 12);
            const bool edge = (x == 0 || y == 0 || x == size - 1 || y == size - 1);
            const bool amberShadow = ((x + y * 2) % 13 == 0 && y > 3);
            const bool sootCrack = ((x * 7 + y * 5) % 41 == 0 && y > 5);

            int r = 244 - band * 3 + noise / 3;
            int g = 214 - band * 4 + noise;
            int b = 18 + noise / 5;

            if (brightPlate)
            {
                r = 255;
                g = 238 + noise / 5;
                b = 38;
            }
            if (palePlate)
            {
                r = 255;
                g = 248;
                b = 72;
            }
            if (goldenStripe)
            {
                r = 250;
                g = 198 + noise / 6;
                b = 18;
            }
            if (amberShadow)
            {
                r = 218;
                g = 126;
                b = 8;
            }
            if (edge)
            {
                r = static_cast<int>(r * 0.72f);
                g = static_cast<int>(g * 0.78f);
                b = static_cast<int>(b * 0.72f);
            }
            if (sootCrack)
            {
                r = 84;
                g = 56;
                b = 10;
            }

            const int index = (y * size + x) * 3;
            pixels[index + 0] = static_cast<unsigned char>(std::max(0, std::min(255, r)));
            pixels[index + 1] = static_cast<unsigned char>(std::max(0, std::min(255, g)));
            pixels[index + 2] = static_cast<unsigned char>(std::max(0, std::min(255, b)));
        }
    }
    return createTexture(pixels, size, size);
}

GLuint buildFloorTexture()
{
    const int size = 32;
    std::vector<unsigned char> pixels(size * size * 3);
    for (int y = 0; y < size; ++y)
    {
        for (int x = 0; x < size; ++x)
        {
            const int noise = (x * 19 + y * 23 + (x * y) % 17) % 42;
            const bool lightBlade = ((x + y * 3) % 11 == 0);
            const bool darkPatch = ((x * 5 + y * 7) % 29 == 0);
            unsigned char r = static_cast<unsigned char>(36 + noise / 4);
            unsigned char g = static_cast<unsigned char>(112 + noise);
            unsigned char b = static_cast<unsigned char>(36 + noise / 5);

            if (lightBlade)
            {
                r = static_cast<unsigned char>(r + 18);
                g = static_cast<unsigned char>(std::min(190, static_cast<int>(g) + 30));
                b = static_cast<unsigned char>(b + 8);
            }
            if (darkPatch)
            {
                r = static_cast<unsigned char>(r * 0.70f);
                g = static_cast<unsigned char>(g * 0.78f);
                b = static_cast<unsigned char>(b * 0.70f);
            }
            const int index = (y * size + x) * 3;
            pixels[index + 0] = r;
            pixels[index + 1] = g;
            pixels[index + 2] = b;
        }
    }
    return createTexture(pixels, size, size);
}

GLuint buildBlindBoxTexture()
{
    const int size = 16;
    std::vector<unsigned char> pixels(size * size * 3);
    for (int y = 0; y < size; ++y)
    {
        for (int x = 0; x < size; ++x)
        {
            const bool ribbon = (x >= 7 && x <= 8) || (y >= 7 && y <= 8);
            const bool edge = (x == 0 || y == 0 || x == size - 1 || y == size - 1);
            unsigned char r = 205;
            unsigned char g = 154;
            unsigned char b = 64;
            if (ribbon)
            {
                r = 136;
                g = 55;
                b = 192;
            }
            if (edge)
            {
                r = static_cast<unsigned char>(r * 0.65f);
                g = static_cast<unsigned char>(g * 0.65f);
                b = static_cast<unsigned char>(b * 0.65f);
            }
            const int index = (y * size + x) * 3;
            pixels[index + 0] = r;
            pixels[index + 1] = g;
            pixels[index + 2] = b;
        }
    }
    return createTexture(pixels, size, size);
}

class PrimitiveRenderer
{
public:
    static void drawUnitCube()
    {
        static const GLfloat vertices[8][3] =
        {
            {-0.5f, -0.5f,  0.5f},
            { 0.5f, -0.5f,  0.5f},
            { 0.5f,  0.5f,  0.5f},
            {-0.5f,  0.5f,  0.5f},
            {-0.5f, -0.5f, -0.5f},
            { 0.5f, -0.5f, -0.5f},
            { 0.5f,  0.5f, -0.5f},
            {-0.5f,  0.5f, -0.5f}
        };

        static const GLint faces[6][4] =
        {
            {0, 1, 2, 3},
            {5, 4, 7, 6},
            {4, 0, 3, 7},
            {1, 5, 6, 2},
            {3, 2, 6, 7},
            {4, 5, 1, 0}
        };

        static const GLfloat normals[6][3] =
        {
            { 0.0f,  0.0f,  1.0f},
            { 0.0f,  0.0f, -1.0f},
            {-1.0f,  0.0f,  0.0f},
            { 1.0f,  0.0f,  0.0f},
            { 0.0f,  1.0f,  0.0f},
            { 0.0f, -1.0f,  0.0f}
        };

        static const GLfloat texCoords[4][2] =
        {
            {0.0f, 0.0f},
            {1.0f, 0.0f},
            {1.0f, 1.0f},
            {0.0f, 1.0f}
        };

        glBegin(GL_QUADS);
        for (int face = 0; face < 6; ++face)
        {
            glNormal3fv(normals[face]);
            for (int corner = 0; corner < 4; ++corner)
            {
                glTexCoord2fv(texCoords[corner]);
                glVertex3fv(vertices[faces[face][corner]]);
            }
        }
        glEnd();
    }

    static void drawBox(float width, float height, float depth)
    {
        glPushMatrix();
        glScalef(width, height, depth);
        drawUnitCube();
        glPopMatrix();
    }

    static void drawFlatRect(float x, float y, float z,
                             float width, float height,
                             float r, float g, float b, float alpha)
    {
        glColor4f(r, g, b, alpha);
        glBegin(GL_QUADS);
        glNormal3f(0.0f, 0.0f, 1.0f);
        glVertex3f(x - width * 0.5f, y - height * 0.5f, z);
        glVertex3f(x + width * 0.5f, y - height * 0.5f, z);
        glVertex3f(x + width * 0.5f, y + height * 0.5f, z);
        glVertex3f(x - width * 0.5f, y + height * 0.5f, z);
        glEnd();
    }

    static void drawCircleXZ(const Vec3& center, float radius, int segments)
    {
        glBegin(GL_LINE_LOOP);
        for (int i = 0; i < segments; ++i)
        {
            const float angle = (2.0f * PI * static_cast<float>(i)) /
                                static_cast<float>(segments);
            glVertex3f(center.x + std::cos(angle) * radius,
                       center.y,
                       center.z + std::sin(angle) * radius);
        }
        glEnd();
    }
};

struct TextureSet
{
    GLuint creeper;
    GLuint enderman;
    GLuint blaze;
    GLuint floor;
    GLuint blindBox;

    TextureSet() : creeper(0), enderman(0), blaze(0), floor(0), blindBox(0) {}
};

struct Particle
{
    Vec3 position;
    Vec3 velocity;
    float age;
    float life;
    float r;
    float g;
    float b;
};

class BattleCharacter
{
public:
    BattleCharacter()
        : mPosition(),
          mFacingDegrees(0.0f),
          mHealth(100.0f),
          mSpeed(3.0f),
          mSkillCooldown(0.0f),
          mSkillCooldownMax(3.0f),
          mWalkPhase(0.0f),
          mMoving(false),
          mKnockbackVelocity(),
          mDamageFlashTimer(0.0f),
          mDeathAnimationTimer(0.0f),
          mVictoryAnimationTimer(0.0f),
          mVictorious(false)
    {
    }

    virtual ~BattleCharacter() {}

    virtual void update(float dt)
    {
        if (mSkillCooldown > 0.0f)
        {
            mSkillCooldown = std::max(0.0f, mSkillCooldown - dt);
        }

        if (mDamageFlashTimer > 0.0f)
        {
            mDamageFlashTimer = std::max(0.0f, mDamageFlashTimer - dt);
        }

        if (mMoving)
        {
            mWalkPhase += dt * 8.0f;
        }

        if (mHealth <= 0.0f)
        {
            mDeathAnimationTimer = std::min(1.0f, mDeathAnimationTimer + dt * 1.35f);
            mMoving = false;
        }
        else
        {
            mDeathAnimationTimer = 0.0f;
        }

        if (mVictorious)
        {
            mVictoryAnimationTimer += dt;
            mMoving = false;
        }
        else
        {
            mVictoryAnimationTimer = 0.0f;
        }

        if (mPosition.y > 0.0f || length2D(mKnockbackVelocity) > 0.01f || mKnockbackVelocity.y > 0.01f)
        {
            mPosition = mPosition + mKnockbackVelocity * dt;
            mKnockbackVelocity.y -= 13.5f * dt;

            mPosition.x = clampFloat(mPosition.x, -ARENA_HALF_SIZE + 0.8f, ARENA_HALF_SIZE - 0.8f);
            mPosition.z = clampFloat(mPosition.z, -ARENA_HALF_SIZE + 0.8f, ARENA_HALF_SIZE - 0.8f);

            if (mPosition.y <= 0.0f)
            {
                mPosition.y = 0.0f;
                mKnockbackVelocity = Vec3();
            }
        }
    }

    virtual void draw(const TextureSet& textures) const = 0;
    virtual const char* name() const = 0;

    void moveBy(const Vec3& direction, float dt)
    {
        if (mPosition.y > 0.02f)
        {
            mMoving = false;
            return;
        }

        const Vec3 normalized = normalize2D(direction);
        mMoving = length2D(normalized) > 0.0f;
        if (!mMoving)
        {
            return;
        }

        mPosition = mPosition + normalized * (mSpeed * dt);
        mPosition.x = clampFloat(mPosition.x, -ARENA_HALF_SIZE + 0.8f, ARENA_HALF_SIZE - 0.8f);
        mPosition.z = clampFloat(mPosition.z, -ARENA_HALF_SIZE + 0.8f, ARENA_HALF_SIZE - 0.8f);
        mFacingDegrees = std::atan2(normalized.x, normalized.z) * DEG_PER_RAD;
    }

    void stopMoving()
    {
        mMoving = false;
    }

    void setPosition(const Vec3& p)
    {
        mPosition = p;
        if (mPosition.y <= 0.0f)
        {
            mPosition.y = 0.0f;
            mKnockbackVelocity = Vec3();
        }
    }

    const Vec3& position() const
    {
        return mPosition;
    }

    Vec3 facingDirection() const
    {
        const float radians = mFacingDegrees / DEG_PER_RAD;
        return Vec3(std::sin(radians), 0.0f, std::cos(radians));
    }

    float health() const
    {
        return mHealth;
    }

    void resetHealth()
    {
        mHealth = 100.0f;
    }

    void setVictorious(bool victorious)
    {
        mVictorious = victorious && mHealth > 0.0f;
    }

    void takeDamage(float damage)
    {
        if (damage <= 0.0f || mHealth <= 0.0f)
        {
            return;
        }

        mHealth = std::max(0.0f, mHealth - damage);
        mDamageFlashTimer = 0.48f;
    }

    void takePoisonDamage(float damage)
    {
        if (damage <= 0.0f || mHealth <= 0.0f)
        {
            return;
        }

        mHealth = std::max(0.0f, mHealth - damage);
        mDamageFlashTimer = 0.16f;
    }

    void applyKnockbackImpulse(const Vec3& direction, float horizontalSpeed, float verticalSpeed)
    {
        const Vec3 normalized = normalize2D(direction);
        if (length2D(normalized) <= 0.0f)
        {
            return;
        }

        mKnockbackVelocity.x = normalized.x * horizontalSpeed;
        mKnockbackVelocity.y = verticalSpeed;
        mKnockbackVelocity.z = normalized.z * horizontalSpeed;
        mMoving = false;
    }

    bool canUseSkill() const
    {
        return mSkillCooldown <= 0.0f && mHealth > 0.0f;
    }

    void resetBattleState()
    {
        mSkillCooldown = 0.0f;
        mWalkPhase = 0.0f;
        mMoving = false;
        mKnockbackVelocity = Vec3();
        mPosition.y = 0.0f;
        mDamageFlashTimer = 0.0f;
        mDeathAnimationTimer = 0.0f;
        mVictoryAnimationTimer = 0.0f;
        mVictorious = false;
    }

    float cooldownPercent() const
    {
        if (mSkillCooldownMax <= 0.0f)
        {
            return 0.0f;
        }
        return clampFloat(mSkillCooldown / mSkillCooldownMax, 0.0f, 1.0f);
    }

    float cooldownSeconds() const
    {
        return mSkillCooldown;
    }

protected:
    bool damageTintActive() const
    {
        return mDamageFlashTimer > 0.0f || mPosition.y > 0.02f;
    }

    Material characterMaterial(float r, float g, float b, float shininess) const
    {
        if (mHealth <= 0.0f)
        {
            const float darken = 0.30f;
            Material dead = makeMaterial(r * darken, g * darken, b * darken, 2.0f);
            dead.ambient[0] *= 0.45f;
            dead.ambient[1] *= 0.45f;
            dead.ambient[2] *= 0.45f;
            return dead;
        }

        if (!damageTintActive())
        {
            return makeMaterial(r, g, b, shininess);
        }

        const float flash = (mPosition.y > 0.02f) ? 0.94f : 0.86f;
        Material m = makeMaterial(r * (1.0f - flash) + 1.0f * flash,
                                  g * (1.0f - flash) + 0.04f * flash,
                                  b * (1.0f - flash) + 0.035f * flash,
                                  shininess);
        m.ambient[0] = std::max(m.ambient[0], 0.62f);
        m.ambient[1] = std::min(m.ambient[1], 0.10f);
        m.ambient[2] = std::min(m.ambient[2], 0.08f);
        m.emission[0] = (mPosition.y > 0.02f) ? 0.30f : 0.22f;
        m.emission[1] = 0.015f;
        m.emission[2] = 0.010f;
        return m;
    }

    Material characterEmissiveMaterial(float r, float g, float b) const
    {
        if (!damageTintActive())
        {
            return makeEmissiveMaterial(r, g, b);
        }

        Material m = makeEmissiveMaterial(1.0f, 0.015f, 0.010f);
        m.diffuse[0] = 1.0f;
        m.diffuse[1] = 0.015f;
        m.diffuse[2] = 0.010f;
        return m;
    }

    void beginCharacterTransform() const
    {
        glTranslatef(mPosition.x, mPosition.y, mPosition.z);

        if (mVictorious)
        {
            const float jump = std::fabs(std::sin(mVictoryAnimationTimer * 4.8f)) * 0.55f;
            const float cheerTurn = mVictoryAnimationTimer * 90.0f;
            glTranslatef(0.0f, jump, 0.0f);
            glRotatef(cheerTurn, 0.0f, 1.0f, 0.0f);
        }

        glRotatef(mFacingDegrees, 0.0f, 1.0f, 0.0f);

        if (mHealth <= 0.0f)
        {
            const float eased = mDeathAnimationTimer * mDeathAnimationTimer *
                                (3.0f - 2.0f * mDeathAnimationTimer);
            glTranslatef(0.0f, eased * 0.32f, 0.0f);
            glRotatef(eased * 88.0f, 0.0f, 0.0f, 1.0f);
        }
    }

    bool victoryAnimationActive() const
    {
        return mVictorious;
    }

    float victoryAnimationTime() const
    {
        return mVictoryAnimationTimer;
    }

    Vec3 mPosition;
    float mFacingDegrees;
    float mHealth;
    float mSpeed;
    float mSkillCooldown;
    float mSkillCooldownMax;
    float mWalkPhase;
    bool mMoving;
    Vec3 mKnockbackVelocity;
    float mDamageFlashTimer;
    float mDeathAnimationTimer;
    float mVictoryAnimationTimer;
    bool mVictorious;
};

class Creeper : public BattleCharacter
{
public:
    Creeper()
        : BattleCharacter(),
          mExplosionActive(false),
          mExplosionTimer(0.0f),
          mExplosionDuration(1.65f),
          mExplosionMaxRadius(3.5f),
          mExplosionDamagePending(false),
          mPowderCloudActive(false),
          mPowderCloudTimer(0.0f),
          mPowderCloudDuration(4.2f),
          mPowderCloudRadius(4.5f),
          mPowderCloudDamageTickTimer(0.0f),
          mPowderCloudCooldown(0.0f),
          mPowderCloudCooldownMax(5.0f)
    {
        mSpeed = 3.1f;
        mSkillCooldownMax = 4.0f;
    }

    const char* name() const
    {
        return "Creeper";
    }

    bool activateExplosion()
    {
        if (!canUseSkill())
        {
            return false;
        }

        mExplosionActive = true;
        mExplosionTimer = 0.0f;
        mExplosionOrigin = mPosition;
        mExplosionDamagePending = true;
        mParticles.clear();
        mSkillCooldown = mSkillCooldownMax;

        for (int i = 0; i < 220; ++i)
        {
            const float angle = randomRange(0.0f, 2.0f * PI);
            const float speed = randomRange(1.4f, 7.2f);
            Particle p;
            p.position = Vec3(mExplosionOrigin.x + randomRange(-0.18f, 0.18f),
                              randomRange(0.15f, 1.45f),
                              mExplosionOrigin.z + randomRange(-0.18f, 0.18f));
            p.velocity = Vec3(std::cos(angle) * speed,
                              randomRange(1.0f, 5.4f),
                              std::sin(angle) * speed);
            p.age = 0.0f;
            p.life = randomRange(0.72f, 1.55f);

            if (i % 8 == 0)
            {
                p.r = randomRange(0.02f, 0.08f);
                p.g = randomRange(0.02f, 0.08f);
                p.b = randomRange(0.02f, 0.08f);
            }
            else if (i % 4 == 0)
            {
                p.r = randomRange(0.18f, 0.42f);
                p.g = randomRange(0.85f, 1.0f);
                p.b = randomRange(0.10f, 0.24f);
            }
            else if (i % 3 == 0)
            {
                p.r = randomRange(0.82f, 1.0f);
                p.g = randomRange(0.86f, 1.0f);
                p.b = randomRange(0.78f, 0.94f);
            }
            else
            {
                const float smoke = randomRange(0.28f, 0.62f);
                p.r = smoke;
                p.g = smoke;
                p.b = smoke;
            }
            mParticles.push_back(p);
        }
        return true;
    }

    bool activatePowderCloud()
    {
        if (mPowderCloudCooldown > 0.0f || mHealth <= 0.0f)
        {
            return false;
        }

        mPowderCloudActive = true;
        mPowderCloudTimer = 0.0f;
        mPowderCloudDamageTickTimer = powderCloudDamageTickInterval();
        mPowderCloudOrigin = mPosition;
        mPowderCloudCooldown = mPowderCloudCooldownMax;
        return true;
    }

    void update(float dt)
    {
        BattleCharacter::update(dt);

        if (mPowderCloudCooldown > 0.0f)
        {
            mPowderCloudCooldown = std::max(0.0f, mPowderCloudCooldown - dt);
        }

        if (mPowderCloudActive)
        {
            mPowderCloudTimer += dt;
            mPowderCloudDamageTickTimer += dt;
            if (mPowderCloudTimer >= mPowderCloudDuration)
            {
                mPowderCloudActive = false;
            }
        }

        if (!mExplosionActive)
        {
            return;
        }

        mExplosionTimer += dt;
        if (mExplosionTimer >= explosionImpactDelay())
        {
            for (std::size_t i = 0; i < mParticles.size(); ++i)
            {
                Particle& p = mParticles[i];
                p.age += dt;

                const bool smokeParticle =
                    std::fabs(p.r - p.g) < 0.02f &&
                    std::fabs(p.g - p.b) < 0.02f &&
                    p.r > 0.20f;
                const float gravity = smokeParticle ? 1.2f : 6.2f;
                const float drag = smokeParticle ? 0.988f : 0.975f;

                p.velocity.x *= drag;
                p.velocity.z *= drag;
                p.velocity.y -= gravity * dt;
                p.position = p.position + p.velocity * dt;
                if (p.position.y < 0.08f)
                {
                    p.position.y = 0.08f;
                    p.velocity.x *= 0.72f;
                    p.velocity.z *= 0.72f;
                    p.velocity.y *= smokeParticle ? 0.0f : -0.22f;
                }
            }
        }

        if (mExplosionTimer >= mExplosionDuration)
        {
            mExplosionActive = false;
            mParticles.clear();
        }
    }

    void draw(const TextureSet& textures) const
    {
        glPushMatrix();
        beginCharacterTransform();

        const float charge = explosionChargeStrength();
        if (charge > 0.0f)
        {
            const float pulse = 0.72f + std::fabs(std::sin(mExplosionTimer * 44.0f)) * 0.28f;
            const float swell = charge * pulse;
            glTranslatef(0.0f, swell * 0.05f, 0.0f);
            glScalef(1.0f + swell * 0.18f,
                     1.0f + swell * 0.10f,
                     1.0f + swell * 0.18f);
        }

        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, textures.creeper);
        applyMaterial(creeperChargeMaterial());

        // Body: each body part starts as its own origin-centered prism.
        glPushMatrix();
        glTranslatef(0.0f, 1.38f + walkBodyBob(), 0.0f);
        PrimitiveRenderer::drawBox(0.95f, 1.45f, 0.72f);
        glPopMatrix();

        // Head: a separate cube stacked above the body.
        glPushMatrix();
        glTranslatef(0.0f, 2.48f + idleBounce() + walkBodyBob(), 0.0f);
        PrimitiveRenderer::drawBox(1.18f, 1.18f, 1.18f);
        glDisable(GL_TEXTURE_2D);
        drawCreeperFace();
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, textures.creeper);
        applyMaterial(creeperChargeMaterial());
        glPopMatrix();

        // Four low feet at the corners, like the Minecraft Creeper reference.
        const float xOffsets[2] = {-0.36f, 0.36f};
        const float zOffsets[2] = {-0.45f, 0.45f};
        int legIndex = 0;
        for (int x = 0; x < 2; ++x)
        {
            for (int z = 0; z < 2; ++z)
            {
                const float phase = (x == z) ? 0.0f : PI;
                const float step = mMoving ? std::sin(mWalkPhase + phase) * 0.10f : 0.0f;
                glPushMatrix();
                glTranslatef(xOffsets[x], 0.32f, zOffsets[z] + step);
                PrimitiveRenderer::drawBox(0.48f, 0.64f, 0.54f);
                glPopMatrix();
                ++legIndex;
            }
        }

        glBindTexture(GL_TEXTURE_2D, 0);
        glDisable(GL_TEXTURE_2D);
        glPopMatrix();

        drawExplosionEffect();
        drawPowderCloudEffect();
    }

    bool explosionDamageReady() const
    {
        return mExplosionActive &&
               mExplosionDamagePending &&
               mExplosionTimer > explosionImpactDelay();
    }

    void markExplosionDamageHandled()
    {
        mExplosionDamagePending = false;
    }

    float explosionRadius() const
    {
        if (!mExplosionActive || mExplosionTimer < explosionImpactDelay())
        {
            return 0.0f;
        }
        const float impactAge = mExplosionTimer - explosionImpactDelay();
        const float expansion = clampFloat(impactAge / 0.20f, 0.0f, 1.0f);
        const float eased = 1.0f - std::pow(1.0f - expansion, 3.0f);
        return mExplosionMaxRadius * eased;
    }

    float explosionDamageRadius() const
    {
        return mExplosionMaxRadius;
    }

    const Vec3& explosionOrigin() const
    {
        return mExplosionOrigin;
    }

    bool explosionActive() const
    {
        return mExplosionActive;
    }

    bool powderCloudActive() const
    {
        return mPowderCloudActive;
    }

    const Vec3& powderCloudOrigin() const
    {
        return mPowderCloudOrigin;
    }

    float powderCloudRadius() const
    {
        return mPowderCloudRadius;
    }

    float powderCloudDamagePerSecond() const
    {
        return 8.0f;
    }

    float powderCloudDamageTickInterval() const
    {
        return 0.62f;
    }

    bool powderCloudDamageTickReady() const
    {
        return mPowderCloudActive && mPowderCloudDamageTickTimer >= powderCloudDamageTickInterval();
    }

    void markPowderCloudDamageTickHandled()
    {
        mPowderCloudDamageTickTimer = std::max(0.0f, mPowderCloudDamageTickTimer - powderCloudDamageTickInterval());
    }

    float powderCloudCooldownPercent() const
    {
        return clampFloat(mPowderCloudCooldown / mPowderCloudCooldownMax, 0.0f, 1.0f);
    }

    float powderCloudCooldownSeconds() const
    {
        return mPowderCloudCooldown;
    }

    void resetForRound()
    {
        resetBattleState();
        mExplosionActive = false;
        mExplosionTimer = 0.0f;
        mExplosionDamagePending = false;
        mParticles.clear();
        mPowderCloudActive = false;
        mPowderCloudTimer = 0.0f;
        mPowderCloudDamageTickTimer = 0.0f;
        mPowderCloudCooldown = 0.0f;
    }

private:
    float explosionImpactDelay() const
    {
        return 0.28f;
    }

    float explosionChargeStrength() const
    {
        if (!mExplosionActive || mExplosionTimer >= explosionImpactDelay())
        {
            return 0.0f;
        }
        return clampFloat(mExplosionTimer / explosionImpactDelay(), 0.0f, 1.0f);
    }

    Material creeperChargeMaterial() const
    {
        const float charge = explosionChargeStrength();
        if (charge <= 0.0f)
        {
            return characterMaterial(0.55f, 0.95f, 0.45f, 12.0f);
        }

        const float flash =
            charge * (0.48f + std::fabs(std::sin(mExplosionTimer * 42.0f)) * 0.52f);
        Material material =
            makeMaterial(0.55f * (1.0f - flash) + flash,
                         0.95f * (1.0f - flash) + flash,
                         0.45f * (1.0f - flash) + flash,
                         18.0f);
        material.emission[0] = flash * 0.34f;
        material.emission[1] = flash * 0.42f;
        material.emission[2] = flash * 0.24f;
        return material;
    }

    float idleBounce() const
    {
        return std::sin(mWalkPhase * 0.5f) * 0.015f;
    }

    float walkBodyBob() const
    {
        if (!mMoving)
        {
            return 0.0f;
        }
        return std::fabs(std::sin(mWalkPhase * 2.0f)) * 0.025f;
    }

    void drawCreeperFace() const
    {
        applyMaterial(damageTintActive() ? characterMaterial(0.08f, 0.0f, 0.0f, 4.0f)
                                         : makeMaterial(0.0f, 0.0f, 0.0f, 4.0f));
        const float z = 0.594f;
        PrimitiveRenderer::drawFlatRect(-0.25f, 0.15f, z, 0.22f, 0.25f, 0.0f, 0.0f, 0.0f, 1.0f);
        PrimitiveRenderer::drawFlatRect(0.25f, 0.15f, z, 0.22f, 0.25f, 0.0f, 0.0f, 0.0f, 1.0f);
        PrimitiveRenderer::drawFlatRect(0.0f, -0.12f, z, 0.20f, 0.42f, 0.0f, 0.0f, 0.0f, 1.0f);
        PrimitiveRenderer::drawFlatRect(-0.15f, -0.32f, z, 0.18f, 0.18f, 0.0f, 0.0f, 0.0f, 1.0f);
        PrimitiveRenderer::drawFlatRect(0.15f, -0.32f, z, 0.18f, 0.18f, 0.0f, 0.0f, 0.0f, 1.0f);
    }

    void drawExplosionEffect() const
    {
        if (!mExplosionActive || mExplosionTimer < explosionImpactDelay())
        {
            return;
        }

        glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_DEPTH_BUFFER_BIT | GL_LINE_BIT | GL_POINT_BIT);
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);

        const float impactAge = mExplosionTimer - explosionImpactDelay();
        const float effectDuration = mExplosionDuration - explosionImpactDelay();
        const float normalizedAge =
            clampFloat(impactAge / effectDuration, 0.0f, 1.0f);
        const float radius = explosionRadius();
        const float fade = 1.0f - normalizedAge;
        const float flash =
            1.0f - clampFloat(impactAge / 0.16f, 0.0f, 1.0f);

        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glLineWidth(7.0f);
        glColor4f(1.0f, 1.0f, 0.88f, flash * 0.95f);
        PrimitiveRenderer::drawCircleXZ(
            Vec3(mExplosionOrigin.x, 0.16f, mExplosionOrigin.z),
            radius * 0.36f, 96);

        glLineWidth(4.5f);
        glColor4f(0.46f, 1.0f, 0.08f, (0.72f * fade) + flash * 0.20f);
        PrimitiveRenderer::drawCircleXZ(
            Vec3(mExplosionOrigin.x, 0.12f, mExplosionOrigin.z),
            radius, 96);
        glLineWidth(2.5f);
        glColor4f(0.88f, 1.0f, 0.70f, 0.54f * fade);
        PrimitiveRenderer::drawCircleXZ(
            Vec3(mExplosionOrigin.x, 0.15f, mExplosionOrigin.z),
            radius * 0.72f, 96);

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glPointSize(10.0f);
        glBegin(GL_POINTS);
        for (std::size_t i = 0; i < mParticles.size(); ++i)
        {
            const Particle& p = mParticles[i];
            if (p.age < p.life)
            {
                const float lifeFade = 1.0f - p.age / p.life;
                const float a = lifeFade * lifeFade;
                glColor4f(p.r, p.g, p.b, a);
                glVertex3f(p.position.x, p.position.y, p.position.z);
            }
        }
        glEnd();

        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glPointSize(6.0f);
        glBegin(GL_POINTS);
        for (int i = 0; i < 96; ++i)
        {
            const float angle = i * 2.399f + impactAge * 1.4f;
            const float shell = 0.48f + (i % 10) * 0.055f;
            const float spread = radius * shell;
            const float height =
                0.18f + std::sin((i % 13) * 0.24f) * radius * 0.36f +
                (i % 5) * 0.07f;
            const float a = fade * (0.36f + (i % 4) * 0.11f);
            if (i % 5 == 0)
            {
                glColor4f(0.16f, 0.95f, 0.06f, a);
            }
            else
            {
                glColor4f(0.88f, 1.0f, 0.76f, a);
            }
            glVertex3f(mExplosionOrigin.x + std::cos(angle) * spread,
                       height,
                       mExplosionOrigin.z + std::sin(angle) * spread);
        }
        glEnd();

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glPointSize(18.0f);
        glBegin(GL_POINTS);
        for (int i = 0; i < 42; ++i)
        {
            const float seed = static_cast<float>(i);
            const float angle = seed * 2.17f + impactAge * 0.22f;
            const float driftRadius = 0.24f + (i % 9) * 0.17f;
            const float rise =
                impactAge * (0.45f + (i % 6) * 0.10f) +
                std::sin(seed * 1.3f) * 0.12f;
            const float smokeFade =
                clampFloat(1.0f - impactAge / effectDuration, 0.0f, 1.0f);
            const float shade = 0.14f + (i % 5) * 0.055f;
            glColor4f(shade, shade, shade, smokeFade * 0.46f);
            glVertex3f(mExplosionOrigin.x + std::cos(angle) * driftRadius,
                       0.35f + rise,
                       mExplosionOrigin.z + std::sin(angle) * driftRadius);
        }
        glEnd();

        glDepthMask(GL_TRUE);
        glPopAttrib();
    }

    void drawPowderCloudEffect() const
    {
        if (!mPowderCloudActive)
        {
            return;
        }

        const float age = clampFloat(mPowderCloudTimer / mPowderCloudDuration, 0.0f, 1.0f);
        const float fade = 1.0f - age;
        const float pulse = 0.85f + std::sin(mPowderCloudTimer * 8.0f) * 0.15f;
        const float radius = mPowderCloudRadius * pulse;

        glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_DEPTH_BUFFER_BIT | GL_LINE_BIT | GL_POINT_BIT);
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);

        glLineWidth(2.0f);
        glColor4f(0.18f, 1.0f, 0.22f, 0.55f * fade);
        PrimitiveRenderer::drawCircleXZ(Vec3(mPowderCloudOrigin.x, 0.09f, mPowderCloudOrigin.z),
                                        radius, 72);
        glColor4f(0.75f, 1.0f, 0.28f, 0.28f * fade);
        PrimitiveRenderer::drawCircleXZ(Vec3(mPowderCloudOrigin.x, 0.11f, mPowderCloudOrigin.z),
                                        radius * 0.62f, 72);

        glPointSize(7.0f);
        glBegin(GL_POINTS);
        for (int i = 0; i < 54; ++i)
        {
            const float angle = i * 0.71f + mPowderCloudTimer * 1.8f;
            const float ring = (0.25f + (i % 9) * 0.08f) * radius;
            const float height = 0.22f + (i % 6) * 0.18f + std::sin(mPowderCloudTimer * 3.0f + i) * 0.12f;
            const float alpha = (0.25f + (i % 4) * 0.07f) * fade;
            glColor4f(0.32f, 1.0f, 0.18f, alpha);
            glVertex3f(mPowderCloudOrigin.x + std::cos(angle) * ring,
                       height,
                       mPowderCloudOrigin.z + std::sin(angle) * ring);
        }
        glEnd();

        glDepthMask(GL_TRUE);
        glPopAttrib();
    }

    bool mExplosionActive;
    float mExplosionTimer;
    float mExplosionDuration;
    float mExplosionMaxRadius;
    bool mExplosionDamagePending;
    Vec3 mExplosionOrigin;
    std::vector<Particle> mParticles;
    bool mPowderCloudActive;
    float mPowderCloudTimer;
    float mPowderCloudDuration;
    float mPowderCloudRadius;
    float mPowderCloudDamageTickTimer;
    float mPowderCloudCooldown;
    float mPowderCloudCooldownMax;
    Vec3 mPowderCloudOrigin;
};

class Enderman : public BattleCharacter
{
public:
    Enderman()
        : BattleCharacter(),
          mTeleportActive(false),
          mTeleportTimer(0.0f),
          mTeleportDuration(0.9f),
          mTeleportDamagePending(false),
          mVoidBeamActive(false),
          mVoidBeamTimer(0.0f),
          mVoidBeamDuration(0.7f),
          mVoidBeamCooldown(0.0f),
          mVoidBeamCooldownMax(4.0f),
          mVoidBeamDamagePending(false),
          mParticleTimer(0.0f)
    {
        mSpeed = 3.55f;
        mSkillCooldownMax = 3.0f;
    }

    const char* name() const
    {
        return "Enderman";
    }

    bool activateTeleport(const BattleCharacter& opponent)
    {
        if (!canUseSkill())
        {
            return false;
        }

        const Vec3& opponentPosition = opponent.position();
        if (length2D(opponentPosition - mPosition) > teleportMaxRange())
        {
            return false;
        }

        mTeleportFrom = mPosition;
        Vec3 enemyForward = normalize2D(opponent.facingDirection());
        if (length2D(enemyForward) < 0.0001f)
        {
            enemyForward = normalize2D(opponentPosition - mPosition);
            if (length2D(enemyForward) < 0.0001f)
            {
                enemyForward = Vec3(0.0f, 0.0f, 1.0f);
            }
        }

        Vec3 target = opponentPosition - enemyForward * 1.18f;
        target.x = clampFloat(target.x, -ARENA_HALF_SIZE + 0.8f, ARENA_HALF_SIZE - 0.8f);
        target.z = clampFloat(target.z, -ARENA_HALF_SIZE + 0.8f, ARENA_HALF_SIZE - 0.8f);

        for (int attempt = 0; attempt < 20; ++attempt)
        {
            if (length2D(target - opponentPosition) > 0.85f)
            {
                break;
            }
            const float angle = attempt * 1.7f;
            Vec3 direction = normalize2D(Vec3(-enemyForward.x + std::cos(angle) * 0.35f,
                                              0.0f,
                                              -enemyForward.z + std::sin(angle) * 0.35f));
            target = opponentPosition + direction * 1.35f;
            target.x = clampFloat(target.x, -ARENA_HALF_SIZE + 0.8f, ARENA_HALF_SIZE - 0.8f);
            target.z = clampFloat(target.z, -ARENA_HALF_SIZE + 0.8f, ARENA_HALF_SIZE - 0.8f);
        }

        mPosition = target;
        mTeleportTo = target;
        mTeleportActive = true;
        mTeleportTimer = 0.0f;
        mTeleportDamagePending = true;
        mSkillCooldown = mSkillCooldownMax;

        const Vec3 face = normalize2D(opponentPosition - mPosition);
        if (length2D(face) > 0.0001f)
        {
            mFacingDegrees = std::atan2(face.x, face.z) * DEG_PER_RAD;
        }
        return true;
    }

    bool activateVoidBeam(const Vec3& opponentPosition)
    {
        if (mVoidBeamCooldown > 0.0f || mHealth <= 0.0f)
        {
            return false;
        }

        if (length2D(opponentPosition - mPosition) > voidBeamMaxRange())
        {
            return false;
        }

        mVoidBeamActive = true;
        mVoidBeamTimer = 0.0f;
        mVoidBeamStart = mPosition;
        mVoidBeamTarget = opponentPosition;
        mVoidBeamDamagePending = true;
        mVoidBeamCooldown = mVoidBeamCooldownMax;

        const Vec3 face = normalize2D(opponentPosition - mPosition);
        if (length2D(face) > 0.0001f)
        {
            mFacingDegrees = std::atan2(face.x, face.z) * DEG_PER_RAD;
        }
        return true;
    }

    void update(float dt)
    {
        BattleCharacter::update(dt);
        mParticleTimer += dt;

        if (mTeleportActive)
        {
            mTeleportTimer += dt;
            if (mTeleportTimer >= mTeleportDuration)
            {
                mTeleportActive = false;
            }
        }

        if (mVoidBeamCooldown > 0.0f)
        {
            mVoidBeamCooldown = std::max(0.0f, mVoidBeamCooldown - dt);
        }

        if (mVoidBeamActive)
        {
            mVoidBeamTimer += dt;
            if (mVoidBeamTimer >= mVoidBeamDuration)
            {
                mVoidBeamActive = false;
            }
        }
    }

    void draw(const TextureSet& textures) const
    {
        if (mTeleportActive)
        {
            drawAfterImage(mTeleportFrom, textures, 0.32f);
        }

        const float blinkAlpha = teleportVisibility();
        glPushMatrix();
        beginCharacterTransform();

        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, textures.enderman);
        applyMaterial(characterMaterial(0.04f, 0.04f, 0.055f, 18.0f));
        glColor4f(0.9f, 0.9f, 1.0f, blinkAlpha);
        if (damageTintActive())
        {
            glColor4f(1.0f, 0.16f, 0.14f, blinkAlpha);
        }

        // Tall center body prism.
        glPushMatrix();
        glTranslatef(0.0f, 2.25f + walkBodyBob(), 0.0f);
        PrimitiveRenderer::drawBox(0.62f, 2.05f, 0.42f);
        glPopMatrix();

        // Local head cube with animated glowing eyes on its front plane.
        glPushMatrix();
        glTranslatef(0.0f, 3.65f + idleSway() + walkBodyBob(), 0.0f);
        PrimitiveRenderer::drawBox(0.88f, 0.88f, 0.88f);
        glDisable(GL_TEXTURE_2D);
        drawEyes();
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, textures.enderman);
        glPopMatrix();

        // Long arms, created symmetrically from the same primitive.
        const float armX[2] = {-0.47f, 0.47f};
        const float castPose = castingPoseStrength();
        for (int i = 0; i < 2; ++i)
        {
            const float swing = mMoving ? std::sin(mWalkPhase + i * PI) * 4.0f : 0.0f;
            glPushMatrix();
            glTranslatef(armX[i], 3.28f, 0.0f);
            glRotatef(swing - castPose * 112.0f, 1.0f, 0.0f, 0.0f);
            glRotatef((i == 0 ? -1.0f : 1.0f) * castPose * 8.0f,
                      0.0f, 0.0f, 1.0f);
            glTranslatef(0.0f, -1.55f, 0.0f);
            PrimitiveRenderer::drawBox(0.18f, 3.10f, 0.18f);
            glPopMatrix();
        }

        // Long legs, also assembled from local prisms.
        const float legX[2] = {-0.22f, 0.22f};
        for (int i = 0; i < 2; ++i)
        {
            const float phase = mWalkPhase + i * PI;
            const float swing = mMoving ? std::sin(phase + PI) * 10.0f : 0.0f;
            const float step = mMoving ? std::sin(phase + PI) * 0.10f : 0.0f;
            glPushMatrix();
            glTranslatef(legX[i], 1.82f, step);
            glRotatef(swing, 1.0f, 0.0f, 0.0f);
            glTranslatef(0.0f, -0.92f, 0.0f);
            PrimitiveRenderer::drawBox(0.25f, 1.85f, 0.25f);
            glPopMatrix();
        }

        glBindTexture(GL_TEXTURE_2D, 0);
        glDisable(GL_TEXTURE_2D);
        glPopMatrix();

        drawEnderParticles();
        drawCastingGatherEffect();
        drawTeleportEffect();
        drawVoidBeamEffect();
    }

    bool teleportActive() const
    {
        return mTeleportActive;
    }

    float teleportPulse() const
    {
        if (!mTeleportActive)
        {
            return 0.0f;
        }
        return 1.0f - clampFloat(mTeleportTimer / mTeleportDuration, 0.0f, 1.0f);
    }

    const Vec3& teleportFrom() const
    {
        return mTeleportFrom;
    }

    const Vec3& teleportTo() const
    {
        return mTeleportTo;
    }

    bool teleportDamageReady() const
    {
        return mTeleportActive && mTeleportDamagePending && mTeleportTimer > 0.12f;
    }

    void markTeleportDamageHandled()
    {
        mTeleportDamagePending = false;
    }

    float teleportSlashRadius() const
    {
        return 2.2f;
    }

    float teleportMaxRange() const
    {
        return 8.0f;
    }

    float teleportSlashDamage() const
    {
        return 28.0f;
    }

    bool voidBeamDamageReady() const
    {
        return mVoidBeamActive && mVoidBeamDamagePending && mVoidBeamTimer > 0.16f;
    }

    void markVoidBeamDamageHandled()
    {
        mVoidBeamDamagePending = false;
    }

    const Vec3& voidBeamTarget() const
    {
        return mVoidBeamTarget;
    }

    float voidBeamRadius() const
    {
        return 1.1f;
    }

    float voidBeamMaxRange() const
    {
        return 12.0f;
    }

    float voidBeamDamage(float castDistance) const
    {
        const float t = clampFloat(castDistance / voidBeamMaxRange(), 0.0f, 1.0f);
        return 26.0f - t * 12.0f;
    }

    float voidBeamCooldownPercent() const
    {
        return clampFloat(mVoidBeamCooldown / mVoidBeamCooldownMax, 0.0f, 1.0f);
    }

    float voidBeamCooldownSeconds() const
    {
        return mVoidBeamCooldown;
    }

    void resetForRound()
    {
        resetBattleState();
        mTeleportActive = false;
        mTeleportTimer = 0.0f;
        mTeleportDamagePending = false;
        mVoidBeamActive = false;
        mVoidBeamTimer = 0.0f;
        mVoidBeamCooldown = 0.0f;
        mVoidBeamDamagePending = false;
    }

private:
    float castingPoseStrength() const
    {
        float pose = 0.0f;
        if (mTeleportActive)
        {
            const float t = clampFloat(mTeleportTimer / 0.34f, 0.0f, 1.0f);
            pose = std::max(pose, std::sin(t * PI));
        }
        if (mVoidBeamActive)
        {
            const float t = clampFloat(mVoidBeamTimer / 0.34f, 0.0f, 1.0f);
            pose = std::max(pose, std::sin(t * PI));
        }
        return clampFloat(pose, 0.0f, 1.0f);
    }

    float idleSway() const
    {
        return std::sin(mWalkPhase * 0.35f) * 0.025f;
    }

    float walkBodyBob() const
    {
        if (!mMoving)
        {
            return 0.0f;
        }
        return std::fabs(std::sin(mWalkPhase * 2.0f)) * 0.025f;
    }

    float teleportVisibility() const
    {
        if (!mTeleportActive)
        {
            return 1.0f;
        }
        const float flicker = std::sin(mTeleportTimer * 80.0f) * 0.5f + 0.5f;
        return 0.45f + flicker * 0.55f;
    }

    void drawEyes() const
    {
        const float pulse = 0.65f + std::sin(mWalkPhase * 2.4f) * 0.25f;
        applyMaterial(characterEmissiveMaterial(0.65f * pulse, 0.12f, 1.0f));

        glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);
        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        const float eyeR = damageTintActive() ? 1.0f : 0.68f;
        const float eyeG = damageTintActive() ? 0.08f : 0.12f;
        const float eyeB = damageTintActive() ? 0.04f : 1.0f;
        PrimitiveRenderer::drawFlatRect(-0.18f, 0.08f, 0.448f, 0.22f, 0.08f, eyeR, eyeG, eyeB, 1.0f);
        PrimitiveRenderer::drawFlatRect(0.18f, 0.08f, 0.448f, 0.22f, 0.08f, eyeR, eyeG, eyeB, 1.0f);
        PrimitiveRenderer::drawFlatRect(-0.18f, 0.08f, 0.451f, 0.34f, 0.16f, eyeR, eyeG + 0.08f, eyeB, 0.22f);
        PrimitiveRenderer::drawFlatRect(0.18f, 0.08f, 0.451f, 0.34f, 0.16f, eyeR, eyeG + 0.08f, eyeB, 0.22f);
        glPopAttrib();
    }

    void drawAfterImage(const Vec3& location, const TextureSet& textures, float alpha) const
    {
        glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);

        glPushMatrix();
        glTranslatef(location.x, location.y, location.z);
        glRotatef(mFacingDegrees, 0.0f, 1.0f, 0.0f);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, textures.enderman);
        applyMaterial(makeMaterial(0.25f, 0.1f, 0.42f, 8.0f));
        glColor4f(0.65f, 0.25f, 1.0f, alpha);

        glPushMatrix();
        glTranslatef(0.0f, 2.25f, 0.0f);
        PrimitiveRenderer::drawBox(0.62f, 2.05f, 0.42f);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(0.0f, 3.65f, 0.0f);
        PrimitiveRenderer::drawBox(0.88f, 0.88f, 0.88f);
        glPopMatrix();

        glDisable(GL_TEXTURE_2D);
        glPopMatrix();

        glDepthMask(GL_TRUE);
        glPopAttrib();
    }

    void drawTeleportEffect() const
    {
        if (!mTeleportActive)
        {
            return;
        }

        glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_DEPTH_BUFFER_BIT | GL_LINE_BIT);
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glDepthMask(GL_FALSE);

        const float pulse = teleportPulse();
        const float radius = 0.8f + (1.0f - pulse) * 3.5f;
        glLineWidth(2.0f);
        glColor4f(0.65f, 0.15f, 1.0f, 0.75f * pulse);
        PrimitiveRenderer::drawCircleXZ(Vec3(mTeleportFrom.x, 0.12f, mTeleportFrom.z), radius, 72);
        glColor4f(0.25f, 0.85f, 1.0f, 0.55f * pulse);
        PrimitiveRenderer::drawCircleXZ(Vec3(mTeleportTo.x, 0.14f, mTeleportTo.z), radius * 0.75f, 72);

        for (int i = 0; i < 16; ++i)
        {
            const float angle = (2.0f * PI * i) / 16.0f;
            const float x = mTeleportTo.x + std::cos(angle) * radius * 0.55f;
            const float z = mTeleportTo.z + std::sin(angle) * radius * 0.55f;
            glBegin(GL_LINES);
            glVertex3f(x, 0.1f, z);
            glVertex3f(mTeleportTo.x, 1.6f * pulse, mTeleportTo.z);
            glEnd();
        }

        glDepthMask(GL_TRUE);
        glPopAttrib();
    }

    void drawEnderParticles() const
    {
        glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_DEPTH_BUFFER_BIT | GL_POINT_BIT);
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glDepthMask(GL_FALSE);
        glLineWidth(2.0f);
        for (int ring = 0; ring < 3; ++ring)
        {
            const float y = 0.25f + ring * 1.05f;
            const float radius = 0.72f + ring * 0.18f +
                                 std::sin(mParticleTimer * 2.0f + ring) * 0.08f;
            glColor4f(0.72f, 0.10f, 1.0f, 0.32f - ring * 0.06f);
            PrimitiveRenderer::drawCircleXZ(Vec3(mPosition.x, mPosition.y + y, mPosition.z),
                                            radius, 48);
        }

        glPointSize(7.0f);

        glBegin(GL_POINTS);
        for (int i = 0; i < 92; ++i)
        {
            const float seed = static_cast<float>(i);
            const float orbit = seed * 2.37f + mParticleTimer * (0.85f + (i % 7) * 0.08f);
            const float radius = 0.55f + static_cast<float>((i * 17) % 90) * 0.016f;
            const float heightBase = static_cast<float>((i * 23) % 100) / 100.0f;
            const float height = 0.10f + heightBase * 4.15f;
            const float drift = std::sin(mParticleTimer * 2.4f + seed) * 0.25f;
            const float alpha = 0.45f + static_cast<float>((i * 11) % 50) / 100.0f;

            glColor4f(0.78f, 0.12f, 1.0f, alpha);
            glVertex3f(mPosition.x + std::cos(orbit) * radius,
                       mPosition.y + height + drift,
                       mPosition.z + std::sin(orbit) * radius);

            if (i % 3 == 0)
            {
                glColor4f(0.98f, 0.42f, 1.0f, alpha * 0.82f);
                glVertex3f(mPosition.x + std::cos(orbit + 0.42f) * (radius + 0.22f),
                           mPosition.y + height - 0.14f,
                           mPosition.z + std::sin(orbit + 0.42f) * (radius + 0.22f));
            }

            if (i % 6 == 0)
            {
                glColor4f(0.38f, 0.06f, 1.0f, alpha * 0.72f);
                glVertex3f(mPosition.x + std::cos(-orbit * 0.85f) * (radius + 0.35f),
                           mPosition.y + 0.25f + heightBase * 3.6f,
                           mPosition.z + std::sin(-orbit * 0.85f) * (radius + 0.35f));
            }
        }
        glEnd();

        glDepthMask(GL_TRUE);
        glPopAttrib();
    }

    void drawCastingGatherEffect() const
    {
        const float cast = castingPoseStrength();
        if (cast <= 0.01f)
        {
            return;
        }

        glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_DEPTH_BUFFER_BIT |
                     GL_POINT_BIT | GL_LINE_BIT);
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glDepthMask(GL_FALSE);

        glPointSize(8.0f);
        glBegin(GL_POINTS);
        for (int i = 0; i < 72; ++i)
        {
            const float seed = static_cast<float>(i);
            const float angle = seed * 2.29f + mParticleTimer * 3.8f;
            const float outerRadius = 2.5f + (i % 8) * 0.12f;
            const float radius = outerRadius * (1.0f - cast * 0.76f);
            const float height = 0.35f + (i % 12) * 0.30f;
            const float spiral = std::sin(mParticleTimer * 8.0f + seed) * 0.16f;
            glColor4f(0.82f, 0.10f, 1.0f, 0.42f + cast * 0.48f);
            glVertex3f(mPosition.x + std::cos(angle) * radius,
                       mPosition.y + height + spiral,
                       mPosition.z + std::sin(angle) * radius);
        }
        glEnd();

        glLineWidth(2.5f);
        for (int ring = 0; ring < 3; ++ring)
        {
            const float radius = (1.8f - ring * 0.38f) * (1.0f - cast * 0.62f);
            glColor4f(0.62f, 0.04f, 1.0f, cast * (0.55f - ring * 0.10f));
            PrimitiveRenderer::drawCircleXZ(
                Vec3(mPosition.x, mPosition.y + 1.0f + ring * 0.85f, mPosition.z),
                radius, 64);
        }

        glDepthMask(GL_TRUE);
        glPopAttrib();
    }

    void drawVoidBeamEffect() const
    {
        if (!mVoidBeamActive)
        {
            return;
        }

        glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_DEPTH_BUFFER_BIT | GL_LINE_BIT | GL_POINT_BIT);
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glDepthMask(GL_FALSE);

        const float age = clampFloat(mVoidBeamTimer / mVoidBeamDuration, 0.0f, 1.0f);
        const float fade = 1.0f - age;
        const Vec3 start(mVoidBeamStart.x, 2.65f, mVoidBeamStart.z);
        const Vec3 end(mVoidBeamTarget.x, 1.2f, mVoidBeamTarget.z);

        glLineWidth(5.0f);
        glBegin(GL_LINES);
        glColor4f(0.85f, 0.22f, 1.0f, 0.95f * fade);
        glVertex3f(start.x, start.y, start.z);
        glColor4f(0.15f, 0.95f, 1.0f, 0.75f * fade);
        glVertex3f(end.x, end.y, end.z);
        glEnd();

        glLineWidth(1.5f);
        for (int i = 0; i < 6; ++i)
        {
            const float sway = std::sin(mVoidBeamTimer * 18.0f + i) * 0.18f;
            const float offset = (i - 2.5f) * 0.045f;
            glBegin(GL_LINES);
            glColor4f(0.45f, 0.05f, 1.0f, 0.45f * fade);
            glVertex3f(start.x + offset, start.y + sway, start.z - offset);
            glColor4f(0.08f, 0.85f, 1.0f, 0.35f * fade);
            glVertex3f(end.x - offset, end.y - sway, end.z + offset);
            glEnd();
        }

        glLineWidth(2.0f);
        glColor4f(0.75f, 0.12f, 1.0f, 0.65f * fade);
        PrimitiveRenderer::drawCircleXZ(Vec3(mVoidBeamTarget.x, 0.13f, mVoidBeamTarget.z),
                                        0.55f + age * 1.2f, 64);

        glPointSize(6.0f);
        glBegin(GL_POINTS);
        for (int i = 0; i < 28; ++i)
        {
            const float angle = i * 0.9f + mVoidBeamTimer * 5.0f;
            const float radius = 0.22f + (i % 7) * 0.12f;
            glColor4f(0.75f, 0.16f, 1.0f, 0.45f * fade);
            glVertex3f(mVoidBeamTarget.x + std::cos(angle) * radius,
                       0.25f + (i % 5) * 0.14f,
                       mVoidBeamTarget.z + std::sin(angle) * radius);
        }
        glEnd();

        glDepthMask(GL_TRUE);
        glPopAttrib();
    }

    bool mTeleportActive;
    float mTeleportTimer;
    float mTeleportDuration;
    bool mTeleportDamagePending;
    Vec3 mTeleportFrom;
    Vec3 mTeleportTo;
    bool mVoidBeamActive;
    float mVoidBeamTimer;
    float mVoidBeamDuration;
    float mVoidBeamCooldown;
    float mVoidBeamCooldownMax;
    bool mVoidBeamDamagePending;
    Vec3 mVoidBeamStart;
    Vec3 mVoidBeamTarget;
    float mParticleTimer;
};

class Blaze : public BattleCharacter
{
public:
    Blaze()
        : BattleCharacter(),
          mFireBurstActive(false),
          mFireBurstTimer(0.0f),
          mFireBurstDuration(0.95f),
          mFireBurstMaxRadius(4.0f),
          mFireBurstDamagePending(false),
          mFireballActive(false),
          mFireballTimer(0.0f),
          mFireballDuration(1.69f),
          mFireballCooldown(0.0f),
          mFireballCooldownMax(4.25f),
          mFireballDamagePending(false),
          mParticleTimer(0.0f)
    {
        mSpeed = 3.28f;
        mSkillCooldownMax = 3.35f;
    }

    const char* name() const
    {
        return "Blaze";
    }

    bool activateFireBurst(const Vec3& opponentPosition)
    {
        if (!canUseSkill())
        {
            return false;
        }

        faceOpponent(opponentPosition);
        mFireBurstActive = true;
        mFireBurstTimer = 0.0f;
        mFireBurstOrigin = mPosition;
        mFireBurstDamagePending = true;
        mSkillCooldown = mSkillCooldownMax;
        mBurstParticles.clear();

        for (int i = 0; i < 84; ++i)
        {
            const float angle = randomRange(0.0f, 2.0f * PI);
            const float speed = randomRange(1.1f, 4.9f);
            Particle p;
            p.position = Vec3(mFireBurstOrigin.x, 0.8f, mFireBurstOrigin.z);
            p.velocity = Vec3(std::cos(angle) * speed,
                              randomRange(1.0f, 3.2f),
                              std::sin(angle) * speed);
            p.age = 0.0f;
            p.life = randomRange(0.45f, 1.05f);
            p.r = randomRange(0.95f, 1.0f);
            p.g = randomRange(0.30f, 0.72f);
            p.b = randomRange(0.02f, 0.12f);
            mBurstParticles.push_back(p);
        }
        return true;
    }

    bool activateFireball(const Vec3& opponentPosition)
    {
        if (mFireballCooldown > 0.0f || mHealth <= 0.0f)
        {
            return false;
        }

        if (length2D(opponentPosition - mPosition) > fireballMaxRange())
        {
            return false;
        }

        faceOpponent(opponentPosition);
        mFireballActive = true;
        mFireballTimer = 0.0f;
        mFireballStart = mPosition;
        mFireballTarget = opponentPosition;
        mFireballDamagePending = true;
        mFireballCooldown = mFireballCooldownMax;
        return true;
    }

    void update(float dt)
    {
        BattleCharacter::update(dt);
        mParticleTimer += dt;

        if (mFireballCooldown > 0.0f)
        {
            mFireballCooldown = std::max(0.0f, mFireballCooldown - dt);
        }

        if (mFireBurstActive)
        {
            mFireBurstTimer += dt;
            for (std::size_t i = 0; i < mBurstParticles.size(); ++i)
            {
                Particle& p = mBurstParticles[i];
                p.age += dt;
                p.velocity.y -= 2.4f * dt;
                p.position = p.position + p.velocity * dt;
                if (p.position.y < 0.08f)
                {
                    p.position.y = 0.08f;
                    p.velocity.y *= -0.18f;
                }
            }

            if (mFireBurstTimer >= mFireBurstDuration)
            {
                mFireBurstActive = false;
                mBurstParticles.clear();
            }
        }

        if (mFireballActive)
        {
            mFireballTimer += dt;
            if (mFireballTimer >= mFireballDuration)
            {
                mFireballActive = false;
            }
        }
    }

    void draw(const TextureSet& textures) const
    {
        const float floatOffset = idleFloat() + walkBodyBob();

        glPushMatrix();
        beginCharacterTransform();

        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, textures.blaze);
        applyMaterial(characterMaterial(1.0f, 0.90f, 0.10f, 20.0f));

        glPushMatrix();
        glTranslatef(0.0f, 2.34f + floatOffset, 0.0f);
        PrimitiveRenderer::drawBox(1.18f, 1.08f, 1.18f);
        glDisable(GL_TEXTURE_2D);
        drawBlazeFace();
        drawBlazeSideHeadPanels();
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, textures.blaze);
        applyMaterial(characterMaterial(1.0f, 0.90f, 0.10f, 20.0f));
        glPopMatrix();

        glDisable(GL_TEXTURE_2D);
        drawBlazeCore(floatOffset);
        drawOrbitingRods();

        glBindTexture(GL_TEXTURE_2D, 0);
        glPopMatrix();

        drawFlameAura();
        drawSmokeColumn();
        drawFireBurstEffect();
        drawFireballChargeEffect();
        drawFireballEffect();
    }

    bool fireBurstDamageReady() const
    {
        return mFireBurstActive && mFireBurstDamagePending && mFireBurstTimer > 0.16f;
    }

    void markFireBurstDamageHandled()
    {
        mFireBurstDamagePending = false;
    }

    float fireBurstRadius() const
    {
        if (!mFireBurstActive)
        {
            return 0.0f;
        }
        const float t = clampFloat(mFireBurstTimer / mFireBurstDuration, 0.0f, 1.0f);
        return mFireBurstMaxRadius * std::sin(t * PI * 0.82f);
    }

    const Vec3& fireBurstOrigin() const
    {
        return mFireBurstOrigin;
    }

    bool fireballDamageReady() const
    {
        return mFireballActive &&
               mFireballDamagePending &&
               mFireballTimer >= fireballImpactTime();
    }

    void markFireballDamageHandled()
    {
        mFireballDamagePending = false;
    }

    const Vec3& fireballTarget() const
    {
        return mFireballTarget;
    }

    float fireballRadius() const
    {
        return 1.5f;
    }

    float fireballMaxRange() const
    {
        return 12.0f;
    }

    float fireballDamage(float distanceFromImpact) const
    {
        const float closeFactor =
            1.0f - clampFloat(distanceFromImpact / fireballRadius(), 0.0f, 1.0f);
        return 7.0f + closeFactor * 21.0f;
    }

    float fireballCooldownPercent() const
    {
        return clampFloat(mFireballCooldown / mFireballCooldownMax, 0.0f, 1.0f);
    }

    float fireballCooldownSeconds() const
    {
        return mFireballCooldown;
    }

    void resetForRound()
    {
        resetBattleState();
        mFireBurstActive = false;
        mFireBurstTimer = 0.0f;
        mFireBurstDamagePending = false;
        mBurstParticles.clear();
        mFireballActive = false;
        mFireballTimer = 0.0f;
        mFireballCooldown = 0.0f;
        mFireballDamagePending = false;
    }

private:
    float fireballLaunchDelay() const
    {
        return 0.24f;
    }

    float fireballImpactTime() const
    {
        return 0.82f;
    }

    void faceOpponent(const Vec3& opponentPosition)
    {
        const Vec3 face = normalize2D(opponentPosition - mPosition);
        if (length2D(face) > 0.0001f)
        {
            mFacingDegrees = std::atan2(face.x, face.z) * DEG_PER_RAD;
        }
    }

    float idleFloat() const
    {
        return std::sin(mParticleTimer * 2.1f) * 0.08f;
    }

    float walkBodyBob() const
    {
        if (!mMoving)
        {
            return 0.0f;
        }
        return std::fabs(std::sin(mWalkPhase * 2.0f)) * 0.035f;
    }

    void drawBlazeFace() const
    {
        glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);
        glDisable(GL_LIGHTING);
        const float z = 0.596f;

        enum BlazeFaceColor
        {
            BF_GOLD,
            BF_BRIGHT,
            BF_PALE,
            BF_AMBER,
            BF_BROWN,
            BF_DARK_BROWN,
            BF_BLACK,
            BF_EYE_WHITE,
            BF_DARK_RED,
            BF_DEEP_RED
        };

        static const int face[8][8] =
        {
            {BF_GOLD, BF_BRIGHT, BF_PALE, BF_AMBER, BF_BRIGHT, BF_GOLD, BF_PALE, BF_PALE},
            {BF_GOLD, BF_PALE, BF_BRIGHT, BF_BRIGHT, BF_BRIGHT, BF_GOLD, BF_PALE, BF_GOLD},
            {BF_BROWN, BF_BRIGHT, BF_GOLD, BF_BRIGHT, BF_BRIGHT, BF_GOLD, BF_GOLD, BF_BROWN},
            {BF_DARK_BROWN, BF_EYE_WHITE, BF_BLACK, BF_BRIGHT, BF_GOLD, BF_BLACK, BF_EYE_WHITE, BF_BROWN},
            {BF_DARK_BROWN, BF_GOLD, BF_AMBER, BF_BROWN, BF_AMBER, BF_BROWN, BF_AMBER, BF_AMBER},
            {BF_DARK_BROWN, BF_DARK_BROWN, BF_DARK_BROWN, BF_BLACK, BF_DEEP_RED, BF_BROWN, BF_DARK_BROWN, BF_DARK_BROWN},
            {BF_DARK_BROWN, BF_BLACK, BF_DARK_RED, BF_DEEP_RED, BF_BLACK, BF_DEEP_RED, BF_DARK_BROWN, BF_BLACK},
            {BF_BLACK, BF_DARK_RED, BF_BLACK, BF_DARK_RED, BF_DEEP_RED, BF_DARK_RED, BF_BLACK, BF_BLACK}
        };

        const float tile = 1.18f / 8.0f;
        const float start = -1.18f * 0.5f + tile * 0.5f;
        for (int row = 0; row < 8; ++row)
        {
            for (int col = 0; col < 8; ++col)
            {
                float r = 1.0f;
                float g = 0.90f;
                float b = 0.05f;
                switch (face[row][col])
                {
                    case BF_GOLD:
                        r = 0.96f; g = 0.72f; b = 0.03f;
                        break;
                    case BF_BRIGHT:
                        r = 1.0f; g = 0.86f; b = 0.02f;
                        break;
                    case BF_PALE:
                        r = 1.0f; g = 0.96f; b = 0.22f;
                        break;
                    case BF_AMBER:
                        r = 0.88f; g = 0.50f; b = 0.02f;
                        break;
                    case BF_BROWN:
                        r = 0.56f; g = 0.29f; b = 0.02f;
                        break;
                    case BF_DARK_BROWN:
                        r = 0.26f; g = 0.10f; b = 0.015f;
                        break;
                    case BF_BLACK:
                        r = 0.035f; g = 0.010f; b = 0.0f;
                        break;
                    case BF_EYE_WHITE:
                        r = 0.96f; g = 0.94f; b = 0.72f;
                        break;
                    case BF_DARK_RED:
                        r = 0.16f; g = 0.025f; b = 0.020f;
                        break;
                    case BF_DEEP_RED:
                        r = 0.32f; g = 0.0f; b = 0.0f;
                        break;
                    default:
                        break;
                }

                PrimitiveRenderer::drawFlatRect(start + col * tile,
                                                -start - row * tile,
                                                z + 0.001f + row * 0.0002f,
                                                tile * 1.01f,
                                                tile * 1.01f,
                                                r, g, b, 1.0f);
            }
        }

        glPopAttrib();
    }

    void blazeHeadColor(int code, float& r, float& g, float& b) const
    {
        switch (code)
        {
            case 0:
                r = 1.0f; g = 0.86f; b = 0.02f;
                break;
            case 1:
                r = 1.0f; g = 0.96f; b = 0.22f;
                break;
            case 2:
                r = 0.96f; g = 0.72f; b = 0.03f;
                break;
            case 3:
                r = 0.86f; g = 0.49f; b = 0.02f;
                break;
            case 4:
                r = 0.42f; g = 0.18f; b = 0.02f;
                break;
            case 5:
                r = 0.22f; g = 0.07f; b = 0.015f;
                break;
            case 6:
                r = 0.13f; g = 0.018f; b = 0.015f;
                break;
            default:
                r = 0.035f; g = 0.010f; b = 0.0f;
                break;
        }
    }

    void drawBlazeSidePanelFace(float zOffset) const
    {
        static const int side[8][8] =
        {
            {0, 1, 0, 2, 0, 1, 0, 1},
            {2, 1, 0, 0, 0, 2, 1, 3},
            {3, 0, 2, 0, 2, 2, 3, 3},
            {2, 1, 0, 2, 3, 2, 1, 3},
            {4, 2, 3, 4, 3, 4, 3, 4},
            {5, 4, 4, 6, 6, 4, 5, 5},
            {5, 7, 6, 6, 7, 6, 4, 7},
            {7, 5, 7, 6, 7, 5, 7, 7}
        };

        const float panelSize = 1.18f;
        const float tile = panelSize / 8.0f;
        const float start = -panelSize * 0.5f + tile * 0.5f;
        for (int row = 0; row < 8; ++row)
        {
            for (int col = 0; col < 8; ++col)
            {
                float r = 1.0f;
                float g = 1.0f;
                float b = 0.0f;
                blazeHeadColor(side[row][col], r, g, b);
                PrimitiveRenderer::drawFlatRect(start + col * tile,
                                                -start - row * tile,
                                                zOffset + row * 0.0002f,
                                                tile * 1.01f,
                                                tile * 1.01f,
                                                r, g, b, 1.0f);
            }
        }
    }

    void drawBlazeSideHeadPanels() const
    {
        glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);

        const float z = 0.600f;
        glPushMatrix();
        glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
        drawBlazeSidePanelFace(z);
        glPopMatrix();

        glPushMatrix();
        glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
        drawBlazeSidePanelFace(z);
        glPopMatrix();

        glPushMatrix();
        glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
        drawBlazeSidePanelFace(z);
        glPopMatrix();

        glPopAttrib();
    }

    void drawBlazeCore(float floatOffset) const
    {
        applyMaterial(characterMaterial(0.025f, 0.018f, 0.010f, 6.0f));
        glPushMatrix();
        glTranslatef(0.0f, 1.22f + floatOffset * 0.35f, 0.0f);
        PrimitiveRenderer::drawBox(0.44f, 1.22f, 0.44f);
        glPopMatrix();

        applyMaterial(characterMaterial(0.10f, 0.075f, 0.030f, 8.0f));
        glPushMatrix();
        glTranslatef(0.0f, 1.74f + floatOffset * 0.35f, 0.0f);
        PrimitiveRenderer::drawBox(0.54f, 0.18f, 0.54f);
        glPopMatrix();

        applyMaterial(characterMaterial(0.035f, 0.030f, 0.018f, 6.0f));
        glPushMatrix();
        glTranslatef(0.0f, 0.54f + floatOffset * 0.24f, 0.0f);
        PrimitiveRenderer::drawBox(0.34f, 0.34f, 0.34f);
        glPopMatrix();

        glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);
        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        const float z = 0.255f;
        const float y = floatOffset * 0.35f;
        PrimitiveRenderer::drawFlatRect(-0.17f, 1.36f + y, z, 0.08f, 0.24f, 1.0f, 0.88f, 0.04f, 0.42f);
        PrimitiveRenderer::drawFlatRect(0.18f, 1.08f + y, z, 0.10f, 0.18f, 1.0f, 0.82f, 0.03f, 0.34f);
        PrimitiveRenderer::drawFlatRect(0.02f, 0.72f + y, z, 0.15f, 0.08f, 1.0f, 0.76f, 0.02f, 0.28f);
        glPopAttrib();
    }

    void drawSegmentedRod(float height, int index) const
    {
        const int segments = 8;
        const float segmentHeight = height / static_cast<float>(segments);
        for (int segment = 0; segment < segments; ++segment)
        {
            const int colorIndex = (segment * 2 + index) % 5;
            if (colorIndex == 0)
            {
                applyMaterial(characterMaterial(1.0f, 0.98f, 0.16f, 18.0f));
            }
            else if (colorIndex == 1)
            {
                applyMaterial(characterMaterial(1.0f, 0.86f, 0.03f, 14.0f));
            }
            else if (colorIndex == 2)
            {
                applyMaterial(characterMaterial(0.92f, 0.58f, 0.02f, 10.0f));
            }
            else
            {
                applyMaterial(characterMaterial(1.0f, 0.93f, 0.08f, 16.0f));
            }

            glPushMatrix();
            glTranslatef(0.0f, -height * 0.5f + segmentHeight * (segment + 0.5f), 0.0f);
            PrimitiveRenderer::drawBox(0.24f, segmentHeight * 0.96f, 0.20f);
            glPopMatrix();
        }

        applyMaterial(characterMaterial(0.42f, 0.27f, 0.02f, 8.0f));
        glPushMatrix();
        glTranslatef(-0.095f, 0.0f, 0.045f);
        PrimitiveRenderer::drawBox(0.032f, height * 0.96f, 0.040f);
        glPopMatrix();
    }

    void drawBlazeRod(float x, float z, float y, float height,
                      float yawDegrees, float rollDegrees, int index) const
    {
        const float sway = std::sin(mParticleTimer * 1.6f + index) * 0.035f;
        const float lean = std::sin(mParticleTimer * 2.1f + index) * 5.0f;
        glPushMatrix();
        glTranslatef(x + sway, y, z);
        glRotatef(yawDegrees, 0.0f, 1.0f, 0.0f);
        glRotatef(rollDegrees + lean, 0.0f, 0.0f, 1.0f);
        drawSegmentedRod(height, index);
        glPopMatrix();
    }

    void drawOrbitingRods() const
    {
        const float celebrationSpeed = victoryAnimationActive() ? 3.4f : 1.0f;
        const float bobAmount = victoryAnimationActive() ? 0.18f : 0.05f;
        const float bob = std::sin(mParticleTimer * 2.0f * celebrationSpeed) * bobAmount;

        glPushMatrix();
        if (victoryAnimationActive())
        {
            glRotatef(victoryAnimationTime() * 260.0f, 0.0f, 1.0f, 0.0f);
            const float pulse = 1.0f + std::fabs(std::sin(victoryAnimationTime() * 5.5f)) * 0.12f;
            glScalef(pulse, 1.0f, pulse);
        }

        drawBlazeRod(-1.08f, 0.12f, 1.72f + bob, 1.18f, -8.0f, 2.0f, 0);
        drawBlazeRod(1.10f, 0.08f, 1.66f - bob, 1.18f, 8.0f, -2.0f, 1);
        drawBlazeRod(-0.86f, -0.32f, 1.18f - bob, 1.28f, 10.0f, -3.0f, 2);
        drawBlazeRod(0.88f, -0.32f, 1.12f + bob, 1.28f, -10.0f, 3.0f, 3);
        drawBlazeRod(-0.42f, 0.18f, 0.34f + bob, 1.20f, 4.0f, 1.0f, 4);
        drawBlazeRod(0.52f, 0.16f, 0.34f - bob, 1.20f, -4.0f, -1.0f, 5);
        drawBlazeRod(-1.28f, -0.10f, 0.92f + bob * 0.5f, 0.88f, 12.0f, 5.0f, 6);
        drawBlazeRod(1.30f, -0.08f, 0.92f - bob * 0.5f, 0.88f, -12.0f, -5.0f, 7);
        glPopMatrix();
    }

    void drawFlameAura() const
    {
        glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_DEPTH_BUFFER_BIT | GL_LINE_BIT | GL_POINT_BIT);
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glDepthMask(GL_FALSE);

        for (int ring = 0; ring < 3; ++ring)
        {
            const float y = 0.18f + ring * 0.72f;
            const float radius = 0.58f + ring * 0.20f +
                                 std::sin(mParticleTimer * 3.0f + ring) * 0.05f;
            glLineWidth(1.5f);
            glColor4f(1.0f, 0.82f + ring * 0.04f, 0.05f, 0.20f - ring * 0.035f);
            PrimitiveRenderer::drawCircleXZ(Vec3(mPosition.x, mPosition.y + y, mPosition.z),
                                            radius, 48);
        }

        glPointSize(6.0f);
        glBegin(GL_POINTS);
        for (int i = 0; i < 58; ++i)
        {
            const float seed = static_cast<float>(i);
            const float angle = seed * 2.15f + mParticleTimer * (1.3f + (i % 4) * 0.22f);
            const float radius = 0.28f + (i % 11) * 0.06f;
            const float height = 0.2f + (i % 14) * 0.16f +
                                 std::sin(mParticleTimer * 3.4f + seed) * 0.10f;
            const float alpha = 0.34f + (i % 5) * 0.06f;
            glColor4f(1.0f, 0.72f + (i % 5) * 0.05f, 0.03f, alpha * 0.82f);
            glVertex3f(mPosition.x + std::cos(angle) * radius,
                       mPosition.y + height,
                       mPosition.z + std::sin(angle) * radius);
        }
        glEnd();

        glDepthMask(GL_TRUE);
        glPopAttrib();
    }

    void drawSmokeColumn() const
    {
        glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_DEPTH_BUFFER_BIT);
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);

        for (int i = 0; i < 44; ++i)
        {
            const float seed = static_cast<float>(i);
            const float layer = static_cast<float>(i / 11);
            const float rise = 2.42f + (i % 11) * 0.20f + layer * 0.34f;
            const float spread = 0.34f + (rise - 2.35f) * 0.28f + (i % 3) * 0.08f;
            const float angle = seed * 2.07f + mParticleTimer * (0.22f + (i % 5) * 0.025f);
            const float sideDrift = std::sin(mParticleTimer * 1.0f + seed * 0.7f) * (0.18f + layer * 0.05f);
            const float x = mPosition.x + std::cos(angle) * spread + sideDrift;
            const float z = mPosition.z + std::sin(angle) * spread * 0.90f;
            const float size = 0.12f + (i % 6) * 0.040f + layer * 0.025f;
            const float shade = 0.05f + (i % 5) * 0.045f;
            const float alpha = 0.34f - layer * 0.035f;

            glColor4f(shade, shade, shade, alpha);
            glPushMatrix();
            glTranslatef(x, mPosition.y + rise, z);
            glRotatef(seed * 19.0f + mParticleTimer * 18.0f, 0.0f, 1.0f, 0.0f);
            PrimitiveRenderer::drawBox(size, size * (0.82f + (i % 3) * 0.10f), size);
            glPopMatrix();
        }

        for (int i = 0; i < 24; ++i)
        {
            const float seed = static_cast<float>(i);
            const float angle = seed * 2.41f - mParticleTimer * 0.35f;
            const float radius = 0.50f + (i % 8) * 0.12f;
            const float height = 0.12f + (i % 10) * 0.21f;
            const float size = 0.08f + (i % 5) * 0.032f;
            glColor4f(0.025f, 0.025f, 0.030f, 0.30f);
            glPushMatrix();
            glTranslatef(mPosition.x + std::cos(angle) * radius,
                         mPosition.y + height,
                         mPosition.z + std::sin(angle) * radius);
            glRotatef(seed * 27.0f, 1.0f, 1.0f, 0.0f);
            PrimitiveRenderer::drawBox(size, size * 1.25f, size);
            glPopMatrix();
        }

        glDepthMask(GL_TRUE);
        glPopAttrib();
    }

    void drawFireBurstEffect() const
    {
        if (!mFireBurstActive)
        {
            return;
        }

        glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_DEPTH_BUFFER_BIT | GL_LINE_BIT | GL_POINT_BIT);
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glDepthMask(GL_FALSE);

        const float radius = fireBurstRadius();
        const float fade = 1.0f - clampFloat(mFireBurstTimer / mFireBurstDuration, 0.0f, 1.0f);
        glLineWidth(3.0f);
        glColor4f(1.0f, 0.28f, 0.02f, 0.78f * fade);
        PrimitiveRenderer::drawCircleXZ(Vec3(mFireBurstOrigin.x, 0.10f, mFireBurstOrigin.z), radius, 96);
        glColor4f(1.0f, 0.86f, 0.12f, 0.45f * fade);
        PrimitiveRenderer::drawCircleXZ(Vec3(mFireBurstOrigin.x, 0.14f, mFireBurstOrigin.z), radius * 0.58f, 96);

        glPointSize(7.0f);
        glBegin(GL_POINTS);
        for (std::size_t i = 0; i < mBurstParticles.size(); ++i)
        {
            const Particle& p = mBurstParticles[i];
            if (p.age < p.life)
            {
                const float alpha = (1.0f - p.age / p.life) * fade;
                glColor4f(p.r, p.g, p.b, alpha);
                glVertex3f(p.position.x, p.position.y, p.position.z);
            }
        }
        glEnd();

        glDepthMask(GL_TRUE);
        glPopAttrib();
    }

    void drawFireballChargeEffect() const
    {
        if (!mFireballActive || mFireballTimer >= fireballLaunchDelay())
        {
            return;
        }

        const float charge =
            clampFloat(mFireballTimer / fireballLaunchDelay(), 0.0f, 1.0f);
        const Vec3 forward = facingDirection();
        const Vec3 center(mFireballStart.x + forward.x * 1.05f,
                          2.05f,
                          mFireballStart.z + forward.z * 1.05f);

        glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_DEPTH_BUFFER_BIT |
                     GL_POINT_BIT | GL_LINE_BIT);
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glDepthMask(GL_FALSE);

        glPointSize(9.0f);
        glBegin(GL_POINTS);
        for (int i = 0; i < 78; ++i)
        {
            const float seed = static_cast<float>(i);
            const float angle = seed * 2.21f - mParticleTimer * 7.0f;
            const float outerRadius = 1.65f + (i % 9) * 0.08f;
            const float radius = outerRadius * (1.0f - charge * 0.82f);
            const float vertical =
                std::sin(seed * 1.71f + mParticleTimer * 6.0f) *
                (0.85f * (1.0f - charge) + 0.10f);
            glColor4f(1.0f,
                      0.22f + (i % 5) * 0.13f,
                      0.01f,
                      0.42f + charge * 0.52f);
            glVertex3f(center.x + std::cos(angle) * radius,
                       center.y + vertical,
                       center.z + std::sin(angle) * radius);
        }
        glEnd();

        glLineWidth(3.0f);
        for (int ring = 0; ring < 3; ++ring)
        {
            const float radius =
                (1.1f - ring * 0.22f) * (1.0f - charge * 0.68f);
            glColor4f(1.0f, 0.45f + ring * 0.18f, 0.02f,
                      charge * (0.68f - ring * 0.12f));
            PrimitiveRenderer::drawCircleXZ(
                Vec3(center.x, center.y - 0.10f + ring * 0.10f, center.z),
                radius, 64);
        }

        glPointSize(18.0f + charge * 12.0f);
        glBegin(GL_POINTS);
        glColor4f(1.0f, 0.92f, 0.24f, 0.88f);
        glVertex3f(center.x, center.y, center.z);
        glEnd();

        glDepthMask(GL_TRUE);
        glPopAttrib();
    }

    void drawFireballEffect() const
    {
        if (!mFireballActive)
        {
            return;
        }

        glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_DEPTH_BUFFER_BIT | GL_LINE_BIT | GL_POINT_BIT);
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glDepthMask(GL_FALSE);

        const float launchDelay = fireballLaunchDelay();
        const float impactTime = fireballImpactTime();
        const float travelAge =
            clampFloat((mFireballTimer - launchDelay) /
                       (impactTime - launchDelay), 0.0f, 1.0f);
        const float explosionAge =
            clampFloat((mFireballTimer - impactTime) /
                       (mFireballDuration - impactTime), 0.0f, 1.0f);
        const Vec3 start(mFireballStart.x, 2.05f, mFireballStart.z);
        const Vec3 end(mFireballTarget.x, 0.35f, mFireballTarget.z);
        const float arcHeight = std::sin(travelAge * PI) * 1.35f;
        Vec3 current = start + (end - start) * travelAge;
        current.y += arcHeight;

        if (mFireballTimer < impactTime)
        {
            const float warningPulse = 0.72f + std::sin(mFireballTimer * 18.0f) * 0.18f;
            glLineWidth(3.0f);
            glColor4f(1.0f, 0.08f, 0.01f, warningPulse);
            PrimitiveRenderer::drawCircleXZ(
                Vec3(mFireballTarget.x, 0.095f, mFireballTarget.z),
                fireballRadius(), 96);
            glLineWidth(1.5f);
            glColor4f(1.0f, 0.58f, 0.02f, warningPulse * 0.78f);
            PrimitiveRenderer::drawCircleXZ(
                Vec3(mFireballTarget.x, 0.105f, mFireballTarget.z),
                fireballRadius() * (0.60f + travelAge * 0.30f), 72);

            glBegin(GL_LINES);
            for (int i = 0; i < 12; ++i)
            {
                const float angle = (2.0f * PI * i) / 12.0f;
                glColor4f(1.0f, 0.22f, 0.01f, warningPulse * 0.65f);
                glVertex3f(mFireballTarget.x + std::cos(angle) * fireballRadius() * 0.78f,
                           0.11f,
                           mFireballTarget.z + std::sin(angle) * fireballRadius() * 0.78f);
                glVertex3f(mFireballTarget.x + std::cos(angle) * fireballRadius(),
                           0.11f,
                           mFireballTarget.z + std::sin(angle) * fireballRadius());
            }
            glEnd();

            if (mFireballTimer >= launchDelay)
            {
                glLineWidth(5.0f);
                glBegin(GL_LINES);
                glColor4f(1.0f, 0.80f, 0.08f, 0.72f);
                glVertex3f(start.x, start.y, start.z);
                glColor4f(1.0f, 0.18f, 0.02f, 0.48f);
                glVertex3f(current.x, current.y, current.z);
                glEnd();

                glPointSize(10.0f);
                glBegin(GL_POINTS);
                for (int i = 0; i < 42; ++i)
                {
                    const float angle = i * 0.78f + mFireballTimer * 12.0f;
                    const float radius = 0.10f + (i % 8) * 0.075f;
                    const float height = (i % 5) * 0.075f;
                    glColor4f(1.0f, 0.38f + (i % 4) * 0.10f, 0.02f, 0.82f);
                    glVertex3f(current.x + std::cos(angle) * radius,
                               current.y + height,
                               current.z + std::sin(angle) * radius);
                }
                glEnd();
            }
        }
        else
        {
            const float fade = 1.0f - explosionAge;
            const float radius =
                fireballRadius() * std::sin(explosionAge * PI * 0.78f);

            glLineWidth(4.0f);
            glColor4f(1.0f, 0.18f, 0.01f, 0.82f * fade);
            PrimitiveRenderer::drawCircleXZ(
                Vec3(mFireballTarget.x, 0.10f, mFireballTarget.z),
                radius, 96);
            glColor4f(1.0f, 0.78f, 0.04f, 0.64f * fade);
            PrimitiveRenderer::drawCircleXZ(
                Vec3(mFireballTarget.x, 0.14f, mFireballTarget.z),
                radius * 0.72f, 96);
            glColor4f(1.0f, 1.0f, 0.72f, 0.48f * fade);
            PrimitiveRenderer::drawCircleXZ(
                Vec3(mFireballTarget.x, 0.18f, mFireballTarget.z),
                radius * 0.38f, 72);

            glPointSize(10.0f);
            glBegin(GL_POINTS);
            for (int i = 0; i < 120; ++i)
            {
                const float seed = static_cast<float>(i);
                const float angle = seed * 2.37f;
                const float spread =
                    radius * (0.18f + static_cast<float>(i % 12) * 0.075f);
                const float height =
                    0.14f + (i % 9) * 0.25f +
                    std::sin(explosionAge * 8.0f + seed) * 0.20f;

                if (i % 7 == 0)
                {
                    glColor4f(0.08f, 0.06f, 0.05f, 0.72f * fade);
                }
                else if (i % 3 == 0)
                {
                    glColor4f(1.0f, 0.92f, 0.42f, 0.88f * fade);
                }
                else
                {
                    glColor4f(1.0f, 0.20f + (i % 5) * 0.10f,
                              0.01f, 0.82f * fade);
                }

                glVertex3f(mFireballTarget.x + std::cos(angle) * spread,
                           height,
                           mFireballTarget.z + std::sin(angle) * spread);
            }
            glEnd();

            glPointSize(14.0f);
            glBegin(GL_POINTS);
            for (int i = 0; i < 34; ++i)
            {
                const float seed = static_cast<float>(i);
                const float angle = seed * 2.11f + explosionAge * 0.8f;
                const float spread = 0.25f + (i % 8) * 0.18f;
                const float rise = explosionAge * (1.2f + (i % 5) * 0.32f);
                const float smoke = 0.16f + (i % 4) * 0.06f;
                glColor4f(smoke, smoke, smoke, 0.58f * fade);
                glVertex3f(mFireballTarget.x + std::cos(angle) * spread,
                           0.35f + rise,
                           mFireballTarget.z + std::sin(angle) * spread);
            }
            glEnd();
        }

        glDepthMask(GL_TRUE);
        glPopAttrib();
    }

    bool mFireBurstActive;
    float mFireBurstTimer;
    float mFireBurstDuration;
    float mFireBurstMaxRadius;
    bool mFireBurstDamagePending;
    Vec3 mFireBurstOrigin;
    std::vector<Particle> mBurstParticles;
    bool mFireballActive;
    float mFireballTimer;
    float mFireballDuration;
    float mFireballCooldown;
    float mFireballCooldownMax;
    bool mFireballDamagePending;
    Vec3 mFireballStart;
    Vec3 mFireballTarget;
    float mParticleTimer;
};

class Battlefield
{
public:
    Battlefield()
        : mRedFlash(0.0f),
          mPurplePulse(0.0f),
          mGreenPulse(0.0f),
          mCreeperChargeTimer(0.0f),
          mCreeperImpactTimer(0.0f),
          mCreeperSmokeTimer(0.0f),
          mCraterTimer(0.0f),
          mFireZoneTimer(0.0f),
          mVoidStormTimer(0.0f),
          mLowHealthDarkness(0.0f),
          mLowHealthTarget(0.0f),
          mEnvironmentTime(0.0f)
    {
    }

    void triggerExplosionFlash(const Vec3& origin)
    {
        mCraterOrigin = origin;
        mCreeperChargeTimer = 0.28f;
    }

    void triggerCreeperImpact(const Vec3& origin)
    {
        mCraterOrigin = origin;
        mCreeperChargeTimer = 0.0f;
        mCreeperImpactTimer = 0.16f;
        mCreeperSmokeTimer = 3.0f;
        mCraterTimer = 7.0f;
    }

    void triggerTeleportPulse()
    {
        mPurplePulse = 1.0f;
        mVoidStormTimer = 3.2f;
    }

    void triggerBlazeFire(const Vec3& origin)
    {
        mRedFlash = 1.0f;
        mFireZoneOrigin = origin;
        mFireZoneTimer = 6.5f;
    }

    void triggerPowderCloudPulse()
    {
        mGreenPulse = 1.0f;
    }

    void setLowHealth(float firstHealth, float secondHealth)
    {
        const float lowestHealth = std::min(firstHealth, secondHealth);
        mLowHealthTarget = clampFloat((35.0f - lowestHealth) / 35.0f, 0.0f, 1.0f);
    }

    void reset()
    {
        mRedFlash = 0.0f;
        mPurplePulse = 0.0f;
        mGreenPulse = 0.0f;
        mCreeperChargeTimer = 0.0f;
        mCreeperImpactTimer = 0.0f;
        mCreeperSmokeTimer = 0.0f;
        mCraterTimer = 0.0f;
        mFireZoneTimer = 0.0f;
        mVoidStormTimer = 0.0f;
        mLowHealthDarkness = 0.0f;
        mLowHealthTarget = 0.0f;
        mEnvironmentTime = 0.0f;
    }

    void update(float dt)
    {
        mRedFlash = std::max(0.0f, mRedFlash - dt * 1.25f);
        mPurplePulse = std::max(0.0f, mPurplePulse - dt * 1.55f);
        mGreenPulse = std::max(0.0f, mGreenPulse - dt * 1.15f);
        mCreeperChargeTimer = std::max(0.0f, mCreeperChargeTimer - dt);
        mCreeperImpactTimer = std::max(0.0f, mCreeperImpactTimer - dt);
        mCreeperSmokeTimer = std::max(0.0f, mCreeperSmokeTimer - dt);
        mCraterTimer = std::max(0.0f, mCraterTimer - dt);
        mFireZoneTimer = std::max(0.0f, mFireZoneTimer - dt);
        mVoidStormTimer = std::max(0.0f, mVoidStormTimer - dt);
        mEnvironmentTime += dt;

        const float darkenSpeed = (mLowHealthTarget > mLowHealthDarkness) ? 1.8f : 0.9f;
        const float step = darkenSpeed * dt;
        if (mLowHealthDarkness < mLowHealthTarget)
        {
            mLowHealthDarkness = std::min(mLowHealthTarget, mLowHealthDarkness + step);
        }
        else
        {
            mLowHealthDarkness = std::max(mLowHealthTarget, mLowHealthDarkness - step);
        }
    }

    void applyLighting() const
    {
        const float red = mRedFlash;
        const float purple = mPurplePulse;
        const float green = mGreenPulse;
        const float fire = fireZoneStrength();
        const float voidStorm = voidStormStrength();
        const float charge = creeperChargeStrength();
        const float impact = creeperImpactStrength();
        const float smoke = creeperSmokeStrength();
        const float darkness = mLowHealthDarkness;
        const float lightScale = (1.0f - darkness * 0.58f) * (1.0f - smoke * 0.32f);

        GLfloat ambient[] =
        {
            (0.22f + red * 0.35f + purple * 0.12f + fire * 0.20f +
             charge * 0.18f + impact * 0.70f) * lightScale,
            (0.24f - red * 0.10f + green * 0.18f - voidStorm * 0.10f +
             charge * 0.42f + impact * 0.78f + smoke * 0.12f) * lightScale,
            (0.28f + purple * 0.26f + voidStorm * 0.12f +
             impact * 0.58f - smoke * 0.08f) * lightScale,
            1.0f
        };
        GLfloat diffuse[] =
        {
            (0.82f + red * 0.60f + fire * 0.42f +
             charge * 0.18f + impact * 0.65f) * lightScale,
            (0.86f - red * 0.25f + green * 0.35f - fire * 0.18f +
             charge * 0.55f + impact * 0.85f + smoke * 0.12f) * lightScale,
            (0.90f + purple * 0.42f + voidStorm * 0.20f +
             impact * 0.52f - smoke * 0.12f) * lightScale,
            1.0f
        };
        GLfloat specular[] = {0.8f, 0.8f, 0.85f, 1.0f};
        GLfloat position[] = {-3.0f, 8.0f, 5.0f, 1.0f};

        glLightfv(GL_LIGHT0, GL_POSITION, position);
        glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
        glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
    }

    void clearSky() const
    {
        const float red = mRedFlash;
        const float purple = mPurplePulse;
        const float green = mGreenPulse;
        const float fire = fireZoneStrength();
        const float voidStorm = voidStormStrength();
        const float charge = creeperChargeStrength();
        const float impact = creeperImpactStrength();
        const float smoke = creeperSmokeStrength();
        const float darkness = mLowHealthDarkness;
        const float skyScale =
            (1.0f - darkness * 0.62f) * (1.0f - smoke * 0.38f);
        glClearColor((0.42f + red * 0.38f + purple * 0.08f + fire * 0.24f +
                      charge * 0.12f + impact * 0.62f + smoke * 0.08f) * skyScale,
                     (0.62f - red * 0.28f + green * 0.08f - fire * 0.20f -
                      voidStorm * 0.20f + charge * 0.34f + impact * 0.72f +
                      smoke * 0.18f) * skyScale,
                     (0.88f - red * 0.45f + purple * 0.10f - fire * 0.34f +
                      voidStorm * 0.06f + impact * 0.50f - smoke * 0.16f) * skyScale,
                     1.0f);
    }

    void draw(const TextureSet& textures) const
    {
        drawSkyBox();
        drawGround(textures.floor);
        drawEnvironmentEffects();
        drawArenaMarkings();
        drawWoodenFence();
        drawSideHills();
        drawStoneBlocks();
        drawGridReaction();
        drawBlindBoxCollection(textures.blindBox);
    }

private:
    void drawSkyBox() const
    {
        const float red = mRedFlash;
        const float purple = mPurplePulse;
        const float green = mGreenPulse;
        const float fire = fireZoneStrength();
        const float voidStorm = voidStormStrength();
        const float charge = creeperChargeStrength();
        const float impact = creeperImpactStrength();
        const float smoke = creeperSmokeStrength();
        const float darkness = mLowHealthDarkness;
        const float skyScale =
            (1.0f - darkness * 0.60f) * (1.0f - smoke * 0.36f);

        glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_DEPTH_BUFFER_BIT);
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glDepthMask(GL_FALSE);

        const float size = 28.0f;
        const float topR = (0.30f + red * 0.45f + purple * 0.10f + fire * 0.35f +
                            charge * 0.10f + impact * 0.68f + smoke * 0.06f) * skyScale;
        const float topG = (0.52f - red * 0.23f + green * 0.07f - fire * 0.22f -
                            voidStorm * 0.22f + charge * 0.34f + impact * 0.78f +
                            smoke * 0.18f) * skyScale;
        const float topB = (0.90f - red * 0.46f + purple * 0.12f - fire * 0.42f +
                            voidStorm * 0.08f + impact * 0.54f - smoke * 0.18f) * skyScale;
        const float lowR = (0.65f + red * 0.30f + purple * 0.08f + fire * 0.25f +
                            charge * 0.14f + impact * 0.62f + smoke * 0.08f) * skyScale;
        const float lowG = (0.80f - red * 0.30f + green * 0.08f - fire * 0.22f -
                            voidStorm * 0.18f + charge * 0.38f + impact * 0.72f +
                            smoke * 0.20f) * skyScale;
        const float lowB = (0.98f - red * 0.45f + purple * 0.05f - fire * 0.34f +
                            voidStorm * 0.05f + impact * 0.56f - smoke * 0.16f) * skyScale;

        glBegin(GL_QUADS);
        glColor3f(lowR, lowG, lowB);
        glVertex3f(-size, 0.0f, -size);
        glVertex3f(size, 0.0f, -size);
        glColor3f(topR, topG, topB);
        glVertex3f(size, size, -size);
        glVertex3f(-size, size, -size);

        glColor3f(lowR, lowG, lowB);
        glVertex3f(size, 0.0f, -size);
        glVertex3f(size, 0.0f, size);
        glColor3f(topR, topG, topB);
        glVertex3f(size, size, size);
        glVertex3f(size, size, -size);

        glColor3f(lowR, lowG, lowB);
        glVertex3f(size, 0.0f, size);
        glVertex3f(-size, 0.0f, size);
        glColor3f(topR, topG, topB);
        glVertex3f(-size, size, size);
        glVertex3f(size, size, size);

        glColor3f(lowR, lowG, lowB);
        glVertex3f(-size, 0.0f, size);
        glVertex3f(-size, 0.0f, -size);
        glColor3f(topR, topG, topB);
        glVertex3f(-size, size, -size);
        glVertex3f(-size, size, size);

        glColor3f(topR, topG, topB);
        glVertex3f(-size, size, -size);
        glVertex3f(size, size, -size);
        glVertex3f(size, size, size);
        glVertex3f(-size, size, size);
        glEnd();

        glDepthMask(GL_TRUE);
        glPopAttrib();
    }

    float fireZoneStrength() const
    {
        return clampFloat(mFireZoneTimer / 2.0f, 0.0f, 1.0f);
    }

    float voidStormStrength() const
    {
        return clampFloat(mVoidStormTimer / 1.2f, 0.0f, 1.0f);
    }

    float creeperChargeStrength() const
    {
        if (mCreeperChargeTimer <= 0.0f)
        {
            return 0.0f;
        }
        const float flicker =
            0.45f + 0.55f * std::fabs(std::sin(mEnvironmentTime * 34.0f));
        return clampFloat(mCreeperChargeTimer / 0.28f, 0.0f, 1.0f) * flicker;
    }

    float creeperImpactStrength() const
    {
        return clampFloat(mCreeperImpactTimer / 0.16f, 0.0f, 1.0f);
    }

    float creeperSmokeStrength() const
    {
        return clampFloat(mCreeperSmokeTimer / 3.0f, 0.0f, 1.0f);
    }

    void drawFilledGroundCircle(const Vec3& center, float radius,
                                float r, float g, float b, float alpha,
                                int segments) const
    {
        glColor4f(r, g, b, alpha);
        glBegin(GL_TRIANGLE_FAN);
        glVertex3f(center.x, center.y, center.z);
        for (int i = 0; i <= segments; ++i)
        {
            const float angle = (2.0f * PI * i) / static_cast<float>(segments);
            glVertex3f(center.x + std::cos(angle) * radius,
                       center.y,
                       center.z + std::sin(angle) * radius);
        }
        glEnd();
    }

    void drawJaggedGroundPatch(const Vec3& center, float radius,
                               float r, float g, float b, float alpha,
                               int segments, float phase) const
    {
        glColor4f(r, g, b, alpha);
        glBegin(GL_TRIANGLE_FAN);
        glVertex3f(center.x, center.y, center.z);
        for (int i = 0; i <= segments; ++i)
        {
            const float angle = (2.0f * PI * i) / static_cast<float>(segments);
            const float irregularity =
                0.82f + 0.13f * std::sin(i * 2.71f + phase) +
                0.08f * std::sin(i * 5.13f - phase);
            const float edgeRadius = radius * irregularity;
            glVertex3f(center.x + std::cos(angle) * edgeRadius,
                       center.y,
                       center.z + std::sin(angle) * edgeRadius);
        }
        glEnd();
    }

    void drawEnvironmentEffects() const
    {
        if (mCraterTimer <= 0.0f && mFireZoneTimer <= 0.0f && mVoidStormTimer <= 0.0f)
        {
            return;
        }

        glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_DEPTH_BUFFER_BIT |
                     GL_LINE_BIT | GL_POINT_BIT);
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);

        if (mCraterTimer > 0.0f)
        {
            const float fade = clampFloat(mCraterTimer / 2.0f, 0.0f, 1.0f);
            drawJaggedGroundPatch(Vec3(mCraterOrigin.x, 0.066f, mCraterOrigin.z),
                                  2.75f, 0.018f, 0.014f, 0.010f,
                                  0.82f * fade, 36, 0.4f);
            drawJaggedGroundPatch(Vec3(mCraterOrigin.x, 0.070f, mCraterOrigin.z),
                                  2.10f, 0.075f, 0.055f, 0.035f,
                                  0.76f * fade, 32, 1.7f);
            drawJaggedGroundPatch(Vec3(mCraterOrigin.x, 0.074f, mCraterOrigin.z),
                                  1.15f, 0.010f, 0.008f, 0.006f,
                                  0.88f * fade, 24, 2.8f);

            glLineWidth(3.5f);
            glColor4f(0.012f, 0.008f, 0.005f, 0.98f * fade);
            for (int crack = 0; crack < 16; ++crack)
            {
                const float angle = crack * 2.17f + (crack % 3) * 0.11f;
                const float length = 1.45f + (crack % 5) * 0.36f;
                const float bend = ((crack % 2) == 0) ? 0.16f : -0.14f;
                const float midX =
                    mCraterOrigin.x + std::cos(angle + bend) * length * 0.48f;
                const float midZ =
                    mCraterOrigin.z + std::sin(angle + bend) * length * 0.48f;
                const float endX = mCraterOrigin.x + std::cos(angle) * length;
                const float endZ = mCraterOrigin.z + std::sin(angle) * length;

                glBegin(GL_LINE_STRIP);
                glVertex3f(mCraterOrigin.x + std::cos(angle) * 0.45f,
                           0.080f,
                           mCraterOrigin.z + std::sin(angle) * 0.45f);
                glVertex3f(midX, 0.082f, midZ);
                glVertex3f(endX, 0.084f, endZ);
                glEnd();

                if (crack % 2 == 0)
                {
                    const float branchAngle = angle + ((crack % 4 == 0) ? 0.72f : -0.66f);
                    const float branchLength = 0.42f + (crack % 3) * 0.18f;
                    glLineWidth(2.0f);
                    glBegin(GL_LINE_STRIP);
                    glVertex3f(midX, 0.085f, midZ);
                    glVertex3f(midX + std::cos(branchAngle) * branchLength,
                               0.086f,
                               midZ + std::sin(branchAngle) * branchLength);
                    glEnd();
                    glLineWidth(3.5f);
                }
            }

            glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);
            glDisable(GL_LIGHTING);
            for (int i = 0; i < 26; ++i)
            {
                const float angle = i * 2.39f;
                const float radius = 0.55f + (i % 7) * 0.30f;
                const float size = 0.055f + (i % 4) * 0.025f;
                const float height =
                    0.09f + std::fabs(std::sin(mEnvironmentTime * 1.8f + i)) *
                    0.08f * fade;
                const float shade = 0.035f + (i % 5) * 0.018f;
                glColor4f(shade, shade * 0.78f, shade * 0.55f, 0.92f * fade);
                glPushMatrix();
                glTranslatef(mCraterOrigin.x + std::cos(angle) * radius,
                             height,
                             mCraterOrigin.z + std::sin(angle) * radius);
                glRotatef(static_cast<float>(i * 31), 0.3f, 1.0f, 0.2f);
                PrimitiveRenderer::drawBox(size * 1.4f, size, size * 1.2f);
                glPopMatrix();
            }
            glPopAttrib();

            glPointSize(15.0f);
            glBegin(GL_POINTS);
            for (int i = 0; i < 46; ++i)
            {
                const float seed = static_cast<float>(i);
                const float angle = seed * 2.43f + mEnvironmentTime * 0.18f;
                const float radius = 0.24f + (i % 10) * 0.17f;
                const float rise =
                    std::fmod(mEnvironmentTime * (0.26f + (i % 5) * 0.045f) +
                              seed * 0.17f, 2.2f);
                const float smoke = 0.12f + (i % 5) * 0.055f;
                glColor4f(smoke, smoke, smoke,
                          std::max(0.04f, 0.42f - rise * 0.14f) * fade);
                glVertex3f(mCraterOrigin.x + std::cos(angle) * radius,
                           0.20f + rise,
                           mCraterOrigin.z + std::sin(angle) * radius);
            }
            glEnd();
        }

        if (mFireZoneTimer > 0.0f)
        {
            const float fade = clampFloat(mFireZoneTimer / 1.5f, 0.0f, 1.0f);
            const float pulse = 0.92f + std::sin(mEnvironmentTime * 9.0f) * 0.08f;
            drawFilledGroundCircle(Vec3(mFireZoneOrigin.x, 0.072f, mFireZoneOrigin.z),
                                   2.65f * pulse, 0.30f, 0.035f, 0.005f, 0.42f * fade, 64);
            drawFilledGroundCircle(Vec3(mFireZoneOrigin.x, 0.075f, mFireZoneOrigin.z),
                                   1.65f * pulse, 1.0f, 0.22f, 0.01f, 0.30f * fade, 48);

            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            glPointSize(10.0f);
            glBegin(GL_POINTS);
            for (int i = 0; i < 72; ++i)
            {
                const float seed = static_cast<float>(i);
                const float angle = seed * 2.09f;
                const float radius = 0.25f + (i % 12) * 0.19f;
                const float flameCycle = std::fmod(mEnvironmentTime * (1.8f + (i % 5) * 0.22f) +
                                                   seed * 0.13f, 1.0f);
                const float height = 0.12f + flameCycle * (0.65f + (i % 4) * 0.20f);
                glColor4f(1.0f,
                          0.16f + (1.0f - flameCycle) * 0.55f,
                          0.01f,
                          (1.0f - flameCycle) * 0.80f * fade);
                glVertex3f(mFireZoneOrigin.x + std::cos(angle) * radius,
                           height,
                           mFireZoneOrigin.z + std::sin(angle) * radius);
            }
            glEnd();
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }

        if (mVoidStormTimer > 0.0f)
        {
            const float strength = voidStormStrength();
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            glLineWidth(2.0f);
            for (int ring = 0; ring < 6; ++ring)
            {
                const float radius = 2.0f + ring * 1.75f +
                                     std::sin(mEnvironmentTime * 4.0f + ring) * 0.28f;
                glColor4f(0.58f, 0.05f, 1.0f, strength * (0.25f - ring * 0.025f));
                PrimitiveRenderer::drawCircleXZ(Vec3(0.0f, 0.085f + ring * 0.003f, 0.0f),
                                                radius, 96);
            }

            glPointSize(6.0f);
            glBegin(GL_POINTS);
            for (int i = 0; i < 90; ++i)
            {
                const float seed = static_cast<float>(i);
                const float x = -10.5f + static_cast<float>((i * 47) % 210) * 0.10f;
                const float z = -10.5f + static_cast<float>((i * 71) % 210) * 0.10f;
                const float flicker = 0.45f + std::sin(mEnvironmentTime * 12.0f + seed) * 0.35f;
                glColor4f(0.75f, 0.12f, 1.0f, strength * flicker);
                glVertex3f(x, 0.12f + (i % 7) * 0.08f, z);
            }
            glEnd();
        }

        glDepthMask(GL_TRUE);
        glPopAttrib();
    }

    void drawGround(GLuint floorTexture) const
    {
        glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);
        glDisable(GL_CULL_FACE);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, floorTexture);
        applyMaterial(makeMaterial(0.48f, 0.90f, 0.34f, 10.0f));

        const float s = ARENA_HALF_SIZE;
        glBegin(GL_QUADS);
        glNormal3f(0.0f, 1.0f, 0.0f);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(-s, 0.0f, -s);
        glTexCoord2f(0.0f, 20.0f);
        glVertex3f(-s, 0.0f, s);
        glTexCoord2f(20.0f, 20.0f);
        glVertex3f(s, 0.0f, s);
        glTexCoord2f(20.0f, 0.0f);
        glVertex3f(s, 0.0f, -s);
        glEnd();

        glBindTexture(GL_TEXTURE_2D, 0);
        glDisable(GL_TEXTURE_2D);

        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glLineWidth(1.0f);
        glColor4f(0.12f, 0.48f, 0.10f, 0.32f);
        glBegin(GL_LINES);
        for (int i = 0; i < 150; ++i)
        {
            const float x = -11.7f + static_cast<float>((i * 37) % 234) * 0.10f;
            const float z = -11.7f + static_cast<float>((i * 53) % 234) * 0.10f;
            const float lean = (i % 5 - 2) * 0.018f;
            glVertex3f(x, 0.035f, z);
            glVertex3f(x + lean, 0.04f, z + 0.12f);
        }
        glEnd();

        glPopAttrib();
    }

    void drawArenaMarkings() const
    {
        glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_LINE_BIT);
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glLineWidth(2.0f);
        glColor4f(0.92f, 0.96f, 0.78f, 0.72f);
        PrimitiveRenderer::drawCircleXZ(Vec3(0.0f, 0.055f, 0.0f), 4.8f, 96);

        glLineWidth(1.5f);
        glBegin(GL_LINE_LOOP);
        glVertex3f(-10.3f, 0.06f, -10.3f);
        glVertex3f(10.3f, 0.06f, -10.3f);
        glVertex3f(10.3f, 0.06f, 10.3f);
        glVertex3f(-10.3f, 0.06f, 10.3f);
        glEnd();

        glColor4f(0.92f, 0.96f, 0.78f, 0.42f);
        glBegin(GL_LINES);
        glVertex3f(-10.3f, 0.06f, 0.0f);
        glVertex3f(10.3f, 0.06f, 0.0f);
        glVertex3f(0.0f, 0.06f, -10.3f);
        glVertex3f(0.0f, 0.06f, 10.3f);
        glEnd();

        glPopAttrib();
    }

    void drawWoodenFence() const
    {
        glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);
        glDisable(GL_TEXTURE_2D);
        applyMaterial(makeMaterial(0.42f, 0.25f, 0.11f, 12.0f));

        for (int i = -11; i <= 11; i += 2)
        {
            drawFencePost(static_cast<float>(i), -11.45f);
            drawFencePost(static_cast<float>(i), 11.45f);
            drawFencePost(-11.45f, static_cast<float>(i));
            drawFencePost(11.45f, static_cast<float>(i));
        }

        drawFenceRail(0.0f, -11.45f, 22.8f, 0.16f);
        drawFenceRail(0.0f, 11.45f, 22.8f, 0.16f);
        drawFenceRail(-11.45f, 0.0f, 0.16f, 22.8f);
        drawFenceRail(11.45f, 0.0f, 0.16f, 22.8f);

        glPopAttrib();
    }

    void drawSideHills() const
    {
        glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);
        glDisable(GL_TEXTURE_2D);

        drawBlockHill(-9.6f, -5.4f, 3, 3.40f, 2.5f);
        drawBlockHill(-9.8f, 6.0f, 2, 2.80f, 2.1f);
        drawBlockHill(9.6f, -6.2f, 3, 3.30f, 2.6f);
        drawBlockHill(9.9f, 5.1f, 2, 2.70f, 2.0f);
        drawBlockHill(-2.5f, 10.7f, 2, 3.00f, 1.8f);
        drawBlockHill(3.6f, -10.6f, 2, 2.80f, 1.8f);

        glPopAttrib();
    }

    void drawStoneBlocks() const
    {
        const Vec3 rocks[8] =
        {
            Vec3(-7.8f, 0.0f, -9.4f),
            Vec3(-9.8f, 0.0f, -1.8f),
            Vec3(-8.0f, 0.0f, 8.9f),
            Vec3(8.6f, 0.0f, -8.7f),
            Vec3(9.7f, 0.0f, 1.5f),
            Vec3(7.4f, 0.0f, 9.1f),
            Vec3(-1.8f, 0.0f, 10.3f),
            Vec3(2.2f, 0.0f, -10.5f)
        };

        glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);
        glDisable(GL_TEXTURE_2D);
        applyMaterial(makeMaterial(0.34f, 0.36f, 0.34f, 18.0f));

        for (int i = 0; i < 8; ++i)
        {
            const float size = 0.34f + (i % 3) * 0.08f;
            glPushMatrix();
            glTranslatef(rocks[i].x, size * 0.42f, rocks[i].z);
            glRotatef(static_cast<float>(i * 23), 0.0f, 1.0f, 0.0f);
            PrimitiveRenderer::drawBox(size * 1.25f, size * 0.85f, size);
            glPopMatrix();
        }

        glPopAttrib();
    }

    void drawFencePost(float x, float z) const
    {
        glPushMatrix();
        glTranslatef(x, 0.42f, z);
        PrimitiveRenderer::drawBox(0.20f, 0.84f, 0.20f);
        glPopMatrix();
    }

    void drawFenceRail(float x, float z, float width, float depth) const
    {
        glPushMatrix();
        glTranslatef(x, 0.58f, z);
        PrimitiveRenderer::drawBox(width, 0.16f, depth);
        glPopMatrix();
    }

    void drawBlockHill(float x, float z, int layers, float width, float depth) const
    {
        for (int layer = 0; layer < layers; ++layer)
        {
            const float shrink = static_cast<float>(layer) * 0.55f;
            const float blockWidth = std::max(0.70f, width - shrink);
            const float blockDepth = std::max(0.70f, depth - shrink * 0.8f);
            const float y = 0.18f + layer * 0.36f;

            applyMaterial(makeMaterial(0.36f, 0.22f, 0.10f, 8.0f));
            glPushMatrix();
            glTranslatef(x, y, z);
            PrimitiveRenderer::drawBox(blockWidth, 0.36f, blockDepth);
            glPopMatrix();

            applyMaterial(makeMaterial(0.22f, 0.62f, 0.20f, 10.0f));
            glPushMatrix();
            glTranslatef(x, y + 0.22f, z);
            PrimitiveRenderer::drawBox(blockWidth + 0.04f, 0.08f, blockDepth + 0.04f);
            glPopMatrix();
        }
    }

    void drawGridReaction() const
    {
        if (mPurplePulse <= 0.0f && mRedFlash <= 0.0f && mGreenPulse <= 0.0f &&
            mCreeperChargeTimer <= 0.0f && mCreeperImpactTimer <= 0.0f)
        {
            return;
        }

        glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_LINE_BIT);
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);

        for (int ring = 0; ring < 4; ++ring)
        {
            const float radius = 2.0f + ring * 1.35f + (1.0f - mPurplePulse) * 1.2f;
            glLineWidth(1.5f + ring * 0.2f);
            glColor4f(0.52f, 0.12f, 1.0f, mPurplePulse * (0.55f - ring * 0.08f));
            PrimitiveRenderer::drawCircleXZ(Vec3(0.0f, 0.06f + ring * 0.01f, 0.0f),
                                            radius, 96);
        }

        for (int ring = 0; ring < 3; ++ring)
        {
            const float radius = 1.4f + ring * 2.0f + (1.0f - mRedFlash) * 2.2f;
            glLineWidth(2.0f);
            glColor4f(1.0f, 0.1f, 0.02f, mRedFlash * (0.50f - ring * 0.1f));
            PrimitiveRenderer::drawCircleXZ(Vec3(0.0f, 0.07f + ring * 0.01f, 0.0f),
                                            radius, 96);
        }

        for (int ring = 0; ring < 3; ++ring)
        {
            const float radius = 1.0f + ring * 1.7f + (1.0f - mGreenPulse) * 1.4f;
            glLineWidth(1.5f);
            glColor4f(0.1f, 1.0f, 0.22f, mGreenPulse * (0.44f - ring * 0.09f));
            PrimitiveRenderer::drawCircleXZ(Vec3(0.0f, 0.09f + ring * 0.01f, 0.0f),
                                            radius, 96);
        }

        const float charge = creeperChargeStrength();
        if (charge > 0.0f)
        {
            for (int ring = 0; ring < 3; ++ring)
            {
                const float radius = 0.65f + ring * 0.48f + (1.0f - charge) * 0.30f;
                glLineWidth(2.0f + ring * 0.5f);
                glColor4f(0.55f, 1.0f, 0.18f,
                          charge * (0.68f - ring * 0.14f));
                PrimitiveRenderer::drawCircleXZ(
                    Vec3(mCraterOrigin.x, 0.11f + ring * 0.01f, mCraterOrigin.z),
                    radius, 72);
            }
        }

        const float impact = creeperImpactStrength();
        if (impact > 0.0f)
        {
            for (int ring = 0; ring < 5; ++ring)
            {
                const float radius =
                    0.8f + ring * 0.72f + (1.0f - impact) * 2.4f;
                glLineWidth(3.5f - ring * 0.35f);
                if (ring == 0)
                {
                    glColor4f(1.0f, 1.0f, 0.82f, impact * 0.92f);
                }
                else
                {
                    glColor4f(0.48f, 1.0f, 0.08f,
                              impact * (0.72f - ring * 0.10f));
                }
                PrimitiveRenderer::drawCircleXZ(
                    Vec3(mCraterOrigin.x, 0.13f + ring * 0.01f, mCraterOrigin.z),
                    radius, 96);
            }
        }

        glPopAttrib();
    }

    void drawQuestionMark(float zOffset) const
    {
        glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glColor3f(0.12f, 0.06f, 0.18f);
        glLineWidth(3.0f);
        glBegin(GL_LINE_STRIP);
        glVertex3f(-0.13f, 0.12f, zOffset);
        glVertex3f(-0.03f, 0.22f, zOffset);
        glVertex3f(0.13f, 0.16f, zOffset);
        glVertex3f(0.11f, 0.02f, zOffset);
        glVertex3f(0.0f, -0.04f, zOffset);
        glVertex3f(0.0f, -0.14f, zOffset);
        glEnd();
        glBegin(GL_POINTS);
        glVertex3f(0.0f, -0.27f, zOffset);
        glEnd();
        glPopAttrib();
    }

    void drawBlindBoxCollection(GLuint texture) const
    {
        const Vec3 positions[4] =
        {
            Vec3(-10.9f, 0.45f, -10.9f),
            Vec3(10.9f, 0.45f, -10.9f),
            Vec3(-10.9f, 0.45f, 10.9f),
            Vec3(10.9f, 0.45f, 10.9f)
        };

        glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texture);
        applyMaterial(makeMaterial(0.95f, 0.72f, 0.32f, 14.0f));

        for (int i = 0; i < 4; ++i)
        {
            glPushMatrix();
            glTranslatef(positions[i].x, positions[i].y, positions[i].z);
            glRotatef(45.0f + i * 90.0f, 0.0f, 1.0f, 0.0f);
            PrimitiveRenderer::drawBox(0.78f, 0.78f, 0.78f);
            glDisable(GL_TEXTURE_2D);
            drawQuestionMark(0.395f);
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, texture);
            glPopMatrix();
        }

        glBindTexture(GL_TEXTURE_2D, 0);
        glDisable(GL_TEXTURE_2D);
        glPopAttrib();
    }

    float mRedFlash;
    float mPurplePulse;
    float mGreenPulse;
    float mCreeperChargeTimer;
    float mCreeperImpactTimer;
    float mCreeperSmokeTimer;
    Vec3 mCraterOrigin;
    float mCraterTimer;
    Vec3 mFireZoneOrigin;
    float mFireZoneTimer;
    float mVoidStormTimer;
    float mLowHealthDarkness;
    float mLowHealthTarget;
    float mEnvironmentTime;
};

namespace CGLabMinecraft
{
struct MyVirtualWorld::WorldData
{
    WorldData()
        : mWindowWidth(WINDOW_START_WIDTH),
          mWindowHeight(WINDOW_START_HEIGHT),
          mLastTimeMs(0),
          mCreeper(),
          mEnderman(),
          mBlaze(),
          mPlayerOneType(CHARACTER_CREEPER),
          mPlayerTwoType(CHARACTER_ENDERMAN),
          mTextures(),
          mBattlefield(),
          mGameOverTimer(0.0f),
          mViewRotateX(0.0f),
          mViewRotateY(0.0f),
          mViewRotateZ(0.0f),
          mCameraShakeTimer(0.0f),
          mCameraShakeAmplitude(0.0f),
          mInitialized(false)
    {
    }

    ~WorldData()
    {
        deleteTexture(mTextures.creeper);
        deleteTexture(mTextures.enderman);
        deleteTexture(mTextures.blaze);
        deleteTexture(mTextures.floor);
        deleteTexture(mTextures.blindBox);
    }

    void initialize()
    {
        std::srand(static_cast<unsigned int>(std::time(0)));

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        glEnable(GL_NORMALIZE);
        glEnable(GL_COLOR_MATERIAL);
        glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
        glShadeModel(GL_SMOOTH);

        GLfloat globalAmbient[] = {0.14f, 0.14f, 0.16f, 1.0f};
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);

        mTextures.creeper = buildCreeperTexture();
        mTextures.enderman = buildEndermanTexture();
        mTextures.blaze = buildBlazeTexture();
        mTextures.floor = buildFloorTexture();
        mTextures.blindBox = buildBlindBoxTexture();

        resetBattle();
        mBattlefield.clearSky();
        mLastTimeMs = glutGet(GLUT_ELAPSED_TIME);
        mInitialized = true;
    }

    void draw()
    {
        if (!mInitialized)
        {
            return;
        }

        GLint viewport[4] = {0, 0, WINDOW_START_WIDTH, WINDOW_START_HEIGHT};
        glGetIntegerv(GL_VIEWPORT, viewport);
        mWindowWidth = std::max(1, viewport[2]);
        mWindowHeight = std::max(1, viewport[3]);

        mBattlefield.clearSky();
        setupCamera();
        mBattlefield.applyLighting();
        mBattlefield.draw(mTextures);
        drawCharacter(mPlayerOneType);
        drawCharacter(mPlayerTwoType);
        drawHud();
    }

    void update()
    {
        if (!mInitialized)
        {
            return;
        }

        const int now = glutGet(GLUT_ELAPSED_TIME);
        float dt = static_cast<float>(now - mLastTimeMs) / 1000.0f;
        mLastTimeMs = now;
        dt = clampFloat(dt, 0.0f, 0.05f);

        updateHeldMovement(dt);
        updateCharacter(mPlayerOneType, dt);
        updateCharacter(mPlayerTwoType, dt);
        mCameraShakeTimer = std::max(0.0f, mCameraShakeTimer - dt);
        mBattlefield.setLowHealth(playerOne().health(), playerTwo().health());
        mBattlefield.update(dt);
        mBattlefield.clearSky();
        resolveSkillDamage(dt);
        updateOutcomeStates();
        updateGameOver(dt);
    }

    bool handleKeyboard(unsigned char key)
    {
        if ((key == 13 || key == '\r') &&
            (playerOne().health() <= 0.0f || playerTwo().health() <= 0.0f))
        {
            resetBattle();
            return true;
        }

        switch (key)
        {
            case 'w':
            case 'W':
                return true;

            case 's':
            case 'S':
                return true;

            case 'a':
            case 'A':
                return true;

            case 'd':
            case 'D':
                return true;

            case 'g':
            case 'G':
                activatePrimarySkill(mPlayerOneType, playerTwo());
                return true;

            case 'h':
            case 'H':
                activateSecondarySkill(mPlayerOneType, playerTwo());
                return true;

            case '8':
                activatePrimarySkill(mPlayerTwoType, playerOne());
                return true;

            case '9':
                activateSecondarySkill(mPlayerTwoType, playerOne());
                return true;

            case 'r':
            case 'R':
                resetBattle();
                return true;

            default:
                break;
        }

        return false;
    }

    bool handleSpecial(int key)
    {
        switch (key)
        {
            case GLUT_KEY_UP:
                return true;

            case GLUT_KEY_DOWN:
                return true;

            case GLUT_KEY_LEFT:
                return true;

            case GLUT_KEY_RIGHT:
                return true;

            default:
                break;
        }

        return false;
    }

    void rotateView(float xinc, float yinc, float zinc)
    {
        mViewRotateX += xinc;
        mViewRotateY += yinc;
        mViewRotateZ += zinc;
    }

    void resetView()
    {
        mViewRotateX = 0.0f;
        mViewRotateY = 0.0f;
        mViewRotateZ = 0.0f;
    }

    void setPlayerCharacters(int playerOneCharacter, int playerTwoCharacter)
    {
        mPlayerOneType = normalizeCharacterType(playerOneCharacter);
        mPlayerTwoType = normalizeCharacterType(playerTwoCharacter);
        if (mPlayerOneType == mPlayerTwoType)
        {
            mPlayerTwoType = static_cast<CharacterType>((mPlayerOneType + 1) % CHARACTER_COUNT);
        }
        resetBattle();
    }

private:
    bool keyDown(int virtualKey) const
    {
        return (GetAsyncKeyState(virtualKey) & 0x8000) != 0;
    }

    BattleCharacter& characterForType(CharacterType type)
    {
        switch (type)
        {
            case CHARACTER_CREEPER:
                return mCreeper;
            case CHARACTER_ENDERMAN:
                return mEnderman;
            case CHARACTER_BLAZE:
                return mBlaze;
            default:
                return mCreeper;
        }
    }

    const BattleCharacter& characterForType(CharacterType type) const
    {
        switch (type)
        {
            case CHARACTER_CREEPER:
                return mCreeper;
            case CHARACTER_ENDERMAN:
                return mEnderman;
            case CHARACTER_BLAZE:
                return mBlaze;
            default:
                return mCreeper;
        }
    }

    BattleCharacter& playerOne()
    {
        return characterForType(mPlayerOneType);
    }

    BattleCharacter& playerTwo()
    {
        return characterForType(mPlayerTwoType);
    }

    const BattleCharacter& playerOne() const
    {
        return characterForType(mPlayerOneType);
    }

    const BattleCharacter& playerTwo() const
    {
        return characterForType(mPlayerTwoType);
    }

    void drawCharacter(CharacterType type) const
    {
        switch (type)
        {
            case CHARACTER_CREEPER:
                mCreeper.draw(mTextures);
                break;
            case CHARACTER_ENDERMAN:
                mEnderman.draw(mTextures);
                break;
            case CHARACTER_BLAZE:
                mBlaze.draw(mTextures);
                break;
            default:
                break;
        }
    }

    void updateCharacter(CharacterType type, float dt)
    {
        switch (type)
        {
            case CHARACTER_CREEPER:
                mCreeper.update(dt);
                break;
            case CHARACTER_ENDERMAN:
                mEnderman.update(dt);
                break;
            case CHARACTER_BLAZE:
                mBlaze.update(dt);
                break;
            default:
                break;
        }
    }

    void resetCharacterForRound(CharacterType type)
    {
        switch (type)
        {
            case CHARACTER_CREEPER:
                mCreeper.resetHealth();
                mCreeper.resetForRound();
                break;
            case CHARACTER_ENDERMAN:
                mEnderman.resetHealth();
                mEnderman.resetForRound();
                break;
            case CHARACTER_BLAZE:
                mBlaze.resetHealth();
                mBlaze.resetForRound();
                break;
            default:
                break;
        }
    }

    void activatePrimarySkill(CharacterType type, const BattleCharacter& opponent)
    {
        switch (type)
        {
            case CHARACTER_CREEPER:
                if (mCreeper.activateExplosion())
                {
                    mBattlefield.triggerExplosionFlash(mCreeper.explosionOrigin());
                }
                break;
            case CHARACTER_ENDERMAN:
                if (mEnderman.activateTeleport(opponent))
                {
                    mBattlefield.triggerTeleportPulse();
                }
                break;
            case CHARACTER_BLAZE:
                if (mBlaze.activateFireBurst(opponent.position()))
                {
                    mBattlefield.triggerBlazeFire(mBlaze.fireBurstOrigin());
                }
                break;
            default:
                break;
        }
    }

    void activateSecondarySkill(CharacterType type, const BattleCharacter& opponent)
    {
        switch (type)
        {
            case CHARACTER_CREEPER:
                if (mCreeper.activatePowderCloud())
                {
                    mBattlefield.triggerPowderCloudPulse();
                }
                break;
            case CHARACTER_ENDERMAN:
                if (mEnderman.activateVoidBeam(opponent.position()))
                {
                    mBattlefield.triggerTeleportPulse();
                }
                break;
            case CHARACTER_BLAZE:
                mBlaze.activateFireball(opponent.position());
                break;
            default:
                break;
        }
    }

    void updateHeldMovement(float dt)
    {
        BattleCharacter& first = playerOne();
        BattleCharacter& second = playerTwo();

        Vec3 playerOneDirection;
        if (keyDown('W')) { playerOneDirection.z -= 1.0f; }
        if (keyDown('S')) { playerOneDirection.z += 1.0f; }
        if (keyDown('A')) { playerOneDirection.x -= 1.0f; }
        if (keyDown('D')) { playerOneDirection.x += 1.0f; }

        if (first.health() > 0.0f && length2D(playerOneDirection) > 0.0f)
        {
            first.moveBy(playerOneDirection, dt);
        }
        else
        {
            first.stopMoving();
        }

        Vec3 playerTwoDirection;
        if (keyDown(VK_UP)) { playerTwoDirection.z -= 1.0f; }
        if (keyDown(VK_DOWN)) { playerTwoDirection.z += 1.0f; }
        if (keyDown(VK_LEFT)) { playerTwoDirection.x -= 1.0f; }
        if (keyDown(VK_RIGHT)) { playerTwoDirection.x += 1.0f; }

        if (second.health() > 0.0f && length2D(playerTwoDirection) > 0.0f)
        {
            second.moveBy(playerTwoDirection, dt);
        }
        else
        {
            second.stopMoving();
        }
    }

    void applyKnockback(BattleCharacter& character, const Vec3& origin, float force)
    {
        Vec3 direction = normalize2D(character.position() - origin);
        if (length2D(direction) < 0.0001f)
        {
            direction = Vec3(1.0f, 0.0f, 0.0f);
        }

        character.applyKnockbackImpulse(direction, force * 2.0f, 3.0f + force);
    }

    void triggerCreeperCameraShake(const Vec3& explosionOrigin)
    {
        const Vec3 cameraGroundPosition(0.0f, 0.0f, 13.0f);
        const float distanceToCamera =
            length2D(explosionOrigin - cameraGroundPosition);
        const float proximity =
            1.0f - clampFloat(distanceToCamera / 20.0f, 0.0f, 1.0f);

        mCameraShakeTimer = 0.25f;
        mCameraShakeAmplitude = 0.07f + proximity * 0.24f;
    }

    void updateOutcomeStates()
    {
        const bool playerOneWon =
            playerOne().health() > 0.0f && playerTwo().health() <= 0.0f;
        const bool playerTwoWon =
            playerTwo().health() > 0.0f && playerOne().health() <= 0.0f;
        playerOne().setVictorious(playerOneWon);
        playerTwo().setVictorious(playerTwoWon);
    }

    void deleteTexture(GLuint& texture)
    {
        if (texture != 0)
        {
            glDeleteTextures(1, &texture);
            texture = 0;
        }
    }

    void resetBattle()
    {
        resetCharacterForRound(CHARACTER_CREEPER);
        resetCharacterForRound(CHARACTER_ENDERMAN);
        resetCharacterForRound(CHARACTER_BLAZE);
        playerOne().setPosition(Vec3(-3.0f, 0.0f, 0.0f));
        playerTwo().setPosition(Vec3(3.0f, 0.0f, 0.0f));
        mBattlefield.reset();
        mGameOverTimer = 0.0f;
        mCameraShakeTimer = 0.0f;
        mCameraShakeAmplitude = 0.0f;
    }

    void setupCamera()
    {
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        float shakeX = 0.0f;
        float shakeY = 0.0f;
        if (mCameraShakeTimer > 0.0f)
        {
            const float fade = clampFloat(mCameraShakeTimer / 0.25f, 0.0f, 1.0f);
            const float phase = static_cast<float>(mLastTimeMs) * 0.085f;
            shakeX = std::sin(phase) * mCameraShakeAmplitude * fade;
            shakeY = std::cos(phase * 1.37f) * mCameraShakeAmplitude * 0.62f * fade;
        }

        gluLookAt(shakeX, 6.2 + shakeY, 13.0,
                  shakeX * 0.22f, 1.6 + shakeY * 0.16f, 0.0,
                  0.0, 1.0, 0.0);

        glRotatef(mViewRotateX, 1.0f, 0.0f, 0.0f);
        glRotatef(mViewRotateY, 0.0f, 1.0f, 0.0f);
        glRotatef(mViewRotateZ, 0.0f, 0.0f, 1.0f);
    }

    void resolveSkillDamage(float dt)
    {
        BattleCharacter& first = playerOne();
        BattleCharacter& second = playerTwo();
        resolveCharacterSkillDamage(mPlayerOneType, first, second, dt);
        resolveCharacterSkillDamage(mPlayerTwoType, second, first, dt);
    }

    void resolveCharacterSkillDamage(CharacterType type,
                                     BattleCharacter& attacker,
                                     BattleCharacter& defender,
                                     float dt)
    {
        switch (type)
        {
            case CHARACTER_CREEPER:
                if (mCreeper.explosionDamageReady())
                {
                    const float distance = length2D(defender.position() - mCreeper.explosionOrigin());
                    const float radius = mCreeper.explosionDamageRadius() + 0.55f;
                    mBattlefield.triggerCreeperImpact(mCreeper.explosionOrigin());
                    triggerCreeperCameraShake(mCreeper.explosionOrigin());
                    if (distance <= radius)
                    {
                        const float closeFactor = 1.0f - clampFloat(distance / radius, 0.0f, 1.0f);
                        defender.takeDamage(10.0f + closeFactor * 25.0f);
                        applyKnockback(defender, mCreeper.explosionOrigin(), 0.6f + closeFactor * 2.4f);
                    }
                    mCreeper.markExplosionDamageHandled();
                }

                if (mCreeper.powderCloudDamageTickReady() && defender.health() > 0.0f)
                {
                    const float distance = length2D(defender.position() - mCreeper.powderCloudOrigin());
                    if (distance <= mCreeper.powderCloudRadius())
                    {
                        defender.takePoisonDamage(mCreeper.powderCloudDamagePerSecond() *
                                                  mCreeper.powderCloudDamageTickInterval());
                    }
                    mCreeper.markPowderCloudDamageTickHandled();
                }
                break;

            case CHARACTER_ENDERMAN:
                if (mEnderman.teleportDamageReady())
                {
                    const float distance = length2D(defender.position() - attacker.position());
                    if (distance <= mEnderman.teleportSlashRadius())
                    {
                        const float closeFactor = 1.0f - clampFloat(distance / mEnderman.teleportSlashRadius(),
                                                                    0.0f, 1.0f);
                        defender.takeDamage(22.0f + closeFactor * 8.0f);
                    }
                    mEnderman.markTeleportDamageHandled();
                }

                if (mEnderman.voidBeamDamageReady())
                {
                    const float hitDistance = length2D(defender.position() - mEnderman.voidBeamTarget());
                    const float castDistance = length2D(mEnderman.voidBeamTarget() - attacker.position());
                    if (castDistance <= mEnderman.voidBeamMaxRange() &&
                        hitDistance <= mEnderman.voidBeamRadius())
                    {
                        defender.takeDamage(mEnderman.voidBeamDamage(castDistance));
                    }
                    mEnderman.markVoidBeamDamageHandled();
                }
                break;

            case CHARACTER_BLAZE:
                if (mBlaze.fireBurstDamageReady())
                {
                    const float radius = mBlaze.fireBurstRadius();
                    const float distance = length2D(defender.position() - mBlaze.fireBurstOrigin());
                    if (distance <= radius + 0.50f)
                    {
                        const float closeFactor = 1.0f - clampFloat(distance / (radius + 0.50f), 0.0f, 1.0f);
                        defender.takeDamage(8.0f + closeFactor * 22.0f);
                    }
                    mBlaze.markFireBurstDamageHandled();
                }

                if (mBlaze.fireballDamageReady())
                {
                    const float hitDistance = length2D(defender.position() - mBlaze.fireballTarget());
                    const float castDistance = length2D(mBlaze.fireballTarget() - attacker.position());
                    if (castDistance <= mBlaze.fireballMaxRange() &&
                        hitDistance <= mBlaze.fireballRadius())
                    {
                        const float closeFactor =
                            1.0f - clampFloat(hitDistance / mBlaze.fireballRadius(),
                                              0.0f, 1.0f);
                        defender.takeDamage(mBlaze.fireballDamage(hitDistance));
                        applyKnockback(defender, mBlaze.fireballTarget(),
                                       0.6f + closeFactor * 2.4f);
                    }
                    mBattlefield.triggerBlazeFire(mBlaze.fireballTarget());
                    mBlaze.markFireballDamageHandled();
                }
                break;

            default:
                break;
        }
    }

    void updateGameOver(float dt)
    {
        if (playerOne().health() <= 0.0f || playerTwo().health() <= 0.0f)
        {
            mGameOverTimer += dt;
        }
        else
        {
            mGameOverTimer = 0.0f;
        }
    }

    void drawText(float x, float y, const char* text, void* font) const
    {
        glRasterPos2f(x, y);
        for (const char* p = text; *p != '\0'; ++p)
        {
            glutBitmapCharacter(font, *p);
        }
    }

    void drawHealthBar(float x, float y, float width, float height,
                       float health, float r, float g, float b) const
    {
        glColor3f(0.08f, 0.08f, 0.09f);
        glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x + width, y);
        glVertex2f(x + width, y + height);
        glVertex2f(x, y + height);
        glEnd();

        const float filled = width * clampFloat(health / 100.0f, 0.0f, 1.0f);
        glColor3f(r, g, b);
        glBegin(GL_QUADS);
        glVertex2f(x + 2.0f, y + 2.0f);
        glVertex2f(x + filled - 2.0f, y + 2.0f);
        glVertex2f(x + filled - 2.0f, y + height - 2.0f);
        glVertex2f(x + 2.0f, y + height - 2.0f);
        glEnd();
    }

    void drawCooldownIcon(float cx, float cy, float radius,
                          float percent, float r, float g, float b) const
    {
        glColor3f(0.08f, 0.08f, 0.1f);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(cx, cy);
        for (int i = 0; i <= 40; ++i)
        {
            const float a = 2.0f * PI * i / 40.0f;
            glVertex2f(cx + std::cos(a) * radius, cy + std::sin(a) * radius);
        }
        glEnd();

        const float ready = 1.0f - percent;
        glColor3f(r, g, b);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(cx, cy);
        for (int i = 0; i <= 40; ++i)
        {
            const float a = -PI * 0.5f + ready * 2.0f * PI * i / 40.0f;
            glVertex2f(cx + std::cos(a) * (radius - 2.0f),
                       cy + std::sin(a) * (radius - 2.0f));
        }
        glEnd();
    }

    void drawCooldownText(float cx, float cy, float seconds, const char* readyText) const
    {
        char text[8];
        if (seconds > 0.05f)
        {
            std::sprintf(text, "%.0f", std::ceil(seconds));
        }
        else
        {
            std::sprintf(text, "%s", readyText);
        }

        int width = 0;
        for (const char* p = text; *p != '\0'; ++p)
        {
            width += glutBitmapWidth(GLUT_BITMAP_HELVETICA_12, *p);
        }

        glColor3f(1.0f, 1.0f, 1.0f);
        drawText(cx - width * 0.5f, cy - 4.0f, text, GLUT_BITMAP_HELVETICA_12);
    }

    void characterColor(CharacterType type, float& r, float& g, float& b) const
    {
        switch (type)
        {
            case CHARACTER_CREEPER:
                r = 0.25f; g = 0.90f; b = 0.25f;
                break;
            case CHARACTER_ENDERMAN:
                r = 0.62f; g = 0.15f; b = 1.0f;
                break;
            case CHARACTER_BLAZE:
                r = 1.0f; g = 0.88f; b = 0.06f;
                break;
            default:
                r = g = b = 0.8f;
                break;
        }
    }

    void skillColor(CharacterType type, bool primary, float& r, float& g, float& b) const
    {
        switch (type)
        {
            case CHARACTER_CREEPER:
                if (primary)
                {
                    r = 1.0f; g = 0.28f; b = 0.05f;
                }
                else
                {
                    r = 0.10f; g = 1.0f; b = 0.18f;
                }
                break;
            case CHARACTER_ENDERMAN:
                if (primary)
                {
                    r = 0.65f; g = 0.18f; b = 1.0f;
                }
                else
                {
                    r = 0.08f; g = 0.88f; b = 1.0f;
                }
                break;
            case CHARACTER_BLAZE:
                if (primary)
                {
                    r = 1.0f; g = 0.82f; b = 0.04f;
                }
                else
                {
                    r = 1.0f; g = 0.94f; b = 0.10f;
                }
                break;
            default:
                r = g = b = 0.8f;
                break;
        }
    }

    const char* primarySkillName(CharacterType type) const
    {
        switch (type)
        {
            case CHARACTER_CREEPER:
                return "Explosion";
            case CHARACTER_ENDERMAN:
                return "Teleport Slash";
            case CHARACTER_BLAZE:
                return "Fire Burst";
            default:
                return "Primary";
        }
    }

    const char* secondarySkillName(CharacterType type) const
    {
        switch (type)
        {
            case CHARACTER_CREEPER:
                return "Powder Cloud";
            case CHARACTER_ENDERMAN:
                return "Void Beam";
            case CHARACTER_BLAZE:
                return "Fireball";
            default:
                return "Secondary";
        }
    }

    float primaryCooldownPercent(CharacterType type) const
    {
        switch (type)
        {
            case CHARACTER_CREEPER:
                return mCreeper.cooldownPercent();
            case CHARACTER_ENDERMAN:
                return mEnderman.cooldownPercent();
            case CHARACTER_BLAZE:
                return mBlaze.cooldownPercent();
            default:
                return 0.0f;
        }
    }

    float primaryCooldownSeconds(CharacterType type) const
    {
        switch (type)
        {
            case CHARACTER_CREEPER:
                return mCreeper.cooldownSeconds();
            case CHARACTER_ENDERMAN:
                return mEnderman.cooldownSeconds();
            case CHARACTER_BLAZE:
                return mBlaze.cooldownSeconds();
            default:
                return 0.0f;
        }
    }

    float secondaryCooldownPercent(CharacterType type) const
    {
        switch (type)
        {
            case CHARACTER_CREEPER:
                return mCreeper.powderCloudCooldownPercent();
            case CHARACTER_ENDERMAN:
                return mEnderman.voidBeamCooldownPercent();
            case CHARACTER_BLAZE:
                return mBlaze.fireballCooldownPercent();
            default:
                return 0.0f;
        }
    }

    float secondaryCooldownSeconds(CharacterType type) const
    {
        switch (type)
        {
            case CHARACTER_CREEPER:
                return mCreeper.powderCloudCooldownSeconds();
            case CHARACTER_ENDERMAN:
                return mEnderman.voidBeamCooldownSeconds();
            case CHARACTER_BLAZE:
                return mBlaze.fireballCooldownSeconds();
            default:
                return 0.0f;
        }
    }

    void drawHud()
    {
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(0.0, mWindowWidth, 0.0, mWindowHeight);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glColor4f(0.02f, 0.02f, 0.025f, 0.58f);
        glBegin(GL_QUADS);
        glVertex2f(0.0f, static_cast<float>(mWindowHeight));
        glVertex2f(static_cast<float>(mWindowWidth), static_cast<float>(mWindowHeight));
        glVertex2f(static_cast<float>(mWindowWidth), static_cast<float>(mWindowHeight - 108));
        glVertex2f(0.0f, static_cast<float>(mWindowHeight - 108));
        glEnd();

        glColor3f(1.0f, 1.0f, 1.0f);
        char title[128];
        char playerOneLine[160];
        char playerTwoLine[180];
        std::sprintf(title, "Blind Boxes Collection: %s vs %s",
                     playerOne().name(), playerTwo().name());
        std::sprintf(playerOneLine, "P1 %s: WASD + G %s + H %s",
                     playerOne().name(),
                     primarySkillName(mPlayerOneType),
                     secondarySkillName(mPlayerOneType));
        std::sprintf(playerTwoLine, "P2 %s: Arrow Keys + 8 %s + 9 %s     R Reset",
                     playerTwo().name(),
                     primarySkillName(mPlayerTwoType),
                     secondarySkillName(mPlayerTwoType));
        drawText(24.0f, static_cast<float>(mWindowHeight - 28),
                 title, GLUT_BITMAP_HELVETICA_18);
        drawText(24.0f, static_cast<float>(mWindowHeight - 56),
                 playerOneLine, GLUT_BITMAP_HELVETICA_12);
        drawText(24.0f, static_cast<float>(mWindowHeight - 76),
                 playerTwoLine, GLUT_BITMAP_HELVETICA_12);

        const float barY = static_cast<float>(mWindowHeight - 100);
        float r = 0.0f;
        float g = 0.0f;
        float b = 0.0f;
        float sr = 0.0f;
        float sg = 0.0f;
        float sb = 0.0f;

        characterColor(mPlayerOneType, r, g, b);
        drawHealthBar(24.0f, barY, 220.0f, 16.0f,
                      playerOne().health(), r, g, b);
        skillColor(mPlayerOneType, true, sr, sg, sb);
        drawCooldownIcon(270.0f, barY + 8.0f, 13.0f,
                         primaryCooldownPercent(mPlayerOneType), sr, sg, sb);
        drawCooldownText(270.0f, barY + 8.0f,
                         primaryCooldownSeconds(mPlayerOneType), "G");
        skillColor(mPlayerOneType, false, sr, sg, sb);
        drawCooldownIcon(304.0f, barY + 8.0f, 13.0f,
                         secondaryCooldownPercent(mPlayerOneType), sr, sg, sb);
        drawCooldownText(304.0f, barY + 8.0f,
                         secondaryCooldownSeconds(mPlayerOneType), "H");

        characterColor(mPlayerTwoType, r, g, b);
        drawHealthBar(static_cast<float>(mWindowWidth - 244), barY, 220.0f, 16.0f,
                      playerTwo().health(), r, g, b);
        skillColor(mPlayerTwoType, true, sr, sg, sb);
        drawCooldownIcon(static_cast<float>(mWindowWidth - 304), barY + 8.0f, 13.0f,
                         primaryCooldownPercent(mPlayerTwoType), sr, sg, sb);
        drawCooldownText(static_cast<float>(mWindowWidth - 304), barY + 8.0f,
                         primaryCooldownSeconds(mPlayerTwoType), "8");
        skillColor(mPlayerTwoType, false, sr, sg, sb);
        drawCooldownIcon(static_cast<float>(mWindowWidth - 270), barY + 8.0f, 13.0f,
                         secondaryCooldownPercent(mPlayerTwoType), sr, sg, sb);
        drawCooldownText(static_cast<float>(mWindowWidth - 270), barY + 8.0f,
                         secondaryCooldownSeconds(mPlayerTwoType), "9");

        if (playerOne().health() <= 0.0f || playerTwo().health() <= 0.0f)
        {
            char winner[64];
            const char* restartText = "Press ENTER to restart";
            std::sprintf(winner, "%s wins!",
                         (playerOne().health() > 0.0f) ? playerOne().name() : playerTwo().name());

            glColor4f(0.0f, 0.0f, 0.0f, 0.58f);
            glBegin(GL_QUADS);
            glVertex2f(static_cast<float>(mWindowWidth / 2 - 170), static_cast<float>(mWindowHeight / 2 - 48));
            glVertex2f(static_cast<float>(mWindowWidth / 2 + 170), static_cast<float>(mWindowHeight / 2 - 48));
            glVertex2f(static_cast<float>(mWindowWidth / 2 + 170), static_cast<float>(mWindowHeight / 2 + 48));
            glVertex2f(static_cast<float>(mWindowWidth / 2 - 170), static_cast<float>(mWindowHeight / 2 + 48));
            glEnd();

            glColor3f(1.0f, 0.95f, 0.35f);
            int winnerWidth = 0;
            for (const char* p = winner; *p != '\0'; ++p)
            {
                winnerWidth += glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, *p);
            }
            drawText(static_cast<float>(mWindowWidth / 2) - winnerWidth * 0.5f,
                     static_cast<float>(mWindowHeight / 2 + 10),
                     winner, GLUT_BITMAP_HELVETICA_18);

            int restartWidth = 0;
            for (const char* p = restartText; *p != '\0'; ++p)
            {
                restartWidth += glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, *p);
            }
            glColor3f(1.0f, 1.0f, 1.0f);
            drawText(static_cast<float>(mWindowWidth / 2) - restartWidth * 0.5f,
                     static_cast<float>(mWindowHeight / 2 - 22),
                     restartText, GLUT_BITMAP_HELVETICA_18);
        }

        glPopAttrib();
        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
    }

    int mWindowWidth;
    int mWindowHeight;
    int mLastTimeMs;
    Creeper mCreeper;
    Enderman mEnderman;
    Blaze mBlaze;
    CharacterType mPlayerOneType;
    CharacterType mPlayerTwoType;
    TextureSet mTextures;
    Battlefield mBattlefield;
    float mGameOverTimer;
    float mViewRotateX;
    float mViewRotateY;
    float mViewRotateZ;
    float mCameraShakeTimer;
    float mCameraShakeAmplitude;
    bool mInitialized;
};

MyVirtualWorld::MyVirtualWorld()
    : mData(new WorldData())
{
}

MyVirtualWorld::~MyVirtualWorld()
{
    delete mData;
}

void MyVirtualWorld::init()
{
    mData->initialize();
}

void MyVirtualWorld::draw()
{
    mData->draw();
}

void MyVirtualWorld::tickTime()
{
    mData->update();
}

bool MyVirtualWorld::handleKeyboard(unsigned char key)
{
    return mData->handleKeyboard(key);
}

bool MyVirtualWorld::handleSpecial(int key)
{
    return mData->handleSpecial(key);
}

void MyVirtualWorld::setPlayerCharacters(int playerOneCharacter, int playerTwoCharacter)
{
    mData->setPlayerCharacters(playerOneCharacter, playerTwoCharacter);
}

void MyVirtualWorld::rotateView(float xinc, float yinc, float zinc)
{
    mData->rotateView(xinc, yinc, zinc);
}

void MyVirtualWorld::resetView()
{
    mData->resetView();
}
}
