#ifndef SCENEGRAPH_H
#define SCENEGRAPH_H

#include "./nodes.h"

// scene graph
class SceneGraph {
private:
public:
    std::shared_ptr<Node> root;
    sf::RenderTarget* renderTarget;

    SceneGraph(sf::RenderTarget& renderTarget) {
        this->renderTarget = &renderTarget;
        root = std::make_shared<Node>();
    }

    void drawTick(int calcTick) {
        renderTarget->clear();
        root->draw(*renderTarget, sf::Transform::Identity, calcTick);
    }

    static std::shared_ptr<Node> create() {
        return std::make_shared<Node>();
    }
};

#endif