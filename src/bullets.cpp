# include "./bullets.h"
# include "./bulletscript.h"

# include <algorithm>

sf::Vector2u Bullet::wSize = { 0,0 };

std::shared_ptr<Node> Bullet::rootNode = std::make_shared<Node>();
std::shared_ptr<Node> Bullet::frontRootNode = std::make_shared<Node>();
std::shared_ptr<Node> Bullet::backRootNode = std::make_shared<Node>();

std::vector<std::shared_ptr<Bullet>> Bullet::bullets = std::vector<std::shared_ptr<Bullet>>();
std::vector<std::shared_ptr<Bullet>> Bullet::deleteQueue = std::vector<std::shared_ptr<Bullet>>();

std::shared_ptr<Bullet> Bullet::create(Type type, sf::Color color, float radius, float x, float y, float dir, float speed, std::shared_ptr<BulletScript> script) {
    Bullet::bullets.push_back(std::make_shared<Bullet>(type, color, radius, x, y, dir, speed, script));
    return Bullet::bullets.back();
}

sf::Shader Bullet::bulletFrontShader = sf::Shader();
sf::Shader Bullet::bulletBackShader = sf::Shader();

const int Bullet::BULLET_RENDER_RADIUS = 32; // (set to power of 2)
const float Bullet::COLLISION_DIST_SQD = 20 * 20;
const int Bullet::BULLET_DEATH_TIME = 15;

Bullet::Bullet(Type type, sf::Color color, float radius, float x, float y, float dir, float speed, std::shared_ptr<BulletScript> script) {
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
Bullet::Bullet() : Bullet::Bullet(Type::orb, sf::Color::White, 0, 0, 0, 0, 0, nullptr) {}

void Bullet::tick() {
    if (time == 0) { // first frame stuff
        // deep copy script
        script = script->clone();
    }
    if (remove) return;
    // movement
    if (alive) {
        // update scripts
        if (!scriptFinished)
            if (script->apply(*this))
                scriptFinished = true;

        // move
        x += std::cos(dir) * speed;
        y += std::sin(dir) * speed;
        if (accel != 0) {
            speed += accel;
            if ((accel > 0 && speed > accelCap) || (accel < 0 && speed < accelCap)) speed = accelCap;
        }

        // collisions
        if (std::pow(Player::pos.x - x, 2) + std::pow(Player::pos.y - y, 2) <= COLLISION_DIST_SQD) {
            kill();
        }
    }

    // update texture
    if (updateTexture) {
        updateTexture = false;
        createTexture();
    }

    // update draw 
    static float INV_BDT = 1.f / BULLET_DEATH_TIME;
    static float INV_BRR = 1.f / BULLET_RENDER_RADIUS;
    float s = (alive ? 1 : (BULLET_DEATH_TIME - this->time) * INV_BDT) * radius * INV_BRR;
    frontNode->tf.setScale(s, s);
    frontNode->tf.setPosition(this->x, this->y);
    backNode->tf.setScale(s, s);
    backNode->tf.setPosition(this->x, this->y);

    // update time
    time++;

    // remove
    if (!alive && time >= BULLET_DEATH_TIME) {
        remove = true;
        frontRootNode->removeChild(this->frontNode);
        backRootNode->removeChild(this->backNode);
    }
}

float Bullet::leftX = NAN;
float Bullet::rightX = NAN;
float Bullet::topY = NAN;
float Bullet::bottomY = NAN;