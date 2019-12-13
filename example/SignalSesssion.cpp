//
// Created by xueyuegui on 19-8-25.
//

//#include "SignalSesssion.h"
#include "SignalServer.h"
#include "SignalSesssion.h"
#include "MyThreadPool.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "Player/PlayerProxy.h"
#include "Common/MediaSource.h"

#include "MyIce/MyLoop.h"

#define TEST 1

typedef struct rtp_header
{
#if __BYTE_ORDER == __BIG_ENDIAN
    uint16_t version:2;
	uint16_t padding:1;
	uint16_t extension:1;
	uint16_t csrccount:4;
	uint16_t markerbit:1;
	uint16_t type:7;
#elif __BYTE_ORDER == __LITTLE_ENDIAN
    uint16_t csrccount:4;
    uint16_t extension:1;
    uint16_t padding:1;
    uint16_t version:2;
    uint16_t type:7;
    uint16_t markerbit:1;
#endif

    uint16_t seq_number;
    uint32_t timestamp;
    uint32_t ssrc;
    uint32_t csrc[16];
} rtp_header;

using namespace rapidjson;

SignalSesssion::SignalSesssion(boost::asio::io_service *ptr_io_service ,SignalServer* server,connection_hdl hdl)
:m_server(server),m_ioservice(ptr_io_service),m_hdl(hdl)
{
}

void SignalSesssion::on_Open() {
#if TEST
    websocket_server::connection_ptr conn = m_server->GetServer()->get_con_from_hdl(m_hdl);
    std::cout << conn->get_uri()->str()  << " : " << conn->get_uri()->get_resource() << " - "<< conn->get_remote_endpoint() << std::endl;

    std::string connid = "1";
    IceConfig cfg;//you may need init the cfg value
    cfg.network_interface = "localhost";
    cfg.media_type = VIDEO_TYPE;
    cfg.connection_id = 1;
    cfg.ice_components = 1;
    cfg.min_port = 50000;
    cfg.max_port = 60000;
    cfg.transport_name = "test";
    cfg.use_nicer = true;
    std::vector<RtpMap> rtp_mappings;//you may need to init the mappings
    RtpMap rtpmap;
    rtpmap.clock_rate = 90000;
    rtpmap.media_type =VIDEO_TYPE;
    rtpmap.payload_type = 96;
    rtpmap.encoding_name = "H264";
    rtpmap.channels = 1;
    rtp_mappings.push_back(rtpmap);
    std::vector<erizo::ExtMap> ext_mappings;
    m_webrtcConn = std::make_shared<WebRtcConnection>(MyThreadPool::GetInstance().GetThreadPool()->getLessUsedWorker(),
                                                    connid,
                                                    cfg,
                                                    rtp_mappings,
                                                    ext_mappings,
                                                    this);
    m_webrtcConn->init();

    std::string stream_id = "1";
    std::string stream_label = "1";
    m_pStream = std::make_shared<MediaStream>(m_webrtcConn->getWorker(),
                                              m_webrtcConn,
                                              stream_id,
                                              stream_label,
                                              false);
    m_pStream->setVideoSinkSSRC(12345678);
    m_pStream->init(true);
    m_webrtcConn->addMediaStream(m_pStream);
    m_webrtcConn->createOffer(true,false, true);
#else
    std::vector<RtpMap> rtp_mappings;
    RtpMap rtpmap;
    rtpmap.clock_rate = 90000;
    rtpmap.media_type =VIDEO_TYPE;
    rtpmap.payload_type = 96;
    rtpmap.encoding_name = "H264";
    rtpmap.channels = 1;
    rtp_mappings.push_back(rtpmap);
    m_remotesdp = std::make_shared<SdpInfo>(rtp_mappings);
    m_webrtctransport.reset(new WebRtcTransport(MyLoop::GetLoop()));
    m_webrtctransport->Start();
    Send(m_webrtctransport->GetLocalSdp());
    GetZLMedia();
#endif
}

void SignalSesssion::on_Close() {

}

void SignalSesssion::on_Message(websocket_server::message_ptr msg) {
#if TEST
    std::string str = msg->get_payload();
    Document d;
    if(d.Parse(str.c_str()).HasParseError()){
        printf("parse error!\n");
    }
    if(!d.IsObject()){
        printf("should be an object!\n");
        return ;
    }
    if(d.HasMember("sdp")){
        Value &m = d["sdp"];
        std::string strSdp = m.GetString();
        m_webrtcConn->setRemoteSdp(strSdp);
        m_strRemoteSdp = strSdp;
        return ;
    }
    if(d.HasMember("candidate")){
        Value &v1 = d["candidate"];
        Value &v2 = d["sdpMLineIndex"];
        Value &v3 = d["sdpMid"];
        std::string strCandidate = v1.GetString();
        int mLineIndex = v2.GetInt();
        std::string mid = v3.GetString();

        std::string cand; //parse from candidate
        cand = strCandidate;
        std::string sdp = m_strRemoteSdp;
        sdp += "a=";
        sdp += cand;
        sdp += "\r\n";
        m_webrtcConn->addRemoteCandidate(mid, mLineIndex, sdp);

        return ;
    }
#else
    std::string str = msg->get_payload();
    Document d;
    if(d.Parse(str.c_str()).HasParseError()){
        printf("parse error!\n");
    }
    if(!d.IsObject()){
        printf("should be an object!\n");
        return ;
    }
    if(d.HasMember("sdp")){
        Value &m = d["sdp"];
        std::string strSdp = m.GetString();
        m_strRemoteSdp = strSdp;
        m_remotesdp->initWithSdp(m_strRemoteSdp,"");
        //m_pStream->setRemoteSdp(m_remotesdp);
        return ;
    }

#endif
}

