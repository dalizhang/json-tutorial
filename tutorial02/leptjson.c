#include "leptjson.h"
#include <assert.h>  /* assert() */
#include <stdlib.h>  /* NULL, strtod() */

#define ISDIGIT(ch)         ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch)     ((ch) >= '1' && (ch) <= '9')

typedef struct {
    const char *json;
} lept_context;

static void lept_parse_whitespace(lept_context *c) {
    const char *p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c->json = p;
}

static int lept_parse_literal(lept_context *c, lept_value *v) {
    if (c->json[0] == 't' && c->json[1] == 'r' && c->json[2] == 'u' && c->json[3] == 'e') {
        c->json += 4;
        v->type = LEPT_TRUE;
        return LEPT_PARSE_OK;
    } else if (c->json[0] == 'f' && c->json[1] == 'a' && c->json[2] == 'l' && c->json[3] == 's' && c->json[4] == 'e') {
        c->json += 5;
        v->type = LEPT_FALSE;
        return LEPT_PARSE_OK;
    } else if (c->json[0] == 'n' && c->json[1] == 'u' && c->json[2] == 'l' && c->json[3] == 'l') {
        c->json += 4;
        v->type = LEPT_NULL;
        return LEPT_PARSE_OK;
    } else {
        return LEPT_PARSE_INVALID_VALUE;
    }
}

static int lept_parse_strtod(lept_context *c, lept_value *v) {
    char *end;
    v->n = strtod(c->json, &end);
    if (c->json == end)
        return LEPT_PARSE_INVALID_VALUE;
    c->json = end;
    v->type = LEPT_NUMBER;
    return LEPT_PARSE_OK;
}

static int lept_parse_number(lept_context *c, lept_value *v) {
    char * jsonP = c->json;
    if (*jsonP == '-') {
        jsonP++;
    }
    if (*jsonP == '0' && !(jsonP[1] == '.' || jsonP[1] == '\0')) {
        return LEPT_PARSE_INVALID_VALUE;
    }
    do {
        if (ISDIGIT(*jsonP)) {
            jsonP++;
        } else {
            return LEPT_PARSE_INVALID_VALUE;
        }
    } while (*jsonP != '.' && *jsonP != 'E' && *jsonP != 'e' && *jsonP != '\0');
    if (*jsonP == '\0') {
        return lept_parse_strtod(c, v);
    } else if (*jsonP == '.') {
        jsonP++;
        do {
            if (ISDIGIT(*jsonP)) {
                jsonP++;
            } else {
                return LEPT_PARSE_INVALID_VALUE;
            }
        } while (*jsonP != 'E' && *jsonP != 'e' && *jsonP != '\0');
    }
    if (*jsonP == '\0') {
        return lept_parse_strtod(c, v);
    } else {
        jsonP++;
    }
    if (*jsonP == '+' || *jsonP == '-') {
        jsonP++;
    }
    do {
        if (ISDIGIT(*jsonP)) {
            jsonP++;
        } else {
            return LEPT_PARSE_INVALID_VALUE;
        }
    } while (*jsonP != '\0');
    return lept_parse_strtod(c, v);
}

static int lept_parse_value(lept_context *c, lept_value *v) {
    switch (*c->json) {
        case 't':
        case 'f':
        case 'n':
            return lept_parse_literal(c, v);
        default:
            return lept_parse_number(c, v);
        case '\0':
            return LEPT_PARSE_EXPECT_VALUE;
    }
}

int lept_parse(lept_value *v, const char *json) {
    lept_context c;
    int ret;
    assert(v != NULL);
    c.json = json;
    v->type = LEPT_NULL;
    lept_parse_whitespace(&c);
    if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {
        lept_parse_whitespace(&c);
        if (*c.json != '\0') {
            v->type = LEPT_NULL;
            ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    return ret;
}

lept_type lept_get_type(const lept_value *v) {
    assert(v != NULL);
    return v->type;
}

double lept_get_number(const lept_value *v) {
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->n;
}
