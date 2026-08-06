/* Copyright (c) 2024 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */
#include "cl_error_lookup.hpp"

namespace clerror
{

ErrorInfo get_error_info(int code)
{
  ErrorInfo error_info;

  switch (code)
  {
    // clang-format off

    case 0:
      {
	error_info.code = code;
	error_info.flag = "CL_SUCCESS";
	error_info.description = "The sweet spot.";
	error_info.type = "Run-time and JIT Compiler Errors (driver-dependent)";
	}
	break;

    case -1:
      {
	error_info.code = code;
	error_info.flag = "CL_DEVICE_NOT_FOUND";
	error_info.description = "if no OpenCL devices that matched device_type were found.";
	error_info.type = "Run-time and JIT Compiler Errors (driver-dependent)";
	}
	break;

    case -2:
      {
	error_info.code = code;
	error_info.flag = "CL_DEVICE_NOT_AVAILABLE";
	error_info.description = "if a device in devices is currently not available even though the device was returned by clGetDeviceIDs.";
	error_info.type = "Run-time and JIT Compiler Errors (driver-dependent)";
	}
	break;

    case -3:
      {
	error_info.code = code;
	error_info.flag = "CL_COMPILER_NOT _AVAILABLE";
	error_info.description = "if program is created with clCreateProgramWithSource and a compiler is not available i.e. CL_DEVICE_COMPILER_AVAILABLE specified in the table of OpenCL Device Queries for clGetDeviceInfo is set to CL_FALSE.";
	error_info.type = "Run-time and JIT Compiler Errors (driver-dependent)";
	}
	break;

    case -4:
      {
	error_info.code = code;
	error_info.flag = "CL_MEM_OBJECT _ALLOCATION_FAILURE";
	error_info.description = "if there is a failure to allocate memory for buffer object.";
	error_info.type = "Run-time and JIT Compiler Errors (driver-dependent)";
	}
	break;

    case -5:
      {
	error_info.code = code;
	error_info.flag = "CL_OUT_OF_RESOURCES";
	error_info.description = "if there is a failure to allocate resources required by the OpenCL implementation on the device.";
	error_info.type = "Run-time and JIT Compiler Errors (driver-dependent)";
	}
	break;

    case -6:
      {
	error_info.code = code;
	error_info.flag = "CL_OUT_OF_HOST_MEMORY";
	error_info.description = "if there is a failure to allocate resources required by the OpenCL implementation on the host.";
	error_info.type = "Run-time and JIT Compiler Errors (driver-dependent)";
	}
	break;

    case -7:
      {
	error_info.code = code;
	error_info.flag = "CL_PROFILING_INFO_NOT _AVAILABLE";
	error_info.description = "if the CL_QUEUE_PROFILING_ENABLE flag is not set for the command-queue, if the execution status of the command identified by event is not CL_COMPLETE or if event is a user event object.";
	error_info.type = "Run-time and JIT Compiler Errors (driver-dependent)";
	}
	break;

    case -8:
      {
	error_info.code = code;
	error_info.flag = "CL_MEM_COPY_OVERLAP";
	error_info.description = "if src_buffer and dst_buffer are the same buffer or subbuffer object and the source and destination regions overlap or if src_buffer and dst_buffer are different sub-buffers of the same associated buffer object and they overlap. The regions overlap if src_offset ≤ to dst_offset ≤ to src_offset + size – 1, or if dst_offset ≤ to src_offset ≤ to dst_offset + size – 1.";
	error_info.type = "Run-time and JIT Compiler Errors (driver-dependent)";
	}
	break;

    case -9:
      {
	error_info.code = code;
	error_info.flag = "CL_IMAGE_FORMAT _MISMATCH";
	error_info.description = "if src_image and dst_image do not use the same image format.";
	error_info.type = "Run-time and JIT Compiler Errors (driver-dependent)";
	}
	break;

    case -10:
      {
	error_info.code = code;
	error_info.flag = "CL_IMAGE_FORMAT_NOT _SUPPORTED";
	error_info.description = "if the image_format is not supported.";
	error_info.type = "Run-time and JIT Compiler Errors (driver-dependent)";
	}
	break;

    case -11:
      {
	error_info.code = code;
	error_info.flag = "CL_BUILD_PROGRAM _FAILURE";
	error_info.description = "if there is a failure to build the program executable. This error will be returned if clBuildProgram does not return until the build has completed.";
	error_info.type = "Run-time and JIT Compiler Errors (driver-dependent)";
	}
	break;

    case -12:
      {
	error_info.code = code;
	error_info.flag = "CL_MAP_FAILURE";
	error_info.description = " if there is a failure to map the requested region into the host address space. This error cannot occur for image objects created with CL_MEM_USE_HOST_PTR or CL_MEM_ALLOC_HOST_PTR.";
	error_info.type = "Run-time and JIT Compiler Errors (driver-dependent)";
	}
	break;

    case -13:
      {
	error_info.code = code;
	error_info.flag = "CL_MISALIGNED_SUB _BUFFER_OFFSET";
	error_info.description = "if a sub-buffer object is specified as the value for an argument that is a buffer object and the offset specified when the sub-buffer object is created is not aligned to CL_DEVICE_MEM_BASE_ADDR_ALIGN value for device associated with queue.";
	error_info.type = "Run-time and JIT Compiler Errors (driver-dependent)";
	}
	break;

    case -14:
      {
	error_info.code = code;
	error_info.flag = "CL_EXEC_STATUS_ERROR_ FOR_EVENTS_IN_WAIT_LIST";
	error_info.description = "if the execution status of any of the events in event_list is a negative integer value.";
	error_info.type = "Run-time and JIT Compiler Errors (driver-dependent)";
	}
	break;

    case -15:
      {
	error_info.code = code;
	error_info.flag = "CL_COMPILE_PROGRAM _FAILURE";
	error_info.description = "if there is a failure to compile the program source. This error will be returned if clCompileProgram does not return until the compile has completed.";
	error_info.type = "Run-time and JIT Compiler Errors (driver-dependent)";
	}
	break;

    case -16:
      {
	error_info.code = code;
	error_info.flag = "CL_LINKER_NOT_AVAILABLE";
	error_info.description = "if a linker is not available i.e. CL_DEVICE_LINKER_AVAILABLE specified in the table of allowed values for param_name for clGetDeviceInfo is set to CL_FALSE.";
	error_info.type = "Run-time and JIT Compiler Errors (driver-dependent)";
	}
	break;

    case -17:
      {
	error_info.code = code;
	error_info.flag = "CL_LINK_PROGRAM_FAILURE";
	error_info.description = "if there is a failure to link the compiled binaries and/or libraries.";
	error_info.type = "Run-time and JIT Compiler Errors (driver-dependent)";
	}
	break;

    case -18:
      {
	error_info.code = code;
	error_info.flag = "CL_DEVICE_PARTITION _FAILED";
	error_info.description = " if the partition name is supported by the implementation but in_device could not be further partitioned.";
	error_info.type = "Run-time and JIT Compiler Errors (driver-dependent)";
	}
	break;

    case -19:
      {
	error_info.code = code;
	error_info.flag = "CL_KERNEL_ARG_INFO _NOT_AVAILABLE";
	error_info.description = "if the argument information is not available for kernel.";
	error_info.type = "Run-time and JIT Compiler Errors (driver-dependent)";
	}
	break;

    case -30:
      {
	error_info.code = code;
	error_info.flag = "CL_INVALID_VALUE";
	error_info.description = "This depends on the function: two or more coupled parameters had errors.";
	error_info.type = "Compile-time Errors (driver-independent)";
	}
	break;

    case -31:
      {
	error_info.code = code;
	error_info.flag = "CL_INVALID_DEVICE_TYPE";
	error_info.description = "if an invalid device_type is given";
	error_info.type = "Compile-time Errors (driver-independent)";
	}
	break;

    case -32:
      {
	error_info.code = code;
	error_info.flag = "CL_INVALID_PLATFORM";
	error_info.description = "if an invalid platform was given";
	error_info.type = "Compile-time Errors (driver-independent)";
	}
	break;

    case -33:
      {
	error_info.code = code;
	error_info.flag = "CL_INVALID_DEVICE";
	error_info.description = "if devices contains an invalid device or are not associated with the specified platform.";
	error_info.type = "Compile-time Errors (driver-independent)";
	}
	break;

    case -34:
      {
	error_info.code = code;
	error_info.flag = "CL_INVALID_CONTEXT";
	error_info.description = "if context is not a valid context.";
	error_info.type = "Compile-time Errors (driver-independent)";
	}
	break;

    case -35:
      {
	error_info.code = code;
	error_info.flag = "CL_INVALID_QUEUE_PROPERTIES";
	error_info.description = "if specified command-queue-properties are valid but are not supported by the device.";
	error_info.type = "Compile-time Errors (driver-independent)";
	}
	break;

    case -36:
      {
	error_info.code = code;
	error_info.flag = "CL_INVALID_COMMAND_QUEUE";
	error_info.description = "if command_queue is not a valid command-queue.";
	error_info.type = "Compile-time Errors (driver-independent)";
	}
	break;

    case -37:
      {
	error_info.code = code;
	error_info.flag = "CL_INVALID_HOST_PTR";
	error_info.description = "This flag is valid only if host_ptr is not NULL. If specified, it indicates that the application wants the OpenCL implementation to allocate memory for the memory object and copy the data from memory referenced by host_ptr.CL_MEM_COPY_HOST_PTR and CL_MEM_USE_HOST_PTR are mutually exclusive.CL_MEM_COPY_HOST_PTR can be used with CL_MEM_ALLOC_HOST_PTR to initialize the contents of the cl_mem object allocated using host-accessible (e.g. PCIe) memory.";
	error_info.type = "Compile-time Errors (driver-independent)";
	}
	break;

    case -38:
      {
	error_info.code = code;
	error_info.flag = "CL_INVALID_MEM_OBJECT";
	error_info.description = "if memobj is not a valid OpenCL memory object.";
	error_info.type = "Compile-time Errors (driver-independent)";
	}
	break;

    case -39:
      {
	error_info.code = code;
	error_info.flag = "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
	error_info.description = "if the OpenGL/DirectX texture internal format does not map to a supported OpenCL image format.";
	error_info.type = "Compile-time Errors (driver-independent)";
	}
	break;

    case -40:
      {
	error_info.code = code;
	error_info.flag = "CL_INVALID_IMAGE_SIZE";
	error_info.description = "if an image object is specified as an argument value and the image dimensions (image width, height, specified or compute row and/or slice pitch) are not supported by device associated with queue.";
	error_info.type = "Compile-time Errors (driver-independent)";
	}
	break;

    case -41:
      {
	error_info.code = code;
	error_info.flag = "CL_INVALID_SAMPLER";
	error_info.description = "if sampler is not a valid sampler object.";
	error_info.type = "Compile-time Errors (driver-independent)";
	}
	break;

    case -42:
      {
	error_info.code = code;
	error_info.flag = "CL_INVALID_BINARY";
	error_info.description = "if program is created with clCreateProgramWithBinary and devices listed in device_list do not have a valid program binary loaded. The provided binary is unfit for the selected device.";
	error_info.type = "Compile-time Errors (driver-independent)";
	}
	break;

    case -43:
      {
	error_info.code = code;
	error_info.flag = "CL_INVALID_BUILD_OPTIONS";
	error_info.description = "if the build options specified by options are invalid.";
	error_info.type = "Compile-time Errors (driver-independent)";
	}
	break;

    case -44:
      {
	error_info.code = code;
	error_info.flag = "CL_INVALID_PROGRAM";
	error_info.description = "if program is a not a valid program object.";
	error_info.type = "Compile-time Errors (driver-independent)";
	}
	break;

    case -45:
      {
	error_info.code = code;
	error_info.flag = "CL_INVALID_PROGRAM_EXECUTABLE";
	error_info.description = "if there is no successfully built program executable available for device associated with command_queue.";
	error_info.type = "Compile-time Errors (driver-independent)";
	}
	break;

    case -46:
      {
	error_info.code = code;
	error_info.flag = "CL_INVALID_KERNEL_NAME";
	error_info.description = "if kernel_name is not found in program.";
	error_info.type = "Compile-time Errors (driver-independent)";
	}
	break;

    case -47:
      {
	error_info.code = code;
	error_info.flag = "CL_INVALID_KERNEL_DEFINITION";
	error_info.description = "if the function definition for __kernel function given by kernel_name such as the number of arguments, the argument types are not the same for all devices for which the program executable has been built.";
	error_info.type = "Compile-time Errors (driver-independent)";
	}
	break;

    case -48:
      {
	error_info.code = code;
	error_info.flag = "CL_INVALID_KERNEL";
	error_info.description = "if kernel is not a valid kernel object.";
	error_info.type = "Compile-time Errors (driver-independent)";
	}
	break;

    case -49:
      {
	error_info.code = code;
	error_info.flag = "CL_INVALID_ARG_INDEX";
	error_info.description = "if arg_index is not a valid argument index.";
	error_info.type = "Compile-time Errors (driver-independent)";
	}
	break;

    case -50:
      {
	error_info.code = code;
	error_info.flag = "CL_INVALID_ARG_VALUE";
	error_info.description = "if arg_value specified is not a valid value.";
	error_info.type = "Compile-time Errors (driver-independent)";
	}
	break;

    case -51:
      {
	error_info.code = code;
	error_info.flag = "CL_INVALID_ARG_SIZE";
	error_info.description = "if arg_size does not match the size of the data type for an argument that is not a memory object or if the argument is a memory object and arg_size != sizeof(cl_mem) or if arg_size is zero and the argument is declared with the __local qualifier or if the argument is a sampler and arg_size != sizeof(cl_sampler).";
	error_info.type = "Compile-time Errors (driver-independent)";
	}
	break;

    case -52:
      {
	error_info.code = code;
	error_info.flag = "CL_INVALID_KERNEL_ARGS";
	error_info.description = "if the kernel argument values have not been specified.";
	error_info.type = "Compile-time Errors (driver-independent)";
	}
	break;

    case -53:
      {
	error_info.code = code;
	error_info.flag = "CL_INVALID_WORK_DIMENSION";
	error_info.description = "if work_dim is not a valid value (i.e. a value between 1 and 3).";
	error_info.type = "Compile-time Errors (driver-independent)";
	}
	break;

    case -54:
      {
	error_info.code = code;
	error_info.flag = "CL_INVALID_WORK_GROUP_SIZE";
	error_info.description = "if local_work_size is specified and number of work-items specified by global_work_size is not evenly divisable by size of work-group given by local_work_size or does not match the work-group size specified for kernel using the __attribute__ ((reqd_work_group_size(X, Y, Z))) qualifier in program source.if local_work_size is specified and the total number of work-items in the work-group computed as local_work_size[0] *… local_work_size[work_dim – 1] is greater than the value specified by CL_DEVICE_MAX_WORK_GROUP_SIZE in the table of OpenCL Device Queries for clGetDeviceInfo.if local_work_size is NULL and the __attribute__ ((reqd_work_group_size(X, Y, Z))) qualifier is used to declare the work-group size for kernel in the program source.";
	error_info.type = "Compile-time Errors (driver-independent)";
	}
	break;

    case -55:
      {
	error_info.code = code;
	error_info.flag = "CL_INVALID_WORK_ITEM_SIZE";
	error_info.description = "if the number of work-items specified in any of local_work_size[0], … local_work_size[work_dim – 1] is greater than the corresponding values specified by CL_DEVICE_MAX_WORK_ITEM_SIZES[0], …. CL_DEVICE_MAX_WORK_ITEM_SIZES[work_dim – 1].";
	error_info.type = "Compile-time Errors (driver-independent)";
	}
	break;

    case -56:
      {
	error_info.code = code;
	error_info.flag = "CL_INVALID_GLOBAL_OFFSET";
	error_info.description = "if the value specified in global_work_size + the corresponding values in global_work_offset for any dimensions is greater than the sizeof(size_t) for the device on which the kernel execution will be enqueued.";
	error_info.type = "Compile-time Errors (driver-independent)";
	}
	break;

    case -57:
      {
	error_info.code = code;
	error_info.flag = "CL_INVALID_EVENT_WAIT_LIST";
	error_info.description = "if event_wait_list is NULL and num_events_in_wait_list > 0, or event_wait_list is not NULL and num_events_in_wait_list is 0, or if event objects in event_wait_list are not valid events.";
	error_info.type = "Compile-time Errors (driver-independent)";
	}
	break;

    case -58:
      {
	error_info.code = code;
	error_info.flag = "CL_INVALID_EVENT";
	error_info.description = "if event objects specified in event_list are not valid event objects.";
	error_info.type = "Compile-time Errors (driver-independent)";
	}
	break;

    case -59:
      {
	error_info.code = code;
	error_info.flag = "CL_INVALID_OPERATION";
	error_info.description = "if interoperability is specified by setting CL_CONTEXT_ADAPTER_D3D9_KHR, CL_CONTEXT_ADAPTER_D3D9EX_KHR or CL_CONTEXT_ADAPTER_DXVA_KHR to a non-NULL value, and interoperability with another graphics API is also specified. (only if the cl_khr_dx9_media_sharing extension is supported).";
	error_info.type = "Compile-time Errors (driver-independent)";
	}
	break;

    case -60:
      {
	error_info.code = code;
	error_info.flag = "CL_INVALID_GL_OBJECT";
	error_info.description = "if texture is not a GL texture object whose type matches texture_target, if the specified miplevel of texture is not defined, or if the width or height of the specified miplevel is zero.";
	error_info.type = "Compile-time Errors (driver-independent)";
	}
	break;

    case -61:
      {
	error_info.code = code;
	error_info.flag = "CL_INVALID_BUFFER_SIZE";
	error_info.description = "if size is 0.Implementations may return CL_INVALID_BUFFER_SIZE if size is greater than the CL_DEVICE_MAX_MEM_ALLOC_SIZE value specified in the table of allowed values for param_name for clGetDeviceInfo for all devices in context.";
	error_info.type = "Compile-time Errors (driver-independent)";
	}
	break;

    case -62:
      {
	error_info.code = code;
	error_info.flag = "CL_INVALID_MIP_LEVEL";
	error_info.description = "if miplevel is greater than zero and the OpenGL implementation does not support creating from non-zero mipmap levels.";
	error_info.type = "Compile-time Errors (driver-independent)";
	}
	break;

    case -63:
      {
	error_info.code = code;
	error_info.flag = "CL_INVALID_GLOBAL_WORK_SIZE";
	error_info.description = "if global_work_size is NULL, or if any of the values specified in global_work_size[0], …global_work_size [work_dim – 1] are 0 or exceed the range given by the sizeof(size_t) for the device on which the kernel execution will be enqueued.";
	error_info.type = "Compile-time Errors (driver-independent)";
	}
	break;

    case -64:
      {
	error_info.code = code;
	error_info.flag = "CL_INVALID_PROPERTY";
	error_info.description = "Vague error, depends on the function";
	error_info.type = "Compile-time Errors (driver-independent)";
	}
	break;

    case -65:
      {
	error_info.code = code;
	error_info.flag = "CL_INVALID_IMAGE_DESCRIPTOR";
	error_info.description = "if values specified in image_desc are not valid or if image_desc is NULL.";
	error_info.type = "Compile-time Errors (driver-independent)";
	}
	break;

    case -66:
      {
	error_info.code = code;
	error_info.flag = "CL_INVALID_COMPILER_OPTIONS";
	error_info.description = "if the compiler options specified by options are invalid.";
	error_info.type = "Compile-time Errors (driver-independent)";
	}
	break;

    case -67:
      {
	error_info.code = code;
	error_info.flag = "CL_INVALID_LINKER_OPTIONS";
	error_info.description = "if the linker options specified by options are invalid.";
	error_info.type = "Compile-time Errors (driver-independent)";
	}
	break;

    case -68:
      {
	error_info.code = code;
	error_info.flag = "CL_INVALID_DEVICE_PARTITION_COUNT";
	error_info.description = "if the partition name specified in properties is CL_DEVICE_PARTITION_BY_COUNTS and the number of sub-devices requested exceeds CL_DEVICE_PARTITION_MAX_SUB_DEVICES or the total number of compute units requested exceeds CL_DEVICE_PARTITION_MAX_COMPUTE_UNITS for in_device, or the number of compute units requested for one or more sub-devices is less than zero or the number of sub-devices requested exceeds CL_DEVICE_PARTITION_MAX_COMPUTE_UNITS for in_device.";
	error_info.type = "Compile-time Errors (driver-independent)";
	}
	break;

    case -69:
      {
	error_info.code = code;
	error_info.flag = "CL_INVALID_PIPE_SIZE";
	error_info.description = "if pipe_packet_size is 0 or the pipe_packet_size exceeds CL_DEVICE_PIPE_MAX_PACKET_SIZE value for all devices in context or if pipe_max_packets is 0.";
	error_info.type = "Compile-time Errors (driver-independent)";
	}
	break;

    case -70:
      {
	error_info.code = code;
	error_info.flag = "CL_INVALID_DEVICE_QUEUE";
	error_info.description = "when an argument is of type queue_t when it’s not a valid device queue object.";
	error_info.type = "Compile-time Errors (driver-independent)";
	}
	break;

    case -1000:
      {
	error_info.code = code;
	error_info.flag = "CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR";
	error_info.description = "CL and GL not on the same device (only when using a GPU).";
	error_info.type = "Errors thrown by extensions";
	}
	break;

    case -1001:
      {
	error_info.code = code;
	error_info.flag = " CL_PLATFORM_NOT_FOUND_KHR";
	error_info.description = "No valid ICDs found";
	error_info.type = "Errors thrown by extensions";
	}
	break;

    case -1002:
      {
	error_info.code = code;
	error_info.flag = "CL_INVALID_D3D10_DEVICE_KHR";
	error_info.description = "if the Direct3D 10 device specified for interoperability is not compatible with the devices against which the context is to be created.";
	error_info.type = "Errors thrown by extensions";
	}
	break;

    case -1003:
      {
	error_info.code = code;
	error_info.flag = "CL_INVALID_D3D10_RESOURCE_KHR";
	error_info.description = "If the resource is not a Direct3D 10 buffer or texture object";
	error_info.type = "Errors thrown by extensions";
	}
	break;

    case -1004:
      {
	error_info.code = code;
	error_info.flag = "CL_D3D10_RESOURCE_ALREADY_ACQUIRED_KHR";
	error_info.description = "If a mem_object is already acquired by OpenCL";
	error_info.type = "Errors thrown by extensions";
	}
	break;

    case -1005:
      {
	error_info.code = code;
	error_info.flag = "CL_D3D10_RESOURCE_NOT_ACQUIRED_KHR";
	error_info.description = "If a mem_object is not acquired by OpenCL";
	error_info.type = "Errors thrown by extensions";
	}
	break;

    case -1006:
      {
	error_info.code = code;
	error_info.flag = "CL_INVALID_D3D11_DEVICE_KHR";
	error_info.description = "if the Direct3D 11 device specified for interoperability is not compatible with the devices against which the context is to be created.";
	error_info.type = "Errors thrown by extensions";
	}
	break;

    case -1007:
      {
	error_info.code = code;
	error_info.flag = "CL_INVALID_D3D11_RESOURCE_KHR";
	error_info.description = "If the resource is not a Direct3D 11 buffer or texture object";
	error_info.type = "Errors thrown by extensions";
	}
	break;

    case -1008:
      {
	error_info.code = code;
	error_info.flag = "CL_D3D11_RESOURCE_ALREADY_ACQUIRED_KHR";
	error_info.description = "If a mem_object is already acquired by OpenCL";
	error_info.type = "Errors thrown by extensions";
	}
	break;

    case -1009:
      {
	error_info.code = code;
	error_info.flag = "CL_D3D11_RESOURCE_NOT_ACQUIRED_KHR";
	error_info.description = "If a mem_object’ is not acquired by OpenCL";
	error_info.type = "Errors thrown by extensions";
	}
	break;

    case -1010:
      {
	error_info.code = code;
	error_info.flag = "CL_INVALID_D3D9_DEVICE_NV CL_INVALID_DX9_DEVICE_INTEL";
	error_info.description = "If the Direct3D 9 device specified for interoperability is not compatible with the devices against which the context is to be created";
	error_info.type = "Errors thrown by extensions";
	}
	break;

    case -1011:
      {
	error_info.code = code;
	error_info.flag = "CL_INVALID_D3D9_RESOURCE_NV CL_INVALID_DX9_RESOURCE_INTEL";
	error_info.description = "If a mem_object’ is not a Direct3D 9 resource of the required type";
	error_info.type = "Errors thrown by extensions";
	}
	break;

    case -1012:
      {
	error_info.code = code;
	error_info.flag = "CL_D3D9_RESOURCE_ALREADY_ACQUIRED_NV CL_DX9_RESOURCE_ALREADY_ACQUIRED_INTEL";
	error_info.description = "If any of the mem_objects’ is currently already acquired by OpenCL";
	error_info.type = "Errors thrown by extensions";
	}
	break;

    case -1013:
      {
	error_info.code = code;
	error_info.flag = "CL_D3D9_RESOURCE_NOT_ACQUIRED_NV CL_DX9_RESOURCE_NOT_ACQUIRED_INTEL";
	error_info.description = "If any of the mem_objects’ is currently not acquired by OpenCL";
	error_info.type = "Errors thrown by extensions";
	}
	break;

    case -1092:
      {
	error_info.code = code;
	error_info.flag = "CL_EGL_RESOURCE_NOT_ACQUIRED_KHR";
	error_info.description = "If a mem_object’ is not acquired by OpenCL";
	error_info.type = "Errors thrown by extensions";
	}
	break;

    case -1093:
      {
	error_info.code = code;
	error_info.flag = "CL_INVALID_EGL_OBJECT_KHR";
	error_info.description = "If a mem_object’ is not a EGL resource of the required type";
	error_info.type = "Errors thrown by extensions";
	}
	break;

    case -1094:
      {
	error_info.code = code;
	error_info.flag = "CL_INVALID_ACCELERATOR_INTEL";
	error_info.description = "when arg_value’ is not a valid accelerator object, and by clRetainAccelerator, clReleaseAccelerator, and clGetAcceleratorInfo when accelerator’ is not a valid accelerator object";
	error_info.type = "Errors thrown by extensions";
	}
	break;

    case -1095:
      {
	error_info.code = code;
	error_info.flag = "CL_INVALID_ACCELERATOR_TYPE_INTEL";
	error_info.description = "when arg_value’ is not an accelerator object of the correct type, or when accelerator_type’ is not a valid accelerator type";
	error_info.type = "Errors thrown by extensions";
	}
	break;

    case -1096:
      {
	error_info.code = code;
	error_info.flag = "CL_INVALID_ACCELERATOR_DESCRIPTOR_INTEL";
	error_info.description = "when values described by descriptor’ are not valid, or if a combination of values is not valid";
	error_info.type = "Errors thrown by extensions";
	}
	break;

    case -1097:
      {
	error_info.code = code;
	error_info.flag = "CL_ACCELERATOR_TYPE_NOT_SUPPORTED_INTEL";
	error_info.description = "when accelerator_type’ is a valid accelerator type, but it not supported by any device in context’";
	error_info.type = "Errors thrown by extensions";
	}
	break;

    case -1098:
      {
	error_info.code = code;
	error_info.flag = "CL_INVALID_VA_API_MEDIA_ADAPTER_INTEL";
	error_info.description = "If the VA API display specified for interoperability is not compatible with the devices against which the context is to be created";
	error_info.type = "Errors thrown by extensions";
	}
	break;

    case -1099:
      {
	error_info.code = code;
	error_info.flag = "CL_INVALID_VA_API_MEDIA_SURFACE_INTEL";
	error_info.description = "If surface’ is not a VA API surface of the required type, by clGetMemObjectInfo when param_name’ is CL_MEM_VA_API_MEDIA_SURFACE_INTEL when was not created from a VA API surface, and from clGetImageInfo when param_name’ is CL_IMAGE_VA_API_PLANE_INTEL and image’ was not created from a VA API surface";
	error_info.type = "Errors thrown by extensions";
	}
	break;

    case -1100:
      {
	error_info.code = code;
	error_info.flag = "CL_VA_API_MEDIA_SURFACE_ALREADY_ACQUIRED_INTEL";
	error_info.description = "If any of the mem_objects’ is already acquired by OpenCL";
	error_info.type = "Errors thrown by extensions";
	}
	break;

    case -1101:
      {
	error_info.code = code;
	error_info.flag = "CL_VA_API_MEDIA_SURFACE_NOT_ACQUIRED_INTEL";
	error_info.description = "If any of the mem_objects’ are not currently acquired by OpenCL";
	error_info.type = "Errors thrown by extensions";
	}
	break;

    case -9999:
      {
	error_info.code = code;
	error_info.flag = "NVidia";
	error_info.description = "Illegal read or write to a buffer";
	error_info.type = "Errors thrown by Vendors";
	}
	break;

    // clang-format on
  default: break;
  }

  return error_info;
}

} // namespace clerror
