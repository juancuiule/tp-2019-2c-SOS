# Instrucciones

### Pasos

Clonar el repo

```
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
