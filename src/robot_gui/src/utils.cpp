#include <utils.h>
#include <iostream>
#include "globalvariable.h"

namespace utils {

void systemCmd(const std::string& cmd)
{
    std::string dstCmd = "gnome-terminal -e \"" + g_cmdDir.toStdString() + cmd +"\"";
    std::cout << "command: " << dstCmd << std::endl;
    system(dstCmd.c_str());
}

}
