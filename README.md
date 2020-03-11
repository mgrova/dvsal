# dvsal
Dynamic Vision Sensors Abstraction Layer

[![Releases](https://img.shields.io/github/release/mgrova/dvsal.svg)](https://github.com/mgrova/dvsal/releases)  [![Issues](https://img.shields.io/github/issues/mgrova/dvsal.svg)](https://github.com/mgrova/dvsal/issues)

[![Build Status](https://travis-ci.com/mgrova/dvsal.svg?branch=master)](https://travis-ci.com/mgrova/dvsal)

## Dependencies

### DVS Libraries
```
sudo add-apt-repository -y ppa:inivation-ppa/inivation
sudo apt-get update
sudo apt-get install -y dv-runtime-dev
```

### OpenCV
```
sudo apt-get install -y libopencv-dev
```

### Boost
```
sudo apt-get install -y libboost-all-dev curl
```

### Eigen 3
```
sudo apt-get install -y libeigen3-dev
```

### flow
```
git clone https://github.com/Bardo91/flow
cd flow 
mkdir build ; cd build
cmake .. ; make -j4 
sudo make install
```
