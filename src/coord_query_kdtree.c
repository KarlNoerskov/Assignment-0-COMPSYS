#include <stdlib.h>
#include <math.h>

#include "record.h"
#include "coord_query.h"

struct node {
    const struct record *point;
    int axis;
    struct node *left;
    struct node *right;
};

int compare_lon(const void *a, const void *b) {
    const struct record *ra = *(const struct record * const *)a;
    const struct record *rb = *(const struct record * const *)b;

    if(ra->lon < rb->lon) {
        return -1;
    }

    if (ra->lon > rb->lon) {
        return 1;
    }
    return 0;
}

int compare_lat(const void *a, const void *b) {
    const struct record *ra = *(const struct record * const *)a;
    const struct record *rb = *(const struct record * const *)b;

    if(ra->lat < rb->lat) {
        return -1;
    }

    if (ra->lat > rb->lat) {
        return 1;
    }
    return 0;
}

struct node *build_tree(const struct record **points, int n, int depth) {
    if (n == 0) {
        return NULL;
    }

    int axis = depth % 2;

    if (axis == 0) {
        qsort(points, n, sizeof(points[0]), compare_lon);
    } else {
        qsort(points, n, sizeof(points[0]), compare_lat);
    }

    int mid = n / 2;

    struct node *node = malloc(sizeof(struct node));

    node->point = points[mid];
    node->axis = axis;

    node->left = build_tree(points, mid, depth + 1);
    node->right = build_tree(points + mid + 1,
                             n - mid - 1,
                             depth + 1);

    return node;
}

int main(void) {
    return 0;
}