#pragma once

#include <thread>
#include <memory>
#include <atomic>

#include "Common/MultiMediaSourceMuxer.h"
#include "Util/util.h"
#include "Util/TimeTicker.h"

class FFmpegSrc
{
public:
	FFmpegSrc(const string& vhost,
        const string& strApp,
        const string& strId,
        float dur_sec = 0.0,
        bool bEanbleRtsp = true,
        bool bEanbleRtmp = true,
        bool bEanbleHls = true,
        bool bEnableMp4 = false);
	~FFmpegSrc();
    void inputH264(const char* pcData, int iDataLen, uint32_t dts = 0, uint32_t pts = 0);
    void Start();
    void Stop();
    void ThreadEntry();
private:
    std::shared_ptr<MultiMediaSourceMuxer> m_Src;
    SmoothTicker m_aTicker[2];
    std::shared_ptr<std::thread> m_pThread;
    std::atomic<bool> m_bStart;
};