void SignalSesssion::notifyEvent(WebRTCEvent newEvent, const std::string &message) {
    std::cout<<"------newEvent-----"<<std::endl;
    std::cout<< static_cast<int>(newEvent)<<std::endl;
    if(newEvent == CONN_GATHERED ){
        m_webrtcConn->getLocalSdpInfo();
        auto sdp = m_webrtcConn->getLocalSdp();
        sdp+="a=ssrc:12345678 cname:testvideo\r\n";
        //sdp+="a=ssrc:12345678 cname:janusvideo\r\n";
        //sdp+="a=ssrc:12345678 msid:janus janusv0\r\n";
        //sdp+="a=ssrc:12345678 mslabel:janus\r\n";
        //sdp+="a=ssrc:12345678 label:janusa0\r\n";
        Send(sdp);
/*
        auto pSdp = m_webrtcConn->getLocalSdpInfoSync();
        char szsdp[1024 * 10] = { 0 };
        int nssrc = 12345678;
        std::string m_strIP = "192.168.0.104";
        sprintf(szsdp, "v=0\r\no=- 1495799811084970 1495799811084970 IN IP4 %s\r\ns=Streaming Test\r\nt=0 0\r\n"
                       "a=group:BUNDLE video\r\na=msid-semantic: WMS janus\r\n"
                       "m=video 1 RTP/SAVPF 96\r\nc=IN IP4 %s\r\na=mid:video\r\na=sendonly\r\na=rtcp-mux\r\n"
                       "a=ice-ufrag:%s\r\n"
                       "a=ice-pwd:%s\r\na=ice-options:trickle\r\na=fingerprint:sha-256 %s\r\na=setup:actpass\r\na=connection:new\r\n"
                       "a=rtpmap:96 H264/90000\r\n"
                       "a=ssrc:%d cname:janusvideo\r\n"
                       "a=ssrc:%d msid:janus janusv0\r\n"
                       "a=ssrc:%d mslabel:janus\r\n"
                       "a=ssrc:%d label:janusv0\r\n"
                       "a=candidate:%s 1 udp %u %s %d typ %s\r\n",
                m_strIP.c_str(), m_strIP.c_str(),
                pSdp->getUsername(VIDEO_TYPE).c_str(), pSdp->getPassword(VIDEO_TYPE).c_str(),
                pSdp->fingerprint.c_str(),
                nssrc, nssrc, nssrc, nssrc,
                "1", 12345678, m_strIP.c_str(), 9500,
                "host"
        );
        std::string strSdp = szsdp;
        Send(strSdp);
        */
    }
    if(newEvent == CONN_READY){
        GetZLMedia();
    }
}

void SignalSesssion::Send(std::string szsdp){
    m_server->GetServer()->send(m_hdl, szsdp, websocketpp::frame::opcode::text);
}

void SignalSesssion::TaskInLoop(std::function<void()> func) {
    m_ioservice->post(func);
}

void SignalSesssion::GetZLMedia() {
    auto poller = EventPollerPool::Instance().getPoller();

    poller->sync([&](){
        MediaInfo info;
        info._schema = "rtsp";
        info._app = "live";
        info._streamid = "1";
        auto src = mediakit::MediaSource::find(info._schema,
                                               info._vhost,
                                               info._app,
                                               info._streamid,
                                               true);
        if(src){
            std::cout<<"src find"<<std::endl;
            std::shared_ptr<RtspMediaSource> rtspsrc = dynamic_pointer_cast<RtspMediaSource>(src);
            auto reader = rtspsrc->getRing()->attach(poller, true);
            reader->setDetachCB([](){
                std::cout<<"Detach"<<std::endl;
            });

            reader->setReadCB([&](const RtpPacket::Ptr &pack){
                if(pack->type == TrackVideo){
                    BufferRtp::Ptr buffer(new BufferRtp(pack,4));
                    std::shared_ptr<DataPacket> dp = std::make_shared<DataPacket>();
                    dp->length = buffer->size();
                    memcpy(dp->data, buffer->data(), dp->length);
                    rtp_header *header = (rtp_header *)dp->data;
                    header->ssrc = htonl(12345678);
                    dp->comp = 1;
                    dp->type = VIDEO_PACKET;
#if TEST
                    m_pStream->deliverVideoData(dp);
#else
                    //m_pStream->deliverVideoData(dp);
                    m_webrtctransport->WritRtpPacket(dp->data,dp->length);
#endif
                }
            });
            m_reader = reader;
        }

    });
}



