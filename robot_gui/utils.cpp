#include <utils.h>
#include <iostream>

namespace utils {

void systemCmd(const std::string& cmd)
{
    std::string dstCmd = "gnome-terminal -e \"" +cmd +"\"";
    std::cout << "command: " << dstCmd << std::endl;
    system(dstCmd.c_str());
}

}
