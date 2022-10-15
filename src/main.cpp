#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#include <vector>
#include <list>
#include <memory>
#include <chrono>

#include "../src/input.cpp"
#include "../src/scenegraph.cpp"

#define DEBUG_TIMER true

#if DEBUG_TIMER
// execution time tracker
class ExecTimer {
private:
    std::chrono::steady_clock::time_point startTime;
    std::list<float> times;
    const int trials;
    long long trialSum;
public:
    const std::string label;
    ExecTimer(std::string label, int trials) : label(label), trials(trials), trialSum(0) {}

    void start() {
        startTime = std::chrono::high_resolution_clock::now();
    }

    void record() {
        times.push_back((std::chrono::high_resolution_clock::now() - startTime).count());
        trialSum += times.back();
        if (times.size() > trials) {
            trialSum -= times.front();
            times.pop_front();
        }
    }

    long long getAvg() {
        return trialSum / trials;
    }

    std::string log() {
        return label + ": " + (trials == times.size() ? std::to_string(getAvg()/1000)  : "<not enough trials>") + "us";
    }
};
#endif

int main() {
    // setup window
    const int FPS = 60;
    sf::RenderWindow window = sf::RenderWindow{ { 1280, 960 }, "CMake SFML Project" };
    window.setFramerateLimit(FPS);
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

    // setup timing
#if DEBUG_TIMER
    const int TRIALS = FPS;
    ExecTimer inputTimer("input time", TRIALS);
    ExecTimer calcTimer("calc time", TRIALS);
    ExecTimer drawTimer("draw time", TRIALS);
    ExecTimer frameTimer("frame time", TRIALS);
#endif

    // game loop
    int calcTick = 0;
    while (window.isOpen())
    {
#if DEBUG_TIMER
        frameTimer.start();
#endif

        // INPUT STEP
#if DEBUG_TIMER
        inputTimer.start();
#endif

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

        // update input states
        Input::inputTick();

#if DEBUG_TIMER
        inputTimer.record();
#endif

        // CALC STEP
#if DEBUG_TIMER
        calcTimer.start();
#endif

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

#if DEBUG_TIMER
        calcTimer.record();
#endif

        // DRAW STEP
#if DEBUG_TIMER
        drawTimer.start();
#endif

        // draw scenegraph
        sceneGraph.drawTick(calcTick);
        window.display();

#if DEBUG_TIMER
        drawTimer.record();
        frameTimer.record();
#endif

        // DEBUG STEP
#if DEBUG_TIMER
        if (calcTick % FPS == 0)
            printf("tick %d: %s %s %s %s\n", calcTick, inputTimer.log().c_str(), calcTimer.log().c_str(), drawTimer.log().c_str(), frameTimer.log().c_str());
#endif
        calcTick++;
    }
}