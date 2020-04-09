
#pragma once
#include <vector>
#include <variant>
#include <utility>
#include "libusb.h"
#include "libusbcpp/device.hpp"
#include "libusbcpp/descriptor.hpp"
#include "libusbcpp/transfer.hpp"
#include "error.hpp"
#include "bulk_in_pipe.hpp"

namespace osf
{
namespace libusbcpp
{
class bulk_transfer;
class context;

class device_list
{
    libusb_device **devs = nullptr;
    std::size_t length = 0;
    friend class context;
    device_list(libusb_device **d, std::size_t l) : devs{d}, length{l} {}

public:
    device_list(const device_list &) = delete;
    device_list &operator=(const device_list &) = delete;
    device_list(device_list &&other)
    {
        std::swap(devs, other.devs);
        std::swap(length, other.length);
    }
    device_list &operator=(device_list &&other)
    {
        length = 0;
        devs = nullptr;
        std::swap(devs, other.devs);
        std::swap(length, other.length);
    }
    ~device_list()
    {
        if (devs != nullptr)
        {
            libusb_free_device_list(devs, 1); //free the list, unref the devices in it
        }
    }
    device_list_iterator begin();
    device_list_iterator end();
};

device_list_iterator device_list::begin()
{
    return device_list_iterator{&devs[0]};
}
device_list_iterator device_list::end()
{
    return device_list_iterator{&devs[length]};
}

//handle to the libusb library
//this object is in a valid state only if it converts to true
class context
{
    libusb_context *ctx = nullptr; //a libusb session
public:
    context()
    {
        if (int r = libusb_init(&ctx); r < 0) //initialize the library for the session we just declared
        {
            ctx = nullptr;
        }
    }
    ~context()
    {
        if (ctx)
        {
            libusb_exit(ctx);
        }
    }

    //true if this object is in a fully formed state
    explicit operator bool()
    {
        return ctx != nullptr;
    }

    void set_verbosity(const int level)
    {
        libusb_set_debug(ctx, level);
    }

    device_list get_device_list()
    {
        libusb_device **devs;
        std::size_t length = libusb_get_device_list(ctx, &devs);
        return device_list{devs, length};
    }

    friend int handle_events(context &ctx)
    {
        return libusb_handle_events(ctx.ctx);
    }
};

template <typename T>
std::vector<device_handle> open_if(context &ctx, T pred)
{
    constexpr auto ignore_error = [](auto) {};
    std::vector<device_handle> out{};
    for (auto dev : ctx.get_device_list())
    {
        auto push_if_descriptor_matches = [&](auto desc) {
            if (pred(desc))
            {
                dev.open()(
                    [&](auto &od) {
                        out.emplace_back(std::move(od));
                    },
                    ignore_error);
            }
        };
        dev.get_device_descriptor()(
            push_if_descriptor_matches,
            ignore_error);
    }
    return out;
}

/*
//libusb descifies one kind of transfer, this library diferentiates
class bulk_transfer
{
    libusb_transfer *transfer = nullptr;

public:
    bulk_transfer(const device_handle &dev, endpoint ep, unsigned char *buffer, int length, libusb_transfer_cb_fn callback, void *user_data, unsigned int timeout)
    {
        transfer = libusb_alloc_transfer(0);
        libusb_fill_bulk_transfer(transfer, dev.dev, static_cast<unsigned char>(ep), buffer, length, callback, user_data, timeout);
    }
    bulk_transfer(const bulk_transfer &) = delete;
    bulk_transfer &operator=(const bulk_transfer &other) = delete;
    bulk_transfer(bulk_transfer &&other)
    {
        std::swap(transfer, other.transfer); //note that we are a constructor so our transfer pointer is nullptr
    }
    bulk_transfer &operator=(bulk_transfer &&other)
    {
        if (transfer != nullptr)
        {
            libusb_free_transfer(transfer);
        }
        std::swap(transfer, other.transfer);
    }
    ~bulk_transfer()
    {
        if (transfer != nullptr)
        {
            libusb_free_transfer(transfer);
        }
    }
    explicit operator bool()
    {
        return transfer != nullptr;
    }
    friend int submit_transfer(bulk_transfer &in)
    {
        return libusb_submit_transfer(in.transfer);
    }
};*/
} // namespace libusbcpp
} // namespace osf
