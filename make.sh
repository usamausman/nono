if [ ! -d build ]
then
  mkdir build
fi

cd build

if [ ! -f Makefile ]
then
  cmake -DCMAKE_PREFIX_PATH=/usr/local/opt/ncurses ..
fi

make