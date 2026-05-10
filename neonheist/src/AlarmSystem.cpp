#include "AlarmSystem.h"

// this line is REQUIRED - defines the static pointer that was declared in the .h file
// without this line you get a linker error saying AlarmSystem::instance is undefined
AlarmSystem* AlarmSystem::instance = nullptr;   //whats the instance keyword??

// returns the single instance of AlarmSystem
// creates it if it doesnt exist yet
AlarmSystem& AlarmSystem::getInstance() {
    if (!instance) {
        instance = new AlarmSystem();
    }
    return *instance;
}

// sets alarm to active and starts the countdown timer
void AlarmSystem::trigger() {
    alertlevel = 1;
    alarmTimer = ALARM_DURATION;
}

// called every frame from GameManager::update()
// counts down the timer and resets alarm when time is up
void AlarmSystem::update(float deltaTime) {
    if (alertlevel == 1) {
        alarmTimer -= deltaTime;
        if (alarmTimer <= 0) {
            reset();
        }
    }
}

// turns alarm off completely
// police go back to patrol after this
void AlarmSystem::reset() {
    alertlevel = 0;
    alarmTimer = 0;
}

// returns 1 if alarm is on, 0 if off
int AlarmSystem::getAlertLevel() {
    return alertlevel;
}

// returns true if alarm is currently ringing
bool AlarmSystem::isActive() {
    return alertlevel == 1;
}

// define the constant - alarm lasts 10 seconds
const float AlarmSystem::ALARM_DURATION = 10.0f;