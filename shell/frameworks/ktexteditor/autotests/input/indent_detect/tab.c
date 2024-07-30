#include <stdio.h>

typedef struct indent_result {
	int use_spaces;
	int indent_width;
} indent_result_t;

int main() {
	int i;
	indent_result_t ir;
	for (i = 0; i < 100; ++i) {
		if (i == 4)
			return ir.use_spaces;
		else
			return ir.indent_width;
	}
	return 0;
}
