#include <arpa/inet.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <unistd.h>

#include "libhttp.h"

/*
 * Global configuration variables.
 * You need to use these in your implementation of handle_files_request and
 * handle_proxy_request. Their values are set up in main() using the
 * command line arguments (already implemented for you).
 */
int server_port; // number of port, for making response using libhttp
char *server_files_directory; // where to look for files
char *server_proxy_hostname; // use for setting up proxy
int server_proxy_port; // make 2 connections, and this will transfer informatoin from proxy to client - we are the middle man
//int BUF_LEN = 100;
int BUF_LEN = 512;


/** Return the path to the file if this is a valid file.
    Return NULL otherwise.*/
char* valid_file(char* path) {
    struct stat stat_buf;


    // check if path starts with "/"
    if ((path == NULL) || (path[0] != '/')) {
        return NULL;
    }

    // check for relative path
    char* relative_path = malloc(strlen(path) + 1 + strlen(server_files_directory));
    strcpy(relative_path, server_files_directory);
    strcat(relative_path, path);

    //printf("%s\n", relative_path);

    int stat_res = stat(relative_path, &stat_buf);
    if ((stat_res == 0) && (S_ISREG(stat_buf.st_mode))) { // a file
        return relative_path;
    }

    stat_res = stat(path, &stat_buf);
    // check for absolute path first
    if ((stat_res == 0) && (S_ISREG(stat_buf.st_mode))) { // a file
        return path;
    }

    return NULL;

}

/** Return 1 if path is a valid file.
    Return 0 otherwise.*/
int exist_file(char* path) {
    struct stat stat_buf;
    int stat_res = stat(path, &stat_buf);

    // check if path starts with "/"
    // check for absolute path first
    if ((stat_res == 0) && (S_ISREG(stat_buf.st_mode))) { // a file
        return 1;
    }
    return 0;
}


/** Return the path to the dir if this path is a valid directory.
    Return NULL otherwise. */
char* valid_dir(char* path) {
    struct stat stat_buf;
    int stat_res;

    // check if path starts with "/"
    if ((path == NULL) || (path[0] != '/')) {
        return NULL;
    }

    /* char cwd[1024]; */
    /* getcwd(cwd, sizeof(cwd)); */
    /* char* relative_path = malloc(strlen(path) + 1 + strlen(cwd)); */
    /* strcpy(relative_path, cwd); */
    /* strcat(relative_path, path); */

    //check for relative path
    char* relative_path = malloc(strlen(path) + 1 + strlen(server_files_directory));
    strcpy(relative_path, server_files_directory);
    strcat(relative_path, path);


    stat_res = stat(relative_path, &stat_buf);
    if ((stat_res == 0) && (S_ISDIR(stat_buf.st_mode))) { // a directory
        return relative_path;
    }


    stat_res = stat(path, &stat_buf);
    // check for absolute path first
    if ((stat_res == 0) && (S_ISDIR(stat_buf.st_mode))) { // a directory
        return path;
    }

    return NULL;
}


/** Input needs to already be a valid directory.
    Return the path if this directory contains target file.
    Return NULL otherwise. */
char* contain_file(char* path, char* target) {
    struct stat stat_buf;

    char* file = malloc(strlen(path) + strlen(target) + 2);
    strcpy(file, path);
    strcat(file, "/");
    strcat(file, target);

    int stat_res = stat(file, &stat_buf);

    if ((stat_res == 0) && (S_ISREG(stat_buf.st_mode))) {
        return file;
    }

    return NULL;
}


/* Send strings of data in FILENAME to request stream FD. */
void read_file(char* filename, int fd) {

    char buf[BUF_LEN];
    FILE * fp;
    size_t bytes_read;

    fp = fopen(filename, "r");
    if (fp == NULL) {
        exit(1);
    }

    while((bytes_read = fread(buf, sizeof(char), BUF_LEN, fp)) != 0) {
        http_send_data(fd, buf, bytes_read);
    }

}


/** Return an HTML link with PATH as href and content of FILENAME. */
char* create_link(char* path, char* filename) {
    char* result = malloc(strlen(path) + strlen(filename) + 15);
    strcpy(result, "<p><a href=\"");
    strcat(result, path);
    strcat(result, "\">");
    strcat(result, filename);
    strcat(result, "</a></p>");
    return result;
}

