#include "client.h"

namespace net
{
    Client::Client()
    {
        context_ = std::make_shared<asio::io_context>();
        input_queue_ = std::make_shared<MessageQueue<Packet>>();
    }

    Client::~Client()
    {
        Disconnect();
    }

    bool Client::Connect(const std::string& host, uint16_t port)
    {
        try
        {
            auto socket = std::make_shared<asio::ip::tcp::socket>(*context_);
            connection_ = std::make_shared<Connection>(input_queue_, context_, std::move(socket));

            asio::ip::tcp::resolver resolver(*context_);
            asio::connect(*connection_->GetSocket(), resolver.resolve(host, std::to_string(port)));

            connection_->Listen();

            auto run = [this]()
            {
                context_->run();
            };
            context_thread_ = std::thread(run);
        }
        catch (std::exception& e)
        {
            return false;
        }
        return true;
    }

    void Client::Disconnect()
    {
        connection_->Disconnect();

        context_->stop();

        if (context_thread_.joinable())
            context_thread_.join();
    }

    bool Client::Connected()
    {
        return connection_->Connected();
    }

    /* Messages */

    void Client::Send(
        uint64_t command,
        std::shared_ptr<const google::protobuf::Message> body
    ) {
        connection_->PushMessage(command, body);
    }

    void Client::Update(const ClientCallback& callback)
    {
        // handle disconnection
        if (connection_ && !connection_->Connected())
            callback.OnDisconnect(std::move(connection_));

        // handle messages
        auto handle_message = [&callback](std::shared_ptr<Packet> packet)
        {
            callback.OnMessage(std::move(packet));
        };
        input_queue_->HandleAll(handle_message);
    }
}
