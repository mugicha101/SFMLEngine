#include <vector>
#include <unordered_map>

#include <SFML/Graphics.hpp>

class Input {
public:
	Input() : pressed(false), just(false) {}

	static void inputEvent(sf::Keyboard::Key key, bool pressed) {
		KeyEvent::events.emplace_back(key, pressed);
	}

	// advance state by a tick
	static void inputTick() {
		// change just to false
		for (auto& kvp : idInputMap)
			kvp.second.tick();

		// update state of inputs that got updated
		for (const KeyEvent& e : KeyEvent::events) {
			if (keyInputMap.find(e.key) == keyInputMap.end()) continue;
			keyInputMap[e.key]->update(e.pressed);
		}
		KeyEvent::events.clear();
	}

	// maps a key to an input (note: only one input per key allowed)
	static void mapInput(sf::Keyboard::Key key, std::string id) {
		keyInputMap[key] = &idInputMap[id];
	}

	friend static bool isPressed(std::string id) {
		return get(id).pressed;
	}

	static bool isReleased(std::string id) {
		return !get(id).pressed;
	}

	static bool justPressed(std::string id) {
		Input& input = get(id);
		return input.pressed && input.just;
	}

	static bool justReleased(std::string id) {
		Input& input = get(id);
		return !input.pressed && input.just;
	}

	static bool justChanged(std::string id) {
		return get(id).just;
	}
private:
	struct KeyEvent {
		static std::vector<KeyEvent> events;

		sf::Keyboard::Key key;
		bool pressed;

		KeyEvent(sf::Keyboard::Key key, bool pressed) : key(key), pressed(pressed) {}
	};

	static Input nullInput;
	static std::unordered_map<std::string, Input> idInputMap;
	static std::unordered_map<sf::Keyboard::Key, Input*> keyInputMap;

	static Input& get(std::string id) {
		return idInputMap.count(id) == 0 ? idInputMap[id] : nullInput;
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
};