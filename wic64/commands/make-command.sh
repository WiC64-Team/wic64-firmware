#!/bin/bash

if [ "$#" -ne 1 ]; then
    echo "Usage: ./make-command.sh <classname>"
    echo
    echo "Arguments:"
    echo "    <classname> : classname in camel-case"
    echo
    echo "Example: ./make-command.sh NewCommand"
    exit 1
fi

name=$1

guard="WIC64_${name^^}_H"
classname="${name^}"
tagname="${name^^}"
basename="${name,}"
header="${basename}.h"
impl="${basename}.cpp"
include="#include \"${header}\""
id_define_example="#define WIC64_CMD_EXAMPLE 0x64"
registry_entry_example="WIC64_COMMAND(WIC64_CMD_EXAMPLE, ${classname}),"
cmakelist_entry="SRCS \"commands/${impl}\""
loglevel_line="esp_log_level_set(${classname}::TAG, loglevel);"

echo -n "#ifndef ${guard}
#define ${guard}

#include \"command.h\"

namespace WiC64 {
    class ${classname} : public Command {
        public:
            static const char* TAG;

            using Command::Command;
            const char* describe(void);
            void execute(void);
    };
}
#endif // ${guard}
" > "${header}"

echo "Created ${header}"

echo -n "
${include}

namespace WiC64 {
    const char* ${classname}::TAG = \"${tagname}\";

    const char* ${classname}::describe() {
        return \"${classname} (short description)\";
    }

    void ${classname}::execute(void) {
        responseReady();
    }
}
" > "${impl}"

echo "Created ${impl}"

echo
echo -e "Add this line to ../CMakeLists.txt and run idf.py reconfigure:\n\n\t${cmakelist_entry}\n"
echo -e "Add this line at the top of ./commands.cpp:\n\n\t${include}\n"
echo -e "Define the command id(s) in ./commands.h, e.g.:\n\n\t${id_define_example}\n"
echo -e "For each defined id, insert a line like this into WIC64_COMMANDS in ./commands.cpp:\n\n\t${registry_entry_example}\n"
echo -e "To enable logging, insert this line into WiC64::loglevel() in wic64.cpp:\n\n\t${loglevel_line}\n"
exit 0