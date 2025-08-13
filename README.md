# ☬ veerjilang — Punjabi Custom Programming Language

## 📖 What & Why
I built **veerjilang** as a learning project while studying how compilers work — from **lexing → parsing → code generation**.  
I wanted to create something meaningful and fun, so I designed a **custom compiler for Punjabi syntax**, powered by a multi-stage C compiler that outputs **x86-64 NASM assembly**.  

The result is a fully working custom language with its own `.veerji` extension, VS Code icon theme, and an automated compile/run setup using Code Runner + a shell script.

---

## ✍ Language Syntax
The main command for output is:

```

ਲਿਖੋ ☬ ਸਤਿ ਸ੍ਰੀ ਅਕਾਲ

````

**Meaning:** Print `"ਸਤਿ ਸ੍ਰੀ ਅਕਾਲ"` to the terminal.  
- `ਲਿਖੋ` → "write" (print command)  
- `☬` → Khanda symbol used as a separator  
- Everything after → output string  

Example `.veerji` file:
```veerji
ਲਿਖੋ ☬ ਸਤਿ ਸ੍ਰੀ ਅਕਾਲ
ਲਿਖੋ ☬ ਤੁਸੀਂ ਕਿਵੇਂ ਹੋ?
````

---

## ⚙ How It Works

1. **`main.c`** — Reads `.veerji` file → Tokenizes → Parses → Generates Assembly
2. **`lexer.c`** — Detects Punjabi keywords (`ਲਿਖੋ`) and Khanda (`☬`)
3. **`parser.c`** — Builds AST for print statements
4. **`codegen.c`** — Writes Linux syscall-based NASM assembly
5. **`veerji-runner.sh`** — Automates compile → assemble → link → run process in VS Code terminal

---

## ▶ Running `.veerji` Files in VS Code (Code Runner)

I integrated the compiler into VS Code so you can **press one shortcut** and run `.veerji` files instantly.

### 1️⃣ Install Code Runner

Search for **"Code Runner"** in Extensions and install.

### 2️⃣ Create `veerji-runner.sh`

Place this script in your project root:

```bash
#!/bin/bash
# Compile and run a .veerji file
src="$1"
base=$(basename "$src" .veerji)
out="$base"

# Stage 1: Compile to assembly
gcc main.c lexer.c parser.c codegen.c -o veerji-compiler
./veerji-compiler "$src"

# Stage 2: Assemble & Link
nasm -f elf64 out.s -o out.o
ld out.o -o "$out"

# Stage 3: Run
./"$out"
```

Make it executable:

```bash
chmod +x veerji-runner.sh
```

### 3️⃣ VS Code `settings.json`

Add:

```json
{
  "code-runner.executorMap": {
    "veerji": "bash veerji-runner.sh"
  },
  "code-runner.runInTerminal": true
}
```

### 4️⃣ File Association

```json
"files.associations": {
  "*.veerji": "veerji"
}
```

### 5️⃣ Keybinding (optional)

Map Code Runner to a shortcut (e.g., Ctrl+Alt+N) in `keybindings.json`:

```json
{
  "key": "ctrl+alt+n",
  "command": "code-runner.run",
  "when": "editorLangId == 'veerji'"
}
```

---

## 📌 Notes

* Requires **GCC**, **NASM**, and **LD** installed
* Works on **Linux** (syscall-based assembly)
* Khanda symbol (`☬`) is Unicode — make sure your terminal font supports it

---

## 🏁 Example Run

```bash
ਲਿਖੋ ☬ ਸਤਿ ਸ੍ਰੀ ਅਕਾਲ
```

**Output:**

```
ਸਤਿ ਸ੍ਰੀ ਅਕਾਲ
```

📜 License
This project is licensed under the MIT License — see LICENSE for details.

---

## **LICENSE** (MIT Example)
```text
MIT License

Copyright (c) 2025 vortoxin

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

[... standard MIT text continues ...]

---

If you want, I can also add a **section showing the full compiler pipeline visually** so people can see `veerji` → tokens → AST → assembly → binary → output. That would make the README much cooler and educational.

