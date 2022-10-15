# include "./bullets.h"

std::shared_ptr<Node> Bullet::rootNode = std::make_shared<Node>();
std::list<std::shared_ptr<Bullet>> Bullet::bullets = std::list<std::shared_ptr<Bullet>>();

std::shared_ptr<Bullet> Bullet::create(std::shared_ptr<DrawableNode> node) {
    Bullet::bullets.push_back(std::make_shared<Bullet>(node));
    return Bullet::bullets.back();
}