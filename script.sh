cd

git clone https://github.com/sisoputnfrba/hilolay
git clone https://github.com/sisoputnfrba/linuse-tests-programs
git clone https://github.com/sisoputnfrba/so-commons-library

cd so-commons-library
sudo make install
cd ..
cd hilolay
sudo make install
cd ..

cd linuse-tests-programs
sudo make 
sudo make entrega

git clone https://github.com/sisoputnfrba/tp-2019-2c-SOS

cd tp-2019-2c-SOS/SUSE/Debug
make

cd

cd tp-2019-2c-SOS/libSUSE/Debug
make
sudo make install

cd

cd tp-2019-2c-SOS/MUSE/Debug
make

cd

cd tp-2019-2c-SOS/libMUSE/Debug
make
sudo make install

