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
#include "httplib.h"

#include <iostream>
#include <thread>

#include <nlohmann/json.hpp>

#include "kc1fsz-tools/Log.h"
#include "kc1fsz-tools/Clock.h"
#include "kc1fsz-tools/threadsafequeue2.h"

#include "Message.h"
#include "MessageConsumer.h"
#include "ThreadUtil.h"
#include "TraceLog.h"
#include "Poker.h"

using namespace std;
using namespace kc1fsz;

using json = nlohmann::json;

namespace kc1fsz {

    namespace amp {

void apiLoop(Log* l, Clock* clock, int listenPort, const char* netTestBindAddr,
    const char* version) {

    Log& log = *l;
    amp::setThreadName("API");

    // If there is no listen port then exist the thread immediately
    if (listenPort == 0) {
        log.info("Not using API thread");
        return;
    }
    
    log.info("Start API thread (HTTP port is %d)", listenPort);

    // HTTP
    httplib::Server svr;

    svr.Get("/ping", 
        []
        (const httplib::Request& req, httplib::Response &res) {
            const char* msg = "Running\r\n";
            res.set_content((const char*)msg, strlen(msg), "text/plain");
        }
    );

    svr.Get("/network-test", 
        [&log, clock, netTestBindAddr, version]
        (const httplib::Request& req, httplib::Response &res) {
            // Check for the Authorization header
            if (req.has_header("Authorization")) {
                std::string authHeader = req.get_header_value("Authorization");
            }

            // Make a network test request 
            if (req.has_param("node")) {
                auto val = req.get_param_value("node");
                Poker::Result r = Poker::poke(log, *clock, netTestBindAddr,
                    val.c_str());
                string status;
                if (r.code == 0)
                    status = "ok";
                else if (r.code == -1 || r.code == -2 || r.code == -3)
                    status = "unregistered";
                else if (r.code == -9 || r.code == -10 || r.code == -11 ||
                    r.code == -12) 
                    status = "unreachable";
                else 
                    status = "unknown";
                json o1;
                o1["status"] = status;
                o1["rc"] = r.code;
                if (r.code == 0) {
                    o1["addr"] = r.addr4;
                    o1["port"] = r.port;
                    o1["pingms"] = r.pokeTimeMs;
                }
                json o0;
                o0["ipv4"] = o1;
                if (req.has_param("cookie")) {
                    auto c = req.get_param_value("cookie");
                    o0["cookie"] = c;
                }
                o0["calleraddr"] = req.remote_addr;
                o0["version"] = version;
                res.set_content(o0.dump(), "application/json");
            } 
            else {
                res.status = httplib::StatusCode::BadRequest_400;
            }
        }
    );

    if (!svr.listen("0.0.0.0", listenPort))
        log.error("Failed to listen on API HTTP port");

    log.info("End API thread");
}

    }
}
