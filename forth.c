#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EXPECT(x)                                                              \
  if (!(x)) {                                                                  \
    printf("%s:%d:%s:%s\n", __FILE__, __LINE__, __func__, "Expected " #x);     \
    exit(1);                                                                   \
  }

#define MAX_STACK_SIZE 1024

#define struct_stack(type)                                                     \
  struct stack##type {                                                                     \
    type *data;                                                                \
    int current;                                                               \
  };\
  struct stack##type *stack##type##_init() {                                   \
    struct stack##type *stack = malloc(sizeof(struct stack##type));            \
    stack->data = malloc(sizeof(type) * MAX_STACK_SIZE);                       \
    stack->current = -1;                                                        \
    return stack;                                                              \
  }\
  void stack##type##_destroy(struct stack##type *stack) {                      \
    free(stack->data);                                                         \
    free(stack);                                                               \
  }

#define push(stack, value)                                                    \
  if (stack->current >= MAX_STACK_SIZE) {                                      \
    printf("%s:%d:%s:%s\n", __FILE__, __LINE__, __func__, "Stack overflow");   \
    exit(1);                                                                   \
  }                                                                            \
  stack->current++;                                                            \
  stack->data[stack->current] = value;

#define pop(stack)                                                            \
  stack->data[stack->current];                                        \
  stack->current--;\
  if (stack->current < -1) {                                                  \
    printf("%s:%d:%s:%s\n", __FILE__, __LINE__, __func__, "Stack underflow"); \
    exit(1);                                                                   \
  }                                                                            

#define print_stack(stack)                                                     \
  for (int i = stack->current; i >= 0; i--) {                                   \
    printf("%d ", stack->data[i]);                                            \
  }                                                                            \
  printf("\n");

#define READLINE(x)                                                            \
  char *x = NULL;                                                              \
  size_t x_len = 0;                                                            \
  int err = getline(&x, &x_len, stdin);                                        \
  if (err == -1 && errno == EINVAL) {                                          \
    printf("%s:%d:%s:%s\n", __FILE__, __LINE__, __func__, "getline failed");   \
    exit(1);                                                                   \
  }


struct_stack(int)

struct word;
typedef void (*forth_func)(struct word* wordlist, struct stackint *data);

struct word {
  char *word;
  char is_immediate;
  char is_forth_defined;
  forth_func func;
  forth_func* list;

  struct word *prev;
};


struct word* word_add(struct word *head, char *word, forth_func func) {
  struct word *new_word = malloc(sizeof(struct word));
  new_word->word = word;
  new_word->is_immediate = 0;
  new_word->is_forth_defined = 0;
  new_word->func = func;

  if(head == NULL) {
    new_word->prev= NULL;
  } else {
    new_word->prev = head;
  }

  return new_word;
}

void forth_add(struct word *head, struct stackint *data) {
  int a = pop(data);
  int b = pop(data);
  push(data, a + b);
}

void forth_sub(struct word *head, struct stackint *data) {
  int a = pop(data);
  int b = pop(data);
  push(data, b - a);
}

forth_func find_word(struct word *head, char *word) {
  while(head != NULL) {
    if(strcmp(head->word, word) == 0) {
      return head->func;
    }
    head = head->prev;
  }
  return NULL;
}

void forth_print_stack(struct word *head, struct stackint *data) {
  print_stack(data);
}

void destroy_wordlist(struct word *head) {
  if(head == NULL) {
    return;
  } else if(head->prev != NULL) {
    destroy_wordlist(head->prev);
  }
  free(head);
}



void forth_exit(struct word *head, struct stackint *data) {
  printf("Exiting...\n");
  destroy_wordlist(head);
  stackint_destroy(data);
  exit(0);
}

void forth_noop(struct word *head, struct stackint *data) {
  return;
}

char* trim_newline(char *str) {
  int len = strlen(str);
  if(str[len - 1] == '\n') {
    str[len - 1] = '\0';
  }
  return str;
}

int main(int argc, char *argv[]) {
  struct stackint *data = stackint_init();
  struct word *wordlist = NULL;

  wordlist = word_add(wordlist, "+", forth_add);
  wordlist = word_add(wordlist, "-", forth_sub);
  wordlist = word_add(wordlist, ".", forth_print_stack);
  wordlist = word_add(wordlist, "", forth_noop);
  wordlist = word_add(wordlist, "exit", forth_exit);

  while (1) {
    READLINE(line);
    if (line[0] == '\0') {
      break;
    }

    char *token = strtok(line, " ");

    do {
      token = trim_newline(token);
      forth_func func = find_word(wordlist, token);

      if(func == NULL) {
        if(token[0] >= '0' && token[0] <= '9') {
          int i = 1;
          for(; token[i] >= '0' && token[i] <= '9'; i++);
          push(data, atoi(token));
        } else {
          printf("%s:%d:%s:%s: %s\n", __FILE__, __LINE__, __func__, "Unknown word", token);
          exit(1);
        }
      } else {
        func(wordlist, data);
      }
    } while ((token = strtok(NULL, " ")));
    free(line);
  }

  return EXIT_SUCCESS;
}
