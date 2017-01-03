CC = clang
LDLIBS = `pkg-config libsecret-1 --libs --cflags`
CFLAGS = -fsanitize=address

setpasswd: setpasswd.c

clean:
	rm setpasswd

install: setpasswd
	cp $< /usr/local/bin/
