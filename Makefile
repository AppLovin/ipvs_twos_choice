CONFIG_MODULE_SIG=n
obj-m += ip_vs_twos.o

ifndef KERNEL_HEADER_VER
KERNEL_HEADER_VER=$(shell uname -r)
endif

TEST_TARGET=test/ip_vs_twos_test

all:
	make -C /lib/modules/$(KERNEL_HEADER_VER)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(KERNEL_HEADER_VER)/build M=$(PWD) clean

$(TEST_TARGET): $(TEST_TARGET).cc
	$(CXX) $(CXX_FLAGS) $(LDFLAGS) -g3 -ggdb -O0 -lgtest $(TEST_TARGET).cc -o $(TEST_TARGET)

test: $(TEST_TARGET)
	$(TEST_TARGET)
