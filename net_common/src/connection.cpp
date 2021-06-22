#include "connection.h"

namespace net
{
    Connection::Connection(
        std::shared_ptr<MessageQueue<Packet>> input_queue,
        std::shared_ptr<asio::io_context> context,
        std::shared_ptr<asio::ip::tcp::socket> socket,
        uint32_t id
    ) :
        input_queue_(std::move(input_queue)),
        context_(std::move(context)),
        socket_(std::move(socket)),
        id_(id)
    {}

    bool Connection::Connected() const
    {
        return socket_->is_open();
    }

    void Connection::Listen()
    {
        ReadHeader();
    }

    void Connection::Disconnect()
    {
        if (context_->stopped() || !Connected())
            return;

        auto disconnect = [this]()
        {
            socket_->close();
        };
        asio::post(*context_, disconnect);
    }

    /* Output */

    void Connection::PushMessage(
        uint64_t command,
        std::shared_ptr<const google::protobuf::Message> body
    ) {
        if (context_->stopped())
            return;

        auto push = [this, command, body]()
        {
            auto item = std::make_shared<MessageItem>();
            item->header.command = command;

            if (body)
            {
                item->header.body_size = body->ByteSizeLong();
                item->body = std::move(body);
            }

            auto was_empty = output_queue_.Push(std::move(item));
            if (was_empty)
                SendHeader();
        };
        asio::post(*context_, push);
    }

    void Connection::SendHeader()
    {
        auto item = output_queue_.Front();
        if (!item)
            return;

        auto buffer = asio::buffer(&item->header, sizeof(Header));

        auto on_sent = [this, item](std::error_code error, std::size_t size)
        {
            if (error)
            {
                // disconnects
                socket_->close();
                return;
            }

            // successfully sent header
            if (item->header.body_size > 0)
                SendBody();
            else
                WrittenOneMessage();
        };
        asio::async_write(*socket_, buffer, on_sent);
    }

    void Connection::SendBody()
    {
        auto item = output_queue_.Front();
        if (!item)
            return;

        auto body = item->body->SerializeAsString();
        auto buffer = asio::buffer(body.data(), body.size());

        auto on_sent = [this](std::error_code error, std::size_t size)
        {
            if (error)
            {
                // disconnects
                socket_->close();
                return;
            }

            // successfully sent body
            WrittenOneMessage();
        };
        asio::async_write(*socket_, buffer, on_sent);
    }

    void Connection::WrittenOneMessage()
    {
        auto empty = output_queue_.Pop();
        if (!empty)
            SendHeader();
    }

    /* Input */

    void Connection::ReadHeader()
    {
        temp_input_item_ = std::make_shared<Packet>();
        auto buffer = asio::buffer(&temp_input_item_->header, sizeof(Header));

        auto on_read = [this](std::error_code error, std::size_t size)
        {
            if (error)
            {
                // disconnects
                socket_->close();
                return;
            }

            // successfully read header
            if (temp_input_item_->header.body_size > 0)
                ReadBody();
            else
                ReadOneMessage();
        };
        asio::async_read(*socket_, buffer, on_read);
    }

    void Connection::ReadBody()
    {
        auto body = std::make_shared<std::vector<char>>();
        body->resize(temp_input_item_->header.body_size);

        auto buffer = asio::buffer(body->data(), body->size());

        auto on_read = [this, body](std::error_code error, std::size_t size)
        {
            if (error)
            {
                // disconnects
                socket_->close();
                return;
            }

            // successfully read body
            temp_input_item_->body = std::move(body);
            ReadOneMessage();
        };
        asio::async_read(*socket_, buffer, on_read);
    }

    void Connection::ReadOneMessage()
    {
        input_queue_->Push(std::move(temp_input_item_));
        ReadHeader();
    }
}
