if [ "$1" = "clean" ]; then
  rm -rf build
fi

if [ ! -d build ]; then
  mkdir build
  cd build
  cmake .. -GNinja
  ninja
  ./astar
else
  cd build
  ninja
  ./astar
fi