/** Return a new string of C2 appends to the end of C1 using separator SEP. */
char* string_append(char* c1, char* c2, char* sep) {
    char* result = malloc(strlen(c1) + strlen(c2) + 2);
    strcpy(result, c1);
    strcat(result, sep);
    strcat(result, c2);
    return result;
}

/** List the files inside DIR. */
void list_dir(char* dir_name, int fd) {
    DIR *dir;
    //struct dirent *dp = malloc(sizeof(struct dirent));
    struct dirent *dp;
    char* full_path;
    char* link;


    if ((dir = opendir(dir_name)) == NULL) {
        //perror("Cannot open directory");
        perror("Cannot open directory");
        exit(1);
    }

    link = create_link("../", "Parent directory");
    http_send_string(fd, link);

    while ((dp = readdir(dir)) != NULL) {
        full_path = string_append(dir_name, dp->d_name, "/");

        if (exist_file(full_path)) {
            link = create_link(full_path, dp->d_name);
            http_send_string(fd, link);
        }
    }
}

void sample(int fd) {
    http_start_response(fd, 200);
    http_send_header(fd, "Content-type", "text/html");
    http_end_headers(fd);
    http_send_string(fd, "<center><h1>Welcome to httpserver!</h1><hr><p>Nothing's here yet.</p></center>");
}

void error(int fd) {
    http_start_response(fd, 404);
    http_send_header(fd, "Content-type", "text/html");
    http_end_headers(fd);
    http_send_string(fd, "<center><h1>Invalid request!</h1></center>");
}


/*
 * Reads an HTTP request from stream (fd), and writes an HTTP response
 * containing:
 *
 *   1) If user requested an existing file, respond with the file
 *   2) If user requested a directory and index.html exists in the directory,
 *      send the index.html file.
 *   3) If user requested a directory and index.html doesn't exist, send a list
 *      of files in the directory with links to each.
 *   4) Send a 404 Not Found response.
 */
void handle_files_request(int fd) // with --files command line arguments
{
    /* YOUR CODE HERE */
    struct http_request *request = http_request_parse(fd);
    int status_code;

    //printf("path: %s\n", server_files_directory);

    //printf("request path: %s\n", request->path);
    char* request_path = request->path;
    //char* data = NULL;
    // so far, all we have is welcome to server and nothing here yet - we need to change this to see the files in directory

    char* path;
    //path = valid_file(server_files_directory);
    /* 1) If user requested an existing file, respond with the file */
    if ((path = valid_file(request_path)) != NULL) { // this is a valid file and the file is present
        //printf("recoginized files\n");
        status_code = 200;
        http_start_response(fd, status_code);
        http_send_header(fd, "Content-Type", http_get_mime_type(request_path));
        http_end_headers(fd);
        //printf("%s\n", path);
        //printf("before read file\n");

        read_file(path, fd);
    } else if ((path = valid_dir(request_path)) != NULL) { // this is a valid directory
        //printf("directory path: %s\n", path);
        //path = valid_dir(server_files_directory);
        status_code = 200;
        char* file = contain_file(path, "index.html");
        /* 2) If user requested a directory and index.html exists in the directory, */
        /* send the index.html file. */
        if (file != NULL) { // the directory contains index.html
            // Display full content of a file, with full path in file

            //TODO: this doesn't work: ./httpserver --files /.. or /.
            //printf("directory with index.html\n");
            http_start_response(fd, status_code);
            //http_send_header(fd, "Content-Type", http_get_mime_type(server_files_directory));
            http_send_header(fd, "Content-Type", http_get_mime_type(file));
            //http_send_header(fd, "Content-Type", "text/html");
            http_end_headers(fd);
            read_file(file, fd);
            /* 3) If user requested a directory and index.html doesn't exist, send a list */
            /* of files in the directory with links to each. */
        } else {
            //printf("directory without index.html\n");
            http_start_response(fd, status_code);
            http_send_header(fd, "Content-type", "text/html");
            http_end_headers(fd);
            list_dir(path, fd);
        }
    } else {    /* 4) Send a 404 Not Found response. */
        //printf("error\n");
        //status_code = 404;
        //http_start_response(fd, status_code);
        error(fd);
    }

}

/* Read a string of BUFFER from CLIENTFD and write it to SOCKFD. */
int read_from_socket(int infd, int outfd) {
    int nbytes;
    char buffer[BUF_LEN + 1];
    //printf("reading from socket\n");
    //printf("before bytes: %s\n", buffer);
    nbytes = read(infd, buffer, BUF_LEN);
    buffer[nbytes] = '\0'; // null-terminate the string
    //printf("after bytes: %s\n", buffer);
    if (nbytes < 0) {
        //printf("read error\n");
        exit(1);
    } else if (nbytes == 0) {
        return -1;
    } else {
        write(outfd, buffer, nbytes);
        return 0;
    }
}

