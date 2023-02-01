FROM ubuntu:22.04 
# Set enviroment variables
ENV DEBIAN_FRONTEND noninteractive
ENV TZ=America/New_York

# Update system
RUN apt-get -y update

# Install dependancies
RUN apt-get -y install g++ clang python3 cmake ninja-build git ccache build-essential
RUN apt-get -y install gir1.2-goocanvas-2.0 python3-gi python3-gi-cairo python3-pygraphviz libgraphviz-dev gir1.2-gtk-3.0 ipython3
RUN apt-get -y install python3-dev pkg-config sqlite3
RUN apt-get -y install qtbase5-dev qtchooser qt5-qmake qtbase5-dev-tools
RUN apt-get -y install openmpi-bin openmpi-common openmpi-doc libopenmpi-dev
RUN apt-get -y install mercurial unzip tar
RUN apt-get -y install gdb valgrind
RUN apt-get -y install clang-format
RUN apt-get -y install doxygen graphviz imagemagick
RUN apt-get -y install texlive texlive-extra-utils texlive-latex-extra texlive-font-utils dvipng latexmk
RUN apt-get -y install python3-sphinx dia
RUN apt-get -y install gsl-bin libgsl-dev libgslcblas0
RUN apt-get -y install tcpdump
RUN apt-get -y install sqlite libsqlite3-dev
RUN apt-get -y install libxml2 libxml2-dev
RUN apt-get -y install libc6-dev libc6-dev-i386 libclang-dev llvm-dev automake python3-pip
RUN apt-get -y install libgtk-3-dev
RUN apt-get -y install vtun lxc uml-utilities
RUN apt-get -y install libboost-all-dev
RUN apt-get -y install ubuntu-gnome-desktop

# Create new user
RUN useradd ns3-user -m -d /home/ns3-user
USER ns3-user

# Install pip dependancies
RUN python3 -m pip install --user pygraphviz
RUN python3 -m pip install --user cppy
RUN python3 -m pip install --user cxxfilt

# Set up ns3
WORKDIR /home/ns3-user/ns3-workspace
RUN git clone https://gitlab.com/nsnam/ns-3-dev.git
WORKDIR /home/ns3-user/ns3-workspace/ns-3-dev
RUN git checkout -b ns-3.37-branch ns-3.37 
RUN ./ns3 configure --enable-examples --enable-tests --enable-python-bindings

# Test
RUN python3 test.py

# Set up custom software
COPY src/ /home/ns3-user/ns3-workspace/ns-3-dev/scratch/

# Build src files 
RUN ./ns3 build
