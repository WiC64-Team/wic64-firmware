#ifndef WIC64_COMMAND_H
#define WIC64_COMMAND_H

#include "esp32-hal-log.h"
#include "service.h"

class Command {
    private:
        Service::Request* m_request;

    public:
        class Undefined;

        static Command* create(Service::Request* request);

        Command(Service::Request* request);
        virtual ~Command();

        Service::Request* request(void) { return m_request; }
        virtual Service::Data* execute(void) { return nullptr; }
};

class Command::Undefined : public Command {
    public:
        using Command::Command;

        Service::Data* execute() {
            log_e("Undefined Command id: 0x%02x", request()->id());
            return new Service::Data(0);
        }
};

#endif // WIC64_COMMAND_H
