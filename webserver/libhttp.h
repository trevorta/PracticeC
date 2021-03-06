/*
 * A simple HTTP library.
 *
 * Usage example:
 *
 *     // Returns NULL if an error was encountered.
 *     struct http_request *request = http_request_parse(fd);
 *
 *     ...
 *
 *     http_start_response(fd, 200);
 *     http_send_header("Content-type", http_get_mime_type("index.html"));
 *     http_send_header("Server", "httpserver/1.0");
 *     http_end_headers();
 *     http_send_string("<html><body><a href='/'>Home</a></body></html>");
 *
 *     close(fd);
 */

#ifndef LIBHTTP_H
#define LIBHTTP_H

/*
 * Functions for parsing an HTTP request.
 */
struct http_request // return an http request
{
  char *method;
  char *path;
};

struct http_request *http_request_parse(int fd); // parse using an id of a socket

/*
 * Functions for sending an HTTP response.
 */
void http_start_response(int fd, int status_code); // sending a response to a request
void http_send_header(int fd, char *key, char *value);
void http_end_headers(int fd);
void http_send_string(int fd, char *data);
void http_send_data(int fd, char *data, size_t size);

/*
 * Helper function: gets the Content-Type based on a file name.
 */
char *http_get_mime_type(char *file_name); // get a content type of a file

#endif
