#pragma once
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

typedef struct dt_local_laplacian_cl_global_t
{
  cl_kernel kernel_pad_input;
  cl_kernel kernel_gauss_expand;
  cl_kernel kernel_gauss_reduce;
  cl_kernel kernel_laplacian_assemble;
  cl_kernel kernel_process_curve;
  cl_kernel kernel_write_back;
}
dt_local_laplacian_cl_global_t;

typedef struct dt_local_laplacian_cl_t
{
  int devid;
  cl_command_queue queue;
  dt_local_laplacian_cl_global_t *global;

  int width, height;
  int num_levels;
  float sigma, highlights, shadows, clarity;
  int blocksize, blockwd, blockht;
  int max_supp;
  size_t bwidth, bheight;

  // pyramid of padded monochrome input buffer
  cl_mem *dev_padded;
  // pyramid of padded output buffer, monochrome, too:
  cl_mem *dev_output;
  // one pyramid of padded monochrome buffers for every value
  // of gamma (curve parameter) that we process:
  cl_mem **dev_processed;
}
dt_local_laplacian_cl_t;

dt_local_laplacian_cl_global_t *dt_local_laplacian_init_cl_global(cl_program program);
dt_local_laplacian_cl_t *dt_local_laplacian_init_cl(
    dt_local_laplacian_cl_global_t *local_laplacian,
    cl_context ctx,
    cl_command_queue queue,
    const int width,            // width of input image
    const int height,           // height of input image
    const float sigma,          // user param: separate shadows/midtones/highlights
    const float shadows,        // user param: lift shadows
    const float highlights,     // user param: compress highlights
    const float clarity);       // user param: increase clarity/local contrast
void dt_local_laplacian_free_cl(dt_local_laplacian_cl_t *g);
cl_int dt_local_laplacian_cl(dt_local_laplacian_cl_t *g, cl_mem input, cl_mem output);



cl_mem dt_opencl_alloc_device(cl_context ctx, const int width, const int height, const int bpp);
int dt_opencl_write_host_to_device(cl_command_queue queue, void *host, void *device, const int width,
                                   const int height, const int bpp);
int dt_opencl_copy_device_to_host(cl_command_queue queue, void *host, void *device, const int width,
                                  const int height, const int bpp);
