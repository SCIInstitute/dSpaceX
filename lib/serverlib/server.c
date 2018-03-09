/*
 *	The Web Socket Transport
 *
 *		simple server code
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifndef WIN32
#include <pthread.h>
#endif

#include "libwebsockets.h"

#include "wstypes.h"


  /* prototypes used & not in the above */
  extern /*@null@*/ /*@out@*/ /*@only@*/ void *wst_alloc(int nbytes);
  extern /*@null@*/ /*@only@*/ 
         void *wst_realloc(/*@null@*/ /*@only@*/ /*@returned@*/ void *ptr, 
                          int nbytes);
  extern void  wst_free(/*@null@*/ /*@only@*/ void *ptr);
  extern void  wst_destroyContext(wstContext **context);

#ifdef STANDALONE
  extern /*@null@*/ wstContext *wst_createContext( );
  extern void wst_freeData( wstData *data );
  extern /*@null@*/ wstData *wst_createData( const char *name,
                                             enum TypedArray type, int hlen,
                                             const int *header, int len,
                                             void *array );
#endif

  /* message call-backs */
  extern void  browserText( struct libwebsocket *wsi, char *buf, int len );
  extern void  browserData( struct libwebsocket *wsi, wstData *data );


enum wst_protocols {
	/* always first */
	PROTOCOL_HTTP = 0,

	PROTOCOL_DATA_BINARY,
	PROTOCOL_UI_TEXT,

	/* always last */
	WV_PROTOCOL_COUNT
};



#ifdef WIN32

static int ThreadCreate(void (*entry)(), void *arg)
{
  return _beginthread(entry, 0, arg);
}

#else

static int ThreadCreate(void (*entry)(), void *arg)
{
  pthread_t thread;
  pthread_attr_t attr;
  int stat;

  pthread_attr_init(&attr);
  stat = pthread_create(&thread, &attr,
                        (void*(*) (void *)) entry, arg);
  if (stat != 0) return -1;
  return 1;
}

#endif


typedef struct {
        int                         nClient;       /* # active clients */
        struct libwebsocket         **wsi;         /* the text wsi per client */
        int                         loop;          /* 1 for continue;
                                                      0 for done */
        int                         index;         /* server index */
        struct libwebsocket_context *WScontext;    /* WebSocket context */
        wstContext                  *WSTcontext;   /* WebSocket Transport context */
} wstServer;


static int       nServers = 0;
static wstServer *servers = NULL;


/* this protocol server (always the first one) just knows how to do HTTP */

static int callback_http(/*@unused@*/ struct libwebsocket_context *context,
		         struct libwebsocket *wsi,
		         enum libwebsocket_callback_reasons reason, 
                         void *user, void *in, /*@unused@*/ size_t len)
{
	char client_name[128];
	char client_ip[128];

	switch (reason) {

	case LWS_CALLBACK_HTTP:
		fprintf(stderr, "serving HTTP URI %s\n", (char *)in);

		if (in && strcmp(in, "/favicon.ico") == 0) {
			if (libwebsockets_serve_http_file(wsi,
			     "favicon.ico", "image/x-icon"))
				fprintf(stderr, "Failed to send favicon\n");
			break;
		}

		/* send the script... when it runs it'll start websockets */

		if (libwebsockets_serve_http_file(wsi,
				  "wv.html", "text/html"))
			fprintf(stderr, "Failed to send HTTP file\n");
		break;

	/*
	 * callback for confirming to continue with client IP appear in
	 * protocol 0 callback since no websocket protocol has been agreed
	 * yet.  You can just ignore this if you won't filter on client IP
	 * since the default uhandled callback return is 0 meaning let the
	 * connection continue.
	 */

	case LWS_CALLBACK_FILTER_NETWORK_CONNECTION:

		libwebsockets_get_peer_addresses(user, client_name,
			     sizeof(client_name), client_ip, sizeof(client_ip));

		fprintf(stderr, "Received network connect from %s (%s)\n",
							client_name, client_ip);

		/* if we returned non-zero from here, we kill the connection */
		break;

	default:
		break;
	}

	return 0;
}


/*
 * this is just an example of parsing handshake headers, this is not needed
 * unless filtering is allowing connections by the header content
 *
 */

