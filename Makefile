CXXFLAGS = -Wall -g

OBJS = main.o mpl.o error.o instruction.o

mpl: $(OBJS)
	g++ -g -o mpl $(OBJS)

$(OBJS): main.h mpl.h symbol.h instruction.h error.h

clean:
	rm -f *~ *.o *.stackdump
