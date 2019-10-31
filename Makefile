# What to call the final executable
TARGET = imapClient

# Which object files that the executable consists of
OBJS= imapClient.o

# What compiler to use
CC = gcc

# Compiler flags, -g for debug, -c to make an object file
#Replace MinGW include Path
CFLAGS = -c -g -DCURL_STATICLIB -I"C:\MinGW\lib\gcc\mingw32\8.2.0\include"

# This should point to a directory that holds libcurl, if it isn't
# in the system's standard lib dir
# We also set a -L to include the directory where we have the openssl
# libraries
#Replace MinGW lib Path
LDFLAGS = -L"C:\MinGW\lib"

# We need -lcurl for the curl stuff
LIBS = -lcurl

# Link the target with all objects and libraries
$(TARGET) : $(OBJS)
	$(CC)  -o $(TARGET) $(OBJS) $(LDFLAGS) $(LIBS)

# Compile the source files into object files
imapClient.o : imapClient.c
	$(CC) $(CFLAGS) $<

clean:
	rm $(OBJS)
