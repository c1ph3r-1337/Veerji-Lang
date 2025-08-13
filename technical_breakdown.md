TechnicalBreakdown.md — veerjilang compiler (C → NASM x86‑64)
This document gives a deep, file‑by‑file technical breakdown of your veerjilang prototype compiler, covering:

main.c — compiler driver & I/O orchestration

lexer.c — tokenization of Punjabi‑style syntax (e.g., ਲਿਖੋ, ☬)

parser.c — construction of a minimal AST/IR (Program of Statements)

codegen.c — NASM x86‑64 code emission using Linux syscalls

It also explains the data flow (source → tokens → AST → assembly), ABI details for syscalls, typical pitfalls (UTF‑8, escaping), and suggested improvements.

Assumptions from your sources
Your language currently supports a single statement form: print (Punjabi ਲਿਖੋ … or ☬ "…"). The parser emits a Program containing a linear list of Statement{ type=STMT_PRINT, value=char* }. Codegen lays out string literals in .data, then emits _start in .text that performs write(1, msgN, lenN) for each print, and exits via exit(0).

1) main.c — Compiler Driver
High‑level role
main.c is the entrypoint. It:

Reads the input .veerji (or any text) into memory.

Calls tokenize() to lex the source into a dynamically sized token array.

Calls parse() to build a Program (array of Statement).

Calls generate_code(program, "out.s") to write NASM assembly.

Cleans up allocated memory and exits.

Typical structure & important lines
int main(int argc, char** argv) — validates args; prints usage if missing input file.

File read: fopen, fseek, ftell, malloc, fread, buf[len]='\0'. Ensures a NUL‑terminated UTF‑8 buffer (critical for Punjabi text).

Lex: int tok_count = tokenize(src, tokens, MAX_TOKENS) or a growable vector.

On error: prints a diagnostic and exits.

Parse: Program prog = parse(tokens, tok_count).

On error: prints the token index/lexeme where parsing failed and exits.

Codegen: generate_code(prog, "out.s").

User feedback: printf("Assembly written to out.s\n");

Free: frees src, token buffer (if heap), and each Statement.value allocated in the parser.

Data flow at this stage
[.veerji text] --read--> [char* source]
                    --tokenize--> [Token[]]
                    --parse-----> [Program { Statement[] }]
                    --codegen---> [out.s]
2) lexer.c — Tokenizer for Punjabi Syntax
High‑level role
Transforms raw UTF‑8 source into a flat sequence of tokens with types like:

TOKEN_PRINT (lexeme: ਲਿਖੋ or a shorthand symbol ☬)

TOKEN_STRING (contents of quotes, without the quotes)

TOKEN_NEWLINE, TOKEN_EOF, maybe TOKEN_UNKNOWN

Key mechanisms
UTF‑8 handling: Punjabi (ਗੁਰਮੁਖੀ) symbols are multibyte; the lexer must compare UTF‑8 strings, not single bytes, for keywords. In C, this typically means strncmp against known UTF‑8 byte sequences or scanning codepoints safely.

Whitespace & newlines: Skips spaces/tabs; yields TOKEN_NEWLINE on \n to separate statements.

Keywords: Recognizes ਲਿਖੋ (UTF‑8 literal in the source file) and/or ☬.
Example (conceptual):

if (match_keyword(p, "ਲਿਖੋ") || match_keyword(p, "☬")) {
    emit(TOKEN_PRINT);
}
Strings: On ", accumulates until the closing " or EOF, allowing any UTF‑8 bytes inside. Stores the raw bytes (no escape processing yet) and NUL‑terminates for C.

Error token: If a byte sequence doesn’t match any rule, produce TOKEN_UNKNOWN to let the parser complain with location context.

Output structure
A typical token record:

typedef enum { TOKEN_PRINT, TOKEN_STRING, TOKEN_NEWLINE, TOKEN_EOF, TOKEN_UNKNOWN } TokenType;

typedef struct {
    TokenType type;
    char*     lexeme; // heap copy for TOKEN_STRING; keyword tokens may store a static or NULL
    int       line;   // optional: line for error messages
} Token;
Example
Source:

ਲਿਖੋ "ਸਤ ਸ੍ਰੀ ਅਕਾਲ"
☬ "veerjilang rocks"
Tokens (by type/lexeme):

