SHELL=/bin/bash
CC=gcc
CPP=g++
CFLAGS=-O3 -fdiagnostics-color=auto -pthread -std=gnu11
CPPFLAGS=$(filter-out -std=gnu11, $(CFLAGS)) -std=gnu++11 -fno-exceptions -Wno-write-strings
MKDIRS=lib bin tst/bin .pass .pass/tst/bin
INCLUDE=$(addprefix -I,include)
EXECS=$(addprefix bin/,dcp)
TESTS=$(addprefix tst/bin,)
PAPERS=proposal/proposal.pdf
.PHONY: default all clean again check papers
.SECONDARY:
default: all
all: $(EXECS) $(TESTS)
clean:
	rm -rf $(MKDIRS) $(PAPERS)
again: clean all
check: $(addprefix .pass/,$(TESTS))
.pass/tst/bin/%: tst/bin/% | .pass/tst/bin
	@printf "$<: "
	@$<\
		&& echo -e "\033[0;32mpass\033[0m" && touch $@\
		|| echo -e "\033[0;32mfail\033[0m"
papers: $(PAPERS)
$(MKDIRS):
	@mkdir -p $@
bin/%: %.cpp | bin
	$(CPP) $(CPPFLAGS) $(INCLUDE) $< -o $@
bin/%: %.c | bin
	$(CC) $(CFLAGS) $(INCLUDE) $< -o $@
lib/%.o: src/%.cpp include/%.h | lib
	$(CPP) -c $(CPPFLAGS) $(INCLUDE) $< -o $@
lib/%.o: src/%.c include/%.h | lib
	$(CC) -c $(CFLAGS) $(INCLUDE) $< -o $@
tst/bin/%: tst/%.cpp lib/%.o | tst/bin
	$(CPP) $(CPPFLAGS) $(INCLUDE) $^ -o $@
tst/bin/%: tst/%.c lib/%.o | tst/bin
	$(CC) $(CFLAGS) $(INCLUDE) $^ -o $@
%.pdf: %.tex
	pdflatex -output-directory $(@D) $<
