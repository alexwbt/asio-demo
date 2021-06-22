#pragma once

#include "connection.h"

namespace net
{
    struct ServerCallback
    {
        virtual void OnClientConnect(std::shared_ptr<Connection>) const {}
        virtual void OnClientDisconnect(std::shared_ptr<Connection>) const {}
        virtual void OnMessage(std::shared_ptr<const Packet>) const {}
    };

    class Server final
    {
    private:
        std::thread context_thread_;
        std::shared_ptr<asio::io_context> context_;
        std::unique_ptr<asio::ip::tcp::acceptor> acceptor_;

        std::shared_ptr<MessageQueue<Packet>> input_queue_;
        std::list<std::shared_ptr<Connection>> connections_;
        uint32_t next_connection_id_ = 1;

        MessageQueue<asio::ip::tcp::socket> new_connection_queue_;

    public:
        Server(uint16_t port);
        ~Server();

        /* Server */
    public:
        bool Start();
        void Stop();

    private:
        void AcceptConnection();

        /* Messages */
    public:
        void SendToAll(
            uint64_t command,
            std::shared_ptr<const google::protobuf::Message> body = nullptr
        );

        void Update(const ServerCallback& callback);
    };
}
