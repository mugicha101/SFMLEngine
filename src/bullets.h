# include "./scenegraph.h"

# include <SFML/Graphics.hpp>
# include <memory>
# include <list>
# include <vector>

class Bullet {
private:
    std::shared_ptr<DrawableNode> node;
public:
    bool alive;

    static std::shared_ptr<Node> rootNode;
    static std::list<std::shared_ptr<Bullet>> bullets;

    Bullet(std::shared_ptr<DrawableNode> node) : node(node) {}

    // create bullet and put into bullets list
    static std::shared_ptr<Bullet> create(std::shared_ptr<DrawableNode> node);

    // run move tick for all bullets
    static void moveTick() {
        for (std::shared_ptr<Bullet> b : bullets) {
            b->tick();
        }
    }

    // move tick
    void tick() {

    }
};