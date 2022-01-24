#pragma once

#include "GameTypes.h"
#include "GameObject.h"
#include "ResourceHandles.h"
#include "Renderable.h"

#include "glm/glm.hpp"
#include "glm/gtx/quaternion.hpp"

#include <stdint.h>

class TimeManager {
public:

    void update(float delta) {
        if (pause) return;

        time += delta * timeDilation;

        // Should clamp the value between 0 and 24
        time = fmod(time, 24.0f);

        if (skyOrb != nullptr) {
            // Rotate the sky orb
            float scaledRotation = glm::radians(360.0f * time / 24.0f); // So 24.0f should be a full 360 rotation
            glm::quat rotation = glm::quat(glm::vec3(0, 0, scaledRotation));
            skyOrb->rotation = rotation;
            skyOrb->calculateTransform();
        }
    }

    float getTime() {
        return time;
    }

    void setTimeDilation(float dilation) { timeDilation = dilation; }
    void togglePause() { pause = !pause; }

    void setObject(edl::GameObject* object) {
        skyOrb = object;
    }

private:
    float time = 0.0f;
    float timeDilation = 1.0f;
    bool pause = false;

    edl::GameObject* skyOrb;
};

class EventSystem {
public:
    void setup(edl::res::Toolchain& toolchain) {

    }

private:

};

class CardSystem {

};

class UISystem {
public:

private:
    std::unordered_map<std::string, edl::GameObject*> elementMap;
};

class CardInventory {
public:
    void setup(edl::res::Toolchain& toolchain);

    void update(edl::res::Toolchain& toolchain, float delta);

private:
    static const int MAX_NUM_CARDS = 5;
    int selectedCard = 0;
    std::string cardNames[MAX_NUM_CARDS];
    bool active[MAX_NUM_CARDS];
    float cardTime[MAX_NUM_CARDS];

    edl::GameObject* leftObject;
    edl::GameObject* selObject;
    edl::GameObject* rightObject;

};

const float MIN_SPAWN_TIME = 2.0f;
const float MAX_SPAWN_TIME = 5.0f;

const glm::vec2 MIN_SPAWN_FIELD = glm::vec2(3, 3);
const glm::vec2 MAX_SPAWN_FIELD = glm::vec2(70, 70);

class CardSpawner {
public:
    void setup(edl::res::Toolchain& toolchain);
    
    void despawnOldest() {
        int index = 0;
        float oldest = 0;
        for (int i = 0; i < MAX_NUM_CARDS; i++) {
            if (cardTime[i] > oldest) {
                index = i;
                oldest = cardTime[i];
            }
        }

        despawn(index);
    }

    std::string despawn(edl::GameObject* object) {
        int index = 0;
        for (int i = 0; i < MAX_NUM_CARDS; i++) {
            if (cardHolders[i] == object) {
                index = i;
                break;
            }
        }

        return despawn(index);
    }

    std::string despawn(int index) {
        active[index] = false;
        cardHolders[index]->setEnabled(false);

        spawnedCount--;

        return cardNames[index];
    }

    void spawnCard(edl::res::Toolchain& toolchain, const std::string& cardName, glm::vec3 pos);

    void update(edl::res::Toolchain& toolchain, float delta);

    void setPause(bool p) { pause = p; }

private:
    static const int MAX_NUM_CARDS = 10;
    std::string cardNames[MAX_NUM_CARDS];
    bool active[MAX_NUM_CARDS];
    float cardTime[MAX_NUM_CARDS];
    glm::vec3 cardPosition[MAX_NUM_CARDS];
    edl::GameObject* cardHolders[MAX_NUM_CARDS];

    int spawnedCount;
    float nextSpawn;
    int nextHolder;

    bool pause = false;
};

class GameEngine {
public:

    void setup(edl::res::Toolchain& toolchain) {
        // Add everything to the toolchain
        toolchain.add("GameEngine", this);
        toolchain.add("TimeManager", &timeManager);
        toolchain.add("CardSpawner", &spawner);

        edl::ObjectRegistry& registry = toolchain.getTool<edl::ObjectRegistry>("ObjectRegistry");

        if (registry.hasObject("SkyOrb")) {
            edl::GameObject& skyOrb = registry.getObject("SkyOrb");
            timeManager.setObject(&skyOrb);
        }

        spawner.setup(toolchain);
    }

    void update(edl::res::Toolchain& toolchain, float delta) {
        if (pause) return;

        // Apply time dilation
        delta *= timeDilation;

        timeManager.update(delta);
        spawner.update(toolchain, delta);
    }

    void setTimeDilation(float dilation) { timeDilation = dilation; }
    void togglePause() { pause = !pause; }

private:
    float timeDilation = 1.0f;
    bool pause = false;


    TimeManager timeManager;
    CardSpawner spawner;
};