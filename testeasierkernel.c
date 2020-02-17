#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef __APPLE__ 
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#include "locallaplaciancl.h"
#include "single-data-input.h"
#include "single-expected.h"

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

#define ROUNDUP(a, n) ((a) % (n) == 0 ? (a) : ((a) / (n)+1) * (n))
#define ROUNDUPWD(a) dt_opencl_roundup(a)
#define ROUNDUPHT(a) dt_opencl_roundup(a)
#define MIN(a, b) ((a) > (b) ? (b) : (a))

// downsample width/height to given level
static inline uint64_t dl(uint64_t size, const int level)
{
  for(int l=0;l<level;l++)
    size = (size-1)/2+1;
  return size;
}


int image_count_inequal(float* a, float* b, int len)
{
  int res = 0;
  float epsilon = 0.0001;
  printf("  Expected == Actual\n");
  for (int i=0; i < len; ++i)
  {
    printf("  %f == %f", a[i], b[i]);
    if (fabs(a[i] - b[i]) > epsilon) {
      res++;
      printf(" x \n");
    }
    else
    {
      printf("\n");
    }
    
  }
  return res;
}

int run_kernel(int width, int height, int bpp, float* data, const char *options)
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
  dt_local_laplacian_cl_global_t* kernels = dt_local_laplacian_init_cl_global(program);
  dt_local_laplacian_cl_t* b = dt_local_laplacian_init_cl(kernels, ctx, queue, width, height, 0.5, 0.5, 0.5, 0.5);
  cl_kernel kernel_laplacian_assemble = clCreateKernel(program, "easier_kernel", &err);
  if (err != 0) { fprintf(stderr, "Error clCreateKernel: %s\n", clGetErrorString(err)); exit(1); }

  // Upload data to device
  // Create some easier to deal with data
  for (int i = 0; i < 16*16; ++i)
  {
    input_dev_padded_l1[i] = (float)i / (16*16+1);
    input_dev_output_l2[i] = (float)i / (16*16+1);
    expected_dev_output_l1[i] = (float)i / (16*16+1);

    input_dev_processed_k0l1[i] = 0.01;
    input_dev_processed_k0l2[i] = 0.02;
    input_dev_processed_k1l1[i] = 0.11;
    input_dev_processed_k1l2[i] = 0.12;
    input_dev_processed_k2l1[i] = 0.21;
    input_dev_processed_k2l2[i] = 0.22;
    input_dev_processed_k3l1[i] = 0.31;
    input_dev_processed_k3l2[i] = 0.32;
    input_dev_processed_k4l1[i] = 0.41;
    input_dev_processed_k4l2[i] = 0.42;
    input_dev_processed_k5l1[i] = 0.51;
    input_dev_processed_k5l2[i] = 0.52;
  }

  err |= dt_opencl_write_host_to_device(queue, input_dev_padded_l1, b->dev_padded[1], 16, 16, bpp);
  err |= dt_opencl_write_host_to_device(queue, input_dev_processed_k0l1, b->dev_processed[0][1], 16, 16, bpp);
  err |= dt_opencl_write_host_to_device(queue, input_dev_processed_k0l2, b->dev_processed[0][2], 16, 16, bpp);
  err |= dt_opencl_write_host_to_device(queue, input_dev_processed_k1l1, b->dev_processed[1][1], 16, 16, bpp);
  err |= dt_opencl_write_host_to_device(queue, input_dev_processed_k1l2, b->dev_processed[1][2], 16, 16, bpp);
  err |= dt_opencl_write_host_to_device(queue, input_dev_processed_k2l1, b->dev_processed[2][1], 16, 16, bpp);
  err |= dt_opencl_write_host_to_device(queue, input_dev_processed_k2l2, b->dev_processed[2][2], 16, 16, bpp);
  err |= dt_opencl_write_host_to_device(queue, input_dev_processed_k3l1, b->dev_processed[3][1], 16, 16, bpp);
  err |= dt_opencl_write_host_to_device(queue, input_dev_processed_k3l2, b->dev_processed[3][2], 16, 16, bpp);
  err |= dt_opencl_write_host_to_device(queue, input_dev_processed_k4l1, b->dev_processed[4][1], 16, 16, bpp);
  err |= dt_opencl_write_host_to_device(queue, input_dev_processed_k4l2, b->dev_processed[4][2], 16, 16, bpp);
  err |= dt_opencl_write_host_to_device(queue, input_dev_processed_k5l1, b->dev_processed[5][1], 16, 16, bpp);
  err |= dt_opencl_write_host_to_device(queue, input_dev_processed_k5l2, b->dev_processed[5][2], 16, 16, bpp);
  err |= dt_opencl_write_host_to_device(queue, input_dev_output_l2, b->dev_output[2], 16, 16, bpp);   // this is actually used as input
  if (err != 0) { fprintf(stderr, "Error dt_opencl_write_host_to_device: %s\n", clGetErrorString(err)); exit(1); }

  err = clFinish(queue);

  // Run 
  // assemble output pyramid coarse to fine
  //for(int l=b->num_levels-2;l >= 0; l--)
  {
    int l = 1;  // only coarse
    const int pw = dl(b->bwidth,l), ph = dl(b->bheight,l);
    size_t sizes[] = { ROUNDUPWD(pw), ROUNDUPHT(ph), 1 };
    printf("level %d, wxh: %dx%d \n", l, pw, ph);
    // this is so dumb: // not my comment
    dt_opencl_set_kernel_arg(kernel_laplacian_assemble,  0, sizeof(cl_mem), (void *)&b->dev_padded[l]);
    dt_opencl_set_kernel_arg(kernel_laplacian_assemble,  1, sizeof(cl_mem), (void *)&b->dev_output[l+1]);
    dt_opencl_set_kernel_arg(kernel_laplacian_assemble,  2, sizeof(cl_mem), (void *)&b->dev_output[l]);
    dt_opencl_set_kernel_arg(kernel_laplacian_assemble,  3, sizeof(cl_mem), (void *)&b->dev_processed[0][l]);
    dt_opencl_set_kernel_arg(kernel_laplacian_assemble,  4, sizeof(cl_mem), (void *)&b->dev_processed[0][l+1]);
    dt_opencl_set_kernel_arg(kernel_laplacian_assemble,  5, sizeof(cl_mem), (void *)&b->dev_processed[1][l]);
    dt_opencl_set_kernel_arg(kernel_laplacian_assemble,  6, sizeof(cl_mem), (void *)&b->dev_processed[1][l+1]);
    dt_opencl_set_kernel_arg(kernel_laplacian_assemble,  7, sizeof(cl_mem), (void *)&b->dev_processed[2][l]);
    dt_opencl_set_kernel_arg(kernel_laplacian_assemble,  8, sizeof(cl_mem), (void *)&b->dev_processed[2][l+1]);
    dt_opencl_set_kernel_arg(kernel_laplacian_assemble,  9, sizeof(cl_mem), (void *)&b->dev_processed[3][l]);
    dt_opencl_set_kernel_arg(kernel_laplacian_assemble, 10, sizeof(cl_mem), (void *)&b->dev_processed[3][l+1]);
    dt_opencl_set_kernel_arg(kernel_laplacian_assemble, 11, sizeof(cl_mem), (void *)&b->dev_processed[4][l]);
    dt_opencl_set_kernel_arg(kernel_laplacian_assemble, 12, sizeof(cl_mem), (void *)&b->dev_processed[4][l+1]);
    dt_opencl_set_kernel_arg(kernel_laplacian_assemble, 13, sizeof(cl_mem), (void *)&b->dev_processed[5][l]);
    dt_opencl_set_kernel_arg(kernel_laplacian_assemble, 14, sizeof(cl_mem), (void *)&b->dev_processed[5][l+1]);
    dt_opencl_set_kernel_arg(kernel_laplacian_assemble, 15, sizeof(int), (void *)&pw);
    dt_opencl_set_kernel_arg(kernel_laplacian_assemble, 16, sizeof(int), (void *)&ph);
    err = dt_opencl_enqueue_kernel_2d(b->queue, kernel_laplacian_assemble, sizes);
    if (err != CL_SUCCESS) { fprintf(stderr, "Error dt_opencl_enqueue_kernel_2d: %s\n", clGetErrorString(err)); exit(1); }
  }

  err = clFinish(queue);
  if (err != CL_SUCCESS) { fprintf(stderr, "Error clFinish: %s\n", clGetErrorString(err)); exit(1); }

  // Get data from device
  int result = 0;
  {
    int l = 1;  // Only the modified one
    printf("level %d \n", l);
    const int wd = ROUNDUPWD(dl(b->bwidth, l));
    const int ht = ROUNDUPHT(dl(b->bheight, l));
    float *buf = dt_alloc_align(16, wd * ht * sizeof(float));

    err = dt_opencl_copy_device_to_host(b->queue, buf, b->dev_output[l], wd, ht, sizeof(float));  
    if (err != CL_SUCCESS) { fprintf(stderr, "Error dt_opencl_copy_device_to_host: %s\n", clGetErrorString(err)); exit(1); }

    // Print image data
    int diff = image_count_inequal(expected_dev_output_l1, buf, wd * ht);
    if (diff > 0)
    {
      printf("Output level %d is different\n", l);
      result += diff;
    }
  }

  // Cleanup 
  err = clFinish(queue);
  dt_local_laplacian_free_cl(b);
  
  err = clFinish(queue);
  if (err != 0) { fprintf(stderr, "Error clFinish: %s\n", clGetErrorString(err)); exit(1); }

  // Release the resources
  clReleaseCommandQueue(queue);
  clReleaseProgram(program);
  clReleaseContext(ctx);
  clReleaseDevice(device);

  return result;
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

  // Run the kernel
  int result = run_kernel(width, height, bpp, NULL, options);

  printf("Different values: %d\n", result); 
  printf("Result: %s\n", (result > 0 ? "failed" : "passed")); 
  return result;
}
