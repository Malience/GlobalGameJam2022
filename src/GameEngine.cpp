#include "GameEngine.h"

#include "ResourceSystem.h"

const float CARD_ROTATION_INTERVAL = 10.0f;
const float CARD_HOVER_INTERVAL = 5.0f;
const float CARD_HOVER_MIN = 1.5f;
const float CARD_HOVER_MAX = 2.5f;
const float CARD_TILT_DEGREES = 70.0f;

void updateCardAnimation(edl::GameObject* object, glm::vec3 pos, float time, float verticalScale = 1.0f) {
    assert(object != nullptr);

    float scaledRotation = glm::radians(360.0f * fmod(time, CARD_ROTATION_INTERVAL) / CARD_ROTATION_INTERVAL); // So it should make a full rotation every CARD_ROTATION_INTERVAL
    glm::quat rotation = glm::quat(glm::vec3(0, scaledRotation, 0));

    glm::quat tilt = glm::quat(glm::vec3(glm::radians(CARD_TILT_DEGREES), 0, 0));

    float hoverInterval = glm::sin(glm::radians(360.0f * fmod(time, CARD_HOVER_INTERVAL) / CARD_HOVER_INTERVAL));
    float scaledHover = (hoverInterval + 1.0f) / 2.0f;
    scaledHover = scaledHover * (CARD_HOVER_MAX - CARD_HOVER_MIN) + CARD_HOVER_MIN;
    scaledHover *= verticalScale;

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
    object->components.insert({ "Renderable", &r });
    object->setEnabled(false);
}

void CardSystem::update(edl::res::Toolchain& toolchain, float delta) {

}


void CardSystem::activate(std::string& card, std::string& character) {
    if (cards.find(card) == cards.end()) {
        std::cout << "Card not found: " << card << std::endl;
        return;
    }
    auto& cardObject = cards.at(card);

    std::string e = "";
    if (cardObject.eventMap.find(character) == cardObject.eventMap.end()) {
        e = cardObject.defaultEvent;
    }
    else {
        e = cardObject.eventMap.at(character);
    }
    activateEvent(e, character);
    
}


