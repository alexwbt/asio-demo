#pragma once

#include "connection.h"

namespace net
{
    class Client
    {
    private:
        std::thread context_thread_;
        std::shared_ptr<asio::io_context> context_;

        std::unique_ptr<Connection> connection_;
        std::shared_ptr<MessageQueue> input_queue_;

    public:
        Client();

        bool Connect(const std::string& host, const uint16_t port);

        void Send(
            EnCommand command,
            std::shared_ptr<const google::protobuf::Message> body = nullptr
        );

        /* Messages */

        void HandleMessages();

        virtual void OnMessage(
            EnCommand command,
            std::shared_ptr<const google::protobuf::Message> body
        ) = 0;
    };
}
