#include "GameEngine.h"

#include "ResourceSystem.h"

const float CARD_ROTATION_INTERVAL = 10.0f;
const float CARD_HOVER_INTERVAL = 5.0f;
const float CARD_HOVER_MIN = 1.5f;
const float CARD_HOVER_MAX = 2.5f;
const float CARD_TILT_DEGREES = 70.0f;

void updateCardAnimation(edl::GameObject* object, glm::vec3 pos, float time) {
    assert(object != nullptr);

    float scaledRotation = glm::radians(360.0f * fmod(time, CARD_ROTATION_INTERVAL) / CARD_ROTATION_INTERVAL); // So it should make a full rotation every CARD_ROTATION_INTERVAL
    glm::quat rotation = glm::quat(glm::vec3(0, scaledRotation, 0));

    glm::quat tilt = glm::quat(glm::vec3(glm::radians(CARD_TILT_DEGREES), 0, 0));

    float hoverInterval = glm::sin(glm::radians(360.0f * fmod(time, CARD_HOVER_INTERVAL) / CARD_HOVER_INTERVAL));
    float scaledHover = (hoverInterval + 1.0f) / 2.0f;
    scaledHover = scaledHover * (CARD_HOVER_MAX - CARD_HOVER_MIN) + CARD_HOVER_MIN;

    object->position = pos + glm::vec3(0, scaledHover, 0);
    object->rotation = rotation *tilt;

    object->calculateTransform();
    object->setEnabled(true);
}

void createObject(edl::ResourceSystem& system, edl::GameObject* object) {
    system.renderables.push_back({});
    edl::Renderable& r = system.renderables.back();
    r.mvpHandle = edl::getStorageBufferIndex(system.transformBuffer);
    r.parent = object;
    object->component = &r;
    object->setEnabled(false);
}

void CardInventory::setup(edl::res::Toolchain& toolchain) {
    edl::ResourceSystem& system = toolchain.getTool<edl::ResourceSystem>("system");
    edl::ObjectRegistry& registry = toolchain.getTool<edl::ObjectRegistry>("ObjectRegistry");

    leftObject = &registry.createObject("CardInventory_Left");
    selObject = &registry.createObject("CardInventory_Select");
    rightObject = &registry.createObject("CardInventory_Right");

    createObject(system, leftObject);
    createObject(system, selObject);
    createObject(system, rightObject);
}

void CardSpawner::setup(edl::res::Toolchain& toolchain) {
    edl::ResourceSystem& system = toolchain.getTool<edl::ResourceSystem>("system");
    edl::ObjectRegistry& registry = toolchain.getTool<edl::ObjectRegistry>("ObjectRegistry");

    for (int i = 0; i < MAX_NUM_CARDS; i++) {
        cardHolders[i] = &registry.createObject("CardHolder_" + std::to_string(i));
        createObject(system, cardHolders[i]);
    }

    nextSpawn = MIN_SPAWN_TIME;
}

void CardSpawner::spawnCard(edl::res::Toolchain& toolchain, const std::string& cardName, glm::vec3 pos) {
    // Despawn the oldest if it's full
    if (spawnedCount >= MAX_NUM_CARDS) despawnOldest();

    // Find the next spawnable card holder
    while (active[nextHolder]) {
        nextHolder++;
        if (nextHolder >= MAX_NUM_CARDS) nextHolder = 0;
    }
    
    cardPosition[nextHolder] = pos;

    // Spawn it
    active[nextHolder] = true;
    edl::GameObject& holder = *cardHolders[nextHolder];
    //holder.setEnabled(true);
    holder.scale = glm::vec3(1.0f, 1.0f, 1.0f);

    // Determine the card and set model
    //TODO: This
    //TEMP:
    edl::ResourceSystem& system = toolchain.getTool<edl::ResourceSystem>("system");
    ((edl::Renderable*)holder.component)->model = edl::hashString("CardModel");

    spawnedCount++;
}

void CardSpawner::update(edl::res::Toolchain& toolchain, float delta) {
    if (pause) return;

    bool spawn = false;

    nextSpawn -= delta;
    if (nextSpawn <= 0) {
        spawn = true;
        nextSpawn = (float(rand()) / float((RAND_MAX))) * (MAX_SPAWN_TIME - MIN_SPAWN_TIME) + MIN_SPAWN_TIME;
    }

    // Spawning
    if (spawn) {
        //Choose position;
        float x = (float(rand()) / float((RAND_MAX))) * (MAX_SPAWN_FIELD.x - MIN_SPAWN_FIELD.x) + MIN_SPAWN_FIELD.x;
        float z = (float(rand()) / float((RAND_MAX))) * (MAX_SPAWN_FIELD.y - MIN_SPAWN_FIELD.y) + MIN_SPAWN_FIELD.y;
        glm::vec3 pos = glm::vec3(x, 0, z);

        spawnCard(toolchain, "", pos);
    }

    // Animate it
    for (int i = 0; i < MAX_NUM_CARDS; i++) {
        if (!active[i]) continue;

        cardTime[i] += delta;
        updateCardAnimation(cardHolders[i], cardPosition[i], cardTime[i]);
    }

}