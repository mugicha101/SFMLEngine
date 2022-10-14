#include <vector>

#include <SFML/Graphics.hpp>

// node on scenegraph heirarchy
class Node {
private:
	sf::Transformable parentTf;
protected:
	Node() : parentTf() {}
public:
	std::vector<std::shared_ptr<Node>> childNodes;
	sf::Transformable tf;

	static std::shared_ptr<Node> create() {
		return std::shared_ptr<Node>(new Node());
	}

	// draw self and children
	void draw(sf::RenderTarget& target, const sf::Transform &parentTrans, int calcTick) {
		sf::Transform trans = tf.getTransform() * parentTrans;	
		for (const std::shared_ptr<Node>& node : childNodes) {
			node->draw(target, trans, calcTick);
		}
	}
};

// scene graph
class SceneGraph {
private:
	std::shared_ptr<Node> root;
public:
	SceneGraph() {
		root = Node::create();
	}

	void drawTick(sf::RenderWindow& window, const sf::Transform& windowTrans, int calcTick) {
		root->draw(window, windowTrans, calcTick);
	}
};

// node with sprite
class ObjectSprite : public Node {
protected:
	sf::Sprite sprite;
	void loadTexture(sf::Texture& texture, std::string path) {
		if (!texture.loadFromFile(path)) {
			throw("cannot load texture from path " + path);
		}
	}
	ObjectSprite() {}
public:
	sf::Sprite& getSprite() {
		return sprite;
	}

	void draw(sf::RenderTarget& target, const sf::Transform& parentTrans, int calcTick) {
		// render sprite
		sf::Transform trans = tf.getTransform() * parentTrans;
		target.draw(sprite, trans);

		// super call
		Node::draw(target, parentTrans, calcTick);
	}
};

// node with a single texture
class StaticSprite : public ObjectSprite {
private:
	sf::Texture texture;
public:
	StaticSprite(std::string path) : ObjectSprite() {
		loadTexture(texture, path);
		sprite.setTexture(texture);
	}
};

// node with multiple images associated with indexes
class IndexedSprite : public ObjectSprite {
protected:
	int index;

	IndexedSprite() : index(0) {}

	// update sprite to match index
	virtual void updateSprite();
public:
	int setIndex(int index) {
		this->index = index;
	}
	int getIndex() {
		return index;
	}
	virtual int size(); // returns size of indexed collection of textures

	void draw(sf::RenderTarget& target, const sf::Transform& parentTrans, int calcTick) {
		updateSprite();
		ObjectSprite::draw(target, parentTrans, calcTick);
	}
};

// indexed sprite using an array of textures
class ArraySprite : public IndexedSprite {
private:
	std::vector<sf::Texture> textures;
protected:
	void updateSprite() override {
		sprite.setTexture(textures[index]);
	}
public:
	ArraySprite(std::vector<sf::Texture> textures) : IndexedSprite() {
		std::swap(this->textures, textures);
	}

	int size() override {
		return textures.size();
	}
};

// indexed sprite using a spritesheet texture
class SheetSprite : public IndexedSprite {
protected:
	sf::Texture texture;
	sf::IntRect activeRect;
	int rows;
	int cols;
	void updateSprite() {
		activeRect.left = index % cols;
		activeRect.top = index / cols;
		sprite.setTextureRect(activeRect);
	}
public:
	SheetSprite(std::string spriteSheetPath, int rows, int cols, int imgWidth, int imgHeight) : rows(rows), cols(cols), IndexedSprite() {
		loadTexture(texture, spriteSheetPath);
		activeRect.width = imgWidth;
		activeRect.height = imgHeight;
	}

	void setIndex(int row, int col) {
		IndexedSprite::setIndex(row * cols + col);
	}
};

// node regularly switching between images at a constant delay (wraps indexedsprite)
class AnimatedSprite : public ObjectSprite {
public:
	enum LoopType {
		forward, // plays from index 0 to N then jumps back to 0
		reverse, // plays from index N to 0 then jumps back to N
		bounce // plays from index 0 to N then N to 0
	};
private:
	IndexedSprite sprite;
	const int frameDelay;
	const LoopType loopType;
public:
	AnimatedSprite(IndexedSprite sprite, int frameDelay, LoopType loopType) : sprite(sprite), frameDelay(frameDelay), loopType(loopType) {}
	AnimatedSprite(IndexedSprite sprite, int frameDelay) : AnimatedSprite(sprite, frameDelay, LoopType::forward) {}

	void draw(sf::RenderTarget& target, const sf::Transform& parentTrans, int calcTick) {
		switch (loopType) {
		case forward:
			sprite.setIndex((calcTick / frameDelay) % sprite.size());
			break;
		case reverse:
			sprite.setIndex(sprite.size() - (calcTick / frameDelay) % sprite.size() - 1);
			break;
		case bounce:
			sprite.setIndex(std::abs(
				(calcTick / frameDelay + sprite.size() - 1)
				% ((sprite.size() << 1) - 1)
				- sprite.size() + 1));
			break;
		}
		ObjectSprite::draw(target, parentTrans, calcTick);
	}
};