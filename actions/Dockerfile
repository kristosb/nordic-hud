# Fetch ubuntu image
FROM ubuntu:22.04

# Install build tools
RUN apt update && \
	apt install -y wget build-essential autoconf automake libtool
	
# Copy project into image
RUN mkdir /project
COPY ../src /project/src
COPY ../tests /project/tests
COPY ../lint /project/lint
COPY ../Makefile /project/Makefile

# Download and build CppUTest
RUN mkdir /project/tools/ && \
	cd /project/ && \
	wget https://github.com/cpputest/cpputest/releases/download/v4.0/cpputest-4.0.tar.gz && \
	tar xf cpputest-4.0.tar.gz && \
	mv cpputest-4.0/ tools/cpputest/ && \
	cd tools/cpputest/ && \
	autoreconf -i && \
	./configure && \
	make && \
	# Download and build CppCheck
	cd .. && \
	cd .. && \
	ls && \
	wget https://github.com/danmar/cppcheck/archive/refs/tags/2.13.0.tar.gz && \
	tar xf 2.13.0.tar.gz && \
	mv cppcheck-2.13.0/ tools/cppcheck/ && \
	cd tools/cppcheck/ && \
	make


# Execute script
#ENTRYPOINT ["make", "test", "-C", "/project/"]
#ENTRYPOINT ["/bin/bash"]
#CMD ["-c", "ls", ";", "pwd"]