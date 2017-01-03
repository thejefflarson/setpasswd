#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>

#include <libsecret/secret.h>

// from man getpass
ssize_t
gp(char **lineptr, size_t *n, FILE *stream) {
  struct termios old, new;
  int nread;

  /* Turn echoing off and fail if we can't. */
  if (tcgetattr(fileno (stream), &old) != 0)
    return -1;
  new = old;
  new.c_lflag &= ~ECHO;
  if (tcsetattr(fileno(stream), TCSAFLUSH, &new) != 0)
    return -1;

  /* Read the password. */
  nread = getline(lineptr, n, stream);

  /* Restore terminal. */
  tcsetattr(fileno(stream), TCSAFLUSH, &old);

  return nread;
}


int main(int argc, const char **argv) {
  if(argc != 3) {
    puts("usage: setpassword <site> <username>");
    return EXIT_FAILURE;
  }

  char *pass = NULL;
  size_t size = 0;
  printf("Enter password: ");
  fflush(stdin);

  ssize_t res = gp((char **)&pass, &size, stdin);
  puts("");
  if(res > 0) {
    static const SecretSchema schema = {
      "org.jeff.Password", SECRET_SCHEMA_DONT_MATCH_NAME,
      {
        {  "username", SECRET_SCHEMA_ATTRIBUTE_STRING },
        {  "service", SECRET_SCHEMA_ATTRIBUTE_STRING },
        {  "NULL", 0 }
      }
    };
    pass[res - 1] = '\0';
    GError *error = NULL;
    secret_password_store_sync(&schema, SECRET_COLLECTION_DEFAULT,
                               argv[2], pass, NULL, &error,
                               "username", argv[2],
                               "service", argv[1], NULL);
    if(error != NULL) {
      perror("Couldn't store password");
      g_error_free(error);
    }
  } else {
    free(pass);
    perror("Couldn't read password");
    return EXIT_FAILURE;
  }
  free(pass);
  return EXIT_SUCCESS;
}
