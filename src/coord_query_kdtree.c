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

// Wrapper to match mk_index_fn signature
void* mk_kdtree(const struct record* rs, int n) {
    const struct record **points = malloc(n * sizeof(const struct record *));
    for (int i = 0; i < n; i++) {
        points[i] = &rs[i];
    }
    struct node *root = build_tree(points, n, 0);
    free(points); 
    return root;
}

double get_distance(const struct record *r, double lon, double lat) {
    double dx = r->lon - lon;
    double dy = r->lat - lat;
    return sqrt(dx * dx + dy * dy);
}


void kdtree_lookup_recursive(const struct record **closest, double q_lon, double q_lat, struct node *node) {
    if (node == NULL) return;

    double current_best_dist = *closest ? get_distance(*closest, q_lon, q_lat) : INFINITY;
    double node_dist = get_distance(node->point, q_lon, q_lat);

    if (node_dist < current_best_dist) {
        *closest = node->point;
        current_best_dist = node_dist;
    }

    double diff = (node->axis == 0) ? (node->point->lon - q_lon) : (node->point->lat - q_lat);

    // search the side where the query point lies .
    if (diff >= 0) {
        kdtree_lookup_recursive(closest, q_lon, q_lat, node->left);
        
        current_best_dist = *closest ? get_distance(*closest, q_lon, q_lat) : current_best_dist;
        
        // only search right if the radius crosses the splitting plane
        if (current_best_dist > diff) {
            kdtree_lookup_recursive(closest, q_lon, q_lat, node->right);
        }
    } else {
        // Query is to the right
        kdtree_lookup_recursive(closest, q_lon, q_lat, node->right);
        
        current_best_dist = *closest ? get_distance(*closest, q_lon, q_lat) : current_best_dist;
        
        if (current_best_dist > -diff) { 
            kdtree_lookup_recursive(closest, q_lon, q_lat, node->left);
        }
    }
}


const struct record* lookup_kdtree(void *index, double lon, double lat) {
    const struct record *closest = NULL;
    kdtree_lookup_recursive(&closest, lon, lat, (struct node*)index);
    return closest;
}

void free_tree(void *index) {
    struct node *node = (struct node*)index;
    if (node == NULL) return;
    free_tree(node->left);
    free_tree(node->right);
    free(node);
}

int main(int argc, char** argv) {
    return coord_query_loop(argc, argv,
                            (mk_index_fn)mk_kdtree,
                            (free_index_fn)free_tree,
                            (lookup_fn)lookup_kdtree);
}