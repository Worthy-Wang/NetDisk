#版本三
CC=g++
CFLAGS=-g -Wall
target=server
src=$(wildcard *.cpp)
obj=$(patsubst %.cpp, %.o, $(src))
link=-lpthread `mysql_config --cflags --libs` -lcrypt
$(target):$(obj)
	$(CC) $(CFLAGS)  $^ -o $@ $(link)
%.o:%.cpp
	$(CC) $(CFLAGS) -c $< -o $@ $(link)
clean:
	rm -rf $(target) $(obj)
