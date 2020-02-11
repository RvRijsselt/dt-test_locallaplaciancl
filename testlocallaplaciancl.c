#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef __APPLE__ 
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#include "locallaplaciancl.h"

#define CL_ERR_TO_STR(err) case err: return #err
char const* clGetErrorString(cl_int const err)
{
  switch(err)
  {
    CL_ERR_TO_STR(CL_SUCCESS);
    CL_ERR_TO_STR(CL_DEVICE_NOT_FOUND);
    CL_ERR_TO_STR(CL_DEVICE_NOT_AVAILABLE);
    CL_ERR_TO_STR(CL_COMPILER_NOT_AVAILABLE);
    CL_ERR_TO_STR(CL_MEM_OBJECT_ALLOCATION_FAILURE);
    CL_ERR_TO_STR(CL_OUT_OF_RESOURCES);
    CL_ERR_TO_STR(CL_OUT_OF_HOST_MEMORY);
    CL_ERR_TO_STR(CL_PROFILING_INFO_NOT_AVAILABLE);
    CL_ERR_TO_STR(CL_MEM_COPY_OVERLAP);
    CL_ERR_TO_STR(CL_IMAGE_FORMAT_MISMATCH);
    CL_ERR_TO_STR(CL_IMAGE_FORMAT_NOT_SUPPORTED);
    CL_ERR_TO_STR(CL_BUILD_PROGRAM_FAILURE);
    CL_ERR_TO_STR(CL_MAP_FAILURE);
    CL_ERR_TO_STR(CL_MISALIGNED_SUB_BUFFER_OFFSET);
    CL_ERR_TO_STR(CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST);
    CL_ERR_TO_STR(CL_COMPILE_PROGRAM_FAILURE);
    CL_ERR_TO_STR(CL_LINKER_NOT_AVAILABLE);
    CL_ERR_TO_STR(CL_LINK_PROGRAM_FAILURE);
    CL_ERR_TO_STR(CL_DEVICE_PARTITION_FAILED);
    CL_ERR_TO_STR(CL_KERNEL_ARG_INFO_NOT_AVAILABLE);
    CL_ERR_TO_STR(CL_INVALID_VALUE);
    CL_ERR_TO_STR(CL_INVALID_DEVICE_TYPE);
    CL_ERR_TO_STR(CL_INVALID_PLATFORM);
    CL_ERR_TO_STR(CL_INVALID_DEVICE);
    CL_ERR_TO_STR(CL_INVALID_CONTEXT);
    CL_ERR_TO_STR(CL_INVALID_QUEUE_PROPERTIES);
    CL_ERR_TO_STR(CL_INVALID_COMMAND_QUEUE);
    CL_ERR_TO_STR(CL_INVALID_HOST_PTR);
    CL_ERR_TO_STR(CL_INVALID_MEM_OBJECT);
    CL_ERR_TO_STR(CL_INVALID_IMAGE_FORMAT_DESCRIPTOR);
    CL_ERR_TO_STR(CL_INVALID_IMAGE_SIZE);
    CL_ERR_TO_STR(CL_INVALID_SAMPLER);
    CL_ERR_TO_STR(CL_INVALID_BINARY);
    CL_ERR_TO_STR(CL_INVALID_BUILD_OPTIONS);
    CL_ERR_TO_STR(CL_INVALID_PROGRAM);
    CL_ERR_TO_STR(CL_INVALID_PROGRAM_EXECUTABLE);
    CL_ERR_TO_STR(CL_INVALID_KERNEL_NAME);
    CL_ERR_TO_STR(CL_INVALID_KERNEL_DEFINITION);
    CL_ERR_TO_STR(CL_INVALID_KERNEL);
    CL_ERR_TO_STR(CL_INVALID_ARG_INDEX);
    CL_ERR_TO_STR(CL_INVALID_ARG_VALUE);
    CL_ERR_TO_STR(CL_INVALID_ARG_SIZE);
    CL_ERR_TO_STR(CL_INVALID_KERNEL_ARGS);
    CL_ERR_TO_STR(CL_INVALID_WORK_DIMENSION);
    CL_ERR_TO_STR(CL_INVALID_WORK_GROUP_SIZE);
    CL_ERR_TO_STR(CL_INVALID_WORK_ITEM_SIZE);
    CL_ERR_TO_STR(CL_INVALID_GLOBAL_OFFSET);
    CL_ERR_TO_STR(CL_INVALID_EVENT_WAIT_LIST);
    CL_ERR_TO_STR(CL_INVALID_EVENT);
    CL_ERR_TO_STR(CL_INVALID_OPERATION);
    CL_ERR_TO_STR(CL_INVALID_GL_OBJECT);
    CL_ERR_TO_STR(CL_INVALID_BUFFER_SIZE);
    CL_ERR_TO_STR(CL_INVALID_MIP_LEVEL);
    CL_ERR_TO_STR(CL_INVALID_GLOBAL_WORK_SIZE);
    CL_ERR_TO_STR(CL_INVALID_PROPERTY);
    CL_ERR_TO_STR(CL_INVALID_IMAGE_DESCRIPTOR);
    CL_ERR_TO_STR(CL_INVALID_COMPILER_OPTIONS);
    CL_ERR_TO_STR(CL_INVALID_LINKER_OPTIONS);
    CL_ERR_TO_STR(CL_INVALID_DEVICE_PARTITION_COUNT);
    //CL_ERR_TO_STR(CL_INVALID_PIPE_SIZE);
    //CL_ERR_TO_STR(CL_INVALID_DEVICE_QUEUE);

  default:
    return "UNKNOWN ERROR CODE";
  }
}
#undef CL_ERR_TO_STR


