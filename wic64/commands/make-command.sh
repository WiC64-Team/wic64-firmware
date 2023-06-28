#!/bin/bash

if [ "$#" -ne 2 ]; then
    echo "Usage: ./make-command.sh <classname> <id>"
    echo
    echo "Arguments:"
    echo "    <classname> : classname in camel-case"
    echo "    <id>        : command id in hex notation"
    echo
    echo "Example: ./make-command.sh HttpGet 0x01"
    exit 1
fi

name=$1
id=$2

guard="WIC64_${name^^}_H"
classname="${name^}"
basename="${name,}"
header="${basename}.h"
impl="${basename}.cpp"
include="#include \"${header}\""
registry_entry="WIC64_COMMAND(${id}, ${classname}),"
cmakelist_entry="SRCS \"commands/${impl}\""

echo -n "#ifndef ${guard}
#define ${guard}

#include \"command.h\"

namespace WiC64 {
    class ${classname} : public Command {

        public:
            using Command::Command;
            void execute(void);
    };
}
#endif // ${guard}
" > "${header}"

echo "Created ${header}"

echo -n "
${include}

namespace WiC64 {
    void ${classname}::execute(void) {
        responseReady();
    }
}
" > "${impl}"

echo "Created ${impl}"

echo
echo -e "Add this line to ../CMakeLists.txt and rebuild:\n\n\t${cmakelist_entry}\n"
echo -e "Add this line at the top of ./commands.h:\n\n\t${include}\n"
echo -e "Insert this line into WIC64_COMMANDS in ./commands.h:\n\n\t${registry_entry}\n"
exit 0