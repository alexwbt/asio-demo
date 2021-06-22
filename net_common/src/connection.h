#pragma once

#include "asio_common.h"
#include "message_queue.h"

namespace net
{
    class Connection
    {
    private:
        std::shared_ptr<asio::io_context> context_;
        std::shared_ptr<asio::ip::tcp::socket> socket_;

        uint32_t id_;

        MessageQueue<MessageItem> output_queue_;

        std::shared_ptr<MessageQueue<Packet>> input_queue_;
        std::shared_ptr<Packet> temp_input_item_;

    public:
        Connection(
            std::shared_ptr<MessageQueue<Packet>> input_queue,
            std::shared_ptr<asio::io_context> context,
            std::shared_ptr<asio::ip::tcp::socket> socket,
            uint32_t id = 0
        );

        bool Connected() const;

        uint32_t GetId() const
        {
            return id_;
        }

        std::shared_ptr<asio::ip::tcp::socket> GetSocket()
        {
            return socket_;
        }

        void Listen();
        void Disconnect();

        /* Output */
    public:
        void PushMessage(
            uint64_t command,
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
