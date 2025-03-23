/* MIT License

Copyright (c) 2024 Adrian Cruceru - https://github.com/AdrianCX/pico_https

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include <string.h>
#include <time.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "request_handler.h"
//#include "pico_logger.h"

#include "fraise.h"

//#define trace(...) fraise_printf(__VA_ARGS__), fraise_printf("\n")
#define trace(...)

std::set<RequestHandler*> RequestHandler::handlers;

void RequestHandler::create(void *arg, bool tls) {
    new RequestHandler(arg, tls);
}

RequestHandler::RequestHandler(void *arg, bool tls)
    : HTTPSession(arg, tls)
{
    trace("RH::RequestHandler: this=%p arg=%p", this, arg);
    handlers.insert(this);
}

RequestHandler::~RequestHandler()
{
    trace("RH::RequestHandler: remove %p", this);
    handlers.erase(this);
}

bool RequestHandler::onRequestReceived(HTTPHeader& header)
{
    if (strcmp(header.getCommand(), "GET") != 0)
    {
        trace("RH::onRequestReceived: this=%p unexpected command[%s]:", this, header.getCommand());
        header.print();
        return false;
    }

    if (strcmp(header.getPath(), "/websocket") == 0)
    {
        trace("RH::onRequestReceived: this=%p acceptWebSocket.", this);
        return acceptWebSocket(header);
    }
    else
    {
        trace("RH::onRequestReceived: this=%p sendReply[index.html]:", this);
        return sendHttpReply("Content-Encoding: gzip\r\n", (const char*)html_gz, html_gz_len);
    }
}

bool RequestHandler::onHttpData(u8_t *data, size_t len)
{
    trace("RH::onHttpData: this=%p data[%s] len[%d]", this, data, len);
    return true;
}

bool RequestHandler::onWebSocketData(u8_t *data, size_t len)
{
    //trace("RH::onWebSocketData: this=%p len=%d data=[%.*s]", this, len, len, data);
    char name[128];
    int val;
    data[len] = 0;
    int ret = sscanf((const char*)data, "%127s %d", name, &val);

    if(ret == 2) fraise_printf("%s %d\n", name, val);

    if(!strcmp(name, "volume")) {
        led_ms = val;
    }

    broadcastWebSocketData(data, len, this);
    return true;
}

void RequestHandler::broadcastWebSocketData(u8_t *data, size_t len, RequestHandler *except)
{
    for(auto handler: handlers) if(handler != except) handler->sendWebSocketData(data, len);
}
