# ifndef BULLETS_H
# define BULLETS_H

# include "./scenegraph.h"
# include "./player.h"

# include <SFML/Graphics.hpp>
# include <memory>
# include <deque>
# include <vector>
# include <cmath>

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

const float div255 = 1.f / 255.f;

class BulletScript;

class Bullet {
private:
    static sf::Shader bulletFrontShader;
    static sf::Shader bulletBackShader;
    static sf::Vector2u wSize;
    static const int BULLET_RENDER_RADIUS;
    static const float COLLISION_DIST_SQD;
    static const int BULLET_DEATH_TIME;

    std::shared_ptr<DrawableNode> frontNode;
    std::shared_ptr<DrawableNode> backNode;

    sf::Texture textureFront;
    sf::Texture textureBack;
    sf::Sprite spriteFront;
    sf::Sprite spriteBack;
public:
    enum Type {
        orb,
    };

    static std::shared_ptr<Node> rootNode;
    static std::shared_ptr<Node> frontRootNode;
    static std::shared_ptr<Node> backRootNode;
    static std::vector<std::shared_ptr<Bullet>> bullets;

    static void init(sf::Vector2u windowSize) {
        rootNode->addChild(backRootNode);
        rootNode->addChild(frontRootNode);

        wSize = windowSize;
        bulletFrontShader.loadFromMemory(VertexShader, BulletFrontGradient);
        bulletBackShader.loadFromMemory(VertexShader, BulletBackGradient);
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

    // TODO: render bullet onto seperate canvas (update only when needed) and render that canvas onto window (saves computation time)
    Bullet(Type type, sf::Color color, float radius, float x, float y, float dir, float speed, std::shared_ptr<BulletScript> script) {
        remove = false;
        alive = true;
        updateTexture = true;
        time = 0;
        this->type = type;
        this->radius = radius;
        this->x = x;
        this->y = y;
        this->dir = dir;
        this->speed = speed;
        accel = 0;
        accelCap = 0;
        this->color = color;
        rotate = false;
        rotOrigin = { x, y };
        rotDist = 0;
        rotSpeed = 0;
        rotAccel = 0;
        rotAccelCap = 0;
        this->script = script;
        scriptFinished = script == nullptr;

        switch (type) {
        case orb:
            circle = sf::CircleShape(BULLET_RENDER_RADIUS * 2);
            circle.setOrigin(circle.getRadius(), circle.getRadius());
            circle.setPosition(circle.getRadius(), circle.getRadius());
            circle.setFillColor(sf::Color::Transparent);

            this->frontNode = std::make_shared<DrawableNode>([this](sf::RenderTarget& renderTarget, sf::Transform trans, int calcTick) {
                renderTarget.draw(spriteFront, trans);
                });
            this->backNode = std::make_shared<DrawableNode>([this](sf::RenderTarget& renderTarget, sf::Transform trans, int calcTick) {
                renderTarget.draw(spriteBack, trans);
                });
            break;
        }
        frontRootNode->addChild((std::shared_ptr<Node>)this->frontNode);
        backRootNode->addChild((std::shared_ptr<Node>)this->backNode);
    }

    // create bullet and put into bullets list
    static std::shared_ptr<Bullet> create(Type type, sf::Color color, float radius, float x, float y, float dir, float speed, std::shared_ptr<BulletScript> script);

    // run move tick for all bullets
    static void moveTick() {
        // move bullets
        for (std::shared_ptr<Bullet> b : bullets)
            b->tick();

        // remove dead bullets
        bullets.erase(std::remove_if(bullets.begin(), bullets.end(), [](const std::shared_ptr<Bullet> &b) {
            return b->remove;
            }), bullets.end());
    }

    void kill() {
        if (!alive) return;
        alive = false;
        time = 0;
    }

    void createTexture() {
        switch (type) {
        case orb:
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
            break;
        }
    }

    // tick
    void tick();
};

# endif