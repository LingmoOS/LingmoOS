ALIOTH_HOME=~/
export ALIOTH_HOME

wget ftp://ftp.gnuplot.info/pub/gnuplot/gnuplot-3.7.3.tar.gz
wget ftp://ftp.gnuplot.info/pub/gnuplot/libpng-1.2.5.tar.gz
tar xzf libpng-1.2.5.tar.gz
cd libpng-1.2.5

wget ftp://ftp.gnuplot.info/pub/gnuplot/zlib-1.2.1.tar.bz2
tar xjf zlib-1.2.1.tar.bz2
cd ..
ln -s libpng-1.2.5/zlib-1.2.1 zlib
cd zlib
./configure;make
cd ../libpng-1.2.5
cp scripts/makefile.gcc ./Makefile ; make
ln -s ../zlib/libz.a libz.a
ln -s ../zlib/zlib.h zlib.h
ln -s ../zlib/zconf.h zconf.h
ln -s . lib
ln -s . include

cd ..
tar xzf gnuplot-3.7.3.tar.gz
cd gnuplot-3.7.3
rm config.cache;./configure --prefix=$ALIOTH_HOME/gnuplot-3.7.3 --with-x=no --with-lasergnu=no --with-linux-vga=no --with-readline=no --with-gd=no --with-png=../libpng-1.2.5
make install


# DEBIAN WAY!
# apt-get source libpng-dev
# cd libpng3-1.2.1
# apt-get source zlib1
# cd zlib-1.1.4/upstream/tarballs;tar xzf zlib-1.1.4.tar.gz;cd ../../../..
# ln -s  libpng3-1.2.1/zlib-1.1.4/upstream/tarballs/zlib-1.1.4 zlib
# cd zlib
# ./configure;make
# cd ../libpng3-1.2.1
# cp scripts/makefile.gcc ./Makefile ; make
# ln -s ../zlib/libz.a libz.a
# ln -s ../zlib/zlib.h zlib.h
# ln -s ../zlib/zconf.h zconf.h
# ln -s . lib
# ln -s . include
# cd ..

#       ***************
#       *** gnuplot ***
#       ***************
# apt-get source gnuplot
# cd gnuplot-3.7.2
# rm config.cache;./configure --prefix=$ALIOTH_HOME/0.50.5 --with-x=no --with-lasergnu=no --with-linux-vga=no --with-readline=no --with-gd=no --with-png=../libpng3-1.2.1
# make install
