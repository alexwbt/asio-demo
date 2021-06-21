#include "server.h"

namespace net
{
    Server::Server(uint16_t port)
    {
        context_ = std::make_unique<asio::io_context>();
        acceptor_ = std::make_unique<asio::ip::tcp::acceptor>(
            *context_, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port));

        input_queue_ = std::make_shared<MessageQueue>();
    }

    /* Server */

    bool Server::Start()
    {
        try
        {
            Listen();
            auto run = [&]()
            {
                context_->run();
            };
            context_thread_ = std::thread(run);
        }
        catch (std::exception& e)
        {
            std::cerr << "Error(Failed to start server): " << e.what() << std::endl;
            return false;
        }

        std::cout << "Server accepting connections..." << std::endl;
        return true;
    }

    void Server::Stop()
    {
        context_->stop();

        if (context_thread_.joinable())
            context_thread_.join();

        std::cout << "Server stopped." << std::endl;
    }

    void Server::Listen()
    {
        auto on_accept = [&](std::error_code error, asio::ip::tcp::socket socket)
        {
            if (error)
            {
                std::cout << "Error(Failed to accept connection): "
                    << error.message() << std::endl;
                return;
            }

            auto connection = std::make_shared<Connection>(
                input_queue_, context_, std::move(socket), next_connection_id_);

            connection->Listen();
            connections_.push_back(std::move(connection));

            std::cout << "Connection(" << next_connection_id_ << "): "
                << "Acccepted connection." << std::endl;

            ++next_connection_id_;
            Listen();
        };
        acceptor_->async_accept(on_accept);
    }

    /* Messages */

    void Server::SendToAll(
        EnCommand command,
        std::shared_ptr<const google::protobuf::Message> body
    ) {
        for (const auto& connection : connections_)
            connection->PushMessage(command, body);
    }

    void Server::HandleMessages()
    {
        while (true)
        {
            auto item = input_queue_->Front();
            auto empty = input_queue_->Pop();

            if (!item)
                break;

            OnMessage(item->header.command, item->body);

            if (empty)
                break;
        }
    }
}
