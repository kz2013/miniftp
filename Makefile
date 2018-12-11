#CFLAGS = -O2	#O2为优化
CFLAGS = -lpthread
TARGET = ftpclient
OBJS = main.o client.o ftpbase.o net_server.o net_client.o \
       ftpconnect.o shell.o strkit.o input.o

$(TARGET): $(OBJS)
	$(CC) $^ -o $@ $(CFLAGS)
#$(OBJS) : %.o: %.c
#	$(CC) -c $(CFLAGS) $< -o $@
clean:
	-$(RM) $(OBJS) $(TARGET)
