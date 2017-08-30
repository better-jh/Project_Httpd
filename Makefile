bin=httpd
cc=gcc
obj=http.o main.o
FLAGS=#-D_DEBUG_
LDFLAGS=-lpthread
CGI_PATH=sql wwwroot/cgi-bin
 
.PHONY:all
all:$(bin) cgi

$(bin):$(obj)
	@$(cc) -o $@ $^ $(LDFLAGS)
	@echo "linking ...done"
%.o:%.c
	@gcc -c $< $(FLAGS)
	@echo "compiling ...done"
cgi:
	@for i in `echo $(CGI_PATH)`;\
	do\
		cd $$i;\
		make;\
		cd -;\
	done


.PHONY:clean
clean:
	@rm -rf $(bin) *.o output
	@for i in `echo $(CGI_PATH)`;\
	do\
		cd $$i;\
		make clean;\
		cd -;\
	done
	@echo "cleaning ...done"

.PHONY:output
output:
	@mkdir -p output/wwwroot/cgi-bin
	@cp -rf log output
	@cp -rf conf output
	@cp wwwroot/index.html output/wwwroot
	@cp wwwroot/cgi-bin/math_cgi output/wwwroot/cgi-bin
	@cp -rf wwwroot/img output/wwwroot
	@cp wwwroot/img/gg.jpg output/wwwroot/img
	@cp sql/insert_cgi output/wwwroot/cgi-bin
	@cp sql/select_cgi output/wwwroot/cgi-bin
	@cp -rf sql/lib output
	@cp plugin/ctl_server.sh output
	@cp httpd output
	@echo "output project...done"
