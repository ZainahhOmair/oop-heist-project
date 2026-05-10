#ifndef ALARMSYSTEM_H
#define ALARMSYSTEM_H

class AlarmSystem {
    private:
    static AlarmSystem* instance;
    int alertlevel;
    float alarmTimer;
    static const float ALARM_DURATION;
    AlarmSystem() : alertlevel(0),alarmTimer(0.0f){}

    public:
    static AlarmSystem& getInstance();
    void trigger();
    void update(float deltaTime);
    void reset();
    int getAlertLevel();
    bool isActive();

};

#endif