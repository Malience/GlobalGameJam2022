#include "MeshLoader.h"

#include "ResourceSystem.h"

#include "edl/logger.h"
#include "edl/io.h"

#include <unordered_set>;
#include <iostream>

namespace edl {

void loadVariable(CardSystem& system, const rapidjson::GenericObject<false, rapidjson::Value>& object) {
    assert(object.HasMember("Name"));
    assert(object.HasMember("DefaultValue"));
    assert(object.HasMember("Min"));
    assert(object.HasMember("Max"));
    assert(object.HasMember("MinExceedEvent"));
    assert(object.HasMember("MaxExceedEvent"));

    std::string name = object["Name"].GetString();
    system.variables.insert({ name, {} });
    Variable& v = system.variables.at(name);

    v.name = name;
    v.defaultValue = object["DefaultValue"].GetFloat();
    v.min = object["Min"].GetFloat();
    v.max = object["Max"].GetFloat();
    v.minExceedEvent = object["MinExceedEvent"].GetString();
    v.maxExceedEvent = object["MaxExceedEvent"].GetString();

    system.variableValue.insert({ name, v.defaultValue });
}

void loadCard(CardSystem& system, const rapidjson::GenericObject<false, rapidjson::Value>& object) {
    assert(object.HasMember("Name"));
    assert(object.HasMember("Material"));
    assert(object.HasMember("DefaultEvent"));
    assert(object.HasMember("EventMap"));

    std::string name = object["Name"].GetString();
    system.cards.insert({ name, {} });
    Card& v = system.cards.at(name);

    system.cardnames.push_back(name);

    v.name = name;
    v.material = object["Material"].GetString();
    v.defaultEvent = object["DefaultEvent"].GetString();

    auto& eventmap = object["EventMap"].GetArray();
    for (auto& ptr : eventmap) {
        auto& o = ptr.GetObject();

        std::string character = o["Character"].GetString();
        std::string e = o["Event"].GetString();

        v.eventMap.insert({ character, e });

    }
}

void loadEvent(CardSystem& system, const rapidjson::GenericObject<false, rapidjson::Value>& object) {
    assert(object.HasMember("Name"));
    assert(object.HasMember("Effects"));

    std::string name = object["Name"].GetString();
    system.events.insert({ name, {} });
    Event& v = system.events.at(name);

    v.name = name;

    auto& eventmap = object["Effects"].GetArray();
    for (auto& ptr : eventmap) {
        auto& o = ptr.GetObject();

        Effect e = {};
        std::string action = o["Action"].GetString();

        if (action == "Add") e.action = EffectAction::ADD;
        else if (action == "Sub") e.action = EffectAction::SUB;
        else if (action == "Set") e.action = EffectAction::SET;
        else if (action == "Special") e.action = EffectAction::SPECIAL;

        e.character = o["Character"].GetString();
        e.var = o["Variable"].GetString();
        e.min = o["Min"].GetFloat();
        e.max = o["Max"].GetFloat();

        v.effects.push_back(e);
    }
}

void loadCharacter(CardSystem& system, const rapidjson::GenericObject<false, rapidjson::Value>& object) {
    assert(object.HasMember("Name"));
    assert(object.HasMember("Variables"));

    std::string name = object["Name"].GetString();
    system.characters.insert({ name, {} });
    Character& v = system.characters.at(name);

    v.name = name;

    auto& vars = object["Variables"].GetArray();
    for (auto& ptr : vars) {
        v.variables.push_back(ptr.GetString());
    }
}

void loadCardJSON(res::Toolchain& toolchain, res::Resource& res) {
    edl::ResourceSystem& system = toolchain.getTool<edl::ResourceSystem>("system");
    CardSystem& cardSystem = toolchain.getTool<CardSystem>("CardSystem");

    edl::res::allocateResourceData(res, sizeof(Mesh), *system.allocator);
    Mesh& meshres = edl::res::getResourceData<Mesh>(res);
    
    std::vector<char> data;
    loadFile(res.path, data);

    rapidjson::Document d;
    d.Parse<rapidjson::kParseStopWhenDoneFlag>(data.data(), data.size());
    if (d.HasParseError()) {
        std::cout << "JSON parsing error, code: " << d.GetParseError() << ", offset: " << d.GetErrorOffset() << std::endl;
    }
    assert(d.IsObject());

    if (d.HasMember("Variables")) {
        auto& member = d["Variables"].GetArray();
        for (auto* ptr = member.Begin(); ptr != member.End(); ++ptr) {
            loadVariable(cardSystem, ptr->GetObject());
        }
    }

    if (d.HasMember("Cards")) {
        auto& member = d["Cards"].GetArray();
        for (auto* ptr = member.Begin(); ptr != member.End(); ++ptr) {
            loadCard(cardSystem, ptr->GetObject());
        }
    }

    if (d.HasMember("Events")) {
        auto& member = d["Events"].GetArray();
        for (auto* ptr = member.Begin(); ptr != member.End(); ++ptr) {
            loadEvent(cardSystem, ptr->GetObject());
        }
    }

    if (d.HasMember("Characters")) {
        auto& member = d["Characters"].GetArray();
        for (auto* ptr = member.Begin(); ptr != member.End(); ++ptr) {
            loadCharacter(cardSystem, ptr->GetObject());
        }
    }

    if (d.HasMember("GlobalVariables")) {
        auto& member = d["GlobalVariables"].GetArray();
        for (auto* ptr = member.Begin(); ptr != member.End(); ++ptr) {
            cardSystem.globalVariables.push_back(ptr->GetString());
        }
    }

    res.status = edl::res::ResourceStatus::LOADED;
}

}