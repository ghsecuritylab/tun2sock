/**
 * @file BSocksClient.h
 * @author Ambroz Bizjak <ambrop7@gmail.com>
 * 
 * @section LICENSE
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the author nor the
 *    names of its contributors may be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * @section DESCRIPTION
 * 
 * SOCKS5 client. TCP only, no authentication.
 */

#ifndef BADVPN_SOCKS_BSOCKSCLIENT_H
#define BADVPN_SOCKS_BSOCKSCLIENT_H

#include <stdint.h>

#include <misc/debug.h>
#include <misc/debugerror.h>
#include <misc/socks_proto.h>
#include <misc/packed.h>
#include <base/DebugObject.h>
#include <system/BConnection.h>
#include <flow/PacketStreamSender.h>

#define BSOCKSCLIENT_EVENT_ERROR 1
#define BSOCKSCLIENT_EVENT_UP 2
#define BSOCKSCLIENT_EVENT_ERROR_CLOSED 3

/**
 * Handler for events generated by the SOCKS client.
 * 
 * @param user as in {@link BSocksClient_Init}
 * @param event event type. One of BSOCKSCLIENT_EVENT_ERROR, BSOCKSCLIENT_EVENT_UP
 *              and BSOCKSCLIENT_EVENT_ERROR_CLOSED.
 *              If event is BSOCKSCLIENT_EVENT_UP, the object was previously in down
 *              state and has transitioned to up state; I/O can be done from this point on.
 *              If event is BSOCKSCLIENT_EVENT_ERROR or BSOCKSCLIENT_EVENT_ERROR_CLOSED,
 *              the object must be freed from within the job closure of this handler,
 *              and no further I/O must be attempted.
 */
typedef void (*BSocksClient_handler) (void *user, int event);

struct BSocksClient_auth_info {
    int auth_type;
    union {
        struct {
            const char *username;
            size_t username_len;
            const char *password;
            size_t password_len;
        } password;
    };
};

typedef struct {
    const struct BSocksClient_auth_info *auth_info;
    size_t num_auth_info;
    BAddr dest_addr;
    BSocksClient_handler handler;
    void *user;
    BReactor *reactor;
    int state;
    char *buffer;
    BConnector connector;
    BConnection con;
    LinkedList1 filter_in_list;
    LinkedList1 filter_out_list;
    union {
        struct {
            PacketPassInterface *send_if;
            PacketStreamSender send_sender;
            StreamRecvInterface *recv_if;
            uint8_t *recv_dest;
            int recv_len;
            int recv_total;
        } control;
    };
    DebugError d_err;
    DebugObject d_obj;
} BSocksClient;


typedef void (*filter_handler_operation)(char * data,size_t len);

typedef enum {
    FILTER_IN = 0,
    FILTER_OUT,
} filter_type_t;

typedef struct{
    char *filter_handler_name;                      //过滤器名称
    size_t filter_handler_name_lenght;              //过滤器名称长度
    filter_type_t filter_type;                      //过滤器类型
    filter_handler_operation filter_handler;
    LinkedList1Node list_node;
} Filter;


struct BSocksClient_auth_info BSocksClient_auth_none (void);
struct BSocksClient_auth_info BSocksClient_auth_password (const char *username, size_t username_len, const char *password, size_t password_len);



/**
 * Initializes the object.
 * The object is initialized in down state. The object must transition to up
 * state before the user may begin any I/O.
 * 
 * @param o the object
 * @param server_addr SOCKS5 server address
 * @param dest_addr remote address
 * @param handler handler for up and error events
 * @param user value passed to handler
 * @param reactor reactor we live in
 * @return 1 on success, 0 on failure
 */
int BSocksClient_Init (BSocksClient *o,
                       BAddr server_addr, const struct BSocksClient_auth_info *auth_info, size_t num_auth_info,
                       BAddr dest_addr, BSocksClient_handler handler, void *user, BReactor *reactor) WARN_UNUSED;

/**
 * Frees the object.
 *
 * @param o the object
 */
void BSocksClient_Free (BSocksClient *o);

/**
 * Returns the send interface.
 * The object must be in up state.
 * 
 * @param o the object
 * @return send interface
 */
StreamPassInterface * BSocksClient_GetSendInterface (BSocksClient *o);

/**
 * Returns the receive interface.
 * The object must be in up state.
 * 
 * @param o the object
 * @return receive interface
 */
StreamRecvInterface * BSocksClient_GetRecvInterface (BSocksClient *o);

#endif
