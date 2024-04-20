// Include necessary headers for file and standard I/O operations, as well as string manipulation and error handling
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h> // For open, creat
#include <sys/types.h> // For open
#include <sys/stat.h> // For open
#include <unistd.h> // For close, write
#include <string.h> // For strcspn, strncat, strlen
#include <errno.h> // For perror

// Define constants for maximum poem length and filenames
#define MAX_POEM_LENGTH 1024
#define FILENAME "poems.txt"
#define TEMPFILE "temp.txt"

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

int main() {
    // Main function to drive the poem management program
    int choice;
    // Continuously display the menu and prompt the user for their choice
    do {
        // Display menu options
        printf("1. Add a new poem\n");
        printf("2. Display the list of poems\n");
        printf("3. Delete a poem\n");
        printf("4. Modify a poem\n");
        printf("5. Exit\n");
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
                printf("Exiting.\n"); // Exit the program
                break;
            default:
                printf("Invalid selection.\n"); // Handle invalid menu choices
        }
    } while (choice != 5); // Continue until the user chooses to exit

    return 0;
}
