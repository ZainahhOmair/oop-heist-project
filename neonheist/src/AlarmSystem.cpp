#include "AlarmSystem.h"


AlarmSystem* AlarmSystem::instance = nullptr;  

// returns the single instance of AlarmSystem
// creates it if it doesnt exist yet
AlarmSystem& AlarmSystem::getInstance() {
    if (!instance) {
        instance = new AlarmSystem();
    }
    return *instance;
}

void AlarmSystem::trigger() {
    alertlevel = 1;
    alarmTimer = ALARM_DURATION;
}

void AlarmSystem::update(float deltaTime) {
    if (alertlevel == 1) {
        alarmTimer -= deltaTime;
        if (alarmTimer <= 0) {
            reset();
        }
    }
}

void AlarmSystem::reset() {
    alertlevel = 0;
    alarmTimer = 0;
}

int AlarmSystem::getAlertLevel() {
    return alertlevel;
}

bool AlarmSystem::isActive() {
    return alertlevel == 1;
}

const float AlarmSystem::ALARM_DURATION = 10.0f;