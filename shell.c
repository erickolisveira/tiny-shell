#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int sh_cd(char **args);
int sh_help(char **args);
int sh_exit(char **args);

char *builtin_str[] = {
  "cd",
  "help",
  "exit"
};

int (*builtin_func[]) (char **) = {
  &sh_cd,
  &sh_help,
  &sh_exit
};

int sh_num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}

int sh_cd(char **args) {
    if(args[1] == NULL) {
        fprintf(stderr, "sh: Argumento esperado para \"cd\".\n");
    } else {
        if(chdir(args[1]) != 0) {
            perror("sh");
        }
    }
    return 1;
}

int sh_help(char **args) {
    int i;
    printf("Tiny Shell por Erick Oliveira, com ajuda de Stephen Brennan!\n");
    printf("Digite o nome do comando e argumentos, e pressione Enter.\n");
    printf("Este shell possui suporte aos seguintes comandos: \n");

    for (i = 0; i < sh_num_builtins(); i++){
        printf("* %s\n", builtin_str[i]);
    }
    return 1;
}

int sh_exit(char **args) {
    return 0;
}

#define RL_BUFFER_SIZE 1024

char *sh_read_line(void){
    int buffer_size = RL_BUFFER_SIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * buffer_size);
    int c;

    if(!buffer){
        fprintf(stderr, "sh: Erro de alocação.\n");
        exit(EXIT_FAILURE);
    }

    while(1){
        c = getchar();

        if(c == EOF || c == '\n'){
            buffer[position] = '\0';
            return buffer;
        } else {
            buffer[position] = c;
        }
        position++;

        if (position >= buffer_size){
            buffer_size += RL_BUFFER_SIZE;
            buffer = realloc(buffer, buffer_size);
            if(!buffer){
                fprintf(stderr, "sh: Erro de alocação.\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

#define TOK_BUFFER_SIZE 64
#define TOK_DELIM " \t\n\r\a"

char **sh_split_line(char *line){
    int buffer_size = TOK_BUFFER_SIZE;
    int position = 0;
    char **tokens = malloc(buffer_size * sizeof(char*));
    char *token;

    if(!tokens) {
        fprintf(stderr, "sh: Erro de alocação.\n");
        exit(EXIT_FAILURE);
    }
    
    token = strtok(line, TOK_DELIM);
    while(token != NULL) {
        tokens[position] = token;
        position++;

        if(position >= buffer_size) {
            buffer_size += TOK_BUFFER_SIZE;
            tokens = realloc(tokens, buffer_size * sizeof(char*));
            if (!tokens) {
                fprintf(stderr, "sh: Erro de alocação.\n");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

int sh_launch(char **args) {
    pid_t pid, wpid;
    int status;

    pid = fork();
    if(pid == 0) {
        //processo filho
        if(execvp(args[0], args) == -1){
            perror("sh");
        }
        exit(EXIT_FAILURE);
    } else if(pid < 0) {
        //erro no fork
        perror("sh");
    } else {
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while(!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    return 1;
}

int sh_execute(char **args) {
    int i;

    if(args[0] == NULL) {
        return 1;
    }

    for(i = 0; i < sh_num_builtins(); i++) {
        if(strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }

    return sh_launch(args);
}

void sh_loop(void){
    char *line;
    char **args;
    int status;

    do {
        printf("> ");
        line = sh_read_line();
        args = sh_split_line(line);
        status = sh_execute(args);

        free(line);
        free(args);
    } while(status);
}

int main(int argc, char **argv){
    sh_loop();

    return 1;
}