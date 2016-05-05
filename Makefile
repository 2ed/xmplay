CC=gcc
LIBS=-lmikmod

OBJ=xmplay.o

%.o: %.c 
	$(CC) -c -o $@ $< 

xmplay: $(OBJ)
	gcc -o xmplay $(OBJ) $(LIBS)

