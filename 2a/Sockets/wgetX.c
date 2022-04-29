/**
 *  Jiazi Yi
 *
 * LIX, Ecole Polytechnique
 * jiazi.yi@polytechnique.edu
 *
 * Updated by Pierre Pfister
 *
 * Cisco Systems
 * ppfister@cisco.com
 *
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "url.h"
#include "wgetX.h"

int main(int argc, char* argv[]) {
    url_info info;
    const char * file_name = "received_page";
    if (argc < 2) {
        fprintf(stderr, "Missing argument. Please enter URL.\n");
        return 1;
    }

    char *url = argv[1];

    // Get optional file name
    if (argc > 2)
	    file_name = argv[2];
    

    // First parse the URL
    int ret = parse_url(url, &info);
    if (ret) {
        fprintf(stderr, "Could not parse URL '%s': %s\n", url, parse_url_errstr[ret]);
        return 2;
    }

    //If needed for debug
    // print_url_info(&info);

    // Download the page
    struct http_reply reply;

    ret = download_page(&info, &reply);
    if (ret)
	    return 3;

    // Now parse the responses
    char *response = read_http_reply(&reply);
    if (response == NULL) {
        fprintf(stderr, "Could not parse http reply\n");
        return 4;
    }

    // Write response to a file
    write_data(file_name, response, reply.reply_buffer + reply.reply_buffer_length - response);

    // Free allocated memory
    free(reply.reply_buffer);

    // Just tell the user where is the file
    fprintf(stderr, "the file is saved in %s.\n", file_name);
    return 0;
}

int download_page(url_info *info, http_reply *reply) {

    /*
     * To be completed:
     *   You will first need to resolve the hostname into an IP address.
     *
     *   Option 1: Simplistic
     *     Use gethostbyname function.
     *
     *   Option 2: Challenge
     *     Use getaddrinfo and implement a function that works for both IPv4 and IPv6.
     *
     */

    // read from man page for getaddrinfo of man7.org
    struct addrinfo *result, *ptr, hints;                            // defining addrinfo struct
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = 0;
    hints.ai_addrlen = 0;
    hints.ai_addr = NULL;
    hints.ai_canonname = NULL;
    hints.ai_next = NULL;

    char port_addr[6];
    snprintf(port_addr, sizeof(port_addr), "%d", info->port); // convert (int) port to (char *) port for getaddrinfo function 

    /* return result as a pointer to first node of linked list of eligible 
     * addresses because of possiblility of multiple addresses for different 
     * protocols and/or socket types. Also, passing hints as NULL means we are 
     * not specifiying any restrictions on type of sockets we want to connect to
     */
    int error;

    if ((error = getaddrinfo(info->host, port_addr, &hints, &result)) != 0) {
        fprintf(stderr, "error in getaddrinfo: %s: %d\n", strerror(errno), errno); // inspiration from Moodle
        // printf("error = %d\n", error);
        return 1;
    } // failure in getting address info error
    int socketfd, connected;

    // looping over the linked list until we connect to a socket
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
        socketfd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (socketfd == -1) 
            continue;
        if ((connected = connect(socketfd, ptr->ai_addr, ptr->ai_addrlen)) != -1)
            break;
        close(socketfd);
    } // code from https://man7.org/linux/man-pages/man3/getaddrinfo.3.html

    // no need of addrinfo once socket connected / not connected
    freeaddrinfo(result);

    /*
     * To be completed:
     *   Next, you will need to send the HTTP request.
     *   Use the http_get_request function given to you below.
     *   It uses malloc to allocate memory, and snprintf to format the request as a string.
     *
     *   Use 'write' function to send the request into the socket.
     *
     *   Note: You do not need to send the end-of-string \0 character.
     *   Note2: It is good practice to test if the function returned an error or not.
     *   Note3: Call the shutdown function with SHUT_WR flag after sending the request
     *          to inform the server you have nothing left to send.
     *   Note4: Free the request buffer returned by http_get_request by calling the 'free' function.
     *
     */

    char *http_request = http_get_request(info); // get http request

    if (http_request == NULL) {
        fprintf(stderr, "Not enough memory to process http_request: %s: %d\n", strerror(errno), errno);
        return 1;
    }

    if (connected != -1) {
        // inspiration for write function from pubs.opengroup.org
        if (write(socketfd, http_request, strlen(http_request)) < strlen(http_request)) {
            fprintf(stderr, "Write failure: %s: %d\n", strerror(errno), errno); // ask for more clarification ...
            return 1;
        } // return write error
    } else {
        fprintf(stderr, "socket connect failure: %s: %d\n", strerror(errno), errno); // inspiration from tutorialspoint to handle errors
        return 1; // is there anything better I can return ?
    } // write http_request to socket
    
    // inspiration for shutdown function from man page
    if (shutdown(socketfd, SHUT_WR)) {
        fprintf(stderr, "shutdwon error: %s: %d\n", strerror(errno), errno);
        return 1;
    } // shutdown socket transmission but accept reception (half socket shutdown)

    free(http_request); // free http_request buffer because no longer needed

    /*
     * To be completed:
     *   Now you will need to read the response from the server.
     *   The response must be stored in a buffer allocated with malloc, and its address must be save in reply->reply_buffer.
     *   The length of the reply (not the length of the buffer), must be saved in reply->reply_buffer_length.
     *
     *   Important: calling recv only once might only give you a fragment of the response.
     *              in order to support large file transfers, you have to keep calling 'recv' until it returns 0.
     *
     *   Option 1: Simplistic
     *     Only call recv once and give up on receiving large files.
     *     BUT: Your program must still be able to store the beginning of the file and
     *          display an error message stating the response was truncated, if it was.
     *
     *   Option 2: Challenge
     *     Do it the proper way by calling recv multiple times.
     *     Whenever the allocated reply->reply_buffer is not large enough, use realloc to increase its size:
     *        reply->reply_buffer = realloc(reply->reply_buffer, new_size);
     *
     *
     */

    int output_size;
    const int SIZE = 500;          // size of reply in every recv
    reply->reply_buffer = (char *) malloc(SIZE);
    reply->reply_buffer_length = 0;
    memcpy(reply->reply_buffer, "", 1);
    char server_reply[SIZE];              // buffer to receive reply
    char *last_ptr = reply->reply_buffer;

    if (reply == NULL) {
        fprintf(stderr, "Not enough memory to receive reply: %s: %d\n", strerror(errno), errno);
        return 1;
    }

    // inspiration for while loop from https://www.binarytides.com/receive-full-data-with-recv-socket-function-in-c/
    while (1) {
        if ((output_size = recv(socketfd, server_reply, SIZE, 0)) < 0) {
            fprintf(stderr, "Error in receiving messages: %s: %d\n", strerror(errno), errno);
            return 1;
        } else if (output_size == 0) {
            break; // break until no output received
        }
        
        if (sizeof(reply->reply_buffer) <= (reply->reply_buffer_length+output_size)) {
            reply->reply_buffer_length += output_size; // add output length
            reply->reply_buffer = realloc(reply->reply_buffer, reply->reply_buffer_length); // realloc memory if not enough space in buffer
            if (reply->reply_buffer == NULL) {
                fprintf(stderr, "Ran out of memory while reading reply: %s: %d\n", strerror(errno), errno);
                return 1;
            } // check for enough memory after realloc
        }

        last_ptr = reply->reply_buffer + reply->reply_buffer_length - output_size;
        memcpy(last_ptr, server_reply, output_size); // concatenate outputs
    }

    close(socketfd); // close the socket eventually
    return 0;
}