PRINT | STRING("ਸਤ ਸ੍ਰੀ ਅਕਾਲ") | NEWLINE |
PRINT | STRING("veerjilang rocks") | NEWLINE | EOF
3) parser.c — Minimal AST/IR Builder
High‑level role
Consumes the token stream and constructs a linear program of print statements:

typedef enum { STMT_PRINT } StatementType;

typedef struct {
    StatementType type;
    char* value; // heap string from TOKEN_STRING
} Statement;

typedef struct {
    Statement* statements; // heap array (resize as needed)
    int        count;
} Program;
Grammar (simplified)
program      → ( print_stmt NEWLINE* )* EOF
print_stmt   → ( 'ਲਿਖੋ' | '☬' ) STRING
Parsing routine
Maintains an index i into Token[].

While not TOKEN_EOF:

Expect TOKEN_PRINT; if not found → error: "expected ਲਿਖੋ or ☬".

Expect following TOKEN_STRING; on success, dup the string into heap memory.

Append Statement{ STMT_PRINT, value } to Program (growable array via realloc).

Consume any TOKEN_NEWLINE runs.

Errors & memory
On syntax error, prints token position and exits (or returns an error sentinel).

The parser owns the value strings it strdups; main.c/a free_program() should free them.

4) codegen.c — NASM x86‑64 Emission (Linux ABI)
High‑level role
Translates the Program into a .s file with two sections:

.data — each print string as msgN db "…", 0xA and lenN equ $ - msgN (adds a newline)

.text — _start that loops over messages and invokes write(1, msgN, lenN) via syscall, then performs exit(0)

Important pieces (from your file)
void generate_code(Program program, const char* filename) {
    FILE* f = fopen(filename, "w");
    if (!f) { perror("Cannot open output file"); exit(1); }

    fprintf(f, "section .data\n");
    for (int i = 0; i < program.count; i++) {
        if (program.statements[i].type == STMT_PRINT) {
            fprintf(f, "msg%d db \"%s\", 0xA\n", i, program.statements[i].value);
            fprintf(f, "len%d equ $ - msg%d\n", i, i);
        }
    }

    fprintf(f, "\nsection .text\n");
    fprintf(f, "global _start\n");
    fprintf(f, "_start:\n");

    for (int i = 0; i < program.count; i++) {
        if (program.statements[i].type == STMT_PRINT) {
            fprintf(f, "    mov rax, 1\n");      // syscall: write
            fprintf(f, "    mov rdi, 1\n");      // fd = 1 (stdout)
            fprintf(f, "    mov rsi, msg%d\n", i); // buf = &msgN
            fprintf(f, "    mov rdx, len%d\n", i); // len = lenN
            fprintf(f, "    syscall\n\n");
        }
    }

    fprintf(f, "    mov rax, 60\n");  // syscall: exit
    fprintf(f, "    xor rdi, rdi\n"); // status = 0
    fprintf(f, "    syscall\n");

    fclose(f);
}
Linux x86‑64 syscall ABI refresher
Place the syscall number in RAX.

1 = sys_write, 60 = sys_exit

Arguments go in: RDI (arg1), RSI (arg2), RDX (arg3), R10 (arg4), R8 (arg5), R9 (arg6)

Execute syscall instruction; return value is in RAX

Thus, for write(1, buf, len):

mov rax, 1     ; SYS_write
mov rdi, 1     ; fd = stdout
mov rsi, buf   ; pointer to bytes
mov rdx, len   ; number of bytes
syscall
And to exit cleanly:

mov rax, 60    ; SYS_exit
xor rdi, rdi   ; status = 0
syscall
Data layout choices
Each message is emitted as a byte array with a trailing newline (0xA).

lenN equ $ - msgN uses NASM’s current‑location symbol $ to compute the assembled byte length.

Unicode (Punjabi) safety
NASM treats "…" as a raw byte string; the bytes in your char* value are copied as‑is. Ensure the compiler writes UTF‑8 literally and that your terminal font supports Gurmukhi to render correctly.

5) End‑to‑End Example
Input (hello.veerji)
ਲਿਖੋ "ਸਤ ਸ੍ਰੀ ਅਕਾਲ"
☬ "veerjilang rocks"
Tokens (conceptually)
PRINT  STRING("ਸਤ ਸ੍ਰੀ ਅਕਾਲ") NEWLINE
PRINT  STRING("veerjilang rocks") NEWLINE
EOF
AST/IR
Program {
  [0] Statement{ STMT_PRINT, "ਸਤ ਸ੍ਰੀ ਅਕਾਲ" }
  [1] Statement{ STMT_PRINT, "veerjilang rocks" }
}
Generated out.s (abridged)
section .data
msg0 db "ਸਤ ਸ੍ਰੀ ਅਕਾਲ", 0xA
len0 equ $ - msg0
msg1 db "veerjilang rocks", 0xA
len1 equ $ - msg1

