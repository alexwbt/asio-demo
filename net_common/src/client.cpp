#include "client.h"

namespace net
{
    Client::Client()
    {
        context_ = std::make_shared<asio::io_context>();
        input_queue_ = std::make_shared<MessageQueue>();
    }

    bool Client::Connect(const std::string& host, uint16_t port)
    {
        try
        {
            asio::ip::tcp::socket socket(*context_);
            connection_ = std::make_unique<Connection>(input_queue_, context_, std::move(socket));

            // connect to server
            auto on_connect = [&](std::error_code error, asio::ip::tcp::endpoint endpoint)
            {
                if (error)
                {
                    std::cout << "Error(Failed to connect to server): " << error.message() << std::endl;
                    return;
                }

                connection_->Listen();
            };
            asio::ip::tcp::resolver resolver(*context_);
            asio::async_connect(connection_->GetSocket(), resolver.resolve(host, std::to_string(port)), on_connect);

            auto run = [&]()
            {
                context_->run();
            };
            context_thread_ = std::thread(run);
        }
        catch (std::exception& e)
        {
            std::cerr << "Error: " << e.what() << "\n";
            return false;
        }
        return true;
    }

    /* Messages */

    void Client::Send(
        EnCommand command,
        std::shared_ptr<const google::protobuf::Message> body
    ) {
        connection_->PushMessage(command, body);
    }

    void Client::HandleMessages()
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
