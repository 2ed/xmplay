CC=gcc
LIBS=-lmikmod -lpthread
FLAGS=-shared -fPIC

LIB=xmplay.so

all: $(LIB)

%.so: %.c
	$(CC) -o $@ $< $(LIBS) $(FLAGS)

clean:
	rm $(LIB)
