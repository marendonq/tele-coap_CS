Para correr clienteTestMessage.c se usa nuestro modulo/libreria message.h, para importar.

```
#include "message.h"
```
Para que el linker la encuentre

```
gcc clientTestMessage.c model/message.c -o clientePrueba
```

No se como ignorar los ejecutables con gitignore, procurar borrarlos antes de subir el codigo.

Aqui esta el socket UDP, que vamos a usar, mirar "reference/Sockets/BSD Socket"

[BSD socket](https://www.keil.com/pack/doc/mw6/network/html/group__bsd__routines.html)

