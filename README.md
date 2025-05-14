# Webserv

![Webserv Architecture](https://miro.medium.com/v2/resize:fit:1400/format:webp/1*age3Dgxl8sz6LZgDIwOSNQ.png)


A lightweight, efficient, and standards-compliant HTTP web server implemented in C++. This project is part of the 42 Network curriculum and follows the HTTP/1.1 specification (RFC 7230), and it's called `GEET`.

## 🚀 Project Overview

**Webserv** is a fully functional HTTP server built from scratch in C++. It supports multiple client connections, configuration parsing, CGI handling, and basic HTTP functionalities. The project showcases the use of system calls like `socket`, `bind`, `listen`, `accept`, and `select`, along with string parsing, file I/O, and process management.

## 📁 Repository Structure

    ├── webserv/
        ├── src/ # Source code for the server 
        ├── include/ # Header files 
        ├── cgi-bin/ # Example CGI scripts 
        ├── conf/ # Server configuration files 
        ├── www/ # Root directory for served content 
        ├── Makefile
        └── README.md

## 👥 Team Members & Contributions

| Name               | GitHub Handle | Responsibilities                                                    |
|--------------------|---------------|---------------------------------------------------------------------|
| JAWAD Anas         | @anasjwd      | Core server architecture, socket management, select-based I/O       |
| LASSIQUI Achraf    | @ahanaf       | Response generation, CGI handling, session                          |
| HANAF Ayoub        | @lassachraf   | HTTP request parsing, error pages, testing, cookies                 |

<!-- > Each team member worked independently on their modules before integrating everything together for the final build. -->

## ⚙️ How to Build

To compile the server, simply run:

```bash
git clone repo-link && make
```

Makefile rules:

```
make clean // To clean the object files:

make fclean // To remove all generated files (including the binary):

make re // To recompile from scratch
```

## 🧪 How to Run

After building, start the server with a configuration file:

```
./webserv conf/server.conf
```

You can edit the conf/server.conf file to set your ports, server names, root directories, and other behavior.

## 🔧 Configuration Example

Here’s a basic example of a server block in the configuration file:
```
server {
    listen 8080;
    server_name localhost;

    location / {
        root ./www;
        index index.html;
    }

    error_page 404 ./www/errors/404.html;

    cgi_extension .py;
    cgi_path /usr/bin/python3;
}
```

You can define multiple server blocks for virtual hosting. Each location block allows customization of route behavior, including root path, index file, allowed methods, and CGI.

## 🌐 Features

    ✅ Non-blocking sockets with select()

    ✅ GET, POST, and DELETE methods

    ✅ HTTP request parsing and response building

    ✅ Autoindex (directory listing)

    ✅ MIME type detection

    ✅ CGI execution (e.g., Python scripts)

    ✅ Configurable via external .conf file

    ✅ Support for custom error pages

    ✅ Virtual hosting with multiple servers

    ✅ Chunked transfer encoding (bonus)

## 💡 Tips

    Make sure the ports you're using are not already in use.

    Place your static content inside the www/ directory.

    CGI scripts should be executable and placed in cgi-bin/.

    You can create your own error pages and point to them in the config file.

## 📄 License

This project is developed as part of the 42 Network curriculum. It is meant for educational purposes only and is not distributed under any specific open-source license.
🙏 Acknowledgements

    The 42 Network community for feedback and support.

    Official C++ and Linux man pages.

    RFC 7230 documentation (HTTP/1.1).

    Online resources and tutorials that helped us understand socket programming, configuration parsing, and web server fundamentals.
