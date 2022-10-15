# include "./scenegraph.h"

# include <SFML/Graphics.hpp>
# include <memory>
# include <list>
# include <vector>

class Bullet {
private:
	std::shared_ptr<DrawableNode> node;
public:
	static std::shared_ptr<Node>& getBulletsNode() {
		static auto* bulletsNode = new std::shared_ptr<Node>();
		return *bulletsNode;
	}

	static std::list<Node>& getBullets() {
		static auto* bullets = new std::list<Node>();
		return *bullets;
	}

	Bullet(std::shared_ptr<DrawableNode> node) {
		this->node = node;
		
	}
};