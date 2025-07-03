# include "Connection.hpp"
# include "conf/Server.hpp"

Connection::Connection()
:	fd(-1), req(NULL), res(NULL), connect(false)
{
}

// Connection*    Connection::addConnection(int fd)
// {
//     return
// }

Server* Connection::findServer()
{

    return NULL;
}