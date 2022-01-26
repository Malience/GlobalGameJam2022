#pragma once

#include "GameTypes.h"
#include "GameObject.h"
#include "ResourceHandles.h"
#include "Renderable.h"
#include "bindless_types.h"

#include "glm/glm.hpp"
#include "glm/gtx/quaternion.hpp"

#include <limits>
#include <stdint.h>

const float MAX_DAYLIGHT = 2.5f;
const float MIN_DAYLIGHT = 0.01f;
const float MAX_NIGHTLIGHT = 0.2f;

class TimeManager {
public:

    void update(edl::res::Toolchain& toolchain, float delta) {
        if (pause) return;

        time += delta * timeDilation;

        // Should clamp the value between 0 and 24
        time = fmod(time, 24.0f);

        float scaledRotation = glm::radians(360.0f * time / 24.0f); // So 24.0f should be a full 360 rotation
        glm::vec3 euler = glm::vec3(0, 0, scaledRotation);
        glm::quat rotation = glm::quat(euler);

        if (skyOrb != nullptr) {
            // Rotate the sky orb
            skyOrb->rotation = rotation;
            skyOrb->calculateTransform();
        }

        DirLight& dirlight = toolchain.getTool<DirLight>("DirLight");
        glm::vec3 lightdir = glm::normalize(rotation * glm::vec3(0, 1, 0));

        float scaledLighting = glm::abs(glm::sin(glm::radians(360.0f * (time + 6.0f) / 24.0f)));

        // Night
        if (time >= 6.0f && time <= 18.0f) {
            dirlight.lightDir = glm::vec4(lightdir, 1.0f);
            dirlight.directionalLightPower = scaledLighting * (MAX_NIGHTLIGHT - MIN_DAYLIGHT) + MIN_DAYLIGHT;
        }
        // Day
        else {
            dirlight.lightDir = glm::vec4(-lightdir, 1.0f);
            dirlight.directionalLightPower = scaledLighting * (MAX_DAYLIGHT - MIN_DAYLIGHT) + MIN_DAYLIGHT;
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

const float INTERACT_RANGE = 15.0f;

const glm::vec2 MIN_SPAWN_FIELD = glm::vec2(3, 3);
const glm::vec2 MAX_SPAWN_FIELD = glm::vec2(70, 70);

struct Ray {
    glm::vec3 pos;
    glm::vec3 dir;
    float range;
};

class Interactable : public edl::GameComponent {
public:

    void update(edl::res::Toolchain& toolchain, float delta) {

    }

    bool collide(const Ray& ray, float& t) {
        if (!parent->getEnabled()) return false;
        if (ray.dir.x == 0.0f && ray.dir.y == 0.0f && ray.dir.z == 0.0f) return false;

        glm::vec3 parentPos = parent->position;

        glm::vec3 aabbMin = min + parentPos;
        glm::vec3 aabbMax = max + parentPos;

        if (ray.dir.x == 0.0f && (ray.pos.x < aabbMin.x || ray.pos.x > aabbMax.x)) return false;
        if (ray.dir.y == 0.0f && (ray.pos.y < aabbMin.y || ray.pos.y > aabbMax.y)) return false;
        if (ray.dir.z == 0.0f && (ray.pos.z < aabbMin.z || ray.pos.z > aabbMax.z)) return false;

        float invdx = ray.dir.x == 0.0f ? std::numeric_limits<float>::infinity() : 1.0f / ray.dir.x;
        float invdy = ray.dir.y == 0.0f ? std::numeric_limits<float>::infinity() : 1.0f / ray.dir.y;
        float invdz = ray.dir.z == 0.0f ? std::numeric_limits<float>::infinity() : 1.0f / ray.dir.z;

        float txmin = ray.dir.x > 0.0f ? (aabbMin.x - ray.pos.x) * invdx : (aabbMax.x - ray.pos.x) * invdx;
        float txmax = ray.dir.x > 0.0f ? (aabbMax.x - ray.pos.x) * invdx : (aabbMin.x - ray.pos.x) * invdx;

        float tymin = ray.dir.y > 0.0f ? (aabbMin.y - ray.pos.y) * invdy : (aabbMax.y - ray.pos.y) * invdy;
        float tymax = ray.dir.y > 0.0f ? (aabbMax.y - ray.pos.y) * invdy : (aabbMin.y - ray.pos.y) * invdy;

        if (txmin > tymax || tymin > txmax) return false;
        float tmin = tymin > txmin ? tymin : txmin;
        float tmax = tymin > txmin ? tymin : txmin;

        float tzmin = ray.dir.z > 0.0f ? (aabbMin.z - ray.pos.z) * invdz : (aabbMax.z - ray.pos.z) * invdz;
        float tzmax = ray.dir.z > 0.0f ? (aabbMax.z - ray.pos.z) * invdz : (aabbMin.z - ray.pos.z) * invdz;

        if (tmin > tzmax || tzmin > tmax) return false;
        tmin = tmin > tzmin ? tzmin : tmin;

        // Ray collides but not in range
        if (tmin > ray.range) return false;

        t = tmin;
        return true;
    }

    glm::vec3 min;
    glm::vec3 max;
};

const int MAX_NUM_CARDS = 10;
const glm::vec3 MIN_AABB = {-1.0f, -3.0f, -1.0f};
const glm::vec3 MAX_AABB = {1.0f, 3.0f, 1.0f};

class InteractionSystem {
public:
    void setup(edl::res::Toolchain& toolchain);
    void update(edl::res::Toolchain& toolchain, float delta);

private:
    Interactable interactables[MAX_NUM_CARDS];
};

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
    std::string cardNames[MAX_NUM_CARDS];
    bool active[MAX_NUM_CARDS];
    float cardTime[MAX_NUM_CARDS];
    glm::vec3 cardPosition[MAX_NUM_CARDS];
    edl::GameObject* cardHolders[MAX_NUM_CARDS];

    int spawnedCount;
    float nextSpawn;
    int nextHolder;

    bool pause = false;

    friend class InteractionSystem;
};

class GameEngine {
public:

    void setup(edl::res::Toolchain& toolchain) {
        // Add everything to the toolchain
        toolchain.add("GameEngine", this);
        toolchain.add("TimeManager", &timeManager);
        toolchain.add("CardSpawner", &spawner);
        toolchain.add("InteractionSystem", &interactionSystem);

        edl::ObjectRegistry& registry = toolchain.getTool<edl::ObjectRegistry>("ObjectRegistry");

        if (registry.hasObject("SkyOrb")) {
            edl::GameObject& skyOrb = registry.getObject("SkyOrb");
            timeManager.setObject(&skyOrb);
        }

        spawner.setup(toolchain);
        interactionSystem.setup(toolchain);
    }

    void update(edl::res::Toolchain& toolchain, float delta) {
        if (pause) return;

        // Apply time dilation
        delta *= timeDilation;

        timeManager.update(toolchain, delta);
        spawner.update(toolchain, delta);
        interactionSystem.update(toolchain, delta);
    }

    void setTimeDilation(float dilation) { timeDilation = dilation; }
    void togglePause() { pause = !pause; }

private:
    float timeDilation = 1.0f;
    bool pause = false;


    TimeManager timeManager;
    CardSpawner spawner;
    InteractionSystem interactionSystem;
};