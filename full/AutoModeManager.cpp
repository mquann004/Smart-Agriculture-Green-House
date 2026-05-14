#include "AutoModeManager.h"

AutoModeManager& AutoModeManager::getInstance() {
    static AutoModeManager instance;
    return instance;
}

void AutoModeManager::initialize() {
    _autoMode = true;
    _lastManualTime = 0;
    _timeoutDuration = 30 * 1000; // 30 giây
    Serial.println("AutoModeManager initialized");
}

void AutoModeManager::setManualMode() {
    _autoMode = false;
    _lastManualTime = millis();
    Serial.println("Switched to MANUAL mode - Auto mode will resume in 30 seconds");
}

void AutoModeManager::setAutoMode() {
    _autoMode = true;
    _lastManualTime = 0;
    Serial.println("Switched to AUTO mode");
}

void AutoModeManager::resetTimer() {
    if (!_autoMode) {
        _lastManualTime = millis();
        Serial.println("Manual mode timer reset");
    }
}

void AutoModeManager::update() {
    if (!_autoMode && _lastManualTime > 0) {
        unsigned long currentTime = millis();
        if (currentTime - _lastManualTime >= _timeoutDuration) {
            setAutoMode();
            if (_timeoutCallback) {
                _timeoutCallback();
            }
        }
    }
}

bool AutoModeManager::isAutoMode() const { return _autoMode; }

void AutoModeManager::setTimeoutCallback(std::function<void()> callback) {
    _timeoutCallback = callback;
}

unsigned long AutoModeManager::getRemainingTime() const {
    if (_autoMode || _lastManualTime == 0) return 0;
    unsigned long elapsed = millis() - _lastManualTime;
    return elapsed >= _timeoutDuration ? 0 : (_timeoutDuration - elapsed) / 1000;
}
