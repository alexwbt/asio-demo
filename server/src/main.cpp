#include <server.h>

class Server : public net::Server
{
public:
    using net::Server::Server;

    void OnMessage(
        net::EnCommand command,
        std::shared_ptr<const google::protobuf::Message> body
    ) {

    }
};

int main()
{
    Server server(5625);
    server.Start();

    while (true)
    {
        server.HandleMessages();
    }
}
