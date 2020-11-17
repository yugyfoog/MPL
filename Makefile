CXXFLAGS = -Wall -Wextra -g

OBJS = mpl.o value.o code.o math.o function.o

all: mpl mpl.pdf

mpl: $(OBJS)
	g++ -g -o mpl $(OBJS)

$(OBJS): mpl.hh value.hh code.hh math.hh function.hh


mpl.pdf: mpl.tex
	pdflatex -interaction nonstopmode -file-line-error mpl.tex

install: mpl stdlib.mpl
	cp mpl /usr/local/bin
	cp stdlib.mpl /usr/local/share/mpl

clean:
	rm -f *.o *~

test: mpl
	mpl test.mpl
