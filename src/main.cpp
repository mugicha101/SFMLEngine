#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/Audio.hpp>

#include <vector>
#include <list>
#include <memory>
#include <chrono>
#include <iostream>
#include <filesystem>
#include <random>

#include <cmath>

#include "./input.h"
#include "./audio.cpp"
#include "./scenegraph.h"
#include "./bullets.h"

#define DEBUG_TIMER true

#if DEBUG_TIMER
// execution time tracker
class ExecTimer {
private:
    std::chrono::high_resolution_clock::time_point startTime;
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

float randDir() {
    static std::default_random_engine e;
    static std::uniform_real_distribution<> randDir(0, M_PI * 2);
    return randDir(e);
}

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

    sceneGraph.root->addChild(Bullet::rootNode);
    Bullet::init(window.getSize());
    Bullet::rootNode->tf.setPosition(window.getSize().x * 0.5f, window.getSize().y * 0.5f);
    
    for (int i = 0; i < 500; ++i)
        Bullet::create(Bullet::Type::orb, sf::Color::Red, 15, rand() % window.getSize().x - window.getSize().x * 0.5f, rand() % window.getSize().y - window.getSize().y * 0.5f, randDir(), 1.0f);

    // setup sounds
    std::unordered_map<std::string, SoundEffect> sounds;

    SoundEffect s("resources/audio/sound/seUseSpellCard.wav");
    s.play();

    MusicTrack m("resources/audio/music/IntoTheAbyssStart.ogg", "resources/audio/music/IntoTheAbyssLoop.ogg");
    m.play();

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
        // clean sounds
        for (auto& kvp : sounds)
            kvp.second.clean();
        m.checkLoop();

        // spawn bullets

        // move bullets
        Bullet::moveTick();

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

#if DEBUG_TIMER
        drawTimer.record();
        frameTimer.record();
#endif

        // DEBUG STEP
#if DEBUG_TIMER
        if (calcTick % FPS == 0)
            printf("tick %d: %s %s %s %s\n", calcTick, inputTimer.log().c_str(), calcTimer.log().c_str(), drawTimer.log().c_str(), frameTimer.log().c_str());
#endif

        // DISPLAY
        window.display();
        calcTick++;
    }
}