#pragma once

class ISensor {
public:
    virtual ~ISensor() = default;
    virtual bool detect() = 0;
};
