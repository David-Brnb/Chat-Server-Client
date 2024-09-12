# Comando para compilar ambos (servidor y cliente)
all:
	meson setup builddir --wipe
	meson compile -C builddir

# Comando para ejecutar el servidor
run-server: all
	meson compile -C builddir
	./builddir/server

# Comando para ejecutar el cliente
run-client: all
	meson compile -C builddir
	./builddir/client

# Limpia el directorio de compilación
clean:
	rm -rf builddir

