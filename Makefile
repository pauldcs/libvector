include libvector.mk

SRCS_OBJS := $(patsubst %.c,$(OBJS_DIR)/%.o,$(SRCS))

$(OBJS_DIR)/%.o:$(SRCS_DIR)/%.c
	@mkdir -vp $(dir $@)
	$(CC) \
		$(CFLAGS) \
		-MMD \
		-MP \
		-o $@ \
		-c $< \
		-I $(INCS_DIR)

all: $(NAME)

-include  $(SRCS_OBJS:.o=.d)

$(NAME): $(SRCS_OBJS)
	ar rc \
		$(NAME) \
		$(SRCS_OBJS)

g: CFLAGS += $(CFLAGS_DBG)
g: all

clean:
	rm -rf $(OBJS_DIR)
	rm -rf .cache
	rm -rf .dSYM

format:
	find . \( -name "*.c" -o -name "*.h" \) \
		-type f                               \
		-exec                                 \
			clang-format                        \
				--verbose                         \
				-style=file -i {} \;

fclean: clean
	rm -rf $(NAME)

re: fclean all

.PHONY	: all clean g fclean re format sparse

