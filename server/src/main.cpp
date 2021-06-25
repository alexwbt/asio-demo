#include <server.h>

struct ServerCallback : public net::ServerCallback
{
    void OnClientConnect(std::shared_ptr<net::Connection> c) override
    {
        std::cout << "New client(" << c->GetId() << ") connected." << std::endl;
    }

    void OnClientDisconnect(std::shared_ptr<net::Connection> c) override
    {
        std::cout << "Client(" << c->GetId() << ") disconnected." << std::endl;
    }

    void OnMessage(std::shared_ptr<const net::Packet> packet) override
    {
        if (packet->header.body_size == 0)
            return;

        proto::StringMessage message;
        if (message.ParseFromArray(packet->body->data(), static_cast<int>(packet->body->size())))
            std::cout << message.message() << std::endl;
    }
};

int main()
{
    constexpr uint16_t kPort = 5625;
    net::Server server(kPort);
    auto success = server.Start();
    if (!success)
    {
        std::cout << "Failed to start server." << std::endl;
        return -1;
    }
    std::cout << "Server started on port " << kPort << std::endl;

    ServerCallback callback;
    while (true)
    {
        server.Update(callback);
    }

    server.Stop();
}
