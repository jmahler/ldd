
OBJS := fifotst1

ARGV := -Wall -Wextra -O2 -g

all: $(OBJS)

%: %.c
	gcc $(ARGV) $< -o $@

clean:
	-rm -f $(OBJS)
