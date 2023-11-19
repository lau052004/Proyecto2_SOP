CC = g++
CFLAGS = 
LDFLAGS =

all: controlador AgenteReservas AgenteReservas2 AgenteReservas3

controlador: ControladorReservas.cpp Controlador.h
	$(CC) $(CFLAGS) -o controlador ControladorReservas.cpp $(LDFLAGS)

AgenteReservas: AgenteReservas.cpp Agente.h
	$(CC) $(CFLAGS) -o AgenteReservas AgenteReservas.cpp $(LDFLAGS)

AgenteReservas2: AgenteReservas2.cpp Agente.h
	$(CC) $(CFLAGS) -o AgenteReservas2 AgenteReservas2.cpp $(LDFLAGS)

AgenteReservas3: AgenteReservas3.cpp Agente.h
	$(CC) $(CFLAGS) -o AgenteReservas3 AgenteReservas3.cpp $(LDFLAGS)

clean:
	rm -f controlador AgenteReservas AgenteReservas2 AgenteReservas3