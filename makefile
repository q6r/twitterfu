CPP_FILES := $(wildcard src/*.cpp)
OBJ_FILES := $(addprefix obj/,$(notdir $(CPP_FILES:.cpp=.o)))
LD_FLAGS := -ltwitcurl
CC_FLAGS := -Wall

twitterfu: $(OBJ_FILES)
	g++ $(LD_FLAGS) $(CC_FLAGS) -o $@ $^
obj/%.o: src/%.cpp
	g++ $(CC_FLAGS) -c -o $@ $<
clean:
	rm -rf obj/*.o twitterfu
