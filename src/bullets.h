# ifndef BULLETS_H
# define BULLETS_H

# include "./scenegraph.h"
# include "./player.h"

# include <SFML/Graphics.hpp>
# include <memory>
# include <deque>
# include <vector>
# include <cmath>

# define USE_SHADER false

# if USE_SHADER
const char RadialGradient[] =
"uniform vec4 color;"
"uniform vec2 center;"
"uniform float radius;"
"uniform float expand;"
"uniform float windowHeight;"
"void main(void)"
"{"
"vec2 centerFromSfml = vec2(center.x, windowHeight - center.y);"
"vec2 p = (gl_FragCoord.xy - centerFromSfml) / radius;"
"float r = sqrt(dot(p, p));"
"if (r < 1.0)"
"{"
"gl_FragColor = mix(color, gl_Color, (r - expand) / (1 - expand));"
"}"
"else"
"{"
"gl_FragColor = gl_Color;"
"}"
"}";

const char BulletGradient[] =
"uniform vec4 colorCenter;"
"uniform vec4 colorPrimary;"
"uniform vec2 center;"
"uniform float radius;"
"uniform float windowHeight;"
"void main(void)"
"{"
"vec2 centerFromSfml = vec2(center.x, windowHeight - center.y);"
"vec2 p = (gl_FragCoord.xy - centerFromSfml) / radius;"
"float r = sqrt(dot(p, p));"
"vec4 clear = colorPrimary * vec4(1.0, 1.0, 1.0, 0.0);"
"vec4 halfClear = colorPrimary * vec4(1.0, 1.0, 1.0, 0.5);"
"vec4 color;"
"if (r < 0.25)"
"{"
"color = colorCenter;"
"}"
"else if (r < 0.5)"
"{"
"color = mix(colorCenter, colorPrimary, smoothstep(0.25, 0.5, r));"
"}"
"else if (r < 0.65)"
"{"
"color = mix(colorPrimary, halfClear, smoothstep(0.5, 0.65, r));"
"}"
"else if (r < 1.0)"
"{"
"color = mix(halfClear, clear, smoothstep(0.65, 1.0, r));"
"}"
"else"
"{"
"color = gl_Color;"
"}"
"gl_FragColor = color;"
"}";

const char BulletBackGradient[] =
"uniform vec4 colorPrimary;"
"uniform vec2 center;"
"uniform float radius;"
"uniform float windowHeight;"
"void main(void)"
"{"
"vec2 centerFromSfml = vec2(center.x, windowHeight - center.y);"
"vec2 p = (gl_FragCoord.xy - centerFromSfml) / radius;"
"float r = sqrt(dot(p, p));"
"vec4 clear = colorPrimary * vec4(1.0, 1.0, 1.0, 0.0);"
"vec4 halfClear = colorPrimary * vec4(1.0, 1.0, 1.0, 0.5);"
"vec4 color;"
"if (r < 0.5)"
"{"
"color = colorPrimary;"
"}"
"else if (r < 0.65)"
"{"
"color = mix(colorPrimary, halfClear, smoothstep(0.5, 0.65, r));"
"}"
"else if (r < 1.0)"
"{"
"color = mix(halfClear, clear, smoothstep(0.65, 1.0, r));"
"}"
"else"
"{"
"color = clear;"
"}"
"gl_FragColor = color;"
"}";

const char BulletFrontGradient[] =
"uniform vec4 colorCenter;"
"uniform vec2 center;"
"uniform float radius;"
"uniform float windowHeight;"
"void main(void)"
"{"
"vec2 centerFromSfml = vec2(center.x, windowHeight - center.y);"
"vec2 p = (gl_FragCoord.xy - centerFromSfml) / radius;"
"float r = sqrt(dot(p, p));"
"vec4 color;"
"vec4 clear = colorCenter * vec4(1.0, 1.0, 1.0, 0.0);"
"if (r < 0.25)"
"{"
"color = colorCenter;"
"}"
"else if (r < 0.5)"
"{"
"color = mix(colorCenter, clear, smoothstep(0.25, 0.5, r));"
"}"
"else"
"{"
"color = clear;"
"}"
"gl_FragColor = color;"
"}";

const char VertexShader[] =
"void main()"
"{"
"gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;"
"gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;"
"gl_FrontColor = gl_Color;"
"}";
# endif

const float div255 = 1.f / 255.f;

class BulletScript;

class Bullet {
private:
# if USE_SHADER
    static sf::Shader bulletFrontShader;
    static sf::Shader bulletBackShader;
# endif

    static const int BULLET_RENDER_RADIUS;
    static const float COLLISION_DIST_SQD;
    static const int BULLET_DEATH_TIME;

    static sf::Vector2u wSize;

    std::shared_ptr<DrawableNode> frontNode;
    std::shared_ptr<DrawableNode> backNode;

# if USE_SHADER
    sf::Texture textureFront;
    sf::Texture textureBack;
    sf::Sprite spriteFront;
    sf::Sprite spriteBack;
# else
    std::vector<sf::CircleShape> frontCircles;
    std::vector<sf::CircleShape> backCircles;
# endif

    static float leftX;
    static float rightX;
    static float topY;
    static float bottomY;

# if USE_SHADER
    static std::vector<std::shared_ptr<Bullet>> deleteQueue;
# endif
public:
    enum Type {
        orb,
    };

    static std::shared_ptr<Node> rootNode;
    static std::shared_ptr<Node> frontRootNode;
    static std::shared_ptr<Node> backRootNode;
    static std::vector<std::shared_ptr<Bullet>> bullets;

