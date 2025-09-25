import asyncio
from aiocoap import *

async def main():
    # Dirección del servidor CoAP (cambia la IP por la de tu AWS)
    uri = "coap://54.209.202.31/sensors/temp"

    # Construir mensaje GET
    request = Message(code=GET, uri=uri)

    # Crear contexto y enviar
    protocol = await Context.create_client_context()

    try:
        response = await protocol.request(request).response
        print("Respuesta del servidor:", response.payload.decode())
    except Exception as e:
        print("Error:", e)

if __name__ == "__main__":
    asyncio.run(main())
