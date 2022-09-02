/*
	ToDo: 	- pack all the local variables into the class--> they are static ones, so we must re-declare them in this file!
			- get rid of all unnecessary inludes

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
#include "http_parser.h"

#include "maag_webserver.h"

// static TAG
static const char *TAG = "maag_webserver";

// Define our static class member variables
//void *MaagWebserver::m_pHttp_serve_args;
//int MaagWebserver::m_iDefaultArg;
http_serve_function MaagWebserver::httpServeFunc;
QueueHandle_t MaagWebserver::m_client_queue;
const int MaagWebserver::m_client_queue_size = 10;

// =============================================================================================================
// STATIC FUNCTIONS
// =============================================================================================================
// handles clients when they first connect. passes to a queue
// static void server_task(void *pvParameters)
void MaagWebserver::server_task(void *pArgs)
{
	const static char *TAG2 = "server_task";
	struct netconn *conn, *newconn;
	static err_t err;
	m_client_queue = xQueueCreate(m_client_queue_size, sizeof(struct netconn *));

	conn = netconn_new(NETCONN_TCP);
	netconn_bind(conn, NULL, 80);
	netconn_listen(conn);
	ESP_LOGI(TAG2, "server listening");
	do
	{
		err = netconn_accept(conn, &newconn);
		// ESP_LOGI(TAG2,"new client");
		if (err == ERR_OK)
		{
			// ESP_LOGI(TAG2, "newconn->socket: %d", newconn->socket);
			xQueueSendToBack(m_client_queue, &newconn, portMAX_DELAY);
			// http_serve(newconn);
		}
		vTaskDelay(pdMS_TO_TICKS(1));
	} while (err == ERR_OK);
	netconn_close(conn);
	netconn_delete(conn);
	ESP_LOGE(TAG2, "task ending, rebooting board");
	esp_restart();
}
// receives clients from queue, handles them
void MaagWebserver::server_handle_task(void *pArgs)
{
	const static char *TAG2 = "server_handle_task";
	struct netconn *conn;
	ESP_LOGI(TAG2, "task starting");
	for (;;)
	{
		xQueueReceive(m_client_queue, &conn, portMAX_DELAY);
		if (!conn)
			continue;
		// http_serve(conn);
		// call user http_server function here
		httpServeFunc(conn, pArgs);
	}
	vTaskDelete(NULL);
}

// =============================================================================================================
// CLASS MaagWebserver
// =============================================================================================================
MaagWebserver::MaagWebserver(/* args */)
{
	ESP_LOGW(TAG, "MaagWebserver instance created, setting default http_serve function...");
	m_newDefaultArg = 0;
	// set our http_serve function pointer to default http_serve function
	setHttpServeFunc(default_http_serve); 		
}

void MaagWebserver::setHttpServeFunc(http_serve_function pHttpServeFunc)
{
	ESP_LOGI(TAG, "setting http_server function pointer");
	httpServeFunc = pHttpServeFunc;
}

void MaagWebserver::createServer(BaseType_t xCoreID)
{
	ESP_LOGI(TAG, "Creating server_task and server_handle_task...");
	if (xCoreID > 1 || xCoreID < 0)
	{
		ESP_LOGE(TAG, "Invalid Core ID. Setting to default (Core 0)");
		xCoreID = 0;
	}
	// create the two tasks needed for http webserver
	xTaskCreatePinnedToCore(server_task, "server_task", 3000, NULL, 5, NULL, xCoreID);
	// we must pass our http_server arguments here because http_server function is called in this created task
	xTaskCreatePinnedToCore(server_handle_task, "server_handle_task", 4000, this, 5, NULL, xCoreID);
}

void MaagWebserver::default_http_serve(struct netconn *conn, void *pArgs)
{
	const static char *TAG2 = "default_http_serve";

	// convert passed arguments to what we know they are
	MaagWebserver *pMaagWebserver = (MaagWebserver *)pArgs;

	// const static char HTML_HEADER[] = "HTTP/1.1 200 OK\nContent-type: text/html\n\n";

	const static char HTML_HEADER[] = "HTTP/1.1 200 OK\nAccess-Control-Allow-Origin: *\nContent-type: text/html\n\n";

	// const static char ERROR_HEADER[] = "HTTP/1.1 404 Not Found\nContent-type: text/html\n\n";
	const static char JS_HEADER[] = "HTTP/1.1 200 OK\nContent-type: text/javascript\n\n";
	const static char CSS_HEADER[] = "HTTP/1.1 200 OK\nContent-type: text/css\n\n";
	// const static char PNG_HEADER[] = "HTTP/1.1 200 OK\nContent-type: image/png\n\n";
	// const static char ICO_HEADER[] = "HTTP/1.1 200 OK\nContent-type: image/x-icon\n\n";
	// const static char PDF_HEADER[] = "HTTP/1.1 200 OK\nContent-type: application/pdf\n\n";
	// const static char EVENT_HEADER[] = "HTTP/1.1 200 OK\nContent-Type: text/event-stream\nCache-Control: no-cache\nretry: 3000\n\n";
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
			// increment testdata and log
			pMaagWebserver->m_newDefaultArg++;
			ESP_LOGW(TAG2, "Current Communication Count with default webserver: %i", pMaagWebserver->m_newDefaultArg);

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