static void
dump_handshake_info(struct lws_tokens *lwst)
{
	int n;
	static const char *token_names[WSI_TOKEN_COUNT] = {
		/*[WSI_TOKEN_GET_URI]		=*/ "GET URI",
		/*[WSI_TOKEN_HOST]		=*/ "Host",
		/*[WSI_TOKEN_CONNECTION]	=*/ "Connection",
		/*[WSI_TOKEN_KEY1]		=*/ "key 1",
		/*[WSI_TOKEN_KEY2]		=*/ "key 2",
		/*[WSI_TOKEN_PROTOCOL]		=*/ "Protocol",
		/*[WSI_TOKEN_UPGRADE]		=*/ "Upgrade",
		/*[WSI_TOKEN_ORIGIN]		=*/ "Origin",
		/*[WSI_TOKEN_DRAFT]		=*/ "Draft",
		/*[WSI_TOKEN_CHALLENGE]		=*/ "Challenge",

		/* new for 04 */
		/*[WSI_TOKEN_KEY]		=*/ "Key",
		/*[WSI_TOKEN_VERSION]		=*/ "Version",
		/*[WSI_TOKEN_SWORIGIN]		=*/ "Sworigin",

		/* new for 05 */
		/*[WSI_TOKEN_EXTENSIONS]	=*/ "Extensions",

		/* client receives these */
		/*[WSI_TOKEN_ACCEPT]		=*/ "Accept",
		/*[WSI_TOKEN_NONCE]		=*/ "Nonce",
		/*[WSI_TOKEN_HTTP]		=*/ "Http",
		/*[WSI_TOKEN_MUXURL]            =*/ "MuxURL",
	};
	
	for (n = 0; n < WSI_TOKEN_COUNT; n++) {
		if (lwst[n].token == NULL) continue;
		fprintf(stderr, "    %s = %s\n", token_names[n], lwst[n].token);
	}
}


/* binary protocol */

/*
 * one of these is auto-created for each connection and a pointer to the
 * appropriate instance is passed to the callback in the user parameter
 *
 */

struct per_session_data__data_binary {
        struct libwebsocket *wsi;
};

static int
callback_data_binary(struct libwebsocket_context *context,
                     struct libwebsocket *wsi,
                     enum libwebsocket_callback_reasons reason,
                     void *user, void *in, size_t len)
{
	int     i, slot, ioff, *ibuf;
        char    *cbuf;
        size_t  dlen;
        wstData *wstdata;
	struct per_session_data__data_binary *pss = user;

        for (slot = 0; slot < nServers; slot++)
                if (servers[slot].WScontext == context) break;
        if (slot == nServers) {
                fprintf(stderr, "ERROR no Slot!");
                exit(1);          
        }

	switch (reason) {

        /*
         * invoked when initial connection is made
         */
         
	case LWS_CALLBACK_ESTABLISHED:
		fprintf(stderr, "callback_data_binary: LWS_CALLBACK_ESTABLISHED\n");
                pss->wsi = wsi;
		break;

	case LWS_CALLBACK_BROADCAST:
                i = libwebsocket_write(wsi, in, len, LWS_WRITE_BINARY);
                if (i < 0) fprintf(stderr, "binary data write failed\n");
		break;

	case LWS_CALLBACK_RECEIVE:
                /* convert the incoming stream to wstData */
                wstdata = (wstData *) wst_alloc(sizeof(wstData));
                if (wstdata == NULL) {
                    browserData(wsi, NULL);
                } else {
                    ibuf = (int *)  in;
                    cbuf = (char *) in;
                    wstdata->name = (char *) wst_alloc((ibuf[0]+1)*sizeof(char));
                    if (wstdata->name == NULL) {
                      wst_free(wstdata);
                      browserData(wsi, NULL);
                    } else {
                      for (i = 0; i < ibuf[0]; i++)
                        wstdata->name[i] = cbuf[i+4];
                      wstdata->name[ibuf[0]] = 0;
                      i = ioff = ibuf[0]/4;
                      if (i*4 != ibuf[0]) ioff++;
                      ioff++;
                      wstdata->type = ibuf[ioff++];
                      wstdata->hlen = ibuf[ioff++];
                      wstdata->header = (int *) wst_alloc(wstdata->hlen*sizeof(int));
                      if (wstdata->header == NULL) {
                        wst_free(wstdata->name);
                        wst_free(wstdata);
                        browserData(wsi, NULL);
                      } else {
                        for (i = 0; i < wstdata->hlen; i++)
                          wstdata->header[i] = ibuf[ioff++];
                        wstdata->len = ibuf[ioff++];
                        dlen = wstdata->len*TypedArrayBytes[wstdata->type];
                        wstdata->data = wst_alloc(dlen*sizeof(char));
                        if (wstdata->data == NULL) {
                          wst_free(wstdata->header);
                          wst_free(wstdata->name);
                          wst_free(wstdata);
                          browserData(wsi, NULL);
                        } else {
                          memcpy(wstdata->data, &ibuf[ioff], dlen);
                          browserData(wsi, wstdata);
                        }
                      }
                    }
                }
		break;
                
        case LWS_CALLBACK_CLOSED:
                fprintf(stderr, "callback_data_binary: LWS_CALLBACK_CLOSED\n");
                break;
        
	/*
	 * this just demonstrates how to use the protocol filter. If you won't
	 * study and reject connections based on header content, you don't need
	 * to handle this callback
	 */

	case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION:
		dump_handshake_info((struct lws_tokens *) user);
		/* you could return non-zero here and kill the connection */
		break;

	default:
		break;
	}

	return 0;
}


