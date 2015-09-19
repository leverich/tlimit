APP = tlimit

CFLAGS = -g
CC = gcc

SRCS = $(APP).c
HDRS = 

#################################################

all: $(APP)

$(APP): $(SRCS:.c=.o)
	$(LINK.c) $^ $(LOADLIBES) $(LDLIBS) -o $@

%.o: %.c $(HDRS)
	$(COMPILE.c) $(OUTPUT_OPTION) $<

clean:
	rm -f *.o *~ $(APP)
