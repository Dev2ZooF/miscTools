default: .push
all: .rapidjson .push
.rapidjson: src/rapidJsonExample.cpp
	g++ -g src/rapidJsonExample.cpp -I include -o build/rapidjson.exe
.push: src/push.cpp
	g++ -g src/push.cpp src/push/Project.cpp -I include -D RAPIDJSON_HAS_STDSTRING -std=c++17 -lstdc++fs -o build/push.exe
.test: src/test.cpp
	g++ -g src/test.cpp -o build/test.exe