int read_file(unsigned char **output, size_t *size, const char *name) 
{
  FILE* fp = fopen(name, "rb");
  if (!fp) {
    return -1;
  }

  fseek(fp, 0, SEEK_END);
  *size = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  *output = (unsigned char *)malloc(*size);
  if (!*output) {
    fclose(fp);
    return -1;
  }

  fread(*output, *size, 1, fp);
  fclose(fp);
  return 0;
}

void print_device_info(cl_device_id device)
{
  size_t valueSize;
  char* value;

  // print device name
  clGetDeviceInfo(device, CL_DEVICE_NAME, 0, NULL, &valueSize);
  value = (char*) malloc(valueSize);
  clGetDeviceInfo(device, CL_DEVICE_NAME, valueSize, value, NULL);
  printf("  Device: %s\n", value);
  free(value);

  // print hardware device version
  clGetDeviceInfo(device, CL_DEVICE_VERSION, 0, NULL, &valueSize);
  value = (char*) malloc(valueSize);
  clGetDeviceInfo(device, CL_DEVICE_VERSION, valueSize, value, NULL);
  printf("  Hardware version: %s\n", value);
  free(value);

  // print software driver version
  clGetDeviceInfo(device, CL_DRIVER_VERSION, 0, NULL, &valueSize);
  value = (char*) malloc(valueSize);
  clGetDeviceInfo(device, CL_DRIVER_VERSION, valueSize, value, NULL);
  printf("  Software version: %s\n", value);
  free(value);

  // print c version supported by compiler for device
  clGetDeviceInfo(device, CL_DEVICE_OPENCL_C_VERSION, 0, NULL, &valueSize);
  value = (char*) malloc(valueSize);
  clGetDeviceInfo(device, CL_DEVICE_OPENCL_C_VERSION, valueSize, value, NULL);
  printf("  OpenCL C version: %s\n", value);
  free(value);
}

