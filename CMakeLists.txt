cmake_minimum_required(VERSION 3.12)
project(osf-libusbcpp VERSION 0.0.0)

set(detail_header_files
)
set(header_files
${CMAKE_CURRENT_SOURCE_DIR}/include/osf/libusbcpp.hpp
${CMAKE_CURRENT_SOURCE_DIR}/include/osf/libusbcpp/descriptor.hpp
${CMAKE_CURRENT_SOURCE_DIR}/include/osf/libusbcpp/device.hpp
${CMAKE_CURRENT_SOURCE_DIR}/include/osf/libusbcpp/error.hpp
${CMAKE_CURRENT_SOURCE_DIR}/include/osf/libusbcpp/transfer.hpp
${CMAKE_CURRENT_SOURCE_DIR}/include/osf/libusbcpp/bulk_in_pipe.hpp
)

include("cmake/osf-cmake-helpers.cmake")
osf_generate_header_only_cmake(osf-libusbcpp osf 1 0 "osf-tmp;osf-sum-type")
