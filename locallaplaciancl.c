/*
    This file is part of darktable,
    copyright (c) 2016 johannes hanika.

    darktable is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    darktable is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with darktable.  If not, see <http://www.gnu.org/licenses/>.
*/
//#include "common/darktable.h"
//#include "common/opencl.h"

#include <CL/cl.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define ROUNDUP(a, n) ((a) % (n) == 0 ? (a) : ((a) / (n)+1) * (n))
#define ROUNDUPWD(a) dt_opencl_roundup(a)
#define ROUNDUPHT(a) dt_opencl_roundup(a)
#define MIN(a, b) ((a) > (b) ? (b) : (a))

int dt_opencl_roundup(int size)
{
  static int roundup = -1;

  /* first time run */
  if(roundup < 0)
  {
    //roundup = dt_conf_get_int("opencl_size_roundup");

    /* if not yet defined (or unsane), set a sane default */
    if(roundup <= 0)
    {
      roundup = 16;
      //dt_conf_set_int("opencl_size_roundup", roundup);
    }
  }

  return (size % roundup == 0 ? size : (size / roundup + 1) * roundup);
}

size_t dt_round_size(const size_t size, const size_t alignment)
{
  // Round the size of a buffer to the closest higher multiple
  return ((size % alignment) == 0) ? size : ((size - 1) / alignment + 1) * alignment;
}

void *dt_alloc_align(size_t alignment, size_t size)
{
  const size_t aligned_size = dt_round_size(size, alignment);
#if defined(__FreeBSD_version) && __FreeBSD_version < 700013
  return malloc(aligned_size);
#elif defined(_WIN32)
  return _aligned_malloc(aligned_size, alignment);
#else
  void *ptr = NULL;
  if(posix_memalign(&ptr, alignment, aligned_size)) return NULL;
  return ptr;
#endif
}

#ifdef _WIN32
void dt_free_align(void *mem)
{
  _aligned_free(mem);
}
#define dt_free_align_ptr dt_free_align
#else
#define dt_free_align(A) free(A)
#define dt_free_align_ptr free
#endif




int dt_opencl_write_host_to_device_raw(cl_command_queue queue, void *host, void *device, const size_t *origin,
                                       const size_t *region, const int rowpitch, const int blocking)
{
  cl_event *eventp = NULL; //dt_opencl_events_get_slot(devid, "[Write Image (from host to device)]");

  return clEnqueueWriteImage(queue,
                                                                    device, blocking, origin, region,
                                                                    rowpitch, 0, host, 0, NULL, eventp);
}

int dt_opencl_write_host_to_device_rowpitch(cl_command_queue queue, void *host, void *device, const int width,
                                            const int height, const int rowpitch)
{
  const size_t origin[] = { 0, 0, 0 };
  const size_t region[] = { width, height, 1 };
  // blocking.
  return dt_opencl_write_host_to_device_raw(queue, host, device, origin, region, rowpitch, CL_TRUE);
}

int dt_opencl_write_host_to_device(cl_command_queue queue, void *host, void *device, const int width,
                                   const int height, const int bpp)
{
  return dt_opencl_write_host_to_device_rowpitch(queue, host, device, width, height, width * bpp);
}





int dt_opencl_read_host_from_device_raw(cl_command_queue queue, void *host, void *device, const size_t *origin,
                                        const size_t *region, const int rowpitch, const int blocking)
{
  cl_event *eventp = NULL; //dt_opencl_events_get_slot(devid, "[Read Image (from device to host)]");

  return clEnqueueReadImage(queue,
                                                                   device, blocking, origin, region, rowpitch,
                                                                   0, host, 0, NULL, eventp);
}
int dt_opencl_read_host_from_device_rowpitch(cl_command_queue queue, void *host, void *device, const int width,
                                             const int height, const int rowpitch)
{
  const size_t origin[] = { 0, 0, 0 };
  const size_t region[] = { width, height, 1 };
  // blocking.
  return dt_opencl_read_host_from_device_raw(queue, host, device, origin, region, rowpitch, CL_TRUE);
}

