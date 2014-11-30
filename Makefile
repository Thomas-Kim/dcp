SHELL=/bin/bash
CC=gcc
CPP=g++
CFLAGS=-O3 -fdiagnostics-color=auto -pthread -std=gnu11 -g
CPPFLAGS=$(filter-out -std=gnu11, $(CFLAGS)) -std=gnu++11 -fno-exceptions -Wno-write-strings
LINK=-lstdc++ -lrt
MKDIRS=lib bin tst/bin .pass .pass/tst/bin .make .make/bin .make/tst/bin
INCLUDE=$(addprefix -I,include)
EXECS=$(addprefix bin/,setup dcp)
TESTS=$(addprefix tst/bin/,todo equals file directory)
PAPERS=proposal/proposal.pdf report/report.pdf
.PHONY: default all clean again check papers distcheck dist-check
.SECONDARY:
default: all
all: $(EXECS) $(TESTS)
clean:
	rm -rf $(MKDIRS) $(PAPERS)
again: clean all
check: $(addprefix .pass/,$(TESTS))
dist-check distcheck:
	@rm -rf .pass
	@make --no-print-directory check
.pass/tst/bin/%: tst/bin/% | .pass/tst/bin
	@printf "$<: "
	@$<\
		&& echo -e "\033[0;32mpass\033[0m" && touch $@\
		|| echo -e "\033[0;31mfail\033[0m"
.pass/tst/bin/benchmarks: bin/dcp
FNM=\([a-z_A-Z]*\)
.make/%.d: %.c
	@mkdir -p $(@D)
	@$(CC) -MM $(INCLUDE) $< -o $@
.make/%.d: %.cpp
	@mkdir -p $(@D)
	@$(CPP) -MM $(INCLUDE) $< -o $@
.make/bin/%.d: .make/%.d | .make/bin
	@sed 's/include\/$(FNM).h/lib\/\1.o/g' $< > $@
	@sed -i 's/$(FNM).o:/bin\/\1:/g' $@
.make/tst/bin/%.d: .make/tst/%.d | .make/tst/bin
	@sed 's/include\/$(FNM).h/lib\/\1.o/g' $< > $@
	@sed -i 's/$(FNM).o:/tst\/bin\/\1:/g' $@
	
MAKES=$(addsuffix .d,$(addprefix .make/, $(EXECS) $(TESTS)))
-include $(MAKES)

papers: $(PAPERS)
$(MKDIRS):
	@mkdir -p $@
bin/%: %.cpp | bin
	$(CPP) $(CPPFLAGS) $(INCLUDE) $^ -o $@ $(LINK)
bin/%: %.c | bin
	$(CC) $(CFLAGS) $(INCLUDE) $^ -o $@ $(LINK)
lib/%.o: src/%.cpp include/%.h | lib
	$(CPP) -c $(CPPFLAGS) $(INCLUDE) $< -o $@
lib/%.o: src/%.c include/%.h | lib
	$(CC) -c $(CFLAGS) $(INCLUDE) $< -o $@
tst/bin/%: tst/%.cpp lib/%.o | tst/bin
	$(CPP) $(CPPFLAGS) $(INCLUDE) $^ -o $@ $(LINK)
tst/bin/%: tst/%.c lib/%.o | tst/bin
	$(CC) $(CFLAGS) $(INCLUDE) $^ -o $@ $(LINK)
%.pdf: %.tex
	pdflatex -output-directory $(@D) $<
