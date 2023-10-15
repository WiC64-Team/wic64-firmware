#include "wic64.h"
#include "data.h"

#include "WString.h"

namespace WiC64 {
    class Url : public String {
        public: static const char* TAG;

        public:
            Url()=default;
            Url(const String& string) : String(string) { };

            bool isHostWic64Net(void);
            bool fetchesProgramFile(void);

            void encode(Data* data);
            void sanitize();
            void expand();
    };
}