/* ui_text protocol */

struct per_session_data__ui_text {
	struct libwebsocket *wsi;
};

static int
callback_ui_text(struct libwebsocket_context *context,
                 struct libwebsocket *wsi,
                 enum libwebsocket_callback_reasons reason,
                 void *user, void *in, size_t len)
{
	int    i, slot, n;
	struct per_session_data__ui_text *pss = user;
        struct libwebsocket **tmp;

        for (slot = 0; slot < nServers; slot++)
                if (servers[slot].WScontext == context) break;
        if (slot == nServers) {
                fprintf(stderr, "ERROR no Slot!");
                exit(1);          
        }

	switch (reason) {

	case LWS_CALLBACK_ESTABLISHED:
		fprintf(stderr, "callback_ui_text: LWS_CALLBACK_ESTABLISHED\n");
		pss->wsi = wsi;
                n        = servers[slot].nClient + 1;
                if (servers[slot].wsi == NULL) {
                  tmp = (struct libwebsocket **)
                        wst_alloc(n*sizeof(struct libwebsocket *));
                } else {
                  tmp = (struct libwebsocket **)
                        wst_realloc(servers[slot].wsi,
                                    n*sizeof(struct libwebsocket *));
                }
                if (tmp == NULL) {
                    fprintf(stderr, "callback_ui_text: Malloc Problem!\n");
                } else {
                    servers[slot].wsi = tmp;
                    servers[slot].wsi[servers[slot].nClient] = wsi;
                    servers[slot].nClient++;
                }
		break;

	case LWS_CALLBACK_BROADCAST:
		n = libwebsocket_write(wsi, in, len, LWS_WRITE_TEXT);
		if (n < 0) fprintf(stderr, "ui-text write failed\n");
		break;

	case LWS_CALLBACK_RECEIVE:
                browserText(wsi, in, len);
		break;
            
        case LWS_CALLBACK_CLOSED:
              fprintf(stderr, "callback_ui_text: LWS_CALLBACK_CLOSED\n");
              for (n = i = 0; i < servers[slot].nClient; i++) {
                  if (servers[slot].wsi[i] == wsi) continue;
                  servers[slot].wsi[n] = servers[slot].wsi[i];
                  n++;
              }
              servers[slot].nClient--;
              if (n != servers[slot].nClient)
                  fprintf(stderr, "callback_ui_text: Client Problem!\n");
              if (servers[slot].nClient <= 0) {
                  wst_free(servers[slot].wsi);
                  servers[slot].wsi  = NULL;
                  servers[slot].loop = 0;
              }
              break;

	/*
	 * this just demonstrates how to use the protocol filter. If you won't
	 * study and reject connections based on header content, you don't need
	 * to handle this callback
	 */

	case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION:
		dump_handshake_info((struct lws_tokens *) user);
		/* you could return non-zero here and kill the connection */
		break;

	default:
		break;
	}

	return 0;
}


/* list of supported protocols and callbacks */

  static struct libwebsocket_protocols wst_protocols[] = {
	/* first protocol must always be HTTP handler */

	{
		"http-only",		/* name */
		callback_http,		/* callback */
		0			/* per_session_data_size */
	},
	{
		"data-binary-protocol",
		callback_data_binary,
		sizeof(struct per_session_data__data_binary),
	},
	{
		"ui-text-protocol",
		callback_ui_text,
		sizeof(struct per_session_data__ui_text)
	},
	{
		NULL, NULL, 0		/* End of list */
	}
  };


