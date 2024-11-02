#pragma once

template<typename T>
class Singleton {
public:
    static T& instance() {
        static T instance;
        return instance;
    }

protected:
    Singleton() {}
    virtual ~Singleton() {}

private:
    Singleton(const Singleton&) = delete;
    Singleton& operator= (const Singleton) = delete;
};
