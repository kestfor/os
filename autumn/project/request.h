#ifndef PROXY_REQUEST_H
#define PROXY_REQUEST_H

enum methods {
    get = 0,
    head = 1,
    post = 2
};

typedef struct http_request {
    enum methods method;
    char *hostname;
    char *path;
    char *version;
    char *headers;
} http_request;

void get_string_method(enum methods method, char **res);
http_request *create_request(const char *request);
void clear_request(http_request *req);
char *to_string(http_request *req);

#endif //PROXY_REQUEST_H
