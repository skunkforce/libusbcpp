cmake_minimum_required(VERSION 3.12)
project(osf-usb VERSION 0.0.0)

set(detail_header_files
)
set(header_files
${CMAKE_CURRENT_SOURCE_DIR}/include/osf/usb.hpp
${CMAKE_CURRENT_SOURCE_DIR}/include/osf/usb/descriptor.hpp
${CMAKE_CURRENT_SOURCE_DIR}/include/osf/usb/device.hpp
${CMAKE_CURRENT_SOURCE_DIR}/include/osf/usb/error.hpp
${CMAKE_CURRENT_SOURCE_DIR}/include/osf/usb/transfer.hpp
${CMAKE_CURRENT_SOURCE_DIR}/include/osf/usb/bulk_in_pipe.hpp
)

include("cmake/osf-cmake-helpers.cmake")
osf_generate_header_only_cmake(osf-usb osf 1 0 "osf-tmp;osf-sum-type")
