# Setup Inicial

### Pasos

Clonar el repo

```bash
git clone https://github.com/sisoputnfrba/tp-2019-2c-SOS.git
```

Correr `./script_https`.
Esto va a:
- Clonar las pruebas, hilolay, y las commons.
- Actualizar el makefile, libmuse.h, estres_privado.c y revolucion_privada.c de las pruebas con arreglos.
    - En makefile agregamos -llibmuse.
    - En libmuse.h sacamos la implementación default que devolvía 0.
    - En las dos pruebas cambiamos la ruta del archivo y agregamos un "hilolay_signal" en estres.
- Instalar commons, hilolay, libSUSE, libmuse
- Buildear SUSE, MUSE, common (de sac), sac server, sac cli
- Crea un disk.bin
- Formatea el disk.bin con sac-format

# Pruebas

## Memoria

Levantar SUSE para prueba de memoria:
```bash
cd ./scripts
./run_SUSE_memoria.sh
```

Levantar MUSE para prueba de memoria:
```bash
cd ./scripts
./run_muse_prueba_recursiva_swap.sh
```

Correr los tres procesos recursiva.c y despues archivo_de_swap_supermasivo.c