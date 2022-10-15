#include <SFML/Audio.hpp>
#include <SFML/Audio/SoundBuffer.hpp>

#include <list>

class SoundEffect {
private:
	sf::SoundBuffer buffer;
	std::list<sf::Sound> sounds;
public:
	SoundEffect(std::string path) {
		buffer.loadFromFile(path);
	}

	void play() {
		sounds.emplace_back(buffer);
		sounds.back().play();
	}

	void clean() {
		while (!sounds.front().getStatus())
			sounds.pop_front();
	}
};

class Music {
private:
public:
};