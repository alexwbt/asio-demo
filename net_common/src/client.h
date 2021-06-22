#pragma once

#include "connection.h"

namespace net
{
    struct ClientCallback
    {
        virtual void OnDisconnect(std::shared_ptr<Connection>) const {}
        virtual void OnMessage(std::shared_ptr<const Packet>) const {}
    };

    class Client final
    {
    private:
        std::thread context_thread_;
        std::shared_ptr<asio::io_context> context_;

        std::shared_ptr<Connection> connection_;
        std::shared_ptr<MessageQueue<Packet>> input_queue_;

    public:
        Client();
        ~Client();

        bool Connect(const std::string& host, const uint16_t port);
        void Disconnect();

        bool Connected();

        void Send(
            uint64_t command,
            std::shared_ptr<const google::protobuf::Message> body = nullptr
        );

        /* Messages */

        void Update(const ClientCallback& callback);
    };
}
