# Easter Poem Manager

## Overview

The Easter Poem Manager is a command-line application developed in C, designed for managing a collection of Easter-themed poems. It enables users to effortlessly add new poems to a file and retrieve all stored poems. This program exemplifies basic file handling and user interaction in C, making it a valuable learning tool for beginners.

## Features

- **Add Poems**: Users can append new poems to a designated text file, allowing for persistent storage across program executions.
- **List Poems**: The program can display all poems currently stored in the text file, providing an easy way to view the entire collection.
- **Interactive Menu**: An intuitive command-line menu allows users to navigate the program's features smoothly.

## Prerequisites

- A Unix-like operating system or an Integrated Development Environment (IDE) that supports C programming.
- Standard C libraries must be accessible.
- GCC or any compatible C compiler should be installed.

## Compilation

Navigate to the directory containing the source code in your terminal and execute the following command to compile the Easter Poem Manager:

```sh
gcc main.c -o easter_poem_manager
```

This will compile the `main.c` file and generate an executable named `easter_poem_manager`.

## Usage

To run the program, execute the compiled binary with:

```sh
./easter_poem_manager
```

Upon execution, you will encounter a menu offering the following options:

1. **Add a new poem**: Input the option `1` to append a poem to the file.
2. **Display the list of poems**: Choose option `2` to view all poems stored in the file.
3. **Exit the program**: Select option `3` to terminate the program.

After making a selection, press `Enter` to proceed with the desired action.

## File Structure

- `main.c`: The main source file containing the program logic for adding and listing poems.
- `poems.txt`: The file where poems are stored. If it doesn't exist, the program will automatically create it.

## How It Works

- **Adding a Poem**: Opting to add a poem prompts the user to type in their poem. The program then opens `poems.txt` in append mode to add the new poem, ensuring to handle any errors during the process.
- **Listing Poems**: When listing poems, the program opens `poems.txt` in read-only mode, reads the contents, and prints them to the console, offering a comprehensive view of the collection.
- **Exiting**: Exiting the program ensures all resources are freed correctly, and file descriptors are closed to prevent resource leaks.

## Error Handling

The program includes robust error handling for each file operation. It checks for errors using the `errno` set by system calls, providing detailed feedback to the user on any encountered issues. This approach ensures the program remains stable and informative, even in the face of errors.

Let's revise the README for the Easter Poem Manager by incorporating test cases with the updated sample data format, where each line in the sample file is considered a separate poem. This adjustment will demonstrate the functionality more clearly, especially for adding, listing, deleting, and modifying poems.

---

## Test Cases with Sample Data

These test cases are designed to demonstrate the usage of each function within the Easter Poem Manager, using a format where each line in `poems.txt` represents a separate poem.

### Preparing Sample Data

Initially, let's consider `poems.txt` to contain the following poems, each on its own line:

```
Easter joy fills the air
Families gather with care
Bunnies hop, eggs are found
Laughter and smiles all around
```

This file now includes four individual poems.

### Test Case 1: Adding a New Poem

1. **Action**: Add a new poem by selecting `1` from the menu.
2. **Input**: Enter the following poem when prompted, then press `Enter`:
   ```
   Spring blooms, Easter calls
   ```
3. **Expected Outcome**: The program appends the new poem to the end of `poems.txt`, making it:
   ```
   Easter joy fills the air
   Families gather with care
   Bunnies hop, eggs are found
   Laughter and smiles all around
   Spring blooms, Easter calls
   ```
   A message confirms the successful addition of the poem.

### Test Case 2: Listing All Poems

1. **Action**: To view all poems, choose option `2`.
2. **Expected Outcome**: The program outputs all the poems stored in `poems.txt` to the console, reflecting the addition made in Test Case 1.

### Test Case 3: Deleting a Poem

1. **Action**: Remove the second poem by selecting `3` and entering `2` for the poem's line number when prompted.
2. **Expected Outcome**: The second poem ("Families gather with care") is deleted from `poems.txt`, which now contains:
   ```
   Easter joy fills the air
   Bunnies hop, eggs are found
   Laughter and smiles all around
   Spring blooms, Easter calls
   ```
   The program confirms the poem's deletion with a success message.

### Test Case 4: Modifying a Poem

1. **Action**: Modify the third poem (originally "Bunnies hop, eggs are found") by selecting `4`. When prompted for the poem's line number, enter `3`. For the new content, input:
   ```
   Bunnies hop in fields of gold
   ```
2. **Expected Outcome**: The third poem in `poems.txt` is updated to the new text, resulting in:
   ```
   Easter joy fills the air
   Bunnies hop, eggs are found
   Bunnies hop in fields of gold
   Spring blooms, Easter calls
   ```
   A message indicates the successful modification of the poem.
