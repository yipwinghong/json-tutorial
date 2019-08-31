# using GNU make to create project
cd github/json-tutorial/tutorial01
mkdir build
cd build

# generate makefile (Debug/Release)
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