    static void init(sf::Vector2u windowSize, float leftX, float rightX, float topY, float bottomY) {
        rootNode->addChild(backRootNode);
        rootNode->addChild(frontRootNode);

        wSize = windowSize;
# if USE_SHADER
        bulletFrontShader.loadFromMemory(VertexShader, BulletFrontGradient);
        bulletBackShader.loadFromMemory(VertexShader, BulletBackGradient);
# endif

        Bullet::leftX = leftX;
        Bullet::rightX = rightX;
        Bullet::topY = topY;
        Bullet::bottomY = bottomY;
    }

    bool remove;
    bool alive;
    bool updateTexture;
    int time;
    Type type;
    float radius;
    float x, y;
    float dir, speed, accel, accelCap;
    sf::Color color;
    sf::CircleShape circle;
    std::shared_ptr<BulletScript> script;
    bool scriptFinished;

    // rotate info
    // note: when rotating, speed/accel = dist speed/accel, dir = dir, and rotation has seperate accel parameter
    bool rotate;
    sf::Vector2f rotOrigin;
    float rotDist, rotSpeed, rotAccel, rotAccelCap;

    // constructor
    Bullet();
    Bullet(Type type, sf::Color color, float radius, float x, float y, float dir, float speed, std::shared_ptr<BulletScript> script);

    // create bullet and put into bullets list
    static std::shared_ptr<Bullet> create(Type type, sf::Color color, float radius, float x, float y, float dir, float speed, std::shared_ptr<BulletScript> script);

    // run move tick for all bullets
    static void moveTick(int calcTick) {
        // move bullets
        for (std::shared_ptr<Bullet> b : bullets)
            b->tick();

        // remove dead bullets
        auto it = std::remove_if(bullets.begin(), bullets.end(), [](const std::shared_ptr<Bullet>& b) {
            return b->remove;
            });
# if USE_SHADER
        deleteQueue.insert(deleteQueue.begin(), it, bullets.end());
# endif
        bullets.erase(it, bullets.end());

# if USE_SHADER
        // update dead bullet queue
        if ((calcTick & 0b10) && deleteQueue.size() > 0) deleteQueue.pop_back();
# endif
    }

    void kill() {
        if (!alive) return;
        alive = false;
        time = 0;
    }

    void renderUpdate() {
        switch (type) {
        case orb:
# if USE_SHADER
            static sf::RenderTexture rtFront;
            static sf::RenderTexture rtBack;
            int ts = std::ceil(circle.getRadius() * 2);
            if (!rtFront.create(ts, ts) || !rtBack.create(ts, ts)) throw("error creating bullet render textures");

            // front texture
            sf::Color colorCenter(255, 255, 255, 255);
            static bool init = false;
            if (!init) {
                bulletFrontShader.setUniform("windowHeight", (float)ts);
                bulletFrontShader.setUniform("radius", circle.getRadius());
                bulletFrontShader.setUniform("center", sf::Vector2f(ts * 0.5f, ts * 0.5f));
            }
            bulletFrontShader.setUniform("colorCenter", sf::Glsl::Vec4(colorCenter.r * div255, colorCenter.g * div255, colorCenter.b * div255, colorCenter.a * div255));
            rtFront.clear(colorCenter * sf::Color(255, 255, 255, 0));
            rtFront.draw(circle, &bulletFrontShader);
            textureFront = rtFront.getTexture();
            spriteFront = sf::Sprite(textureFront);
            spriteFront.setOrigin(ts * 0.5f, ts * 0.5f);

            // back texture
            if (!init) {
                init = true;
                bulletBackShader.setUniform("windowHeight", (float)ts);
                bulletBackShader.setUniform("radius", circle.getRadius());
                bulletBackShader.setUniform("center", sf::Vector2f(ts * 0.5f, ts * 0.5f));
            }
            bulletBackShader.setUniform("colorPrimary", sf::Glsl::Vec4(color.r * div255, color.g * div255, color.b * div255, color.a * div255));
            rtBack.clear(color * sf::Color(255, 255, 255, 0));
            rtBack.draw(circle, &bulletBackShader);
            textureBack = rtBack.getTexture();
            spriteBack = sf::Sprite(textureBack);
            spriteBack.setOrigin(ts * 0.5f, ts * 0.5f);
# else
            frontCircles = std::vector<sf::CircleShape>(1);
            frontCircles[0].setRadius(BULLET_RENDER_RADIUS * 0.5);
            frontCircles[0].setOutlineThickness(BULLET_RENDER_RADIUS * 0.25);
            frontCircles[0].setFillColor(sf::Color::White);
            frontCircles[0].setOutlineColor(sf::Color(255,255,255,200));
            backCircles = std::vector<sf::CircleShape>(1);
            backCircles[0].setRadius(BULLET_RENDER_RADIUS * 1);
            backCircles[0].setOutlineThickness(BULLET_RENDER_RADIUS * 0.5);
            backCircles[0].setFillColor(color);
            backCircles[0].setOutlineColor(color * sf::Color(color.r, color.g, color.b, 64));
# endif
            break;
        }
        for (sf::CircleShape& circle : frontCircles) {
            circle.setOrigin(circle.getRadius(), circle.getRadius());
        }
        for (sf::CircleShape& circle : backCircles) {
            circle.setOrigin(circle.getRadius(), circle.getRadius());
        }
    }

    // tick
    void tick();

    // returns true iff bullet off screen
    bool offScreen() {
        float r = radius * 2;
        return x < leftX - r || x > rightX + r || y < topY - r || y > bottomY + r;
    }
};

# endif