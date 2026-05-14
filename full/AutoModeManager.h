#ifndef AUTO_MODE_MANAGER_H
#define AUTO_MODE_MANAGER_H

#include <Arduino.h>
#include <functional>

// ===== AUTO MODE MANAGER (Singleton) =====
// Quản lý chế độ Auto/Manual với timeout tự động quay lại Auto
class AutoModeManager {
public:
    static AutoModeManager& getInstance();

    void initialize();
    void setManualMode();
    void setAutoMode();
    void resetTimer();
    void update();
    bool isAutoMode() const;
    void setTimeoutCallback(std::function<void()> callback);
    unsigned long getRemainingTime() const;

private:
    AutoModeManager() = default;
    ~AutoModeManager() = default;

    bool _autoMode = true;
    unsigned long _lastManualTime = 0;
    unsigned long _timeoutDuration = 30000; // 30 giây
    std::function<void()> _timeoutCallback = nullptr;
};

#endif