void write_data(const char *path, const char * data, int len) {
    /*
     * To be completed:
     *   Use fopen, fwrite and fclose functions.
     */

    // inspiration to write this code from man pages

    FILE *file = fopen(path, "w");
    
    if (file == NULL) { // error found on man page
        // better code found at https://en.cppreference.com/w/c/program/EXIT_status
        fprintf(stderr,"fopen() failed in file %s at line # %d", __FILE__,__LINE__);
        exit(EXIT_FAILURE);
    } // check if file was created

    if (fwrite(data, 1, len, file) == 0) { //error found on man page
        fprintf(stderr, "Error in writing file: %s: %d\n", strerror(errno), errno);
        exit(EXIT_FAILURE);
    } // check if all data is written correctly

    if (fclose(file) == EOF) { // error found on man page
        fprintf(stderr, "Could not close file: %s: %d\n", strerror(errno), errno);
        exit(EXIT_FAILURE);
    } // check if file was closed

}

char* http_get_request(url_info *info) {
    char * request_buffer = (char *) malloc(100 + strlen(info->path) + strlen(info->host));
    snprintf(request_buffer, 1024, "GET /%s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n",
	    info->path, info->host);
    return request_buffer;
}

char *next_line(char *buff, int len) {
    if (len == 0)
	    return NULL;

    char *last = buff + len - 1;
    while (buff != last) {
        if (*buff == '\r' && *(buff+1) == '\n')
            return buff;
        buff++;
    }
    return NULL;
}

