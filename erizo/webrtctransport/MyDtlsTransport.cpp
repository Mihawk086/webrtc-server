//
// Created by xueyuegui on 19-12-7.
//

#include "MyDtlsTransport.h"

using namespace erizo;
using dtls::DtlsSocketContext;

MyDtlsTransport::MyDtlsTransport() {
    m_pDtls.reset(new DtlsSocketContext());
    m_pDtls->createServer();
    m_pDtls->setDtlsReceiver(this);
}

MyDtlsTransport::~MyDtlsTransport() {

}


void MyDtlsTransport::Start() {
    //m_pDtls->start();
}

void MyDtlsTransport::Close() {

}

void MyDtlsTransport::onHandshakeCompleted(dtls::DtlsSocketContext *ctx, std::string clientKey, std::string serverKey,
                                           std::string srtp_profile) {
    if(m_HandshakeCompletedCB){
        m_HandshakeCompletedCB(clientKey,serverKey);
    }
}

void MyDtlsTransport::onHandshakeFailed(dtls::DtlsSocketContext *ctx, const std::string &error) {
    if(m_HandshakeFailedCB){
        m_HandshakeFailedCB();
    }
}

void MyDtlsTransport::onDtlsPacket(dtls::DtlsSocketContext *ctx, const unsigned char *data, unsigned int len) {
    OutputData((char*)data,len);
}

bool MyDtlsTransport::isDtlsPacket(const char *buf, int len) {
    int data = DtlsSocketContext::demuxPacket(reinterpret_cast<const unsigned char*>(buf), len);
    switch (data) {
        case DtlsSocketContext::dtls:
            return true;
            break;
        default:
            return false;
            break;
    }
}

void MyDtlsTransport::InputData(char *buf, int len) {
    m_pDtls->read((unsigned char*)buf,len);
}

void MyDtlsTransport::OutputData(char *buf, int len) {
    if(m_OutPutCB){
        m_OutPutCB(buf,len);
    }
}


