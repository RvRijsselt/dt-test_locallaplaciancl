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
  cl_kernel kernel_laplacian_assemble = clCreateKernel(program, "laplacian_assemble", &err);
  if (err != 0) { fprintf(stderr, "Error clCreateKernel: %s\n", clGetErrorString(err)); exit(1); }

  // Upload data to device
  err |= dt_opencl_write_host_to_device(queue, input_dev_padded_l0, b->dev_padded[0], 32, 32, bpp);
  err |= dt_opencl_write_host_to_device(queue, input_dev_padded_l1, b->dev_padded[1], 16, 16, bpp);
  //err |= dt_opencl_write_host_to_device(queue, input_dev_padded_l2, b->dev_padded[2], 16, 16, bpp);   // is uninitialized and not used
  err |= dt_opencl_write_host_to_device(queue, input_dev_processed_k0l0, b->dev_processed[0][0], 32, 32, bpp);
  err |= dt_opencl_write_host_to_device(queue, input_dev_processed_k0l1, b->dev_processed[0][1], 16, 16, bpp);
  err |= dt_opencl_write_host_to_device(queue, input_dev_processed_k0l2, b->dev_processed[0][2], 16, 16, bpp);
  err |= dt_opencl_write_host_to_device(queue, input_dev_processed_k1l0, b->dev_processed[1][0], 32, 32, bpp);
  err |= dt_opencl_write_host_to_device(queue, input_dev_processed_k1l1, b->dev_processed[1][1], 16, 16, bpp);
  err |= dt_opencl_write_host_to_device(queue, input_dev_processed_k1l2, b->dev_processed[1][2], 16, 16, bpp);
  err |= dt_opencl_write_host_to_device(queue, input_dev_processed_k2l0, b->dev_processed[2][0], 32, 32, bpp);
  err |= dt_opencl_write_host_to_device(queue, input_dev_processed_k2l1, b->dev_processed[2][1], 16, 16, bpp);
  err |= dt_opencl_write_host_to_device(queue, input_dev_processed_k2l2, b->dev_processed[2][2], 16, 16, bpp);
  //err |= dt_opencl_write_host_to_device(queue, input_dev_output_l0, b->dev_output[0], 32, 32, bpp);     // is uninitialized, write used only
  //err |= dt_opencl_write_host_to_device(queue, input_dev_output_l1, b->dev_output[1], 16, 16, bpp);     // is uninitialized, write used only
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
    printf("level %d \n", l);
    // this is so dumb: // not my comment
    dt_opencl_set_kernel_arg(b->global->kernel_laplacian_assemble,  0, sizeof(cl_mem), (void *)&b->dev_padded[l]);
    dt_opencl_set_kernel_arg(b->global->kernel_laplacian_assemble,  1, sizeof(cl_mem), (void *)&b->dev_output[l+1]);
    dt_opencl_set_kernel_arg(b->global->kernel_laplacian_assemble,  2, sizeof(cl_mem), (void *)&b->dev_output[l]);
    dt_opencl_set_kernel_arg(b->global->kernel_laplacian_assemble,  3, sizeof(cl_mem), (void *)&b->dev_processed[0][l]);
    dt_opencl_set_kernel_arg(b->global->kernel_laplacian_assemble,  4, sizeof(cl_mem), (void *)&b->dev_processed[0][l+1]);
    dt_opencl_set_kernel_arg(b->global->kernel_laplacian_assemble,  5, sizeof(cl_mem), (void *)&b->dev_processed[1][l]);
    dt_opencl_set_kernel_arg(b->global->kernel_laplacian_assemble,  6, sizeof(cl_mem), (void *)&b->dev_processed[1][l+1]);
    dt_opencl_set_kernel_arg(b->global->kernel_laplacian_assemble,  7, sizeof(cl_mem), (void *)&b->dev_processed[2][l]);
    dt_opencl_set_kernel_arg(b->global->kernel_laplacian_assemble,  8, sizeof(cl_mem), (void *)&b->dev_processed[2][l+1]);
    dt_opencl_set_kernel_arg(b->global->kernel_laplacian_assemble,  9, sizeof(cl_mem), (void *)&b->dev_processed[3][l]);
    dt_opencl_set_kernel_arg(b->global->kernel_laplacian_assemble, 10, sizeof(cl_mem), (void *)&b->dev_processed[3][l+1]);
    dt_opencl_set_kernel_arg(b->global->kernel_laplacian_assemble, 11, sizeof(cl_mem), (void *)&b->dev_processed[4][l]);
    dt_opencl_set_kernel_arg(b->global->kernel_laplacian_assemble, 12, sizeof(cl_mem), (void *)&b->dev_processed[4][l+1]);
    dt_opencl_set_kernel_arg(b->global->kernel_laplacian_assemble, 13, sizeof(cl_mem), (void *)&b->dev_processed[5][l]);
    dt_opencl_set_kernel_arg(b->global->kernel_laplacian_assemble, 14, sizeof(cl_mem), (void *)&b->dev_processed[5][l+1]);
    // dt_opencl_set_kernel_arg(b->global->kernel_laplacian_assemble, 15, sizeof(cl_mem), (void *)&b->dev_processed[6][l]);
    // dt_opencl_set_kernel_arg(b->global->kernel_laplacian_assemble, 16, sizeof(cl_mem), (void *)&b->dev_processed[6][l+1]);
    // dt_opencl_set_kernel_arg(b->global->kernel_laplacian_assemble, 17, sizeof(cl_mem), (void *)&b->dev_processed[7][l]);
    // dt_opencl_set_kernel_arg(b->global->kernel_laplacian_assemble, 18, sizeof(cl_mem), (void *)&b->dev_processed[7][l+1]);
    dt_opencl_set_kernel_arg(b->global->kernel_laplacian_assemble, 15, sizeof(int), (void *)&pw);
    dt_opencl_set_kernel_arg(b->global->kernel_laplacian_assemble, 16, sizeof(int), (void *)&ph);
    err = dt_opencl_enqueue_kernel_2d(b->queue, b->global->kernel_laplacian_assemble, sizes);
    if (err != CL_SUCCESS) { fprintf(stderr, "Error dt_opencl_enqueue_kernel_2d: %s\n", clGetErrorString(err)); exit(1); }
  }

  err = clFinish(queue);

  // Get data from device
  int result = 0;
  //for (int l=0; l < b->num_levels; l++)
  {
    int l = 1;  // Only the modified one
    printf("level %d \n", l);
    const int wd = ROUNDUPWD(dl(b->bwidth, l));
    const int ht = ROUNDUPHT(dl(b->bheight, l));
    float *buf = dt_alloc_align(16, wd * ht * sizeof(float));

    err = dt_opencl_copy_device_to_host(b->queue, buf, b->dev_output[l], wd, ht, sizeof(float));  
    if (err != CL_SUCCESS) { fprintf(stderr, "Error dt_opencl_copy_device_to_host: %s\n", clGetErrorString(err)); exit(1); }

    FILE *txt = fopen("single-actual.h", "w");
    fprintf(txt, "float actual_dev_output_l%d[%d*%d] = { \n", l, wd, ht);

    char filename[256] = { 0 };
    snprintf(filename, sizeof(filename), "out/post_actual_output_%d.pfm", l);
    {
      FILE *f = fopen(filename, "wb");
      fprintf(f, "Pf\n%d %d\n-1.0\n", wd, ht);
      for(int j=0;j<ht;j++)
        for(int i=0;i<wd;i++) {
            fwrite(buf + wd*j+i, 1, sizeof(float), f);
            fprintf(txt, "%a, ", buf[wd*j+i]);
        }
      fprintf(txt, "};\n");
      fclose(f);
    }
    fprintf(txt, "\n\n");
    fclose(txt);

    float actual_dev_output_l1_with_O0[16*16] = { -0x1.1af24cp-5, -0x1.1af24cp-5, -0x1.f511bp-6, -0x1.77b9ap-7, 0x1.8975ep-7, 0x1.247e9cp-5, 0x1.e810ap-5, 0x1.227628p-4, 0x1.1ef63cp-4, 0x1.1c3a38p-4, 0x1.1bd24ep-4, 0x1.1bafa8p-4, 0x1.1bafacp-4, 0x1.1bafa8p-4, 0x1.1bafa8p-4, 0x1.1bafa8p-4, -0x1.1af24cp-5, -0x1.1af24cp-5, -0x1.f511bp-6, -0x1.77b9ap-7, 0x1.8975ep-7, 0x1.247e9cp-5, 0x1.e810ap-5, 0x1.227628p-4, 0x1.1ef63cp-4, 0x1.1c3a38p-4, 0x1.1bd24ep-4, 0x1.1bafa8p-4, 0x1.1bafacp-4, 0x1.1bafa8p-4, 0x1.1bafa8p-4, 0x1.1bafa8p-4, 0x1.229f3p-6, 0x1.229f3p-6, 0x1.68225p-6, 0x1.574cp-5, 0x1.0f1cdp-4, 0x1.6bce36p-4, 0x1.c2b684p-4, 0x1.ecc36cp-4, 0x1.ea8bfp-4, 0x1.e8745cp-4, 0x1.e8246p-4, 0x1.e809b6p-4, 0x1.e809bap-4, 0x1.e809b6p-4, 0x1.e809b6p-4, 0x1.e809b6p-4, 0x1.a7c5ccp-3, 0x1.a7c5ccp-3, 0x1.af5ffp-3, 0x1.d1dbbp-3, 0x1.fd1a8p-3, 0x1.143d68p-2, 0x1.28a4ccp-2, 0x1.32cfa8p-2, 0x1.32f376p-2, 0x1.32bcb2p-2, 0x1.32b44cp-2, 0x1.32b18p-2, 0x1.32b17ep-2, 0x1.32b18p-2, 0x1.32b18p-2, 0x1.32b18p-2, 0x1.9b3726p-2, 0x1.9b3726p-2, 0x1.9f2c06p-2, 0x1.b1f76ep-2, 0x1.cc964ep-2, 0x1.e4b5e2p-2, 0x1.f92beap-2, 0x1.01bfb2p-1, 0x1.030cb6p-1, 0x1.037a14p-1, 0x1.038a82p-1, 0x1.038ffcp-1, 0x1.038ffep-1, 0x1.038ffcp-1, 0x1.038ffcp-1, 0x1.038ffcp-1, 0x1.2c0f5ap-1, 0x1.2c0f5ap-1, 0x1.2e0ffcp-1, 0x1.3411eap-1, 0x1.3c51eap-1, 0x1.450cccp-1, 0x1.4cab84p-1, 0x1.51970cp-1, 0x1.5451eep-1, 0x1.555ebcp-1, 0x1.55870ep-1, 0x1.55948p-1, 0x1.55948p-1, 0x1.55948p-1, 0x1.55948p-1, 0x1.55948p-1, 0x1.78428cp-1, 0x1.78428cp-1, 0x1.7a433p-1, 0x1.80451ap-1, 0x1.88851cp-1, 0x1.914p-1, 0x1.98debap-1, 0x1.9dca4p-1, 0x1.a0852p-1, 0x1.a191fp-1, 0x1.a1ba44p-1, 0x1.a1c7b6p-1, 0x1.a1c7b4p-1, 0x1.a1c7b6p-1, 0x1.a1c7b6p-1, 0x1.a1c7b6p-1, 0x1.a975cp-1, 0x1.a975cp-1, 0x1.ab7664p-1, 0x1.b1784ep-1, 0x1.b9b85p-1, 0x1.c27332p-1, 0x1.ca11ecp-1, 0x1.cefd74p-1, 0x1.d1b856p-1, 0x1.d2c522p-1, 0x1.d2ed78p-1, 0x1.d2fae6p-1, 0x1.d2fae8p-1, 0x1.d2fae6p-1, 0x1.d2fae6p-1, 0x1.d2fae6p-1, 0x1.c4c28ep-1, 0x1.c4c28ep-1, 0x1.c6c332p-1, 0x1.ccc51ep-1, 0x1.d5051ep-1, 0x1.ddcp-1, 0x1.e55ebap-1, 0x1.ea4a4p-1, 0x1.ed0522p-1, 0x1.ee11fp-1, 0x1.ee3a44p-1, 0x1.ee47b4p-1, 0x1.ee47b4p-1, 0x1.ee47b4p-1, 0x1.ee47b4p-1, 0x1.ee47b4p-1, 0x1.cf428ep-1, 0x1.cf428ep-1, 0x1.d14332p-1, 0x1.d7451cp-1, 0x1.df851cp-1, 0x1.e84p-1, 0x1.efdebap-1, 0x1.f4ca4p-1, 0x1.f78522p-1, 0x1.f891fp-1, 0x1.f8ba42p-1, 0x1.f8c7b4p-1, 0x1.f8c7b4p-1, 0x1.f8c7b4p-1, 0x1.f8c7b4p-1, 0x1.f8c7b4p-1, 0x1.d0d5c2p-1, 0x1.d0d5c2p-1, 0x1.d2d666p-1, 0x1.d8d85p-1, 0x1.e1184ep-1, 0x1.e9d332p-1, 0x1.f171ecp-1, 0x1.f65d74p-1, 0x1.f91856p-1, 0x1.fa252p-1, 0x1.fa4d78p-1, 0x1.fa5ae8p-1, 0x1.fa5aeap-1, 0x1.fa5ae8p-1, 0x1.fa5ae8p-1, 0x1.fa5ae8p-1, 0x1.d15c28p-1, 0x1.d15c28p-1, 0x1.d35cccp-1, 0x1.d95eb4p-1, 0x1.e19eb4p-1, 0x1.ea599ap-1, 0x1.f1f854p-1, 0x1.f6e3d8p-1, 0x1.f99eb8p-1, 0x1.faab8ap-1, 0x1.fad3dcp-1, 0x1.fae14ep-1, 0x1.fae14cp-1, 0x1.fae14ep-1, 0x1.fae14ep-1, 0x1.fae14ep-1, 0x1.d15c26p-1, 0x1.d15c26p-1, 0x1.d35ccep-1, 0x1.d95eb6p-1, 0x1.e19eb6p-1, 0x1.ea5996p-1, 0x1.f1f854p-1, 0x1.f6e3dap-1, 0x1.f99ebep-1, 0x1.faab88p-1, 0x1.fad3ep-1, 0x1.fae14cp-1, 0x1.fae15p-1, 0x1.fae14cp-1, 0x1.fae14cp-1, 0x1.fae14cp-1, 0x1.d15c28p-1, 0x1.d15c28p-1, 0x1.d35cccp-1, 0x1.d95eb4p-1, 0x1.e19eb4p-1, 0x1.ea599ap-1, 0x1.f1f854p-1, 0x1.f6e3d8p-1, 0x1.f99eb8p-1, 0x1.faab8ap-1, 0x1.fad3dcp-1, 0x1.fae14ep-1, 0x1.fae14cp-1, 0x1.fae14ep-1, 0x1.fae14ep-1, 0x1.fae14ep-1, 0x1.d15c28p-1, 0x1.d15c28p-1, 0x1.d35cccp-1, 0x1.d95eb4p-1, 0x1.e19eb4p-1, 0x1.ea599ap-1, 0x1.f1f854p-1, 0x1.f6e3d8p-1, 0x1.f99eb8p-1, 0x1.faab8ap-1, 0x1.fad3dcp-1, 0x1.fae14ep-1, 0x1.fae14cp-1, 0x1.fae14ep-1, 0x1.fae14ep-1, 0x1.fae14ep-1, 0x1.d15c28p-1, 0x1.d15c28p-1, 0x1.d35cccp-1, 0x1.d95eb4p-1, 0x1.e19eb4p-1, 0x1.ea599ap-1, 0x1.f1f854p-1, 0x1.f6e3d8p-1, 0x1.f99eb8p-1, 0x1.faab8ap-1, 0x1.fad3dcp-1, 0x1.fae14ep-1, 0x1.fae14cp-1, 0x1.fae14ep-1, 0x1.fae14ep-1, 0x1.fae14ep-1, };

    // hmm strange
    //result += image_count_inequal(l==0?expected_dev_output_l0:l==1?expected_dev_output_l1:expected_dev_output_l2, buf, wd * ht);
    int diff = image_count_inequal(actual_dev_output_l1, buf, wd * ht);
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
  int result = run_kernel(width, height, bpp, data, options);

  printf("Different values: %d\n", result); 
  printf("Result: %s\n", (result > 0 ? "failed" : "passed")); 
  return result;
}
