#ifndef NODES_H
#define NODES_H

#include <vector>
#include <functional>
#include <list>
#include <memory>
#include <algorithm>

#include <SFML/Graphics.hpp>

// node on scenegraph heirarchy
class Node {
private:
    Node* parent; // parent 

    // sets parent
    void setParent(Node* parent) {
        this->parent = parent;
    }
protected:
    std::list<std::shared_ptr<Node>> childNodes;
public:
    sf::Transformable tf; // local transformable

    // constructor
    Node() : parent(nullptr) {}

    // get children
    const std::list<std::shared_ptr<Node>>& getChildren() {
        return childNodes;
    }

    // add child (return if succeeds)
    bool addChild(std::shared_ptr<Node> child) {
        if (std::count(childNodes.begin(), childNodes.end(), child) != 0) return false;
        childNodes.push_back(child);
        child->setParent(this);
        return true;
    }

    // remove child (return if succeeds)
    bool removeChild(std::shared_ptr<Node> child) {
        int s = childNodes.size();
        childNodes.remove(child);
        return s < childNodes.size();
    }

    // get absoluate transform (recursive call up the scene graph)
    sf::Transform getAbsTransform() {
        return parent->getAbsTransform() * tf.getTransform();
    }

    // draw self and children
    virtual void draw(sf::RenderTarget& target, const sf::Transform &parentTrans, int calcTick) {
        sf::Transform trans = parentTrans * tf.getTransform();
        for (const std::shared_ptr<Node>& node : childNodes) {
            node->draw(target, trans, calcTick);
        }
    }
};

// node that can be drawn
class DrawableNode : public Node {
private:
    std::function<void(sf::RenderTarget&, const sf::Transform&, int)> drawFunction;
public:
    DrawableNode(std::function<void(sf::RenderTarget&, sf::Transform, int)> drawFunction) : drawFunction(drawFunction) {}

    DrawableNode() : drawFunction([](sf::RenderTarget&, sf::Transform, int) {}) {}

    virtual void draw(sf::RenderTarget& target, const sf::Transform& parentTrans, int calcTick) override {
        drawFunction(target, parentTrans * tf.getTransform(), calcTick);
        Node::draw(target, parentTrans, calcTick);
    }

    static std::shared_ptr<DrawableNode> create() {
        return std::make_shared<DrawableNode>();
    }

    static std::shared_ptr<DrawableNode> create(std::function<void(sf::RenderTarget&, sf::Transform, int)> drawFunction) {
        return std::make_shared<DrawableNode>(drawFunction);
    }
};

// node with sprite
class ObjectSprite : public DrawableNode {
protected:
    sf::Sprite sprite;
    void loadTexture(sf::Texture& texture, std::string path) {
        if (!texture.loadFromFile(path)) {
            throw("cannot load texture from path " + path);
        }
    }

    ObjectSprite() : DrawableNode([this](sf::RenderTarget& target, sf::Transform trans, int calcTick) {
        target.draw(sprite, trans);
        }) {}
public:
    sf::Sprite& getSprite() {
        return sprite;
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
    void setIndex(int index) {
        this->index = index;
    }
    int getIndex() {
        return index;
    }
    virtual int size(); // returns size of indexed collection of textures

    virtual void draw(sf::RenderTarget& target, const sf::Transform& parentTrans, int calcTick) override {
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

    virtual void draw(sf::RenderTarget& target, const sf::Transform& parentTrans, int calcTick) override {
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

#endif