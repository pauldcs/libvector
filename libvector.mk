NAME          := libvector.a
CC            := clang
SRCS_DIR      := srcs
OBJS_DIR      := .objs
BUILD_DIR     := build
INCS_DIR      := incs

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
-fno-optimize-sibling-calls \
-D DEBUG=1

SRCS := \
	vector.c
