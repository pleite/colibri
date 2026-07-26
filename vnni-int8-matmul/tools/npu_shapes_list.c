/**
 * npu_shapes_list.c — print the enumerated NPU kernel shape set.
 *
 * The AIE toolchain has to compile one artifact per (rows, inner, out) tuple,
 * and the tuples are enumerated in sched/npu_shapes.c. Rather than repeating
 * that enumeration in the build scripts — where it would drift the first time a
 * projection is added — the build reads it from here, so npu_shapes.c stays the
 * single source of truth.
 *
 * Output is one record per line, tab separated:
 *
 *   rows<TAB>inner<TAB>out<TAB>filename<TAB>role
 *
 * With --json the same records are printed as a JSON array.
 */

#include <stdio.h>
#include <string.h>

#include "../sched/npu_shapes.h"

static void print_json_string(const char *s) {
    putchar('"');
    for (const char *p = s; p && *p; ++p) {
        switch (*p) {
            case '"':  fputs("\\\"", stdout); break;
            case '\\': fputs("\\\\", stdout); break;
            case '\n': fputs("\\n", stdout); break;
            default:   putchar(*p); break;
        }
    }
    putchar('"');
}

int main(int argc, char **argv) {
    int json = 0;
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--json") == 0) {
            json = 1;
        } else {
            fprintf(stderr, "usage: %s [--json]\n", argv[0]);
            return 2;
        }
    }

    size_t count = 0;
    const coli_npu_shape_t *shapes = coli_npu_shape_set(&count);

    if (json) printf("[\n");
    for (size_t i = 0; i < count; ++i) {
        char name[128];
        if (coli_npu_shape_filename(shapes[i].rows, shapes[i].inner, shapes[i].out,
                                    name, sizeof(name)) < 0) {
            fprintf(stderr, "shape %zu has no representable artifact name\n", i);
            return 1;
        }
        if (json) {
            printf("  {\"rows\": %d, \"inner\": %d, \"out\": %d, \"artifact\": ",
                   shapes[i].rows, shapes[i].inner, shapes[i].out);
            print_json_string(name);
            printf(", \"role\": ");
            print_json_string(shapes[i].role ? shapes[i].role : "");
            printf("}%s\n", (i + 1 < count) ? "," : "");
        } else {
            printf("%d\t%d\t%d\t%s\t%s\n", shapes[i].rows, shapes[i].inner,
                   shapes[i].out, name, shapes[i].role ? shapes[i].role : "");
        }
    }
    if (json) printf("]\n");
    return 0;
}
