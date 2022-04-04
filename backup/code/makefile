src=$(wildcard *.cc)
target=$(patsubst %.cc,%,$(src))

LIB=-std=c++11 -lpthread
curr=$(shell pwd)
cgi=test_cgi

.PHONY:clean all output
all:$(target) $(cgi)

%:%.cc 
	g++ $< -o $@ $(LIB) 
$(cgi):cgi/test_cgi.cc 
	g++ -o $@ $^

clean:
	-rm -f $(target) $(cgi)
	-rm -rf output 

output:
	mkdir -p output 
	cp $(target) output 
	cp -rf wwwroot output 
	cp $(cgi) output/wwwroot
