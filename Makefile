CFLAGS := -DCL_USE_DEPRECATED_OPENCL_1_1_APIS=1 -DCL_USE_DEPRECATED_OPENCL_1_2_APIS=1 -g
LDLIBS := -lOpenCL -lm 

OUT := testlocallaplaciancl testsinglekernel
all: $(OUT)

testlocallaplaciancl: testlocallaplaciancl.c locallaplaciancl.c
testsinglekernel: testsinglekernel.c locallaplaciancl.c single-data-input.h single-expected.h

clean:
	-rm $(OUT)
