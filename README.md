# dt-test_locallaplaciancl
Standalone darktable OpenCL kernel test program for Local Contrast issue with AMD graphics cards and ROCm.

 * darktable https://github.com/darktable-org/darktable/issues/3756
 
 * amdrocm https://github.com/RadeonOpenCompute/ROCm-OpenCL-Runtime/issues/103

Build with `make`

## Minimal test

Run with `pr -T -m <(./testeasierkernel -O0) <(./testeasierkernel -O1)`

The [kernel](https://github.com/RvRijsselt/dt-test_locallaplaciancl/blob/master/locallaplacian.cl#L336) has 14 input images as input. There is one switch statement which should step through 4 of them. The output of optimized version shows that only 2 images are read while the unoptimized reads the expected 4.

```
    1  Darktable local laplacian test      Darktable local laplacian test                                                   
     2    Device: gfx900                      Device: gfx900                                                                 
     3    Hardware version: OpenCL 2.0        Hardware version: OpenCL 2.0                                                   
     4    Software version: 3052.0 (HSA1.1,   Software version: 3052.0 (HSA1.1,                                              
     5    OpenCL C version: OpenCL C 2.0      OpenCL C version: OpenCL C 2.0                                                 
     6    Build options: -O0                  Build options: -O1        \
...
    10    0.000000 == 0.010000 x              0.000000 == 0.010000 x      
    62    0.202335 == 1.110000 x              0.202335 == 1.010000 x
   113    0.400778 == 2.210000 x              0.400778 == 2.010000 x
   165    0.603113 == 3.310000 x              0.603113 == 3.210000 x   
   216    0.801556 == 4.410000 x              0.801556 == 4.210000 x
   265    0.992218 == 4.410000 x              0.992218 == 4.210000 x
```
Note in the output the value (e.g. 3.31) the most significant digit represents the selected switch case ([switch case 3](https://github.com/RvRijsselt/dt-test_locallaplaciancl/blob/master/locallaplacian.cl#L373)). The next digits is simply the value of the first pixel of the selected image (here value 0.31, comes from image with parameter name [input_dev_processed_k3l1](https://github.com/RvRijsselt/dt-test_locallaplaciancl/blob/master/testeasierkernel.c#L238), kernel param name [buf_g3_l0](https://github.com/RvRijsselt/dt-test_locallaplaciancl/blob/master/locallaplacian.cl#L346)). In the unoptimized output you see that the switch case and image numbers match. In the optimized version cases 0-2 read image 0 and cases 3-4 read image 2. 

## Full program test
Run with `./testlocallaplaciancl`

Example output with AMDPro (for me this driver's results is the ground truth. i.e. no artifacts when kernel is used in darktable):
```
Darktable local laplacian test
/opt/amdgpu/share/libdrm/amdgpu.ids: No such file or directory
  Device: gfx900
  Hardware version: OpenCL 2.0 AMD-APP (2906.7)
  Software version: 2906.7 (PAL,HSAIL)
  OpenCL C version: OpenCL C 2.0 
  Build options: 
Output data:
-4.23  -2.96  -1.72  -0.49  00.73  01.93  03.12  04.32  05.54  06.82  
08.94  10.17  11.34  12.48  13.59  14.68  15.74  16.79  17.85  18.96  
20.11  21.23  22.33  23.43  24.53  25.65  26.75  27.82  28.90  30.01  
30.39  31.49  32.54  33.57  34.58  35.59  36.59  37.60  38.63  39.74  
39.91  41.08  42.22  43.31  44.37  45.40  46.40  47.39  48.41  49.47  
49.59  50.68  51.72  52.74  53.74  54.74  55.74  56.76  57.84  58.99  
59.32  60.47  61.54  62.56  63.55  64.53  65.49  66.45  67.45  68.52  
69.20  70.30  71.37  72.41  73.45  74.50  75.56  76.60  77.66  78.76  
79.90  81.02  82.09  83.12  84.12  85.13  86.14  87.18  88.26  89.41  
91.97  93.23  94.46  95.67  96.86  98.02  99.18  100.33  101.51  102.73  
Done
```

Example output with ROCm (for me this driver results in ugly artifacts when kernel is used in darktable):

```
Darktable local laplacian test
  Device: gfx900
  Hardware version: OpenCL 2.0 
  Software version: 3052.0 (HSA1.1,LC)
  OpenCL C version: OpenCL C 2.0 
  Build options: 
Output data:
-4.23  -2.96  -1.72  -0.49  00.74  01.95  03.17  04.40  05.66  06.96  
08.94  10.17  11.34  12.48  13.61  14.76  15.92  17.11  18.32  19.54  
20.65  21.56  22.45  23.53  24.74  26.01  27.73  29.52  31.32  33.11  
34.79  35.49  36.21  37.64  39.43  41.21  42.99  44.76  46.48  48.11  
43.30  43.18  43.28  44.72  45.72  47.21  48.66  50.07  51.43  52.72  
56.16  56.95  57.77  59.27  55.61  56.98  58.31  59.58  60.84  62.36  
62.03  63.89  65.69  67.57  67.93  69.51  71.00  72.40  73.76  75.14  
76.31  77.58  78.68  79.58  80.40  75.01  76.54  77.95  79.35  80.83  
82.24  83.48  84.48  85.25  88.11  88.92  89.58  90.09  90.57  91.17  
89.76  89.63  89.22  88.76  95.15  94.76  94.31  93.83  93.44  93.29  
Different values: 92
Result: failed
```

OpenCL build options can be given as the first parameter: `./testlocallaplaciancl "between quotes :-) -cl-fast-relaxed-math"`

Output of ROCm without optimizations: `./testlocallaplaciancl -O0`

```
Darktable local laplacian test
  Device: gfx900
  Hardware version: OpenCL 2.0 
  Software version: 3052.0 (HSA1.1,LC)
  OpenCL C version: OpenCL C 2.0 
  Build options: -O0
Output data:
-4.23  -2.96  -1.72  -0.49  00.73  01.93  03.12  04.32  05.54  06.82  
08.94  10.17  11.34  12.48  13.59  14.68  15.74  16.79  17.85  18.96  
20.11  21.23  22.33  23.43  24.53  25.65  26.75  27.82  28.90  30.01  
30.39  31.49  32.54  33.57  34.58  35.59  36.59  37.60  38.63  39.74  
39.91  41.08  42.22  43.31  44.37  45.40  46.40  47.39  48.41  49.47  
49.59  50.68  51.72  52.74  53.74  54.74  55.74  56.76  57.84  58.99  
59.32  60.47  61.54  62.56  63.55  64.53  65.49  66.45  67.45  68.52  
69.20  70.30  71.37  72.41  73.45  74.50  75.56  76.60  77.66  78.76  
79.90  81.02  82.09  83.12  84.12  85.13  86.14  87.18  88.26  89.41  
91.97  93.23  94.46  95.67  96.86  98.02  99.18  100.33  101.51  102.73  
Different values: 0
Result: passed
```

With optimizations disabled the output is exactly the same as the ground truth. Unfortunately it is not possible to use -O0 with darktable... `export AMD_OCL_BUILD_OPTIONS_APPEND="-O0"` results in compilation errors.

