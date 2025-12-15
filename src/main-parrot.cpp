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
#include "MessageBus.h"
#include "RegisterTask.h"
#include "ManagerTask.h"
#include "EventLoop.h"
#include "NodeParrot.h"

#include "service-thread.h"

using namespace std;
using namespace kc1fsz;

static const char* VERSION = "20251210.0";

// TODO: NEED MORE RESEARCH ON THIS
static const char* LOCAL_USER = "radio";

/*
Development:
export AMP_NODE0_NUMBER=nnnnn
export AMP_NODE0_PASSWORD=xxxxx
export AMP_NODE0_MGR_PORT=5039
export AMP_IAX_PROTO=IPV4
export AMP_IAX_PORT=4569
export AMP_ASL_REG_URL=https://register.allstarlink.org
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
    //pthread_setname_np(pthread_self(), "Parrot");
    pthread_setname_np("Parrot");
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

    // This goes from IAX2->Parrot
    MessageBus bus0;
    // This goes from Parrot->IAX2
    MessageBus bus1;

    CallDestinationValidatorStd val;
    // IMPORTANT: The directed POKE feature is turned on here!
    LineIAX2 iax2Channel0(log, clock, 1, bus0, &val, 0, false);
    //iax2Channel0.setTrace(true);

    NodeParrot parrot0(log, clock, bus1);

    // Setup the patch panel - establish message flow between components
    bus0.targetChannel = &parrot0;
    bus1.targetChannel = &iax2Channel0;

    // Determine the address family, defaulting to IPv4
    short addrFamily = getenv("AMP_IAX_PROTO") != 0 && 
        strcmp(getenv("AMP_IAX_PROTO"), "IPV6") == 0 ? AF_INET6 : AF_INET;
    // Open up the nIAX2 network connection
    iax2Channel0.open(addrFamily, atoi(getenv("AMP_IAX_PORT")), LOCAL_USER);

    // Main loop        
    const unsigned task2Count = 2;
    Runnable2* tasks2[task2Count] = { &iax2Channel0, &parrot0 };
    EventLoop::run(log, clock, 0, 0, tasks2, task2Count, nullptr, false);

    iax2Channel0.close();
    
    log.info("Done");

    return 0;
}
