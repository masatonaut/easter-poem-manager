// Include necessary headers for file and standard I/O operations, as well as string manipulation and error handling
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h> // For open, creat
#include <sys/types.h> // For open
#include <sys/stat.h> // For open
#include <unistd.h> // For close, write
#include <string.h> // For strcspn, strncat, strlen
#include <errno.h> // For perror
#include <sys/ipc.h>
#include <sys/msg.h>
#include <time.h>
#include <sys/wait.h>

// Define constants for maximum poem length and filenames
#define MAX_POEMS 100
#define NUM_BUNNY_BOYS 4
#define MAX_POEM_LENGTH 1024
#define FILENAME "poems.txt"
#define TEMPFILE "temp.txt"

struct my_msg_st {
    long int my_msg_type;
    char some_text[MAX_POEM_LENGTH];
};

// Function to add a new poem to the file
void addPoem() {
    // Open the file for writing, create it if it doesn't exist, and append to it if it does
    int fd = open(FILENAME, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        // Handle error opening the file
        perror("Error occurred while opening the file");
        return;
    }
    char poem[MAX_POEM_LENGTH];
    // Prompt the user for the poem
    printf("Please enter the poem: ");
    // Read the poem from standard input
    if (fgets(poem, MAX_POEM_LENGTH, stdin) == NULL) {
        // Handle failure to read the poem
        printf("Failed to read the poem.\n");
        close(fd); // Close the file descriptor
        return;
    }
    // Remove the newline character at the end of the poem
    poem[strcspn(poem, "\n")] = 0; // it can be not needed
    // Add a newline character to the end for consistency
    strncat(poem, "\n", MAX_POEM_LENGTH - strlen(poem) - 1);
    // Write the poem to the file
    ssize_t bytes_written = write(fd, poem, strlen(poem));
    if (bytes_written < 0) {
        // Handle error writing to the file
        perror("Error occurred while writing the poem to the file");
        close(fd); // Close the file descriptor
        return;
    }
    // Close the file
    if (close(fd) < 0) {
        // Handle error closing the file
        perror("Error occurred while closing the file");
    }
    // Notify the user of success
    printf("Poem added successfully.\n");
}

// Function to display all poems in the file
void listPoems() {
    // Open the file for reading
    int fd = open(FILENAME, O_RDONLY);
    if (fd < 0) {
        // Handle error opening the file
        perror("Error occurred while opening the file");
        return;
    }

    char poem[MAX_POEM_LENGTH];
    ssize_t bytes_read;
    // Read the poems from the file one block at a time and display them
    while ((bytes_read = read(fd, poem, MAX_POEM_LENGTH - 1)) > 0) {
        poem[bytes_read] = '\0'; // Null-terminate the string
        printf("%s", poem); // Display the poem
    }

    if (bytes_read < 0) {
        // Check for and handle reading error
        perror("Error occurred while reading the poems");
    }

    // Close the file
    if (close(fd) < 0) {
        // Handle error closing the file
        perror("Error occurred while closing the file");
    }
}

