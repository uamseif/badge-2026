#ifndef NAME_EDITOR_H
#define NAME_EDITOR_H

#include <stdint.h>
#include <stdbool.h>
#include "cbts_matrix.h"

typedef struct {
    uint8_t cursor;
} NameEditor;

void name_editor_init(NameEditor *ne);
bool name_editor_update(NameEditor *ne, CBTS_MATRIX *display);

#endif /* NAME_EDITOR_H */