int dt_opencl_read_host_from_device(cl_command_queue queue, void *host, void *device, const int width,
                                    const int height, const int bpp)
{
  return dt_opencl_read_host_from_device_rowpitch(queue, host, device, width, height, bpp * width);
}

int dt_opencl_copy_device_to_host(cl_command_queue queue, void *host, void *device, const int width,
                                  const int height, const int bpp)
{
  return dt_opencl_read_host_from_device(queue, host, device, width, height, bpp);
}



cl_mem dt_opencl_alloc_device(cl_context ctx, const int width, const int height, const int bpp)
{
  cl_int err;
  cl_image_format fmt;
  // guess pixel format from bytes per pixel
  if(bpp == 4 * sizeof(float))
    fmt = (cl_image_format){ CL_RGBA, CL_FLOAT };
  else if(bpp == sizeof(float))
    fmt = (cl_image_format){ CL_R, CL_FLOAT };
  else if(bpp == sizeof(uint16_t))
    fmt = (cl_image_format){ CL_R, CL_UNSIGNED_INT16 };
  else
    return NULL;

  cl_mem dev = clCreateImage2D(ctx, CL_MEM_READ_WRITE, &fmt, width, height, 0, NULL, &err);
  if(err != CL_SUCCESS)
    printf("[opencl alloc_device] could not alloc img buffer on device : %d\n", err);

  return dev;
}


int dt_opencl_set_kernel_arg(cl_kernel kernel, const int num, const size_t size,
                             const void *arg)
{
  return clSetKernelArg(kernel, num, size, arg);
}

cl_event *dt_opencl_events_get_slot(const int devid, const char *tag)
{
  // meh
  return NULL;
}

int dt_opencl_enqueue_kernel_2d_with_local(cl_command_queue queue, cl_kernel kernel, const size_t *sizes,
                                           const size_t *local)
{
  int err;
  cl_event *eventp = NULL; //dt_opencl_events_get_slot(0, NULL);
  err = clEnqueueNDRangeKernel(queue, kernel, 2, NULL, sizes, local, 0, NULL, eventp);
  // if (err == CL_SUCCESS) err = dt_opencl_finish(dev);
  if(err != CL_SUCCESS) goto error;
  return err;
error:
  fprintf(stderr, "dt_opencl_enqueue_kernel_2d_with_local failed: %d\n", err);
  return err;
}

int dt_opencl_enqueue_kernel_2d(cl_command_queue queue, cl_kernel kernel, const size_t *sizes)
{
  return dt_opencl_enqueue_kernel_2d_with_local(queue, kernel, sizes, NULL);
}






#include "locallaplaciancl.h"

#define max_levels 30
#define num_gamma 6

// downsample width/height to given level
static inline uint64_t dl(uint64_t size, const int level)
{
  for(int l=0;l<level;l++)
    size = (size-1)/2+1;
  return size;
}

dt_local_laplacian_cl_global_t *dt_local_laplacian_init_cl_global(cl_program program)
{
  dt_local_laplacian_cl_global_t *g = (dt_local_laplacian_cl_global_t *)malloc(sizeof(dt_local_laplacian_cl_global_t));

  cl_int err;

  g->kernel_pad_input          = clCreateKernel(program, "pad_input", &err);
  g->kernel_gauss_expand       = clCreateKernel(program, "gauss_expand", &err);
  g->kernel_gauss_reduce       = clCreateKernel(program, "gauss_reduce", &err);
  g->kernel_laplacian_assemble = clCreateKernel(program, "laplacian_assemble", &err);
  g->kernel_process_curve      = clCreateKernel(program, "process_curve", &err);
  g->kernel_write_back         = clCreateKernel(program, "write_back", &err);
  return g;
}

