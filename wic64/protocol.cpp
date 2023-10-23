#include "protocol.h"
#include "protocols/legacy.h"
#include "protocols/standard.h"
#include "protocols/extended.h"

namespace WiC64 {

    const std::map<uint8_t, Protocol*> Protocol::m_protocols = {
        { LEGACY,   new Legacy(LEGACY,     "legacy",   3, 2) },
        { STANDARD, new Standard(STANDARD, "standard", 3, 3) },
        { EXTENDED, new Extended(EXTENDED, "extended", 5, 5) }
    };

    bool Protocol::exists(uint8_t id) {
        return get(id) != nullptr;
    }

    Protocol* Protocol::get(uint8_t id) {
        if (m_protocols.find(id) != m_protocols.end()) {
            return m_protocols.at(id);
        }
        return nullptr;
    }
}