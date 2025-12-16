#ifndef HTTP_H
#define HTTP_H

#define BUFFER_SIZE 1024
#define MAX_METHOD_SIZE 10
#define MAX_PATH_SIZE 256
#define MAX_HEADERS 16

typedef struct {
  char method[MAX_METHOD_SIZE];
  char path[MAX_PATH_SIZE];
  char headers[MAX_HEADERS][BUFFER_SIZE];
  int headers_len;
} http_request;

http_request parse_http_request(char *request);

typedef struct {
  int status;
  char headers[MAX_HEADERS][BUFFER_SIZE];
  int headers_len;

  char body[BUFFER_SIZE];
} http_response;

http_response build_http_response(int status, char *headers[], int headers_len,
                                  char *body);
char *http_response_to_string(http_response response);

http_response redirect_to_response(char *path);
http_response not_found_response();

#endif
