/*
 * Brainf**k -> LLVM IR Compiler
 *  $ gcc bf2llvm.c -o bf2llvm
 *  $ echo "+++++++++[>++++++++>+++++++++++>+++++<<<-]>.>++.+++++++..+++.\
            >-.------------.<++++++++.--------.+++.------.--------.>+." | \
            ./bf2llvm | opt -S -O3 | lli
 */
#include <stdio.h>
#include <stdlib.h>

void emit_header() {
  printf("define i32 @main() {\n");
  printf("  %%data = alloca i8*, align 8\n");
  printf("  %%ptr = alloca i8*, align 8\n");
  printf("  %%data_ptr = call i8* @calloc(i64 30000, i64 1)\n");
  printf("  store i8* %%data_ptr, i8** %%data, align 8\n");
  printf("  store i8* %%data_ptr, i8** %%ptr, align 8\n");
}

int idx = 1;
void emit_move_ptr(int diff) {
  printf("  %%%d = load i8*, i8** %%ptr, align 8\n", idx);
  printf("  %%%d = getelementptr inbounds i8, i8* %%%d, i32 %d\n", idx + 1, idx, diff);
  printf("  store i8* %%%d, i8** %%ptr, align 8\n", idx + 1);
  idx += 2;
}

void emit_add(int diff) {
  printf("  %%%d = load i8*, i8** %%ptr, align 8\n", idx);
  printf("  %%%d = load i8, i8* %%%d, align 1\n", idx + 1, idx);
  printf("  %%%d = add i8 %%%d, %d\n", idx + 2, idx + 1, diff);
  printf("  store i8 %%%d, i8* %%%d, align 1\n", idx + 2, idx);
  idx += 3;
}

void emit_put() {
  printf("  %%%d = load i8*, i8** %%ptr, align 8\n", idx);
  printf("  %%%d = load i8, i8* %%%d, align 1\n", idx + 1, idx);
  printf("  %%%d = sext i8 %%%d to i32\n", idx + 2, idx + 1);
  printf("  %%%d = call i32 @putchar(i32 %%%d)\n", idx + 3, idx + 2);
  idx += 4;
}

void emit_get() {
  printf("  %%%d = call i32 @getchar()\n", idx);
  printf("  %%%d = trunc i32 %%%d to i8\n", idx + 1, idx);
  printf("  %%%d = load i8*, i8** %%ptr, align 8\n", idx + 2);
  printf("  store i8 %%%d, i8* %%%d, align 1\n", idx + 1, idx + 2);
  idx += 3;
}

void emit_while_start(int while_index) {
  printf("  br label %%while_cond%d\n", while_index);
  printf("while_cond%d:\n", while_index);
  printf("  %%%d = load i8*, i8** %%ptr, align 8\n", idx);
  printf("  %%%d = load i8, i8* %%%d, align 1\n", idx + 1, idx);
  printf("  %%%d = icmp ne i8 %%%d, 0\n", idx + 2, idx + 1);
  printf("  br i1 %%%d, label %%while_body%d, label %%while_end%d\n", idx + 2, while_index, while_index);
  printf("while_body%d:\n", while_index);
  idx += 3;
}

void emit_while_end(int while_index) {
  printf("  br label %%while_cond%d\n", while_index);
  printf("while_end%d:\n", while_index);
}

void emit_footer() {
  printf("  %%%d = load i8*, i8** %%data, align 8\n", idx);
  printf("  call void @free(i8* %%%d)\n", idx);
  printf("  ret i32 0\n");
  printf("}\n\n");
  printf("declare i8* @calloc(i64, i64)\n\n");
  printf("declare void @free(i8*)\n\n");
  printf("declare i32 @putchar(i32)\n\n");
  printf("declare i32 @getchar()\n");
}

int main() {
  char c;
  int while_index = 0;
  int while_indices[1000];
  int* while_index_ptr = while_indices;
  emit_header();
  while ((c = getchar()) != EOF) {
    switch (c) {
      case '>': emit_move_ptr(1); break;
      case '<': emit_move_ptr(-1); break;
      case '+': emit_add(1); break;
      case '-': emit_add(-1); break;
      case '[': emit_while_start(*while_index_ptr++ = while_index++); break;
      case ']': if (--while_index_ptr < while_indices) {
                  fprintf(stderr, "unmatching ]\n");
                  return 1;
                }
                emit_while_end(*while_index_ptr); break;
      case '.': emit_put(); break;
      case ',': emit_get(); break;
    }
  }
  emit_footer();
  return 0;
}
