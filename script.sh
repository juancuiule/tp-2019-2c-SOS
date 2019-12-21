cd

sudo echo "export LD_LIBRARY_PATH=$HOME/tp-2019-2c-SOS/common/Debug" >> .bashrc

git clone https://github.com/sisoputnfrba/hilolay
git clone https://github.com/sisoputnfrba/linuse-tests-programs
git clone https://github.com/sisoputnfrba/so-commons-library

sudo cp $HOME/tp-2019-2c-SOS/makefile-pruebas $HOME/linuse-tests-programs/Makefile

cd so-commons-library
sudo make install
cd ..
cd hilolay
sudo make install
cd ..

cd linuse-tests-programs
sudo make 
sudo make entrega

cd tp-2019-2c-SOS/SUSE/Debug
sudo make

cd

cd tp-2019-2c-SOS/libSUSE/Debug
sudo make clean install

cd

cd tp-2019-2c-SOS/memoria/MUSE/Debug
sudo make

cd

cd tp-2019-2c-SOS/memoria/libmuse/Debug
sudo make
sudo make install



