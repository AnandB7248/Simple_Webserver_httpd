# Simple_Webserver_httpd

**Overview**: httpd is a simple webserver that supports a subset of the Hypertext Transfer Protocol(HTTP). In addition, the server supports functionality for program invocation similar to the common gateway interface approach.

*Usage*: Server Invocation: /http [port]
                            port - Value between 1024 and 65535

*Client* 
*Supported HTTP Request Types*: HEAD and GET

*Proper request from client*: TYPE filename HTTP/version
                     example: GET /index.html HTTP/1.1

*Note*: Since this is a simplified webserver. HTTP version is ignored.


**cgi-like Support**
* Server provides support for executing programs on the server and providing the outback back to the user.
* Only the programs in the cgi-like directory (a subdirectory of the server's working directory) may be executed. These programs may be passed arguments as part of the request. These arguments are provided, in the URL, after a ? following the program name and the arguments are separated by & characters. The arguments themselves are simply strings. If there are no argumens, then there will be no ?.
* As such, the request header for an attempt to run(as an example) the ls program might be as follows (to list the two files mentioned in long format).
   
   GET /cgi-like/ls?-l&index.html&main.html HTTP/1.1
