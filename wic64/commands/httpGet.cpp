
#include "httpGet.h"
#include "client.h"
#include "WString.h"

namespace WiC64 {

    extern Client *client;

    HttpGet::~HttpGet() {
        if (m_response != NULL) {
            delete m_response;
        }
    }

    Data* HttpGet::execute(void) {
        char* url = (char*) request()->argument()->data();
        return m_response = client->get(url);
    }
}
