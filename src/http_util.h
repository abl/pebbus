#ifndef HTTP_UTIL_H
#define HTTP_UTIL_H

#define APP_HTTP_COOKIE 8675309
#define STOP_HTTP_COOKIE 8675310
#define ARRIVAL_HTTP_COOKIE 8675311

extern long http_latitude;
extern long http_longitude;
extern bool http_located;
extern bool http_connected;

void http_request_stops();

void http_request_arrivals(char *stop);

void http_success(int32_t cookie, int http_status, DictionaryIterator* received, void* context);

void http_failed(int32_t cookie, int http_status, void* context);

void http_location(float latitude, float longitude, float altitude, float accuracy, void* context);

void http_reconnect(void* context);

#endif