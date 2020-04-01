#pragma once
#include "libusb.h"
#include <vector>
#include <variant>
#include <utility>
#include "error.hpp"
#include "sum_type.hpp"

namespace osf
{
namespace usb
{

class endpoint_address
{
    unsigned char num;

public:
    explicit endpoint_address(unsigned char in) noexcept : num{in} {}
    explicit operator unsigned char()
    {
        return num;
    }
};
class device;
class endpoint_descriptor_iterator;
class interface_descriptor_range;
class interface_range;

//move only owning view of a config descripter
class config_descriptor
{
    friend class device;
    const libusb_config_descriptor *pcfg = nullptr;
    config_descriptor(const libusb_config_descriptor *p) : pcfg{p} {}
    void free() const noexcept
    {
        if (pcfg != nullptr)
        {
            libusb_free_config_descriptor(const_cast<libusb_config_descriptor *>(pcfg));
        }
    }

public:
    config_descriptor(const config_descriptor &) = delete;
    config_descriptor &operator=(const config_descriptor &) = delete;
    config_descriptor(config_descriptor &&other) noexcept
    {
        std::swap(pcfg, other.pcfg);
    }
    config_descriptor &operator=(config_descriptor &&other) noexcept
    {
        free();
        pcfg = other.pcfg;
        other.pcfg = nullptr;
    }
    ~config_descriptor()
    {
        free();
    }
    interface_range get_interfaces() const noexcept;
    //note that the pointer is only valid as long as the config_descriptor object lives
    const libusb_config_descriptor *get() const noexcept
    {
        return pcfg;
    }
};

class device_list_iterator
{
    friend class device_list;
    libusb_device **pdev;
    device_list_iterator(libusb_device **d) noexcept : pdev{d} {}

public:
    device_list_iterator &operator++() noexcept
    {
        ++pdev;
        return *this;
    }
    device_list_iterator operator++(int) noexcept
    {
        auto p = pdev++;
        return device_list_iterator{p};
    }
    device operator*() const noexcept;
    friend bool operator==(const device_list_iterator &lhs, const device_list_iterator &rhs) noexcept
    {
        return lhs.pdev == rhs.pdev;
    }
    friend bool operator!=(const device_list_iterator &lhs, const device_list_iterator &rhs) noexcept
    {
        return !(lhs == rhs);
    }
    friend bool operator<(const device_list_iterator &lhs, const device_list_iterator &rhs) noexcept
    {
        return lhs.pdev < rhs.pdev;
    }
    friend bool operator>(const device_list_iterator &lhs, const device_list_iterator &rhs) noexcept
    {
        return rhs < lhs;
    }
    friend bool operator>=(const device_list_iterator &lhs, const device_list_iterator &rhs) noexcept
    {
        return !(lhs < rhs);
    }
    friend bool operator<=(const device_list_iterator &lhs, const device_list_iterator &rhs) noexcept
    {
        return !(rhs < lhs);
    }
};

class endpoint_descriptor_iterator;

//non owning view of an endpoint descriptor
//note: this view is only valid for the lifetime of the object
//which it was obtained from
class endpoint_descriptor
{
    friend class endpoint_descriptor_iterator;
    const libusb_endpoint_descriptor *pdesc;
    endpoint_descriptor(const libusb_endpoint_descriptor *p) : pdesc{p} {}

public:
    const libusb_endpoint_descriptor *get() const noexcept
    {
        return pdesc;
    }
    endpoint_address get_ep_address() const noexcept
    {
        return endpoint_address(pdesc->bEndpointAddress);
    }
    bool is_bulk() const noexcept
    {
        return (pdesc->bmAttributes & LIBUSB_TRANSFER_TYPE_MASK) == LIBUSB_TRANSFER_TYPE_BULK;
    }
    bool is_control() const noexcept
    {
        return (pdesc->bmAttributes & LIBUSB_TRANSFER_TYPE_MASK) == LIBUSB_TRANSFER_TYPE_CONTROL;
    }
    bool is_isochronous() const noexcept
    {
        return (pdesc->bmAttributes & LIBUSB_TRANSFER_TYPE_MASK) == LIBUSB_TRANSFER_TYPE_ISOCHRONOUS;
    }
    bool is_interrupt() const noexcept
    {
        return (pdesc->bmAttributes & LIBUSB_TRANSFER_TYPE_MASK) == LIBUSB_TRANSFER_TYPE_INTERRUPT;
    }
    bool is_in() const noexcept
    {
        return pdesc->bEndpointAddress | LIBUSB_ENDPOINT_IN;
    }
    bool is_out() const noexcept
    {
        return !is_in();
    }
};

class endpoint_descriptor_iterator
{
    friend class interface_descriptor;
    const libusb_endpoint_descriptor *pdesc;
    endpoint_descriptor_iterator(const libusb_endpoint_descriptor *d) : pdesc{d} {}

public:
    endpoint_descriptor_iterator &operator++() noexcept
    {
        ++pdesc;
        return *this;
    }
    endpoint_descriptor_iterator operator++(int) noexcept
    {
        auto p = pdesc++;
        return endpoint_descriptor_iterator{p};
    }
    endpoint_descriptor operator*() noexcept
    {
        return endpoint_descriptor(pdesc);
    }
    friend bool operator==(const endpoint_descriptor_iterator &lhs, const endpoint_descriptor_iterator &rhs) noexcept
    {
        return lhs.pdesc == rhs.pdesc;
    }
    friend bool operator!=(const endpoint_descriptor_iterator &lhs, const endpoint_descriptor_iterator &rhs) noexcept
    {
        return !(lhs == rhs);
    }
    friend bool operator<(const endpoint_descriptor_iterator &lhs, const endpoint_descriptor_iterator &rhs) noexcept
    {
        return lhs.pdesc < rhs.pdesc;
    }
    friend bool operator>(const endpoint_descriptor_iterator &lhs, const endpoint_descriptor_iterator &rhs) noexcept
    {
        return rhs < lhs;
    }
    friend bool operator>=(const endpoint_descriptor_iterator &lhs, const endpoint_descriptor_iterator &rhs) noexcept
    {
        return !(lhs < rhs);
    }
    friend bool operator<=(const endpoint_descriptor_iterator &lhs, const endpoint_descriptor_iterator &rhs) noexcept
    {
        return !(rhs < lhs);
    }
};

class endpoint_descriptor_range
{
    endpoint_descriptor_iterator _begin;
    endpoint_descriptor_iterator _end;

public:
    endpoint_descriptor_range(endpoint_descriptor_iterator begin, endpoint_descriptor_iterator end) : _begin{begin}, _end{end} {}
    endpoint_descriptor_range(const endpoint_descriptor_range &other) : _begin{other._begin}, _end{other._end} {}
    endpoint_descriptor_range &operator=(const endpoint_descriptor_range &other)
    {
        _begin = other._begin;
        _end = other._end;
    }
    endpoint_descriptor_iterator begin() noexcept
    {
        return _begin;
    }
    endpoint_descriptor_iterator end() noexcept
    {
        return _end;
    }
};

class interface_iterator;
class config_descriptor;

//non owning view of an interface
//note: this view is only valid for the lifetime of the object
//which it was obtained from
class interface
{
    friend class interface_iterator;
    const libusb_interface *pdesc;
    interface(const libusb_interface *p) : pdesc{p} {}

public:
    interface(const interface &) = default;
    interface &operator=(const interface &) = default;
    interface_descriptor_range get_interface_descriptors() const noexcept;
};

class interface_iterator
{
    friend class config_descriptor;
    const libusb_interface *pdesc;
    interface_iterator(const libusb_interface *d) : pdesc{d} {}

public:
    interface_iterator &operator++() noexcept
    {
        ++pdesc;
        return *this;
    }
    interface_iterator operator++(int) noexcept
    {
        auto p = pdesc++;
        return interface_iterator{p};
    }
    interface operator*() const noexcept
    {
        return interface(pdesc);
    }
    friend bool operator==(const interface_iterator &lhs, const interface_iterator &rhs) noexcept
    {
        return lhs.pdesc == rhs.pdesc;
    }
    friend bool operator!=(const interface_iterator &lhs, const interface_iterator &rhs) noexcept
    {
        return !(lhs == rhs);
    }
    friend bool operator<(const interface_iterator &lhs, const interface_iterator &rhs) noexcept
    {
        return lhs.pdesc < rhs.pdesc;
    }
    friend bool operator>(const interface_iterator &lhs, const interface_iterator &rhs) noexcept
    {
        return rhs < lhs;
    }
    friend bool operator>=(const interface_iterator &lhs, const interface_iterator &rhs) noexcept
    {
        return !(lhs < rhs);
    }
    friend bool operator<=(const interface_iterator &lhs, const interface_iterator &rhs) noexcept
    {
        return !(rhs < lhs);
    }
};

class interface_range
{
    const interface_iterator _begin;
    const interface_iterator _end;

public:
    interface_range(const interface_iterator begin, const interface_iterator end) : _begin{begin}, _end{end} {}
    interface_iterator begin() const noexcept
    {
        return _begin;
    }
    interface_iterator end() const noexcept
    {
        return _end;
    }
};

class interface_descriptor_iterator;
//non owning view of an interface descriptor
//note this view is only valid as long as the object it is viewing is
class interface_descriptor
{
    friend class interface_descriptor_iterator;
    friend class configuration_descriptor;
    const libusb_interface_descriptor *pdesc;
    interface_descriptor(const libusb_interface_descriptor *p) : pdesc{p} {}

public:
    endpoint_descriptor_range get_endpoint_descriptors() const noexcept
    {
        return endpoint_descriptor_range(
            endpoint_descriptor_iterator{&pdesc->endpoint[0]},
            endpoint_descriptor_iterator{&pdesc->endpoint[pdesc->bNumEndpoints]});
    }
};

class interface_descriptor_iterator
{
    friend class interface;
    const libusb_interface_descriptor *pdesc;
    interface_descriptor_iterator(const libusb_interface_descriptor *d) : pdesc{d} {}

public:
    interface_descriptor_iterator &operator++() noexcept
    {
        ++pdesc;
        return *this;
    }
    interface_descriptor_iterator operator++(int) noexcept
    {
        auto p = pdesc++;
        return interface_descriptor_iterator{p};
    }
    interface_descriptor operator*() const noexcept
    {
        return interface_descriptor(pdesc);
    }
    friend bool operator==(const interface_descriptor_iterator &lhs, const interface_descriptor_iterator &rhs) noexcept
    {
        return lhs.pdesc == rhs.pdesc;
    }
    friend bool operator!=(const interface_descriptor_iterator &lhs, const interface_descriptor_iterator &rhs) noexcept
    {
        return !(lhs == rhs);
    }
    friend bool operator<(const interface_descriptor_iterator &lhs, const interface_descriptor_iterator &rhs) noexcept
    {
        return lhs.pdesc < rhs.pdesc;
    }
    friend bool operator>(const interface_descriptor_iterator &lhs, const interface_descriptor_iterator &rhs) noexcept
    {
        return rhs < lhs;
    }
    friend bool operator>=(const interface_descriptor_iterator &lhs, const interface_descriptor_iterator &rhs) noexcept
    {
        return !(lhs < rhs);
    }
    friend bool operator<=(const interface_descriptor_iterator &lhs, const interface_descriptor_iterator &rhs) noexcept
    {
        return !(rhs < lhs);
    }
};

class interface_descriptor_range
{
    const interface_descriptor_iterator _begin;
    const interface_descriptor_iterator _end;

public:
    interface_descriptor_range(const interface_descriptor_iterator begin, const interface_descriptor_iterator end) : _begin{begin}, _end{end} {}
    interface_descriptor_iterator begin() const noexcept
    {
        return _begin;
    }
    interface_descriptor_iterator end() const noexcept
    {
        return _end;
    }
};

interface_range config_descriptor::get_interfaces() const noexcept
{
    return interface_range(
        interface_iterator{&(pcfg->interface[0])},
        interface_iterator{&(pcfg->interface[pcfg->bNumInterfaces])});
}
interface_descriptor_range interface::get_interface_descriptors() const noexcept
{
    return interface_descriptor_range(
        interface_descriptor_iterator{&(pdesc->altsetting[0])},
        interface_descriptor_iterator{&(pdesc->altsetting[pdesc->num_altsetting])});
}

} // namespace usb
} // namespace osf