static void serverThread(wstServer *server)
{
  unsigned char buf[LWS_SEND_BUFFER_PRE_PADDING + 128 +
                    LWS_SEND_BUFFER_POST_PADDING];
  
  buf[LWS_SEND_BUFFER_PRE_PADDING] = 'x';
  
  while (server->loop) {
    
    usleep(50000);
    libwebsocket_service(server->WScontext, 0);
    
  }
  
  /* mark the thread as down */
  server->loop = -1;
  wst_destroyContext(&server->WSTcontext);
  libwebsocket_context_destroy(server->WScontext);
}


int wst_startServer(int port, /*@null@*/ char *interface,
                   /*@null@*/ char *cert_path, /*@null@*/ char *key_path,
                   int opts, /*@only@*/ wstContext *WSTcontext)
{
  int       slot;
  wstServer *tserver;
  struct libwebsocket_context *context;
  
  fprintf(stderr, "\nwst libwebsockets server thread start\n\n");
  
  context = libwebsocket_create_context(port, interface, wst_protocols,
                                        libwebsocket_internal_extensions,
                                        cert_path, key_path, -1, -1, opts);
  if (context == NULL) {
    fprintf(stderr, "libwebsocket init failed!\n");
    return -1;
  }
  
  for (slot = 0; slot < nServers; slot++)
    if (servers[slot].loop == -1) break;
  
  if (slot == nServers) {
    if (nServers == 0) {
      tserver = (wstServer *) wst_alloc(sizeof(wstServer));
    } else {
      tserver = (wstServer *) wst_realloc(servers,
                                          (nServers+1)*sizeof(wstServer));
    }
    if (tserver == NULL) {
      libwebsocket_context_destroy(context);
      fprintf(stderr, "Server malloc failed!\n");
      return -1;
    }
    servers = tserver;
    nServers++;
  }
  servers[slot].loop       = 1;
  servers[slot].nClient    = 0;
  servers[slot].index      = slot;
  servers[slot].wsi        = NULL;
  servers[slot].WScontext  =   context;
  servers[slot].WSTcontext = WSTcontext;
  
  /* spawn off the server thread */
  if (ThreadCreate(serverThread, &servers[slot]) == -1) {
    libwebsocket_context_destroy(context);
    wst_destroyContext(&WSTcontext);
    servers[slot].loop = -1;
    fprintf(stderr, "thread init failed!\n");
    return -1;
  }
  
  return slot;
}


void wst_cleanupServers()
{
  int i;
  
  for (i = 0; i < nServers; i++) {
    if (servers[i].loop == -1) continue;
    wst_destroyContext(&servers[i].WSTcontext);
    libwebsocket_context_destroy(servers[i].WScontext);
  }
  wst_free(servers);
   servers = NULL;
  nServers = 0;
}


int wst_statusServer(int index)
{
  int loop;

  if ((index < 0) || (index >= nServers)) return -2;
  loop = servers[index].loop;
  if (loop == -1) loop = 0;
  return loop;
}


int wst_nClientServer(int index)
{
  if ((index < 0) || (index >= nServers)) return -2;
  return servers[index].nClient;
}


int wst_activeTextInterfaces(int index, int *Nwsi, void ***wsi)
{
  *Nwsi = 0;
  *wsi  = NULL;
  if ((index < 0) || (index >= nServers)) return -2;
  
  *Nwsi = servers[index].nClient;
  *wsi  = (void **) servers[index].wsi;
  return 0;
}


void wst_killInterface(int index, void *wsix)
{
  struct libwebsocket *wsi;
  
  if ((index < 0) || (index >= nServers)) return;
  
  wsi = (struct libwebsocket *) wsix;
  libwebsocket_close_and_free_session(servers[index].WScontext, wsi,
                                      LWS_CLOSE_STATUS_GOINGAWAY);
}


