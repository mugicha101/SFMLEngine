#include <SFML/Graphics.hpp>

#include <SFML/Window.hpp>

#include "../src/input.cpp"

int main()
{
    // setup window
    auto window = sf::RenderWindow{ { 1280, 960 }, "CMake SFML Project" };
    window.setFramerateLimit(60);
    window.setKeyRepeatEnabled(false);

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
        window.clear(sf::Color(255, 0, 0));
        window.display();
    }
}