/*
 * Opens a connection to the proxy target (hostname=server_proxy_hostname and
 * port=server_proxy_port) and relays traffic to/from the stream fd and the
 * proxy target. HTTP requests from the client (fd) should be sent to the
 * proxy target, and HTTP responses from the proxy target should be sent to
 * the client (fd).
 *
 *   +--------+     +------------+     +--------------+
 *   | client | <-> | httpserver | <-> | proxy target |
 *   +--------+     +------------+     +--------------+
 */
void handle_proxy_request(int fd)
{
    int clientfd = fd;

    struct hostent * server = gethostbyname(server_proxy_hostname);;
    if (server != NULL) {
        //printf("hostname: %s\n", server->h_name);
    }

    // get the address list of host
    // the socket file descriptor
    int sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        //printf("error on socket\n");
        exit(1);
    }
    //printf("socket file descriptor : %d\n", sockfd);
    //printf("client file descriptor : %d\n", clientfd);

    struct sockaddr_in server_address;
    int n;

    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    bcopy((char*) server->h_addr, (char*) &server_address.sin_addr.s_addr, server->h_length);
    server_address.sin_port = htons(server_proxy_port); //TODO: using server_port instead of 8000

    //printf("sin_port: %d\n", server_address.sin_port);

    n = connect(sockfd, (struct sockaddr *) &server_address, sizeof(server_address));
    if (n < 0) {
        //printf("ERROR connecting\n");
        exit(1);
    }

    //printf("finish connecting\n");

    fd_set active_fd_set, read_fd_set;
    int i;

    // Initialize the set of active sockets.
    FD_ZERO(&active_fd_set);
    FD_SET(sockfd, &active_fd_set);
    FD_SET(clientfd, &active_fd_set);

    while (1) {
        //printf("inside while loop\n");
        //printf("setsize: %d\n", FD_SETSIZE);
        read_fd_set = active_fd_set;
        if (select(FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) < 0) {
            //printf("error in select\n");
            exit(1);
        }
        //printf("after while loop\n");

        if (FD_ISSET(clientfd, &read_fd_set)) {
            //printf("inside client socket\n");
            if (read_from_socket(clientfd, sockfd) < 0) { // read from the proxy socket
                close(clientfd);
                FD_CLR(i, &active_fd_set);
                //printf("break client\n");
                break;
            }

        } else if (FD_ISSET(sockfd, &read_fd_set)) {
            //printf("inside server socket\n");
            if (read_from_socket(sockfd, clientfd) < 0) { // read from the proxy socket
                close(sockfd);
                FD_CLR(i, &active_fd_set);
                //printf("break socket\n");
                break;
            }

        }
    }


}



/*
 * Opens a TCP stream socket on all interfaces with port number PORTNO. Saves
 * the fd number of the server socket in *socket_number. For each accepted
 * connection, calls request_handler with the accepted fd number.
 */
