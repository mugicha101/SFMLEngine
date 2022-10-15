#include <vector>
#include <unordered_map>
#include <unordered_set>

#include <SFML/Graphics.hpp>

class Input {
public:
    struct Data {
        std::unordered_set<sf::Keyboard::Key> keysDown;
        bool pressed;
        bool just;
        
        Data() : pressed(false), just(false) {}

        void tick() {
            just = false;
        }

        void update(sf::Keyboard::Key key, bool newPressedState) {
            if (newPressedState) {
                keysDown.insert(key);
            }
            else {
                keysDown.erase(key);
            }
            bool newPressed = keysDown.size() != 0;
            just = newPressed != pressed;
            pressed = newPressed;
        }
    };
private:
    static std::unordered_map<std::string, Data> idMap;
    static std::unordered_map<sf::Keyboard::Key, Data*> keyMap;
    static std::vector<std::pair<sf::Keyboard::Key, bool>> events;
    static Data nullData;

    static Data& get(std::string id);
public:
    static void inputEvent(sf::Keyboard::Key key, bool pressed) {
        events.emplace_back(key, pressed);
    }

    // advance state by a tick
    static void inputTick();

    // maps a key to an input (note: only one input per key allowed)
    static void mapInput(sf::Keyboard::Key key, std::string id);

    static bool isPressed(std::string id) {
        return get(id).pressed;
    }

    static bool isReleased(std::string id) {
        return !get(id).pressed;
    }

    static bool justPressed(std::string id) {
        Data& data = get(id);
        return data.pressed && data.just;
    }

    static bool justReleased(std::string id) {
        Data& data = get(id);
        return !data.pressed && data.just;
    }

    static bool justChanged(std::string id) {
        return get(id).just;
    }
};