#include <vector>
#include <unordered_map>

#include <SFML/Graphics.hpp>

class Input {
private:
	struct Data {
		bool pressed;
		bool just;
		
		Data() : pressed(false), just(false) {}

		void tick() {
			just = false;
		}

		void update(bool newPressedState) {
			just = newPressedState != pressed;
			pressed = newPressedState;
		}
	};

	static Data nullData;

	inline static std::unordered_map<std::string, Data> idMap;
	inline static std::unordered_map<sf::Keyboard::Key, Data*> keyMap;
	inline static std::vector<std::pair<sf::Keyboard::Key, bool>> events;

	static Data& get(std::string id) {
		return idMap.count(id) == 0 ? nullData : idMap[id];
	}
public:
	static void inputEvent(sf::Keyboard::Key key, bool pressed) {
		events.emplace_back(key, pressed);
	}

	// advance state by a tick
	static void inputTick() {
		// change just to false
		for (auto& kvp : idMap)
			kvp.second.tick();

		// update state of inputs that got updated
		for (const auto& e : events) {
			if (keyMap.find(e.first) == keyMap.end()) continue;
			keyMap[e.first]->update(e.second);
		}
		events.clear();
	}

	// maps a key to an input (note: only one input per key allowed)
	static void mapInput(sf::Keyboard::Key key, std::string id) {
		keyMap[key] = &idMap[id];
	}

	static bool isPressed(std::string id) {
		return get(id).pressed;
	}

	static bool isReleased(std::string id) {
		return !get(id).pressed;
	}

	static bool justPressed(std::string id) {
		Data& input = get(id);
		return input.pressed && input.just;
	}

	static bool justReleased(std::string id) {
		Data& input = get(id);
		return !input.pressed && input.just;
	}

	static bool justChanged(std::string id) {
		return get(id).just;
	}
};