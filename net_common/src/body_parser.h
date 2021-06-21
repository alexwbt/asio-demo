#pragma once

#include "message_queue.h"

namespace net
{
    template <typename ProtoType>
    std::shared_ptr<google::protobuf::Message> Parse(const void* data, int size)
    {
        auto item = std::make_shared<ProtoType>();
        if (item->ParseFromArray(data, size))
            return item;
        return nullptr;
    }

    std::shared_ptr<google::protobuf::Message> ParseBody(EnCommand command, const void* data, int size)
    {
        switch (command)
        {
        case EnCommand::kSendText:
        case EnCommand::kSetDisplayName:
            return Parse<proto::StringMessage>(data, size);
        default:
            return nullptr;
        }
    }
}
