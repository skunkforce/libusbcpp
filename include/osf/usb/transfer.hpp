#pragma once
#include "libusb.h"
#include <functional>
#include "sum_type.hpp"
#include "descriptor.hpp"

namespace osf
{
namespace usb
{
class device_handle;
class transfer
{
    friend class device_handle;
    libusb_transfer *body = nullptr;
    std::function<void(transfer &)> cb;

    static void LIBUSB_CALL callback(libusb_transfer *tp)
    {
        transfer *self = static_cast<transfer *>(tp->user_data);
        self->cb(*self);
    }
    transfer(libusb_device_handle *dev, endpoint_address ep) : body{libusb_alloc_transfer(0)}
    {
        libusb_fill_bulk_transfer(body, dev, static_cast<unsigned char>(ep), nullptr, 0, &callback, static_cast<void *>(this), 0);
    }

public:
    void set_callback(std::function<void(transfer &)> f)
    {
        cb = std::move(f);
    }
    void set_buffer(unsigned char *begin, unsigned char *end)
    {
        body->buffer = begin;
        body->length = end - begin;
    }
    void set_timeout(std::chrono::milliseconds t)
    {
        body->timeout = t.count();
    }
};
transfer device_handle::async_bulk_transfer(endpoint_address ep)
{
    return transfer(dev, ep);
}
} // namespace usb
} // namespace osf