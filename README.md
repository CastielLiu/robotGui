# vedioTransmission
vedio and audio transmission based on socket in qt

server/ 远程转发服务器程序

videoTransmission/ qt客户端

release.bat 生成qt可执行程序依赖文件（可在其他windows上运行）
将可执行文件和此批处理文件复制到新建文件夹中，然后双击运行批处理文件即可  

scp.bat 将server文件夹中的代码上传到服务器
(原来将scp.bat文件放在了server文件夹下，但总是循环执行且不成功，移到外部可用)
