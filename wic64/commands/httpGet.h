#ifndef WIC64_HTTPGET_H
#define WIC64_HTTPGET_H

#include "command.h"
#include "data.h"
#include "WString.h"

namespace WiC64 {
    class HttpGet : public Command {
        private:
            Data* m_response;

        public:
            using Command::Command;
            static void expandURL(String &url);

            ~HttpGet();
            Data* execute(void);
    };
}
#endif // WIC64_HTTPGET_H