void CardSystem::activateEvent(std::string& e, std::string& character) {
    if (e == "") return;
    if (events.find(e) == events.end()) {
        std::cout << "Event not found: " << e << std::endl;
        return;
    }

    auto& eventObject = events.at(e);
    for (auto& effect : eventObject.effects) {
        if (effect.action == edl::EffectAction::SPECIAL) {
            //TODO:
            continue;
        }

        if (variables.find(effect.var) == variables.end()) {
            std::cout << "Variable not found: " << effect.var << " on event: " << e << std::endl;
            continue;
        }
        auto& var = variables.at(effect.var);
        
        float& value = variableValue.at(effect.var);
        float preval = value;

        float r = ((float(rand()) / float((RAND_MAX))) * (effect.max - effect.min) + effect.min);

        if (effect.action == edl::EffectAction::ADD) {
            value += r;
        }
        else if (effect.action == edl::EffectAction::SUB) {
            value -= r;
        }
        else if (effect.action == edl::EffectAction::SET) {
            value = r;
        }

        std::cout << "Variable: " << effect.var << " is updated from: " << preval << " to: " << value << std::endl;
    }
    
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

void CardInventory::displayCard(edl::res::Toolchain& toolchain, edl::GameObject& o, int card, bool primary, bool left) {
    edl::Camera& camera = toolchain.getTool<edl::Camera>("Camera");
    edl::Renderable& r = (edl::Renderable&)o.getComponent("Renderable");

    o.setEnabled(true);

    glm::vec3 position(0);
    if (primary) {
        position = PRIMARY_OFFSET;
    }
    else if (left) {
        position = LEFT_OFFSET;
    }
    else {
        position = RIGHT_OFFSET;
    }

    glm::quat q = glm::quat(camera.getForward());
    position = camera.getPosition() + camera.getForward() * position.z + camera.getUp() * position.y + camera.getRight() * position.x;
    //o.position = position;

    if (primary) {
        o.scale = glm::vec3(PRIMARY_CARD_SCALE);
    }
    else {
        o.scale = glm::vec3(SECONDARY_CARD_SCALE);
    }

    o.calculateTransform();

    //TEMP
    auto& system = toolchain.getTool<CardSystem>("CardSystem");
    ((edl::Renderable*)o.components.at("Renderable"))->model = edl::hashString(system.cards.at(cardNames[card]).material);

    updateCardAnimation(&o, position, cardTime[card], primary ? PRIMARY_CARD_HOVER_SCALE : SECONDARY_CARD_HOVER_SCALE);
}

void CardInventory::update(edl::res::Toolchain& toolchain, float delta) {
    edl::Camera& camera = toolchain.getTool<edl::Camera>("Camera");

    static bool PRESS_Q = false;
    if (glfwGetKey(camera.window, GLFW_KEY_Q) == GLFW_RELEASE) {
        PRESS_Q = false;
    }

    if (glfwGetKey(camera.window, GLFW_KEY_Q) == GLFW_PRESS && !PRESS_Q) {
        cycleLeft();
        PRESS_Q = true;
    }

    static bool PRESS_E = false;
    if (glfwGetKey(camera.window, GLFW_KEY_E) == GLFW_RELEASE) {
        PRESS_E = false;
    }

    if (glfwGetKey(camera.window, GLFW_KEY_E) == GLFW_PRESS && !PRESS_E) {
        cycleRight();
        PRESS_E = true;
    }

    static bool PRESS_R = false;
    if (glfwGetKey(camera.window, GLFW_KEY_R) == GLFW_RELEASE) {
        PRESS_R = false;
    }

    if (glfwGetKey(camera.window, GLFW_KEY_R) == GLFW_PRESS && !PRESS_R) {
        if (nextCard > 0) {
            std::string name = dropCard();

            CardSpawner& spawner = toolchain.getTool<CardSpawner>("CardSpawner");
            PlayerMotion& motion = toolchain.getTool<PlayerMotion>("PlayerMotion");
            spawner.spawnCard(toolchain, name, motion.getPosition());
        }

        PRESS_R = true;
    }

    if (nextCard > 0) {
        // Animate it
        for (int i = 0; i < nextCard; i++) {
            //if (!active[i]) continue;

            cardTime[i] += delta;
        }

        int leftCard = nextCard > 2 ? nextCard - 1 : 0;
        int rightCard = nextCard > 1 ? 1 : 0;

        displayCard(toolchain, *selObject, 0, true, false);

        if (leftCard > 0) {
            displayCard(toolchain, *leftObject, leftCard, false, true);
        }
        else {
            leftObject->setEnabled(false);
        }

        if (rightCard > 0) {
            displayCard(toolchain, *rightObject, rightCard, false, false);
        }
        else {
            rightObject->setEnabled(false);
        }
    }
    else {
        selObject->setEnabled(false);
    }

}

void InteractionSystem::setup(edl::res::Toolchain& toolchain) {
    CardSpawner& spawner = toolchain.getTool<CardSpawner>("CardSpawner");

    for (int i = 0; i < MAX_NUM_CARDS; i++) {
        Interactable& interactable = interactables[i];
        interactable.min = MIN_AABB;
        interactable.max = MAX_AABB;

        spawner.cardHolders[i]->addComponent("Interactable", interactable);
    }


    auto& registry = toolchain.getTool<edl::ObjectRegistry>("ObjectRegistry");
    CardSystem& cardSystem = toolchain.getTool<CardSystem>("CardSystem");
    charcount = 0;
    for (auto& e : cardSystem.characters) {
        Interactable& interactable = characters[charcount];
        interactable.min = CHAR_MIN_AABB;
        interactable.max = CHAR_MAX_AABB;

        auto& name = e.first;

        if (!registry.hasObject(name)) {
            edl::log::debug("GameLoading", "Character not found with name: %s", name.c_str());
        }
        else {
            registry.getObject(name).addComponent("Interactable", interactable);
        }

        charnames[charcount] = name;

        charcount++;
    }
}



void InteractionSystem::update(edl::res::Toolchain& toolchain, float delta) {
    edl::Camera& camera = toolchain.getTool<edl::Camera>("Camera");

    static bool PRESS = false;
    if (glfwGetKey(camera.window, GLFW_KEY_F) == GLFW_RELEASE) {
        PRESS = false;
        return;
    }

    if (PRESS || glfwGetKey(camera.window, GLFW_KEY_F) != GLFW_PRESS) return;
    PRESS = true;

    const glm::vec3& forward = camera.getForward();
    const glm::vec3& position = camera.getPosition();

    Ray ray = { position, glm::normalize(forward), INTERACT_RANGE };
    float t = -1.0f;

    for (int i = 0; i < MAX_NUM_CARDS; i++) {
        if (interactables[i].collide(ray, t)) {
            std::cout << "Interacted with card #" << i << " at range " << t << std::endl;

            auto& inventory = toolchain.getTool<CardInventory>("CardInventory");
            auto& spawner = toolchain.getTool<CardSpawner>("CardSpawner");
            if (inventory.getCard(spawner.cardNames[i])) {
                spawner.despawn(i);
            }

            return;
        }
    }

    auto& inventory = toolchain.getTool<CardInventory>("CardInventory");
    if (!inventory.hasCard()) return;

    for (int i = 0; i < charcount; i++) {
        if (characters[i].collide(ray, t)) {

            auto& cardSystem = toolchain.getTool<CardSystem>("CardSystem");
            std::string card = inventory.dropCard();

            std::cout << "Interacted with character: " << charnames[i] << " at range " << t << " with card " << card << std::endl;

            cardSystem.activate(card, charnames[i]);

            break;
        }
    }

    //std::cout << "Didn't interact with anything :(" << std::endl;
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
    cardTime[nextHolder] = 0.0f;
    edl::GameObject& holder = *cardHolders[nextHolder];
    //holder.setEnabled(true);
    holder.scale = glm::vec3(1.0f, 1.0f, 1.0f);

    // Determine the card and set model
    //TODO: This
    //TEMP:
    auto& system = toolchain.getTool<CardSystem>("CardSystem");
    ((edl::Renderable*)holder.components.at("Renderable"))->model = edl::hashString(system.cards.at(cardName).material);

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

        auto& cardSystem = toolchain.getTool<CardSystem>("CardSystem");

        int index = rand() % cardSystem.cardnames.size();
        
        spawnCard(toolchain, cardSystem.cardnames[index], pos);
    }

    // Animate it
    for (int i = 0; i < MAX_NUM_CARDS; i++) {
        if (!active[i]) continue;

        cardTime[i] += delta;
        updateCardAnimation(cardHolders[i], cardPosition[i], cardTime[i]);
    }

}