void deletePoem() {
    // Prompt the user for the line number of the poem to delete
    int targetLine, currentLine = 1;
    printf("Enter the line number of the poem you wish to delete: ");
    scanf("%d", &targetLine);
    getchar(); // Consume the newline character

    // Open the original file for reading
    int fd = open(FILENAME, O_RDONLY);
    if (fd < 0) {
        // Handle error opening the file for reading
        perror("Error opening the file for reading");
        return;
    }

    // Open a temporary file for writing
    int tempFd = open(TEMPFILE, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (tempFd < 0) {
        // Handle error opening the temporary file for writing
        perror("Error opening the temporary file for writing");
        close(fd);
        return;
    }

    char c;
    int isNewLine = 1; // Flag to handle the first line as a new line
    // Read the original file one character at a time
    while (read(fd, &c, 1) > 0) {
        // Check if we're starting a new line
        if (isNewLine) {
            if (currentLine != targetLine) {
                // If not the target line, reset the flag
                isNewLine = 0;
            } else {
                // If it is the target line, skip it
                while(c != '\n' && read(fd, &c, 1) > 0);
                currentLine++;
                continue;
            }
        }

        // Write character to the temporary file unless it's the target line
        if (write(tempFd, &c, 1) < 0) {
            // Handle error writing to the temporary file
            perror("Error writing to the temporary file");
            close(fd);
            close(tempFd);
            return;
        }

        // Check for a new line and adjust flags and counters accordingly
        if (c == '\n') {
            isNewLine = 1;
            currentLine++;
        }
    }

    // Close both the original and temporary files
    close(fd);
    close(tempFd);

    // Replace the original file with the temporary file
    remove(FILENAME);
    rename(TEMPFILE, FILENAME);

    // Notify the user of successful deletion
    printf("Line %d deleted successfully.\n", targetLine);
}

void modifyPoem() {
    // Prompt the user for the line number to modify
    int lineNum;
    printf("Enter the line number of the poem you wish to modify: ");
    scanf("%d", &lineNum);
    getchar(); // Consume the newline character

    // Prompt the user for the new content of the poem
    char newContent[MAX_POEM_LENGTH];
    printf("Enter the new content for the poem: ");
    fgets(newContent, MAX_POEM_LENGTH, stdin);
    // Ensure the new content ends with a newline character for consistency
    size_t len = strlen(newContent);
    if(newContent[len - 1] != '\n') {
        newContent[len] = '\n';
        newContent[len + 1] = '\0';
    }

    // Open the original file for reading
    int fd = open(FILENAME, O_RDONLY);
    if (fd < 0) {
        // Handle error opening the file
        perror("Error occurred while opening the file for reading");
        return;
    }

    // Open a temporary file for writing
    int tempFd = open(TEMPFILE, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (tempFd < 0) {
        // Handle error opening the temporary file
        perror("Error occurred while opening the temporary file for writing");
        close(fd);
        return;
    }

    char buffer;
    int currentLine = 1;
    // Read the original file one character at a time
    while (read(fd, &buffer, 1) > 0) {
        // Check if the current line is the targeted line for modification
        if (currentLine == lineNum) {
            if (buffer == '\n') {
                // Write the new content to the temporary file instead of the original line
                if (write(tempFd, newContent, strlen(newContent)) < 0) {
                    // Handle error writing the new content
                    perror("Error occurred while writing the new content to the temporary file");
                    close(fd);
                    close(tempFd);
                    return;
                }
                currentLine++;
                continue;
            }
        } else {
            // Write the original content to the temporary file
            if (write(tempFd, &buffer, 1) < 0) {
                // Handle error writing to the temporary file
                perror("Error occurred while writing to the temporary file");
                close(fd);
                close(tempFd);
                return;
            }
            if (buffer == '\n') currentLine++;
        }
    }

    // Close both files
    close(fd);
    close(tempFd);

    // Replace the original file with the updated temporary file
    remove(FILENAME);
    rename(TEMPFILE, FILENAME);

    // Notify the user of successful modification
    printf("Line %d modified successfully.\n", lineNum);
}

// ファイルから詩を読み込む関数
int getPoemsFromFile(char poems[][MAX_POEM_LENGTH], const char* filename, int maxPoems) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("Unable to open the file");
        return -1;
    }

    int count = 0;
    while (fgets(poems[count], MAX_POEM_LENGTH, fp) != NULL) {
        poems[count][strcspn(poems[count], "\n")] = '\0'; // Remove newline
        if (++count >= maxPoems) break;
    }
    fclose(fp);
    return count;
}

void receivePoem(int msgid) {
    struct my_msg_st received_data;
    if (msgrcv(msgid, &received_data, sizeof(received_data), 1, 0) >= 0) {
        printf("Mama Bunny has received the chosen poem: \n%s\n", received_data.some_text);
    } else {
        perror("Failed to receive message");
    }
}

