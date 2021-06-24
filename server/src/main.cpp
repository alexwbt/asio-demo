#include <server.h>

struct ServerCallback : public net::ServerCallback
{
    void OnClientConnect(std::shared_ptr<net::Connection> c) const override
    {
        std::cout << "New client(" << c->GetId() << ") connected." << std::endl;
    }

    void OnClientDisconnect(std::shared_ptr<net::Connection> c) const override
    {
        std::cout << "Client(" << c->GetId() << ") disconnected." << std::endl;
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
    constexpr uint16_t kPort = 5625;
    net::Server server(kPort);
    auto success = server.Start();
    if (!success)
    {
        std::cout << "Failed to start server." << std::endl;
        return -1;
    }
    std::cout << "Server started on port " << kPort << std::endl;

    auto message = std::make_shared<proto::StringMessage>();
    message->set_message("hello world");

    ServerCallback callback;
    while (true)
    {
        server.SendToAll(1, message);
        server.Update(callback);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    server.Stop();
}
