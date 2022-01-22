#pragma once

#include "ResourceHandles.h"

#include <string>

namespace edl {

typedef void (*EventFunction)(edl::res::Toolchain& toolchain, float delta);

struct SpecialEvent {
    std::string name;

    float duration;

    EventFunction start;
    EventFunction ongoing;
    EventFunction end;
};

class EventChain : public res::Keychain<SpecialEvent> {
public:
    void add(SpecialEvent e) {
        res::Keychain<SpecialEvent>::add(e.name, e);
    }
};

void globalEventAdd(EventChain& eventchain);

}