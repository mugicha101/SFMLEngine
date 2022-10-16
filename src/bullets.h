# ifndef BULLETS_H
# define BULLETS_H

# include "./scenegraph.h"

# include <SFML/Graphics.hpp>
# include <memory>
# include <list>
# include <vector>

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
"color = gl_Color;"
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
"color = gl_Color;"
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

class Bullet {
private:
    static sf::Shader bulletFrontShader;
    static sf::Shader bulletBackShader;
    static sf::Vector2u wSize;

    std::shared_ptr<DrawableNode> frontNode;
    std::shared_ptr<DrawableNode> backNode;
public:
    enum Type {
        orb,
    };

    static std::shared_ptr<Node> rootNode;
    static std::shared_ptr<Node> frontRootNode;
    static std::shared_ptr<Node> backRootNode;
    static std::list<std::shared_ptr<Bullet>> bullets;

    static void init(sf::Vector2u windowSize) {
        rootNode->addChild(backRootNode);
        rootNode->addChild(frontRootNode);

        wSize = windowSize;
        bulletFrontShader.loadFromMemory(VertexShader, BulletFrontGradient);
        bulletFrontShader.setUniform("windowHeight", static_cast<float>(wSize.y)); // this must be set, but only needs to be set once (or whenever the size of the window changes)
        bulletBackShader.loadFromMemory(VertexShader, BulletBackGradient);
        bulletBackShader.setUniform("windowHeight", static_cast<float>(wSize.y)); // this must be set, but only needs to be set once (or whenever the size of the window changes)
    }

    bool alive;
    Type type;
    float radius;
    float x, y;
    float dir, speed;
    sf::Color color;
    sf::CircleShape circle;

    Bullet(Type type, sf::Color color, float radius, float x, float y, float dir, float speed) : alive(true), type(type), radius(radius), x(x), y(y), color(color), dir(dir), speed(speed) {
        switch (type) {
        case orb:
            circle = sf::CircleShape(2.f);
            circle.setOrigin(circle.getRadius(), circle.getRadius());
            circle.setFillColor(sf::Color::Transparent);
            this->frontNode = std::make_shared<DrawableNode>([this](sf::RenderTarget& renderTarget, sf::Transform trans, int calcTick) {
                sf::Color& color = this->color;
                sf::RenderStates states;
                bulletFrontShader.setUniform("colorCenter", sf::Glsl::Vec4(1.f, 1.f, 1.f, 1.f));
                bulletFrontShader.setUniform("center", trans * circle.getPosition());
                bulletFrontShader.setUniform("radius", this->radius * circle.getRadius());
                states.shader = &bulletFrontShader;
                states.transform = trans;
                renderTarget.draw(circle, states);
                });
            this->backNode = std::make_shared<DrawableNode>([this](sf::RenderTarget& renderTarget, sf::Transform trans, int calcTick) {
                sf::Color& color = this->color;
                sf::RenderStates states;
                bulletBackShader.setUniform("colorPrimary", sf::Glsl::Vec4(color.r * div255, color.g * div255, color.b * div255, color.a * div255));
                bulletBackShader.setUniform("center", trans * circle.getPosition());
                bulletBackShader.setUniform("radius", this->radius * circle.getRadius());
                states.shader = &bulletBackShader;
                states.transform = trans;
                renderTarget.draw(circle, states);
                });
            break;
        }
        frontRootNode->addChild((std::shared_ptr<Node>)this->frontNode);
        backRootNode->addChild((std::shared_ptr<Node>)this->backNode);
    }

    // create bullet and put into bullets list
    static std::shared_ptr<Bullet> create(Type type, sf::Color color, float radius, float x, float y, float dir, float speed);

    // run move tick for all bullets
    static void moveTick() {
        for (std::shared_ptr<Bullet> b : bullets) {
            b->tick();
        }
    }

    // move tick
    void tick() {
        // move
        x += std::cos(dir) * speed;
        y += std::sin(dir) * speed;

        // update draw nodes
        frontNode->tf.setScale(this->radius, this->radius);
        frontNode->tf.setPosition(this->x, this->y);
        backNode->tf.setScale(this->radius, this->radius);
        backNode->tf.setPosition(this->x, this->y);
    }
};

# endif