# ifndef BULLETSCRIPT_H
# define BULLETSCRIPT_H

# include <SFML/Graphics.hpp>
# include "./bullets.h"
# include <memory>

class BulletScript {
protected:
    
public:
    BulletScript() {}

    // returns whether this event is finished
    virtual bool apply(Bullet& b) {};

    virtual void reset() {};
};

class MoveScript : public BulletScript {
protected:
    float x;
    float y;
    bool relative;
public:
    MoveScript(sf::Vector2f pos, bool relative) : x(pos.x), y(pos.y), relative(relative) {}
    MoveScript(float x, float y, bool relative) : x(x), y(y), relative(relative) {}

    bool apply(Bullet& b) override {
        if (relative) {
            b.x += x;
            b.y += y;
        } else {
            b.x = x;
            b.y = y;
        }
        return true;
    }
};

class DirScript : public BulletScript {
protected:
    float val;
    bool relative;
public:
    DirScript(float val, bool relative) : val(val), relative(relative) {}

    bool apply(Bullet& b) override {
        if (relative)
            b.dir += val;
        else
            b.dir = val;
        return true;
    }
};

class ColorScript : public BulletScript {
protected:
    sf::Color color;
public:
    ColorScript(sf::Color color) : color(color) {}
    bool apply(Bullet& b) override {
        if (b.color == color) return 0;
        b.color = color;
        b.updateTexture = true;
        return true;
    }
};

class SpeedScript : public BulletScript {
protected:
    float amount;
    bool relative;
public:
    SpeedScript(float amount, bool relative) : amount(amount), relative(relative) {}

    bool apply(Bullet& b) override {
        if (relative)
            b.speed += amount;
        else
            b.speed = amount;
        return true;
    }
};

class AccelScript : public BulletScript {
protected:
    float amount;
    float cap;
    bool waitUntilCapHit; // waits until acceleration finished before moving to next script
public:
    AccelScript(float amount, float cap, bool waitUntilCapHit) : amount(amount), cap(cap), waitUntilCapHit(waitUntilCapHit) {}

    bool apply(Bullet& b) override {
        b.rotAccel = amount;
        b.rotAccelCap = cap;
        return !waitUntilCapHit || b.speed == b.rotAccelCap;
    }
};

class RotateEnableScript : public BulletScript {
protected:
    bool setOriginToPos;
public:
    RotateEnableScript(bool setOriginToPos) : setOriginToPos(setOriginToPos) {}

    bool apply(Bullet& b) override {
        if (b.rotate == true) {
            if (setOriginToPos) {
                b.rotOrigin = { b.x, b.y };
                b.rotDist = 0;
            }
            return true;
        }
        b.rotate = true;
        if (setOriginToPos) {
            b.rotOrigin = { b.x, b.y };
            b.rotDist = 0;
        }
        else {
            b.rotDist = std::sqrtf(std::pow(b.x - b.rotOrigin.x, 2) + std::pow(b.y - b.rotOrigin.y, 2));
            b.dir = std::atan2f(b.rotOrigin.y - b.y, b.rotOrigin.x - b.x);
        }
        return true;
    }
};

class RotateDisableScript : public BulletScript {
protected:
    bool keepVelocity;
public:
    RotateDisableScript(bool keepVelocity) : keepVelocity(keepVelocity) {}

    bool apply(Bullet& b) {
        if (b.rotate == false) return 0;
        b.rotate = false;
        if (keepVelocity) { // preserve direction of current movement from rotation
            float dist = b.rotDist + b.speed;
            float dir = b.dir + b.rotSpeed;
            float dx = std::cosf(dir) * dist - b.x;
            float dy = std::sinf(dir) * dist - b.y;
            b.dir = std::atan2f(dy,dx);
        }
        return true;
    }
};

class WaitTimeScript : public BulletScript {
protected:
    unsigned int frames;
    unsigned int currentFrames;
public:
    WaitTimeScript(unsigned int frames) : frames(frames), currentFrames(0) {}

    bool apply(Bullet& b) {
        return currentFrames++ >= frames;
    }

