# Chat Management and Moderation System

A POSIX-compliant multi-process chat monitoring and moderation system built in C, simulating chat groups, users, and a central moderator. Designed for execution on **Ubuntu 22.04+**, this project uses **message queues and unnamed pipes** for inter-process communication (IPC), complying with BITS Pilani OS Assignment guidelines.

---

## 🔧 Project Structure
```
.
├── app.c                # Main application process
├── groups.c             # Group process spawns users and handles communication
├── moderator.c          # Moderator process tracks violations and bans users
├── validation.out       # Provided binary for validating test cases
├── input.txt            # Defines group paths, keys, and thresholds
├── filtered_words.txt   # List of banned words (1 per line)
├── groups/              # Contains group_X.txt files defining users per group
├── users/               # Contains user_X_Y.txt message files
└── Makefile             # (Optional) Automates compilation
```

---

## 📦 Features
- Spawns **group processes** from `app.c`, each of which spawns **user processes**
- Users send timestamped messages to their group using **pipes**
- Groups forward messages to:
  - **moderator** (for filtering)
  - **validation.out** (for timestamp/order verification)
- Moderator tracks violations using **case-insensitive substring match**
- Users are **banned** if violations ≥ threshold
- Groups **terminate** if <2 users remain
- System ensures all processes communicate only via specified **3 message queues + pipes**

---

## 🧪 Test Case Input Format

### input.txt
```
3                          # Number of groups
3430                      # Message queue key (group → validation)
4928                      # Message queue key (group ↔ app)
9131                      # Message queue key (group ↔ moderator)
5                          # Violation threshold
groups/group_0.txt
groups/group_3.txt
groups/group_7.txt
```

### filtered_words.txt
```
hack
ban
leak
```

### groups/group_X.txt
```
3
users/user_X_0.txt
users/user_X_1.txt
users/user_X_2.txt
```

### users/user_X_Y.txt
```
1 HelloWorld
2 NoLeakAllowed
3 JustTesting
```

---

## ⚙️ Setup & Compilation

### Pre-requisites
- Ubuntu 22.04+ (or UTM VM on Mac)
- GCC compiler

### Compile All Files
```bash
gcc app.c -o app.out
gcc groups.c -o groups.out
gcc moderator.c -o moderator.out
chmod 777 validation.out  # Make validation executable
```

---

## 🚀 Execution Steps

Open **3 separate terminals** and execute the following:

### Terminal 1: Start Validation
```bash
./validation.out X
```

### Terminal 2: Start Moderator
```bash
./moderator.out X
```

### Terminal 3: Start App
```bash
./app.out X
```

Where `X` is your test case number.

---

## 🔄 Inter-Process Communication

| Communication                  | Mechanism        |
|-------------------------------|------------------|
| group → validation            | Message Queue 1  |
| group ↔ app                   | Message Queue 2  |
| group ↔ moderator             | Message Queue 3  |
| user ↔ group                  | Pipe (1 per user)|

---

## 📤 Expected Output

### App Terminal
```
All users terminated. Exiting group process 0.
All users terminated. Exiting group process 3.
```

### Moderator Terminal
```
User 2 from group 0 has been removed due to 4 violations.
```

### Validation Terminal
```
Testcase passed!
```

---

## 🛠 Troubleshooting

### Error: `strcasestr` implicit declaration  
💡 Solution: Ensure `#define _GNU_SOURCE` is added at the top of `moderator.c` before `#include <string.h>`.

---

## 🧹 Clean Up IPC Queues
After crashing or re-running:
```bash
ipcs -q           # List queues
ipcrm -q <id>     # Remove queue manually
```

---

## 📚 Learning Outcomes
- Mastery of process creation using `fork()`, `exec()`
- Using unnamed pipes and SysV message queues
- Coordinating complex multi-process systems
- Implementing real-time monitoring and filtering

---

## 👨‍💻 Author
BITS Pilani Hyderabad — OS Assignment | Spring 2024-25  
Developed by: *Your Group Name*  
Members: *NAME1 (ID), NAME2 (ID), NAME3 (ID)*
