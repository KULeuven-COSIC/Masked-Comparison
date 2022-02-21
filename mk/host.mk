CC = gcc

CFLAGS += -Wall -Wextra -Wimplicit-function-declaration \
          -Wredundant-decls -Wmissing-prototypes -Wstrict-prototypes \
          -Wundef -Wshadow -Wunused-variable\

CFLAGS += $(INCLUDES) $(OPT)

OBJS = $(CFILES:%.c=$(BUILD_DIR)/%.o)

# Targets
$(BUILD_DIR)/%.o: %.c $(HEADERS)
	@printf "  CC\t$<\n"
	@mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) -o $@ $< 

$(PROJECT).bin: $(OBJS) 
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: all
all: $(PROJECT).bin

.PHONY: run
run: all
	./$(PROJECT).bin

.PHONY: clean 
clean:
	rm -rf $(BUILD_DIR) $(PROJECT).bin