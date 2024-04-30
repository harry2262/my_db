

# Simple Database Implementation in C

This is a simple database implementation in C. It demonstrates the basic principles of building a database engine, including file handling, memory management, and B-tree indexing.

## Features

- **Data Types:** Supports integer IDs, usernames, and email addresses.
- **Operations:** Currently supports `INSERT` and `SELECT` statements.
- **Persistence:** Data is persisted on disk between program executions.
- **Error Handling:** Includes error handling for file operations and input parsing.

## Requirements

- **C Compiler:** Ensure you have a C compiler installed on your system (e.g., GCC).
- **Unix-like System:** This code is primarily designed to run on Unix-like systems.

## Installation

1. Clone the repository to your local machine:

   ```bash
   git clone https://github.com/your_username/your_repository.git
   ```

2. Navigate to the project directory:

   ```bash
   cd your_repository
   ```

3. Compile the code using your preferred C compiler (e.g., GCC):

   ```bash
   gcc main.c -o simple_db
   ```

4. Run the executable:

   ```bash
   ./simple_db your_database_file.db
   ```

## Usage

- Upon running the program, you'll be greeted with a prompt (`db >`).
- Enter SQL-like commands to interact with the database.
- Supported commands:
  - `.exit`: Exit the program.
  - `.btree`: Print the B-tree structure of the database.
  - `.constants`: Print constants used in the database implementation.
  - `INSERT [ID] [Username] [Email]`: Insert a new row into the database.
  - `SELECT`: Retrieve and display all rows from the database.

## Contributing

Contributions are welcome! If you'd like to contribute to this project, feel free to submit a pull request.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

