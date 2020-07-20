CXXFLAGS = -Wall -Wextra -g

OBJS = mpl.o value.o code.o math.o function.o

all: mpl mpl.pdf

mpl: $(OBJS)
	g++ -g -o mpl $(OBJS)

$(OBJS): mpl.hh value.hh code.hh math.hh function.hh


mpl.pdf: mpl.tex
	pdflatex -interaction batchmode mpl.tex

clean:
	rm -f *.o *~

test: mpl
	mpl test.mpl
