
OBJS := fifo_rw

ARGV := -Wall -Wextra -O2 -g

all: $(OBJS)

%: %.c
	gcc $(ARGV) $< -o $@

clean:
	-rm -f $(OBJS)