void wst_sendText(struct libwebsocket *wsi, char *text)
{
  int  n;
  unsigned char *message;
  
  if (text == NULL) {
    fprintf(stderr, "ERROR: sendText called with NULL text!");
    return;
  }
  n = strlen(text);
  message = (unsigned char *) wst_alloc((n+LWS_SEND_BUFFER_PRE_PADDING +
                                           LWS_SEND_BUFFER_POST_PADDING)*
                                        sizeof(unsigned char));
  if (message == NULL) {
    fprintf(stderr, "ERROR: sendText Malloc with length %d!", n);
    return;    
  }
  memcpy(&message[LWS_SEND_BUFFER_PRE_PADDING], text, n);
  n = libwebsocket_write(wsi, &message[LWS_SEND_BUFFER_PRE_PADDING], 
                         n, LWS_WRITE_TEXT);
  if (n < 0) fprintf(stderr, "sendText write failed\n");
  wst_free(message);
}


void wst_broadcastText(char *text)
{
  int  n;
  unsigned char *message;
  
  if (text == NULL) {
    fprintf(stderr, "ERROR: broadcastText called with NULL text!");
    return;
  }
  n = strlen(text);
  message = (unsigned char *) wst_alloc((n+LWS_SEND_BUFFER_PRE_PADDING +
                                           LWS_SEND_BUFFER_POST_PADDING)*
                                        sizeof(unsigned char));
  if (message == NULL) {
    fprintf(stderr, "ERROR: broadcastText Malloc with length %d!", n);
    return;    
  }
  memcpy(&message[LWS_SEND_BUFFER_PRE_PADDING], text, n);
  libwebsockets_broadcast(&wst_protocols[PROTOCOL_UI_TEXT],
                          &message[LWS_SEND_BUFFER_PRE_PADDING], n);
  wst_free(message);
}


void wst_sendData(struct libwebsocket *wsi, wstData *wstdata)
{
  char          zero = 0;
  int           i, j, k, odd = 0;
  size_t        n, ioff;
  unsigned char *message;
  
  if (wstdata == NULL) {
    fprintf(stderr, "ERROR: sendData called with NULL!");
    return;
  }
  n = strlen(wstdata->name);
  i = n/4;
  if (i*4 != n) i++;
  k = i + 4 + wstdata->hlen;
  n = k*sizeof(int);
  if (wstdata->type == WST_Float64) {
    odd = k % 2;
    n += odd*sizeof(int);
  }
  n += wstdata->len*TypedArrayBytes[wstdata->type];
  message = (unsigned char *) wst_alloc((n+LWS_SEND_BUFFER_PRE_PADDING +
                                           LWS_SEND_BUFFER_POST_PADDING)*
                                        sizeof(unsigned char));
  if (message == NULL) {
    i = n;
    fprintf(stderr, "ERROR: sendData Malloc with length %d!", i);
    return;
  }
  ioff  = LWS_SEND_BUFFER_PRE_PADDING;
  i = j = strlen(wstdata->name);
  memcpy(&message[ioff], &i, sizeof(int));
  ioff += sizeof(int);
  j = i/4;
  if (j*4 != i) j++;
  memcpy(&message[ioff], wstdata->name, i);
  ioff += i;
  for (k = i; k < 4*j; k++) memcpy(&message[ioff++], &zero, 1);
  i = wstdata->type;
  memcpy(&message[ioff], &i, sizeof(int));
  ioff += sizeof(int);
  memcpy(&message[ioff], &wstdata->hlen, sizeof(int));
  ioff += sizeof(int);
  memcpy(&message[ioff], wstdata->header, wstdata->hlen*sizeof(int));
  ioff += sizeof(int)*wstdata->hlen;
  memcpy(&message[ioff], &wstdata->len, sizeof(int));
  ioff += sizeof(int);
  if (odd == 1) {
    i = 0;
    memcpy(&message[ioff], &i, sizeof(int));
    ioff += sizeof(int);
  }
  n = wstdata->len*TypedArrayBytes[wstdata->type];
  memcpy(&message[ioff], wstdata->data, n);
  n += ioff - LWS_SEND_BUFFER_PRE_PADDING;
  
  i  = libwebsocket_write(wsi, &message[LWS_SEND_BUFFER_PRE_PADDING],
                          n, LWS_WRITE_BINARY);
  if (i < 0) fprintf(stderr, "sendData write failed\n");
  wst_free(message);
}


