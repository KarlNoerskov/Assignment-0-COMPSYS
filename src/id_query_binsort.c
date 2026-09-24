#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>

#include "record.h"
#include "id_query.h"

struct binsort_record {
    int64_t osm_id; //id of original element
    const struct record *record; //pointer to original element
};

struct binsort_data {
    struct binsort_record *irs;
    int n;
};

int comp(const void *a, const void *b) {
    int64_t x = ((const struct binsort_record *)a)->osm_id;
    int64_t y = ((const struct binsort_record *)b)->osm_id;

    if (x < y)
        return -1;
    if (x > y)
        return 1;
    return 0;
}

int binarySearch(struct binsort_data *data, int64_t x) {
    int low = 0;
    int high = data->n - 1;
    while (low <= high) {
        int mid = low + (high - low) / 2;

        if (data->irs[mid].osm_id == x)
            return mid;

        if (data->irs[mid].osm_id < x)
            low = mid + 1;

        else
            high = mid - 1;
    }
    return -1;
}


struct binsort_data* mk_binsort(struct record* rs, int n) {
    struct binsort_data *pbinsort_data = malloc(sizeof(struct binsort_data));
    pbinsort_data->irs = malloc(n * sizeof(struct binsort_record));
    pbinsort_data->n = n;
    for (int i = 0; i < n; i++)
    {
        pbinsort_data->irs[i].osm_id = rs[i].osm_id;
        pbinsort_data->irs[i].record = &rs[i];
    }
    qsort(pbinsort_data->irs, pbinsort_data->n, sizeof(pbinsort_data->irs[0]), comp);
    return pbinsort_data;
}

const struct record *lookup_binsort(struct binsort_data *data, int64_t needle){
    int x = binarySearch(data, needle);
    if (x == -1){
        return NULL;
    }
    else {
        return data->irs[x].record;
    }
}

void free_binsort(struct binsort_data* data) {
    free(data->irs);
    free(data);
}

int main(int argc, char** argv) {
    return id_query_loop(argc, argv,
                      (mk_index_fn)mk_binsort,
                      (free_index_fn)free_binsort,
                      (lookup_fn)lookup_binsort);
}
