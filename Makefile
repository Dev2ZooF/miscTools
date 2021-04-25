ifeq ($(OS),Windows_NT)
	EXT := .exe
    CCFLAGS += -D WIN32
else
	EXT :=
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        CCFLAGS += -D LINUX
    endif
    ifeq ($(UNAME_S),Darwin)
        CCFLAGS += -D OSX
    endif
endif

default: .push
all: .rapidjson .push .test
.push: src/push.cpp
	g++ -g src/push.cpp src/push/Project.cpp src/utils/files.cpp src/utils/strings.cpp -I include $(CCFLAGS) -D RAPIDJSON_HAS_STDSTRING -std=c++17 -lstdc++fs -o build/push$(EXT)
.rapidjson: src/rapidJsonExample.cpp
	g++ -g src/rapidJsonExample.cpp -I include $(CCFLAGS) -o build/rapidjson$(EXT)
.test: src/test.cpp
	g++ -g src/test.cpp $(CCFLAGS) -o build/test$(EXT)