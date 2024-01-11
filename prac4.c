#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
int main() {
    char buffer[1000];
    char uptimeOutput[1000]; 
    char separator[] = "* * * * * * * * * * * * * * * * * * * *\n";
    FILE *uptimeStream;
    while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
    	printf("%s", buffer);
    }
    size_t length = strlen(buffer);
    if (length > 0 && buffer[length - 1] == '\n') {
    	buffer[length - 1] = '\0';
    }
    uptimeStream = popen("uptime" , "r");
    if (fgets(uptimeOutput, sizeof(uptimeOutput), uptimeStream) == NULL) {
    	perror("Error");
    	pclose(uptimeStream);
    	return 0;
    } 
    pclose(uptimeStream);
    
    strcat(buffer, separator);
    strcat(buffer, uptimeOutput);
    printf (buffer);
    
    
    //2
    int fd[2], result;
    size_t size;
    char resstring[200];
    if (pipe(fd) < 0) {
    	printf("Cant create pipe");
    	exit(-1);
    }
    result = fork();
    if (!result) {
    	printf("Cant fork child\n");
    	exit(-1);
    } else if (result > 0) {
    	close(fd[0]);
    	size = write(fd[1], buffer, 200);
  	close(fd[1]);
    } else {
    	FILE *file = fopen("file_res1.txt", "w");
    	if (file == NULL) {
    		perror("File error");
    		return 1;
    	}
    	close (fd[1]);
    	size = read(fd[0], resstring, 200);
    	int words = countWords(resstring);
    	fprintf(file,"", words);
    	fclose(file);
    	close(fd[0]);
    }
    
    
    return 0;
}

	int countWords (const char *str) {
		int wordCount = 0;
		bool inWord = false;
		
		while (*str) {
			if (*str == ' ' || *str == '\t' || *str == '\n' || *str == '\r') {
				if (inWord) {
					inWord = false;
					wordCount++;
				}
			} else {
				inWord = true;
			}
			
			str++;
		}
		if (inWord) {
			wordCount++;
		}
		return wordCount;
	}

