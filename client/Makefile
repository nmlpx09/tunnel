CC = g++
CCFLAGS = -O3 -std=c++23 -pedantic -Wall -W -Werror -Wextra -c -I.
DEFINES =

OBJ = main.o
LDFLAGS=

RUN = client

alsa: $(OBJ)
	$(CC) $^ -o $(RUN) $(LDFLAGS)

%.o: %.cpp
	$(CC) $(DEFINES) $(CCFLAGS) $< -o $@

clean:
	rm -f $(OBJ) $(RUN)