    void reset() {
        currentFrames = 0;
    }
};

class WaitUntilDist : public BulletScript {
protected:
    float distSqd;
    bool within; // if true, waits until player within dist, else, waits until player outside of dist
public:
    WaitUntilDist(float dist, bool within) : distSqd(dist * dist), within(within) {}

    bool apply(Bullet& b) {
        bool outside = (
            std::pow(Player::pos.x - b.x, 2) +
            std::pow(Player::pos.y - b.y, 2)
            ) > distSqd;
        return within ^ outside;
    }
};

// a sequential collection of scripts
class Thread : public BulletScript {
protected:
    std::vector<BulletScript> scripts;
    bool loop; // if true, loops
    int index;
public:
    Thread(std::vector<BulletScript> scripts, bool loop) : scripts(scripts), loop(loop), index(0) {}

    bool apply(Bullet& b) override {
        if (index >= scripts.size())
            return true;
        while (true) {
            if (!scripts[index++].apply(b)) return false;
            if (index == scripts.size()) {
                if (loop)
                    index = 0;
                else
                    return true;
            }
        }
    }

    void reset() override {
        index = 0;
        for (BulletScript& script : scripts)
            script.reset();
    }
};

// a parallel collection of scripts
class Bundle : public BulletScript {
protected:
    std::vector<BulletScript> scripts;
    std::vector<bool> active;
    int activeCount;
public:
    Bundle(std::vector<BulletScript> scripts) : scripts(scripts), activeCount(0) {
        active = std::vector<bool>(scripts.size(), true);
    }

    bool apply(Bullet& b) override {
        if (activeCount == scripts.size()) return true;
        for (int i = 0; i < scripts.size(); ++i) {
            if (active[i]) {
                if (!scripts[i].apply(b)) {
                    active[i] = false;
                    activeCount++;
                }
            }
        }
        return true;
    }

    void reset() override {
        activeCount = 0;
        for (int i = 0; i < scripts.size(); ++i) {
            scripts[i].reset();
            active[i] = true;
        }
    }
};

// bullet script factory
class BSF {
    // changes position of bullet
    static BulletScript move(float x, float y) {
        return MoveScript(x, y, true);
    }

    // sets position of bullet
    static BulletScript goTo(float x, float y) {
        return MoveScript(x, y, false);
    }

    // changes direction of bullet
    static BulletScript turn(float dir) {
        return DirScript(dir, true);
    }

    // sets direction of bullet
    static BulletScript dir(float dir) {
        return DirScript(dir, false);
    }

    // sets color of bullet
    static BulletScript color(sf::Color color) {
        return ColorScript(color);
    }

    // changes speed of bullet
    static BulletScript changeSpeed(float speed) {
        return SpeedScript(speed, true);
    }

    // sets speed of bullet
    static BulletScript setSpeed(float speed) {
        return SpeedScript(speed, false);
    }

    // sets acceleration of bullet
    static BulletScript accel(float amount, float cap, bool waitUntilCapHit) {
        return AccelScript(amount, cap, waitUntilCapHit);
    }

    // enables rotation movement (if flag set, rotation origin set to current pos)
    static BulletScript enableRotate(bool setOriginToPos) {
        return RotateEnableScript(setOriginToPos);
    }

    // disables rotation movement (if flag set, velocity maintained)
    static BulletScript disableRotate(bool keepVelocity) {
        return RotateDisableScript(keepVelocity);
    }

    // waits a set amount of frames
    static BulletScript wait(unsigned int frames) {
        return WaitTimeScript(frames);
    }

    // waits until player within a certain range of bullet
    static BulletScript waitUntilInside(float dist) {
        return WaitUntilDist(dist, true);
    }

    // waits until player outside a certain range of bullet
    static BulletScript waitUntilOutside(float dist) {
        return WaitUntilDist(dist, false);
    }

    // create thread
    static BulletScript thread(std::vector<BulletScript> scripts, bool loop) {
        return Thread(scripts, loop);
    }

    // create bundle
    static BulletScript bundle(std::vector<BulletScript> scripts) {
        return Bundle(scripts);
    }
};

# endif