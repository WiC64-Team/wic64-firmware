#include "getIP.h"
#include "connection.h"

namespace WiC64 {

    extern Connection* connection;

    GetIP::~GetIP() {
        if (m_response != NULL) {
            delete m_response;
        }
    }

    Data* GetIP::execute(void) {
        return m_response = new Data(connection->getIP());
    }
}