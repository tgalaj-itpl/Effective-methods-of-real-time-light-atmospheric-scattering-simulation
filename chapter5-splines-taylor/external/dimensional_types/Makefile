# Copyright (c) 2016 Eric Bruneton
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
# list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
# this list of conditions and the following disclaimer in the documentation
# and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its contributors
# may be used to endorse or promote products derived from this software without
# specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

GPP := g++
GPP_FLAGS := -Wall -Wmain -pedantic -pedantic-errors -std=c++11

HEADERS := $(shell find . -name "*.h")
SOURCES := $(shell find . -name "*.cc")
OBJECTS := $(SOURCES:%.cc=output/Debug/%.o)

all: lint test

# cpplint can be installed with "pip install cpplint".
lint: $(HEADERS) $(SOURCES)
	cpplint --root=$(PWD) $^

test: output/Debug/test_main
	output/Debug/test_main

output/Debug/%.o: %.cc
	mkdir -p $(@D)
	$(GPP) $(GPP_FLAGS) -g -I. -c $< -o $@

output/Debug/test_main: $(OBJECTS)
	mkdir -p $(@D)
	$(GPP) -o $@ $^

clean:
	rm -rf output

