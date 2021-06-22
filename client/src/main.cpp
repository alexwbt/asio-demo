#include <client.h>

struct ClientCallback : public net::ClientCallback
{
    void OnDisconnect(std::shared_ptr<net::Connection> c) const override
    {
        std::cout << "Disconnected from server." << std::endl;
    }

    void OnMessage(std::shared_ptr<const net::Packet> packet) const override
    {
        std::cout << "You have a message (" << packet->header.command << ")." << std::endl;
        proto::StringMessage message;
        if (message.ParseFromArray(packet->body->data(), static_cast<int>(packet->body->size())))
            std::cout << message.message() << std::endl;
    }
};

int main()
{
    net::Client client;
    auto success = client.Connect("127.0.0.1", 5625);
    if (!success)
    {
        std::cout << "Failed to connect to server." << std::endl;
        return -1;
    }
    std::cout << "Connected to server." << std::endl;

    ClientCallback callback;
    while (true)
    {
        client.Update(callback);
    }
}
