CFLAGS = -std=c++20 \
		 -O2 \

IFLAGS = -Iinc

LDFLAGS = -lglfw \
		  -lvulkan \
		  -ldl \
		  -lpthread \
		  -lshaderc \
		  -lglslang \
		  -lSPIRV-Tools \
		  -lSPIRV-Tools-opt \
		  -lshaderc_combined \

VulkanApp: main.cpp $(wildcard src/**/*.cpp)
	g++ $(CFLAGS) -o $@ $^ $(LDFLAGS) $(IFLAGS)

.PHONY: test clean

test: VulkanApp
	./VulkanApp

clean:
	rm -f VulkanApp
