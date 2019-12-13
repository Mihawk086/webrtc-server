//
// Created by xueyuegui on 19-12-6.
//

#include "MyIceConnection.h"

using namespace erizo;

erizo::MyIceConnection::MyIceConnection(xop::EventLoop *loop,IceConfig& ice_config)
:IceConnection(ice_config),m_loop(loop)
{
    m_strIp = "192.168.127.128";
    m_nPort = 9500;
    m_ice_server.reset(new IceServer(Utils::Crypto::GetRandomString(4), Utils::Crypto::GetRandomString(24)));
    m_udp_socket.reset(new UdpSocket(m_strIp,m_nPort,loop));

    m_udp_socket->setReadCallback([this](char* buf, int len, struct sockaddr_in* remoteAddr) {
        this->OnPacketReceived(buf,len, remoteAddr);
        return;
    });
    m_ice_server->SetIceServerCompletedCB([this]() {
        this->OnIceServerCompleted();
    });
    m_ice_server->SetSendCB([this](char* buf, int len, struct sockaddr_in* remoteAddr) {
        this->SendPacket(buf, len,remoteAddr);
    });

    ufrag_ = m_ice_server->GetUsernameFragment();
    upass_ = m_ice_server->GetPassword();
}

erizo::MyIceConnection::~MyIceConnection() {

}

void erizo::MyIceConnection::start() {
    CandidateInfo cand_info;
    cand_info.componentId = 1;
    cand_info.foundation = "123";
    cand_info.priority = 123;
    cand_info.hostAddress = m_strIp;
    cand_info.hostPort = m_nPort;
    cand_info.rAddress = m_strIp;
    cand_info.rPort = m_nPort;
    cand_info.hostType = HOST;
    cand_info.mediaType = VIDEO_TYPE;
    cand_info.netProtocol = "udp";
    cand_info.transProtocol = "udp";
    cand_info.username = ufrag_;
    cand_info.password = upass_;
    if (auto listener = getIceListener().lock()) {
        listener->onCandidate(cand_info, this);
    }
    updateIceState(IceState::CANDIDATES_RECEIVED);
}

bool erizo::MyIceConnection::setRemoteCandidates(const std::vector<erizo::CandidateInfo> &candidates, bool is_bundle) {
    return true;
}

void erizo::MyIceConnection::setRemoteCredentials(const std::string &username, const std::string &password) {

}

int erizo::MyIceConnection::sendData(unsigned int component_id, const void *buf, int len) {
    return m_udp_socket->Send((char*)buf, len, m_remoteAddr);
}

void erizo::MyIceConnection::onData(unsigned int component_id, char *buf, int len) {
    packetPtr packet (new DataPacket());
    memcpy(packet->data, buf, len);
    packet->comp = component_id;
    packet->length = len;
    packet->received_time_ms = ClockUtils::timePointToMs(clock::now());
    if (auto listener = getIceListener().lock()) {
        listener->onPacketReceived(packet);
    }
}

erizo::CandidatePair erizo::MyIceConnection::getSelectedPair() {
    return erizo::CandidatePair();
}

void erizo::MyIceConnection::setReceivedLastCandidate(bool hasReceived) {

}

void erizo::MyIceConnection::close() {

}




void erizo::MyIceConnection::OnPacketReceived(char *buf, int len, struct sockaddr_in *remoteAddr) {
    if (RTC::StunPacket::IsStun((const uint8_t*)buf, len)) {
        RTC::StunPacket* packet = RTC::StunPacket::Parse((const uint8_t*)buf, len);
        if (packet == nullptr)
        {
            std::cout << "parse stun error" << std::endl;
            return;
        }
        m_ice_server->ProcessStunPacket(packet, remoteAddr);
    }else{
        onData(1,buf,len);
    }

}

void erizo::MyIceConnection::OnIceServerCompleted() {
    if(ice_state_ != READY) {
        m_remoteAddr = *m_ice_server->GetSelectAddr();
        updateIceState(IceState::READY);
        ice_state_ = READY;

    }
}

int erizo::MyIceConnection::SendPacket(char *buf, int len, struct sockaddr_in *remoteAddr) {

    return m_udp_socket->Send(buf, len, *remoteAddr);
}
