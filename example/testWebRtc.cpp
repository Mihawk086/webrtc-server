//
// Created by xueyuegui on 19-8-25.
//

#include <boost/asio.hpp>

#include "SignalServer.h"
#include "MyThreadPool.h"

#include "Player/PlayerProxy.h"
#include "Common/MediaSource.h"
#include "FFmpegSrc.h"

#include <thread>
#include "webrtctransport/Utils.hpp"

int main(){

	FFmpegSrc src("","live","1");
	src.Start();
	Utils::Crypto::ClassInit();
    dtls::DtlsSocketContext::Init();
    MyThreadPool::GetInstance().Start(1);
    boost::asio::io_service ioService;
    SignalServer wserver(&ioService, 3000);
    wserver.Start();
    ioService.run();
}
