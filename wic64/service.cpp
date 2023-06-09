#include "esp32-hal.h"
#include "esp_event.h"

#include "service.h"
#include "userport.h"
#include "utilities.h"

extern Service *service;
extern Userport *userport;

ESP_EVENT_DEFINE_BASE(SERVICE_EVENTS);

Service::Service() {
    esp_event_loop_args_t event_loop_args = {
        .queue_size = 16,
        .task_name = "SESSION",
        .task_priority = 5,
        .task_stack_size = 8192,
        .task_core_id = 1
    };

    esp_event_loop_create(&event_loop_args, &event_loop_handle);

    esp_event_handler_register_with(
        event_loop_handle,
        SERVICE_EVENTS,
        SERVICE_REQUEST_RECEIVED_EVENT,
        onRequestReceived,
        NULL);
}

bool Service::supports(uint8_t api) {
    return api == API_V1_ID;
}

void Service::receiveRequest(uint8_t api) {
    static uint8_t header[HEADER_SIZE];

    if (api == API_V1_ID) {
        userport->receivePartial(header, HEADER_SIZE, parseRequestHeaderV1);
    }
}

void Service::parseRequestHeaderV1(uint8_t *header, uint16_t size) {
    log_d("Parsing v1 request header...");
    hexdump("Header:", (uint8_t*) header, size);

    uint8_t api = API_V1_ID;
    uint8_t id = header[2];
    uint16_t arg_size = (*((uint16_t*) header)) - API_V1_COMMAND_SIZE;
    uint8_t argc = (size > 0) ? 1 : 0;

    service->command = new Service::Command(api, id, argc);

    log_d("command=0x%02X argc=%d size=%d",
        service->command->id(),
        service->command->argc(),
        arg_size);

    if(service->command->argc()) {
        Data* argument = service->command->addArgument(new Data(arg_size));
        if(argument == NULL) {
            log_e("Could not add single argument to V1 command");
            return;
        }
        userport->receive(argument->data(), argument->size(), postRequestReceivedEvent);
    }
}

void Service::postRequestReceivedEvent(uint8_t *ignoreData, uint16_t ignoreSize) {
    log_d("Posting command received event to service event loop");

    esp_event_post_to(
        service->event_loop_handle,
        SERVICE_EVENTS,
        SERVICE_REQUEST_RECEIVED_EVENT,
        NULL, 0, pdMS_TO_TICKS(1000));
}

void Service::onRequestReceived(void* arg, esp_event_base_t base, int32_t id, void* data) {
    log_d("Handling request received event");

    Command *command = service->command;

    log_d("Executing Command 0x%02X:", command->id());
    log_d("Command arguments...");

    for (uint8_t i=0; i<command->argc(); i++) {
        Data* argument = command->argument(i);
        hexdump("Argument:", argument->data(), argument->size());
    }

    static uint8_t response[6] = { 0x00, 0x04, 0xde, 0xad, 0xbe, 0xef };
    service->sendResponse(response, 6);

    delete service->command;
    service->command = NULL;
}

void Service::sendResponse(uint8_t *data, uint16_t size) {
    userport->send(data, size, onResponseSent);
}

void Service::onResponseSent(uint8_t *data, uint16_t size) {
    hexdump("Response sent:", data, size);
}

Service::Data::Data(uint16_t size) : _size(size) {
    _data = (uint8_t*) calloc(_size, sizeof(uint8_t));
}

Service::Data::~Data() {
    if(_data != NULL) {
        free(_data);
    }
}

Service::Command::Command(uint8_t api, uint8_t id, uint8_t argc) : _id(id), _argc(argc) {
    _argv = (Data**) calloc(_argc, sizeof(Data*));
}

Service::Command::~Command()
{
    for (uint8_t i=0; i<_argc; i++) {
        if (_argv[i] != NULL) {
            delete _argv[i];
        }
    }

    if(_argv != NULL) {
        free(_argv);
    }
}

int16_t Service::Command::getNextFreeArgumentIndex() {
    for (uint8_t i=0; i<_argc; i++) {
        if (_argv[i] == NULL) {
            return i;
        }
    }
    return -1;
}

Service::Data* Service::Command::addArgument(Service::Data *argument) {
    int16_t index = getNextFreeArgumentIndex();

    if (index == -1) {
        log_e("All %d arguments have already been added", _argc);
        return NULL;
    }

    _argv[(uint8_t)index] = argument;
    return argument;
}

Service::Data* Service::Command::argument(uint8_t index) {
    return _argv[index];
}
