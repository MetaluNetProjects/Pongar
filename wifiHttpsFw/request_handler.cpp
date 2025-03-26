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

#include "fraise_master.h"

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
    char *indata = (char*)data;
    indata[len] = 0; // terminate string
    //trace("RH::onWebSocketData: this=%p len=%d data=[%.*s]", this, len, len, data);
    if(indata[0] == 'f') {
        indata += 2; // remove "f "
        char *token = strsep(&indata, " ");
        if(token) {
            int fruit_id;
            fruit_id = atoi(token);
            if(fruit_id > 0 && fruit_id < 127) {
                char buf[64];
                int buflen = 0;
                while(indata && (token = strsep(&indata, " "))) {
                    buf[buflen++] = atoi(token);
                }
                fraise_master_sendbytes(fruit_id, buf, buflen);
            }
        }
    }
    if(indata[0] == 'F') {
        indata += 2; // remove "F "
        char *token = strsep(&indata, " ");
        if(token) {
            int fruit_id;
            fruit_id = atoi(token);
            if(fruit_id > 0 && fruit_id < 127) {
                fraise_master_sendchars(fruit_id, indata);
            }
        }
    }
    else if(indata[0] == 'v') { // test
        char name[128];
        int val;
        int ret = sscanf((const char*)data, "%127s %d", name, &val);
        if(ret == 2) fraise_printf("%s %d\n", name, val);
        if(!strcmp(name, "volume")) {
            led_ms = val;
        }
    }
    return true;
}

void RequestHandler::broadcastWebSocketData(u8_t *data, size_t len, RequestHandler *except)
{
    for(auto handler: handlers) if(handler != except) handler->sendWebSocketData(data, len);
}

void fraise_master_fruit_detected(uint8_t fruit_id, bool detected) {
    printf("pied detect fruit %d %d\n", fruit_id, detected);
}

void fraise_master_receivebytes(uint8_t fruit_id, const char *data, uint8_t len) {
    char buf[256];
    int buflen = 0;
    buflen = snprintf(buf, sizeof(buf), "f %d", fruit_id);
    for(int i = 0; i < len; i++) buflen += snprintf(buf + buflen, sizeof(buf) - buflen," %d", data[i]);
    RequestHandler::broadcastWebSocketData((u8_t *)buf, buflen);
}

void fraise_master_receivechars(uint8_t fruit_id, const char *data, uint8_t len) {
    //printf("pied fruit %d: %s\n", fruit_id, data);
    char buf[256];
    snprintf(buf, 256, "F %d %*s", fruit_id, len , data);
    RequestHandler::broadcastWebSocketData((u8_t *)buf, strlen(buf));
}


