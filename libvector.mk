NAME          := libvector.a
CC            := clang
SRCS_DIR      := srcs
OBJS_DIR      := .objs
BUILD_DIR     := build
INCS_DIR      := incs

TESTS_BIN := tester
TESTS_DIR := tests
TESTS_INCS_DIR := $(TESTS_DIR)/srcs
TESTS_SRCS := \
	$(TESTS_DIR)/main.c \
	$(TESTS_DIR)/srcs/unit_tests.c \
	$(TESTS_DIR)/srcs/asserts.c

CFLAGS := \
	-fstack-protector        \
	-fstack-protector-strong \
	-fstack-check            \
	-O3

CFLAGS_DBG := \
-g3                         \
-Og                         \
-Wall                       \
-Werror                     \
-Wextra                     \
-pedantic                   \
-fsanitize=address          \
-fsanitize=undefined        \
-fno-omit-frame-pointer     \
-fstack-protector           \
-fstack-protector-strong    \
-fstack-check               \
-fno-optimize-sibling-calls \
-D DEBUG=1

SRCS := \
	vector.c \
	string.c