void run_kernel(int width, int height, int bpp, float* data, const char *options)
{
  cl_int err;
  cl_platform_id platform;
  cl_device_id device;

  // Query platforms and devices
  err = clGetPlatformIDs(1, &platform, NULL);
  if (err != 0) { fprintf(stderr, "Error clGetPlatformIDs: %s\n", clGetErrorString(err)); exit(1); }

  err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 1, &device, NULL);
  if (err != 0) { fprintf(stderr, "Error clGetDeviceIDs: %s\n", clGetErrorString(err)); exit(1); }

  print_device_info(device);

  // Create context
  const cl_context_properties prop[] = {CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0};
  cl_context ctx = clCreateContext(prop, 1, &device, NULL, NULL, &err);
  if (err != 0) { fprintf(stderr, "Error clCreateContext: %s\n", clGetErrorString(err)); exit(1); }

  // Create program
  unsigned char* program_file = NULL;
  size_t program_size = 0;
  read_file(&program_file, &program_size, "locallaplacian.cl");

  cl_program program = clCreateProgramWithSource(ctx, 1, (const char **)&program_file, &program_size, &err);
  if (err != 0) { fprintf(stderr, "Error clCreateProgramWithSource: %s\n", clGetErrorString(err)); exit(1); }

  printf("  Build options: %s\n", options);
  err = clBuildProgram(program, 1, &device, options, NULL, NULL);
  if (err != 0) { fprintf(stderr, "Error clBuildProgram: %s\n", clGetErrorString(err)); exit(1); }

  free(program_file);

  // Create command queue
  cl_command_queue queue = clCreateCommandQueue(ctx, device, 0, &err);
  if (err != 0) { fprintf(stderr, "Error clCreateCommandQueue: %s\n", clGetErrorString(err)); exit(1); }

  // Allocate in/out buffers
  cl_mem dev_in = dt_opencl_alloc_device(ctx, width, height, bpp);
  cl_mem dev_out = dt_opencl_alloc_device(ctx, width, height, bpp);

  // Upload data to device
  err = dt_opencl_write_host_to_device(queue, data, dev_in, width, height, bpp);
  if (err != 0) { fprintf(stderr, "Error dt_opencl_write_host_to_device: %s\n", clGetErrorString(err)); exit(1); }

  // Init
  dt_local_laplacian_cl_global_t* kernels = dt_local_laplacian_init_cl_global(program);
  dt_local_laplacian_cl_t* llplc = dt_local_laplacian_init_cl(kernels, ctx, queue, width, height, 0.5, 0.5, 0.5, 0.5);

  // Run 
  err = dt_local_laplacian_cl(llplc, dev_in, dev_out);
  if (err != 0) { fprintf(stderr, "Error dt_local_laplacian_cl: %s\n", clGetErrorString(err)); exit(1); }

  // Cleanup 
  dt_local_laplacian_free_cl(llplc);
  
  // Get data from device
  err = dt_opencl_copy_device_to_host(queue, data, dev_out, width, height, bpp);
  if (err != 0) { fprintf(stderr, "Error dt_opencl_copy_device_to_host: %s\n", clGetErrorString(err)); exit(1); }

  err = clFinish(queue);
  if (err != 0) { fprintf(stderr, "Error clFinish: %s\n", clGetErrorString(err)); exit(1); }

  // Release the resources
  clReleaseMemObject(dev_out);
  clReleaseMemObject(dev_in);
  clReleaseCommandQueue(queue);
  clReleaseProgram(program);
  clReleaseContext(ctx);
  clReleaseDevice(device);
}

// Expected result based on AMDGPU-Pro and checked with Intel GPUs
float expectedResult[] = {
    -4.23, -2.96, -1.72, -0.49, 00.73, 01.93, 03.12, 04.32, 05.54, 06.82,
    08.94, 10.17, 11.34, 12.48, 13.59, 14.68, 15.74, 16.79, 17.85, 18.96, 
    20.11, 21.23, 22.33, 23.43, 24.53, 25.65, 26.75, 27.82, 28.90, 30.01, 
    30.39, 31.49, 32.54, 33.57, 34.58, 35.59, 36.59, 37.60, 38.63, 39.74, 
    39.91, 41.08, 42.22, 43.31, 44.37, 45.40, 46.40, 47.39, 48.41, 49.47, 
    49.59, 50.68, 51.72, 52.74, 53.74, 54.74, 55.74, 56.76, 57.84, 58.99, 
    59.32, 60.47, 61.54, 62.56, 63.55, 64.53, 65.49, 66.45, 67.45, 68.52, 
    69.20, 70.30, 71.37, 72.41, 73.45, 74.50, 75.56, 76.60, 77.66, 78.76, 
    79.90, 81.02, 82.09, 83.12, 84.12, 85.13, 86.14, 87.18, 88.26, 89.41, 
    91.97, 93.23, 94.46, 95.67, 96.86, 98.02, 99.18, 100.33, 101.51, 102.73
};

// Not entirely correct comparison but it does the job
int floatcmp(float a, float b)
{
    return (int)(a * 100 + .5) == (int)(b * 100 + .5);
}

int main(int argc, char *argv[]) 
{
  cl_int i, x, y;
  int width = 10;
  int height = 10;
  int bpp = sizeof(float);
  const char* options = "";

  printf("Darktable local laplacian test\n");

  if (argc > 1) 
  {
    options = argv[1];
  }

  // Prepare some input data
  const size_t buf_size = width * height * bpp;
  float *data = (float *)malloc(buf_size);
  for (y = 0; y < height; ++y) 
  {
    for (x = 0; x < width; ++x) 
    {
      i = y * width + x;
      data[i] = i;
    }
  }

  // Run the kernel
  run_kernel(width, height, bpp, data, options);

  // Print the result
  printf("Output data:\n");
  int result = 0;
  for (y = 0; y < height; ++y) 
  {
    for (x = 0; x < width; ++x) 
    {
      i = y * width + x;
      if (!floatcmp(data[i], expectedResult[i]))
      {
          result++;
          printf("x");
      }
      printf("%05.2f  ", data[i]);
    }
    printf("\n");
  }

  printf("Different values: %d\n", result); 
  printf("Result: %s\n", (result > 0 ? "failed" : "passed")); 
  return result;
}
