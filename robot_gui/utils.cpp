#include <utils.h>

namespace utils {

void systemCmd(const std::string& cmd)
{
    std::string dstCmd = "gnome-terminal -e \"" +cmd +"\"";
    system(dstCmd.c_str());
}

}
