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
#include <signal.h>

// Define constants for maximum poem length and filenames
#define MAX_POEMS 100
#define NUM_BUNNY_BOYS 4
#define MAX_POEM_LENGTH 1024
#define FILENAME "poems.txt"
#define TEMPFILE "temp.txt"

typedef struct my_msg_st {
    long my_msg_type;
    char some_text[MAX_POEM_LENGTH];
} my_msg_st;

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

void deletePoemByContent(const char* poemToDelete) {
    FILE *fp = fopen(FILENAME, "r");  // Open the original file for reading
    if (!fp) {
        perror("Error opening the file for reading");
        return;
    }

    FILE *fpTemp = fopen(TEMPFILE, "w");  // Open a temporary file for writing
    if (!fpTemp) {
        perror("Error opening the temporary file for writing");
        fclose(fp);  // Make sure to close the original file if opening temp file fails
        return;
    }

    char line[MAX_POEM_LENGTH];
    int found = 0;

    while (fgets(line, sizeof(line), fp) != NULL) {
        // Remove newline character at end if present
        line[strcspn(line, "\n")] = '\0';

        if (strcmp(line, poemToDelete) != 0) {
            // If it's not the poem to delete, write it to the temp file
            fprintf(fpTemp, "%s\n", line);  // Use fprintf to directly write to the file
        } else {
            found = 1;  // Mark that the poem was found and deleted
        }
    }

    // Close both files
    fclose(fp);
    fclose(fpTemp);

    // Replace the original file with the updated temporary file
    if (found) {
        remove(FILENAME);
        rename(TEMPFILE, FILENAME);
        printf("Poem deleted successfully.\n");
    } else {
        printf("Poem not found.\n");
    }
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

// Function to load poems from a file into an array.
int getPoemsFromFile(char poems[][MAX_POEM_LENGTH], const char* filename, int maxPoems) {
    FILE *fp = fopen(filename, "r"); // Open the specified file for reading.
    if (!fp) {
        perror("Unable to open the file"); // Print an error message if the file cannot be opened.
        return -1; // Return -1 to indicate failure.
    }

    int count = 0; // Counter to track the number of poems read.
    // Read lines from the file until the end of the file or the maximum number of poems is reached.
    while (fgets(poems[count], MAX_POEM_LENGTH, fp) != NULL) {
        poems[count][strcspn(poems[count], "\n")] = '\0'; // Remove the newline character at the end of each poem.
        if (++count >= maxPoems) break; // Increment count and break if the maximum number of poems has been read.
    }
    fclose(fp); // Close the file.
    return count; // Return the number of poems successfully read.
}

// Function to receive a poem message via a message queue.
void receivePoem(int msgid) {
    my_msg_st received_data;
    // Attempt to receive a message of type 1 from the message queue.
    if (msgrcv(msgid, &received_data, sizeof(received_data) - sizeof(long), 1, 0) >= 0) {
        // If the message is successfully received, print the poem.5
        printf("Mama Bunny has received the chosen poem: \n%s\n", received_data.some_text);
        deletePoemByContent(received_data.some_text);
    } else {
        // If there is an error receiving the message, print an error message.
        perror("Failed to receive message");
    }
}

void arrivalHandler(int signum) {
    printf("Signal with number %d has arrived - Bunny Boy has reached Barátfa.\n", signum);
}

// Watering option 処理を行う関数
void wateringOption(int msgid) {
    char poems[MAX_POEMS][MAX_POEM_LENGTH];  // 詩を格納する配列を宣言
    int numPoems = getPoemsFromFile(poems, FILENAME, MAX_POEMS);  // ファイルから詩を読み込み、詩の総数を取得
    if (numPoems < 2) {
        // 詩の数が2未満の場合、実行を中断して警告を出力
        printf("Mama Bunny is concerned... Not enough poems are available in the collection.\n");
        return;
    }

    srand(time(NULL)); // 乱数生成器を初期化
    int selected_bunny = rand() % NUM_BUNNY_BOYS;  // ランダムにバニーボーイを選択
    printf("Mama Bunny has chosen Bunny Boy %d to perform the sacred watering ritual in Barátfa.\n", selected_bunny + 1);

    int pipe_fd[2];  // パイプのファイルディスクリプタを格納する配列
    if (pipe(pipe_fd) != 0) {
        // パイプの作成に失敗した場合、エラーを報告して関数を終了
        perror("Failed to create pipe");
        return;
    }

    signal(SIGUSR1, arrivalHandler); // シグナルハンドラを設定

    pid_t pid = fork();  // 子プロセスを作成
    if (pid == 0) {  // 子プロセスの場合の処理
        close(pipe_fd[1]); // 書き込み用のパイプ端を閉じる
        sleep(2);
        kill(getppid(), SIGUSR1); // 親プロセスにシグナルを送信

        char buffer[2 * MAX_POEM_LENGTH + 2];  // 詩を受け取るためのバッファ
        read(pipe_fd[0], buffer, sizeof(buffer));  // パイプから詩を読み込む
        close(pipe_fd[0]);  // 読み込み用のパイプ端を閉じる

        // Splitting received poems for display　e.x.:The road not taken\nStopping by Woods on a Snowy Evening\n
        char *firstPoem = strtok(buffer, "\n");
        char *secondPoem = strtok(NULL, "\n"); // Second poem starts after the null terminator of the first poem

        printf("Upon arriving in Barátfa, Bunny Boy %d has received these poems:\nPoem 1: %s\nPoem 2: %s\n", selected_bunny + 1, firstPoem, secondPoem);

        char *chosenPoem = (rand() % 2 == 0) ? firstPoem : secondPoem;
        printf("At the Friendly Tree, he decides to recite:\n%s\nAnd asks, 'May I water!'\n", chosenPoem);

        my_msg_st msg;// メッセージキューへのメッセージを準備
        msg.my_msg_type = 1; // メッセージタイプを設定
        strcpy(msg.some_text, chosenPoem);// 選択した詩をメッセージにコピー
        if (msgsnd(msgid, &msg, sizeof(msg) - sizeof(long), 0) < 0) {// メッセージを送信 //0 nothing special send as blocking
            perror("Failed to send message");
            exit(1);
        }
        printf("Watering completed. Bunny Boy %d is now returning home.\n", selected_bunny + 1);
        sleep(2);
        exit(0); // 子プロセスを正常に終了させる
    } else {  // 親プロセスの場合の処理
        close(pipe_fd[0]);  // 読み込み用のパイプ端を閉じる
        pause(); // SIGUSR1のシグナルを待つ

        char toSend[2 * MAX_POEM_LENGTH + 2];  // 送信する詩を格納するバッファ
        int first = rand() % numPoems;  // 1つ目の詩をランダムに選択
        int second = (first + rand() % (numPoems - 1) + 1) % numPoems;  // 2つ目の詩をランダムに選択し、1つ目と異なるものを保証
        sprintf(toSend, "%s\n%s", poems[first], poems[second]);  // 送信データを整形
        write(pipe_fd[1], toSend, strlen(toSend) + 1);  // パイプにデータを書き込む
        close(pipe_fd[1]);  // 書き込み用のパイプ端を閉じる

        wait(NULL);  // 子プロセスの終了を待つ
        receivePoem(msgid);  // メッセージキューから詩を受け取る処理を実行
    }
}

int main() {
    // Main function to drive the poem management program
    int choice;
    int msgid;

    //メッセージキューの作成
    msgid = msgget(IPC_PRIVATE, 0666 | IPC_CREAT);
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

    msgctl(msgid, IPC_RMID, NULL);  // Remove the message queue

    return 0;
}
