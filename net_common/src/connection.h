#pragma once

#include "asio_common.h"
#include "message_queue.h"

namespace net
{
    class Connection
    {
    private:
        std::shared_ptr<asio::io_context> context_;
        asio::ip::tcp::socket socket_;

        uint32_t id_;

        MessageQueue output_queue_;

        std::shared_ptr<MessageQueue> input_queue_;
        std::shared_ptr<MessageItem> temp_input_item_;

    public:
        Connection(
            std::shared_ptr<MessageQueue> input_queue,
            std::shared_ptr<asio::io_context> context,
            asio::ip::tcp::socket socket,
            uint32_t id = 0
        );

        bool Connected() const;

        auto& GetSocket()
        {
            return socket_;
        }

        void Listen();
        void Disconnect();

        /* Output */
    public:
        void PushMessage(
            EnCommand command,
            std::shared_ptr<const google::protobuf::Message> body = nullptr
        );
    private:
        void SendHeader();
        void SendBody();
        void WrittenOneMessage();

        /* Input */
        void ReadHeader();
        void ReadBody();
        void ReadOneMessage();
    };
}
