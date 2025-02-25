#include "lab.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <getopt.h>
#include <pwd.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/wait.h>
#include <errno.h>

/**
 * Parses command line arguments
 */
void parse_args(int argc, char **argv) {
    int c;
    while ((c = getopt(argc, argv, "v")) != -1) {
        switch (c) {
            case 'v':
                // Print version and exit
                printf("Version %d.%d\n", lab_VERSION_MAJOR, lab_VERSION_MINOR);
                exit(EXIT_SUCCESS);
                break;
            default:
                break;
        }
    }
}

/**
 * Set the shell prompt
 */
char *get_prompt(const char *env) {
    const char *prompt_value = getenv(env);
    char *prompt;
    
    if (prompt_value) {
        prompt = strdup(prompt_value);
    } else {
        prompt = strdup("shell>");
    }
    
    return prompt;
}

/**
 * Trim whitespace from beginning and end of string
 */
char *trim_white(char *line) {
    if (!line)
        return line;
    
    // Trim leading whitespace
    char *start = line;
    while (isspace(*start)) start++;
    
    // If string is all whitespace
    if (*start == '\0') {
        *line = '\0';
        return line;
    }
    
    // Trim trailing whitespace
    char *end = start + strlen(start) - 1;
    while (end > start && isspace(*end)) end--;
    
    // Null terminate
    *(end + 1) = '\0';
    
    // Move to front of string if needed
    if (start != line) {
        memmove(line, start, (end - start) + 2);
    }
    
    return line;
}

/**
 * Parse command line into args for exec
 */
char **cmd_parse(char const *line) {
    if (!line)
        return NULL;
    
    // Get maximum args
    long arg_max = sysconf(_SC_ARG_MAX);
    int max_args = (arg_max > 0) ? arg_max : 4096;
    
    // Allocate memory for array of string pointers
    char **cmd = (char **)malloc((max_args + 1) * sizeof(char *));
    if (!cmd)
        return NULL;
    
    memset(cmd, 0, (max_args + 1) * sizeof(char *));
    
    // Make a copy of the line that we can modify
    char *line_copy = strdup(line);
    if (!line_copy) {
        free(cmd);
        return NULL;
    }
    
    // Parse the command
    int i = 0;
    char *token = strtok(line_copy, " ");
    
    while (token != NULL && i < max_args) {
        cmd[i] = strdup(token);
        i++;
        token = strtok(NULL, " ");
    }
    cmd[i] = NULL;  // NULL terminate
    
    free(line_copy);
    return cmd;
}

/**
 * Free memory allocated by cmd_parse
 */
void cmd_free(char **line) {
    if (!line)
        return;
    
    for (int i = 0; line[i] != NULL; i++) {
        free(line[i]);
    }
    free(line);
}

/**
 * Change directory command
 */
int change_dir(char **dir) {
    char *path;
    
    if (!dir[1]) {
        // No argument, go to home directory
        path = getenv("HOME");
        
        if (!path) {
            // If HOME not set, use getpwuid
            uid_t uid = getuid();
            struct passwd *pwd = getpwuid(uid);
            if (pwd) {
                path = pwd->pw_dir;
            } else {
                fprintf(stderr, "cd: could not determine home directory\n");
                return -1;
            }
        }
    } else {
        path = dir[1];
    }
    
    if (chdir(path) != 0) {
        fprintf(stderr, "cd: %s: %s\n", path, strerror(errno));
        return -1;
    }
    
    return 0;
}

/**
 * Handle shell built-in commands
 */
bool do_builtin(struct shell *sh, char **argv) {
    if (!argv || !argv[0])
        return false;
    
    if (strcmp(argv[0], "exit") == 0) {
        // Exit command
        sh_destroy(sh);
        exit(EXIT_SUCCESS);
        return true;
    } else if (strcmp(argv[0], "cd") == 0) {
        // Change directory command
        change_dir(argv);
        return true;
    } else if (strcmp(argv[0], "history") == 0) {
        // History command
        HIST_ENTRY **history = history_list();
        if (history) {
            for (int i = 0; history[i]; i++) {
                printf("%d: %s\n", i + 1, history[i]->line);
            }
        }
        return true;
    }
    
    return false;
}

/**
 * Initialize the shell
 */
void sh_init(struct shell *sh) {
    // Initialize shell variables
    sh->prompt = get_prompt("MY_PROMPT");
    
    // Get the terminal for the shell
    sh->shell_terminal = STDIN_FILENO;
    sh->shell_is_interactive = isatty(sh->shell_terminal);
    
    if (sh->shell_is_interactive) {
        // Wait until we are in the foreground
        while (tcgetpgrp(sh->shell_terminal) != (sh->shell_pgid = getpgrp())) {
            kill(-sh->shell_pgid, SIGTTIN);
        }
        
        // Ignore interactive and job-control signals
        signal(SIGINT, SIG_IGN);
        signal(SIGQUIT, SIG_IGN);
        signal(SIGTSTP, SIG_IGN);
        signal(SIGTTIN, SIG_IGN);
        signal(SIGTTOU, SIG_IGN);
        
        // Put ourselves in our own process group
        sh->shell_pgid = getpid();
        if (setpgid(sh->shell_pgid, sh->shell_pgid) < 0) {
            perror("Couldn't put the shell in its own process group");
            exit(1);
        }
        
        // Grab control of the terminal
        tcsetpgrp(sh->shell_terminal, sh->shell_pgid);
        
        // Save default terminal attributes
        tcgetattr(sh->shell_terminal, &sh->shell_tmodes);
        
        // Initialize readline/history
        using_history();
    }
}

/**
 * Clean up shell resources
 */
void sh_destroy(struct shell *sh) {
    if (sh->prompt) {
        free(sh->prompt);
        sh->prompt = NULL;
    }
    
    // Clear history
    clear_history();
}
