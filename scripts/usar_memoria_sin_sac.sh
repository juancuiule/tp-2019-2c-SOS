sudo cp $HOME/tp-2019-2c-SOS/memoria/pruebas_con_path/estres_privado.c $HOME/linuse-tests-programs/estres_privado.c
sudo cp $HOME/tp-2019-2c-SOS/memoria/pruebas_con_path/revolucion_privada.c $HOME/linuse-tests-programs/revolucion_privada.c

cd $HOME/linuse-tests-programs/
sudo make clean
sudo make
sudo make entrega

