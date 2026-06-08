/**
 * Copyright (C) 2025, Bruce MacKinnon KC1FSZ
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#ifdef _WIN32
#else
#include <execinfo.h>
#include <signal.h>
#include <pthread.h>
#include <sys/syscall.h> 
#endif 

#include <iostream>
#include <cmath> 
#include <queue>
#include <thread>
#include <sstream>

// 3rd party
#include <curl/curl.h>

// KC1FSZ
#include "kc1fsz-tools/Log.h"
#include "kc1fsz-tools/linux/StdClock.h"
#include "kc1fsz-tools/fixedqueue.h"
#include "kc1fsz-tools/threadsafequeue.h"
#include "kc1fsz-tools/threadsafequeue2.h"
#ifdef _WIN32
#include "kc1fsz-tools/win32/Win32MTLog.h"
#else
#include "kc1fsz-tools/linux/MTLog.h"
#endif

// amp-core
#include "NullLog.h"
#include "LineIAX2.h"
#include "ManagerTask.h"
#include "EventLoop.h"
#include "Bridge.h"
#include "MultiRouter.h"
#include "ThreadUtil.h"
#include "MultiRouter.h"
#include "Poker.h"
#include "TTSService.h"
#include "QueueConsumer.h"
#include "LineParrot.h"

// asl-parrot
#include "service-thread.h"
#include "api-thread.h"

#define MAX_CALLS (64)
#define LINE_ID_IAX (1)
#define LINE_ID_TTS (7)
#define LINE_ID_BRIDGE (10)
#define LINE_ID_STATS (12)
#define LINE_ID_PARROT (34)

using namespace std;
using namespace kc1fsz;

static const char* VERSION = "20260608.1";
static const char* PUBLIC_USER = "radio";

static void sigHandler(int sig);

// Keep these large structures off the stack
static amp::BridgeCall bridgeCallSpace[MAX_CALLS];
static LineIAX2::Call iaxCallSpace[MAX_CALLS];

int main(int argc, const char** argv) {

    amp::setThreadName("Parrot");

#ifndef _WIN32    
    signal(SIGSEGV, sigHandler);
    MTLog log;
#else
    Win32MTLog log;
#endif

    log.info("KC1FSZ ASL Parrot");
    log.info("Powered by the Ampersand ASL Project https://github.com/Ampersand-ASL");
    log.info("Version %s", VERSION);
    if (getenv("AMP_NET_TEST_BIND_ADDR4") != 0)
        log.info("Bind interface for network tests: %s", getenv("AMP_NET_TEST_BIND_ADDR4"));

    StdClock clock;
    NullLog traceLog;

    CURLcode res = curl_global_init(CURL_GLOBAL_ALL);
    if (res) {
        log.error("curl_global_init() failed");
        return -1;
    }

    // Get the service thread running. This handles registration,
    // status, and the monitor.
    std::thread serviceThread(service_thread, &log, VERSION);

    // Setup the message router
    threadsafequeue2<MessageCarrier> respQueue;
    MultiRouter router(respQueue);

    // Setup a background thread to do TTS. 
    // There are queues in/out to handle requests/response.
    // This is hard-coded as line #7.
    threadsafequeue2<MessageCarrier> ttsReqQueue;
    QueueConsumer ttsConsumer7(ttsReqQueue);
    router.addRoute(&ttsConsumer7, LINE_ID_TTS);
    std::atomic<bool> ttsRun(true);
    std::thread ttsThread(amp::ttsLoop, &log, &ttsReqQueue, &respQueue, &ttsRun);

    // Setup a background thread that can perform network testing functions.
    // There are queues in/out to handle requests/response.
    // This is hard-coded as line #8.
    threadsafequeue2<MessageCarrier> networkTestReqQueue;
    QueueConsumer networkTestConsumer8(networkTestReqQueue);
    router.addRoute(&networkTestConsumer8, 8);
    std::atomic<bool> netTestRun(true);
    std::thread netTestThread(Poker::loop, &log, &clock, 
        &networkTestReqQueue, &respQueue, &netTestRun);

    // Setup the conference budget in Parrot mode
    const amp::BridgeCall::Mode mode = (getenv("AMP_PROGRAM_ROOT") == 0) ? 
        amp::BridgeCall::Mode::PARROT : amp::BridgeCall::Mode::PROGRAM;
    const bool parrotConference = getenv("AMP_PARROT_MODE") != 0 &&
        strcmp("CONFERENCE", getenv("AMP_PARROT_MODE")) == 0;

    amp::Bridge bridge10(log, traceLog, clock, router, mode,
        LINE_ID_BRIDGE, LINE_ID_TTS, 
        8, getenv("AMP_NET_TEST_BIND_ADDR4"), LINE_ID_IAX, LINE_ID_STATS, 
        // Not using simple TTS
        0,
        bridgeCallSpace, MAX_CALLS, parrotConference);
    router.addRoute(&bridge10, LINE_ID_BRIDGE);
    amp::BridgeCall::initializeWhiteNoise();
    bridge10.setLocalNodeNumber(getenv("AMP_NODE0_NUMBER"));

    // Setup the IAX line
    // IMPORTANT: The directed POKE feature is turned on here!
    LineIAX2 iax2Channel1(log, traceLog, clock, LINE_ID_IAX, router, 0, 0, 
        // Not using LocalRegistry
        0, 
        // Not using LocalAuthenticator
        0, LINE_ID_BRIDGE, PUBLIC_USER,
        iaxCallSpace, MAX_CALLS);
    router.addRoute(&iax2Channel1, LINE_ID_IAX);
    //iax2Channel0.setTrace(true);
    iax2Channel1.setPrivateKey(getenv("AMP_PRIVATE_KEY"));
    iax2Channel1.setASLDNSRoot(getenv("AMP_ASL_DNS_ROOT"));
    if (getenv("AMP_IAX_AUTHMODE")) {
        if (strcmp(getenv("AMP_IAX_AUTHMODE"), "OPEN") == 0) {
            iax2Channel1.setAuthenticationRequired(false);
            iax2Channel1.setAuthenticationChecked(true);
        }
        else if (strcmp(getenv("AMP_IAX_AUTHMODE"), "REQUIRED") == 0) {
            iax2Channel1.setAuthenticationRequired(true);
            iax2Channel1.setAuthenticationChecked(true);
        }
    }

    if (getenv("AMP_IAX_CAPTURE") != 0)
        iax2Channel1.setCapture(true);

    // Determine the address family, defaulting to IPv4
    short addrFamily = getenv("AMP_IAX_PROTO") != 0 && 
        strcmp(getenv("AMP_IAX_PROTO"), "IPV6") == 0 ? AF_INET6 : AF_INET;
    // Open up the IAX2 network connection
    iax2Channel1.open(addrFamily, atoi(getenv("AMP_IAX_PORT")));

    // Setup the API thread
    int apiPort = 0;
    if (getenv("AMP_HTTP_PORT") != 0) {
        apiPort = atoi(getenv("AMP_HTTP_PORT"));
    }
    std::thread apiThread(amp::apiLoop, &log, &clock, apiPort,
        getenv("AMP_NET_TEST_BIND_ADDR4"), VERSION);

    // This is a special-purpose line that is used if the conference-parrot is 
    // enabled. Note that normally all parrot functionality is handled within 
    // a BridgeCall (i.e. never gets to the Bridge), but in some cases it may
    // be desired to have the parrot be a participant in the Bride conference.
    // #### TODO ENABLE VARIABLE
    amp::LineParrot parrot34(log, clock, LINE_ID_PARROT, router, LINE_ID_BRIDGE, LINE_ID_TTS);
    router.addRoute(&parrot34, LINE_ID_PARROT);
    if (parrotConference)
        parrot34.open(); 

    // Configuration parameter - line thresholds.
    if (getenv("AMP_PARROT_LEVEL_THRESHOLDS")) {
        log.info("Parrot level thresholds: [%s]", getenv("AMP_PARROT_LEVEL_THRESHOLDS"));
        std::vector<int> thresholdList;
        // Parse comma-delimited list
        string s(getenv("AMP_PARROT_LEVEL_THRESHOLDS"));
        stringstream ss(s);
        string token;
        while (std::getline(ss, token, ',')) {
            trim(token);
            thresholdList.push_back(std::stoi(token));
        }
        bridge10.setParrotLevelThresholds(thresholdList);
        parrot34.setParrotLevelThresholds(thresholdList);
    }

    // Main loop        
    Runnable2* tasks2[] = { &iax2Channel1, &bridge10, &router, &parrot34 };
    EventLoop::run(log, clock, 0, 0, tasks2, std::size(tasks2), nullptr, false);

    return 0;
}

#ifndef _WIN32
// A crash signal handler that displays stack information
static void sigHandler(int sig) {
    void *array[32];
    // get void*'s for all entries on the stack
    size_t size = backtrace(array, 32);
    // print out all the frames to stderr
    fprintf(stderr, "Error: signal %d:\n", sig);
    backtrace_symbols_fd(array, size, STDERR_FILENO);
    // Now do the regular thing
    signal(sig, SIG_DFL); 
    raise(sig);
}
#endif