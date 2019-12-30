# webrtc-server
** webrtc流媒体服务器  
** 基于licode的erizo改造  

# 依赖库
```  
sudo apt-get install liblog4cxx-dev  
sudo apt install libavcodec-dev libavdevice-dev libavfilter-dev libavformat-dev libavresample-dev libavutil-dev libpostproc-dev   libswresample-dev libswscale-dev  
sudo apt-get install libboost-all-dev  
```  

# 使用说明
ubuntu18.04安装依赖库  
```  
mkdir build  
cd build   
cmake ..  
make  
```  
运行程序   
修改webrtchtml/js/main.js下websocket的ip地址   
打开webrtchtml/index.html 播放视频 

