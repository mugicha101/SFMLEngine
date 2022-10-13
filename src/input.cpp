#include <vector>
#include <unordered_map>

#include <SFML/Graphics.hpp>

class Input {
private:
	inline static std::unordered_map<std::string, Input> idInputMap;
	inline static std::unordered_map<sf::Keyboard::Key, Input*> keyInputMap;
	inline static std::vector<std::pair<sf::Keyboard::Key, bool>> events;

	static Input& get(std::string id) {
		return idInputMap[id];
	}

	bool pressed;
	bool just;

	void tick() {
		just = false;
	}

	void update(bool newPressedState) {
		just = newPressedState != pressed;
		pressed = newPressedState;
	}
public:
	Input() : pressed(false), just(false) {}

	static void inputEvent(sf::Keyboard::Key key, bool pressed) {
		events.emplace_back(key, pressed);
	}

	// advance state by a tick
	static void inputTick() {
		// change just to false
		for (auto& kvp : idInputMap)
			kvp.second.tick();

		// update state of inputs that got updated
		for (const auto& e : events) {
			if (keyInputMap.find(e.first) == keyInputMap.end()) continue;
			keyInputMap[e.first]->update(e.second);
		}
		events.clear();
	}

	// maps a key to an input (note: only one input per key allowed)
	static void mapInput(sf::Keyboard::Key key, std::string id) {
		keyInputMap[key] = &idInputMap[id];
	}

	static bool isPressed(std::string id) {
		return idInputMap.count(id) == 0 ? false : get(id).pressed;
	}

	static bool isReleased(std::string id) {
		return idInputMap.count(id) == 0 ? true : !get(id).pressed;
	}

	static bool justPressed(std::string id) {
		if (idInputMap.count(id) == 0) return false;
		Input& input = get(id);
		return input.pressed && input.just;
	}

	static bool justReleased(std::string id) {
		if (idInputMap.count(id) == 0) return false;
		Input& input = get(id);
		return !input.pressed && input.just;
	}

	static bool justChanged(std::string id) {
		return idInputMap.count(id) == 0 ? false : get(id).just;
	}
};