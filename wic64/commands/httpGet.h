#ifndef WIC64_HTTPGET_H
#define WIC64_HTTPGET_H

#include "command.h"
#include "data.h"
#include "WString.h"

namespace WiC64 {
    class HttpGet : public Command {
        public: static const char* TAG;
        private:
            bool m_isProgramFile = false;

        public:
            using Command::Command;
            bool isProgramFile() { return m_isProgramFile; }

            void encode(String& url);
            void sanitize(String& url);
            void analyze(const String& url);
            void expand(String& url);

            void execute(void);
            void responseReady(void);
    };
}
#endif // WIC64_HTTPGET_H
