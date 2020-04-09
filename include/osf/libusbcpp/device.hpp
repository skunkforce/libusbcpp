
#pragma once
#include <vector>
#include <variant>
#include <utility>
#include <chrono>
#include "libusb.h"
#include "error.hpp"
#include "sum_type.hpp"
#include "descriptor.hpp"

namespace osf
{
namespace libusbcpp
{
class context;
class device;
class config_descriptor;
class transfer;
//this corresponds to a libusb_device_handle
class device_handle
{
    friend class context;
    friend class device;
    libusb_device_handle *dev = nullptr;
    std::vector<int> claimed_interfaces{};
    device_handle(libusb_device_handle *d) : dev{d} {}

public:
    device_handle(const device_handle &) = delete;
    device_handle(device_handle &&rhs)
    {
        dev = rhs.dev;
        std::swap(claimed_interfaces, rhs.claimed_interfaces);
        rhs.dev = nullptr;
    }
    ~device_handle()
    {
        if (dev != nullptr)
        {
            for (auto num : claimed_interfaces)
            {
                libusb_release_interface(dev, num);
            }
            libusb_close(dev);
        }
    }
    explicit operator bool()
    {
        return dev != nullptr;
    }
    int claim(const int interface_number)
    {
        return libusb_claim_interface(dev, interface_number);
    }
    int release(const int interface_number)
    {
        return libusb_release_interface(dev, interface_number);
    }
    device get_device();
    sum_type<config_descriptor, error> get_active_config_descriptor();

    //c++ style interface for libusb_bulk_transfer
    //this function will try to fill the entire input range
    //check the distance between the input begin iterator
    //and the returned iterator to get the length which was
    //actually read by the function
    sum_type<unsigned char *, error> bulk_transfer(endpoint_address ep, unsigned char *begin, unsigned char *end, std::chrono::milliseconds timeout) noexcept
    {
        int actual_len = 0;
        if (int r = libusb_bulk_transfer(dev, static_cast<unsigned char>(ep), begin, end - begin, &actual_len, timeout.count()); r == 0)
        {
            return begin + actual_len; //advance iterator upon success
        }
        else
        {
            return error(r);
        }
    }

    transfer async_bulk_transfer(endpoint_address ep);
};

//this corresponds to a libusb_device
class device
{
    friend class device_list_iterator;
    friend class device_handle;
    libusb_device *pdev = nullptr;
    device(libusb_device *p) : pdev{p}
    {
        libusb_ref_device(pdev); //bump up ref count
    }

public:
    device(const device &other) : pdev{other.pdev}
    {
        libusb_ref_device(pdev);
    }
    device(device &&other) : pdev{other.pdev}
    {
        other.pdev = nullptr; //set other to null because we essentially took its ref count
    }
    device &operator=(const device &other)
    {
        if (pdev != nullptr)
        {
            libusb_unref_device(pdev);
        }
        pdev = other.pdev;
        libusb_ref_device(pdev);
    }
    device &operator=(device &&other)
    {
        if (pdev != nullptr)
        {
            libusb_unref_device(pdev);
        }
        pdev = other.pdev;
        other.pdev = nullptr;
    }
    ~device()
    {
        //nullptr should only be possible in a moved from object,
        //but we still need to make sure and only free valid pointers
        if (pdev != nullptr)
        {
            libusb_unref_device(pdev);
        }
    }
    sum_type<device_handle, error> open() const
    {
        libusb_device_handle *dev;
        if (int err = libusb_open(pdev, &dev); err == 0)
        {
            return device_handle{dev};
        }
        else
        {
            return error(err);
        }
    }
    sum_type<libusb_device_descriptor, error> get_device_descriptor() const
    {
        libusb_device_descriptor desc;                                 //partially formed
        if (int r = libusb_get_device_descriptor(pdev, &desc); r == 0) //well formed on success
        {
            return desc;
        }
        else
        {
            return error(r);
        }
    }
    sum_type<config_descriptor, error> get_active_config_descriptor() const
    {
        libusb_config_descriptor *cfg;
        if (int r = libusb_get_active_config_descriptor(pdev, &cfg); r == 0)
        {
            return config_descriptor(cfg);
        }
        else
        {
            return error(r);
        }
    }
};

device device_list_iterator::operator*() const noexcept
{
    return device{*pdev};
}

device device_handle::get_device()
{
    return device{libusb_get_device(dev)};
}

sum_type<config_descriptor, error> device_handle::get_active_config_descriptor()
{
    return get_device().get_active_config_descriptor();
}

} // namespace libusbcpp
} // namespace osf