section .text
global _start
_start:
    ; print msg0
    mov rax, 1
    mov rdi, 1
    mov rsi, msg0
    mov rdx, len0
    syscall

    ; print msg1
    mov rax, 1
    mov rdi, 1
    mov rsi, msg1
    mov rdx, len1
    syscall

    ; exit(0)
    mov rax, 60
    xor rdi, rdi
    syscall
To assemble & link on Linux:

nasm -felf64 out.s -o out.o
ld -o out out.o
./out
6) Common Pitfalls & Hardening
Escaping quotes in strings: If a user writes " inside a string, your codegen currently emits it verbatim, which can break NASM quoting.
Fix: escape " → \" and \ → \\ during emission.

Embedded newlines inside strings: Consider handling \n in the lexer (convert to byte 0x0A) or keep raw and let users write multiline strings — but adjust length accordingly.

CRLF normalization: On Windows‑edited files, strip \r during lexing.

UTF‑8 validation: Ensure the lexer never splits a multi‑byte sequence mistakenly. Consider a helper that advances by codepoint.

Overflow/limits: Guard i < count in loops; bound maximum string length; check all allocations (malloc/realloc).

Error messages with context: Keep line and (optionally) column in tokens to print file:line:col diagnostics.

Memory ownership: Define free_program(Program*) to free each Statement.value and the array.

7) Suggested Next Steps
Add integers & expressions: Introduce tokens for numbers and + - * /, parse expressions, and map to write via integer→string conversion or printf‑style runtime.

Variables: Add a symbol table and mov/add codegen to registers/stack; or keep an IR layer before assembly.

Function calls / runtime: Instead of raw syscalls, call C library (puts) via dynamic linking for portability.

Testing: Unit‑test lexer (golden token streams) and parser (AST shapes). Emit .dot graphs for AST visualization.

8. Optimization Opportunities

After understanding the complete pipeline, there are several potential areas for optimization:

Tokenization Speed: Implement a more efficient string matching algorithm for Punjabi keywords (e.g., trie-based matching instead of repeated strcmp).

Parser Scalability: Switch from a simple statement-by-statement parser to a proper recursive descent parser for future language expansion.

Codegen Efficiency: Use registers more effectively to reduce syscall overhead.

File I/O: Use memory-mapped files (mmap) for reading .veerji sources for large files.

Error Handling: Replace exit(1) with structured error codes and centralized logging.

9. Testing and Debugging Tools

To ensure correctness and stability:

Lexer Tests: Feed multiple .veerji snippets and verify token streams.

Parser Tests: Compare expected AST structures against parser output.

Codegen Tests: Run generated assembly through a disassembler to verify instructions.

Integration Tests: Compile and run .veerji files automatically, compare output to expected.

Debug Logging: Add -DDEBUG compilation flag to enable detailed state dumps.

10. Future Roadmap

Potential features for Veerjilang:

Variables and Arithmetic: Allow integer/string storage and basic math.

Conditionals and Loops: ਜੇ (if) and ਜਦੋਂ (while) support.

Functions: Define and call reusable Punjabi-named functions.

File I/O: Read/write files from .veerji programs.

Cross-platform Codegen: Support Windows (Win64) syscalls alongside Linux.

JIT Compilation: Embed a JIT engine for immediate execution without writing temp assembly files.

11. Security Considerations

Validate all string lengths before codegen to prevent buffer overflows.

Avoid unsafe functions like gets() or unchecked strcpy().

Sandbox execution when running unknown .veerji code.

Consider adding a restricted mode that disables file/network syscalls.

12. Closing Notes

The Veerjilang compiler already demonstrates a fully working language-to-assembly pipeline using Punjabi syntax. Expanding beyond simple print statements will require careful planning in parsing, code generation, and error handling. With incremental improvements, Veerjilang can evolve into a robust, expressive, and culturally unique programming language.
]

Escaping & raw strings: Support "", \n, \t, \\, and maybe raw string delimiters.
