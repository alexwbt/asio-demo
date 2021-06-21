#include <client.h>

class Client : public net::Client
{
    void OnMessage(
        net::EnCommand command,
        std::shared_ptr<const google::protobuf::Message> body
    ) {

    }
};

int main()
{
    Client client;
    client.Connect("127.0.0.1", 5625);

    while (true)
    {
        client.HandleMessages();
    }
}
