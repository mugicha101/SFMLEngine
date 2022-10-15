#include <SFML/Audio.hpp>

#include <list>

class SoundEffect {
private:
	sf::SoundBuffer buffer;
	std::list<sf::Sound> sounds;
public:
	SoundEffect(std::string path) {
		if (!buffer.loadFromFile(path))
			throw("cannot load audio from path " + path);
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

class MusicTrack {
private:
	sf::Music music;
public:
	bool loop;
	MusicTrack(std::string path, bool loop) : loop(loop) {
		if (!music.openFromFile(path))
			throw("cannot load audio from path " + path);
		music.setLoop(loop);
	}

	void play() {
		music.play();
	}
};