void dt_local_laplacian_free_cl(dt_local_laplacian_cl_t *g)
{
  if(!g) return;
  // be sure we're done with the memory:
  //dt_opencl_finish(g->devid);

  // free device mem
  for(int l=0;l<g->num_levels;l++)  // oops we are releasing more levels than we created
  {
    clReleaseMemObject(g->dev_padded[l]);
    clReleaseMemObject(g->dev_output[l]);
    for(int k=0;k<num_gamma;k++)
      clReleaseMemObject(g->dev_processed[k][l]);
  }
  for(int k=0;k<num_gamma;k++) free(g->dev_processed[k]);
  free(g->dev_padded);
  free(g->dev_output);
  free(g->dev_processed);
  g->dev_padded = g->dev_output = 0;
  g->dev_processed = 0;
  free(g);
}

dt_local_laplacian_cl_t *dt_local_laplacian_init_cl(
    dt_local_laplacian_cl_global_t *local_laplacian,
    cl_context ctx,
    cl_command_queue queue,
    const int width,            // width of input image
    const int height,           // height of input image
    const float sigma,          // user param: separate shadows/midtones/highlights
    const float shadows,        // user param: lift shadows
    const float highlights,     // user param: compress highlights
    const float clarity)        // user param: increase clarity/local contrast
{
  dt_local_laplacian_cl_t *g = (dt_local_laplacian_cl_t *)malloc(sizeof(dt_local_laplacian_cl_t));
  if(!g) return 0;

  g->global = local_laplacian;
  g->queue = queue;
  g->width = width;
  g->height = height;
  g->sigma = sigma;
  g->shadows = shadows;
  g->highlights = highlights;
  g->clarity = clarity;
  g->dev_padded = (cl_mem *)calloc(max_levels, sizeof(cl_mem *));   // g->num_levels should be enough right?
  g->dev_output = (cl_mem *)calloc(max_levels, sizeof(cl_mem *));
  g->dev_processed = (cl_mem **)calloc(num_gamma, sizeof(cl_mem **));
  for(int k=0;k<num_gamma;k++)
    g->dev_processed[k] = (cl_mem *)calloc(max_levels, sizeof(cl_mem *));

  g->num_levels = MIN(max_levels, 31-__builtin_clz(MIN(width,height)));
  g->max_supp = 1<<(g->num_levels-1);
  g->bwidth = ROUNDUPWD(width  + 2*g->max_supp);
  g->bheight = ROUNDUPHT(height + 2*g->max_supp);

  // get intermediate vector buffers with read-write access
  for(int l=0;l<g->num_levels;l++)
  {
    g->dev_padded[l] = dt_opencl_alloc_device(ctx, ROUNDUPWD(dl(g->bwidth, l)), ROUNDUPHT(dl(g->bheight, l)), sizeof(float));
    if(!g->dev_padded[l]) goto error;
    g->dev_output[l] = dt_opencl_alloc_device(ctx, ROUNDUPWD(dl(g->bwidth, l)), ROUNDUPHT(dl(g->bheight, l)), sizeof(float));
    if(!g->dev_output[l]) goto error;
    for(int k=0;k<num_gamma;k++)
    {
      g->dev_processed[k][l] = dt_opencl_alloc_device(ctx, ROUNDUPWD(dl(g->bwidth, l)), ROUNDUPHT(dl(g->bheight, l)), sizeof(float));
      if(!g->dev_processed[k][l]) goto error;
    }
  }

  return g;

error:
  fprintf(stderr, "[local laplacian cl] could not allocate temporary buffers\n");
  dt_local_laplacian_free_cl(g);
  return 0;
}


