#pragma once

#include "ResourceHandles.h"
#include "GameTypes.h"

#include <string>

namespace edl {

class EventChain : public res::Keychain<SpecialEvent> {
public:
    void add(SpecialEvent e) {
        res::Keychain<SpecialEvent>::add(e.name, e);
    }
};

void globalEventAdd(EventChain& eventchain);

}