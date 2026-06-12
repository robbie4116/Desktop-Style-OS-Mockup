#include "logic/clock_format.h"
namespace csopesy {
std::string formatClock(const std::tm& t) {
    char buf[64];
    std::strftime(buf, sizeof(buf), "%A, %b %d, %Y | %I:%M %p", &t);
    return std::string(buf);
}
}
