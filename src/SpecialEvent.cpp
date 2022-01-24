#include "SpecialEvent.h"

#include "GameObject.h"

namespace edl {

void weatherRainStart(res::Toolchain& toolchain, const std::string& character, float delta) {
    // Start something
    ObjectRegistry& registry = toolchain.getTool<ObjectRegistry>("ObjectRegistry");
    GameObject& farmer = registry.getObject("Farmer");


}


void weatherRainOngoing(res::Toolchain& toolchain, const std::string& character, float delta) {
    // Do something
}


void weatherRainEnd(res::Toolchain& toolchain, const std::string& character, float delta) {
    // End Something
}

void globalEventAdd(EventChain& eventchain) {

    SpecialEvent weatherRain = {};
    weatherRain.name = "weatherRain"; // the name
    weatherRain.duration = 30.0f; // the duration in milliseconds?
    weatherRain.start = weatherRainStart; // what function should be called when the event is started
    weatherRain.ongoing = weatherRainOngoing; // what function should be called every frame the event is active
    weatherRain.end = weatherRainEnd; // what function should be called when the event is ended

    eventchain.add(weatherRain);
}

}