void PlayerMotion::setup(edl::res::Toolchain& toolchain) {

}

void PlayerMotion::update(edl::res::Toolchain& toolchain, float delta) {
    auto& camera = toolchain.getTool<edl::Camera>("Camera");

    static bool PRESS_SPACE = false;
    if (glfwGetKey(camera.window, GLFW_KEY_SPACE) == GLFW_RELEASE) {
        PRESS_SPACE = false;
    }

    if (glfwGetKey(camera.window, GLFW_KEY_SPACE) == GLFW_PRESS && !PRESS_SPACE) {
        if (jumpheight == 0.0f) {
            velocity = PLAYER_JUMP_FORCE;
        }
        PRESS_SPACE = true;
    }

    bool crouching = false;
    bool sprinting = glfwGetKey(camera.window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;
    // Crouch
    if (glfwGetKey(camera.window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
        anim -= delta * PLAYER_CROUCH_ANIM_SPEED;
        crouching = true;
        if (anim < 0.0f) anim = 0.0f;
    }
    else {
        anim += delta * PLAYER_CROUCH_ANIM_SPEED;
        if (anim >= 1.0f) anim = 1.0f;
    }

    if (velocity > 0.0f || jumpheight > 0.0f) {
        velocity += GRAVITY_FORCE * delta;
        jumpheight += velocity * delta;
        if (jumpheight < 0.0f) {
            jumpheight = 0.0f;
            velocity = 0.0f;
        }
    }

    float height = anim * (PLAYER_HEIGHT - PLAYER_CROUCH_HEIGHT) + PLAYER_CROUCH_HEIGHT + jumpheight + GROUND_Y;

    float speed = crouching ? PLAYER_CROUCH_SPEED : PLAYER_MOVE_SPEED;
    speed *= sprinting ? PLAYER_SPRINT_MULT : 1.0f;

    float forward = glfwGetKey(camera.window, GLFW_KEY_W) == GLFW_PRESS;
    float back = glfwGetKey(camera.window, GLFW_KEY_S) == GLFW_PRESS;
    float left = glfwGetKey(camera.window, GLFW_KEY_A) == GLFW_PRESS;
    float right = glfwGetKey(camera.window, GLFW_KEY_D) == GLFW_PRESS;
    forward *= speed; back *= speed; left *= speed; right *= speed;

    glm::vec3 forwdir = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), camera.getRight()));
    
    playerPosition += forwdir * (forward - back);
    playerPosition += camera.getRight() * (right - left);
    playerPosition.y = height;

    camera.setPos(playerPosition.x, playerPosition.y, playerPosition.z);

    camera.update(delta);
}