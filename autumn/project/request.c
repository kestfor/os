#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "request.h"

char *GET = "GET";
char *HEAD = "HEAD";
char *POST = "POST";


void get_string_method(const enum methods method, char **res) {
    switch (method) {
        case get:
            *res = GET;
            return;
        case head:
            *res = HEAD;
            return;
        case post:
            *res = POST;
            return;
        default:
            *res = NULL;
    }
}

void clear_request(http_request *req) {
    free(req->hostname);
    free(req->version);
    free(req->path);
    free(req->headers);
    free(req);
}

char *to_string(const http_request *req) {
    char method_buff[4];
    char *method = method_buff;

    get_string_method(req->method, &method);
    const size_t method_len = strlen(method);
    const size_t path_len = strlen(req->path);
    const size_t version_len = strlen(req->version);
    const size_t headers_len = strlen(req->headers);
    const size_t res_len = method_len + path_len + version_len + headers_len + 3;
    char *result = malloc(res_len * sizeof(char));

    snprintf(result, res_len, "%s %s %s%s", method, req->path, req->version, req->headers);
    result[res_len - 1] = '\0';

    return result;
}

http_request *create_request(const char *request) {
    const char *space = strchr(request, ' ');
    if (space == NULL) {
        return NULL;
    }

    const long space_ind = space - request;
    char method[space_ind + 1];

    strncpy(method, request, space_ind);
    method[space_ind] = '\0';
    enum methods method_num;
    if (strcmp(method, GET) == 0) {
        method_num = get;
    } else if (strcmp(method, POST) == 0) {
        method_num = post;
    } else if (strcmp(method, HEAD) == 0) {
        method_num = head;
    } else {
        printf("unsupported method: [%s]\n", method);
        return NULL;
    }

    const char *http = strstr(space, "http://");

    const int http_len = http == NULL ? 1 : 8;
    const char *hostname_start = space + http_len;
    const char *hostname_end = strchr(hostname_start, '/');
    if (hostname_end == NULL) {
        printf("invalid hostname\n");
        return NULL;
    }

    const char *path_start = hostname_end;
    const char *path_end = strchr(path_start, ' ');
    if (path_end == NULL) {
        printf("invalid path");
        return NULL;
    }


    const char *version_start = path_end + 1;
    char *version_end = strstr(version_start, "\r\n");
    if (version_end == NULL) {
        printf("invalid version");
        return NULL;
    }

    char *hostname = malloc(sizeof(char) * (hostname_end - hostname_start + 1));
    strncpy(hostname, hostname_start, hostname_end - hostname_start);
    hostname[hostname_end - hostname_start] = '\0';

    char *path = malloc(sizeof(char) * (path_end - path_start + 1));
    strncpy(path, path_start, path_end - path_start);
    path[path_end - path_start] = '\0';

    char *version = malloc(sizeof(char) * (version_end - version_start + 1));
    strncpy(version, version_start, version_end - version_start);
    version[version_end - version_start] = '\0';


    const char *headers_end = request + strlen(request);
    const char *headers_start = version_end;
    char *headers = malloc(sizeof(char) * (headers_end - headers_start + 1));
    strncpy(headers, headers_start, headers_end - headers_start);
    headers[headers_end - headers_start] = '\0';

    http_request *res = malloc(sizeof(http_request));

    res->headers = headers;
    res->hostname = hostname;
    res->path = path;
    res->version = version;
    res->method = method_num;

    return res;
}
