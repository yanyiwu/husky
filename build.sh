CURRENTDIR=`pwd`
cd deps/libevent \
    && ./configure \
    && make \
    && cd $CURRENTDIR \
    && mkdir build \
    && cd build \
    && cmake .. \
    && make
