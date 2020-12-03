#ifndef UTILS_FACTORY_H
#define UTILS_FACTORY_H
#include <functional>
#include <map>
#include <string>

#include <QDebug>
#include <QFile>
#include <QString>

QString read(QString Filename);
double clamp(float val, float lower, float upper);
int sign(double val);
void delay(int time);

class Base {
public:
    Base() = default;
    // Destructor of base class must always be virtual
    virtual ~Base() = default;
    virtual std::string getType() {
        return "Base";
    }
};

class Factory {
private:
    using FactoryMap = std::map<std::string, Factory*>;
    // Force global variable to be initialized, thus it avoid
    // the inialization order fisaco.
    static FactoryMap& getRegister() {
        static FactoryMap classRegister{};
        return classRegister;
    }

public:
    /** Register factory object of derived class */
    static void registerFactory(const std::string& name, Factory* factory) {
        auto& reg = Factory::getRegister();
        reg[name] = factory;
    }
    /** Show all registered classes */
    static void showClasses() {
        qDebug() << " Function registry. ";
        qDebug() << " =================== ";
        for (const auto& pair : Factory::getRegister())
            qDebug() << " + " << QString::fromStdString(pair.first) << "\n";
    }

    static bool isRegistered(const std::string& name) {
        auto it = Factory::getRegister().find(name);
        return it != Factory::getRegister().end();
    }

    /**  Construct derived class returning a raw pointer */
    static Base* makeRaw(const std::string& name) {
        auto it = Factory::getRegister().find(name);
        if (it != Factory::getRegister().end())
            return it->second->construct();
        return nullptr;
    }

    /** Construct derived class returning an unique ptr  */
    static std::unique_ptr<Base> makeUnique(const std::string& name) {
        return std::unique_ptr<Base>(Factory::makeRaw(name));
    }

    // Destructor
    virtual ~Factory() = default;

    virtual auto construct() const -> Base* = 0;
};

template <typename DerivedClass>
class ConcreteFactory : Factory {
public:
    // Register this global object on the Factory register
    ConcreteFactory(const std::string& name) {
        qDebug() << " [TRACE] "
                 << " Registered Function = " << QString::fromStdString(name);
        Factory::registerFactory(name, this);
    }
    auto construct() const -> Base* {
        return new DerivedClass;
    }
};


#define SWITCH(str) switch (s_s::str_hash_for_switch(str))
#define CASE(str)                                                                            \
    static_assert(s_s::str_is_correct(str) && (s_s::str_len(str) <= s_s::MAX_LEN),           \
                  "CASE string contains wrong characters, or its length is greater than 9"); \
    case s_s::str_hash(str, s_s::str_len(str))
#define DEFAULT default

namespace s_s {
typedef unsigned char uchar;
typedef unsigned long long ullong;

const uchar MAX_LEN = 9;
const ullong N_HASH = static_cast<ullong>(-1);

constexpr ullong raise_128_to(const uchar power) {
    return 1ULL << 7 * power;
}

constexpr bool str_is_correct(const char* const str) {
    return (static_cast<signed char>(*str) > 0) ? str_is_correct(str + 1) : (*str ? false : true);
}

constexpr uchar str_len(const char* const str) {
    return *str ? (1 + str_len(str + 1)) : 0;
}

constexpr ullong str_hash(const char* const str, const uchar current_len) {
    return *str ? (raise_128_to(current_len - 1) * static_cast<uchar>(*str) + str_hash(str + 1, current_len - 1)) : 0;
}

inline ullong str_hash_for_switch(const char* const str) {
    return (str_is_correct(str) && (str_len(str) <= MAX_LEN)) ? str_hash(str, str_len(str)) : N_HASH;
}

inline ullong str_hash_for_switch(const std::string& str) {
    return (str_is_correct(str.c_str()) && (str.length() <= MAX_LEN)) ? str_hash(str.c_str(), str.length()) : N_HASH;
}
}  // namespace s_s

#endif  // UTILS_H
