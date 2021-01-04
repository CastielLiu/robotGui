#include <utils.h>
#include <iostream>
#include "globalvariable.h"

namespace utils {

/*@param bg_operation 是否后台运行
 *
 */
void systemCmd(const std::string& cmd, bool bg_operation=false)
{
  std::string dstCmd;
  if(bg_operation)
    dstCmd = "gnome-terminal -e \"" + g_cmdDir.toStdString() + cmd +" &\"";
  else
    dstCmd = "gnome-terminal -e \"" + g_cmdDir.toStdString() + cmd +"\"";

  std::cout << "command: " << dstCmd << std::endl;
  system(dstCmd.c_str());
}

}
