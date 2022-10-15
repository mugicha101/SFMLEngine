# include "./input.h"

std::unordered_map<std::string, Input::Data> Input::idMap = std::unordered_map<std::string, Input::Data>();
std::unordered_map<sf::Keyboard::Key, Input::Data*> Input::keyMap = std::unordered_map<sf::Keyboard::Key, Input::Data*>();
std::vector<std::pair<sf::Keyboard::Key, bool>> Input::events = std::vector<std::pair<sf::Keyboard::Key, bool>>();
Input::Data Input::nullData = Input::Data();

void Input::inputTick() {
    // change just to false
    for (auto& kvp : idMap)
        kvp.second.tick();

    // update state of inputs that got updated
    for (const auto& e : events) {
        if (keyMap.find(e.first) == keyMap.end()) continue;
        keyMap[e.first]->update(e.first, e.second);
    }
    events.clear();
}

void Input::mapInput(sf::Keyboard::Key key, std::string id) {
    keyMap[key] = &idMap[id];
}

Input::Data& Input::get(std::string id) {
    return idMap.count(id) == 0 ? nullData : idMap[id];
}