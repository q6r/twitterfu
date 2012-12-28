CPP_FILES := $(wildcard src/*.cpp)
OBJ_FILES := $(addprefix obj/,$(notdir $(CPP_FILES:.cpp=.o)))
LD_FLAGS := -ltwitcurl -lsqlite3
CC_FLAGS := -Wall -O3 -std=c++0x

twitterfu: $(OBJ_FILES)
	g++ $(LD_FLAGS) $(CC_FLAGS) -o $@ $^
obj/%.o: src/%.cpp
	g++ $(CC_FLAGS) -c -o $@ $<
clean:
	rm -rf obj/*.o twitterfu
