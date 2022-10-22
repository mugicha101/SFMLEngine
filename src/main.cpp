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
#include "./bulletscript.h"

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

    std::shared_ptr<DrawableNode> playerSprite = DrawableNode::create();
    std::shared_ptr<ArraySprite> playerBase = ArraySprite::create({ "resources/graphics/NuvenMove.png", "resources/graphics/NuvenCharge.png" });
    std::shared_ptr<ArraySprite> playerExtra = ArraySprite::create({ "resources/graphics/NuvenConstructOff.png", "resources/graphics/NuvenConstructOn.png" });
    sf::CircleShape orb;
    orb.setFillColor(sf::Color::White);
    
    std::shared_ptr<DrawableNode> playerOrb = DrawableNode::create([&orb](sf::RenderTarget& renderTarget, sf::Transform& trans, int ticks) {
        float radius = Player::charge * 20.f * (1 + 0.1f * std::sin(ticks * M_PI / 3.f));
        orb.setOutlineColor(Player::charge == 1? sf::Color::Red : sf::Color::Yellow);
        if (Input::isPressed("charge")) {
            orb.setRadius(radius * 0.75f);
            orb.setOutlineThickness(radius * 0.25f);
            orb.setOrigin(orb.getRadius(), orb.getRadius());
            orb.setPosition(0, -40);
            renderTarget.draw(orb, trans);
        } else {
            orb.setRadius(radius * 0.375f);
            orb.setOutlineThickness(radius * 0.125f);
            orb.setOrigin(orb.getRadius(), orb.getRadius());
            orb.setPosition(-12, 11);
            renderTarget.draw(orb, trans);
            orb.setPosition(12, 11);
            renderTarget.draw(orb, trans);
        }
        });
    playerBase->tf.setOrigin(32, 36);
    playerExtra->tf.setOrigin(32, 36);
    sceneGraph.root->addChild(playerSprite);
    playerSprite->addChild(playerExtra);
    playerSprite->addChild(playerOrb);
    playerSprite->addChild(playerBase);
    playerSprite->tf.setScale(2.0f, 2.0f);

    sceneGraph.root->addChild(Bullet::rootNode);
    Bullet::init(window.getSize(), window.getSize().x * -0.5f, window.getSize().x * 0.5f, window.getSize().y * -0.5f, window.getSize().y * 0.5f);
    Bullet::rootNode->tf.setPosition(window.getSize().x * 0.5f, window.getSize().y * 0.5f);

    auto rainbow = [](float t) {
        int r = std::round(255 * std::sin(t * 2.f * M_PI));
        int g = std::round(255 * std::sin((t + 1.f / 3.f) * 2.f * M_PI));
        int b = std::round(255 * std::sin((t + 2.f / 3.f) * 2.f * M_PI));
        return sf::Color(r, g, b, 255);
    };
    
    // create background
    std::shared_ptr<Node> starField = Node::create();
    sceneGraph.root->addChild(starField);

    // setup sounds
    std::unordered_map<std::string, SoundEffect> sounds;

    SoundEffect s("resources/audio/sound/seUseSpellCard.wav");

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
    Input::mapInput(sf::Keyboard::Space, "charge");
    Input::mapInput(sf::Keyboard::LShift, "charge");

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
        // TODO: add ability to do multiple calc ticks in order to account for draw lag
        // clean sounds
        for (auto& kvp : sounds)
            kvp.second.clean();
        m.checkLoop();

        // update background

        // spawn bullets
        std::shared_ptr<BulletScript> bs = BSF::thread({
        BSF::accel(-0.1f, 3.f , false),
        BSF::waitUntilOffscreen(),
        BSF::kill()
            });
        for (int i = 0; i < 2; ++i)
            Bullet::create(Bullet::Type::orb, rainbow(calcTick / 750.f), 15, 0, -200, randDir(), 5.f, bs);


        // move bullets
        Bullet::moveTick(calcTick);

        // move player
        sf::Vector2f movement;
        float speed = Input::isPressed("charge") ? 2.f : 6.f;
        float tilt = Input::isPressed("charge") ? 0 : 15;
        if (Input::isPressed("up"))
            movement.y -= 1;
        if (Input::isPressed("down"))
            movement.y += 1;
        if (Input::isPressed("left"))
            movement.x -= 1;
        if (Input::isPressed("right"))
            movement.x += 1;
        Player::pos += movement * speed;
        playerBase->tf.setRotation(tilt * movement.x);
        static sf::Vector2f offset = sf::Vector2f(window.getSize().x * 0.5f, window.getSize().y * 0.5f);
        playerSprite->tf.setPosition(Player::pos + offset);
        playerBase->setIndex(Input::isPressed("charge") ? 1 : 0);
        bool on = Player::charge == 1.f || sin(calcTick * (M_PI/12)) + 1 < Player::charge * 2;
        playerExtra->setIndex((int)on);
        playerExtra->tf.setScale(0.25f + 1.25f * Player::charge, 1.5f);

        // charge
        if (Input::justReleased("charge") && Player::charge == 1) {
            s.play();
            Player::charge = 0;
        }
        if (Input::isPressed("charge")) {
            static float chargeAmount = 1.f / 600.f;
            Player::charge += chargeAmount;
            if (Player::charge >= 1.f) Player::charge = 1.f;
        }
        else {
            static float releaseAmount = 1.f / 200.f;
            Player::charge -= releaseAmount;
            if (Player::charge < 0.f)
                Player::charge = 0;
        }

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