int redirect_pages = 0; // variable to prevent from infinite redirects

char *redirect(char *loc, struct http_reply *reply) {

    if (redirect_pages == 10) {
        fprintf(stderr, "The page has been lost can't find it. \n");
        redirect_pages = 0;
        return NULL;
    }

    redirect_pages++;
    char *status_line = next_line(loc, strlen(loc));
    *status_line = '\0'; // Make the first line is a null-terminated string

    printf("HTTP requested has been redirected to: %s \n", loc);
    url_info info;
    int ret = parse_url(loc, &info);
    if (ret) {
        fprintf(stderr, "Could not parse redirected URL '%s': %s\n", loc, parse_url_errstr[ret]);
        return NULL;
    }

    ret = download_page(&info, reply);
    if (ret)
	    return NULL;

    // Now parse the responses
    char *response = read_http_reply(reply);
    if (response == NULL) {
        fprintf(stderr, "Could not parse redirected http reply\n");
        return NULL;
    }

    return response;
} // copy pasted from function main


char *read_http_reply(struct http_reply *reply) {

    // Let's first isolate the first line of the reply
    char *status_line = next_line(reply->reply_buffer, reply->reply_buffer_length);
    if (status_line == NULL) {
        fprintf(stderr, "Could not find status\n");
        return NULL;
    }
    *status_line = '\0'; // Make the first line is a null-terminated string

    // Now let's read the status (parsing the first line)
    int status;
    double http_version;
    int rv = sscanf(reply->reply_buffer, "HTTP/%lf %d", &http_version, &status);
    if (rv != 2) {
        fprintf(stderr, "Could not parse http response first line (rv=%d, %s)\n", rv, reply->reply_buffer);
        return NULL;
    }

    if (status != 200) {
        if (status==301 || status==302) {
            char *loc = NULL;
            loc = strstr(status_line+2, "Location: ");
            loc += strlen("Location: ");
            if (loc == NULL) {
                fprintf(stderr, "Request redirected but could not parse new location (%s)\n", status_line+2);
                return NULL;
            }
            char *redirected_reply = redirect(loc, reply);
            return redirected_reply;
        } // statement to redirect URL if error code is 302

        fprintf(stderr, "Server returned status %d (should be 200)\n", status);
        return NULL;
    }

    char *buf = status_line + 2;

    /*
     * To be completed:
     *   The previous code only detects and parses the first line of the reply.
     *   But servers typically send additional header lines:
     *     Date: Mon, 05 Aug 2019 12:54:36 GMT<CR><LF>
     *     Content-type: text/css<CR><LF>
     *     Content-Length: 684<CR><LF>
     *     Last-Modified: Mon, 03 Jun 2019 22:46:31 GMT<CR><LF>
     *     <CR><LF>
     *
     *   Keep calling next_line until you read an empty line, and return only what remains (without the empty line).
     *
     *   Difficul challenge:
     *     If you feel like having a real challenge, go on and implement HTTP redirect support for your client.
     *
     */

    status_line = next_line(buf, reply->reply_buffer_length);

    while (1) {
        *status_line = '\0';
        buf = status_line + 2;
        status_line = next_line(buf, reply->reply_buffer_length);
        if (strncmp(buf, "\r\n", 2) == 0)
            break;
    }
    
    buf+=2;
    return buf;
}
