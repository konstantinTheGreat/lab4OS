#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>

ssize_t send_message(const char *fifo_name, const char *message) {
    // Открываем канал для записи
    int fd = open(fifo_name, O_WRONLY | O_TRUNC);
    if (fd == -1) {
        perror("Error opening FIFO for writing");
        exit(EXIT_FAILURE);
    }

    // Пишем сообщение в канал
    size_t message_len = strlen(message);
    ssize_t bytes_written = write(fd, message, message_len);
    if (bytes_written == -1) {
        perror("Error writing to FIFO");
        close(fd);
        exit(EXIT_FAILURE);
    }

    close(fd);
    return bytes_written;
}

char *receive_message(const char *fifo_name, int buffer_size) {
    int fd = open(fifo_name, O_RDONLY);
    if (fd == -1) {
        perror("Error opening FIFO for reading");
        exit(EXIT_FAILURE);
    }

    // Считываем сообщение из канала
    char *buffer = malloc(buffer_size);
    if (buffer == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    ssize_t bytes_read = read(fd, buffer, buffer_size);
    if (bytes_read == -1) {
        perror("Error reading from FIFO");
        close(fd);
        free(buffer); // Освобождаем память в случае ошибки
        exit(EXIT_FAILURE);
    }

    close(fd);

    return buffer;
}

ssize_t echo(const char *write_fifo, char *argument) {
    // Выполняем команду echo
    if (argument != NULL) {
        // Отправляем аргумент обратно клиенту
        return send_message(write_fifo, argument);
    } else {
        // Отправляем сообщение об ошибке клиенту
        return send_message(write_fifo, "Invalid usage of echo command");
    }
}

ssize_t get_time(const char *write_fifo) {
    time_t t = time(NULL);
    struct tm date = *localtime(&t);
    char message[100];
    snprintf(message, sizeof(message), "Current date is %d-%02d-%02d %02d:%02d\n", date.tm_year + 1900, date.tm_mon + 1, date.tm_mday, date.tm_hour, date.tm_min);
    return send_message(write_fifo, message);
}

void run_command(const char *write_fifo, const char *command) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("Error forking");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        // Child process
        dup2(open(write_fifo, O_WRONLY | O_TRUNC), STDOUT_FILENO); // Redirect stdout to the client pipe
        execlp("/bin/sh", "/bin/sh", "-c", command, (char *)NULL);
        perror("Error executing command");
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        wait(NULL); // Wait for the child process to complete
    }
}


int main() {
    const char *write_fifo = "client_fifo";
    const char *read_fifo = "server_fifo";
    
	if (mkfifo(write_fifo, 0666) == -1) {
		perror("Failed to create server fifo\n");
		exit(EXIT_FAILURE);
	}

	printf("Creating FIFO...\n");

	sleep(3);
    
    int commands_executed = 0;
    int bytes_written = 0;
    
    void sigusr_handler(int signal) {
    	if(signal == SIGUSR1) {
		    printf("Commands executed: %d\n", commands_executed);
		    printf("Bytes written: %d\n", bytes_written);
    	}
    }
    
    (void) signal(SIGUSR1, sigusr_handler);


    while (1) {
        char *command_line = receive_message(read_fifo, 100);
        if (strncmp(command_line, "stop", 4) == 0) {
		    printf("Commands executed: %d\n", commands_executed);
		    printf("Bytes written: %d\n", bytes_written);
            printf("STOP\n");
            free(command_line);
            break;
        }

        // Парсим команду и её аргументы
        char *command = strtok(command_line, " ");
        printf("Command: %s\n", command);
        char *argument = strtok(NULL, "\n");
        printf("Received argument: %s\n", argument);

        if (command != NULL) {
            if (strncmp(command, "echo", 4) == 0) {
                bytes_written += echo(write_fifo, argument);
            } else if (strncmp(command, "time", 4) == 0) {
                bytes_written += get_time(write_fifo);
            } else if (strncmp(command, "exec", 4) == 0) {
                run_command(write_fifo, argument);
            } else if (strncmp(command, "pid", 3) == 0) {
                char buffer[10];
                snprintf(buffer, sizeof(buffer), "%d", getpid());
                bytes_written += send_message(write_fifo, buffer);
            } else {
                // Отправляем сообщение об ошибке клиенту (неизвестная команда)
                bytes_written += send_message(write_fifo, "Unknown command");
            }
        }

		commands_executed += 1;
	
        printf("Received command from client: %s\n", command_line);
        free(command_line);
    }

    // Удаляем именованный канал
    if (unlink(write_fifo) == -1) {
        perror("Error unlinking FIFO");
        exit(EXIT_FAILURE);
    }

    return 0;
}
