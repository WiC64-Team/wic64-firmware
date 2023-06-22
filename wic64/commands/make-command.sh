#!/bin/bash

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
#include \"data.h\"

namespace WiC64 {
    class ${classname} : public Command {
        private:
            Data* m_response;

        public:
            using Command::Command;
            ~${classname}();
            Data* execute(void);
    };
}
#endif // ${guard}
" > "${header}"

echo "Created ${header}"

echo -n "
${include}

namespace WiC64 {

    ${classname}::~${classname}() {
        if (m_response != NULL) {
            delete m_response;
        }
    }

    Data* ${classname}::execute(void) {
        // m_response = new Data(...);
        return m_response;
    }
}
" > "${impl}"

echo "Created ${impl}"

echo
echo -e "Add this line to ../CMakeLists.txt and rebuild:\n\n\t${cmakelist_entry}\n"
echo -e "Add this line at the top of ./commands.h:\n\n\t${include}\n"
echo -e "Insert this line into WIC64_COMMANDS in ./commands.h:\n\n\t${registry_entry}\n"
