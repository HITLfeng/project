src=$(wildcard *.cc)
target=$(patsubst %.cc,%,$(src))

LIB=-std=c++11 -lpthread
curr=$(shell pwd)
cgi=test_cgi
sql=mysql_con

.PHONY:clean all output
all:$(target) $(cgi) $(sql)

%:%.cc 
	g++ $< -o $@ $(LIB) 
#%:cgi/%.cc
#	g++ $< -o $@ $(LIB) 
$(cgi):cgi/test_cgi.cc 
	g++ -o $@ $^
$(sql):cgi/mysql_con.cc 
	g++ -o $@ $^ -std=c++11 -I/home/fxj/HTTP/code/cgi/my_sql/include -L/home/fxj/HTTP/code/cgi/my_sql/lib -lmysqlclient -lpthread -ldl -static

clean:
	-rm -f $(target) $(cgi)
	-rm -rf output 

output:
	mkdir -p output 
	cp $(target) output 
	cp -rf wwwroot output 
	cp $(cgi) output/wwwroot
	cp $(sql) output/wwwroot
