#ifndef WIC64_HTTP_H
#define WIC64_HTTP_H

#include "command.h"
#include "data.h"
#include "url.h"

namespace WiC64 {
    class Http : public Command {
        public: static const char* TAG;
        private:
            Url m_url;

        public:
            using Command::Command;
            bool supportsProtocol();
            bool supportsQueuedRequest();
            const char* describe(void);

            bool isEncoded(void);

            void execute(void);
            void get(void);
            void postUrl(void);
            void postData(void);

            void lieAboutResponseSizeForProgramFile(void);
            void sendRequestedResponseOnStatusCode201(void);

            void responseReady(void);
    };
}
#endif // WIC64_HTTP_H
