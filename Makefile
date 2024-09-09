all:
	meson setup builddir --wipe
	meson compile -C builddir

run: all
	./builddir/chat-server
