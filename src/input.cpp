#include <vector>
#include <unordered_map>
#include <unordered_set>

#include <SFML/Graphics.hpp>

class Input {
private:
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

	static std::unordered_map<std::string, Data>& getIdMap() {
		static auto* idMap = new std::unordered_map<std::string, Data>();
		return *idMap;
	}

	static std::unordered_map<sf::Keyboard::Key, Data*>& getKeyMap() {
		static auto* keyMap = new std::unordered_map<sf::Keyboard::Key, Data*>();
		return *keyMap;
	}

	static std::vector<std::pair<sf::Keyboard::Key, bool>>& getEvents() {
		static auto* events = new std::vector<std::pair<sf::Keyboard::Key, bool>>();
		return *events;
	}

	static Data& getNullData() {
		static auto* nullData = new Data();
		return *nullData;
	}

	static Data& get(std::string id) {
		return getIdMap().count(id) == 0 ? getNullData() : getIdMap()[id];
	}
public:
	static void inputEvent(sf::Keyboard::Key key, bool pressed) {
		getEvents().emplace_back(key, pressed);
	}

	// advance state by a tick
	static void inputTick() {
		// change just to false
		for (auto& kvp : getIdMap())
			kvp.second.tick();

		// update state of inputs that got updated
		for (const auto& e : getEvents()) {
			if (getKeyMap().find(e.first) == getKeyMap().end()) continue;
			getKeyMap()[e.first]->update(e.first, e.second);
		}
		getEvents().clear();
	}

	// maps a key to an input (note: only one input per key allowed)
	static void mapInput(sf::Keyboard::Key key, std::string id) {
		getKeyMap()[key] = &getIdMap()[id];
	}

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