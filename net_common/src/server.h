#pragma once

#include "connection.h"

namespace net
{
    class Server
    {
    private:
        std::thread context_thread_;
        std::shared_ptr<asio::io_context> context_;
        std::unique_ptr<asio::ip::tcp::acceptor> acceptor_;

        std::list<std::shared_ptr<Connection>> connections_;
        std::shared_ptr<MessageQueue> input_queue_;
        uint32_t next_connection_id_ = 1;

    public:
        Server(uint16_t port);

        /* Server */
    public:
        bool Start();
        void Stop();

    private:
        void Listen();

        /* Messages */
    public:
        void SendToAll(
            EnCommand command,
            std::shared_ptr<const google::protobuf::Message> body = nullptr
        );

        void HandleMessages();

        virtual void OnMessage(
            EnCommand command,
            std::shared_ptr<const google::protobuf::Message> body
        ) = 0;
    };
}
