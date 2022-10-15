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
    int status;
    sf::Music startMusic;
    sf::Music loopMusic;
public:
    bool loop;
    MusicTrack(std::string startPath, std::string loopPath) : status(0) {
        if (!startMusic.openFromFile(startPath))
            throw("cannot load audio from path " + startPath);
        if (!loopMusic.openFromFile(loopPath))
            throw("cannot load audio from path " + loopPath);
        loopMusic.setLoop(true);
    }

    void play() {
        status = 1;
        startMusic.play();
    }

    void checkLoop() {
        if (status != 1) return;
        if (!startMusic.getStatus()) {
            loopMusic.play();
            status = 2;
        }
    }
};