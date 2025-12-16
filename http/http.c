#include "http.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

http_response build_http_response(int status, char *headers[], int headers_len,
                                  char *body) {
  http_response response;
  response.status = status;
  response.headers_len = headers_len;

  for (int i = 0; i < headers_len; i++) {
    if (response.headers[i] == NULL) {
      perror("response header is null");
      continue;
    }
    strcpy(response.headers[i], headers[i]);
  }

  if (body != NULL) {
    strcpy(response.body, body);
  }

  return response;
}

char *http_response_to_string(http_response response) {
  size_t total_size = BUFFER_SIZE + (BUFFER_SIZE * MAX_HEADERS) + 200;

  char *response_str = malloc(total_size);
  char *headers_str = malloc(BUFFER_SIZE * MAX_HEADERS);

  if (!response_str || !headers_str) {
    free(response_str);
    free(headers_str);
    return NULL;
  }

  char *p = headers_str;
  headers_str[0] = '\0';

  for (int i = 0; i < response.headers_len; i++) {
    int written = snprintf(p, (BUFFER_SIZE * MAX_HEADERS) - (p - headers_str),
                           "%s\r\n", response.headers[i]);
    p += written;
  }

  snprintf(response_str, total_size, "HTTP/1.1 %d\r\n%s\r\n%s", response.status,
           headers_str, response.body);

  free(headers_str);
  return response_str;
}

http_response redirect_to_response(char *path) {
  char *location_redirect_header = malloc(BUFFER_SIZE);
  sprintf(location_redirect_header, "Location: http://localhost:8080%s", path);
  char *redirect_headers[] = {"Content-Type: text/html",
                              location_redirect_header};

  return build_http_response(302, redirect_headers, 2, "Redirect");
}

http_response not_found_response() {
  char *not_found_headers[] = {"Content-Type: text/html", "Content-Length: 0"};

  return build_http_response(404, not_found_headers, 1, "Not Found");
}

http_request parse_http_request(char *request) {
  http_request req;

  char *method = strtok(request, " ");
  strcpy(req.method, method);

  char *path = strtok(NULL, " ");
  strcpy(req.path, path);

  strtok(NULL, "\r\n");
  char *headers = strtok(NULL, "\r\n");
  req.headers_len = 0;

  while (headers != NULL) {
    strcpy(req.headers[req.headers_len], headers);
    req.headers_len++;
    headers = strtok(NULL, "\r\n");
  }

  return req;
}
