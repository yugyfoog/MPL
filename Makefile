CXXFLAGS = -Wall -g

OBJS = main.o mpl.o error.o

mpl: $(OBJS)
	g++ -g -o mpl $(OBJS)

$(OBJS): main.h mpl.h instruction.h error.h

clean:
	rm -f *~ *.o *.stackdump
