# dvsal

## Dependencies

### DVS Libraries
```
sudo add-apt-repository ppa:inivation-ppa/inivation
sudo apt-get update
sudo apt-get install dv-runtime-dev
```

### OpenCV
```
sudo apt-get install libopencv-dev
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
