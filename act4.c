/*
 * Simple Line Editor in C
 * ------------------------
 * A terminal-based, command-driven line editor.
 * Data structure: dynamic array of char* (see DESIGN.md for justification).
 *
 * Core features implemented: insert, delete, display, save/load
 * Bonus features implemented: search, line/word count
 *
 * Compile:  gcc -Wall -Wextra -o editor editor.c
 * Run:      ./editor
 * Help:     type 'h' at the prompt, or see HELP.md
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 8
#define MAX_LINE_LEN     1024
#define MAX_INPUT_LEN    1100

typedef struct {
    char **lines;     /* array of pointers to heap-allocated line strings */
    int    count;      /* number of lines currently stored */
    int    capacity;    /* allocated slots in the lines array */
} Document;

/* ---------- lifecycle ---------- */

void doc_init(Document *doc) {
    doc->capacity = INITIAL_CAPACITY;
    doc->count = 0;
    doc->lines = malloc(sizeof(char *) * doc->capacity);
    if (!doc->lines) {
        fprintf(stderr, "Fatal: out of memory during init.\n");
        exit(1);
    }
}

void doc_free(Document *doc) {
    for (int i = 0; i < doc->count; i++) {
        free(doc->lines[i]);
    }
    free(doc->lines);
    doc->lines = NULL;
    doc->count = 0;
    doc->capacity = 0;
}

static void doc_grow_if_needed(Document *doc) {
    if (doc->count >= doc->capacity) {
        doc->capacity *= 2;
        char **grown = realloc(doc->lines, sizeof(char *) * doc->capacity);
        if (!grown) {
            fprintf(stderr, "Fatal: out of memory while growing document.\n");
            exit(1);
        }
        doc->lines = grown;
    }
}

/* ---------- core feature: insert ---------- */
/* lineno is 1-indexed. Valid range for insert is [1, count+1] (count+1 = append at end). */

int doc_insert(Document *doc, int lineno, const char *text) {
    if (lineno < 1 || lineno > doc->count + 1) {
        printf("Error: line number %d is out of range (valid: 1-%d).\n",
               lineno, doc->count + 1);
        return -1;
    }

    doc_grow_if_needed(doc);

    /* shift everything from lineno..end down by one slot */
    int idx = lineno - 1; /* convert to 0-indexed */
    for (int i = doc->count; i > idx; i--) {
        doc->lines[i] = doc->lines[i - 1];
    }

    char *copy = malloc(strlen(text) + 1);
    if (!copy) {
        fprintf(stderr, "Fatal: out of memory during insert.\n");
        exit(1);
    }
    strcpy(copy, text);
    doc->lines[idx] = copy;
    doc->count++;
    return 0;
}

/* ---------- core feature: delete ---------- */

int doc_delete(Document *doc, int lineno) {
    if (doc->count == 0) {
        printf("Error: document is empty, nothing to delete.\n");
        return -1;
    }
    if (lineno < 1 || lineno > doc->count) {
        printf("Error: line number %d is out of range (valid: 1-%d).\n",
               lineno, doc->count);
        return -1;
    }

    int idx = lineno - 1;
    free(doc->lines[idx]);

    /* shift everything after idx up by one slot */
    for (int i = idx; i < doc->count - 1; i++) {
        doc->lines[i] = doc->lines[i + 1];
    }
    doc->count--;
    return 0;
}

/* ---------- core feature: display ---------- */

void doc_display(const Document *doc) {
    if (doc->count == 0) {
        printf("(document is empty)\n");
        return;
    }
    for (int i = 0; i < doc->count; i++) {
        printf("%4d| %s\n", i + 1, doc->lines[i]);
    }
}

/* ---------- core feature: save / load ---------- */

int doc_save(const Document *doc, const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        printf("Error: could not open '%s' for writing.\n", filename);
        return -1;
    }
    for (int i = 0; i < doc->count; i++) {
        fprintf(fp, "%s\n", doc->lines[i]);
    }
    fclose(fp);
    printf("Saved %d line(s) to '%s'.\n", doc->count, filename);
    return 0;
}

int doc_load(Document *doc, const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        printf("Error: could not open '%s' for reading.\n", filename);
        return -1;
    }

    /* clear existing in-memory document before loading */
    for (int i = 0; i < doc->count; i++) {
        free(doc->lines[i]);
    }
    doc->count = 0;

    char buf[MAX_LINE_LEN];
    int loaded = 0;
    while (fgets(buf, sizeof(buf), fp)) {
        size_t len = strlen(buf);
        if (len > 0 && buf[len - 1] == '\n') {
            buf[len - 1] = '\0';
        }
        doc_grow_if_needed(doc);
        char *copy = malloc(strlen(buf) + 1);
        if (!copy) {
            fprintf(stderr, "Fatal: out of memory during load.\n");
            exit(1);
        }
        strcpy(copy, buf);
        doc->lines[doc->count++] = copy;
        loaded++;
    }
    fclose(fp);
    printf("Loaded %d line(s) from '%s'.\n", loaded, filename);
    return 0;
}

