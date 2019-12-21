cd

./scripts/cargar_ld.sh

# Clonar repos
git clone git@github.com:sisoputnfrba/linuse-tests-programs.git
git clone git@github.com:sisoputnfrba/hilolay.git
# git clone git@github.com:sisoputnfrba/tp-2019-2c-SOS.git
git clone git@github.com:sisoputnfrba/so-commons-library.git

# Cambiar makefile para que usen -llibmuse
sudo cp $HOME/tp-2019-2c-SOS/makefile-pruebas $HOME/linuse-tests-programs/Makefile
sudo cp $HOME/tp-2019-2c-SOS/memoria/libmuse/libmuse/libmuse.h $HOME/linuse-tests-programs/libmuse.h
sudo cp $HOME/tp-2019-2c-SOS/memoria/pruebas_con_path/estres_privado.c $HOME/linuse-tests-programs/estres_privado.c
sudo cp $HOME/tp-2019-2c-SOS/memoria/pruebas_con_path/revolucion_privada.c $HOME/linuse-tests-programs/revolucion_privada.c

cd so-commons-library
sudo make install

cd $HOME

cd hilolay
sudo make install

cd $HOME

cd linuse-tests-programs
sudo make 
sudo make entrega

cd $HOME

cd tp-2019-2c-SOS/SUSE/Debug
sudo make

cd $HOME

cd tp-2019-2c-SOS/libSUSE/Debug
sudo make clean install

cd $HOME

cd tp-2019-2c-SOS/scripts
./build_muse.sh
./install_libmuse.sh

cd $HOME

cd tp-2019-2c-SOS/common/Debug
sudo make clean
sudo make

cd $HOME

cd tp-2019-2c-SOS/sac_server/Debug
sudo make clean
sudo make
dd if=/dev/urandom of=disk.bin bs=1024 count=512000
./sac-format disk.bin

cd $HOME

cd tp-2019-2c-SOS/sac_cli/Debug
sudo make clean
sudo make
mkdir tmp

cd $HOME