void serve_forever(int* socket_number, void (*request_handler)(int)) // given a port number and the handler method to call on the new created connection
{
    // socket address structures
    struct sockaddr_in server_address, client_address;
    size_t client_address_length = sizeof(client_address);
    int client_socket_number;
    pid_t pid;

    /* printf("serve forever\n"); */

    // create a new socket
    *socket_number = socket(PF_INET, SOCK_STREAM, 0);
    if (*socket_number == -1) {
        fprintf(stderr, "Failed to create a new socket: error %d: %s\n", errno, strerror(errno));
        exit(errno);
    }

    // set socket option
    int socket_option = 1;
    if (setsockopt(*socket_number, SOL_SOCKET, SO_REUSEADDR, &socket_option,
                   sizeof(socket_option)) == -1) {
        fprintf(stderr, "Failed to set socket options: error %d: %s\n", errno, strerror(errno));
        exit(errno);
    }


    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY; // get any input address, not for a proxy
    server_address.sin_port = htons(server_port);

    // bind on socket
    if (bind(*socket_number, (struct sockaddr*) &server_address,
             sizeof(server_address)) == -1) {
        fprintf(stderr, "Failed to bind on socket: error %d: %s\n", errno, strerror(errno));
        exit(errno);
    }

    // listen to socket
    if (listen(*socket_number, 1024) == -1) {
        fprintf(stderr, "Failed to listen on socket: error %d: %s\n", errno, strerror(errno));
        exit(errno);
    }

    // start listening on port
    printf("Listening on port %d...\n", server_port);

    while (1) {

        // accept client socket number
        client_socket_number = accept(*socket_number, (struct sockaddr*) &client_address,
                                      (socklen_t*) &client_address_length);
        if (client_socket_number < 0) {
            fprintf(stderr, "Error accepting socket: error %d: %s\n", errno, strerror(errno));
            continue;
        }


        printf("Accepted connection from %s on port %d\n", inet_ntoa(client_address.sin_addr),
               client_address.sin_port);

        // fork a different thread to serve the requests
        pid = fork();
        if (pid > 0) { // in parent
            close(client_socket_number); // close the client socket (the processing socket)
        } else if (pid == 0) {
            signal(SIGINT, SIG_DFL); // Un-register signal handler (only parent should have it)
            close(*socket_number);
            // handling request on client socket - using either file or proxy handling
            request_handler(client_socket_number);
            close(client_socket_number);
            exit(EXIT_SUCCESS);
        } else {
            fprintf(stderr, "Failed to fork child: error %d: %s\n", errno, strerror(errno));
            exit(errno);
        }
    }

    close(*socket_number);

}

int server_fd;
void signal_callback_handler(int signum)
{
    printf("Caught signal %d: %s\n", signum, strsignal(signum));
    printf("Closing socket %d\n", server_fd);
    if (close(server_fd) < 0) perror("Failed to close server_fd (ignoring)\n");
    exit(signum);
}

char *USAGE = "Usage: ./httpserver --files www_directory/ --port 8000\n"
    "       ./httpserver --proxy inst.eecs.berkeley.edu:80 --port 8000\n";

void exit_with_usage() {
    fprintf(stderr, "%s", USAGE);
    exit(EXIT_SUCCESS);
}

int main(int argc, char **argv)
{

    signal(SIGINT, signal_callback_handler); // calling signal_callback_handler

    /* Default settings */
    server_port = 8000;
    server_files_directory = malloc(1024);
    getcwd(server_files_directory, 1024); // allocate space for directory for --list
    server_proxy_hostname = "inst.eecs.berkeley.edu"; // set the hostname to berkeley first
    server_proxy_port = 80; // set this too

    void (*request_handler)(int) = handle_files_request;

    int i;
    for (i = 1; i < argc; i++)
        {
            if (strcmp("--files", argv[i]) == 0) {
                request_handler = handle_files_request;
                free(server_files_directory);
                server_files_directory = argv[++i];
                if (!server_files_directory) {
                    fprintf(stderr, "Expected argument after --files\n");
                    exit_with_usage();
                }
            } else if (strcmp("--proxy", argv[i]) == 0) {
                // example: ./httpserver --proxy inst.eecs.berkeley.edu:80 --port 8000
                request_handler = handle_proxy_request; // change the handler (pointer to function) to handle_proxy_request

                char *proxy_target = argv[++i]; // the entire line
                if (!proxy_target) {
                    fprintf(stderr, "Expected argument after --proxy\n");
                    exit_with_usage();
                }

                char *colon_pointer = strchr(proxy_target, ':');
                if (colon_pointer != NULL) {
                    *colon_pointer = '\0';
                    server_proxy_hostname = proxy_target; // split right in the middle of the string, then set the string up to hostname and port
                    server_proxy_port = atoi(colon_pointer + 1); // extract the port number out
                } else {
                    server_proxy_hostname = proxy_target;
                    server_proxy_port = 80;

                    //server_proxy_hostname = inst.eecs.berkeley.edu
                    //server_proxy_port = 80
                }
            } else if (strcmp("--port", argv[i]) == 0) { // parsing from command line
                char *server_port_string = argv[++i];
                if (!server_port_string) {
                    fprintf(stderr, "Expected argument after --port\n");
                    exit_with_usage();
                }
                server_port = atoi(server_port_string);
            } else if (strcmp("--help", argv[i]) == 0) {
                exit_with_usage();
            } else {
                fprintf(stderr, "Unrecognized option: %s\n", argv[i]);
                exit_with_usage();
            }
        }

    serve_forever(&server_fd, request_handler);

    return EXIT_SUCCESS;

}