/* ---------- bonus feature: search ---------- */

void doc_search(const Document *doc, const char *word) {
    int matches = 0;
    for (int i = 0; i < doc->count; i++) {
        if (strstr(doc->lines[i], word) != NULL) {
            printf("  match on line %d: %s\n", i + 1, doc->lines[i]);
            matches++;
        }
    }
    if (matches == 0) {
        printf("No matches found for \"%s\".\n", word);
    } else {
        printf("%d line(s) matched.\n", matches);
    }
}

/* ---------- bonus feature: line/word count ---------- */

void doc_stats(const Document *doc) {
    int words = 0;
    int chars = 0;
    for (int i = 0; i < doc->count; i++) {
        chars += (int)strlen(doc->lines[i]);
        int in_word = 0;
        for (const char *p = doc->lines[i]; *p; p++) {
            if (*p == ' ' || *p == '\t') {
                in_word = 0;
            } else if (!in_word) {
                in_word = 1;
                words++;
            }
        }
    }
    printf("Lines: %d  Words: %d  Characters: %d\n", doc->count, words, chars);
}

/* ---------- help ---------- */

void print_help(void) {
    printf(
        "Commands:\n"
        "  i <lineno> <text>   Insert text at line number (shifts lines down)\n"
        "  d <lineno>          Delete the line at line number (shifts lines up)\n"
        "  p                   Print/display the whole document\n"
        "  s <filename>        Save document to a text file\n"
        "  o <filename>        Open/load a document from a text file\n"
        "  f <word>            Search for a word/phrase, report matching lines\n"
        "  w                   Show line/word/character counts\n"
        "  h                   Show this help message\n"
        "  q                   Quit the editor\n"
    );
}

/* ---------- command loop ---------- */

/* Splits the leading command token from the rest of the line.
 * Returns pointer to the remainder (may be empty string), never NULL. */
static char *skip_command_token(char *line) {
    char *p = line;
    while (*p && *p != ' ' && *p != '\t' && *p != '\n') p++;
    while (*p == ' ' || *p == '\t') p++;
    return p;
}

int main(void) {
    Document doc;
    doc_init(&doc);

    char input[MAX_INPUT_LEN];

    printf("Simple Line Editor (type 'h' for help, 'q' to quit)\n");

    while (1) {
        printf("> ");
        fflush(stdout);

        if (!fgets(input, sizeof(input), stdin)) {
            printf("\n");
            break; /* EOF (e.g. Ctrl-D) */
        }

        /* strip trailing newline */
        size_t len = strlen(input);
        if (len > 0 && input[len - 1] == '\n') {
            input[len - 1] = '\0';
        }

        if (strlen(input) == 0) {
            continue; /* blank line, just re-prompt */
        }

        char cmd = input[0];
        char *rest = skip_command_token(input);

        switch (cmd) {
            case 'i': {
                int lineno;
                int consumed = 0;
                if (sscanf(rest, "%d%n", &lineno, &consumed) != 1) {
                    printf("Usage: i <lineno> <text>\n");
                    break;
                }
                char *text = rest + consumed;
                while (*text == ' ' || *text == '\t') text++;
                if (*text == '\0') {
                    printf("Error: no text given to insert.\n");
                    break;
                }
                doc_insert(&doc, lineno, text);
                break;
            }
            case 'd': {
                int lineno;
                if (sscanf(rest, "%d", &lineno) != 1) {
                    printf("Usage: d <lineno>\n");
                    break;
                }
                doc_delete(&doc, lineno);
                break;
            }
            case 'p':
                doc_display(&doc);
                break;
            case 's': {
                if (*rest == '\0') {
                    printf("Usage: s <filename>\n");
                    break;
                }
                doc_save(&doc, rest);
                break;
            }
            case 'o': {
                if (*rest == '\0') {
                    printf("Usage: o <filename>\n");
                    break;
                }
                doc_load(&doc, rest);
                break;
            }
            case 'f': {
                if (*rest == '\0') {
                    printf("Usage: f <word>\n");
                    break;
                }
                doc_search(&doc, rest);
                break;
            }
            case 'w':
                doc_stats(&doc);
                break;
            case 'h':
                print_help();
                break;
            case 'q':
                doc_free(&doc);
                printf("Goodbye.\n");
                return 0;
            default:
                printf("Unknown command '%c'. Type 'h' for help.\n", cmd);
                break;
        }
    }

    doc_free(&doc);
    return 0;
}