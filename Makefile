CC=gcc
LIBS=-lmikmod -lpthread
FLAGS=-shared -fPIC

LIB=xmplay.so

all: $(LIB)

%.o: %.c
	$(CC) -c -o $@ $< $(LIBS) $(FLAGS)

clean:
	rm $(LIB)