void wst_broadcastData(wstData *wstdata)
{
  char          zero = 0;
  int           i, j, k, odd = 0;
  size_t        n, ioff;
  unsigned char *message;
  
  if (wstdata == NULL) {
    fprintf(stderr, "ERROR: broadcastData called with NULL!");
    return;
  }
  n = strlen(wstdata->name);
  i = n/4;
  if (i*4 != n) i++;
  k = i + 4 + wstdata->hlen;
  n = k*sizeof(int);
  if (wstdata->type == WST_Float64) {
    odd = k % 2;
    n += odd*sizeof(int);
  }
  n += wstdata->len*TypedArrayBytes[wstdata->type];
  message = (unsigned char *) wst_alloc((n+LWS_SEND_BUFFER_PRE_PADDING +
                                         LWS_SEND_BUFFER_POST_PADDING)*
                                        sizeof(unsigned char));
  if (message == NULL) {
    i = n;
    fprintf(stderr, "ERROR: broadcastData Malloc with length %d!", i);
    return;
  }
  ioff  = LWS_SEND_BUFFER_PRE_PADDING;
  i = j = strlen(wstdata->name);
  memcpy(&message[ioff], &i, sizeof(int));
  ioff += sizeof(int);
  j = i/4;
  if (j*4 != i) j++;
  memcpy(&message[ioff], wstdata->name, i);
  ioff += i;
  for (k = i; k < 4*j; k++) memcpy(&message[ioff++], &zero, 1);
  i = wstdata->type;
  memcpy(&message[ioff], &i, sizeof(int));
  ioff += sizeof(int);
  memcpy(&message[ioff], &wstdata->hlen, sizeof(int));
  ioff += sizeof(int);
  memcpy(&message[ioff], wstdata->header, wstdata->hlen*sizeof(int));
  ioff += sizeof(int)*wstdata->hlen;
  memcpy(&message[ioff], &wstdata->len, sizeof(int));
  ioff += sizeof(int);
  if (odd == 1) {
    i = 0;
    memcpy(&message[ioff], &i, sizeof(int));
    ioff += sizeof(int);
  }
  n = wstdata->len*TypedArrayBytes[wstdata->type];
  memcpy(&message[ioff], wstdata->data, n);
  n += ioff - LWS_SEND_BUFFER_PRE_PADDING;
  
  libwebsockets_broadcast(&wst_protocols[PROTOCOL_DATA_BINARY],
                          &message[LWS_SEND_BUFFER_PRE_PADDING], n);
  wst_free(message);
}


#ifdef STANDALONE

void browserText(struct libwebsocket *wsi, char *text, /*@unused@*/ int len)
{
  printf(" RXt %lx: %s\n", (long) wsi, text);
  /* bounce it back */
  wst_sendText(wsi, text);
}


void browserData(struct libwebsocket *wsi, wstData *data)
{

  int     header[4] = {4, 4, 4, 4};
  double  vec[10]   = {0.5, 1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5, 8.5, 9.5};
  wstData *newData;

  if (data == NULL) {
    printf(" RXd: Malloc Error!\n");
  } else {
    printf(" RXd %lx: %s\n", (long) wsi, data->name);
    /* bounce it back */
    wst_sendData(wsi, data);
    wst_freeData(data);
    /* add our own */
    newData = wst_createData("doubleName", WST_Float64, 4, header, 10, vec);
/*  wst_sendData(wsi, newData);  */
    wst_broadcastData(newData);
    wst_freeData(newData);
  }
}


int main(/*@unused@*/ int argc, /*@unused@*/ char **argv)
{
  int        stat;
  char       *startapp;
  wstContext *cntxt;
  
  /* get our starting application line
   *
   * for example on a Mac:
   * setenv DSX_START "open -a /Applications/Firefox.app ../client/wv.html"
   */
  startapp = getenv("DSX_START");
  
  // create the Web Socket Transport context  
  cntxt = wst_createContext();
  if (cntxt == NULL) {
    printf(" failed to create wstContext!\n");
    return -1;
  }
  
  // start the server code  
  stat = 0;
  if (wst_startServer(7681, NULL, NULL, NULL, 0, cntxt) == 0) {
    
    /* we have a single valid server */
    while (wst_statusServer(0)) {
      usleep(500000);
      if (stat == 0) {
        if (startapp != NULL) system(startapp);
        stat++;
      }
      if (wst_nClientServer(0) == 0) continue;
      // wst_broadcastText("I'm here!");
    }
  }

  wst_cleanupServers();
  return 0;
}

#endif