// Watering option 処理を行う関数
void wateringOption(int msgid) {
    char poems[MAX_POEMS][MAX_POEM_LENGTH];
    int numPoems = getPoemsFromFile(poems, FILENAME, MAX_POEMS);
    if (numPoems < 2) {
        printf("Mama Bunny is concerned... Not enough poems are available in the collection.\n");
        return;
    }

    srand(time(NULL)); // Seed the random generator
    int selected_bunny = rand() % NUM_BUNNY_BOYS;
    printf("Mama Bunny has chosen Bunny Boy %d to perform the sacred watering ritual in Barátfa.\n", selected_bunny + 1);

    int pipe_fd[2];
    if (pipe(pipe_fd) != 0) {
        perror("Failed to create pipe");
        return;
    }

    pid_t pid = fork();
    if (pid == 0) { // Child process
        close(pipe_fd[1]); // Close the write end of the pipe
        char buffer[2 * MAX_POEM_LENGTH + 2];
        read(pipe_fd[0], buffer, sizeof(buffer));
        close(pipe_fd[0]);

        int first = rand() % numPoems;
        int second = (first + rand() % (numPoems - 1) + 1) % numPoems; // Ensure different poem

        printf("Upon arriving in Barátfa, Bunny Boy %d has received these poems:\nPoem 1: %s\nPoem 2: %s\n", selected_bunny + 1, poems[first], poems[second]);

        int chosenIndex = (rand() % 2 == 0) ? first : second;
        printf("At the Friendly Tree, he decides to recite:\n%s\nAnd asks, 'May I water!'\n", poems[chosenIndex]);
        sleep(2);
        printf("Watering completed. Bunny Boy %d is now returning home.\n", selected_bunny + 1);

        struct my_msg_st msg;
        msg.my_msg_type = 1;
        strcpy(msg.some_text, poems[chosenIndex]);
        msgsnd(msgid, &msg, sizeof(msg), 0);

        exit(0); // Exit child process cleanly
    } else { // Parent process
        close(pipe_fd[0]);
        char toSend[2 * MAX_POEM_LENGTH + 2];
        int first = rand() % numPoems;
        int second = (first + rand() % (numPoems - 1) + 1) % numPoems;
        sprintf(toSend, "%s\n%s", poems[first], poems[second]);
        write(pipe_fd[1], toSend, strlen(toSend) + 1);
        close(pipe_fd[1]);

        wait(NULL);
        receivePoem(msgid);
    }
}

int main() {
    // Main function to drive the poem management program
    int choice;
    int msgid;

    // メッセージキューの作成
    msgid = msgget((key_t)1234, 0666 | IPC_CREAT);
    if (msgid == -1) {
        fprintf(stderr, "msgget failed with error\n");
        exit(EXIT_FAILURE);
    }

    // Continuously display the menu and prompt the user for their choice
    do {
        // Display menu options
        printf("1. Add a new poem\n");
        printf("2. Display the list of poems\n");
        printf("3. Delete a poem\n");
        printf("4. Modify a poem\n");
        printf("5. Watering Option\n");
        printf("6. Exit\n");
        // Get the user's choice
        printf("Please choose: ");
        scanf("%d", &choice);
        getchar();  // Clear the newline character

        // Handle the user's choice using a switch statement
        switch (choice) {
            case 1:
                addPoem(); // Call function to add a new poem
                break;
            case 2:
                listPoems(); // Call function to list all poems
                break;
            case 3:
                deletePoem(); // Call function to delete a specified poem
                break;
            case 4:
                modifyPoem(); // Call function to modify a specified poem
                break;
            case 5:
                wateringOption(msgid); // Watering option with dynamic pipe handling
                break;
            case 6:
                printf("Exiting.\n"); // Exit the program
                break;
            default:
                printf("Invalid selection.\n"); // Handle invalid menu choices
        }
    } while (choice != 6); // Continue until the user chooses to exit

    // メッセージキューのクリーンアップ
    if (msgctl(msgid, IPC_RMID, NULL) == -1) {
        fprintf(stderr, "msgctl(IPC_RMID) failed\n");
    }

    return 0;
}
