//
// Created by xueyuegui on 19-8-15.
//
#include <string>

#include "Player/PlayerProxy.h"
#include "Common/MediaSource.h"

int main(){

    std::string url = "rtsp://192.168.1.108:8554/live";
    PlayerProxy::Ptr player(new PlayerProxy(DEFAULT_VHOST, "live", std::to_string(1).data()));
    //指定RTP over TCP(播放rtsp时有效)
    (*player)[kRtpType] = Rtsp::RTP_TCP;
    //开始播放，如果播放失败或者播放中止，将会自动重试若干次，重试次数在配置文件中配置，默认一直重试
    player->play(url);

    RingBuffer<RtpPacket::Ptr>::RingReader::Ptr reader;
    auto poller = EventPollerPool::Instance().getPoller();
    while (true){
        char c ;
        std::cin>>c;

        poller->sync([&](){
            MediaInfo info;
            info._schema = "rtsp";
            info._app = "live";
            info._streamid = "1";
            auto src = MediaSource::find(info._schema,
                                         info._vhost,
                                         info._app,
                                         info._streamid,
                                         true);
            if(src){
                std::cout<<"src find"<<std::endl;
                std::shared_ptr<RtspMediaSource> rtspsrc = dynamic_pointer_cast<RtspMediaSource>(src);
                reader = rtspsrc->getRing()->attach(poller, true);
                reader->setDetachCB([](){
                    std::cout<<"Detache"<<std::endl;
                });
                reader->setReadCB([](const RtpPacket::Ptr &pack){
                    if(pack->type == TrackVideo){
                        std::cout<<"receive rtp"<<std::endl;
                        std::cout<<pack->ssrc<<std::endl;
                    }
                });

            }

        });

    }


}