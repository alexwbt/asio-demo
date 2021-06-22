#include "server.h"

namespace net
{
    Server::Server(uint16_t port)
    {
        context_ = std::make_unique<asio::io_context>();
        acceptor_ = std::make_unique<asio::ip::tcp::acceptor>(
            *context_, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port));

        input_queue_ = std::make_shared<MessageQueue<Packet>>();
    }

    Server::~Server()
    {
        Stop();
    }

    /* Server */

    bool Server::Start()
    {
        try
        {
            AcceptConnection();
            auto run = [this]()
            {
                context_->run();
            };
            context_thread_ = std::thread(run);
        }
        catch (const std::exception&)
        {
            return false;
        }
        return true;
    }

    void Server::Stop()
    {
        context_->stop();

        if (context_thread_.joinable())
            context_thread_.join();
    }

    void Server::AcceptConnection()
    {
        auto on_accept = [this](std::error_code error, asio::ip::tcp::socket socket)
        {
            if (!error)
            {
                auto socket_ptr = std::make_shared<asio::ip::tcp::socket>(std::move(socket));
                new_connection_queue_.Push(socket_ptr);
            }
            AcceptConnection();
        };
        acceptor_->async_accept(on_accept);
    }

    /* Messages */

    void Server::SendToAll(
        uint64_t command,
        std::shared_ptr<const google::protobuf::Message> body
    ) {
        for (const auto& connection : connections_)
            connection->PushMessage(command, body);
    }

    void Server::Update(const ServerCallback& callback)
    {
        // handle new connections
        auto handle_new_connection = [this, &callback](std::shared_ptr<asio::ip::tcp::socket> socket)
        {
            auto connection = std::make_shared<Connection>(
                input_queue_, context_, std::move(socket), next_connection_id_);

            connection->Listen();
            connections_.push_back(connection);

            callback.OnClientConnect(std::move(connection));

            ++next_connection_id_;
        };
        new_connection_queue_.HandleAll(handle_new_connection);

        // handle disconnected connections
        auto remove_if = [&callback](std::shared_ptr<Connection> connection)
        {
            auto disconnected = !connection->Connected();
            if (disconnected)
                callback.OnClientDisconnect(connection);
            return disconnected;
        };
        connections_.remove_if(remove_if);

        // handle messages
        auto handle_message = [&callback](std::shared_ptr<Packet> packet)
        {
            callback.OnMessage(std::move(packet));
        };
        input_queue_->HandleAll(handle_message);
    }
}
