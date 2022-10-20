# include "./bullets.h"

sf::Vector2u Bullet::wSize = { 0,0 };

std::shared_ptr<Node> Bullet::rootNode = std::make_shared<Node>();
std::shared_ptr<Node> Bullet::frontRootNode = std::make_shared<Node>();
std::shared_ptr<Node> Bullet::backRootNode = std::make_shared<Node>();

std::vector<std::shared_ptr<Bullet>> Bullet::bullets = std::vector<std::shared_ptr<Bullet>>();

std::shared_ptr<Bullet> Bullet::create(Type type, sf::Color color, float radius, float x, float y, float dir, float speed) {
    Bullet::bullets.push_back(std::make_shared<Bullet>(type, color, radius, x, y, dir, speed));
    return Bullet::bullets.back();
}

sf::Shader Bullet::bulletFrontShader = sf::Shader();
sf::Shader Bullet::bulletBackShader = sf::Shader();

const int Bullet::BULLET_RENDER_RADIUS = 32; // (set to power of 2)
const float Bullet::COLLISION_DIST_SQD = 20 * 20;
const int Bullet::BULLET_DEATH_TIME = 15;