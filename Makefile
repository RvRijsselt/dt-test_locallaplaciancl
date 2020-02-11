CFLAGS := -DCL_USE_DEPRECATED_OPENCL_1_1_APIS=1 -DCL_USE_DEPRECATED_OPENCL_1_2_APIS=1
LDLIBS := -lOpenCL -lm

OUT := testlocallaplaciancl 
all: $(OUT)

testlocallaplaciancl: testlocallaplaciancl.c locallaplaciancl.c

clean:
	-rm $(OUT)
