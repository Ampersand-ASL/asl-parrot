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
//#include <sched.h>
//#include <linux/sched.h>
//#include <linux/sched/types.h>
#include <pthread.h>
#include <sys/syscall.h> 
#endif 

#include <iostream>
#include <cmath> 
#include <queue>

#include <curl/curl.h>

#include "kc1fsz-tools/Log.h"
//#include "kc1fsz-tools/linux/LinuxPollTimer.h"
#include "kc1fsz-tools/linux/StdClock.h"
#include "kc1fsz-tools/fixedqueue.h"

#ifdef _WIN32
#include "kc1fsz-tools/win32/Win32MTLog.h"
#else
#include "kc1fsz-tools/linux/MTLog.h"
#endif

#include "LineIAX2.h"
#include "ManagerTask.h"
#include "EventLoop.h"
#include "Bridge.h"

#include "service-thread.h"

using namespace std;
using namespace kc1fsz;

static const char* VERSION = "20251220.0";

// TODO: NEED MORE RESEARCH ON THIS
static const char* LOCAL_USER = "radio";

/*
Development:
export AMP_NODE0_NUMBER=nnnnn
export AMP_NODE0_PASSWORD=xxxxx
export AMP_NODE0_MGR_PORT=5039
export AMP_IAX_PROTO=IPV4
export AMP_IAX_PORT=4569
export AMP_IAX_AUTHMODE=OPEN
export AMP_ASL_REG_URL=https://register.allstarlink.org
export AMP_ASL_STAT_URL=http://stats.allstarlink.org/uhandler
export AMP_ASL_DNS_ROOT=allstarlink.org
*/

// TEMPORARY: Accept all calls
class CallDestinationValidatorStd : public CallValidator {
public:
    virtual bool isNumberAllowed(const char* targetNumber) const {
        return true;
    }
};

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

int main(int argc, const char** argv) {

#ifndef _WIN32    
    pthread_setname_np(pthread_self(), "Parrot");
    signal(SIGSEGV, sigHandler);
    MTLog log;
#else
    Win32MTLog log;
#endif

    log.info("KC1FSZ ASL Parrot");
    log.info("Powered by the Ampersand ASL Project https://github.com/Ampersand-ASL");
    log.info("Version %s", VERSION);

    StdClock clock;
  
    CURLcode res = curl_global_init(CURL_GLOBAL_ALL);
    if (res) {
        log.error("curl_global_init() failed");
        return -1;
    }

    // Get the service thread running. This handles registration,
    // status, and the monitor.
    pthread_t new_thread_id;
    int trc = pthread_create(&new_thread_id,0, service_thread, (Log*)(&log));
    if (trc != 0) {
        log.error("Unable to create thread %d/%d", trc, errno);
        return -1;
    }

    amp::Bridge bridge10(log, clock, amp::BridgeCall::Mode::PARROT);
    
    CallDestinationValidatorStd val;
    // IMPORTANT: The directed POKE feature is turned on here!
    LineIAX2 iax2Channel1(log, clock, 1, bridge10, &val, 0);
    //iax2Channel0.setTrace(true);
    iax2Channel1.setPrivateKey(getenv("AMP_PRIVATE_KEY"));
    iax2Channel1.setDNSRoot(getenv("AMP_ASL_DNS_ROOT"));
    if (getenv("AMP_IAX_AUTHMODE")) {
        if (strcmp(getenv("AMP_IAX_AUTHMODE"), "OPEN") == 0) 
            iax2Channel1.setAuthMode(LineIAX2::AuthMode::OPEN);
        else if (strcmp(getenv("AMP_IAX_AUTHMODE"), "SOURCE_IP") == 0) 
            iax2Channel1.setAuthMode(LineIAX2::AuthMode::SOURCE_IP);
        else if (strcmp(getenv("AMP_IAX_AUTHMODE"), "CHALLENGE_ED25519") == 0) 
            iax2Channel1.setAuthMode(LineIAX2::AuthMode::CHALLENGE_ED25519);
    }
    bridge10.setSink(&iax2Channel1);

    // Determine the address family, defaulting to IPv4
    short addrFamily = getenv("AMP_IAX_PROTO") != 0 && 
        strcmp(getenv("AMP_IAX_PROTO"), "IPV6") == 0 ? AF_INET6 : AF_INET;
    // Open up the IAX2 network connection
    iax2Channel1.open(addrFamily, atoi(getenv("AMP_IAX_PORT")), LOCAL_USER);

    // Main loop        
    Runnable2* tasks2[] = { &iax2Channel1, &bridge10 };
    EventLoop::run(log, clock, 0, 0, tasks2, std::size(tasks2), nullptr, false);

    return 0;
}
