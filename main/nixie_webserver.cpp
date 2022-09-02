/*
	...


*/

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "lwip/ip4_addr.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "tcpip_adapter.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/api.h"

#include "nixie_webserver.h"

// static TAG
static const char *TAG = "nixie_webserver";

// =============================================================================================================
// CLASS NixieWebserver
// =============================================================================================================

NixieWebserver::NixieWebserver(/* args */)
{
	ESP_LOGW(TAG, "NixieWebserver instance created, setting specific http_serve function...");
	// init data
	m_data.ui16HttpRequestCounter = 0;
	// init all user data here
	// ...
	
	// point function in the right direction
	setHttpServeFunc(nixie_http_serve);
}

// nixie-specific implementation of http_server
void NixieWebserver::nixie_http_serve(struct netconn *conn, void *pArgs)
{
	const static char *TAG2 = "nixie_http_serve";

	// get our arguments and cast them to what we know they are:
	NixieWebserver *pNixieWebserver = (NixieWebserver *)pArgs;

	// communication count
	pNixieWebserver->m_data.ui16HttpRequestCounter++;
	//ESP_LOGW(TAG2, "Current Communication Count with default webserver: %i", pNixieWebserver->m_data.ui16HttpRequestCounter);

	const static char HTML_HEADER[] = "HTTP/1.1 200 OK\nAccess-Control-Allow-Origin: *\nContent-type: text/html\n\n";
	const static char JS_HEADER[] = "HTTP/1.1 200 OK\nContent-type: text/javascript\n\n";
	const static char CSS_HEADER[] = "HTTP/1.1 200 OK\nContent-type: text/css\n\n";

	struct netbuf *inbuf;
	static char *buf;
	static uint16_t buflen;
	static err_t err;

	char outbuf[1000];

	// default page
	extern const uint8_t root_html_start[] asm("_binary_index_html_start");
	extern const uint8_t root_html_end[] asm("_binary_index_html_end");
	const uint32_t root_html_len = root_html_end - root_html_start;

	// index.js
	extern const uint8_t test_js_start[] asm("_binary_index_js_start");
	extern const uint8_t test_js_end[] asm("_binary_index_js_end");
	const uint32_t test_js_len = test_js_end - test_js_start;

	// Chart.js
	// extern const uint8_t Chart_js_start[] asm("_binary_Chart_js_start");
	// extern const uint8_t Chart_js_end[] asm("_binary_Chart_js_end");
	// const uint32_t Chart_js_len = Chart_js_end - Chart_js_start;

	// index.css
	extern const uint8_t index_css_start[] asm("_binary_index_css_start");
	extern const uint8_t index_css_end[] asm("_binary_index_css_end");
	const uint32_t index_css_len = index_css_end - index_css_start;

	netconn_set_recvtimeout(conn, 1000); // allow a connection timeout of 1 second
	err = netconn_recv(conn, &inbuf);

	if (err == ERR_OK)
	{
		netbuf_data(inbuf, (void **)&buf, &buflen);
		if (buf)
		{
			// print the whole request for debugging purposes

			// printf("\n\n%s\n",buf);

			// default page
			if (strstr(buf, "GET / ") && !strstr(buf, "Upgrade: websocket"))
			{
				ESP_LOGI(TAG2, "Sending /index.html");
				netconn_write(conn, HTML_HEADER, sizeof(HTML_HEADER) - 1, NETCONN_NOCOPY);
				netconn_write(conn, root_html_start, root_html_len, NETCONN_NOCOPY);
				netconn_close(conn);
				netconn_delete(conn);
				netbuf_delete(inbuf);
			}
			else if (strstr(buf, "GET /index.js "))
			{
				ESP_LOGI(TAG2, "Sending /index.js");
				netconn_write(conn, JS_HEADER, sizeof(JS_HEADER) - 1, NETCONN_NOCOPY);
				netconn_write(conn, test_js_start, test_js_len, NETCONN_NOCOPY);
				netconn_close(conn);
				netconn_delete(conn);
				netbuf_delete(inbuf);
			}
			// else if (strstr(buf, "GET /Chart.js "))
			// {
			// 	ESP_LOGI(TAG2, "Sending /Chart.js");
			// 	netconn_write(conn, JS_HEADER, sizeof(JS_HEADER) - 1, NETCONN_NOCOPY);
			// 	netconn_write(conn, Chart_js_start, Chart_js_len, NETCONN_NOCOPY);
			// 	netconn_close(conn);
			// 	netconn_delete(conn);
			// 	netbuf_delete(inbuf);
			// 	ESP_LOGI(TAG2, "Sent Chart.js and closed connection");
			// }
			else if (strstr(buf, "GET /index.css "))
			{
				ESP_LOGI(TAG2, "Sending /index.css");
				netconn_write(conn, CSS_HEADER, sizeof(CSS_HEADER) - 1, NETCONN_NOCOPY);
				netconn_write(conn, index_css_start, index_css_len, NETCONN_NOCOPY);
				netconn_close(conn);
				netconn_delete(conn);
				netbuf_delete(inbuf);
			}
			else if (strstr(buf, "GET /favicon.ico "))
			{
				ESP_LOGI(TAG2, "Requested favicon, however none to send yet");
				netconn_write(conn, CSS_HEADER, sizeof(CSS_HEADER) - 1, NETCONN_NOCOPY);
				netconn_close(conn);
				netconn_delete(conn);
				netbuf_delete(inbuf);
			}
			else
			{
				ESP_LOGI(TAG2, "Unknown request");
				netconn_close(conn);
				netconn_delete(conn);
				netbuf_delete(inbuf);
			}
		}
		else
		{
			ESP_LOGI(TAG2, "Unknown request (empty?...)");
			netconn_close(conn);
			netconn_delete(conn);
			netbuf_delete(inbuf);
		}
	}
	else
	{ // if err!=ERR_OK
		ESP_LOGI(TAG2, "error on read, closing connection");
		netconn_close(conn);
		netconn_delete(conn);
		netbuf_delete(inbuf);
	}
}