cl_int dt_local_laplacian_cl(
    dt_local_laplacian_cl_t *b, // opencl context with temp buffers
    cl_mem input,               // input buffer in some Labx or yuvx format
    cl_mem output)              // output buffer with colour
{
  cl_int err = -666;

  size_t sizes_pad[] = { ROUNDUPWD(b->bwidth), ROUNDUPHT(b->bheight), 1 };
  dt_opencl_set_kernel_arg(b->global->kernel_pad_input, 0, sizeof(cl_mem), (void *)&input);
  dt_opencl_set_kernel_arg(b->global->kernel_pad_input, 1, sizeof(cl_mem), (void *)&b->dev_padded[0]);
  dt_opencl_set_kernel_arg(b->global->kernel_pad_input, 2, sizeof(int), (void *)&b->width);
  dt_opencl_set_kernel_arg(b->global->kernel_pad_input, 3, sizeof(int), (void *)&b->height);
  dt_opencl_set_kernel_arg(b->global->kernel_pad_input, 4, sizeof(int), (void *)&b->max_supp);
  dt_opencl_set_kernel_arg(b->global->kernel_pad_input, 5, sizeof(int), (void *)&b->bwidth);
  dt_opencl_set_kernel_arg(b->global->kernel_pad_input, 6, sizeof(int), (void *)&b->bheight);
  err = dt_opencl_enqueue_kernel_2d(b->queue, b->global->kernel_pad_input, sizes_pad);
  if(err != CL_SUCCESS) goto error;

  // create gauss pyramid of padded input, write coarse directly to output
  for(int l=1;l<b->num_levels;l++)
  {
    const int wd = dl(b->bwidth, l), ht = dl(b->bheight, l);
    size_t sizes[] = { ROUNDUPWD(wd), ROUNDUPHT(ht), 1 };
    dt_opencl_set_kernel_arg(b->global->kernel_gauss_reduce, 0, sizeof(cl_mem), (void *)&b->dev_padded[l-1]);
    if(l == b->num_levels-1)
      dt_opencl_set_kernel_arg(b->global->kernel_gauss_reduce, 1, sizeof(cl_mem), (void *)&b->dev_output[l]);
    else
      dt_opencl_set_kernel_arg(b->global->kernel_gauss_reduce, 1, sizeof(cl_mem), (void *)&b->dev_padded[l]);
    dt_opencl_set_kernel_arg(b->global->kernel_gauss_reduce, 2, sizeof(int), (void *)&wd);
    dt_opencl_set_kernel_arg(b->global->kernel_gauss_reduce, 3, sizeof(int), (void *)&ht);
    err = dt_opencl_enqueue_kernel_2d(b->queue, b->global->kernel_gauss_reduce, sizes);
    if(err != CL_SUCCESS) goto error;
  }

  for(int k=0;k<num_gamma;k++)
  { // process images
    const float g = (k+.5f)/(float)num_gamma;
    dt_opencl_set_kernel_arg(b->global->kernel_process_curve, 0, sizeof(cl_mem), (void *)&b->dev_padded[0]);
    dt_opencl_set_kernel_arg(b->global->kernel_process_curve, 1, sizeof(cl_mem), (void *)&b->dev_processed[k][0]);
    dt_opencl_set_kernel_arg(b->global->kernel_process_curve, 2, sizeof(float), (void *)&g);
    dt_opencl_set_kernel_arg(b->global->kernel_process_curve, 3, sizeof(float), (void *)&b->sigma);
    dt_opencl_set_kernel_arg(b->global->kernel_process_curve, 4, sizeof(float), (void *)&b->shadows);
    dt_opencl_set_kernel_arg(b->global->kernel_process_curve, 5, sizeof(float), (void *)&b->highlights);
    dt_opencl_set_kernel_arg(b->global->kernel_process_curve, 6, sizeof(float), (void *)&b->clarity);
    dt_opencl_set_kernel_arg(b->global->kernel_process_curve, 7, sizeof(int), (void *)&b->bwidth);
    dt_opencl_set_kernel_arg(b->global->kernel_process_curve, 8, sizeof(int), (void *)&b->bheight);
    err = dt_opencl_enqueue_kernel_2d(b->queue, b->global->kernel_process_curve, sizes_pad);
    if(err != CL_SUCCESS) goto error;

    // create gaussian pyramids
    for(int l=1;l<b->num_levels;l++)
    {
      const int wd = dl(b->bwidth, l), ht = dl(b->bheight, l);
      size_t sizes[] = { ROUNDUPWD(wd), ROUNDUPHT(ht), 1 };
      dt_opencl_set_kernel_arg(b->global->kernel_gauss_reduce, 0, sizeof(cl_mem), (void *)&b->dev_processed[k][l-1]);
      dt_opencl_set_kernel_arg(b->global->kernel_gauss_reduce, 1, sizeof(cl_mem), (void *)&b->dev_processed[k][l]);
      dt_opencl_set_kernel_arg(b->global->kernel_gauss_reduce, 2, sizeof(int), (void *)&wd);
      dt_opencl_set_kernel_arg(b->global->kernel_gauss_reduce, 3, sizeof(int), (void *)&ht);
      err = dt_opencl_enqueue_kernel_2d(b->queue, b->global->kernel_gauss_reduce, sizes);
      if(err != CL_SUCCESS) goto error;
    }
  }



  // Dump all inputs to the next kernel
  err = clFinish(b->queue);

  FILE *txt = fopen("program-dump-laplacian-inout.txt", "w");

  for(int k=0;k<num_gamma;k++)
  for(int l=0;l<b->num_levels;l++)
  {
    const int wd = ROUNDUPWD(dl(b->bwidth, l));
    const int ht = ROUNDUPHT(dl(b->bheight, l));
    float *buf = dt_alloc_align(16, wd * ht * sizeof(float));

    err = dt_opencl_copy_device_to_host(b->queue, buf, b->dev_processed[k][l], wd, ht, sizeof(float));  

    fprintf(txt, "float input_dev_processed_k%dl%d[%d*%d] = { \n", k, l, wd, ht);

    char filename[256] = { 0 };
    snprintf(filename, sizeof(filename), "out/pre_dev_processed_%d_%d.pfm", k,l);
    {
      FILE *f = fopen(filename, "wb");
      fprintf(f, "Pf\n%d %d\n-1.0\n", wd, ht);
      for(int j=0;j<ht;j++)
        for(int i=0;i<wd;i++) {
            fwrite(buf + wd*j+i, 1, sizeof(float), f);
            fprintf(txt, "%a, ", buf[wd*j+i]);
        }
      fclose(f);
    }
    fprintf(txt, "};\n");

    dt_free_align(buf);
  }

  fprintf(txt, "\n\n\n");
  
  for(int l=0;l<b->num_levels;l+=1)
  {
    const int wd = ROUNDUPWD(dl(b->bwidth, l));
    const int ht = ROUNDUPHT(dl(b->bheight, l));
    float *buf = dt_alloc_align(16, wd * ht * sizeof(float));

    err = dt_opencl_copy_device_to_host(b->queue, buf, b->dev_padded[l], wd, ht, sizeof(float));  

    fprintf(txt, "float input_dev_padded_l%d[%d*%d] = { \n", l, wd, ht);

    char filename[256] = { 0 };
    snprintf(filename, sizeof(filename), "out/pre_padded_%d.pfm", l);
    {
      FILE *f = fopen(filename, "wb");
      fprintf(f, "Pf\n%d %d\n-1.0\n", wd, ht);
      for(int j=0;j<ht;j++)
        for(int i=0;i<wd;i++) {
            fwrite(buf + wd*j+i, 1, sizeof(float), f);
            fprintf(txt, "%a, ", buf[wd*j+i]);
        }
      fclose(f);
    }
    
    fprintf(txt, "};\n");

    dt_free_align(buf);
  }
  
  fprintf(txt, "\n\n\n");
  
  for(int l=0;l<b->num_levels;l+=1)
  {
    const int wd = ROUNDUPWD(dl(b->bwidth, l));
    const int ht = ROUNDUPHT(dl(b->bheight, l));
    float *buf = dt_alloc_align(16, wd * ht * sizeof(float));

    err = dt_opencl_copy_device_to_host(b->queue, buf, b->dev_output[l], wd, ht, sizeof(float));  

    fprintf(txt, "float input_dev_output_l%d[%d*%d] = { \n", l, wd, ht);

    char filename[256] = { 0 };
    snprintf(filename, sizeof(filename), "out/pre_output_%d.pfm", l);
    {
      FILE *f = fopen(filename, "wb");
      fprintf(f, "Pf\n%d %d\n-1.0\n", wd, ht);
      for(int j=0;j<ht;j++)
        for(int i=0;i<wd;i++) {
            fwrite(buf + wd*j+i, 1, sizeof(float), f);
            fprintf(txt, "%a, ", buf[wd*j+i]);
        }
      fclose(f);
    }

    fprintf(txt, "};\n");

    dt_free_align(buf);
  }

  fprintf(txt, "\n\n\n");





  // assemble output pyramid coarse to fine
  for(int l=b->num_levels-2;l >= 0; l--)
  {
    const int pw = dl(b->bwidth,l), ph = dl(b->bheight,l);
    size_t sizes[] = { ROUNDUPWD(pw), ROUNDUPHT(ph), 1 };
    // this is so dumb:
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
    if(err != CL_SUCCESS) goto error;
  }


  // Dump all outputs of the last kernel
  err = clFinish(b->queue);

  for(int l=0;l<b->num_levels;l+=1)
  {
    const int wd = ROUNDUPWD(dl(b->bwidth, l));
    const int ht = ROUNDUPHT(dl(b->bheight, l));
    float *buf = dt_alloc_align(16, wd * ht * sizeof(float));

    err = dt_opencl_copy_device_to_host(b->queue, buf, b->dev_output[l], wd, ht, sizeof(float));  

    fprintf(txt, "float expected_dev_output_l%d[%d*%d] = { \n", l, wd, ht);

    char filename[256] = { 0 };
    snprintf(filename, sizeof(filename), "out/post_output_%d.pfm", l);
    {
      FILE *f = fopen(filename, "wb");
      fprintf(f, "Pf\n%d %d\n-1.0\n", wd, ht);
      for(int j=0;j<ht;j++)
        for(int i=0;i<wd;i++) {
            fwrite(buf + wd*j+i, 1, sizeof(float), f);
            fprintf(txt, "%a, ", buf[wd*j+i]);
        }
      fclose(f);
    }

    fprintf(txt, "};\n");

    dt_free_align(buf);
  }
  
  fprintf(txt, "\n\n");
  fclose(txt);





  // read back processed L channel and copy colours:
  size_t sizes[] = { ROUNDUPWD(b->width), ROUNDUPHT(b->height), 1 };
  dt_opencl_set_kernel_arg(b->global->kernel_write_back, 0, sizeof(cl_mem), (void *)&input);
  dt_opencl_set_kernel_arg(b->global->kernel_write_back, 1, sizeof(cl_mem), (void *)&b->dev_output[0]);
  dt_opencl_set_kernel_arg(b->global->kernel_write_back, 2, sizeof(cl_mem), (void *)&output);
  dt_opencl_set_kernel_arg(b->global->kernel_write_back, 3, sizeof(int), (void *)&b->max_supp);
  dt_opencl_set_kernel_arg(b->global->kernel_write_back, 4, sizeof(int), (void *)&b->width);
  dt_opencl_set_kernel_arg(b->global->kernel_write_back, 5, sizeof(int), (void *)&b->height);
  err = dt_opencl_enqueue_kernel_2d(b->queue, b->global->kernel_write_back, sizes);
  if(err != CL_SUCCESS) goto error;
  return CL_SUCCESS;
error:
  fprintf(stderr, "[local laplacian cl] failed: %d\n", err);
  return err;
}
#undef max_levels
#undef num_gamma
