//
// Created by xueyuegui on 19-12-7.
//

#ifndef MYWEBRTC_WEBRTCTRANSPORT_H
#define MYWEBRTC_WEBRTCTRANSPORT_H

#include <memory>
#include <string>

#include "MyDtlsTransport.h"
#include "IceServer.h"
#include "UdpSocket.h"
#include "StunPacket.hpp"
#include "net/EventLoop.h"

#include "logger.h"
#include "SdpInfo.h"
#include "MediaDefinitions.h"
#include "Transport.h"
#include "Stats.h"
#include "bandwidth/BandwidthDistributionAlgorithm.h"
#include "pipeline/Pipeline.h"
#include "thread/Worker.h"
#include "rtp/RtcpProcessor.h"
#include "rtp/RtpExtensionProcessor.h"
#include "lib/Clock.h"
#include "pipeline/Handler.h"
#include "pipeline/Service.h"
#include "rtp/QualityManager.h"
#include "rtp/PacketBufferService.h"

namespace erizo {
    class WebRtcTransport {
    public:
        WebRtcTransport(xop::EventLoop* loop);
        ~WebRtcTransport();

        void Start();
        std::string GetLocalSdp();
        void OnIceServerCompleted();
        void OnDtlsCompleted(std::string clientKey, std::string serverKey);

        void onInputDataPacket(char* buf ,int len ,struct sockaddr_in* remoteAddr);
        void WritePacket(char* buf ,int len,struct sockaddr_in* remoteAddr);
        void WritePacket(char* buf ,int len);
        void WritRtpPacket(char* buf , int len);
        void Write(std::shared_ptr<DataPacket> packet);
    private:
        UdpSocket::Ptr m_pUdpSocket;
        IceServer::Ptr m_IceServer;
        MyDtlsTransport::Ptr m_pDtlsTransport;
        std::shared_ptr<SrtpChannel> m_Srtp;

        char m_ProtectBuf[5000];
        bool m_bReady;
        std::string m_strIP;
        int16_t m_nPort;
        xop::EventLoop* m_loop;
        struct sockaddr_in m_RemoteSockaddr;
        std::vector<RtpMap> rtp_mappings_;
        //RtpExtensionProcessor extension_processor_;
        std::unique_ptr<BandwidthDistributionAlgorithm> distributor_;
        std::vector<std::shared_ptr<MediaStream>> media_streams_;
        std::shared_ptr<SdpInfo> remote_sdp_;
        std::shared_ptr<SdpInfo> local_sdp_;
    };
}

#endif //MYWEBRTC_WEBRTCTRANSPORT_H
