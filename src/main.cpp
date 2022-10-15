#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#include <vector>
#include <memory>

#include "../src/input.cpp"
#include "../src/scenegraph.cpp"

int main() {
    // setup window
    sf::RenderWindow window = sf::RenderWindow{ { 1280, 960 }, "CMake SFML Project" };
    window.setFramerateLimit(60);
    window.setKeyRepeatEnabled(false);

    // setup scene
    SceneGraph sceneGraph(window);
    sf::CircleShape circleA(100.0f);
    circleA.setFillColor(sf::Color::Green);
    std::shared_ptr<Node> A = DrawableNode::create([&circleA](sf::RenderTarget& renderTarget, sf::Transform trans, int calcTick) {
        renderTarget.draw(circleA, trans);
        });
    A->tf.setOrigin(circleA.getRadius(), circleA.getRadius());
    sf::CircleShape circleB(50.0f);
    std::shared_ptr<Node> B1 = DrawableNode::create([&circleB](sf::RenderTarget& renderTarget, sf::Transform trans, int calcTick) {
        renderTarget.draw(circleB, trans);
        });
    B1->tf.setOrigin(circleB.getRadius(), circleB.getRadius());
    std::shared_ptr<Node> B2 = DrawableNode::create([&circleB](sf::RenderTarget& renderTarget, sf::Transform trans, int calcTick) {
        renderTarget.draw(circleB, trans);
        });
    B2->tf.setOrigin(circleB.getRadius(), circleB.getRadius());
    std::shared_ptr<Node> B3 = DrawableNode::create([&circleB](sf::RenderTarget& renderTarget, sf::Transform trans, int calcTick) {
        renderTarget.draw(circleB, trans);
        });
    B3->tf.setOrigin(circleB.getRadius(), circleB.getRadius());
    B3->tf.setPosition(circleA.getRadius(), circleA.getRadius());
    std::shared_ptr<Node> B4 = DrawableNode::create([&circleB](sf::RenderTarget& renderTarget, sf::Transform trans, int calcTick) {
        renderTarget.draw(circleB, trans);
        });
    B4->tf.setOrigin(circleB.getRadius(), circleB.getRadius());
    B4->tf.setPosition(circleA.getRadius()*2, circleA.getRadius()*2);
    sceneGraph.root->addChild(A);
    A->addChild(B1);
    A->addChild(B2);
    A->addChild(B3);
    A->addChild(B4);
    A->tf.setPosition(300, 300);

    // setup inputs
    Input::mapInput(sf::Keyboard::W, "up");
    Input::mapInput(sf::Keyboard::A, "left");
    Input::mapInput(sf::Keyboard::S, "down");
    Input::mapInput(sf::Keyboard::D, "right");
    Input::mapInput(sf::Keyboard::Up, "up");
    Input::mapInput(sf::Keyboard::Left, "left");
    Input::mapInput(sf::Keyboard::Down, "down");
    Input::mapInput(sf::Keyboard::Right, "right");

    // game loop
    int calcTick = 0;
    while (window.isOpen())
    {
        // event loop
        for (auto event = sf::Event{}; window.pollEvent(event);)
        {
            switch (event.type) {
            case sf::Event::Closed:
                window.close();
                break;
            case sf::Event::KeyPressed:
                Input::inputEvent(event.key.code, true);
                break;
            case sf::Event::KeyReleased:
                Input::inputEvent(event.key.code, false);
                break;
            }
        }
        Input::inputTick();

        // move player
        sf::Vector2f movement;
        float speed = 2.0f;
        if (Input::isPressed("up"))
            movement.y -= 1;
        if (Input::isPressed("down"))
            movement.y += 1;
        if (Input::isPressed("left"))
            movement.x -= 1;
        if (Input::isPressed("right"))
            movement.x += 1;
        A->tf.move(movement * speed);
        B1->tf.move(movement * speed);
        A->tf.setScale(0.5f + movement.x * movement.x, 0.5f + movement.y * movement.y);

        // draw scenegraph
        sceneGraph.drawTick(calcTick);
        
        // display window
        window.display();

        calcTick++;
    }
}